/* SPDX-License-Identifier: GPL-2.0+
 *
 * Copyright 2020 Broadcom Ltd.
 */

/***************************************************************************
*
* Description: Secure OTP  accessor for SoCs: 
* 	6856,6858,6846,63158,4908,47622,63178,6756


* write(fuse)/read key and rows for the SEC KEY section , read/write(fuse) status per region 
*
*
***************************************************************************/

/* Includes. */

#include <common.h>
#include <linux/types.h>
#include <linux/io.h>
#include "linux/printk.h"
#include <asm/arch/misc.h>

#include "otp_map_cmn.h"
#include "sotp.h"

//#define OTP_DRY_RUN
//#define DEBUG_SOTP

#ifdef DEBUG_SOTP
#define   SOTP_DBG(...) printf(__VA_ARGS__)
#define _ETR(__p__)  printf("%s \n\t\t\t\t -> %s:%d ERROR code %d\n",__FILE__, __FUNCTION__,__LINE__,__p__)
#define _TR(__p__)  printf("%s \n\t\t\t\t -> %s:%d code %d / 0x%x\n",__FILE__, __FUNCTION__,__LINE__,__p__,__p__)
#else
#define   SOTP_DBG(...)
#define _ETR(__p__)
#define _TR(__p__)
#endif

#define SOTP_ERR_PRINT printf


typedef enum __section_id {
	SOTP_SECT_ID_RSVD = 0,
	SOTP_SECT_ID_BLOCK_LOCK,
	SOTP_SECT_ID_PROGLOCK,
	SOTP_SECT_ID_DEVICE_CFG,
	SOTP_SECT_ID_GENERAL_CFG_BCM,
	SOTP_SECT_ID_GENERAL_CUST_ID,
	SOTP_SECT_ID_GENERAL_VEND_ID,
	SOTP_SECT_ID_SEC_KEY_1,
	SOTP_SECT_ID_SEC_KEY_2,
	SOTP_SECT_ID_SEC_KEY_3,
	SOTP_SECT_ID_SEC_KEY_4,
	SOTP_SECT_ID_SEC_KEY_5,
	SOTP_SECT_ID_SEC_KEY_6,
	SOTP_SECT_ID_SEC_KEY_7,
	SOTP_SECT_ID_MAX,
} sotp_section_id_t;

static  int sectn_map[SOTP_SECT_ID_MAX] = 
			{8, 2, 2, 4, 4, 4, 4, 12, 12, 12, 12, 12, 12, 12 };


/*
 * locates row id
 * */
static otp_hw_cmn_err_t  lookup_row(otp_hw_cmn_t* dev, 
	u32 addr,
	otp_hw_cmn_row_t** row)
{
	u32 i;
	otp_hw_cmn_row_t* rows = dev->rows;
	for (i = 0; i < dev->row_max; i++) {
		if ( (addr >= rows[i].addr) && (addr < (rows[i].addr + rows[i].range)) ) {
			*row = &rows[i];
			return OTP_HW_CMN_OK;	
		}
	}
	_ETR(OTP_HW_CMN_ERR_UNSP);
	return OTP_HW_CMN_ERR_UNSP;
}

static u32 row2regn(u32 r)
{
	return (r/ROWS_PER_RGN);
}

static int row2sect(int row, u32* sect)
{
	int i; 
	if (row < 0 || row > sectn_map[SOTP_SECT_ID_MAX-1] - 1) {
		return -1;
	}

	for (i = 0; i < SOTP_SECT_ID_MAX; i++) {
		if (row <= (int)sectn_map[i] - 1) {
			*sect = i;
			break;	
		}
	}
	SOTP_DBG("row %u --> sect %u\n",row, *sect); 
	return 0;
}

/*
static inline int sect2row(otp_hw_cmn_section_id_t s, u32 row_range[2])
{
	if (s >= SOTP_SECT_ID_RSVD  && s < SOTP_SECT_ID_MAX) {
		row_range[0] = !s ? 0 : sectn_map[s-1];
		row_range[1] = sectn_map[s] - row_range[0];
		return 0;
	}
	return -1;
}
*/
static inline u32 sect2regn(u32 s, u32 regn_range[2])
{
	if (s >= SOTP_SECT_ID_RSVD  && s < SOTP_SECT_ID_MAX) {
		regn_range[0] = !s ? 0 : sectn_map[s-1]/ROWS_PER_RGN ;
		regn_range[1] = sectn_map[s]/ROWS_PER_RGN - regn_range[0];
		return 0;
	}
	return -1;
}

#define sotp_hw_writel_or(__DEV__,__OFST__,__data)		\
	sotp_hw_writel(__DEV__,__OFST__,(sotp_hw_readl(__DEV__,__OFST__)|__data))


#define sotp_hw_writel_and(__DEV__,__OFST__,__data)		\
	sotp_hw_writel(__DEV__,__OFST__,(sotp_hw_readl(__DEV__,__OFST__)&__data))

__weak u32 sotp_hw_readl(void* dev, u32 offs)
{
	otp_hw_cmn_t* hw_dev = (otp_hw_cmn_t*)dev; 
	return readl(hw_dev->mm+offs);
}

__weak void sotp_hw_writel(void* dev, u32 offs, u32 data)
{
	otp_hw_cmn_t* hw_dev = (otp_hw_cmn_t*)dev; 
	writel(data, hw_dev->mm + offs);
}

/***************************************************************************
* Prototypes / Globals
***************************************************************************/

/***************************************************************************
* SOTP functions
***************************************************************************/

