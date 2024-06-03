// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2012-2013, Xilinx, Michal Simek
 *
 * (C) Copyright 2012
 * Joe Hershberger <joe.hershberger@ni.com>
 */

#include <common.h>
#include <console.h>
#include <asm/io.h>
#include <fs.h>
#include <zynqpl.h>
#include <linux/sizes.h>
#include <asm/arch/hardware.h>
#include <asm/arch/sys_proto.h>

#define DEVCFG_CTRL_PCFG_PROG_B		0x40000000
#define DEVCFG_CTRL_PCFG_AES_EFUSE_MASK	0x00001000
#define DEVCFG_CTRL_PCAP_RATE_EN_MASK	0x02000000
#define DEVCFG_ISR_FATAL_ERROR_MASK	0x00740040
#define DEVCFG_ISR_ERROR_FLAGS_MASK	0x00340840
#define DEVCFG_ISR_RX_FIFO_OV		0x00040000
#define DEVCFG_ISR_DMA_DONE		0x00002000
#define DEVCFG_ISR_PCFG_DONE		0x00000004
#define DEVCFG_STATUS_DMA_CMD_Q_F	0x80000000
#define DEVCFG_STATUS_DMA_CMD_Q_E	0x40000000
#define DEVCFG_STATUS_DMA_DONE_CNT_MASK	0x30000000
#define DEVCFG_STATUS_PCFG_INIT		0x00000010
#define DEVCFG_MCTRL_PCAP_LPBK		0x00000010
#define DEVCFG_MCTRL_RFIFO_FLUSH	0x00000002
#define DEVCFG_MCTRL_WFIFO_FLUSH	0x00000001

#ifndef CONFIG_SYS_FPGA_WAIT
#define CONFIG_SYS_FPGA_WAIT CONFIG_SYS_HZ/100	/* 10 ms */
#endif

#ifndef CONFIG_SYS_FPGA_PROG_TIME
#define CONFIG_SYS_FPGA_PROG_TIME	(CONFIG_SYS_HZ * 4) /* 4 s */
#endif

#define DUMMY_WORD	0xffffffff

/* Xilinx binary format header */
static const u32 bin_format[] = {
	DUMMY_WORD, /* Dummy words */
	DUMMY_WORD,
	DUMMY_WORD,
	DUMMY_WORD,
	DUMMY_WORD,
	DUMMY_WORD,
	DUMMY_WORD,
	DUMMY_WORD,
	0x000000bb, /* Sync word */
	0x11220044, /* Sync word */
	DUMMY_WORD,
	DUMMY_WORD,
	0xaa995566, /* Sync word */
};

#define SWAP_NO		1
#define SWAP_DONE	2

/*
 * Load the whole word from unaligned buffer
 * Keep in your mind that it is byte loading on little-endian system
 */
static u32 load_word(const void *buf, u32 swap)
{
	u32 word = 0;
	u8 *bitc = (u8 *)buf;
	int p;

	if (swap == SWAP_NO) {
		for (p = 0; p < 4; p++) {
			word <<= 8;
			word |= bitc[p];
		}
	} else {
		for (p = 3; p >= 0; p--) {
			word <<= 8;
			word |= bitc[p];
		}
	}

	return word;
}

static u32 check_header(const void *buf)
{
	u32 i, pattern;
	int swap = SWAP_NO;
	u32 *test = (u32 *)buf;

	debug("%s: Let's check bitstream header\n", __func__);

	/* Checking that passing bin is not a bitstream */
	for (i = 0; i < ARRAY_SIZE(bin_format); i++) {
		pattern = load_word(&test[i], swap);

		/*
		 * Bitstreams in binary format are swapped
		 * compare to regular bistream.
		 * Do not swap dummy word but if swap is done assume
		 * that parsing buffer is binary format
		 */
		if ((__swab32(pattern) != DUMMY_WORD) &&
		    (__swab32(pattern) == bin_format[i])) {
			pattern = __swab32(pattern);
			swap = SWAP_DONE;
			debug("%s: data swapped - let's swap\n", __func__);
		}

		debug("%s: %d/%x: pattern %x/%x bin_format\n", __func__, i,
		      (u32)&test[i], pattern, bin_format[i]);
		if (pattern != bin_format[i]) {
			debug("%s: Bitstream is not recognized\n", __func__);
			return 0;
		}
	}
	debug("%s: Found bitstream header at %x %s swapinng\n", __func__,
	      (u32)buf, swap == SWAP_NO ? "without" : "with");

	return swap;
}

