/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Definitions for the GPIO subsystem on Apollolake
 *
 * Copyright (C) 2015 - 2017 Intel Corp.
 * (Written by Alexandru Gagniuc <alexandrux.gagniuc@intel.com> for Intel Corp.)
 *
 * Placed in a separate file since some of these definitions can be used from
 * assembly code
 *
 * Taken from gpio_apl.h in coreboot
 */

#ifndef _ASM_ARCH_GPIO_H_
#define _ASM_ARCH_GPIO_H_

/* Port ids */
#define PID_GPIO_SW	0xC0
#define PID_GPIO_S	0xC2
#define PID_GPIO_W	0xC7
#define PID_GPIO_NW	0xC4
#define PID_GPIO_N	0xC5
#define PID_ITSS	0xD0
#define PID_RTC		0xD1

/*
 * Miscellaneous Configuration register(MISCCFG). These are community-specific
 * registers and are meant to house miscellaneous configuration fields per
 * community. There are 8 GPIO groups: GPP_0 -> GPP_8 (Group 3 is absent)
 */
#define GPIO_MISCCFG		0x10 /* Miscellaneous Configuration offset */
#define  GPIO_GPE_SW_31_0	0 /* SOUTHWEST GPIO#  0 ~ 31 belong to GROUP0 */
#define  GPIO_GPE_SW_63_32	1 /* SOUTHWEST GPIO# 32 ~ 42 belong to GROUP1 */
#define  GPIO_GPE_W_31_0	2 /* WEST      GPIO#  0 ~ 25 belong to GROUP2 */
#define  GPIO_GPE_NW_31_0	4 /* NORTHWEST GPIO#  0 ~ 17 belong to GROUP4 */
#define  GPIO_GPE_NW_63_32	5 /* NORTHWEST GPIO# 32 ~ 63 belong to GROUP5 */
#define  GPIO_GPE_NW_95_64	6 /* NORTHWEST GPIO# 64 ~ 76 belong to GROUP6 */
#define  GPIO_GPE_N_31_0	7 /* NORTH     GPIO#  0 ~ 31 belong to GROUP7 */
#define  GPIO_GPE_N_63_32	8 /* NORTH     GPIO# 32 ~ 61 belong to GROUP8 */

#define GPIO_MAX_NUM_PER_GROUP	32

/*
 * Host Software Pad Ownership Register.
 * The pins in the community are divided into 3 groups:
 * GPIO 0 ~ 31, GPIO 32 ~ 63, GPIO 64 ~ 95
 */
#define HOSTSW_OWN_REG_0		0x80

#define PAD_CFG_BASE			0x500

#define GPI_INT_STS_0			0x100
#define GPI_INT_EN_0			0x110

#define GPI_SMI_STS_0			0x140
#define GPI_SMI_EN_0			0x150

#define NUM_N_PADS			(PAD_N(SVID0_CLK) + 1)
#define NUM_NW_PADS			(PAD_NW(GPIO_123) + 1)
#define NUM_W_PADS			(PAD_W(SUSPWRDNACK) + 1)
#define NUM_SW_PADS			(PAD_SW(LPC_FRAMEB) + 1)

#define NUM_N_GPI_REGS	\
	(ALIGN(NUM_N_PADS, GPIO_MAX_NUM_PER_GROUP) / GPIO_MAX_NUM_PER_GROUP)

#define NUM_NW_GPI_REGS	\
	(ALIGN(NUM_NW_PADS, GPIO_MAX_NUM_PER_GROUP) / GPIO_MAX_NUM_PER_GROUP)

#define NUM_W_GPI_REGS	\
	(ALIGN(NUM_W_PADS, GPIO_MAX_NUM_PER_GROUP) / GPIO_MAX_NUM_PER_GROUP)

#define NUM_SW_GPI_REGS	\
	(ALIGN(NUM_SW_PADS, GPIO_MAX_NUM_PER_GROUP) / GPIO_MAX_NUM_PER_GROUP)

/*
 * Total number of GPI status registers across all GPIO communities in the SOC
 */
#define NUM_GPI_STATUS_REGS		(NUM_N_GPI_REGS + NUM_NW_GPI_REGS \
					+ NUM_W_GPI_REGS + NUM_SW_GPI_REGS)

