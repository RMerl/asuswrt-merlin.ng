/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2016 NXP Semiconductor, Inc.
 */

#include <config.h>
#include <linux/linkage.h>
#include <asm/system.h>
#include <asm/macro.h>

WEAK(_sec_firmware_entry)
	/*
	 * x0: Secure Firmware entry point
	 * x1: Exception return address Low
	 * x2: Exception return address High
	 */

	/* Save stack pointer for EL2 */
	mov	x3, sp
	msr	sp_el2, x3

	/* Set exception return address hold pointer */
        adr	x4, 1f
        mov	x3, x4
#ifdef CONFIG_ARMV8_SEC_FIRMWARE_ERET_ADDR_REVERT
        rev	w3, w3
#endif
        str	w3, [x1]
        lsr	x3, x4, #32
#ifdef CONFIG_ARMV8_SEC_FIRMWARE_ERET_ADDR_REVERT
        rev	w3, w3
#endif
        str	w3, [x2]

	/* Call SEC monitor */
        br	x0

1:
        mov	x0, #0
        ret
ENDPROC(_sec_firmware_entry)

#ifdef CONFIG_SEC_FIRMWARE_ARMV8_PSCI
ENTRY(_sec_firmware_support_psci_version)
	mov	x0, 0x84000000
	mov	x1, 0x0
	mov	x2, 0x0
	mov	x3, 0x0
	smc	#0
	ret
ENDPROC(_sec_firmware_support_psci_version)

/*
 * Switch from AArch64 EL2 to AArch32 EL2
 * @param inputs:
 * x0: argument, zero
 * x1: machine nr
 * x2: fdt address
 * x3: input argument
 * x4: kernel entry point
 * @param outputs for secure firmware:
 * x0: function id
 * x1: kernel entry point
 * x2: machine nr
 * x3: fdt address
*/
ENTRY(armv8_el2_to_aarch32)
	mov	x3, x2
	mov	x2, x1
	mov	x1, x4
	ldr	x0, =0xc200ff17
	smc	#0
	ret
ENDPROC(armv8_el2_to_aarch32)
#endif
