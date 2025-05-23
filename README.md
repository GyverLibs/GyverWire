[![latest](https://img.shields.io/github/v/release/GyverLibs/GyverWire.svg?color=brightgreen)](https://github.com/GyverLibs/GyverWire/releases/latest/download/GyverWire.zip)
[![PIO](https://badges.registry.platformio.org/packages/gyverlibs/library/GyverWire.svg)](https://registry.platformio.org/libraries/gyverlibs/GyverWire)
[![Foo](https://img.shields.io/badge/Website-AlexGyver.ru-blue.svg?style=flat-square)](https://alexgyver.ru/)
[![Foo](https://img.shields.io/badge/%E2%82%BD%24%E2%82%AC%20%D0%9F%D0%BE%D0%B4%D0%B4%D0%B5%D1%80%D0%B6%D0%B0%D1%82%D1%8C-%D0%B0%D0%B2%D1%82%D0%BE%D1%80%D0%B0-orange.svg?style=flat-square)](https://alexgyver.ru/support_alex/)
[![Foo](https://img.shields.io/badge/README-ENGLISH-blueviolet.svg?style=flat-square)](https://github-com.translate.goog/GyverLibs/GyverWire?_x_tr_sl=ru&_x_tr_tl=en)  

[![Foo](https://img.shields.io/badge/ПОДПИСАТЬСЯ-НА%20ОБНОВЛЕНИЯ-brightgreen.svg?style=social&logo=telegram&color=blue)](https://t.me/GyverLibs)

# GyverWire
Лёгкая библиотека для передачи данных любого типа и размера по программному интерфейсу GyverWire (GW)
- Надёжный DC-сбалансированный пакетный интерфейс связи на базе Manchester Encoding с дополнительными фреймами
- Приём полностью в прерывании без опроса миллисов в loop
- Самосинхронизация, фильтрация шумов
- Отправка и приём полностью своих "сырых" данных
- Готовый инструмент для создания протоколов связи: отправка любых данных с указанием типа, библиотека проверяет целостность пакета при помощи CRC Dallas
- Опционально Extended Hamming84 кодирование с перемешиванием для восстановления повреждённых при передаче данных
- Из коробки реализована передача по проводу, радио 433 МГц и ИК-каналу
- Расчёт качества связи на приёмнике

> Библиотека легче, удобнее и надёжнее библиотек Gyver433 и GyverTransfer и призвана их заменить

### Совместимость
Совместима со всеми Arduino платформами (используются Arduino-функции)

### Зависимости
- GyverIO
- Hamming

## Содержание
- [Использование](#usage)
- [Версии](#versions)
- [Установка](#install)
- [Баги и обратная связь](#feedback)

<a id="usage"></a>

## Использование
Библиотека содержит 3 класса передатчиков и 1 общий класс приёмника:

### Передатчики
- `GW_TX <pin, baud = 5000>` - для передачи по проводу (чистый сигнал без модификаций)
- `GW_TX_RF <pin, baud = 5000>` - радиомодули 433 МГц и им подобные (встроен механизм "тренировки" канала связи)
- `GW_TX_IR <pin, baud = 5000, freq = 38000>` - ИК-канал (модуляция 38 кГц с инверсией)
  - `pin` - пин МК
  - `baud` - скорость (бит/с, бод)
  - `freq` - частота модуляции для ИК (Гц)

#### GW_TX, GW_TX_IR
```cpp
// прошло времени с конца последней отправки, мс
uint16_t lastSend();

// ======== PACKET ========

// отправить пакет авто (тип GW_AUTO_TYPE)
void sendPacket(const Td& data);

// отправить пакет авто (тип GW_AUTO_TYPE)
void sendPacket(const void* data, size_t len);

// отправить пакет с типом
void sendPacketT(Tp type, const Td& data);

// отправить пакет с типом
void sendPacketT(Tp type, const void* data, size_t len);

// ======== RAW ========

// начать отправку сырых данных
void beginRaw();

// отправить сырые данные
void sendRaw(const T& data);

// отправить сырые данные (можно вызвать несколько раз)
void sendRaw(const void* data, size_t len);

// закончить отправку сырых данных
void endRaw();

// отправить одиночные сырые данные (не нужно вызывать begin + end)
void sendRawSingle(const void* data, size_t len);
void sendRawSingle(const T& data);
```

#### GW_TX_RF
```cpp
GW_TX_RF(uint8_t trainMs = 30);

// установить время раскачки синхронизации в мс
void setTrain(uint16_t ms);
```

### Приёмник
- `GW_RX <pin, baud = 5000, bufsize = 64>` - приёмник для всех типов передачи
  - `pin` - пин МК
  - `baud` - скорость (бит/с, бод)
  - `bufsize` - размер приёмного буфера (байт) - *должен быть больше, чем самый большой потенциальный пакет данных*

```cpp
// подключить обработчик пакета вида f(uint8_t type, void* data, size_t len)
void onPacket(PacketCallback cb);

// подключить обработчик сырых данных вида f(void* data, size_t len)
void onRaw(RawCallback cb);

// вызывать при изменении сигнала на пине
void pinChange();

// вызывать в loop
void tick();

// получить качество приёма в процентах
uint8_t getRSSI();
```

### Отправка
Отправка блокирующая, на высоких скоростях передачи рекомендуется оборачивать отправку в запрет прерываний для улучшения качества связи.

#### Сырые данные
Для отправки сырых данных нужно вызвать метод начала отправки, отправить данные, вызвать завершение:

```cpp
GW_TX<2> tx;

struct Data {
  int i;
  float f;
};

uint32_t data_32 = 123456;             // целое
uint8_t data_arr[] = {1, 2, 3, 4, 5};  // массив
char cstr[10] = "hello";               // char array
String str = "hello";                  // String
Data data{1234, 3.1415};               // структура

// noInterrupts();
tx.beginRaw();

// размер вручную
// tx.sendRaw(&data_32, sizeof(data_32));
// tx.sendRaw(data_arr, sizeof(data_arr));
// tx.sendRaw(cstr, strlen(cstr));
// tx.sendRaw(str.c_str(), str.length());
// tx.sendRaw(&data, sizeof(data));

// авто размер
// tx.sendRaw(data_32);
// tx.sendRaw(data_arr);
// tx.sendRaw(data);

tx.endRaw();
// interrupts();
```

Если данные отправляются частями - то вызываем начало, затем нужное кол-во раз отправку и завершение:

```cpp
tx.beginRaw();
tx.sendRaw(...);
tx.sendRaw(...);
tx.sendRaw(...);
tx.endRaw();
```

Если данные отправляются за один заход, то можно использовать `sendRawSingle()` без вызова начала и окончания отправки:

```cpp
tx.sendRawSingle(...);
```

> Библиотека никак не контролирует размер и целостность данных - можно реализовать полностью свой протокол связи без оверхэда

#### Пакет
Библиотека также позволяет передавать данные по универсальному пакетному протоколу связи - метод `sendPacketT` - указывается тип передаваемых данных, чтобы на приёмном устройстве удобнее парсить пакет. В этом случае библиотека контролирует целостность и размер данных и не вызовет обработчик, если они повреждены. Например пакет типа `1` - 32 бит целое, пакет типа `2` - байтовый массив, `3` - строка произвольной длины, и так далее.

- Тип пакета - число от 0 до 30
- Размер данных - до 2047 Байт

```cpp
GW_TX<2> tx;

struct Data {
  int i;
  float f;
};

uint32_t data_32 = 123456;             // целое
uint8_t data_arr[] = {1, 2, 3, 4, 5};  // массив
char cstr[10] = "hello";               // char array
String str = "hello";                  // String
Data data{1234, 3.1415};               // структура

// noInterrupts();

// размер вручную
// tx.sendPacketT(0, &data_32, sizeof(data_32));
// tx.sendPacketT(1, data_arr, sizeof(data_arr));
// tx.sendPacketT(2, cstr, strlen(cstr));
// tx.sendPacketT(3, str.c_str(), str.length());
// tx.sendPacketT(4, &data, sizeof(data));

// авто размер
// tx.sendPacketT(0, data_32);
// tx.sendPacketT(1, data_arr);
// tx.sendPacketT(4, data);

// interrupts();
```

Тип для удобства может быть `enum`:

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

Если не указывать тип пакета (метод `sendPacket`), то он будет равен типу `31` при парсинге (константа `GW_AUTO_TYPE`). Удобно, если в системе присутствует только один тип пакетов:

```cpp
// размер вручную
// tx.sendPacket(&data_32, sizeof(data_32));
// tx.sendPacket(data_arr, sizeof(data_arr));
// tx.sendPacket(cstr, strlen(cstr));
// tx.sendPacket(str.c_str(), str.length());
// tx.sendPacket(&data, sizeof(data));

// авто размер
// tx.sendPacket(data_32);
// tx.sendPacket(data_arr);
// tx.sendPacket(data);
```

### Приём
- Приём асинхронный - нужно вызывать метод `pinChange()` в момент изменения сигнала на пине: в прерывании по `CHANGE` или вручную
- Для получения данных нужно подключить функцию-обработчик
- В основном цикле программы нужно вызывать тикер `tick()` - в нём будет обработан пакет и вызван подключенный обработчик

```cpp
// пример для Arduino NANO

GW_RX<2> rx;  // пин 2, прерывание 0

void setup() {
  // !!! опрос в прерывании CHANGE
  attachInterrupt(0, []() { rx.pinChange(); }, CHANGE);

  // обработчик сырых данных
  rx.onRaw([](void* data, size_t len) {
    // ...
  });

  // обработчик пакетов
  rx.onPacket([](uint8_t type, void* data, size_t len) {
    // ...
  });
}

void loop() {
  rx.tick();

  // !!! или опрос вручную без прерываний
  // static bool prev;
  // if (prev != gio::read(2)) {
  //   prev ^= 1;
  //   rx.pinChange();
  // }
}
```

#### Парсинг пакетов
Если с сырыми данными всё понятно, то у пакета при получении нужно сверить тип и преобразовать данные в нужный формат. Пример с теми данными, которые отправляли выше:

```cpp
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
```

### Типы связи
#### Радио 433 МГц
Данные можно передавать при помощи самых простых радио модулей, у которых сигнал передатчика просто дублируется на выходе приёмника (например FS1000A и MX-RM-5V на 433 МГц). Для этого используется класс `GW_TX_RF`, он добавляет при отправке данных "раскачку" канала связи для синхронизации передатчика и приёмника, в конструктор можно передать продолжительность раскачки в миллисекундах. Раскачка будет происходить перед каждой отправкой данных, если они отправляются реже, чем через 50 мс (получено экспериментально). То есть если отправлять данные чаще - раскачки не будет и на передачу будет уходить меньше времени.

```cpp
GW_TX_RF<3> tx(20);   // раскачка 20 мс
```

Чем хуже потенциальное качество связи и чем хуже по качеству сами модули, тем дольше нужна раскачка. Хорошие модули (например зелёные FS1000A и MX-RM-5V) раскачиваются за 10 мс, плохие (например SYNxxx) - за 100 мс.

Максимальная стабильная скорость для пары зелёных модулей (FS1000A и MX-RM-5V) - в районе 15'000 бод, быстрее не может сам модуль.

#### ИК
Библиотека может модулировать сигнал 38 кГц для ИК-светодиода, чтобы он принимался ИК-приёмником - например стандартный Ардуиновский набор из пульта, приёмника и светодиода, светодиод подключается без инверсии (GND-GND, анод на пин МК через резистор или транзистор для усиления). Для отправки нужно использовать класс `GW_TX_IR`.

### Качество связи
#### Фильтрация
По умолчанию библиотека использует фильтр шумов для класса приёмника `GW_RX` - он фильтрует случайные пики, когда сигнал на короткое время меняет состояние, это встречается при передаче по радио или при плохой схемотехнике и передаче по проводам (наличие рядом катушек, моторов итд). Для несущественного облегчения библиотеки фильтр можно отключить дефайном:

```cpp
#define GW_NO_FILTER
#include <GW_RX.h>
```

#### Hamming
Hamming кодирование включается для всей связи (raw и пакет) на передатчике и приёмнике - это лёгкое и быстрое кодирование (8,4), позволяет восстанавливать повреждённые при передаче данные (до 12% повреждений). Размер передаваемых данных увеличивается в два раза. Включается дефайном, требует наличия внешней библиотеки [Hamming](https://github.com/GyverLibs/Hamming):

```cpp
// выбрать один из двух вариантов:
#define GW_USE_HAMMING          // обычное (8,4)
// #define GW_USE_HAMMING_MIX   // с перемешиванием (больше расход памяти, но надёжнее)

#include <GW_RX.h>
#include <GW_TX.h>
```

> Примечание: в режиме `GW_USE_HAMMING_MIX` нельзя использовать отправку через `sendRaw` несколько раз подряд (в рамках одного begin-end). Должна быть только одна отправка, либо `sendRawSingle`

## Примеры
### Строка
Отправляем пакетом без типа, выводим в порт всю принятую длину:

```cpp
// отправка
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
```

### Структура
```cpp
// отправка
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
```

<a id="versions"></a>

## Версии
- v1.0

<a id="install"></a>
## Установка
- Библиотеку можно найти по названию **GyverWire** и установить через менеджер библиотек в:
    - Arduino IDE
    - Arduino IDE v2
    - PlatformIO
- [Скачать библиотеку](https://github.com/GyverLibs/GyverWire/archive/refs/heads/main.zip) .zip архивом для ручной установки:
    - Распаковать и положить в *C:\Program Files (x86)\Arduino\libraries* (Windows x64)
    - Распаковать и положить в *C:\Program Files\Arduino\libraries* (Windows x32)
    - Распаковать и положить в *Документы/Arduino/libraries/*
    - (Arduino IDE) автоматическая установка из .zip: *Скетч/Подключить библиотеку/Добавить .ZIP библиотеку…* и указать скачанный архив
- Читай более подробную инструкцию по установке библиотек [здесь](https://alexgyver.ru/arduino-first/#%D0%A3%D1%81%D1%82%D0%B0%D0%BD%D0%BE%D0%B2%D0%BA%D0%B0_%D0%B1%D0%B8%D0%B1%D0%BB%D0%B8%D0%BE%D1%82%D0%B5%D0%BA)
### Обновление
- Рекомендую всегда обновлять библиотеку: в новых версиях исправляются ошибки и баги, а также проводится оптимизация и добавляются новые фичи
- Через менеджер библиотек IDE: найти библиотеку как при установке и нажать "Обновить"
- Вручную: **удалить папку со старой версией**, а затем положить на её место новую. "Замену" делать нельзя: иногда в новых версиях удаляются файлы, которые останутся при замене и могут привести к ошибкам!

<a id="feedback"></a>

## Баги и обратная связь
При нахождении багов создавайте **Issue**, а лучше сразу пишите на почту [alex@alexgyver.ru](mailto:alex@alexgyver.ru)  
Библиотека открыта для доработки и ваших **Pull Request**'ов!

При сообщении о багах или некорректной работе библиотеки нужно обязательно указывать:
- Версия библиотеки
- Какой используется МК
- Версия SDK (для ESP)
- Версия Arduino IDE
- Корректно ли работают ли встроенные примеры, в которых используются функции и конструкции, приводящие к багу в вашем коде
- Какой код загружался, какая работа от него ожидалась и как он работает в реальности
- В идеале приложить минимальный код, в котором наблюдается баг. Не полотно из тысячи строк, а минимальный код