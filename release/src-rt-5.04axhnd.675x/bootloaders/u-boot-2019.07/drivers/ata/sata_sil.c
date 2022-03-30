// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2011 Freescale Semiconductor, Inc.
 * Author: Tang Yuantian <b29983@freescale.com>
 */

#include <common.h>
#include <pci.h>
#include <command.h>
#include <asm/byteorder.h>
#include <malloc.h>
#include <asm/io.h>
#include <fis.h>
#include <sata.h>
#include <libata.h>
#include <sata.h>
#include "sata_sil.h"

/* Convert sectorsize to wordsize */
#define ATA_SECTOR_WORDS (ATA_SECT_SIZE/2)
#define virt_to_bus(devno, v)	pci_virt_to_mem(devno, (void *) (v))

static struct sata_info sata_info;

static struct pci_device_id supported[] = {
	{PCI_VENDOR_ID_SILICONIMAGE, PCI_DEVICE_ID_SIL3131},
	{PCI_VENDOR_ID_SILICONIMAGE, PCI_DEVICE_ID_SIL3132},
	{PCI_VENDOR_ID_SILICONIMAGE, PCI_DEVICE_ID_SIL3124},
	{}
};

static void sil_sata_dump_fis(struct sata_fis_d2h *s)
{
	printf("Status FIS dump:\n");
	printf("fis_type:		%02x\n", s->fis_type);
	printf("pm_port_i:		%02x\n", s->pm_port_i);
	printf("status:			%02x\n", s->status);
	printf("error:			%02x\n", s->error);
	printf("lba_low:		%02x\n", s->lba_low);
	printf("lba_mid:		%02x\n", s->lba_mid);
	printf("lba_high:		%02x\n", s->lba_high);
	printf("device:			%02x\n", s->device);
	printf("lba_low_exp:		%02x\n", s->lba_low_exp);
	printf("lba_mid_exp:		%02x\n", s->lba_mid_exp);
	printf("lba_high_exp:		%02x\n", s->lba_high_exp);
	printf("res1:			%02x\n", s->res1);
	printf("sector_count:		%02x\n", s->sector_count);
	printf("sector_count_exp:	%02x\n", s->sector_count_exp);
}

static const char *sata_spd_string(unsigned int speed)
{
	static const char * const spd_str[] = {
		"1.5 Gbps",
		"3.0 Gbps",
		"6.0 Gbps",
	};

	if ((speed - 1) > 2)
		return "<unknown>";

	return spd_str[speed - 1];
}

static u32 ata_wait_register(void *reg, u32 mask,
			 u32 val, int timeout_msec)
{
	u32 tmp;

	tmp = readl(reg);
	while ((tmp & mask) == val && timeout_msec > 0) {
		mdelay(1);
		timeout_msec--;
		tmp = readl(reg);
	}

	return tmp;
}

static void sil_config_port(void *port)
{
	/* configure IRQ WoC */
	writel(PORT_CS_IRQ_WOC, port + PORT_CTRL_CLR);

	/* zero error counters. */
	writew(0x8000, port + PORT_DECODE_ERR_THRESH);
	writew(0x8000, port + PORT_CRC_ERR_THRESH);
	writew(0x8000, port + PORT_HSHK_ERR_THRESH);
	writew(0x0000, port + PORT_DECODE_ERR_CNT);
	writew(0x0000, port + PORT_CRC_ERR_CNT);
	writew(0x0000, port + PORT_HSHK_ERR_CNT);

	/* always use 64bit activation */
	writel(PORT_CS_32BIT_ACTV, port + PORT_CTRL_CLR);

	/* clear port multiplier enable and resume bits */
	writel(PORT_CS_PMP_EN | PORT_CS_PMP_RESUME, port + PORT_CTRL_CLR);
}

