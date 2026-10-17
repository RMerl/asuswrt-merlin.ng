// SPDX-License-Identifier: GPL-2.0
/*
 * Driver for Atmel QSPI Controller
 *
 * Copyright (C) 2015 Atmel Corporation
 * Copyright (C) 2018 Cryptera A/S
 *
 * Author: Cyrille Pitchen <cyrille.pitchen@atmel.com>
 * Author: Piotr Bugalski <bugalski.piotr@gmail.com>
 */

#include <malloc.h>
#include <asm/io.h>
#include <clk.h>
#include <common.h>
#include <dm.h>
#include <errno.h>
#include <fdtdec.h>
#include <dm/device_compat.h>
#include <linux/bitfield.h>
#include <linux/bitops.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/iopoll.h>
#include <linux/ioport.h>
#include <mach/clk.h>
#include <spi.h>
#include <spi-mem.h>

/* QSPI register offsets */
#define QSPI_CR      0x0000  /* Control Register */
#define QSPI_MR      0x0004  /* Mode Register */
#define QSPI_RD      0x0008  /* Receive Data Register */
#define QSPI_TD      0x000c  /* Transmit Data Register */
#define QSPI_SR      0x0010  /* Status Register */
#define QSPI_SR2     0x0024  /* SAMA7G5 Status Register */
#define QSPI_IER     0x0014  /* Interrupt Enable Register */
#define QSPI_IDR     0x0018  /* Interrupt Disable Register */
#define QSPI_IMR     0x001c  /* Interrupt Mask Register */
#define QSPI_SCR     0x0020  /* Serial Clock Register */

#define QSPI_IAR     0x0030  /* Instruction Address Register */
#define QSPI_ICR     0x0034  /* Instruction Code Register */
#define QSPI_WICR    0x0034  /* Write Instruction Code Register */
#define QSPI_IFR     0x0038  /* Instruction Frame Register */
#define QSPI_RICR    0x003C  /* Read Instruction Code Register */

#define QSPI_SMR     0x0040  /* Scrambling Mode Register */
#define QSPI_SKR     0x0044  /* Scrambling Key Register */

#define QSPI_REFRESH 0x0050  /* Refresh Register */
#define QSPI_WRACNT  0x0054  /* Write Access Counter Register */
#define QSPI_DLLCFG  0x0058  /* DLL Configuration Register */
#define QSPI_PCALCFG 0x005C  /* Pad Calibration Configuration Register */
#define QSPI_PCALBP  0x0060  /* Pad Calibration Bypass Register */
#define QSPI_TOUT    0x0064  /* Timeout Register */

#define QSPI_WPMR    0x00E4  /* Write Protection Mode Register */
#define QSPI_WPSR    0x00E8  /* Write Protection Status Register */

#define QSPI_VERSION 0x00FC  /* Version Register */

/* Bitfields in QSPI_CR (Control Register) */
#define QSPI_CR_QSPIEN                  BIT(0)
#define QSPI_CR_QSPIDIS                 BIT(1)
#define QSPI_CR_DLLON			BIT(2)
#define QSPI_CR_DLLOFF			BIT(3)
#define QSPI_CR_STPCAL			BIT(4)
#define QSPI_CR_SRFRSH			BIT(5)
#define QSPI_CR_SWRST                   BIT(7)
#define QSPI_CR_UPDCFG			BIT(8)
#define QSPI_CR_STTFR			BIT(9)
#define QSPI_CR_RTOUT			BIT(10)
#define QSPI_CR_LASTXFER                BIT(24)

/* Bitfields in QSPI_MR (Mode Register) */
#define QSPI_MR_SMM                     BIT(0)
#define QSPI_MR_LLB                     BIT(1)
#define QSPI_MR_WDRBT                   BIT(2)
#define QSPI_MR_SMRM                    BIT(3)
#define QSPI_MR_DQSDLYEN		BIT(3)

#define QSPI_MR_CSMODE_MASK             GENMASK(5, 4)
#define QSPI_MR_CSMODE_NOT_RELOADED     (0 << 4)
#define QSPI_MR_CSMODE_LASTXFER         (1 << 4)
#define QSPI_MR_CSMODE_SYSTEMATICALLY   (2 << 4)
#define QSPI_MR_NBBITS_MASK             GENMASK(11, 8)
#define QSPI_MR_NBBITS(n)               ((((n) - 8) << 8) & QSPI_MR_NBBITS_MASK)
#define QSPI_MR_OENSD			BIT(15)
#define QSPI_MR_DLYBCT_MASK             GENMASK(23, 16)
#define QSPI_MR_DLYBCT(n)               (((n) << 16) & QSPI_MR_DLYBCT_MASK)
#define QSPI_MR_DLYCS_MASK              GENMASK(31, 24)
#define QSPI_MR_DLYCS(n)                (((n) << 24) & QSPI_MR_DLYCS_MASK)

/* Bitfields in QSPI_SR/QSPI_IER/QSPI_IDR/QSPI_IMR  */
#define QSPI_SR_RDRF                    BIT(0)
#define QSPI_SR_TDRE                    BIT(1)
#define QSPI_SR_TXEMPTY                 BIT(2)
#define QSPI_SR_OVRES                   BIT(3)
#define QSPI_SR_CSR                     BIT(8)
#define QSPI_SR_CSS                     BIT(9)
#define QSPI_SR_INSTRE                  BIT(10)
#define QSPI_SR_LWRA			BIT(11)
#define QSPI_SR_QITF			BIT(12)
#define QSPI_SR_QITR			BIT(13)
#define QSPI_SR_CSFA			BIT(14)
#define QSPI_SR_CSRA			BIT(15)
#define QSPI_SR_RFRSHD			BIT(16)
#define QSPI_SR_TOUT			BIT(17)
#define QSPI_SR_QSPIENS                 BIT(24)

#define QSPI_SR_CMD_COMPLETED	(QSPI_SR_INSTRE | QSPI_SR_CSR)

