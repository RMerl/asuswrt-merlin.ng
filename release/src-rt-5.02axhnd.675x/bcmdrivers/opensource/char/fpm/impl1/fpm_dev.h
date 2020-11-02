/*
* <:copyright-BRCM:2015:DUAL/GPL:standard
* 
*    Copyright (c) 2015 Broadcom 
*    All Rights Reserved
* 
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed
* to you under the terms of the GNU General Public License version 2
* (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
* with the following added to such license:
* 
*    As a special exception, the copyright holders of this software give
*    you permission to link this software with independent modules, and
*    to copy and distribute the resulting executable under terms of your
*    choice, provided that you also meet, for each linked independent
*    module, the terms and conditions of the license of that module.
*    An independent module is a module which is not derived from this
*    software.  The special exception does not apply to any modifications
*    of the software.
* 
* Not withstanding the above, under no circumstances may you combine
* this software in any way with any other Broadcom software provided
* under a license other than the GPL, without Broadcom's express prior
* written consent.
* 
* :> 
*/

/*****************************************************************************
 *
 * Copyright (c) 2013 Broadcom Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * Author: Tim Ross <tross@broadcom.com>
 *****************************************************************************/
#ifndef _FPM_DEV_H_
#define _FPM_DEV_H_

#include <linux/types.h>
#include <linux/skbuff.h>
#include "fpm.h"

extern struct fpmdev *fpm;
extern spinlock_t fpm_reg_lock;
extern unsigned long isr_timer_period;

static inline u32 fpm_reg_read(u32 *reg)
{
	unsigned long flags;
	u32 val;

	spin_lock_irqsave(&fpm_reg_lock, flags);
	val = __raw_readl(reg);
	spin_unlock_irqrestore(&fpm_reg_lock, flags);
	return val;
}

static inline void fpm_reg_write(u32 *reg, u32 val)
{
	unsigned long flags;

	spin_lock_irqsave(&fpm_reg_lock, flags);
	__raw_writel(val, reg);
	spin_unlock_irqrestore(&fpm_reg_lock, flags);
}

static inline void fpm_reg_write_mask(u32 *reg, u32 mask, u32 val)
{
	unsigned long flags;
	u32 v;

	spin_lock_irqsave(&fpm_reg_lock, flags);
	v = __raw_readl(reg);
	v &= ~mask;
	v |= (val & mask);
	__raw_writel(v, reg);
	spin_unlock_irqrestore(&fpm_reg_lock, flags);
}

/* Ctl reg */
#define FPM_CTL_POOL1_CFG1		\
	(fpm->reg_vbase + ((offsetof(FpmControl, ctrl) + offsetof(FpmCtrl, pool1_cfg1)) >> 2))
#define FPM_SET_CHUNK_SIZE(val)	\
	(fpm_reg_write_mask(FPM_CTL_POOL1_CFG1, \
	 FPMCTRL_FP_BUF_SIZE_MASK, \
	 (val) << FPMCTRL_FP_BUF_SIZE_SHIFT))
#define FPM_GET_CHUNK_SIZE()	\
	((fpm_reg_read(FPM_CTL_POOL1_CFG1) & \
	FPMCTRL_FP_BUF_SIZE_MASK) >> \
	FPMCTRL_FP_BUF_SIZE_SHIFT)
#define FPM_CHUNK_SIZE_512	0
#define FPM_CHUNK_SIZE_256	1

#define FPM_CTL_FPM_CTL		\
	(fpm->reg_vbase + ((offsetof(FpmControl, ctrl) + offsetof(FpmCtrl, fpm_ctl)) >> 2))
#define FPM_SET_POOL_INIT(pool) \
{ \
	if ((pool) == 0) \
		fpm_reg_write_mask(FPM_CTL_FPM_CTL, \
		FPMCTRL_FPM_CTRL_INIT_MEM_MASK, \
		FPMCTRL_FPM_CTRL_INIT_MEM_MASK); \
	else \
		fpm_reg_write_mask(FPM_CTL_FPM_CTL, \
		FPMCTRL_FPM_CTRL_INIT_MEM_POOL2_MASK, \
		FPMCTRL_FPM_CTRL_INIT_MEM_POOL2_MASK); \
}
#define FPM_GET_POOL_INIT(pool) \
	(((pool) == 0) ? ((fpm_reg_read(FPM_CTL_FPM_CTL) & \
			   FPMCTRL_FPM_CTRL_INIT_MEM_MASK) >> \
			   FPMCTRL_FPM_CTRL_INIT_MEM_SHIFT) : \
			 ((fpm_reg_read(FPM_CTL_FPM_CTL) & \
			   FPMCTRL_FPM_CTRL_INIT_MEM_POOL2_MASK) >> \
			   FPMCTRL_FPM_CTRL_INIT_MEM_POOL2_SHIFT))

#define FPM_SET_POOL_ENABLE(pool) \
{ \
	if ((pool) == 0) \
		fpm_reg_write_mask(FPM_CTL_FPM_CTL, \
		FPMCTRL_FPM_CTRL_POOL1_ENABLE_MASK, \
		FPMCTRL_FPM_CTRL_POOL1_ENABLE_MASK); \
	else \
		fpm_reg_write_mask(FPM_CTL_FPM_CTL, \
		FPMCTRL_FPM_CTRL_POOL2_ENABLE_MASK, \
		FPMCTRL_FPM_CTRL_POOL2_ENABLE_MASK); \
}

#define FPM_SET_BB_RESET(reset) \
{ \
	fpm_reg_write_mask(FPM_CTL_FPM_CTL, \
		FPMCTRL_FPM_CTRL_SOFT_RESET_MASK, \
		((reset) << FPMCTRL_FPM_CTRL_SOFT_RESET_SHIFT)); \
}

