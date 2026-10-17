/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Workaround for "PSCI bug" on DragonBoard 410c
 * Copyright (C) 2021 Stephan Gerhold <stephan@gerhold.net>
 *
 * Syscall parameters taken from Qualcomm's LK fork (scm.h):
 * Copyright (c) 2011-2015, The Linux Foundation. All rights reserved.
 *
 * The PSCI implementation in the TrustZone/tz firmware on DragonBoard 410c has
 * a bug that starts all other CPU cores in 32-bit mode unless the TZ syscall
 * that switches from 32-bit to 64-bit mode is executed at least once.
 *
 * Normally this happens inside Qualcomm's LK bootloader which runs in 32-bit
 * mode and uses the TZ syscall to boot a kernel in 64-bit mode. However, if
 * U-Boot is installed to the "aboot" partition (replacing LK) the switch to
 * 64-bit mode never happens since U-Boot is already running in 64-bit mode.
 *
 * A workaround for this "PSCI bug" is to execute the TZ syscall when entering
 * U-Boot. That way PSCI is made aware of the 64-bit switch and starts all other
 * CPU cores in 64-bit mode as well.
 */
#include <linux/arm-smccc.h>

#define ARM_SMCCC_SIP32_FAST_CALL \
	ARM_SMCCC_CALL_VAL(ARM_SMCCC_FAST_CALL, ARM_SMCCC_SMC_32, ARM_SMCCC_OWNER_SIP, 0)

	/*
	 * U-Boot might be started in EL2 or EL3 with custom firmware.
	 * In that case, we assume that the workaround is not necessary or is
	 * handled already by the alternative firmware. Using the syscall in EL2
	 * would demote U-Boot to EL1; in EL3 it would probably just crash.
	 */
	mrs	x0, CurrentEL
	cmp	x0, #(1 << 2)	/* EL1 */
	bne	reset

	/* Prepare TZ syscall parameters */
	mov	x0, #ARM_SMCCC_SIP32_FAST_CALL
	movk	x0, #0x10f	/* SCM_SVC_MILESTONE_CMD_ID */
	mov	x1, #0x12	/* MAKE_SCM_ARGS(0x2, SMC_PARAM_TYPE_BUFFER_READ) */
	adr	x2, el1_system_param
	mov	x3, el1_system_param_end - el1_system_param

	/* Switch PSCI to 64-bit mode. Resets CPU and returns at el1_elr */
	smc	#0

	/* Something went wrong, perhaps PSCI is already in 64-bit mode? */
	b	reset

	.align	3
el1_system_param:
	.quad	0, 0, 0, 0, 0, 0, 0, 0, 0	/* el1_x0-x8 */
	.quad	reset				/* el1_elr */
el1_system_param_end:
