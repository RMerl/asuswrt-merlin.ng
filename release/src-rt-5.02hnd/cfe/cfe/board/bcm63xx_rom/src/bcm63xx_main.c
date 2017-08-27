/*
<:label-BRCM:2011:NONE:standard

     :> 
*/

/*  *********************************************************************
    *  Broadcom Common Firmware Environment (CFE)
    *  
    *  Main Module              File: bcm63xxBoot_main.c       
    *  
    *  This module contains the main "C" routine for CFE bootstrap loader 
    *  and decompressor to decompress the real CFE to ram and jump over.
    *
    *  Author:  Mitch Lichtenberg (mpl@broadcom.com)
    *  Revised: seanl
    *  
    *********************************************************************  
    *
    <:label-BRCM:2012:proprietary:standard
    *
    *  Copyright 2000,2001,2002,2003
    *  Broadcom Corporation. All rights reserved.
    *  
    *  This software is furnished under license and may be used and 
    *  copied only in accordance with the following terms and 
    *  conditions.  Subject to these conditions, you may download, 
    *  copy, install, use, modify and distribute modified or unmodified 
    *  copies of this software in source and/or binary form.  No title 
    *  or ownership is transferred hereby.
    *  
    *  1) Any source code used, modified or distributed must reproduce 
    *     and retain this copyright notice and list of conditions 
    *     as they appear in the source file.
    *  
    *  2) No right is granted to use any trade name, trademark, or 
    *     logo of Broadcom Corporation.  The "Broadcom Corporation" 
    *     name may not be used to endorse or promote products derived 
    *     from this software without the prior written permission of 
    *     Broadcom Corporation.
    *  
    *  3) THIS SOFTWARE IS PROVIDED "AS-IS" AND ANY EXPRESS OR
    *     IMPLIED WARRANTIES, INCLUDING BUT NOT LIMITED TO, ANY IMPLIED
    *     WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR 
    *     PURPOSE, OR NON-INFRINGEMENT ARE DISCLAIMED. IN NO EVENT 
    *     SHALL BROADCOM BE LIABLE FOR ANY DAMAGES WHATSOEVER, AND IN 
    *     PARTICULAR, BROADCOM SHALL NOT BE LIABLE FOR DIRECT, INDIRECT,
    *     INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
    *     (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
    *     GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
    *     BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY 
    *     OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR 
    *     TORT (INCLUDING NEGLIGENCE OR OTHERWISE), EVEN IF ADVISED OF 
    *     THE POSSIBILITY OF SUCH DAMAGE.
    :> 
    ********************************************************************* */


#include "lib_types.h"
#include "lib_string.h"
#include "lib_printf.h"

#include "cfe_iocb.h"
#include "cfe_device.h"
#include "cfe_console.h"
#include "cfe_timer.h"

#include "env_subr.h"
#include "ui_command.h"
#include "cfe_mem.h"
#include "cfe.h"

#include "bsp_config.h"
#include "cpu_config.h"
#include "bcm_hwdefs.h"
#include "bcm_map.h"
#include "boardparms.h"
#include "rom_parms.h"
#include "bcm63xx_ipc.h"

#include "exception.h"
#include "segtable.h"
#include "initdata.h"
#include "lib_printf.h"
#include "lib_byteorder.h"

#if defined(_BCM96362_) ||  defined(_BCM96328_) ||    \
 defined (_BCM963268_) ||    \
   defined (_BCM96838_) ||    \
    defined (_BCM963138_) ||    \
     defined (_BCM963381_) ||    \
     \
 defined (_BCM963148_) || defined (_BCM96848_) || defined (_BCM94908_) || defined (_BCM96858_)  || defined (_BCM947189_) || defined (_BCM968360_)
#include "flash_api.h"
#include "jffs2.h"
#include "bcm_ubi.h"
#endif

extern uint64_t _getticks(void);

#if (INC_PMC_DRIVER==1)
#include "pmc_drv.h"
#include "clk_rst.h"
#include "BPCM.h"
#endif

#if CFG_PCI
#include "pcivar.h"
#endif

#if (BOOT_PRE_CFE==1)
#include "bcm_pinmux.h"
#endif
#include "bcm_otp.h"

#ifndef INC_BTRM_BOOT
#define INC_BTRM_BOOT         0
#endif

extern void board_setleds(uint32_t);
#if defined (_BCM963138_) || defined (_BCM963148_) || defined (_BCM94908_)
#if defined(_BCM94908_)
int fast_cpu_clock = 0;
#endif
extern unsigned long rom_option;
extern int ddr_init(unsigned long mcb_selector);
#endif

#if defined(_BCM96858_) || defined(_BCM968360_)
int cpu_clock = 0;
extern int bcm_otp_get_cpu_clk(uint32* val);
extern int ddr_init(unsigned long mcb_selector);
#endif

#if defined(_BCM963268_) || defined(_BCM96838_) || defined(_BCM963138_) || defined(_BCM963148_) || (defined(_BCM963381_) && (INC_BTRM_BOOT==1)) || defined(_BCM94908_) || defined(_BCM96858_) || defined(_BCM968360_)
#include "bcm_common.h"
#include "btrm_if.h"

// Struct for capturing args passed down by CFE BTRM
Booter1Args bootArgs;
#endif

#if defined (_BCM963381_)
static void apply_ddr_ssc(void);
#if (INC_NAND_FLASH_DRIVER==1)
static void bump_nand_phase(int n);
#endif
#endif

#if defined(_BCM96858_) || defined(_BCM94908_)
extern void  armv8_disable_mmu(void);
#endif


int cfe_size_ram(void);
inline static int is_aliased(int max_bits) __attribute__((always_inline));

/*  *********************************************************************
    *  Constants
    ********************************************************************* */

#ifndef CFG_STACK_SIZE
#define STACK_SIZE  8192
#else
#define STACK_SIZE  ((CFG_STACK_SIZE+1023) & ~1023)
#endif

inline static int is_aliased(int max_bits)
{
    volatile uint32 *mem_base;
    volatile uint32 *test_ptr;
    uint32 tmp;
    int bit;
    int res = 0;

    mem_base = (uint32*)DRAM_BASE_NOCACHE;

    *mem_base = 0;
    
    for (bit = 8; bit < max_bits; bit++) {
        test_ptr = (uint32*)((uintptr_t)mem_base | 1 << bit);
        /* ram may contain useful data, save location before modifying */
        tmp = *test_ptr;
        *test_ptr = -1;
        if (*mem_base == *test_ptr) {
            *test_ptr = tmp;
            res = 1;
            break;
        }
        *test_ptr = tmp;
    }
    return res;
}

#define MEMC_MAX_ROWS   14

void stop_cferom(void);
void stop_cferom(void)
{
    while(1);
}


int cfe_size_ram(void)
{
#if defined(IKOS_NO_DDRINIT)
    return 64 << 10;    /* assuming 64MB memory for IKOS */
#endif
#if defined(_BCM96328_) || defined (_BCM96362_) || defined(_BCM96816_) || defined(_BCM96828_)
    return (DDR->CSEND << 24);
#elif defined(_BCM96318_) || defined(_BCM960333_) || defined(_BCM963381_)
    uint32 memCfg;

#ifdef _BCM963381_
    memCfg = MEMC->SDR_CFG.SDR_CFG;
#else
    memCfg = MEMC->SDR_CFG;
#endif

    memCfg = (memCfg&MEMC_SDRAM_SPACE_MASK)>>MEMC_SDRAM_SPACE_SHIFT;

    return 1<<(memCfg+20);
#elif defined(_BCM963138_) || defined (_BCM963148_) || defined (_BCM96848_)  || defined(_BCM94908_) || defined (_BCM96858_)
    return 1<<(((MEMC->GLB_GCFG&MEMC_GLB_GCFG_SIZE1_MASK)>>MEMC_GLB_GCFG_SIZE1_SHIFT)+20);
#elif defined(_BCM968360_)
    return 1<<(((MEMC->CHN_CFG_DRAM_SZ_CHK&CHN_CFG_DRAM_SZ_CHK_MASK)>>CHN_CFG_DRAM_SZ_CHK_SHIFT)+20);
#elif defined(_BCM947189_)
    /*
     * 47189: Complete this with a suitable implementation if necessary (this
     * function is not really useful at all)
     */
    return 0;
#else
    return (MEMC->CSEND << 24);
#endif
}

