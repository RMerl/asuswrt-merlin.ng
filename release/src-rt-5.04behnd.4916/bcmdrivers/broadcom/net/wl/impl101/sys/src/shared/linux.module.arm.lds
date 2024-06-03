/*
 * ARM NIC driver size reduction. Because of the gcc -fdata-sections and -ffunction-sections flags,
 * the amount of sections increases enormously, which causes problems further down the road.
 * This linker script reduces the amount of sections.
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 */

SECTIONS {
	__ksymtab               0 : { KEEP(*(SORT(___ksymtab+*))) }
	__ksymtab_gpl           0 : { KEEP(*(SORT(___ksymtab_gpl+*))) }
	__ksymtab_unused        0 : { KEEP(*(SORT(___ksymtab_unused+*))) }
	__ksymtab_unused_gpl    0 : { KEEP(*(SORT(___ksymtab_unused_gpl+*))) }
	__ksymtab_gpl_future    0 : { KEEP(*(SORT(___ksymtab_gpl_future+*))) }
	__ksymtab_strings : { KEEP(*(__ksymtab_strings)) }
	___ksymtab+wl_pktfwd_lut : { KEEP(*(___ksymtab+wl_pktfwd_lut)) }
	__kcrctab               0 : { KEEP(*(SORT(___kcrctab+*))) }
	__kcrctab_gpl           0 : { KEEP(*(SORT(___kcrctab_gpl+*))) }
	__kcrctab_unused        0 : { KEEP(*(SORT(___kcrctab_unused+*))) }
	__kcrctab_unused_gpl    0 : { KEEP(*(SORT(___kcrctab_unused_gpl+*))) }
	__kcrctab_gpl_future    0 : { KEEP(*(SORT(___kcrctab_gpl_future+*))) }
	.alt.smp.init : { KEEP(*(.alt.smp.init)) }
	__bug_table : { KEEP(*(__bug_table)) }
	__param : { KEEP(*(__param)) }
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
	.pv_table : { KEEP(*(.pv_table)) }
	.rel.pv_table : { KEEP(*(.rel.pv_table)) }
	__versions : { KEEP(*(__versions)) }
}
