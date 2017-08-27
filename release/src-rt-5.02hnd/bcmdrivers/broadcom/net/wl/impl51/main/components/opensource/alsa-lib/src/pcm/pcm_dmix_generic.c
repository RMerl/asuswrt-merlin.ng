
#ifndef ARCH_ADD
#define ARCH_ADD(p,a) (*(p) += (a))
#define ARCH_CMPXCHG(p,a,b) (*(p)) /* fake */
#define NO_CONCURRENT_ACCESS	/* use semaphore to avoid race */
#define IS_CONCURRENT	0	/* no race check */
#endif

#if IS_CONCURRENT
static void mix_areas_16(unsigned int size,
			 volatile signed short *dst, signed short *src,
			 volatile signed int *sum, size_t dst_step,
			 size_t src_step, size_t sum_step)
{
	register signed int sample, old_sample;

	for (;;) {
		sample = *src;
		old_sample = *sum;
		if (ARCH_CMPXCHG(dst, 0, 1) == 0)
			sample -= old_sample;
		ARCH_ADD(sum, sample);
		do {
			old_sample = *sum;
			if (old_sample > 0x7fff)
				sample = 0x7fff;
			else if (old_sample < -0x8000)
				sample = -0x8000;
			else
				sample = old_sample;
			*dst = sample;
		} while (IS_CONCURRENT && *sum != old_sample);
		if (!--size)
			return;
		src = (signed short *) ((char *)src + src_step);
		dst = (signed short *) ((char *)dst + dst_step);
		sum = (signed int *)   ((char *)sum + sum_step);
	}
}

static void mix_areas_32(unsigned int size,
			 volatile signed int *dst, signed int *src,
			 volatile signed int *sum, size_t dst_step,
			 size_t src_step, size_t sum_step)
{
	register signed int sample, old_sample;

	for (;;) {
		sample = *src >> 8;
		old_sample = *sum;
		if (ARCH_CMPXCHG(dst, 0, 1) == 0)
			sample -= old_sample;
		ARCH_ADD(sum, sample);
		do {
			old_sample = *sum;
			if (old_sample > 0x7fffff)
				sample = 0x7fffffff;
			else if (old_sample < -0x800000)
				sample = -0x80000000;
			else
				sample = old_sample * 256;
			*dst = sample;
		} while (IS_CONCURRENT && *sum != old_sample);
		if (!--size)
			return;
		src = (signed int *) ((char *)src + src_step);
		dst = (signed int *) ((char *)dst + dst_step);
		sum = (signed int *) ((char *)sum + sum_step);
	}
}

static void mix_select_callbacks(snd_pcm_direct_t *dmix)
{
	dmix->u.dmix.mix_areas_16 = mix_areas_16;
	dmix->u.dmix.mix_areas_32 = mix_areas_32;
}

#else

/* non-concurrent version, supporting both endians */
#define generic_dmix_supported_format \
	((1ULL << SND_PCM_FORMAT_S16_LE) | (1ULL << SND_PCM_FORMAT_S32_LE) |\
	 (1ULL << SND_PCM_FORMAT_S16_BE) | (1ULL << SND_PCM_FORMAT_S32_BE) |\
	 (1ULL << SND_PCM_FORMAT_S24_LE) | (1ULL << SND_PCM_FORMAT_S24_3LE) | \
	 (1ULL << SND_PCM_FORMAT_U8))

#include <byteswap.h>

static void generic_mix_areas_16_native(unsigned int size,
					volatile signed short *dst,
					signed short *src,
					volatile signed int *sum,
					size_t dst_step,
					size_t src_step,
					size_t sum_step)
{
	register signed int sample;

	for (;;) {
		sample = *src;
		if (! *dst) {
			*sum = sample;
			*dst = *src;
		} else {
			sample += *sum;
			*sum = sample;
			if (sample > 0x7fff)
				sample = 0x7fff;
			else if (sample < -0x8000)
				sample = -0x8000;
			*dst = sample;
		}
		if (!--size)
			return;
		src = (signed short *) ((char *)src + src_step);
		dst = (signed short *) ((char *)dst + dst_step);
		sum = (signed int *)   ((char *)sum + sum_step);
	}
}

static void generic_remix_areas_16_native(unsigned int size,
					  volatile signed short *dst,
					  signed short *src,
					  volatile signed int *sum,
					  size_t dst_step,
					  size_t src_step,
					  size_t sum_step)
{
	register signed int sample;

	for (;;) {
		sample = *src;
		if (! *dst) {
			*sum = -sample;
			*dst = -sample;
		} else {
			*sum = sample = *sum - sample;
			if (sample > 0x7fff)
				sample = 0x7fff;
			else if (sample < -0x8000)
				sample = -0x8000;
			*dst = sample;
		}
		if (!--size)
			return;
		src = (signed short *) ((char *)src + src_step);
		dst = (signed short *) ((char *)dst + dst_step);
		sum = (signed int *)   ((char *)sum + sum_step);
	}
}

