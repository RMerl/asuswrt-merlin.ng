/*
<:copyright-BRCM:2018:DUAL/GPL:standard

   Copyright (c) 2018 Broadcom 
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
*/
/***************************************************************************
* File Name  : bcm_sotp_impl1.c
*
* Description: provides core API for sotp read/write
*
*
***************************************************************************/

#include <linux/version.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/module.h>
#include <linux/uaccess.h>
#include <linux/delay.h>
#include <linux/crc32.h>
#include <bcm_sotp.h>
#include "sotp_base_defs.h"

#define SOTP_DBG_API        0
#define SOTP_DBG_ENABLE     0

#if SOTP_DBG_ENABLE
#   define SOTP_DBG(fmt, args...) printk( KERN_DEBUG "SOTP_drv: " fmt, ## args)
#else
#   define SOTP_DBG(fmt, args...) /* not DBGging: nothing */
#endif /* SOTP_DBG_ENABLE */

#define SOTP_ERR_PRINT(fmt, args...) printk( KERN_ERR "SOTP_drv: " fmt, ## args)
#define SOTP_PRINT(fmt, args...) printk( fmt, ## args)

#define SOTP_BASE_CHK() {\
                            if(!sotp_base_ptr)\
                            {\
                                SOTP_ERR_PRINT("\nSOTP base address not specified!\n");\
                                return -1;\
                            }\
                        }

/***************************************************************************
* Prototypes / Globals
***************************************************************************/
static volatile SotpRegs * sotp_base_ptr = NULL;

/***************************************************************************
* SOTP functions
***************************************************************************/
#if SOTP_DBG_API
static int rdlock=0;
static uint32_t row_data=0;
static uint32_t key_data[SOTP_NUM_WORDS_IN_KEYSLOT] = {0};
static int sotpKeySlotReadLockStatus(int section_num)
{
   if( rdlock == 1)
        return SOTP_E_ROW_READ_LOCK;
   else 
        return 0;
}
static int sotpReadLockKeyslot(int section_num)
{
    rdlock = 1;
    return SOTP_S_KEY_SUCCESS;
}

static int sotpWriteKeyslotData(int section_num,  uint32_t* sotp_data, int num_words)
{
    memcpy((char*)&key_data[0], (char*)sotp_data, num_words*sizeof(uint32_t));
    return SOTP_S_KEY_SUCCESS;
}
static int sotpReadKeyslotData(int section_num,  uint32_t* sotp_data, int num_words)
{
    memcpy((char*)sotp_data, (char*)&key_data[0], num_words*sizeof(uint32_t));
    return SOTP_S_KEY_SUCCESS;
}

static int sotpWriteEccRow(int row_addr, uint32_t sotp_data, uint32_t raw_write )
{
    row_data = sotp_data;
    return SOTP_S_ROW_SUCCESS;
}
static int sotpReadEccRow(int row_addr, uint32_t* sotp_data, uint32_t raw_read )
{
    *sotp_data = row_data;
    return SOTP_S_ROW_SUCCESS;
}

#else
#define CRC32_INIT_VALUE   0xffffffff /* Initial CRC32 checksum value */

/***************************************************************************
// Function Name: getCrc32
// Description  : caculate the CRC 32 of the given data.
// Parameters   : pdata - array of data.
//                size - number of input data bytes.
//                crc - either CRC32_INIT_VALUE or previous return value.
// Returns      : crc.
****************************************************************************/
static uint32_t getCrc32(uint8_t *pdata, uint32_t size, uint32_t crc)
{
    crc = crc32(crc, (unsigned char const *)pdata, size);
    return crc;
}

/*  *********************************************************************
    *  static SotpRowStatus sotpPollForFDone(void)
    *  
    *  Input parameters: 
    *      none 
    *      
    *  Return value:
    *      SOTP_S_ROW_SUCCESS - The function has completed successfully
    *      SOTP_E_ROW_TIMEOUT - The function experienced unexpected FSM timeout 
    *                           from the SOTP block 
    *
    ********************************************************************* */
static SotpRowStatus sotpPollForFDone(void)
{
   SotpRowStatus rval = SOTP_S_ROW_SUCCESS;

   uint32_t reg_rd_data;
   uint32_t cntr = 0;

   reg_rd_data = sotp_base_ptr->sotp_sotp_out_0;
   while (( ! (reg_rd_data & SOTP_SOTP_OUT_0_SOTP_OTP_READY)) && (cntr < SOTP_MAX_CNTR))
   {
      cntr++;
      msleep(1);
      reg_rd_data = sotp_base_ptr->sotp_sotp_out_0;
   }

   if (cntr == SOTP_MAX_CNTR)
   {
      rval = SOTP_E_ROW_TIMEOUT;
      SOTP_ERR_PRINT("\nTimeout waiting for FDONE. \n");
   }

   return rval;
}

/*  *********************************************************************
    *  static SotpRowStatus sotpSetStartAndPollForCmdDone(void)
    *  
    *  Input parameters: 
    *      none 
    *      
    *  Return value:
    *      SOTP_S_ROW_SUCCESS - The function has completed successfully
    *      SOTP_E_ROW_TIMEOUT - The function experienced unexpected FSM timeout 
    *                           from the SOTP block 
    *
    ********************************************************************* */
