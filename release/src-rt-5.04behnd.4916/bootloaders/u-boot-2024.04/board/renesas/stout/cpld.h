/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Stout board CPLD definition
 *
 * Copyright (C) 2015 Renesas Electronics Europe GmbH
 * Copyright (C) 2015 Renesas Electronics Corporation
 * Copyright (C) 2015 Cogent Embedded, Inc.
 */

#ifndef _CPLD_H_
#define _CPLD_H_

/* power-up behaviour */
#define MODE_MSK_FREE_RUN		0x00000001
#define MODE_VAL_FREE_RUN		0x00000000
#define MODE_MSK_STEP_UP		0x00000001
#define MODE_VAL_STEP_UP		0x00000000

/* boot source */
#define MODE_MSK_BOOT_SQPI_16KB_FAST	0x0000000E
#define MODE_VAL_BOOT_SQPI_16KB_FAST	0x00000004
#define MODE_MSK_BOOT_SQPI_16KB_SLOW	0x0000000E
#define MODE_VAL_BOOT_SQPI_16KB_SLOW	0x00000008
#define MODE_MSK_BOOT_SQPI_4KB_SLOW	0x0000000E
#define MODE_VAL_BOOT_SQPI_4KB_SLOW	0x0000000C

/* booting CPU */
#define MODE_MSK_BOOT_CA15		0x000000C0
#define MODE_VAL_BOOT_CA15		0x00000000
#define MODE_MSK_BOOT_CA7		0x000000C0
#define MODE_VAL_BOOT_CA7		0x00000040
#define MODE_MSK_BOOT_SH4		0x000000C0
#define MODE_VAL_BOOT_SH4		0x000000C0

/* JTAG connection */
#define MODE_MSK_JTAG_CORESIGHT		0xC0301C00
#define MODE_VAL_JTAG_CORESIGHT		0x00200000
#define MODE_MSK_JTAG_SH4		0xC0301C00
#define MODE_VAL_JTAG_SH4		0x00300000

/* DDR3 (PLL) speed */
#define MODE_MSK_DDR3_1600		0x00080000
#define MODE_VAL_DDR3_1600		0x00000000
#define MODE_MSK_DDR3_1333		0x00080000
#define MODE_VAL_DDR3_1333		0x00080000

/* ComboPhy0 mode */
#define MODE_MSK_PHY0_SATA0		0x01000000
#define MODE_VAL_PHY0_SATA0		0x00000000
#define MODE_MSK_PHY0_PCIE		0x01000000
#define MODE_VAL_PHY0_PCIE		0x01000000

/* ComboPhy1 mode */
#define MODE_MSK_PHY1_SATA1		0x00800000
#define MODE_VAL_PHY1_SATA1		0x00000000
#define MODE_MSK_PHY1_USB3		0x00800000
#define MODE_VAL_PHY1_USB3		0x00800000

/*
 * Illegal multiplexer combinations.
 *    MUX                      Conflicts
 *    name                  with any one of
 * VIN0_BT656            VIN0_full, SD2
 * VIN0_full             VIN0_BT656, SD2, AVB, VIN2_(all)
 * VIN1_BT656            VIN1_(others), SD0
 * VIN1_10bit            VIN1_(others), SD0, VIN3_with*, I2C1
 * VIN1_12bit            VIN1_(others), SD0, VIN3_with*, I2C1, SCIFA0_(all)
 * VIN2_BT656            VIN0_full, VIN2_(others), AVB,
 * VIN2_withSYNC         VIN0_full, VIN2_(others), AVB, I2C1, SCIFA0_(all),
 *                       VIN3_with*
 * VIN2_withFIELD        VIN0_full, VIN2_(others), AVB, SQPI_(all)
 * VIN2_withSYNCandFIELD VIN0_full, VIN2_(others), AVB, SQPI_(all), I2C1,
 *                       SCIFA0_(all), VIN3_with*
 * VIN3_BT656            VIN3_(others), IRQ3
 * VIN3_withFIELD        VIN3_(others), IRQ3, VIN1_12bit, VIN2_withSYNC,
 *                       VIN2_withSYNCandFIELD, VIN1_10bit
 * VIN3_withSYNCandFIELD VIN3_(others), IRQ3, VIN1_12bit, VIN2_withSYNC,
 *                       VIN2_withSYNCandFIELD, VIN1_10bit, I2C1
 * AVB                   VIN0_full, VIN2_(all)
 * QSPI_ONBOARD          VIN2_withFIELD, VIN2_withSYNCandFIELD, QSPI_COMEXPRESS
 * QSPI_COMEXPRESS       VIN2_withFIELD, VIN2_withSYNCandFIELD, QSPI_ONBOARD
 * I2C1                  VIN1_12bit, VIN2_withSYNC, VIN2_withSYNCandFIELD,
 *                       VIN3_withSYNCandFIELD
 * IRQ3                  VIN3_(all)
 * SCIFA0_USB            VIN1_12bit, VIN2_withSYNC, VIN2_withSYNCandFIELD,
 *                       SCIFA0_COMEXPRESS
 * SCIFA0_COMEXPRESS     VIN1_12bit, VIN2_withSYNC, VIN2_withSYNCandFIELD,
 *                       SCIFA0_USB
 * SCIFA2                PWM210
 * ETH_ONBOARD           ETH_COMEXPRESS
 * ETH_COMEXPRESS        ETH_ONBOARD
 * SD0                   VIN1_(all)
 * SD2                   VIN0_(all)
 * PWM210                SCIFA2
 */

