/* SPDX-License-Identifier: GPL-2.0+
 *
 * Copyright 2020 Broadcom Ltd.
 */
/**********************************************************************
 *	
 * sko_hw.c			 
 * Includes both secure and non-secure otp operations 
 *
 *	
 *********************************************************************	
 */

#include <common.h>
#include <linux/types.h>
#include <linux/io.h>
#include "linux/printk.h"
#include <asm/arch/misc.h>
#include "otp_map_cmn.h"
#include "skp.h"

//#define DBG_OTP

#ifdef DBG_OTP
#define _ETR(__p__)  printf("%s \n\t\t\t\t -> :%s :%d  code %d / 0x%x\n",__FILE__,__FUNCTION__,__LINE__,__p__,__p__)
#define _DPRT(...)  printf(__VA_ARGS__) 
#else
#define _ETR(__p__)
#define _DPRT(...)
#endif

static otp_hw_cmn_err_t  lookup_row(otp_hw_cmn_t* dev, 
	u32 addr,
	otp_hw_cmn_row_t** row)
{
	u32 i;
	otp_hw_cmn_row_t* rows = dev->rows;
	for (i = 0; i < dev->row_max; i++) {
		if (rows[i].addr == addr) {
			*row = &rows[i];
			return OTP_HW_CMN_OK;	
		}
	}
	_ETR(OTP_HW_CMN_ERR_UNSP);
	return OTP_HW_CMN_ERR_UNSP;
}

/******************** FSR ACCESS FUNCTIONS ********************/
__weak u32 fsr_hw_readl(void* dev, u32 offs)
{
	u32 val;
	otp_hw_cmn_t* hw_dev = (otp_hw_cmn_t*)dev; 
	val = readl(hw_dev->mm+FSR_OFFSET+offs);
	_DPRT("0x%p > 0x%x\n",(void*)(hw_dev->mm+offs+FSR_OFFSET), val);
	return val;
}

__weak void fsr_hw_writel(void* dev, u32 offs, u32 data)
{
	otp_hw_cmn_t* hw_dev = (otp_hw_cmn_t*)dev; 
	_DPRT("0x%p < 0x%x\n",(void*)(hw_dev->mm+offs+FSR_OFFSET), data);
	writel(data, hw_dev->mm+FSR_OFFSET+offs);
}

static  otp_hw_cmn_err_t fsr_poll_otp_status(otp_hw_cmn_t* dev, 
						u32 fsr_offs)
{
	int to = 10000, errc = 0; 

	do {
		u32 st = fsr_hw_readl(dev, FSR_OTP_STATUS_OFFSET+fsr_offs);
		_DPRT("Fsr status 0x%x\n",st);
		if (!FSR_OTP_STATUS_ACTIVE(fsr_hw_readl(dev, FSR_OTP_STATUS_OFFSET+fsr_offs) )) {
			break;
		}
		udelay(1);
		to--;
	} while(to);

	errc = FSR_OTP_STATUS_CMD_ERROR(fsr_hw_readl(dev,FSR_OTP_STATUS_OFFSET+fsr_offs));
	if (errc) {
		_DPRT("%s: ERROR: otp status err 0x%x\n",
			__FUNCTION__, fsr_hw_readl(dev, FSR_OTP_STATUS_OFFSET+fsr_offs));
	}
	return (!to || errc);
}

