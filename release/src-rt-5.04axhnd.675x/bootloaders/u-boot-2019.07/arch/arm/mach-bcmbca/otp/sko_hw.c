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
#include "sko.h"


//#define DBG_OTP

#ifdef DBG_OTP
#define _ETR(__p__)  printf("%s \n\t\t\t\t -> %s:%d error code %d\n",__FILE__,__FUNCTION__,__LINE__,__p__)
#define _TR(__p__)  printf("%s \n\t\t\t\t -> %s:%d  %d / 0x%x\n",__FILE__,__FUNCTION__,__LINE__,__p__,__p__)
#define _DPRT(...)  printf(__VA_ARGS__) 
#else
#define _ETR(__p__)
#define _TR(__p__)
#define _DPRT(...)
#endif
/* Descriptor fields for  SKO objects 
Row:42 SKO_0 
		desc=0x9B009000 :(ecc which calculated at run-time) 
			7-bit_ECC=0x46, VALID=1, Rsvd=2'b0, WLOCK=1, 
			RLOCK=1, SEND_ECC=0, ECC_EN=1, FOUT=1, 
			12-bit_LAST_PTR=0x9, 12-bit_FIRST_PTR=0x0
Row 46 CRC
*/
/*
 * This a debug SKO_1 if needed for the test
Row:43 SKO_1 
*	desc=0x9B01300A : 7-bit_ECC=0x4E, VALID=1, Rsvd=2'b0, 
*			WLOCK=1, RLOCK=1, 
*			SEND_ECC=0, ECC_EN=1, FOUT=1, 
*			12-bit_LAST_PTR=0x13,12-bit_FIRST_PTR=0xA
*
*
*    	{{43, 0x0, 0xffffffff, 0, 1, 0, (void*)0x8101300A}, 
*        {56, 0x0, 0xffffffff, 0, 1, 0, (void*)0x80000000}}
*/
__weak u32 sko_hw_readl(void* dev, u32 offs)
{
	otp_hw_cmn_t* hw_dev = (otp_hw_cmn_t*)dev;
	return readl(hw_dev->mm+offs);
}

__weak void sko_hw_writel(void* dev, u32 offs, u32 data)
{
	otp_hw_cmn_t* hw_dev = (otp_hw_cmn_t*)dev; 
	_DPRT("0x%x < 0x%x\n",hw_dev->mm+offs, data);
	writel(data, hw_dev->mm + offs);
}
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
		if (rows[i].addr == addr) {
			*row = &rows[i];
			return OTP_HW_CMN_OK;	
		}
	}
	return OTP_HW_CMN_ERR_UNSP;
}
#if !defined(CONFIG_SPL_BUILD) || defined (CONFIG_BCMBCA_BOARD_TK_PROG)
static inline u32 _bit(u32 v, u32 no) {
		return ((v >> no)&0x1);
}
static inline u8 ecc7(u32 v) 
{
		return (u8) ((_bit(v,0)  ^ _bit(v,1)  ^ _bit(v,2)  ^ _bit(v,3)  ^ _bit(v,4)  ^ _bit(v,5)  ^ _bit(v,6)  ^ _bit(v,7)  ^ _bit(v,14) ^ _bit(v,19) ^ _bit(v,22) ^ _bit(v,24) ^ _bit(v,30) ^ _bit(v,31) ) |
          ((_bit(v,4)  ^ _bit(v,7)  ^ _bit(v,8)  ^ _bit(v,9)  ^ _bit(v,10) ^ _bit(v,11) ^ _bit(v,12) ^ _bit(v,13) ^ _bit(v,14) ^ _bit(v,15) ^ _bit(v,18) ^ _bit(v,21) ^ _bit(v,24) ^ _bit(v,29))<<0x1) |
          ((_bit(v,3)  ^ _bit(v,11) ^ _bit(v,16) ^ _bit(v,17) ^ _bit(v,18) ^ _bit(v,19) ^ _bit(v,20) ^ _bit(v,21) ^ _bit(v,22) ^ _bit(v,23) ^ _bit(v,26) ^ _bit(v,27) ^ _bit(v,29) ^ _bit(v,30))<<0x2) |
          ((_bit(v,2)  ^ _bit(v,6)  ^ _bit(v,10) ^ _bit(v,13) ^ _bit(v,15) ^ _bit(v,16) ^ _bit(v,24) ^ _bit(v,25) ^ _bit(v,26) ^ _bit(v,27) ^ _bit(v,28) ^ _bit(v,29) ^ _bit(v,30) ^ _bit(v,31))<<0x3) |
          ((_bit(v,1)  ^ _bit(v,2)  ^ _bit(v,5)  ^ _bit(v,7)  ^ _bit(v,9)  ^ _bit(v,12) ^ _bit(v,15) ^ _bit(v,20) ^ _bit(v,21) ^ _bit(v,22) ^ _bit(v,23) ^ _bit(v,25) ^ _bit(v,26) ^ _bit(v,28))<<0x4) |
          ((_bit(v,0)  ^ _bit(v,5)  ^ _bit(v,6)  ^ _bit(v,8)  ^ _bit(v,12) ^ _bit(v,13) ^ _bit(v,14) ^ _bit(v,16) ^ _bit(v,17) ^ _bit(v,18) ^ _bit(v,19) ^ _bit(v,20) ^ _bit(v,28))<<0x5) |
          ((_bit(v,0)  ^ _bit(v,1)  ^ _bit(v,3)  ^ _bit(v,4)  ^ _bit(v,8)  ^ _bit(v,9)  ^ _bit(v,10) ^ _bit(v,11) ^ _bit(v,17) ^ _bit(v,23) ^ _bit(v,25) ^ _bit(v,27) ^ _bit(v,31))<<0x6));
};


