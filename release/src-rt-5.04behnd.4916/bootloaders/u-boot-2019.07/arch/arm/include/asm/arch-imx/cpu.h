/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2014 Freescale Semiconductor, Inc.
 */

#define MXC_CPU_MX23		0x23
#define MXC_CPU_MX25		0x25
#define MXC_CPU_MX27		0x27
#define MXC_CPU_MX28		0x28
#define MXC_CPU_MX31		0x31
#define MXC_CPU_MX35		0x35
#define MXC_CPU_MX51		0x51
#define MXC_CPU_MX53		0x53
#define MXC_CPU_MX6SL		0x60
#define MXC_CPU_MX6DL		0x61
#define MXC_CPU_MX6SX		0x62
#define MXC_CPU_MX6Q		0x63
#define MXC_CPU_MX6UL		0x64
#define MXC_CPU_MX6ULL		0x65
#define MXC_CPU_MX6SOLO		0x66 /* dummy */
#define MXC_CPU_MX6SLL		0x67
#define MXC_CPU_MX6D		0x6A
#define MXC_CPU_MX6DP		0x68
#define MXC_CPU_MX6QP		0x69
#define MXC_CPU_MX7S		0x71 /* dummy ID */
#define MXC_CPU_MX7D		0x72
#define MXC_CPU_IMX8MQ		0x82
#define MXC_CPU_IMX8QXP_A0	0x90 /* dummy ID */
#define MXC_CPU_IMX8QM		0x91 /* dummy ID */
#define MXC_CPU_IMX8QXP		0x92 /* dummy ID */
#define MXC_CPU_MX7ULP		0xE1 /* Temporally hard code */
#define MXC_CPU_VF610		0xF6 /* dummy ID */

#define MXC_SOC_MX6		0x60
#define MXC_SOC_MX7		0x70
#define MXC_SOC_IMX8M		0x80
#define MXC_SOC_IMX8		0x90 /* dummy */
#define MXC_SOC_MX7ULP		0xE0 /* dummy */

#define CHIP_REV_1_0            0x10
#define CHIP_REV_1_1            0x11
#define CHIP_REV_1_2            0x12
#define CHIP_REV_1_5            0x15
#define CHIP_REV_2_0            0x20
#define CHIP_REV_2_1            0x21
#define CHIP_REV_2_5            0x25
#define CHIP_REV_3_0            0x30

#define CHIP_REV_A		0x0
#define CHIP_REV_B		0x1

#define BOARD_REV_1_0           0x0
#define BOARD_REV_2_0           0x1
#define BOARD_VER_OFFSET        0x8

#define CS0_128					0
#define CS0_64M_CS1_64M				1
#define CS0_64M_CS1_32M_CS2_32M			2
#define CS0_32M_CS1_32M_CS2_32M_CS3_32M		3

u32 get_imx_reset_cause(void);
ulong get_systemPLLCLK(void);
ulong get_FCLK(void);
ulong get_HCLK(void);
ulong get_BCLK(void);
ulong get_PERCLK1(void);
ulong get_PERCLK2(void);
ulong get_PERCLK3(void);
