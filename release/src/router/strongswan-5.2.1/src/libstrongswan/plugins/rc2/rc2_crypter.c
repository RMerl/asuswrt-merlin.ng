/*
 * Copyright (C) 2013 Tobias Brunner
 * Hochschule fuer Technik Rapperswil
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

#include "rc2_crypter.h"

typedef struct private_rc2_crypter_t private_rc2_crypter_t;

#define RC2_BLOCK_SIZE 8

#define ROL16(x, k)	({ u_int16_t _x = (x); (_x << (k)) | (_x >> (16 - (k))); })
#define ROR16(x, k)	({ u_int16_t _x = (x); (_x >> (k)) | (_x << (16 - (k))); })

#define GET16(x)	({ u_char *_x = (x); (u_int16_t)_x[0] | ((u_int16_t)_x[1] << 8); })
#define PUT16(x, v)	({ u_char *_x = (x); u_int16_t _v = (v); _x[0] = _v, _x[1] = _v >> 8; })

/**
 * Private data of rc2_crypter_t
 */
struct private_rc2_crypter_t {

	/**
	 * Public interface
	 */
	rc2_crypter_t public;

	/**
	* The expanded key in 16-bit words
	*/
	u_int16_t  K[64];

	/**
	* Key size in bytes
	*/
	size_t T;

	/**
	* Effective key size in bits
	*/
	size_t T1;
};

/**
 * PITABLE
 */
static const u_char PITABLE[256] =
{
	0xd9, 0x78, 0xf9, 0xc4, 0x19, 0xdd, 0xb5, 0xed,
	0x28, 0xe9, 0xfd, 0x79, 0x4a, 0xa0, 0xd8, 0x9d,
	0xc6, 0x7e, 0x37, 0x83, 0x2b, 0x76, 0x53, 0x8e,
	0x62, 0x4c, 0x64, 0x88, 0x44, 0x8b, 0xfb, 0xa2,
	0x17, 0x9a, 0x59, 0xf5, 0x87, 0xb3, 0x4f, 0x13,
	0x61, 0x45, 0x6d, 0x8d, 0x09, 0x81, 0x7d, 0x32,
	0xbd, 0x8f, 0x40, 0xeb, 0x86, 0xb7, 0x7b, 0x0b,
	0xf0, 0x95, 0x21, 0x22, 0x5c, 0x6b, 0x4e, 0x82,
	0x54, 0xd6, 0x65, 0x93, 0xce, 0x60, 0xb2, 0x1c,
	0x73, 0x56, 0xc0, 0x14, 0xa7, 0x8c, 0xf1, 0xdc,
	0x12, 0x75, 0xca, 0x1f, 0x3b, 0xbe, 0xe4, 0xd1,
	0x42, 0x3d, 0xd4, 0x30, 0xa3, 0x3c, 0xb6, 0x26,
	0x6f, 0xbf, 0x0e, 0xda, 0x46, 0x69, 0x07, 0x57,
	0x27, 0xf2, 0x1d, 0x9b, 0xbc, 0x94, 0x43, 0x03,
	0xf8, 0x11, 0xc7, 0xf6, 0x90, 0xef, 0x3e, 0xe7,
	0x06, 0xc3, 0xd5, 0x2f, 0xc8, 0x66, 0x1e, 0xd7,
	0x08, 0xe8, 0xea, 0xde, 0x80, 0x52, 0xee, 0xf7,
	0x84, 0xaa, 0x72, 0xac, 0x35, 0x4d, 0x6a, 0x2a,
	0x96, 0x1a, 0xd2, 0x71, 0x5a, 0x15, 0x49, 0x74,
	0x4b, 0x9f, 0xd0, 0x5e, 0x04, 0x18, 0xa4, 0xec,
	0xc2, 0xe0, 0x41, 0x6e, 0x0f, 0x51, 0xcb, 0xcc,
	0x24, 0x91, 0xaf, 0x50, 0xa1, 0xf4, 0x70, 0x39,
	0x99, 0x7c, 0x3a, 0x85, 0x23, 0xb8, 0xb4, 0x7a,
	0xfc, 0x02, 0x36, 0x5b, 0x25, 0x55, 0x97, 0x31,
	0x2d, 0x5d, 0xfa, 0x98, 0xe3, 0x8a, 0x92, 0xae,
	0x05, 0xdf, 0x29, 0x10, 0x67, 0x6c, 0xba, 0xc9,
	0xd3, 0x00, 0xe6, 0xcf, 0xe1, 0x9e, 0xa8, 0x2c,
	0x63, 0x16, 0x01, 0x3f, 0x58, 0xe2, 0x89, 0xa9,
	0x0d, 0x38, 0x34, 0x1b, 0xab, 0x33, 0xff, 0xb0,
	0xbb, 0x48, 0x0c, 0x5f, 0xb9, 0xb1, 0xcd, 0x2e,
	0xc5, 0xf3, 0xdb, 0x47, 0xe5, 0xa5, 0x9c, 0x77,
	0x0a, 0xa6, 0x20, 0x68, 0xfe, 0x7f, 0xc1, 0xad,
};

