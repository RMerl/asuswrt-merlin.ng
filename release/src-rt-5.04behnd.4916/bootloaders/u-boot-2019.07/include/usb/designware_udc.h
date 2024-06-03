/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2009
 * Vipin Kumar, ST Micoelectronics, vipin.kumar@st.com.
 */

#ifndef __DW_UDC_H
#define __DW_UDC_H

/*
 * Defines for  USBD
 *
 * The udc_ahb controller has three AHB slaves:
 *
 * 1.  THe UDC registers
 * 2.  The plug detect
 * 3.  The RX/TX FIFO
 */

#define MAX_ENDPOINTS		16

struct udc_endp_regs {
	u32 endp_cntl;
	u32 endp_status;
	u32 endp_bsorfn;
	u32 endp_maxpacksize;
	u32 reserved_1;
	u32 endp_desc_point;
	u32 reserved_2;
	u32 write_done;
};

/* Endpoint Control Register definitions */

#define  ENDP_CNTL_STALL		0x00000001
#define  ENDP_CNTL_FLUSH		0x00000002
#define  ENDP_CNTL_SNOOP		0x00000004
#define  ENDP_CNTL_POLL			0x00000008
#define  ENDP_CNTL_CONTROL		0x00000000
#define  ENDP_CNTL_ISO			0x00000010
#define  ENDP_CNTL_BULK			0x00000020
#define  ENDP_CNTL_INT			0x00000030
#define  ENDP_CNTL_NAK			0x00000040
#define  ENDP_CNTL_SNAK			0x00000080
#define  ENDP_CNTL_CNAK			0x00000100
#define  ENDP_CNTL_RRDY			0x00000200

/* Endpoint Satus Register definitions */

#define  ENDP_STATUS_PIDMSK		0x0000000f
#define  ENDP_STATUS_OUTMSK		0x00000030
#define  ENDP_STATUS_OUT_NONE		0x00000000
#define  ENDP_STATUS_OUT_DATA		0x00000010
#define  ENDP_STATUS_OUT_SETUP		0x00000020
#define  ENDP_STATUS_IN			0x00000040
#define  ENDP_STATUS_BUFFNAV		0x00000080
#define  ENDP_STATUS_FATERR		0x00000100
#define  ENDP_STATUS_HOSTBUSERR		0x00000200
#define  ENDP_STATUS_TDC		0x00000400
#define  ENDP_STATUS_RXPKTMSK		0x003ff800

struct udc_regs {
	struct udc_endp_regs in_regs[MAX_ENDPOINTS];
	struct udc_endp_regs out_regs[MAX_ENDPOINTS];
	u32 dev_conf;
	u32 dev_cntl;
	u32 dev_stat;
	u32 dev_int;
	u32 dev_int_mask;
	u32 endp_int;
	u32 endp_int_mask;
	u32 reserved_3[0x39];
	u32 reserved_4;		/* offset 0x500 */
	u32 udc_endp_reg[MAX_ENDPOINTS];
};

/* Device Configuration Register definitions */

#define  DEV_CONF_HS_SPEED		0x00000000
#define  DEV_CONF_LS_SPEED		0x00000002
#define  DEV_CONF_FS_SPEED		0x00000003
#define  DEV_CONF_REMWAKEUP		0x00000004
#define  DEV_CONF_SELFPOW		0x00000008
#define  DEV_CONF_SYNCFRAME		0x00000010
#define  DEV_CONF_PHYINT_8		0x00000020
#define  DEV_CONF_PHYINT_16		0x00000000
#define  DEV_CONF_UTMI_BIDIR		0x00000040
#define  DEV_CONF_STATUS_STALL		0x00000080

/* Device Control Register definitions */

#define  DEV_CNTL_RESUME		0x00000001
#define  DEV_CNTL_TFFLUSH		0x00000002
#define  DEV_CNTL_RXDMAEN		0x00000004
#define  DEV_CNTL_TXDMAEN		0x00000008
#define  DEV_CNTL_DESCRUPD		0x00000010
#define  DEV_CNTL_BIGEND		0x00000020
#define  DEV_CNTL_BUFFILL		0x00000040
#define  DEV_CNTL_TSHLDEN		0x00000080
#define  DEV_CNTL_BURSTEN		0x00000100
#define  DEV_CNTL_DMAMODE		0x00000200
#define  DEV_CNTL_SOFTDISCONNECT	0x00000400
#define  DEV_CNTL_SCALEDOWN		0x00000800
#define  DEV_CNTL_BURSTLENU		0x00010000
#define  DEV_CNTL_BURSTLENMSK		0x00ff0000
#define  DEV_CNTL_TSHLDLENU		0x01000000
#define  DEV_CNTL_TSHLDLENMSK		0xff000000

/* Device Status Register definitions */

#define  DEV_STAT_CFG			0x0000000f
#define  DEV_STAT_INTF			0x000000f0
#define  DEV_STAT_ALT			0x00000f00
#define  DEV_STAT_SUSP			0x00001000
#define  DEV_STAT_ENUM			0x00006000
#define  DEV_STAT_ENUM_SPEED_HS		0x00000000
#define  DEV_STAT_ENUM_SPEED_FS		0x00002000
#define  DEV_STAT_ENUM_SPEED_LS		0x00004000
#define  DEV_STAT_RXFIFO_EMPTY		0x00008000
#define  DEV_STAT_PHY_ERR		0x00010000
#define  DEV_STAT_TS			0xf0000000

/* Device Interrupt Register definitions */

#define  DEV_INT_MSK			0x0000007f
#define  DEV_INT_SETCFG			0x00000001
#define  DEV_INT_SETINTF		0x00000002
#define  DEV_INT_INACTIVE		0x00000004
#define  DEV_INT_USBRESET		0x00000008
#define  DEV_INT_SUSPUSB		0x00000010
#define  DEV_INT_SOF			0x00000020
#define  DEV_INT_ENUM			0x00000040

/* Endpoint Interrupt Register definitions */

#define  ENDP0_INT_CTRLIN		0x00000001
#define  ENDP1_INT_BULKIN		0x00000002
#define  ENDP_INT_NONISOIN_MSK		0x0000AAAA
#define  ENDP2_INT_BULKIN		0x00000004
#define  ENDP0_INT_CTRLOUT		0x00010000
#define  ENDP1_INT_BULKOUT		0x00020000
#define  ENDP2_INT_BULKOUT		0x00040000
#define  ENDP_INT_NONISOOUT_MSK		0x55540000

/* Endpoint Register definitions */
#define  ENDP_EPDIR_OUT			0x00000000
#define  ENDP_EPDIR_IN			0x00000010
#define  ENDP_EPTYPE_CNTL		0x0
#define  ENDP_EPTYPE_ISO		0x1
#define  ENDP_EPTYPE_BULK		0x2
#define  ENDP_EPTYPE_INT		0x3

/*
 * Defines for Plug Detect
 */

struct plug_regs {
	u32 plug_state;
	u32 plug_pending;
};

/* Plug State Register definitions */
#define  PLUG_STATUS_EN			0x1
#define  PLUG_STATUS_ATTACHED		0x2
#define  PLUG_STATUS_PHY_RESET		0x4
#define  PLUG_STATUS_PHY_MODE		0x8

/*
 * Defines for UDC FIFO (Slave Mode)
 */
struct udcfifo_regs {
	u32 *fifo_p;
};

/*
 * UDC endpoint definitions
 */
#define  UDC_EP0			0
#define  UDC_EP1			1
#define  UDC_EP2			2
#define  UDC_EP3			3

#endif /* __DW_UDC_H */
