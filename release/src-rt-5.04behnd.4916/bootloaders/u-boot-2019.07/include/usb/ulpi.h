/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Generic ULPI interface.
 *
 * Copyright (C) 2011 Jana Rapava <fermata7@gmail.com>
 * Copyright (C) 2011 CompuLab, Ltd. <www.compulab.co.il>
 *
 * Authors: Jana Rapava <fermata7@gmail.com>
 *	    Igor Grinberg <grinberg@compulab.co.il>
 *
 * Register offsets taken from:
 * linux/include/linux/usb/ulpi.h
 *
 * Original Copyrights follow:
 * Copyright (C) 2010 Nokia Corporation
 */

#ifndef __USB_ULPI_H__
#define __USB_ULPI_H__

#define ULPI_ERROR	(1 << 8) /* overflow from any register value */

#ifndef CONFIG_USB_ULPI_TIMEOUT
#define CONFIG_USB_ULPI_TIMEOUT 1000	/* timeout in us */
#endif

/*
 * ulpi view port address and
 * Port_number that can be passed.
 * Any additional data to be passed can
 * be extended from this structure
 */
struct ulpi_viewport {
	uintptr_t viewport_addr;
	u32 port_num;
};

/*
 * Initialize the ULPI transciever and check the interface integrity.
 * @ulpi_vp -  structure containing ULPI viewport data
 *
 * returns 0 on success, ULPI_ERROR on failure.
 */
int ulpi_init(struct ulpi_viewport *ulpi_vp);

/*
 * Select transceiver speed.
 * @speed	- ULPI_FC_HIGH_SPEED, ULPI_FC_FULL_SPEED (default),
 *                ULPI_FC_LOW_SPEED,  ULPI_FC_FS4LS
 * returns 0 on success, ULPI_ERROR on failure.
 */
int ulpi_select_transceiver(struct ulpi_viewport *ulpi_vp, unsigned speed);

/*
 * Enable/disable VBUS.
 * @ext_power		- external VBUS supply is used (default is false)
 * @ext_indicator	- external VBUS over-current indicator is used
 *
 * returns 0 on success, ULPI_ERROR on failure.
 */
int ulpi_set_vbus(struct ulpi_viewport *ulpi_vp, int on, int ext_power);

/*
 * Configure VBUS indicator
 * @external		- external VBUS over-current indicator is used
 * @passthru		- disables ANDing of internal VBUS comparator
 *                    with external VBUS input
 * @complement		- inverts the external VBUS input
 */
int ulpi_set_vbus_indicator(struct ulpi_viewport *ulpi_vp, int external,
			int passthru, int complement);

/*
 * Enable/disable pull-down resistors on D+ and D- USB lines.
 *
 * returns 0 on success, ULPI_ERROR on failure.
 */
int ulpi_set_pd(struct ulpi_viewport *ulpi_vp, int enable);

/*
 * Select OpMode.
 * @opmode	- ULPI_FC_OPMODE_NORMAL (default), ULPI_FC_OPMODE_NONDRIVING,
 *		  ULPI_FC_OPMODE_DISABLE_NRZI,	   ULPI_FC_OPMODE_NOSYNC_NOEOP
 *
 * returns 0 on success, ULPI_ERROR on failure.
 */
int ulpi_opmode_sel(struct ulpi_viewport *ulpi_vp, unsigned opmode);

/*
 * Switch to Serial Mode.
 * @smode	- ULPI_IFACE_6_PIN_SERIAL_MODE or ULPI_IFACE_3_PIN_SERIAL_MODE
 *
 * returns 0 on success, ULPI_ERROR on failure.
 *
 * Notes:
 * Switches immediately to Serial Mode.
 * To return from Serial Mode, STP line needs to be asserted.
 */
int ulpi_serial_mode_enable(struct ulpi_viewport *ulpi_vp, unsigned smode);

/*
 * Put PHY into low power mode.
 *
 * returns 0 on success, ULPI_ERROR on failure.
 *
 * Notes:
 * STP line must be driven low to keep the PHY in suspend.
 * To resume the PHY, STP line needs to be asserted.
 */
int ulpi_suspend(struct ulpi_viewport *ulpi_vp);

/*
 * Reset the transceiver. ULPI interface and registers are not affected.
 *
 * returns 0 on success, ULPI_ERROR on failure.
 */
int ulpi_reset(struct ulpi_viewport *ulpi_vp);


/* ULPI access methods below must be implemented for each ULPI viewport. */

/*
 * Write to the ULPI PHY register via the viewport.
 * @reg		- the ULPI register (one of the fields in struct ulpi_regs).
 *		  Due to ULPI design, only 8 lsb of address are used.
 * @value	- the value - only 8 lower bits are used, others ignored.
 *
 * returns 0 on success, ULPI_ERROR on failure.
 */
int ulpi_write(struct ulpi_viewport *ulpi_vp, u8 *reg, u32 value);

