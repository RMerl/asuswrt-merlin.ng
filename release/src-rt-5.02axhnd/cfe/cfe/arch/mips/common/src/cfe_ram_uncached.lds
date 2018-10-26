OUTPUT_ARCH(mips)
ENTRY(vec_reset)
SECTIONS
{
  . = 0xbfc00000;		/* was 0x9fc00000 */
  .text : 
/*    AT ( 0xBFC00000 ) */		/* was 0xbfc00000 */
    {
     _ftext = . ;
    *(.init)
     eprol  =  .;
    *(.text)
    *(.fini)
    *(.rodata)
     _etext  =  .;
   }

  .data :
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
