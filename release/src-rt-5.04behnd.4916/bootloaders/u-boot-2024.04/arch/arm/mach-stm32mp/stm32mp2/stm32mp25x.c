// SPDX-License-Identifier: GPL-2.0-or-later OR BSD-3-Clause
/*
 * Copyright (C) 2023, STMicroelectronics - All Rights Reserved
 */

#define LOG_CATEGORY LOGC_ARCH

#include <log.h>
#include <syscon.h>
#include <asm/io.h>
#include <asm/arch/stm32.h>
#include <asm/arch/sys_proto.h>

/* SYSCFG register */
#define SYSCFG_DEVICEID_OFFSET		0x6400
#define SYSCFG_DEVICEID_DEV_ID_MASK	GENMASK(11, 0)
#define SYSCFG_DEVICEID_DEV_ID_SHIFT	0
#define SYSCFG_DEVICEID_REV_ID_MASK	GENMASK(31, 16)
#define SYSCFG_DEVICEID_REV_ID_SHIFT	16

/* Device Part Number (RPN) = OTP9 */
#define RPN_SHIFT	0
#define RPN_MASK	GENMASK(31, 0)

/* Package = bit 0:2 of OTP122 => STM32MP25_PKG defines
 * - 000: Custom package
 * - 011: TFBGA361 => AL = 10x10, 361 balls pith 0.5mm
 * - 100: TFBGA424 => AK = 14x14, 424 balls pith 0.5mm
 * - 101: TFBGA436 => AI = 18x18, 436 balls pith 0.5mm
 * - others: Reserved
 */
#define PKG_SHIFT	0
#define PKG_MASK	GENMASK(2, 0)

static u32 read_deviceid(void)
{
	void *syscfg = syscon_get_first_range(STM32MP_SYSCON_SYSCFG);

	return readl(syscfg + SYSCFG_DEVICEID_OFFSET);
}

u32 get_cpu_dev(void)
{
	return (read_deviceid() & SYSCFG_DEVICEID_DEV_ID_MASK) >> SYSCFG_DEVICEID_DEV_ID_SHIFT;
}

u32 get_cpu_rev(void)
{
	return (read_deviceid() & SYSCFG_DEVICEID_REV_ID_MASK) >> SYSCFG_DEVICEID_REV_ID_SHIFT;
}

/* Get Device Part Number (RPN) from OTP */
u32 get_cpu_type(void)
{
	return get_otp(BSEC_OTP_RPN, RPN_SHIFT, RPN_MASK);
}

/* Get Package options from OTP */
u32 get_cpu_package(void)
{
	return get_otp(BSEC_OTP_PKG, PKG_SHIFT, PKG_MASK);
}

int get_eth_nb(void)
{
	int nb_eth;

	switch (get_cpu_type()) {
	case CPU_STM32MP257Fxx:
		fallthrough;
	case CPU_STM32MP257Dxx:
		fallthrough;
	case CPU_STM32MP257Cxx:
		fallthrough;
	case CPU_STM32MP257Axx:
		nb_eth = 5; /* dual ETH with TSN support */
		break;
	case CPU_STM32MP253Fxx:
		fallthrough;
	case CPU_STM32MP253Dxx:
		fallthrough;
	case CPU_STM32MP253Cxx:
		fallthrough;
	case CPU_STM32MP253Axx:
		nb_eth = 2; /* dual ETH */
		break;
	case CPU_STM32MP251Fxx:
		fallthrough;
	case CPU_STM32MP251Dxx:
		fallthrough;
	case CPU_STM32MP251Cxx:
		fallthrough;
	case CPU_STM32MP251Axx:
		nb_eth = 1; /* single ETH */
		break;
	default:
		nb_eth = 0;
		break;
	}

	return nb_eth;
}

void get_soc_name(char name[SOC_NAME_SIZE])
{
	char *cpu_s, *cpu_r, *package;

	cpu_s = "????";
	cpu_r = "?";
	package = "??";
	if (get_cpu_dev() == CPU_DEV_STM32MP25) {
		switch (get_cpu_type()) {
		case CPU_STM32MP257Fxx:
			cpu_s = "257F";
			break;
		case CPU_STM32MP257Dxx:
			cpu_s = "257D";
			break;
		case CPU_STM32MP257Cxx:
			cpu_s = "257C";
			break;
		case CPU_STM32MP257Axx:
			cpu_s = "257A";
			break;
		case CPU_STM32MP255Fxx:
			cpu_s = "255F";
			break;
		case CPU_STM32MP255Dxx:
			cpu_s = "255D";
			break;
		case CPU_STM32MP255Cxx:
			cpu_s = "255C";
			break;
		case CPU_STM32MP255Axx:
			cpu_s = "255A";
			break;
		case CPU_STM32MP253Fxx:
			cpu_s = "253F";
			break;
		case CPU_STM32MP253Dxx:
			cpu_s = "253D";
			break;
		case CPU_STM32MP253Cxx:
			cpu_s = "253C";
			break;
		case CPU_STM32MP253Axx:
			cpu_s = "253A";
			break;
		case CPU_STM32MP251Fxx:
			cpu_s = "251F";
			break;
		case CPU_STM32MP251Dxx:
			cpu_s = "251D";
			break;
		case CPU_STM32MP251Cxx:
			cpu_s = "251C";
			break;
		case CPU_STM32MP251Axx:
			cpu_s = "251A";
			break;
		default:
			cpu_s = "25??";
			break;
		}
		/* REVISION */
		switch (get_cpu_rev()) {
		case CPU_REV1:
			cpu_r = "A";
			break;
		case CPU_REV2:
			cpu_r = "B";
			break;
		default:
			break;
		}
		/* PACKAGE */
		switch (get_cpu_package()) {
		case STM32MP25_PKG_CUSTOM:
			package = "XX";
			break;
		case STM32MP25_PKG_AL_TBGA361:
			package = "AL";
			break;
		case STM32MP25_PKG_AK_TBGA424:
			package = "AK";
			break;
		case STM32MP25_PKG_AI_TBGA436:
			package = "AI";
			break;
		default:
			break;
		}
	}

	snprintf(name, SOC_NAME_SIZE, "STM32MP%s%s Rev.%s", cpu_s, package, cpu_r);
}
