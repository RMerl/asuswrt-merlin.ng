/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2013
 * Texas Instruments Incorporated
 *
 * Nishant Kamat <nskamat@ti.com>
 * Lokesh Vutla <lokeshvutla@ti.com>
 */
#ifndef _MUX_DRA7XX_H_
#define _MUX_DRA7XX_H_

#include <asm/types.h>

#define PULL_ENA		(0 << 16)
#define PULL_DIS		(1 << 16)
#define PULL_UP			(1 << 17)
#define INPUT_EN		(1 << 18)
#define SLEWCONTROL		(1 << 19)

/* Active pin states */
#define PIN_OUTPUT		(0 | PULL_DIS)
#define PIN_OUTPUT_PULLUP	(PULL_UP)
#define PIN_OUTPUT_PULLDOWN	(0)
#define PIN_INPUT		(INPUT_EN | PULL_DIS)
#define PIN_INPUT_SLEW		(INPUT_EN | SLEWCONTROL)
#define PIN_INPUT_PULLUP	(PULL_ENA | INPUT_EN | PULL_UP)
#define PIN_INPUT_PULLDOWN	(PULL_ENA | INPUT_EN)

#define M0	0
#define M1	1
#define M2	2
#define M3	3
#define M4	4
#define M5	5
#define M6	6
#define M7	7
#define M8	8
#define M9	9
#define M10	10
#define M11	11
#define M12	12
#define M13	13
#define M14	14
#define M15	15

#define MODE_SELECT		(1 << 8)
#define DELAYMODE_SHIFT		4

#define MANUAL_MODE	MODE_SELECT

#define VIRTUAL_MODE0	(MODE_SELECT | (0x0 << DELAYMODE_SHIFT))
#define VIRTUAL_MODE1	(MODE_SELECT | (0x1 << DELAYMODE_SHIFT))
#define VIRTUAL_MODE2	(MODE_SELECT | (0x2 << DELAYMODE_SHIFT))
#define VIRTUAL_MODE3	(MODE_SELECT | (0x3 << DELAYMODE_SHIFT))
#define VIRTUAL_MODE4	(MODE_SELECT | (0x4 << DELAYMODE_SHIFT))
#define VIRTUAL_MODE5	(MODE_SELECT | (0x5 << DELAYMODE_SHIFT))
#define VIRTUAL_MODE6	(MODE_SELECT | (0x6 << DELAYMODE_SHIFT))
#define VIRTUAL_MODE7	(MODE_SELECT | (0x7 << DELAYMODE_SHIFT))
#define VIRTUAL_MODE8	(MODE_SELECT | (0x8 << DELAYMODE_SHIFT))
#define VIRTUAL_MODE9	(MODE_SELECT | (0x9 << DELAYMODE_SHIFT))
#define VIRTUAL_MODE10	(MODE_SELECT | (0xa << DELAYMODE_SHIFT))
#define VIRTUAL_MODE11	(MODE_SELECT | (0xb << DELAYMODE_SHIFT))
#define VIRTUAL_MODE12	(MODE_SELECT | (0xc << DELAYMODE_SHIFT))
#define VIRTUAL_MODE13	(MODE_SELECT | (0xd << DELAYMODE_SHIFT))
#define VIRTUAL_MODE14	(MODE_SELECT | (0xe << DELAYMODE_SHIFT))
#define VIRTUAL_MODE15	(MODE_SELECT | (0xf << DELAYMODE_SHIFT))

#define SAFE_MODE	M15

