/****************************************************************************
*
*			 BIOS emulator and interface
*		       to Realmode X86 Emulator Library
*
*  Copyright (C) 2007 Freescale Semiconductor, Inc.
*  Jason Jin <Jason.jin@freescale.com>
*
*		Copyright (C) 1996-1999 SciTech Software, Inc.
*
*  ========================================================================
*
*  Permission to use, copy, modify, distribute, and sell this software and
*  its documentation for any purpose is hereby granted without fee,
*  provided that the above copyright notice appear in all copies and that
*  both that copyright notice and this permission notice appear in
*  supporting documentation, and that the name of the authors not be used
*  in advertising or publicity pertaining to distribution of the software
*  without specific, written prior permission.	The authors makes no
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
* Language:	ANSI C
* Environment:	Any
* Developer:	Kendall Bennett
*
* Description:	Module implementing the system specific functions. This
*		module is always compiled and linked in the OS depedent
*		libraries, and never in a binary portable driver.
*
*		Jason ported this file to u-boot to run the ATI video card BIOS
*		in u-boot. Made all the video memory be emulated during the
*		BIOS runing process which may affect the VGA function but the
*		frambuffer function can work after run the BIOS.
*
****************************************************************************/

#include <malloc.h>
#include <common.h>
#include "biosemui.h"

BE_sysEnv _BE_env = {{0}};
static X86EMU_memFuncs _BE_mem __section(GOT2_TYPE) = {
	BE_rdb,
	BE_rdw,
	BE_rdl,
	BE_wrb,
	BE_wrw,
	BE_wrl,
	};

static X86EMU_pioFuncs _BE_pio __section(GOT2_TYPE) = {
	BE_inb,
	BE_inw,
	BE_inl,
	BE_outb,
	BE_outw,
	BE_outl,
	};

#define OFF(addr)	(u16)(((addr) >> 0) & 0xffff)
#define SEG(addr)	(u16)(((addr) >> 4) & 0xf000)

/****************************************************************************
PARAMETERS:
debugFlags  - Flags to enable debugging options (debug builds only)
memSize	    - Amount of memory to allocate for real mode machine
info	    - Pointer to default VGA device information

REMARKS:
This functions initialises the BElib, and uses the passed in
BIOS image as the BIOS that is used and emulated at 0xC0000.
****************************************************************************/
int X86API BE_init(u32 debugFlags, int memSize, BE_VGAInfo * info, int shared)
{
#if !defined(__DRIVER__)  && !defined(__KERNEL__)

	PM_init();
#endif
	memset(&M, 0, sizeof(M));
	if (memSize < 20480){
		printf("Emulator requires at least 20Kb of memory!\n");
		return 0;
	}

	M.mem_base = malloc(memSize);

	if (M.mem_base == NULL){
		printf("Biosemu:Out of memory!");
		return 0;
	}
	M.mem_size = memSize;

	_BE_env.emulateVGA = 0;
	_BE_env.busmem_base = (unsigned long)malloc(128 * 1024);
	if ((void *)_BE_env.busmem_base == NULL){
		printf("Biosemu:Out of memory!");
		return 0;
	}
	M.x86.debug = debugFlags;
	_BE_bios_init((u32*)info->LowMem);
	X86EMU_setupMemFuncs(&_BE_mem);
	X86EMU_setupPioFuncs(&_BE_pio);
	BE_setVGA(info);
	return 1;
}

/****************************************************************************
PARAMETERS:
info	    - Pointer to VGA device information to make current

REMARKS:
This function sets the VGA BIOS functions in the emulator to point to the
specific VGA BIOS in use. This includes swapping the BIOS interrupt
vectors, BIOS image and BIOS data area to the new BIOS. This allows the
real mode BIOS to be swapped without resetting the entire emulator.
****************************************************************************/
void X86API BE_setVGA(BE_VGAInfo * info)
{

#ifdef __KERNEL__
	_BE_env.vgaInfo.function = info->function;
	_BE_env.vgaInfo.device = info->device;
	_BE_env.vgaInfo.bus = info->bus;
	_BE_env.vgaInfo.pcidev = info->pcidev;
#else
	_BE_env.vgaInfo.pciInfo = info->pciInfo;
#endif
	_BE_env.vgaInfo.BIOSImage = info->BIOSImage;
	if (info->BIOSImage) {
		_BE_env.biosmem_base = (ulong) info->BIOSImage;
		_BE_env.biosmem_limit = 0xC0000 + info->BIOSImageLen - 1;
	} else {
		_BE_env.biosmem_base = _BE_env.busmem_base + 0x20000;
		_BE_env.biosmem_limit = 0xC7FFF;
	}
	if ((info->LowMem[0] == 0) && (info->LowMem[1] == 0) &&
	    (info->LowMem[2] == 0) && (info->LowMem[3] == 0))
		_BE_bios_init((u32 *) info->LowMem);
	memcpy((u8 *) M.mem_base, info->LowMem, sizeof(info->LowMem));
}

