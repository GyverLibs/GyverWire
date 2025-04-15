#pragma once
#include <stddef.h>
#include <stdint.h>

#define _GW_FRAME(baud) (1000000L / baud / 2)
#define _GW_TYPE_SIZE 5
#define _GW_TYPE_MASK ((1 << _GW_TYPE_SIZE) - 1)

namespace gwutil {

// crc8
static uint8_t crc8(const void* data, size_t len, uint8_t crc = 0) {
    const uint8_t* p = (const uint8_t*)data;
#ifdef __AVR__
    while (len--) {
        uint8_t data = *p++;
        uint8_t counter;
        uint8_t buffer;
        asm volatile(
            "EOR %[crc_out], %[data_in] \n\t"
            "LDI %[counter], 8          \n\t"
            "LDI %[buffer], 0x8C        \n\t"
            "_loop_start_%=:            \n\t"
            "LSR %[crc_out]             \n\t"
            "BRCC _loop_end_%=          \n\t"
            "EOR %[crc_out], %[buffer]  \n\t"
            "_loop_end_%=:              \n\t"
            "DEC %[counter]             \n\t"
            "BRNE _loop_start_%="
            : [crc_out] "=r"(crc), [counter] "=d"(counter), [buffer] "=d"(buffer)
            : [crc_in] "0"(crc), [data_in] "r"(data));
    }
#else
    while (len--) {
        uint8_t b = *p++, j = 8;
        while (j--) {
            crc = ((crc ^ b) & 1) ? (crc >> 1) ^ 0x8C : (crc >> 1);
            b >>= 1;
        }
    }
#endif
    return crc;
}

}  // namespace gwutil