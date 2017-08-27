OUTPUT_ARCH(mips)
ENTRY(vec_reset)
SECTIONS
{
  .text : 
    {
     _ftext = . ;
    *(.init*)
     eprol  =  .;
    *(.text*)
    *(.fini*)
    *(.rodata*)
     _etext  =  .;
   }

  .data :
   {
    _fdata = ALIGN(16) ;
    *(.data*)
    CONSTRUCTORS
    . = ALIGN(16);
    _gp = . + 0x8000;
    *(.sdata*)
   }
   . = ALIGN(16);
   _edata  =  .;
   _romsize = _edata - _ftext;
  .reginfo : {}
   _fbss = .;
  .sbss : {
    *(.sbss*)
    *(.scommon*)
  }
  .bss : {
    *(.bss*)
    *(COMMON)
  }
  . = ALIGN(16);
   _end = .;
}
