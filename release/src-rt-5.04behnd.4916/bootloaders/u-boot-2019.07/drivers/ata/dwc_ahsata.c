// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2010-2011 Freescale Semiconductor, Inc.
 * Terry Lv <r65388@freescale.com>
 */

#include <common.h>
#include <ahci.h>
#include <dm.h>
#include <dwc_ahsata.h>
#include <fis.h>
#include <libata.h>
#include <malloc.h>
#include <memalign.h>
#include <sata.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/sys_proto.h>
#include <asm/mach-imx/sata.h>
#include <linux/bitops.h>
#include <linux/ctype.h>
#include <linux/errno.h>
#include "dwc_ahsata_priv.h"

struct sata_port_regs {
	u32 clb;
	u32 clbu;
	u32 fb;
	u32 fbu;
	u32 is;
	u32 ie;
	u32 cmd;
	u32 res1[1];
	u32 tfd;
	u32 sig;
	u32 ssts;
	u32 sctl;
	u32 serr;
	u32 sact;
	u32 ci;
	u32 sntf;
	u32 res2[1];
	u32 dmacr;
	u32 res3[1];
	u32 phycr;
	u32 physr;
};

struct sata_host_regs {
	u32 cap;
	u32 ghc;
	u32 is;
	u32 pi;
	u32 vs;
	u32 ccc_ctl;
	u32 ccc_ports;
	u32 res1[2];
	u32 cap2;
	u32 res2[30];
	u32 bistafr;
	u32 bistcr;
	u32 bistfctr;
	u32 bistsr;
	u32 bistdecr;
	u32 res3[2];
	u32 oobr;
	u32 res4[8];
	u32 timer1ms;
	u32 res5[1];
	u32 gparam1r;
	u32 gparam2r;
	u32 pparamr;
	u32 testr;
	u32 versionr;
	u32 idr;
};

#define MAX_DATA_BYTES_PER_SG  (4 * 1024 * 1024)
#define MAX_BYTES_PER_TRANS (AHCI_MAX_SG * MAX_DATA_BYTES_PER_SG)

#define writel_with_flush(a, b)	do { writel(a, b); readl(b); } while (0)

static inline void __iomem *ahci_port_base(void __iomem *base, u32 port)
{
	return base + 0x100 + (port * 0x80);
}

static int waiting_for_cmd_completed(u8 *offset,
					int timeout_msec,
					u32 sign)
{
	int i;
	u32 status;

	for (i = 0;
		((status = readl(offset)) & sign) && i < timeout_msec;
		++i)
		mdelay(1);

	return (i < timeout_msec) ? 0 : -1;
}

static int ahci_setup_oobr(struct ahci_uc_priv *uc_priv, int clk)
{
	struct sata_host_regs *host_mmio = uc_priv->mmio_base;

	writel(SATA_HOST_OOBR_WE, &host_mmio->oobr);
	writel(0x02060b14, &host_mmio->oobr);

	return 0;
}

