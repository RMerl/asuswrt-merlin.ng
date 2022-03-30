// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2006-2009 Freescale Semiconductor, Inc.
 *
 * Dave Liu <daveliu@freescale.com>
 * based on source code of Shlomi Gridish
 */

#include <common.h>
#include <malloc.h>
#include <command.h>
#include <linux/errno.h>
#include <asm/io.h>
#include <linux/immap_qe.h>
#include <fsl_qe.h>
#include <mmc.h>
#include <environment.h>

#ifdef CONFIG_ARCH_LS1021A
#include <asm/arch/immap_ls102xa.h>
#endif
#ifdef CONFIG_ARM64
#include <asm/armv8/mmu.h>
#include <asm/arch/cpu.h>
#endif

#define MPC85xx_DEVDISR_QE_DISABLE	0x1

qe_map_t		*qe_immr = NULL;
#ifdef CONFIG_QE
static qe_snum_t	snums[QE_NUM_OF_SNUM];
#endif

DECLARE_GLOBAL_DATA_PTR;

void qe_issue_cmd(uint cmd, uint sbc, u8 mcn, u32 cmd_data)
{
	u32 cecr;

	if (cmd == QE_RESET) {
		out_be32(&qe_immr->cp.cecr,(u32) (cmd | QE_CR_FLG));
	} else {
		out_be32(&qe_immr->cp.cecdr, cmd_data);
		out_be32(&qe_immr->cp.cecr, (sbc | QE_CR_FLG |
			 ((u32) mcn<<QE_CR_PROTOCOL_SHIFT) | cmd));
	}
	/* Wait for the QE_CR_FLG to clear */
	do {
		cecr = in_be32(&qe_immr->cp.cecr);
	} while (cecr & QE_CR_FLG);

	return;
}

#ifdef CONFIG_QE
uint qe_muram_alloc(uint size, uint align)
{
	uint	retloc;
	uint	align_mask, off;
	uint	savebase;

	align_mask = align - 1;
	savebase = gd->arch.mp_alloc_base;

	off = gd->arch.mp_alloc_base & align_mask;
	if (off != 0)
		gd->arch.mp_alloc_base += (align - off);

	if ((off = size & align_mask) != 0)
		size += (align - off);

	if ((gd->arch.mp_alloc_base + size) >= gd->arch.mp_alloc_top) {
		gd->arch.mp_alloc_base = savebase;
		printf("%s: ran out of ram.\n",  __FUNCTION__);
	}

	retloc = gd->arch.mp_alloc_base;
	gd->arch.mp_alloc_base += size;

	memset((void *)&qe_immr->muram[retloc], 0, size);

	__asm__ __volatile__("sync");

	return retloc;
}
#endif

void *qe_muram_addr(uint offset)
{
	return (void *)&qe_immr->muram[offset];
}

#ifdef CONFIG_QE
static void qe_sdma_init(void)
{
	volatile sdma_t	*p;
	uint		sdma_buffer_base;

	p = (volatile sdma_t *)&qe_immr->sdma;

	/* All of DMA transaction in bus 1 */
	out_be32(&p->sdaqr, 0);
	out_be32(&p->sdaqmr, 0);

	/* Allocate 2KB temporary buffer for sdma */
	sdma_buffer_base = qe_muram_alloc(2048, 4096);
	out_be32(&p->sdwbcr, sdma_buffer_base & QE_SDEBCR_BA_MASK);

	/* Clear sdma status */
	out_be32(&p->sdsr, 0x03000000);

	/* Enable global mode on bus 1, and 2KB buffer size */
	out_be32(&p->sdmr, QE_SDMR_GLB_1_MSK | (0x3 << QE_SDMR_CEN_SHIFT));
}

/* This table is a list of the serial numbers of the Threads, taken from the
 * "SNUM Table" chart in the QE Reference Manual. The order is not important,
 * we just need to know what the SNUMs are for the threads.
 */
static u8 thread_snum[] = {
/* Evthreads 16-29 are not supported in MPC8309 */
#if !defined(CONFIG_ARCH_MPC8309)
	0x04, 0x05, 0x0c, 0x0d,
	0x14, 0x15, 0x1c, 0x1d,
	0x24, 0x25, 0x2c, 0x2d,
	0x34, 0x35,
#endif
	0x88, 0x89, 0x98, 0x99,
	0xa8, 0xa9, 0xb8, 0xb9,
	0xc8, 0xc9, 0xd8, 0xd9,
	0xe8, 0xe9, 0x08, 0x09,
	0x18, 0x19, 0x28, 0x29,
	0x38, 0x39, 0x48, 0x49,
	0x58, 0x59, 0x68, 0x69,
	0x78, 0x79, 0x80, 0x81
};

