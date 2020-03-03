/*
 * Applied Micro X-Gene SoC DMA engine Driver
 *
 * Copyright (c) 2015, Applied Micro Circuits Corporation
 * Authors: Rameshwar Prasad Sahu <rsahu@apm.com>
 *	    Loc Ho <lho@apm.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * NOTE: PM support is currently not available.
 */

#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/dma-mapping.h>
#include <linux/dmaengine.h>
#include <linux/dmapool.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/of_device.h>

#include "dmaengine.h"

/* X-Gene DMA ring csr registers and bit definations */
#define XGENE_DMA_RING_CONFIG			0x04
#define XGENE_DMA_RING_ENABLE			BIT(31)
#define XGENE_DMA_RING_ID			0x08
#define XGENE_DMA_RING_ID_SETUP(v)		((v) | BIT(31))
#define XGENE_DMA_RING_ID_BUF			0x0C
#define XGENE_DMA_RING_ID_BUF_SETUP(v)		(((v) << 9) | BIT(21))
#define XGENE_DMA_RING_THRESLD0_SET1		0x30
#define XGENE_DMA_RING_THRESLD0_SET1_VAL	0X64
#define XGENE_DMA_RING_THRESLD1_SET1		0x34
#define XGENE_DMA_RING_THRESLD1_SET1_VAL	0xC8
#define XGENE_DMA_RING_HYSTERESIS		0x68
#define XGENE_DMA_RING_HYSTERESIS_VAL		0xFFFFFFFF
#define XGENE_DMA_RING_STATE			0x6C
#define XGENE_DMA_RING_STATE_WR_BASE		0x70
#define XGENE_DMA_RING_NE_INT_MODE		0x017C
#define XGENE_DMA_RING_NE_INT_MODE_SET(m, v)	\
	((m) = ((m) & ~BIT(31 - (v))) | BIT(31 - (v)))
#define XGENE_DMA_RING_NE_INT_MODE_RESET(m, v)	\
	((m) &= (~BIT(31 - (v))))
#define XGENE_DMA_RING_CLKEN			0xC208
#define XGENE_DMA_RING_SRST			0xC200
#define XGENE_DMA_RING_MEM_RAM_SHUTDOWN		0xD070
#define XGENE_DMA_RING_BLK_MEM_RDY		0xD074
#define XGENE_DMA_RING_BLK_MEM_RDY_VAL		0xFFFFFFFF
#define XGENE_DMA_RING_DESC_CNT(v)		(((v) & 0x0001FFFE) >> 1)
#define XGENE_DMA_RING_ID_GET(owner, num)	(((owner) << 6) | (num))
#define XGENE_DMA_RING_DST_ID(v)		((1 << 10) | (v))
#define XGENE_DMA_RING_CMD_OFFSET		0x2C
#define XGENE_DMA_RING_CMD_BASE_OFFSET(v)	((v) << 6)
#define XGENE_DMA_RING_COHERENT_SET(m)		\
	(((u32 *)(m))[2] |= BIT(4))
#define XGENE_DMA_RING_ADDRL_SET(m, v)		\
	(((u32 *)(m))[2] |= (((v) >> 8) << 5))
#define XGENE_DMA_RING_ADDRH_SET(m, v)		\
	(((u32 *)(m))[3] |= ((v) >> 35))
#define XGENE_DMA_RING_ACCEPTLERR_SET(m)	\
	(((u32 *)(m))[3] |= BIT(19))
#define XGENE_DMA_RING_SIZE_SET(m, v)		\
	(((u32 *)(m))[3] |= ((v) << 23))
#define XGENE_DMA_RING_RECOMBBUF_SET(m)		\
	(((u32 *)(m))[3] |= BIT(27))
#define XGENE_DMA_RING_RECOMTIMEOUTL_SET(m)	\
	(((u32 *)(m))[3] |= (0x7 << 28))
#define XGENE_DMA_RING_RECOMTIMEOUTH_SET(m)	\
	(((u32 *)(m))[4] |= 0x3)
#define XGENE_DMA_RING_SELTHRSH_SET(m)		\
	(((u32 *)(m))[4] |= BIT(3))
#define XGENE_DMA_RING_TYPE_SET(m, v)		\
	(((u32 *)(m))[4] |= ((v) << 19))

/* X-Gene DMA device csr registers and bit definitions */
#define XGENE_DMA_IPBRR				0x0
#define XGENE_DMA_DEV_ID_RD(v)			((v) & 0x00000FFF)
#define XGENE_DMA_BUS_ID_RD(v)			(((v) >> 12) & 3)
#define XGENE_DMA_REV_NO_RD(v)			(((v) >> 14) & 3)
#define XGENE_DMA_GCR				0x10
#define XGENE_DMA_CH_SETUP(v)			\
	((v) = ((v) & ~0x000FFFFF) | 0x000AAFFF)
#define XGENE_DMA_ENABLE(v)			((v) |= BIT(31))
#define XGENE_DMA_DISABLE(v)			((v) &= ~BIT(31))
#define XGENE_DMA_RAID6_CONT			0x14
#define XGENE_DMA_RAID6_MULTI_CTRL(v)		((v) << 24)
#define XGENE_DMA_INT				0x70
#define XGENE_DMA_INT_MASK			0x74
#define XGENE_DMA_INT_ALL_MASK			0xFFFFFFFF
#define XGENE_DMA_INT_ALL_UNMASK		0x0
#define XGENE_DMA_INT_MASK_SHIFT		0x14
#define XGENE_DMA_RING_INT0_MASK		0x90A0
#define XGENE_DMA_RING_INT1_MASK		0x90A8
#define XGENE_DMA_RING_INT2_MASK		0x90B0
#define XGENE_DMA_RING_INT3_MASK		0x90B8
#define XGENE_DMA_RING_INT4_MASK		0x90C0
#define XGENE_DMA_CFG_RING_WQ_ASSOC		0x90E0
#define XGENE_DMA_ASSOC_RING_MNGR1		0xFFFFFFFF
#define XGENE_DMA_MEM_RAM_SHUTDOWN		0xD070
#define XGENE_DMA_BLK_MEM_RDY			0xD074
#define XGENE_DMA_BLK_MEM_RDY_VAL		0xFFFFFFFF

/* X-Gene SoC EFUSE csr register and bit defination */
#define XGENE_SOC_JTAG1_SHADOW			0x18
#define XGENE_DMA_PQ_DISABLE_MASK		BIT(13)

/* X-Gene DMA Descriptor format */
#define XGENE_DMA_DESC_NV_BIT			BIT_ULL(50)
#define XGENE_DMA_DESC_IN_BIT			BIT_ULL(55)
#define XGENE_DMA_DESC_C_BIT			BIT_ULL(63)
#define XGENE_DMA_DESC_DR_BIT			BIT_ULL(61)
#define XGENE_DMA_DESC_ELERR_POS		46
#define XGENE_DMA_DESC_RTYPE_POS		56
#define XGENE_DMA_DESC_LERR_POS			60
#define XGENE_DMA_DESC_FLYBY_POS		4
#define XGENE_DMA_DESC_BUFLEN_POS		48
#define XGENE_DMA_DESC_HOENQ_NUM_POS		48

#define XGENE_DMA_DESC_NV_SET(m)		\
	(((u64 *)(m))[0] |= XGENE_DMA_DESC_NV_BIT)
#define XGENE_DMA_DESC_IN_SET(m)		\
	(((u64 *)(m))[0] |= XGENE_DMA_DESC_IN_BIT)
#define XGENE_DMA_DESC_RTYPE_SET(m, v)		\
	(((u64 *)(m))[0] |= ((u64)(v) << XGENE_DMA_DESC_RTYPE_POS))
#define XGENE_DMA_DESC_BUFADDR_SET(m, v)	\
	(((u64 *)(m))[0] |= (v))
#define XGENE_DMA_DESC_BUFLEN_SET(m, v)		\
	(((u64 *)(m))[0] |= ((u64)(v) << XGENE_DMA_DESC_BUFLEN_POS))
#define XGENE_DMA_DESC_C_SET(m)			\
	(((u64 *)(m))[1] |= XGENE_DMA_DESC_C_BIT)
#define XGENE_DMA_DESC_FLYBY_SET(m, v)		\
	(((u64 *)(m))[2] |= ((v) << XGENE_DMA_DESC_FLYBY_POS))
#define XGENE_DMA_DESC_MULTI_SET(m, v, i)	\
	(((u64 *)(m))[2] |= ((u64)(v) << (((i) + 1) * 8)))
#define XGENE_DMA_DESC_DR_SET(m)		\
	(((u64 *)(m))[2] |= XGENE_DMA_DESC_DR_BIT)
#define XGENE_DMA_DESC_DST_ADDR_SET(m, v)	\
	(((u64 *)(m))[3] |= (v))
#define XGENE_DMA_DESC_H0ENQ_NUM_SET(m, v)	\
	(((u64 *)(m))[3] |= ((u64)(v) << XGENE_DMA_DESC_HOENQ_NUM_POS))
#define XGENE_DMA_DESC_ELERR_RD(m)		\
	(((m) >> XGENE_DMA_DESC_ELERR_POS) & 0x3)
#define XGENE_DMA_DESC_LERR_RD(m)		\
	(((m) >> XGENE_DMA_DESC_LERR_POS) & 0x7)
#define XGENE_DMA_DESC_STATUS(elerr, lerr)	\
	(((elerr) << 4) | (lerr))

/* X-Gene DMA descriptor empty s/w signature */
#define XGENE_DMA_DESC_EMPTY_INDEX		0
#define XGENE_DMA_DESC_EMPTY_SIGNATURE		~0ULL
#define XGENE_DMA_DESC_SET_EMPTY(m)		\
	(((u64 *)(m))[XGENE_DMA_DESC_EMPTY_INDEX] =	\
	 XGENE_DMA_DESC_EMPTY_SIGNATURE)
