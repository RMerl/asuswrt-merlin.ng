/*
<:label-BRCM:2012:NONE:standard

:>
*/

/*  *********************************************************************
    *  
    *  bcm63xx_sotp_if.c       
    *
    *  Author:  Brian Nay (brian.nay@broadcom.com)
    *  
    *********************************************************************  
    *
    *  Copyright 2011
    *  Broadcom Corporation. All rights reserved.
    *  
    *  This program is the proprietary software of Broadcom Corporation and/or its
    *  licensors, and may only be used, duplicated, modified or distributed pursuant
    *  to the terms and conditions of a separate, written license agreement executed
    *  between you and Broadcom (an "Authorized License").  Except as set forth in
    *  an Authorized License, Broadcom grants no license (express or implied), right
    *  to use, or waiver of any kind with respect to the Software, and Broadcom
    *  expressly reserves all rights in and to the Software and all intellectual
    *  property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
    *  NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
    *  BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
    * 
    *  Except as expressly set forth in the Authorized License,
    * 
    *  1. This program, including its structure, sequence and organization,
    *     constitutes the valuable trade secrets of Broadcom, and you shall use
    *     all reasonable efforts to protect the confidentiality thereof, and to
    *     use this information only in connection with your use of Broadcom
    *     integrated circuit products.
    * 
    *  2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
    *     AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
    *     WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
    *     RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
    *     ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
    *     FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
    *     COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
    *     TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
    *     PERFORMANCE OF THE SOFTWARE.
    * 
    *  3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
    *     ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
    *     INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
    *     WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
    *     IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
    *     OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
    *     SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
    *     SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
    *     LIMITED REMEDY.
    * :>
    *
    ********************************************************************* */

#include "bcm63xx_sotp.h"
#include "lib_string.h"
#include "lib_printf.h"
#include "lib_crc.h"
#include "bcm_map.h"

#if (CFG_RAMAPP)

#define info(... ) { printf( __VA_ARGS__ ); }
#define print(... ) { }

#else

#define info(... ) { }
#if (INC_BTRM_BUILD==1)
extern int btrm_print(int);
#define print(... ) {btrm_print( __VA_ARGS__); }
#else
#define print(... ) { }
#endif

#endif

extern void cfe_usleep(int);

#if 0
static void print_32(uint32_t val)
{
   uint32_t i, nibble, ascii=0;
   for (i = 8; i < 0; i--)
   {
      nibble = (val >> ((i-1)*4)) & 0xf;

      if (nibble < 10)
         ascii |= nibble+0x30;
      else
         ascii |= nibble+0x51;

      if ((i == 5) || (i == 1))
         print(ascii);

      ascii = ascii << 8;
   }
}
#endif

/*  *********************************************************************
    *  static SotpRowStatus sotpPollForFDone(void)
    *  
    *  Input parameters: 
    *  	   none 
    *  	   
    *  Return value:
    *  	   SOTP_S_ROW_SUCCESS - The function has completed successfully
    *  	   SOTP_E_ROW_TIMEOUT - The function experienced unexpected FSM timeout 
    *  	                        from the SOTP block 
    *
    ********************************************************************* */
static SotpRowStatus sotpPollForFDone(void)
{
   SotpRowStatus rval = SOTP_S_ROW_SUCCESS;

   volatile uint32_t *pSotpOut0Reg = (uint32_t *)(SOTP_BASE + SOTP_SOTP_OUT_0);
   uint32_t reg_rd_data;
   uint32_t cntr = 0;

   reg_rd_data = *pSotpOut0Reg;
   while (( ! (reg_rd_data & SOTP_SOTP_OUT_0_SOTP_OTP_READY)) && (cntr < SOTP_MAX_CNTR))
   {
      cntr++;
      cfe_usleep(100);
      reg_rd_data = *pSotpOut0Reg;
   }

   if (cntr == SOTP_MAX_CNTR)
   {
      rval = SOTP_E_ROW_TIMEOUT;
      info("\nSOTP: Timeout waiting for FDONE. \n");
   }

   return rval;
}



/*  *********************************************************************
    *  static SotpRowStatus sotpSetStartAndPollForCmdDone(void)
    *  
    *  Input parameters: 
    *  	   none 
    *  	   
    *  Return value:
    *  	   SOTP_S_ROW_SUCCESS - The function has completed successfully
    *  	   SOTP_E_ROW_TIMEOUT - The function experienced unexpected FSM timeout 
    *  	                        from the SOTP block 
    *
    ********************************************************************* */
static SotpRowStatus sotpSetStartAndPollForCmdDone(void)
{
   SotpRowStatus rval = SOTP_S_ROW_SUCCESS;

   volatile uint32_t *pSotpOtpCtrl0Reg     = (uint32_t *)(SOTP_BASE + SOTP_OTP_CTRL_0);
   volatile uint32_t *pSotpOtpStatus1Reg   = (uint32_t *)(SOTP_BASE + SOTP_OTP_STATUS_1);

   uint32_t reg_rd_data;
   uint32_t cntr = 0;

   /* Set the start bit */
   *pSotpOtpCtrl0Reg |= SOTP_OTP_CTRL_0_START;

   cfe_usleep(100);

   /* Wait for the FSM to say it is done */
   reg_rd_data = *pSotpOtpStatus1Reg;
   while (( ! (reg_rd_data & SOTP_OTP_STATUS_1_CMD_DONE)) && (cntr < SOTP_MAX_CNTR))
   {
      cntr++;
      cfe_usleep(100);
      reg_rd_data = *pSotpOtpStatus1Reg;
   }

   if (cntr != SOTP_MAX_CNTR)
   {
      /* Clear the start bit */
      *pSotpOtpCtrl0Reg &= ~SOTP_OTP_CTRL_0_START;
   }
   else
   {
      rval = SOTP_E_ROW_TIMEOUT;
      info("\nSOTP: Timeout waiting for CMD_DONE. \n");
   }

   return rval;
}



/*  *********************************************************************
    *  static SotpRowStatus sotpPrepForFusing(void)
    *  
    *  Input parameters: 
    *      none  	   
    *
    *  Return value:
    *  	   SOTP_S_ROW_SUCCESS - The function has completed successfully
    *  	   SOTP_E_ROW_TIMEOUT - The function experienced unexpected FSM timeout 
    *  	                        from the SOTP block 
    *
    ********************************************************************* */
