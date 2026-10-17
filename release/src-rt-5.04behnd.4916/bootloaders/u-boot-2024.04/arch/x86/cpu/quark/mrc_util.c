// SPDX-License-Identifier: Intel
/*
 * Copyright (C) 2013, Intel Corporation
 * Copyright (C) 2015, Bin Meng <bmeng.cn@gmail.com>
 *
 * Ported from Intel released Quark UEFI BIOS
 * QuarkSocPkg/QuarkNorthCluster/MemoryInit/Pei
 */

#include <common.h>
#include <hang.h>
#include <asm/arch/device.h>
#include <asm/arch/mrc.h>
#include <asm/arch/msg_port.h>
#include <asm/arch/quark.h>
#include "mrc_util.h"
#include "hte.h"
#include "smc.h"

static const uint8_t vref_codes[64] = {
	/* lowest to highest */
	0x3f, 0x3e, 0x3d, 0x3c, 0x3b, 0x3a, 0x39, 0x38,
	0x37, 0x36, 0x35, 0x34, 0x33, 0x32, 0x31, 0x30,
	0x2f, 0x2e, 0x2d, 0x2c, 0x2b, 0x2a, 0x29, 0x28,
	0x27, 0x26, 0x25, 0x24, 0x23, 0x22, 0x21, 0x20,
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
	0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
	0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f
};

void mrc_write_mask(u32 unit, u32 addr, u32 data, u32 mask)
{
	msg_port_write(unit, addr,
		       (msg_port_read(unit, addr) & ~(mask)) |
		       ((data) & (mask)));
}

void mrc_alt_write_mask(u32 unit, u32 addr, u32 data, u32 mask)
{
	msg_port_alt_write(unit, addr,
			   (msg_port_alt_read(unit, addr) & ~(mask)) |
			   ((data) & (mask)));
}

void mrc_post_code(uint8_t major, uint8_t minor)
{
	/* send message to UART */
	DPF(D_INFO, "POST: 0x%01x%02x\n", major, minor);

	/* error check */
	if (major == 0xee)
		hang();
}

/* Delay number of nanoseconds */
void delay_n(uint32_t ns)
{
	/* 1000 MHz clock has 1ns period --> no conversion required */
	uint64_t final_tsc = rdtsc();

	final_tsc += ((get_tbclk_mhz() * ns) / 1000);

	while (rdtsc() < final_tsc)
		;
}

/* Delay number of microseconds */
void delay_u(uint32_t ms)
{
	/* 64-bit math is not an option, just use loops */
	while (ms--)
		delay_n(1000);
}

/* Select Memory Manager as the source for PRI interface */
void select_mem_mgr(void)
{
	u32 dco;

	ENTERFN();

	dco = msg_port_read(MEM_CTLR, DCO);
	dco &= ~DCO_PMICTL;
	msg_port_write(MEM_CTLR, DCO, dco);

	LEAVEFN();
}

/* Select HTE as the source for PRI interface */
void select_hte(void)
{
	u32 dco;

	ENTERFN();

	dco = msg_port_read(MEM_CTLR, DCO);
	dco |= DCO_PMICTL;
	msg_port_write(MEM_CTLR, DCO, dco);

	LEAVEFN();
}

/*
 * Send DRAM command
 * data should be formated using DCMD_Xxxx macro or emrsXCommand structure
 */
void dram_init_command(uint32_t data)
{
	qrk_pci_write_config_dword(QUARK_HOST_BRIDGE, MSG_DATA_REG, data);
	qrk_pci_write_config_dword(QUARK_HOST_BRIDGE, MSG_CTRL_EXT_REG, 0);
	msg_port_setup(MSG_OP_DRAM_INIT, MEM_CTLR, 0);

	DPF(D_REGWR, "WR32 %03X %08X %08X\n", MEM_CTLR, 0, data);
}

/* Send DRAM wake command using special MCU side-band WAKE opcode */
void dram_wake_command(void)
{
	ENTERFN();

	msg_port_setup(MSG_OP_DRAM_WAKE, MEM_CTLR, 0);

	LEAVEFN();
}

void training_message(uint8_t channel, uint8_t rank, uint8_t byte_lane)
{
	/* send message to UART */
	DPF(D_INFO, "CH%01X RK%01X BL%01X\n", channel, rank, byte_lane);
}

/*
 * This function will program the RCVEN delays
 *
 * (currently doesn't comprehend rank)
 */
void set_rcvn(uint8_t channel, uint8_t rank,
	      uint8_t byte_lane, uint32_t pi_count)
{
	uint32_t reg;
	uint32_t msk;
	uint32_t temp;

	ENTERFN();

	DPF(D_TRN, "Rcvn ch%d rnk%d ln%d : pi=%03X\n",
	    channel, rank, byte_lane, pi_count);

	/*
	 * RDPTR (1/2 MCLK, 64 PIs)
	 * BL0 -> B01PTRCTL0[11:08] (0x0-0xF)
	 * BL1 -> B01PTRCTL0[23:20] (0x0-0xF)
	 */
	reg = B01PTRCTL0 + (byte_lane >> 1) * DDRIODQ_BL_OFFSET +
		channel * DDRIODQ_CH_OFFSET;
	msk = (byte_lane & 1) ? 0xf00000 : 0xf00;
	temp = (byte_lane & 1) ? (pi_count / HALF_CLK) << 20 :
		(pi_count / HALF_CLK) << 8;
	mrc_alt_write_mask(DDRPHY, reg, temp, msk);

	/* Adjust PI_COUNT */
	pi_count -= ((pi_count / HALF_CLK) & 0xf) * HALF_CLK;

	/*
	 * PI (1/64 MCLK, 1 PIs)
	 * BL0 -> B0DLLPICODER0[29:24] (0x00-0x3F)
	 * BL1 -> B1DLLPICODER0[29:24] (0x00-0x3F)
	 */
	reg = (byte_lane & 1) ? B1DLLPICODER0 : B0DLLPICODER0;
	reg += ((byte_lane >> 1) * DDRIODQ_BL_OFFSET +
		channel * DDRIODQ_CH_OFFSET);
	msk = 0x3f000000;
	temp = pi_count << 24;
	mrc_alt_write_mask(DDRPHY, reg, temp, msk);

	/*
	 * DEADBAND
	 * BL0/1 -> B01DBCTL1[08/11] (+1 select)
	 * BL0/1 -> B01DBCTL1[02/05] (enable)
	 */
	reg = B01DBCTL1 + (byte_lane >> 1) * DDRIODQ_BL_OFFSET +
		channel * DDRIODQ_CH_OFFSET;
	msk = 0x00;
	temp = 0x00;

	/* enable */
	msk |= (byte_lane & 1) ? (1 << 5) : (1 << 2);
	if ((pi_count < EARLY_DB) || (pi_count > LATE_DB))
		temp |= msk;

	/* select */
	msk |= (byte_lane & 1) ? (1 << 11) : (1 << 8);
	if (pi_count < EARLY_DB)
		temp |= msk;

	mrc_alt_write_mask(DDRPHY, reg, temp, msk);

	/* error check */
	if (pi_count > 0x3f) {
		training_message(channel, rank, byte_lane);
		mrc_post_code(0xee, 0xe0);
	}

	LEAVEFN();
}

