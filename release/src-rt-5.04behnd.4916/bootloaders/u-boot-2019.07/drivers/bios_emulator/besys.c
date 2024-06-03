/****************************************************************************
*
*                        BIOS emulator and interface
*                      to Realmode X86 Emulator Library
*
*  ========================================================================
*
*   Copyright (C) 2007 Freescale Semiconductor, Inc.
*   Jason Jin<Jason.jin@freescale.com>
*
*   Copyright (C) 1991-2004 SciTech Software, Inc. All rights reserved.
*
*   This file may be distributed and/or modified under the terms of the
*   GNU General Public License version 2.0 as published by the Free
*   Software Foundation and appearing in the file LICENSE.GPL included
*   in the packaging of this file.
*
*   Licensees holding a valid Commercial License for this product from
*   SciTech Software, Inc. may use this file in accordance with the
*   Commercial License Agreement provided with the Software.
*
*   This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING
*   THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
*   PURPOSE.
*
*   See http://www.scitechsoft.com/license/ for information about
*   the licensing options available and how to purchase a Commercial
*   License Agreement.
*
*   Contact license@scitechsoft.com if any conditions of this licensing
*   are not clear to you, or you have questions about licensing options.
*
*  ========================================================================
*
* Language:     ANSI C
* Environment:  Any
* Developer:    Kendall Bennett
*
* Description:  This file includes BIOS emulator I/O and memory access
*               functions.
*
*		Jason ported this file to u-boot to run the ATI video card
*		BIOS in u-boot. Removed some emulate functions such as the
*		timer port access. Made all the VGA port except reading 0x3c3
*		be emulated. Seems like reading 0x3c3 should return the high
*		16 bit of the io port.
*
****************************************************************************/

#define __io
#include <common.h>
#include <asm/io.h>
#include "biosemui.h"

/*------------------------- Global Variables ------------------------------*/

#ifndef CONFIG_X86EMU_RAW_IO
static char *BE_biosDate = "08/14/99";
static u8 BE_model = 0xFC;
static u8 BE_submodel = 0x00;
#endif

#undef DEBUG_IO_ACCESS

#ifdef DEBUG_IO_ACCESS
#define debug_io(fmt, ...)	printf(fmt, ##__VA_ARGS__)
#else
#define debug_io(x, b...)
#endif

/*----------------------------- Implementation ----------------------------*/

/****************************************************************************
PARAMETERS:
addr    - Emulator memory address to convert

RETURNS:
Actual memory address to read or write the data

REMARKS:
This function converts an emulator memory address in a 32-bit range to
a real memory address that we wish to access. It handles splitting up the
memory address space appropriately to access the emulator BIOS image, video
memory and system BIOS etc.
****************************************************************************/
static u8 *BE_memaddr(u32 addr)
{
	if (addr >= 0xC0000 && addr <= _BE_env.biosmem_limit) {
		return (u8*)(_BE_env.biosmem_base + addr - 0xC0000);
	} else if (addr > _BE_env.biosmem_limit && addr < 0xD0000) {
		DB(printf("BE_memaddr: address %#lx may be invalid!\n",
			  (ulong)addr);)
		return (u8 *)M.mem_base;
	} else if (addr >= 0xA0000 && addr <= 0xBFFFF) {
		return (u8*)(_BE_env.busmem_base + addr - 0xA0000);
	}
#ifdef CONFIG_X86EMU_RAW_IO
	else if (addr >= 0xD0000 && addr <= 0xFFFFF) {
		/* We map the real System BIOS directly on real PC's */
		DB(printf("BE_memaddr: System BIOS address %#lx\n",
			  (ulong)addr);)
		    return (u8 *)_BE_env.busmem_base + addr - 0xA0000;
	}
#else
	else if (addr >= 0xFFFF5 && addr < 0xFFFFE) {
		/* Return a faked BIOS date string for non-x86 machines */
		debug_io("BE_memaddr - Returning BIOS date\n");
		return (u8 *)(BE_biosDate + addr - 0xFFFF5);
	} else if (addr == 0xFFFFE) {
		/* Return system model identifier for non-x86 machines */
		debug_io("BE_memaddr - Returning model\n");
		return &BE_model;
	} else if (addr == 0xFFFFF) {
		/* Return system submodel identifier for non-x86 machines */
		debug_io("BE_memaddr - Returning submodel\n");
		return &BE_submodel;
	}
#endif
	else if (addr > M.mem_size - 1) {
		HALT_SYS();
		return (u8 *)M.mem_base;
	}

	return (u8 *)(M.mem_base + addr);
}

