#if defined(CONFIG_BCM_KF_ARM_BCM963XX)
/*
<:copyright-BRCM:2013:DUAL/GPL:standard

   Copyright (c) 2013 Broadcom 
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

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#include <asm/mach/map.h>
#include <mach/memory.h>
#include <plat/bcm63xx_acp.h>
#include <linux/mm.h>

#include "bcm_map_part.h"
#include "pmc_drv.h"
#include "BPCM.h"

typedef struct {
	uint32_t addr_in;
	uint32_t addr_out;
	uint8_t dst_pid;
	uint8_t size_shift;
	uint8_t en;
	struct proc_dir_entry *proc_dir;
} ubus_cfg_t;

typedef struct {
	ubus_cfg_t ubus_cfg[BCM_UBUS_CFG_MAX];
	int pmb_addr;
	uint32_t acp_ctrl;
	uint8_t en;
	uint8_t name[10];
	struct proc_dir_entry *proc_dir;
} acp_cfg_entry_t;

acp_cfg_entry_t acp_cfg_tbl[BCM_UBUS_PID_MAX];
struct proc_dir_entry *proc_acp_dir = NULL;

static void acp_ctrl_set(uint8_t id, uint8_t enable)
{
	if (enable)
		ARMAIPCTRL->acp_ctrl[id] = acp_cfg_tbl[id].acp_ctrl;
	else
		ARMAIPCTRL->acp_ctrl[id] = 0;
}

static int ubus_cfg_entry_set(uint8_t ubus_pid, uint8_t entry_id,
		uint32_t addr_in, uint32_t addr_out, uint8_t dst_pid,
		uint8_t size_shift, uint8_t enable)
{
	BPCM_UBUS_CFG_REG ubus_cfg;
	int ret;

	if (acp_cfg_tbl[ubus_pid].pmb_addr == 0)
		return -1;

	ret = ReadBPCMRegister(acp_cfg_tbl[ubus_pid].pmb_addr,
			UBUSBPCMRegOffset(cfg[entry_id]),
			&ubus_cfg.Regs32.word0);
	if (ret)
		return ret;

	ret = ReadBPCMRegister(acp_cfg_tbl[ubus_pid].pmb_addr,
			UBUSBPCMRegOffset(cfg[entry_id]) + 1,
			&ubus_cfg.Regs32.word1);
	if (ret)
		return ret;

	ubus_cfg.Bits.addr_in = addr_in >> 8;
	ubus_cfg.Bits.addr_out = addr_out >> 8;
	ubus_cfg.Bits.pid = dst_pid;
	ubus_cfg.Bits.size = size_shift;
	ubus_cfg.Bits.cmddta = 0;
	ubus_cfg.Bits.en = enable;

	ret = WriteBPCMRegister(acp_cfg_tbl[ubus_pid].pmb_addr,
			UBUSBPCMRegOffset(cfg[entry_id]),
			ubus_cfg.Regs32.word0);
	if (ret)
		return ret;

	ret = WriteBPCMRegister(acp_cfg_tbl[ubus_pid].pmb_addr,
			UBUSBPCMRegOffset(cfg[entry_id]) + 1,
			ubus_cfg.Regs32.word1);
	return ret;
}

int bcm63xx_acp_ubus_cfg_get_entry(uint8_t ubus_pid, uint8_t idx,
		bcm_acp_ubus_cfg_t *acp_ubus_cfg)
{
	if (acp_ubus_cfg == NULL)
		return -EINVAL;

	if (idx >= 4)
		return -EINVAL;

	acp_ubus_cfg->addr_in = acp_cfg_tbl[ubus_pid].ubus_cfg[idx].addr_in;
	acp_ubus_cfg->addr_out = acp_cfg_tbl[ubus_pid].ubus_cfg[idx].addr_out;
	acp_ubus_cfg->dst_pid = acp_cfg_tbl[ubus_pid].ubus_cfg[idx].dst_pid;
	acp_ubus_cfg->size_shift = acp_cfg_tbl[ubus_pid].ubus_cfg[idx].size_shift;
	acp_ubus_cfg->en = acp_cfg_tbl[ubus_pid].ubus_cfg[idx].en;

	return 0;
}

int bcm63xx_acp_ubus_cfg_get_all(uint8_t ubus_pid,
		bcm_acp_ubus_cfg_t *acp_ubus_cfg)
{
	int i;

	for (i = 0; i < 4; i++) {
		if (bcm63xx_acp_ubus_cfg_get_entry(ubus_pid, i, &acp_ubus_cfg[i]) != 0)
			return -EINVAL;
	}
	return 0;
}

int bcm63xx_acp_ubus_cfg_set_entry(uint8_t ubus_pid, uint8_t idx,
		bcm_acp_ubus_cfg_t *acp_ubus_cfg)
{
	int ret = 0;

	if (acp_ubus_cfg == NULL)
		return -EINVAL;

	if (idx >= 4)
		return -EINVAL;

	ret = ubus_cfg_entry_set(ubus_pid, idx, acp_ubus_cfg->addr_in,
			acp_ubus_cfg->addr_out, acp_ubus_cfg->dst_pid,
			acp_ubus_cfg->size_shift, acp_ubus_cfg->en);
	if (ret != 0)
		return ret;

	acp_cfg_tbl[ubus_pid].ubus_cfg[idx].addr_in = acp_ubus_cfg->addr_in;
	acp_cfg_tbl[ubus_pid].ubus_cfg[idx].addr_out = acp_ubus_cfg->addr_out;
	acp_cfg_tbl[ubus_pid].ubus_cfg[idx].dst_pid = acp_ubus_cfg->dst_pid;
	acp_cfg_tbl[ubus_pid].ubus_cfg[idx].size_shift = acp_ubus_cfg->size_shift;
	acp_cfg_tbl[ubus_pid].ubus_cfg[idx].en = acp_ubus_cfg->en;

	return ret;
}

int bcm63xx_acp_ubus_cfg_set_all(uint8_t ubus_pid, bcm_acp_ubus_cfg_t *acp_ubus_cfg)
{
	int i;

	for (i = 0; i < 4; i++) {
		if (bcm63xx_acp_ubus_cfg_set_entry(ubus_pid, i, &acp_ubus_cfg[i]) != 0)
			goto SET_UBUS_CFG_FAIL;
	}
	return 0;

SET_UBUS_CFG_FAIL:
	bcm63xx_acp_ubus_cfg_reset(ubus_pid);
	return -EINVAL;
}

void bcm63xx_acp_ubus_cfg_reset(uint8_t ubus_pid)
{
	int i;

	for (i = 0; i < 4; i++) {
		ubus_cfg_entry_set(ubus_pid, i, 0, 0, 0, 0, 0);
		acp_cfg_tbl[ubus_pid].ubus_cfg[i].addr_in = 0;
		acp_cfg_tbl[ubus_pid].ubus_cfg[i].addr_out = 0;
		acp_cfg_tbl[ubus_pid].ubus_cfg[i].dst_pid = 0;
		acp_cfg_tbl[ubus_pid].ubus_cfg[i].size_shift = 0;
		acp_cfg_tbl[ubus_pid].ubus_cfg[i].en = 0;
	}
}

int bcm63xx_acp_cache_ctrl_get(uint8_t ubus_pid, bcm_acp_cache_ctrl_t *cache_ctrl)
{
	if (cache_ctrl == NULL)
		return -EINVAL;

	cache_ctrl->wcache = (acp_cfg_tbl[ubus_pid].acp_ctrl >> AIPACP_WCACHE_SHIFT) & 0xf;
	cache_ctrl->rcache = (acp_cfg_tbl[ubus_pid].acp_ctrl >> AIPACP_RCACHE_SHIFT) & 0xf;
	cache_ctrl->wuser = (acp_cfg_tbl[ubus_pid].acp_ctrl >> AIPACP_WUSER_SHIFT) & 0x1f;
	cache_ctrl->ruser = (acp_cfg_tbl[ubus_pid].acp_ctrl >> AIPACP_RUSER_SHIFT) & 0x1f;
	return 0;
}

int bcm63xx_acp_cache_ctrl_set(uint8_t ubus_pid, bcm_acp_cache_ctrl_t *cache_ctrl)
{
	if (cache_ctrl == NULL)
		return -EINVAL;

	if ((cache_ctrl->ruser > 0x1f) || (cache_ctrl->wuser > 0x1f) ||
			(cache_ctrl->rcache > 0xf) || (cache_ctrl->wcache > 0xf))
		return -EINVAL;

	acp_cfg_tbl[ubus_pid].acp_ctrl = cache_ctrl->ruser << AIPACP_RUSER_SHIFT |
			cache_ctrl->wuser << AIPACP_WUSER_SHIFT |
			cache_ctrl->rcache << AIPACP_RCACHE_SHIFT |
			cache_ctrl->wcache << AIPACP_WCACHE_SHIFT;

	acp_ctrl_set(ubus_pid, acp_cfg_tbl[ubus_pid].en);
	return 0;
}

int bcm63xx_acp_enable(uint8_t ubus_pid)
{
	uint8_t i;
	int ret = 0;

	/* enable ACP ctrl */
	acp_ctrl_set(ubus_pid, 1);

	for (i = 0; i < 4; i++) {
		ret |= ubus_cfg_entry_set(ubus_pid, i,
				acp_cfg_tbl[ubus_pid].ubus_cfg[i].addr_in,
				acp_cfg_tbl[ubus_pid].ubus_cfg[i].addr_out,
				acp_cfg_tbl[ubus_pid].ubus_cfg[i].dst_pid,
				acp_cfg_tbl[ubus_pid].ubus_cfg[i].size_shift,
				acp_cfg_tbl[ubus_pid].ubus_cfg[i].en);
	}
	if (ret)
		goto fail_reset_hw;

	acp_cfg_tbl[ubus_pid].en = 1;

	return 0;

