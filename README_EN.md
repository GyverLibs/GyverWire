This is an automatic translation and may be incorrect in some places. See the source README and examples for authoritative information.

[![latest](https://img.shields.io/github/v/release/GyverLibs/GyverWire.svg?color=brightgreen)](https://github.com/GyverLibs/GyverWire/releases/latest/download/GyverWire.zip)
[![PIO](https://badges.registry.platformio.org/packages/gyverlibs/library/GyverWire.svg)](https://registry.platformio.org/libraries/gyverlibs/GyverWire)
[![Foo](https://img.shields.io/badge/Website-AlexGyver.ru-blue.svg?style=flat-square)](https://alexgyver.ru/)
[![Foo](https://img.shields.io/badge/%E2%82%BD%24%E2%82%AC%20%D0%9F%D0%BE%D0%B4%D0%B4%D0%B5%D1%80%D0%B6%D0%B0%D1%82%D1%8C-%D0%B0%D0%B2%D1%82%D0%BE%D1%80%D0%B0-orange.svg?style=flat-square)](https://alexgyver.ru/support_alex/)
[![Foo](https://img.shields.io/badge/README-ENGLISH-blueviolet.svg?style=flat-square)](https://github-com.translate.goog/GyverLibs/GyverWire?_x_tr_sl=ru&_x_tr_tl=en)  

[![Foo](https://img.shields.io/badge/ПОДПИСАТЬСЯ-НА%20ОБНОВЛЕНИЯ-brightgreen.svg?style=social&logo=telegram&color=blue)](https://t.me/GyverLibs)

# GyverWire
Lightweight library for transferring data of any type and size over the GyverWire (GW) software interface
- Reliable DC-balanced Manchester Encoding-based packet communication interface with additional frames
- Reception completely in interruption without polling millis in loop
- Self-synchronization, noise filtering
- Sending and receiving your raw data
- Ready-made tool for creating communication protocols: send any type-specific data, library checks packet integrity with CRC Dallas
- Optional Extended Hamming84 stirred encoding to repair damaged data
- Out of the box implemented transmission by wire, radio 433 MHz and IR channel
- Calculation of communication quality on the receiver

> The library is lighter, more convenient and more reliable than the Gyver433 and GyverTransfer libraries and is designed to replace them.

### Compatibility
Compatible with all Arduino platforms (Arduino features are used)

### Dependencies
- GyverIO
- Hamming

## Contents
- [Use of use](#usage)
- [Versions](#versions)
- [Installation](#install)
- [Bugs and feedback](#feedback)

<a id="usage"></a>

## Use of use
The library contains 3 classes of transmitters and 1 general class of receiver:

### Transmitters
- `GW_TX <pin, baud = 5000>`- for transmission by wire (clean signal without modification)
- `GW_TX_RF <pin, baud = 5000>`433 MHz radio modules and the like (a mechanism for “training” the communication channel is built in)
- `GW_TX_IR <pin, baud = 5000, freq = 38000>`IR channel (38 kHz modulation with inversion)
  - `pin`- pin MK
  - `baud`- speed (bit/s, bod)
  - `freq`- modulation frequency for IR (Hz)

#### GW_TX, GW_TX_IR
```cpp
// Time has passed since the last shipment, ms.
uint16_t lastSend();

// ======== PACKET ========

// Send a package of cars (type GW AUTO TYPE)
void sendPacket(const Td& data);

// Send a package of cars (type GW AUTO TYPE)
void sendPacket(const void* data, size_t len);

// type-bag
void sendPacketT(Tp type, const Td& data);

// type-bag
void sendPacketT(Tp type, const void* data, size_t len);

// ======== RAW ========

// start sending raw data
void beginRaw();

// send out raw data
void sendRaw(const T& data);

// send raw data (can be called several times)
void sendRaw(const void* data, size_t len);

// finish up sending raw data
void endRaw();

// send single raw data (no need to call start + end)
void sendRawSingle(const void* data, size_t len);
void sendRawSingle(const T& data);
```

#### GW_TX_RF
```cpp
GW_TX_RF(uint8_t trainMs = 30);

// set the synchronization time in MS
void setTrain(uint16_t ms);
```

### Receiver
- `GW_RX <pin, baud = 5000, bufsize = 64>`receiver for all types of transmission
  - `pin`- pin MK
  - `baud`- speed (bit/s, bod)
  - `bufsize`Receiving buffer (byte) - *should be larger than the largest potential data packet *

```cpp
// connect a packet handler of the form f(uint8 t type, void* data, size t len)
void onPacket(PacketCallback cb);

// connect a raw data processor of the form f(void* data, size t len)
void onRaw(RawCallback cb);

// call up
void pinChange();

// loop
void tick();

// Get quality of reception as a percentage
uint8_t getRSSI();

// Read in (call in the handler)
template <typename T>
bool readTo(T& var);

// Read how (call in the handler)
template <typename T>
T readAs();
```

### Sending.
Sending blocking, at high transmission speeds, it is recommended to wrap the sending in the prohibition of interruptions to improve the quality of communication.

#### Raw data
To send raw data, you need to call the sending method, send the data, cause completion:

```cpp
GW_TX<2> tx;

struct Data {
  int i;
  float f;
};

uint32_t data_32 = 123456;             // whole
uint8_t data_arr[] = {1, 2, 3, 4, 5};  // stratum
char cstr[10] = "hello";               // char array
String str = "hello";                  // String
Data data{1234, 3.1415};               // structure

// noInterrupts();
tx.beginRaw();

// manually
// tx.sendRaw(&data_32, sizeof(data_32));
// tx.sendRaw(data_arr, sizeof(data_arr));
// tx.sendRaw(cstr, strlen(cstr));
// tx.sendRaw(str.c_str(), str.length());
// tx.sendRaw(&data, sizeof(data));

// size
// tx.sendRaw(data_32);
// tx.sendRaw(data_arr);
// tx.sendRaw(data);

tx.endRaw();
// interrupts();
```

If the data is sent in parts, then we call the beginning, then the necessary number of times sending and completing:

```cpp
tx.beginRaw();
tx.sendRaw(...);
tx.sendRaw(...);
tx.sendRaw(...);
tx.endRaw();
```

If data is sent in one go, it can be used`sendRawSingle()`without calling the start and end of the shipment:

```cpp
tx.sendRawSingle(...);
```

> The library does not control the size and integrity of the data - you can implement your entire communication protocol without overhead

#### Package
The library also allows data to be transmitted over a universal packet communication protocol - method`sendPacketT`specifies the type of data transmitted so that it is more convenient to parse the packet on the receiving device. In this case, the library controls the integrity and size of the data and will not call the processor if it is corrupted. For example, a package`1`- 32 bits whole, package type`2`- byte array,`3`A string of arbitrary length, and so on.

- Packet type - number from 0 to 30
- Data size - up to 2047 bytes

```cpp
GW_TX<2> tx;

struct Data {
  int i;
  float f;
};

uint32_t data_32 = 123456;             // whole
uint8_t data_arr[] = {1, 2, 3, 4, 5};  // stratum
char cstr[10] = "hello";               // char array
String str = "hello";                  // String
Data data{1234, 3.1415};               // structure

// noInterrupts();

// manually
// tx.sendPacketT(0, &data_32, sizeof(data_32));
// tx.sendPacketT(1, data_arr, sizeof(data_arr));
// tx.sendPacketT(2, cstr, strlen(cstr));
// tx.sendPacketT(3, str.c_str(), str.length());
// tx.sendPacketT(4, &data, sizeof(data));

// size
// tx.sendPacketT(0, data_32);
// tx.sendPacketT(1, data_arr);
// tx.sendPacketT(4, data);

// interrupts();
```

The type for convenience may be`enum`:

```cpp
enum class packet_t {
  Data32,
  Array,
  Cstring,
  String,
  Struct,
};

// tx.sendPacketT(packet_t::Data32, data_32);
// tx.sendPacketT(packet_t::Array, data_arr);
// tx.sendPacketT(packet_t::Cstring, cstr, strlen(cstr));
// tx.sendPacketT(packet_t::String, str.c_str(), str.length());
// tx.sendPacketT(packet_t::Struct, data);
```

If you do not specify the package type (method)`sendPacket`), it will be equal to the type`31`parsing`GW_AUTO_TYPE`). It is convenient if there is only one type of package in the system:

```cpp
// manually
// tx.sendPacket(&data_32, sizeof(data_32));
// tx.sendPacket(data_arr, sizeof(data_arr));
// tx.sendPacket(cstr, strlen(cstr));
// tx.sendPacket(str.c_str(), str.length());
// tx.sendPacket(&data, sizeof(data));

// size
// tx.sendPacket(data_32);
// tx.sendPacket(data_arr);
// tx.sendPacket(data);
```

### Reception.
- Asynchronous reception - you need to call the method`pinChange()`at the time of change of the signal on the pin: in interruption`CHANGE`manually
- To obtain data, you need to connect the processing function
- In the main cycle of the program you need to call the ticker`tick()`It will process the packet and call the connected processor.

```cpp
// An example for Arduino Nano

GW_RX<2> rx;  // pin 2, interruption 0

void setup() {
  // !!! poll in interruption of CHANGE
  attachInterrupt(0, []() { rx.pinChange(); }, CHANGE);

  // raw-dataman
  rx.onRaw([](void* data, size_t len) {
    // ...
  });

  // packet-handler
  rx.onPacket([](uint8_t type, void* data, size_t len) {
    // ...
  });
}

void loop() {
  rx.tick();

  // !!! or a manual interview without interruption
  // static bool prev;
  // if (prev != gio::read(2)) {
  //   prev ^= 1;
  //   rx.pinChange();
  // }
}
```

#### Packet parsing
If everything is clear with raw data, then the package, when received, needs to verify the type and convert the data into the desired format. An example of the data sent above:

```cpp
rx.onPacket([](uint8_t type, void* data, size_t len) {
    Serial.print("received type ");
    Serial.print(type);
    Serial.print(": ");

    switch (packet_t(type)) {
        case packet_t::Data32: {
            // You can additionally check the length of the data, for example here if (len = 4).
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
```

### Communication types
#### Radio 433 MHz
Data can be transmitted using the simplest radio modules, in which the transmitter signal is simply duplicated at the receiver output (for example, FS1000A and MX-RM-5V at 433 MHz). For this purpose, a class is used.`GW_TX_RF`When sending data, it adds a "pumping" of the communication channel to synchronize the transmitter and receiver, the builder can transfer the duration of the swing in milliseconds. Upgrades will occur before each data is sent if they are sent less than 50 ms (obtained experimentally). That is, if you send data more often - there will be no swing and it will take less time to transfer.

```cpp
GW_TX_RF<3, 1000> tx(20);   // swing 20 ms, speed 1000
```

The worse the potential quality of communication and the worse the quality of the modules themselves, the longer it takes to pump. Good modules (e.g. green FS1000A and MX-RM-5V) sway in 10 ms, bad modules (e.g. SYNxxx) in 100 ms.

For more stable and high-quality communication, it is recommended to reduce the speed to 1000 and below.

> Maximum stable speed for a pair of green modules (FS1000A and MX-RM-5V): 15,000 baud

#### ICU
The library can modulate the 38 kHz signal for the IR LED so that it is received by the IR receiver - for example, a standard Arduin set of remote, receiver and LED, the LED is connected without inversion (GND-GND, an anode to pin MK through a resistor or transistor for amplification). To send you need to use a class`GW_TX_IR`.

> Maximum stable speed: 1'500 baud

### Quality of communication
#### Filtration
By default, the library uses a noise filter for the receiver class.`GW_RX`- it filters random peaks, when the signal changes state for a short time, this occurs when transmitting by radio or with poor circuitry and transmission by wires (the presence of coils, motors, etc.). For insignificant library relief, the filter can be defiled:

```cpp
#define GW_NO_FILTER
#include <GW_RX.h>
```

#### Hamming
Hamming encoding is enabled for the entire communication (raw and packet) on the transmitter and receiver - this is easy and fast encoding (8.4), allows you to restore damaged data during transmission (up to 12% of damages). The size of the transmitted data is doubled. Enabled by defile, requires an external library[Hamming](https://github.com/GyverLibs/Hamming):

```cpp
// Choose one of two options:
#define GW_USE_HAMMING          // regular (8.4)
// #define GW USE HAMMING MIX // with stirring (more memory consumption, but more reliable)

#include <GW_RX.h>
#include <GW_TX.h>
```

> Note: in mode`GW_USE_HAMMING_MIX`cannot be used for sending through`sendRaw`Several times in a row (within one start-end). There must be only one shipment, or`sendRawSingle`

## Examples
### Line.
Send a package without a type, output to the port the entire accepted length:

```cpp
// shipment
#include <Arduino.h>
#include <GW_TX.h>

void setup() {
}

void loop() {
    GW_TX<3> tx;

    String s;
    s += "hello! ";
    static uint8_t i;
    s += ++i;

    noInterrupts();
    tx.sendPacket(s.c_str(), s.length());
    interrupts();

    delay(1000);
}
```
```cpp
// reception
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
```

### Structure
```cpp
// shipment
#include <Arduino.h>
#include <GW_TX.h>

struct Data {
    int i;
    float f;
};

void setup() {
}

void loop() {
    GW_TX<3> tx;

    static int i;
    Data data{i++, 3.14};

    noInterrupts();
    tx.sendPacket(data);
    interrupts();

    delay(1000);
}
```
```cpp
// reception
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
        if (sizeof(Data) != len) return;  // length-test
        
        Data& d = *((Data*)data);
        Serial.print(d.i);
        Serial.print(',');
        Serial.println(d.f);

        // or
        // Serial.println(static_cast<Data*>(data)->f);
    });
}

void loop() {
    rx.tick();
}
```

<a id="versions"></a>

## Versions
- v1.0

<a id="install"></a>
## Installation
- The library can be found under the name **GyverWire** and installed through the library manager in:
    - Arduino IDE
    - Arduino IDE v2
    - PlatformIO
- [Download the library](https://github.com/GyverLibs/GyverWire/archive/refs/heads/main.zip).zip archive for manual installation:
    - Unpack and put in *C:\Program Files (x86)\Arduino\libraries* (Windows x64)
    - Unpack and put in *C:\Program Files\Arduino\libraries* (Windows x32)
    - Unpack and put in *Documents/Arduino/libraries/ *
    - (Arduino IDE) Automatic installation from .zip: *Sketch/Connect library/Add .ZIP library...* and specify downloaded archive
- Read more detailed instructions for installing libraries[here](https://alexgyver.ru/arduino-first/#%D0%A3%D1%81%D1%82%D0%B0%D0%BD%D0%BE%D0%B2%D0%BA%D0%B0_%D0%B1%D0%B8%D0%B1%D0%BB%D0%B8%D0%BE%D1%82%D0%B5%D0%BA)
### Update
- I recommend always updating the library: new versions fix errors and bugs, as well as optimize and add new features.
- Through the library manager IDE: find the library as when installing and click "Update"
- Manually: **Delete the folder with the old version** and then put the new one in its place. “Replacement” can not be done: sometimes new versions delete files that will remain when replaced and can lead to errors!

<a id="feedback"></a>

## Bugs and feedback
If you find bugs, create **Issue**, or better write to the mail immediately.[alex@alexgyver.ru](mailto:alex@alexgyver.ru)  
The library is open for revision and your **Pull Requests*!

When reporting bugs or incorrect work of the library, it is necessary to specify:
- Library version
- What is used by the IC
- SDK version (for ESP)
- Arduino IDE version
- Are embedded examples that use features and designs that cause bugs in your code working correctly?
- What code was downloaded, what work was expected from it and how it works in reality
- Ideally, attach the minimum code in which the bug is observed. Not a canvas of a thousand lines, but a minimum code.