static int ahci_host_init(struct ahci_uc_priv *uc_priv)
{
	u32 tmp, cap_save, num_ports;
	int i, j, timeout = 1000;
	struct sata_port_regs *port_mmio = NULL;
	struct sata_host_regs *host_mmio = uc_priv->mmio_base;
	int clk = mxc_get_clock(MXC_SATA_CLK);

	cap_save = readl(&host_mmio->cap);
	cap_save |= SATA_HOST_CAP_SSS;

	/* global controller reset */
	tmp = readl(&host_mmio->ghc);
	if ((tmp & SATA_HOST_GHC_HR) == 0)
		writel_with_flush(tmp | SATA_HOST_GHC_HR, &host_mmio->ghc);

	while ((readl(&host_mmio->ghc) & SATA_HOST_GHC_HR) && --timeout)
		;

	if (timeout <= 0) {
		debug("controller reset failed (0x%x)\n", tmp);
		return -1;
	}

	/* Set timer 1ms */
	writel(clk / 1000, &host_mmio->timer1ms);

	ahci_setup_oobr(uc_priv, 0);

	writel_with_flush(SATA_HOST_GHC_AE, &host_mmio->ghc);
	writel(cap_save, &host_mmio->cap);
	num_ports = (cap_save & SATA_HOST_CAP_NP_MASK) + 1;
	writel_with_flush((1 << num_ports) - 1, &host_mmio->pi);

	/*
	 * Determine which Ports are implemented by the DWC_ahsata,
	 * by reading the PI register. This bit map value aids the
	 * software to determine how many Ports are available and
	 * which Port registers need to be initialized.
	 */
	uc_priv->cap = readl(&host_mmio->cap);
	uc_priv->port_map = readl(&host_mmio->pi);

	/* Determine how many command slots the HBA supports */
	uc_priv->n_ports = (uc_priv->cap & SATA_HOST_CAP_NP_MASK) + 1;

	debug("cap 0x%x  port_map 0x%x  n_ports %d\n",
		uc_priv->cap, uc_priv->port_map, uc_priv->n_ports);

	for (i = 0; i < uc_priv->n_ports; i++) {
		uc_priv->port[i].port_mmio = ahci_port_base(host_mmio, i);
		port_mmio = uc_priv->port[i].port_mmio;

		/* Ensure that the DWC_ahsata is in idle state */
		tmp = readl(&port_mmio->cmd);

		/*
		 * When P#CMD.ST, P#CMD.CR, P#CMD.FRE and P#CMD.FR
		 * are all cleared, the Port is in an idle state.
		 */
		if (tmp & (SATA_PORT_CMD_CR | SATA_PORT_CMD_FR |
			SATA_PORT_CMD_FRE | SATA_PORT_CMD_ST)) {

			/*
			 * System software places a Port into the idle state by
			 * clearing P#CMD.ST and waiting for P#CMD.CR to return
			 * 0 when read.
			 */
			tmp &= ~SATA_PORT_CMD_ST;
			writel_with_flush(tmp, &port_mmio->cmd);

			/*
			 * spec says 500 msecs for each bit, so
			 * this is slightly incorrect.
			 */
			mdelay(500);

			timeout = 1000;
			while ((readl(&port_mmio->cmd) & SATA_PORT_CMD_CR)
				&& --timeout)
				;

			if (timeout <= 0) {
				debug("port reset failed (0x%x)\n", tmp);
				return -1;
			}
		}

		/* Spin-up device */
		tmp = readl(&port_mmio->cmd);
		writel((tmp | SATA_PORT_CMD_SUD), &port_mmio->cmd);

		/* Wait for spin-up to finish */
		timeout = 1000;
		while (!(readl(&port_mmio->cmd) | SATA_PORT_CMD_SUD)
			&& --timeout)
			;
		if (timeout <= 0) {
			debug("Spin-Up can't finish!\n");
			return -1;
		}

		for (j = 0; j < 100; ++j) {
			mdelay(10);
			tmp = readl(&port_mmio->ssts);
			if (((tmp & SATA_PORT_SSTS_DET_MASK) == 0x3) ||
				((tmp & SATA_PORT_SSTS_DET_MASK) == 0x1))
				break;
		}

		/* Wait for COMINIT bit 26 (DIAG_X) in SERR */
		timeout = 1000;
		while (!(readl(&port_mmio->serr) | SATA_PORT_SERR_DIAG_X)
			&& --timeout)
			;
		if (timeout <= 0) {
			debug("Can't find DIAG_X set!\n");
			return -1;
		}

		/*
		 * For each implemented Port, clear the P#SERR
		 * register, by writing ones to each implemented\
		 * bit location.
		 */
		tmp = readl(&port_mmio->serr);
		debug("P#SERR 0x%x\n",
				tmp);
		writel(tmp, &port_mmio->serr);

		/* Ack any pending irq events for this port */
		tmp = readl(&host_mmio->is);
		debug("IS 0x%x\n", tmp);
		if (tmp)
			writel(tmp, &host_mmio->is);

		writel(1 << i, &host_mmio->is);

		/* set irq mask (enables interrupts) */
		writel(DEF_PORT_IRQ, &port_mmio->ie);

		/* register linkup ports */
		tmp = readl(&port_mmio->ssts);
		debug("Port %d status: 0x%x\n", i, tmp);
		if ((tmp & SATA_PORT_SSTS_DET_MASK) == 0x03)
			uc_priv->link_port_map |= (0x01 << i);
	}

	tmp = readl(&host_mmio->ghc);
	debug("GHC 0x%x\n", tmp);
	writel(tmp | SATA_HOST_GHC_IE, &host_mmio->ghc);
	tmp = readl(&host_mmio->ghc);
	debug("GHC 0x%x\n", tmp);

	return 0;
}

static void ahci_print_info(struct ahci_uc_priv *uc_priv)
{
	struct sata_host_regs *host_mmio = uc_priv->mmio_base;
	u32 vers, cap, impl, speed;
	const char *speed_s;
	const char *scc_s;

	vers = readl(&host_mmio->vs);
	cap = uc_priv->cap;
	impl = uc_priv->port_map;

	speed = (cap & SATA_HOST_CAP_ISS_MASK)
		>> SATA_HOST_CAP_ISS_OFFSET;
	if (speed == 1)
		speed_s = "1.5";
	else if (speed == 2)
		speed_s = "3";
	else
		speed_s = "?";

	scc_s = "SATA";

	printf("AHCI %02x%02x.%02x%02x "
		"%u slots %u ports %s Gbps 0x%x impl %s mode\n",
		(vers >> 24) & 0xff,
		(vers >> 16) & 0xff,
		(vers >> 8) & 0xff,
		vers & 0xff,
		((cap >> 8) & 0x1f) + 1,
		(cap & 0x1f) + 1,
		speed_s,
		impl,
		scc_s);

	printf("flags: "
		"%s%s%s%s%s%s"
		"%s%s%s%s%s%s%s\n",
		cap & (1 << 31) ? "64bit " : "",
		cap & (1 << 30) ? "ncq " : "",
		cap & (1 << 28) ? "ilck " : "",
		cap & (1 << 27) ? "stag " : "",
		cap & (1 << 26) ? "pm " : "",
		cap & (1 << 25) ? "led " : "",
		cap & (1 << 24) ? "clo " : "",
		cap & (1 << 19) ? "nz " : "",
		cap & (1 << 18) ? "only " : "",
		cap & (1 << 17) ? "pmp " : "",
		cap & (1 << 15) ? "pio " : "",
		cap & (1 << 14) ? "slum " : "",
		cap & (1 << 13) ? "part " : "");
}