/***************************************************************************
// Function Name: getCrc32
// Description  : caculate the CRC 32 of the given data.
// Parameters   : pdata - array of data.
//                size - number of input data bytes.
//                crc -  either 0 or previous return value.
// Returns      : crc.
****************************************************************************/
static u32 getCrc32(u8 *pdata, u32 size, u32 crc)
{
     	if (crc == 0) {
		crc = 0xffffffff;
     	}
	/* 
 	*  no complement crc32 is used for an SOTP controller   
 	* 
 	* */
	return  crc32_no_comp(crc, pdata, size);
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
static SotpRowStatus sotpPollForFDone(otp_hw_cmn_t *dev)
{
   SotpRowStatus rval = SOTP_S_ROW_SUCCESS;

   u32 reg_rd_data;
   u32 cntr = 0;
   
   reg_rd_data = sotp_hw_readl(dev,SOTP_SOTP_OUT_0_OFFSET);
   while (( ! (reg_rd_data & SOTP_SOTP_OUT_0_SOTP_OTP_READY)) && (cntr < SOTP_MAX_CNTR))
   {
      cntr++;
      mdelay(1);
      reg_rd_data = sotp_hw_readl(dev,SOTP_SOTP_OUT_0_OFFSET);
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
static SotpRowStatus sotpSetStartAndPollForCmdDone(otp_hw_cmn_t *dev)
{
   
   SotpRowStatus rval = SOTP_S_ROW_SUCCESS;

   u32 reg_rd_data;
   u32 cntr = 0;
   /* Set the start bit */
   sotp_hw_writel_or(dev, SOTP_OTP_CTRL_0_OFFSET, SOTP_OTP_CTRL_0_START);

   mdelay(1);

   /* Wait for the FSM to say it is done */
   reg_rd_data = sotp_hw_readl(dev, SOTP_OTP_STATUS_1_OFFSET);
   while (( ! (reg_rd_data & SOTP_OTP_STATUS_1_CMD_DONE)) && (cntr < SOTP_MAX_CNTR)) {
      cntr++;
      mdelay(1);
      reg_rd_data = sotp_hw_readl(dev, SOTP_OTP_STATUS_1_OFFSET);
   }
   if (cntr != SOTP_MAX_CNTR)
   {
      /* Clear the start bit */
      sotp_hw_writel_and(dev, SOTP_OTP_CTRL_0_OFFSET, (~SOTP_OTP_CTRL_0_START));
   }
   else
   {
      rval = SOTP_E_ROW_TIMEOUT;
      SOTP_ERR_PRINT("\nTimeout waiting for CMD_DONE. 0x%x timeout after %d ms\n",reg_rd_data, cntr);
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
static SotpRowStatus sotpPrepForFusing(otp_hw_cmn_t *dev)
{
   SotpRowStatus rval = SOTP_S_ROW_SUCCESS;
   int i;
   u32 authVal[4] = {0xf,0x4,0x8,0xd};

   /* Poll for FDONE bit before doing any access */
   if ((rval = sotpPollForFDone(dev)) == SOTP_E_ROW_TIMEOUT) {
	_ETR(rval);
      return rval;
    }
   /* turn on the cpu mode */
   sotp_hw_writel_or(dev, SOTP_OTP_PROG_CTRL_OFFSET , SOTP_OTP_PROG_CTRL_OTP_CPU_MODE_EN);

   /* set up the OTP_CMD for OTP_ProgEnable */
   sotp_hw_writel(dev, SOTP_OTP_CTRL_0_OFFSET , SOTP_OTP_CTRL_0_OTP_CMD_OTP_PROG_ENABLE);

   /* Write F, 4, 8, D in sequence to BITSEL */
   for (i = 0; i < sizeof(authVal)/sizeof(u32); i++) {
      sotp_hw_writel(dev, SOTP_OTP_WDATA_0_OFFSET , authVal[i]);
      /* Start the state machine */
      rval = sotpSetStartAndPollForCmdDone(dev);
	if (rval != SOTP_S_ROW_SUCCESS) {
	        _ETR(rval);
		goto err;
	}
         /* Clear the CMD_DONE bit */
	sotp_hw_writel(dev, SOTP_OTP_STATUS_1_OFFSET , SOTP_OTP_STATUS_1_CMD_DONE);

   }

   /* By default, SOTP block state is "unprogrammed" which prevents writing to      */
   /* rows 16-111. Set the state to ManuProg and SwNonAbDevice to allow programming */
   sotp_hw_writel_or(dev, SOTP_CHIP_CNTRL_OFFSET, (SOTP_CHIP_CTRL_SW_MANU_PROG | SOTP_CHIP_CTRL_SW_NON_AB_DEVICE | \
                    SOTP_CHIP_CNTRL_SW_OVERRIDE_CHIP_STATES));

   /* Poll for FDONE bit before doing any further access */
   rval = sotpPollForFDone(dev);
err:
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
static SotpRowStatus sotpFinishFusing(otp_hw_cmn_t *dev)
{
   
   SotpRowStatus rval = SOTP_S_ROW_SUCCESS;


   /* set up the OTP_CMD for PROG  */
#ifndef OTP_DRY_RUN
   sotp_hw_writel(dev, SOTP_OTP_CTRL_0_OFFSET , SOTP_OTP_CTRL_0_OTP_CMD_PROG);
   /* Start the state machine */
   if ((rval = sotpSetStartAndPollForCmdDone(dev)) == SOTP_S_ROW_SUCCESS)
   {
      /* Clear the CMD_DONE bit */
      sotp_hw_writel(dev, SOTP_OTP_STATUS_1_OFFSET , SOTP_OTP_STATUS_1_CMD_DONE);

      /* turn off cpu mode */
      sotp_hw_writel_and(dev, SOTP_OTP_PROG_CTRL_OFFSET , (~SOTP_OTP_PROG_CTRL_OTP_CPU_MODE_EN));
   }
#else
      /* Clear the CMD_DONE bit */
      sotp_hw_writel(dev, SOTP_OTP_STATUS_1_OFFSET , SOTP_OTP_STATUS_1_CMD_DONE);

      /* turn off cpu mode */
      sotp_hw_writel_and(dev, SOTP_OTP_PROG_CTRL_OFFSET , (~SOTP_OTP_PROG_CTRL_OTP_CPU_MODE_EN));
#endif
   return rval;
}

/*  *********************************************************************
    *  static SotpRowStatus sotpReadNonEccRow(u32 row, u64 *pData)
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
static SotpRowStatus sotpReadNonEccRow(otp_hw_cmn_t *dev,u32 row, u64 *pDst)
{
   
   SotpRowStatus rval = SOTP_S_ROW_SUCCESS;


   /* Poll for FDONE bit before doing any access */
   if ((rval = sotpPollForFDone(dev)) == SOTP_E_ROW_TIMEOUT)
      return rval;

   /* Clear ECC detect, ecc correct as well as command done bits */
   sotp_hw_writel(dev, SOTP_OTP_STATUS_1_OFFSET , (SOTP_OTP_STATUS_1_CMD_DONE | SOTP_OTP_STATUS_1_ECC_COR | SOTP_OTP_STATUS_1_ECC_DET));

   /* zero out the regs_ecc_en bits, then turn on cpu mode, and set regs_ecc_en to disabled */
   sotp_hw_writel_and(dev, SOTP_OTP_PROG_CTRL_OFFSET , (~SOTP_OTP_PROG_CTRL_REGS_ECC_EN_MASK));
   sotp_hw_writel_or(dev, SOTP_OTP_PROG_CTRL_OFFSET,  ( SOTP_OTP_PROG_CTRL_OTP_CPU_MODE_EN | SOTP_OTP_PROG_CTRL_OTP_DISABLE_ECC | \
                              SOTP_OTP_PROG_CTRL_REGS_ECC_DIS ));

   /* Set OTP addr with the row to be read */
   sotp_hw_writel(dev, SOTP_OTP_ADDR_OFFSET , (row << SOTP_OTP_ADDR_OTP_ADDR_SHIFT));

   /* set up the OTP_CMD for OTP_READ */
   sotp_hw_writel(dev, SOTP_OTP_CTRL_0_OFFSET , SOTP_OTP_CTRL_0_OTP_CMD_OTP_READ);

   /* Start the state machine */
   if ((rval = sotpSetStartAndPollForCmdDone(dev)) == SOTP_S_ROW_SUCCESS)
   {
      /* Data in row is valid. Pass it to return buffer */
      *pDst = (((u64)sotp_hw_readl(dev, SOTP_OTP_RDATA_1_OFFSET) << 32) | (sotp_hw_readl(dev,SOTP_OTP_RDATA_0_OFFSET)));

      /* Clear the CMD_DONE bit */
      sotp_hw_writel(dev, SOTP_OTP_STATUS_1_OFFSET , SOTP_OTP_STATUS_1_CMD_DONE);

      /* turn off cpu mode */
      sotp_hw_writel_and(dev, SOTP_OTP_PROG_CTRL_OFFSET , (~SOTP_OTP_PROG_CTRL_OTP_CPU_MODE_EN));
   }

   return rval;
}

/*  *********************************************************************
    *  static SotpRowStatus sotpFuseNonEccRow(u32 row, u64 src)
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
static SotpRowStatus sotpFuseNonEccRow(otp_hw_cmn_t *dev, u32 row, u64 src)
{
   
   SotpRowStatus rval = SOTP_S_ROW_SUCCESS;


   /* Prepare the SOTP FSM for fusing a row of data */
   if ((rval = sotpPrepForFusing(dev)) != SOTP_S_ROW_SUCCESS)
      return rval;

   /* Disable ECC to be generated and fused by hardware */
   sotp_hw_writel_and(dev, SOTP_OTP_PROG_CTRL_OFFSET , (~SOTP_OTP_PROG_CTRL_OTP_ECC_WREN));

   /* Set OTP addr with the row to be fused */
   sotp_hw_writel(dev, SOTP_OTP_ADDR_OFFSET , (row << SOTP_OTP_ADDR_OTP_ADDR_SHIFT));
#ifndef OTP_DRY_RUN  
   /* Set up the registers with the data to be written */
   sotp_hw_writel(dev, SOTP_OTP_WDATA_0_OFFSET , (u32)(src & 0xffffffff)); /* lower 32 bit data word */
   sotp_hw_writel(dev, SOTP_OTP_WDATA_1_OFFSET , (u32)(src >> 32)); /* upper 32 bit data word */
#endif
   /* ... and fuse */
   rval = sotpFinishFusing(dev);

   return rval;
}

/*  *********************************************************************
    *  static SotpRowStatus sotpIsRegionFuseLocked(u32 region)
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
static SotpRowStatus sotpIsRegionFuseLocked(otp_hw_cmn_t *dev,u32 region)
{
   SotpRowStatus rval = SOTP_S_ROW_SUCCESS;
   u64 rowData;

   /* The locking of all the regions require two rows of OTP. Each row has 41 */
   /* bits. There are no ecc nor fail bits in these two rows. Hence, regions  */
   /* 0 thru 19 lock bits are in SOTP row 8. Region 20 has a lock bit in each */
   /* row. Regions 21 thru 27 are all in row 9. Retrieve row 8 if required. If*/
   /* If either one of the two approp. bits are set, the region is fuse locked*/
   if (region < SOTP_NUM_REG_IN_FUSELOCK_ROW)
   {
      if ((rval = sotpReadNonEccRow(dev, SOTP_FIRST_FUSELOCK_ROW, &rowData)) != SOTP_S_ROW_SUCCESS)
         return rval;
      else
      {
         if (rowData & ( (u64)0x3 << (region * 2) )) 
            return SOTP_E_ROW_FUSE_LOCK;
      }
   }

   if (region >= (SOTP_NUM_REG_IN_FUSELOCK_ROW - 1))
   {
      if ((rval = sotpReadNonEccRow(dev, (SOTP_FIRST_FUSELOCK_ROW + 1), &rowData)) != SOTP_S_ROW_SUCCESS)
         return rval;
      else
      {
         if ((region == (SOTP_NUM_REG_IN_FUSELOCK_ROW - 1)) && (rowData & 0x1))
            return SOTP_E_ROW_FUSE_LOCK;
         else 
         { 
            if (rowData & ( (u64)0x3 << (((region-SOTP_NUM_REG_IN_FUSELOCK_ROW)*2)+1) )) 
               return SOTP_E_ROW_FUSE_LOCK;
         } 
      }
   }

   return rval;
}



/*  *********************************************************************
    *  static SotpRowStatus sotpFuseLockRegion(u32 region)
    *  
    *  Description: PERMANENTLY WRITE LOCK regions
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
static SotpRowStatus sotpFuseLockRegion(otp_hw_cmn_t *dev,u32 region)
{
   SotpRowStatus rval = SOTP_S_ROW_SUCCESS;
   u64 bitsToFuse;

   /* The locking of all the regions require two rows of OTP. Each row has 41 */
   /* bits. There are no ecc nor fail bits in these two rows. Hence, regions  */
   /* 0 thru 19 lock bits are in SOTP row 8. Region 20 has a lock bit in each */
   /* row. Regions 21 thru 27 are all in row 9. Retrieve row 8 if required. If*/
   /* If either one of the two approp. bits are set, the region is fuse locked*/
   if (region < SOTP_NUM_REG_IN_FUSELOCK_ROW)
   {
      // Figure out the bits to fuse (truncate to 41 bits)
      bitsToFuse = ((u64)0x3 << (region * 2)) & 0x1ffffffffff;

      if ((rval = sotpFuseNonEccRow(dev, SOTP_FIRST_FUSELOCK_ROW, bitsToFuse)) != SOTP_S_ROW_SUCCESS)
         return rval;
   }

   if (region >= (SOTP_NUM_REG_IN_FUSELOCK_ROW - 1))
   {
      // Figure out the bits to fuse (truncate to 41 bits)
      if (region == (SOTP_NUM_REG_IN_FUSELOCK_ROW - 1)) 
         bitsToFuse = 0x1;
      else
         bitsToFuse =  (u64)0x3 << (((region-SOTP_NUM_REG_IN_FUSELOCK_ROW)*2)+1);

      rval = sotpFuseNonEccRow(dev, (SOTP_FIRST_FUSELOCK_ROW + 1), bitsToFuse);
   }

   return rval;
}

/*  *********************************************************************
    *  static SotpRowStatus sotpWriteEccRow(u32 row, u32 src)
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
static SotpRowStatus sotpWriteEccRow(otp_hw_cmn_t *dev, u32 row, u32 src, u32 raw_write)
{
   
   SotpRowStatus rval = SOTP_S_ROW_SUCCESS;

   SOTP_DBG("%s: row:%d data:0x%08x\n", __FUNCTION__, row, src);

   if( row < SOTP_FIRST_ECC_CONFIG_ROW || row > SOTP_NUM_ROWS-1 )
   {
      SOTP_ERR_PRINT("%s: ECC Row %d out of bound! Min:%d Max:%d\n", __FUNCTION__, row, SOTP_FIRST_ECC_CONFIG_ROW, SOTP_NUM_ROWS-1);
      return SOTP_E_ROW_ERROR;
   }

   /* Check to see that the row in question is not fuse locked */
   if ((rval = sotpIsRegionFuseLocked(dev, row/SOTP_ROWS_IN_REGION)) == SOTP_E_ROW_FUSE_LOCK)
   {
      SOTP_ERR_PRINT("%s: Row %d is fuse locked. Cannot write row data.\n", __FUNCTION__, row);
      return rval;
   }

   /* Prepare the SOTP FSM for fusing a row of data */
   if ((rval = sotpPrepForFusing(dev)) != SOTP_S_ROW_SUCCESS) {
	_ETR(rval);
      return rval;
   }
   if( raw_write )
   {
      /* Disable ECC computation and fusing by hardware */
      sotp_hw_writel_or(dev, SOTP_OTP_PROG_CTRL_OFFSET , SOTP_OTP_PROG_CTRL_OTP_DISABLE_ECC);
      sotp_hw_writel_and(dev, SOTP_OTP_PROG_CTRL_OFFSET , (~SOTP_OTP_PROG_CTRL_OTP_ECC_WREN));
   }
   else
   {
      /* Enable ECC to be generated and fused by hardware */
      sotp_hw_writel_and(dev, SOTP_OTP_PROG_CTRL_OFFSET , (~SOTP_OTP_PROG_CTRL_OTP_DISABLE_ECC));
      sotp_hw_writel_or(dev, SOTP_OTP_PROG_CTRL_OFFSET , SOTP_OTP_PROG_CTRL_OTP_ECC_WREN);
   }
#ifndef  OTP_DRY_RUN
   /* Set OTP addr with the row to be fused */
   sotp_hw_writel(dev, SOTP_OTP_ADDR_OFFSET , (row << SOTP_OTP_ADDR_OTP_ADDR_SHIFT));

   /* Set up the registers with the data to be written */
   sotp_hw_writel(dev, SOTP_OTP_WDATA_0_OFFSET , src); /* 32 bit data word */
   sotp_hw_writel(dev, SOTP_OTP_WDATA_1_OFFSET , 0);   /* ensure the two fail bits are not fused */
#endif
   /* ... and fuse */
   rval = sotpFinishFusing(dev);

   return rval;
}

/*  *********************************************************************
    *  static SotpRowStatus sotpFuseSecKeyRowFailBits(u32 row)
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
static SotpRowStatus sotpFuseSecKeyRowFailBits(otp_hw_cmn_t *dev, u32 row)
{
   
   SotpRowStatus rval = SOTP_S_ROW_SUCCESS;


   /* Prepare the SOTP FSM for fusing a row of data */
   if ((rval = sotpPrepForFusing(dev)) != SOTP_S_ROW_SUCCESS)
      return rval;

   /* Enable ECC to be generated and fused by hardware */
   sotp_hw_writel_or(dev, SOTP_OTP_PROG_CTRL_OFFSET , SOTP_OTP_PROG_CTRL_OTP_ECC_WREN);
#ifndef OTP_DRY_RUN
   /* Set OTP addr with the row to be fused */
   sotp_hw_writel(dev, SOTP_OTP_ADDR_OFFSET , (row << SOTP_OTP_ADDR_OTP_ADDR_SHIFT));

   /* Set up the registers with the data to be written */
   sotp_hw_writel(dev, SOTP_OTP_WDATA_0_OFFSET , 0);                          /* 32 bit data word  ... no change    */
   sotp_hw_writel(dev, SOTP_OTP_WDATA_1_OFFSET , SOTP_OTP_WDATA_1_FAIL_MASK); /* ensure the two fail bits are fused */
#endif
   /* ... and fuse */
   rval = sotpFinishFusing(dev);

   return rval;
}