/* North community pads */
#define GPIO_0				0
#define GPIO_1				1
#define GPIO_2				2
#define GPIO_3				3
#define GPIO_4				4
#define GPIO_5				5
#define GPIO_6				6
#define GPIO_7				7
#define GPIO_8				8
#define GPIO_9				9
#define GPIO_10				10
#define GPIO_11				11
#define GPIO_12				12
#define GPIO_13				13
#define GPIO_14				14
#define GPIO_15				15
#define GPIO_16				16
#define GPIO_17				17
#define GPIO_18				18
#define GPIO_19				19
#define GPIO_20				20
#define GPIO_21				21
#define GPIO_22				22
#define GPIO_23				23
#define GPIO_24				24
#define GPIO_25				25
#define GPIO_26				26
#define GPIO_27				27
#define GPIO_28				28
#define GPIO_29				29
#define GPIO_30				30
#define GPIO_31				31
#define GPIO_32				32
#define GPIO_33				33
#define GPIO_34				34
#define GPIO_35				35
#define GPIO_36				36
#define GPIO_37				37
#define GPIO_38				38
#define GPIO_39				39
#define GPIO_40				40
#define GPIO_41				41
#define GPIO_42				42
#define GPIO_43				43
#define GPIO_44				44
#define GPIO_45				45
#define GPIO_46				46
#define GPIO_47				47
#define GPIO_48				48
#define GPIO_49				49
#define GPIO_62				50
#define GPIO_63				51
#define GPIO_64				52
#define GPIO_65				53
#define GPIO_66				54
#define GPIO_67				55
#define GPIO_68				56
#define GPIO_69				57
#define GPIO_70				58
#define GPIO_71				59
#define GPIO_72				60
#define GPIO_73				61
#define JTAG_TCK			62
#define JTAG_TRST_B			63
#define JTAG_TMS			64
#define JTAG_TDI			65
#define JTAG_CX_PMODE			66
#define JTAG_CX_PREQ_B			67
#define JTAGX				68
#define JTAG_CX_PRDY_B			69
#define JTAG_TDO			70
#define CNV_BRI_DT			71
#define CNV_BRI_RSP			72
#define CNV_RGI_DT			73
#define CNV_RGI_RSP			74
#define SVID0_ALERT_B			75
#define SVID0_DATA			76
#define SVID0_CLK			77

/* Northwest community pads */
#define GPIO_187			78
#define GPIO_188			79
#define GPIO_189			80
#define GPIO_190			81
#define GPIO_191			82
#define GPIO_192			83
#define GPIO_193			84
#define GPIO_194			85
#define GPIO_195			86
#define GPIO_196			87
#define GPIO_197			88
#define GPIO_198			89
#define GPIO_199			90
#define GPIO_200			91
#define GPIO_201			92
#define GPIO_202			93
#define GPIO_203			94
#define GPIO_204			95
#define PMC_SPI_FS0			96
#define PMC_SPI_FS1			97
#define PMC_SPI_FS2			98
#define PMC_SPI_RXD			99
#define PMC_SPI_TXD			100
#define PMC_SPI_CLK			101
#define PMIC_PWRGOOD			102
#define PMIC_RESET_B			103
#define GPIO_213			104
#define GPIO_214			105
#define GPIO_215			106
#define PMIC_THERMTRIP_B		107
#define PMIC_STDBY			108
#define PROCHOT_B			109
#define PMIC_I2C_SCL			110
#define PMIC_I2C_SDA			111
#define GPIO_74				112
#define GPIO_75				113
#define GPIO_76				114
#define GPIO_77				115
#define GPIO_78				116
#define GPIO_79				117
#define GPIO_80				118
#define GPIO_81				119
#define GPIO_82				120
#define GPIO_83				121
#define GPIO_84				122
#define GPIO_85				123
#define GPIO_86				124
#define GPIO_87				125
#define GPIO_88				126
#define GPIO_89				127
#define GPIO_90				128
#define GPIO_91				129
#define GPIO_92				130
#define GPIO_97				131
#define GPIO_98				132
#define GPIO_99				133
#define GPIO_100			134
#define GPIO_101			135
#define GPIO_102			136
#define GPIO_103			137
#define FST_SPI_CLK_FB			138
#define GPIO_104			139
#define GPIO_105			140
#define GPIO_106			141
#define GPIO_109			142
#define GPIO_110			143
#define GPIO_111			144
#define GPIO_112			145
#define GPIO_113			146
#define GPIO_116			147
#define GPIO_117			148
#define GPIO_118			149
#define GPIO_119			150
#define GPIO_120			151
#define GPIO_121			152
#define GPIO_122			153
#define GPIO_123			154

