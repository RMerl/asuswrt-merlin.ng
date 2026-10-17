/* SPDX-License-Identifier: GPL-2.0 */
/*-
 * Copyright (c) 2007-2008, Juniper Networks, Inc.
 * Copyright (c) 2008, Michael Trimarchi <trimarchimichael@yahoo.it>
 * All rights reserved.
 */

#ifndef USB_EHCI_H
#define USB_EHCI_H

#include <stdbool.h>
#include <usb.h>
#include <generic-phy.h>

/* Section 2.2.3 - N_PORTS */
#define MAX_HC_PORTS		15

/*
 * Register Space.
 */
struct ehci_hccr {
	uint32_t cr_capbase;
#define HC_LENGTH(p)		(((p) >> 0) & 0x00ff)
#define HC_VERSION(p)		(((p) >> 16) & 0xffff)
	uint32_t cr_hcsparams;
#define HCS_PPC(p)		((p) & (1 << 4))
#define HCS_INDICATOR(p)	((p) & (1 << 16)) /* Port indicators */
#define HCS_N_PORTS(p)		(((p) >> 0) & 0xf)
	uint32_t cr_hccparams;
	uint8_t cr_hcsp_portrt[8];
} __attribute__ ((packed, aligned(4)));

struct ehci_hcor {
	uint32_t or_usbcmd;
#define CMD_PARK	(1 << 11)		/* enable "park" */
#define CMD_PARK_CNT(c)	(((c) >> 8) & 3)	/* how many transfers to park */
#define CMD_LRESET	(1 << 7)		/* partial reset */
#define CMD_IAAD	(1 << 6)		/* "doorbell" interrupt */
#define CMD_ASE		(1 << 5)		/* async schedule enable */
#define CMD_PSE		(1 << 4)		/* periodic schedule enable */
#define CMD_RESET	(1 << 1)		/* reset HC not bus */
#define CMD_RUN		(1 << 0)		/* start/stop HC */
	uint32_t or_usbsts;
#define STS_ASS		(1 << 15)
#define	STS_PSS		(1 << 14)
#define STS_HALT	(1 << 12)
#define STS_IAA		(1 << 5)
	uint32_t or_usbintr;
#define INTR_UE         (1 << 0)                /* USB interrupt enable */
#define INTR_UEE        (1 << 1)                /* USB error interrupt enable */
#define INTR_PCE        (1 << 2)                /* Port change detect enable */
#define INTR_SEE        (1 << 4)                /* system error enable */
#define INTR_AAE        (1 << 5)                /* Interrupt on async adavance enable */
	uint32_t or_frindex;
	uint32_t or_ctrldssegment;
	uint32_t or_periodiclistbase;
	uint32_t or_asynclistaddr;
	uint32_t _reserved_0_;
	uint32_t or_burstsize;
	uint32_t or_txfilltuning;
#define TXFIFO_THRESH_MASK		(0x3f << 16)
#define TXFIFO_THRESH(p)		((p & 0x3f) << 16)
	uint32_t _reserved_1_[6];
	uint32_t or_configflag;
#define FLAG_CF		(1 << 0)	/* true:  we'll support "high speed" */
	uint32_t or_portsc[MAX_HC_PORTS];
#define PORTSC_PSPD(x)		(((x) >> 26) & 0x3)
#define PORTSC_PSPD_FS			0x0
#define PORTSC_PSPD_LS			0x1
#define PORTSC_PSPD_HS			0x2
#define PORTSC_FSL_PFSC		BIT(24) /* PFSC bit to disable HS chirping */

	uint32_t or_systune;
} __attribute__ ((packed, aligned(4)));

#define USBMODE		0x68		/* USB Device mode */
#define USBMODE_SDIS	(1 << 3)	/* Stream disable */
#define USBMODE_BE	(1 << 2)	/* BE/LE endiannes select */
#define USBMODE_CM_HC	(3 << 0)	/* host controller mode */
#define USBMODE_CM_IDLE	(0 << 0)	/* idle state */

/* Interface descriptor */
struct usb_linux_interface_descriptor {
	unsigned char	bLength;
	unsigned char	bDescriptorType;
	unsigned char	bInterfaceNumber;
	unsigned char	bAlternateSetting;
	unsigned char	bNumEndpoints;
	unsigned char	bInterfaceClass;
	unsigned char	bInterfaceSubClass;
	unsigned char	bInterfaceProtocol;
	unsigned char	iInterface;
} __attribute__ ((packed));

/* Configuration descriptor information.. */
struct usb_linux_config_descriptor {
	unsigned char	bLength;
	unsigned char	bDescriptorType;
	unsigned short	wTotalLength;
	unsigned char	bNumInterfaces;
	unsigned char	bConfigurationValue;
	unsigned char	iConfiguration;
	unsigned char	bmAttributes;
	unsigned char	MaxPower;
} __attribute__ ((packed));

#if defined CONFIG_EHCI_DESC_BIG_ENDIAN
#define ehci_readl(x)		be32_to_cpu(__raw_readl(x))
#define ehci_writel(a, b)	__raw_writel(cpu_to_be32(b), a)
#else
#define ehci_readl(x)		readl(x)
#define ehci_writel(a, b)	writel(b, a)
#endif

