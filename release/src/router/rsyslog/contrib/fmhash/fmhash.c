/*
 * Copyright (c) 2018, Harshvardhan Shrivastava
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * * Neither the name of the original author; nor the names of any contributors
 * may be used to endorse or promote products derived from this software
 * without specific prior written permission.
 *
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include <stdint.h>
#include <stddef.h>
#include <typedefs.h>
#include <sys/types.h>
#include <string.h>
#ifdef USE_HASH_XXHASH
#  include <xxhash.h>
#endif

#include "rsyslog.h"
#include "parserif.h"
#include "module-template.h"
#include "rainerscript.h"



MODULE_TYPE_FUNCTION
MODULE_TYPE_NOKEEP
DEF_FMOD_STATIC_DATA

typedef uint64_t hash_t;
typedef uint32_t seed_t;
typedef struct hash_context_s hash_context_t;

typedef hash_t (*hash_impl)(const void*, size_t, seed_t);

typedef rsRetVal (*hash_wrapper_2)(struct svar *__restrict__ const
		, struct svar *__restrict__ const, hash_context_t*);
typedef rsRetVal (*hash_wrapper_3)(struct svar *__restrict__ const, struct svar *__restrict__ const
		, struct svar *__restrict__ const, hash_context_t*);

struct hash_context_s {
	hash_impl hashXX;
	hash_wrapper_2 hash_wrapper_1_2;
	hash_wrapper_3 hash_wrapper_2_3;
	hash_t xhash;
};

/*
 * Fowler–Noll–Vo hash 32 bit
 * http://www.isthe.com/chongo/src/fnv/hash_32.c
 */
#if defined(__clang__)
#pragma GCC diagnostic ignored "-Wunknown-attributes"
#endif
static hash_t
#if defined(__clang__)
__attribute__((no_sanitize("unsigned-integer-overflow")))
#endif
fnv_32(const void* input, size_t len, seed_t seed) {
	unsigned char *bp = (unsigned char *)input;	/* start of buffer */

	/*
	 * FNV-1 hash each octet in the buffer
	 */
	size_t i;
	for (i = 0; i < len; i++) {
		/* multiply by the 32 bit FNV magic prime mod 2^32 */
		seed += (seed<<1) + (seed<<4) + (seed<<7) + (seed<<8) + (seed<<24);

		/* xor the bottom with the current octet */
		seed ^= (seed_t)*bp++;
	}

	/* return our new hash value */
	return seed;
}


/*
 * Modified Bernstein
 * http://www.eternallyconfuzzled.com/tuts/algorithms/jsw_tut_hashing.aspx
 */
#if defined(__clang__)
#pragma GCC diagnostic ignored "-Wunknown-attributes"
#endif
static hash_t
#if defined(__clang__)
__attribute__((no_sanitize("unsigned-integer-overflow")))
#endif
djb_hash(const void* input, size_t len, seed_t seed) {
	const char *p = input;
	hash_t hash = 5381;
	size_t i;
	for (i = 0; i < len; i++) {
		hash = 33 * hash ^ p[i];
	}

	return hash + seed;
}

/*Get 32 bit hash for input*/
static hash_t
hash32(const void* input, size_t len, seed_t seed) {
	hash_t xhash = 0;
#ifdef USE_HASH_XXHASH
	xhash = XXH32(input, len, seed);
#else
	xhash = fnv_32(input, len, seed);
#endif
	return xhash;
}

/*Get 64 bit hash for input*/
static hash_t
hash64(const void* input, size_t len, seed_t seed) {
	hash_t xhash = 0;
#ifdef USE_HASH_XXHASH
	xhash = XXH64(input, len, seed);
#else
	xhash = djb_hash(input, len, seed);
#endif
	return xhash;
}

