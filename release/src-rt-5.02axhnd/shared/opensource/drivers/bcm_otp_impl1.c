/*
 * <:copyright-BRCM:2013:DUAL/GPL:standard
 * 
 *    Copyright (c) 2013 Broadcom 
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
// BCMFORMAT: notabs reindent:uncrustify:bcm_minimal_i4.cfg

/*
  These are low level functions that can be called from CFE or from the Linux init code.
  If pinmux changes are needed after Linux init, support should be added to the board 
  driver including any necessary locking.
*/

#include "bcm_otp.h"

#ifdef _CFE_                                                
#include "lib_types.h"
#include "lib_string.h"
#include "bcm_map_part.h"
#include "rom_parms.h"
#define printk  printf
#else // Linux
#include <bcm_map_part.h>
#include <linux/string.h>
#include <linux/spinlock.h>

static DEFINE_SPINLOCK(otp_spinlock);
#endif

#if defined(CONFIG_BCM94908)
#define POTP_READ JTAGOTP
#else
#define POTP_READ JTAG_OTP
#endif

#if ! defined(_BCM963268_) && ! defined(_BCM96838_)
/**********************************************
** bcm_otp_get_row                           **
** This function reads otp row               **
** INPUT:                                    **
**     row  - The row number to read from    **
**     val  - The row val returned value     **
** OUTPUT:                                   **
**     0 on success, -1 on fail.             **
***********************************************/
int bcm_otp_get_row(int row, unsigned int* val)
{

#if defined(CONFIG_BRCM_IKOS)
    *val =0;
    return 0;
#else
    int      rval = 0;
    uint32_t cntr = BTRM_OTP_READ_TIMEOUT_CNT;

#ifndef _CFE_
    unsigned long flags;

    spin_lock_irqsave(&otp_spinlock, flags);
#endif

    /* turn on cpu mode, set up row addr, activate read word */
    POTP_READ->ctrl1 |= JTAG_OTP_CTRL_CPU_MODE;
    POTP_READ->ctrl3 = row;
    POTP_READ->ctrl0 = JTAG_OTP_CTRL_START | JTAG_OTP_CTRL_PROG_EN | JTAG_OTP_CTRL_ACCESS_MODE;

    /* Wait for low CMD_DONE (current operation has begun), reset countdown, wait for retrieval to complete */
    while(!((POTP_READ->status1) & JTAG_OTP_STATUS_1_CMD_DONE))
    {
        cntr--;
        if (cntr == 0)
        {
            rval = -1;
             break;
        }
    }

    /* If cntr nonzero, read was successful, retrieve data */
    if (cntr)
        *val = POTP_READ->status0;

    /* zero out the ctrl_0 reg, turn off cpu mode, return results */
    POTP_READ->ctrl0 = 0;
    POTP_READ->ctrl1 &= ~JTAG_OTP_CTRL_CPU_MODE;

#ifndef _CFE_
    spin_unlock_irqrestore(&otp_spinlock, flags);
#endif

    return rval;
#endif
}
#endif



#if defined(_BCM94908_) || defined(CONFIG_BCM94908) || defined(_BCM96858_) || defined(CONFIG_BCM96858) || \
    defined(_BCM96836_) || defined(CONFIG_BCM96836) || defined(_BCM963158_) || defined(CONFIG_BCM963158) || \
    defined(_BCM96846_) || defined(CONFIG_BCM96846) || defined(_BCM96856_) || defined(CONFIG_BCM96856)

/*  *********************************************************************
 *  int sotp_sec89_both_readlocked(void)
 *  
 *  Input parameters: 
 *      none
 *      
 *  Return value:
 *      returns 1 if both sotp sections 8 and 9 are locked down from being read
 *
 ********************************************************************* */
