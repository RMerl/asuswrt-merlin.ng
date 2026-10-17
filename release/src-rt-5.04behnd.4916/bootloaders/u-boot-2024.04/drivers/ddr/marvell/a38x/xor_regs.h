/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) Marvell International Ltd. and its affiliates
 */

#ifndef _XOR_REGS_h
#define _XOR_REGS_h

/*
 * For controllers that have two XOR units, then chans 2 & 3 will be
 * mapped to channels 0 & 1 of unit 1
 */
#define XOR_UNIT(chan)	((chan) >> 1)
#define XOR_CHAN(chan)  ((chan) & 1)

#define MV_XOR_REGS_OFFSET(unit)	(0x60900)
#define MV_XOR_REGS_BASE(unit)		(MV_XOR_REGS_OFFSET(unit))

/* XOR Engine Control Register Map */
#define XOR_CHANNEL_ARBITER_REG(unit)	(MV_XOR_REGS_BASE(unit))
#define XOR_CONFIG_REG(unit, chan)	(MV_XOR_REGS_BASE(unit) + \
					 (0x10 + ((chan) * 4)))
#define XOR_ACTIVATION_REG(unit, chan)	(MV_XOR_REGS_BASE(unit) + \
					 (0x20 + ((chan) * 4)))

/* XOR Engine Interrupt Register Map */
#define XOR_CAUSE_REG(unit)		(MV_XOR_REGS_BASE(unit)+(0x30))
#define XOR_MASK_REG(unit)		(MV_XOR_REGS_BASE(unit)+(0x40))
#define XOR_ERROR_CAUSE_REG(unit)	(MV_XOR_REGS_BASE(unit)+(0x50))
#define XOR_ERROR_ADDR_REG(unit)	(MV_XOR_REGS_BASE(unit)+(0x60))

/* XOR Engine Descriptor Register Map */
#define XOR_NEXT_DESC_PTR_REG(unit, chan) (MV_XOR_REGS_BASE(unit) + \
					   (0x200 + ((chan) * 4)))
#define XOR_CURR_DESC_PTR_REG(unit, chan) (MV_XOR_REGS_BASE(unit) + \
					   (0x210 + ((chan) * 4)))
#define XOR_BYTE_COUNT_REG(unit, chan)	(MV_XOR_REGS_BASE(unit) + \
					 (0x220 + ((chan) * 4)))

/* XOR Engine ECC/Mem_init Register Map */
#define XOR_DST_PTR_REG(unit, chan)	(MV_XOR_REGS_BASE(unit) + \
					 (0x2b0 + ((chan) * 4)))
#define XOR_BLOCK_SIZE_REG(unit, chan)	(MV_XOR_REGS_BASE(unit) + \
					 (0x2c0 + ((chan) * 4)))
#define XOR_TIMER_MODE_CTRL_REG(unit)	(MV_XOR_REGS_BASE(unit) + (0x2d0))
#define XOR_TIMER_MODE_INIT_VAL_REG(unit) (MV_XOR_REGS_BASE(unit) + (0x2d4))
#define XOR_TIMER_MODE_CURR_VAL_REG(unit) (MV_XOR_REGS_BASE(unit) + (0x2d8))
#define XOR_INIT_VAL_LOW_REG(unit)	(MV_XOR_REGS_BASE(unit) + (0x2e0))
#define XOR_INIT_VAL_HIGH_REG(unit)	(MV_XOR_REGS_BASE(unit) + (0x2e4))

/* XOR Engine Debug Register Map */
#define XOR_DEBUG_REG(unit)		(MV_XOR_REGS_BASE(unit) + (0x70))

/* XOR register fileds */

/* XOR Engine Channel Arbiter Register */
#define XECAR_SLICE_OFFS(slice_num)	(slice_num)
#define XECAR_SLICE_MASK(slice_num)	(1 << (XECAR_SLICE_OFFS(slice_num)))