/* Bitfields in QSPI_SCR (Serial Clock Register) */
#define QSPI_SCR_CPOL                   BIT(0)
#define QSPI_SCR_CPHA                   BIT(1)
#define QSPI_SCR_SCBR_MASK              GENMASK(15, 8)
#define QSPI_SCR_SCBR(n)                (((n) << 8) & QSPI_SCR_SCBR_MASK)
#define QSPI_SCR_DLYBS_MASK             GENMASK(23, 16)
#define QSPI_SCR_DLYBS(n)               (((n) << 16) & QSPI_SCR_DLYBS_MASK)

/* Bitfields in QSPI_SR2 (SAMA7G5 Status Register) */
#define QSPI_SR2_SYNCBSY		BIT(0)
#define QSPI_SR2_QSPIENS		BIT(1)
#define QSPI_SR2_CSS			BIT(2)
#define QSPI_SR2_RBUSY			BIT(3)
#define QSPI_SR2_HIDLE			BIT(4)
#define QSPI_SR2_DLOCK			BIT(5)
#define QSPI_SR2_CALBSY			BIT(6)

/* Bitfields in QSPI_IAR (Instruction Address Register) */
#define QSPI_IAR_ADDR			GENMASK(31, 0)

/* Bitfields in QSPI_ICR (Read/Write Instruction Code Register) */
#define QSPI_ICR_INST_MASK              GENMASK(7, 0)
#define QSPI_ICR_INST(inst)             (((inst) << 0) & QSPI_ICR_INST_MASK)
#define QSPI_ICR_INST_MASK_SAMA7G5	GENMASK(15, 0)
#define QSPI_ICR_OPT_MASK               GENMASK(23, 16)
#define QSPI_ICR_OPT(opt)               (((opt) << 16) & QSPI_ICR_OPT_MASK)

/* Bitfields in QSPI_IFR (Instruction Frame Register) */
#define QSPI_IFR_WIDTH_MASK             GENMASK(2, 0)
#define QSPI_IFR_WIDTH_SINGLE_BIT_SPI   (0 << 0)
#define QSPI_IFR_WIDTH_DUAL_OUTPUT      (1 << 0)
#define QSPI_IFR_WIDTH_QUAD_OUTPUT      (2 << 0)
#define QSPI_IFR_WIDTH_DUAL_IO          (3 << 0)
#define QSPI_IFR_WIDTH_QUAD_IO          (4 << 0)
#define QSPI_IFR_WIDTH_DUAL_CMD         (5 << 0)
#define QSPI_IFR_WIDTH_QUAD_CMD         (6 << 0)
#define QSPI_IFR_WIDTH_OCT_OUTPUT	(7 << 0)
#define QSPI_IFR_WIDTH_OCT_IO		(8 << 0)
#define QSPI_IFR_WIDTH_OCT_CMD		(9 << 0)
#define QSPI_IFR_INSTEN                 BIT(4)
#define QSPI_IFR_ADDREN                 BIT(5)
#define QSPI_IFR_OPTEN                  BIT(6)
#define QSPI_IFR_DATAEN                 BIT(7)
#define QSPI_IFR_OPTL_MASK              GENMASK(9, 8)
#define QSPI_IFR_OPTL_1BIT              (0 << 8)
#define QSPI_IFR_OPTL_2BIT              (1 << 8)
#define QSPI_IFR_OPTL_4BIT              (2 << 8)
#define QSPI_IFR_OPTL_8BIT              (3 << 8)
#define QSPI_IFR_ADDRL                  BIT(10)
#define QSPI_IFR_ADDRL_SAMA7G5		GENMASK(11, 10)
#define QSPI_IFR_TFRTYP_MEM		BIT(12)
#define QSPI_IFR_SAMA5D2_WRITE_TRSFR	BIT(13)
#define QSPI_IFR_CRM                    BIT(14)
#define QSPI_IFR_DDREN			BIT(15)
#define QSPI_IFR_NBDUM_MASK             GENMASK(20, 16)
#define QSPI_IFR_NBDUM(n)               (((n) << 16) & QSPI_IFR_NBDUM_MASK)
#define QSPI_IFR_END			BIT(22)
#define QSPI_IFR_SMRM			BIT(23)
#define QSPI_IFR_APBTFRTYP_READ		BIT(24)	/* Defined in SAM9X60 */
#define QSPI_IFR_DQSEN			BIT(25)
#define QSPI_IFR_DDRCMDEN		BIT(26)
#define QSPI_IFR_HFWBEN			BIT(27)
#define QSPI_IFR_PROTTYP		GENMASK(29, 28)
#define QSPI_IFR_PROTTYP_STD_SPI	0
#define QSPI_IFR_PROTTYP_TWIN_QUAD	1
#define QSPI_IFR_PROTTYP_OCTAFLASH	2
#define QSPI_IFR_PROTTYP_HYPERFLASH	3

/* Bitfields in QSPI_SMR (Scrambling Mode Register) */
#define QSPI_SMR_SCREN                  BIT(0)
#define QSPI_SMR_RVDIS                  BIT(1)
#define QSPI_SMR_SCRKL			BIT(2)

/* Bitfields in QSPI_REFRESH (Refresh Register) */
#define QSPI_REFRESH_DELAY_COUNTER	GENMASK(31, 0)

/* Bitfields in QSPI_WRACNT (Write Access Counter Register) */
#define QSPI_WRACNT_NBWRA		GENMASK(31, 0)

/* Bitfields in QSPI_DLLCFG (DLL Configuration Register) */
#define QSPI_DLLCFG_RANGE		BIT(0)