/*
 * Read the ULPI PHY register content via the viewport.
 * @reg		- the ULPI register (one of the fields in struct ulpi_regs).
 *		  Due to ULPI design, only 8 lsb of address are used.
 *
 * returns register content on success, ULPI_ERROR on failure.
 */
u32 ulpi_read(struct ulpi_viewport *ulpi_vp, u8 *reg);

/*
 * Wait for the reset to complete.
 * The Link must not attempt to access the PHY until the reset has
 * completed and DIR line is de-asserted.
 */
int ulpi_reset_wait(struct ulpi_viewport *ulpi_vp);

/* Access Extended Register Set (indicator) */
#define ACCESS_EXT_REGS_OFFSET	0x2f	/* read-write */
/* Vendor-specific */
#define VENDOR_SPEC_OFFSET	0x30

/*
 * Extended Register Set
 *
 * Addresses 0x00-0x3F map directly to Immediate Register Set.
 * Addresses 0x40-0x7F are reserved.
 * Addresses 0x80-0xff are vendor-specific.
 */
#define EXT_VENDOR_SPEC_OFFSET	0x80

/* ULPI registers, bits and offsets definitions */
struct ulpi_regs {
	/* Vendor ID and Product ID: 0x00 - 0x03 Read-only */
	u8	vendor_id_low;
	u8	vendor_id_high;
	u8	product_id_low;
	u8	product_id_high;
	/* Function Control: 0x04 - 0x06 Read */
	u8	function_ctrl;		/* 0x04 Write */
	u8	function_ctrl_set;	/* 0x05 Set */
	u8	function_ctrl_clear;	/* 0x06 Clear */
	/* Interface Control: 0x07 - 0x09 Read */
	u8	iface_ctrl;		/* 0x07 Write */
	u8	iface_ctrl_set;		/* 0x08 Set */
	u8	iface_ctrl_clear;	/* 0x09 Clear */
	/* OTG Control: 0x0A - 0x0C Read */
	u8	otg_ctrl;		/* 0x0A Write */
	u8	otg_ctrl_set;		/* 0x0B Set */
	u8	otg_ctrl_clear;		/* 0x0C Clear */
	/* USB Interrupt Enable Rising: 0x0D - 0x0F Read */
	u8	usb_ie_rising;		/* 0x0D Write */
	u8	usb_ie_rising_set;	/* 0x0E Set */
	u8	usb_ie_rising_clear;	/* 0x0F Clear */
	/* USB Interrupt Enable Falling: 0x10 - 0x12 Read */
	u8	usb_ie_falling;		/* 0x10 Write */
	u8	usb_ie_falling_set;	/* 0x11 Set */
	u8	usb_ie_falling_clear;	/* 0x12 Clear */
	/* USB Interrupt Status: 0x13 Read-only */
	u8	usb_int_status;
	/* USB Interrupt Latch: 0x14 Read-only with auto-clear */
	u8	usb_int_latch;
	/* Debug: 0x15 Read-only */
	u8	debug;
	/* Scratch Register: 0x16 - 0x18 Read */
	u8	scratch;		/* 0x16 Write */
	u8	scratch_set;		/* 0x17 Set */
	u8	scratch_clear;		/* 0x18 Clear */
	/*
	 * Optional Carkit registers:
	 * Carkit Control: 0x19 - 0x1B Read
	 */
	u8	carkit_ctrl;		/* 0x19 Write */
	u8	carkit_ctrl_set;	/* 0x1A Set */
	u8	carkit_ctrl_clear;	/* 0x1B Clear */
	/* Carkit Interrupt Delay: 0x1C Read, Write */
	u8	carkit_int_delay;
	/* Carkit Interrupt Enable: 0x1D - 0x1F Read */
	u8	carkit_ie;		/* 0x1D Write */
	u8	carkit_ie_set;		/* 0x1E Set */
	u8	carkit_ie_clear;	/* 0x1F Clear */
	/* Carkit Interrupt Status: 0x20 Read-only */
	u8	carkit_int_status;
	/* Carkit Interrupt Latch: 0x21 Read-only with auto-clear */
	u8	carkit_int_latch;
	/* Carkit Pulse Control: 0x22 - 0x24 Read */
	u8	carkit_pulse_ctrl;		/* 0x22 Write */
	u8	carkit_pulse_ctrl_set;		/* 0x23 Set */
	u8	carkit_pulse_ctrl_clear;	/* 0x24 Clear */
	/*
	 * Other optional registers:
	 * Transmit Positive Width: 0x25 Read, Write
	 */
	u8	transmit_pos_width;
	/* Transmit Negative Width: 0x26 Read, Write */
	u8	transmit_neg_width;
	/* Receive Polarity Recovery: 0x27 Read, Write */
	u8	recv_pol_recovery;
	/*
	 * Addresses 0x28 - 0x2E are reserved, so we use offsets
	 * for immediate registers with higher addresses
	 */
};

