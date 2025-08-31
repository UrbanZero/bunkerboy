#ifndef RNG_H
#define RNG_H
#include <stdint.h>
void rng_seed(uint16_t s);
uint16_t rng_u16(void);
uint8_t  rng_u8(void);
uint8_t  rng_range(uint8_t max_inclusive);
#endif
