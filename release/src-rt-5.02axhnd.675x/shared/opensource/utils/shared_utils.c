
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

#ifdef _CFE_
#include "lib_types.h"
#include "lib_printf.h"
#include "lib_string.h"
#include "bcm_map.h"
#define printk  printf
#else // Linux
#include <linux/kernel.h>
#include <linux/module.h>
#include <bcm_map_part.h>
#include <linux/string.h>
#endif

#if defined (CONFIG_BCM96848) || defined(_BCM96848_) || defined (CONFIG_BCM96858) || defined(_BCM96858_)
#include "bcm_otp.h"
#endif

#if defined (CONFIG_BCM96838) || defined(_BCM96838_)
#ifndef _CFE_
#include <linux/spinlock.h>                                   
static DEFINE_SPINLOCK(brcm_sharedUtilslock);
#endif
#endif

unsigned int UtilGetChipId(void)
{
#if defined (CONFIG_BCM960333) || defined (_BCM960333_)
    unsigned int chipId = (PERF->ChipID & CHIP_PART_NUMBER_MASK)
                            >> CHIP_PART_NUMBER_SHIFT;
#elif defined(CONFIG_BCM947189) || defined(_BCM947189_)
    unsigned int chipId = (MISC->chipid & CID_ID_MASK);
#else
    unsigned int chipId = (PERF->RevID & CHIP_ID_MASK) >> CHIP_ID_SHIFT;
#endif

#if defined(CONFIG_BCM94908) || defined(_BCM94908_)
    // 62118 and 62116 use 20 bits to represent the chip id 
    // as compared to 16 in case of 4908/4906
    if((chipId & 0x49000) == 0x49000)
    {
        chipId = chipId >> 4;
    }
#endif

    return  chipId;
}

/* Wrapper of UtilGetChipId for RDPA compilation*/
/* linux kernel func. name format */
unsigned int util_get_chip_id(void) 
{
    return UtilGetChipId();
}

#if !defined(_CFE_)
EXPORT_SYMBOL(UtilGetChipId);
EXPORT_SYMBOL(util_get_chip_id);
#endif

unsigned int UtilGetChipRev(void)
{
    unsigned int revId;
#if defined (CONFIG_BCM96848) || defined(_BCM96848_)
    unsigned int otp_revId = bcm_otp_get_revId();
    if (otp_revId == 0)
        revId = 0xA0;
    else if (otp_revId == 1)
        revId = 0xA1;
    else if (otp_revId == 2)
        revId = 0xB0;

#elif defined(CONFIG_BCM960333) || defined(_BCM960333_)
    revId = (PERF->ChipID & CHIP_VERSION_MASK) >> CHIP_VERSION_SHIFT;
#elif defined(CONFIG_BCM947189) || defined(_BCM947189_)
    revId = (MISC->chipid & CID_REV_MASK) >> CID_REV_SHIFT;
#else
    revId = PERF->RevID & REV_ID_MASK;
#endif

    return  revId;
}

#if !defined(_CFE_)
EXPORT_SYMBOL(UtilGetChipRev);
#endif

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

#if !defined(_CFE_)
EXPORT_SYMBOL(UtilGetChipIsLP);
#endif

char *UtilGetChipName(char *buf, int len) {
#if defined (CONFIG_BCM960333) || defined (_BCM960333_)
    /* DUNA TODO: Complete implementation for 60333 (chipId and revId) */
    char *mktname = "60333";
    unsigned int chipId = (PERF->ChipID & CHIP_PART_NUMBER_MASK)
                            >> CHIP_PART_NUMBER_SHIFT;
    unsigned int revId = (PERF->ChipID & CHIP_VERSION_MASK)
                            >> CHIP_VERSION_SHIFT;
#elif defined(CONFIG_BCM947189) || defined(_BCM947189_)
    /* 47189 TODO: This is a placeholder only */
    char *mktname = "47189";
    unsigned int chipId = 1;
    unsigned int revId = 1;
#else
    unsigned int chipId;
    unsigned int revId;
    char *mktname = NULL;
#if defined (_BCM96848_) || defined(CONFIG_BCM96848)
    revId = UtilGetChipRev();
#else
    revId = (int) (PERF->RevID & REV_ID_MASK);
#endif
#if defined (_BCM96858_) || defined(CONFIG_BCM96858)
    if (bcm_otp_get_chipid(&chipId))
        chipId = 0;
#else
    chipId = UtilGetChipId();
#endif
#endif

#if defined (_BCM96838_) || defined(CONFIG_BCM96838)
    switch (chipId) {
    case(1):
        mktname = "68380";
        break;
    case(3):
        mktname = "68380F";
        break;
    case(4):
        mktname = "68385";
        break;
    case(5):
        mktname = "68381";
        break;
    case(6):
        mktname = "68380M";
        break;
    case(7):
        mktname = "68389";
        break;
    default:
        mktname = NULL;
    }
#elif defined (CONFIG_BCM96848) || defined (_BCM96848_)
    switch (chipId) {
    case(0x050d):
        mktname = "68480F";
        break;
    case(0x05bd):
        mktname = "68485W";
        break;
    case(0x050e):
        mktname = "68485";
        break;
    case(0x050f):
        mktname = "68481";
        break;
    case(0x051a):
        mktname = "68481P";
        break;
    case(0x05c0):
        mktname = "68486";
        break;
    case(0x05be):
        mktname = "68488";
        break;
    default: 
        mktname = NULL;
    }
#elif defined (CONFIG_BCM96858) || defined (_BCM96858_)
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
    case(0x6878A):
        mktname = "68782G";
        break;
    case(0x68789):
        mktname = "68781H";
        break;
    case(0x6878C):
        mktname = "68789";
        break;
    case(0x6878E):
        mktname = "68782N";
        break;
    case(0x6878D):
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
    default:
        mktname = NULL;
    }