/*
 * This function will return the current RCVEN delay on the given
 * channel, rank, byte_lane as an absolute PI count.
 *
 * (currently doesn't comprehend rank)
 */
uint32_t get_rcvn(uint8_t channel, uint8_t rank, uint8_t byte_lane)
{
	uint32_t reg;
	uint32_t temp;
	uint32_t pi_count;

	ENTERFN();

	/*
	 * RDPTR (1/2 MCLK, 64 PIs)
	 * BL0 -> B01PTRCTL0[11:08] (0x0-0xF)
	 * BL1 -> B01PTRCTL0[23:20] (0x0-0xF)
	 */
	reg = B01PTRCTL0 + (byte_lane >> 1) * DDRIODQ_BL_OFFSET +
		channel * DDRIODQ_CH_OFFSET;
	temp = msg_port_alt_read(DDRPHY, reg);
	temp >>= (byte_lane & 1) ? 20 : 8;
	temp &= 0xf;

	/* Adjust PI_COUNT */
	pi_count = temp * HALF_CLK;

	/*
	 * PI (1/64 MCLK, 1 PIs)
	 * BL0 -> B0DLLPICODER0[29:24] (0x00-0x3F)
	 * BL1 -> B1DLLPICODER0[29:24] (0x00-0x3F)
	 */
	reg = (byte_lane & 1) ? B1DLLPICODER0 : B0DLLPICODER0;
	reg += ((byte_lane >> 1) * DDRIODQ_BL_OFFSET +
		channel * DDRIODQ_CH_OFFSET);
	temp = msg_port_alt_read(DDRPHY, reg);
	temp >>= 24;
	temp &= 0x3f;

	/* Adjust PI_COUNT */
	pi_count += temp;

	LEAVEFN();

	return pi_count;
}

/*
 * This function will program the RDQS delays based on an absolute
 * amount of PIs.
 *
 * (currently doesn't comprehend rank)
 */
void set_rdqs(uint8_t channel, uint8_t rank,
	      uint8_t byte_lane, uint32_t pi_count)
{
	uint32_t reg;
	uint32_t msk;
	uint32_t temp;

	ENTERFN();
	DPF(D_TRN, "Rdqs ch%d rnk%d ln%d : pi=%03X\n",
	    channel, rank, byte_lane, pi_count);

	/*
	 * PI (1/128 MCLK)
	 * BL0 -> B0RXDQSPICODE[06:00] (0x00-0x47)
	 * BL1 -> B1RXDQSPICODE[06:00] (0x00-0x47)
	 */
	reg = (byte_lane & 1) ? B1RXDQSPICODE : B0RXDQSPICODE;
	reg += ((byte_lane >> 1) * DDRIODQ_BL_OFFSET +
		channel * DDRIODQ_CH_OFFSET);
	msk = 0x7f;
	temp = pi_count << 0;
	mrc_alt_write_mask(DDRPHY, reg, temp, msk);

	/* error check (shouldn't go above 0x3F) */
	if (pi_count > 0x47) {
		training_message(channel, rank, byte_lane);
		mrc_post_code(0xee, 0xe1);
	}

	LEAVEFN();
}

/*
 * This function will return the current RDQS delay on the given
 * channel, rank, byte_lane as an absolute PI count.
 *
 * (currently doesn't comprehend rank)
 */
uint32_t get_rdqs(uint8_t channel, uint8_t rank, uint8_t byte_lane)
{
	uint32_t reg;
	uint32_t temp;
	uint32_t pi_count;

	ENTERFN();

	/*
	 * PI (1/128 MCLK)
	 * BL0 -> B0RXDQSPICODE[06:00] (0x00-0x47)
	 * BL1 -> B1RXDQSPICODE[06:00] (0x00-0x47)
	 */
	reg = (byte_lane & 1) ? B1RXDQSPICODE : B0RXDQSPICODE;
	reg += ((byte_lane >> 1) * DDRIODQ_BL_OFFSET +
		channel * DDRIODQ_CH_OFFSET);
	temp = msg_port_alt_read(DDRPHY, reg);

	/* Adjust PI_COUNT */
	pi_count = temp & 0x7f;

	LEAVEFN();

	return pi_count;
}

/*
 * This function will program the WDQS delays based on an absolute
 * amount of PIs.
 *
 * (currently doesn't comprehend rank)
 */
void set_wdqs(uint8_t channel, uint8_t rank,
	      uint8_t byte_lane, uint32_t pi_count)
{
	uint32_t reg;
	uint32_t msk;
	uint32_t temp;

	ENTERFN();

	DPF(D_TRN, "Wdqs ch%d rnk%d ln%d : pi=%03X\n",
	    channel, rank, byte_lane, pi_count);

	/*
	 * RDPTR (1/2 MCLK, 64 PIs)
	 * BL0 -> B01PTRCTL0[07:04] (0x0-0xF)
	 * BL1 -> B01PTRCTL0[19:16] (0x0-0xF)
	 */
	reg = B01PTRCTL0 + (byte_lane >> 1) * DDRIODQ_BL_OFFSET +
		channel * DDRIODQ_CH_OFFSET;
	msk = (byte_lane & 1) ? 0xf0000 : 0xf0;
	temp = pi_count / HALF_CLK;
	temp <<= (byte_lane & 1) ? 16 : 4;
	mrc_alt_write_mask(DDRPHY, reg, temp, msk);

	/* Adjust PI_COUNT */
	pi_count -= ((pi_count / HALF_CLK) & 0xf) * HALF_CLK;

	/*
	 * PI (1/64 MCLK, 1 PIs)
	 * BL0 -> B0DLLPICODER0[21:16] (0x00-0x3F)
	 * BL1 -> B1DLLPICODER0[21:16] (0x00-0x3F)
	 */
	reg = (byte_lane & 1) ? B1DLLPICODER0 : B0DLLPICODER0;
	reg += ((byte_lane >> 1) * DDRIODQ_BL_OFFSET +
		channel * DDRIODQ_CH_OFFSET);
	msk = 0x3f0000;
	temp = pi_count << 16;
	mrc_alt_write_mask(DDRPHY, reg, temp, msk);

	/*
	 * DEADBAND
	 * BL0/1 -> B01DBCTL1[07/10] (+1 select)
	 * BL0/1 -> B01DBCTL1[01/04] (enable)
	 */
	reg = B01DBCTL1 + (byte_lane >> 1) * DDRIODQ_BL_OFFSET +
		channel * DDRIODQ_CH_OFFSET;
	msk = 0x00;
	temp = 0x00;

	/* enable */
	msk |= (byte_lane & 1) ? (1 << 4) : (1 << 1);
	if ((pi_count < EARLY_DB) || (pi_count > LATE_DB))
		temp |= msk;

	/* select */
	msk |= (byte_lane & 1) ? (1 << 10) : (1 << 7);
	if (pi_count < EARLY_DB)
		temp |= msk;

	mrc_alt_write_mask(DDRPHY, reg, temp, msk);

	/* error check */
	if (pi_count > 0x3f) {
		training_message(channel, rank, byte_lane);
		mrc_post_code(0xee, 0xe2);
	}

	LEAVEFN();
}