// fake functions for not to modifying init_mips.S
void _exc_entry(void);
void cfe_command_restart(void);
void cfe_doxreq(void);

void _exc_entry(void)
{
}

void cfe_command_restart(void)
{
}
void cfe_doxreq(void)
{
}

/*  *********************************************************************
    *  Externs
    ********************************************************************* */
void cfe_main(int,int);
int pll_init(void);
#if defined(_BCM96848_)
extern uint32_t clks_per_usec;
extern uint32_t otp_get_max_clk_sel(void);
#endif
/*  *********************************************************************
    *  Globals
    ********************************************************************* */

void cfe_ledstr(const char *leds)
{
}

extern int board_puts(const char*);

extern void _binArrayStart(void);
extern void _binArrayEnd(void);
extern int decompressLZMA(unsigned char *in, unsigned insize, unsigned char *out, unsigned outsize);

#if defined(_BCM963381_)
static void apply_ddr_ssc(void)
{
    uint32_t val;

    val = 0x1003200;
    WriteBPCMRegister(PMB_ADDR_SYSPLL, BPCMRegOffset(pwd_accum_control), val);
    val = 0x7AE1;
    WriteBPCMRegister(PMB_ADDR_SYSPLL, BPCMRegOffset(sr_control), val);

    cfe_usleep(10);

    val = 0x80007AE1;
    WriteBPCMRegister(PMB_ADDR_SYSPLL, BPCMRegOffset(sr_control), val);

    return;
}
#endif

#if (INC_NAND_FLASH_DRIVER==1) || (INC_SPI_PROG_NAND==1) || (BOOT_PRE_CFE==1)
extern int nand_flash_get_sector_size(unsigned short sector);
extern int nand_flash_get_numsectors(void);
extern int nand_flash_read_buf(unsigned short blk, int offset,
    unsigned char *buffer, int len);
extern void rom_nand_flash_init(void);
#if defined(_BCM963138_) || defined (_BCM963148_) || defined(_BCM94908_) || defined (_BCM96858_) || defined(_BCM968360_)
#define flash_get_sector_size nand_flash_get_sector_size 
#endif
#endif

#if ((INC_NAND_FLASH_DRIVER==1) || (BOOT_PRE_CFE==1)) && !defined(JTAG_DOWNLOAD)
static int strap_check_nand(void)
{

#if (defined(_BCM963138_) && !(INC_SPI_PROG_NAND==1))
    return 1; // special case for chip bug for BCM963138 
#elif defined(_BCM947189_)
        GCI_BASE->status_idx_reg_wr=GCI_CHIP_STATUS_REGISTER_INDEX_7;
        if((GCI_BASE->status_idx_reg_rd & BOOT_MODE_MASK) == BOOT_MODE_NAND)
                return 1;
	else
		return 0;
#elif (defined(_BCM96362_) ||   defined(_BCM96328_) ||    defined (_BCM963268_))
    return ( ((MISC->miscStrapBus & MISC_STRAP_BUS_BOOT_SEL_MASK) >>
        MISC_STRAP_BUS_BOOT_SEL_SHIFT) == MISC_STRAP_BUS_BOOT_NAND );

#elif defined(_BCM96838_)
    return ( (((GPIO->strap_bus & GPIO_PER_STRAP_BUS_BOOT_CFG_MASK) >> GPIO_PER_STRAP_BUS_BOOT_CFG_SHIFT) != GPIO_PER_STRAP_BUS_SPI_3_BOOT) &&
             (((GPIO->strap_bus & GPIO_PER_STRAP_BUS_BOOT_CFG_MASK) >> GPIO_PER_STRAP_BUS_BOOT_CFG_SHIFT) != GPIO_PER_STRAP_BUS_SPI_4_BOOT)  );

#elif (defined(_BCM963138_) || defined(_BCM963381_) || defined(_BCM963148_))
    return ((MISC->miscStrapBus&MISC_STRAP_BUS_BOOT_SPI_NOR) != MISC_STRAP_BUS_BOOT_SPI_NOR);
#elif defined(_BCM96848_)
    return ((MISC->miscStrapBus&MISC_STRAP_BUS_BOOT_SEL_MASK) != MISC_STRAP_BUS_BOOT_SPI_NOR);
#elif defined(_BCM94908_) || defined(_BCM968360_)
    return ((MISC->miscStrapBus&MISC_STRAP_BUS_BOOT_SEL_NAND_MASK) == MISC_STRAP_BUS_BOOT_NAND);
#elif defined(_BCM96858_)
    {
        uint32 bootsel = ((MISC->miscStrapBus & MISC_STRAP_BUS_BOOT_SEL0_4_MASK) >> MISC_STRAP_BUS_BOOT_SEL0_4_SHIFT) |
                         ((MISC->miscStrapBus & MISC_STRAP_BUS_BOOT_SEL5_MASK) >> BOOT_SEL5_STRAP_ADJ_SHIFT);

        if ( ((bootsel & BOOT_SEL_STRAP_BOOT_SEL_MASK) != BOOT_SEL_STRAP_SPI_NOR) &&
             ((bootsel & BOOT_SEL_STRAP_BOOT_SEL_MASK) != BOOT_SEL_STRAP_EMMC) )
            return 1;
        else
            return 0;
    }
#else
    return 0;

#endif 
}

#if (INC_SPI_NAND_DRIVER==1)
extern int spi_nand_init(flash_device_info_t **flash_info);
extern int spi_nand_get_sector_size(unsigned short sector);
extern int spi_nand_get_numsectors(void);

extern int spi_nand_read_buf(unsigned short blk, int offset, unsigned char *buffer, int len);
extern void rom_spi_nand_init(void);

int strap_check_spinand(void);
int strap_check_spinand(void)
{
#if defined(_BCM96838_)
    return ( (((GPIO->strap_bus & GPIO_PER_STRAP_BUS_BOOT_CFG_MASK) >> GPIO_PER_STRAP_BUS_BOOT_CFG_SHIFT) == GPIO_PER_STRAP_BUS_SPI_4_BOOT) &&
             (((GPIO->strap_bus & GPIO_PER_STRAP_BUS_PAGE_SIZE_MASK) >> GPIO_PER_STRAP_BUS_PAGE_SIZE_SHIFT) & GPIO_PER_STRAP_BUS_SPI_NAND_BOOT) );

#elif defined(_BCM963381_)
    return ((MISC->miscStrapBus&MISC_STRAP_BUS_SPI_NAND_DISABLE) == 0);

#elif defined(_BCM963138_)
    return ( (MISC->miscStrapBus & MISC_STRAP_BUS_SW_BOOT_SPI_SPINAND_EMMC_MASK) &&
            ((MISC->miscStrapBus & MISC_STRAP_BUS_BOOT_OPT_MASK) == MISC_STRAP_BUS_BOOT_SPI_NAND) );

#elif defined(_BCM96848_)
    return (MISC->miscStrapBus & MISC_STRAP_BUS_BOOT_SPI_NAND);

#elif defined(_BCM94908_) || defined(_BCM968360_)
    return ((MISC->miscStrapBus&MISC_STRAP_BUS_BOOT_SEL_MASK) == MISC_STRAP_BUS_BOOT_SPI_NAND);
#elif defined(_BCM96858_)
    {
        uint32 bootsel = ((MISC->miscStrapBus & MISC_STRAP_BUS_BOOT_SEL0_4_MASK) >> MISC_STRAP_BUS_BOOT_SEL0_4_SHIFT) |
                         ((MISC->miscStrapBus & MISC_STRAP_BUS_BOOT_SEL5_MASK) >> BOOT_SEL5_STRAP_ADJ_SHIFT);

        return ((bootsel & BOOT_SEL_STRAP_BOOT_SEL_MASK) == BOOT_SEL_STRAP_SPI_NAND);
    }
#else
    return 0;

#endif
}
#endif
#endif


#if defined(_BCM96838_) && !(INC_NAND_FLASH_DRIVER==1)
extern int spi_flash_init(flash_device_info_t **flash_info);
extern int spi_flash_get_sector_size(unsigned short sector);
extern int spi_flash_read_buf(unsigned short sector, int offset, unsigned char *buffer, int nbytes);
#endif