static otp_hw_cmn_err_t fsr_read(otp_hw_cmn_t* dev,
			otp_hw_cmn_row_t  *row,
			otp_hw_cmn_row_conf_t* row_conf,
			u32 *data,
			u32 size)
{
        int i;
	otp_hw_cmn_err_t rc = OTP_HW_CMN_ERR_FAIL; 
	u32 fsr_offs;
	u32 raw_read = 0;
	u32 desc = 0;

	fsr_offs = FSR_SIZE*(row->addr-FSR_START_ROW_ADDR);
	if (row_conf && row_conf->op_type != OTP_HW_CMN_CTL_OTPCMD_ECC) {
		_DPRT("%s %d \n\t\t\t\t Raw read!\n",
			__FUNCTION__,__LINE__);
		raw_read = 1;
	}

	/* 1 - Check portal status */
	/* Issue RELOAD_DESC command to FSR_PORTAL_FP0_OTP_COMMAND */
	fsr_hw_writel(dev, FSR_OTP_CMD_OFFSET+fsr_offs,FSR_OTP_CMD_RELOAD_DESC);
	/* Check cmd complete,FSR_PORTAL_FP0_OTP_STATUS active(31)=0 cmd_err(7:0)=0 */
	if (fsr_poll_otp_status(dev, fsr_offs)) {
		rc = OTP_HW_CMN_ERR_TMO; 
		_ETR(rc);
		goto err;
	}

	/* Check valid(20)==1, ready(21)==1 */
	if (!FSR_PORTAL_VALID(fsr_hw_readl(dev, FSR_PORTAL_STATUS_OFFSET+fsr_offs))) {
		/* Portal is empty --> return zeroed data */
		_DPRT("%s %d \n\t\t\t\t Empty Portal! portal_status 0x%x\n",
			__FUNCTION__,__LINE__,
			fsr_hw_readl(dev, FSR_PORTAL_STATUS_OFFSET+fsr_offs));
		memset(data, 0x0, size);
		rc = OTP_HW_CMN_OK; 
		goto err;
	}

	/* DESC status is in FSR_PORTAL_FP0_PORTAL_STATUS */
	/* RLOCK = 1 --> Cant read */
	if( FSR_PORTAL_RLOCK(fsr_hw_readl(dev, FSR_PORTAL_STATUS_OFFSET+fsr_offs)) ) {
		rc = OTP_HW_CMN_ERR_FAIL; 
		_DPRT("%s %d \n\t\t\t\t Cant Read! RLOCK is set!\n",
			__FUNCTION__,__LINE__);
		_ETR(rc);
		goto err;
	}

	/* RAW and ECC_PGM_DIS = 0 --> Cant raw read */
	if( raw_read && !FSR_PORTAL_ECC_DIS(fsr_hw_readl(dev, FSR_PORTAL_STATUS_OFFSET+fsr_offs))){
		rc = OTP_HW_CMN_ERR_FAIL; 
		_DPRT("%s %d \n\t\t\t\t Cant Read RAW! ECC_DIS is NOT set!\n",
			__FUNCTION__,__LINE__);
		_ETR(rc);
		goto err;
	}

	/* 2 - Read Decriptor to determine portal size */
	/* Issue READ_DESC command to FSR_PORTAL_FP0_OTP_COMMAND */
	fsr_hw_writel(dev, FSR_OTP_CMD_OFFSET+fsr_offs,FSR_OTP_CMD_READ_DESC);
	/* Check command complete */
	if (fsr_poll_otp_status(dev, fsr_offs)) {
		rc = OTP_HW_CMN_ERR_TMO; 
		_ETR(rc);
		goto err;
	}
	/* DESC is in FSR_PORTAL_FP0_DATA */
	/* Check valid(31)==1 , this should succeed as PORTAL status is good*/
	desc = fsr_hw_readl(dev, FSR_DATA_OFFSET+fsr_offs);
	if(!FSR_RD_DGOOD(desc)) {
		_ETR(rc);
		_DPRT("%s %d \n\t\t\t\t desc 0x%x\n",
			__FUNCTION__,__LINE__,
			fsr_hw_readl(dev, FSR_DATA_OFFSET+fsr_offs));
		rc = OTP_HW_CMN_ERR_FAIL; 
		goto err;
	}
	/* Check if requested data fits inside this FSR, check SIZE(23:12) */
	if(size/sizeof(u32) > FSR_RD_DDATA_SIZE(desc)) {
		_ETR(rc);
		_DPRT("%s %d \n\t\t\t\t size 0x%x > 0x%0x\n",
			__FUNCTION__,__LINE__, size/sizeof(u32), FSR_RD_DDATA_SIZE(desc));
		rc =  OTP_HW_CMN_ERR_BAD_PARAM; 
		goto err;
	}

	/* 3 - Read data */
	for (i = 0; i < size/sizeof(u32); i++ ) {
		/* Issue READ_DATA command - Set WORD SEL(15:4) to pick word# */
		fsr_hw_writel(dev, FSR_OTP_CMD_OFFSET+fsr_offs,
			(i<<FSR_PORTAL_OTP_CMD_WORD_SEL_SHIFT)|FSR_OTP_CMD_READ_DATA);
		/* Check command complete */
		if (fsr_poll_otp_status(dev, fsr_offs)) {
			rc = OTP_HW_CMN_ERR_TMO; 
			_ETR(rc);
			goto err;
		}

		/* Read data from FSR_PORTAL_FP0_DATA */
		data[i] = fsr_hw_readl(dev, FSR_DATA_OFFSET+fsr_offs);
		_DPRT("%s %d \n\t\t\t\t read data[%d] 0x%x \n",
			__FUNCTION__,__LINE__, i, data[i]);

		/* Optional - Read ECC from FSR_PORTAL_FP0_RD_ECC */
	}
	rc = OTP_HW_CMN_OK;
err:
	return rc;
}