/*
 * This function will return the amount of WDQS delay on the given
 * channel, rank, byte_lane as an absolute PI count.
 *
 * (currently doesn't comprehend rank)
 */
uint32_t get_wdqs(uint8_t channel, uint8_t rank, uint8_t byte_lane)
{
	uint32_t reg;
	uint32_t temp;
	uint32_t pi_count;

	ENTERFN();

	/*
	 * RDPTR (1/2 MCLK, 64 PIs)
	 * BL0 -> B01PTRCTL0[07:04] (0x0-0xF)
	 * BL1 -> B01PTRCTL0[19:16] (0x0-0xF)
	 */
	reg = B01PTRCTL0 + (byte_lane >> 1) * DDRIODQ_BL_OFFSET +
		channel * DDRIODQ_CH_OFFSET;
	temp = msg_port_alt_read(DDRPHY, reg);
	temp >>= (byte_lane & 1) ? 16 : 4;
	temp &= 0xf;

	/* Adjust PI_COUNT */
	pi_count = (temp * HALF_CLK);

	/*
	 * PI (1/64 MCLK, 1 PIs)
	 * BL0 -> B0DLLPICODER0[21:16] (0x00-0x3F)
	 * BL1 -> B1DLLPICODER0[21:16] (0x00-0x3F)
	 */
	reg = (byte_lane & 1) ? B1DLLPICODER0 : B0DLLPICODER0;
	reg += ((byte_lane >> 1) * DDRIODQ_BL_OFFSET +
		channel * DDRIODQ_CH_OFFSET);
	temp = msg_port_alt_read(DDRPHY, reg);
	temp >>= 16;
	temp &= 0x3f;

	/* Adjust PI_COUNT */
	pi_count += temp;

	LEAVEFN();

	return pi_count;
}

/*
 * This function will program the WDQ delays based on an absolute
 * number of PIs.
 *
 * (currently doesn't comprehend rank)
 */
void set_wdq(uint8_t channel, uint8_t rank,
	     uint8_t byte_lane, uint32_t pi_count)
{
	uint32_t reg;
	uint32_t msk;
	uint32_t temp;

	ENTERFN();

	DPF(D_TRN, "Wdq ch%d rnk%d ln%d : pi=%03X\n",
	    channel, rank, byte_lane, pi_count);

	/*
	 * RDPTR (1/2 MCLK, 64 PIs)
	 * BL0 -> B01PTRCTL0[03:00] (0x0-0xF)
	 * BL1 -> B01PTRCTL0[15:12] (0x0-0xF)
	 */
	reg = B01PTRCTL0 + (byte_lane >> 1) * DDRIODQ_BL_OFFSET +
		channel * DDRIODQ_CH_OFFSET;
	msk = (byte_lane & 1) ? 0xf000 : 0xf;
	temp = pi_count / HALF_CLK;
	temp <<= (byte_lane & 1) ? 12 : 0;
	mrc_alt_write_mask(DDRPHY, reg, temp, msk);

	/* Adjust PI_COUNT */
	pi_count -= ((pi_count / HALF_CLK) & 0xf) * HALF_CLK;

	/*
	 * PI (1/64 MCLK, 1 PIs)
	 * BL0 -> B0DLLPICODER0[13:08] (0x00-0x3F)
	 * BL1 -> B1DLLPICODER0[13:08] (0x00-0x3F)
	 */
	reg = (byte_lane & 1) ? B1DLLPICODER0 : B0DLLPICODER0;
	reg += ((byte_lane >> 1) * DDRIODQ_BL_OFFSET +
		channel * DDRIODQ_CH_OFFSET);
	msk = 0x3f00;
	temp = pi_count << 8;
	mrc_alt_write_mask(DDRPHY, reg, temp, msk);

	/*
	 * DEADBAND
	 * BL0/1 -> B01DBCTL1[06/09] (+1 select)
	 * BL0/1 -> B01DBCTL1[00/03] (enable)
	 */
	reg = B01DBCTL1 + (byte_lane >> 1) * DDRIODQ_BL_OFFSET +
		channel * DDRIODQ_CH_OFFSET;
	msk = 0x00;
	temp = 0x00;

	/* enable */
	msk |= (byte_lane & 1) ? (1 << 3) : (1 << 0);
	if ((pi_count < EARLY_DB) || (pi_count > LATE_DB))
		temp |= msk;

	/* select */
	msk |= (byte_lane & 1) ? (1 << 9) : (1 << 6);
	if (pi_count < EARLY_DB)
		temp |= msk;

	mrc_alt_write_mask(DDRPHY, reg, temp, msk);

	/* error check */
	if (pi_count > 0x3f) {
		training_message(channel, rank, byte_lane);
		mrc_post_code(0xee, 0xe3);
	}

	LEAVEFN();
}

/*
 * This function will return the amount of WDQ delay on the given
 * channel, rank, byte_lane as an absolute PI count.
 *
 * (currently doesn't comprehend rank)
 */
