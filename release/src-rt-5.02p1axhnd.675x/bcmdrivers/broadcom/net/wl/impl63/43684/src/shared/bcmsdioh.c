/*
 *  SDIO HOST CONTROLLER driver
 *
 *  Functions are clustered to a few layers:
 *         external sdioh APIs(sdioh_api_xx)
 *         sdioh host/core control(sdioh_host_xx, sdioh_core_xx)
 *         sdio protocol(sdio_)
 *         utilities
 * Copyright 2020 Broadcom
 *
 * This program is the proprietary software of Broadcom and/or
 * its licensors, and may only be used, duplicated, modified or distributed
 * pursuant to the terms and conditions of a separate, written license
 * agreement executed between you and Broadcom (an "Authorized License").
 * Except as set forth in an Authorized License, Broadcom grants no license
 * (express or implied), right to use, or waiver of any kind with respect to
 * the Software, and Broadcom expressly reserves all rights in and to the
 * Software and all intellectual property rights therein.  IF YOU HAVE NO
 * AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY
 * WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF
 * THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use
 * all reasonable efforts to protect the confidentiality thereof, and to
 * use this information only in connection with your use of Broadcom
 * integrated circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
 * REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR
 * OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
 * DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
 * NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
 * ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
 * OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
 * BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL,
 * SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR
 * IN ANY WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
 * IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii)
 * ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF
 * OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY
 * NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 * $Id: bcmsdioh.c 654158 2016-08-11 09:30:01Z $
 */

#include <typedefs.h>

#include <pcicfg.h>

#include <bcmdevs.h>
#include <bcmendian.h>
#include <bcmutils.h>
#include <hndsoc.h>

#include <osl.h>

#include <siutils.h>
#include <sbhnddma.h>
#include <hnddma.h>

#include <sdio.h>	/* standard SDIO */
#include <sbsdio.h>	/* BRCM sdio device core */
#include <sbsdioh.h>	/* BRCM sdio host core */

#include <bcmsdbus.h>	/* Controller driver APIs */
#include <sdiovar.h>	/* ioctl/iovars */

/* Private Constants and Types */

/* global msglevel for debug messages */
uint sdioh_msglevel = SDH_ERROR_VAL;

/* Use external (undivided) clock (25MHz) */
uint sdioh_extclk = 0;

/* Divide clock an extra time (6.25Mhz) */
uint sdioh_slowclk = 0;

/* Usecs to wait after issuing RES cmd */
uint sdioh_resetwait = 0;

/* message macros - bitvals defined in sdiovar.h */

#define SDIOH_ERROR(x)	do { if (sdioh_msglevel & SDH_ERROR_VAL) printf x; } while (0)
#define SDIOH_TRACE(x)	do { if (sdioh_msglevel & SDH_TRACE_VAL) printf x; } while (0)
#define SDIOH_INFO(x)	do { if (sdioh_msglevel & SDH_INFO_VAL)  printf x; } while (0)
#define SDIOH_DEBUG(x)	do { if (sdioh_msglevel & SDH_DEBUG_VAL) printf x; } while (0)
#define SDIOH_DATA(x)	do { if (sdioh_msglevel & SDH_DATA_VAL)  printf x; } while (0)
#define SDIOH_CTRL(x)	do { if (sdioh_msglevel & SDH_CTRL_VAL)  printf x; } while (0)

#define SDIOH_ASSERT(exp) do { if (!(exp)) \
			     printf("!!!ASSERT fail: file %s lines %d", __FILE__, __LINE__); \
			  } while (0)

/* internal return code */
#define SDIOH_SUCCESS					0 /* Success */
#define SDIOH_FAIL					1 /* Fail */

#define SDIOH_CMDPASS					0 /* Command Passed */
#define SDIOH_CMDERR_INVALID_RESPONSE			2 /* Error: Invalid Response */
#define SDIOH_CMDERR_WRITE_TIMEOUT			3 /* Error: Write Timeout */
#define SDIOH_CMDERR_READ_TIMEOUT			4 /* Error: Read Timeout */
#define SDIOH_CMDERR_NO_IO_FUNC				5 /* Error: No I/O Func */
#define SDIOH_CMDERR_INVALID_PARAMETER			6 /* Error: Invalid Parameter */
#define SDIOH_CMDERR_R1_ILLEGAL_COMMAND			7 /* Error: Illegal Command */
#define SDIOH_CMDERR_R1_COM_CRC_ERROR			8 /* Error: CRC errors */
#define SDIOH_CMDERR_R1_FUNC_NUM_ERROR			9 /* Error: Function Num error */
#define SDIOH_CMDERR_R1_ADDRESS_ERROR			10 /* Error: Address error */
#define SDIOH_CMDERR_R1_PARAMETER_ERROR			11 /* Error: Parameter Errror */
#define SDIOH_CMDERR_DATA_ERROR_TOKEN			12 /* Error: Data error token */
#define SDIOH_CMDERR_DATA_NOT_ACCEPTED			13 /* Error: Not accepted */

#define SDIOH_API_RC_PENDING                          (0x01)
#define SDIOH_API_RC_BUFFER_OVERFLOW                  (0x01)
#define SDIOH_API_RC_DEVICE_BUSY                      (0x02)
#define SDIOH_API_RC_UNSUCCESSFUL                     (0x03)
#define SDIOH_API_RC_NOT_IMPLEMENTED                  (0x04)
#define SDIOH_API_RC_ACCESS_VIOLATION                 (0x05)
#define SDIOH_API_RC_INVALID_HANDLE                   (0x06)
#define SDIOH_API_RC_INVALID_PARAMETER                (0x07)
#define SDIOH_API_RC_NO_SUCH_DEVICE                   (0x08)
#define SDIOH_API_RC_INVALID_DEVICE_REQUEST           (0x09)
#define SDIOH_API_RC_NO_MEMORY                        (0x0A)
#define SDIOH_API_RC_BUS_DRIVER_NOT_READY             (0x0B)
#define SDIOH_API_RC_DATA_ERROR                       (0x0C)
#define SDIOH_API_RC_CRC_ERROR                        (0x0D)
#define SDIOH_API_RC_INSUFFICIENT_RESOURCES           (0x0E)
#define SDIOH_API_RC_DEVICE_NOT_CONNECTED             (0x10)
#define SDIOH_API_RC_DEVICE_REMOVED                   (0x11)
#define SDIOH_API_RC_DEVICE_NOT_RESPONDING            (0x12)
#define SDIOH_API_RC_CANCELED                         (0x13)
#define SDIOH_API_RC_RESPONSE_TIMEOUT                 (0x14)
#define SDIOH_API_RC_DATA_TIMEOUT                     (0x15)
#define SDIOH_API_RC_DEVICE_RESPONSE_ERROR            (0x16)
#define SDIOH_API_RC_DEVICE_UNSUPPORTED               (0x17)

#define BLOCK_TRANSFER_LENGTH		(1)	/* BRCM sdsdio core doesn't support block mode */
#define BYTEMODE_MAX_LENGTH		(52)

#define SDIO_MAX_WRITE_WAIT			100	/* Max write wait */
#define SDIO_MAX_READ_WAIT			10000	/* Max read wait time */
#define SDIO_CMD5_RETRY_LIMIT			100	/* Retry limit */

#define SDIOH_TOTCNT_NONE       0

struct sdioh_info
{
	/* handler */
	osl_t 		*osh;			/* osh handler */
	uint32		*pci_mem_regsva;	/* pci device memory va */
	bool		interrupt_enabled;	/* interrupt connnected flag */
	uint		irq;			/* irq number passed to us */

	bool		intr_handler_valid;	/* client driver interrupt handler valid */
	sdioh_cb_fn_t	intr_handler;		/* registered interrupt handler */
	void		*intr_handler_arg;	/* argument to call interrupt handler */

	bool		devrem_handler_valid;	/* client driver device remove handler valid */
	sdioh_cb_fn_t	devrem_handler;		/* registered device remove handler */
	void		*devrem_handler_arg;	/* argument to call device remove handler */

	/* register access */
	uint		PCI_DMA_BASE;
	sdioh_regs_t	sdioh_regs;		/* sdio host registers */
	sdio_regs_t	sdio_regs;		/* sdio registers */

	/* state */
	bool		initialized;		/* card initialized */
	uint8		num_io_func;		/* total I/O function number */
	bool		mem_present;		/* memory present */
	bool		io_ready;		/* io ready */
	uint		ioocr;			/* io ocr value */
	bool		notready;		/* card is not ready */

	uint16		rca;    		/* relative card address */

	uint		sdio_mode;
	uint		host_support_ocr;

	uint		fn_cis[SDIOD_MAX_IOFUNCS+1];

	uint		target_dev;		/* Target device ID */

	/* init configurations */
	bool		initmemory;
	uint		initcmd5_retry_limit;

	uint		sderrormask;		/* enabled error bits */
	uint		sdintmask;		/* enabled interrupt bits */

	bool		card_blk_cap;		/* device's block mode capability */
	uint16		cmd53_blk_size;		/* current host cmd53 block size */
	uint16		fn_blk_size[SDIOD_MAX_IOFUNCS+1]; /* per-fn block size */

	bool		dma_nochop;
	uint		dma_start_mode;
	bool		dontwait_dmadone;

	bool		precmd_count_en;
	uint		sddelay_precount;
	uint		sddelay_txstartcount;
	uint		rdto;
	uint		rbto;

	uint		arvm_sdx_mask;
	uint		arvm_sdx_value;
	uint		arvm_spi_mask;
	uint		arvm_spi_value;

	/* TBD, use hnddma */
	uint8 *dma_xmt_buf;	/* xmt dma buffer pointer */
	uint8 *dma_xmt_desp;	/* xmt dma descriptor pointer */
	uint8 *dma_rcv_buf;	/* rcv dma buffer pointer */
	uint8 *dma_rcv_desp;	/* rcv dma descriptor pointer */

	uint8		*data_buf_tx;	/* storage for sample tx data, for testing */
	uint8		*data_buf_rx;	/* storage for sample rx data, for testing */
};

uint16 defblksz[SDIOD_MAX_IOFUNCS+1] = { 64, 64, 64, 0 };

/* TODO */
#define SDIO_CTRL_PRINT(buf, len) {\
	int i; \
	len += sprintf(buf + len, "sdio control structure:\n"); \
	len += sprintf(buf + len, "  initialized:          %d\n", sdioh->initialized); \
	len += sprintf(buf + len, "  card_ready:           %d\n", sdioh->io_ready); \
	len += sprintf(buf + len, "  num_io_func:          %d\n", sdioh->num_io_func); \
	len += sprintf(buf + len, "  mem_present:          %d\n", sdioh->mem_present); \
	for (i = 0; i <= sdioh->num_io_func; i++) { \
		len += sprintf(buf + len, "  f%d_cis:               0x%08X\n", \
				i, sdioh->fn_cis[i]); \
	} \
}

#if DEBUG
typedef struct sdio_stats_t
{
	uint32	isr_called;
	uint32	send_sd_command;
	uint32	invalid_parameter;
	uint32	write_timeouts;
	uint32	read_timeouts;
	uint32	alignment_error;
	uint32	r1_illegal_command;
	uint32	r1_com_crc_error;
	uint32	r1_func_num_error;
	uint32	r1_address_error;
	uint32	r1_parameter_error;
	uint32	data_error_token;
	uint32	data_not_accepted;
} sdio_stats_t;

#define STATS_ISR_CALLED() (sdio_stats.isr_called++)
#define STATS_SEND_SPI_COMMAND() (sdio_stats.send_sd_command++)
#define STATS_INVALID_PARAMETER() (sdio_stats.invalid_parameter++)
#define STATS_WRITE_TIMEOUT() (sdio_stats.write_timeouts++)
#define STATS_READ_TIMEOUT() (sdio_stats.read_timeouts++)
#define STATS_ALIGNMENT_ERROR() (sdio_stats.alignment_error++)
#define STATS_R1_ILLEGAL_COMMAND() (sdio_stats.r1_illegal_command++)
#define STATS_R1_COM_CRC_ERROR() (sdio_stats.r1_com_crc_error++)
#define STATS_R1_FUNC_NUM_ERROR() (sdio_stats.r1_func_num_error++)
#define STATS_R1_ADDRESS_ERROR() (sdio_stats.r1_address_error++)
#define STATS_R1_PARAMETER_ERROR() (sdio_stats.r1_parameter_error++)
#define STATS_DATA_ERROR_TOKEN() (sdio_stats.data_error_token++)
#define STATS_DATA_NOT_ACCEPTED() (sdio_stats.data_not_accepted++)

#define STATS_PRINT(buf, len) {\
	len += sprintf(buf + len, "sdio stats:\n"); \
	len += sprintf(buf + len, "  isr_called:         %d\n", sdio_stats.isr_called); \
	len += sprintf(buf + len, "  send_sd_command:   %d\n", sdio_stats.send_sd_command); \
	len += sprintf(buf + len, "  invalid_parameter:  %d\n", sdio_stats.invalid_parameter); \
	len += sprintf(buf + len, "  write_timeouts:     %d\n", sdio_stats.write_timeouts); \
	len += sprintf(buf + len, "  read_timeouts:      %d\n", sdio_stats.read_timeouts); \
	len += sprintf(buf + len, "  alignment_error:    %d\n", sdio_stats.alignment_error); \
	len += sprintf(buf + len, "  r1_illegal_command: %d\n", sdio_stats.r1_illegal_command); \
	len += sprintf(buf + len, "  r1_com_crc_error:   %d\n", sdio_stats.r1_com_crc_error); \
	len += sprintf(buf + len, "  r1_func_num_error:  %d\n", sdio_stats.r1_func_num_error); \
	len += sprintf(buf + len, "  r1_address_error:   %d\n", sdio_stats.r1_address_error); \
	len += sprintf(buf + len, "  r1_parameter_error: %d\n", sdio_stats.r1_parameter_error); \
	len += sprintf(buf + len, "  data_error_token:   %d\n", sdio_stats.data_error_token); \
	len += sprintf(buf + len, "  data_not_accepted:  %d\n", sdio_stats.data_not_accepted); \
}

#else
#define STATS_ISR_CALLED()
#define STATS_SEND_SPI_COMMAND()
#define STATS_INVALID_PARAMETER()
#define STATS_WRITE_TIMEOUT()
#define STATS_READ_TIMEOUT()
#define STATS_ALIGNMENT_ERROR()
#define STATS_R1_ILLEGAL_COMMAND()
#define STATS_R1_COM_CRC_ERROR()
#define STATS_R1_FUNC_NUM_ERROR()
#define STATS_R1_ADDRESS_ERROR()
#define STATS_R1_PARAMETER_ERROR()
#define STATS_DATA_ERROR_TOKEN()
#define STATS_DATA_NOT_ACCEPTED()

#define STATS_PRINT(buf, len)
#endif	/* DEBUG */

/* ---- Private Variables ------------------------------------------------ */

#if DEBUG
static sdio_stats_t sdio_stats = {0};
#endif /* DEBUG */

typedef struct sd_cmd0_arg_t
{
	uint32 stuff;
} sd_cmd0_arg_t;

typedef struct sd_cmd3_arg_t
{
	uint32 stuff;
} sd_cmd3_arg_t;

#define SD_CMD5_ARG_IO_OCR_MASK		0x00FFFFFF /* Argument mask */
#define SD_CMD5_ARG_IO_OCR_SHIFT	0

typedef struct sd_cmd5_arg_t
{
	uint32 io_ocr;
} sd_cmd5_arg_t;

#define SD_CMD7_ARG_RCA_SHIFT		16	/* No. of bits for shift */

typedef struct sd_cmd7_arg_t
{
	uint16 rca;
} sd_cmd7_arg_t;

typedef struct sd_cmd15_arg_t
{
	uint16 rca;
} sd_cmd15_arg_t;

typedef struct sd_cmd52_arg_t
{
	uint8 rw_flag;
	uint8 func_num;
	uint8 raw_flag;
	uint32 address;
	uint8 data;
} sd_cmd52_arg_t;

typedef struct sd_cmd53_arg_t
{
	uint8 rw_flag;
	uint8 func_num;
	uint8 block_mode;
	uint8 op_code;
	uint32 address;
	uint16 blknum_bytecnt;
	bool  arc_en;
} sd_cmd53_arg_t;

#define SD_CMD59_ARG_CRC_OPTION_MASK	0x01	/* Mask for CRC option */
#define SD_CMD59_ARG_CRC_OPTION_SHIFT	0

typedef struct sd_cmd59_arg_t
{
	uint8 crc_option;
} sd_cmd59_arg_t;

typedef union sd_arg_t
{
	sd_cmd0_arg_t	cmd0;
	sd_cmd0_arg_t	cmd3;
	sd_cmd5_arg_t	cmd5;
	sd_cmd7_arg_t	cmd7;
	sd_cmd15_arg_t	cmd15;
	sd_cmd52_arg_t	cmd52;
	sd_cmd53_arg_t	cmd53;
	sd_cmd59_arg_t	cmd59;
} sd_arg_t;

/* response structure */
typedef struct sd_r3_resp_t
{
	uint16	rca;
} sd_r3_resp_t;

typedef struct sd_r4_resp_t
{
	uint8	card_ready;
	uint8	num_io_func;
	uint8	mem_present;
	uint32	io_ocr;
} sd_r4_resp_t;

typedef struct sd_r5_resp_t
{
	uint8  	data;
	uint8	flags;
} sd_r5_resp_t;

typedef struct sd_r7_resp_t
{
	uint16	rca;
} sd_r7_resp_t;