static void generic_mix_areas_32_native(unsigned int size,
					volatile signed int *dst,
					signed int *src,
					volatile signed int *sum,
					size_t dst_step,
					size_t src_step,
					size_t sum_step)
{
	register signed int sample;

	for (;;) {
		sample = *src >> 8;
		if (! *dst) {
			*sum = sample;
			*dst = *src;
		} else {
			sample += *sum;
			*sum = sample;
			if (sample > 0x7fffff)
				sample = 0x7fffffff;
			else if (sample < -0x800000)
				sample = -0x80000000;
			else
				sample *= 256;
			*dst = sample;
		}
		if (!--size)
			return;
		src = (signed int *) ((char *)src + src_step);
		dst = (signed int *) ((char *)dst + dst_step);
		sum = (signed int *) ((char *)sum + sum_step);
	}
}

static void generic_remix_areas_32_native(unsigned int size,
					  volatile signed int *dst,
					  signed int *src,
					  volatile signed int *sum,
					  size_t dst_step,
					  size_t src_step,
					  size_t sum_step)
{
	register signed int sample;

	for (;;) {
		sample = *src >> 8;
		if (! *dst) {
			*sum = -sample;
			*dst = -*src;
		} else {
			*sum = sample = *sum - sample;
			if (sample > 0x7fffff)
				sample = 0x7fffffff;
			else if (sample < -0x800000)
				sample = -0x80000000;
			else
				sample *= 256;
			*dst = sample;
		}
		if (!--size)
			return;
		src = (signed int *) ((char *)src + src_step);
		dst = (signed int *) ((char *)dst + dst_step);
		sum = (signed int *) ((char *)sum + sum_step);
	}
}

static void generic_mix_areas_16_swap(unsigned int size,
				      volatile signed short *dst,
				      signed short *src,
				      volatile signed int *sum,
				      size_t dst_step,
				      size_t src_step,
				      size_t sum_step)
{
	register signed int sample;

	for (;;) {
		sample = (signed short) bswap_16(*src);
		if (! *dst) {
			*sum = sample;
			*dst = *src;
		} else {
			sample += *sum;
			*sum = sample;
			if (sample > 0x7fff)
				sample = 0x7fff;
			else if (sample < -0x8000)
				sample = -0x8000;
			*dst = (signed short) bswap_16((signed short) sample);
		}
		if (!--size)
			return;
		src = (signed short *) ((char *)src + src_step);
		dst = (signed short *) ((char *)dst + dst_step);
		sum = (signed int *)   ((char *)sum + sum_step);
	}
}

static void generic_remix_areas_16_swap(unsigned int size,
				        volatile signed short *dst,
				        signed short *src,
				        volatile signed int *sum,
				        size_t dst_step,
				        size_t src_step,
				        size_t sum_step)
{
	register signed int sample;

	for (;;) {
		sample = (signed short) bswap_16(*src);
		if (! *dst) {
			*sum = -sample;
			*dst = (signed short) bswap_16((signed short) -sample);
		} else {
			*sum = sample = *sum - sample;
			if (sample > 0x7fff)
				sample = 0x7fff;
			else if (sample < -0x8000)
				sample = -0x8000;
			*dst = (signed short) bswap_16((signed short) sample);
		}
		if (!--size)
			return;
		src = (signed short *) ((char *)src + src_step);
		dst = (signed short *) ((char *)dst + dst_step);
		sum = (signed int *)   ((char *)sum + sum_step);
	}
}

static void generic_mix_areas_32_swap(unsigned int size,
				      volatile signed int *dst,
				      signed int *src,
				      volatile signed int *sum,
				      size_t dst_step,
				      size_t src_step,
				      size_t sum_step)
{
	register signed int sample;

	for (;;) {
		sample = bswap_32(*src) >> 8;
		if (! *dst) {
			*sum = sample;
			*dst = *src;
		} else {
			sample += *sum;
			*sum = sample;
			if (sample > 0x7fffff)
				sample = 0x7fffffff;
			else if (sample < -0x800000)
				sample = -0x80000000;
			else
				sample *= 256;
			*dst = bswap_32(sample);
		}
		if (!--size)
			return;
		src = (signed int *) ((char *)src + src_step);
		dst = (signed int *) ((char *)dst + dst_step);
		sum = (signed int *) ((char *)sum + sum_step);
	}
}

static void generic_remix_areas_32_swap(unsigned int size,
				        volatile signed int *dst,
				        signed int *src,
				        volatile signed int *sum,
				        size_t dst_step,
				        size_t src_step,
				        size_t sum_step)
{
	register signed int sample;

	for (;;) {
		sample = bswap_32(*src) >> 8;
		if (! *dst) {
			*sum = -sample;
			*dst = bswap_32(-sample);
		} else {
			*sum = sample = *sum - sample;
			if (sample > 0x7fffff)
				sample = 0x7fffffff;
			else if (sample < -0x800000)
				sample = -0x80000000;
			else
				sample *= 256;
			*dst = bswap_32(sample);
		}
		if (!--size)
			return;
		src = (signed int *) ((char *)src + src_step);
		dst = (signed int *) ((char *)dst + dst_step);
		sum = (signed int *) ((char *)sum + sum_step);
	}
}

