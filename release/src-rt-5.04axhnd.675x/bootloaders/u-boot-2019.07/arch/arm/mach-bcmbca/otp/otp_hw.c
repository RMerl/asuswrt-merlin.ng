/* SPDX-License-Identifier: GPL-2.0+
 *
 *  Copyright 2020 Broadcom Ltd.
 */

/**********************************************************************
 *	
 * OTP controller driver 
 *	
 *********************************************************************	
 */
#include <common.h>
#include <linux/ctype.h>
#include <linux/io.h>
#include "linux/printk.h"
#include <asm/arch/misc.h>
#include <asm/arch/otp.h>
#include "otp_hw.h" 
#include "otp_map_cmn.h"

//#define OTP_DRY_RUN
//#define DBG_OTP
#ifdef DBG_OTP
#define _ETR(__p__)  printf("%s ERROR: \n\t\t\t\t -> %s:%d code %d\n",__FILE__,__FUNCTION__,__LINE__,__p__)
#define _TR(__p__)  printf("%s \n\t\t\t\t -> %s:%d %d (0x%x)\n",__FILE__,__FUNCTION__,__LINE__,__p__,__p__)
#define _DPRT(...)  printf(__VA_ARGS__) 
#else
#define _ETR(__p__)
#define _TR(__p__)
#define _DPRT(...)
#endif

__weak void otp_hw_writel(void* dev, u32 offset, u32 data)
{
	otp_hw_cmn_t* otp_hw = (otp_hw_cmn_t*)dev; 
	writel(data, otp_hw->mm + offset);
}

__weak u32 otp_hw_readl(void* dev, u32 offset)
{
	otp_hw_cmn_t* otp_hw = (otp_hw_cmn_t*)dev;
	return readl(otp_hw->mm+offset);	
}

__weak otp_hw_cmn_err_t otp_hw_cpu_lock(otp_hw_cmn_t* dev)
{
	return OTP_HW_CMN_OK;
}

__weak void otp_hw_cpu_unlock(otp_hw_cmn_t* dev)
{
	/* Release hardware spinlock for OTP */
}

__weak otp_hw_cmn_err_t otp_hw_set_ecc(otp_hw_cmn_t* dev, u32 ecc)
{
	return OTP_HW_CMN_ERR_UNSP;
}

__weak u32 otp_hw_get_ecc(otp_hw_cmn_t* dev, u32* ecc)
{
	return OTP_HW_CMN_ERR_UNSP;
}

static int otp_wait_cmd_done(otp_hw_cmn_t *dev)
{
	long to = OTP_STATUS_CMD_DONE_TMO_CNT;
	while (to && !(otp_hw_readl(dev, OTP_STATUS1_OFFSET)&OTP_STATUS1_CMD_DONE)) {
		udelay(1);
		to--;
	}
	return (!to);
}

static int otp_wait_cmd_ready(otp_hw_cmn_t *dev)
{
	int to = OTP_STATUS_CMD_DONE_TMO_CNT;
	do {
		if (((otp_hw_readl(dev, OTP_STATUS1_OFFSET))&OTP_STATUS1_CMD_DONE) != 0) {
			break;
		}
		udelay(1);
		to--;
	} while (to);
	return (!to);	
}

/*Enables OTP programming mode*/
static otp_hw_cmn_err_t  otp_auth_prog_mode(otp_hw_cmn_t *dev)
{
	int i = 0;
	otp_hw_cmn_err_t rc = OTP_HW_CMN_ERR_TMO;
	u32 authVal[ ] = {0xf, 0x4, 0x8, 0xd};
	if(otp_hw_cpu_lock(dev)) {
		_ETR(rc);	
      		goto err;
	}

	otp_hw_writel(dev, OTP_CTRL0_OFFSET, 0x0);
	/* Enable CPU side programming of OTP */
	otp_hw_writel(dev, OTP_CTRL1_OFFSET, 
			(otp_hw_readl(dev, OTP_CTRL1_OFFSET)|OTP_CTRL1_CPU_MODE));
	/* Clear row register. A non-empty row register results in a failed prog-enable sequence on some SoCs */
	otp_hw_writel(dev, OTP_CTRL3_OFFSET, 0x0);
	/* Put OTP in program mode --> prog-enable sequence */
	for (i = 0; i < sizeof(authVal)/sizeof(u32); i++) {
		otp_hw_writel(dev, OTP_CTRL2_OFFSET, authVal[i]);
		otp_hw_writel(dev, OTP_CTRL0_OFFSET, OTP_CTRL0_PROG_MODE_ENABLE);
		if ( otp_wait_cmd_done(dev)) {
			_ETR(rc);	
         		printf("%s: ERROR! Timed out waiting for  Program Mode; status:0x%08x\n", 
				__FUNCTION__, otp_hw_readl(dev, OTP_STATUS1_OFFSET));
         		goto err;
      		}
		otp_hw_writel(dev, OTP_CTRL0_OFFSET, 0x0);
   	}
	udelay(300);
	if ((otp_hw_readl(dev, OTP_STATUS1_OFFSET) & OTP_STATUS1_PROG_OK) != OTP_STATUS1_PROG_OK) {
		_ETR(rc);
		printf("%s: ERROR: Unable to set OTP program mode OTP\n",__FUNCTION__);
		goto err;
	}
	rc = OTP_HW_CMN_OK;
err:
	if (rc) {
		otp_hw_cpu_unlock(dev);
	}
	return rc;
}