/* West community pads */
#define GPIO_124			155
#define GPIO_125			156
#define GPIO_126			157
#define GPIO_127			158
#define GPIO_128			159
#define GPIO_129			160
#define GPIO_130			161
#define GPIO_131			162
#define GPIO_132			163
#define GPIO_133			164
#define GPIO_134			165
#define GPIO_135			166
#define GPIO_136			167
#define GPIO_137			168
#define GPIO_138			169
#define GPIO_139			170
#define GPIO_146			171
#define GPIO_147			172
#define GPIO_148			173
#define GPIO_149			174
#define GPIO_150			175
#define GPIO_151			176
#define GPIO_152			177
#define GPIO_153			178
#define GPIO_154			179
#define GPIO_155			180
#define GPIO_209			181
#define GPIO_210			182
#define GPIO_211			183
#define GPIO_212			184
#define OSC_CLK_OUT_0			185
#define OSC_CLK_OUT_1			186
#define OSC_CLK_OUT_2			187
#define OSC_CLK_OUT_3			188
#define OSC_CLK_OUT_4			189
#define PMU_AC_PRESENT			190
#define PMU_BATLOW_B			191
#define PMU_PLTRST_B			192
#define PMU_PWRBTN_B			193
#define PMU_RESETBUTTON_B		194
#define PMU_SLP_S0_B			195
#define PMU_SLP_S3_B			196
#define PMU_SLP_S4_B			197
#define PMU_SUSCLK			198
#define PMU_WAKE_B			199
#define SUS_STAT_B			200
#define SUSPWRDNACK			201

/* Southwest community pads */
#define GPIO_205			202
#define GPIO_206			203
#define GPIO_207			204
#define GPIO_208			205
#define GPIO_156			206
#define GPIO_157			207
#define GPIO_158			208
#define GPIO_159			209
#define GPIO_160			210
#define GPIO_161			211
#define GPIO_162			212
#define GPIO_163			213
#define GPIO_164			214
#define GPIO_165			215
#define GPIO_166			216
#define GPIO_167			217
#define GPIO_168			218
#define GPIO_169			219
#define GPIO_170			220
#define GPIO_171			221
#define GPIO_172			222
#define GPIO_179			223
#define GPIO_173			224
#define GPIO_174			225
#define GPIO_175			226
#define GPIO_176			227
#define GPIO_177			228
#define GPIO_178			229
#define GPIO_186			230
#define GPIO_182			231
#define GPIO_183			232
#define SMB_ALERTB			233
#define SMB_CLK				234
#define SMB_DATA			235
#define LPC_ILB_SERIRQ			236
#define LPC_CLKOUT0			237
#define LPC_CLKOUT1			238
#define LPC_AD0				239
#define LPC_AD1				240
#define LPC_AD2				241
#define LPC_AD3				242
#define LPC_CLKRUNB			243
#define LPC_FRAMEB			244

/* PERST_0 not defined */
#define GPIO_PRT0_UDEF			0xFF

#define TOTAL_PADS			245
#define N_OFFSET			GPIO_0
#define NW_OFFSET			GPIO_187
#define W_OFFSET			GPIO_124
#define SW_OFFSET			GPIO_205

/* Macros for translating a global pad offset to a local offset */
#define PAD_N(pad)			(pad - N_OFFSET)
#define PAD_NW(pad)			(pad - NW_OFFSET)
#define PAD_W(pad)			(pad - W_OFFSET)
#define PAD_SW(pad)			(pad - SW_OFFSET)

/* Linux names of the GPIO devices */
#define GPIO_COMM_N_NAME		"INT3452:00"
#define GPIO_COMM_NW_NAME		"INT3452:01"
#define GPIO_COMM_W_NAME		"INT3452:02"
#define GPIO_COMM_SW_NAME		"INT3452:03"

/* Following is used in gpio asl */
#define GPIO_COMM_NAME			"INT3452"
#define GPIO_COMM_0_DESC	\
	"General Purpose Input/Output (GPIO) Controller - North"
#define GPIO_COMM_1_DESC	\
	"General Purpose Input/Output (GPIO) Controller - Northwest"
#define GPIO_COMM_2_DESC	\
	"General Purpose Input/Output (GPIO) Controller - West"
#define GPIO_COMM_3_DESC	\
	"General Purpose Input/Output (GPIO) Controller - Southwest"

#define GPIO_COMM0_PID			PID_GPIO_N
#define GPIO_COMM1_PID			PID_GPIO_NW
#define GPIO_COMM2_PID			PID_GPIO_W
#define GPIO_COMM3_PID			PID_GPIO_SW

/*
 * IOxAPIC IRQs for the GPIOs, overlap is expected as we encourage to use
 * shared IRQ instead of direct IRQ, in case of overlapping, we can easily
 * program one of the overlap to shared IRQ to avoid the conflict.
 */

