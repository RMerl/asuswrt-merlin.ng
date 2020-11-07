/* bcm_bn.c
 *
 * Copyright (C) 2020, Broadcom. All Rights Reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * $Id: $
 *
 *   <<Broadcom-WL-IPTag/Open:>>
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <bcm_bn.h>

#define LV(b)	(uint32 *)(b+1)

/* layout of the BN.
 *     1.  BN_CTX address  --  only the address appears in the struct definition
 *     2.  length -- number of data array uint32 integers
 *     3.  data array
 */

struct bn_ctx {
	bn_alloc_fn_t  bn_alloc;
	bn_free_fn_t   bn_free;
	void *ctx;
	int max_bit_len; // not used
};

/*  bn exists on the bn_ctx as a TLV. The bn_ctx is the tag */
struct bn {
	struct bn_ctx* bnx;
};

/* convert to and from the integer format of the bn_t */
static
void buf_to_bn(uint32* a, const uint8* x, int len_bytes)
{
	int j = 0, k;
	for (k = 0; k < (len_bytes & 3); k++)
		a[j] = a[j] << 8 | x[k];

	for (j = (k > 0); k < len_bytes; j++, k += 4)
		a[j] = x[k] << 24 | x[k+1] << 16 | x[k+2] << 8 | x[k+3];
}

static
void bn_to_buf(const uint32* a, uint8* x, int len_bytes)
{
	int j = len_bytes & 3;
	int k = j;
	uint32 a0 = a[0];
	while (j) {
		x[--j] = (uint8)a0;
		a0 >>= 8;
	}
	for (j = (k > 0); k < len_bytes; j++) {
		x[k++] = a[j] >> 24;
		x[k++] = (a[j] >> 16) & 0xff;
		x[k++] = (a[j] >> 8) & 0xff;
		x[k++] = a[j] & 0xff;
	}
}

/*		*******************************************************************
 *				b n _ c t x   f u n c t i o n s
 *		*******************************************************************
 */

/* get the size allocated to the BN on the context */
static
int bn_get_size(int len_bytes)
{
	int size = sizeof(int *) + sizeof(uint32) + (((len_bytes + 3) >> 2) << 2);
	return size;
}

/*  create a bn context; numbers are expected to be of given bit length (at most) */
bn_ctx_t* bn_ctx_alloc(bn_alloc_fn_t alloc_fn, bn_free_fn_t free_fn, void *ctx, int max_bit_len)
{
	bn_ctx_t* bnx;
	if (alloc_fn == NULL || free_fn == NULL || ctx == NULL)
		return 0;

	/*  alloc_fn  also sets the bnx->block_id */
	bnx = (bn_ctx_t *)alloc_fn(ctx, sizeof(bn_ctx_t));
	if (!bnx)
		return 0;

	bnx->bn_alloc = alloc_fn;
	bnx->bn_free = free_fn;
	bnx->ctx = ctx;
	bnx->max_bit_len = max_bit_len;
	return bnx;
}

/*  the bn context is destroyed. All BN's must have already been freed */
void bn_ctx_destroy(bn_ctx_t **ctx)
{
	bn_ctx_t* bnx;
	if (ctx == NULL || *ctx == NULL)
		return;

	bnx = *ctx;
	bnx->bn_free(bnx->ctx, bnx, sizeof(bn_ctx_t));
	*ctx = 0;
}

/*		*******************************************************************
 *				b n    f u n c t i o n s
 *		*******************************************************************
 */

/* create a bn; initialize with data from buf (length in bytes) or 0 if NULL.
 * Context used for bn is expected to be valid for the lifetime of bn.
 */
