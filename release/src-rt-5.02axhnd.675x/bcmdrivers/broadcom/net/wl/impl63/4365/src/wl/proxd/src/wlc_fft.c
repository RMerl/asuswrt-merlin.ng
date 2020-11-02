/*
 * TOF based proximity detection implementation for Broadcom 802.11 Networking Driver
 *
 * Copyright 2020 Broadcom
 *
 * This program is the proprietary software of Broadcom and/or
 * its licensors, and may only be used, duplicated, modified or distributed
 * pursuant to the terms and conditions of a separate, written license
 * agreement executed between you and Broadcom (an "Authorized License").
 * Except as set forth in an Authorized License, Broadcom grants no license
 * (express or implied), right to use, or waiver of any kind with respect to
 * the Software, and Broadcom expressly reserves all rights in and to the
 * Software and all intellectual property rights therein.  IF YOU HAVE NO
 * AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY
 * WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF
 * THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use
 * all reasonable efforts to protect the confidentiality thereof, and to
 * use this information only in connection with your use of Broadcom
 * integrated circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
 * REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR
 * OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
 * DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
 * NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
 * ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
 * OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
 * BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL,
 * SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR
 * IN ANY WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
 * IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii)
 * ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF
 * OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY
 * NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 * $Id: wlc_fft.c 491282 2014-07-15 19:26:17Z $
 */
#include <wlc_fft.h>
#include <osl.h>

#define satfft(x)	(x)
#define SQRT_HALF	181
#define SQRT_HALF_SFT	(8)
#define TWDL_SFT	(8)

#define FFT64_MEMLEN	128
#define FFT128_MEMLEN	192
#define FFT256_MEMLEN	256

#ifdef TOF_SEQ_20_IN_80MHz
#define FFT512_MEMLEN   512
static int16 TWDL_cos_128[129] = {
	256,
	256,
	256,
	256,
	256,
	256,
	255,
	255,
	255,
	254,
	254,
	254,
	253,
	253,
	252,
	252,
	251,
	250,
	250,
	249,
	248,
	248,
	247,
	246,
	245,
	244,
	243,
	242,
	241,
	240,
	239,
	238,
	237,
	235,
	234,
	233,
	231,
	230,
	229,
	227,
	226,
	224,
	223,
	221,
	220,
	218,
	216,
	215,
	213,
	211,
	209,
	207,
	206,
	204,
	202,
	200,
	198,
	196,
	194,
	192,
	190,
	188,
	185,
	183,
	181,
	179,
	177,
	174,
	172,
	170,
	167,
	165,
	162,
	160,
	157,
	155,
	152,
	150,
	147,
	145,
	142,
	140,
	137,
	134,
	132,
	129,
	126,
	123,
	121,
	118,
	115,
	112,
	109,
	107,
	104,
	101,
	98,
	95,
	92,
	89,
	86,
	83,
	80,
	77,
	74,
	71,
	68,
	65,
	62,
	59,
	56,
	53,
	50,
	47,
	44,
	41,
	38,
	34,
	31,
	28,
	25,
	22,
	19,
	16,
	13,
	9,
	6,
	3,
	0,
};
#else
static int16 TWDL_cos[65] = {
	/* cos(2*M_PI*i/256.0) * pow(2.0, TWDL_SFT) */
	 256, /* 256.00000000 */
	 256, /* 255.92289759 */
	 256, /* 255.69163679 */
	 255, /* 255.30635691 */
	 255, /* 254.76729003 */
	 254, /* 254.07476086 */
	 253, /* 253.22918655 */
	 252, /* 252.23107645 */
	 251, /* 251.08103178 */
	 250, /* 249.77974529 */
	 248, /* 248.32800082 */
	 247, /* 246.72667284 */
	 245, /* 244.97672595 */
	 243, /* 243.07921423 */
	 241, /* 241.03528069 */
	 239, /* 238.84615650 */
	 237, /* 236.51316032 */
	 234, /* 234.03769746 */
	 231, /* 231.42125904 */
	 229, /* 228.66542111 */
	 226, /* 225.77184367 */
	 223, /* 222.74226972 */
	 220, /* 219.57852416 */
	 216, /* 216.28251270 */
	 213, /* 212.85622075 */
	 209, /* 209.30171217 */
	 206, /* 205.62112806 */
	 202, /* 201.81668547 */
	 198, /* 197.89067606 */
	 194, /* 193.84546471 */
	 190, /* 189.68348809 */
	 185, /* 185.40725324 */
	 181, /* 181.01933598 */
	 177, /* 176.52237945 */
	 172, /* 171.91909244 */
	 167, /* 167.21224780 */
	 162, /* 162.40468075 */
	 157, /* 157.49928719 */
	 152, /* 152.49902195 */
	 147, /* 147.40689700 */
	 142, /* 142.22597965 */
	 137, /* 136.95939069 */
	 132, /* 131.61030251 */
	 126, /* 126.18193721 */
	 121, /* 120.67756463 */
	 115, /* 115.10050039 */
	 109, /* 109.45410392 */
	 104, /* 103.74177639 */
	  98, /* 97.96695869 */
	  92, /* 92.13312935 */
	  86, /* 86.24380247 */
	  80, /* 80.30252554 */
	  74, /* 74.31287738 */
	  68, /* 68.27846591 */
	  62, /* 62.20292606 */
	  56, /* 56.08991748 */
	  50, /* 49.94312244 */
	  44, /* 43.76624352 */
	  38, /* 37.56300146 */
	  31, /* 31.33713285 */
	  25, /* 25.09238792 */
	  19, /* 18.83252828 */
	  13, /* 12.56132463 */
	   6, /* 6.28255450 */
	   0, /* 0.00000000 */
};