/*  *********************************************************************
    *  static SotpRowStatus sotpRegionReadLockStatus(u32 region)
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
static SotpRowStatus sotpRegionReadLockStatus(otp_hw_cmn_t *dev, u32 region)
{
   
   SotpRowStatus rval = SOTP_S_ROW_SUCCESS;

   /* Check to see if that region is locked */
   if ((sotp_hw_readl(dev, SOTP_OTP_RD_LOCK_OFFSET) & ( 0x1 << region )) )
      rval = SOTP_E_ROW_READ_LOCK;

   return rval;
}

/*  *********************************************************************
    *  static SotpRowStatus sotpReadEccRow(u32 row, u32 *pDst)
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
static SotpRowStatus sotpReadEccRow(otp_hw_cmn_t *dev, u32 row, u32 *pDst, u32 raw_read)
{
   
   SotpRowStatus rval;


   SOTP_DBG("%s: row:%d bufp:%px\n", __FUNCTION__, row, pDst);

   if( row < SOTP_FIRST_ECC_CONFIG_ROW || row > SOTP_NUM_ROWS-1 )
   {
      SOTP_DBG("%s: ECC Row %d out of bound! Min:%d Max:%d\n", __FUNCTION__, row, SOTP_FIRST_ECC_CONFIG_ROW, SOTP_NUM_ROWS-1);
      return SOTP_E_ROW_ERROR;
   }

   /* Check to see that the row in question is not readlocked until the next POR */
   if ((rval = sotpRegionReadLockStatus(dev, row/SOTP_ROWS_IN_REGION)) == SOTP_E_ROW_READ_LOCK)
   {
      SOTP_DBG("%s: Row %d is locked from reading. Returned key data is invalid\n", __FUNCTION__, row);
      return SOTP_E_ROW_READ_LOCK;
   }

   /* Poll for FDONE bit before doing any access */
   if ((rval = sotpPollForFDone(dev)) == SOTP_E_ROW_TIMEOUT)
   {
      SOTP_ERR_PRINT("%s: Row %d read timed out. \n", __FUNCTION__, row);
      return SOTP_E_ROW_TIMEOUT;
   }

   /* Clear ECC detect, ecc correct as well as command done bits */
   sotp_hw_writel(dev, SOTP_OTP_STATUS_1_OFFSET , 
		(SOTP_OTP_STATUS_1_CMD_DONE | SOTP_OTP_STATUS_1_ECC_COR | SOTP_OTP_STATUS_1_ECC_DET));

   /* enable ecc checks (low), zero out the regs_ecc_en bits  ... then turn on cpu mode, and set regs_ecc_en to enabled */
   /* expect Bug?  When ecc=0 then programs 0x0, Reg says that it should be set to 4'b0101 : ECC check DISABLED */
   sotp_hw_writel_and(dev, SOTP_OTP_PROG_CTRL_OFFSET , (~( SOTP_OTP_PROG_CTRL_OTP_DISABLE_ECC | SOTP_OTP_PROG_CTRL_REGS_ECC_EN_MASK )));

   /* Disable ECC checks for raw reads */
   if( raw_read )
      sotp_hw_writel_or(dev, SOTP_OTP_PROG_CTRL_OFFSET , ( SOTP_OTP_PROG_CTRL_OTP_CPU_MODE_EN | SOTP_OTP_PROG_CTRL_OTP_DISABLE_ECC | SOTP_OTP_PROG_CTRL_REGS_ECC_DIS ));
   else
      sotp_hw_writel_or(dev, SOTP_OTP_PROG_CTRL_OFFSET , ( SOTP_OTP_PROG_CTRL_OTP_CPU_MODE_EN | SOTP_OTP_PROG_CTRL_REGS_ECC_EN ));

   /* Set OTP addr with the row to be read */
   sotp_hw_writel(dev, SOTP_OTP_ADDR_OFFSET , (row << SOTP_OTP_ADDR_OTP_ADDR_SHIFT));

   /* set up the OTP_CMD for OTP_READ */
   sotp_hw_writel(dev, SOTP_OTP_CTRL_0_OFFSET , SOTP_OTP_CTRL_0_OTP_CMD_OTP_READ);

   /* Start the state machine */
   if ((rval = sotpSetStartAndPollForCmdDone(dev)) == SOTP_S_ROW_SUCCESS)
   {
      if( !raw_read ) 
      {
         /* If the row is marked as failed, abort */
         if (sotp_hw_readl(dev, SOTP_OTP_RDATA_1_OFFSET) & SOTP_OTP_RDATA_1_FAIL_MASK )
         {
            SOTP_ERR_PRINT("Row %d is marked as failed . \n", row);
            return ( SOTP_E_ROW_FAIL_SET );
         }
    
         /* See if the data is bad and ecc correction was not possible */
         if (sotp_hw_readl(dev, SOTP_OTP_STATUS_1_OFFSET) & SOTP_OTP_STATUS_1_ECC_DET )
         {
            SOTP_ERR_PRINT("Row data failed ecc with 2 or more bits . \n");
               return ( SOTP_E_ROW_ECC_DET );
         }

         /* See if the data is good but an ecc correction took place */
         if (sotp_hw_readl(dev, SOTP_OTP_STATUS_1_OFFSET) & SOTP_OTP_STATUS_1_ECC_COR )
         {
            SOTP_ERR_PRINT("Row data valid, but ecc corrected 1 bit. \n");
            rval = SOTP_S_ROW_ECC_COR;
         }
      }

      /* Data in row is valid. Pass it to return buffer */
      *pDst = sotp_hw_readl(dev,SOTP_OTP_RDATA_0_OFFSET);

      /* Clear the CMD_DONE bit */
      sotp_hw_writel(dev, SOTP_OTP_STATUS_1_OFFSET , SOTP_OTP_STATUS_1_CMD_DONE);

      /* turn off cpu mode */
      sotp_hw_writel_and(dev, SOTP_OTP_PROG_CTRL_OFFSET , (~SOTP_OTP_PROG_CTRL_OTP_CPU_MODE_EN));
   }

   return rval;
}

