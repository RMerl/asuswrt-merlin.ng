/*
<:copyright-BRCM:2020:DUAL/GPL:standard

   Copyright (c) 2020 Broadcom 
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
* File Name  : skp_base_funcs.c
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
#include <linux/types.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <bcm_sotp.h>
#include <bcm_otp.h>
#include "skp_base_defs.h"

//#define DBG_OTP

#ifdef DBG_OTP
#define _ETR(__p__)  printk("%s \n\t\t\t\t -> :%s :%d  code %d / 0x%x\n",__FILE__,__FUNCTION__,__LINE__,__p__,__p__)
#define _DPRT(...)  printk(__VA_ARGS__) 
#else
#define _ETR(__p__)
#define _DPRT(...)
#endif

#define SKP_BASE_CHK() {\
                            if(!skp_base_ptr)\
                            {\
                                printk("\nSKP base address not specified!\n");\
                                return -1;\
                            }\
                        }

#define IS_KSR_ADDR(i)			((int)i < KSR_MAX_ROW_ADDR )
#define IS_FSR_ADDR(i)			(((int)i >=FSR_START_ROW_ADDR) && ((int)i < FSR_MAX_ROW_ADDR))

static char * skp_base_ptr = NULL;

/* This is an array representation of all the available SOTP slots. 
 * KSR slots are used for secure item/key storage. KSR slots cannot
 * be rewritten, i.e they have no raw write/read capabilities.
 * FSR slots are used for antirollback ONLY for the moment. FSR slots
 * can be rewritten, currently they are default programmed for raw 
 * reads writes
 */
static skp_hw_cmn_row_t sotp_rows[ ] = {															
		{.feat = SKP_CONT_ID_FLD_ROE,          "ROE", .addr = (KSR_START_ROW_ADDR+0), .range = 8, .conf = {.perm = KSR_PORTAL_PGM_DESC_SRW_PERM, .op_type = SKP_HW_CMN_CTL_OTPCMD_ECC}},	
		{.feat = SKP_CONT_ID_FLD_HMID,         "MID", .addr = (KSR_START_ROW_ADDR+1), .range = 8, .conf = {.perm = KSR_PORTAL_PGM_DESC_SRW_PERM, .op_type = SKP_HW_CMN_CTL_OTPCMD_ECC}},	
#if defined(CONFIG_BCM96765)
		{.feat = SKP_CONT_ID_FLD_ROE1,         "ROE (extended)", .addr = (KSR_START_ROW_ADDR+2), .range = 8, .conf = {.perm = (KSR_PORTAL_PGM_DESC_NRW_PERM), .op_type = SKP_HW_CMN_CTL_OTPCMD_ECC}},
		{.feat = SKP_CONT_ID_KEY_DEV_SPECIFIC, "Device Specific Key", .addr = (KSR_START_ROW_ADDR+3), .range = 8, .conf = {.perm = (KSR_PORTAL_PGM_DESC_NRW_PERM), .op_type = SKP_HW_CMN_CTL_OTPCMD_ECC}},
#else		
		{.feat = SKP_CONT_ID_KEY_DEV_SPECIFIC, "Device Specific Key", .addr = (KSR_START_ROW_ADDR+2), .range = 8, .conf = {.perm = (KSR_PORTAL_PGM_DESC_NRW_PERM), .op_type = SKP_HW_CMN_CTL_OTPCMD_ECC}},
		{.feat = SKP_CONT_ID_SER_NUM,          "Secure Serial Num", .addr = (KSR_START_ROW_ADDR+3), .range = 8, .conf = {.perm = (KSR_PORTAL_PGM_DESC_NRW_PERM), .op_type = SKP_HW_CMN_CTL_OTPCMD_ECC}},
#endif		
		{.feat = SKP_CONT_ID_KEY_SECT_1,       NULL, .addr = (KSR_START_ROW_ADDR+4), .range = 8, .conf = {.perm = (KSR_PORTAL_PGM_DESC_NRW_PERM), .op_type = SKP_HW_CMN_CTL_OTPCMD_ECC}},
		{.feat = SKP_CONT_ID_KEY_SECT_2,       NULL, .addr = (KSR_START_ROW_ADDR+5), .range = 8, .conf = {.perm = (KSR_PORTAL_PGM_DESC_NRW_PERM), .op_type = SKP_HW_CMN_CTL_OTPCMD_ECC}},
		{.feat = SKP_CONT_ID_ANTI_ROLLBACK,    "Anti-Rollback lvl", .addr = (FSR_START_ROW_ADDR+0), .range = 4, .conf = {.op_type = SKP_HW_CMN_CTL_CONF}},	
		{.feat = SKP_CONT_ID_UNUSED_1,         NULL, .addr = (FSR_START_ROW_ADDR+1), .range = 4, .conf = {.op_type = SKP_HW_CMN_CTL_CONF}},	
#if defined(CONFIG_BCM96765)
		{.feat = SKP_CONT_ID_SER_NUM,          "Secure Serial Num", .addr = (FSR_START_ROW_ADDR+2), .range = 8, .conf = {.op_type = SKP_HW_CMN_CTL_OTPCMD_ECC}},	
#else		
		{.feat = SKP_CONT_ID_UNUSED_2,         NULL, .addr = (FSR_START_ROW_ADDR+2), .range = 8, .conf = {.op_type = SKP_HW_CMN_CTL_CONF}},	
#endif		
		{.feat = SKP_CONT_ID_UNUSED_3,         NULL, .addr = (FSR_START_ROW_ADDR+3), .range = 8, .conf = {.op_type = SKP_HW_CMN_CTL_CONF}},	
		{.feat = SKP_CONT_ID_UNUSED_4,         NULL, .addr = (FSR_START_ROW_ADDR+4), .range = 2, .conf = {.op_type = SKP_HW_CMN_CTL_CONF}},	
#if defined(CONFIG_BCM96765)
		{.feat = SKP_CONT_ID_LIC_UNIQUE_ID,    "Unique Licensing ID", .addr = (FSR_START_ROW_ADDR+5), .range = 4, .conf = {.op_type = SKP_HW_CMN_CTL_CONF}},	
#else		
		{.feat = SKP_CONT_ID_UNUSED_5,         NULL, .addr = (FSR_START_ROW_ADDR+5), .range = 4, .conf = {.op_type = SKP_HW_CMN_CTL_CONF}},	
#endif		
		{.feat = SKP_CONT_ID_UNUSED_6,         NULL, .addr = (FSR_START_ROW_ADDR+6), .range = 4, .conf = {.op_type = SKP_HW_CMN_CTL_CONF}},	
		{.feat = SKP_CONT_ID_UNUSED_7,         NULL, .addr = (FSR_START_ROW_ADDR+7), .range = 2, .conf = {.op_type = SKP_HW_CMN_CTL_CONF}},	
};

static skp_hw_cmn_err_t  skp_lookup_row( 
			uint32_t addr,
			skp_hw_cmn_row_t** row)
{
	uint32_t i;
	skp_hw_cmn_row_t* rows = sotp_rows;
	
	for (i = 0; i <  sizeof(sotp_rows)/sizeof(skp_hw_cmn_row_t); i++) {
		if (rows[i].addr == addr) {
			*row = &rows[i];
			return SKP_HW_CMN_OK;	
		}
	}
	_ETR(SKP_HW_CMN_ERR_UNSP);
	return SKP_HW_CMN_ERR_UNSP;
}