/**
 * Encrypt a single block of data
 */
static void encrypt_block(private_rc2_crypter_t *this, u_char R[])
{
	register u_int16_t R0, R1, R2, R3, *Kj;
	int rounds = 3, mix = 5;

	R0 = GET16(R);
	R1 = GET16(R + 2);
	R2 = GET16(R + 4);
	R3 = GET16(R + 6);
	Kj = &this->K[0];

	/* 5 mix, mash, 6 mix, mash, 5 mix */
	while (TRUE)
	{
		/* mix */
		R0 = ROL16(R0 + *(Kj++) + (R3 & R2) + (~R3 & R1), 1);
		R1 = ROL16(R1 + *(Kj++) + (R0 & R3) + (~R0 & R2), 2);
		R2 = ROL16(R2 + *(Kj++) + (R1 & R0) + (~R1 & R3), 3);
		R3 = ROL16(R3 + *(Kj++) + (R2 & R1) + (~R2 & R0), 5);

		if (--mix == 0)
		{
			if (--rounds == 0)
			{
				break;
			}
			mix = (rounds == 2) ? 6 : 5;
			/* mash */
			R0 += this->K[R3 & 63];
			R1 += this->K[R0 & 63];
			R2 += this->K[R1 & 63];
			R3 += this->K[R2 & 63];
		}
	}

	PUT16(R, R0);
	PUT16(R + 2, R1);
	PUT16(R + 4, R2);
	PUT16(R + 6, R3);
}

/**
 * Decrypt a single block of data.
 */
static void decrypt_block(private_rc2_crypter_t *this, u_char R[])
{
	register u_int16_t R0, R1, R2, R3, *Kj;
	int rounds = 3, mix = 5;

	R0 = GET16(R);
	R1 = GET16(R + 2);
	R2 = GET16(R + 4);
	R3 = GET16(R + 6);
	Kj = &this->K[63];

	/* 5 r-mix, r-mash, 6 r-mix, r-mash, 5 r-mix */
	while (TRUE)
	{
		/* r-mix */
		R3 = ROR16(R3, 5);
		R3 = R3 - *(Kj--) - (R2 & R1) - (~R2 & R0);
		R2 = ROR16(R2, 3);
		R2 = R2 - *(Kj--) - (R1 & R0) - (~R1 & R3);
		R1 = ROR16(R1, 2);
		R1 = R1 - *(Kj--) - (R0 & R3) - (~R0 & R2);
		R0 = ROR16(R0, 1);
		R0 = R0 - *(Kj--) - (R3 & R2) - (~R3 & R1);

		if (--mix == 0)
		{
			if (--rounds == 0)
			{
				break;
			}
			mix = (rounds == 2) ? 6 : 5;
			/* r-mash */
			R3 -= this->K[R2 & 63];
			R2 -= this->K[R1 & 63];
			R1 -= this->K[R0 & 63];
			R0 -= this->K[R3 & 63];
		}
	}

	PUT16(R, R0);
	PUT16(R + 2, R1);
	PUT16(R + 4, R2);
	PUT16(R + 6, R3);
}