static void *check_data(u8 *buf, size_t bsize, u32 *swap)
{
	u32 word, p = 0; /* possition */

	/* Because buf doesn't need to be aligned let's read it by chars */
	for (p = 0; p < bsize; p++) {
		word = load_word(&buf[p], SWAP_NO);
		debug("%s: word %x %x/%x\n", __func__, word, p, (u32)&buf[p]);

		/* Find the first bitstream dummy word */
		if (word == DUMMY_WORD) {
			debug("%s: Found dummy word at position %x/%x\n",
			      __func__, p, (u32)&buf[p]);
			*swap = check_header(&buf[p]);
			if (*swap) {
				/* FIXME add full bitstream checking here */
				return &buf[p];
			}
		}
		/* Loop can be huge - support CTRL + C */
		if (ctrlc())
			return NULL;
	}
	return NULL;
}

static int zynq_dma_transfer(u32 srcbuf, u32 srclen, u32 dstbuf, u32 dstlen)
{
	unsigned long ts;
	u32 isr_status;

	/* Set up the transfer */
	writel((u32)srcbuf, &devcfg_base->dma_src_addr);
	writel(dstbuf, &devcfg_base->dma_dst_addr);
	writel(srclen, &devcfg_base->dma_src_len);
	writel(dstlen, &devcfg_base->dma_dst_len);

	isr_status = readl(&devcfg_base->int_sts);

	/* Polling the PCAP_INIT status for Set */
	ts = get_timer(0);
	while (!(isr_status & DEVCFG_ISR_DMA_DONE)) {
		if (isr_status & DEVCFG_ISR_ERROR_FLAGS_MASK) {
			debug("%s: Error: isr = 0x%08X\n", __func__,
			      isr_status);
			debug("%s: Write count = 0x%08X\n", __func__,
			      readl(&devcfg_base->write_count));
			debug("%s: Read count = 0x%08X\n", __func__,
			      readl(&devcfg_base->read_count));

			return FPGA_FAIL;
		}
		if (get_timer(ts) > CONFIG_SYS_FPGA_PROG_TIME) {
			printf("%s: Timeout wait for DMA to complete\n",
			       __func__);
			return FPGA_FAIL;
		}
		isr_status = readl(&devcfg_base->int_sts);
	}

	debug("%s: DMA transfer is done\n", __func__);

	/* Clear out the DMA status */
	writel(DEVCFG_ISR_DMA_DONE, &devcfg_base->int_sts);

	return FPGA_SUCCESS;
}