static int sil_init_port(void *port)
{
	u32 tmp;

	writel(PORT_CS_INIT, port + PORT_CTRL_STAT);
	ata_wait_register(port + PORT_CTRL_STAT,
			  PORT_CS_INIT, PORT_CS_INIT, 100);
	tmp = ata_wait_register(port + PORT_CTRL_STAT,
				PORT_CS_RDY, 0, 100);

	if ((tmp & (PORT_CS_INIT | PORT_CS_RDY)) != PORT_CS_RDY)
		return 1;

	return 0;
}

static void sil_read_fis(int dev, int tag, struct sata_fis_d2h *fis)
{
	struct sil_sata *sata = sata_dev_desc[dev].priv;
	void *port = sata->port;
	struct sil_prb *prb;
	int i;
	u32 *src, *dst;

	prb = port + PORT_LRAM + tag * PORT_LRAM_SLOT_SZ;
	src = (u32 *)&prb->fis;
	dst = (u32 *)fis;
	for (i = 0; i < sizeof(struct sata_fis_h2d); i += 4)
		*dst++ = readl(src++);
}

static int sil_exec_cmd(int dev, struct sil_cmd_block *pcmd, int tag)
{
	struct sil_sata *sata = sata_dev_desc[dev].priv;
	void *port = sata->port;
	u64 paddr = virt_to_bus(sata->devno, pcmd);
	u32 irq_mask, irq_stat;
	int rc;

	writel(PORT_IRQ_COMPLETE | PORT_IRQ_ERROR, port + PORT_IRQ_ENABLE_CLR);

	/* better to add momery barrior here */
	writel((u32)paddr, port + PORT_CMD_ACTIVATE + tag * 8);
	writel((u64)paddr >> 32, port + PORT_CMD_ACTIVATE + tag * 8 + 4);

	irq_mask = (PORT_IRQ_COMPLETE | PORT_IRQ_ERROR) << PORT_IRQ_RAW_SHIFT;
	irq_stat = ata_wait_register(port + PORT_IRQ_STAT, irq_mask,
			0, 10000);

	/* clear IRQs */
	writel(irq_mask, port + PORT_IRQ_STAT);
	irq_stat >>= PORT_IRQ_RAW_SHIFT;

	if (irq_stat & PORT_IRQ_COMPLETE)
		rc = 0;
	else {
		/* force port into known state */
		sil_init_port(port);
		if (irq_stat & PORT_IRQ_ERROR)
			rc = 1; /* error */
		else
			rc = 2; /* busy */
	}

	return rc;
}

static int sil_cmd_set_feature(int dev)
{
	struct sil_sata *sata = sata_dev_desc[dev].priv;
	struct sil_cmd_block cmdb, *pcmd = &cmdb;
	struct sata_fis_d2h fis;
	u8 udma_cap;
	int ret;

	memset((void *)&cmdb, 0, sizeof(struct sil_cmd_block));
	pcmd->prb.fis.fis_type = SATA_FIS_TYPE_REGISTER_H2D;
	pcmd->prb.fis.pm_port_c = (1 << 7);
	pcmd->prb.fis.command = ATA_CMD_SET_FEATURES;
	pcmd->prb.fis.features = SETFEATURES_XFER;

	/* First check the device capablity */
	udma_cap = (u8)(sata->udma & 0xff);
	debug("udma_cap %02x\n", udma_cap);

	if (udma_cap == ATA_UDMA6)
		pcmd->prb.fis.sector_count = XFER_UDMA_6;
	if (udma_cap == ATA_UDMA5)
		pcmd->prb.fis.sector_count = XFER_UDMA_5;
	if (udma_cap == ATA_UDMA4)
		pcmd->prb.fis.sector_count = XFER_UDMA_4;
	if (udma_cap == ATA_UDMA3)
		pcmd->prb.fis.sector_count = XFER_UDMA_3;

	ret = sil_exec_cmd(dev, pcmd, 0);
	if (ret) {
		sil_read_fis(dev, 0, &fis);
		printf("Err: exe cmd(0x%x).\n",
				readl(sata->port + PORT_SERROR));
		sil_sata_dump_fis(&fis);
		return 1;
	}

	return 0;
}