/* Base regs */
#define FPM_CTL_POOL1_CFG2	\
	(fpm->reg_vbase + ((offsetof(FpmControl, ctrl) + offsetof(FpmCtrl, pool1_cfg2)) >> 2))
#define FPM_CTL_POOL1_CFG3	\
	(fpm->reg_vbase + ((offsetof(FpmControl, ctrl) + offsetof(FpmCtrl, pool1_cfg3)) >> 2))

#define FPM_SET_POOL_BASE_ADDR(pool, val) \
{ \
	if ((pool) == 0) \
		fpm_reg_write(FPM_CTL_POOL1_CFG2, (val)); \
	else \
		fpm_reg_write(FPM_CTL_POOL1_CFG3, (val)); \
}
#define FPM_GET_POOL_BASE_ADDR(pool) \
	(((pool) == 0) ? \
		fpm_reg_read(FPM_CTL_POOL1_CFG2) & \
		FPMCTRL_FPM_POOL_BASE_ADDRESS_MASK : \
		fpm_reg_read(FPM_CTL_POOL1_CFG3) & \
		FPMCTRL_FPM_POOL_BASE_ADDRESS_MASK)

/* Pool weight regs */
#define FPM_CTL_FPM_WEIGHT		\
	(fpm->reg_vbase + ((offsetof(FpmControl, ctrl) + offsetof(FpmCtrl, fpm_weight)) >> 2))
#define FPM_SET_ALLOC_WEIGHT(pool, weight) \
{ \
	if ((pool) == 0) \
		fpm_reg_write_mask(FPM_CTL_FPM_WEIGHT, \
		FPMCTRL_FPM_WEIGHT_DDR0_ALLOC_MASK, \
		(weight) << FPMCTRL_FPM_WEIGHT_DDR0_ALLOC_SHIFT); \
	else \
		fpm_reg_write_mask(FPM_CTL_FPM_WEIGHT, \
		FPMCTRL_FPM_WEIGHT_DDR1_ALLOC_MASK, \
		(weight) << FPMCTRL_FPM_WEIGHT_DDR1_ALLOC_SHIFT); \
}
#define FPM_GET_ALLOC_WEIGHT(pool) \
	(((pool) == 0) ? ((fpm_reg_read(FPM_CTL_FPM_WEIGHT) & \
			   FPMCTRL_FPM_WEIGHT_DDR0_ALLOC_MASK) >> \
			   FPMCTRL_FPM_WEIGHT_DDR0_ALLOC_SHIFT) : \
			 ((fpm_reg_read(FPM_CTL_FPM_WEIGHT) & \
			   FPMCTRL_FPM_WEIGHT_DDR1_ALLOC_MASK) >> \
			   FPMCTRL_FPM_WEIGHT_DDR1_ALLOC_SHIFT))
#define FPM_SET_FREE_WEIGHT(pool, weight) \
{ \
	if ((pool) == 0) \
		fpm_reg_write_mask(FPM_CTL_FPM_WEIGHT, \
		FPMCTRL_FPM_WEIGHT_DDR0_FREE_MASK, \
		(weight) << FPMCTRL_FPM_WEIGHT_DDR0_FREE_SHIFT); \
	else \
		fpm_reg_write_mask(FPM_CTL_FPM_WEIGHT, \
		FPMCTRL_FPM_WEIGHT_DDR1_FREE_MASK, \
		(weight) << FPMCTRL_FPM_WEIGHT_DDR1_FREE_SHIFT); \
}
#define FPM_GET_FREE_WEIGHT(pool) \
	(((pool) == 0) ? ((fpm_reg_read(FPM_CTL_FPM_WEIGHT) & \
			   FPMCTRL_FPM_WEIGHT_DDR0_FREE_MASK) >> \
			   FPMCTRL_FPM_WEIGHT_DDR0_FREE_SHIFT) : \
			 ((fpm_reg_read(FPM_CTL_FPM_WEIGHT) & \
			   FPMCTRL_FPM_WEIGHT_DDR1_FREE_MASK) >> \
			   FPMCTRL_FPM_WEIGHT_DDR1_FREE_SHIFT))

/* Broadbus allocation pool selection */
#define FPM_CTL_FPM_BB_CFG		\
	(fpm->reg_vbase + ((offsetof(FpmControl, ctrl) + offsetof(FpmCtrl, fpm_bb_cfg)) >> 2))
#define FPM_BB_POOL_SEL_POOL0		FPMCTRL_FPM_BB_CFG_DDR0
#define FPM_BB_POOL_SEL_POOL1		FPMCTRL_FPM_BB_CFG_DDR1
#define FPM_BB_POOL_SEL_POOL_BOTH	FPMCTRL_FPM_BB_CFG_DDR_BOTH

#define FPM_GET_BB_POOL_SEL()		\
	fpm_reg_read(FPM_CTL_FPM_BB_CFG)
#define FPM_SET_BB_POOL_SEL(val)	\
	fpm_reg_write(FPM_CTL_FPM_BB_CFG, (val))

/* IRQ regs */
#define FPM_POOL1_INTR_MASK		\
	(fpm->reg_vbase + ((offsetof(FpmControl, ctrl) + offsetof(FpmCtrl, pool1_intr_msk)) >> 2))
#define FPM_POOL2_INTR_MASK		\
	(fpm->reg_vbase + ((offsetof(FpmControl, ctrl) + offsetof(FpmCtrl, pool2_intr_msk)) >> 2))