/****************************************************************************
PARAMETERS:
addr    - Emulator memory address to read

RETURNS:
Byte value read from emulator memory.

REMARKS:
Reads a byte value from the emulator memory. We have three distinct memory
regions that are handled differently, which this function handles.
****************************************************************************/
u8 X86API BE_rdb(u32 addr)
{
	if (_BE_env.emulateVGA && addr >= 0xA0000 && addr <= 0xBFFFF)
		return 0;
	else {
		u8 val = readb_le(BE_memaddr(addr));
		return val;
	}
}

/****************************************************************************
PARAMETERS:
addr    - Emulator memory address to read

RETURNS:
Word value read from emulator memory.

REMARKS:
Reads a word value from the emulator memory. We have three distinct memory
regions that are handled differently, which this function handles.
****************************************************************************/
u16 X86API BE_rdw(u32 addr)
{
	if (_BE_env.emulateVGA && addr >= 0xA0000 && addr <= 0xBFFFF)
		return 0;
	else {
		u8 *base = BE_memaddr(addr);
		u16 val = readw_le(base);
		return val;
	}
}

/****************************************************************************
PARAMETERS:
addr    - Emulator memory address to read

RETURNS:
Long value read from emulator memory.

REMARKS:
Reads a 32-bit value from the emulator memory. We have three distinct memory
regions that are handled differently, which this function handles.
****************************************************************************/
u32 X86API BE_rdl(u32 addr)
{
	if (_BE_env.emulateVGA && addr >= 0xA0000 && addr <= 0xBFFFF)
		return 0;
	else {
		u8 *base = BE_memaddr(addr);
		u32 val = readl_le(base);
		return val;
	}
}

/****************************************************************************
PARAMETERS:
addr    - Emulator memory address to read
val     - Value to store

REMARKS:
Writes a byte value to emulator memory. We have three distinct memory
regions that are handled differently, which this function handles.
****************************************************************************/
void X86API BE_wrb(u32 addr, u8 val)
{
	if (!(_BE_env.emulateVGA && addr >= 0xA0000 && addr <= 0xBFFFF)) {
		writeb_le(BE_memaddr(addr), val);
	}
}

/****************************************************************************
PARAMETERS:
addr    - Emulator memory address to read
val     - Value to store

REMARKS:
Writes a word value to emulator memory. We have three distinct memory
regions that are handled differently, which this function handles.
****************************************************************************/
void X86API BE_wrw(u32 addr, u16 val)
{
	if (!(_BE_env.emulateVGA && addr >= 0xA0000 && addr <= 0xBFFFF)) {
		u8 *base = BE_memaddr(addr);
		writew_le(base, val);

	}
}

/****************************************************************************
PARAMETERS:
addr    - Emulator memory address to read
val     - Value to store

REMARKS:
Writes a 32-bit value to emulator memory. We have three distinct memory
regions that are handled differently, which this function handles.
****************************************************************************/
void X86API BE_wrl(u32 addr, u32 val)
{
	if (!(_BE_env.emulateVGA && addr >= 0xA0000 && addr <= 0xBFFFF)) {
		u8 *base = BE_memaddr(addr);
		writel_le(base, val);
	}
}

#if !defined(CONFIG_X86EMU_RAW_IO)

/* For Non-Intel machines we may need to emulate some I/O port accesses that
 * the BIOS may try to access, such as the PCI config registers.
 */

#define IS_TIMER_PORT(port) (0x40 <= port && port <= 0x43)
#define IS_CMOS_PORT(port)  (0x70 <= port && port <= 0x71)
/*#define IS_VGA_PORT(port)   (_BE_env.emulateVGA && 0x3C0 <= port && port <= 0x3DA)*/
#define IS_VGA_PORT(port)   (0x3C0 <= port && port <= 0x3DA)
#define IS_PCI_PORT(port)   (0xCF8 <= port && port <= 0xCFF)
#define IS_SPKR_PORT(port)  (port == 0x61)

