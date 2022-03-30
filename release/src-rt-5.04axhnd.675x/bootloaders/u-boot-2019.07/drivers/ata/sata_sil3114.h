/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) Excito Elektronik i Skåne AB, All rights reserved.
 * Author: Tor Krill <tor@excito.com>
 */

#ifndef SATA_SIL3114_H
#define SATA_SIL3114_H

struct sata_ioports {
	unsigned long cmd_addr;
	unsigned long data_addr;
	unsigned long error_addr;
	unsigned long feature_addr;
	unsigned long nsect_addr;
	unsigned long lbal_addr;
	unsigned long lbam_addr;
	unsigned long lbah_addr;
	unsigned long device_addr;
	unsigned long status_addr;
	unsigned long command_addr;
	unsigned long altstatus_addr;
	unsigned long ctl_addr;
	unsigned long bmdma_addr;
	unsigned long scr_addr;
};

struct sata_port {
	unsigned char port_no;	/* primary=0, secondary=1       */
	struct sata_ioports ioaddr;	/* ATA cmd/ctl/dma reg blks     */
	unsigned char ctl_reg;
	unsigned char last_ctl;
	unsigned char port_state;	/* 1-port is available and      */
	/* 0-port is not available      */
	unsigned char dev_mask;
};

/* Missing ata defines */
#define ATA_CMD_STANDBY			0xE2
#define ATA_CMD_STANDBYNOW1		0xE0
#define ATA_CMD_IDLE			0xE3
#define ATA_CMD_IDLEIMMEDIATE	0xE1

/* Defines for SIL3114 chip */

/* PCI defines */
#define SIL_VEND_ID		0x1095
#define SIL3114_DEVICE_ID	0x3114

/* some vendor specific registers */
#define	VND_SYSCONFSTAT	0x88	/* System Configuration Status and Command */
#define VND_SYSCONFSTAT_CHN_0_INTBLOCK (1<<22)
#define VND_SYSCONFSTAT_CHN_1_INTBLOCK (1<<23)
#define VND_SYSCONFSTAT_CHN_2_INTBLOCK (1<<24)
#define VND_SYSCONFSTAT_CHN_3_INTBLOCK (1<<25)

/* internal registers mapped by BAR5 */
/* SATA Control*/
#define VND_SCONTROL_CH0	0x100
#define VND_SCONTROL_CH1	0x180
#define VND_SCONTROL_CH2	0x300
#define VND_SCONTROL_CH3	0x380

#define SATA_SC_IPM_T2P		(1<<16)
#define SATA_SC_IPM_T2S		(2<<16)
#define SATA_SC_SPD_1_5		(1<<4)
#define SATA_SC_SPD_3_0		(2<<4)
#define SATA_SC_DET_RST		(1)	/* ATA Reset sequence */
#define SATA_SC_DET_PDIS	(4)	/* PHY Disable */

/* SATA Status */
#define VND_SSTATUS_CH0		0x104
#define VND_SSTATUS_CH1		0x184
#define VND_SSTATUS_CH2		0x304
#define VND_SSTATUS_CH3		0x384

#define SATA_SS_IPM_ACTIVE	(1<<8)
#define SATA_SS_IPM_PARTIAL	(2<<8)
#define SATA_SS_IPM_SLUMBER	(6<<8)
#define SATA_SS_SPD_1_5		(1<<4)
#define SATA_SS_SPD_3_0		(2<<4)
#define SATA_DET_P_NOPHY	(1)	/* Device presence but no PHY connection established */
#define SATA_DET_PRES		(3)	/* Device presence and active PHY */
#define SATA_DET_OFFLINE	(4)	/* Device offline or in loopback mode */

/* Task file registers in BAR5 mapping */
#define VND_TF0_CH0			0x80
#define VND_TF0_CH1			0xc0
#define VND_TF0_CH2			0x280
#define VND_TF0_CH3			0x2c0
#define VND_TF1_CH0			0x88
#define VND_TF1_CH1			0xc8
#define VND_TF1_CH2			0x288
#define VND_TF1_CH3			0x2c8
#define VND_TF2_CH0			0x88
#define VND_TF2_CH1			0xc8
#define VND_TF2_CH2			0x288
#define VND_TF2_CH3			0x2c8

#define VND_BMDMA_CH0		0x00
#define VND_BMDMA_CH1		0x08
#define VND_BMDMA_CH2		0x200
#define VND_BMDMA_CH3		0x208
#define VND_BMDMA2_CH0		0x10
#define VND_BMDMA2_CH1		0x18
#define VND_BMDMA2_CH2		0x210
#define VND_BMDMA2_CH3		0x218

/* FIFO control */
#define	VND_FIFOCFG_CH0		0x40
#define	VND_FIFOCFG_CH1		0x44
#define	VND_FIFOCFG_CH2		0x240
#define	VND_FIFOCFG_CH3		0x244

/* Task File configuration and status */
#define VND_TF_CNST_CH0		0xa0
#define VND_TF_CNST_CH1		0xe0
#define VND_TF_CNST_CH2		0x2a0
#define VND_TF_CNST_CH3		0x2e0

#define VND_TF_CNST_BFCMD	(1<<1)
#define VND_TF_CNST_CHNRST	(1<<2)
#define VND_TF_CNST_VDMA	(1<<10)
#define VND_TF_CNST_INTST	(1<<11)
#define VND_TF_CNST_WDTO	(1<<12)
#define VND_TF_CNST_WDEN	(1<<13)
#define VND_TF_CNST_WDIEN	(1<<14)

/* for testing */
#define VND_SSDR			0x04c	/* System Software Data Register */
#define VND_FMACS			0x050	/* Flash Memory Address control and status */

#endif
