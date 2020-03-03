/*
    drivers/ni_tio_internal.h
    Header file for NI general purpose counter support code (ni_tio.c and
    ni_tiocmd.c)

    COMEDI - Linux Control and Measurement Device Interface

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
*/

#ifndef _COMEDI_NI_TIO_INTERNAL_H
#define _COMEDI_NI_TIO_INTERNAL_H

#include "ni_tio.h"

#define NITIO_AUTO_INC_REG(x)		(NITIO_G0_AUTO_INC + (x))
#define GI_AUTO_INC_MASK		0xff
#define NITIO_CMD_REG(x)		(NITIO_G0_CMD + (x))
#define GI_ARM				(1 << 0)
#define GI_SAVE_TRACE			(1 << 1)
#define GI_LOAD				(1 << 2)
#define GI_DISARM			(1 << 4)
#define GI_CNT_DIR(x)			(((x) & 0x3) << 5)
#define GI_CNT_DIR_MASK			(3 << 5)
#define GI_WRITE_SWITCH			(1 << 7)
#define GI_SYNC_GATE			(1 << 8)
#define GI_LITTLE_BIG_ENDIAN		(1 << 9)
#define GI_BANK_SWITCH_START		(1 << 10)
#define GI_BANK_SWITCH_MODE		(1 << 11)
#define GI_BANK_SWITCH_ENABLE		(1 << 12)
#define GI_ARM_COPY			(1 << 13)
#define GI_SAVE_TRACE_COPY		(1 << 14)
#define GI_DISARM_COPY			(1 << 15)
#define NITIO_HW_SAVE_REG(x)		(NITIO_G0_HW_SAVE + (x))
#define NITIO_SW_SAVE_REG(x)		(NITIO_G0_SW_SAVE + (x))
#define NITIO_MODE_REG(x)		(NITIO_G0_MODE + (x))
#define GI_GATING_DISABLED		(0 << 0)
#define GI_LEVEL_GATING			(1 << 0)
#define GI_RISING_EDGE_GATING		(2 << 0)
#define GI_FALLING_EDGE_GATING		(3 << 0)
#define GI_GATING_MODE_MASK		(3 << 0)
#define GI_GATE_ON_BOTH_EDGES		(1 << 2)
#define GI_EDGE_GATE_STARTS_STOPS	(0 << 3)
#define GI_EDGE_GATE_STOPS_STARTS	(1 << 3)
#define GI_EDGE_GATE_STARTS		(2 << 3)
#define GI_EDGE_GATE_NO_STARTS_OR_STOPS	(3 << 3)
#define GI_EDGE_GATE_MODE_MASK		(3 << 3)
#define GI_STOP_ON_GATE			(0 << 5)
#define GI_STOP_ON_GATE_OR_TC		(1 << 5)
#define GI_STOP_ON_GATE_OR_SECOND_TC	(2 << 5)
#define GI_STOP_MODE_MASK		(3 << 5)
#define GI_LOAD_SRC_SEL			(1 << 7)
#define GI_OUTPUT_TC_PULSE		(1 << 8)
#define GI_OUTPUT_TC_TOGGLE		(2 << 8)
#define GI_OUTPUT_TC_OR_GATE_TOGGLE	(3 << 8)
#define GI_OUTPUT_MODE_MASK		(3 << 8)
#define GI_NO_HARDWARE_DISARM		(0 << 10)
#define GI_DISARM_AT_TC			(1 << 10)
#define GI_DISARM_AT_GATE		(2 << 10)
#define GI_DISARM_AT_TC_OR_GATE		(3 << 10)
#define GI_COUNTING_ONCE_MASK		(3 << 10)
#define GI_LOADING_ON_TC		(1 << 12)
#define GI_GATE_POL_INVERT		(1 << 13)
#define GI_LOADING_ON_GATE		(1 << 14)
#define GI_RELOAD_SRC_SWITCHING		(1 << 15)
#define NITIO_LOADA_REG(x)		(NITIO_G0_LOADA + (x))
#define NITIO_LOADB_REG(x)		(NITIO_G0_LOADB + (x))
#define NITIO_INPUT_SEL_REG(x)		(NITIO_G0_INPUT_SEL + (x))
#define GI_READ_ACKS_IRQ		(1 << 0)
#define GI_WRITE_ACKS_IRQ		(1 << 1)
#define GI_BITS_TO_SRC(x)		(((x) >> 2) & 0x1f)
#define GI_SRC_SEL(x)			(((x) & 0x1f) << 2)
#define GI_SRC_SEL_MASK			(0x1f << 2)
#define GI_BITS_TO_GATE(x)		(((x) >> 7) & 0x1f)
#define GI_GATE_SEL(x)			(((x) & 0x1f) << 7)
#define GI_GATE_SEL_MASK		(0x1f << 7)
#define GI_GATE_SEL_LOAD_SRC		(1 << 12)
#define GI_OR_GATE			(1 << 13)
#define GI_OUTPUT_POL_INVERT		(1 << 14)
#define GI_SRC_POL_INVERT		(1 << 15)
#define NITIO_CNT_MODE_REG(x)		(NITIO_G0_CNT_MODE + (x))
#define GI_CNT_MODE(x)			(((x) & 0x7) << 0)
#define GI_CNT_MODE_NORMAL		GI_CNT_MODE(0)
#define GI_CNT_MODE_QUADX1		GI_CNT_MODE(1)
#define GI_CNT_MODE_QUADX2		GI_CNT_MODE(2)
#define GI_CNT_MODE_QUADX4		GI_CNT_MODE(3)
#define GI_CNT_MODE_TWO_PULSE		GI_CNT_MODE(4)
#define GI_CNT_MODE_SYNC_SRC		GI_CNT_MODE(6)
#define GI_CNT_MODE_MASK		(7 << 0)
#define GI_INDEX_MODE			(1 << 4)
#define GI_INDEX_PHASE(x)		(((x) & 0x3) << 5)
#define GI_INDEX_PHASE_MASK		(3 << 5)
#define GI_HW_ARM_ENA			(1 << 7)
#define GI_HW_ARM_SEL(x)		((x) << 8)
#define GI_660X_HW_ARM_SEL_MASK		(0x7 << 8)
#define GI_M_HW_ARM_SEL_MASK		(0x1f << 8)
#define GI_660X_PRESCALE_X8		(1 << 12)
#define GI_M_PRESCALE_X8		(1 << 13)
#define GI_660X_ALT_SYNC		(1 << 13)
#define GI_M_ALT_SYNC			(1 << 14)
#define GI_660X_PRESCALE_X2		(1 << 14)
#define GI_M_PRESCALE_X2		(1 << 15)
#define NITIO_GATE2_REG(x)		(NITIO_G0_GATE2 + (x))
#define GI_GATE2_MODE			(1 << 0)
#define GI_BITS_TO_GATE2(x)		(((x) >> 7) & 0x1f)
#define GI_GATE2_SEL(x)			(((x) & 0x1f) << 7)
#define GI_GATE2_SEL_MASK		(0x1f << 7)
#define GI_GATE2_POL_INVERT		(1 << 13)
#define GI_GATE2_SUBSEL			(1 << 14)
#define GI_SRC_SUBSEL			(1 << 15)
#define NITIO_SHARED_STATUS_REG(x)	(NITIO_G01_STATUS + ((x) / 2))
#define GI_SAVE(x)			(((x) % 2) ? (1 << 1) : (1 << 0))
#define GI_COUNTING(x)			(((x) % 2) ? (1 << 3) : (1 << 2))
#define GI_NEXT_LOAD_SRC(x)		(((x) % 2) ? (1 << 5) : (1 << 4))
#define GI_STALE_DATA(x)		(((x) % 2) ? (1 << 7) : (1 << 6))
#define GI_ARMED(x)			(((x) % 2) ? (1 << 9) : (1 << 8))
#define GI_NO_LOAD_BETWEEN_GATES(x)	(((x) % 2) ? (1 << 11) : (1 << 10))
#define GI_TC_ERROR(x)			(((x) % 2) ? (1 << 13) : (1 << 12))
#define GI_GATE_ERROR(x)		(((x) % 2) ? (1 << 15) : (1 << 14))
#define NITIO_RESET_REG(x)		(NITIO_G01_RESET + ((x) / 2))
#define GI_RESET(x)			(1 << (2 + ((x) % 2)))
#define NITIO_STATUS1_REG(x)		(NITIO_G01_STATUS1 + ((x) / 2))
#define NITIO_STATUS2_REG(x)		(NITIO_G01_STATUS2 + ((x) / 2))
#define GI_OUTPUT(x)			(((x) % 2) ? (1 << 1) : (1 << 0))
#define GI_HW_SAVE(x)			(((x) % 2) ? (1 << 13) : (1 << 12))
#define GI_PERMANENT_STALE(x)		(((x) % 2) ? (1 << 15) : (1 << 14))
#define NITIO_DMA_CFG_REG(x)		(NITIO_G0_DMA_CFG + (x))
#define GI_DMA_ENABLE			(1 << 0)
#define GI_DMA_WRITE			(1 << 1)
#define GI_DMA_INT_ENA			(1 << 2)
#define GI_DMA_RESET			(1 << 3)
#define GI_DMA_BANKSW_ERROR		(1 << 4)
#define NITIO_DMA_STATUS_REG(x)		(NITIO_G0_DMA_STATUS + (x))
#define GI_DMA_READBANK			(1 << 13)
#define GI_DRQ_ERROR			(1 << 14)
#define GI_DRQ_STATUS			(1 << 15)
#define NITIO_ABZ_REG(x)		(NITIO_G0_ABZ + (x))
#define NITIO_INT_ACK_REG(x)		(NITIO_G0_INT_ACK + (x))
#define GI_GATE_ERROR_CONFIRM(x)	(((x) % 2) ? (1 << 1) : (1 << 5))
#define GI_TC_ERROR_CONFIRM(x)		(((x) % 2) ? (1 << 2) : (1 << 6))
#define GI_TC_INTERRUPT_ACK		(1 << 14)
#define GI_GATE_INTERRUPT_ACK		(1 << 15)
#define NITIO_STATUS_REG(x)		(NITIO_G0_STATUS + (x))
#define GI_GATE_INTERRUPT		(1 << 2)
#define GI_TC				(1 << 3)
#define GI_INTERRUPT			(1 << 15)
#define NITIO_INT_ENA_REG(x)		(NITIO_G0_INT_ENA + (x))
#define GI_TC_INTERRUPT_ENABLE(x)	(((x) % 2) ? (1 << 9) : (1 << 6))
#define GI_GATE_INTERRUPT_ENABLE(x)	(((x) % 2) ? (1 << 10) : (1 << 8))