/****************************************************************************
PARAMETERS:
port    - Port to read from
type    - Type of access to perform

REMARKS:
Performs an emulated read from the Standard VGA I/O ports. If the target
hardware does not support mapping the VGA I/O and memory (such as some
PowerPC systems), we emulate the VGA so that the BIOS will still be able to
set NonVGA display modes such as on ATI hardware.
****************************************************************************/
static u8 VGA_inpb (const int port)
{
	u8 val = 0xff;

	debug_io("vga_inb.%04X -> ", (u16) port);
	switch (port) {
	case 0x3C0:
		/* 3C0 has funky characteristics because it can act as either
		   a data register or index register depending on the state
		   of an internal flip flop in the hardware. Hence we have
		   to emulate that functionality in here. */
		if (_BE_env.flipFlop3C0 == 0) {
			/* Access 3C0 as index register */
			val = _BE_env.emu3C0;
		} else {
			/* Access 3C0 as data register */
			if (_BE_env.emu3C0 < ATT_C)
				val = _BE_env.emu3C1[_BE_env.emu3C0];
		}
		_BE_env.flipFlop3C0 ^= 1;
		break;
	case 0x3C1:
		if (_BE_env.emu3C0 < ATT_C)
			return _BE_env.emu3C1[_BE_env.emu3C0];
		break;
	case 0x3CC:
		return _BE_env.emu3C2;
	case 0x3C4:
		return _BE_env.emu3C4;
	case 0x3C5:
		if (_BE_env.emu3C4 < ATT_C)
			return _BE_env.emu3C5[_BE_env.emu3C4];
		break;
	case 0x3C6:
		return _BE_env.emu3C6;
	case 0x3C7:
		return _BE_env.emu3C7;
	case 0x3C8:
		return _BE_env.emu3C8;
	case 0x3C9:
		if (_BE_env.emu3C7 < PAL_C)
			return _BE_env.emu3C9[_BE_env.emu3C7++];
		break;
	case 0x3CE:
		return _BE_env.emu3CE;
	case 0x3CF:
		if (_BE_env.emu3CE < GRA_C)
			return _BE_env.emu3CF[_BE_env.emu3CE];
		break;
	case 0x3D4:
		if (_BE_env.emu3C2 & 0x1)
			return _BE_env.emu3D4;
		break;
	case 0x3D5:
		if ((_BE_env.emu3C2 & 0x1) && (_BE_env.emu3D4 < CRT_C))
			return _BE_env.emu3D5[_BE_env.emu3D4];
		break;
	case 0x3DA:
		_BE_env.flipFlop3C0 = 0;
		val = _BE_env.emu3DA;
		_BE_env.emu3DA ^= 0x9;
		break;
	}
	return val;
}

/****************************************************************************
PARAMETERS:
port    - Port to write to
type    - Type of access to perform

REMARKS:
Performs an emulated write to one of the 8253 timer registers. For now
we only emulate timer 0 which is the only timer that the BIOS code appears
to use.
****************************************************************************/
static void VGA_outpb (int port, u8 val)
{
	switch (port) {
	case 0x3C0:
		/* 3C0 has funky characteristics because it can act as either
		   a data register or index register depending on the state
		   of an internal flip flop in the hardware. Hence we have
		   to emulate that functionality in here. */
		if (_BE_env.flipFlop3C0 == 0) {
			/* Access 3C0 as index register */
			_BE_env.emu3C0 = val;
		} else {
			/* Access 3C0 as data register */
			if (_BE_env.emu3C0 < ATT_C)
				_BE_env.emu3C1[_BE_env.emu3C0] = val;
		}
		_BE_env.flipFlop3C0 ^= 1;
		break;
	case 0x3C2:
		_BE_env.emu3C2 = val;
		break;
	case 0x3C4:
		_BE_env.emu3C4 = val;
		break;
	case 0x3C5:
		if (_BE_env.emu3C4 < ATT_C)
			_BE_env.emu3C5[_BE_env.emu3C4] = val;
		break;
	case 0x3C6:
		_BE_env.emu3C6 = val;
		break;
	case 0x3C7:
		_BE_env.emu3C7 = (int) val *3;

		break;
	case 0x3C8:
		_BE_env.emu3C8 = (int) val *3;

		break;
	case 0x3C9:
		if (_BE_env.emu3C8 < PAL_C)
			_BE_env.emu3C9[_BE_env.emu3C8++] = val;
		break;
	case 0x3CE:
		_BE_env.emu3CE = val;
		break;
	case 0x3CF:
		if (_BE_env.emu3CE < GRA_C)
			_BE_env.emu3CF[_BE_env.emu3CE] = val;
		break;
	case 0x3D4:
		if (_BE_env.emu3C2 & 0x1)
			_BE_env.emu3D4 = val;
		break;
	case 0x3D5:
		if ((_BE_env.emu3C2 & 0x1) && (_BE_env.emu3D4 < CRT_C))
			_BE_env.emu3D5[_BE_env.emu3D4] = val;
		break;
	}
}

