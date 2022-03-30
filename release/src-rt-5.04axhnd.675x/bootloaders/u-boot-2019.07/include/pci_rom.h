/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * From coreboot file of same name
 */

#ifndef _PCI_ROM_H
#define _PCI_ROM_H

#define PCI_ROM_HDR			0xaa55

struct pci_rom_header {
	uint16_t signature;
	uint8_t size;
	uint8_t init[3];
	uint8_t reserved[0x12];
	uint16_t data;
};

struct pci_rom_data {
	uint32_t signature;
	uint16_t vendor;
	uint16_t device;
	uint16_t reserved_1;
	uint16_t dlen;
	uint8_t drevision;
	uint8_t class_lo;
	uint16_t class_hi;
	uint16_t ilen;
	uint16_t irevision;
	uint8_t type;
	uint8_t indicator;
	uint16_t reserved_2;
};

/*
 * Determines which execution method is used and whether we allow falling back
 * to the other if the requested method is not available.
 */
enum pci_rom_emul {
	PCI_ROM_EMULATE		= 0 << 0,
	PCI_ROM_USE_NATIVE	= 1 << 0,
	PCI_ROM_ALLOW_FALLBACK	= 1 << 1,
};

 /**
 * dm_pci_run_vga_bios() - Run the VGA BIOS in an x86 PC
 *
 * @dev:	Video device containing the BIOS
 * @int15_handler:	Function to call to handle int 0x15
 * @exec_method:	flags from enum pci_rom_emul
 */
int dm_pci_run_vga_bios(struct udevice *dev, int (*int15_handler)(void),
			int exec_method);

/**
 * board_map_oprom_vendev() - map several PCI IDs to the one the ROM expects
 *
 * Some VGA option roms are used for several chipsets but they only have one
 * PCI ID in their header. If we encounter such an option rom, we need to do
 * the mapping ourselves.
 *
 * @vendev:	Vendor and device for the video device
 * @return standard vendor and device expected by the ROM
 */
uint32_t board_map_oprom_vendev(uint32_t vendev);

#endif