static otp_hw_cmn_err_t fsr_write(otp_hw_cmn_t* dev,
			otp_hw_cmn_row_t  *row,
			otp_hw_cmn_row_conf_t* row_conf,
			u32 *data,
			u32 size)
{
        int i;
	otp_hw_cmn_err_t rc = OTP_HW_CMN_ERR_FAIL; 
	u32 fsr_offs; 
	otp_hw_cmn_ctl_cmd_t ctl = {.ctl = OTP_HW_CMN_CTL_OTPCMD_AUTH_PROG};
	u32 res = 0;
	u32 raw_write = 0;
	u32 desc = 0;
	otp_hw_cmn_t *ext_drv = dev->drv_ext;

	fsr_offs = FSR_SIZE*(row->addr-FSR_START_ROW_ADDR);
	if (row_conf && row_conf->op_type != OTP_HW_CMN_CTL_OTPCMD_ECC) {
		_DPRT("%s %d \n\t\t\t\t Raw write!\n",
			__FUNCTION__,__LINE__);
		raw_write = 1;
	}

	/* 1 - Place OTP in program mode */
	rc = ext_drv->ctl(ext_drv, &ctl, &res);
	if (rc) {
		_ETR(rc);
		goto err;
	}
	ctl.ctl = OTP_HW_CMN_CTL_OTPCMD_AUTH_PROG_DONE;
	rc = ext_drv->ctl(ext_drv, &ctl, &res);
	if (rc) {
		_ETR(rc);
		goto err;
	}

	/* 2 - Check portal status */
	/* Issue RELOAD_DESC command to FSR_PORTAL_FP0_OTP_COMMAND */
	fsr_hw_writel(dev, FSR_OTP_CMD_OFFSET+fsr_offs,FSR_OTP_CMD_RELOAD_DESC);
	/* Check cmd complete,FSR_PORTAL_FP0_OTP_STATUS active(31)=0 cmd_err(7:0)=0 */
	if (fsr_poll_otp_status(dev, fsr_offs)) {
		rc = OTP_HW_CMN_ERR_TMO; 
		_ETR(rc);
		goto err;
	}

	/* Check valid(20)==1, ready(21)==1*/
	if (!FSR_PORTAL_VALID(fsr_hw_readl(dev, FSR_PORTAL_STATUS_OFFSET+fsr_offs))) {
		/* If Portal DESC is not valid ---> empty!, program it */
		/* Set VALID = 1 and RLOCK = 0 */
		desc = FSR_RD_DESC_VALID_MASK<<FSR_RD_DESC_VALID_SHIFT;

		/* If RAW, set ECC_PGM_DIS */
		if( raw_write )
			desc |= FSR_RD_DESC_ECC_PGM_DIS_MASK<<FSR_RD_DESC_ECC_PGM_DIS_SHIFT;

		/* A - Write descriptor to FSR_PORTAL_FP0_DATA */
		fsr_hw_writel(dev, FSR_DATA_OFFSET+fsr_offs,desc);

		/* B - Issue PGM_DESC command to FSR_PORTAL_FP0_OTP_COMMAND */
		fsr_hw_writel(dev, FSR_OTP_CMD_OFFSET+fsr_offs,FSR_OTP_CMD_PGM_DESC);
		/* Check cmd complete,FSR_PORTAL_FP0_OTP_STATUS active(31)=0 cmd_err(7:0)=0 */
		if (fsr_poll_otp_status(dev, fsr_offs)) {
			rc = OTP_HW_CMN_ERR_TMO; 
			_ETR(rc);
			goto err;
		}

		/* C - Issue PGM_DESC_ECC command to FSR_PORTAL_FP0_OTP_COMMAND */
		fsr_hw_writel(dev, FSR_OTP_CMD_OFFSET+fsr_offs,FSR_OTP_CMD_PGM_DESC_ECC);
		/* Check cmd complete,FSR_PORTAL_FP0_OTP_STATUS active(31)=0 cmd_err(7:0)=0 */
		if (fsr_poll_otp_status(dev, fsr_offs)) {
			rc = OTP_HW_CMN_ERR_TMO; 
			_ETR(rc);
			goto err;
		}

		/* D - Issue RELOAD_DESC command to FSR_PORTAL_FP0_OTP_COMMAND */
		fsr_hw_writel(dev, FSR_OTP_CMD_OFFSET+fsr_offs,FSR_OTP_CMD_RELOAD_DESC);
		/* Check cmd complete,FSR_PORTAL_FP0_OTP_STATUS active(31)=0 cmd_err(7:0)=0 */
		if (fsr_poll_otp_status(dev, fsr_offs)) {
			rc = OTP_HW_CMN_ERR_TMO; 
			_ETR(rc);
			goto err;
		}
	} else {
		/* Valid PORTAL and DESC */
		/* DESC status is in FSR_PORTAL_FP0_PORTAL_STATUS */
		/* RLOCK = 1 --> Cant write */
		if( FSR_PORTAL_RLOCK(fsr_hw_readl(dev, FSR_PORTAL_STATUS_OFFSET+fsr_offs)) ) {
			rc = OTP_HW_CMN_ERR_WRITE_FAIL; 
			_DPRT("%s %d \n\t\t\t\t Cant Write! RLOCK is set!\n",
				__FUNCTION__,__LINE__);
			_ETR(rc);
			goto err;
		}

		/* RAW and ECC_PGM_DIS = 0 --> Cant raw write */
		if( raw_write && !FSR_PORTAL_ECC_DIS(fsr_hw_readl(dev, 
				FSR_PORTAL_STATUS_OFFSET+fsr_offs))) {
			rc = OTP_HW_CMN_ERR_WRITE_FAIL; 
			_DPRT("%s %d \n\t\t\t\t Cant Write RAW! ECC_DIS is NOT set!\n",
				__FUNCTION__,__LINE__);
			_ETR(rc);
			goto err;
		}

		/* NOT RAW and ECC_PGM_DIS = 1 --> Cant non-raw write without ECC */
		if( !raw_write && FSR_PORTAL_ECC_DIS(fsr_hw_readl(dev, 
				FSR_PORTAL_STATUS_OFFSET+fsr_offs))) {
			rc = OTP_HW_CMN_ERR_WRITE_FAIL; 
			_DPRT("%s %d \n\t\t\t\t Cant Write Non-raw! ECC_DIS is set!\n",
				__FUNCTION__,__LINE__);
			_ETR(rc);
			goto err;
		}
	}


	/* 3 - Read Decriptor to determine portal size */
	/* Issue READ_DESC command to FSR_PORTAL_FP0_OTP_COMMAND */
	fsr_hw_writel(dev, FSR_OTP_CMD_OFFSET+fsr_offs,FSR_OTP_CMD_READ_DESC);
	/* Check command complete */
	if (fsr_poll_otp_status(dev, fsr_offs)) {
		rc = OTP_HW_CMN_ERR_TMO; 
		_ETR(rc);
		goto err;
	}
	/* Read and check if descriptor is valid(31) */
	desc = fsr_hw_readl(dev, FSR_DATA_OFFSET+fsr_offs);
	if(!FSR_RD_DGOOD(desc)) {
		_ETR(rc);
		_DPRT("%s %d \n\t\t\t\t desc 0x%x\n",
			__FUNCTION__,__LINE__,
			fsr_hw_readl(dev, FSR_DATA_OFFSET+fsr_offs));
		rc = OTP_HW_CMN_ERR_FAIL; 
		goto err;
	}
	/* Check if requested data fits inside this FSR, check SIZE(23:12) */
	if(size/sizeof(u32) > FSR_RD_DDATA_SIZE(desc)) {
		_ETR(rc);
		_DPRT("%s %d \n\t\t\t\t size 0x%x > 0x%0x\n",
			__FUNCTION__,__LINE__, size/sizeof(u32), FSR_RD_DDATA_SIZE(desc));
		rc =  OTP_HW_CMN_ERR_BAD_PARAM; 
		goto err;
	}
	/* If not a raw_write, check if portal has been written to */
	if( !raw_write ) {
		for (i = 0; i < size/sizeof(u32); i++ ) {
			/* Issue READ_DATA command - Set WORD SEL(15:4) to pick word# */
			fsr_hw_writel(dev, FSR_OTP_CMD_OFFSET+fsr_offs,
				(i<<FSR_PORTAL_OTP_CMD_WORD_SEL_SHIFT)|FSR_OTP_CMD_READ_DATA);
			/* Check command complete */
			if (fsr_poll_otp_status(dev, fsr_offs)) {
				rc = OTP_HW_CMN_ERR_TMO; 
				_ETR(rc);
				goto err;
			}

			/* Read ECC from FSR_PORTAL_FP0_RD_ECC */
			if(fsr_hw_readl(dev,FSR_RD_ECC_OFFSET+fsr_offs) & FSR_PORTAL_RD_ECC_MASK) {
				rc = OTP_HW_CMN_ERR_WRITE_FAIL; 
				_DPRT("%s %d \n\t\t\t\t Cannot overwrite portal! ECC is set!\n",
					__FUNCTION__,__LINE__);
				_ETR(rc);
				goto err;
			}
		}
	}

	/* 4 - Write data and ECC ( if reqd ) */
	for (i = 0; i < size/sizeof(u32); i++ ) {
		/* Read data from FSR_PORTAL_FP0_DATA */
		_DPRT("%s %d \n\t\t\t\t write data[%d] 0x%x \n",
			__FUNCTION__,__LINE__, i, data[i]);

		fsr_hw_writel(dev, FSR_DATA_OFFSET+fsr_offs, data[i]);

		/* Issue WRITE_DATA command - Set WORD SEL(15:4) to pick word# */
		fsr_hw_writel(dev, FSR_OTP_CMD_OFFSET+fsr_offs,
			(i<<FSR_PORTAL_OTP_CMD_WORD_SEL_SHIFT)|FSR_OTP_CMD_PGM_DATA);
		/* Check command complete */
		if (fsr_poll_otp_status(dev, fsr_offs)) {
			rc = OTP_HW_CMN_ERR_TMO; 
			_ETR(rc);
			goto err;
		}

		if( !raw_write ) {		
			/* Issue WRITE_ECC command - Set WORD SEL(15:4) to pick word# */
			fsr_hw_writel(dev, FSR_OTP_CMD_OFFSET+fsr_offs,
				(i<<FSR_PORTAL_OTP_CMD_WORD_SEL_SHIFT)|FSR_OTP_CMD_PGM_DATA_ECC);
			/* Check command complete */
			if (fsr_poll_otp_status(dev, fsr_offs)) {
				rc = OTP_HW_CMN_ERR_TMO; 
				_ETR(rc);
				goto err;
			}
		}
	}
	
	_DPRT("Fused FSR rows as expected \n");
       	rc = OTP_HW_CMN_OK; 
err:
        return rc;
}