static void qe_snums_init(void)
{
	int	i;

	for (i = 0; i < QE_NUM_OF_SNUM; i++) {
		snums[i].state = QE_SNUM_STATE_FREE;
		snums[i].num   = thread_snum[i];
	}
}

int qe_get_snum(void)
{
	int	snum = -EBUSY;
	int	i;

	for (i = 0; i < QE_NUM_OF_SNUM; i++) {
		if (snums[i].state == QE_SNUM_STATE_FREE) {
			snums[i].state = QE_SNUM_STATE_USED;
			snum = snums[i].num;
			break;
		}
	}

	return snum;
}

void qe_put_snum(u8 snum)
{
	int	i;

	for (i = 0; i < QE_NUM_OF_SNUM; i++) {
		if (snums[i].num == snum) {
			snums[i].state = QE_SNUM_STATE_FREE;
			break;
		}
	}
}

#ifdef CONFIG_TFABOOT
void qe_init(uint qe_base)
{
	enum boot_src src = get_boot_src();

	/* Init the QE IMMR base */
	qe_immr = (qe_map_t *)qe_base;

	if (src == BOOT_SOURCE_IFC_NOR) {
		/*
		 * Upload microcode to IRAM for those SOCs
		 * which do not have ROM in QE.
		 */
		qe_upload_firmware((const void *)(CONFIG_SYS_QE_FW_ADDR +
				   CONFIG_SYS_FSL_IFC_BASE));

		/* enable the microcode in IRAM */
		out_be32(&qe_immr->iram.iready, QE_IRAM_READY);
	}

	gd->arch.mp_alloc_base = QE_DATAONLY_BASE;
	gd->arch.mp_alloc_top = gd->arch.mp_alloc_base + QE_DATAONLY_SIZE;

	qe_sdma_init();
	qe_snums_init();
}
#else
void qe_init(uint qe_base)
{
	/* Init the QE IMMR base */
	qe_immr = (qe_map_t *)qe_base;

#ifdef CONFIG_SYS_QE_FMAN_FW_IN_NOR
	/*
	 * Upload microcode to IRAM for those SOCs which do not have ROM in QE.
	 */
	qe_upload_firmware((const void *)CONFIG_SYS_QE_FW_ADDR);

	/* enable the microcode in IRAM */
	out_be32(&qe_immr->iram.iready,QE_IRAM_READY);
#endif

	gd->arch.mp_alloc_base = QE_DATAONLY_BASE;
	gd->arch.mp_alloc_top = gd->arch.mp_alloc_base + QE_DATAONLY_SIZE;

	qe_sdma_init();
	qe_snums_init();
}
#endif
#endif

