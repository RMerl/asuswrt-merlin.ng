/*
 * Broadcom synopsis spi host controller register definitions.
 *
 * Refer to http://hwnbu-twiki.broadcom.com/bin/view/Mwgroup/SpihostRev0.
 * Copyright(c) 2007 Broadcom Corporation
 *
 * $Id: bcmspih.h 708017 2017-06-29 14:11:45Z $
 */

#ifndef	_BCMSPIH_H
#define	_BCMSPIH_H

#include <typedefs.h>

#define	VENDOR_BROADCOM		0x14e4

#define	BCM_SPIH_DEVICE_ID	0x43f6

#define	BCMSPIH_MAX_FIFO	512
#define	BCMSPIH_MAX_DMA		4096

/* cpp contortions to concatenate w/arg prescan */
#define	_PADLINE(line)	pad ## line
#define	_XSTR(line)	_PADLINE(line)
#define	PAD		_XSTR(__LINE__)

/* host interface registers */
typedef volatile struct {
	uint32 corecontrol;		/* CoreControl, 0x000, rev8 */
	uint32 corestatus;		/* CoreStatus, 0x004, rev8  */
	uint32 PAD[1];
	uint32 biststatus;		/* BistStatus, 0x00c, rev8  */

	uint32 PAD[4];

	/* interrupt */
	uint32 intstatus;		/* IntStatus, 0x020, rev8   */
	uint32 hostintmask;		/* IntHostMask, 0x024, rev8   */
	uint32 intmask;			/* IntSbMask, 0x028, rev8   */
	uint32 sbintstatus;		/* SBIntStatus, 0x02c, rev8   */
	uint32 sbintmask;		/* SBIntMask, 0x030, rev8   */
	uint32 PAD[3];
	uint32 tosbmailbox;		/* ToSBMailbox, 0x040, rev8   */
	uint32 tohostmailbox;		/* ToHostMailbox, 0x044, rev8   */
	uint32 tosbmailboxdata;		/* ToSbMailboxData, 0x048, rev8   */
	uint32 tohostmailboxdata;	/* ToHostMailboxData, 0x04c, rev8   */

	uint32 PAD[44];

	/* interrupt batching control */
	uint32 intrcvlazy;		/* IntRcvLazy, 0x100, rev8 */
	uint32 PAD[63];

	/* DMA engines */
	dma32regp_t dmaregs;		/* DMA Regs, 0x200-0x21c, rev8 */
	dma32diag_t dmafifo;		/* DMA Diagnostic Regs, 0x220-0x22c */
	uint32 PAD[436];

	uint32 ctrlr0;		/* 0x900 SPI Control Register 0 */
	uint32 ctrlr1;		/* 0x904 SPI Control Register 1, NDF:Num Data Frames */
	uint32 ssienr;		/* 0x908 SPI SSI Enable Register */
	uint32 mwcr;		/* 0x90C SPI Microwire control Reg */
	uint32 ser;		/* 0x910 SPI Slave Enable Reg */
	uint32 baudr;		/* 0x914 SPI Baud Rate Selection Reg */
	uint32 txftlr;		/* 0x918 SPI Tx Fifo Thresh level */
	uint32 rxftlr;		/* 0x91C SPI Rx Fifo Thresh level */
	uint32 thflr;		/* 0x920 SPI Tx Fifo Level Reg */
	uint32 rxflr;		/* 0x924 SPI Rx Fifo Level Reg */
	uint32 sr;		/* 0x928 SPI Status Reg */
	uint32 imr;		/* 0x92C SPI Intr Mask Reg */
	uint32 isr;		/* 0x930 SPI Intr Status Reg */
	uint32 risr;		/* 0x934 SPI Raw Intr Status Reg */
	uint32 txoicr;		/* 0x938 SPI Tx Fifo Overflow Clr Reg */
	uint32 rxoicr;		/* 0x93C SPI Rx Fifo Flow Overflow Clr Reg */
	uint32 rxuicr;		/* 0x940 SPI Rx Fifo Underflow Intr Clr */
	uint32 msticr;		/* 0x944 SPI Multi Master Intr Clr */
	uint32 icr;		/* 0x948 SPI Intr Clr Reg */
	uint32 dmacr;		/* 0x94C SPI Dma Ctrl Reg */
	uint32 dmatdlr;		/* 0x950 SPI Dma Tx Data Level */
	uint32 dmardlr;		/* 0x954 SPI Dma Rx Data Level */
	uint32 idr;		/* 0x958 SPI Indentification Reg */
	uint32 ssi_comp_ver;	/* 0x95C SPI CoreKit Id Reg */
	uint32 dr;		/* 0x960-99c SPI Data Fifo Reg */
	uint32 PAD[15];
	uint32 propctrl;	/* 0x9A0 SPI Bit 1 - device_intr_pol, bit 0 - highspeed mode */
	uint32 dmarxcmd0;	/* 0x9A4 SPI cmd[15:0] for rx */
	uint32 dmarxcmd1;	/* 0x9A8 SPI cmd[31:16] for rx */
	uint32 PAD;		/* 0x9AC Padding */
	uint32 combinedintstat;	/* 0x9B0 SPI Bit 7 device_intr, bit6 rsvd, bit 5-0 same as 0x930 */

	uint32 PAD[339];

	/* Sonics SiliconBackplane registers */
	sbconfig_t sbconfig;		/* SbConfig Regs, 0xf00-0xfff, rev8 */

} sdioh_regs_t;

