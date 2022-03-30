/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2012 Samsung Electronics
 */

#ifndef __WM8994_REGISTERS_H__
#define __WM8994_REGISTERS_H__

/*
 * Register values.
 */
#define WM8994_SOFTWARE_RESET                   0x00
#define WM8994_POWER_MANAGEMENT_1               0x01
#define WM8994_POWER_MANAGEMENT_2               0x02
#define WM8994_POWER_MANAGEMENT_4		0x04
#define WM8994_POWER_MANAGEMENT_5               0x05
#define WM8994_LEFT_OUTPUT_VOLUME               0x1C
#define WM8994_RIGHT_OUTPUT_VOLUME              0x1D
#define WM8994_OUTPUT_MIXER_1                   0x2D
#define WM8994_OUTPUT_MIXER_2                   0x2E
#define WM8994_CHARGE_PUMP_1                    0x4C
#define WM8994_DC_SERVO_1                       0x54
#define WM8994_ANALOGUE_HP_1                    0x60
#define WM8994_CHIP_REVISION                    0x100
#define WM8994_AIF1_CLOCKING_1                  0x200
#define WM8994_AIF1_CLOCKING_2                  0x201
#define WM8994_AIF2_CLOCKING_1                  0x204
#define WM8994_CLOCKING_1                       0x208
#define WM8994_CLOCKING_2                       0x209
#define WM8994_AIF1_RATE                        0x210
#define WM8994_AIF2_RATE                        0x211
#define WM8994_RATE_STATUS                      0x212
#define WM8994_AIF1_CONTROL_1                   0x300
#define WM8994_AIF1_CONTROL_2                   0x301
#define WM8994_AIF1_MASTER_SLAVE                0x302
#define WM8994_AIF1_BCLK                        0x303
#define WM8994_AIF2_CONTROL_1                   0x310
#define WM8994_AIF2_CONTROL_2                   0x311
#define WM8994_AIF2_MASTER_SLAVE                0x312
#define WM8994_AIF2_BCLK                        0x313
#define WM8994_AIF1_DAC_FILTERS_1		0x420
#define WM8994_AIF2_DAC_LEFT_VOLUME             0x502
#define WM8994_AIF2_DAC_RIGHT_VOLUME            0x503
#define WM8994_AIF2_DAC_FILTERS_1               0x520
#define WM8994_DAC1_LEFT_MIXER_ROUTING          0x601
#define WM8994_DAC1_RIGHT_MIXER_ROUTING         0x602
#define WM8994_DAC1_LEFT_VOLUME                 0x610
#define WM8994_DAC1_RIGHT_VOLUME                0x611
#define WM8994_GPIO_1				0x700
#define WM8994_GPIO_3                           0x702
#define WM8994_GPIO_4                           0x703
#define WM8994_GPIO_5                           0x704

/*
 * Field Definitions.
 */

/*
 * R0 (0x00) - Software Reset
 */
/* SW_RESET */
#define WM8994_SW_RESET                              1
/*
 * R1 (0x01) - Power Management (1)
 */
/* HPOUT1L_ENA */
#define WM8994_HPOUT1L_ENA                      0x0200
/* HPOUT1L_ENA */
#define WM8994_HPOUT1L_ENA_MASK                 0x0200
/* HPOUT1R_ENA */
#define WM8994_HPOUT1R_ENA                      0x0100
/* HPOUT1R_ENA */
#define WM8994_HPOUT1R_ENA_MASK                 0x0100
/* VMID_SEL - [2:1] */
#define WM8994_VMID_SEL_MASK                    0x0006
/* BIAS_ENA */
#define WM8994_BIAS_ENA                         0x0001
/* BIAS_ENA */
#define WM8994_BIAS_ENA_MASK                    0x0001

/*
 * R2 (0x02) - Power Management (2)
 */
/* OPCLK_ENA */
#define WM8994_OPCLK_ENA                        0x0800

#define WM8994_TSHUT_ENA			0x4000
#define WM8994_MIXINL_ENA			0x0200
#define WM8994_MIXINR_ENA			0x0100
#define WM8994_IN2L_ENA				0x0080
#define WM8994_IN2R_ENA				0x0020

/*
 * R5 (0x04) - Power Management (4)
 */
#define WM8994_ADCL_ENA				0x0001
#define WM8994_ADCR_ENA				0x0002
#define WM8994_AIF1ADC1R_ENA			0x0100
#define WM8994_AIF1ADC1L_ENA			0x0200

/*
 * R5 (0x05) - Power Management (5)
 */
