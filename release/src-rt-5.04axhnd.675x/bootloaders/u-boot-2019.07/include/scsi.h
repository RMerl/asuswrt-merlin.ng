/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2001
 * Denis Peter, MPL AG Switzerland
 */
 #ifndef _SCSI_H
 #define _SCSI_H

struct scsi_cmd {
	unsigned char		cmd[16];					/* command				   */
	/* for request sense */
	unsigned char		sense_buf[64]
		__attribute__((aligned(ARCH_DMA_MINALIGN)));
	unsigned char		status;						/* SCSI Status			 */
	unsigned char		target;						/* Target ID				 */
	unsigned char		lun;							/* Target LUN        */
	unsigned char		cmdlen;						/* command len				*/
	unsigned long		datalen;					/* Total data length	*/
	unsigned char	*	pdata;						/* pointer to data		*/
	unsigned char		msgout[12];				/* Messge out buffer (NOT USED) */
	unsigned char		msgin[12];				/* Message in buffer	*/
	unsigned char		sensecmdlen;			/* Sense command len	*/
	unsigned long		sensedatalen;			/* Sense data len			*/
	unsigned char		sensecmd[6];			/* Sense command			*/
	unsigned long		contr_stat;				/* Controller Status	*/
	unsigned long		trans_bytes;			/* tranfered bytes		*/

	unsigned int		priv;
};

/*-----------------------------------------------------------
**
**	SCSI  constants.
**
**-----------------------------------------------------------
*/

/*
**	Messages
*/

#define	M_COMPLETE	(0x00)
#define	M_EXTENDED	(0x01)
#define	M_SAVE_DP	(0x02)
#define	M_RESTORE_DP	(0x03)
#define	M_DISCONNECT	(0x04)
#define	M_ID_ERROR	(0x05)
#define	M_ABORT		(0x06)
#define	M_REJECT	(0x07)
#define	M_NOOP		(0x08)
#define	M_PARITY	(0x09)
#define	M_LCOMPLETE	(0x0a)
#define	M_FCOMPLETE	(0x0b)
#define	M_RESET		(0x0c)
#define	M_ABORT_TAG	(0x0d)
#define	M_CLEAR_QUEUE	(0x0e)
#define	M_INIT_REC	(0x0f)
#define	M_REL_REC	(0x10)
#define	M_TERMINATE	(0x11)
#define	M_SIMPLE_TAG	(0x20)
#define	M_HEAD_TAG	(0x21)
#define	M_ORDERED_TAG	(0x22)
#define	M_IGN_RESIDUE	(0x23)
#define	M_IDENTIFY	(0x80)

#define	M_X_MODIFY_DP	(0x00)
#define	M_X_SYNC_REQ	(0x01)
#define	M_X_WIDE_REQ	(0x03)
#define	M_X_PPR_REQ	(0x04)


/*
**	Status
*/

#define	S_GOOD		(0x00)
#define	S_CHECK_COND	(0x02)
#define	S_COND_MET	(0x04)
#define	S_BUSY		(0x08)
#define	S_INT		(0x10)
#define	S_INT_COND_MET	(0x14)
#define	S_CONFLICT	(0x18)
#define	S_TERMINATED	(0x20)
#define	S_QUEUE_FULL	(0x28)
#define	S_ILLEGAL	(0xff)
#define	S_SENSE		(0x80)

/*
 * Sense_keys
 */

#define SENSE_NO_SENSE				0x0
#define SENSE_RECOVERED_ERROR	0x1
#define SENSE_NOT_READY				0x2
#define SENSE_MEDIUM_ERROR		0x3
#define SENSE_HARDWARE_ERROR	0x4
#define SENSE_ILLEGAL_REQUEST	0x5
#define SENSE_UNIT_ATTENTION	0x6
#define SENSE_DATA_PROTECT		0x7
#define SENSE_BLANK_CHECK			0x8
#define SENSE_VENDOR_SPECIFIC	0x9
#define SENSE_COPY_ABORTED		0xA
#define SENSE_ABORTED_COMMAND	0xB
#define SENSE_VOLUME_OVERFLOW	0xD
#define SENSE_MISCOMPARE			0xE


#define SCSI_CHANGE_DEF	0x40		/* Change Definition (Optional) */
#define SCSI_COMPARE		0x39		/* Compare (O) */
#define SCSI_COPY			0x18		/* Copy (O) */
#define SCSI_COP_VERIFY	0x3A		/* Copy and Verify (O) */
#define SCSI_INQUIRY		0x12		/* Inquiry (MANDATORY) */
#define SCSI_LOG_SELECT	0x4C		/* Log Select (O) */
#define SCSI_LOG_SENSE	0x4D		/* Log Sense (O) */
#define SCSI_MODE_SEL6	0x15		/* Mode Select 6-byte (Device Specific) */
#define SCSI_MODE_SEL10	0x55		/* Mode Select 10-byte (Device Specific) */
#define SCSI_MODE_SEN6	0x1A		/* Mode Sense 6-byte (Device Specific) */
#define SCSI_MODE_SEN10	0x5A		/* Mode Sense 10-byte (Device Specific) */
#define SCSI_READ_BUFF	0x3C		/* Read Buffer (O) */
#define SCSI_REQ_SENSE	0x03		/* Request Sense (MANDATORY) */
#define SCSI_SEND_DIAG	0x1D		/* Send Diagnostic (O) */
#define SCSI_TST_U_RDY	0x00		/* Test Unit Ready (MANDATORY) */
#define SCSI_WRITE_BUFF	0x3B		/* Write Buffer (O) */
/***************************************************************************
 *			  %%% Commands Unique to Direct Access Devices %%%
 ***************************************************************************/
