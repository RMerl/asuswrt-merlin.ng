/***************************************************************************
    Copyright 2000-2016 Broadcom Corporation

    <:label-BRCM:2016:DUAL/GPL:standard
    
    Unless you and Broadcom execute a separate written software license
    agreement governing use of this software, this software is licensed
    to you under the terms of the GNU General Public License version 2
    (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
    with the following added to such license:
    
       As a special exception, the copyright holders of this software give
       you permission to link this software with independent modules, and
       to copy and distribute the resulting executable under terms of your
       choice, provided that you also meet, for each linked independent
       module, the terms and conditions of the license of that module.
       An independent module is a module which is not derived from this
       software.  The special exception does not apply to any modifications
       of the software.
    
    Not withstanding the above, under no circumstances may you combine
    this software in any way with any other Broadcom software provided
    under a license other than the GPL, without Broadcom's express prior
    written consent.
    
    :>
 ***************************************************************************/
#include "rom_main.h"
#include "rom_emmc_drv.h"
#include "bcm_otp.h"
#include "cfe_gpt_common.h"
#include "rom_parms.h"
#include "btrm_if.h"
#include "rom_parms.h"
#include "dev_bcm63xx_emmc_common.h"
#include "bcm63xx_util.h"
#include "bcm_bootimgsts.h"
#include "bcm63xx_sec.h"

/************************************************************
 *               EMMC I/O Related Functions                 *
 ************************************************************/

/* Defines and Types */
#define DEBUG_EMMC_FSBL 0   /* 1:enable, 0:disable */
#define DEBUG_EMMC_RDWR 0
#define DEBUG_EMMC_DATA 0
#define DEBUG_EMMC_BOOT 0

#define BDEV_WR(x, y)   (*(x) = y)
#define BDEV_RD(x)      (*(x))

#if DEBUG_EMMC_FSBL
#define DBG_MSG_PUTS(...) xprintf(__VA_ARGS__)
#define DBG_MSG_WR_INT(...) xprintf("__VA_ARGS__ %d\n", __VA_ARGS__)
#define DBG_MSG_WR_HEX(...) xprintf("__VA_ARGS__ 0x%08x\n", __VA_ARGS__)
#else
#define DBG_MSG_PUTS(...)
#define DBG_MSG_WR_INT(...)
#define DBG_MSG_WR_HEX(...)
#endif

#define EMMC_OK     0
#define EMMC_NG     1

/* HS_TIMING_OPTION */
#define HS_TIMING_FULL  0   /* 0 ~  26MHz, Backwards Compatibility */
#define HS_TIMING_HS    1   /* 0 ~  52MHz, High Speed Mode */
#define HS_TIMING_HS200 2   /* 0 ~ 200MHz, HS200 Mode */

/* BUS_FREQ_OPTION */
#define BUS_FREQ_52MHZ  1   /* 100MHz/2^1, Base clock = 100MHz */
#define BUS_FREQ_26MHZ  2
#define BUS_FREQ_13MHZ  4
#define BUS_FREQ_06MHZ  8
#define BUS_FREQ_03MHZ  16
/* BUS_WIDTH_OPTION */
#define BUS_WIDTH_1BIT  0
#define BUS_WIDTH_4BIT  1
#define BUS_WIDTH_8BIT  2
/* BUS_VOLTAGE_OPTION */
#define BUS_VOLTAGE_33  7   /* 3.3 volts */
#define BUS_VOLTAGE_30  6   /* 3.0 volts */
#define BUS_VOLTAGE_18  5   /* 1.8 volts */

/* OCR */
#define OCR_READY   0x80000000
#define OCR_SECTOR_MODE 0x40000000
#define OCR_VDD_33_34   0x00200000
#define OCR_VDD_17_195  0x00000080
/* RCA */
#define EMMC_RCA 0x00020000  /* From 0x00020000, 0x00030000, ..., 0xFFFF0000 */

/* op-code issue related timeout values */
#define POLL_DELAY_USECS 1
#define SOFTWARE_RESET_TIMEOUT_USECS 1000000    /* 1 second */

/* op-code issue related timeout values */
#define POLL_DELAY_USECS 1
#define CMD_TIMEOUT_USECS 200000
#define CMD_SLEEP_CNT (CMD_TIMEOUT_USECS / POLL_DELAY_USECS)
#define DATA_TIMEOUT_USECS 1000000
#define DATA_SLEEP_CNT (DATA_TIMEOUT_USECS / POLL_DELAY_USECS)
#define SOFTWARE_RESET_TIMEOUT_USECS 1000000    /* 1 second */

#define HOST_BUF_SIZE       7    /* 0:4K, 1:8K, 2:16K, 3:32K, 4:64K, 5:128K, 6:256K, 7:512K : Double buffering. */
#define EMMC_DMA_BUF_ALIGN  512

/* ARASAN IP Specification */
#define ARASAN_SDMA_MAX_BLKS    512

/* Interrupt bit defines */
#define HOST_INT_STATUS_ALL 0xffffffff
/*
 * Enable the following interrupt bits:
 * [28] - TARGET_RESP_ERR_INT
 * [25] - ADMA_ERR_INT
 * [24] - AUTO_CMD_ERR_INT
 * [23] - CURRENT_LIMIT_ERR_INT
 * [22] - CURRENT_LIMIT_ERR_INT
 * [21] - DATA_CRC_ERR_INT
 * [20] - DATA_TIMEOUT_ERR_INT
 * [19] - CMD_INDEX_ERR_INT
 * [18] - CMD_END_BIT_ERR_INT
 * [17] - CMD_CRC_ERR_INT
 * [16] - CMD_TIMEOUT_ERR_INT
 * [10] - INT_B
 * [9] - INT_A
 * [5] - BUFFER_READ_INT
 * [4] - BUFFER_WRITE_INT
 * [3] - DMA_INT
 * [2] - BLOCK_GAP_INT
 * [1] - TRANSFER_COMPLETE_INT
 * [0] - COMMAND_COMPLETE_INT
 * Disable the following interrupt bits:
 * [26] - TUNE_ERR_STAT_EN
 * [14] - BOOT_TERM_EN
 * [13] - BOOT_ACK_RCV_EN
 * [12] - RETUNE_EVENT_EN
 * [11] - INT_C_EN
 * [08] - CARD_INT_ENA
 * [07] - CAR_REMOVAL_INT_ENA
 * [06] - CAR_INSERT_INT_ENA
 */
#define HOST_INT_ENABLES 0x13FF063F

/*  
 * CMD index, argument
*/
struct mmc_cmd {
    int index;
    uint32_t arg;
};

/*  
 * MMC data info
*/
struct mmc_data {
    uintptr_t dma_address;
    //uint32_t block_size;
    uint32_t block_count;
    uint32_t block_address;
};

struct cmd_info {
    uint8_t index;          /* CMD0 - CMD63 */
    uint8_t type;           /* Cmd type from MMC spec */
#define CMDINFO_TYPE_BC 0
#define CMDINFO_TYPE_BCR 1
#define CMDINFO_TYPE_AC 2
#define CMDINFO_TYPE_ADTC 3
    uint8_t resp;           /* Cmd response type from MMC spec */
#define CMDINFO_RESP_NONE 0
#define CMDINFO_RESP_R1 1
#define CMDINFO_RESP_R1B 2
#define CMDINFO_RESP_R2 3
#define CMDINFO_RESP_R3 4
#define CMDINFO_RESP_R4 5
#define CMDINFO_RESP_R5 6
#define CMDINFO_RESP_R6 7
#define CMDINFO_RESP_R7 8
    uint8_t data_xfer;      /* Transfer direction (Host based) */
#define CMDINFO_DXFER_NONE 0
#define CMDINFO_DXFER_READ 1
#define CMDINFO_DXFER_READ_MULTI 2
#define CMDINFO_DXFER_WRITE 3
#define CMDINFO_DXFER_WRITE_MULTI 4
    char *desc;
};

/*
 * Command Implementation
 * ----------------------
 * Respons Type Information
 *  R1 (Index check  On, CRC check  On, 48bit)
 *    : CMD 3,7,8,11,12,13,14,19,16,17,18,20,23,24,25,26,27,30,31,35,36,42,55,56
 *  R1b(Index check  On, CRC check  On, 48bit/busy_check)
 *    : CMD 5,6,7,12,28.29,38
 *  R2 (Index check Off, CRC check  On, 136bit) : CMD 2,9,10
 *  R3 (Index check Off, CRC check Off, 48bit)  : CMD 1
 *  R4 (Index check Off, CRC check Off, 48bit)  : CMD 39
 *  R5 (Index check  On, CRC check  On, 48bit)  : CMD 40
 * Clase Information v4.41
 *  class 0 : basic : CMD 0,1,2,3,4,5,6,7,8,9,10,12,13,14,15,19
 *  class 1 : stream read (Obsolete on v4.5) : CMD 11
 *  class 2 : block read      : CMD 16,17,18,21(v4.5)
 *  class 3 : stream write (Obsolete on v4.5) : CMD 20
 *  class 4 : block write     : CMD 23,24,25,26,27,49(v4.5)
 *  class 5 : erase       : CMD 35,36,38
 *  class 6 : write protection    : CMD 28,29,30,31
 *  class 7 : lock card       : CMD 42
 *  class 8 : applicationspecific : CMD 55,56
 *  class 9 : I/O mode        : CMD 39,40
 *  class 10???11 : reserved
 * -------------------------------------------------
 */
#define IDX_CMD0    0 
#define IDX_CMD1    1
#define IDX_CMD2    2
#define IDX_CMD3    3
#define IDX_CMD6    4
#define IDX_CMD7    5
#define IDX_CMD13   6
#define IDX_CMD16   7
#define IDX_CMD18   8
#define IDX_CMD_MAX 9

/* Local Variables */