static int zynq_dma_xfer_init(bitstream_type bstype)
{
	u32 status, control, isr_status;
	unsigned long ts;

	/* Clear loopback bit */
	clrbits_le32(&devcfg_base->mctrl, DEVCFG_MCTRL_PCAP_LPBK);

	if (bstype != BIT_PARTIAL) {
		zynq_slcr_devcfg_disable();

		/* Setting PCFG_PROG_B signal to high */
		control = readl(&devcfg_base->ctrl);
		writel(control | DEVCFG_CTRL_PCFG_PROG_B, &devcfg_base->ctrl);

		/*
		 * Delay is required if AES efuse is selected as
		 * key source.
		 */
		if (control & DEVCFG_CTRL_PCFG_AES_EFUSE_MASK)
			mdelay(5);

		/* Setting PCFG_PROG_B signal to low */
		writel(control & ~DEVCFG_CTRL_PCFG_PROG_B, &devcfg_base->ctrl);

		/*
		 * Delay is required if AES efuse is selected as
		 * key source.
		 */
		if (control & DEVCFG_CTRL_PCFG_AES_EFUSE_MASK)
			mdelay(5);

		/* Polling the PCAP_INIT status for Reset */
		ts = get_timer(0);
		while (readl(&devcfg_base->status) & DEVCFG_STATUS_PCFG_INIT) {
			if (get_timer(ts) > CONFIG_SYS_FPGA_WAIT) {
				printf("%s: Timeout wait for INIT to clear\n",
				       __func__);
				return FPGA_FAIL;
			}
		}

		/* Setting PCFG_PROG_B signal to high */
		writel(control | DEVCFG_CTRL_PCFG_PROG_B, &devcfg_base->ctrl);

		/* Polling the PCAP_INIT status for Set */
		ts = get_timer(0);
		while (!(readl(&devcfg_base->status) &
			DEVCFG_STATUS_PCFG_INIT)) {
			if (get_timer(ts) > CONFIG_SYS_FPGA_WAIT) {
				printf("%s: Timeout wait for INIT to set\n",
				       __func__);
				return FPGA_FAIL;
			}
		}
	}

	isr_status = readl(&devcfg_base->int_sts);

	/* Clear it all, so if Boot ROM comes back, it can proceed */
	writel(0xFFFFFFFF, &devcfg_base->int_sts);

	if (isr_status & DEVCFG_ISR_FATAL_ERROR_MASK) {
		debug("%s: Fatal errors in PCAP 0x%X\n", __func__, isr_status);

		/* If RX FIFO overflow, need to flush RX FIFO first */
		if (isr_status & DEVCFG_ISR_RX_FIFO_OV) {
			writel(DEVCFG_MCTRL_RFIFO_FLUSH, &devcfg_base->mctrl);
			writel(0xFFFFFFFF, &devcfg_base->int_sts);
		}
		return FPGA_FAIL;
	}

	status = readl(&devcfg_base->status);

	debug("%s: Status = 0x%08X\n", __func__, status);

	if (status & DEVCFG_STATUS_DMA_CMD_Q_F) {
		debug("%s: Error: device busy\n", __func__);
		return FPGA_FAIL;
	}

	debug("%s: Device ready\n", __func__);

	if (!(status & DEVCFG_STATUS_DMA_CMD_Q_E)) {
		if (!(readl(&devcfg_base->int_sts) & DEVCFG_ISR_DMA_DONE)) {
			/* Error state, transfer cannot occur */
			debug("%s: ISR indicates error\n", __func__);
			return FPGA_FAIL;
		} else {
			/* Clear out the status */
			writel(DEVCFG_ISR_DMA_DONE, &devcfg_base->int_sts);
		}
	}

	if (status & DEVCFG_STATUS_DMA_DONE_CNT_MASK) {
		/* Clear the count of completed DMA transfers */
		writel(DEVCFG_STATUS_DMA_DONE_CNT_MASK, &devcfg_base->status);
	}

	return FPGA_SUCCESS;
}

static u32 *zynq_align_dma_buffer(u32 *buf, u32 len, u32 swap)
{
	u32 *new_buf;
	u32 i;

	if ((u32)buf != ALIGN((u32)buf, ARCH_DMA_MINALIGN)) {
		new_buf = (u32 *)ALIGN((u32)buf, ARCH_DMA_MINALIGN);

		/*
		 * This might be dangerous but permits to flash if
		 * ARCH_DMA_MINALIGN is greater than header size
		 */
		if (new_buf > buf) {
			debug("%s: Aligned buffer is after buffer start\n",
			      __func__);
			new_buf -= ARCH_DMA_MINALIGN;
		}
		printf("%s: Align buffer at %x to %x(swap %d)\n", __func__,
		       (u32)buf, (u32)new_buf, swap);

		for (i = 0; i < (len/4); i++)
			new_buf[i] = load_word(&buf[i], swap);

		buf = new_buf;
	} else if (swap != SWAP_DONE) {
		/* For bitstream which are aligned */
		u32 *new_buf = (u32 *)buf;

		printf("%s: Bitstream is not swapped(%d) - swap it\n", __func__,
		       swap);

		for (i = 0; i < (len/4); i++)
			new_buf[i] = load_word(&buf[i], swap);
	}

	return buf;
}

