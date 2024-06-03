/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2004-2009
 * Texas Instruments Incorporated
 * Richard Woodruff		<r-woodruff2@ti.com>
 * Aneesh V			<aneesh@ti.com>
 * Balaji Krishnamoorthy	<balajitk@ti.com>
 */
#ifndef _MUX_OMAP5_H_
#define _MUX_OMAP5_H_

#include <asm/types.h>

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
#define EMMC_CLK		0x0040
#define EMMC_CMD		0x0042
#define EMMC_DATA0		0x0044
#define EMMC_DATA1		0x0046
#define EMMC_DATA2		0x0048
#define EMMC_DATA3		0x004a
#define EMMC_DATA4		0x004c
#define EMMC_DATA5		0x004e
#define EMMC_DATA6		0x0050
#define EMMC_DATA7		0x0052
#define C2C_CLKOUT0		0x0054
#define C2C_CLKOUT1		0x0056
#define C2C_CLKIN0		0x0058
#define C2C_CLKIN1		0x005a
#define C2C_DATAIN0		0x005c
#define C2C_DATAIN1		0x005e
#define C2C_DATAIN2		0x0060
#define C2C_DATAIN3		0x0062
#define C2C_DATAIN4		0x0064
#define C2C_DATAIN5		0x0066
#define C2C_DATAIN6		0x0068
#define C2C_DATAIN7		0x006a
#define C2C_DATAOUT0		0x006c
#define C2C_DATAOUT1		0x006e
#define C2C_DATAOUT2		0x0070
#define C2C_DATAOUT3		0x0072
#define C2C_DATAOUT4		0x0074
#define C2C_DATAOUT5		0x0076
#define C2C_DATAOUT6		0x0078
#define C2C_DATAOUT7		0x007a
#define C2C_DATA8		0x007c
#define C2C_DATA9		0x007e
#define C2C_DATA10		0x0080
#define C2C_DATA11		0x0082
#define C2C_DATA12		0x0084
#define C2C_DATA13		0x0086
#define C2C_DATA14		0x0088
#define C2C_DATA15		0x008a
#define LLIA_WAKEREQOUT		0x008c
#define LLIB_WAKEREQOUT		0x008e
#define HSI1_ACREADY		0x0090
#define HSI1_CAREADY		0x0092
#define HSI1_ACWAKE		0x0094
#define HSI1_CAWAKE		0x0096
#define HSI1_ACFLAG		0x0098
#define HSI1_ACDATA		0x009a
#define HSI1_CAFLAG		0x009c
#define HSI1_CADATA		0x009e
#define UART1_TX		0x00a0
#define UART1_CTS		0x00a2
#define UART1_RX		0x00a4
#define UART1_RTS		0x00a6
#define HSI2_CAREADY		0x00a8
#define HSI2_ACREADY		0x00aa
#define HSI2_CAWAKE		0x00ac
#define HSI2_ACWAKE		0x00ae
#define HSI2_CAFLAG		0x00b0
#define HSI2_CADATA		0x00b2
#define HSI2_ACFLAG		0x00b4
#define HSI2_ACDATA		0x00b6
#define UART2_RTS		0x00b8
#define UART2_CTS		0x00ba
#define UART2_RX		0x00bc
#define UART2_TX		0x00be
#define USBB1_HSIC_STROBE	0x00c0
#define USBB1_HSIC_DATA		0x00c2
#define USBB2_HSIC_STROBE	0x00c4
#define USBB2_HSIC_DATA		0x00c6
#define TIMER10_PWM_EVT		0x00c8
#define DSIPORTA_TE0		0x00ca
#define DSIPORTA_LANE0X		0x00cc
#define DSIPORTA_LANE0Y		0x00ce
#define DSIPORTA_LANE1X		0x00d0
#define DSIPORTA_LANE1Y		0x00d2
#define DSIPORTA_LANE2X		0x00d4
#define DSIPORTA_LANE2Y		0x00d6
#define DSIPORTA_LANE3X		0x00d8
#define DSIPORTA_LANE3Y		0x00da
#define DSIPORTA_LANE4X		0x00dc
#define DSIPORTA_LANE4Y		0x00de
#define DSIPORTC_LANE0X		0x00e0
#define DSIPORTC_LANE0Y		0x00e2
#define DSIPORTC_LANE1X		0x00e4
#define DSIPORTC_LANE1Y		0x00e6
#define DSIPORTC_LANE2X		0x00e8
#define DSIPORTC_LANE2Y		0x00ea
#define DSIPORTC_LANE3X		0x00ec
#define DSIPORTC_LANE3Y		0x00ee
#define DSIPORTC_LANE4X		0x00f0
#define DSIPORTC_LANE4Y		0x00f2
#define DSIPORTC_TE0		0x00f4
#define TIMER9_PWM_EVT		0x00f6
#define I2C4_SCL		0x00f8
#define I2C4_SDA		0x00fa
#define MCSPI2_CLK		0x00fc
#define MCSPI2_SIMO		0x00fe
#define MCSPI2_SOMI		0x0100
#define MCSPI2_CS0		0x0102
#define RFBI_DATA15		0x0104
#define RFBI_DATA14		0x0106
#define RFBI_DATA13		0x0108
#define RFBI_DATA12		0x010a
#define RFBI_DATA11		0x010c
#define RFBI_DATA10		0x010e
#define RFBI_DATA9		0x0110
#define RFBI_DATA8		0x0112
#define RFBI_DATA7		0x0114
#define RFBI_DATA6		0x0116
#define RFBI_DATA5		0x0118
#define RFBI_DATA4		0x011a
#define RFBI_DATA3		0x011c
#define RFBI_DATA2		0x011e
#define RFBI_DATA1		0x0120
#define RFBI_DATA0		0x0122
#define RFBI_WE			0x0124
#define RFBI_CS0		0x0126
#define RFBI_A0			0x0128
#define RFBI_RE			0x012a
#define RFBI_HSYNC0		0x012c
#define RFBI_TE_VSYNC0		0x012e
#define GPIO6_182		0x0130
#define GPIO6_183		0x0132
#define GPIO6_184		0x0134
#define GPIO6_185		0x0136
#define GPIO6_186		0x0138
#define GPIO6_187		0x013a
#define HDMI_CEC		0x013c
#define HDMI_HPD		0x013e
#define HDMI_DDC_SCL		0x0140
#define HDMI_DDC_SDA		0x0142
#define CSIPORTC_LANE0X		0x0144
#define CSIPORTC_LANE0Y		0x0146
#define CSIPORTC_LANE1X		0x0148
#define CSIPORTC_LANE1Y		0x014a
#define CSIPORTB_LANE0X		0x014c
#define CSIPORTB_LANE0Y		0x014e
#define CSIPORTB_LANE1X		0x0150
#define CSIPORTB_LANE1Y		0x0152
#define CSIPORTB_LANE2X		0x0154
#define CSIPORTB_LANE2Y		0x0156
#define CSIPORTA_LANE0X		0x0158
#define CSIPORTA_LANE0Y		0x015a
#define CSIPORTA_LANE1X		0x015c
#define CSIPORTA_LANE1Y		0x015e
#define CSIPORTA_LANE2X		0x0160
#define CSIPORTA_LANE2Y		0x0162
#define CSIPORTA_LANE3X		0x0164
#define CSIPORTA_LANE3Y		0x0166
#define CSIPORTA_LANE4X		0x0168
#define CSIPORTA_LANE4Y		0x016a
#define CAM_SHUTTER		0x016c
#define CAM_STROBE		0x016e
#define CAM_GLOBALRESET		0x0170
#define TIMER11_PWM_EVT		0x0172
#define TIMER5_PWM_EVT		0x0174
#define TIMER6_PWM_EVT		0x0176
#define TIMER8_PWM_EVT		0x0178
#define I2C3_SCL		0x017a
#define I2C3_SDA		0x017c
#define GPIO8_233		0x017e
#define GPIO8_234		0x0180
#define ABE_CLKS		0x0182
#define ABEDMIC_DIN1		0x0184
#define ABEDMIC_DIN2		0x0186
#define ABEDMIC_DIN3		0x0188
#define ABEDMIC_CLK1		0x018a
#define ABEDMIC_CLK2		0x018c
#define ABEDMIC_CLK3		0x018e
#define ABESLIMBUS1_CLOCK	0x0190
#define ABESLIMBUS1_DATA	0x0192
#define ABEMCBSP2_DR		0x0194
#define ABEMCBSP2_DX		0x0196
#define ABEMCBSP2_FSX		0x0198
#define ABEMCBSP2_CLKX		0x019a
#define ABEMCPDM_UL_DATA	0x019c
#define ABEMCPDM_DL_DATA	0x019e
#define ABEMCPDM_FRAME		0x01a0
#define ABEMCPDM_LB_CLK		0x01a2
#define WLSDIO_CLK		0x01a4
#define WLSDIO_CMD		0x01a6
#define WLSDIO_DATA0		0x01a8
#define WLSDIO_DATA1		0x01aa
#define WLSDIO_DATA2		0x01ac
#define WLSDIO_DATA3		0x01ae
#define UART5_RX		0x01b0
#define UART5_TX		0x01b2
#define UART5_CTS		0x01b4
#define UART5_RTS		0x01b6
#define I2C2_SCL		0x01b8
#define I2C2_SDA		0x01ba
#define MCSPI1_CLK		0x01bc
#define MCSPI1_SOMI		0x01be
#define MCSPI1_SIMO		0x01c0
#define MCSPI1_CS0		0x01c2
#define MCSPI1_CS1		0x01c4
#define I2C5_SCL		0x01c6
#define I2C5_SDA		0x01c8
#define PERSLIMBUS2_CLOCK	0x01ca
#define PERSLIMBUS2_DATA	0x01cc
#define UART6_TX		0x01ce
#define UART6_RX		0x01d0
#define UART6_CTS		0x01d2
#define UART6_RTS		0x01d4
#define UART3_CTS_RCTX		0x01d6
#define UART3_RTS_IRSD		0x01d8
#define UART3_TX_IRTX		0x01da
#define UART3_RX_IRRX		0x01dc
#define USBB3_HSIC_STROBE	0x01de
#define USBB3_HSIC_DATA		0x01e0
#define SDCARD_CLK		0x01e2
#define SDCARD_CMD		0x01e4
#define SDCARD_DATA2		0x01e6
#define SDCARD_DATA3		0x01e8
#define SDCARD_DATA0		0x01ea
#define SDCARD_DATA1		0x01ec
#define USBD0_HS_DP		0x01ee
#define USBD0_HS_DM		0x01f0
#define I2C1_PMIC_SCL		0x01f2
#define I2C1_PMIC_SDA		0x01f4
#define USBD0_SS_RX		0x01f6

