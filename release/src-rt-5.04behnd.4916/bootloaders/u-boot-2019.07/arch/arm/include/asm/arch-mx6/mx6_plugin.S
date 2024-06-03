/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2016 Freescale Semiconductor, Inc.
 */

#include <config.h>

#ifdef CONFIG_ROM_UNIFIED_SECTIONS
#define ROM_API_TABLE_BASE_ADDR_LEGACY		0x180
#define ROM_VERSION_OFFSET               	0x80
#else
#define ROM_API_TABLE_BASE_ADDR_LEGACY		0xC0
#define ROM_VERSION_OFFSET               	0x48
#endif
#define ROM_API_TABLE_BASE_ADDR_MX6DQ_TO15	0xC4
#define ROM_API_TABLE_BASE_ADDR_MX6DL_TO12	0xC4
#define ROM_API_HWCNFG_SETUP_OFFSET		0x08
#define ROM_VERSION_TO10			0x10
#define ROM_VERSION_TO12			0x12
#define ROM_VERSION_TO15			0x15

plugin_start:

	push    {r0-r4, lr}

	imx6_ddr_setting
	imx6_clock_gating
	imx6_qos_setting

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

#ifdef CONFIG_NOR_BOOT
#ifdef CONFIG_MX6SX
	ldr r3, =ROM_VERSION_OFFSET
	ldr r4, [r3]
	cmp r4, #ROM_VERSION_TO10
	bgt before_calling_rom___pu_irom_hwcnfg_setup
	ldr r3, =0x00900b00
	ldr r4, =0x50000000
	str r4, [r3, #0x5c]
#else
	ldr r3, =0x00900800
	ldr r4, =0x08000000
	str r4, [r3, #0xc0]
#endif
#endif

/*
 * check the _pu_irom_api_table for the address
 */
before_calling_rom___pu_irom_hwcnfg_setup:
	ldr r3, =ROM_VERSION_OFFSET
	ldr r4, [r3]
#if defined(CONFIG_MX6SOLO) || defined(CONFIG_MX6DL)
	ldr r3, =ROM_VERSION_TO12
	cmp r4, r3
	ldrge r3, =ROM_API_TABLE_BASE_ADDR_MX6DL_TO12
	ldrlt r3, =ROM_API_TABLE_BASE_ADDR_LEGACY
#elif defined(CONFIG_MX6Q)
	ldr r3, =ROM_VERSION_TO15
	cmp r4, r3
	ldrge r3, =ROM_API_TABLE_BASE_ADDR_MX6DQ_TO15
	ldrlt r3, =ROM_API_TABLE_BASE_ADDR_LEGACY
#else
	ldr r3, =ROM_API_TABLE_BASE_ADDR_LEGACY
#endif
	ldr r4, [r3, #ROM_API_HWCNFG_SETUP_OFFSET]
	blx r4
after_calling_rom___pu_irom_hwcnfg_setup:

/*
 * ROM_API_HWCNFG_SETUP function enables MMU & Caches.
 * Thus disable MMU & Caches.
 */

	mrc     p15, 0, r0, c1, c0, 0   /* read CP15 register 1 into r0*/
	ands    r0, r0, #0x1            /* check if MMU is enabled */
	beq     mmu_disable_notreq      /* exit if MMU is already disabled */

	/* Disable caches, MMU */
	mrc     p15, 0, r0, c1, c0, 0	/* read CP15 register 1 into r0 */
	bic     r0, r0, #(1 << 2)	/* disable D Cache */
	bic     r0, r0, #0x1		/* clear bit 0 ; MMU off */

	bic     r0, r0, #(0x1 << 11)	/* disable Z, branch prediction */
	bic     r0, r0, #(0x1 << 1)	/* disable A, Strict alignment */
					/* check enabled. */
	mcr     p15, 0, r0, c1, c0, 0	/* write CP15 register 1 */
	mov     r0, r0
	mov     r0, r0
	mov     r0, r0
	mov     r0, r0

mmu_disable_notreq:
    NOP

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

#if (defined(CONFIG_NOR_BOOT) || defined(CONFIG_QSPI_BOOT))
#define FLASH_OFFSET 0x1000
#else
#define FLASH_OFFSET 0x400
#endif

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