/* writes row with ecc. Calls for an external driver
 * to do the job
 * */
static inline int write_row(otp_hw_cmn_t* dev, 
		u32 addr, 
		otp_hw_cmn_row_conf_t* row_conf,
		u32 data)
{
	u32	d[2] = {data, 0};
	u32	size = sizeof(u32);
	if (row_conf->op_type == OTP_HW_CMN_CTL_OTPCMD_ECC) {
		d[1] = ecc7(d[0]);
		size *= 2;
	}
        _DPRT("row: %d data: 0x%x ecc: 0x%x\n", addr, d[0], d[1]);
        return dev->drv_ext->write_ex(dev->drv_ext, 
			addr, row_conf, (const u32*)d, size);
}

static inline u32 get_crc32(u32 v, u32 crc)
{
        v = ntohl(v);
	/* uboot crc32 negates results this however implemented assuming otherwise*/
     	if (crc == 0) {
		crc = 0xffffffff;
     	}
	/* for this implementation chosen no complement crc32 as exepcted by crc calc for 
 	* sko object
 	* */
	return  crc32_no_comp(crc, (u8*)&v, sizeof(u32));
}

/* Writes full or partial key object at once*/  

static  otp_hw_cmn_err_t sko_hw_write(otp_hw_cmn_t* dev, 
		u32 addr,
		const u32* data,
		u32 size)
{
        int i;
	otp_hw_cmn_err_t rc = OTP_HW_CMN_ERR_FAIL; 
	u32 *d = (u32*)data;
	u32 crc = 0;
        otp_hw_cmn_row_t *row, *row_ctl, *row_desc;
	rc = lookup_row(dev, addr, &row);
	if (rc) {
		goto err;
	}

	u32 sko_sect = SKO_ADDR2SECT(row->addr);
	if (sko_sect == SKO_NON_SECT) {
		int i; 
		if (size > (row->range*sizeof(u32)) || 
			!dev->drv_ext->write_ex)	{
			rc = OTP_HW_CMN_ERR_UNSP;
			goto err;
		}
		for (i = 0; i < size/sizeof(u32); i++) {	
        		rc = dev->drv_ext->write_ex(dev->drv_ext, 
				addr+i, &row->conf, (const u32*)&d[i], sizeof(u32));
			if (rc) {
				goto err;
			}
		}
		return rc;
	}

	rc = lookup_row(dev, row->conf.arg, &row_ctl);
	if (rc) {
		goto err;
	}
	row_desc = row_ctl + 1;

        /* order is important; */
        /* 1. Fuse a control data for the SKO */
        rc = write_row(dev, row_ctl->addr, 
			&row_ctl->conf, row_ctl->conf.arg); 
	if (rc){
            goto err;
        }
	
	crc = get_crc32((u32)row_ctl->conf.arg, 0);
        /* otp data*/
	for (i = 0; i < row->range; i++) {
		rc = write_row(dev, row->addr + i, 
			&row->conf, d[i]);
		if (rc){
               		goto err;
            	}
		crc = get_crc32(d[i], crc);
        }
        crc = (~crc);
	/* crc over ctl and data - fuse */
        rc = write_row(dev, row->addr + i,
			 &row->conf, crc);
	if (rc) {
		goto err;
	}
	_DPRT ("final crc 0x%x\n",crc);
        /* descriptor;  must be written at last; 
 	*it can have config with  enabled data ecc and WR LOCK  */
        rc = write_row(dev, row_desc->addr, 
			&row_desc->conf, row_desc->conf.arg); 
	if (rc) {
		goto err; 
	}
	rc = OTP_HW_CMN_OK; 
err:
	return rc;
}
#else
static  otp_hw_cmn_err_t sko_hw_write(otp_hw_cmn_t* dev, 
		u32 addr,
		const u32* data,
		u32 size)
{
	return OTP_HW_CMN_OK; 
}
#endif