#if ((INC_NAND_FLASH_DRIVER==1) || (BOOT_PRE_CFE==1)) && !defined(JTAG_DOWNLOAD)
/* Find uncompressed file cferam.bin on the JFFS2 file system, load it into
 * memory and jump to its entry point function.
 */

int nand_read_buf(unsigned short blk, int offset, unsigned char *buffer, int len);
int nand_read_buf(unsigned short blk, int offset, unsigned char *buffer, int len)
{
#if (INC_SPI_NAND_DRIVER==1)
    if (strap_check_spinand())
        return(spi_nand_read_buf(blk, offset, buffer, len));
#endif
    if (strap_check_nand())
        return(nand_flash_read_buf(blk, offset, buffer, len));

    return(FLASH_API_ERROR);
}

#if defined(_BCM963381_)
static void bump_nand_phase(int n) {
    int i;
    uint32_t val, val2;

    for (i = 0 ; i < n ; i++) {
        ReadBPCMRegister(PMB_ADDR_SYSPLL1, PLLBPCMRegOffset(ch01_cfg), &val);
        WriteBPCMRegister(PMB_ADDR_SYSPLL1, PLLBPCMRegOffset(ch01_cfg), ((uint32_t)val)|(0x1<<27));
        ReadBPCMRegister(PMB_ADDR_SYSPLL1, PLLBPCMRegOffset(ch01_cfg), &val2);
        WriteBPCMRegister(PMB_ADDR_SYSPLL1, PLLBPCMRegOffset(ch01_cfg), ((uint32_t)val));
    }
}
#endif

#if (BOOT_PRE_CFE==0)
extern unsigned int lib_get_crc32(unsigned char *pdata, unsigned int size, unsigned int crc);
#define getCrc32 lib_get_crc32

#define je16_to_cpu(x) ((x).v16)
#define je32_to_cpu(x) ((x).v32)

char g_fname[] = NAND_CFE_RAM_NAME;
int g_fname_actual_len = sizeof(g_fname) - 1;
int g_fname_cmp_len = sizeof(g_fname) - 4; /* last three are digits */

static unsigned int read_blk(unsigned char * start, unsigned int block, unsigned int offset, unsigned int blk_size, unsigned char * buf, unsigned int amount, void * mtd, int mtd_fd)
{
    return(nand_read_buf(block, offset, buf + offset, amount));
}

