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
#include "lib_printf.h"
#else // Linux
#include <bcm_map_part.h>
#include <linux/string.h>
#include <linux/spinlock.h>
#if !defined(_BCM963268_) && !defined(CONFIG_BCM963268)
static DEFINE_SPINLOCK(otp_spinlock);
#endif
#endif

#define OTP_DEBUG_IF    0
#define OTP_DBG_ENABLE  0

#if OTP_DBG_ENABLE
#if (INC_BTRM_BUILD==1)
#define OTP_DBG(... ) 
#else
#ifndef _CFE_
#   define OTP_DBG_LEVEL      KERN_DEBUG
#   define OTP_DBG(fmt, args...) printk( OTP_DBG_LEVEL "OTP_drv: " fmt, ## args)
#else
#   define OTP_DBG(fmt, args...) printf( "OTP: " fmt, ## args)
#endif /* _CFE_ */
#endif /* INC_BTRM_BUILD==1 */
#else
#   define OTP_DBG(fmt, args...) /* not DBGging: nothing */
#endif /* OTP_DBG_ENABLE */

#if !defined(_BCM963268_) && !defined(CONFIG_BCM963268)

/************************************************************
 *  OTP CORE API                                            *
 ************************************************************/

#if OTP_DEBUG_IF
uint32_t otp_array[100] = {0};
#else
static int otp_wait_status( volatile uint32 * status_reg, int status_mask, int status_val )
{
   int cntr = BTRM_OTP_READ_TIMEOUT_CNT;
   while( cntr && ( (*status_reg & status_mask) != status_val ) )
   {
      cntr--;
   }

   if( cntr )
      return 0;
   else
      return -1;
}
#endif /* OTP_DEBUG_IF */

#if defined(_BCM96838_) || defined(CONFIG_BCM96838) 

/************************************************************
 *  OTP CORE API for 6838 only                              *
 ************************************************************/
static void otp_prog_enable(uint32_t val);
static void otp_prog_enable(uint32_t val) 
{
   OTP->Bitsel = val;
   OTP->Control = OTP_CONTROL_CMD_PROG_ENAB | OTP_CONTROL_START;
   otp_wait_status( &OTP->Status, 
            (OTP_STATUS_CMD_DONE | OTP_STATUS_WRP_DATA_READY |
            OTP_STATUS_WRP_FDONE | OTP_STATUS_OTP_READY),
            (OTP_STATUS_CMD_DONE | OTP_STATUS_WRP_DATA_READY |
            OTP_STATUS_WRP_FDONE | OTP_STATUS_OTP_READY));
}

int bcm_otp_fuse_row(int row, unsigned int val)
{ 
   int ret=0;
#ifndef _CFE_
   unsigned long flags;
#endif   
   // poll for otp_ready, enable cpu mode, point the address register to the proper customer row
   otp_wait_status( &OTP->Status, OTP_STATUS_OTP_READY, OTP_STATUS_OTP_READY);

#ifndef _CFE_
   spin_lock_irqsave(&otp_spinlock, flags);
#endif   

   OTP->Config = OTP_CONFIG_CPU_MODE; 
   OTP->Addr = row;

   // perform the prog enable sequence
   otp_prog_enable(0xf);
   otp_prog_enable(0x4);
   otp_prog_enable(0x8);
   otp_prog_enable(0xd);

   OTP->WriteData = val;
   OTP->Control = OTP_CONTROL_START | OTP_CONTROL_CMD_PROG_WORD | OTP_CONTROL_PROG_EN | OTP_CONTROL_ACCESS_MODE;

   otp_wait_status( &OTP->Status, OTP_STATUS_OTP_READY, OTP_STATUS_OTP_READY);

   if ((OTP->Status & 0xffff0) == (OTP_STATUS_WRP_FDONE | OTP_STATUS_PROG_OK | OTP_STATUS_OTP_READY))
      OTP_DBG("OTP fusing has completed as expected\n");
   else
   {
      OTP_DBG("OTP fusing didn't complete as expected\n");
      ret = -1;
   }

   OTP->Control = 0;
   
#ifndef _CFE_
   spin_unlock_irqrestore(&otp_spinlock, flags);
#endif   
   return ret;
}