#ifdef CONFIG_U_QE
#ifdef CONFIG_TFABOOT
void u_qe_init(void)
{
	enum boot_src src = get_boot_src();

	qe_immr = (qe_map_t *)(CONFIG_SYS_IMMR + QE_IMMR_OFFSET);

	void *addr = (void *)CONFIG_SYS_QE_FW_ADDR;

	if (src == BOOT_SOURCE_IFC_NOR)
		addr = (void *)(CONFIG_SYS_QE_FW_ADDR + CONFIG_SYS_FSL_IFC_BASE);

	if (src == BOOT_SOURCE_QSPI_NOR)
		addr = (void *)(CONFIG_SYS_QE_FW_ADDR + CONFIG_SYS_FSL_QSPI_BASE);

	if (src == BOOT_SOURCE_SD_MMC) {
		int dev = CONFIG_SYS_MMC_ENV_DEV;
		u32 cnt = CONFIG_SYS_QE_FMAN_FW_LENGTH / 512;
		u32 blk = CONFIG_SYS_QE_FW_ADDR / 512;

		if (mmc_initialize(gd->bd)) {
			printf("%s: mmc_initialize() failed\n", __func__);
			return;
		}
		addr = malloc(CONFIG_SYS_QE_FMAN_FW_LENGTH);
		struct mmc *mmc = find_mmc_device(CONFIG_SYS_MMC_ENV_DEV);

		if (!mmc) {
			free(addr);
			printf("\nMMC cannot find device for ucode\n");
		} else {
			printf("\nMMC read: dev # %u, block # %u, count %u ...\n",
			       dev, blk, cnt);
			mmc_init(mmc);
			(void)blk_dread(mmc_get_blk_desc(mmc), blk, cnt,
						addr);
		}
	}
	if (!u_qe_upload_firmware(addr))
		out_be32(&qe_immr->iram.iready, QE_IRAM_READY);
	if (src == BOOT_SOURCE_SD_MMC)
		free(addr);
}
#else
void u_qe_init(void)
{
	qe_immr = (qe_map_t *)(CONFIG_SYS_IMMR + QE_IMMR_OFFSET);

	void *addr = (void *)CONFIG_SYS_QE_FW_ADDR;
#ifdef CONFIG_SYS_QE_FMAN_FW_IN_MMC
	int dev = CONFIG_SYS_MMC_ENV_DEV;
	u32 cnt = CONFIG_SYS_QE_FMAN_FW_LENGTH / 512;
	u32 blk = CONFIG_SYS_QE_FW_ADDR / 512;

	if (mmc_initialize(gd->bd)) {
		printf("%s: mmc_initialize() failed\n", __func__);
		return;
	}
	addr = malloc(CONFIG_SYS_QE_FMAN_FW_LENGTH);
	struct mmc *mmc = find_mmc_device(CONFIG_SYS_MMC_ENV_DEV);

	if (!mmc) {
		free(addr);
		printf("\nMMC cannot find device for ucode\n");
	} else {
		printf("\nMMC read: dev # %u, block # %u, count %u ...\n",
		       dev, blk, cnt);
		mmc_init(mmc);
		(void)blk_dread(mmc_get_blk_desc(mmc), blk, cnt,
						addr);
	}
#endif
	if (!u_qe_upload_firmware(addr))
		out_be32(&qe_immr->iram.iready, QE_IRAM_READY);
#ifdef CONFIG_SYS_QE_FMAN_FW_IN_MMC
	free(addr);
#endif
}
#endif
#endif

#ifdef CONFIG_U_QE
void u_qe_resume(void)
{
	qe_map_t *qe_immrr;

	qe_immrr = (qe_map_t *)(CONFIG_SYS_IMMR + QE_IMMR_OFFSET);
	u_qe_firmware_resume((const void *)CONFIG_SYS_QE_FW_ADDR, qe_immrr);
	out_be32(&qe_immrr->iram.iready, QE_IRAM_READY);
}
#endif

void qe_reset(void)
{
	qe_issue_cmd(QE_RESET, QE_CR_SUBBLOCK_INVALID,
			 (u8) QE_CR_PROTOCOL_UNSPECIFIED, 0);
}

#ifdef CONFIG_QE
void qe_assign_page(uint snum, uint para_ram_base)
{
	u32	cecr;

	out_be32(&qe_immr->cp.cecdr, para_ram_base);
	out_be32(&qe_immr->cp.cecr, ((u32) snum<<QE_CR_ASSIGN_PAGE_SNUM_SHIFT)
					 | QE_CR_FLG | QE_ASSIGN_PAGE);

	/* Wait for the QE_CR_FLG to clear */
	do {
		cecr = in_be32(&qe_immr->cp.cecr);
	} while (cecr & QE_CR_FLG );

	return;
}
#endif

/*
 * brg: 0~15 as BRG1~BRG16
   rate: baud rate
 * BRG input clock comes from the BRGCLK (internal clock generated from
   the QE clock, it is one-half of the QE clock), If need the clock source
   from CLKn pin, we have te change the function.
 */

#define BRG_CLK		(gd->arch.brg_clk)

#ifdef CONFIG_QE
int qe_set_brg(uint brg, uint rate)
{
	volatile uint	*bp;
	u32		divisor;
	int		div16 = 0;

	if (brg >= QE_NUM_OF_BRGS)
		return -EINVAL;
	bp = (uint *)&qe_immr->brg.brgc1;
	bp += brg;

	divisor = (BRG_CLK / rate);
	if (divisor > QE_BRGC_DIVISOR_MAX + 1) {
		div16 = 1;
		divisor /= 16;
	}

	*bp = ((divisor - 1) << QE_BRGC_DIVISOR_SHIFT) | QE_BRGC_ENABLE;
	__asm__ __volatile__("sync");

	if (div16) {
		*bp |= QE_BRGC_DIV16;
		__asm__ __volatile__("sync");
	}

	return 0;
}
#endif