/*
*  A decision to override has to be made in a bit 29 of the parameter
*  1 to override 0 otherwise 
*  bits 0-15 hold an image selector
*  This functionality is for future implementation. 
*  Currently, when overriden 
*  previous image is forced to be selected
* 
*/
static void bootImageFromNand(unsigned long rom_param)
{
    const unsigned int bv_invalid = 0xfffffff;
    const int max_not_jffs2 = MAX_NOT_JFFS2_VALUE;
    int read_len;

    struct rootfs_info
    {
        int rootfs;
        int start_blk;
        int end_blk;
        int ubi;
        int committed;
        unsigned int boot_val;
        unsigned int ino;
        unsigned int blk;
        unsigned int mctime;
    } rfs_info[2], *prfs_info[2], *rfsi;

#if (CFG_COPY_PSRAM==1)
    unsigned char *buf = (unsigned char *) ((mem_heapstart & 0x0000ffff) | DRAM_BASE);
#else
#if defined(_BCM963148_) || defined(_BCM963138_) || (defined(_BCM963381_) && (INC_BTRM_BOOT==1)) || defined(_BCM94908_) || defined(_BCM96858_) || defined(_BCM968360_)
    /* 631x8 run cfe rom in internal memory which is only 512KB, switch heap to ddr 
       in case nand block size is 512KB or larger */
    unsigned char *buf = (unsigned char *) DRAM_BASE + 0x1000;
#else
    unsigned char *buf = (unsigned char *) mem_heapstart;
#endif
#endif

    uint32_t version = 0;
    struct jffs2_raw_dirent *pdir;
    struct jffs2_raw_inode *pino;
    unsigned char *p;
    int i, j, k, done, not_jffs2;
    int num_blks;
    int len, try = 0, blen = 0;
    int boot_prev;
    PNVRAM_DATA nd;
    char *bootline;
    struct ubi_ec_hdr *ec = (struct ubi_ec_hdr *) buf;

#if defined(_BCM963381_)
    /* apply the NAND phase bump patch */    
    board_setleds(0x4E44434B);  //NDCK
    bump_nand_phase(16);
#endif


#if defined(_BCM94908_) || defined(_BCM96858_) || defined(_BCM968360_)  || defined(_BCM963268_) || defined(_BCM96838_) || defined(_BCM963138_) || defined (_BCM963148_) || (defined(_BCM963381_) && (INC_BTRM_BOOT==1))
    /* If secure boot is in play, the CFE RAM file name is different */
    int boot_secure = bcm_otp_is_boot_secure();
    if (boot_secure)
       strcpy(g_fname, NAND_CFE_RAM_SECBT_NAME);
#if defined(_BCM94908_) || defined(_BCM96858_) || defined(_BCM968360_)
    else
    {
       /* Gen3 bootrom. If secure boot is in play, it can be mfg-secure or field-secure, hence two CFE RAM names */
       int boot_mfg_secure = bcm_otp_is_boot_mfg_secure();
       if (boot_mfg_secure)
       {
          strcpy(g_fname, NAND_CFE_RAM_SECBT_MFG_NAME);
	  boot_secure = 1;
       }
    }
#endif
#endif


#if (INC_SPI_NAND_DRIVER==1)
    if (strap_check_spinand())
    {
        num_blks = spi_nand_get_numsectors();
        len = spi_nand_get_sector_size(0);
    }
    else
#endif
    {
        num_blks = nand_flash_get_numsectors();
        len = nand_flash_get_sector_size(0);
    }

    nd =  (PNVRAM_DATA) (buf + len);

    nand_read_buf(NVRAM_SECTOR, NVRAM_DATA_OFFSET, (unsigned char *)nd, sizeof(NVRAM_DATA));

    /* The partition to boot from is stored in the /data partition in a file
     * named boot_state_X where X indicates current or previous partition.
     * In order to not search /data and keep rom in 16KB, we use bootline for simplification 
     * This covers all the cases except where the bootstate is BOOT_SET_NEW_IMAGE_ONCE and 
     * bootline is boot previous. The cferam will boot the appropriate (current or
     * previous) Linux image.
     */
    boot_prev = 0;
    board_setleds(0x46505330 | NAND_FULL_PARTITION_SEARCH); // FPS0
    bootline = nd->szBootline;
    /* search for p='x' */
    if( (rom_param&NAND_IMAGESEL_OVERRIDE) == NAND_IMAGESEL_OVERRIDE)  {
	boot_prev = 1;
    } else {
    	while( *bootline && blen < NVRAM_BOOTLINE_LEN )
    	{
          	if ((*bootline == 'p') && (*(bootline+1) == '='))
                {
                    boot_prev = (*(bootline+2) == '1') ? 1:0;
                    break;
                }
                else
                {
                    bootline++;
                    blen++;
                }
       }
    }

    for( k = 0, rfsi = rfs_info; k < 2; k++, rfsi++ )
    {
        unsigned int type = 0; // image type

        board_setleds(0x2D2D2D2D);  // ----
        board_setleds(0x50415230 | k);  // PAR# searching partition

        version = 0;
        not_jffs2 = 0;
        rfsi->rootfs = k + NP_ROOTFS_1;
        rfsi->boot_val = bv_invalid;
        rfsi->ubi = 0;
        rfsi->committed = 0;

        if( nd->ulNandPartOfsKb[rfsi->rootfs] > 0 &&
            nd->ulNandPartOfsKb[rfsi->rootfs] < ((num_blks * len) / 1024))
        {
            rfsi->start_blk = nd->ulNandPartOfsKb[rfsi->rootfs] / (len/1024);
            rfsi->end_blk = rfsi->start_blk +
                (nd->ulNandPartSizeKb[rfsi->rootfs] / (len / 1024));
        }
        else
            rfsi->start_blk = rfsi->end_blk = 0;

        if( rfsi->start_blk == 0 || rfsi->start_blk >= rfsi->end_blk ||
            rfsi->start_blk >= num_blks || rfsi->end_blk >= num_blks )
        {
            /* NVRAM_DATA fields for this rootfs are not valid. */
            if( k == 1 )
            {
                /* Skip this rootfs. */
                // board_setleds(0x4e414e36); // NAN6
                continue;
            }

            if( rfs_info[0].boot_val == bv_invalid )
            {
                /* File system info cannot be found for either rootfs.
                 * NVRAM_DATA may not be set.  Use default values.
                 */
                // board_setleds(0x4e414e37); // NAN7
                rfsi = rfs_info;
                rfsi->start_blk = 1;
                rfsi->end_blk = num_blks;
            }
        }

        /* Find the directory entry. */
        for( i = rfsi->start_blk, done = 0; i < rfsi->end_blk && done == 0; i++ )
        {
            /* This loop sequentially reads a NAND flash block into memory and
             * processes it.  First, read a little bit to verify that it is a
             * JFFS2 block.  If it is, read the entire block.
             */
            if( (read_len = nand_read_buf(i, 0, buf, 512)) > 0 )
            {
                pdir = (struct jffs2_raw_dirent *) buf;

                if( je16_to_cpu(pdir->magic) == JFFS2_MAGIC_BITMASK )
                {
                    if (!type)
                        type = JFFS2_IMAGE;

                    read_len = nand_read_buf(i, 0, buf, len);

                    /* This loop reads inodes in a block. */
                    p = buf;
                    while( p < buf + len )
                    {
                        pdir = (struct jffs2_raw_dirent *) p;
                        if( je16_to_cpu(pdir->magic) == JFFS2_MAGIC_BITMASK )
                        {
                            if( je16_to_cpu(pdir->nodetype) ==
                                    JFFS2_NODETYPE_DIRENT &&
                                g_fname_actual_len == pdir->nsize &&
                                !memcmp(g_fname, pdir->name, g_fname_cmp_len) &&
                                je32_to_cpu(pdir->version) > version &&
                                je32_to_cpu(pdir->ino) != 0 )
                            {
                                /* The desired directory was found. */
                                unsigned char *fname =
                                    pdir->name + g_fname_cmp_len;
                                rfsi->boot_val =
                                    ((fname[0] - '0') * 100) +
                                    ((fname[1] - '0') *  10) +
                                    ((fname[2] - '0') *   1);
                                version = je32_to_cpu(pdir->version);
                                rfsi->mctime = je32_to_cpu(pdir->mctime);
                                rfsi->ino = je32_to_cpu(pdir->ino);
                                rfsi->blk = i;
                                /* Setting 'done = 1' assumes there is only
                                 * one version of the directory entry.
                                 * There will be more than one version if
                                 * the file (cferam.xxx) is renamed,
                                 * modified, copied, etc after it was
                                 * initially flashed.
                                 */
                                board_setleds(0x4A000000 | (fname[0] << 16) | (fname[1] << 8) | fname[2]); // J###
                                done = NAND_FULL_PARTITION_SEARCH ^ 1;
                                if( done )
                                {
                                    board_setleds(0x4A464653); // JFFS
                                    break;
                                }
                            }

                            p += (je32_to_cpu(pdir->totlen) + 0x03) & ~0x03;
                            not_jffs2 = 0;
                        }
                        else
                            break;
                    }
                }
                else if ( (be32_to_cpu(ec->magic) == UBI_EC_HDR_MAGIC) && (getCrc32((void *)ec, UBI_EC_HDR_SIZE-4, -1) == be32_to_cpu(ec->hdr_crc)) )
                {
                    char string[3];

                    if (!type)
                        type = UBI_IMAGE;

                    not_jffs2 = 0;
                    rfsi->ubi = 1;

                    for (j = 0; j < 2; j++)
                    {
                        if (j)
                           try = VOLID_METADATA_COPY;
                        else
                           try = VOLID_METADATA;
                        
                        if (parse_ubi(0, buf, i, rfsi->end_blk, len, try, g_fname, string, 0, 0, read_blk, 0, 0, 0, 0) == 3)
                        { // successfully retrieved sequence number
                            rfsi->boot_val = ((string[0] - '0') * 100) + ((string[1] - '0') * 10) + (string[2] - '0');
                            board_setleds(0x55000000 | (string[0] << 16) | (string[1] << 8) | string[2]); // U###

                            if (parse_ubi(0, buf, i, rfsi->end_blk, len, try, "committed", string, 0, 0, read_blk, 0, 0, 0, 0) == 1)
                            { // if either copy of metadata is committed then we are committed
                                rfsi->committed |= (string[0] == '1') ? 1 : 0;
                                board_setleds(0x434F4D00 | string[0]); // COM#
                            }
                        }
                    }

                    board_setleds(0x55424923);  // UBI#
                    BOARD_SETLED_HEX16(rfsi->boot_val);
                    rfsi->blk = i;

                    done = 1;
                }
                else
                {
                    if( (max_not_jffs2 > 0) && (not_jffs2++ > max_not_jffs2) )
                    {
                        /* No JFFS2 magic bitmask for consecutive blocks.
                         * Assume this partion does not have a file system
                         * on it.
                         */
                        board_setleds(0x4E4F4E45); // NONE
                        break;
                    }
                }
            }
            else
            {
                read_len = 0;
            }
        }

        if( rfsi->boot_val != bv_invalid )
        {
            board_setleds(0x42540000 + // BT
                ((((((rfsi->boot_val % 100) / 10) + '0') << 8)) |
                 (((rfsi->boot_val % 10) + '0'))));
            board_setleds(
                (((rfsi->blk / 1000) + '0') << 24) |
                ((((rfsi->blk % 1000) / 100) + '0') << 16) |
                (((((rfsi->blk % 1000) % 100) / 10) + '0') << 8) |
                (((((rfsi->blk % 1000) % 100) % 10) + '0') << 0));
        }
        else
            board_setleds(0x4e414e39); // NAN9
    }

    board_setleds(0x2D2D2D2D);  // ----

    /* Set the rfs_info to the index to boot from. */
    /* Deal with wrap around case */
    if (rfs_info[0].boot_val == 0 && rfs_info[1].boot_val == 999)
        rfs_info[0].boot_val = 1000;
    if (rfs_info[1].boot_val == 0 && rfs_info[0].boot_val == 999)
        rfs_info[1].boot_val = 1000;

    if(rfs_info[0].boot_val > rfs_info[1].boot_val)
       try = 0;
    else
       try = 1;

    /* boot sequence priority */
    if (rfs_info[try].ubi)
    { // pure UBI image
//#if defined(_BCM96838_) || defined(_BCM96848_)
//        unsigned int * software_debug_1 = (unsigned int *)0x14e013e4;
//#endif

// boot latest image if:
//   it's committed or
//   was written by an image writer that didn't understand the format (998 is default sequence number) or
//   there is a soft reset boot latest non-committed image override
        if ( rfs_info[try].committed || (rfs_info[try].boot_val == 998)
//#if defined(_BCM96838_) || defined(_BCM96848_)
//            || (*software_debug_1 == 0xdabedabe)
// #endif
            )
        {
            boot_prev = 0;
//#if defined(_BCM96838_) || defined(_BCM96848_)
//            *software_debug_1 = 0;
//            board_setleds(0x4F565244);  // OVRD
//#endif
        }
        else
        { // no boot priority override
            boot_prev = 1;
        }
    }

    if(((boot_prev == 0 && rfs_info[0].boot_val > rfs_info[1].boot_val) ||
      (boot_prev == 1 && rfs_info[0].boot_val < rfs_info[1].boot_val) ||
       rfs_info[1].boot_val == bv_invalid ))
    {
        /* Boot from the most recent image. */
        prfs_info[0] = &rfs_info[0];
        prfs_info[1] = &rfs_info[1];
        try = 0;
    }
    else
    {
    /* Boot from the previous image. */
        prfs_info[0] = &rfs_info[1];
        prfs_info[1] = &rfs_info[0];
        try = 1;
    }

    /* If the directory entry for the desired file, which is the CFE RAM image,
     * is found, read it into memory and jump to its entry point function.
     * This loop checks for CFE RAM image in two possible locations/partitions/rootfs file sytems.
     */

    for( k = 0; k < 2; k++ )
    {
        unsigned char *pucDest = NULL;
        unsigned char *pucEntry = NULL;
        uint32_t isize = 0;
#if defined(_BCM96838_) || defined(_BCM94908_) || defined(_BCM96858_) || defined(_BCM968360_) || ((INC_BTRM_BOOT==1) && (defined(_BCM963268_) || defined(_BCM963381_) || defined(_BCM963138_) || defined(_BCM963148_)))
        uint32_t secCfeRamSize = 0;
#endif
        board_setleds(0x54525930 | try);  // TRY# searching partition

        board_setleds(0x4e414e33); // NAN3
        rfsi = prfs_info[k];
        if( rfsi->boot_val == bv_invalid )
            continue;

        /* When j == 0, get the first inode to find the entry point address.
         * When j == 1, read the file contents into memory.
         */
        if (!rfsi->ubi)
        {
            for( j = 0; j < 2; j++ )
            {
                /* This loop sequentially reads a NAND flash block into memory and
                 * processes it.
                 */
                board_setleds(0x4A465332);  // JFS2

                for(i = rfsi->start_blk, done = 0; i<rfsi->end_blk && done==0; i++)
                {
                    if( nand_read_buf(i, 0, buf, len) > 0 )
                    {
                        /* This loop reads inodes in a block. */
                        p = buf;

                        while( p < buf + len )
                        {
                            /* Verify the first short word is the JFFS2 magic
                             * number.
                             */

                            pino = (struct jffs2_raw_inode *) p;
                            if( je16_to_cpu(pino->magic) == JFFS2_MAGIC_BITMASK )
                            {
                                if( je16_to_cpu(pino->nodetype) ==
                                        JFFS2_NODETYPE_INODE &&
                                    je32_to_cpu(pino->ino) == rfsi->ino )
                                {
                                    uint32_t size = je32_to_cpu(pino->dsize);
                                    uint32_t ofs = je32_to_cpu(pino->offset);
                                    uint32_t mtime = je32_to_cpu(pino->mtime);

                                    if( size && mtime == rfsi->mctime )
                                    {
                                        /* A node of the CFE RAM file was found
                                         * with data. */
                                        if( pucDest == NULL )
                                        {
                                            /* The entry point and copy destination
                                             * addresses have not been obtained.
                                             * If this is the first node of the CFE
                                             * RAM file, obtain this information.
                                             */
                                            if( ofs == 0 )
                                            {
                                                /* The first 12 bytes contain a
                                                 * header.  The first word is the
                                                 * entry point address.
                                                 */
                                                memcpy(&pucEntry, pino->data, 4);

                                                pucDest = pucEntry - 12;

                                                isize = je32_to_cpu(pino->isize);

#if defined(_BCM96838_) || defined(_BCM94908_) || defined(_BCM96858_) || defined(_BCM968360_) || ((INC_BTRM_BOOT==1) && (defined(_BCM963268_) || defined(_BCM963381_) || defined(_BCM963138_) || defined(_BCM963148_)))
                                                secCfeRamSize = isize;
                                                /* If secure boot, compressed, encrypted CFE RAM */
                                                /* is authenticated within internal memory       */
                                                if (boot_secure)
                                                   pucDest = (unsigned char *)BTRM_INT_MEM_ENCR_COMP_CFE_RAM_ADDR;
#endif
                                                done = 1;
                                                board_setleds(0x52465330 | // RFS0
                                                    rfsi->rootfs);
                                                board_setleds(0x4A464653); // JFFS
                                                break;
                                            }
                                        }
                                        else
                                        {
                                            /* Copy the image to memory. Stop when
                                             * the entire image has been copied.
                                             */
                                            memcpy(pucDest+ofs, pino->data, size);
                                            if( (isize -= size) <= 0 )
                                            {
                                                done = 1;
                                                break;
                                            }
                                        }
                                    }
                                }
    
                                /* Skip to the next inode entry. */
                                p += (je32_to_cpu(pino->totlen) + 0x03) & ~0x03;
                            }
                            else
                                break;
                        }
                    }
                    else
                        board_setleds(0x45525231); // ERR1
                }
            }
        }
        else
        { // process UBI
            board_setleds(0x55424921);  // UBI!

            parse_ubi(0, buf, rfsi->start_blk, rfsi->end_blk, len, VOLID_UBIFILES, g_fname, (char *)&pucEntry, 0, 4, read_blk, 0, 0, 0, 0);

#if defined(_BCM96838_) || defined(_BCM94908_) || defined(_BCM96858_) || defined(_BCM968360_) || ((INC_BTRM_BOOT==1) && (defined(_BCM963268_) || defined(_BCM963381_) || defined(_BCM963138_) || defined(_BCM963148_)))
            // If secure boot, compressed, encrypted CFE RAM
            // is authenticated within internal memory
            if (boot_secure)
                pucDest = (unsigned char *)BTRM_INT_MEM_ENCR_COMP_CFE_RAM_ADDR;
            else
#endif
                pucDest = pucEntry - 12;

            if (!parse_ubi(0, buf, rfsi->start_blk, rfsi->end_blk, len, VOLID_UBIFILES, g_fname, (char *)pucDest, 0, 0, read_blk, 0, 0, 0, 0))
                pucEntry = 0; // signify an error
        }

#if defined(_BCM96838_) || defined(_BCM94908_) || defined(_BCM96858_) || defined(_BCM968360_) || ((INC_BTRM_BOOT==1) && (defined(_BCM963268_) || defined(_BCM963381_) || defined(_BCM963138_) || defined(_BCM963148_)))
        if (boot_secure)
        {
           /* Retrieve the security materials */
           memcpy((void *)&bootArgs, (void *)BTRM_INT_MEM_CREDENTIALS_ADDR, sizeof(Booter1Args));

           /* Authenticate the CFE RAM bootloader */
           board_setleds(0x42544c3f); // BTL?
           uint8_t *pEncrCfeRam = (uint8_t *)authenticate((uint8_t *)pucDest, secCfeRamSize, bootArgs.authArgs.manu);
           board_setleds(0x42544c41); // BTLA
           board_setleds(0x50415353); // PASS

           /* Move pucDest to point to where the decrypted (but still compressed) CFE RAM will be put */
           pucDest = (unsigned char *)(BTRM_INT_MEM_COMP_CFE_RAM_ADDR);

           /* Get ready to decrypt the CFE RAM bootloader */
           /* decryptWithEk() will whack the content of the iv structure, therefore create a copy and pass that in */
           unsigned char origIv[CIPHER_IV_LEN];
           memcpy((void *)origIv, (void *)bootArgs.encrArgs.biv, CIPHER_IV_LEN);

           /* Decrypt the CFE RAM bootloader */
           decryptWithEk(pucDest, (unsigned char *)(&pEncrCfeRam[0]), bootArgs.encrArgs.bek,
                        (uint32_t)(secCfeRamSize-SEC_S_SIGNATURE), origIv);

           /* The reference sw is done with the bek/biv at this point ... cleaning it up */
           /* Any remnants of the keys on the stack will be cleaned up when cfe_launch() runs */
           ek_iv_cleanup(&bootArgs.encrArgs);
           memset((void *)origIv, 0, CIPHER_IV_LEN);

           /* First 12 bytes are not compressed ... First word of the 12 bytes is the address the cferam is linked to run at */
           /* Note: don't change the line below by adding uintptr_t to make it arch32 and arch64 compatible */
	   /* you want it to grab only the first 4 bytes of the 12 bytes in both cases */
           pucEntry = (unsigned char *) (unsigned long)(*(uint32_t *)BTRM_INT_MEM_COMP_CFE_RAM_ADDR);

           /* Decompress the image */
           decompressLZMA((unsigned char *)(BTRM_INT_MEM_COMP_CFE_RAM_ADDR+12), 
                          (unsigned int)(secCfeRamSize - SEC_S_SIGNATURE - CIPHER_BLK_LEN - 12),
                          pucEntry, 23*1024*1024); 
        }  
#endif

        if( pucEntry && isize <= 0 )
        {
            board_setleds(0x4e414e35); // NAN5

            /* 63138 and 63148 use 16KB below the cfe ram image as the mmu table,
             * so rfs number, cfe number has to be saved 16K down further */
            
           /* Save the rootfs partition that the CFE RAM image boots from
             * at the memory location before the CFE RAM load address. The
             * CFE RAM image uses this value to determine the partition to
             * flash a new rootfs to.
             */
            ROM_PARMS_SET_ROOTFS(pucEntry,(unsigned char)rfsi->rootfs);
            /* Save the sequence numbers so the CFE RAM image does not have
             * to find them again.
             */
            ROM_PARMS_SET_SEQ_P1(pucEntry, (rfs_info[0].boot_val & 0xffff) | NAND_SEQ_MAGIC);
            ROM_PARMS_SET_SEQ_P2(pucEntry, (rfs_info[1].boot_val & 0xffff) | NAND_SEQ_MAGIC);
            if( (rom_param&NAND_IMAGESEL_OVERRIDE) == NAND_IMAGESEL_OVERRIDE && boot_prev == 1)  {
                ROM_PARMS_SET_ROM_PARM(pucEntry, ((unsigned int)BOOT_SET_OLD_IMAGE)|NAND_IMAGESEL_OVERRIDE);
            } else {
                ROM_PARMS_SET_ROM_PARM(pucEntry, 0);
            }
#if defined(_BCM96838_) || defined(_BCM94908_) || defined(_BCM96858_) || defined(_BCM968360_) || ((INC_BTRM_BOOT==1) && (defined(_BCM963268_) || defined(_BCM963381_) || defined(_BCM963138_) || defined(_BCM963148_)))
            if (boot_secure)
            {
               /* Copy the authentication credentials from internal memory 
                * into ddr. Cferam on some targets (6838) uses the internal
                * memory. Therefore, linux kernel authentication has to use 
                * these credentials from ddr. Put the data above the 12 bytes
                * of info that were just placed above the cferam */
               ROM_PARMS_AUTH_PARM_SETM(pucEntry, &bootArgs.authArgs);
            }
#endif

            cfe_launch((unsigned long) pucEntry); // never return...
        }
        board_setleds(0x4e414e38); // NAN8
        try ^= 1;
    }

    /* Error occurred. */

#if defined(_BCM96838_) || defined(_BCM94908_) || defined(_BCM96858_) || defined(_BCM968360_) || ((INC_BTRM_BOOT==1) && (defined(_BCM963268_) || defined(_BCM963381_) || defined(_BCM963138_) || defined(_BCM963148_)))
    if (boot_secure)
    {
       /* Customer should zero out the bek and the biv at this point because the CFE RAM   */
       /* was not found. Hence, the flash is toast and JTAG needs to be enabled.           */
       ek_iv_cleanup(&bootArgs.encrArgs);

       /* Cleanup internal memory, and set the external interface enable bit high so that  */
       /* interfaces such as JTAG are suppose to be accessible, they become enabled        */
       cfe_launch(0);
    }
#endif

    board_setleds(0x44494530); // DIE0
    while(1);
}
#endif
#endif

