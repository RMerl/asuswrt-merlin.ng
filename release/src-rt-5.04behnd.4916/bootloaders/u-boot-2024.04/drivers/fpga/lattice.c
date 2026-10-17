// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2010
 * Stefano Babic, DENX Software Engineering, sbabic@denx.de.
 *
 * (C) Copyright 2002
 * Rich Ireland, Enterasys Networks, rireland@enterasys.com.
 *
 * ispVM functions adapted from Lattice's ispmVMEmbedded code:
 * Copyright 2009 Lattice Semiconductor Corp.
 */

#include <common.h>
#include <log.h>
#include <malloc.h>
#include <fpga.h>
#include <lattice.h>
#include <linux/delay.h>

static lattice_board_specific_func *pfns;
static const char *fpga_image;
static unsigned long read_bytes;
static unsigned long bufsize;
static unsigned short expectedCRC;

/*
 * External variables and functions declared in ivm_core.c module.
 */
extern unsigned short g_usCalculatedCRC;
extern unsigned short g_usDataType;
extern unsigned char *g_pucIntelBuffer;
extern unsigned char *g_pucHeapMemory;
extern unsigned short g_iHeapCounter;
extern unsigned short g_iHEAPSize;
extern unsigned short g_usIntelDataIndex;
extern unsigned short g_usIntelBufferSize;
extern char *const g_szSupportedVersions[];


/*
 * ispVMDelay
 *
 * Users must implement a delay to observe a_usTimeDelay, where
 * bit 15 of the a_usTimeDelay defines the unit.
 *      1 = milliseconds
 *      0 = microseconds
 * Example:
 *      a_usTimeDelay = 0x0001 = 1 microsecond delay.
 *      a_usTimeDelay = 0x8001 = 1 millisecond delay.
 *
 * This subroutine is called upon to provide a delay from 1 millisecond to a few
 * hundreds milliseconds each time.
 * It is understood that due to a_usTimeDelay is defined as unsigned short, a 16
 * bits integer, this function is restricted to produce a delay to 64000
 * micro-seconds or 32000 milli-second maximum. The VME file will never pass on
 * to this function a delay time > those maximum number. If it needs more than
 * those maximum, the VME file will launch the delay function several times to
 * realize a larger delay time cummulatively.
 * It is perfectly alright to provide a longer delay than required. It is not
 * acceptable if the delay is shorter.
 */
void ispVMDelay(unsigned short delay)
{
	if (delay & 0x8000)
		delay = (delay & ~0x8000) * 1000;
	udelay(delay);
}

void writePort(unsigned char a_ucPins, unsigned char a_ucValue)
{
	a_ucValue = a_ucValue ? 1 : 0;

	switch (a_ucPins) {
	case g_ucPinTDI:
		pfns->jtag_set_tdi(a_ucValue);
		break;
	case g_ucPinTCK:
		pfns->jtag_set_tck(a_ucValue);
		break;
	case g_ucPinTMS:
		pfns->jtag_set_tms(a_ucValue);
		break;
	default:
		printf("%s: requested unknown pin\n", __func__);
	}
}

unsigned char readPort(void)
{
	return pfns->jtag_get_tdo();
}

void sclock(void)
{
	writePort(g_ucPinTCK, 0x01);
	writePort(g_ucPinTCK, 0x00);
}

void calibration(void)
{
	/* Apply 2 pulses to TCK. */
	writePort(g_ucPinTCK, 0x00);
	writePort(g_ucPinTCK, 0x01);
	writePort(g_ucPinTCK, 0x00);
	writePort(g_ucPinTCK, 0x01);
	writePort(g_ucPinTCK, 0x00);

	ispVMDelay(0x8001);

	/* Apply 2 pulses to TCK. */
	writePort(g_ucPinTCK, 0x01);
	writePort(g_ucPinTCK, 0x00);
	writePort(g_ucPinTCK, 0x01);
	writePort(g_ucPinTCK, 0x00);
}

/*
 * GetByte
 *
 * Returns a byte to the caller. The returned byte depends on the
 * g_usDataType register. If the HEAP_IN bit is set, then the byte
 * is returned from the HEAP. If the LHEAP_IN bit is set, then
 * the byte is returned from the intelligent buffer. Otherwise,
 * the byte is returned directly from the VME file.
 */
unsigned char GetByte(void)
{
	unsigned char ucData;
	unsigned int block_size = 4 * 1024;

	if (g_usDataType & HEAP_IN) {

		/*
		 * Get data from repeat buffer.
		 */

		if (g_iHeapCounter > g_iHEAPSize) {

			/*
			 * Data over-run.
			 */

			return 0xFF;
		}

		ucData = g_pucHeapMemory[g_iHeapCounter++];
	} else if (g_usDataType & LHEAP_IN) {

		/*
		 * Get data from intel buffer.
		 */

		if (g_usIntelDataIndex >= g_usIntelBufferSize) {
			return 0xFF;
		}

		ucData = g_pucIntelBuffer[g_usIntelDataIndex++];
	} else {
		if (read_bytes == bufsize) {
			return 0xFF;
		}
		ucData = *fpga_image++;
		read_bytes++;

		if (!(read_bytes % block_size)) {
			printf("Downloading FPGA %ld/%ld completed\r",
				read_bytes,
				bufsize);
		}

		if (expectedCRC != 0) {
			ispVMCalculateCRC32(ucData);
		}
	}

	return ucData;
}

