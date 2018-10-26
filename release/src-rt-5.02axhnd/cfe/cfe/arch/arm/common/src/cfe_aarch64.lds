/*
 * This is the linker script for both cfe ram and cfe rom. The -Ttext linker option in tools.mk
 * will direct the linker to generate different .text address.
 */

OUTPUT_FORMAT("elf64-littleaarch64", "elf64-littleaarch64", "elf64-littleaarch64")
OUTPUT_ARCH(aarch64)
ENTRY(startup)
SECTIONS
{
	.text : {
		_ftext = . ;
		text_start = .;
		*(.text.startup)
		*(.init)
		*(.text)
		*(.text.*)
		*(.fini)
	}

	.ARM.exidx : {
		__exidx_start = .;
		*(.ARM.exidx* .gnu.linkonce.armexidx.*)
		__exidx_end = .;
		. = ALIGN(8);	
		_etext = .;
		text_end = .;
	}

	. = ALIGN(8);
	.rodata : {
		rodata_start = .;
		*(.rdata)
		*(.rdata.*)
		*(.rodata)
		*(.rodata.*)
		/* make sure there is no padding between .rodata and .data */
		. = ALIGN(8);
		rodata_end = .;
	}

	. = ALIGN(8);
	.data :
	AT ( ADDR (.rodata) + SIZEOF (.rodata) )
	{
		data_start = .;
		_fdata = .;
		*(.data)
		*(.data.*)
		*(.sdata)
		. = ALIGN(8);
		data_end = .;
		_edata = .;
	}
	_romsize = _edata - _ftext;

	. = ALIGN(8);
	.bss : {
		_fbss = .;
		bss_start = .;
		*(.sbss)
		*(.scommon)
		*(.bss)
		*(.bss.*)
		*(COMMON)
		. = ALIGN(8);
		bss_end = .;
	}

	/DISCARD/ : { *(.dynstr*) }
	/DISCARD/ : { *(.dynamic*) }
	/DISCARD/ : { *(.plt*) }
	/DISCARD/ : { *(.interp*) }
	/DISCARD/ : { *(.gnu*) }
	/DISCARD/ : { *(.rela.*) }
	/DISCARD/ : { *(.plt.*) }
	/DISCARD/ : { *(.iplt.*) }
	/DISCARD/ : { *(.igot.*) }
	_end = .;
}