fail_reset_hw:
	for (i = 0; i < 4; i++)
		ubus_cfg_entry_set(ubus_pid, i, 0, 0, 0, 0, 0);
	acp_ctrl_set(ubus_pid, 0);

	return ret;
}

int bcm63xx_acp_disable(uint8_t ubus_pid)
{
	int i;

	for (i = 0; i < 4; i++)
		ubus_cfg_entry_set(ubus_pid, i, 0, 0, 0, 0, 0);
	acp_ctrl_set(ubus_pid, 0);
	acp_cfg_tbl[ubus_pid].en = 0;

	return 0;
}

bool bcm63xx_acp_on(uint8_t ubus_pid)
{
	return acp_cfg_tbl[ubus_pid].en != 0;
}

static void acp_cfg_tbl_init(int entry_use, uint32_t *addr_in,
		uint32_t *addr_out, uint8_t *dst_pid, uint8_t *size_shift)
{
	uint8_t i, j;

	memset(acp_cfg_tbl, 0x0, BCM_UBUS_PID_MAX * sizeof(acp_cfg_entry_t));

	/* only initialize table for supported device */
#ifdef CONFIG_BCM963138
	acp_cfg_tbl[BCM_UBUS_PID_PCIE0].pmb_addr = UBUS_CFG_PMB_ADDR_PCIE0;
	sprintf((char *)acp_cfg_tbl[BCM_UBUS_PID_PCIE0].name, "pcie0");

	acp_cfg_tbl[BCM_UBUS_PID_ARMAXIACP].pmb_addr = UBUS_PMB_ADDR_ARM;
	sprintf((char *)acp_cfg_tbl[BCM_UBUS_PID_ARMAXIACP].name, "arm");

	acp_cfg_tbl[BCM_UBUS_PID_PERIPH].pmb_addr = UBUS_CFG_PMB_ADDR_PERIPH;
	sprintf((char *)acp_cfg_tbl[BCM_UBUS_PID_PERIPH].name, "periph");

	acp_cfg_tbl[BCM_UBUS_PID_USBD].pmb_addr = UBUS_CFG_PMB_ADDR_USBD;
	sprintf((char *)acp_cfg_tbl[BCM_UBUS_PID_USBD].name, "usbd");

	acp_cfg_tbl[BCM_UBUS_PID_USBH].pmb_addr = UBUS_CFG_PMB_ADDR_USBH;
	sprintf((char *)acp_cfg_tbl[BCM_UBUS_PID_USBH].name, "usbh");

	acp_cfg_tbl[BCM_UBUS_PID_SATA].pmb_addr = UBUS_CFG_PMB_ADDR_SATA;
	sprintf((char *)acp_cfg_tbl[BCM_UBUS_PID_SATA].name, "sata");

	acp_cfg_tbl[BCM_UBUS_PID_DECT].pmb_addr = UBUS_PMB_ADDR_DECT;
	sprintf((char *)acp_cfg_tbl[BCM_UBUS_PID_DECT].name, "dect");

	acp_cfg_tbl[BCM_UBUS_PID_APM].pmb_addr = UBUS_CFG_PMB_ADDR_APM;
	sprintf((char *)acp_cfg_tbl[BCM_UBUS_PID_APM].name, "apm");

#if 0
	// FIXME! not sure which PMB_ADDR VDSL_PID uses.
	acp_cfg_tbl[BCM_UBUS_PID_VDSL].pmb_addr = UBUS_PMB_ADDR_VDSL3_CORE;
	sprintf((char *)acp_cfg_tbl[BCM_UBUS_PID_VDSL].name, "vdsl");
#endif

	acp_cfg_tbl[BCM_UBUS_PID_SAR].pmb_addr = UBUS_CFG0_PMB_ADDR_SAR;
	sprintf((char *)acp_cfg_tbl[BCM_UBUS_PID_SAR].name, "sar0");

	acp_cfg_tbl[BCM_UBUS_PID_RNR].pmb_addr = UBUS_CFG_PMB_ADDR_DBR;
	sprintf((char *)acp_cfg_tbl[BCM_UBUS_PID_RNR].name, "rnr0");

	acp_cfg_tbl[BCM_UBUS_PID_RNR_RABR].pmb_addr = UBUS_CFG_PMB_ADDR_RABR;
	sprintf((char *)acp_cfg_tbl[BCM_UBUS_PID_RNR_RABR].name, "rnr1");

	acp_cfg_tbl[BCM_UBUS_PID_SF2].pmb_addr = UBUS_CFG_PMB_ADDR_SWITCH;
	sprintf((char *)acp_cfg_tbl[BCM_UBUS_PID_SF2].name, "sf2");

	acp_cfg_tbl[BCM_UBUS_PID_PCIE1].pmb_addr = UBUS_CFG_PMB_ADDR_PCIE1;
	sprintf((char *)acp_cfg_tbl[BCM_UBUS_PID_PCIE1].name, "pcie1");

	acp_cfg_tbl[BCM_UBUS_PID_ARMAIPDAP].pmb_addr = UBUS_PMB_ADDR_DAP;
	sprintf((char *)acp_cfg_tbl[BCM_UBUS_PID_ARMAIPDAP].name, "dap");

	acp_cfg_tbl[BCM_UBUS_PID_SAR2].pmb_addr = UBUS_CFG1_PMB_ADDR_SAR;
	sprintf((char *)acp_cfg_tbl[BCM_UBUS_PID_SAR2].name, "sar1");

	acp_cfg_tbl[BCM_UBUS_PID_RNR_RBBR].pmb_addr = UBUS_CFG_PMB_ADDR_RBBR;
	sprintf((char *)acp_cfg_tbl[BCM_UBUS_PID_RNR_RBBR].name, "rnr2");
#endif

	/* initialize software entry for the first ubus cfg entry */
	for (i = 0; i < BCM_UBUS_PID_MAX; i++) {
		if (acp_cfg_tbl[i].pmb_addr != 0) {
			acp_cfg_tbl[i].acp_ctrl = (0xf << AIPACP_WCACHE_SHIFT) |
					(0xf << AIPACP_RCACHE_SHIFT) |
					(0x1 << AIPACP_WUSER_SHIFT) | 
					(0x1 << AIPACP_RUSER_SHIFT);
			for (j = 0; j < entry_use; j++) {
				acp_cfg_tbl[i].ubus_cfg[j].addr_in = addr_in[j];
				acp_cfg_tbl[i].ubus_cfg[j].addr_out = addr_out[j];
				acp_cfg_tbl[i].ubus_cfg[j].dst_pid = dst_pid[j];
				acp_cfg_tbl[i].ubus_cfg[j].size_shift = size_shift[j];
				acp_cfg_tbl[i].ubus_cfg[j].en = 1;
			}
		}
	}
}

