/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2007-2008 Freescale Semiconductor, Inc.
 *		Dave Liu <daveliu@freescale.com>
 */

#ifndef __FSL_SATA_H__
#define __FSL_SATA_H__

#define SATA_HC_MAX_NUM		4 /* Max host controller numbers */
#define SATA_HC_MAX_CMD		16 /* Max command queue depth per host controller */
#define SATA_HC_MAX_PORT	16 /* Max port number per host controller */

/*
* SATA Host Controller Registers
*/
typedef struct fsl_sata_reg {
	/* SATA command registers */
	u32 cqr;		/* Command queue register */
	u8 res1[0x4];
	u32 car;		/* Command active register */
	u8 res2[0x4];
	u32 ccr;		/* Command completed register */
	u8 res3[0x4];
	u32 cer;		/* Command error register */
	u8 res4[0x4];
	u32 der;		/* Device error register */
	u32 chba;		/* Command header base address */
	u32 hstatus;		/* Host status register */
	u32 hcontrol;		/* Host control register */
	u32 cqpmp;		/* Port number queue register */
	u32 sig;		/* Signature register */
	u32 icc;		/* Interrupt coalescing control register */
	u8 res5[0xc4];

	/* SATA supperset registers */
	u32 sstatus;		/* SATA interface status register */
	u32 serror;		/* SATA interface error register */
	u32 scontrol;		/* SATA interface control register */
	u32 snotification;	/* SATA interface notification register */
	u8 res6[0x30];

	/* SATA control status registers */
	u32 transcfg;		/* Transport layer configuration */
	u32 transstatus;	/* Transport layer status */
	u32 linkcfg;		/* Link layer configuration */
	u32 linkcfg1;		/* Link layer configuration1 */
	u32 linkcfg2;		/* Link layer configuration2 */
	u32 linkstatus;		/* Link layer status */
	u32 linkstatus1;	/* Link layer status1 */
	u32 phyctrlcfg;		/* PHY control configuration */
	u8 res7[0x2b0];

	/* SATA system control registers */
	u32 syspr;		/* System priority register - big endian */
	u8 res8[0xbec];
} __attribute__ ((packed)) fsl_sata_reg_t;

/* HStatus register
*/
#define HSTATUS_ONOFF			0x80000000 /* Online/offline status */
#define HSTATUS_FORCE_OFFLINE		0x40000000 /* In process going offline */
#define HSTATUS_BIST_ERR		0x20000000

/* Fatal error */
#define HSTATUS_MASTER_ERR		0x00004000
#define HSTATUS_DATA_UNDERRUN		0x00002000
#define HSTATUS_DATA_OVERRUN		0x00001000
#define HSTATUS_CRC_ERR_TX		0x00000800
#define HSTATUS_CRC_ERR_RX		0x00000400
#define HSTATUS_FIFO_OVERFLOW_TX	0x00000200
#define HSTATUS_FIFO_OVERFLOW_RX	0x00000100
#define HSTATUS_FATAL_ERR_ALL		(HSTATUS_MASTER_ERR | \
					HSTATUS_DATA_UNDERRUN | \
					HSTATUS_DATA_OVERRUN | \
					HSTATUS_CRC_ERR_TX | \
					HSTATUS_CRC_ERR_RX | \
					HSTATUS_FIFO_OVERFLOW_TX | \
					HSTATUS_FIFO_OVERFLOW_RX)
/* Interrupt status */
#define HSTATUS_FATAL_ERR		0x00000020
#define HSTATUS_PHY_RDY			0x00000010
#define HSTATUS_SIGNATURE		0x00000008
#define HSTATUS_SNOTIFY			0x00000004
#define HSTATUS_DEVICE_ERR		0x00000002
#define HSTATUS_CMD_COMPLETE		0x00000001

/* HControl register
*/
#define HCONTROL_ONOFF			0x80000000 /* Online or offline request */
#define HCONTROL_FORCE_OFFLINE		0x40000000 /* Force offline request */
#define HCONTROL_ENTERPRISE_EN		0x10000000 /* Enterprise mode enabled */
#define HCONTROL_HDR_SNOOP		0x00000400 /* Command header snoop */
#define HCONTROL_PMP_ATTACHED		0x00000200 /* Port multiplier attached */

/* Interrupt enable */
#define HCONTROL_FATAL_ERR		0x00000020
#define HCONTROL_PHY_RDY		0x00000010
#define HCONTROL_SIGNATURE		0x00000008
#define HCONTROL_SNOTIFY		0x00000004
#define HCONTROL_DEVICE_ERR		0x00000002
#define HCONTROL_CMD_COMPLETE		0x00000001