#endif /* TOF_SEQ_20_IN_80MHz */

/* Bit Reverse Function */
static int bitRev(int idxIn, int nBits)
{
	int i, idxOut;

	idxOut = 0;
	for (i = 0; i < nBits; i++)
	{
		idxOut = idxOut << 1;
		idxOut = idxOut | (idxIn & 0x1);
		idxIn = idxIn >> 1;
	}

	return idxOut;
}

#ifdef TOF_SEQ_20_IN_80MHz

/* Get cos value from the table */
static int32 fftTWDL_cos(int i)
{
	if (i < 65)
		return TWDL_cos_128[i*2];

	if (i < 129)
		return (-1 *TWDL_cos_128[256 - i*2]);

	if (i < 193)
		return (-1*TWDL_cos_128[i*2 - 256]);

	return TWDL_cos_128[512 -i*2];
}

static int32 fftTWDL_cos_128(int i)
{
	if (i < 129)
		return TWDL_cos_128[i];
	if (i < 257)
		return (-1 * TWDL_cos_128[256 - i]);
	if (i < 385)
		return (-1 * TWDL_cos_128[i-256]);
	return TWDL_cos_128[512 - i];
}

/* Get sin value from the table */
static int32 fftTWDL_sin(int i)
{
	if (i < 65)
		return (-1 * TWDL_cos_128[128 - i*2]);

	if (i < 129)
		return (-1 * TWDL_cos_128[i*2 - 128]);

	if (i < 193)
		return TWDL_cos_128[384 - i*2];

	return TWDL_cos_128[i*2 - 384];
}

static int32 fftTWDL_sin_128(int i)
{
	if (i < 129)
		return (-1 * TWDL_cos_128[128 - i]);
	if (i < 257)
		return (-1 * TWDL_cos_128[i - 128]);
	if (i < 385)
		return TWDL_cos_128[384 - i];
	return TWDL_cos_128[i - 384];

}
#else
/* Get cos value from the table */
static int32 fftTWDL_cos(int i)
{
	if (i < 65)
		return TWDL_cos[i];
	if (i < 129)
		return (-1 *TWDL_cos[128 - i]);
	if (i < 193)
		return (-1*TWDL_cos[i-128]);
	return TWDL_cos[256 - i];
}

