// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018 NXP
 */

#include <common.h>
#include <errno.h>
#include <log.h>
#include <asm/io.h>
#include <asm/arch/ddr.h>
#include <asm/arch/clock.h>
#include <asm/arch/ddr.h>
#include <asm/arch/sys_proto.h>

static inline void poll_pmu_message_ready(void)
{
	unsigned int reg;

	do {
		reg = reg32_read(IP2APB_DDRPHY_IPS_BASE_ADDR(0) + ddrphy_addr_remap(0xd0004));
	} while (reg & 0x1);
}

static inline void ack_pmu_message_receive(void)
{
	unsigned int reg;

	reg32_write(IP2APB_DDRPHY_IPS_BASE_ADDR(0) + ddrphy_addr_remap(0xd0031), 0x0);

	do {
		reg = reg32_read(IP2APB_DDRPHY_IPS_BASE_ADDR(0) + ddrphy_addr_remap(0xd0004));
	} while (!(reg & 0x1));

	reg32_write(IP2APB_DDRPHY_IPS_BASE_ADDR(0) + ddrphy_addr_remap(0xd0031), 0x1);
}

static inline unsigned int get_mail(void)
{
	unsigned int reg;

	poll_pmu_message_ready();

	reg = reg32_read(IP2APB_DDRPHY_IPS_BASE_ADDR(0) + ddrphy_addr_remap(0xd0032));

	ack_pmu_message_receive();

	return reg;
}

static inline unsigned int get_stream_message(void)
{
	unsigned int reg, reg2;

	poll_pmu_message_ready();

	reg = reg32_read(IP2APB_DDRPHY_IPS_BASE_ADDR(0) + ddrphy_addr_remap(0xd0032));

	reg2 = reg32_read(IP2APB_DDRPHY_IPS_BASE_ADDR(0) + ddrphy_addr_remap(0xd0034));

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

int wait_ddrphy_training_complete(void)
{
	unsigned int mail;

	while (1) {
		mail = get_mail();
		decode_major_message(mail);
		if (mail == 0x08) {
			decode_streaming_message();
		} else if (mail == 0x07) {
			debug("Training PASS\n");
			return 0;
		} else if (mail == 0xff) {
			printf("Training FAILED\n");
			return -1;
		}
	}
}

void ddrphy_init_set_dfi_clk(unsigned int drate)
{
	switch (drate) {
	case 4000:
		dram_pll_init(MHZ(1000));
		dram_disable_bypass();
		break;
	case 3734:
	case 3733:
	case 3732:
		dram_pll_init(MHZ(933));
		dram_disable_bypass();
		break;
	case 3600:
		dram_pll_init(MHZ(900));
		dram_disable_bypass();
		break;
	case 3200:
		dram_pll_init(MHZ(800));
		dram_disable_bypass();
		break;
	case 3000:
		dram_pll_init(MHZ(750));
		dram_disable_bypass();
		break;
	case 2800:
		dram_pll_init(MHZ(700));
		dram_disable_bypass();
		break;
	case 2400:
		dram_pll_init(MHZ(600));
		dram_disable_bypass();
		break;
	case 1866:
		dram_pll_init(MHZ(466));
		dram_disable_bypass();
		break;
	case 1600:
		dram_pll_init(MHZ(400));
		dram_disable_bypass();
		break;
	case 1066:
		dram_pll_init(MHZ(266));
		dram_disable_bypass();
		break;
	case 667:
		dram_pll_init(MHZ(167));
		dram_disable_bypass();
		break;
	case 625:
		dram_enable_bypass(MHZ(625));
		break;
	case 400:
		dram_enable_bypass(MHZ(400));
		break;
	case 333:
		dram_enable_bypass(MHZ(333));
		break;
	case 200:
		dram_enable_bypass(MHZ(200));
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