/* the following are for the proc file control */
static uint8_t get_ubus_pid_by_proc_dir(struct proc_dir_entry *proc_dir)
{
	uint8_t i, j;

	for (i = 0; i < BCM_UBUS_PID_MAX; i++) {
		if (acp_cfg_tbl[i].pmb_addr != 0) {
			if (acp_cfg_tbl[i].proc_dir == proc_dir)
				return i;
			for (j = 0; j < BCM_UBUS_CFG_MAX; j++) {
				if (acp_cfg_tbl[i].ubus_cfg[j].proc_dir
						== proc_dir)
					return i;
			}
		}
	}
	return BCM_UBUS_PID_INVALID;
}

static inline uint8_t get_cfg_id_by_ubus_pid_proc_dir(uint8_t ubus_pid,
		struct proc_dir_entry *proc_dir)
{
	uint8_t i;

	for (i = 0; i < BCM_UBUS_CFG_MAX; i++) {
		if (acp_cfg_tbl[ubus_pid].ubus_cfg[i].proc_dir == proc_dir)
			return i;
	}
	return BCM_UBUS_CFG_MAX;

}

static ssize_t read_proc_acp_en(struct file *f, char *page, size_t cnt, loff_t *off)
{
	uint8_t ubus_pid;
	int len = 0;
	void *data;

	if(*off != 0)
		return 0;
	
	data = PDE_DATA(file_inode(f));
	if (data == NULL)
		return -EINVAL;
	
	ubus_pid = get_ubus_pid_by_proc_dir((struct proc_dir_entry *)data);
	if (ubus_pid == BCM_UBUS_PID_INVALID)
		return -EINVAL;

	len = sprintf(page + len, "%d\n", acp_cfg_tbl[ubus_pid].en);

	*off = len;
	return len;
}

static ssize_t write_proc_acp_en(struct file *f, const char __user *buffer, size_t count, loff_t *off)
{
	uint8_t ubus_pid;
	char buf[16];
	unsigned long input_val;
	int len, ret;
	void *data;

	if (count >= sizeof(buf))
		goto WRITE_PROC_ACP_EN_EXIT;

	len = min((unsigned int)count, (sizeof(buf) - 1));

	if (copy_from_user(buf, buffer, len))
		goto WRITE_PROC_ACP_EN_EXIT;

	buf[len] = '\0';
	if (strict_strtoul(buf, 0, &input_val))
		goto WRITE_PROC_ACP_EN_EXIT;

	data = PDE_DATA(file_inode(f));
	if (data == NULL)
		goto WRITE_PROC_ACP_EN_EXIT;

	ubus_pid = get_ubus_pid_by_proc_dir((struct proc_dir_entry *)data);
	if (ubus_pid == BCM_UBUS_PID_INVALID)
		goto WRITE_PROC_ACP_EN_EXIT;

	if ((uint8_t)input_val == acp_cfg_tbl[ubus_pid].en) {
		printk(KERN_WARNING "Nothing has been done\n");
		return count;
	}

	if (input_val == 0) {
		ret = bcm63xx_acp_disable(ubus_pid);
		if (ret == 0)
			printk(KERN_WARNING "Done disabling ACP for %s\n",
					acp_cfg_tbl[ubus_pid].name);
	} else {
		ret = bcm63xx_acp_enable(ubus_pid);
		if (ret == 0)
			printk(KERN_WARNING "Done enabling ACP for %s\n",
					acp_cfg_tbl[ubus_pid].name);
	}
	if (ret)
		printk(KERN_WARNING "Fail to configure\n");

	return count;

WRITE_PROC_ACP_EN_EXIT:
	printk(KERN_WARNING "invalid input value\n");
	return count;
}

