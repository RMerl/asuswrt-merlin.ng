/* SPDX-License-Identifier: GPL-2.0+
 *
 *  Copyright 2019 Broadcom Ltd.
 */

#include <asm/arch/timer.h>
#include <asm/arch/misc.h>

/*
The algorithm to disable AVS in PMC ROM if enabled in STRAP:

   if (POR) {
      clear_marker();
   }

   if (AVS_ENABLED) {
     override (AVS_DISABLE);
     set_marker();
     do_soft_reset();
   }

   if (is_marker_set()) {
     override (AVS_ENABLE);
   }

   clear_marker();
*/
 ///////////////////////////////////////////
	// Check if this is POR
	ldr	r0, =TIMR_BASE
	ldr	r1, [r0, #TIMER_RESET_STATUS]
	tst	r1, #(TIMER_RESET_STATUS_POR)
	beq	do_avs_disabled
	// for POR, always clear marker
	ldr	r0, =MISC_BASE
	ldr	r1, [r0, #MISC_STRAP_BUS]
	bic	r1, r1, #(MISC_STRAP_BUS_PMC_AVS_OVERRIDE_MARKER)
	str	r1, [r0, #MISC_STRAP_BUS]

do_avs_disabled:
	ldr	r0, =MISC_BASE
	ldr	r1, [r0, #MISC_STRAP_BUS]
	// Check if AVS "was" intentionally disabled earlier
	tst	r1, #(MISC_STRAP_BUS_PMC_BOOT_AVS)
	beq	do_marker_check
	// Since AVS is enabled,
	// disable AVS and do soft reset
	bic	r1, r1, #(MISC_STRAP_BUS_PMC_BOOT_AVS)
	// Set marker to indicate AVS was disabled
	orr	r1, r1, #(MISC_STRAP_BUS_PMC_AVS_OVERRIDE_MARKER)
	str	r1, [r0, #MISC_STRAP_BUS]
	// Save changes (toggle override)
	mov	r1, #1
	str	r1, [r0, #MISC_STRAP_BUS_OVERRIDE]
	mov	r1, #0
	str	r1, [r0, #MISC_STRAP_BUS_OVERRIDE]
	//soft reset the chip
	ldr	r0, =TIMR_BASE
	mov	r1, #1
	str	r1, [r0, #TIMER_WD_RESET]
	// See you in next life
rstc:	
	b	rstc

do_marker_check:
	// re-enable AVS only if it "was" intentionally disabled earlier
	tst r1, #(MISC_STRAP_BUS_PMC_AVS_OVERRIDE_MARKER)
	beq do_clear_marker
	// Enable AVS
	orr	r1, r1, #(MISC_STRAP_BUS_PMC_BOOT_AVS)
        
do_clear_marker:
	// Clear marker
	bic	r1, r1, #(MISC_STRAP_BUS_PMC_AVS_OVERRIDE_MARKER)
	str	r1, [r0, #MISC_STRAP_BUS]
	// Save changes (toggle override)
	mov	r1, #1
	str	r1, [r0, #MISC_STRAP_BUS_OVERRIDE]
	mov	r1, #0
	str	r1, [r0, #MISC_STRAP_BUS_OVERRIDE]
