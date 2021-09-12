#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../qrspec.h"
#include "rsecc_decoder.h"

#define SYMBOL_SIZE (8)
#define symbols ((1 << SYMBOL_SIZE) - 1)
static const int proot = 0x11d; /* stands for x^8+x^4+x^3+x^2+1 (see pp.37 of JIS X0510:2004) */

/* min/max codeword length of ECC, calculated from the specification. */
#define min_length (2)
#define max_length (30)
#define max_generatorSize (max_length)

static unsigned char alpha[symbols + 1];
static unsigned char aindex[symbols + 1];

void RSECC_decoder_init() {
	int i, b;

	alpha[symbols] = 0;
	aindex[0] = symbols;

	b = 1;
	for(i = 0; i < symbols; i++) {
		alpha[i] = b;
		aindex[b] = i;
		b <<= 1;
		if(b & (symbols + 1)) {
			b ^= proot;
		}
		b &= symbols;
	}
}

int RSECC_decoder_checkSyndrome(int dl, unsigned char *data, int el, unsigned char *ecc)
{
	int i, j;
	int s;

	for(i=0; i<el; i++) {
		s = data[0];
		for(j=1; j<dl; j++) {
			if(s == 0) {
				s = data[j];
			} else {
				s = data[j] ^ alpha[(aindex[s] + i) % symbols];
			}
		}
		for(j=0; j<el; j++) {
			if(s == 0) {
				s = ecc[j];
			} else {
				s = ecc[j] ^ alpha[(aindex[s] + i) % symbols];
			}
		}
		if(s != 0) {
			return -1;
		}
	}

	return 0;
}
