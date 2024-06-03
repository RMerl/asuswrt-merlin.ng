/****************************************************************************
*
*                        BIOS emulator and interface
*                      to Realmode X86 Emulator Library
*
*               Copyright (C) 1996-1999 SciTech Software, Inc.
*
*  ========================================================================
*
*  Permission to use, copy, modify, distribute, and sell this software and
*  its documentation for any purpose is hereby granted without fee,
*  provided that the above copyright notice appear in all copies and that
*  both that copyright notice and this permission notice appear in
*  supporting documentation, and that the name of the authors not be used
*  in advertising or publicity pertaining to distribution of the software
*  without specific, written prior permission.  The authors makes no
*  representations about the suitability of this software for any purpose.
*  It is provided "as is" without express or implied warranty.
*
*  THE AUTHORS DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
*  INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
*  EVENT SHALL THE AUTHORS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
*  CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
*  USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
*  OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
*  PERFORMANCE OF THIS SOFTWARE.
*
*  ========================================================================
*
* Language:     ANSI C
* Environment:  Any
* Developer:    Kendall Bennett
*
* Description:  Header file for the real mode x86 BIOS emulator, which is
*               used to warmboot any number of VGA compatible PCI/AGP
*               controllers under any OS, on any processor family that
*               supports PCI. We also allow the user application to call
*               real mode BIOS functions and Int 10h functions (including
*               the VESA BIOS).
*
****************************************************************************/

#ifndef __BIOSEMU_H
#define __BIOSEMU_H

#include <bios_emul.h>

#ifdef __KERNEL__
#include "x86emu.h"
#else
#include "x86emu.h"
#include "pmapi.h"
#include "pcilib.h"
#endif

/*---------------------- Macros and type definitions ----------------------*/

#pragma pack(1)

#define CRT_C   24		/* 24  CRT Controller Registers             */
#define ATT_C   21		/* 21  Attribute Controller Registers       */
#define GRA_C   9		/* 9   Graphics Controller Registers        */
#define SEQ_C   5		/* 5   Sequencer Registers                  */
#define PAL_C   768		/* 768 Palette Registers                    */

/****************************************************************************
REMARKS:
Data structure used to describe the details for the BIOS emulator system
environment as used by the X86 emulator library.

HEADER:
biosemu.h

MEMBERS:
vgaInfo         - VGA BIOS information structure
biosmem_base    - Base of the BIOS image
biosmem_limit   - Limit of the BIOS image
busmem_base     - Base of the VGA bus memory
timer           - Timer used to emulate PC timer ports
timer0          - Latched value for timer 0
timer0Latched   - true if timer 0 value was just latched
timer2          - Current value for timer 2
emulateVGA      - true to emulate VGA I/O and memory accesses
****************************************************************************/

typedef struct {
	BE_VGAInfo vgaInfo;
	ulong biosmem_base;
	ulong biosmem_limit;
	ulong busmem_base;

	u32 timer0;
	int timer0Latched;
	u32 timer1;
	int timer1Latched;
	u32 timer2;
	int timer2Latched;

	int emulateVGA;
	u8 emu61;
	u8 emu70;
	int flipFlop3C0;
	u32 configAddress;
	u8 emu3C0;
	u8 emu3C1[ATT_C];
	u8 emu3C2;
	u8 emu3C4;
	u8 emu3C5[SEQ_C];
	u8 emu3C6;
	uint emu3C7;
	uint emu3C8;
	u8 emu3C9[PAL_C];
	u8 emu3CE;
	u8 emu3CF[GRA_C];
	u8 emu3D4;
	u8 emu3D5[CRT_C];
	u8 emu3DA;

} BE_sysEnv;

#ifdef __KERNEL__

/* Define some types when compiling for the Linux kernel that normally
 * come from the SciTech PM library.
 */

/****************************************************************************
REMARKS:
Structure describing the 32-bit extended x86 CPU registers

HEADER:
pmapi.h

MEMBERS:
eax     - Value of the EAX register
ebx     - Value of the EBX register
ecx     - Value of the ECX register
edx     - Value of the EDX register
esi     - Value of the ESI register
edi     - Value of the EDI register
cflag   - Value of the carry flag
****************************************************************************/
typedef struct {
	u32 eax;
	u32 ebx;
	u32 ecx;
	u32 edx;
	u32 esi;
	u32 edi;
	u32 cflag;
} RMDWORDREGS;

/****************************************************************************
REMARKS:
Structure describing the 16-bit x86 CPU registers

HEADER:
pmapi.h

MEMBERS:
ax      - Value of the AX register
bx      - Value of the BX register
cx      - Value of the CX register
dx      - Value of the DX register
si      - Value of the SI register
di      - Value of the DI register
cflag   - Value of the carry flag
****************************************************************************/
#ifdef __BIG_ENDIAN__
typedef struct {
	u16 ax_hi, ax;
	u16 bx_hi, bx;
	u16 cx_hi, cx;
	u16 dx_hi, dx;
	u16 si_hi, si;
	u16 di_hi, di;
	u16 cflag_hi, cflag;
} RMWORDREGS;
#else
typedef struct {
	u16 ax, ax_hi;
	u16 bx, bx_hi;
	u16 cx, cx_hi;
	u16 dx, dx_hi;
	u16 si, si_hi;
	u16 di, di_hi;
	u16 cflag, cflag_hi;
} RMWORDREGS;
#endif