int bcm_otp_get_row(int row, unsigned int* val)
{
#if defined(CONFIG_BRCM_IKOS)
   *val =0;
    return 0;
#else
#ifndef _CFE_
   unsigned long flags;
   spin_lock_irqsave(&otp_spinlock, flags);
#endif   
   *val = (*((uint32_t *)(OTP_BASE + row)));
#ifndef _CFE_
   spin_unlock_irqrestore(&otp_spinlock, flags);
#endif   
   return 0;
#endif /* defined(CONFIG_BRCM_IKOS) */   
}
#else

/************************************************************
 *  OTP CORE API for All Devices EXCEPT 6838                *
 ************************************************************/

/************************************************************
 *  int bcm_otp_fuse_row 
 *  Input parameters: 
 *     row   - Row address
 *     value - 32-bit OTP value
 *  Return value:
 *      returns 0 if successful
 ***********************************************************/
int bcm_otp_fuse_row(int row, unsigned int val)
{
    return bcm_otp_fuse_row_ecc(row, val, 0);
}
/************************************************************
 *  int bcm_otp_fuse_row_ecc 
 *  Input parameters: 
 *     row   - Row address
 *     ecc   - row 7 bit ecc  
 *     value - 32-bit OTP value
 *  Return value:
 *      returns 0 if successful
 ***********************************************************/
int bcm_otp_fuse_row_ecc(int row, unsigned int val, unsigned int ecc)
{
   int ret = -1;
#if OTP_DEBUG_IF
   otp_array[addr] |= data;
   ret = 0;
#else      
   int i;
   int authVal[4] = {0xf,0x4,0x8,0xd};

#ifndef _CFE_
   unsigned long flags;
   spin_lock_irqsave(&otp_spinlock, flags);
#endif   

#if defined(WOTP_CPU_LOCK)
   /* Get the hardware spinlock for OTP */
   JTAG_OTP->WOTP_CPU_LOCK = WOTP_CPU_SOFT_LOCK_MASK;
   ret =  otp_wait_status( &JTAG_OTP->WOTP_CPU_LOCK, WOTP_CPU_SOFT_LOCK_MASK, WOTP_CPU_SOFT_LOCK_MASK);
   if( ret )
   {
      OTP_DBG("%s: Error! Timed out waiting for WOTP_CPU_LOCK!\n", __FUNCTION__);
      goto exit_fuse;
   }
#endif

   /* Enable CPU side programming of OTP */
   JTAG_OTP->ctrl1 |= JTAG_OTP_CTRL_CPU_MODE;

   /* Clear row register. A non-empty row register results in a failed prog-enable sequence on some SoCs */
   JTAG_OTP->ctrl3 = 0x0; 

   /* Put OTP in program mode --> prog-enable sequence */
   for (i=0;i<4;i++)
   {
      JTAG_OTP->ctrl2 = authVal[i];
      JTAG_OTP->ctrl0 = JTAG_OTP_CTRL_START | JTAG_OTP_CTRL_CMD_OTP_PROG_EN | JTAG_OTP_CTRL_PROG_EN;
      ret = otp_wait_status( &JTAG_OTP->status1, JTAG_OTP_STATUS_1_CMD_DONE, JTAG_OTP_STATUS_1_CMD_DONE);
      if( ret )
      {
         OTP_DBG("%s: Error! Timed out waiting for OTP command completion (PROG)! status:0x%08x\n", __FUNCTION__, JTAG_OTP->status1);
         goto exit_fuse;
      }
      JTAG_OTP->ctrl0 = 0x0;
   }

   /* Write OTP row */
   if ((JTAG_OTP->status1 & JTAG_OTP_STATUS_1_PROG_OK) == JTAG_OTP_STATUS_1_PROG_OK)
   {
      JTAG_OTP->ctrl2 = val;
#if defined(_BCM96878_) || defined(CONFIG_BCM96878)  || \
   defined(_BCM963158_) || defined(CONFIG_BCM96158)  || \
   defined(_BCM96856_)  || defined(CONFIG_BCM9656)   || \
   defined(_BCM947622_) || defined(CONFIG_BCM947622) || \
   defined(_BCM963178_) || defined(CONFIG_BCM963178) || \
   defined(_BCM96846_)  || defined(CONFIG_BCM9646)
      if (ecc) {
          JTAG_OTP->ctrl2_hi = ecc;
      }
#endif
      JTAG_OTP->ctrl3 = row;

      if( row == OTP_JTAG_CUST_LOCK_ROW )
         JTAG_OTP->ctrl0 = JTAG_OTP_CTRL_START | JTAG_OTP_CTRL_CMD_PROG_LOCK | JTAG_OTP_CTRL_ACCESS_MODE | JTAG_OTP_CTRL_PROG_EN;
      else
         JTAG_OTP->ctrl0 = JTAG_OTP_CTRL_START | JTAG_OTP_CTRL_CMD_PROG | JTAG_OTP_CTRL_ACCESS_MODE | JTAG_OTP_CTRL_PROG_EN;

      ret = otp_wait_status( &JTAG_OTP->status1, JTAG_OTP_STATUS_1_CMD_DONE, JTAG_OTP_STATUS_1_CMD_DONE);
      if( ret )
      {
         OTP_DBG("%s: Error! Timed out waiting for OTP command completion (WRITE)! status: 0x%08x\n", __FUNCTION__, JTAG_OTP->status1);
         goto exit_fuse;
      }
      JTAG_OTP->ctrl0 = 0x0;
      OTP_DBG("OTP fusing has completed as expected\n");
      ret = 0;
   }
   else
   {
      ret = -1;
      OTP_DBG("OTP fusing didn't complete as expected\n");
   }

   JTAG_OTP->ctrl1 &= ~JTAG_OTP_CTRL_CPU_MODE;

exit_fuse:

#if defined(WOTP_CPU_LOCK)
   /* Release hardware spinlock for OTP */
   JTAG_OTP->WOTP_CPU_LOCK=0;
#endif

#ifndef _CFE_
   spin_unlock_irqrestore(&otp_spinlock, flags);
#endif
      
#endif  /* OTP_DEBUG_IF */ 
   return ret;
}