/* XOR Engine [0..1] Configuration Registers */
#define XEXCR_OPERATION_MODE_OFFS	(0)
#define XEXCR_OPERATION_MODE_MASK	(7 << XEXCR_OPERATION_MODE_OFFS)
#define XEXCR_OPERATION_MODE_XOR	(0 << XEXCR_OPERATION_MODE_OFFS)
#define XEXCR_OPERATION_MODE_CRC	(1 << XEXCR_OPERATION_MODE_OFFS)
#define XEXCR_OPERATION_MODE_DMA	(2 << XEXCR_OPERATION_MODE_OFFS)
#define XEXCR_OPERATION_MODE_ECC	(3 << XEXCR_OPERATION_MODE_OFFS)
#define XEXCR_OPERATION_MODE_MEM_INIT	(4 << XEXCR_OPERATION_MODE_OFFS)

#define XEXCR_SRC_BURST_LIMIT_OFFS	(4)
#define XEXCR_SRC_BURST_LIMIT_MASK	(7 << XEXCR_SRC_BURST_LIMIT_OFFS)
#define XEXCR_DST_BURST_LIMIT_OFFS	(8)
#define XEXCR_DST_BURST_LIMIT_MASK	(7 << XEXCR_DST_BURST_LIMIT_OFFS)
#define XEXCR_DRD_RES_SWP_OFFS		(12)
#define XEXCR_DRD_RES_SWP_MASK		(1 << XEXCR_DRD_RES_SWP_OFFS)
#define XEXCR_DWR_REQ_SWP_OFFS		(13)
#define XEXCR_DWR_REQ_SWP_MASK		(1 << XEXCR_DWR_REQ_SWP_OFFS)
#define XEXCR_DES_SWP_OFFS		(14)
#define XEXCR_DES_SWP_MASK		(1 << XEXCR_DES_SWP_OFFS)
#define XEXCR_REG_ACC_PROTECT_OFFS	(15)
#define XEXCR_REG_ACC_PROTECT_MASK	(1 << XEXCR_REG_ACC_PROTECT_OFFS)

/* XOR Engine [0..1] Activation Registers */
#define XEXACTR_XESTART_OFFS		(0)
#define XEXACTR_XESTART_MASK		(1 << XEXACTR_XESTART_OFFS)
#define XEXACTR_XESTOP_OFFS		(1)
#define XEXACTR_XESTOP_MASK		(1 << XEXACTR_XESTOP_OFFS)
#define XEXACTR_XEPAUSE_OFFS		(2)
#define XEXACTR_XEPAUSE_MASK		(1 << XEXACTR_XEPAUSE_OFFS)
#define XEXACTR_XERESTART_OFFS		(3)
#define XEXACTR_XERESTART_MASK		(1 << XEXACTR_XERESTART_OFFS)
#define XEXACTR_XESTATUS_OFFS		(4)
#define XEXACTR_XESTATUS_MASK		(3 << XEXACTR_XESTATUS_OFFS)
#define XEXACTR_XESTATUS_IDLE		(0 << XEXACTR_XESTATUS_OFFS)
#define XEXACTR_XESTATUS_ACTIVE		(1 << XEXACTR_XESTATUS_OFFS)
#define XEXACTR_XESTATUS_PAUSED		(2 << XEXACTR_XESTATUS_OFFS)

/* XOR Engine Interrupt Cause Register (XEICR) */
#define XEICR_CHAN_OFFS			16
#define XEICR_CAUSE_OFFS(chan)		(chan * XEICR_CHAN_OFFS)
#define XEICR_CAUSE_MASK(chan, cause)	(1 << (cause + XEICR_CAUSE_OFFS(chan)))
#define XEICR_COMP_MASK_ALL		0x000f000f
#define XEICR_COMP_MASK(chan)		(0x000f << XEICR_CAUSE_OFFS(chan))
#define XEICR_ERR_MASK			0x03800380

/* XOR Engine Error Cause Register (XEECR) */
#define XEECR_ERR_TYPE_OFFS		0
#define XEECR_ERR_TYPE_MASK		(0x1f << XEECR_ERR_TYPE_OFFS)

/* XOR Engine Error Address Register (XEEAR) */
#define XEEAR_ERR_ADDR_OFFS		(0)
#define XEEAR_ERR_ADDR_MASK		(0xffffffff << XEEAR_ERR_ADDR_OFFS)

/* XOR Engine [0..1] Next Descriptor Pointer Register */
#define XEXNDPR_NEXT_DESC_PTR_OFFS	(0)
#define XEXNDPR_NEXT_DESC_PTR_MASK	(0xffffffff << \
					 XEXNDPR_NEXT_DESC_PTR_OFFS)