#if defined CONFIG_EHCI_MMIO_BIG_ENDIAN
#define hc32_to_cpu(x)		be32_to_cpu((x))
#define cpu_to_hc32(x)		cpu_to_be32((x))
#else
#define hc32_to_cpu(x)		le32_to_cpu((x))
#define cpu_to_hc32(x)		cpu_to_le32((x))
#endif

#define EHCI_PS_WKOC_E		(1 << 22)	/* RW wake on over current */
#define EHCI_PS_WKDSCNNT_E	(1 << 21)	/* RW wake on disconnect */
#define EHCI_PS_WKCNNT_E	(1 << 20)	/* RW wake on connect */
#define EHCI_PS_PO		(1 << 13)	/* RW port owner */
#define EHCI_PS_PP		(1 << 12)	/* RW,RO port power */
#define EHCI_PS_LS		(3 << 10)	/* RO line status */
#define EHCI_PS_PR		(1 << 8)	/* RW port reset */
#define EHCI_PS_SUSP		(1 << 7)	/* RW suspend */
#define EHCI_PS_FPR		(1 << 6)	/* RW force port resume */
#define EHCI_PS_OCC		(1 << 5)	/* RWC over current change */
#define EHCI_PS_OCA		(1 << 4)	/* RO over current active */
#define EHCI_PS_PEC		(1 << 3)	/* RWC port enable change */
#define EHCI_PS_PE		(1 << 2)	/* RW port enable */
#define EHCI_PS_CSC		(1 << 1)	/* RWC connect status change */
#define EHCI_PS_CS		(1 << 0)	/* RO connect status */
#define EHCI_PS_CLEAR		(EHCI_PS_OCC | EHCI_PS_PEC | EHCI_PS_CSC)

#define EHCI_PS_IS_LOWSPEED(x)	(((x) & EHCI_PS_LS) == (1 << 10))

/*
 * Schedule Interface Space.
 *
 * IMPORTANT: Software must ensure that no interface data structure
 * reachable by the EHCI host controller spans a 4K page boundary!
 *
 * Periodic transfers (i.e. isochronous and interrupt transfers) are
 * not supported.
 */

/* Queue Element Transfer Descriptor (qTD). */
struct qTD {
	/* this part defined by EHCI spec */
	uint32_t qt_next;			/* see EHCI 3.5.1 */
#define	QT_NEXT_TERMINATE	1
	uint32_t qt_altnext;			/* see EHCI 3.5.2 */
	uint32_t qt_token;			/* see EHCI 3.5.3 */
#define QT_TOKEN_DT(x)		(((x) & 0x1) << 31)	/* Data Toggle */
#define QT_TOKEN_GET_DT(x)		(((x) >> 31) & 0x1)
#define QT_TOKEN_TOTALBYTES(x)	(((x) & 0x7fff) << 16)	/* Total Bytes to Transfer */
#define QT_TOKEN_GET_TOTALBYTES(x)	(((x) >> 16) & 0x7fff)
#define QT_TOKEN_IOC(x)		(((x) & 0x1) << 15)	/* Interrupt On Complete */
#define QT_TOKEN_CPAGE(x)	(((x) & 0x7) << 12)	/* Current Page */
#define QT_TOKEN_CERR(x)	(((x) & 0x3) << 10)	/* Error Counter */
#define QT_TOKEN_PID(x)		(((x) & 0x3) << 8)	/* PID Code */
#define QT_TOKEN_PID_OUT		0x0
#define QT_TOKEN_PID_IN			0x1
#define QT_TOKEN_PID_SETUP		0x2
#define QT_TOKEN_STATUS(x)	(((x) & 0xff) << 0)	/* Status */
#define QT_TOKEN_GET_STATUS(x)		(((x) >> 0) & 0xff)
#define QT_TOKEN_STATUS_ACTIVE		0x80
#define QT_TOKEN_STATUS_HALTED		0x40
#define QT_TOKEN_STATUS_DATBUFERR	0x20
#define QT_TOKEN_STATUS_BABBLEDET	0x10
#define QT_TOKEN_STATUS_XACTERR		0x08
#define QT_TOKEN_STATUS_MISSEDUFRAME	0x04
#define QT_TOKEN_STATUS_SPLITXSTATE	0x02
#define QT_TOKEN_STATUS_PERR		0x01
#define QT_BUFFER_CNT		5
	uint32_t qt_buffer[QT_BUFFER_CNT];	/* see EHCI 3.5.4 */
	uint32_t qt_buffer_hi[QT_BUFFER_CNT];	/* Appendix B */
	/* pad struct for 32 byte alignment */
	uint32_t unused[3];
};

#define EHCI_PAGE_SIZE		4096