/* Get sin value from the table */
static int32 fftTWDL_sin(int i)
{
	if (i < 65)
		return (-1 * TWDL_cos[64 - i]);
	if (i < 129)
		return (-1 *TWDL_cos[i - 64]);
	if (i < 193)
		return TWDL_cos[192 - i];
	return TWDL_cos[i - 192];
}
#endif /* TOF_SEQ_20_IN_80MHz */

/* Fast fourier transform R2 */
static void fftR2(cint32 *inBuf, int inStep, int inSft, cint32 *outBuf, int outStep, int outSft)
{
	int n1 = inStep+inSft;
	int n2 = outStep+outSft;

	outBuf[outSft].i = inBuf[inSft].i + inBuf[n1].i;
	outBuf[outSft].q = inBuf[inSft].q + inBuf[n1].q;
	outBuf[n2].i = inBuf[inSft].i - inBuf[n1].i;
	outBuf[n2].q = inBuf[inSft].q - inBuf[n1].q;
}

/* Fast fourier transform with R8 */
static void fftR8(cint32 *inBuf, int inStep, int inSft, cint32 *outBuf, int outStep, int outSft)
{
	int i, n1, n2, n3, n4;
	cint32 tmp1[8];
	cint32 tmp2[8];
	int32 tmp0_i, tmp0_q;

	/* 1st R2 stage */
	for (i = 0; i < 4; i++)
	{
		n1 = inStep * i + inSft;
		n2 = n1 + (inStep << 2);
		n3 = i + 4;

		tmp1[i].i = inBuf[n1].i + inBuf[n2].i;
		tmp1[i].q = inBuf[n1].q + inBuf[n2].q;
		tmp1[n3].i = inBuf[n1].i - inBuf[n2].i;
		tmp1[n3].q = inBuf[n1].q - inBuf[n2].q;
	}

	/* 1st R2 stage Twiddles */
	tmp0_i = (tmp1[5].i + tmp1[5].q) * (int32)SQRT_HALF;
	tmp0_q = (-tmp1[5].i + tmp1[5].q) * (int32)SQRT_HALF;
	tmp1[5].i = ROUND(tmp0_i, SQRT_HALF_SFT);
	tmp1[5].q = ROUND(tmp0_q, SQRT_HALF_SFT);

	tmp0_i = tmp1[6].q;
	tmp0_q = -tmp1[6].i;
	tmp1[6].i = tmp0_i;
	tmp1[6].q = tmp0_q;

	tmp0_i = (-tmp1[7].i + tmp1[7].q) * (int32)SQRT_HALF;
	tmp0_q = (-tmp1[7].i - tmp1[7].q) * (int32)SQRT_HALF;
	tmp1[7].i = ROUND(tmp0_i, SQRT_HALF_SFT);
	tmp1[7].q = ROUND(tmp0_q, SQRT_HALF_SFT);

	/* 2nd R2 stage */
	for (i = 0; i < 2; i++)
	{
		n2 = i + 2;
		n3 = i + 4;
		n4 = i + 6;

		tmp2[i].i = tmp1[i].i + tmp1[n2].i;
		tmp2[i].q = tmp1[i].q + tmp1[n2].q;
		tmp2[n2].i = tmp1[i].i - tmp1[n2].i;
		tmp2[n2].q = tmp1[i].q - tmp1[n2].q;
		tmp2[n3].i = tmp1[n3].i + tmp1[n4].i;
		tmp2[n3].q = tmp1[n3].q + tmp1[n4].q;
		tmp2[n4].i = tmp1[n3].i - tmp1[n4].i;
		tmp2[n4].q = tmp1[n3].q - tmp1[n4].q;
	}

	/* 2nd R2 stage Twiddle */
	tmp0_i = tmp2[3].q;
	tmp0_q = -tmp2[3].i;
	tmp2[3].i = tmp0_i;
	tmp2[3].q = tmp0_q;

	tmp0_i = tmp2[7].q;
	tmp0_q = -tmp2[7].i;
	tmp2[7].i = tmp0_i;
	tmp2[7].q = tmp0_q;

	/* 3rd R2 stage */
	for (i = 0; i < 4; i++)
	{
		n1 = (i << 1);
		n2 = n1 + 1;
		n3 = n1 * outStep+outSft;
		n4 = n3 + outStep;

		outBuf[n3].i = tmp2[n1].i + tmp2[n2].i;
		outBuf[n3].q = tmp2[n1].q + tmp2[n2].q;
		outBuf[n4].i = tmp2[n1].i - tmp2[n2].i;
		outBuf[n4].q = tmp2[n1].q - tmp2[n2].q;
	}
}
/* Fast fourier transform with R64 */
static void fftR64(cint32 *inBuf, cint32 *outBuf, cint32 *tmp1)
{
	int i, j, n1, n2, n3, n4;
	int32 twdl_i, twdl_q, tmp_i, tmp_q, tmp0_i, tmp0_q;

	/* 1st Stage of R8 */
	for (i = 0; i < 8; i++)
	{
		fftR8(inBuf, 8, i, tmp1, 8, i);
	}

	/* 1st Stage of R8 Twiddle */
	for (i = 0; i < 8; i++)
	{
		n1 = bitRev(i, 3) << 2;
		n2 = (i << 3);

		for (j = 0; j < 8; j++)
		{
			n3 = j * n1;
			n4 = j + n2;

			twdl_i = fftTWDL_cos(n3);
			twdl_q = fftTWDL_sin(n3);
			tmp_i = (tmp1[n4].i >> 2);
			tmp_q = (tmp1[n4].q >>2);

			tmp0_i = tmp_i * twdl_i - tmp_q * twdl_q;
			tmp0_q = tmp_i * twdl_q + tmp_q * twdl_i;

			outBuf[n4].i = ROUND(tmp0_i, TWDL_SFT);
			outBuf[n4].q = ROUND(tmp0_q, TWDL_SFT);
		}
	}

	/* 2nd Stage of R8 */
	for (i = 0; i < 8; i++)
	{
		fftR8(outBuf, 1, 8*i, tmp1, 1, 8*i);
	}

	for (i = 0; i < 64; i++)
	{
		outBuf[i].i = tmp1[i].i >> 1;
		outBuf[i].q = tmp1[i].q >> 1;
	}
}