#define FPM_GET_IRQ_MASK(pool) \
	(((pool) == 0) ? fpm_reg_read(FPM_POOL1_INTR_MASK) : \
			 fpm_reg_read(FPM_POOL2_INTR_MASK))
#define FPM_SET_IRQ_MASK(pool, mask) \
{ \
	if ((pool) == 0) \
		fpm_reg_write(FPM_POOL1_INTR_MASK, (mask)); \
	else \
		fpm_reg_write(FPM_POOL2_INTR_MASK, (mask)); \
}

#define FPM_POOL1_INTR_STS		\
	(fpm->reg_vbase + ((offsetof(FpmControl, ctrl) + offsetof(FpmCtrl, pool1_intr_sts)) >> 2))
#define FPM_POOL2_INTR_STS		\
	(fpm->reg_vbase + ((offsetof(FpmControl, ctrl) + offsetof(FpmCtrl, pool2_intr_sts)) >> 2))
#define FPM_GET_IRQ_STATUS(pool) \
	(((pool) == 0) ? fpm_reg_read(FPM_POOL1_INTR_STS) : \
			 fpm_reg_read(FPM_POOL2_INTR_STS))
#define FPM_CLEAR_IRQ_STATUS(pool, mask) \
{ \
	if ((pool) == 0) \
		fpm_reg_write(FPM_POOL1_INTR_STS, (mask)); \
	else \
		fpm_reg_write(FPM_POOL2_INTR_STS, (mask)); \
}

/* Spare regs */
#define FPM_CTL_FPM_SPARE		\
	(fpm->reg_vbase + ((offsetof(FpmControl, ctrl) + offsetof(FpmCtrl, spare)) >> 2))
#define FPM_SET_CTL_SPARE(val)	(fpm_reg_write(FPM_CTL_FPM_SPARE, val))

/* Pool management regs */
#define FPM_ALLOC_8_CHUNKS_BASE(base)	\
	((base) + ((offsetof(FpmControl, pool) + offsetof(FpmPoolMgmt, pool1_alloc_dealloc)) >> 2))
#define FPM_ALLOC_4_CHUNKS_BASE(base)	\
	((base) + ((offsetof(FpmControl, pool) + offsetof(FpmPoolMgmt, pool2_alloc_dealloc)) >> 2))
#define FPM_ALLOC_2_CHUNKS_BASE(base)	\
	((base) + ((offsetof(FpmControl, pool) + offsetof(FpmPoolMgmt, pool3_alloc_dealloc)) >> 2))
#define FPM_ALLOC_1_CHUNKS_BASE(base)	\
	((base) + ((offsetof(FpmControl, pool) + offsetof(FpmPoolMgmt, pool4_alloc_dealloc)) >> 2))
#define FPM_DEALLOC_BASE(base)	FPM_ALLOC_8_CHUNKS_BASE(base)
#define FPM_POOL_MULTI_BASE(base)	\
	((base) + ((offsetof(FpmControl, pool) + offsetof(FpmPoolMgmt, pool_multi)) >> 2))
#define FPM_ALLOC_8_CHUNKS	\
	FPM_ALLOC_8_CHUNKS_BASE(fpm->reg_vbase)
#define FPM_ALLOC_4_CHUNKS	\
	FPM_ALLOC_4_CHUNKS_BASE(fpm->reg_vbase)
#define FPM_ALLOC_2_CHUNKS	\
	FPM_ALLOC_2_CHUNKS_BASE(fpm->reg_vbase)
#define FPM_ALLOC_1_CHUNKS	\
	FPM_ALLOC_1_CHUNKS_BASE(fpm->reg_vbase)
#define FPM_DEALLOC		\
	FPM_DEALLOC_BASE(fpm->reg_vbase)
#define FPM_POOL_MULTI	\
	FPM_POOL_MULTI_BASE(fpm->reg_vbase)
#define FPM_ALLOC_8_CHUNKS_PHYS	\
	((unsigned long)FPM_ALLOC_8_CHUNKS_BASE((u32 *)fpm->reg_pbase))
#define FPM_ALLOC_4_CHUNKS_PHYS	\
	((unsigned long)FPM_ALLOC_4_CHUNKS_BASE((u32 *)fpm->reg_pbase))
#define FPM_ALLOC_2_CHUNKS_PHYS	\
	((unsigned long)FPM_ALLOC_2_CHUNKS_BASE((u32 *)fpm->reg_pbase))
#define FPM_ALLOC_1_CHUNKS_PHYS	\
	((unsigned long)FPM_ALLOC_1_CHUNKS_BASE((u32 *)fpm->reg_pbase))
#define FPM_POOL_MULTI_PHYS	\
	((unsigned long)FPM_POOL_MULTI_BASE((u32 *)fpm->reg_pbase))

#define FPM_POOL_0_ALLOC_8_CHUNKS_BASE(base)	\
	((base) + ((offsetof(FpmControl, pool0) + offsetof(FpmPoolMgmt, pool1_alloc_dealloc)) >> 2))
#define FPM_POOL_0_ALLOC_4_CHUNKS_BASE(base)	\
	((base) + ((offsetof(FpmControl, pool0) + offsetof(FpmPoolMgmt, pool2_alloc_dealloc)) >> 2))
#define FPM_POOL_0_ALLOC_2_CHUNKS_BASE(base)	\
	((base) + ((offsetof(FpmControl, pool0) + offsetof(FpmPoolMgmt, pool3_alloc_dealloc)) >> 2))
#define FPM_POOL_0_ALLOC_1_CHUNKS_BASE(base)	\
	((base) + ((offsetof(FpmControl, pool0) + offsetof(FpmPoolMgmt, pool4_alloc_dealloc)) >> 2))
