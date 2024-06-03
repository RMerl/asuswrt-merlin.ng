// SPDX-License-Identifier: GPL-2.0+
/*
 * NXP PPC SATA platform driver
 *
 * (C) Copyright 2019 NXP, Inc.
 *
 */
#include <common.h>
#include <asm/fsl_serdes.h>
#include <dm/lists.h>
#include <dm.h>
#include <ahci.h>
#include <scsi.h>
#include <libata.h>
#include <sata.h>
#include <malloc.h>
#include <memalign.h>
#include <fis.h>

#include "fsl_sata.h"

struct fsl_ahci_priv {
	u32 base;
	u32 flag;
	u32 number;
	fsl_sata_t *fsl_sata;
};

static int fsl_ahci_bind(struct udevice *dev)
{
	return device_bind_driver(dev, "fsl_ahci_scsi", "fsl_ahci_scsi", NULL);
}

static int fsl_ahci_ofdata_to_platdata(struct udevice *dev)
{
	struct fsl_ahci_priv *priv = dev_get_priv(dev);

	priv->number = dev_read_u32_default(dev, "sata-number", -1);
	priv->flag = dev_read_u32_default(dev, "sata-fpdma", -1);

	priv->base = dev_read_addr(dev);
	if (priv->base == FDT_ADDR_T_NONE)
		return -EINVAL;

	return 0;
}

static int ata_wait_register(unsigned __iomem *addr, u32 mask,
			     u32 val, u32 timeout_msec)
{
	int i;

	for (i = 0; ((in_le32(addr) & mask) != val) && i < timeout_msec; i++)
		mdelay(1);

	return (i < timeout_msec) ? 0 : -1;
}

static void fsl_sata_dump_sfis(struct sata_fis_d2h *s)
{
	printf("Status FIS dump:\n\r");
	printf("fis_type:		%02x\n\r", s->fis_type);
	printf("pm_port_i:		%02x\n\r", s->pm_port_i);
	printf("status:			%02x\n\r", s->status);
	printf("error:			%02x\n\r", s->error);
	printf("lba_low:		%02x\n\r", s->lba_low);
	printf("lba_mid:		%02x\n\r", s->lba_mid);
	printf("lba_high:		%02x\n\r", s->lba_high);
	printf("device:			%02x\n\r", s->device);
	printf("lba_low_exp:		%02x\n\r", s->lba_low_exp);
	printf("lba_mid_exp:		%02x\n\r", s->lba_mid_exp);
	printf("lba_high_exp:		%02x\n\r", s->lba_high_exp);
	printf("res1:			%02x\n\r", s->res1);
	printf("sector_count:		%02x\n\r", s->sector_count);
	printf("sector_count_exp:	%02x\n\r", s->sector_count_exp);
}

static void fsl_sata_dump_regs(fsl_sata_reg_t __iomem *reg)
{
	printf("\n\rSATA:           %08x\n\r", (u32)reg);
	printf("CQR:            %08x\n\r", in_le32(&reg->cqr));
	printf("CAR:            %08x\n\r", in_le32(&reg->car));
	printf("CCR:            %08x\n\r", in_le32(&reg->ccr));
	printf("CER:            %08x\n\r", in_le32(&reg->cer));
	printf("CQR:            %08x\n\r", in_le32(&reg->cqr));
	printf("DER:            %08x\n\r", in_le32(&reg->der));
	printf("CHBA:           %08x\n\r", in_le32(&reg->chba));
	printf("HStatus:        %08x\n\r", in_le32(&reg->hstatus));
	printf("HControl:       %08x\n\r", in_le32(&reg->hcontrol));
	printf("CQPMP:          %08x\n\r", in_le32(&reg->cqpmp));
	printf("SIG:            %08x\n\r", in_le32(&reg->sig));
	printf("ICC:            %08x\n\r", in_le32(&reg->icc));
	printf("SStatus:        %08x\n\r", in_le32(&reg->sstatus));
	printf("SError:         %08x\n\r", in_le32(&reg->serror));
	printf("SControl:       %08x\n\r", in_le32(&reg->scontrol));
	printf("SNotification:  %08x\n\r", in_le32(&reg->snotification));
	printf("TransCfg:       %08x\n\r", in_le32(&reg->transcfg));
	printf("TransStatus:    %08x\n\r", in_le32(&reg->transstatus));
	printf("LinkCfg:        %08x\n\r", in_le32(&reg->linkcfg));
	printf("LinkCfg1:       %08x\n\r", in_le32(&reg->linkcfg1));
	printf("LinkCfg2:       %08x\n\r", in_le32(&reg->linkcfg2));
	printf("LinkStatus:     %08x\n\r", in_le32(&reg->linkstatus));
	printf("LinkStatus1:    %08x\n\r", in_le32(&reg->linkstatus1));
	printf("PhyCtrlCfg:     %08x\n\r", in_le32(&reg->phyctrlcfg));
	printf("SYSPR:          %08x\n\r", in_be32(&reg->syspr));
}

