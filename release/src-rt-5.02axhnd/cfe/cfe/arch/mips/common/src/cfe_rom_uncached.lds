OUTPUT_ARCH(mips)
ENTRY(vec_reset)
SECTIONS
{
  . = 0xBFC00000;
  .text : {
     _ftext = . ;
    *(.init)
     eprol  =  .;
    *(.text)
    *(.fini)
    *(.rodata)
     _etext  =  .;
   }

  .data 0xA1F00000 :
    AT ( ((ADDR(.text)|0xB0000000) + SIZEOF ( .text ) + 15) & 0xFFFFFFF0)
   {
    _fdata = ALIGN(16) ;
    *(.data)
    CONSTRUCTORS
    . = ALIGN(16);
    _gp = . + 0x8000;
    *(.sdata)
   }
   . = ALIGN(16);
   _edata  =  .;
   _fbss = .;
  .sbss : {
    *(.sbss)
    *(.scommon)
  }
  .bss : {
    *(.bss)
    *(COMMON)
  }
  . = ALIGN(16);
   _end = .;
}