/* Set ethernet MII clock master
*/
int qe_set_mii_clk_src(int ucc_num)
{
	u32	cmxgcr;

	/* check if the UCC number is in range. */
	if ((ucc_num > UCC_MAX_NUM - 1) || (ucc_num < 0)) {
		printf("%s: ucc num not in ranges\n", __FUNCTION__);
		return -EINVAL;
	}

	cmxgcr = in_be32(&qe_immr->qmx.cmxgcr);
	cmxgcr &= ~QE_CMXGCR_MII_ENET_MNG_MASK;
	cmxgcr |= (ucc_num <<QE_CMXGCR_MII_ENET_MNG_SHIFT);
	out_be32(&qe_immr->qmx.cmxgcr, cmxgcr);

	return 0;
}

/* Firmware information stored here for qe_get_firmware_info() */
static struct qe_firmware_info qe_firmware_info;

/*
 * Set to 1 if QE firmware has been uploaded, and therefore
 * qe_firmware_info contains valid data.
 */
static int qe_firmware_uploaded;

/*
 * Upload a QE microcode
 *
 * This function is a worker function for qe_upload_firmware().  It does
 * the actual uploading of the microcode.
 */
static void qe_upload_microcode(const void *base,
	const struct qe_microcode *ucode)
{
	const u32 *code = base + be32_to_cpu(ucode->code_offset);
	unsigned int i;

	if (ucode->major || ucode->minor || ucode->revision)
		printf("QE: uploading microcode '%s' version %u.%u.%u\n",
		       (char *)ucode->id, (u16)ucode->major, (u16)ucode->minor,
		       (u16)ucode->revision);
	else
		printf("QE: uploading microcode '%s'\n", (char *)ucode->id);

	/* Use auto-increment */
	out_be32(&qe_immr->iram.iadd, be32_to_cpu(ucode->iram_offset) |
		QE_IRAM_IADD_AIE | QE_IRAM_IADD_BADDR);

	for (i = 0; i < be32_to_cpu(ucode->count); i++)
		out_be32(&qe_immr->iram.idata, be32_to_cpu(code[i]));
}

/*
 * Upload a microcode to the I-RAM at a specific address.
 *
 * See docs/README.qe_firmware for information on QE microcode uploading.
 *
 * Currently, only version 1 is supported, so the 'version' field must be
 * set to 1.
 *
 * The SOC model and revision are not validated, they are only displayed for
 * informational purposes.
 *
 * 'calc_size' is the calculated size, in bytes, of the firmware structure and
 * all of the microcode structures, minus the CRC.
 *
 * 'length' is the size that the structure says it is, including the CRC.
 */
int qe_upload_firmware(const struct qe_firmware *firmware)
{
	unsigned int i;
	unsigned int j;
	u32 crc;
	size_t calc_size = sizeof(struct qe_firmware);
	size_t length;
	const struct qe_header *hdr;
#ifdef CONFIG_DEEP_SLEEP
#ifdef CONFIG_ARCH_LS1021A
	struct ccsr_gur __iomem *gur = (void *)CONFIG_SYS_FSL_GUTS_ADDR;
#else
	ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
#endif
#endif
	if (!firmware) {
		printf("Invalid address\n");
		return -EINVAL;
	}

	hdr = &firmware->header;
	length = be32_to_cpu(hdr->length);

	/* Check the magic */
	if ((hdr->magic[0] != 'Q') || (hdr->magic[1] != 'E') ||
	    (hdr->magic[2] != 'F')) {
		printf("QE microcode not found\n");
#ifdef CONFIG_DEEP_SLEEP
		setbits_be32(&gur->devdisr, MPC85xx_DEVDISR_QE_DISABLE);
#endif
		return -EPERM;
	}

	/* Check the version */
	if (hdr->version != 1) {
		printf("Unsupported version\n");
		return -EPERM;
	}

	/* Validate some of the fields */
	if ((firmware->count < 1) || (firmware->count > MAX_QE_RISC)) {
		printf("Invalid data\n");
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
		printf("Invalid length\n");
		return -EPERM;
	}

	/*
	 * Validate the CRC.  We would normally call crc32_no_comp(), but that
	 * function isn't available unless you turn on JFFS support.
	 */
	crc = be32_to_cpu(*(u32 *)((void *)firmware + calc_size));
	if (crc != (crc32(-1, (const void *) firmware, calc_size) ^ -1)) {
		printf("Firmware CRC is invalid\n");
		return -EIO;
	}

	/*
	 * If the microcode calls for it, split the I-RAM.
	 */
	if (!firmware->split) {
		out_be16(&qe_immr->cp.cercr,
			in_be16(&qe_immr->cp.cercr) | QE_CP_CERCR_CIR);
	}

	if (firmware->soc.model)
		printf("Firmware '%s' for %u V%u.%u\n",
			firmware->id, be16_to_cpu(firmware->soc.model),
			firmware->soc.major, firmware->soc.minor);
	else
		printf("Firmware '%s'\n", firmware->id);

	/*
	 * The QE only supports one microcode per RISC, so clear out all the
	 * saved microcode information and put in the new.
	 */
	memset(&qe_firmware_info, 0, sizeof(qe_firmware_info));
	strncpy(qe_firmware_info.id, (char *)firmware->id, 62);
	qe_firmware_info.extended_modes = firmware->extended_modes;
	memcpy(qe_firmware_info.vtraps, firmware->vtraps,
		sizeof(firmware->vtraps));
	qe_firmware_uploaded = 1;

	/* Loop through each microcode. */
	for (i = 0; i < firmware->count; i++) {
		const struct qe_microcode *ucode = &firmware->microcode[i];

		/* Upload a microcode if it's present */
		if (ucode->code_offset)
			qe_upload_microcode(firmware, ucode);

		/* Program the traps for this processor */
		for (j = 0; j < 16; j++) {
			u32 trap = be32_to_cpu(ucode->traps[j]);

			if (trap)
				out_be32(&qe_immr->rsp[i].tibcr[j], trap);
		}

		/* Enable traps */
		out_be32(&qe_immr->rsp[i].eccr, be32_to_cpu(ucode->eccr));
	}

	return 0;
}