static otp_hw_cmn_err_t sko_hw_read(otp_hw_cmn_t *dev,
			u32 addr,
			u32 *data,
			u32 size)
{
        otp_hw_cmn_row_t *row;
	otp_hw_cmn_err_t  rc = OTP_HW_CMN_ERR_FAIL;
	u32 sko_sect, sko_offs; 
	int i;
	rc = lookup_row(dev, addr, &row);
	if (rc) {
		_ETR(addr);
		goto err;
	}
	
	sko_sect = SKO_ADDR2SECT(row->addr);
	_TR(sko_sect);
	_TR(row->addr);
	if ( sko_sect >= SKO_SECT_MAX ) {
		int i;	
		if (size > row->range*sizeof(u32)) {
			rc = OTP_HW_CMN_ERR_INVAL;
			goto err;
		}
		if (sko_sect != SKO_NON_SECT ||
			!dev->drv_ext->read ) {
			rc = OTP_HW_CMN_ERR_UNSP;
			_ETR(addr); 
			goto err; 
		}
		for (i = 0; i <  size/sizeof(u32); i++) {	
			rc = dev->drv_ext->read(dev->drv_ext, 
					row->addr+i, &data[i], size);
			if (rc) {
				goto err;
			}
		}
		return rc;
	}
	sko_offs = sko_sect*SKO_SIZE;
	_DPRT("0x%x status 0x%x\n",sko, sko_hw_readl(dev, sko_hw_readl(dev, SKO_KEYN_STATUS_OFFSET + sko_offs)));
	if ( !SKO_KEY_DATA_GOOD(sko_hw_readl(dev, SKO_KEYN_STATUS_OFFSET + sko_offs))) {
		goto err;
	}
	for (i = 0; i <  size/sizeof(u32); i ++) {
		data[i] = sko_hw_readl(dev, sko_offs + SKO_KEYN_DATA_OFFSET + i*sizeof(u32));
		_DPRT(" 0x%x\n",  data[i]);
	}
	rc = OTP_HW_CMN_OK;
err:
	return rc;	
}

/* return status of the key object - valid or not
 *
 * Various PAC lock modes if selected
 * */