/* Bitfields in QSPI_PCALCFG (DLL Pad Calibration Configuration Register) */
#define QSPI_PCALCFG_AAON		BIT(0)
#define QSPI_PCALCFG_DAPCAL		BIT(1)
#define QSPI_PCALCFG_DIFFPM		BIT(2)
#define QSPI_PCALCFG_CLKDIV		GENMASK(6, 4)
#define QSPI_PCALCFG_CALCNT		GENMASK(16, 8)
#define QSPI_PCALCFG_CALP		GENMASK(27, 24)
#define QSPI_PCALCFG_CALN		GENMASK(31, 28)

/* Bitfields in QSPI_PCALBP (DLL Pad Calibration Bypass Register) */
#define QSPI_PCALBP_BPEN		BIT(0)
#define QSPI_PCALBP_CALPBP		GENMASK(11, 8)
#define QSPI_PCALBP_CALNBP		GENMASK(19, 16)

/* Bitfields in QSPI_TOUT (Timeout Register) */
#define QSPI_TOUT_TCNTM			GENMASK(15, 0)

/* Bitfields in QSPI_WPMR (Write Protection Mode Register) */
#define QSPI_WPMR_WPEN                  BIT(0)
#define QSPI_WPMR_WPITEN		BIT(1)
#define QSPI_WPMR_WPCREN		BIT(2)
#define QSPI_WPMR_WPKEY_MASK            GENMASK(31, 8)
#define QSPI_WPMR_WPKEY(wpkey)          (((wpkey) << 8) & QSPI_WPMR_WPKEY_MASK)

/* Bitfields in QSPI_WPSR (Write Protection Status Register) */
#define QSPI_WPSR_WPVS                  BIT(0)
#define QSPI_WPSR_WPVSRC_MASK           GENMASK(15, 8)
#define QSPI_WPSR_WPVSRC(src)           (((src) << 8) & QSPI_WPSR_WPVSRC)

#define ATMEL_QSPI_TIMEOUT		1000000	/* us */
#define ATMEL_QSPI_SYNC_TIMEOUT		300000	/* us */
#define QSPI_DLLCFG_THRESHOLD_FREQ	90000000U
#define QSPI_TOUT_MAX			0xffff

/**
 * struct atmel_qspi_pcal - Pad Calibration Clock Division
 * @pclk_rate: peripheral clock rate.
 * @pclkdiv: calibration clock division. The clock applied to the calibration
 *	     cell is divided by pclkdiv + 1.
 */
struct atmel_qspi_pcal {
	u32 pclk_rate;
	u8 pclk_div;
};

#define ATMEL_QSPI_PCAL_ARRAY_SIZE     8
static const struct atmel_qspi_pcal pcal[ATMEL_QSPI_PCAL_ARRAY_SIZE] = {
	{25000000, 0},
	{50000000, 1},
	{75000000, 2},
	{100000000, 3},
	{125000000, 4},
	{150000000, 5},
	{175000000, 6},
	{200000000, 7},
};

struct atmel_qspi_caps {
	bool has_qspick;
	bool has_gclk;
	bool has_ricr;
	bool octal;
};

struct atmel_qspi_priv_ops;

struct atmel_qspi {
	void __iomem *regs;
	void __iomem *mem;
	resource_size_t mmap_size;
	const struct atmel_qspi_caps *caps;
	const struct atmel_qspi_priv_ops *ops;
	struct udevice *dev;
	ulong bus_clk_rate;
	u32 mr;
};

struct atmel_qspi_priv_ops {
	int (*set_cfg)(struct atmel_qspi *aq, const struct spi_mem_op *op,
		       u32 *offset);
	int (*transfer)(struct atmel_qspi *aq, const struct spi_mem_op *op,
			u32 offset);
};

struct atmel_qspi_mode {
	u8 cmd_buswidth;
	u8 addr_buswidth;
	u8 data_buswidth;
	u32 config;
};

static const struct atmel_qspi_mode atmel_qspi_modes[] = {
	{ 1, 1, 1, QSPI_IFR_WIDTH_SINGLE_BIT_SPI },
	{ 1, 1, 2, QSPI_IFR_WIDTH_DUAL_OUTPUT },
	{ 1, 1, 4, QSPI_IFR_WIDTH_QUAD_OUTPUT },
	{ 1, 2, 2, QSPI_IFR_WIDTH_DUAL_IO },
	{ 1, 4, 4, QSPI_IFR_WIDTH_QUAD_IO },
	{ 2, 2, 2, QSPI_IFR_WIDTH_DUAL_CMD },
	{ 4, 4, 4, QSPI_IFR_WIDTH_QUAD_CMD },
};

static const struct atmel_qspi_mode atmel_qspi_sama7g5_modes[] = {
	{ 1, 1, 1, QSPI_IFR_WIDTH_SINGLE_BIT_SPI },
	{ 1, 1, 2, QSPI_IFR_WIDTH_DUAL_OUTPUT },
	{ 1, 1, 4, QSPI_IFR_WIDTH_QUAD_OUTPUT },
	{ 1, 2, 2, QSPI_IFR_WIDTH_DUAL_IO },
	{ 1, 4, 4, QSPI_IFR_WIDTH_QUAD_IO },
	{ 2, 2, 2, QSPI_IFR_WIDTH_DUAL_CMD },
	{ 4, 4, 4, QSPI_IFR_WIDTH_QUAD_CMD },
	{ 1, 1, 8, QSPI_IFR_WIDTH_OCT_OUTPUT },
	{ 1, 8, 8, QSPI_IFR_WIDTH_OCT_IO },
	{ 8, 8, 8, QSPI_IFR_WIDTH_OCT_CMD },
};