/* XOR Engine [0..1] Current Descriptor Pointer Register */
#define XEXCDPR_CURRENT_DESC_PTR_OFFS	(0)
#define XEXCDPR_CURRENT_DESC_PTR_MASK	(0xffffffff << \
					 XEXCDPR_CURRENT_DESC_PTR_OFFS)

/* XOR Engine [0..1] Byte Count Register */
#define XEXBCR_BYTE_CNT_OFFS		(0)
#define XEXBCR_BYTE_CNT_MASK		(0xffffffff << XEXBCR_BYTE_CNT_OFFS)

/* XOR Engine [0..1] Destination Pointer Register */
#define XEXDPR_DST_PTR_OFFS		(0)
#define XEXDPR_DST_PTR_MASK		(0xffffffff << XEXDPR_DST_PTR_OFFS)
#define XEXDPR_DST_PTR_XOR_MASK		(0x3f)
#define XEXDPR_DST_PTR_DMA_MASK		(0x1f)
#define XEXDPR_DST_PTR_CRC_MASK		(0x1f)

/* XOR Engine[0..1] Block Size Registers */
#define XEXBSR_BLOCK_SIZE_OFFS		(0)
#define XEXBSR_BLOCK_SIZE_MASK		(0xffffffff << XEXBSR_BLOCK_SIZE_OFFS)
#define XEXBSR_BLOCK_SIZE_MIN_VALUE	(128)
#define XEXBSR_BLOCK_SIZE_MAX_VALUE	(0xffffffff)

/* XOR Engine Timer Mode Control Register (XETMCR) */
#define XETMCR_TIMER_EN_OFFS		(0)
#define XETMCR_TIMER_EN_MASK		(1 << XETMCR_TIMER_EN_OFFS)
#define XETMCR_TIMER_EN_ENABLE		(1 << XETMCR_TIMER_EN_OFFS)
#define XETMCR_TIMER_EN_DISABLE		(0 << XETMCR_TIMER_EN_OFFS)
#define XETMCR_SECTION_SIZE_CTRL_OFFS	(8)
#define XETMCR_SECTION_SIZE_CTRL_MASK	(0x1f << XETMCR_SECTION_SIZE_CTRL_OFFS)
#define XETMCR_SECTION_SIZE_MIN_VALUE	(7)
#define XETMCR_SECTION_SIZE_MAX_VALUE	(31)

/* XOR Engine Timer Mode Initial Value Register (XETMIVR) */
#define XETMIVR_TIMER_INIT_VAL_OFFS	(0)
#define XETMIVR_TIMER_INIT_VAL_MASK	(0xffffffff << \
					 XETMIVR_TIMER_INIT_VAL_OFFS)

/* XOR Engine Timer Mode Current Value Register (XETMCVR) */
#define XETMCVR_TIMER_CRNT_VAL_OFFS	(0)
#define XETMCVR_TIMER_CRNT_VAL_MASK	(0xffffffff << \
					 XETMCVR_TIMER_CRNT_VAL_OFFS)

/* XOR Engine Initial Value Register Low (XEIVRL) */
#define XEIVRL_INIT_VAL_L_OFFS		(0)
#define XEIVRL_INIT_VAL_L_MASK		(0xffffffff << XEIVRL_INIT_VAL_L_OFFS)

/* XOR Engine Initial Value Register High (XEIVRH) */
#define XEIVRH_INIT_VAL_H_OFFS		(0)
#define XEIVRH_INIT_VAL_H_MASK		(0xffffffff << XEIVRH_INIT_VAL_H_OFFS)

/* XOR Engine Debug Register (XEDBR) */
#define XEDBR_PARITY_ERR_INSR_OFFS	(0)
#define XEDBR_PARITY_ERR_INSR_MASK	(1 << XEDBR_PARITY_ERR_INSR_OFFS)
#define XEDBR_XBAR_ERR_INSR_OFFS	(1)
#define XEDBR_XBAR_ERR_INSR_MASK	(1 << XEDBR_XBAR_ERR_INSR_OFFS)

/* XOR Engine address decode registers.	*/
/* Maximum address decode windows */
#define XOR_MAX_ADDR_DEC_WIN		8
/* Maximum address arbiter windows */
#define XOR_MAX_REMAP_WIN		4

