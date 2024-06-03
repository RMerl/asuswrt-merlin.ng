/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2014, Bin Meng <bmeng.cn@gmail.com>
 */

#include <config.h>
#include <asm/post.h>

.globl car_init
car_init:
	/*
	 * Note: ebp holds the BIST value (built-in self test) so far, but ebp
	 * will be destroyed through the FSP call, thus we have to test the
	 * BIST value here before we call into FSP.
	 */
	test	%ebp, %ebp
	jz	car_init_start
	post_code(POST_BIST_FAILURE)
	jmp	die

car_init_start:
	post_code(POST_CAR_START)
	lea	find_fsp_header_romstack, %esp
	jmp	find_fsp_header

find_fsp_header_ret:
	/* EAX points to FSP_INFO_HEADER */
	mov	%eax, %ebp

	/* sanity test */
	cmp	$CONFIG_FSP_ADDR, %eax
	jb	die

	/* calculate TempRamInitEntry address */
	mov	0x30(%ebp), %eax
	add	0x1c(%ebp), %eax

	/* call FSP TempRamInitEntry to setup temporary stack */
	lea	temp_ram_init_romstack, %esp
	jmp	*%eax

temp_ram_init_ret:
	addl	$4, %esp
	cmp	$0, %eax
	jnz	car_init_fail

	post_code(POST_CAR_CPU_CACHE)

	/*
	 * The FSP TempRamInit initializes the ecx and edx registers to
	 * point to a temporary but writable memory range (Cache-As-RAM).
	 * ecx: the start of this temporary memory range,
	 * edx: the end of this range.
	 */

	/* stack grows down from top of CAR */
	movl	%edx, %esp
	subl	$4, %esp

	xor	%esi, %esi
	jmp	car_init_done

.global fsp_init_done
fsp_init_done:
	/*
	 * We come here from fsp_continue() with eax pointing to the HOB list.
	 * Save eax to esi temporarily.
	 */
	movl	%eax, %esi

car_init_done:
	/*
	 * Re-initialize the ebp (BIST) to zero, as we already reach here
	 * which means we passed BIST testing before.
	 */
	xorl	%ebp, %ebp
	jmp	car_init_ret

car_init_fail:
	post_code(POST_CAR_FAILURE)

die:
	hlt
	jmp	die
	hlt

	/*
	 * The function call before CAR initialization is tricky. It cannot
	 * be called using the 'call' instruction but only the 'jmp' with
	 * the help of a handcrafted stack in the ROM. The stack needs to
	 * contain the function return address as well as the parameters.
	 */
	.balign	4
find_fsp_header_romstack:
	.long	find_fsp_header_ret

	.balign	4
temp_ram_init_romstack:
	.long	temp_ram_init_ret
	.long	temp_ram_init_params
temp_ram_init_params:
_dt_ucode_base_size:
	/* These next two fields are filled in by binman */
.globl ucode_base
ucode_base:	/* Declared in microcode.h */
	.long	0			/* microcode base */
.globl ucode_size
ucode_size:	/* Declared in microcode.h */
	.long	0			/* microcode size */
	.long	CONFIG_SYS_MONITOR_BASE	/* code region base */
	.long	CONFIG_SYS_MONITOR_LEN	/* code region size */