/* Queue Head (QH). */
struct QH {
	uint32_t qh_link;
#define	QH_LINK_TERMINATE	1
#define	QH_LINK_TYPE_ITD	0
#define	QH_LINK_TYPE_QH		2
#define	QH_LINK_TYPE_SITD	4
#define	QH_LINK_TYPE_FSTN	6
	uint32_t qh_endpt1;
#define QH_ENDPT1_RL(x)		(((x) & 0xf) << 28)	/* NAK Count Reload */
#define QH_ENDPT1_C(x)		(((x) & 0x1) << 27)	/* Control Endpoint Flag */
#define QH_ENDPT1_MAXPKTLEN(x)	(((x) & 0x7ff) << 16)	/* Maximum Packet Length */
#define QH_ENDPT1_H(x)		(((x) & 0x1) << 15)	/* Head of Reclamation List Flag */
#define QH_ENDPT1_DTC(x)	(((x) & 0x1) << 14)	/* Data Toggle Control */
#define QH_ENDPT1_DTC_IGNORE_QTD_TD	0x0
#define QH_ENDPT1_DTC_DT_FROM_QTD	0x1
#define QH_ENDPT1_EPS(x)	(((x) & 0x3) << 12)	/* Endpoint Speed */
#define QH_ENDPT1_EPS_FS		0x0
#define QH_ENDPT1_EPS_LS		0x1
#define QH_ENDPT1_EPS_HS		0x2
#define QH_ENDPT1_ENDPT(x)	(((x) & 0xf) << 8)	/* Endpoint Number */
#define QH_ENDPT1_I(x)		(((x) & 0x1) << 7)	/* Inactivate on Next Transaction */
#define QH_ENDPT1_DEVADDR(x)	(((x) & 0x7f) << 0)	/* Device Address */
	uint32_t qh_endpt2;
#define QH_ENDPT2_MULT(x)	(((x) & 0x3) << 30)	/* High-Bandwidth Pipe Multiplier */
#define QH_ENDPT2_PORTNUM(x)	(((x) & 0x7f) << 23)	/* Port Number */
#define QH_ENDPT2_HUBADDR(x)	(((x) & 0x7f) << 16)	/* Hub Address */
#define QH_ENDPT2_UFCMASK(x)	(((x) & 0xff) << 8)	/* Split Completion Mask */
#define QH_ENDPT2_UFSMASK(x)	(((x) & 0xff) << 0)	/* Interrupt Schedule Mask */
	uint32_t qh_curtd;
	struct qTD qh_overlay;
	/*
	 * Add dummy fill value to make the size of this struct
	 * aligned to 32 bytes
	 */
	union {
		uint32_t fill[4];
		void *buffer;
	};
};

/* Tweak flags for EHCI, used to control operation */
enum {
	/* don't use or_configflag in init */
	EHCI_TWEAK_NO_INIT_CF		= 1 << 0,
};

struct ehci_ctrl;

struct ehci_ops {
	void (*set_usb_mode)(struct ehci_ctrl *ctrl);
	int (*get_port_speed)(struct ehci_ctrl *ctrl, uint32_t reg);
	void (*powerup_fixup)(struct ehci_ctrl *ctrl, uint32_t *status_reg,
			      uint32_t *reg);
	uint32_t *(*get_portsc_register)(struct ehci_ctrl *ctrl, int port);
	int (*init_after_reset)(struct ehci_ctrl *ctrl);
};

struct ehci_ctrl {
	enum usb_init_type init;
	struct ehci_hccr *hccr;	/* R/O registers, not need for volatile */
	struct ehci_hcor *hcor;
	int rootdev;
	uint16_t portreset;
	struct QH qh_list __aligned(USB_DMA_MINALIGN);
	struct QH periodic_queue __aligned(USB_DMA_MINALIGN);
	uint32_t *periodic_list;
	int periodic_schedules;
	int ntds;
	bool has_fsl_erratum_a005275;	/* Freescale HS silicon quirk */
	bool async_locked;
	struct ehci_ops ops;
	void *priv;	/* client's private data */
};

/**
 * ehci_set_controller_info() - Set up private data for the controller
 *
 * This function can be called in ehci_hcd_init() to tell the EHCI layer
 * about the controller's private data pointer. Then in the above functions
 * this can be accessed given the struct ehci_ctrl pointer. Also special
 * EHCI operation methods can be provided if required
 *
 * @index:	Controller number to set
 * @priv:	Controller pointer
 * @ops:	Controller operations, or NULL to use default
 */
void ehci_set_controller_priv(int index, void *priv,
			      const struct ehci_ops *ops);

/**
 * ehci_get_controller_priv() - Get controller private data
 *
 * @index	Controller number to get
 * Return: controller pointer for this index
 */
void *ehci_get_controller_priv(int index);

/* Low level init functions */
int ehci_hcd_init(int index, enum usb_init_type init,
		struct ehci_hccr **hccr, struct ehci_hcor **hcor);
int ehci_hcd_stop(int index);

int ehci_register(struct udevice *dev, struct ehci_hccr *hccr,
		  struct ehci_hcor *hcor, const struct ehci_ops *ops,
		  uint tweaks, enum usb_init_type init);
int ehci_deregister(struct udevice *dev);
extern struct dm_usb_ops ehci_usb_ops;

#include <linux/bitops.h>
#endif /* USB_EHCI_H */
