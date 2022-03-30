/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2009-2012 Freescale Semiconductor, Inc.
 */

#include "config.h"

#ifndef CONFIG_SYS_MONITOR_LEN
#define CONFIG_SYS_MONITOR_LEN	0x80000
#endif

OUTPUT_ARCH(powerpc)
/* Do we need any of these for elf?
   __DYNAMIC = 0;    */
PHDRS
{
  text PT_LOAD;
  bss PT_LOAD;
}

SECTIONS
{
  /* Read-only sections, merged into text segment: */
  . = + SIZEOF_HEADERS;
  .interp : { *(.interp) }
  .text      :
  {
    *(.text*)
   } :text
    _etext = .;
    PROVIDE (etext = .);
    .rodata    :
   {
    *(SORT_BY_ALIGNMENT(SORT_BY_NAME(.rodata*)))
  } :text

  /* Read-write section, merged into data segment: */
  . = (. + 0x00FF) & 0xFFFFFF00;
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

  .u_boot_list : {
	KEEP(*(SORT(.u_boot_list*)));
  }

  . = .;
  __start___ex_table = .;
  __ex_table : { *(__ex_table) }
  __stop___ex_table = .;

  . = ALIGN(256);
  __init_begin = .;
  .text.init : { *(.text.init) }
  .data.init : { *(.data.init) }
  . = ALIGN(256);
  __init_end = .;
  _end = .;

  .bootpg ADDR(.text) - 0x1000 :
  {
    KEEP(arch/powerpc/cpu/mpc85xx/start.o (.bootpg))
  } :text = 0xffff

  . = ADDR(.text) + CONFIG_SYS_MONITOR_LEN;

  __bss_start = .;
  .bss (NOLOAD)       :
  {
   *(.sbss*)
   *(.bss*)
   *(COMMON)
  } :bss

  . = ALIGN(4);
  __bss_end = . ;
  PROVIDE (end = .);
}
