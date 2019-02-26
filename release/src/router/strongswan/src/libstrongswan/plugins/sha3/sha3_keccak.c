/*
 * Copyright (C) 2015-2016 Andreas Steffen
 * HSR Hochschule fuer Technik Rapperswil
 *
 * Based on the implementation by the Keccak, Keyak and Ketje Teams, namely,
 * Guido Bertoni, Joan Daemen, Michaël Peeters, Gilles Van Assche and
 * Ronny Van Keer, hereby denoted as "the implementer".
 *
 * To the extent possible under law, the implementer has waived all copyright
 * and related or neighboring rights to the source code in this file.
 * http://creativecommons.org/publicdomain/zero/1.0/
 */

#include <string.h>

#include "sha3_keccak.h"

typedef struct private_sha3_keccak_t private_sha3_keccak_t;

#define KECCAK_STATE_SIZE	 200	/* bytes */
#define KECCAK_MAX_RATE		 168	/* bytes */

static const uint64_t round_constants[] = {
    0x0000000000000001ULL,
    0x0000000000008082ULL,
    0x800000000000808aULL,
    0x8000000080008000ULL,
    0x000000000000808bULL,
    0x0000000080000001ULL,
    0x8000000080008081ULL,
    0x8000000000008009ULL,
    0x000000000000008aULL,
    0x0000000000000088ULL,
    0x0000000080008009ULL,
    0x000000008000000aULL,
    0x000000008000808bULL,
    0x800000000000008bULL,
    0x8000000000008089ULL,
    0x8000000000008003ULL,
    0x8000000000008002ULL,
    0x8000000000000080ULL,
    0x000000000000800aULL,
    0x800000008000000aULL,
    0x8000000080008081ULL,
    0x8000000000008080ULL,
    0x0000000080000001ULL,
    0x8000000080008008ULL
};

/**
 * Private data structure with hashing context for SHA-3
 */
struct private_sha3_keccak_t {

	/**
	 * Public interface for this hasher.
	 */
	sha3_keccak_t public;

	/**
	 * Internal state of 1600 bits as defined by FIPS-202
	 */
	uint8_t state[KECCAK_STATE_SIZE];

	/**
	 * Rate in bytes
	 */
	u_int rate;

	/**
	 * Rate input buffer
	 */
	uint8_t rate_buffer[KECCAK_MAX_RATE];

	/**
	 * Index pointing to the current position in the rate buffer
	 */
	u_int rate_index;

	/**
	 * Suffix delimiting the input message
	 */
	uint8_t delimited_suffix;

};

#if BYTE_ORDER != LITTLE_ENDIAN
/**
 * Function to load a 64-bit value using the little-endian (LE) convention.
 * On a LE platform, this could be greatly simplified using a cast.
 */
static uint64_t load64(const uint8_t *x)
{
	int i;
	uint64_t u = 0;

	for (i = 7; i >= 0; --i)
	{
		u <<= 8;
		u |= x[i];
	}
	return u;
}

/**
 * Function to store a 64-bit value using the little-endian (LE) convention.
 * On a LE platform, this could be greatly simplified using a cast.
 */
static void store64(uint8_t *x, uint64_t u)
{
	u_int i;

	for (i = 0; i < 8; ++i)
	{
		x[i] = u;
		u >>= 8;
	}
}

/**
 * Function to XOR into a 64-bit value using the little-endian (LE) convention.
 * On a LE platform, this could be greatly simplified using a cast.
 */
static void xor64(uint8_t *x, uint64_t u)
{
	u_int i;

	for (i = 0; i < 8; ++i)
	{
		x[i] ^= u;
		u >>= 8;
	}
}
#endif

/**
 * Some macros used by the Keccak-f[1600] permutation.
 */
#define ROL64(a, offset) ((((uint64_t)a) << offset) ^ (((uint64_t)a) >> (64-offset)))

#if BYTE_ORDER == LITTLE_ENDIAN
    #define readLane(i)          (((uint64_t*)state)[i])
    #define writeLane(i, lane)   (((uint64_t*)state)[i])  = (lane)
    #define XORLane(i, lane)     (((uint64_t*)state)[i]) ^= (lane)
#elif BYTE_ORDER == BIG_ENDIAN
    #define readLane(i)          load64((uint8_t*)state+sizeof(uint64_t)*i))
    #define writeLane(i, lane)   store64((uint8_t*)state+sizeof(uint64_t)*i, lane)
    #define XORLane(i, lane)     xor64((uint8_t*)state+sizeof(uint64_t)*i, lane)
#endif

/**
 * Function that computes the Keccak-f[1600] permutation on the given state.
 */