static int ahci_fill_sg(struct ahci_uc_priv *uc_priv, u8 port,
			unsigned char *buf, int buf_len)
{
	struct ahci_ioports *pp = &uc_priv->port[port];
	struct ahci_sg *ahci_sg = pp->cmd_tbl_sg;
	u32 sg_count, max_bytes;
	int i;

	max_bytes = MAX_DATA_BYTES_PER_SG;
	sg_count = ((buf_len - 1) / max_bytes) + 1;
	if (sg_count > AHCI_MAX_SG) {
		printf("Error:Too much sg!\n");
		return -1;
	}

	for (i = 0; i < sg_count; i++) {
		ahci_sg->addr =
			cpu_to_le32((u32)buf + i * max_bytes);
		ahci_sg->addr_hi = 0;
		ahci_sg->flags_size = cpu_to_le32(0x3fffff &
					(buf_len < max_bytes
					? (buf_len - 1)
					: (max_bytes - 1)));
		ahci_sg++;
		buf_len -= max_bytes;
	}

	return sg_count;
}

static void ahci_fill_cmd_slot(struct ahci_ioports *pp, u32 cmd_slot, u32 opts)
{
	struct ahci_cmd_hdr *cmd_hdr = (struct ahci_cmd_hdr *)(pp->cmd_slot +
					AHCI_CMD_SLOT_SZ * cmd_slot);

	memset(cmd_hdr, 0, AHCI_CMD_SLOT_SZ);
	cmd_hdr->opts = cpu_to_le32(opts);
	cmd_hdr->status = 0;
	pp->cmd_slot->tbl_addr = cpu_to_le32((u32)pp->cmd_tbl & 0xffffffff);
#ifdef CONFIG_PHYS_64BIT
	pp->cmd_slot->tbl_addr_hi =
	    cpu_to_le32((u32)(((pp->cmd_tbl) >> 16) >> 16));
#endif
}

#define AHCI_GET_CMD_SLOT(c) ((c) ? ffs(c) : 0)

static int ahci_exec_ata_cmd(struct ahci_uc_priv *uc_priv, u8 port,
			     struct sata_fis_h2d *cfis, u8 *buf, u32 buf_len,
			     s32 is_write)
{
	struct ahci_ioports *pp = &uc_priv->port[port];
	struct sata_port_regs *port_mmio = pp->port_mmio;
	u32 opts;
	int sg_count = 0, cmd_slot = 0;

	cmd_slot = AHCI_GET_CMD_SLOT(readl(&port_mmio->ci));
	if (32 == cmd_slot) {
		printf("Can't find empty command slot!\n");
		return 0;
	}

	/* Check xfer length */
	if (buf_len > MAX_BYTES_PER_TRANS) {
		printf("Max transfer length is %dB\n\r",
			MAX_BYTES_PER_TRANS);
		return 0;
	}

	memcpy((u8 *)(pp->cmd_tbl), cfis, sizeof(struct sata_fis_h2d));
	if (buf && buf_len)
		sg_count = ahci_fill_sg(uc_priv, port, buf, buf_len);
	opts = (sizeof(struct sata_fis_h2d) >> 2) | (sg_count << 16);
	if (is_write) {
		opts |= 0x40;
		flush_cache((ulong)buf, buf_len);
	}
	ahci_fill_cmd_slot(pp, cmd_slot, opts);

	flush_cache((int)(pp->cmd_slot), AHCI_PORT_PRIV_DMA_SZ);
	writel_with_flush(1 << cmd_slot, &port_mmio->ci);

	if (waiting_for_cmd_completed((u8 *)&port_mmio->ci, 10000,
				      0x1 << cmd_slot)) {
		printf("timeout exit!\n");
		return -1;
	}
	invalidate_dcache_range((int)(pp->cmd_slot),
				(int)(pp->cmd_slot)+AHCI_PORT_PRIV_DMA_SZ);
	debug("ahci_exec_ata_cmd: %d byte transferred.\n",
	      pp->cmd_slot->status);
	if (!is_write)
		invalidate_dcache_range((ulong)buf, (ulong)buf+buf_len);

	return buf_len;
}

