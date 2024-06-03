/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2013
 * Texas Instruments Inc, <www.ti.com>
 *
 * Author: Dan Murphy <dmurphy@ti.com>
 */

#ifndef _ASM_ARCH_XHCI_OMAP_H_
#define _ASM_ARCH_XHCI_OMAP_H_

#ifdef CONFIG_DRA7XX
#if CONFIG_USB_XHCI_DRA7XX_INDEX == 1
#define OMAP_XHCI_BASE 0x488d0000
#define OMAP_OCP1_SCP_BASE 0x4A081000
#define OMAP_OTG_WRAPPER_BASE 0x488c0000
#elif CONFIG_USB_XHCI_DRA7XX_INDEX == 0
#define OMAP_XHCI_BASE 0x48890000
#define OMAP_OCP1_SCP_BASE 0x4A084c00
#define OMAP_OTG_WRAPPER_BASE 0x48880000
#endif /* CONFIG_USB_XHCI_DRA7XX_INDEX == 1 */
#elif defined CONFIG_AM43XX
#define OMAP_XHCI_BASE 0x483d0000
#define OMAP_OCP1_SCP_BASE 0x483E8000
#define OMAP_OTG_WRAPPER_BASE 0x483dc100
#else
/* Default to the OMAP5 XHCI defines */
#define OMAP_XHCI_BASE 0x4a030000
#define OMAP_OCP1_SCP_BASE 0x4a084c00
#define OMAP_OTG_WRAPPER_BASE 0x4A020000
#endif

/* Phy register MACRO definitions */
#define	PLL_REGM_MASK		0x001FFE00
#define	PLL_REGM_SHIFT		0x9
#define	PLL_REGM_F_MASK		0x0003FFFF
#define	PLL_REGM_F_SHIFT	0x0
#define	PLL_REGN_MASK		0x000001FE
#define	PLL_REGN_SHIFT		0x1
#define	PLL_SELFREQDCO_MASK	0x0000000E
#define	PLL_SELFREQDCO_SHIFT	0x1
#define	PLL_SD_MASK		0x0003FC00
#define	PLL_SD_SHIFT		0x9
#define	SET_PLL_GO		0x1
#define	PLL_TICOPWDN		0x10000
#define	PLL_LOCK		0x2
#define	PLL_IDLE		0x1

#define USB3_PWRCTL_CLK_CMD_MASK	0x3FE000
#define USB3_PWRCTL_CLK_FREQ_MASK	0xFFC
#define USB3_PHY_PARTIAL_RX_POWERON     (1 << 6)
#define USB3_PHY_RX_POWERON		(1 << 14)
#define USB3_PHY_TX_POWERON		(1 << 15)
#define USB3_PHY_TX_RX_POWERON	(USB3_PHY_RX_POWERON | USB3_PHY_TX_POWERON)
#define USB3_PWRCTL_CLK_CMD_SHIFT   14
#define USB3_PWRCTL_CLK_FREQ_SHIFT	22

/* USBOTGSS_WRAPPER definitions */
#define USBOTGSS_WRAPRESET	(1 << 17)
#define USBOTGSS_DMADISABLE (1 << 16)
#define USBOTGSS_STANDBYMODE_NO_STANDBY (1 << 4)
#define USBOTGSS_STANDBYMODE_SMRT		(1 << 5)
#define USBOTGSS_STANDBYMODE_SMRT_WKUP (0x3 << 4)
#define USBOTGSS_IDLEMODE_NOIDLE (1 << 2)
#define USBOTGSS_IDLEMODE_SMRT (1 << 3)
#define USBOTGSS_IDLEMODE_SMRT_WKUP (0x3 << 2)

/* USBOTGSS_IRQENABLE_SET_0 bit */
#define USBOTGSS_COREIRQ_EN	(1 << 0)

/* USBOTGSS_IRQENABLE_SET_1 bits */
#define USBOTGSS_IRQ_SET_1_IDPULLUP_FALL_EN	(1 << 0)
#define USBOTGSS_IRQ_SET_1_DISCHRGVBUS_FALL_EN	(1 << 3)
#define USBOTGSS_IRQ_SET_1_CHRGVBUS_FALL_EN	(1 << 4)
#define USBOTGSS_IRQ_SET_1_DRVVBUS_FALL_EN	(1 << 5)
#define USBOTGSS_IRQ_SET_1_IDPULLUP_RISE_EN	(1 << 8)
#define USBOTGSS_IRQ_SET_1_DISCHRGVBUS_RISE_EN	(1 << 11)
#define USBOTGSS_IRQ_SET_1_CHRGVBUS_RISE_EN	(1 << 12)
#define USBOTGSS_IRQ_SET_1_DRVVBUS_RISE_EN	(1 << 13)
#define USBOTGSS_IRQ_SET_1_OEVT_EN	(1 << 16)
#define USBOTGSS_IRQ_SET_1_DMADISABLECLR_EN	(1 << 17)

/*
 * USBOTGSS_WRAPPER registers
 */
struct omap_dwc_wrapper {
	u32 revision;

	u32 reserve_1[3];

	u32 sysconfig; /* offset of 0x10 */

	u32 reserve_2[3];
	u16 reserve_3;

	u32 irqstatus_raw_0; /* offset of 0x24 */
	u32 irqstatus_0;
	u32 irqenable_set_0;
	u32 irqenable_clr_0;

	u32 irqstatus_raw_1; /* offset of 0x34 */
	u32 irqstatus_1;
	u32 irqenable_set_1;
	u32 irqenable_clr_1;

	u32 reserve_4[15];

	u32 utmi_otg_ctrl; /* offset of 0x80 */
	u32 utmi_otg_status;

	u32 reserve_5[30];

	u32 mram_offset; /* offset of 0x100 */
	u32 fladj;
	u32 dbg_config;
	u32 dbg_data;
	u32 dev_ebc_en;
};

/* XHCI PHY register structure */
struct omap_usb3_phy {
	u32 reserve1;
	u32 pll_status;
	u32 pll_go;
	u32 pll_config_1;
	u32 pll_config_2;
	u32 pll_config_3;
	u32 pll_ssc_config_1;
	u32 pll_ssc_config_2;
	u32 pll_config_4;
};

struct omap_xhci {
	struct omap_dwc_wrapper *otg_wrapper;
	struct omap_usb3_phy *usb3_phy;
	struct xhci_hccr *hcd;
	struct dwc3 *dwc3_reg;
};

/* USB PHY functions */
void omap_enable_phy(struct omap_xhci *omap);
void omap_reset_usb_phy(struct dwc3 *dwc3_reg);
void usb_phy_power(int on);

#endif /* _ASM_ARCH_XHCI_OMAP_H_ */