static SotpRowStatus sotpSetStartAndPollForCmdDone(void)
{
   SotpRowStatus rval = SOTP_S_ROW_SUCCESS;

   uint32_t reg_rd_data;
   uint32_t cntr = 0;

   /* Set the start bit */
   sotp_base_ptr->sotp_otp_ctrl_0 |= SOTP_OTP_CTRL_0_START;

   msleep(1);

   /* Wait for the FSM to say it is done */
   reg_rd_data = sotp_base_ptr->sotp_otp_status_1;
   while (( ! (reg_rd_data & SOTP_OTP_STATUS_1_CMD_DONE)) && (cntr < SOTP_MAX_CNTR))
   {
      cntr++;
      msleep(1);
      reg_rd_data = sotp_base_ptr->sotp_otp_status_1;
   }

   if (cntr != SOTP_MAX_CNTR)
   {
      /* Clear the start bit */
      sotp_base_ptr->sotp_otp_ctrl_0 &= ~SOTP_OTP_CTRL_0_START;
   }
   else
   {
      rval = SOTP_E_ROW_TIMEOUT;
      SOTP_ERR_PRINT("\nTimeout waiting for CMD_DONE. \n");
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
    *      SOTP_S_ROW_SUCCESS - The function has completed successfully
    *      SOTP_E_ROW_TIMEOUT - The function experienced unexpected FSM timeout 
    *                           from the SOTP block 
    *
    ********************************************************************* */
static SotpRowStatus sotpPrepForFusing(void)
{
   SotpRowStatus rval = SOTP_S_ROW_SUCCESS;


   int i;
   uint32_t authVal[4] = {0xf,0x4,0x8,0xd};

   /* Poll for FDONE bit before doing any access */
   if ((rval = sotpPollForFDone()) == SOTP_E_ROW_TIMEOUT)
      return rval;

   /* turn on the cpu mode */
   sotp_base_ptr->sotp_otp_prog_ctrl |= SOTP_OTP_PROG_CTRL_OTP_CPU_MODE_EN;

   /* set up the OTP_CMD for OTP_ProgEnable */
   sotp_base_ptr->sotp_otp_ctrl_0 = SOTP_OTP_CTRL_0_OTP_CMD_OTP_PROG_ENABLE;

   /* Write F, 4, 8, D in sequence to BITSEL */
   for (i=0;i<4;i++)
   {
      sotp_base_ptr->sotp_otp_wdata_0 = authVal[i];

      /* Start the state machine */
      if ((rval = sotpSetStartAndPollForCmdDone()) == SOTP_S_ROW_SUCCESS)
      {
         /* Clear the CMD_DONE bit */
         sotp_base_ptr->sotp_otp_status_1 = SOTP_OTP_STATUS_1_CMD_DONE;
      }
      else
         return rval;

   }

   /* By default, SOTP block state is "unprogrammed" which prevents writing to      */
   /* rows 16-111. Set the state to ManuProg and SwNonAbDevice to allow programming */
   sotp_base_ptr->sotp_chip_cntrl |= (SOTP_CHIP_CNTRL_SW_MANU_PROG | SOTP_CHIP_CNTRL_SW_NON_AB_DEVICE | \
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
    *     SOTP_S_ROW_SUCCESS - The function has completed successfully
    *     SOTP_E_ROW_TIMEOUT - The function experienced unexpected FSM timeout 
    *                          from the SOTP block 
    *
    ********************************************************************* */
static SotpRowStatus sotpFinishFusing(void)
{
   SotpRowStatus rval = SOTP_S_ROW_SUCCESS;


   /* set up the OTP_CMD for PROG  */
   sotp_base_ptr->sotp_otp_ctrl_0 = SOTP_OTP_CTRL_0_OTP_CMD_PROG;

   /* Start the state machine */
   if ((rval = sotpSetStartAndPollForCmdDone()) == SOTP_S_ROW_SUCCESS)
   {
      /* Clear the CMD_DONE bit */
      sotp_base_ptr->sotp_otp_status_1 = SOTP_OTP_STATUS_1_CMD_DONE;

      /* turn off cpu mode */
      sotp_base_ptr->sotp_otp_prog_ctrl &= ~SOTP_OTP_PROG_CTRL_OTP_CPU_MODE_EN;
   }

   return rval;
}

/*  *********************************************************************
    *  static SotpRowStatus sotpReadNonEccRow(uint32_t row, uint64_t *pData)
    *  
    *  Input parameters: 
    *      row   - row in question
    *      pDst  - pointer to where the data will be returned to
    *      
    *  Return value:
    *      SOTP_S_ROW_SUCCESS     - The function has completed successfully
    *      SOTP_E_ROW_TIMEOUT     - The function experienced unexpected FSM 
    *                               timeout from the SOTP block (pDst is invalid)
    *
    ********************************************************************* */
static SotpRowStatus sotpReadNonEccRow(uint32_t row, uint64_t *pDst)
{
   SotpRowStatus rval = SOTP_S_ROW_SUCCESS;


   /* Poll for FDONE bit before doing any access */
   if ((rval = sotpPollForFDone()) == SOTP_E_ROW_TIMEOUT)
      return rval;

   /* Clear ECC detect, ecc correct as well as command done bits */
   sotp_base_ptr->sotp_otp_status_1 = (SOTP_OTP_STATUS_1_CMD_DONE | SOTP_OTP_STATUS_1_ECC_COR | SOTP_OTP_STATUS_1_ECC_DET);

   /* zero out the regs_ecc_en bits, then turn on cpu mode, and set regs_ecc_en to disabled */
   sotp_base_ptr->sotp_otp_prog_ctrl &= ~( SOTP_OTP_PROG_CTRL_REGS_ECC_EN_MASK );
   sotp_base_ptr->sotp_otp_prog_ctrl |=  ( SOTP_OTP_PROG_CTRL_OTP_CPU_MODE_EN | SOTP_OTP_PROG_CTRL_OTP_DISABLE_ECC | \
                              SOTP_OTP_PROG_CTRL_REGS_ECC_DIS );

   /* Set OTP addr with the row to be read */
   sotp_base_ptr->sotp_otp_addr = row << SOTP_OTP_ADDR_OTP_ADDR_SHIFT;

   /* set up the OTP_CMD for OTP_READ */
   sotp_base_ptr->sotp_otp_ctrl_0 = SOTP_OTP_CTRL_0_OTP_CMD_OTP_READ;

   /* Start the state machine */
   if ((rval = sotpSetStartAndPollForCmdDone()) == SOTP_S_ROW_SUCCESS)
   {
      /* Data in row is valid. Pass it to return buffer */
      *pDst = (((uint64_t)(sotp_base_ptr->sotp_otp_rdata_1) << 32) | (sotp_base_ptr->sotp_otp_rdata_0));

      /* Clear the CMD_DONE bit */
      sotp_base_ptr->sotp_otp_status_1 = SOTP_OTP_STATUS_1_CMD_DONE;

      /* turn off cpu mode */
      sotp_base_ptr->sotp_otp_prog_ctrl &= ~SOTP_OTP_PROG_CTRL_OTP_CPU_MODE_EN;
   }

   return rval;
}

/*  *********************************************************************
    *  static SotpRowStatus sotpFuseNonEccRow(uint32_t row, uint64_t src)
    *  
    *  Input parameters: 
    *      row   - row in question
    *      src   - data bits to fuse (only the bottom 41 bits are relevant)
    *     
    *  Return value:
    *     SOTP_S_ROW_SUCCESS     - The function has completed successfully
    *     SOTP_E_ROW_TIMEOUT     - The function experienced unexpected FSM 
    *                              timeout from the SOTP block 
    *
    ********************************************************************* */
static SotpRowStatus sotpFuseNonEccRow(uint32_t row, uint64_t src)
{
   SotpRowStatus rval = SOTP_S_ROW_SUCCESS;


   /* Prepare the SOTP FSM for fusing a row of data */
   if ((rval = sotpPrepForFusing()) != SOTP_S_ROW_SUCCESS)
      return rval;

   /* Disable ECC to be generated and fused by hardware */
   sotp_base_ptr->sotp_otp_prog_ctrl &= ~(SOTP_OTP_PROG_CTRL_OTP_ECC_WREN);

   /* Set OTP addr with the row to be fused */
   sotp_base_ptr->sotp_otp_addr = row << SOTP_OTP_ADDR_OTP_ADDR_SHIFT;

   /* Set up the registers with the data to be written */
   sotp_base_ptr->sotp_otp_wdata_0 = (uint32_t)(src & 0xffffffff); /* lower 32 bit data word */
   sotp_base_ptr->sotp_otp_wdata_1 = (uint32_t)(src >> 32); /* upper 32 bit data word */

   /* ... and fuse */
   rval = sotpFinishFusing();

   return rval;
}

/*  *********************************************************************
    *  static SotpRowStatus sotpIsRegionFuseLocked(uint32_t region)
    *  
    *  Input parameters: 
    *      row   - rows 28 thru 111 reside in different regions that may be fuse-locked
    *      
    *  Return value:
    *      SOTP_S_ROW_SUCCESS     - The function has completed successfully
    *      SOTP_E_ROW_FUSE_LOCK   - The row in question is within a region that 
    *                               is permanently fuse-locked 
    *      SOTP_E_ROW_TIMEOUT     - The function experienced unexpected FSM 
    *                               timeout from the SOTP block 
    *
    ********************************************************************* */
static SotpRowStatus sotpIsRegionFuseLocked(uint32_t region)
{
   SotpRowStatus rval = SOTP_S_ROW_SUCCESS;
   uint64_t rowData;

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
         if (rowData & ( (uint64_t)0x3 << (region * 2) )) 
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
            if (rowData & ( (uint64_t)0x3 << (((region-SOTP_NUM_REG_IN_FUSELOCK_ROW)*2)+1) )) 
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
    *     row   - rows 28 thru 111 reside in different regions that may be fuse-locked
    *     
    *  Return value:
    *     SOTP_S_ROW_SUCCESS     - The function has completed successfully
    *     SOTP_E_ROW_TIMEOUT     - The function experienced unexpected FSM 
    *                              timeout from the SOTP block 
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
      bitsToFuse = ((uint64_t)0x3 << (region * 2)) & 0x1ffffffffff;

      if ((rval = sotpFuseNonEccRow(SOTP_FIRST_FUSELOCK_ROW, bitsToFuse)) != SOTP_S_ROW_SUCCESS)
         return rval;
   }

   if (region >= (SOTP_NUM_REG_IN_FUSELOCK_ROW - 1))
   {
      // Figure out the bits to fuse (truncate to 41 bits)
      if (region == (SOTP_NUM_REG_IN_FUSELOCK_ROW - 1)) 
         bitsToFuse = 0x1;
      else
         bitsToFuse =  (uint64_t)0x3 << (((region-SOTP_NUM_REG_IN_FUSELOCK_ROW)*2)+1);

      rval = sotpFuseNonEccRow((SOTP_FIRST_FUSELOCK_ROW + 1), bitsToFuse);
   }

   return rval;
}

/*  *********************************************************************
    *  static SotpRowStatus sotpWriteEccRow(uint32_t row, uint32_t src)
    *  
    *  Input parameters: 
    *     row  - SOTP has 112 rows total. This func is allowed to write to
    *            rows 28 thru 111 (first row is row 0)
    *     src  - Each row has 41 bits. Rows 16 thru 111 have 2 fail bits,
    *            7 ecc bits, and 32 data bits. This input parameter is
    *            the 32 data bits to be fused. 
    *     raw_write - If set then ECC checks are skipped
    *     
    *  Return value:
    *     SOTP_S_ROW_SUCCESS  - The function has completed successfully
    *     SOTP_E_ROW_FUSE_LOCK- The row is permanently locked from being changed
    *     SOTP_E_ROW_TIMEOUT  - The function experienced unexpected FSM timeout 
    *                           from the SOTP block 
    *
    ********************************************************************* */
static SotpRowStatus sotpWriteEccRow(uint32_t row, uint32_t src, uint32_t raw_write)
{
   SotpRowStatus rval = SOTP_S_ROW_SUCCESS;

   SOTP_DBG("%s: row:%d data:0x%08x\n", __FUNCTION__, row, src);

   if( row < SOTP_FIRST_ECC_CONFIG_ROW || row > SOTP_NUM_ROWS-1 )
   {
      SOTP_ERR_PRINT("%s: ECC Row %d out of bound! Min:%d Max:%d\n", __FUNCTION__, row, SOTP_FIRST_ECC_CONFIG_ROW, SOTP_NUM_ROWS-1);
      return SOTP_E_ROW_ERROR;
   }

   /* Check to see that the row in question is not fuse locked */
   if ((rval = sotpIsRegionFuseLocked(row/SOTP_ROWS_IN_REGION)) == SOTP_E_ROW_FUSE_LOCK)
   {
      SOTP_ERR_PRINT("%s: Row %d is fuse locked. Cannot write row data.\n", __FUNCTION__, row);
      return rval;
   }

   /* Prepare the SOTP FSM for fusing a row of data */
   if ((rval = sotpPrepForFusing()) != SOTP_S_ROW_SUCCESS)
      return rval;

   if( raw_write )
   {
      /* Disable ECC computation and fusing by hardware */
      sotp_base_ptr->sotp_otp_prog_ctrl |= SOTP_OTP_PROG_CTRL_OTP_DISABLE_ECC;
      sotp_base_ptr->sotp_otp_prog_ctrl &= ~(SOTP_OTP_PROG_CTRL_OTP_ECC_WREN);
   }
   else
   {
      /* Enable ECC to be generated and fused by hardware */
      sotp_base_ptr->sotp_otp_prog_ctrl &= ~(SOTP_OTP_PROG_CTRL_OTP_DISABLE_ECC);
      sotp_base_ptr->sotp_otp_prog_ctrl |= SOTP_OTP_PROG_CTRL_OTP_ECC_WREN;
   }

   /* Set OTP addr with the row to be fused */
   sotp_base_ptr->sotp_otp_addr = row << SOTP_OTP_ADDR_OTP_ADDR_SHIFT;

   /* Set up the registers with the data to be written */
   sotp_base_ptr->sotp_otp_wdata_0 = src; /* 32 bit data word */
   sotp_base_ptr->sotp_otp_wdata_1 = 0;   /* ensure the two fail bits are not fused */

   /* ... and fuse */
   rval = sotpFinishFusing();

   return rval;
}

/*  *********************************************************************
    *  static SotpRowStatus sotpFuseSecKeyRowFailBits(uint32_t row)
    *  
    *  Input parameters: 
    *      row   - This func is allowed to fuse fail bits in rows 28 thru 111
    *      
    *  Return value:
    *      SOTP_S_ROW_SUCCESS     - The function has completed successfully
    *      SOTP_E_ROW_TIMEOUT     - The function experienced unexpected FSM 
    *                               timeout from the SOTP block 
    *      SOTP_E_ROW_ERROR       - The function experienced unexpected register 
    *                               ret value from SOTP block 
    *
    ********************************************************************* */
static SotpRowStatus sotpFuseSecKeyRowFailBits(uint32_t row)
{
   SotpRowStatus rval = SOTP_S_ROW_SUCCESS;


   /* Prepare the SOTP FSM for fusing a row of data */
   if ((rval = sotpPrepForFusing()) != SOTP_S_ROW_SUCCESS)
      return rval;

   /* Enable ECC to be generated and fused by hardware */
   sotp_base_ptr->sotp_otp_prog_ctrl |= SOTP_OTP_PROG_CTRL_OTP_ECC_WREN;

   /* Set OTP addr with the row to be fused */
   sotp_base_ptr->sotp_otp_addr = row << SOTP_OTP_ADDR_OTP_ADDR_SHIFT;

   /* Set up the registers with the data to be written */
   sotp_base_ptr->sotp_otp_wdata_0 = 0;                          /* 32 bit data word  ... no change    */
   sotp_base_ptr->sotp_otp_wdata_1 = SOTP_OTP_WDATA_1_FAIL_MASK; /* ensure the two fail bits are fused */

   /* ... and fuse */
   rval = sotpFinishFusing();

   return rval;
}

/*  *********************************************************************
    *  static SotpRowStatus sotpRegionReadLockStatus(uint32_t region)
    *  
    *  Input parameters: 
    *     row   - regions 0 - 28
    *     
    *  Return value:
    *     SOTP_S_ROW_SUCCESS     - The function has completed successfully
    *     SOTP_E_ROW_READ_LOCK   - The row in question is within a region that 
    *                               is read-locked until the next POR
    *
    ********************************************************************* */
static SotpRowStatus sotpRegionReadLockStatus(uint32_t region)
{
   SotpRowStatus rval = SOTP_S_ROW_SUCCESS;

   /* Check to see if that region is locked */
   if (sotp_base_ptr->sotp_otp_rd_lock & ( 0x1 << region )) 
      rval = SOTP_E_ROW_READ_LOCK;

   return rval;
}

/*  *********************************************************************
    *  static SotpRowStatus sotpReadEccRow(uint32_t row, uint32_t *pDst)
    *  
    *  Input parameters: 
    *     row   - This func is allowed to read from rows 28 thru 111
    *     pDst  - A pointer to the location that the retrieved 32 data
    *             bits will be written to.
    *     raw_read - If set then ECC checks are skipped
    *     
    *  Return value:
    *     SOTP_S_ROW_SUCCESS     - The function has completed successfully
    *     SOTP_S_ROW_ECC_COR     - The function ecc corrected 1 bad bit when 
    *                              returning row data (pDst points to valid data)
    *     SOTP_E_ROW_ECC_DET     - The function detected 2 or more uncorrectable 
    *                              bits within the row (pDst is invalid)
    *     SOTP_E_ROW_FAIL_SET    - The function detected that 1 or both of 
    *                              the fail bits are set (pDst is invalid)
    *     SOTP_E_ROW_READ_LOCK   - The row in question is within a region that 
    *                              is read-locked until the next POR
    *     SOTP_E_ROW_TIMEOUT     - The function experienced unexpected FSM 
    *                              timeout from the SOTP block (pDst is invalid)
    *     SOTP_E_ROW_ERROR       - The function experienced unexpected register 
    *                              ret value from SOTP block (pDst is invalid)
    *
    ********************************************************************* */
static SotpRowStatus sotpReadEccRow(uint32_t row, uint32_t *pDst, uint32_t raw_read)
{
   SotpRowStatus rval;


   SOTP_DBG("%s: row:%d bufp:%px\n", __FUNCTION__, row, pDst);

   if( row < SOTP_FIRST_ECC_CONFIG_ROW || row > SOTP_NUM_ROWS-1 )
   {
      SOTP_ERR_PRINT("%s: ECC Row %d out of bound! Min:%d Max:%d\n", __FUNCTION__, row, SOTP_FIRST_ECC_CONFIG_ROW, SOTP_NUM_ROWS-1);
      return SOTP_E_ROW_ERROR;
   }

   /* Check to see that the row in question is not readlocked until the next POR */
   if ((rval = sotpRegionReadLockStatus(row/SOTP_ROWS_IN_REGION)) == SOTP_E_ROW_READ_LOCK)
   {
      SOTP_ERR_PRINT("%s: Row %d is locked from reading. Returned key data is invalid\n", __FUNCTION__, row);
      return rval;
   }

   /* Poll for FDONE bit before doing any access */
   if ((rval = sotpPollForFDone()) == SOTP_E_ROW_TIMEOUT)
   {
      SOTP_ERR_PRINT("%s: Row %d read timed out. \n", __FUNCTION__, row);
      return rval;
   }

   /* Clear ECC detect, ecc correct as well as command done bits */
   sotp_base_ptr->sotp_otp_status_1 = (SOTP_OTP_STATUS_1_CMD_DONE | SOTP_OTP_STATUS_1_ECC_COR | SOTP_OTP_STATUS_1_ECC_DET);

   /* enable ecc checks (low), zero out the regs_ecc_en bits  ... then turn on cpu mode, and set regs_ecc_en to enabled */
   /* expect Bug?  When ecc=0 then programs 0x0, Reg says that it should be set to 4'b0101 : ECC check DISABLED */
   sotp_base_ptr->sotp_otp_prog_ctrl &= ~( SOTP_OTP_PROG_CTRL_OTP_DISABLE_ECC | SOTP_OTP_PROG_CTRL_REGS_ECC_EN_MASK );

   /* Disable ECC checks for raw reads */
   if( raw_read )
      sotp_base_ptr->sotp_otp_prog_ctrl |= ( SOTP_OTP_PROG_CTRL_OTP_CPU_MODE_EN | SOTP_OTP_PROG_CTRL_OTP_DISABLE_ECC | SOTP_OTP_PROG_CTRL_REGS_ECC_DIS );
   else
      sotp_base_ptr->sotp_otp_prog_ctrl |= ( SOTP_OTP_PROG_CTRL_OTP_CPU_MODE_EN | SOTP_OTP_PROG_CTRL_REGS_ECC_EN );

   /* Set OTP addr with the row to be read */
   sotp_base_ptr->sotp_otp_addr = row << SOTP_OTP_ADDR_OTP_ADDR_SHIFT;

   /* set up the OTP_CMD for OTP_READ */
   sotp_base_ptr->sotp_otp_ctrl_0 = SOTP_OTP_CTRL_0_OTP_CMD_OTP_READ;

   /* Start the state machine */
   if ((rval = sotpSetStartAndPollForCmdDone()) == SOTP_S_ROW_SUCCESS)
   {
      /* If the row is marked as failed, abort */
      if ( sotp_base_ptr->sotp_otp_rdata_1 & SOTP_OTP_RDATA_1_FAIL_MASK )
      {
         SOTP_ERR_PRINT("Row is marked as failed. \n");
         return ( SOTP_E_ROW_FAIL_SET );
      }
 
      /* See if the data is bad and ecc correction was not possible */
      if ( sotp_base_ptr->sotp_otp_status_1 & SOTP_OTP_STATUS_1_ECC_DET )
      {
         SOTP_ERR_PRINT("Row data failed ecc with 2 or more bits . \n");
            return ( SOTP_E_ROW_ECC_DET );
      }

      /* See if the data is good but an ecc correction took place */
      if ( sotp_base_ptr->sotp_otp_status_1 & SOTP_OTP_STATUS_1_ECC_COR )
      {
         SOTP_ERR_PRINT("Row data valid, but ecc corrected 1 bit. \n");
         rval = SOTP_S_ROW_ECC_COR;
      }

      /* Data in row is valid. Pass it to return buffer */
      *pDst = sotp_base_ptr->sotp_otp_rdata_0;

      /* Clear the CMD_DONE bit */
      sotp_base_ptr->sotp_otp_status_1 = SOTP_OTP_STATUS_1_CMD_DONE;

      /* turn off cpu mode */
      sotp_base_ptr->sotp_otp_prog_ctrl &= ~SOTP_OTP_PROG_CTRL_OTP_CPU_MODE_EN;
   }

   return rval;
}

/*  *********************************************************************
    *  extern SotpKeyStatus sotpReadKeyslotData(uint32_t section, uint32_t *pDst, uint32_t len)
    *  
    *  Description - Used by the bootrom or bootloader to read a key from SOTP
    *
    *  Input parameters: 
    *     section - Section that the key info should be read from
    *               (7 =< section =< 13)
    *     pDst    - pointer to the buffer containing the key (chopped 
    *               up into 32 bit words)
    *     len     - The number of 32 bit words in the key to read in (len =< 8)
    *     
    *  Return value:
    *     SOTP_S_KEY_SUCCESS - The function has completed successfully, and
    *                          pDst holds valid data
    *     SOTP_S_KEY_EMPTY   - The function has completed successfully, but 
    *                          no credential is currently stored in this section
    *     SOTP_E_KEY_CRC_MIS - The function detected a CRC mismatch during the key read 
    *     SOTP_E_KEY_UNDERRUN- The function ran out of rows within a section
    *                          before reading the entire key and crc
    *     SOTP_E_KEY_ERROR   - The function experienced an error in one of the
    *                          rows of the section
    *
    ********************************************************************* */
static SotpKeyStatus sotpReadKeyslotData(uint32_t section, uint32_t *pDst, uint32_t len)
{
   uint32_t startRow, row, printRow, readCrc;
   uint32_t calcCrc = CRC32_INIT_VALUE;
   uint32_t i = 0;
   SotpKeyStatus rval    = SOTP_E_KEY_BADPARAM;
   SotpRowStatus rowRval = SOTP_S_ROW_SUCCESS;

   SOTP_DBG("%s: section:%d bufp:%px len:%d\n", __FUNCTION__, section, pDst, len);

   /* perform checks of input params */
   if ((pDst == NULL) || (len == 0) || (len > SOTP_MAX_KEYLEN))
      return rval;

   if ((section < SOTP_MIN_KEYSLOT) || (section > SOTP_MAX_KEYSLOT))
   {
      SOTP_ERR_PRINT("%s: keyslot section:%d out of bound! Min:%d Max:%d\n", __FUNCTION__, section, SOTP_MIN_KEYSLOT, SOTP_MAX_KEYSLOT);
      return rval;
   }

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
         rowRval = sotpReadEccRow(row, &pDst[i], 0); /* currently reading the key */

      if (i == len)
         rowRval = sotpReadEccRow(row, &readCrc, 0); /* currently reading the crc */

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
                        SOTP_ERR_PRINT("\nCalculated crc did not match crc stored in row %d. Returned key data is invalid\n", row);
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
            SOTP_ERR_PRINT("\n2 or more Uncorrectable bits within row %d. Returned key data is invalid \n", row);
            return SOTP_E_KEY_ERROR;
            break;

         case SOTP_E_ROW_READ_LOCK:
            SOTP_ERR_PRINT("\nRow %d is locked from reading. Returned key data is invalid\n", row);
            return SOTP_E_KEY_ERROR;
            break;

         case SOTP_E_ROW_TIMEOUT:
            SOTP_ERR_PRINT("\nUnexpected SOTP block timeout while reading row %d. Returned key data is invalid\n", row);
            return SOTP_E_KEY_ERROR;
            break;

         case SOTP_E_ROW_ERROR:
         default:
            SOTP_ERR_PRINT("\nUnknown condition in reading row %d. Returned key data is invalid\n", row);
            return SOTP_E_KEY_ERROR;
            break;
      }
   }

   /* If we make it here, we ran out of rows in the key slot ... returned data is not valid */
   SOTP_ERR_PRINT("\nRan out of rows in section %d before reading entire key and crc. Returned key data is invalid\n", section);
   return SOTP_E_KEY_UNDERRUN;
}

/*  *********************************************************************
    *  static SotpKeyStatus sotpLockSecKeyFuse(uint32_t section)
    *  
    *  Input parameters: 
    *      section - There are 7 locations called sections that can hold 
    *                up to 256 bits of key information. (7 =< section =< 13).
    *                This is the location that is to be locked down from
    *                future fusing. This is a permanent lockout even after POR.
    *      
    *  Return value:
    *      SOTP_S_KEY_SUCCESS     - The function has completed successfully
    *      SOTP_E_KEY_BADPARAM    - The function recieved a bad input parameter
    *      SOTP_E_KEY_ERROR       - The function experienced unexpected register 
    *                               ret value from SOTP block 
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

static SotpKeyStatus sotpReadLockRegion(uint32_t region)
{

   /* enable ecc checks (low), zero out the regs_ecc_en bits  ... then turn on cpu mode, and set regs_ecc_en to enabled */
   /* expect Bug?  When ecc=0 then programs 0x0, Reg says that it should be set to 4'b0101 : ECC check DISABLED */
   sotp_base_ptr->sotp_otp_prog_ctrl &= ~SOTP_OTP_PROG_CTRL_REGS_ECC_EN_MASK;
   sotp_base_ptr->sotp_otp_prog_ctrl |=  ( SOTP_OTP_PROG_CTRL_OTP_CPU_MODE_EN | SOTP_OTP_PROG_CTRL_REGS_ECC_DIS );

   /* wait a little */
   msleep(1);

   /* The read lock register is a write-once register on a bit-for-bit bassis     */
   /* so you don't have to worry about preserving bits that are already set to 1. */
   sotp_base_ptr->sotp_otp_rd_lock = 1 << region;

   /* wait a little */
   msleep(1);

   /* turn off cpu mode */
   sotp_base_ptr->sotp_otp_prog_ctrl &= ~SOTP_OTP_PROG_CTRL_OTP_CPU_MODE_EN;

   return SOTP_S_KEY_SUCCESS;
}