static ssize_t read_proc_acp_ctrl(struct file *f, char *page, size_t cnt, loff_t *off)
{
	uint8_t ubus_pid;
	int len = 0;
	void *data;

	if(*off != 0)
		return 0;

	data = PDE_DATA(file_inode(f));
	if (data == NULL)
		return -EINVAL;
	
	ubus_pid = get_ubus_pid_by_proc_dir((struct proc_dir_entry *)data);
	if (ubus_pid == BCM_UBUS_PID_INVALID)
		return -EINVAL;

	len += sprintf(page + len, "bit[3-0] = WCACHE, bit[7-4] = RCACHE, bit[12-8] = WUSER, bit[17-13] = RUSER\n");
	len += sprintf(page + len, "0x%lx\n", (unsigned long)acp_cfg_tbl[ubus_pid].acp_ctrl);

	*off = len;
	return len;
}

static ssize_t write_proc_acp_ctrl(struct file *f, const char __user *buffer, size_t count, loff_t *off)
{
	uint8_t ubus_pid;
	char buf[16];
	unsigned long input_val;
	int len;
	void *data;

	if (count >= sizeof(buf))
		goto WRITE_PROC_ACP_EN_EXIT;

	len = min((unsigned int)count, (sizeof(buf) - 1));

	if (copy_from_user(buf, buffer, len))
		goto WRITE_PROC_ACP_EN_EXIT;

	buf[len] = '\0';
	if (strict_strtoul(buf, 0, &input_val))
		goto WRITE_PROC_ACP_EN_EXIT;

	data = PDE_DATA(file_inode(f));
	if (data == NULL)
		goto WRITE_PROC_ACP_EN_EXIT;

	ubus_pid = get_ubus_pid_by_proc_dir((struct proc_dir_entry *)data);
	if (ubus_pid == BCM_UBUS_PID_INVALID)
		goto WRITE_PROC_ACP_EN_EXIT;

	if ((uint32_t)input_val == acp_cfg_tbl[ubus_pid].acp_ctrl) {
		printk(KERN_WARNING "Nothing has been done\n");
		return count;
	}

	acp_cfg_tbl[ubus_pid].acp_ctrl = (uint32_t)input_val;
	acp_ctrl_set(ubus_pid, acp_cfg_tbl[ubus_pid].en);
	printk(KERN_WARNING "Done setting ACP ctrl for %s\n",
				acp_cfg_tbl[ubus_pid].name);

	return count;

WRITE_PROC_ACP_EN_EXIT:
	printk(KERN_WARNING "invalid input value\n");
	return count;
}

static ssize_t read_proc_addr_in(struct file *f, char *page, size_t cnt, loff_t *off)
{
	uint8_t ubus_pid, cfg_id;
	int len = 0;
	void *data;

	if(*off != 0)
		return 0;

	data = PDE_DATA(file_inode(f));
	if (data == NULL)
		return -EINVAL;
	
	ubus_pid = get_ubus_pid_by_proc_dir((struct proc_dir_entry *)data);
	if (unlikely(ubus_pid == BCM_UBUS_PID_INVALID))
		return -EINVAL;

	cfg_id = get_cfg_id_by_ubus_pid_proc_dir(ubus_pid,
			(struct proc_dir_entry *)data);
	if (unlikely(cfg_id >= BCM_UBUS_CFG_MAX))
		return -EINVAL;

	len = sprintf(page + len, "%p\n",
			(void *)acp_cfg_tbl[ubus_pid].ubus_cfg[cfg_id].addr_in);

	*off = len;
	return len;
}

static ssize_t write_proc_addr_in(struct file *f, const char __user *buffer, size_t count, loff_t *off)
{
	uint8_t ubus_pid, cfg_id;
	char buf[16];
	unsigned long input_val;
	int len, ret;
	void *data;

	if (count >= sizeof(buf))
		goto WRITE_PROC_ADDR_IN_EXIT;

	len = min((unsigned int)count, (sizeof(buf) - 1));

	if (copy_from_user(buf, buffer, len))
		goto WRITE_PROC_ADDR_IN_EXIT;

	buf[len] = '\0';
	if (strict_strtoul(buf, 0, &input_val))
		goto WRITE_PROC_ADDR_IN_EXIT;

	data = PDE_DATA(file_inode(f));
	if (data == NULL)
		goto WRITE_PROC_ADDR_IN_EXIT;

	ubus_pid = get_ubus_pid_by_proc_dir((struct proc_dir_entry *)data);
	if (ubus_pid == BCM_UBUS_PID_INVALID)
		goto WRITE_PROC_ADDR_IN_EXIT;

	cfg_id = get_cfg_id_by_ubus_pid_proc_dir(ubus_pid,
			(struct proc_dir_entry *)data);
	if (unlikely(cfg_id >= BCM_UBUS_CFG_MAX))
		goto WRITE_PROC_ADDR_IN_EXIT;

	if ((uint32_t)input_val == acp_cfg_tbl[ubus_pid].ubus_cfg[cfg_id].addr_in) {
		printk(KERN_WARNING "Nothing has been done\n");
		return count;
	}

	ret = ubus_cfg_entry_set(ubus_pid, cfg_id, (uint32_t)input_val,
			acp_cfg_tbl[ubus_pid].ubus_cfg[cfg_id].addr_out,
			acp_cfg_tbl[ubus_pid].ubus_cfg[cfg_id].dst_pid,
			acp_cfg_tbl[ubus_pid].ubus_cfg[cfg_id].size_shift,
			acp_cfg_tbl[ubus_pid].ubus_cfg[cfg_id].en);
	if (ret) {
		printk(KERN_WARNING "Fail to configure\n");
	} else {
		printk(KERN_WARNING "Done setting the new value\n");
		acp_cfg_tbl[ubus_pid].ubus_cfg[cfg_id].addr_in = (uint32_t)input_val;

	}
	return count;

WRITE_PROC_ADDR_IN_EXIT:
	printk(KERN_WARNING "invalid input value\n");
	return count;
}