#if defined(_BCM963158_)  /* temporary change to support runtime detection for A0 and B0 chips */
#undef EMMC_HOSTIF
#undef EMMC_TOP_CFG
#undef EMMC_BOOT
volatile EmmcHostIfRegs* EMMC_HOSTIF = (volatile EmmcHostIfRegs *) EMMC_HOSTIF_BASE;
volatile EmmcTopCfgRegs* EMMC_TOP_CFG = (volatile EmmcTopCfgRegs *) EMMC_TOP_CFG_BASE;
volatile EmmcBootRegs* EMMC_BOOT = (volatile EmmcBootRegs *) EMMC_BOOT_BASE;
#endif

static int emmc_phys_partition = PCFG_PARTITION_ACCESS_DATA;
static int emmc_dma_enable = 0;
static const struct cmd_info emmc_cmd_table[IDX_CMD_MAX] = {
    {
        0, CMDINFO_TYPE_BC, CMDINFO_RESP_NONE, CMDINFO_DXFER_NONE,  /* CMD0, idx0 */
        "GO_IDLE_STATE"
    },
    {
        1, CMDINFO_TYPE_BCR, CMDINFO_RESP_R3, CMDINFO_DXFER_NONE,   /* CMD1, idx1 */
        "SEND_OP_COND"
    },
    {
        2, CMDINFO_TYPE_BCR, CMDINFO_RESP_R2, CMDINFO_DXFER_NONE,   /* CMD2, idx2 */
        "ALL_SEND_CID"
    },
    {
        3, CMDINFO_TYPE_AC, CMDINFO_RESP_R1, CMDINFO_DXFER_NONE,    /* CMD3, idx3 */
        "SET_RELATIVE_ADDR"
    },
    {
        6, CMDINFO_TYPE_AC, CMDINFO_RESP_R1B, CMDINFO_DXFER_NONE,   /* CMD6, idx4 */
        "SWITCH"
    },
    {
        7, CMDINFO_TYPE_AC, CMDINFO_RESP_R1, CMDINFO_DXFER_NONE,    /* CMD7, idx5 */
        "SELECT_CARD"
    },
    {
        13, CMDINFO_TYPE_AC, CMDINFO_RESP_R1, CMDINFO_DXFER_NONE,   /* CMD13, idx6 */
        "SEND_STATUS"
    },
    {
        16, CMDINFO_TYPE_AC, CMDINFO_RESP_R1, CMDINFO_DXFER_NONE,   /* CMD16, idx7 */
        "SET_BLOCKLEN"
    },
    {
        18, CMDINFO_TYPE_ADTC, CMDINFO_RESP_R1, CMDINFO_DXFER_READ_MULTI,   /* CMD18, idx8 */
        "READ_MULTIPLE_BLOCK"
    }
};
uint32_t raw_cid[4]; /* CID(Device IDentification) for CMD1 */

/* Externs */
extern void cfe_usleep(int usec);

/* Prototypes */ 
static int emmc_software_reset(uint32_t mask);
static int emmc_cmd13_send_status(uint32_t *status);

/* Functions */
static void handle_interrupt_error(void)
{
    uint32_t status;

    status = BDEV_RD(&EMMC_HOSTIF->emmc_host_int_status);
    xprintf(" error interrupt, status = 0x%08x\n", status & SDHC_INT_ERR_MASK);
    BDEV_WR(&EMMC_HOSTIF->emmc_host_int_status, status);       
}


static int wait_cmd_complete(void)
{
    uint32_t timeout_cnt = 0;
    uint32_t int_status;

    /* Wait for COMMAND COMPLETE or error interrupt */
    timeout_cnt = 0;
    while (++timeout_cnt <= (CMD_TIMEOUT_USECS / POLL_DELAY_USECS)) {
        int_status = BDEV_RD(&EMMC_HOSTIF->emmc_host_int_status) &
            (SDHC_INT_CMD_COMPLETE | SDHC_INT_ERR);
        if (int_status)
            break;
        cfe_usleep(POLL_DELAY_USECS);
    }
    if (timeout_cnt > CMD_SLEEP_CNT) {
        xprintf(" timed out waiting for cmd complete, intr = ");
        DBG_MSG_WR_HEX(int_status);
        xprintf("\n");
        return EMMC_NG;
    }

    /* Check for clean completion */
    if (int_status & SDHC_INT_ERR) {
        xprintf(" error waiting for cmd complete\n");
        handle_interrupt_error();
        return EMMC_NG;
    }

    /* Clear the interrupt for next time */
    BDEV_WR(&EMMC_HOSTIF->emmc_host_int_status, int_status);
    return EMMC_OK;
}

static int wait_buffer_read_int(void)
{
    uint32_t int_status;
    uint32_t timeout = 0;

    while (1) {
        int_status = BDEV_RD(&EMMC_HOSTIF->emmc_host_int_status);
        if (int_status & SDHC_INT_READ_BUF) {
            DBG_MSG_PUTS("buffer_read interrupt\n");
            /* Clear the Buffer Read interrupt */
            BDEV_WR(&EMMC_HOSTIF->emmc_host_int_status, int_status & SDHC_INT_READ_BUF);
            break;
        }
        if (int_status & SDHC_INT_ERR_MASK) {
            DBG_MSG_PUTS(" buffer read intr error : 0x");
            xprintf(" buffer read intr error : 0x%08x\n", int_status);
            DBG_MSG_WR_HEX(int_status);
            return EMMC_NG;
        }
        if (++timeout >= DATA_SLEEP_CNT) {
            DBG_MSG_PUTS(" buffer read timed out, intr : 0x");
            xprintf(" buffer read timed out, intr : 0x%08x\n", int_status);
            DBG_MSG_WR_HEX(int_status);
            return EMMC_NG;
        }
        cfe_usleep(POLL_DELAY_USECS);
    }

    DBG_MSG_PUTS(" OK: intr=0x");
    DBG_MSG_WR_HEX(int_status);
    DBG_MSG_PUTS(", timeout=");
    DBG_MSG_WR_INT(timeout);
    return EMMC_OK;
}

static int wait_xfer_complete(void)
{
    uint32_t int_status;
    uint32_t timeout = 0;

    DBG_MSG_PUTS(" wait_xfer_complete\n");
    while (1) {
        int_status = BDEV_RD(&EMMC_HOSTIF->emmc_host_int_status);
        if (int_status & SDHC_INT_XFER_COMPLETE)
            break;
        if (int_status & SDHC_INT_XFER_DMA) {
            DBG_MSG_PUTS(" DMA interrupt, SDMA addr\n");
            /* Clear the DMA interrupt */
            BDEV_WR(&EMMC_HOSTIF->emmc_host_int_status, int_status & SDHC_INT_XFER_DMA);
            /*
             * write the address back to the address
             * to restart the DMA.
             */
            BDEV_WR(&EMMC_HOSTIF->emmc_host_sdma, BDEV_RD(&EMMC_HOSTIF->emmc_host_sdma));
        }
        if (int_status & SDHC_INT_ERR_MASK) {
            DBG_MSG_PUTS(" transfer complete intr error : 0x");
            DBG_MSG_WR_HEX(int_status);
            return EMMC_NG;
        }
        if (++timeout >= DATA_SLEEP_CNT) {
            DBG_MSG_PUTS(" transfer complete timed out, intr : 0x");
            DBG_MSG_WR_HEX(int_status);
            return EMMC_NG;
        }
        cfe_usleep(POLL_DELAY_USECS);
    }

    DBG_MSG_PUTS(" OK: intr=0x");
    DBG_MSG_WR_HEX(int_status);
    DBG_MSG_PUTS(", timeout=");
    DBG_MSG_WR_INT(timeout);
    return EMMC_OK;
}

static int wait_ready_data_xfer(void)
{
    uint32_t emmc_status = 0;
    uint32_t emmc_state = 0;
    uint32_t timeout_cnt = 0;
    uint32_t cst = 0;

    do {
        if (emmc_cmd13_send_status(&cst))
            return EMMC_NG;
        emmc_status = (cst & CST_READY_FOR_DATA_MASK) >>
            CST_READY_FOR_DATA_SHIFT;
        emmc_state = (cst & CST_CURRENT_STATE_MASK) >>
            CST_CURRENT_STATE_SHIFT;
        if ((emmc_status == 1) && (emmc_state == CST_STATE_TRAN))
            break;
        cfe_usleep(POLL_DELAY_USECS);
        timeout_cnt++;
    } while (timeout_cnt < DATA_SLEEP_CNT);
    if (timeout_cnt >= DATA_SLEEP_CNT) {
        DBG_MSG_PUTS(" Timeout(1000000 cnt) waiting for data xfer ready, CST: 0x");
        DBG_MSG_WR_HEX(cst);
        DBG_MSG_PUTS("\n");
        return EMMC_NG;
    }
    DBG_MSG_PUTS("[wait_ready_data_xfer], CST: 0x");
    DBG_MSG_WR_HEX(cst);
    DBG_MSG_PUTS("\n");
        
    return EMMC_OK;
}


/*  
 * wait_inhibit_clear
*/
static int wait_inhibit_clear(struct mmc_cmd *cmd, int data_inhibit)
{
    int mask = SDHC_STATE_CMD_INHIBIT;
    int timeout;

    /*
     * If new command has a data transfer or is using BUSY signaling,
     * also wait for DATA INHIBIT to clear.
     */
    if (data_inhibit) {
        timeout = DATA_SLEEP_CNT;
        mask |= SDHC_STATE_CMD_INHIBIT_DAT;
    }
    else {
        timeout = CMD_SLEEP_CNT;
    }
    while (1) {
        if ((BDEV_RD(&EMMC_HOSTIF->emmc_host_state) & mask) == 0)
            break;
        if (timeout-- == 0)
            break;
        cfe_usleep(POLL_DELAY_USECS);
    }
    if (timeout < 0) {
        xprintf("Timeout waiting for INHIBIT lines to clear: 0x");
        DBG_MSG_WR_HEX(BDEV_RD(&EMMC_HOSTIF->emmc_host_state));
        return EMMC_NG;
    }
    return EMMC_OK;
}