static otp_hw_cmn_err_t otp_auth_prog_mode_done(otp_hw_cmn_t *dev)
{
	otp_hw_writel(dev, OTP_CTRL0_OFFSET, 0x0);
	otp_hw_writel(dev, OTP_CTRL1_OFFSET, 
			(otp_hw_readl(dev, OTP_CTRL1_OFFSET)&(~OTP_CTRL1_CPU_MODE)));
	otp_hw_cpu_unlock(dev);
	return OTP_HW_CMN_OK;
}

__weak otp_hw_cmn_err_t otp_hw_cmn_ctl(otp_hw_cmn_t *dev, 
			const otp_hw_cmn_ctl_cmd_t *cmd,
			u32* res)
{
	otp_hw_cmn_err_t rc = OTP_HW_CMN_ERR_UNSP;
	switch ((u32)cmd->ctl) {
   		case OTP_HW_CMN_CTL_OTPCMD_AUTH_PROG:
 			rc = otp_auth_prog_mode(dev);
			break;
  		case OTP_HW_CMN_CTL_OTPCMD_AUTH_PROG_DONE:
 			rc =otp_auth_prog_mode_done(dev);
			break;
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
		default :
			break;	
	}
	return rc;
	
}

/************************************************************
 *  int bcm_otp_hw_write 
 *  Input parameters: 
 *     addr   - Row address
 *  Return value:
 ***********************************************************/
__weak otp_hw_cmn_err_t  otp_hw_write(otp_hw_cmn_t *dev, 
				u32 addr,
				otp_hw_cmn_row_conf_t* row_conf,
				const u32* data,
				u32 size)
{
	otp_hw_cmn_err_t rc = OTP_HW_CMN_ERR_TMO;
	if (size < sizeof(u32)) {
		rc = OTP_HW_CMN_ERR_INVAL;
		_ETR(rc);
		goto err;
	}
	if (row_conf && (row_conf->op_type & OTP_HW_CMN_CTL_OTPCMD_ECC)) {
		if (size < sizeof(u32)*2) {
			rc = OTP_HW_CMN_ERR_INVAL;
			_ETR(rc);
			goto err;
		}
	}
	_DPRT("%s:%d DEBUG: fusing data 0x%x at 0x%x\n",__FUNCTION__,__LINE__, *data, addr);
	rc = otp_auth_prog_mode(dev);
	if (rc) {
		_ETR(rc);
		goto err;
	}

#ifndef OTP_DRY_RUN
	otp_hw_writel(dev, OTP_CTRL2_OFFSET, *data);
#endif
	if ( row_conf && (row_conf->op_type&OTP_HW_CMN_CTL_OTPCMD_ECC) ) {
		otp_hw_set_ecc(dev, *(data + 1));
	}
#ifndef OTP_DRY_RUN
	otp_hw_writel(dev, OTP_CTRL3_OFFSET, addr);
#endif
	if (row_conf && (row_conf->op_type & OTP_HW_CMN_CTL_OTPCMD_PROG_LOCK)) {
		otp_hw_writel(dev, OTP_CTRL0_OFFSET, (OTP_CTRL0_PROG_CMD_START|OTP_CTRL0_CMD_PROG_LOCK));
	} else {
		otp_hw_writel(dev, OTP_CTRL0_OFFSET, OTP_CTRL0_PROG_CMD_START);
	}

#ifndef OTP_DRY_RUN
	if (otp_wait_cmd_done(dev)) {
		printf("%s: ERROR! Timed out waiting for OTP command completion (WRITE)! status: 0x%08x\n", 
		__FUNCTION__, otp_hw_readl(dev, OTP_STATUS1_OFFSET));
		goto err;
	}
#endif
	_DPRT("%s:%d Fused: 0x%x  \n", __FUNCTION__,__LINE__,*data);
	rc = OTP_HW_CMN_OK;
err:
	otp_auth_prog_mode_done(dev);
	return rc;
}

/***********************************************************
 *  otp_hw_cmn_err_t otp_hw_read
 *  Input parameters: 
 *     addr    - Row address
 *  Return value:
 *      returns 0 if successful, value in *value
 ***********************************************************/
