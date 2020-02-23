/*
 * This is the linker script for both cfe ram and cfe rom. The -Ttext linker option in tools.mk
 * will direct the linker to generate different .text address.
 */

OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)
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
		. = ALIGN(4);
		_etext = .;
		text_end = .;
	}

	. = ALIGN(4);
	.rodata : {
		rodata_start = .;
		*(.rdata)
		*(.rdata.*)
		*(.rodata)
		*(.rodata.*)
		. = ALIGN(8);
		rodata_end = .;
	}

	. = ALIGN(4);
	.data :
	AT ( ADDR (.rodata) + SIZEOF (.rodata) )
	{
		data_start = .;
		_fdata = .;
		*(.data)
		*(.data.*)
		*(.sdata)
		. = ALIGN(4);
		data_end = .;
		_edata = .;
	}
	_romsize = _edata - _ftext;

	. = ALIGN(4);
	.bss : {
		_fbss = .;
		bss_start = .;
		*(.sbss)
		*(.scommon)
		*(.bss)
		*(.bss.*)
		*(COMMON)
		. = ALIGN(4);
		bss_end = .;
	}

	/DISCARD/ : { *(.dynstr*) }
	/DISCARD/ : { *(.dynamic*) }
	/DISCARD/ : { *(.plt*) }
	/DISCARD/ : { *(.interp*) }
	/DISCARD/ : { *(.gnu*) }

	_end = .;
}
