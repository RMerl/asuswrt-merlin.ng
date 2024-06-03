/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2005-2006 Atmel Corporation
 */
#ifndef __ATMEL_MCI_H__
#define __ATMEL_MCI_H__

int atmel_mci_init(void *regs);

#ifndef __ASSEMBLY__

/*
 * Structure for struct SoC access.
 * Names starting with '_' are fillers.
 */
typedef struct atmel_mci {
	/*	reg	Offset */
	u32	cr;	/* 0x00 */
	u32	mr;	/* 0x04 */
	u32	dtor;	/* 0x08 */
	u32	sdcr;	/* 0x0c */
	u32	argr;	/* 0x10 */
	u32	cmdr;	/* 0x14 */
	u32	blkr;	/* 0x18 */
	u32	_1c;	/* 0x1c */
	u32	rspr;	/* 0x20 */
	u32	rspr1;	/* 0x24 */
	u32	rspr2;	/* 0x28 */
	u32	rspr3;	/* 0x2c */
	u32	rdr;	/* 0x30 */
	u32	tdr;	/* 0x34 */
	u32	_38;	/* 0x38 */
	u32	_3c;	/* 0x3c */
	u32	sr;	/* 0x40 */
	u32	ier;	/* 0x44 */
	u32	idr;	/* 0x48 */
	u32	imr;	/* 0x4c */
	u32	dma;	/* 0x50 */
	u32	cfg;	/* 0x54 */
	u32	reserved[41];
	u32	version;
} atmel_mci_t;

#endif /* __ASSEMBLY__ */

/* Bitfields in CR */
#define MMCI_MCIEN_OFFSET			0
#define MMCI_MCIEN_SIZE				1
#define MMCI_MCIDIS_OFFSET			1
#define MMCI_MCIDIS_SIZE			1
#define MMCI_PWSEN_OFFSET			2
#define MMCI_PWSEN_SIZE				1
#define MMCI_PWSDIS_OFFSET			3
#define MMCI_PWSDIS_SIZE			1
#define MMCI_SWRST_OFFSET			7
#define MMCI_SWRST_SIZE				1

/* Bitfields in MR */
#define MMCI_CLKDIV_OFFSET			0
#define MMCI_CLKDIV_SIZE			8
#define MMCI_PWSDIV_OFFSET			8
#define MMCI_PWSDIV_SIZE			3
#define MMCI_RDPROOF_OFFSET			11
#define MMCI_RDPROOF_SIZE			1
#define MMCI_WRPROOF_OFFSET			12
#define MMCI_WRPROOF_SIZE			1
#define MMCI_PDCPADV_OFFSET			14
#define MMCI_PDCPADV_SIZE			1
#define MMCI_PDCMODE_OFFSET			15
#define MMCI_PDCMODE_SIZE			1
/* MCI IP version >= 0x500, MR bit 16 used for CLKODD */
#define MMCI_CLKODD_OFFSET			16
#define MMCI_CLKODD_SIZE			1
/* MCI IP version < 0x200, MR higher 16bits for BLKLEN */
#define MMCI_BLKLEN_OFFSET			16
#define MMCI_BLKLEN_SIZE			16

/* Bitfields in DTOR */
#define MMCI_DTOCYC_OFFSET			0
#define MMCI_DTOCYC_SIZE			4
#define MMCI_DTOMUL_OFFSET			4
#define MMCI_DTOMUL_SIZE			3

/* Bitfields in SDCR */
#define MMCI_SCDSEL_OFFSET			0
#define MMCI_SCDSEL_SIZE			4
#define MMCI_SCDBUS_OFFSET			7
#define MMCI_SCDBUS_SIZE			1

/* Bitfields in ARGR */
#define MMCI_ARG_OFFSET				0
#define MMCI_ARG_SIZE				32

/* Bitfields in CMDR */
#define MMCI_CMDNB_OFFSET			0
#define MMCI_CMDNB_SIZE				6
#define MMCI_RSPTYP_OFFSET			6
#define MMCI_RSPTYP_SIZE			2
#define MMCI_SPCMD_OFFSET			8
#define MMCI_SPCMD_SIZE				3
#define MMCI_OPDCMD_OFFSET			11
#define MMCI_OPDCMD_SIZE			1
#define MMCI_MAXLAT_OFFSET			12
#define MMCI_MAXLAT_SIZE			1
#define MMCI_TRCMD_OFFSET			16
#define MMCI_TRCMD_SIZE				2
#define MMCI_TRDIR_OFFSET			18
#define MMCI_TRDIR_SIZE				1
#define MMCI_TRTYP_OFFSET			19
#define MMCI_TRTYP_SIZE				2

/* Bitfields in BLKR */
/* MMCI_BLKLEN_OFFSET/SIZE already defined in MR */
#define MMCI_BCNT_OFFSET			0
#define MMCI_BCNT_SIZE			16