typedef struct sd_resp_t
{
	uint8	r1;
	uint32  cardstatus;
	union
	{
		sd_r3_resp_t	r3;
		sd_r4_resp_t	r4;
		sd_r5_resp_t	r5;
		sd_r7_resp_t	r7;
	} data;
} sd_resp_t;

/* ******************* Function declaration **************** */
/* lower level PCI r/w */
static void sdioh_reg_write(sdioh_info_t *sdioh, uint32 reg_offset, uint32 val);
static uint32 sdioh_reg_read(sdioh_info_t *sdioh, uint32 reg_offset);

/* sdio host control utilities */
#ifdef NOTUSED
static void sdioh_host_regbit_set(sdioh_info_t *sdioh, uint32 regoff, uint32 mask, uint32 val);
#endif /* NOTUSED */
static uint sdioh_host_regbit_poll(sdioh_info_t *sdioh, uint reg_offset, uint bitmask,
                                   bool set_clear, uint cnt_limit, bool *success);
static void sdioh_host_mode(sdioh_info_t *sdioh, uint32 mask, uint32 val);
static void sdioh_host_onoff(sdioh_info_t *sdioh, bool val);
static void sdioh_host_opmode_set(sdioh_info_t *sdioh, int newcardmode);
static void sdioh_host_blksize_set(sdioh_info_t *sdioh, uint16 blk_size);
static void sdioh_host_codec_dma_start(sdioh_info_t *sdioh, uint tx_rx);
#ifdef NOTUSED
static void sdioh_host_codec_dma_onoff(sdioh_info_t *sdioh, uint tx_rx, bool on);
static void sdioh_host_codec_dma_fill(sdioh_info_t *sdioh, uint tx_rx, uint8 *dsp, uint8 *buf,
                                      uint bytes);
static void sdioh_host_codec_reg(sdioh_info_t *sdioh, uint32 regoff, uint32 mask, uint32 val);
#endif /* NOTUSED */

static uint sdioh_core_opmode_set(sdioh_info_t *sdioh, int opmode);
static void sdioh_core_blksize_set(sdioh_info_t *sdioh, uint func, uint16 blk_size);

/* sdioh host/core INIT and RESET */
static bool sdioh_dev_find(osl_t *osh, uint32 *pci_mem_va);
static void sdioh_init_host(sdioh_info_t *sdioh);
static bool sdioh_init_core(sdioh_info_t *sdioh, uint opmode);
static int sdioh_init_corestart(sdioh_info_t *sdioh, uint opmode);
static bool sdioh_reset_core(sdioh_info_t *sdioh);
static void sdioh_reset_host(sdioh_info_t *sdioh, bool warm);
static void sdioh_init_hostdma(sdioh_info_t *sdioh);

static int sdioh_buf_pkt(sdioh_info_t *sdioh, uint dma, uint inc, uint rw, uint fnc,
	uint addr, bool wordreg, uint tot_bytes, uint8 *base);

/* sdioh host/core command SEND and response CHECK */
static int sdioh_cmd_send(sdioh_info_t *sdioh, bool dma_on, uint cmd_index, uint32 cmd_content,
	uint32 *sh_status, uint32 *sh_error, uint32 c53_bn_bc, bool cmd_type, bool arc_en,
	bool blockmode, bool append, bool abort, bool notpolling);
static int sdio_cmd_build(sdioh_info_t *sdioh, uint8 cmd, const sd_arg_t *arg, uint32 *builtcmd);
static int sdioh_cmd_build_and_send(sdioh_info_t *sdioh, uint cmd_index, sd_arg_t *arg,
	uint *sd_status, uint *sd_error, uint8 send_method);
static int sdio_cmd_check(sdioh_info_t *sdioh, uint32 cmd_index, uint32 rsp1, uint32 rsp2,
	sd_resp_t *resp);

/* sdio command protocol */
static int sdio_cmd3(sdioh_info_t *sdioh, uint16 *newrca);
static int sdio_cmd5(sdioh_info_t *sdioh, uint32 ioocr, uint *newioocr, uint8 *num_io_func,
	bool *mem_present, bool *io_ready, bool *nr);
static int sdio_cmd7(sdioh_info_t *sdioh, uint16 rca);
static int sdio_cmd15(sdioh_info_t *sdioh, uint16 rca);

static int sdio_cmd52(sdioh_info_t *sdioh, uint rw, uint fnc, uint raw, uint32 addr,
	uint8 data, uint8 *rspdata);
static int sdio_cmd53(sdioh_info_t *sdioh, uint dma_pio, uint blkmode, uint rw, uint fnc,
	uint op, uint32 addr, uint cnt, uint8 *datastream);

/* diagnostic routines */
#ifdef NOTUSED
static int diag_sdio_sb_buf_loopback(sdioh_info_t *sdioh, bool inc, uint addr, uint tot_bytes);
#endif /* NOTUSED */
static void diag_dumpreg_sdioh(sdioh_info_t *sdioh);
static void diag_dumpreg_sdio(sdioh_info_t *sdioh);

/* ******************* lower level PCI r/w******************************** */

/* linux version of R/W_REG */
#define SDIOH_R_REG(NULL, r) (\
	sizeof(*(r)) == sizeof(uint8) ? readb((volatile uint8*)(r)) : \
	sizeof(*(r)) == sizeof(uint16) ? readw((volatile uint16*)(r)) : \
	readl((volatile uint32*)(r)) \
)
#define SDIOH_W_REG(NULL, r, v) do { \
	switch (sizeof(*(r))) { \
	case sizeof(uint8):	writeb((uint8)(v), (volatile uint8*)(r)); break; \
	case sizeof(uint16):	writew((uint16)(v), (volatile uint16*)(r)); break; \
	case sizeof(uint32):	writel((uint32)(v), (volatile uint32*)(r)); break; \
	} \
} while (0)

static void
sdioh_reg_write(sdioh_info_t *sdioh, uint32 reg_offset, uint32 val)
{
#ifdef BCMDRIVER
	SDIOH_W_REG(NULL, (uint32 *)((uint32)(sdioh->pci_mem_regsva) + reg_offset), val);
#else	/* for epidiag */
extern int episdio_pciwrite_word(uint32 addr, uint32 *data);
	episdio_pciwrite_word(((uint32)(sdioh->pci_mem_regsva) + reg_offset), &val);
#endif /* BCMDRIVER */
}

static uint32
sdioh_reg_read(sdioh_info_t *sdioh, uint32 reg_offset)
{
	uint32 tmp;
#ifdef BCMDRIVER
	tmp = SDIOH_R_REG(NULL, (uint32 *)((uint32)(sdioh->pci_mem_regsva) + reg_offset));
#else /* for epidiag */
extern int episdio_pciread_word(uint32 addr, uint32 *data);
	if (episdio_pciread_word(((uint32)(sdioh->pci_mem_regsva) + reg_offset), &tmp))
		return 0xffffff;
#endif /* BCMDRIVER */
	return tmp;
}

/* ******************* SDIOH register access******************************** */
#ifdef NOTUSED
static void
sdioh_host_regbit_set(sdioh_info_t *sdioh, uint32 regoff, uint32 mask, uint32 val)
{
	uint32 tmp;
	tmp = sdioh_reg_read(sdioh, regoff);

	ASSERT((val & ~mask) == 0);

	if ((tmp & mask) == val) {
		SDIOH_INFO(("SDIOH reg 0x%x bit(s) 0x%x is already set, no change\n", regoff,
		            mask));
	} else {
		SDIOH_INFO(("SDIOH reg 0x%x bit(s) 0x%x changed to 0x%x\n", regoff, (tmp & mask),
		            val));

		tmp = (tmp & ~mask) | val;
		sdioh_reg_write(sdioh, regoff, tmp);
	}
}
#endif /* NOTUSED */

static uint
sdioh_host_regbit_poll(sdioh_info_t *sdioh, uint reg_offset, uint bitmask, bool set_clear,
                       uint cnt_limit, bool *success)
{
	uint count = 0;
	uint regval;

	*success = TRUE;

	while (1) {
		regval = sdioh_reg_read(sdioh, reg_offset);

		regval &= bitmask;

		if (set_clear) {
			if (regval == 0) break;
		} else {
			if (regval != 0) break;
		}

		count++;
		if (count >= cnt_limit)
			break;
	}

	if (count >= sdioh->rdto) {
		SDIOH_ERROR(("SDIOH_ERROR!!!fatal, polling timeout %d at reg_oft 0x%x bitmask"
			     " 0x%x\n",
			     count, reg_offset, bitmask));
		*success = FALSE;
		return 0;
	}

	return regval;
}

static void
sdioh_host_mode(sdioh_info_t *sdioh, uint32 mask, uint32 val)
{
	uint32 curmode;
	curmode = sdioh_reg_read(sdioh, OFFSETOF(sdioh_regs_t, mode));

	ASSERT((val & ~mask) == 0);

	if ((curmode & mask) == val) {
		SDIOH_INFO(("SDIOH mode bit(s) 0x%x is already set, no change\n", mask));
	} else {
		SDIOH_INFO(("SDIOH mode bit(s) 0x%x changed to 0x%x\n", (curmode & mask), val));

		curmode = (curmode & ~mask) | val;

		sdioh_reg_write(sdioh, OFFSETOF(sdioh_regs_t, mode), curmode);
	}
}

static void
sdioh_host_onoff(sdioh_info_t *sdioh, bool val)
{
	uint32 curmode;
	curmode = sdioh_reg_read(sdioh, OFFSETOF(sdioh_regs_t, devcontrol));

	if (val) {
		if (!(curmode & CODEC_DEVCTRL_SDIOH)) {
			SDIOH_INFO(("soft reset clear. assert it\n"));
			curmode |= CODEC_DEVCTRL_SDIOH;
			sdioh_reg_write(sdioh, OFFSETOF(sdioh_regs_t, devcontrol), curmode);
		}
	} else {
		if (curmode & CODEC_DEVCTRL_SDIOH) {
			SDIOH_INFO(("soft reset was asserted. clear it\n"));
			curmode &= ~CODEC_DEVCTRL_SDIOH;
			sdioh_reg_write(sdioh, OFFSETOF(sdioh_regs_t, devcontrol), curmode);
		}
	}
}

static void
sdioh_host_opmode_set(sdioh_info_t *sdioh, int newcardmode)
{
	uint32 curmode, newmode, curcardmode;

	curmode = sdioh_reg_read(sdioh, OFFSETOF(sdioh_regs_t, mode));

	curcardmode = curmode & MODE_OP_MASK;

	if (curcardmode != newcardmode) {
		newmode = (curmode & ~MODE_OP_MASK) | newcardmode;
		sdioh_reg_write(sdioh, OFFSETOF(sdioh_regs_t, mode), newmode);

		OSL_DELAY(20); /* I get errors without delay or the printf */
		SDIOH_INFO(("SDIOH mode switch from %d to %d\n", curcardmode, newcardmode));
	}
}

static void
sdioh_host_blksize_set(sdioh_info_t *sdioh, uint16 blk_size)
{
	ASSERT((blk_size < SDIOH_BLOCK_SIZE_MAX) && (blk_size >= SDIOH_BLOCK_SIZE_MIN));

	/* update sw cache */
	sdioh->cmd53_blk_size = blk_size;

	/* set host controller */
	sdioh_reg_write(sdioh, OFFSETOF(sdioh_regs_t, blocksize), blk_size);
}

static void
sdioh_host_codec_dma_start(sdioh_info_t *sdioh, uint tx_rx)
{
	if (tx_rx == SDIOH_DMA_TX)
		sdioh_reg_write(sdioh, sdioh->PCI_DMA_BASE + OFFSETOF(dma32regs_t, ptr), 8);
	else
		sdioh_reg_write(sdioh, sdioh->PCI_DMA_BASE + sizeof(dma32regs_t) +
		                OFFSETOF(dma32regs_t, ptr), 8);
}

#ifdef NOTUSED
static void
sdioh_host_codec_dma_onoff(sdioh_info_t *sdioh, uint tx_rx, bool on)
{
	uint reg_off;
	if (tx_rx == SDIOH_DMA_TX) {
		reg_off = sdioh->PCI_DMA_BASE + OFFSETOF(dma32regs_t, control);
		if (on)
			sdioh_host_regbit_set(sdioh, reg_off, XC_XE, XC_XE);
		else
			sdioh_host_regbit_set(sdioh, reg_off, XC_XE, 0);

	} else {
		reg_off = sdioh->PCI_DMA_BASE + sizeof(dma32regs_t) +
			OFFSETOF(dma32regs_t, control);
		if (on)
			sdioh_host_regbit_set(sdioh, reg_off, RC_RE, RC_RE);
		else
			sdioh_host_regbit_set(sdioh, reg_off, RC_RE, 0);
	}
}

static void
sdioh_host_codec_dma_fill(sdioh_info_t *sdioh, uint tx_rx, uint8 *dsp, uint8 *buf, uint bytes)
{
	if (tx_rx == SDIOH_DMA_TX) {
		sdioh_reg_write(sdioh, sdioh->PCI_DMA_BASE + OFFSETOF(dma32regs_t, addr),
		                (uint32)dsp);
	} else {
		sdioh_reg_write(sdioh, sdioh->PCI_DMA_BASE + sizeof(dma32regs_t) +
		                OFFSETOF(dma32regs_t, addr), (uint32)dsp);
	}
	*(uint32 *)(dsp)     = 0xe0000000 | bytes;
	*(uint32 *)(dsp + 4) = (uint32)(buf);
}

static void
sdioh_host_codec_reg(sdioh_info_t *sdioh, uint32 regoff, uint32 mask, uint32 val)
{
	uint32 tmp;
	tmp = sdioh_reg_read(sdioh, regoff);

	ASSERT((val & ~mask) == 0);

	if ((tmp & mask) == val) {
		SDIOH_INFO(("SDIOH reg 0x%x bit(s) 0x%x is already set, no change\n", regoff,
		            mask));
	} else {
		SDIOH_INFO(("SDIOH reg 0x%x bit(s) 0x%x changed to 0x%x\n", regoff, (tmp & mask),
		            val));

		tmp = (tmp & ~mask) | val;
		sdioh_reg_write(sdioh, regoff, tmp);
	}
}
#endif /* NOTUSED */

static uint
sdioh_core_opmode_set(sdioh_info_t *sdioh, int opmode)
{
	uint8 busint_val = 0, busint_val_new = 0;
	uint8 data = 0;

	if (opmode == MODE_OP_SPI) {
		/* tbd */
	} else {
		/* read-modify-write */
		if (SDIOH_CMDPASS != sdio_cmd52(sdioh, SD_IO_OP_READ, SDIO_FUNC_0, SD_IO_RW_NORMAL,
		                                OFFSETOF(sdio_regs_t, bus_inter), 0, &data))
			return SDIOH_FAIL;
		busint_val = (data & 0xFC); /* why not ~BUS_SD_DATA_WIDTH_MASK? */

		if (opmode == MODE_OP_SDIO4BIT)
			busint_val |= BUS_SD_DATA_WIDTH_4BIT & BUS_SD_DATA_WIDTH_MASK;
		else if (opmode == MODE_OP_SDIO1BIT)
			busint_val |= BUS_SD_DATA_WIDTH_1BIT & BUS_SD_DATA_WIDTH_MASK;
		else {
			SDIOH_ERROR(("invalid opmode %d\n", opmode));
			return SDIOH_FAIL;
		}

		/* use RAW=1 mode to verify the data */
		if (SDIOH_CMDPASS != sdio_cmd52(sdioh, SD_IO_OP_WRITE, 0, SD_IO_RW_RAW,
		                                OFFSETOF(sdio_regs_t, bus_inter), busint_val,
		                                &data))
			return SDIOH_FAIL;

		busint_val_new = data;
		if (busint_val != busint_val_new) {
			SDIOH_ERROR(("RAW mode does not work or bus_interface data not gets"
				     " written\n"));
			return SDIOH_FAIL;
		} else {
			if (opmode == MODE_OP_SDIO1BIT)
				SDIOH_INFO(("Card mode switched to 1 bit mode\n"));
			else if (opmode == MODE_OP_SDIO4BIT)
				SDIOH_INFO(("Card mode switched to 4 bit mode\n"));

			sdioh_host_opmode_set(sdioh, opmode);
			return 0;
		}
	}
	return 0;
}

static void
sdioh_core_blksize_set(sdioh_info_t *sdioh, uint func, uint16 blk_size)
{
	uint addr = SDIOD_FBR_BASE(func) + OFFSETOF(sdio_fbr_t, fnx_blk_size);
	uint8 data = 0;

	/* Ignore request for nonexistent function */
	ASSERT(func <= sdioh->num_io_func);
	if (func > sdioh->num_io_func)
		return;

	SDIOH_DEBUG(("program block size %d to func %d addr 0x%x\n", blk_size, func, addr));
	sdio_cmd52(sdioh, SD_IO_OP_WRITE, SDIO_FUNC_0, SD_IO_RW_NORMAL, addr, blk_size & 0xff,
	           &data);
	sdio_cmd52(sdioh, SD_IO_OP_WRITE, SDIO_FUNC_0, SD_IO_RW_NORMAL, addr+1, blk_size >> 8,
	           &data);

	/* Record block size in SW cache */
	sdioh->fn_blk_size[func] = blk_size;
}

