/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2004-2009
 * Texas Instruments Incorporated
 * Richard Woodruff		<r-woodruff2@ti.com>
 * Aneesh V			<aneesh@ti.com>
 * Balaji Krishnamoorthy	<balajitk@ti.com>
 */
#ifndef _MUX_OMAP4_H_
#define _MUX_OMAP4_H_

#include <asm/types.h>

struct pad_conf_entry {

	u16 offset;

	u16 val;

};

#ifdef CONFIG_OFF_PADCONF
#define OFF_PD          (1 << 12)
#define OFF_PU          (3 << 12)
#define OFF_OUT_PTD     (0 << 10)
#define OFF_OUT_PTU     (2 << 10)
#define OFF_IN          (1 << 10)
#define OFF_OUT         (0 << 10)
#define OFF_EN          (1 << 9)
#else
#define OFF_PD          (0 << 12)
#define OFF_PU          (0 << 12)
#define OFF_OUT_PTD     (0 << 10)
#define OFF_OUT_PTU     (0 << 10)
#define OFF_IN          (0 << 10)
#define OFF_OUT         (0 << 10)
#define OFF_EN          (0 << 9)
#endif

#define IEN             (1 << 8)
#define IDIS            (0 << 8)
#define PTU             (3 << 3)
#define PTD             (1 << 3)
#define EN              (1 << 3)
#define DIS             (0 << 3)

#define M0              0
#define M1              1
#define M2              2
#define M3              3
#define M4              4
#define M5              5
#define M6              6
#define M7              7

#define SAFE_MODE	M7

#ifdef CONFIG_OFF_PADCONF
#define OFF_IN_PD       (OFF_PD | OFF_IN | OFF_EN)
#define OFF_IN_PU       (OFF_PU | OFF_IN | OFF_EN)
#define OFF_OUT_PD      (OFF_OUT_PTD | OFF_OUT | OFF_EN)
#define OFF_OUT_PU      (OFF_OUT_PTU | OFF_OUT | OFF_EN)
#else
#define OFF_IN_PD       0
#define OFF_IN_PU       0
#define OFF_OUT_PD      0
#define OFF_OUT_PU      0
#endif

