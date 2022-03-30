/* SPDX-License-Identifier: GPL-2.0+
 *
 *  Copyright 2020 Broadcom Ltd.
 */
#include <common.h>
#include <linux/ctype.h>
#include <linux/io.h>
#include "linux/printk.h"
#include <asm/arch/otp.h>
#include <asm/arch/misc.h>
#include <malloc.h>
#include "bcm_secure.h"
#include "otp_map_cmn.h"

//#define DBG_OTP

#ifdef DBG_OTP
#define _ETR(__p__)  printf("%s \n\t\t\t\t -> %s:%d code %d\n",__FILE__, __FUNCTION__,__LINE__,__p__)
#define _DPRT(...)  printf(__VA_ARGS__) 
#else
#define _ETR(__p__)
#define _DPRT(...)
#endif
__weak void* otp_map_cmn_malloc(size_t sz)
{
	return malloc(sz);
}

/* address finder  */
static otp_map_cmn_err_t __get_addr(otp_map_cmn_t *obj, 
				otp_map_feat_t ft,
				u32* idx)
{
	int i ;
	otp_hw_cmn_t* dev = &obj->dev;
	otp_hw_cmn_row_t* rows = dev->rows;	
	for (i = 0; i < dev->row_max; i++ ) {
		if (ft > OTP_MAP_INVALID && ft < OTP_MAP_MAX) {
			if (ft ==  rows[i].feat) {
				*idx = i;
				_DPRT("%s:%d id %d \n",__FUNCTION__,__LINE__, i);
				return OTP_MAP_CMN_OK;	
			} 
		}
	}
	_DPRT("%s:%d feat %d \n",__FUNCTION__,__LINE__, ft);
	return OTP_MAP_CMN_ERR_UNSP;	
}
 
/* redirects to device base control function.  */
static otp_map_cmn_err_t otp_map_cmn_ctl(otp_map_cmn_t *obj,
			otp_hw_cmn_ctl_cmd_t* pcmd,  u32* res)
{
	otp_map_cmn_err_t rc = OTP_MAP_CMN_ERR_FAIL;
	otp_hw_cmn_t* dev = &obj->dev;
	otp_hw_cmn_row_t* rows = dev->rows;	
	u32 id = 0;
	otp_hw_cmn_ctl_cmd_t cmd;
	otp_hw_ctl_data_t* cmd_data = (otp_hw_ctl_data_t*)malloc((size_t)pcmd->size);
	if (!cmd_data) {
		_ETR(rc);
		goto err;
	}	
	memcpy(&cmd, pcmd, sizeof(otp_hw_cmn_ctl_cmd_t));
	memcpy(cmd_data, (void*)pcmd->data, pcmd->size);
	cmd.data = (uintptr_t)cmd_data;
	_DPRT("control 0x%x \n",cmd.ctl);
	if (cmd.ctl == OTP_HW_CMN_CTL_STATUS)  {
		otp_hw_ctl_data_t* ctl_data = (otp_hw_ctl_data_t*)cmd.data;
		/*convert: from feature to row*/
		rc = __get_addr(obj, ctl_data->addr, &id); 
		if (rc) {
			_ETR(rc);
			goto err;
		}
		ctl_data->addr = rows[id].addr;
	}

	rc = dev->ctl(dev, &cmd, res);
	if (rc) {
		_ETR(rc);
		goto err;
	} 
	rc = OTP_MAP_CMN_OK;
err:
	if (cmd_data) {
		free(cmd_data);
	}
	return rc;
}

/* fuse/write either one a feature mapped row or set of rows depending on 
 * underlying ROW intializer   (otp_hw_cmn_row_t)  
 * otp row mask and shift are also applied here
 * */
static otp_map_cmn_err_t otp_map_cmn_write(otp_map_cmn_t *obj, 
			u32 feat, const u32* data,  u32 size)
{
	int i;
	u32 range = 1, write_sz;
	otp_hw_cmn_t* dev = &obj->dev;
	otp_map_cmn_err_t rc = OTP_MAP_CMN_ERR_FAIL;
	otp_hw_cmn_row_t* rows = dev->rows;
	u32 id = 0;
	u32 *row_data;
	if (__get_addr(obj, feat, &id)) {
		_ETR(rc);
		goto err;
	}
	if (id >= dev->row_max || 
			size > rows[id].range*sizeof(u32)) {
		_ETR(OTP_MAP_CMN_ERR_INVAL);
		return OTP_MAP_CMN_ERR_INVAL;
	}
	row_data = malloc(size);
	if (!row_data) {
		_ETR(rc);
		goto err;
	}
	memcpy(row_data, data, size); 
	if (rows[id].mask) {
		for (i = 0; i < rows[id].range ;i++ ) {
			row_data[i] = ((data[i]&rows[id].mask)<<rows[id].shift);
		}
	}
	if (rows[id].conf.addr_type == OTP_HW_CMN_ROW_ADDR_ROW) {
		range = rows[id].range;
		write_sz = sizeof(u32);
	} else {
		range = 1;
		write_sz = sizeof(u32)*rows[id].range;
	}  
	for (i = 0; i < range; i++)  {
		_DPRT("%s:%d attempt to write: 0x%x at row 0x%x\n",__FUNCTION__,__LINE__,
			row_data[i],(rows[id].addr+i));
		if (dev->write(dev, rows[id].addr + i, &row_data[i], write_sz)) {
				_ETR(rc);
				/* Commit failed, invalidate cached data */
				rows[id].valid = 0;
				goto err;	
		}
	}
	rc = OTP_MAP_CMN_OK;
err:
	free(row_data);
	return rc;
}