static int init_sata(struct fsl_ahci_priv *priv)
{
	int i;
	u32 cda;
	u32 val32;
	u32 sig;
	fsl_sata_t *sata;
	u32 length, align;
	cmd_hdr_tbl_t *cmd_hdr;
	fsl_sata_reg_t __iomem *reg;

	int dev = priv->number;

	if (dev < 0 || dev > (CONFIG_SYS_SATA_MAX_DEVICE - 1)) {
		printf("the sata index %d is out of ranges\n\r", dev);
		return -EINVAL;
	}

#ifdef CONFIG_MPC85xx
	if (dev == 0 && (!is_serdes_configured(SATA1))) {
		printf("SATA%d [dev = %d] is not enabled\n", dev + 1, dev);
		return -EINVAL;
	}
	if (dev == 1 && (!is_serdes_configured(SATA2))) {
		printf("SATA%d [dev = %d] is not enabled\n", dev + 1, dev);
		return -EINVAL;
	}
#endif

	/* Allocate SATA device driver struct */
	sata = (fsl_sata_t *)malloc(sizeof(fsl_sata_t));
	if (!sata) {
		printf("alloc the sata device struct failed\n\r");
		return -ENOMEM;
	}
	/* Zero all of the device driver struct */
	memset((void *)sata, 0, sizeof(fsl_sata_t));

	sata->dma_flag = priv->flag;
	snprintf(sata->name, 12, "SATA%d", dev);

	/* Set the controller register base address to device struct */
	reg = (fsl_sata_reg_t *)priv->base;
	sata->reg_base = reg;

	/* Allocate the command header table, 4 bytes aligned */
	length = sizeof(struct cmd_hdr_tbl);
	align = SATA_HC_CMD_HDR_TBL_ALIGN;
	sata->cmd_hdr_tbl_offset = (void *)malloc(length + align);
	if (!sata->cmd_hdr_tbl_offset) {
		printf("alloc the command header failed\n\r");
		return -ENOMEM;
	}

	cmd_hdr = (cmd_hdr_tbl_t *)(((u32)sata->cmd_hdr_tbl_offset + align)
						& ~(align - 1));
	sata->cmd_hdr = cmd_hdr;

	/* Zero all of the command header table */
	memset((void *)sata->cmd_hdr_tbl_offset, 0, length + align);

	/* Allocate command descriptor for all command */
	length = sizeof(struct cmd_desc) * SATA_HC_MAX_CMD;
	align = SATA_HC_CMD_DESC_ALIGN;
	sata->cmd_desc_offset = (void *)malloc(length + align);
	if (!sata->cmd_desc_offset) {
		printf("alloc the command descriptor failed\n\r");
		return -ENOMEM;
	}
	sata->cmd_desc = (cmd_desc_t *)(((u32)sata->cmd_desc_offset + align)
						& ~(align - 1));
	/* Zero all of command descriptor */
	memset((void *)sata->cmd_desc_offset, 0, length + align);

	/* Link the command descriptor to command header */
	for (i = 0; i < SATA_HC_MAX_CMD; i++) {
		cda = ((u32)sata->cmd_desc + SATA_HC_CMD_DESC_SIZE * i)
					 & ~(CMD_HDR_CDA_ALIGN - 1);
		cmd_hdr->cmd_slot[i].cda = cpu_to_le32(cda);
	}

	/* To have safe state, force the controller offline */
	val32 = in_le32(&reg->hcontrol);
	val32 &= ~HCONTROL_ONOFF;
	val32 |= HCONTROL_FORCE_OFFLINE;
	out_le32(&reg->hcontrol, val32);

	/* Wait the controller offline */
	ata_wait_register(&reg->hstatus, HSTATUS_ONOFF, 0, 1000);

	/* Set the command header base address to CHBA register to tell DMA */
	out_le32(&reg->chba, (u32)cmd_hdr & ~0x3);

	/* Snoop for the command header */
	val32 = in_le32(&reg->hcontrol);
	val32 |= HCONTROL_HDR_SNOOP;
	out_le32(&reg->hcontrol, val32);

	/* Disable all of interrupts */
	val32 = in_le32(&reg->hcontrol);
	val32 &= ~HCONTROL_INT_EN_ALL;
	out_le32(&reg->hcontrol, val32);

	/* Clear all of interrupts */
	val32 = in_le32(&reg->hstatus);
	out_le32(&reg->hstatus, val32);

	/* Set the ICC, no interrupt coalescing */
	out_le32(&reg->icc, 0x01000000);

	/* No PM attatched, the SATA device direct connect */
	out_le32(&reg->cqpmp, 0);

	/* Clear SError register */
	val32 = in_le32(&reg->serror);
	out_le32(&reg->serror, val32);

	/* Clear CER register */
	val32 = in_le32(&reg->cer);
	out_le32(&reg->cer, val32);

	/* Clear DER register */
	val32 = in_le32(&reg->der);
	out_le32(&reg->der, val32);

	/* No device detection or initialization action requested */
	out_le32(&reg->scontrol, 0x00000300);

	/* Configure the transport layer, default value */
	out_le32(&reg->transcfg, 0x08000016);

	/* Configure the link layer, default value */
	out_le32(&reg->linkcfg, 0x0000ff34);

	/* Bring the controller online */
	val32 = in_le32(&reg->hcontrol);
	val32 |= HCONTROL_ONOFF;
	out_le32(&reg->hcontrol, val32);

	mdelay(100);

	/* print sata device name */
	printf("%s ", sata->name);

	/* Wait PHY RDY signal changed for 500ms */
	ata_wait_register(&reg->hstatus, HSTATUS_PHY_RDY,
			  HSTATUS_PHY_RDY, 500);

	/* Check PHYRDY */
	val32 = in_le32(&reg->hstatus);
	if (val32 & HSTATUS_PHY_RDY) {
		sata->link = 1;
	} else {
		sata->link = 0;
		printf("(No RDY)\n\r");
		return -EINVAL;
	}

	/* Wait for signature updated, which is 1st D2H */
	ata_wait_register(&reg->hstatus, HSTATUS_SIGNATURE,
			  HSTATUS_SIGNATURE, 10000);

	if (val32 & HSTATUS_SIGNATURE) {
		sig = in_le32(&reg->sig);
		debug("Signature updated, the sig =%08x\n\r", sig);
		sata->ata_device_type = ata_dev_classify(sig);
	}

	/* Check the speed */
	val32 = in_le32(&reg->sstatus);
	if ((val32 & SSTATUS_SPD_MASK) == SSTATUS_SPD_GEN1)
		printf("(1.5 Gbps)\n\r");
	else if ((val32 & SSTATUS_SPD_MASK) == SSTATUS_SPD_GEN2)
		printf("(3 Gbps)\n\r");

	priv->fsl_sata = sata;

	return 0;
}

