/*
    Copyright 2000-2010 Broadcom Corporation
   <:label-BRCM:2012:DUAL/GPL:standard
   
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
*/                       

/***************************************************************************
 * File Name  : flash_api.c
 *
 * Description: This file contains the implementation of the wrapper functions
 *              for the flash device interface.
 ***************************************************************************/

/** Includes. */
#ifdef _CFE_
#include "lib_types.h"
#include "lib_printf.h"
#include "lib_string.h"
#include "bcm_map_part.h"  
#define printk  printf
#else // Linux
#include <linux/kernel.h>
#include "bcm_map_part.h"
#endif

#include "bcmtypes.h"
#include "bcm_hwdefs.h"
#include "flash_api.h"

/** Externs. **/
#if !defined(INC_CFI_FLASH_DRIVER)
#define INC_CFI_FLASH_DRIVER        0  
#endif

#if !defined(INC_SPI_FLASH_DRIVER)
#define INC_SPI_FLASH_DRIVER        0
#endif

#if !defined(INC_NAND_FLASH_DRIVER)
#define INC_NAND_FLASH_DRIVER       0
#endif

#if !defined(INC_SPI_PROG_NAND)
#define INC_SPI_PROG_NAND       0
#endif

#if !defined(INC_SPI_NAND_DRIVER)
#define INC_SPI_NAND_DRIVER     0
#endif

#if (INC_CFI_FLASH_DRIVER==1)
extern int cfi_flash_init(flash_device_info_t **flash_info);
#else
#define cfi_flash_init(x)           FLASH_API_ERROR
#endif

#if (INC_SPI_FLASH_DRIVER==1)
extern int spi_flash_init(flash_device_info_t **flash_info);
#else
#define spi_flash_init(x)           FLASH_API_ERROR
#endif

#if (INC_NAND_FLASH_DRIVER==1) || (INC_SPI_PROG_NAND==1)
extern int nand_flash_init(flash_device_info_t **flash_info);
#else
#define nand_flash_init(x)           FLASH_API_ERROR
#endif

#if (INC_SPI_NAND_DRIVER==1) || defined(CONFIG_MTD_BCM_SPI_NAND)
extern int spi_nand_init(flash_device_info_t **flash_info);
#else
#define spi_nand_init(x)           FLASH_API_ERROR
#endif

/** Variables. **/
static flash_device_info_t *g_flash_info = NULL;
#if (INC_SPI_PROG_NAND==1)
static flash_device_info_t *g_nand_flash_info = NULL;
static flash_device_info_t *g_spi_flash_info = NULL;
#endif

static flash_device_info_t flash_dummy_dev =
    {
        0xffff,
        FLASH_IFC_UNKNOWN,
        "",
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL
    };