/* NorthWest community pads */
#define PMIC_I2C_SDA_IRQ		0x32
#define GPIO_74_IRQ			0x33
#define GPIO_75_IRQ			0x34
#define GPIO_76_IRQ			0x35
#define GPIO_77_IRQ			0x36
#define GPIO_78_IRQ			0x37
#define GPIO_79_IRQ			0x38
#define GPIO_80_IRQ			0x39
#define GPIO_81_IRQ			0x3A
#define GPIO_82_IRQ			0x3B
#define GPIO_83_IRQ			0x3C
#define GPIO_84_IRQ			0x3D
#define GPIO_85_IRQ			0x3E
#define GPIO_86_IRQ			0x3F
#define GPIO_87_IRQ			0x40
#define GPIO_88_IRQ			0x41
#define GPIO_89_IRQ			0x42
#define GPIO_90_IRQ			0x43
#define GPIO_91_IRQ			0x44
#define GPIO_97_IRQ			0x49
#define GPIO_98_IRQ			0x4A
#define GPIO_99_IRQ			0x4B
#define GPIO_100_IRQ			0x4C
#define GPIO_101_IRQ			0x4D
#define GPIO_102_IRQ			0x4E
#define GPIO_103_IRQ			0x4F
#define GPIO_104_IRQ			0x50
#define GPIO_105_IRQ			0x51
#define GPIO_106_IRQ			0x52
#define GPIO_109_IRQ			0x54
#define GPIO_110_IRQ			0x55
#define GPIO_111_IRQ			0x56
#define GPIO_112_IRQ			0x57
#define GPIO_113_IRQ			0x58
#define GPIO_116_IRQ			0x5B
#define GPIO_117_IRQ			0x5C
#define GPIO_118_IRQ			0x5D
#define GPIO_119_IRQ			0x5E
#define GPIO_120_IRQ			0x5F
#define GPIO_121_IRQ			0x60
#define GPIO_122_IRQ			0x61
#define GPIO_123_IRQ			0x62

/* North community pads */
#define GPIO_0_IRQ			0x63
#define GPIO_1_IRQ			0x64
#define GPIO_2_IRQ			0x65
#define GPIO_3_IRQ			0x66
#define GPIO_4_IRQ			0x67
#define GPIO_5_IRQ			0x68
#define GPIO_6_IRQ			0x69
#define GPIO_7_IRQ			0x6A
#define GPIO_8_IRQ			0x6B
#define GPIO_9_IRQ			0x6C
#define GPIO_10_IRQ			0x6D
#define GPIO_11_IRQ			0x6E
#define GPIO_12_IRQ			0x6F
#define GPIO_13_IRQ			0x70
#define GPIO_14_IRQ			0x71
#define GPIO_15_IRQ			0x72
#define GPIO_16_IRQ			0x73
#define GPIO_17_IRQ			0x74
#define GPIO_18_IRQ			0x75
#define GPIO_19_IRQ			0x76
#define GPIO_20_IRQ			0x77
#define GPIO_21_IRQ			0x32
#define GPIO_22_IRQ			0x33
#define GPIO_23_IRQ			0x34
#define GPIO_24_IRQ			0x35
#define GPIO_25_IRQ			0x36
#define GPIO_26_IRQ			0x37
#define GPIO_27_IRQ			0x38
#define GPIO_28_IRQ			0x39
#define GPIO_29_IRQ			0x3A
#define GPIO_30_IRQ			0x3B
#define GPIO_31_IRQ			0x3C
#define GPIO_32_IRQ			0x3D
#define GPIO_33_IRQ			0x3E
#define GPIO_34_IRQ			0x3F
#define GPIO_35_IRQ			0x40
#define GPIO_36_IRQ			0x41
#define GPIO_37_IRQ			0x42
#define GPIO_38_IRQ			0x43
#define GPIO_39_IRQ			0x44
#define GPIO_40_IRQ			0x45
#define GPIO_41_IRQ			0x46
#define GPIO_42_IRQ			0x47
#define GPIO_43_IRQ			0x48
#define GPIO_44_IRQ			0x49
#define GPIO_45_IRQ			0x4A
#define GPIO_46_IRQ			0x4B
#define GPIO_47_IRQ			0x4C
#define GPIO_48_IRQ			0x4D
#define GPIO_49_IRQ			0x4E
#define GPIO_62_IRQ			0x5B
#define GPIO_63_IRQ			0x5C
#define GPIO_64_IRQ			0x5D
#define GPIO_65_IRQ			0x5E
#define GPIO_66_IRQ			0x5F
#define GPIO_67_IRQ			0x60
#define GPIO_68_IRQ			0x61
#define GPIO_69_IRQ			0x62
#define GPIO_70_IRQ			0x63
#define GPIO_71_IRQ			0x64
#define GPIO_72_IRQ			0x65
#define GPIO_73_IRQ			0x66

/* This is needed by ACPI */
#define GPIO_NUM_PAD_CFG_REGS   2 /* DW0, DW1 */

#ifndef __ASSEMBLY__

#include <dt-structs.h>

/**
 * struct apl_gpio_plat - platform data for each device
 *
 * @dtplat: of-platdata data from C struct
 */
struct apl_gpio_plat {
#if CONFIG_IS_ENABLED(OF_PLATDATA)
	/* Put this first since driver model will copy the data here */
	struct dtd_intel_apl_pinctrl dtplat;
#endif
};

#endif /* __ASSEMBLY__ */

#endif /* _ASM_ARCH_GPIO_H_ */
