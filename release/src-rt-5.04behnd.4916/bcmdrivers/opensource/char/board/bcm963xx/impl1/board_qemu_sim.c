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
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/errno.h>
#include <linux/module.h>
#include <linux/ctype.h>
#include <linux/sched.h>
#include <linux/device.h>
#include <linux/dma-mapping.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <bcmtypes.h>
#include <board.h>
#include "board_nvram.h"

void kerSysEarlyFlashInitUbootEnv(void)
{
    get_uboot_env_area(UBOOT_MAX_ENV_LEN);
    uboot_env_add("MCB=0x14162f");
    uboot_env_add("nummacaddrs=10");
    uboot_env_add("ethaddr=0a:02:03:04:37:42");
    uboot_env_add("ipaddr=192.168.1.1");
    uboot_env_add("netmask=255.255.255.0");
    uboot_env_add("boardid=968880QEMU_SIM");
}
