/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) Marvell International Ltd. and its affiliates
 */

#ifndef __XOR_REGS_H
#define __XOR_REGS_H

/*
 * For controllers that have two XOR units, then chans 2 & 3 will be mapped
 * to channels 0 & 1 of unit 1
 */
#define XOR_UNIT(chan)			((chan) >> 1)
#define XOR_CHAN(chan)			((chan) & 1)

#ifdef CONFIG_ARMADA_MSYS
#define MV_XOR_REGS_OFFSET(unit)	(0xF0800)
#else
#define MV_XOR_REGS_OFFSET(unit)	(0x60900)
#endif
#define MV_XOR_REGS_BASE(unit)		(MV_XOR_REGS_OFFSET(unit))

/* XOR Engine Control Register Map */
#define XOR_CHANNEL_ARBITER_REG(unit)	(MV_XOR_REGS_BASE(unit))
#define XOR_CONFIG_REG(unit, chan)	(MV_XOR_REGS_BASE(unit) + (0x10 + ((chan) * 4)))
#define XOR_ACTIVATION_REG(unit, chan)	(MV_XOR_REGS_BASE(unit) + (0x20 + ((chan) * 4)))

/* XOR Engine Interrupt Register Map */
#define XOR_CAUSE_REG(unit)		(MV_XOR_REGS_BASE(unit) + 0x30)
#define XOR_MASK_REG(unit)		(MV_XOR_REGS_BASE(unit) + 0x40)
#define XOR_ERROR_CAUSE_REG(unit)	(MV_XOR_REGS_BASE(unit) + 0x50)
#define XOR_ERROR_ADDR_REG(unit)	(MV_XOR_REGS_BASE(unit) + 0x60)

/* XOR Engine Descriptor Register Map */
#define XOR_NEXT_DESC_PTR_REG(unit, chan)	(MV_XOR_REGS_BASE(unit) + (0x200 + ((chan) * 4)))
#define XOR_CURR_DESC_PTR_REG(unit, chan)	(MV_XOR_REGS_BASE(unit) + (0x210 + ((chan) * 4)))
#define XOR_BYTE_COUNT_REG(unit, chan)		(MV_XOR_REGS_BASE(unit) + (0x220 + ((chan) * 4)))

#define XOR_DST_PTR_REG(unit, chan)		(MV_XOR_REGS_BASE(unit) + (0x2B0 + ((chan) * 4)))
#define XOR_BLOCK_SIZE_REG(unit, chan)		(MV_XOR_REGS_BASE(unit) + (0x2C0 + ((chan) * 4)))
#define XOR_TIMER_MODE_CTRL_REG(unit)		(MV_XOR_REGS_BASE(unit) + 0x2D0)
#define XOR_TIMER_MODE_INIT_VAL_REG(unit)	(MV_XOR_REGS_BASE(unit) + 0x2D4)
#define XOR_TIMER_MODE_CURR_VAL_REG(unit)	(MV_XOR_REGS_BASE(unit) + 0x2D8)
#define XOR_INIT_VAL_LOW_REG(unit)		(MV_XOR_REGS_BASE(unit) + 0x2E0)
#define XOR_INIT_VAL_HIGH_REG(unit)		(MV_XOR_REGS_BASE(unit) + 0x2E4)

/* XOR register fileds */

/* XOR Engine [0..1] Configuration Registers (XExCR) */
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

/* XOR Engine [0..1] Activation Registers (XExACTR) */
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

/* XOR Engine [0..1] Destination Pointer Register (XExDPR0) */
#define XEXDPR_DST_PTR_OFFS		(0)
#define XEXDPR_DST_PTR_MASK		(0xFFFFFFFF << XEXDPR_DST_PTR_OFFS)
#define XEXDPR_DST_PTR_XOR_MASK		(0x3F)
#define XEXDPR_DST_PTR_DMA_MASK		(0x1F)
#define XEXDPR_DST_PTR_CRC_MASK		(0x1F)

/* XOR Engine[0..1] Block Size Registers (XExBSR) */
#define XEXBSR_BLOCK_SIZE_OFFS		(0)
#define XEXBSR_BLOCK_SIZE_MASK		(0xFFFFFFFF << XEXBSR_BLOCK_SIZE_OFFS)
#define XEXBSR_BLOCK_SIZE_MIN_VALUE	(128)
#define XEXBSR_BLOCK_SIZE_MAX_VALUE	(0xFFFFFFFF)

/* XOR Engine Address Decoding Register Map */
#define XOR_WINDOW_CTRL_REG(unit, chan)	(MV_XOR_REGS_BASE(unit) + (0x240 + ((chan) * 4)))
#define XOR_BASE_ADDR_REG(unit, win)	(MV_XOR_REGS_BASE(unit) + (0x250 + ((win) * 4)))
#define XOR_SIZE_MASK_REG(unit, win)	(MV_XOR_REGS_BASE(unit) + (0x270 + ((win) * 4)))
#define XOR_HIGH_ADDR_REMAP_REG(unit, win) (MV_XOR_REGS_BASE(unit) + (0x290 + ((win) * 4)))
#define XOR_ADDR_OVRD_REG(unit, win)	(MV_XOR_REGS_BASE(unit) + (0x2A0 + ((win) * 4)))

#endif /* __XOR_REGS_H */