uint32_t get_wdq(uint8_t channel, uint8_t rank, uint8_t byte_lane)
{
	uint32_t reg;
	uint32_t temp;
	uint32_t pi_count;

	ENTERFN();

	/*
	 * RDPTR (1/2 MCLK, 64 PIs)
	 * BL0 -> B01PTRCTL0[03:00] (0x0-0xF)
	 * BL1 -> B01PTRCTL0[15:12] (0x0-0xF)
	 */
	reg = B01PTRCTL0 + (byte_lane >> 1) * DDRIODQ_BL_OFFSET +
		channel * DDRIODQ_CH_OFFSET;
	temp = msg_port_alt_read(DDRPHY, reg);
	temp >>= (byte_lane & 1) ? 12 : 0;
	temp &= 0xf;

	/* Adjust PI_COUNT */
	pi_count = temp * HALF_CLK;

	/*
	 * PI (1/64 MCLK, 1 PIs)
	 * BL0 -> B0DLLPICODER0[13:08] (0x00-0x3F)
	 * BL1 -> B1DLLPICODER0[13:08] (0x00-0x3F)
	 */
	reg = (byte_lane & 1) ? B1DLLPICODER0 : B0DLLPICODER0;
	reg += ((byte_lane >> 1) * DDRIODQ_BL_OFFSET +
		channel * DDRIODQ_CH_OFFSET);
	temp = msg_port_alt_read(DDRPHY, reg);
	temp >>= 8;
	temp &= 0x3f;

	/* Adjust PI_COUNT */
	pi_count += temp;

	LEAVEFN();

	return pi_count;
}

/*
 * This function will program the WCMD delays based on an absolute
 * number of PIs.
 */
void set_wcmd(uint8_t channel, uint32_t pi_count)
{
	uint32_t reg;
	uint32_t msk;
	uint32_t temp;

	ENTERFN();

	/*
	 * RDPTR (1/2 MCLK, 64 PIs)
	 * CMDPTRREG[11:08] (0x0-0xF)
	 */
	reg = CMDPTRREG + channel * DDRIOCCC_CH_OFFSET;
	msk = 0xf00;
	temp = pi_count / HALF_CLK;
	temp <<= 8;
	mrc_alt_write_mask(DDRPHY, reg, temp, msk);

	/* Adjust PI_COUNT */
	pi_count -= ((pi_count / HALF_CLK) & 0xf) * HALF_CLK;

	/*
	 * PI (1/64 MCLK, 1 PIs)
	 * CMDDLLPICODER0[29:24] -> CMDSLICE R3 (unused)
	 * CMDDLLPICODER0[21:16] -> CMDSLICE L3 (unused)
	 * CMDDLLPICODER0[13:08] -> CMDSLICE R2 (unused)
	 * CMDDLLPICODER0[05:00] -> CMDSLICE L2 (unused)
	 * CMDDLLPICODER1[29:24] -> CMDSLICE R1 (unused)
	 * CMDDLLPICODER1[21:16] -> CMDSLICE L1 (0x00-0x3F)
	 * CMDDLLPICODER1[13:08] -> CMDSLICE R0 (unused)
	 * CMDDLLPICODER1[05:00] -> CMDSLICE L0 (unused)
	 */
	reg = CMDDLLPICODER1 + channel * DDRIOCCC_CH_OFFSET;
	msk = 0x3f3f3f3f;
	temp = (pi_count << 24) | (pi_count << 16) |
		(pi_count << 8) | (pi_count << 0);

	mrc_alt_write_mask(DDRPHY, reg, temp, msk);
	reg = CMDDLLPICODER0 + channel * DDRIOCCC_CH_OFFSET;	/* PO */
	mrc_alt_write_mask(DDRPHY, reg, temp, msk);

	/*
	 * DEADBAND
	 * CMDCFGREG0[17] (+1 select)
	 * CMDCFGREG0[16] (enable)
	 */
	reg = CMDCFGREG0 + channel * DDRIOCCC_CH_OFFSET;
	msk = 0x00;
	temp = 0x00;

	/* enable */
	msk |= (1 << 16);
	if ((pi_count < EARLY_DB) || (pi_count > LATE_DB))
		temp |= msk;

	/* select */
	msk |= (1 << 17);
	if (pi_count < EARLY_DB)
		temp |= msk;

	mrc_alt_write_mask(DDRPHY, reg, temp, msk);

	/* error check */
	if (pi_count > 0x3f)
		mrc_post_code(0xee, 0xe4);

	LEAVEFN();
}

/*
 * This function will return the amount of WCMD delay on the given
 * channel as an absolute PI count.
 */
uint32_t get_wcmd(uint8_t channel)
{
	uint32_t reg;
	uint32_t temp;
	uint32_t pi_count;

	ENTERFN();

	/*
	 * RDPTR (1/2 MCLK, 64 PIs)
	 * CMDPTRREG[11:08] (0x0-0xF)
	 */
	reg = CMDPTRREG + channel * DDRIOCCC_CH_OFFSET;
	temp = msg_port_alt_read(DDRPHY, reg);
	temp >>= 8;
	temp &= 0xf;

	/* Adjust PI_COUNT */
	pi_count = temp * HALF_CLK;

	/*
	 * PI (1/64 MCLK, 1 PIs)
	 * CMDDLLPICODER0[29:24] -> CMDSLICE R3 (unused)
	 * CMDDLLPICODER0[21:16] -> CMDSLICE L3 (unused)
	 * CMDDLLPICODER0[13:08] -> CMDSLICE R2 (unused)
	 * CMDDLLPICODER0[05:00] -> CMDSLICE L2 (unused)
	 * CMDDLLPICODER1[29:24] -> CMDSLICE R1 (unused)
	 * CMDDLLPICODER1[21:16] -> CMDSLICE L1 (0x00-0x3F)
	 * CMDDLLPICODER1[13:08] -> CMDSLICE R0 (unused)
	 * CMDDLLPICODER1[05:00] -> CMDSLICE L0 (unused)
	 */
	reg = CMDDLLPICODER1 + channel * DDRIOCCC_CH_OFFSET;
	temp = msg_port_alt_read(DDRPHY, reg);
	temp >>= 16;
	temp &= 0x3f;

	/* Adjust PI_COUNT */
	pi_count += temp;

	LEAVEFN();

	return pi_count;
}

/*
 * This function will program the WCLK delays based on an absolute
 * number of PIs.
 */