/*  *********************************************************************
    *  extern SotpKeyStatus sotpReadKeyslotData(u32 section, u32 *pDst, u32 len)
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
static SotpKeyStatus sotpReadKeyslotData(otp_hw_cmn_t *dev, u32 section, u32 *pDst, u32 len)
{
   u32 startRow, row, printRow, readCrc;
   u32 calcCrc = 0;

   u32 i = 0;
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

      if (i < len) {
         rowRval = sotpReadEccRow(dev, row, &pDst[i], 0); /* currently reading the key */
		_TR(pDst[i]);
}

      if (i == len)
         rowRval = sotpReadEccRow(dev, row, &readCrc, 0); /* currently reading the crc */

      switch(rowRval)
      {
         /* If success, just keep going */
         case SOTP_S_ROW_SUCCESS: 
         case SOTP_S_ROW_ECC_COR:
            i++;
            if (i > len)
            {
               /* done reading sotp. Perform crc validation */
               calcCrc = getCrc32((u8 *)pDst, (len*4), calcCrc);
_TR(calcCrc);	
_TR(readCrc);	
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
            SOTP_DBG("\nRow %d is locked from reading. Returned key data is invalid\n", row);
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
    *  static SotpKeyStatus sotpLockSecKeyFuse(u32 section)
    *  
    *  Input parameters: 
    *      section - There are 7 locations called sections that can hold 
    *                up to 256 bits of key information. (7 =< section =< 13).
    *                This is the location that is to be PERMANENTLY locked down from
    *                future fusing. This is a permanent lockout even after POR.
    *      
    *  Return value:
    *      SOTP_S_KEY_SUCCESS     - The function has completed successfully
    *      SOTP_E_KEY_BADPARAM    - The function recieved a bad input parameter
    *      SOTP_E_KEY_ERROR       - The function experienced unexpected register 
    *                               ret value from SOTP block 
    *
    ********************************************************************* */
static SotpKeyStatus sotpLockSecKeyFuse(otp_hw_cmn_t *dev, u32 section)
{
   SotpKeyStatus rval = SOTP_S_KEY_SUCCESS;
   u32 region, startRegion;

   /* perform checks of input params */
   if ((section < SOTP_MIN_KEYSLOT) || (section > SOTP_MAX_KEYSLOT))
      return SOTP_E_KEY_BADPARAM;

   /* Figure out the start region, and then for each of the three regions */
   /* in the section, do the for loop */
   startRegion = SOTP_FIRST_KEYSLOT_REGION + ((section - SOTP_MIN_KEYSLOT) * SOTP_REGIONS_IN_KEYSLOT);
   for (region = startRegion; region < (startRegion+3); region++)
   {
      if (sotpFuseLockRegion(dev, region) != SOTP_S_ROW_SUCCESS)
         return SOTP_E_KEY_ERROR;
   }
   
   return rval;
}

/*  *********************************************************************
    *  static SotpRowStatus sotpLockRegion(u32 region)
    *  
    *  Description: TEMPORARILY READ/WRITE LOCK regions. Lock gets reset on POR
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
static SotpKeyStatus sotpLockRegion(otp_hw_cmn_t *dev, u32 region_mask) 
{
   if( region_mask ) 
   {
      /* enable ecc checks (low), zero out the regs_ecc_en bits  ... then turn on cpu mode, and set regs_ecc_en to enabled */
      /* expect Bug?  When ecc=0 then programs 0x0, Reg says that it should be set to 4'b0101 : ECC check DISABLED */
      sotp_hw_writel_and(dev, SOTP_OTP_PROG_CTRL_OFFSET , (~SOTP_OTP_PROG_CTRL_REGS_ECC_EN_MASK));
      sotp_hw_writel_or(dev, SOTP_OTP_PROG_CTRL_OFFSET , ( SOTP_OTP_PROG_CTRL_OTP_CPU_MODE_EN | SOTP_OTP_PROG_CTRL_REGS_ECC_DIS ));

      /* wait a little */
      mdelay(1);

      /* The read/write lock register is a write-once register on a bit-for-bit bassis     */
      /* so you don't have to worry about preserving bits that are already set to 1. */
      sotp_hw_writel_or(dev, SOTP_OTP_RD_LOCK_OFFSET , region_mask);
      sotp_hw_writel_or(dev, SOTP_OTP_WR_LOCK_OFFSET , region_mask);

      /* wait a little */
      mdelay(1);

      /* turn off cpu mode */
      sotp_hw_writel_and(dev, SOTP_OTP_PROG_CTRL_OFFSET , (~SOTP_OTP_PROG_CTRL_OTP_CPU_MODE_EN));
   }

   return SOTP_S_KEY_SUCCESS;
}

