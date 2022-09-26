/*
 * Copyright (C) 2019 Sean Parkinson, wolfSSL Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <wolfssl_common.h>

#ifndef WC_NO_RNG

#include <library.h>
#include <utils/debug.h>

#include <wolfssl/wolfcrypt/random.h>

#include "wolfssl_rng.h"

typedef struct private_wolfssl_rng_t private_wolfssl_rng_t;

#ifndef SINGLE_THREADED
wolfSSL_Mutex globalRngMutex;
#endif
static WC_RNG globalRng;
static bool globalRngInit;

/**
 * Private data of wolfssl_rng_t
 */
struct private_wolfssl_rng_t {

	/**
	 * Public part of this class.
	 */
	wolfssl_rng_t public;

	/**
	 * Random number generator to use
	 * Either own instance or reference to global.
	 */
	WC_RNG *rng;
};

METHOD(rng_t, get_bytes, bool,
	private_wolfssl_rng_t *this, size_t bytes, uint8_t *buffer)
{
	int ret;

#ifndef SINGLE_THREADED
	if (this->rng == &globalRng)
	{
		ret = wc_LockMutex(&globalRngMutex);
		if (ret != 0)
		{
			DBG1(DBG_LIB, "locking failed, get bytes failed");
			return FALSE;
		}
	}
#endif
	ret = wc_RNG_GenerateBlock(this->rng, buffer, bytes);
#ifndef SINGLE_THREADED
	if (this->rng == &globalRng)
	{
		wc_UnLockMutex(&globalRngMutex);
	}
#endif

	return ret == 0;
}

METHOD(rng_t, allocate_bytes, bool,
	private_wolfssl_rng_t *this, size_t bytes, chunk_t *chunk)
{
	*chunk = chunk_alloc(bytes);
	if (!get_bytes(this, chunk->len, chunk->ptr))
	{
		chunk_free(chunk);
		return FALSE;
	}
	return TRUE;
}

METHOD(rng_t, destroy, void,
	private_wolfssl_rng_t *this)
{
	if (this->rng != &globalRng)
	{
		wc_FreeRng(this->rng);
		free(this->rng);
	}
	free(this);
}

/*
 * Described in header
 */
wolfssl_rng_t *wolfssl_rng_create(rng_quality_t quality)
{
	private_wolfssl_rng_t *this;

	INIT(this,
		.public = {
			.rng = {
				.get_bytes = _get_bytes,
				.allocate_bytes = _allocate_bytes,
				.destroy = _destroy,
			},
		},
		.rng = &globalRng,
	);

	if (quality > RNG_WEAK)
	{
		this->rng = malloc(sizeof(*this->rng));
		if (wc_InitRng(this->rng) != 0)
		{
			DBG1(DBG_LIB, "init RNG failed, rng create failed");
			free(this->rng);
			free(this);
			return NULL;
		}
	}
	return &this->public;
}

/*
 * Described in header
 */
int wolfssl_rng_global_init()
{
	int ret = 0;

	if (!globalRngInit)
	{
		ret = wc_InitRng(&globalRng);
		if (ret != 0)
		{
			DBG1(DBG_LIB, "init RNG failed, rng global init failed");
		}
#ifndef SINGLE_THREADED
		else if ((ret = wc_InitMutex(&globalRngMutex)) != 0)
		{
			DBG1(DBG_LIB, "init Mutex failed, rng global init failed");
		}
#endif
		else
		{
			globalRngInit = TRUE;
		}
	}
	return ret == 0;
}

/*
 * Described in header
 */
void wolfssl_rng_global_final()
{
	if (globalRngInit)
	{
#ifndef SINGLE_THREADED
		wc_FreeMutex(&globalRngMutex);
#endif
		wc_FreeRng(&globalRng);
		globalRngInit = FALSE;
	}
}

#endif /* WC_NO_RNG */