/* devcontrol */
#define CODEC_DEVCTRL_SDIOH	0x4000		/* 1: config codec to SDIOH mode,
						 * 0: normal codec mode
						 */

/* mode */
#define MODE_DONT_WAIT_DMA	0x2000		/* diag only: in receive DMA mode, Cmdatadone
						 * will be generated when data transfer done
						 */
#define MODE_BIG_ENDIAN		0x1000		/* 1: big endian, 0: small endian */
#define MODE_STOP_ALL_CLK	0x0800		/* diag only: 1: stop all SD clock */
#define MODE_PRECMD_CNT_EN	0x0400		/* 1: enable precmd count,
						 * 0: disable precmd count
						 */
#define	MODE_CLK_OUT_EN		0x0200		/* 0/1: en/disable the clock output to sdio bus */
#define	MODE_USE_EXT_CLK	0x0100		/* use external clock or not */
#define	MODE_CLK_DIV_MASK	0x00f0		/* divide host clock by 2*this field */
#define	MODE_OP_MASK		0x000f		/* mode is [3:0] bits */
#define	MODE_OP_SDIO4BIT	2		/* SDIO 4 bit mode */
#define	MODE_OP_SDIO1BIT	1		/* SDIO 1 bit mode */
#define	MODE_OP_SPI		0		/* SPI mode */
#define	MODE_HIGHSPEED_EN	0x10000		/* Enable High-Speed clocking Mode. */

/* delay */
#define DLY_CLK_COUNT_PRE_M	0x0000ffff	/* dynamic clock: pre clock on cycles before tx */
#define DLY_CLK_COUNT_PRE_O	0
#define DLY_TX_START_COUNT_M	0xffff0000	/* DMA mode only: wait cycle before transferring
						 * non-empty fifo
						 */
#define DLY_TX_START_COUNT_O	23

/* test */
#define TEST_BAD_CMD_CRC	0x1		/* force bad CMD crc */
#define TEST_BAD_DAT_CRC	0x2		/* force bad DAT crc */

/* arvm */
#define	AR_MASK_OFT		8		/* CMD53 auto response mask */
#define	AR_VAL			0x00ff		/* CMD53 expected value of auto response,
						 * after mask
						 */

/* cmd dat */
#define CMDAT_INDEX_M		0x0000003f	/* command index */
#define CMDAT_EXP_RSPTYPE_M	0x000001c0	/* expected response type */
#define CMDAT_EXP_RSPTYPE_O	6
#define CMDAT_DAT_EN_M		0x00000200	/* data command flag */
#define CMDAT_DAT_EN_O		9
#define CMDAT_DAT_WR_M		0x00000400	/* 0: read from SD device,
						 * 1: write to SD device
						 */