#define HCONTROL_INT_EN_ALL		(HCONTROL_FATAL_ERR | \
					HCONTROL_PHY_RDY | \
					HCONTROL_SIGNATURE | \
					HCONTROL_SNOTIFY | \
					HCONTROL_DEVICE_ERR | \
					HCONTROL_CMD_COMPLETE)

/* SStatus register
*/
#define SSTATUS_IPM_MASK		0x00000780
#define SSTATUS_IPM_NOPRESENT		0x00000000
#define SSTATUS_IPM_ACTIVE		0x00000080
#define SSTATUS_IPM_PATIAL		0x00000100
#define SSTATUS_IPM_SLUMBER		0x00000300

#define SSTATUS_SPD_MASK		0x000000f0
#define SSTATUS_SPD_GEN1		0x00000010
#define SSTATUS_SPD_GEN2		0x00000020

#define SSTATUS_DET_MASK		0x0000000f
#define SSTATUS_DET_NODEVICE		0x00000000
#define SSTATUS_DET_DISCONNECT		0x00000001
#define SSTATUS_DET_CONNECT		0x00000003
#define SSTATUS_DET_PHY_OFFLINE		0x00000004

/* SControl register
*/
#define SCONTROL_SPM_MASK		0x0000f000
#define SCONTROL_SPM_GO_PARTIAL		0x00001000
#define SCONTROL_SPM_GO_SLUMBER		0x00002000
#define SCONTROL_SPM_GO_ACTIVE		0x00004000

#define SCONTROL_IPM_MASK		0x00000f00
#define SCONTROL_IPM_NO_RESTRICT	0x00000000
#define SCONTROL_IPM_PARTIAL		0x00000100
#define SCONTROL_IPM_SLUMBER		0x00000200
#define SCONTROL_IPM_PART_SLUM		0x00000300

#define SCONTROL_SPD_MASK		0x000000f0
#define SCONTROL_SPD_NO_RESTRICT	0x00000000
#define SCONTROL_SPD_GEN1		0x00000010
#define SCONTROL_SPD_GEN2		0x00000020

#define SCONTROL_DET_MASK		0x0000000f
#define SCONTROL_DET_HRESET		0x00000001
#define SCONTROL_DET_DISABLE		0x00000004

/* TransCfg register
*/
#define TRANSCFG_DFIS_SIZE_SHIFT	16
#define TRANSCFG_RX_WATER_MARK_MASK	0x0000001f

/* PhyCtrlCfg register
*/
#define PHYCTRLCFG_FPRFTI_MASK		0x00000018
#define PHYCTRLCFG_LOOPBACK_MASK	0x0000000e

/*
* Command Header Entry
*/
typedef struct cmd_hdr_entry {
	__le32 cda;		/* Command Descriptor Address,
				   4 bytes aligned */
	__le32 prde_fis_len;	/* Number of PRD entries and FIS length */
	__le32 ttl;		/* Total transfer length */
	__le32 attribute;	/* the attribute of command */
} __attribute__ ((packed)) cmd_hdr_entry_t;

#define SATA_HC_CMD_HDR_ENTRY_SIZE	sizeof(struct cmd_hdr_entry)

/* cda
*/
#define CMD_HDR_CDA_ALIGN	4

/* prde_fis_len
*/
#define CMD_HDR_PRD_ENTRY_SHIFT	16
#define CMD_HDR_PRD_ENTRY_MASK	0x003f0000
#define CMD_HDR_FIS_LEN_SHIFT	2

/* attribute
*/
#define CMD_HDR_ATTR_RES	0x00000800 /* Reserved bit, should be 1 */
#define CMD_HDR_ATTR_VBIST	0x00000400 /* Vendor BIST */
#define CMD_HDR_ATTR_SNOOP	0x00000200 /* Snoop enable for all descriptor */
#define CMD_HDR_ATTR_FPDMA	0x00000100 /* FPDMA queued command */
#define CMD_HDR_ATTR_RESET	0x00000080 /* Reset - a SRST or device reset */
#define CMD_HDR_ATTR_BIST	0x00000040 /* BIST - require the host to enter BIST mode */
#define CMD_HDR_ATTR_ATAPI	0x00000020 /* ATAPI command */
#define CMD_HDR_ATTR_TAG	0x0000001f /* TAG mask */

/* command type
*/
enum cmd_type {
	CMD_VENDOR_BIST,
	CMD_BIST,
	CMD_RESET,	/* SRST or device reset */
	CMD_ATAPI,
	CMD_NCQ,
	CMD_ATA,	/* None of all above */
};