#define CORE_REVISION		0x0000
#define CORE_HWINFO		0x0004
#define CORE_SYSCONFIG		0x0010
#define GPMC_AD0		0x0040
#define GPMC_AD1		0x0042
#define GPMC_AD2		0x0044
#define GPMC_AD3		0x0046
#define GPMC_AD4		0x0048
#define GPMC_AD5		0x004A
#define GPMC_AD6		0x004C
#define GPMC_AD7		0x004E
#define GPMC_AD8		0x0050
#define GPMC_AD9		0x0052
#define GPMC_AD10		0x0054
#define GPMC_AD11		0x0056
#define GPMC_AD12		0x0058
#define GPMC_AD13		0x005A
#define GPMC_AD14		0x005C
#define GPMC_AD15		0x005E
#define GPMC_A16		0x0060
#define GPMC_A17		0x0062
#define GPMC_A18		0x0064
#define GPMC_A19		0x0066
#define GPMC_A20		0x0068
#define GPMC_A21		0x006A
#define GPMC_A22		0x006C
#define GPMC_A23		0x006E
#define GPMC_A24		0x0070
#define GPMC_A25		0x0072
#define GPMC_NCS0		0x0074
#define GPMC_NCS1		0x0076
#define GPMC_NCS2		0x0078
#define GPMC_NCS3		0x007A
#define GPMC_NWP		0x007C
#define GPMC_CLK		0x007E
#define GPMC_NADV_ALE		0x0080
#define GPMC_NOE		0x0082
#define GPMC_NWE		0x0084
#define GPMC_NBE0_CLE		0x0086
#define GPMC_NBE1		0x0088
#define GPMC_WAIT0		0x008A
#define GPMC_WAIT1		0x008C
#define C2C_DATA11		0x008E
#define C2C_DATA12		0x0090
#define C2C_DATA13		0x0092
#define C2C_DATA14		0x0094
#define C2C_DATA15		0x0096
#define HDMI_HPD		0x0098
#define HDMI_CEC		0x009A
#define HDMI_DDC_SCL		0x009C
#define HDMI_DDC_SDA		0x009E
#define CSI21_DX0		0x00A0
#define CSI21_DY0		0x00A2
#define CSI21_DX1		0x00A4
#define CSI21_DY1		0x00A6
#define CSI21_DX2		0x00A8
#define CSI21_DY2		0x00AA
#define CSI21_DX3		0x00AC
#define CSI21_DY3		0x00AE
#define CSI21_DX4		0x00B0
#define CSI21_DY4		0x00B2
#define CSI22_DX0		0x00B4
#define CSI22_DY0		0x00B6
#define CSI22_DX1		0x00B8
#define CSI22_DY1		0x00BA
#define CAM_SHUTTER		0x00BC
#define CAM_STROBE		0x00BE
#define CAM_GLOBALRESET		0x00C0
#define USBB1_ULPITLL_CLK	0x00C2
#define USBB1_ULPITLL_STP	0x00C4
#define USBB1_ULPITLL_DIR	0x00C6
#define USBB1_ULPITLL_NXT	0x00C8
#define USBB1_ULPITLL_DAT0	0x00CA
#define USBB1_ULPITLL_DAT1	0x00CC
#define USBB1_ULPITLL_DAT2	0x00CE
#define USBB1_ULPITLL_DAT3	0x00D0
#define USBB1_ULPITLL_DAT4	0x00D2
#define USBB1_ULPITLL_DAT5	0x00D4
#define USBB1_ULPITLL_DAT6	0x00D6
#define USBB1_ULPITLL_DAT7	0x00D8
#define USBB1_HSIC_DATA		0x00DA
#define USBB1_HSIC_STROBE	0x00DC
#define USBC1_ICUSB_DP		0x00DE
#define USBC1_ICUSB_DM		0x00E0
#define SDMMC1_CLK		0x00E2
#define SDMMC1_CMD		0x00E4
#define SDMMC1_DAT0		0x00E6
#define SDMMC1_DAT1		0x00E8
#define SDMMC1_DAT2		0x00EA
#define SDMMC1_DAT3		0x00EC
#define SDMMC1_DAT4		0x00EE
#define SDMMC1_DAT5		0x00F0
#define SDMMC1_DAT6		0x00F2
#define SDMMC1_DAT7		0x00F4
#define ABE_MCBSP2_CLKX		0x00F6
#define ABE_MCBSP2_DR		0x00F8
#define ABE_MCBSP2_DX		0x00FA
#define ABE_MCBSP2_FSX		0x00FC
#define ABE_MCBSP1_CLKX		0x00FE
#define ABE_MCBSP1_DR		0x0100
#define ABE_MCBSP1_DX		0x0102
#define ABE_MCBSP1_FSX		0x0104
#define ABE_PDM_UL_DATA		0x0106
#define ABE_PDM_DL_DATA		0x0108
#define ABE_PDM_FRAME		0x010A
#define ABE_PDM_LB_CLK		0x010C
#define ABE_CLKS		0x010E
#define ABE_DMIC_CLK1		0x0110
#define ABE_DMIC_DIN1		0x0112
#define ABE_DMIC_DIN2		0x0114
#define ABE_DMIC_DIN3		0x0116
#define UART2_CTS		0x0118
#define UART2_RTS		0x011A
#define UART2_RX		0x011C
#define UART2_TX		0x011E
#define HDQ_SIO			0x0120
#define I2C1_SCL		0x0122
#define I2C1_SDA		0x0124
#define I2C2_SCL		0x0126
#define I2C2_SDA		0x0128
#define I2C3_SCL		0x012A
#define I2C3_SDA		0x012C
#define I2C4_SCL		0x012E
#define I2C4_SDA		0x0130
#define MCSPI1_CLK		0x0132
#define MCSPI1_SOMI		0x0134
#define MCSPI1_SIMO		0x0136
#define MCSPI1_CS0		0x0138
#define MCSPI1_CS1		0x013A
#define MCSPI1_CS2		0x013C
#define MCSPI1_CS3		0x013E
#define UART3_CTS_RCTX		0x0140
#define UART3_RTS_SD		0x0142
#define UART3_RX_IRRX		0x0144
#define UART3_TX_IRTX		0x0146
#define SDMMC5_CLK		0x0148
#define SDMMC5_CMD		0x014A
#define SDMMC5_DAT0		0x014C
#define SDMMC5_DAT1		0x014E
#define SDMMC5_DAT2		0x0150
#define SDMMC5_DAT3		0x0152
#define MCSPI4_CLK		0x0154
#define MCSPI4_SIMO		0x0156
#define MCSPI4_SOMI		0x0158
#define MCSPI4_CS0		0x015A
#define UART4_RX		0x015C
#define UART4_TX		0x015E
#define USBB2_ULPITLL_CLK	0x0160
#define USBB2_ULPITLL_STP	0x0162
#define USBB2_ULPITLL_DIR	0x0164
#define USBB2_ULPITLL_NXT	0x0166
#define USBB2_ULPITLL_DAT0	0x0168
#define USBB2_ULPITLL_DAT1	0x016A
#define USBB2_ULPITLL_DAT2	0x016C
#define USBB2_ULPITLL_DAT3	0x016E
#define USBB2_ULPITLL_DAT4	0x0170
#define USBB2_ULPITLL_DAT5	0x0172
#define USBB2_ULPITLL_DAT6	0x0174
#define USBB2_ULPITLL_DAT7	0x0176
#define USBB2_HSIC_DATA		0x0178
#define USBB2_HSIC_STROBE	0x017A
#define UNIPRO_TX0		0x017C
#define UNIPRO_TY0		0x017E
#define UNIPRO_TX1		0x0180
#define UNIPRO_TY1		0x0182
#define UNIPRO_TX2		0x0184
#define UNIPRO_TY2		0x0186
#define UNIPRO_RX0		0x0188
#define UNIPRO_RY0		0x018A
#define UNIPRO_RX1		0x018C
#define UNIPRO_RY1		0x018E
#define UNIPRO_RX2		0x0190
#define UNIPRO_RY2		0x0192
#define USBA0_OTG_CE		0x0194
#define USBA0_OTG_DP		0x0196
#define USBA0_OTG_DM		0x0198
#define FREF_CLK1_OUT		0x019A
#define FREF_CLK2_OUT		0x019C
#define SYS_NIRQ1		0x019E
#define SYS_NIRQ2		0x01A0
#define SYS_BOOT0		0x01A2
#define SYS_BOOT1		0x01A4
#define SYS_BOOT2		0x01A6
#define SYS_BOOT3		0x01A8
#define SYS_BOOT4		0x01AA
#define SYS_BOOT5		0x01AC
#define DPM_EMU0		0x01AE
#define DPM_EMU1		0x01B0
#define DPM_EMU2		0x01B2
#define DPM_EMU3		0x01B4
#define DPM_EMU4		0x01B6
#define DPM_EMU5		0x01B8
#define DPM_EMU6		0x01BA
#define DPM_EMU7		0x01BC
#define DPM_EMU8		0x01BE
#define DPM_EMU9		0x01C0
#define DPM_EMU10		0x01C2
#define DPM_EMU11		0x01C4
#define DPM_EMU12		0x01C6
#define DPM_EMU13		0x01C8
#define DPM_EMU14		0x01CA
#define DPM_EMU15		0x01CC
#define DPM_EMU16		0x01CE
#define DPM_EMU17		0x01D0
#define DPM_EMU18		0x01D2
#define DPM_EMU19		0x01D4
#define WAKEUPEVENT_0		0x01D8
#define WAKEUPEVENT_1		0x01DC
#define WAKEUPEVENT_2		0x01E0
#define WAKEUPEVENT_3		0x01E4
#define WAKEUPEVENT_4		0x01E8
#define WAKEUPEVENT_5		0x01EC
#define WAKEUPEVENT_6		0x01F0