static int sil_cmd_identify_device(int dev, u16 *id)
{
	struct sil_sata *sata = sata_dev_desc[dev].priv;
	struct sil_cmd_block cmdb, *pcmd = &cmdb;
	struct sata_fis_d2h fis;
	int ret;

	memset((void *)&cmdb, 0, sizeof(struct sil_cmd_block));
	pcmd->prb.ctrl = cpu_to_le16(PRB_CTRL_PROTOCOL);
	pcmd->prb.prot = cpu_to_le16(PRB_PROT_READ);
	pcmd->prb.fis.fis_type = SATA_FIS_TYPE_REGISTER_H2D;
	pcmd->prb.fis.pm_port_c = (1 << 7);
	pcmd->prb.fis.command = ATA_CMD_ID_ATA;
	pcmd->sge.addr = cpu_to_le64(virt_to_bus(sata->devno, id));
	pcmd->sge.cnt = cpu_to_le32(sizeof(id[0]) * ATA_ID_WORDS);
	pcmd->sge.flags = cpu_to_le32(SGE_TRM);

	ret = sil_exec_cmd(dev, pcmd, 0);
	if (ret) {
		sil_read_fis(dev, 0, &fis);
		printf("Err: id cmd(0x%x).\n", readl(sata->port + PORT_SERROR));
		sil_sata_dump_fis(&fis);
		return 1;
	}
	ata_swap_buf_le16(id, ATA_ID_WORDS);

	return 0;
}

static int sil_cmd_soft_reset(int dev)
{
	struct sil_cmd_block cmdb, *pcmd = &cmdb;
	struct sil_sata *sata = sata_dev_desc[dev].priv;
	struct sata_fis_d2h fis;
	void *port = sata->port;
	int ret;

	/* put the port into known state */
	if (sil_init_port(port)) {
		printf("SRST: port %d not ready\n", dev);
		return 1;
	}

	memset((void *)&cmdb, 0, sizeof(struct sil_cmd_block));

	pcmd->prb.ctrl = cpu_to_le16(PRB_CTRL_SRST);
	pcmd->prb.fis.fis_type = SATA_FIS_TYPE_REGISTER_H2D;
	pcmd->prb.fis.pm_port_c = 0xf;

	ret = sil_exec_cmd(dev, &cmdb, 0);
	if (ret) {
		sil_read_fis(dev, 0, &fis);
		printf("SRST cmd error.\n");
		sil_sata_dump_fis(&fis);
		return 1;
	}

	return 0;
}

static ulong sil_sata_rw_cmd(int dev, ulong start, ulong blkcnt,
		u8 *buffer, int is_write)
{
	struct sil_sata *sata = sata_dev_desc[dev].priv;
	struct sil_cmd_block cmdb, *pcmd = &cmdb;
	struct sata_fis_d2h fis;
	u64 block;
	int ret;

	block = (u64)start;
	memset(pcmd, 0, sizeof(struct sil_cmd_block));
	pcmd->prb.ctrl = cpu_to_le16(PRB_CTRL_PROTOCOL);
	pcmd->prb.fis.fis_type = SATA_FIS_TYPE_REGISTER_H2D;
	pcmd->prb.fis.pm_port_c = (1 << 7);
	if (is_write) {
		pcmd->prb.fis.command = ATA_CMD_WRITE;
		pcmd->prb.prot = cpu_to_le16(PRB_PROT_WRITE);
	} else {
		pcmd->prb.fis.command = ATA_CMD_READ;
		pcmd->prb.prot = cpu_to_le16(PRB_PROT_READ);
	}

	pcmd->prb.fis.device = ATA_LBA;
	pcmd->prb.fis.device |= (block >> 24) & 0xf;
	pcmd->prb.fis.lba_high = (block >> 16) & 0xff;
	pcmd->prb.fis.lba_mid = (block >> 8) & 0xff;
	pcmd->prb.fis.lba_low = block & 0xff;
	pcmd->prb.fis.sector_count = (u8)blkcnt & 0xff;

	pcmd->sge.addr = cpu_to_le64(virt_to_bus(sata->devno, buffer));
	pcmd->sge.cnt = cpu_to_le32(blkcnt * ATA_SECT_SIZE);
	pcmd->sge.flags = cpu_to_le32(SGE_TRM);

	ret = sil_exec_cmd(dev, pcmd, 0);
	if (ret) {
		sil_read_fis(dev, 0, &fis);
		printf("Err: rw cmd(0x%08x).\n",
				readl(sata->port + PORT_SERROR));
		sil_sata_dump_fis(&fis);
		return 1;
	}

	return blkcnt;
}