/*  *********************************************************************
    *  extern SotpKeyStatus sotpReadLockKeyslot(uint32_t section)
    *  
    *  Description - After the bootrom or bootloader has read a key from
    *                SOTP, this function can be called so that no 
    *                other sw can read this key at a later time.
    *
    *  Input parameters: 
    *      section - There are 7 locations called sections that can hold 
    *                up to 256 bits of key information. (7 =< section =< 13)
    *                This is the location that is to be locked down from
    *                reading until the next power-on-reset.
    *      
    *  Return value:
    *      SOTP_S_KEY_SUCCESS     - The function has completed successfully
    *      SOTP_E_KEY_BADPARAM    - The function recieved a bad input parameter
    *
    ********************************************************************* */
static SotpKeyStatus sotpReadLockKeyslot(uint32_t section)
{
   uint32_t startRegion;

   /* perform checks of input params */
   if ((section < SOTP_MIN_KEYSLOT) || (section > SOTP_MAX_KEYSLOT))
      return SOTP_E_KEY_BADPARAM;

   /* enable ecc checks (low), zero out the regs_ecc_en bits  ... then turn on cpu mode, and set regs_ecc_en to enabled */
   /* expect Bug?  When ecc=0 then programs 0x0, Reg says that it should be set to 4'b0101 : ECC check DISABLED */
   sotp_base_ptr->sotp_otp_prog_ctrl &= ~SOTP_OTP_PROG_CTRL_REGS_ECC_EN_MASK;
   sotp_base_ptr->sotp_otp_prog_ctrl |=  ( SOTP_OTP_PROG_CTRL_OTP_CPU_MODE_EN | SOTP_OTP_PROG_CTRL_REGS_ECC_DIS );

   /* Determine the starting region to lock */
   startRegion = SOTP_FIRST_KEYSLOT_REGION + ((section - SOTP_MIN_KEYSLOT) * SOTP_REGIONS_IN_KEYSLOT);

   /* wait a little */
   msleep(1);

   /* The read lock register is a write-once register on a bit-for-bit bassis     */
   /* so you don't have to worry about preserving bits that are already set to 1. */
   sotp_base_ptr->sotp_otp_rd_lock = SOTP_REGIONS_MASK_IN_KEYSLOT << startRegion;

   /* wait a little */
   msleep(1);

   /* turn off cpu mode */
   sotp_base_ptr->sotp_otp_prog_ctrl &= ~SOTP_OTP_PROG_CTRL_OTP_CPU_MODE_EN;

   return SOTP_S_KEY_SUCCESS;
}