/*  
 * Copies blocks of data via PIO 
*/
static int copy_pio_block_data( unsigned char * data_buffer, int block_count )
{
    unsigned int * buffer = (unsigned int *)data_buffer;
    int i,j;
    int num_words = EMMC_DFLT_BLOCK_SIZE/sizeof(unsigned int);

    for( i=0; i< block_count; i++ )
    {
        /* Wait for buffer data to be ready */
        if (wait_buffer_read_int())
            return EMMC_NG;

        /* Read data from internal transfer buffer */
        for( j=0; j<num_words; j++ )  // 512B = 128 * 32bit(Reg. size)
        {
            cfe_usleep(POLL_DELAY_USECS);
            buffer[ (i*num_words) + j ] = EMMC_HOSTIF->emmc_host_buffdata;
#if DEBUG_EMMC_DATA
            xprintf("buffer[0x%x] = 0x%08x\n", ((i*num_words) + j )*4, buffer[(i*num_words) + j ]);
#endif
        }
    }
    return EMMC_OK;
}

/*  
 * get_cmd_mode
*/
static uint32_t get_cmd_mode(int index)
{
    uint32_t cmd_mode = 0;
    const struct cmd_info *entry = &emmc_cmd_table[index];

    cmd_mode = ((uint32_t)entry->index & 0x3f) << EMMC_HOSTIF_CMD_MODE_CMD_INDEX_SHIFT;
    switch (entry->resp) {
    case CMDINFO_RESP_R1:
    case CMDINFO_RESP_R5:
    case CMDINFO_RESP_R6:
    case CMDINFO_RESP_R7:
        cmd_mode |= (SDHC_CMD_CHK_INDEX |
                 SDHC_CMD_CHK_CRC |
                 SDHC_CMD_RESP_48);
        break;
    case CMDINFO_RESP_R2:
        cmd_mode |= (SDHC_CMD_CHK_CRC |
                 SDHC_CMD_RESP_136);
        break;
    case CMDINFO_RESP_R3:
    case CMDINFO_RESP_R4:
        cmd_mode |= SDHC_CMD_RESP_48;
        break;
    case CMDINFO_RESP_R1B:
        cmd_mode |= (SDHC_CMD_CHK_INDEX |
                 SDHC_CMD_CHK_CRC |
                 SDHC_CMD_RESP_48_BSY);
        break;
    }

    /* Setup for transfer on DATA lines */
    if (entry->data_xfer != CMDINFO_DXFER_NONE) {
        cmd_mode |= SDHC_CMD_DATA;
        
        if( emmc_dma_enable )
            cmd_mode |= SDHC_MODE_DMA;
        else
            cmd_mode &= ~SDHC_MODE_DMA;
    }

    /*
     * If multiblock, set multi bit, enable block cnt register
     * and enable Auto CMD12
     */
    if ((entry->data_xfer == CMDINFO_DXFER_READ_MULTI) ||
        (entry->data_xfer == CMDINFO_DXFER_WRITE_MULTI)) {
        cmd_mode |= (SDHC_MODE_MULTI | SDHC_MODE_ACMD12 | SDHC_MODE_BLK_CNT);
    }

    /* bit is zero for write, 1 for read */
    if ((entry->data_xfer == CMDINFO_DXFER_READ) ||
        (entry->data_xfer == CMDINFO_DXFER_READ_MULTI))
        cmd_mode |= SDHC_MODE_XFER_DIR_READ;

    return cmd_mode;
}

/*  
 * Issue command
*/
static int issue_cmd(struct mmc_cmd *cmd, struct mmc_data *data)
{
    uint32_t cmd_mode;
    const struct cmd_info *entry = &emmc_cmd_table[cmd->index];
    int data_xfer;
    //int cache_range; /* unused if CFG_UNCACHED */

    data_xfer = (entry->data_xfer != CMDINFO_DXFER_NONE);
#if DEBUG_EMMC_FSBL
    if (data_xfer) {
        DBG_MSG_PUTS("\n dma addr : ");
        DBG_MSG_WR_HEX(data->dma_address);
        DBG_MSG_PUTS(", block size : ");
        DBG_MSG_WR_INT(EMMC_DFLT_BLOCK_SIZE);
        DBG_MSG_PUTS(", blocks : ");
        DBG_MSG_WR_INT(data->block_count);
        DBG_MSG_PUTS(", emmc addr : ");
        DBG_MSG_WR_HEX(data->block_address);
    }
    DBG_MSG_PUTS("\n");
#endif

    /* sanity check */
    if ((data != NULL) ^ (data_xfer)) {
        xprintf("entry->desc");
        xprintf("- data phase mismatch");
        goto err;
    }
    if (wait_inhibit_clear(cmd, data_xfer)) {
        xprintf("inhibit_clear failed\n");
        goto err;
    }

    /* Start with all interrupts cleared */
    BDEV_WR(&EMMC_HOSTIF->emmc_host_int_status, HOST_INT_STATUS_ALL);

    /* The command includes a data transfer */
    if (data_xfer) {
        //cache_range = data->block_count * EMMC_DFLT_BLOCK_SIZE;
        BDEV_WR(&EMMC_HOSTIF->emmc_host_sdma, data->dma_address);
        BDEV_WR(&EMMC_HOSTIF->emmc_host_block, SDHC_MAKE_BLK_REG(data->block_count,
             EMMC_DFLT_BLOCK_SIZE, HOST_BUF_SIZE));
    }
    cmd_mode = get_cmd_mode(cmd->index);
    BDEV_WR(&EMMC_HOSTIF->emmc_host_argument, cmd->arg);
#if DEBUG_EMMC_FSBL
    DBG_MSG_PUTS(" SDMA : 0x");
    DBG_MSG_WR_HEX(BDEV_RD(&EMMC_HOSTIF->emmc_host_sdma));
    DBG_MSG_PUTS(", BLOCK : ");
    DBG_MSG_WR_HEX(BDEV_RD(&EMMC_HOSTIF->emmc_host_block));
    DBG_MSG_PUTS(", ARG : ");
    DBG_MSG_WR_HEX(BDEV_RD(&EMMC_HOSTIF->emmc_host_argument));
    DBG_MSG_PUTS(", CMD : ");
    DBG_MSG_WR_HEX(cmd_mode);
    DBG_MSG_PUTS("\n");
#endif  
    BDEV_WR(&EMMC_HOSTIF->emmc_host_cmd_mode, cmd_mode);
    if (wait_cmd_complete()) {
        xprintf("wait_cmd_complete failed\n");
        goto err;
    }
    
    /* If the command has a data transfer, wait for transfer complete */
    if (data_xfer) {
        /* For PIO reads we need to wait for the buffer read ready signal */
        if ( !emmc_dma_enable ) {
            if (copy_pio_block_data( (unsigned char *)data->dma_address, data->block_count ))
                    goto err;
            }

        if (wait_xfer_complete())
            goto err;
    }

    /*
     * If command is using BUSY signalling, also wait for DATA INHIBIT
     * to clear.
     */
    if (emmc_cmd_table[cmd->index].resp == CMDINFO_RESP_R1B)
        wait_inhibit_clear(cmd, 1);

    DBG_MSG_PUTS("[issue_cmd] success\n");
    return EMMC_OK;

err:
    xprintf("[issue_cmd] error\n");
    emmc_software_reset(SDHC_SW_RESET_DAT | SDHC_SW_RESET_CMD);
    return EMMC_NG;
}

#define ARG_CMD0_GO_IDEL_STATE  0x00000000
static int emmc_cmd0_go_idle_state(void)
{
    int res;
    struct mmc_cmd cmd;

    /*
     * CMD 0 - GO_IDLE_STATE (bc,-), GO_PRE_IDLE_STATE (bc,-),
     * BOOT_INITIATION (-,-)
     */
    cmd.index = IDX_CMD0;
    cmd.arg = ARG_CMD0_GO_IDEL_STATE;
    res = issue_cmd(&cmd, NULL);
    
    /* wait for card to settle */
    cfe_usleep(1000);
    return res; 
}

/*
 * Get the card in idle state To send its Operating Conditions
 * Register (OCR) contents in the response
 */
static int emmc_cmd1_send_op_cond(uint32_t *ocr)
{
    struct mmc_cmd cmd;
    int res = EMMC_OK;

    cmd.index = IDX_CMD1;
    cmd.arg = OCR_SECTOR_MODE | OCR_VDD_33_34;
    res = issue_cmd(&cmd, NULL);
    *ocr = BDEV_RD(&EMMC_HOSTIF->emmc_host_resp_01);
    return res;
}

/*
 * Gets card CID (Card Identification) from eMMC device.
 */
static int  emmc_cmd2_all_send_cid_single_device(void)
{
    struct mmc_cmd cmd;
    int res;

    cmd.index = IDX_CMD2;
    cmd.arg = 0;
    res = issue_cmd(&cmd, NULL);
    if (res)
        return res;
    raw_cid[0] = BDEV_RD(&EMMC_HOSTIF->emmc_host_resp_01);
    raw_cid[1] = BDEV_RD(&EMMC_HOSTIF->emmc_host_resp_23);
    raw_cid[2] = BDEV_RD(&EMMC_HOSTIF->emmc_host_resp_45);
    raw_cid[3] = BDEV_RD(&EMMC_HOSTIF->emmc_host_resp_67);
    return res;
}

/*
 * Set relative card address
 */