/*  *********************************************************************
    *  cfe_main(a,b)
    *  
    *  It's gotta start somewhere.
    *  Input parameters: 
    *      a,b - not used
    *      
    *  Return value:
    *      does not return
    ********************************************************************* */
void cfe_main(int a,int b)
{
#if (CFG_BOOT_PSRAM==0)
    unsigned char *pucSrc;
    unsigned char *pucDst;
    unsigned int *entryPoint;   
    uintptr_t binArrayStart = (uintptr_t)_binArrayStart;
    uintptr_t binArrayEnd = (uintptr_t) _binArrayEnd;
    unsigned int dataLen = binArrayEnd - binArrayStart - 4;
    int ret;
#endif

#if defined(_BCM96848_)
    unsigned int mips_otp = (otp_get_max_clk_sel() & MISC_STRAP_CLOCK_SEL_400) ? 400 : 250;
    unsigned int mips_index = (((MISC->miscStrapBus & MISC_STRAP_CLOCK_SEL_MASK) >> MISC_STRAP_CLOCK_SEL_SHIFT) & MISC_STRAP_CLOCK_SEL_400) ? 400 : 250;
    clks_per_usec = (mips_index <= mips_otp) ? mips_index : mips_otp;
#endif

#if (CFG_ROM_PRINTF==1)
    xprinthook = board_puts;
#endif

#if defined(_BCM963381_)
    /* Make sure PMC is up running if using PMC */
    if ( (MISC->miscStrapBus & MISC_STRAP_BUS_PMC_ROM_BOOT)
#if (INC_NAND_FLASH_DRIVER==1)
        && !bcm_otp_is_btrm_boot() && !strap_check_spinand()
#endif
    )
        while( (((PMC->ctrl.hostMboxIn)>>24)&0x7) < 2 );

    pmc_init();
    apply_ddr_ssc();
#endif

#if defined(_BCM94908_)
    fast_cpu_clock = ((MISC->miscStrapBus)&MISC_STRAP_BUS_CPU_SLOW_FREQ) == 0 ? 1 : 0;
#elif defined(_BCM96858_) && !defined(CONFIG_BRCM_IKOS) && (BOOT_PRE_CFE==0)
    {
    unsigned int clk_index;
    if ( !bcm_otp_get_cpu_clk(&clk_index) )
        cpu_clock = 500 + 500*(2-clk_index);
    else
        cpu_clock = 0;
    }

    pmc_init();

    /* configure AXI clock */
    if (pll_ch_freq_set(PMB_ADDR_BIU_PLL, 2, 4))
        xprintf("Error: failed to set AXI clock\n");

    /* configure ubus clock */
    bcm_dcm_config(DCM_UBUS, 0x4, 0x2, 1);

    /* Change cpu to fast clock */
    if ((MISC->miscStrapBus)&MISC_STRAP_BUS_CPU_SLOW_FREQ)
    {
        int stat;
        PLL_CTRL_REG ctrl_reg;
        stat = ReadBPCMRegister(PMB_ADDR_BIU_PLL, PLLBPCMRegOffset(resets), &ctrl_reg.Reg32);
        ctrl_reg.Bits.byp_wait = 0;
        stat |= WriteBPCMRegister(PMB_ADDR_BIU_PLL, PLLBPCMRegOffset(resets), ctrl_reg.Reg32);
        if (stat)
            xprintf("Error: failed to set cpu fast mode\n");
    }
#elif defined(_BCM968360_) && !defined(CONFIG_BRCM_IKOS)
/* FIXMET: implement for 68360 */
#endif

#if !defined(_BCM947189_)
    cfe_mailbox_message_set(cfe_get_api_version());
    cfe_mailbox_status_set();
#endif
#if (INC_NAND_FLASH_DRIVER==1)
#if (INC_SPI_NAND_DRIVER==1)
    if (strap_check_spinand())
        rom_spi_nand_init();
    else
#endif
        rom_nand_flash_init();
#endif

/* read NAND cfe-rom from NAND flash */
#if (BOOT_PRE_CFE==1)
{
    unsigned int* buf = (unsigned int*)0x82620000; 
    unsigned int* src = (unsigned int*)0xffe1143c;
    unsigned int read_len = 0;
    int i;

    for (i=33; i<=48; i++)
        bcm_set_pinmux(33, 1);

    rom_nand_flash_init();
    NAND->NandCsNandXor = 1;

    while (read_len < 131072/4)
    {
        buf[read_len] = src[read_len];
        read_len++;
    }
    cfe_launch(0x82620000);
    while(1);
}
#endif

#if !defined(IKOS_NO_DDRINIT) && (defined (_BCM963138_) || defined (_BCM963148_) || defined (_BCM94908_))

#ifdef INC_DDR_DIAGS
    /* don't care about the shmoo result, always run diags */
extern void cde_main_entry(int value);
    ddr_init(rom_option);
#if defined(_BCM963138_)
    /* disable AIP fast ack as cde will modify the memc/phy config and want to take effect right away */
    ARMAIPCTRL->cfg &= ~0x2;
#endif
#if defined(_BCM963148_)
    /* disable ubus on the fly multiple read/write transaction */
    B15CTRL->cpu_ctrl.ubus_cfg &= ~0x70;
#endif
    cde_main_entry(0);
#if defined(_BCM963138_)
    ARMAIPCTRL->cfg |= 0x2;
#endif
#if defined(_BCM963148_)
    B15CTRL->cpu_ctrl.ubus_cfg |= 0x70;
#endif
#else /* INC_DDR_DIAGS */
#if !defined(IKOS_SMPL_DDRINIT)
    if( ddr_init(rom_option) != 0 )
        stop_cferom();
#endif
#endif /* INC_DDR_DIAGS */

#endif

#if defined(_BCM96858_)
    if( ddr_init(0) != 0 )
        stop_cferom();
#endif

#if defined(CFE_ROM_STOP) || (CFG_BOOT_PSRAM==1)
#if defined(_BCM96858_) || defined(_BCM94908_)
    armv8_disable_mmu();
#endif
    stop_cferom();
#endif

#if defined(IKOS_BD_LINUX_ROM)
    {
#if defined (_BCM963138_) || defined(_BCM963148_)
   static unsigned long linuxStartAddr=0x8000;
//   printf("L2C_FILT_START 0x%x L2C_FILT_END 0x%x\n", *((unsigned int*)0x8001dc00), *((unsigned int*)0x8001dc04));
#elif defined(_BCM963381_)
   static unsigned long linuxStartAddr=0x80010400;
#elif defined(_BCM94908_) || defined(_BCM96858_) || defined(_BCM968360_)
   static unsigned long linuxStartAddr=0x80000;/* 512KB offset */
#else
    /*0x694b6f32 (iKo2) is replaced with actual addr during the build process*/
    static unsigned long linuxStartAddr=0x694b6f32;
#endif
    board_setleds(0x4c494e58);  //LINX
    cfe_size_ram();

    cfe_launch(linuxStartAddr);
    }
#endif

#if (CFG_COPY_PSRAM==1)
    /* copy NVRAM to DDR */
    memcpy((unsigned char *) (DRAM_BASE + NVRAM_DATA_OFFSET), (unsigned char *)
        PSRAM_BASE + NVRAM_DATA_OFFSET, sizeof(NVRAM_DATA));
#endif

#if !defined(JTAG_DOWNLOAD)
#if (INC_NAND_FLASH_DRIVER==1)
    if (strap_check_nand())
    {
        bootImageFromNand(rom_option); /* Will not return. */
    }
#endif

#if (INC_SPI_NAND_DRIVER==1)
    if (strap_check_spinand())
    {
        bootImageFromNand(rom_option); /* Will not return. */
    }
#endif
#endif

#if (CFG_BOOT_PSRAM==0)
#if defined(_BCM96838_) && !(INC_NAND_FLASH_DRIVER==1)
{
    flash_device_info_t* flash_info;
    volatile unsigned long start_sector, end_sector, sector_size;
    unsigned short i;
    unsigned int buf_addr = DRAM_BASE;
    
    spi_flash_init(&flash_info);
    sector_size = spi_flash_get_sector_size(0); //assume all sectors have the same size

    start_sector = (binArrayStart-PSRAM_BASE_KSEG0)/sector_size;
    end_sector = (binArrayEnd-PSRAM_BASE_KSEG0)/sector_size;
    
    for (i=start_sector; i<=end_sector; i++)
    {
        int offset, nbytes;
        unsigned char* buff = (unsigned char*)buf_addr;
        
        if (i==start_sector)
        {
            offset = (binArrayStart - PSRAM_BASE_KSEG0)%sector_size;
            nbytes = sector_size - offset;
        }
        else if (i==end_sector)
        {
            offset = 0;
            nbytes = (binArrayEnd - PSRAM_BASE_KSEG0)%sector_size;
        }
        else
        {
            offset = 0;
            nbytes = sector_size;
        }
        spi_flash_read_buf(i, offset, buff, nbytes);
        buf_addr += nbytes;
    }

    entryPoint = (unsigned int*) DRAM_BASE;
    pucSrc = (unsigned char*)(DRAM_BASE+4);
}
#else

#if defined(_BCM96848_)
    binArrayStart = ((unsigned int)_binArrayStart - PSRAM_BASE_KSEG0) + FLASH_BASE;
    binArrayEnd = ((unsigned int)_binArrayEnd - PSRAM_BASE_KSEG0) + FLASH_BASE;
#endif

    entryPoint = (unsigned int*) (binArrayStart);
    pucSrc = (unsigned char *) (binArrayStart + 4);

#if (INC_BTRM_BOOT==1) && ( defined(_BCM963268_) || defined(_BCM963381_) || defined(_BCM963138_) || defined (_BCM963148_) )
    if (bcm_otp_is_boot_secure())
    {
       /* Decryption comes before decompression ... get the AES keys ready */
       memcpy((void *)&bootArgs, (void *)BTRM_INT_MEM_CREDENTIALS_ADDR, sizeof(Booter1Args));

       /* Move pucDest to point to where the decrypted (but still compressed) CFE RAM will be put */
       unsigned char *pucDest = (unsigned char *)(BTRM_INT_MEM_ENCR_COMP_CFE_RAM_ADDR - 4);

       /* Get ready to decrypt the CFE RAM bootloader */
       /* decryptWithEk() will whack the content of the iv structure, therefore create a copy and pass that in */
       unsigned char origIv[CIPHER_IV_LEN];
       memcpy((void *)origIv, (void *)bootArgs.encrArgs.biv, CIPHER_IV_LEN);

       /* Decrypt the CFE RAM bootloader */
       decryptWithEk(pucDest, pucSrc, bootArgs.encrArgs.bek, dataLen, origIv);

       /* The reference sw is done with the bek/biv at this point ... cleaning it up */
       /* Any remnants of the keys on the stack will be cleaned up when cfe_launch() runs */
       ek_iv_cleanup(&bootArgs.encrArgs);
       memset((void *)origIv, 0, CIPHER_IV_LEN);

       /* Get the size of the compressed file */
       dataLen = *(unsigned int *)pucDest;

       /* pucSrc now becomes the location of where we just decrypted the image into because */
       /* decompression comes next */
       pucSrc = (unsigned char *)(BTRM_INT_MEM_ENCR_COMP_CFE_RAM_ADDR);
    }

#endif

#endif

    pucDst = (unsigned char *) ((uintptr_t)(*entryPoint));

    ret = decompressLZMA((unsigned char*)pucSrc,
        (unsigned int)dataLen,
        (unsigned char *) pucDst,
        23*1024*1024);

#if (INC_BTRM_BOOT==1) && ( defined(_BCM963268_) || defined(_BCM963381_) || defined(_BCM963138_) || defined (_BCM963148_) )
    if (bcm_otp_is_boot_secure())
    {
       /* If decompression is a success, place the authentication credentials just before the */
       /* cferam. Otherwise, perform clean up */
       if (ret != 0)
       {
          board_setleds(0x434d5046); /* CMPF for deCoMPression Failed */
          cfe_launch(0);
       }
       else
       {
#if defined(_BCM963138_) || defined(_BCM963148_)
          memcpy((void *)(pucDst - (16*1024) - sizeof(Booter1AuthArgs)), (void *)&bootArgs.authArgs, sizeof(Booter1AuthArgs));
#else
          memcpy((void *)(pucDst - sizeof(Booter1AuthArgs)), (void *)&bootArgs.authArgs, sizeof(Booter1AuthArgs));
#endif
       } 
    }
#else
    if (ret != 0) 
        while (1);          // if not decompressed ok, loop for EJTAG
#endif

    cfe_launch((unsigned long) pucDst); // never return...
#endif
}