#endif

    if (mktname == NULL) {
	sprintf(buf,"%X_%X",chipId,revId);
    } else {
#if  defined (_BCM96838_) || defined(CONFIG_BCM96838)
		sprintf(buf,"%s_", mktname);
		switch(revId)
		{
		case 0:
            strcat(buf,"A0");
			break;
		case 1:
            strcat(buf,"A1");
			break;
		case 4:
            strcat(buf,"B0");
			break;
		case 5:
            strcat(buf,"B1");
			break;
		}
#else
        sprintf(buf,"%s_%X",mktname,revId);
#if defined (CONFIG_BCM96848) || defined(_BCM96848_)
        if ((UtilGetChipRev()==0xA1) && ((PERF->RevID & 0xff) == 0xA1))
            strcat(buf, "_");
#endif
#endif
    }
    return(buf);
}

int UtilGetChipIsPinCompatible(void) 
{

    int ret = 0;

    return(ret);
}

#if defined (CONFIG_BCM96838) || defined(_BCM96838_)
unsigned int gpio_get_dir(unsigned int gpio_num)
{
	if( gpio_num < 32 )
		return (GPIO->GPIODir_low & (1<<gpio_num));
	else if( gpio_num < 64 )
		return (GPIO->GPIODir_mid0 & (1<<(gpio_num-32)));
	else
		return (GPIO->GPIODir_mid1 & (1<<(gpio_num-64)));
}

void gpio_set_dir(unsigned int gpio_num, unsigned int dir)
{
	if(gpio_num<32)
	{ 				
		if(dir)	
			GPIO->GPIODir_low |= GPIO_NUM_TO_MASK(gpio_num);	
		else												
			GPIO->GPIODir_low &= ~GPIO_NUM_TO_MASK(gpio_num);	
	}				
	else if(gpio_num<64) 
	{				
		if(dir)	
			GPIO->GPIODir_mid0 |= GPIO_NUM_TO_MASK(gpio_num-32);	
		else													
			GPIO->GPIODir_mid0 &= ~GPIO_NUM_TO_MASK(gpio_num-32);	
	}				
	else			
	{				
		if(dir)	
			GPIO->GPIODir_mid1 |= GPIO_NUM_TO_MASK(gpio_num-64);	
		else													
			GPIO->GPIODir_mid1 &= ~GPIO_NUM_TO_MASK(gpio_num-64);	
	}
}

unsigned int gpio_get_data(unsigned int gpio_num)
{
	if( gpio_num < 32 )
		return ((GPIO->GPIOData_low & (1<<gpio_num)) >> gpio_num);
	else if( gpio_num < 64 )
		return ((GPIO->GPIOData_mid0 & (1<<(gpio_num-32))) >> (gpio_num-32));
	else
		return ((GPIO->GPIOData_mid1 & (1<<(gpio_num-64))) >> (gpio_num-64));
}

void gpio_set_data(unsigned int gpio_num, unsigned int data)
{
	if(gpio_num<32)
	{ 				
		if(data)	
			GPIO->GPIOData_low |= GPIO_NUM_TO_MASK(gpio_num);	
		else												
			GPIO->GPIOData_low &= ~GPIO_NUM_TO_MASK(gpio_num);	
	}				
	else if(gpio_num<64) 
	{				
		if(data)	
			GPIO->GPIOData_mid0 |= GPIO_NUM_TO_MASK(gpio_num-32);	
		else													
			GPIO->GPIOData_mid0 &= ~GPIO_NUM_TO_MASK(gpio_num-32);	
	}				
	else			
	{				
		if(data)	
			GPIO->GPIOData_mid1 |= GPIO_NUM_TO_MASK(gpio_num-64);	
		else													
			GPIO->GPIOData_mid1 &= ~GPIO_NUM_TO_MASK(gpio_num-64);	
	}
}

void set_pinmux(unsigned int pin_num, unsigned int mux_num)
{
   unsigned int tp_blk_data_lsb;
   
#ifndef _CFE_
   int flags;
   spin_lock_irqsave(&brcm_sharedUtilslock, flags);
#endif

   tp_blk_data_lsb= 0;
   tp_blk_data_lsb |= pin_num;
   tp_blk_data_lsb |= (mux_num << PINMUX_DATA_SHIFT);
   GPIO->port_block_data1 = 0;
   GPIO->port_block_data2 = tp_blk_data_lsb;
   GPIO->port_command = LOAD_MUX_REG_CMD;

#ifndef _CFE_
   spin_unlock_irqrestore(&brcm_sharedUtilslock, flags);	
#endif
}

#endif
#if defined (CONFIG_BCM96838) || defined(_BCM96838_)
#ifndef _CFE_
EXPORT_SYMBOL(set_pinmux);
EXPORT_SYMBOL(gpio_get_dir);
EXPORT_SYMBOL(gpio_set_dir);
EXPORT_SYMBOL(gpio_get_data);
EXPORT_SYMBOL(gpio_set_data);
#endif
#endif
    