static inline void write_register(struct ni_gpct *counter, unsigned bits,
				  enum ni_gpct_register reg)
{
	BUG_ON(reg >= NITIO_NUM_REGS);
	counter->counter_dev->write_register(counter, bits, reg);
}

static inline unsigned read_register(struct ni_gpct *counter,
				     enum ni_gpct_register reg)
{
	BUG_ON(reg >= NITIO_NUM_REGS);
	return counter->counter_dev->read_register(counter, reg);
}

static inline int ni_tio_counting_mode_registers_present(const struct
							 ni_gpct_device
							 *counter_dev)
{
	switch (counter_dev->variant) {
	case ni_gpct_variant_e_series:
		return 0;
	case ni_gpct_variant_m_series:
	case ni_gpct_variant_660x:
		return 1;
	default:
		BUG();
		break;
	}
	return 0;
}

static inline void ni_tio_set_bits_transient(struct ni_gpct *counter,
					     enum ni_gpct_register
					     register_index, unsigned bit_mask,
					     unsigned bit_values,
					     unsigned transient_bit_values)
{
	struct ni_gpct_device *counter_dev = counter->counter_dev;
	unsigned long flags;

	BUG_ON(register_index >= NITIO_NUM_REGS);
	spin_lock_irqsave(&counter_dev->regs_lock, flags);
	counter_dev->regs[register_index] &= ~bit_mask;
	counter_dev->regs[register_index] |= (bit_values & bit_mask);
	write_register(counter,
		       counter_dev->regs[register_index] | transient_bit_values,
		       register_index);
	mmiowb();
	spin_unlock_irqrestore(&counter_dev->regs_lock, flags);
}