void set_wclk(uint8_t channel, uint8_t rank, uint32_t pi_count)
{
	uint32_t reg;
	uint32_t msk;
	uint32_t temp;

	ENTERFN();

	/*
	 * RDPTR (1/2 MCLK, 64 PIs)
	 * CCPTRREG[15:12] -> CLK1 (0x0-0xF)
	 * CCPTRREG[11:08] -> CLK0 (0x0-0xF)
	 */
	reg = CCPTRREG + channel * DDRIOCCC_CH_OFFSET;
	msk = 0xff00;
	temp = ((pi_count / HALF_CLK) << 12) | ((pi_count / HALF_CLK) << 8);
	mrc_alt_write_mask(DDRPHY, reg, temp, msk);

	/* Adjust PI_COUNT */
	pi_count -= ((pi_count / HALF_CLK) & 0xf) * HALF_CLK;

	/*
	 * PI (1/64 MCLK, 1 PIs)
	 * ECCB1DLLPICODER0[13:08] -> CLK0 (0x00-0x3F)
	 * ECCB1DLLPICODER0[21:16] -> CLK1 (0x00-0x3F)
	 */
	reg = rank ? ECCB1DLLPICODER0 : ECCB1DLLPICODER0;
	reg += (channel * DDRIOCCC_CH_OFFSET);
	msk = 0x3f3f00;
	temp = (pi_count << 16) | (pi_count << 8);
	mrc_alt_write_mask(DDRPHY, reg, temp, msk);

	reg = rank ? ECCB1DLLPICODER1 : ECCB1DLLPICODER1;
	reg += (channel * DDRIOCCC_CH_OFFSET);
	mrc_alt_write_mask(DDRPHY, reg, temp, msk);

	reg = rank ? ECCB1DLLPICODER2 : ECCB1DLLPICODER2;
	reg += (channel * DDRIOCCC_CH_OFFSET);
	mrc_alt_write_mask(DDRPHY, reg, temp, msk);

	reg = rank ? ECCB1DLLPICODER3 : ECCB1DLLPICODER3;
	reg += (channel * DDRIOCCC_CH_OFFSET);
	mrc_alt_write_mask(DDRPHY, reg, temp, msk);

	/*
	 * DEADBAND
	 * CCCFGREG1[11:08] (+1 select)
	 * CCCFGREG1[03:00] (enable)
	 */
	reg = CCCFGREG1 + channel * DDRIOCCC_CH_OFFSET;
	msk = 0x00;
	temp = 0x00;

	/* enable */
	msk |= 0xf;
	if ((pi_count < EARLY_DB) || (pi_count > LATE_DB))
		temp |= msk;

	/* select */
	msk |= 0xf00;
	if (pi_count < EARLY_DB)
		temp |= msk;

	mrc_alt_write_mask(DDRPHY, reg, temp, msk);

	/* error check */
	if (pi_count > 0x3f)
		mrc_post_code(0xee, 0xe5);

	LEAVEFN();
}

/*
 * This function will return the amout of WCLK delay on the given
 * channel, rank as an absolute PI count.
 */
uint32_t get_wclk(uint8_t channel, uint8_t rank)
{
	uint32_t reg;
	uint32_t temp;
	uint32_t pi_count;

	ENTERFN();

	/*
	 * RDPTR (1/2 MCLK, 64 PIs)
	 * CCPTRREG[15:12] -> CLK1 (0x0-0xF)
	 * CCPTRREG[11:08] -> CLK0 (0x0-0xF)
	 */
	reg = CCPTRREG + channel * DDRIOCCC_CH_OFFSET;
	temp = msg_port_alt_read(DDRPHY, reg);
	temp >>= rank ? 12 : 8;
	temp &= 0xf;

	/* Adjust PI_COUNT */
	pi_count = temp * HALF_CLK;

	/*
	 * PI (1/64 MCLK, 1 PIs)
	 * ECCB1DLLPICODER0[13:08] -> CLK0 (0x00-0x3F)
	 * ECCB1DLLPICODER0[21:16] -> CLK1 (0x00-0x3F)
	 */
	reg = rank ? ECCB1DLLPICODER0 : ECCB1DLLPICODER0;
	reg += (channel * DDRIOCCC_CH_OFFSET);
	temp = msg_port_alt_read(DDRPHY, reg);
	temp >>= rank ? 16 : 8;
	temp &= 0x3f;

	pi_count += temp;

	LEAVEFN();

	return pi_count;
}

/*
 * This function will program the WCTL delays based on an absolute
 * number of PIs.
 *
 * (currently doesn't comprehend rank)
 */
void set_wctl(uint8_t channel, uint8_t rank, uint32_t pi_count)
{
	uint32_t reg;
	uint32_t msk;
	uint32_t temp;

	ENTERFN();

	/*
	 * RDPTR (1/2 MCLK, 64 PIs)
	 * CCPTRREG[31:28] (0x0-0xF)
	 * CCPTRREG[27:24] (0x0-0xF)
	 */
	reg = CCPTRREG + channel * DDRIOCCC_CH_OFFSET;
	msk = 0xff000000;
	temp = ((pi_count / HALF_CLK) << 28) | ((pi_count / HALF_CLK) << 24);
	mrc_alt_write_mask(DDRPHY, reg, temp, msk);

	/* Adjust PI_COUNT */
	pi_count -= ((pi_count / HALF_CLK) & 0xf) * HALF_CLK;

	/*
	 * PI (1/64 MCLK, 1 PIs)
	 * ECCB1DLLPICODER?[29:24] (0x00-0x3F)
	 * ECCB1DLLPICODER?[29:24] (0x00-0x3F)
	 */
	reg = ECCB1DLLPICODER0 + channel * DDRIOCCC_CH_OFFSET;
	msk = 0x3f000000;
	temp = (pi_count << 24);
	mrc_alt_write_mask(DDRPHY, reg, temp, msk);

	reg = ECCB1DLLPICODER1 + channel * DDRIOCCC_CH_OFFSET;
	mrc_alt_write_mask(DDRPHY, reg, temp, msk);

	reg = ECCB1DLLPICODER2 + channel * DDRIOCCC_CH_OFFSET;
	mrc_alt_write_mask(DDRPHY, reg, temp, msk);

	reg = ECCB1DLLPICODER3 + channel * DDRIOCCC_CH_OFFSET;
	mrc_alt_write_mask(DDRPHY, reg, temp, msk);

	/*
	 * DEADBAND
	 * CCCFGREG1[13:12] (+1 select)
	 * CCCFGREG1[05:04] (enable)
	 */
	reg = CCCFGREG1 + channel * DDRIOCCC_CH_OFFSET;
	msk = 0x00;
	temp = 0x00;

	/* enable */
	msk |= 0x30;
	if ((pi_count < EARLY_DB) || (pi_count > LATE_DB))
		temp |= msk;

	/* select */
	msk |= 0x3000;
	if (pi_count < EARLY_DB)
		temp |= msk;

	mrc_alt_write_mask(DDRPHY, reg, temp, msk);

	/* error check */
	if (pi_count > 0x3f)
		mrc_post_code(0xee, 0xe6);

	LEAVEFN();
}