#ifdef VERBOSE_DEBUG
static const char *atmel_qspi_reg_name(u32 offset, char *tmp, size_t sz)
{
	switch (offset) {
	case QSPI_CR:
		return "CR";
	case QSPI_MR:
		return "MR";
	case QSPI_RD:
		return "RD";
	case QSPI_TD:
		return "TD";
	case QSPI_SR:
		return "SR";
	case QSPI_IER:
		return "IER";
	case QSPI_IDR:
		return "IDR";
	case QSPI_IMR:
		return "IMR";
	case QSPI_SCR:
		return "SCR";
	case QSPI_SR2:
		return "SR2";
	case QSPI_IAR:
		return "IAR";
	case QSPI_ICR:
		return "ICR/WICR";
	case QSPI_IFR:
		return "IFR";
	case QSPI_RICR:
		return "RICR";
	case QSPI_SMR:
		return "SMR";
	case QSPI_SKR:
		return "SKR";
	case QSPI_REFRESH:
		return "REFRESH";
	case QSPI_WRACNT:
		return "WRACNT";
	case QSPI_DLLCFG:
		return "DLLCFG";
	case QSPI_PCALCFG:
		return "PCALCFG";
	case QSPI_PCALBP:
		return "PCALBP";
	case QSPI_TOUT:
		return "TOUT";
	case QSPI_WPMR:
		return "WPMR";
	case QSPI_WPSR:
		return "WPSR";
	case QSPI_VERSION:
		return "VERSION";
	default:
		snprintf(tmp, sz, "0x%02x", offset);
		break;
	}

	return tmp;
}
#endif /* VERBOSE_DEBUG */

static u32 atmel_qspi_read(struct atmel_qspi *aq, u32 offset)
{
	u32 value = readl(aq->regs + offset);

#ifdef VERBOSE_DEBUG
	char tmp[16];

	dev_vdbg(aq->dev, "read 0x%08x from %s\n", value,
		 atmel_qspi_reg_name(offset, tmp, sizeof(tmp)));
#endif /* VERBOSE_DEBUG */

	return value;
}

static void atmel_qspi_write(u32 value, struct atmel_qspi *aq, u32 offset)
{
#ifdef VERBOSE_DEBUG
	char tmp[16];

	dev_vdbg(aq->dev, "write 0x%08x into %s\n", value,
		 atmel_qspi_reg_name(offset, tmp, sizeof(tmp)));
#endif /* VERBOSE_DEBUG */

	writel(value, aq->regs + offset);
}

static inline bool atmel_qspi_is_compatible(const struct spi_mem_op *op,
					    const struct atmel_qspi_mode *mode)
{
	if (op->cmd.buswidth != mode->cmd_buswidth)
		return false;

	if (op->addr.nbytes && op->addr.buswidth != mode->addr_buswidth)
		return false;

	if (op->data.nbytes && op->data.buswidth != mode->data_buswidth)
		return false;

	return true;
}

static int atmel_qspi_find_mode(const struct spi_mem_op *op)
{
	u32 i;

	for (i = 0; i < ARRAY_SIZE(atmel_qspi_modes); i++)
		if (atmel_qspi_is_compatible(op, &atmel_qspi_modes[i]))
			return i;

	return -ENOTSUPP;
}

static int atmel_qspi_sama7g5_find_mode(const struct spi_mem_op *op)
{
	u32 i;

	for (i = 0; i < ARRAY_SIZE(atmel_qspi_sama7g5_modes); i++)
		if (atmel_qspi_is_compatible(op, &atmel_qspi_sama7g5_modes[i]))
			return i;

	return -EOPNOTSUPP;
}

static bool atmel_qspi_supports_op(struct spi_slave *slave,
				   const struct spi_mem_op *op)
{
	struct atmel_qspi *aq = dev_get_priv(slave->dev->parent);

	if (!spi_mem_default_supports_op(slave, op))
		return false;

	if (aq->caps->octal) {
		if (atmel_qspi_sama7g5_find_mode(op) < 0)
			return false;
		else
			return true;
	}

	if (atmel_qspi_find_mode(op) < 0)
		return false;

	/* special case not supported by hardware */
	if (op->addr.nbytes == 2 && op->cmd.buswidth != op->addr.buswidth &&
	    op->dummy.nbytes == 0)
		return false;

	return true;
}

static int atmel_qspi_set_cfg(struct atmel_qspi *aq,
			      const struct spi_mem_op *op, u32 *offset)
{
	u32 iar, icr, ifr;
	u32 dummy_cycles = 0;
	int mode;

	iar = 0;
	icr = QSPI_ICR_INST(op->cmd.opcode);
	ifr = QSPI_IFR_INSTEN;

	mode = atmel_qspi_find_mode(op);
	if (mode < 0)
		return mode;
	ifr |= atmel_qspi_modes[mode].config;

	if (op->dummy.buswidth && op->dummy.nbytes)
		dummy_cycles = op->dummy.nbytes * 8 / op->dummy.buswidth;

	/*
	 * The controller allows 24 and 32-bit addressing while NAND-flash
	 * requires 16-bit long. Handling 8-bit long addresses is done using
	 * the option field. For the 16-bit addresses, the workaround depends
	 * of the number of requested dummy bits. If there are 8 or more dummy
	 * cycles, the address is shifted and sent with the first dummy byte.
	 * Otherwise opcode is disabled and the first byte of the address
	 * contains the command opcode (works only if the opcode and address
	 * use the same buswidth). The limitation is when the 16-bit address is
	 * used without enough dummy cycles and the opcode is using a different
	 * buswidth than the address.
	 */
	if (op->addr.buswidth) {
		switch (op->addr.nbytes) {
		case 0:
			break;
		case 1:
			ifr |= QSPI_IFR_OPTEN | QSPI_IFR_OPTL_8BIT;
			icr |= QSPI_ICR_OPT(op->addr.val & 0xff);
			break;
		case 2:
			if (dummy_cycles < 8 / op->addr.buswidth) {
				ifr &= ~QSPI_IFR_INSTEN;
				ifr |= QSPI_IFR_ADDREN;
				iar = (op->cmd.opcode << 16) |
					(op->addr.val & 0xffff);
			} else {
				ifr |= QSPI_IFR_ADDREN;
				iar = (op->addr.val << 8) & 0xffffff;
				dummy_cycles -= 8 / op->addr.buswidth;
			}
			break;
		case 3:
			ifr |= QSPI_IFR_ADDREN;
			iar = op->addr.val & 0xffffff;
			break;
		case 4:
			ifr |= QSPI_IFR_ADDREN | QSPI_IFR_ADDRL;
			iar = op->addr.val & 0x7ffffff;
			break;
		default:
			return -ENOTSUPP;
		}
	}

	/* offset of the data access in the QSPI memory space */
	*offset = iar;

	/* Set number of dummy cycles */
	if (dummy_cycles)
		ifr |= QSPI_IFR_NBDUM(dummy_cycles);

	/* Set data enable */
	if (op->data.nbytes)
		ifr |= QSPI_IFR_DATAEN;

	/*
	 * If the QSPI controller is set in regular SPI mode, set it in
	 * Serial Memory Mode (SMM).
	 */
	if (aq->mr != QSPI_MR_SMM) {
		atmel_qspi_write(QSPI_MR_SMM, aq, QSPI_MR);
		aq->mr = QSPI_MR_SMM;
	}

	/* Clear pending interrupts */
	(void)atmel_qspi_read(aq, QSPI_SR);

	if (aq->caps->has_ricr) {
		if (!op->addr.nbytes && op->data.dir == SPI_MEM_DATA_IN)
			ifr |= QSPI_IFR_APBTFRTYP_READ;

		/* Set QSPI Instruction Frame registers */
		atmel_qspi_write(iar, aq, QSPI_IAR);
		if (op->data.dir == SPI_MEM_DATA_IN)
			atmel_qspi_write(icr, aq, QSPI_RICR);
		else
			atmel_qspi_write(icr, aq, QSPI_WICR);
		atmel_qspi_write(ifr, aq, QSPI_IFR);
	} else {
		if (op->data.dir == SPI_MEM_DATA_OUT)
			ifr |= QSPI_IFR_SAMA5D2_WRITE_TRSFR;

		/* Set QSPI Instruction Frame registers */
		atmel_qspi_write(iar, aq, QSPI_IAR);
		atmel_qspi_write(icr, aq, QSPI_ICR);
		atmel_qspi_write(ifr, aq, QSPI_IFR);
	}

	return 0;
}