#define WKUP_REVISION		0x0000
#define WKUP_HWINFO		0x0004
#define WKUP_SYSCONFIG		0x0010
#define PAD0_SIM_IO		0x0040
#define PAD1_SIM_CLK		0x0042
#define PAD0_SIM_RESET		0x0044
#define PAD1_SIM_CD		0x0046
#define PAD0_SIM_PWRCTRL		0x0048
#define PAD1_SR_SCL		0x004A
#define PAD0_SR_SDA		0x004C
#define PAD1_FREF_XTAL_IN		0x004E
#define PAD0_FREF_SLICER_IN	0x0050
#define PAD1_FREF_CLK_IOREQ	0x0052
#define PAD0_FREF_CLK0_OUT		0x0054
#define PAD1_FREF_CLK3_REQ		0x0056
#define PAD0_FREF_CLK3_OUT		0x0058
#define PAD1_FREF_CLK4_REQ		0x005A
#define PAD0_FREF_CLK4_OUT		0x005C
#define PAD1_SYS_32K		0x005E
#define PAD0_SYS_NRESPWRON		0x0060
#define PAD1_SYS_NRESWARM		0x0062
#define PAD0_SYS_PWR_REQ		0x0064
#define PAD1_SYS_PWRON_RESET	0x0066
#define PAD0_SYS_BOOT6		0x0068
#define PAD1_SYS_BOOT7		0x006A
#define PAD0_JTAG_NTRST		0x006C
#define PAD1_JTAG_TCK		0x006D
#define PAD0_JTAG_RTCK		0x0070
#define PAD1_JTAG_TMS_TMSC		0x0072
#define PAD0_JTAG_TDI		0x0074
#define PAD1_JTAG_TDO		0x0076
#define PADCONF_WAKEUPEVENT_0	0x007C
#define CONTROL_SMART1NOPMIO_PADCONF_0		0x05A0
#define CONTROL_SMART1NOPMIO_PADCONF_1		0x05A4
#define PADCONF_MODE		0x05A8
#define CONTROL_XTAL_OSCILLATOR			0x05AC
#define CONTROL_CONTROL_I2C_2			0x0604
#define CONTROL_CONTROL_JTAG			0x0608
#define CONTROL_CONTROL_SYS			0x060C
#define CONTROL_SPARE_RW		0x0614
#define CONTROL_SPARE_R		0x0618
#define CONTROL_SPARE_R_C0		0x061C

#define CONTROL_WKUP_PAD1_FREF_CLK4_REQ	0x4A31E05A
#endif /* _MUX_OMAP4_H_ */