static void ahci_set_feature(struct ahci_uc_priv *uc_priv, u8 port)
{
	struct sata_fis_h2d h2d __aligned(ARCH_DMA_MINALIGN);
	struct sata_fis_h2d *cfis = &h2d;

	memset(cfis, 0, sizeof(struct sata_fis_h2d));
	cfis->fis_type = SATA_FIS_TYPE_REGISTER_H2D;
	cfis->pm_port_c = 1 << 7;
	cfis->command = ATA_CMD_SET_FEATURES;
	cfis->features = SETFEATURES_XFER;
	cfis->sector_count = ffs(uc_priv->udma_mask + 1) + 0x3e;

	ahci_exec_ata_cmd(uc_priv, port, cfis, NULL, 0, READ_CMD);
}

static int ahci_port_start(struct ahci_uc_priv *uc_priv, u8 port)
{
	struct ahci_ioports *pp = &uc_priv->port[port];
	struct sata_port_regs *port_mmio = pp->port_mmio;
	u32 port_status;
	u32 mem;
	int timeout = 10000000;

	debug("Enter start port: %d\n", port);
	port_status = readl(&port_mmio->ssts);
	debug("Port %d status: %x\n", port, port_status);
	if ((port_status & 0xf) != 0x03) {
		printf("No Link on this port!\n");
		return -1;
	}

	mem = (u32)malloc(AHCI_PORT_PRIV_DMA_SZ + 1024);
	if (!mem) {
		free(pp);
		printf("No mem for table!\n");
		return -ENOMEM;
	}

	mem = (mem + 0x400) & (~0x3ff);	/* Aligned to 1024-bytes */
	memset((u8 *)mem, 0, AHCI_PORT_PRIV_DMA_SZ);

	/*
	 * First item in chunk of DMA memory: 32-slot command table,
	 * 32 bytes each in size
	 */
	pp->cmd_slot = (struct ahci_cmd_hdr *)mem;
	debug("cmd_slot = 0x%x\n", (unsigned int) pp->cmd_slot);
	mem += (AHCI_CMD_SLOT_SZ * DWC_AHSATA_MAX_CMD_SLOTS);

	/*
	 * Second item: Received-FIS area, 256-Byte aligned
	 */
	pp->rx_fis = mem;
	mem += AHCI_RX_FIS_SZ;

	/*
	 * Third item: data area for storing a single command
	 * and its scatter-gather table
	 */
	pp->cmd_tbl = mem;
	debug("cmd_tbl_dma = 0x%lx\n", pp->cmd_tbl);

	mem += AHCI_CMD_TBL_HDR;

	writel_with_flush(0x00004444, &port_mmio->dmacr);
	pp->cmd_tbl_sg = (struct ahci_sg *)mem;
	writel_with_flush((u32)pp->cmd_slot, &port_mmio->clb);
	writel_with_flush(pp->rx_fis, &port_mmio->fb);

	/* Enable FRE */
	writel_with_flush((SATA_PORT_CMD_FRE | readl(&port_mmio->cmd)),
			  &port_mmio->cmd);

	/* Wait device ready */
	while ((readl(&port_mmio->tfd) & (SATA_PORT_TFD_STS_ERR |
		SATA_PORT_TFD_STS_DRQ | SATA_PORT_TFD_STS_BSY))
		&& --timeout)
		;
	if (timeout <= 0) {
		debug("Device not ready for BSY, DRQ and"
			"ERR in TFD!\n");
		return -1;
	}

	writel_with_flush(PORT_CMD_ICC_ACTIVE | PORT_CMD_FIS_RX |
			  PORT_CMD_POWER_ON | PORT_CMD_SPIN_UP |
			  PORT_CMD_START, &port_mmio->cmd);

	debug("Exit start port %d\n", port);

	return 0;
}

static void dwc_ahsata_print_info(struct blk_desc *pdev)
{
	printf("SATA Device Info:\n\r");
	printf("S/N: %s\n\rProduct model number: %s\n\r"
		"Firmware version: %s\n\rCapacity: " LBAFU " sectors\n\r",
		pdev->product, pdev->vendor, pdev->revision, pdev->lba);
}

static void dwc_ahsata_identify(struct ahci_uc_priv *uc_priv, u16 *id)
{
	struct sata_fis_h2d h2d __aligned(ARCH_DMA_MINALIGN);
	struct sata_fis_h2d *cfis = &h2d;
	u8 port = uc_priv->hard_port_no;

	memset(cfis, 0, sizeof(struct sata_fis_h2d));

	cfis->fis_type = SATA_FIS_TYPE_REGISTER_H2D;
	cfis->pm_port_c = 0x80; /* is command */
	cfis->command = ATA_CMD_ID_ATA;

	ahci_exec_ata_cmd(uc_priv, port, cfis, (u8 *)id, ATA_ID_WORDS * 2,
			  READ_CMD);
	ata_swap_buf_le16(id, ATA_ID_WORDS);
}