#define SCSI_COMPARE		0x39		/* Compare (O) */
#define SCSI_FORMAT		0x04		/* Format Unit (MANDATORY) */
#define SCSI_LCK_UN_CAC	0x36		/* Lock Unlock Cache (O) */
#define SCSI_PREFETCH	0x34		/* Prefetch (O) */
#define SCSI_MED_REMOVL	0x1E		/* Prevent/Allow medium Removal (O) */
#define SCSI_READ6		0x08		/* Read 6-byte (MANDATORY) */
#define SCSI_READ10		0x28		/* Read 10-byte (MANDATORY) */
#define SCSI_READ16	0x48
#define SCSI_RD_CAPAC	0x25		/* Read Capacity (MANDATORY) */
#define SCSI_RD_CAPAC10	SCSI_RD_CAPAC	/* Read Capacity (10) */
#define SCSI_RD_CAPAC16	0x9e		/* Read Capacity (16) */
#define SCSI_RD_DEFECT	0x37		/* Read Defect Data (O) */
#define SCSI_READ_LONG	0x3E		/* Read Long (O) */
#define SCSI_REASS_BLK	0x07		/* Reassign Blocks (O) */
#define SCSI_RCV_DIAG	0x1C		/* Receive Diagnostic Results (O) */
#define SCSI_RELEASE	0x17		/* Release Unit (MANDATORY) */
#define SCSI_REZERO		0x01		/* Rezero Unit (O) */
#define SCSI_SRCH_DAT_E	0x31		/* Search Data Equal (O) */
#define SCSI_SRCH_DAT_H	0x30		/* Search Data High (O) */
#define SCSI_SRCH_DAT_L	0x32		/* Search Data Low (O) */
#define SCSI_SEEK6		0x0B		/* Seek 6-Byte (O) */
#define SCSI_SEEK10		0x2B		/* Seek 10-Byte (O) */
#define SCSI_SEND_DIAG	0x1D		/* Send Diagnostics (MANDATORY) */
#define SCSI_SET_LIMIT	0x33		/* Set Limits (O) */
#define SCSI_START_STP	0x1B		/* Start/Stop Unit (O) */
#define SCSI_SYNC_CACHE	0x35		/* Synchronize Cache (O) */
#define SCSI_VERIFY		0x2F		/* Verify (O) */
#define SCSI_WRITE6		0x0A		/* Write 6-Byte (MANDATORY) */
#define SCSI_WRITE10	0x2A		/* Write 10-Byte (MANDATORY) */
#define SCSI_WRT_VERIFY	0x2E		/* Write and Verify (O) */
#define SCSI_WRITE_LONG	0x3F		/* Write Long (O) */
#define SCSI_WRITE_SAME	0x41		/* Write Same (O) */

/**
 * struct scsi_platdata - stores information about SCSI controller
 *
 * @base: Controller base address
 * @max_lun: Maximum number of logical units
 * @max_id: Maximum number of target ids
 */
struct scsi_platdata {
	unsigned long base;
	unsigned long max_lun;
	unsigned long max_id;
};

/* Operations for SCSI */
struct scsi_ops {
	/**
	 * exec() - execute a command
	 *
	 * @dev:	SCSI bus
	 * @cmd:	Command to execute
	 * @return 0 if OK, -ve on error
	 */
	int (*exec)(struct udevice *dev, struct scsi_cmd *cmd);

	/**
	 * bus_reset() - reset the bus
	 *
	 * @dev:	SCSI bus to reset
	 * @return 0 if OK, -ve on error
	 */
	int (*bus_reset)(struct udevice *dev);
};

#define scsi_get_ops(dev)        ((struct scsi_ops *)(dev)->driver->ops)

extern struct scsi_ops scsi_ops;

/**
 * scsi_exec() - execute a command
 *
 * @dev:	SCSI bus
 * @cmd:	Command to execute
 * @return 0 if OK, -ve on error
 */
int scsi_exec(struct udevice *dev, struct scsi_cmd *cmd);

/**
 * scsi_bus_reset() - reset the bus
 *
 * @dev:	SCSI bus to reset
 * @return 0 if OK, -ve on error
 */
int scsi_bus_reset(struct udevice *dev);

/**
 * scsi_scan() - Scan all SCSI controllers for available devices
 *
 * @vebose: true to show information about each device found
 */
int scsi_scan(bool verbose);

/**
 * scsi_scan_dev() - scan a SCSI bus and create devices
 *
 * @dev:	SCSI bus
 * @verbose:	true to show information about each device found
 */
int scsi_scan_dev(struct udevice *dev, bool verbose);

#ifndef CONFIG_DM_SCSI
void scsi_low_level_init(int busdevfunc);
void scsi_init(void);
#endif

#define SCSI_IDENTIFY					0xC0  /* not used */

/* Hardware errors  */
#define SCSI_SEL_TIME_OUT			 0x00000101	 /* Selection time out */
#define SCSI_HNS_TIME_OUT			 0x00000102  /* Handshake */
#define SCSI_MA_TIME_OUT			 0x00000103  /* Phase error */
#define SCSI_UNEXP_DIS				 0x00000104  /* unexpected disconnect */

#define SCSI_INT_STATE				 0x00010000  /* unknown Interrupt number is stored in 16 LSB */

#endif /* _SCSI_H */