int pll_init(void)
{
#if defined(_BCM96838_) && (INC_PMC_DRIVER==1)
    unsigned int retval = 0;
    unsigned int ddr_freq_straps[] = {0, 333, 333, 400, 400, 533, 400, 533};
    unsigned int ddr_jedec_straps[] = {0, 3, 10, 21, 6, 8, 10, 13};
    unsigned int ddr_speed_straps[] = {0, 1, 2, 4, 2, 7, 2, 7}; // {0,333,333,400,400,533,400,533}
    unsigned int ddr_max_freq[] = {1000, 666, 533, 400, 333};
    unsigned int straps = ((*(volatile unsigned int*)(GPIO_BASE+GPIO_DATA_MID)) & (0x7 << (GPIO_STRAP_PIN_STRT-32))) >> (GPIO_STRAP_PIN_STRT-32) ;

    unsigned int ddr_freq = ddr_freq_straps[straps];
    unsigned int ddr_jedec = ddr_jedec_straps[straps];
    unsigned int ddr_speed = ddr_speed_straps[straps]; 

    volatile unsigned long otp_shadow_reg;
    unsigned long viper_freq;
    unsigned long rdp_freq;

    board_setleds(0x504c4c49); // PLLI

    otp_shadow_reg = *((volatile unsigned long*)(OTP_BASE+OTP_SHADOW_BRCM_BITS_0_31));

    // enforce max DDR frequency from OTP - if greater then max program minimum
    if(ddr_freq > ddr_max_freq[(otp_shadow_reg & OTP_BRCM_DDR_MAX_FREQ_MASK) >> OTP_BRCM_DDR_MAX_FREQ_SHIFT])
    {
        if((straps == 4) || (straps == 5))
            straps = 1; // DDR2
        else
            straps = 2; // DDR3

        ddr_freq = ddr_freq_straps[straps];
        ddr_jedec = ddr_jedec_straps[straps];
        ddr_speed = ddr_speed_straps[straps];
    }

    // for DDR3 1600MHz set phy_4x_mode
    if(straps == 3)
    {
        GPIO->memc_phy_control |= 1;
    }

    {
        volatile unsigned long *p = (unsigned long *)0xb4e00458;
        *p = 0x20000000;
    }
    
    WaitPmc(kPMCRunStateAVSCompleteWaitingForImage);
    board_setleds(0x504d4342); // PMCB

    switch( (otp_shadow_reg & OTP_BRCM_VIPER_FREQ_MASK) >> OTP_BRCM_VIPER_FREQ_SHIFT )
    {
        case 0:
            viper_freq = 600;
            break;

        case 1:
            viper_freq = 400;
            break;

        case 2:
            viper_freq = 240;
            break;

        default:
            viper_freq = 0;
            break;
    }
    if ( viper_freq )
        viper_freq_set(viper_freq);

#ifdef TEST_MODE
       rdp_freq = -1;
#else
    switch( (otp_shadow_reg & OTP_BRCM_RDP_FREQ_MASK) >> OTP_BRCM_RDP_FREQ_SHIFT )
    {
        case 0:
            rdp_freq = 800;
            break;

        case 1:
            rdp_freq = 400;
            break;

        default:
            rdp_freq = 0;
            break;
    }
#endif
    if ( rdp_freq )
        rdp_freq_set(rdp_freq);

    ddr_freq_set(ddr_freq);

    if(straps)
    {
        if(ddr_freq == 333)
            retval = 0x80000000;
        retval |= (ddr_jedec << PHY_CONTROL_REGS_STRAP_STATUS_JEDEC_TYPE_STRT);
        retval |= ((ddr_speed & 0x3) << PHY_CONTROL_REGS_STRAP_STATUS_SPEED_LO_STRT);
        retval |= (((ddr_speed >> 2) & 0x1) << PHY_CONTROL_REGS_STRAP_STATUS_SPEED_HI_STRT);
        retval |= PHY_CONTROL_REGS_STRAP_STATUS_STRAPS_VLD_MASK;
    }
    
    return retval;
#endif
    return 0;
}