static ssize_t read_proc_addr_out(struct file *f, char *page, size_t cnt, loff_t *off)
{
	uint8_t ubus_pid, cfg_id;
	int len = 0;
	void *data;

	if(*off != 0)
		return 0;

	data = PDE_DATA(file_inode(f));
	if (data == NULL)
		return -EINVAL;
	
	ubus_pid = get_ubus_pid_by_proc_dir((struct proc_dir_entry *)data);
	if (unlikely(ubus_pid == BCM_UBUS_PID_INVALID))
		return -EINVAL;

	cfg_id = get_cfg_id_by_ubus_pid_proc_dir(ubus_pid,
			(struct proc_dir_entry *)data);
	if (unlikely(cfg_id >= BCM_UBUS_CFG_MAX))
		return -EINVAL;

	len = sprintf(page + len, "%p\n",
			(void *)acp_cfg_tbl[ubus_pid].ubus_cfg[cfg_id].addr_out);

	*off = len;
	return len;
}

static ssize_t write_proc_addr_out(struct file *f, const char __user *buffer, size_t count, loff_t *off)
{
	uint8_t ubus_pid, cfg_id;
	char buf[16];
	unsigned long input_val;
	int len, ret;
	void *data;

	if (count >= sizeof(buf))
		goto WRITE_PROC_ADDR_OUT_EXIT;

	len = min((unsigned int)count, (sizeof(buf) - 1));

	if (copy_from_user(buf, buffer, len))
		goto WRITE_PROC_ADDR_OUT_EXIT;

	buf[len] = '\0';
	if (strict_strtoul(buf, 0, &input_val))
		goto WRITE_PROC_ADDR_OUT_EXIT;

	data = PDE_DATA(file_inode(f));
	if (data == NULL)
		goto WRITE_PROC_ADDR_OUT_EXIT;

	ubus_pid = get_ubus_pid_by_proc_dir((struct proc_dir_entry *)data);
	if (ubus_pid == BCM_UBUS_PID_INVALID)
		goto WRITE_PROC_ADDR_OUT_EXIT;

	cfg_id = get_cfg_id_by_ubus_pid_proc_dir(ubus_pid,
			(struct proc_dir_entry *)data);
	if (unlikely(cfg_id >= BCM_UBUS_CFG_MAX))
		goto WRITE_PROC_ADDR_OUT_EXIT;

	if ((uint32_t)input_val == acp_cfg_tbl[ubus_pid].ubus_cfg[cfg_id].addr_out) {
		printk(KERN_WARNING "Nothing has been done\n");
		return count;
	}

	ret = ubus_cfg_entry_set(ubus_pid, cfg_id,
			acp_cfg_tbl[ubus_pid].ubus_cfg[cfg_id].addr_in,
			(uint32_t)input_val,
			acp_cfg_tbl[ubus_pid].ubus_cfg[cfg_id].dst_pid,
			acp_cfg_tbl[ubus_pid].ubus_cfg[cfg_id].size_shift,
			acp_cfg_tbl[ubus_pid].ubus_cfg[cfg_id].en);
	if (ret) {
		printk(KERN_WARNING "Fail to configure\n");
	} else {
		printk(KERN_WARNING "Done setting the new value\n");
		acp_cfg_tbl[ubus_pid].ubus_cfg[cfg_id].addr_out = (uint32_t)input_val;

	}
	return count;

WRITE_PROC_ADDR_OUT_EXIT:
	printk(KERN_WARNING "invalid input value\n");
	return count;
}

static ssize_t read_proc_dst_pid(struct file *f, char *page, size_t cnt, loff_t *off)
{
	uint8_t ubus_pid, cfg_id;
	int len = 0;
	void *data;

	if(*off != 0)
		return 0;

	data = PDE_DATA(file_inode(f));
	if (data == NULL)
		return -EINVAL;
	
	ubus_pid = get_ubus_pid_by_proc_dir((struct proc_dir_entry *)data);
	if (unlikely(ubus_pid == BCM_UBUS_PID_INVALID))
		return -EINVAL;

	cfg_id = get_cfg_id_by_ubus_pid_proc_dir(ubus_pid,
			(struct proc_dir_entry *)data);
	if (unlikely(cfg_id >= BCM_UBUS_CFG_MAX))
		return -EINVAL;

	len = sprintf(page + len, "%u\n",
			(unsigned)acp_cfg_tbl[ubus_pid].ubus_cfg[cfg_id].dst_pid);

	*off = len;
	return len;
}

static ssize_t write_proc_dst_pid(struct file *f, const char __user *buffer, size_t count, loff_t *off)
{
	uint8_t ubus_pid, cfg_id;
	char buf[16];
	unsigned long input_val;
	int len, ret;
	void *data;

	if (count >= sizeof(buf))
		goto WRITE_PROC_SIZE_SHIFT_EXIT;

	len = min((unsigned int)count, (sizeof(buf) - 1));

	if (copy_from_user(buf, buffer, len))
		goto WRITE_PROC_SIZE_SHIFT_EXIT;

	buf[len] = '\0';
	if (strict_strtoul(buf, 0, &input_val))
		goto WRITE_PROC_SIZE_SHIFT_EXIT;

	data = PDE_DATA(file_inode(f));
	if (data == NULL)
		goto WRITE_PROC_SIZE_SHIFT_EXIT;

	ubus_pid = get_ubus_pid_by_proc_dir((struct proc_dir_entry *)data);
	if (ubus_pid == BCM_UBUS_PID_INVALID)
		goto WRITE_PROC_SIZE_SHIFT_EXIT;

	cfg_id = get_cfg_id_by_ubus_pid_proc_dir(ubus_pid,
			(struct proc_dir_entry *)data);
	if (unlikely(cfg_id >= BCM_UBUS_CFG_MAX))
		goto WRITE_PROC_SIZE_SHIFT_EXIT;

	if (acp_cfg_tbl[ubus_pid].ubus_cfg[cfg_id].dst_pid ==
			(uint8_t)input_val) {
		printk(KERN_WARNING "Nothing has been done\n");
		return count;
	}

	ret = ubus_cfg_entry_set(ubus_pid, cfg_id,
			acp_cfg_tbl[ubus_pid].ubus_cfg[cfg_id].addr_in,
			acp_cfg_tbl[ubus_pid].ubus_cfg[cfg_id].addr_out,
			(uint8_t)input_val,
			acp_cfg_tbl[ubus_pid].ubus_cfg[cfg_id].size_shift,
			acp_cfg_tbl[ubus_pid].ubus_cfg[cfg_id].en);
	if (ret) {
		printk(KERN_WARNING "Fail to configure\n");
	} else {
		printk(KERN_WARNING "Done setting the new value\n");
		acp_cfg_tbl[ubus_pid].ubus_cfg[cfg_id].dst_pid =
			(uint8_t)input_val;

	}
	return count;

WRITE_PROC_SIZE_SHIFT_EXIT:
	printk(KERN_WARNING "invalid input value\n");
	return count;
}