/* Fast fourier transform 128 R64 */
static void fft128R64(cint32 *inBuf, cint32 *outBuf, cint32 *tmp1)
{
	int i, n1, n2, n3;
	int32 twdl_i, twdl_q, tmp0_i, tmp0_q;
	cint32 *tmp3, *tmp4;

	tmp4 = tmp1 + 128;

	/* R64 Stage */
	for (i = 0; i < 64; i++)
	{
		n1 = (i << 1);
		tmp1[i].i = inBuf[n1].i;
		tmp1[i].q = inBuf[n1].q;
		tmp1[i + 64].i = inBuf[n1+1].i;
		tmp1[i + 64].q = inBuf[n1+1].q;
	}
	memcpy(inBuf, tmp1, 128 * sizeof(cint32));

	tmp3 = inBuf;
	fftR64(tmp3, tmp3, tmp4);
	for (i = 0; i < 64; i++)
	{
		n1 = (i << 1);
		tmp1[n1].i = tmp3[i].i;
		tmp1[n1].q = tmp3[i].q;
	}

	tmp3 = inBuf + 64;
	fftR64(tmp3, tmp3, tmp4);
	for (i = 0; i < 64; i++)
	{
		n1 = (i << 1) + 1;
		tmp1[n1].i = tmp3[i].i;
		tmp1[n1].q = tmp3[i].q;
	}

	/* R64 Stage Twiddle */
	for (i = 0; i < 64; i++)
	{
		n1 = (i << 1);
		n2 = n1 + 1;
		n3 = bitRev(i, 6) << 1;

		outBuf[n1].i = satfft(tmp1[n1].i);
		outBuf[n1].q = satfft(tmp1[n1].q);

		twdl_i = fftTWDL_cos(n3);
		twdl_q = fftTWDL_sin(n3);
		tmp0_i = tmp1[n2].i * twdl_i - tmp1[n2].q * twdl_q;
		tmp0_q = tmp1[n2].i * twdl_q + tmp1[n2].q * twdl_i;

		outBuf[n2].i = satfft(ROUND(tmp0_i, TWDL_SFT));
		outBuf[n2].q = satfft(ROUND(tmp0_q, TWDL_SFT));
	}

	/* R2 Stage */
	for (i = 0; i < 64; i++)
	{
		fftR2(outBuf, 1, 2*i, tmp1, 1, 2*i);
	}

	/* Bit-reveresed re-ordering */
	for (i = 0; i < 128; i++)
	{
		n1 = bitRev(i, 7);
		outBuf[i].i = satfft(tmp1[n1].i >> 1);
		outBuf[i].q = satfft(tmp1[n1].q >> 1);
	}
}
/* Fast fourier transform 64 */
int FFT64(osl_t *osh, cint32 *inBuf, cint32 *outBuf)
{
	int i, n1;
	cint32 *tmp1;

	if ((tmp1 = MALLOC(osh, FFT64_MEMLEN* sizeof(cint32))) == NULL)
		return BCME_NOMEM;

	fftR64(inBuf, tmp1, tmp1 + 64);

	/* Bit-reveresed re-ordering */
	for (i = 0; i < 64; i++)
	{
		n1 = bitRev(i, 6);
		outBuf[i].i = tmp1[n1].i;
		outBuf[i].q = tmp1[n1].q;
	}
	MFREE(osh, tmp1, FFT64_MEMLEN * sizeof(cint32));

	return 0;
}
/* Fast fourier transform 128 */
int FFT128(osl_t *osh, cint32 *inBuf, cint32 *outBuf)
{
	cint32 *tmp1;

	if ((tmp1 = MALLOC(osh, FFT128_MEMLEN * sizeof(cint32))) == NULL)
		return BCME_NOMEM;

	fft128R64(inBuf, outBuf, tmp1);

	MFREE(osh, tmp1, FFT128_MEMLEN * sizeof(cint32));

	return 0;
}
/* Fast fourier transform with 256 */
int FFT256(osl_t *osh, cint32 *inBuf, cint32 *outBuf)
{
	int i, n1;
	int32 tmp0_i, tmp0_q;
	cint32 *tmp1, *tmp3, *tmp4;

	if ((tmp4 = MALLOC(osh, FFT256_MEMLEN * sizeof(cint32))) == NULL)
		return BCME_NOMEM;

	tmp1 = inBuf;
	tmp3 = tmp1 + 128;

	/* First 128 pt even samples, next 128 pt odd samples of 256 pt FFT */
	for (i = 0; i < 128; i++)
	{
		n1 = (i << 1);
		tmp4[i].i = inBuf[n1].i;
		tmp4[i].q = inBuf[n1].q;
		tmp4[i + 128].i = inBuf[n1+1].i;
		tmp4[i + 128].q = inBuf[n1+1].q;
	}
	memcpy(tmp1, tmp4, 256 * sizeof(cint32));
	fft128R64(tmp1, tmp1, tmp4);
	fft128R64(tmp3, tmp3, tmp4);

	/* Combine the two 128 pt FFTs */
	for (i = 0; i < 128; i++)
	{
		tmp0_i = tmp3[i].i * fftTWDL_cos(i) - tmp3[i].q * fftTWDL_sin(i);
		tmp0_i = ROUND(tmp0_i, TWDL_SFT) + tmp1[i].i;
		tmp4[i].i = satfft(tmp0_i);

		tmp0_i = tmp3[i].i * fftTWDL_cos(i+128) - tmp3[i].q * fftTWDL_sin(i+128);
		tmp0_i = ROUND(tmp0_i, TWDL_SFT) + tmp1[i].i;
		tmp4[i+128].i = satfft(tmp0_i);

		tmp0_q = tmp3[i].i * fftTWDL_sin(i) + tmp3[i].q * fftTWDL_cos(i);
		tmp0_q = ROUND(tmp0_q, TWDL_SFT) + tmp1[i].q;
		tmp4[i].q = satfft(tmp0_q);

		tmp0_q = tmp3[i].i * fftTWDL_sin(i+128) + tmp3[i].q * fftTWDL_cos(i+128);
		tmp0_q = ROUND(tmp0_q, TWDL_SFT) + tmp1[i].q;
		tmp4[i+128].q = satfft(tmp0_q);
	}
	memcpy(outBuf, tmp4, 256 *sizeof(cint32));
	MFREE(osh, tmp4, FFT256_MEMLEN * sizeof(cint32));

	return 0;
}

