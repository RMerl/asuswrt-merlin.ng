/*
 * Copyright (C) 2007 Ben Skeggs.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE COPYRIGHT OWNER(S) AND/OR ITS SUPPLIERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#ifndef __NOUVEAU_DMA_H__
#define __NOUVEAU_DMA_H__

#include "nouveau_bo.h"
#include "nouveau_chan.h"

int nouveau_dma_wait(struct nouveau_channel *, int slots, int size);
void nv50_dma_push(struct nouveau_channel *, struct nouveau_bo *,
		   int delta, int length);

/*
 * There's a hw race condition where you can't jump to your PUT offset,
 * to avoid this we jump to offset + SKIPS and fill the difference with
 * NOPs.
 *
 * xf86-video-nv configures the DMA fetch size to 32 bytes, and uses
 * a SKIPS value of 8.  Lets assume that the race condition is to do
 * with writing into the fetch area, we configure a fetch size of 128
 * bytes so we need a larger SKIPS value.
 */
#define NOUVEAU_DMA_SKIPS (128 / 4)

/* Hardcoded object assignments to subchannels (subchannel id). */
enum {
	NvSubCtxSurf2D  = 0,
	NvSubSw		= 1,
	NvSubImageBlit  = 2,
	NvSubGdiRect    = 3,

	NvSub2D		= 3, /* DO NOT CHANGE - hardcoded for kepler gr fifo */
	NvSubCopy	= 4, /* DO NOT CHANGE - hardcoded for kepler gr fifo */
	FermiSw		= 5, /* DO NOT CHANGE (well.. 6/7 will work...) */
};

/* Object handles - for stuff that's doesn't use handle == oclass. */
enum {
	NvDmaFB		= 0x80000002,
	NvDmaTT		= 0x80000003,
	NvNotify0       = 0x80000006,
	NvSema		= 0x8000000f,
	NvEvoSema0	= 0x80000010,
	NvEvoSema1	= 0x80000011,
};

#define NV_MEMORY_TO_MEMORY_FORMAT                                    0x00000039
#define NV_MEMORY_TO_MEMORY_FORMAT_NAME                               0x00000000
#define NV_MEMORY_TO_MEMORY_FORMAT_SET_REF                            0x00000050
#define NV_MEMORY_TO_MEMORY_FORMAT_NOP                                0x00000100
#define NV_MEMORY_TO_MEMORY_FORMAT_NOTIFY                             0x00000104
#define NV_MEMORY_TO_MEMORY_FORMAT_NOTIFY_STYLE_WRITE                 0x00000000
#define NV_MEMORY_TO_MEMORY_FORMAT_NOTIFY_STYLE_WRITE_LE_AWAKEN       0x00000001
#define NV_MEMORY_TO_MEMORY_FORMAT_DMA_NOTIFY                         0x00000180
#define NV_MEMORY_TO_MEMORY_FORMAT_DMA_SOURCE                         0x00000184
#define NV_MEMORY_TO_MEMORY_FORMAT_OFFSET_IN                          0x0000030c

#define NV50_MEMORY_TO_MEMORY_FORMAT                                  0x00005039
#define NV50_MEMORY_TO_MEMORY_FORMAT_UNK200                           0x00000200
#define NV50_MEMORY_TO_MEMORY_FORMAT_UNK21C                           0x0000021c
#define NV50_MEMORY_TO_MEMORY_FORMAT_OFFSET_IN_HIGH                   0x00000238
#define NV50_MEMORY_TO_MEMORY_FORMAT_OFFSET_OUT_HIGH                  0x0000023c

static __must_check inline int
RING_SPACE(struct nouveau_channel *chan, int size)
{
	int ret;

	ret = nouveau_dma_wait(chan, 1, size);
	if (ret)
		return ret;

	chan->dma.free -= size;
	return 0;
}

static inline void
OUT_RING(struct nouveau_channel *chan, int data)
{
	nouveau_bo_wr32(chan->push.buffer, chan->dma.cur++, data);
}

extern void
OUT_RINGp(struct nouveau_channel *chan, const void *data, unsigned nr_dwords);