/* AIF2DACL_ENA */
#define WM8994_AIF2DACL_ENA                     0x2000
#define WM8994_AIF2DACL_ENA_MASK                0x2000
/* AIF2DACR_ENA */
#define WM8994_AIF2DACR_ENA                     0x1000
#define WM8994_AIF2DACR_ENA_MASK                0x1000
/* AIF1DACL_ENA */
#define WM8994_AIF1DACL_ENA			0x0200
#define WM8994_AIF1DACL_ENA_MASK		0x0200
/* AIF1DACR_ENA */
#define WM8994_AIF1DACR_ENA			0x0100
#define WM8994_AIF1DACR_ENA_MASK		0x0100
/* DAC1L_ENA */
#define WM8994_DAC1L_ENA                        0x0002
#define WM8994_DAC1L_ENA_MASK                   0x0002
/* DAC1R_ENA */
#define WM8994_DAC1R_ENA                        0x0001
#define WM8994_DAC1R_ENA_MASK                   0x0001

/*
 * R45 (0x2D) - Output Mixer (1)
 */
/* DAC1L_TO_HPOUT1L */
#define WM8994_DAC1L_TO_HPOUT1L                 0x0100
#define WM8994_DAC1L_TO_HPOUT1L_MASK            0x0100

/*
 * R46 (0x2E) - Output Mixer (2)
 */
/* DAC1R_TO_HPOUT1R */
#define WM8994_DAC1R_TO_HPOUT1R                 0x0100
#define WM8994_DAC1R_TO_HPOUT1R_MASK            0x0100

/*
 * R76 (0x4C) - Charge Pump (1)
 */
/* CP_ENA */
#define WM8994_CP_ENA                           0x8000
#define WM8994_CP_ENA_MASK                      0x8000
/*
 * R84 (0x54) - DC Servo (1)
 */
/* DCS_ENA_CHAN_1 */
#define WM8994_DCS_ENA_CHAN_1                   0x0002
#define WM8994_DCS_ENA_CHAN_1_MASK              0x0002
/* DCS_ENA_CHAN_0 */
#define WM8994_DCS_ENA_CHAN_0                   0x0001
#define WM8994_DCS_ENA_CHAN_0_MASK              0x0001

/*
 * R96 (0x60) - Analogue HP (1)
 */
/* HPOUT1L_RMV_SHORT */
#define WM8994_HPOUT1L_RMV_SHORT                0x0080
#define WM8994_HPOUT1L_RMV_SHORT_MASK           0x0080
/* HPOUT1L_OUTP */
#define WM8994_HPOUT1L_OUTP                     0x0040
#define WM8994_HPOUT1L_OUTP_MASK                0x0040
/* HPOUT1L_DLY */
#define WM8994_HPOUT1L_DLY                      0x0020
#define WM8994_HPOUT1L_DLY_MASK                 0x0020
/* HPOUT1R_RMV_SHORT */
#define WM8994_HPOUT1R_RMV_SHORT                0x0008
#define WM8994_HPOUT1R_RMV_SHORT_MASK           0x0008
/* HPOUT1R_OUTP */
#define WM8994_HPOUT1R_OUTP                     0x0004
#define WM8994_HPOUT1R_OUTP_MASK                0x0004
/* HPOUT1R_DLY */
#define WM8994_HPOUT1R_DLY                      0x0002
#define WM8994_HPOUT1R_DLY_MASK                 0x0002

/*
 * R512 (0x200) - AIF1 Clocking (1)
 */
/* AIF1CLK_SRC - [4:3] */
#define WM8994_AIF1CLK_SRC_MASK                 0x0018
/* AIF1CLK_DIV */
#define WM8994_AIF1CLK_DIV                      0x0002
/* AIF1CLK_ENA */
#define WM8994_AIF1CLK_ENA                      0x0001
#define WM8994_AIF1CLK_ENA_MASK                 0x0001

/*
 * R517 (0x205) - AIF2 Clocking (2)
 */
/* AIF2DAC_DIV - [5:3] */
#define WM8994_AIF2DAC_DIV_MASK                 0x0038

/*
 * R520 (0x208) - Clocking (1)
 */
/* AIF1DSPCLK_ENA */
#define WM8994_AIF1DSPCLK_ENA			0x0008
#define WM8994_AIF1DSPCLK_ENA_MASK		0x0008
/* AIF2DSPCLK_ENA */
#define WM8994_AIF2DSPCLK_ENA                   0x0004
#define WM8994_AIF2DSPCLK_ENA_MASK              0x0004
/* SYSDSPCLK_ENA */
#define WM8994_SYSDSPCLK_ENA                    0x0002
#define WM8994_SYSDSPCLK_ENA_MASK               0x0002
/* SYSCLK_SRC */
#define WM8994_SYSCLK_SRC                       0x0001