/******************** KSR ACCESS FUNCTIONS ********************/
__weak u32 ksr_hw_readl(void* dev, u32 offs)
{
	otp_hw_cmn_t* hw_dev = (otp_hw_cmn_t*)dev; 
	return readl(hw_dev->mm+KSR_OFFSET+offs);
}

__weak void ksr_hw_writel(void* dev, u32 offs, u32 data)
{
	otp_hw_cmn_t* hw_dev = (otp_hw_cmn_t*)dev; 
	_DPRT("0x%x < 0x%x\n",hw_dev->mm+offs, data);
	writel(data, hw_dev->mm+KSR_OFFSET+offs);
}

static  otp_hw_cmn_err_t ksr_poll_otp_status(otp_hw_cmn_t* dev, 
						u32 ksr_offs)
{
	int to = 10000, errc = 0; 

	do {
		u32 st = ksr_hw_readl(dev, KSR_OTP_STATUS_OFFSET+ksr_offs);
		_DPRT("Ksr status 0x%x\n",st);
		if (!KSR_OTP_STATUS_ACTIVE(ksr_hw_readl(dev, KSR_OTP_STATUS_OFFSET+ksr_offs) )) {
			break;
		}
		udelay(1);
		to--;
	} while(to);

	errc = KSR_OTP_STATUS_CMD_ERROR(ksr_hw_readl(dev,KSR_OTP_STATUS_OFFSET+ksr_offs));
	if (errc) {
		printf("%s: ERROR: otp status err 0x%x\n",
			__FUNCTION__, ksr_hw_readl(dev, KSR_OTP_STATUS_OFFSET+ksr_offs));
	}
	return (!to || errc);
}

static otp_hw_cmn_err_t ksr_read(otp_hw_cmn_t* dev,
			otp_hw_cmn_row_t  *row,
			otp_hw_cmn_row_conf_t* row_conf,
			u32 *data,
			u32 size)
{
        int i;
	otp_hw_cmn_err_t rc = OTP_HW_CMN_ERR_FAIL; 
	u32 ksr_offs;
	ksr_offs = KSR_SIZE*(row->addr-KSR_START_ROW_ADDR);
	if ( !KSR_PORTAL_VALID(ksr_hw_readl(dev, KSR_PORTAL_STATUS_OFFSET+ksr_offs)) ||
		!KSR_RD_DGOOD(ksr_hw_readl(dev, KSR_RD_DESC_OFFSET+ksr_offs))) {
		_ETR(rc);
		_DPRT("%s %d \n\t\t\t\t portal_status 0x%x rd_desc 0x%x\n",
			__FUNCTION__,__LINE__,ksr_hw_readl(dev, KSR_PORTAL_STATUS_OFFSET+ksr_offs), ksr_hw_readl(dev, KSR_RD_DESC_OFFSET+ksr_offs));
		rc = OTP_HW_CMN_ERR_KEY_EMPTY; 
		goto err;
	}
	for (i = 0; i <  size/sizeof(u32); i++ ) {
		data[i] = ksr_hw_readl(dev, KSR_DATA_OFFSET+i*sizeof(u32)+ksr_offs);
		_DPRT("%s %d \n\t\t\t\t data[%d] 0x%x \n",
			__FUNCTION__,__LINE__, i, data[i]);
	}
	rc = OTP_HW_CMN_OK;
err:
	return rc;

}