static int fsl_ata_exec_ata_cmd(struct fsl_sata *sata,
				struct sata_fis_h2d *cfis,
				int is_ncq, int tag,
				u8 *buffer, u32 len)
{
	cmd_hdr_entry_t *cmd_hdr;
	cmd_desc_t *cmd_desc;
	sata_fis_h2d_t *h2d;
	prd_entry_t *prde;
	u32 ext_c_ddc;
	u32 prde_count;
	u32 val32;
	u32 ttl;
	u32 der;
	int i;

	fsl_sata_reg_t *reg = sata->reg_base;

	/* Check xfer length */
	if (len > SATA_HC_MAX_XFER_LEN) {
		printf("max transfer length is 64MB\n\r");
		return 0;
	}

	/* Setup the command descriptor */
	cmd_desc = sata->cmd_desc + tag;

	/* Get the pointer cfis of command descriptor */
	h2d = (sata_fis_h2d_t *)cmd_desc->cfis;

	/* Zero the cfis of command descriptor */
	memset((void *)h2d, 0, SATA_HC_CMD_DESC_CFIS_SIZE);

	/* Copy the cfis from user to command descriptor */
	h2d->fis_type = cfis->fis_type;
	h2d->pm_port_c = cfis->pm_port_c;
	h2d->command = cfis->command;

	h2d->features = cfis->features;
	h2d->features_exp = cfis->features_exp;

	h2d->lba_low = cfis->lba_low;
	h2d->lba_mid = cfis->lba_mid;
	h2d->lba_high = cfis->lba_high;
	h2d->lba_low_exp = cfis->lba_low_exp;
	h2d->lba_mid_exp = cfis->lba_mid_exp;
	h2d->lba_high_exp = cfis->lba_high_exp;

	if (!is_ncq) {
		h2d->sector_count = cfis->sector_count;
		h2d->sector_count_exp = cfis->sector_count_exp;
	} else { /* NCQ */
		h2d->sector_count = (u8)(tag << 3);
	}

	h2d->device = cfis->device;
	h2d->control = cfis->control;

	/* Setup the PRD table */
	prde = (prd_entry_t *)cmd_desc->prdt;
	memset((void *)prde, 0, sizeof(struct prdt));

	prde_count = 0;
	ttl = len;
	for (i = 0; i < SATA_HC_MAX_PRD_DIRECT; i++) {
		if (!len)
			break;
		prde->dba = cpu_to_le32((u32)buffer & ~0x3);
		debug("dba = %08x\n\r", (u32)buffer);

		if (len < PRD_ENTRY_MAX_XFER_SZ) {
			ext_c_ddc = PRD_ENTRY_DATA_SNOOP | len;
			debug("ext_c_ddc1 = %08x, len = %08x\n\r",
			      ext_c_ddc, len);
			prde->ext_c_ddc = cpu_to_le32(ext_c_ddc);
			prde_count++;
			prde++;
		} else {
			ext_c_ddc = PRD_ENTRY_DATA_SNOOP; /* 4M bytes */
			debug("ext_c_ddc2 = %08x, len = %08x\n\r",
			      ext_c_ddc, len);
			prde->ext_c_ddc = cpu_to_le32(ext_c_ddc);
			buffer += PRD_ENTRY_MAX_XFER_SZ;
			len -= PRD_ENTRY_MAX_XFER_SZ;
			prde_count++;
			prde++;
		}
	}

	/* Setup the command slot of cmd hdr */
	cmd_hdr = (cmd_hdr_entry_t *)&sata->cmd_hdr->cmd_slot[tag];

	cmd_hdr->cda = cpu_to_le32((u32)cmd_desc & ~0x3);

	val32 = prde_count << CMD_HDR_PRD_ENTRY_SHIFT;
	val32 |= sizeof(sata_fis_h2d_t);
	cmd_hdr->prde_fis_len = cpu_to_le32(val32);

	cmd_hdr->ttl = cpu_to_le32(ttl);

	if (!is_ncq)
		val32 = CMD_HDR_ATTR_RES | CMD_HDR_ATTR_SNOOP;
	else
		val32 = CMD_HDR_ATTR_RES | CMD_HDR_ATTR_SNOOP |
			CMD_HDR_ATTR_FPDMA;

	tag &= CMD_HDR_ATTR_TAG;
	val32 |= tag;

	debug("attribute = %08x\n\r", val32);
	cmd_hdr->attribute = cpu_to_le32(val32);

	/* Make sure cmd desc and cmd slot valid before command issue */
	sync();

	/* PMP*/
	val32 = (u32)(h2d->pm_port_c & 0x0f);
	out_le32(&reg->cqpmp, val32);

	/* Wait no active */
	if (ata_wait_register(&reg->car, (1 << tag), 0, 10000))
		printf("Wait no active time out\n\r");

	/* Issue command */
	if (!(in_le32(&reg->cqr) & (1 << tag))) {
		val32 = 1 << tag;
		out_le32(&reg->cqr, val32);
	}

	/* Wait command completed for 10s */
	if (ata_wait_register(&reg->ccr, (1 << tag), (1 << tag), 10000)) {
		if (!is_ncq)
			printf("Non-NCQ command time out\n\r");
		else
			printf("NCQ command time out\n\r");
	}

	val32 = in_le32(&reg->cer);

	if (val32) {
		fsl_sata_dump_sfis((struct sata_fis_d2h *)cmd_desc->sfis);
		printf("CE at device\n\r");
		fsl_sata_dump_regs(reg);
		der = in_le32(&reg->der);
		out_le32(&reg->cer, val32);
		out_le32(&reg->der, der);
	}

	/* Clear complete flags */
	val32 = in_le32(&reg->ccr);
	out_le32(&reg->ccr, val32);

	return len;
}