bn_t* bn_alloc(bn_ctx_t *bnx, bn_format_t fmt, const uint8 *buf, int len_bytes)
{
	int size;
	int len_adj;
	bn_t *b;
	uint32 *data;
	if (!bnx)
		return 0;

	len_adj = (((len_bytes + 3) >> 2) << 2);
	size = bn_get_size(len_adj);
	b = bnx->bn_alloc(bnx->ctx, size);
	b->bnx = bnx;
	data = LV(b);
	*data++ = len_adj >> 2;
	if (buf == NULL)
		memset(data, 0, len_adj);
	else if (fmt == BN_FMT_LE) {  /* input and output are in the same format */
		int k = len_adj - len_bytes;
		memset(data, 0, k);
		memcpy((uint8 *)data + k, buf, len_bytes);
	}
	else
		buf_to_bn(data, buf, len_bytes);

	return b;
}

/* destroy a bn and initialize bn to NULL */
void bn_free(bn_t **bn)
{
	bn_ctx_t *bnx;
	int size, len_bytes;
	if (bn == NULL || *bn == NULL)
		return;

	bnx = bn_get_ctx(*bn);
	len_bytes = *LV(*bn) << 2;
	size = bn_get_size(len_bytes);
	bnx->bn_free(bnx->ctx, *bn, size);
	*bn = 0;
}

/* bn length in bits, excluding high level zero bits */
int bn_get_len_bits(const bn_t *bn)
{
	uint32* data = LV(bn);
	uint32 k = 0x80000000;
	int length = *data++;
	int len_bits;
	while (*data == 0 && length > 0) {
		data++;
		length--;
	}
	if (length == 0)
		return 0;

	len_bits = length << 5;
	while (!(*data & k)) {
		k >>= 1;
		len_bits--;
	}
	return len_bits;
}

/* truncate bn  --  reduce  len_bytes  by len_bits */
void bn_truncate(bn_t *bn, int trunc_len_bits)
{
	uint32 *data = LV(bn);
	int length = *data++;
	int total_len_bits = length << 5;
	int len_bits = total_len_bits - trunc_len_bits;
	if (len_bits <= 0) {
		bn_set(bn, BN_FMT_BE, 0, 0);
		return;
	}

	while (trunc_len_bits > 32) {
		*data++ = 0;
		trunc_len_bits -= 32;
	}
	*data &= (2 << (31 - trunc_len_bits)) - 1;
}

/* shift bn - shift is negative for left shift */
void bn_shift(bn_t *bn, int shift)
{
	int negative_shift = (shift < 0);
	uint32* data = LV(bn);
	int length = *data++, j, k;
	int shift_int = (negative_shift ? -shift : shift) >> 5;		/* from bits to uint32 */
	int shift_bits = (negative_shift ? -shift : shift) - (shift_int << 5);

	if (!negative_shift && shift > 0) {	/* right shift */
		uint val;
		j = length - 1;		/* output */
		k = j - shift_int;	/* input */
		while (k > 0) {
			val = data[k] >> shift_bits | data[k-1] << (32 - shift_bits);
			k--;
			data[j--] = val;
		}
		data[j--] = data[k] >> shift_bits;
		while (j >= 0)
			data[j--] = 0;
	}
	else if (negative_shift) {		/* left shift */
		uint val;
		j = 0;		/* output */
		k = j + shift_int;	/* input */
		while (k < length - 1) {
			val = data[k] << shift_bits | data[k+1] >> (32 - shift_bits);
			k++;
			data[j++] = val;
		}
		data[j++] = data[k] << shift_bits;
		while (j < length)
			data[j++] = 0;
	}
}

/* init a bn with data from buffer --  returns bytes copied */
int bn_set(bn_t *bn, bn_format_t buf_fmt, const uint8 *buf, int buf_len)
{
	uint32* data = LV(bn);
	int len_bytes = *data++ << 2;
	int copy, zero = 0, trunc = 0;
	if (len_bytes > buf_len) {
		copy = buf_len;
		zero = len_bytes - buf_len;
		memset(data, 0, zero);
	} else {
		copy = len_bytes;
		trunc = buf_len - len_bytes;
	}

	if (buf_fmt == BN_FMT_LE)
		memcpy((uint8 *)data+zero, buf+trunc, copy);
	else {
		int j = zero >> 2;  /*  sizeof uint32 */
		buf_to_bn(data+j, buf+trunc, copy);
	}
	return copy;
}