static otp_hw_cmn_err_t status(otp_hw_cmn_t *dev,
			u32 addr,
			otp_hw_cmn_status_t status,
			u32* res)
{
	otp_hw_cmn_err_t rc = OTP_HW_CMN_ERR_UNSP; 
	u32 sko_sect,sko_offs; 
        otp_hw_cmn_row_t  *row = NULL;
	u32 perm_mask = 0;
	if ( lookup_row(dev, addr, &row)) {
		_ETR(addr); 
		goto err;
	}
	if ((status&(OTP_HW_CMN_STATUS_NS_LOCKED|OTP_HW_CMN_STATUS_S_LOCKED|\
			OTP_HW_CMN_STATUS_ROW_W_LOCKED))){
		rc = OTP_HW_CMN_ERR_UNSP;
		_ETR(rc); 
		goto err;
	}

	sko_sect = SKO_ADDR2SECT(row->addr);
	_DPRT("%s sect %d row %d \n",__FUNCTION__,sko_sect,row->addr); 
	if ( sko_sect >= SKO_SECT_MAX) {
		if (sko_sect == SKO_NON_SECT) {
			u32 data = 0;
			if (!(status & OTP_HW_CMN_STATUS_ROW_DATA_VALID) || 
				!dev->drv_ext->read) {
				_ETR(addr); 
				goto err; 
			}
			rc = dev->drv_ext->read(dev->drv_ext, 
					row->addr, &data, sizeof(u32));
			if (rc) {
				_ETR(addr); 
				goto err;	
			}
			_TR(data);
			if (!data) {
				status &= ~OTP_HW_CMN_STATUS_ROW_DATA_VALID;
			}
			*res = status;
			return 	OTP_HW_CMN_OK;
		} else {
			rc = OTP_HW_CMN_ERR_INVAL;
			_ETR(sko_sect); 
			goto err; 
		}
	}

	sko_offs = sko_sect*SKO_SIZE;	
	perm_mask = (sko_hw_readl(dev, SKO_KEYN_PAC_CTRL_OFFSET+sko_offs)&SKO_PAC_MASK);
	
	/*for an SKO PERM_PAC bit if set means unlocked(enabled) */
	/* clearing bit OTP_HW_CMN_STATUS_NSRD_PAC_LOCKED if was requested*/
	if (perm_mask & SKO_PERM_PAC_NSR_LOCK) {
   		status &= ~OTP_HW_CMN_STATUS_NSRD_PAC_LOCKED;
	}
	if (perm_mask & SKO_PERM_PAC_SR_LOCK) {
   		status &= ~OTP_HW_CMN_STATUS_SRD_PAC_LOCKED;
	}
	if (perm_mask & SKO_PERM_PAC_NSW_LOCK) {
		status &= ~OTP_HW_CMN_STATUS_NSW_PAC_LOCKED;
	}
	if (perm_mask & SKO_PERM_PAC_SW_LOCK) {
		status &= ~OTP_HW_CMN_STATUS_SW_PAC_LOCKED;
	}
	/*for an SKO PERM_KEY bit if set means locked(disabled) */
	if (!(perm_mask & SKO_PERM_KEY_SR_LOCK)) {
		status &= ~OTP_HW_CMN_STATUS_SRD_LOCKED;
	}
	if (!(perm_mask & SKO_PERM_KEY_NSR_LOCK)) {
   		status &=  ~OTP_HW_CMN_STATUS_NSRD_LOCKED;
	}
	if (!(perm_mask & SKO_PERM_KEY_SW_LOCK)) {
   		status &= ~OTP_HW_CMN_STATUS_SW_LOCKED;
	}
	if (!(perm_mask & SKO_PERM_KEY_NSW_LOCK)) {
		status &= ~OTP_HW_CMN_STATUS_NSW_LOCKED;
	}
	if ( !SKO_KEY_DATA_GOOD(sko_hw_readl(dev, SKO_KEYN_STATUS_OFFSET+sko_offs))) {
		status &= ~OTP_HW_CMN_STATUS_ROW_DATA_VALID;
	}
	if ( !SKO_KEY_DATA_GOOD(sko_hw_readl(dev, SKO_KEYN_STATUS_OFFSET+sko_offs)) &&
		!SKO_KEY_CTRL_OTP_WLOCK(sko_hw_readl(dev, SKO_KEYN_STATUS_OFFSET+sko_offs))) {
		status &= ~OTP_HW_CMN_STATUS_ROW_RD_LOCKED;
	}
	*res = status;
	rc = OTP_HW_CMN_OK;
err:
	return rc;
}