static SotpKeyStatus sotpReadLockRegion(otp_hw_cmn_t *dev, u32 region)
{
   return sotpLockRegion(dev, (1 << region));
}


/*  *********************************************************************
    *  extern SotpKeyStatus readLockKeyslot(u32 section)
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
static SotpKeyStatus readLockKeyslot(otp_hw_cmn_t *dev, u32 section)
{
   u32 startRegion;

   /* perform checks of input params */
   if ((section < SOTP_MIN_KEYSLOT) || (section > SOTP_MAX_KEYSLOT))
      return SOTP_E_KEY_BADPARAM;

   /* Determine the starting region to lock */
   startRegion = SOTP_FIRST_KEYSLOT_REGION + 
      ((section - SOTP_MIN_KEYSLOT) * SOTP_REGIONS_IN_KEYSLOT);

   return (sotpLockRegion(dev, (SOTP_REGIONS_MASK_IN_KEYSLOT << startRegion))); 
}

static SotpKeyStatus sotpReadLockNonEmptyRegions(otp_hw_cmn_t *dev)
{
   int i,j;
   u32 value;
   u32 startRegion = SOTP_FIRST_FUSELOCK_ROW/SOTP_ROWS_IN_REGION + SOTP_REGIONS_IN_LOCK;
   u32 regionMask = 0;
   SotpRowStatus rval = SOTP_S_ROW_SUCCESS;

   /* readlock all regions before start region */
   if( startRegion ) 
	   regionMask = ((1<<startRegion) - 1);

   for( i=startRegion; i<SOTP_NUM_ROWS/SOTP_ROWS_IN_REGION; i++) 
   {
      for( j=0; j<SOTP_ROWS_IN_REGION; j++) 
      {
         /* Do a raw read */
         value = 0;
         if( j+(i*SOTP_ROWS_IN_REGION) < SOTP_FIRST_ECC_CONFIG_ROW )
            rval = sotpReadNonEccRow( dev, j+(i*SOTP_ROWS_IN_REGION), &value);
         else
            rval = sotpReadEccRow(dev, j+(i*SOTP_ROWS_IN_REGION), &value, 1);

         if( value )
         {
            regionMask |= 1 << i;
            break;
         }
      }
   }
   return (sotpLockRegion(dev, regionMask));
}

