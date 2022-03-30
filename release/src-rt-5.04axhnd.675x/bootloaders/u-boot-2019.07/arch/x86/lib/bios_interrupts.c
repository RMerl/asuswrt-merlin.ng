// SPDX-License-Identifier: GPL-2.0
/*
 * From Coreboot
 *
 * Copyright (C) 2001 Ronald G. Minnich
 * Copyright (C) 2005 Nick.Barker9@btinternet.com
 * Copyright (C) 2007-2009 coresystems GmbH
 */

#include <common.h>
#include <asm/pci.h>
#include "bios_emul.h"

/* errors go in AH. Just set these up so that word assigns will work */
enum {
	PCIBIOS_SUCCESSFUL = 0x0000,
	PCIBIOS_UNSUPPORTED = 0x8100,
	PCIBIOS_BADVENDOR = 0x8300,
	PCIBIOS_NODEV = 0x8600,
	PCIBIOS_BADREG = 0x8700
};

int int10_handler(void)
{
	static u8 cursor_row, cursor_col;
	int res = 0;

	switch ((M.x86.R_EAX & 0xff00) >> 8) {
	case 0x01: /* Set cursor shape */
		res = 1;
		break;
	case 0x02: /* Set cursor position */
		if (cursor_row != ((M.x86.R_EDX >> 8) & 0xff) ||
		    cursor_col >= (M.x86.R_EDX & 0xff)) {
			debug("\n");
		}
		cursor_row = (M.x86.R_EDX >> 8) & 0xff;
		cursor_col = M.x86.R_EDX & 0xff;
		res = 1;
		break;
	case 0x03: /* Get cursor position */
		M.x86.R_EAX &= 0x00ff;
		M.x86.R_ECX = 0x0607;
		M.x86.R_EDX = (cursor_row << 8) | cursor_col;
		res = 1;
		break;
	case 0x06: /* Scroll up */
		debug("\n");
		res = 1;
		break;
	case 0x08: /* Get Character and Mode at Cursor Position */
		M.x86.R_EAX = 0x0f00 | 'A'; /* White on black 'A' */
		res = 1;
		break;
	case 0x09: /* Write Character and attribute */
	case 0x0e: /* Write Character */
		debug("%c", M.x86.R_EAX & 0xff);
		res = 1;
		break;
	case 0x0f: /* Get video mode */
		M.x86.R_EAX = 0x5002; /*80 x 25 */
		M.x86.R_EBX &= 0x00ff;
		res = 1;
		break;
	default:
		printf("Unknown INT10 function %04x\n", M.x86.R_EAX & 0xffff);
		break;
	}
	return res;
}

int int12_handler(void)
{
	M.x86.R_EAX = 64 * 1024;
	return 1;
}

int int16_handler(void)
{
	int res = 0;

	switch ((M.x86.R_EAX & 0xff00) >> 8) {
	case 0x00: /* Check for Keystroke */
		M.x86.R_EAX = 0x6120; /* Space Bar, Space */
		res = 1;
		break;
	case 0x01: /* Check for Keystroke */
		M.x86.R_EFLG |= 1 << 6; /* Zero Flag set (no key available) */
		res = 1;
		break;
	default:
		printf("Unknown INT16 function %04x\n", M.x86.R_EAX & 0xffff);

break;
	}
	return res;
}

#define PCI_CONFIG_SPACE_TYPE1	(1 << 0)
#define PCI_SPECIAL_CYCLE_TYPE1	(1 << 4)