int bcm_otp_get_row(int row, unsigned int* val)
{
   return bcm_otp_get_row_ecc(row, val, NULL);
}
/***********************************************************
 *  int bcm_otp_get_row_ecc
 *  Input parameters: 
 *     row    - Row address
 *     *val - Pointer to 32-bit OTP value
 *     *val_hi - Pointer to 32-bit OTP hi word data (typically ecc)
 *  Return value:
 *      returns 0 if successful, value in *value
 ***********************************************************/
int bcm_otp_get_row_ecc(int row, unsigned int* val, unsigned int* val_hi)
{
#if defined(CONFIG_BRCM_IKOS)
   *val =0;
    return 0;
#else
   int ret = -1;
#if OTP_DEBUG_IF
   *value = otp_array[row];
   ret = 0;
#else      

#ifndef _CFE_
   unsigned long flags;
   spin_lock_irqsave(&otp_spinlock, flags);
#endif   

#if defined(WOTP_CPU_LOCK)
   /* Get the hardware spinlock for OTP */
   JTAG_OTP->WOTP_CPU_LOCK = WOTP_CPU_SOFT_LOCK_MASK;
   ret = otp_wait_status( &JTAG_OTP->WOTP_CPU_LOCK, WOTP_CPU_SOFT_LOCK_MASK, WOTP_CPU_SOFT_LOCK_MASK );
   if( ret )
   {
      OTP_DBG("%s: Error! Timed out waiting for WOTP_CPU_LOCK!\n", __FUNCTION__);
      goto exit_read;
   }
#endif

   /* turn on cpu mode, set up row addr, activate read word */
   JTAG_OTP->ctrl1 |= JTAG_OTP_CTRL_CPU_MODE;
   JTAG_OTP->ctrl3 = row;
   JTAG_OTP->ctrl0 = JTAG_OTP_CTRL_START | JTAG_OTP_CTRL_PROG_EN | JTAG_OTP_CTRL_ACCESS_MODE;

   /* Wait for low CMD_DONE (current operation has begun), reset countdown, wait for retrieval to complete */
   ret = otp_wait_status( &JTAG_OTP->status1, JTAG_OTP_STATUS_1_CMD_DONE, 0);

   /* Wait for high CMD_DONE */
   ret = otp_wait_status( &JTAG_OTP->status1, JTAG_OTP_STATUS_1_CMD_DONE, JTAG_OTP_STATUS_1_CMD_DONE);
   if( ret )
   {
      OTP_DBG("%s: Error! Timed out waiting for OTP command completion (READ)! status: 0x%08x\n", __FUNCTION__, JTAG_OTP->status1);
      goto exit_read;
   }

   /* If read was successful, retrieve data */
   if( ret == 0 )
   {
      *val = JTAG_OTP->status0;
#if defined(_BCM96878_) || defined(CONFIG_BCM96878)  || \
   defined(_BCM963158_) || defined(CONFIG_BCM96158)  || \
   defined(_BCM96856_)  || defined(CONFIG_BCM9656)   || \
   defined(_BCM947622_) || defined(CONFIG_BCM947622) || \
   defined(_BCM963178_) || defined(CONFIG_BCM963178) || \
   defined(_BCM96846_)  || defined(CONFIG_BCM9646)
      if (val_hi) {
         *val_hi = JTAG_OTP->status0_hi;
      }
#endif
   }

   /* zero out the ctrl_0 reg, turn off cpu mode, return results */
   JTAG_OTP->ctrl0 = 0;
   JTAG_OTP->ctrl1 &= ~JTAG_OTP_CTRL_CPU_MODE;

exit_read:

#if defined(WOTP_CPU_LOCK)
   /* Release hardware spinlock for OTP */
   JTAG_OTP->WOTP_CPU_LOCK=0;
#endif

#ifndef _CFE_
   spin_unlock_irqrestore(&otp_spinlock, flags);
#endif
      
#endif  /* OTP_DEBUG_IF */ 
   return ret;
#endif /* CONFIG_BRCM_IKOS */   
}
#endif /* defined(_BCM96838_) || defined(CONFIG_BCM96838) */
#endif /* !defined(_BCM963268_) && !defined(CONFIG_BCM963268) */