#if (INC_SPI_NAND_DRIVER==1) || defined(CONFIG_MTD_BCM_SPI_NAND)
static int strap_check_spinand(void)
{
#if defined(_BCM96838_) || defined(CONFIG_BCM96838)
    return ( (((GPIO->strap_bus & MISC_STRAP_BUS_BOOT_CFG_MASK)>>MISC_STRAP_BUS_BOOT_SEL_SHIFT) == MISC_STRAP_SPI_4BYTE_ADDR_MASK) &&
             (((GPIO->strap_bus & MISC_STRAP_BUS_PAGE_SIZE_MASK)>>MISC_STRAP_BUS_PAGE_SIZE_SHIFT) & MISC_STRAP_BUS_SPI_NAND_BOOT)    );

#elif defined(_BCM963381_) || defined(CONFIG_BCM963381)
    return ((MISC->miscStrapBus & MISC_STRAP_BUS_SPI_NAND_DISABLE) == 0);

#elif defined(_BCM963138_) || defined(CONFIG_BCM963138)
    return ( (MISC->miscStrapBus & MISC_STRAP_BUS_SW_BOOT_SPI_SPINAND_EMMC_MASK) &&
            ((MISC->miscStrapBus & MISC_STRAP_BUS_BOOT_OPT_MASK) == MISC_STRAP_BUS_BOOT_SPI_NAND) );
#elif defined(_BCM96848_) || defined(CONFIG_BCM96848)
    return  ((MISC->miscStrapBus&MISC_STRAP_BUS_BOOT_SEL_MASK)==MISC_STRAP_BUS_BOOT_SPI_NAND);
#elif defined(_BCM94908_) || defined(CONFIG_BCM94908) || \
      defined(_BCM963158_) || defined(CONFIG_BCM963158) || defined(_BCM96846_) || defined(CONFIG_BCM96846) || \
      defined(_BCM96856_) || defined(CONFIG_BCM96856) || defined(_BCM963178_) || defined(CONFIG_BCM963178) || \
      defined(_BCM947622_) || defined(CONFIG_BCM947622) || defined(_BCM96878_) || defined(CONFIG_BCM96878)
    return ((MISC->miscStrapBus&MISC_STRAP_BUS_BOOT_SEL_MASK) == MISC_STRAP_BUS_BOOT_SPI_NAND);
#elif defined(_BCM96858_) || defined(CONFIG_BCM96858)
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

/***************************************************************************
 * Function Name: display_flash_info
 * Description  : Displays information about the flash part.
 * Returns      : None.
 ***************************************************************************/
static void display_flash_info(int ret, flash_device_info_t *flash_info)
{
    switch (flash_info->flash_type) {
    case FLASH_IFC_PARALLEL:
        printk( "Parallel flash device");
        break;

    case FLASH_IFC_SPI:
        printk( "Serial flash device");
        break;

    case FLASH_IFC_HS_SPI:
        printk( "HS Serial flash device");
        break;

    case FLASH_IFC_NAND:
        printk( "NAND flash device");
        break;

    case FLASH_IFC_SPINAND:
        printk( "SPI NAND flash device");
        break;

    case FLASH_IFC_UNSUP_EMMC:
        /* Unsupported eMMC device do nothing */
        return;
    }

    if( ret == FLASH_API_OK ) {
        printk(": %s, id 0x%08x",
            flash_info->flash_device_name, flash_info->flash_device_id);
#if (INC_SPI_PROG_NAND==1)
        printk(" block %dKB", flash_info->fn_flash_get_sector_size(0) / 1024);
        printk(" size %luKB", (unsigned long )
            flash_info->fn_flash_get_total_size() / 1024);
#else
        if ((flash_info->flash_type == FLASH_IFC_NAND) || (flash_info->flash_type == FLASH_IFC_SPINAND))
            printk(" block %dKB", flash_get_sector_size(0) / 1024);
	else
            printk(" sector %dKB", flash_get_sector_size(0) / 1024);
        printk(" size %luKB", flash_get_total_size() / 1024);
#endif
        printk("\n");
    }
    else {
        printk( " id %08x is not supported.\n", flash_info->flash_device_id );
    }
} /* display_flash_info */

#ifdef _CFE_
extern unsigned short g_force_mode;
#endif

/***************************************************************************
 * Function Name: flash_init
 * Description  : Initialize flash part.
 * Returns      : FLASH_API_OK or FLASH_API_ERROR
 ***************************************************************************/
int flash_init(void)
{
    int type = FLASH_IFC_UNKNOWN;
    int ret = FLASH_API_ERROR;
#if (INC_SPI_PROG_NAND==1)
    int ret_nand = FLASH_API_ERROR;
#endif

    /* If available, use bootstrap to decide which flash to use */
#if defined(_BCM963268_) || defined(CONFIG_BCM963268)
    if( ((MISC->miscStrapBus & MISC_STRAP_BUS_BOOT_SEL_MASK) >>
        MISC_STRAP_BUS_BOOT_SEL_SHIFT) ==  MISC_STRAP_BUS_BOOT_SERIAL )
        type = FLASH_IFC_SPI;
    else
        type = FLASH_IFC_NAND;

#elif defined(_BCM96838_) || defined(CONFIG_BCM96838)
#if (INC_SPI_NAND_DRIVER==1) || defined(CONFIG_MTD_BCM_SPI_NAND)
    if(strap_check_spinand())
        type = FLASH_IFC_SPINAND;
    else
#endif
    if( (((GPIO->strap_bus & MISC_STRAP_BUS_BOOT_SEL_MASK)>>MISC_STRAP_BUS_BOOT_SEL_SHIFT) == MISC_STRAP_SPI_3BYTE_ADDR_MASK) ||
        (((GPIO->strap_bus & MISC_STRAP_BUS_BOOT_SEL_MASK)>>MISC_STRAP_BUS_BOOT_SEL_SHIFT) == MISC_STRAP_SPI_4BYTE_ADDR_MASK) )
        type = FLASH_IFC_SPI;
    else
        type = FLASH_IFC_NAND;

#elif defined(_BCM963381_) || defined(CONFIG_BCM963381) || defined(_BCM96848_) || defined(CONFIG_BCM96848)
#if (INC_SPI_NAND_DRIVER==1) || defined(CONFIG_MTD_BCM_SPI_NAND)
    if(strap_check_spinand())
        type = FLASH_IFC_SPINAND;
    else
#endif
    if ((MISC->miscStrapBus & MISC_STRAP_BUS_BOOT_SPI_NOR) == MISC_STRAP_BUS_BOOT_SPI_NOR)
        type = FLASH_IFC_SPI;
    else
        type = FLASH_IFC_NAND;

#elif defined(_BCM963138_) || defined(CONFIG_BCM963138) || defined(_BCM963148_) || defined(CONFIG_BCM963148)
    /* Set default to NAND */
    type = FLASH_IFC_NAND;

#if (defined(_BCM963138_) || defined(CONFIG_BCM963138))
#if ((INC_SPI_NAND_DRIVER==1) || defined(CONFIG_MTD_BCM_SPI_NAND))
    if(strap_check_spinand())
        type = FLASH_IFC_SPINAND;
    else
#endif
    if(MISC->miscStrapBus & MISC_STRAP_BUS_SW_BOOT_SPI_SPINAND_EMMC_MASK)
    {
        if((MISC->miscStrapBus & MISC_STRAP_BUS_BOOT_OPT_MASK) == MISC_STRAP_BUS_BOOT_SPI_NOR)
            type = FLASH_IFC_SPI;
        else if((MISC->miscStrapBus & MISC_STRAP_BUS_BOOT_OPT_MASK) == MISC_STRAP_BUS_BOOT_EMMC)
            type = FLASH_IFC_UNSUP_EMMC;
    }
#else /* 63148 */
    if((MISC->miscStrapBus & MISC_STRAP_BUS_BOOT_OPT_MASK) == MISC_STRAP_BUS_BOOT_SPI_NOR)
        type = FLASH_IFC_SPI;
#endif

#if !(INC_SPI_PROG_NAND==1)
    if ((type == FLASH_IFC_SPI) && (NAND->NandNandBootConfig & NBC_AUTO_DEV_ID_CFG )) {
        // Strapped for SPI but Nand initialized ... Linux should assume NAND
        type = FLASH_IFC_NAND;
        printk("Originally booted from SPI, now using NAND\n");
    }
#endif
#elif defined(_BCM94908_) || defined(CONFIG_BCM94908) || \
    defined(_BCM963158_) || defined(CONFIG_BCM963158) || \
    defined(_BCM96846_) || defined(CONFIG_BCM96846) || \
    defined(_BCM96878_) || defined(CONFIG_BCM96878) || \
    defined(_BCM963178_) || defined(CONFIG_BCM963178) || \
    defined(_BCM947622_) || defined(CONFIG_BCM947622) || \
    defined(_BCM96856_) || defined(CONFIG_BCM96856)
#if ((INC_SPI_NAND_DRIVER==1) || defined(CONFIG_MTD_BCM_SPI_NAND))
    if(strap_check_spinand())
        type = FLASH_IFC_SPINAND;
    else
#endif
    if( (MISC->miscStrapBus&MISC_STRAP_BUS_BOOT_SEL_MASK) == MISC_STRAP_BUS_BOOT_SPI_NOR )
        type = FLASH_IFC_SPI;
    else if( (MISC->miscStrapBus&MISC_STRAP_BUS_BOOT_SEL_NAND_MASK) == MISC_STRAP_BUS_BOOT_NAND )
        type = FLASH_IFC_NAND;
#if defined(MISC_STRAP_BUS_BOOT_EMMC)
    else if( (MISC->miscStrapBus&MISC_STRAP_BUS_BOOT_EMMC) == MISC_STRAP_BUS_BOOT_EMMC)
        type = FLASH_IFC_UNSUP_EMMC;
#endif

#if !(INC_SPI_PROG_NAND==1)
    if ((type == FLASH_IFC_SPI) && (NAND->NandNandBootConfig & NBC_AUTO_DEV_ID_CFG )) {
        // Strapped for SPI but Nand initialized ... Linux should assume NAND
        type = FLASH_IFC_NAND;
        printk("Originally booted from SPI-NOR, now using NAND\n");
    }
#endif

#elif defined(_BCM96858_) || defined(CONFIG_BCM96858)
#if ((INC_SPI_NAND_DRIVER==1) || defined(CONFIG_MTD_BCM_SPI_NAND))
    if(strap_check_spinand())
        type = FLASH_IFC_SPINAND;
    else
#endif
    {
#if (INC_SPI_PROG_NAND==1)
    type = FLASH_IFC_SPI;
#else
    uint32 bootsel = ((MISC->miscStrapBus & MISC_STRAP_BUS_BOOT_SEL0_4_MASK) >> MISC_STRAP_BUS_BOOT_SEL0_4_SHIFT) |
                     ((MISC->miscStrapBus & MISC_STRAP_BUS_BOOT_SEL5_MASK) >> BOOT_SEL5_STRAP_ADJ_SHIFT);
    if ((bootsel & BOOT_SEL_STRAP_BOOT_SEL_MASK) == BOOT_SEL_STRAP_SPI_NOR)
        type = FLASH_IFC_SPI;
    else if ((bootsel & BOOT_SEL_STRAP_BOOT_SEL_MASK) != BOOT_SEL_STRAP_EMMC)
        type = FLASH_IFC_NAND;
    else if ((bootsel & BOOT_SEL_STRAP_BOOT_SEL_MASK) == BOOT_SEL_STRAP_EMMC)
        type = FLASH_IFC_UNSUP_EMMC;
#endif
    }
#if !(INC_SPI_PROG_NAND==1)
    if ((type == FLASH_IFC_SPI) && (NAND->NandNandBootConfig & NBC_AUTO_DEV_ID_CFG )) {
        // Strapped for SPI but Nand initialized ... Linux should assume NAND
        type = FLASH_IFC_NAND;
        printk("Originally booted from SPI-NOR, now using NAND\n");
    }
#endif
#endif

#if defined(_BCM947189_) || defined(CONFIG_BCM947189)
        type = FLASH_IFC_SPI;
	GCI_BASE->status_idx_reg_wr=GCI_CHIP_STATUS_REGISTER_INDEX_7;
	if((GCI_BASE->status_idx_reg_rd & BOOT_MODE_MASK) == BOOT_MODE_NAND)
        	type = FLASH_IFC_NAND;
#endif

#if (INC_SPI_NAND_DRIVER==1)
    if (g_force_mode==2)
        type = FLASH_IFC_SPINAND;
    if (g_force_mode==3)
        type = FLASH_IFC_NAND;
#endif

#if defined(MISC_STRAP_BUS_BOOT_ETH)
/* In case of netboot, default storage is set to NAND*/
    if (((MISC->miscStrapBus & MISC_STRAP_BUS_BOOTSEL_MASK) >> MISC_STRAP_BUS_BOOTSEL_SHIFT) == MISC_STRAP_BUS_BOOT_ETH)
        type=FLASH_IFC_NAND;
#endif

    switch (type) {
    case FLASH_IFC_PARALLEL:
        ret = cfi_flash_init( &g_flash_info );
        break;

    case FLASH_IFC_SPI:
        ret = spi_flash_init( &g_flash_info );
#if (INC_SPI_PROG_NAND==1)
        ret_nand = nand_flash_init( &g_nand_flash_info );
#endif
        break;

    case FLASH_IFC_NAND:
        ret = nand_flash_init( &g_flash_info );
       break;

#ifdef _CFE_
    case FLASH_IFC_SPINAND:
        ret = spi_nand_init( &g_flash_info );
        break;
#endif

    case FLASH_IFC_UNSUP_EMMC:
        g_flash_info = &flash_dummy_dev;
        g_flash_info->flash_type = type;
        break;

    case FLASH_IFC_UNKNOWN:
        /* Try to detect flash chips, give priority to parallel flash */
        /* Our reference design has both, and we usually use parallel. */
#ifndef _CFE_
        ret = cfi_flash_init( &g_flash_info );
        if (ret == FLASH_API_OK) {
            type = FLASH_IFC_PARALLEL;
        } else {
            ret = spi_flash_init( &g_flash_info );
            if ( ret == FLASH_API_OK )
                type = FLASH_IFC_SPI;
        }

#endif	
        break;
    }

    if (g_flash_info != NULL) {
        display_flash_info(ret, g_flash_info);
#if (INC_SPI_PROG_NAND==1)
        if( g_nand_flash_info != NULL )
            display_flash_info(ret_nand, g_nand_flash_info );
#endif
    }
    else {
#ifdef _CFE_
        printk( "BCM Flash API. Flash device is not found.\n" );
#endif
    }

    return( ret );
} /* flash_init */

/***************************************************************************
 * Function Name: flash_sector_erase_int
 * Description  : Erase the specfied flash sector.
 * Returns      : FLASH_API_OK or FLASH_API_ERROR
 ***************************************************************************/
int flash_sector_erase_int(unsigned short sector)
{
    return( (g_flash_info)
        ? (*g_flash_info->fn_flash_sector_erase_int) (sector)
        : FLASH_API_ERROR );
} /* flash_sector_erase_int */

/***************************************************************************
 * Function Name: flash_read_buf
 * Description  : Reads from flash memory.
 * Returns      : number of bytes read or FLASH_API_ERROR
 ***************************************************************************/
int flash_read_buf(unsigned short sector, int offset, unsigned char *buffer,
    int numbytes)
{
    return( (g_flash_info)
        ? (*g_flash_info->fn_flash_read_buf) (sector, offset, buffer, numbytes)
        : FLASH_API_ERROR );
} /* flash_read_buf */

/***************************************************************************
 * Function Name: flash_write_buf
 * Description  : Writes to flash memory.
 * Returns      : number of bytes written or FLASH_API_ERROR
 ***************************************************************************/
int flash_write_buf(unsigned short sector, int offset, unsigned char *buffer,
    int numbytes)
{
    return( (g_flash_info)
        ? (*g_flash_info->fn_flash_write_buf) (sector, offset, buffer, numbytes)
        : FLASH_API_ERROR );
} /* flash_write_buf */

/***************************************************************************
 * Function Name: flash_get_numsectors
 * Description  : Returns the number of sectors in the flash device.
 * Returns      : Number of sectors in the flash device.
 ***************************************************************************/
int flash_get_numsectors(void)
{
    return( (g_flash_info)
        ? (*g_flash_info->fn_flash_get_numsectors) ()
        : FLASH_API_ERROR );
} /* flash_get_numsectors */

/***************************************************************************
 * Function Name: flash_get_sector_size
 * Description  : Returns the number of bytes in the specfied flash sector.
 * Returns      : Number of bytes in the specfied flash sector.
 ***************************************************************************/
int flash_get_sector_size(unsigned short sector)
{
    return( (g_flash_info)
        ? (*g_flash_info->fn_flash_get_sector_size) (sector)
        : FLASH_API_ERROR );
} /* flash_get_sector_size */

/***************************************************************************
 * Function Name: flash_get_memptr
 * Description  : Returns the base MIPS memory address for the specfied flash
 *                sector.
 * Returns      : Base MIPS memory address for the specfied flash sector.
 ***************************************************************************/
unsigned char *flash_get_memptr(unsigned short sector)
{
    return( (g_flash_info)
        ? (*g_flash_info->fn_flash_get_memptr) (sector)
        : NULL );
} /* flash_get_memptr */

/***************************************************************************
 * Function Name: flash_get_blk
 * Description  : Returns the flash sector for the specfied MIPS address.
 * Returns      : Flash sector for the specfied MIPS address.
 ***************************************************************************/
int flash_get_blk(int addr)
{
    return( (g_flash_info)
        ? (*g_flash_info->fn_flash_get_blk) (addr)
        : FLASH_API_ERROR );
} /* flash_get_blk */

/***************************************************************************
 * Function Name: flash_get_total_size
 * Description  : Returns the number of bytes in the flash device.
 * Returns      : Number of bytes in the flash device.
 ***************************************************************************/
unsigned long flash_get_total_size(void)
{
    return( (g_flash_info)
        ? (unsigned long) (*g_flash_info->fn_flash_get_total_size) ()
        : FLASH_API_ERROR );
} /* flash_get_total_size */

/***************************************************************************
 * Function Name: flash_dev_specific_cmd
 * Description  : Triggers a device specific feature, used to access 
 *                non-standard features.
 * Returns      : Feature specific return code.
 ***************************************************************************/
int flash_dev_specific_cmd(unsigned int command, void * inBuf, void * outBuf)
{
    return( (g_flash_info)
        ? (*g_flash_info->fn_flash_dev_specific_cmd) (command, inBuf, outBuf)
        : FLASH_API_ERROR );
} /* flash_dev_specific_cmd */

/***************************************************************************
 * Function Name: flash_get_flash_type
 * Description  : Returns type of the flash memory.
 * Returns      : Type of the flash memory.
 ***************************************************************************/
int flash_get_flash_type(void)
{
    if(g_flash_info)
       return(g_flash_info->flash_type);
#ifdef _CFE_
    return(FLASH_API_ERROR);
#endif
    /* For Linux */
#if defined(CONFIG_MTD_BCM_SPI_NAND)
    if (strap_check_spinand())
        return(FLASH_IFC_SPINAND);
#endif
    return(FLASH_IFC_NAND);
} /* flash_get_flash_type */

#if (INC_SPI_PROG_NAND==1)
/***************************************************************************
 * Function Name: flash_change_flash_type
 * Description  : change type of the flash memory.
 * Returns      : none
 ***************************************************************************/
void flash_change_flash_type(int type)
{

    if (type == FLASH_IFC_NAND)
    {
        if (g_spi_flash_info == NULL)
            g_spi_flash_info = g_flash_info;
        g_flash_info = g_nand_flash_info;
    }
    else
    {
        if (g_spi_flash_info != NULL)
            g_flash_info = g_spi_flash_info;
    }
} /* flash_change_flash_type */

#endif