static SotpKeyStatus sotpKeySlotReadLockStatus(uint32_t section)
{
   uint32_t startRegion;
   int i;

   /* perform checks of input params */
   if ((section < SOTP_MIN_KEYSLOT) || (section > SOTP_MAX_KEYSLOT))
      return SOTP_E_KEY_BADPARAM;

   /* enable ecc checks (low), zero out the regs_ecc_en bits  ... then turn on cpu mode, and set regs_ecc_en to enabled */
   /* expect Bug?  When ecc=0 then programs 0x0, Reg says that it should be set to 4'b0101 : ECC check DISABLED */
   sotp_base_ptr->sotp_otp_prog_ctrl &= ~SOTP_OTP_PROG_CTRL_REGS_ECC_EN_MASK;
   sotp_base_ptr->sotp_otp_prog_ctrl |=  ( SOTP_OTP_PROG_CTRL_OTP_CPU_MODE_EN | SOTP_OTP_PROG_CTRL_REGS_ECC_DIS );

   /* Determine the starting region to lock */
   startRegion = SOTP_FIRST_KEYSLOT_REGION + ((section - SOTP_MIN_KEYSLOT) * SOTP_REGIONS_IN_KEYSLOT);

   for( i=0; i<SOTP_REGIONS_IN_KEYSLOT; i++ )
   {

       /* Check to see if that region is locked */
       if (sotp_base_ptr->sotp_otp_rd_lock & ( 0x1 << startRegion )) 
          return SOTP_E_ROW_READ_LOCK;

       /* Go to next region in keyslot */
       startRegion++;
   }

   /* turn off cpu mode */
   sotp_base_ptr->sotp_otp_prog_ctrl &= ~SOTP_OTP_PROG_CTRL_OTP_CPU_MODE_EN;

   return 0;
}