static SotpRowStatus sotpPrepForFusing(void)
{
   SotpRowStatus rval = SOTP_S_ROW_SUCCESS;

   volatile uint32_t *pSotpOtpProgCtrlReg  = (uint32_t *)(SOTP_BASE + SOTP_OTP_PROG_CTRL);
   volatile uint32_t *pSotpOtpCtrl0Reg     = (uint32_t *)(SOTP_BASE + SOTP_OTP_CTRL_0);
   volatile uint32_t *pSotpChipCtrlReg     = (uint32_t *)(SOTP_BASE + SOTP_CHIP_CTRL);
   volatile uint32_t *pSotpOtpStatus1Reg   = (uint32_t *)(SOTP_BASE + SOTP_OTP_STATUS_1);
   volatile uint32_t *pSotpOtpWdata0Reg    = (uint32_t *)(SOTP_BASE + SOTP_OTP_WDATA_0);

   int i;
   uint32_t authVal[4] = {0xf,0x4,0x8,0xd};

   /* Poll for FDONE bit before doing any access */
   if ((rval = sotpPollForFDone()) == SOTP_E_ROW_TIMEOUT)
      return rval;

   /* turn on the cpu mode */
   *pSotpOtpProgCtrlReg |= SOTP_OTP_PROG_CTRL_OTP_CPU_MODE_EN;

   /* set up the OTP_CMD for OTP_ProgEnable */
   *pSotpOtpCtrl0Reg = SOTP_OTP_CTRL_0_OTP_CMD_OTP_PROG_ENABLE;

   /* Write F, 4, 8, D in sequence to BITSEL */
   for (i=0;i<4;i++)
   {
      *pSotpOtpWdata0Reg = authVal[i];

      /* Start the state machine */
      if ((rval = sotpSetStartAndPollForCmdDone()) == SOTP_S_ROW_SUCCESS)
      {
         /* Clear the CMD_DONE bit */
         *pSotpOtpStatus1Reg = SOTP_OTP_STATUS_1_CMD_DONE;
      }
      else
         return rval;

   }

   /* By default, SOTP block state is "unprogrammed" which prevents writing to      */
   /* rows 16-111. Set the state to ManuProg and SwNonAbDevice to allow programming */
   *pSotpChipCtrlReg |= (SOTP_CHIP_CTRL_SW_MANU_PROG | SOTP_CHIP_CTRL_SW_NON_AB_DEVICE | \
			SOTP_CHIP_CNTRL_SW_OVERRIDE_CHIP_STATES);

   /* Poll for FDONE bit before doing any further access */
   rval = sotpPollForFDone();

   /* return if prep is ok or not */
   return rval;
}



/*  *********************************************************************
    *  static SotpRowStatus sotpFinishFusing(void)
    *  
    *  Input parameters: 
    *      none  	   
    *
    *  Return value:
    *  	   SOTP_S_ROW_SUCCESS - The function has completed successfully
    *  	   SOTP_E_ROW_TIMEOUT - The function experienced unexpected FSM timeout 
    *  	                        from the SOTP block 
    *
    ********************************************************************* */
static SotpRowStatus sotpFinishFusing(void)
{
   SotpRowStatus rval = SOTP_S_ROW_SUCCESS;

   volatile uint32_t *pSotpOtpProgCtrlReg  = (uint32_t *)(SOTP_BASE + SOTP_OTP_PROG_CTRL);
   volatile uint32_t *pSotpOtpCtrl0Reg     = (uint32_t *)(SOTP_BASE + SOTP_OTP_CTRL_0);
   volatile uint32_t *pSotpOtpStatus1Reg   = (uint32_t *)(SOTP_BASE + SOTP_OTP_STATUS_1);

   /* set up the OTP_CMD for PROG  */
   *pSotpOtpCtrl0Reg = SOTP_OTP_CTRL_0_OTP_CMD_PROG;

   /* Start the state machine */
   if ((rval = sotpSetStartAndPollForCmdDone()) == SOTP_S_ROW_SUCCESS)
   {
      /* Clear the CMD_DONE bit */
      *pSotpOtpStatus1Reg = SOTP_OTP_STATUS_1_CMD_DONE;

      /* turn off cpu mode */
      *pSotpOtpProgCtrlReg &= ~SOTP_OTP_PROG_CTRL_OTP_CPU_MODE_EN;
   }

   return rval;
}



/*  *********************************************************************
    *  static SotpRowStatus sotpReadNonEccRow(uint32_t row, uint64_t *pData)
    *  
    *  Input parameters: 
    *  	   row   - row in question
    *  	   pDst  - pointer to where the data will be returned to
    *  	   
    *  Return value:
    *  	   SOTP_S_ROW_SUCCESS     - The function has completed successfully
    *  	   SOTP_E_ROW_TIMEOUT     - The function experienced unexpected FSM 
    *  	                            timeout from the SOTP block (pDst is invalid)
    *
    ********************************************************************* */