/****************************************************************************
PARAMETERS:
info	    - Pointer to VGA device information to retrieve current

REMARKS:
This function returns the VGA BIOS functions currently active in the
emulator, so they can be restored at a later date.
****************************************************************************/
void X86API BE_getVGA(BE_VGAInfo * info)
{
#ifdef __KERNEL__
	info->function = _BE_env.vgaInfo.function;
	info->device = _BE_env.vgaInfo.device;
	info->bus = _BE_env.vgaInfo.bus;
	info->pcidev = _BE_env.vgaInfo.pcidev;
#else
	info->pciInfo = _BE_env.vgaInfo.pciInfo;
#endif
	info->BIOSImage = _BE_env.vgaInfo.BIOSImage;
	memcpy(info->LowMem, (u8 *) M.mem_base, sizeof(info->LowMem));
}

/****************************************************************************
PARAMETERS:
r_seg	- Segment for pointer to convert
r_off	- Offset for pointer to convert

REMARKS:
This function maps a real mode pointer in the emulator memory to a protected
mode pointer that can be used to directly access the memory.

NOTE:	The memory is *always* in little endian format, son on non-x86
	systems you will need to do endian translations to access this
	memory.
****************************************************************************/
void *X86API BE_mapRealPointer(uint r_seg, uint r_off)
{
	u32 addr = ((u32) r_seg << 4) + r_off;

	if (addr >= 0xC0000 && addr <= _BE_env.biosmem_limit) {
		return (void *)(_BE_env.biosmem_base + addr - 0xC0000);
	} else if (addr >= 0xA0000 && addr <= 0xFFFFF) {
		return (void *)(_BE_env.busmem_base + addr - 0xA0000);
	}
	return (void *)(M.mem_base + addr);
}

/****************************************************************************
PARAMETERS:
len	- Return the length of the VESA buffer
rseg	- Place to store VESA buffer segment
roff	- Place to store VESA buffer offset

REMARKS:
This function returns the address of the VESA transfer buffer in real
_BE_piomode emulator memory. The VESA transfer buffer is always 1024 bytes long,
and located at 15Kb into the start of the real mode memory (16Kb is where
we put the real mode code we execute for issuing interrupts).

NOTE:	The memory is *always* in little endian format, son on non-x86
	systems you will need to do endian translations to access this
	memory.
****************************************************************************/
void *X86API BE_getVESABuf(uint * len, uint * rseg, uint * roff)
{
	*len = 1024;
	*rseg = SEG(0x03C00);
	*roff = OFF(0x03C00);
	return (void *)(M.mem_base + ((u32) * rseg << 4) + *roff);
}

/****************************************************************************
REMARKS:
Cleans up and exits the emulator.
****************************************************************************/
void X86API BE_exit(void)
{
	free(M.mem_base);
	free((void *)_BE_env.busmem_base);
}

/****************************************************************************
PARAMETERS:
seg	- Segment of code to call
off	- Offset of code to call
regs	- Real mode registers to load
sregs	- Real mode segment registers to load

REMARKS:
This functions calls a real mode far function at the specified address,
and loads all the x86 registers from the passed in registers structure.
On exit the registers returned from the call are returned in the same
structures.
****************************************************************************/
void X86API BE_callRealMode(uint seg, uint off, RMREGS * regs, RMSREGS * sregs)
{
	M.x86.R_EAX = regs->e.eax;
	M.x86.R_EBX = regs->e.ebx;
	M.x86.R_ECX = regs->e.ecx;
	M.x86.R_EDX = regs->e.edx;
	M.x86.R_ESI = regs->e.esi;
	M.x86.R_EDI = regs->e.edi;
	M.x86.R_DS = sregs->ds;
	M.x86.R_ES = sregs->es;
	M.x86.R_FS = sregs->fs;
	M.x86.R_GS = sregs->gs;

	((u8 *) M.mem_base)[0x4000] = 0x9A;
	((u8 *) M.mem_base)[0x4001] = (u8) off;
	((u8 *) M.mem_base)[0x4002] = (u8) (off >> 8);
	((u8 *) M.mem_base)[0x4003] = (u8) seg;
	((u8 *) M.mem_base)[0x4004] = (u8) (seg >> 8);
	((u8 *) M.mem_base)[0x4005] = 0xF1;	/* Illegal op-code */
	M.x86.R_CS = SEG(0x04000);
	M.x86.R_IP = OFF(0x04000);

	M.x86.R_SS = SEG(M.mem_size - 2);
	M.x86.R_SP = OFF(M.mem_size - 2) + 2;

	X86EMU_exec();

	regs->e.cflag = M.x86.R_EFLG & F_CF;
	regs->e.eax = M.x86.R_EAX;
	regs->e.ebx = M.x86.R_EBX;
	regs->e.ecx = M.x86.R_ECX;
	regs->e.edx = M.x86.R_EDX;
	regs->e.esi = M.x86.R_ESI;
	regs->e.edi = M.x86.R_EDI;
	sregs->ds = M.x86.R_DS;
	sregs->es = M.x86.R_ES;
	sregs->fs = M.x86.R_FS;
	sregs->gs = M.x86.R_GS;
}