static int emmc_cmd3_set_rca(void)
{
    struct mmc_cmd cmd;

    /* CMD3, R1 (Index check  On, CRC check  On, 48bit) */
    cmd.index = IDX_CMD3;
    cmd.arg = EMMC_RCA;
    return issue_cmd(&cmd, NULL);
}

/*
 * Switch ExtCSD
 */
static int emmc_cmd6_switch_extcsd(uint32_t access,
                uint32_t index,
                uint32_t val)
{
    struct mmc_cmd cmd;
    uint32_t arg;
    uint32_t status = 0;
    int res;
    uint32_t emmc_state;
    uint32_t timeout_cnt = 0;

    arg = 0;
    arg = (access & 0x03) << 24;
    arg = arg + ((index & 0xFF) << 16);
    arg = arg + ((val & 0xFF) << 8);

    /* CMD6 - R1b(Index check  On, CRC check  On, 48bit/busy_check) */
    cmd.index = IDX_CMD6;
    cmd.arg = arg;
    res = issue_cmd(&cmd, NULL);
    if (res)
        return res;

    /* Wait until we transition from PRG to TRAN state */
    do {
        if (emmc_cmd13_send_status(&status))
            return EMMC_NG;
        if (status & CST_SWITCH_ERROR_MASK) {
            xprintf("CMD 6 - SWITCH error in status: 0x");
            DBG_MSG_WR_HEX(status);
            xprintf("\n");
            return EMMC_NG;
        }
        emmc_state = (status & CST_CURRENT_STATE_MASK) >>
            CST_CURRENT_STATE_SHIFT;
        if (emmc_state == CST_STATE_TRAN)
            break;
        cfe_usleep(POLL_DELAY_USECS);
        timeout_cnt++;
    } while (timeout_cnt < DATA_SLEEP_CNT);
    if (timeout_cnt >= DATA_SLEEP_CNT) {
        xprintf("Timeout waiting for TRAN state after SWITCH cmd, "
            "STATUS: 0x");
        DBG_MSG_WR_HEX(status);
        xprintf("\n");
        return EMMC_NG;
    }
    return EMMC_OK;
}

static int emmc_cmd7_select_card_stby_tans(void)
{
    struct mmc_cmd cmd;

    /*
     * CMD7
     * While selecting from Stand-By State to Transfer State
     *   R1 (Index check  On, CRC check  On, 48bit)
     * While selecting from Disconnected State to Programming State.
     *   R1b(Index check  On, CRC check  On, 48bit/busy_check)
     */
    cmd.index = IDX_CMD7;
    cmd.arg = EMMC_RCA;
    return issue_cmd(&cmd, NULL);
}

static int emmc_cmd13_send_status(uint32_t *status)
{
    struct mmc_cmd cmd;
    int res;

    /* CMD13, R1 (Index check  On, CRC check  On, 48bit) */
    cmd.index = IDX_CMD13;
    cmd.arg = EMMC_RCA;
    res = issue_cmd(&cmd, NULL);
    if (res)
        return res;
    *status = BDEV_RD(&EMMC_HOSTIF->emmc_host_resp_01);

    return res;
}

static int emmc_cmd16_set_blocklen(uint32_t block_length)
{
    struct mmc_cmd cmd;
    uint32_t reg;

    //reg = HOST_REG(chip, BLKCNT_BLKSIZE);
    reg = BDEV_RD(&EMMC_HOSTIF->emmc_host_block);
    BDEV_WR(&EMMC_HOSTIF->emmc_host_block, (reg & ~SDHC_REG_BLKSIZE_MASK) + 
                     block_length);
    //HOST_REG(chip, BLKCNT_BLKSIZE) = (reg & ~SDHC_REG_BLKSIZE_MASK) +
    //               block_length;

    /* CMD16 -  R1 (Index check  On, CRC check  On, 48bit) */
    cmd.index = IDX_CMD16;
    cmd.arg = block_length;
    return issue_cmd(&cmd, NULL);
}

static int emmc_cmd18_read_multiple_block(
                uint32_t dma_addr,
                uint32_t block_count,
                uint32_t block_addr)
{
    struct mmc_cmd cmd;
    struct mmc_data data;

    cmd.index = IDX_CMD18;
    cmd.arg = block_addr;
    data.dma_address = dma_addr;
    data.block_count = block_count;
    data.block_address = block_addr;
    return issue_cmd(&cmd, &data);
}

static int emmc_software_reset(uint32_t mask)
{
    int cnt;
    
    BDEV_WR(&EMMC_HOSTIF->emmc_host_ctrl_set1, BDEV_RD(&EMMC_HOSTIF->emmc_host_ctrl_set1) | mask);
    
    cnt = SOFTWARE_RESET_TIMEOUT_USECS;
    while (cnt > 0) {
        cfe_usleep(POLL_DELAY_USECS);
        if ((BDEV_RD(&EMMC_HOSTIF->emmc_host_ctrl_set1) & mask) == 0)
            return EMMC_OK;
        cnt -= POLL_DELAY_USECS;
    }
    xprintf("Host RESET timeout.");
    DBG_MSG_PUTS("Host RESET timeout waiting for 0x%x");
    DBG_MSG_WR_HEX(BDEV_RD(&EMMC_BOOT->emmc_boot_status) & mask);
    return EMMC_NG;
}

static int emmc_set_clock(uint32_t set_clock)
{
    uint32_t control1;
    int timeout;

    /* Set clock values : freq, timeout, enable int_clk
     * SDHC_CLKCTL_FREQ, SDHC_CLKCTL_FREQ_UPPER, 
     * SDHC_TOCTL_TIMEOUT, SDHC_CLKCTL_INTERN_CLK_ENA
     */
    control1 = BDEV_RD(&EMMC_HOSTIF->emmc_host_ctrl_set1);
    DBG_MSG_PUTS(" HOST_CTRL_SET1 0x");
    DBG_MSG_WR_HEX(control1);
    DBG_MSG_PUTS("\n");
    control1 = (control1 & ~0xfffff ) | set_clock;
    BDEV_WR(&EMMC_HOSTIF->emmc_host_ctrl_set1, control1);

#define INT_CLK_STABLE_TIMEOUT  50000   /* 50[ms] */
    /* Wait 50ms for internal clock stable */
    timeout = INT_CLK_STABLE_TIMEOUT / POLL_DELAY_USECS;
    while (1) {
        if (timeout-- == 0)
            break;
        if (BDEV_RD(&EMMC_HOSTIF->emmc_host_ctrl_set1) & SDHC_CLKCTL_INTERN_CLK_STABLE)    /*EMMC_HOSTIF_CTRL_SET1_INTERNAL_CLK_STABLE_MASK*/
            break;
        cfe_usleep(POLL_DELAY_USECS);
    }
    if (timeout <= 0) {
        xprintf(" timeout : internal clock stable");
        return 1;
    }

    /* Enable SD_Clk */
    control1 |= SDHC_CLKCTL_SD_CLK_ENA;
    BDEV_WR(&EMMC_HOSTIF->emmc_host_ctrl_set1, control1);
    DBG_MSG_PUTS(" Enable HOST_CTRL_SET1 0x");
    DBG_MSG_WR_HEX(control1);
    DBG_MSG_PUTS("\n");

#define EXT_CLK_STABLE_TIMEOUT  1000   /* 1[ms] */
    cfe_usleep(EXT_CLK_STABLE_TIMEOUT * POLL_DELAY_USECS);

    return 0;
}


static int setup_bus_freq_width(void)
{
    uint32_t reg_ctrl_set0;
    int res = 0;

    reg_ctrl_set0 = BDEV_RD(&EMMC_HOSTIF->emmc_host_ctrl_set0);
    
    /* Bus Frequency Setting : eMMC, Host controller */
#if (EMMC_HS_TIMING == HS_TIMING_FULL)
#define EMMC_DATA_TRAN_CLK_SET 0xe0201 /* timeout=0xe0000, freq=0x0200(25MHz), enable_int_clk=0x01 frequency to work all eMMC as possible */
    /* Full Speed Mode - 25MHz */
    DBG_MSG_PUTS("[setup_bus_freq_width] HS_TIMING_FULL\n");
    res = emmc_cmd6_switch_extcsd(Idx_ExtCSD_ACC_WRB,
                 Idx_ExtCSD_HS_TIMING, HS_TIMING_FULL);
#else
#define EMMC_DATA_TRAN_CLK_SET 0xe0101 /* timeout=0xe0000, freq=0x0100(50MHz), enable_int_clk=0x01 frequency to work all eMMC as possible */
    /* High Speed Mode - 50MHz */
    DBG_MSG_PUTS("[setup_bus_freq_width] HS_TIMING_HS\n");
    res = emmc_cmd6_switch_extcsd(Idx_ExtCSD_ACC_WRB,
                 Idx_ExtCSD_HS_TIMING, HS_TIMING_HS);
    reg_ctrl_set0 |= SDHC_CTL1_HIGH_SPEED;
    BDEV_WR(&EMMC_HOSTIF->emmc_host_ctrl_set0, reg_ctrl_set0);
#endif
    if (res)
        return res;
    DBG_MSG_PUTS("[setup_bus_freq_width] emmc_set_clock\n");
    if (emmc_set_clock(EMMC_DATA_TRAN_CLK_SET))
        return EMMC_NG;

    /*
     * Bus Width Setting : eMMC, Host controller, BUS_WIDTH_8BIT
     */
    DBG_MSG_PUTS("[setup_bus_freq_width] BUS_WIDTH_8BIT\n");
    res = emmc_cmd6_switch_extcsd(Idx_ExtCSD_ACC_WRB,
                 Idx_ExtCSD_BUS_WIDTH, BUS_WIDTH_8BIT);
    if (res)
        return res;
    reg_ctrl_set0 &= ~SDHC_CTL1_4BIT;
    reg_ctrl_set0 |= SDHC_CTL1_8BIT;
    /*  Set registers */
    BDEV_WR(&EMMC_HOSTIF->emmc_host_ctrl_set0, reg_ctrl_set0);
    
    /* BOOT_BUS_WIDTH : BRCM EMMC BOOT support only 8-bit */
    res = emmc_cmd6_switch_extcsd(Idx_ExtCSD_ACC_WRB,
                  Idx_ExtCSD_BOOT_BUS_CONDITIONS, 0x2);
    if (res)
        return res;

    return EMMC_OK;
}