static skp_hw_cmn_err_t skp_lookup_feat(
			skp_hw_content_id_t feat_id, 
			uint32_t * addr,
			int * size_bytes)
{
	uint32_t i;
	skp_hw_cmn_row_t* rows = sotp_rows;
	*addr = 0;
	*size_bytes = 0;
	
	for (i = 0; i <  sizeof(sotp_rows)/sizeof(skp_hw_cmn_row_t); i++) {
		if (rows[i].feat == feat_id) {
			*addr = rows[i].addr;
			*size_bytes = rows[i].range * sizeof(uint32_t);
			return SKP_HW_CMN_OK;	
		}
	}
	_ETR(SKP_HW_CMN_ERR_UNSP);
	return SKP_HW_CMN_ERR_UNSP;
}
/******************** FSR ACCESS FUNCTIONS ********************/
 uint32_t fsr_hw_readl(uint32_t offs)
{
	uint32_t val;
	val = readl((const void __iomem *)(skp_base_ptr+FSR_BASE_OFFSET+offs));
	_DPRT("0x%p > 0x%x\n",(void*)(skp_base_ptr+offs+FSR_BASE_OFFSET), val);
	return val;
}

 void fsr_hw_writel(uint32_t offs, uint32_t data)
{
	_DPRT("0x%p < 0x%x\n",(void*)(skp_base_ptr+offs+FSR_BASE_OFFSET), data);
	writel(data, 
		(void __iomem *)(skp_base_ptr+FSR_BASE_OFFSET+offs));
}

static  skp_hw_cmn_err_t fsr_poll_otp_status( 
						uint32_t fsr_offs)
{
	int to = 100, errc = 0; 

	do {
		uint32_t st = fsr_hw_readl( FSR_OTP_STATUS_OFFSET+fsr_offs);
		_DPRT("Fsr status 0x%x\n",st);
                (void)st;
		if (!FSR_OTP_STATUS_ACTIVE(fsr_hw_readl( FSR_OTP_STATUS_OFFSET+fsr_offs) )) {
			break;
		}
		msleep(1);
		to--;
	} while(to);

	errc = FSR_OTP_STATUS_CMD_ERROR(fsr_hw_readl(FSR_OTP_STATUS_OFFSET+fsr_offs));
	if (errc) {
		_DPRT("%s: ERROR: otp status err 0x%x\n",
			__FUNCTION__, fsr_hw_readl( FSR_OTP_STATUS_OFFSET+fsr_offs));
	}
	return (!to || errc);
}

static skp_hw_cmn_err_t fsr_read(
			skp_hw_cmn_row_t  *row,
			skp_hw_cmn_row_conf_t* row_conf,
			uint32_t *data,
			uint32_t size)
{
        int i;
	skp_hw_cmn_err_t rc = SKP_HW_CMN_ERR_FAIL; 
	uint32_t fsr_offs;
	uint32_t raw_read = 0;
	uint32_t desc = 0;
	uint32_t num_words;

	fsr_offs = FSR_SIZE*(row->addr-FSR_START_ROW_ADDR);
	if (row_conf && row_conf->op_type != SKP_HW_CMN_CTL_OTPCMD_ECC) {
		_DPRT("%s %d \n\t\t\t\t Raw read addr:0x%08x!\n",
			__FUNCTION__,__LINE__, row->addr);
		raw_read = 1;
	}

	/* 1 - Check portal status */
	/* Issue RELOAD_DESC command to FSR_PORTAL_FP0_OTP_COMMAND */
	fsr_hw_writel( FSR_OTP_CMD_OFFSET+fsr_offs,FSR_OTP_CMD_RELOAD_DESC);
	/* Check cmd complete,FSR_PORTAL_FP0_OTP_STATUS active(31)=0 cmd_err(7:0)=0 */
	if (fsr_poll_otp_status( fsr_offs)) {
		rc = SKP_HW_CMN_ERR_TMO; 
		_ETR(rc);
		goto err;
	}

	/* Check valid(20)==1, ready(21)==1 */
	if (!FSR_PORTAL_VALID(fsr_hw_readl( FSR_PORTAL_STATUS_OFFSET+fsr_offs))) {
		if (FSR_PORTAL_READY(fsr_hw_readl( FSR_PORTAL_STATUS_OFFSET+fsr_offs))) {
			_DPRT("%s %d Empty Portal! portal_status 0x%x\n",
				__FUNCTION__,__LINE__,
			fsr_hw_readl( FSR_PORTAL_STATUS_OFFSET+fsr_offs));
			/* Portal is empty --> return zeroed data */
			memset(data, 0x0, size);
		} else {
			_DPRT("%s %d Blocked Portal! portal_status 0x%x\n",
				__FUNCTION__,__LINE__,
				fsr_hw_readl( FSR_PORTAL_STATUS_OFFSET+fsr_offs));
			/* Portal is bocked */
			for (i = 0; i < size/sizeof(uint32_t); i++ ) {
				data[i] = 0xDEADBEEF;
			}
		}
		for (i = 0; i < size/sizeof(uint32_t); i++ ) {
			_DPRT("%s %d \n\t\t\t\t read data[%d] 0x%x \n",
				__FUNCTION__,__LINE__, i, data[i]);
		}
		rc = SKP_HW_CMN_OK; 
		goto err;
	}

	/* DESC status is in FSR_PORTAL_FP0_PORTAL_STATUS */
	/* RLOCK = 1 --> Cant read */
	if( FSR_PORTAL_RLOCK(fsr_hw_readl( FSR_PORTAL_STATUS_OFFSET+fsr_offs)) ) {
		rc = SKP_HW_CMN_ERR_FAIL; 
		printk("%s %d Cant Read! RLOCK is set!\n",
			__FUNCTION__,__LINE__);
		_ETR(rc);
		goto err;
	}

	/* RAW and ECC_PGM_DIS = 0 --> Cant raw read */
	if( raw_read && !FSR_PORTAL_ECC_DIS(fsr_hw_readl( FSR_PORTAL_STATUS_OFFSET+fsr_offs))){
		rc = SKP_HW_CMN_ERR_FAIL; 
		printk("%s %d Cant Read RAW! ECC_DIS is NOT set!\n",
			__FUNCTION__,__LINE__);
		_ETR(rc);
		goto err;
	}

	/* 2 - Read Decriptor to determine portal size */
	/* Issue READ_DESC command to FSR_PORTAL_FP0_OTP_COMMAND */
	fsr_hw_writel( FSR_OTP_CMD_OFFSET+fsr_offs,FSR_OTP_CMD_READ_DESC);
	/* Check command complete */
	if (fsr_poll_otp_status( fsr_offs)) {
		rc = SKP_HW_CMN_ERR_TMO; 
		_ETR(rc);
		goto err;
	}
	/* DESC is in FSR_PORTAL_FP0_DATA */
	/* Check valid(31)==1 , this should succeed as PORTAL status is good*/
	desc = fsr_hw_readl( FSR_DATA_OFFSET+fsr_offs);
	if(!FSR_RD_DGOOD(desc)) {
		_ETR(rc);
		printk("%s %d desc 0x%x\n",
			__FUNCTION__,__LINE__,
			fsr_hw_readl( FSR_DATA_OFFSET+fsr_offs));
		rc = SKP_HW_CMN_ERR_FAIL; 
		goto err;
	}
	/* Check if requested data fits inside this FSR, check SIZE(23:12) */
	num_words = ((size/sizeof(uint32_t)) > FSR_RD_DDATA_SIZE(desc))? FSR_RD_DDATA_SIZE(desc):(size/sizeof(uint32_t));

	/* 3 - Read data */
	memset(data, 0x0, size);
	_DPRT("%s %d \n\t\t\t\t Num_words:%d \n",
		__FUNCTION__,__LINE__, (int)num_words);
	for (i = 0; i < num_words; i++ ) {
		/* Issue READ_DATA command - Set WORD SEL(15:4) to pick word# */
		fsr_hw_writel( FSR_OTP_CMD_OFFSET+fsr_offs,
			(i<<FSR_PORTAL_OTP_CMD_WORD_SEL_SHIFT)|FSR_OTP_CMD_READ_DATA);
		/* Check command complete */
		if (fsr_poll_otp_status( fsr_offs)) {
			rc = SKP_HW_CMN_ERR_TMO; 
			_ETR(rc);
			goto err;
		}

		/* Read data from FSR_PORTAL_FP0_DATA */
		data[i] = fsr_hw_readl( FSR_DATA_OFFSET+fsr_offs);
		_DPRT("%s %d \n\t\t\t\t read data[%d] 0x%x \n",
			__FUNCTION__,__LINE__, i, data[i]);

		/* Optional - Read ECC from FSR_PORTAL_FP0_RD_ECC */
	}
	rc = SKP_HW_CMN_OK;
err:
	return rc;
}

