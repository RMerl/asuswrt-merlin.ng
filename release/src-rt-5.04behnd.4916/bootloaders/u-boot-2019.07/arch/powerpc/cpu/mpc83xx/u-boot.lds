/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2006-2010
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

OUTPUT_ARCH(powerpc)
SECTIONS
{
  /* Read-only sections, merged into text segment: */
  .text      :
  {
    arch/powerpc/cpu/mpc83xx/start.o	(.text*)
    *(.text*)
    . = ALIGN(16);
    *(SORT_BY_ALIGNMENT(SORT_BY_NAME(.rodata*)))
  }

  /* Read-write section, merged into data segment: */
  . = (. + 0x0FFF) & 0xFFFFF000;
  _erotext = .;
  PROVIDE (erotext = .);
  .reloc   :
  {
    _GOT2_TABLE_ = .;
    KEEP(*(.got2))
    KEEP(*(.got))
    _FIXUP_TABLE_ = .;
    KEEP(*(.fixup))
  }
  __got2_entries = ((_GLOBAL_OFFSET_TABLE_ - _GOT2_TABLE_) >> 2) - 1;
  __fixup_entries = (. - _FIXUP_TABLE_) >> 2;

  .data    :
  {
    *(.data*)
    *(.sdata*)
  }
  _edata  =  .;
  PROVIDE (edata = .);

  . = .;

  . = ALIGN(4);
  .u_boot_list : {
	KEEP(*(SORT(.u_boot_list*)));
  }


  . = .;
  __start___ex_table = .;
  __ex_table : { *(__ex_table) }
  __stop___ex_table = .;

  /*
   * _end - This is end of u-boot.bin image.
   * dtb will be appended here to make u-boot-dtb.bin
   */
  _end = .;

  . = ALIGN(4096);
  __init_begin = .;
  .text.init : { *(.text.init) }
  .data.init : { *(.data.init) }
  . = ALIGN(4096);
  __init_end = .;

  __bss_start = .;
  .bss (NOLOAD)       :
  {
   *(.bss*)
   *(.sbss*)
   *(COMMON)
   . = ALIGN(4);
  }
  __bss_end = . ;
  PROVIDE (end = .);
}
ENTRY(_start)
