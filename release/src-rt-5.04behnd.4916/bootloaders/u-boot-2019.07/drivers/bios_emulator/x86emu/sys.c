/****************************************************************************
*
*                       Realmode X86 Emulator Library
*
*               Copyright (C) 1991-2004 SciTech Software, Inc.
*                    Copyright (C) David Mosberger-Tang
*                      Copyright (C) 1999 Egbert Eich
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
* Description:  This file includes subroutines which are related to
*               programmed I/O and memory access. Included in this module
*               are default functions that do nothing. For real uses these
*               functions will have to be overridden by the user library.
*
****************************************************************************/

#include <common.h>
#include "x86emu/x86emui.h"

/*------------------------- Global Variables ------------------------------*/

X86EMU_sysEnv _X86EMU_env;	/* Global emulator machine state */
X86EMU_intrFuncs _X86EMU_intrTab[256];

int debug_intr;

/*----------------------------- Implementation ----------------------------*/

/****************************************************************************
PARAMETERS:
addr    - Emulator memory address to read

RETURNS:
Byte value read from emulator memory.

REMARKS:
Reads a byte value from the emulator memory.
****************************************************************************/
u8 X86API rdb(u32 addr)
{
	return 0;
}

/****************************************************************************
PARAMETERS:
addr    - Emulator memory address to read

RETURNS:
Word value read from emulator memory.

REMARKS:
Reads a word value from the emulator memory.
****************************************************************************/
u16 X86API rdw(u32 addr)
{
	return 0;
}

/****************************************************************************
PARAMETERS:
addr    - Emulator memory address to read

RETURNS:
Long value read from emulator memory.
REMARKS:
Reads a long value from the emulator memory.
****************************************************************************/
u32 X86API rdl(u32 addr)
{
	return 0;
}

/****************************************************************************
PARAMETERS:
addr    - Emulator memory address to read
val     - Value to store

REMARKS:
Writes a byte value to emulator memory.
****************************************************************************/
void X86API wrb(u32 addr, u8 val)
{
}

/****************************************************************************
PARAMETERS:
addr    - Emulator memory address to read
val     - Value to store

REMARKS:
Writes a word value to emulator memory.
****************************************************************************/
void X86API wrw(u32 addr, u16 val)
{
}

/****************************************************************************
PARAMETERS:
addr    - Emulator memory address to read
val     - Value to store

REMARKS:
Writes a long value to emulator memory.
****************************************************************************/
void X86API wrl(u32 addr, u32 val)
{
}

/****************************************************************************
PARAMETERS:
addr    - PIO address to read
RETURN:
0
REMARKS:
Default PIO byte read function. Doesn't perform real inb.
****************************************************************************/
static u8 X86API p_inb(X86EMU_pioAddr addr)
{
	DB(if (DEBUG_IO_TRACE())
	   printk("inb %#04x \n", addr);)
		return 0;
}

/****************************************************************************
PARAMETERS:
addr    - PIO address to read
RETURN:
0
REMARKS:
Default PIO word read function. Doesn't perform real inw.
****************************************************************************/
static u16 X86API p_inw(X86EMU_pioAddr addr)
{
	DB(if (DEBUG_IO_TRACE())
	   printk("inw %#04x \n", addr);)
		return 0;
}

/****************************************************************************
PARAMETERS:
addr    - PIO address to read
RETURN:
0
REMARKS:
Default PIO long read function. Doesn't perform real inl.
****************************************************************************/
static u32 X86API p_inl(X86EMU_pioAddr addr)
{
	DB(if (DEBUG_IO_TRACE())
	   printk("inl %#04x \n", addr);)
		return 0;
}

/****************************************************************************
PARAMETERS:
addr    - PIO address to write
val     - Value to store
REMARKS:
Default PIO byte write function. Doesn't perform real outb.
****************************************************************************/
static void X86API p_outb(X86EMU_pioAddr addr, u8 val)
{
	DB(if (DEBUG_IO_TRACE())
	   printk("outb %#02x -> %#04x \n", val, addr);)
		return;
}

/****************************************************************************
PARAMETERS:
addr    - PIO address to write
val     - Value to store
REMARKS:
Default PIO word write function. Doesn't perform real outw.
****************************************************************************/
static void X86API p_outw(X86EMU_pioAddr addr, u16 val)
{
	DB(if (DEBUG_IO_TRACE())
	   printk("outw %#04x -> %#04x \n", val, addr);)
		return;
}