/* return byte length of buffer needed to output bn in a given format */
int bn_get_len_bytes(const bn_t *bn, bn_format_t fmt)
{
	uint32* data = LV(bn);
	int length = *data++;
	int len_bytes = length << 2;

	while (*data == 0 && len_bytes > 0) {
		data++;
		len_bytes -= 4;
	}
	if (len_bytes == 0 || fmt == BN_FMT_LE)
		return len_bytes;

	if (*data <= 0xffffff)
		len_bytes--;
	if (*data <= 0xffff)
		len_bytes--;
	if (*data <= 0xff)
		len_bytes--;
	return len_bytes;
}

/* serialize a bn, returns status BCME_* or output length >= 0 */
int bn_get(const bn_t *bn, bn_format_t buf_fmt, uint8 *buf, int buf_len)
{
	uint32* data = LV(bn);
	int len_bytes = *data++ << 2;
	int copy, zero = 0, trunc = 0;
	if (len_bytes >= buf_len) {
		copy = buf_len;
		trunc = len_bytes - buf_len;
	} else {
		copy = len_bytes;
		zero = buf_len - len_bytes;
		memset(buf, 0, zero);
	}

	if (buf_fmt == BN_FMT_LE)
		memcpy(buf+zero, (uint8 *)data+trunc, copy);
	else {
		int j = trunc >> 2;  /* sizeof uint32 */
		bn_to_buf(data+j, buf+zero, copy);
	}
	return copy;
}

bn_ctx_t* bn_get_ctx(bn_t *bn)
{
	return bn->bnx;
}

/* compare - returns -1, 0, or +1 if a < b, a == b or a > b respectively */
int bn_cmp(const bn_t *a, const bn_t *b)
{
	uint32* dataA = LV(a);
	uint32* dataB = LV(b);
	int j, posA, posB, length;
	int len_bytes_A = bn_get_len_bytes(a, BN_FMT_BE);
	int len_bytes_B = bn_get_len_bytes(b, BN_FMT_BE);
	if (len_bytes_A > len_bytes_B)
		return 1;

	if (len_bytes_A < len_bytes_B)
		return -1;

	length = (len_bytes_A + 3) >> 2;
	posA = *dataA++ - length;
	posB = *dataB++ - length;

	for (j=0; j<length; j++) {	/* a longer BN is automatically greater that a shorter BN */
		if (dataA[posA] > dataB[posB])
			return 1;

		if (dataA[posA] < dataB[posB])
			return -1;

		posA++;
		posB++;
	}
	return 0;
}

/* copy -- numeric from right to left */
int bn_copy(bn_t *dst, const bn_t *src)
{
	uint32* dataA = LV(dst);
	int a = *dataA++;
	uint32* dataB = LV(src);
	int b = *dataB++;
	while (a > 0 && b > 0) {
		a--;
		b--;
		dataA[a] = dataB[b];
	}
	while (a > 0)
		dataA[--a] = 0;

	return b==0;  /*  returns  0, if there had been trunctation */
}

/*		**********************************************************************
 *				b n    a r i h m e t i c  - - -  p r i v a t e
 *		**********************************************************************
 */

typedef union {
	uint64 f;
	uint32 h[2];
} uu64;

typedef union {
	uint32 f;
	uint16 h[2];
} uu;

#if _BYTE_ORDER == _LITTER_ENDIAN
#define uHI     U.h[1]
#define uLO     U.h[0]
#else
#define uHI     U.h[0]
#define uLO     U.h[1]
#endif // endif

#define uDW     U.f

static	/* used by the inverse function */
int is_one(uint16* a, int hi, int length)
{
	while (!a[hi] && hi < length - 1)
		hi++;

	return (a[hi] <= 1 && hi == length - 1);
}

