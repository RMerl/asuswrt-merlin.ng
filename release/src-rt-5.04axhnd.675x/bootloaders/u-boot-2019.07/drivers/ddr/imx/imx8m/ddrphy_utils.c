// SPDX-License-Identifier: GPL-2.0+
/*
* Copyright 2018 NXP
*/

#include <common.h>
#include <errno.h>
#include <asm/io.h>
#include <asm/arch/ddr.h>
#include <asm/arch/clock.h>
#include <asm/arch/ddr.h>
#include <asm/arch/lpddr4_define.h>

static inline void poll_pmu_message_ready(void)
{
	unsigned int reg;

	do {
		reg = reg32_read(IP2APB_DDRPHY_IPS_BASE_ADDR(0) + 4 * 0xd0004);
	} while (reg & 0x1);
}

static inline void ack_pmu_message_receive(void)
{
	unsigned int reg;

	reg32_write(IP2APB_DDRPHY_IPS_BASE_ADDR(0) + 4 * 0xd0031, 0x0);

	do {
		reg = reg32_read(IP2APB_DDRPHY_IPS_BASE_ADDR(0) + 4 * 0xd0004);
	} while (!(reg & 0x1));

	reg32_write(IP2APB_DDRPHY_IPS_BASE_ADDR(0) + 4 * 0xd0031, 0x1);
}

static inline unsigned int get_mail(void)
{
	unsigned int reg;

	poll_pmu_message_ready();

	reg = reg32_read(IP2APB_DDRPHY_IPS_BASE_ADDR(0) + 4 * 0xd0032);

	ack_pmu_message_receive();

	return reg;
}

static inline unsigned int get_stream_message(void)
{
	unsigned int reg, reg2;

	poll_pmu_message_ready();

	reg = reg32_read(IP2APB_DDRPHY_IPS_BASE_ADDR(0) + 4 * 0xd0032);

	reg2 = reg32_read(IP2APB_DDRPHY_IPS_BASE_ADDR(0) + 4 * 0xd0034);

	reg2 = (reg2 << 16) | reg;

	ack_pmu_message_receive();

	return reg2;
}

static inline void decode_major_message(unsigned int mail)
{
	debug("[PMU Major message = 0x%08x]\n", mail);
}

static inline void decode_streaming_message(void)
{
	unsigned int string_index, arg __maybe_unused;
	int i = 0;

	string_index = get_stream_message();
	debug("PMU String index = 0x%08x\n", string_index);
	while (i < (string_index & 0xffff)) {
		arg = get_stream_message();
		debug("arg[%d] = 0x%08x\n", i, arg);
		i++;
	}

	debug("\n");
}

void wait_ddrphy_training_complete(void)
{
	unsigned int mail;

	while (1) {
		mail = get_mail();
		decode_major_message(mail);
		if (mail == 0x08) {
			decode_streaming_message();
		} else if (mail == 0x07) {
			debug("Training PASS\n");
			break;
		} else if (mail == 0xff) {
			printf("Training FAILED\n");
			break;
		}
	}
}

void ddrphy_init_set_dfi_clk(unsigned int drate)
{
	switch (drate) {
	case 3200:
		dram_pll_init(MHZ(800));
		dram_disable_bypass();
		break;
	case 3000:
		dram_pll_init(MHZ(750));
		dram_disable_bypass();
		break;
	case 2400:
		dram_pll_init(MHZ(600));
		dram_disable_bypass();
		break;
	case 1600:
		dram_pll_init(MHZ(400));
		dram_disable_bypass();
		break;
	case 667:
		dram_pll_init(MHZ(167));
		dram_disable_bypass();
		break;
	case 400:
		dram_enable_bypass(MHZ(400));
		break;
	case 100:
		dram_enable_bypass(MHZ(100));
		break;
	default:
		return;
	}
}

void ddrphy_init_read_msg_block(enum fw_type type)
{
}

void lpddr4_mr_write(unsigned int mr_rank, unsigned int mr_addr,
		     unsigned int mr_data)
{
	unsigned int tmp;
	/*
	 * 1. Poll MRSTAT.mr_wr_busy until it is 0.
	 * This checks that there is no outstanding MR transaction.
	 * No writes should be performed to MRCTRL0 and MRCTRL1 if
	 * MRSTAT.mr_wr_busy = 1.
	 */
	do {
		tmp = reg32_read(DDRC_MRSTAT(0));
	} while (tmp & 0x1);
	/*
	 * 2. Write the MRCTRL0.mr_type, MRCTRL0.mr_addr, MRCTRL0.mr_rank and
	 * (for MRWs) MRCTRL1.mr_data to define the MR transaction.
	 */
	reg32_write(DDRC_MRCTRL0(0), (mr_rank << 4));
	reg32_write(DDRC_MRCTRL1(0), (mr_addr << 8) | mr_data);
	reg32setbit(DDRC_MRCTRL0(0), 31);
}

unsigned int lpddr4_mr_read(unsigned int mr_rank, unsigned int mr_addr)
{
	unsigned int tmp;

	reg32_write(DRC_PERF_MON_MRR0_DAT(0), 0x1);
	do {
		tmp = reg32_read(DDRC_MRSTAT(0));
	} while (tmp & 0x1);

	reg32_write(DDRC_MRCTRL0(0), (mr_rank << 4) | 0x1);
	reg32_write(DDRC_MRCTRL1(0), (mr_addr << 8));
	reg32setbit(DDRC_MRCTRL0(0), 31);
	do {
		tmp = reg32_read(DRC_PERF_MON_MRR0_DAT(0));
	} while ((tmp & 0x8) == 0);
	tmp = reg32_read(DRC_PERF_MON_MRR1_DAT(0));
	tmp = tmp & 0xff;
	reg32_write(DRC_PERF_MON_MRR0_DAT(0), 0x4);

	return tmp;
}