int sotp_sec89_both_readlocked(void)
{
   int rval = 0;
   uint32_t sotpOtpRegionRdLockReg  = *((uint32_t *)(SOTP_BASE + SOTP_OTP_REGION_RD_LOCK));

   /* Check to see if the SOTP register set is even accessible.         */
   if (sotpOtpRegionRdLockReg == 0xdeadbeef)
      rval = 1;
   else
   {
      /* regions 10 thru 12 are section 8; regions 13 thru 15 are section 9 */
      /* so a mask of 0x3f is enough bits to check both sections in one shot*/
      if (((sotpOtpRegionRdLockReg >> 10) & 0x3f) == 0x3f)
         rval = 1;
   }

   return rval;
}
#endif

int bcm_otp_is_btrm_boot(void)
{
    int rval = 0;

#if defined(_BCM963268_)

#if (INC_BTRM_BOOT==1)
   rval = 1;
#else
   rval = 0;
#endif

#elif defined(_BCM96838_)

   /* If either or the brcm or customer OTP bootrom enable OTP */
   /* bits are set, then the bootrom ran at the beginning      */
   rval = ((*(uint32_t *)(OTP_BASE + OTP_SHADOW_ADDR_BTRM_ENABLE_BTRM_ROW)) & OTP_BRCM_BTRM_BOOT_ENABLE_MASK) >> OTP_BRCM_BTRM_BOOT_ENABLE_SHIFT;
   rval |= ((*(uint32_t *)(OTP_BASE + OTP_SHADOW_ADDR_BTRM_ENABLE_CUST_ROW)) & OTP_CUST_BTRM_BOOT_ENABLE_MASK) >> OTP_CUST_BTRM_BOOT_ENABLE_SHIFT;

#else
    unsigned int row;

#if !defined(_BCM963148_) && !defined(CONFIG_BCM963148)

#if defined(_BCM96858_) || defined(CONFIG_BCM96858) || defined(_BCM963158_)
    uint32 bootsel = ((MISC->miscStrapBus & MISC_STRAP_BUS_BOOT_SEL0_4_MASK) >> MISC_STRAP_BUS_BOOT_SEL0_4_SHIFT) |
                     ((MISC->miscStrapBus & MISC_STRAP_BUS_BOOT_SEL5_MASK) >> BOOT_SEL5_STRAP_ADJ_SHIFT);
    if ( ((bootsel & BOOT_SEL_STRAP_BOOT_SEL_MASK) != BOOT_SEL_STRAP_SPI_NOR) ||
          !(MISC->miscStrapBus & MISC_STRAP_BUS_BOOROM_BOOT_N) )
#elif defined(_BCM963381_) || defined(CONFIG_BCM963381)
    if ((MISC->miscStrapBus & MISC_STRAP_BUS_SPI_NAND_DISABLE) == 0)
#elif defined (CONFIG_BCM96848) || defined(_BCM96848_)
    if ((MISC->miscStrapBus & MISC_STRAP_BUS_BOOT_SEL_MASK) == MISC_STRAP_BUS_BOOT_SPI_NAND)
#elif defined(_BCM963138_) || defined(CONFIG_BCM963138)
    if ((MISC->miscStrapBus & MISC_STRAP_BUS_SW_BOOT_NORMAL_MASK) == 0)
#elif defined(_BCM94908_) || defined(CONFIG_BCM94908) || defined(_BCM96836_) || defined(CONFIG_BCM96836) || \
      defined(_BCM963158_) || defined(CONFIG_BCM963158) || defined(_BCM96846_) || defined(CONFIG_BCM96846) || defined(_BCM96856_) || defined(CONFIG_BCM96856)
    if ((MISC->miscStrapBus & MISC_STRAP_BUS_BOOTROM_BOOT_N) == 0)
#endif
    {
        return 1;
    }
#endif

    rval = bcm_otp_get_row(OTP_BRCM_BTRM_BOOT_ENABLE_ROW, &row);
    if ( !rval && (row & OTP_BRCM_BTRM_BOOT_ENABLE_MASK) )
    {
        rval = bcm_otp_get_row( OTP_CUST_BTRM_BOOT_ENABLE_ROW, &row);
        if ( !rval && (row & OTP_CUST_BTRM_BOOT_ENABLE_MASK) )
            rval = 1;
        else
            rval = 0;
    }
    else
        rval = 0;

#endif

    return rval;
}


int bcm_otp_is_boot_secure(void)
{
    int rval;

#if defined(_BCM963268_)

#if (INC_BTRM_BOOT==1)
   rval = 1;
#else
   rval = 0;
#endif

#elif defined(_BCM96838_)

   rval = bcm_otp_is_btrm_boot();
   if (rval)
   {
      rval = ((*(uint32_t *)(OTP_BASE + OTP_SHADOW_ADDR_MARKET_ID_CUST_ROW)) & OTP_MFG_MRKTID_OTP_BITS_MASK);
      if (rval) 
         rval = 1;
   }

#else

    unsigned int bcmBtrmEn, cusBtrmEn, cusMtkid;

#if defined(_BTRM_DEVEL_)
    return ROM_ARG_ISSET(ROM_ARG_SEC_FLD_DEVEL);
#endif
    rval = bcm_otp_get_row(OTP_BRCM_BTRM_BOOT_ENABLE_ROW, &bcmBtrmEn);
    rval |= bcm_otp_get_row( OTP_CUST_BTRM_BOOT_ENABLE_ROW, &cusBtrmEn);
    rval |= bcm_otp_get_row( OTP_CUST_MFG_MRKTID_ROW, &cusMtkid);
    
    if ( !rval && (bcmBtrmEn & OTP_BRCM_BTRM_BOOT_ENABLE_MASK) &&
          (cusBtrmEn & OTP_CUST_BTRM_BOOT_ENABLE_MASK) && 
          (cusMtkid & OTP_CUST_MFG_MRKTID_MASK) )
    {
        rval = 1;
#if defined(_BCM94908_) || defined(CONFIG_BCM94908) || defined(_BCM96858_) || defined(CONFIG_BCM96858) || \
    defined(_BCM96836_) || defined(CONFIG_BCM96836) || defined(_BCM963158_) || defined(CONFIG_BCM963158) || \
    defined(_BCM96856_) || defined(CONFIG_BCM96856) || defined(_BCM96846_) || defined(CONFIG_BCM96846)
        /* For these targets, the mode still might be mfg secure if sotp section 8 or 9 are not readlocked */
        if (! sotp_sec89_both_readlocked())
           rval = 0;
#endif
    }
    else
        rval = 0;

#endif

    return rval;
}


#if defined(_BCM94908_) || defined(CONFIG_BCM94908) || defined(_BCM96858_) || defined(CONFIG_BCM96858) || \
    defined(_BCM96836_) || defined(CONFIG_BCM96836) || defined(_BCM963158_) || defined(CONFIG_BCM963158) || \
    defined(_BCM96846_) || defined(CONFIG_BCM96846) || defined(_BCM96856_) || defined(CONFIG_BCM96856)
int bcm_otp_is_boot_mfg_secure(void)
{
    int rval;
    unsigned int bcmBtrmEn, cusBtrmEn, cusMtkid;

#if defined(_BTRM_DEVEL_)
    return ROM_ARG_ISSET(ROM_ARG_SEC_MFG_DEVEL);
#endif

    rval = bcm_otp_get_row(OTP_BRCM_BTRM_BOOT_ENABLE_ROW, &bcmBtrmEn);
    rval |= bcm_otp_get_row( OTP_CUST_BTRM_BOOT_ENABLE_ROW, &cusBtrmEn);
    rval |= bcm_otp_get_row( OTP_CUST_MFG_MRKTID_ROW, &cusMtkid);
    
    if ( !rval && (bcmBtrmEn & OTP_BRCM_BTRM_BOOT_ENABLE_MASK) &&
          (cusBtrmEn & OTP_CUST_BTRM_BOOT_ENABLE_MASK) )
    {
       if (cusMtkid & OTP_CUST_MFG_MRKTID_MASK)
       {
          if (sotp_sec89_both_readlocked())
             rval = 0; /* field secure */
          else
             rval = 1; /* mfg secure */
       }
       else
          rval = 1; /* mfg secure */
    }
    else
       rval = 0; /* unsecure */

    return rval;
}
#endif


#if defined (CONFIG_BCM96848) || defined(_BCM96848_)
int bcm_otp_get_revId(void)
{
    unsigned int row;
    int rval = bcm_otp_get_row(OTP_BRCM_CHIPID_ROW, &row);

    if (!rval)
        rval = (row & OTP_BRCM_CHIP_REVID_MASK) >> OTP_BRCM_CHIP_REVID_SHIFT;

    return rval;
}

int bcm_otp_get_max_clksel(void)
{
    unsigned int row;
    int rval = bcm_otp_get_row(OTP_BRCM_FEATURE_DISABLE_ROW, &row);

    if (!rval)
        rval = (row & OTP_BRCM_MAX_CLOCK_FREQ_MASK) >> OTP_BRCM_MAX_CLOCK_FREQ_SHIFT;

    return rval;
}
#endif

#if defined(_BCM94908_) || defined(CONFIG_BCM94908)
int bcm_otp_is_sgmii_disabled(unsigned int* val)
{
    unsigned int row;
    int rval = bcm_otp_get_row(OTP_SGMII_DISABLE_ROW, &row);
    *val = (row & OTP_SGMII_DISABLE_MASK) >> OTP_SGMII_DISABLE_SHIFT;

    return rval;
}
#endif

#if defined(_BCM96858_) || defined(CONFIG_BCM96858) || defined(_BCM96836_) || defined(CONFIG_BCM96836) || \
    defined(_BCM96846_) || defined(CONFIG_BCM96846) || defined(_BCM96856_) || defined(CONFIG_BCM96856)
int bcm_otp_get_cpu_clk(unsigned int* val)
{
    unsigned int row;
    int rval = bcm_otp_get_row(OTP_CPU_CLOCK_FREQ_ROW, &row);
    *val = (row & OTP_CPU_CLOCK_FREQ_MASK) >> OTP_CPU_CLOCK_FREQ_SHIFT; 

    return rval;
}
#endif

#if defined(_BCM96858_) || defined(CONFIG_BCM96858)
int bcm_otp_get_chipid(unsigned int* val)
{
    unsigned int row;
    int rval = bcm_otp_get_row(OTP_SEC_CHIPVAR_ROW, &row);
    *val = (row & OTP_SEC_CHIPVAR_MASK) >> OTP_SEC_CHIPVAR_SHIFT;

    return rval;
}

int bcm_otp_get_usb_port_disabled(int port, unsigned int* val)
{
    unsigned int row;
    int rval = bcm_otp_get_row(OTP_USB_DISABLE_ROW, &row);
    switch (port)
    {
        case 0:
            *val = (row & OTP_USB_DISABLE0_MASK) >> OTP_USB_DISABLE0_SHIFT;
            break;
        case 1:
            *val = (row & OTP_USB_DISABLE1_MASK) >> OTP_USB_DISABLE1_SHIFT;
            break;
        default:
            *val = (row & OTP_USB_DISABLE_XHCI_MASK) >> OTP_USB_DISABLE_XHCI_SHIFT;
    }

    return rval;
}
#endif

#if defined(_BCM96858_) || defined(CONFIG_BCM96858) || \
    defined(_BCM94908_) || defined(CONFIG_BCM94908) || \
    defined(_BCM96836_) || defined(CONFIG_BCM96836) || \
    defined(_BCM963158_) || defined(CONFIG_BCM963158) || \
    defined(_BCM96846_) || defined(CONFIG_BCM96846) || \
    defined(_BCM96856_) || defined(CONFIG_BCM96856)

#if defined(_BCM96846_) || defined(CONFIG_BCM96846)
extern unsigned int UtilGetChipRev(void);
#endif

int bcm_otp_get_nr_cpus(unsigned int* val)
{
    unsigned int row;
    int rval = bcm_otp_get_row(OTP_CPU_CORE_CFG_ROW, &row);
    *val = (row & OTP_CPU_CORE_CFG_MASK) >> OTP_CPU_CORE_CFG_SHIFT;

#if defined(_BCM96858_) || defined(CONFIG_BCM96858)
    {
        unsigned int chipId = 0;

        bcm_otp_get_chipid(&chipId);
        if (chipId == 0x5 || chipId == 0x2 || chipId == 0xb)
            *val = 2;
    }
#endif
#if defined(_BCM96846_) || defined(CONFIG_BCM96846)
    {
        if (UtilGetChipRev() == 0xA0)
            *val = 1;
        else 
            *val = 0;
    }
#endif
    return rval;
}

#if !defined(_BCM96846_) && !defined(CONFIG_BCM96846) && !defined(_BCM96856_) && !defined(CONFIG_BCM96856)
int bcm_otp_get_pmc_boot_sts(unsigned int* val)
{
    unsigned int row;
    int rval = bcm_otp_get_row(OTP_PMC_BOOT_ROW, &row);
    *val = (row & OTP_PMC_BOOT_MASK) >> OTP_PMC_BOOT_SHIFT;

    return rval;
}

int bcm_otp_is_sata_disabled(unsigned int* val)
{
    unsigned int row;
    int rval = bcm_otp_get_row(OTP_SATA_DISABLE_ROW, &row);
    *val = (row & OTP_SATA_DISABLE_MASK) >> OTP_SATA_DISABLE_SHIFT;

    return rval;
}
#endif
int bcm_otp_is_pcm_disabled(unsigned int* val)
{
    unsigned int row;
    int rval = bcm_otp_get_row(OTP_PCM_DISABLE_ROW, &row);
    *val = (row & OTP_PCM_DISABLE_MASK) >> OTP_PCM_DISABLE_SHIFT;

    return rval;
}
#endif

#if !defined(_CFE_)
EXPORT_SYMBOL(bcm_otp_get_row);
EXPORT_SYMBOL(bcm_otp_is_btrm_boot);
EXPORT_SYMBOL(bcm_otp_is_boot_secure);
#if defined (CONFIG_BCM96848) || defined(_BCM96848_)
EXPORT_SYMBOL(bcm_otp_get_revId);
EXPORT_SYMBOL(bcm_otp_get_max_clksel);
#endif
#if defined (CONFIG_BCM96858) || defined(_BCM96858_) || defined(CONFIG_BCM94908) || defined(_BCM94908_) || \
    defined(CONFIG_BCM96836) || defined(_BCM96836_) || defined(_BCM963158_) || defined(CONFIG_BCM963158) || \
    defined(CONFIG_BCM96846) || defined(_BCM96846_) || defined(_BCM96856_)  || defined(CONFIG_BCM96856)
#if defined(_BCM94908_) || defined(CONFIG_BCM94908)
EXPORT_SYMBOL(bcm_otp_is_sgmii_disabled);
#endif
#if defined (CONFIG_BCM96858) || defined(_BCM96858_) || \
    defined(CONFIG_BCM96836) || defined(_BCM96836_) || \
    defined(CONFIG_BCM96846) || defined(_BCM96846_) || \
    defined(CONFIG_BCM96856) || defined(_BCM96856_)
EXPORT_SYMBOL(bcm_otp_get_cpu_clk);
#endif
#if defined(_BCM96858_) || defined(CONFIG_BCM96858)
EXPORT_SYMBOL(bcm_otp_get_usb_port_disabled);
#endif

EXPORT_SYMBOL(bcm_otp_get_nr_cpus);
#if !defined(_BCM96846_) && !defined(CONFIG_BCM96846) && !defined(_BCM96856_) && !defined(CONFIG_BCM96856)
EXPORT_SYMBOL(bcm_otp_get_pmc_boot_sts);
#endif
EXPORT_SYMBOL(bcm_otp_is_boot_mfg_secure);
EXPORT_SYMBOL(bcm_otp_is_pcm_disabled);
#if !defined(_BCM96846_) && !defined(CONFIG_BCM96846) && !defined(_BCM96856_) && !defined(CONFIG_BCM96856)
EXPORT_SYMBOL(bcm_otp_is_sata_disabled);
#endif
#endif
#endif

