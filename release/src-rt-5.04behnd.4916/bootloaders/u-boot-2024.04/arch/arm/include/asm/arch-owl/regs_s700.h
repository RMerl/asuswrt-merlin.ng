/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Actions Semi S700 Register Definitions
 *
 */

#ifndef _OWL_REGS_S700_H_
#define _OWL_REGS_S700_H_

#define CMU_COREPLL		0x0000
#define CMU_DEVPLL		0x0004
#define CMU_DDRPLL		0x0008
#define CMU_NANDPLL		0x000C
#define CMU_DISPLAYPLL		0x0010
#define CMU_AUDIOPLL		0x0014
#define CMU_TVOUTPLL		0x0018
#define CMU_BUSCLK		0x001C
#define CMU_SENSORCLK		0x0020
#define CMU_LCDCLK		0x0024
#define CMU_DSIPLLCLK		0x0028
#define CMU_CSICLK		0x002C
#define CMU_DECLK		0x0030
#define CMU_SICLK		0x0034
#define CMU_BUSCLK1		0x0038
#define CMU_HDECLK		0x003C
#define CMU_VDECLK		0x0040
#define CMU_VCECLK		0x0044
#define CMU_NANDCCLK		0x004C
#define CMU_SD0CLK		0x0050
#define CMU_SD1CLK		0x0054
#define CMU_SD2CLK		0x0058
#define CMU_UART0CLK		0x005C
#define CMU_UART1CLK		0x0060
#define CMU_UART2CLK		0x0064
#define CMU_UART3CLK		0x0068
#define CMU_UART4CLK		0x006C
#define CMU_UART5CLK		0x0070
#define CMU_UART6CLK		0x0074
#define CMU_PWM0CLK		0x0078
#define CMU_PWM1CLK		0x007C
#define CMU_PWM2CLK		0x0080
#define CMU_PWM3CLK		0x0084
#define CMU_PWM4CLK		0x0088
#define CMU_PWM5CLK		0x008C
#define CMU_GPU3DCLK		0x0090
#define CMU_CORECTL		0x009C
#define CMU_DEVCLKEN0		0x00A0
#define CMU_DEVCLKEN1		0x00A4
#define CMU_DEVRST0		0x00A8
#define CMU_DEVRST1		0x00AC
#define CMU_USBPLL		0x00B0
#define CMU_ETHERNETPLL		0x00B4
#define CMU_CVBSPLL		0x00B8
#define CMU_SSTSCLK		0x00C0

#define CMU_DEVCLKEN1_ETH	BIT(23)

#define GPIO_MFP_PWM           (0xE01B0000)
#define MFP_CTL0               (GPIO_MFP_PWM + 0x40)
#define MFP_CTL1               (GPIO_MFP_PWM + 0x44)
#define MFP_CTL2               (GPIO_MFP_PWM + 0x48)
#define MFP_CTL3               (GPIO_MFP_PWM + 0x4C)

#endif