static skp_hw_cmn_err_t fsr_write(
			skp_hw_cmn_row_t  *row,
			skp_hw_cmn_row_conf_t* row_conf,
			uint32_t *data,
			uint32_t size)
{
        int i;
	skp_hw_cmn_err_t rc = SKP_HW_CMN_ERR_FAIL; 
	uint32_t fsr_offs; 
	uint32_t raw_write = 0;
	uint32_t desc = 0;
	uint32_t num_words;

	fsr_offs = FSR_SIZE*(row->addr-FSR_START_ROW_ADDR);
	if (row_conf && row_conf->op_type != SKP_HW_CMN_CTL_OTPCMD_ECC) {
		_DPRT("%s %d \n\t\t\t\t Raw write!\n",
			__FUNCTION__,__LINE__);
		raw_write = 1;
	}

	/* 1 - Place OTP in program mode */
	if( bcm_otp_auth_prog_mode() ) {
		rc = SKP_HW_CMN_ERR_FAIL;
		_ETR(rc);
		goto err;
	}

	/* 2 - Check portal status */
	/* Issue RELOAD_DESC command to FSR_PORTAL_FP0_OTP_COMMAND */
	fsr_hw_writel( FSR_OTP_CMD_OFFSET+fsr_offs,FSR_OTP_CMD_RELOAD_DESC);
	/* Check cmd complete,FSR_PORTAL_FP0_OTP_STATUS active(31)=0 cmd_err(7:0)=0 */
	if (fsr_poll_otp_status( fsr_offs)) {
		rc = SKP_HW_CMN_ERR_TMO; 
		_ETR(rc);
		goto err;
	}

	/* Check valid(20)==1, ready(21)==1*/
	if (!FSR_PORTAL_VALID(fsr_hw_readl( FSR_PORTAL_STATUS_OFFSET+fsr_offs))) {
		/* If portal is not ready ---> Blocked */
		if (!FSR_PORTAL_READY(fsr_hw_readl(FSR_PORTAL_STATUS_OFFSET+fsr_offs))){
			printk("%s %d \n\t\t\t\t Cant Write to Keyslot!\n",
				__FUNCTION__,__LINE__);
			rc = SKP_HW_CMN_ERR_WRITE_FAIL; 
			_ETR(rc);
			goto err;
		}

		/* If Portal DESC is not valid ---> empty!, program it */
		/* Set VALID = 1 and RLOCK = 0 */
		desc = FSR_RD_DESC_VALID_MASK<<FSR_RD_DESC_VALID_SHIFT;

		/* If RAW, set ECC_PGM_DIS */
		if( raw_write )
			desc |= FSR_RD_DESC_ECC_PGM_DIS_MASK<<FSR_RD_DESC_ECC_PGM_DIS_SHIFT;

		/* A - Write descriptor to FSR_PORTAL_FP0_DATA */
		fsr_hw_writel( FSR_DATA_OFFSET+fsr_offs,desc);

		/* B - Issue PGM_DESC command to FSR_PORTAL_FP0_OTP_COMMAND */
		fsr_hw_writel( FSR_OTP_CMD_OFFSET+fsr_offs,FSR_OTP_CMD_PGM_DESC);
		/* Check cmd complete,FSR_PORTAL_FP0_OTP_STATUS active(31)=0 cmd_err(7:0)=0 */
		if (fsr_poll_otp_status( fsr_offs)) {
			rc = SKP_HW_CMN_ERR_TMO; 
			_ETR(rc);
			goto err;
		}

		/* C - Issue PGM_DESC_ECC command to FSR_PORTAL_FP0_OTP_COMMAND */
		fsr_hw_writel( FSR_OTP_CMD_OFFSET+fsr_offs,FSR_OTP_CMD_PGM_DESC_ECC);
		/* Check cmd complete,FSR_PORTAL_FP0_OTP_STATUS active(31)=0 cmd_err(7:0)=0 */
		if (fsr_poll_otp_status( fsr_offs)) {
			rc = SKP_HW_CMN_ERR_TMO; 
			_ETR(rc);
			goto err;
		}

		/* D - Issue RELOAD_DESC command to FSR_PORTAL_FP0_OTP_COMMAND */
		fsr_hw_writel( FSR_OTP_CMD_OFFSET+fsr_offs,FSR_OTP_CMD_RELOAD_DESC);
		/* Check cmd complete,FSR_PORTAL_FP0_OTP_STATUS active(31)=0 cmd_err(7:0)=0 */
		if (fsr_poll_otp_status( fsr_offs)) {
			rc = SKP_HW_CMN_ERR_TMO; 
			_ETR(rc);
			goto err;
		}
	} else {
		/* Valid PORTAL and DESC */
		/* DESC status is in FSR_PORTAL_FP0_PORTAL_STATUS */
		/* RLOCK = 1 --> Cant write */
		if( FSR_PORTAL_RLOCK(fsr_hw_readl( FSR_PORTAL_STATUS_OFFSET+fsr_offs)) ) {
			rc = SKP_HW_CMN_ERR_WRITE_FAIL; 
			printk("%s %d \n\t\t\t\t Cant Write! RLOCK is set!\n",
				__FUNCTION__,__LINE__);
			_ETR(rc);
			goto err;
		}

		/* RAW and ECC_PGM_DIS = 0 --> Cant raw write */
		if( raw_write && !FSR_PORTAL_ECC_DIS(fsr_hw_readl( 
				FSR_PORTAL_STATUS_OFFSET+fsr_offs))) {
			rc = SKP_HW_CMN_ERR_WRITE_FAIL; 
			printk("%s %d \n\t\t\t\t Cant Write RAW! ECC_DIS is NOT set!\n",
				__FUNCTION__,__LINE__);
			_ETR(rc);
			goto err;
		}

		/* NOT RAW and ECC_PGM_DIS = 1 --> Cant non-raw write without ECC */
		if( !raw_write && FSR_PORTAL_ECC_DIS(fsr_hw_readl( 
				FSR_PORTAL_STATUS_OFFSET+fsr_offs))) {
			rc = SKP_HW_CMN_ERR_WRITE_FAIL; 
			printk("%s %d \n\t\t\t\t Cant Write Non-raw! ECC_DIS is set!\n",
				__FUNCTION__,__LINE__);
			_ETR(rc);
			goto err;
		}
	}


	/* 3 - Read Decriptor to determine portal size */
	/* Issue READ_DESC command to FSR_PORTAL_FP0_OTP_COMMAND */
	fsr_hw_writel( FSR_OTP_CMD_OFFSET+fsr_offs,FSR_OTP_CMD_READ_DESC);
	/* Check command complete */
	if (fsr_poll_otp_status( fsr_offs)) {
		rc = SKP_HW_CMN_ERR_TMO; 
		_ETR(rc);
		goto err;
	}
	/* Read and check if descriptor is valid(31) */
	desc = fsr_hw_readl( FSR_DATA_OFFSET+fsr_offs);
	if(!FSR_RD_DGOOD(desc)) {
		_ETR(rc);
		printk("%s %d \n\t\t\t\t desc 0x%x\n",
			__FUNCTION__,__LINE__,
			fsr_hw_readl( FSR_DATA_OFFSET+fsr_offs));
		rc = SKP_HW_CMN_ERR_FAIL; 
		goto err;
	}
	/* Check if requested data fits inside this FSR, check SIZE(23:12) */
	num_words = ((size/sizeof(uint32_t)) > FSR_RD_DDATA_SIZE(desc))? 
			FSR_RD_DDATA_SIZE(desc):(size/sizeof(uint32_t));

	/* If not a raw_write, check if portal has been written to */
	if( !raw_write ) {
		for (i = 0; i < num_words; i++ ) {
			/* Issue READ_DATA command - Set WORD SEL(15:4) to pick word# */
			fsr_hw_writel( FSR_OTP_CMD_OFFSET+fsr_offs,
				(i<<FSR_PORTAL_OTP_CMD_WORD_SEL_SHIFT)|FSR_OTP_CMD_READ_DATA);
			/* Check command complete */
			if (fsr_poll_otp_status( fsr_offs)) {
				rc = SKP_HW_CMN_ERR_TMO; 
				_ETR(rc);
				goto err;
			}

			/* Read ECC from FSR_PORTAL_FP0_RD_ECC */
			if(fsr_hw_readl(FSR_RD_ECC_OFFSET+fsr_offs) & FSR_PORTAL_RD_ECC_MASK) {
				rc = SKP_HW_CMN_ERR_WRITE_FAIL; 
				printk("%s %d \n\t\t\t\t Cannot overwrite portal! ECC is set!\n",
					__FUNCTION__,__LINE__);
				_ETR(rc);
				goto err;
			}
		}
	}

	/* 4 - Write data and ECC ( if reqd ) */
	for (i = 0; i < num_words; i++ ) {
		/* Read data from FSR_PORTAL_FP0_DATA */
		_DPRT("%s %d \n\t\t\t\t write data[%d] 0x%x \n",
			__FUNCTION__,__LINE__, i, data[i]);

		fsr_hw_writel( FSR_DATA_OFFSET+fsr_offs, data[i]);

		/* Issue WRITE_DATA command - Set WORD SEL(15:4) to pick word# */
		fsr_hw_writel( FSR_OTP_CMD_OFFSET+fsr_offs,
			(i<<FSR_PORTAL_OTP_CMD_WORD_SEL_SHIFT)|FSR_OTP_CMD_PGM_DATA);
		/* Check command complete */
		if (fsr_poll_otp_status( fsr_offs)) {
			rc = SKP_HW_CMN_ERR_TMO; 
			_ETR(rc);
			goto err;
		}

		if( !raw_write ) {		
			/* Issue WRITE_ECC command - Set WORD SEL(15:4) to pick word# */
			fsr_hw_writel( FSR_OTP_CMD_OFFSET+fsr_offs,
				(i<<FSR_PORTAL_OTP_CMD_WORD_SEL_SHIFT)|FSR_OTP_CMD_PGM_DATA_ECC);
			/* Check command complete */
			if (fsr_poll_otp_status( fsr_offs)) {
				rc = SKP_HW_CMN_ERR_TMO; 
				_ETR(rc);
				goto err;
			}
		}
	}
	
	_DPRT("Fused FSR rows as expected \n");
       	rc = SKP_HW_CMN_OK; 
err:
        return rc;
}