/*
 * This function will return the amount of WCTL delay on the given
 * channel, rank as an absolute PI count.
 *
 * (currently doesn't comprehend rank)
 */
uint32_t get_wctl(uint8_t channel, uint8_t rank)
{
	uint32_t reg;
	uint32_t temp;
	uint32_t pi_count;

	ENTERFN();

	/*
	 * RDPTR (1/2 MCLK, 64 PIs)
	 * CCPTRREG[31:28] (0x0-0xF)
	 * CCPTRREG[27:24] (0x0-0xF)
	 */
	reg = CCPTRREG + channel * DDRIOCCC_CH_OFFSET;
	temp = msg_port_alt_read(DDRPHY, reg);
	temp >>= 24;
	temp &= 0xf;

	/* Adjust PI_COUNT */
	pi_count = temp * HALF_CLK;

	/*
	 * PI (1/64 MCLK, 1 PIs)
	 * ECCB1DLLPICODER?[29:24] (0x00-0x3F)
	 * ECCB1DLLPICODER?[29:24] (0x00-0x3F)
	 */
	reg = ECCB1DLLPICODER0 + channel * DDRIOCCC_CH_OFFSET;
	temp = msg_port_alt_read(DDRPHY, reg);
	temp >>= 24;
	temp &= 0x3f;

	/* Adjust PI_COUNT */
	pi_count += temp;

	LEAVEFN();

	return pi_count;
}

/*
 * This function will program the internal Vref setting in a given
 * byte lane in a given channel.
 */
void set_vref(uint8_t channel, uint8_t byte_lane, uint32_t setting)
{
	uint32_t reg = (byte_lane & 0x1) ? B1VREFCTL : B0VREFCTL;

	ENTERFN();

	DPF(D_TRN, "Vref ch%d ln%d : val=%03X\n",
	    channel, byte_lane, setting);

	mrc_alt_write_mask(DDRPHY, reg + channel * DDRIODQ_CH_OFFSET +
		(byte_lane >> 1) * DDRIODQ_BL_OFFSET,
		vref_codes[setting] << 2, 0xfc);

	/*
	 * need to wait ~300ns for Vref to settle
	 * (check that this is necessary)
	 */
	delay_n(300);

	/* ??? may need to clear pointers ??? */

	LEAVEFN();
}

/*
 * This function will return the internal Vref setting for the given
 * channel, byte_lane.
 */
uint32_t get_vref(uint8_t channel, uint8_t byte_lane)
{
	uint8_t j;
	uint32_t ret_val = sizeof(vref_codes) / 2;
	uint32_t reg = (byte_lane & 0x1) ? B1VREFCTL : B0VREFCTL;
	uint32_t temp;

	ENTERFN();

	temp = msg_port_alt_read(DDRPHY, reg + channel * DDRIODQ_CH_OFFSET +
		(byte_lane >> 1) * DDRIODQ_BL_OFFSET);
	temp >>= 2;
	temp &= 0x3f;

	for (j = 0; j < sizeof(vref_codes); j++) {
		if (vref_codes[j] == temp) {
			ret_val = j;
			break;
		}
	}

	LEAVEFN();

	return ret_val;
}

/*
 * This function will return a 32-bit address in the desired
 * channel and rank.
 */
uint32_t get_addr(uint8_t channel, uint8_t rank)
{
	uint32_t offset = 32 * 1024 * 1024;	/* 32MB */

	/* Begin product specific code */
	if (channel > 0) {
		DPF(D_ERROR, "ILLEGAL CHANNEL\n");
		DEAD_LOOP();
	}

	if (rank > 1) {
		DPF(D_ERROR, "ILLEGAL RANK\n");
		DEAD_LOOP();
	}

	/* use 256MB lowest density as per DRP == 0x0003 */
	offset += rank * (256 * 1024 * 1024);

	return offset;
}

/*
 * This function will sample the DQTRAINSTS registers in the given
 * channel/rank SAMPLE_SIZE times looking for a valid '0' or '1'.
 *
 * It will return an encoded 32-bit date in which each bit corresponds to
 * the sampled value on the byte lane.
 */
uint32_t sample_dqs(struct mrc_params *mrc_params, uint8_t channel,
		    uint8_t rank, bool rcvn)
{
	uint8_t j;	/* just a counter */
	uint8_t bl;	/* which BL in the module (always 2 per module) */
	uint8_t bl_grp;	/* which BL module */
	/* byte lane divisor */
	uint8_t bl_divisor = (mrc_params->channel_width == X16) ? 2 : 1;
	uint32_t msk[2];	/* BLx in module */
	/* DQTRAINSTS register contents for each sample */
	uint32_t sampled_val[SAMPLE_SIZE];
	uint32_t num_0s;	/* tracks the number of '0' samples */
	uint32_t num_1s;	/* tracks the number of '1' samples */
	uint32_t ret_val = 0x00;	/* assume all '0' samples */
	uint32_t address = get_addr(channel, rank);

	/* initialise msk[] */
	msk[0] = rcvn ? (1 << 1) : (1 << 9);	/* BL0 */
	msk[1] = rcvn ? (1 << 0) : (1 << 8);	/* BL1 */

	/* cycle through each byte lane group */
	for (bl_grp = 0; bl_grp < (NUM_BYTE_LANES / bl_divisor) / 2; bl_grp++) {
		/* take SAMPLE_SIZE samples */
		for (j = 0; j < SAMPLE_SIZE; j++) {
			hte_mem_op(address, mrc_params->first_run,
				   rcvn ? 0 : 1);
			mrc_params->first_run = 0;

			/*
			 * record the contents of the proper
			 * DQTRAINSTS register
			 */
			sampled_val[j] = msg_port_alt_read(DDRPHY,
				DQTRAINSTS +
				bl_grp * DDRIODQ_BL_OFFSET +
				channel * DDRIODQ_CH_OFFSET);
		}

		/*
		 * look for a majority value (SAMPLE_SIZE / 2) + 1
		 * on the byte lane and set that value in the corresponding
		 * ret_val bit
		 */
		for (bl = 0; bl < 2; bl++) {
			num_0s = 0x00;	/* reset '0' tracker for byte lane */
			num_1s = 0x00;	/* reset '1' tracker for byte lane */
			for (j = 0; j < SAMPLE_SIZE; j++) {
				if (sampled_val[j] & msk[bl])
					num_1s++;
				else
					num_0s++;
			}
		if (num_1s > num_0s)
			ret_val |= (1 << (bl + bl_grp * 2));
		}
	}

	/*
	 * "ret_val.0" contains the status of BL0
	 * "ret_val.1" contains the status of BL1
	 * "ret_val.2" contains the status of BL2
	 * etc.
	 */
	return ret_val;
}

