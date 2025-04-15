// отправка
#include <Arduino.h>
#include <GW_TX.h>

struct Data {
    int i;
    float f;
};

void setup() {
}

GW_TX<3> tx;     // провод
// GW_TX_IR<3> tx;  // ИК диод
// GW_TX_RF<3> tx;  // радио 433

void loop() {
    static int i;
    Data data{i++, 3.14};

    noInterrupts();
    tx.sendPacket(data);
    interrupts();

    delay(1000);
}