/* Bitfields in RSPRx */
#define MMCI_RSP_OFFSET				0
#define MMCI_RSP_SIZE				32

/* Bitfields in SR/IER/IDR/IMR */
#define MMCI_CMDRDY_OFFSET			0
#define MMCI_CMDRDY_SIZE			1
#define MMCI_RXRDY_OFFSET			1
#define MMCI_RXRDY_SIZE				1
#define MMCI_TXRDY_OFFSET			2
#define MMCI_TXRDY_SIZE				1
#define MMCI_BLKE_OFFSET			3
#define MMCI_BLKE_SIZE				1
#define MMCI_DTIP_OFFSET			4
#define MMCI_DTIP_SIZE				1
#define MMCI_NOTBUSY_OFFSET			5
#define MMCI_NOTBUSY_SIZE			1
#define MMCI_ENDRX_OFFSET			6
#define MMCI_ENDRX_SIZE				1
#define MMCI_ENDTX_OFFSET			7
#define MMCI_ENDTX_SIZE				1
#define MMCI_RXBUFF_OFFSET			14
#define MMCI_RXBUFF_SIZE			1
#define MMCI_TXBUFE_OFFSET			15
#define MMCI_TXBUFE_SIZE			1
#define MMCI_RINDE_OFFSET			16
#define MMCI_RINDE_SIZE				1
#define MMCI_RDIRE_OFFSET			17
#define MMCI_RDIRE_SIZE				1
#define MMCI_RCRCE_OFFSET			18
#define MMCI_RCRCE_SIZE				1
#define MMCI_RENDE_OFFSET			19
#define MMCI_RENDE_SIZE				1
#define MMCI_RTOE_OFFSET			20
#define MMCI_RTOE_SIZE				1
#define MMCI_DCRCE_OFFSET			21
#define MMCI_DCRCE_SIZE				1
#define MMCI_DTOE_OFFSET			22
#define MMCI_DTOE_SIZE				1
#define MMCI_OVRE_OFFSET			30
#define MMCI_OVRE_SIZE				1
#define MMCI_UNRE_OFFSET			31
#define MMCI_UNRE_SIZE				1

/* Constants for DTOMUL */
#define MMCI_DTOMUL_1_CYCLE			0
#define MMCI_DTOMUL_16_CYCLES			1
#define MMCI_DTOMUL_128_CYCLES			2
#define MMCI_DTOMUL_256_CYCLES			3
#define MMCI_DTOMUL_1024_CYCLES			4
#define MMCI_DTOMUL_4096_CYCLES			5
#define MMCI_DTOMUL_65536_CYCLES		6
#define MMCI_DTOMUL_1048576_CYCLES		7

/* Constants for RSPTYP */
#define MMCI_RSPTYP_NO_RESP			0
#define MMCI_RSPTYP_48_BIT_RESP			1
#define MMCI_RSPTYP_136_BIT_RESP		2

/* Constants for SPCMD */
#define MMCI_SPCMD_NO_SPEC_CMD			0
#define MMCI_SPCMD_INIT_CMD			1
#define MMCI_SPCMD_SYNC_CMD			2
#define MMCI_SPCMD_INT_CMD			4
#define MMCI_SPCMD_INT_RESP			5

/* Constants for TRCMD */
#define MMCI_TRCMD_NO_TRANS			0
#define MMCI_TRCMD_START_TRANS			1
#define MMCI_TRCMD_STOP_TRANS			2

/* Constants for TRTYP */
#define MMCI_TRTYP_BLOCK			0
#define MMCI_TRTYP_MULTI_BLOCK			1
#define MMCI_TRTYP_STREAM			2

/* Bitfields in CFG */
#define MMCI_FIFOMODE_OFFSET			0
#define MMCI_FIFOMODE_SIZE			1
#define MMCI_FERRCTRL_OFFSET			4
#define MMCI_FERRCTRL_SIZE			1
#define MMCI_HSMODE_OFFSET			8
#define MMCI_HSMODE_SIZE			1
#define MMCI_LSYNC_OFFSET			12
#define MMCI_LSYNC_SIZE				1

/* Bit manipulation macros */
#define MMCI_BIT(name)					\
	(1 << MMCI_##name##_OFFSET)
#define MMCI_BF(name,value)				\
	(((value) & ((1 << MMCI_##name##_SIZE) - 1))	\
	 << MMCI_##name##_OFFSET)
#define MMCI_BFEXT(name,value)				\
	(((value) >> MMCI_##name##_OFFSET)\
	 & ((1 << MMCI_##name##_SIZE) - 1))
#define MMCI_BFINS(name,value,old)			\
	(((old) & ~(((1 << MMCI_##name##_SIZE) - 1)	\
		    << MMCI_##name##_OFFSET))		\
	 | MMCI_BF(name,value))

#endif /* __ATMEL_MCI_H__ */