#define CMDAT_DAT_WR_O		10
#define CMDAT_DMA_MODE_M	0x00000800	/* 0: pio, 1: dma */
#define CMDAT_DMA_MODE_O	11
#define CMDAT_ARC_EN_M		0x00001000	/* auto response check enable/disable */
#define CMDAT_ARC_EN_O		12
#define CMDAT_EXP_BUSY_M	0x00002000	/* R1b only: expect busy after response */
#define CMDAT_EXP_BUSY_O	13
#define CMDAT_NO_RSP_CRC_CHK_M	0x00004000	/* disable response crc checking */
#define CMDAT_NO_RSP_CRC_CHK_O	14
#define CMDAT_NO_RSP_CDX_CHK_M	0x00008000	/* disable response command index checking */
#define CMDAT_NO_RSP_CDX_CHK_O	15
#define CMDAT_DAT_TX_CNT_M	0x1fff0000	/* total number of bytes to transfer */
#define CMDAT_DAT_TX_CNT_O	16
#define CMDAT_DATLEN_PIO	64		/* data length limit for pio mode */
#define CMDAT_DATLEN_DMA_NON53	512		/* data length limit for DMA mode non cmd53 */
#define CMDAT_DATLEN_DMA_53	8096		/* data length limit for DMA mode cmd53 */
#define CMDAT_APPEND_EN_M	0x20000000	/* enable sdioh to append a command */
#define CMDAT_APPEND_EN_O	29
#define CMDAT_ABORT_M		0x40000000	/* abort data */
#define CMDAT_ABORT_O		30
#define CMDAT_BLK_EN_M		0x80000000	/* use block mode */
#define CMDAT_BLK_EN_O		31

/* error and error_mask */
#define ERROR_RSP_CRC		0x0001		/* response crc error */
#define ERROR_RSP_TIME		0x0002		/* response time error */
#define ERROR_RSP_DBIT		0x0004		/* response D bit error */
#define ERROR_RSP_EBIT		0x0008		/* response E bit error */
#define ERROR_DAT_CRC		0x0010		/* data r/w crc error */
#define ERROR_DAT_SBIT		0x0020		/* receive data START bir error */
#define ERROR_DAT_EBIT		0x0040		/* receive data END bit error */
#define ERROR_DAT_RSP_S		0x0080		/* data crc response START bit error */
#define ERROR_DAT_RSP_E		0x0100		/* data crc response END bit error */
#define ERROR_DAT_RSP_UNKNOWN	0x0200		/* data response unknown, not 101 or 010 */
#define ERROR_DAT_RSP_TURNARD	0x0400		/* no 2 turnaround cycle between WRITE and
						 * CRC reponse
						 */
#define ERROR_DAT_READ_TO	0x0800		/* data read timeout */
#define ERROR_SPI_TOKEN_UNK	0x1000		/* SPI token unknown */
#define ERROR_SPI_TOKEN_BAD	0x2000		/* SPI error token received */
#define ERROR_SPI_ET_OUTRANGE	0x4000		/* SPI error token: out of range */
#define ERROR_SPI_ET_ECC	0x8000		/* SPI error token: ECC failed */
#define ERROR_SPI_ET_CC		0x010000	/* SPI error token: cc error */
#define ERROR_SPI_ET_ERR	0x020000	/* SPI error token: error */
#define ERROR_AUTO_RSP_CHK	0x040000	/* auto response check error */
#define ERROR_RSP_BUSY_TO	0x080000	/* busy timeout for RBTO */
#define ERROR_RSP_CMDIDX_BAD	0x100000	/* response command index error */

/* intstatus and intmask */
#define INT_CMD_DAT_DONE	0x0001		/* sticky, sdio command/data xfer done */
#define INT_HOST_BUSY		0x0002		/* host busy */
#define INT_DEV_INT		0x0004		/* sdio card interrupt recieved */
#define INT_ERROR_SUM		0x0008		/* logic OR of Error register masked by ErrorMask */
#define INT_CARD_INS		0x0010		/* card inserted */
#define INT_CARD_GONE		0x0020		/* card removed */
#define INT_CMDBUSY_CUTTHRU	0x0040		/* sdioh is busy writing to cmdl_cutthru register */
#define INT_CMDBUSY_APPEND	0x0080		/* this bit is clear when writing cmdl,
						 * and set when APPEND starts
						 */
#define INT_CARD_PRESENT	0x0100		/* card is present */
#define INT_STD_PCI_DESC	0x0400		/* standard DMA engine definition */
#define INT_STD_PCI_DATA	0x0800		/* standard DMA engine definition */
#define INT_STD_DESC_ERR	0x1000		/* standard DMA engine definition */
#define INT_STD_RCV_DESC_UF	0x2000		/* standard DMA engine definition */
#define INT_STD_RCV_FIFO_OF	0x4000		/* standard DMA engine definition */
#define INT_STD_XMT_FIFO_UF	0x8000		/* standard DMA engine definition */
#define INT_RCV_INT		0x00010000		/* standard DMA engine definition */
#define INT_XMT_INT		0x01000000		/* standard DMA engine definition */

/* debuginfo */
#define DBGI_REMAIN_COUNT	0x00001fff	/* remaining count for data comand,
						 * change on the fly
						 */
#define DBGI_CUR_ADDR		0xCfffE000	/* current address of CDM53 */
#define DBGI_CARD_WASBUSY	0x40000000	/* receive card busy signal on data line */
#define DBGI_R1B_DETECTED	0x80000000	/* R1B detected, overwritten by next cmd's status */

/* fifoctl(rcv/xmt) */
#define FIFO_RCV_BUF_RDY	0x10		/* HW set 1 when data are ready and avaiable in
						 * FIFO, write 1 before read RCVFIFODATA
						 */
#define FIFO_XMT_BYTE_VALID	0x0f		/* which bit is valid in all subsequent writes to
						 * xmtfifodata
						 */
#define FIFO_VALID_BYTE1	0x01		/* byte 0 valid */
#define FIFO_VALID_BYTE2	0x02		/* byte 1 valid */
#define FIFO_VALID_BYTE3	0x04		/* byte 2 valid */
#define FIFO_VALID_BYTE4	0x08		/* byte 3 valid */
#define FIFO_VALID_ALL		0x0f		/* all four bytes are valid */

#define SDIOH_MODE_PIO		0		/* pio mode */
#define SDIOH_MODE_DMA		1		/* dma mode */

#define SDIOH_CMDTYPE_NORMAL	0		/* normal command */
#define SDIOH_CMDTYPE_APPEND	1		/* append command */
#define SDIOH_CMDTYPE_CUTTHRU	2		/* cut through command */

#define SDIOH_DMA_START_EARLY	0
#define SDIOH_DMA_START_LATE	1

#define SDIOH_DMA_TX		1
#define SDIOH_DMA_RX		2

#define SDIOH_BLOCK_SIZE_MIN	4
#define SDIOH_BLOCK_SIZE_MAX	0x200

#define SDIOH_HOST_SUPPORT_OCR	0xfff000	/* supported OCR by host controller */

#define RESP_TYPE_NONE 		0
#define RESP_TYPE_R1  		1
#define RESP_TYPE_R2  		2
#define RESP_TYPE_R3  		3
#define RESP_TYPE_R4  		4
#define RESP_TYPE_R5  		5
#define RESP_TYPE_R6  		6

/* SDCMDAT Register */
#define SDIOH_CMD_INDEX_M	BITFIELD_MASK(6)	/* Bits [5:0] 	- Command number */
#define SDIOH_CMD_INDEX_S	0
#define SDIOH_CMD_RESP_TYPE_M	BITFIELD_MASK(3)	/* Bits [8:6] 	- Response type */
#define SDIOH_CMD_RESP_TYPE_S	6
#define SDIOH_CMD_DATA_EN_M	BITFIELD_MASK(1)	/* Bit 9 	- Using DAT line */
#define SDIOH_CMD_DATA_EN_S	9

#define SDIOH_CMD_DATWR_M	BITFIELD_MASK(1)	/* Bit 10 	- Data Write */
#define SDIOH_CMD_DATWR_S	10
#define SDIOH_CMD_DMAMODE_M	BITFIELD_MASK(1)	/* Bit 11 	- DMA Mode */
#define SDIOH_CMD_DMAMODE_S	11
#define SDIOH_CMD_ARC_EN_M	BITFIELD_MASK(1)	/* Bit 12 	- Auto Response Checking */
#define SDIOH_CMD_ARC_EN_S	12
#define SDIOH_CMD_EXP_BSY_M	BITFIELD_MASK(1)	/* Bit 13 	- Expect Busy (R1b) */
#define SDIOH_CMD_EXP_BSY_S	13

#define SDIOH_CMD_CRC_DIS_M	BITFIELD_MASK(1)	/* Bit 14 	- CRC disable */
#define SDIOH_CMD_CRC_DIS_S	14
#define SDIOH_CMD_INDEX_DIS_M	BITFIELD_MASK(1)	/* Bit 15 	- Disable index checking */
#define SDIOH_CMD_INDEX_DIS_S	15

#define SDIOH_CMD_TR_COUNT_M 	BITFIELD_MASK(13)	/* Bits [28:16] - Transfer Count */
#define SDIOH_CMD_TR_COUNT_S	16

#define SDIOH_CMD_APPEND_EN_M	BITFIELD_MASK(1)	/* Bit 29 	- Append enable */
#define SDIOH_CMD_APPEND_EN_S	29
#define SDIOH_CMD_ABORT_EN_M	BITFIELD_MASK(1)	/* Bit 30 	- Abort enable */
#define SDIOH_CMD_ABORT_EN_S	30
#define SDIOH_CMD_BLKMODE_EN_M	BITFIELD_MASK(1)	/* Bit 31 	- Blockmode enable */
#define SDIOH_CMD_BLKMODE_EN_S	31

/* intstatus and intmask */
#define INT_CMD_DAT_DONE_M	BITFIELD_MASK(1)	/* Bit 0: sticky,
							 * sdio command/data xfer done
							 */
#define INT_CMD_DAT_DONE_S	0
#define INT_HOST_BUSY_M		BITFIELD_MASK(1)	/* Bit 1: host busy */
#define INT_HOST_BUSY_S		1
#define INT_DEV_INT_M		BITFIELD_MASK(1)	/* Bit 2: sdio dev interrupt recieved */
#define INT_DEV_INT_S		2
#define INT_ERROR_SUM_M		BITFIELD_MASK(1)	/* Bit 3: OR of Error reg
							 * masked by ErrorMask
							 */
#define INT_ERROR_SUM_S		3
#define INT_CARD_INS_M		BITFIELD_MASK(1)	/* Bit 4: dev inserted */
#define INT_CARD_INS_S		4
#define INT_CARD_GONE_M		BITFIELD_MASK(1)	/* Bit 5: dev removed */
#define INT_CARD_GONE_S		5
#define INT_CMDBUSY_CUTTHRU_M	BITFIELD_MASK(1)	/* sdioh is busy writing to cmdl_cutthru reg
							 */
#define INT_CMDBUSY_CUTTHRU_S	6
#define INT_CMDBUSY_APPEND_M	BITFIELD_MASK(1)	/* this bit is clear when writing cmdl, */
#define INT_CMDBUSY_APPEND_S	7	/* and set when APPEND starts */

#define INT_RCV_INT_M		BITFIELD_MASK(1)	/* Receive DMA Interrupt */
#define INT_RCV_INT_S		16
#define INT_XMT_INT_M		BITFIELD_MASK(1)	/* Transmit DMA Interrupt */
#define INT_XMT_INT_S		24

/* SDBLOCK Register */
#define SDBLOCK_M		BITFIELD_MASK(10)	/* Bits [9:0] Blocksize */
#define SDBLOCK_S		0

#define SD1_MODE 0x1				/* SD Host Cntrlr Spec */
#define SD4_MODE 0x2				/* SD Host Cntrlr Spec */

#define SPID_CMD_RW_S			31
#define SPID_CMD_RW_M			BITFIELD_MASK(1)
#define SPID_CMD_ACCESS_S		30
#define SPID_CMD_ACCESS_M		BITFIELD_MASK(1)
#define SPID_CMD_FUNCTION_S		28
#define SPID_CMD_FUNCTION_M		BITFIELD_MASK(2)
#define SPID_CMD_ADDR_S			11
#define SPID_CMD_ADDR_M			BITFIELD_MASK(17)
#define SPID_CMD_LEN_S			0
#define SPID_CMD_LEN_M			BITFIELD_MASK(11)

#endif /* _BCMSPIH_H */