#ifdef CONFIG_U_QE
/*
 * Upload a microcode to the I-RAM at a specific address.
 *
 * See docs/README.qe_firmware for information on QE microcode uploading.
 *
 * Currently, only version 1 is supported, so the 'version' field must be
 * set to 1.
 *
 * The SOC model and revision are not validated, they are only displayed for
 * informational purposes.
 *
 * 'calc_size' is the calculated size, in bytes, of the firmware structure and
 * all of the microcode structures, minus the CRC.
 *
 * 'length' is the size that the structure says it is, including the CRC.
 */
int u_qe_upload_firmware(const struct qe_firmware *firmware)
{
	unsigned int i;
	unsigned int j;
	u32 crc;
	size_t calc_size = sizeof(struct qe_firmware);
	size_t length;
	const struct qe_header *hdr;
#ifdef CONFIG_DEEP_SLEEP
#ifdef CONFIG_ARCH_LS1021A
	struct ccsr_gur __iomem *gur = (void *)CONFIG_SYS_FSL_GUTS_ADDR;
#else
	ccsr_gur_t __iomem *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
#endif
#endif
	if (!firmware) {
		printf("Invalid address\n");
		return -EINVAL;
	}

	hdr = &firmware->header;
	length = be32_to_cpu(hdr->length);

	/* Check the magic */
	if ((hdr->magic[0] != 'Q') || (hdr->magic[1] != 'E') ||
	    (hdr->magic[2] != 'F')) {
		printf("Not a microcode\n");
#ifdef CONFIG_DEEP_SLEEP
		setbits_be32(&gur->devdisr, MPC85xx_DEVDISR_QE_DISABLE);
#endif
		return -EPERM;
	}

	/* Check the version */
	if (hdr->version != 1) {
		printf("Unsupported version\n");
		return -EPERM;
	}

	/* Validate some of the fields */
	if ((firmware->count < 1) || (firmware->count > MAX_QE_RISC)) {
		printf("Invalid data\n");
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
		printf("Invalid length\n");
		return -EPERM;
	}

	/*
	 * Validate the CRC.  We would normally call crc32_no_comp(), but that
	 * function isn't available unless you turn on JFFS support.
	 */
	crc = be32_to_cpu(*(u32 *)((void *)firmware + calc_size));
	if (crc != (crc32(-1, (const void *)firmware, calc_size) ^ -1)) {
		printf("Firmware CRC is invalid\n");
		return -EIO;
	}

	/*
	 * If the microcode calls for it, split the I-RAM.
	 */
	if (!firmware->split) {
		out_be16(&qe_immr->cp.cercr,
			 in_be16(&qe_immr->cp.cercr) | QE_CP_CERCR_CIR);
	}

	if (firmware->soc.model)
		printf("Firmware '%s' for %u V%u.%u\n",
		       firmware->id, be16_to_cpu(firmware->soc.model),
		       firmware->soc.major, firmware->soc.minor);
	else
		printf("Firmware '%s'\n", firmware->id);

	/* Loop through each microcode. */
	for (i = 0; i < firmware->count; i++) {
		const struct qe_microcode *ucode = &firmware->microcode[i];

		/* Upload a microcode if it's present */
		if (ucode->code_offset)
			qe_upload_microcode(firmware, ucode);

		/* Program the traps for this processor */
		for (j = 0; j < 16; j++) {
			u32 trap = be32_to_cpu(ucode->traps[j]);

			if (trap)
				out_be32(&qe_immr->rsp[i].tibcr[j], trap);
		}

		/* Enable traps */
		out_be32(&qe_immr->rsp[i].eccr, be32_to_cpu(ucode->eccr));
	}

	return 0;
}
#endif