static ssize_t read_proc_size_shift(struct file *f, char *page, size_t cnt, loff_t *off)
{
	uint8_t ubus_pid, cfg_id;
	int len = 0;
	void *data;

	if(*off != 0)
 		return 0;

	data = PDE_DATA(file_inode(f));
	if (data == NULL)
		return -EINVAL;
	
	ubus_pid = get_ubus_pid_by_proc_dir((struct proc_dir_entry *)data);
	if (unlikely(ubus_pid == BCM_UBUS_PID_INVALID))
		return -EINVAL;

	cfg_id = get_cfg_id_by_ubus_pid_proc_dir(ubus_pid,
			(struct proc_dir_entry *)data);
	if (unlikely(cfg_id >= BCM_UBUS_CFG_MAX))
		return -EINVAL;

	len = sprintf(page + len, "%u\n",
			(unsigned)acp_cfg_tbl[ubus_pid].ubus_cfg[cfg_id].size_shift);

	*off = len;
	return len;
}

static ssize_t write_proc_size_shift(struct file *f, const char __user *buffer, size_t count, loff_t *off)
{
	uint8_t ubus_pid, cfg_id;
	char buf[16];
	unsigned long input_val;
	int len, ret;
	void *data;

	if (count >= sizeof(buf))
		goto WRITE_PROC_SIZE_SHIFT_EXIT;

	len = min((unsigned int)count, (sizeof(buf) - 1));

	if (copy_from_user(buf, buffer, len))
		goto WRITE_PROC_SIZE_SHIFT_EXIT;

	buf[len] = '\0';
	if (strict_strtoul(buf, 0, &input_val))
		goto WRITE_PROC_SIZE_SHIFT_EXIT;

	data = PDE_DATA(file_inode(f));
	if (data == NULL)
		goto WRITE_PROC_SIZE_SHIFT_EXIT;

	ubus_pid = get_ubus_pid_by_proc_dir((struct proc_dir_entry *)data);
	if (ubus_pid == BCM_UBUS_PID_INVALID)
		goto WRITE_PROC_SIZE_SHIFT_EXIT;

	cfg_id = get_cfg_id_by_ubus_pid_proc_dir(ubus_pid,
			(struct proc_dir_entry *)data);
	if (unlikely(cfg_id >= BCM_UBUS_CFG_MAX))
		goto WRITE_PROC_SIZE_SHIFT_EXIT;

	if (acp_cfg_tbl[ubus_pid].ubus_cfg[cfg_id].size_shift ==
			(uint8_t)input_val) {
		printk(KERN_WARNING "Nothing has been done\n");
		return count;
	}

	ret = ubus_cfg_entry_set(ubus_pid, cfg_id,
			acp_cfg_tbl[ubus_pid].ubus_cfg[cfg_id].addr_in,
			acp_cfg_tbl[ubus_pid].ubus_cfg[cfg_id].addr_out,
			acp_cfg_tbl[ubus_pid].ubus_cfg[cfg_id].dst_pid,
			(uint8_t)input_val,
			acp_cfg_tbl[ubus_pid].ubus_cfg[cfg_id].en);
	if (ret) {
		printk(KERN_WARNING "Fail to configure\n");
	} else {
		printk(KERN_WARNING "Done setting the new value\n");
		acp_cfg_tbl[ubus_pid].ubus_cfg[cfg_id].size_shift =
			(uint8_t)input_val;

	}
	return count;

WRITE_PROC_SIZE_SHIFT_EXIT:
	printk(KERN_WARNING "invalid input value\n");
	return count;
}

static ssize_t read_proc_cfg_en(struct file *f, char *page, size_t cnt, loff_t *off)
{
	uint8_t ubus_pid, cfg_id;
	int len = 0;
	void *data;

	if(*off != 0)
		return 0;

	data = PDE_DATA(file_inode(f));
	if (data == NULL)
		return -EINVAL;
	
	ubus_pid = get_ubus_pid_by_proc_dir((struct proc_dir_entry *)data);
	if (unlikely(ubus_pid == BCM_UBUS_PID_INVALID))
		return -EINVAL;

	cfg_id = get_cfg_id_by_ubus_pid_proc_dir(ubus_pid,
			(struct proc_dir_entry *)data);
	if (unlikely(cfg_id >= BCM_UBUS_CFG_MAX))
		return -EINVAL;

	len = sprintf(page + len, "%u\n",
			(unsigned)acp_cfg_tbl[ubus_pid].ubus_cfg[cfg_id].en);

	*off = len;
	return len;
}

static ssize_t write_proc_cfg_en(struct file *f, const char __user *buffer, size_t count, loff_t *off)
{
	uint8_t ubus_pid, cfg_id;
	char buf[16];
	unsigned long input_val;
	int len, ret;
	void *data;

	if (count >= sizeof(buf))
		goto WRITE_PROC_ENABLE_EXIT;

	len = min((unsigned int)count, (sizeof(buf) - 1));

	if (copy_from_user(buf, buffer, len))
		goto WRITE_PROC_ENABLE_EXIT;

	buf[len] = '\0';
	if (strict_strtoul(buf, 0, &input_val))
		goto WRITE_PROC_ENABLE_EXIT;

	data = PDE_DATA(file_inode(f));
	if (data == NULL)
		goto WRITE_PROC_ENABLE_EXIT;

	ubus_pid = get_ubus_pid_by_proc_dir((struct proc_dir_entry *)data);
	if (ubus_pid == BCM_UBUS_PID_INVALID)
		goto WRITE_PROC_ENABLE_EXIT;

	cfg_id = get_cfg_id_by_ubus_pid_proc_dir(ubus_pid,
			(struct proc_dir_entry *)data);
	if (unlikely(cfg_id >= BCM_UBUS_CFG_MAX))
		goto WRITE_PROC_ENABLE_EXIT;

	if (acp_cfg_tbl[ubus_pid].ubus_cfg[cfg_id].en == (uint8_t)input_val) {
		printk(KERN_WARNING "Nothing has been done\n");
		return count;
	}

	ret = ubus_cfg_entry_set(ubus_pid, cfg_id,
			acp_cfg_tbl[ubus_pid].ubus_cfg[cfg_id].addr_in,
			acp_cfg_tbl[ubus_pid].ubus_cfg[cfg_id].addr_out,
			acp_cfg_tbl[ubus_pid].ubus_cfg[cfg_id].dst_pid,
			acp_cfg_tbl[ubus_pid].ubus_cfg[cfg_id].size_shift,
			(uint8_t)input_val);
	if (ret) {
		printk(KERN_WARNING "Fail to configure\n");
	} else {
		printk(KERN_WARNING "Done setting the new value\n");
		acp_cfg_tbl[ubus_pid].ubus_cfg[cfg_id].en = (uint8_t)input_val;

	}
	return count;

WRITE_PROC_ENABLE_EXIT:
	printk(KERN_WARNING "invalid input value\n");
	return count;
}

