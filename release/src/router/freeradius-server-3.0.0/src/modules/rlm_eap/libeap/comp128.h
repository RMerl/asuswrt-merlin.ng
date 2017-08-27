#ifndef _COMP128_H
#define _COMP128_H

#include <string.h>
#include <stdint.h>
#include <stdbool.h>

void comp128v1(uint8_t *sres, uint8_t *kc, const uint8_t *ki, const uint8_t *rand);
void comp128v23(uint8_t *sres, uint8_t *kc, uint8_t const *ki, uint8_t const *rand, bool v2);

#endif