static ulong sil_sata_rw_cmd_ext(int dev, ulong start, ulong blkcnt,
		u8 *buffer, int is_write)
{
	struct sil_sata *sata = sata_dev_desc[dev].priv;
	struct sil_cmd_block cmdb, *pcmd = &cmdb;
	struct sata_fis_d2h fis;
	u64 block;
	int ret;

	block = (u64)start;
	memset(pcmd, 0, sizeof(struct sil_cmd_block));
	pcmd->prb.ctrl = cpu_to_le16(PRB_CTRL_PROTOCOL);
	pcmd->prb.fis.fis_type = SATA_FIS_TYPE_REGISTER_H2D;
	pcmd->prb.fis.pm_port_c = (1 << 7);
	if (is_write) {
		pcmd->prb.fis.command = ATA_CMD_WRITE_EXT;
		pcmd->prb.prot = cpu_to_le16(PRB_PROT_WRITE);
	} else {
		pcmd->prb.fis.command = ATA_CMD_READ_EXT;
		pcmd->prb.prot = cpu_to_le16(PRB_PROT_READ);
	}

	pcmd->prb.fis.lba_high_exp = (block >> 40) & 0xff;
	pcmd->prb.fis.lba_mid_exp = (block >> 32) & 0xff;
	pcmd->prb.fis.lba_low_exp = (block >> 24) & 0xff;
	pcmd->prb.fis.lba_high = (block >> 16) & 0xff;
	pcmd->prb.fis.lba_mid = (block >> 8) & 0xff;
	pcmd->prb.fis.lba_low = block & 0xff;
	pcmd->prb.fis.device = ATA_LBA;
	pcmd->prb.fis.sector_count_exp = (blkcnt >> 8) & 0xff;
	pcmd->prb.fis.sector_count = blkcnt & 0xff;

	pcmd->sge.addr = cpu_to_le64(virt_to_bus(sata->devno, buffer));
	pcmd->sge.cnt = cpu_to_le32(blkcnt * ATA_SECT_SIZE);
	pcmd->sge.flags = cpu_to_le32(SGE_TRM);

	ret = sil_exec_cmd(dev, pcmd, 0);
	if (ret) {
		sil_read_fis(dev, 0, &fis);
		printf("Err: rw ext cmd(0x%08x).\n",
				readl(sata->port + PORT_SERROR));
		sil_sata_dump_fis(&fis);
		return 1;
	}

	return blkcnt;
}

static ulong sil_sata_rw_lba28(int dev, ulong blknr, lbaint_t blkcnt,
			       const void *buffer, int is_write)
{
	ulong start, blks, max_blks;
	u8 *addr;

	start = blknr;
	blks = blkcnt;
	addr = (u8 *)buffer;

	max_blks = ATA_MAX_SECTORS;
	do {
		if (blks > max_blks) {
			sil_sata_rw_cmd(dev, start, max_blks, addr, is_write);
			start += max_blks;
			blks -= max_blks;
			addr += ATA_SECT_SIZE * max_blks;
		} else {
			sil_sata_rw_cmd(dev, start, blks, addr, is_write);
			start += blks;
			blks = 0;
			addr += ATA_SECT_SIZE * blks;
		}
	} while (blks != 0);

	return blkcnt;
}

