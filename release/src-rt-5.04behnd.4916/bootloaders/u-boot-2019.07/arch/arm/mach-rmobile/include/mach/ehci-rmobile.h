/* SPDX-License-Identifier: GPL-2.0 */
/*
 *  Copyright (C) 2013,2014 Renesas Electronics Corporation
 *  Copyright (C) 2014 Nobuhiro Iwamatsu <nobuhiro.iwamatsu.yj@renesas.com>
 */

#ifndef __EHCI_RMOBILE_H__
#define __EHCI_RMOBILE_H__

/* Register offset */
#define OHCI_OFFSET	0x00
#define OHCI_SIZE	0x1000
#define EHCI_OFFSET	0x1000
#define EHCI_SIZE	0x1000

#define EHCI_USBCMD	(EHCI_OFFSET + 0x0020)

/* USBCTR */
#define DIRPD		(1 << 8)
#define PLL_RST		(1 << 2)
#define PCICLK_MASK	(1 << 1)
#define USBH_RST	(1 << 0)

/* CMND_STS */
#define SERREN		(1 << 8)
#define PERREN		(1 << 6)
#define MASTEREN	(1 << 2)
#define MEMEN		(1 << 1)

/* PCIAHB_WIN1_CTR and PCIAHB_WIN2_CTR */
#define PCIAHB_WIN_PREFETCH	((1 << 1)|(1 << 0))

/* AHBPCI_WIN1_CTR */
#define PCIWIN1_PCICMD		((1 << 3)|(1 << 1))
#define AHB_CFG_AHBPCI		0x40000000
#define AHB_CFG_HOST		0x80000000

/* AHBPCI_WIN2_CTR */
#define PCIWIN2_PCICMD		((1 << 2)|(1 << 1))

/* PCI_INT_ENABLE */
#define USBH_PMEEN		(1 << 19)
#define USBH_INTBEN		(1 << 17)
#define USBH_INTAEN		(1 << 16)

/* AHB_BUS_CTR */
#define SMODE_READY_CTR		(1 << 17)
#define SMODE_READ_BURST	(1 << 16)
#define MMODE_HBUSREQ		(1 << 7)
#define MMODE_BOUNDARY		((1 << 6)|(1 << 5))
#define MMODE_BURST_WIDTH	((1 << 4)|(1 << 3))
#define MMODE_SINGLE_MODE	((1 << 4)|(1 << 3))
#define MMODE_WR_INCR		(1 << 2)
#define MMODE_BYTE_BURST	(1 << 1)
#define MMODE_HTRANS		(1 << 0)

/* PCI_ARBITER_CTR */
#define PCIBUS_PARK_TIMER       0x00FF0000
#define PCIBUS_PARK_TIMER_SET   0x00070000
#define PCIBP_MODE		(1 << 12)
#define PCIREQ7                 (1 << 7)
#define PCIREQ6                 (1 << 6)
#define PCIREQ5                 (1 << 5)
#define PCIREQ4                 (1 << 4)
#define PCIREQ3                 (1 << 3)
#define PCIREQ2                 (1 << 2)
#define PCIREQ1                 (1 << 1)
#define PCIREQ0                 (1 << 0)

#define SMSTPCR7        0xE615014C
#define SMSTPCR703      (1 << 3)

/* Init AHB master and slave functions of the host logic */
#define AHB_BUS_CTR_INIT \
	(SMODE_READY_CTR | MMODE_HBUSREQ | MMODE_WR_INCR | \
	 MMODE_BYTE_BURST | MMODE_HTRANS)

#define USBCTR_WIN_SIZE_1GB	0x800

/* PCI Configuration Registers */
#define PCI_CONF_OHCI_OFFSET	0x10000
#define PCI_CONF_EHCI_OFFSET	0x10100
struct ahb_pciconf {
	u32 vid_did;
	u32 cmnd_sts;
	u32 rev;
	u32 cache_line;
	u32 basead;
};

/* PCI Configuration Registers for AHB-PCI Bridge Registers */
#define PCI_CONF_AHBPCI_OFFSET	0x10000
struct ahbconf_pci_bridge {
	u32 vid_did;		/* 0x00 */
	u32 cmnd_sts;
	u32 revid_cc;
	u32 cls_lt_ht_bist;
	u32 basead;		/* 0x10 */
	u32 win1_basead;
	u32 win2_basead;
	u32 dummy0[5];
	u32 ssvdi_ssid;		/* 0x2C */
	u32 dummy1[4];
	u32 intr_line_pin;
};

/* AHB-PCI Bridge PCI Communication Registers */
#define AHBPCI_OFFSET	0x10800
struct ahbcom_pci_bridge {
	u32 pciahb_win1_ctr;	/* 0x00 */
	u32 pciahb_win2_ctr;
	u32 pciahb_dct_ctr;
	u32 dummy0;
	u32 ahbpci_win1_ctr;	/* 0x10 */
	u32 ahbpci_win2_ctr;
	u32 dummy1;
	u32 ahbpci_dct_ctr;
	u32 pci_int_enable;	/* 0x20 */
	u32 pci_int_status;
	u32 dummy2[2];
	u32 ahb_bus_ctr;	/* 0x30 */
	u32 usbctr;
	u32 dummy3[2];
	u32 pci_arbiter_ctr;	/* 0x40 */
	u32 dummy4;
	u32 pci_unit_rev;	/* 0x48 */
};

struct rmobile_ehci_reg {
	u32 hciversion;		/* hciversion/caplength */
	u32 hcsparams;		/* hcsparams */
	u32 hccparams;		/* hccparams */
	u32 hcsp_portroute;	/* hcsp_portroute */
	u32 usbcmd;		/* usbcmd */
	u32 usbsts;		/* usbsts */
	u32 usbintr;		/* usbintr */
	u32 frindex;		/* frindex */
	u32 ctrldssegment;	/* ctrldssegment */
	u32 periodiclistbase;	/* periodiclistbase */
	u32 asynclistaddr;	/* asynclistaddr */
	u32 dummy[9];
	u32 configflag;		/* configflag */
	u32 portsc;		/* portsc */
};

#endif /* __EHCI_RMOBILE_H__ */
