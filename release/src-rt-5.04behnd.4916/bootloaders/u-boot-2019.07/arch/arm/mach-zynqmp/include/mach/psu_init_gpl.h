/* SPDX-License-Identifier: GPL-2.0+ */

#ifndef _PSU_INIT_GPL_H_ /* prevent circular inclusions */
#define _PSU_INIT_GPL_H_

#include <asm/io.h>
#include <common.h>

int mask_pollonvalue(unsigned long add, u32 mask, u32 value);

int mask_poll(u32 add, u32 mask);

u32 mask_read(u32 add, u32 mask);

void mask_delay(u32 delay);

void psu_mask_write(unsigned long offset, unsigned long mask,
		    unsigned long val);

void prog_reg(unsigned long addr, unsigned long mask,
	      unsigned long shift, unsigned long value);

int psu_init(void);

#endif /* _PSU_INIT_GPL_H_ */