static SotpRowStatus sotpReadNonEccRow(uint32_t row, uint64_t *pDst)
{
   SotpRowStatus rval = SOTP_S_ROW_SUCCESS;

   volatile uint32_t *pSotpOtpProgCtrlReg  = (uint32_t *)(SOTP_BASE + SOTP_OTP_PROG_CTRL);
   volatile uint32_t *pSotpOtpCtrl0Reg     = (uint32_t *)(SOTP_BASE + SOTP_OTP_CTRL_0);
   volatile uint32_t *pSotpOtpStatus1Reg   = (uint32_t *)(SOTP_BASE + SOTP_OTP_STATUS_1);
   volatile uint32_t *pSotpOtpAddrReg      = (uint32_t *)(SOTP_BASE + SOTP_OTP_ADDR);
   volatile uint32_t *pSotpOtpRData0Reg    = (uint32_t *)(SOTP_BASE + SOTP_OTP_RDATA_0);
   volatile uint32_t *pSotpOtpRData1Reg    = (uint32_t *)(SOTP_BASE + SOTP_OTP_RDATA_1);

   /* Poll for FDONE bit before doing any access */
   if ((rval = sotpPollForFDone()) == SOTP_E_ROW_TIMEOUT)
      return rval;

   /* Clear ECC detect, ecc correct as well as command done bits */
   *pSotpOtpStatus1Reg = (SOTP_OTP_STATUS_1_CMD_DONE | SOTP_OTP_STATUS_1_ECC_COR | SOTP_OTP_STATUS_1_ECC_DET);

   /* zero out the regs_ecc_en bits, then turn on cpu mode, and set regs_ecc_en to disabled */
   *pSotpOtpProgCtrlReg &= ~( SOTP_OTP_PROG_CTRL_REGS_ECC_EN_MASK );
   *pSotpOtpProgCtrlReg |=  ( SOTP_OTP_PROG_CTRL_OTP_CPU_MODE_EN | SOTP_OTP_PROG_CTRL_OTP_DISABLE_ECC | \
                              SOTP_OTP_PROG_CTRL_REGS_ECC_DIS );

   /* Set OTP addr with the row to be read */
   *pSotpOtpAddrReg = row << SOTP_OTP_ADDR_OTP_ADDR_SHIFT;

   /* set up the OTP_CMD for OTP_READ */
   *pSotpOtpCtrl0Reg = SOTP_OTP_CTRL_0_OTP_CMD_OTP_READ;

   /* Start the state machine */
   if ((rval = sotpSetStartAndPollForCmdDone()) == SOTP_S_ROW_SUCCESS)
   {
      /* Data in row is valid. Pass it to return buffer */
      *pDst = (((uint64_t)(*pSotpOtpRData1Reg) << 32) | (*pSotpOtpRData0Reg));

      /* Clear the CMD_DONE bit */
      *pSotpOtpStatus1Reg = SOTP_OTP_STATUS_1_CMD_DONE;

      /* turn off cpu mode */
      *pSotpOtpProgCtrlReg &= ~SOTP_OTP_PROG_CTRL_OTP_CPU_MODE_EN;
   }

   return rval;
}



/*  *********************************************************************
    *  static SotpRowStatus sotpFuseNonEccRow(uint32_t row, uint64_t src)
    *  
    *  Input parameters: 
    *  	   row   - row in question
    *  	   src   - data bits to fuse (only the bottom 41 bits are relevant)
    *  	   
    *  Return value:
    *  	   SOTP_S_ROW_SUCCESS     - The function has completed successfully
    *  	   SOTP_E_ROW_TIMEOUT     - The function experienced unexpected FSM 
    *  	                            timeout from the SOTP block 
    *
    ********************************************************************* */
static SotpRowStatus sotpFuseNonEccRow(uint32_t row, uint64_t src)
{
   SotpRowStatus rval = SOTP_S_ROW_SUCCESS;

   volatile uint32_t *pSotpOtpProgCtrlReg  = (uint32_t *)(SOTP_BASE + SOTP_OTP_PROG_CTRL);
   volatile uint32_t *pSotpOtpAddrReg      = (uint32_t *)(SOTP_BASE + SOTP_OTP_ADDR);
   volatile uint32_t *pSotpOtpWdata0Reg    = (uint32_t *)(SOTP_BASE + SOTP_OTP_WDATA_0);
   volatile uint32_t *pSotpOtpWdata1Reg    = (uint32_t *)(SOTP_BASE + SOTP_OTP_WDATA_1);

   /* Prepare the SOTP FSM for fusing a row of data */
   if ((rval = sotpPrepForFusing()) != SOTP_S_ROW_SUCCESS)
      return rval;

   /* Disable ECC to be generated and fused by hardware */
   *pSotpOtpProgCtrlReg &= ~(SOTP_OTP_PROG_CTRL_OTP_ECC_WREN);

   /* Set OTP addr with the row to be fused */
   *pSotpOtpAddrReg = row << SOTP_OTP_ADDR_OTP_ADDR_SHIFT;

   /* Set up the registers with the data to be written */
   *pSotpOtpWdata0Reg = (uint32_t)(src & 0xffffffff); /* lower 32 bit data word */
   *pSotpOtpWdata1Reg = (uint32_t)(src >> 32); /* upper 32 bit data word */

   /* ... and fuse */
   rval = sotpFinishFusing();

   return rval;
}
  


/*  *********************************************************************
    *  static SotpRowStatus sotpIsRegionFuseLocked(uint32_t row)
    *  
    *  Input parameters: 
    *  	   row   - rows 28 thru 111 reside in different regions that may be fuse-locked
    *  	   
    *  Return value:
    *  	   SOTP_S_ROW_SUCCESS     - The function has completed successfully
    *      SOTP_E_ROW_FUSE_LOCK   - The row in question is within a region that 
    *                               is permanently fuse-locked 
    *  	   SOTP_E_ROW_TIMEOUT     - The function experienced unexpected FSM 
    *  	                            timeout from the SOTP block 
    *
    ********************************************************************* */
static SotpRowStatus sotpIsRegionFuseLocked(uint32_t row)
{
   SotpRowStatus rval = SOTP_S_ROW_SUCCESS;
   uint64_t rowData;

   /* Determine the region the row in question is in */
   uint32_t region = row / SOTP_ROWS_IN_REGION;

   /* The locking of all the regions require two rows of OTP. Each row has 41 */
   /* bits. There are no ecc nor fail bits in these two rows. Hence, regions  */
   /* 0 thru 19 lock bits are in SOTP row 8. Region 20 has a lock bit in each */
   /* row. Regions 21 thru 27 are all in row 9. Retrieve row 8 if required. If*/
   /* If either one of the two approp. bits are set, the region is fuse locked*/
   if (region < SOTP_NUM_REG_IN_FUSELOCK_ROW)
   {
      if ((rval = sotpReadNonEccRow(SOTP_FIRST_FUSELOCK_ROW, &rowData)) != SOTP_S_ROW_SUCCESS)
         return rval;
      else
      {
         if (rowData & ( 0x3 << (region * 2) )) 
            return SOTP_E_ROW_FUSE_LOCK;
      }
   }

   if (region >= (SOTP_NUM_REG_IN_FUSELOCK_ROW - 1))
   {
      if ((rval = sotpReadNonEccRow((SOTP_FIRST_FUSELOCK_ROW + 1), &rowData)) != SOTP_S_ROW_SUCCESS)
         return rval;
      else
      {
         if ((region == (SOTP_NUM_REG_IN_FUSELOCK_ROW - 1)) && (rowData & 0x1))
            return SOTP_E_ROW_FUSE_LOCK;
         else 
	 { 
            if (rowData & ( 0x3 << (((region-SOTP_NUM_REG_IN_FUSELOCK_ROW)*2)+1) )) 
               return SOTP_E_ROW_FUSE_LOCK;
	 } 
      }
   }

   return rval;
}