static ulong sil_sata_rw_lba48(int dev, ulong blknr, lbaint_t blkcnt,
			       const void *buffer, int is_write)
{
	ulong start, blks, max_blks;
	u8 *addr;

	start = blknr;
	blks = blkcnt;
	addr = (u8 *)buffer;

	max_blks = ATA_MAX_SECTORS_LBA48;
	do {
		if (blks > max_blks) {
			sil_sata_rw_cmd_ext(dev, start, max_blks,
					addr, is_write);
			start += max_blks;
			blks -= max_blks;
			addr += ATA_SECT_SIZE * max_blks;
		} else {
			sil_sata_rw_cmd_ext(dev, start, blks,
					addr, is_write);
			start += blks;
			blks = 0;
			addr += ATA_SECT_SIZE * blks;
		}
	} while (blks != 0);

	return blkcnt;
}

static void sil_sata_cmd_flush_cache(int dev)
{
	struct sil_cmd_block cmdb, *pcmd = &cmdb;

	memset((void *)pcmd, 0, sizeof(struct sil_cmd_block));
	pcmd->prb.fis.fis_type = SATA_FIS_TYPE_REGISTER_H2D;
	pcmd->prb.fis.pm_port_c = (1 << 7);
	pcmd->prb.fis.command = ATA_CMD_FLUSH;

	sil_exec_cmd(dev, pcmd, 0);
}

static void sil_sata_cmd_flush_cache_ext(int dev)
{
	struct sil_cmd_block cmdb, *pcmd = &cmdb;

	memset((void *)pcmd, 0, sizeof(struct sil_cmd_block));
	pcmd->prb.fis.fis_type = SATA_FIS_TYPE_REGISTER_H2D;
	pcmd->prb.fis.pm_port_c = (1 << 7);
	pcmd->prb.fis.command = ATA_CMD_FLUSH_EXT;

	sil_exec_cmd(dev, pcmd, 0);
}

static void sil_sata_init_wcache(int dev, u16 *id)
{
	struct sil_sata *sata = sata_dev_desc[dev].priv;

	if (ata_id_has_wcache(id) && ata_id_wcache_enabled(id))
		sata->wcache = 1;
	if (ata_id_has_flush(id))
		sata->flush = 1;
	if (ata_id_has_flush_ext(id))
		sata->flush_ext = 1;
}

static int sil_sata_get_wcache(int dev)
{
	struct sil_sata *sata = sata_dev_desc[dev].priv;

	return sata->wcache;
}

static int sil_sata_get_flush(int dev)
{
	struct sil_sata *sata = sata_dev_desc[dev].priv;

	return sata->flush;
}

static int sil_sata_get_flush_ext(int dev)
{
	struct sil_sata *sata = sata_dev_desc[dev].priv;

	return sata->flush_ext;
}

/*
 * SATA interface between low level driver and command layer
 */
ulong sata_read(int dev, ulong blknr, lbaint_t blkcnt, void *buffer)
{
	struct sil_sata *sata = sata_dev_desc[dev].priv;
	ulong rc;

	if (sata->lba48)
		rc = sil_sata_rw_lba48(dev, blknr, blkcnt, buffer, READ_CMD);
	else
		rc = sil_sata_rw_lba28(dev, blknr, blkcnt, buffer, READ_CMD);

	return rc;
}

/*
 * SATA interface between low level driver and command layer
 */