/*
 * Register Bits
 */

/* Function Control */
#define ULPI_FC_XCVRSEL_MASK		(3 << 0)
#define ULPI_FC_HIGH_SPEED		(0 << 0)
#define ULPI_FC_FULL_SPEED		(1 << 0)
#define ULPI_FC_LOW_SPEED		(2 << 0)
#define ULPI_FC_FS4LS			(3 << 0)
#define ULPI_FC_TERMSELECT		(1 << 2)
#define ULPI_FC_OPMODE_MASK		(3 << 3)
#define ULPI_FC_OPMODE_NORMAL		(0 << 3)
#define ULPI_FC_OPMODE_NONDRIVING	(1 << 3)
#define ULPI_FC_OPMODE_DISABLE_NRZI	(2 << 3)
#define ULPI_FC_OPMODE_NOSYNC_NOEOP	(3 << 3)
#define ULPI_FC_RESET			(1 << 5)
#define ULPI_FC_SUSPENDM		(1 << 6)

/* Interface Control */
#define ULPI_IFACE_6_PIN_SERIAL_MODE	(1 << 0)
#define ULPI_IFACE_3_PIN_SERIAL_MODE	(1 << 1)
#define ULPI_IFACE_CARKITMODE		(1 << 2)
#define ULPI_IFACE_CLOCKSUSPENDM	(1 << 3)
#define ULPI_IFACE_AUTORESUME		(1 << 4)
#define ULPI_IFACE_EXTVBUS_COMPLEMENT	(1 << 5)
#define ULPI_IFACE_PASSTHRU		(1 << 6)
#define ULPI_IFACE_PROTECT_IFC_DISABLE	(1 << 7)

/* OTG Control */
#define ULPI_OTG_ID_PULLUP		(1 << 0)
#define ULPI_OTG_DP_PULLDOWN		(1 << 1)
#define ULPI_OTG_DM_PULLDOWN		(1 << 2)
#define ULPI_OTG_DISCHRGVBUS		(1 << 3)
#define ULPI_OTG_CHRGVBUS		(1 << 4)
#define ULPI_OTG_DRVVBUS		(1 << 5)
#define ULPI_OTG_DRVVBUS_EXT		(1 << 6)
#define ULPI_OTG_EXTVBUSIND		(1 << 7)

/*
 * USB Interrupt Enable Rising,
 * USB Interrupt Enable Falling,
 * USB Interrupt Status and
 * USB Interrupt Latch
 */
#define ULPI_INT_HOST_DISCONNECT	(1 << 0)
#define ULPI_INT_VBUS_VALID		(1 << 1)
#define ULPI_INT_SESS_VALID		(1 << 2)
#define ULPI_INT_SESS_END		(1 << 3)
#define ULPI_INT_IDGRD			(1 << 4)

/* Debug */
#define ULPI_DEBUG_LINESTATE0		(1 << 0)
#define ULPI_DEBUG_LINESTATE1		(1 << 1)

/* Carkit Control */
#define ULPI_CARKIT_CTRL_CARKITPWR		(1 << 0)
#define ULPI_CARKIT_CTRL_IDGNDDRV		(1 << 1)
#define ULPI_CARKIT_CTRL_TXDEN			(1 << 2)
#define ULPI_CARKIT_CTRL_RXDEN			(1 << 3)
#define ULPI_CARKIT_CTRL_SPKLEFTEN		(1 << 4)
#define ULPI_CARKIT_CTRL_SPKRIGHTEN		(1 << 5)
#define ULPI_CARKIT_CTRL_MICEN			(1 << 6)

/* Carkit Interrupt Enable */
#define ULPI_CARKIT_INT_EN_IDFLOAT_RISE		(1 << 0)
#define ULPI_CARKIT_INT_EN_IDFLOAT_FALL		(1 << 1)
#define ULPI_CARKIT_INT_EN_CARINTDET		(1 << 2)
#define ULPI_CARKIT_INT_EN_DP_RISE		(1 << 3)
#define ULPI_CARKIT_INT_EN_DP_FALL		(1 << 4)

/* Carkit Interrupt Status and Latch */
#define ULPI_CARKIT_INT_IDFLOAT			(1 << 0)
#define ULPI_CARKIT_INT_CARINTDET		(1 << 1)
#define ULPI_CARKIT_INT_DP			(1 << 2)

/* Carkit Pulse Control*/
#define ULPI_CARKIT_PLS_CTRL_TXPLSEN		(1 << 0)
#define ULPI_CARKIT_PLS_CTRL_RXPLSEN		(1 << 1)
#define ULPI_CARKIT_PLS_CTRL_SPKRLEFT_BIASEN	(1 << 2)
#define ULPI_CARKIT_PLS_CTRL_SPKRRIGHT_BIASEN	(1 << 3)


#endif /* __USB_ULPI_H__ */