static void keccak_f1600_state_permute(void *state)
{
	int round;

	for (round = 0; round < 24; round++)
	{
		{   /* θ step (see [Keccak Reference, Section 2.3.2]) */

			uint64_t C[5], D;

			/* Compute the parity of the columns */
			C[0] = readLane(0) ^ readLane( 5) ^ readLane(10)
							   ^ readLane(15) ^ readLane(20);
			C[1] = readLane(1) ^ readLane( 6) ^ readLane(11)
							   ^ readLane(16) ^ readLane(21);
			C[2] = readLane(2) ^ readLane( 7) ^ readLane(12)
							   ^ readLane(17) ^ readLane(22);
			C[3] = readLane(3) ^ readLane( 8) ^ readLane(13)
							   ^ readLane(18) ^ readLane(23);
			C[4] = readLane(4) ^ readLane( 9) ^ readLane(14)
							   ^ readLane(19) ^ readLane(24);

			/* Compute and add the θ effect to the whole column */
			D = C[4] ^ ROL64(C[1], 1);
			XORLane( 0, D);
			XORLane( 5, D);
			XORLane(10, D);
			XORLane(15, D);
			XORLane(20, D);

			D = C[0] ^ ROL64(C[2], 1);
			XORLane( 1, D);
			XORLane( 6, D);
			XORLane(11, D);
			XORLane(16, D);
			XORLane(21, D);

			D = C[1] ^ ROL64(C[3], 1);
			XORLane( 2, D);
			XORLane( 7, D);
			XORLane(12, D);
			XORLane(17, D);
			XORLane(22, D);

			D = C[2] ^ ROL64(C[4], 1);
			XORLane( 3, D);
			XORLane( 8, D);
			XORLane(13, D);
			XORLane(18, D);
			XORLane(23, D);

			D = C[3] ^ ROL64(C[0], 1);
			XORLane( 4, D);
			XORLane( 9, D);
			XORLane(14, D);
			XORLane(19, D);
			XORLane(24, D);
		}

		{   /* ρ and π steps (see [Keccak Reference, Sections 2.3.3 and 2.3.4]) */

			uint64_t t1, t2;

			t1 = readLane( 1);

			t2 = readLane(10);
			writeLane(10, ROL64(t1,  1));

			t1 = readLane( 7);
			writeLane( 7, ROL64(t2,  3));

			t2 = readLane(11);
			writeLane(11, ROL64(t1,  6));

			t1 = readLane(17);
			writeLane(17, ROL64(t2, 10));

			t2 = readLane(18);
			writeLane(18, ROL64(t1, 15));

			t1 = readLane( 3);
			writeLane( 3, ROL64(t2, 21));

			t2 = readLane( 5);
			writeLane( 5, ROL64(t1, 28));

			t1 = readLane(16);
			writeLane(16, ROL64(t2, 36));

			t2 = readLane( 8);
			writeLane( 8, ROL64(t1, 45));

			t1 = readLane(21);
			writeLane(21, ROL64(t2, 55));

			t2 = readLane(24);
			writeLane(24, ROL64(t1,  2));

			t1 = readLane( 4);
			writeLane( 4, ROL64(t2, 14));

			t2 = readLane(15);
			writeLane(15, ROL64(t1, 27));

			t1 = readLane(23);
			writeLane(23, ROL64(t2, 41));

			t2 = readLane(19);
			writeLane(19, ROL64(t1, 56));

			t1 = readLane(13);
			writeLane(13, ROL64(t2,  8));

			t2 = readLane(12);
			writeLane(12, ROL64(t1, 25));

			t1 = readLane( 2);
			writeLane( 2, ROL64(t2, 43));

			t2 = readLane(20);
			writeLane(20, ROL64(t1, 62));

			t1 = readLane(14);
			writeLane(14, ROL64(t2, 18));

			t2 = readLane(22);
			writeLane(22, ROL64(t1, 39));

			t1 = readLane( 9);
			writeLane( 9, ROL64(t2, 61));

			t2 = readLane( 6);
			writeLane( 6, ROL64(t1, 20));

			writeLane( 1, ROL64(t2, 44));
		}

		{   /* χ step (see [Keccak Reference, Section 2.3.1]) */

			uint64_t t[5];

			t[0] = readLane(0);
			t[1] = readLane(1);
			t[2] = readLane(2);
			t[3] = readLane(3);
			t[4] = readLane(4);

			writeLane(0, t[0] ^ ((~t[1]) & t[2]));
			writeLane(1, t[1] ^ ((~t[2]) & t[3]));
			writeLane(2, t[2] ^ ((~t[3]) & t[4]));
			writeLane(3, t[3] ^ ((~t[4]) & t[0]));
			writeLane(4, t[4] ^ ((~t[0]) & t[1]));

			t[0] = readLane(5);
			t[1] = readLane(6);
			t[2] = readLane(7);
			t[3] = readLane(8);
			t[4] = readLane(9);

			writeLane(5, t[0] ^ ((~t[1]) & t[2]));
			writeLane(6, t[1] ^ ((~t[2]) & t[3]));
			writeLane(7, t[2] ^ ((~t[3]) & t[4]));
			writeLane(8, t[3] ^ ((~t[4]) & t[0]));
			writeLane(9, t[4] ^ ((~t[0]) & t[1]));

			t[0] = readLane(10);
			t[1] = readLane(11);
			t[2] = readLane(12);
			t[3] = readLane(13);
			t[4] = readLane(14);

			writeLane(10, t[0] ^ ((~t[1]) & t[2]));
			writeLane(11, t[1] ^ ((~t[2]) & t[3]));
			writeLane(12, t[2] ^ ((~t[3]) & t[4]));
			writeLane(13, t[3] ^ ((~t[4]) & t[0]));
			writeLane(14, t[4] ^ ((~t[0]) & t[1]));

			t[0] = readLane(15);
			t[1] = readLane(16);
			t[2] = readLane(17);
			t[3] = readLane(18);
			t[4] = readLane(19);

			writeLane(15, t[0] ^ ((~t[1]) & t[2]));
			writeLane(16, t[1] ^ ((~t[2]) & t[3]));
			writeLane(17, t[2] ^ ((~t[3]) & t[4]));
			writeLane(18, t[3] ^ ((~t[4]) & t[0]));
			writeLane(19, t[4] ^ ((~t[0]) & t[1]));

			t[0] = readLane(20);
			t[1] = readLane(21);
			t[2] = readLane(22);
			t[3] = readLane(23);
			t[4] = readLane(24);

			writeLane(20, t[0] ^ ((~t[1]) & t[2]));
			writeLane(21, t[1] ^ ((~t[2]) & t[3]));
			writeLane(22, t[2] ^ ((~t[3]) & t[4]));
			writeLane(23, t[3] ^ ((~t[4]) & t[0]));
			writeLane(24, t[4] ^ ((~t[0]) & t[1]));
		}

		{   /* ι step (see [Keccak Reference, Section 2.3.5]) */

			XORLane(0, round_constants[round]);
		}
	}
}