/*  *********************************************************************
    *  static SotpRowStatus sotpFuseLockRegion(uint32_t region)
    *  
    *  Input parameters: 
    *  	   row   - rows 28 thru 111 reside in different regions that may be fuse-locked
    *  	   
    *  Return value:
    *  	   SOTP_S_ROW_SUCCESS     - The function has completed successfully
    *  	   SOTP_E_ROW_TIMEOUT     - The function experienced unexpected FSM 
    *  	                            timeout from the SOTP block 
    *
    ********************************************************************* */
static SotpRowStatus sotpFuseLockRegion(uint32_t region)
{
   SotpRowStatus rval = SOTP_S_ROW_SUCCESS;
   uint64_t bitsToFuse;

   /* The locking of all the regions require two rows of OTP. Each row has 41 */
   /* bits. There are no ecc nor fail bits in these two rows. Hence, regions  */
   /* 0 thru 19 lock bits are in SOTP row 8. Region 20 has a lock bit in each */
   /* row. Regions 21 thru 27 are all in row 9. Retrieve row 8 if required. If*/
   /* If either one of the two approp. bits are set, the region is fuse locked*/
   if (region < SOTP_NUM_REG_IN_FUSELOCK_ROW)
   {
      // Figure out the bits to fuse (truncate to 41 bits)
      bitsToFuse = (0x3 << (region * 2)) & 0x1ffffffffff;

      if ((rval = sotpFuseNonEccRow(SOTP_FIRST_FUSELOCK_ROW, bitsToFuse)) != SOTP_S_ROW_SUCCESS)
         return rval;
   }

   if (region >= (SOTP_NUM_REG_IN_FUSELOCK_ROW - 1))
   {
      // Figure out the bits to fuse (truncate to 41 bits)
      if (region == (SOTP_NUM_REG_IN_FUSELOCK_ROW - 1)) 
         bitsToFuse = 0x1;
      else
         bitsToFuse =  0x3 << (((region-SOTP_NUM_REG_IN_FUSELOCK_ROW)*2)+1);

      rval = sotpFuseNonEccRow((SOTP_FIRST_FUSELOCK_ROW + 1), bitsToFuse);
   }

   return rval;
}



/*  *********************************************************************
    *  static SotpRowStatus sotpWriteSecKeyRow(uint32_t row, uint32_t src)
    *  
    *  Input parameters: 
    *  	   row  - SOTP has 112 rows total. This func is allowed to write to
    *  	          rows 28 thru 111 (first row is row 0)
    *  	   src  - Each row has 41 bits. Rows 16 thru 111 have 2 fail bits,
    *  	          7 ecc bits, and 32 data bits. This input parameter is
    *  	          the 32 data bits to be fused. 
    *  	   
    *  Return value:
    *  	   SOTP_S_ROW_SUCCESS  - The function has completed successfully
    *  	   SOTP_E_ROW_FUSE_LOCK- The row is permanently locked from being changed
    *  	   SOTP_E_ROW_TIMEOUT  - The function experienced unexpected FSM timeout 
    *  	                         from the SOTP block 
    *
    ********************************************************************* */
static SotpRowStatus sotpWriteSecKeyRow(uint32_t row, uint32_t src)
{
   SotpRowStatus rval = SOTP_S_ROW_SUCCESS;

   volatile uint32_t *pSotpOtpProgCtrlReg  = (uint32_t *)(SOTP_BASE + SOTP_OTP_PROG_CTRL);
   volatile uint32_t *pSotpOtpAddrReg      = (uint32_t *)(SOTP_BASE + SOTP_OTP_ADDR);
   volatile uint32_t *pSotpOtpWdata0Reg    = (uint32_t *)(SOTP_BASE + SOTP_OTP_WDATA_0);
   volatile uint32_t *pSotpOtpWdata1Reg    = (uint32_t *)(SOTP_BASE + SOTP_OTP_WDATA_1);

   /* Check to see that the row in question is not fuse locked */
   if ((rval = sotpIsRegionFuseLocked(row)) == SOTP_E_ROW_FUSE_LOCK)
      return rval;

   /* Prepare the SOTP FSM for fusing a row of data */
   if ((rval = sotpPrepForFusing()) != SOTP_S_ROW_SUCCESS)
      return rval;

   /* Enable ECC to be generated and fused by hardware */
   *pSotpOtpProgCtrlReg |= SOTP_OTP_PROG_CTRL_OTP_ECC_WREN;

   /* Set OTP addr with the row to be fused */
   *pSotpOtpAddrReg = row << SOTP_OTP_ADDR_OTP_ADDR_SHIFT;

   /* Set up the registers with the data to be written */
   *pSotpOtpWdata0Reg = src; /* 32 bit data word */
   *pSotpOtpWdata1Reg = 0;   /* ensure the two fail bits are not fused */

   /* ... and fuse */
   rval = sotpFinishFusing();

   return rval;
}



/*  *********************************************************************
    *  static SotpRowStatus sotpFuseSecKeyRowFailBits(uint32_t row)
    *  
    *  Input parameters: 
    *  	   row   - This func is allowed to fuse fail bits in rows 28 thru 111
    *  	   
    *  Return value:
    *  	   SOTP_S_ROW_SUCCESS     - The function has completed successfully
    *  	   SOTP_E_ROW_TIMEOUT     - The function experienced unexpected FSM 
    *  	                            timeout from the SOTP block 
    *  	   SOTP_E_ROW_ERROR       - The function experienced unexpected register 
    *  	                            ret value from SOTP block 
    *
    ********************************************************************* */
