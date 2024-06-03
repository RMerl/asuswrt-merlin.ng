// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2009-2011 Freescale Semiconductor, Inc.
 *	Dave Liu <daveliu@freescale.com>
 */
#include <common.h>
#include <malloc.h>
#include <asm/io.h>
#include <linux/errno.h>

#include "fm.h"
#include <fsl_qe.h>		/* For struct qe_firmware */

#include <nand.h>
#include <spi_flash.h>
#include <mmc.h>
#include <environment.h>

#ifdef CONFIG_ARM64
#include <asm/armv8/mmu.h>
#include <asm/arch/cpu.h>
#endif

struct fm_muram muram[CONFIG_SYS_NUM_FMAN];

void *fm_muram_base(int fm_idx)
{
	return muram[fm_idx].base;
}

void *fm_muram_alloc(int fm_idx, size_t size, ulong align)
{
	void *ret;
	ulong align_mask;
	size_t off;
	void *save;

	align_mask = align - 1;
	save = muram[fm_idx].alloc;

	off = (ulong)save & align_mask;
	if (off != 0)
		muram[fm_idx].alloc += (align - off);
	off = size & align_mask;
	if (off != 0)
		size += (align - off);
	if ((muram[fm_idx].alloc + size) >= muram[fm_idx].top) {
		muram[fm_idx].alloc = save;
		printf("%s: run out of ram.\n", __func__);
		return NULL;
	}

	ret = muram[fm_idx].alloc;
	muram[fm_idx].alloc += size;
	memset((void *)ret, 0, size);

	return ret;
}

static void fm_init_muram(int fm_idx, void *reg)
{
	void *base = reg;

	muram[fm_idx].base = base;
	muram[fm_idx].size = CONFIG_SYS_FM_MURAM_SIZE;
	muram[fm_idx].alloc = base + FM_MURAM_RES_SIZE;
	muram[fm_idx].top = base + CONFIG_SYS_FM_MURAM_SIZE;
}

/*
 * fm_upload_ucode - Fman microcode upload worker function
 *
 * This function does the actual uploading of an Fman microcode
 * to an Fman.
 */
static void fm_upload_ucode(int fm_idx, struct fm_imem *imem,
			    u32 *ucode, unsigned int size)
{
	unsigned int i;
	unsigned int timeout = 1000000;

	/* enable address auto increase */
	out_be32(&imem->iadd, IRAM_IADD_AIE);
	/* write microcode to IRAM */
	for (i = 0; i < size / 4; i++)
		out_be32(&imem->idata, (be32_to_cpu(ucode[i])));

	/* verify if the writing is over */
	out_be32(&imem->iadd, 0);
	while ((in_be32(&imem->idata) != be32_to_cpu(ucode[0])) && --timeout)
		;
	if (!timeout)
		printf("Fman%u: microcode upload timeout\n", fm_idx + 1);

	/* enable microcode from IRAM */
	out_be32(&imem->iready, IRAM_READY);
}

/*
 * Upload an Fman firmware
 *
 * This function is similar to qe_upload_firmware(), exception that it uploads
 * a microcode to the Fman instead of the QE.
 *
 * Because the process for uploading a microcode to the Fman is similar for
 * that of the QE, the QE firmware binary format is used for Fman microcode.
 * It should be possible to unify these two functions, but for now we keep them
 * separate.
 */
