/*
 * Copyright (C) 2013 - ARM Ltd
 * Author: Marc Zyngier <marc.zyngier@arm.com>
 *
 * Based on code by Carl van Schaik <carl@ok-labs.com>.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _GENERICTIMER_H_
#define _GENERICTIMER_H_

#ifdef __ASSEMBLY__

/*
 * This macro provide a physical timer that can be used for delay in the code.
 * The macro is moved from sunxi/psci_sun7i.S
 *
 * reg: is used in this macro.
 * ticks: The freq is based on generic timer.
 */
.macro	timer_wait	reg, ticks
	movw	\reg, #(\ticks & 0xffff)
	movt	\reg, #(\ticks >> 16)
	mcr	p15, 0, \reg, c14, c2, 0
	isb
	mov	\reg, #3
	mcr	p15, 0, \reg, c14, c2, 1
1 :	isb
	mrc	p15, 0, \reg, c14, c2, 1
	ands	\reg, \reg, #4
	bne	1b
	mov	\reg, #0
	mcr	p15, 0, \reg, c14, c2, 1
	isb
.endm

#endif /* __ASSEMBLY__ */

#endif /* _GENERICTIMER_H_ */