static void dwc_ahsata_xfer_mode(struct ahci_uc_priv *uc_priv, u16 *id)
{
	uc_priv->pio_mask = id[ATA_ID_PIO_MODES];
	uc_priv->udma_mask = id[ATA_ID_UDMA_MODES];
	debug("pio %04x, udma %04x\n\r", uc_priv->pio_mask, uc_priv->udma_mask);
}

static u32 dwc_ahsata_rw_cmd(struct ahci_uc_priv *uc_priv, u32 start,
			     u32 blkcnt, u8 *buffer, int is_write)
{
	struct sata_fis_h2d h2d __aligned(ARCH_DMA_MINALIGN);
	struct sata_fis_h2d *cfis = &h2d;
	u8 port = uc_priv->hard_port_no;
	u32 block;

	block = start;

	memset(cfis, 0, sizeof(struct sata_fis_h2d));

	cfis->fis_type = SATA_FIS_TYPE_REGISTER_H2D;
	cfis->pm_port_c = 0x80; /* is command */
	cfis->command = (is_write) ? ATA_CMD_WRITE : ATA_CMD_READ;
	cfis->device = ATA_LBA;

	cfis->device |= (block >> 24) & 0xf;
	cfis->lba_high = (block >> 16) & 0xff;
	cfis->lba_mid = (block >> 8) & 0xff;
	cfis->lba_low = block & 0xff;
	cfis->sector_count = (u8)(blkcnt & 0xff);

	if (ahci_exec_ata_cmd(uc_priv, port, cfis, buffer,
			      ATA_SECT_SIZE * blkcnt, is_write) > 0)
		return blkcnt;
	else
		return 0;
}

static void dwc_ahsata_flush_cache(struct ahci_uc_priv *uc_priv)
{
	struct sata_fis_h2d h2d __aligned(ARCH_DMA_MINALIGN);
	struct sata_fis_h2d *cfis = &h2d;
	u8 port = uc_priv->hard_port_no;

	memset(cfis, 0, sizeof(struct sata_fis_h2d));

	cfis->fis_type = SATA_FIS_TYPE_REGISTER_H2D;
	cfis->pm_port_c = 0x80; /* is command */
	cfis->command = ATA_CMD_FLUSH;

	ahci_exec_ata_cmd(uc_priv, port, cfis, NULL, 0, 0);
}

static u32 dwc_ahsata_rw_cmd_ext(struct ahci_uc_priv *uc_priv, u32 start,
				 lbaint_t blkcnt, u8 *buffer, int is_write)
{
	struct sata_fis_h2d h2d __aligned(ARCH_DMA_MINALIGN);
	struct sata_fis_h2d *cfis = &h2d;
	u8 port = uc_priv->hard_port_no;
	u64 block;

	block = (u64)start;

	memset(cfis, 0, sizeof(struct sata_fis_h2d));

	cfis->fis_type = SATA_FIS_TYPE_REGISTER_H2D;
	cfis->pm_port_c = 0x80; /* is command */

	cfis->command = (is_write) ? ATA_CMD_WRITE_EXT
				 : ATA_CMD_READ_EXT;

	cfis->lba_high_exp = (block >> 40) & 0xff;
	cfis->lba_mid_exp = (block >> 32) & 0xff;
	cfis->lba_low_exp = (block >> 24) & 0xff;
	cfis->lba_high = (block >> 16) & 0xff;
	cfis->lba_mid = (block >> 8) & 0xff;
	cfis->lba_low = block & 0xff;
	cfis->device = ATA_LBA;
	cfis->sector_count_exp = (blkcnt >> 8) & 0xff;
	cfis->sector_count = blkcnt & 0xff;

	if (ahci_exec_ata_cmd(uc_priv, port, cfis, buffer,
			      ATA_SECT_SIZE * blkcnt, is_write) > 0)
		return blkcnt;
	else
		return 0;
}

static void dwc_ahsata_flush_cache_ext(struct ahci_uc_priv *uc_priv)
{
	struct sata_fis_h2d h2d __aligned(ARCH_DMA_MINALIGN);
	struct sata_fis_h2d *cfis = &h2d;
	u8 port = uc_priv->hard_port_no;

	memset(cfis, 0, sizeof(struct sata_fis_h2d));

	cfis->fis_type = SATA_FIS_TYPE_REGISTER_H2D;
	cfis->pm_port_c = 0x80; /* is command */
	cfis->command = ATA_CMD_FLUSH_EXT;

	ahci_exec_ata_cmd(uc_priv, port, cfis, NULL, 0, 0);
}

static void dwc_ahsata_init_wcache(struct ahci_uc_priv *uc_priv, u16 *id)
{
	if (ata_id_has_wcache(id) && ata_id_wcache_enabled(id))
		uc_priv->flags |= SATA_FLAG_WCACHE;
	if (ata_id_has_flush(id))
		uc_priv->flags |= SATA_FLAG_FLUSH;
	if (ata_id_has_flush_ext(id))
		uc_priv->flags |= SATA_FLAG_FLUSH_EXT;
}