/* This function will find the rising edge transition on RCVN or WDQS */
void find_rising_edge(struct mrc_params *mrc_params, uint32_t delay[],
		      uint8_t channel, uint8_t rank, bool rcvn)
{
	bool all_edges_found;	/* determines stop condition */
	bool direction[NUM_BYTE_LANES];	/* direction indicator */
	uint8_t sample;	/* sample counter */
	uint8_t bl;	/* byte lane counter */
	/* byte lane divisor */
	uint8_t bl_divisor = (mrc_params->channel_width == X16) ? 2 : 1;
	uint32_t sample_result[SAMPLE_CNT];	/* results of sample_dqs() */
	uint32_t temp;
	uint32_t transition_pattern;

	ENTERFN();

	/* select hte and request initial configuration */
	select_hte();
	mrc_params->first_run = 1;

	/* Take 3 sample points (T1,T2,T3) to obtain a transition pattern */
	for (sample = 0; sample < SAMPLE_CNT; sample++) {
		/* program the desired delays for sample */
		for (bl = 0; bl < (NUM_BYTE_LANES / bl_divisor); bl++) {
			/* increase sample delay by 26 PI (0.2 CLK) */
			if (rcvn) {
				set_rcvn(channel, rank, bl,
					 delay[bl] + sample * SAMPLE_DLY);
			} else {
				set_wdqs(channel, rank, bl,
					 delay[bl] + sample * SAMPLE_DLY);
			}
		}

		/* take samples (Tsample_i) */
		sample_result[sample] = sample_dqs(mrc_params,
			channel, rank, rcvn);

		DPF(D_TRN,
		    "Find rising edge %s ch%d rnk%d: #%d dly=%d dqs=%02X\n",
		    rcvn ? "RCVN" : "WDQS", channel, rank, sample,
		    sample * SAMPLE_DLY, sample_result[sample]);
	}

	/*
	 * This pattern will help determine where we landed and ultimately
	 * how to place RCVEN/WDQS.
	 */
	for (bl = 0; bl < NUM_BYTE_LANES / bl_divisor; bl++) {
		/* build transition_pattern (MSB is 1st sample) */
		transition_pattern = 0;
		for (sample = 0; sample < SAMPLE_CNT; sample++) {
			transition_pattern |=
				((sample_result[sample] & (1 << bl)) >> bl) <<
				(SAMPLE_CNT - 1 - sample);
		}

		DPF(D_TRN, "=== transition pattern %d\n", transition_pattern);

		/*
		 * set up to look for rising edge based on
		 * transition_pattern
		 */
		switch (transition_pattern) {
		case 0:	/* sampled 0->0->0 */
			/* move forward from T3 looking for 0->1 */
			delay[bl] += 2 * SAMPLE_DLY;
			direction[bl] = FORWARD;
			break;
		case 1:	/* sampled 0->0->1 */
		case 5:	/* sampled 1->0->1 (bad duty cycle) *HSD#237503* */
			/* move forward from T2 looking for 0->1 */
			delay[bl] += 1 * SAMPLE_DLY;
			direction[bl] = FORWARD;
			break;
		case 2:	/* sampled 0->1->0 (bad duty cycle) *HSD#237503* */
		case 3:	/* sampled 0->1->1 */
			/* move forward from T1 looking for 0->1 */
			delay[bl] += 0 * SAMPLE_DLY;
			direction[bl] = FORWARD;
			break;
		case 4:	/* sampled 1->0->0 (assumes BL8, HSD#234975) */
			/* move forward from T3 looking for 0->1 */
			delay[bl] += 2 * SAMPLE_DLY;
			direction[bl] = FORWARD;
			break;
		case 6:	/* sampled 1->1->0 */
		case 7:	/* sampled 1->1->1 */
			/* move backward from T1 looking for 1->0 */
			delay[bl] += 0 * SAMPLE_DLY;
			direction[bl] = BACKWARD;
			break;
		default:
			mrc_post_code(0xee, 0xee);
			break;
		}

		/* program delays */
		if (rcvn)
			set_rcvn(channel, rank, bl, delay[bl]);
		else
			set_wdqs(channel, rank, bl, delay[bl]);
	}

	/*
	 * Based on the observed transition pattern on the byte lane,
	 * begin looking for a rising edge with single PI granularity.
	 */
	do {
		all_edges_found = true;	/* assume all byte lanes passed */
		/* take a sample */
		temp = sample_dqs(mrc_params, channel, rank, rcvn);
		/* check all each byte lane for proper edge */
		for (bl = 0; bl < NUM_BYTE_LANES / bl_divisor; bl++) {
			if (temp & (1 << bl)) {
				/* sampled "1" */
				if (direction[bl] == BACKWARD) {
					/*
					 * keep looking for edge
					 * on this byte lane
					 */
					all_edges_found = false;
					delay[bl] -= 1;
					if (rcvn) {
						set_rcvn(channel, rank,
							 bl, delay[bl]);
					} else {
						set_wdqs(channel, rank,
							 bl, delay[bl]);
					}
				}
			} else {
				/* sampled "0" */
				if (direction[bl] == FORWARD) {
					/*
					 * keep looking for edge
					 * on this byte lane
					 */
					all_edges_found = false;
					delay[bl] += 1;
					if (rcvn) {
						set_rcvn(channel, rank,
							 bl, delay[bl]);
					} else {
						set_wdqs(channel, rank,
							 bl, delay[bl]);
					}
				}
			}
		}
	} while (!all_edges_found);

	/* restore DDR idle state */
	dram_init_command(DCMD_PREA(rank));

	DPF(D_TRN, "Delay %03X %03X %03X %03X\n",
	    delay[0], delay[1], delay[2], delay[3]);

	LEAVEFN();
}

/*
 * This function will return a 32 bit mask that will be used to
 * check for byte lane failures.
 */