static int atmel_qspi_transfer(struct atmel_qspi *aq,
			       const struct spi_mem_op *op, u32 offset)
{
	u32 sr, imr;

	/* Skip to the final steps if there is no data */
	if (op->data.nbytes) {
		/* Dummy read of QSPI_IFR to synchronize APB and AHB accesses */
		(void)atmel_qspi_read(aq, QSPI_IFR);

		/* Send/Receive data */
		if (op->data.dir == SPI_MEM_DATA_IN)
			memcpy_fromio(op->data.buf.in, aq->mem + offset,
				      op->data.nbytes);
		else
			memcpy_toio(aq->mem + offset, op->data.buf.out,
				    op->data.nbytes);

		/* Release the chip-select */
		atmel_qspi_write(QSPI_CR_LASTXFER, aq, QSPI_CR);
	}

	/* Poll INSTruction End and Chip Select Rise flags. */
	imr = QSPI_SR_INSTRE | QSPI_SR_CSR;
	return readl_poll_timeout(aq->regs + QSPI_SR, sr, (sr & imr) == imr,
				  ATMEL_QSPI_TIMEOUT);
}

static int atmel_qspi_reg_sync(struct atmel_qspi *aq)
{
	u32 val;

	return readl_poll_timeout(aq->regs + QSPI_SR2, val,
				  !(val & QSPI_SR2_SYNCBSY),
				  ATMEL_QSPI_SYNC_TIMEOUT);
}

static int atmel_qspi_update_config(struct atmel_qspi *aq)
{
	int ret;

	ret = atmel_qspi_reg_sync(aq);
	if (ret)
		return ret;
	atmel_qspi_write(QSPI_CR_UPDCFG, aq, QSPI_CR);
	return atmel_qspi_reg_sync(aq);
}

static int atmel_qspi_sama7g5_set_cfg(struct atmel_qspi *aq,
				      const struct spi_mem_op *op, u32 *offset)
{
	u32 iar, icr, ifr;
	int mode, ret;

	iar = 0;
	icr = FIELD_PREP(QSPI_ICR_INST_MASK_SAMA7G5, op->cmd.opcode);
	ifr = QSPI_IFR_INSTEN;

	mode = atmel_qspi_sama7g5_find_mode(op);
	if (mode < 0)
		return mode;
	ifr |= atmel_qspi_sama7g5_modes[mode].config;

	if (op->dummy.buswidth && op->dummy.nbytes) {
		if (op->addr.dtr && op->dummy.dtr && op->data.dtr)
			ifr |= QSPI_IFR_NBDUM(op->dummy.nbytes * 8 /
					      (2 * op->dummy.buswidth));
		else
			ifr |= QSPI_IFR_NBDUM(op->dummy.nbytes * 8 /
					      op->dummy.buswidth);
	}

	if (op->addr.buswidth && op->addr.nbytes) {
		ifr |= FIELD_PREP(QSPI_IFR_ADDRL_SAMA7G5, op->addr.nbytes - 1) |
		       QSPI_IFR_ADDREN;
		iar = FIELD_PREP(QSPI_IAR_ADDR, op->addr.val);
	}

	if (op->addr.dtr && op->dummy.dtr && op->data.dtr) {
		ifr |= QSPI_IFR_DDREN;
		if (op->cmd.dtr)
			ifr |= QSPI_IFR_DDRCMDEN;
		ifr |= QSPI_IFR_DQSEN;
	}

	if (op->cmd.buswidth == 8 || op->addr.buswidth == 8 ||
	    op->data.buswidth == 8)
		ifr |= FIELD_PREP(QSPI_IFR_PROTTYP, QSPI_IFR_PROTTYP_OCTAFLASH);

	/* offset of the data access in the QSPI memory space */
	*offset = iar;

	/* Set data enable */
	if (op->data.nbytes) {
		ifr |= QSPI_IFR_DATAEN;
		if (op->addr.nbytes)
			ifr |= QSPI_IFR_TFRTYP_MEM;
	}

	/*
	 * If the QSPI controller is set in regular SPI mode, set it in
	 * Serial Memory Mode (SMM).
	 */
	if (aq->mr != QSPI_MR_SMM) {
		atmel_qspi_write(QSPI_MR_SMM | QSPI_MR_DQSDLYEN, aq, QSPI_MR);
		ret = atmel_qspi_update_config(aq);
		if (ret)
			return ret;
		aq->mr = QSPI_MR_SMM;
	}

	/* Clear pending interrupts */
	(void)atmel_qspi_read(aq, QSPI_SR);

	/* Set QSPI Instruction Frame registers */
	if (op->addr.nbytes && !op->data.nbytes)
		atmel_qspi_write(iar, aq, QSPI_IAR);

	if (op->data.dir == SPI_MEM_DATA_IN) {
		atmel_qspi_write(icr, aq, QSPI_RICR);
	} else {
		atmel_qspi_write(icr, aq, QSPI_WICR);
		if (op->data.nbytes)
			atmel_qspi_write(FIELD_PREP(QSPI_WRACNT_NBWRA,
						    op->data.nbytes),
					 aq, QSPI_WRACNT);
	}

	atmel_qspi_write(ifr, aq, QSPI_IFR);

	return atmel_qspi_update_config(aq);
}