#define SDIO_SW_RESET_MASK 0x07000000   /* (SDHC_SW_RESET_DAT | SDHC_SW_RESET_CMD | SDHC_SW_RESET_ALL) */

/* Init the eMMC device
 * Return:
 *  0 - success
 *  1 - retry
 *  -1 - fail
 */
uint32_t ocr_data;
static int  init_device(void)
{
    int res;
    int timeout_cnt;

    DBG_MSG_PUTS("\n[init_device] emmc_software_reset\n");
    if (emmc_software_reset(SDHC_SW_RESET_DAT | SDHC_SW_RESET_CMD | SDHC_SW_RESET_ALL))
    {
        xprintf("\n[init_device] emmc_software_reset failed\n");
        return EMMC_NG;
    }

    /* (SDHC_BUS_PWR + (EMMC_BUS_VOLTAGE << SDHC_BUS_VOLT_SHIFT) */
#if (EMMC_BUS_VOLTAGE == BUS_VOLTAGE_33)
 #define EMMC_BUS_PWR_VOLT  0x00000F00
#else /* (EMMC_BUS_VOLTAGE == BUS_VOLTAGE_18) */
 #define EMMC_BUS_PWR_VOLT  0x00000B00
#endif
    BDEV_WR(&EMMC_HOSTIF->emmc_host_ctrl_set0, EMMC_BUS_PWR_VOLT);

#define EMMC_BOOT_MODE_CLK_SET 0xef001 /* timeout=0xe0000, freq=0xf000(208KHz), enable_int_clk=0x01 frequency to work all eMMC as possible */
    DBG_MSG_PUTS("[init_device] emmc_set_clock\n");
    if (emmc_set_clock(EMMC_BOOT_MODE_CLK_SET))
    {
        xprintf("[init_device] emmc_set_clock failed\n");
        return EMMC_NG;
    }
    
    DBG_MSG_PUTS("[init_device] Enable interrupts\n");
    /* Enable interrupts */
    BDEV_WR(&EMMC_HOSTIF->emmc_host_int_status_ena, HOST_INT_ENABLES);

    /* CMD0 : Device Reset */
    DBG_MSG_PUTS("[init_device] emmc_cmd0_go_idle_state\n");
    if (emmc_cmd0_go_idle_state())
    {
        xprintf("[init_device] emmc_cmd0_go_idle_state failed\n");
        return EMMC_NG;
    }

    /*
     * Voltage validation and wait for device ready.
     * Wait for up to 1 second  for ready bit
     */
    DBG_MSG_PUTS("[init_device] Voltage validation\n");
    timeout_cnt = 1000000 / 1000;
    while (1) {
        res = emmc_cmd1_send_op_cond(&ocr_data);
        if (res)
        {
            xprintf("[init_device] Voltage validation failed");
            return EMMC_NG;
        }

        /* Make sure the card supports 3.3V */
        if ((ocr_data & OCR_VDD_33_34) == 0) {
            xprintf("eMMC device does not support 3.3V");
            return EMMC_NG;
        }
        if (ocr_data & 0x80000000)  /* ready bit */
            break;
        if (timeout_cnt-- == 0)
            break;
        cfe_usleep(1000);
    }
    if (timeout_cnt < 0)
    {
        xprintf("[init_device] Voltage validation timed out failed\n");
        return EMMC_NG;
    }

    /* TODO: What does this mean? */
    /* eMMC must be over 4GB */

    /* Set CID : CMD2 */
    DBG_MSG_PUTS("[init_device] emmc_cmd2_all_send_cid_single_device\n");
    res = emmc_cmd2_all_send_cid_single_device();
    if (res)
    {
        xprintf("[init_device] emmc_cmd2_all_send_cid_single_device failed\n");
        return EMMC_NG;
    }

    /* Set RCA : CMD3*/
    DBG_MSG_PUTS("[init_device] emmc_cmd3_set_rca\n");
    res = emmc_cmd3_set_rca();
    if (res)
    {
        xprintf("[init_device] emmc_cmd3_set_rca failed\n");
        return EMMC_NG;
    }

    return EMMC_OK;
}

/*  Data Transfer Mode */
static int init_transfer_mode(void)
{
    int res = 0;

    DBG_MSG_PUTS("\n<<< Start : init_transfer_mode() >>>\n");
    
    /* Change state from stand-by to transfer */
    DBG_MSG_PUTS("[init_transfer_mode] emmc_cmd7_select_card_stby_tans\n");
    res = emmc_cmd7_select_card_stby_tans();
    if (res)
        return res;

    /* Bus Mode Configuration */
    DBG_MSG_PUTS("[init_transfer_mode] setup_bus_freq_width\n");
    res = setup_bus_freq_width(); // [DREAMER]Need to implement.
    if (res) {
        xprintf(" bus mode configuration failed!");
        return res;
    }

    /* Set-up High Capacity */
    DBG_MSG_PUTS("[init_transfer_mode] ERASE_GROUP_DEF High Capacity\n");
    res = emmc_cmd6_switch_extcsd(Idx_ExtCSD_ACC_WRB,
                     Idx_ExtCSD_ERASE_GROUP_DEF, 1);
    /* SET_BLOCKLEN to set the block length as 512 bytes. */
    DBG_MSG_PUTS("[init_transfer_mode] emmc_cmd16_set_blocklen\n");
    res = emmc_cmd16_set_blocklen(EMMC_DFLT_BLOCK_SIZE);
    if (res)
        return res;
    
    /* Clear all interrupt status bits */
    BDEV_WR(&EMMC_HOSTIF->emmc_host_int_status, HOST_INT_STATUS_ALL);
    return EMMC_OK;
}

/*
 * eMMC Initialization
 */
int emmc_initialize(void)
{
    int res;

    DBG_MSG_PUTS("\n<<< Start : emmc_initialize() >>>\n");

    /* [Step 1] Host(STB Chip) Configuration */
    /* Disable Boot Logic in chip.
     * FSBL eMMC driver run only boot from eMMC */
    DBG_MSG_PUTS("\n[Step 1] Host(STB Chip) Configuration\n");
    BDEV_WR(&EMMC_BOOT->emmc_boot_main_ctl, 0);
    while (1) {
        if ((BDEV_RD(&EMMC_BOOT->emmc_boot_status) & 1) == 0)
            break;
        cfe_usleep(10);
    }
    /* Clear any boot mode bits */
    BDEV_WR(&EMMC_HOSTIF->emmc_host_ctrl_set0, 0);

    /*
     * The early init commands are run in open drain mode where the
     * bus pull-ups are important. Broadcom chips use a value higher
     * than recommended by the spec, but the spec says the clock can be
     * slowed down to compensate. Most eMMC modules handle the default
     * speed of 400KHz, but a least some SanDisk modules only run at
     * 200KHz-300KHz. To handle this we start at 400KHz and if we get
     * errors, retry at 300KHz, 200KHz and 100KHz. Once we're out
     * of open-drain mode, the pull-ups don't matter.
     */
    DBG_MSG_PUTS("\n[Step 2] Initialize eMMC : CMD0 - CMD3\n");
    /* Initialize the eMMC device */
    res = init_device();
    if (res) {
        xprintf("eMMC error!\n");
        return EMMC_NG;
    }
    
    /* Get device into transfer mode */
    DBG_MSG_PUTS("\n[Step 3] Initialize transfer_mode\n");
    if (init_transfer_mode()) {
        xprintf(" failed to transfer mode!");
        return EMMC_NG;
    }
    DBG_MSG_PUTS("End : emmc_initialize()\n");
    return EMMC_OK;
}

