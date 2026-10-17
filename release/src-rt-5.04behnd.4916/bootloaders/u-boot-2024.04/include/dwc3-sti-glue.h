/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2017, STMicroelectronics - All Rights Reserved
 * Author(s): Patrice Chotard, <patrice.chotard@foss.st.com> for STMicroelectronics.
 */

#ifndef __DWC3_STI_UBOOT_H_
#define __DWC3_STI_UBOOT_H_

/* glue registers */
#include <linux/bitops.h>
#define CLKRST_CTRL		0x00
#define AUX_CLK_EN		BIT(0)
#define SW_PIPEW_RESET_N	BIT(4)
#define EXT_CFG_RESET_N		BIT(8)

#define XHCI_REVISION		BIT(12)

#define USB2_VBUS_MNGMNT_SEL1	0x2C
#define USB2_VBUS_UTMIOTG	0x1

#define SEL_OVERRIDE_VBUSVALID(n)	((n) << 0)
#define SEL_OVERRIDE_POWERPRESENT(n)	((n) << 4)
#define SEL_OVERRIDE_BVALID(n)		((n) << 8)

/* Static DRD configuration */
#define USB3_CONTROL_MASK		0xf77

#define USB3_DEVICE_NOT_HOST		BIT(0)
#define USB3_FORCE_VBUSVALID		BIT(1)
#define USB3_DELAY_VBUSVALID		BIT(2)
#define USB3_SEL_FORCE_OPMODE		BIT(4)
#define USB3_FORCE_OPMODE(n)		((n) << 5)
#define USB3_SEL_FORCE_DPPULLDOWN2	BIT(8)
#define USB3_FORCE_DPPULLDOWN2		BIT(9)
#define USB3_SEL_FORCE_DMPULLDOWN2	BIT(10)
#define USB3_FORCE_DMPULLDOWN2		BIT(11)

int sti_dwc3_init(enum usb_dr_mode mode);

#endif /* __DWC3_STI_UBOOT_H_ */