uint32_t byte_lane_mask(struct mrc_params *mrc_params)
{
	uint32_t j;
	uint32_t ret_val = 0x00;

	/*
	 * set ret_val based on NUM_BYTE_LANES such that you will check
	 * only BL0 in result
	 *
	 * (each bit in result represents a byte lane)
	 */
	for (j = 0; j < MAX_BYTE_LANES; j += NUM_BYTE_LANES)
		ret_val |= (1 << ((j / NUM_BYTE_LANES) * NUM_BYTE_LANES));

	/*
	 * HSD#235037
	 * need to adjust the mask for 16-bit mode
	 */
	if (mrc_params->channel_width == X16)
		ret_val |= (ret_val << 2);

	return ret_val;
}

/*
 * Check memory executing simple write/read/verify at the specified address.
 *
 * Bits in the result indicate failure on specific byte lane.
 */
uint32_t check_rw_coarse(struct mrc_params *mrc_params, uint32_t address)
{
	uint32_t result = 0;
	uint8_t first_run = 0;

	if (mrc_params->hte_setup) {
		mrc_params->hte_setup = 0;
		first_run = 1;
		select_hte();
	}

	result = hte_basic_write_read(mrc_params, address, first_run,
				      WRITE_TRAIN);

	DPF(D_TRN, "check_rw_coarse result is %x\n", result);

	return result;
}

/*
 * Check memory executing write/read/verify of many data patterns
 * at the specified address. Bits in the result indicate failure
 * on specific byte lane.
 */
uint32_t check_bls_ex(struct mrc_params *mrc_params, uint32_t address)
{
	uint32_t result;
	uint8_t first_run = 0;

	if (mrc_params->hte_setup) {
		mrc_params->hte_setup = 0;
		first_run = 1;
		select_hte();
	}

	result = hte_write_stress_bit_lanes(mrc_params, address, first_run);

	DPF(D_TRN, "check_bls_ex result is %x\n", result);

	return result;
}

/*
 * 32-bit LFSR with characteristic polynomial: X^32 + X^22 +X^2 + X^1
 *
 * The function takes pointer to previous 32 bit value and
 * modifies it to next value.
 */
void lfsr32(uint32_t *lfsr_ptr)
{
	uint32_t bit;
	uint32_t lfsr;
	int i;

	lfsr = *lfsr_ptr;

	for (i = 0; i < 32; i++) {
		bit = 1 ^ (lfsr & 1);
		bit = bit ^ ((lfsr & 2) >> 1);
		bit = bit ^ ((lfsr & 4) >> 2);
		bit = bit ^ ((lfsr & 0x400000) >> 22);

		lfsr = ((lfsr >> 1) | (bit << 31));
	}

	*lfsr_ptr = lfsr;
}

/* Clear the pointers in a given byte lane in a given channel */
void clear_pointers(void)
{
	uint8_t channel;
	uint8_t bl;

	ENTERFN();

	for (channel = 0; channel < NUM_CHANNELS; channel++) {
		for (bl = 0; bl < NUM_BYTE_LANES; bl++) {
			mrc_alt_write_mask(DDRPHY,
					   B01PTRCTL1 +
					   channel * DDRIODQ_CH_OFFSET +
					   (bl >> 1) * DDRIODQ_BL_OFFSET,
					   ~(1 << 8), (1 << 8));

			mrc_alt_write_mask(DDRPHY,
					   B01PTRCTL1 +
					   channel * DDRIODQ_CH_OFFSET +
					   (bl >> 1) * DDRIODQ_BL_OFFSET,
					   (1 << 8), (1 << 8));
		}
	}

	LEAVEFN();
}

static void print_timings_internal(uint8_t algo, uint8_t channel, uint8_t rank,
				   uint8_t bl_divisor)
{
	uint8_t bl;

	switch (algo) {
	case RCVN:
		DPF(D_INFO, "\nRCVN[%02d:%02d]", channel, rank);
		break;
	case WDQS:
		DPF(D_INFO, "\nWDQS[%02d:%02d]", channel, rank);
		break;
	case WDQX:
		DPF(D_INFO, "\nWDQx[%02d:%02d]", channel, rank);
		break;
	case RDQS:
		DPF(D_INFO, "\nRDQS[%02d:%02d]", channel, rank);
		break;
	case VREF:
		DPF(D_INFO, "\nVREF[%02d:%02d]", channel, rank);
		break;
	case WCMD:
		DPF(D_INFO, "\nWCMD[%02d:%02d]", channel, rank);
		break;
	case WCTL:
		DPF(D_INFO, "\nWCTL[%02d:%02d]", channel, rank);
		break;
	case WCLK:
		DPF(D_INFO, "\nWCLK[%02d:%02d]", channel, rank);
		break;
	default:
		break;
	}

	for (bl = 0; bl < NUM_BYTE_LANES / bl_divisor; bl++) {
		switch (algo) {
		case RCVN:
			DPF(D_INFO, " %03d", get_rcvn(channel, rank, bl));
			break;
		case WDQS:
			DPF(D_INFO, " %03d", get_wdqs(channel, rank, bl));
			break;
		case WDQX:
			DPF(D_INFO, " %03d", get_wdq(channel, rank, bl));
			break;
		case RDQS:
			DPF(D_INFO, " %03d", get_rdqs(channel, rank, bl));
			break;
		case VREF:
			DPF(D_INFO, " %03d", get_vref(channel, bl));
			break;
		case WCMD:
			DPF(D_INFO, " %03d", get_wcmd(channel));
			break;
		case WCTL:
			DPF(D_INFO, " %03d", get_wctl(channel, rank));
			break;
		case WCLK:
			DPF(D_INFO, " %03d", get_wclk(channel, rank));
			break;
		default:
			break;
		}
	}
}

void print_timings(struct mrc_params *mrc_params)
{
	uint8_t algo;
	uint8_t channel;
	uint8_t rank;
	uint8_t bl_divisor = (mrc_params->channel_width == X16) ? 2 : 1;

	DPF(D_INFO, "\n---------------------------");
	DPF(D_INFO, "\nALGO[CH:RK] BL0 BL1 BL2 BL3");
	DPF(D_INFO, "\n===========================");

	for (algo = 0; algo < MAX_ALGOS; algo++) {
		for (channel = 0; channel < NUM_CHANNELS; channel++) {
			if (mrc_params->channel_enables & (1 << channel)) {
				for (rank = 0; rank < NUM_RANKS; rank++) {
					if (mrc_params->rank_enables &
						(1 << rank)) {
						print_timings_internal(algo,
							channel, rank,
							bl_divisor);
					}
				}
			}
		}
	}

	DPF(D_INFO, "\n---------------------------");
	DPF(D_INFO, "\n");
}