/****************************************************************************
PARAMETERS:
regOffset   - Offset into register space for non-DWORD accesses
value       - Value to write to register for PCI_WRITE_* operations
func        - Function to perform (PCIAccessRegFlags)

RETURNS:
Value read from configuration register for PCI_READ_* operations

REMARKS:
Accesses a PCI configuration space register by decoding the value currently
stored in the _BE_env.configAddress variable and passing it through to the
portable PCI_accessReg function.
****************************************************************************/
static u32 BE_accessReg(int regOffset, u32 value, int func)
{
#ifdef __KERNEL__
	int function, device, bus;
	u8 val8;
	u16 val16;
	u32 val32;


	/* Decode the configuration register values for the register we wish to
	 * access
	 */
	regOffset += (_BE_env.configAddress & 0xFF);
	function = (_BE_env.configAddress >> 8) & 0x7;
	device = (_BE_env.configAddress >> 11) & 0x1F;
	bus = (_BE_env.configAddress >> 16) & 0xFF;

	/* Ignore accesses to all devices other than the one we're POSTing */
	if ((function == _BE_env.vgaInfo.function) &&
	    (device == _BE_env.vgaInfo.device) &&
	    (bus == _BE_env.vgaInfo.bus)) {
		switch (func) {
		case REG_READ_BYTE:
			pci_read_config_byte(_BE_env.vgaInfo.pcidev, regOffset,
					     &val8);
			return val8;
		case REG_READ_WORD:
			pci_read_config_word(_BE_env.vgaInfo.pcidev, regOffset,
					     &val16);
			return val16;
		case REG_READ_DWORD:
			pci_read_config_dword(_BE_env.vgaInfo.pcidev, regOffset,
					      &val32);
			return val32;
		case REG_WRITE_BYTE:
			pci_write_config_byte(_BE_env.vgaInfo.pcidev, regOffset,
					      value);

			return 0;
		case REG_WRITE_WORD:
			pci_write_config_word(_BE_env.vgaInfo.pcidev, regOffset,
					      value);

			return 0;
		case REG_WRITE_DWORD:
			pci_write_config_dword(_BE_env.vgaInfo.pcidev,
					       regOffset, value);

			return 0;
		}
	}
	return 0;
#else
	PCIDeviceInfo pciInfo;

	pciInfo.mech1 = 1;
	pciInfo.slot.i = 0;
	pciInfo.slot.p.Function = (_BE_env.configAddress >> 8) & 0x7;
	pciInfo.slot.p.Device = (_BE_env.configAddress >> 11) & 0x1F;
	pciInfo.slot.p.Bus = (_BE_env.configAddress >> 16) & 0xFF;
	pciInfo.slot.p.Enable = 1;

	/* Ignore accesses to all devices other than the one we're POSTing */
	if ((pciInfo.slot.p.Function ==
	     _BE_env.vgaInfo.pciInfo->slot.p.Function)
	    && (pciInfo.slot.p.Device == _BE_env.vgaInfo.pciInfo->slot.p.Device)
	    && (pciInfo.slot.p.Bus == _BE_env.vgaInfo.pciInfo->slot.p.Bus))
		return PCI_accessReg((_BE_env.configAddress & 0xFF) + regOffset,
				     value, func, &pciInfo);
	return 0;
#endif
}