/* ******************* SDIOH init/stop ********************************* */
static bool
sdioh_dev_find(osl_t *osh, uint32 *pci_mem_va)
{
	sbconfig_t	*sb;
	uint32		sbidh;

#ifdef BCMDRIVER
	/* make sure pci bar0 window pointing to sdioh-codec core */
	OSL_PCI_WRITE_CONFIG(osh, PCI_BAR0_WIN, sizeof(uint32), (si_enum_base(0) +
	                                                         SDIOH_SB_ENUM_OFFSET));

	sb = (sbconfig_t *)((uint32)pci_mem_va + SBCONFIGOFF);
	sbidh = sb->sbidhigh;

	/* Check that it really is a sdioh core, rev >= 4 */
	if (((((sbidh & SBIDH_CC_MASK) >> SBIDH_CC_SHIFT) != CODEC_CORE_ID)) ||
	    (SBCOREREV(sbidh) < 4))
		return FALSE;

	SDIOH_INFO(("found SDIO host(rev %d) in pci device, membase 0x%x\n", SBCOREREV(sbidh),
	            (uint32)pci_mem_va));

	/* Enable interrupts from the sdioh core onto the PCI bus */
	OSL_PCI_WRITE_CONFIG(osh, PCI_INT_MASK, sizeof(uint32), 0x800);

	return TRUE;

#else /* !BCMDRIVER */

	/* for epidiag */
	extern int sdioh_dev_detect(void *hdl);
	return sdioh_dev_detect(NULL);

#endif /* !BCMDRIVER */
}

static bool
sdioh_init_core(sdioh_info_t *sdioh, uint opmode)
{
	SDIOH_TRACE(("sdioh_init_core, opmode %d\n", opmode));

	/* init sdio core */
	if (opmode == MODE_OP_SPI) {
		sdioh_host_opmode_set(sdioh, MODE_OP_SPI);
		SDIOH_ERROR(("sdio_init, MODE_OP_SPI not supported !\n"));
		return TRUE;

	} else if (opmode == MODE_OP_SDIO1BIT) {
		sdioh_host_opmode_set(sdioh, (int)MODE_OP_SDIO1BIT);

		if (sdioh_init_corestart(sdioh, MODE_OP_SDIO1BIT) != SD_CARD_TYPE_IO) {
			SDIOH_ERROR(("sdio_init: cardtype not supported !\n"));
			return FALSE;
		}

	} else if (opmode == MODE_OP_SDIO4BIT) {
		/* switch to 1BIT mode for setup */
		sdioh_host_opmode_set(sdioh, (int)MODE_OP_SDIO1BIT);

		if (sdioh_init_corestart(sdioh, MODE_OP_SDIO1BIT) != SD_CARD_TYPE_IO) {
			SDIOH_ERROR(("sdio_init: cardtype not supported or could not init"
				     " core!\n"));
			return FALSE;
		}
		/* Card activated. Switch to SDIO 4 bit mode (!both device and host) */
		if (SDIOH_SUCCESS != sdioh_core_opmode_set(sdioh, MODE_OP_SDIO4BIT))
			return FALSE;

	} else {
		SDIOH_ERROR(("ERROR!!!!!: unknown sd mode\n"));
		return FALSE;
	}
	return TRUE;
}

static void
sdioh_init_host(sdioh_info_t *sdioh)
{
	uint i = 0;

	/* cold reset sdioh */
	sdioh_reset_host(sdioh, FALSE);

	/* setup sdioh registers */
	sdioh_init_hostdma(sdioh);

	if (sdioh->precmd_count_en)
		sdioh_host_mode(sdioh, MODE_PRECMD_CNT_EN, MODE_PRECMD_CNT_EN);

	if (sdioh->dontwait_dmadone)
		sdioh_host_mode(sdioh, MODE_DONT_WAIT_DMA, MODE_DONT_WAIT_DMA);

	/* Turn on external clock or divide to slower clock */
	if (sdioh_extclk) {
		sdioh_host_mode(sdioh, MODE_USE_EXT_CLK, MODE_USE_EXT_CLK);
	} else if (sdioh_slowclk) {
		sdioh_host_mode(sdioh, MODE_CLK_DIV_MASK, 0x10);
	} else {
		sdioh_host_mode(sdioh, MODE_CLK_DIV_MASK, 0x0);
	}

	/* init mask */
	sdioh_reg_write(sdioh, OFFSETOF(sdioh_regs_t, errormask), sdioh->sderrormask);
	sdioh_reg_write(sdioh, OFFSETOF(sdioh_regs_t, intmask), sdioh->sdintmask);

	/* init auto response value and mask */
	if (sdioh->sdio_mode == MODE_OP_SPI)
		i = (sdioh->arvm_spi_mask << AR_MASK_OFT) + sdioh->arvm_spi_value;
	else
		i = (sdioh->arvm_sdx_mask << AR_MASK_OFT) + sdioh->arvm_sdx_value;
	sdioh_reg_write(sdioh, OFFSETOF(sdioh_regs_t, arvm), i);

	/* init read timeout and response timeout */
	sdioh_reg_write(sdioh, OFFSETOF(sdioh_regs_t, rdto), sdioh->rdto);
	sdioh_reg_write(sdioh, OFFSETOF(sdioh_regs_t, rbto), sdioh->rbto);

	/* init delay */
	i = ((sdioh->sddelay_precount << DLY_CLK_COUNT_PRE_O) & DLY_CLK_COUNT_PRE_M)
	  + ((sdioh->sddelay_txstartcount << DLY_TX_START_COUNT_O) &DLY_TX_START_COUNT_M);
	sdioh_reg_write(sdioh, OFFSETOF(sdioh_regs_t, delay), i);

	/* !! enable sd clock */
	sdioh_host_mode(sdioh, MODE_CLK_OUT_EN, MODE_CLK_OUT_EN);
}

static void
sdioh_init_hostdma(sdioh_info_t *sdioh)
{
	SDIOH_TRACE(("sdioh_init_hostdma\n"));

#ifdef BCMDRIVER

#else	/* for epidiag */
	sdioh->dma_xmt_buf  = (uint8 *)xmalloc(SDIOH_DMABUF_LEN);
	sdioh->dma_xmt_desp = (uint8 *)xmalloc(SDIOH_DMABUF_LEN / 2);
	sdioh->dma_rcv_buf  = (uint8 *)xmalloc(SDIOH_DMABUF_LEN);
	sdioh->dma_rcv_desp = (uint8 *)xmalloc(SDIOH_DMABUF_LEN / 2);
	ASSERT(ISALIGNED(sdioh->dma_xmt_buf, sizeof(uint32)));
	ASSERT(ISALIGNED(sdioh->dma_xmt_desp, sizeof(uint32)));
	ASSERT(ISALIGNED(sdioh->dma_rcv_buf, sizeof(uint32)));
	ASSERT(ISALIGNED(sdioh->dma_rcv_desp, sizeof(uint32)));
#endif /* BCMDRIVER */
	SDIOH_TRACE(("sdioh_init_hostdma done\n"));
}

static int
sdioh_init_corestart(sdioh_info_t *sdioh, uint opmode)
{
	bool test_flag_A, test_flag_B, test_init_memory;
	bool is_io, is_mem, is_pi;
	uint init_ioocr = 0;
	uint retrycount;
	uint rc;
	uint fn;
	uint retrylimit = sdioh->initcmd5_retry_limit;
	uint card_type = SD_CARD_TYPE_UNKNOWN;
	uint8 data = 0;

	test_flag_A = test_flag_B = test_init_memory = FALSE;
	is_io = is_mem = is_pi = FALSE;

	rc = sdio_cmd5(sdioh, init_ioocr, &sdioh->ioocr, &sdioh->num_io_func, &sdioh->mem_present,
	               &sdioh->io_ready, &sdioh->notready);

	if (SDIOH_CMDPASS != rc) {
		SDIOH_ERROR(("ERROR!!!!!sdio_core_init: CMD5 FAILED !!!\n"));
		sdioh->initialized = FALSE;
		return SD_CARD_TYPE_UNKNOWN;
	}

	SDIOH_INFO(("ioready %d num_iofunc %d mem_present %d ioocr 0x%x nr %d \n",
	            sdioh->io_ready, sdioh->num_io_func, sdioh->mem_present, sdioh->ioocr,
	            sdioh->notready));

	if (sdioh->notready || ((sdioh->num_io_func == 0) && sdioh->mem_present)) {
		test_init_memory = TRUE;
	} else if ((sdioh->num_io_func == 0) && !sdioh->mem_present) {
		test_flag_A = TRUE;
	} else if (sdioh->num_io_func > 0) {
		if (!(sdioh->ioocr & sdioh->host_support_ocr)) {
			/* OCR invalid */
			if (sdioh->mem_present)
				test_init_memory = TRUE;
			else
				test_flag_A = TRUE;
		} else	{
			SDIOH_INFO(("OCR valid, set new voltage 0x%x\n", sdioh->host_support_ocr));
			sdioh->io_ready = FALSE;
			sdioh->notready = FALSE;
			retrycount = 0;
			/* issue CMD5 with new ioocr */
			while ((!sdioh->io_ready) && (sdioh->notready == 0) && (retrycount <
										retrylimit)) {
				sdio_cmd5(sdioh, sdioh->host_support_ocr, &sdioh->ioocr,
				          &sdioh->num_io_func, &sdioh->mem_present,
				          &sdioh->io_ready,
				          &sdioh->notready);
				retrycount++;
			}

			if (sdioh->notready) {
				SDIOH_INFO(("sdio_core_init: Card is in Inactive State, NR of"
					    " CMD5\n"));
			} else if (retrycount >= retrylimit) {
				SDIOH_ERROR(("set ioocr exceeds retry limit\n"));
				if (opmode == MODE_OP_SPI) {
					if (!sdioh->mem_present) {
						test_flag_A = TRUE;
					}
				} else {
					test_init_memory = TRUE;
				}
			} else {
				/* g_ioready must be 1 now, test MP */
				is_io = TRUE;
				if (!is_pi && sdioh->mem_present) {
					/* "CardInitSD: Init Memory ? */
				} else if (is_pi || !sdioh->mem_present) {
					test_flag_A = TRUE;
				}
			}
		}
	}

	/* test Init Memory */
	if (sdioh->initmemory && test_init_memory) {
		/* tbd */
	} else {
		test_flag_A = TRUE;
	}

	if (test_flag_A) {
		if (is_io || is_mem) {
			if (opmode == MODE_OP_SPI) {
				/* tbd */
			} else {
				sdio_cmd3(sdioh, &sdioh->rca);
				is_pi = TRUE;
			}
			test_flag_B = TRUE;
		} else {
			if (opmode == MODE_OP_SPI) {
				SDIOH_ERROR(("sdio_core_init: SPI MODE: Card is rejected\n"));
				return SD_CARD_TYPE_UNKNOWN;
			} else {
				if (0) {
					sdio_cmd15(sdioh, 0);
				}
				SDIOH_ERROR(("sdio_core_init: !!!BAD, Card is in Inactive state by"
					     " CMD15\n"));
				sdioh->initialized = FALSE;
				return SD_CARD_TYPE_UNKNOWN;
			}
		}
	}

	if (test_flag_B) {
		if (is_io && is_mem) {
			SDIOH_INFO(("This is a COMBO card\n"));
			card_type = SD_CARD_TYPE_COMBO;
		} else if (is_io && !is_mem) {
			SDIOH_INFO(("This is an IO Only card\n"));
			card_type = SD_CARD_TYPE_IO;
		} else if (!is_io && is_mem) {
			SDIOH_INFO(("This is a Memory Only card\n"));
			card_type = SD_CARD_TYPE_MEMORY;
		}
	}

	/* get right rca */
	rc = sdio_cmd3(sdioh, &sdioh->rca);
	rc = sdio_cmd3(sdioh, &sdioh->rca);
	sdio_cmd7(sdioh, sdioh->rca);

	SDIOH_INFO(("RCA is %d\n", sdioh->rca));

	/* save cis base address */

	sdio_cmd52(sdioh, SD_IO_OP_READ, SDIO_FUNC_0, SD_IO_RW_NORMAL,
	           OFFSETOF(sdio_regs_t, cis_base_low), 0, &data);
	sdioh->fn_cis[0] = data;
	sdio_cmd52(sdioh, SD_IO_OP_READ, SDIO_FUNC_0, SD_IO_RW_NORMAL,
	           OFFSETOF(sdio_regs_t, cis_base_mid), 0, &data);
	sdioh->fn_cis[0] += data << 8;
	sdio_cmd52(sdioh, SD_IO_OP_READ, SDIO_FUNC_0, SD_IO_RW_NORMAL,
	           OFFSETOF(sdio_regs_t, cis_base_high), 0, &data);
	sdioh->fn_cis[0] += data << 16;
	sdioh->fn_cis[0] &= SBSDIO_CIS_OFT_ADDR_MASK;

	SDIOH_INFO(("sdio_core_init: common cis base is 0x%x\n", sdioh->fn_cis[0]));

	for (fn = 1; fn <= sdioh->num_io_func; fn++) {
		sdio_cmd52(sdioh, SD_IO_OP_READ, SDIO_FUNC_0, SD_IO_RW_NORMAL,
		           (SDIOD_FBR_BASE(fn) + OFFSETOF(sdio_fbr_t, cis_low)), 0, &data);
		sdioh->fn_cis[fn] = data;
		sdio_cmd52(sdioh, SD_IO_OP_READ, SDIO_FUNC_0, SD_IO_RW_NORMAL,
		           (SDIOD_FBR_BASE(fn) + OFFSETOF(sdio_fbr_t, cis_mid)), 0, &data);
		sdioh->fn_cis[fn] += data << 8;
		sdio_cmd52(sdioh, SD_IO_OP_READ, SDIO_FUNC_0, SD_IO_RW_NORMAL,
		           (SDIOD_FBR_BASE(fn) + OFFSETOF(sdio_fbr_t, cis_high)), 0, &data);
		sdioh->fn_cis[fn] += data << 16;
		SDIOH_INFO(("sdio_core_init: func%d cis base is 0x%x\n", fn, sdioh->fn_cis[fn]));
	}

	if (card_type == SD_CARD_TYPE_IO) {
		SDIOH_INFO(("sdio_core_init: enable function 1\n"));
		sdio_cmd52(sdioh, SD_IO_OP_WRITE, SDIO_FUNC_0, SD_IO_RW_NORMAL,
		           OFFSETOF(sdio_regs_t, io_en), SDIO_FUNC_ENABLE_1,  &data);
	}

	/* some sanity checking */
	if (sdioh->fn_cis[0] != SBSDIO_CIS_BASE_COMMON)
		return SD_CARD_TYPE_UNKNOWN;

	return card_type;
}

static bool
sdioh_reset_core(sdioh_info_t *sdioh)
{
	int res;
	uint8 data = 0;
	SDIOH_INFO(("sdio_core_reset\n"));

	res = sdio_cmd52(sdioh, SD_IO_OP_WRITE, SDIO_FUNC_0, SD_IO_RW_NORMAL,
	                 OFFSETOF(sdio_regs_t, io_abort), IO_ABORT_RESET_ALL, &data);

	/* on reset, card's RCA should return to 0 */
	sdioh->rca = 0;

	if (res != SDIOH_SUCCESS) {
		SDIOH_ERROR(("sdio_core_reset\n"));
		return FALSE;
	}

	/* Give the device a chance to complete reset (for testing) */
	OSL_DELAY(sdioh_resetwait);

	res = sdio_cmd52(sdioh, SD_IO_OP_READ, SDIO_FUNC_0, SD_IO_RW_NORMAL,
	                 OFFSETOF(sdio_regs_t, io_abort), 0, &data);
	SDIOH_INFO(("%s: read io_abort after reset %s, data 0x%02x\n", __FUNCTION__,
	            ((res == SDIOH_SUCCESS) ? "SUCCEEDED" : "FAILED"), data));

	return TRUE;
}

static void
sdioh_reset_host(sdioh_info_t *sdioh, bool warm)
{
	uint cur_val[10] = { 0 };

	SDIOH_INFO(("sdioh_host_reset, warm/cold=%d\n", warm));

	if (warm) {
		/* save current state */
		cur_val[0] = sdioh_reg_read(sdioh, OFFSETOF(sdioh_regs_t, mode));
		cur_val[1] = sdioh_reg_read(sdioh, OFFSETOF(sdioh_regs_t, delay));
		cur_val[2] = sdioh_reg_read(sdioh, OFFSETOF(sdioh_regs_t, rdto));
		cur_val[3] = sdioh_reg_read(sdioh, OFFSETOF(sdioh_regs_t, rbto));
		cur_val[4] = sdioh_reg_read(sdioh, OFFSETOF(sdioh_regs_t, test));
		cur_val[5] = sdioh_reg_read(sdioh, OFFSETOF(sdioh_regs_t, arvm));
		cur_val[6] = sdioh_reg_read(sdioh, OFFSETOF(sdioh_regs_t, errormask));
		cur_val[7] = sdioh_reg_read(sdioh, OFFSETOF(sdioh_regs_t, intmask));
		cur_val[8] = sdioh_reg_read(sdioh, OFFSETOF(sdioh_regs_t, blocksize));
	}

	sdioh_host_onoff(sdioh, FALSE);
	sdioh_host_onoff(sdioh, TRUE);

	if (warm) {
		/* write back saved state */
		sdioh_reg_write(sdioh, OFFSETOF(sdioh_regs_t,  mode), cur_val[0]);
		sdioh_reg_write(sdioh, OFFSETOF(sdioh_regs_t,  delay), cur_val[1]);
		sdioh_reg_write(sdioh, OFFSETOF(sdioh_regs_t,  rbto), cur_val[2]);
		sdioh_reg_write(sdioh, OFFSETOF(sdioh_regs_t,  rdto), cur_val[3]);
		sdioh_reg_write(sdioh, OFFSETOF(sdioh_regs_t,  test), cur_val[4]);
		sdioh_reg_write(sdioh, OFFSETOF(sdioh_regs_t,  arvm), cur_val[5]);
		sdioh_reg_write(sdioh, OFFSETOF(sdioh_regs_t,  errormask), cur_val[6]);
		sdioh_reg_write(sdioh, OFFSETOF(sdioh_regs_t,  intmask), cur_val[7]);
		sdioh_reg_write(sdioh, OFFSETOF(sdioh_regs_t,  blocksize), cur_val[8]);
	}
}