static SotpRowStatus sotpFuseSecKeyRowFailBits(uint32_t row)
{
   SotpRowStatus rval = SOTP_S_ROW_SUCCESS;

   volatile uint32_t *pSotpOtpProgCtrlReg  = (uint32_t *)(SOTP_BASE + SOTP_OTP_PROG_CTRL);
   volatile uint32_t *pSotpOtpAddrReg      = (uint32_t *)(SOTP_BASE + SOTP_OTP_ADDR);
   volatile uint32_t *pSotpOtpWdata0Reg    = (uint32_t *)(SOTP_BASE + SOTP_OTP_WDATA_0);
   volatile uint32_t *pSotpOtpWdata1Reg    = (uint32_t *)(SOTP_BASE + SOTP_OTP_WDATA_1);

   /* Prepare the SOTP FSM for fusing a row of data */
   if ((rval = sotpPrepForFusing()) != SOTP_S_ROW_SUCCESS)
      return rval;

   /* Enable ECC to be generated and fused by hardware */
   *pSotpOtpProgCtrlReg |= SOTP_OTP_PROG_CTRL_OTP_ECC_WREN;

   /* Set OTP addr with the row to be fused */
   *pSotpOtpAddrReg = row << SOTP_OTP_ADDR_OTP_ADDR_SHIFT;

   /* Set up the registers with the data to be written */
   *pSotpOtpWdata0Reg = 0;                          /* 32 bit data word  ... no change    */
   *pSotpOtpWdata1Reg = SOTP_OTP_WDATA_1_FAIL_MASK; /* ensure the two fail bits are fused */

   /* ... and fuse */
   rval = sotpFinishFusing();

   return rval;
}



/*  *********************************************************************
    *  static SotpRowStatus sotpRowReadLockStatus(uint32_t row)
    *  
    *  Input parameters: 
    *  	   row   - rows 28 thru 111
    *  	   
    *  Return value:
    *  	   SOTP_S_ROW_SUCCESS     - The function has completed successfully
    *      SOTP_E_ROW_READ_LOCK   - The row in question is within a region that 
    *                               is read-locked until the next POR
    *
    ********************************************************************* */
static SotpRowStatus sotpRowReadLockStatus(uint32_t row)
{
   SotpRowStatus rval = SOTP_S_ROW_SUCCESS;
   volatile uint32_t *pSotpOtpRegionRdLockReg  = (uint32_t *)(SOTP_BASE + SOTP_OTP_REGION_RD_LOCK);

   /* Determine the region the row in question is in */
   uint32_t region = row / SOTP_ROWS_IN_REGION;

   /* Check to see if that region is locked */
   if (*pSotpOtpRegionRdLockReg & ( 0x1 << region )) 
      rval = SOTP_E_ROW_READ_LOCK;

   return rval;
}


/*  *********************************************************************
    *  static SotpRowStatus sotpReadSecKeyRow(uint32_t row, uint32_t *pDst)
    *  
    *  Input parameters: 
    *  	   row   - This func is allowed to read from rows 28 thru 111
    *  	   pDst  - A pointer to the location that the retrieved 32 data
    *  	           bits will be written to.
    *  	   
    *  Return value:
    *  	   SOTP_S_ROW_SUCCESS     - The function has completed successfully
    *  	   SOTP_S_ROW_ECC_COR     - The function ecc corrected 1 bad bit when 
    *  	                            returning row data (pDst points to valid data)
    *  	   SOTP_E_ROW_ECC_DET     - The function detected 2 or more uncorrectable 
    *  	                            bits within the row (pDst is invalid)
    *  	   SOTP_E_ROW_FAIL_SET    - The function detected that 1 or both of 
    *  	                            the fail bits are set (pDst is invalid)
    *      SOTP_E_ROW_READ_LOCK   - The row in question is within a region that 
    *                               is read-locked until the next POR
    *  	   SOTP_E_ROW_TIMEOUT     - The function experienced unexpected FSM 
    *  	                            timeout from the SOTP block (pDst is invalid)
    *  	   SOTP_E_ROW_ERROR       - The function experienced unexpected register 
    *  	                            ret value from SOTP block (pDst is invalid)
    *
    ********************************************************************* */
static SotpRowStatus sotpReadSecKeyRow(uint32_t row, uint32_t *pDst)
{
   SotpRowStatus rval;

   volatile uint32_t *pSotpOtpProgCtrlReg  = (uint32_t *)(SOTP_BASE + SOTP_OTP_PROG_CTRL);
   volatile uint32_t *pSotpOtpCtrl0Reg     = (uint32_t *)(SOTP_BASE + SOTP_OTP_CTRL_0);
   volatile uint32_t *pSotpOtpStatus1Reg   = (uint32_t *)(SOTP_BASE + SOTP_OTP_STATUS_1);
   volatile uint32_t *pSotpOtpAddrReg      = (uint32_t *)(SOTP_BASE + SOTP_OTP_ADDR);
   volatile uint32_t *pSotpOtpRData0Reg    = (uint32_t *)(SOTP_BASE + SOTP_OTP_RDATA_0);
   volatile uint32_t *pSotpOtpRData1Reg    = (uint32_t *)(SOTP_BASE + SOTP_OTP_RDATA_1);

   /* Check to see that the row in question is not readlocked until the next POR */
   if ((rval = sotpRowReadLockStatus(row)) == SOTP_E_ROW_READ_LOCK)
      return rval;

   /* Poll for FDONE bit before doing any access */
   if ((rval = sotpPollForFDone()) == SOTP_E_ROW_TIMEOUT)
      return rval;

   /* Clear ECC detect, ecc correct as well as command done bits */
   *pSotpOtpStatus1Reg = (SOTP_OTP_STATUS_1_CMD_DONE | SOTP_OTP_STATUS_1_ECC_COR | SOTP_OTP_STATUS_1_ECC_DET);

   /* enable ecc checks (low), zero out the regs_ecc_en bits  ... then turn on cpu mode, and set regs_ecc_en to enabled */
   /* expect Bug?  When ecc=0 then programs 0x0, Reg says that it should be set to 4'b0101 : ECC check DISABLED */
   *pSotpOtpProgCtrlReg &= ~( SOTP_OTP_PROG_CTRL_OTP_DISABLE_ECC | SOTP_OTP_PROG_CTRL_REGS_ECC_EN_MASK );
   *pSotpOtpProgCtrlReg |=  ( SOTP_OTP_PROG_CTRL_OTP_CPU_MODE_EN | SOTP_OTP_PROG_CTRL_REGS_ECC_EN );

   /* Set OTP addr with the row to be read */
   *pSotpOtpAddrReg = row << SOTP_OTP_ADDR_OTP_ADDR_SHIFT;

   /* set up the OTP_CMD for OTP_READ */
   *pSotpOtpCtrl0Reg = SOTP_OTP_CTRL_0_OTP_CMD_OTP_READ;

   /* Start the state machine */
   if ((rval = sotpSetStartAndPollForCmdDone()) == SOTP_S_ROW_SUCCESS)
   {
      /* If the row is marked as failed, abort */
      if ( *pSotpOtpRData1Reg & SOTP_OTP_RDATA_1_FAIL_MASK )
      {
         info("\nSOTP: Row is marked as failed. \n");
         return ( SOTP_E_ROW_FAIL_SET );
      }
 
      /* See if the data is bad and ecc correction was not possible */
      if ( *pSotpOtpStatus1Reg & SOTP_OTP_STATUS_1_ECC_DET )
      {
         info("\nSOTP: Row data failed ecc with 2 or more bits . \n");
	 return ( SOTP_E_ROW_ECC_DET );
      }

      /* See if the data is good but an ecc correction took place */
      if ( *pSotpOtpStatus1Reg & SOTP_OTP_STATUS_1_ECC_COR )
      {
         info("\nSOTP: Row data valid, but ecc corrected 1 bit. \n");
	 rval = SOTP_S_ROW_ECC_COR;
      }

      /* Data in row is valid. Pass it to return buffer */
      *pDst = *pSotpOtpRData0Reg;

      /* Clear the CMD_DONE bit */
      *pSotpOtpStatus1Reg = SOTP_OTP_STATUS_1_CMD_DONE;

      /* turn off cpu mode */
      *pSotpOtpProgCtrlReg &= ~SOTP_OTP_PROG_CTRL_OTP_CPU_MODE_EN;
   }

   return rval;
}