#define XGENE_DMA_DESC_IS_EMPTY(m)		\
	(((u64 *)(m))[XGENE_DMA_DESC_EMPTY_INDEX] ==	\
	 XGENE_DMA_DESC_EMPTY_SIGNATURE)

/* X-Gene DMA configurable parameters defines */
#define XGENE_DMA_RING_NUM		512
#define XGENE_DMA_BUFNUM		0x0
#define XGENE_DMA_CPU_BUFNUM		0x18
#define XGENE_DMA_RING_OWNER_DMA	0x03
#define XGENE_DMA_RING_OWNER_CPU	0x0F
#define XGENE_DMA_RING_TYPE_REGULAR	0x01
#define XGENE_DMA_RING_WQ_DESC_SIZE	32	/* 32 Bytes */
#define XGENE_DMA_RING_NUM_CONFIG	5
#define XGENE_DMA_MAX_CHANNEL		4
#define XGENE_DMA_XOR_CHANNEL		0
#define XGENE_DMA_PQ_CHANNEL		1
#define XGENE_DMA_MAX_BYTE_CNT		0x4000	/* 16 KB */
#define XGENE_DMA_MAX_64B_DESC_BYTE_CNT	0x14000	/* 80 KB */
#define XGENE_DMA_XOR_ALIGNMENT		6	/* 64 Bytes */
#define XGENE_DMA_MAX_XOR_SRC		5
#define XGENE_DMA_16K_BUFFER_LEN_CODE	0x0
#define XGENE_DMA_INVALID_LEN_CODE	0x7800

/* X-Gene DMA descriptor error codes */
#define ERR_DESC_AXI			0x01
#define ERR_BAD_DESC			0x02
#define ERR_READ_DATA_AXI		0x03
#define ERR_WRITE_DATA_AXI		0x04
#define ERR_FBP_TIMEOUT			0x05
#define ERR_ECC				0x06
#define ERR_DIFF_SIZE			0x08
#define ERR_SCT_GAT_LEN			0x09
#define ERR_CRC_ERR			0x11
#define ERR_CHKSUM			0x12
#define ERR_DIF				0x13

/* X-Gene DMA error interrupt codes */
#define ERR_DIF_SIZE_INT		0x0
#define ERR_GS_ERR_INT			0x1
#define ERR_FPB_TIMEO_INT		0x2
#define ERR_WFIFO_OVF_INT		0x3
#define ERR_RFIFO_OVF_INT		0x4
#define ERR_WR_TIMEO_INT		0x5
#define ERR_RD_TIMEO_INT		0x6
#define ERR_WR_ERR_INT			0x7
#define ERR_RD_ERR_INT			0x8
#define ERR_BAD_DESC_INT		0x9
#define ERR_DESC_DST_INT		0xA
#define ERR_DESC_SRC_INT		0xB

/* X-Gene DMA flyby operation code */
#define FLYBY_2SRC_XOR			0x8
#define FLYBY_3SRC_XOR			0x9
#define FLYBY_4SRC_XOR			0xA
#define FLYBY_5SRC_XOR			0xB

/* X-Gene DMA SW descriptor flags */
#define XGENE_DMA_FLAG_64B_DESC		BIT(0)

/* Define to dump X-Gene DMA descriptor */
#define XGENE_DMA_DESC_DUMP(desc, m)	\
	print_hex_dump(KERN_ERR, (m),	\
			DUMP_PREFIX_ADDRESS, 16, 8, (desc), 32, 0)

#define to_dma_desc_sw(tx)		\
	container_of(tx, struct xgene_dma_desc_sw, tx)
#define to_dma_chan(dchan)		\
	container_of(dchan, struct xgene_dma_chan, dma_chan)