/*
* Command Header Table
*/
typedef struct cmd_hdr_tbl {
	cmd_hdr_entry_t cmd_slot[SATA_HC_MAX_CMD];
} __attribute__ ((packed)) cmd_hdr_tbl_t;

#define SATA_HC_CMD_HDR_TBL_SIZE	sizeof(struct cmd_hdr_tbl)
#define SATA_HC_CMD_HDR_TBL_ALIGN	4

/*
* PRD entry - Physical Region Descriptor entry
*/
typedef struct prd_entry {
	__le32 dba;	/* Data base address, 4 bytes aligned */
	u32 res1;
	u32 res2;
	__le32 ext_c_ddc; /* Indirect PRD flags, snoop and data word count */
} __attribute__ ((packed)) prd_entry_t;

#define SATA_HC_CMD_DESC_PRD_SIZE	sizeof(struct prd_entry)

/* dba
*/
#define PRD_ENTRY_DBA_ALIGN	4

/* ext_c_ddc
*/
#define PRD_ENTRY_EXT		0x80000000 /* extension flag */
#ifdef CONFIG_FSL_SATA_V2
#define PRD_ENTRY_DATA_SNOOP	0x10000000 /* Data snoop enable */
#else
#define PRD_ENTRY_DATA_SNOOP	0x00400000 /* Data snoop enable */
#endif
#define PRD_ENTRY_LEN_MASK	0x003fffff /* Data word count */

#define PRD_ENTRY_MAX_XFER_SZ	(PRD_ENTRY_LEN_MASK + 1)

/*
 * This SATA host controller supports a max of 16 direct PRD entries, but if use
 * chained indirect PRD entries, then the contollers supports upto a max of 63
 * entries including direct and indirect PRD entries.
 * The PRDT is an array of 63 PRD entries contigiously, but the PRD entries#15
 * will be setup as an indirect descriptor, pointing to it's next (contigious)
 * PRD entries#16.
 */
#define SATA_HC_MAX_PRD		63 /* Max PRD entry numbers per command */
#define SATA_HC_MAX_PRD_DIRECT	16 /* Direct PRDT entries */
#define SATA_HC_MAX_PRD_USABLE	(SATA_HC_MAX_PRD - 1)
#define SATA_HC_MAX_XFER_LEN	0x4000000

/*
* PRDT - Physical Region Descriptor Table
*/
typedef struct prdt {
	prd_entry_t prdt[SATA_HC_MAX_PRD];
} __attribute__ ((packed)) prdt_t;

/*
* Command Descriptor
*/
#define SATA_HC_CMD_DESC_CFIS_SIZE	32 /* bytes */
#define SATA_HC_CMD_DESC_SFIS_SIZE	32 /* bytes */
#define SATA_HC_CMD_DESC_ACMD_SIZE	16 /* bytes */
#define SATA_HC_CMD_DESC_RES		16 /* bytes */

typedef struct cmd_desc {
	u8 cfis[SATA_HC_CMD_DESC_CFIS_SIZE];
	u8 sfis[SATA_HC_CMD_DESC_SFIS_SIZE];
	u8 acmd[SATA_HC_CMD_DESC_ACMD_SIZE];
	u8 res[SATA_HC_CMD_DESC_RES];
	prd_entry_t prdt[SATA_HC_MAX_PRD];
} __attribute__ ((packed)) cmd_desc_t;

#define SATA_HC_CMD_DESC_SIZE		sizeof(struct cmd_desc)
#define SATA_HC_CMD_DESC_ALIGN		4

/*
 * SATA device driver info
 */
typedef struct fsl_sata_info {
	u32	sata_reg_base;
	u32	flags;
} fsl_sata_info_t;

#define FLAGS_DMA	0x00000000
#define FLAGS_FPDMA	0x00000001

/*
 * SATA device driver struct
 */
typedef struct fsl_sata {
	char		name[12];
	fsl_sata_reg_t	*reg_base;		/* the base address of controller register */
	void		*cmd_hdr_tbl_offset;	/* alloc address of command header table */
	cmd_hdr_tbl_t	*cmd_hdr;		/* aligned address of command header table */
	void		*cmd_desc_offset;	/* alloc address of command descriptor */
	cmd_desc_t	*cmd_desc;		/* aligned address of command descriptor */
	int		link;			/* PHY link status */
	/* device attribute */
	int		ata_device_type;	/* device type */
	int		lba48;
	int		queue_depth;		/* Max NCQ queue depth */
	u16		pio;
	u16		mwdma;
	u16		udma;
	int		wcache;
	int		flush;
	int		flush_ext;
	u32		dma_flag;
} fsl_sata_t;

#define READ_CMD	0
#define WRITE_CMD	1

#endif /* __FSL_SATA_H__ */