/****************************************************************************
REMARKS:
Structure describing the 8-bit x86 CPU registers

HEADER:
pmapi.h

MEMBERS:
al      - Value of the AL register
ah      - Value of the AH register
bl      - Value of the BL register
bh      - Value of the BH register
cl      - Value of the CL register
ch      - Value of the CH register
dl      - Value of the DL register
dh      - Value of the DH register
****************************************************************************/
#ifdef __BIG_ENDIAN__
typedef struct {
	u16 ax_hi;
	u8 ah, al;
	u16 bx_hi;
	u8 bh, bl;
	u16 cx_hi;
	u8 ch, cl;
	u16 dx_hi;
	u8 dh, dl;
} RMBYTEREGS;
#else
typedef struct {
	u8 al;
	u8 ah;
	u16 ax_hi;
	u8 bl;
	u8 bh;
	u16 bx_hi;
	u8 cl;
	u8 ch;
	u16 cx_hi;
	u8 dl;
	u8 dh;
	u16 dx_hi;
} RMBYTEREGS;
#endif

/****************************************************************************
REMARKS:
Structure describing all the x86 CPU registers

HEADER:
pmapi.h

MEMBERS:
e   - Member to access registers as 32-bit values
x   - Member to access registers as 16-bit values
h   - Member to access registers as 8-bit values
****************************************************************************/
typedef union {
	RMDWORDREGS e;
	RMWORDREGS x;
	RMBYTEREGS h;
} RMREGS;

/****************************************************************************
REMARKS:
Structure describing all the x86 segment registers

HEADER:
pmapi.h

MEMBERS:
es  - ES segment register
cs  - CS segment register
ss  - SS segment register
ds  - DS segment register
fs  - FS segment register
gs  - GS segment register
****************************************************************************/
typedef struct {
	u16 es;
	u16 cs;
	u16 ss;
	u16 ds;
	u16 fs;
	u16 gs;
} RMSREGS;

#endif				/* __KERNEL__ */

#ifndef __KERNEL__

/****************************************************************************
REMARKS:
Structure defining all the BIOS Emulator API functions as exported from
the Binary Portable DLL.
{secret}
****************************************************************************/
typedef struct {
	ulong dwSize;
	 ibool(PMAPIP BE_init) (u32 debugFlags, int memSize, BE_VGAInfo * info);
	void (PMAPIP BE_setVGA) (BE_VGAInfo * info);
	void (PMAPIP BE_getVGA) (BE_VGAInfo * info);
	void *(PMAPIP BE_mapRealPointer) (uint r_seg, uint r_off);
	void *(PMAPIP BE_getVESABuf) (uint * len, uint * rseg, uint * roff);
	void (PMAPIP BE_callRealMode) (uint seg, uint off, RMREGS * regs,
				       RMSREGS * sregs);
	int (PMAPIP BE_int86) (int intno, RMREGS * in, RMREGS * out);
	int (PMAPIP BE_int86x) (int intno, RMREGS * in, RMREGS * out,
				RMSREGS * sregs);
	void *reserved1;
	void (PMAPIP BE_exit) (void);
} BE_exports;

/****************************************************************************
REMARKS:
Function pointer type for the Binary Portable DLL initialisation entry point.
{secret}
****************************************************************************/
typedef BE_exports *(PMAPIP BE_initLibrary_t) (PM_imports * PMImp);
#endif

#pragma pack()

/*---------------------------- Global variables ---------------------------*/

#ifdef  __cplusplus
extern "C" {			/* Use "C" linkage when in C++ mode */
#endif

/* {secret} Global BIOS emulator system environment */
	extern BE_sysEnv _BE_env;

/*-------------------------- Function Prototypes --------------------------*/

/* BIOS emulator library entry points */
	int X86API BE_init(u32 debugFlags, int memSize, BE_VGAInfo * info,
			   int shared);
	void X86API BE_setVGA(BE_VGAInfo * info);
	void X86API BE_getVGA(BE_VGAInfo * info);
	void X86API BE_setDebugFlags(u32 debugFlags);
	void *X86API BE_mapRealPointer(uint r_seg, uint r_off);
	void *X86API BE_getVESABuf(uint * len, uint * rseg, uint * roff);
	void X86API BE_callRealMode(uint seg, uint off, RMREGS * regs,
				    RMSREGS * sregs);
	int X86API BE_int86(int intno, RMREGS * in, RMREGS * out);
	int X86API BE_int86x(int intno, RMREGS * in, RMREGS * out,
			     RMSREGS * sregs);
	void X86API BE_exit(void);

#ifdef  __cplusplus
}				/* End of "C" linkage for C++       */
#endif
#endif				/* __BIOSEMU_H */