static SotpKeyStatus sotpKeySlotReadLockStatus(otp_hw_cmn_t *dev, u32 section)
{
   u32 startRegion;
   int i;

   /* perform checks of input params */
   if ((section < SOTP_MIN_KEYSLOT) || (section > SOTP_MAX_KEYSLOT))
      return SOTP_E_KEY_BADPARAM;

   /* enable ecc checks (low), zero out the regs_ecc_en bits  ... then turn on cpu mode, and set regs_ecc_en to enabled */
   /* expect Bug?  When ecc=0 then programs 0x0, Reg says that it should be set to 4'b0101 : ECC check DISABLED */
   sotp_hw_writel_and(dev, SOTP_OTP_PROG_CTRL_OFFSET , (~SOTP_OTP_PROG_CTRL_REGS_ECC_EN_MASK));
   sotp_hw_writel_or(dev, SOTP_OTP_PROG_CTRL_OFFSET , ( SOTP_OTP_PROG_CTRL_OTP_CPU_MODE_EN | SOTP_OTP_PROG_CTRL_REGS_ECC_DIS ));

   /* Determine the starting region to lock */
   startRegion = SOTP_FIRST_KEYSLOT_REGION + ((section - SOTP_MIN_KEYSLOT) * SOTP_REGIONS_IN_KEYSLOT);

   for( i=0; i<SOTP_REGIONS_IN_KEYSLOT; i++ )
   {

       /* Check to see if that region is locked */
       if (sotp_hw_readl(dev, SOTP_OTP_RD_LOCK_OFFSET) & ( 0x1 << startRegion ) )
          return SOTP_E_ROW_READ_LOCK;

       /* Go to next region in keyslot */
       startRegion++;
   }

   /* turn off cpu mode */
   sotp_hw_writel_and(dev, SOTP_OTP_PROG_CTRL_OFFSET , (~SOTP_OTP_PROG_CTRL_OTP_CPU_MODE_EN));

   return 0;
}

