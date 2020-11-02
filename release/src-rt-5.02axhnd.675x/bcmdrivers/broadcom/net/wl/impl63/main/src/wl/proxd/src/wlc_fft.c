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
 * $Id: wlc_fft.c 681798 2017-01-28 11:09:32Z $
 */
#include <wlc_fft.h>
#include <osl.h>
#include <bcm_math.h>

#define satfft(x)	(x)
#define SQRT_HALF	181
#define SQRT_HALF_SFT	(8)
#define TWDL_SFT	(8)

#define FFT64_MEMLEN	128
#define FFT128_MEMLEN	192
#define FFT256_MEMLEN	256

#if defined(TOF_SEQ_20_IN_80MHz) || defined(TOF_SEQ_20MHz_BW_512IFFT)
#define FFT512_MEMLEN   512
#define fftTWDL_cos(i)	math_fft_cos_seq20(i)
#define fftTWDL_sin(i)	math_fft_sin_seq20(i)
#else
#define fftTWDL_cos(i)	math_fft_cos(i)
#define fftTWDL_sin(i)	math_fft_sin(i)
#endif /* TOF_SEQ_20_IN_80MHz || TOF_SEQ_20MHz_BW_512IFFT */

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

#if defined(TOF_SEQ_20_IN_80MHz) || defined(TOF_SEQ_20MHz_BW_512IFFT)

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
		tmp0_i = tmp3[i].i * math_fft_cos_128(i) - tmp3[i].q * math_fft_sin_128(i);
		tmp0_i = ROUND(tmp0_i, TWDL_SFT) + tmp1[i].i;
		tmp4[i].i = satfft(tmp0_i);

		tmp0_i = tmp3[i].i * math_fft_cos_128(i+256) - tmp3[i].q * math_fft_sin_128(i+256);
		tmp0_i = ROUND(tmp0_i, TWDL_SFT) + tmp1[i].i;
		tmp4[i+256].i = satfft(tmp0_i);

		tmp0_q = tmp3[i].i * math_fft_sin_128(i) + tmp3[i].q * math_fft_cos_128(i);
		tmp0_q = ROUND(tmp0_q, TWDL_SFT) + tmp1[i].q;
		tmp4[i].q = satfft(tmp0_q);

		tmp0_q = tmp3[i].i * math_fft_sin_128(i+256) + tmp3[i].q * math_fft_cos_128(i+256);
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
#if defined(TOF_SEQ_20_IN_80MHz) || defined(TOF_SEQ_20MHz_BW_512IFFT)
	else if (n <= 512)
		FFT512(osh, (cint32*)inBuf, (cint32*)outBuf);
#endif // endif
}
