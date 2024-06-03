/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2011 Freescale Semiconductor, Inc.
 * Author: Tang Yuantian <b29983@freescale.com>
 */

#ifndef SATA_SIL3132_H
#define SATA_SIL3132_H

#define READ_CMD	0
#define WRITE_CMD	1

/*
 * SATA device driver struct for each dev
 */
struct sil_sata {
	char	name[12];
	void	*port;	/* the port base address */
	int		lba48;
	u16		pio;
	u16		mwdma;
	u16		udma;
	pci_dev_t devno;
	int		wcache;
	int		flush;
	int		flush_ext;
};

/* sata info for each controller */
struct sata_info {
	ulong iobase[3];
	pci_dev_t devno;
	int portbase;
	int maxport;
};

/*
 * Scatter gather entry (SGE),MUST 8 bytes aligned
 */
struct sil_sge {
	__le64 addr;
	__le32 cnt;
	__le32 flags;
} __attribute__ ((aligned(8), packed));

/*
 * Port request block, MUST 8 bytes aligned
 */
struct sil_prb {
	__le16 ctrl;
	__le16 prot;
	__le32 rx_cnt;
	struct sata_fis_h2d fis;
} __attribute__ ((aligned(8), packed));

struct sil_cmd_block {
	struct sil_prb prb;
	struct sil_sge sge;
};

enum {
	HOST_SLOT_STAT		= 0x00, /* 32 bit slot stat * 4 */
	HOST_CTRL		= 0x40,
	HOST_IRQ_STAT		= 0x44,
	HOST_PHY_CFG		= 0x48,
	HOST_BIST_CTRL		= 0x50,
	HOST_BIST_PTRN		= 0x54,
	HOST_BIST_STAT		= 0x58,
	HOST_MEM_BIST_STAT	= 0x5c,
	HOST_FLASH_CMD		= 0x70,
		/* 8 bit regs */
	HOST_FLASH_DATA		= 0x74,
	HOST_TRANSITION_DETECT	= 0x75,
	HOST_GPIO_CTRL		= 0x76,
	HOST_I2C_ADDR		= 0x78, /* 32 bit */
	HOST_I2C_DATA		= 0x7c,
	HOST_I2C_XFER_CNT	= 0x7e,
	HOST_I2C_CTRL		= 0x7f,

	/* HOST_SLOT_STAT bits */
	HOST_SSTAT_ATTN		= (1 << 31),

	/* HOST_CTRL bits */
	HOST_CTRL_M66EN		= (1 << 16), /* M66EN PCI bus signal */
	HOST_CTRL_TRDY		= (1 << 17), /* latched PCI TRDY */
	HOST_CTRL_STOP		= (1 << 18), /* latched PCI STOP */
	HOST_CTRL_DEVSEL	= (1 << 19), /* latched PCI DEVSEL */
	HOST_CTRL_REQ64		= (1 << 20), /* latched PCI REQ64 */
	HOST_CTRL_GLOBAL_RST	= (1 << 31), /* global reset */

	/*
	 * Port registers
	 * (8192 bytes @ +0x0000, +0x2000, +0x4000 and +0x6000 @ BAR2)
	 */
	PORT_REGS_SIZE		= 0x2000,

	PORT_LRAM		= 0x0000, /* 31 LRAM slots and PMP regs */
	PORT_LRAM_SLOT_SZ	= 0x0080, /* 32 bytes PRB + 2 SGE, ACT... */

	PORT_PMP		= 0x0f80, /* 8 bytes PMP * 16 (128 bytes) */
	PORT_PMP_STATUS		= 0x0000, /* port device status offset */
	PORT_PMP_QACTIVE	= 0x0004, /* port device QActive offset */
	PORT_PMP_SIZE		= 0x0008, /* 8 bytes per PMP */

	/* 32 bit regs */
	PORT_CTRL_STAT		= 0x1000, /* write: ctrl-set, read: stat */
	PORT_CTRL_CLR		= 0x1004, /* write: ctrl-clear */
	PORT_IRQ_STAT		= 0x1008, /* high: status, low: interrupt */
	PORT_IRQ_ENABLE_SET	= 0x1010, /* write: enable-set */
	PORT_IRQ_ENABLE_CLR	= 0x1014, /* write: enable-clear */
	PORT_ACTIVATE_UPPER_ADDR = 0x101c,
	PORT_EXEC_FIFO		= 0x1020, /* command execution fifo */
	PORT_CMD_ERR		= 0x1024, /* command error number */
	PORT_FIS_CFG		= 0x1028,
	PORT_FIFO_THRES		= 0x102c,

	/* 16 bit regs */
	PORT_DECODE_ERR_CNT	= 0x1040,
	PORT_DECODE_ERR_THRESH	= 0x1042,
	PORT_CRC_ERR_CNT	= 0x1044,
	PORT_CRC_ERR_THRESH	= 0x1046,
	PORT_HSHK_ERR_CNT	= 0x1048,
	PORT_HSHK_ERR_THRESH	= 0x104a,