static struct file_operations acp_en_proc_fops = {
	.read = read_proc_acp_en,
	.write = write_proc_acp_en,
};

static struct file_operations acp_ctrl_proc_fops = {
	.read = read_proc_acp_ctrl,
	.write = write_proc_acp_ctrl,
};

static struct file_operations addr_in_proc_fops = {
	.read = read_proc_addr_in,
	.write = write_proc_addr_in,
};

static struct file_operations addr_out_proc_fops = {
	.read = read_proc_addr_out,
	.write = write_proc_addr_out,
};

static struct file_operations dst_pid_proc_fops = {
	.read = read_proc_dst_pid,
	.write = write_proc_dst_pid,
};

static struct file_operations size_shift_proc_fops = {
	.read = read_proc_size_shift,
	.write = write_proc_size_shift,
};

static struct file_operations cfg_en_proc_fops = {
	.read = read_proc_cfg_en,
	.write = write_proc_cfg_en,
};

static void create_proc_dir_file(uint8_t ubus_pid)
{
	int i;
	char buff[10];
	acp_cfg_entry_t *cur_cfg = &acp_cfg_tbl[ubus_pid];
	struct proc_dir_entry *res;

	if (cur_cfg->proc_dir == NULL)
		cur_cfg->proc_dir = proc_mkdir(cur_cfg->name, proc_acp_dir);

	if (cur_cfg->proc_dir == NULL) {
		printk(KERN_ERR "fail to create proc dir (%s)\n",
				cur_cfg->name);
		return;
	}

	/* proc file for acp_en */
	res = proc_create_data("acp_enable", S_IRUGO | S_IWUGO, cur_cfg->proc_dir, 
			&acp_en_proc_fops, (void *)cur_cfg->proc_dir);

	if (!res) {
		printk(KERN_ERR "fail to create proc file (%s)"
				"->acp_enable\n", cur_cfg->name);
	}

	/* proc file for acp_ctrl */
	res = proc_create_data("acp_ctrl", S_IRUGO | S_IWUGO, cur_cfg->proc_dir,
			&acp_ctrl_proc_fops, (void *)cur_cfg->proc_dir);

	if (!res) {
		printk(KERN_ERR "fail to create proc file (%s)"
				"->acp_ctrl\n", cur_cfg->name);
	}

	for (i = 0; i < BCM_UBUS_CFG_MAX; i++) {
		if (cur_cfg->ubus_cfg[i].proc_dir == NULL) {
			sprintf(buff, "cfg%d", i);
			cur_cfg->ubus_cfg[i].proc_dir = proc_mkdir(buff,
					cur_cfg->proc_dir);
		}

		/* supposedly shouldn't happen */
		if (unlikely(cur_cfg->ubus_cfg[i].proc_dir == NULL)) {
			printk(KERN_ERR "fail to create proc dir (%s)\n", buff);
			return;
		}

		/* proc file for addr_in */
		res = proc_create_data("addr_in", S_IRUGO | S_IWUGO, cur_cfg->ubus_cfg[i].proc_dir,
				&addr_in_proc_fops, (void *)cur_cfg->ubus_cfg[i].proc_dir);

		if (!res) {
			printk(KERN_ERR "fail to create proc file (%s)"
					"->addr_in\n", buff);
		}

		/* proc file for addr_out */
		res = proc_create_data("addr_out", S_IRUGO | S_IWUGO, cur_cfg->ubus_cfg[i].proc_dir,
				&addr_out_proc_fops, (void *)cur_cfg->ubus_cfg[i].proc_dir);

		if (!res) {
			printk(KERN_ERR "fail to create proc file (%s)"
					"->addr_out\n", buff);
		}

		/* proc file for dst_pid */
		res = proc_create_data("dst_pid", S_IRUGO | S_IWUGO, cur_cfg->ubus_cfg[i].proc_dir,
				&dst_pid_proc_fops, (void *)cur_cfg->ubus_cfg[i].proc_dir);

		if (!res) {
			printk(KERN_ERR "fail to create proc file (%s)"
					"->dst_pid\n", buff);
		}

		/* proc file for size_shift */
		res = proc_create_data("size_shift", S_IRUGO | S_IWUGO, cur_cfg->ubus_cfg[i].proc_dir,
				&size_shift_proc_fops, (void *)cur_cfg->ubus_cfg[i].proc_dir);

		if (!res) {
			printk(KERN_ERR "fail to create proc file (%s)"
					"->size_shift\n", buff);
		}

		/* proc file for enable */
		res = proc_create_data("config_enable", S_IRUGO | S_IWUGO, cur_cfg->ubus_cfg[i].proc_dir,
				&cfg_en_proc_fops, (void *)cur_cfg->ubus_cfg[i].proc_dir);

		if (!res) {
			printk(KERN_ERR "fail to create proc file (%s)"
					"->en\n", buff);
		}
	}
}

static void remove_proc_dir_file(uint8_t ubus_pid)
{
	int i;
	char buff[10];
	acp_cfg_entry_t *cur_cfg = &acp_cfg_tbl[ubus_pid];

	for (i = 0; i < BCM_UBUS_CFG_MAX; i++) {
		remove_proc_entry("addr_in", cur_cfg->ubus_cfg[i].proc_dir);
		remove_proc_entry("addr_out", cur_cfg->ubus_cfg[i].proc_dir);
		remove_proc_entry("dst_pid", cur_cfg->ubus_cfg[i].proc_dir);
		remove_proc_entry("size_shift", cur_cfg->ubus_cfg[i].proc_dir);
		remove_proc_entry("config_enable", cur_cfg->ubus_cfg[i].proc_dir);
		sprintf(buff, "cfg%d", i);
		remove_proc_entry(buff, cur_cfg->proc_dir);
	}
	remove_proc_entry("acp_enable", cur_cfg->proc_dir);
	remove_proc_entry("acp_ctrl", cur_cfg->proc_dir);
	remove_proc_entry(cur_cfg->name, proc_acp_dir);
}

static void acp_proc_file_init(void)
{
	uint8_t i;

	if (proc_acp_dir == NULL)
		proc_acp_dir = proc_mkdir("driver/acp", NULL);

	if (proc_acp_dir == NULL) {
		printk(KERN_ERR "fail to create proc dir driver/acp\n");
		return;
	}

	for (i = 0; i < BCM_UBUS_PID_MAX; i++) {
		if (acp_cfg_tbl[i].pmb_addr != 0)
			create_proc_dir_file(i);
	}
}