/************************************************************
 *  OTP utility functions                                   *
 ************************************************************/

#if defined(_BCM94908_) || defined(CONFIG_BCM94908) || defined(_BCM96858_) || defined(CONFIG_BCM96858) || \
    defined(_BCM963158_) || defined(CONFIG_BCM963158) || defined(_BCM96846_) || defined(CONFIG_BCM96846) || \
    defined(_BCM96856_) || defined(CONFIG_BCM96856) || defined(_BCM963178_) || defined(CONFIG_BCM963178) || \
    defined(_BCM947622_) || defined(CONFIG_BCM947622)

/***********************************************************
 *  int sotp_sec89_both_readlocked(void)
 *  Input parameters: 
 *      none
 *  Return value:
 *      returns 1 if both sotp sections 8 and 9 are locked down from being read
 **********************************************************/
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

/***********************************************************
 *  int bcm_otp_fld_secure_rows(void)
 *  Input parameters: 
 *      none
 *  Return value:
 *      returns 1 if rows fld are fused and readlocked 
 **********************************************************/
int bcm_otp_fld_secure_rows(void)
{
#if defined(_BCM94908_) || defined(CONFIG_BCM94908) || defined(_BCM96858_) || defined(CONFIG_BCM96858) || \
    defined(_BCM963158_) || defined(CONFIG_BCM963158) || defined(_BCM96846_) || defined(CONFIG_BCM96846) || \
    defined(_BCM96856_) || defined(CONFIG_BCM96856) || defined(_BCM963178_) || defined(CONFIG_BCM963178) || \
    defined(_BCM947622_) || defined(CONFIG_BCM947622)
    return !sotp_sec89_both_readlocked();
#elif defined(_BCM96878_) || defined(CONFIG_BCM96878)
    return !((*(volatile uint32_t*)(SEC_KEY_OBJ_BASE+SEC_KEY_OBJ_KEY0_STATUS_OFFSET) >> SEC_KEY_OBJ_KEY0_STATUS_SHIFT) & SEC_KEY_OBJ_KEY0_STATUS_MASK );
#else
    return -1;
#endif 
}
int bcm_otp_is_btrm_boot(void)
{
    int rval = 0;

#if defined(_BCM963268_) || defined(CONFIG_BCM963268)

#if (INC_BTRM_BOOT==1)
   rval = 1;
#else
   /* If either or the brcm or customer OTP bootrom enable OTP */
   /* bits are set, then the bootrom ran at the beginning      */
   rval = ((*(uint32_t *)(OTP_BASE + OTP_SHADOW_ADDR_BTRM_ENABLE_CUST_ROW)) & OTP_CUST_BTRM_BOOT_ENABLE_MASK) >> OTP_CUST_BTRM_BOOT_ENABLE_SHIFT;
#endif

#elif defined(_BCM96838_) || defined(CONFIG_BCM96838)

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
#elif defined(_BCM94908_) || defined(CONFIG_BCM94908) || \
      defined(_BCM963158_) || defined(CONFIG_BCM963158) || defined(_BCM96846_) || defined(CONFIG_BCM96846) || defined(_BCM96856_) || defined(CONFIG_BCM96856) || defined(_BCM96878_) || defined(CONFIG_BCM96878)
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

#if defined(_BCM963268_) || defined(CONFIG_BCM963268)

#if (INC_BTRM_BOOT==1)
   rval = 1;
#else
   rval = bcm_otp_is_btrm_boot();
   if (rval)
   {
      rval = ((*(uint32_t *)(OTP_BASE + OTP_SHADOW_ADDR_MARKET_ID_CUST_ROW)) & OTP_MFG_MRKTID_OTP_BITS_MASK) >> OTP_MFG_MRKTID_OTP_BITS_SHIFT;
      if (rval) 
         rval = 1;
   }
#endif

#elif defined(_BCM96838_) || defined(CONFIG_BCM96838)

   rval = bcm_otp_is_btrm_boot();
   if (rval)
   {
      rval = ((*(uint32_t *)(OTP_BASE + OTP_SHADOW_ADDR_MARKET_ID_CUST_ROW)) & OTP_MFG_MRKTID_OTP_BITS_MASK) >> OTP_MFG_MRKTID_OTP_BITS_SHIFT;
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
    defined(_BCM963158_) || defined(CONFIG_BCM963158) || \
    defined(_BCM96856_) || defined(CONFIG_BCM96856) || defined(_BCM96846_) || defined(CONFIG_BCM96846)
        /* For these targets, the mode still might be mfg secure if sotp section 8 or 9 are not readlocked */
        if (bcm_otp_fld_secure_rows())
           rval = 0;
#endif
    }
    else
        rval = 0;

#endif

    return rval;
}