static otp_hw_cmn_err_t ksr_write(otp_hw_cmn_t* dev,
			otp_hw_cmn_row_t  *row,
			otp_hw_cmn_row_conf_t* row_conf,
			u32 *data,
			u32 size)
{
        int i;
	otp_hw_cmn_err_t rc = OTP_HW_CMN_ERR_FAIL; 
	u32 ksr_offs; 
	otp_hw_cmn_ctl_cmd_t ctl = {.ctl = OTP_HW_CMN_CTL_OTPCMD_AUTH_PROG};
	u32 res = 0;
	otp_hw_cmn_t *ext_drv = dev->drv_ext;
	ksr_offs = KSR_SIZE*(row->addr-KSR_START_ROW_ADDR);
	
/*
	1) Set OTP in program mode using classic CPU access as mentioned in Sec3.
	2) Write bit[31] (valid) to “1” and bit[7:0] PAC fields as required (refer RDB for definition) into KEY_PORTAL_KP0_PGM_DESCRIPTOR
	3) Issue PGM_DESCRIPTOR_WORD command, write bit[3:0] CMD field to “0x8”  in KEY_PORTAL_KP0_OTP_COMMAND, 
		this command programs into the OTP Macro one 32-bit Descriptor word. The data to be written comes from the KP_PGM_DESCRIPTOR register.
	4) Wait/Check for command_active to be low which indicates that the command issued is completed. 
		Poll for the register bit “command_active(bit [31])” in KEY_PORTAL_KP0_OTP_STATUS. 
		Once command is completed. Check for the bits[7:0] CMD_ERR (refer RDB for definition of the bits).
	5) Issue PGM_DESCRIPTOR_ECC command, write bit[3:0] CMD field to “0x9”  in KEY_PORTAL_KP0_OTP_COMMAND, 
		this command programs into the OTP Macro just the 10 ECC bits for the Descriptor row, 
		which the OTP Controller automatically calculates based on the OTP data already programmed in this row.
	6) Wait/Check for command_active to be low which indicates that the command issued is completed. 
		Poll for the register bit “command_active(bit [31])” in KEY_PORTAL_KP0_OTP_STATUS.
		Once command is completed. Check for the bits[7:0] CMD_ERR (refer RDB for definition of the bits).
	7) Write all the 8 Keys into KEY_PORTAL_KP0_DATA_0, KEY_PORTAL_KP0_DATA_1, KEY_PORTAL_KP0_DATA_2, KEY_PORTAL_KP0_DATA_3, 
		KEY_PORTAL_KP0_DATA_4, KEY_PORTAL_KP0_DATA_5, KEY_PORTAL_KP0_DATA_6, KEY_PORTAL_KP0_DATA_7 respectively.
	8) Issue PGM_KEY command, write bit[3:0] CMD field to “0x2”  in KEY_PORTAL_KP0_OTP_COMMAND, 
		this command programs (in the OTP Array) the entire key (including the CRC value) for this Key Portal. 
		The hardware internally calculates the CRC value for the key and programs it into the CRC OTP field for this key.
	9) Wait/Check for command_active to be low which indicates that the command issued is completed. 
		Poll for the register bit “command_active(bit [31])” in KEY_PORTAL_KP0_OTP_STATUS. 
		Once command is completed. Check for the bits[7:0] CMD_ERR (refer RDB for definition of the bits).
	10) Issue chip reset.
 
4.3 KSR PORTAL CPU READ ACCESS:

KSR sectioned region thru portal can be accessed only in secure-boot mode, i.e. bit 2 in Row13 and bit 28 in Row 14 should be “1”.

All 6 Key portals can be accessed in any order. Below sequence is explained by considering Key portal0 as an example same applies for other 5 Key portals as well.

	1) Upon chip reboot, design loads the KSR descriptor, checks for its validity, 
		and loads the keys into the registers once they are good.
	2) Read KEY_PORTAL_KP0_RD_DESCRIPTOR, check for bit[29] DGOOD, this should be “1” which indicates descriptor 
			validity checks passed and the also all 8 Keys are good with no CRC/ECC errors. 
		a. If DGOOD is “0”, all 8 keys read in Step(3) will be zeroes, KEY_PORTAL_KP0_PORTAL_STATUS 
			register will have more status info bits like Descriptor validity, Desc ECC error, Data ECC error, Keys loaded (Refer RDB).
	3) Read KEY_PORTAL_KP0_PAC register, this register initially gets loaded with the PAC values 
			from OTP but later can also be programmed, based on the PAC permissions Key portal registers can be accessed. Please refer RDB for more information. 
	4) Read all the 8 Keys which are loaded in KEY_PORTAL_KP0_DATA_0, KEY_PORTAL_KP0_DATA_1, KEY_PORTAL_KP0_DATA_2, KEY_PORTAL_KP0_DATA_3, KEY_PORTAL_KP0_DATA_4, KEY_PORTAL_KP0_DATA_5, KEY_PORTAL_KP0_DATA_6, KEY_PORTAL_KP0_DATA_7 respectively.

*/
	rc = ext_drv->ctl(ext_drv, &ctl, &res);
	if (rc) {
		_ETR(rc);
		goto err;
	}

	ctl.ctl = OTP_HW_CMN_CTL_OTPCMD_AUTH_PROG_DONE;
	rc = ext_drv->ctl(ext_drv, &ctl, &res);
	if (rc) {
		_ETR(rc);
		goto err;
	}

	_ETR((u32)row_conf->perm);
	_ETR(KSR_PORTAL_PGM_DESC_SRW_PERM);
	if (ksr_poll_otp_status(dev, ksr_offs)) {
		rc = OTP_HW_CMN_ERR_TMO; 
		_ETR(rc);
		goto err;
	}
	ksr_hw_writel(dev, KSR_PGM_DESC_OFFSET + ksr_offs, (u32)row_conf->perm);
	_ETR(ksr_hw_readl(dev, KSR_PGM_DESC_OFFSET+ksr_offs));
	ksr_hw_writel(dev, KSR_OTP_CMD_OFFSET + ksr_offs, KSR_PORTAL_OTP_CMD_PGM_DESC_WORD);
	if (ksr_poll_otp_status(dev, ksr_offs)) {
		rc = OTP_HW_CMN_ERR_TMO; 
		_ETR(rc);
		goto err;
	}
	_ETR(KSR_PORTAL_OTP_CMD_PGM_DESC_WORD);
	if ((row_conf->op_type&OTP_HW_CMN_CTL_OTPCMD_ECC)) {
		ksr_hw_writel(dev, KSR_OTP_CMD_OFFSET+ksr_offs,KSR_PORTAL_OTP_CMD_PGM_DESC_ECC);
		if (ksr_poll_otp_status(dev,ksr_offs)) {
			rc = OTP_HW_CMN_ERR_TMO; 
			_ETR(rc);
			goto err;
		}
		_ETR(KSR_PORTAL_OTP_CMD_PGM_DESC_ECC);
	}
	for (i = 0 ; i <  size/sizeof(u32); i++) {
		ksr_hw_writel(dev, KSR_DATA_OFFSET+ksr_offs+i*sizeof(u32), data[i]);
	}
	ksr_hw_writel(dev, KSR_OTP_CMD_OFFSET + ksr_offs, KSR_PORTAL_OTP_CMD_PGM_KEY);
	_ETR(ksr_hw_readl(dev, KSR_OTP_CMD_OFFSET+ksr_offs));
	if (ksr_poll_otp_status(dev, ksr_offs)) {
		rc = OTP_HW_CMN_ERR_TMO;
		_ETR(rc);
		goto err;
	}
	
	_DPRT("Fused KSR rows as expected \n");
       	rc = OTP_HW_CMN_OK; 
err:
        return rc;
}

