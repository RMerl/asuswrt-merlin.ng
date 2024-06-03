/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2015, Bin Meng <bmeng.cn@gmail.com>
 */

#ifndef _QUEENSBAY_DEVICE_H_
#define _QUEENSBAY_DEVICE_H_

#include <pci.h>

/* TunnelCreek PCI Devices */
#define TNC_HOST_BRIDGE_DEV	0
#define TNC_HOST_BRIDGE_FUNC	0
#define TNC_IGD_DEV		2
#define TNC_IGD_FUNC		0
#define TNC_SDVO_DEV		3
#define TNC_SDVO_FUNC		0
#define TNC_PCIE0_DEV		23
#define TNC_PCIE0_FUNC		0
#define TNC_PCIE1_DEV		24
#define TNC_PCIE1_FUNC		0
#define TNC_PCIE2_DEV		25
#define TNC_PCIE2_FUNC		0
#define TNC_PCIE3_DEV		26
#define TNC_PCIE3_FUNC		0
#define TNC_HDA_DEV		27
#define TNC_HDA_FUNC		0
#define TNC_LPC_DEV		31
#define TNC_LPC_FUNC		0

#define TNC_HOST_BRIDGE		\
	PCI_BDF(0, TNC_HOST_BRIDGE_DEV, TNC_HOST_BRIDGE_FUNC)
#define TNC_IGD			\
	PCI_BDF(0, TNC_IGD_DEV, TNC_IGD_FUNC)
#define TNC_SDVO		\
	PCI_BDF(0, TNC_SDVO_DEV, TNC_SDVO_FUNC)
#define TNC_PCIE0		\
	PCI_BDF(0, TNC_PCIE0_DEV, TNC_PCIE0_FUNC)
#define TNC_PCIE1		\
	PCI_BDF(0, TNC_PCIE1_DEV, TNC_PCIE1_FUNC)
#define TNC_PCIE2		\
	PCI_BDF(0, TNC_PCIE2_DEV, TNC_PCIE2_FUNC)
#define TNC_PCIE3		\
	PCI_BDF(0, TNC_PCIE3_DEV, TNC_PCIE3_FUNC)
#define TNC_HDA			\
	PCI_BDF(0, TNC_HDA_DEV, TNC_HDA_FUNC)
#define TNC_LPC			\
	PCI_BDF(0, TNC_LPC_DEV, TNC_LPC_FUNC)

/* Topcliff IOH PCI Devices */
#define TCF_PCIE_PORT_DEV	0
#define TCF_PCIE_PORT_FUNC	0

#define TCF_DEV_0		0
#define TCF_PKT_HUB_FUNC	0
#define TCF_GBE_FUNC		1
#define TCF_GPIO_FUNC		2

#define TCF_DEV_2		2
#define TCF_USB1_OHCI0_FUNC	0
#define TCF_USB1_OHCI1_FUNC	1
#define TCF_USB1_OHCI2_FUNC	2
#define TCF_USB1_EHCI_FUNC	3
#define TCF_USB_DEVICE_FUNC	4

#define TCF_DEV_4		4
#define TCF_SDIO0_FUNC		0
#define TCF_SDIO1_FUNC		1

#define TCF_DEV_6		6
#define TCF_SATA_FUNC		0

#define TCF_DEV_8		8
#define TCF_USB2_OHCI0_FUNC	0
#define TCF_USB2_OHCI1_FUNC	1
#define TCF_USB2_OHCI2_FUNC	2
#define TCF_USB2_EHCI_FUNC	3

#define TCF_DEV_10		10
#define TCF_DMA1_FUNC		0
#define TCF_UART0_FUNC		1
#define TCF_UART1_FUNC		2
#define TCF_UART2_FUNC		3
#define TCF_UART3_FUNC		4

#define TCF_DEV_12		12
#define TCF_DMA2_FUNC		0
#define TCF_SPI_FUNC		1
#define TCF_I2C_FUNC		2
#define TCF_CAN_FUNC		3
#define TCF_1588_FUNC		4

#endif /* _QUEENSBAY_DEVICE_H_ */
