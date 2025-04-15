// само-тест. Пины 2 и 3 соединены проводом (3 отправляет, 2 принимает)

#include <Arduino.h>
#include <GW_RX.h>
#include <GW_TX.h>

GW_RX<2> rx;

enum class packet_t {
    Data32,
    Array,
    Cstring,
    String,
    Struct,
};

struct Data {
    int i;
    float f;
};

void setup() {
    Serial.begin(115200);
    attachInterrupt(0, []() { rx.pinChange(); }, CHANGE);

    rx.onPacket([](uint8_t type, void* data, size_t len) {
        Serial.print("received type ");
        Serial.print(type);
        Serial.print(": ");

        switch (packet_t(type)) {
            case packet_t::Data32: {
                // можно дополнительно проверить длину данных, например тут if (len == 4)
                Serial.print(*((uint32_t*)data));
            } break;

            case packet_t::Array: {
                uint8_t* arr = (uint8_t*)data;
                for (size_t i = 0; i < len; i++) {
                    Serial.print(arr[i]);
                    Serial.print(',');
                }
            } break;

            case packet_t::Cstring: {
                Serial.write((uint8_t*)data, len);
            } break;

            case packet_t::String: {
                Serial.write((uint8_t*)data, len);
            } break;

            case packet_t::Struct: {
                Data& p = *((Data*)data);
                Serial.print(p.i);
                Serial.print(',');
                Serial.print(p.f);
            } break;
        }
        Serial.println();
    });
}

void loop() {
    GW_TX<3> tx;

    uint32_t data_32 = 123456;  // целое
    tx.sendPacketT(packet_t::Data32, &data_32, sizeof(data_32));
    rx.tick();
    delay(100);

    uint8_t data_arr[] = {1, 2, 3, 4, 5};  // массив
    tx.sendPacketT(packet_t::Array, data_arr, sizeof(data_arr));
    rx.tick();
    delay(100);

    char cstr[10] = "cstring";  // char array
    tx.sendPacketT(packet_t::Cstring, cstr, strlen(cstr));
    rx.tick();
    delay(100);

    String str = "String";  // String
    tx.sendPacketT(packet_t::String, str.c_str(), str.length());
    rx.tick();
    delay(100);

    Data data{1234, 3.1415};  // структура
    tx.sendPacketT(packet_t::Struct, &data, sizeof(data));
    rx.tick();
    delay(100);

    delay(2000);
}