/****************************************************************************
PARAMETERS:
port    - Port to read from
type    - Type of access to perform

REMARKS:
Performs an emulated read from one of the PCI configuration space registers.
We emulate this using our PCI_accessReg function which will access the PCI
configuration space registers in a portable fashion.
****************************************************************************/
static u32 PCI_inp(int port, int type)
{
	switch (type) {
	case REG_READ_BYTE:
		if ((_BE_env.configAddress & 0x80000000) && 0xCFC <= port
		    && port <= 0xCFF)
			return BE_accessReg(port - 0xCFC, 0, REG_READ_BYTE);
		break;
	case REG_READ_WORD:
		if ((_BE_env.configAddress & 0x80000000) && 0xCFC <= port
		    && port <= 0xCFF)
			return BE_accessReg(port - 0xCFC, 0, REG_READ_WORD);
		break;
	case REG_READ_DWORD:
		if (port == 0xCF8)
			return _BE_env.configAddress;
		else if ((_BE_env.configAddress & 0x80000000) && port == 0xCFC)
			return BE_accessReg(0, 0, REG_READ_DWORD);
		break;
	}
	return 0;
}

/****************************************************************************
PARAMETERS:
port    - Port to write to
type    - Type of access to perform

REMARKS:
Performs an emulated write to one of the PCI control registers.
****************************************************************************/
static void PCI_outp(int port, u32 val, int type)
{
	switch (type) {
	case REG_WRITE_BYTE:
		if ((_BE_env.configAddress & 0x80000000) && 0xCFC <= port
		    && port <= 0xCFF)
			BE_accessReg(port - 0xCFC, val, REG_WRITE_BYTE);
		break;
	case REG_WRITE_WORD:
		if ((_BE_env.configAddress & 0x80000000) && 0xCFC <= port
		    && port <= 0xCFF)
			BE_accessReg(port - 0xCFC, val, REG_WRITE_WORD);
		break;
	case REG_WRITE_DWORD:
		if (port == 0xCF8)
		{
			_BE_env.configAddress = val & 0x80FFFFFC;
		}
		else if ((_BE_env.configAddress & 0x80000000) && port == 0xCFC)
			BE_accessReg(0, val, REG_WRITE_DWORD);
		break;
	}
}

#endif

/****************************************************************************
PARAMETERS:
port    - Port to write to

RETURNS:
Value read from the I/O port

REMARKS:
Performs an emulated 8-bit read from an I/O port. We handle special cases
that we need to emulate in here, and fall through to reflecting the write
through to the real hardware if we don't need to special case it.
****************************************************************************/
u8 X86API BE_inb(X86EMU_pioAddr port)
{
	u8 val = 0;

#if !defined(CONFIG_X86EMU_RAW_IO)
	if (IS_VGA_PORT(port)){
		/*seems reading port 0x3c3 return the high 16 bit of io port*/
		if(port == 0x3c3)
			val = LOG_inpb(port);
		else
			val = VGA_inpb(port);
	}
	else if (IS_TIMER_PORT(port))
		DB(printf("Can not interept TIMER port now!\n");)
	else if (IS_SPKR_PORT(port))
		DB(printf("Can not interept SPEAKER port now!\n");)
	else if (IS_CMOS_PORT(port))
		DB(printf("Can not interept CMOS port now!\n");)
	else if (IS_PCI_PORT(port))
		val = PCI_inp(port, REG_READ_BYTE);
	else if (port < 0x100) {
		DB(printf("WARN: INVALID inb.%04X -> %02X\n", (u16) port, val);)
		val = LOG_inpb(port);
	} else
#endif
	{
		debug_io("inb.%04X -> ", (u16) port);
		val = LOG_inpb(port);
		debug_io("%02X\n", val);
	}

	return val;
}

/****************************************************************************
PARAMETERS:
port    - Port to write to

RETURNS:
Value read from the I/O port

REMARKS:
Performs an emulated 16-bit read from an I/O port. We handle special cases
that we need to emulate in here, and fall through to reflecting the write
through to the real hardware if we don't need to special case it.
****************************************************************************/
u16 X86API BE_inw(X86EMU_pioAddr port)
{
	u16 val = 0;

#if !defined(CONFIG_X86EMU_RAW_IO)
	if (IS_PCI_PORT(port))
		val = PCI_inp(port, REG_READ_WORD);
	else if (port < 0x100) {
		DB(printf("WARN: Maybe INVALID inw.%04X -> %04X\n", (u16) port, val);)
		val = LOG_inpw(port);
	} else
#endif
	{
		debug_io("inw.%04X -> ", (u16) port);
		val = LOG_inpw(port);
		debug_io("%04X\n", val);
	}

	return val;
}