/******************** KSR ACCESS FUNCTIONS ********************/
 uint32_t ksr_hw_readl(uint32_t offs)
{
	return readl((const void __iomem *)(skp_base_ptr+KSR_BASE_OFFSET+offs));
}

 void ksr_hw_writel(uint32_t offs, uint32_t data)
{
	_DPRT("0x%x < 0x%x\n",skp_base_ptr+offs, data);
	writel(data, 
		(void __iomem *)(skp_base_ptr+KSR_BASE_OFFSET+offs));
}

static  skp_hw_cmn_err_t ksr_poll_otp_status( 
						uint32_t ksr_offs)
{
	int to = 100, errc = 0; 

	do {
		uint32_t st = ksr_hw_readl( KSR_OTP_STATUS_OFFSET+ksr_offs);
		_DPRT("Ksr status 0x%x\n",st);
                (void)st;
		if (!KSR_OTP_STATUS_ACTIVE(ksr_hw_readl( KSR_OTP_STATUS_OFFSET+ksr_offs) )) {
			break;
		}
		msleep(1);
		to--;
	} while(to);

	errc = KSR_OTP_STATUS_CMD_ERROR(ksr_hw_readl(KSR_OTP_STATUS_OFFSET+ksr_offs));
	if (errc) {
		printk("%s: ERROR: otp status err 0x%x\n",
			__FUNCTION__, ksr_hw_readl( KSR_OTP_STATUS_OFFSET+ksr_offs));
	}
	return (!to || errc);
}

static skp_hw_cmn_err_t ksr_read(
			skp_hw_cmn_row_t  *row,
			skp_hw_cmn_row_conf_t* row_conf,
			uint32_t *data,
			uint32_t size)
{
        int i;
	skp_hw_cmn_err_t rc = SKP_HW_CMN_ERR_FAIL; 
	uint32_t ksr_offs;
	ksr_offs = KSR_SIZE*(row->addr-KSR_START_ROW_ADDR);

	/* Check if proper size has been requested -- No partial reads/writes for KSR */
	if ( size != row->range*sizeof(uint32_t)) {
		printk("%s %d \n\t\t\t\t Invalid size:%d requested. Avail size:%d!\n",
			__FUNCTION__,__LINE__, size, (int)(row->range*sizeof(uint32_t)));
		rc = SKP_HW_CMN_ERR_BAD_PARAM;
		_ETR(rc);
		goto err;
	}

	if ( !KSR_PORTAL_VALID(ksr_hw_readl( KSR_PORTAL_STATUS_OFFSET+ksr_offs)) ||
		!KSR_RD_DGOOD(ksr_hw_readl( KSR_RD_DESC_OFFSET+ksr_offs))) {
		_ETR(rc);
		_DPRT("%s %d \n\t\t\t\t portal_status 0x%x rd_desc 0x%x\n",
			__FUNCTION__,__LINE__,ksr_hw_readl( KSR_PORTAL_STATUS_OFFSET+ksr_offs), ksr_hw_readl( KSR_RD_DESC_OFFSET+ksr_offs));
		if(KSR_PORTAL_READY(ksr_hw_readl( KSR_PORTAL_STATUS_OFFSET+ksr_offs))){
			/* Portal is empty but accessable */
			memset((char*)data, 0, size);
			rc = SKP_HW_CMN_OK; 
		} else {
			/* Portal not ready --> blocked */
			for (i = 0; i <  size/sizeof(uint32_t); i++ ) {
				data[i] = 0xDEADBEEF;
			}
			rc = SKP_HW_CMN_OK; 
		}

		goto err;
	}
	for (i = 0; i <  size/sizeof(uint32_t); i++ ) {
		data[i] = ksr_hw_readl( KSR_DATA_OFFSET+i*sizeof(uint32_t)+ksr_offs);
		_DPRT("%s %d \n\t\t\t\t data[%d] 0x%x \n",
			__FUNCTION__,__LINE__, i, data[i]);
	}
	rc = SKP_HW_CMN_OK;
err:
	return rc;

}