static int fman_upload_firmware(int fm_idx,
				struct fm_imem *fm_imem,
				const struct qe_firmware *firmware)
{
	unsigned int i;
	u32 crc;
	size_t calc_size = sizeof(struct qe_firmware);
	size_t length;
	const struct qe_header *hdr;

	if (!firmware) {
		printf("Fman%u: Invalid address for firmware\n", fm_idx + 1);
		return -EINVAL;
	}

	hdr = &firmware->header;
	length = be32_to_cpu(hdr->length);

	/* Check the magic */
	if ((hdr->magic[0] != 'Q') || (hdr->magic[1] != 'E') ||
		(hdr->magic[2] != 'F')) {
		printf("Fman%u: Data at %p is not a firmware\n", fm_idx + 1,
		       firmware);
		return -EPERM;
	}

	/* Check the version */
	if (hdr->version != 1) {
		printf("Fman%u: Unsupported firmware version %u\n", fm_idx + 1,
		       hdr->version);
		return -EPERM;
	}

	/* Validate some of the fields */
	if ((firmware->count != 1)) {
		printf("Fman%u: Invalid data in firmware header\n", fm_idx + 1);
		return -EINVAL;
	}

	/* Validate the length and check if there's a CRC */
	calc_size += (firmware->count - 1) * sizeof(struct qe_microcode);

	for (i = 0; i < firmware->count; i++)
		/*
		 * For situations where the second RISC uses the same microcode
		 * as the first, the 'code_offset' and 'count' fields will be
		 * zero, so it's okay to add those.
		 */
		calc_size += sizeof(u32) *
			be32_to_cpu(firmware->microcode[i].count);

	/* Validate the length */
	if (length != calc_size + sizeof(u32)) {
		printf("Fman%u: Invalid length in firmware header\n",
		       fm_idx + 1);
		return -EPERM;
	}

	/*
	 * Validate the CRC.  We would normally call crc32_no_comp(), but that
	 * function isn't available unless you turn on JFFS support.
	 */
	crc = be32_to_cpu(*(u32 *)((void *)firmware + calc_size));
	if (crc != (crc32(-1, (const void *)firmware, calc_size) ^ -1)) {
		printf("Fman%u: Firmware CRC is invalid\n", fm_idx + 1);
		return -EIO;
	}

	/* Loop through each microcode. */
	for (i = 0; i < firmware->count; i++) {
		const struct qe_microcode *ucode = &firmware->microcode[i];

		/* Upload a microcode if it's present */
		if (be32_to_cpu(ucode->code_offset)) {
			u32 ucode_size;
			u32 *code;
			printf("Fman%u: Uploading microcode version %u.%u.%u\n",
			       fm_idx + 1, ucode->major, ucode->minor,
			       ucode->revision);
			code = (void *)firmware +
			       be32_to_cpu(ucode->code_offset);
			ucode_size = sizeof(u32) * be32_to_cpu(ucode->count);
			fm_upload_ucode(fm_idx, fm_imem, code, ucode_size);
		}
	}

	return 0;
}

static u32 fm_assign_risc(int port_id)
{
	u32 risc_sel, val;
	risc_sel = (port_id & 0x1) ? FMFPPRC_RISC2 : FMFPPRC_RISC1;
	val = (port_id << FMFPPRC_PORTID_SHIFT) & FMFPPRC_PORTID_MASK;
	val |= ((risc_sel << FMFPPRC_ORA_SHIFT) | risc_sel);

	return val;
}

static void fm_init_fpm(struct fm_fpm *fpm)
{
	int i, port_id;
	u32 val;

	setbits_be32(&fpm->fmfpee, FMFPEE_EHM | FMFPEE_UEC |
				   FMFPEE_CER | FMFPEE_DER);

	/* IM mode, each even port ID to RISC#1, each odd port ID to RISC#2 */

	/* offline/parser port */
	for (i = 0; i < MAX_NUM_OH_PORT; i++) {
		port_id = OH_PORT_ID_BASE + i;
		val = fm_assign_risc(port_id);
		out_be32(&fpm->fpmprc, val);
	}
	/* Rx 1G port */
	for (i = 0; i < MAX_NUM_RX_PORT_1G; i++) {
		port_id = RX_PORT_1G_BASE + i;
		val = fm_assign_risc(port_id);
		out_be32(&fpm->fpmprc, val);
	}
	/* Tx 1G port */
	for (i = 0; i < MAX_NUM_TX_PORT_1G; i++) {
		port_id = TX_PORT_1G_BASE + i;
		val = fm_assign_risc(port_id);
		out_be32(&fpm->fpmprc, val);
	}
	/* Rx 10G port */
	port_id = RX_PORT_10G_BASE;
	val = fm_assign_risc(port_id);
	out_be32(&fpm->fpmprc, val);
	/* Tx 10G port */
	port_id = TX_PORT_10G_BASE;
	val = fm_assign_risc(port_id);
	out_be32(&fpm->fpmprc, val);

	/* disable the dispatch limit in IM case */
	out_be32(&fpm->fpmflc, FMFP_FLC_DISP_LIM_NONE);
	/* clear events */
	out_be32(&fpm->fmfpee, FMFPEE_CLEAR_EVENT);

	/* clear risc events */
	for (i = 0; i < 4; i++)
		out_be32(&fpm->fpmcev[i], 0xffffffff);

	/* clear error */
	out_be32(&fpm->fpmrcr, FMFP_RCR_MDEC | FMFP_RCR_IDEC);
}