/* ****************** Packet transfer  ********************************** */
/* Transfer whole buffer packet
 * Operation theory:
 * due to block mode restriction, block mode total bytes has to be multiple of block
 * size(sdioh->cmd53_blk_siz), extra bytes has to using byte mode,
 * due to host fifo restriction, PIO can't push more than SBSDIO_BYTEMODE_DATALEN_MAX bytes once

 * For DMA: bytemode_bytes = total_bytes % sdioh->cmd53_blk_siz
 * blockmode_bytes = total_bytes - bytemode_bytes
 * For PIO: num_of_blockmode = total_bytes / SBSDIO_BYTEMODE_DATALEN_MAX
 * blockmode_bytes = SBSDIO_BYTEMODE_DATALEN_MAX
 * bytemode_bytes = total_bytes % SBSDIO_BYTEMODE_DATALEN_MAX
 */

/* transmit/receive any number of bytes(whole pkt) from local storage(data_buf) */
static int
sdioh_buf_pkt(sdioh_info_t *sdioh, uint dma, uint inc, uint rw, uint fnc, uint addr, bool is4bytes,
              uint tot_bytes, uint8 *base)
{
	uint num_of_blockmode;
	uint blockmode_bytes = 0, bytemode_bytes = 0;
	uint8 *buf_start_offset = NULL;
	uint i = 0;

	/* use blockmode for main chunk and byte mode for leftover bytes */
	if (dma == SDIOH_MODE_PIO) {
		/* limited to fifo size, each block mode transfer can only take
		 * <= SBSDIO_BYTEMODE_DATALEN_MAX bytes
		 */
		num_of_blockmode = tot_bytes / SBSDIO_BYTEMODE_DATALEN_MAX;
		blockmode_bytes = SBSDIO_BYTEMODE_DATALEN_MAX;
		bytemode_bytes = tot_bytes % SBSDIO_BYTEMODE_DATALEN_MAX;

		SDIOH_DATA(("PIO, tot_bytes %d, num_of_blockmode %d, blockmode_bytes %d"
			    " bytemode_bytes %d\n",
			    tot_bytes, num_of_blockmode, blockmode_bytes, bytemode_bytes));

		/* send block mode first, how to use repetitive cmd53 ?? */
		if (num_of_blockmode > 0) {
			for (i = 0; i < num_of_blockmode; i++) {
				buf_start_offset = base + i * blockmode_bytes;
				SDIOH_DATA(("blockmode: addr 0x%x, buf_start_offset 0x%x\n", addr,
				            (uint32)buf_start_offset));
				if (SDIOH_CMDPASS != sdio_cmd53(sdioh, SDIOH_MODE_PIO,
				                                SD_IO_BLOCK_MODE, rw,
				                                fnc, inc, addr, blockmode_bytes,
				                                buf_start_offset)) {
					SDIOH_ERROR(("ERROR!!!!!%d blockmode cmd53 failure\n", i));
					return SDIOH_FAIL;
				}
				if (inc == SD_IO_INCREMENT_ADDRESS) {
					addr += blockmode_bytes;
				}
			}
			SDIOH_DATA(("blockmode done\n"));
		}

		/* send byte mode second if there is any */

		buf_start_offset = base + i * blockmode_bytes;
		if (bytemode_bytes > 0) {
			SDIOH_DATA(("bytemode: addr 0x%x, buf_start_offset 0x%x\n", addr,
			            (uint32)buf_start_offset));
			if (SDIOH_CMDPASS != sdio_cmd53(sdioh, SDIOH_MODE_PIO, SD_IO_BYTE_MODE, rw,
			                                fnc, inc, addr, bytemode_bytes,
			                                buf_start_offset)) {
				SDIOH_ERROR(("ERROR!!!!!bytemode cmd53 %d failure\n",
				             bytemode_bytes));
				return SDIOH_FAIL;
			}
			SDIOH_DATA(("bytemode done\n"));
		}
	} else { /* dma mode, no limitation of block mode length, but need to be divisible by
		  * blksize
		  */
		bytemode_bytes = tot_bytes % sdioh->fn_blk_size[fnc];
		blockmode_bytes = tot_bytes - bytemode_bytes;

		/* send block mode */
		buf_start_offset = base;
		if (blockmode_bytes > 0) {
			if (SDIOH_CMDPASS != sdio_cmd53(sdioh, SDIOH_MODE_DMA, SD_IO_BLOCK_MODE, rw,
			      fnc, inc, addr, blockmode_bytes, buf_start_offset)) {
				SDIOH_ERROR(("ERROR!!!blockmode cmd53 %d failure\n",
				             blockmode_bytes));
				return SDIOH_FAIL;
			}
			SDIOH_DATA(("blockmode done\n"));
		}

		buf_start_offset = base + blockmode_bytes;
		if (bytemode_bytes > 0) {
			if (SDIOH_CMDPASS != sdio_cmd53(sdioh, SDIOH_MODE_DMA, SD_IO_BYTE_MODE, rw,
			      fnc, inc, addr, bytemode_bytes, buf_start_offset)) {
				SDIOH_ERROR(("ERROR!!!bytemode cmd53 %d failure\n",
				             bytemode_bytes));
				return SDIOH_FAIL;
			}
			SDIOH_DATA(("byte mode done\n"));
		}
	}

	return SDIOH_SUCCESS;
}

/* Handler for device interrupts */
irqreturn_t
bcmsdioh_isr(int irq, void *arg, struct pt_regs *ptregs)
{
	/* Check if it's ours */
	sdioh_info_t *sdioh = (sdioh_info_t *)arg;
	uint32 intstatus;

	intstatus = sdioh_reg_read(sdioh, OFFSETOF(sdioh_regs_t, intstatus));
	intstatus &= sdioh->sdintmask;

	if (!intstatus)
		return IRQ_RETVAL(0); /* not ours */

	/* Acknowledge the interrupt bits */
	sdioh_reg_write(sdioh, OFFSETOF(sdioh_regs_t, intstatus), intstatus);

	/* Pass up client interrupts */
	if (intstatus & INT_DEV_INT) {
		if (sdioh->interrupt_enabled && sdioh->intr_handler_valid) {
			sdioh->intr_handler(sdioh->intr_handler_arg);
		} else {
			SDIOH_ERROR(("DEV_INT IN BAD STATE, ENABLED %d VALID %d\n",
			             sdioh->interrupt_enabled, sdioh->intr_handler_valid));
			SDIOH_ERROR(("MASKING FURTHER DEVICE INTERRUPTS\n"));
			sdioh->sdintmask &= ~INT_DEV_INT;
			sdioh_reg_write(sdioh, OFFSETOF(sdioh_regs_t, intmask), sdioh->sdintmask);
			printk("ISR: Wrote intmask 0x%08x, now reads 0x%08x\n",
			       sdioh->sdintmask,
			       sdioh_reg_read(sdioh, OFFSETOF(sdioh_regs_t, intmask)));

		}
	}

	/* Don't handle anything else right now */
	ASSERT((intstatus & ~INT_DEV_INT) == 0);

	return IRQ_RETVAL(1);
}

/* ****************** SDIOH APIs ********************************** */
extern sdioh_info_t *
sdioh_attach(osl_t *osh, void *cfghdl, uint irq)
{
	uint8 data = 0;
	uint32 word = 0;
	sdioh_info_t *sdioh;
	uint fn;

	SDIOH_TRACE(("sdioh_attach, osh 0x%x cfghdl 0x%x\n", (uint32)osh, (uint32)cfghdl));

	if ((sdioh = (sdioh_info_t *)MALLOC(osh, sizeof(sdioh_info_t))) == NULL) {
		SDIOH_ERROR(("sdioh_attach: out of memory, malloced %d bytes\n", MALLOCED(osh)));
		return NULL;
	}
	bzero((char *)sdioh, sizeof(sdioh_info_t));

	sdioh->osh = osh;

	if ((sdioh->pci_mem_regsva = ioremap_nocache((int32)cfghdl, 8192)) == NULL) {
		SDIOH_ERROR(("%s:ioremap() failed\n", __FUNCTION__));
		goto fail;
	}

	/* initialize structure */
	sdioh->PCI_DMA_BASE = OFFSETOF(sdioh_regs_t, dmaregs);

	sdioh->irq = irq;

	sdioh->initmemory = FALSE;
	sdioh->sdio_mode = MODE_OP_SDIO4BIT;
	sdioh->rca = 0;
	sdioh->host_support_ocr = SDIOH_HOST_SUPPORT_OCR;

	sdioh->initcmd5_retry_limit = 400;

	/* Start without any interrupts */
	sdioh->sdintmask = 0;
	sdioh->sderrormask = 0xffffffff;

	sdioh->dma_nochop = TRUE;
	sdioh->dma_start_mode = SDIOH_DMA_START_LATE;
	sdioh->dontwait_dmadone = FALSE;

	sdioh->card_blk_cap = FALSE;
	sdioh->cmd53_blk_size = 64;
	sdioh->precmd_count_en = FALSE;
	sdioh->sddelay_precount = 0;
	sdioh->sddelay_txstartcount = 0;
	sdioh->rdto = 25000;	/* Min timeout 1ms (at max 25 Mhz) */
	sdioh->rbto = 25000;

	sdioh->arvm_sdx_mask = 0xff;
	sdioh->arvm_sdx_value = 0x10;
	sdioh->arvm_spi_mask = 0xff;
	sdioh->arvm_spi_value = 0x00;

	sdioh->intr_handler = NULL;
	sdioh->intr_handler_arg = NULL;
	sdioh->intr_handler_valid = FALSE;
	sdioh->devrem_handler = NULL;
	sdioh->devrem_handler_arg = NULL;
	sdioh->devrem_handler_valid = FALSE;

	sdioh->dma_xmt_buf = NULL;
	sdioh->dma_xmt_desp = NULL;
	sdioh->dma_rcv_buf = NULL;
	sdioh->dma_rcv_desp = NULL;

	if (!sdioh_dev_find(osh, sdioh->pci_mem_regsva))
		goto fail;

	if (!request_irq(irq, bcmsdioh_isr, SA_SHIRQ, "bcmsdioh", sdioh) < 0)
		goto fail;

	/* init and start host */
	sdioh_init_host(sdioh);

	/* init and start device */
	sdioh->initialized = sdioh_init_core(sdioh, sdioh->sdio_mode);

	if (!sdioh->initialized)
		goto fail;

	sdioh_host_blksize_set(sdioh, sdioh->cmd53_blk_size);
	for (fn = 0; fn <= sdioh->num_io_func; fn++) {
		if (defblksz[fn])
			sdioh_core_blksize_set(sdioh, fn, defblksz[fn]);
	}

	SDIOH_INFO(("available commands: sdio sdioh\n"));
	sdio_cmd52(sdioh, SD_IO_OP_READ, SDIO_FUNC_0, SD_IO_RW_NORMAL,
	           OFFSETOF(sdio_regs_t, capability), 0, &data);
	sdioh->card_blk_cap = data & SDIO_CAP_SMB;
	SDIOH_INFO(("SDIO init SUCCEEDED: %s\n", (sdioh->card_blk_cap != 0) ?
		    "blockmode capable":""));

	if (SDIOH_CMDPASS == sdio_cmd53(sdioh, SDIOH_MODE_PIO, SD_IO_BYTE_MODE, SD_IO_OP_READ,
	                                SDIO_FUNC_1, SD_IO_FIXED_ADDRESS,
	                                (0x0 | SBSDIO_SB_ACCESS_2_4B_FLAG), 4, (uint8 *)&word)) {
		SDIOH_INFO(("%s: chipid 0x%x\n", __FUNCTION__, word));
		sdioh->target_dev = word;
	}

	sdioh->initialized = TRUE;

	return sdioh;

fail:
	SDIOH_ERROR(("%s: FAILURE\n", __FUNCTION__));
	if (sdioh) {
		free_irq(sdioh->irq, sdioh);
		if (sdioh->pci_mem_regsva)
		        iounmap(sdioh->pci_mem_regsva);
		MFREE(osh, sdioh, sizeof(sdioh_info_t));
	}
	return NULL;
}

extern SDIOH_API_RC
sdioh_detach(osl_t *osh, sdioh_info_t *sdioh)
{

	SDIOH_TRACE(("sdioh_detach, osh 0x%x sdioh 0x%x\n", (uint32)osh, (uint32)sdioh));

	if (sdioh) {
		/* Make sure sdioh interrupts are off */
		sdioh->sdintmask = 0;
		sdioh_reg_write(sdioh, OFFSETOF(sdioh_regs_t, intmask), sdioh->sdintmask);
		free_irq(sdioh->irq, sdioh);
	}

	if (sdioh_reset_core(sdioh))
		return SDIOH_API_RC_FAIL;

	sdioh_reset_host(sdioh, FALSE); /* reset to cold */

	if (sdioh)
		MFREE(sdioh->osh, sdioh, sizeof(sdioh_info_t));

	return SDIOH_API_RC_SUCCESS;
}

extern SDIOH_API_RC
sdioh_interrupt_register(sdioh_info_t *sdioh, sdioh_cb_fn_t fn, void *argh)
{
	SDIOH_TRACE(("%s: Enter", __FUNCTION__));
	sdioh->intr_handler = fn;
	sdioh->intr_handler_arg = argh;
	sdioh->intr_handler_valid = TRUE;

	return SDIOH_API_RC_SUCCESS;
}

static void
sdioh_intr_dummy(void *arg)
{
	return;
}

extern SDIOH_API_RC
sdioh_interrupt_deregister(sdioh_info_t *sdioh)
{
	SDIOH_TRACE(("%s: Enter", __FUNCTION__));
	sdioh->intr_handler_valid = FALSE;
	sdioh->intr_handler = sdioh_intr_dummy;
	sdioh->intr_handler_arg = NULL;

	return SDIOH_API_RC_SUCCESS;
}

extern SDIOH_API_RC
sdioh_interrupt_query(sdioh_info_t *sdioh, bool *onoff)
{
	*onoff = sdioh->interrupt_enabled;
	return SDIOH_API_RC_SUCCESS;
}

extern SDIOH_API_RC
sdioh_interrupt_set(sdioh_info_t *sdioh, bool en_dis)
{
	uint8 data = 0;
	uint8 tmp = 0;
	uint32 check;

	SDIOH_TRACE(("%s: %sable", __FUNCTION__, (en_dis ? "en" : "dis")));

	if (en_dis) {
		data = INTR_CTL_FUNC1_EN | INTR_CTL_MASTER_EN;
		sdioh->interrupt_enabled = TRUE;
		sdioh->sdintmask |= INT_DEV_INT;
		sdioh_reg_write(sdioh, OFFSETOF(sdioh_regs_t, intmask),
		                sdioh->sdintmask);
	} else {
		data = 0;
		sdioh->sdintmask &= ~INT_DEV_INT;
		sdioh_reg_write(sdioh, OFFSETOF(sdioh_regs_t, intmask),
		                sdioh->sdintmask);
		sdioh->interrupt_enabled = FALSE;
	}

	check = sdioh_reg_read(sdioh, OFFSETOF(sdioh_regs_t, intmask));
	if (sdioh->sdintmask != check)
		SDIOH_ERROR(("%s: wrote intmask 0x%08x, read 0x%08x\n",
		             __FUNCTION__, sdioh->sdintmask, check));

	return sdio_cmd52(sdioh, SD_IO_OP_WRITE, SDIO_FUNC_0, SD_IO_RW_NORMAL,
	                  OFFSETOF(sdio_regs_t, intr_ctl), data, &tmp);
}

#ifdef BCMDBG
extern bool
sdioh_interrupt_pending(sdioh_info_t *sdioh)
{
	uint32 intstatus;
	intstatus = sdioh_reg_read(sdioh, OFFSETOF(sdioh_regs_t, intstatus));
	return !!(intstatus & INT_DEV_INT);
}
#endif // endif

extern SDIOH_API_RC
sdioh_config_device(sdioh_info_t *sdioh)
{
	return SDIOH_API_RC_SUCCESS;
}

extern SDIOH_API_RC
sdioh_cfg_read(sdioh_info_t *sdioh, uint fnc_num, uint32 addr, uint8 *data)
{
	ASSERT(fnc_num <= sdioh->num_io_func);
	return sdioh_request_byte(sdioh, SDIOH_READ, fnc_num, addr, data);
}

extern SDIOH_API_RC
sdioh_cfg_write(sdioh_info_t *sdioh, uint fnc_num, uint32 addr, uint8 *data)
{
	ASSERT(fnc_num <= sdioh->num_io_func);

	if ((fnc_num == 0) && (addr >= SBSDIO_CIS_BASE_COMMON))
	{
		SDIOH_DEBUG(("SDIOH - sdioh_cfg_write: Function 0 CIS region is"
			     " write-protected.\n"));
		return SDIOH_API_RC_FAIL;
	}

	return sdioh_request_byte(sdioh, SDIOH_WRITE, fnc_num, addr, data);
}