#if defined(_BCM94908_) || defined(CONFIG_BCM94908) || defined(_BCM96858_) || defined(CONFIG_BCM96858) || \
    defined(_BCM963158_) || defined(CONFIG_BCM963158) || defined(_BCM96878_) || defined(CONFIG_BCM96878) || \
    defined(_BCM96846_) || defined(CONFIG_BCM96846) || defined(_BCM96856_) || defined(CONFIG_BCM96856) || \
    defined(_BCM963178_) || defined(CONFIG_BCM963178) || defined(_BCM947622_) || defined(CONFIG_BCM947622)
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
          if (!bcm_otp_fld_secure_rows())
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

#if defined(_BCM94908_) || defined(CONFIG_BCM94908) || defined(_BCM947622_) || defined(CONFIG_BCM947622)
int bcm_otp_is_sgmii_disabled(unsigned int* val)
{
    unsigned int row;
    int rval = bcm_otp_get_row(OTP_SGMII_DISABLE_ROW, &row);
    *val = (row & OTP_SGMII_DISABLE_MASK) >> OTP_SGMII_DISABLE_SHIFT;

    return rval;
}
#endif

#if defined(_BCM96858_) || defined(CONFIG_BCM96858) || defined(_BCM96878_) || defined(CONFIG_BCM96878) || \
    defined(_BCM96846_) || defined(CONFIG_BCM96846) || defined(_BCM96856_) || defined(CONFIG_BCM96856) || \
    defined(_BCM947622_) || defined(CONFIG_BCM947622)
int bcm_otp_get_cpu_clk(unsigned int* val)
{
    unsigned int row;
    int rval = bcm_otp_get_row(OTP_CPU_CLOCK_FREQ_ROW, &row);
    *val = (row & OTP_CPU_CLOCK_FREQ_MASK) >> OTP_CPU_CLOCK_FREQ_SHIFT; 

    return rval;
}
#endif

int bcm_otp_is_usb3_disabled(unsigned int* val)
{
    int rval = 0; 
#if defined(_BCM947622_) || defined(CONFIG_BCM947622)
    unsigned int row;
    rval = bcm_otp_get_row(OTP_USB3_DISABLE_ROW, &row);

    if(row & OTP_USB3_DISABLE_MASK)
        *val = 1;
    else
        *val = 0;
#else
    *val = 0;
#endif

    return rval;
}

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
    defined(_BCM963158_) || defined(CONFIG_BCM963158) || \
    defined(_BCM96846_) || defined(CONFIG_BCM96846) || \
    defined(_BCM96878_) || defined(CONFIG_BCM96878) || \
    defined(_BCM96856_) || defined(CONFIG_BCM96856) || \
    defined(_BCM963178_) || defined(CONFIG_BCM963178) || \
    defined(_BCM947622_) || defined(CONFIG_BCM947622)

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