static int fsl_sata_exec_cmd(struct fsl_sata *sata, struct sata_fis_h2d *cfis,
			     enum cmd_type command_type, int tag, u8 *buffer,
			     u32 len)
{
	int rc;

	if (tag > SATA_HC_MAX_CMD || tag < 0) {
		printf("tag is out of range, tag=%d\n\r", tag);
		return -1;
	}

	switch (command_type) {
	case CMD_ATA:
		rc = fsl_ata_exec_ata_cmd(sata, cfis, 0, tag, buffer, len);
		return rc;
	case CMD_NCQ:
		rc = fsl_ata_exec_ata_cmd(sata, cfis, 1, tag, buffer, len);
		return rc;
	case CMD_ATAPI:
	case CMD_VENDOR_BIST:
	case CMD_BIST:
		printf("not support now\n\r");
		return -1;
	default:
		break;
	}

	return -1;
}

static void fsl_sata_identify(fsl_sata_t *sata, u16 *id)
{
	struct sata_fis_h2d h2d, *cfis = &h2d;

	memset(cfis, 0, sizeof(struct sata_fis_h2d));

	cfis->fis_type = SATA_FIS_TYPE_REGISTER_H2D;
	cfis->pm_port_c = 0x80; /* is command */
	cfis->command = ATA_CMD_ID_ATA;

	fsl_sata_exec_cmd(sata, cfis, CMD_ATA, 0, (u8 *)id, ATA_ID_WORDS * 2);
	ata_swap_buf_le16(id, ATA_ID_WORDS);
}