/*  *********************************************************************
    *  extern SotpKeyStatus sotpReadSecKey(uint32_t section, uint32_t *pDst, uint32_t len)
    *  
    *  Description - Used by the bootrom or bootloader to read a key from SOTP
    *
    *  Input parameters: 
    *  	   section - Section that the key info should be read from
    *  	             (7 =< section =< 13)
    *      pDst    - pointer to the buffer containing the key (chopped 
    *                up into 32 bit words)
    *      len     - The number of 32 bit words in the key to read in (len =< 8)
    *  	   
    *  Return value:
    *  	   SOTP_S_KEY_SUCCESS - The function has completed successfully, and
    *  	                        pDst holds valid data
    *  	   SOTP_S_KEY_EMPTY   - The function has completed successfully, but 
    *  	                        no credential is currently stored in this section
    *      SOTP_E_KEY_CRC_MIS - The function detected a CRC mismatch during the key read 
    *  	   SOTP_E_KEY_UNDERRUN- The function ran out of rows within a section
    *  	                        before reading the entire key and crc
    *  	   SOTP_E_KEY_ERROR   - The function experienced an error in one of the
    *  	                        rows of the section
    *
    ********************************************************************* */
SotpKeyStatus sotpReadSecKey(uint32_t section, uint32_t *pDst, uint32_t len)
{
   uint32_t startRow, row, printRow, readCrc;
   uint32_t calcCrc = CRC32_INIT_VALUE;
   uint32_t i = 0;
   SotpKeyStatus rval    = SOTP_E_KEY_BADPARAM;
   SotpRowStatus rowRval = SOTP_S_ROW_SUCCESS;

   /* perform checks of input params */
   if ((section < SOTP_MIN_KEYSLOT) || (section > SOTP_MAX_KEYSLOT) || 
       (pDst == NULL) || (len == 0) || (len > SOTP_MAX_KEYLEN))
      return rval;

   /* Determine the starting row */
   startRow = ((section - SOTP_MIN_KEYSLOT) * SOTP_ROWS_IN_KEYSLOT) + SOTP_FIRST_KEYSLOT_ROW;

   /* Start reading the key and CRC */
   for ( row = startRow ; row < (startRow + SOTP_ROWS_IN_KEYSLOT); row++)
   {
      /* Determine the current row in a printable format for if something fails */
      printRow = row - startRow;
      if (printRow >= 0xa)
         printRow += (0x100 - 0xa);

      if (i < len)
         rowRval = sotpReadSecKeyRow(row, &pDst[i]); /* currently reading the key */

      if (i == len)
         rowRval = sotpReadSecKeyRow(row, &readCrc); /* currently reading the crc */

      switch(rowRval)
      {
         /* If success, just keep going */
         case SOTP_S_ROW_SUCCESS: 
	 case SOTP_S_ROW_ECC_COR:
            i++;
            if (i > len)
            {
               /* done reading sotp. Perform crc validation */
               calcCrc = getCrc32((unsigned char *)pDst, (len*4), calcCrc);
               if (readCrc == calcCrc)
                  return SOTP_S_KEY_SUCCESS; /* Done. pDst buffer holds the key and is good */
               else
               {
                  /* Either the section is empty, or a valid crc mismatch condition exists */
                  for (i = 0; i < len; i++)
                  {
                     if (pDst[i] != 0)
                     {
                        info("\nSOTP: Calculated crc did not match crc stored in row %d. Returned key data is invalid\n", row);
                        print(0x4352434d); /* CRCM for CRC Mismatch */
                        return SOTP_E_KEY_CRC_MIS;
                     }
                  }
                  return SOTP_S_KEY_EMPTY;
               }
            }      
            break;

	 /* If fail bit(s) set, row has been marked bad during fusing ... ignore this row */
         case SOTP_E_ROW_FAIL_SET:
            break;

         /* Read had 2 or more uncorrectable bits, AND the fail bits were NOT set */
	 /* Row has failed over time in the field */
         case SOTP_E_ROW_ECC_DET:
            info("\nSOTP: 2 or more Uncorrectable bits within row %d. Returned key data is invalid \n", row);
            print(0x45433030+printRow); /* ECxx for ECc failure on row xx of this section */
            return SOTP_E_KEY_ERROR;
            break;

         case SOTP_E_ROW_READ_LOCK:
            info("\nSOTP: Row %d is locked from reading. Returned key data is invalid\n", row);
            print(0x4c4b3030+printRow); /* LKxx for row xx of this section is LocKed from a read */
            return SOTP_E_KEY_ERROR;
            break;

         case SOTP_E_ROW_TIMEOUT:
            info("\nSOTP: Unexpected SOTP block timeout while reading row %d. Returned key data is invalid\n", row);
            print(0x544f3030+printRow); /* TOxx for row xx of this section had an unexpected TimeOut during state machine traversal */
            return SOTP_E_KEY_ERROR;
            break;

         case SOTP_E_ROW_ERROR:
         default:
            info("\nSOTP: Unknown condition in reading row %d. Returned key data is invalid\n", row);
            print(0x45523030+printRow); /* ERxx for row xx had an unexpected unknown ERror while reading it */
            return SOTP_E_KEY_ERROR;
            break;
      }
   }

   /* If we make it here, we ran out of rows in the key slot ... returned data is not valid */
   info("\nSOTP: Ran out of rows in section %d before reading entire key and crc. Returned key data is invalid\n", section);
   print(0x5544524e); /* UDRN for read ran ot of valid rows within this section before reading entire key and crc */
   return SOTP_E_KEY_UNDERRUN;
}



