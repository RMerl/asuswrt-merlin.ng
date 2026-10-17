// SPDX-License-Identifier: GPL-2.0-or-later OR BSD-3-Clause
/*
 * Copyright (C) 2022, STMicroelectronics - All Rights Reserved
 */

#define LOG_CATEGORY LOGC_ARCH

#include <common.h>
#include <log.h>
#include <syscon.h>
#include <asm/io.h>
#include <asm/arch/stm32.h>
#include <asm/arch/sys_proto.h>

/* SYSCFG register */
#define SYSCFG_IDC_OFFSET	0x380
#define SYSCFG_IDC_DEV_ID_MASK	GENMASK(11, 0)
#define SYSCFG_IDC_DEV_ID_SHIFT	0
#define SYSCFG_IDC_REV_ID_MASK	GENMASK(31, 16)
#define SYSCFG_IDC_REV_ID_SHIFT	16

/* Device Part Number (RPN) = OTP_DATA1 lower 11 bits */
#define RPN_SHIFT	0
#define RPN_MASK	GENMASK(11, 0)

static u32 read_idc(void)
{
	void *syscfg = syscon_get_first_range(STM32MP_SYSCON_SYSCFG);

	return readl(syscfg + SYSCFG_IDC_OFFSET);
}

u32 get_cpu_dev(void)
{
	return (read_idc() & SYSCFG_IDC_DEV_ID_MASK) >> SYSCFG_IDC_DEV_ID_SHIFT;
}

u32 get_cpu_rev(void)
{
	return (read_idc() & SYSCFG_IDC_REV_ID_MASK) >> SYSCFG_IDC_REV_ID_SHIFT;
}

/* Get Device Part Number (RPN) from OTP */
static u32 get_cpu_rpn(void)
{
	return get_otp(BSEC_OTP_RPN, RPN_SHIFT, RPN_MASK);
}

u32 get_cpu_type(void)
{
	return (get_cpu_dev() << 16) | get_cpu_rpn();
}

int get_eth_nb(void)
{
	int nb_eth = 2;

	switch (get_cpu_type()) {
	case CPU_STM32MP131Dxx:
		fallthrough;
	case CPU_STM32MP131Cxx:
		fallthrough;
	case CPU_STM32MP131Axx:
		nb_eth = 1;
		break;
	default:
		nb_eth = 2;
		break;
	}

	return nb_eth;
}

void get_soc_name(char name[SOC_NAME_SIZE])
{
	char *cpu_s, *cpu_r;

	/* MPUs Part Numbers */
	switch (get_cpu_type()) {
	case CPU_STM32MP135Fxx:
		cpu_s = "135F";
		break;
	case CPU_STM32MP135Dxx:
		cpu_s = "135D";
		break;
	case CPU_STM32MP135Cxx:
		cpu_s = "135C";
		break;
	case CPU_STM32MP135Axx:
		cpu_s = "135A";
		break;
	case CPU_STM32MP133Fxx:
		cpu_s = "133F";
		break;
	case CPU_STM32MP133Dxx:
		cpu_s = "133D";
		break;
	case CPU_STM32MP133Cxx:
		cpu_s = "133C";
		break;
	case CPU_STM32MP133Axx:
		cpu_s = "133A";
		break;
	case CPU_STM32MP131Fxx:
		cpu_s = "131F";
		break;
	case CPU_STM32MP131Dxx:
		cpu_s = "131D";
		break;
	case CPU_STM32MP131Cxx:
		cpu_s = "131C";
		break;
	case CPU_STM32MP131Axx:
		cpu_s = "131A";
		break;
	default:
		cpu_s = "????";
		break;
	}

	/* REVISION */
	switch (get_cpu_rev()) {
	case CPU_REV1:
		cpu_r = "A";
		break;
	case CPU_REV1_1:
		cpu_r = "Z";
		break;
	case CPU_REV1_2:
		cpu_r = "Y";
		break;
	default:
		cpu_r = "?";
		break;
	}

	snprintf(name, SOC_NAME_SIZE, "STM32MP%s Rev.%s", cpu_s, cpu_r);
}