static int fm_init_bmi(int fm_idx, struct fm_bmi_common *bmi)
{
	int blk, i, port_id;
	u32 val;
	size_t offset;
	void *base;

	/* alloc free buffer pool in MURAM */
	base = fm_muram_alloc(fm_idx, FM_FREE_POOL_SIZE, FM_FREE_POOL_ALIGN);
	if (!base) {
		printf("%s: no muram for free buffer pool\n", __func__);
		return -ENOMEM;
	}
	offset = base - fm_muram_base(fm_idx);

	/* Need 128KB total free buffer pool size */
	val = offset / 256;
	blk = FM_FREE_POOL_SIZE / 256;
	/* in IM, we must not begin from offset 0 in MURAM */
	val |= ((blk - 1) << FMBM_CFG1_FBPS_SHIFT);
	out_be32(&bmi->fmbm_cfg1, val);

	/* disable all BMI interrupt */
	out_be32(&bmi->fmbm_ier, FMBM_IER_DISABLE_ALL);

	/* clear all events */
	out_be32(&bmi->fmbm_ievr, FMBM_IEVR_CLEAR_ALL);

	/*
	 * set port parameters - FMBM_PP_x
	 * max tasks 10G Rx/Tx=12, 1G Rx/Tx 4, others is 1
	 * max dma 10G Rx/Tx=3, others is 1
	 * set port FIFO size - FMBM_PFS_x
	 * 4KB for all Rx and Tx ports
	 */
	/* offline/parser port */
	for (i = 0; i < MAX_NUM_OH_PORT; i++) {
		port_id = OH_PORT_ID_BASE + i - 1;
		/* max tasks=1, max dma=1, no extra */
		out_be32(&bmi->fmbm_pp[port_id], 0);
		/* port FIFO size - 256 bytes, no extra */
		out_be32(&bmi->fmbm_pfs[port_id], 0);
	}
	/* Rx 1G port */
	for (i = 0; i < MAX_NUM_RX_PORT_1G; i++) {
		port_id = RX_PORT_1G_BASE + i - 1;
		/* max tasks=4, max dma=1, no extra */
		out_be32(&bmi->fmbm_pp[port_id], FMBM_PP_MXT(4));
		/* FIFO size - 4KB, no extra */
		out_be32(&bmi->fmbm_pfs[port_id], FMBM_PFS_IFSZ(0xf));
	}
	/* Tx 1G port FIFO size - 4KB, no extra */
	for (i = 0; i < MAX_NUM_TX_PORT_1G; i++) {
		port_id = TX_PORT_1G_BASE + i - 1;
		/* max tasks=4, max dma=1, no extra */
		out_be32(&bmi->fmbm_pp[port_id], FMBM_PP_MXT(4));
		/* FIFO size - 4KB, no extra */
		out_be32(&bmi->fmbm_pfs[port_id], FMBM_PFS_IFSZ(0xf));
	}
	/* Rx 10G port */
	port_id = RX_PORT_10G_BASE - 1;
	/* max tasks=12, max dma=3, no extra */
	out_be32(&bmi->fmbm_pp[port_id], FMBM_PP_MXT(12) | FMBM_PP_MXD(3));
	/* FIFO size - 4KB, no extra */
	out_be32(&bmi->fmbm_pfs[port_id], FMBM_PFS_IFSZ(0xf));

	/* Tx 10G port */
	port_id = TX_PORT_10G_BASE - 1;
	/* max tasks=12, max dma=3, no extra */
	out_be32(&bmi->fmbm_pp[port_id], FMBM_PP_MXT(12) | FMBM_PP_MXD(3));
	/* FIFO size - 4KB, no extra */
	out_be32(&bmi->fmbm_pfs[port_id], FMBM_PFS_IFSZ(0xf));

	/* initialize internal buffers data base (linked list) */
	out_be32(&bmi->fmbm_init, FMBM_INIT_START);

	return 0;
}

