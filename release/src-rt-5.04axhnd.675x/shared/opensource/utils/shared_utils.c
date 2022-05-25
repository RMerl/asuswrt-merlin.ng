
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

#include "boardparms.h"
#include "shared_utils.h"

#include <linux/kernel.h>
#include <linux/module.h>
#include <bcm_map_part.h>
#include <linux/string.h>

#if defined (CONFIG_BCM96858) || defined(_BCM96858_) || defined (CONFIG_BCM96856) || defined(_BCM96856_)
#include "bcm_otp.h"
#endif

unsigned int UtilGetChipId(void)
{
#if defined(CONFIG_BCM947189) || defined(_BCM947189_)
    unsigned int chipId = (MISC->chipid & CID_ID_MASK);
#else
    unsigned int chipId = (PERF->RevID & CHIP_ID_MASK) >> CHIP_ID_SHIFT;
#endif

#if defined (CONFIG_BCM96855) || defined (_BCM96855_)
    unsigned int chipIdLC = (TOP->OtpChipidLC & CHIP_ID_LC_MASK);
    if (chipIdLC)
        chipId = (chipId << CHIP_ID_LC_SIZE) | chipIdLC;
#endif

#if defined(CONFIG_BCM94908) || defined(_BCM94908_)
    // 62118 and 62116 use 20 bits to represent the chip id 
    // as compared to 16 in case of 4908/4906
    if((chipId & 0x49000) == 0x49000)
    {
        chipId = chipId >> 4;
    }
#endif

#if defined (CONFIG_BCM96856) || defined(_BCM96856_)
    unsigned int chipvar;
    bcm_otp_get_chipvar(&chipvar);
    if ((chipId==0x68560) && (chipvar==3))
        chipId=0x68560B;    
#endif

    return  chipId;
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
    unsigned int revId;
#if defined(CONFIG_BCM947189) || defined(_BCM947189_)
    revId = (MISC->chipid & CID_REV_MASK) >> CID_REV_SHIFT;
#else
    revId = PERF->RevID & REV_ID_MASK;
#endif

    return  revId;
}
EXPORT_SYMBOL(UtilGetChipRev);

unsigned int UtilGetChipIsLP(void)
{
#if defined(_BCM963138_)
    return (UtilGetChipId() == CHIP_63132_ID_HEX);
#elif defined (CONFIG_BCM963138)
    return ( (UtilGetChipId() == CHIP_63132_ID_HEX) || (1 == num_online_cpus()) );
#else
    return 0;
#endif
}
EXPORT_SYMBOL(UtilGetChipIsLP);

char *UtilGetChipName(char *buf, int len) {
#if defined(CONFIG_BCM947189) || defined(_BCM947189_)
    /* 47189 TODO: This is a placeholder only */
    char *mktname = "47189";
    unsigned int chipId = 1;
    unsigned int revId = 1;
#else
    unsigned int chipId;
    unsigned int revId;
    char *mktname = NULL;
    revId = (int) (PERF->RevID & REV_ID_MASK);
#if defined (_BCM96858_) || defined(CONFIG_BCM96858)
    if (bcm_otp_get_chipid(&chipId))
        chipId = 0;
#else
    chipId = UtilGetChipId();
#endif
#endif

#if defined (CONFIG_BCM96858) || defined (_BCM96858_)
    switch (chipId) {
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
#elif defined (CONFIG_BCM96878) || defined (_BCM96878_)
    switch (chipId) {
    case 0x6878A:
        mktname = "68782G";
        break;
    case 0x68781:
    case 0x43505:
        mktname = "68781";
        break;
    case 0x68782:
        mktname = "68782";
        break;
    case 0x68789:
        mktname = "68781H";
        break;
    case 0x6878C:
        mktname = "68789";
        break;
    case 0x6878E:
        mktname = "68782N";
        break;
    case 0x6878D:
        mktname = "68781G";
        break;
    default:
        mktname = NULL;
    }
#elif defined (CONFIG_BCM96846) || defined (_BCM96846_)
    switch (chipId) {
    case(0x68463):
        mktname = "68460U";
        break;
    case(0x68464):
        mktname = "68461S";
    default:
        mktname = NULL;
    }
#elif defined (CONFIG_BCM96855) || defined (_BCM96855_)
    switch (chipId) {
    case(0x68252d):
        mktname = "68252R";
        break;
    case(0x67530):
        mktname = "6753";
    default:
        mktname = NULL;
    }
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