static rsRetVal
hash_wrapper2(struct svar *__restrict__ const sourceVal
		, struct svar *__restrict__ const seedVal, hash_context_t* hcontext) {
	DEFiRet;
	int freeHashStr = 0, success = 0;
	char *hashStr = NULL;
	seed_t seed = 0;
	if(seedVal) {
		seed = var2Number(seedVal, &success);
		if (!success) {
			parser_warnmsg("fmhash: hashXX(string, seed) didn't get a valid 'seed' limit"
					", defaulting hash value to 0");
			iRet = RS_RET_ERR;
			FINALIZE;
		}
	}

	hashStr = (char*)var2CString(sourceVal, &freeHashStr);
	size_t len = strlen(hashStr);
	hcontext->xhash = hcontext->hashXX(hashStr, len, seed);
	DBGPRINTF("fmhash: hashXX generated hash %" PRIu64 " for string(%.*s)"
			, hcontext->xhash, (int)len, hashStr);
finalize_it:
	if (freeHashStr) {
		free(hashStr);
	}
	RETiRet;
}

static rsRetVal
hash_wrapper3(struct svar *__restrict__ const sourceVal, struct svar *__restrict__ const modVal
		, struct svar *__restrict__ const seedVal, hash_context_t* hcontext) {

	DEFiRet;
	int success = 0;
	hash_t xhash = 0;
	hash_t mod = var2Number(modVal, &success);
	if (! success) {
		parser_warnmsg("fmhash: hashXXmod(string, mod)/hash64mod(string, mod, seed) didn't"
				" get a valid 'mod' limit, defaulting hash value to 0");
		iRet = RS_RET_ERR;
		FINALIZE;
	}
	if(mod == 0) {
		parser_warnmsg("fmhash: hashXXmod(string, mod)/hash64mod(string, mod, seed) invalid"
				", 'mod' is zero, , defaulting hash value to 0");
		iRet = RS_RET_ERR;
		FINALIZE;
	}

	CHKiRet((hcontext->hash_wrapper_1_2(sourceVal, seedVal, hcontext)));
	xhash = hcontext->xhash % mod;
	DBGPRINTF("fmhash: hashXXmod generated hash-mod %" PRIu64 ".", xhash);
	hcontext->xhash = xhash;
finalize_it:
	RETiRet;
}

static void
init_hash32_context(hash_context_t* hash32_context) {
	hash32_context->hashXX = hash32;
	hash32_context->hash_wrapper_1_2 = hash_wrapper2;
	hash32_context->hash_wrapper_2_3 = hash_wrapper3;
	hash32_context->xhash = 0;
};

static void
init_hash64_context(hash_context_t* hash64_context) {
	hash64_context->hashXX = hash64;
	hash64_context->hash_wrapper_1_2 = hash_wrapper2;
	hash64_context->hash_wrapper_2_3 = hash_wrapper3;
	hash64_context->xhash = 0;
};

static void ATTR_NONNULL()
fmHashXX(struct cnffunc *__restrict__ const func, struct svar *__restrict__ const ret,
		void *__restrict__ const usrptr, wti_t *__restrict__ const pWti) {
	DEFiRet;
	struct svar hashStrVal;
	struct svar seedVal;
	hash_context_t* hcontext = NULL;
	cnfexprEval(func->expr[0], &hashStrVal, usrptr, pWti);
	if(func->nParams == 2) cnfexprEval(func->expr[1], &seedVal, usrptr, pWti);
	ret->d.n = 0;
	ret->datatype = 'N';
	hcontext = (hash_context_t*) func->funcdata;
	CHKiRet((hcontext->hash_wrapper_1_2(&hashStrVal
			, (func->nParams == 2 ? &seedVal : NULL)
			, hcontext)));
	ret->d.n = hcontext->xhash;
finalize_it:
	varFreeMembers(&hashStrVal);
	if(func->nParams == 2) varFreeMembers(&seedVal);
}

static void ATTR_NONNULL()
fmHashXXmod(struct cnffunc *__restrict__ const func, struct svar *__restrict__ const ret,
		void *__restrict__ const usrptr, wti_t *__restrict__ const pWti) {

	DEFiRet;
	struct svar hashStrVal;
	struct svar modVal;
	struct svar seedVal;
	hash_context_t* hcontext = NULL;
	cnfexprEval(func->expr[0], &hashStrVal, usrptr, pWti);
	cnfexprEval(func->expr[1], &modVal, usrptr, pWti);
	if(func->nParams == 3) cnfexprEval(func->expr[2], &seedVal, usrptr, pWti);
	ret->d.n = 0;
	ret->datatype = 'N';
	hcontext = (hash_context_t*) func->funcdata;
	CHKiRet((hcontext->hash_wrapper_2_3(&hashStrVal
			, &modVal, func->nParams > 2 ? &seedVal : NULL
			, hcontext)));
	ret->d.n = hcontext->xhash;
finalize_it:
	varFreeMembers(&hashStrVal);
	varFreeMembers(&modVal);
	if(func->nParams == 3) varFreeMembers(&seedVal);
}