/* XOR Engine Address Decoding Register Map */
#define XOR_WINDOW_CTRL_REG(unit, chan)	(MV_XOR_REGS_BASE(unit) + \
					 (0x240 + ((chan) * 4)))
#define XOR_BASE_ADDR_REG(unit, win_num) (MV_XOR_REGS_BASE(unit) + \
					  (0x250 + ((win_num) * 4)))
#define XOR_SIZE_MASK_REG(unit, win_num) (MV_XOR_REGS_BASE(unit) + \
					  (0x270 + ((win_num) * 4)))
#define XOR_HIGH_ADDR_REMAP_REG(unit, win_num) (MV_XOR_REGS_BASE(unit) + \
						(0x290 + ((win_num) * 4)))
#define XOR_ADDR_OVRD_REG(unit, win_num) (MV_XOR_REGS_BASE(unit) + \
					  (0x2a0 + ((win_num) * 4)))

/* XOR Engine [0..1] Window Control Registers */
#define XEXWCR_WIN_EN_OFFS(win_num)	(win_num)
#define XEXWCR_WIN_EN_MASK(win_num)	(1 << (XEXWCR_WIN_EN_OFFS(win_num)))
#define XEXWCR_WIN_EN_ENABLE(win_num)	(1 << (XEXWCR_WIN_EN_OFFS(win_num)))
#define XEXWCR_WIN_EN_DISABLE(win_num)	(0 << (XEXWCR_WIN_EN_OFFS(win_num)))

#define XEXWCR_WIN_ACC_OFFS(win_num)	((2 * win_num) + 16)
#define XEXWCR_WIN_ACC_MASK(win_num)	(3 << (XEXWCR_WIN_ACC_OFFS(win_num)))
#define XEXWCR_WIN_ACC_NO_ACC(win_num)	(0 << (XEXWCR_WIN_ACC_OFFS(win_num)))
#define XEXWCR_WIN_ACC_RO(win_num)	(1 << (XEXWCR_WIN_ACC_OFFS(win_num)))
#define XEXWCR_WIN_ACC_RW(win_num)	(3 << (XEXWCR_WIN_ACC_OFFS(win_num)))

/* XOR Engine Base Address Registers (XEBARx) */
#define XEBARX_TARGET_OFFS		(0)
#define XEBARX_TARGET_MASK		(0xf << XEBARX_TARGET_OFFS)
#define XEBARX_ATTR_OFFS		(8)
#define XEBARX_ATTR_MASK		(0xff << XEBARX_ATTR_OFFS)
#define XEBARX_BASE_OFFS		(16)
#define XEBARX_BASE_MASK		(0xffff << XEBARX_BASE_OFFS)

/* XOR Engine Size Mask Registers (XESMRx) */
#define XESMRX_SIZE_MASK_OFFS		(16)
#define XESMRX_SIZE_MASK_MASK		(0xffff << XESMRX_SIZE_MASK_OFFS)
#define XOR_WIN_SIZE_ALIGN		_64K

/* XOR Engine High Address Remap Register (XEHARRx1) */
#define XEHARRX_REMAP_OFFS		(0)
#define XEHARRX_REMAP_MASK		(0xffffffff << XEHARRX_REMAP_OFFS)

#define XOR_OVERRIDE_CTRL_REG(chan)	(MV_XOR_REGS_BASE(XOR_UNIT(chan)) + \
					 (0x2a0 + ((XOR_CHAN(chan)) * 4)))

/* XOR Engine [0..1] Address Override Control Register */
#define XEXAOCR_OVR_EN_OFFS(target)	(3 * target)
#define XEXAOCR_OVR_EN_MASK(target)	(1 << (XEXAOCR_OVR_EN_OFFS(target)))
#define XEXAOCR_OVR_PTR_OFFS(target)	((3 * target) + 1)
#define XEXAOCR_OVR_PTR_MASK(target)	(3 << (XEXAOCR_OVR_PTR_OFFS(target)))
#define XEXAOCR_OVR_BAR(win_num, target) (win_num << \
					  (XEXAOCR_OVR_PTR_OFFS(target)))

/* Maximum address override windows */
#define XOR_MAX_OVERRIDE_WIN		4

#endif /* _XOR_REGS_h */