__weak otp_hw_cmn_err_t  otp_hw_read(otp_hw_cmn_t *dev, 
				u32 addr,
				otp_hw_cmn_row_conf_t* row_conf,
				u32* data,
				u32 size)
{
	otp_hw_cmn_err_t rc = OTP_HW_CMN_ERR_TMO;
	if (size < sizeof(u32)) {
		rc = OTP_HW_CMN_ERR_FAIL;
		_ETR(rc);
		goto err;
	}
	if (row_conf && (row_conf->op_type & OTP_HW_CMN_CTL_OTPCMD_ECC)) {
		if (size < sizeof(u32)*2) {
			rc = OTP_HW_CMN_ERR_INVAL;
			_ETR(rc);
			goto err;
		}
	}

	rc = otp_hw_cpu_lock(dev);
	if (rc) {
		_ETR(rc);
		goto err;
	}

	if ((otp_hw_readl(dev, OTP_STATUS1_OFFSET)&OTP_STATUS1_CMD_DONE) == 0){
		_DPRT("%s 1 CMD done was not set\n", __FUNCTION__);
	}	
	/* turn on cpu mode, set up row addr, activate read word */
	otp_hw_writel(dev, OTP_CTRL1_OFFSET, 
			(otp_hw_readl(dev, OTP_CTRL1_OFFSET)|OTP_CTRL1_CPU_MODE));
	otp_hw_writel(dev, OTP_CTRL3_OFFSET, addr);
	otp_hw_writel(dev, OTP_CTRL0_OFFSET, OTP_CTRL0_START);

	/* Wait for low CMD_DONE (current operation has begun), reset countdown, wait for retrieval to complete 
 	*	Redundant; can't be always caught 
 	*	 	*/
	otp_wait_cmd_ready(dev);
	/* Wait for high CMD_DONE */
	if (otp_wait_cmd_done(dev)) {
		_ETR(rc);
		printf("%s: ERROR! Timed out waiting for OTP command completion (READ)! status: 0x%08x\n", 
		__FUNCTION__, otp_hw_readl(dev, OTP_STATUS1_OFFSET));
		goto err;
   	}

	/* If read was successful, retrieve data */
   
	*data = otp_hw_readl(dev, OTP_STATUS0_OFFSET);
	_DPRT("DEBUG: got 0x%x from otp\n", *data);
	if (row_conf && (row_conf->op_type & OTP_HW_CMN_CTL_OTPCMD_ECC)) {
		otp_hw_get_ecc(dev, data + 1);
	}

	rc = OTP_HW_CMN_OK;
	/* zero out the ctrl_0 reg, turn off cpu mode, return results */
err:
	otp_hw_writel(dev, OTP_CTRL0_OFFSET, 0x0);
	otp_hw_writel(dev, OTP_CTRL1_OFFSET,
			(otp_hw_readl(dev, OTP_CTRL1_OFFSET)&(~OTP_CTRL1_CPU_MODE)));
	otp_hw_cpu_unlock(dev);
	return rc;
}

static  otp_hw_cmn_err_t write(otp_hw_cmn_t* dev, 
		u32 addr,
		const u32* data,
		u32 size)
{
	otp_hw_cmn_row_conf_t *cfg =  
				(dev->ctl_cmd.ctl&OTP_HW_CMN_CTL_CONF)? &dev->row_conf : NULL;
	return dev->write_ex? 
		dev->write_ex(dev, addr, cfg, data, size) : OTP_HW_CMN_ERR_UNSP;
}

static otp_hw_cmn_err_t read(otp_hw_cmn_t *dev,
			u32 addr,
			u32 *data,
			u32 size)
{
	otp_hw_cmn_row_conf_t *cfg =  
		(dev->ctl_cmd.ctl&OTP_HW_CMN_CTL_CONF)? 
			&dev->row_conf : NULL;
	return dev->read_ex?
		dev->read_ex(dev, addr, cfg, data, size):OTP_HW_CMN_ERR_UNSP;

}
__weak otp_hw_cmn_err_t otp_hw_dev_mmap(otp_hw_cmn_t* dev)
{
	dev->mm = JTAG_OTP_BASE;
	return OTP_HW_CMN_OK;
}

__weak otp_hw_cmn_err_t otp_hw_init(otp_hw_cmn_t* dev)
{
	DEFINE_OTP_MAP_ROW_INITLR(rows);
	if (otp_hw_dev_mmap(dev)) {
		return OTP_HW_CMN_ERR_FAIL;
	}
	dev->rows = rows;
	dev->row_max = sizeof(rows)/sizeof(otp_hw_cmn_row_t);
	return OTP_HW_CMN_OK;
}


otp_hw_cmn_err_t otp_hw_cmn_init(otp_hw_cmn_t* dev)
{
	if (otp_hw_init(dev)) {
		return OTP_HW_CMN_ERR_FAIL;
	}
	dev->read = read;
	dev->write = write;
	dev->read_ex = otp_hw_read;
	dev->write_ex = otp_hw_write;
	dev->ctl = otp_hw_cmn_ctl;
	return OTP_HW_CMN_OK;
}

