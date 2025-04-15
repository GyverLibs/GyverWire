// приём
#include <Arduino.h>
#include <GW_RX.h>

GW_RX<2> rx;

void setup() {
    Serial.begin(115200);
    attachInterrupt(0, []() { rx.pinChange(); }, CHANGE);

    rx.onPacket([](uint8_t type, void* data, size_t len) {
        Serial.write((uint8_t*)data, len);
        Serial.println();
    });
}

void loop() {
    rx.tick();
}