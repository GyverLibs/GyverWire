#pragma once
#include <./utils.h>
#include <Arduino.h>
#include <GyverIO.h>

#define GW_MAX_LEN ((1 << (16 - _GW_TYPE_SIZE)) - 1)
#define GW_MAX_TYPE ((1 << _GW_TYPE_SIZE) - 1)
#define GW_AUTO_TYPE GW_MAX_TYPE

#define _GW_RF_MIN_TRAIN 5
#define _GW_RF_DESYNC 50
#define _GW_MODUL_FREQ(freq) uint16_t(1000000ul / freq / 2)

// GW_TX
template <uint8_t pin, int32_t baud = 5000>
class GW_TX {
   public:
    GW_TX() {
        pinMode(pin, OUTPUT);
        gio::write(pin, 0);
    }

    // ======== PACKET ========

    // отправить пакет авто (тип GW_AUTO_TYPE, 255)
    template <typename Td>
    void sendPacket(const Td& data) {
        _sendPacket(GW_AUTO_TYPE, &data, sizeof(Td));
    }

    // отправить пакет авто (тип GW_AUTO_TYPE, 255)
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

    // отправить сырые данные
    void sendRaw(const void* data, size_t len) {
        const uint8_t* p = (const uint8_t*)data;
        if (_first) {
            _first = false;
            if (p[0] & _BV(7)) {
                _writeUs(1, _GW_FRAME(baud));
                _writeUs(0, _GW_FRAME(baud));
                _writeUs(1, _GW_FRAME(baud) * 3);
            } else {
                _writeUs(1, _GW_FRAME(baud));
                _writeUs(0, _GW_FRAME(baud) * 3);
            }
        }
        while (len--) sendByte(*p++);
    }

    // отправить raw байт
    void sendByte(uint8_t b) {
        uint8_t b8 = 8;
        while (b8--) {
            _last = b & _BV(7);
            _writeUs(!_last, _GW_FRAME(baud));
            _writeUs(_last, _GW_FRAME(baud));
            b <<= 1;
        }
    }

    // закончить отправку сырых данных
    void endRaw() {
        if (_last) {
            _writeUs(0, _GW_FRAME(baud) * 2);
            _writeUs(1, _GW_FRAME(baud));
        } else {
            _writeUs(1, _GW_FRAME(baud) * 2);
            _writeUs(0, _GW_FRAME(baud));
            _writeUs(1, _GW_FRAME(baud));
        }
        _writeUs(0, _GW_FRAME(baud));
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

        beginRaw();
        sendRaw(data, len);
        sendRaw(lentype, 2);
        sendByte(crc);
        endRaw();
    }

    virtual void _writeUs(bool v, uint16_t us) {
        gio::write(pin, v);
        delayMicroseconds(us);
    }

   private:
    bool _first = true;
    bool _last;
};

// GW_TX_RF
template <uint8_t pin, int32_t baud = 5000>
class GW_TX_RF : public GW_TX<pin, baud> {
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
        _train(GW_TX<pin, baud>::lastSend() >= _GW_RF_DESYNC ? _trainMs : 0);
        GW_TX<pin, baud>::beginRaw();
    }

   private:
    uint8_t _trainMs;

    using GW_TX<pin, baud>::_writeUs;

    void _train(uint8_t ms) {
        uint16_t n = ms ? (1000ul * ms / (_GW_FRAME(baud) * 4)) : _GW_RF_MIN_TRAIN;
        while (n--) {
            _writeUs(1, _GW_FRAME(baud) * 2);
            _writeUs(0, _GW_FRAME(baud) * 2);
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