#if DEBUG_EMMC_DATA
static void ui_emmc_dumphex( unsigned char *pAddr, unsigned int offset, int nLen, int sz )
{
    int a;
    unsigned char *j;
    unsigned char *crow;
    unsigned short *hrow;
    unsigned int wrow[4];
    int i;
    crow = (unsigned char *)wrow;
    hrow = (unsigned short *)wrow;
    a = 0;
    pAddr = (unsigned char *)((uintptr_t)pAddr & (~(sz-1)));
    do {
        if ((a & 15) == 0) {
            xprintf("%08x: ", offset +a);
        }
        if (a < nLen * sz) {
            switch (sz) {
            case 1:
                xprintf("%02x ",crow[a & 15] = *(unsigned char *)(pAddr+a));
                break;
            case 2:
                xprintf("%04x ",hrow[(a/2)&7] = *(unsigned short *)(pAddr+a));
                break;
            case 4:
                xprintf("%08x ",wrow[(a/4)&3] = *(unsigned int *)(pAddr+a));
                break;
            }
        } else {
            for (i = 0; i < sz*2 + 1 ; i++) {
                xprintf(" ");
            }
        }
        if (((a + sz) & 15) == 0) {
            for (i = 0, j = pAddr +(a & ~15) ; (j < pAddr + nLen*sz) && (j <= pAddr +a ) ; j++, i++) {
                xprintf("%c", (crow[i] >= ' ' && crow[i] <= '~') ? crow[i] : '.');
            }
            xprintf("\n");
        }
        a = a + sz;
    } while (a < ((nLen*sz+15) & ~15));
}
#endif
static int emmc_read(uint32_t dma_addr, uint32_t bfw_size, uint32_t emmc_addr)
{
    int res = 0;
    int num_blocks = 0;

    res = emmc_cmd6_switch_extcsd(Idx_ExtCSD_ACC_WRB,
                Idx_ExtCSD_PARTITION_CONFIG,
                PCFG_BOOT_ACK | PCFG_BOOT_PARTITION_ENABLE_BOOT1 |
                emmc_phys_partition);
    //cfe_usleep(500000);
    if (res) {
        xprintf("\n[emmc_read] Error to select %d partition!\n", emmc_phys_partition);
        return res;
    }
        
    res = wait_ready_data_xfer();
    if (res) {
        xprintf("\n[emmc_read] eMMC is not ready!\n");
        return res;
    }

    /* Adjust eMMC address if sector mode device */
    if( ocr_data & OCR_SECTOR_MODE )
        emmc_addr = emmc_addr/EMMC_DFLT_BLOCK_SIZE;

    /* Calculate number of blocks required, if we require partial block
     * then get the whole block instead. 
     * Note: This might be problematic if not enough memory has been allocated
     * by calling function. Since ther is no KMALLOC in cfe rom, we are ok
     */
    num_blocks = bfw_size/EMMC_DFLT_BLOCK_SIZE + ((bfw_size%EMMC_DFLT_BLOCK_SIZE)?1:0);
        
    res = emmc_cmd18_read_multiple_block(dma_addr, num_blocks, emmc_addr);

    if (res) {
        xprintf("\n[emmc_read] failed!\n");
        return res;
    }
#if DEBUG_EMMC_DATA
    xprintf("\n Memory data %d size\n", bfw_size);
    ui_emmc_dumphex((unsigned char *)(unsigned long)dma_addr, 0, bfw_size/16, 1);    
#endif  
    DBG_MSG_PUTS("\n[emmc_read] success!\n");
    /* Clear all interrupt status bits */
    BDEV_WR(&EMMC_HOSTIF->emmc_host_int_status, HOST_INT_STATUS_ALL);
    return EMMC_OK;
}

static int emmc_ready_boot(void)
{
    int res = 0;

    /* [Step 1] Switch BOOT partition. */
    DBG_MSG_PUTS("\n[Step 1] Switch BOOT partition\n");
    res = emmc_cmd6_switch_extcsd(Idx_ExtCSD_ACC_WRB,
                Idx_ExtCSD_PARTITION_CONFIG,
                PCFG_BOOT_ACK | PCFG_BOOT_PARTITION_ENABLE_BOOT1 |
                PCFG_PARTITION_ACCESS_BOOT1);
    cfe_usleep(500000);
    if (res) {
        DBG_MSG_PUTS("\n[emmc_ready_boot] Error to select BOOT partition!\n");
        return res;
    }

    /* [Step 2] Reset SDIO_HOST. */
    DBG_MSG_PUTS("[Step 2] Reset SDIO_HOST\n");
    if (emmc_software_reset(SDHC_SW_RESET_DAT | SDHC_SW_RESET_CMD | SDHC_SW_RESET_ALL))
        return EMMC_NG;
    /* [Step 3] Enable Boot Logic in SDIO_BOOT. */
    DBG_MSG_PUTS("[Step 3] Enable Boot Logic in SDIO_BOOT.\n");
    BDEV_WR(&EMMC_BOOT->emmc_boot_main_ctl, 1);
    while (1) {
        if ((BDEV_RD(&EMMC_BOOT->emmc_boot_status) & 1) == 1)
            break;
        cfe_usleep(10);
    }
    return EMMC_OK;
}




/************************************************************
 *          CFEROM eMMC Boot related  Functions             *
 ************************************************************/

/* Defines and types */
#define EMMC_SDMA_ALIGN_ADDR  ((1<<(HOST_BUF_SIZE+2)) * 1024)
#define EMMC_SDMA_MAX_BYTES   ((1<<(HOST_BUF_SIZE+2)) * 1024)
#define ALIGN_SDMA(x)       (((unsigned long)x + EMMC_SDMA_ALIGN_ADDR - 1 ) & ~((unsigned long)EMMC_SDMA_ALIGN_ADDR-1))
#define ALIGN_SDMA_MIN(x)   (((unsigned long)x + EMMC_DMA_BUF_ALIGN - 1 ) & ~((unsigned long)EMMC_DMA_BUF_ALIGN-1))
#define CHK_EMMC_CFE_PART(x,y)        (strcmp((x + (strlen(x) - strlen(y))), y) == 0)

/* Global Variables */
BOOT_INFO bootInfo;

/* Local Variables */
static unsigned char * dma_staging_buffer = 0;
static unsigned char * temp_data_buffer = 0;;
static int emmc_initialized = 0;
static cfe_gpt_probe_t cfe_gpt_probe; 

/* Externs */

/* Functions */

static void cferom_emmc_select_phys_part( int partition_sel ) 
{
    emmc_phys_partition = partition_sel;
}

int cferom_emmc_read_full_blocks( int handle_not_used, unsigned long long byte_offset, unsigned char * dest_buf, int length )
{
    int res = 0;
    
    /* Add offset of partition from start of flash to the actual 
     * required byte offset within the partition 
     */
    uint32_t emmc_addr = byte_offset;
    uint32_t dst_addr = (uint32_t)(unsigned long)dest_buf;
    uint32_t rem_size = length; 
    
    /* Check if emmc_addr is emmc block aligned, if not then return error */
    if( emmc_addr & (EMMC_DFLT_BLOCK_SIZE-1) )
    {
        xprintf("rom_emmc: emmc_addr 0x%08x not aligned to %d byte block boundary\n", emmc_addr, EMMC_DFLT_BLOCK_SIZE);
        return -1;
    }
    
#if DEBUG_EMMC_BOOT    
    xprintf("rom_emmc: Offset %d, length: %d\n", emmc_addr, length);
#endif

    while (rem_size > EMMC_SDMA_MAX_BYTES && !res)
    {
        res = emmc_read(dst_addr, EMMC_SDMA_MAX_BYTES, emmc_addr);
        rem_size -= EMMC_SDMA_MAX_BYTES;
        emmc_addr += EMMC_SDMA_MAX_BYTES;
        dst_addr += EMMC_SDMA_MAX_BYTES;
    }
    
    if (!res && rem_size)
        res = emmc_read(dst_addr, rem_size, emmc_addr );
    
    return res;
}

int cferom_emmc_read_full_boot_blocks( unsigned long long byte_offset, unsigned char * dest_buf, int length )
{
    int res;
    
#if DEBUG_EMMC_BOOT    
    xprintf("rom_emmc: Offset %d, length: %d\n", byte_offset, length);
#endif
    /* Select the physical BOOT partition */
    cferom_emmc_select_phys_part( PCFG_PARTITION_ACCESS_BOOT1 ); 
    /* Read from partition */
    res = cferom_emmc_read_full_blocks( 0, byte_offset, dest_buf, length );
    /* Reset partition selection to the physical DATA partition */
    cferom_emmc_select_phys_part( PCFG_PARTITION_ACCESS_DATA ); 
#if DEBUG_EMMC_BOOT    
    xprintf("rom_emmc: res: %d\n", res);
#endif
    return res;
}

int64_t get_offset_from_flashdev(char * flashdev)
{
    int i;
    uint64_t byte_offset = 0;
    for( i=0; i<cfe_gpt_probe.num_parts; i++ )
    {
        byte_offset = cfe_gpt_probe.cfe_parts[i].fp_offset_bytes;

        if( CHK_EMMC_CFE_PART(flashdev, cfe_gpt_probe.cfe_parts[i].fp_name) )
        {
#if DEBUG_EMMC_BOOT    
            xprintf("rom_emmc: %s offset %d\n", cfe_gpt_probe.cfe_parts[i].fp_name, byte_offset   ); 
#endif      
            return byte_offset;
        }
    }
    return -1;
}

static int parse_emmc_bootfs( char * flashdev, char * filename, char * data_buffer, int * data_length )
{
    int ret = 0;
               
    char * dev_buffer = (char *)dma_staging_buffer;
    int64_t offset = get_offset_from_flashdev(flashdev);
    
    if( offset < 0 )
        return -1;
    
    ret = parse_emmc_bootfs_common( 0, offset, filename, dev_buffer, data_buffer, data_length, EMMC_DFLT_BLOCK_SIZE, &cferom_emmc_read_full_blocks);

    return ret;
}

int emmc_set_boot_partition_choice(unsigned long rom_param, char * data_buffer)
{
    int boot_prev = 0;
    int ret;
    int blen = 0;
    char * bootline;
    NVRAM_DATA * nd = (NVRAM_DATA *)data_buffer;
    int64_t nvram_partition_offset = get_offset_from_flashdev(EMMC_PNAME_STR_NVRAM);
    
    if( nvram_partition_offset < 0 )
        return -1;

    if( (ret = cferom_emmc_read_full_blocks( 0, nvram_partition_offset, (unsigned char*)nd, sizeof(NVRAM_DATA))) == 0)
    {
        bootline = nd->szBootline;
        /* search for p='x' */
        if(rom_param&NAND_IMAGESEL_OVERRIDE)  
        {
            xprintf("rom_emmc: Selecting old image due to early-key-abrt selection!\n");
            boot_prev = 1;
        } 
#if defined(_BCM96858_) || defined(_BCM96848_) || defined(_BCM96838_) || defined(_BCM96846_) || defined(_BCM96856_)        
        else if (BOOT_INACTIVE_IMAGE_ONCE_REG & BOOT_INACTIVE_IMAGE_ONCE_MASK)
        {
            boot_prev = 1;
        }
#endif        
        else 
        {
            while( *bootline && blen < NVRAM_BOOTLINE_LEN )
            {
                if ((*bootline == 'p') && (*(bootline+1) == '='))
                {
                    boot_prev = (*(bootline+2) == '1') ? 1:0;
                    if( boot_prev )
                        xprintf("rom_emmc: Selecting old image due to bootline selection!\n");
                    break;
                }
                else
                {
                    bootline++;
                    blen++;
                }
            }
        }

#if defined(_BCM96858_) || defined(_BCM96848_) || defined(_BCM96838_)        
        BOOT_INACTIVE_IMAGE_ONCE_REG &= ~BOOT_INACTIVE_IMAGE_ONCE_MASK;
#endif        

        if( boot_prev )
            bootInfo.bootPartition = BOOT_SET_OLD_IMAGE;
        else
            bootInfo.bootPartition = BOOT_SET_NEW_IMAGE;
    }
    
#if DEBUG_EMMC_BOOT    
    xprintf("rom_emmc: Boot_prev: %d\n", boot_prev);
#endif    
    return ret;
}

