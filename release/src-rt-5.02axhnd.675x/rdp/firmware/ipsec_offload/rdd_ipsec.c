/*
* <:copyright-BRCM:2014:proprietary:standard
* 
*    Copyright (c) 2014 Broadcom 
*    All Rights Reserved
* 
*  This program is the proprietary software of Broadcom and/or its
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
*/


#include "rdd.h"


/******************************************************************************/
/*                                                                            */
/*                            Global Variables                                */
/*                                                                            */
/******************************************************************************/


void rdd_ipsec_sa_desc_table_address(uint32_t sa_table_addr, uint16_t sa_entry_size)
{
   uint32_t *ptr_32;
   uint16_t *ptr_16;
   int i;
   
   ptr_32 = (uint32_t *)(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + IPSEC_DS_DDR_SA_DESC_TABLE_PTR_ADDRESS);
   MWRITE_32(ptr_32, (uint32_t *)VIRT_TO_PHYS(sa_table_addr));

   ptr_16 = (uint16_t *)(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + IPSEC_DS_DDR_SA_DESC_SIZE_ADDRESS);
   MWRITE_16(ptr_16, sa_entry_size);

   ptr_16 = (uint16_t *)(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + IPSEC_DS_SA_DESC_CAM_TABLE_ADDRESS);
   for (i = 0; i < RDD_IPSEC_DS_SA_DESC_CAM_TABLE_SIZE; i++, ptr_16++)
      MWRITE_16(ptr_16, 0xFFFF);

   ptr_16 = (uint16_t *)(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + IPSEC_US_SA_DESC_CAM_TABLE_ADDRESS);
   for (i = 0; i < RDD_IPSEC_DS_SA_DESC_CAM_TABLE_SIZE; i++, ptr_16++)
      MWRITE_16(ptr_16, 0xFFFF);
}


BL_LILAC_RDD_ERROR_DTE rdd_ipsec_sa_desc_read(rdpa_traffic_dir dir, uint32_t index, void *val)
{
   uint8_t *sram_desc;

   if (dir == rdpa_dir_ds)
   {
      if (index >= RDD_IPSEC_DS_SA_DESC_TABLE_SIZE)
         return BL_LILAC_RDD_ERROR_ILLEGAL_IPSEC_SA_DESC_TABLE_INDEX;

      sram_desc = (uint8_t *)(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) +
                              IPSEC_DS_SA_DESC_TABLE_ADDRESS +
                              (index * sizeof(RDD_IPSEC_SA_DESC_DTS)));
   }
   else
   {
      if (index >= RDD_IPSEC_US_SA_DESC_TABLE_SIZE)
         return BL_LILAC_RDD_ERROR_ILLEGAL_IPSEC_SA_DESC_TABLE_INDEX;

      sram_desc = (uint8_t *)(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) +
                              IPSEC_US_SA_DESC_TABLE_ADDRESS +
                              (index * sizeof(RDD_IPSEC_SA_DESC_DTS)));
   }

   /* Read the descriptor from SRAM. */
   MREAD_BLK_8((uint8_t *)val, sram_desc, sizeof(RDD_IPSEC_SA_DESC_DTS));

   return BL_LILAC_RDD_OK;
}


BL_LILAC_RDD_ERROR_DTE rdd_ipsec_sa_desc_write(rdpa_traffic_dir dir, uint32_t index, const void *val)
{
   uint8_t *sram_desc;

   if (dir == rdpa_dir_ds)
   {
      if (index >= RDD_IPSEC_DS_SA_DESC_TABLE_SIZE)
         return BL_LILAC_RDD_ERROR_ILLEGAL_IPSEC_SA_DESC_TABLE_INDEX;

      sram_desc = (uint8_t *)(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) +
                              IPSEC_DS_SA_DESC_TABLE_ADDRESS +
                              (index * sizeof(RDD_IPSEC_SA_DESC_DTS)));
   }
   else
   {
      if (index >= RDD_IPSEC_US_SA_DESC_TABLE_SIZE)
         return BL_LILAC_RDD_ERROR_ILLEGAL_IPSEC_SA_DESC_TABLE_INDEX;

      sram_desc = (uint8_t *)(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) +
                              IPSEC_US_SA_DESC_TABLE_ADDRESS +
                              (index * sizeof(RDD_IPSEC_SA_DESC_DTS)));
   }

   if (*(uint32_t *)val)
      /* spi is not 0. Set the descriptor to SRAM. */
      MWRITE_BLK_8(sram_desc, (uint8_t *)val, sizeof(RDD_IPSEC_SA_DESC_DTS));
   else
      /* set spi to 0 to invalidate the descriptor entry */
      MWRITE_32(sram_desc, 0);

   return BL_LILAC_RDD_OK;
}


BL_LILAC_RDD_ERROR_DTE rdd_ipsec_sa_desc_cam_tbl_read(rdpa_traffic_dir dir, uint32_t index, void *val)
{
   uint16_t *ptr_16;

   if (dir == rdpa_dir_ds)
   {
      if (index >= RDD_IPSEC_DS_SA_DESC_CAM_TABLE_SIZE)
         return BL_LILAC_RDD_ERROR_ILLEGAL_IPSEC_SA_DESC_TABLE_INDEX;

      ptr_16 = (uint16_t *)(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) +
                              IPSEC_DS_SA_DESC_CAM_TABLE_ADDRESS +
                              (index * sizeof(uint16_t)));
   }
   else
   {
      if (index >= RDD_IPSEC_US_SA_DESC_CAM_TABLE_SIZE)
         return BL_LILAC_RDD_ERROR_ILLEGAL_IPSEC_SA_DESC_TABLE_INDEX;

      ptr_16 = (uint16_t *)(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) +
                              IPSEC_US_SA_DESC_CAM_TABLE_ADDRESS +
                              (index * sizeof(uint16_t)));
   }

   /* Read the descriptor from SRAM. */
   MREAD_16(ptr_16, *(uint16_t *)val);

   return BL_LILAC_RDD_OK;
}
