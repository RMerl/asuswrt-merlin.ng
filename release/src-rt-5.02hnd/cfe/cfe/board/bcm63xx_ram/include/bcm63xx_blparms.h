/***************************************************************************
 <:copyright-BRCM:2015:DUAL/GPL:standard
 
    Copyright (c) 2015 Broadcom 
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

#if !defined(__BCM63XX_BLPARMS_H__)
#define __BCM63XX_BLPARMS_H__

#include "lib_types.h"
#include "lib_string.h"
#include "lib_printf.h"
#include "cfe.h"
#include "bcm_hwdefs.h"
#include "bcm_map.h"
#ifdef CFG_ATAG
#include "atag.h"
#endif
#ifdef CFG_DT
#include "bcm63xx_dtb.h"
#endif


#define BLPARMS_LEN            1024
#define BLKERNPARM             "BRCM_EXTRA_KERN"

extern void blparms_init(void);
extern void blparms_set_int(char *name, int value);
extern void blparms_set_str(char *name, const char *value);
extern void blparms_install(unsigned long *loadaddr);
extern int  blparms_add_extra_parms(char* parm, char parm_op, char parm_delimiter, 
                char parm_data_delimiter, char escape_char,char* ext_parm);

extern void dtb_install(void);
extern void set_reserved_memory(void);
#endif // __BCM63XX_BLPARMS_H__
