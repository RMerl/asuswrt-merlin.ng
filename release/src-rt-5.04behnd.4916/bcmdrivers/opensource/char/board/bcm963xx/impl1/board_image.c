/*
* <:copyright-BRCM:2016:DUAL/GPL:standard
* 
*    Copyright (c) 2016 Broadcom 
*    All Rights Reserved
* 
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed
* to you under the terms of the GNU General Public License version 2
* (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
* with the following added to such license:
* 
*    As a special exception, the copyright holders of this software give
*    you permission to link this software with independent modules, and
*    to copy and distribute the resulting executable under terms of your
*    choice, provided that you also meet, for each linked independent
*    module, the terms and conditions of the license of that module.
*    An independent module is a module which is not derived from this
*    software.  The special exception does not apply to any modifications
*    of the software.
* 
* Not withstanding the above, under no circumstances may you combine
* this software in any way with any other Broadcom software provided
* under a license other than the GPL, without Broadcom's express prior
* written consent.
* 
* :> 
*/

#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <asm/setup.h>
#include <linux/ctype.h>
#include <linux/sched.h>
#include <linux/semaphore.h>
#include <linux/syscalls.h>
#include <linux/crc32.h>
#include <linux/mtd/mtd.h>
#include <linux/jffs2.h>
#include <linux/kmod.h>
#include <linux/syscalls.h>

#include <bcm_assert_locks.h>
#include <bcmtypes.h>
#include <board.h>

#include <flash_api.h>
#include <flash_common.h>

#include "board_image.h"

#include "board_wl.h"
#include "board_util.h"

#include "bcm_otp.h"

#if defined(CONFIG_MTD_NAND_CORE)
int setup_mtd_parts(struct mtd_info* mtd)
{
    int nr_parts = mtd_device_parse_register(mtd, NULL, NULL, NULL, 0);

    return(nr_parts);
}
EXPORT_SYMBOL(setup_mtd_parts);
#endif