/* always little endian */
static void generic_mix_areas_24(unsigned int size,
				 volatile unsigned char *dst,
				 unsigned char *src,
				 volatile signed int *sum,
				 size_t dst_step,
				 size_t src_step,
				 size_t sum_step)
{
	register signed int sample;

	for (;;) {
		sample = src[0] | (src[1] << 8) | (((signed char *)src)[2] << 16);
		if (!(dst[0] | dst[1] | dst[2])) {
			*sum = sample;
		} else {
			sample += *sum;
			*sum = sample;
			if (sample > 0x7fffff)
				sample = 0x7fffff;
			else if (sample < -0x800000)
				sample = -0x800000;
		}
		dst[0] = sample;
		dst[1] = sample >> 8;
		dst[2] = sample >> 16;
		if (!--size)
			return;
		dst += dst_step;
		src += src_step;
		sum = (signed int *) ((char *)sum + sum_step);
	}
}

static void generic_remix_areas_24(unsigned int size,
				   volatile unsigned char *dst,
				   unsigned char *src,
				   volatile signed int *sum,
				   size_t dst_step,
				   size_t src_step,
				   size_t sum_step)
{
	register signed int sample;

	for (;;) {
		sample = src[0] | (src[1] << 8) | (((signed char *)src)[2] << 16);
		if (!(dst[0] | dst[1] | dst[2])) {
			sample = -sample;
			*sum = sample;
		} else {
			*sum = sample = *sum - sample;
			if (sample > 0x7fffff)
				sample = 0x7fffff;
			else if (sample < -0x800000)
				sample = -0x800000;
		}
		dst[0] = sample;
		dst[1] = sample >> 8;
		dst[2] = sample >> 16;
		if (!--size)
			return;
		dst += dst_step;
		src += src_step;
		sum = (signed int *) ((char *)sum + sum_step);
	}
}

static void generic_mix_areas_u8(unsigned int size,
				 volatile unsigned char *dst,
				 unsigned char *src,
				 volatile signed int *sum,
				 size_t dst_step,
				 size_t src_step,
				 size_t sum_step)
{
	for (;;) {
		register int sample = *src - 0x80;
		if (*dst == 0x80) {
			*sum = sample;
		} else {
			sample += *sum;
			*sum = sample;
			if (sample > 0x7f)
				sample = 0x7f;
			else if (sample < -0x80)
				sample = -0x80;
		}
		*dst = sample + 0x80;
		if (!--size)
			return;
		dst += dst_step;
		src += src_step;
		sum = (signed int *) ((char *)sum + sum_step);
	}
}

static void generic_remix_areas_u8(unsigned int size,
				   volatile unsigned char *dst,
				   unsigned char *src,
				   volatile signed int *sum,
				   size_t dst_step,
				   size_t src_step,
				   size_t sum_step)
{
	for (;;) {
		register int sample = *src - 0x80;
		if (*dst == 0x80) {
			sample = -sample;
			*sum = sample;
		} else {
			*sum = sample = *sum - sample;
			if (sample > 0x7f)
				sample = 0x7f;
			else if (sample < -0x80)
				sample = -0x80;
		}
		*dst = sample + 0x80;
		if (!--size)
			return;
		dst += dst_step;
		src += src_step;
		sum = (signed int *) ((char *)sum + sum_step);
	}
}


static void generic_mix_select_callbacks(snd_pcm_direct_t *dmix)
{
	if (snd_pcm_format_cpu_endian(dmix->shmptr->s.format)) {
		dmix->u.dmix.mix_areas_16 = generic_mix_areas_16_native;
		dmix->u.dmix.mix_areas_32 = generic_mix_areas_32_native;
		dmix->u.dmix.remix_areas_16 = generic_remix_areas_16_native;
		dmix->u.dmix.remix_areas_32 = generic_remix_areas_32_native;
	} else {
		dmix->u.dmix.mix_areas_16 = generic_mix_areas_16_swap;
		dmix->u.dmix.mix_areas_32 = generic_mix_areas_32_swap;
		dmix->u.dmix.remix_areas_16 = generic_remix_areas_16_swap;
		dmix->u.dmix.remix_areas_32 = generic_remix_areas_32_swap;
	}
	dmix->u.dmix.mix_areas_24 = generic_mix_areas_24;
	dmix->u.dmix.mix_areas_u8 = generic_mix_areas_u8;
	dmix->u.dmix.remix_areas_24 = generic_remix_areas_24;
	dmix->u.dmix.remix_areas_u8 = generic_remix_areas_u8;
}

#endif