ulong sata_write(int dev, ulong blknr, lbaint_t blkcnt, const void *buffer)
{
	struct sil_sata *sata = sata_dev_desc[dev].priv;
	ulong rc;

	if (sata->lba48) {
		rc = sil_sata_rw_lba48(dev, blknr, blkcnt, buffer, WRITE_CMD);
		if (sil_sata_get_wcache(dev) && sil_sata_get_flush_ext(dev))
			sil_sata_cmd_flush_cache_ext(dev);
	} else {
		rc = sil_sata_rw_lba28(dev, blknr, blkcnt, buffer, WRITE_CMD);
		if (sil_sata_get_wcache(dev) && sil_sata_get_flush(dev))
			sil_sata_cmd_flush_cache(dev);
	}

	return rc;
}

/*
 * SATA interface between low level driver and command layer
 */
int init_sata(int dev)
{
	static int init_done, idx;
	pci_dev_t devno;
	u16 word;

	if (init_done == 1 && dev < sata_info.maxport)
		return 0;

	init_done = 1;

	/* Find PCI device(s) */
	devno = pci_find_devices(supported, idx++);
	if (devno == -1)
		return 1;

	pci_read_config_word(devno, PCI_DEVICE_ID, &word);

	/* get the port count */
	word &= 0xf;

	sata_info.portbase = sata_info.maxport;
	sata_info.maxport = sata_info.portbase + word;
	sata_info.devno = devno;

	/* Read out all BARs */
	sata_info.iobase[0] = (ulong)pci_map_bar(devno,
			PCI_BASE_ADDRESS_0, PCI_REGION_MEM);
	sata_info.iobase[1] = (ulong)pci_map_bar(devno,
			PCI_BASE_ADDRESS_2, PCI_REGION_MEM);
	sata_info.iobase[2] = (ulong)pci_map_bar(devno,
			PCI_BASE_ADDRESS_4, PCI_REGION_MEM);

	/* mask out the unused bits */
	sata_info.iobase[0] &= 0xffffff80;
	sata_info.iobase[1] &= 0xfffffc00;
	sata_info.iobase[2] &= 0xffffff80;

	/* Enable Bus Mastering and memory region */
	pci_write_config_word(devno, PCI_COMMAND,
			PCI_COMMAND_MEMORY | PCI_COMMAND_MASTER);

	/* Check if mem accesses and Bus Mastering are enabled. */
	pci_read_config_word(devno, PCI_COMMAND, &word);
	if (!(word & PCI_COMMAND_MEMORY) ||
			(!(word & PCI_COMMAND_MASTER))) {
		printf("Error: Can not enable MEM access or Bus Mastering.\n");
		debug("PCI command: %04x\n", word);
		return 1;
	}

	/* GPIO off */
	writel(0, (void *)(sata_info.iobase[0] + HOST_FLASH_CMD));
	/* clear global reset & mask interrupts during initialization */
	writel(0, (void *)(sata_info.iobase[0] + HOST_CTRL));

	return 0;
}

int reset_sata(int dev)
{
	return 0;
}

/*
 * SATA interface between low level driver and command layer
 */