/*  *********************************************************************
    *  extern SotpKeyStatus sotpWriteKeyslotData(u32 section, u32 *pSrc, u32 len)
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
static SotpKeyStatus sotpWriteKeyslotData(otp_hw_cmn_t *dev,
 					u32 section, 
					u32 *pSrc, 
					u32 len)
{
   u32 startRow, row, calcCrc, startRegion, fuse_lock, read_lock;
   u32 rowData = 0;
   u32 i = 0;
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
   calcCrc = getCrc32((u8 *)pSrc, (len*4), 0 );

   /* Determine the starting row to fuse */
   startRow = ((section - SOTP_MIN_KEYSLOT) * SOTP_ROWS_IN_KEYSLOT) + SOTP_FIRST_KEYSLOT_ROW;

   /* Determine starting region */
   startRegion = startRow / SOTP_ROWS_IN_REGION;

   /* Make sure that the regions we are writing to are not read-locked or fuse-locked */
   for( i = startRegion; i < startRegion + SOTP_REGIONS_IN_KEYSLOT; i++ )
   {
      fuse_lock = sotpIsRegionFuseLocked(dev, i);
      read_lock = sotpRegionReadLockStatus(dev, i);
      if( fuse_lock || read_lock )
      {
         SOTP_ERR_PRINT("%s: section:%d region:%d %s_locked! Abort keyslot programming!\n", __FUNCTION__, section, i, (fuse_lock?"fuse":"read"));
         return rval;
      }
   }

   /* Make sure that entire keyslot is empty before attempting to program */
   for ( row = startRow ; row < (startRow + SOTP_ROWS_IN_KEYSLOT); row++) {
      /* TODO: Add counter to allow certain amount of failiures ( 3 redundant ) */
      rowRval = sotpReadEccRow(dev, row, &rowData, 0);
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
   for ( row = startRow, i = 0 ; row < (startRow + SOTP_ROWS_IN_KEYSLOT); row++)
   {
	_TR(pSrc[i]);
      if (i < len)
         rowRval = sotpWriteEccRow(dev, row, pSrc[i], 0); /* currently writing the key */

      if (i == len) {
	 _TR(calcCrc);
         rowRval = sotpWriteEccRow(dev, row, calcCrc, 0); /* write the crc */
      }
	_TR(pSrc[i]);
      /* See if it was successful */
      switch(rowRval)
      {

         case SOTP_S_ROW_SUCCESS:
         {
            /* Check the integrity of the row just fused */
            rowRval = sotpReadEccRow(dev, row, &rowData, 0);

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
                     if (calcCrc == rowData) {
#ifdef DEBUG_SOTP
			return SOTP_S_ROW_SUCCESS;
#else
                        return(sotpLockSecKeyFuse(dev, section)); /* key and crc fused. Lock section from further writes */
#endif 
                     }
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
                  if ((rowRval = sotpFuseSecKeyRowFailBits(dev, row)) != SOTP_S_ROW_SUCCESS)
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
                  SOTP_DBG("\nRow %d is locked from reading. Cannot verify. Section %d may be unusable\n", row, section);
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

__weak otp_hw_cmn_err_t sotp_hw_read(otp_hw_cmn_t *dev, 
				u32 addr,
				otp_hw_cmn_row_conf_t *conf, 
				u32* data,
				u32 size)
{
	otp_hw_cmn_err_t rc = OTP_HW_CMN_ERR_FAIL;
	int res = 0;

	if (conf && conf->addr_type == OTP_HW_CMN_ROW_ADDR_ROW) {
    		res = sotpReadEccRow(dev, addr, (u32*)data, 
			conf->op_type == OTP_HW_CMN_CTL_OTPCMD_ECC? 0:1);
    		if (res != SOTP_S_KEY_SUCCESS && res != SOTP_S_ROW_ECC_COR ) {
			goto err;
		}
	} else {
		u32 sect = 0;
		if (row2sect(addr, &sect)) {
			rc = OTP_HW_CMN_ERR_UNSP;
			goto err;
		}	
    		res = sotpReadKeyslotData(dev, sect, (u32*)data, size/sizeof(u32));
    		if (res != SOTP_S_KEY_SUCCESS && res != SOTP_S_ROW_ECC_COR ) {
			goto err;
		}
	}
	rc = OTP_HW_CMN_OK;
err:
	return rc;
}

static otp_hw_cmn_err_t dev_status(otp_hw_cmn_t *dev,
			u32 sect,
			otp_hw_cmn_status_t status,
			u32* res)
{
	otp_hw_cmn_err_t rc = OTP_HW_CMN_ERR_FAIL;
        u32 perm_mask;
	if ((status&(OTP_HW_CMN_STATUS_NS_LOCKED|OTP_HW_CMN_STATUS_S_LOCKED))) {
		rc = OTP_HW_CMN_ERR_UNSP;
		goto err;
	}
        perm_mask  = sotp_hw_readl(dev, SOTP_PERM_OFFSET); 
	if ( status & OTP_HW_CMN_STATUS_ROW_DATA_VALID )  {
		u32 tmp[8];
		if (sotpReadKeyslotData(dev, sect, tmp, 8) != SOTP_S_KEY_SUCCESS || 
			sotpKeySlotReadLockStatus(dev, sect) == SOTP_E_ROW_READ_LOCK) {
			status &= ~OTP_HW_CMN_STATUS_ROW_DATA_VALID;
		_TR(status);	
		}
	}
	if (status & OTP_HW_CMN_STATUS_ROW_W_LOCKED) {
		u32 regn_range[2];
		if ( sect2regn(sect, regn_range) ) {
			rc = OTP_HW_CMN_ERR_UNSP;
			goto err;
		}
    		if(sotpIsRegionFuseLocked(dev, sect) != SOTP_E_ROW_FUSE_LOCK) {
			status &= ~OTP_HW_CMN_STATUS_ROW_W_LOCKED;
		}
	}
	if ( status & OTP_HW_CMN_STATUS_ROW_RD_LOCKED )  {
    		if (sotpKeySlotReadLockStatus(dev, sect) != SOTP_E_ROW_READ_LOCK) {
			status &= ~OTP_HW_CMN_STATUS_ROW_RD_LOCKED;
		}
	}
	/* permission register access status*/
	if ( !(perm_mask & SOTP_PERM_NSEC_R) ) {
   		status &= ~OTP_HW_CMN_STATUS_NSRD_PAC_LOCKED;
	}
	if ( !(perm_mask & SOTP_PERM_SEC_R) ) {
   		status &= ~OTP_HW_CMN_STATUS_SRD_PAC_LOCKED;
	}
	if ( !(perm_mask & SOTP_PERM_NSEC_W) ) {
		status &= ~OTP_HW_CMN_STATUS_NSW_PAC_LOCKED;
	}
	if ( !( perm_mask & SOTP_PERM_SEC_W) ) {
		status &= ~OTP_HW_CMN_STATUS_SW_PAC_LOCKED;
	}
	if ( !(perm_mask & (SOTP_PERM_BLK_SEC_R<<SOTP_PERM_BLK_SHIFT) )) {
		status &= ~OTP_HW_CMN_STATUS_SRD_LOCKED;
	}
	if ( !(perm_mask & (SOTP_PERM_BLK_NSEC_R<<SOTP_PERM_BLK_SHIFT)	)) {
   		status &=  ~OTP_HW_CMN_STATUS_NSRD_LOCKED;
	}
	if ( !(perm_mask & (SOTP_PERM_BLK_SEC_W<<SOTP_PERM_BLK_SHIFT) )) {
   		status &= ~OTP_HW_CMN_STATUS_SW_LOCKED;
	}

	if ( !(perm_mask & (SOTP_PERM_BLK_NSEC_W<<SOTP_PERM_BLK_SHIFT) )) {
		status &= ~OTP_HW_CMN_STATUS_NSW_LOCKED;
	}
	*res = status;
	rc = OTP_HW_CMN_OK;
	
err:
	return rc;
}

static otp_hw_cmn_err_t  sotp_hw_lock(otp_hw_cmn_t *dev, 
				u32 sect,
				otp_hw_cmn_perm_t perm, 
				u32 clear_lock) 
{
	otp_hw_cmn_err_t rc = OTP_HW_CMN_ERR_UNSP;
	u32 perm_mask = 0;
	if (perm & OTP_HW_CMN_CTL_LOCK_ALL) {
		if (clear_lock) {
                        SOTP_DBG("%s:UNLOCKING ALL!\n", __FUNCTION__);
			sotp_hw_writel(dev, SOTP_PERM_OFFSET, SOTP_PERM_ENABLE_ALL);
		} else {
                        SOTP_DBG("%s:LOCKING ALL!\n", __FUNCTION__);
			int i;
			for (i = 0; i <= SOTP_MIN_KEYSLOT; i++) {
    				if (readLockKeyslot(dev, i) != SOTP_S_KEY_SUCCESS) { 
					rc = OTP_HW_CMN_ERR_FAIL;
					goto err;
				}
			}
			sotp_hw_writel(dev, SOTP_PERM_OFFSET, SOTP_PERM_DISABLE_ALL);
		}
		return OTP_HW_CMN_OK;
	}
	if (perm & OTP_HW_CMN_CTL_LOCK_S) {
		if (clear_lock) {
                        SOTP_DBG("%s:UNLOCKING SEC!\n", __FUNCTION__);
			sotp_hw_writel(dev, SOTP_PERM_OFFSET, SOTP_PERM_ALLOW_SECURE_ACCESS_ONLY);
		} else {
                        SOTP_DBG("%s:LOCKING SEC!\n", __FUNCTION__);
			sotp_hw_writel(dev, SOTP_PERM_OFFSET, SOTP_PERM_DISABLE_SECURE_ACCESS);
		}
		return OTP_HW_CMN_OK;
	}

	if (perm & OTP_HW_CMN_CTL_LOCK_NS || perm & OTP_HW_CMN_CTL_LOCK_NS_PROV) {
		if (clear_lock) {
                        SOTP_DBG("%s:UNLOCKING NONSEC BLK!\n", __FUNCTION__);
			sotp_hw_writel(dev, SOTP_PERM_OFFSET, SOTP_PERM_ALLOW_NONSEC_BLK_ACCESS);
		} else {
                        SOTP_DBG("%s:LOCKING NONSEC!\n", __FUNCTION__);
			sotp_hw_writel_and(dev, SOTP_PERM_OFFSET, 
                           ((~SOTP_PERM_ALLOW_NONSEC_ACCESS_ONLY)&0xff));
		}
		if( perm & OTP_HW_CMN_CTL_LOCK_NS_PROV ) {
			sotpReadLockNonEmptyRegions(dev);
		}
		return OTP_HW_CMN_OK;
	}
	if (perm & OTP_HW_CMN_CTL_LOCK_ROW_RD) {
                if (!clear_lock) {
    			if (readLockKeyslot(dev, sect) != SOTP_S_KEY_SUCCESS) { 
				rc = OTP_HW_CMN_ERR_FAIL;
				goto err;
			}
		}
	} 
	/* permission register access status*/
  	if (perm & OTP_HW_CMN_CTL_LOCK_PAC_NSRD) {
		perm_mask |= SOTP_PERM_NSEC_R;
	}
   	if (perm & OTP_HW_CMN_CTL_LOCK_PAC_SRD) {
		perm_mask |= SOTP_PERM_SEC_R;  
	}
	if (perm & OTP_HW_CMN_CTL_LOCK_PAC_NSW) {
		perm_mask |= SOTP_PERM_NSEC_W;
	}
	if (perm & OTP_HW_CMN_CTL_LOCK_PAC_SW) {
		perm_mask |= SOTP_PERM_SEC_W;
	}
	if (perm & OTP_HW_CMN_CTL_LOCK_SRD) {
		perm_mask |= (SOTP_PERM_BLK_SEC_R<<SOTP_PERM_BLK_SHIFT);
	}
   	if (perm & OTP_HW_CMN_CTL_LOCK_NSRD) {
		perm_mask |= (SOTP_PERM_BLK_NSEC_R<<SOTP_PERM_BLK_SHIFT);
	}
	if (perm & OTP_HW_CMN_CTL_LOCK_SW) {
		perm_mask |= (SOTP_PERM_BLK_SEC_W<<SOTP_PERM_BLK_SHIFT);
	}

	if (perm & OTP_HW_CMN_CTL_LOCK_NSW) {
		perm_mask |= (SOTP_PERM_BLK_NSEC_W<<SOTP_PERM_BLK_SHIFT);
	}
	if (perm_mask) {
               if (clear_lock) {
                       sotp_hw_writel_and(dev, SOTP_PERM_OFFSET, (~perm_mask));
               } 
               sotp_hw_writel(dev, SOTP_PERM_OFFSET, perm_mask);
               rc = OTP_HW_CMN_OK;
	}
err:
	return rc;
}

__weak otp_hw_cmn_err_t  sotp_hw_write(otp_hw_cmn_t *dev, 
				u32 addr,
				otp_hw_cmn_row_conf_t *conf, 
				const u32* data,
				u32 size)
{
	otp_hw_cmn_err_t rc = OTP_HW_CMN_ERR_FAIL;
	if (conf && conf->addr_type == OTP_HW_CMN_ROW_ADDR_ROW) {
		u32 raw_write = 1;
		if (conf->op_type == OTP_HW_CMN_CTL_OTPCMD_ECC) {
			u32 ecc = 0;
			if (sotpReadEccRow(dev, addr, &ecc, 0) != SOTP_S_ROW_SUCCESS) {
				goto err;
			}
			if (ecc) {
				rc  = OTP_HW_CMN_ERR_DATA_ECC;
				SOTP_ERR_PRINT("%s: Row already has valid data! Not writing SOTP row\n", 
					__FUNCTION__);
				goto err;
			}
			raw_write = 0;
		}
    		if ( sotpWriteEccRow(dev, addr, *(u32*)data, raw_write) != SOTP_S_ROW_SUCCESS) {
			goto err; 		
		}
	} else {
		u32 sect = 0;
		if (row2sect(addr, &sect)) {
			goto err;
		}	
    		if (sotpWriteKeyslotData(dev, sect, (u32*)data, 
			size/sizeof(u32)) != SOTP_S_KEY_SUCCESS) {
			goto err;
		}
	}
	/* Non ecc raw row write
 	*	case OTP_HW_CMN_ADDR_TYPE_ROW: {
    		if ( sotpWriteEccRow(dev, addr, *(u32*)data, 1) != SOTP_S_ROW_SUCCESS) {
			goto err; 		
		}
		break;
	}*/
	rc = OTP_HW_CMN_OK;
err:
	return rc;
}

__weak otp_hw_cmn_err_t sotp_hw_ctl(otp_hw_cmn_t *dev, 
			const otp_hw_cmn_ctl_cmd_t *cmd,
			u32* res)
{
	otp_hw_cmn_err_t  rc = OTP_HW_CMN_ERR_FAIL;
	u32 clear_lock = 0;
	switch((u32)cmd->ctl) {
   		case OTP_HW_CMN_CTL_CONF:
			memcpy(&dev->row_conf, 
				(otp_hw_cmn_row_conf_t*)cmd->data, 
				cmd->size);

			dev->ctl_cmd.ctl |= cmd->ctl;
			rc = OTP_HW_CMN_OK;
			break;
   		case (~((u32)OTP_HW_CMN_CTL_CONF)):
			dev->ctl_cmd.ctl &= ~cmd->ctl;
			rc = OTP_HW_CMN_OK;
			break;
		case	OTP_HW_CMN_CTL_STATUS: {
			u32 sect;
			otp_hw_ctl_data_t* ctl_data = (otp_hw_ctl_data_t*)cmd->data;
			_TR(ctl_data->addr);
			if (row2sect(ctl_data->addr, &sect)) {
				rc = OTP_HW_CMN_ERR_UNSP;
				return rc;
			}
  			rc = dev_status(dev, sect, 
				ctl_data->perm, res);
			break;
		}
		case	OTP_HW_CMN_CTL_UNLOCK: 
			clear_lock = 1;
		case	OTP_HW_CMN_CTL_LOCK: {
			u32 sect;
			otp_hw_ctl_data_t* ctl_data = (otp_hw_ctl_data_t*)cmd->data;	
			if (row2sect(ctl_data->addr, &sect)) {
				rc = OTP_HW_CMN_ERR_UNSP;
				return rc;
			}
  			rc = sotp_hw_lock(dev, sect, ctl_data->perm, clear_lock);
			break;
		}
		default:
			rc = OTP_HW_CMN_ERR_UNSP;
			break;
	}
	return  rc;
}

static  otp_hw_cmn_err_t sotp_hw_write_dev(otp_hw_cmn_t* dev, 
		u32 addr,
		const u32* data,
		u32 size)
{
        otp_hw_cmn_row_t *row;
	otp_hw_cmn_row_conf_t *cfg;  
	otp_hw_cmn_err_t  rc = OTP_HW_CMN_ERR_FAIL;
	rc = lookup_row(dev, addr, &row);

	if( dev->ctl_cmd.ctl&OTP_HW_CMN_CTL_CONF )
		cfg = &dev->row_conf;
	else if (rc == 0)
		cfg = &row->conf;
	else
		cfg = NULL;

	SOTP_DBG("%s: ctl_cmd.ctl:%d rc:%d\n", __FUNCTION__, dev->ctl_cmd.ctl, rc);

	return dev->write_ex? 
		dev->write_ex(dev, addr, cfg, data, size) : OTP_HW_CMN_ERR_UNSP;
}

static otp_hw_cmn_err_t sotp_hw_read_dev(otp_hw_cmn_t *dev,
			u32 addr,
			u32 *data,
			u32 size)
{
        otp_hw_cmn_row_t *row;
	otp_hw_cmn_row_conf_t *cfg;  
	otp_hw_cmn_err_t  rc = OTP_HW_CMN_ERR_FAIL;
	rc = lookup_row(dev, addr, &row);

	if( dev->ctl_cmd.ctl&OTP_HW_CMN_CTL_CONF )
		cfg = &dev->row_conf;
	else if (rc == 0)
		cfg = &row->conf;
	else
		cfg = NULL;

	SOTP_DBG("%s: ctl_cmd.ctl:%d rc:%d\n", __FUNCTION__, dev->ctl_cmd.ctl, rc);

	return dev->read_ex?
		dev->read_ex(dev, addr, cfg, data, size):OTP_HW_CMN_ERR_UNSP;

}

__weak otp_hw_cmn_err_t sotp_hw_dev_mmap(otp_hw_cmn_t* dev)
{
	dev->mm = SOTP_BASE;
	return OTP_HW_CMN_OK;
}

__weak otp_hw_cmn_err_t sotp_hw_init (otp_hw_cmn_t *dev)
{
	int i = 0;
	int *m = sectn_map;
	DEFINE_SOTP_MAP_ROW_INITLR(rows);
	if (sotp_hw_dev_mmap(dev)) {
		return OTP_HW_CMN_ERR_FAIL;
	}
	dev->rows = rows;
	dev->row_max = sizeof(rows)/sizeof(otp_hw_cmn_row_t);
	/* map for a section to a row count */	
	for (i = 1 ; i < SOTP_SECT_ID_MAX; i++) {
		 m[i] += m[i-1];
	}
	return OTP_HW_CMN_OK;
}

otp_hw_cmn_err_t sotp_hw_cmn_init (otp_hw_cmn_t *dev)
{
	if (sotp_hw_init(dev)) {
		return OTP_HW_CMN_ERR_FAIL;
	}
	dev->write_ex = sotp_hw_write;
	dev->write = sotp_hw_write_dev;
	dev->read_ex = sotp_hw_read;
	dev->read = sotp_hw_read_dev;
	dev->ctl  = sotp_hw_ctl;
	return OTP_HW_CMN_OK;
}