static int zynq_validate_bitstream(xilinx_desc *desc, const void *buf,
				   size_t bsize, u32 blocksize, u32 *swap,
				   bitstream_type *bstype)
{
	u32 *buf_start;
	u32 diff;

	buf_start = check_data((u8 *)buf, blocksize, swap);

	if (!buf_start)
		return FPGA_FAIL;

	/* Check if data is postpone from start */
	diff = (u32)buf_start - (u32)buf;
	if (diff) {
		printf("%s: Bitstream is not validated yet (diff %x)\n",
		       __func__, diff);
		return FPGA_FAIL;
	}

	if ((u32)buf < SZ_1M) {
		printf("%s: Bitstream has to be placed up to 1MB (%x)\n",
		       __func__, (u32)buf);
		return FPGA_FAIL;
	}

	if (zynq_dma_xfer_init(*bstype))
		return FPGA_FAIL;

	return 0;
}

static int zynq_load(xilinx_desc *desc, const void *buf, size_t bsize,
		     bitstream_type bstype)
{
	unsigned long ts; /* Timestamp */
	u32 isr_status, swap;

	/*
	 * send bsize inplace of blocksize as it was not a bitstream
	 * in chunks
	 */
	if (zynq_validate_bitstream(desc, buf, bsize, bsize, &swap,
				    &bstype))
		return FPGA_FAIL;

	buf = zynq_align_dma_buffer((u32 *)buf, bsize, swap);

	debug("%s: Source = 0x%08X\n", __func__, (u32)buf);
	debug("%s: Size = %zu\n", __func__, bsize);

	/* flush(clean & invalidate) d-cache range buf */
	flush_dcache_range((u32)buf, (u32)buf +
			   roundup(bsize, ARCH_DMA_MINALIGN));

	if (zynq_dma_transfer((u32)buf | 1, bsize >> 2, 0xffffffff, 0))
		return FPGA_FAIL;

	isr_status = readl(&devcfg_base->int_sts);
	/* Check FPGA configuration completion */
	ts = get_timer(0);
	while (!(isr_status & DEVCFG_ISR_PCFG_DONE)) {
		if (get_timer(ts) > CONFIG_SYS_FPGA_WAIT) {
			printf("%s: Timeout wait for FPGA to config\n",
			       __func__);
			return FPGA_FAIL;
		}
		isr_status = readl(&devcfg_base->int_sts);
	}

	debug("%s: FPGA config done\n", __func__);

	if (bstype != BIT_PARTIAL)
		zynq_slcr_devcfg_enable();

	puts("INFO:post config was not run, please run manually if needed\n");

	return FPGA_SUCCESS;
}