extern SDIOH_API_RC
sdioh_cis_read(sdioh_info_t *sdioh, uint cisfuc, uint8 *cis, uint32 length)
{
	uint32 addr, i;

	ASSERT(cisfuc <= SDIOD_MAX_IOFUNCS);
	addr = sdioh->fn_cis[cisfuc];

	if ((cisfuc > sdioh->num_io_func) || !addr) {
		SDIOH_ERROR(("%s: bad fn or cis ptr: fn %d, num_io %d, cis 0x%08x\n",
		             __FUNCTION__, cisfuc, sdioh->num_io_func, addr));
		bzero(cis, length);
		return SDIOH_API_RC_FAIL;
	}

	/* all function xx cis are in FUNCTION 0 */
	/* use pio cmd52 byte read for simplicity */
	for (i = 0; i < length; i++) {
		if (SDIOH_CMDPASS != sdio_cmd52(sdioh, SD_IO_OP_READ, SDIO_FUNC_0, SD_IO_RW_NORMAL,
		                           addr + i, 0, (cis+i))) {
			SDIOH_ERROR(("cis read failed\n"));
			return SDIOH_API_RC_FAIL;
		}
	}

	return SDIOH_API_RC_SUCCESS;
}

extern SDIOH_API_RC
sdioh_request_byte(sdioh_info_t *sdioh, uint rw, uint fnc_num, uint addr, uint8 *byte)
{
	uint8 tmp = 0;
	uint8 data = 0;
	ASSERT(fnc_num <= sdioh->num_io_func);

	if (rw == SDIOH_WRITE)
		data = *byte;

	sdio_cmd52(sdioh, rw, fnc_num, SD_IO_RW_NORMAL, addr, data, &tmp);

	if (rw == SDIOH_READ)
		*byte = tmp;

	return SDIOH_API_RC_SUCCESS;
}

extern SDIOH_API_RC
sdioh_request_word(sdioh_info_t *sdioh, uint cmd_type, uint rw, uint fnc_num, uint addr,
                   uint32 *word, uint nbyte)
{
	uint rc;

	SDIOH_ASSERT(nbyte == 2 || nbyte == 4);

	SDIOH_ASSERT(fnc_num <= sdioh->num_io_func);
	if (rw == SDIOH_READ)
		rw = SD_IO_OP_READ;
	else if (rw == SDIOH_WRITE)
		rw = SD_IO_OP_WRITE;
	else
		SDIOH_ASSERT(0);

	rc = sdio_cmd53(sdioh, SDIOH_MODE_PIO, SD_IO_BYTE_MODE, rw, fnc_num, SD_IO_FIXED_ADDRESS,
	                addr, nbyte, (uint8 *)word);

	if (SDIOH_CMDPASS != rc) {
		SDIOH_ERROR(("access 0x%x fail!!\n", addr));
		return SDIOH_FAIL;
	}

	return SDIOH_API_RC_SUCCESS;
}

extern SDIOH_API_RC
sdioh_request_buffer(sdioh_info_t *sdioh, uint pio_dma, uint fix_inc, uint rw, uint fnc_num,
                     uint addr, uint reg_width, uint buflen, uint8 *buffer)
{
	/* check argument, convert to internal macros */
	ASSERT((pio_dma == SDIOH_DATA_PIO) || (pio_dma == SDIOH_DATA_DMA));
	pio_dma = (pio_dma == SDIOH_DATA_PIO) ? SDIOH_MODE_PIO : SDIOH_MODE_DMA;
	ASSERT((fix_inc == SDIOH_DATA_FIX) || (fix_inc == SDIOH_DATA_INC));
	fix_inc = (fix_inc == SDIOH_DATA_FIX) ? SD_IO_FIXED_ADDRESS : SD_IO_INCREMENT_ADDRESS;
	ASSERT((rw == SDIOH_READ) || (rw == SDIOH_WRITE));
	rw = (rw == SDIOH_READ) ? SD_IO_OP_READ : SD_IO_OP_WRITE;
	ASSERT(fnc_num <= sdioh->num_io_func);

	SDIOH_DATA(("sdioh_request_buffer, pio_dma %d fix_inc %d rw %d, func %d addr 0x%x,"
		    " reg_width %d, bytes %d, buffer 0x%x\n",
		     pio_dma, fix_inc, rw, fnc_num, addr, reg_width, buflen, (uint32)buffer));

	if (SDIOH_SUCCESS != sdioh_buf_pkt(sdioh, pio_dma, fix_inc, rw, fnc_num, addr,
	                                   (reg_width == 4), buflen, buffer)) {
		SDIOH_ERROR(("ERROR!!!!!sdioh_request_buffer failed!!!!\n"));
		return SDIOH_API_RC_FAIL;
	}

	return SDIOH_API_RC_SUCCESS;
}

/* ****************** SDIOH control **************************************** */

/* !! there is no checking for data buffer to be long enough, but
 * memory corruption will cause crash
 */
static int
sdioh_cmd_buf_pio_write(sdioh_info_t *sdioh, uint32 num_bytes, uint8 *data)
{
	uint32 data_val, ctl_val;
	int i = 0, j;

	if (num_bytes > SBSDIO_BYTEMODE_DATALEN_MAX) {
		SDIOH_ERROR(("sdioh_buf_pio_write: %d is too large for PIO byte mode\n",
		             num_bytes));
		return 1;
	}

	SDIOH_TRACE(("sdioh_buf_pio_write: PIO, num_bytes %d\n", num_bytes));

	/* PIO mode, push into fifo word by word */
	j = num_bytes % 4;

	if (num_bytes >= 4) {
		for (i = 0; i < num_bytes - j; i += 4) {
			data_val = *(uint32 *)(data + i);
			SDIOH_DATA(("p_w: 0x%x ", data_val));
			sdioh_reg_write(sdioh, OFFSETOF(sdioh_regs_t, fifodata), data_val);
		}
		SDIOH_DATA(("%d main bytes written\n", i));
	}

	if (j > 0) {
		switch (j) {
		case 1:
			data_val = *(uint8 *)(data + i);
			break;
		case 2:
			data_val = *(uint16 *)(data + i);
			break;
		case 3:
			data_val = (*(uint16 *)(data + i)) + ((*(uint8 *)(data + i + 2)) << 16);
			break;
		case 4:
		default:
			ASSERT(0);
			return 1;
		}
		/* !! use byte valid only when bus is not 32bits */
		ctl_val = FIFO_VALID_ALL;
		sdioh_reg_write(sdioh, OFFSETOF(sdioh_regs_t, fifoctl), ctl_val);

		SDIOH_DEBUG(("sdioh_buf_pio_write: %d tail bytes written\n", j));
		sdioh_reg_write(sdioh, OFFSETOF(sdioh_regs_t, fifodata), data_val);
	}

	return 0;
}

static int
sdioh_cmd_buf_pio_read(sdioh_info_t *sdioh, uint32 num_bytes, uint8 *data)
{
	uint32 data_val;
	int i = 0, j;

	if (num_bytes > SBSDIO_BYTEMODE_DATALEN_MAX) {
		SDIOH_ERROR(("sdioh_buf_pio_read: %d is too large for PIO byte mode\n", num_bytes));
		return 1;
	}

	SDIOH_TRACE(("sdioh_buf_pio_read: PIO, num_bytes %d\n", num_bytes));

	/* PIO mode, extract fifo word by word */
	j = num_bytes % 4;
	sdioh_reg_write(sdioh, OFFSETOF(sdioh_regs_t, fifoctl), FIFO_RCV_BUF_RDY | FIFO_VALID_ALL);

	if (num_bytes >= 4) {
		for (i = 0; i < num_bytes - j; i += 4) {
			data_val = sdioh_reg_read(sdioh, OFFSETOF(sdioh_regs_t, fifodata));
			SDIOH_DATA(("p_r: 0x%x ", data_val));
			*(uint32 *)(data + i) = data_val;
		}
		SDIOH_DATA(("%d main bytes read\n", i));
	}

	if (j > 0) {
		data_val = sdioh_reg_read(sdioh, OFFSETOF(sdioh_regs_t, fifodata));
		SDIOH_DATA(("sdioh_buf_pio_read: %d tail bytes out of 0x%x read\n", j, data_val));

		switch (j) {	/* little endian */
		case 1:
			*(uint8 *)(data + i) = data_val & 0xff;
			break;
		case 2:
			*(uint16 *)(data + i) = data_val & 0xffff;
			break;
		case 3:
			*(uint16 *)(data + i) = data_val & 0xffff;
			*(uint8 *)(data + i + 2) = (data_val >> 16) & 0xff;
			break;
		case 4:
		default:
			ASSERT(0);
			return 1;
		}

	}

	return 0;
}

static int
sdioh_cmd_wait_cmdatdone(sdioh_info_t *sdioh, uint32 waitlimit)
{
	bool res;
	sdioh_host_regbit_poll(sdioh, OFFSETOF(sdioh_regs_t, intstatus), INT_CMD_DAT_DONE, FALSE,
	                       waitlimit, &res);

	if (!res)
		SDIOH_ERROR(("polling FAILED\n"));

	return (res == TRUE) ? 0 : 1;
}

static int
sdioh_cmd_wait_cmdat_ct(sdioh_info_t *sdioh, uint32 waitlimit)
{
	bool res;
	sdioh_host_regbit_poll(sdioh, OFFSETOF(sdioh_regs_t, intstatus), INT_CMDBUSY_CUTTHRU, TRUE,
	                       waitlimit, &res);

	return (res == TRUE) ? 0 : 1;
}

static int
sdioh_cmd_build_and_send(sdioh_info_t *sdioh, uint cmd_index, sd_arg_t *arg, uint *sd_status,
                         uint *sd_error, uint8 send_method)
{
	uint32 sdioh_cmd_arg;
	uint8 pio_dma;

	/* build command argument and find response type */
	sdio_cmd_build(sdioh, cmd_index, arg, &sdioh_cmd_arg);

	/* ??? use PIO for now */
	pio_dma = SDIOH_MODE_PIO;

	if (cmd_index == SD_CMD_IO_RW_EXTENDED)
		sdioh_cmd_send(sdioh, (pio_dma == SDIOH_MODE_DMA), cmd_index, sdioh_cmd_arg,
		               sd_status, sd_error,
		               arg->cmd53.blknum_bytecnt, SDIOH_CMDTYPE_NORMAL,
		               arg->cmd53.arc_en, arg->cmd53.block_mode, FALSE, FALSE, FALSE);
	else
		sdioh_cmd_send(sdioh, (pio_dma == SDIOH_MODE_DMA), cmd_index, sdioh_cmd_arg,
		               sd_status, sd_error,
		               SDIOH_TOTCNT_NONE, SDIOH_CMDTYPE_NORMAL,
		               FALSE, FALSE, FALSE, FALSE, FALSE);

	return SDIOH_CMDPASS;
}

static int
sdioh_cmd_send(sdioh_info_t *sdioh, bool dma_on, uint cmd_index, uint32 cmd_content,
               uint32 *sh_status, uint32 *sh_error,
               uint32 c53_bn_bc, bool cmd_type, bool arc_en, bool blockmode, bool append,
               bool abort, bool notpolling)
{
	bool is_spi = (sdioh->sdio_mode == MODE_OP_SPI);
	uint8 data_rw = 0;
	uint8 data_en = 0;
	uint8 use_dma = 0;
	uint8 exp_busy = 0;
	uint8 no_chk_rspcrc = 0;
	uint8 no_chk_rsp_cmdidx = 0;
	uint8 rsp_type = 0;
	uint16 tot_len = 0;
	uint8 abortcmd = 0;
	uint8 appendcmd = 0;
	uint8 use_blk = (uint8)(blockmode == TRUE);
	uint32 cmd_dat = 0;	/* final composed value for cmdat register */
	uint16 cmd_data_oft = 0, cmd_low_oft = 0;

	if (!notpolling) {
		abortcmd = (uint8)(abort == TRUE);
		appendcmd = (uint8)(append == TRUE);

		no_chk_rspcrc = is_spi ? 1 : 0;

		switch (cmd_index) {
		case SD_CMD_GO_IDLE_STATE:
			rsp_type = is_spi ? SD_RSP_NO_1 : SD_RSP_NO_NONE;
			no_chk_rspcrc = 1;
			break;
		case SD_CMD_MMC_SET_RCA:
			rsp_type = SD_RSP_NO_6;
			break;
		case SD_CMD_IO_SEND_OP_COND:
			rsp_type = SD_RSP_NO_4;
			no_chk_rspcrc = 1;
			break;
		case SD_CMD_SELECT_DESELECT_CARD:
			rsp_type = SD_RSP_NO_1;
			exp_busy = 1;
			break;
		case SD_CMD_CRC_ON_OFF:
			rsp_type = SD_RSP_NO_1;	/* ?? */
			break;
		case SD_CMD_IO_RW_DIRECT:
			rsp_type = SD_RSP_NO_5;
			break;
		case SD_CMD_IO_RW_EXTENDED:
			rsp_type = SD_RSP_NO_5;
			data_rw = (cmd_content >> 31) & 0x1;
			data_en = 1;
			tot_len = c53_bn_bc & 0x1ffff;
			break;

		/* SD memory */
		case SD_CMD_SEND_CSD:
		case SD_CMD_SEND_CID:
			rsp_type = SD_RSP_NO_2;
			break;
		case SD_CMD_STOP_TRANSMISSION:
			rsp_type = SD_RSP_NO_1;
			exp_busy = 1;
			break;
		case SD_CMD_SEND_STATUS:
			rsp_type = SD_RSP_NO_1;
			break;
		case SD_CMD_SET_BLOCKLEN:
		case SD_CMD_READ_MULTIPLE_BLOCK:
			rsp_type = SD_RSP_NO_1;
			break;
		case SD_CMD_READ_SINGLE_BLOCK:
		case SD_CMD_WRITE_BLOCK:
			rsp_type = SD_RSP_NO_1;
			data_en = 1;
			data_rw = 1;
			tot_len = c53_bn_bc & 0x1ffff;
			break;
		case SD_CMD_WRITE_MULTIPLE_BLOCK:
		case SD_CMD_PROGRAM_CSD:
		case SD_CMD_SEND_WRITE_PROT:
		case SD_CMD_ERASE_WR_BLK_START:
		case SD_CMD_ERASE_WR_BLK_END:
		case SD_CMD_LOCK_UNLOCK:
		case SD_CMD_APP_CMD:
		case SD_CMD_GEN_CMD:
			rsp_type = SD_RSP_NO_1;
			break;
		case SD_CMD_SET_WRITE_PROT:
		case SD_CMD_CLR_WRITE_PROT:
		case SD_CMD_ERASE:
			rsp_type = SD_RSP_NO_1;
			exp_busy = 1;
			break;
		case SD_CMD_READ_OCR:
			rsp_type = SD_RSP_NO_3;
			break;

		/* ACMD */
		/* SD_ACMD_SD_STATUS dup with above */
		case SD_ACMD_SEND_NUM_WR_BLOCKS:
		case SD_ACMD_SET_WR_BLOCK_ERASE_CNT:
		/* SD_ACMD_SET_CLR_CARD_DETECT dup with above */
		case SD_ACMD_SEND_SCR:
			rsp_type = SD_RSP_NO_1;
			break;
		case SD_ACMD_SD_SEND_OP_COND:
			rsp_type = SD_RSP_NO_3;
			break;

		/* MMC command */
		case SD_CMD_SEND_OPCOND:
			rsp_type = SD_RSP_NO_1;	/* ?? */
			break;

		default:
			SDIOH_ERROR(("wrong cmd_index %d\n", cmd_index));
			return 1;
		}

		if (cmd_type == SDIOH_CMDTYPE_NORMAL) {
			if (dma_on)
				use_dma = 1;
		} else
			use_dma = 0;

		if (is_spi) {
			switch (rsp_type) {
			case SD_RSP_NO_NONE: no_chk_rsp_cmdidx = 1; break;
			case SD_RSP_NO_1: no_chk_rsp_cmdidx = 1; break;
			case SD_RSP_NO_2: no_chk_rsp_cmdidx = 0; break;
			case SD_RSP_NO_3: no_chk_rsp_cmdidx = 0; break;
			case SD_RSP_NO_4: no_chk_rsp_cmdidx = 1; break;
			case SD_RSP_NO_5: no_chk_rsp_cmdidx = 1; break;
			case SD_RSP_NO_6: no_chk_rsp_cmdidx = 0; break;
			default:
				SDIOH_ERROR(("wrong rsp_type %d\n", rsp_type));
				return 1;
			}
		} else {
			switch (rsp_type) {
			case SD_RSP_NO_NONE: no_chk_rsp_cmdidx = 1; break;
			case SD_RSP_NO_1: no_chk_rsp_cmdidx = 0; break;
			case SD_RSP_NO_2: no_chk_rsp_cmdidx = 0; break;
			case SD_RSP_NO_3: no_chk_rsp_cmdidx = 0; break;
			case SD_RSP_NO_4: no_chk_rsp_cmdidx = 1; break;
			case SD_RSP_NO_5: no_chk_rsp_cmdidx = 0; break;
			case SD_RSP_NO_6: no_chk_rsp_cmdidx = 0; break;
			default:
				SDIOH_ERROR(("wrong rsp_type %d\n", rsp_type));
				return 1;
			}
		}

		SDIOH_DEBUG(("cmdxx: cmd_index %d rsp_type %d data_en %d data_rw %d use_dma %d"
			     " arc_en %d exp_busy %d no_chk_rspcrc %d no_chk_rsp_cmdidx %d tot_len"
			     " %d use_blk %d\n",
			     cmd_index, rsp_type, data_en, data_rw, use_dma, arc_en, exp_busy,
			     no_chk_rspcrc, no_chk_rsp_cmdidx, tot_len, use_blk));

		cmd_dat =   (cmd_index & CMDAT_INDEX_M)
			+ ((rsp_type << CMDAT_EXP_RSPTYPE_O) & CMDAT_EXP_RSPTYPE_M)
			+ ((data_en <<  CMDAT_DAT_EN_O) & CMDAT_DAT_EN_M)
			+ ((data_rw << CMDAT_DAT_WR_O) & CMDAT_DAT_WR_M)
			+ ((use_dma << CMDAT_DMA_MODE_O) & CMDAT_DMA_MODE_M)
			+ ((arc_en << CMDAT_ARC_EN_O) & CMDAT_ARC_EN_M)
			+ ((exp_busy << CMDAT_EXP_BUSY_O) & CMDAT_EXP_BUSY_M)
			+ ((no_chk_rspcrc << CMDAT_NO_RSP_CRC_CHK_O) & CMDAT_NO_RSP_CRC_CHK_M)
			+ ((no_chk_rsp_cmdidx << CMDAT_NO_RSP_CDX_CHK_O) & CMDAT_NO_RSP_CDX_CHK_M)
			+ ((tot_len << CMDAT_DAT_TX_CNT_O) & CMDAT_DAT_TX_CNT_M)
			+ ((appendcmd << CMDAT_APPEND_EN_O) & CMDAT_APPEND_EN_M)
			+ ((abortcmd << CMDAT_ABORT_O) & CMDAT_ABORT_M)
			+ ((use_blk << CMDAT_BLK_EN_O) & CMDAT_BLK_EN_M);

		if (cmd_type == SDIOH_CMDTYPE_NORMAL) {
			cmd_data_oft = OFFSETOF(sdioh_regs_t, cmddat);
			cmd_low_oft  = OFFSETOF(sdioh_regs_t, cmdl);
		} else if (cmd_type == SDIOH_CMDTYPE_APPEND) {
			cmd_data_oft = OFFSETOF(sdioh_regs_t, ct_cmddat);
			cmd_low_oft  = OFFSETOF(sdioh_regs_t, ct_cmdl);
		} else if (cmd_type == SDIOH_CMDTYPE_CUTTHRU) {
			cmd_data_oft = OFFSETOF(sdioh_regs_t, ap_cmddat);
			cmd_low_oft  = OFFSETOF(sdioh_regs_t, ap_cmdl);
		} else {
			SDIOH_ERROR(("wrong cmd_type %d\n", cmd_type));
		}

		SDIOH_DEBUG(("cmdxx: issue cmd cmd_dat 0x%x, cmd_content 0x%x\n", cmd_dat,
		             cmd_content));
		sdioh_reg_write(sdioh, cmd_data_oft, cmd_dat);
		sdioh_reg_write(sdioh, cmd_low_oft, cmd_content);

		if (cmd_type == SDIOH_CMDTYPE_NORMAL) {
			if ((data_rw == 1) && (dma_on) && (sdioh->dma_start_mode ==
			                                   SDIOH_DMA_START_LATE)) {
				sdioh_host_codec_dma_start(sdioh, SDIOH_DMA_TX);
			}
		}
	} else { /* notpolling == TRUE */

	}

	if (cmd_type == SDIOH_CMDTYPE_NORMAL) {
		if (notpolling == FALSE) {
			*sh_status = sdioh_cmd_wait_cmdatdone(sdioh, sdioh->rdto);
			sdioh_reg_write(sdioh, OFFSETOF(sdioh_regs_t, intstatus),
			                (INT_CMD_DAT_DONE|INT_HOST_BUSY|
			                 INT_ERROR_SUM|INT_CMDBUSY_APPEND));

			*sh_error = sdioh_reg_read(sdioh, OFFSETOF(sdioh_regs_t, error));
			sdioh_reg_write(sdioh, OFFSETOF(sdioh_regs_t, error), 0xffffffff);
		}
	}

	if (cmd_type == SDIOH_CMDTYPE_CUTTHRU) {
		if (notpolling == FALSE) {
			*sh_status = sdioh_cmd_wait_cmdat_ct(sdioh, sdioh->rdto);
			sdioh_reg_write(sdioh, OFFSETOF(sdioh_regs_t, intstatus),
			                INT_CMDBUSY_CUTTHRU);

			*sh_error = sdioh_reg_read(sdioh, OFFSETOF(sdioh_regs_t, error));
			sdioh_reg_write(sdioh, OFFSETOF(sdioh_regs_t, error), 0xffffffff);
		}
	}

	if (cmd_type == SDIOH_CMDTYPE_APPEND) {
		/* do nothing */
	}

	return SDIOH_CMDPASS;
}