#define FPM_POOL_0_DEALLOC_BASE(base)	FPM_POOL_1_ALLOC_8_CHUNKS_BASE(base)
#define FPM_POOL_0_POOL_MULTI_BASE(base)	\
	((base) + ((offsetof(FpmControl, pool0) + offsetof(FpmPoolMgmt, pool_multi)) >> 2))
#define FPM_POOL_1_ALLOC_8_CHUNKS_BASE(base)	\
	((base) + ((offsetof(FpmControl, pool1) + offsetof(FpmPoolMgmt, pool1_alloc_dealloc)) >> 2))
#define FPM_POOL_1_ALLOC_4_CHUNKS_BASE(base)	\
	((base) + ((offsetof(FpmControl, pool1) + offsetof(FpmPoolMgmt, pool2_alloc_dealloc)) >> 2))
#define FPM_POOL_1_ALLOC_2_CHUNKS_BASE(base)	\
	((base) + ((offsetof(FpmControl, pool1) + offsetof(FpmPoolMgmt, pool3_alloc_dealloc)) >> 2))
#define FPM_POOL_1_ALLOC_1_CHUNKS_BASE(base)	\
	((base) + ((offsetof(FpmControl, pool1) + offsetof(FpmPoolMgmt, pool4_alloc_dealloc)) >> 2))
#define FPM_POOL_1_DEALLOC_BASE(base)	FPM_POOL_1_ALLOC_8_CHUNKS_BASE(base)
#define FPM_POOL_1_POOL_MULTI_BASE(base)	\
	((base) + ((offsetof(FpmControl, pool1) + offsetof(FpmPoolMgmt, pool_multi)) >> 2))
#define FPM_ALLOC_8_CHUNKS_FROM_POOL_0 \
	FPM_POOL_0_ALLOC_8_CHUNKS_BASE(fpm->reg_vbase)
#define FPM_ALLOC_8_CHUNKS_FROM_POOL_1 \
	FPM_POOL_1_ALLOC_8_CHUNKS_BASE(fpm->reg_vbase)
