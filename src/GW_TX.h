#pragma once
#include <./utils.h>
#include <Arduino.h>
#include <GyverIO.h>

// #ifdef GW_USE_HAMMING
#include <Hamming.h>
// #endif

#define GW_MAX_LEN ((1 << (16 - _GW_TYPE_SIZE)) - 1)
#define GW_MAX_TYPE ((1 << _GW_TYPE_SIZE) - 1)
#define GW_AUTO_TYPE GW_MAX_TYPE

#define _GW_RF_MIN_TRAIN 5  // мин. кол-во импульсов для раскачки радио
#define _GW_RF_DESYNC 50    // минимальный период отправки по радио, мс
#define _GW_MODUL_FREQ(freq) uint16_t(1000000ul / freq / 2)

// GW_TX
template <uint8_t pin, int32_t baud = 5000>
class GW_TX {
   protected:
    static constexpr size_t GW_FRAME = _GW_FRAME(baud);

   public:
    GW_TX() {
        pinMode(pin, OUTPUT);
        gio::write(pin, 0);
    }

    // ======== PACKET ========

    // отправить пакет авто (тип GW_AUTO_TYPE)
    template <typename Td>
    void sendPacket(const Td& data) {
        _sendPacket(GW_AUTO_TYPE, &data, sizeof(Td));
    }

    // отправить пакет авто (тип GW_AUTO_TYPE)
    void sendPacket(const void* data, size_t len) {
        _sendPacket(GW_AUTO_TYPE, data, len);
    }

    // отправить пакет с типом
    template <typename Tp, typename Td>
    void sendPacketT(Tp type, const Td& data) {
        _sendPacket(uint8_t(type), &data, sizeof(Td));
    }

    // отправить пакет с типом
    template <typename Tp>
    void sendPacketT(Tp type, const void* data, size_t len) {
        _sendPacket(uint8_t(type), data, len);
    }

    // ======== RAW ========

    // начать отправку сырых данных
    virtual void beginRaw() {
        _first = true;
    }

    // отправить сырые данные
    template <typename T>
    void sendRaw(const T& data) {
        sendRaw(&data, sizeof(T));
    }

    // отправить одиночные сырые данные (не нужно вызывать begin + end)
    template <typename T>
    void sendRawSingle(const T& data) {
        sendRawSingle(&data, sizeof(T));
    }

    // отправить одиночные сырые данные (не нужно вызывать begin + end)
    void sendRawSingle(const void* data, size_t len) {
        beginRaw();
        sendRaw(data, len);
        endRaw();
    }

    // отправить сырые данные
    void sendRaw(const void* data, size_t len) {
#if defined(GW_USE_HAMMING)
        const uint8_t* p = (const uint8_t*)data;
        if (_first) _startFrame(Hamming3::encode(*p & 0xf));

        while (len--) {
            _sendByte(Hamming3::encode(*p & 0xf));
            _sendByte(Hamming3::encode(*p >> 4));
            ++p;
        }

#elif defined(GW_USE_HAMMING_MIX)
        size_t elen = Hamming3::encodedSize(len);
        uint8_t* buf = new uint8_t[elen];
        if (!buf) return;

        Hamming3::encode(buf, data, len);
        Hamming3::mix8(buf, elen);

        uint8_t* p = buf;
        _startFrame(*p);
        while (elen--) _sendByte(*p++);
        delete[] buf;
#else
        const uint8_t* p = (const uint8_t*)data;
        _startFrame(*p);
        while (len--) _sendByte(*p++);
#endif
    }

    // закончить отправку сырых данных
    void endRaw() {
        if (_last) {
            _writeUs(0, GW_FRAME * 2);
            _writeUs(1, GW_FRAME);
        } else {
            _writeUs(1, GW_FRAME * 2);
            _writeUs(0, GW_FRAME);
            _writeUs(1, GW_FRAME);
        }
        _writeUs(0, GW_FRAME);
        _lastSend = millis();
    }

    // прошло времени с конца последней отправки, мс
    uint16_t lastSend() {
        return uint16_t(millis()) - _lastSend;
    }

   protected:
    uint16_t _lastSend = 0;