/****************************************************************************
PARAMETERS:
intno	- Interrupt number to execute
in	- Real mode registers to load
out	- Place to store resulting real mode registers

REMARKS:
This functions calls a real mode interrupt function at the specified address,
and loads all the x86 registers from the passed in registers structure.
On exit the registers returned from the call are returned in out stucture.
****************************************************************************/
int X86API BE_int86(int intno, RMREGS * in, RMREGS * out)
{
	M.x86.R_EAX = in->e.eax;
	M.x86.R_EBX = in->e.ebx;
	M.x86.R_ECX = in->e.ecx;
	M.x86.R_EDX = in->e.edx;
	M.x86.R_ESI = in->e.esi;
	M.x86.R_EDI = in->e.edi;
	((u8 *) M.mem_base)[0x4000] = 0xCD;
	((u8 *) M.mem_base)[0x4001] = (u8) intno;
	((u8 *) M.mem_base)[0x4002] = 0xF1;
	M.x86.R_CS = SEG(0x04000);
	M.x86.R_IP = OFF(0x04000);

	M.x86.R_SS = SEG(M.mem_size - 1);
	M.x86.R_SP = OFF(M.mem_size - 1) - 1;

	X86EMU_exec();
	out->e.cflag = M.x86.R_EFLG & F_CF;
	out->e.eax = M.x86.R_EAX;
	out->e.ebx = M.x86.R_EBX;
	out->e.ecx = M.x86.R_ECX;
	out->e.edx = M.x86.R_EDX;
	out->e.esi = M.x86.R_ESI;
	out->e.edi = M.x86.R_EDI;
	return out->x.ax;
}

/****************************************************************************
PARAMETERS:
intno	- Interrupt number to execute
in	- Real mode registers to load
out	- Place to store resulting real mode registers
sregs	- Real mode segment registers to load

REMARKS:
This functions calls a real mode interrupt function at the specified address,
and loads all the x86 registers from the passed in registers structure.
On exit the registers returned from the call are returned in out stucture.
****************************************************************************/
int X86API BE_int86x(int intno, RMREGS * in, RMREGS * out, RMSREGS * sregs)
{
	M.x86.R_EAX = in->e.eax;
	M.x86.R_EBX = in->e.ebx;
	M.x86.R_ECX = in->e.ecx;
	M.x86.R_EDX = in->e.edx;
	M.x86.R_ESI = in->e.esi;
	M.x86.R_EDI = in->e.edi;
	M.x86.R_DS = sregs->ds;
	M.x86.R_ES = sregs->es;
	M.x86.R_FS = sregs->fs;
	M.x86.R_GS = sregs->gs;
	((u8 *) M.mem_base)[0x4000] = 0xCD;
	((u8 *) M.mem_base)[0x4001] = (u8) intno;
	((u8 *) M.mem_base)[0x4002] = 0xF1;
	M.x86.R_CS = SEG(0x04000);
	M.x86.R_IP = OFF(0x04000);

	M.x86.R_SS = SEG(M.mem_size - 1);
	M.x86.R_SP = OFF(M.mem_size - 1) - 1;

	X86EMU_exec();
	out->e.cflag = M.x86.R_EFLG & F_CF;
	out->e.eax = M.x86.R_EAX;
	out->e.ebx = M.x86.R_EBX;
	out->e.ecx = M.x86.R_ECX;
	out->e.edx = M.x86.R_EDX;
	out->e.esi = M.x86.R_ESI;
	out->e.edi = M.x86.R_EDI;
	sregs->ds = M.x86.R_DS;
	sregs->es = M.x86.R_ES;
	sregs->fs = M.x86.R_FS;
	sregs->gs = M.x86.R_GS;
	return out->x.ax;
}
