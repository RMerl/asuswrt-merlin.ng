/*  *********************************************************************
    *
    <:copyright-BRCM:2017:proprietary:standard
    
       Copyright (c) 2017 Broadcom 
       All Rights Reserved
    
     This program is the proprietary software of Broadcom and/or its
     licensors, and may only be used, duplicated, modified or distributed pursuant
     to the terms and conditions of a separate, written license agreement executed
     between you and Broadcom (an "Authorized License").  Except as set forth in
     an Authorized License, Broadcom grants no license (express or implied), right
     to use, or waiver of any kind with respect to the Software, and Broadcom
     expressly reserves all rights in and to the Software and all intellectual
     property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
     NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
     BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
    
     Except as expressly set forth in the Authorized License,
    
     1. This program, including its structure, sequence and organization,
        constitutes the valuable trade secrets of Broadcom, and you shall use
        all reasonable efforts to protect the confidentiality thereof, and to
        use this information only in connection with your use of Broadcom
        integrated circuit products.
    
     2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
        AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
        WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
        RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
        ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
        FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
        COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
        TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
        PERFORMANCE OF THE SOFTWARE.
    
     3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
        ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
        INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
        WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
        IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
        OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
        SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
        SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
        LIMITED REMEDY.
    :> 
    ********************************************************************* */


#ifndef _ROM_MAIN_H_
#define _ROM_MAIN_H_

#ifndef INC_BTRM_BOOT
#define INC_BTRM_BOOT         0
#endif

#include "cfe.h"
#include "lib_types.h"
#include "lib_string.h"
#include "lib_printf.h"
#include "bcm_map.h"
#include "bcm_hwdefs.h"
#if (!defined(_BCM960333_) && !defined(_BCM96848_) && !defined(_BCM947189_) && !defined(_BCM963381_)) || (defined(_BCM963381_) && (INC_BTRM_BOOT==1))
#include "btrm_if.h"
extern Booter1Args bootArgs;
#endif
#include "bcm_auth_if.h"
#include "bcm_encr_if.h"

#include "bcm63xx_boot.h"



void bootPreCfeImage(void);
void bootNand(cfe_rom_media_params *media_params);
void bootSpiNand(cfe_rom_media_params *media_params);
void bootEmmc(cfe_rom_media_params *media_params);
void bootNor(void);
void stopCfeRom(void);
void launchLinux(void);

int bootInit(void);
int ddrInit(void);

int clk_init(void);
int pll_init(void);

void cfe_usleep(int usec);
int strap_check_spinand(void);

extern unsigned long rom_option;

extern unsigned long _getticks(void);
extern void board_setleds(uint32_t);
extern int board_puts(const char*);
extern int board_stsc(void);
extern char board_getc(void);
extern int decompressLZMA(unsigned char *in, unsigned insize, unsigned char *out, unsigned outsize);
extern int nand_read_buf(unsigned short blk, int offset, unsigned char *buffer, int len);
extern void rom_nand_flash_init(void);
extern void rom_spi_nand_init(void);

#endif