#define GPMC_AD0	0x000
#define GPMC_AD1	0x004
#define GPMC_AD2	0x008
#define GPMC_AD3	0x00C
#define GPMC_AD4	0x010
#define GPMC_AD5	0x014
#define GPMC_AD6	0x018
#define GPMC_AD7	0x01C
#define GPMC_AD8	0x020
#define GPMC_AD9	0x024
#define GPMC_AD10	0x028
#define GPMC_AD11	0x02C
#define GPMC_AD12	0x030
#define GPMC_AD13	0x034
#define GPMC_AD14	0x038
#define GPMC_AD15	0x03C
#define GPMC_A0		0x040
#define GPMC_A1		0x044
#define GPMC_A2		0x048
#define GPMC_A3		0x04C
#define GPMC_A4		0x050
#define GPMC_A5		0x054
#define GPMC_A6		0x058
#define GPMC_A7		0x05C
#define GPMC_A8		0x060
#define GPMC_A9		0x064
#define GPMC_A10	0x068
#define GPMC_A11	0x06C
#define GPMC_A12	0x070
#define GPMC_A13	0x074
#define GPMC_A14	0x078
#define GPMC_A15	0x07C
#define GPMC_A16	0x080
#define GPMC_A17	0x084
#define GPMC_A18	0x088
#define GPMC_A19	0x08C
#define GPMC_A20	0x090
#define GPMC_A21	0x094
#define GPMC_A22	0x098
#define GPMC_A23	0x09C
#define GPMC_A24	0x0A0
#define GPMC_A25	0x0A4
#define GPMC_A26	0x0A8
#define GPMC_A27	0x0AC
#define GPMC_CS1	0x0B0
#define GPMC_CS0	0x0B4
#define GPMC_CS2	0x0B8
#define GPMC_CS3	0x0BC
#define GPMC_CLK	0x0C0
#define GPMC_ADVN_ALE	0x0C4
#define GPMC_OEN_REN	0x0C8
#define GPMC_WEN	0x0CC
#define GPMC_BEN0	0x0D0
#define GPMC_BEN1	0x0D4
#define GPMC_WAIT0	0x0D8
#define VIN1A_CLK0	0x0DC
#define VIN1B_CLK1	0x0E0
#define VIN1A_DE0	0x0E4
#define VIN1A_FLD0	0x0E8
#define VIN1A_HSYNC0	0x0EC
#define VIN1A_VSYNC0	0x0F0
#define VIN1A_D0	0x0F4
#define VIN1A_D1	0x0F8
#define VIN1A_D2	0x0FC
#define VIN1A_D3	0x100
#define VIN1A_D4	0x104
#define VIN1A_D5	0x108
#define VIN1A_D6	0x10C
#define VIN1A_D7	0x110
#define VIN1A_D8	0x114
#define VIN1A_D9	0x118
#define VIN1A_D10	0x11C
#define VIN1A_D11	0x120
#define VIN1A_D12	0x124
#define VIN1A_D13	0x128
#define VIN1A_D14	0x12C
#define VIN1A_D15	0x130
#define VIN1A_D16	0x134
#define VIN1A_D17	0x138
#define VIN1A_D18	0x13C
#define VIN1A_D19	0x140
#define VIN1A_D20	0x144
#define VIN1A_D21	0x148
#define VIN1A_D22	0x14C
#define VIN1A_D23	0x150
#define VIN2A_CLK0	0x154
#define VIN2A_DE0	0x158
#define VIN2A_FLD0	0x15C
#define VIN2A_HSYNC0	0x160
#define VIN2A_VSYNC0	0x164
#define VIN2A_D0	0x168
#define VIN2A_D1	0x16C
#define VIN2A_D2	0x170
#define VIN2A_D3	0x174
#define VIN2A_D4	0x178
#define VIN2A_D5	0x17C
#define VIN2A_D6	0x180
#define VIN2A_D7	0x184
#define VIN2A_D8	0x188
#define VIN2A_D9	0x18C
#define VIN2A_D10	0x190
#define VIN2A_D11	0x194
#define VIN2A_D12	0x198
#define VIN2A_D13	0x19C
#define VIN2A_D14	0x1A0
#define VIN2A_D15	0x1A4
#define VIN2A_D16	0x1A8
#define VIN2A_D17	0x1AC
#define VIN2A_D18	0x1B0
#define VIN2A_D19	0x1B4
#define VIN2A_D20	0x1B8
#define VIN2A_D21	0x1BC
#define VIN2A_D22	0x1C0
#define VIN2A_D23	0x1C4
#define VOUT1_CLK	0x1C8
#define VOUT1_DE	0x1CC
#define VOUT1_FLD	0x1D0
#define VOUT1_HSYNC	0x1D4
#define VOUT1_VSYNC	0x1D8
#define VOUT1_D0	0x1DC
#define VOUT1_D1	0x1E0
#define VOUT1_D2	0x1E4
#define VOUT1_D3	0x1E8
#define VOUT1_D4	0x1EC
#define VOUT1_D5	0x1F0
#define VOUT1_D6	0x1F4
#define VOUT1_D7	0x1F8
#define VOUT1_D8	0x1FC
#define VOUT1_D9	0x200
#define VOUT1_D10	0x204
#define VOUT1_D11	0x208
#define VOUT1_D12	0x20C
#define VOUT1_D13	0x210
#define VOUT1_D14	0x214
#define VOUT1_D15	0x218
#define VOUT1_D16	0x21C
#define VOUT1_D17	0x220
#define VOUT1_D18	0x224
#define VOUT1_D19	0x228
#define VOUT1_D20	0x22C
#define VOUT1_D21	0x230
#define VOUT1_D22	0x234
#define VOUT1_D23	0x238
#define MDIO_MCLK	0x23C
#define MDIO_D		0x240
#define RMII_MHZ_50_CLK	0x244
#define UART3_RXD	0x248
#define UART3_TXD	0x24C
#define RGMII0_TXC	0x250
#define RGMII0_TXCTL	0x254
#define RGMII0_TXD3	0x258
#define RGMII0_TXD2	0x25C
#define RGMII0_TXD1	0x260
#define RGMII0_TXD0	0x264
#define RGMII0_RXC	0x268
#define RGMII0_RXCTL	0x26C
#define RGMII0_RXD3	0x270
#define RGMII0_RXD2	0x274
#define RGMII0_RXD1	0x278
#define RGMII0_RXD0	0x27C
#define USB1_DRVVBUS	0x280
#define USB2_DRVVBUS	0x284
#define GPIO6_14	0x288
#define GPIO6_15	0x28C
#define GPIO6_16	0x290
#define XREF_CLK0	0x294
#define XREF_CLK1	0x298
#define XREF_CLK2	0x29C
#define XREF_CLK3	0x2A0
#define MCASP1_ACLKX	0x2A4
#define MCASP1_FSX	0x2A8
#define MCASP1_ACLKR	0x2AC
#define MCASP1_FSR	0x2B0
#define MCASP1_AXR0	0x2B4
#define MCASP1_AXR1	0x2B8
#define MCASP1_AXR2	0x2BC
#define MCASP1_AXR3	0x2C0
#define MCASP1_AXR4	0x2C4
#define MCASP1_AXR5	0x2C8
#define MCASP1_AXR6	0x2CC
#define MCASP1_AXR7	0x2D0
#define MCASP1_AXR8	0x2D4
#define MCASP1_AXR9	0x2D8
#define MCASP1_AXR10	0x2DC
#define MCASP1_AXR11	0x2E0
#define MCASP1_AXR12	0x2E4
#define MCASP1_AXR13	0x2E8
#define MCASP1_AXR14	0x2EC
#define MCASP1_AXR15	0x2F0
#define MCASP2_ACLKX	0x2F4
#define MCASP2_FSX	0x2F8
#define MCASP2_ACLKR	0x2FC
#define MCASP2_FSR	0x300
#define MCASP2_AXR0	0x304
#define MCASP2_AXR1	0x308
#define MCASP2_AXR2	0x30C
#define MCASP2_AXR3	0x310
#define MCASP2_AXR4	0x314
#define MCASP2_AXR5	0x318
#define MCASP2_AXR6	0x31C
#define MCASP2_AXR7	0x320
#define MCASP3_ACLKX	0x324
#define MCASP3_FSX	0x328
#define MCASP3_AXR0	0x32C
#define MCASP3_AXR1	0x330
#define MCASP4_ACLKX	0x334
#define MCASP4_FSX	0x338
#define MCASP4_AXR0	0x33C
#define MCASP4_AXR1	0x340
#define MCASP5_ACLKX	0x344
#define MCASP5_FSX	0x348
#define MCASP5_AXR0	0x34C
#define MCASP5_AXR1	0x350
#define MMC1_CLK	0x354
#define MMC1_CMD	0x358
#define MMC1_DAT0	0x35C
#define MMC1_DAT1	0x360
#define MMC1_DAT2	0x364
#define MMC1_DAT3	0x368
#define MMC1_SDCD	0x36C
#define MMC1_SDWP	0x370
#define GPIO6_10	0x374
#define GPIO6_11	0x378
#define MMC3_CLK	0x37C
#define MMC3_CMD	0x380
#define MMC3_DAT0	0x384
#define MMC3_DAT1	0x388
#define MMC3_DAT2	0x38C
#define MMC3_DAT3	0x390
#define MMC3_DAT4	0x394
#define MMC3_DAT5	0x398
#define MMC3_DAT6	0x39C
#define MMC3_DAT7	0x3A0
#define SPI1_SCLK	0x3A4
#define SPI1_D1		0x3A8
#define SPI1_D0		0x3AC
#define SPI1_CS0	0x3B0
#define SPI1_CS1	0x3B4
#define SPI1_CS2	0x3B8
#define SPI1_CS3	0x3BC
#define SPI2_SCLK	0x3C0
#define SPI2_D1		0x3C4
#define SPI2_D0		0x3C8
#define SPI2_CS0	0x3CC
#define DCAN1_TX	0x3D0
#define DCAN1_RX	0x3D4
#define DCAN2_TX	0x3D8
#define DCAN2_RX	0x3DC
#define UART1_RXD	0x3E0
#define UART1_TXD	0x3E4
#define UART1_CTSN	0x3E8
#define UART1_RTSN	0x3EC
#define UART2_RXD	0x3F0
#define UART2_TXD	0x3F4
#define UART2_CTSN	0x3F8
#define UART2_RTSN	0x3FC
#define I2C1_SDA	0x400
#define I2C1_SCL	0x404
#define I2C2_SDA	0x408
#define I2C2_SCL	0x40C
#define I2C3_SDA	0x410
#define I2C3_SCL	0x414
#define WAKEUP0		0x418
#define WAKEUP1		0x41C
#define WAKEUP2		0x420
#define WAKEUP3		0x424
#define ON_OFF		0x428
#define RTC_PORZ	0x42C
#define TMS		0x430
#define TDI		0x434
#define TDO		0x438
#define TCLK		0x43C
#define TRSTN		0x440
#define RTCK		0x444
#define EMU0		0x448
#define EMU1		0x44C
#define EMU2		0x450
#define EMU3		0x454
#define EMU4		0x458
#define RESETN		0x45C
#define NMIN_DSP	0x460
#define RSTOUTN		0x464

#define MCAN_SEL_ALT_MASK	0x6000
#define MCAN_SEL		0x2000

#endif /* _MUX_DRA7XX_H_ */