	/* 32 bit regs */
	PORT_PHY_CFG		= 0x1050,
	PORT_SLOT_STAT		= 0x1800,
	PORT_CMD_ACTIVATE	= 0x1c00, /* 64 bit cmd activate * 31 */
	PORT_CONTEXT		= 0x1e04,
	PORT_EXEC_DIAG		= 0x1e00, /* 32bit exec diag * 16 */
	PORT_PSD_DIAG		= 0x1e40, /* 32bit psd diag * 16 */
	PORT_SCONTROL		= 0x1f00,
	PORT_SSTATUS		= 0x1f04,
	PORT_SERROR		= 0x1f08,
	PORT_SACTIVE		= 0x1f0c,

	/* PORT_CTRL_STAT bits */
	PORT_CS_PORT_RST	= (1 << 0), /* port reset */
	PORT_CS_DEV_RST		= (1 << 1), /* device reset */
	PORT_CS_INIT		= (1 << 2), /* port initialize */
	PORT_CS_IRQ_WOC		= (1 << 3), /* interrupt write one to clear */
	PORT_CS_CDB16		= (1 << 5), /* 0=12b cdb, 1=16b cdb */
	PORT_CS_PMP_RESUME	= (1 << 6), /* PMP resume */
	PORT_CS_32BIT_ACTV	= (1 << 10), /* 32-bit activation */
	PORT_CS_PMP_EN		= (1 << 13), /* port multiplier enable */
	PORT_CS_RDY		= (1 << 31), /* port ready to accept commands */

	/* PORT_IRQ_STAT/ENABLE_SET/CLR */
	/* bits[11:0] are masked */
	PORT_IRQ_COMPLETE	= (1 << 0), /* command(s) completed */
	PORT_IRQ_ERROR		= (1 << 1), /* command execution error */
	PORT_IRQ_PORTRDY_CHG	= (1 << 2), /* port ready change */
	PORT_IRQ_PWR_CHG	= (1 << 3), /* power management change */
	PORT_IRQ_PHYRDY_CHG	= (1 << 4), /* PHY ready change */
	PORT_IRQ_COMWAKE	= (1 << 5), /* COMWAKE received */
	PORT_IRQ_UNK_FIS	= (1 << 6), /* unknown FIS received */
	PORT_IRQ_DEV_XCHG	= (1 << 7), /* device exchanged */
	PORT_IRQ_8B10B		= (1 << 8), /* 8b/10b decode error threshold */
	PORT_IRQ_CRC		= (1 << 9), /* CRC error threshold */
	PORT_IRQ_HANDSHAKE	= (1 << 10), /* handshake error threshold */
	PORT_IRQ_SDB_NOTIFY	= (1 << 11), /* SDB notify received */

	DEF_PORT_IRQ		= PORT_IRQ_COMPLETE | PORT_IRQ_ERROR |
				  PORT_IRQ_PHYRDY_CHG | PORT_IRQ_DEV_XCHG |
				  PORT_IRQ_UNK_FIS | PORT_IRQ_SDB_NOTIFY,

	/* bits[27:16] are unmasked (raw) */
	PORT_IRQ_RAW_SHIFT	= 16,
	PORT_IRQ_MASKED_MASK	= 0x7ff,
	PORT_IRQ_RAW_MASK	= (0x7ff << PORT_IRQ_RAW_SHIFT),

	/* ENABLE_SET/CLR specific, intr steering - 2 bit field */
	PORT_IRQ_STEER_SHIFT	= 30,
	PORT_IRQ_STEER_MASK	= (3 << PORT_IRQ_STEER_SHIFT),

	/* PORT_CMD_ERR constants */
	PORT_CERR_DEV		= 1, /* Error bit in D2H Register FIS */
	PORT_CERR_SDB		= 2, /* Error bit in SDB FIS */
	PORT_CERR_DATA		= 3, /* Error in data FIS not detected by dev */
	PORT_CERR_SEND		= 4, /* Initial cmd FIS transmission failure */
	PORT_CERR_INCONSISTENT	= 5, /* Protocol mismatch */
	PORT_CERR_DIRECTION	= 6, /* Data direction mismatch */
	PORT_CERR_UNDERRUN	= 7, /* Ran out of SGEs while writing */
	PORT_CERR_OVERRUN	= 8, /* Ran out of SGEs while reading */

	/* bits of PRB control field */
	PRB_CTRL_PROTOCOL	= (1 << 0), /* override def. ATA protocol */
	PRB_CTRL_PACKET_READ	= (1 << 4), /* PACKET cmd read */
	PRB_CTRL_PACKET_WRITE	= (1 << 5), /* PACKET cmd write */
	PRB_CTRL_NIEN		= (1 << 6), /* Mask completion irq */
	PRB_CTRL_SRST		= (1 << 7), /* Soft reset request (ign BSY?) */

	/* PRB protocol field */
	PRB_PROT_PACKET		= (1 << 0),
	PRB_PROT_TCQ		= (1 << 1),
	PRB_PROT_NCQ		= (1 << 2),
	PRB_PROT_READ		= (1 << 3),
	PRB_PROT_WRITE		= (1 << 4),
	PRB_PROT_TRANSPARENT	= (1 << 5),

	/*
	 * Other constants
	 */
	SGE_TRM			= (1 << 31), /* Last SGE in chain */
	SGE_LNK			= (1 << 30), /* linked list
						Points to SGT, not SGE */
	SGE_DRD			= (1 << 29), /* discard data read (/dev/null)
						data address ignored */

	CMD_ERR		= 0x21,
};

#endif