static void fsl_sata_xfer_mode(fsl_sata_t *sata, u16 *id)
{
	sata->pio = id[ATA_ID_PIO_MODES];
	sata->mwdma = id[ATA_ID_MWDMA_MODES];
	sata->udma = id[ATA_ID_UDMA_MODES];
	debug("pio %04x, mwdma %04x, udma %04x\n\r", sata->pio,
	      sata->mwdma, sata->udma);
}

static void fsl_sata_init_wcache(fsl_sata_t *sata, u16 *id)
{
	if (ata_id_has_wcache(id) && ata_id_wcache_enabled(id))
		sata->wcache = 1;
	if (ata_id_has_flush(id))
		sata->flush = 1;
	if (ata_id_has_flush_ext(id))
		sata->flush_ext = 1;
}

static void fsl_sata_set_features(fsl_sata_t *sata)
{
	struct sata_fis_h2d h2d, *cfis = &h2d;
	u8 udma_cap;

	memset(cfis, 0, sizeof(struct sata_fis_h2d));

	cfis->fis_type = SATA_FIS_TYPE_REGISTER_H2D;
	cfis->pm_port_c = 0x80; /* is command */
	cfis->command = ATA_CMD_SET_FEATURES;
	cfis->features = SETFEATURES_XFER;

	/* First check the device capablity */
	udma_cap = (u8)(sata->udma & 0xff);
	debug("udma_cap %02x\n\r", udma_cap);

	if (udma_cap == ATA_UDMA6)
		cfis->sector_count = XFER_UDMA_6;
	if (udma_cap == ATA_UDMA5)
		cfis->sector_count = XFER_UDMA_5;
	if (udma_cap == ATA_UDMA4)
		cfis->sector_count = XFER_UDMA_4;
	if (udma_cap == ATA_UDMA3)
		cfis->sector_count = XFER_UDMA_3;

	fsl_sata_exec_cmd(sata, cfis, CMD_ATA, 0, NULL, 0);
}

static u32 fsl_sata_rw_cmd(fsl_sata_t *sata, u32 start, u32 blkcnt,
			   u8 *buffer, int is_write)
{
	struct sata_fis_h2d h2d, *cfis = &h2d;
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

	fsl_sata_exec_cmd(sata, cfis, CMD_ATA, 0, buffer,
			  ATA_SECT_SIZE * blkcnt);
	return blkcnt;
}

static void fsl_sata_flush_cache(fsl_sata_t *sata)
{
	struct sata_fis_h2d h2d, *cfis = &h2d;

	memset(cfis, 0, sizeof(struct sata_fis_h2d));

	cfis->fis_type = SATA_FIS_TYPE_REGISTER_H2D;
	cfis->pm_port_c = 0x80; /* is command */
	cfis->command = ATA_CMD_FLUSH;

	fsl_sata_exec_cmd(sata, cfis, CMD_ATA, 0, NULL, 0);
}

static u32 fsl_sata_rw_cmd_ext(fsl_sata_t *sata, u32 start,
			       u32 blkcnt, u8 *buffer, int is_write)
{
	struct sata_fis_h2d h2d, *cfis = &h2d;
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

	fsl_sata_exec_cmd(sata, cfis, CMD_ATA, 0, buffer,
			  ATA_SECT_SIZE * blkcnt);
	return blkcnt;
}