static otp_hw_cmn_err_t ksr_set_populated_pacs(otp_hw_cmn_t *dev, u32 pac_val)
{
	int i;
	otp_hw_cmn_err_t rc = OTP_HW_CMN_ERR_FAIL; 
	/* First 2 KSR Portals have their PACS fused via their descriptors -- ignore */
	for (i = 2; i < KSR_MAX_ROW_ADDR; i++) { 
		/* If portal is valid and descriptor is good, we have valid data so lock NS */
		if ( KSR_PORTAL_VALID(ksr_hw_readl(dev, KSR_PORTAL_STATUS_OFFSET+i*KSR_SIZE)) &&
			KSR_RD_DGOOD(ksr_hw_readl(dev, KSR_RD_DESC_OFFSET+i*KSR_SIZE))) {
				ksr_hw_writel(dev, KSR_PAC_OFFSET+i*KSR_SIZE, pac_val);
		}
	}
	rc = OTP_HW_CMN_OK;
	return rc;
}

static otp_hw_cmn_err_t ksr_set_all_pacs(otp_hw_cmn_t *dev, u32 pac_val)
{
	int i;
	otp_hw_cmn_err_t rc = OTP_HW_CMN_ERR_FAIL; 
	/* First 2 KSR Portals have their PACS fused via their descriptors -- ignore */
	for (i = 2; i < KSR_MAX_ROW_ADDR; i++) { 
		ksr_hw_writel(dev, KSR_PAC_OFFSET+i*KSR_SIZE, pac_val);
	}
	rc = OTP_HW_CMN_OK;
	return rc;

}

static otp_hw_cmn_err_t ksr_portal_lock(otp_hw_cmn_t *dev,
			u32 addr, 
			otp_hw_cmn_perm_t perm,
			u8 lock)
{
	otp_hw_cmn_row_t  *row;
	otp_hw_cmn_err_t rc = OTP_HW_CMN_ERR_FAIL; 
	u32 curr_perm_mask = 0;
	u32 perm_mask = 0;
	if ( lookup_row(dev, addr, &row)) {
		rc = OTP_HW_CMN_ERR_UNSP;
		_ETR(rc); 
		goto err;
	}
	if (perm & OTP_HW_CMN_CTL_LOCK_PAC_NSRD) {
		perm_mask |= KSR_PERM_PAC_NSR_LOCK;
	}
   	if (perm & OTP_HW_CMN_CTL_LOCK_PAC_SRD) {
		perm_mask |= KSR_PERM_PAC_SR_LOCK;
	}
	if (perm & OTP_HW_CMN_CTL_LOCK_PAC_NSW) {
		perm_mask |= KSR_PERM_PAC_NSW_LOCK;
	}
	if (perm & OTP_HW_CMN_CTL_LOCK_PAC_SW) {
		perm_mask |= KSR_PERM_PAC_SW_LOCK;
	}
	if (perm & OTP_HW_CMN_CTL_LOCK_SRD) {
		perm_mask |= KSR_PERM_BLK_SR_LOCK;
	}
   	if (perm & OTP_HW_CMN_CTL_LOCK_NSRD) {
		perm_mask |= KSR_PERM_BLK_NSR_LOCK;
	}
   	if (perm & OTP_HW_CMN_CTL_LOCK_SW) {
		perm_mask |= KSR_PERM_BLK_SW_LOCK;
	}
	if (perm & OTP_HW_CMN_CTL_LOCK_NSW) {
		perm_mask |= KSR_PERM_BLK_NSW_LOCK;
	}
	if (perm_mask) {
		curr_perm_mask = ksr_hw_readl(dev, KSR_PAC_OFFSET + row->addr*KSR_SIZE);
		if( lock ) {
			ksr_hw_writel(dev, KSR_PAC_OFFSET + row->addr*KSR_SIZE, 
				curr_perm_mask|perm_mask);
		} else {
			ksr_hw_writel(dev, KSR_PAC_OFFSET + row->addr*KSR_SIZE, 
				curr_perm_mask&(~perm_mask));
		}

		rc = OTP_HW_CMN_OK;
	}
err:
	return rc;
}