#define LLIA_WAKEREQIN		0x0040
#define LLIB_WAKEREQIN		0x0042
#define DRM_EMU0		0x0044
#define DRM_EMU1		0x0046
#define JTAG_NTRST		0x0048
#define JTAG_TCK		0x004a
#define JTAG_RTCK		0x004c
#define JTAG_TMSC		0x004e
#define JTAG_TDI		0x0050
#define JTAG_TDO		0x0052
#define SYS_32K			0x0054
#define FREF_CLK_IOREQ		0x0056
#define FREF_CLK0_OUT		0x0058
#define FREF_CLK1_OUT		0x005a
#define FREF_CLK2_OUT		0x005c
#define FREF_CLK2_REQ		0x005e
#define FREF_CLK1_REQ		0x0060
#define SYS_NRESPWRON		0x0062
#define SYS_NRESWARM		0x0064
#define SYS_PWR_REQ		0x0066
#define SYS_NIRQ1		0x0068
#define SYS_NIRQ2		0x006a
#define SR_PMIC_SCL		0x006c
#define SR_PMIC_SDA		0x006e
#define SYS_BOOT0		0x0070
#define SYS_BOOT1		0x0072
#define SYS_BOOT2		0x0074
#define SYS_BOOT3		0x0076
#define SYS_BOOT4		0x0078
#define SYS_BOOT5		0x007a

#endif /* _MUX_OMAP5_H_ */