/*  *********************************************************************
    *  extern SotpKeyStatus sotpWriteKeyslotData(uint32_t section, uint32_t *pSrc, uint32_t len)
    *  
    *  Description - Used during manufacturing to fuse a security credential
    *                into SOTP
    *
    *  Input parameters: 
    *     section - There are 7 locations called sections that can hold 
    *               up to 256 bits of key information. (7 =< section =< 13)
    *     pSrc    - pointer to the buffer containing the key (chopped 
    *               up into 32 bit words)
    *     len     - The number of 32 bit words in the key (len =< 8)
    *     
    *  Return value:
    *     SOTP_S_KEY_SUCCESS - The function has completed successfully
    *     SOTP_E_KEY_BADPARAM- The function recieved a bad input parameter
    *     SOTP_E_KEY_OVERRUN - The function ran out of rows within a section
    *                          before the entire key and crc was fused
    *     SOTP_E_KEY_TIMEOUT - The function experienced unexpected FSM 
    *                          timeout from the SOTP block 
    *     SOTP_E_KEY_ERROR   - The function experienced unexpected register 
    *                          ret value from SOTP block 
    *
    ********************************************************************* */
static SotpKeyStatus sotpWriteKeyslotData(uint32_t section, uint32_t *pSrc, uint32_t len)
{
   uint32_t startRow, row, calcCrc, startRegion, fuse_lock, read_lock;
   uint32_t rowData = 0;
   uint32_t i = 0;
   SotpKeyStatus rval    = SOTP_E_KEY_BADPARAM;
   SotpRowStatus rowRval = SOTP_S_ROW_SUCCESS;

   SOTP_DBG("%s: section:%d bufp:%px len:%d\n", __FUNCTION__, section, pSrc, len);

   /* perform checks of input params */
   if ((pSrc == NULL) || (len == 0) || (len > SOTP_MAX_KEYLEN))
      return rval;

   if ((section < SOTP_MIN_KEYSLOT) || (section > SOTP_MAX_KEYSLOT))
   {
      SOTP_ERR_PRINT("%s: keyslot section:%d out of bound! Min:%d Max:%d\n", __FUNCTION__, section, SOTP_MIN_KEYSLOT, SOTP_MAX_KEYSLOT);
      return rval;
   }

   /* Calculate the crc that is to be stored into the key slot */
   calcCrc = getCrc32((unsigned char *)pSrc, (len*4), CRC32_INIT_VALUE);

   /* Determine the starting row to fuse */
   startRow = ((section - SOTP_MIN_KEYSLOT) * SOTP_ROWS_IN_KEYSLOT) + SOTP_FIRST_KEYSLOT_ROW;

   /* Determine starting region */
   startRegion = startRow / SOTP_ROWS_IN_REGION;

   /* Make sure that the regions we are writing to are not read-locked or fuse-locked */
   for( i = startRegion; i < startRegion + SOTP_REGIONS_IN_KEYSLOT; i++ )
   {
      fuse_lock = sotpIsRegionFuseLocked(i);
      read_lock = sotpRegionReadLockStatus(i);
      if( fuse_lock || read_lock )
      {
         SOTP_ERR_PRINT("%s: section:%d region:%d %s_locked! Abort keyslot programming!\n", __FUNCTION__, section, i, (fuse_lock?"fuse":"read"));
         return rval;
      }
   }

   /* Make sure that entire keyslot is empty before attempting to program */
   for ( row = startRow ; row < (startRow + SOTP_ROWS_IN_KEYSLOT); row++)
   {
      /* TODO: Add counter to allow certain amount of failiures ( 3 redundant ) */
      rowRval = sotpReadEccRow(row, &rowData, 0);
      if( rowRval != SOTP_S_ROW_SUCCESS )
      {
         SOTP_ERR_PRINT("%s: Cant read keyslot section:%d row:%d! Abort keyslot programming!\n", __FUNCTION__, section, row);
         return rval;
      }

      if(rowData)
      {
         SOTP_ERR_PRINT("%s: keyslot section:%d row:%d not empty! Abort keyslot programming!\n", __FUNCTION__, section, row);
         return rval;
      }
   }

   /* Start fusing the key and CRC */
   for ( row=startRow, i=0 ; row < (startRow + SOTP_ROWS_IN_KEYSLOT); row++)
   {
      if (i < len)
         rowRval = sotpWriteEccRow(row, pSrc[i], 0); /* currently writing the key */

      if (i == len)
         rowRval = sotpWriteEccRow(row, calcCrc, 0); /* write the crc */

      /* See if it was successful */
      switch(rowRval)
      {

         case SOTP_S_ROW_SUCCESS:
         {
            /* Check the integrity of the row just fused */
            rowRval = sotpReadEccRow(row, &rowData, 0);

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
                        SOTP_ERR_PRINT("\nWritten and read data didn't match within row %d. Section %d is unusable\n", row, section);
                        return SOTP_E_KEY_ERROR;
                     }
                  }
                  else
                  {
                     if (calcCrc == rowData)
                        return(sotpLockSecKeyFuse(section)); /* key and crc fused. Lock section from further writes */
                     else
                     {
                        SOTP_ERR_PRINT("\nCalculated and read crc didn't match within row %d. Section %d is unusable\n", row, section);
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
                     SOTP_ERR_PRINT("\nFailed to fuse fail bits within row %d. Section %d is unusable\n", row, section);
                     return SOTP_E_KEY_ERROR;
                  }
                  break;
               }

               case SOTP_E_ROW_FAIL_SET:
               {
                  SOTP_ERR_PRINT("\nFail bits have previously been fused within row %d. Section %d is unusable\n", row, section);
                  return SOTP_E_KEY_ERROR;
                  break;
               }

               case SOTP_E_ROW_READ_LOCK:
               {
                  SOTP_ERR_PRINT("\nRow %d is locked from reading. Cannot verify. Section %d may be unusable\n", row, section);
                  return SOTP_E_KEY_ERROR;
                  break;
               }

               default:
               {
                  SOTP_ERR_PRINT("\nUnknown condition in reading row %d. Section %d may be unusable\n", row, section);
                  return SOTP_E_KEY_ERROR;
                  break;
               }
            }

            break;
         }

         case SOTP_E_ROW_FUSE_LOCK:
         {
            SOTP_ERR_PRINT("\nRow %d is locked from fusing. Section %d may be unusable\n", row, section);
            return SOTP_E_KEY_ERROR;
            break;
         }

         case SOTP_E_ROW_TIMEOUT:
         {
            SOTP_ERR_PRINT("\nUnexpected SOTP block timeout while fusing row %d. Section %d may be unusable\n", row, section);
            return SOTP_E_KEY_ERROR;
            break;
         }

         case SOTP_E_ROW_ERROR:
         default:
         {
            SOTP_ERR_PRINT("\nUnknown condition while fusing row %d. Section %d may be unusable\n", row, section);
            return SOTP_E_KEY_ERROR;
            break;
         }

      }
   }

   SOTP_ERR_PRINT("\nRan out of rows before fusing entire key and crc. Section %d is unusable \n", section);
   return SOTP_E_KEY_OVERRUN;
}
#endif /* SOTP_DBG_API */

