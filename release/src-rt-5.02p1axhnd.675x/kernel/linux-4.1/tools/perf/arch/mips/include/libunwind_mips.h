/* libunwind - a platform-independent unwind library
   Copyright (C) 2008 CodeSourcery

This file is part of libunwind.

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.  */

#ifndef LIBUNWIND_H
#define LIBUNWIND_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif


#ifdef mips
# undef mips
#endif

#define UNW_TARGET      mips
#define UNW_TARGET_MIPS 1

#define _U_TDEP_QP_TRUE 0       /* see libunwind-dynamic.h  */

/* This needs to be big enough to accommodate "struct cursor", while
   leaving some slack for future expansion.  Changing this value will
   require recompiling all users of this library.  Stack allocation is
   relatively cheap and unwind-state copying is relatively rare, so we
   want to err on making it rather too big than too small.  */
   
/* FIXME for MIPS. Too big?  What do other things use for similar tasks?  */
#define UNW_TDEP_CURSOR_LEN     4096



typedef enum
  {
    UNW_MIPS_R0,
    UNW_MIPS_R1,
    UNW_MIPS_R2,
    UNW_MIPS_R3,
    UNW_MIPS_R4,
    UNW_MIPS_R5,
    UNW_MIPS_R6,
    UNW_MIPS_R7,
    UNW_MIPS_R8,
    UNW_MIPS_R9,
    UNW_MIPS_R10,
    UNW_MIPS_R11,
    UNW_MIPS_R12,
    UNW_MIPS_R13,
    UNW_MIPS_R14,
    UNW_MIPS_R15,
    UNW_MIPS_R16,
    UNW_MIPS_R17,
    UNW_MIPS_R18,
    UNW_MIPS_R19,
    UNW_MIPS_R20,
    UNW_MIPS_R21,
    UNW_MIPS_R22,
    UNW_MIPS_R23,
    UNW_MIPS_R24,
    UNW_MIPS_R25,
    UNW_MIPS_R26,
    UNW_MIPS_R27,
    UNW_MIPS_R28,
    UNW_MIPS_R29,
    UNW_MIPS_R30,
    UNW_MIPS_R31,

    UNW_MIPS_PC = 34,

    /* FIXME: Other registers!  */

    /* For MIPS, the CFA is the value of SP (r29) at the call site in the
       previous frame.  */
    UNW_MIPS_CFA,

    UNW_TDEP_LAST_REG = UNW_MIPS_R31,

    UNW_TDEP_IP = UNW_MIPS_R31,
    UNW_TDEP_SP = UNW_MIPS_R29,
    UNW_TDEP_EH = UNW_MIPS_R0   /* FIXME.  */
  }
mips_regnum_t;

typedef enum
  {
    UNW_MIPS_ABI_O32,
    UNW_MIPS_ABI_N32,
    UNW_MIPS_ABI_N64
  }
mips_abi_t;

#define UNW_TDEP_NUM_EH_REGS    2       /* FIXME for MIPS.  */


#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif /* LIBUNWIND_H */