static otp_hw_cmn_err_t ksr_multi_portal_lock(otp_hw_cmn_t *dev,
			otp_hw_cmn_perm_t perm,
			u8 lock)
{
	otp_hw_cmn_err_t rc = OTP_HW_CMN_ERR_FAIL; 
	if( lock ) {
		if (perm & OTP_HW_CMN_CTL_LOCK_NS) {
	  		rc = ksr_set_all_pacs(dev, KSR_PERM_LOCK_NS);
		} else if (perm & OTP_HW_CMN_CTL_LOCK_ALL) {
	  		rc = ksr_set_all_pacs(dev, KSR_PERM_LOCK_ALL);
		} else if (perm & OTP_HW_CMN_CTL_LOCK_S) {
	  		rc = ksr_set_all_pacs(dev, KSR_PERM_LOCK_S);
		} 
	} else {
		if (perm & OTP_HW_CMN_CTL_LOCK_NS || perm & OTP_HW_CMN_CTL_LOCK_NS_PROV ) {
	  		rc = ksr_set_all_pacs(dev, KSR_PERM_UNLOCK_NS);
			if( perm & OTP_HW_CMN_CTL_LOCK_NS_PROV) {
				/* LOCK NS access for keys that are populated */
				rc = ksr_set_populated_pacs(dev, KSR_PERM_LOCK_NS);
			}
		} else if (perm & OTP_HW_CMN_CTL_LOCK_S) {
	  		rc = ksr_set_all_pacs(dev, KSR_PERM_UNLOCK_S);
		} else if (perm & OTP_HW_CMN_CTL_LOCK_ALL) {
	  		rc = ksr_set_all_pacs(dev, KSR_PERM_UNLOCK_ALL);
		} 
	}
	return rc;
}

static otp_hw_cmn_err_t ksr_portal_status(otp_hw_cmn_t *dev,
			u32 addr,
			otp_hw_cmn_status_t status,
			u32* res)
{
	otp_hw_cmn_err_t rc = OTP_HW_CMN_ERR_FAIL; 
	u32 perm_mask = 0,ksr_offs;
        otp_hw_cmn_row_t  *row = NULL;

	if ((status&(OTP_HW_CMN_STATUS_NS_LOCKED|OTP_HW_CMN_STATUS_S_LOCKED|OTP_HW_CMN_STATUS_ROW_W_LOCKED))){
		rc = OTP_HW_CMN_ERR_UNSP;
		_ETR(rc); 
		goto err;
	}

	if ( lookup_row(dev, addr, &row)) {
		rc = OTP_HW_CMN_ERR_UNSP;
		_ETR(rc); 
		goto err;
	} 

	ksr_offs = KSR_SIZE*(row->addr-KSR_START_ROW_ADDR);
	perm_mask = ksr_hw_readl(dev, KSR_PAC_OFFSET+ksr_offs);
	
	if (!(perm_mask & KSR_PERM_PAC_NSR_LOCK)) {
   		status &= ~OTP_HW_CMN_STATUS_NSRD_PAC_LOCKED;
	}
	if (!(perm_mask & KSR_PERM_PAC_SR_LOCK)) {
   		status &= ~OTP_HW_CMN_STATUS_SRD_PAC_LOCKED;
	}
	if (!(perm_mask & KSR_PERM_PAC_NSW_LOCK)) {
		status &= ~OTP_HW_CMN_STATUS_NSW_PAC_LOCKED;
	}
	if (!(perm_mask & KSR_PERM_PAC_SW_LOCK)) {
		status &= ~OTP_HW_CMN_STATUS_SW_PAC_LOCKED;
	}
	if (!(perm_mask & KSR_PERM_BLK_SR_LOCK)) {
		status &= ~OTP_HW_CMN_STATUS_SRD_LOCKED;
	}
	if (!(perm_mask & KSR_PERM_BLK_NSR_LOCK)) {
   		status &=  ~OTP_HW_CMN_STATUS_NSRD_LOCKED;
	}
	if (!(perm_mask & KSR_PERM_BLK_SW_LOCK)) {
   		status &= ~OTP_HW_CMN_STATUS_SW_LOCKED;
	}
	if (!(perm_mask & KSR_PERM_BLK_NSW_LOCK)) {
		status &= ~OTP_HW_CMN_STATUS_NSW_LOCKED;
	}
	if (!KSR_PORTAL_VALID(ksr_hw_readl(dev, KSR_PORTAL_STATUS_OFFSET+ksr_offs)) ||
		!KSR_RD_DGOOD(ksr_hw_readl(dev, KSR_RD_DESC_OFFSET+ksr_offs))) {
		status &= ~OTP_HW_CMN_STATUS_ROW_DATA_VALID;
	}
	if (KSR_PORTAL_READY(ksr_hw_readl(dev, KSR_PORTAL_STATUS_OFFSET+ksr_offs))) {
		status &= ~OTP_HW_CMN_STATUS_ROW_RD_LOCKED;
	}
	*res = status;
	rc = OTP_HW_CMN_OK;
err:
	return rc;
}


/******************** SKP ACCESS FUNCTIONS ********************/
static  otp_hw_cmn_err_t skp_write(otp_hw_cmn_t* dev, 
		u32 addr,
		otp_hw_cmn_row_conf_t* conf,
		const u32* data,
		u32 size)
{
	otp_hw_cmn_err_t rc = OTP_HW_CMN_ERR_FAIL; 
        otp_hw_cmn_row_t *row;
	otp_hw_cmn_row_conf_t* row_conf;
	if (!dev->drv_ext) {
		rc = OTP_HW_CMN_ERR_INVAL;
		_ETR(rc);
		goto err;
	}
	if (lookup_row(dev, addr, &row)) {
		rc = OTP_HW_CMN_ERR_UNSP;
		_ETR(rc);
		goto err;
	}

	if ((int)size <= 0 || (int)size > row->range*sizeof(u32) || (int)row->addr < 0) {
		rc = OTP_HW_CMN_ERR_INVAL; 
		_ETR(rc);
		goto err;
	}

	row_conf = conf?conf:&row->conf;
	if( (int)row->addr < KSR_MAX_ROW_ADDR ) {
		rc = ksr_write( dev, row, row_conf, (u32*)data, size );
	} else if(((int)row->addr >=FSR_START_ROW_ADDR) && ((int)row->addr < FSR_MAX_ROW_ADDR)) {
		rc = fsr_write( dev, row, row_conf, (u32*)data, size );
	} else {
		rc = OTP_HW_CMN_ERR_INVAL; 
		_ETR(rc);
	}
err:
	_DPRT("%s: rc %d\n", __FUNCTION__, rc);
	return rc;

}