static otp_hw_cmn_err_t lock_all(otp_hw_cmn_t *dev)
{
	int i;
	for (i = 1; i < SKO_SECT_MAX; i++) { 
		sko_hw_writel(dev, 
			SKO_KEYN_PAC_CTRL_OFFSET+i*SKO_SIZE, SKO_PERM_LOCK_ALL);
	}
	return OTP_HW_CMN_OK;

}

static otp_hw_cmn_err_t lock(otp_hw_cmn_t *dev,
			u32 addr, 
			otp_hw_cmn_perm_t perm)
{

	otp_hw_cmn_err_t rc = OTP_HW_CMN_ERR_FAIL; 
	u32	sko_sect;
        otp_hw_cmn_row_t  *row;
	u32 perm_mask = 0;
	if (perm & OTP_HW_CMN_CTL_LOCK_ALL) {
  		return lock_all(dev);
	}

	if ( lookup_row(dev, addr, &row)) {
		rc = OTP_HW_CMN_ERR_UNSP;
		_ETR(rc); 
		goto err;
	}
	sko_sect = SKO_ADDR2SECT(row->addr);
	if ( sko_sect >= SKO_SECT_MAX) {
		rc = OTP_HW_CMN_ERR_INVAL;
		goto err;
	}
   	if (perm & OTP_HW_CMN_CTL_LOCK_PAC_NSRD) {
		SKO_PERM_PAC_LOCK_SET(perm_mask, SKO_PERM_PAC_NSR_LOCK);
	}
   	if (perm & OTP_HW_CMN_CTL_LOCK_PAC_SRD) {
		SKO_PERM_PAC_LOCK_SET(perm_mask, SKO_PERM_PAC_SR_LOCK);
	}
	if (perm & OTP_HW_CMN_CTL_LOCK_PAC_NSW) {
		SKO_PERM_PAC_LOCK_SET(perm_mask, SKO_PERM_PAC_NSW_LOCK);
	}
	if (perm & OTP_HW_CMN_CTL_LOCK_PAC_SW) {
		SKO_PERM_PAC_LOCK_SET(perm_mask, SKO_PERM_PAC_SW_LOCK);
	}
	if (perm & OTP_HW_CMN_CTL_LOCK_SRD) {
		SKO_PERM_KEY_LOCK_SET(perm_mask, SKO_PERM_KEY_SR_LOCK);
	}
   	if (perm & OTP_HW_CMN_CTL_LOCK_NSRD) {
		SKO_PERM_KEY_LOCK_SET(perm_mask, SKO_PERM_KEY_NSR_LOCK);
	}
   	if (perm & OTP_HW_CMN_CTL_LOCK_SW) {
		SKO_PERM_KEY_LOCK_SET(perm_mask, SKO_PERM_KEY_SW_LOCK);
	}
	if (perm & OTP_HW_CMN_CTL_LOCK_NSW) {
		SKO_PERM_KEY_LOCK_SET(perm_mask, SKO_PERM_KEY_NSW_LOCK);
	}
	sko_hw_writel(dev, SKO_KEYN_PAC_CTRL_OFFSET+sko_sect*SKO_SIZE, perm_mask);
	rc = OTP_HW_CMN_OK;
err:
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
   		case OTP_HW_CMN_CTL_LOCK: {
				otp_hw_ctl_data_t* ctl_data = (otp_hw_ctl_data_t*)cmd->data;	
				rc = lock(dev, ctl_data->addr, ctl_data->perm);
			}
			break;
   		case OTP_HW_CMN_CTL_STATUS: {
				otp_hw_ctl_data_t* ctl_data = (otp_hw_ctl_data_t*)cmd->data;	
				rc = status(dev, ctl_data->addr, ctl_data->status, res);
			}
			break;
		default :
			break;
	}
	return rc;
}

__weak otp_hw_cmn_err_t sotp_hw_dev_mmap(otp_hw_cmn_t* dev)
{
	dev->mm = SKO_BASE;
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
		return OTP_HW_CMN_ERR_FAIL;
	}
	dev->read = sko_hw_read;
	dev->write = sko_hw_write;
	dev->ctl = sotp_hw_cmn_ctl;
	return OTP_HW_CMN_OK;
}
