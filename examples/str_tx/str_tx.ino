// отправка
#include <Arduino.h>
#include <GW_TX.h>

void setup() {
}

GW_TX<3> tx;     // провод
// GW_TX_IR<3> tx;  // ИК диод
// GW_TX_RF<3> tx;  // радио 433

void loop() {
    String s;
    s += "hello! ";
    static uint8_t i;
    s += ++i;

    noInterrupts();
    tx.sendPacket(s.c_str(), s.length());
    interrupts();

    delay(1000);
}