static skp_hw_cmn_err_t ksr_write(
			skp_hw_cmn_row_t  *row,
			skp_hw_cmn_row_conf_t* row_conf,
			uint32_t *data,
			uint32_t size)
{
        int i;
	skp_hw_cmn_err_t rc = SKP_HW_CMN_ERR_FAIL; 
	uint32_t ksr_offs; 
	ksr_offs = KSR_SIZE*(row->addr-KSR_START_ROW_ADDR);
	
	/* Check if proper size has been requested -- No partial reads/writes for KSR */
	if ( size != row->range*sizeof(uint32_t)) {
		printk("%s %d \n\t\t\t\t Invalid size:%d requested. Avail size:%d!\n",
			__FUNCTION__,__LINE__, size, (int)(row->range*sizeof(uint32_t)));
		rc = SKP_HW_CMN_ERR_BAD_PARAM;
		_ETR(rc);
		goto err;
	}

	/* Check if portal is blocked or not empty */
	if(!KSR_PORTAL_READY(ksr_hw_readl( KSR_PORTAL_STATUS_OFFSET+ksr_offs))){   
		printk("%s %d \n\t\t\t\t Cant Write to Blocked Keyslot!\n",
			__FUNCTION__,__LINE__);
		rc = SKP_HW_CMN_ERR_WRITE_FAIL; 
		_ETR(rc);
		goto err;
	}
	if(KSR_PORTAL_KEY_LOADED(ksr_hw_readl( KSR_PORTAL_STATUS_OFFSET+ksr_offs)) ||
           KSR_PORTAL_DESC_VALID(ksr_hw_readl( KSR_PORTAL_STATUS_OFFSET+ksr_offs))){
		printk("%s %d \n\t\t\t\t Cant Write to non-empty Keyslot!\n",
			__FUNCTION__,__LINE__);
		rc = SKP_HW_CMN_ERR_WRITE_FAIL; 
		_ETR(rc);
		goto err;
	}

	/* Set OTP interface in to program mode */
	if( bcm_otp_auth_prog_mode() ) {
		rc = SKP_HW_CMN_ERR_FAIL;
		_ETR(rc);
		goto err;
	}

	_ETR((uint32_t)row_conf->perm);
	_ETR(KSR_PORTAL_PGM_DESC_SRW_PERM);
	if (ksr_poll_otp_status( ksr_offs)) {
		rc = SKP_HW_CMN_ERR_TMO; 
		_ETR(rc);
		goto err;
	}
	ksr_hw_writel( KSR_PGM_DESC_OFFSET + ksr_offs, (uint32_t)row_conf->perm);
	_ETR(ksr_hw_readl( KSR_PGM_DESC_OFFSET+ksr_offs));
	ksr_hw_writel( KSR_OTP_CMD_OFFSET + ksr_offs, KSR_PORTAL_OTP_CMD_PGM_DESC_WORD);
	if (ksr_poll_otp_status( ksr_offs)) {
		rc = SKP_HW_CMN_ERR_TMO; 
		_ETR(rc);
		goto err;
	}
	_ETR(KSR_PORTAL_OTP_CMD_PGM_DESC_WORD);
	if ((row_conf->op_type&SKP_HW_CMN_CTL_OTPCMD_ECC)) {
		ksr_hw_writel( KSR_OTP_CMD_OFFSET+ksr_offs,KSR_PORTAL_OTP_CMD_PGM_DESC_ECC);
		if (ksr_poll_otp_status(ksr_offs)) {
			rc = SKP_HW_CMN_ERR_TMO; 
			_ETR(rc);
			goto err;
		}
		_ETR(KSR_PORTAL_OTP_CMD_PGM_DESC_ECC);
	}
	for (i = 0 ; i <  size/sizeof(uint32_t); i++) {
		ksr_hw_writel( KSR_DATA_OFFSET+ksr_offs+i*sizeof(uint32_t), data[i]);
	}
	ksr_hw_writel( KSR_OTP_CMD_OFFSET + ksr_offs, KSR_PORTAL_OTP_CMD_PGM_KEY);
	_ETR(ksr_hw_readl( KSR_OTP_CMD_OFFSET+ksr_offs));
	if (ksr_poll_otp_status( ksr_offs)) {
		rc = SKP_HW_CMN_ERR_TMO;
		_ETR(rc);
		goto err;
	}
	
	_DPRT("Fused KSR rows as expected \n");
       	rc = SKP_HW_CMN_OK; 
err:
        return rc;
}

static skp_hw_cmn_err_t fsr_set_populated_pacs( uint32_t pac_val)
{
	int i;
	skp_hw_cmn_err_t rc = SKP_HW_CMN_ERR_FAIL; 
	for (i = 0; i < FSR_MAX_ROW_ADDR - FSR_START_ROW_ADDR; i++) { 
		/* If portal is valid and descriptor is good, we have valid data so lock NS */
		if ( FSR_PORTAL_VALID(fsr_hw_readl(FSR_PORTAL_STATUS_OFFSET+i*FSR_SIZE)) ) {
				fsr_hw_writel(FSR_PAC_OFFSET+i*FSR_SIZE, pac_val);
		}
	}
	rc = SKP_HW_CMN_OK;
	return rc;
}

static skp_hw_cmn_err_t ksr_set_populated_pacs( uint32_t pac_val)
{
	int i;
	skp_hw_cmn_err_t rc = SKP_HW_CMN_ERR_FAIL; 
	/* First 2 KSR Portals have their PACS fused via their descriptors -- ignore */
	for (i = 2; i < KSR_MAX_ROW_ADDR; i++) { 
		/* If portal is valid and descriptor is good, we have valid data so lock NS */
		if ( KSR_PORTAL_VALID(ksr_hw_readl( KSR_PORTAL_STATUS_OFFSET+i*KSR_SIZE)) &&
			KSR_RD_DGOOD(ksr_hw_readl( KSR_RD_DESC_OFFSET+i*KSR_SIZE))) {
				ksr_hw_writel( KSR_PAC_OFFSET+i*KSR_SIZE, pac_val);
		}
	}
	rc = SKP_HW_CMN_OK;
	return rc;
}

static skp_hw_cmn_err_t fsr_set_all_pacs( uint32_t pac_val)
{
	int i;
	skp_hw_cmn_err_t rc = SKP_HW_CMN_ERR_FAIL; 
	for (i = 0; i < FSR_MAX_ROW_ADDR - FSR_START_ROW_ADDR; i++) { 
		fsr_hw_writel(FSR_PAC_OFFSET+i*FSR_SIZE, pac_val);
	}
	rc = SKP_HW_CMN_OK;
	return rc;

}