static otp_hw_cmn_err_t skp_read(otp_hw_cmn_t *dev,
			u32 addr,
			otp_hw_cmn_row_conf_t* conf,
			u32 *data,
			u32 size)
{
	otp_hw_cmn_err_t rc = OTP_HW_CMN_ERR_FAIL; 
        otp_hw_cmn_row_t  *row;
	otp_hw_cmn_row_conf_t* row_conf;
	if (!dev->drv_ext) {
		rc = OTP_HW_CMN_ERR_INVAL; 
		_ETR(rc);
		goto err;
	}
	if( lookup_row(dev, addr, &row)) {
		rc = OTP_HW_CMN_ERR_UNSP;
		_ETR(rc); 
		goto err;
	}
	if ((int)size <= 0 || (int)size > row->range*sizeof(u32) || (int)row->addr < 0) {
		rc = OTP_HW_CMN_ERR_INVAL; 
		_ETR(rc);
		goto err;
	}

	row_conf = conf? conf : &row->conf;
	if( (int)row->addr < KSR_MAX_ROW_ADDR ) {
		rc = ksr_read( dev, row, row_conf, (u32*)data, size );
	} else if(((int)row->addr >=FSR_START_ROW_ADDR) && ((int)row->addr < FSR_MAX_ROW_ADDR)) {
		rc = fsr_read( dev, row, row_conf, (u32*)data, size );
	} else {
		rc = OTP_HW_CMN_ERR_INVAL; 
		_ETR(rc);
	}
err:
	_DPRT("%s: rc %d\n", __FUNCTION__, rc);
	return rc;
}

static  otp_hw_cmn_err_t skp_hw_write(otp_hw_cmn_t* dev, 
		u32 addr,
		const u32* data,
		u32 size)
{
	otp_hw_cmn_row_conf_t *cfg =  
				!(dev->ctl_cmd.ctl&OTP_HW_CMN_CTL_CONF)? NULL: &dev->row_conf;
	_ETR(dev->ctl_cmd.ctl);	
	return skp_write(dev, addr, cfg, data, size);
}

static otp_hw_cmn_err_t skp_hw_read(otp_hw_cmn_t *dev,
			u32 addr,
			u32 *data,
			u32 size)
{

	otp_hw_cmn_row_conf_t *cfg =  
			  !(dev->ctl_cmd.ctl&OTP_HW_CMN_CTL_CONF)? NULL : &dev->row_conf;
	_ETR(dev->ctl_cmd.ctl);	
	return skp_read(dev, addr, cfg, data, size);

}

static otp_hw_cmn_err_t skp_hw_lock(otp_hw_cmn_t *dev,
			u32 addr, 
			otp_hw_cmn_perm_t perm,
			u8 lock)
{
	otp_hw_cmn_err_t rc = OTP_HW_CMN_ERR_FAIL; 
	switch ( perm ) {
		case OTP_HW_CMN_CTL_LOCK_NS:
		case OTP_HW_CMN_CTL_LOCK_NS_PROV:
		case OTP_HW_CMN_CTL_LOCK_ALL:
		case OTP_HW_CMN_CTL_LOCK_S:
			/* TODO: Add FSR support */
			rc = ksr_multi_portal_lock(dev, perm, lock);
		break;
		default:
			/* TODO: Add FSR support */
			rc = ksr_portal_lock(dev, addr, perm, lock);
	}
	return rc;
}


static otp_hw_cmn_err_t skp_hw_status(otp_hw_cmn_t *dev,
			u32 addr,
			otp_hw_cmn_status_t status,
			u32* res)
{

	otp_hw_cmn_err_t rc = OTP_HW_CMN_ERR_FAIL; 
	/* TODO: Add FSR support */
	rc = ksr_portal_status(dev, addr, status, res);
	return rc;
}

__weak otp_hw_cmn_err_t sotp_hw_cmn_ctl(otp_hw_cmn_t *dev, 
			const otp_hw_cmn_ctl_cmd_t *cmd,
			u32* res)
{
	otp_hw_cmn_err_t rc = OTP_HW_CMN_ERR_UNSP;
	switch ((u32)(cmd->ctl)) {
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
   		case OTP_HW_CMN_CTL_UNLOCK: {
				otp_hw_ctl_data_t* ctl_data = (otp_hw_ctl_data_t*)cmd->data;
				rc = skp_hw_lock(dev, ctl_data->addr, ctl_data->perm, 0);
			}
			break;
		case OTP_HW_CMN_CTL_LOCK: {
				otp_hw_ctl_data_t* ctl_data = (otp_hw_ctl_data_t*)cmd->data;
				rc = skp_hw_lock(dev, ctl_data->addr, ctl_data->perm, 1);
			}
			break;
   		case OTP_HW_CMN_CTL_STATUS: {
				otp_hw_ctl_data_t* ctl_data = (otp_hw_ctl_data_t*)cmd->data;	
				rc = skp_hw_status(dev, ctl_data->addr, ctl_data->status, res);
			}
			break;
		default :
			break;	
	}
	return rc;
	
}


__weak otp_hw_cmn_err_t sotp_hw_dev_mmap(otp_hw_cmn_t* dev)
{
	dev->mm = KSR_BASE;
	return OTP_HW_CMN_OK;
}

__weak otp_hw_cmn_err_t sotp_hw_init(otp_hw_cmn_t* dev)
{
	DEFINE_SOTP_MAP_ROW_INITLR(rows);
	if (sotp_hw_dev_mmap(dev)) {
		return OTP_HW_CMN_ERR_FAIL;
	}
	dev->rows = rows;
	dev->row_max = sizeof(rows)/sizeof(otp_hw_cmn_row_t);
	return OTP_HW_CMN_OK;
}


otp_hw_cmn_err_t sotp_hw_cmn_init(otp_hw_cmn_t* dev)
{
	if (sotp_hw_init(dev)) {
		_ETR(OTP_HW_CMN_ERR_FAIL);
		return OTP_HW_CMN_ERR_FAIL;
	}
	dev->read = skp_hw_read;
	dev->write = skp_hw_write;
	dev->ctl = sotp_hw_cmn_ctl;
	return OTP_HW_CMN_OK;
}