#ifdef CONFIG_U_QE
int u_qe_firmware_resume(const struct qe_firmware *firmware, qe_map_t *qe_immrr)
{
	unsigned int i;
	unsigned int j;
	const struct qe_header *hdr;
	const u32 *code;
#ifdef CONFIG_DEEP_SLEEP
#ifdef CONFIG_PPC
	ccsr_gur_t __iomem *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
#else
	struct ccsr_gur __iomem *gur = (void *)CONFIG_SYS_FSL_GUTS_ADDR;
#endif
#endif

	if (!firmware)
		return -EINVAL;

	hdr = &firmware->header;

	/* Check the magic */
	if ((hdr->magic[0] != 'Q') || (hdr->magic[1] != 'E') ||
	    (hdr->magic[2] != 'F')) {
#ifdef CONFIG_DEEP_SLEEP
		setbits_be32(&gur->devdisr, MPC85xx_DEVDISR_QE_DISABLE);
#endif
		return -EPERM;
	}

	/*
	 * If the microcode calls for it, split the I-RAM.
	 */
	if (!firmware->split) {
		out_be16(&qe_immrr->cp.cercr,
			 in_be16(&qe_immrr->cp.cercr) | QE_CP_CERCR_CIR);
	}

	/* Loop through each microcode. */
	for (i = 0; i < firmware->count; i++) {
		const struct qe_microcode *ucode = &firmware->microcode[i];

		/* Upload a microcode if it's present */
		if (!ucode->code_offset)
			return 0;

		code = (const void *)firmware + be32_to_cpu(ucode->code_offset);

		/* Use auto-increment */
		out_be32(&qe_immrr->iram.iadd, be32_to_cpu(ucode->iram_offset) |
			QE_IRAM_IADD_AIE | QE_IRAM_IADD_BADDR);

		for (i = 0; i < be32_to_cpu(ucode->count); i++)
			out_be32(&qe_immrr->iram.idata, be32_to_cpu(code[i]));

		/* Program the traps for this processor */
		for (j = 0; j < 16; j++) {
			u32 trap = be32_to_cpu(ucode->traps[j]);

			if (trap)
				out_be32(&qe_immrr->rsp[i].tibcr[j], trap);
		}

		/* Enable traps */
		out_be32(&qe_immrr->rsp[i].eccr, be32_to_cpu(ucode->eccr));
	}

	return 0;
}
#endif

struct qe_firmware_info *qe_get_firmware_info(void)
{
	return qe_firmware_uploaded ? &qe_firmware_info : NULL;
}

static int qe_cmd(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	ulong addr;

	if (argc < 3)
		return cmd_usage(cmdtp);

	if (strcmp(argv[1], "fw") == 0) {
		addr = simple_strtoul(argv[2], NULL, 16);

		if (!addr) {
			printf("Invalid address\n");
			return -EINVAL;
		}

		/*
		 * If a length was supplied, compare that with the 'length'
		 * field.
		 */

		if (argc > 3) {
			ulong length = simple_strtoul(argv[3], NULL, 16);
			struct qe_firmware *firmware = (void *) addr;

			if (length != be32_to_cpu(firmware->header.length)) {
				printf("Length mismatch\n");
				return -EINVAL;
			}
		}

		return qe_upload_firmware((const struct qe_firmware *) addr);
	}

	return cmd_usage(cmdtp);
}

U_BOOT_CMD(
	qe, 4, 0, qe_cmd,
	"QUICC Engine commands",
	"fw <addr> [<length>] - Upload firmware binary at address <addr> to "
		"the QE,\n"
	"\twith optional length <length> verification."
);