static	/* used by the inverse function */
int compare(uint16* a, uint16* b, int hi, int length)
{
	int n = hi;
	while (n < length) {
		if (a[n] < b[n])
			return -1;

		if (a[n] > b[n])
			return 1;

		n++;
	}
	return 0;
}

static  //  out = a + b
int add(uint32 *out, uint32 *a, uint32 *b)
{
	uu64 U;
	int nA = *a++;
	int nB = *b++;
	int nOut = *out++;
	uDW = 0;
	while (nB > 0 && nA > 0 && nOut > 0) {
		nOut--;
		nB--;
		nA--;
		uDW = (uint64)uHI + a[nA] + b[nB];
		out[nOut] = uLO;
	}
	// the next 2 if statements cannot both be true
	while (nB > 0 && nOut > 0) {
		nOut--;
		nB--;
		uDW = (uint64)uHI + b[nB];
		out[nOut] = uLO;
	}

	while (nA > 0 && nOut > 0) {
		nOut--;
		nA--;
		uDW = (uint64)uHI + a[nA];
		out[nOut] = uLO;
	}

	while (nOut > 0) {
		nOut--;
		uDW = uHI;
		out[nOut] = uLO;
	}

	return uHI;
}

static  //  out = a - b
int subtract(uint32 *out, uint32 *a, uint32 *b)
{
	uu64 U;
	int nA = *a++;
	int nB = *b++;
	int nOut = *out++;
	uDW = 0;
	while (nB > 0 && nA > 0 && nOut > 0) {
		nB--;
		nA--;
		nOut--;
		uDW += (uint64)a[nA] - b[nB];
		out[nOut] = uLO;
		uLO = uHI; /* leading sign */
	}
	// the next 2 if statements cannot both be true
	while (nB > 0 && nOut > 0) {
		nOut--;
		nB--;
		uDW -= (uint64)b[nB];
		out[nOut] = uLO;
		uLO = uHI;
	}

	while (nA > 0 && nOut > 0) {
		nOut--;
		nA--;
		uDW += (uint64)a[nA];
		out[nOut] = uLO;
		uDW = uHI;
	}

	while (nOut > 0) {
		nOut--;
		out[nOut] = uLO;
		uLO = uHI;
	}

	return uHI;
}

static
void iswap(uu *b)
{
	int j = b->f;
	uu U;
	while (j) {
		U = b[j];
		b[j].h[0] = uHI;
		b[j].h[1] = uLO;
		j--;
	}
}

static  /*  t = t mod r  --  r[1] >= 0x80000000 */
void reduce_ordinary(uint32* tt, const uint32* rr)
{
	uint16 *t = (uint16 *)(tt+1);
	uint16 *r = (uint16 *)(rr+1);
	uint32 div = 1;
	int nT = *tt << 1;
	int nR = *rr << 1;
	int tHi = 0, nPos;
	int n, k, borrow;
	uu U;

	iswap((uu *)tt);
	iswap((uu *)rr);
	while (!*r) {
		r++;
		nR--;
	}
	while (1) {
		uDW = borrow = 0;
		while (uDW < r[0] + !div) {
			uHI = uLO;
			uLO = t[tHi++];
			if (tHi >= nT - (nR - 2)) {
				iswap((uu *)tt);
				iswap((uu *)rr);
				return;
			}
		}
		div = uDW / (r[0] + 1);
		nPos = tHi + nR - 2;
		n = nR;
		uDW = 0;
		while (n > 0) {
			n--;
			uDW = uHI + div * r[n];
			k = t[nPos];
			t[nPos] -= (uLO + borrow);
			borrow = k < (t[nPos] + borrow);
			nPos--;
		}
		t[nPos] -= (uHI + borrow);
		tHi = nPos;
	}
}

