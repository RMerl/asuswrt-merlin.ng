/*
 * ARM NIC driver size reduction. Because of the gcc -fdata-sections and -ffunction-sections flags,
 * the amount of sections increases enormously, which causes problems further down the road.
 * This linker script reduces the amount of sections.
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 */

SECTIONS {
	.alt.smp.init : { KEEP(*(.alt.smp.init)) }
	__bug_table : { KEEP(*(__bug_table)) }
	__param : { KEEP(*(__param)) }
	__ksymtab_strings : { KEEP(*(__ksymtab_strings)) }
	___ksymtab+wl_pktfwd_lut : { KEEP(*(___ksymtab+wl_pktfwd_lut)) }
	.modinfo : { KEEP(*(.modinfo)) }
	.ARM.exidx.text.unlikely : { *(.ARM.exidx.text.unlikely.*) }
	.ARM.exidx.init.text : { *(.ARM.exidx.init.text) }
	.ARM.exidx.exit.text : { *(.ARM.exidx.exit.text) }
	.ARM.exidx : { *(.ARM.exidx.*) }
	.ARM.extab.exit.text : { KEEP(*(.ARM.extab.exit.text)) }
	.ARM.extab.init.text : { KEEP(*(.ARM.extab.init.text)) }
	.ARM.extab.text.unlikely : { KEEP(*(.ARM.extab.text.unlikely.*)) }
	.ARM.extab : { *(.ARM.extab.*) }
	.rel.ARM.exidx.text.unlikely : { *(.rel.ARM.exidx.text.unlikely.*) }
	.rel.ARM.exidx.init.text : { *(.rel.ARM.exidx.init.text.*) }
	.rel.ARM.exidx.exit.text : { *(.rel.ARM.exidx.exit.text.*) }
	.rel.ARM.exidx : { *(.rel.ARM.exidx.*) }
	.text.unlikely : { *(.text.unlikely.*) }
	.text : { *(.text.*) }
	.data : { *(.data.*) }
	.bss : { *(.bss.*) }
	.rodata.str1.4 : { *(.rodata.str1.4) }
	.rodata.str : { *(.rodata.str) }
	.rodata : { *(.rodata.*) }
}