static u32 ata_low_level_rw_lba48(struct ahci_uc_priv *uc_priv, u32 blknr,
				  lbaint_t blkcnt, const void *buffer,
				  int is_write)
{
	u32 start, blks;
	u8 *addr;
	int max_blks;

	start = blknr;
	blks = blkcnt;
	addr = (u8 *)buffer;

	max_blks = ATA_MAX_SECTORS_LBA48;

	do {
		if (blks > max_blks) {
			if (max_blks != dwc_ahsata_rw_cmd_ext(uc_priv, start,
							      max_blks, addr,
							      is_write))
				return 0;
			start += max_blks;
			blks -= max_blks;
			addr += ATA_SECT_SIZE * max_blks;
		} else {
			if (blks != dwc_ahsata_rw_cmd_ext(uc_priv, start, blks,
							  addr, is_write))
				return 0;
			start += blks;
			blks = 0;
			addr += ATA_SECT_SIZE * blks;
		}
	} while (blks != 0);

	return blkcnt;
}

static u32 ata_low_level_rw_lba28(struct ahci_uc_priv *uc_priv, u32 blknr,
				  lbaint_t blkcnt, const void *buffer,
				  int is_write)
{
	u32 start, blks;
	u8 *addr;
	int max_blks;

	start = blknr;
	blks = blkcnt;
	addr = (u8 *)buffer;

	max_blks = ATA_MAX_SECTORS;
	do {
		if (blks > max_blks) {
			if (max_blks != dwc_ahsata_rw_cmd(uc_priv, start,
							  max_blks, addr,
							  is_write))
				return 0;
			start += max_blks;
			blks -= max_blks;
			addr += ATA_SECT_SIZE * max_blks;
		} else {
			if (blks != dwc_ahsata_rw_cmd(uc_priv, start, blks,
						      addr, is_write))
				return 0;
			start += blks;
			blks = 0;
			addr += ATA_SECT_SIZE * blks;
		}
	} while (blks != 0);

	return blkcnt;
}

static int dwc_ahci_start_ports(struct ahci_uc_priv *uc_priv)
{
	u32 linkmap;
	int i;

	linkmap = uc_priv->link_port_map;

	if (0 == linkmap) {
		printf("No port device detected!\n");
		return -ENXIO;
	}

	for (i = 0; i < uc_priv->n_ports; i++) {
		if ((linkmap >> i) && ((linkmap >> i) & 0x01)) {
			if (ahci_port_start(uc_priv, (u8)i)) {
				printf("Can not start port %d\n", i);
				return 1;
			}
			uc_priv->hard_port_no = i;
			break;
		}
	}

	return 0;
}

static int dwc_ahsata_scan_common(struct ahci_uc_priv *uc_priv,
				  struct blk_desc *pdev)
{
	u8 serial[ATA_ID_SERNO_LEN + 1] = { 0 };
	u8 firmware[ATA_ID_FW_REV_LEN + 1] = { 0 };
	u8 product[ATA_ID_PROD_LEN + 1] = { 0 };
	u8 port = uc_priv->hard_port_no;
	ALLOC_CACHE_ALIGN_BUFFER(u16, id, ATA_ID_WORDS);

	/* Identify device to get information */
	dwc_ahsata_identify(uc_priv, id);

	/* Serial number */
	ata_id_c_string(id, serial, ATA_ID_SERNO, sizeof(serial));
	memcpy(pdev->product, serial, sizeof(serial));

	/* Firmware version */
	ata_id_c_string(id, firmware, ATA_ID_FW_REV, sizeof(firmware));
	memcpy(pdev->revision, firmware, sizeof(firmware));

	/* Product model */
	ata_id_c_string(id, product, ATA_ID_PROD, sizeof(product));
	memcpy(pdev->vendor, product, sizeof(product));

	/* Total sectors */
	pdev->lba = ata_id_n_sectors(id);

	pdev->type = DEV_TYPE_HARDDISK;
	pdev->blksz = ATA_SECT_SIZE;
	pdev->lun = 0;

	/* Check if support LBA48 */
	if (ata_id_has_lba48(id)) {
		pdev->lba48 = 1;
		debug("Device support LBA48\n\r");
	}

	/* Get the NCQ queue depth from device */
	uc_priv->flags &= (~SATA_FLAG_Q_DEP_MASK);
	uc_priv->flags |= ata_id_queue_depth(id);

	/* Get the xfer mode from device */
	dwc_ahsata_xfer_mode(uc_priv, id);

	/* Get the write cache status from device */
	dwc_ahsata_init_wcache(uc_priv, id);

	/* Set the xfer mode to highest speed */
	ahci_set_feature(uc_priv, port);

	dwc_ahsata_print_info(pdev);

	return 0;
}

/*
 * SATA interface between low level driver and command layer
 */