static inline void
BEGIN_NV04(struct nouveau_channel *chan, int subc, int mthd, int size)
{
	OUT_RING(chan, 0x00000000 | (subc << 13) | (size << 18) | mthd);
}

static inline void
BEGIN_NI04(struct nouveau_channel *chan, int subc, int mthd, int size)
{
	OUT_RING(chan, 0x40000000 | (subc << 13) | (size << 18) | mthd);
}

static inline void
BEGIN_NVC0(struct nouveau_channel *chan, int subc, int mthd, int size)
{
	OUT_RING(chan, 0x20000000 | (size << 16) | (subc << 13) | (mthd >> 2));
}

static inline void
BEGIN_NIC0(struct nouveau_channel *chan, int subc, int mthd, int size)
{
	OUT_RING(chan, 0x60000000 | (size << 16) | (subc << 13) | (mthd >> 2));
}

static inline void
BEGIN_IMC0(struct nouveau_channel *chan, int subc, int mthd, u16 data)
{
	OUT_RING(chan, 0x80000000 | (data << 16) | (subc << 13) | (mthd >> 2));
}

#define WRITE_PUT(val) do {                                                    \
	mb();                                                   \
	nouveau_bo_rd32(chan->push.buffer, 0);                                 \
	nvif_wr32(chan, chan->user_put, ((val) << 2) + chan->push.vma.offset); \
} while (0)

static inline void
FIRE_RING(struct nouveau_channel *chan)
{
	if (chan->dma.cur == chan->dma.put)
		return;
	chan->accel_done = true;

	if (chan->dma.ib_max) {
		nv50_dma_push(chan, chan->push.buffer, chan->dma.put << 2,
			      (chan->dma.cur - chan->dma.put) << 2);
	} else {
		WRITE_PUT(chan->dma.cur);
	}

	chan->dma.put = chan->dma.cur;
}

static inline void
WIND_RING(struct nouveau_channel *chan)
{
	chan->dma.cur = chan->dma.put;
}

/* FIFO methods */
#define NV01_SUBCHAN_OBJECT                                          0x00000000
#define NV84_SUBCHAN_SEMAPHORE_ADDRESS_HIGH                          0x00000010
#define NV84_SUBCHAN_SEMAPHORE_ADDRESS_LOW                           0x00000014
#define NV84_SUBCHAN_SEMAPHORE_SEQUENCE                              0x00000018
#define NV84_SUBCHAN_SEMAPHORE_TRIGGER                               0x0000001c
#define NV84_SUBCHAN_SEMAPHORE_TRIGGER_ACQUIRE_EQUAL                 0x00000001
#define NV84_SUBCHAN_SEMAPHORE_TRIGGER_WRITE_LONG                    0x00000002
#define NV84_SUBCHAN_SEMAPHORE_TRIGGER_ACQUIRE_GEQUAL                0x00000004
#define NVC0_SUBCHAN_SEMAPHORE_TRIGGER_YIELD                         0x00001000
#define NV84_SUBCHAN_UEVENT                                          0x00000020
#define NV84_SUBCHAN_WRCACHE_FLUSH                                   0x00000024
#define NV10_SUBCHAN_REF_CNT                                         0x00000050
#define NV11_SUBCHAN_DMA_SEMAPHORE                                   0x00000060
#define NV11_SUBCHAN_SEMAPHORE_OFFSET                                0x00000064
#define NV11_SUBCHAN_SEMAPHORE_ACQUIRE                               0x00000068
#define NV11_SUBCHAN_SEMAPHORE_RELEASE                               0x0000006c
#define NV40_SUBCHAN_YIELD                                           0x00000080

/* NV_SW object class */
#define NV_SW_DMA_VBLSEM                                             0x0000018c
#define NV_SW_VBLSEM_OFFSET                                          0x00000400
#define NV_SW_VBLSEM_RELEASE_VALUE                                   0x00000404
#define NV_SW_VBLSEM_RELEASE                                         0x00000408
#define NV_SW_PAGE_FLIP                                              0x00000500

#endif