static void dump_regions(int first_region, int last_region, int section, int regions_in_section )
{
    int i, j, fuse_lock, read_lock, row_data;
    uint64_t row_data_nonecc;
    int ret = 0;
    int row_result;
    for( i=first_region; i<last_region; i++ )
    {
        if( ((i-first_region) % regions_in_section) == 0 )
        {
                if( i-first_region )
                        section++;
                SOTP_PRINT("    Section[%d]:\n", section);
        }

        fuse_lock = sotpIsRegionFuseLocked(i);
        read_lock = sotpRegionReadLockStatus(i);
        SOTP_PRINT("        Region[%d]: Fuselock=%d Readlock=%d\n",
                i,
                (fuse_lock == SOTP_E_ROW_FUSE_LOCK),
                (read_lock == SOTP_E_ROW_READ_LOCK) );
        if( !read_lock )
        {
            for( j=i*SOTP_ROWS_IN_REGION; j<i*SOTP_ROWS_IN_REGION + SOTP_ROWS_IN_REGION;j++ )
            {
                if( j < SOTP_FIRST_ECC_CONFIG_ROW )
                    ret = sotpReadNonEccRow( j, &row_data_nonecc);
                else
                    ret = sotp_get_row_data( j, (char*)&row_data, sizeof(row_data), &row_result, 1 );
                if( ret < 0 )
                {
                    row_data = 0xDEADBEEF;
                    row_data_nonecc = 0xDEADBEEFDEADBEEF;
                }

                if( j < SOTP_FIRST_ECC_CONFIG_ROW )
                    SOTP_PRINT("            Row[%d]: 0x%016llx\n", j, row_data_nonecc);
                else
                    SOTP_PRINT("            Row[%d]: 0x%08x\n", j, row_data);

                row_data = 0;
                row_data_nonecc = 0;
            }
        }

    }
}