static ulong sata_read_common(struct ahci_uc_priv *uc_priv,
			      struct blk_desc *desc, ulong blknr,
			      lbaint_t blkcnt, void *buffer)
{
	u32 rc;

	if (desc->lba48)
		rc = ata_low_level_rw_lba48(uc_priv, blknr, blkcnt, buffer,
					    READ_CMD);
	else
		rc = ata_low_level_rw_lba28(uc_priv, blknr, blkcnt, buffer,
					    READ_CMD);

	return rc;
}

static ulong sata_write_common(struct ahci_uc_priv *uc_priv,
			       struct blk_desc *desc, ulong blknr,
			       lbaint_t blkcnt, const void *buffer)
{
	u32 rc;
	u32 flags = uc_priv->flags;

	if (desc->lba48) {
		rc = ata_low_level_rw_lba48(uc_priv, blknr, blkcnt, buffer,
					    WRITE_CMD);
		if ((flags & SATA_FLAG_WCACHE) && (flags & SATA_FLAG_FLUSH_EXT))
			dwc_ahsata_flush_cache_ext(uc_priv);
	} else {
		rc = ata_low_level_rw_lba28(uc_priv, blknr, blkcnt, buffer,
					    WRITE_CMD);
		if ((flags & SATA_FLAG_WCACHE) && (flags & SATA_FLAG_FLUSH))
			dwc_ahsata_flush_cache(uc_priv);
	}

	return rc;
}

#if !CONFIG_IS_ENABLED(AHCI)
static int ahci_init_one(int pdev)
{
	int rc;
	struct ahci_uc_priv *uc_priv = NULL;

	uc_priv = malloc(sizeof(struct ahci_uc_priv));
	memset(uc_priv, 0, sizeof(struct ahci_uc_priv));
	uc_priv->dev = pdev;

	uc_priv->host_flags = ATA_FLAG_SATA
				| ATA_FLAG_NO_LEGACY
				| ATA_FLAG_MMIO
				| ATA_FLAG_PIO_DMA
				| ATA_FLAG_NO_ATAPI;

	uc_priv->mmio_base = (void __iomem *)CONFIG_DWC_AHSATA_BASE_ADDR;

	/* initialize adapter */
	rc = ahci_host_init(uc_priv);
	if (rc)
		goto err_out;

	ahci_print_info(uc_priv);

	/* Save the uc_private struct to block device struct */
	sata_dev_desc[pdev].priv = uc_priv;

	return 0;

err_out:
	return rc;
}

int init_sata(int dev)
{
	struct ahci_uc_priv *uc_priv = NULL;

#if defined(CONFIG_MX6)
	if (!is_mx6dq() && !is_mx6dqp())
		return 1;
#endif
	if (dev < 0 || dev > (CONFIG_SYS_SATA_MAX_DEVICE - 1)) {
		printf("The sata index %d is out of ranges\n\r", dev);
		return -1;
	}

	ahci_init_one(dev);

	uc_priv = sata_dev_desc[dev].priv;

	return dwc_ahci_start_ports(uc_priv) ? 1 : 0;
}

int reset_sata(int dev)
{
	struct ahci_uc_priv *uc_priv;
	struct sata_host_regs *host_mmio;

	if (dev < 0 || dev > (CONFIG_SYS_SATA_MAX_DEVICE - 1)) {
		printf("The sata index %d is out of ranges\n\r", dev);
		return -1;
	}

	uc_priv = sata_dev_desc[dev].priv;
	if (NULL == uc_priv)
		/* not initialized, so nothing to reset */
		return 0;

	host_mmio = uc_priv->mmio_base;
	setbits_le32(&host_mmio->ghc, SATA_HOST_GHC_HR);
	while (readl(&host_mmio->ghc) & SATA_HOST_GHC_HR)
		udelay(100);

	return 0;
}

int sata_port_status(int dev, int port)
{
	struct sata_port_regs *port_mmio;
	struct ahci_uc_priv *uc_priv = NULL;

	if (dev < 0 || dev > (CONFIG_SYS_SATA_MAX_DEVICE - 1))
		return -EINVAL;

	if (sata_dev_desc[dev].priv == NULL)
		return -ENODEV;

	uc_priv = sata_dev_desc[dev].priv;
	port_mmio = uc_priv->port[port].port_mmio;

	return readl(&port_mmio->ssts) & SATA_PORT_SSTS_DET_MASK;
}

/*
 * SATA interface between low level driver and command layer
 */
ulong sata_read(int dev, ulong blknr, lbaint_t blkcnt, void *buffer)
{
	struct ahci_uc_priv *uc_priv = sata_dev_desc[dev].priv;

	return sata_read_common(uc_priv, &sata_dev_desc[dev], blknr, blkcnt,
				buffer);
}

ulong sata_write(int dev, ulong blknr, lbaint_t blkcnt, const void *buffer)
{
	struct ahci_uc_priv *uc_priv = sata_dev_desc[dev].priv;

	return sata_write_common(uc_priv, &sata_dev_desc[dev], blknr, blkcnt,
				 buffer);
}

