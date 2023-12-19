/* Code to dump registers for the Freescale/NXP ENETC controller.
 *
 * Copyright 2022 NXP
 */
#include <stdio.h>
#include "internal.h"

#define BIT(x)			(1U << (x))

enum enetc_bdr_type {TX, RX};
#define ENETC_SIMR		0
#define ENETC_SIPMAR0		0x80
#define ENETC_SIPMAR1		0x84
#define ENETC_SICBDRMR		0x800
#define ENETC_SICBDRSR		0x804
#define ENETC_SICBDRBAR0	0x810
#define ENETC_SICBDRBAR1	0x814
#define ENETC_SICBDRPIR		0x818
#define ENETC_SICBDRCIR		0x81c
#define ENETC_SICBDRLENR	0x820
#define ENETC_SICAPR0		0x900
#define ENETC_SICAPR1		0x904
#define ENETC_SIUEFDCR		0xe28

#define ENETC_BDR_OFF(i)	((i) * 0x200)
#define ENETC_BDR(t, i, r)	(0x8000 + (t) * 0x100 + ENETC_BDR_OFF(i) + (r))

/* RX BDR reg offsets */
#define ENETC_RBMR		0
#define ENETC_RBSR		0x4
#define ENETC_RBBSR		0x8
#define ENETC_RBCIR		0xc
#define ENETC_RBBAR0		0x10
#define ENETC_RBBAR1		0x14
#define ENETC_RBPIR		0x18
#define ENETC_RBLENR		0x20
#define ENETC_RBIER		0xa0
#define ENETC_RBICR0		0xa8
#define ENETC_RBICR1		0xac

/* TX BDR reg offsets */
#define ENETC_TBMR		0
#define ENETC_TBSR		0x4
#define ENETC_TBBAR0		0x10
#define ENETC_TBBAR1		0x14
#define ENETC_TBPIR		0x18
#define ENETC_TBCIR		0x1c
#define ENETC_TBLENR		0x20
#define ENETC_TBIER		0xa0
#define ENETC_TBIDR		0xa4
#define ENETC_TBICR0		0xa8
#define ENETC_TBICR1		0xac

/* Port registers */
#define ENETC_PORT_BASE		0x10000
#define ENETC_PMR		ENETC_PORT_BASE + 0x0000
#define ENETC_PSR		ENETC_PORT_BASE + 0x0004
#define ENETC_PSIPMR		ENETC_PORT_BASE + 0x0018
#define ENETC_PSIPMAR0(n)	ENETC_PORT_BASE + (0x0100 + (n) * 0x8) /* n = SI index */
#define ENETC_PSIPMAR1(n)	ENETC_PORT_BASE + (0x0104 + (n) * 0x8)
#define ENETC_PTXMBAR		ENETC_PORT_BASE + 0x0608
#define ENETC_PCAPR0		ENETC_PORT_BASE + 0x0900
#define ENETC_PCAPR1		ENETC_PORT_BASE + 0x0904
#define ENETC_PSICFGR0(n)	ENETC_PORT_BASE + (0x0940 + (n) * 0xc)  /* n = SI index */

#define ENETC_PRFSCAPR		ENETC_PORT_BASE + 0x1804
#define ENETC_PTCMSDUR(n)	ENETC_PORT_BASE + (0x2020 + (n) * 4) /* n = TC index [0..7] */

#define ENETC_PM0_CMD_CFG	ENETC_PORT_BASE + 0x8008
#define ENETC_PM0_CMD_TX_EN		BIT(0)
#define ENETC_PM0_CMD_RX_EN		BIT(1)
#define ENETC_PM0_CMD_WAN		BIT(3)
#define ENETC_PM0_CMD_PROMISC		BIT(4)
#define ENETC_PM0_CMD_PAD		BIT(5)
#define ENETC_PM0_CMD_CRC		BIT(6)
#define ENETC_PM0_CMD_PAUSE_FWD		BIT(7)
#define ENETC_PM0_CMD_PAUSE_IGN		BIT(8)
#define ENETC_PM0_CMD_TX_ADDR_INS	BIT(9)
#define ENETC_PM0_CMD_XGLP		BIT(10)
#define ENETC_PM0_CMD_TXP		BIT(11)
#define ENETC_PM0_CMD_SWR		BIT(12)
#define ENETC_PM0_CMD_CNT_FRM_EN	BIT(13)
#define ENETC_PM0_CMD_SEND_IDLE		BIT(16)
#define ENETC_PM0_CMD_NO_LEN_CHK	BIT(17)
#define ENETC_PM0_CMD_SFD		BIT(21)
#define ENETC_PM0_CMD_TX_LOWP_ENA	BIT(23)
#define ENETC_PM0_CMD_REG_LOWP_RXETY	BIT(24)
#define ENETC_PM0_CMD_RXSTP		BIT(29)
#define ENETC_PM0_CMD_MG		BIT(31)

#define ENETC_PM0_MAXFRM	ENETC_PORT_BASE + 0x8014
#define ENETC_PM0_IF_MODE	ENETC_PORT_BASE + 0x8300