/*  *********************************************************************
    *  cfe_usleep(usec)
    *  
    *  Sleep for approximately the specified number of microseconds.
    *  
    *  Input parameters: 
    *      usec - number of microseconds to wait
    *      
    *  Return value:
    *      nothing
    ********************************************************************* */
#if defined(_BCM963138_)
#define CFE_CLOCKS_PER_USEC 200
#elif defined(_BCM963148_)
#define CFE_CLOCKS_PER_USEC 750
#elif defined(_BCM94908_)
#define CFE_CLOCKS_PER_USEC (400+1400*fast_cpu_clock)
#elif defined(_BCM96858_) || defined(_BCM968360_)
#define CFE_CLOCKS_PER_USEC cpu_clock
#else
// use maximum frequency value for clock ticks
#define CFE_CLOCKS_PER_USEC 1000
#endif
// provide buffer zone such that we don't hang waiting for a corner case value such as max
#define BUFFER 8
void cfe_usleep(int usec)
{
    unsigned long newcount;
    unsigned long now;

    now = _getticks();
#if defined(_BCM96848_)
    newcount = now + (usec * clks_per_usec/2);
#else
    newcount = now + (usec * CFE_CLOCKS_PER_USEC);
#endif
    if (newcount > 0) // keep away from max
        newcount -= BUFFER;

    if (newcount < now)
        while (_getticks() > now)
            ;

    while (_getticks() < newcount)
            ;
}

