/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2009 Wolfgang Denk <wd@denx.de>
 */

#include <ppc_asm.tmpl>

	.file	"reloc.S"

	.text
#ifndef CONFIG_NAND_SPL
	/*
	 * Function: relocate entries for one exception vector
	 */
	.globl trap_reloc
	.type	trap_reloc, @function
trap_reloc:
	lwz	r0, 0(r7)		/* hdlr ...		*/
	add	r0, r0, r3		/*  ... += dest_addr	*/
	stw	r0, 0(r7)

	lwz	r0, 4(r7)		/* int_return ...	*/
	add	r0, r0, r3		/*  ... += dest_addr	*/
	stw	r0, 4(r7)

	lwz	r0, 8(r7)		/* transfer_to_handler ...*/
	add	r0, r0, r3		/*  ... += dest_addr	*/
	stw	r0, 8(r7)

	blr
	.size	trap_reloc, .-trap_reloc
#endif