    void _sendPacket(uint8_t type, const void* data, size_t len) {
        // uint8_t crc = gwutil::crc8(data, len);
        // crc = gwutil::crc8(&type, 1, crc);

        // beginRaw();
        // sendRaw(data, len);
        // sendByte(type);
        // sendByte(crc);
        // endRaw();

        if (len > GW_MAX_LEN || type > GW_MAX_TYPE) return;

        uint8_t lentype[2];
        lentype[0] = (len << _GW_TYPE_SIZE) >> 8;
        lentype[1] = (len << _GW_TYPE_SIZE) | (type & _GW_TYPE_MASK);
        uint8_t crc = gwutil::crc8(data, len);
        crc = gwutil::crc8(lentype, 2, crc);

#ifdef GW_USE_HAMMING_MIX
        size_t elen = Hamming3::encodedSize(len + 3);
        uint8_t* buf = new uint8_t[elen];
        if (!buf) return;

        uint8_t* p = buf;
        Hamming3::encode(p, data, len), p += Hamming3::encodedSize(len);
        Hamming3::encode(p, lentype, 2), p += Hamming3::encodedSize(2);
        Hamming3::encode(p, &crc, 1);

        Hamming3::mix8(buf, elen);

        p = buf;
        beginRaw();
        _startFrame(*p);
        while (elen--) _sendByte(*p++);
        endRaw();
        delete[] buf;
#else
        beginRaw();
        sendRaw(data, len);
        sendRaw(lentype, 2);
        sendRaw(&crc, 1);
        endRaw();
#endif
    }

    virtual void _writeUs(bool v, uint16_t us) {
        gio::write(pin, v);
        delayMicroseconds(us);
    }

   private:
    bool _first = true;
    bool _last = false;

    void _sendByte(uint8_t b) {
        uint8_t b8 = 8;
        while (b8--) {
            _last = b & _BV(7);
            _writeUs(!_last, GW_FRAME);
            _writeUs(_last, GW_FRAME);
            b <<= 1;
        }
    }
    void _startFrame(uint8_t b) {
        if (_first) {
            _first = false;
            if (b & _BV(7)) {
                _writeUs(1, GW_FRAME);
                _writeUs(0, GW_FRAME);
                _writeUs(1, GW_FRAME * 3);
            } else {
                _writeUs(1, GW_FRAME);
                _writeUs(0, GW_FRAME * 3);
            }
        }
    }
};

// GW_TX_RF
template <uint8_t pin, int32_t baud = 5000>
class GW_TX_RF : public GW_TX<pin, baud> {
    typedef GW_TX<pin, baud> GW;

   public:
    GW_TX_RF(uint8_t trainMs = 30) {
        setTrain(trainMs);
    }

    // установить время раскачки синхронизации в мс
    void setTrain(uint8_t ms) {
        _trainMs = ms;
    }

    // начать отправку сырых данных
    void beginRaw() override {
        _train(GW::lastSend() >= _GW_RF_DESYNC ? _trainMs : 0);
        GW::beginRaw();
    }

   private:
    uint8_t _trainMs;

    using GW::_writeUs;
    using GW::GW_FRAME;

    void _train(uint8_t ms) {
        uint16_t n = ms ? (1000ul * ms / (GW_FRAME * 4)) : _GW_RF_MIN_TRAIN;
        while (n--) {
            _writeUs(1, GW_FRAME * 2);
            _writeUs(0, GW_FRAME * 2);
        }
    }
};

// GW_TX_IR
template <uint8_t pin, int32_t baud = 5000, uint32_t freq = 38000>
class GW_TX_IR : public GW_TX<pin, baud> {
   private:
    void _writeUs(bool v, uint16_t us) override {
        v ? delayMicroseconds(us) : _pulse38kHz(us);
    }

    void _pulse38kHz(uint16_t dur) {
        dur = (dur / _GW_MODUL_FREQ(freq)) & ~1;  // до чётного
        bool f = 0;
        while (dur--) {
            gio::write(pin, f ^= 1);
            delayMicroseconds(_GW_MODUL_FREQ(freq));
        }
    }
};