static skp_hw_cmn_err_t ksr_set_all_pacs( uint32_t pac_val)
{
	int i;
	skp_hw_cmn_err_t rc = SKP_HW_CMN_ERR_FAIL; 
	/* First 2 KSR Portals have their PACS fused via their descriptors -- ignore */
	for (i = 2; i < KSR_MAX_ROW_ADDR; i++) { 
		ksr_hw_writel( KSR_PAC_OFFSET+i*KSR_SIZE, pac_val);
	}
	rc = SKP_HW_CMN_OK;
	return rc;

}

static skp_hw_cmn_err_t portal_lock(
			uint32_t addr, 
			skp_hw_cmn_perm_t perm,
			u8 lock)
{
	skp_hw_cmn_row_t  *row;
	skp_hw_cmn_err_t rc = SKP_HW_CMN_ERR_FAIL; 
	uint32_t curr_perm_mask = 0;
	uint32_t perm_mask = 0;
	if ( skp_lookup_row( addr, &row)) {
		rc = SKP_HW_CMN_ERR_UNSP;
		_ETR(rc); 
		goto err;
	}
	if (perm & SKP_HW_CMN_CTL_LOCK_PAC_NSRD) {
		perm_mask |= KSR_PERM_PAC_NSR_LOCK;
	}
   	if (perm & SKP_HW_CMN_CTL_LOCK_PAC_SRD) {
		perm_mask |= KSR_PERM_PAC_SR_LOCK;
	}
	if (perm & SKP_HW_CMN_CTL_LOCK_PAC_NSW) {
		perm_mask |= KSR_PERM_PAC_NSW_LOCK;
	}
	if (perm & SKP_HW_CMN_CTL_LOCK_PAC_SW) {
		perm_mask |= KSR_PERM_PAC_SW_LOCK;
	}
	if (perm & SKP_HW_CMN_CTL_LOCK_SRD) {
		perm_mask |= KSR_PERM_BLK_SR_LOCK;
	}
   	if (perm & SKP_HW_CMN_CTL_LOCK_NSRD) {
		perm_mask |= KSR_PERM_BLK_NSR_LOCK;
	}
   	if (perm & SKP_HW_CMN_CTL_LOCK_SW) {
		perm_mask |= KSR_PERM_BLK_SW_LOCK;
	}
	if (perm & SKP_HW_CMN_CTL_LOCK_NSW) {
		perm_mask |= KSR_PERM_BLK_NSW_LOCK;
	}
	if (perm_mask) {
		rc = SKP_HW_CMN_OK;
		if( IS_KSR_ADDR(row->addr) ) {
			curr_perm_mask = ksr_hw_readl(KSR_PAC_OFFSET + (row->addr-KSR_START_ROW_ADDR)*KSR_SIZE);
			if( lock ) {
				ksr_hw_writel(KSR_PAC_OFFSET + (row->addr-KSR_START_ROW_ADDR)*KSR_SIZE, 
					curr_perm_mask|perm_mask);
			} else {
				ksr_hw_writel(KSR_PAC_OFFSET + (row->addr-KSR_START_ROW_ADDR)*KSR_SIZE, 
					curr_perm_mask&(~perm_mask));
			}
		} else if(IS_FSR_ADDR(row->addr)) {
			curr_perm_mask = fsr_hw_readl(FSR_PAC_OFFSET + (row->addr-FSR_START_ROW_ADDR)*FSR_SIZE);
			if( lock ) {
				fsr_hw_writel(FSR_PAC_OFFSET + (row->addr-FSR_START_ROW_ADDR)*FSR_SIZE, 
					curr_perm_mask|perm_mask);
			} else {
				fsr_hw_writel(FSR_PAC_OFFSET + (row->addr-FSR_START_ROW_ADDR)*FSR_SIZE, 
					curr_perm_mask&(~perm_mask));
			}
		} else {
			rc = SKP_HW_CMN_ERR_INVAL; 
			_ETR(rc);
		}
	}
err:
	return rc;
}

static skp_hw_cmn_err_t multi_portal_lock(
			skp_hw_cmn_perm_t perm,
			u8 lock)
{
	skp_hw_cmn_err_t rc = SKP_HW_CMN_ERR_FAIL; 
	if( lock ) {
		if (perm & SKP_HW_CMN_CTL_LOCK_NS) {
			rc = ksr_set_all_pacs(KSR_PERM_LOCK_NS);
			rc |= fsr_set_all_pacs(KSR_PERM_LOCK_NS);
		} else if (perm & SKP_HW_CMN_CTL_LOCK_ALL) {
			rc = ksr_set_all_pacs(KSR_PERM_LOCK_ALL);
			rc |= fsr_set_all_pacs(KSR_PERM_LOCK_ALL);
		} else if (perm & SKP_HW_CMN_CTL_LOCK_S) {
			rc = ksr_set_all_pacs(KSR_PERM_LOCK_S);
			rc |= fsr_set_all_pacs(KSR_PERM_LOCK_S);
		} 
	} else {
		if (perm & SKP_HW_CMN_CTL_LOCK_NS || perm & SKP_HW_CMN_CTL_LOCK_NS_PROV ) {
			rc = ksr_set_all_pacs(KSR_PERM_UNLOCK_NS);
			rc |= fsr_set_all_pacs(KSR_PERM_UNLOCK_NS);
			if( perm & SKP_HW_CMN_CTL_LOCK_NS_PROV) {
				/* LOCK NS access for keys that are populated */
				rc = ksr_set_populated_pacs(KSR_PERM_LOCK_NS);
				rc |= fsr_set_populated_pacs(KSR_PERM_LOCK_NS);
			}
		} else if (perm & SKP_HW_CMN_CTL_LOCK_S) {
			rc = ksr_set_all_pacs(KSR_PERM_UNLOCK_S);
			rc |= fsr_set_all_pacs(KSR_PERM_UNLOCK_S);
		} else if (perm & SKP_HW_CMN_CTL_LOCK_ALL) {
			rc = ksr_set_all_pacs(KSR_PERM_UNLOCK_ALL);
			rc |= fsr_set_all_pacs(KSR_PERM_UNLOCK_ALL);
		} 
	}
	return rc;
}