struct enetc_register {
	u32 addr;
	const char *name;
	void (*decode)(u32 val, char *buf);
};

#define REG(_reg, _name)	{ .addr = (_reg), .name = (_name) }

#define REG_DEC(_reg, _name, _decode) \
	{ .addr = (_reg), .name = (_name), .decode = (_decode) }

static void decode_cmd_cfg(u32 val, char *buf)
{
	sprintf(buf, "\tMG %d\n\tRXSTP %d\n\tREG_LOWP_RXETY %d\n"
		"\tTX_LOWP_ENA %d\n\tSFD %d\n\tNO_LEN_CHK %d\n\tSEND_IDLE %d\n"
		"\tCNT_FRM_EN %d\n\tSWR %d\n\tTXP %d\n\tXGLP %d\n"
		"\tTX_ADDR_INS %d\n\tPAUSE_IGN %d\n\tPAUSE_FWD %d\n\tCRC %d\n"
		"\tPAD %d\n\tPROMIS %d\n\tWAN %d\n\tRX_EN %d\n\tTX_EN %d\n",
		!!(val & ENETC_PM0_CMD_MG),
		!!(val & ENETC_PM0_CMD_RXSTP),
		!!(val & ENETC_PM0_CMD_REG_LOWP_RXETY),
		!!(val & ENETC_PM0_CMD_TX_LOWP_ENA),
		!!(val & ENETC_PM0_CMD_SFD),
		!!(val & ENETC_PM0_CMD_NO_LEN_CHK),
		!!(val & ENETC_PM0_CMD_SEND_IDLE),
		!!(val & ENETC_PM0_CMD_CNT_FRM_EN),
		!!(val & ENETC_PM0_CMD_SWR),
		!!(val & ENETC_PM0_CMD_TXP),
		!!(val & ENETC_PM0_CMD_XGLP),
		!!(val & ENETC_PM0_CMD_TX_ADDR_INS),
		!!(val & ENETC_PM0_CMD_PAUSE_IGN),
		!!(val & ENETC_PM0_CMD_PAUSE_FWD),
		!!(val & ENETC_PM0_CMD_CRC),
		!!(val & ENETC_PM0_CMD_PAD),
		!!(val & ENETC_PM0_CMD_PROMISC),
		!!(val & ENETC_PM0_CMD_WAN),
		!!(val & ENETC_PM0_CMD_RX_EN),
		!!(val & ENETC_PM0_CMD_TX_EN));
}

