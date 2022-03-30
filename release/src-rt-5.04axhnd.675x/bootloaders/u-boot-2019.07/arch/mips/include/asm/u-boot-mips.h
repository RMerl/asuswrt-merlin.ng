/* SPDX-License-Identifier: GPL-2.0+ */

#ifndef _U_BOOT_MIPS_H_
#define _U_BOOT_MIPS_H_

void exc_handler(void);
void except_vec3_generic(void);
void except_vec_ejtag_debug(void);

int arch_misc_init(void);

#endif /* _U_BOOT_MIPS_H_ */
