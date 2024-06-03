/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2016 Freescale Semiconductor, Inc.
 */

#include <config.h>

#define ROM_API_TABLE_BASE_ADDR_LEGACY		0x180
#define ROM_VERSION_OFFSET               	0x80
#define ROM_API_HWCNFG_SETUP_OFFSET		0x08

plugin_start:

	push    {r0-r4, lr}

	imx7_ddr_setting
	imx7_clock_gating
	imx7_qos_setting

/*
 * Check if we are in USB serial download mode and immediately return to ROM
 * Need to check USB CTRL clock firstly, then check the USBx_nASYNCLISTADDR
 */
	ldr r0, =0x30384680
	ldr r1, [r0]
	cmp r1, #0
	beq normal_boot

	ldr r0, =0x30B10158
	ldr r1, [r0]
	cmp r1, #0
	beq normal_boot

	pop {r0-r4, lr}
	bx lr

normal_boot:

/*
 * The following is to fill in those arguments for this ROM function
 * pu_irom_hwcnfg_setup(void **start, size_t *bytes, const void *boot_data)
 * This function is used to copy data from the storage media into DDR.
 * start - Initial (possibly partial) image load address on entry.
 *         Final image load address on exit.
 * bytes - Initial (possibly partial) image size on entry.
 *         Final image size on exit.
 * boot_data - Initial @ref ivt Boot Data load address.
 */
	adr r0, boot_data2
	adr r1, image_len2
	adr r2, boot_data2

/*
 * check the _pu_irom_api_table for the address
 */
before_calling_rom___pu_irom_hwcnfg_setup:
	ldr r3, =ROM_VERSION_OFFSET
	ldr r4, [r3]
	ldr r3, =ROM_API_TABLE_BASE_ADDR_LEGACY
	ldr r4, [r3, #ROM_API_HWCNFG_SETUP_OFFSET]
	blx r4
after_calling_rom___pu_irom_hwcnfg_setup:


/* To return to ROM from plugin, we need to fill in these argument.
 * Here is what need to do:
 * Need to construct the paramters for this function before return to ROM:
 * plugin_download(void **start, size_t *bytes, UINT32 *ivt_offset)
 */
	pop {r0-r4, lr}
	push {r5}
	ldr r5, boot_data2
	str r5, [r0]
	ldr r5, image_len2
	str r5, [r1]
	ldr r5, second_ivt_offset
	str r5, [r2]
	mov r0, #1
	pop {r5}

	/* return back to ROM code */
	bx lr

/* make the following data right in the end of the output*/
.ltorg

#define FLASH_OFFSET 0x400

/*
 * second_ivt_offset is the offset from the "second_ivt_header" to
 * "image_copy_start", which involves FLASH_OFFSET, plus the first
 * ivt_header, the plugin code size itself recorded by "ivt2_header"
 */

second_ivt_offset:      .long (ivt2_header + 0x2C + FLASH_OFFSET)

/*
 * The following is the second IVT header plus the second boot data
 */
ivt2_header:            .long 0x0
app2_code_jump_v:       .long 0x0
reserv3:                .long 0x0
dcd2_ptr:               .long 0x0
boot_data2_ptr:         .long 0x0
self_ptr2:              .long 0x0
app_code_csf2:          .long 0x0
reserv4:                .long 0x0
boot_data2:             .long 0x0
image_len2:             .long 0x0
plugin2:                .long 0x0