static int
sdioh_cmd_parse_and_check(sdioh_info_t *sdioh, uint32 cmd_index, sd_resp_t *resp)
{
	uint32 rsp_mid, rsp_hl;

	/* response queue is 32bits by 5 */
	rsp_mid  = sdioh_reg_read(sdioh, OFFSETOF(sdioh_regs_t, respq));
	rsp_hl   = sdioh_reg_read(sdioh, OFFSETOF(sdioh_regs_t, respq));

	sdio_cmd_check(sdioh, cmd_index, rsp_mid, rsp_hl, resp);
	return SDIOH_CMDPASS;
}

/* return SDIOH_CMDPASS on success */
static int
sdio_cmd_check(sdioh_info_t *sdioh, uint32 cmd_index, uint32 rsp1, uint32 rsp2, sd_resp_t *resp)
{
	uint result = SDIOH_CMDPASS;
	uint32 r1 		=  (rsp1 >> 24) & 0xff;
	uint32 lvcardrca 	=  (rsp1 >> 8) & 0xffff;
	uint32 Cbit		=  (rsp1 >> 23) & 0x1;
	uint32 NoOfIOFunc 	=  (rsp1 >> 20) & 0x7;
	uint32 MemoryPresent 	=  (rsp1 >> 19) & 0x1;
	uint32 IOOCR 		=  ((rsp1 & 0xffff) << 8) | ((rsp2 >> 24) & 0xff);
	uint32 hi6rsvd1		=  (rsp1 >> 24) & 0x3f;
	uint32 lo7rsvd1		=  (rsp2 >> 17) & 0x7f;
	uint32 cardstatus 	=  (((rsp1 >> 0) & 0xffffff) << 8) | ((rsp2 >> 24) & 0xff);
	uint32 spiReadWriteData	=  (rsp1 >> 16) & 0xff;
	uint32 stuffbits0 	=  (rsp1 >> 8) & 0xffff;
	uint32 readwritedata    =  (sdioh->sdio_mode == MODE_OP_SPI) ?
		spiReadWriteData : ((rsp2 >> 24) & 0xff);
#ifdef NOTUSED
	uint32 modifiedr1 	=  (rsp1 >> 24) & 0xff;
	uint32 r6cardstatus	=  (((rsp1 >> 0) & 0xff) << 8) | ((rsp2 >> 24) & 0xff);
#endif /* NOTUSED */
	uint32 responseflags	=  (rsp1 >> 0) & 0xff;
	SDIOH_TRACE(("sdio_cmd_check\n"));

	if (1) {
		/* build error value and return failure if response timeout */
	}

	if (sdioh->sdio_mode == MODE_OP_SPI) {

	} else {
		resp->r1 = r1;
		resp->cardstatus = cardstatus;

		if (cmd_index == SD_CMD_IO_SEND_OP_COND) {
			if (hi6rsvd1 != 0x3f) {
				SDIOH_ERROR(("High 6 Reserved bits should be all one's, not 0x%x\n",
				             hi6rsvd1));
				result = SDIOH_CMDERR_INVALID_RESPONSE;
			}
			if (lo7rsvd1 != 0x7f) {
				SDIOH_ERROR(("Low 7 Reserved bits should be all one's, not 0x%x\n",
				             lo7rsvd1));
				result = SDIOH_CMDERR_INVALID_RESPONSE;
			}
			resp->data.r4.card_ready = Cbit;
			resp->data.r4.num_io_func = NoOfIOFunc;
			resp->data.r4.mem_present = MemoryPresent;
			resp->data.r4.io_ocr = IOOCR;
		}

		if (cmd_index == SD_CMD_MMC_SET_RCA) {
			resp->data.r3.rca = lvcardrca;
		}

		if (cmd_index == SD_CMD_SELECT_DESELECT_CARD) {
			resp->data.r7.rca = lvcardrca;
		}

		if (cmd_index == SD_CMD_IO_RW_DIRECT) {
			if (stuffbits0 != 0) {
				SDIOH_ERROR(("CMD52, Stuff bits should be all 0, not 0x%x\n",
				             stuffbits0));
				result = SDIOH_CMDERR_INVALID_RESPONSE;
			}

			resp->data.r5.data = readwritedata;
			resp->data.r5.flags = responseflags;
		}

		if (cmd_index == SD_CMD_IO_RW_EXTENDED) {
			if (stuffbits0 != 0) {
				SDIOH_ERROR(("CMD53, Stuff bits should be all 0, not 0x%x\n",
				             stuffbits0));
				result = SDIOH_CMDERR_INVALID_RESPONSE;
			}

			resp->data.r5.data = readwritedata;
			resp->data.r5.flags = responseflags;
		}
	}
	return result;
}

/* ****************** SDIO protocol **************************************** */

/* CMD3 (SEND_RELATIVE_ADDR) is in SDx mode only. No SPI mode

	 S  D  CommandIndex StuffBits CRC7 E"
	47 46  45:40        39:8      7:1  0"
	 1  1  6            32        7    1"
	The response to CMD3 is Modified R6, format as follows
	 S  D  CommandIndex  newrca  r6cardstatus  crc7  E"
	47 46  45:40         39:24   23:8          7:1   0"
	 1  1      6         16      16            7     1"
	r6cardstatus:
	Bit 15: ComCRCError"
	Bit 14: IllegalCommand"
	Bit 13: Error"
	Bit 12-0: Reserved for Future Use (RFU)"
*/
static int
sdio_cmd3(sdioh_info_t *sdioh, uint16 *newrca)
{
	int res;
	uint sd_status, sd_error;
	sd_arg_t arg;
	sd_resp_t resp;

	SDIOH_TRACE(("cmd3\n"));

	/* build arguments */
	arg.cmd3.stuff = 0;

	/* send cmd and parse result */
	res = sdioh_cmd_build_and_send(sdioh, SD_CMD_MMC_SET_RCA, &arg, &sd_status, &sd_error,
	                               SDIOH_CMDTYPE_NORMAL);
	res = sdioh_cmd_parse_and_check(sdioh, SD_CMD_MMC_SET_RCA, &resp);

	*newrca = resp.data.r3.rca;
	return res;
}

/* CMD5 (IO_SEND_OP_COND)"
 *   valid command for both SPI and SD modes
 *	 S  D  CommandIndex StuffBits IOOCR CRC7 E"
 *	47 46  45:40        39:32     31:8  7:1  0"
 *	 1  1  6            32        24    7    1"
 *
 *   the response in spimode is R4
 *	modifiedr1                 cbit  noofiofunc  memorypresent  StuffBits  ioocr"
 *	47:40                      39    38:36       35             34:32      31:8 "
 *	 8                         1     3           1              3          24   "
 *	 modifiedr1:
 *	 Bit 7 6 5 4 3 2 1 0"
 *	     | | | | | | | |"
 *	     | | | | | | |  --> IdleState"
 *	     | | | | | |  ----> RFU, must be 0"
 *	     | | | | | -------> IllegalCommand"
 *	     | | | | ---------> ComCRCError"
 *	     | | | -----------> FunctionNumberError"
 *	     | | -------------> RFU, must be 0"
 *	     | ---------------> ParameterError"
 *	     -----------------> Sbit, must be 0"
 *   the response in sdxmode is R4
 *         S  D  Reserved(all one's) cbit  noofiofunc  memorypresent  StuffBits  ioocr  Reserved(all
 * one's)  E"
 *	47 46  45:40               39    38:36       35             34:32      31:8   7:1
 *      0"
 *	 1  1  6                   1     3           1              3          24     7
 *      1"
 */
static int
sdio_cmd5(sdioh_info_t *sdioh, uint32 ioocr, uint *newioocr, uint8 *num_io_func, bool *mem_present,
          bool *io_ready, bool *nr)
{
	int res;
	uint sd_status, sd_error;
	sd_arg_t arg;
	sd_resp_t resp;

	SDIOH_TRACE(("cmd5\n"));

	/* build arguments */
	arg.cmd5.io_ocr = ioocr;

	/* send cmd and parse result */
	res = sdioh_cmd_build_and_send(sdioh, SD_CMD_IO_SEND_OP_COND, &arg, &sd_status, &sd_error,
	                               SDIOH_CMDTYPE_NORMAL);
	res = sdioh_cmd_parse_and_check(sdioh, SD_CMD_IO_SEND_OP_COND, &resp);

	*nr = FALSE;

	if (sd_error & ERROR_DAT_READ_TO) {
		SDIOH_ERROR(("error: read timeout\n"));
		*nr = TRUE;
		res = SDIOH_CMDERR_READ_TIMEOUT;
	}

	*newioocr = resp.data.r4.io_ocr;
	*num_io_func = resp.data.r4.num_io_func;
	*mem_present = resp.data.r4.mem_present;
	*io_ready = resp.data.r4.card_ready;

	return SDIOH_SUCCESS;
}

/* CMD7 (SELECT/DE-SELECT_CARD), no SPI mode
	 S  D  CommandIndex RCA       Stuffbits CRC7 E
	47 46  45:40        39:24     23:8      7:1  0
	 1  1  6            16        16        7    1
	The response to CMD7 in SDx mode is R1b, format as follows
	 S  D  CommandIndex  cardstatus  crc7  E
	47 46  45:40         39:8        7:1   0
	 1  1      6         32          7     1
	cardstatus:
	 Bit 31 23 22 19 12:9 4
	     |   |  |  |    | |
	     |   |  |  |    | --> FunctionNumberError
	     |   |  |  |    ----> CURRENT_STATE must be 1111 for IO Only Card
	     |   |  |  ---------> Error
	     |   |  ------------> IllegalCommand
	     |   ---------------> ComCRCError
	     -------------------> OutOfRange
*/
static int
sdio_cmd7(sdioh_info_t *sdioh, uint16 rca)
{
	int res;
	uint sd_status, sd_error;
	sd_arg_t arg;
	sd_resp_t resp;

	SDIOH_TRACE(("cmd7\n"));

	/* build arguments */
	arg.cmd7.rca = rca;

	/* send cmd and parse result */
	res = sdioh_cmd_build_and_send(sdioh, SD_CMD_SELECT_DESELECT_CARD, &arg, &sd_status,
	                               &sd_error, SDIOH_CMDTYPE_NORMAL);
	res = sdioh_cmd_parse_and_check(sdioh, SD_CMD_SELECT_DESELECT_CARD, &resp);

	return SDIOH_SUCCESS;
}

/* CMD15, no SPI mode
	There is no response for cmd15 in SD mode
	But in SPI mode, we will still get response saying illegal command
*/
static int
sdio_cmd15(sdioh_info_t *sdioh, uint16 rca)
{
	int res;
	uint sd_status, sd_error;
	sd_arg_t arg;
	sd_resp_t resp;

	SDIOH_TRACE(("cmd15\n"));

/* something is very wrong, check card, skip inactive */
return SDIOH_CMDPASS;

	/* build arguments */
	arg.cmd7.rca = rca;

	/* send cmd and parse result */
	res = sdioh_cmd_build_and_send(sdioh, SD_CMD_GO_INACTIVE_STATE, &arg, &sd_status, &sd_error,
	                               SDIOH_CMDTYPE_NORMAL);
	res = sdioh_cmd_parse_and_check(sdioh, SD_CMD_GO_INACTIVE_STATE, &resp);

	return SDIOH_CMDPASS;
}

static int nesting = 0;

/* CMD52 (IO_RW_DIRECT)
 *	S D CommandIndex R/W FunctionNumber RAWFlag Stuff RegisterAddress Stuff WriteData/StuffBits
 *	CRC7 E"
 *	47 46  45:40        39  38:36          35      34    33:17           16    15:8
 * 7:1  0"
 *	1  1  6            1   3              1       1     17              1     8
 * 7    1"
 * spimode> The response to CMD52 in SPI mode is R5, format as follows"
 *  modifiedr1 + spireadwritedata"
 *  Bit 7 6 5 4 3 2 1 0 spireadwritedata"
 *      | | | | | | | |"
 *      | | | | | | |  --> IdleState"
 *      | | | | | |  ----> RFU, must be 0"
 *      | | | | | -------> IllegalCommand"
 *      | | | | ---------> ComCRCError"
 *      | | | -----------> FunctionNumberError"
 *      | | -------------> RFU, must be 0"
 *      | ---------------> ParameterError"
 *      -----------------> Sbit, must be 0"
 * sdiomode> The response to CMD52 in SDx mode is R5, format as follows"
 *	S  D  CommandIndex  stuff    responseflags readwritedata  crc7  E"
 *	47 46  45:40         39:24    23:16         15:8           7:1   0"
 *	1  1      6         16       8             8              7     1"
 *  responseflags:"
 *  Bit 7 6 5 4 3 2 1 0"
 *      | | | | | | | |"
 *      | | | | | | |  --> OutOfRange"
 *      | | | | | |  ----> FunctionNumberError"
 *      | | | | | -------> RFU"
 *      | | | | ---------> Error"
 *      | | | -----------> IOCurrentState0"
 *      | | -------------> IOCurrentState1"
 *      | ---------------> IllegalCommand"
 *      -----------------> ComCRCError"
 */
/* data is for writing, not used for read. rspdata is the response data */
static int
sdio_cmd52(sdioh_info_t *sdioh, uint rw, uint fnc, uint raw, uint32 addr, uint8 data,
           uint8 *rspdata)
{
	int res;
	uint sd_status, sd_error;
	sd_arg_t arg;
	sd_resp_t resp;

	SDIOH_TRACE(("sdio_cmd52\n"));
	if (nesting)
		printk("CMD52: nesting %d\n", nesting);
	nesting++;

	sdioh_reg_write(sdioh, OFFSETOF(sdioh_regs_t, intmask), 0);

	/* build arguments */
	arg.cmd52.rw_flag = rw;
	arg.cmd52.func_num = fnc;
	arg.cmd52.raw_flag = raw;
	arg.cmd52.data = (rw == SD_IO_OP_READ) ? 0 : data;
	arg.cmd52.address = addr;

	/* send cmd and parse result */
	res = sdioh_cmd_build_and_send(sdioh, SD_CMD_IO_RW_DIRECT, &arg, &sd_status, &sd_error,
	                               SDIOH_CMDTYPE_NORMAL);
	if (sd_status || sd_error) {
		SDIOH_ERROR(("%s: ERROR on f%d %s of addr 0x%08x, status %d err 0x%08x\n",
		             __FUNCTION__, fnc, ((rw == SD_IO_OP_READ) ? "read " : "write"),
		             addr, sd_status, sd_error));
		nesting--;
		if (nesting)
			printk("CMD52fail: nesting %d\n", nesting);
		return SDIOH_FAIL;
	}

	res = sdioh_cmd_parse_and_check(sdioh, SD_CMD_IO_RW_DIRECT, &resp);
	if (resp.data.r5.flags & SD_RSP_R5_ERRBITS) {
		SDIOH_ERROR(("%s: ERROR bits in R5, f%d %s of addr 0x%08x, r5 0x%02x\n",
		             __FUNCTION__, fnc, ((rw == SD_IO_OP_READ) ? "read " : "write"),
		             addr, resp.data.r5.flags));
		nesting--;
		if (nesting)
			printk("CMD52fail: nesting %d\n", nesting);
		return SDIOH_FAIL;
	}

	sdioh_reg_write(sdioh, OFFSETOF(sdioh_regs_t, intmask), sdioh->sdintmask);

	nesting--;
	if (nesting)
		printk("CMD52end: nesting %d\n", nesting);

	*rspdata = resp.data.r5.data;

	return SDIOH_SUCCESS;
}

/* valid for SPI and SD modes */
/* CMD53 (IO_RW_EXTENDED)
 * S  D  CommandIndex R/W FunctionNumber BLK  OP  RegisterAddress Blk/Bytecount  CRC7 E"
 * 47 46  45:40        39  38:36          35   34  33:17           16:8           7:1  0"
 * 1  1   6            1   3              1    1   17              9              7    1"
 * spimode, the response to CMD53 in SPI mode is R5, format as follows
 *	modifiedr1 + spireadwritedata"
 *	Bit 7 6 5 4 3 2 1 0 spireadwritedata"
 *	     | | | | | | | |"
 *	     | | | | | | |  --> IdleState"
 *	     | | | | | |  ----> RFU, must be 0"
 *	     | | | | | -------> IllegalCommand"
 *	     | | | | ---------> ComCRCError"
 *	     | | | -----------> FunctionNumberError"
 *	     | | -------------> RFU, must be 0"
 *	     | ---------------> ParameterError"
 *	     -----------------> Sbit, must be 0"
 *   sdio 1, 4 bit mode, the response to CMD53 in SDx mode is R5, format as follows
 *   S  D  CommandIndex  stuff    responseflags readwritedata  crc7  E"
 *   47 46  45:40        39:24    23:16         15:8           7:1   0"
 *   1  1      6         16       8             8              7     1"
 *	responseflags:
 *	Bit 7 6 5 4 3 2 1 0"
 *	     | | | | | | | |"
 *	     | | | | | | |  --> OutOfRange"
 *	     | | | | | |  ----> FunctionNumberError"
 *	     | | | | | -------> RFU"
 *	     | | | | ---------> Error"
 *	     | | | -----------> IOCurrentState0"
 *	     | | -------------> IOCurrentState1"
 *	     | ---------------> IllegalCommand"
 *	     -----------------> ComCRCError"
 *   CMD53 $rw => Middle 32-bits of CMD53 is $tmpcmdl
 */
/* cmd53 supports
 * PIO: byte mode
 * DMA: byte mode(repetitive), block mode(SD_IO_BLOCK_MODE)
 * argument should be native macros, not sdioh API external macros
 */

static int
sdio_cmd53(sdioh_info_t *sdioh, uint dma_pio, uint blkmode, uint rw,
      uint fnc, uint op, uint32 addr, uint bufferlen, uint8 *datastream)
{
	int res;
	uint sd_status, sd_error;
	sd_arg_t arg;
	sd_resp_t resp;

	bool arc_en;
	uint c53_blknum_bytecount, cmdcnt;
	uint16 fn_blk_size = sdioh->fn_blk_size[fnc];

	SDIOH_TRACE(("cmd53: dma_pio %d, blkmode %d, bufferlen %d addr 0x%x\n", dma_pio, blkmode,
	             bufferlen, addr));

	if (nesting)
		printk("CMD53: nesting %d\n", nesting);
	nesting++;

	sdioh_reg_write(sdioh, OFFSETOF(sdioh_regs_t, intmask), 0);

	if ((dma_pio == SDIOH_MODE_PIO) && (bufferlen > SBSDIO_BYTEMODE_DATALEN_MAX)) {
		SDIOH_ERROR(("PIO mode: count must be less or equal to 64\n"));
		nesting--;
		if (nesting)
			printk("CMD53fail: nesting %d\n", nesting);
		sdioh_reg_write(sdioh, OFFSETOF(sdioh_regs_t, intmask), sdioh->sdintmask);
		return SDIOH_CMDERR_INVALID_PARAMETER;
	}

	if (blkmode == SD_IO_BLOCK_MODE) {
		if ((bufferlen % fn_blk_size) != 0) {
			SDIOH_ERROR(("icmd53: blkmode, tot_bytes %d is not multiple of blk_size"
			             " %d\n", bufferlen, fn_blk_size));
			sdioh_reg_write(sdioh, OFFSETOF(sdioh_regs_t, intmask), sdioh->sdintmask);
			nesting--;
			if (nesting)
				printk("CMD53fail: nesting %d\n", nesting);
			return SDIOH_CMDERR_INVALID_PARAMETER;
		}
		if (!sdioh->card_blk_cap) {
			SDIOH_ERROR(("icmd53: device doesn't support blkmode\n"));
			sdioh_reg_write(sdioh, OFFSETOF(sdioh_regs_t, intmask), sdioh->sdintmask);
			nesting--;
			if (nesting)
				printk("CMD53fail: nesting %d\n", nesting);
			return SDIOH_CMDERR_INVALID_PARAMETER;
		}
		/* Update host blockmode to match this function */
		if (sdioh->cmd53_blk_size != fn_blk_size)
			sdioh_host_blksize_set(sdioh, fn_blk_size);
	}

	/* build arguments */
	arg.cmd53.rw_flag = rw;
	arg.cmd53.func_num = fnc;
	arg.cmd53.block_mode = blkmode;
	arg.cmd53.op_code = op;
	arg.cmd53.address = addr;

	c53_blknum_bytecount = (blkmode == SD_IO_BLOCK_MODE)
	        ? (bufferlen / fn_blk_size) : bufferlen;

	if (dma_pio == SDIOH_MODE_DMA) {
		arc_en = TRUE;
		cmdcnt = c53_blknum_bytecount;

		if (blkmode == SD_IO_BYTE_MODE)
			cmdcnt = 0;
	} else {
		cmdcnt = c53_blknum_bytecount;
		arc_en = FALSE;
	}
	arg.cmd53.blknum_bytecnt = cmdcnt;
	arg.cmd53.arc_en = arc_en;

	/* prepare data to sdioh FIFO */
	if (dma_pio == SDIOH_MODE_PIO) {
		if (rw == SD_IO_OP_WRITE) {
			if (sdioh_cmd_buf_pio_write(sdioh, bufferlen, datastream)) {
				SDIOH_ERROR(("cmd53: failed to write to PIO fifo\n"));
				sdioh_reg_write(sdioh, OFFSETOF(sdioh_regs_t, intmask),
				                sdioh->sdintmask);
				nesting--;
				if (nesting)
					printk("CMD53fail: nesting %d\n", nesting);
				return SDIOH_FAIL;
		}
		}
	} else {
		/* dma buffer ? */
		SDIOH_ERROR(("dma error\n"));
		ASSERT(0);
	}

	/* send cmd and parse result */
	res = sdioh_cmd_build_and_send(sdioh, SD_CMD_IO_RW_EXTENDED, &arg, &sd_status, &sd_error,
	                               SDIOH_CMDTYPE_NORMAL);
	if (sd_status || sd_error) {
		SDIOH_ERROR(("%s: ERROR on f%d %s of %d byte(s) at 0x%08x, status %d err 0x%08x\n",
		             __FUNCTION__, fnc, ((rw == SD_IO_OP_READ) ? "read " : "write"),
		             bufferlen, addr, sd_status, sd_error));
		nesting--;
		if (nesting)
			printk("CMD53fail: nesting %d\n", nesting);
		return SDIOH_FAIL;
	}

	res = sdioh_cmd_parse_and_check(sdioh, SD_CMD_IO_RW_EXTENDED, &resp);
	if (resp.data.r5.flags & SD_RSP_R5_ERRBITS) {
		SDIOH_ERROR(("%s: ERROR bits in R5, f%d %s of %d byte(s) at 0x%08x, r5 0x%02x\n",
		             __FUNCTION__, fnc, ((rw == SD_IO_OP_READ) ? "read " : "write"),
		             bufferlen, addr, resp.data.r5.flags));
		nesting--;
		if (nesting)
			printk("CMD53fail: nesting %d\n", nesting);
		return SDIOH_FAIL;
	}

	/* fetch data from sdioh FIFO */
	if (dma_pio == SDIOH_MODE_PIO) {
		if (rw == SD_IO_OP_READ) {
			if (sdioh_cmd_buf_pio_read(sdioh, bufferlen, datastream)) {
				SDIOH_ERROR(("cmd53: failed to read from PIO fifo\n"));
				sdioh_reg_write(sdioh, OFFSETOF(sdioh_regs_t, intmask),
				                sdioh->sdintmask);
				nesting--;
				if (nesting)
					printk("CMD53fail: nesting %d\n", nesting);
				return SDIOH_FAIL;
		}
		}
	} else {
		/* dma buffer ? */
		SDIOH_ERROR(("dma error\n"));
		ASSERT(0);
	}

	sdioh_reg_write(sdioh, OFFSETOF(sdioh_regs_t, intmask), sdioh->sdintmask);
	nesting--;
	if (nesting)
		printk("CMD53end: nesting %d\n", nesting);

	return SDIOH_SUCCESS;
}

/* SD spec: Build command mid 4 bytes */
static int
sdio_cmd_build(sdioh_info_t *sdioh, uint8 cmd, const sd_arg_t *arg, uint32 *builtcmd)
{
	uint32 cmd_body = 0;

	SDIOH_TRACE(("sdio_cmd_build\n"));

	SDIOH_ASSERT(arg);

	/* Build argument in command token */
	switch (cmd)
	{
	case SD_CMD_GO_IDLE_STATE:
		cmd_body = 0; /* TBD */
		break;

	case SD_CMD_MMC_SET_RCA:
		cmd_body = 0;
		break;

	case SD_CMD_IO_SEND_OP_COND:
		cmd_body = (arg->cmd5.io_ocr & SD_CMD5_ARG_IO_OCR_MASK) << SD_CMD5_ARG_IO_OCR_SHIFT;
		break;

	case SD_CMD_SELECT_DESELECT_CARD:
		cmd_body = arg->cmd7.rca << SD_CMD7_ARG_RCA_SHIFT;
		break;

	case SD_CMD_GO_INACTIVE_STATE:
		/* ?? */
		break;

	case SD_CMD_IO_RW_DIRECT:
		cmd_body = SDIO_IO_RW_DIRECT_ARG(arg->cmd52.rw_flag, arg->cmd52.raw_flag,
		                                 arg->cmd52.func_num, arg->cmd52.address,
		                                 arg->cmd52.data);
		break;

	case SD_CMD_IO_RW_EXTENDED:
		cmd_body = SDIO_IO_RW_EXTENDED_ARG(arg->cmd53.rw_flag, arg->cmd53.block_mode,
		                                   arg->cmd53.func_num, arg->cmd53.address,
		                                   arg->cmd53.op_code,
		                                   arg->cmd53.blknum_bytecnt);
		break;

	case SD_CMD_CRC_ON_OFF:
		cmd_body |= (arg->cmd59.crc_option & SD_CMD59_ARG_CRC_OPTION_MASK) <<
			SD_CMD59_ARG_CRC_OPTION_SHIFT;
		break;

	default:
		STATS_INVALID_PARAMETER();
		SDIOH_TRACE(("wrong cmd_index %d\n", cmd));
		return SDIOH_CMDERR_INVALID_PARAMETER;
	}

	*builtcmd = cmd_body;

	return SDIOH_SUCCESS;
}

/* ****************** Diagnostic ************************** */
#define DIAG_TEST_DATA_BUF_LEN	2000 /* Buffer length */

/* MAC and SHM need to be enabled if test SHM */
#ifdef NOTUSED
int
diag_sdio_sb_buf_loopback(sdioh_info_t *sdioh, bool inc, uint addr, uint tot_bytes)
{
	uint i = 0;
	uint func = SDIO_FUNC_1;
	bool reg_is4byte = TRUE;
	uint fix_inc;
	int res;

	SDIOH_ERROR(("cmd53 loopback test: addr 0x%x, tot_bytes %d, incremental %d\n", addr,
	             tot_bytes, inc));

	sdioh->data_buf_tx = (uint8 *)MALLOC(sdioh->osh, DIAG_TEST_DATA_BUF_LEN);
	sdioh->data_buf_rx = (uint8 *)MALLOC(sdioh->osh, DIAG_TEST_DATA_BUF_LEN);

	if ((sdioh->data_buf_tx == NULL) || (sdioh->data_buf_rx == NULL)) {
		SDIOH_ERROR(("test_sdio_sb_buf_loopback: malloc failed\n"));
	} else {
		ASSERT(ISALIGNED(sdioh->data_buf_tx, sizeof(uint32)));
		ASSERT(ISALIGNED(sdioh->data_buf_rx, sizeof(uint32)));
		for (i = 0; i < DIAG_TEST_DATA_BUF_LEN; i++) {
			*(sdioh->data_buf_tx + i) = i;
			*(sdioh->data_buf_rx + i) = 0;
		}
	}

	fix_inc = inc ? SD_IO_INCREMENT_ADDRESS : SD_IO_FIXED_ADDRESS;

	if (tot_bytes > SBSDIO_BYTEMODE_DATALEN_MAX) {
		if (SDIOH_SUCCESS != sdioh_buf_pkt(sdioh, SDIOH_MODE_PIO, fix_inc, SD_IO_OP_WRITE,
		                                   func, addr, reg_is4byte, tot_bytes,
		                                   sdioh->data_buf_tx))
			return 1;

		if (SDIOH_SUCCESS != sdioh_buf_pkt(sdioh, SDIOH_MODE_PIO, fix_inc, SD_IO_OP_READ,
		                                   func, addr, reg_is4byte, tot_bytes,
		                                   sdioh->data_buf_rx))
			return 1;
	} else {

		/* one cmd write/readback */
		res  = sdio_cmd53(sdioh, SDIOH_MODE_PIO,  SD_IO_BLOCK_MODE, SD_IO_OP_WRITE,
		             func, SD_IO_INCREMENT_ADDRESS, addr, tot_bytes, sdioh->data_buf_tx);
		if (SDIOH_CMDPASS != res) {
			SDIOH_ERROR(("!!!block tx to shm failed, reason %d !!!\n", res));
			return 1;
		}
		SDIOH_ERROR(("block tx done\n"));

		res = sdio_cmd53(sdioh, SDIOH_MODE_PIO,  SD_IO_BLOCK_MODE, SD_IO_OP_READ,
		            func, SD_IO_INCREMENT_ADDRESS, addr, tot_bytes, sdioh->data_buf_rx);
		if (SDIOH_CMDPASS != res) {
			SDIOH_ERROR(("!!!block rx from shm failed, reason %d !!!\n", res));
			return 1;
		}
		SDIOH_ERROR(("block rx done\n"));
	}

	/* compare */
	for (i = 0; i < tot_bytes; i++) {
		if (*(sdioh->data_buf_tx + i) != *(sdioh->data_buf_rx + i)) {
			SDIOH_ERROR(("!!!block transfer to 0x%x failed, at byte %d!!!\n", addr, i));
			return 1;
		}
	}

	/* free sample data */
	if (sdioh->data_buf_tx) MFREE(sdioh->osh, sdioh->data_buf_tx, DIAG_TEST_DATA_BUF_LEN);
	if (sdioh->data_buf_rx) MFREE(sdioh->osh, sdioh->data_buf_rx, DIAG_TEST_DATA_BUF_LEN);

	SDIOH_ERROR(("test_sdio_sb_buf_loopback completed successful\n"));
	return 0;
}
#endif /* NOTUSED */

static void
diag_dumpreg_sdioh(sdioh_info_t *sdioh)
{
	sdioh->sdioh_regs.mode = sdioh_reg_read(sdioh, OFFSETOF(sdioh_regs_t, mode));
	sdioh->sdioh_regs.delay = sdioh_reg_read(sdioh, OFFSETOF(sdioh_regs_t, delay));
	sdioh->sdioh_regs.rdto = sdioh_reg_read(sdioh, OFFSETOF(sdioh_regs_t, rdto));
	sdioh->sdioh_regs.rbto = sdioh_reg_read(sdioh, OFFSETOF(sdioh_regs_t, rbto));
	sdioh->sdioh_regs.test = sdioh_reg_read(sdioh, OFFSETOF(sdioh_regs_t, test));
	sdioh->sdioh_regs.arvm = sdioh_reg_read(sdioh, OFFSETOF(sdioh_regs_t, arvm));

	sdioh->sdioh_regs.error = sdioh_reg_read(sdioh, OFFSETOF(sdioh_regs_t, error));
	sdioh->sdioh_regs.errormask = sdioh_reg_read(sdioh, OFFSETOF(sdioh_regs_t, errormask));
	sdioh->sdioh_regs.cmddat = sdioh_reg_read(sdioh, OFFSETOF(sdioh_regs_t, cmddat));
	sdioh->sdioh_regs.cmdl = sdioh_reg_read(sdioh, OFFSETOF(sdioh_regs_t, cmdl));
	sdioh->sdioh_regs.fifodata = sdioh_reg_read(sdioh, OFFSETOF(sdioh_regs_t, fifodata));
	sdioh->sdioh_regs.respq = sdioh_reg_read(sdioh, OFFSETOF(sdioh_regs_t, respq));

	sdioh->sdioh_regs.ct_cmddat = sdioh_reg_read(sdioh, OFFSETOF(sdioh_regs_t, ct_cmddat));
	sdioh->sdioh_regs.ct_cmdl = sdioh_reg_read(sdioh, OFFSETOF(sdioh_regs_t, ct_cmdl));
	sdioh->sdioh_regs.ct_fifodata = sdioh_reg_read(sdioh, OFFSETOF(sdioh_regs_t, ct_fifodata));
	sdioh->sdioh_regs.ap_cmddat = sdioh_reg_read(sdioh, OFFSETOF(sdioh_regs_t, ap_cmddat));
	sdioh->sdioh_regs.ap_cmdl = sdioh_reg_read(sdioh, OFFSETOF(sdioh_regs_t, ap_cmdl));
	sdioh->sdioh_regs.ap_fifodata = sdioh_reg_read(sdioh, OFFSETOF(sdioh_regs_t, ap_fifodata));

	sdioh->sdioh_regs.intstatus = sdioh_reg_read(sdioh, OFFSETOF(sdioh_regs_t, intstatus));
	sdioh->sdioh_regs.intmask = sdioh_reg_read(sdioh, OFFSETOF(sdioh_regs_t, intmask));
	sdioh->sdioh_regs.debuginfo = sdioh_reg_read(sdioh, OFFSETOF(sdioh_regs_t, debuginfo));
	sdioh->sdioh_regs.intmask = sdioh_reg_read(sdioh, OFFSETOF(sdioh_regs_t, intmask));
	sdioh->sdioh_regs.blocksize = sdioh_reg_read(sdioh, OFFSETOF(sdioh_regs_t, blocksize));

	SDIOH_INFO((" mode 0x%x\n delay 0x%x\n rdto 0x%x\n rbto 0x%x\n\n test 0x%x\n arvm 0x%x\n"
		    " cmddat 0x%x\n cmdl 0x%x\n\n error 0x%x\n errormask 0x%x\n intstatus 0x%x\n"
		    " intmask 0x%x\n\n",
		    sdioh->sdioh_regs.mode, sdioh->sdioh_regs.delay, sdioh->sdioh_regs.rdto,
		    sdioh->sdioh_regs.rbto,
		    sdioh->sdioh_regs.test, sdioh->sdioh_regs.arvm, sdioh->sdioh_regs.cmddat,
		    sdioh->sdioh_regs.cmdl,
		    sdioh->sdioh_regs.error, sdioh->sdioh_regs.errormask,
		    sdioh->sdioh_regs.intstatus, sdioh->sdioh_regs.intmask));

	SDIOH_INFO((" respq 0x%x\n debuginfo 0x%x\n fifoctl 0x%x\n fifodata 0x%x\n blocksize"
		    " 0x%x\n\n ct_cmddat 0x%x\n ct_cmdl 0x%x\n ct_fifodata 0x%x\n ap_cmddat 0x%x\n"
		    " ap_cmddl 0x%x\n ap_fifodata 0x%x\n \n",
		    sdioh->sdioh_regs.respq, sdioh->sdioh_regs.debuginfo, sdioh->sdioh_regs.fifoctl,
		    sdioh->sdioh_regs.fifodata, sdioh->sdioh_regs.blocksize,
		    sdioh->sdioh_regs.ct_cmddat, sdioh->sdioh_regs.ct_cmdl,
		    sdioh->sdioh_regs.ct_fifodata,
		    sdioh->sdioh_regs.ap_cmddat, sdioh->sdioh_regs.ap_cmdl,
		    sdioh->sdioh_regs.ap_fifodata));
}

static void
diag_dumpreg_sdio(sdioh_info_t *sdioh)
{
	sdio_cmd52(sdioh, SD_IO_OP_READ, SDIO_FUNC_0, SD_IO_RW_NORMAL,
	           OFFSETOF(sdio_regs_t, cccr_sdio_rev), 0,
	           (uint8 *)&sdioh->sdio_regs.cccr_sdio_rev);
	sdio_cmd52(sdioh, SD_IO_OP_READ, SDIO_FUNC_0, SD_IO_RW_NORMAL,
	           OFFSETOF(sdio_regs_t, sd_rev), 0,
	           (uint8 *)&sdioh->sdio_regs.sd_rev);
	sdio_cmd52(sdioh, SD_IO_OP_READ, SDIO_FUNC_0, SD_IO_RW_NORMAL,
	           OFFSETOF(sdio_regs_t, io_en), 0,
	           (uint8 *)&sdioh->sdio_regs.io_en);
	sdio_cmd52(sdioh, SD_IO_OP_READ, SDIO_FUNC_0, SD_IO_RW_NORMAL,
	           OFFSETOF(sdio_regs_t, io_rdy), 0, (uint8 *)&sdioh->sdio_regs.io_rdy);
	sdio_cmd52(sdioh, SD_IO_OP_READ, SDIO_FUNC_0, SD_IO_RW_NORMAL,
	           OFFSETOF(sdio_regs_t, intr_ctl), 0, (uint8 *)&sdioh->sdio_regs.intr_ctl);
	sdio_cmd52(sdioh, SD_IO_OP_READ, SDIO_FUNC_0, SD_IO_RW_NORMAL,
	           OFFSETOF(sdio_regs_t, intr_status), 0, (uint8 *)&sdioh->sdio_regs.intr_status);
	sdio_cmd52(sdioh, SD_IO_OP_READ, SDIO_FUNC_0, SD_IO_RW_NORMAL,
	           OFFSETOF(sdio_regs_t, io_abort), 0, (uint8 *)&sdioh->sdio_regs.io_abort);
	sdio_cmd52(sdioh, SD_IO_OP_READ, SDIO_FUNC_0, SD_IO_RW_NORMAL,
	           OFFSETOF(sdio_regs_t, bus_inter), 0, (uint8 *)&sdioh->sdio_regs.bus_inter);
	sdio_cmd52(sdioh, SD_IO_OP_READ, SDIO_FUNC_0, SD_IO_RW_NORMAL,
	           OFFSETOF(sdio_regs_t, capability), 0, (uint8 *)&sdioh->sdio_regs.capability);
	sdio_cmd52(sdioh, SD_IO_OP_READ, SDIO_FUNC_0, SD_IO_RW_NORMAL,
	           OFFSETOF(sdio_regs_t, cis_base_low), 0, (uint8 *)&sdioh->sdio_regs.cis_base_low);
	sdio_cmd52(sdioh, SD_IO_OP_READ, SDIO_FUNC_0, SD_IO_RW_NORMAL,
	           OFFSETOF(sdio_regs_t, cis_base_mid), 0, (uint8 *)&sdioh->sdio_regs.cis_base_mid);
	sdio_cmd52(sdioh, SD_IO_OP_READ, SDIO_FUNC_0, SD_IO_RW_NORMAL,
	           OFFSETOF(sdio_regs_t, cis_base_high), 0,
	           (uint8 *)&sdioh->sdio_regs.cis_base_high);

		SDIOH_INFO((" cccr_sdio_rev 0x%x\n sd_rev 0x%x\n io_en 0x%x\n io_rdy 0x%x\n"
			    " intr_ctl 0x%x\n intr_status 0x%x\n io_abort 0x%x\n bus_inter 0x%x\n"
			    " capability 0x%x\n cis_base_low 0x%x\n cis_base_mid 0x%x\n"
			    " cis_base_high 0x%x\n",
			    sdioh->sdio_regs.cccr_sdio_rev, sdioh->sdio_regs.sd_rev,
			    sdioh->sdio_regs.io_en, sdioh->sdio_regs.io_rdy,
			    sdioh->sdio_regs.intr_ctl, sdioh->sdio_regs.intr_status,
			    sdioh->sdio_regs.io_abort, sdioh->sdio_regs.bus_inter,
			    sdioh->sdio_regs.capability, sdioh->sdio_regs.cis_base_low,
			    sdioh->sdio_regs.cis_base_mid, sdioh->sdio_regs.cis_base_high));

	if ((sdioh->sdio_regs.io_en & SDIO_FUNC_ENABLE_1) == 0)
		SDIOH_ERROR(("!!!!! access sdio core failed !!!!!\n"));
}

extern SDIOH_API_RC
sdioh_test_diag(sdioh_info_t *sdioh)
{
	ASSERT(sdioh);

	SDIOH_ERROR(("SDIOH test "));

	diag_dumpreg_sdioh(sdioh);
	diag_dumpreg_sdio(sdioh);

	/* diag_sdio_sb_buf_loopback(sdioh, TRUE, 0x800, 100); */
	SDIOH_ERROR(("SDIOH test done\n"));
	return (0);
}

/* IOVar table */
enum {
	IOV_MSGLEVEL = 1,
	IOV_BLOCKMODE,
	IOV_BLOCKSIZE,
	IOV_DMA,
	IOV_USEINTS,
	IOV_NUMINTS,
	IOV_NUMLOCALINTS,
	IOV_HOSTREG,
	IOV_CLIENTREG
};

const bcm_iovar_t sdioh_iovars[] = {
	{"sd_msglevel",	IOV_MSGLEVEL,	0,	0, IOVT_UINT32,	0 },
#if NOTYET
	{"sd_blockmode", IOV_BLOCKMODE,	0,	0, IOVT_BOOL,	0 },
	{"sd_blocksize", IOV_BLOCKSIZE, 0,	0, IOVT_UINT32,	0 }, /* ((fn << 16) | size) */
	{"sd_dma",	IOV_DMA,	0,	0, IOVT_BOOL,	0 },
	{"sd_ints",	IOV_USEINTS,	0,	0, IOVT_BOOL,	0 },
	{"sd_numints",	IOV_NUMINTS,	0,	0, IOVT_UINT32,	0 },
	{"sd_numlocalints", IOV_NUMLOCALINTS, 0, 0, IOVT_UINT32,	0 },
#endif // endif
	{"sd_hostreg",	IOV_HOSTREG,	0,	0, IOVT_BUFFER,	sizeof(sdreg_t) },
	{"sd_clientreg", IOV_CLIENTREG, 0,	0, IOVT_BUFFER,	sizeof(sdreg_t)	},
	{NULL, 0, 0, 0, 0, 0 }
};

int
sdioh_iovar_op(sdioh_info_t *si, const char *name,
               void *params, int plen, void *arg, int len, bool set)
{
	const bcm_iovar_t *vi = NULL;
	int bcmerror = 0;
	int val_size;
	int32 int_val = 0;
	bool bool_val;
	uint32 actionid;

	ASSERT(name);
	ASSERT(len >= 0);

	/* Get must have return space; Set does not take qualifiers */
	ASSERT(set || (arg && len));
	ASSERT(!set || (!params && !plen));

	SDIOH_TRACE(("%s: Enter (%s %s)\n", __FUNCTION__, (set ? "set" : "get"), name));

	if ((vi = bcm_iovar_lookup(sdioh_iovars, name)) == NULL) {
		bcmerror = BCME_UNSUPPORTED;
		goto exit;
	}

	if ((bcmerror = bcm_iovar_lencheck(vi, arg, len, set)) != 0)
		goto exit;

	/* Set up params so get and set can share the convenience variables */
	if (params == NULL) {
		params = arg;
		plen = len;
	}

	if (vi->type == IOVT_VOID)
		val_size = 0;
	else if (vi->type == IOVT_BUFFER)
		val_size = len;
	else
		val_size = sizeof(int);

	if (plen >= (int)sizeof(int_val))
		bcopy(params, &int_val, sizeof(int_val));

	bool_val = (int_val != 0) ? TRUE : FALSE;

	actionid = set ? IOV_SVAL(vi->varid) : IOV_GVAL(vi->varid);
	switch (actionid) {
	case IOV_GVAL(IOV_MSGLEVEL):
		int_val = (int32)sdioh_msglevel;
		bcopy(&int_val, arg, val_size);
		break;

	case IOV_SVAL(IOV_MSGLEVEL):
		sdioh_msglevel = int_val;
		break;

#if NOTYET
	/* Placeholder code from bcmsdstd; not yet supported by bcmsdioh */
	case IOV_GVAL(IOV_BLOCKMODE):
		int_val = (int32)si->sd_blockmode;
		bcopy(&int_val, arg, val_size);
		break;

	case IOV_SVAL(IOV_BLOCKMODE):
		si->sd_blockmode = int_val;
		/* Haven't figured out how to make non-block mode with DMA */
		if (!si->sd_blockmode)
			si->sd_use_dma = 0;
		break;

	case IOV_GVAL(IOV_BLOCKSIZE):
		if ((uint32)int_val > si->num_funcs) {
			bcmerror = BCME_BADARG;
			break;
		}
		int_val = (int32)si->client_block_size[int_val];
		bcopy(&int_val, arg, val_size);
		break;

	case IOV_SVAL(IOV_BLOCKSIZE):
	{
		uint func = ((uint32)int_val >> 16);
		uint blksize = (uint16)int_val;
		uint maxsize;

		if (func > si->num_funcs) {
			bcmerror = BCME_BADARG;
			break;
		}

		switch (func) {
		case 0: maxsize = 32; break;
		case 1: maxsize = BLOCK_SIZE_4318; break;
		case 2: maxsize = BLOCK_SIZE_4328; break;
		default: maxsize = 0;
		}
		if (blksize > maxsize) {
			bcmerror = BCME_BADARG;
			break;
		}
		if (!blksize) {
			blksize = maxsize;
		}

		/* Now set it */
		bcmerror = set_client_block_size(si, func, blksize);
		break;
	}

	case IOV_GVAL(IOV_DMA):
		int_val = (int32)si->sd_use_dma;
		bcopy(&int_val, arg, val_size);
		break;

	case IOV_SVAL(IOV_DMA):
		si->sd_use_dma = int_val;
		break;

	case IOV_GVAL(IOV_USEINTS):
		int_val = (int32)si->use_client_ints;
		bcopy(&int_val, arg, val_size);
		break;

	case IOV_SVAL(IOV_USEINTS):
		si->use_client_ints = int_val;
		if (si->use_client_ints)
			si->intmask |= CLIENT_INTR;
		else
			si->intmask &= ~CLIENT_INTR;
		break;

	case IOV_GVAL(IOV_NUMINTS):
		int_val = (int32)si->intrcount;
		bcopy(&int_val, arg, val_size);
		break;

	case IOV_GVAL(IOV_NUMLOCALINTS):
		int_val = (int32)si->local_intrcount;
		bcopy(&int_val, arg, val_size);
		break;
#endif /* NOTYET */

	case IOV_GVAL(IOV_HOSTREG):
	{
		sdreg_t *sd_ptr = (sdreg_t *)params;

		if ((sd_ptr->offset < 0) || (sd_ptr->offset & 3) ||
		    (sd_ptr->offset > OFFSETOF(sdioh_regs_t, dmaregs.rcv.status))) {
			SDIOH_ERROR(("%s: bad offset 0x%x\n", __FUNCTION__, sd_ptr->offset));
			bcmerror = BCME_BADARG;
			break;
		}

		int_val = sdioh_reg_read(si, sd_ptr->offset);
		bcopy(&int_val, arg, sizeof(int_val));
		break;
	}

	case IOV_SVAL(IOV_HOSTREG):
	{
		sdreg_t *sd_ptr = (sdreg_t *)params;

		if ((sd_ptr->offset < 0) || (sd_ptr->offset & 3) ||
		    (sd_ptr->offset > OFFSETOF(sdioh_regs_t, dmaregs.rcv.status))) {
			SDIOH_ERROR(("%s: bad offset 0x%x\n", __FUNCTION__, sd_ptr->offset));
			bcmerror = BCME_BADARG;
			break;
		}

		sdioh_reg_write(si, sd_ptr->offset, (uint32)sd_ptr->value);
		break;
	}

	case IOV_GVAL(IOV_CLIENTREG):
	{
		sdreg_t *sd_ptr = (sdreg_t *)params;
		uint8 data;

		if (sdioh_cfg_read(si, sd_ptr->func, sd_ptr->offset, &data)) {
			bcmerror = BCME_SDIO_ERROR;
			break;
		}

		int_val = (int)data;
		bcopy(&int_val, arg, sizeof(int_val));
		break;
	}

	case IOV_SVAL(IOV_CLIENTREG):
	{
		sdreg_t *sd_ptr = (sdreg_t *)params;
		uint8 data = sd_ptr->value;

		if (sdioh_cfg_write(si, sd_ptr->func, sd_ptr->offset, &data)) {
			bcmerror = BCME_SDIO_ERROR;
			break;
		}
		break;
	}

	default:
		bcmerror = BCME_UNSUPPORTED;
		break;
	}
exit:
	return bcmerror;
}