static void fm_init_qmi(struct fm_qmi_common *qmi)
{
	/* disable all error interrupts */
	out_be32(&qmi->fmqm_eien, FMQM_EIEN_DISABLE_ALL);
	/* clear all error events */
	out_be32(&qmi->fmqm_eie, FMQM_EIE_CLEAR_ALL);

	/* disable all interrupts */
	out_be32(&qmi->fmqm_ien, FMQM_IEN_DISABLE_ALL);
	/* clear all interrupts */
	out_be32(&qmi->fmqm_ie, FMQM_IE_CLEAR_ALL);
}

/* Init common part of FM, index is fm num# like fm as above */
#ifdef CONFIG_TFABOOT
int fm_init_common(int index, struct ccsr_fman *reg)
{
	int rc;
	void *addr = NULL;
	enum boot_src src = get_boot_src();

	if (src == BOOT_SOURCE_IFC_NOR) {
		addr = (void *)(CONFIG_SYS_FMAN_FW_ADDR +
				CONFIG_SYS_FSL_IFC_BASE);
	} else if (src == BOOT_SOURCE_IFC_NAND) {
		size_t fw_length = CONFIG_SYS_QE_FMAN_FW_LENGTH;

		addr = malloc(CONFIG_SYS_QE_FMAN_FW_LENGTH);

		rc = nand_read(get_nand_dev_by_index(0),
			       (loff_t)CONFIG_SYS_FMAN_FW_ADDR,
			       &fw_length, (u_char *)addr);
		if (rc == -EUCLEAN) {
			printf("NAND read of FMAN firmware at offset 0x%x failed %d\n",
			       CONFIG_SYS_FMAN_FW_ADDR, rc);
		}
	} else if (src == BOOT_SOURCE_QSPI_NOR) {
		struct spi_flash *ucode_flash;

		addr = malloc(CONFIG_SYS_QE_FMAN_FW_LENGTH);
		int ret = 0;

#ifdef CONFIG_DM_SPI_FLASH
		struct udevice *new;

		/* speed and mode will be read from DT */
		ret = spi_flash_probe_bus_cs(CONFIG_ENV_SPI_BUS,
					     CONFIG_ENV_SPI_CS, 0, 0, &new);

		ucode_flash = dev_get_uclass_priv(new);
#else
		ucode_flash = spi_flash_probe(CONFIG_ENV_SPI_BUS,
					      CONFIG_ENV_SPI_CS,
					      CONFIG_ENV_SPI_MAX_HZ,
					      CONFIG_ENV_SPI_MODE);
#endif
		if (!ucode_flash) {
			printf("SF: probe for ucode failed\n");
		} else {
			ret = spi_flash_read(ucode_flash,
					     CONFIG_SYS_FMAN_FW_ADDR +
					     CONFIG_SYS_FSL_QSPI_BASE,
					     CONFIG_SYS_QE_FMAN_FW_LENGTH,
					     addr);
			if (ret)
				printf("SF: read for ucode failed\n");
			spi_flash_free(ucode_flash);
		}
	} else if (src == BOOT_SOURCE_SD_MMC) {
		int dev = CONFIG_SYS_MMC_ENV_DEV;

		addr = malloc(CONFIG_SYS_QE_FMAN_FW_LENGTH);
		u32 cnt = CONFIG_SYS_QE_FMAN_FW_LENGTH / 512;
		u32 blk = CONFIG_SYS_FMAN_FW_ADDR / 512;
		struct mmc *mmc = find_mmc_device(CONFIG_SYS_MMC_ENV_DEV);

		if (!mmc) {
			printf("\nMMC cannot find device for ucode\n");
		} else {
			printf("\nMMC read: dev # %u, block # %u, count %u ...\n",
			       dev, blk, cnt);
			mmc_init(mmc);
			(void)blk_dread(mmc_get_blk_desc(mmc), blk, cnt,
						addr);
		}
	} else {
		addr = NULL;
	}

	/* Upload the Fman microcode if it's present */
	rc = fman_upload_firmware(index, &reg->fm_imem, addr);
	if (rc)
		return rc;
	env_set_addr("fman_ucode", addr);

	fm_init_muram(index, &reg->muram);
	fm_init_qmi(&reg->fm_qmi_common);
	fm_init_fpm(&reg->fm_fpm);

	/* clear DMA status */
	setbits_be32(&reg->fm_dma.fmdmsr, FMDMSR_CLEAR_ALL);

	/* set DMA mode */
	setbits_be32(&reg->fm_dma.fmdmmr, FMDMMR_SBER);

	return fm_init_bmi(index, &reg->fm_bmi_common);
}
#else
int fm_init_common(int index, struct ccsr_fman *reg)
{
	int rc;
#if defined(CONFIG_SYS_QE_FMAN_FW_IN_NOR)
	void *addr = (void *)CONFIG_SYS_FMAN_FW_ADDR;
#elif defined(CONFIG_SYS_QE_FMAN_FW_IN_NAND)
	size_t fw_length = CONFIG_SYS_QE_FMAN_FW_LENGTH;
	void *addr = malloc(CONFIG_SYS_QE_FMAN_FW_LENGTH);

	rc = nand_read(get_nand_dev_by_index(0),
		       (loff_t)CONFIG_SYS_FMAN_FW_ADDR,
		       &fw_length, (u_char *)addr);
	if (rc == -EUCLEAN) {
		printf("NAND read of FMAN firmware at offset 0x%x failed %d\n",
			CONFIG_SYS_FMAN_FW_ADDR, rc);
	}
#elif defined(CONFIG_SYS_QE_FMAN_FW_IN_SPIFLASH)
	struct spi_flash *ucode_flash;
	void *addr = malloc(CONFIG_SYS_QE_FMAN_FW_LENGTH);
	int ret = 0;

#ifdef CONFIG_DM_SPI_FLASH
	struct udevice *new;

	/* speed and mode will be read from DT */
	ret = spi_flash_probe_bus_cs(CONFIG_ENV_SPI_BUS, CONFIG_ENV_SPI_CS,
				     0, 0, &new);

	ucode_flash = dev_get_uclass_priv(new);
#else
	ucode_flash = spi_flash_probe(CONFIG_ENV_SPI_BUS, CONFIG_ENV_SPI_CS,
			CONFIG_ENV_SPI_MAX_HZ, CONFIG_ENV_SPI_MODE);
#endif
	if (!ucode_flash)
		printf("SF: probe for ucode failed\n");
	else {
		ret = spi_flash_read(ucode_flash, CONFIG_SYS_FMAN_FW_ADDR,
				CONFIG_SYS_QE_FMAN_FW_LENGTH, addr);
		if (ret)
			printf("SF: read for ucode failed\n");
		spi_flash_free(ucode_flash);
	}
#elif defined(CONFIG_SYS_QE_FMAN_FW_IN_MMC)
	int dev = CONFIG_SYS_MMC_ENV_DEV;
	void *addr = malloc(CONFIG_SYS_QE_FMAN_FW_LENGTH);
	u32 cnt = CONFIG_SYS_QE_FMAN_FW_LENGTH / 512;
	u32 blk = CONFIG_SYS_FMAN_FW_ADDR / 512;
	struct mmc *mmc = find_mmc_device(CONFIG_SYS_MMC_ENV_DEV);

	if (!mmc)
		printf("\nMMC cannot find device for ucode\n");
	else {
		printf("\nMMC read: dev # %u, block # %u, count %u ...\n",
				dev, blk, cnt);
		mmc_init(mmc);
		(void)blk_dread(mmc_get_blk_desc(mmc), blk, cnt,
						addr);
	}
#elif defined(CONFIG_SYS_QE_FMAN_FW_IN_REMOTE)
	void *addr = (void *)CONFIG_SYS_FMAN_FW_ADDR;
#else
	void *addr = NULL;
#endif

	/* Upload the Fman microcode if it's present */
	rc = fman_upload_firmware(index, &reg->fm_imem, addr);
	if (rc)
		return rc;
	env_set_addr("fman_ucode", addr);

	fm_init_muram(index, &reg->muram);
	fm_init_qmi(&reg->fm_qmi_common);
	fm_init_fpm(&reg->fm_fpm);

	/* clear DMA status */
	setbits_be32(&reg->fm_dma.fmdmsr, FMDMSR_CLEAR_ALL);

	/* set DMA mode */
	setbits_be32(&reg->fm_dma.fmdmmr, FMDMMR_SBER);

	return fm_init_bmi(index, &reg->fm_bmi_common);
}
#endif