/***************************************************************************
* Accessor functions
***************************************************************************/
int sotp_dump_map( int * result )
{
    int first_region,last_region, section;

    SOTP_BASE_CHK();

    SOTP_PRINT("+---------------+:\n");
    SOTP_PRINT("| SOTP raw data |:\n");
    SOTP_PRINT("+---------------+:\n\n");
    
    section = SOTP_WRAPPER_LOCK_SECTION;
    first_region = SOTP_FIRST_FUSELOCK_ROW/SOTP_ROWS_IN_REGION;
    last_region = first_region + SOTP_REGIONS_IN_LOCK;
    SOTP_PRINT("Wrapper/Prog LOCK SOTP Sections:\n");
    dump_regions(first_region, last_region, section, SOTP_REGIONS_IN_LOCK);

    section = SOTP_DEVICE_CFG_SECTION;
    first_region = last_region;
    last_region = first_region + SOTP_REGIONS_IN_DEVCFG;
    SOTP_PRINT("Device Config SOTP Section:\n");
    dump_regions(first_region, last_region, section, SOTP_REGIONS_IN_DEVCFG);

    section = SOTP_FIRST_CFG_SECTION;
    first_region = last_region;
    last_region = SOTP_FIRST_KEYSLOT_ROW/SOTP_ROWS_IN_REGION;
    SOTP_PRINT("General Config SOTP Sections:\n");
    dump_regions(first_region, last_region, section, SOTP_REGIONS_IN_CFG);

    section = SOTP_FIRST_KEYSLOT_SECTION;
    first_region = last_region;
    last_region = SOTP_NUM_ROWS/SOTP_ROWS_IN_REGION;
    SOTP_PRINT("Keyslot SOTP Sections:\n");
    dump_regions(first_region, last_region, section, SOTP_REGIONS_IN_KEYSLOT);
    *result = SOTP_S_ROW_SUCCESS;
    return (0);
}

int sotp_get_row_data( int row_addr, char* sotp_data, int data_len, int * result, int raw )
{
    SOTP_BASE_CHK();

    *result = sotpReadEccRow( row_addr, (uint32_t*)sotp_data, raw);
    SOTP_DBG("%s: addr:%d, len:%d, res:%d, value:0x%08x\n", __FUNCTION__, row_addr, data_len, *result, *(uint32_t*)sotp_data);

    if( *result != SOTP_S_KEY_SUCCESS && *result != SOTP_S_ROW_ECC_COR )
        return -1;
    else
        return (0);
}

int sotp_set_row_data( int row_addr, char* sotp_data, int data_len, int * result, int raw )
{
    uint32_t temp_data = 0;

    SOTP_BASE_CHK();

    /* Protected write. Ensure that row has never been written to */
    if( !raw )
    {
        *result = sotpReadEccRow( row_addr, &temp_data, 1);
        if( *result != SOTP_S_ROW_SUCCESS)
            return -1;

        if( temp_data )
        {
            SOTP_ERR_PRINT("%s: Row already has valid data! Not writing SOTP row\n", __FUNCTION__);
            return -1;
        }
    }

    *result = sotpWriteEccRow( row_addr, *(uint32_t*)sotp_data, raw);
    SOTP_DBG("%s: addr:%d, len:%d, res:%d, value:0x%08x\n", __FUNCTION__, row_addr, data_len, *result, *(uint32_t*)sotp_data);

    if( *result != SOTP_S_ROW_SUCCESS)
        return -1;
    else
        return (0);
}

int sotp_get_keyslot_data( int section_num, char* sotp_data, int data_len, int * result )
{
    int i=0;

    SOTP_BASE_CHK();

    *result = sotpReadKeyslotData(section_num, (uint32_t*)sotp_data, data_len/sizeof(uint32_t));
    SOTP_DBG("%s: addr:%d, len:%d, res:%d, value:\n", __FUNCTION__, section_num, data_len, *result);
    for( i=0; i<data_len/sizeof(uint32_t); i++ )
    {
        SOTP_DBG("0x%08x\n", *((uint32_t *)sotp_data+i));
    }

    if( *result != SOTP_S_KEY_SUCCESS && *result != SOTP_S_ROW_ECC_COR )
        return -1;
    else
        return (0);
}

