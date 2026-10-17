/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2013 Google Inc.
 * Copyright (C) 2016 Bin Meng <bmeng.cn@gmail.com>
 *
 * Modified from coreboot src/soc/intel/baytrail/include/soc/irq.h
 */

#ifndef _BAYTRAIL_IRQ_H_
#define _BAYTRAIL_IRQ_H_

#define PIRQA_APIC_IRQ			16
#define PIRQB_APIC_IRQ			17
#define PIRQC_APIC_IRQ			18
#define PIRQD_APIC_IRQ			19
#define PIRQE_APIC_IRQ			20
#define PIRQF_APIC_IRQ			21
#define PIRQG_APIC_IRQ			22
#define PIRQH_APIC_IRQ			23

/* The below IRQs are for when devices are in ACPI mode */
#define LPE_DMA0_IRQ			24
#define LPE_DMA1_IRQ			25
#define LPE_SSP0_IRQ			26
#define LPE_SSP1_IRQ			27
#define LPE_SSP2_IRQ			28
#define LPE_IPC2HOST_IRQ		29
#define LPSS_I2C1_IRQ			32
#define LPSS_I2C2_IRQ			33
#define LPSS_I2C3_IRQ			34
#define LPSS_I2C4_IRQ			35
#define LPSS_I2C5_IRQ			36
#define LPSS_I2C6_IRQ			37
#define LPSS_I2C7_IRQ			38
#define LPSS_HSUART1_IRQ		39
#define LPSS_HSUART2_IRQ		40
#define LPSS_SPI_IRQ			41
#define LPSS_DMA1_IRQ			42
#define LPSS_DMA2_IRQ			43
#define SCC_EMMC_IRQ			44
#define SCC_SDIO_IRQ			46
#define SCC_SD_IRQ			47
#define GPIO_NC_IRQ			48
#define GPIO_SC_IRQ			49
#define GPIO_SUS_IRQ			50
/* GPIO direct / dedicated IRQs */
#define GPIO_S0_DED_IRQ_0		51
#define GPIO_S0_DED_IRQ_1		52
#define GPIO_S0_DED_IRQ_2		53
#define GPIO_S0_DED_IRQ_3		54
#define GPIO_S0_DED_IRQ_4		55
#define GPIO_S0_DED_IRQ_5		56
#define GPIO_S0_DED_IRQ_6		57
#define GPIO_S0_DED_IRQ_7		58
#define GPIO_S0_DED_IRQ_8		59
#define GPIO_S0_DED_IRQ_9		60
#define GPIO_S0_DED_IRQ_10		61
#define GPIO_S0_DED_IRQ_11		62
#define GPIO_S0_DED_IRQ_12		63
#define GPIO_S0_DED_IRQ_13		64
#define GPIO_S0_DED_IRQ_14		65
#define GPIO_S0_DED_IRQ_15		66
#define GPIO_S5_DED_IRQ_0		67
#define GPIO_S5_DED_IRQ_1		68
#define GPIO_S5_DED_IRQ_2		69
#define GPIO_S5_DED_IRQ_3		70
#define GPIO_S5_DED_IRQ_4		71
#define GPIO_S5_DED_IRQ_5		72
#define GPIO_S5_DED_IRQ_6		73
#define GPIO_S5_DED_IRQ_7		74
#define GPIO_S5_DED_IRQ_8		75
#define GPIO_S5_DED_IRQ_9		76
#define GPIO_S5_DED_IRQ_10		77
#define GPIO_S5_DED_IRQ_11		78
#define GPIO_S5_DED_IRQ_12		79
#define GPIO_S5_DED_IRQ_13		80
#define GPIO_S5_DED_IRQ_14		81
#define GPIO_S5_DED_IRQ_15		82
/* DIRQs - Two levels of expansion to evaluate to numeric constants for ASL */
#define _GPIO_S0_DED_IRQ(slot)		GPIO_S0_DED_IRQ_##slot
#define _GPIO_S5_DED_IRQ(slot)		GPIO_S5_DED_IRQ_##slot
#define GPIO_S0_DED_IRQ(slot)		_GPIO_S0_DED_IRQ(slot)
#define GPIO_S5_DED_IRQ(slot)		_GPIO_S5_DED_IRQ(slot)

#endif /* _BAYTRAIL_IRQ_H_ */