int emmc_load_secure_image(char* image, int secCfeRamSize, unsigned char** entry, cfe_rom_media_params *media_params)
{
    uint8_t iv[CIPHER_IV_LEN];
    uint8_t* pucDest;
    uint8_t* pucEntry;
    uint8_t *pEncrCfeRam;
    int ret;
    int sec_should_decrypt;
    int sec_should_decompress;
#ifdef  CONFIG_CFE_SUPPORT_HASH_BLOCK
    sec_should_decrypt = (media_params->boot_file_flags & BOOT_FILE_FLAG_ENCRYPTED) ? 1 : 0 ;
    sec_should_decompress = (media_params->boot_file_flags & BOOT_FILE_FLAG_COMPRESSED) ? 1 : 0 ;
#else
    sec_should_decrypt = 1;
    sec_should_decompress = 1;
#endif        

    /* Retrieve the security materials */
    Booter1Args* sec_args = cfe_sec_get_bootrom_args();
    if (!sec_args)
        die();

    /* If secure boot, compressed, encrypted CFE RAM
       is authenticated within internal memory*/
    pucDest = (unsigned char *)BTRM_INT_MEM_ENCR_COMP_CFE_RAM_ADDR;

    /* For these chips we are going to authenticate CFE RAM in DDR, so we already copied it there when it was first retrieved from emmc */ 
#if !defined(_BCM94908_) && !defined(_BCM96858_) && !defined(_BCM963158_) && !defined(_BCM96856_) && !defined(_BCM96846_) && !defined(_BCM947622_)
    /* Copy encypted compressed CFE RAM to internal memory for authentication */
    memcpy(pucDest, image, secCfeRamSize);
#endif    
    
    /* Authenticate the CFE RAM bootloader */
    board_setleds(0x42544c3f); /* BTL? */
    
#ifdef  CONFIG_CFE_SUPPORT_HASH_BLOCK
    if (media_params->boot_file_hash_valid) 
    {
        /* Verify that sha256 hash of cferam matches hash retreived from hash block */
        if (sec_verify_sha256((uint8_t const*)pucDest, secCfeRamSize, (const uint8_t *)media_params->boot_file_hash)) 
        {
            xprintf("Digest failed\n");
            die();
        } 
        else 
        {
            xprintf("Digest has been succesfully matched\n");
        }
        pEncrCfeRam = pucDest; 
    } 
    else 
#endif        
    {
        /* Verify the signature located right before the cferam image */
        if (sec_verify_signature((uint8_t const*)(pucDest+SEC_S_MODULUS), secCfeRamSize-SEC_S_MODULUS, pucDest, sec_args->authArgs.manu)) {
            die();
        }
        pEncrCfeRam = pucDest+SEC_S_MODULUS; 
    }
    board_setleds(0x42544c41); // BTLA
    board_setleds(0x50415353); // PASS
    
    /* Move pucDest to point to where the authenticated and decrypted (but still compressed) CFE RAM will be put */
    pucDest = (unsigned char *)(BTRM_INT_MEM_COMP_CFE_RAM_ADDR); 

    /* Get ready to decrypt the CFE RAM bootloader */
    /* decryptWithEk() will whack the content of the iv structure, therefore create a copy and pass that in */
    memcpy((void *)iv, (void *)sec_args->encrArgs.biv, CIPHER_IV_LEN);
    
    /* Decrypt the CFE RAM bootloader */
    if (sec_should_decrypt) 
    {
        decryptWithEk(pucDest, (uint8_t *)(&pEncrCfeRam[0]), sec_args->encrArgs.bek, (uint32_t)(secCfeRamSize-SEC_S_SIGNATURE), iv);
    } 
    else 
    {
        memcpy(pucDest, (unsigned char *)(&pEncrCfeRam[0]), secCfeRamSize);
    }

    /* The reference sw is done with the bek/biv at this point ... cleaning it up */
    /* Any remnants of the keys on the stack will be cleaned up when cfe_launch() runs */
    cfe_sec_reset_keys();
    memset((void *)iv, 0, CIPHER_IV_LEN);
    
    /* First 12 bytes are not compressed ... First word of the 12 bytes is the address the cferam is linked to run at */
    /* Note: don't change the line below by adding uintptr_t to make it arch32 and arch64 compatible */
    /* you want it to grab only the first 4 bytes of the 12 bytes in both cases */
    pucEntry = (unsigned char *) (unsigned long)(*(uint32_t *)BTRM_INT_MEM_COMP_CFE_RAM_ADDR);
    
    /* decompress or copy RAM+12 for RAM+8 bytes to Entry point , depending on flag */
    if (sec_should_decompress ) 
    {
        /* Decompress the image */
        ret = decompressLZMA((unsigned char *)(BTRM_INT_MEM_COMP_CFE_RAM_ADDR+12), 
                             (unsigned int)(*(uint32_t *)(BTRM_INT_MEM_COMP_CFE_RAM_ADDR + 8)),
                             pucEntry, 23*1024*1024);
        xprintf("Decompress returns code = %d\n",ret); 
    } 
    else 
    {
        memcpy(pucEntry,((unsigned char *)BTRM_INT_MEM_COMP_CFE_RAM_ADDR) + 12,(unsigned int)(*(uint32_t *)(BTRM_INT_MEM_COMP_CFE_RAM_ADDR + 8)));
        xprintf("no decompress -- copied to %p \n",pucEntry); 
    }
        
    *entry = pucEntry; 
    return 0; 
}

void emmc_run_image( char * loaded_image, int image_size, int boot_secure , int image_num, unsigned long rom_param, cfe_rom_media_params *media_params )
{
    unsigned char *pucEntry;
    unsigned char *pucDest = NULL;


#if defined(_BCM94908_) || defined(_BCM96858_) || defined(_BCM963158_) || \
        ((INC_BTRM_BOOT==1) && defined(_BCM963138_)) || defined(_BCM96856_)
    if (boot_secure) 
    {
       emmc_load_secure_image(loaded_image, image_size, &pucEntry, media_params);
    } 
    else
#endif
    {
       /* Copy over load address from loaded cferam image */
       memcpy(&pucEntry, loaded_image, 4);
       pucDest = pucEntry - 12;
       /* Copy over loaded image to actual load address */
       memcpy(pucDest, loaded_image, image_size);
    }

#if DEBUG_EMMC_BOOT    
    xprintf("rom_emmc: load address %p, bsec %d, img_idx %d, romp %ul\n", pucEntry, boot_secure, image_num, rom_param);
#endif    

    if( pucEntry )
    {
        board_setleds(0x454d4d35); // EMM5

        /* 63138 and 63148 use 16KB below the cfe ram image as the mmu table,
         * so rfs number, cfe number has to be saved 16K down further */
        
        /* Save the rootfs partition that the CFE RAM image boots from
         * at the memory location before the CFE RAM load address. The
         * CFE RAM image uses this value to determine the partition to
         * flash a new rootfs to.
         */
        ROM_PARMS_SET_ROOTFS(pucEntry,(unsigned char)image_num);

#if 0        
        /* TODO: Figure out if we need this */
        /* Save the sequence numbers so the CFE RAM image does not have
         * to find them again.
         */
        ROM_PARMS_SET_SEQ_P1(pucEntry, (rfs_info[0].boot_val & 0xffff) | NAND_SEQ_MAGIC);
        ROM_PARMS_SET_SEQ_P2(pucEntry, (rfs_info[1].boot_val & 0xffff) | NAND_SEQ_MAGIC);
#endif
        if( bootInfo.bootPartition == BOOT_SET_OLD_IMAGE )
        {
            ROM_PARMS_SET_ROM_PARM(pucEntry, ((unsigned int)BOOT_SET_OLD_IMAGE)|NAND_IMAGESEL_OVERRIDE);
        } 
        else 
        {
            ROM_PARMS_SET_ROM_PARM(pucEntry, 0);
        }
                
#if defined(_BCM94908_) || defined(_BCM96858_) || defined(_BCM96856_) || defined(_BCM963158_) || ((INC_BTRM_BOOT==1) && defined(_BCM963138_))
        if (boot_secure)
        {
           Booter1Args* bootArgs = cfe_sec_get_bootrom_args();
           if (!bootArgs) 
              die();
           
           /* Copy the authentication credentials from internal memory 
            * into ddr. Cferam on some targets (6838) uses the internal
            * memory. Therefore, linux kernel authentication has to use 
            * these credentials from ddr. Put the data above the 12 bytes
            * of info that were just placed above the cferam */
           ROM_PARMS_AUTH_PARM_SETM(pucEntry, &bootArgs->authArgs);
        }
#endif
        cfe_launch((unsigned long) pucEntry); // never return...
    }
    board_setleds(0x454d4d38); // EMM8

    /* Error occurred. */
#if defined(_BCM94908_) || defined(_BCM96858_) || defined(_BCM963158_) || defined(_BCM96856_) || \
            ((INC_BTRM_BOOT==1) && defined(_BCM963138_))
    if (boot_secure)
    {
       Booter1Args* bootArgs = cfe_sec_get_bootrom_args();
       if (!bootArgs) 
          die();
       
       /* Customer should zero out the bek and the biv at this point because the CFE RAM   */
       /* was not found. Hence, the flash is toast and JTAG needs to be enabled.           */
       ek_iv_cleanup(&bootArgs->encrArgs);

       /* Cleanup internal memory, and set the external interface enable bit high so that  */
       /* interfaces such as JTAG are suppose to be accessible, they become enabled        */
       cfe_launch(0);
    }
#endif
    board_setleds(0x44494530); // DIE0
    while(1);
}