/* ni_tio_set_bits( ) is for safely writing to registers whose bits may be
 * twiddled in interrupt context, or whose software copy may be read in
 * interrupt context.
 */
static inline void ni_tio_set_bits(struct ni_gpct *counter,
				   enum ni_gpct_register register_index,
				   unsigned bit_mask, unsigned bit_values)
{
	ni_tio_set_bits_transient(counter, register_index, bit_mask, bit_values,
				  0x0);
}

/* ni_tio_get_soft_copy( ) is for safely reading the software copy of a register
whose bits might be modified in interrupt context, or whose software copy
might need to be read in interrupt context.
*/
static inline unsigned ni_tio_get_soft_copy(const struct ni_gpct *counter,
					    enum ni_gpct_register
					    register_index)
{
	struct ni_gpct_device *counter_dev = counter->counter_dev;
	unsigned long flags;
	unsigned value;

	BUG_ON(register_index >= NITIO_NUM_REGS);
	spin_lock_irqsave(&counter_dev->regs_lock, flags);
	value = counter_dev->regs[register_index];
	spin_unlock_irqrestore(&counter_dev->regs_lock, flags);
	return value;
}

int ni_tio_arm(struct ni_gpct *counter, int arm, unsigned start_trigger);
int ni_tio_set_gate_src(struct ni_gpct *counter, unsigned gate_index,
			unsigned int gate_source);

#endif /* _COMEDI_NI_TIO_INTERNAL_H */