int sotp_set_keyslot_data( int section_num, char* sotp_data, int data_len, int * result )
{
    int i=0;

    SOTP_BASE_CHK();

    *result = sotpWriteKeyslotData(section_num, (uint32_t*)sotp_data, data_len/sizeof(uint32_t));
    SOTP_DBG("%s: addr:%d, len:%d, res:%d, value:\n", __FUNCTION__, section_num, data_len, *result);
    for( i=0; i<data_len/sizeof(uint32_t); i++ )
    {
        SOTP_DBG("0x%08x\n", *((uint32_t *)sotp_data+i));
    }

    if( *result != SOTP_S_KEY_SUCCESS)
        return -1;
    else
        return (0);
}

int sotp_set_region_readlock(int region_num, int * result )
{
    SOTP_BASE_CHK();

    SOTP_DBG("%s: addr:%d\n", __FUNCTION__, region_num);
    *result = sotpReadLockRegion(region_num);

    if( *result != SOTP_S_KEY_SUCCESS)
        return -1;
    else
        return (0);
}

int sotp_set_region_fuselock(int region_num, int * result )
{
    SOTP_BASE_CHK();

    SOTP_DBG("%s: addr:%d\n", __FUNCTION__, region_num);
    *result = sotpFuseLockRegion(region_num);

    if( *result != SOTP_S_KEY_SUCCESS)
        return -1;
    else
        return (0);
}

int sotp_set_keyslot_readlock(int section_num, int * result )
{
    SOTP_BASE_CHK();

    SOTP_DBG("%s: addr:%d\n", __FUNCTION__, section_num);
    *result = sotpReadLockKeyslot(section_num);

    if( *result != SOTP_S_KEY_SUCCESS)
        return -1;
    else
        return (0);
}

int sotp_get_region_fuselock_status(int region_num, int * result )
{
    SOTP_BASE_CHK();

    *result = sotpIsRegionFuseLocked(region_num);

    if( *result == SOTP_E_ROW_FUSE_LOCK)
        *result = 1;
    else
        *result = 0;

    SOTP_DBG("%s: addr:%d res:%d\n", __FUNCTION__, region_num, *result);

    return (0);
}

int sotp_get_region_readlock_status(int region_num, int * result )
{
    SOTP_BASE_CHK();

    *result = sotpRegionReadLockStatus(region_num);

    if( *result == SOTP_E_ROW_READ_LOCK)
        *result = 1;
    else
        *result = 0;

    SOTP_DBG("%s: addr:%d res:%d\n", __FUNCTION__, region_num, *result);

    return (0);
}

int sotp_get_keyslot_readlock_status(int section_num, int * result )
{  
    SOTP_BASE_CHK();

    *result = sotpKeySlotReadLockStatus(section_num);

    if( *result == SOTP_E_ROW_READ_LOCK)
        *result = 1;
    else
        *result = 0;

    SOTP_DBG("%s: addr:%d res:%d\n", __FUNCTION__, section_num, *result);

    return (0);
}

int sotp_set_rollback_lvl( char * lvl, int * result)
{
    uint32_t row_val = 0;
    int i;
    int raw = 1;
    uint32_t level = *(uint32_t*)lvl;
    uint32_t current_level = 0;

    SOTP_BASE_CHK();

    if( level > SOTP_MAX_ROLLBACK_LVL )
    {       
        SOTP_ERR_PRINT("Invalid Rollback Level! Max Rollback lvl is %d\n", SOTP_MAX_ROLLBACK_LVL);
        return -1;
    }

    SOTP_DBG("%s: level:%d\n", __FUNCTION__, level);

    /* get current level and determine if we can increase level */
    sotp_get_rollback_lvl( (char *)&current_level, result); 
    if( *result != SOTP_S_KEY_SUCCESS && *result != SOTP_S_ROW_ECC_COR )
    {
        SOTP_ERR_PRINT("Error retrieving Current Rollback Level %d\n",*result );
        return -1;
    }
    else if( level <= current_level )
    {
        SOTP_ERR_PRINT("Error! requested Rollback Level %d <= Current Level %d\n", level, current_level );
        return -1;
    }

    for( i=STOP_FIRST_ROLLBACK_ROW; (i <= SOTP_LAST_ROLLBACK_ROW) && (level > 0); i++ )
    {
        *result = sotpReadEccRow( i, &row_val, raw);
        SOTP_DBG("%s: addr:%d, res:%d, rd_value:0x%08x\n", __FUNCTION__, i, *result, row_val);
        if( *result != SOTP_S_KEY_SUCCESS && *result != SOTP_S_ROW_ECC_COR )
            return -1;

        /* Batch fill the row if level exceeds it */
        if( level >= SOTP_MAX_ROLLBACK_LVLS_PER_ROW )
        {
            level -= SOTP_MAX_ROLLBACK_LVLS_PER_ROW;
            if( row_val == SOTP_MAX_ROLLBACK_ROW_VALUE )
                continue;
            else
                row_val = SOTP_MAX_ROLLBACK_ROW_VALUE;
        }
        else
        {
            /* Selectively fill the row according to level */
            while( level )
            {
                row_val |= SOTP_ROLLBACK_LVL_MASK << (level-1)*SOTP_ROLLBACK_LVL_SHIFT;
                level--;
            }   
        }

        /* Write row to SOTP */
        if( row_val )
        {
            *result = sotpWriteEccRow( i, row_val, raw);
            SOTP_DBG("%s: addr:%d, res:%d, wr_value:0x%08x, level:%d\n", __FUNCTION__, i, *result, row_val, level);
            if( *result != SOTP_S_ROW_SUCCESS)
                return -1;
        }
    }
    return 0;
}
 
int sotp_get_rollback_lvl( char * lvl, int * result)
{
    int raw = 1;
    uint32_t row_val = 0;
    uint32_t current_lvl = 0;
    int i,j;

    SOTP_BASE_CHK();

    for( i=STOP_FIRST_ROLLBACK_ROW; i <= SOTP_LAST_ROLLBACK_ROW; i++ )
    {
        /* Read row from SOTP */
        *result = sotpReadEccRow( i, &row_val, raw);
        SOTP_DBG("%s: addr:%d, res:%d, value:0x%08x\n", __FUNCTION__, i, *result, row_val);
        if( *result != SOTP_S_KEY_SUCCESS && *result != SOTP_S_ROW_ECC_COR )
                    return -1;

        /* Calculate rollback lvl from row data */
        for( j=0; j < SOTP_MAX_ROLLBACK_LVLS_PER_ROW; j++ )
        {
            if( row_val & (SOTP_ROLLBACK_LVL_MASK << j*SOTP_ROLLBACK_LVL_SHIFT))
                current_lvl++;
            else
                goto finish;    
        }
    }

finish: 
    *(uint32_t *)lvl = current_lvl;
    return 0;
}

int sotp_init ( void * base_ptr )
{
    if( !base_ptr )
        return -1;

    sotp_base_ptr = (SotpRegs*)base_ptr;
    return 0;
}

