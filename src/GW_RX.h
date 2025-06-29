#pragma once
#include <./utils.h>
#include <Arduino.h>
#include <GyverIO.h>
#include <Hamming.h>

#define _GW_TOLERANCE 2
#define _GW_FILT_DIV 5
#define _GW_SHIFTED(baud, mult, sign) (_GW_FRAME(baud) * mult + _GW_FRAME(baud) * sign / _GW_TOLERANCE)
#define _GW_WINDOW(baud, mult) _GW_SHIFTED(baud, mult, -1)... _GW_SHIFTED(baud, mult, 1) - 1

template <uint8_t pin, int32_t baud = 5000, size_t bufsize = 64>
class GW_RX {
    typedef void (*PacketCallback)(uint8_t type, void* data, size_t len);
    typedef void (*RawCallback)(void* data, size_t len);

#if defined(GW_USE_HAMMING) || defined(GW_USE_HAMMING_MIX)
    static constexpr size_t GW_BUFSIZE = (bufsize + 3) * 2;
#else
    static constexpr size_t GW_BUFSIZE = bufsize + 3;
#endif

    enum class State : uint8_t {
        Idle,
        Reading,
        Done,
    };

    // FrontFilter
    template <uint8_t noise>
    class FrontFilter {
       public:
        int32_t getPulse(uint16_t t) {
            int32_t pulse = -1;
            if (_front) {
                if (uint16_t(t - _buf) < noise) {
                    _front = false;
                    return -1;
                } else {
                    pulse = uint16_t(_pt - _ppt);
                    _ppt = _pt;
                }
            }
            _front = true;
            _buf = _pt = t;
            return pulse;
        }

       private:
        uint16_t _pt = 0;   // значение, которое потенциально является фронтом
        uint16_t _ppt = 0;  // предыдущий _pt
        uint16_t _buf = 0;  // значение прошлого сигнала
        bool _front = 0;    // флаг ожидания окончания обработки фронта
    };

   public:
    GW_RX() {
        pinMode(pin, INPUT);
    }

    // подключить обработчик пакета вида f(uint8_t type, void* data, size_t len)
    void onPacket(PacketCallback cb) {
        _pack_cb = cb;
    }

    // подключить обработчик сырых данных вида f(void* data, size_t len)
    void onRaw(RawCallback cb) {
        _raw_cb = cb;
    }

    // прочитать в (вызывать в обработчике)
    template <typename T>
    bool readTo(T& var) {
        if (_state != State::Done || sizeof(var) != _len) return false;
        memcpy(&var, _buf, _len);
        return true;
    }

    // прочитать как (вызывать в обработчике)
    template <typename T>
    T readAs() {
        T var;
        readTo(var);
        return var;
    }

    // вызывать при изменении сигнала на пине
    void pinChange() {
#ifndef GW_NO_FILTER
        switch (_filt.getPulse(micros())) {
            case -1: return;
#else
        uint16_t pulse = uint16_t(micros()) - _prevUs;
        _prevUs += pulse;
        switch (pulse) {
#endif
            default:
                if (_state == State::Reading) _state = State::Idle;
                return;

            case _GW_WINDOW(baud, 3):
                if (_state != State::Done) {
                    _state = State::Reading;
                    _pinv = gio::read(pin);
#ifndef GW_NO_FILTER
                    _pinv ^= 1;
#endif
                    _len = _bit = 0;
                    _edge = 1;
                }
                return;

            case _GW_WINDOW(baud, 1):
                if (_state != State::Reading) return;
                _edge ^= 1;
                break;

            case _GW_WINDOW(baud, 2):
                if (_state != State::Reading) return;
                if (_edge) {
                    _state = (_len && !_bit) ? State::Done : State::Idle;
                    return;
                }
                break;
        }

        _pinv ^= 1;
        if (!_edge) {
            if (_len >= GW_BUFSIZE) {
                _state = State::Idle;
                return;
            }
            _buf[_len] <<= 1;
            if (_pinv) _buf[_len] |= 1;
            if (++_bit >= 8) {
                _bit = 0;
                ++_len;
            }
        }
    }

    // вызывать в loop
    void tick() {
        if (_state == State::Done) {
#if defined(GW_USE_HAMMING)
            if (!Hamming3::decode(_buf, _len)) {
                _state = State::Idle;
                return;
            }
            _len = Hamming3::decodedSize(_len);
#elif defined(GW_USE_HAMMING_MIX)
            if (!Hamming3::unmix8(_buf, _len) || !Hamming3::decode(_buf, _len)) {
                _state = State::Idle;
                return;
            }
            _len = Hamming3::decodedSize(_len);
#endif

            if (_raw_cb) _raw_cb(_buf, _len);
            // if (_pack_cb && _len > 2 && !gwutil::crc8(_buf, _len)) {
            //     _pack_cb(_buf[_len - 2], _buf, _len - 2);
            // }
            if (_pack_cb) {
                _rssi <<= 1;
                if (_len > 3 && !gwutil::crc8(_buf, _len)) {
                    uint16_t len = ((_buf[_len - 3] << 8) | _buf[_len - 2]) >> _GW_TYPE_SIZE;
                    if (len == _len - 3) {
                        _rssi |= 1;
                        uint8_t type = _buf[_len - 2] & _GW_TYPE_MASK;
                        _len -= 3;
                        _pack_cb(type, _buf, _len);
                    }
                }
            }
            _state = State::Idle;
        }
    }

    // получить качество приёма в процентах
    uint8_t getRSSI() {
        return ((countBits(_rssi) + countBits(_rssi >> 8)) * 100) >> 4;
    }

   private:
#ifndef GW_NO_FILTER
    FrontFilter<_GW_FRAME(baud) / _GW_FILT_DIV> _filt;
#else
    uint16_t _prevUs = 0;
#endif
    uint8_t _buf[GW_BUFSIZE];
    PacketCallback _pack_cb = nullptr;
    RawCallback _raw_cb = nullptr;
    uint16_t _len = 0;
    uint16_t _rssi = 0xffff;
    uint8_t _bit = 0;
    State _state = State::Idle;
    bool _edge;
    bool _pinv;

    static uint8_t countBits(uint8_t x) {
        x -= (x >> 1) & 0x55;
        x = (x & 0x33) + ((x >> 2) & 0x33);
        return (x + (x >> 4)) & 0x0F;
    }
};