static u32 fsl_sata_rw_ncq_cmd(fsl_sata_t *sata, u32 start, u32 blkcnt,
			       u8 *buffer,
			       int is_write)
{
	struct sata_fis_h2d h2d, *cfis = &h2d;
	int ncq_channel;
	u64 block;

	if (sata->lba48 != 1) {
		printf("execute FPDMA command on non-LBA48 hard disk\n\r");
		return -1;
	}

	block = (u64)start;

	memset(cfis, 0, sizeof(struct sata_fis_h2d));

	cfis->fis_type = SATA_FIS_TYPE_REGISTER_H2D;
	cfis->pm_port_c = 0x80; /* is command */

	cfis->command = (is_write) ? ATA_CMD_FPDMA_WRITE
				 : ATA_CMD_FPDMA_READ;

	cfis->lba_high_exp = (block >> 40) & 0xff;
	cfis->lba_mid_exp = (block >> 32) & 0xff;
	cfis->lba_low_exp = (block >> 24) & 0xff;
	cfis->lba_high = (block >> 16) & 0xff;
	cfis->lba_mid = (block >> 8) & 0xff;
	cfis->lba_low = block & 0xff;

	cfis->device = ATA_LBA;
	cfis->features_exp = (blkcnt >> 8) & 0xff;
	cfis->features = blkcnt & 0xff;

	if (sata->queue_depth >= SATA_HC_MAX_CMD)
		ncq_channel = SATA_HC_MAX_CMD - 1;
	else
		ncq_channel = sata->queue_depth - 1;

	/* Use the latest queue */
	fsl_sata_exec_cmd(sata, cfis, CMD_NCQ, ncq_channel, buffer,
			  ATA_SECT_SIZE * blkcnt);
	return blkcnt;
}

static void fsl_sata_flush_cache_ext(fsl_sata_t *sata)
{
	struct sata_fis_h2d h2d, *cfis = &h2d;

	memset(cfis, 0, sizeof(struct sata_fis_h2d));

	cfis->fis_type = SATA_FIS_TYPE_REGISTER_H2D;
	cfis->pm_port_c = 0x80; /* is command */
	cfis->command = ATA_CMD_FLUSH_EXT;

	fsl_sata_exec_cmd(sata, cfis, CMD_ATA, 0, NULL, 0);
}

static u32 ata_low_level_rw_lba48(fsl_sata_t *sata, u32 blknr, lbaint_t blkcnt,
				  const void *buffer, int is_write)
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
			if (sata->dma_flag != FLAGS_FPDMA)
				fsl_sata_rw_cmd_ext(sata, start, max_blks,
						    addr, is_write);
			else
				fsl_sata_rw_ncq_cmd(sata, start, max_blks,
						    addr, is_write);
			start += max_blks;
			blks -= max_blks;
			addr += ATA_SECT_SIZE * max_blks;
		} else {
			if (sata->dma_flag != FLAGS_FPDMA)
				fsl_sata_rw_cmd_ext(sata, start, blks,
						    addr, is_write);
			else
				fsl_sata_rw_ncq_cmd(sata, start, blks,
						    addr, is_write);
			start += blks;
			blks = 0;
			addr += ATA_SECT_SIZE * blks;
		}
	} while (blks != 0);

	return blks;
}

static u32 ata_low_level_rw_lba28(fsl_sata_t *sata, u32 blknr, u32 blkcnt,
				  const void *buffer, int is_write)
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
			fsl_sata_rw_cmd(sata, start, max_blks, addr, is_write);
			start += max_blks;
			blks -= max_blks;
			addr += ATA_SECT_SIZE * max_blks;
		} else {
			fsl_sata_rw_cmd(sata, start, blks, addr, is_write);
			start += blks;
			blks = 0;
			addr += ATA_SECT_SIZE * blks;
		}
	} while (blks != 0);

	return blks;
}

/*
 * SATA interface between low level driver and command layer
 */
static int sata_read(fsl_sata_t *sata, ulong blknr, lbaint_t blkcnt,
		     void *buffer)
{
	u32 rc;

	if (sata->lba48)
		rc = ata_low_level_rw_lba48(sata, blknr, blkcnt, buffer,
					    READ_CMD);
	else
		rc = ata_low_level_rw_lba28(sata, blknr, blkcnt, buffer,
					    READ_CMD);
	return rc;
}

static int sata_write(fsl_sata_t *sata, ulong blknr, lbaint_t blkcnt,
		      const void *buffer)
{
	u32 rc;

	if (sata->lba48) {
		rc = ata_low_level_rw_lba48(sata, blknr, blkcnt, buffer,
					    WRITE_CMD);
		if (sata->wcache && sata->flush_ext)
			fsl_sata_flush_cache_ext(sata);
	} else {
		rc = ata_low_level_rw_lba28(sata, blknr, blkcnt, buffer,
					    WRITE_CMD);
		if (sata->wcache && sata->flush)
			fsl_sata_flush_cache(sata);
	}

	return rc;
}

