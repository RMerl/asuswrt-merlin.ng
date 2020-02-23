/***************************************************************************
    <:copyright-BRCM:2012:DUAL/GPL:standard
    
       Copyright (c) 2012 Broadcom 
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
 ***************************************************************************
 * File Name  : dev_bcm63xx_flash.h 
 *
 * Created on :  04/18/2002  seanl
 ***************************************************************************/

#if !defined(_DEV_BCM63XX_FLASH_)
#define _DEV_BCM63XX_FLASH_

#include "bcmtypes.h"
#include "bcm_hwdefs.h"

// Used for images that do not contain a FILE_TAG record.
#define FLASH_IMAGE_START_ADDR          (FLASH_BASE + FLASH_LENGTH_BOOT_ROM)

// FLASH_ADDR_INFO is now defined in flash_common.h
#include "flash_common.h"

extern void kerSysFlashInit(void);
extern void kerSysFlashAddrInfoGet(PFLASH_ADDR_INFO pflash_addr_info);
extern int kerSysNvRamSet(PNVRAM_DATA pNvramData);
extern int kerSysNvRamGet(PNVRAM_DATA pNvramData);
extern int kerSysBcmImageSet( int flash_start_addr, unsigned char *string, int size, int fWholeImage);
extern int kerSysErasePsi(void);
extern int kerSysEraseNvRam(void);
extern unsigned long kerSysReadFromFlash(void *toaddr, unsigned long fromaddr, unsigned long len);
extern int kerSysSetBootImageState( int state );
extern int kerSysGetBootImageState( void );
void get_flash_boot_cfe_version(char **version, int *size);

#endif /* _DEV_BCM63XX_FLASH_ */