#ifdef TOF_SEQ_20_IN_80MHz

/* Fast fourier transform with 512 */
int FFT512(osl_t *osh, cint32 *inBuf, cint32 *outBuf)
{
	int i, n1;
	int32 tmp0_i, tmp0_q;
	cint32 *tmp1, *tmp3, *tmp4;

	if ((tmp4 = MALLOC(osh, FFT512_MEMLEN * sizeof(cint32))) == NULL)
		return BCME_NOMEM;

	tmp1 = inBuf;
	tmp3 = tmp1 + 256;

	/* First 256 pt even samples, next 256 pt odd samples of 512 pt FFT */
	for (i = 0; i < 256; i++)
	{
		n1 = (i << 1);
		tmp4[i].i = inBuf[n1].i;
		tmp4[i].q = inBuf[n1].q;
		tmp4[i + 256].i = inBuf[n1+1].i;
		tmp4[i + 256].q = inBuf[n1+1].q;
	}
	memcpy(tmp1, tmp4, 512 * sizeof(cint32));
	FFT256(osh, tmp1, tmp1);
	FFT256(osh, tmp3, tmp3);

	/* Combine the two 256 pt FFTs */
	for (i = 0; i < 256; i++)
	{
		tmp0_i = tmp3[i].i * fftTWDL_cos_128(i) - tmp3[i].q * fftTWDL_sin_128(i);
		tmp0_i = ROUND(tmp0_i, TWDL_SFT) + tmp1[i].i;
		tmp4[i].i = satfft(tmp0_i);

		tmp0_i = tmp3[i].i * fftTWDL_cos_128(i+256) - tmp3[i].q * fftTWDL_sin_128(i+256);
		tmp0_i = ROUND(tmp0_i, TWDL_SFT) + tmp1[i].i;
		tmp4[i+256].i = satfft(tmp0_i);

		tmp0_q = tmp3[i].i * fftTWDL_sin_128(i) + tmp3[i].q * fftTWDL_cos_128(i);
		tmp0_q = ROUND(tmp0_q, TWDL_SFT) + tmp1[i].q;
		tmp4[i].q = satfft(tmp0_q);

		tmp0_q = tmp3[i].i * fftTWDL_sin_128(i+256) + tmp3[i].q * fftTWDL_cos_128(i+256);
		tmp0_q = ROUND(tmp0_q, TWDL_SFT) + tmp1[i].q;
		tmp4[i+256].q = satfft(tmp0_q);
	}
	memcpy(outBuf, tmp4, 512 *sizeof(cint32));
	MFREE(osh, tmp4, FFT512_MEMLEN * sizeof(cint32));

	return 0;
}

#endif /* TOF_SEQ_20_IN_80MHz */

void
wlapi_pdtof_fft(osl_t *osh, int n, void *inBuf, void *outBuf, int oversamp)
{
	if (n <= 64)
		FFT64(osh, (cint32*)inBuf, (cint32*)outBuf);
	else if (n <= 128)
		FFT128(osh, (cint32*)inBuf, (cint32*)outBuf);
	else if (n <= 256)
		FFT256(osh, (cint32*)inBuf, (cint32*)outBuf);
#ifdef TOF_SEQ_20_IN_80MHz
	else if (n <= 512)
		FFT512(osh, (cint32*)inBuf, (cint32*)outBuf);
#endif // endif
}