#if defined(_BCM96858_) || defined(CONFIG_BCM96858)
int bcm_otp_get_pmc_boot_sts(unsigned int* val)
{
    unsigned int row;
    int rval = bcm_otp_get_row(OTP_PMC_BOOT_ROW, &row);
    *val = (row & OTP_PMC_BOOT_MASK) >> OTP_PMC_BOOT_SHIFT;

    return rval;
}
#endif

#if !defined(_BCM96846_) && !defined(CONFIG_BCM96846) && !defined(_BCM96856_) && !defined(CONFIG_BCM96856) && \
    !defined(_BCM963178_) && !defined(CONFIG_BCM963178) && !defined(_BCM947622_) && !defined(CONFIG_BCM947622) && \
    !defined(_BCM96878_) && !defined(CONFIG_BCM96878)
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

#if defined(_BCM947622_) || defined(CONFIG_BCM947622)
int bcm_otp_is_pcie_port_disabled(unsigned int pcie_port_num, unsigned int* val)
{
    unsigned int row;
    int rval = bcm_otp_get_row(OTP_PCIE_PORT_DISABLE_ROW, &row);
    row &= (OTP_PCIE_PORT_DISABLE_MASK);
    row = row >> OTP_PCIE_PORT_DISABLE_SHIFT;
    *val = row & (1 << pcie_port_num);
    return rval;
}
#endif

#if defined(_BCM947622_) || defined(CONFIG_BCM947622) || defined(_BCM963178_) || defined(CONFIG_BCM963178)
int bcm_otp_get_ldo_trim(unsigned int* val)
{
    unsigned int row;
    int rval = bcm_otp_get_row(OTP_LDO_TRIM_ROW, &row);

    *val = (row & OTP_LDO_TRIM_MASK) >> OTP_LDO_TRIM_SHIFT;

    return rval;
}
#endif


#if !defined(_CFE_)
EXPORT_SYMBOL(bcm_otp_is_btrm_boot);
EXPORT_SYMBOL(bcm_otp_is_boot_secure);

#if !defined(_BCM963268_) && !defined(CONFIG_BCM963268)
EXPORT_SYMBOL(bcm_otp_get_row);
EXPORT_SYMBOL(bcm_otp_fuse_row);
#if !defined(_BCM96838_) && !defined(CONFIG_BCM96838)
EXPORT_SYMBOL(bcm_otp_fuse_row_ecc);
EXPORT_SYMBOL(bcm_otp_get_row_ecc);
#endif
#endif

#if defined (CONFIG_BCM96848)
EXPORT_SYMBOL(bcm_otp_get_revId);
EXPORT_SYMBOL(bcm_otp_get_max_clksel);
#endif

#if defined (CONFIG_BCM96858) || defined(CONFIG_BCM94908) || \
    defined(CONFIG_BCM963158) || defined(CONFIG_BCM96878) || \
    defined(CONFIG_BCM96846) || defined(CONFIG_BCM96856)
EXPORT_SYMBOL(bcm_otp_get_nr_cpus);
EXPORT_SYMBOL(bcm_otp_is_boot_mfg_secure);
EXPORT_SYMBOL(bcm_otp_is_pcm_disabled);

#if defined(_BCM94908_) || defined(CONFIG_BCM94908) || defined(_BCM947622_) || defined(CONFIG_BCM947622)
EXPORT_SYMBOL(bcm_otp_is_sgmii_disabled);
#endif

#if defined (CONFIG_BCM96858) || defined(CONFIG_BCM96846) || defined(CONFIG_BCM96878) || defined(CONFIG_BCM96856) || defined(CONFIG_BCM947622)
EXPORT_SYMBOL(bcm_otp_get_cpu_clk);
#endif

EXPORT_SYMBOL(bcm_otp_is_usb3_disabled);

#if defined(CONFIG_BCM96858)
EXPORT_SYMBOL(bcm_otp_get_usb_port_disabled);
EXPORT_SYMBOL(bcm_otp_get_pmc_boot_sts);
#endif

#if !defined(CONFIG_BCM96846) && !defined(CONFIG_BCM96856) && !defined(CONFIG_BCM96878) && !defined(CONFIG_BCM963178) && \
    !defined(CONFIG_BCM947622)
EXPORT_SYMBOL(bcm_otp_is_sata_disabled);
#endif

#endif
#endif /* !defined(_CFE_) */