#if defined(_BCM96858_) || defined(_BCM94908_) || defined(_BCM968360_)
uint64_t cfe_get_utime(void)
{ 
    return (_getticks()/CFE_CLOCKS_PER_USEC);
}

uint32_t get_ms_timer_tick(void)
{
    return (uint32_t)(cfe_get_utime()/1000);
}

uint64_t get_us_timer_tick(void)
{
    return cfe_get_utime();
}

#endif

#if defined(_BCM963138_) || defined(_BCM963148_)

/* The 64 bit ARM Glob Timer is implemented here only for 63138 memc diagnotics code. The original gettick use 32 bit ARM performance 
monitor counter which runs at 200MHz initial CPU clock and it wraps around every 20s. The diagnotics code requires longer wait time 
period. Without introducing the interrupt, only way to get longer rollover timer is used to the 64 bit ARM timer */

/* ARM GTimer runs at CPU_CLK/2. At cfe rom, ARM runs 200MHz. Need to update this value if we
decide to speed up CPU to 1GHz in cfe rom */
#if defined(_BCM963138_)
#define GTIMER_CLOCK_MHZ          100

void enable_arm_gtimer(int enable)
{
    if( enable )
    {
        /* stop first */
        ARMGTIM->gtim_glob_ctrl &= ~ARM_GTIM_GLOB_CTRL_TIMER_EN;
        
        /* enable timer, no interrupt, no comparison, no prescale */
        ARMGTIM->gtim_glob_low = 0;
        ARMGTIM->gtim_glob_hi = 0;
        ARMGTIM->gtim_glob_ctrl = ARM_GTIM_GLOB_CTRL_TIMER_EN;
    }

    if( !enable )
        ARMGTIM->gtim_glob_ctrl &= ~ARM_GTIM_GLOB_CTRL_TIMER_EN;

    return;
}

uint64_t get_arm_gtimer_tick(void)
{
    uint32_t low, high;

    high = ARMGTIM->gtim_glob_hi;
    low = ARMGTIM->gtim_glob_low;

    /* read high again to ensure there is no rollover from low 32 bit */
    if( high != ARMGTIM->gtim_glob_hi ) 
    {
        //read again if high changed 
        high = ARMGTIM->gtim_glob_hi;
        low = ARMGTIM->gtim_glob_low;  
    }

    return ((uint64_t)low)|(((uint64_t)high)<<32);
}

#elif defined(_BCM963148_)  /* place holder for 63148. to be implemented. these function used by DDR library */
#define GTIMER_CLOCK_MHZ          50
void enable_arm_gtimer(int enable)
{
    uint32_t val0;
    if (enable) {
        val0 = GTIMER_CLOCK_MHZ * 1000000;
        asm volatile("mcr p15, 0, %0, c14, c0, 0" : : "r" (val0));
    } else {
        /* we will just let it run */
    }

    return;
}

uint64_t get_arm_gtimer_tick(void)
{
    uint32_t low, high;

    asm volatile("mrrc p15, 0, %0, %1, c14" : "=r"  (low), "=r" (high));
    return ((uint64_t)low) | (((uint64_t)high) << 32);
}
#endif

/* a rough function, return ticks in ms.It does not consider the case where ms tick exceeds the 32 bit range but
nobody really need 4x10^6 seconds */
uint32_t get_ms_timer_tick(void)
{
    return (uint32_t)(get_arm_gtimer_tick()/(GTIMER_CLOCK_MHZ*1000));
}

uint64_t get_us_timer_tick(void)
{
    return (get_arm_gtimer_tick()/(GTIMER_CLOCK_MHZ));
}

#endif