extern void _cfe_flushcache(int);

int rom_emmc_get_nvram_memcfg(uint32_t * memcfg, unsigned char * dma_virt_addr )
{
    int ret = 0;
    int nvram_partition_offset_bytes = 0;
    unsigned long long dma_buffer_size = 0;
    unsigned char * dma_phys_addr = (unsigned char*)(uintptr_t)VA_TO_PHYS_SIZE((uintptr_t)dma_virt_addr, &dma_buffer_size);

    if( dma_buffer_size >= sizeof(NVRAM_DATA) )
    {
        /* Note: This is basically a Hail Mary. Here NVRAM memory config
         * is required before DDR init, we therefore dont have a lot of memory
         * to play with, so we skip the fetching/parsing of GPT tables and just
         * assume that there is a valid NVRAM in the first ALIGNED partition.
         * We then use hardcoded internal SRAM virt/phys addresses to DMA the data
         * over. This also puts in 2 design constraints
         * (1) The nvram logical partition always has to be the first partition 
         * after the primary GPT table. 
         * (2) The ALIGNMENT value has to be larger than the size of all GPT related
         * headers
         */

        nvram_partition_offset_bytes = EMMC_PART_ALIGN_BYTES;

       /* Flush d cache */
        _cfe_flushcache(0);

        /* Get value at offset */
        ret = cferom_emmc_read_full_blocks( 0, nvram_partition_offset_bytes,  
                    (emmc_dma_enable?dma_phys_addr:dma_virt_addr), sizeof(NVRAM_DATA));

        /* Get MCB selector */
        *memcfg = *((uint32_t *)(dma_virt_addr + 
                    offsetof(NVRAM_DATA,ulMemoryConfig)));   
        xprintf("rom_emmc: MCB from NVRAM: 0x%08x\n", *memcfg);

    }
    else
    {
        xprintf("rom_emmc: Not enough contiguous dma memory!\n");
        ret = -1;
    }

    return ret;
}


int rom_emmc_init(void)
{

#if (CFG_ROM_PRINTF==1)
    xprinthook = board_puts;
#endif

#if defined(_BCM963158_)
    if((PERF->RevID&REV_ID_MASK) == 0xA0)
    {
        EMMC_HOSTIF = (volatile EmmcHostIfRegs *) EMMC_HOSTIF_BASE_A0;
        EMMC_TOP_CFG = (volatile EmmcTopCfgRegs *) EMMC_TOP_CFG_BASE_A0;
        EMMC_BOOT = (volatile EmmcBootRegs *) EMMC_BOOT_BASE_A0;
    }
#endif

    if(emmc_initialize() == EMMC_OK)
    {
        /* Set initialized flag */
        emmc_initialized = 1;
        xprintf("rom_emmc: emmc initialized!\n");
        return 0;
    }
    else
    {
        xprintf("rom_emmc: emmc initializiation failed!\n");
        return -1;
    }
}

int rom_emmc_gpt_run( void )
{
    /* Look for GPT partitions */
     
    /* setup gpt probe */
    cfe_gpt_probe.block_size = EMMC_DFLT_BLOCK_SIZE;

    /* configure read/write functions */
    cfe_gpt_probe.read_func = &cferom_emmc_read_full_blocks;
    cfe_gpt_probe.write_func = NULL;

    /* configure offsets to prim/backup GPT headers */
    cfe_gpt_probe.offset_prim = 0;

    /* Configure scratch memory */
    cfe_gpt_probe.ptr_gpt_prim = temp_data_buffer;
    cfe_gpt_probe.ptr_gpt_back = temp_data_buffer + CFE_GPT_PRIMRY_SIZE;

    /* Configure file handle - Set to 0 for cferom */
    cfe_gpt_probe.fd_prim = 0; 
    cfe_gpt_probe.fd_back = 0;

    /* Putting in dummy values, since at this point we do not know the entire 
     * size of the device. We will be retrieving that info from the valid PMBR
     * in the primary GPT that is hopefully intact and present on the device
     */
    cfe_gpt_probe.lba_size = 0;
    cfe_gpt_probe.offset_back = 0;

#if DEBUG_EMMC_BOOT    
    xprintf("rom_emmc: Starting GPT init!\n");
#endif    

    if (cfe_gpt_run( &cfe_gpt_probe) != EMMC_OK) 
    {
        xprintf("rom_emmc: GPT init failed!\n");
        return EMMC_NG;
    }
    xprintf("rom:emmc: GPT init success!\n");

#if DEBUG_EMMC_BOOT    
    int i;
    int byte_offset = 0;
    for( i=0; i<cfe_gpt_probe.num_parts ; i++)
    {
        xprintf("CFE partition: %s (%u bytes) @ offset 0x%x\n",   cfe_gpt_probe.cfe_parts[i].fp_name, 
                                                    cfe_gpt_probe.cfe_parts[i].fp_size,
                                                    byte_offset );
        byte_offset += cfe_gpt_probe.cfe_parts[i].fp_size;
    }
#endif    
    return EMMC_OK;
}

#ifdef CONFIG_CFE_SUPPORT_HASH_BLOCK
static void rom_emmc_get_hashblock( int * bootImg, int * imageNum, cfe_rom_media_params * media_params)
{
    int file_length = 0;

    if (media_params->boot_secure)
    {
        const uint8_t *pHashes = (const uint8_t *)BTRM_INT_MEM_ENCR_COMP_CFE_RAM_ADDR;
        file_length = load_file_from_next_bootfs( media_params->hash_file_name, (char*)pHashes, bootImg, imageNum, &parse_emmc_bootfs );
    
        if (file_length > SEC_S_MODULUS) 
        { 
            Booter1Args* sec_args = cfe_sec_get_bootrom_args();
            xprintf("authenticate...");
            // authenticate(pHashes, res, sec_args->authArgs.manu);
            if (sec_verify_signature((uint8_t const*)(&pHashes[SEC_S_MODULUS]), file_length-SEC_S_MODULUS, &pHashes[0], sec_args->authArgs.manu)) 
            {
                xprintf("..FAIL\n");
                die();
            } 
            else 
            {
                xprintf("..success\n");
                parse_boot_hashes((char *)&pHashes[SEC_S_MODULUS], media_params);
                xprintf("got bootable cferam %s\n",media_params->boot_file_name);
            }
        }
    }
}
#endif
                
int rom_emmc_boot(unsigned char * dma_addr, unsigned long rom_param, cfe_rom_media_params *media_params)
{
    int bootImg=0; 
    int imageNum=0 ;
    int file_length = 0;
    /* static because of stack overrun issues */

    /* 0-Setup dma_staging_buffer, temp_data_buffer and enable DMA now that DDR is available */
    dma_staging_buffer = (unsigned char *)(ALIGN_SDMA(dma_addr));
    temp_data_buffer   = dma_staging_buffer + 1024*1024;
    emmc_dma_enable    = 1;
#if DEBUG_EMMC_BOOT    
    xprintf("dma_staging_buffer 0x%08x!\n", dma_staging_buffer);
    xprintf("temp_data__buffer  0x%08x!\n", temp_data_buffer);
#endif    

    /* 1-Check if Initialized eMMC device */
    if (emmc_initialized) 
    {
        /* 2-Look for GPT partitions */
        if( rom_emmc_gpt_run() != EMMC_OK )
            return EMMC_NG;        

        /* 3-Update bootinfo variable with boot partition choice from NVRAM */
        emmc_set_boot_partition_choice(rom_param, (char*)temp_data_buffer);

        /* 3.1-Get hashblock if needed */
#ifdef CONFIG_CFE_SUPPORT_HASH_BLOCK
        rom_emmc_get_hashblock( &bootImg, &imageNum, media_params);
#endif

#if defined(_BCM94908_) || defined(_BCM96858_) || defined(_BCM963158_) || defined(_BCM96856_) || defined(_BCM96846_) || defined(_BCM947622_)
	/* For these chips we authenticate secure cferam in DDR (instead of SRAM), so copy it directly there in the first place */
	if(media_params->boot_secure)
	    temp_data_buffer = (unsigned char *)BTRM_INT_MEM_ENCR_COMP_CFE_RAM_ADDR;
#endif

        /* 4-Load cferam into temporary memory */
        if( (file_length = load_file_from_next_bootfs( media_params->boot_file_name, (char*)temp_data_buffer, &bootImg, &imageNum, &parse_emmc_bootfs )) < 0 )
        {
            xprintf("rom_emmc: unable to load file %s\n", media_params->boot_file_name);
            return EMMC_NG;
        }

        /* 5-Put eMMC device into idle mode */
        if (emmc_ready_boot() == EMMC_OK) 
        {
            xprintf("rom_emmc: emmc_ready_boot\n");
        }
        else 
        {
            xprintf("rom_emmc: emmc_ready_boot failed!\n");
            return EMMC_NG;
        }

        /* 6-Run loaded image */
        emmc_run_image( (char*)temp_data_buffer, file_length, media_params->boot_secure, imageNum, rom_param, media_params);

        /* Should never get here */
        return EMMC_OK;
    }
    else 
    {
        xprintf("rom_emmc: emmc not initialized!\n");
        return EMMC_NG;
    }
}