#define RXBDR_REGS(_i) \
	REG(ENETC_BDR(RX, (_i), ENETC_RBMR), "RX BDR " #_i " mode register"), \
	REG(ENETC_BDR(RX, (_i), ENETC_RBSR), "RX BDR " #_i " status register"), \
	REG(ENETC_BDR(RX, (_i), ENETC_RBBSR), "RX BDR " #_i " buffer size register"), \
	REG(ENETC_BDR(RX, (_i), ENETC_RBPIR), "RX BDR " #_i " producer index register"), \
	REG(ENETC_BDR(RX, (_i), ENETC_RBCIR), "RX BDR " #_i " consumer index register"), \
	REG(ENETC_BDR(RX, (_i), ENETC_RBBAR0), "RX BDR " #_i " base address register 0"), \
	REG(ENETC_BDR(RX, (_i), ENETC_RBBAR1), "RX BDR " #_i " base address register 1"), \
	REG(ENETC_BDR(RX, (_i), ENETC_RBLENR), "RX BDR " #_i " length register"), \
	REG(ENETC_BDR(RX, (_i), ENETC_RBIER), "RX BDR " #_i " interrupt enable register"), \
	REG(ENETC_BDR(RX, (_i), ENETC_RBICR0), "RX BDR " #_i " interrupt coalescing register 0"), \
	REG(ENETC_BDR(RX, (_i), ENETC_RBICR1), "RX BDR " #_i " interrupt coalescing register 1")

#define TXBDR_REGS(_i) \
	REG(ENETC_BDR(TX, (_i), ENETC_TBMR), "TX BDR " #_i " mode register"), \
	REG(ENETC_BDR(TX, (_i), ENETC_TBSR), "TX BDR " #_i " status register"), \
	REG(ENETC_BDR(TX, (_i), ENETC_TBBAR0), "TX BDR " #_i " base address register 0"), \
	REG(ENETC_BDR(TX, (_i), ENETC_TBBAR1), "TX BDR " #_i " base address register 1"), \
	REG(ENETC_BDR(TX, (_i), ENETC_TBPIR), "TX BDR " #_i " producer index register"), \
	REG(ENETC_BDR(TX, (_i), ENETC_TBCIR), "TX BDR " #_i " consumer index register"), \
	REG(ENETC_BDR(TX, (_i), ENETC_TBLENR), "TX BDR " #_i " length register"), \
	REG(ENETC_BDR(TX, (_i), ENETC_TBIER), "TX BDR " #_i " interrupt enable register"), \
	REG(ENETC_BDR(TX, (_i), ENETC_TBICR0), "TX BDR " #_i " interrupt coalescing register 0"), \
	REG(ENETC_BDR(TX, (_i), ENETC_TBICR1), "TX BDR " #_i " interrupt coalescing register 1")

static const struct enetc_register known_enetc_regs[] = {
	REG(ENETC_SIMR, "SI mode register"),
	REG(ENETC_SIPMAR0, "SI primary MAC address register 0"),
	REG(ENETC_SIPMAR1, "SI primary MAC address register 1"),
	REG(ENETC_SICBDRMR, "SI control BDR mode register"),
	REG(ENETC_SICBDRSR, "SI control BDR status register"),
	REG(ENETC_SICBDRBAR0, "SI control BDR base address register 0"),
	REG(ENETC_SICBDRBAR1, "SI control BDR base address register 1"),
	REG(ENETC_SICBDRPIR, "SI control BDR producer index register"),
	REG(ENETC_SICBDRCIR, "SI control BDR consumer index register"),
	REG(ENETC_SICBDRLENR, "SI control BDR length register"),
	REG(ENETC_SICAPR0, "SI capability register 0"),
	REG(ENETC_SICAPR1, "SI capability register 1"),
	REG(ENETC_SIUEFDCR, "SI uncorrectable error frame drop count register"),

	TXBDR_REGS(0), TXBDR_REGS(1), TXBDR_REGS(2), TXBDR_REGS(3),
	TXBDR_REGS(4), TXBDR_REGS(5), TXBDR_REGS(6), TXBDR_REGS(7),
	TXBDR_REGS(8), TXBDR_REGS(9), TXBDR_REGS(10), TXBDR_REGS(11),
	TXBDR_REGS(12), TXBDR_REGS(13), TXBDR_REGS(14), TXBDR_REGS(15),

	RXBDR_REGS(0), RXBDR_REGS(1), RXBDR_REGS(2), RXBDR_REGS(3),
	RXBDR_REGS(4), RXBDR_REGS(5), RXBDR_REGS(6), RXBDR_REGS(7),
	RXBDR_REGS(8), RXBDR_REGS(9), RXBDR_REGS(10), RXBDR_REGS(11),
	RXBDR_REGS(12), RXBDR_REGS(13), RXBDR_REGS(14), RXBDR_REGS(15),

	REG(ENETC_PMR, "Port mode register"),
	REG(ENETC_PSR, "Port status register"),
	REG(ENETC_PSIPMR, "Port SI promiscuous mode register"),
	REG(ENETC_PSIPMAR0(0), "Port SI0 primary MAC address register 0"),
	REG(ENETC_PSIPMAR1(0), "Port SI0 primary MAC address register 1"),
	REG(ENETC_PTXMBAR, "Port HTA transmit memory buffer allocation register"),
	REG(ENETC_PCAPR0, "Port capability register 0"),
	REG(ENETC_PCAPR1, "Port capability register 1"),
	REG(ENETC_PSICFGR0(0), "Port SI0 configuration register 0"),
	REG(ENETC_PRFSCAPR, "Port RFS capability register"),
	REG(ENETC_PTCMSDUR(0), "Port traffic class 0 maximum SDU register"),
	REG_DEC(ENETC_PM0_CMD_CFG, "Port eMAC Command and Configuration Register",
		decode_cmd_cfg),
	REG(ENETC_PM0_MAXFRM, "Port eMAC Maximum Frame Length Register"),
	REG(ENETC_PM0_IF_MODE, "Port eMAC Interface Mode Control Register"),
};

static void decode_known_reg(const struct enetc_register *reg, u32 val)
{
	char buf[512];

	reg->decode(val, buf);
	fprintf(stdout, "%s: 0x%x\n%s", reg->name, val, buf);
}

static void dump_known_reg(const struct enetc_register *reg, u32 val)
{
	fprintf(stdout, "%s: 0x%x\n", reg->name, val);
}

static void dump_unknown_reg(u32 addr, u32 val)
{
	fprintf(stdout, "Reg 0x%x: 0x%x\n", addr, val);
}

static void dump_reg(u32 addr, u32 val)
{
	const struct enetc_register *reg;
	u32 i;

	for (i = 0; i < ARRAY_SIZE(known_enetc_regs); i++) {
		reg = &known_enetc_regs[i];
		if (reg->addr == addr) {
			if (reg->decode)
				decode_known_reg(reg, val);
			else
				dump_known_reg(reg, val);
			return;
		}
	}

	dump_unknown_reg(addr, val);
}

/* Registers are structured in an array of key/value u32 pairs.
 * Compare each key to our list of known registers, or print it
 * as a raw address otherwise.
 */
int fsl_enetc_dump_regs(struct ethtool_drvinfo *info __maybe_unused,
			struct ethtool_regs *regs)
{
	u32 *data = (u32 *)regs->data;
	u32 len = regs->len;

	if (len % 8) {
		fprintf(stdout, "Expected length to be multiple of 8 bytes\n");
		return -1;
	}

	while (len) {
		dump_reg(data[0], data[1]);
		data += 2; len -= 8;
	}

	return 0;
}
