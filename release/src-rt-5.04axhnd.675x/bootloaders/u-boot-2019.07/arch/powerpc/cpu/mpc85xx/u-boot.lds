/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2007-2009, 2011 Freescale Semiconductor, Inc.
 */

#include "config.h"

#ifdef CONFIG_RESET_VECTOR_ADDRESS
#define RESET_VECTOR_ADDRESS	CONFIG_RESET_VECTOR_ADDRESS
#else
#define RESET_VECTOR_ADDRESS	0xfffffffc
#endif

#ifndef CONFIG_SYS_MONITOR_LEN
#define CONFIG_SYS_MONITOR_LEN	0x80000
#endif

OUTPUT_ARCH(powerpc)
ENTRY(_start_e500)

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

  . = ALIGN(4);
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

#ifdef CONFIG_SYS_MPC85XX_NO_RESETVEC
  .bootpg ADDR(.text) - 0x1000 :
  {
    KEEP(arch/powerpc/cpu/mpc85xx/start.o (.bootpg))
  } :text = 0xffff
  . = ADDR(.text) + CONFIG_SYS_MONITOR_LEN;
#else
  .bootpg RESET_VECTOR_ADDRESS - 0xffc :
  {
    arch/powerpc/cpu/mpc85xx/start.o	(.bootpg)
  } :text = 0xffff

  .resetvec RESET_VECTOR_ADDRESS :
  {
    KEEP(*(.resetvec))
  } :text = 0xffff

  . = RESET_VECTOR_ADDRESS + 0x4;

  /*
   * Make sure that the bss segment isn't linked at 0x0, otherwise its
   * address won't be updated during relocation fixups.  Note that
   * this is a temporary fix.  Code to dynamically the fixup the bss
   * location will be added in the future.  When the bss relocation
   * fixup code is present this workaround should be removed.
   */
#if (RESET_VECTOR_ADDRESS == 0xfffffffc)
  . |= 0x10;
#endif
#endif

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