METHOD(crypter_t, decrypt, bool,
	private_rc2_crypter_t *this, chunk_t data, chunk_t iv, chunk_t *decrypted)
{
	u_int8_t *in, *out, *prev;

	if (data.len % RC2_BLOCK_SIZE || iv.len != RC2_BLOCK_SIZE)
	{
		return FALSE;
	}

	in = data.ptr + data.len - RC2_BLOCK_SIZE;
	out = data.ptr;
	if (decrypted)
	{
		*decrypted = chunk_alloc(data.len);
		out = decrypted->ptr;
	}
	out += data.len - RC2_BLOCK_SIZE;

	prev = in;
	for (; in >= data.ptr; in -= RC2_BLOCK_SIZE, out -= RC2_BLOCK_SIZE)
	{
		if (decrypted)
		{
			memcpy(out, in, RC2_BLOCK_SIZE);
		}
		decrypt_block(this, out);
		prev -= RC2_BLOCK_SIZE;
		if (prev < data.ptr)
		{
			prev = iv.ptr;
		}
		memxor(out, prev, RC2_BLOCK_SIZE);
	}
	return TRUE;
}

METHOD(crypter_t, encrypt, bool,
	private_rc2_crypter_t *this, chunk_t data, chunk_t iv, chunk_t *encrypted)
{
	u_int8_t *in, *out, *end, *prev;

	if (data.len % RC2_BLOCK_SIZE || iv.len != RC2_BLOCK_SIZE)
	{
		return FALSE;
	}

	in = data.ptr;
	end = data.ptr + data.len;
	out = data.ptr;
	if (encrypted)
	{
		*encrypted = chunk_alloc(data.len);
		out = encrypted->ptr;
	}

	prev = iv.ptr;
	for (; in < end; in += RC2_BLOCK_SIZE, out += RC2_BLOCK_SIZE)
	{
		if (encrypted)
		{
			memcpy(out, in, RC2_BLOCK_SIZE);
		}
		memxor(out, prev, RC2_BLOCK_SIZE);
		encrypt_block(this, out);
		prev = out;
	}
	return TRUE;
}

METHOD(crypter_t, get_block_size, size_t,
	private_rc2_crypter_t *this)
{
	return RC2_BLOCK_SIZE;
}

METHOD(crypter_t, get_iv_size, size_t,
	private_rc2_crypter_t *this)
{
	return RC2_BLOCK_SIZE;
}

METHOD(crypter_t, get_key_size, size_t,
	private_rc2_crypter_t *this)
{
	return this->T;
}

METHOD(crypter_t, set_key, bool,
	private_rc2_crypter_t *this, chunk_t key)
{
	u_int8_t L[128], T8, TM, idx;
	int i;

	if (key.len != this->T)
	{
		return FALSE;
	}
	for (i = 0; i < key.len; i++)
	{
		L[i] = key.ptr[i];
	}
	for (; i < 128; i++)
	{
		idx = L[i-1] + L[i-key.len];
		L[i] = PITABLE[idx];
	}
	T8 = (this->T1 + 7) / 8;
	TM = ~(0xff << (8 - (8*T8 - this->T1)));
	L[128-T8] = PITABLE[L[128-T8] & TM];
	for (i = 127-T8; i >= 0; i--)
	{
		idx = L[i+1] ^ L[i+T8];
		L[i] = PITABLE[idx];
	}
	for (i = 0; i < 64; i++)
	{
		this->K[i] = GET16(&L[i << 1]);
	}
	memwipe(L, sizeof(L));
	return TRUE;
}

METHOD(crypter_t, destroy, void,
	private_rc2_crypter_t *this)
{
	memwipe(this->K, sizeof(this->K));
	free(this);
}

/*
 * Described in header
 */
rc2_crypter_t *rc2_crypter_create(encryption_algorithm_t algo, size_t key_size)
{
	private_rc2_crypter_t *this;
	size_t effective;

	if (algo != ENCR_RC2_CBC)
	{
		return NULL;
	}
	key_size = max(1, key_size);
	effective = RC2_EFFECTIVE_KEY_LEN(key_size);
	key_size = min(128, RC2_KEY_LEN(key_size));
	effective = max(1, min(1024, effective ?: key_size * 8));

	INIT(this,
		.public = {
			.crypter = {
				.encrypt = _encrypt,
				.decrypt = _decrypt,
				.get_block_size = _get_block_size,
				.get_iv_size = _get_iv_size,
				.get_key_size = _get_key_size,
				.set_key = _set_key,
				.destroy = _destroy,
			},
		},
		.T = key_size,
		.T1 = effective,
	);

	return &this->public;
}