static rsRetVal ATTR_NONNULL(1)
init_fmHash64(struct cnffunc *const func)
{
	DEFiRet;
	hash_context_t *hash_context = NULL;
	if(func->nParams < 1) {
		parser_errmsg("fmhash: hash64(string) / hash64(string, seed)"
				" insufficient params.\n");
		iRet = RS_RET_ERR;
		FINALIZE;
	}
	func->destructable_funcdata = 1;
	CHKmalloc(hash_context = calloc(1, sizeof(hash_context_t)));
	init_hash64_context(hash_context);
	func->funcdata = (void*)hash_context;

finalize_it:
	RETiRet;
}

static rsRetVal ATTR_NONNULL(1)
init_fmHash64mod(struct cnffunc *const func)
{
	DEFiRet;
	hash_context_t *hash_context = NULL;
	if(func->nParams < 2) {
		parser_errmsg("fmhash: hash64mod(string, mod)/hash64mod(string, mod, seed)"
				" insufficient params.\n");
		iRet = RS_RET_ERR;
		FINALIZE;
	}
	func->destructable_funcdata = 1;
	CHKmalloc(hash_context = calloc(1, sizeof(hash_context_t)));
	init_hash64_context(hash_context);
	func->funcdata = (void*)hash_context;
finalize_it:
	RETiRet;
}

static rsRetVal ATTR_NONNULL(1)
init_fmHash32(struct cnffunc *const func)
{
	DEFiRet;
	hash_context_t *hash_context = NULL;
	if(func->nParams < 1) {
		parser_errmsg("fmhash: hash32(string) / hash32(string, seed)"
				" insufficient params.\n");
		iRet = RS_RET_ERR;
		FINALIZE;
	}
	func->destructable_funcdata = 1;
	CHKmalloc(hash_context = calloc(1, sizeof(hash_context_t)));
	init_hash32_context(hash_context);
	func->funcdata = (void*)hash_context;

finalize_it:
	RETiRet;
}

static rsRetVal ATTR_NONNULL(1)
init_fmHash32mod(struct cnffunc *const func)
{
	DEFiRet;
	hash_context_t *hash_context = NULL;
	if(func->nParams < 2) {
		parser_errmsg("fmhash: hash32mod(string, mod)/hash32mod(string, mod, seed)"
				" insufficient params.\n");
		iRet = RS_RET_ERR;
		FINALIZE;
	}
	func->destructable_funcdata = 1;
	CHKmalloc(hash_context = calloc(1, sizeof(hash_context_t)));
	init_hash32_context(hash_context);
	func->funcdata = (void*)hash_context;
finalize_it:
	RETiRet;
}


static struct scriptFunct functions[] = {
		{"hash64", 1, 2, fmHashXX, init_fmHash64, NULL},
		{"hash64mod", 2, 3, fmHashXXmod, init_fmHash64mod, NULL},
		{"hash32", 1, 2, fmHashXX, init_fmHash32, NULL},
		{"hash32mod", 2, 3, fmHashXXmod, init_fmHash32mod, NULL},
		{NULL, 0, 0, NULL, NULL, NULL} //last element to check end of array
};


BEGINgetFunctArray
CODESTARTgetFunctArray
	dbgprintf("Hash: fmhhash\n");
	*version = 1;
	*functArray = functions;
ENDgetFunctArray


BEGINmodExit
CODESTARTmodExit
ENDmodExit


BEGINqueryEtryPt
CODESTARTqueryEtryPt
CODEqueryEtryPt_STD_FMOD_QUERIES
ENDqueryEtryPt


BEGINmodInit()
CODESTARTmodInit
	*ipIFVersProvided = CURR_MOD_IF_VERSION; /* we only support the current interface specification */
CODEmodInit_QueryRegCFSLineHdlr
	dbgprintf("rsyslog fmhash init called, compiled with version %s\n", VERSION);
ENDmodInit