static void acp_proc_file_deinit(void)
{
	uint8_t i;
	for (i = 0; i < BCM_UBUS_PID_MAX; i++) {
		if (acp_cfg_tbl[i].pmb_addr != 0)
			remove_proc_dir_file(i);
	}

	if (proc_acp_dir)
		remove_proc_entry("driver/acp", NULL);
	proc_acp_dir = NULL;
}

static void acp_cfg_tbl_deinit(void)
{
	memset(acp_cfg_tbl, 0x0, BCM_UBUS_PID_MAX * sizeof(acp_cfg_entry_t));
}

/* size will be in the multiple of MB */
/* some limitations of the resulted value.  For each entry, addr must be in
 * multiple of the size. An invalid example is, if addr is 0x800000, and size
 * is 0x1600000.  HW will not be able to process it. */
static int ubus_cfg_convert(uint32_t addr_start, uint32_t size,
		uint32_t *addr_in, uint32_t *addr_out,
		uint8_t *dst_pid, uint8_t *size_shift)
{
	int used = 0, cur_bit_to_add;
	uint32_t added_size = 0, extra_added = 0;

	/* first method, go from the least significant bit set in the address
	 * to add the size into the table.  Then go from remaining size to add
	 * from the most significant bit */
	do {
		cur_bit_to_add = ffs((addr_start + added_size)) - 1;
		if ((0x1 << cur_bit_to_add) > (size - added_size))
			cur_bit_to_add = fls((size - added_size)) - 1;
		addr_in[used] = addr_start + added_size;
		addr_out[used] = addr_start + added_size;
		dst_pid[used] = BCM_UBUS_PID_ARMAXIACP;
		size_shift[used] = cur_bit_to_add; 
		added_size += 0x1 << cur_bit_to_add;
		used++;
	} while ((used < 4) && ((size - added_size) != 0));

	if ((size - added_size) == 0)
		return used;

	/* second method, add the total and subtract those that should go DDR */
	added_size = 0;
	extra_added = 0;
	used = 0;
	do {
		if (extra_added != 0) {
			cur_bit_to_add = fls(extra_added) - 1;
			if ((cur_bit_to_add >= 2) &&
					((0x1 << cur_bit_to_add) & extra_added) &&
					((0x1 << (cur_bit_to_add - 1)) & extra_added) &&
					((0x1 << (cur_bit_to_add - 2)) & extra_added)) {
				cur_bit_to_add++;
				extra_added = 0;
			} else {
				extra_added -= 0x1 << cur_bit_to_add;
			}
			added_size -= 0x1 << cur_bit_to_add;
			addr_in[used] = added_size;
			addr_out[used] = added_size;
			dst_pid[used] = BCM_UBUS_PID_DDR;
			size_shift[used] = cur_bit_to_add; 
			used++;
		} else {
			cur_bit_to_add = fls((addr_start + size - added_size)) - 1;
			if (0x1 << (cur_bit_to_add - 1) & (addr_start + size - added_size)) {
				cur_bit_to_add++;
				extra_added = (0x1 << cur_bit_to_add) + added_size - addr_start - size;
			}
			addr_in[used] = added_size;
			addr_out[used] = added_size;
			dst_pid[used] = BCM_UBUS_PID_ARMAXIACP;
			size_shift[used] = cur_bit_to_add; 
			added_size += 0x1 << cur_bit_to_add;
			used++;
		}
	} while ((used < 4) && ((addr_start + size) != added_size));

	if ((addr_start + size) != added_size) {
		printk("BCM63XX ACP ERROR!: please define a new ACP_MEM_SIZE\n");
		return -1;
	}

	added_size = 0;
	while ((used < 4) && (addr_start - added_size)) {
		cur_bit_to_add = fls((addr_start - added_size)) - 1;
		addr_in[used] = added_size;
		addr_out[used] = added_size;
		dst_pid[used] = BCM_UBUS_PID_DDR;
		size_shift[used] = cur_bit_to_add; 
		added_size += 0x1 << cur_bit_to_add;
		used++;
	}

	if ((addr_start - added_size) == 0)
		return used;

	/* TODO: Maybe other way to fill the table entry? */

	printk("BCM63XX ACP ERROR!: please define a new ACP_MEM_SIZE\n");
	return -1;
}

int bcm63xx_acp_init(void)
{
	uint32_t addr_in[4], addr_out[4];
	uint8_t size_shift[4], dst_pid[4];
	int entry_use;
	struct zone *acp_zone = &NODE_DATA(0)->node_zones[ZONE_ACP];

	printk("BCM63XX ACP: zone_acp start at 0x%08lx of size %d MB\n",
		(acp_zone->zone_start_pfn << PAGE_SHIFT),
		CONFIG_BCM_ACP_MEM_SIZE);

	memset(addr_in, 0, sizeof(uint32_t) << 2);
	memset(addr_out, 0, sizeof(uint32_t) << 2);
	memset(size_shift, 0, sizeof(uint8_t) << 2);
	memset(dst_pid, 0, sizeof(uint8_t) << 2);
	entry_use = ubus_cfg_convert(acp_zone->zone_start_pfn << PAGE_SHIFT,
		CONFIG_BCM_ACP_MEM_SIZE * SZ_1M, addr_in, addr_out, dst_pid,
		size_shift);

	if (entry_use == -1)
		return -EPERM;

	acp_cfg_tbl_init(entry_use, addr_in, addr_out, dst_pid, size_shift);
	acp_proc_file_init();

	bcm63xx_acp_enable(BCM_UBUS_PID_RNR);
	bcm63xx_acp_enable(BCM_UBUS_PID_RNR_RABR);
	bcm63xx_acp_enable(BCM_UBUS_PID_RNR_RBBR);
	bcm63xx_acp_enable(BCM_UBUS_PID_SAR);
	bcm63xx_acp_enable(BCM_UBUS_PID_SAR2);
	return 0;
}

void bcm63xx_acp_exit(void)
{
	bcm63xx_acp_disable(BCM_UBUS_PID_RNR);
	bcm63xx_acp_disable(BCM_UBUS_PID_RNR_RABR);
	bcm63xx_acp_disable(BCM_UBUS_PID_RNR_RBBR);
	bcm63xx_acp_disable(BCM_UBUS_PID_SAR);
	bcm63xx_acp_disable(BCM_UBUS_PID_SAR2);

	acp_proc_file_deinit();
	acp_cfg_tbl_deinit();
}

module_init(bcm63xx_acp_init);
module_exit(bcm63xx_acp_exit);
#endif /* defined(CONFIG_BCM_KF_ARM_BCM963XX) */
