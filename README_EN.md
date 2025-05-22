This is an automatic translation, may be incorrect in some places. See sources and examples!

# Gyverwire
Easy Library for transmitting data of any type and size on the program interface Gyverwire (GW)
- reliable DC-Balanced Balance Communication Interface based on Manchester Encoding with additional frames
- reception completely in interruption without polling millisa in LOOP
- Self -Synchronization, noise filtering
- Sending and receiving completely their "raw" data
- Ready -made tool for creating communication protocols: sending any data indicating the type, the library checks the integrity of the package using CRC Dallas
- Optionally Extnded Hamming84 Coding with mixing to restore damaged during data transfer
- from the box, a transmission by wire, radio 433 MHz and IR channel
- calculation of the quality of communication at the receiver

> The library is easier, more convenient and reliable than the Gyver433 and Gyvertransfer libraries and is designed to replace them

## compatibility
Compatible with all arduino platforms (used arduino functions)

### Dependencies
- Gyverio
- Hamming

## Content
- [use] (#usage)
- [versions] (#varsions)
- [installation] (# Install)
- [bugs and feedback] (#fedback)

<a id = "USAGE"> </A>

## Usage
The library contains 3 classes of transmitters and 1 General class of the receiver:

### transmitters
- `gw_tx <pin, baud = 5000>` - for transmission by wire (clean signal without modifications)
- `gw_tx_rf <pin, baud = 5000>` `Radio Moduli 433 MHz and the similar ones (the mechanism of" training "of the communication channel)
- `gw_tx_ir <pin, baud = 5000, freq = 38000>` ik channel (modulation 38 kHz with inversion)
- `pin` - PIN MK
- `baud` - speed (bit/s, body)
- `freq` - modulation frequency for IR (Hz)

### gw_tx, gw_tx_ir
`` `CPP
// time has passed since the end of the last shipment, ms
uint16_t LastSend ();

// ======== Packet ==========

// Send a car package (type gw_auto_type)
Void Sendpacket (Consta TD & DATA);

// Send a car package (type gw_auto_type)
VOID SENDPACket (Consta* Data, Size_t Len);

// Send a package with a type
Void Sendpackett (TP Type, Consta TD & DATA);

// Send a package with a type
Void Sendpackett (TP Type, Consta* Data, Size_t Len);

// ======== RAW =============

// Start sending raw data
VOID Beginraft ();

// Send raw data
VOID SENDRAW (Consta T & Data);

// Send raw data (you can call several times)
VOID SENDRAW (const VOID* DATA, SIZE_T LEN);

// Get the sending of raw data
VOID Endraw ();

// Send single raw data (no need to call Begin + End)
VOID SENDRAWSINGLE (COST VOID* DATA, SIZE_T LEN);
VOID SENDRAWSINGLE (COST T&DATA);
`` `

### gw_tx_rf
`` `CPP
Gw_tx_rf (uint8_t Trainms = 30);

// set the time to build synchronization in MS
VOID Settrain (Uint16_T MS);
`` `

### receiver
- `gw_rx <pin, baud = 5000, bufsize = 64>` - receiver for all transmission types
- `pin` - PIN MK
- `baud` - speed (bit/s, body)
- `buffsize` - the size of the reception buffer (byte) - *should be larger than the largest potential data package *

`` `CPP
// Connect the packet handler F (Uint8_t Type, Void* Data, Size_t Len)
VOID Onpacket (PacketCallback CB);

// ByDRAILLED Raw Data Processing Type F (Void* Data, Size_t Len)
VOID online (RAWCALLBACK CB);

// Call when changing the signal on the pin
VOID PINCHANGE ();

// Call in Loop
VOID Tick ();

// Get the quality of reception as a percentage
uint8_t getrssi ();
`` `

### Sending
Sending blocking, at high transmission speeds it is recommended to wrap the sending into the ban on interruptions to improve the quality of communication.

### raw data
To send raw data, you need to call the method of starting sending, sending data, providing completion:

`` `CPP
GW_TX <2> TX;

Struct Data {
int i;
Float F;
};

uint32_t data_32 = 123456;// whole
uint8_t data_arr [] = {1, 2, 3, 4, 5};// Massive
Char CSTR [10] = "HELLO";// char Array
String str = "Hello";// String
Data Data {1234, 3.1415};// Structure

// nointerrupts ();
TX.BeginRAW ();

// Size manually
// TX.SendRAW (& DATA_32, SIZEOF (DATA_32));
// tx.sendraw (Data_arr, Sizeof (Data_arr));
// tx.sendraw (CSTR, Strlen (CSTR));
// tx.sendraw (str.c_str (), str.length ());
// TX.SendRAW (& DATA, SIZEOF (DATA));

// Auto size
// tx.sendraw (Data_32);
// tx.sendraw (Data_arr);
// tx.Sendraw (Data);

tx.ndraw ();
// Interrupts ();
`` `

If the data is sent in parts, then we call the beginning, then the desired number is sent and completion:

`` `CPP
TX.BeginRAW ();
TX.SendRAW (...);
TX.SendRAW (...);
TX.SendRAW (...);
tx.ndraw ();
`` `

If the data is sent for one entry, then you can use `sendrawsingle ()` without calling the start and end of sending:

`` `CPP
TX.SendRAWSINGLE (...);
`` `

> The library does not control the size and integrity of the data - you can fully realize your communication protocol without overwoman

#### Plastic bag
The library also allows you to transmit data on the universal bag of communication - the `sendpackett` method - the type of data is indicated so that it is more convenient to parse the package on the receiving device.In this case, the library controls the integrity and size of the data and does not cause the processor if they are damaged.For example, a package of type `1` - 32 bits the whole, a package of the` 2` - byte array, `3` - a string of arbitrary length, and so on.

- Type of package - number from 0 to 30
- Data size - up to 2047 bytes

`` `CPP
GW_TX <2> TX;

Struct Data {
int i;
Float F;
};

uint32_t data_32 = 123456;// whole
uint8_t data_arr [] = {1, 2, 3, 4, 5};// Massive
Char CSTR [10] = "HELLO";// char Array
String str = "Hello";// String
Data Data {1234, 3.1415};// Structure

// nointerrupts ();

// Size manually
// tx.Sendpackett (0, & Data_32, Sizeof (Data_32));
// tx.Sendpackett (1, Data_arr, Sizeof (Data_arr));
// tx.Sendpackett (2, CSTR, StREN (CSTR));
// tx.sendpackett (3, str.c_str (), str.length ());
// TX.Sendpackett (4, & Data, Sizeof (Data));

// Auto size
// tx.sendpackett (0, Data_32);
// tx.Sendpackett (1, Data_arr);
// tx.Sendpackett (4, Data);

// Interrupts ();
`` `

The type for convenience can be `enum`:

`` `CPP
enum class packet_t {
Data32,
Array,
CSTring,
String,
Struct,
};

// tx.Sendpackett (Packet_t :: Data32, Data_32);
// tx.Sendpackett (Packet_t :: Array, Data_arr);
// tx.Sendpackett (Packet_t :: Cstring, Cstr, Strlen (CSTR));
// tx.Sendpackett (packet_t :: string, str.c_str (), str.length ());
// tx.Sendpackett (Packet_t :: Struct, Data);
`` `

If you do not specify the type of package (the `sendpacket` method), then it will be equal to the type` 31` during parsing (constant `gw_auto_type`).It is convenient if the system contains only one type of package:

`` `CPP
// Size manually
// TX.Sendpacket (& Data_32, Sizeof (Data_32));
// tx.sendpacket (Data_arr, Sizeof (Data_arr));
// tx.Sendpacket (CSTR, Strlen (CSTR));
// tx.sendpacket (str.c_str (), str.length ());
// TX.Sendpacket (& Data, Sizeof (Data));

// Auto size
// tx.Sendpacket (Data_32);
// tx.Sendpacket (Data_arr);
// tx.Sendpacket (Data);
`` `

### Reception
- asynchronous reception - you need to call the `pinchange ()` method at the moment the signal changes on the pin: in the interruption by `chenge` or manually
- To obtain data, you need to connect the function-shapeOtchik
- In the main cycle of the program, you need to call a ticker `tick ()` - a package will be processed in it and a connected processor will be called

`` `CPP
// Example for Arduino Nano

Gw_rx <2> rx;// PIN 2, interruption 0

VOID setup () {
// !!!CHANGE interruption survey
Attachinterrupt (0, [] () {rx.pinchange ();}, chean);

// raw data handler
rx.onraw ([] (void* data, size_t len) {
// ...
});

// packet handler
rx.onpacket ([] (Uint8_t Type, VOID* DATA, SIZE_T LEN) {
// ...
});
}

VOID loop () {
rx.tick ();

// !!!or manually survey without interruption
// Static Bool Prev;
// If! = Gio :: Read (2)) {
// Prev ^= 1;
// rx.pinchange ();
//}
}
`` `

#### Parsing packages
If everything is clear with raw data, then at the package you need to verify the type and convert the data into the desired format.An example with the data that sent above:

`` `CPP
rx.onpacket ([] (Uint8_t Type, VOID* DATA, SIZE_T LEN) {
Serial.print ("Receved Type");
Serial.print (type);
Serial.print (":");

Switch (Packet_t (Type)) {
Case Packet_t :: Data32: {
// you can additionally check the data length, for example, here if (Len == 4)
Serial.print (*((uint32_t*) data));
} Break;

Case Packet_t :: Array: {
uint8_t* arr = (uint8_t*) Data;
for (size_t i = 0; i <len; i ++) {
Serial.print (arr [i]);
Serial.print (',');
}
} Break;

Case Packet_t :: Cstring: {
Serial.write ((uint8_t*) Data, Len);
} Break;

Case Packet_t :: String: {
Serial.write ((uint8_t*) Data, Len);
} Break;

Case Packet_t :: Struct: {
Data & P = *((Data *) Data);
Serial.print (P.I);
Serial.print (',');
Serial.print (P.F);
} Break;
}
Serial.println ();
});
`` `

### Types of communication
### Radio 433 MHz
Data can be transmitted using the simplest radio modules, in which the transmitter signal simply duplicates at the receiving receiver (for example, FS1000A and MX-RM-5V at 433 MHz).To do this, the `gw_tx_rf` class is used, it adds when sending data to the“ swing ”of the communication channel for synchronizing the transmitter and receiver, the duration of the buildup in milliseconds can be transferred to the designer.Rolling will occur before each data sending if they are sent less often than 50 ms (received experimentally).That is, if you send data more often, there will be no swing and less time will go to the program.

`` `CPP
Gw_tx_rf <3> tx (20);// rocking 20 ms
`` `

The worse the potential quality of communication and the worse the quality of the modules themselves, the longer the swing is needed.Good modules (for example, green FS1000A and MX-RM-5V) swing for 10 ms, bad (for example Synxxx)-for 100 ms.

The maximum stable speed for a pair of green modules (FS1000A and MX-RM-5V)-in the area of ​​15'000 BOD, the module itself cannot quickly.

#### ik
The library can modulate a 38 kHz signal for an infected IR-LED so that it is accepted by an IR receiver-for example, a standard arduinsky set from a remote control, receiver and LED, the LED is connected without an inversion (GND-GND, anode on a PIN MK through a resistor or transistor for amplification).To send, you need to use the class `gw_tx_ir`.

### Quality of communication
### Filtering
By default, the library uses the noise filter for the `gw_rx` receiver class - it filters random peaks when the signal changes the condition for a short time, this is found when transmitting on the radio or with poor circuitry and transmission by wires (the presence of a number of coils, ITD engines).For insignificant relief of the library, the filter can be turned off with define:

`` `CPP
#define gw_no_filter
#include <gw_rx.h>
`` `

### Hamming
Hamming Coding is turned on for the entire communication (RAW and Pack) on the transmitter and receiver - this is light and quick coding (8.4), allows you to restore the data damaged during transmission (up to 12% of the damage).The size of the transmitted data is doubled.DefAinu, requires an external library [Hamming] (https://github.com/gyverlibs/hamming):

`` `CPP
// Select one of two options:
#define gw_use_Hamming // ordinary (8.4)
// #define gw_use_hamming_mix // with mixing (more memory consumption, but more reliable)

#include <gw_rx.h>
#include <gw_tx.h>
`` `

## Examples
### Line
We send a package without type, we bring to the port the entire length:

`` `CPP
// Sending
#include <arduino.h>
#include <gw_tx.h>

VOID setup () {
}

VOID loop () {
Gw_tx <3> tx;

String S;
S += "Hello!";
static uint8_t i;
S += ++ I;

nointerrupts ();
tx.sendpacket (S.C_STR (), S.LENGTH ());
interrupts ();

DELAY (1000);
}
`` `
`` `CPP
// Reception
#include <arduino.h>
#include <gw_rx.h>

Gw_rx <2> rx;

VOID setup () {
Serial.Begin (115200);
Attachinterrupt (0, [] () {rx.pinchange ();}, chean);

rx.onpacket ([] (Uint8_t Type, VOID* DATA, SIZE_T LEN) {
Serial.write ((uint8_t*) Data, Len);
Serial.println ();
});
}

VOID loop () {
rx.tick ();
}
`` `

### structure
`` `CPP
// Sending
#include <arduino.h>
#include <gw_tx.h>

Struct Data {
int i;
Float F;
};

VOID setup () {
}

VOID loop () {
Gw_tx <3> tx;

static int i;
Data Data {I ++, 3.14};

nointerrupts ();
tx.Sendpacket (Data);
interrupts ();

DELAY (1000);
}
`` `
`` `CPP
// Reception
#include <arduino.h>
#include <gw_rx.h>

Struct Data {
int i;
Float F;
};

Gw_rx <2> rx;

VOID setup () {
Serial.Begin (115200);
Attachinterrupt (0, [] () {rx.pinchange ();}, chean);

rx.onpacket ([] (Uint8_t Type, VOID* DATA, SIZE_T LEN) {
if (sizeof (Data)! = Len) return;// Checking the correctness of the length

Data & D = *((Data *) Data);
Serial.print (D.I);
Serial.print (',');
Serial.println (D.F);

// or
// serial.println (static_cast <data*> (data)-> f);
});
}

VOID loop () {
rx.tick ();
}
`` `

<a ID = "Versions"> </a>

## versions
- V1.0

<a id = "Install"> </a>
## Installation
- The library can be found by the name ** gyverwire ** and installed through the library manager in:
- Arduino ide
- Arduino ide v2
- Platformio
- [download the library] (https://github.com/gyverlibs/gyverwire/archive/refs/heads/main.zip). Zip archive for manual installation:
- unpack and put in * C: \ Program Files (X86) \ Arduino \ Libraries * (Windows X64)
- unpack and put in * C: \ Program Files \ Arduino \ Libraries * (Windows X32)
- unpack and put in *documents/arduino/libraries/ *
- (Arduino id) Automatic installation from. Zip: * sketch/connect the library/add .Zip library ... * and specify downloaded archive
- Read more detailed instructions for installing libraries[here] (https://alexgyver.ru/arduino-first/#%D0%A3%D1%81%D1%82%D0%B0%D0%BD%D0%BE%D0%B2%D0%BA%D0%B0_%D0%B1%D0%B8%D0%B1%D0%B8%D0%BE%D1%82%D0%B5%D0%BA)
### Update
- I recommend always updating the library: errors and bugs are corrected in the new versions, as well as optimization and new features are added
- through the IDE library manager: find the library how to install and click "update"
- Manually: ** remove the folder with the old version **, and then put a new one in its place.“Replacement” cannot be done: sometimes in new versions, files that remain when replacing are deleted and can lead to errors!

<a id = "Feedback"> </a>

## bugs and feedback
Create ** Issue ** when you find the bugs, and better immediately write to the mail [alex@alexgyver.ru] (mailto: alex@alexgyver.ru)
The library is open for refinement and your ** pull Request ** 'ow!

When reporting about bugs or incorrect work of the library, it is necessary to indicate:
- The version of the library
- What is MK used
- SDK version (for ESP)
- version of Arduino ide
- whether the built -in examples work correctly, in which the functions and designs are used, leading to a bug in your code
- what code has been loaded, what work was expected from it and how it works in reality
- Ideally, attach the minimum code in which the bug is observed.Not a thousand out of a thousandCranberries, and minimum code