/*
 * R521 (0x209) - Clocking (2)
 */
/* OPCLK_DIV - [2:0] */
#define WM8994_OPCLK_DIV_MASK                   0x0007

/*
 * R528 (0x210) - AIF1 Rate
 */
/* AIF1_SR - [7:4] */
#define WM8994_AIF1_SR_MASK                     0x00F0
#define WM8994_AIF1_SR_SHIFT                         4
/* AIF1CLK_RATE - [3:0] */
#define WM8994_AIF1CLK_RATE_MASK                0x000F

/*
 * R768 (0x300) - AIF1 Control (1)
 */
/* AIF1_BCLK_INV */
#define WM8994_AIF1_BCLK_INV                    0x0100
/* AIF1_LRCLK_INV */
#define WM8994_AIF1_LRCLK_INV                   0x0080
#define WM8994_AIF1_LRCLK_INV_MASK              0x0080
/* AIF1_WL - [6:5] */
#define WM8994_AIF1_WL_MASK                     0x0060
/* AIF1_FMT - [4:3] */
#define WM8994_AIF1_FMT_MASK                    0x0018

/*
 * R769 (0x301) - AIF1 Control (2)
 */
/* AIF1_MONO */
#define WM8994_AIF1_MONO                        0x0100

/*
 * R770 (0x302) - AIF1 Master/Slave
 */
/* AIF1_MSTR */
#define WM8994_AIF1_MSTR                        0x4000
#define WM8994_AIF1_MSTR_MASK                   0x4000

/*
 * R771 (0x303) - AIF1 BCLK
 */
/* AIF1_BCLK_DIV - [8:4] */
#define WM8994_AIF1_BCLK_DIV_MASK               0x01F0
#define WM8994_AIF1_BCLK_DIV_SHIFT                   4

/*
 * R1282 (0x502) - AIF2 DAC Left Volume
 */
/* AIF2DAC_VU */
#define WM8994_AIF2DAC_VU                       0x0100
#define WM8994_AIF2DAC_VU_MASK                  0x0100
/* AIF2DACL_VOL - [7:0] */
#define WM8994_AIF2DACL_VOL_MASK                0x00FF

/*
 * R1283 (0x503) - AIF2 DAC Right Volume
 */
/* AIF2DACR_VOL - [7:0] */
#define WM8994_AIF2DACR_VOL_MASK                0x00FF

/*
 * R1312 (0x520) - AIF2 DAC Filters (1)
 */
/* AIF2DAC_MUTE */
#define WM8994_AIF2DAC_MUTE_MASK                0x0200

/*
 * R1537 (0x601) - DAC1 Left Mixer Routing
 */
/* AIF2DACL_TO_DAC1L */
#define WM8994_AIF2DACL_TO_DAC1L                0x0004
#define WM8994_AIF2DACL_TO_DAC1L_MASK           0x0004
/* AIF1DAC1L_TO_DAC1L */
#define WM8994_AIF1DAC1L_TO_DAC1L		0x0001

/*
 * R1538 (0x602) - DAC1 Right Mixer Routing
 */
/* AIF2DACR_TO_DAC1R */
#define WM8994_AIF2DACR_TO_DAC1R                0x0004
#define WM8994_AIF2DACR_TO_DAC1R_MASK           0x0004
/* AIF1DAC1R_TO_DAC1R */
#define WM8994_AIF1DAC1R_TO_DAC1R		0x0001

/*
 * R1552 (0x610) - DAC1 Left Volume
 */
/* DAC1L_MUTE */
#define WM8994_DAC1L_MUTE_MASK                  0x0200
/* DAC1_VU */
#define WM8994_DAC1_VU                          0x0100
#define WM8994_DAC1_VU_MASK                     0x0100
/* DAC1L_VOL - [7:0] */
#define WM8994_DAC1L_VOL_MASK                   0x00FF

/*
 * R1553 (0x611) - DAC1 Right Volume
 */
/* DAC1R_MUTE */
#define WM8994_DAC1R_MUTE_MASK                  0x0200
/* DAC1R_VOL - [7:0] */
#define WM8994_DAC1R_VOL_MASK                   0x00FF

/*
 *  GPIO
 */
/* OUTPUT PIN */
#define WM8994_GPIO_DIR_OUTPUT			0x8000
/* GPIO PIN MASK */
#define WM8994_GPIO_DIR_MASK			0xFFE0
/* I2S CLK */
#define WM8994_GPIO_FUNCTION_I2S_CLK		0x0001
#define WM8994_GPIO_INPUT_DEBOUNCE		0x0100
/* GPn FN */
#define WM8994_GPIO_FUNCTION_MASK		0x001F
#endif