int sata_getinfo(fsl_sata_t *sata, u16 *id)
{
	/* if no detected link */
	if (!sata->link)
		return -EINVAL;

#ifdef CONFIG_LBA48
	/* Check if support LBA48 */
	if (ata_id_has_lba48(id)) {
		sata->lba48 = 1;
		debug("Device support LBA48\n\r");
	} else {
		debug("Device supports LBA28\n\r");
	}
#endif

	/* Get the NCQ queue depth from device */
	sata->queue_depth = ata_id_queue_depth(id);

	/* Get the xfer mode from device */
	fsl_sata_xfer_mode(sata, id);

	/* Get the write cache status from device */
	fsl_sata_init_wcache(sata, id);

	/* Set the xfer mode to highest speed */
	fsl_sata_set_features(sata);

	return 0;
}

static int fsl_scsi_exec(fsl_sata_t *sata, struct scsi_cmd *pccb,
			 bool is_write)
{
	int ret;
	u32 temp;
	u16 blocks = 0;
	lbaint_t start = 0;
	u8 *buffer = pccb->pdata;

	/* Retrieve the base LBA number from the ccb structure. */
	if (pccb->cmd[0] == SCSI_READ16) {
		memcpy(&start, pccb->cmd + 2, 8);
		start = be64_to_cpu(start);
	} else {
		memcpy(&temp, pccb->cmd + 2, 4);
		start = be32_to_cpu(temp);
	}

	if (pccb->cmd[0] == SCSI_READ16)
		blocks = (((u16)pccb->cmd[13]) << 8) | ((u16)pccb->cmd[14]);
	else
		blocks = (((u16)pccb->cmd[7]) << 8) | ((u16)pccb->cmd[8]);

	debug("scsi_ahci: %s %u blocks starting from lba 0x" LBAFU "\n",
	      is_write ?  "write" : "read", blocks, start);

	if (is_write)
		ret = sata_write(sata, start, blocks, buffer);
	else
		ret = sata_read(sata, start, blocks, buffer);

	return ret;
}

static char *fsl_ata_id_strcpy(u16 *target, u16 *src, int len)
{
	int i;

	for (i = 0; i < len / 2; i++)
		target[i] = src[i];

	return (char *)target;
}

static int fsl_ata_scsiop_inquiry(struct ahci_uc_priv *uc_priv,
				  struct scsi_cmd *pccb,
				  fsl_sata_t *sata)
{
	u8 port;
	u16 *idbuf;

	ALLOC_CACHE_ALIGN_BUFFER(u16, tmpid, ATA_ID_WORDS);

	/* Clean ccb data buffer */
	memset(pccb->pdata, 0, pccb->datalen);

	if (pccb->datalen <= 35)
		return 0;

	/* Read id from sata */
	port = pccb->target;

	fsl_sata_identify(sata, (u16 *)tmpid);

	if (!uc_priv->ataid[port]) {
		uc_priv->ataid[port] = malloc(ATA_ID_WORDS * 2);
		if (!uc_priv->ataid[port]) {
			printf("%s: No memory for ataid[port]\n", __func__);
			return -ENOMEM;
		}
	}

	idbuf = uc_priv->ataid[port];

	memcpy(idbuf, tmpid, ATA_ID_WORDS * 2);

	memcpy(&pccb->pdata[8], "ATA     ", 8);
	fsl_ata_id_strcpy((u16 *)&pccb->pdata[16], &idbuf[ATA_ID_PROD], 16);
	fsl_ata_id_strcpy((u16 *)&pccb->pdata[32], &idbuf[ATA_ID_FW_REV], 4);

	sata_getinfo(sata, (u16 *)idbuf);
#ifdef DEBUG
	ata_dump_id(idbuf);
#endif
	return 0;
}

/*
 * SCSI READ CAPACITY10 command operation.
 */
static int fsl_ata_scsiop_read_capacity10(struct ahci_uc_priv *uc_priv,
					  struct scsi_cmd *pccb)
{
	u32 cap;
	u64 cap64;
	u32 block_size;

	if (!uc_priv->ataid[pccb->target]) {
		printf("scsi_ahci: SCSI READ CAPACITY10 command failure.");
		printf("\tNo ATA info!\n");
		printf("\tPlease run SCSI command INQUIRY first!\n");
		return -EPERM;
	}

