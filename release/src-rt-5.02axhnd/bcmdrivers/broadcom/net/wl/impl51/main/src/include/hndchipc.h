/*
 * HND SiliconBackplane chipcommon support - OS independent.
 *
 * Copyright (C) 2020, Broadcom. All Rights Reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: hndchipc.h 689775 2017-03-13 12:37:05Z $
 */

#ifndef _hndchipc_h_
#define _hndchipc_h_

#include <typedefs.h>
#include <siutils.h>

#ifdef RTE_UART
typedef void (*si_serial_init_fn)(si_t *sih, void *regs, uint irq, uint baud_base, uint reg_shift);
#else
typedef void (*si_serial_init_fn)(void *regs, uint irq, uint baud_base, uint reg_shift);
#endif // endif
extern void si_serial_init(si_t *sih, si_serial_init_fn add);

extern volatile void *hnd_jtagm_init(si_t *sih, uint clkd, bool exttap);
extern void hnd_jtagm_disable(si_t *sih, volatile void *h);
extern uint32 jtag_scan(si_t *sih, volatile void *h, uint irsz, uint32 ir0, uint32 ir1,
                        uint drsz, uint32 dr0, uint32 *dr1, bool rti);
extern uint32 jtag_read_128(si_t *sih, volatile void *h, uint irsz, uint32 ir0, uint drsz,
	uint32 dr0, uint32 *dr1, uint32 *dr2, uint32 *dr3);
extern uint32 jtag_write_128(si_t *sih, volatile void *h, uint irsz, uint32 ir0, uint drsz,
	uint32 dr0, uint32 *dr1, uint32 *dr2, uint32 *dr3);
extern int jtag_setbit_128(si_t *sih, uint32 jtagureg_addr, uint8 bit_pos, uint8 bit_val);

#endif /* _hndchipc_h_ */