signed char ispVM(void)
{
	char szFileVersion[9]      = { 0 };
	signed char cRetCode         = 0;
	signed char cIndex           = 0;
	signed char cVersionIndex    = 0;
	unsigned char ucReadByte     = 0;
	unsigned short crc;

	g_pucHeapMemory		= NULL;
	g_iHeapCounter		= 0;
	g_iHEAPSize		= 0;
	g_usIntelDataIndex	= 0;
	g_usIntelBufferSize	= 0;
	g_usCalculatedCRC = 0;
	expectedCRC   = 0;
	ucReadByte = GetByte();
	switch (ucReadByte) {
	case FILE_CRC:
		crc = (unsigned char)GetByte();
		crc <<= 8;
		crc |= GetByte();
		expectedCRC = crc;

		for (cIndex = 0; cIndex < 8; cIndex++)
			szFileVersion[cIndex] = GetByte();

		break;
	default:
		szFileVersion[0] = (signed char) ucReadByte;
		for (cIndex = 1; cIndex < 8; cIndex++)
			szFileVersion[cIndex] = GetByte();

		break;
	}

	/*
	 *
	 * Compare the VME file version against the supported version.
	 *
	 */

	for (cVersionIndex = 0; g_szSupportedVersions[cVersionIndex] != 0;
		cVersionIndex++) {
		for (cIndex = 0; cIndex < 8; cIndex++) {
			if (szFileVersion[cIndex] !=
				g_szSupportedVersions[cVersionIndex][cIndex]) {
				cRetCode = VME_VERSION_FAILURE;
				break;
			}
			cRetCode = 0;
		}

		if (cRetCode == 0) {
			break;
		}
	}

	if (cRetCode < 0) {
		return VME_VERSION_FAILURE;
	}

	printf("VME file checked: starting downloading to FPGA\n");

	ispVMStart();

	cRetCode = ispVMCode();

	ispVMEnd();
	ispVMFreeMem();
	puts("\n");

	if (cRetCode == 0 && expectedCRC != 0 &&
			(expectedCRC != g_usCalculatedCRC)) {
		printf("Expected CRC:   0x%.4X\n", expectedCRC);
		printf("Calculated CRC: 0x%.4X\n", g_usCalculatedCRC);
		return VME_CRC_FAILURE;
	}
	return cRetCode;
}

static int lattice_validate(Lattice_desc *desc, const char *fn)
{
	int ret_val = false;

	if (desc) {
		if ((desc->family > min_lattice_type) &&
			(desc->family < max_lattice_type)) {
			if ((desc->iface > min_lattice_iface_type) &&
				(desc->iface < max_lattice_iface_type)) {
				if (desc->size) {
					ret_val = true;
				} else {
					printf("%s: NULL part size\n", fn);
				}
			} else {
				printf("%s: Invalid Interface type, %d\n",
					fn, desc->iface);
			}
		} else {
			printf("%s: Invalid family type, %d\n",
				fn, desc->family);
		}
	} else {
		printf("%s: NULL descriptor!\n", fn);
	}

	return ret_val;
}

int lattice_load(Lattice_desc *desc, const void *buf, size_t bsize)
{
	int ret_val = FPGA_FAIL;

	if (!lattice_validate(desc, (char *)__func__)) {
		printf("%s: Invalid device descriptor\n", __func__);
	} else {
		pfns = desc->iface_fns;

		switch (desc->family) {
		case Lattice_XP2:
			fpga_image = buf;
			read_bytes = 0;
			bufsize = bsize;
			debug("%s: Launching the Lattice ISPVME Loader:"
				" addr %p size 0x%lx...\n",
				__func__, fpga_image, bufsize);
			ret_val = ispVM();
			if (ret_val)
				printf("%s: error %d downloading FPGA image\n",
					__func__, ret_val);
			else
				puts("FPGA downloaded successfully\n");
			break;
		default:
			printf("%s: Unsupported family type, %d\n",
					__func__, desc->family);
		}
	}

	return ret_val;
}

int lattice_dump(Lattice_desc *desc, const void *buf, size_t bsize)
{
	puts("Dump not supported for Lattice FPGA\n");

	return FPGA_FAIL;

}

int lattice_info(Lattice_desc *desc)
{
	int ret_val = FPGA_FAIL;

	if (lattice_validate(desc, (char *)__func__)) {
		printf("Family:        \t");
		switch (desc->family) {
		case Lattice_XP2:
			puts("XP2\n");
			break;
			/* Add new family types here */
		default:
			printf("Unknown family type, %d\n", desc->family);
		}

		puts("Interface type:\t");
		switch (desc->iface) {
		case lattice_jtag_mode:
			puts("JTAG Mode\n");
			break;
			/* Add new interface types here */
		default:
			printf("Unsupported interface type, %d\n", desc->iface);
		}

		printf("Device Size:   \t%d bytes\n",
				desc->size);

		if (desc->iface_fns) {
			printf("Device Function Table @ 0x%p\n",
				desc->iface_fns);
			switch (desc->family) {
			case Lattice_XP2:
				break;
				/* Add new family types here */
			default:
				break;
			}
		} else {
			puts("No Device Function Table.\n");
		}

		if (desc->desc)
			printf("Model:         \t%s\n", desc->desc);

		ret_val = FPGA_SUCCESS;
	} else {
		printf("%s: Invalid device descriptor\n", __func__);
	}

	return ret_val;
}