/*  *********************************************************************
    *  static SotpKeyStatus sotpLockSecKeyFuse(uint32_t section)
    *  
    *  Input parameters: 
    *  	   section - There are 7 locations called sections that can hold 
    *  	             up to 256 bits of key information. (7 =< section =< 13).
    *  	             This is the location that is to be locked down from
    *  	             future fusing. This is a permanent lockout even after POR.
    *  	   
    *  Return value:
    *  	   SOTP_S_KEY_SUCCESS     - The function has completed successfully
    *  	   SOTP_E_KEY_BADPARAM    - The function recieved a bad input parameter
    *  	   SOTP_E_KEY_ERROR       - The function experienced unexpected register 
    *  	                            ret value from SOTP block 
    *
    ********************************************************************* */
static SotpKeyStatus sotpLockSecKeyFuse(uint32_t section)
{
   SotpKeyStatus rval = SOTP_S_KEY_SUCCESS;
   uint32_t region, startRegion;

   /* perform checks of input params */
   if ((section < SOTP_MIN_KEYSLOT) || (section > SOTP_MAX_KEYSLOT))
      return SOTP_E_KEY_BADPARAM;

   /* Figure out the start region, and then for each of the three regions */
   /* in the section, do the for loop */
   startRegion = SOTP_FIRST_KEYSLOT_REGION + ((section - SOTP_MIN_KEYSLOT) * SOTP_REGIONS_IN_KEYSLOT);
   for (region = startRegion; region < (startRegion+3); region++)
   {
      if (sotpFuseLockRegion(region) != SOTP_S_ROW_SUCCESS)
         return SOTP_E_KEY_ERROR;
   }
   
   return rval;
}



/*  *********************************************************************
    *  extern SotpKeyStatus sotpLockSecKeyRead(uint32_t section)
    *  
    *  Description - After the bootrom or bootloader has read a key from
    *                SOTP, this function can be called so that no 
    *                other sw can read this key at a later time.
    *
    *  Input parameters: 
    *  	   section - There are 7 locations called sections that can hold 
    *  	             up to 256 bits of key information. (7 =< section =< 13)
    *  	             This is the location that is to be locked down from
    *  	             reading until the next power-on-reset.
    *  	   
    *  Return value:
    *  	   SOTP_S_KEY_SUCCESS     - The function has completed successfully
    *  	   SOTP_E_KEY_BADPARAM    - The function recieved a bad input parameter
    *
    ********************************************************************* */
SotpKeyStatus sotpLockSecKeyRead(uint32_t section)
{
   uint32_t startRegion;
   volatile uint32_t *pSotpOtpRegionRdLockReg  = (uint32_t *)(SOTP_BASE + SOTP_OTP_REGION_RD_LOCK);
   volatile uint32_t *pSotpOtpProgCtrlReg      = (uint32_t *)(SOTP_BASE + SOTP_OTP_PROG_CTRL);

   /* perform checks of input params */
   if ((section < SOTP_MIN_KEYSLOT) || (section > SOTP_MAX_KEYSLOT))
      return SOTP_E_KEY_BADPARAM;

   /* enable ecc checks (low), zero out the regs_ecc_en bits  ... then turn on cpu mode, and set regs_ecc_en to enabled */
   /* expect Bug?  When ecc=0 then programs 0x0, Reg says that it should be set to 4'b0101 : ECC check DISABLED */
   *pSotpOtpProgCtrlReg &= ~SOTP_OTP_PROG_CTRL_REGS_ECC_EN_MASK;
   *pSotpOtpProgCtrlReg |=  ( SOTP_OTP_PROG_CTRL_OTP_CPU_MODE_EN | SOTP_OTP_PROG_CTRL_REGS_ECC_DIS );

   /* Determine the starting region to lock */
   startRegion = SOTP_FIRST_KEYSLOT_REGION + ((section - SOTP_MIN_KEYSLOT) * SOTP_REGIONS_IN_KEYSLOT);

   /* wait a little */
   cfe_usleep(100);

   /* The read lock register is a write-once register on a bit-for-bit bassis     */
   /* so you don't have to worry about preserving bits that are already set to 1. */
   *pSotpOtpRegionRdLockReg = SOTP_REGIONS_MASK_IN_KEYSLOT << startRegion;

   /* wait a little */
   cfe_usleep(100);

   /* turn off cpu mode */
   *pSotpOtpProgCtrlReg &= ~SOTP_OTP_PROG_CTRL_OTP_CPU_MODE_EN;

   return SOTP_S_KEY_SUCCESS;
}



/*  *********************************************************************
    *  extern SotpKeyStatus sotpWriteSecKey(uint32_t section, uint32_t *pSrc, uint32_t len)
    *  
    *  Description - Used during manufacturing to fuse a security credential
    *                into SOTP
    *
    *  Input parameters: 
    *  	   section - There are 7 locations called sections that can hold 
    *  	             up to 256 bits of key information. (7 =< section =< 13)
    *      pSrc    - pointer to the buffer containing the key (chopped 
    *                up into 32 bit words)
    *      len     - The number of 32 bit words in the key (len =< 8)
    *  	   
    *  Return value:
    *  	   SOTP_S_KEY_SUCCESS - The function has completed successfully
    *  	   SOTP_E_KEY_BADPARAM- The function recieved a bad input parameter
    *  	   SOTP_E_KEY_OVERRUN - The function ran out of rows within a section
    *  	                        before the entire key and crc was fused
    *  	   SOTP_E_KEY_TIMEOUT - The function experienced unexpected FSM 
    *  	                        timeout from the SOTP block 
    *  	   SOTP_E_KEY_ERROR   - The function experienced unexpected register 
    *  	                        ret value from SOTP block 
    *
    ********************************************************************* */
