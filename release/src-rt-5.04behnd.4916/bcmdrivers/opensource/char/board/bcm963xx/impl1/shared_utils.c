
/*
 * <:copyright-BRCM:2012:DUAL/GPL:standard
 * 
 *    Copyright (c) 2012 Broadcom 
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


#include "shared_utils.h"

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/string.h>

#if defined (CONFIG_BCM96858) || defined (CONFIG_BCM96856) || defined (CONFIG_BCM96765) || defined (CONFIG_BCM96764)
#include "bcm_otp.h"
#endif
#include "board_dt.h"

#define INVALID_CHIP_ID 0xffffffff

#ifndef CONFIG_BCM_QEMU_SIM
static unsigned int UtilGetChipIdLC(void)
{
    u32 __iomem *reg;
    u32 chipid_lc;

    reg = bcm_get_ioreg("brcm,misc-io", "misc-top-chip-id-lc");
    if (IS_ERR_OR_NULL(reg)) {
        //pr_err("Error: chip id lc register not defined in the device tree!");
        return 0;
    }

    chipid_lc = readl(reg)&0xf;
    bcm_unmap_ioreg(reg);

    return chipid_lc;
}
#endif

unsigned int UtilGetChipIdRaw(void)
{
#ifndef CONFIG_BCM_QEMU_SIM
    u32 __iomem *reg;
    u32 chipid;

    reg = bcm_get_ioreg("brcm,misc-io", "misc-periph-chip-id-rev");
    if (IS_ERR_OR_NULL(reg)) {
        pr_err("Error: chip id register not defined in the device tree!");
        return INVALID_CHIP_ID;
    }

    chipid = readl(reg);

    bcm_unmap_ioreg(reg);
    return chipid;
#else
#if defined(CONFIG_BCM968880)
    return 0x68880;
#elif defined(CONFIG_BCM96837)
    return 0x6837;
#elif defined(CONFIG_BCM968375)
    return 0x68375;    
#else
#error unsupported chipset
#endif
#endif
}
EXPORT_SYMBOL(UtilGetChipIdRaw);

unsigned int UtilGetChipId(void)
{
#ifndef CONFIG_BCM_QEMU_SIM
    unsigned int chipId;
    unsigned int chipIdLC;
    u32 chipid_mask, chipid_shift;
    bool ret;
#if defined (CONFIG_BCM96856)
    unsigned int chipvar;
#endif
	
    chipId = UtilGetChipIdRaw();

    ret = bcm_get_prop32("brcm,misc-io", "misc-periph-chip-id-mask", &chipid_mask);
    ret &= bcm_get_prop32("brcm,misc-io", "misc-periph-chip-id-shift", &chipid_shift);
    if (!ret) {
        pr_err("Error: chip id shift or mask not defined in the device tree!");
        return INVALID_CHIP_ID;
    }

    chipId  = (chipId & chipid_mask) >> chipid_shift;

    chipIdLC = UtilGetChipIdLC();
    if (chipIdLC)
        chipId = (chipId << 4) | chipIdLC;

#if defined (CONFIG_BCM96856)
    bcm_otp_get_chipvar(&chipvar);
    if ((chipId==0x68560) && (chipvar==3))
        chipId=0x68560B;
#endif
    return  chipId;
#else
#if defined(CONFIG_BCM968880)
    return 0x68880;
#elif defined(CONFIG_BCM96837)
    return 0x6837;
#elif defined(CONFIG_BCM968375)
    return 0x68375;
#elif defined(CONFIG_BCM968850)
    return 0x68850;    
#else
#error unsupported chipset
#endif
#endif
}

/* Wrapper of UtilGetChipId for RDPA compilation*/
/* linux kernel func. name format */
unsigned int util_get_chip_id(void) 
{
    return UtilGetChipId();
}

EXPORT_SYMBOL(UtilGetChipId);
EXPORT_SYMBOL(util_get_chip_id);

unsigned int UtilGetChipRev(void)
{
#ifndef CONFIG_BCM_QEMU_SIM
    unsigned int revId;
    u32 chiprev_mask;
    bool ret;

    revId = UtilGetChipIdRaw();
    ret = bcm_get_prop32("brcm,misc-io", "misc-periph-chip-rev-mask", &chiprev_mask);
    if (!ret) {
        pr_err("Error: chip rev mask not defined in the device tree!");
        return INVALID_CHIP_ID;
    }

    revId = revId & chiprev_mask;
#if defined(CONFIG_BCM968880) || defined(CONFIG_BCM96837) 
    revId += 0xA0;
#endif

    return  revId;
#else
#if defined(CONFIG_BCM968880)
    return 0xb0;
#elif defined(CONFIG_BCM96837)
    return 0xa0;
#elif defined(CONFIG_BCM968375)
    return 0xa0;    
#else
#error unsupported chipset
#endif
#endif
}
EXPORT_SYMBOL(UtilGetChipRev);

unsigned int UtilGetChipIsLP(void)
{
    return 0;
}
EXPORT_SYMBOL(UtilGetChipIsLP);

char *UtilGetChipName(char *buf, int len) {
    unsigned int chipId;
    unsigned int revId;
    char *mktname = NULL;
#if defined(CONFIG_BCM96765) || defined(CONFIG_BCM96764) || defined(CONFIG_BCM96858)
    unsigned int chipvar;
#endif

    revId = UtilGetChipRev();
#if defined(CONFIG_BCM96858)
    if (bcm_otp_get_chipvar(&chipvar))
        chipvar = 0;
#else
    chipId = UtilGetChipId();
#endif

#if defined (CONFIG_BCM96858)
    switch (chipvar) {
    case(0x0):
    case(0x1):
        mktname = "68580X";
        break;
    case(0x2):
        mktname = "55040";
        break;
    case(0x3):
        mktname = "68580H";
        break;
    case(0x4):
        mktname = "55040P";
        break;
    case(0x5):
        mktname = "55045";
        break;
    case(0x6):
        mktname = "68580XV";
        break;
    case(0x7):
        mktname = "49508";
        break;
    case(0x8):
        mktname = "62119";
        break;
    case(0x9):
        mktname = "68580XP";
        break;
    case(0xA):
        mktname = "62119P";
        break;
    case(0xB):
        mktname = "55040B";
        break;
    case(0xC):
        mktname = "55040M";
        break;
    case(0xD):
        mktname = "68580XF";
        break;
    default:
        mktname = NULL;
    }
#elif defined (CONFIG_BCM96855)
    switch (chipId) {
    case(0x68252d):
        mktname = "68252R";
        break;
    case(0x68252e):
        mktname = "68252CR";
        break;
    case(0x67530):
        mktname = "6753";
        break;
    default:
        mktname = NULL;
    }
#elif defined (CONFIG_BCM96765)
    bcm_otp_get_chipvar(&chipvar);
    if ((chipId == 0x6765) && (chipvar == 1))
        mktname = "6764L";
#elif defined (CONFIG_BCM96764)
    bcm_otp_get_chipvar(&chipvar);
    if ((chipId == 0x6764) && (chipvar == 5))
        mktname = "47722L";
#endif

    if (mktname == NULL) {
        sprintf(buf,"%X_%X",chipId,revId);
    } else {
        sprintf(buf,"%s_%X",mktname,revId);
    }
    return(buf);
}

int UtilGetChipIsPinCompatible(void) 
{

    int ret = 0;

    return(ret);
}