static  /* t = a * a       length(t) = 2 * length(a)         */
void square(uint32* t, uint32* a)
{
	int length = *a++;
	int aa = length, bb, cc = *t++ -1, tt;
	uu64 U;
	uint64 W;
	memset(t, 0, 8*length);
	while (aa > 1) {
		aa--;
		bb = aa;
		tt = aa << 1;
		uDW = 0;
		W = a[aa];
		while (bb > 0) {
			bb--;
			uDW =  W * a[bb] + uHI + t[tt];
			t[tt--] = uLO;
		}
		t[tt] = uHI;
	}
	tt = 2*length;
	uDW = 0;
	while (tt > 0) {
		tt--;
		uDW = (uint64)uHI + t[tt] + t[tt];
		t[tt] = uLO;
	}
	/*  the square part  */
	aa = length;
	uDW = 0;
	while (aa > 0) {
		aa--;
		uDW = uHI + (uint64)a[aa] * a[aa] + t[cc];
		t[cc--] = uLO;
		uDW = (uint64)uHI + t[cc];
		t[cc--] = uLO;
	}
}

static  /*  t = a * b      length: t = length: a + b  */
void multiply(uint32* t, uint32* a, uint32* b)
{
	int nT = *t++;
	int nA = *a++;
	int nB = *b++;
	int bb;
	int kB = 0, kA = 0;
	uu64 U;
	uint64 W;
	while (!a[kA]) kA++;
	while (!b[kB]) kB++;

	memset(t, 0, nT << 2);
	while (nA > kA) {
		nA--;
		nT = nB + nA;
		uDW = 0;
		bb = nB;
		W = a[nA];
		while (bb > kB) {
			bb--;
			uDW = W * b[bb] + uHI + t[nT];
			t[nT--] = uLO;
		}
		t[nT] = uHI;
	}
}

static  /*      a += b * c      called by divide -- inverse */
int mult_add(uint16* a, uint16* b, uint16* c, int length)
{
	int j = length, k, n;
	uu U;
	while (j > 0) {
		j--;
		if (c[j] == 0)
			continue;
		uDW = 0;
		k = length;
		n = j;
		while (k >= length-j) {
			k--;
			uDW = uHI + c[j] * b[k] + a[n];
			a[n] = uLO;
			n--;
		}
		while (uHI) {
			if (n < 0)
				return n;       /*  error */
			uDW = a[n] + uHI;
			a[n] = uLO;
			n--;
		}
	}
	return 1;
}

static  /*  called by divide */
void add_word(uint16 *a, uint16 val)
{
	uu U;
	uDW = *a + val;
	*a = uLO;
	while (uHI) {
		a--;
		uDW = uHI + *a;
		*a = uLO;
	}
}

static  /*      a / b = c  remainder  a     */
int divide(uint16* a, uint16* b, uint16* c, int hi, int length)
{
	int aHi = hi, bHi = hi, cHi, j, k;
	uint16 borrow, borrow2;
	uint16 cc, b1;
	uint32 b2;
	uu U;
	memset(c, 0, 2*length);	/* in bytes */

	while (b[bHi] == 0) bHi++;      /* start of divisor */
	b1 = b[bHi];
	if (bHi < length-1)
		b2 = b[bHi] << 16 | b[bHi+1];
	else
		b2 = b1;

	while (compare(a, b, hi, length) >= 0) {
		while (a[aHi] == 0) aHi++;
		hi = aHi;

		if (aHi < length-1) {
			uHI = a[aHi];
			uLO = a[aHi+1];
		} else
			uDW = a[aHi];

		if (a[aHi] > b[bHi] || (a[aHi] == b[bHi] && aHi == bHi)) {
			cHi = length - 1 - (bHi - aHi);
			if (b2 > b1)
				if (uDW == b2)
					cc = 1;
				else
					cc = uDW / (b2 + 1);
			else
				cc = a[aHi] / b[bHi];
		} else {
			cHi = length - (bHi - aHi);
			cc = uDW / (b1 + (b2 > b1));
			if (!cc)   // due to truncation from 0x1000 -- very rare
				cc = uDW / (b1 + 1);
		}
		add_word(c+cHi, cc);

		/*  a -= cc * b  */
		uDW = 0;
		j = length;
		k = cHi;
		while (1) {
			if (j > bHi)
				uDW += cc * b[--j];     /* input */
			else if (uDW == 0)
				break;
			borrow2 = a[k] < uLO;
			a[k--] -= uLO;
			if (k < 0)
				break;
			borrow = borrow2;
			uDW = uHI + borrow;     /* left-over */
		}
	}
	return hi;
}