SotpKeyStatus sotpWriteSecKey(uint32_t section, uint32_t *pSrc, uint32_t len)
{
   uint32_t startRow, row, rowData, calcCrc;
   uint32_t i = 0;
   SotpKeyStatus rval    = SOTP_E_KEY_BADPARAM;
   SotpRowStatus rowRval = SOTP_S_ROW_SUCCESS;

   /* perform checks of input params */
   if ((section < SOTP_MIN_KEYSLOT) || (section > SOTP_MAX_KEYSLOT) || 
       (pSrc == NULL) || (len == 0) || (len > SOTP_MAX_KEYLEN))
      return rval;

   /* Calculate the crc that is to be stored into the key slot */
   calcCrc = getCrc32((unsigned char *)pSrc, (len*4), CRC32_INIT_VALUE);

   /* Determine the starting row to fuse */
   startRow = ((section - SOTP_MIN_KEYSLOT) * SOTP_ROWS_IN_KEYSLOT) + SOTP_FIRST_KEYSLOT_ROW;

   /* Start fusing the key and CRC */
   for ( row = startRow ; row < (startRow + SOTP_ROWS_IN_KEYSLOT); row++)
   {
      if (i < len)
         rowRval = sotpWriteSecKeyRow(row, pSrc[i]); /* currently writing the key */

      if (i == len)
         rowRval = sotpWriteSecKeyRow(row, calcCrc); /* write the crc */

      /* See if it was successful */
      switch(rowRval)
      {

         case SOTP_S_ROW_SUCCESS:
         {
            /* Check the integrity of the row just fused */
            rowRval = sotpReadSecKeyRow(row, &rowData);

            switch(rowRval)
            {
               case SOTP_S_ROW_SUCCESS: 
	       case SOTP_S_ROW_ECC_COR:
               {
                  if (i < len)
                  {
                     if (pSrc[i] == rowData)
                        i++;
                     else
                     {
                        info("\nSOTP: Written and read data didn't match within row %d. Section %d is unusable\n", row, section);
                        return SOTP_E_KEY_ERROR;
                     }
                  }
                  else
                  {
                     if (calcCrc == rowData)
                        return(sotpLockSecKeyFuse(section)); /* key and crc fused. Lock section from further writes */
                     else
                     {
                        info("\nSOTP: Calculated and read crc didn't match within row %d. Section %d is unusable\n", row, section);
                        return SOTP_E_KEY_ERROR;
                     }
                  }

                  break;
               }

               case SOTP_E_ROW_ECC_DET:
               {
                  /* row has 2 or more uncorrectable bits. Mark the row as failed */
                  if ((rowRval = sotpFuseSecKeyRowFailBits(row)) != SOTP_S_ROW_SUCCESS)
                  {
                     info("\nSOTP: Failed to fuse fail bits within row %d. Section %d is unusable\n", row, section);
                     return SOTP_E_KEY_ERROR;
                  }
                  break;
               }

	       case SOTP_E_ROW_FAIL_SET:
               {
                  info("\nSOTP: Fail bits have previously been fused within row %d. Section %d is unusable\n", row, section);
                  return SOTP_E_KEY_ERROR;
                  break;
               }

	       case SOTP_E_ROW_READ_LOCK:
               {
                  info("\nSOTP: Row %d is locked from reading. Cannot verify. Section %d may be unusable\n", row, section);
                  return SOTP_E_KEY_ERROR;
                  break;
               }

               default:
               {
                  info("\nSOTP: Unknown condition in reading row %d. Section %d may be unusable\n", row, section);
                  return SOTP_E_KEY_ERROR;
                  break;
               }
            }

            break;
         }

         case SOTP_E_ROW_FUSE_LOCK:
         {
            info("\nSOTP: Row %d is locked from fusing. Section %d may be unusable\n", row, section);
            return SOTP_E_KEY_ERROR;
            break;
         }

         case SOTP_E_ROW_TIMEOUT:
         {
            info("\nSOTP: Unexpected SOTP block timeout while fusing row %d. Section %d may be unusable\n", row, section);
            return SOTP_E_KEY_ERROR;
            break;
         }

         case SOTP_E_ROW_ERROR:
         default:
         {
            info("\nSOTP: Unknown condition while fusing row %d. Section %d may be unusable\n", row, section);
            return SOTP_E_KEY_ERROR;
            break;
         }

      }
   }

   info("\nSOTP: Ran out of rows before fusing entire key and crc. Section %d is unusable \n", section);
   return SOTP_E_KEY_OVERRUN;
}



/*  *********************************************************************
    *  extern SotpMode sotpSetMode(void)
    *  
    *  Description - After the manufacuring personalization is complete,
    *                this routine is called. Upon the next POR, the bootrom
    *                will know that the SOTP block is in the field mode
    *                rather than manufacturing mode.
    *
    *  Input parameters: 
    *  	   SOTP_S_MODE_FIELD  - The SOTP block has been personalized and
    *  	                        is ready for deployment
    *  	   
    *  Return value:
    *  	   SOTP_S_MODE_FIELD  - The function has completed successfully
    *  	   SOTP_E_MODE_TIMEOUT- The function experienced unexpected FSM 
    *  	                        timeout from the SOTP block 
    *  	   SOTP_E_MODE_ERROR  - The function experienced unexpected register 
    *  	                        ret value from SOTP block 
    *
    ********************************************************************* */
SotpMode sotpSetMode(SotpMode mode)
{
   SotpMode rval = SOTP_E_MODE_ERROR;

   return rval;
}



/*  *********************************************************************
    *  extern SotpMode sotpGetMode(void)
    *  
    *  Description - Returns what mode the SOTP block is in.
    *
    *  Input parameters: 
    *  	   none 
    *  	   
    *  Return value:
    *  	   SOTP_S_MODE_MFG    - The SOTP block has not been personalized and
    *  	                        is in the manufacturing mode
    *  	   SOTP_S_MODE_FIELD  - The SOTP block has been personalized and
    *  	                        is ready for deployment
    *  	   SOTP_E_MODE_TIMEOUT- The function experienced unexpected FSM 
    *  	                        timeout from the SOTP block 
    *  	   SOTP_E_MODE_ERROR  - The function experienced unexpected register 
    *  	                        ret value from SOTP block 
    *
    ********************************************************************* */
SotpMode sotpGetMode(void)
{
   SotpMode rval = SOTP_E_MODE_ERROR;

   return rval;
}