static skp_hw_cmn_err_t portal_status(
			uint32_t addr,
			skp_hw_cmn_status_t status,
			uint32_t* res)
{
	skp_hw_cmn_err_t rc = SKP_HW_CMN_ERR_FAIL; 
	uint32_t perm_mask = 0;
        skp_hw_cmn_row_t  *row = NULL;

	if ( skp_lookup_row( addr, &row)) {
		rc = SKP_HW_CMN_ERR_UNSP;
		_ETR(rc); 
		goto err;
	} 

	if( IS_KSR_ADDR(row->addr) ) {
		perm_mask = ksr_hw_readl(KSR_PAC_OFFSET + 
			(row->addr-KSR_START_ROW_ADDR)*KSR_SIZE);
	} else if(IS_FSR_ADDR(row->addr)) {
		perm_mask = fsr_hw_readl(FSR_PAC_OFFSET + 
			(row->addr-FSR_START_ROW_ADDR)*KSR_SIZE);
	}
	
	if (!(perm_mask & KSR_PERM_PAC_NSR_LOCK)) {
		status &= ~SKP_HW_CMN_STATUS_NSRD_PAC_LOCKED;
	}
	if (!(perm_mask & KSR_PERM_PAC_SR_LOCK)) {
		status &= ~SKP_HW_CMN_STATUS_SRD_PAC_LOCKED;
	}
	if (!(perm_mask & KSR_PERM_PAC_NSW_LOCK)) {
		status &= ~SKP_HW_CMN_STATUS_NSW_PAC_LOCKED;
	}
	if (!(perm_mask & KSR_PERM_PAC_SW_LOCK)) {
		status &= ~SKP_HW_CMN_STATUS_SW_PAC_LOCKED;
	}
	if (!(perm_mask & KSR_PERM_BLK_SR_LOCK)) {
		status &= ~SKP_HW_CMN_STATUS_SRD_LOCKED;
	}
	if (!(perm_mask & KSR_PERM_BLK_NSR_LOCK)) {
		status &=  ~SKP_HW_CMN_STATUS_NSRD_LOCKED;
	}
	if (!(perm_mask & KSR_PERM_BLK_SW_LOCK)) {
		status &= ~SKP_HW_CMN_STATUS_SW_LOCKED;
	}
	if (!(perm_mask & KSR_PERM_BLK_NSW_LOCK)) {
		status &= ~SKP_HW_CMN_STATUS_NSW_LOCKED;
	}
	if( IS_KSR_ADDR(row->addr) ) {
		if ( !KSR_PORTAL_VALID(ksr_hw_readl(KSR_PORTAL_STATUS_OFFSET+(row->addr-KSR_START_ROW_ADDR)*KSR_SIZE)) ||
			!KSR_RD_DGOOD(ksr_hw_readl(KSR_RD_DESC_OFFSET+(row->addr-KSR_START_ROW_ADDR)*KSR_SIZE))) {
			status &= ~SKP_HW_CMN_STATUS_ROW_DATA_VALID;
		}
	} else if(IS_FSR_ADDR(row->addr)) {
		if ( !FSR_PORTAL_VALID(fsr_hw_readl(FSR_PORTAL_STATUS_OFFSET+(row->addr-FSR_START_ROW_ADDR)*FSR_SIZE))) {
			status &= ~SKP_HW_CMN_STATUS_ROW_DATA_VALID;
		}
	}
	*res = status;
	rc = SKP_HW_CMN_OK;
err:
	return rc;
}


/******************** SKP ACCESS FUNCTIONS ********************/
static  skp_hw_cmn_err_t skp_write( 
		uint32_t addr,
		skp_hw_cmn_row_conf_t* conf,
		const uint32_t* data,
		int size)
{
	skp_hw_cmn_err_t rc = SKP_HW_CMN_ERR_FAIL; 
        skp_hw_cmn_row_t *row;
	skp_hw_cmn_row_conf_t* row_conf;

	if (skp_lookup_row( addr, &row)) {
		rc = SKP_HW_CMN_ERR_UNSP;
		_ETR(rc);
		goto err;
	}

	if ((int)size <= 0 || (int)row->addr < 0) {
		rc = SKP_HW_CMN_ERR_INVAL; 
		_ETR(rc);
		goto err;
	}

	row_conf = conf?conf:&row->conf;
	if( IS_KSR_ADDR(row->addr) ) {
		rc = ksr_write( row, row_conf, (uint32_t*)data, size );
	} else if(IS_FSR_ADDR(row->addr)) {
		rc = fsr_write( row, row_conf, (uint32_t*)data, size );
	} else {
		rc = SKP_HW_CMN_ERR_INVAL; 
		_ETR(rc);
	}
err:
	_DPRT("%s: rc %d\n", __FUNCTION__, rc);
	return rc;

}

static skp_hw_cmn_err_t skp_read(
			uint32_t addr,
			skp_hw_cmn_row_conf_t* conf,
			uint32_t *data,
			int size)
{
	skp_hw_cmn_err_t rc = SKP_HW_CMN_ERR_FAIL; 
        skp_hw_cmn_row_t  *row;
	skp_hw_cmn_row_conf_t* row_conf;

	if( skp_lookup_row( addr, &row)) {
		rc = SKP_HW_CMN_ERR_UNSP;
		_ETR(rc); 
		goto err;
	}
	if ((int)size <= 0 || (int)row->addr < 0) {
		rc = SKP_HW_CMN_ERR_INVAL; 
		_ETR(rc);
		goto err;
	}

	row_conf = conf? conf : &row->conf;
	if( IS_KSR_ADDR(row->addr) ) {
		rc = ksr_read( row, row_conf, (uint32_t*)data, size );
	} else if(IS_FSR_ADDR(row->addr)) {
		rc = fsr_read( row, row_conf, (uint32_t*)data, size );
	} else {
		rc = SKP_HW_CMN_ERR_INVAL; 
		_ETR(rc);
	}
err:
	_DPRT("%s: rc %d\n", __FUNCTION__, rc);
	return rc;
}


static skp_hw_cmn_err_t skp_hw_lock(
			uint32_t addr, 
			skp_hw_cmn_perm_t perm,
			u8 lock)
{
	skp_hw_cmn_err_t rc = SKP_HW_CMN_ERR_FAIL; 
	switch ( perm ) {
		case SKP_HW_CMN_CTL_LOCK_NS:
		case SKP_HW_CMN_CTL_LOCK_NS_PROV:
		case SKP_HW_CMN_CTL_LOCK_ALL:
		case SKP_HW_CMN_CTL_LOCK_S:
			rc = multi_portal_lock( perm, lock);
		break;
		default:
			rc = portal_lock( addr, perm, lock);
	}
	return rc;
}


static skp_hw_cmn_err_t skp_hw_status(
			uint32_t addr,
			skp_hw_cmn_status_t status,
			uint32_t* res)
{

	skp_hw_cmn_err_t rc = SKP_HW_CMN_ERR_FAIL; 
	rc = portal_status( addr, status, res);
	return rc;
}

 skp_hw_cmn_err_t sotp_hw_cmn_ctl(const skp_hw_cmn_ctl_cmd_t *cmd,
			uint32_t* res)
{
	skp_hw_cmn_err_t rc = SKP_HW_CMN_ERR_UNSP;
	switch ((uint32_t)(cmd->ctl)) {
   		case SKP_HW_CMN_CTL_UNLOCK: {
				skp_hw_ctl_data_t* ctl_data = (skp_hw_ctl_data_t*)cmd->data;
				rc = skp_hw_lock( ctl_data->addr, ctl_data->perm, 0);
			}
			break;
		case SKP_HW_CMN_CTL_LOCK: {
				skp_hw_ctl_data_t* ctl_data = (skp_hw_ctl_data_t*)cmd->data;
				rc = skp_hw_lock( ctl_data->addr, ctl_data->perm, 1);
			}
			break;
		case SKP_HW_CMN_CTL_STATUS: {
				skp_hw_ctl_data_t* ctl_data = (skp_hw_ctl_data_t*)cmd->data;	
				rc = skp_hw_status( ctl_data->addr, ctl_data->status, res);
			}
			break;
		default :
			break;	
	}
	return rc;
	
}