/* connected to COM Express connector and CN6 for camera, BT656 only */
#define MUX_MSK_VIN0_BT656		0x00001001
#define MUX_VAL_VIN0_BT656		0x00000000
/* connected to COM Express connector and CN6 for camera, all modes */
#define MUX_MSK_VIN0_full		0x00001007
#define MUX_VAL_VIN0_full		0x00000002
/* connected to COM Express connector, BT656 only */
#define MUX_MSK_VIN1_BT656		0x00000801
#define MUX_VAL_VIN1_BT656		0x00000800
/* connected to COM Express connector, all 10-bit modes */
#define MUX_MSK_VIN1_10bit		0x00000821
#define MUX_VAL_VIN1_10bit		0x00000800
/* connected to COM Express connector, all 12-bit modes */
#define MUX_MSK_VIN1_12bit		0x000008A1
#define MUX_VAL_VIN1_12bit		0x00000880
/* connected to COM Express connector, BT656 only */
#define MUX_MSK_VIN2_BT656		0x00000007
#define MUX_VAL_VIN2_BT656		0x00000006
/* connected to COM Express connector, modes with sync signals */
#define MUX_MSK_VIN2_withSYNC		0x000000A7
#define MUX_VAL_VIN2_withSYNC		0x00000086
/* connected to COM Express connector, modes with field, clken signals */
#define MUX_MSK_VIN2_withFIELD		0x0000000F
#define MUX_VAL_VIN2_withFIELD		0x0000000E
/* connected to COM Express connector, modes with sync, field, clken signals */
#define MUX_MSK_VIN2_withSYNCandFIELD	0x000000AF
#define MUX_VAL_VIN2_withSYNCandFIELD	0x0000008E
/* connected to COM Express connector, BT656 only */
#define MUX_MSK_VIN3_BT656		0x00000101
#define MUX_VAL_VIN3_BT656		0x00000100
/* connected to COM Express connector, modes with field, clken signals */
#define MUX_MSK_VIN3_withFIELD		0x00000121
#define MUX_VAL_VIN3_withFIELD		0x00000120
/* connected to COM Express connector, modes with sync, field, clken signals */
#define MUX_MSK_VIN3_withSYNCandFIELD	0x00000161
#define MUX_VAL_VIN3_withSYNCandFIELD	0x00000120
/* connected to COM Express connector (RGMII) */
#define MUX_MSK_AVB			0x00000003
#define MUX_VAL_AVB			0x00000000
/* connected to on-board QSPI flash */
#define MUX_MSK_QSPI_ONBOARD		0x00000019
#define MUX_VAL_QSPI_ONBOARD		0x00000000
/* connected to COM Express connector */
#define MUX_MSK_QSPI_COMEXPRESS		0x00000019
#define MUX_VAL_QSPI_COMEXPRESS		0x00000010
/* connected to COM Express connector and PMIC */
#define MUX_MSK_I2C1			0x00000061
#define MUX_VAL_I2C1			0x00000060
/* connected to HDMI driver */
#define MUX_MSK_IRQ3			0x00000101
#define MUX_VAL_IRQ3			0x00000000
/* connected to USB/FTDI */
#define MUX_MSK_SCIFA0_USB		0x00004081
#define MUX_VAL_SCIFA0_USB		0x00004000
/* connected to COM Express connector */
#define MUX_MSK_SCIFA0_COMEXPRESS	0x00004081
#define MUX_VAL_SCIFA0_COMEXPRESS	0x00000000
/* connected to COM Express connector */
#define MUX_MSK_SCIFA2			0x00002001
#define MUX_VAL_SCIFA2			0x00000000
/* connected to on-board 10/100 Phy */
#define MUX_MSK_ETH_ONBOARD		0x00000600
#define MUX_VAL_ETH_ONBOARD		0x00000000
/* connected to COM Express connector (RMII) */
#define MUX_MSK_ETH_COMEXPRESS		0x00000600
#define MUX_VAL_ETH_COMEXPRESS		0x00000400
/* connected to on-board MicroSD slot */
#define MUX_MSK_SD0			0x00000801
#define MUX_VAL_SD0			0x00000000
/* connected to COM Express connector */
#define MUX_MSK_SD2			0x00001001
#define MUX_VAL_SD2			0x00001000
/* connected to COM Express connector */
#define MUX_MSK_PWM210			0x00002001
#define MUX_VAL_PWM210			0x00002000

#define HDMI_MSK			0x07
#define HDMI_OFF			0x00
#define HDMI_ONBOARD			0x07
#define HDMI_COMEXPRESS			0x05
#define HDMI_ONBOARD_NODDC		0x03
#define HDMI_COMEXPRESS_NODDC		0x01

void cpld_init(void);

#endif	/* _CPLD_H_ */