#define FPM_ALLOC_8_CHUNKS_FROM_POOL(pool) \
	((pool == 0) ? \
		FPM_ALLOC_8_CHUNKS_FROM_POOL_0 \
		FPM_ALLOC_8_CHUNKS_FROM_POOL_1
#define FPM_ALLOC_4_CHUNKS_FROM_POOL_0 \
	FPM_POOL_0_ALLOC_4_CHUNKS_BASE(fpm->reg_vbase)
#define FPM_ALLOC_4_CHUNKS_FROM_POOL_1 \
	FPM_POOL_1_ALLOC_4_CHUNKS_BASE(fpm->reg_vbase)
#define FPM_ALLOC_4_CHUNKS_FROM_POOL(pool) \
	((pool == 0) ? \
		FPM_ALLOC_4_CHUNKS_FROM_POOL_0 \
		FPM_ALLOC_4_CHUNKS_FROM_POOL_1
#define FPM_ALLOC_2_CHUNKS_FROM_POOL_0 \
	FPM_POOL_0_ALLOC_2_CHUNKS_BASE(fpm->reg_vbase)
#define FPM_ALLOC_2_CHUNKS_FROM_POOL_1 \
	FPM_POOL_1_ALLOC_2_CHUNKS_BASE(fpm->reg_vbase)
#define FPM_ALLOC_2_CHUNKS_FROM_POOL(pool) \
	((pool == 0) ? \
		FPM_ALLOC_2_CHUNKS_FROM_POOL_0 \
		FPM_ALLOC_2_CHUNKS_FROM_POOL_1
#define FPM_ALLOC_1_CHUNKS_FROM_POOL_0 \
	FPM_POOL_0_ALLOC_1_CHUNKS_BASE(fpm->reg_vbase)
#define FPM_ALLOC_1_CHUNKS_FROM_POOL_1 \
	FPM_POOL_1_ALLOC_1_CHUNKS_BASE(fpm->reg_vbase)
#define FPM_ALLOC_1_CHUNKS_FROM_POOL(pool) \
	((pool == 0) ? \
		FPM_ALLOC_1_CHUNKS_FROM_POOL_0 \
		FPM_ALLOC_1_CHUNKS_FROM_POOL_1
#define FPM_DEALLOC_TO_POOL(pool, token) \
{ \
	if ((pool) == 0) \
		fpm_reg_write(FPM_POOL_0_DEALLOC_BASE(fpm->reg_vbase, (token))); \
	else \
		fpm_reg_write(FPM_POOL_1_DEALLOC_BASE(fpm->reg_vbase, (token))); \
}

/* Token format */
#define	FPM_TOKEN_POOL(token)	\
	((token & FPM_TOKEN_POOL_MASK) >> FPM_TOKEN_POOL_SHIFT)
#define FPM_MAX_TOKEN_INDEX	\
	((FPM_TOKEN_INDEX_MASK >> FPM_TOKEN_INDEX_SHIFT) + 1)
#define FPM_TOKEN_INDEX(token)  \
	((token & FPM_TOKEN_INDEX_MASK) >> FPM_TOKEN_INDEX_SHIFT)
#define FPM_TOKEN_SIZE(token) \
	((token & FPM_TOKEN_SIZE_MASK) >> FPM_TOKEN_SIZE_SHIFT)

/* Status regs */
#define FPM_POOL1_STAT1		\
	(fpm->reg_vbase + ((offsetof(FpmControl, ctrl) + offsetof(FpmCtrl, pool1_stat1)) >> 2))
#define FPM_POOL2_STAT1		\
	(fpm->reg_vbase + ((offsetof(FpmControl, ctrl) + offsetof(FpmCtrl, pool2_stat1)) >> 2))
#define FPM_GET_UNDERFLOW(pool) \
	(((pool) == 0) ? ((fpm_reg_read(FPM_POOL1_STAT1) & \
			   FPMCTRL_POOL_STAT1_UNDRFL_MASK) >> \
			   FPMCTRL_POOL_STAT1_UNDRFL_SHIFT) : \
			 ((fpm_reg_read(FPM_POOL2_STAT1) & \
			   FPMCTRL_POOL_STAT1_UNDRFL_MASK) >> \
			   FPMCTRL_POOL_STAT1_UNDRFL_SHIFT))
#define FPM_GET_OVERFLOW(pool) \
	(((pool) == 0) ? ((fpm_reg_read(FPM_POOL1_STAT1) & \
			   FPMCTRL_POOL_STAT1_OVRFL_MASK) >> \
			   FPMCTRL_POOL_STAT1_OVRFL_SHIFT) : \
			 ((fpm_reg_read(FPM_POOL2_STAT1) & \
			   FPMCTRL_POOL_STAT1_OVRFL_MASK) >> \
			   FPMCTRL_POOL_STAT1_OVRFL_SHIFT))

#define FPM_POOL1_STAT2		\
	(fpm->reg_vbase + ((offsetof(FpmControl, ctrl) + offsetof(FpmCtrl, pool1_stat2)) >> 2))
#define FPM_POOL2_STAT2		\
	(fpm->reg_vbase + ((offsetof(FpmControl, ctrl) + offsetof(FpmCtrl, pool2_stat2)) >> 2))
#define FPM_GET_TOK_AVAIL(pool) \
	(((pool) == 0) ? ((fpm_reg_read(FPM_POOL1_STAT2) & \
			   FPMCTRL_POOL_STAT2_NUM_OF_TOKENS_AVAL_MASK) >> \
			   FPMCTRL_POOL_STAT2_NUM_OF_TOKENS_AVAL_SHIFT) : \
			 ((fpm_reg_read(FPM_POOL2_STAT2) & \
			   FPMCTRL_POOL_STAT2_NUM_OF_TOKENS_AVAL_MASK) >> \
			   FPMCTRL_POOL_STAT2_NUM_OF_TOKENS_AVAL_SHIFT))
#define FPM_GET_ALLOC_FIFO_EMPTY(pool) \
	(((pool) == 0) ? ((fpm_reg_read(FPM_POOL1_STAT2) & \
			   FPMCTRL_POOL_STAT2_ALLOC_FIFO_EMPTY_MASK) >> \
			   FPMCTRL_POOL_STAT2_ALLOC_FIFO_EMPTY_SHIFT) : \
			 ((fpm_reg_read(FPM_POOL2_STAT2) & \
			   FPMCTRL_POOL_STAT2_ALLOC_FIFO_EMPTY_MASK) >> \
			   FPMCTRL_POOL_STAT2_ALLOC_FIFO_EMPTY_SHIFT))
#define FPM_GET_ALLOC_FIFO_FULL(pool) \
	(((pool) == 0) ? ((fpm_reg_read(FPM_POOL1_STAT2) & \
			   FPMCTRL_POOL_STAT2_ALLOC_FIFO_FULL_MASK) >> \
			   FPMCTRL_POOL_STAT2_ALLOC_FIFO_FULL_SHIFT) : \
			 ((fpm_reg_read(FPM_POOL2_STAT2) & \
			   FPMCTRL_POOL_STAT2_ALLOC_FIFO_FULL_MASK) >> \
			   FPMCTRL_POOL_STAT2_ALLOC_FIFO_FULL_SHIFT))
#define FPM_GET_FREE_FIFO_EMPTY(pool) \
	(((pool) == 0) ? ((fpm_reg_read(FPM_POOL1_STAT2) & \
			   FPMCTRL_POOL_STAT2_FREE_FIFO_EMPTY_MASK) >> \
			   FPMCTRL_POOL_STAT2_FREE_FIFO_EMPTY_SHIFT) : \
			 ((fpm_reg_read(FPM_POOL2_STAT2) & \
			   FPMCTRL_POOL_STAT2_FREE_FIFO_EMPTY_MASK) >> \
			   FPMCTRL_POOL_STAT2_FREE_FIFO_EMPTY_SHIFT))
#define FPM_GET_FREE_FIFO_FULL(pool) \
	(((pool) == 0) ? ((fpm_reg_read(FPM_POOL1_STAT2) & \
			   FPMCTRL_POOL_STAT2_FREE_FIFO_FULL_MASK) >> \
			   FPMCTRL_POOL_STAT2_FREE_FIFO_FULL_SHIFT) : \
			 ((fpm_reg_read(FPM_POOL2_STAT2) & \
			   FPMCTRL_POOL_STAT2_FREE_FIFO_FULL_MASK) >> \
			   FPMCTRL_POOL_STAT2_FREE_FIFO_FULL_SHIFT))
#define FPM_GET_POOL_FULL(pool) \
	(((pool) == 0) ? ((fpm_reg_read(FPM_POOL1_STAT2) & \
			   FPMCTRL_POOL_STAT2_POOL_FULL_MASK) >> \
			   FPMCTRL_POOL_STAT2_POOL_FULL_SHIFT) : \
			 ((fpm_reg_read(FPM_POOL2_STAT2) & \
			   FPMCTRL_POOL_STAT2_POOL_FULL_MASK) >> \
			   FPMCTRL_POOL_STAT2_POOL_FULL_SHIFT))

#define FPM_POOL1_STAT3		\
	(fpm->reg_vbase + ((offsetof(FpmControl, ctrl) + offsetof(FpmCtrl, pool1_stat3)) >> 2))
#define FPM_POOL2_STAT3		\
	(fpm->reg_vbase + ((offsetof(FpmControl, ctrl) + offsetof(FpmCtrl, pool2_stat3)) >> 2))
#define FPM_GET_INVAL_TOK_FREES(pool) \
	(((pool) == 0) ? ((fpm_reg_read(FPM_POOL1_STAT3) & \
			   FPMCTRL_POOL_STAT3_NUM_OF_NOT_VALID_TOKEN_FREES_MASK) >> \
			   FPMCTRL_POOL_STAT3_NUM_OF_NOT_VALID_TOKEN_FREES_SHIFT) : \
			 ((fpm_reg_read(FPM_POOL2_STAT3) & \
			   FPMCTRL_POOL_STAT3_NUM_OF_NOT_VALID_TOKEN_FREES_MASK) >> \
			   FPMCTRL_POOL_STAT3_NUM_OF_NOT_VALID_TOKEN_FREES_SHIFT))

#define FPM_POOL1_STAT4		\
	(fpm->reg_vbase + ((offsetof(FpmControl, ctrl) + offsetof(FpmCtrl, pool1_stat4)) >> 2))
#define FPM_POOL2_STAT4		\
	(fpm->reg_vbase + ((offsetof(FpmControl, ctrl) + offsetof(FpmCtrl, pool2_stat4)) >> 2))
#define FPM_GET_INVAL_TOK_MULTI(pool) \
	(((pool) == 0) ? ((fpm_reg_read(FPM_POOL1_STAT4) & \
			   FPMCTRL_POOL_STAT4_NUM_OF_NOT_VALID_TOKEN_MULTI_MASK) >> \
			   FPMCTRL_POOL_STAT4_NUM_OF_NOT_VALID_TOKEN_MULTI_SHIFT) : \
			 ((fpm_reg_read(FPM_POOL2_STAT4) & \
			   FPMCTRL_POOL_STAT4_NUM_OF_NOT_VALID_TOKEN_MULTI_MASK) >> \
			   FPMCTRL_POOL_STAT4_NUM_OF_NOT_VALID_TOKEN_MULTI_SHIFT))

#define FPM_POOL1_STAT5		\
	(fpm->reg_vbase + ((offsetof(FpmControl, ctrl) + offsetof(FpmCtrl, pool1_stat5)) >> 2))
#define FPM_POOL2_STAT5		\
	(fpm->reg_vbase + ((offsetof(FpmControl, ctrl) + offsetof(FpmCtrl, pool2_stat5)) >> 2))
#define FPM_GET_MEM_CORRUPT_TOK(pool) \
	(((pool) == 0) ? ((fpm_reg_read(FPM_POOL1_STAT5) & \
			   FPMCTRL_POOL_STAT5_MEM_CORRUPT_STS_RELATED_ALLOC_TOKEN_MASK) >> \
			   FPMCTRL_POOL_STAT5_MEM_CORRUPT_STS_RELATED_ALLOC_TOKEN_SHIFT) : \
			 ((fpm_reg_read(FPM_POOL2_STAT5) & \
			   FPMCTRL_POOL_STAT5_MEM_CORRUPT_STS_RELATED_ALLOC_TOKEN_MASK) >> \
			   FPMCTRL_POOL_STAT5_MEM_CORRUPT_STS_RELATED_ALLOC_TOKEN_SHIFT))
#define FPM_GET_MEM_CORRUPT_TOK_VALID(pool) \
	(((pool) == 0) ? ((fpm_reg_read(FPM_POOL1_STAT5) & \
			   FPMCTRL_POOL_STAT5_MEM_CORRUPT_STS_RELATED_ALLOC_TOKEN_VALID_MASK) >> \
			   FPMCTRL_POOL_STAT5_MEM_CORRUPT_STS_RELATED_ALLOC_TOKEN_VALID_SHIFT) : \
			 ((fpm_reg_read(FPM_POOL2_STAT5) & \
			   FPMCTRL_POOL_STAT5_MEM_CORRUPT_STS_RELATED_ALLOC_TOKEN_VALID_MASK) >> \
			   FPMCTRL_POOL_STAT5_MEM_CORRUPT_STS_RELATED_ALLOC_TOKEN_VALID_SHIFT))

#define FPM_POOL1_STAT6		\
	(fpm->reg_vbase + ((offsetof(FpmControl, ctrl) + offsetof(FpmCtrl, pool1_stat6)) >> 2))
#define FPM_POOL2_STAT6		\
	(fpm->reg_vbase + ((offsetof(FpmControl, ctrl) + offsetof(FpmCtrl, pool2_stat6)) >> 2))
#define FPM_GET_INVALID_FREE_TOK(pool) \
	(((pool) == 0) ? ((fpm_reg_read(FPM_POOL1_STAT6) & \
			   FPMCTRL_POOL_STAT6_INVALID_FREE_TOKEN_MASK) >> \
			   FPMCTRL_POOL_STAT6_INVALID_FREE_TOKEN_SHIFT) : \
			 ((fpm_reg_read(FPM_POOL2_STAT6) & \
			   FPMCTRL_POOL_STAT6_INVALID_FREE_TOKEN_MASK) >> \
			   FPMCTRL_POOL_STAT6_INVALID_FREE_TOKEN_SHIFT))
#define FPM_GET_INVALID_FREE_TOK_VALID(pool) \
	(((pool) == 0) ? ((fpm_reg_read(FPM_POOL1_STAT6) & \
			   FPMCTRL_POOL_STAT6_INVALID_FREE_TOKEN_VALID_MASK) >> \
			   FPMCTRL_POOL_STAT6_INVALID_FREE_TOKEN_VALID_SHIFT) : \
			 ((fpm_reg_read(FPM_POOL2_STAT6) & \
			   FPMCTRL_POOL_STAT6_INVALID_FREE_TOKEN_VALID_MASK) >> \
			   FPMCTRL_POOL_STAT6_INVALID_FREE_TOKEN_VALID_SHIFT))
#define FPM_CLEAR_INVALID_FREE_TOK(pool) \
{ \
	if ((pool) == 0) \
		fpm_reg_write(FPM_POOL1_STAT6, 0); \
	else \
		fpm_reg_write(FPM_POOL2_STAT6, 0); \
}

#define FPM_POOL1_STAT7		\
	(fpm->reg_vbase + ((offsetof(FpmControl, ctrl) + offsetof(FpmCtrl, pool1_stat7)) >> 2))
#define FPM_POOL2_STAT7		\
	(fpm->reg_vbase + ((offsetof(FpmControl, ctrl) + offsetof(FpmCtrl, pool2_stat7)) >> 2))
#define FPM_GET_INVALID_MCAST_TOK(pool) \
	(((pool) == 0) ? ((fpm_reg_read(FPM_POOL1_STAT7) & \
			   FPMCTRL_POOL_STAT7_INVALID_MCAST_TOKEN_MASK) >> \
			   FPMCTRL_POOL_STAT7_INVALID_MCAST_TOKEN_SHIFT) : \
			 ((fpm_reg_read(FPM_POOL2_STAT7) & \
			   FPMCTRL_POOL_STAT7_INVALID_MCAST_TOKEN_MASK) >> \
			   FPMCTRL_POOL_STAT7_INVALID_MCAST_TOKEN_SHIFT))
#define FPM_GET_INVALID_MCAST_TOK_VALID(pool) \
	(((pool) == 0) ? ((fpm_reg_read(FPM_POOL1_STAT7) & \
			   FPMCTRL_POOL_STAT7_INVALID_MCAST_TOKEN_VALID_MASK) >> \
			   FPMCTRL_POOL_STAT7_INVALID_MCAST_TOKEN_VALID_SHIFT) : \
			 ((fpm_reg_read(FPM_POOL2_STAT7) & \
			   FPMCTRL_POOL_STAT7_INVALID_MCAST_TOKEN_VALID_MASK) >> \
			   FPMCTRL_POOL_STAT7_INVALID_MCAST_TOKEN_VALID_SHIFT))
#define FPM_CLEAR_INVALID_MCAST_TOK(pool) \
{ \
	if ((pool) == 0) \
		fpm_reg_write(FPM_POOL1_STAT7, 0); \
	else \
		fpm_reg_write(FPM_POOL2_STAT7, 0); \
}

#define FPM_POOL1_STAT8		\
	(fpm->reg_vbase + ((offsetof(FpmControl, ctrl) + offsetof(FpmCtrl, pool1_stat8)) >> 2))
#define FPM_POOL2_STAT8		\
	(fpm->reg_vbase + ((offsetof(FpmControl, ctrl) + offsetof(FpmCtrl, pool2_stat8)) >> 2))
#define FPM_GET_TOK_AVAIL_LOW_WTMK(pool) \
	(((pool) == 0) ? ((fpm_reg_read(FPM_POOL1_STAT8) & \
			   FPMCTRL_POOL_STAT8_TOKENS_AVAIL_LOW_WTMK_MASK) >> \
			   FPMCTRL_POOL_STAT8_TOKENS_AVAIL_LOW_WTMK_SHIFT) : \
			 ((fpm_reg_read(FPM_POOL2_STAT8) & \
			   FPMCTRL_POOL_STAT8_TOKENS_AVAIL_LOW_WTMK_MASK) >> \
			   FPMCTRL_POOL_STAT8_TOKENS_AVAIL_LOW_WTMK_SHIFT))

/* Token recovery regs */
#define FPM_TOK_RECOVER_CTRL		\
	(fpm->reg_vbase + ((offsetof(FpmControl, ctrl) + offsetof(FpmCtrl, token_recover_ctl)) >> 2))
#define FPM_SET_TOK_RECOVER_ENABLE(enable) \
{ \
	if (enable) { \
		fpm_reg_write_mask(FPM_TOK_RECOVER_CTRL, \
		FPMCTRL_TOKEN_RECOVER_CTL_TOKEN_RECOVER_EN_MASK, \
		FPMCTRL_TOKEN_RECOVER_CTL_TOKEN_RECOVER_EN_MASK); \
	} else { \
		fpm_reg_write_mask(FPM_TOK_RECOVER_CTRL, \
		FPMCTRL_TOKEN_RECOVER_CTL_TOKEN_RECOVER_EN_MASK, \
		0); \
	} \
}
#define FPM_SET_TOK_RECOVER_SINGLE_PASS(enable) \
{ \
	if (enable) { \
		fpm_reg_write_mask(FPM_TOK_RECOVER_CTRL, \
		FPMCTRL_TOKEN_RECOVER_CTL_SINGLE_PASS_EN_MASK, \
		FPMCTRL_TOKEN_RECOVER_CTL_SINGLE_PASS_EN_MASK); \
	} else { \
		fpm_reg_write_mask(FPM_TOK_RECOVER_CTRL, \
		FPMCTRL_TOKEN_RECOVER_CTL_SINGLE_PASS_EN_MASK, \
		0); \
	} \
}
#define FPM_SET_TOK_RECOVER_REMARK(enable) \
{ \
	if (enable) { \
		fpm_reg_write_mask(FPM_TOK_RECOVER_CTRL, \
		FPMCTRL_TOKEN_RECOVER_CTL_TOKEN_REMARK_EN_MASK, \
		FPMCTRL_TOKEN_RECOVER_CTL_TOKEN_REMARK_EN_MASK); \
	} else { \
		fpm_reg_write_mask(FPM_TOK_RECOVER_CTRL, \
		FPMCTRL_TOKEN_RECOVER_CTL_TOKEN_REMARK_EN_MASK, \
		0); \
	} \
}
#define FPM_SET_TOK_RECOVER_AUTO_RECLAIM(enable) \
{ \
	if (enable) { \
		fpm_reg_write_mask(FPM_TOK_RECOVER_CTRL, \
		FPMCTRL_TOKEN_RECOVER_CTL_TOKEN_RECLAIM_EN_MASK, \
		FPMCTRL_TOKEN_RECOVER_CTL_TOKEN_RECLAIM_EN_MASK); \
	} else { \
		fpm_reg_write_mask(FPM_TOK_RECOVER_CTRL, \
		FPMCTRL_TOKEN_RECOVER_CTL_TOKEN_RECLAIM_EN_MASK, \
		0); \
	} \
}
#define FPM_SET_TOK_RECOVER_FORCE_RECLAIM(enable) \
{ \
	if (enable) { \
		fpm_reg_write_mask(FPM_TOK_RECOVER_CTRL, \
		FPMCTRL_TOKEN_RECOVER_CTL_FORCE_TOKEN_RECLAIM_MASK, \
		FPMCTRL_TOKEN_RECOVER_CTL_FORCE_TOKEN_RECLAIM_MASK); \
	} else { \
		fpm_reg_write_mask(FPM_TOK_RECOVER_CTRL, \
		FPMCTRL_TOKEN_RECOVER_CTL_FORCE_TOKEN_RECLAIM_MASK, \
		0); \
	} \
}
#define FPM_CLEAR_EXPIRED_TOK_COUNT() \
{ \
	fpm_reg_write_mask(FPM_TOK_RECOVER_CTRL, \
	FPMCTRL_TOKEN_RECOVER_CTL_CLR_EXPIRED_TOKEN_COUNT_MASK, \
	FPMCTRL_TOKEN_RECOVER_CTL_CLR_EXPIRED_TOKEN_COUNT_MASK); \
}
#define FPM_CLEAR_RECOVERED_TOK_COUNT() \
{ \
	fpm_reg_write_mask(FPM_TOK_RECOVER_CTRL, \
	FPMCTRL_TOKEN_RECOVER_CTL_CLR_RECOVERED_TOKEN_COUNT_MASK, \
	FPMCTRL_TOKEN_RECOVER_CTL_CLR_RECOVERED_TOKEN_COUNT_MASK); \
}

#define FPM_SHORT_AGING_TIMER		\
	(fpm->reg_vbase + ((offsetof(FpmControl, ctrl) + offsetof(FpmCtrl, short_aging_timer)) >> 2))
#define FPM_SET_SHORT_AGING_TIMER(value) \
{ \
	fpm_reg_write(FPM_SHORT_AGING_TIMER, (value)); \
}

#define FPM_LONG_AGING_TIMER		\
	(fpm->reg_vbase + ((offsetof(FpmControl, ctrl) + offsetof(FpmCtrl, long_aging_timer)) >> 2))
#define FPM_SET_LONG_AGING_TIMER(value) \
{ \
	fpm_reg_write(FPM_LONG_AGING_TIMER, (value)); \
}

#define FPM_CACHE_RECYCLE_TIMER		\
	(fpm->reg_vbase + ((offsetof(FpmControl, ctrl) + offsetof(FpmCtrl, cache_recycle_timer)) >> 2))
#define FPM_SET_CACHE_RECYCLE_TIMER(value) \
{ \
	fpm_reg_write(FPM_CACHE_RECYCLE_TIMER, (value)); \
}

#define FPM_POOL1_EXPIRED_TOK_COUNT		\
	(fpm->reg_vbase + ((offsetof(FpmControl, ctrl) + offsetof(FpmCtrl, expired_token_count_pool1)) >> 2))
#define FPM_POOL2_EXPIRED_TOK_COUNT		\
	(fpm->reg_vbase + ((offsetof(FpmControl, ctrl) + offsetof(FpmCtrl, expired_token_count_pool2)) >> 2))
#define FPM_GET_EXPIRED_TOK_COUNT(pool) \
	(((pool) == 0) ? fpm_reg_read(FPM_POOL1_EXPIRED_TOK_COUNT) : \
			 fpm_reg_read(FPM_POOL2_EXPIRED_TOK_COUNT))

#define FPM_POOL1_RECOVERED_TOK_COUNT		\
	(fpm->reg_vbase + ((offsetof(FpmControl, ctrl) + offsetof(FpmCtrl, recovered_token_count_pool1)) >> 2))
#define FPM_POOL2_RECOVERED_TOK_COUNT		\
	(fpm->reg_vbase + ((offsetof(FpmControl, ctrl) + offsetof(FpmCtrl, recovered_token_count_pool2)) >> 2))
#define FPM_GET_RECOVERED_TOK_COUNT(pool) \
	(((pool) == 0) ? fpm_reg_read(FPM_POOL1_RECOVERED_TOK_COUNT) : \
			 fpm_reg_read(FPM_POOL2_RECOVERED_TOK_COUNT))

#endif