/*		************************************************************************
 *				b n    a r i h m e t i c  - - -  p u b l i c
 *		************************************************************************
 */

/* r = a + b mod m;  m may be NULL;  b may be negative  */
void bn_iadd(bn_t *r, const bn_t *a, int b, const bn_t* m)
{
	uint nb = (b < 0) ? -b : b;
	bn_ctx_t *bnx = bn_get_ctx(r);
	bn_t* bb  = bn_alloc(bnx, BN_FMT_LE, (uint8 *)&nb, 4);	/* LE = memcpy */
	if (b < 0)
		bn_sub(r, a, bb, m);
	else
		bn_add(r, a, bb, m);

	bn_free(&bb);
}

/* r = a * b mod m; m may be NULL */
void bn_imul(bn_t *r, const bn_t *a, uint b, const bn_t *m)
{
	bn_ctx_t *bnx = bn_get_ctx(r);
	bn_t *ab = bn_alloc(bnx, BN_FMT_LE, (uint8 *)&b, 4);
	bn_mul(r, a, ab, m);
	bn_free(&ab);
}

/* r = a + b mod m; m may be NULL */
void bn_add(bn_t *r, const bn_t *a, const bn_t *b, const bn_t *m)
{
	uint32* dataA = LV(a);	/* to get the length field */
	uint32* dataB = LV(b);
	uint32* dataR = LV(r);
	uint32* dataM;

	int carry = add(dataR, dataA, dataB);
	if (!m)
		return;

	dataM = LV(m);
	if (carry || bn_cmp(r, m) > 0)
		subtract(dataR, dataR, dataM);
}

/* r = a - b mod m; m may be NULL */
void bn_sub(bn_t *r, const bn_t *a, const bn_t *b, const bn_t *m)
{
	uint32* dataA = LV(a);	/* to get the length field */
	uint32* dataB = LV(b);
	uint32* dataR = LV(r);
	uint32* dataM;
	int borrow = subtract(dataR, dataA, dataB);
	if (!m)
		return;

	dataM = LV(m);
	if (borrow)
		add(dataR, dataR, dataM);
}

/* r = a ^ 2 mod m; m may be NULL */
void bn_sqr(bn_t *r, const bn_t *a, const bn_t *m)
{
	bn_ctx_t *bnx = bn_get_ctx(r);
	bn_t* t = r;
	uint32 *aa = LV(a);
	uint32 *tt = LV(r);

	if (m || *tt < 2 * *aa) {
		t = bn_alloc(bnx, BN_FMT_BE, 0, *aa << 3); /* = twice the len-bytes of a  */
		tt = LV(t);
	}
	square(tt, aa);

	if (!m) {
		if (r != t)
			bn_copy(r, t);
	}
	else
		bn_mod(r, t, m);

	if (r != t)
		bn_free(&t);
}

/* r = a * b mod m; m may be NULL */
void bn_mul(bn_t *r, const bn_t *a, const bn_t *b, const bn_t *m)
{
	bn_ctx_t *bnx = bn_get_ctx(r);
	bn_t *t = r;
	uint32 *aa = LV(a);
	uint32 *bb = LV(b);
	uint32 *tt = LV(t);

	if (m || *tt < *aa + *bb) {
		t = bn_alloc(bnx, BN_FMT_BE, 0, (*aa + *bb) << 2);
		tt = LV(t);
	}

	multiply(tt, aa, bb);

	if (!m) {
		if (r != t)
			bn_copy(r, t);
	}
	else
		bn_mod(r, t, m);

	if (r != t)
		bn_free(&t);
}