	cap64 = ata_id_n_sectors(uc_priv->ataid[pccb->target]);
	if (cap64 > 0x100000000ULL)
		cap64 = 0xffffffff;

	cap = cpu_to_be32(cap64);
	memcpy(pccb->pdata, &cap, sizeof(cap));

	block_size = cpu_to_be32((u32)512);
	memcpy(&pccb->pdata[4], &block_size, 4);

	return 0;
}

/*
 * SCSI READ CAPACITY16 command operation.
 */
static int fsl_ata_scsiop_read_capacity16(struct ahci_uc_priv *uc_priv,
					  struct scsi_cmd *pccb)
{
	u64 cap;
	u64 block_size;

	if (!uc_priv->ataid[pccb->target]) {
		printf("scsi_ahci: SCSI READ CAPACITY16 command failure.");
		printf("\tNo ATA info!\n");
		printf("\tPlease run SCSI command INQUIRY first!\n");
		return -EPERM;
	}

	cap = ata_id_n_sectors(uc_priv->ataid[pccb->target]);
	cap = cpu_to_be64(cap);
	memcpy(pccb->pdata, &cap, sizeof(cap));

	block_size = cpu_to_be64((u64)512);
	memcpy(&pccb->pdata[8], &block_size, 8);

	return 0;
}

/*
 * SCSI TEST UNIT READY command operation.
 */
static int fsl_ata_scsiop_test_unit_ready(struct ahci_uc_priv *uc_priv,
					  struct scsi_cmd *pccb)
{
	return (uc_priv->ataid[pccb->target]) ? 0 : -EPERM;
}

static int fsl_ahci_scsi_exec(struct udevice *dev, struct scsi_cmd *pccb)
{
	struct ahci_uc_priv *uc_priv = dev_get_uclass_priv(dev->parent);
	struct fsl_ahci_priv *priv = dev_get_priv(dev->parent);
	fsl_sata_t *sata = priv->fsl_sata;
	int ret;

	switch (pccb->cmd[0]) {
	case SCSI_READ16:
	case SCSI_READ10:
		ret = fsl_scsi_exec(sata, pccb, 0);
		break;
	case SCSI_WRITE10:
		ret = fsl_scsi_exec(sata, pccb, 1);
		break;
	case SCSI_RD_CAPAC10:
		ret = fsl_ata_scsiop_read_capacity10(uc_priv, pccb);
		break;
	case SCSI_RD_CAPAC16:
		ret = fsl_ata_scsiop_read_capacity16(uc_priv, pccb);
		break;
	case SCSI_TST_U_RDY:
		ret = fsl_ata_scsiop_test_unit_ready(uc_priv, pccb);
		break;
	case SCSI_INQUIRY:
		ret = fsl_ata_scsiop_inquiry(uc_priv, pccb, sata);
		break;
	default:
		printf("Unsupport SCSI command 0x%02x\n", pccb->cmd[0]);
		return -ENOTSUPP;
	}

	if (ret) {
		debug("SCSI command 0x%02x ret errno %d\n", pccb->cmd[0], ret);
		return ret;
	}

	return 0;
}

static int fsl_ahci_probe(struct udevice *dev)
{
	struct fsl_ahci_priv *priv = dev_get_priv(dev);
	struct udevice *child_dev;
	struct scsi_platdata *uc_plat;

	device_find_first_child(dev, &child_dev);
	if (!child_dev)
		return -ENODEV;
	uc_plat = dev_get_uclass_platdata(child_dev);
	uc_plat->base = priv->base;
	uc_plat->max_lun = 1;
	uc_plat->max_id = 1;

	return init_sata(priv);
}

struct scsi_ops fsl_scsi_ops = {
	.exec		= fsl_ahci_scsi_exec,
};

static const struct udevice_id fsl_ahci_ids[] = {
	{ .compatible = "fsl,pq-sata-v2" },
	{ }
};

U_BOOT_DRIVER(fsl_ahci_scsi) = {
	.name		= "fsl_ahci_scsi",
	.id		= UCLASS_SCSI,
	.ops		= &fsl_scsi_ops,
};

U_BOOT_DRIVER(fsl_ahci) = {
	.name	= "fsl_ahci",
	.id	= UCLASS_AHCI,
	.of_match = fsl_ahci_ids,
	.bind	= fsl_ahci_bind,
	.ofdata_to_platdata = fsl_ahci_ofdata_to_platdata,
	.probe	= fsl_ahci_probe,
	.priv_auto_alloc_size = sizeof(struct fsl_ahci_priv),
};