/****************************************************************************
PARAMETERS:
port    - Port to write to

RETURNS:
Value read from the I/O port

REMARKS:
Performs an emulated 32-bit read from an I/O port. We handle special cases
that we need to emulate in here, and fall through to reflecting the write
through to the real hardware if we don't need to special case it.
****************************************************************************/
u32 X86API BE_inl(X86EMU_pioAddr port)
{
	u32 val = 0;

#if !defined(CONFIG_X86EMU_RAW_IO)
	if (IS_PCI_PORT(port))
		val = PCI_inp(port, REG_READ_DWORD);
	else if (port < 0x100) {
		val = LOG_inpd(port);
	} else
#endif
	{
		debug_io("inl.%04X -> ", (u16) port);
		val = LOG_inpd(port);
		debug_io("%08X\n", val);
	}

	return val;
}

/****************************************************************************
PARAMETERS:
port    - Port to write to
val     - Value to write to port

REMARKS:
Performs an emulated 8-bit write to an I/O port. We handle special cases
that we need to emulate in here, and fall through to reflecting the write
through to the real hardware if we don't need to special case it.
****************************************************************************/
void X86API BE_outb(X86EMU_pioAddr port, u8 val)
{
#if !defined(CONFIG_X86EMU_RAW_IO)
	if (IS_VGA_PORT(port))
		VGA_outpb(port, val);
	else if (IS_TIMER_PORT(port))
		DB(printf("Can not interept TIMER port now!\n");)
	else if (IS_SPKR_PORT(port))
		DB(printf("Can not interept SPEAKER port now!\n");)
	else if (IS_CMOS_PORT(port))
		DB(printf("Can not interept CMOS port now!\n");)
	else if (IS_PCI_PORT(port))
		PCI_outp(port, val, REG_WRITE_BYTE);
	else if (port < 0x100) {
		DB(printf("WARN:Maybe INVALID outb.%04X <- %02X\n", (u16) port, val);)
		LOG_outpb(port, val);
	} else
#endif
	{
		debug_io("outb.%04X <- %02X", (u16) port, val);
		LOG_outpb(port, val);
		debug_io("\n");
	}
}

/****************************************************************************
PARAMETERS:
port    - Port to write to
val     - Value to write to port

REMARKS:
Performs an emulated 16-bit write to an I/O port. We handle special cases
that we need to emulate in here, and fall through to reflecting the write
through to the real hardware if we don't need to special case it.
****************************************************************************/
void X86API BE_outw(X86EMU_pioAddr port, u16 val)
{
#if !defined(CONFIG_X86EMU_RAW_IO)
	if (IS_VGA_PORT(port)) {
		VGA_outpb(port, val);
		VGA_outpb(port + 1, val >> 8);
	} else if (IS_PCI_PORT(port)) {
		PCI_outp(port, val, REG_WRITE_WORD);
	} else if (port < 0x100) {
		DB(printf("WARN: MAybe INVALID outw.%04X <- %04X\n", (u16)port,
			  val);)
		LOG_outpw(port, val);
	} else
#endif
	{
		debug_io("outw.%04X <- %04X", (u16) port, val);
		LOG_outpw(port, val);
		debug_io("\n");
	}
}

/****************************************************************************
PARAMETERS:
port    - Port to write to
val     - Value to write to port

REMARKS:
Performs an emulated 32-bit write to an I/O port. We handle special cases
that we need to emulate in here, and fall through to reflecting the write
through to the real hardware if we don't need to special case it.
****************************************************************************/
void X86API BE_outl(X86EMU_pioAddr port, u32 val)
{
#if !defined(CONFIG_X86EMU_RAW_IO)
	if (IS_PCI_PORT(port)) {
		PCI_outp(port, val, REG_WRITE_DWORD);
	} else if (port < 0x100) {
		DB(printf("WARN: INVALID outl.%04X <- %08X\n", (u16) port,val);)
		LOG_outpd(port, val);
	} else
#endif
	{
		debug_io("outl.%04X <- %08X", (u16) port, val);
		LOG_outpd(port, val);
		debug_io("\n");
	}
}