static int atmel_qspi_sama7g5_transfer(struct atmel_qspi *aq,
				       const struct spi_mem_op *op, u32 offset)
{
	int err;
	u32 val;

	if (!op->data.nbytes) {
		/* Start the transfer. */
		err = atmel_qspi_reg_sync(aq);
		if (err)
			return err;
		atmel_qspi_write(QSPI_CR_STTFR, aq, QSPI_CR);

		return readl_poll_timeout(aq->regs + QSPI_SR, val,
					  val & QSPI_SR_CSRA,
					  ATMEL_QSPI_TIMEOUT);
	}

	/* Send/Receive data. */
	if (op->data.dir == SPI_MEM_DATA_IN) {
		memcpy_fromio(op->data.buf.in, aq->mem + offset,
			      op->data.nbytes);

		if (op->addr.nbytes) {
			err = readl_poll_timeout(aq->regs + QSPI_SR2, val,
						 !(val & QSPI_SR2_RBUSY),
						 ATMEL_QSPI_SYNC_TIMEOUT);
			if (err)
				return err;
		}
	} else {
		memcpy_toio(aq->mem + offset, op->data.buf.out,
			    op->data.nbytes);

		err = readl_poll_timeout(aq->regs + QSPI_SR, val,
					 val & QSPI_SR_LWRA,
					 ATMEL_QSPI_TIMEOUT);
		if (err)
			return err;
	}

	/* Release the chip-select. */
	err = atmel_qspi_reg_sync(aq);
	if (err)
		return err;
	atmel_qspi_write(QSPI_CR_LASTXFER, aq, QSPI_CR);

	return readl_poll_timeout(aq->regs + QSPI_SR, val, val & QSPI_SR_CSRA,
				  ATMEL_QSPI_TIMEOUT);
}

static int atmel_qspi_exec_op(struct spi_slave *slave,
			      const struct spi_mem_op *op)
{
	struct atmel_qspi *aq = dev_get_priv(slave->dev->parent);
	u32 offset;
	int err;

	/*
	 * Check if the address exceeds the MMIO window size. An improvement
	 * would be to add support for regular SPI mode and fall back to it
	 * when the flash memories overrun the controller's memory space.
	 */
	if (op->addr.val + op->data.nbytes > aq->mmap_size)
		return -ENOTSUPP;

	if (op->addr.nbytes > 4)
		return -EOPNOTSUPP;

	err = aq->ops->set_cfg(aq, op, &offset);
	if (err)
		return err;

	return aq->ops->transfer(aq, op, offset);
}

static int atmel_qspi_set_pad_calibration(struct udevice *bus, uint hz)
{
	struct atmel_qspi *aq = dev_get_priv(bus);
	u32 status, val;
	int i, ret;
	u8 pclk_div = 0;

	for (i = 0; i < ATMEL_QSPI_PCAL_ARRAY_SIZE; i++) {
		if (aq->bus_clk_rate <= pcal[i].pclk_rate) {
			pclk_div = pcal[i].pclk_div;
			break;
		}
	}

	/*
	 * Use the biggest divider in case the peripheral clock exceeds
	 * 200MHZ.
	 */
	if (aq->bus_clk_rate > pcal[ATMEL_QSPI_PCAL_ARRAY_SIZE - 1].pclk_rate)
		pclk_div = pcal[ATMEL_QSPI_PCAL_ARRAY_SIZE - 1].pclk_div;

	/* Disable QSPI while configuring the pad calibration. */
	status = atmel_qspi_read(aq, QSPI_SR2);
	if (status & QSPI_SR2_QSPIENS) {
		ret = atmel_qspi_reg_sync(aq);
		if (ret)
			return ret;
		atmel_qspi_write(QSPI_CR_QSPIDIS, aq, QSPI_CR);
	}

	/*
	 * The analog circuitry is not shut down at the end of the calibration
	 * and the start-up time is only required for the first calibration
	 * sequence, thus increasing performance. Set the delay between the Pad
	 * calibration analog circuitry and the calibration request to 2us.
	 */
	atmel_qspi_write(QSPI_PCALCFG_AAON |
			 FIELD_PREP(QSPI_PCALCFG_CLKDIV, pclk_div) |
			 FIELD_PREP(QSPI_PCALCFG_CALCNT,
				    2 * (aq->bus_clk_rate / 1000000)),
			 aq, QSPI_PCALCFG);

	/* DLL On + start calibration. */
	atmel_qspi_write(QSPI_CR_DLLON | QSPI_CR_STPCAL, aq, QSPI_CR);
	ret =  readl_poll_timeout(aq->regs + QSPI_SR2, val,
				  (val & QSPI_SR2_DLOCK) &&
				  !(val & QSPI_SR2_CALBSY),
				  ATMEL_QSPI_TIMEOUT);

	/* Refresh analogic blocks every 1 ms.*/
	atmel_qspi_write(FIELD_PREP(QSPI_REFRESH_DELAY_COUNTER, hz / 1000),
			 aq, QSPI_REFRESH);

	return ret;
}

