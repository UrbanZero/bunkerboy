#include <gbdk/platform.h>
#include <gb/gb.h>

#include "rng.h"

static uint16_t s = 1;

void rng_seed(uint16_t seed) { s = seed ? seed : 1; }

uint16_t rng_u16(void) {
    // xorshift16
    uint16_t x = s;
    x ^= x << 7;
    x ^= x >> 9;
    x ^= x << 8;
    s = x;
    return x;
}

uint8_t rng_u8(void) { return (uint8_t)(rng_u16() & 0xFF); }

uint8_t rng_range(uint8_t max_inclusive) {
    uint16_t r = rng_u16();
    return (uint8_t)(r % (max_inclusive + 1));
}