/* read either a feature mapped row or set of rows depending on 
 * underlying ROW intializer   (otp_hw_cmn_row_t)  
 * otp row mask and shift are also applied here
 * An objec is returned with size. Second read on the same feature will return
 * it's  cached copy
 * */
static otp_map_cmn_err_t otp_map_cmn_read(otp_map_cmn_t *obj,
			u32 feat, 
			u32** data, u32* size)
{
	int i;
	otp_map_cmn_err_t rc = OTP_MAP_CMN_ERR_FAIL;
	otp_hw_cmn_t* dev = &obj->dev;
	otp_hw_cmn_row_t* rows = dev->rows;	
	u32* p;
	u32 id = 0;
	if (!data) {
		rc = OTP_MAP_CMN_ERR_INVAL;
		_ETR(rc);
		goto err;
	}
	rc = __get_addr(obj, feat, &id); 
	if (rc) {
		_ETR(rc);
		goto err;
	}
	_DPRT("%s: id:%d, range:%d, addr_type:%d, op_type:%d\n", __FUNCTION__, id, rows[id].range, rows[id].conf.addr_type, rows[id].conf.op_type);
	p = rows[id].range > 1?
			(u32*)(rows[id].pdata) : &rows[id].data;
	if (!rows[id].valid) {
		u32 range;
		u32 read_sz;
		if (rows[id].conf.addr_type == OTP_HW_CMN_ROW_ADDR_ROW) {
			range = rows[id].range;
			read_sz = sizeof(u32);
		} else {
			range = 1;
			read_sz = sizeof(u32)*rows[id].range;
		}  
		
		for (i = 0; i < range; i++) { 
			rc = dev->read(dev, rows[id].addr+i, 
				 &p[i], read_sz);
			if (rc) {
				_ETR(rc);
				goto err;
			}
		}
		if (rows[id].mask) {
			for (i = 0; i < rows[id].range ;i++ ) {
				p[i] = ((p[i] >> rows[id].shift)&rows[id].mask);
			}
		}
		rows[id].valid = 1;
	}

	*data = p;
	if (size) {
		*size = rows[id].range*sizeof(u32);
	}
	rc = OTP_MAP_CMN_OK;
err:
	return rc;	
}
#if defined BCM_OTP_READ_MAP 
static inline otp_map_cmn_err_t otp_map_cmn_read_map(otp_map_cmn_t* map)
{
	u32 i, size = 0, *val;
	otp_hw_cmn_t* dev = &map->dev;
	otp_hw_cmn_row_t* rows = dev->rows;	
	otp_map_cmn_err_t rc = OTP_MAP_CMN_ERR_FAIL;
	for( i = 0; i < dev->row_max; i++ ) {
		if (!rows[i].feat) {
			continue;
		}
		rc = map->read(map, rows[i].feat, 
			&val, &size);
		if (rc) {
			_ETR(rc);
		} else { 
			_DPRT("%s:%d Got feat %d val 0x%x size 0x%x \n", __FUNCTION__,__LINE__,rows[i].feat, *val, size);
		}
	}
	rc = OTP_MAP_CMN_OK;
	return rc;
}
#endif
static inline otp_map_cmn_err_t  initialize(otp_map_cmn_t* obj)
{
	u32 i;
	otp_hw_cmn_row_t* rows = obj->dev.rows;
	otp_map_cmn_err_t rc = OTP_MAP_CMN_ERR_FAIL;
	for (i = 0; i < obj->dev.row_max; i++ ) {
		if (rows[i].range > 1) {
			u8* p = otp_map_cmn_malloc(rows[i].range*sizeof(u32));
			if (!p) {
				_ETR(rc);
				goto err;
			}
			rows[i].pdata = p;
		}
	}
	rc = OTP_MAP_CMN_OK;
err:
	return rc;
}

 
otp_map_cmn_err_t otp_map_cmn_init(otp_map_cmn_t *map, 
			otp_hw_cmn_init_t hw_init,
			otp_hw_cmn_t* ext_drv)
{
	otp_map_cmn_err_t rc = OTP_MAP_CMN_ERR_FAIL;
	if (!hw_init ) {
		goto err;
	} 
	rc = hw_init(&map->dev); 
	if (rc) {
		if (rc == OTP_HW_CMN_ERR_UNSP) {
			rc = OTP_MAP_CMN_ERR_UNSP;
		}
		_ETR(rc);
		goto err;
	}
	if (ext_drv) {
		map->dev.drv_ext = ext_drv;
	}
	if (initialize(map)) {
		_ETR(rc);
		goto err;
	}
	map->ctl = otp_map_cmn_ctl;
	map->write = otp_map_cmn_write;
	map->read = otp_map_cmn_read;

#if defined BCM_OTP_READ_MAP 
	if(otp_map_cmn_read_map(map)) {
		_ETR(rc);
		/*goto err;*/
	}
#endif
	rc = OTP_MAP_CMN_OK;
err:
	return rc;
}