static int atmel_qspi_set_gclk(struct udevice *bus, uint hz)
{
	struct atmel_qspi *aq = dev_get_priv(bus);
	struct clk gclk;
	u32 status, val;
	int ret;

	/* Disable DLL before setting GCLK */
	status = atmel_qspi_read(aq, QSPI_SR2);
	if (status & QSPI_SR2_DLOCK) {
		atmel_qspi_write(QSPI_CR_DLLOFF, aq, QSPI_CR);
		ret = readl_poll_timeout(aq->regs + QSPI_SR2, val,
					 !(val & QSPI_SR2_DLOCK),
					 ATMEL_QSPI_TIMEOUT);
		if (ret)
			return ret;
	}

	if (hz > QSPI_DLLCFG_THRESHOLD_FREQ)
		atmel_qspi_write(QSPI_DLLCFG_RANGE, aq, QSPI_DLLCFG);
	else
		atmel_qspi_write(0, aq, QSPI_DLLCFG);

	ret = clk_get_by_name(bus, "gclk", &gclk);
	if (ret) {
		dev_err(bus, "Missing QSPI generic clock\n");
		return ret;
	}

	ret = clk_disable(&gclk);
	if (ret)
		dev_err(bus, "Failed to disable QSPI generic clock\n");

	ret = clk_set_rate(&gclk, hz);
	if (ret < 0) {
		dev_err(bus, "Failed to set generic clock rate.\n");
		return ret;
	}

	ret = clk_enable(&gclk);
	if (ret)
		dev_err(bus, "Failed to enable QSPI generic clock\n");

	return ret;
}

static int atmel_qspi_sama7g5_set_speed(struct udevice *bus, uint hz)
{
	struct atmel_qspi *aq = dev_get_priv(bus);
	u32 val;
	int ret;

	ret = atmel_qspi_set_gclk(bus, hz);
	if (ret)
		return ret;

	if (aq->caps->octal) {
		ret = atmel_qspi_set_pad_calibration(bus, hz);
		if (ret)
			return ret;
	} else {
		atmel_qspi_write(QSPI_CR_DLLON, aq, QSPI_CR);
		ret =  readl_poll_timeout(aq->regs + QSPI_SR2, val,
					  val & QSPI_SR2_DLOCK,
					  ATMEL_QSPI_TIMEOUT);
	}

	/* Set the QSPI controller by default in Serial Memory Mode */
	atmel_qspi_write(QSPI_MR_SMM | QSPI_MR_DQSDLYEN, aq, QSPI_MR);
	ret = atmel_qspi_update_config(aq);
	if (ret)
		return ret;
	aq->mr = QSPI_MR_SMM;

	/* Enable the QSPI controller. */
	ret = atmel_qspi_reg_sync(aq);
	if (ret)
		return ret;
	atmel_qspi_write(QSPI_CR_QSPIEN, aq, QSPI_CR);
	ret = readl_poll_timeout(aq->regs + QSPI_SR2, val,
				 val & QSPI_SR2_QSPIENS,
				 ATMEL_QSPI_SYNC_TIMEOUT);
	if (ret)
		return ret;

	if (aq->caps->octal)
		ret = readl_poll_timeout(aq->regs + QSPI_SR, val,
					 val & QSPI_SR_RFRSHD,
					 ATMEL_QSPI_TIMEOUT);

	atmel_qspi_write(FIELD_PREP(QSPI_TOUT_TCNTM, QSPI_TOUT_MAX),
			 aq, QSPI_TOUT);

	return ret;
}

static int atmel_qspi_set_speed(struct udevice *bus, uint hz)
{
	struct atmel_qspi *aq = dev_get_priv(bus);
	u32 scr, scbr, mask, new_value;

	if (aq->caps->has_gclk)
		return atmel_qspi_sama7g5_set_speed(bus, hz);

	/* Compute the QSPI baudrate */
	scbr = DIV_ROUND_UP(aq->bus_clk_rate, hz);
	if (scbr > 0)
		scbr--;

	new_value = QSPI_SCR_SCBR(scbr);
	mask = QSPI_SCR_SCBR_MASK;

	scr = atmel_qspi_read(aq, QSPI_SCR);
	if ((scr & mask) == new_value)
		return 0;

	scr = (scr & ~mask) | new_value;
	atmel_qspi_write(scr, aq, QSPI_SCR);

	return 0;
}

static int atmel_qspi_set_mode(struct udevice *bus, uint mode)
{
	struct atmel_qspi *aq = dev_get_priv(bus);
	u32 scr, mask, new_value = 0;

	if (mode & SPI_CPOL)
		new_value = QSPI_SCR_CPOL;
	if (mode & SPI_CPHA)
		new_value = QSPI_SCR_CPHA;

	mask = QSPI_SCR_CPOL | QSPI_SCR_CPHA;

	scr = atmel_qspi_read(aq, QSPI_SCR);
	if ((scr & mask) == new_value)
		return 0;

	scr = (scr & ~mask) | new_value;
	atmel_qspi_write(scr, aq, QSPI_SCR);
	if (aq->caps->has_gclk)
		return atmel_qspi_update_config(aq);

	return 0;
}