#define chan_dbg(chan, fmt, arg...)	\
	dev_dbg(chan->dev, "%s: " fmt, chan->name, ##arg)
#define chan_err(chan, fmt, arg...)	\
	dev_err(chan->dev, "%s: " fmt, chan->name, ##arg)

struct xgene_dma_desc_hw {
	u64 m0;
	u64 m1;
	u64 m2;
	u64 m3;
};

enum xgene_dma_ring_cfgsize {
	XGENE_DMA_RING_CFG_SIZE_512B,
	XGENE_DMA_RING_CFG_SIZE_2KB,
	XGENE_DMA_RING_CFG_SIZE_16KB,
	XGENE_DMA_RING_CFG_SIZE_64KB,
	XGENE_DMA_RING_CFG_SIZE_512KB,
	XGENE_DMA_RING_CFG_SIZE_INVALID
};

struct xgene_dma_ring {
	struct xgene_dma *pdma;
	u8 buf_num;
	u16 id;
	u16 num;
	u16 head;
	u16 owner;
	u16 slots;
	u16 dst_ring_num;
	u32 size;
	void __iomem *cmd;
	void __iomem *cmd_base;
	dma_addr_t desc_paddr;
	u32 state[XGENE_DMA_RING_NUM_CONFIG];
	enum xgene_dma_ring_cfgsize cfgsize;
	union {
		void *desc_vaddr;
		struct xgene_dma_desc_hw *desc_hw;
	};
};

struct xgene_dma_desc_sw {
	struct xgene_dma_desc_hw desc1;
	struct xgene_dma_desc_hw desc2;
	u32 flags;
	struct list_head node;
	struct list_head tx_list;
	struct dma_async_tx_descriptor tx;
};

/**
 * struct xgene_dma_chan - internal representation of an X-Gene DMA channel
 * @dma_chan: dmaengine channel object member
 * @pdma: X-Gene DMA device structure reference
 * @dev: struct device reference for dma mapping api
 * @id: raw id of this channel
 * @rx_irq: channel IRQ
 * @name: name of X-Gene DMA channel
 * @lock: serializes enqueue/dequeue operations to the descriptor pool
 * @pending: number of transaction request pushed to DMA controller for
 *	execution, but still waiting for completion,
 * @max_outstanding: max number of outstanding request we can push to channel
 * @ld_pending: descriptors which are queued to run, but have not yet been
 *	submitted to the hardware for execution
 * @ld_running: descriptors which are currently being executing by the hardware
 * @ld_completed: descriptors which have finished execution by the hardware.
 *	These descriptors have already had their cleanup actions run. They
 *	are waiting for the ACK bit to be set by the async tx API.
 * @desc_pool: descriptor pool for DMA operations
 * @tasklet: bottom half where all completed descriptors cleans
 * @tx_ring: transmit ring descriptor that we use to prepare actual
 *	descriptors for further executions
 * @rx_ring: receive ring descriptor that we use to get completed DMA
 *	descriptors during cleanup time
 */
struct xgene_dma_chan {
	struct dma_chan dma_chan;
	struct xgene_dma *pdma;
	struct device *dev;
	int id;
	int rx_irq;
	char name[10];
	spinlock_t lock;
	int pending;
	int max_outstanding;
	struct list_head ld_pending;
	struct list_head ld_running;
	struct list_head ld_completed;
	struct dma_pool *desc_pool;
	struct tasklet_struct tasklet;
	struct xgene_dma_ring tx_ring;
	struct xgene_dma_ring rx_ring;
};

/**
 * struct xgene_dma - internal representation of an X-Gene DMA device
 * @err_irq: DMA error irq number
 * @ring_num: start id number for DMA ring
 * @csr_dma: base for DMA register access
 * @csr_ring: base for DMA ring register access
 * @csr_ring_cmd: base for DMA ring command register access
 * @csr_efuse: base for efuse register access
 * @dma_dev: embedded struct dma_device
 * @chan: reference to X-Gene DMA channels
 */
struct xgene_dma {
	struct device *dev;
	struct clk *clk;
	int err_irq;
	int ring_num;
	void __iomem *csr_dma;
	void __iomem *csr_ring;
	void __iomem *csr_ring_cmd;
	void __iomem *csr_efuse;
	struct dma_device dma_dev[XGENE_DMA_MAX_CHANNEL];
	struct xgene_dma_chan chan[XGENE_DMA_MAX_CHANNEL];
};

static const char * const xgene_dma_desc_err[] = {
	[ERR_DESC_AXI] = "AXI error when reading src/dst link list",
	[ERR_BAD_DESC] = "ERR or El_ERR fields not set to zero in desc",
	[ERR_READ_DATA_AXI] = "AXI error when reading data",
	[ERR_WRITE_DATA_AXI] = "AXI error when writing data",
	[ERR_FBP_TIMEOUT] = "Timeout on bufpool fetch",
	[ERR_ECC] = "ECC double bit error",
	[ERR_DIFF_SIZE] = "Bufpool too small to hold all the DIF result",
	[ERR_SCT_GAT_LEN] = "Gather and scatter data length not same",
	[ERR_CRC_ERR] = "CRC error",
	[ERR_CHKSUM] = "Checksum error",
	[ERR_DIF] = "DIF error",
};

static const char * const xgene_dma_err[] = {
	[ERR_DIF_SIZE_INT] = "DIF size error",
	[ERR_GS_ERR_INT] = "Gather scatter not same size error",
	[ERR_FPB_TIMEO_INT] = "Free pool time out error",
	[ERR_WFIFO_OVF_INT] = "Write FIFO over flow error",
	[ERR_RFIFO_OVF_INT] = "Read FIFO over flow error",
	[ERR_WR_TIMEO_INT] = "Write time out error",
	[ERR_RD_TIMEO_INT] = "Read time out error",
	[ERR_WR_ERR_INT] = "HBF bus write error",
	[ERR_RD_ERR_INT] = "HBF bus read error",
	[ERR_BAD_DESC_INT] = "Ring descriptor HE0 not set error",
	[ERR_DESC_DST_INT] = "HFB reading dst link address error",
	[ERR_DESC_SRC_INT] = "HFB reading src link address error",
};

static bool is_pq_enabled(struct xgene_dma *pdma)
{
	u32 val;

	val = ioread32(pdma->csr_efuse + XGENE_SOC_JTAG1_SHADOW);
	return !(val & XGENE_DMA_PQ_DISABLE_MASK);
}

static void xgene_dma_cpu_to_le64(u64 *desc, int count)
{
	int i;

	for (i = 0; i < count; i++)
		desc[i] = cpu_to_le64(desc[i]);
}

static u16 xgene_dma_encode_len(u32 len)
{
	return (len < XGENE_DMA_MAX_BYTE_CNT) ?
		len : XGENE_DMA_16K_BUFFER_LEN_CODE;
}

static u8 xgene_dma_encode_xor_flyby(u32 src_cnt)
{
	static u8 flyby_type[] = {
		FLYBY_2SRC_XOR, /* Dummy */
		FLYBY_2SRC_XOR, /* Dummy */
		FLYBY_2SRC_XOR,
		FLYBY_3SRC_XOR,
		FLYBY_4SRC_XOR,
		FLYBY_5SRC_XOR
	};

	return flyby_type[src_cnt];
}

static u32 xgene_dma_ring_desc_cnt(struct xgene_dma_ring *ring)
{
	u32 __iomem *cmd_base = ring->cmd_base;
	u32 ring_state = ioread32(&cmd_base[1]);

	return XGENE_DMA_RING_DESC_CNT(ring_state);
}

static void xgene_dma_set_src_buffer(void *ext8, size_t *len,
				     dma_addr_t *paddr)
{
	size_t nbytes = (*len < XGENE_DMA_MAX_BYTE_CNT) ?
			*len : XGENE_DMA_MAX_BYTE_CNT;

	XGENE_DMA_DESC_BUFADDR_SET(ext8, *paddr);
	XGENE_DMA_DESC_BUFLEN_SET(ext8, xgene_dma_encode_len(nbytes));
	*len -= nbytes;
	*paddr += nbytes;
}

static void xgene_dma_invalidate_buffer(void *ext8)
{
	XGENE_DMA_DESC_BUFLEN_SET(ext8, XGENE_DMA_INVALID_LEN_CODE);
}

static void *xgene_dma_lookup_ext8(u64 *desc, int idx)
{
	return (idx % 2) ? (desc + idx - 1) : (desc + idx + 1);
}

static void xgene_dma_init_desc(void *desc, u16 dst_ring_num)
{
	XGENE_DMA_DESC_C_SET(desc); /* Coherent IO */
	XGENE_DMA_DESC_IN_SET(desc);
	XGENE_DMA_DESC_H0ENQ_NUM_SET(desc, dst_ring_num);
	XGENE_DMA_DESC_RTYPE_SET(desc, XGENE_DMA_RING_OWNER_DMA);
}

static void xgene_dma_prep_cpy_desc(struct xgene_dma_chan *chan,
				    struct xgene_dma_desc_sw *desc_sw,
				    dma_addr_t dst, dma_addr_t src,
				    size_t len)
{
	void *desc1, *desc2;
	int i;

	/* Get 1st descriptor */
	desc1 = &desc_sw->desc1;
	xgene_dma_init_desc(desc1, chan->tx_ring.dst_ring_num);

	/* Set destination address */
	XGENE_DMA_DESC_DR_SET(desc1);
	XGENE_DMA_DESC_DST_ADDR_SET(desc1, dst);

	/* Set 1st source address */
	xgene_dma_set_src_buffer(desc1 + 8, &len, &src);

	if (len <= 0) {
		desc2 = NULL;
		goto skip_additional_src;
	}

	/*
	 * We need to split this source buffer,
	 * and need to use 2nd descriptor
	 */
	desc2 = &desc_sw->desc2;
	XGENE_DMA_DESC_NV_SET(desc1);

	/* Set 2nd to 5th source address */
	for (i = 0; i < 4 && len; i++)
		xgene_dma_set_src_buffer(xgene_dma_lookup_ext8(desc2, i),
					 &len, &src);

	/* Invalidate unused source address field */
	for (; i < 4; i++)
		xgene_dma_invalidate_buffer(xgene_dma_lookup_ext8(desc2, i));

	/* Updated flag that we have prepared 64B descriptor */
	desc_sw->flags |= XGENE_DMA_FLAG_64B_DESC;

skip_additional_src:
	/* Hardware stores descriptor in little endian format */
	xgene_dma_cpu_to_le64(desc1, 4);
	if (desc2)
		xgene_dma_cpu_to_le64(desc2, 4);
}

static void xgene_dma_prep_xor_desc(struct xgene_dma_chan *chan,
				    struct xgene_dma_desc_sw *desc_sw,
				    dma_addr_t *dst, dma_addr_t *src,
				    u32 src_cnt, size_t *nbytes,
				    const u8 *scf)
{
	void *desc1, *desc2;
	size_t len = *nbytes;
	int i;

	desc1 = &desc_sw->desc1;
	desc2 = &desc_sw->desc2;

	/* Initialize DMA descriptor */
	xgene_dma_init_desc(desc1, chan->tx_ring.dst_ring_num);

	/* Set destination address */
	XGENE_DMA_DESC_DR_SET(desc1);
	XGENE_DMA_DESC_DST_ADDR_SET(desc1, *dst);

	/* We have multiple source addresses, so need to set NV bit*/
	XGENE_DMA_DESC_NV_SET(desc1);

	/* Set flyby opcode */
	XGENE_DMA_DESC_FLYBY_SET(desc1, xgene_dma_encode_xor_flyby(src_cnt));

	/* Set 1st to 5th source addresses */
	for (i = 0; i < src_cnt; i++) {
		len = *nbytes;
		xgene_dma_set_src_buffer((i == 0) ? (desc1 + 8) :
					 xgene_dma_lookup_ext8(desc2, i - 1),
					 &len, &src[i]);
		XGENE_DMA_DESC_MULTI_SET(desc1, scf[i], i);
	}

	/* Hardware stores descriptor in little endian format */
	xgene_dma_cpu_to_le64(desc1, 4);
	xgene_dma_cpu_to_le64(desc2, 4);

	/* Update meta data */
	*nbytes = len;
	*dst += XGENE_DMA_MAX_BYTE_CNT;

	/* We need always 64B descriptor to perform xor or pq operations */
	desc_sw->flags |= XGENE_DMA_FLAG_64B_DESC;
}

static dma_cookie_t xgene_dma_tx_submit(struct dma_async_tx_descriptor *tx)
{
	struct xgene_dma_desc_sw *desc;
	struct xgene_dma_chan *chan;
	dma_cookie_t cookie;

	if (unlikely(!tx))
		return -EINVAL;

	chan = to_dma_chan(tx->chan);
	desc = to_dma_desc_sw(tx);

	spin_lock_bh(&chan->lock);

	cookie = dma_cookie_assign(tx);

	/* Add this transaction list onto the tail of the pending queue */
	list_splice_tail_init(&desc->tx_list, &chan->ld_pending);

	spin_unlock_bh(&chan->lock);

	return cookie;
}

static void xgene_dma_clean_descriptor(struct xgene_dma_chan *chan,
				       struct xgene_dma_desc_sw *desc)
{
	list_del(&desc->node);
	chan_dbg(chan, "LD %p free\n", desc);
	dma_pool_free(chan->desc_pool, desc, desc->tx.phys);
}

static struct xgene_dma_desc_sw *xgene_dma_alloc_descriptor(
				 struct xgene_dma_chan *chan)
{
	struct xgene_dma_desc_sw *desc;
	dma_addr_t phys;

	desc = dma_pool_alloc(chan->desc_pool, GFP_NOWAIT, &phys);
	if (!desc) {
		chan_err(chan, "Failed to allocate LDs\n");
		return NULL;
	}

	memset(desc, 0, sizeof(*desc));

	INIT_LIST_HEAD(&desc->tx_list);
	desc->tx.phys = phys;
	desc->tx.tx_submit = xgene_dma_tx_submit;
	dma_async_tx_descriptor_init(&desc->tx, &chan->dma_chan);

	chan_dbg(chan, "LD %p allocated\n", desc);

	return desc;
}

/**
 * xgene_dma_clean_completed_descriptor - free all descriptors which
 * has been completed and acked
 * @chan: X-Gene DMA channel
 *
 * This function is used on all completed and acked descriptors.
 */
static void xgene_dma_clean_completed_descriptor(struct xgene_dma_chan *chan)
{
	struct xgene_dma_desc_sw *desc, *_desc;

	/* Run the callback for each descriptor, in order */
	list_for_each_entry_safe(desc, _desc, &chan->ld_completed, node) {
		if (async_tx_test_ack(&desc->tx))
			xgene_dma_clean_descriptor(chan, desc);
	}
}

/**
 * xgene_dma_run_tx_complete_actions - cleanup a single link descriptor
 * @chan: X-Gene DMA channel
 * @desc: descriptor to cleanup and free
 *
 * This function is used on a descriptor which has been executed by the DMA
 * controller. It will run any callbacks, submit any dependencies.
 */
static void xgene_dma_run_tx_complete_actions(struct xgene_dma_chan *chan,
					      struct xgene_dma_desc_sw *desc)
{
	struct dma_async_tx_descriptor *tx = &desc->tx;

	/*
	 * If this is not the last transaction in the group,
	 * then no need to complete cookie and run any callback as
	 * this is not the tx_descriptor which had been sent to caller
	 * of this DMA request
	 */

	if (tx->cookie == 0)
		return;

	dma_cookie_complete(tx);

	/* Run the link descriptor callback function */
	if (tx->callback)
		tx->callback(tx->callback_param);

	dma_descriptor_unmap(tx);

	/* Run any dependencies */
	dma_run_dependencies(tx);
}

/**
 * xgene_dma_clean_running_descriptor - move the completed descriptor from
 * ld_running to ld_completed
 * @chan: X-Gene DMA channel
 * @desc: the descriptor which is completed
 *
 * Free the descriptor directly if acked by async_tx api,
 * else move it to queue ld_completed.
 */
static void xgene_dma_clean_running_descriptor(struct xgene_dma_chan *chan,
					       struct xgene_dma_desc_sw *desc)
{
	/* Remove from the list of running transactions */
	list_del(&desc->node);

	/*
	 * the client is allowed to attach dependent operations
	 * until 'ack' is set
	 */
	if (!async_tx_test_ack(&desc->tx)) {
		/*
		 * Move this descriptor to the list of descriptors which is
		 * completed, but still awaiting the 'ack' bit to be set.
		 */
		list_add_tail(&desc->node, &chan->ld_completed);
		return;
	}

	chan_dbg(chan, "LD %p free\n", desc);
	dma_pool_free(chan->desc_pool, desc, desc->tx.phys);
}

static int xgene_chan_xfer_request(struct xgene_dma_ring *ring,
				   struct xgene_dma_desc_sw *desc_sw)
{
	struct xgene_dma_desc_hw *desc_hw;

	/* Check if can push more descriptor to hw for execution */
	if (xgene_dma_ring_desc_cnt(ring) > (ring->slots - 2))
		return -EBUSY;

	/* Get hw descriptor from DMA tx ring */
	desc_hw = &ring->desc_hw[ring->head];

	/*
	 * Increment the head count to point next
	 * descriptor for next time
	 */
	if (++ring->head == ring->slots)
		ring->head = 0;

	/* Copy prepared sw descriptor data to hw descriptor */
	memcpy(desc_hw, &desc_sw->desc1, sizeof(*desc_hw));

	/*
	 * Check if we have prepared 64B descriptor,
	 * in this case we need one more hw descriptor
	 */
	if (desc_sw->flags & XGENE_DMA_FLAG_64B_DESC) {
		desc_hw = &ring->desc_hw[ring->head];

		if (++ring->head == ring->slots)
			ring->head = 0;

		memcpy(desc_hw, &desc_sw->desc2, sizeof(*desc_hw));
	}

	/* Notify the hw that we have descriptor ready for execution */
	iowrite32((desc_sw->flags & XGENE_DMA_FLAG_64B_DESC) ?
		  2 : 1, ring->cmd);

	return 0;
}

/**
 * xgene_chan_xfer_ld_pending - push any pending transactions to hw
 * @chan : X-Gene DMA channel
 *
 * LOCKING: must hold chan->desc_lock
 */
static void xgene_chan_xfer_ld_pending(struct xgene_dma_chan *chan)
{
	struct xgene_dma_desc_sw *desc_sw, *_desc_sw;
	int ret;

	/*
	 * If the list of pending descriptors is empty, then we
	 * don't need to do any work at all
	 */
	if (list_empty(&chan->ld_pending)) {
		chan_dbg(chan, "No pending LDs\n");
		return;
	}

	/*
	 * Move elements from the queue of pending transactions onto the list
	 * of running transactions and push it to hw for further executions
	 */
	list_for_each_entry_safe(desc_sw, _desc_sw, &chan->ld_pending, node) {
		/*
		 * Check if have pushed max number of transactions to hw
		 * as capable, so let's stop here and will push remaining
		 * elements from pening ld queue after completing some
		 * descriptors that we have already pushed
		 */
		if (chan->pending >= chan->max_outstanding)
			return;

		ret = xgene_chan_xfer_request(&chan->tx_ring, desc_sw);
		if (ret)
			return;

		/*
		 * Delete this element from ld pending queue and append it to
		 * ld running queue
		 */
		list_move_tail(&desc_sw->node, &chan->ld_running);

		/* Increment the pending transaction count */
		chan->pending++;
	}
}

/**
 * xgene_dma_cleanup_descriptors - cleanup link descriptors which are completed
 * and move them to ld_completed to free until flag 'ack' is set
 * @chan: X-Gene DMA channel
 *
 * This function is used on descriptors which have been executed by the DMA
 * controller. It will run any callbacks, submit any dependencies, then
 * free these descriptors if flag 'ack' is set.
 */
static void xgene_dma_cleanup_descriptors(struct xgene_dma_chan *chan)
{
	struct xgene_dma_ring *ring = &chan->rx_ring;
	struct xgene_dma_desc_sw *desc_sw, *_desc_sw;
	struct xgene_dma_desc_hw *desc_hw;
	u8 status;

	/* Clean already completed and acked descriptors */
	xgene_dma_clean_completed_descriptor(chan);

	/* Run the callback for each descriptor, in order */
	list_for_each_entry_safe(desc_sw, _desc_sw, &chan->ld_running, node) {
		/* Get subsequent hw descriptor from DMA rx ring */
		desc_hw = &ring->desc_hw[ring->head];

		/* Check if this descriptor has been completed */
		if (unlikely(XGENE_DMA_DESC_IS_EMPTY(desc_hw)))
			break;

		if (++ring->head == ring->slots)
			ring->head = 0;

		/* Check if we have any error with DMA transactions */
		status = XGENE_DMA_DESC_STATUS(
				XGENE_DMA_DESC_ELERR_RD(le64_to_cpu(
							desc_hw->m0)),
				XGENE_DMA_DESC_LERR_RD(le64_to_cpu(
						       desc_hw->m0)));
		if (status) {
			/* Print the DMA error type */
			chan_err(chan, "%s\n", xgene_dma_desc_err[status]);

			/*
			 * We have DMA transactions error here. Dump DMA Tx
			 * and Rx descriptors for this request */
			XGENE_DMA_DESC_DUMP(&desc_sw->desc1,
					    "X-Gene DMA TX DESC1: ");

			if (desc_sw->flags & XGENE_DMA_FLAG_64B_DESC)
				XGENE_DMA_DESC_DUMP(&desc_sw->desc2,
						    "X-Gene DMA TX DESC2: ");

			XGENE_DMA_DESC_DUMP(desc_hw,
					    "X-Gene DMA RX ERR DESC: ");
		}

		/* Notify the hw about this completed descriptor */
		iowrite32(-1, ring->cmd);

		/* Mark this hw descriptor as processed */
		XGENE_DMA_DESC_SET_EMPTY(desc_hw);

		xgene_dma_run_tx_complete_actions(chan, desc_sw);

		xgene_dma_clean_running_descriptor(chan, desc_sw);

		/*
		 * Decrement the pending transaction count
		 * as we have processed one
		 */
		chan->pending--;
	}

	/*
	 * Start any pending transactions automatically
	 * In the ideal case, we keep the DMA controller busy while we go
	 * ahead and free the descriptors below.
	 */
	xgene_chan_xfer_ld_pending(chan);
}

static int xgene_dma_alloc_chan_resources(struct dma_chan *dchan)
{
	struct xgene_dma_chan *chan = to_dma_chan(dchan);

	/* Has this channel already been allocated? */
	if (chan->desc_pool)
		return 1;

	chan->desc_pool = dma_pool_create(chan->name, chan->dev,
					  sizeof(struct xgene_dma_desc_sw),
					  0, 0);
	if (!chan->desc_pool) {
		chan_err(chan, "Failed to allocate descriptor pool\n");
		return -ENOMEM;
	}

	chan_dbg(chan, "Allocate descripto pool\n");

	return 1;
}

/**
 * xgene_dma_free_desc_list - Free all descriptors in a queue
 * @chan: X-Gene DMA channel
 * @list: the list to free
 *
 * LOCKING: must hold chan->desc_lock
 */
static void xgene_dma_free_desc_list(struct xgene_dma_chan *chan,
				     struct list_head *list)
{
	struct xgene_dma_desc_sw *desc, *_desc;

	list_for_each_entry_safe(desc, _desc, list, node)
		xgene_dma_clean_descriptor(chan, desc);
}

static void xgene_dma_free_tx_desc_list(struct xgene_dma_chan *chan,
					struct list_head *list)
{
	struct xgene_dma_desc_sw *desc, *_desc;

	list_for_each_entry_safe(desc, _desc, list, node)
		xgene_dma_clean_descriptor(chan, desc);
}

static void xgene_dma_free_chan_resources(struct dma_chan *dchan)
{
	struct xgene_dma_chan *chan = to_dma_chan(dchan);

	chan_dbg(chan, "Free all resources\n");

	if (!chan->desc_pool)
		return;

	spin_lock_bh(&chan->lock);

	/* Process all running descriptor */
	xgene_dma_cleanup_descriptors(chan);

	/* Clean all link descriptor queues */
	xgene_dma_free_desc_list(chan, &chan->ld_pending);
	xgene_dma_free_desc_list(chan, &chan->ld_running);
	xgene_dma_free_desc_list(chan, &chan->ld_completed);

	spin_unlock_bh(&chan->lock);

	/* Delete this channel DMA pool */
	dma_pool_destroy(chan->desc_pool);
	chan->desc_pool = NULL;
}

static struct dma_async_tx_descriptor *xgene_dma_prep_memcpy(
	struct dma_chan *dchan, dma_addr_t dst, dma_addr_t src,
	size_t len, unsigned long flags)
{
	struct xgene_dma_desc_sw *first = NULL, *new;
	struct xgene_dma_chan *chan;
	size_t copy;

	if (unlikely(!dchan || !len))
		return NULL;

	chan = to_dma_chan(dchan);

	do {
		/* Allocate the link descriptor from DMA pool */
		new = xgene_dma_alloc_descriptor(chan);
		if (!new)
			goto fail;

		/* Create the largest transaction possible */
		copy = min_t(size_t, len, XGENE_DMA_MAX_64B_DESC_BYTE_CNT);

		/* Prepare DMA descriptor */
		xgene_dma_prep_cpy_desc(chan, new, dst, src, copy);

		if (!first)
			first = new;

		new->tx.cookie = 0;
		async_tx_ack(&new->tx);

		/* Update metadata */
		len -= copy;
		dst += copy;
		src += copy;

		/* Insert the link descriptor to the LD ring */
		list_add_tail(&new->node, &first->tx_list);
	} while (len);

	new->tx.flags = flags; /* client is in control of this ack */
	new->tx.cookie = -EBUSY;
	list_splice(&first->tx_list, &new->tx_list);

	return &new->tx;

fail:
	if (!first)
		return NULL;

	xgene_dma_free_tx_desc_list(chan, &first->tx_list);
	return NULL;
}

static struct dma_async_tx_descriptor *xgene_dma_prep_sg(
	struct dma_chan *dchan, struct scatterlist *dst_sg,
	u32 dst_nents, struct scatterlist *src_sg,
	u32 src_nents, unsigned long flags)
{
	struct xgene_dma_desc_sw *first = NULL, *new = NULL;
	struct xgene_dma_chan *chan;
	size_t dst_avail, src_avail;
	dma_addr_t dst, src;
	size_t len;

	if (unlikely(!dchan))
		return NULL;

	if (unlikely(!dst_nents || !src_nents))
		return NULL;

	if (unlikely(!dst_sg || !src_sg))
		return NULL;

	chan = to_dma_chan(dchan);

	/* Get prepared for the loop */
	dst_avail = sg_dma_len(dst_sg);
	src_avail = sg_dma_len(src_sg);
	dst_nents--;
	src_nents--;

	/* Run until we are out of scatterlist entries */
	while (true) {
		/* Create the largest transaction possible */
		len = min_t(size_t, src_avail, dst_avail);
		len = min_t(size_t, len, XGENE_DMA_MAX_64B_DESC_BYTE_CNT);
		if (len == 0)
			goto fetch;

		dst = sg_dma_address(dst_sg) + sg_dma_len(dst_sg) - dst_avail;
		src = sg_dma_address(src_sg) + sg_dma_len(src_sg) - src_avail;

		/* Allocate the link descriptor from DMA pool */
		new = xgene_dma_alloc_descriptor(chan);
		if (!new)
			goto fail;

		/* Prepare DMA descriptor */
		xgene_dma_prep_cpy_desc(chan, new, dst, src, len);

		if (!first)
			first = new;

		new->tx.cookie = 0;
		async_tx_ack(&new->tx);

		/* update metadata */
		dst_avail -= len;
		src_avail -= len;

		/* Insert the link descriptor to the LD ring */
		list_add_tail(&new->node, &first->tx_list);

fetch:
		/* fetch the next dst scatterlist entry */
		if (dst_avail == 0) {
			/* no more entries: we're done */
			if (dst_nents == 0)
				break;

			/* fetch the next entry: if there are no more: done */
			dst_sg = sg_next(dst_sg);
			if (!dst_sg)
				break;

			dst_nents--;
			dst_avail = sg_dma_len(dst_sg);
		}

		/* fetch the next src scatterlist entry */
		if (src_avail == 0) {
			/* no more entries: we're done */
			if (src_nents == 0)
				break;

			/* fetch the next entry: if there are no more: done */
			src_sg = sg_next(src_sg);
			if (!src_sg)
				break;

			src_nents--;
			src_avail = sg_dma_len(src_sg);
		}
	}

	if (!new)
		return NULL;

	new->tx.flags = flags; /* client is in control of this ack */
	new->tx.cookie = -EBUSY;
	list_splice(&first->tx_list, &new->tx_list);

	return &new->tx;
fail:
	if (!first)
		return NULL;

	xgene_dma_free_tx_desc_list(chan, &first->tx_list);
	return NULL;
}

static struct dma_async_tx_descriptor *xgene_dma_prep_xor(
	struct dma_chan *dchan, dma_addr_t dst,	dma_addr_t *src,
	u32 src_cnt, size_t len, unsigned long flags)
{
	struct xgene_dma_desc_sw *first = NULL, *new;
	struct xgene_dma_chan *chan;
	static u8 multi[XGENE_DMA_MAX_XOR_SRC] = {
				0x01, 0x01, 0x01, 0x01, 0x01};

	if (unlikely(!dchan || !len))
		return NULL;

	chan = to_dma_chan(dchan);

	do {
		/* Allocate the link descriptor from DMA pool */
		new = xgene_dma_alloc_descriptor(chan);
		if (!new)
			goto fail;

		/* Prepare xor DMA descriptor */
		xgene_dma_prep_xor_desc(chan, new, &dst, src,
					src_cnt, &len, multi);

		if (!first)
			first = new;

		new->tx.cookie = 0;
		async_tx_ack(&new->tx);

		/* Insert the link descriptor to the LD ring */
		list_add_tail(&new->node, &first->tx_list);
	} while (len);

	new->tx.flags = flags; /* client is in control of this ack */
	new->tx.cookie = -EBUSY;
	list_splice(&first->tx_list, &new->tx_list);

	return &new->tx;

fail:
	if (!first)
		return NULL;

	xgene_dma_free_tx_desc_list(chan, &first->tx_list);
	return NULL;
}

static struct dma_async_tx_descriptor *xgene_dma_prep_pq(
	struct dma_chan *dchan, dma_addr_t *dst, dma_addr_t *src,
	u32 src_cnt, const u8 *scf, size_t len, unsigned long flags)
{
	struct xgene_dma_desc_sw *first = NULL, *new;
	struct xgene_dma_chan *chan;
	size_t _len = len;
	dma_addr_t _src[XGENE_DMA_MAX_XOR_SRC];
	static u8 multi[XGENE_DMA_MAX_XOR_SRC] = {0x01, 0x01, 0x01, 0x01, 0x01};

	if (unlikely(!dchan || !len))
		return NULL;

	chan = to_dma_chan(dchan);

	/*
	 * Save source addresses on local variable, may be we have to
	 * prepare two descriptor to generate P and Q if both enabled
	 * in the flags by client
	 */
	memcpy(_src, src, sizeof(*src) * src_cnt);

	if (flags & DMA_PREP_PQ_DISABLE_P)
		len = 0;

	if (flags & DMA_PREP_PQ_DISABLE_Q)
		_len = 0;

	do {
		/* Allocate the link descriptor from DMA pool */
		new = xgene_dma_alloc_descriptor(chan);
		if (!new)
			goto fail;

		if (!first)
			first = new;

		new->tx.cookie = 0;
		async_tx_ack(&new->tx);

		/* Insert the link descriptor to the LD ring */
		list_add_tail(&new->node, &first->tx_list);

		/*
		 * Prepare DMA descriptor to generate P,
		 * if DMA_PREP_PQ_DISABLE_P flag is not set
		 */
		if (len) {
			xgene_dma_prep_xor_desc(chan, new, &dst[0], src,
						src_cnt, &len, multi);
			continue;
		}

		/*
		 * Prepare DMA descriptor to generate Q,
		 * if DMA_PREP_PQ_DISABLE_Q flag is not set
		 */
		if (_len) {
			xgene_dma_prep_xor_desc(chan, new, &dst[1], _src,
						src_cnt, &_len, scf);
		}
	} while (len || _len);

	new->tx.flags = flags; /* client is in control of this ack */
	new->tx.cookie = -EBUSY;
	list_splice(&first->tx_list, &new->tx_list);

	return &new->tx;

fail:
	if (!first)
		return NULL;

	xgene_dma_free_tx_desc_list(chan, &first->tx_list);
	return NULL;
}

static void xgene_dma_issue_pending(struct dma_chan *dchan)
{
	struct xgene_dma_chan *chan = to_dma_chan(dchan);

	spin_lock_bh(&chan->lock);
	xgene_chan_xfer_ld_pending(chan);
	spin_unlock_bh(&chan->lock);
}

static enum dma_status xgene_dma_tx_status(struct dma_chan *dchan,
					   dma_cookie_t cookie,
					   struct dma_tx_state *txstate)
{
	return dma_cookie_status(dchan, cookie, txstate);
}

static void xgene_dma_tasklet_cb(unsigned long data)
{
	struct xgene_dma_chan *chan = (struct xgene_dma_chan *)data;

	spin_lock_bh(&chan->lock);

	/* Run all cleanup for descriptors which have been completed */
	xgene_dma_cleanup_descriptors(chan);

	/* Re-enable DMA channel IRQ */
	enable_irq(chan->rx_irq);

	spin_unlock_bh(&chan->lock);
}

static irqreturn_t xgene_dma_chan_ring_isr(int irq, void *id)
{
	struct xgene_dma_chan *chan = (struct xgene_dma_chan *)id;

	BUG_ON(!chan);

	/*
	 * Disable DMA channel IRQ until we process completed
	 * descriptors
	 */
	disable_irq_nosync(chan->rx_irq);

	/*
	 * Schedule the tasklet to handle all cleanup of the current
	 * transaction. It will start a new transaction if there is
	 * one pending.
	 */
	tasklet_schedule(&chan->tasklet);

	return IRQ_HANDLED;
}

static irqreturn_t xgene_dma_err_isr(int irq, void *id)
{
	struct xgene_dma *pdma = (struct xgene_dma *)id;
	unsigned long int_mask;
	u32 val, i;

	val = ioread32(pdma->csr_dma + XGENE_DMA_INT);

	/* Clear DMA interrupts */
	iowrite32(val, pdma->csr_dma + XGENE_DMA_INT);

	/* Print DMA error info */
	int_mask = val >> XGENE_DMA_INT_MASK_SHIFT;
	for_each_set_bit(i, &int_mask, ARRAY_SIZE(xgene_dma_err))
		dev_err(pdma->dev,
			"Interrupt status 0x%08X %s\n", val, xgene_dma_err[i]);

	return IRQ_HANDLED;
}

static void xgene_dma_wr_ring_state(struct xgene_dma_ring *ring)
{
	int i;

	iowrite32(ring->num, ring->pdma->csr_ring + XGENE_DMA_RING_STATE);

	for (i = 0; i < XGENE_DMA_RING_NUM_CONFIG; i++)
		iowrite32(ring->state[i], ring->pdma->csr_ring +
			  XGENE_DMA_RING_STATE_WR_BASE + (i * 4));
}

static void xgene_dma_clr_ring_state(struct xgene_dma_ring *ring)
{
	memset(ring->state, 0, sizeof(u32) * XGENE_DMA_RING_NUM_CONFIG);
	xgene_dma_wr_ring_state(ring);
}

static void xgene_dma_setup_ring(struct xgene_dma_ring *ring)
{
	void *ring_cfg = ring->state;
	u64 addr = ring->desc_paddr;
	void *desc;
	u32 i, val;

	ring->slots = ring->size / XGENE_DMA_RING_WQ_DESC_SIZE;

	/* Clear DMA ring state */
	xgene_dma_clr_ring_state(ring);

	/* Set DMA ring type */
	XGENE_DMA_RING_TYPE_SET(ring_cfg, XGENE_DMA_RING_TYPE_REGULAR);

	if (ring->owner == XGENE_DMA_RING_OWNER_DMA) {
		/* Set recombination buffer and timeout */
		XGENE_DMA_RING_RECOMBBUF_SET(ring_cfg);
		XGENE_DMA_RING_RECOMTIMEOUTL_SET(ring_cfg);
		XGENE_DMA_RING_RECOMTIMEOUTH_SET(ring_cfg);
	}

	/* Initialize DMA ring state */
	XGENE_DMA_RING_SELTHRSH_SET(ring_cfg);
	XGENE_DMA_RING_ACCEPTLERR_SET(ring_cfg);
	XGENE_DMA_RING_COHERENT_SET(ring_cfg);
	XGENE_DMA_RING_ADDRL_SET(ring_cfg, addr);
	XGENE_DMA_RING_ADDRH_SET(ring_cfg, addr);
	XGENE_DMA_RING_SIZE_SET(ring_cfg, ring->cfgsize);

	/* Write DMA ring configurations */
	xgene_dma_wr_ring_state(ring);

	/* Set DMA ring id */
	iowrite32(XGENE_DMA_RING_ID_SETUP(ring->id),
		  ring->pdma->csr_ring + XGENE_DMA_RING_ID);

	/* Set DMA ring buffer */
	iowrite32(XGENE_DMA_RING_ID_BUF_SETUP(ring->num),
		  ring->pdma->csr_ring + XGENE_DMA_RING_ID_BUF);

	if (ring->owner != XGENE_DMA_RING_OWNER_CPU)
		return;

	/* Set empty signature to DMA Rx ring descriptors */
	for (i = 0; i < ring->slots; i++) {
		desc = &ring->desc_hw[i];
		XGENE_DMA_DESC_SET_EMPTY(desc);
	}

	/* Enable DMA Rx ring interrupt */
	val = ioread32(ring->pdma->csr_ring + XGENE_DMA_RING_NE_INT_MODE);
	XGENE_DMA_RING_NE_INT_MODE_SET(val, ring->buf_num);
	iowrite32(val, ring->pdma->csr_ring + XGENE_DMA_RING_NE_INT_MODE);
}

static void xgene_dma_clear_ring(struct xgene_dma_ring *ring)
{
	u32 ring_id, val;

	if (ring->owner == XGENE_DMA_RING_OWNER_CPU) {
		/* Disable DMA Rx ring interrupt */
		val = ioread32(ring->pdma->csr_ring +
			       XGENE_DMA_RING_NE_INT_MODE);
		XGENE_DMA_RING_NE_INT_MODE_RESET(val, ring->buf_num);
		iowrite32(val, ring->pdma->csr_ring +
			  XGENE_DMA_RING_NE_INT_MODE);
	}

	/* Clear DMA ring state */
	ring_id = XGENE_DMA_RING_ID_SETUP(ring->id);
	iowrite32(ring_id, ring->pdma->csr_ring + XGENE_DMA_RING_ID);

	iowrite32(0, ring->pdma->csr_ring + XGENE_DMA_RING_ID_BUF);
	xgene_dma_clr_ring_state(ring);
}

static void xgene_dma_set_ring_cmd(struct xgene_dma_ring *ring)
{
	ring->cmd_base = ring->pdma->csr_ring_cmd +
				XGENE_DMA_RING_CMD_BASE_OFFSET((ring->num -
							  XGENE_DMA_RING_NUM));

	ring->cmd = ring->cmd_base + XGENE_DMA_RING_CMD_OFFSET;
}

static int xgene_dma_get_ring_size(struct xgene_dma_chan *chan,
				   enum xgene_dma_ring_cfgsize cfgsize)
{
	int size;

	switch (cfgsize) {
	case XGENE_DMA_RING_CFG_SIZE_512B:
		size = 0x200;
		break;
	case XGENE_DMA_RING_CFG_SIZE_2KB:
		size = 0x800;
		break;
	case XGENE_DMA_RING_CFG_SIZE_16KB:
		size = 0x4000;
		break;
	case XGENE_DMA_RING_CFG_SIZE_64KB:
		size = 0x10000;
		break;
	case XGENE_DMA_RING_CFG_SIZE_512KB:
		size = 0x80000;
		break;
	default:
		chan_err(chan, "Unsupported cfg ring size %d\n", cfgsize);
		return -EINVAL;
	}

	return size;
}

static void xgene_dma_delete_ring_one(struct xgene_dma_ring *ring)
{
	/* Clear DMA ring configurations */
	xgene_dma_clear_ring(ring);

	/* De-allocate DMA ring descriptor */
	if (ring->desc_vaddr) {
		dma_free_coherent(ring->pdma->dev, ring->size,
				  ring->desc_vaddr, ring->desc_paddr);
		ring->desc_vaddr = NULL;
	}
}

static void xgene_dma_delete_chan_rings(struct xgene_dma_chan *chan)
{
	xgene_dma_delete_ring_one(&chan->rx_ring);
	xgene_dma_delete_ring_one(&chan->tx_ring);
}

static int xgene_dma_create_ring_one(struct xgene_dma_chan *chan,
				     struct xgene_dma_ring *ring,
				     enum xgene_dma_ring_cfgsize cfgsize)
{
	/* Setup DMA ring descriptor variables */
	ring->pdma = chan->pdma;
	ring->cfgsize = cfgsize;
	ring->num = chan->pdma->ring_num++;
	ring->id = XGENE_DMA_RING_ID_GET(ring->owner, ring->buf_num);

	ring->size = xgene_dma_get_ring_size(chan, cfgsize);
	if (ring->size <= 0)
		return ring->size;

	/* Allocate memory for DMA ring descriptor */
	ring->desc_vaddr = dma_zalloc_coherent(chan->dev, ring->size,
					       &ring->desc_paddr, GFP_KERNEL);
	if (!ring->desc_vaddr) {
		chan_err(chan, "Failed to allocate ring desc\n");
		return -ENOMEM;
	}

	/* Configure and enable DMA ring */
	xgene_dma_set_ring_cmd(ring);
	xgene_dma_setup_ring(ring);

	return 0;
}

static int xgene_dma_create_chan_rings(struct xgene_dma_chan *chan)
{
	struct xgene_dma_ring *rx_ring = &chan->rx_ring;
	struct xgene_dma_ring *tx_ring = &chan->tx_ring;
	int ret;

	/* Create DMA Rx ring descriptor */
	rx_ring->owner = XGENE_DMA_RING_OWNER_CPU;
	rx_ring->buf_num = XGENE_DMA_CPU_BUFNUM + chan->id;

	ret = xgene_dma_create_ring_one(chan, rx_ring,
					XGENE_DMA_RING_CFG_SIZE_64KB);
	if (ret)
		return ret;

	chan_dbg(chan, "Rx ring id 0x%X num %d desc 0x%p\n",
		 rx_ring->id, rx_ring->num, rx_ring->desc_vaddr);

	/* Create DMA Tx ring descriptor */
	tx_ring->owner = XGENE_DMA_RING_OWNER_DMA;
	tx_ring->buf_num = XGENE_DMA_BUFNUM + chan->id;

	ret = xgene_dma_create_ring_one(chan, tx_ring,
					XGENE_DMA_RING_CFG_SIZE_64KB);
	if (ret) {
		xgene_dma_delete_ring_one(rx_ring);
		return ret;
	}

	tx_ring->dst_ring_num = XGENE_DMA_RING_DST_ID(rx_ring->num);

	chan_dbg(chan,
		 "Tx ring id 0x%X num %d desc 0x%p\n",
		 tx_ring->id, tx_ring->num, tx_ring->desc_vaddr);

	/* Set the max outstanding request possible to this channel */
	chan->max_outstanding = rx_ring->slots;

	return ret;
}

static int xgene_dma_init_rings(struct xgene_dma *pdma)
{
	int ret, i, j;

	for (i = 0; i < XGENE_DMA_MAX_CHANNEL; i++) {
		ret = xgene_dma_create_chan_rings(&pdma->chan[i]);
		if (ret) {
			for (j = 0; j < i; j++)
				xgene_dma_delete_chan_rings(&pdma->chan[j]);
			return ret;
		}
	}

	return ret;
}

static void xgene_dma_enable(struct xgene_dma *pdma)
{
	u32 val;

	/* Configure and enable DMA engine */
	val = ioread32(pdma->csr_dma + XGENE_DMA_GCR);
	XGENE_DMA_CH_SETUP(val);
	XGENE_DMA_ENABLE(val);
	iowrite32(val, pdma->csr_dma + XGENE_DMA_GCR);
}

static void xgene_dma_disable(struct xgene_dma *pdma)
{
	u32 val;

	val = ioread32(pdma->csr_dma + XGENE_DMA_GCR);
	XGENE_DMA_DISABLE(val);
	iowrite32(val, pdma->csr_dma + XGENE_DMA_GCR);
}

static void xgene_dma_mask_interrupts(struct xgene_dma *pdma)
{
	/*
	 * Mask DMA ring overflow, underflow and
	 * AXI write/read error interrupts
	 */
	iowrite32(XGENE_DMA_INT_ALL_MASK,
		  pdma->csr_dma + XGENE_DMA_RING_INT0_MASK);
	iowrite32(XGENE_DMA_INT_ALL_MASK,
		  pdma->csr_dma + XGENE_DMA_RING_INT1_MASK);
	iowrite32(XGENE_DMA_INT_ALL_MASK,
		  pdma->csr_dma + XGENE_DMA_RING_INT2_MASK);
	iowrite32(XGENE_DMA_INT_ALL_MASK,
		  pdma->csr_dma + XGENE_DMA_RING_INT3_MASK);
	iowrite32(XGENE_DMA_INT_ALL_MASK,
		  pdma->csr_dma + XGENE_DMA_RING_INT4_MASK);

	/* Mask DMA error interrupts */
	iowrite32(XGENE_DMA_INT_ALL_MASK, pdma->csr_dma + XGENE_DMA_INT_MASK);
}

static void xgene_dma_unmask_interrupts(struct xgene_dma *pdma)
{
	/*
	 * Unmask DMA ring overflow, underflow and
	 * AXI write/read error interrupts
	 */
	iowrite32(XGENE_DMA_INT_ALL_UNMASK,
		  pdma->csr_dma + XGENE_DMA_RING_INT0_MASK);
	iowrite32(XGENE_DMA_INT_ALL_UNMASK,
		  pdma->csr_dma + XGENE_DMA_RING_INT1_MASK);
	iowrite32(XGENE_DMA_INT_ALL_UNMASK,
		  pdma->csr_dma + XGENE_DMA_RING_INT2_MASK);
	iowrite32(XGENE_DMA_INT_ALL_UNMASK,
		  pdma->csr_dma + XGENE_DMA_RING_INT3_MASK);
	iowrite32(XGENE_DMA_INT_ALL_UNMASK,
		  pdma->csr_dma + XGENE_DMA_RING_INT4_MASK);

	/* Unmask DMA error interrupts */
	iowrite32(XGENE_DMA_INT_ALL_UNMASK,
		  pdma->csr_dma + XGENE_DMA_INT_MASK);
}

static void xgene_dma_init_hw(struct xgene_dma *pdma)
{
	u32 val;

	/* Associate DMA ring to corresponding ring HW */
	iowrite32(XGENE_DMA_ASSOC_RING_MNGR1,
		  pdma->csr_dma + XGENE_DMA_CFG_RING_WQ_ASSOC);

	/* Configure RAID6 polynomial control setting */
	if (is_pq_enabled(pdma))
		iowrite32(XGENE_DMA_RAID6_MULTI_CTRL(0x1D),
			  pdma->csr_dma + XGENE_DMA_RAID6_CONT);
	else
		dev_info(pdma->dev, "PQ is disabled in HW\n");

	xgene_dma_enable(pdma);
	xgene_dma_unmask_interrupts(pdma);

	/* Get DMA id and version info */
	val = ioread32(pdma->csr_dma + XGENE_DMA_IPBRR);

	/* DMA device info */
	dev_info(pdma->dev,
		 "X-Gene DMA v%d.%02d.%02d driver registered %d channels",
		 XGENE_DMA_REV_NO_RD(val), XGENE_DMA_BUS_ID_RD(val),
		 XGENE_DMA_DEV_ID_RD(val), XGENE_DMA_MAX_CHANNEL);
}

static int xgene_dma_init_ring_mngr(struct xgene_dma *pdma)
{
	if (ioread32(pdma->csr_ring + XGENE_DMA_RING_CLKEN) &&
	    (!ioread32(pdma->csr_ring + XGENE_DMA_RING_SRST)))
		return 0;

	iowrite32(0x3, pdma->csr_ring + XGENE_DMA_RING_CLKEN);
	iowrite32(0x0, pdma->csr_ring + XGENE_DMA_RING_SRST);

	/* Bring up memory */
	iowrite32(0x0, pdma->csr_ring + XGENE_DMA_RING_MEM_RAM_SHUTDOWN);

	/* Force a barrier */
	ioread32(pdma->csr_ring + XGENE_DMA_RING_MEM_RAM_SHUTDOWN);

	/* reset may take up to 1ms */
	usleep_range(1000, 1100);

	if (ioread32(pdma->csr_ring + XGENE_DMA_RING_BLK_MEM_RDY)
		!= XGENE_DMA_RING_BLK_MEM_RDY_VAL) {
		dev_err(pdma->dev,
			"Failed to release ring mngr memory from shutdown\n");
		return -ENODEV;
	}

	/* program threshold set 1 and all hysteresis */
	iowrite32(XGENE_DMA_RING_THRESLD0_SET1_VAL,
		  pdma->csr_ring + XGENE_DMA_RING_THRESLD0_SET1);
	iowrite32(XGENE_DMA_RING_THRESLD1_SET1_VAL,
		  pdma->csr_ring + XGENE_DMA_RING_THRESLD1_SET1);
	iowrite32(XGENE_DMA_RING_HYSTERESIS_VAL,
		  pdma->csr_ring + XGENE_DMA_RING_HYSTERESIS);

	/* Enable QPcore and assign error queue */
	iowrite32(XGENE_DMA_RING_ENABLE,
		  pdma->csr_ring + XGENE_DMA_RING_CONFIG);

	return 0;
}

static int xgene_dma_init_mem(struct xgene_dma *pdma)
{
	int ret;

	ret = xgene_dma_init_ring_mngr(pdma);
	if (ret)
		return ret;

	/* Bring up memory */
	iowrite32(0x0, pdma->csr_dma + XGENE_DMA_MEM_RAM_SHUTDOWN);

	/* Force a barrier */
	ioread32(pdma->csr_dma + XGENE_DMA_MEM_RAM_SHUTDOWN);

	/* reset may take up to 1ms */
	usleep_range(1000, 1100);

	if (ioread32(pdma->csr_dma + XGENE_DMA_BLK_MEM_RDY)
		!= XGENE_DMA_BLK_MEM_RDY_VAL) {
		dev_err(pdma->dev,
			"Failed to release DMA memory from shutdown\n");
		return -ENODEV;
	}

	return 0;
}

static int xgene_dma_request_irqs(struct xgene_dma *pdma)
{
	struct xgene_dma_chan *chan;
	int ret, i, j;

	/* Register DMA error irq */
	ret = devm_request_irq(pdma->dev, pdma->err_irq, xgene_dma_err_isr,
			       0, "dma_error", pdma);
	if (ret) {
		dev_err(pdma->dev,
			"Failed to register error IRQ %d\n", pdma->err_irq);
		return ret;
	}

	/* Register DMA channel rx irq */
	for (i = 0; i < XGENE_DMA_MAX_CHANNEL; i++) {
		chan = &pdma->chan[i];
		ret = devm_request_irq(chan->dev, chan->rx_irq,
				       xgene_dma_chan_ring_isr,
				       0, chan->name, chan);
		if (ret) {
			chan_err(chan, "Failed to register Rx IRQ %d\n",
				 chan->rx_irq);
			devm_free_irq(pdma->dev, pdma->err_irq, pdma);

			for (j = 0; j < i; j++) {
				chan = &pdma->chan[i];
				devm_free_irq(chan->dev, chan->rx_irq, chan);
			}

			return ret;
		}
	}

	return 0;
}

static void xgene_dma_free_irqs(struct xgene_dma *pdma)
{
	struct xgene_dma_chan *chan;
	int i;

	/* Free DMA device error irq */
	devm_free_irq(pdma->dev, pdma->err_irq, pdma);

	for (i = 0; i < XGENE_DMA_MAX_CHANNEL; i++) {
		chan = &pdma->chan[i];
		devm_free_irq(chan->dev, chan->rx_irq, chan);
	}
}

static void xgene_dma_set_caps(struct xgene_dma_chan *chan,
			       struct dma_device *dma_dev)
{
	/* Initialize DMA device capability mask */
	dma_cap_zero(dma_dev->cap_mask);

	/* Set DMA device capability */
	dma_cap_set(DMA_MEMCPY, dma_dev->cap_mask);
	dma_cap_set(DMA_SG, dma_dev->cap_mask);

	/* Basically here, the X-Gene SoC DMA engine channel 0 supports XOR
	 * and channel 1 supports XOR, PQ both. First thing here is we have
	 * mechanism in hw to enable/disable PQ/XOR supports on channel 1,
	 * we can make sure this by reading SoC Efuse register.
	 * Second thing, we have hw errata that if we run channel 0 and
	 * channel 1 simultaneously with executing XOR and PQ request,
	 * suddenly DMA engine hangs, So here we enable XOR on channel 0 only
	 * if XOR and PQ supports on channel 1 is disabled.
	 */
	if ((chan->id == XGENE_DMA_PQ_CHANNEL) &&
	    is_pq_enabled(chan->pdma)) {
		dma_cap_set(DMA_PQ, dma_dev->cap_mask);
		dma_cap_set(DMA_XOR, dma_dev->cap_mask);
	} else if ((chan->id == XGENE_DMA_XOR_CHANNEL) &&
		   !is_pq_enabled(chan->pdma)) {
		dma_cap_set(DMA_XOR, dma_dev->cap_mask);
	}

	/* Set base and prep routines */
	dma_dev->dev = chan->dev;
	dma_dev->device_alloc_chan_resources = xgene_dma_alloc_chan_resources;
	dma_dev->device_free_chan_resources = xgene_dma_free_chan_resources;
	dma_dev->device_issue_pending = xgene_dma_issue_pending;
	dma_dev->device_tx_status = xgene_dma_tx_status;
	dma_dev->device_prep_dma_memcpy = xgene_dma_prep_memcpy;
	dma_dev->device_prep_dma_sg = xgene_dma_prep_sg;

	if (dma_has_cap(DMA_XOR, dma_dev->cap_mask)) {
		dma_dev->device_prep_dma_xor = xgene_dma_prep_xor;
		dma_dev->max_xor = XGENE_DMA_MAX_XOR_SRC;
		dma_dev->xor_align = XGENE_DMA_XOR_ALIGNMENT;
	}

	if (dma_has_cap(DMA_PQ, dma_dev->cap_mask)) {
		dma_dev->device_prep_dma_pq = xgene_dma_prep_pq;
		dma_dev->max_pq = XGENE_DMA_MAX_XOR_SRC;
		dma_dev->pq_align = XGENE_DMA_XOR_ALIGNMENT;
	}
}

static int xgene_dma_async_register(struct xgene_dma *pdma, int id)
{
	struct xgene_dma_chan *chan = &pdma->chan[id];
	struct dma_device *dma_dev = &pdma->dma_dev[id];
	int ret;

	chan->dma_chan.device = dma_dev;

	spin_lock_init(&chan->lock);
	INIT_LIST_HEAD(&chan->ld_pending);
	INIT_LIST_HEAD(&chan->ld_running);
	INIT_LIST_HEAD(&chan->ld_completed);
	tasklet_init(&chan->tasklet, xgene_dma_tasklet_cb,
		     (unsigned long)chan);

	chan->pending = 0;
	chan->desc_pool = NULL;
	dma_cookie_init(&chan->dma_chan);

	/* Setup dma device capabilities and prep routines */
	xgene_dma_set_caps(chan, dma_dev);

	/* Initialize DMA device list head */
	INIT_LIST_HEAD(&dma_dev->channels);
	list_add_tail(&chan->dma_chan.device_node, &dma_dev->channels);

	/* Register with Linux async DMA framework*/
	ret = dma_async_device_register(dma_dev);
	if (ret) {
		chan_err(chan, "Failed to register async device %d", ret);
		tasklet_kill(&chan->tasklet);

		return ret;
	}

	/* DMA capability info */
	dev_info(pdma->dev,
		 "%s: CAPABILITY ( %s%s%s%s)\n", dma_chan_name(&chan->dma_chan),
		 dma_has_cap(DMA_MEMCPY, dma_dev->cap_mask) ? "MEMCPY " : "",
		 dma_has_cap(DMA_SG, dma_dev->cap_mask) ? "SGCPY " : "",
		 dma_has_cap(DMA_XOR, dma_dev->cap_mask) ? "XOR " : "",
		 dma_has_cap(DMA_PQ, dma_dev->cap_mask) ? "PQ " : "");

	return 0;
}

static int xgene_dma_init_async(struct xgene_dma *pdma)
{
	int ret, i, j;

	for (i = 0; i < XGENE_DMA_MAX_CHANNEL ; i++) {
		ret = xgene_dma_async_register(pdma, i);
		if (ret) {
			for (j = 0; j < i; j++) {
				dma_async_device_unregister(&pdma->dma_dev[j]);
				tasklet_kill(&pdma->chan[j].tasklet);
			}

			return ret;
		}
	}

	return ret;
}

static void xgene_dma_async_unregister(struct xgene_dma *pdma)
{
	int i;

	for (i = 0; i < XGENE_DMA_MAX_CHANNEL; i++)
		dma_async_device_unregister(&pdma->dma_dev[i]);
}

static void xgene_dma_init_channels(struct xgene_dma *pdma)
{
	struct xgene_dma_chan *chan;
	int i;

	pdma->ring_num = XGENE_DMA_RING_NUM;

	for (i = 0; i < XGENE_DMA_MAX_CHANNEL; i++) {
		chan = &pdma->chan[i];
		chan->dev = pdma->dev;
		chan->pdma = pdma;
		chan->id = i;
		snprintf(chan->name, sizeof(chan->name), "dmachan%d", chan->id);
	}
}

static int xgene_dma_get_resources(struct platform_device *pdev,
				   struct xgene_dma *pdma)
{
	struct resource *res;
	int irq, i;

	/* Get DMA csr region */
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		dev_err(&pdev->dev, "Failed to get csr region\n");
		return -ENXIO;
	}

	pdma->csr_dma = devm_ioremap(&pdev->dev, res->start,
				     resource_size(res));
	if (!pdma->csr_dma) {
		dev_err(&pdev->dev, "Failed to ioremap csr region");
		return -ENOMEM;
	}

	/* Get DMA ring csr region */
	res = platform_get_resource(pdev, IORESOURCE_MEM, 1);
	if (!res) {
		dev_err(&pdev->dev, "Failed to get ring csr region\n");
		return -ENXIO;
	}

	pdma->csr_ring =  devm_ioremap(&pdev->dev, res->start,
				       resource_size(res));
	if (!pdma->csr_ring) {
		dev_err(&pdev->dev, "Failed to ioremap ring csr region");
		return -ENOMEM;
	}

	/* Get DMA ring cmd csr region */
	res = platform_get_resource(pdev, IORESOURCE_MEM, 2);
	if (!res) {
		dev_err(&pdev->dev, "Failed to get ring cmd csr region\n");
		return -ENXIO;
	}

	pdma->csr_ring_cmd = devm_ioremap(&pdev->dev, res->start,
					  resource_size(res));
	if (!pdma->csr_ring_cmd) {
		dev_err(&pdev->dev, "Failed to ioremap ring cmd csr region");
		return -ENOMEM;
	}

	/* Get efuse csr region */
	res = platform_get_resource(pdev, IORESOURCE_MEM, 3);
	if (!res) {
		dev_err(&pdev->dev, "Failed to get efuse csr region\n");
		return -ENXIO;
	}

	pdma->csr_efuse = devm_ioremap(&pdev->dev, res->start,
				       resource_size(res));
	if (!pdma->csr_efuse) {
		dev_err(&pdev->dev, "Failed to ioremap efuse csr region");
		return -ENOMEM;
	}

	/* Get DMA error interrupt */
	irq = platform_get_irq(pdev, 0);
	if (irq <= 0) {
		dev_err(&pdev->dev, "Failed to get Error IRQ\n");
		return -ENXIO;
	}

	pdma->err_irq = irq;

	/* Get DMA Rx ring descriptor interrupts for all DMA channels */
	for (i = 1; i <= XGENE_DMA_MAX_CHANNEL; i++) {
		irq = platform_get_irq(pdev, i);
		if (irq <= 0) {
			dev_err(&pdev->dev, "Failed to get Rx IRQ\n");
			return -ENXIO;
		}

		pdma->chan[i - 1].rx_irq = irq;
	}

	return 0;
}

static int xgene_dma_probe(struct platform_device *pdev)
{
	struct xgene_dma *pdma;
	int ret, i;

	pdma = devm_kzalloc(&pdev->dev, sizeof(*pdma), GFP_KERNEL);
	if (!pdma)
		return -ENOMEM;

	pdma->dev = &pdev->dev;
	platform_set_drvdata(pdev, pdma);

	ret = xgene_dma_get_resources(pdev, pdma);
	if (ret)
		return ret;

	pdma->clk = devm_clk_get(&pdev->dev, NULL);
	if (IS_ERR(pdma->clk)) {
		dev_err(&pdev->dev, "Failed to get clk\n");
		return PTR_ERR(pdma->clk);
	}

	/* Enable clk before accessing registers */
	ret = clk_prepare_enable(pdma->clk);
	if (ret) {
		dev_err(&pdev->dev, "Failed to enable clk %d\n", ret);
		return ret;
	}

	/* Remove DMA RAM out of shutdown */
	ret = xgene_dma_init_mem(pdma);
	if (ret)
		goto err_clk_enable;

	ret = dma_set_mask_and_coherent(&pdev->dev, DMA_BIT_MASK(42));
	if (ret) {
		dev_err(&pdev->dev, "No usable DMA configuration\n");
		goto err_dma_mask;
	}

	/* Initialize DMA channels software state */
	xgene_dma_init_channels(pdma);

	/* Configue DMA rings */
	ret = xgene_dma_init_rings(pdma);
	if (ret)
		goto err_clk_enable;

	ret = xgene_dma_request_irqs(pdma);
	if (ret)
		goto err_request_irq;

	/* Configure and enable DMA engine */
	xgene_dma_init_hw(pdma);

	/* Register DMA device with linux async framework */
	ret = xgene_dma_init_async(pdma);
	if (ret)
		goto err_async_init;

	return 0;

err_async_init:
	xgene_dma_free_irqs(pdma);

err_request_irq:
	for (i = 0; i < XGENE_DMA_MAX_CHANNEL; i++)
		xgene_dma_delete_chan_rings(&pdma->chan[i]);

err_dma_mask:
err_clk_enable:
	clk_disable_unprepare(pdma->clk);

	return ret;
}

static int xgene_dma_remove(struct platform_device *pdev)
{
	struct xgene_dma *pdma = platform_get_drvdata(pdev);
	struct xgene_dma_chan *chan;
	int i;

	xgene_dma_async_unregister(pdma);

	/* Mask interrupts and disable DMA engine */
	xgene_dma_mask_interrupts(pdma);
	xgene_dma_disable(pdma);
	xgene_dma_free_irqs(pdma);

	for (i = 0; i < XGENE_DMA_MAX_CHANNEL; i++) {
		chan = &pdma->chan[i];
		tasklet_kill(&chan->tasklet);
		xgene_dma_delete_chan_rings(chan);
	}

	clk_disable_unprepare(pdma->clk);

	return 0;
}

static const struct of_device_id xgene_dma_of_match_ptr[] = {
	{.compatible = "apm,xgene-storm-dma",},
	{},
};
MODULE_DEVICE_TABLE(of, xgene_dma_of_match_ptr);

static struct platform_driver xgene_dma_driver = {
	.probe = xgene_dma_probe,
	.remove = xgene_dma_remove,
	.driver = {
		.name = "X-Gene-DMA",
		.of_match_table = xgene_dma_of_match_ptr,
	},
};

module_platform_driver(xgene_dma_driver);

MODULE_DESCRIPTION("APM X-Gene SoC DMA driver");
MODULE_AUTHOR("Rameshwar Prasad Sahu <rsahu@apm.com>");
MODULE_AUTHOR("Loc Ho <lho@apm.com>");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0");