int int1a_handler(void)
{
	unsigned short func = (unsigned short)M.x86.R_EAX;
	int retval = 1;
	unsigned short devid, vendorid, devfn;
	struct udevice *dev;
	/* Use short to get rid of gabage in upper half of 32-bit register */
	short devindex;
	unsigned char bus;
	pci_dev_t bdf;
	u32 dword;
	u16 word;
	u8 byte, reg;
	int ret;

	switch (func) {
	case 0xb101: /* PCIBIOS Check */
		M.x86.R_EDX = 0x20494350;	/* ' ICP' */
		M.x86.R_EAX &= 0xffff0000; /* Clear AH / AL */
		M.x86.R_EAX |= PCI_CONFIG_SPACE_TYPE1 |
				PCI_SPECIAL_CYCLE_TYPE1;
		/*
		 * last bus in the system. Hard code to 255 for now.
		 * dev_enumerate() does not seem to tell us (publically)
		 */
		M.x86.R_ECX = 0xff;
		M.x86.R_EDI = 0x00000000;	/* protected mode entry */
		retval = 1;
		break;
	case 0xb102: /* Find Device */
		devid = M.x86.R_ECX;
		vendorid = M.x86.R_EDX;
		devindex = M.x86.R_ESI;
		bdf = -1;
		ret = dm_pci_find_device(vendorid, devid, devindex, &dev);
		if (!ret) {
			unsigned short busdevfn;

			bdf = dm_pci_get_bdf(dev);
			M.x86.R_EAX &= 0xffff00ff; /* Clear AH */
			M.x86.R_EAX |= PCIBIOS_SUCCESSFUL;
			/*
			 * busnum is an unsigned char;
			 * devfn is an int, so we mask it off.
			 */
			busdevfn = (PCI_BUS(bdf) << 8) | PCI_DEV(bdf) << 3 |
				PCI_FUNC(bdf);
			debug("0x%x: return 0x%x\n", func, busdevfn);
			M.x86.R_EBX = busdevfn;
			retval = 1;
		} else {
			M.x86.R_EAX &= 0xffff00ff; /* Clear AH */
			M.x86.R_EAX |= PCIBIOS_NODEV;
			retval = 0;
		}
		break;
	case 0xb10a: /* Read Config Dword */
	case 0xb109: /* Read Config Word */
	case 0xb108: /* Read Config Byte */
	case 0xb10d: /* Write Config Dword */
	case 0xb10c: /* Write Config Word */
	case 0xb10b: /* Write Config Byte */
		devfn = M.x86.R_EBX & 0xff;
		bus = M.x86.R_EBX >> 8;
		reg = M.x86.R_EDI;
		bdf = PCI_BDF(bus, devfn >> 3, devfn & 7);

		ret = dm_pci_bus_find_bdf(bdf, &dev);
		if (ret) {
			debug("%s: Device %x not found\n", __func__, bdf);
			break;
		}

		switch (func) {
		case 0xb108: /* Read Config Byte */
			dm_pci_read_config8(dev, reg, &byte);
			M.x86.R_ECX = byte;
			break;
		case 0xb109: /* Read Config Word */
			dm_pci_read_config16(dev, reg, &word);
			M.x86.R_ECX = word;
			break;
		case 0xb10a: /* Read Config Dword */
			dm_pci_read_config32(dev, reg, &dword);
			M.x86.R_ECX = dword;
			break;
		case 0xb10b: /* Write Config Byte */
			byte = M.x86.R_ECX;
			dm_pci_write_config8(dev, reg, byte);
			break;
		case 0xb10c: /* Write Config Word */
			word = M.x86.R_ECX;
			dm_pci_write_config16(dev, reg, word);
			break;
		case 0xb10d: /* Write Config Dword */
			dword = M.x86.R_ECX;
			dm_pci_write_config32(dev, reg, dword);
			break;
		}
#ifdef CONFIG_REALMODE_DEBUG
		debug("0x%x: bus %d devfn 0x%x reg 0x%x val 0x%x\n", func,
		      bus, devfn, reg, M.x86.R_ECX);
#endif
		M.x86.R_EAX &= 0xffff00ff; /* Clear AH */
		M.x86.R_EAX |= PCIBIOS_SUCCESSFUL;
		retval = 1;
		break;
	default:
		printf("UNSUPPORTED PCIBIOS FUNCTION 0x%x\n", func);
		M.x86.R_EAX &= 0xffff00ff; /* Clear AH */
		M.x86.R_EAX |= PCIBIOS_UNSUPPORTED;
		retval = 0;
		break;
	}

	return retval;
}
