/***************************************************************************
 <:copyright-BRCM:2016:DUAL/GPL:standard
 
    Copyright (c) 2016 Broadcom 
    All Rights Reserved
 
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

#include "lib_types.h"
#include "lib_malloc.h"
#include "lib_printf.h"
#include "lib_string.h"
#include "cfe_iocb.h"
#include "cfe_devfuncs.h"
#include "cfe_ioctl.h"
#include "cfe_error.h"
#include "cfe_timer.h"
#include "env_subr.h"
#include "dev_emmcflash.h"
#include "bcm_map_part.h"
#include "addrspace.h"
#include "shared_utils.h"

//----------------------------------------------------------
#define ALIGN_INC(x, a)             (((x) + (a) - 1) & ~((a) - 1))
#define ALIGN_DEC(x, a)              (x & ~((a) - 1))
// Driver Definition
// Status definition
#define EMMC_STATUS_ERROR   0x10    // Reserved bit on CST(Device Status)
#define EMMC_STATUS_OK      0x00
#define EMMC_OK             0x01
#define EMMC_NG             0x00
#define EMMC_NO_ERROR       0
#define EMMC_ERROR          -1

// eMMC partition support
#define EMMC_PART_SUP_EN    0x01
#define EMMC_PART_SUP_ENH   0x02
#define EMMC_PART_SUP_EXT   0x04

// For Time Information
#define EMMC_TICKS_10NS     3
#define EMMC_TICKS_USEC     163

// For Sector buffer
#define EMMC_MAX_ONE_BLOCK_SIZE     4096   // 4K for v4.5 & over 256GB eMMC, Max Size
#define ARASAN_MAX_ONE_BLOCK_SIZE   512     // Block Size
#define ARASAN_MAX_BLOCKS           65535
#define HOST_SDMA_BUFF_BOUNDARY     7       // 0:4K, 1:8K, 2:16K, 3:32K, 4:64K, 5:128K, 6:256K, 7:512K 
#define EMMC_UNALIGNED_DMA          1

#if EMMC_UNALIGNED_DMA
/* Unaligned SDMA scheme: Host dma buffer does not need to be aligned to any arbitrary boundary */
#define EMMC_SDMA_ADDR_ALIGN        ((1<<(HOST_SDMA_BUFF_BOUNDARY+2)) * 1024)
#else
/* Aligned SDMA scheme */
#define ARASAN_SDMA_MAX_BLKS        512  // 4:2K, 8:4K, 512:256K, 1024:512K : 2^(HOST_SDMA_BUFF_BOUNDARY+2), ARASAN_SDMA_MAX_BLKS * 2 = HOST_SDMA_BUFF_BOUNDARY
#define MAX_ALIGNED_ADDR            ARASAN_SDMA_MAX_BLKS*ARASAN_MAX_ONE_BLOCK_SIZE  // 0x00040000, 256K
#define MAX_ALIGNED_ADDR_MASK       MAX_ALIGNED_ADDR-1                              // 0x0003FFFF, 256K

#if EMMC_ALIGN_256K    
/* Force Aligned SDMA scheme */
#define EMMC_STAGING_BUFFER_4_OFFSET    0x86FC0000  // 0x8700_0000 - 256KB to support offset options of flash command   
#define EMMC_SDMA_ALIGNEMENT            0x00040000  // 256K
#define EMMC_STAGING_BUFFER_OFFSET      0x83800000  // 56MB of 112MB(0x87000000) : half of total staging buffer
#define EMMC_NON_ALIGNED_MAX_SIZE       0x03700000  // 55MB
#endif
#endif

#define SDIO_HOST_INT_CLR_ALL           0xFFFFFFFF  //0x13FF063F : Enabled INT
#define EMMC_DRV_FUTURE                 EMMC_OFF    // Value always is EMMC_OFF. Change code to use.
//----------------------------------------------------------

#if !defined(GET_REG_FIELD) && !defined(SET_REG_FIELD)
#define REG_MASK(m)    m##_MASK
#define REG_SHIFT(m)   m##_SHIFT

#define GET_REG_FIELD(reg,m) ((((reg) =  ((reg) &  REG_MASK(m)) >> REG_SHIFT(m))))
#define SET_REG_FIELD(reg,m,d) ((reg) = (((reg) & ~REG_MASK(m)) | ((((d) ) << REG_SHIFT(m)) & REG_MASK(m))) )
#endif

#define EMMC_GET_REG_FIELD(field,m,reg) ((((field) = ((reg) & REG_MASK(m)) >> REG_SHIFT(m))))

#define PHYS_TO_K1b(x)  x
#define K1_TO_PHYSb(x)  x

//----------------------------------------------
#define EMMC_MID_SANDISK      0x2
#define EMMC_MID_TOSHIBA      0x11
#define EMMC_MID_MICRON       0x13
#define EMMC_MID_SAMSUNG      0x15

//----------------------------------------------------------
// extern Variable & Function Declaration

#if defined(_BCM963158_)  /* temporary change to support runtime detection for A0 and B0 chips */
#undef EMMC_HOSTIF
#undef EMMC_TOP_CFG
#undef EMMC_BOOT
volatile EmmcHostIfRegs* EMMC_HOSTIF = (volatile EmmcHostIfRegs *) EMMC_HOSTIF_BASE;
volatile EmmcTopCfgRegs* EMMC_TOP_CFG = (volatile EmmcTopCfgRegs *) EMMC_TOP_CFG_BASE;
volatile EmmcBootRegs* EMMC_BOOT = (volatile EmmcBootRegs *) EMMC_BOOT_BASE;
#endif

extern uint64_t cfe_flash_size;
#if EMMC_PARTITION_CTRL
extern void master_reboot(void);
#endif
//----------------------------------------------------------


//----------------------------------------------------------
// Function Declaration for cfe_driver_t emmcflashdrv & cfe_devdisp_t emmcdrv_dispatch
static int  emmcdrv_open(cfe_devctx_t *ctx);
static int  emmcdrv_read(cfe_devctx_t *ctx,iocb_buffer_t *buffer);
static int  emmcdrv_inpstat(cfe_devctx_t *ctx,iocb_inpstat_t *inpstat);
static int  emmcdrv_write(cfe_devctx_t *ctx,iocb_buffer_t *buffer);
static int  emmcdrv_ioctl(cfe_devctx_t *ctx,iocb_buffer_t *buffer) ;
static int  emmcdrv_close(cfe_devctx_t *ctx);
//----------------------------------------------------------

//----------------------------------------------------------
// Function Declaration 2
static void emmcdrv_probe(cfe_driver_t *drv, unsigned long probe_a, unsigned long probe_b, void *probe_ptr);
static void emmc_do_probe(emmcflashdev_t *softc);
static void emmc_do_parts(emmcflashdev_t *softc);
static int emmc_get_config( emmcflashdev_t *softc );
//uint64_t emmc_get_dev_phy_size( emmcflash_probe_t *fd_probe );    // in 'dev_emmcflash.h'
//uint64_t emmc_get_dev_log_size( emmcflash_probe_t *fd_probe );    // in 'dev_emmcflash.h'
int32_t emmc_get_block_size ( emmcflash_probe_t *fd_probe );
static int emmc_erase_range ( emmcflashdev_t *softc, flash_range_t *range );

static int emmcdrv_read_write( cfe_devctx_t *ctx, iocb_buffer_t *buffer, uint8_t read_write );
uint32_t emmc_block_read( emmc_data_descriptor_t data_descrptr, emmcdev_t *emmc_config, emmcflash_probe_t *emmc_probe );
uint32_t emmc_block_write( emmc_data_descriptor_t data_descrptr, emmcdev_t *emmc_config, emmcflash_probe_t *emmc_probe );
static int64_t emmc_get_range_intersection ( uint64_t range_base, uint64_t range_len, uint32_t blk_size, uint8_t blk_length_bit, emmc_range_descriptor_t* rdt);

//void emmc_set_device_name( void );            // in 'dev_emmcflash.h'

#if EMMC_DRV_FUTURE
int32_t emmc_erase_block (uint32_t blk_addr, uint32_t blk_size);
static int emmc_erase_all ( emmcflashdev_t *softc );
#endif
//----------------------------------------------------------

//----------------------------------------------------------
// Function Declaration 3 : Dreamer
//----------------------------------------------------------
// eMMC Macro Commands
//int emmc_Initialize( emmcdev_t *emmcdev );            // in 'dev_emmcflash.h'
void emmc_config_init( emmcdev_t *emmcdev );
void emmc_Host_Config( emmcdev_t *emmcdev );
void emmc_Ctrl_PinMux( void );
void emmc_Boot_Mode( emmcdev_t *emmcdev, uint32_t eMMC_Boot_Clk );
static int emmc_DeviceID_Mode_SingleDevice( emmcdev_t *emmcdev );
void emmc_Setup_DataTransfer_Mode( emmcdev_t *emmcdev );
void emmc_Setup_BusFreqWidth( emmcdev_t *emmcdev );
//int emmc_Partition_DataArea( emmcdev_t *emmcdev );    // in 'dev_emmcflash.h'
int emmc_Sel_Partition( emmcdev_t *emmcdev, uint32_t partition_sel );
//----------------------------------------------------------

//----------------------------------------------------------
// eMMC Commands
// Class 0 Basic Commands
uint32_t emmc_CMD0_C0ba_bc_GO_IDLE_STATE( emmcdev_t *emmcdev );
uint32_t emmc_CMD0_C0ba_bc_GO_PRE_IDLE_STATE( emmcdev_t *emmcdev );
uint32_t emmc_CMD0_C0ba_bc_BOOT_INITIATION( emmcdev_t *emmcdev );
uint32_t emmc_CMD0_C0ba_bc( uint32_t cmd0_cmd, emmcdev_t *emmcdev );
uint32_t emmc_CMD1_C0ba_bcrR3_SEND_OP_COND( uint32_t arg_OCR, emmcdev_t *emmcdev );
uint32_t emmc_CMD2_C0ba_bcrR2_ALL_SEND_CID_SingleDevice( emmcdev_t *emmcdev );
uint32_t emmc_CMD3_C0ba_acR1_SET_RCA( uint32_t arg_RCA, emmcdev_t *emmcdev );
uint32_t emmc_CMD4_C0ba_bc_SET_DSR( uint32_t arg_DSR, emmcdev_t *emmcdev );
#if EMMC_DRV_FUTURE
uint32_t emmc_CMD5_C0ba_acR1b_SLEEP_AWAKE( uint32_t arg_RCA, uint32_t arg_SleepAwake, emmcdev_t *emmcdev );
#endif
uint32_t emmc_CMD6_C0ba_acR1b_SWITCH_ExtCSD( uint32_t arg_Access, uint32_t arg_Index, uint32_t arg_Val, emmcdev_t *emmcdev );
#if EMMC_DRV_FUTURE
uint32_t emmc_CMD6_C0ba_acR1b_SWITCH_CmdSet( uint32_t arg_CmdSet, emmcdev_t *emmcdev );
#endif
uint32_t emmc_CMD7_C0ba_acR1_SELECT_CARD_stby_tans( uint32_t arg_RCA, emmcdev_t *emmcdev );
#if EMMC_DRV_FUTURE
uint32_t emmc_CMD7_C0ba_acR1b_SELECT_CARD_dis_prg( uint32_t arg_RCA, emmcdev_t *emmcdev );
#endif
uint32_t emmc_CMD7_c0ba_acR1R1b_SELECT_CARD( uint32_t arg_RCA, uint32_t arg_cmd_mode, emmcdev_t *emmcdev );
#if EMMC_DRV_FUTURE
uint32_t emmc_CMD7_C0ba_acR1_DESELECT_CARD_ALL_stby_tans( emmcdev_t *emmcdev );
uint32_t emmc_CMD7_C0ba_acR1b_DESELECT_CARD_ALL_dis_prg( emmcdev_t *emmcdev );
uint32_t emmc_CMD7_c0ba_acR1R1b_DESELECT_CARD_ALL( uint32_t arg_cmd_mode, emmcdev_t *emmcdev );
#endif
uint32_t emmc_CMD8_C0ba_adtcR1_SEND_EXT_CSD( emmcdev_t *emmcdev );
uint32_t emmc_CMD9_C0ba_acR2_SEND_CSD( emmcdev_t *emmcdev );
uint32_t emmc_CMD10_C0ba_acR2_SEND_CID( emmcdev_t *emmcdev );
#if EMMC_DRV_FUTURE
uint32_t emmc_CMD12_C0ba_acR1_STOP_TRANSMISSION_Read( emmcdev_t *emmcdev );
uint32_t emmc_CMD12_C0ba_acR1b_STOP_TRANSMISSION_Write( emmcdev_t *emmcdev );
#endif
uint32_t emmc_CMD13_C0ba_acR1_SEND_STATUS( emmcdev_t *emmcdev );
uint32_t emmc_CMD14_C0ba_adtcR1_BUSTEST_R( emmcdev_t *emmcdev );
uint32_t emmc_CMD15_C0ba_ac_GO_INACTIVE_STATE( uint32_t arg_RCA, emmcdev_t *emmcdev );
uint32_t emmc_CMD19_C0ba_adtcR1_BUSTEST_W( emmcdev_t *emmcdev );
// Class 2 Bock Read Commands
uint32_t emmc_CMD16_C2br_acR1_SET_BLOCKLEN( uint32_t arg_BlockLength, emmcdev_t *emmcdev );
uint32_t emmc_CMD17_C2br_adtcR1_READ_SINGLE_BLOCK( uint32_t emmc_Addr, uint32_t DMA_Addr, uint32_t arg_BlockLength, uint32_t *read_data, emmcdev_t *emmcdev );
uint32_t emmc_CMD18_C2br_adtcR1_READ_MULTIPLE_BLOCK( uint32_t emmc_Addr, uint32_t DMA_Addr, uint32_t nBlocks, uint32_t arg_BlockLength, uint32_t *read_data, emmcdev_t *emmcdev );
// Class 4 Bock Write Commands
#if EMMC_DRV_FUTURE
uint32_t emmc_CMD23_C4bw_acR1_SET_BLOCK_COUNT( uint32_t arg_NumBlocks, uint32_t ReliableWriteReq, uint32_t TagReq, uint32_t ContextID, uint32_t ForcedPrg, emmcdev_t *emmcdev );
uint32_t emmc_CMD23_C4bw_acR1_SET_BLOCK_COUNT_packed( uint32_t arg_NumBlocks, emmcdev_t *emmcdev );
#endif
uint32_t emmc_CMD24_C4bw_adtcR1_WRITE_BLOCK( uint32_t emmc_Addr, uint32_t DMA_Addr, uint32_t arg_BlockLength, uint32_t *write_data, emmcdev_t *emmcdev );
uint32_t emmc_CMD25_C4bw_adtcR1_WRITE_MULTIPLE_BLOCK( uint32_t emmc_Addr, uint32_t DMA_Addr, uint32_t nBlocks, uint32_t arg_BlockLength, uint32_t *write_data, emmcdev_t *emmcdev );
uint32_t emmc_CMD35_C5er_acR1_ERASE_GROUP_START( uint32_t arg_StartErAddr, emmcdev_t *emmcdev );
uint32_t emmc_CMD36_C5er_acR1_ERASE_GROUP_END( uint32_t arg_EndErAddr, emmcdev_t *emmcdev );
uint32_t emmc_CMD38_C5er_acR1b_ERASE( uint32_t TrimData, uint32_t IdentifyWriteBlock, emmcdev_t *emmcdev );
#if EMMC_DRV_FUTURE
uint32_t emmc_CMD27_C4bw_adtcR1_PROGRAM_CSD( uint32_t *write_data, emmcdev_t *emmcdev );
// Class 5 Erase Commands
// Class 6 Write Protection Commands
uint32_t emmc_CMD28_C6wp_acR1b_SET_WRITE_PROT( uint32_t arg_Addr, emmcdev_t *emmcdev );
uint32_t emmc_CMD29_C6wp_acR1b_CLR_WRITE_PROT( uint32_t arg_Addr, emmcdev_t *emmcdev );
uint32_t emmc_CMD30_C6wp_adtcR1_SEND_WRITE_PROT( uint32_t arg_WpAddr, emmcdev_t *emmcdev );
uint32_t emmc_CMD31_C6wp_adtcR1_SEND_WRITE_PROT_TYPE( uint32_t arg_WpAddr, emmcdev_t *emmcdev );
// Class 7 Lock Card Commands
uint32_t emmc_CMD42_C7lc_adtcR1_LOCK_UNLOCK( emmcdev_t *emmcdev );
// Class 8 Application Specific Commands
uint32_t emmc_CMD55_C8as_acR1_APP_CMD( uint32_t arg_RCA, emmcdev_t *emmcdev );
uint32_t emmc_CMD56_C8as_adtcR1_GEN_CMD( uint32_t arg_RD_WR, emmcdev_t *emmcdev );
// Class 9 I/O Mode Commands
uint32_t emmc_CMD39_C9io_acR4_FAST_IO( emmcdev_t *emmcdev );
uint32_t emmc_CMD40_C9io_bcrR5_GO_IRQ_STATE( emmcdev_t *emmcdev );
#endif
//----------------------------------------------------------

//----------------------------------------------------------
// eMMC Sub Functions
void emmc_Enable_Host_Int( uint32_t int_status_env );
void emmc_Clear_HostIntStatus( uint32_t IntMask );
uint32_t emmc_Get_HostState( void );
uint32_t emmc_Wait_CMD_complete( uint32_t sleep_time, uint8_t cmd_num );
uint32_t emmc_Wait_BufferReadInt( uint32_t sleep_time, uint8_t cmd_num );
uint32_t emmc_Wait_BufferWriteInt( uint32_t sleep_time, uint8_t cmd_num );
uint32_t emmc_Wait_Xfer_complete( uint32_t sleep_time, uint8_t cmd_num );
uint32_t emmc_Wait_ReadyDataXfer( emmcdev_t *emmc_config, uint32_t sleep_time, uint8_t cmd_num );
uint32_t emmc_Wait_EmmcNextState( emmcdev_t *emmc_config, uint32_t next_state, uint32_t sleep_time, uint8_t cmd_num );
void emmc_Write_Blocks( uint32_t *write_data, uint32_t nBlock, uint32_t nBuffData, emmcdev_t *emmcdev );
void emmc_Read_Blocks( uint32_t *read_data, uint32_t nBlocks, uint32_t nBuffData, emmcdev_t *emmcdev );
uint32_t emmc_GetResponse07_1( uint32_t regval, uint32_t regidx, uint32_t width, uint32_t offset );
uint32_t emmc_GetResponse07_2( uint32_t reg1val, uint32_t reg0val, uint32_t reg0idx, uint32_t width, uint32_t offset );
uint32_t emmc_GetCSDEraseTimeout( emmcdev_t *emmcdev );
void emmc_Get_ExtCSD( emmcdev_t *emmcdev );
void emmc_Decode_ExtCSD_value( uint32_t Reg_Data[], uint8_t ExtCSD_Data[], uint8_t nByte, uint16_t CSD_Addr );
#if EMMC_DRV_FUTURE
void emmc_timeout_ns(uint32_t ticks);
void emmc_sleep_10ns( uint32_t sleep_time );
#endif
#if 1//DEBUG_EMMC_INIT
void emmc_Print_CID( emmcdev_t *emmcdev );
void emmc_Print_CSD( emmcdev_t *emmcdev );
void emmc_Print_ExtCSD( emmcdev_t *emmcdev );
void emmc_Print_HostAndEmmcInfo( emmcdev_t *emmcdev );
void emmc_Print_EmmcSizePartitionInfo( emmcdev_t *emmcdev );
#endif
#if DEBUG_EMMC_SLEEP_TIMER
void emmc_Check_SleepTime( void );
#endif
//----------------------------------------------------------


//----------------------------------------------------------
// Structures for emmc_drv
const static cfe_devdisp_t emmcdrv_dispatch = {
    emmcdrv_open,
    emmcdrv_read,
    emmcdrv_inpstat,
    emmcdrv_write,
    emmcdrv_ioctl,
    emmcdrv_close,
    NULL,
    NULL
};

const cfe_driver_t emmcflashdrv = {
    "EMMC",
    "emmcflash",        /* leave enough space for nandflash */
    CFE_DEV_FLASH,
    &emmcdrv_dispatch,
    emmcdrv_probe
};

typedef struct {
    int mid;
    char mname[15];
} emmc_mid_name_map_t;

emmc_mid_name_map_t emmc_mid_name_map[]={
                                            { EMMC_MID_SANDISK, "SanDisk" },
                                            { EMMC_MID_TOSHIBA, "Toshiba" },
                                            { EMMC_MID_MICRON , "Micron"  }, 
                                            { EMMC_MID_SAMSUNG, "Samsung" },   
                                            { 0, "Unknown" },   
                                        };
// Structure
//----------------------------------------------------


//===========================================================
// eMMC driver main functions
void emmc_set_device_name( void )
{
    strcpy(emmcflashdrv.drv_bootname, "emmcflash");
}


/*  *********************************************************************
    *  emmcdrv_open(ctx)
    *
    *  Called when the flash device is opened.
    *
    *  Input parameters:
    *      ctx - device context
    *
    *  Return value:
    *      0 if ok else error code
    ********************************************************************* */
static int emmcdrv_open( cfe_devctx_t *ctx )
{
    emmcflash_cfepart_cfg_t     *part  = ctx->dev_softc;
    emmcflashdev_t      *softc = part->fp_dev;
    emmcflash_probe_t   *emmc_probe = &softc->fd_probe;   
    emmcdev_t           *emmc_config = &emmc_probe->emmc_config;
    uint32_t            partition_config=0x00, return_val;

#if DEBUG_EMMC_DRV_DISPATCH
    printf("\n\n ----->>> emmcdrv_open\n\n");
    printf("      ctx->dev_dev.dev_fullname    : %s\n", ctx->dev_dev->dev_fullname);
    printf("      ctx->dev_dev.dev_class       : %d\n", ctx->dev_dev->dev_class);
    printf("      ctx->dev_dev.dev_opencount   : %d\n", ctx->dev_dev->dev_opencount);
    printf("      ctx->dev_dev.dev_description : %s\n", ctx->dev_dev->dev_description);
    printf("      emmc_probe->flash_type : %d\n", emmc_probe->flash_type);
//     printf("      emmc_probe->flash_phy_addr   : 0x%llu\n",      emmc_probe->flash_phy_addr);
    printf("      emmc_probe->flash_phy_size   : 0x%llu (%dMB)\n",  emmc_probe->flash_phy_size, (uint32_t)(emmc_probe->flash_phy_size>>20));
    printf("      emmc_probe->flash_block_size : %d\n", emmc_probe->flash_block_size);
    printf("      emmc_probe->flash_part_attr : %d\n",          emmc_probe->flash_part_attr);
    printf("      emmc_probe->flash_nparts : %d\n", emmc_probe->flash_nparts);
    printf("      emmc_probe->flash_log_size  : 0x%llu (%dMB)\n",   emmc_probe->flash_log_size, (uint32_t)(emmc_probe->flash_log_size>>20));
    printf("      part->fp_partition : %d\n",           part->fp_partition);
    printf("      part->fp_size      : 0x%llu (%dKB)\n",    part->fp_size, (uint32_t)(part->fp_size>>10));
    printf("      part->fp_offset_bytes    : 0x%llu\n",       part->fp_offset_bytes, part->fp_offset_bytes);
#endif
    
    // Set-up sleep parameters
    emmc_config->config.InterruptSleep = EMMC_INT_SLEEP;
    
#if DEBUG_EMMC_DRV_DISPATCH
    emmc_config->CST = emmc_CMD8_C0ba_adtcR1_SEND_EXT_CSD( emmc_config );
    printf(" emmc_config->ExtCSD.PARTITION_CONFIG = 0x%02X (%d)\n", emmc_config->ExtCSD.PARTITION_CONFIG, emmc_config->ExtCSD.PARTITION_CONFIG);    
#endif
                
    switch( emmc_probe->flash_part_attr )
    {
        case EMMC_NO_ACCESS_BOOT:   partition_config = EMMC_PART_DATA;    break;
        case EMMC_RW_BOOT_PART_1:   partition_config = EMMC_PART_BOOT1;   break;
        case EMMC_RW_BOOT_PART_2:   partition_config = EMMC_PART_BOOT2;   break;
        case EMMC_RW_RPMB:          partition_config = EMMC_PART_RPMB;    break;
        case EMMC_RW_GP1:           partition_config = EMMC_PART_GP1;     break;
        case EMMC_RW_GP2:           partition_config = EMMC_PART_GP2;     break;
        case EMMC_RW_GP3:           partition_config = EMMC_PART_GP3;     break;
        case EMMC_RW_GP4:           partition_config = EMMC_PART_GP4;     break;
        default:                    partition_config = EMMC_PART_DATA;    break;
    }

#if (EMMC_BOOT_PART==EMMC_BOOT_DISABLE)
    emmc_config->config.BootPartitionEnable = EMMC_BOOT_DISABLE;
#endif

#if (EMMC_BOOT_PART==EMMC_BOOT_FROM_BOOT1)
    partition_config |= 0x48;    // Boot Partition 1 enable for boot
    emmc_config->config.BootPartitionEnable = EMMC_BOOT1_4_BOOT;
#endif

#if (EMMC_BOOT_PART==EMMC_BOOT_FROM_BOOT2)
    partition_config |= 0x50;    // Boot Partition 2 enable for boot
    emmc_config->config.BootPartitionEnable = EMMC_BOOT2_4_BOOT;
#endif

#if (EMMC_BOOT_PART==EMMC_BOOT_FROM_LAST)
    partition_config |= (emmc_config->config.BootPartitionEnable << 3);
    //emmc_config->config.BootPartitionEnable = emmc_config->config.BootPartitionEnable;
#endif

#if (EMMC_BOOT_PART==EMMC_BOOT_FROM_LAST_ACCESS)
    if( emmc_config->config.CFEBootMode != BOOT_FROM_EMMC )
    {
        if( emmc_probe->flash_part_attr == EMMC_RW_BOOT_PART_1 )
        {    
            partition_config |= 0x48;    // Boot Partition 1 enable for boot : Keeping boot partition
        }
        else if( emmc_probe->flash_part_attr == EMMC_RW_BOOT_PART_2 )
        {
            partition_config |= 0x50;    // Boot Partition 2 enable for boot
        }
    }
    else
    {
        if( emmc_config->config.BootPartitionEnable == EMMC_BOOT2_4_BOOT )
        {
            if( emmc_probe->flash_part_attr == EMMC_RW_BOOT_PART_1 )
            {
                partition_config |= 0x48;   // For Boot1 partition to boot on next power on.
            }
            else
            {
                partition_config |= 0x50;   // Keep Boot2 partition to boot on next power on.
            }
        }
        else
        {
            if( emmc_probe->flash_part_attr == EMMC_RW_BOOT_PART_2 )
            {
                partition_config |= 0x50;   // For Boot2 partition to boot on next power on.
            }
            else
            {
                partition_config |= 0x48;   // Keep Boot1 partition to boot on next power on.
            }
        }
    }
#endif
        
    return_val = emmc_CMD6_C0ba_acR1b_SWITCH_ExtCSD( Idx_ExtCSD_ACC_WRB, Idx_ExtCSD_PARTITION_CONFIG, partition_config, emmc_config );
    cfe_usleep(1000 * emmc_config->config.PartitionSwitchTime );     // Important

#if DEBUG_EMMC_DRV_DISPATCH
    emmc_config->CST = emmc_CMD8_C0ba_adtcR1_SEND_EXT_CSD( emmc_config );
    printf(" partition_config : 0x%02X (%d)\n", partition_config, partition_config);
    printf(" emmc_config->ExtCSD.PARTITION_CONFIG = 0x%02X (%d)\n", emmc_config->ExtCSD.PARTITION_CONFIG, emmc_config->ExtCSD.PARTITION_CONFIG);    
#endif
    
    (void)return_val;
    
    return 0;
}


/*  *********************************************************************
    *  emmcdrv_read(ctx, buffer)
    *
    *  Called when the flash device is read.
    *
    *  Input parameters:
    *       ctx - device context
    *       buffer - data buffer context
    *  
    *  Return value:
    *       0 if ok else error code
    ********************************************************************* */
static int emmcdrv_read( cfe_devctx_t *ctx, iocb_buffer_t *buffer )
{   
    uintptr_t       buf_aligned=0;
    emmcflash_cfepart_cfg_t *part  = ctx->dev_softc;
    
#if (DEBUG_EMMC_DRV_DISPATCH || EMMC_ALIGN_256K)
    emmcflashdev_t      *softc = part->fp_dev;
    emmcflash_probe_t   *emmc_probe = &softc->fd_probe;   
#endif  

#if DEBUG_EMMC_DRV_DISPATCH
    printf("\n\n ----->>> emmcdrv_read\n\n");
    printf("      ctx->dev_dev.dev_fullname    : %s\n", ctx->dev_dev->dev_fullname);
    printf("      ctx->dev_dev.dev_class       : %d\n", ctx->dev_dev->dev_class);
    printf("      ctx->dev_dev.dev_opencount   : %d\n", ctx->dev_dev->dev_opencount);
    printf("      ctx->dev_dev.dev_description : %s\n", ctx->dev_dev->dev_description);
    printf("      emmc_probe->flash_type       : %d\n", emmc_probe->flash_type);
    printf("      emmc_probe->flash_phy_size   : 0x%llu (%dMB)\n", emmc_probe->flash_phy_size, (uint32_t)(emmc_probe->flash_phy_size>>20));
    printf("      emmc_probe->flash_block_size : %d\n", emmc_probe->flash_block_size);
    printf("      emmc_probe->flash_part_attr  : %d\n", emmc_probe->flash_part_attr);
    printf("      emmc_probe->flash_nparts     : %d\n", emmc_probe->flash_nparts);
    printf("      emmc_probe->flash_log_size   : 0x%llu (%dMB)\n", emmc_probe->flash_log_size, (uint32_t)(emmc_probe->flash_log_size>>20));
    printf("      part->fp_partition : %d\n",part->fp_partition);
    printf("      part->fp_size      : 0x%llu (%dKB)\n",part->fp_size, (uint32_t)(part->fp_size>>10));
    printf("      part->fp_offset_bytes    : 0x%llu\n",part->fp_offset_bytes, part->fp_offset_bytes);
    printf("      buffer->buf_ptr      : 0x%08X\n", buffer->buf_ptr);
    printf("      buffer->buf_offset   : %d (0x%08X)\n", (unsigned long)buffer->buf_offset, buffer->buf_offset);
    printf("      buffer->buf_length   : %d (0x%08X)\n", (uint32_t)buffer->buf_length, buffer->buf_length);
    printf("      buffer->buf_retlen   : %d (0x%08X)\n", (uint32_t)buffer->buf_retlen, buffer->buf_retlen);    
    printf("      buffer->buf_ioctlcmd : %d (0x%08X)\n\n", (uint32_t)buffer->buf_ioctlcmd, buffer->buf_ioctlcmd);    
#endif    
    
    if( part->fp_size < (buffer->buf_offset + buffer->buf_length) )
    {
        printf("Error: read out of bounds. Accessed range:0x%08x - 0x%08x\n"
            , buffer->buf_offset, (buffer->buf_offset + buffer->buf_length));
        return CFE_ERR_INV_PARAM;
    }

#if EMMC_ALIGN_256K    
    // Find new buf_offset to consider SDMA alignment
    // 56MB line @ 0x8380_0000 - 1 block , FLASH_STAGING_BUFFER_RD (Support Max 55MB)
    uint32_t i;
    uint32_t buf_offset=0;
    uint8_t     *buf_read_ptr, *buf_tmp_ptr;
    buf_offset  = (uint32_t)buffer->buf_offset & (emmc_probe->flash_block_size-1) ; // emmc_probe->flash_block_size must be 512 OR 4K
    buf_aligned = (uintptr_t)(buffer->buf_ptr & (emmc_probe->flash_block_size-1)) || buf_offset; //need to realign buffer if either buf_ptr is misaligned OR buf_offset misaligned
    buf_aligned = (uintptr_t)(buffer->buf_ptr + (uint32_t)buffer->buf_offset) & (emmc_probe->flash_block_size-1) ;  // DDR address must be aligned at block size(512 OR 4K)
    buf_aligned = (uintptr_t)(buffer->buf_ptr) & (EMMC_SDMA_ALIGNEMENT-1) || buf_offset;    
    buf_aligned = (uintptr_t)(buffer->buf_ptr) & (emmc_probe->flash_block_size-1) || buf_offset;    
#else   
    buf_aligned = 0;    
#endif
        
    if( buf_aligned == 0 )
    {
        emmcdrv_read_write( ctx, buffer, EMMC_READ );
    }
    else
    {
#if EMMC_ALIGN_256K    
        if(buffer->buf_length > EMMC_NON_ALIGNED_MAX_SIZE)
        {
            // -offset option can't support over 55MB, due to staging buffer size.
            printf("\n\n!!! -offset option can't support over 55MB !!!\n\n");
            return CFE_ERR_INV_PARAM;
        }
        else
        {
            // [1] Copy data for alignment at 256K from 2nd block, partial first block can be unaligned to 256K
#if CFG_RUNFROMKSEG0
            buf_read_ptr = (uint8_t *) KERNADDR(EMMC_STAGING_BUFFER_OFFSET + EMMC_SDMA_ALIGNEMENT - emmc_probe->flash_block_size + buf_offset);
#else
            buf_read_ptr = (uint8_t *) UNCADDR(EMMC_STAGING_BUFFER_OFFSET + EMMC_SDMA_ALIGNEMENT - emmc_probe->flash_block_size + buf_offset);
#endif
            buf_tmp_ptr     = buffer->buf_ptr;
            buffer->buf_ptr = buf_read_ptr;
            
            // [2] Read
            emmcdrv_read_write( ctx, buffer, EMMC_READ );
    
            // [3] Rewrite to align
            buffer->buf_ptr = buf_tmp_ptr;          // Recover original buffer pointer
            for( i=0; i<buffer->buf_length; i++ )
            {
                buffer->buf_ptr[i] = buf_read_ptr[i];
            }
            //printf("     buf_read_ptr : 0x%08X\n", buffer->buf_ptr);

        }
#endif      
    }
    
    return 0;
}


/*  *********************************************************************
    *  emmcdrv_write(ctx, buffer)
    *
    *  Called when the flash device is read.
    *
    *  Input parameters:
    *       ctx - device context
    *       buffer - data buffer context
    *  
    *  Return value:
    *       0 if ok else error code
    ********************************************************************* */
static int emmcdrv_write( cfe_devctx_t *ctx, iocb_buffer_t *buffer )
{   
    uintptr_t buf_aligned=0;
    emmcflash_cfepart_cfg_t     *part  = ctx->dev_softc;

#if (DEBUG_EMMC_DRV_DISPATCH || EMMC_ALIGN_256K)
    emmcflashdev_t      *softc = part->fp_dev;
    emmcflash_probe_t   *emmc_probe = &softc->fd_probe;   
#endif  
        
#if DEBUG_EMMC_DRV_DISPATCH
    printf("\n\n ----->>> emmcdrv_write\n\n");
    printf("      ctx->dev_dev.dev_fullname    : %s\n", ctx->dev_dev->dev_fullname);
    printf("      ctx->dev_dev.dev_class       : %d\n", ctx->dev_dev->dev_class);
    printf("      ctx->dev_dev.dev_opencount   : %d\n", ctx->dev_dev->dev_opencount);
    printf("      ctx->dev_dev.dev_description : %s\n", ctx->dev_dev->dev_description);
    printf("      emmc_probe->flash_type       : %d\n", emmc_probe->flash_type);
    printf("      emmc_probe->flash_phy_size   : 0x%llu (%dMB)\n",  emmc_probe->flash_phy_size, (uint32_t)(emmc_probe->flash_phy_size>>20));
    printf("      emmc_probe->flash_block_size : %d\n", emmc_probe->flash_block_size);
    printf("      emmc_probe->flash_part_attr  : %d\n",         emmc_probe->flash_part_attr);
    printf("      emmc_probe->flash_nparts     : %d\n", emmc_probe->flash_nparts);
    printf("      emmc_probe->flash_log_size   : 0x%llu (%dMB)\n",emmc_probe->flash_log_size, (uint32_t)(emmc_probe->flash_log_size>>20));
    printf("      part->fp_partition : %d\n",           part->fp_partition);
    printf("      part->fp_size      : 0x%llu (%dKB)\n",    part->fp_size, (uint32_t)(part->fp_size>>10));
    printf("      part->fp_offset_bytes    : 0x%llu\n",       part->fp_offset_bytes, part->fp_offset_bytes);
    printf("      buffer->buf_ptr      : 0x%08X\n", buffer->buf_ptr);
    printf("      buffer->buf_offset   : %d (0x%08X)\n", (unsigned long)buffer->buf_offset, buffer->buf_offset);
    printf("      buffer->buf_length   : %d (0x%08X)\n", (uint32_t)buffer->buf_length, buffer->buf_length);
    printf("      buffer->buf_retlen   : %d (0x%08X)\n", (uint32_t)buffer->buf_retlen, buffer->buf_retlen);    
    printf("      buffer->buf_ioctlcmd : %d (0x%08X)\n\n", (uint32_t)buffer->buf_ioctlcmd, buffer->buf_ioctlcmd);
#endif
     
    if( part->fp_size < (buffer->buf_offset + buffer->buf_length) )
    {
        printf("Error: read out of bounds. Accessed range:0x%08x - 0x%08x\n"
            , buffer->buf_offset, (buffer->buf_offset + buffer->buf_length));
        return CFE_ERR_INV_PARAM;
    }

#if EMMC_ALIGN_256K    
    // Find new buf_offset to consider SDMA alignment
    // 56MB line @ 0x8380_0000 - 1 block , FLASH_STAGING_BUFFER_RD (Support Max 55MB)
    uint32_t    i;
    uint8_t     *buf_tmp_ptr;
    uint32_t    buf_offset=0;
    buf_offset  = (uint32_t)buffer->buf_offset & (emmc_probe->flash_block_size-1) ; // emmc_probe->flash_block_size must be 512 OR 4K
    buf_aligned = (uintptr_t)(buffer->buf_ptr + (uint32_t)buffer->buf_offset) & (emmc_probe->flash_block_size-1) ;  // DDR address must be aligned at block size(512 OR 4K)
#else
    buf_aligned = 0;
#endif

    if( buf_aligned == 0 )
    {
        emmcdrv_read_write( ctx, buffer, EMMC_WRITE );
    }
    else
    {
#if EMMC_ALIGN_256K    
        if(buffer->buf_length > EMMC_NON_ALIGNED_MAX_SIZE) // && !Cache Area.
        {
            // -offset option can't support over 55MB, due to staging buffer size.
            printf("\n\n!!! -offset option can't support over 55MB !!!\n\n");
            return CFE_ERR_INV_PARAM;
        }
        else
        {
            // [1] Copy data for alignment at 256K from 2nd block
            // 56MB line @ 0x8380_0000 - 1 block , FLASH_STAGING_BUFFER_RD (Support Max 55MB)
#if CFG_RUNFROMKSEG0
            buf_tmp_ptr = (uint8_t *) KERNADDR(EMMC_STAGING_BUFFER_OFFSET - emmc_probe->flash_block_size + buf_offset);
#else
            buf_tmp_ptr = (uint8_t *) UNCADDR(EMMC_STAGING_BUFFER_OFFSET - emmc_probe->flash_block_size + buf_offset);
#endif
            for( i=0; i<buffer->buf_length; i++ )
            {
                buf_tmp_ptr[i] = buffer->buf_ptr[i];
            }
            buffer->buf_ptr = buf_tmp_ptr;
            printf("      buf_tmp_ptr      : 0x%08X\n", buffer->buf_ptr);
            
            // [2] Write
            emmcdrv_read_write( ctx, buffer, EMMC_WRITE );
        }
#endif      
    }
    
    return 0;
}


/*  *********************************************************************
    *  emmcdrv_inpstat(ctx,inpstat)
    *
    *  Return "input status".  For flash devices, we always return true.
    *
    *  Input parameters:
    *      ctx - device context
    *      inpstat - input status structure
    *
    *  Return value:
    *      0 if ok, else error code
    ********************************************************************* */
static int emmcdrv_inpstat( cfe_devctx_t *ctx, iocb_inpstat_t *inpstat )
{
#if DEBUG_EMMC_DRV_DISPATCH
    printf("\n\n ----->>> emmcdrv_inpstat\n\n");
#endif    
    inpstat->inp_status = 1;

    return 0;
}



/*  *********************************************************************
    *  emmcdrv_ioctl(ctx,buffer)
    *
    *  Handle special IOCTL functions for the flash.  Flash devices
    *  support NVRAM information, sector and chip erase, and a
    *  special IOCTL for updating the running copy of CFE.
    *
    *  Input parameters:
    *      ctx - device context
    *      buffer - descriptor for IOCTL parameters
    *
    *  Return value:
    *      0 if ok else error
    ********************************************************************* */
static int emmcdrv_ioctl( cfe_devctx_t *ctx, iocb_buffer_t *buffer )
{
    emmcflash_cfepart_cfg_t     *part  = ctx->dev_softc;
    emmcflashdev_t      *softc = part->fp_dev;
    emmcflash_probe_t   *emmc_probe = &softc->fd_probe;   
    emmcdev_t           *emmc_config = &emmc_probe->emmc_config;
    uint32_t            cst=0;
    flash_range_t       range;

#if DEBUG_EMMC_DRV_DISPATCH
    printf("\n\n ----->>> emmcdrv_ioctl : cmd = %d\n\n", (int)buffer->buf_ioctlcmd );
#endif

    switch( (int)buffer->buf_ioctlcmd )
    {
        case IOCTL_FLASH_ERASE_RANGE:
            memcpy(&range,buffer->buf_ptr,sizeof(flash_range_t));
            range.range_base += part->fp_offset_bytes;
            if (range.range_length > part->fp_size)
            {
                range.range_length = (unsigned long)part->fp_size;
            }
            return emmc_erase_range(softc,&range);
        
        case IOCTL_EMMC_INFO:
            cst = emmc_CMD8_C0ba_adtcR1_SEND_EXT_CSD( emmc_config );
            emmc_Print_CID( emmc_config );
            emmc_Print_CSD( emmc_config );
            emmc_Print_ExtCSD( emmc_config );
            emmc_Print_HostAndEmmcInfo( emmc_config );
            emmc_Print_EmmcSizePartitionInfo( emmc_config );
            return 0;
    
        case IOCTL_EMMC_BOOTPART_SEL:
            cst = emmc_Sel_Partition( emmc_config, buffer->buf_offset );
            return 0;
    
        case IOCTL_EMMC_RESET:
            cst = emmc_CMD0_C0ba_bc_GO_IDLE_STATE( emmc_config );
            emmc_Enable_Host_Int( 0 );
            emmc_Clear_HostIntStatus( SDIO_HOST_INT_CLR_ALL );        
            EMMC_TOP_CFG->emmc_top_cfg_sdio_emmc_ctrl1 |= ~0xFFFFFBFF; 
            EMMC_HOSTIF->emmc_host_ctrl_set1 = 0x070F0000;        
            printf("\n!!! Resetting eMMC to idle state NOW %d!!!\n", cst);
           return 0;
                
        default:
            /* Call hook if present. */
            if (softc->fd_probe.flash_ioctl_hook)
            {    
#if DEBUG_EMMC_DRV
                printf("softc->fd_probe.flash_ioctl_hook:\n", softc->fd_probe.flash_ioctl_hook);
#endif
                return (*(softc->fd_probe.flash_ioctl_hook))(ctx,buffer);
            }
            return -1;
    }
    return -1;
}



/*  *********************************************************************
    *  emmcdrv_close(ctx)
    *
    *  Close the flash device.
    *
    *  Input parameters:
    *      ctx - device context
    *
    *  Return value:
    *      0
    ********************************************************************* */
static int emmcdrv_close( cfe_devctx_t *ctx )
{
    return 0;
}



/*  *********************************************************************
    *  emmcdrv_probe(drv, probe_ptr, emmc_config_ptr)
    *
    *  Device probe routine.  Attach the flash device to
    *  CFE//s device table.
    *
    *  Input parameters:
    *      drv - driver descriptor
    *      probe_a - physical address of flash
    *      probe_b - size of flash (bytes)
    *      probe_ptr - unused
    *
    *  Return value:
    *      nothing
    ********************************************************************* */
static void emmcdrv_probe( cfe_driver_t *drv, unsigned long probe_a, unsigned long probe_b, void *probe_ptr )
{
    emmcflashdev_t      *softc;
    emmcflash_probe_t   *probe;
    static int  flashidx = 0;
    int         idx;
    char        descr[100];
    //char        *x;
    
    
    //------------------------------------------------------
    // [1] probe, softc
    probe = (emmcflash_probe_t *)probe_ptr;
    softc = (emmcflashdev_t *)KMALLOC( sizeof(emmcflashdev_t), 0 );
#if DEBUG_EMMC_DRV_DISPATCH
        printf(" --->>> 1. probe = %08X, softc = %08X\n", probe, softc);
#endif
    
    if( softc )
    {
        memset( softc, 0, sizeof(emmcflashdev_t) );
        
#if DEBUG_EMMC_DRV_DISPATCH
                printf(" --->>> 2. emmc_do_probe = %08X\n", softc);
#endif
        //------------------------------------------------------
        // [2] Do automatic probing
        if( probe )
        {
            memcpy( &(softc->fd_probe), probe, sizeof(emmcflash_probe_t) );
            //if (softc->fd_probe.flash_nchips == 0)
            //{
            //  softc->fd_probe.flash_nchips = 1;
            //}
        }
        emmc_do_probe( softc );
        // Information after probing
        softc->fd_sectorbuffer = (unsigned char *)KMALLOC( EMMC_MAX_ONE_BLOCK_SIZE, EMMC_MAX_ONE_BLOCK_SIZE );
        softc->fd_pttlsize = softc->fd_probe.flash_phy_size;
        
        //------------------------------------------------------
        // [3] Set description
#if DEBUG_EMMC_DRV_DISPATCH
        printf(" --->>> 3. Set description \n");
#endif
        /* Remember total size of all devices */
        //softc->fd_ttlsize = softc->fd_probe.flash_nchips * softc->fd_probe.flash_size;
        
        /* Set description */
        //x = descr;
        //x += xsprintf(x,"%s at %08X psize %uKB lsize %uKB",
        //        drv->drv_description, softc->fd_probe.flash_phy_addr/1024, softc->fd_lttlsize/1024);
        //if (softc->fd_probe.flash_nchips > 1) {
        //      xsprintf(x," (%d chips)",softc->fd_probe.flash_nchips);
        //}
#if DEBUG_EMMC_DRV_DISPATCH
        printf("    %s at %016llX psize %uKB lsize %uKB\n",
                drv->drv_description, softc->fd_probe.flash_phy_addr/1024, softc->fd_lttlsize/1024);        
#endif
        
        //------------------------------------------------------
        // [2] Do Partitions
#if DEBUG_EMMC_DRV_DISPATCH
        printf(" --->>> 4. emmc_do_parts \n");
#endif
        /*
         * If flash is not partitioned, just instantiate one device.
         * Otherwise, instantiate multiple flashes to cover the entire device.
         */
        if (softc->fd_probe.flash_nparts == 0)
        {
            softc->fd_part_cfg[0].fp_dev = softc;
            softc->fd_part_cfg[0].fp_offset_bytes = 0;
            softc->fd_part_cfg[0].fp_size = softc->fd_probe.flash_log_size;
            //softc->fd_part_cfg[0].fp_size = 0;
            cfe_attach( drv, &(softc->fd_part_cfg[0]), NULL,descr);
        }
        else
        {
            /* Partition flash into chunks */
            if( softc->fd_probe.flash_phy_size > 0 )
            {
                emmc_do_parts( softc );
                
                // Final logical size after partition
                softc->fd_probe.flash_log_size = emmc_get_dev_log_size( &(softc->fd_probe) );
                softc->fd_lttlsize = softc->fd_probe.flash_log_size;
            }
            else
            {
                printf(" !!! Can't parition logically due to no physical size!!!\n");
                goto no_instantiate;
            }
            
            /* Instantiate devices for each piece */
            for (idx = 0; idx < softc->fd_probe.flash_nparts; idx++)
            {
                char name[32];
                char *nptr, *emmc_part_name;
                
                //printf(" --->>> 4. fd_probe 0, idx=%d, fp_partition=%d\n", idx, softc->fd_part_cfg[idx].fp_partition);
                switch( softc->fd_part_cfg[idx].fp_partition )
                {
                    case EMMC_PART_DATA:    emmc_part_name = "DATA";  break;
                    case EMMC_PART_BOOT1:   emmc_part_name = "BOOT1"; break;
                    case EMMC_PART_BOOT2:   emmc_part_name = "BOOT2"; break;
                    case EMMC_PART_RPMB:    emmc_part_name = "RPMB";  break;
                    case EMMC_PART_GP1:     emmc_part_name = "GP1"; break;
                    case EMMC_PART_GP2:     emmc_part_name = "GP2"; break;
                    case EMMC_PART_GP3:     emmc_part_name = "GP3"; break;
                    case EMMC_PART_GP4:     emmc_part_name = "GP4"; break;
                    default:                emmc_part_name = "ERROR"; break;
                }    
                      
                xsprintf(descr,"%s phys partition %s, offset:%016llX size:%08uKB",
                    drv->drv_description, 
                    emmc_part_name,
                    softc->fd_part_cfg[idx].fp_offset_bytes, (uint32_t)((softc->fd_part_cfg[idx].fp_size+1023)/1024));                    
#if DEBUG_EMMC_DRV_DISPATCH
                printf("    %s at %016llX offset %16llX size %uKB : %s",
                    drv->drv_description, softc->fd_probe.flash_phy_addr, 
                    softc->fd_part_cfg[idx].fp_offset_bytes, (uint32_t)((softc->fd_part_cfg[idx].fp_size+1023)/1024), emmc_part_name);
#endif
                softc->fd_part_cfg[idx].fp_dev = softc;
                if (softc->fd_probe.flash_part_spec[idx].fp_name == NULL)
                {
                    sprintf(name,"%d",idx);
                    nptr = name;
                }
                else
                {
                    nptr = softc->fd_probe.flash_part_spec[idx].fp_name;
                }
                
                cfe_attach_idx( drv, softc->fd_part_cfg[idx].fp_partition, &(softc->fd_part_cfg[idx]), nptr, descr );
            }
        }
        
        flashidx++; /* Count total sector(physical partition) on the device */

        no_instantiate:
        if( probe )
        {
            memcpy( probe, &(softc->fd_probe), sizeof(emmcflash_probe_t) );
#if 0
            printf("--- flashidx = %d\n",flashidx);
            printf(" probe->flash_type       = %d\n", probe->flash_type);
            printf(" probe->flash_phy_addr   = %016llX\n", probe->flash_phy_addr);
            printf(" probe->flash_phy_size   = %016llX\n", probe->flash_phy_size);
            printf(" probe->flash_block_size = %d\n", probe->flash_block_size);
            
            printf(" probe->flash_part_attr = %d\n", probe->flash_part_attr);
            printf(" probe->flash_nparts    = %d\n", probe->flash_nparts);
            printf(" probe->flash_log_size  = %016llX\n", probe->flash_log_size);
            printf("---\n");
            printf("\n");
#endif                  
        };
                 
    }

}


/*  *********************************************************************
    *  emmc_do_probe(softc)
    *
    *  Probe to see if we//re ROM or RAM.  If ROM, see if we//re flash.
    *  If flash, do CFI query.
    *
    *  Input parameters:
    *      softc - our structure
    *
    *  Return value:
    *      FLASH_TYPE_xxx
    ********************************************************************* */
static void emmc_do_probe( emmcflashdev_t *softc )
{
    emmc_get_config( softc );
}



/*  *********************************************************************
    *  emmc_do_parts(probe,parts)
    *
    *  Partition the flash into the sizes specified.  We use
    *  the sizes in the table to generate a table of {offset,size}
    *  pairs that eventually become partitions.
    *
    *  The only thing magical about this is that size "0" means
    *  "fill to max" and that partitions beyond the "0" are aligned
    *  to the top of the flash.  Therefore, if you had a 4MB
    *  flash and listed four partitions, 512K, 0, 512K, 512K,
    *  then there would be a 2.5M partition in the middle and two
    *  512K partitions at the top.
    *
    *  Alignment: First and last logical partitions are hard aligned
    *  to top and bottom of flash. Rest of the partitions are aligned
    *  to the default alingnment EMMC_PART_ALIGN_BYTES
    *
    *  Input parameters:
    *      probe - flash probe data (user-supplied table)
    *      parts - our partition table (output)
    *
    *  Return value:
    *      nothing
    ********************************************************************* */
static void emmc_do_parts( emmcflashdev_t *softc )
{
    int idx;
    int middlepart = -1;
    emmcflash_probe_t *probe = &(softc->fd_probe);
    emmcflash_cfepart_cfg_t *parts = &(softc->fd_part_cfg[0]);
    uint64_t lobound = 0;
    uint64_t hibound = probe->flash_phy_size;
    
#if 0
    for (idx = 0; idx < probe->flash_nparts; idx++)
    {
        printf(" probe->flash_part_spec[%d].fp_offset_bytes  = %016llX\n", idx, probe->flash_part_spec[idx].fp_offset_bytes);
        printf(" probe->flash_part_spec[%d].fp_size          = %016llX\n", idx, probe->flash_part_spec[idx].fp_size);
    }
#endif

    /* We start off by calculating offsets of all partitions upto
     * the partition with a fp_size = 0, i.e one which has to stretch
     * to fill the remaining flash */
    for (idx = 0; idx < probe->flash_nparts; idx++)
    {

        /* Quit if we encounter an fp_size of 0, meaning this is a FILL partition */
        if (probe->flash_part_spec[idx].fp_size == 0)
        {
            middlepart = idx;
            break;
        }
        else
        {
            /* If partition offset is already specified, this means we are recreating partitions
             * from an existing partition table. We forego any calculations and use specified
             * offsets and sizes 
             */
            if( probe->flash_part_spec[idx].fp_offset_bytes )
            {
                /* Add partition offset */
                parts[idx].fp_offset_bytes = probe->flash_part_spec[idx].fp_offset_bytes;
                lobound += probe->flash_part_spec[idx].fp_offset_bytes + probe->flash_part_spec[idx].fp_size;
            }
            else
            {
                /* Align all partitions to default alignment, except
                 * first one which gets aligned to the start of flash
                 */
                if( idx )
                {
                    /* Align partition */
                    lobound = ALIGN_INC(lobound, EMMC_PART_ALIGN_BYTES);
                }

                if( lobound +  probe->flash_part_spec[idx].fp_size > hibound )
                {
                    printf("!!! Not creating partition %d, Offset %lu + Size %lu > hibound %lu", 
                        idx, lobound, probe->flash_part_spec[idx].fp_size, hibound);
                    continue;
                }

                /* Add partition offset */
                parts[idx].fp_offset_bytes = lobound;

                /* Update partition specification to reflect offsets */
                probe->flash_part_spec[idx].fp_offset_bytes = parts[idx].fp_offset_bytes;

                /* Update lobound after adding partition */
                lobound += probe->flash_part_spec[idx].fp_size;
            }
                    
            /* Add partition */
            parts[idx].fp_size = probe->flash_part_spec[idx].fp_size;
            parts[idx].fp_partition = probe->flash_part_spec[idx].fp_partition;
#if 0
            printf(" [emmc_do_parts] 1. parts[%d].fp_offset_bytes = %016llX\n", idx, parts[idx].fp_offset_bytes);
            printf(" [emmc_do_parts] 1. parts[%d].fp_size   = %016llX\n", idx, parts[idx].fp_size);
            printf(" [emmc_do_parts] 1. parts[%d].fp_partition = %d\n", idx, parts[idx].fp_partition);
#endif
        }
    }

    /* If we are here, then this means that we have hit a partition with
     * fp_size = 0 i.e one that we need to fill to the maximum, after we
     * have created the other partitions with specified sizes. In this case
     * We create that partitions after this 'no size' partition first 
     * and then move backwards */
    if (idx != probe->flash_nparts)
    {
        for (idx = probe->flash_nparts - 1; idx > middlepart; idx--)
        {
            /* Try to add partition to the end --> update hibound */
            hibound -= probe->flash_part_spec[idx].fp_size;

            /* Align all partitions to default alignment, except
             * last one which gets aligned to the end of flash
             */
            if( idx !=  probe->flash_nparts - 1 )
            {
                /* Align partition */
                hibound = ALIGN_DEC(hibound, EMMC_PART_ALIGN_BYTES);
            }

            if( lobound > hibound )
            {
                printf("!!! Not creating partition %d, lobound %d > hibound %d", 
                    idx, lobound, hibound);
                continue;
            }

            /* Add partition */
            parts[idx].fp_size = probe->flash_part_spec[idx].fp_size;
            parts[idx].fp_partition = probe->flash_part_spec[idx].fp_partition;
            parts[idx].fp_offset_bytes = hibound ;

            /* Update partition specification to reflect offsets */
            probe->flash_part_spec[idx].fp_offset_bytes = parts[idx].fp_offset_bytes;
#if 0
            printf(" [emmc_do_parts] 2. parts[%d].fp_offset_bytes = %016llX\n", idx, parts[idx].fp_offset_bytes);
            printf(" [emmc_do_parts] 2. parts[%d].fp_size   = %016llX\n", idx, parts[idx].fp_size);
            printf(" [emmc_do_parts] 2. parts[%d].fp_partition = %d\n", idx, parts[idx].fp_partition);
            printf(" [emmc_do_parts] 2. hibound             = %016llX\n", hibound);
#endif
        }
    }

    /* Here we create the partition that fills the remaining flash */
    if (middlepart != -1)
    {
        /* Align partition */
        lobound = ALIGN_INC(lobound, EMMC_PART_ALIGN_BYTES);

        if( lobound > hibound )
        {
            printf("!!! Not creating fill partition %d, lobound %d > hibound %d", 
                middlepart, lobound, hibound);
        }
        else
        {
            /* Calculate size of 'fill' partition */
            parts[middlepart].fp_size           = hibound - lobound;
            parts[middlepart].fp_offset_bytes   = lobound;
            parts[middlepart].fp_partition      = probe->flash_part_spec[middlepart].fp_partition;

            /* Update partition specification to reflect offset and size */
            probe->flash_part_spec[middlepart].fp_size          = parts[middlepart].fp_size;
            probe->flash_part_spec[middlepart].fp_offset_bytes  = parts[middlepart].fp_offset_bytes;
        }
#if 0
        printf(" [emmc_do_parts] 3. parts[%d].fp_offset_bytes = %016llX\n", middlepart, parts[middlepart].fp_offset_bytes);
        printf(" [emmc_do_parts] 3. parts[%d].fp_size   = %016llX\n", middlepart, parts[middlepart].fp_size);
        printf(" [emmc_do_parts] 3. parts[%d].fp_partition = %d\n", middlepart, parts[middlepart].fp_partition);
#endif
    }
    
#if 0
    for (idx = 0; idx < probe->flash_nparts; idx++)
    {
        printf(" probe->flash_part_spec[%d].fp_offset_bytes  = %016llX\n", idx, probe->flash_part_spec[idx].fp_offset_bytes);
        printf(" probe->flash_part_spec[%d].fp_size          = %016llX\n", idx, probe->flash_part_spec[idx].fp_size);
    }
#endif
        
}


/*  *********************************************************************
    *  emmc_get_config( emmcflashdev_t *softc )
    *
    *
    *  Input parameters:
    *      softc - out flash
    *
    *  Return value:
    *      0 if successful, <0 if error
    ********************************************************************* */
static int emmc_get_config( emmcflashdev_t *softc )
{
    softc->fd_probe.flash_type          = FLASH_TYPE_FLASH;
    softc->fd_probe.flash_phy_size      = emmc_get_dev_phy_size( &softc->fd_probe );
    softc->fd_probe.flash_block_size    = emmc_get_block_size( &softc->fd_probe );
    
    return 0;
}


/*  *********************************************************************
    *  emmc_get_dev_phy/log_size(fd_probe)
    *
    *  Get flash device size
    *
    *  Input parameters:
    *      fd_probe - our flash
    *
    *  Return value:
    *      emmc_size for physical/logical size
    *
    ********************************************************************* */
uint64_t emmc_get_dev_phy_size( emmcflash_probe_t *fd_probe )
{
    uint64_t emmc_size=0;
    
#if 1
    switch( fd_probe->flash_part_attr )
    {
        case EMMC_PART_DATA:    emmc_size = fd_probe->emmc_config.config.DataSize;  break;
        case EMMC_PART_BOOT1:   emmc_size = fd_probe->emmc_config.config.Boot1Size; break;
        case EMMC_PART_BOOT2:   emmc_size = fd_probe->emmc_config.config.Boot2Size; break;
        case EMMC_PART_RPMB:    emmc_size = fd_probe->emmc_config.config.RPMBSize;  break;
        case EMMC_PART_GP1:     emmc_size = fd_probe->emmc_config.config.GP1Size;   break;
        case EMMC_PART_GP2:     emmc_size = fd_probe->emmc_config.config.GP2Size;   break;
        case EMMC_PART_GP3:     emmc_size = fd_probe->emmc_config.config.GP3Size;   break;
        case EMMC_PART_GP4:     emmc_size = fd_probe->emmc_config.config.GP4Size;   break;
        default:                emmc_size = 0;                                      break;
    }
#else
    uint8_t i;

    if( fd_probe->flash_part_attr == EMMC_PART_DATA )
    {
        // CFE handle only max 2GB for eMMC even if phsycal eMMC size is bigger than 2GB, because size is much bigger than CFE.
        if( fd_probe->emmc_config.config.OCR_SectorMode == EMMC_ON )    // ON(>2GB), OFF(<=2GB)
        {
            emmc_size = 0x80000000;     // 2GB
        }
        else
        {
            emmc_size = fd_probe->emmc_config.config.DataSize;
        }
    }
    else
    {
        for( i = 0; i < fd_probe->flash_nparts; i++ )
            emmc_size += fd_probe->flash_part_spec[i].fp_size;
    }
#endif
    
    return ( emmc_size );
}


uint64_t emmc_get_dev_log_size( emmcflash_probe_t *fd_probe )
{
    int         i;
    uint64_t    emmc_size=0;

    for( i = 0; i < fd_probe->flash_nparts; i++ )
    {
        emmc_size += fd_probe->flash_part_spec[i].fp_size;
    }

    return ( emmc_size );
}


/*  *********************************************************************
    *  emmc_get_block_size(softc,range)
    *
    *  Erase a range of sectors
    *
    *  Input parameters:
    *      softc - our flash
    *      range - range structure
    *
    *  Return value:
    *      0 if ok
    *      else error
    *
    ********************************************************************* */
int32_t emmc_get_block_size ( emmcflash_probe_t *fd_probe )
{
    return (int32_t)fd_probe->emmc_config.config.ReadBlkLen;
}



/*  *********************************************************************
    *  emmcdrv_read_write(ctx, buffer, read_write)
    *
    *  Called when the flash device is read.
    *
    *  Input parameters:
    *       ctx - device context
    *       buffer - data buffer context
    *       read_write - selection
    *  
    *  Return value:
    *       0 if ok else error code
    ********************************************************************* */
static int emmcdrv_read_write( cfe_devctx_t *ctx, iocb_buffer_t *buffer, uint8_t read_write )
{
    emmcflash_cfepart_cfg_t     *part  = ctx->dev_softc;
    emmcflashdev_t      *softc = part->fp_dev;
    emmcflash_probe_t   *emmc_probe = &softc->fd_probe;   
    emmcdev_t           *emmc_config = &emmc_probe->emmc_config;
    
    emmc_data_descriptor_t  data_descrptr;
    emmc_range_descriptor_t range_descrptr;

    uint64_t range_base=0;
    uint32_t return_val=0;
#if DEBUG_EMMC_SPEED
    unsigned long StartTime, EndTime;
    uint32_t RdWrTimeUsec, KByteRate;
#endif
    
#if EMMC_UNALIGNED_DMA
    uint32_t temp_block_cnt;
    unsigned int next_sdma_buffer_boundary;
    unsigned int num_full_blocks_till_boundary;
#endif    

    //------------------------------------------------------
    // [1] buffer->buf_retlen
    buffer->buf_retlen = buffer->buf_length;
    
    // [2] data_descriptor
    // eMMC flash information in one logical partition
    data_descrptr.block_attr    = 0;                            // [temp]   Block attribute : 1st/last/middle/less_one_block
    data_descrptr.block_offset  = 0;                                // [input]  [Byte] Offset of data from start of eMMC block  
    data_descrptr.block_address = 0;                            // [output] [Byte] Actual physical dddress in flash to access
    data_descrptr.block_size    = softc->fd_probe.flash_block_size; // [input]  [Byte] Block size of eMMC
    data_descrptr.block_cnt     = 0;                            // [temp]   Counter for transfered blocks
    // DRAM buffer information
    data_descrptr.read_buffer   = softc->fd_sectorbuffer;       // [temp]   For temporal buffer
    data_descrptr.data_buffer   = buffer->buf_ptr;          // [input]  Buffer pointer
    data_descrptr.data_size     = buffer->buf_length;           // [input]  [Byte] Length to transfer
    data_descrptr.data_offset   = 0;                    // [temp]   [Byte] The current offset to transfer now
    data_descrptr.data_copy_len = 0;                            // [temp]   [Byte] Copy length transfered
    data_descrptr.dma_address   = K0_TO_PHYS( (uint32_t)(uintptr_t)data_descrptr.data_buffer );   // [Usage] address in DRAM for DMA
#if ( __MIPSEB == 1 )
    if( ((uint32_t)data_descrptr.data_buffer & 0xE0000000) == 0x80000000 )
    data_descrptr.dma_address   = K0_TO_PHYS( (uint32_t)data_descrptr.data_buffer );        // [Usage] address in DRAM for DMA
    else if( ((uint32_t)data_descrptr.data_buffer & 0xE0000000) == 0xA0000000 )
        data_descrptr.dma_address   = K1_TO_PHYS( (uint32_t)data_descrptr.data_buffer );    // [Usage] address in DRAM for DMA
    else
        data_descrptr.dma_address   = (uint32_t)data_descrptr.data_buffer;                  // [Usage] address in DRAM for DMA
#endif
    
    // [3] range_descriptor
    range_base = PHYS_TO_K1b(buffer->buf_offset + part->fp_offset_bytes + part->fp_dev->fd_probe.flash_phy_addr);
    emmc_get_range_intersection(range_base, data_descrptr.data_size, data_descrptr.block_size, 
        softc->fd_probe.emmc_config.config.ReadBlkLenBit4Addr, &range_descrptr);
    
    // [4] data_descriptor : block_address
    data_descrptr.block_address = range_descrptr.new_range_base;
    
#if DEBUG_EMMC_DRV_DISPATCH
    if( read_write == EMMC_READ )    printf(" [READ]\n");
    else                             printf(" [WRITE]\n");
    //printf(" [buffer] : buf_offset = 0x%llu, length = 0x%llu, ptr = 0x%llu\n",    buffer->buf_offset, buffer->buf_length, buffer->buf_ptr);
    printf(" [buffer] :       buf_offset = 0x%8X, length = 0x%8X, ptr = 0x%8X\n",   buffer->buf_offset, buffer->buf_length, buffer->buf_ptr);
    printf(" flash offset :   buffer->buf_offset      = 0x%llu, part->fp_offset_bytes = 0x%llu\n",    buffer->buf_offset, part->fp_offset_bytes);
    printf("                  fd_probe.flash_phy_addr = 0x%llu, range_base      = 0x%llu\n",    part->fp_dev->fd_probe.flash_phy_addr, range_base);
    printf(" flash offset :   buffer->buf_offset      = 0x%X, part->fp_offset_bytes = 0x%X\n",    buffer->buf_offset, part->fp_offset_bytes);
    printf("                  fd_probe.flash_phy_addr = 0x%X, range_base      = 0x%X\n",    part->fp_dev->fd_probe.flash_phy_addr, range_base);
    printf(" [data_descrptr]  block_address       = 0x%llu\n", data_descrptr.block_address);
    printf(" [data_descrptr]  data_buffer(ptr)    = 0x%X\n", data_descrptr.data_buffer);
    printf(" [data_descrptr]  dma_address         = 0x%X\n", data_descrptr.dma_address);
    printf(" [data_descrptr]  data_size           = 0x%llu\n", data_descrptr.data_size);
    printf(" [data_descrptr]  read_buffer         = 0x%X\n", data_descrptr.read_buffer);
    printf(" [             ]  range_base          = 0x%llu\n", range_base);
    printf(" [range_descrptr] new_range_base          = 0x%llu\n",      range_descrptr.new_range_base);
    printf(" [range_descrptr] first_block_data_offset = %d (0x%X)\n", range_descrptr.first_block_data_offset, range_descrptr.first_block_data_offset);
    printf(" [range_descrptr] last_block_data_offset  = %d (0x%X)\n", range_descrptr.last_block_data_offset, range_descrptr.last_block_data_offset);
    printf(" [range_descrptr] num_block_access        = %d (0x%X)\n",   range_descrptr.num_block_access, range_descrptr.num_block_access);
#endif
    
    // [5] range_descriptor : block_attr, block_cnt, data_copy_len, dma_address
    //     -> Below [1] ~ [2]
    
#if DEBUG_EMMC_SPEED
    StartTime = _getticks( );
#endif

    //------------------------------------------------------
    // [1] Check & Read 1st block
    //printf("[1] Check & read/write 1st block\n");
    if( range_descrptr.first_block_data_offset != 0 )
    {
        // [1-1] Check 1st block
        data_descrptr.block_cnt = 1;
        if ( data_descrptr.data_size <= (data_descrptr.block_size - range_descrptr.first_block_data_offset) )
        {
            data_descrptr.block_attr    = EMMC_ACCESS_LESS_THAN_BLOCK;
            data_descrptr.block_offset  = range_descrptr.first_block_data_offset;
            data_descrptr.data_copy_len = data_descrptr.data_size;
        }
        else
        {
            data_descrptr.block_attr    = EMMC_ACCESS_FIRST_BLOCK;
            data_descrptr.block_offset  = range_descrptr.first_block_data_offset;
            data_descrptr.data_copy_len = data_descrptr.block_size - range_descrptr.first_block_data_offset;
        }
        data_descrptr.dma_address = K0_TO_PHYS( (uint32_t)(uintptr_t)data_descrptr.read_buffer );
                
        // [1-2] Read 1st block
        if( read_write == EMMC_READ )
        {
            return_val = emmc_block_read( data_descrptr, emmc_config, emmc_probe );
        }    
        else
        {
            return_val = emmc_block_write( data_descrptr, emmc_config, emmc_probe );
        }
        
        // [1-3] Update for middle blocks
        data_descrptr.block_address += data_descrptr.block_size;
        data_descrptr.data_offset   += data_descrptr.data_copy_len;
    }
    else
    {
        data_descrptr.block_cnt = 0;
    }

    //------------------------------------------------------
    // [2] Count & read middle block
    // [2-1] Count middle blocks
    //printf("[2] Count & read/write middle block\n");
    data_descrptr.block_cnt = range_descrptr.num_block_access - data_descrptr.block_cnt;
    if( range_descrptr.last_block_data_offset != 0 )    // There is last block.
        data_descrptr.block_cnt -= 1;
        
#if EMMC_UNALIGNED_DMA  
    /* This data will be written to full sectors on the eMMC device
     * However, we need to make sure that in the DMA buffer, the data 
     * for a single block doesnt span the 512K SDMA buffer address boundary
     * Therefore we must calculate the number of full blocks from the current
     * DMA address to the next 512K dma buffer address bondary, write those 
     * full blocks using a multi block write, then take the data for the block
     * which spans the 512K dma buffer boundary and write as a single block 
     * write. This way we avoid having to align the dma buffer to the 512L SDMA
     * buffer address boundary
     */ 
    while( data_descrptr.block_cnt )
    {
        /* Update dma_address */
#if ( __MIPSEB == 1 )
        if( ((uint32_t)(uintptr_t)data_descrptr.data_buffer & 0xE0000000) == 0x80000000 )
            data_descrptr.dma_address   = K0_TO_PHYS( (uint32_t)(uintptr_t)data_descrptr.data_buffer ) 
                                            + data_descrptr.data_offset;   // [Usage] address in DRAM for DMA
        else if( ((uint32_t)(uintptr_t)data_descrptr.data_buffer & 0xE0000000) == 0xA0000000 )
            data_descrptr.dma_address   = K1_TO_PHYS( (uint32_t)(uintptr_t)data_descrptr.data_buffer ) 
                                            + data_descrptr.data_offset;   // [Usage] address in DRAM for DMA
        else
#endif
        data_descrptr.dma_address   = (uint32_t)(uintptr_t)data_descrptr.data_buffer + data_descrptr.data_offset;  // [Usage] address in DRAM for DMA

        /* Calculate number of full eMMC blocks of data that fit into the dma buffer area 
         * between the current dma buffer address and the next 512K aligned dma buffer address
         */
        next_sdma_buffer_boundary = (data_descrptr.dma_address + (EMMC_SDMA_ADDR_ALIGN - 1)) & (~(EMMC_SDMA_ADDR_ALIGN-1));

        /* Calculate number of full blocks till next SDMA address boundary */
        num_full_blocks_till_boundary = (next_sdma_buffer_boundary - data_descrptr.dma_address) / data_descrptr.block_size;

        /* Ensure that number of full blocks doesnt go over the total block count */
        num_full_blocks_till_boundary = (num_full_blocks_till_boundary < data_descrptr.block_cnt) ? num_full_blocks_till_boundary:data_descrptr.block_cnt;

#if 0
        printf("addr:0x%08x alig:0x%08x nfbb:%d tblk:%d\n", data_descrptr.dma_address, next_sdma_buffer_boundary, num_full_blocks_till_boundary, data_descrptr.block_cnt);
#endif  

        if( num_full_blocks_till_boundary )
        {
            /* Preserve block_cnt */
            temp_block_cnt = data_descrptr.block_cnt;

            data_descrptr.block_cnt     = num_full_blocks_till_boundary;
            data_descrptr.block_attr    = EMMC_ACCESS_MIDDLE_BLOCK;
            data_descrptr.block_offset  = 0;
            data_descrptr.data_copy_len = data_descrptr.block_size * data_descrptr.block_cnt;

            /* Do a multi-block transfer for the full blocks */
            // [2-2] Read/write middle blocks
            if( read_write == EMMC_READ )
            {
                return_val = emmc_block_read( data_descrptr, emmc_config, emmc_probe );
            }    
            else
            {
                return_val = emmc_block_write( data_descrptr, emmc_config, emmc_probe );
            }
            
            // [2-3] Update for last block
            data_descrptr.block_address += data_descrptr.data_copy_len;
            data_descrptr.data_offset   += data_descrptr.data_copy_len;

            /* Restore and update block_cnt */
            data_descrptr.block_cnt     = temp_block_cnt - data_descrptr.block_cnt;
        }

        /* Do a single block transfer for the eMMC block data which spans the SDMA address alignment boundary */
        if( data_descrptr.block_cnt )
        {
            /* Preserve block_cnt */
            temp_block_cnt = data_descrptr.block_cnt;
            // [3-1] Check last blocks
            data_descrptr.block_cnt     = 1;
            data_descrptr.block_attr    = EMMC_ACCESS_LAST_BLOCK;
            data_descrptr.block_offset  = 0;
            data_descrptr.data_copy_len = data_descrptr.block_size;
            data_descrptr.dma_address   = K0_TO_PHYS( (uint32_t)(uintptr_t)data_descrptr.read_buffer );
            
            // [3-2] Read last blocks
            if( read_write == EMMC_READ )
            {
                return_val = emmc_block_read( data_descrptr, emmc_config, emmc_probe );
            }    
            else
            {
                return_val = emmc_block_write( data_descrptr, emmc_config, emmc_probe );
            }

            data_descrptr.block_address += data_descrptr.data_copy_len;
            data_descrptr.data_offset   += data_descrptr.data_copy_len;

        /* Restore and update block_cnt */
            data_descrptr.block_cnt     = temp_block_cnt - data_descrptr.block_cnt;
            //data_descrptr.block_cnt--;
        }
    }
#else    
    if( data_descrptr.block_cnt != 0 )
    {
        data_descrptr.block_attr    = EMMC_ACCESS_MIDDLE_BLOCK;
        data_descrptr.block_offset  = 0;
        data_descrptr.data_copy_len = data_descrptr.block_size * data_descrptr.block_cnt;
        //data_descrptr.dma_address   = K0_TO_PHYS( (uint32_t)data_descrptr.data_buffer ) + data_descrptr.data_offset;
#if ( __MIPSEB == 1 )
        if( ((uint32_t)(uintptr_t)data_descrptr.data_buffer & 0xE0000000) == 0x80000000 )
            data_descrptr.dma_address   = K0_TO_PHYS( (uint32_t)(uintptr_t)data_descrptr.data_buffer ) 
                                            + data_descrptr.data_offset;   // [Usage] address in DRAM for DMA
        else if( ((uint32_t)(uintptr_t)data_descrptr.data_buffer & 0xE0000000) == 0xA0000000 )
            data_descrptr.dma_address   = K1_TO_PHYS( (uint32_t)(uintptr_t)data_descrptr.data_buffer ) 
                                            + data_descrptr.data_offset;   // [Usage] address in DRAM for DMA
        else
#endif
        data_descrptr.dma_address   = (uint32_t)(uintptr_t)data_descrptr.data_buffer + data_descrptr.data_offset; // [Usage] address in DRAM for DMA

        
        // [2-2] Read middle blocks
        if( read_write == EMMC_READ )
        {
            return_val = emmc_block_read( data_descrptr, emmc_config, emmc_probe );
        }    
        else
        {
            return_val = emmc_block_write( data_descrptr, emmc_config, emmc_probe );
        }
        
        // [2-3] Update for last block
        data_descrptr.block_address += data_descrptr.data_copy_len;
        data_descrptr.data_offset   += data_descrptr.data_copy_len;
    }
#endif

    //------------------------------------------------------
    // [3] Check & read last block
    //printf("[3] Check & read/write last block\n");
    if( range_descrptr.last_block_data_offset != 0 )
    {
        // [3-1] Check last blocks
        data_descrptr.block_cnt     = 1;
        data_descrptr.block_attr    = EMMC_ACCESS_LAST_BLOCK;
        data_descrptr.block_offset  = 0;
        data_descrptr.data_copy_len = range_descrptr.last_block_data_offset;
        data_descrptr.dma_address   = K0_TO_PHYS( (uint32_t)(uintptr_t)data_descrptr.read_buffer );
        
        // [3-2] Read last blocks
        if( read_write == EMMC_READ )
        {
            return_val = emmc_block_read( data_descrptr, emmc_config, emmc_probe );
        }    
        else
        {
            return_val = emmc_block_write( data_descrptr, emmc_config, emmc_probe );
        }
    }

    //------------------------------------------------------
    // [4] Wait for ready for data transfer of eMMC device for next transfer
    if( read_write == EMMC_READ )
    {
        return_val = emmc_Wait_ReadyDataXfer( emmc_config, emmc_config->config.InterruptSleep, 178 );
    }
    else
    {
        return_val = emmc_Wait_ReadyDataXfer( emmc_config, emmc_config->config.InterruptSleep, 245 );
    }
    
#if DEBUG_EMMC_SPEED
    EndTime = _getticks( );
    RdWrTimeUsec = (uint32_t)((EndTime-StartTime)/EMMC_TICKS_USEC);
    if( read_write == EMMC_READ )
    {
        printf(" Time  To Read             : %d [us]\n", RdWrTimeUsec);
        printf(" Time  To Read (Avg/block) : %d [us]\n", RdWrTimeUsec/range_descrptr.num_block_access);
        
        //KByteRate = (range_descrptr.num_block_access*emmc_config->config.ReadBlkLen) / RdWrTimeUsec * 1024;   : Original Equation
        KByteRate = range_descrptr.num_block_access*emmc_config->config.WriteBlkLen;
        
        if( KByteRate > RdWrTimeUsec )
            KByteRate = (KByteRate / RdWrTimeUsec) * 1024;
        else
            KByteRate = (KByteRate * 1024) / RdWrTimeUsec ;
        
        if( KByteRate > 1024 )
            printf(" Speed To Read             : %d [MB/s]\n", KByteRate/1024);
        else
            printf(" Speed To Read             : %d [KB/s]\n", KByteRate);
    }
    else
    {
        printf(" Time  To Write             : %d [us]\n", RdWrTimeUsec);
        printf(" Time  To Write (Avg/block) : %d [us]\n", RdWrTimeUsec/range_descrptr.num_block_access);
        
        KByteRate = range_descrptr.num_block_access*emmc_config->config.WriteBlkLen;
        
        if( KByteRate > RdWrTimeUsec )
            KByteRate = (KByteRate / RdWrTimeUsec) * 1024;
        else
            KByteRate = (KByteRate * 1024) / RdWrTimeUsec ;
        
        if( KByteRate > 1024 )
            printf(" Speed To Write             : %d [MB/s]\n", KByteRate/1024);
        else
            printf(" Speed To Write             : %d [KB/s]\n", KByteRate);
    }
    printf("----------------------------\n");
    //--------------------------------------
#endif

    (void)return_val;
    
    return 0;
}


/*  *********************************************************************
    *  emmc_block_read(data_descrptr, emmcdev_ptr, emmc_probe_ptr)
    *
    *  Read emmc as block.
    *
    *  Input parameters:
    *       data_descrptr - data descriptor
    *       emmcdev_ptr   - emmc device
    *       emmcflash_probe_t - emmcflash_probe
    *
    *  Return value:
    *      nothing
    ********************************************************************* */
uint32_t emmc_block_read( emmc_data_descriptor_t data_descrptr, emmcdev_t *emmcdev, emmcflash_probe_t *emmc_probe )
{
    uint32_t i;
    uint32_t return_val=0;
    uint32_t *emmc_buf=0;
    uint32_t block_address=0, dma_address=0;

#if ( __MIPSEB == 1 )
    uint32_t j;
    uint32_t temp32d=0, temp32r=0;
    uint32_t temp32_0=0, temp32_1=0, temp32_2=0, temp32_3=0;
    uint32_t dma_offset=0;
#endif

#if !EMMC_UNALIGNED_DMA 
    // For non-aligned 256K (buffer size)
    uint32_t block_inc=0;
    uint32_t num_blocks=0;
    int32_t  numRemainedBlocks;
    uint8_t  alignedAddrShiftBit;
    uint32_t minAlignedAddr=0;
    uint32_t maxBlocks4AlignedAddr=0;
    uint32_t maxAlignedAddrMask;
#endif  
    
#if DEBUG_EMMC_DRV_DISPATCH
    printf("[BLOCK READ]\n");
    printf(" data_descrptr.block_address    = %d (0x%X)\n", data_descrptr.block_address, data_descrptr.block_address);
    printf(" data_descrptr.block_attr    = %d (0x%X)\n", data_descrptr.block_attr, data_descrptr.block_attr);
    printf(" data_descrptr.block_offset  = 0x%llX\n", data_descrptr.block_offset);
    printf(" data_descrptr.data_copy_len = %d (0x%X)\n", data_descrptr.data_copy_len, data_descrptr.data_copy_len);
    printf(" data_descrptr.block_cnt      = %d (0x%X)\n", data_descrptr.block_cnt, data_descrptr.block_cnt);
    printf(" data_descrptr.dma_address   = %d (0x%X)\n", data_descrptr.dma_address, data_descrptr.dma_address);
    printf(" data_descrptr.data_offset   = %d (0x%X)\n", data_descrptr.data_offset, data_descrptr.data_offset);
#endif
    
    // Set-up Sleep time for reading    
    if( data_descrptr.block_attr == EMMC_READ_FIRST_BLOCK || data_descrptr.block_attr == EMMC_READ_LAST_BLOCK || data_descrptr.block_attr == EMMC_READ_LESS_THAN_BLOCK )
    {
        // The read_buffer will be read.
        // Wait for ready to read the block
        return_val = emmc_Wait_ReadyDataXfer( emmcdev, emmcdev->config.InterruptSleep, 17 );
        
        cfe_flushcache(CFE_CACHE_FLUSH_D);
        
            if( emmcdev->config.OCR_SectorMode == EMMC_OFF )
        {
                block_address = data_descrptr.block_address;
            }
            else
            {
                block_address = data_descrptr.block_address>>emmcdev->config.ReadBlkLenBit4Addr;
        }
            
        //return_val = emmc_CMD16_C2br_acR1_SET_BLOCKLEN( emmcdev->config.ReadBlkLen, emmcdev );        
        //return_val = emmc_CMD23_C4bw_acR1_SET_BLOCK_COUNT( 1, EMMC_OFF, EMMC_OFF, EMMC_OFF, EMMC_OFF, emmcdev );
        return_val = emmc_CMD17_C2br_adtcR1_READ_SINGLE_BLOCK( block_address, data_descrptr.dma_address, emmcdev->config.ReadBlkLen, emmc_buf, emmcdev );
           
#if ( __MIPSEB == 1 )
        if( emmcdev->config.CFEBigEndian == EMMC_ON )
        {
            /* word access */
            for ( j = 0; j < emmcdev->config.ReadBlkLen; j+=4 )
            {
                temp32r = *(uint32_t*)(&data_descrptr.read_buffer[j]);
                temp32_0 = (temp32r & 0x000000FF)<<24;
                temp32_1 = (temp32r & 0x0000FF00)<<8;
                temp32_2 = (temp32r & 0x00FF0000)>>8;
                temp32_3 = (temp32r & 0xFF000000)>>24;
                *(uint32_t*)(&data_descrptr.read_buffer[j]) = temp32_0 + temp32_1 + temp32_2 + temp32_3;
            }
        }
#endif
        // now modify the data with the new data
        for( i = 0; i < data_descrptr.data_copy_len; i++ )
        {
            data_descrptr.data_buffer[ i + data_descrptr.data_offset ] = data_descrptr.read_buffer[ i + data_descrptr.block_offset ];
        }
        
        // Cache flushing
        cfe_flushcache(CFE_CACHE_FLUSH_D);
    }
    else    /* data_descrptr.block_attr == EMMC_READ_MIDDLE_BLOCK || data_descrptr.block_attr == EMMC_READ_MIDDLE_BLOCK_4_WRITE */
    {   
#if EMMC_UNALIGNED_DMA      
        dma_address = data_descrptr.dma_address;
        if( emmcdev->config.OCR_SectorMode == EMMC_OFF )
            block_address = data_descrptr.block_address;
        else
            block_address = (data_descrptr.block_address>>emmcdev->config.ReadBlkLenBit4Addr);

        //[3] Wait for ready to read the block and Read data
        return_val = emmc_Wait_ReadyDataXfer( emmcdev, emmcdev->config.InterruptSleep, 18 );
        cfe_flushcache(CFE_CACHE_FLUSH_D);
        return_val = emmc_CMD18_C2br_adtcR1_READ_MULTIPLE_BLOCK( block_address, dma_address, data_descrptr.block_cnt, emmcdev->config.ReadBlkLen, emmc_buf, emmcdev );
            
#if ( __MIPSEB == 1 )
        if( emmcdev->config.CFEBigEndian == EMMC_ON )
        {
            /* word access */
            if( data_descrptr.block_attr == EMMC_READ_MIDDLE_BLOCK_4_WRITE )
            {
                for ( j = 0; j < emmcdev->config.ReadBlkLen; j+=4 )
                {
                    temp32r = *(uint32_t*)(&data_descrptr.read_buffer[j]);
                    temp32_0 = (temp32r & 0x000000FF)<<24;
                    temp32_1 = (temp32r & 0x0000FF00)<<8;
                    temp32_2 = (temp32r & 0x00FF0000)>>8;
                    temp32_3 = (temp32r & 0xFF000000)>>24;
                    *(uint32_t*)(&data_descrptr.read_buffer[j]) = temp32_0 + temp32_1 + temp32_2 + temp32_3;
                }
            }
            else
            {
                for ( j = 0; j < data_descrptr.block_cnt*emmcdev->config.ReadBlkLen; j+=4 )
                {
                    temp32d = *(uint32_t*)(&data_descrptr.data_buffer[data_descrptr.data_offset+dma_offset+j]);
                    temp32_0 = (temp32d & 0x000000FF)<<24;
                    temp32_1 = (temp32d & 0x0000FF00)<<8;
                    temp32_2 = (temp32d & 0x00FF0000)>>8;
                    temp32_3 = (temp32d & 0xFF000000)>>24;
                    *(uint32_t*)(&data_descrptr.data_buffer[data_descrptr.data_offset+dma_offset+j]) = temp32_0 + temp32_1 + temp32_2 + temp32_3;
                }
            }
        }
#endif
#else           
        // New approach based on aligned information.
        // [Step 0] Initialzie Varialbles
        maxAlignedAddrMask = MAX_ALIGNED_ADDR_MASK;
        numRemainedBlocks = (int32_t)data_descrptr.block_cnt;
        alignedAddrShiftBit = 0;
        (void)alignedAddrShiftBit;
        block_inc = 0;
        //addrOffset = 0;
        
        dma_address = data_descrptr.dma_address;
        if( emmcdev->config.OCR_SectorMode == EMMC_OFF )
            block_address = data_descrptr.block_address;
        else
            block_address = (data_descrptr.block_address>>emmcdev->config.ReadBlkLenBit4Addr);

        while( numRemainedBlocks > 0 )
        {
            //[1] Check alignment
            if( (dma_address & maxAlignedAddrMask) == 0 )
            {
                minAlignedAddr = MAX_ALIGNED_ADDR;
                maxBlocks4AlignedAddr = ARASAN_SDMA_MAX_BLKS;
            }
            else
            {
                // Non-aligned 256K (buffer size) case.
                // but dma_address must be aligned to block size(512 or 4K).
                minAlignedAddr = MAX_ALIGNED_ADDR;
                maxBlocks4AlignedAddr = (minAlignedAddr - (dma_address & maxAlignedAddrMask))>> emmcdev->config.ReadBlkLenBit4Addr;
            }
            
            //[2] Calculate block_address, dma_address,
            if( numRemainedBlocks > maxBlocks4AlignedAddr )
            {
                num_blocks = maxBlocks4AlignedAddr;
            }
            else
            {   
                num_blocks = numRemainedBlocks;
            }
            
                    //[3] Wait for ready to read the block and Read data
            return_val = emmc_Wait_ReadyDataXfer( emmcdev, emmcdev->config.InterruptSleep, 18 );
            cfe_flushcache(CFE_CACHE_FLUSH_D);
            return_val = emmc_CMD18_C2br_adtcR1_READ_MULTIPLE_BLOCK( block_address, dma_address, num_blocks, emmcdev->config.ReadBlkLen, emmc_buf, emmcdev );
            
#if ( __MIPSEB == 1 )
            if( emmcdev->config.CFEBigEndian == EMMC_ON )
            {
                /* word access */
                if( data_descrptr.block_attr == EMMC_READ_MIDDLE_BLOCK_4_WRITE )
                {
                    for ( j = 0; j < emmcdev->config.ReadBlkLen; j+=4 )
                    {
                        temp32r = *(uint32_t*)(&data_descrptr.read_buffer[j]);
                        temp32_0 = (temp32r & 0x000000FF)<<24;
                        temp32_1 = (temp32r & 0x0000FF00)<<8;
                        temp32_2 = (temp32r & 0x00FF0000)>>8;
                        temp32_3 = (temp32r & 0xFF000000)>>24;
                        *(uint32_t*)(&data_descrptr.read_buffer[j]) = temp32_0 + temp32_1 + temp32_2 + temp32_3;
                    }
                }
                else
                {
                    for ( j = 0; j < num_blocks*emmcdev->config.ReadBlkLen; j+=4 )
                    {
                        temp32d = *(uint32_t*)(&data_descrptr.data_buffer[data_descrptr.data_offset+dma_offset+j]);
                        temp32_0 = (temp32d & 0x000000FF)<<24;
                        temp32_1 = (temp32d & 0x0000FF00)<<8;
                        temp32_2 = (temp32d & 0x00FF0000)>>8;
                        temp32_3 = (temp32d & 0xFF000000)>>24;
                        *(uint32_t*)(&data_descrptr.data_buffer[data_descrptr.data_offset+dma_offset+j]) = temp32_0 + temp32_1 + temp32_2 + temp32_3;
                    }
                }
                
                dma_offset += num_blocks*emmcdev->config.ReadBlkLen;
            }
#endif

            //[4] Update parameters for next bigger aligned address
            block_inc += num_blocks;            //maxBlocks4AlignedAddr;
            numRemainedBlocks -= num_blocks;
            if( emmcdev->config.OCR_SectorMode == EMMC_OFF )
            {
                block_address = data_descrptr.block_address + block_inc*emmcdev->config.ReadBlkLen;
            }
            else
            {
                block_address = (data_descrptr.block_address>>emmcdev->config.ReadBlkLenBit4Addr) + block_inc;
            }

            dma_address = data_descrptr.dma_address + (block_inc<<emmcdev->config.ReadBlkLenBit4Addr);
        }
#endif  
    }

    // Back-up Sleep time
    emmcdev->config.InterruptSleep = EMMC_INT_SLEEP;    // e.g. 600[ms] ( NAC @ 50MHz = 400ms ) x 1.5
    
    /* in case of a uncorr error the return val would be -1 */
    return return_val;
}


/*  *********************************************************************
    *  emmc_block_write(data_descrptr, emmcdev_ptr, emmc_probe_ptr)
    *
    *  Write emmc as block.
    *
    *  Input parameters:
    *       data_descrptr - data descriptor
    *       emmcdev_ptr   - emmc device
    *       emmcflash_probe_t - emmcflash_probe
    *
    *  Return value:
    *      nothing
    ********************************************************************* */
/* emmc_block_write:
 * This will write the data from the data buffer to the block specified by the address.
 * The address always need to be block alligned.
 * There can be four types of block writes:
 *      - Program the whole block
 *      - Program the bottom part of the block with the new data in the buffer,
 *        but preserve the old data in the top part.
 *      - Program the top part of the block but preserve the bottop part.
 *      - Program only a portion of a block.
 */
uint32_t emmc_block_write( emmc_data_descriptor_t data_descrptr, emmcdev_t *emmcdev, emmcflash_probe_t *emmc_probe )
{
    uint32_t i;
    uint32_t return_val=0;
    uint32_t *emmc_buf=0;
    uint32_t block_address=0, dma_address=0;
    uint32_t block_attr_tmp=0;
#if ( __MIPSEB == 1 )
    uint32_t j;
    uint32_t temp32d=0, temp32r=0;
    uint32_t temp32_0=0, temp32_1=0, temp32_2=0, temp32_3=0;
    uint32_t dma_offset=0;
#endif

#if !EMMC_UNALIGNED_DMA 
    // For non-aligned 256K (buffer size)
    uint32_t block_inc=0;
    uint32_t num_blocks=0;
    int32_t  numRemainedBlocks;
    uint8_t  alignedAddrShiftBit;
    uint32_t minAlignedAddr=0;
    uint32_t maxBlocks4AlignedAddr=0;
    uint32_t maxAlignedAddrMask;
#endif
    
#if DEBUG_EMMC_DRV_DISPATCH
    printf("[BLOCK WRITE]\n");
    printf(" data_descrptr.block_address = %d (0x%X)\n", data_descrptr.block_address, data_descrptr.block_address);
    printf(" data_descrptr.block_attr    = %d (0x%X)\n", data_descrptr.block_attr, data_descrptr.block_attr);
    printf(" data_descrptr.block_offset  = 0x%llX\n", data_descrptr.block_offset);
    printf(" data_descrptr.data_copy_len = %d (0x%X)\n", data_descrptr.data_copy_len, data_descrptr.data_copy_len);
    printf(" data_descrptr.block_cn      = %d (0x%X)\n", data_descrptr.block_cnt, data_descrptr.block_cnt);
    printf(" data_descrptr.dma_address   = %d (0x%X)\n", data_descrptr.dma_address, data_descrptr.dma_address);
    printf(" data_descrptr.data_offset   = %d (0x%X)\n", data_descrptr.data_offset, data_descrptr.data_offset);
#endif
    // Set-up Sleep time for reading    
    if ( data_descrptr.block_attr == EMMC_WRITE_FIRST_BLOCK || data_descrptr.block_attr == EMMC_WRITE_LAST_BLOCK || data_descrptr.block_attr == EMMC_WRITE_LESS_THAN_BLOCK )
    {
        if( emmcdev->config.OCR_SectorMode == EMMC_OFF )
        {
            block_address = data_descrptr.block_address;
        }
        else
        {
            block_address = (data_descrptr.block_address>>emmcdev->config.WriteBlkLenBit4Addr);
        }

                /* Calling function must ensure that data_descrptr.dma_address must 
                 * point to read_buffer in the case of single/partial block writes 
                 * */
        dma_address   = data_descrptr.dma_address;
            
        /* Read the block first if we need to do a partial block write */
                if( data_descrptr.block_offset )
                {
            block_attr_tmp = data_descrptr.block_attr;
#if ( __MIPSEB == 1 )
            data_descrptr.block_attr = EMMC_READ_MIDDLE_BLOCK_4_WRITE;  // Cheat emmc_block_read( )
#else
            data_descrptr.block_attr = EMMC_READ_MIDDLE_BLOCK;          // Cheat emmc_block_read( )
#endif        
            /*cfe_usleep( emmcdev->config.InterruptSleep );  // [usec]*/
            return_val = emmc_block_read( data_descrptr, emmcdev, emmc_probe );
            data_descrptr.block_attr = block_attr_tmp;
                }
        
        /* now modify the data with the new data */
        for ( i = 0; i < data_descrptr.data_copy_len; i++ )
        {
            data_descrptr.read_buffer[ i + data_descrptr.block_offset ] = data_descrptr.data_buffer[ i + data_descrptr.data_offset ];
        }
        
#if ( __MIPSEB == 1 )
        if( emmcdev->config.CFEBigEndian == EMMC_ON )
        {
            /* word access : Swap to write by SDIO SDMA */
            for ( j = 0; j < emmcdev->config.WriteBlkLen; j+=4 )
            {
                temp32r = *(uint32_t*)(&data_descrptr.read_buffer[j]);
                temp32_0 = (temp32r & 0x000000FF)<<24;
                temp32_1 = (temp32r & 0x0000FF00)<<8;
                temp32_2 = (temp32r & 0x00FF0000)>>8;
                temp32_3 = (temp32r & 0xFF000000)>>24;
                *(uint32_t*)(&data_descrptr.read_buffer[j]) = temp32_0 + temp32_1 + temp32_2 + temp32_3;
            }
        }
#endif
        // The read_buffer will be written.
        // Wait for ready to write the block
        return_val = emmc_Wait_ReadyDataXfer( emmcdev, emmcdev->config.InterruptSleep, 24 );
        
        cfe_flushcache(CFE_CACHE_FLUSH_D);

        //return_val = emmc_CMD16_C2br_acR1_SET_BLOCKLEN( emmcdev->config.WriteBlkLen, emmcdev ); 
        //return_val = emmc_CMD23_C4bw_acR1_SET_BLOCK_COUNT( 1, EMMC_OFF, EMMC_OFF, EMMC_OFF, EMMC_OFF, emmcdev );
        return_val = emmc_CMD24_C4bw_adtcR1_WRITE_BLOCK( block_address, dma_address, emmcdev->config.WriteBlkLen, emmc_buf, emmcdev );
        
#if ( __MIPSEB == 1 )
        if( emmcdev->config.CFEBigEndian == EMMC_ON )
        {
            /* word access : Recover data for verify */
            for ( j = 0; j < emmcdev->config.WriteBlkLen; j+=4 )
            {
                temp32r = *(uint32_t*)(&data_descrptr.read_buffer[j]);
                temp32_0 = (temp32r & 0x000000FF)<<24;
                temp32_1 = (temp32r & 0x0000FF00)<<8;
                temp32_2 = (temp32r & 0x00FF0000)>>8;
                temp32_3 = (temp32r & 0xFF000000)>>24;
                *(uint32_t*)(&data_descrptr.read_buffer[j]) = temp32_0 + temp32_1 + temp32_2 + temp32_3;
            }
        }
#endif
    }
    else
    {
#if EMMC_UNALIGNED_DMA
        dma_address = data_descrptr.dma_address;
        if( emmcdev->config.OCR_SectorMode == EMMC_OFF )
            block_address = data_descrptr.block_address;
        else
            block_address = (data_descrptr.block_address>>emmcdev->config.WriteBlkLenBit4Addr);

#if ( __MIPSEB == 1 )
        if( emmcdev->config.CFEBigEndian == EMMC_ON )
        {
            /* word access : Swap to write by SDIO SDMA */
            for ( j = 0; j < data_descrptr.block_cnt*emmcdev->config.WriteBlkLen; j+=4 )
            {
                temp32d = *(uint32_t*)(&data_descrptr.data_buffer[data_descrptr.data_offset+dma_offset+j]);
                temp32_0 = (temp32d & 0x000000FF)<<24;
                temp32_1 = (temp32d & 0x0000FF00)<<8;
                temp32_2 = (temp32d & 0x00FF0000)>>8;
                temp32_3 = (temp32d & 0xFF000000)>>24;
                *(uint32_t*)(&data_descrptr.data_buffer[data_descrptr.data_offset+dma_offset+j]) = temp32_0 + temp32_1 + temp32_2 + temp32_3;
            }
            //dma_offset += data_descrptr.block_cnt*emmcdev->config.WriteBlkLen;
        }           
#endif
        //[3] Wait for ready to read the block and Read data
        return_val = emmc_Wait_ReadyDataXfer( emmcdev, emmcdev->config.InterruptSleep, 24 );
        cfe_flushcache(CFE_CACHE_FLUSH_D);
        return_val = emmc_CMD25_C4bw_adtcR1_WRITE_MULTIPLE_BLOCK( block_address, dma_address, data_descrptr.block_cnt, emmcdev->config.WriteBlkLen, emmc_buf, emmcdev );
        
#else                        
        // [Step 0] Initialzie Varialbles
        maxAlignedAddrMask = MAX_ALIGNED_ADDR_MASK;
        numRemainedBlocks = (int32_t)data_descrptr.block_cnt;
        alignedAddrShiftBit = 0;
        (void)alignedAddrShiftBit;
        block_inc = 0;
        //addrOffset = 0;
            
        dma_address = data_descrptr.dma_address;
        if( emmcdev->config.OCR_SectorMode == EMMC_OFF )
            block_address = data_descrptr.block_address;
        else
            block_address = (data_descrptr.block_address>>emmcdev->config.WriteBlkLenBit4Addr);

        while( numRemainedBlocks > 0 )
        {
            //[1] Check alignment
            if( (dma_address & maxAlignedAddrMask) == 0 )
            {
                minAlignedAddr = MAX_ALIGNED_ADDR;
                maxBlocks4AlignedAddr = ARASAN_SDMA_MAX_BLKS;
            }
            else
            {
                // Non-aligned 256K (buffer size) case.
                // but dma_address must be aligned to block size(512 or 4K).
                minAlignedAddr = MAX_ALIGNED_ADDR;
                maxBlocks4AlignedAddr = (minAlignedAddr - (dma_address & maxAlignedAddrMask))>> emmcdev->config.ReadBlkLenBit4Addr;
            }
    
            //[2] Calculate block_address, dma_address,
            if( numRemainedBlocks > maxBlocks4AlignedAddr )
            {
                num_blocks = maxBlocks4AlignedAddr;
            }
            else
            {   
                    num_blocks = numRemainedBlocks;
            }
            
#if ( __MIPSEB == 1 )
            if( emmcdev->config.CFEBigEndian == EMMC_ON )
            {
                /* word access : Swap to write by SDIO SDMA */
                for ( j = 0; j < num_blocks*emmcdev->config.WriteBlkLen; j+=4 )
                {
                    temp32d = *(uint32_t*)(&data_descrptr.data_buffer[data_descrptr.data_offset+dma_offset+j]);
                    temp32_0 = (temp32d & 0x000000FF)<<24;
                    temp32_1 = (temp32d & 0x0000FF00)<<8;
                    temp32_2 = (temp32d & 0x00FF0000)>>8;
                    temp32_3 = (temp32d & 0xFF000000)>>24;
                    *(uint32_t*)(&data_descrptr.data_buffer[data_descrptr.data_offset+dma_offset+j]) = temp32_0 + temp32_1 + temp32_2 + temp32_3;
                }
                //dma_offset += num_blocks*emmcdev->config.WriteBlkLen;
            }           
#endif
            //[3] Wait for ready to read the block and Read data
            return_val = emmc_Wait_ReadyDataXfer( emmcdev, emmcdev->config.InterruptSleep, 24 );
            cfe_flushcache(CFE_CACHE_FLUSH_D);
            return_val = emmc_CMD25_C4bw_adtcR1_WRITE_MULTIPLE_BLOCK( block_address, dma_address, num_blocks, emmcdev->config.WriteBlkLen, emmc_buf, emmcdev );
            
            //[4] Update parameters for next bigger aligned address
            block_inc += num_blocks;            //maxBlocks4AlignedAddr;
            numRemainedBlocks -= num_blocks;
            if( emmcdev->config.OCR_SectorMode == EMMC_OFF )
            {
                block_address = data_descrptr.block_address + block_inc*emmcdev->config.WriteBlkLen;
            }
            else
            {
                block_address = (data_descrptr.block_address>>emmcdev->config.WriteBlkLenBit4Addr) + block_inc;
            }
                
            dma_address = data_descrptr.dma_address + (block_inc<<emmcdev->config.WriteBlkLenBit4Addr);
        }
#endif
    }
    // Back-up Sleep time
    emmcdev->config.InterruptSleep = EMMC_INT_SLEEP;    // e.g. 600[ms] ( NAC @ 50MHz = 400ms ) x 1.5

    return return_val;
}


/*  *********************************************************************
    *  emmc_get_range_intersection ( uint32_t range_base, uint32_t range_len, uint32_t blk_size, uint8_t blk_length_bit, emmc_range_descriptor_t* rdt  )
    *
    *  Input parameters:
    *      range_base - Base address in eMMC
    *      range_len - length to transfer
    *      blk_size - block size
    *  Output parameters:
    *      emmc_range_descriptor_t - new_range_base, first_block_data_offset, last_block_data_offset, num_block_access
    *  Return value:
    *      0 if successful, <0 if error
    ********************************************************************* */
static int64_t emmc_get_range_intersection ( uint64_t range_base, uint64_t range_len, uint32_t blk_size, uint8_t blk_length_bit, emmc_range_descriptor_t* rdt  )
{
    uint64_t size, extra_byte;
    
    rdt->first_block_data_offset = 0;
    rdt->last_block_data_offset = 0;
    rdt->new_range_base = range_base;
    rdt->num_block_access = 0;
    
    size = range_len;
    
#if DEBUG_EMMC_DRV
    printf("######### emmc_get_range_intersection   called with range_base = 0x%x, range_len = %d\n", range_base, range_len );
#endif
    
    /* first align the address to blk boundary */
    rdt->new_range_base = range_base & (~((uint64_t)blk_size-1));
    rdt->first_block_data_offset = range_base - rdt->new_range_base;
    
    if ( rdt->first_block_data_offset > 0 )
    {
        rdt->num_block_access = 1;
        
        if ( range_len <= (uint64_t)(blk_size - rdt->first_block_data_offset) )
        {
            /* the data is less than 1 blk */
        
            goto end_fn;
        }
        size -= (uint64_t)(blk_size - rdt->first_block_data_offset);
    }
    
    /* the last blk may not cover a full block */
    //if ( size % (uint64_t)blk_size != 0 )
    extra_byte = size & ((uint64_t)blk_size-1);
    if ( extra_byte != 0 )
    {
        rdt->last_block_data_offset = extra_byte;
        size -= (uint64_t)rdt->last_block_data_offset;
        rdt->num_block_access++;
    }
    
    /* now the data should be blk aligned */
    //rdt->num_block_access += (uint32_t)(size / blk_size);
    rdt->num_block_access += (uint32_t)(size >> blk_length_bit);
    
    end_fn:
    return 0;
}


/*  *********************************************************************
    *  emmc_erase_block(blk_addr, blk_size)
    *
    *  Erase a range of sectors
    *
    *  Input parameters:
    *      blk_addr - our flash
    *      blk_size - range structure
    *
    *  Return value:
    *      0 if ok
    *      else error
    *
    ********************************************************************* */
#if EMMC_DRV_FUTURE
int32_t emmc_erase_block (uint32_t blk_addr, uint32_t blk_size)
{
#if 0
    int32_t rval;

#if NAND_DEBUG_LELVEL > 2
    printf("emmc_erase_block called: blk addr = 0x\r\n", blk_addr);
#endif

    rval = 0;
    blk_addr = blk_addr & (~(blk_size -1 ));

    rval = emmc_get_block_status ( blk_addr, blk_size, pg_size, bbi_map );
    if ( rval == 0 )
    {
        rval =  do_emmc_cmd ( NAND_CMD_BLOCK_ERASE, blk_addr );

        if ( rval != 0 )
        {
            printf("block_erase_failed\n");
        }
    }
    else
    {
#if NAND_DEBUG_LELVEL > 2
        printf("emmc_erase_block faild: at blk addr = 0x\r\n", blk_addr);
#endif
    }

    return rval;
#endif

    return 0;
}


/*  *********************************************************************
    *  emmc_erase_all(softc)
    *
    *  Erase the entire flash device, except the NVRAM area,
    *  sector-by-sector.
    *
    *  Input parameters:
    *      softc - our flash
    *
    *  Return value:
    *      0 if ok
    *      else error code
    ********************************************************************* */
static int emmc_erase_all ( emmcflashdev_t *softc )
{
    return 0;
}
#endif

/*  *********************************************************************
    *  emmc_erase_range(softc,range)
    *
    *  Erase a range of sectors
    *
    *  Input parameters:
    *      softc - our flash
    *      range - range structure
    *
    *  Return value:
    *      0 if ok
    *      else error
    *
    ********************************************************************* */
static int emmc_erase_range ( emmcflashdev_t *softc, flash_range_t *range )
{
    emmcflash_probe_t   *emmc_probe = &softc->fd_probe;   
    emmcdev_t           *emmc_config = &emmc_probe->emmc_config;

    int blockstatus;
    long erase_start_addr, erase_end_addr;

    int32_t blocksize = emmc_get_block_size( &softc->fd_probe );

    if (blocksize <= 0) {
        printf("invalid block size %d\n", blocksize);
        return -1;
    }

    if( emmc_config->config.OCR_SectorMode == EMMC_ON )
    {
        erase_start_addr = range->range_base >> emmc_config->config.ReadBlkLenBit4Addr;
        erase_end_addr   = (range->range_base + range->range_length) >> emmc_config->config.ReadBlkLenBit4Addr;
    }

    printf("%s: Erasing block range 0x%08x-0x%08x\n", __FUNCTION__, erase_start_addr, erase_end_addr);
    blockstatus = emmc_CMD35_C5er_acR1_ERASE_GROUP_START( erase_start_addr, emmc_config );

    if( blockstatus == EMMC_STATUS_ERROR )
        goto erase_err;

    blockstatus = emmc_CMD36_C5er_acR1_ERASE_GROUP_END( erase_end_addr, emmc_config );

    if( blockstatus == EMMC_STATUS_ERROR )
        goto erase_err;

    /* Issue TRIM command to erase writeblocks */
    blockstatus = emmc_CMD38_C5er_acR1b_ERASE( 0, 1, emmc_config );

    if( emmc_Wait_ReadyDataXfer( emmc_config, emmc_config->config.InterruptSleep, 38 ) == EMMC_NG )
        blockstatus = EMMC_STATUS_ERROR;    

erase_err:
    if(blockstatus == EMMC_STATUS_ERROR)
    {
        printf("Erase address  0x%lx failed, bad block?\n", erase_start_addr);
        /* seems ok to return 0; bad block will be marked during write */
    }
    
    return (0);
}

//===========================================================


//===========================================================
// Macro Command Implementation
//  CIM_SINGLE_DEVICE_ACQ : Starts an identification cycle of a single Device.
//  CIM_SETUP_DEVICE      : Select a Device by writing the RCA and reads its CSD.
//  CIM_READ_BLOCK        : Sets the block length and the starting address and reads a data block from the Device.
//  CIM_READ_MBLOCK       : Sets the block length and the starting address and reads (continuously) data blocks from the Device. Data transfer is terminated by a stop command.
//  CIM_WRITE_BLOCK       : Sets the block length and the starting address and writes a data block from the Device.
//  CIM_WRITE_MBLOCK      : Sets the block length and the starting address and writes (continuously) data blocks to the Device. Data transfer is terminated by a stop command.
//  CIM_ERASE_GROUP       : Erases a range of erase groups on the Device.
//  CIM_TRIM              : Erases a number of ranges of write blocks on the Device.
//  CIM_US_PWR_WP         : Applies power-on write protection to a write protection group on the Device.
//  CIM_US_PERM_WP        : Applies permanent write protection to a write protection group on the Device.
//-------------------------------------------------

// eMMC Initialization
int emmc_Initialize( emmcdev_t *emmcdev )
{
    int eMMC_check;
    uint32_t eMMC_Boot_Clk, numClkRescan=0;

    printf("Initializing eMMC (v0.94). 2013.11.12.\n");
#if DEBUG_EMMC_INIT
    printf("------------------------------\n");
    printf(" Start : emmc_Initialize( ) \n");
#endif
    
#if defined(_BCM963158_)
    if(UtilGetChipRev() == 0xA0)
    {
        EMMC_HOSTIF = (volatile EmmcHostIfRegs *) EMMC_HOSTIF_BASE_A0;
        EMMC_TOP_CFG = (volatile EmmcTopCfgRegs *) EMMC_TOP_CFG_BASE_A0;
        EMMC_BOOT = (volatile EmmcBootRegs *) EMMC_BOOT_BASE_A0;
    }
#endif

    //--------------------------------------------
    //[Step 0] SMDA_OFF, BigEndian detection
    emmcdev->config.SDMA_On = EMMC_OFF;    
#if ( __MIPSEB == 1 )
    emmcdev->config.CFEBigEndian = EMMC_ON;
#else
    emmcdev->config.CFEBigEndian = EMMC_OFF;       
#endif
        //--------------------------------------------
    
    eMMC_Boot_Clk = EMMC_BOOT_CLK;

    do
    {
    //--------------------------------------------
    // [Step 1] Initialize eMMC configuration parameters
    emmc_config_init( emmcdev );
    
    // [Step 2] Host(STB Chip) Configuration
    emmc_Host_Config( emmcdev );
    //--------------------------------------------
    
    //--------------------------------------------
    // {Step 3~5] Standard sequence
    // [Step 3]
        emmc_Boot_Mode( emmcdev, eMMC_Boot_Clk );
    
    // [Step 4]
    eMMC_check = emmc_DeviceID_Mode_SingleDevice( emmcdev );
    
        numClkRescan++;
        eMMC_Boot_Clk += 0x20;
        if( eMMC_Boot_Clk > 0xFF )  eMMC_Boot_Clk = 0xff;
    } while( eMMC_check == EMMC_NG && numClkRescan < 3 );

    // [Step 5]
    if( eMMC_check == EMMC_NG )
    {
        printf("\n\n !!! Warning : eMMC wasn't detected ( Rescan = %d )!!!\n\n", numClkRescan);
        return EMMC_ERROR;
    }
    else
    {
        emmc_Setup_DataTransfer_Mode( emmcdev );
    }    
    //--------------------------------------------
    
#if DEBUG_EMMC_INIT
    printf(" End : emmc_Initialize( )\n");
    printf("------------------------------\n");
#endif

#if DEBUG_EMMC_INIT
    emmc_Print_CID( emmcdev );
    emmc_Print_CSD( emmcdev );
    emmc_Print_ExtCSD( emmcdev );
    emmc_Print_HostAndEmmcInfo( emmcdev );
#endif

#if DEBUG_EMMC_SLEEP_TIMER
    emmc_Check_SleepTime( );
#endif

    //--------------------------------------------
    //[Step 6] SMDA_ON
    emmcdev->config.SDMA_On = EMMC_ON;
    //--------------------------------------------

    return EMMC_NO_ERROR;
}


void emmc_config_init( emmcdev_t *emmcdev )
{
    //--------------------------------------------
    // [Step 1] EMMC flash data initialization
    // eMMC Device
    emmcdev->RCA                    = EMMC_RCA;             // For single device, means 2, 0x00020000 For argument register
    //emmcdev->config.OCR_SectorMode  = EMMC_OCR_SECTOR_MODE; // ON(>2GB), OFF(<=2GB), Hynix H26M42001FMR(8GB), H26M54001DQR(16GB)
    // Bus Mode : Speed, Width, Voltage
    emmcdev->config.HSTiming        = EMMC_HS_TIMING;   // HS_TIMING_HS, HS_TIMING_FULL
    emmcdev->config.HostHS_On       = EMMC_HOST_HS;     // EMMC_ON, EMMC_OFF
    emmcdev->config.BusFreq         = EMMC_BUS_FREQ;    // BUS_FREQ_50MHZ, BUS_FREQ_25MHZ, BUS_FREQ_13MHZ, BUS_FREQ_06MHZ, BUS_FREQ_03MHZ
    emmcdev->config.BusWidth        = EMMC_BUS_WIDTH;   // BUS_WIDTH_8BIT, BUS_WIDTH_4BIT, BUS_WIDTH_1BIT
    emmcdev->config.BusVoltage      = EMMC_BUS_VOLTAGE; // BUS_VOLTAGE_33, BUS_VOLTAGE_30, BUS_VOLTAGE_18
    // Block Size & Control
    //emmcdev->config.HighCap_On      = EMMC_OCR_SECTOR_MODE;  // ON(>2GB), OFF(<=2GB)
    emmcdev->config.ReadTimeout     = EMMC_BOOT_SLEEP;
    emmcdev->config.WriteTimeout    = EMMC_BOOT_SLEEP;
    // HPI Flag
    emmcdev->config.HPI_On          = EMMC_OFF;
    //--------------------------------------------
    
    //--------------------------------------------
    // [Step 2] Set-up Sleep Time Control : DeviceID Mode(CMD0/1/2/3) & DataTransfer Mode(CMD9/7/8/6)
    emmcdev->config.BootModeHostSleep   = EMMC_BOOT_SLEEP;         // CMD 0/1
    emmcdev->config.InterruptSleep      = EMMC_INIT_CMDS_SLEEP;    // CMD 0/1/2/3/9/7/8 // 391KHz * 512 = 1310[us]
    //--------------------------------------------

    //--------------------------------------------
    // [Step 3] In(Input)/Out(Write) Delay Control
#if EMMC_IO_PAD_4_HS	
    emmcdev->config.HostInDly_On        = BCM_EMMC_IN_DLY_ON;    //emmc_cfg[4];   // ENABLE, DISABLE
    emmcdev->config.HostInDly_Ctrl      = BCM_EMMC_IN_DLY_CTRL;  //emmc_cfg[5];   // 0 ~ 3(default)  : 
    emmcdev->config.HostInDly_Dly       = BCM_EMMC_IN_DLY_DLY;   //emmc_cfg[6];   // 0(default) ~ 63 : 
    emmcdev->config.HostOutDly_On       = BCM_EMMC_OUT_DLY_ON;   //emmc_cfg[7];   // ENABLE, DISABLE
    emmcdev->config.HostOutDly_Ctrl     = BCM_EMMC_OUT_DLY_CTRL; //emmc_cfg[8];   // 0 ~ 3(default)  : 
    emmcdev->config.HostOutDly_Dly      = BCM_EMMC_OUT_DLY_DLY;  //emmc_cfg[9];   // 0(default) ~ 15 :
#endif
    //--------------------------------------------
    
#if DEBUG_EMMC_INIT //DEBUG_EMMC_CONFIG
    printf("----------------------------------------------\n");
    printf(" <<< emmc_config_init >>>\n");
    printf("----------------------------------------------\n");
    printf(" [Step 1] EMMC flash data initialization\n");
    printf("  emmcdev->RCA                    = 0x%08X\n", emmcdev->RCA);
    //printf("  emmcdev->config.OCR_SectorMode  = %d (0=Off(<=2GB),1=On(>2GB))\n", emmcdev->config.OCR_SectorMode);
    // Bus Mode : Speed, Width, Voltage
    printf("  emmcdev->config.HSTiming        = %d (0=Full,1=HS,2=HS200)\n", emmcdev->config.HSTiming);
    printf("  emmcdev->config.HostHS_On       = %d (0=Off,1=On)\n", emmcdev->config.HostHS_On);
    printf("  emmcdev->config.BusFreq         = %d (1=50M,2=25M,3=13M,4=6M,5=3M)\n", emmcdev->config.BusFreq);
    printf("  emmcdev->config.BusWidth        = %d (0=1bit,1=4bit,2=8bit)\n", emmcdev->config.BusWidth);
    printf("  emmcdev->config.BusVoltage      = %d (5=1.8V,6=3.0V,7=3.3V)\n", emmcdev->config.BusVoltage);
    // Block Size & Control
    //printf("  emmcdev->config.HighCap_On      = %d (0=Off(<=2GB),1=On(>2GB))\n", emmcdev->config.HighCap_On);
    printf("  emmcdev->config.ReadTimeout     = %d (init value)\n", emmcdev->config.ReadTimeout);
    printf("  emmcdev->config.WriteTimeout    = %d (init value)\n", emmcdev->config.WriteTimeout);
    // HPI Flag
    printf("  emmcdev->config.HPI_On          = %d\n", emmcdev->config.HPI_On);
    //--------------------------------------------
    
    //--------------------------------------------
    // [Step 2] Set-up Sleep Time Control : DeviceID Mode(CMD0/1/2/3) & DataTransfer Mode(CMD9/7/8/6)
    printf("----------------------------------------------\n");
    printf(" [Step 2] Set-up Sleep Time Control : DeviceID Mode & DataTransfer Mode\n");
    printf("  emmcdev->config.BootModeHostSleep   = %d[ms]\n", emmcdev->config.BootModeHostSleep);  //EMMC_BOOT_SLEEP;
    printf("  emmcdev->config.InterruptSleep      = %d[us]\n", emmcdev->config.InterruptSleep);  //EMMC_INIT_CMDS_SLEEP;
    printf("  emmcdev->config.ReadBufSleep        = %d[us]\n", emmcdev->config.ReadBufSleep);    //0;
    printf("  emmcdev->config.WriteBufSleep       = %d[us]\n", emmcdev->config.WriteBufSleep);   //0;
    //--------------------------------------------
    
    //--------------------------------------------
    // [Step 3] In(Input)/Out(Write) Delay Control
    printf("----------------------------------------------\n");
    printf(" [Step 3] In(Input)/Out(Write) Delay Control\n");
    printf("  emmcdev->config.HostInDly_On        = %d\n", emmcdev->config.HostInDly_On);      //BCM_EMMC_IN_DLY_ON;    // ENABLE, DISABLE
    printf("  emmcdev->config.HostInDly_Ctrl      = %d\n", emmcdev->config.HostInDly_Ctrl);    //BCM_EMMC_IN_DLY_CTRL;  // 0 ~ 3(default)  : 
    printf("  emmcdev->config.HostInDly_Dly       = %d\n", emmcdev->config.HostInDly_Dly);     //BCM_EMMC_IN_DLY_DLY;   // 0(default) ~ 63 : 
    printf("  emmcdev->config.HostOutDly_On       = %d\n", emmcdev->config.HostOutDly_On);     //BCM_EMMC_OUT_DLY_ON;   // ENABLE, DISABLE
    printf("  emmcdev->config.HostOutDly_Ctrl     = %d\n", emmcdev->config.HostOutDly_Ctrl);   //BCM_EMMC_OUT_DLY_CTRL; // 0 ~ 3(default)  : 
    printf("  emmcdev->config.HostOutDly_Dly      = %d\n", emmcdev->config.HostOutDly_Dly);    //BCM_EMMC_OUT_DLY_DLY;  // 0(default) ~ 15 :
    printf("----------------------------------------------\n");
#endif
}


void emmc_Host_Config( emmcdev_t *emmcdev )
{
#if EMMC_SCB_SEQ_EN
    uint32_t scb_seq_en;
#endif
    
#if DEBUG_EMMC_INIT
    printf("----------------------------------------------\n");
    printf(" <<< emmc_Host_Config >>>\n");
    printf("----------------------------------------------\n");
#endif
    
    //--------------------------------------------
    // [Step 1] Disable Boot Logic in chip : We can apply any case secondary_emmc and emmc_boot.
    // Boot_from_eMMC : FSBL set to 1, SSBL need to set 0
#if( EMMC_BOOT_SEL == EMMC_ON )
    if( emmcdev->config.CFEBootMode == BOOT_FROM_EMMC )
    {
        uint32_t boot_status;
#if( BCHP_CHIP == 7563 )
        EMMC_BOOT->emmc_boot_main_ctl = 0x00;
        do
        {
            boot_status = EMMC_BOOT->emmc_boot_status;
            cfe_usleep( 1 );
        } while( (boot_status & 0x00000001) == 1 );
#else
        EMMC_BOOT->emmc_boot_main_ctl = 0x00;
        do
        {
            boot_status = EMMC_BOOT->emmc_boot_status;
            cfe_usleep( 1 );
        } while( (boot_status & 0x00000001) == 1 );
#endif
    }
#endif  
    //--------------------------------------------
    
#if EMMC_SCB_SEQ_EN
    //--------------------------------------------
    // [Step 2] Disable SCB_SEQ_EN
    scb_seq_en = EMMC_TOP_CFG->emmc_top_cfg_sdio_emmc_ctrl1;
    EMMC_TOP_CFG->emmc_top_cfg_sdio_emmc_ctrl1 = scb_seq_en & 0xFFFFFBFF;

    cfe_usleep( 1 );
    //--------------------------------------------
#endif
    
    //--------------------------------------------
    // [Step 3] Control pin_mux
    emmc_Ctrl_PinMux( );
    cfe_usleep( 1 );
    //--------------------------------------------
}


void emmc_Ctrl_PinMux( void )
{
}


// Boot Mode
void emmc_Boot_Mode( emmcdev_t *emmcdev, uint32_t eMMC_Boot_Clk )
{
    uint32_t reg_ctrl_set0, reg_ctrl_set1;
    
#if DEBUG_EMMC_INIT
    printf("----------------------------------------------\n");
    printf(" <<< emmc_Boot_Mode >>>\n");
    printf("----------------------------------------------\n");
#endif
    
    //--------------------------------------------
    // [Step 1] Reset Host
    EMMC_HOSTIF->emmc_host_ctrl_set1 = 0x00000000;  //Host reset
    EMMC_HOSTIF->emmc_host_ctrl_set1 = 0x07000000;  //Host reset
    //EMMC_HOSTIF->emmc_host_ctrl_set1 = 0x070F0000;
    //EMMC_HOSTIF->emmc_host_ctrl_set0 |= EMMC_HOSTIF_CTRL_SET0_HW_RESET_MASK;
    
    do
    {
        cfe_usleep(1000 * emmcdev->config.BootModeHostSleep );
        reg_ctrl_set1 = EMMC_HOSTIF->emmc_host_ctrl_set1 & 0x07000000;
        // 0x07000000 = SOFT_RESET_DAT = 1, SOFT_RESET_CMD = 1, SOFT_RESET_CORE = 1 => Reset

#if DEBUG_EMMC_INIT
        printf("  SOFT_RESET_DAT  = %d\n", (reg_ctrl_set1 & EMMC_HOSTIF_CTRL_SET1_SOFT_RESET_DAT_MASK));
        printf("  SOFT_RESET_CMD  = %d\n", (reg_ctrl_set1 & EMMC_HOSTIF_CTRL_SET1_SOFT_RESET_CMD_MASK));
        printf("  SOFT_RESET_CORE = %d\n", (reg_ctrl_set1 & EMMC_HOSTIF_CTRL_SET1_SOFT_RESET_CORE_MASK));
#endif
    } while( reg_ctrl_set1 != 0 );
    //--------------------------------------------
    
    
    //--------------------------------------------
    // [Step 2] Control set
    // [Step 2-1] HOST_CTRL_SET0
    //Use 1-bit mode, sd_bus_voltage = 3.3v, normal-speed
    EMMC_HOSTIF->emmc_host_ctrl_set0 = 0x00000100 + (emmcdev->config.BusVoltage<<EMMC_HOSTIF_CTRL_SET0_SD_BUS_VOLTAGE_SELECT_SHIFT);
    // SD_BUS_VOLTAGE_SELECT[11:09] = EMMC_BUS_VOLTAGE (7=3.3volts, 6=3.0volt, 5=1.8volt)
    // SD_BUS_POWER[08]      = 1(On) 
    // HIGH_SPEED_ENABLE[02] = 0(Normal)
    //            SD_8BIT_MODE[05] SD_4BIT_MODE[01]
    // 1-bit mode               0                0
    // 4-bit mode               0                1
    // 8-bit mode               1                x
    
    // [Step 2-2] HOST_CTRL_SET1
    reg_ctrl_set1 = EMMC_HOSTIF->emmc_host_ctrl_set1;
    SET_REG_FIELD(reg_ctrl_set1, EMMC_HOSTIF_CTRL_SET1_TIMEOUT_COUNT, 0x0E);
    // 0 = 2^13, ..., 14(0x0E) = 2^27(TMCLK * 2^27)
    //supply internal clock, which is base clock divided by 2
    //SDIO_1_HOST.CTRL_SET1 = &h00000101& //getting lot of error intr
    //SDIO_1_HOST.CTRL_SET1 = &h00008001& //sbadada ////No data comming with this setting on palladium
    //SDIO_1_HOST.CTRL_SET1 = &h00000001& //sbadada  ////getting lot of error intr
    //SET_REG_FIELD(reg_ctrl_set1, EMMC_HOSTIF_CTRL_SET1_FREQ_CTRL, 0xA0);
    SET_REG_FIELD(reg_ctrl_set1, EMMC_HOSTIF_CTRL_SET1_FREQ_CTRL, eMMC_Boot_Clk);
    // 0xFF = Base clock div 510. (100MHz / 510 = 195KHz) for CID freq(fOD:0~400KHz)
    // 0xF0 = Base clock div 480. (100MHz / 480 = 208KHz)
    // 0xE0 = Base clock div 448. (100MHz / 448 = 223KHz)
    // 0xA0 = Base clock div 320. (100MHz / 320 = 313KHz)
    // 0x90 = Base clock div 288. (100MHz / 288 = 347KHz)
    // 0x80 = Base clock div 256. (100MHz / 256 = 391KHz)
    // 0x40 = Base clock div 128. (100MHz / 128 = 781KHz)
    // 0x01 = Base clock div   2. (100MHz /   2 =  50MHz)
    // Hynix OK at 0x90, 0x80, Toshiba OK at 0xA0
    //printf("  FREQ_CTRL = %d\n", 0x80);
    SET_REG_FIELD(reg_ctrl_set1, EMMC_HOSTIF_CTRL_SET1_INTERNAL_CLK_ENA, 0x01);
    EMMC_HOSTIF->emmc_host_ctrl_set1 = reg_ctrl_set1;

    // [Step 2-3] HOST_CTRL_SET0 : SD_BUS_POWER Off
#if 1   // eMMC 
    reg_ctrl_set0 = EMMC_HOSTIF->emmc_host_ctrl_set0;
    SET_REG_FIELD(reg_ctrl_set0, EMMC_HOSTIF_CTRL_SET0_SD_BUS_POWER, 0x00);
    EMMC_HOSTIF->emmc_host_ctrl_set0 = reg_ctrl_set0;
#else   // SD_Card
    reg_ctrl_set0 = EMMC_HOSTIF->emmc_host_ctrl_set0;
    SET_REG_FIELD(reg_ctrl_set0, EMMC_HOSTIF_CTRL_SET0_SD_BUS_POWER, 0x01);
    EMMC_HOSTIF->emmc_host_ctrl_set0 = reg_ctrl_set0;
#endif
    //--------------------------------------------
    
    
    //--------------------------------------------
    // [Step 3] Wait for internal clock stable
    do
    {
        cfe_usleep(1000 * emmcdev->config.BootModeHostSleep );     // cfe_usleep Doesn't work.
        reg_ctrl_set1 = EMMC_HOSTIF->emmc_host_ctrl_set1;
        GET_REG_FIELD(reg_ctrl_set1, EMMC_HOSTIF_CTRL_SET1_INTERNAL_CLK_STABLE);
    }while( reg_ctrl_set1 == EMMC_NG );
    //--------------------------------------------

    
    //--------------------------------------------
    // [Step 4] Enable SD_Clk & Host Interrupt
    //Enable SD CLK
    reg_ctrl_set1 = EMMC_HOSTIF->emmc_host_ctrl_set1;
    SET_REG_FIELD(reg_ctrl_set1, EMMC_HOSTIF_CTRL_SET1_SD_CLK_ENA, 0x01);
    EMMC_HOSTIF->emmc_host_ctrl_set1 = reg_ctrl_set1;
    // Enable SD_BUS_POWER (CLK, CMD, DAT0~3)
    reg_ctrl_set0 = EMMC_HOSTIF->emmc_host_ctrl_set0;
    SET_REG_FIELD(reg_ctrl_set0, EMMC_HOSTIF_CTRL_SET0_SD_BUS_POWER, 0x01);
    EMMC_HOSTIF->emmc_host_ctrl_set0 = reg_ctrl_set0;
    
    //Enable interrupts
    // E.g. SDIO_1_HOST.INT_STATUS_ENA = &h33FF07FF&
    //      SDIO_1_HOST.INT_STATUS_ENA = &h33FF063F&
#if( ARASAN_IP_VER < ARASAN_IP_V10_7A )     // ARASAN_IP_V9_98
    emmc_Enable_Host_Int( 0x13FF003F );
    //< INT_SIGNAL_ENA = 0x13FF003F >
    // CEATA_ERR_INT_SIG_ENA        [29]  0
    // TARGET_RESP_ERR_INT_SIG_ENA  [28]  1
    // TUNE_ERR_SIG_EN              [26]  0
    // ADMA_ERR_INT_SIG_ENA         [25]  1
    // AUTO_CMD12_ERR_INT_SIG_ENA   [24]  1
    // CURRENT_LIMIT_ERR_INT_SIG_ENA[23]  1
    // DATA_END_BIT_ERR_INT_SIG_ENA [22]  1
    // DATA_CRC_ERR_INT_SIG_ENA     [21]  1
    // DATA_TIMEOUT_ERR_INT_SIG_ENA [20]  1
    // CMD_INDEX_ERR_INT_SIG_ENA    [19]  1
    // CMD_END_BIT_ERR_INT_SIG_ENA  [18]  1
    // CMD_CRC_ERR_INT_SIG_ENA      [17]  1
    // CMD_TIMEOUT_ERR_INT_SIG_ENA  [16]  1
    //                              [14]  0
    //                              [13]  0
    //                              [12]  0
    //                              [11]  0
    // BOOT_TERM_INT_SIG_ENA        [10]  0
    // BOOT_ACK_RCV_INT_SIG_ENA     [09]  0
    // CARD_INT_SIG_ENA             [08]  0
    // CAR_REMOVAL_INT_SIG_ENA      [07]  0
    // CAR_INSERT_INT_SIG_ENA       [06]  0
    // BUFFER_READ_INT_SIG_ENA      [05]  1
    // BUFFER_WRITE_INT_SIG_ENA     [04]  1
    // DMA_INT_SIG_ENA              [03]  1
    // BLOCK_GAP_INT_SIG_ENA        [02]  1
    // TRANSFER_COMPLETE_INT_SIG_ENA[01]  1
    // COMMAND_COMPLETE_INT_SIG_ENA [00]  1
#else
    emmc_Enable_Host_Int( 0x13FF063F );
    //< INT_SIGNAL_ENA = 0x13FF063F >
    // TARGET_RESP_ERR_INT_SIG_ENA  [28]  1
    // TUNE_ERR_SIG_EN              [26]  0
    // ADMA_ERR_INT_SIG_ENA         [25]  1
    // AUTO_CMD12_ERR_INT_SIG_ENA   [24]  1
    // CURRENT_LIMIT_ERR_INT_SIG_ENA[23]  1
    // DATA_END_BIT_ERR_INT_SIG_ENA [22]  1
    // DATA_CRC_ERR_INT_SIG_ENA     [21]  1
    // DATA_TIMEOUT_ERR_INT_SIG_ENA [20]  1
    // CMD_INDEX_ERR_INT_SIG_ENA    [19]  1
    // CMD_END_BIT_ERR_INT_SIG_ENA  [18]  1
    // CMD_CRC_ERR_INT_SIG_ENA      [17]  1
    // CMD_TIMEOUT_ERR_INT_SIG_ENA  [16]  1
    // BOOT_TERM_INT_SIG_ENA        [14]  0
    // BOOT_ACK_RCV_INT_SIG_ENA     [13]  0
    // RETUNE_EVENT_EN              [12]  0
    // INT_C_EN                     [11]  0
    // INT_B_EN                     [10]  1
    // INT_A_EN                     [09]  1
    // CARD_INT_SIG_ENA             [08]  0
    // CAR_REMOVAL_INT_SIG_ENA      [07]  0
    // CAR_INSERT_INT_SIG_ENA       [06]  0
    // BUFFER_READ_INT_SIG_ENA      [05]  1
    // BUFFER_WRITE_INT_SIG_ENA     [04]  1
    // DMA_INT_SIG_ENA              [03]  1
    // BLOCK_GAP_INT_SIG_ENA        [02]  1
    // TRANSFER_COMPLETE_INT_SIG_ENA[01]  1
    // COMMAND_COMPLETE_INT_SIG_ENA [00]  1
#endif
    //--------------------------------------------
}


// Device Identification Mode
static int emmc_DeviceID_Mode_SingleDevice( emmcdev_t *emmcdev )
{
    uint32_t OCR_data, OCR_timeout=0;
    uint8_t i, eMMC_mode_detect=EMMC_NG;
    uint32_t CMD2_Status;
    
#if DEBUG_EMMC_INIT
    printf("----------------------------------------------\n");
    printf(" <<< emmc_DeviceID_Mode_SingleDevice >>>\n");
    printf("----------------------------------------------\n");
#endif
    
    //--------------------------------------------
    // [Step 1] CMD 0
    // Device Reset
    emmc_CMD0_C0ba_bc_GO_IDLE_STATE( emmcdev );
    //emmc_CMD0_C0ba_bc_BOOT_INITIATION( emmcdev );
    //emmc_CMD0_C0ba_bc_GO_PRE_IDLE_STATE( emmcdev );
#if DEBUG_EMMC_INIT
    printf(" ---> CMD0 : Device Reset\n");
#endif
    //--------------------------------------------
    
    //--------------------------------------------
    // [Step 2] CMD 1
    //-------------------------------------
    // Access mode validation & Waiting Power-up/Reset Procedure
    // eMMC doesn//t change into //Inactive state// during voltage range validation stage
    //---------------------------------------------
    do
    {
        OCR_data = emmc_CMD1_C0ba_bcrR3_SEND_OP_COND( 0x00000000, emmcdev );
        //printf(" ---> CMD1_1 : OCR_data = 0x%08X\n", OCR_data);
        cfe_usleep( emmcdev->config.InterruptSleep );
        OCR_timeout++;
        if( OCR_timeout > 5 )
        {
            return EMMC_NG;
        }
    } while ((OCR_data & 0x00FF8080) != 0x00FF8080);
#if DEBUG_EMMC_INIT
    printf(" ---> Access mode validation\n");
#endif
    
    // eMMC access mode detection using OCR_data
    for( i=0; i<EMMC_CMD1_TIMEOUT_CNT; i++ )
    {
        OCR_data = emmc_CMD1_C0ba_bcrR3_SEND_OP_COND( 0x40FF8080, emmcdev );
        //cfe_usleep( emmcdev->config.InterruptSleep );
        //printf(" ---> CMD1_2[0x40FF8080] : OCR_data = 0x%08X\n", OCR_data);
        cfe_usleep(1000 * 10 );
        if( OCR_data == 0xC0FF8080 )
        {
#if DEBUG_EMMC_INIT
            printf(" ---> device is Sector mode eMMC (%dth scan)\n", i);
#endif           
            eMMC_mode_detect = EMMC_OK;
            emmcdev->config.OCR_SectorMode = EMMC_ON;
            //emmcdev->config.HighCap_On = EMMC_ON;
            break;
        }    
        else if( OCR_data == 0x80FF8080 )
        {
#if DEBUG_EMMC_INIT
            printf(" ---> device is Byte mode eMMC (%dth scan)\n", i);
#endif
            
            eMMC_mode_detect = EMMC_OK;
            emmcdev->config.OCR_SectorMode = EMMC_OFF;
            //emmcdev->config.HighCap_On = EMMC_OFF;
            break;
        }
    }
    //printf(" ---> Sector mode eMMC (%dth scan), sleep %d\n", i, emmcdev->config.InterruptSleep);
    
    if( eMMC_mode_detect != EMMC_OK )
    {
        for( i=0; i<EMMC_CMD1_TIMEOUT_CNT; i++ )
        {
            OCR_data = emmc_CMD1_C0ba_bcrR3_SEND_OP_COND( 0x00FF8080, emmcdev );
           //cfe_usleep( emmcdev->config.InterruptSleep );
            //printf(" ---> CMD1_3[0x00FF8080] : OCR_data = 0x%08X\n", OCR_data);
            cfe_usleep(1000 * 10 );
            if( OCR_data == 0x80FF8080 )
            {
                printf(" ---> device is Byte mode eMMC (%dth scan)\n", i);
                eMMC_mode_detect = EMMC_OK;
                emmcdev->config.OCR_SectorMode = EMMC_OFF;
                //emmcdev->config.HighCap_On = EMMC_OFF;
                break;
            }    
        }
    }
    
    if( eMMC_mode_detect == EMMC_OK )
    {
        //printf(" ---> Waiting Power-up/Reset Procedure\n");
        //-------------------------------------
        
        //--------------------------------------------
        // [Step 3] CMD 2 & 3
        // Get CID
        CMD2_Status = emmc_CMD2_C0ba_bcrR2_ALL_SEND_CID_SingleDevice( emmcdev );
#if DEBUG_EMMC_INIT
        printf(" ---> CMD2 : Get CID\n");
#endif
        
        // Set RCA
        emmcdev->CST = emmc_CMD3_C0ba_acR1_SET_RCA( emmcdev->RCA, emmcdev );
        if( emmcdev->CST != EMMC_STATUS_ERROR )
        {
        if( ((emmcdev->CST & CST_CURRENT_STATE_MASK)>>CST_CURRENT_STATE_SHIFT) != CST_STATE_STBY )
        {
            emmcdev->CST = emmc_Wait_EmmcNextState( emmcdev, CST_STATE_STBY, emmcdev->config.InterruptSleep, 3 );
        }
#if DEBUG_EMMC_INIT
        printf(" ---> CMD3 : Set RCA ( resp(Status): 0x%08X )\n", emmcdev->CST);
#endif
        }
        //--------------------------------------------
        
        if( (CMD2_Status==EMMC_STATUS_ERROR) || (emmcdev->CST==EMMC_STATUS_ERROR) || (emmcdev->CST==EMMC_NG) )
        {
#if DEBUG_EMMC_INIT
            printf(" ---> CMD2_Status : 0x%X, CMD3_Status = 0x%X\n", CMD2_Status, emmcdev->CST);
#endif
            printf(" !!! Warning : eMMC wasn't detected and rescan with lower clock !!!\n");
            printf(" !!! Recommend : Change EMMC_BOOT_CLK to lower value to avoid rescan !!!\n");
            return EMMC_NG;
        }
        else
        {
        return EMMC_OK;
    }
    }
    else
    {
        printf("\n\n !!! eMMC access mode wasn't detected. Please check device and board!!! \n");
        return EMMC_NG;
    }
    
}


// Data Transfer Mode
void emmc_Setup_DataTransfer_Mode( emmcdev_t *emmcdev )
{
    uint32_t reg_ctrl_set0=0, HcSizeTemp;
        
#if DEBUG_EMMC_INIT
    printf("----------------------------------------------\n");
    printf(" <<< emmc_Setup_DataTransfer_Mode : After boot mode >>>\n");
    printf("----------------------------------------------\n");
#endif
    
    //--------------------------------------------
    // [Step 1] CSD Retrieval and Host Adjustment
    emmcdev->CST = emmc_CMD9_C0ba_acR2_SEND_CSD( emmcdev );
    if( ((emmcdev->CST & CST_CURRENT_STATE_MASK)>>CST_CURRENT_STATE_SHIFT) != CST_STATE_STBY )
    {
        emmcdev->CST = emmc_Wait_EmmcNextState( emmcdev, CST_STATE_STBY, emmcdev->config.InterruptSleep, 9 );
    }
#if DEBUG_EMMC_INIT
    printf(" ---> CMD9 : emmcdev->CST = 0x%08X !!!\n", emmcdev->CST);
#endif
    
    // DSR Setting
#if 0    
    if( emmcdev->CSD.DSR_IMP == EMMC_ON )
    {
        // Need to find device having feature to make function with algorithm
        //emmcdev->CST = emmc_SetDSR( emmcdev );
    }
#endif  
    //--------------------------------------------
    
    
    //--------------------------------------------
    // [Step 2] Change state from stand-by to transfer
    emmcdev->CST = emmc_CMD7_C0ba_acR1_SELECT_CARD_stby_tans( emmcdev->RCA, emmcdev );
    if( ((emmcdev->CST & CST_CURRENT_STATE_MASK)>>CST_CURRENT_STATE_SHIFT) != CST_STATE_TRAN )
    {
        emmcdev->CST = emmc_Wait_EmmcNextState( emmcdev, CST_STATE_TRAN, emmcdev->config.InterruptSleep, 7 );
    }
#if DEBUG_EMMC_INIT
    printf(" ---> CMD7 : emmcdev->CST = 0x%08X !!!\n", emmcdev->CST);
#endif
    //--------------------------------------------
    
    
    //--------------------------------------------
    // [Step 3] Extended_CSD Retrieval and Host Adjustment
    if( emmcdev->CSD.SPEC_VERS > 3 )    // From eMMC v4.1
    {
        emmcdev->CST = emmc_CMD8_C0ba_adtcR1_SEND_EXT_CSD( emmcdev );
#if DEBUG_EMMC_INIT
        printf(" ---> First CMD8 Execute : emmcdev->CST = 0x%08X !!! ---\n", emmcdev->CST);
#endif
    }
    //--------------------------------------------
    
    //--------------------------------------------
    // [Step 4] Calculate configuration parameters & Partition/Size Information
    // Generic CMD6 Timeout
    if( emmcdev->ExtCSD.GENERIC_CMD6_TIME == 0 )
    {
        emmcdev->config.GenCmd6Timeout = emmcdev->ExtCSD.PARTITION_SWITCH_TIME * 10;    // [ms]
    }
    else    // JEDEC v4.5
    {
        emmcdev->config.GenCmd6Timeout = emmcdev->ExtCSD.GENERIC_CMD6_TIME * 10;    // [ms]
    }
    
    // Partition Config
    emmcdev->config.BootPartitionEnable = (emmcdev->ExtCSD.PARTITION_CONFIG & 0x38) >> 3;   // No Boot, BOOT1, BOOT2, UserArea
    emmcdev->config.PartitionAccess     = emmcdev->ExtCSD.PARTITION_CONFIG & 0x07;
    emmcdev->config.PartitionSwitchTime = emmcdev->ExtCSD.PARTITION_SWITCH_TIME * 10;   // [msec]

    // Check New Partition and setup HC
    emmcdev->config.PartitionCompleted  = emmcdev->ExtCSD.PARTITION_SETTING_COMPLETED;
    emmcdev->config.EraseGroupDef       = emmcdev->ExtCSD.ERASE_GROUP_DEF;
    if( emmcdev->config.PartitionCompleted == EMMC_ON )
    {
        emmcdev->config.EraseGroupDef    = EMMC_ON;
        emmcdev->CST = emmc_CMD6_C0ba_acR1b_SWITCH_ExtCSD( Idx_ExtCSD_ACC_WRB, Idx_ExtCSD_ERASE_GROUP_DEF, EMMC_ON, emmcdev );
    }
    
    // Calculate Read & Write Block Size : Super Page Size
    // - As a guide for format clusters
    // - To prevent format-page misalignment
    // - As a guide for minimum data-transfer size
    emmcdev->config.AccessSize  = 512 << (emmcdev->ExtCSD.ACC_SIZE-1);      // [Byte]
    emmcdev->config.CacheSize   = emmcdev->ExtCSD.CACHE_SIZE << 7;          // [Byte], Reg. x 1kb
    
    // Calculate Read & Write Block Size and Bit
    if( emmcdev->CSD.READ_BL_LEN != emmcdev->CSD.WRITE_BL_LEN )
    {
        printf("\n !!! READ_BL_LEN is different from WRITE_BL_LEN. eMMC looks 2GB. Check eMMC datasheet !!!\n");
    }

    if( emmcdev->config.OCR_SectorMode == EMMC_ON )
    {
        // Sector mode devices can have different read/write block lengths
        // basedon their native sector sizes. However according to the 
        // standard, by default all devices must have 512B read/write
        // block lenght. Even devices with 4kb sector sizes operate with
        // emulated 512B sectors by default
        emmcdev->config.ReadBlkLenBit4Addr  = emmcdev->CSD.READ_BL_LEN;
        emmcdev->config.WriteBlkLenBit4Addr = emmcdev->CSD.WRITE_BL_LEN;
        emmcdev->config.ReadBlkLen  = (1 << emmcdev->CSD.READ_BL_LEN);
        emmcdev->config.WriteBlkLen = (1 << emmcdev->CSD.WRITE_BL_LEN);
    }
    else        // Byte mode
    {
        emmcdev->config.ReadBlkLenBit4Addr  = 9;    // Block size = 512
        emmcdev->config.WriteBlkLenBit4Addr = 9;    // Block size = 512
        emmcdev->config.ReadBlkLen  = 512;          // Standard Mandatory
        emmcdev->config.WriteBlkLen = 512;          // Standard Mandatory
    }
    
    // Calculate Boot1/Boot2/RPMB partition size
    emmcdev->config.Boot1Size = emmcdev->ExtCSD.BOOT_SIZE_MULT * 128 * 1024;
    emmcdev->config.Boot2Size = emmcdev->config.Boot1Size;
    emmcdev->config.RPMBSize  = emmcdev->ExtCSD.RPMB_SIZE_MULT * 128 * 1024;
    
    // Calculate EraseUnitSize, EraseTimeout, WriteProtectionGroupSize : CSD_WRITE_BL_LEN[512B]
    emmcdev->config.EraseUnitSize   = (emmcdev->CSD.ERASE_GRP_SIZE + 1) * (emmcdev->CSD.ERASE_GRP_MULT + 1); // * CSD_WRITE_BL_LEN //=[512B]
    emmcdev->config.WpGrpSize       = (emmcdev->CSD.WP_GRP_SIZE + 1) * emmcdev->config.EraseUnitSize;
    emmcdev->config.EraseTimeout    = emmc_GetCSDEraseTimeout( emmcdev );   // Including ReadTimeout, WriteTimeout
    
    // Calculate Data partition & Enhanced Data Address
    if( emmcdev->config.OCR_SectorMode == EMMC_ON )
    //if( emmcdev->CSD.C_SIZE == 0xFFF )
    {
        emmcdev->config.DataSize    = (uint64_t)emmcdev->ExtCSD.SEC_COUNT << emmcdev->config.ReadBlkLenBit4Addr;        // Sector count
        emmcdev->config.DataEnhAddr = (uint64_t)emmcdev->ExtCSD.ENH_START_ADDR * emmcdev->config.ReadBlkLen;    // [Byte]
    }
    else    // Byte Mode
    {
        emmcdev->config.DataSize = (uint64_t)((1<<emmcdev->CSD.READ_BL_LEN) * (emmcdev->CSD.C_SIZE + 1) * 1<<(emmcdev->CSD.C_SIZE_MULT+2));
        emmcdev->config.DataEnhAddr = (uint64_t)emmcdev->ExtCSD.ENH_START_ADDR; // [Byte]
    }
    
    // Calculate HighCapacity EraseUnitSize, EraseTimeout, WriteProtectionGroupSize
    emmcdev->config.HcEraseUnitSize = (uint32_t)emmcdev->ExtCSD.HC_ERASE_GRP_SIZE << 19;    // [Byte]
    emmcdev->config.HcWpGrpSize     = (uint32_t)emmcdev->ExtCSD.HC_WP_GRP_SIZE * emmcdev->config.HcEraseUnitSize;   // [Byte]
    emmcdev->config.HcEraseTimeout  = (uint32_t)emmcdev->ExtCSD.ERASE_TIMEOUT_MULT * 300;   // [ms]
    emmcdev->config.HcReadTimeout   = (uint32_t)emmcdev->config.ReadTimeout;        // [ms], Need to investigate.
    emmcdev->config.HcWriteTimeout  = (uint32_t)emmcdev->config.HcEraseTimeout; // [ms]
    
    // Calculate Partition Size in DataArea
    HcSizeTemp = emmcdev->ExtCSD.HC_ERASE_GRP_SIZE * emmcdev->ExtCSD.HC_WP_GRP_SIZE;
    emmcdev->config.MaxEnhSize  = (uint64_t)(emmcdev->ExtCSD.MAX_ENH_SIZE_MULT * HcSizeTemp) << 19;
    emmcdev->config.GP1Size     = (uint64_t)(emmcdev->ExtCSD.GP_SIZE_MULT[0] * HcSizeTemp) << 19;   // * emmcdev->config.WpGrpSize;
    emmcdev->config.GP2Size     = (uint64_t)(emmcdev->ExtCSD.GP_SIZE_MULT[1] * HcSizeTemp) << 19;
    emmcdev->config.GP3Size     = (uint64_t)(emmcdev->ExtCSD.GP_SIZE_MULT[2] * HcSizeTemp) << 19;
    emmcdev->config.GP4Size     = (uint64_t)(emmcdev->ExtCSD.GP_SIZE_MULT[3] * HcSizeTemp) << 19;
    emmcdev->config.DataEnhSize = (uint64_t)(emmcdev->ExtCSD.ENH_SIZE_MULT * HcSizeTemp) << 19;
    
#if DEBUG_EMMC_INIT
    emmc_Print_EmmcSizePartitionInfo( emmcdev );
#endif
    //--------------------------------------------
    
    
    //--------------------------------------------
    // [Step 5] Bus Mode Configuration
    emmc_Setup_BusFreqWidth( emmcdev );
#if DEBUG_EMMC_INIT
    printf(" emmc_Setup_BusFreqWidth : new bus specs are loaded !!!\n\n");
#endif
    //--------------------------------------------
    
    //--------------------------------------------
    // [Step 6] SET_BLOCKLEN to set the block length as 512/4K Bytes.
    emmcdev->CST = emmc_CMD16_C2br_acR1_SET_BLOCKLEN( emmcdev->config.ReadBlkLen, emmcdev );
    //printf(" SET_BLOCKLEN emmcdev->CST : 0x%08X\n", emmcdev->CST);
    
    emmc_Clear_HostIntStatus( SDIO_HOST_INT_CLR_ALL );
    
    // Host Bus Power On
    reg_ctrl_set0 = EMMC_HOSTIF->emmc_host_ctrl_set0;
    reg_ctrl_set0 |= EMMC_HOSTIF_CTRL_SET0_SD_BUS_POWER_MASK;
    EMMC_HOSTIF->emmc_host_ctrl_set0 = reg_ctrl_set0;
    
    // Update ExtCSD
    //printf(" emmc_CMD8_C0ba_adtcR1_SEND_EXT_CSD : Update ExtCSD!!!\n");
    emmcdev->CST = emmc_CMD8_C0ba_adtcR1_SEND_EXT_CSD( emmcdev );
    //--------------------------------------------
#if DEBUG_EMMC_INIT
    printf(" ---> Important ExtCSD!!! ---\n");
    printf( "ExtCSD.HS_TIMING                   [185] = 0x%02X (%d)\n", emmcdev->ExtCSD.HS_TIMING           , emmcdev->ExtCSD.HS_TIMING            );
    printf( "ExtCSD.BUS_WIDTH                   [183] = 0x%02X (%d)\n", emmcdev->ExtCSD.BUS_WIDTH           , emmcdev->ExtCSD.BUS_WIDTH            );
    printf( "ExtCSD.ERASED_MEM_CONT             [181] = 0x%02X (%d)\n", emmcdev->ExtCSD.ERASED_MEM_CONT     , emmcdev->ExtCSD.ERASED_MEM_CONT      );
    printf( "ExtCSD.PARTITION_CONFIG            [179] = 0x%02X (%d)\n", emmcdev->ExtCSD.PARTITION_CONFIG    , emmcdev->ExtCSD.PARTITION_CONFIG     );
    printf( "ExtCSD.BOOT_CONFIG_PROT            [178] = 0x%02X (%d)\n", emmcdev->ExtCSD.BOOT_CONFIG_PROT    , emmcdev->ExtCSD.BOOT_CONFIG_PROT     );
    printf( "ExtCSD.BOOT_BUS_CONDITIONS         [177] = 0x%02X (%d)\n", emmcdev->ExtCSD.BOOT_BUS_CONDITIONS , emmcdev->ExtCSD.BOOT_BUS_CONDITIONS  );
#endif
    
    //--------------------------------------------
    // Check the current device status
#if DEBUG_EMMC_INIT  //DEBUG_EMMC_CONFIG
    printf("------------------------------\n");
    printf(" - High Capacity Mode : %d (1:On, 0:Off)\n", emmcdev->ExtCSD.ERASE_GROUP_DEF);
    printf(" Data Area Size = %d[MB]\n", (uint32_t)(emmcdev->config.DataSize >> 20));
    printf(" Read  Block Length = %d[B]\n", emmcdev->config.ReadBlkLen);
    printf(" Write Block Length = %d[B]\n", emmcdev->config.WriteBlkLen);
    
    if( emmcdev->ExtCSD.ERASE_GROUP_DEF == EMMC_ON )
        printf(" Erase Unit Size   = %d[512KB] : %d[MB]\n", emmcdev->config.EraseUnitSize, emmcdev->config.EraseUnitSize/2);
    else
        printf(" Erase Unit Size   = %d[512B] : %d[KB]\n", emmcdev->config.EraseUnitSize, emmcdev->config.EraseUnitSize/2);
    
    //printf(" Erase Unit Sector = %d[512B]\n", emmcdev->config.EraseUnitSector);
    printf(" Erase Timeout     = %d[ms]\n", emmcdev->config.EraseTimeout);
    
    if( emmcdev->ExtCSD.ERASE_GROUP_DEF == EMMC_ON )
        printf(" Write Protection Group Size = %d[512KB] : %d[MB]\n", emmcdev->config.WpGrpSize, emmcdev->config.WpGrpSize/2);
    else
        printf(" Write Protection Group Size = %d[512B] : %d[KB]\n", emmcdev->config.WpGrpSize, emmcdev->config.WpGrpSize/2);
    
    printf("------------------------------\n");
    
    // Check Status
    emmcdev->CST = emmc_CMD13_C0ba_acR1_SEND_STATUS( emmcdev );
    // printf(New Extended_CSD
    emmc_Print_HostAndEmmcInfo( emmcdev );
#endif
    //--------------------------------------------
    
}


void emmc_Setup_BusFreqWidth( emmcdev_t *emmcdev )
{
    uint32_t reg_ctrl_set0, reg_ctrl_set1;
    uint32_t reg_ip_dly, reg_op_dly;
    
#if DEBUG_EMMC_INIT  //DEBUG_EMMC_CONFIG
    printf(" [emmc_Setup_BusFreqWidth]\n");
    printf("    emmcdev->config.HSTiming  : %d \n", emmcdev->config.HSTiming);
    printf("    emmcdev->config.HostHS_On : %d \n", emmcdev->config.HostHS_On);
    printf("    emmcdev->config.BusFreq   : %d (1=50M,2=25M,3=13M,4=6M,5=3M)\n", emmcdev->config.BusFreq);
    printf("    emmcdev->config.BusWidth  : %d (0=1-bit,1=4-bit,2=8-bit)\n", emmcdev->config.BusWidth);
    printf("    emmcdev->config.InterruptSleep  : %d \n", emmcdev->config.InterruptSleep);
#endif

    //--------------------------------------------
    // [Step 1] Bus Frequency Setting : eMMC, Host controller
    // [Step 1-1] eMMC
    if( emmcdev->config.HSTiming == HS_TIMING_HS )
    {
        //High Speed Mode : ~ 52MHz, 50MHz(100MHz/2), High speed Mode
        emmcdev->CST = emmc_CMD6_C0ba_acR1b_SWITCH_ExtCSD( Idx_ExtCSD_ACC_WRB, Idx_ExtCSD_HS_TIMING, HS_TIMING_HS, emmcdev );        // arg_Access, arg_Index, arg_Val )
    }
    else
    {
        //Full Speed Mode : ~ 26MHz, 25MHz(100MHz/4), Base clock : 100MHz 
        emmcdev->CST = emmc_CMD6_C0ba_acR1b_SWITCH_ExtCSD( Idx_ExtCSD_ACC_WRB, Idx_ExtCSD_HS_TIMING, HS_TIMING_FULL, emmcdev );       // arg_Access, arg_Index, arg_Val )
    }
    cfe_usleep(1000 * emmcdev->config.GenCmd6Timeout );     // Important
    // [Step 1-2] Host controller
    reg_ctrl_set0 = EMMC_HOSTIF->emmc_host_ctrl_set0;
    reg_ctrl_set1 = EMMC_HOSTIF->emmc_host_ctrl_set1;
    SET_REG_FIELD( reg_ctrl_set0, EMMC_HOSTIF_CTRL_SET0_HIGH_SPEED_ENABLE, emmcdev->config.HostHS_On );  // 
    SET_REG_FIELD( reg_ctrl_set1, EMMC_HOSTIF_CTRL_SET1_FREQ_CTRL, emmcdev->config.BusFreq );            // Need to tune set-up/hold time manually (on after 7425B2)
    EMMC_HOSTIF->emmc_host_ctrl_set0 = reg_ctrl_set0;
    EMMC_HOSTIF->emmc_host_ctrl_set1 = reg_ctrl_set1;
    // [Step 1-3] In(Read)/Out(Write) Delay Control (IO_Pad) for High Speed Mode
    if( emmcdev->config.HSTiming == HS_TIMING_HS )
    {        
        reg_ip_dly = EMMC_TOP_CFG->emmc_top_cfg_ip_dly;
        reg_op_dly = EMMC_TOP_CFG->emmc_top_cfg_op_dly;
#if EMMC_IO_PAD_4_HS	
        SET_REG_FIELD( reg_ip_dly, EMMC_TOP_CFG_IP_DLY_IP_TAP_EN,     emmcdev->config.HostInDly_On   );
        SET_REG_FIELD( reg_ip_dly, EMMC_TOP_CFG_IP_DLY_IP_DELAY_CTRL, emmcdev->config.HostInDly_Ctrl );
        SET_REG_FIELD( reg_ip_dly, EMMC_TOP_CFG_IP_DLY_IP_TAP_DELAY,  emmcdev->config.HostInDly_Dly  );
        SET_REG_FIELD( reg_op_dly, EMMC_TOP_CFG_OP_DLY_OP_TAP_EN,     emmcdev->config.HostOutDly_On   );
        SET_REG_FIELD( reg_op_dly, EMMC_TOP_CFG_OP_DLY_OP_DELAY_CTRL, emmcdev->config.HostOutDly_Ctrl );
        SET_REG_FIELD( reg_op_dly, EMMC_TOP_CFG_OP_DLY_OP_TAP_DELAY,  emmcdev->config.HostOutDly_Dly  );
#endif	
        EMMC_TOP_CFG->emmc_top_cfg_ip_dly = reg_ip_dly;
        EMMC_TOP_CFG->emmc_top_cfg_op_dly = reg_op_dly;
    }
    //--------------------------------------------
    
    //--------------------------------------------
    // [Step 2] Bus Width Setting : eMMC, Host controller
    switch( emmcdev->config.BusWidth )
    {
        case BUS_WIDTH_1BIT:
            emmcdev->CST = emmc_CMD6_C0ba_acR1b_SWITCH_ExtCSD( Idx_ExtCSD_ACC_WRB, Idx_ExtCSD_BUS_WIDTH, BUS_WIDTH_1BIT, emmcdev );
            SET_REG_FIELD( reg_ctrl_set0, EMMC_HOSTIF_CTRL_SET0_SD_4BIT_MODE, 0 );
            SET_REG_FIELD( reg_ctrl_set0, EMMC_HOSTIF_CTRL_SET0_SD_8BIT_MODE, 0 );
            break;
        case BUS_WIDTH_4BIT:
            emmcdev->CST = emmc_CMD6_C0ba_acR1b_SWITCH_ExtCSD( Idx_ExtCSD_ACC_WRB, Idx_ExtCSD_BUS_WIDTH, BUS_WIDTH_4BIT, emmcdev );
            SET_REG_FIELD( reg_ctrl_set0, EMMC_HOSTIF_CTRL_SET0_SD_4BIT_MODE, 1 );
            SET_REG_FIELD( reg_ctrl_set0, EMMC_HOSTIF_CTRL_SET0_SD_8BIT_MODE, 0 );
            break;
        case BUS_WIDTH_8BIT:
            emmcdev->CST = emmc_CMD6_C0ba_acR1b_SWITCH_ExtCSD( Idx_ExtCSD_ACC_WRB, Idx_ExtCSD_BUS_WIDTH, BUS_WIDTH_8BIT, emmcdev );
            SET_REG_FIELD( reg_ctrl_set0, EMMC_HOSTIF_CTRL_SET0_SD_4BIT_MODE, 0 );
            SET_REG_FIELD( reg_ctrl_set0, EMMC_HOSTIF_CTRL_SET0_SD_8BIT_MODE, 1 );
            break;
        default:
            emmcdev->CST = emmc_CMD6_C0ba_acR1b_SWITCH_ExtCSD( Idx_ExtCSD_ACC_WRB, Idx_ExtCSD_BUS_WIDTH, BUS_WIDTH_1BIT, emmcdev );
            SET_REG_FIELD( reg_ctrl_set0, EMMC_HOSTIF_CTRL_SET0_SD_4BIT_MODE, 0 );
            SET_REG_FIELD( reg_ctrl_set0, EMMC_HOSTIF_CTRL_SET0_SD_8BIT_MODE, 0 );
            break;
    }
    // Set registers
    EMMC_HOSTIF->emmc_host_ctrl_set0 = reg_ctrl_set0;
    cfe_usleep(1000 * emmcdev->config.GenCmd6Timeout );     // After CMD6
    //--------------------------------------------
    
    //--------------------------------------------
    // [Step 3] Sleep Time Control
    emmcdev->config.InterruptSleep  = EMMC_INT_SLEEP;    // 50MHz * 512 = 10[us], 25MHz * 512 = 20[us]
    emmcdev->config.ReadBufSleep    = EMMC_INT_SLEEP;
    emmcdev->config.WriteBufSleep   = EMMC_INT_SLEEP;
    //--------------------------------------------
    
    //--------------------------------------------
    // [Step 4] BOOT_BUS_WIDTH : BRCM EMMC BOOT support only 8-bit.
#if (EMMC_BOOT_SEL==EMMC_ON)    
    //printf("\n\n\n emmcdev->ExtCSD.BOOT_BUS_CONDITIONS = 0x%X \n\n\n", emmcdev->ExtCSD.BOOT_BUS_CONDITIONS);
    if( emmcdev->ExtCSD.BOOT_BUS_CONDITIONS != 0x02 )
    {
        emmcdev->CST = emmc_CMD6_C0ba_acR1b_SWITCH_ExtCSD( \
            Idx_ExtCSD_ACC_WRB, Idx_ExtCSD_BOOT_BUS_CONDITIONS, 0x02, emmcdev );    // SDR_Boot_Mode, 8-bit, For 40nm
        //  Idx_ExtCSD_ACC_WRB, Idx_ExtCSD_BOOT_BUS_CONDITIONS, (emmcdev->ExtCSD.BOOT_BUS_CONDITIONS&0xFC)|0x02, emmcdev ); // 8-bit, For 40nm
        //  Idx_ExtCSD_ACC_WRB, Idx_ExtCSD_BOOT_BUS_CONDITIONS, (emmcdev->ExtCSD.BOOT_BUS_CONDITIONS&0xE4)|0x0A, emmcdev ); // HS_Boot_Mode + 8-bit, For 28nm
        cfe_usleep(1000 * emmcdev->config.GenCmd6Timeout );     // Important
    }
#endif
     //--------------------------------------------
}


#if EMMC_PARTITION_CTRL
int emmc_Partition_DataArea( emmcdev_t *emmcdev )
{
    uint32_t i, PartitionCtrl=0, PartitionError=0, PartitionTryCnt=0;
    uint32_t PartitionAttribute=0, ExtPartitionAttribute=0;
    uint32_t GP1_SizeMulti=0, GP2_SizeMulti=0, GP3_SizeMulti=0, GP4_SizeMulti=0;
    uint32_t ENH_SizeMulti=0;
    uint64_t ENH_StartAddr=0;
    uint64_t TotalEnhSize=0;
    uint32_t PartSizeTemp, PartAddrTemp;
    
    // Check partition support
    if( (emmcdev->ExtCSD.PARTITIONING_SUPPORT & EMMC_PART_SUP_EN) && EMMC_PHY_PARTITION )
    {
        PartitionCtrl = EMMC_PART_SUP_EN;
        
        if( EMMC_PART_ATTR_ALL )
        {
            if( (emmcdev->ExtCSD.PARTITIONING_SUPPORT & EMMC_PART_SUP_ENH) && EMMC_PART_ATTR_ALL )
            {
                PartitionCtrl += EMMC_PART_SUP_ENH;
            }
            else
            {
                printf("---> This eMMC doesn't support enhanced attribution. Please check your eMMC. !!!\n");
            }
        }
        
        if( EMMC_PART_ATTR_EXT )
        {
            if( (emmcdev->ExtCSD.PARTITIONING_SUPPORT & EMMC_PART_SUP_EXT) && EMMC_PART_ATTR_EXT )
            {
                PartitionCtrl += EMMC_PART_SUP_EXT;
            }
            else
            {
                printf("---> This eMMC doesn't support extended attribution. Please check your eMMC. !!!\n");
            }
        }
        
        // Get partition attribute.
        if( EMMC_PART_ATTR_GP1 ){   TotalEnhSize = EMMC_PART_SIZE_GP1<<20;  }
        if( EMMC_PART_ATTR_GP2 ){   TotalEnhSize += EMMC_PART_SIZE_GP2<<20; }
        if( EMMC_PART_ATTR_GP3 ){   TotalEnhSize += EMMC_PART_SIZE_GP3<<20; }
        if( EMMC_PART_ATTR_GP4 ){   TotalEnhSize += EMMC_PART_SIZE_GP4<<20; }
        if( EMMC_PART_ATTR_ENH ){   TotalEnhSize += EMMC_PART_SIZE_ENH<<20; }

        // Check all paramters are correct.
        if( EMMC_GP1_PARTITION )
        {   
            if( (EMMC_PART_SIZE_GP1<<20) < emmcdev->config.HcWpGrpSize )
            {
                printf("---> EMMC_PART_SIZE_GP1 Error\n");
                PartitionError++;
            }
        }
        if( EMMC_GP2_PARTITION )
        {   
            if( (EMMC_PART_SIZE_GP2<<20) < emmcdev->config.HcWpGrpSize )
            {
                printf("---> EMMC_PART_SIZE_GP2 Error\n");
                PartitionError++;
            }
        }
        if( EMMC_GP3_PARTITION )
        {   
            if( (EMMC_PART_SIZE_GP3<<20) < emmcdev->config.HcWpGrpSize )
            {
                printf("---> EMMC_PART_SIZE_GP3 Error\n");
                PartitionError++;
            }
        }
        if( EMMC_GP4_PARTITION )
        {   
            if( (EMMC_PART_SIZE_GP4<<20) < emmcdev->config.HcWpGrpSize )
            {
                printf("---> EMMC_PART_SIZE_GP4 Error\n");
                PartitionError++;
            }
        }
        if( EMMC_ENH_PARTITION )
        {   
            if( (EMMC_PART_SIZE_ENH<<20) < emmcdev->config.HcWpGrpSize )
            {
                printf("---> EMMC_PART_SIZE_ENH Error\n");
                PartitionError++;
            }
            if( ((uint64_t)(EMMC_PART_SIZE_ENH<<20) + EMMC_PART_ENH_ADDR) > emmcdev->config.DataSize )
            {   
                printf("---> EMMC_PART_ENH_ADDR Error\n");
                PartitionError++;
            }
        }
        
        if( (TotalEnhSize > emmcdev->config.MaxEnhSize) || PartitionError )
        {
            printf("---> The set total enhanced size is over Max Enhanced Size of eMMC. Please check parameters in 'bsp_config.h'. !!!\n");
            printf("     - TotalEnhSize = 0x%llu, MaxEnhSize = 0x%llu\n", TotalEnhSize, emmcdev->config.MaxEnhSize);
            PartitionCtrl = EMMC_NG;
        }   
    }
    else
    {   
        printf("---> Thie eMMC doesn't support partition in Data Area. Please check your eMMC. !!!\n");
        PartitionCtrl = EMMC_NG;
    }
    //printf("[emmc_Partition_DataArea] 1.PartitionCtrl = 0x%08X\n", PartitionCtrl);
    
    // Partition!!!
    if( PartitionCtrl != EMMC_NG )
    {
        set_partition_parameters:
        
        // [Step 0] Set ERASE_GROUP_DEF
        if( emmcdev->config.OCR_SectorMode == EMMC_ON )
        {
            emmcdev->config.EraseGroupDef    = EMMC_ON;
            emmcdev->CST = emmc_CMD6_C0ba_acR1b_SWITCH_ExtCSD( Idx_ExtCSD_ACC_WRB, Idx_ExtCSD_ERASE_GROUP_DEF, EMMC_ON, emmcdev );
            cfe_usleep(1000 * emmcdev->config.GenCmd6Timeout );
        }
        
        // [Step 1] Set Partition Attribute
        PartitionAttribute = (uint32_t)EMMC_PART_ATTR_ENH;
        PartitionAttribute |= ((uint32_t)EMMC_PART_ATTR_GP1<<1);
        PartitionAttribute |= ((uint32_t)EMMC_PART_ATTR_GP2<<2);
        PartitionAttribute |= ((uint32_t)EMMC_PART_ATTR_GP3<<3);
        PartitionAttribute |= ((uint32_t)EMMC_PART_ATTR_GP4<<4);
        
        emmcdev->CST = emmc_CMD6_C0ba_acR1b_SWITCH_ExtCSD( Idx_ExtCSD_ACC_WRB, Idx_ExtCSD_PARTITIONS_ATTRIBUTE, PartitionAttribute, emmcdev );
        cfe_usleep(1000 * emmcdev->config.GenCmd6Timeout );
        //printf("[emmc_Partition_DataArea] 3.PartitionAttribute = 0x%08X\n", PartitionAttribute);
        
        // [Step 2] Set Partition Size/StartAddress
#if EMMC_GP1_PARTITION
        GP1_SizeMulti = (uint32_t)EMMC_PART_SIZE_GP1 / (emmcdev->config.HcWpGrpSize>>20);
        for( i=0; i<3; i++ )
        {   
            PartSizeTemp = (uint32_t)( (GP1_SizeMulti>>(8*i)) & 0xFF );
            emmcdev->CST = emmc_CMD6_C0ba_acR1b_SWITCH_ExtCSD( Idx_ExtCSD_ACC_WRB, Idx_ExtCSD_GP_SIZE_MULT+0+i, PartSizeTemp, emmcdev );
            cfe_usleep(1000 * emmcdev->config.GenCmd6Timeout );
        }
        //printf("[emmc_Partition_DataArea] 4.GP1_SizeMulti = 0x%0X\n", GP1_SizeMulti);
#endif
        
#if EMMC_GP2_PARTITION
        GP2_SizeMulti = (uint32_t)EMMC_PART_SIZE_GP2 / (emmcdev->config.HcWpGrpSize>>20);
        for( i=0; i<3; i++ )
        {   
            PartSizeTemp = (uint32_t)( (GP2_SizeMulti>>(8*i)) & 0xFF );
            emmcdev->CST = emmc_CMD6_C0ba_acR1b_SWITCH_ExtCSD( Idx_ExtCSD_ACC_WRB, Idx_ExtCSD_GP_SIZE_MULT+3+i, PartSizeTemp, emmcdev );
            cfe_usleep(1000 * emmcdev->config.GenCmd6Timeout );
        }
        //printf("[emmc_Partition_DataArea] 4.GP2_SizeMulti = 0x%0X\n", GP2_SizeMulti);
#endif
        
#if EMMC_GP3_PARTITION
        GP3_SizeMulti = (uint32_t)EMMC_PART_SIZE_GP3 / (emmcdev->config.HcWpGrpSize>>20);
        for( i=0; i<3; i++ )
        {   
            PartSizeTemp = (uint32_t)( (GP3_SizeMulti>>(8*i)) & 0xFF );
            emmcdev->CST = emmc_CMD6_C0ba_acR1b_SWITCH_ExtCSD( Idx_ExtCSD_ACC_WRB, Idx_ExtCSD_GP_SIZE_MULT+6+i, PartSizeTemp, emmcdev );
            cfe_usleep(1000 * emmcdev->config.GenCmd6Timeout );
        }
        //printf("[emmc_Partition_DataArea] 4.GP3_SizeMulti = 0x%0X\n", GP3_SizeMulti);
#endif
        
#if EMMC_GP4_PARTITION
        GP4_SizeMulti = (uint32_t)EMMC_PART_SIZE_GP4 / (emmcdev->config.HcWpGrpSize>>20);
        for( i=0; i<3; i++ )
        {   
            PartSizeTemp = (uint32_t)( (GP4_SizeMulti>>(8*i)) & 0xFF );
            emmcdev->CST = emmc_CMD6_C0ba_acR1b_SWITCH_ExtCSD( Idx_ExtCSD_ACC_WRB, Idx_ExtCSD_GP_SIZE_MULT+9+i, PartSizeTemp, emmcdev );
            cfe_usleep(1000 * emmcdev->config.GenCmd6Timeout );
        }
        //printf("[emmc_Partition_DataArea] 4.GP4_SizeMulti = 0x%0X\n", GP4_SizeMulti);
#endif
        
#if EMMC_ENH_PARTITION
        ENH_SizeMulti = (uint32_t)EMMC_PART_SIZE_ENH / (emmcdev->config.HcWpGrpSize>>20);
        for( i=0; i<3; i++ )
        {   
            PartSizeTemp = (uint32_t)( (ENH_SizeMulti>>(8*i)) & 0xFF );
            emmcdev->CST = emmc_CMD6_C0ba_acR1b_SWITCH_ExtCSD( Idx_ExtCSD_ACC_WRB, Idx_ExtCSD_ENH_SIZE_MULT+i, PartSizeTemp, emmcdev );
            cfe_usleep(1000 * emmcdev->config.GenCmd6Timeout );
        }
        //printf("[emmc_Partition_DataArea] 4.ENH_SizeMulti = 0x%0X\n", ENH_SizeMulti);
        
        // Send StartAddr to eMMC
        ENH_StartAddr = (uint64_t)EMMC_PART_ENH_ADDR;
        if( emmcdev->config.OCR_SectorMode == EMMC_ON )
        {
            ENH_StartAddr >>= emmcdev->config.ReadBlkLenBit4Addr;
        }
        for( i=0; i<4; i++ )
        {   
            PartAddrTemp = (uint32_t)( (ENH_StartAddr>>(8*i)) & 0xFF );
            emmcdev->CST = emmc_CMD6_C0ba_acR1b_SWITCH_ExtCSD( Idx_ExtCSD_ACC_WRB, Idx_ExtCSD_ENH_START_ADDR+i, PartAddrTemp, emmcdev );
            cfe_usleep(1000 * emmcdev->config.GenCmd6Timeout );
        }
        //printf("[emmc_Partition_DataArea] 4.ENH_StartAddr = 0x%llu\n", ENH_StartAddr);
#endif
        
        // Send ExtPartitionAttribute to eMMC
        if( PartitionCtrl & EMMC_PART_SUP_EXT )
        {
            ExtPartitionAttribute = ((uint32_t)EMMC_PART_ATTR_EXT_GP1);
            ExtPartitionAttribute |= ((uint32_t)EMMC_PART_ATTR_EXT_GP1<<4);
            ExtPartitionAttribute |= ((uint32_t)EMMC_PART_ATTR_EXT_GP1<<8);
            ExtPartitionAttribute |= ((uint32_t)EMMC_PART_ATTR_EXT_GP1<<12);
            
            emmcdev->CST = emmc_CMD6_C0ba_acR1b_SWITCH_ExtCSD( Idx_ExtCSD_ACC_WRB, Idx_ExtCSD_EXT_PARTITIONS_ATTRIBUTE, (ExtPartitionAttribute & 0xFF), emmcdev );
            cfe_usleep(1000 * emmcdev->config.GenCmd6Timeout );
            emmcdev->CST = emmc_CMD6_C0ba_acR1b_SWITCH_ExtCSD( Idx_ExtCSD_ACC_WRB, Idx_ExtCSD_EXT_PARTITIONS_ATTRIBUTE+1, ((ExtPartitionAttribute>>8) & 0xFF), emmcdev );
            cfe_usleep(1000 * emmcdev->config.GenCmd6Timeout );
            //printf("[emmc_Partition_DataArea] 5.ExtPartitionAttribute = 0x%0X\n", ExtPartitionAttribute);
        }
        
        
        //------------------------------------------------------------
        // [Step 3] [Important] Notify the device that the host has completed partitioning configuration.
        emmcdev->CST = emmc_CMD13_C0ba_acR1_SEND_STATUS( emmcdev );
        if( emmcdev->CST & CST_SWITCH_ERROR_MASK )
        {
            PartitionTryCnt++;
            if( PartitionTryCnt < 4 )
            {   
                printf("[emmc_Partition_DataArea] 6.CST_SWITCH_ERROR : PartitionTryCnt = 0x%0X\n", PartitionTryCnt);
                goto set_partition_parameters;
            }
            else
            {
                printf("\n\n !!! Partition parameters are not correct. Please check parameters in 'bsp_config.h'. !!!\n\n");
                return EMMC_ERROR;
            }   
        }
        //------------------------------------------------------------
        
        //------------------------------------------------------------
        // [Step 4] [Important] Notify the device that the host has completed partitioning configuration.
        //printf("[emmc_Partition_DataArea] 7.ExtCSD_PARTITION_SETTING_COMPLETED Start\n");
        emmcdev->CST = emmc_CMD6_C0ba_acR1b_SWITCH_ExtCSD( Idx_ExtCSD_ACC_WRB, Idx_ExtCSD_PARTITION_SETTING_COMPLETED, EMMC_ON, emmcdev );
        cfe_usleep(1000 * emmcdev->config.GenCmd6Timeout );
        //------------------------------------------------------------
        
        //------------------------------------------------------------
        // [Step 5] Power Cycle
        printf("\n\n !!! Please wait about 7[sec] while partitioning. !!!\n\n");
        cfe_usleep(1000 * emmcdev->ExtCSD.INI_TIMEOUT_AP * 100 + 2000 );        // extra 2sec, [ms]
        //------------------------------------------------------------
        
        //------------------------------------------------------------
        // [Step 6] Check configured partition information.
        emmcdev->CST = emmc_CMD8_C0ba_adtcR1_SEND_EXT_CSD( emmcdev );
        PartitionError = 0;
        if( emmcdev->ExtCSD.PARTITION_SETTING_COMPLETED == EMMC_NG ){   PartitionError++;   }
        if( emmcdev->ExtCSD.PARTITIONS_ATTRIBUTE != PartitionAttribute ){   PartitionError++;   }
        if( emmcdev->ExtCSD.GP_SIZE_MULT[0] != GP1_SizeMulti ){ PartitionError++;   }
        if( emmcdev->ExtCSD.GP_SIZE_MULT[1] != GP2_SizeMulti ){ PartitionError++;   }
        if( emmcdev->ExtCSD.GP_SIZE_MULT[2] != GP3_SizeMulti ){ PartitionError++;   }
        if( emmcdev->ExtCSD.GP_SIZE_MULT[3] != GP4_SizeMulti ){ PartitionError++;   }
        if( emmcdev->ExtCSD.ENH_SIZE_MULT   != ENH_SizeMulti ){ PartitionError++;   }
        if( emmcdev->ExtCSD.ENH_START_ADDR  != ENH_StartAddr ){ PartitionError++;   }
        if( PartitionCtrl & EMMC_PART_SUP_EXT )
        {
            if( emmcdev->ExtCSD.EXT_PARTITIONS_ATTRIBUTE != ExtPartitionAttribute ){ PartitionError++;  }
        }
        
        if( PartitionError )
        {
            printf("\n\n !!! The partition might not complete. Please check eMMC device after power off & on. !!!\n\n");
            master_reboot( );
            return EMMC_ERROR;
        }
        else
        {
            printf("\n\n !!! The partition completed. congratulation. Please power off & on to complete partition if reboot has issue. !!!\n\n");
            cfe_usleep(1000 * 2000 );
            master_reboot( );
            return EMMC_OK;
        }
        //------------------------------------------------------------
    }
    else
    {
        printf("\n\n !!! The current partition parameters are something wrong. Please check parameters in 'bsp_config.h'. !!!\n\n");
        return EMMC_ERROR;
    }
    

}
#endif


int emmc_Sel_Partition( emmcdev_t *emmcdev, uint32_t partition_sel )
{
    uint32_t            partition_config=0x00;  // No partition for boot

    emmcdev->config.InterruptSleep = EMMC_INT_SLEEP;
    
    if( partition_sel == EMMC_BOOT1_4_BOOT )
    {    
        partition_config = 0x48;    // For Boot1 partition to boot on next power on.
        emmcdev->config.BootPartitionEnable = EMMC_BOOT1_4_BOOT;
    }
    else if( partition_sel == EMMC_BOOT2_4_BOOT )
    {
        partition_config = 0x50;    // For Boot2 partition to boot on next power on.
        emmcdev->config.BootPartitionEnable = EMMC_BOOT2_4_BOOT;
    }
    
    emmcdev->CST = emmc_CMD6_C0ba_acR1b_SWITCH_ExtCSD( Idx_ExtCSD_ACC_WRB, Idx_ExtCSD_PARTITION_CONFIG, partition_config, emmcdev );
    cfe_usleep(1000 * emmcdev->config.PartitionSwitchTime );       // Important
    
    emmcdev->CST = emmc_CMD8_C0ba_adtcR1_SEND_EXT_CSD( emmcdev );
    printf(" emmc_config->ExtCSD.PARTITION_CONFIG = 0x%02X (%d)\n", emmcdev->ExtCSD.PARTITION_CONFIG, emmcdev->ExtCSD.PARTITION_CONFIG);    
        
    return 0;
}

//-------------------------------------------------
//===========================================================



//===========================================================
// Command Implementation
//-------------------------------------------------
// Respons Type Information
//  R1 (Index check  On, CRC check  On, 48bit)  
//     : CMD 3,7,8,11,12,13,14,19,16,17,18,20,23,24,25,26,27,30,31,35,36,42,55,56
//  R1b(Index check  On, CRC check  On, 48bit/busy_check) : CMD 5,6,7,12,28.29,38
//  R2 (Index check Off, CRC check  On, 136bit) : CMD 2,9,10
//  R3 (Index check Off, CRC check Off, 48bit)  : CMD 1
//  R4 (Index check Off, CRC check Off, 48bit)  : CMD 39
//  R5 (Index check  On, CRC check  On, 48bit)  : CMD 40
// Clase Information v4.41
//  class 0 : basic : CMD 0,1,2,3,4,5,6,7,8,9,10,12,13,14,15,19
//  class 1 : stream read (Obsolete on v4.5) : CMD 11
//  class 2 : block read          : CMD 16,17,18,21(v4.5)
//  class 3 : stream write (Obsolete on v4.5) : CMD 20
//  class 4 : block write         : CMD 23,24,25,26,27,49(v4.5)
//  class 5 : erase               : CMD 35,36,38
//  class 6 : write protection    : CMD 28,29,30,31
//  class 7 : lock card           : CMD 42
//  class 8 : applicationspecific : CMD 55,56
//  class 9 : I/O mode            : CMD 39,40
//  class 10?11 : reserved
//-------------------------------------------------
//-------------------------------------------------
// Class 0 Basic Commands
// <summary>
// Sending CMD0 To the eMMC device. There is no response.
// </summary>
// <param name="arg">Argument To send with CMD0.</param>
// <remarks>
// Depending on the argument being set, there are three type oc commands:
// 0x00000000 (GO_IDLE_STATE)     - Resets the card To idle state.
// 0xF0F0F0F0 (GO_PRE_IDLE_STATE) - Resets the card To pre-idle state
// 0xFFFFFFFA (BOOT_INITIATION)   - Initiate alternative boot operation
// </remarks>
uint32_t emmc_CMD0_C0ba_bc_GO_IDLE_STATE( emmcdev_t *emmcdev )
{
    return emmc_CMD0_C0ba_bc( 0x00000000, emmcdev );
    
}


uint32_t emmc_CMD0_C0ba_bc_GO_PRE_IDLE_STATE( emmcdev_t *emmcdev )
{
    return emmc_CMD0_C0ba_bc( 0xF0F0F0F0, emmcdev );
    
}

uint32_t emmc_CMD0_C0ba_bc_BOOT_INITIATION( emmcdev_t *emmcdev )
{
    // boot initiation in Pre-boot state
    return emmc_CMD0_C0ba_bc( 0xFFFFFFFA, emmcdev );
    
}

uint32_t emmc_CMD0_C0ba_bc( uint32_t cmd0_cmd, emmcdev_t *emmcdev )
{
    //'----------------
    //' CMD 0 - GO_IDLE_STATE (bc,-), GO_PRE_IDLE_STATE (bc,-), BOOT_INITIATION (-,-) 
    //'----------------
#if DEBUG_EMMC_CMD
    if( cmd0_cmd == 0x00000000 )
        printf("Sending CMD0_GO_IDLE_STATE (bc, -) : 0x%08X\n", cmd0_cmd);
    if( cmd0_cmd == 0xF0F0F0F0 )
        printf("Sending CMD0_GO_PRE_IDLE_STATE (bc, -) : 0x%08X\n", cmd0_cmd);
    if( cmd0_cmd == 0xFFFFFFFA )
        printf("Sending CMD0_BOOT_INITIATION (-, -) : 0x%08X\n", cmd0_cmd);
#endif
    
    emmc_Clear_HostIntStatus( SDIO_HOST_INT_CLR_ALL );
    
    //rd_data = EMMC_HOSTIF->emmc_host_int_status;
    //EMMC_HOSTIF->emmc_host_int_status = rd_data;
    EMMC_HOSTIF->emmc_host_argument = cmd0_cmd;
    EMMC_HOSTIF->emmc_host_cmd_mode = 0x00000000;    //cmd<<24 | resp<<16;
    // CMD0, No Response

    if( (emmc_Wait_CMD_complete( emmcdev->config.InterruptSleep, 0 ) & EMMC_OK) == EMMC_NG )
    {
        printf("ERROR: CMD 0 didn't complete\n");
        return EMMC_NG;
    }
    else
    {   
#if DEBUG_EMMC_CMD
        printf(" CMD 0 : Success!!!\n");
#endif        
        return EMMC_OK;
    }
    
}


// <summary>
// Get the card in idle state To send its Operating Conditions Register (OCR) contents in the response
// </summary>
// <param name="arg">Argument send with CMD1</param>
// <returns></returns>
uint32_t emmc_CMD1_C0ba_bcrR3_SEND_OP_COND( uint32_t arg_OCR, emmcdev_t *emmcdev )
{
    uint32_t OCR_data;
    
#if DEBUG_EMMC_CMD
    printf("Sending CMD1_SEND_OP_COND (bcr, R3) : 0x%08X (OCR without busy)\n", arg_OCR);
#endif    
    
    emmc_Clear_HostIntStatus( SDIO_HOST_INT_CLR_ALL );
    
    // Setup Registers
    EMMC_HOSTIF->emmc_host_argument = arg_OCR;
    EMMC_HOSTIF->emmc_host_cmd_mode = 0x01020000;
    //' CMD1, R3 (Index check Off, CRC check Off, 48bit)=0x00020000
    
    if( (emmc_Wait_CMD_complete( emmcdev->config.InterruptSleep, 1 ) & EMMC_OK) == EMMC_NG )
    {
        printf("ERROR: CMD 1 didn't complete\n");
        return EMMC_STATUS_ERROR;
    }
    else
    {   
        OCR_data = EMMC_HOSTIF->emmc_host_resp_01;
#if DEBUG_EMMC_CMD
        printf(" CMD 1 resp(OCR): 0x%08X\n", OCR_data );
#endif
        return OCR_data;
    }
    
}


// <summary>
// Gets card CID (Card Identification) from eMMC device.
// </summary>
uint32_t emmc_CMD2_C0ba_bcrR2_ALL_SEND_CID_SingleDevice( emmcdev_t *emmcdev )
{
    uint32_t resp01, resp23, resp45, resp67;
    
#if DEBUG_EMMC_CMD
      printf("Sending CMD2_ALL_SEND_CID (bcr, R2)\n");
#endif
    
    emmc_Clear_HostIntStatus( SDIO_HOST_INT_CLR_ALL );
    
    // Setup Registers
    EMMC_HOSTIF->emmc_host_argument = 0x0000;
    EMMC_HOSTIF->emmc_host_cmd_mode = 0x02090000;
    // CMD2, R2 (Index check Off, CRC check  On, 136bit) = 0x00090000 : CMD 2/9/10

    if( (emmc_Wait_CMD_complete( emmcdev->config.InterruptSleep, 2 ) & EMMC_OK) == EMMC_NG )
    {
        printf("ERROR: CMD 2 didn't complete\n");
        return EMMC_STATUS_ERROR;
    }
    else
    {   
        resp67 = EMMC_HOSTIF->emmc_host_resp_67;
        resp45 = EMMC_HOSTIF->emmc_host_resp_45;
        resp23 = EMMC_HOSTIF->emmc_host_resp_23;
        resp01 = EMMC_HOSTIF->emmc_host_resp_01;
        emmcdev->CID.MID = (uint8_t)(resp67 >> 16);
        emmcdev->CID.CBX = (uint8_t)(resp67 >>  8);
        emmcdev->CID.OID = (uint8_t)(resp67);
        emmcdev->CID.PNM[0] = (uint8_t)(resp45 >> 24);
        emmcdev->CID.PNM[1] = (uint8_t)(resp45 >> 16);
        emmcdev->CID.PNM[2] = (uint8_t)(resp45 >>  8);
        emmcdev->CID.PNM[3] = (uint8_t)(resp45 >>  0);
        emmcdev->CID.PNM[4] = (uint8_t)(resp23 >> 24);
        emmcdev->CID.PNM[5] = (uint8_t)(resp23 >> 16);
        emmcdev->CID.PRV = (uint8_t)(resp23 >>  8);
        emmcdev->CID.PSN = (uint32_t)( (uint32_t)(resp01 >> 8) + (resp23 << 24) );
        emmcdev->CID.MDT = (uint8_t)(resp01);

#if DEBUG_EMMC_CONFIG
        
        printf(" CMD 2 response_67 CID[127:104]: 0x%08X\n", resp67);
        printf(" CMD 2 response_45 CID[103: 72]: 0x%08X\n", resp45);
        printf(" CMD 2 response_23 CID[ 71: 40]: 0x%08X\n", resp23);
        printf(" CMD 2 response_01 CID[ 39:  8]: 0x%08X\n", resp01);
        printf(" ManufactureId       : 0x%02X\n", emmcdev->CID.MID);
        printf(" CardBGA             : %d (0:RemovableDevice, 1:BGA, 2:POP, 3:RSVD)\n", emmcdev->CID.CBX);
        printf(" OEMApplicationId    : 0x%02X\n", emmcdev->CID.OID);
        printf(" ProductName         : %s\n", &emmcdev->CID.PNM);
        printf(" ProductRevision     : %d.%d\n", (emmcdev->CID.PRV >> 4), (emmcdev->CID.PRV & 0x0F));
        printf(" ProductSerialNumber : 0x%08X\n", emmcdev->CID.PSN);
        printf(" ManufacturingDate   : %d/%d\n", (uint8_t)(emmcdev->CID.MDT >> 4), (uint16_t)(emmcdev->CID.MDT & 0x0F) + 1997);
#endif

        return EMMC_STATUS_OK;
    }
    
}


// <summary>
// Set relative card address
// </summary>
uint32_t emmc_CMD3_C0ba_acR1_SET_RCA( uint32_t arg_RCA, emmcdev_t *emmcdev )
{
    uint32_t status;

#if DEBUG_EMMC_CMD
    printf("Sending CMD3_SET_RCA (ac, R1) : 0x%08X (RCA)\n", arg_RCA);
#endif
    
    emmc_Clear_HostIntStatus( SDIO_HOST_INT_CLR_ALL );
    
    // Setup Registers
    EMMC_HOSTIF->emmc_host_argument = arg_RCA;
    EMMC_HOSTIF->emmc_host_cmd_mode = 0x031A0000;
    // CMD3, R1 (Index check  On, CRC check  On, 48bit)
    
    if( (emmc_Wait_CMD_complete( emmcdev->config.InterruptSleep, 3 ) & EMMC_OK) == EMMC_NG )
    {
       printf("ERROR: CMD 3 didn't complete\n");
       return EMMC_STATUS_ERROR;
    }
    else
    {
        status = EMMC_HOSTIF->emmc_host_resp_01;

#if DEBUG_EMMC_CMD
        printf(" CMD 3 resp(Status): 0x%08X\n", status);
#endif
        return status;
    }

}


#if EMMC_DRV_FUTURE
uint32_t emmc_CMD4_C0ba_bc_SET_DSR( uint32_t arg_DSR, emmcdev_t *emmcdev )
{
#if DEBUG_EMMC_CMD
        printf("Sending CMD4_SET_DSR( (bc, -) : 0x%02X (DSR)\n", arg_DSR);
#endif
    
    emmc_Clear_HostIntStatus( SDIO_HOST_INT_CLR_ALL );
    
    //rd_data = REG( &EMMC_HOSTIF->emmc_host_int_status);
    //EMMC_HOSTIF->emmc_host_int_status = rd_data;
    EMMC_HOSTIF->emmc_host_argument = (arg_DSR << 16);
    EMMC_HOSTIF->emmc_host_cmd_mode = 0x04000000;
    // CMD4, No Response

    if( (emmc_Wait_CMD_complete( emmcdev->config.InterruptSleep, 4 ) & EMMC_OK) == EMMC_NG )
    {
        printf("ERROR: CMD 4 didn't complete\n");
        return EMMC_STATUS_ERROR;
    }
    else
    {   
#if EMMC_DRV_FUTURE
        printf(" CMD 4 : Success!!!\n");
#endif
        return EMMC_STATUS_OK;
    }
    
}


uint32_t emmc_CMD5_C0ba_acR1b_SLEEP_AWAKE( uint32_t arg_RCA, uint32_t arg_SleepAwake, emmcdev_t *emmcdev )
{
    return EMMC_STATUS_ERROR;
}
#endif


uint32_t emmc_CMD6_C0ba_acR1b_SWITCH_ExtCSD( uint32_t arg_Access, uint32_t arg_Index, uint32_t arg_Val, emmcdev_t *emmcdev )
{
    uint32_t arg, status, timeout_cnt=0;

    arg = 0;
    arg = (arg_Access & 0x03) << 24;
    arg = arg + ((arg_Index & 0xFF) << 16);
    arg = arg + ((arg_Val & 0xFF) << 8);

#if DEBUG_EMMC_CMD
    printf("Sending CMD6_SWITCH_ExtCSD (ac, R1b) : arg = 0x%08X\n", arg);
    printf(" - Access: 0x%02X, [0x%02X] = 0x%02X\n", arg_Access, arg_Index, arg_Val);
#endif
    
    emmc_Clear_HostIntStatus( SDIO_HOST_INT_CLR_ALL );
    
    // Setup Registers
    EMMC_HOSTIF->emmc_host_argument = arg;
    EMMC_HOSTIF->emmc_host_cmd_mode = 0x061B0000;
    // CMD6
    // &h061B0000& : R1b(Index check  On, CRC check  On, 48bit/busy_check)

    cfe_usleep(1000 * 100 );     // Important, for 7563SV board
    
    if( (emmc_Wait_CMD_complete( emmcdev->config.InterruptSleep, 6 ) & EMMC_OK) == EMMC_NG )
    {
       printf("ERROR: CMD 6 didn't complete\n");
       return EMMC_STATUS_ERROR;
    }
    else
    {
        status = EMMC_HOSTIF->emmc_host_resp_01;
        //printf(" CMD 6 ExtCSD resp(Status) EMMC_HOSTIF_RESP_01 : 0x%08X\n", status);
        
        while( (status & CST_SWITCH_ERROR_MASK) && (timeout_cnt < EMMC_TIMEOUT_CNT) )
        {
            timeout_cnt++;
            cfe_usleep( emmcdev->config.InterruptSleep );
            status = emmc_CMD13_C0ba_acR1_SEND_STATUS( emmcdev );
            //printf(" CMD 6 ExtCSD resp(Status) emmc_CMD13_C0ba_acR1_SEND_STATUS : 0x%08X\n", status);
        }
        
#if DEBUG_EMMC_CMD
        printf(" CMD 6 ExtCSD resp(Status): 0x%08X\n", status);
#endif
        
        if( timeout_cnt >= EMMC_TIMEOUT_CNT )
        {
            printf("\n >>>>> emmc_CMD13 status in CMD6_ExtCSD = 0x%08X (timeout_cnt=%d)\n", status, timeout_cnt);
            // Call Error_Recovery_Sequence
            
            return EMMC_NG;
        }
        else
        {
            return status;
        }
    }

}


#if EMMC_DRV_FUTURE
uint32_t emmc_CMD6_C0ba_acR1b_SWITCH_CmdSet( uint32_t arg_CmdSet, emmcdev_t *emmcdev )
{
    uint32_t arg, status, timeout_cnt=0;

    
    arg = (arg_CmdSet & 0x03);
    
#if DEBUG_EMMC_CMD
    printf("Sending CMD6_SWITCH_CmdSet(ac, R1b) : CmdSet = 0x%08X\n", arg);
#endif
    
    emmc_Clear_HostIntStatus( SDIO_HOST_INT_CLR_ALL );
    
    // Setup Registers
    EMMC_HOSTIF->emmc_host_argument = arg;
    EMMC_HOSTIF->emmc_host_cmd_mode = 0x061B0000;
    // CMD6
    // &h061B0000& : R1b(Index check  On, CRC check  On, 48bit/busy_check)

    if( (emmc_Wait_CMD_complete( emmcdev->config.InterruptSleep, 6 ) & EMMC_OK) == EMMC_NG )
    {
       printf("ERROR: CMD 6 didn't complete\n");
       return EMMC_STATUS_ERROR;
    }
    else
    {
        status = EMMC_HOSTIF->emmc_host_resp_01;
        //printf(" CMD 6 CmdSet resp(Status) EMMC_HOSTIF_RESP_01 : 0x%08X\n", status);
        
        while( (status & CST_SWITCH_ERROR_MASK) && (timeout_cnt < EMMC_TIMEOUT_CNT) )
        {
            timeout_cnt++;
            cfe_usleep( emmcdev->config.InterruptSleep );
            status = emmc_CMD13_C0ba_acR1_SEND_STATUS( emmcdev );
            //printf(" CMD 6 ExtCSD resp(Status) emmc_CMD13_C0ba_acR1_SEND_STATUS : 0x%08X\n", status);
        }
                
#if DEBUG_EMMC_CMD
        printf(" CMD 6 CmdSet resp(Status): 0x%08X\n", status);
#endif
        
        return status;
    }

}
#endif


uint32_t emmc_CMD7_C0ba_acR1_SELECT_CARD_stby_tans( uint32_t arg_RCA, emmcdev_t *emmcdev )
{
    uint32_t status;

    // &h071A0000&
    // CMD7
    // R1 (Index check  On, CRC check  On, 48bit) 
    // : while selecting from Stand-By State to Transfer State
    status = emmc_CMD7_c0ba_acR1R1b_SELECT_CARD( arg_RCA, 0x071A0000, emmcdev );

    return status;

}

#if EMMC_DRV_FUTURE
uint32_t emmc_CMD7_C0ba_acR1b_SELECT_CARD_dis_prg( uint32_t arg_RCA, emmcdev_t *emmcdev )
{
    uint32_t status;

    // &h071B0000&
    // CMD7
    // R1b(Index check  On, CRC check  On, 48bit/busy_check) 
    // : while selecting from Disconnected State to Programming State.
    status = emmc_CMD7_c0ba_acR1R1b_SELECT_CARD( arg_RCA, 0x071B0000, emmcdev );

    return status;

}
#endif

uint32_t emmc_CMD7_c0ba_acR1R1b_SELECT_CARD( uint32_t arg_RCA, uint32_t arg_cmd_mode, emmcdev_t *emmcdev )
{
    uint32_t status;
    
#if DEBUG_EMMC_CMD
    if( arg_cmd_mode == 0x071A0000 )
        printf("Sending CMD7_SELECT_CARD_stby_to_tans (ac, R1) : 0x%08X (RCA)\n", arg_RCA);
    if( arg_cmd_mode == 0x071B0000 )
        printf("Sending CMD7_SELECT_CARD_dis_to_prg (ac, R1b) : 0x%08X (RCA)\n", arg_RCA);
#endif
    
    emmc_Clear_HostIntStatus( SDIO_HOST_INT_CLR_ALL );
    
    // Setup Registers
    EMMC_HOSTIF->emmc_host_argument = arg_RCA;
    EMMC_HOSTIF->emmc_host_cmd_mode = arg_cmd_mode;
    // CMD7
    // &h071A0000& : R1 (Index check  On, CRC check  On, 48bit) : while selecting from Stand-By State to Transfer State
    // &h071B0000& : R1b(Index check  On, CRC check  On, 48bit/busy_check) : while selecting from Disconnected State to Programming State.

    if( (emmc_Wait_CMD_complete( emmcdev->config.InterruptSleep, 7 ) & EMMC_OK) == EMMC_NG )
    {
       printf("ERROR: CMD 7 didn't complete\n");
       return EMMC_STATUS_ERROR;
    }
    else
    {   
        status = EMMC_HOSTIF->emmc_host_resp_01;
        
#if DEBUG_EMMC_CMD
        printf(" CMD 7 resp(Status): 0x%08X\n", status);
#endif
        
        return status;
    }
    
}

#if EMMC_DRV_FUTURE
uint32_t emmc_CMD7_C0ba_acR1_DESELECT_CARD_ALL_stby_tans( emmcdev_t *emmcdev )
{
    uint32_t status;

    // &h071A0000&
    // CMD7
    // R1 (Index check  On, CRC check  On, 48bit) 
    // : while deselecting from Stand-By State to Transfer State
    status = emmc_CMD7_c0ba_acR1R1b_DESELECT_CARD_ALL( 0x071A0000, emmcdev );

    return status;

}

uint32_t emmc_CMD7_C0ba_acR1b_DESELECT_CARD_ALL_dis_prg( emmcdev_t *emmcdev )
{
    uint32_t status;

    // &h071B0000&
    // CMD7
    // R1b(Index check  On, CRC check  On, 48bit/busy_check) 
    // : while selecting from Disconnected State to Programming State.
    status = emmc_CMD7_c0ba_acR1R1b_DESELECT_CARD_ALL( 0x071B0000, emmcdev );

    return status;

}

uint32_t emmc_CMD7_c0ba_acR1R1b_DESELECT_CARD_ALL( uint32_t arg_cmd_mode, emmcdev_t *emmcdev )
{
    uint32_t status;
    
#if DEBUG_EMMC_CMD
    if( arg_cmd_mode == 0x071A0000 )
        printf("Sending CMD7_DESELECT_CARD (ac, R1)\n");
    if( arg_cmd_mode == 0x071B0000 )
        printf("Sending CMD7_DESELECT_CARD (ac, R1b)\n");
#endif
    
    emmc_Clear_HostIntStatus( SDIO_HOST_INT_CLR_ALL );
    
    // Setup Registers
    EMMC_HOSTIF->emmc_host_argument = 0;
    EMMC_HOSTIF->emmc_host_cmd_mode = arg_cmd_mode;
    // CMD7
    // &h071A0000& : R1 (Index check  On, CRC check  On, 48bit) : while selecting from Stand-By State to Transfer State
    // &h071B0000& : R1b(Index check  On, CRC check  On, 48bit/busy_check) : while selecting from Disconnected State to Programming State.

    if( (emmc_Wait_CMD_complete( emmcdev->config.InterruptSleep, 7 ) & EMMC_OK) == EMMC_NG )
    {
       printf("ERROR: CMD 7 didn't complete\n");
       return EMMC_STATUS_ERROR;
    }
    else
    {   
        status = EMMC_HOSTIF->emmc_host_resp_01;
        
#if DEBUG_EMMC_CMD
        printf(" CMD 7 resp(Status): 0x%08X\n", status);
#endif
        
        return status;
    }
    
}
#endif


uint32_t emmc_CMD8_C0ba_adtcR1_SEND_EXT_CSD( emmcdev_t *emmcdev )
{
    uint32_t status;

#if DEBUG_EMMC_CMD
    printf("Sending CMD8_SEND_EXT_CSD (adtc, R1) : No use SDMA\n");
#endif
    
    emmc_Clear_HostIntStatus( SDIO_HOST_INT_CLR_ALL );
    
    // Setup Registers
    //EMMC_HOSTIF->emmc_host_sdma  = 0x80040000;
    EMMC_HOSTIF->emmc_host_block =   0 << EMMC_HOSTIF_BLOCK_TRANSFER_BLOCK_COUNT_SHIFT |        // 0 ~ 64K, don't care on single block
                                    0 << EMMC_HOSTIF_BLOCK_TRANSFER_BLOCK_SIZE_MSB_SHIFT |     // 1:4K block transfer
                                    0 << EMMC_HOSTIF_BLOCK_HOST_BUFFER_SIZE_SHIFT |            // 0 ~    7 = 4K/8K/16K ~ 128K/256K/512K
                                  512 << EMMC_HOSTIF_BLOCK_TRANSFER_BLOCK_SIZE_SHIFT;          // 0 ~ 2047, ( 8/16/ 32 ~  256/ 512/1024 )
    EMMC_HOSTIF->emmc_host_argument = 0;
    EMMC_HOSTIF->emmc_host_cmd_mode =    8 << EMMC_HOSTIF_CMD_MODE_CMD_INDEX_SHIFT |            // 0~63
                                            0 << EMMC_HOSTIF_CMD_MODE_CMD_TYPE_SHIFT |             // 0:Normal, 1:Suspend, 2:Resume, 3:Abort
                                            1 << EMMC_HOSTIF_CMD_MODE_DATA_PRESENT_SHIFT |         // 0:No, 1:Yes (use data line to capture data)
                                            1 << EMMC_HOSTIF_CMD_MODE_CMD_INDEX_CHECK_SHIFT |      // 0:Off(R2/3/4), 1:On(R1/1b/5)
                                            1 << EMMC_HOSTIF_CMD_MODE_CMD_CRC_CHECK_SHIFT |        // 0:Off(R3/4), 1:On(R1/1b/2/5)
                                            2 << EMMC_HOSTIF_CMD_MODE_RESPONSE_TYPE_SHIFT |        // 0:No-Resp, 1:136bit(R2), 2:48bit(R1/3/4/5), 3:48bit-busy(R1b)
#if( ARASAN_IP_VER > ARASAN_IP_V10_7A )     // ARASAN_IP_V10_9
                                            0 << EMMC_HOSTIF_CMD_MODE_reserved_for_eco3_SHIFT |    // 0:Command Completion Disable, 1:Enable
#else
                                            0 << &EMMC_HOSTIF->emmc_host_cmd_mode_CMD_COMP_ATA_SHIFT |         // 0:Command Completion Disable, 1:Enable
#endif
                                            0 << EMMC_HOSTIF_CMD_MODE_MULTI_BLOCK_SHIFT |          // 0:single, 1:multiple
                                            1 << EMMC_HOSTIF_CMD_MODE_TRANFER_WRITE_SHIFT |        // 0:Write(Host-to-Card), 1:Read(Card-to-Host)
#if( ARASAN_IP_VER < ARASAN_IP_V10_7A )     // ARASAN_IP_V9_98
                                            0 << &EMMC_HOSTIF->emmc_host_cmd_mode_AUTO_CMD12_ENA_SHIFT |         // 0:No-Auto, 1:AutoCMD12
#else
                                            0 << EMMC_HOSTIF_CMD_MODE_AUTO_CMD_ENA_SHIFT |         // 0:No-Auto, 1:AutoCMD12, 2:AutoCMD23
#endif
                                            0 << EMMC_HOSTIF_CMD_MODE_BLOCK_COUNT_ENABLE_SHIFT |   // 0:Off, 1:On, don't care on single block
                                            0 << EMMC_HOSTIF_CMD_MODE_DMA_ENABLE_SHIFT;            // 0:Off, 1:On
    // R1 (Index check  On, CRC check  On, 48bit)

    // Wait for Command Completion.
    if( (emmc_Wait_CMD_complete( emmcdev->config.InterruptSleep, 8 ) & EMMC_OK) == EMMC_NG )
    {
       printf("ERROR: CMD 8 didn't complete\n");
       return EMMC_STATUS_ERROR;
    }
    else
    {
        status = EMMC_HOSTIF->emmc_host_resp_01;

#if DEBUG_EMMC_CMD
        printf(" CMD 8 resp(Status): 0x%08X (Before Transfer)\n", status);
#endif
    }

#if 1
    // Wait for Buffer Read Interrupt.
    //emmc_Clear_HostIntStatus( SDIO_HOST_INT_CLR_ALL );
    if( (emmc_Wait_BufferReadInt( emmcdev->config.InterruptSleep, 8 ) & EMMC_OK) == EMMC_NG )
    {
        printf("ERROR: CMD 8 buffer read int\n");
        return EMMC_STATUS_ERROR;
    }
#endif

    // Read ExtCSD.
    emmc_Get_ExtCSD( emmcdev );

    // Wait for Transfer Completion.
    //emmc_Clear_HostIntStatus( SDIO_HOST_INT_CLR_ALL );
    if( (emmc_Wait_Xfer_complete( emmcdev->config.InterruptSleep, 8 ) & EMMC_OK) == EMMC_NG )
    {
        printf("ERROR: CMD 8 read transfer\n");
        return EMMC_STATUS_ERROR;
    }

    status = EMMC_HOSTIF->emmc_host_resp_01;
#if DEBUG_EMMC_CMD
    printf(" CMD 8 resp(Status): 0x%08X (After Transfer)\n", status);
#endif
    return status;

}


uint32_t emmc_CMD9_C0ba_acR2_SEND_CSD( emmcdev_t *emmcdev )
{
    
#if DEBUG_EMMC_CMD
    printf("Sending CMD9_SEND_CSD (bcr, R2)\n");
#endif
    
    emmc_Clear_HostIntStatus( SDIO_HOST_INT_CLR_ALL );
    
    // Setup Registers
    EMMC_HOSTIF->emmc_host_argument = emmcdev->RCA;
    EMMC_HOSTIF->emmc_host_cmd_mode = 0x09090000;
    // CMD2, R2 (Index check Off, CRC check  On, 136bit) = 0x00090000 : CMD 2/9/10

    if( (emmc_Wait_CMD_complete( emmcdev->config.InterruptSleep, 9 ) & EMMC_OK) == EMMC_NG )
    {
        printf("ERROR: CMD 9 didn't complete\n");
        return EMMC_STATUS_ERROR;
    }
    else
    {   
        emmcdev->CSD.RESP67 = EMMC_HOSTIF->emmc_host_resp_67;
        emmcdev->CSD.RESP45 = EMMC_HOSTIF->emmc_host_resp_45;
        emmcdev->CSD.RESP23 = EMMC_HOSTIF->emmc_host_resp_23;
        emmcdev->CSD.RESP01 = EMMC_HOSTIF->emmc_host_resp_01;
                                                      // ( regval, regidx, width, offset )
        emmcdev->CSD.CSD_STRUCTURE      = (uint8_t)emmc_GetResponse07_1( emmcdev->CSD.RESP67, 3,  2, 126);
        emmcdev->CSD.SPEC_VERS          = (uint8_t)emmc_GetResponse07_1( emmcdev->CSD.RESP67, 3,  4, 122);
        emmcdev->CSD.TAAC               = (uint8_t)emmc_GetResponse07_1( emmcdev->CSD.RESP67, 3,  8, 112);
        emmcdev->CSD.NSAC               = (uint8_t)emmc_GetResponse07_1( emmcdev->CSD.RESP67, 3,  8, 104);
        emmcdev->CSD.TRAN_SPEED         = (uint8_t)emmc_GetResponse07_1( emmcdev->CSD.RESP45, 2,  8,  96);
        emmcdev->CSD.CCC                = (uint16_t)emmc_GetResponse07_1( emmcdev->CSD.RESP45, 2, 12,  84);
        emmcdev->CSD.READ_BL_LEN        = (uint8_t)emmc_GetResponse07_1( emmcdev->CSD.RESP45, 2,  4,  80);
        emmcdev->CSD.READ_BL_PARTIAL    = (uint8_t)emmc_GetResponse07_1( emmcdev->CSD.RESP45, 2,  1,  79);
        emmcdev->CSD.WRITE_BLK_MISALIGN = (uint8_t)emmc_GetResponse07_1( emmcdev->CSD.RESP45, 2,  1,  78);
        emmcdev->CSD.READ_BLK_MISALIGN  = (uint8_t)emmc_GetResponse07_1( emmcdev->CSD.RESP45, 2,  1,  77);
        emmcdev->CSD.DSR_IMP            = (uint8_t)emmc_GetResponse07_1( emmcdev->CSD.RESP45, 2,  1,  76);
        emmcdev->CSD.C_SIZE             = (uint16_t)emmc_GetResponse07_2(emmcdev->CSD.RESP45, emmcdev->CSD.RESP23, 1, 12,  62);  // ( reg1val, reg0val, reg0idx, width, offset )
        emmcdev->CSD.VDD_R_CURR_MIN     = (uint8_t)emmc_GetResponse07_1( emmcdev->CSD.RESP23, 1,  3,  59);
        emmcdev->CSD.VDD_R_CURR_MAX     = (uint8_t)emmc_GetResponse07_1( emmcdev->CSD.RESP23, 1,  3,  56);
        emmcdev->CSD.VDD_W_CURR_MIN     = (uint8_t)emmc_GetResponse07_1( emmcdev->CSD.RESP23, 1,  3,  53);
        emmcdev->CSD.VDD_W_CURR_MAX     = (uint8_t)emmc_GetResponse07_1( emmcdev->CSD.RESP23, 1,  3,  50);
        emmcdev->CSD.C_SIZE_MULT        = (uint8_t)emmc_GetResponse07_1( emmcdev->CSD.RESP23, 1,  3,  47);
        emmcdev->CSD.ERASE_GRP_SIZE     = (uint8_t)emmc_GetResponse07_1( emmcdev->CSD.RESP23, 1,  5,  42);
        emmcdev->CSD.ERASE_GRP_MULT     = (uint8_t)emmc_GetResponse07_2(emmcdev->CSD.RESP23, emmcdev->CSD.RESP01, 0,  5,  37);
        emmcdev->CSD.WP_GRP_SIZE        = (uint8_t)emmc_GetResponse07_1( emmcdev->CSD.RESP01, 0,  5,  32);
        emmcdev->CSD.WP_GRP_ENABLE      = (uint8_t)emmc_GetResponse07_1( emmcdev->CSD.RESP01, 0,  1,  31);
        emmcdev->CSD.DEFAULT_ECC        = (uint8_t)emmc_GetResponse07_1( emmcdev->CSD.RESP01, 0,  2,  29);
        emmcdev->CSD.R2W_FACTOR         = (uint8_t)emmc_GetResponse07_1( emmcdev->CSD.RESP01, 0,  3,  26);
        emmcdev->CSD.WRITE_BL_LEN       = (uint8_t)emmc_GetResponse07_1( emmcdev->CSD.RESP01, 0,  4,  22);
        emmcdev->CSD.WRITE_BL_PARTIAL   = (uint8_t)emmc_GetResponse07_1( emmcdev->CSD.RESP01, 0,  1,  21);
        emmcdev->CSD.CONTENT_PROT_APP   = (uint8_t)emmc_GetResponse07_1( emmcdev->CSD.RESP01, 0,  1,  16);
        emmcdev->CSD.FILE_FORMAT_GRP    = (uint8_t)emmc_GetResponse07_1( emmcdev->CSD.RESP01, 0,  1,  15);
        emmcdev->CSD.COPY               = (uint8_t)emmc_GetResponse07_1( emmcdev->CSD.RESP01, 0,  1,  14);
        emmcdev->CSD.PERM_WRITE_PROTECT = (uint8_t)emmc_GetResponse07_1( emmcdev->CSD.RESP01, 0,  1,  13);
        emmcdev->CSD.TMP_WRITE_PROTECT  = (uint8_t)emmc_GetResponse07_1( emmcdev->CSD.RESP01, 0,  1,  12);
        emmcdev->CSD.FILE_FORMAT        = (uint8_t)emmc_GetResponse07_1( emmcdev->CSD.RESP01, 0,  2,  10);
        emmcdev->CSD.ECC                = (uint8_t)emmc_GetResponse07_1( emmcdev->CSD.RESP01, 0,  2,   8);

#if DEBUG_EMMC_INIT
        printf(" CMD 9 response_67 CID[127:104]: 0x%08X\n", emmcdev->CSD.RESP67);
        printf(" CMD 9 response_45 CID[103: 72]: 0x%08X\n", emmcdev->CSD.RESP45);
        printf(" CMD 9 response_23 CID[ 71: 40]: 0x%08X\n", emmcdev->CSD.RESP23);
        printf(" CMD 9 response_01 CID[ 39:  8]: 0x%08X\n", emmcdev->CSD.RESP01);
        emmc_Print_CSD( emmcdev );
#endif
        
        return EMMC_STATUS_OK;
    }

}


uint32_t emmc_CMD10_C0ba_acR2_SEND_CID( emmcdev_t *emmcdev )
{
    uint32_t resp01, resp23, resp45, resp67;
    
#if DEBUG_EMMC_CMD
    printf("Sending CMD10_SEND_CID (bcr, R2)");
#endif
    
    emmc_Clear_HostIntStatus( SDIO_HOST_INT_CLR_ALL );
    
    // Setup Registers
    EMMC_HOSTIF->emmc_host_argument = 0x0000;
    EMMC_HOSTIF->emmc_host_cmd_mode = 0x0A090000;
    // CMD2, R2 (Index check Off, CRC check  On, 136bit) = 0x00090000 : CMD 2/9/10

    if( (emmc_Wait_CMD_complete( emmcdev->config.InterruptSleep, 10 ) & EMMC_OK) == EMMC_NG )
    {
        printf("ERROR: CMD 10 didn't complete\n");
        return EMMC_STATUS_ERROR;
    }
    else
    {   
        resp67 = EMMC_HOSTIF->emmc_host_resp_67;
        resp45 = EMMC_HOSTIF->emmc_host_resp_45;
        resp23 = EMMC_HOSTIF->emmc_host_resp_23;
        resp01 = EMMC_HOSTIF->emmc_host_resp_01;
        emmcdev->CID.MID = (uint8_t)(resp67 >> 16);
        emmcdev->CID.CBX = (uint8_t)(resp67 >>  8);
        emmcdev->CID.OID = (uint8_t)(resp67);
        emmcdev->CID.PNM[0] = (uint8_t)(resp45 >> 24);
        emmcdev->CID.PNM[1] = (uint8_t)(resp45 >> 16);
        emmcdev->CID.PNM[2] = (uint8_t)(resp45 >>  8);
        emmcdev->CID.PNM[3] = (uint8_t)(resp45 >>  0);
        emmcdev->CID.PNM[4] = (uint8_t)(resp23 >> 24);
        emmcdev->CID.PNM[5] = (uint8_t)(resp23 >> 16);
        emmcdev->CID.PRV = (uint8_t)(resp23 >>  8);
        emmcdev->CID.PSN = (resp01 >> 8) +  ((resp23 & 0x00FF) << 24);
        emmcdev->CID.MDT = (uint8_t)(resp01);

#if DEBUG_EMMC_CONFIG
        printf(" CMD 2 response_67 CID[127:104]: 0x%08X\n", resp67);
        printf(" CMD 2 response_45 CID[103: 72]: 0x%08X\n", resp45);
        printf(" CMD 2 response_23 CID[ 71: 40]: 0x%08X\n", resp23);
        printf(" CMD 2 response_01 CID[ 39:  8]: 0x%08X\n", resp01);
        emmc_Print_CID( emmcdev );
#endif            
        return EMMC_STATUS_OK;
    }

}


#if EMMC_DRV_FUTURE
uint32_t emmc_CMD12_C0ba_acR1_STOP_TRANSMISSION_Read( emmcdev_t *emmcdev )
{
    uint32_t arg, status;

    if( emmcdev->config.HPI_On == EMMC_ON )
        arg = emmcdev->RCA + emmcdev->config.HPI_On;
    else
        arg = emmcdev->RCA;
    
#if DEBUG_EMMC_CMD
    printf("Sending CMD12_STOP_TRANSMISSION_Read(ac, R1) : HPI = %d, RCA = 0x%08X\n", emmcdev->config.HPI_On, arg = emmcdev->RCA);
    printf(" - RCA in CMD12 is used only if HPI bit is set.\n");
#endif
    
    emmc_Clear_HostIntStatus( SDIO_HOST_INT_CLR_ALL );
    
    // Setup Registers
    EMMC_HOSTIF->emmc_host_argument = arg;
    EMMC_HOSTIF->emmc_host_cmd_mode = 0x0C1A0000;
    // CMD12
    // &h0C1A0000& : R1(Index check  On, CRC check  On, 48bit)

    if( (emmc_Wait_CMD_complete( emmcdev->config.InterruptSleep, 12 ) & EMMC_OK) == EMMC_NG )
    {
        printf("ERROR: CMD 12 didn't complete\n");
        return EMMC_STATUS_ERROR;
    }
    else
    {
        status = EMMC_HOSTIF->emmc_host_resp_01;

#if DEBUG_EMMC_CMD
        printf(" CMD 12 Read resp(Status): 0x%08X\n", status);
#endif
        return status;
    }

}


uint32_t emmc_CMD12_C0ba_acR1b_STOP_TRANSMISSION_Write( emmcdev_t *emmcdev )
{
    uint32_t arg, status;

    if( emmcdev->config.HPI_On == EMMC_ON )
        arg = emmcdev->RCA + emmcdev->config.HPI_On;
    else
        arg = emmcdev->RCA;
    
#if DEBUG_EMMC_CMD
    printf("Sending emmc_CMD12_C0ba_acR1b_STOP_TRANSMISSION_Write(ac, R1b) : HPI = %d, RCA = 0x%08X\n", emmcdev->config.HPI_On, arg = emmcdev->RCA);
    printf(" - RCA in CMD12 is used only if HPI bit is set.\n");
#endif
    
    emmc_Clear_HostIntStatus( SDIO_HOST_INT_CLR_ALL );
    
    // Setup Registers
    EMMC_HOSTIF->emmc_host_argument = arg;
    EMMC_HOSTIF->emmc_host_cmd_mode = 0x0C1B0000;
    // CMD12
    // &h0C1A0000& : R1b(Index check  On, CRC check  On, 48bit/busy_check)
    
    if( (emmc_Wait_CMD_complete( emmcdev->config.InterruptSleep, 12 ) & EMMC_OK) == EMMC_NG )
    {
        printf("ERROR: CMD 12 didn't complete\n");
        return EMMC_STATUS_ERROR;
    }
    else
    {
        status = EMMC_HOSTIF->emmc_host_resp_01;

        while( status & CST_ERROR_MASK )
        {
            status = emmc_CMD13_C0ba_acR1_SEND_STATUS( emmcdev );
        }

#if DEBUG_EMMC_CMD
        printf(" CMD 12 Write resp(Status): 0x%08X\n", status);
#endif
        return status;
    }

}
#endif


uint32_t emmc_CMD13_C0ba_acR1_SEND_STATUS( emmcdev_t *emmcdev )
{
    uint32_t arg, status;

    if( emmcdev->config.HPI_On == EMMC_ON )
        arg = emmcdev->RCA + emmcdev->config.HPI_On;
    else
        arg = emmcdev->RCA;
    
#if DEBUG_EMMC_CMD
    printf("Sending CMD13_SEND_STATUS (ac, R1) : 0x%08X (RCA), %d (HPI)\n", emmcdev->RCA, emmcdev->config.HPI_On);
#endif
    
    emmc_Clear_HostIntStatus( SDIO_HOST_INT_CLR_ALL );
    
    // Setup Registers
    EMMC_HOSTIF->emmc_host_argument = arg;
    EMMC_HOSTIF->emmc_host_cmd_mode = 0x0D1A0000;
    // CMD13, R1 (Index check  On, CRC check  On, 48bit)
    
    if( (emmc_Wait_CMD_complete( emmcdev->config.InterruptSleep, 13 ) & EMMC_OK) == EMMC_NG )
    {
        printf("ERROR: CMD 13 didn't complete\n");
        status = EMMC_HOSTIF->emmc_host_resp_01;
	emmcdev->CST = status;
#if DEBUG_EMMC_CONFIG
        printf(" CMD 13 resp(Status): 0x%08X\n", status);
        printf(" ----------------------------------------\n");
        printf(" CST_ADDRESS_OUT_OF_RANGE [    31] = %d\n", (emmcdev->CST & CST_ADDRESS_OUT_OF_RANGE_MASK) >> CST_ADDRESS_OUT_OF_RANGE_SHIFT );
        printf(" CST_ADDRESS_MISALIGN     [    30] = %d\n", (emmcdev->CST & CST_ADDRESS_MISALIGN_MASK    ) >> CST_ADDRESS_MISALIGN_SHIFT     );
        printf(" CST_BLOCK_LEN_ERROR      [    29] = %d\n", (emmcdev->CST & CST_BLOCK_LEN_ERROR_MASK     ) >> CST_BLOCK_LEN_ERROR_SHIFT      );
        printf(" CST_ERASE_SEQ_ERROR      [    28] = %d\n", (emmcdev->CST & CST_ERASE_SEQ_ERROR_MASK     ) >> CST_ERASE_SEQ_ERROR_SHIFT      );
        printf(" CST_ERASE_PARAM          [    27] = %d\n", (emmcdev->CST & CST_ERASE_PARAM_MASK         ) >> CST_ERASE_PARAM_SHIFT          );
        printf(" CST_WP_VIOLATION         [    26] = %d\n", (emmcdev->CST & CST_WP_VIOLATION_MASK        ) >> CST_WP_VIOLATION_SHIFT         );
        printf(" CST_DEVICE_IS_LOCKED     [    25] = %d\n", (emmcdev->CST & CST_DEVICE_IS_LOCKED_MASK    ) >> CST_DEVICE_IS_LOCKED_SHIFT     );
        printf(" CST_LOCK_UNLOCK_FAILED   [    24] = %d\n", (emmcdev->CST & CST_LOCK_UNLOCK_FAILED_MASK  ) >> CST_LOCK_UNLOCK_FAILED_SHIFT   );
        printf(" CST_COM_CRC_ERROR        [    23] = %d\n", (emmcdev->CST & CST_COM_CRC_ERROR_MASK       ) >> CST_COM_CRC_ERROR_SHIFT        );
        printf(" CST_ILLEGAL_COMMAND      [    22] = %d\n", (emmcdev->CST & CST_ILLEGAL_COMMAND_MASK     ) >> CST_ILLEGAL_COMMAND_SHIFT      );
        printf(" CST_DEVICE_ECC_FAILED    [    21] = %d\n", (emmcdev->CST & CST_DEVICE_ECC_FAILED_MASK   ) >> CST_DEVICE_ECC_FAILED_SHIFT    );
        printf(" CST_CC_ERROR             [    20] = %d\n", (emmcdev->CST & CST_CC_ERROR_MASK            ) >> CST_CC_ERROR_SHIFT             );
        printf(" CST_ERROR                [    19] = %d\n", (emmcdev->CST & CST_ERROR_MASK               ) >> CST_ERROR_SHIFT                );
        printf(" CST_CID_CSD_OVERWRITE    [    16] = %d\n", (emmcdev->CST & CST_CID_CSD_OVERWRITE_MASK   ) >> CST_CID_CSD_OVERWRITE_SHIFT    );
        printf(" CST_WP_ERASE_SKIP        [    15] = %d\n", (emmcdev->CST & CST_WP_ERASE_SKIP_MASK       ) >> CST_WP_ERASE_SKIP_SHIFT        );
        printf(" CST_ERASE_RESET          [    13] = %d\n", (emmcdev->CST & CST_ERASE_RESET_MASK         ) >> CST_ERASE_RESET_SHIFT          );
        printf(" CST_CURRENT_STATE        [12 : 9] = %d\n", (emmcdev->CST & CST_CURRENT_STATE_MASK       ) >> CST_CURRENT_STATE_SHIFT        );
        printf("  ( 0:Idle  1:Ready  2:Ident  3:Stby  4:Tran  5:Data )\n");
        printf("  ( 6:Rcv   7:Prg    8:Dis    9:Btst  10:Slp )\n");
        printf(" CST_READY_FOR_DATA       [     8] = %d\n", (emmcdev->CST & CST_READY_FOR_DATA_MASK      ) >> CST_READY_FOR_DATA_SHIFT       );
        printf(" CST_SWITCH_ERROR         [     7] = %d\n", (emmcdev->CST & CST_SWITCH_ERROR_MASK        ) >> CST_SWITCH_ERROR_SHIFT         );
        printf(" CST_EXCEPTION_EVENT      [     6] = %d\n", (emmcdev->CST & CST_EXCEPTION_EVENT_MASK     ) >> CST_EXCEPTION_EVENT_SHIFT      );
        printf(" CST_APP_CMD              [     5] = %d\n", (emmcdev->CST & CST_APP_CMD_MASK             ) >> CST_APP_CMD_SHIFT              );
        printf("\n");
#endif
        return EMMC_STATUS_ERROR;
    }
    else
    {
        status = EMMC_HOSTIF->emmc_host_resp_01;
	emmcdev->CST = status;
#if DEBUG_EMMC_CONFIG
        printf(" CMD 13 resp(Status): 0x%08X\n", status);
        printf(" ----------------------------------------\n");
        printf(" CST_ADDRESS_OUT_OF_RANGE [    31] = %d\n", (emmcdev->CST & CST_ADDRESS_OUT_OF_RANGE_MASK) >> CST_ADDRESS_OUT_OF_RANGE_SHIFT );
        printf(" CST_ADDRESS_MISALIGN     [    30] = %d\n", (emmcdev->CST & CST_ADDRESS_MISALIGN_MASK    ) >> CST_ADDRESS_MISALIGN_SHIFT     );
        printf(" CST_BLOCK_LEN_ERROR      [    29] = %d\n", (emmcdev->CST & CST_BLOCK_LEN_ERROR_MASK     ) >> CST_BLOCK_LEN_ERROR_SHIFT      );
        printf(" CST_ERASE_SEQ_ERROR      [    28] = %d\n", (emmcdev->CST & CST_ERASE_SEQ_ERROR_MASK     ) >> CST_ERASE_SEQ_ERROR_SHIFT      );
        printf(" CST_ERASE_PARAM          [    27] = %d\n", (emmcdev->CST & CST_ERASE_PARAM_MASK         ) >> CST_ERASE_PARAM_SHIFT          );
        printf(" CST_WP_VIOLATION         [    26] = %d\n", (emmcdev->CST & CST_WP_VIOLATION_MASK        ) >> CST_WP_VIOLATION_SHIFT         );
        printf(" CST_DEVICE_IS_LOCKED     [    25] = %d\n", (emmcdev->CST & CST_DEVICE_IS_LOCKED_MASK    ) >> CST_DEVICE_IS_LOCKED_SHIFT     );
        printf(" CST_LOCK_UNLOCK_FAILED   [    24] = %d\n", (emmcdev->CST & CST_LOCK_UNLOCK_FAILED_MASK  ) >> CST_LOCK_UNLOCK_FAILED_SHIFT   );
        printf(" CST_COM_CRC_ERROR        [    23] = %d\n", (emmcdev->CST & CST_COM_CRC_ERROR_MASK       ) >> CST_COM_CRC_ERROR_SHIFT        );
        printf(" CST_ILLEGAL_COMMAND      [    22] = %d\n", (emmcdev->CST & CST_ILLEGAL_COMMAND_MASK     ) >> CST_ILLEGAL_COMMAND_SHIFT      );
        printf(" CST_DEVICE_ECC_FAILED    [    21] = %d\n", (emmcdev->CST & CST_DEVICE_ECC_FAILED_MASK   ) >> CST_DEVICE_ECC_FAILED_SHIFT    );
        printf(" CST_CC_ERROR             [    20] = %d\n", (emmcdev->CST & CST_CC_ERROR_MASK            ) >> CST_CC_ERROR_SHIFT             );
        printf(" CST_ERROR                [    19] = %d\n", (emmcdev->CST & CST_ERROR_MASK               ) >> CST_ERROR_SHIFT                );
        printf(" CST_CID_CSD_OVERWRITE    [    16] = %d\n", (emmcdev->CST & CST_CID_CSD_OVERWRITE_MASK   ) >> CST_CID_CSD_OVERWRITE_SHIFT    );
        printf(" CST_WP_ERASE_SKIP        [    15] = %d\n", (emmcdev->CST & CST_WP_ERASE_SKIP_MASK       ) >> CST_WP_ERASE_SKIP_SHIFT        );
        printf(" CST_ERASE_RESET          [    13] = %d\n", (emmcdev->CST & CST_ERASE_RESET_MASK         ) >> CST_ERASE_RESET_SHIFT          );
        printf(" CST_CURRENT_STATE        [12 : 9] = %d\n", (emmcdev->CST & CST_CURRENT_STATE_MASK       ) >> CST_CURRENT_STATE_SHIFT        );
        printf("  ( 0:Idle  1:Ready  2:Ident  3:Stby  4:Tran  5:Data )\n");
        printf("  ( 6:Rcv   7:Prg    8:Dis    9:Btst  10:Slp )\n");
        printf(" CST_READY_FOR_DATA       [     8] = %d\n", (emmcdev->CST & CST_READY_FOR_DATA_MASK      ) >> CST_READY_FOR_DATA_SHIFT       );
        printf(" CST_SWITCH_ERROR         [     7] = %d\n", (emmcdev->CST & CST_SWITCH_ERROR_MASK        ) >> CST_SWITCH_ERROR_SHIFT         );
        printf(" CST_EXCEPTION_EVENT      [     6] = %d\n", (emmcdev->CST & CST_EXCEPTION_EVENT_MASK     ) >> CST_EXCEPTION_EVENT_SHIFT      );
        printf(" CST_APP_CMD              [     5] = %d\n", (emmcdev->CST & CST_APP_CMD_MASK             ) >> CST_APP_CMD_SHIFT              );
        printf("\n");
#endif
        return status;
    }

}


#if EMMC_DRV_FUTURE
uint32_t emmc_CMD14_C0ba_adtcR1_BUSTEST_R( emmcdev_t *emmcdev )
{
    uint32_t status;

#if DEBUG_EMMC_CMD
    printf("Sending CMD14_BUSTEST_R (adtc, R1)\n");
#endif
    
    emmc_Clear_HostIntStatus( SDIO_HOST_INT_CLR_ALL );
    
    // Setup Registers
    EMMC_HOSTIF->emmc_host_argument = 0;
    EMMC_HOSTIF->emmc_host_cmd_mode = 0x0E1A0000;
    // CMD14, R1 (Index check  On, CRC check  On, 48bit)
    
    if( (emmc_Wait_CMD_complete( emmcdev->config.InterruptSleep, 14 ) & EMMC_OK) == EMMC_NG )
    {
        printf("ERROR: CMD 14 didn't complete\n");
        return EMMC_STATUS_ERROR;
    }
    else
    {
        status = EMMC_HOSTIF->emmc_host_resp_01;

#if DEBUG_EMMC_CMD
        printf(" CMD 14 resp(Status): 0x%08X\n", status);
#endif
        return status;
    }

}


uint32_t emmc_CMD15_C0ba_ac_GO_INACTIVE_STATE( uint32_t arg_RCA, emmcdev_t *emmcdev )
{
    uint32_t status;

#if DEBUG_EMMC_CMD
    printf("Sending CMD15_GO_INACTIVE_STATE( (ac, -) : 0x%08X (RCA)\n", arg_RCA);
#endif
    
    emmc_Clear_HostIntStatus( SDIO_HOST_INT_CLR_ALL );
    
    // Setup Registers
    EMMC_HOSTIF->emmc_host_argument = arg_RCA;
    EMMC_HOSTIF->emmc_host_cmd_mode = 0x0F000000;
    // CMD15, R0 (Index check  Off, CRC check  Off)
    
    if( (emmc_Wait_CMD_complete( emmcdev->config.InterruptSleep, 15 ) & EMMC_OK) == EMMC_NG )
    {
        printf("ERROR: CMD 15 didn't complete\n");
        return EMMC_STATUS_ERROR;
    }
    else
    {
        status = EMMC_HOSTIF->emmc_host_resp_01;

#if DEBUG_EMMC_CMD
        printf(" CMD 15 : Success!!!\n");
#endif
        return status;
    }

}


uint32_t emmc_CMD19_C0ba_adtcR1_BUSTEST_W( emmcdev_t *emmcdev )
{
    uint32_t status;

#if DEBUG_EMMC_CMD
    printf("Sending CMD19_BUSTEST_W (adtc, R1)\n");
#endif
    
    emmc_Clear_HostIntStatus( SDIO_HOST_INT_CLR_ALL );
    
    // Setup Registers
    EMMC_HOSTIF->emmc_host_argument = 0;
    EMMC_HOSTIF->emmc_host_cmd_mode = 0x131A0000;
    // CMD19, R1 (Index check  On, CRC check  On, 48bit)
    
    if( (emmc_Wait_CMD_complete( emmcdev->config.InterruptSleep, 19 ) & EMMC_OK) == EMMC_NG )
    {
        printf("ERROR: CMD 19 didn't complete\n");
        return EMMC_STATUS_ERROR;
    }
    else
    {
        status = EMMC_HOSTIF->emmc_host_resp_01;

#if DEBUG_EMMC_CMD
        printf(" CMD 19 resp(Status): 0x%08X\n", status);
        printf("\n");
#endif
        return status;
    }

}
#endif
//-------------------------------------------------


//-------------------------------------------------
// Class 2 Bock Read Commands
uint32_t emmc_CMD16_C2br_acR1_SET_BLOCKLEN( uint32_t arg_BlockLength, emmcdev_t *emmcdev )
{
    uint32_t reg, status;

#if DEBUG_EMMC_CMD
    printf("Sending CMD16_SET_BLOCKLEN( (ac, R1) : 0x%08X (RCA)\n", arg_BlockLength);
#endif
    
    emmc_Clear_HostIntStatus( SDIO_HOST_INT_CLR_ALL );
    
    // Setup Registers
    reg = EMMC_HOSTIF->emmc_host_block;
    EMMC_HOSTIF->emmc_host_block = (reg & (~EMMC_HOSTIF_BLOCK_TRANSFER_BLOCK_SIZE_MASK)) + arg_BlockLength;
    EMMC_HOSTIF->emmc_host_argument = arg_BlockLength;
    EMMC_HOSTIF->emmc_host_cmd_mode = 0x101A0000;
    // CMD16, R1 (Index check  On, CRC check  On, 48bit)
    
    if( (emmc_Wait_CMD_complete( emmcdev->config.InterruptSleep, 16 ) & EMMC_OK) == EMMC_NG )
    {
        printf("ERROR: CMD 16 didn't complete\n");
        return EMMC_STATUS_ERROR;
    }
    else
    {
        status = EMMC_HOSTIF->emmc_host_resp_01;

#if DEBUG_EMMC_CMD
        printf(" CMD 16 resp(Status): 0x%08X\n", status);
        printf("\n");
#endif
        return status;
    }

}


uint32_t emmc_CMD17_C2br_adtcR1_READ_SINGLE_BLOCK( uint32_t emmc_Addr, uint32_t DMA_Addr, uint32_t BlockLength, uint32_t *read_data, emmcdev_t *emmcdev )
{
    uint32_t status;
    
#if DEBUG_EMMC_CMD
    printf("Sending CMD17_READ_SINGLE_BLOCK (adtc, R1) : SDMA %d\n", emmcdev->config.SDMA_On);
    printf("  emmc_Addr = 0x%X, DMA_Addr = 0x%X\n", emmc_Addr, DMA_Addr);
#endif
    
    emmc_Clear_HostIntStatus( SDIO_HOST_INT_CLR_ALL );
    
    // Setup Registers
    EMMC_HOSTIF->emmc_host_sdma  = DMA_Addr;
    EMMC_HOSTIF->emmc_host_block =   1 << EMMC_HOSTIF_BLOCK_TRANSFER_BLOCK_COUNT_SHIFT |        // 0 ~ 64K, don't care on single block
                                    0 << EMMC_HOSTIF_BLOCK_TRANSFER_BLOCK_SIZE_MSB_SHIFT |     // 1:4K block transfer
                                    0 << EMMC_HOSTIF_BLOCK_HOST_BUFFER_SIZE_SHIFT |            // 0 ~    7 = 4K/8K/16K ~ 128K/256K/512K
                          BlockLength << EMMC_HOSTIF_BLOCK_TRANSFER_BLOCK_SIZE_SHIFT;          // 0 ~ 2047,   8/16/ 32 ~  256/ 512/1024
    EMMC_HOSTIF->emmc_host_argument = emmc_Addr;
    if( emmcdev->config.SDMA_On == EMMC_ON )
    {
        EMMC_HOSTIF->emmc_host_cmd_mode =   17 << EMMC_HOSTIF_CMD_MODE_CMD_INDEX_SHIFT |            // 0~63
                                            0 << EMMC_HOSTIF_CMD_MODE_CMD_TYPE_SHIFT |             // 0:Normal, 1:Suspend, 2:Resume, 3:Abort
                                            1 << EMMC_HOSTIF_CMD_MODE_DATA_PRESENT_SHIFT |         // 0:No, 1:Yes (use data line to capture data)
                                            1 << EMMC_HOSTIF_CMD_MODE_CMD_INDEX_CHECK_SHIFT |      // 0:Off(R2/3/4), 1:On(R1/1b/5)
                                            1 << EMMC_HOSTIF_CMD_MODE_CMD_CRC_CHECK_SHIFT |        // 0:Off(R3/4), 1:On(R1/1b/2/5)
                                            2 << EMMC_HOSTIF_CMD_MODE_RESPONSE_TYPE_SHIFT |        // 0:No-Resp, 1:136bit(R2), 2:48bit(R1/3/4/5), 3:48bit-busy(R1b)
#if( ARASAN_IP_VER > ARASAN_IP_V10_7A )     // ARASAN_IP_V10_9
                                            0 << EMMC_HOSTIF_CMD_MODE_reserved_for_eco3_SHIFT |    // 0:Command Completion Disable, 1:Enable
#else
                                            0 << &EMMC_HOSTIF->emmc_host_cmd_mode_CMD_COMP_ATA_SHIFT |         // 0:Command Completion Disable, 1:Enable
#endif
                                            0 << EMMC_HOSTIF_CMD_MODE_MULTI_BLOCK_SHIFT |          // 0:single, 1:multiple
                                            1 << EMMC_HOSTIF_CMD_MODE_TRANFER_WRITE_SHIFT |        // 0:Write(Host-to-Card), 1:Read(Card-to-Host)
#if( ARASAN_IP_VER < ARASAN_IP_V10_7A )     // ARASAN_IP_V9_98
                                            0 << &EMMC_HOSTIF->emmc_host_cmd_mode_AUTO_CMD12_ENA_SHIFT |         // 0:No-Auto, 1:AutoCMD12
#else
                                            0 << EMMC_HOSTIF_CMD_MODE_AUTO_CMD_ENA_SHIFT |         // 0:No-Auto, 1:AutoCMD12, 2:AutoCMD23
#endif
                                            0 << EMMC_HOSTIF_CMD_MODE_BLOCK_COUNT_ENABLE_SHIFT |   // 0:Off, 1:On, don't care on single block
                                            1 << EMMC_HOSTIF_CMD_MODE_DMA_ENABLE_SHIFT;            // 0:Off, 1:On
    }
    else
    {
        EMMC_HOSTIF->emmc_host_cmd_mode =   17 << EMMC_HOSTIF_CMD_MODE_CMD_INDEX_SHIFT |            // 0~63
                                            0 << EMMC_HOSTIF_CMD_MODE_CMD_TYPE_SHIFT |             // 0:Normal, 1:Suspend, 2:Resume, 3:Abort
                                            1 << EMMC_HOSTIF_CMD_MODE_DATA_PRESENT_SHIFT |         // 0:No, 1:Yes (use data line to capture data)
                                            1 << EMMC_HOSTIF_CMD_MODE_CMD_INDEX_CHECK_SHIFT |      // 0:Off(R2/3/4), 1:On(R1/1b/5)
                                            1 << EMMC_HOSTIF_CMD_MODE_CMD_CRC_CHECK_SHIFT |        // 0:Off(R3/4), 1:On(R1/1b/2/5)
                                            2 << EMMC_HOSTIF_CMD_MODE_RESPONSE_TYPE_SHIFT |        // 0:No-Resp, 1:136bit(R2), 2:48bit(R1/3/4/5), 3:48bit-busy(R1b)
#if( ARASAN_IP_VER > ARASAN_IP_V10_7A )     // ARASAN_IP_V10_9
                                            0 << EMMC_HOSTIF_CMD_MODE_reserved_for_eco3_SHIFT |    // 0:Command Completion Disable, 1:Enable
#else
                                            0 << &EMMC_HOSTIF->emmc_host_cmd_mode_CMD_COMP_ATA_SHIFT |         // 0:Command Completion Disable, 1:Enable
#endif
                                            0 << EMMC_HOSTIF_CMD_MODE_MULTI_BLOCK_SHIFT |          // 0:single, 1:multiple
                                            1 << EMMC_HOSTIF_CMD_MODE_TRANFER_WRITE_SHIFT |        // 0:Write(Host-to-Card), 1:Read(Card-to-Host)
#if( ARASAN_IP_VER < ARASAN_IP_V10_7A )     // ARASAN_IP_V9_98
                                            0 << &EMMC_HOSTIF->emmc_host_cmd_mode_AUTO_CMD12_ENA_SHIFT |         // 0:No-Auto, 1:AutoCMD12
#else
                                            0 << EMMC_HOSTIF_CMD_MODE_AUTO_CMD_ENA_SHIFT |         // 0:No-Auto, 1:AutoCMD12, 2:AutoCMD23
#endif
                                            0 << EMMC_HOSTIF_CMD_MODE_BLOCK_COUNT_ENABLE_SHIFT |   // 0:Off, 1:On, don't care on single block
                                            0 << EMMC_HOSTIF_CMD_MODE_DMA_ENABLE_SHIFT;            // 0:Off, 1:On
    }
    
    // Wait for Command Completion.
    if( (emmc_Wait_CMD_complete( emmcdev->config.InterruptSleep, 17 ) & EMMC_OK) == EMMC_NG )
    {
        printf("ERROR: CMD 17 didn't complete\n");
        return EMMC_STATUS_ERROR;
    }
    else
    {
        status = EMMC_HOSTIF->emmc_host_resp_01;

#if DEBUG_EMMC_CMD
        printf(" CMD 17 resp(Status): 0x%08X (Before Transfer)\n", status);
#endif
    }
    
#if DEBUG_BUFFDATA
    if( emmcdev->config.SDMA_On == EMMC_OFF )
    {
        // Wait for Buffer Read Interrupt.
        if( (emmc_Wait_BufferReadInt( emmcdev->config.InterruptSleep, 17 ) & EMMC_OK) == EMMC_NG )
        {
            printf("ERROR: CMD 17 buffer read int\n");
            return EMMC_STATUS_ERROR;
        }
        
        // Read data.
        emmc_Read_Blocks( read_data, 1, BlockLength/4, emmcdev );
    }
#endif
    
    // Wait for Transfer Completion.
    if( (emmc_Wait_Xfer_complete( emmcdev->config.InterruptSleep, 17 ) & EMMC_OK) == EMMC_NG )
    {
        printf("ERROR: CMD 17 read transfer\n");
        return EMMC_STATUS_ERROR;
    }
    
    status = EMMC_HOSTIF->emmc_host_resp_01;
#if DEBUG_EMMC_CMD
    printf(" CMD 17 resp(Status): 0x%08X (After Transfer)\n", status);
#endif
    return status;

}


uint32_t emmc_CMD18_C2br_adtcR1_READ_MULTIPLE_BLOCK( uint32_t emmc_Addr, uint32_t DMA_Addr, uint32_t nBlocks, uint32_t BlockLength, uint32_t *read_data, emmcdev_t *emmcdev )
{
    uint32_t status;

#if DEBUG_EMMC_CMD
    printf("Sending CMD18_READ_MULTIPLE_BLOCK (adtc, R1) : SDMA %d\n", emmcdev->config.SDMA_On);
    printf("  emmc_Addr = 0x%X, DMA_Addr = 0x%X, nBlocks = 0x%X\n", emmc_Addr, DMA_Addr, nBlocks);
#endif

    emmc_Clear_HostIntStatus( SDIO_HOST_INT_CLR_ALL );
    
    // Setup Registers
    EMMC_HOSTIF->emmc_host_sdma  = DMA_Addr;
    EMMC_HOSTIF->emmc_host_block = nBlocks << EMMC_HOSTIF_BLOCK_TRANSFER_BLOCK_COUNT_SHIFT |        // 0 ~ 64K, don't care on single block
                                        0 << EMMC_HOSTIF_BLOCK_TRANSFER_BLOCK_SIZE_MSB_SHIFT |     // 1:4K block transfer
                            HOST_SDMA_BUFF_BOUNDARY << EMMC_HOSTIF_BLOCK_HOST_BUFFER_SIZE_SHIFT |            // 0 ~    7 = 4K/8K/16K ~ 128K/256K/512K
                              BlockLength << EMMC_HOSTIF_BLOCK_TRANSFER_BLOCK_SIZE_SHIFT;          // 0 ~ 2047,   8/16/ 32 ~  256/ 512/1024
    EMMC_HOSTIF->emmc_host_argument = emmc_Addr;
    if( emmcdev->config.SDMA_On == EMMC_ON )
    {
        EMMC_HOSTIF->emmc_host_cmd_mode =    18 << EMMC_HOSTIF_CMD_MODE_CMD_INDEX_SHIFT |            // 0~63
                                             0 << EMMC_HOSTIF_CMD_MODE_CMD_TYPE_SHIFT |             // 0:Normal, 1:Suspend, 2:Resume, 3:Abort
                                             1 << EMMC_HOSTIF_CMD_MODE_DATA_PRESENT_SHIFT |         // 0:No, 1:Yes (use data line to capture data)
                                             1 << EMMC_HOSTIF_CMD_MODE_CMD_INDEX_CHECK_SHIFT |      // 0:Off(R2/3/4), 1:On(R1/1b/5)
                                             1 << EMMC_HOSTIF_CMD_MODE_CMD_CRC_CHECK_SHIFT |        // 0:Off(R3/4), 1:On(R1/1b/2/5)
                                             2 << EMMC_HOSTIF_CMD_MODE_RESPONSE_TYPE_SHIFT |        // 0:No-Resp, 1:136bit(R2), 2:48bit(R1/3/4/5), 3:48bit-busy(R1b)
#if( ARASAN_IP_VER > ARASAN_IP_V10_7A )     // ARASAN_IP_V10_9
                                             0 << EMMC_HOSTIF_CMD_MODE_reserved_for_eco3_SHIFT |    // 0:Command Completion Disable, 1:Enable
#else
                                             0 << &EMMC_HOSTIF->emmc_host_cmd_mode_CMD_COMP_ATA_SHIFT |         // 0:Command Completion Disable, 1:Enable
#endif
                                             1 << EMMC_HOSTIF_CMD_MODE_MULTI_BLOCK_SHIFT |          // 0:single, 1:multiple
                                             1 << EMMC_HOSTIF_CMD_MODE_TRANFER_WRITE_SHIFT |        // 0:Write(Host-to-Card), 1:Read(Card-to-Host)
#if( ARASAN_IP_VER < ARASAN_IP_V10_7A )     // ARASAN_IP_V9_98
                                             1 << &EMMC_HOSTIF->emmc_host_cmd_mode_AUTO_CMD12_ENA_SHIFT |         // 0:No-Auto, 1:AutoCMD12
#else
                                             1 << EMMC_HOSTIF_CMD_MODE_AUTO_CMD_ENA_SHIFT |         // 0:No-Auto, 1:AutoCMD12, 2:AutoCMD23
#endif
                                             1 << EMMC_HOSTIF_CMD_MODE_BLOCK_COUNT_ENABLE_SHIFT |   // 0:Off, 1:On, don't care on single block
                                             1 << EMMC_HOSTIF_CMD_MODE_DMA_ENABLE_SHIFT;            // 0:Off, 1:On
    }
    else
    {
        EMMC_HOSTIF->emmc_host_cmd_mode =    18 << EMMC_HOSTIF_CMD_MODE_CMD_INDEX_SHIFT |            // 0~63
                                             0 << EMMC_HOSTIF_CMD_MODE_CMD_TYPE_SHIFT |             // 0:Normal, 1:Suspend, 2:Resume, 3:Abort
                                             1 << EMMC_HOSTIF_CMD_MODE_DATA_PRESENT_SHIFT |         // 0:No, 1:Yes (use data line to capture data)
                                             1 << EMMC_HOSTIF_CMD_MODE_CMD_INDEX_CHECK_SHIFT |      // 0:Off(R2/3/4), 1:On(R1/1b/5)
                                             1 << EMMC_HOSTIF_CMD_MODE_CMD_CRC_CHECK_SHIFT |        // 0:Off(R3/4), 1:On(R1/1b/2/5)
                                             2 << EMMC_HOSTIF_CMD_MODE_RESPONSE_TYPE_SHIFT |        // 0:No-Resp, 1:136bit(R2), 2:48bit(R1/3/4/5), 3:48bit-busy(R1b)
#if( ARASAN_IP_VER > ARASAN_IP_V10_7A )     // ARASAN_IP_V10_9
                                             0 << EMMC_HOSTIF_CMD_MODE_reserved_for_eco3_SHIFT |    // 0:Command Completion Disable, 1:Enable
#else
                                             0 << &EMMC_HOSTIF->emmc_host_cmd_mode_CMD_COMP_ATA_SHIFT |         // 0:Command Completion Disable, 1:Enable
#endif
                                             1 << EMMC_HOSTIF_CMD_MODE_MULTI_BLOCK_SHIFT |          // 0:single, 1:multiple
                                             1 << EMMC_HOSTIF_CMD_MODE_TRANFER_WRITE_SHIFT |        // 0:Write(Host-to-Card), 1:Read(Card-to-Host)
#if( ARASAN_IP_VER < ARASAN_IP_V10_7A )     // ARASAN_IP_V9_98
                                             1 << &EMMC_HOSTIF->emmc_host_cmd_mode_AUTO_CMD12_ENA_SHIFT |         // 0:No-Auto, 1:AutoCMD12
#else
                                             1 << EMMC_HOSTIF_CMD_MODE_AUTO_CMD_ENA_SHIFT |         // 0:No-Auto, 1:AutoCMD12, 2:AutoCMD23
#endif
                                             1 << EMMC_HOSTIF_CMD_MODE_BLOCK_COUNT_ENABLE_SHIFT |   // 0:Off, 1:On, don't care on single block
                                             0 << EMMC_HOSTIF_CMD_MODE_DMA_ENABLE_SHIFT;            // 0:Off, 1:On
    }
    
    // Wait for Command Completion.
    if( (emmc_Wait_CMD_complete( emmcdev->config.InterruptSleep, 18 ) & EMMC_OK) == EMMC_NG )
    {
        printf("ERROR: CMD 18 didn't complete\n");
        return EMMC_STATUS_ERROR;
    }
    else
    {
        status = EMMC_HOSTIF->emmc_host_resp_01;

#if DEBUG_EMMC_CMD
        printf(" CMD 18 resp(Status): 0x%08X (Before Transfer)\n", status);
#endif
    }
    
#if DEBUG_BUFFDATA
    if( emmcdev->config.SDMA_On == EMMC_OFF )
    {
        // Wait for Buffer Read Interrupt.
        if( (emmc_Wait_BufferReadInt( emmcdev->config.InterruptSleep, 18 ) & EMMC_OK) == EMMC_NG )
        {
            printf("ERROR: CMD 18 buffer read int\n");
            return EMMC_STATUS_ERROR;
        }
        
        // Read data.
        emmc_Read_Blocks( read_data, nBlocks, BlockLength/4, emmcdev );
    }
#endif
    
    // Wait for Transfer Completion.
    if( (emmc_Wait_Xfer_complete( emmcdev->config.InterruptSleep, 18 ) & EMMC_OK) == EMMC_NG )
    {
        printf("ERROR: CMD 18 read transfer\n");
        return EMMC_STATUS_ERROR;
    }

    status = EMMC_HOSTIF->emmc_host_resp_01;
#if DEBUG_EMMC_CMD
    printf(" CMD 18 resp(Status): 0x%08X (After Transfer)\n", status);
#endif
    return status;
    
}
//-------------------------------------------------



//-------------------------------------------------
// Class 4 Block Write Commands
#if EMMC_DRV_FUTURE
uint32_t emmc_CMD23_C4bw_acR1_SET_BLOCK_COUNT( uint32_t NumBlocks, uint32_t ReliableWriteReq, uint32_t TagReq, uint32_t ContextID, uint32_t ForcedPrg, emmcdev_t *emmcdev )
{
    uint32_t arg, status;
    
    arg = 0;
    arg = (ReliableWriteReq & 0x01) << 31;
    arg = arg + (NumBlocks & 0xFFFF);
    
#if DEBUG_EMMC_CMD
    if( emmcdev->ExtCSD.EXT_CSD_REV < JESD84_V45 )
        printf("Sending CMD23_C4bw_SET_BLOCK_COUNT(ac, R1) : NumBlocks = %d, ReliableWriteReq = %d\n", NumBlocks, ReliableWriteReq);
#endif
    
    emmc_Clear_HostIntStatus( SDIO_HOST_INT_CLR_ALL );
    
    // Setup Registers
    EMMC_HOSTIF->emmc_host_argument = arg;
    EMMC_HOSTIF->emmc_host_cmd_mode = 0x171A0000;
    // CMD23
    // &h171A0000& : R1(Index check  On, CRC check  On, 48bit)

    if( (emmc_Wait_CMD_complete( emmcdev->config.InterruptSleep, 23 ) & EMMC_OK) == EMMC_NG )
    {
        printf("ERROR: CMD 23 didn't complete\n");
        return EMMC_STATUS_ERROR;
    }
    else
    {
        status = EMMC_HOSTIF->emmc_host_resp_01;

#if DEBUG_EMMC_CMD
        printf(" CMD 23 Read resp(Status): 0x%08X\n", status);
#endif
        return status;
    }

}


uint32_t emmc_CMD23_C4bw_acR1_SET_BLOCK_COUNT_packed( uint32_t arg_NumBlocks, emmcdev_t *emmcdev )
{
    return EMMC_STATUS_ERROR;

}
#endif


uint32_t emmc_CMD24_C4bw_adtcR1_WRITE_BLOCK( uint32_t emmc_Addr, uint32_t DMA_Addr, uint32_t BlockLength, uint32_t *write_data, emmcdev_t *emmcdev )
{
    uint32_t status;

#if DEBUG_EMMC_CMD
    printf("Sending CMD24_WRITE_BLOCK (adtc, R1) : SDMA %d\n", emmcdev->config.SDMA_On);
    printf("  emmc_Addr = 0x%X, DMA_Addr = 0x%X, BlockLength\n", emmc_Addr, DMA_Addr, BlockLength);
#endif
            
    emmc_Clear_HostIntStatus( SDIO_HOST_INT_CLR_ALL );
    
    // Setup Registers
    EMMC_HOSTIF->emmc_host_sdma  = DMA_Addr;
    EMMC_HOSTIF->emmc_host_block =   1 << EMMC_HOSTIF_BLOCK_TRANSFER_BLOCK_COUNT_SHIFT |        // 0 ~ 64K, don't care on single block
                                    0 << EMMC_HOSTIF_BLOCK_TRANSFER_BLOCK_SIZE_MSB_SHIFT |     // 1:4K block transfer
                                    0 << EMMC_HOSTIF_BLOCK_HOST_BUFFER_SIZE_SHIFT |            // 0 ~    7 = 4K/8K/16K ~ 128K/256K/512K
                          BlockLength << EMMC_HOSTIF_BLOCK_TRANSFER_BLOCK_SIZE_SHIFT;          // 0 ~ 2047,   8/16/ 32 ~  256/ 512/1024
    EMMC_HOSTIF->emmc_host_argument = emmc_Addr;
    if( emmcdev->config.SDMA_On == EMMC_ON )
    {
        EMMC_HOSTIF->emmc_host_cmd_mode =   24 << EMMC_HOSTIF_CMD_MODE_CMD_INDEX_SHIFT |            // 0~63
                                            0 << EMMC_HOSTIF_CMD_MODE_CMD_TYPE_SHIFT |             // 0:Normal, 1:Suspend, 2:Resume, 3:Abort
                                            1 << EMMC_HOSTIF_CMD_MODE_DATA_PRESENT_SHIFT |         // 0:No, 1:Yes (use data line to capture data)
                                            1 << EMMC_HOSTIF_CMD_MODE_CMD_INDEX_CHECK_SHIFT |      // 0:Off(R2/3/4), 1:On(R1/1b/5)
                                            1 << EMMC_HOSTIF_CMD_MODE_CMD_CRC_CHECK_SHIFT |        // 0:Off(R3/4), 1:On(R1/1b/2/5)
                                            2 << EMMC_HOSTIF_CMD_MODE_RESPONSE_TYPE_SHIFT |        // 0:No-Resp, 1:136bit(R2), 2:48bit(R1/3/4/5), 3:48bit-busy(R1b)
#if( ARASAN_IP_VER > ARASAN_IP_V10_7A )     // ARASAN_IP_V10_9
                                            0 << EMMC_HOSTIF_CMD_MODE_reserved_for_eco3_SHIFT |    // 0:Command Completion Disable, 1:Enable
#else
                                            0 << &EMMC_HOSTIF->emmc_host_cmd_mode_CMD_COMP_ATA_SHIFT |         // 0:Command Completion Disable, 1:Enable
#endif
                                            0 << EMMC_HOSTIF_CMD_MODE_MULTI_BLOCK_SHIFT |          // 0:single, 1:multiple
                                            0 << EMMC_HOSTIF_CMD_MODE_TRANFER_WRITE_SHIFT |        // 0:Write(Host-to-Card), 1:Read(Card-to-Host)
#if( ARASAN_IP_VER < ARASAN_IP_V10_7A )     // ARASAN_IP_V9_98
                                            0 << &EMMC_HOSTIF->emmc_host_cmd_mode_AUTO_CMD12_ENA_SHIFT |         // 0:No-Auto, 1:AutoCMD12
#else
                                            0 << EMMC_HOSTIF_CMD_MODE_AUTO_CMD_ENA_SHIFT |         // 0:No-Auto, 1:AutoCMD12, 2:AutoCMD23
#endif
                                            0 << EMMC_HOSTIF_CMD_MODE_BLOCK_COUNT_ENABLE_SHIFT |   // 0:Off, 1:On, don't care on single block
                                            1 << EMMC_HOSTIF_CMD_MODE_DMA_ENABLE_SHIFT;            // 0:Off, 1:On
    }
    else
    {
        EMMC_HOSTIF->emmc_host_cmd_mode =   24 << EMMC_HOSTIF_CMD_MODE_CMD_INDEX_SHIFT |            // 0~63
                                            0 << EMMC_HOSTIF_CMD_MODE_CMD_TYPE_SHIFT |             // 0:Normal, 1:Suspend, 2:Resume, 3:Abort
                                            1 << EMMC_HOSTIF_CMD_MODE_DATA_PRESENT_SHIFT |         // 0:No, 1:Yes (use data line to capture data)
                                            1 << EMMC_HOSTIF_CMD_MODE_CMD_INDEX_CHECK_SHIFT |      // 0:Off(R2/3/4), 1:On(R1/1b/5)
                                            1 << EMMC_HOSTIF_CMD_MODE_CMD_CRC_CHECK_SHIFT |        // 0:Off(R3/4), 1:On(R1/1b/2/5)
                                            2 << EMMC_HOSTIF_CMD_MODE_RESPONSE_TYPE_SHIFT |        // 0:No-Resp, 1:136bit(R2), 2:48bit(R1/3/4/5), 3:48bit-busy(R1b)
#if( ARASAN_IP_VER > ARASAN_IP_V10_7A )     // ARASAN_IP_V10_9
                                            0 << EMMC_HOSTIF_CMD_MODE_reserved_for_eco3_SHIFT |    // 0:Command Completion Disable, 1:Enable
#else
                                            0 << &EMMC_HOSTIF->emmc_host_cmd_mode_CMD_COMP_ATA_SHIFT |         // 0:Command Completion Disable, 1:Enable
#endif
                                            0 << EMMC_HOSTIF_CMD_MODE_MULTI_BLOCK_SHIFT |          // 0:single, 1:multiple
                                            0 << EMMC_HOSTIF_CMD_MODE_TRANFER_WRITE_SHIFT |        // 0:Write(Host-to-Card), 1:Read(Card-to-Host)
#if( ARASAN_IP_VER < ARASAN_IP_V10_7A )     // ARASAN_IP_V9_98
                                            0 << &EMMC_HOSTIF->emmc_host_cmd_mode_AUTO_CMD12_ENA_SHIFT |         // 0:No-Auto, 1:AutoCMD12
#else
                                            0 << EMMC_HOSTIF_CMD_MODE_AUTO_CMD_ENA_SHIFT |         // 0:No-Auto, 1:AutoCMD12, 2:AutoCMD23
#endif
                                            0 << EMMC_HOSTIF_CMD_MODE_BLOCK_COUNT_ENABLE_SHIFT |   // 0:Off, 1:On, don't care on single block
                                            0 << EMMC_HOSTIF_CMD_MODE_DMA_ENABLE_SHIFT;            // 0:Off, 1:On
    }
    
    // Wait for Command Completion.
    if( (emmc_Wait_CMD_complete( emmcdev->config.InterruptSleep, 24 ) & EMMC_OK) == EMMC_NG )
    {
        printf("ERROR: CMD 24 didn't complete\n");
        return EMMC_STATUS_ERROR;
    }
    else
    {
        status = EMMC_HOSTIF->emmc_host_resp_01;

#if DEBUG_EMMC_CMD
        printf(" CMD 24 resp(Status): 0x%08X (Before Transfer)\n", status);
#endif
    }
    
#if DEBUG_BUFFDATA
    if( emmcdev->config.SDMA_On == EMMC_OFF )
    {
        // Wait for Buffer Write Interrupt.
        if( (emmc_Wait_BufferWriteInt( emmcdev->config.InterruptSleep, 24 ) & EMMC_OK) == EMMC_NG )
        {
            printf("ERROR: CMD 24 buffer write int\n");
            return EMMC_STATUS_ERROR;
        }
        //printf(" Done : emmc_Wait_BufferWriteInt \n");

        // Write data.
        emmc_Write_Blocks( write_data, 1, BlockLength/4, emmcdev );
    }
#endif
    
    // Wait for Transfer Completion.
    if( (emmc_Wait_Xfer_complete( emmcdev->config.InterruptSleep, 24 ) & EMMC_OK) == EMMC_NG )
    {
        printf("ERROR: CMD 24 write transfer\n");
        return EMMC_STATUS_ERROR;
    }
    //printf(" Done : emmc_Wait_Xfer_complete \n");

    status = EMMC_HOSTIF->emmc_host_resp_01;
#if DEBUG_EMMC_CMD
    printf(" CMD 24 resp(Status): 0x%08X (After Transfer)\n", status);
#endif
    return status;

}


uint32_t emmc_CMD25_C4bw_adtcR1_WRITE_MULTIPLE_BLOCK( uint32_t emmc_Addr, uint32_t DMA_Addr, uint32_t nBlocks, uint32_t BlockLength, uint32_t *write_data, emmcdev_t *emmcdev )
{
    uint32_t status;

#if DEBUG_EMMC_CMD
    printf("Sending CMD25_WRITE_MULTIPLE_BLOCK (adtc, R1) : SDMA %d\n", emmcdev->config.SDMA_On);
    printf("  emmc_Addr = 0x%X, DMA_Addr = 0x%X, nBlocks = 0x%X\n", emmc_Addr, DMA_Addr, nBlocks);
#endif
    
    emmc_Clear_HostIntStatus( SDIO_HOST_INT_CLR_ALL );
    
    // Setup Registers
    EMMC_HOSTIF->emmc_host_sdma  = DMA_Addr;
    EMMC_HOSTIF->emmc_host_block = nBlocks << EMMC_HOSTIF_BLOCK_TRANSFER_BLOCK_COUNT_SHIFT |        // 0 ~ 64K, don't care on single block
                                        0 << EMMC_HOSTIF_BLOCK_TRANSFER_BLOCK_SIZE_MSB_SHIFT |     // 1:4K block transfer
                            HOST_SDMA_BUFF_BOUNDARY << EMMC_HOSTIF_BLOCK_HOST_BUFFER_SIZE_SHIFT |            // 0 ~    7 = 4K/8K/16K ~ 128K/256K/512K
                              BlockLength << EMMC_HOSTIF_BLOCK_TRANSFER_BLOCK_SIZE_SHIFT;          // 0 ~ 2047,   8/16/ 32 ~  256/ 512/1024
    EMMC_HOSTIF->emmc_host_argument = emmc_Addr;
    if( emmcdev->config.SDMA_On == EMMC_ON )
    {
        EMMC_HOSTIF->emmc_host_cmd_mode =   25 << EMMC_HOSTIF_CMD_MODE_CMD_INDEX_SHIFT |            // 0~63
                                            0 << EMMC_HOSTIF_CMD_MODE_CMD_TYPE_SHIFT |             // 0:Normal, 1:Suspend, 2:Resume, 3:Abort
                                            1 << EMMC_HOSTIF_CMD_MODE_DATA_PRESENT_SHIFT |         // 0:No, 1:Yes (use data line to capture data)
                                            1 << EMMC_HOSTIF_CMD_MODE_CMD_INDEX_CHECK_SHIFT |      // 0:Off(R2/3/4), 1:On(R1/1b/5)
                                            1 << EMMC_HOSTIF_CMD_MODE_CMD_CRC_CHECK_SHIFT |        // 0:Off(R3/4), 1:On(R1/1b/2/5)
                                            2 << EMMC_HOSTIF_CMD_MODE_RESPONSE_TYPE_SHIFT |        // 0:No-Resp, 1:136bit(R2), 2:48bit(R1/3/4/5), 3:48bit-busy(R1b)
#if( ARASAN_IP_VER > ARASAN_IP_V10_7A )     // ARASAN_IP_V10_9
                                            0 << EMMC_HOSTIF_CMD_MODE_reserved_for_eco3_SHIFT |    // 0:Command Completion Disable, 1:Enable
#else
                                            0 << &EMMC_HOSTIF->emmc_host_cmd_mode_CMD_COMP_ATA_SHIFT |         // 0:Command Completion Disable, 1:Enable
#endif
                                            1 << EMMC_HOSTIF_CMD_MODE_MULTI_BLOCK_SHIFT |          // 0:single, 1:multiple
                                            0 << EMMC_HOSTIF_CMD_MODE_TRANFER_WRITE_SHIFT |        // 0:Write(Host-to-Card), 1:Read(Card-to-Host)
#if( ARASAN_IP_VER < ARASAN_IP_V10_7A )     // ARASAN_IP_V9_98
                                            1 << &EMMC_HOSTIF->emmc_host_cmd_mode_AUTO_CMD12_ENA_SHIFT |         // 0:No-Auto, 1:AutoCMD12
#else
                                            1 << EMMC_HOSTIF_CMD_MODE_AUTO_CMD_ENA_SHIFT |         // 0:No-Auto, 1:AutoCMD12, 2:AutoCMD23
#endif
                                            1 << EMMC_HOSTIF_CMD_MODE_BLOCK_COUNT_ENABLE_SHIFT |   // 0:Off, 1:On, don't care on single block
                                            1 << EMMC_HOSTIF_CMD_MODE_DMA_ENABLE_SHIFT;            // 0:Off, 1:On
    }
    else
    {
        EMMC_HOSTIF->emmc_host_cmd_mode =   25 << EMMC_HOSTIF_CMD_MODE_CMD_INDEX_SHIFT |            // 0~63
                                            0 << EMMC_HOSTIF_CMD_MODE_CMD_TYPE_SHIFT |             // 0:Normal, 1:Suspend, 2:Resume, 3:Abort
                                            1 << EMMC_HOSTIF_CMD_MODE_DATA_PRESENT_SHIFT |         // 0:No, 1:Yes (use data line to capture data)
                                            1 << EMMC_HOSTIF_CMD_MODE_CMD_INDEX_CHECK_SHIFT |      // 0:Off(R2/3/4), 1:On(R1/1b/5)
                                            1 << EMMC_HOSTIF_CMD_MODE_CMD_CRC_CHECK_SHIFT |        // 0:Off(R3/4), 1:On(R1/1b/2/5)
                                            2 << EMMC_HOSTIF_CMD_MODE_RESPONSE_TYPE_SHIFT |        // 0:No-Resp, 1:136bit(R2), 2:48bit(R1/3/4/5), 3:48bit-busy(R1b)
#if( ARASAN_IP_VER > ARASAN_IP_V10_7A )     // ARASAN_IP_V10_9
                                            0 << EMMC_HOSTIF_CMD_MODE_reserved_for_eco3_SHIFT |    // 0:Command Completion Disable, 1:Enable
#else
                                            0 << &EMMC_HOSTIF->emmc_host_cmd_mode_CMD_COMP_ATA_SHIFT |         // 0:Command Completion Disable, 1:Enable
#endif
                                            1 << EMMC_HOSTIF_CMD_MODE_MULTI_BLOCK_SHIFT |          // 0:single, 1:multiple
                                            0 << EMMC_HOSTIF_CMD_MODE_TRANFER_WRITE_SHIFT |        // 0:Write(Host-to-Card), 1:Read(Card-to-Host)
#if( ARASAN_IP_VER < ARASAN_IP_V10_7A )     // ARASAN_IP_V9_98
                                            1 << &EMMC_HOSTIF->emmc_host_cmd_mode_AUTO_CMD12_ENA_SHIFT |         // 0:No-Auto, 1:AutoCMD12
#else
                                            1 << EMMC_HOSTIF_CMD_MODE_AUTO_CMD_ENA_SHIFT |         // 0:No-Auto, 1:AutoCMD12, 2:AutoCMD23
#endif
                                            1 << EMMC_HOSTIF_CMD_MODE_BLOCK_COUNT_ENABLE_SHIFT |   // 0:Off, 1:On, don't care on single block
                                            0 << EMMC_HOSTIF_CMD_MODE_DMA_ENABLE_SHIFT;            // 0:Off, 1:On
    }
    
    // Wait for Command Completion.
    if( (emmc_Wait_CMD_complete( emmcdev->config.InterruptSleep, 25 ) & EMMC_OK) == EMMC_NG )
    {
        printf("ERROR: CMD 25 didn't complete\n");
        return EMMC_STATUS_ERROR;
    }
    else
    {
        status = EMMC_HOSTIF->emmc_host_resp_01;

#if DEBUG_EMMC_CMD
        printf(" CMD 25 resp(Status): 0x%08X (Before Transfer)\n", status);
#endif
    }
    
#if DEBUG_BUFFDATA    
    if( emmcdev->config.SDMA_On == EMMC_OFF )
    {
        // Wait for Buffer Write Interrupt.
        if( (emmc_Wait_BufferWriteInt( emmcdev->config.InterruptSleep, 25 ) & EMMC_OK) == EMMC_NG )
        {
            printf("ERROR: CMD 25 buffer write int\n");
            return EMMC_STATUS_ERROR;
        }
        
        // Write data.
        emmc_Write_Blocks( write_data, nBlocks, BlockLength/4, emmcdev );
    }
#endif
    
    // Wait for Transfer Completion.
    if( (emmc_Wait_Xfer_complete( emmcdev->config.InterruptSleep, 25 ) & EMMC_OK) == EMMC_NG )
    {
        printf("ERROR: CMD 25 write transfer\n");
        return EMMC_STATUS_ERROR;
    }
    //printf("-----> After emmc_Wait_Xfer_complete\n");
    
    status = EMMC_HOSTIF->emmc_host_resp_01;
#if DEBUG_EMMC_CMD
    printf(" CMD 25 resp(Status): 0x%08X (After Transfer)\n", status);
#endif
    return status;

}


//-------------------------------------------------
// Class 5 Erase Commands
uint32_t emmc_CMD35_C5er_acR1_ERASE_GROUP_START( uint32_t arg_StartErAddr, emmcdev_t *emmcdev )
{
    uint32_t status;

#if DEBUG_EMMC_CMD
    printf("Sending CMD35_ERASE_GROUP_START (ac, R1) : 0x%08X [addr]\n", arg_StartErAddr);
#endif
    
    emmc_Clear_HostIntStatus( SDIO_HOST_INT_CLR_ALL );
    
    // Setup Registers
    EMMC_HOSTIF->emmc_host_argument = arg_StartErAddr;
    EMMC_HOSTIF->emmc_host_cmd_mode = 0x231A0000;
    // CMD35, R1 (Index check  On, CRC check  On, 48bit)
    
    if( (emmc_Wait_CMD_complete( emmcdev->config.InterruptSleep, 35 ) & EMMC_OK) == EMMC_NG )
    {
        printf("ERROR: CMD 35 didn't complete\n");
        return EMMC_STATUS_ERROR;
    }
    else
    {
        status = EMMC_HOSTIF->emmc_host_resp_01;

#if DEBUG_EMMC_CMD
        printf(" CMD 35 resp(Status): 0x%08X\n", status);
#endif
        return status;
    }

}
    
    
uint32_t emmc_CMD36_C5er_acR1_ERASE_GROUP_END( uint32_t arg_EndErAddr, emmcdev_t *emmcdev )
{
    uint32_t status;

#if DEBUG_EMMC_CMD
    printf("Sending CMD36_ERASE_GROUP_END (ac, R1) : 0x%08X [addr]\n", arg_EndErAddr);
#endif
    
    emmc_Clear_HostIntStatus( SDIO_HOST_INT_CLR_ALL );
    
    // Setup Registers
    EMMC_HOSTIF->emmc_host_argument = arg_EndErAddr;
    EMMC_HOSTIF->emmc_host_cmd_mode = 0x241A0000;
    // CMD36, R1 (Index check  On, CRC check  On, 48bit)

    if( (emmc_Wait_CMD_complete( emmcdev->config.InterruptSleep, 36 ) & EMMC_OK) == EMMC_NG )
    {
        printf("ERROR: CMD 36 didn't complete\n");
        return EMMC_STATUS_ERROR;
    }
    else
    {
        status = EMMC_HOSTIF->emmc_host_resp_01;

#if DEBUG_EMMC_CMD
        printf(" CMD 36 resp(Status): 0x%08X\n", status);
#endif
        return status;
    }

}
    
    
uint32_t emmc_CMD38_C5er_acR1b_ERASE( uint32_t TrimData, uint32_t IdentifyWriteBlock, emmcdev_t *emmcdev )
{
    uint32_t arg, status;

    arg = 0;
    arg = (TrimData & 0x01) << 1; //THIS IS incorrect TRIM is bit 0 of ARG
    arg = arg + (IdentifyWriteBlock & 0x01); //THIS is incorrect Discard is bit 1 and 0 of ARG

#if DEBUG_EMMC_CMD
    printf("Sending CMD38_ERASE (ac, R1b) : TrimData = %d, IdentifyWriteBlock = %d\n", TrimData, IdentifyWriteBlock);
#endif
    
    emmc_Clear_HostIntStatus( SDIO_HOST_INT_CLR_ALL );
    // Setup Registers
    EMMC_HOSTIF->emmc_host_argument = arg;
    EMMC_HOSTIF->emmc_host_cmd_mode = 0x261B0000;
    // CMD6
    // &h061B0000& : R1b(Index check  On, CRC check  On, 48bit/busy_check)

    if( (emmc_Wait_CMD_complete( emmcdev->config.InterruptSleep, 38 ) & EMMC_OK) == EMMC_NG )
    {
        printf("ERROR: CMD 38 didn't complete\n");
        return EMMC_STATUS_ERROR;
    }
    else
    {
        status = EMMC_HOSTIF->emmc_host_resp_01;
        
        while( status & CST_SWITCH_ERROR_MASK )
        {
            status = emmc_CMD13_C0ba_acR1_SEND_STATUS( emmcdev );
        }

#if DEBUG_EMMC_CMD
        printf(" CMD 38 ExtCSD resp(Status): 0x%08X\n", status);
#endif
        return status;
    }


}
//-------------------------------------------------
#if EMMC_DRV_FUTURE
uint32_t emmc_CMD27_C4bw_adtcR1_PROGRAM_CSD( uint32_t *write_data, emmcdev_t *emmcdev )
{
    //uint32_t write_data[4];    // 16Bytes = 4Bytes x 4
    uint32_t status;

#if DEBUG_EMMC_CMD
    printf("Sending CMD27_PROGRAM_CSD (adtc, R1)\n");
#endif
    
    emmc_Clear_HostIntStatus( SDIO_HOST_INT_CLR_ALL );
    
    // Setup Registers
    //EMMC_HOSTIF->emmc_host_sdma  = 0;
    EMMC_HOSTIF->emmc_host_block =   1 << EMMC_HOSTIF_BLOCK_TRANSFER_BLOCK_COUNT_SHIFT |        // 0 ~ 64K, don't care on single block
                                    0 << EMMC_HOSTIF_BLOCK_TRANSFER_BLOCK_SIZE_MSB_SHIFT |     // 1:4K block transfer
                        HOST_SDMA_BUFF_BOUNDARY << EMMC_HOSTIF_BLOCK_HOST_BUFFER_SIZE_SHIFT |            // 0 ~ 7 = 4K/8K/16K ~ 128K/256K/512K
                                   16 << EMMC_HOSTIF_BLOCK_TRANSFER_BLOCK_SIZE_SHIFT;          // 0~2047, 8/16/32 ~ 256/512/1024
    EMMC_HOSTIF->emmc_host_argument = 0;
    EMMC_HOSTIF->emmc_host_cmd_mode =   27 << EMMC_HOSTIF_CMD_MODE_CMD_INDEX_SHIFT |            // 0~63
                                        0 << EMMC_HOSTIF_CMD_MODE_CMD_TYPE_SHIFT |             // 0:Normal, 1:Suspend, 2:Resume, 3:Abort
                                        1 << EMMC_HOSTIF_CMD_MODE_DATA_PRESENT_SHIFT |         // 0:No, 1:Yes (use data line to capture data)
                                        1 << EMMC_HOSTIF_CMD_MODE_CMD_INDEX_CHECK_SHIFT |      // 0:Off(R2/3/4), 1:On(R1/1b/5)
                                        1 << EMMC_HOSTIF_CMD_MODE_CMD_CRC_CHECK_SHIFT |        // 0:Off(R3/4), 1:On(R1/1b/2/5)
                                        2 << EMMC_HOSTIF_CMD_MODE_RESPONSE_TYPE_SHIFT |        // 0:No-Resp, 1:136bit(R2), 2:48bit(R1/3/4/5), 3:48bit-busy(R1b)
#if( ARASAN_IP_VER > ARASAN_IP_V10_7A ) // ARASAN_IP_V10_9
                                        0 << EMMC_HOSTIF_CMD_MODE_reserved_for_eco3_SHIFT |    // 0:Command Completion Disable, 1:Enable
#else
                                        0 << &EMMC_HOSTIF->emmc_host_cmd_mode_CMD_COMP_ATA_SHIFT |         // 0:Command Completion Disable, 1:Enable
#endif
                                        0 << EMMC_HOSTIF_CMD_MODE_MULTI_BLOCK_SHIFT |          // 0:single, 1:multiple
                                        0 << EMMC_HOSTIF_CMD_MODE_TRANFER_WRITE_SHIFT |        // 0:Write(Host-to-Card), 1:Read(Card-to-Host)
#if( ARASAN_IP_VER < ARASAN_IP_V10_7A )     // ARASAN_IP_V9_98
                                        1 << &EMMC_HOSTIF->emmc_host_cmd_mode_AUTO_CMD12_ENA_SHIFT |         // 0:No-Auto, 1:AutoCMD12
#else
                                        1 << EMMC_HOSTIF_CMD_MODE_AUTO_CMD_ENA_SHIFT |         // 0:No-Auto, 1:AutoCMD12, 2:AutoCMD23
#endif
                                        0 << EMMC_HOSTIF_CMD_MODE_BLOCK_COUNT_ENABLE_SHIFT |   // 0:Off, 1:On, don't care on single block
                                        0 << EMMC_HOSTIF_CMD_MODE_DMA_ENABLE_SHIFT;            // 0:Off, 1:On
    
    // Wait for Command Completion.
    if( (emmc_Wait_CMD_complete( emmcdev->config.InterruptSleep, 27 ) & EMMC_OK) == EMMC_NG )
    {
        printf("ERROR: CMD 27 didn't complete\n");
        return EMMC_STATUS_ERROR;
    }
    else
    {
        status = EMMC_HOSTIF->emmc_host_resp_01;
        
#if DEBUG_EMMC_CMD
        printf(" CMD 27 resp(Status): 0x%08X (Before Transfer)\n", status);
#endif
    }
    //printf(" Done : emmc_Wait_CMD_complete \n");
    
    // Wait for Buffer Write Interrupt.
    if( (emmc_Wait_BufferWriteInt( emmcdev->config.InterruptSleep, 27 ) & EMMC_OK) == EMMC_NG )
    {
        printf("ERROR: CMD 27 buffer write int\n");
        return EMMC_STATUS_ERROR;
    }
    //printf(" Done : emmc_Wait_BufferWriteInt \n");
    
    // Write data.
    emmc_Write_Blocks( write_data, 1, 4, emmcdev );
        
    // Wait for Transfer Completion.
    if( (emmc_Wait_Xfer_complete( emmcdev->config.InterruptSleep, 27 ) & EMMC_OK) == EMMC_NG )
    {
        printf("ERROR: CMD 27 write transfer\n");
        return EMMC_STATUS_ERROR;
    }
    //printf(" Done : emmc_Wait_Xfer_complete \n");
    
    status = EMMC_HOSTIF->emmc_host_resp_01;
#if DEBUG_EMMC_CMD
    printf(" CMD 27 resp(Status): 0x%08X (After Transfer)\n", status);
#endif    
    return status;

}

//-------------------------------------------------






//-------------------------------------------------
// Class 6 Write Protection Commands
uint32_t emmc_CMD28_C6wp_acR1b_SET_WRITE_PROT( uint32_t arg_Addr, emmcdev_t *emmcdev )
{
    return EMMC_STATUS_ERROR;

}


uint32_t emmc_CMD29_C6wp_acR1b_CLR_WRITE_PROT( uint32_t arg_Addr, emmcdev_t *emmcdev )
{
    return EMMC_STATUS_ERROR;

}


uint32_t emmc_CMD30_C6wp_adtcR1_SEND_WRITE_PROT( uint32_t arg_WpAddr, emmcdev_t *emmcdev )
{
    return EMMC_STATUS_ERROR;

}


uint32_t emmc_CMD31_C6wp_adtcR1_SEND_WRITE_PROT_TYPE( uint32_t arg_WpAddr, emmcdev_t *emmcdev )
{
    return EMMC_STATUS_ERROR;

}
//-------------------------------------------------



//-------------------------------------------------
// Class 7 Lock Card Commands
uint32_t emmc_CMD42_C7lc_adtcR1_LOCK_UNLOCK( emmcdev_t *emmcdev )
{
    uint32_t status;

#if DEBUG_EMMC_CMD
    printf("Sending CMD42_READ_LOCK_UNLOCK (adtc, R1)\n");
#endif
    
    emmc_Clear_HostIntStatus( SDIO_HOST_INT_CLR_ALL );
    
    // Setup Registers
    //EMMC_HOSTIF->emmc_host_sdma  = DMA_Addr;
    EMMC_HOSTIF->emmc_host_block =       1 << EMMC_HOSTIF_BLOCK_TRANSFER_BLOCK_COUNT_SHIFT |        // 0 ~ 64K, don't care on single block
                                        0 << EMMC_HOSTIF_BLOCK_TRANSFER_BLOCK_SIZE_MSB_SHIFT |     // 1:4K block transfer
                                        0 << EMMC_HOSTIF_BLOCK_HOST_BUFFER_SIZE_SHIFT |            // 0 ~    7 = 4K/8K/16K ~ 128K/256K/512K
                                        1 << EMMC_HOSTIF_BLOCK_TRANSFER_BLOCK_SIZE_SHIFT;          // 0 ~ 2047,   8/16/ 32 ~  256/ 512/1024
    EMMC_HOSTIF->emmc_host_argument = 0;
    EMMC_HOSTIF->emmc_host_cmd_mode =   42 << EMMC_HOSTIF_CMD_MODE_CMD_INDEX_SHIFT |            // 0~63
                                        0 << EMMC_HOSTIF_CMD_MODE_CMD_TYPE_SHIFT |             // 0:Normal, 1:Suspend, 2:Resume, 3:Abort
                                        1 << EMMC_HOSTIF_CMD_MODE_DATA_PRESENT_SHIFT |         // 0:No, 1:Yes (use data line to capture data)
                                        1 << EMMC_HOSTIF_CMD_MODE_CMD_INDEX_CHECK_SHIFT |      // 0:Off(R2/3/4), 1:On(R1/1b/5)
                                        1 << EMMC_HOSTIF_CMD_MODE_CMD_CRC_CHECK_SHIFT |        // 0:Off(R3/4), 1:On(R1/1b/2/5)
                                        2 << EMMC_HOSTIF_CMD_MODE_RESPONSE_TYPE_SHIFT |        // 0:No-Resp, 1:136bit(R2), 2:48bit(R1/3/4/5), 3:48bit-busy(R1b)
#if( ARASAN_IP_VER > ARASAN_IP_V10_7A ) // ARASAN_IP_V10_9
                                        0 << EMMC_HOSTIF_CMD_MODE_reserved_for_eco3_SHIFT |    // 0:Command Completion Disable, 1:Enable
#else
                                        0 << &EMMC_HOSTIF->emmc_host_cmd_mode_CMD_COMP_ATA_SHIFT |         // 0:Command Completion Disable, 1:Enable
#endif
                                        0 << EMMC_HOSTIF_CMD_MODE_MULTI_BLOCK_SHIFT |          // 0:single, 1:multiple
                                        0 << EMMC_HOSTIF_CMD_MODE_TRANFER_WRITE_SHIFT |        // 0:Write(Host-to-Card), 1:Read(Card-to-Host)
#if( ARASAN_IP_VER < ARASAN_IP_V10_7A )     // ARASAN_IP_V9_98
                                        0 << &EMMC_HOSTIF->emmc_host_cmd_mode_AUTO_CMD12_ENA_SHIFT |         // 0:No-Auto, 1:AutoCMD12
#else
                                        0 << EMMC_HOSTIF_CMD_MODE_AUTO_CMD_ENA_SHIFT |         // 0:No-Auto, 1:AutoCMD12, 2:AutoCMD23
#endif
                                        1 << EMMC_HOSTIF_CMD_MODE_BLOCK_COUNT_ENABLE_SHIFT |   // 0:Off, 1:On, don't care on single block
                                        0 << EMMC_HOSTIF_CMD_MODE_DMA_ENABLE_SHIFT;            // 0:Off, 1:On
    
    if( (emmc_Wait_CMD_complete( emmcdev->config.InterruptSleep, 42 ) & EMMC_OK) == EMMC_NG )
    {
        printf("ERROR: CMD 42 didn't complete\n");
        return EMMC_STATUS_ERROR;
    }
    else
    {
        status = EMMC_HOSTIF->emmc_host_resp_01;

#if DEBUG_EMMC_CMD
        printf(" CMD 42 resp(Status): 0x%08X (Before Transfer)\n", status);
#endif
    }

    if( (emmc_Wait_BufferWriteInt( emmcdev->config.InterruptSleep, 42 ) & EMMC_OK) == EMMC_NG )
    {
        printf("ERROR: CMD 42 buffer write int\n");
        return EMMC_STATUS_ERROR;
    }

    // Write data
    EMMC_HOSTIF->emmc_host_buffdata = 0x0008;
    EMMC_HOSTIF->emmc_host_buffdata = 0x0000;
    cfe_sleep( 100 );

    // poll of DMA to complete
    if( (emmc_Wait_Xfer_complete( emmcdev->config.InterruptSleep, 42 ) & EMMC_OK) == EMMC_NG )
    {
        printf("ERROR: CMD 18 read transfer\n");
        return EMMC_STATUS_ERROR;
    }
    
    return status;
    

}
//-------------------------------------------------


//-------------------------------------------------
// Class 8 Application Specific Commands
uint32_t emmc_CMD55_C8as_acR1_APP_CMD( uint32_t arg_RCA, emmcdev_t *emmcdev )
{
    return EMMC_STATUS_ERROR;

}


uint32_t emmc_CMD56_C8as_adtcR1_GEN_CMD( uint32_t arg_RD_WR, emmcdev_t *emmcdev )
{
    return EMMC_STATUS_ERROR;

}
//-------------------------------------------------


//-------------------------------------------------
// Class 9 I/O Mode Commands
uint32_t emmc_CMD39_C9io_acR4_FAST_IO( emmcdev_t *emmcdev )
{
    return EMMC_STATUS_ERROR;

}


uint32_t emmc_CMD40_C9io_bcrR5_GO_IRQ_STATE( emmcdev_t *emmcdev )
{
    return EMMC_STATUS_ERROR;

}
#endif
//-------------------------------------------------
//===========================================================


//===========================================================
//-------------------------------------------------
// eMMC Sub Functions
// Enable interrupts.
void emmc_Enable_Host_Int( uint32_t int_status_env )
{
    EMMC_HOSTIF->emmc_host_int_signal_ena = int_status_env;
    EMMC_HOSTIF->emmc_host_int_status_ena = int_status_env;
}


void emmc_Clear_HostIntStatus( uint32_t IntMask )
{
    EMMC_HOSTIF->emmc_host_int_status = IntMask;
}


uint32_t emmc_Get_HostState( void )
{
    return EMMC_HOSTIF->emmc_host_state;
}


uint32_t emmc_Wait_CMD_complete( uint32_t sleep_time, uint8_t cmd_num )
{
    uint32_t timeout_cnt, time_tick;
    uint32_t reg_val=0, field_val=0;
    timeout_cnt = sleep_time;
    time_tick   = sleep_time;

    do
    {
        cfe_usleep( time_tick );
        reg_val = EMMC_HOSTIF->emmc_host_int_status;
        field_val = reg_val;
        GET_REG_FIELD(field_val, EMMC_HOSTIF_INT_STATUS_COMMAND_COMPLETE_INT);
        if( timeout_cnt < 10 )
        {
            timeout_cnt += time_tick;
        }   
        else if( timeout_cnt < 1000 )
        {
            time_tick = sleep_time * 100;
            timeout_cnt += time_tick;
        }
        else if( timeout_cnt < 100000 )
        {
            time_tick = sleep_time * 10000;
            timeout_cnt += time_tick;
        }
        else
        {
            time_tick = 100000;
            timeout_cnt += time_tick;
        }
    }while( (timeout_cnt < EMMC_TIMEOUT_CNT) && field_val == EMMC_OFF );
    
#if DEBUG_EMMC_HOST_INT
    if( timeout_cnt > 100000 )
    printf("[emmc_Wait_CMD_complete] CMD %d : HOST_INT_STATUS = 0x%X, COMMAND_COMPLETE_INT = %d (timeout_cnt=%d, sleep_time=%d)\n", 
        cmd_num, reg_val, field_val, timeout_cnt, sleep_time);
#endif
    if( timeout_cnt >= EMMC_TIMEOUT_CNT )
    {
        // Check CMD_TIMEOUT_ERR_INT
        reg_val = EMMC_HOSTIF->emmc_host_int_status;
        reg_val = field_val;
        GET_REG_FIELD(field_val, EMMC_HOSTIF_INT_STATUS_CMD_TIMEOUT_ERR_INT);
        printf("[emmc_Wait_CMD_complete:INT_STATUS] CMD %d : HOST_INT_STATUS = 0x%X, CMD_TIMEOUT_ERR_INT = %d\n", cmd_num, reg_val, field_val); 
        
        // Call Error_Recovery_Sequence
        
        return EMMC_NG;
    }
    else
    {
        emmc_Clear_HostIntStatus(EMMC_HOSTIF_INT_STATUS_COMMAND_COMPLETE_INT_MASK);
        return EMMC_OK;
    }
}


uint32_t emmc_Wait_BufferReadInt( uint32_t sleep_time, uint8_t cmd_num )
{
    uint32_t timeout_cnt, time_tick;
    uint32_t reg_val=0, field_val=0;
    
    timeout_cnt = sleep_time;
    time_tick   = sleep_time;
    
    do
    {
        cfe_usleep( time_tick );  // [usec]
        reg_val = EMMC_HOSTIF->emmc_host_int_status;
        field_val = reg_val;
        GET_REG_FIELD(field_val, EMMC_HOSTIF_INT_STATUS_BUFFER_READ_INT);
        if( timeout_cnt < 10 )
        {
                timeout_cnt += time_tick;
        }   
        else if( timeout_cnt < 1000 )
        {
            time_tick = sleep_time * 100;
            timeout_cnt += time_tick;
        }
        else if( timeout_cnt < 100000 )
        {
            time_tick = sleep_time * 10000;
            timeout_cnt += time_tick;
        }
        else
        {
            time_tick = 100000;
            timeout_cnt += time_tick;
        }
    }while( (timeout_cnt < EMMC_TIMEOUT_CNT) && (field_val == EMMC_OFF) );
    
#if DEBUG_EMMC_HOST_INT
    if( timeout_cnt > 100000 )
    printf("[emmc_Wait_BufferReadInt] CMD %d : HOST_INT_STATUS = 0x%X, BUFFER_READ_INT = %d (timeout_cnt=%d, sleep_time=%d)\n", 
        cmd_num, reg_val, field_val, timeout_cnt, sleep_time);
#endif
        
    if( timeout_cnt >= EMMC_TIMEOUT_CNT )
    {
        // Check CMD_TIMEOUT_ERR_INT
        reg_val = EMMC_HOSTIF->emmc_host_int_status;
        reg_val = field_val;
        GET_REG_FIELD(field_val, EMMC_HOSTIF_INT_STATUS_CMD_TIMEOUT_ERR_INT);
        printf("[emmc_Wait_BufferReadInt] CMD %d : HOST_INT_STATUS = 0x%X, CMD_TIMEOUT_ERR_INT = %d\n", cmd_num, reg_val, field_val); 
        
        // Call Error_Recovery_Sequence
        
        return EMMC_NG;
    }
    else
    {
        emmc_Clear_HostIntStatus(EMMC_HOSTIF_INT_STATUS_BUFFER_READ_INT_MASK);
        return EMMC_OK;
    }

}


uint32_t emmc_Wait_BufferWriteInt( uint32_t sleep_time, uint8_t cmd_num )
{
    uint32_t timeout_cnt, time_tick;
    uint32_t reg_val=0, field_val=0;
    
    timeout_cnt = sleep_time;
    time_tick   = sleep_time;
    
    do
    {
        cfe_usleep( time_tick );  // [usec]
        reg_val = EMMC_HOSTIF->emmc_host_int_status;
        field_val = reg_val;
        GET_REG_FIELD(field_val, EMMC_HOSTIF_INT_STATUS_BUFFER_WRITE_INT);
        if( timeout_cnt < 10 )
        {
            timeout_cnt += time_tick;
        }   
        else if( timeout_cnt < 1000 )
        {
            time_tick = sleep_time * 100;
            timeout_cnt += time_tick;
        }
        else if( timeout_cnt < 100000 )
        {
            time_tick = sleep_time * 10000;
            timeout_cnt += time_tick;
        }
        else
        {
            time_tick = 100000;
            timeout_cnt += time_tick;
        }
    }while( (timeout_cnt < EMMC_TIMEOUT_CNT) && (field_val == EMMC_OFF) );
    
#if DEBUG_EMMC_HOST_INT
    if( timeout_cnt > 100000 )
    printf("[emmc_Wait_BufferWriteInt] CMD %d : HOST_INT_STATUS = 0x%X, BUFFER_WRITE_INT = %d (timeout_cnt=%d, sleep_time=%d)\n", 
        cmd_num, reg_val, field_val, timeout_cnt, sleep_time);
#endif
    
    if( timeout_cnt >= EMMC_TIMEOUT_CNT )
    {
        // Check CMD_TIMEOUT_ERR_INT
        reg_val = EMMC_HOSTIF->emmc_host_int_status;
        reg_val = field_val;
        GET_REG_FIELD(field_val, EMMC_HOSTIF_INT_STATUS_CMD_TIMEOUT_ERR_INT);
        printf("[emmc_Wait_BufferWriteInt] CMD %d : HOST_INT_STATUS = 0x%X, CMD_TIMEOUT_ERR_INT = %d\n", cmd_num, reg_val, field_val); 
        
        // Call Error_Recovery_Sequence
        
        return EMMC_NG;
    }
    else
    {
        emmc_Clear_HostIntStatus(EMMC_HOSTIF_INT_STATUS_BUFFER_WRITE_INT_MASK);
        return EMMC_OK;
    }

}

uint32_t emmc_Wait_Xfer_complete( uint32_t sleep_time, uint8_t cmd_num )
{
    uint32_t timeout_cnt, time_tick;
    uint32_t reg_val=0, field_val=0;
    
    timeout_cnt = sleep_time;
    time_tick   = sleep_time;

    do
    {
        cfe_usleep( time_tick );  // [usec]
        reg_val = EMMC_HOSTIF->emmc_host_int_status;
        field_val = reg_val;
        GET_REG_FIELD(field_val, EMMC_HOSTIF_INT_STATUS_TRANSFER_COMPLETE_INT);
        if( timeout_cnt < 10 )
        {
            timeout_cnt += time_tick;
        }   
        else if( timeout_cnt < 1000 )
        {
            time_tick = sleep_time * 100;
            timeout_cnt += time_tick;
        }
        else if( timeout_cnt < 100000 )
        {
            time_tick = sleep_time * 10000;
            timeout_cnt += time_tick;
        }
        else
        {
            time_tick = 100000;
            timeout_cnt += time_tick;
        }
    }while( (timeout_cnt < EMMC_TIMEOUT_CNT) && (field_val == EMMC_OFF) );
    
#if DEBUG_EMMC_HOST_INT
    if( timeout_cnt > 100000 )  // Over 100[ms]
    printf("[emmc_Wait_Xfer_complete] CMD %d : HOST_INT_STATUS = 0x%X, TRANSFER_COMPLETE_INT = %d (timeout_cnt=%d, sleep_time=%d)\n", 
        cmd_num, reg_val, field_val, timeout_cnt, sleep_time);
#endif
    
    if( timeout_cnt >= EMMC_TIMEOUT_CNT )
    {
        // Check CMD_TIMEOUT_ERR_INT
        reg_val = EMMC_HOSTIF->emmc_host_int_status;
        reg_val = field_val;
        GET_REG_FIELD(field_val, EMMC_HOSTIF_INT_STATUS_CMD_TIMEOUT_ERR_INT);
        printf("[emmc_Wait_Xfer_complete] CMD %d : HOST_INT_STATUS = 0x%X, CMD_TIMEOUT_ERR_INT = %d\n", cmd_num, reg_val, field_val);
        
        // Call Error_Recovery_Sequence
        return EMMC_NG;
    }
    else
    {
        emmc_Clear_HostIntStatus(EMMC_HOSTIF_INT_STATUS_TRANSFER_COMPLETE_INT_MASK);
        return EMMC_OK;
    }

}


uint32_t emmc_Wait_ReadyDataXfer( emmcdev_t *emmc_config, uint32_t sleep_time, uint8_t cmd_num )
{
    uint32_t timeout_cnt, time_tick;
    uint32_t emmc_status=0, emmc_state=0;
    
    timeout_cnt = sleep_time;
    time_tick   = sleep_time;
    
    do
    {
        emmc_config->CST = emmc_CMD13_C0ba_acR1_SEND_STATUS( emmc_config );
        emmc_status = ( emmc_config->CST & CST_READY_FOR_DATA_MASK ) >> CST_READY_FOR_DATA_SHIFT;
        emmc_state  = ( emmc_config->CST & CST_CURRENT_STATE_MASK ) >> CST_CURRENT_STATE_SHIFT;
        cfe_usleep( time_tick );
        if( timeout_cnt < 10 )
        {
            timeout_cnt += time_tick;
        }   
        else if( timeout_cnt < 1000 )
        {
            time_tick = sleep_time * 100;
            timeout_cnt += time_tick;
        }
        else if( timeout_cnt < 100000 )
        {
            time_tick = sleep_time * 10000;
            timeout_cnt += time_tick;
        }
        else
        {
            time_tick = 100000;
            timeout_cnt += time_tick;
        }
    }while( (emmc_status!=EMMC_ON || emmc_state!=CST_STATE_TRAN) && (timeout_cnt < EMMC_TIMEOUT_CNT) );
    
#if DEBUG_EMMC_HOST_INT
    if( timeout_cnt > 100000 )
    printf("[emmc_Wait_ReadyDataXfer] CMD %d : EMMC_STATUS = 0x%X, EMMC_STATE = 0x%X (timeout_cnt=%d, sleep_time=%d)\n", 
        cmd_num, emmc_config->CST, emmc_state, timeout_cnt, sleep_time);
#endif
    
    if( timeout_cnt >= EMMC_TIMEOUT_CNT )
    {
        printf("[emmc_Wait_ReadyDataXfer] CMD %d : EMMC_STATUS = 0x%X, timeout_cnt = %d\n", cmd_num, emmc_config->CST, timeout_cnt);
        // Call Error_Recovery_Sequence
        
        return EMMC_NG;
    }
    else
    {
        return EMMC_OK;
    }
    
}


uint32_t emmc_Wait_EmmcNextState( emmcdev_t *emmc_config, uint32_t next_state, uint32_t sleep_time, uint8_t cmd_num )
{
    uint32_t timeout_cnt, time_tick;
    //uint32_t emmc_status=0;
    uint32_t emmc_state=0;
    
    timeout_cnt = sleep_time;
    time_tick   = sleep_time;
    
    do
    {
        emmc_config->CST = emmc_CMD13_C0ba_acR1_SEND_STATUS( emmc_config );
        //emmc_status = ( emmc_config->CST & CST_READY_FOR_DATA_MASK ) >> CST_READY_FOR_DATA_SHIFT;
        emmc_state  = ( emmc_config->CST & CST_CURRENT_STATE_MASK ) >> CST_CURRENT_STATE_SHIFT;
        cfe_usleep( time_tick );
        if( timeout_cnt < 10 )
        {
            timeout_cnt += time_tick;
        }   
        else if( timeout_cnt < 1000 )
        {
            time_tick = sleep_time * 100;
            timeout_cnt += time_tick;
        }
        else if( timeout_cnt < 100000 )
        {
            time_tick = sleep_time * 10000;
            timeout_cnt += time_tick;
        }
        else
        {
            time_tick = 100000;
            timeout_cnt += time_tick;
        }
    //}while( (emmc_status!=EMMC_ON || emmc_state!=next_state) && (timeout_cnt < EMMC_TIMEOUT_CNT) );
    }while( (emmc_state!=next_state) && (timeout_cnt < EMMC_TIMEOUT_CNT) );
    
#if DEBUG_EMMC_HOST_INT
    if( timeout_cnt > 100000 )
    printf("[emmc_Wait_EmmcNextState] CMD %d : EMMC_STATUS = 0x%X, EMMC_STATE = 0x%X (timeout_cnt=%d, time_tick=%d, sleep_time=%d)\n", 
        cmd_num, emmc_config->CST, emmc_state, timeout_cnt, time_tick, sleep_time);
#endif
    
    if( timeout_cnt >= EMMC_TIMEOUT_CNT )
    {
        printf("\n >>>>> emmc_Wait_EmmcNextState : EMMC_STATUS = 0x%08X (timeout_cnt=%d, CMD %d)\n\n", emmc_config->CST, timeout_cnt, cmd_num);
        // Call Error_Recovery_Sequence
        
        return EMMC_NG;
    }
    else
    {
        return EMMC_OK;
    }
    
}


#if DEBUG_BUFFDATA
void emmc_Write_Blocks( uint32_t *write_data, uint32_t nBlocks, uint32_t nBuffData, emmcdev_t *emmcdev )
{
    uint32_t i, j, k;
            
    for( j=0; j<nBlocks; j++ )
    {
        k = 128*j;
        
        for( i=0; i<nBuffData; i++ )  // nBuffData = 128 : 512 Bytes
        {
            emmcdev->CST = emmc_Wait_BufferWriteInt( 1, 245 );
            EMMC_HOSTIF->emmc_host_buffdata = write_data[k+i];
        }
    }

}
#else
void emmc_Write_Blocks( uint32_t *write_data, uint32_t nBlocks, uint32_t nBuffData, emmcdev_t *emmcdev )
{
    uint32_t i, j, k, ii;
    uint32_t regval;

    do
    {
        cfe_usleep( emmcdev->config.ReadBufSleep );  // [usec]
        regval = EMMC_HOSTIF->emmc_host_state;
        GET_REG_FIELD(regval, EMMC_HOSTIF_STATE_BUFF_WREN);
    }while( regval == EMMC_OFF );
    
    for( j=0; j<nBlocks; j++ )
    {
        k = 128*j;
        
        // Wait for buffering ( Buffer : 2 x 1K )
        if( j != 0 )
        {
            if( (j%2) == 0 )
            {
                //cfe_usleep( emmcdev->config.WriteBufSleep );
                cfe_usleep( emmcdev->config.ReadBufSleep * 20 );         // * 1000
                if( (j%32) == 0 )
                    cfe_usleep( emmcdev->config.WriteBufSleep * 20 );    // * 1000
            }
        }    
        
        for( i=0; i<nBuffData; i++ )  // nBuffData = 128 : 512 Bytes
        {
            ii = k + i;
            //cfe_usleep( emmcdev->config.WriteBufSleep );
            EMMC_HOSTIF->emmc_host_buffdata = write_data[ii];
        }
    }

}
#endif


#if DEBUG_BUFFDATA
void emmc_Read_Blocks( uint32_t *read_data, uint32_t nBlocks, uint32_t nBuffData, emmcdev_t *emmcdev )
{
    uint32_t i, j, k;
        
    for( j=0; j<nBlocks; j++ )
    {
        k = 128*j;
        
        for( i=0; i<nBuffData; i++ )  // nBuffData = 128 : 512 Bytes
        {
            emmcdev->CST = emmc_Wait_BufferReadInt( 1, 178 );
            read_data[k+i] = EMMC_HOSTIF->emmc_host_buffdata;
        }
    }

}
#else
void emmc_Read_Blocks( uint32_t *read_data, uint32_t nBlocks, uint32_t nBuffData, emmcdev_t *emmcdev )
{
    uint32_t i, j, k, ii;
    uint32_t regval;

    do
    {
        cfe_usleep( emmcdev->config.ReadBufSleep );  // [usec]
        regval = EMMC_HOSTIF->emmc_host_state;
        GET_REG_FIELD(regval, EMMC_HOSTIF_STATE_BUFF_RDEN);
    }while( regval == EMMC_OFF );

    for( j=0; j<nBlocks; j++ )
    {
        k = 128*j;

        // Wait for buffering ( Buffer : 2 x 1K )
        if( j != 0 )
        {
            if( (j%2) == 0 )
            {
                cfe_usleep( emmcdev->config.ReadBufSleep );
                if( (j%32) == 0 )
                    //cfe_usleep( emmcdev->config.ReadBufSleep * 100 );
                    cfe_usleep( emmcdev->config.WriteBufSleep );
            }
        }
        
        for( i=0; i<nBuffData; i++ )  // nBuffData = 128 : 512 Bytes
        {
            ii = k + i;
            //cfe_usleep( emmcdev->config.ReadBufSleep );
            read_data[ii] = EMMC_HOSTIF->emmc_host_buffdata;
        }
    }

}
#endif


uint32_t emmc_GetResponse07_1( uint32_t regval, uint32_t regidx, uint32_t width, uint32_t offset )
{
    uint32_t nShift, mask;

    nShift = offset - regidx*32 - 8;  // 32=RegSize, 8=Offset_for_CRC7
    mask = (1 << width) - 1;

    return (regval >> nShift) & mask;

}


uint32_t emmc_GetResponse07_2( uint32_t reg1val, uint32_t reg0val, uint32_t reg0idx, uint32_t width, uint32_t offset )
{
    uint32_t nShift0, width1, width0, mask1, mask0, val1, val0;

    nShift0 = offset - reg0idx*32 - 8;  // 32=RegSize, 8=Offset_for_CRC7
    width1 = width - ( 32 - nShift0 );
    width0 = width - width1;
    mask1 = (1 << width1) - 1;
    mask0 = (1 << width0) - 1;
    val1 = reg1val & mask1;
    val0 = (reg0val >> nShift0) & mask0;

    return (val1 << width0) + val0;

}


uint32_t emmc_GetCSDEraseTimeout( emmcdev_t *emmcdev )
{
    uint32_t EraseTimeout, ReadTimeout;
    uint32_t TimeUnit, MultiFactor;
    uint32_t TimeUnitData[8] = { 1, 10, 100, 1000, 10000, 100000, 1000000, 10000000 };
    //float    MultiData[16]   = { 0, 1.0, 1.2, 1.3, 1.5, 2.0, 2.5, 3.0, 3.5, 4.0, 4.5, 5.0, 5.5, 6.0, 7.0, 8.0 };
    uint32_t MultiData[16]   = { 0, 10, 12, 13, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60, 70, 80 };

    TimeUnit = (uint32_t)(emmcdev->CSD.TAAC & 0x07);
    MultiFactor = (emmcdev->CSD.TAAC >> 3) & 0xFF;
    ReadTimeout = ( TimeUnitData[TimeUnit] * MultiData[MultiFactor] ) / 10;

    EraseTimeout = ReadTimeout * (1 << emmcdev->CSD.R2W_FACTOR);
    
    emmcdev->config.ReadTimeout  = ReadTimeout;
    emmcdev->config.WriteTimeout = EraseTimeout;
    //emmcdev->config.EraseTimeout = EraseTimeout;

    // Make [ms] unit from [ns]
    EraseTimeout = (uint32_t)(EraseTimeout/1000000);
    if( EraseTimeout < 1 )
        EraseTimeout = 1;

#if DEBUG_EMMC_CONFIG
    printf("# emmc_GetCSDEraseTimeout \n");
    printf("  TimeUnitData(TimeUnit) = %d, (CSD.TAAC=%d)\n", TimeUnitData[TimeUnit], emmcdev->CSD.TAAC);
    printf("  MultiData(MultiFactor) = %f\n", MultiData[MultiFactor]);
    printf("  ReadTimeout[ns] = %d, (CSD_R2W_FACTOR=%d)\n", ReadTimeout, emmcdev->CSD.R2W_FACTOR);
    printf("  EraseWriteTimeout[ms] = %d\n", EraseTimeout);
#endif

    return EraseTimeout;

}


void emmc_Get_ExtCSD( emmcdev_t *emmcdev )
{
    uint32_t read_data[128];   // 128 dword = 512 Byte
    uint8_t  i, Temp_ExtCSD[64];

    
    // Read Register values
    for( i=0; i<128; i++ )  // 512B = 128 * 32bit(Reg. size)
    {
        cfe_usleep ( emmcdev->config.ReadBufSleep );
        read_data[i] = EMMC_HOSTIF->emmc_host_buffdata;
        //printf(" read_ExtCSD( %3d ) = 0x%08X (%d)\n", i, read_data[i], read_data[i]);
    }
    
    // Properties Segment
    emmc_Decode_ExtCSD_value( read_data, Temp_ExtCSD,  1, 504 );  emmcdev->ExtCSD.S_CMD_SET                   = Temp_ExtCSD[0];
    emmc_Decode_ExtCSD_value( read_data, Temp_ExtCSD,  1, 503 );  emmcdev->ExtCSD.HPI_FEATURES                = Temp_ExtCSD[0];
    emmc_Decode_ExtCSD_value( read_data, Temp_ExtCSD,  1, 502 );  emmcdev->ExtCSD.BKOPS_SUPPORT               = Temp_ExtCSD[0];
    emmc_Decode_ExtCSD_value( read_data, Temp_ExtCSD,  1, 501 );  emmcdev->ExtCSD.MAX_PACKED_READS            = Temp_ExtCSD[0];
    emmc_Decode_ExtCSD_value( read_data, Temp_ExtCSD,  1, 500 );  emmcdev->ExtCSD.MAX_PACKED_WRITES           = Temp_ExtCSD[0];
    emmc_Decode_ExtCSD_value( read_data, Temp_ExtCSD,  1, 499 );  emmcdev->ExtCSD.DATA_TAG_SUPPORT            = Temp_ExtCSD[0];
    emmc_Decode_ExtCSD_value( read_data, Temp_ExtCSD,  1, 498 );  emmcdev->ExtCSD.TAG_UNIT_SIZE               = Temp_ExtCSD[0];
    emmc_Decode_ExtCSD_value( read_data, Temp_ExtCSD,  1, 497 );  emmcdev->ExtCSD.TAG_RES_SIZE                = Temp_ExtCSD[0];
    emmc_Decode_ExtCSD_value( read_data, Temp_ExtCSD,  1, 496 );  emmcdev->ExtCSD.CONTEXT_CAPABILITIES        = Temp_ExtCSD[0];
    emmc_Decode_ExtCSD_value( read_data, Temp_ExtCSD,  1, 495 );  emmcdev->ExtCSD.LARGE_UNIT_SIZE_M1          = Temp_ExtCSD[0];
    emmc_Decode_ExtCSD_value( read_data, Temp_ExtCSD,  1, 494 );  emmcdev->ExtCSD.EXT_SUPPORT                 = Temp_ExtCSD[0];
    emmc_Decode_ExtCSD_value( read_data, Temp_ExtCSD,  4, 249 );  emmcdev->ExtCSD.CACHE_SIZE                  = (Temp_ExtCSD[3]<<24) + (Temp_ExtCSD[2]<<16) + (Temp_ExtCSD[1]<<8) + Temp_ExtCSD[0];
    emmc_Decode_ExtCSD_value( read_data, Temp_ExtCSD,  1, 248 );  emmcdev->ExtCSD.GENERIC_CMD6_TIME           = Temp_ExtCSD[0];
    emmc_Decode_ExtCSD_value( read_data, Temp_ExtCSD,  1, 247 );  emmcdev->ExtCSD.POWER_OFF_LONG_TIME         = Temp_ExtCSD[0];
    emmc_Decode_ExtCSD_value( read_data, Temp_ExtCSD,  1, 246 );  emmcdev->ExtCSD.BKOPS_STATUS                = Temp_ExtCSD[0];
    emmc_Decode_ExtCSD_value( read_data, Temp_ExtCSD,  4, 242 );  emmcdev->ExtCSD.CORRECTLY_PRG_SECTORS_NUM   = (Temp_ExtCSD[3]<<24) + (Temp_ExtCSD[2]<<16) + (Temp_ExtCSD[1]<<8) + Temp_ExtCSD[0];
    emmc_Decode_ExtCSD_value( read_data, Temp_ExtCSD,  1, 241 );  emmcdev->ExtCSD.INI_TIMEOUT_AP              = Temp_ExtCSD[0];
    emmc_Decode_ExtCSD_value( read_data, Temp_ExtCSD,  1, 239 );  emmcdev->ExtCSD.PWR_CL_DDR_52_360           = Temp_ExtCSD[0];
    emmc_Decode_ExtCSD_value( read_data, Temp_ExtCSD,  1, 238 );  emmcdev->ExtCSD.PWR_CL_DDR_52_195           = Temp_ExtCSD[0];
    emmc_Decode_ExtCSD_value( read_data, Temp_ExtCSD,  1, 237 );  emmcdev->ExtCSD.PWR_CL_200_360              = Temp_ExtCSD[0];
    emmc_Decode_ExtCSD_value( read_data, Temp_ExtCSD,  1, 236 );  emmcdev->ExtCSD.PWR_CL_200_195              = Temp_ExtCSD[0];
    emmc_Decode_ExtCSD_value( read_data, Temp_ExtCSD,  1, 235 );  emmcdev->ExtCSD.MIN_PERF_DDR_W_8_52         = Temp_ExtCSD[0];
    emmc_Decode_ExtCSD_value( read_data, Temp_ExtCSD,  1, 234 );  emmcdev->ExtCSD.MIN_PERF_DDR_R_8_52         = Temp_ExtCSD[0];
    emmc_Decode_ExtCSD_value( read_data, Temp_ExtCSD,  1, 232 );  emmcdev->ExtCSD.TRIM_MULT                   = Temp_ExtCSD[0];
    emmc_Decode_ExtCSD_value( read_data, Temp_ExtCSD,  1, 231 );  emmcdev->ExtCSD.SEC_FEATURE_SUPPORT         = Temp_ExtCSD[0];
    emmc_Decode_ExtCSD_value( read_data, Temp_ExtCSD,  1, 230 );  emmcdev->ExtCSD.SEC_ERASE_MULT              = Temp_ExtCSD[0];
    emmc_Decode_ExtCSD_value( read_data, Temp_ExtCSD,  1, 229 );  emmcdev->ExtCSD.SEC_TRIM_MULT               = Temp_ExtCSD[0];
    emmc_Decode_ExtCSD_value( read_data, Temp_ExtCSD,  1, 228 );  emmcdev->ExtCSD.BOOT_INFO                   = Temp_ExtCSD[0];
    emmc_Decode_ExtCSD_value( read_data, Temp_ExtCSD,  1, 226 );  emmcdev->ExtCSD.BOOT_SIZE_MULT              = Temp_ExtCSD[0];
    emmc_Decode_ExtCSD_value( read_data, Temp_ExtCSD,  1, 225 );  emmcdev->ExtCSD.ACC_SIZE                    = Temp_ExtCSD[0];
    emmc_Decode_ExtCSD_value( read_data, Temp_ExtCSD,  1, 224 );  emmcdev->ExtCSD.HC_ERASE_GRP_SIZE           = Temp_ExtCSD[0];
    emmc_Decode_ExtCSD_value( read_data, Temp_ExtCSD,  1, 223 );  emmcdev->ExtCSD.ERASE_TIMEOUT_MULT          = Temp_ExtCSD[0];
    emmc_Decode_ExtCSD_value( read_data, Temp_ExtCSD,  1, 222 );  emmcdev->ExtCSD.REL_WR_SEC_C                = Temp_ExtCSD[0];
    emmc_Decode_ExtCSD_value( read_data, Temp_ExtCSD,  1, 221 );  emmcdev->ExtCSD.HC_WP_GRP_SIZE              = Temp_ExtCSD[0];
    emmc_Decode_ExtCSD_value( read_data, Temp_ExtCSD,  1, 220 );  emmcdev->ExtCSD.S_C_VCC                     = Temp_ExtCSD[0];
    emmc_Decode_ExtCSD_value( read_data, Temp_ExtCSD,  1, 219 );  emmcdev->ExtCSD.S_C_VCCQ                    = Temp_ExtCSD[0];
    emmc_Decode_ExtCSD_value( read_data, Temp_ExtCSD,  1, 217 );  emmcdev->ExtCSD.S_A_TIMEOUT                 = Temp_ExtCSD[0];
    emmc_Decode_ExtCSD_value( read_data, Temp_ExtCSD,  4, 212 );  emmcdev->ExtCSD.SEC_COUNT                   = (Temp_ExtCSD[3]<<24) + (Temp_ExtCSD[2]<<16) + (Temp_ExtCSD[1]<<8) + Temp_ExtCSD[0];
    emmc_Decode_ExtCSD_value( read_data, Temp_ExtCSD,  1, 210 );  emmcdev->ExtCSD.MIN_PERF_W_8_52             = Temp_ExtCSD[0];
    emmc_Decode_ExtCSD_value( read_data, Temp_ExtCSD,  1, 209 );  emmcdev->ExtCSD.MIN_PERF_R_8_52             = Temp_ExtCSD[0];
    emmc_Decode_ExtCSD_value( read_data, Temp_ExtCSD,  1, 208 );  emmcdev->ExtCSD.MIN_PERF_W_8_26_4_52        = Temp_ExtCSD[0];
    emmc_Decode_ExtCSD_value( read_data, Temp_ExtCSD,  1, 207 );  emmcdev->ExtCSD.MIN_PERF_R_8_26_4_52        = Temp_ExtCSD[0];
    emmc_Decode_ExtCSD_value( read_data, Temp_ExtCSD,  1, 206 );  emmcdev->ExtCSD.MIN_PERF_W_4_26             = Temp_ExtCSD[0];
    emmc_Decode_ExtCSD_value( read_data, Temp_ExtCSD,  1, 205 );  emmcdev->ExtCSD.MIN_PERF_R_4_26             = Temp_ExtCSD[0];
    emmc_Decode_ExtCSD_value( read_data, Temp_ExtCSD,  1, 203 );  emmcdev->ExtCSD.PWR_CL_26_360               = Temp_ExtCSD[0];
    emmc_Decode_ExtCSD_value( read_data, Temp_ExtCSD,  1, 202 );  emmcdev->ExtCSD.PWR_CL_52_360               = Temp_ExtCSD[0];
    emmc_Decode_ExtCSD_value( read_data, Temp_ExtCSD,  1, 201 );  emmcdev->ExtCSD.PWR_CL_26_195               = Temp_ExtCSD[0];
    emmc_Decode_ExtCSD_value( read_data, Temp_ExtCSD,  1, 200 );  emmcdev->ExtCSD.PWR_CL_52_195               = Temp_ExtCSD[0];
    emmc_Decode_ExtCSD_value( read_data, Temp_ExtCSD,  1, 199 );  emmcdev->ExtCSD.PARTITION_SWITCH_TIME       = Temp_ExtCSD[0];
    emmc_Decode_ExtCSD_value( read_data, Temp_ExtCSD,  1, 198 );  emmcdev->ExtCSD.OUT_OF_INTERRUPT_TIME       = Temp_ExtCSD[0];
    emmc_Decode_ExtCSD_value( read_data, Temp_ExtCSD,  1, 197 );  emmcdev->ExtCSD.DRIVER_STRENGTH             = Temp_ExtCSD[0];
    emmc_Decode_ExtCSD_value( read_data, Temp_ExtCSD,  1, 196 );  emmcdev->ExtCSD.DEVICE_TYPE                 = Temp_ExtCSD[0];
    emmc_Decode_ExtCSD_value( read_data, Temp_ExtCSD,  1, 194 );  emmcdev->ExtCSD.CSD_STRUCTURE               = Temp_ExtCSD[0];
    // Modes Segment
    emmc_Decode_ExtCSD_value( read_data, Temp_ExtCSD,  1, 192 );  emmcdev->ExtCSD.EXT_CSD_REV                 = Temp_ExtCSD[0];
    emmc_Decode_ExtCSD_value( read_data, Temp_ExtCSD,  1, 191 );  emmcdev->ExtCSD.CMD_SET                     = Temp_ExtCSD[0];
    emmc_Decode_ExtCSD_value( read_data, Temp_ExtCSD,  1, 189 );  emmcdev->ExtCSD.CMD_SET_REV                 = Temp_ExtCSD[0];
    emmc_Decode_ExtCSD_value( read_data, Temp_ExtCSD,  1, 187 );  emmcdev->ExtCSD.POWER_CLASS                 = Temp_ExtCSD[0];
    emmc_Decode_ExtCSD_value( read_data, Temp_ExtCSD,  1, 185 );  emmcdev->ExtCSD.HS_TIMING                   = Temp_ExtCSD[0];
    emmc_Decode_ExtCSD_value( read_data, Temp_ExtCSD,  1, 183 );  emmcdev->ExtCSD.BUS_WIDTH                   = Temp_ExtCSD[0];
    emmc_Decode_ExtCSD_value( read_data, Temp_ExtCSD,  1, 181 );  emmcdev->ExtCSD.ERASED_MEM_CONT             = Temp_ExtCSD[0];
    emmc_Decode_ExtCSD_value( read_data, Temp_ExtCSD,  1, 179 );  emmcdev->ExtCSD.PARTITION_CONFIG            = Temp_ExtCSD[0];
    emmc_Decode_ExtCSD_value( read_data, Temp_ExtCSD,  1, 178 );  emmcdev->ExtCSD.BOOT_CONFIG_PROT            = Temp_ExtCSD[0];
    emmc_Decode_ExtCSD_value( read_data, Temp_ExtCSD,  1, 177 );  emmcdev->ExtCSD.BOOT_BUS_CONDITIONS         = Temp_ExtCSD[0];
    emmc_Decode_ExtCSD_value( read_data, Temp_ExtCSD,  1, 175 );  emmcdev->ExtCSD.ERASE_GROUP_DEF             = Temp_ExtCSD[0];
    emmc_Decode_ExtCSD_value( read_data, Temp_ExtCSD,  1, 174 );  emmcdev->ExtCSD.BOOT_WP_STATUS              = Temp_ExtCSD[0];
    emmc_Decode_ExtCSD_value( read_data, Temp_ExtCSD,  1, 173 );  emmcdev->ExtCSD.BOOT_WP                     = Temp_ExtCSD[0];
    emmc_Decode_ExtCSD_value( read_data, Temp_ExtCSD,  1, 171 );  emmcdev->ExtCSD.USER_WP                     = Temp_ExtCSD[0];
    emmc_Decode_ExtCSD_value( read_data, Temp_ExtCSD,  1, 169 );  emmcdev->ExtCSD.FW_CONFIG                   = Temp_ExtCSD[0];
    emmc_Decode_ExtCSD_value( read_data, Temp_ExtCSD,  1, 168 );  emmcdev->ExtCSD.RPMB_SIZE_MULT              = Temp_ExtCSD[0];
    emmc_Decode_ExtCSD_value( read_data, Temp_ExtCSD,  1, 167 );  emmcdev->ExtCSD.WR_REL_SET                  = Temp_ExtCSD[0];
    emmc_Decode_ExtCSD_value( read_data, Temp_ExtCSD,  1, 166 );  emmcdev->ExtCSD.WR_REL_PARAM                = Temp_ExtCSD[0];
    emmc_Decode_ExtCSD_value( read_data, Temp_ExtCSD,  1, 165 );  emmcdev->ExtCSD.SANITIZE_START              = Temp_ExtCSD[0];
    emmc_Decode_ExtCSD_value( read_data, Temp_ExtCSD,  1, 164 );  emmcdev->ExtCSD.BKOPS_START                 = Temp_ExtCSD[0];
    emmc_Decode_ExtCSD_value( read_data, Temp_ExtCSD,  1, 163 );  emmcdev->ExtCSD.BKOPS_EN                    = Temp_ExtCSD[0];
    emmc_Decode_ExtCSD_value( read_data, Temp_ExtCSD,  1, 162 );  emmcdev->ExtCSD.RST_n_FUNCTION              = Temp_ExtCSD[0];
    emmc_Decode_ExtCSD_value( read_data, Temp_ExtCSD,  1, 161 );  emmcdev->ExtCSD.HPI_MGMT                    = Temp_ExtCSD[0];
    emmc_Decode_ExtCSD_value( read_data, Temp_ExtCSD,  1, 160 );  emmcdev->ExtCSD.PARTITIONING_SUPPORT        = Temp_ExtCSD[0];
    emmc_Decode_ExtCSD_value( read_data, Temp_ExtCSD,  3, 157 );  emmcdev->ExtCSD.MAX_ENH_SIZE_MULT           = (Temp_ExtCSD[2]<<16) + (Temp_ExtCSD[1]<<8) + Temp_ExtCSD[0];
    emmc_Decode_ExtCSD_value( read_data, Temp_ExtCSD,  1, 156 );  emmcdev->ExtCSD.PARTITIONS_ATTRIBUTE        = Temp_ExtCSD[0];
    emmc_Decode_ExtCSD_value( read_data, Temp_ExtCSD,  1, 155 );  emmcdev->ExtCSD.PARTITION_SETTING_COMPLETED = Temp_ExtCSD[0];
    emmc_Decode_ExtCSD_value( read_data, Temp_ExtCSD, 12, 143 );  for( i=0; i<4; i++ ){ emmcdev->ExtCSD.GP_SIZE_MULT[i] = (Temp_ExtCSD[3*i+2]<<16) + (Temp_ExtCSD[3*i+1]<<8) + Temp_ExtCSD[3*i]; }
    emmc_Decode_ExtCSD_value( read_data, Temp_ExtCSD,  3, 140 );  emmcdev->ExtCSD.ENH_SIZE_MULT               = (Temp_ExtCSD[2]<<16) + (Temp_ExtCSD[1]<<8) + Temp_ExtCSD[0];
    emmc_Decode_ExtCSD_value( read_data, Temp_ExtCSD,  4, 136 );  emmcdev->ExtCSD.ENH_START_ADDR              = (Temp_ExtCSD[3]<<24) + (Temp_ExtCSD[2]<<16) + (Temp_ExtCSD[1]<<8) + Temp_ExtCSD[0];
    emmc_Decode_ExtCSD_value( read_data, Temp_ExtCSD,  1, 134 );  emmcdev->ExtCSD.SEC_BAD_BLK_MGMNT           = Temp_ExtCSD[0];
    if( emmcdev->ExtCSD.EXT_CSD_REV > JESD84_V441)      // JESD84-B45
    {
        emmc_Decode_ExtCSD_value( read_data, Temp_ExtCSD,  1, 132 );  emmcdev->ExtCSD.TCASE_SUPPORT               = Temp_ExtCSD[0];
        emmc_Decode_ExtCSD_value( read_data, Temp_ExtCSD,  1, 131 );  emmcdev->ExtCSD.PERIODIC_WAKEUP             = Temp_ExtCSD[0];
        emmc_Decode_ExtCSD_value( read_data, Temp_ExtCSD,  1, 130 );  emmcdev->ExtCSD.PROGRAM_CID_CSD_DDR_SUPPORT = Temp_ExtCSD[0];
        emmc_Decode_ExtCSD_value( read_data, Temp_ExtCSD, 64,  64 );  for( i=0; i<=63; i++ ) emmcdev->ExtCSD.VENDOR_SPECIFIC_FIELD[i] = Temp_ExtCSD[i];
        emmc_Decode_ExtCSD_value( read_data, Temp_ExtCSD,  1,  63 );  emmcdev->ExtCSD.NATIVE_SECTOR_SIZE          = Temp_ExtCSD[0];
        emmc_Decode_ExtCSD_value( read_data, Temp_ExtCSD,  1,  62 );  emmcdev->ExtCSD.USE_NATIVE_SECTOR           = Temp_ExtCSD[0];
        emmc_Decode_ExtCSD_value( read_data, Temp_ExtCSD,  1,  61 );  emmcdev->ExtCSD.DATA_SECTOR_SIZE            = Temp_ExtCSD[0];
        emmc_Decode_ExtCSD_value( read_data, Temp_ExtCSD,  1,  60 );  emmcdev->ExtCSD.INI_TIMEOUT_EMU             = Temp_ExtCSD[0];
        emmc_Decode_ExtCSD_value( read_data, Temp_ExtCSD,  1,  59 );  emmcdev->ExtCSD.CLASS_6_CTRL                = Temp_ExtCSD[0];
        emmc_Decode_ExtCSD_value( read_data, Temp_ExtCSD,  1,  58 );  emmcdev->ExtCSD.DYNCAP_NEEDED               = Temp_ExtCSD[0];
        emmc_Decode_ExtCSD_value( read_data, Temp_ExtCSD,  2,  56 );  emmcdev->ExtCSD.EXCEPTION_EVENTS_CTRL       = (Temp_ExtCSD[1]<<8) + Temp_ExtCSD[0];
        emmc_Decode_ExtCSD_value( read_data, Temp_ExtCSD,  2,  54 );  emmcdev->ExtCSD.EXCEPTION_EVENTS_STATUS     = (Temp_ExtCSD[1]<<8) + Temp_ExtCSD[0];
        emmc_Decode_ExtCSD_value( read_data, Temp_ExtCSD,  2,  52 );  emmcdev->ExtCSD.EXT_PARTITIONS_ATTRIBUTE    = (Temp_ExtCSD[1]<<8) + Temp_ExtCSD[0];
        emmc_Decode_ExtCSD_value( read_data, Temp_ExtCSD, 15,  37 );  for( i=0; i<=14; i++ ) emmcdev->ExtCSD.CONTEXT_CONF[i] = Temp_ExtCSD[i];
        emmc_Decode_ExtCSD_value( read_data, Temp_ExtCSD,  1,  36 );  emmcdev->ExtCSD.PACKED_COMMAND_STATUS       = Temp_ExtCSD[0];
        emmc_Decode_ExtCSD_value( read_data, Temp_ExtCSD,  1,  35 );  emmcdev->ExtCSD.PACKED_FAILURE_INDEX        = Temp_ExtCSD[0];
        emmc_Decode_ExtCSD_value( read_data, Temp_ExtCSD,  1,  34 );  emmcdev->ExtCSD.POWER_OFF_NOTIFICATION      = Temp_ExtCSD[0];
        emmc_Decode_ExtCSD_value( read_data, Temp_ExtCSD,  1,  33 );  emmcdev->ExtCSD.CACHE_CTRL                  = Temp_ExtCSD[0];
        emmc_Decode_ExtCSD_value( read_data, Temp_ExtCSD,  1,  32 );  emmcdev->ExtCSD.FLUSH_CACHE                 = Temp_ExtCSD[0];
    }

}


void emmc_Decode_ExtCSD_value( uint32_t Reg_Data[], uint8_t ExtCSD_Data[], uint8_t nByte, uint16_t CSD_Addr )
{
    uint8_t AddrIdxL, ByteIdxL, ArrayIdx;
    uint8_t AddrIdx, ByteIdx;
    uint8_t i;

    AddrIdxL = (uint8_t)(CSD_Addr/4);
    ByteIdxL = CSD_Addr - AddrIdxL*4;

    for( i = 0; i < nByte; i++ )
    {
        ArrayIdx = ByteIdxL + i;
        AddrIdx = AddrIdxL + (uint8_t)(ArrayIdx/4);
        ByteIdx = ArrayIdx - (uint8_t)(ArrayIdx/4)*4;
        ExtCSD_Data[i] = (Reg_Data[AddrIdx] >> ByteIdx*8) & 0xFF;
        //printf( " [%2d, %3d, %3d, %2d, %2d] : Reg_Data = %08X, ExtCSD_Data[%d] = 0x%08X ( %d )\n", 
        //  nByte, CSD_Addr+i, AddrIdx, ArrayIdx, ByteIdx, Reg_Data[AddrIdx], i, ExtCSD_Data[i], ExtCSD_Data[i]);
    }

}


//uint32_t emmc_set_reg_field( uint32_t reg, uint32_t data, uint32_t mask, uint32_t shift )
//{
//    return ( (reg & ~mask) | ((data << shift) & mask) );
//}

#if EMMC_DRV_FUTURE
void emmc_timeout_ns(uint32_t ticks)
{
    while( ticks -- > 0 );
}


void emmc_sleep_10ns( uint32_t sleep_time )
{
    uint32_t i, ns_cnt;

    ns_cnt = EMMC_TICKS_10NS;

    //while( sleep_time -- > 0 );

    for( i=0; i<sleep_time; i++ )
    {
        while( ns_cnt -- > 0 );
        ns_cnt = EMMC_TICKS_10NS;
    }
}
#endif


#if 1//DEBUG_EMMC_INIT

char * emmc_get_manufacturer_name( int mid )
{
    int i=0;
    
    while( emmc_mid_name_map[i].mid )
    {
        if( emmc_mid_name_map[i].mid == mid )
            break;
                        
        i++;
    }       
    
    return emmc_mid_name_map[i].mname;          
}

void emmc_Print_CID( emmcdev_t *emmcdev )
{
    printf( "< CID >\n");
    printf(" ManufactureId       : 0x%02X\n", emmcdev->CID.MID);
    printf(" CardBGA             : %d (0:RemovableDevice, 1:BGA, 2:POP, 3:RSVD)\n", emmcdev->CID.CBX);
    printf(" OEMApplicationId    : 0x%02X\n", emmcdev->CID.OID);
    printf(" ProductName         : %s\n", &emmcdev->CID.PNM);
    printf(" ProductRevision     : %d.%d\n", (emmcdev->CID.PRV >> 4), (emmcdev->CID.PRV & 0x0F));
    printf(" ProductSerialNumber : 0x%08X (%d)\n", emmcdev->CID.PSN, emmcdev->CID.PSN);
    printf(" ManufacturingDate   : %d/%d\n", (uint8_t)(emmcdev->CID.MDT >> 4), (uint16_t)(emmcdev->CID.MDT & 0x0F) + 1997);
    printf("\n");
}


void emmc_Print_CSD( emmcdev_t *emmcdev )
{
    printf( "< CSD >\n");
    printf(" CSD_CSD_STRUCTURE      [127:126] = 0x%02X (%d)\n", emmcdev->CSD.CSD_STRUCTURE  , emmcdev->CSD.CSD_STRUCTURE     );
    printf(" CSD_SPEC_VERS          [125:122] = 0x%02X (%d)\n", emmcdev->CSD.SPEC_VERS      , emmcdev->CSD.SPEC_VERS         );
    printf(" CSD_TAAC               [119:112] = 0x%02X (%d)\n", emmcdev->CSD.TAAC               , emmcdev->CSD.TAAC              );
    printf(" CSD_NSAC               [111:104] = 0x%02X (%d)\n", emmcdev->CSD.NSAC               , emmcdev->CSD.NSAC              );
    printf(" CSD_TRAN_SPEED         [103: 96] = 0x%02X (%d)\n", emmcdev->CSD.TRAN_SPEED     , emmcdev->CSD.TRAN_SPEED        );
    printf(" CSD_CCC                [ 95: 84] = 0x%02X (%d)\n", emmcdev->CSD.CCC                , emmcdev->CSD.CCC               );
    printf(" CSD_READ_BL_LEN        [ 83: 80] = 0x%02X (%d)\n", emmcdev->CSD.READ_BL_LEN        , emmcdev->CSD.READ_BL_LEN       );
    printf(" CSD_READ_BL_PARTIAL    [ 79: 79] = 0x%02X (%d)\n", emmcdev->CSD.READ_BL_PARTIAL    , emmcdev->CSD.READ_BL_PARTIAL   );
    printf(" CSD_WRITE_BLK_MISALIGN [ 78: 78] = 0x%02X (%d)\n", emmcdev->CSD.WRITE_BLK_MISALIGN, emmcdev->CSD.WRITE_BLK_MISALIGN);
    printf(" CSD_READ_BLK_MISALIGN  [ 77: 77] = 0x%02X (%d)\n", emmcdev->CSD.READ_BLK_MISALIGN , emmcdev->CSD.READ_BLK_MISALIGN );
    printf(" CSD_DSR_IMP            [ 76: 76] = 0x%02X (%d)\n", emmcdev->CSD.DSR_IMP            , emmcdev->CSD.DSR_IMP           );
    printf(" CSD_C_SIZE             [ 73: 62] = 0x%02X (%d)\n", emmcdev->CSD.C_SIZE         , emmcdev->CSD.C_SIZE            );
    printf(" CSD_VDD_R_CURR_MIN     [ 61: 59] = 0x%02X (%d)\n", emmcdev->CSD.VDD_R_CURR_MIN , emmcdev->CSD.VDD_R_CURR_MIN    );
    printf(" CSD_VDD_R_CURR_MAX     [ 58: 56] = 0x%02X (%d)\n", emmcdev->CSD.VDD_R_CURR_MAX , emmcdev->CSD.VDD_R_CURR_MAX    );
    printf(" CSD_VDD_W_CURR_MIN     [ 55: 53] = 0x%02X (%d)\n", emmcdev->CSD.VDD_W_CURR_MIN , emmcdev->CSD.VDD_W_CURR_MIN    );
    printf(" CSD_VDD_W_CURR_MAX     [ 52: 50] = 0x%02X (%d)\n", emmcdev->CSD.VDD_W_CURR_MAX , emmcdev->CSD.VDD_W_CURR_MAX    );
    printf(" CSD_C_SIZE_MULT        [ 49: 47] = 0x%02X (%d)\n", emmcdev->CSD.C_SIZE_MULT        , emmcdev->CSD.C_SIZE_MULT       );
    printf(" CSD_ERASE_GRP_SIZE     [ 46: 42] = 0x%02X (%d)\n", emmcdev->CSD.ERASE_GRP_SIZE , emmcdev->CSD.ERASE_GRP_SIZE    );
    printf(" CSD_ERASE_GRP_MULT     [ 41: 37] = 0x%02X (%d)\n", emmcdev->CSD.ERASE_GRP_MULT , emmcdev->CSD.ERASE_GRP_MULT    );
    printf(" CSD_WP_GRP_SIZE        [ 36: 32] = 0x%02X (%d)\n", emmcdev->CSD.WP_GRP_SIZE        , emmcdev->CSD.WP_GRP_SIZE       );
    printf(" CSD_WP_GRP_ENABLE      [ 31: 31] = 0x%02X (%d)\n", emmcdev->CSD.WP_GRP_ENABLE  , emmcdev->CSD.WP_GRP_ENABLE     );
    printf(" CSD_DEFAULT_ECC        [ 30: 29] = 0x%02X (%d)\n", emmcdev->CSD.DEFAULT_ECC        , emmcdev->CSD.DEFAULT_ECC       );
    printf(" CSD_R2W_FACTOR         [ 28: 26] = 0x%02X (%d)\n", emmcdev->CSD.R2W_FACTOR     , emmcdev->CSD.R2W_FACTOR        );
    printf(" CSD_WRITE_BL_LEN       [ 25: 22] = 0x%02X (%d)\n", emmcdev->CSD.WRITE_BL_LEN       , emmcdev->CSD.WRITE_BL_LEN      );
    printf(" CSD_WRITE_BL_PARTIAL   [ 21: 21] = 0x%02X (%d)\n", emmcdev->CSD.WRITE_BL_PARTIAL   , emmcdev->CSD.WRITE_BL_PARTIAL  );
    printf(" CSD_CONTENT_PROT_APP   [ 16: 16] = 0x%02X (%d)\n", emmcdev->CSD.CONTENT_PROT_APP   , emmcdev->CSD.CONTENT_PROT_APP  );
    printf(" CSD_FILE_FORMAT_GRP    [ 15: 15] = 0x%02X (%d)\n", emmcdev->CSD.FILE_FORMAT_GRP    , emmcdev->CSD.FILE_FORMAT_GRP   );
    printf(" CSD_COPY               [ 14: 14] = 0x%02X (%d)\n", emmcdev->CSD.COPY               , emmcdev->CSD.COPY              );
    printf(" CSD_PERM_WRITE_PROTECT [ 13: 13] = 0x%02X (%d)\n", emmcdev->CSD.PERM_WRITE_PROTECT, emmcdev->CSD.PERM_WRITE_PROTECT);
    printf(" CSD_TMP_WRITE_PROTECT  [ 12: 12] = 0x%02X (%d)\n", emmcdev->CSD.TMP_WRITE_PROTECT , emmcdev->CSD.TMP_WRITE_PROTECT );
    printf(" CSD_FILE_FORMAT        [ 11: 10] = 0x%02X (%d)\n", emmcdev->CSD.FILE_FORMAT        , emmcdev->CSD.FILE_FORMAT       );
    printf(" CSD_ECC                [  9:  8] = 0x%02X (%d)\n", emmcdev->CSD.ECC                , emmcdev->CSD.ECC               );
    printf("\n");
}


void emmc_Print_ExtCSD( emmcdev_t *emmcdev )
{
    uint8_t i;
    
    printf( "< ExtCSD >\n");
    // Properties Segment
    printf( "[ Properties Segment ]\n");
    printf( "ExtCSD.S_CMD_SET             [504] = 0x%02X (%d)\n", emmcdev->ExtCSD.S_CMD_SET           , emmcdev->ExtCSD.S_CMD_SET            );
    printf( "ExtCSD.HPI_FEATURES          [503] = 0x%02X (%d)\n", emmcdev->ExtCSD.HPI_FEATURES        , emmcdev->ExtCSD.HPI_FEATURES         );
    printf( "ExtCSD.BKOPS_SUPPORT         [502] = 0x%02X (%d)\n", emmcdev->ExtCSD.BKOPS_SUPPORT       , emmcdev->ExtCSD.BKOPS_SUPPORT        );
    printf( "ExtCSD.MAX_PACKED_READS      [501] = 0x%02X (%d)\n", emmcdev->ExtCSD.MAX_PACKED_READS    , emmcdev->ExtCSD.MAX_PACKED_READS     );
    printf( "ExtCSD.MAX_PACKED_WRITES     [500] = 0x%02X (%d)\n", emmcdev->ExtCSD.MAX_PACKED_WRITES   , emmcdev->ExtCSD.MAX_PACKED_WRITES    );
    printf( "ExtCSD.DATA_TAG_SUPPORT      [499] = 0x%02X (%d)\n", emmcdev->ExtCSD.DATA_TAG_SUPPORT    , emmcdev->ExtCSD.DATA_TAG_SUPPORT     );
    printf( "ExtCSD.TAG_UNIT_SIZE         [498] = 0x%02X (%d)\n", emmcdev->ExtCSD.TAG_UNIT_SIZE       , emmcdev->ExtCSD.TAG_UNIT_SIZE        );
    printf( "ExtCSD.TAG_RES_SIZE          [497] = 0x%02X (%d)\n", emmcdev->ExtCSD.TAG_RES_SIZE        , emmcdev->ExtCSD.TAG_RES_SIZE         );
    printf( "ExtCSD.CONTEXT_CAPABILITIES  [496] = 0x%02X (%d)\n", emmcdev->ExtCSD.CONTEXT_CAPABILITIES, emmcdev->ExtCSD.CONTEXT_CAPABILITIES );
    printf( "ExtCSD.LARGE_UNIT_SIZE_M1    [495] = 0x%02X (%d)\n", emmcdev->ExtCSD.LARGE_UNIT_SIZE_M1  , emmcdev->ExtCSD.LARGE_UNIT_SIZE_M1   );
    printf( "ExtCSD.EXT_SUPPORT           [494] = 0x%02X (%d)\n", emmcdev->ExtCSD.EXT_SUPPORT         , emmcdev->ExtCSD.EXT_SUPPORT          );
    printf( "ExtCSD.CACHE_SIZE            [249] = 0x%08X (%d)\n", emmcdev->ExtCSD.CACHE_SIZE          , emmcdev->ExtCSD.CACHE_SIZE          );
    printf( "ExtCSD.GENERIC_CMD6_TIME     [248] = 0x%02X (%d)\n", emmcdev->ExtCSD.GENERIC_CMD6_TIME   , emmcdev->ExtCSD.GENERIC_CMD6_TIME    );
    printf( "ExtCSD.POWER_OFF_LONG_TIME   [247] = 0x%02X (%d)\n", emmcdev->ExtCSD.POWER_OFF_LONG_TIME , emmcdev->ExtCSD.POWER_OFF_LONG_TIME  );
    printf( "ExtCSD.BKOPS_STATUS          [246] = 0x%02X (%d)\n", emmcdev->ExtCSD.BKOPS_STATUS        , emmcdev->ExtCSD.BKOPS_STATUS         );
    printf( "ExtCSD.CORRECTLY_PRG_SECTORS_NUM[242] = 0x%08X (%d)\n", emmcdev->ExtCSD.CORRECTLY_PRG_SECTORS_NUM, emmcdev->ExtCSD.CORRECTLY_PRG_SECTORS_NUM);
    printf( "ExtCSD.INI_TIMEOUT_AP        [241] = 0x%02X (%d)\n", emmcdev->ExtCSD.INI_TIMEOUT_AP     , emmcdev->ExtCSD.INI_TIMEOUT_AP      );
    printf( "ExtCSD.PWR_CL_DDR_52_360     [239] = 0x%02X (%d)\n", emmcdev->ExtCSD.PWR_CL_DDR_52_360  , emmcdev->ExtCSD.PWR_CL_DDR_52_360   );
    printf( "ExtCSD.PWR_CL_DDR_52_195     [238] = 0x%02X (%d)\n", emmcdev->ExtCSD.PWR_CL_DDR_52_195  , emmcdev->ExtCSD.PWR_CL_DDR_52_195   );
    printf( "ExtCSD.PWR_CL_200_360        [237] = 0x%02X (%d)\n", emmcdev->ExtCSD.PWR_CL_200_360     , emmcdev->ExtCSD.PWR_CL_200_360      );
    printf( "ExtCSD.PWR_CL_200_195        [236] = 0x%02X (%d)\n", emmcdev->ExtCSD.PWR_CL_200_195     , emmcdev->ExtCSD.PWR_CL_200_195      );
    printf( "ExtCSD.MIN_PERF_DDR_W_8_52   [235] = 0x%02X (%d)\n", emmcdev->ExtCSD.MIN_PERF_DDR_W_8_52, emmcdev->ExtCSD.MIN_PERF_DDR_W_8_52 );
    printf( "ExtCSD.MIN_PERF_DDR_R_8_52   [234] = 0x%02X (%d)\n", emmcdev->ExtCSD.MIN_PERF_DDR_R_8_52, emmcdev->ExtCSD.MIN_PERF_DDR_R_8_52 );
    printf( "ExtCSD.TRIM_MULT             [232] = 0x%02X (%d)\n", emmcdev->ExtCSD.TRIM_MULT          , emmcdev->ExtCSD.TRIM_MULT           );
    printf( "ExtCSD.SEC_FEATURE_SUPPORT   [231] = 0x%02X (%d)\n", emmcdev->ExtCSD.SEC_FEATURE_SUPPORT, emmcdev->ExtCSD.SEC_FEATURE_SUPPORT );
    printf( "ExtCSD.ExtCSD_SEC_ERASE_MULT [230] = 0x%02X (%d)\n", emmcdev->ExtCSD.SEC_ERASE_MULT     , emmcdev->ExtCSD.SEC_ERASE_MULT );
    printf( "ExtCSD.ExtCSD_SEC_TRIM_MULT  [229] = 0x%02X (%d)\n", emmcdev->ExtCSD.SEC_TRIM_MULT      , emmcdev->ExtCSD.SEC_TRIM_MULT  );
    printf( "ExtCSD.BOOT_INFO             [228] = 0x%02X (%d)\n", emmcdev->ExtCSD.BOOT_INFO         , emmcdev->ExtCSD.BOOT_INFO          );
    printf( "ExtCSD.BOOT_SIZE_MULT        [226] = 0x%02X (%d)\n", emmcdev->ExtCSD.BOOT_SIZE_MULT    , emmcdev->ExtCSD.BOOT_SIZE_MULT     );
    printf( "ExtCSD.ACC_SIZE              [225] = 0x%02X (%d)\n", emmcdev->ExtCSD.ACC_SIZE          , emmcdev->ExtCSD.ACC_SIZE           );
    printf( "ExtCSD.HC_ERASE_GRP_SIZE     [224] = 0x%02X (%d)\n", emmcdev->ExtCSD.HC_ERASE_GRP_SIZE , emmcdev->ExtCSD.HC_ERASE_GRP_SIZE  );
    printf( "ExtCSD.ERASE_TIMEOUT_MULT    [223] = 0x%02X (%d)\n", emmcdev->ExtCSD.ERASE_TIMEOUT_MULT, emmcdev->ExtCSD.ERASE_TIMEOUT_MULT );
    printf( "ExtCSD.REL_WR_SEC_C          [222] = 0x%02X (%d)\n", emmcdev->ExtCSD.REL_WR_SEC_C      , emmcdev->ExtCSD.REL_WR_SEC_C       );
    printf( "ExtCSD.HC_WP_GRP_SIZE        [221] = 0x%02X (%d)\n", emmcdev->ExtCSD.HC_WP_GRP_SIZE    , emmcdev->ExtCSD.HC_WP_GRP_SIZE     );
    printf( "ExtCSD.S_C_VCC               [220] = 0x%02X (%d)\n", emmcdev->ExtCSD.S_C_VCC           , emmcdev->ExtCSD.S_C_VCC            );
    printf( "ExtCSD.S_C_VCCQ              [219] = 0x%02X (%d)\n", emmcdev->ExtCSD.S_C_VCCQ          , emmcdev->ExtCSD.S_C_VCCQ           );
    printf( "ExtCSD.S_A_TIMEOUT           [217] = 0x%02X (%d)\n", emmcdev->ExtCSD.S_A_TIMEOUT       , emmcdev->ExtCSD.S_A_TIMEOUT        );
    printf( "ExtCSD.SEC_COUNT             [212] = 0x%08X (%d)\n", emmcdev->ExtCSD.SEC_COUNT         , emmcdev->ExtCSD.SEC_COUNT          );
    printf( "ExtCSD.MIN_PERF_W_8_52       [210] = 0x%02X (%d)\n", emmcdev->ExtCSD.MIN_PERF_W_8_52      , emmcdev->ExtCSD.MIN_PERF_W_8_52       );
    printf( "ExtCSD.MIN_PERF_R_8_52       [209] = 0x%02X (%d)\n", emmcdev->ExtCSD.MIN_PERF_R_8_52      , emmcdev->ExtCSD.MIN_PERF_R_8_52       );
    printf( "ExtCSD.MIN_PERF_W_8_26_4_52  [208] = 0x%02X (%d)\n", emmcdev->ExtCSD.MIN_PERF_W_8_26_4_52 , emmcdev->ExtCSD.MIN_PERF_W_8_26_4_52  );
    printf( "ExtCSD.MIN_PERF_R_8_26_4_52  [207] = 0x%02X (%d)\n", emmcdev->ExtCSD.MIN_PERF_R_8_26_4_52 , emmcdev->ExtCSD.MIN_PERF_R_8_26_4_52  );
    printf( "ExtCSD.MIN_PERF_W_4_26       [206] = 0x%02X (%d)\n", emmcdev->ExtCSD.MIN_PERF_W_4_26      , emmcdev->ExtCSD.MIN_PERF_W_4_26       );
    printf( "ExtCSD.MIN_PERF_R_4_26       [205] = 0x%02X (%d)\n", emmcdev->ExtCSD.MIN_PERF_R_4_26      , emmcdev->ExtCSD.MIN_PERF_R_4_26       );
    printf( "ExtCSD.PWR_CL_26_360         [203] = 0x%02X (%d)\n", emmcdev->ExtCSD.PWR_CL_26_360        , emmcdev->ExtCSD.PWR_CL_26_360         );
    printf( "ExtCSD.PWR_CL_52_360         [202] = 0x%02X (%d)\n", emmcdev->ExtCSD.PWR_CL_52_360        , emmcdev->ExtCSD.PWR_CL_52_360         );
    printf( "ExtCSD.PWR_CL_26_195         [201] = 0x%02X (%d)\n", emmcdev->ExtCSD.PWR_CL_26_195        , emmcdev->ExtCSD.PWR_CL_26_195         );
    printf( "ExtCSD.PWR_CL_52_195         [200] = 0x%02X (%d)\n", emmcdev->ExtCSD.PWR_CL_52_195        , emmcdev->ExtCSD.PWR_CL_52_195         );
    printf( "ExtCSD.PARTITION_SWITCH_TIME [199] = 0x%02X (%d)\n", emmcdev->ExtCSD.PARTITION_SWITCH_TIME, emmcdev->ExtCSD.PARTITION_SWITCH_TIME );
    printf( "ExtCSD.OUT_OF_INTERRUPT_TIME [198] = 0x%02X (%d)\n", emmcdev->ExtCSD.OUT_OF_INTERRUPT_TIME, emmcdev->ExtCSD.OUT_OF_INTERRUPT_TIME );
    printf( "ExtCSD.DRIVER_STRENGTH       [197] = 0x%02X (%d)\n", emmcdev->ExtCSD.DRIVER_STRENGTH      , emmcdev->ExtCSD.DRIVER_STRENGTH       );
    printf( "ExtCSD.DEVICE_TYPE           [196] = 0x%02X (%d)\n", emmcdev->ExtCSD.DEVICE_TYPE          , emmcdev->ExtCSD.DEVICE_TYPE           );
    printf( "ExtCSD.CSD_STRUCTURE         [194] = 0x%02X (%d)\n", emmcdev->ExtCSD.CSD_STRUCTURE        , emmcdev->ExtCSD.CSD_STRUCTURE         );
    printf( "ExtCSD.EXT_CSD_REV           [192] = 0x%02X (%d)\n", emmcdev->ExtCSD.EXT_CSD_REV          , emmcdev->ExtCSD.EXT_CSD_REV           );
    // Modes Segment
    printf( "[ Modes Segment ]\n");
    printf( "ExtCSD.CMD_SET                     [191] = 0x%02X (%d)\n", emmcdev->ExtCSD.CMD_SET             , emmcdev->ExtCSD.CMD_SET              );
    printf( "ExtCSD.CMD_SET_REV                 [189] = 0x%02X (%d)\n", emmcdev->ExtCSD.CMD_SET_REV         , emmcdev->ExtCSD.CMD_SET_REV          );
    printf( "ExtCSD.POWER_CLASS                 [187] = 0x%02X (%d)\n", emmcdev->ExtCSD.POWER_CLASS         , emmcdev->ExtCSD.POWER_CLASS          );
    printf( "ExtCSD.HS_TIMING                   [185] = 0x%02X (%d)\n", emmcdev->ExtCSD.HS_TIMING           , emmcdev->ExtCSD.HS_TIMING            );
    printf( "ExtCSD.BUS_WIDTH                   [183] = 0x%02X (%d)\n", emmcdev->ExtCSD.BUS_WIDTH           , emmcdev->ExtCSD.BUS_WIDTH            );
    printf( "ExtCSD.ERASED_MEM_CONT             [181] = 0x%02X (%d)\n", emmcdev->ExtCSD.ERASED_MEM_CONT     , emmcdev->ExtCSD.ERASED_MEM_CONT      );
    printf( "ExtCSD.PARTITION_CONFIG            [179] = 0x%02X (%d)\n", emmcdev->ExtCSD.PARTITION_CONFIG    , emmcdev->ExtCSD.PARTITION_CONFIG     );
    printf( "ExtCSD.BOOT_CONFIG_PROT            [178] = 0x%02X (%d)\n", emmcdev->ExtCSD.BOOT_CONFIG_PROT    , emmcdev->ExtCSD.BOOT_CONFIG_PROT     );
    printf( "ExtCSD.BOOT_BUS_CONDITIONS         [177] = 0x%02X (%d)\n", emmcdev->ExtCSD.BOOT_BUS_CONDITIONS , emmcdev->ExtCSD.BOOT_BUS_CONDITIONS  );
    printf( "ExtCSD.ERASE_GROUP_DEF             [175] = 0x%02X (%d)\n", emmcdev->ExtCSD.ERASE_GROUP_DEF     , emmcdev->ExtCSD.ERASE_GROUP_DEF      );
    printf( "ExtCSD.BOOT_WP_STATUS              [174] = 0x%02X (%d)\n", emmcdev->ExtCSD.BOOT_WP_STATUS      , emmcdev->ExtCSD.BOOT_WP_STATUS       );
    printf( "ExtCSD.BOOT_WP                     [173] = 0x%02X (%d)\n", emmcdev->ExtCSD.BOOT_WP             , emmcdev->ExtCSD.BOOT_WP              );
    printf( "ExtCSD.USER_WP                     [171] = 0x%02X (%d)\n", emmcdev->ExtCSD.USER_WP             , emmcdev->ExtCSD.USER_WP              );
    printf( "ExtCSD.FW_CONFIG                   [169] = 0x%02X (%d)\n", emmcdev->ExtCSD.FW_CONFIG           , emmcdev->ExtCSD.FW_CONFIG            );
    printf( "ExtCSD.RPMB_SIZE_MULT              [168] = 0x%02X (%d)\n", emmcdev->ExtCSD.RPMB_SIZE_MULT      , emmcdev->ExtCSD.RPMB_SIZE_MULT       );
    printf( "ExtCSD.WR_REL_SET                  [167] = 0x%02X (%d)\n", emmcdev->ExtCSD.WR_REL_SET          , emmcdev->ExtCSD.WR_REL_SET           );
    printf( "ExtCSD.WR_REL_PARAM                [166] = 0x%02X (%d)\n", emmcdev->ExtCSD.WR_REL_PARAM        , emmcdev->ExtCSD.WR_REL_PARAM         );
    printf( "ExtCSD.SANITIZE_START              [165] = 0x%02X (%d)\n", emmcdev->ExtCSD.SANITIZE_START      , emmcdev->ExtCSD.SANITIZE_START       );
    printf( "ExtCSD.BKOPS_START                 [164] = 0x%02X (%d)\n", emmcdev->ExtCSD.BKOPS_START         , emmcdev->ExtCSD.BKOPS_START          );
    printf( "ExtCSD.BKOPS_EN                    [163] = 0x%02X (%d)\n", emmcdev->ExtCSD.BKOPS_EN            , emmcdev->ExtCSD.BKOPS_EN             );
    printf( "ExtCSD.RST_n_FUNCTION              [162] = 0x%02X (%d)\n", emmcdev->ExtCSD.RST_n_FUNCTION      , emmcdev->ExtCSD.RST_n_FUNCTION       );
    printf( "ExtCSD.HPI_MGMT                    [161] = 0x%02X (%d)\n", emmcdev->ExtCSD.HPI_MGMT            , emmcdev->ExtCSD.HPI_MGMT             );
    printf( "ExtCSD.PARTITIONING_SUPPORT        [160] = 0x%02X (%d)\n", emmcdev->ExtCSD.PARTITIONING_SUPPORT, emmcdev->ExtCSD.PARTITIONING_SUPPORT );
    printf( "ExtCSD.MAX_ENH_SIZE_MULT           [157] = 0x%08X (%d)\n", emmcdev->ExtCSD.MAX_ENH_SIZE_MULT          , emmcdev->ExtCSD.MAX_ENH_SIZE_MULT           );
    printf( "ExtCSD.PARTITIONS_ATTRIBUTE        [156] = 0x%02X (%d)\n", emmcdev->ExtCSD.PARTITIONS_ATTRIBUTE       , emmcdev->ExtCSD.PARTITIONS_ATTRIBUTE        );
    printf( "ExtCSD.PARTITION_SETTING_COMPLETED [155] = 0x%02X (%d)\n", emmcdev->ExtCSD.PARTITION_SETTING_COMPLETED, emmcdev->ExtCSD.PARTITION_SETTING_COMPLETED );
    printf( "ExtCSD.GP_SIZE_MULT_4              [152] = 0x%08X (%d)\n", emmcdev->ExtCSD.GP_SIZE_MULT[3]            , emmcdev->ExtCSD.GP_SIZE_MULT[3]             );
    printf( "ExtCSD.GP_SIZE_MULT_3              [149] = 0x%08X (%d)\n", emmcdev->ExtCSD.GP_SIZE_MULT[2]            , emmcdev->ExtCSD.GP_SIZE_MULT[2]             );
    printf( "ExtCSD.GP_SIZE_MULT_2              [146] = 0x%08X (%d)\n", emmcdev->ExtCSD.GP_SIZE_MULT[1]            , emmcdev->ExtCSD.GP_SIZE_MULT[1]             );
    printf( "ExtCSD.GP_SIZE_MULT_1              [143] = 0x%08X (%d)\n", emmcdev->ExtCSD.GP_SIZE_MULT[0]            , emmcdev->ExtCSD.GP_SIZE_MULT[0]             );
    printf( "ExtCSD.ENH_SIZE_MULT               [140] = 0x%08X (%d)\n", emmcdev->ExtCSD.ENH_SIZE_MULT              , emmcdev->ExtCSD.ENH_SIZE_MULT               );
    printf( "ExtCSD.ENH_START_ADDR              [136] = 0x%08X (%d)\n", emmcdev->ExtCSD.ENH_START_ADDR             , emmcdev->ExtCSD.ENH_START_ADDR              );
    printf( "ExtCSD.SEC_BAD_BLK_MGMNT           [134] = 0x%02X (%d)\n", emmcdev->ExtCSD.SEC_BAD_BLK_MGMNT          , emmcdev->ExtCSD.SEC_BAD_BLK_MGMNT           );
    
    if( emmcdev->ExtCSD.EXT_CSD_REV > JESD84_V441 ) // JESD84-B45
    {
        printf( "ExtCSD.TCASE_SUPPORT               [132] = 0x%02X (%d)\n", emmcdev->ExtCSD.TCASE_SUPPORT              , emmcdev->ExtCSD.TCASE_SUPPORT               );
        printf( "ExtCSD.PERIODIC_WAKEUP             [131] = 0x%02X (%d)\n", emmcdev->ExtCSD.PERIODIC_WAKEUP            , emmcdev->ExtCSD.PERIODIC_WAKEUP             );
        printf( "ExtCSD.PROGRAM_CID_CSD_DDR_SUPPORT [130] = 0x%02X (%d)\n", emmcdev->ExtCSD.PROGRAM_CID_CSD_DDR_SUPPORT, emmcdev->ExtCSD.PROGRAM_CID_CSD_DDR_SUPPORT );
        for( i=0; i<64; i++ )
            printf( "ExtCSD.VENDOR_SPECIFIC_FIELD       [%3d] = 0x%02X (%d)\n", 127-i, emmcdev->ExtCSD.VENDOR_SPECIFIC_FIELD[63-i], emmcdev->ExtCSD.VENDOR_SPECIFIC_FIELD[63-i] );
        printf( "ExtCSD.VENDOR_SPECIFIC_FIELD       [ 64] = 0x%02X (%d)\n", emmcdev->ExtCSD.VENDOR_SPECIFIC_FIELD, emmcdev->ExtCSD.VENDOR_SPECIFIC_FIELD );
        printf( "ExtCSD.NATIVE_SECTOR_SIZE          [ 63] = 0x%02X (%d)\n", emmcdev->ExtCSD.NATIVE_SECTOR_SIZE   , emmcdev->ExtCSD.NATIVE_SECTOR_SIZE    );
        printf( "ExtCSD.USE_NATIVE_SECTOR           [ 62] = 0x%02X (%d)\n", emmcdev->ExtCSD.USE_NATIVE_SECTOR    , emmcdev->ExtCSD.USE_NATIVE_SECTOR     );
        printf( "ExtCSD.DATA_SECTOR_SIZE            [ 61] = 0x%02X (%d)\n", emmcdev->ExtCSD.DATA_SECTOR_SIZE     , emmcdev->ExtCSD.DATA_SECTOR_SIZE      );
        printf( "ExtCSD.INI_TIMEOUT_EMU             [ 60] = 0x%02X (%d)\n", emmcdev->ExtCSD.INI_TIMEOUT_EMU      , emmcdev->ExtCSD.INI_TIMEOUT_EMU       );
        printf( "ExtCSD.CLASS_6_CTRL                [ 59] = 0x%02X (%d)\n", emmcdev->ExtCSD.CLASS_6_CTRL         , emmcdev->ExtCSD.CLASS_6_CTRL          );
        printf( "ExtCSD.DYNCAP_NEEDED               [ 58] = 0x%02X (%d)\n", emmcdev->ExtCSD.DYNCAP_NEEDED        , emmcdev->ExtCSD.DYNCAP_NEEDED         );
        printf( "ExtCSD.EXCEPTION_EVENTS_CTRL       [ 56] = %016X (%d)\n", emmcdev->ExtCSD.EXCEPTION_EVENTS_CTRL   , emmcdev->ExtCSD.EXCEPTION_EVENTS_CTRL    );
        printf( "ExtCSD.EXCEPTION_EVENTS_STATUS     [ 54] = %016X (%d)\n", emmcdev->ExtCSD.EXCEPTION_EVENTS_STATUS , emmcdev->ExtCSD.EXCEPTION_EVENTS_STATUS  );
        printf( "ExtCSD.EXT_PARTITIONS_ATTRIBUTE    [ 52] = %016X (%d)\n", emmcdev->ExtCSD.EXT_PARTITIONS_ATTRIBUTE, emmcdev->ExtCSD.EXT_PARTITIONS_ATTRIBUTE );
        for( i=0; i<15; i++ )
            printf( "ExtCSD.CONTEXT_CONF                  [ %2d] = %016X (%d)\n", 51-i, emmcdev->ExtCSD.CONTEXT_CONF[14-i], emmcdev->ExtCSD.CONTEXT_CONF[14-i] );
        printf( "ExtCSD.PACKED_COMMAND_STATUS         [ 36] = 0x%08X (%d)\n", emmcdev->ExtCSD.PACKED_COMMAND_STATUS , emmcdev->ExtCSD.PACKED_COMMAND_STATUS   );
        printf( "ExtCSD.PACKED_FAILURE_INDEX          [ 35] = 0x%08X (%d)\n", emmcdev->ExtCSD.PACKED_FAILURE_INDEX  , emmcdev->ExtCSD.PACKED_FAILURE_INDEX    );
        printf( "ExtCSD.POWER_OFF_NOTIFICATION        [ 34] = 0x%08X (%d)\n", emmcdev->ExtCSD.POWER_OFF_NOTIFICATION, emmcdev->ExtCSD.POWER_OFF_NOTIFICATION  );
        printf( "ExtCSD.CACHE_CTRL                    [ 33] = 0x%08X (%d)\n", emmcdev->ExtCSD.CACHE_CTRL            , emmcdev->ExtCSD.CACHE_CTRL              );
        printf( "ExtCSD.FLUSH_CACHE                   [ 32] = 0x%08X (%d)\n", emmcdev->ExtCSD.FLUSH_CACHE           , emmcdev->ExtCSD.FLUSH_CACHE             );
    }

}


void emmc_Print_HostAndEmmcInfo( emmcdev_t *emmcdev )
{
    uint32_t reg_ctrl_set0, reg_ctrl_set1;
    uint32_t FreqDiv, ClkFreq;
    uint32_t Mode_8bit, Mode_4bit, BusVolt, HighSpeed, Timeout;
    

    reg_ctrl_set0 = EMMC_HOSTIF->emmc_host_ctrl_set0;
    reg_ctrl_set1 = EMMC_HOSTIF->emmc_host_ctrl_set1;
        
    EMMC_GET_REG_FIELD(FreqDiv, EMMC_HOSTIF_CTRL_SET1_FREQ_CTRL, reg_ctrl_set1);
    if( FreqDiv == 0 )
        ClkFreq = 100000;
    else
        ClkFreq = (uint32_t)( 100000 / ( 2 * FreqDiv ) );

    EMMC_GET_REG_FIELD(Mode_8bit, EMMC_HOSTIF_CTRL_SET0_SD_8BIT_MODE, reg_ctrl_set0);
    EMMC_GET_REG_FIELD(Mode_4bit, EMMC_HOSTIF_CTRL_SET0_SD_4BIT_MODE, reg_ctrl_set0);
    EMMC_GET_REG_FIELD(BusVolt,   EMMC_HOSTIF_CTRL_SET0_SD_BUS_VOLTAGE_SELECT, reg_ctrl_set0);
    EMMC_GET_REG_FIELD(HighSpeed, EMMC_HOSTIF_CTRL_SET0_HIGH_SPEED_ENABLE, reg_ctrl_set0);
    EMMC_GET_REG_FIELD(Timeout,   EMMC_HOSTIF_CTRL_SET1_TIMEOUT_COUNT, reg_ctrl_set1);
    
    //emmcdev->CST = emmc_CMD8_C0ba_adtcR1_SEND_EXT_CSD( emmcdev );
    
    printf("------------------------------\n");
    printf(" HOST CTRL_SET Information\n");
    printf("  Bus Width   : %d%d (00:1-bit, 01:4-bit, 1x:8-bit)\n", Mode_8bit, Mode_4bit);    
    printf("  Bus Voltage : %d (7=3.3volts, 6=3.0volt, 5=1.8volt)\n", BusVolt);
    printf("  Bus Speed   : %d (1=High Speed, 0=Normal Speed)\n", HighSpeed);
    printf("  Bus Freq.   : %d[KHz] (Base Clk=100MHz)\n", ClkFreq);
    printf("  TIMEOUT_COUNT   : 2^%d\n", Timeout);
    printf("  SDIO_1_HOST.CTRL_SET0 : 0x%08X\n", reg_ctrl_set0);
    printf("  SDIO_1_HOST.CTRL_SET1 : 0x%08X\n", reg_ctrl_set1);
    printf(" ------------------------------\n");
    printf(" eMMC CSD, ExtCSD Information\n");
    printf("  < General Info >\n");
    printf("  Device Type               : 0x%02X (xxxx : DDR_52M_Low, DDR_52M_High, SDR_52M, SDR_26M)\n", emmcdev->ExtCSD.DEVICE_TYPE); // V4.41
    printf("  < Size/Partition >\n");
    printf("  < Performance >\n");
    printf("  < Timing >\n");
    printf("  < Power Info >\n");
    printf("  Power Class : %d (0-15 : 100mA/200mA~>800mA/900mA @3.6V(RMS/Max))\n", emmcdev->ExtCSD.POWER_CLASS);
    printf("  Power Class (3.6V, 52MHz) : %d (0-15 : 100mA/200mA~>800mA/900mA (RMS/Max))\n", emmcdev->ExtCSD.PWR_CL_52_360);
    printf("  Power Class (3.6V, 26MHz) : %d\n", emmcdev->ExtCSD.PWR_CL_26_360);
    //printf("  Driver Strength           : %d (v4.5)\n", emmcdev->ExtCSD.DRIVER_STRENGTH);
    printf(" ------------------------------\n");
    printf(" eMMC Setup Information\n");
    printf("  < Bus >\n");
    printf("  HS Timing   : %d (0=Full Speed(~26MHz), 1=High Speed(~52MHz,~104MHz(DDR)), 2=HS200)\n", emmcdev->ExtCSD.HS_TIMING);
    printf("  Bus Width   : %d (0=1b, 1=4b, 2=8b, 5=4b(DDR), 6=4b(DDR)\n", emmcdev->ExtCSD.BUS_WIDTH);
    printf("  < Partition >\n");
    printf("------------------------------\n\n");

}
#if 0
// < eMMC Control >
    // Bus Mode
    uint32_t            HSTiming;
    uint8_t             HostHS_On;
    uint16_t            BusFreq;        // User configuration
    uint8_t             BusWidth;       // User configuration
    uint8_t             BusVoltage;     // User configuration
    // Genral
    uint32_t            GenCmd6Timeout;
    //uint32_t            BusDDR_On;
    // Partition Access Info
    uint8_t             BootPartitionEnable;
    uint8_t             PartitionAccess;
    uint16_t            PartitionSwitchTime;
    // Partition Info
    uint8_t             PartitionCompleted;
    uint8_t             EraseGroupDef;
    // Block Size & Control
    uint16_t            ReadBlkLen;
    uint8_t             ReadBlkLenBit4Addr;
    uint16_t            WriteBlkLen;
    uint8_t             WriteBlkLenBit4Addr;
    //uint32_t            ReadUnitSize;
    //uint32_t            WriteUnitSize;
    //uint8_t             HighCap_On;
    uint32_t            AccSize;
    uint32_t            HcEraseUnitSize;
    uint32_t            HcWpGrpSize;
    uint16_t            HcEraseTimeout;
    uint16_t            HcReadTimeout;
    uint16_t            HcWriteTimeout;
    uint32_t            EraseUnitSize;
    uint32_t            WpGrpSize;
    uint16_t            EraseTimeout;
    uint16_t            ReadTimeout;
    uint16_t            WriteTimeout;
    // HPI Flag
    uint8_t             HPI_On;
#endif    

void emmc_Print_EmmcSizePartitionInfo( emmcdev_t *emmcdev )
{
    printf(" ------------------------------\n");
    printf(" [[[ Size/Partition ]]]\n");
    printf(" < eMMC original Size >\n");
    printf(" ReadBlkLen      = 0x%08X(%d)[B]\n", emmcdev->config.ReadBlkLen, emmcdev->config.WriteBlkLen);
    printf(" WriteBlkLen     = 0x%08X(%d)[B]\n", emmcdev->config.WriteBlkLen, emmcdev->config.WriteBlkLen);
    printf(" AccessSize      = %d[B], %4d[KB]\n", emmcdev->config.AccessSize, emmcdev->config.AccessSize>>10);
    printf(" CacheSize       = %d[B], %4d[KB]\n", emmcdev->config.CacheSize, emmcdev->config.CacheSize>>10);
    printf(" HcEraseUnitSize = 0x%08X[B], %4d[MB]\n", emmcdev->config.HcEraseUnitSize, emmcdev->config.HcEraseUnitSize>>20);
    printf(" HcWpGrpSize     = 0x%08X[B], %4d[MB]\n", emmcdev->config.HcWpGrpSize, emmcdev->config.HcWpGrpSize>>20);
    printf(" DataSize    = 0x%012llX[B], %6d[MB]\n", emmcdev->config.DataSize,  (uint32_t)(emmcdev->config.DataSize>>20));
    printf(" Boot1Size   = 0x%012X[B], %6d[MB]\n", emmcdev->config.Boot1Size,   (emmcdev->config.Boot1Size>>20));
    printf(" Boot2Size   = 0x%012X[B], %6d[MB]\n", emmcdev->config.Boot2Size,   (emmcdev->config.Boot2Size>>20));
    printf(" RPMBSize    = 0x%012X[B], %6d[MB]\n", emmcdev->config.RPMBSize,    (emmcdev->config.RPMBSize>>20));
    printf(" MaxEnhSize  = 0x%012llX[B], %6d[MB]\n", emmcdev->config.MaxEnhSize,    (uint32_t)(emmcdev->config.MaxEnhSize>>20));
    printf(" < User Defined Size >\n");
    printf(" GP1Size     = 0x%012llX[B], %4d[MB]\n", emmcdev->config.GP1Size, (uint32_t)(emmcdev->config.GP1Size>>20));
    printf(" GP2Size     = 0x%012llX[B], %4d[MB]\n", emmcdev->config.GP2Size, (uint32_t)(emmcdev->config.GP2Size>>20));
    printf(" GP3Size     = 0x%012llX[B], %4d[MB]\n", emmcdev->config.GP3Size, (uint32_t)(emmcdev->config.GP3Size>>20));
    printf(" GP4Size     = 0x%012llX[B], %4d[MB]\n", emmcdev->config.GP4Size, (uint32_t)(emmcdev->config.GP4Size>>20));
    printf(" DataEnhSize = 0x%012llX[B], %4d[MB]\n", emmcdev->config.DataEnhSize, (uint32_t)(emmcdev->config.DataEnhSize>>20));
    printf(" DataEnhAddr = 0x%012llX\n", emmcdev->config.DataEnhAddr);
    printf(" ------------------------------\n");
    printf("\n");
#if 0
    // Calculate EraseUnitSize, EraseTimeout, WriteProtectionGroupSize : CSD_WRITE_BL_LEN[512B]
    emmcdev->config.EraseUnitSize   = (emmcdev->CSD.ERASE_GRP_SIZE + 1) * (emmcdev->CSD.ERASE_GRP_MULT + 1); // * CSD_WRITE_BL_LEN //=[512B]
    emmcdev->config.WpGrpSize       = (emmcdev->CSD.WP_GRP_SIZE + 1) * emmcdev->config.EraseUnitSize;
    emmcdev->config.EraseTimeout    = emmc_GetCSDEraseTimeout( emmcdev );   // Including ReadTimeout, WriteTimeout
    
    // Calculate HighCapacity EraseUnitSize, EraseTimeout, WriteProtectionGroupSize
    emmcdev->config.HcEraseTimeout  = emmcdev->ExtCSD.ERASE_TIMEOUT_MULT * 300; // [ms]
    emmcdev->config.HcReadTimeout   = emmcdev->config.ReadTimeout;      // [ms], Need to investigate.
    emmcdev->config.HcWriteTimeout  = emmcdev->config.HcEraseTimeout;   // [ms]
#endif
}   
#endif

#if DEBUG_EMMC_SLEEP_TIMER
void emmc_Check_SleepTime( void )
{
    unsigned long StartTime, EndTime;
    uint32_t Time, i;
    
    printf("------------------------------\n");
    for( i=3; i>0; i-- )
    {
        printf(" Prepare Stopwatch in %d[sec]!!!\n", i);
        cfe_usleep(1000 * 1000 );
    }
    printf("\n [cfe_usleep] Start Stopwatch for 10[sec]!!!\n");
    StartTime = _getticks( );
    cfe_usleep( 10000000 );
    EndTime = _getticks( );
    Time = (uint32_t)(EndTime - StartTime);
    printf(" [cfe_usleep] Stop Stopwatch!!!\n");
    printf(" Result\n");
    printf(" - tikcs/10[sec]  = %lu\n", Time );
    printf(" - tikcs/ 1[sec]  = %lu\n", Time/10 );
    printf(" - tikcs/ 1[msec] = %lu\n", Time/10000 );
    printf(" - tikcs/ 1[usec] = %lu\n", Time/10000000 );
    
    printf("\n [cfe_usleep] Start Stopwatch for 1[sec]!!!\n");
    StartTime = _getticks( );
    cfe_usleep( 1000000 );
    EndTime = _getticks( );
    Time = (uint32_t)(EndTime - StartTime);
    printf(" [cfe_usleep] Stop Stopwatch!!!\n");
    printf(" Result\n");
    printf(" - tikcs/ 1[sec]  = %lu\n", Time );
    printf(" - tikcs/ 1[msec] = %lu\n", Time/1000 );
    printf(" - tikcs/ 1[usec] = %lu\n\n", Time/1000000 );
    
    printf("\n\n [cfe_usleep] Start Stopwatch for 1[msec]!!!\n");
    StartTime = _getticks( );
    cfe_usleep( 1000 );
    EndTime = _getticks( );
    Time = (uint32_t)(EndTime - StartTime);
    printf(" [cfe_usleep] Stop Stopwatch!!!\n\n");
    printf(" Result\n");
    printf(" - tikcs/ 1[msec]  = %lu\n", Time );
    printf(" - tikcs/ 1[usec] = %lu\n", Time/1000 );
    
    printf("\n\n [cfe_usleep] Start Stopwatch for 1[usec]!!!\n");
    StartTime = _getticks( );
    cfe_usleep( 1 );
    EndTime = _getticks( );
    Time = (uint32_t)(EndTime - StartTime);
    printf(" [cfe_usleep] Stop Stopwatch!!!\n\n");
    printf(" Result\n");
    printf(" - tikcs/ 1[usec]  = %lu\n", Time );
    
    printf("\n------------------------------\n");
    for( i=3; i>0; i-- )
    {
        printf(" Prepare Stopwatch in %d[sec]!!!\n", i);
        cfe_usleep(1000 * 1000 );
    }
    printf("\n [cfe_msleep] Start Stopwatch for 10[sec]!!!\n");
    StartTime = _getticks( );
    cfe_usleep(1000 * 10000 );
    EndTime = _getticks( );
    Time = (uint32_t)(EndTime - StartTime);
    printf(" [cfe_msleep] Stop Stopwatch!!!\n\n");
    printf(" Result\n");
    printf(" - tikcs/10[sec]  = %lu\n", Time );
    printf(" - tikcs/ 1[sec]  = %lu\n", Time/10 );
    printf(" - tikcs/ 1[msec] = %lu\n", Time/10000 );
        
    printf("\n\n [cfe_msleep] Start Stopwatch for 1[sec]!!!\n");
    StartTime = _getticks( );
    cfe_usleep(1000 * 1000 );
    EndTime = _getticks( );
    Time = (uint32_t)(EndTime - StartTime);
    printf(" [cfe_msleep] Stop Stopwatch!!!\n\n");
    printf(" Result\n");
    printf(" - tikcs/ 1[sec]  = %lu\n", Time );
    printf(" - tikcs/ 1[msec] = %lu\n", Time/1000 );
    
    printf("\n\n [cfe_msleep] Start Stopwatch for 1[msec]!!!\n");
    StartTime = _getticks( );
    cfe_usleep(1000 * 1 );
    EndTime = _getticks( );
    Time = (uint32_t)(EndTime - StartTime);
    printf(" [cfe_msleep] Stop Stopwatch!!!\n\n");
    printf(" Result\n");
    printf(" - tikcs/ 1[msec]  = %lu\n", Time );
    printf("------------------------------\n");
}
#endif
//-------------------------------------------------
//===========================================================