METHOD(sha3_keccak_t, get_rate, u_int,
	private_sha3_keccak_t *this)
{
	return this->rate;
}

METHOD(sha3_keccak_t, reset, void,
	private_sha3_keccak_t *this)
{
    memset(this->state, 0x00, KECCAK_STATE_SIZE);
	this->rate_index = 0;
}


METHOD(sha3_keccak_t, absorb, void,
	private_sha3_keccak_t *this, chunk_t data)
{
	uint64_t *buffer_lanes, *state_lanes;
	size_t len, rate_lanes;
	int i;

	buffer_lanes = (uint64_t*)this->rate_buffer;
	state_lanes  = (uint64_t*)this->state;
	rate_lanes = this->rate / sizeof(uint64_t);

	while (data.len)
	{
		len = min(data.len, this->rate - this->rate_index);
		memcpy(this->rate_buffer + this->rate_index, data.ptr, len);
		this->rate_index += len;
		data.ptr += len;
		data.len -= len;

		if (this->rate_index == this->rate)
		{
			for (i = 0; i < rate_lanes; i++)
			{
				state_lanes[i] ^= buffer_lanes[i];
			}
			this->rate_index = 0;

			keccak_f1600_state_permute(this->state);
		}
	}
}

METHOD(sha3_keccak_t, finalize, void,
	private_sha3_keccak_t *this)
{
	uint64_t *buffer_lanes, *state_lanes;
	size_t rate_lanes, remainder;
	int i;

	/* Add the delimitedSuffix as the first bit of padding */
	this->rate_buffer[this->rate_index++] = this->delimited_suffix;

	buffer_lanes = (uint64_t*)this->rate_buffer;
	state_lanes  = (uint64_t*)this->state;
	rate_lanes = this->rate_index / sizeof(uint64_t);

	remainder = this->rate_index - rate_lanes * sizeof(uint64_t);
	if (remainder)
	{
		memset(this->rate_buffer + this->rate_index, 0x00,
			   sizeof(uint64_t) - remainder);
		rate_lanes++;
	}
	for (i = 0; i < rate_lanes; i++)
	{
		state_lanes[i] ^= buffer_lanes[i];
	}

	/* Add the second bit of padding */
	this->state[this->rate - 1] ^= 0x80;

	/* Switch to the squeezing phase */
	keccak_f1600_state_permute(this->state);
	this->rate_index = 0;
}

METHOD(sha3_keccak_t, squeeze, void,
	private_sha3_keccak_t *this, size_t out_len, uint8_t *out)
{
	size_t index = 0, len;

	while (index < out_len)
	{
		if (this->rate_index == this->rate)
		{
			keccak_f1600_state_permute(this->state);
			this->rate_index = 0;
		}
		len = min(out_len - index, this->rate - this->rate_index);
		memcpy(out, &this->state[this->rate_index], len);
		out += len;
		index += len;
		this->rate_index += len;
	}
}

METHOD(sha3_keccak_t, destroy, void,
	private_sha3_keccak_t *this)
{
	free(this);
}

/*
 * Described in header.
 */
sha3_keccak_t *sha3_keccak_create(u_int capacity, uint8_t delimited_suffix)
{
	private_sha3_keccak_t *this;
	int rate;

	rate = KECCAK_STATE_SIZE - capacity;

	if (rate <= 0 || rate > KECCAK_MAX_RATE)
	{
		return NULL;
	}

	INIT(this,
		.public = {
			.get_rate = _get_rate,
			.reset = _reset,
			.absorb = _absorb,
			.finalize = _finalize,
			.squeeze = _squeeze,
			.destroy = _destroy,
		},
		.rate = rate,
		.delimited_suffix = delimited_suffix,
	);

	return &this->public;
}