int scan_sata(int dev)
{
	unsigned char serial[ATA_ID_SERNO_LEN + 1];
	unsigned char firmware[ATA_ID_FW_REV_LEN + 1];
	unsigned char product[ATA_ID_PROD_LEN + 1];
	struct sil_sata *sata;
	void *port;
	int cnt;
	u16 *id;
	u32 tmp;

	if (dev >= sata_info.maxport) {
		printf("SATA#%d is not present\n", dev);
		return 1;
	}

	printf("SATA#%d\n", dev);
	port = (void *)sata_info.iobase[1] +
		PORT_REGS_SIZE * (dev - sata_info.portbase);

	/* Initial PHY setting */
	writel(0x20c, port + PORT_PHY_CFG);

	/* clear port RST */
	tmp = readl(port + PORT_CTRL_STAT);
	if (tmp & PORT_CS_PORT_RST) {
		writel(PORT_CS_PORT_RST, port + PORT_CTRL_CLR);
		tmp = ata_wait_register(port + PORT_CTRL_STAT,
				PORT_CS_PORT_RST, PORT_CS_PORT_RST, 100);
		if (tmp & PORT_CS_PORT_RST)
			printf("Err: Failed to clear port RST\n");
	}

	/* Check if device is present */
	for (cnt = 0; cnt < 100; cnt++) {
		tmp = readl(port + PORT_SSTATUS);
		if ((tmp & 0xF) == 0x3)
			break;
		mdelay(1);
	}

	tmp = readl(port + PORT_SSTATUS);
	if ((tmp & 0xf) != 0x3) {
		printf("	(No RDY)\n");
		return 1;
	}

	/* Wait for port ready */
	tmp = ata_wait_register(port + PORT_CTRL_STAT,
				PORT_CS_RDY, PORT_CS_RDY, 100);
	if ((tmp & PORT_CS_RDY) != PORT_CS_RDY) {
		printf("%d port not ready.\n", dev);
		return 1;
	}

	/* configure port */
	sil_config_port(port);

	/* Reset port */
	writel(PORT_CS_DEV_RST, port + PORT_CTRL_STAT);
	readl(port + PORT_CTRL_STAT);
	tmp = ata_wait_register(port + PORT_CTRL_STAT, PORT_CS_DEV_RST,
				PORT_CS_DEV_RST, 100);
	if (tmp & PORT_CS_DEV_RST) {
		printf("%d port reset failed.\n", dev);
		return 1;
	}

	sata = (struct sil_sata *)malloc(sizeof(struct sil_sata));
	if (!sata) {
		printf("%d no memory.\n", dev);
		return 1;
	}
	memset((void *)sata, 0, sizeof(struct sil_sata));

	/* turn on port interrupt */
	tmp = readl((void *)(sata_info.iobase[0] + HOST_CTRL));
	tmp |= (1 << (dev - sata_info.portbase));
	writel(tmp, (void *)(sata_info.iobase[0] + HOST_CTRL));

	/* Save the private struct to block device struct */
	sata_dev_desc[dev].priv = (void *)sata;
	sata->port = port;
	sata->devno = sata_info.devno;
	sprintf(sata->name, "SATA#%d", dev);
	sil_cmd_soft_reset(dev);
	tmp = readl(port + PORT_SSTATUS);
	tmp = (tmp >> 4) & 0xf;
	printf("	(%s)\n", sata_spd_string(tmp));

	id = (u16 *)malloc(ATA_ID_WORDS * 2);
	if (!id) {
		printf("Id malloc failed\n");
		free((void *)sata);
		return 1;
	}
	sil_cmd_identify_device(dev, id);

#ifdef CONFIG_LBA48
	/* Check if support LBA48 */
	if (ata_id_has_lba48(id)) {
		sata_dev_desc[dev].lba48 = 1;
		sata->lba48 = 1;
		debug("Device supports LBA48\n");
	} else
		debug("Device supports LBA28\n");
#endif

	/* Serial number */
	ata_id_c_string(id, serial, ATA_ID_SERNO, sizeof(serial));
	memcpy(sata_dev_desc[dev].product, serial, sizeof(serial));

	/* Firmware version */
	ata_id_c_string(id, firmware, ATA_ID_FW_REV, sizeof(firmware));
	memcpy(sata_dev_desc[dev].revision, firmware, sizeof(firmware));

	/* Product model */
	ata_id_c_string(id, product, ATA_ID_PROD, sizeof(product));
	memcpy(sata_dev_desc[dev].vendor, product, sizeof(product));

	/* Totoal sectors */
	sata_dev_desc[dev].lba = ata_id_n_sectors(id);

	sil_sata_init_wcache(dev, id);
	sil_cmd_set_feature(dev);

#ifdef DEBUG
	sil_cmd_identify_device(dev, id);
	ata_dump_id(id);
#endif
	free((void *)id);

	return 0;
}