static int atmel_qspi_enable_clk(struct udevice *dev)
{
	struct atmel_qspi *aq = dev_get_priv(dev);
	struct clk pclk, qspick, gclk;
	int ret;

	ret = clk_get_by_name(dev, "pclk", &pclk);
	if (ret)
		ret = clk_get_by_index(dev, 0, &pclk);

	if (ret) {
		dev_err(dev, "Missing QSPI peripheral clock\n");
		return ret;
	}

	ret = clk_enable(&pclk);
	if (ret) {
		dev_err(dev, "Failed to enable QSPI peripheral clock\n");
		return ret;
	}

	if (aq->caps->has_qspick) {
		/* Get the QSPI system clock */
		ret = clk_get_by_name(dev, "qspick", &qspick);
		if (ret) {
			dev_err(dev, "Missing QSPI peripheral clock\n");
			return ret;
		}

		ret = clk_enable(&qspick);
		if (ret)
			dev_err(dev, "Failed to enable QSPI system clock\n");
	} else if (aq->caps->has_gclk) {
		ret = clk_get_by_name(dev, "gclk", &gclk);
		if (ret) {
			dev_err(dev, "Missing QSPI generic clock\n");
			return ret;
		}

		ret = clk_enable(&gclk);
		if (ret)
			dev_err(dev, "Failed to enable QSPI system clock\n");
	}

	aq->bus_clk_rate = clk_get_rate(&pclk);
	if (!aq->bus_clk_rate)
		return -EINVAL;

	return ret;
}

static int atmel_qspi_init(struct atmel_qspi *aq)
{
	int ret;

	if (aq->caps->has_gclk) {
		ret = atmel_qspi_reg_sync(aq);
		if (ret)
			return ret;
		atmel_qspi_write(QSPI_CR_SWRST, aq, QSPI_CR);
		return 0;
	}

	/* Reset the QSPI controller */
	atmel_qspi_write(QSPI_CR_SWRST, aq, QSPI_CR);

	/* Set the QSPI controller by default in Serial Memory Mode */
	atmel_qspi_write(QSPI_MR_SMM, aq, QSPI_MR);
	aq->mr = QSPI_MR_SMM;

	/* Enable the QSPI controller */
	atmel_qspi_write(QSPI_CR_QSPIEN, aq, QSPI_CR);

	return 0;
}

static const struct atmel_qspi_priv_ops atmel_qspi_priv_ops = {
	.set_cfg = atmel_qspi_set_cfg,
	.transfer = atmel_qspi_transfer,
};

static const struct atmel_qspi_priv_ops atmel_qspi_sama7g5_priv_ops = {
	.set_cfg = atmel_qspi_sama7g5_set_cfg,
	.transfer = atmel_qspi_sama7g5_transfer,
};

static int atmel_qspi_probe(struct udevice *dev)
{
	struct atmel_qspi *aq = dev_get_priv(dev);
	struct resource res;
	int ret;

	aq->caps = (struct atmel_qspi_caps *)dev_get_driver_data(dev);
	if (!aq->caps) {
		dev_err(dev, "Could not retrieve QSPI caps\n");
		return -EINVAL;
	};

	if (aq->caps->has_gclk)
		aq->ops = &atmel_qspi_sama7g5_priv_ops;
	else
		aq->ops = &atmel_qspi_priv_ops;

	/* Map the registers */
	ret = dev_read_resource_byname(dev, "qspi_base", &res);
	if (ret) {
		dev_err(dev, "missing registers\n");
		return ret;
	}

	aq->regs = devm_ioremap(dev, res.start, resource_size(&res));
	if (IS_ERR(aq->regs))
		return PTR_ERR(aq->regs);

	/* Map the AHB memory */
	ret = dev_read_resource_byname(dev, "qspi_mmap", &res);
	if (ret) {
		dev_err(dev, "missing AHB memory\n");
		return ret;
	}

	aq->mem = devm_ioremap(dev, res.start, resource_size(&res));
	if (IS_ERR(aq->mem))
		return PTR_ERR(aq->mem);

	aq->mmap_size = resource_size(&res);

	ret = atmel_qspi_enable_clk(dev);
	if (ret)
		return ret;

	aq->dev = dev;
	return atmel_qspi_init(aq);
}

static const struct spi_controller_mem_ops atmel_qspi_mem_ops = {
	.supports_op = atmel_qspi_supports_op,
	.exec_op = atmel_qspi_exec_op,
};

static const struct dm_spi_ops atmel_qspi_ops = {
	.set_speed = atmel_qspi_set_speed,
	.set_mode = atmel_qspi_set_mode,
	.mem_ops = &atmel_qspi_mem_ops,
};

static const struct atmel_qspi_caps atmel_sama5d2_qspi_caps = {};

static const struct atmel_qspi_caps atmel_sam9x60_qspi_caps = {
	.has_qspick = true,
	.has_ricr = true,
};

static const struct atmel_qspi_caps atmel_sama7g5_ospi_caps = {
	.has_gclk = true,
	.octal = true,
};

static const struct atmel_qspi_caps atmel_sama7g5_qspi_caps = {
	.has_gclk = true,
};

static const struct udevice_id atmel_qspi_ids[] = {
	{
		.compatible = "atmel,sama5d2-qspi",
		.data = (ulong)&atmel_sama5d2_qspi_caps,
	},
	{
		.compatible = "microchip,sam9x60-qspi",
		.data = (ulong)&atmel_sam9x60_qspi_caps,
	},
	{
		.compatible = "microchip,sama7g5-ospi",
		.data = (ulong)&atmel_sama7g5_ospi_caps,
	},
	{
		.compatible = "microchip,sama7g5-qspi",
		.data = (ulong)&atmel_sama7g5_qspi_caps,
	},
	{ /* sentinel */ }
};

U_BOOT_DRIVER(atmel_qspi) = {
	.name           = "atmel_qspi",
	.id             = UCLASS_SPI,
	.of_match       = atmel_qspi_ids,
	.ops            = &atmel_qspi_ops,
	.priv_auto	= sizeof(struct atmel_qspi),
	.probe          = atmel_qspi_probe,
};