int scan_sata(int dev)
{
	struct ahci_uc_priv *uc_priv = sata_dev_desc[dev].priv;
	struct blk_desc *pdev = &sata_dev_desc[dev];

	return dwc_ahsata_scan_common(uc_priv, pdev);
}
#endif /* CONFIG_IS_ENABLED(AHCI) */

#if CONFIG_IS_ENABLED(AHCI)

int dwc_ahsata_port_status(struct udevice *dev, int port)
{
	struct ahci_uc_priv *uc_priv = dev_get_uclass_priv(dev);
	struct sata_port_regs *port_mmio;

	port_mmio = uc_priv->port[port].port_mmio;
	return readl(&port_mmio->ssts) & SATA_PORT_SSTS_DET_MASK ? 0 : -ENXIO;
}

int dwc_ahsata_bus_reset(struct udevice *dev)
{
	struct ahci_uc_priv *uc_priv = dev_get_uclass_priv(dev);
	struct sata_host_regs *host_mmio = uc_priv->mmio_base;

	setbits_le32(&host_mmio->ghc, SATA_HOST_GHC_HR);
	while (readl(&host_mmio->ghc) & SATA_HOST_GHC_HR)
		udelay(100);

	return 0;
}

int dwc_ahsata_scan(struct udevice *dev)
{
	struct ahci_uc_priv *uc_priv = dev_get_uclass_priv(dev);
	struct blk_desc *desc;
	struct udevice *blk;
	int ret;

	/*
	* Create only one block device and do detection
	* to make sure that there won't be a lot of
	* block devices created
	*/
	device_find_first_child(dev, &blk);
	if (!blk) {
		ret = blk_create_devicef(dev, "dwc_ahsata_blk", "blk",
					 IF_TYPE_SATA, -1, 512, 0, &blk);
		if (ret) {
			debug("Can't create device\n");
			return ret;
		}
	}

	desc = dev_get_uclass_platdata(blk);
	ret = dwc_ahsata_scan_common(uc_priv, desc);
	if (ret) {
		debug("%s: Failed to scan bus\n", __func__);
		return ret;
	}

	return 0;
}

int dwc_ahsata_probe(struct udevice *dev)
{
	struct ahci_uc_priv *uc_priv = dev_get_uclass_priv(dev);
	int ret;

#if defined(CONFIG_MX6)
	setup_sata();
#endif
	uc_priv->host_flags = ATA_FLAG_SATA | ATA_FLAG_NO_LEGACY |
			ATA_FLAG_MMIO | ATA_FLAG_PIO_DMA | ATA_FLAG_NO_ATAPI;
	uc_priv->mmio_base = (void __iomem *)dev_read_addr(dev);

	/* initialize adapter */
	ret = ahci_host_init(uc_priv);
	if (ret)
		return ret;

	ahci_print_info(uc_priv);

	return dwc_ahci_start_ports(uc_priv);
}

static ulong dwc_ahsata_read(struct udevice *blk, lbaint_t blknr,
			     lbaint_t blkcnt, void *buffer)
{
	struct blk_desc *desc = dev_get_uclass_platdata(blk);
	struct udevice *dev = dev_get_parent(blk);
	struct ahci_uc_priv *uc_priv;

	uc_priv = dev_get_uclass_priv(dev);
	return sata_read_common(uc_priv, desc, blknr, blkcnt, buffer);
}

static ulong dwc_ahsata_write(struct udevice *blk, lbaint_t blknr,
			      lbaint_t blkcnt, const void *buffer)
{
	struct blk_desc *desc = dev_get_uclass_platdata(blk);
	struct udevice *dev = dev_get_parent(blk);
	struct ahci_uc_priv *uc_priv;

	uc_priv = dev_get_uclass_priv(dev);
	return sata_write_common(uc_priv, desc, blknr, blkcnt, buffer);
}

static const struct blk_ops dwc_ahsata_blk_ops = {
	.read	= dwc_ahsata_read,
	.write	= dwc_ahsata_write,
};

U_BOOT_DRIVER(dwc_ahsata_blk) = {
	.name		= "dwc_ahsata_blk",
	.id		= UCLASS_BLK,
	.ops		= &dwc_ahsata_blk_ops,
};

#if CONFIG_IS_ENABLED(DWC_AHSATA_AHCI)
struct ahci_ops dwc_ahsata_ahci_ops = {
	.port_status = dwc_ahsata_port_status,
	.reset       = dwc_ahsata_bus_reset,
	.scan        = dwc_ahsata_scan,
};

static const struct udevice_id dwc_ahsata_ahci_ids[] = {
	{ .compatible = "fsl,imx6q-ahci" },
	{ }
};

U_BOOT_DRIVER(dwc_ahsata_ahci) = {
	.name     = "dwc_ahsata_ahci",
	.id       = UCLASS_AHCI,
	.of_match = dwc_ahsata_ahci_ids,
	.ops      = &dwc_ahsata_ahci_ops,
	.probe    = dwc_ahsata_probe,
};
#endif
#endif