/* mod: r = a mod m */
void bn_mod(bn_t* r, const bn_t *t, const bn_t *m)
{
	bn_ctx_t *bnx = bn_get_ctx(r);
	uint32 *tt = LV(t);
	uint32 *mm = LV(m);

	bn_t* modM = bn_alloc(bnx, BN_FMT_LE, (uint8 *)(mm + 1), *mm * 4);
	bn_t* modT = bn_alloc(bnx, BN_FMT_LE, 0, (*tt + 1) << 2);
	bn_t* modR = bn_alloc(bnx, BN_FMT_LE, 0, *mm << 2);

	int shift = bn_get_len_bits(m) - (*mm << 5);  /* negative if not zero */
	while (shift < -31)
		shift += 32;

	bn_copy(modT, t);
	if (shift) {
		bn_shift(modM, shift);
		bn_shift(modT, shift);
	}
	mm = LV(modM);
	tt = LV(modT);

	reduce_ordinary(tt, mm);

	if (bn_cmp(modT, modM) >= 0)
		bn_sub(modT, modT, modM, 0);

	if (shift)
		bn_shift(modT, -shift);

	bn_copy(r, modT);

	bn_free(&modR);
	bn_free(&modT);
	bn_free(&modM);
}

/* inverse a.inv = 1 mod (n) - used by ecdsa, for e.g. */
void bn_inv(const bn_t *a, const bn_t *n, bn_t *inv)
{
	bn_ctx_t *bnx = bn_get_ctx(inv);
	uint32* data = LV(n);
	uint32 length = *data << 1;
	uint32 hi = 0;
	bn_t *bk = bn_alloc(bnx, BN_FMT_BE, 0, length*2);
	bn_t *bk1 = bn_alloc(bnx, BN_FMT_BE, 0, length*2);
	bn_t *bk2 = bn_alloc(bnx, BN_FMT_BE, 0, length*2);
	bn_t *bj1 = bn_alloc(bnx, BN_FMT_BE, 0, length*2);
	bn_t *bj2 = bn_alloc(bnx, BN_FMT_BE, 0, length*2);

	uint16 *k = (uint16 *)(bk + 1) + 2;
	uint16 *k1 = (uint16 *)(bk1 + 1) + 2;
	uint16 *k2 = (uint16 *)(bk2 + 1) + 2;
	uint16 *j1 = (uint16 *)(bj1 + 1) + 2;
	uint16 *j2 = (uint16 *)(bj2 + 1) + 2;

	bn_iadd(bj2, bj2, 1, 0);
	if (bn_cmp(a, bj2) <= 0) {
		bn_copy(inv, a);
		goto bn_inv_free;
	}

	bn_copy(bk2, n);
	bn_copy(bk1, a);
	iswap((uu *)(k1 - 2));
	iswap((uu *)(k2 - 2));
	iswap((uu *)(j2 - 2)); // swap the one value

	while (1) {
		hi = divide(k2, k1, k, hi, length);     /* remainder in k2  */
		mult_add(j1, j2, k, length);
		if (is_one(k2, hi, length)) {
			iswap((uu *)(j1 - 2));
			bn_sub(inv, n, bj1, 0);			/* a = P - a */
			break;
		}

		hi = divide(k1, k2, k, hi, length);     /* remainder in k1  */
		mult_add(j2, j1, k, length);
		if (is_one(k1, hi, length)) {
			iswap((uu *)(j2 - 2));
			bn_copy(inv, bj2);
			break;
		}
	}
bn_inv_free:
	bn_free(&bj2);
	bn_free(&bj1);
	bn_free(&bk2);
	bn_free(&bk1);
	bn_free(&bk);
}
