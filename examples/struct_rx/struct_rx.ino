// приём
#include <Arduino.h>
#include <GW_RX.h>

struct Data {
    int i;
    float f;
};

GW_RX<2> rx;

void setup() {
    Serial.begin(115200);
    attachInterrupt(0, []() { rx.pinChange(); }, CHANGE);

    rx.onPacket([](uint8_t type, void* data, size_t len) {
        if (sizeof(Data) != len) return;  // проверка корректности длины
        Data& d = *((Data*)data);
        Serial.print(d.i);
        Serial.print(',');
        Serial.println(d.f);

        // или
        // Serial.println(static_cast<Data*>(data)->f);
    });
}

void loop() {
    rx.tick();
}