/****************************************************************************
PARAMETERS:
addr    - PIO address to write
val     - Value to store
REMARKS:
Default PIO ;ong write function. Doesn't perform real outl.
****************************************************************************/
static void X86API p_outl(X86EMU_pioAddr addr, u32 val)
{
	DB(if (DEBUG_IO_TRACE())
	   printk("outl %#08x -> %#04x \n", val, addr);)
		return;
}

/*------------------------- Global Variables ------------------------------*/

u8(X86APIP sys_rdb) (u32 addr) = rdb;
u16(X86APIP sys_rdw) (u32 addr) = rdw;
u32(X86APIP sys_rdl) (u32 addr) = rdl;
void (X86APIP sys_wrb) (u32 addr, u8 val) = wrb;
void (X86APIP sys_wrw) (u32 addr, u16 val) = wrw;
void (X86APIP sys_wrl) (u32 addr, u32 val) = wrl;
u8(X86APIP sys_inb) (X86EMU_pioAddr addr) = p_inb;
u16(X86APIP sys_inw) (X86EMU_pioAddr addr) = p_inw;
u32(X86APIP sys_inl) (X86EMU_pioAddr addr) = p_inl;
void (X86APIP sys_outb) (X86EMU_pioAddr addr, u8 val) = p_outb;
void (X86APIP sys_outw) (X86EMU_pioAddr addr, u16 val) = p_outw;
void (X86APIP sys_outl) (X86EMU_pioAddr addr, u32 val) = p_outl;

/*----------------------------- Setup -------------------------------------*/

/****************************************************************************
PARAMETERS:
funcs   - New memory function pointers to make active

REMARKS:
This function is used to set the pointers to functions which access
memory space, allowing the user application to override these functions
and hook them out as necessary for their application.
****************************************************************************/
void X86EMU_setupMemFuncs(X86EMU_memFuncs * funcs)
{
	sys_rdb = funcs->rdb;
	sys_rdw = funcs->rdw;
	sys_rdl = funcs->rdl;
	sys_wrb = funcs->wrb;
	sys_wrw = funcs->wrw;
	sys_wrl = funcs->wrl;
}

/****************************************************************************
PARAMETERS:
funcs   - New programmed I/O function pointers to make active

REMARKS:
This function is used to set the pointers to functions which access
I/O space, allowing the user application to override these functions
and hook them out as necessary for their application.
****************************************************************************/
void X86EMU_setupPioFuncs(X86EMU_pioFuncs * funcs)
{
	sys_inb = funcs->inb;
	sys_inw = funcs->inw;
	sys_inl = funcs->inl;
	sys_outb = funcs->outb;
	sys_outw = funcs->outw;
	sys_outl = funcs->outl;
}

void X86EMU_setupIntrFunc(int intnum, X86EMU_intrFuncs func)
{
	_X86EMU_intrTab[intnum] = func;
}

/****************************************************************************
PARAMETERS:
funcs   - New interrupt vector table to make active

REMARKS:
This function is used to set the pointers to functions which handle
interrupt processing in the emulator, allowing the user application to
hook interrupts as necessary for their application. Any interrupts that
are not hooked by the user application, and reflected and handled internally
in the emulator via the interrupt vector table. This allows the application
to get control when the code being emulated executes specific software
interrupts.
****************************************************************************/
void X86EMU_setupIntrFuncs(X86EMU_intrFuncs funcs[])
{
	int i;

	for (i = 0; i < 256; i++)
		_X86EMU_intrTab[i] = NULL;
	if (funcs) {
		for (i = 0; i < 256; i++)
			_X86EMU_intrTab[i] = funcs[i];
	}
}

/****************************************************************************
PARAMETERS:
int - New software interrupt to prepare for

REMARKS:
This function is used to set up the emulator state to exceute a software
interrupt. This can be used by the user application code to allow an
interrupt to be hooked, examined and then reflected back to the emulator
so that the code in the emulator will continue processing the software
interrupt as per normal. This essentially allows system code to actively
hook and handle certain software interrupts as necessary.
****************************************************************************/
void X86EMU_prepareForInt(int num)
{
	push_word((u16) M.x86.R_FLG);
	CLEAR_FLAG(F_IF);
	CLEAR_FLAG(F_TF);
	push_word(M.x86.R_CS);
	M.x86.R_CS = mem_access_word(num * 4 + 2);
	push_word(M.x86.R_IP);
	M.x86.R_IP = mem_access_word(num * 4);
	M.x86.intr = 0;
}