static int skp_antirollback_lvl_ctl( uint32_t * lvl, int write )
{
	int rc = -1;
	uint32_t * pdata = NULL;
	uint32_t size = 0;
	uint32_t addr = 0;
	uint32_t current_lvl = 0;
	uint32_t num_lvls_word = sizeof(uint32_t)*8/SKP_ANTI_ROLLBACK_LVL_NUMBITS;
	uint32_t num_words = 0;
	uint32_t i,j,tmp_lvl;

	/* Get anti_rollback fields from S/OTP */
	rc = skp_lookup_feat(SKP_CONT_ID_ANTI_ROLLBACK, &addr, &size);

	if( rc || !size) {
		printk("%s: ERROR! Failed to map feat_id:%d\n", 
			__FUNCTION__, SKP_CONT_ID_ANTI_ROLLBACK);
		return rc;
	}

	pdata = kmalloc(size, GFP_KERNEL);
	if( !pdata ) {
		printk("%s: ERROR! Failed to alloc mem for feat_id:%d\n",
			__FUNCTION__, SKP_CONT_ID_ANTI_ROLLBACK);
		return rc;
	}

	rc = skp_read( addr, NULL, pdata, size);
	if (rc) {
		printk("%s: ERROR! Failed to retrieve antirollback level data! rc:%d\n",
			__FUNCTION__, rc);
		goto finish_read;	
	}

	/* Calculate level */
	num_words = size/sizeof(uint32_t);
	for( i=0; i<num_words; i++ ){
		for( j=0; j<num_lvls_word; j++) {
			if( pdata[i] & (SKP_ANTI_ROLLBACK_LVL_MASK << (j*SKP_ANTI_ROLLBACK_LVL_SHIFT))) {
				current_lvl++;
			} else {
				goto finish_read;
			}
		}
	}

finish_read:
	_DPRT("%s: anti-rollback level %d %d %d\n", __FUNCTION__, current_lvl, num_words, num_lvls_word);
	if( write ) {
		tmp_lvl = *lvl;
		/* Do boundary checks on requested level */
		if( tmp_lvl <= current_lvl ) {
			printk("%s: ERROR! Requested anti-rollback level %d is lower than current %d\n", __FUNCTION__, tmp_lvl, current_lvl);
			rc = -1;
			goto finish;
		}
		if( tmp_lvl > num_words * num_lvls_word ) { //FIXME BOUNDARY
			printk("%s: ERROR! Requested anti-rollback level %d is higher than maximum %d\n", __FUNCTION__, tmp_lvl, num_words * num_lvls_word );
			rc = -1;
			goto finish;
		}

		/* Encode new rollback level */
		for( i=0; i<num_words; i++ ){
			/* Batch fill the row if level exceeds it */
			if( tmp_lvl >= num_lvls_word )
			{
				/* adjust level */
				tmp_lvl -= num_lvls_word;

				/* If current word is filled continue to next word */
				if( pdata[i] == 0xFFFFFFFF )
					continue;
				else
					pdata[i] = 0xFFFFFFFF;
			}
			else
			{
				/* Selectively fill the row according to level */
				while( tmp_lvl )
				{
					pdata[i] |= SKP_ANTI_ROLLBACK_LVL_MASK << ((tmp_lvl-1)*SKP_ANTI_ROLLBACK_LVL_SHIFT);
					tmp_lvl--;
				}
			}
			if( tmp_lvl == 0 )
				break;
		}

		/* Commit the new anti-rollback level to S/OTP */
		rc = skp_write( addr, NULL, pdata, size);
		if(rc) {
			printk("%s: ERROR! Failed to commit anti-rollback level data! rc:%d\n", __FUNCTION__, rc);
			goto finish;	
		}
	} else {
		*lvl = current_lvl;
	}
finish:
	kfree(pdata);
	return rc;
}

int skp_get_keyslot_data( int section_num, char* skp_data, int data_len, int * result )
{
	*result = skp_read( KSR_START_ROW_ADDR + section_num, NULL, 
			(uint32_t *)skp_data, data_len );
	if( *result )
		return -1;
	else
		return 0;
}

int skp_set_keyslot_data( int section_num, char* skp_data, int data_len, int * result )
{
	*result = skp_write( KSR_START_ROW_ADDR + section_num, NULL, 
			(uint32_t *)skp_data, data_len );
	if( *result )
		return -1;
	else
		return 0;
}

int skp_set_keyslot_readlock(int section_num, int * result)
{
	*result = SKP_HW_CMN_OK;
	if( *result )
		return -1;
	else
		return 0;
}

int skp_get_keyslot_readlock_status(int section_num, int * result )
{
	*result = SKP_HW_CMN_OK;
	if( *result )
		return -1;
	else
		return 0;
}

int skp_dump_map( int * result )
{
	uint32_t i,j;
	uint32_t * skp_data;
	skp_hw_cmn_row_t* rows = sotp_rows;
	
	printk("+---------------+:\n");
	printk("| SOTP raw data |:\n");
	printk("+---------------+:\n\n");

	printk("Keyslot SOTP Sections:\n");
	for (i = 0; i <  sizeof(sotp_rows)/sizeof(skp_hw_cmn_row_t); i++) {
		if( rows[i].addr >= FSR_START_ROW_ADDR )
	                printk("    FSR Keyslot Section[%d]:\n", i);
		else
	                printk("    KSR Keyslot Section[%d]:\n", i);

		if (rows[i].reserved_name)
	                printk("    RESERVED: %s\n", rows[i].reserved_name);

		skp_data = kmalloc( rows[i].range * sizeof(uint32_t), GFP_KERNEL);
		*result = skp_read( rows[i].addr, NULL,
		      skp_data, rows[i].range * sizeof(uint32_t) );
		for (j = 0; j < rows[i].range; j++ ) {
			printk("        Row[%d]: 0x%08x\n", j, skp_data[j]);
		}
		kfree(skp_data);
	}
	*result = SKP_HW_CMN_OK;
	if( *result )
		return -1;
	else
		return 0;
}

int skp_set_rollback_lvl( uint32_t * lvl, int * result)
{
	SKP_BASE_CHK();
	*result =skp_antirollback_lvl_ctl( lvl, 1 );
	if( *result )
		return -1;
	else
		return 0;
}
int skp_get_rollback_lvl( uint32_t * lvl, int * result)
{
	SKP_BASE_CHK();
	*result = skp_antirollback_lvl_ctl( lvl, 0 );
	if( *result )
		return -1;
	else
		return 0;
}

int skp_get_info( SOTP_INFO_BLK * info_blk, int data_len, int * result )
{
	uint32_t addr,size;
	info_blk->version = SOTP_VERSION_SKP;

	*result = skp_lookup_feat(SKP_CONT_ID_FLD_ROE, &addr, &size);
	if( *result ) {
		printk("%s: ERROR! could not determine num_words_in_keyslot!\n", 
			__FUNCTION__);
		return *result;
	}
	info_blk->num_words_in_keyslot = size/sizeof(uint32_t);

	*result = skp_lookup_feat(SKP_CONT_ID_ANTI_ROLLBACK, &addr, &size);
	if( *result ) {
		printk("%s: ERROR! could not determine max_rollback_lvl!\n",
			__FUNCTION__);
		return *result;
	}
	info_blk->max_rollback_lvl = (size * 8) / SKP_ANTI_ROLLBACK_LVL_NUMBITS;

	*result = SKP_HW_CMN_OK;
	return 0;
}

int skp_init ( void * base_ptr )
{
	if( !base_ptr )
		return -1;
	
	skp_base_ptr = (char*)base_ptr;
	
	return 0;
}