#if defined(CONFIG_CMD_FPGA_LOADFS) && !defined(CONFIG_SPL_BUILD)
static int zynq_loadfs(xilinx_desc *desc, const void *buf, size_t bsize,
		       fpga_fs_info *fsinfo)
{
	unsigned long ts; /* Timestamp */
	u32 isr_status, swap;
	u32 partialbit = 0;
	loff_t blocksize, actread;
	loff_t pos = 0;
	int fstype;
	char *interface, *dev_part;
	const char *filename;

	blocksize = fsinfo->blocksize;
	interface = fsinfo->interface;
	dev_part = fsinfo->dev_part;
	filename = fsinfo->filename;
	fstype = fsinfo->fstype;

	if (fs_set_blk_dev(interface, dev_part, fstype))
		return FPGA_FAIL;

	if (fs_read(filename, (u32) buf, pos, blocksize, &actread) < 0)
		return FPGA_FAIL;

	if (zynq_validate_bitstream(desc, buf, bsize, blocksize, &swap,
				    &partialbit))
		return FPGA_FAIL;

	dcache_disable();

	do {
		buf = zynq_align_dma_buffer((u32 *)buf, blocksize, swap);

		if (zynq_dma_transfer((u32)buf | 1, blocksize >> 2,
				      0xffffffff, 0))
			return FPGA_FAIL;

		bsize -= blocksize;
		pos   += blocksize;

		if (fs_set_blk_dev(interface, dev_part, fstype))
			return FPGA_FAIL;

		if (bsize > blocksize) {
			if (fs_read(filename, (u32) buf, pos, blocksize, &actread) < 0)
				return FPGA_FAIL;
		} else {
			if (fs_read(filename, (u32) buf, pos, bsize, &actread) < 0)
				return FPGA_FAIL;
		}
	} while (bsize > blocksize);

	buf = zynq_align_dma_buffer((u32 *)buf, blocksize, swap);

	if (zynq_dma_transfer((u32)buf | 1, bsize >> 2, 0xffffffff, 0))
		return FPGA_FAIL;

	dcache_enable();

	isr_status = readl(&devcfg_base->int_sts);

	/* Check FPGA configuration completion */
	ts = get_timer(0);
	while (!(isr_status & DEVCFG_ISR_PCFG_DONE)) {
		if (get_timer(ts) > CONFIG_SYS_FPGA_WAIT) {
			printf("%s: Timeout wait for FPGA to config\n",
			       __func__);
			return FPGA_FAIL;
		}
		isr_status = readl(&devcfg_base->int_sts);
	}

	debug("%s: FPGA config done\n", __func__);

	if (!partialbit)
		zynq_slcr_devcfg_enable();

	return FPGA_SUCCESS;
}
#endif

struct xilinx_fpga_op zynq_op = {
	.load = zynq_load,
#if defined(CONFIG_CMD_FPGA_LOADFS) && !defined(CONFIG_SPL_BUILD)
	.loadfs = zynq_loadfs,
#endif
};

#ifdef CONFIG_CMD_ZYNQ_AES
/*
 * Load the encrypted image from src addr and decrypt the image and
 * place it back the decrypted image into dstaddr.
 */
int zynq_decrypt_load(u32 srcaddr, u32 srclen, u32 dstaddr, u32 dstlen)
{
	if (srcaddr < SZ_1M || dstaddr < SZ_1M) {
		printf("%s: src and dst addr should be > 1M\n",
		       __func__);
		return FPGA_FAIL;
	}

	if (zynq_dma_xfer_init(BIT_NONE)) {
		printf("%s: zynq_dma_xfer_init FAIL\n", __func__);
		return FPGA_FAIL;
	}

	writel((readl(&devcfg_base->ctrl) | DEVCFG_CTRL_PCAP_RATE_EN_MASK),
	       &devcfg_base->ctrl);

	debug("%s: Source = 0x%08X\n", __func__, (u32)srcaddr);
	debug("%s: Size = %zu\n", __func__, srclen);

	/* flush(clean & invalidate) d-cache range buf */
	flush_dcache_range((u32)srcaddr, (u32)srcaddr +
			roundup(srclen << 2, ARCH_DMA_MINALIGN));
	/*
	 * Flush destination address range only if image is not
	 * bitstream.
	 */
	flush_dcache_range((u32)dstaddr, (u32)dstaddr +
			   roundup(dstlen << 2, ARCH_DMA_MINALIGN));

	if (zynq_dma_transfer(srcaddr | 1, srclen, dstaddr | 1, dstlen))
		return FPGA_FAIL;

	writel((readl(&devcfg_base->ctrl) & ~DEVCFG_CTRL_PCAP_RATE_EN_MASK),
	       &devcfg_base->ctrl);

	return FPGA_SUCCESS;
}
#endif
