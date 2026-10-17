/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Renesas RZ/N1 Package Table format
 * (C) 2015-2016 Renesas Electronics Europe, LTD
 * All rights reserved.
 *
 * Converted to mkimage plug-in
 * (C) Copyright 2022 Schneider Electric
 */

#ifndef _SPKGIMAGE_H_
#define _SPKGIMAGE_H_

#ifdef __GNUC__
#define __packed __attribute((packed))
#else
#define __packed
#endif

#define SPKG_HEADER_MARKER	{'R', 'Z', 'N', '1'}
#define SPKG_HEADER_SIZE	24
#define SPKG_HEADER_COUNT	8
#define SPKG_BLP_SIZE		264
#define SPKG_CRC_SIZE		4

/**
 * struct spkg_hdr - SPKG header
 * @marker:           magic pattern "RZN1"
 * @version:          header version (currently 1)
 * @ecc:              ECC enable and block size.
 * @ecc_scheme:       ECC algorithm selction
 * @ecc_bytes:        ECC bytes per block
 * @payload_length:   length of the payload (including CRC)
 * @load_address:     address in memory where payload should be loaded
 * @execution_offset: offset from @load_address where execution starts
 * @crc:              32-bit CRC of the above header fields
 *
 * SPKG header format is defined by Renesas. It is documented in the Reneasas
 * RZ/N1 User Manual, Chapter 7.4 ("SPKG format").
 *
 * The BootROM searches this header in order to find and validate the boot
 * payload. It is therefore mandatory to wrap the payload in this header.
 *
 * The ECC-related fields @ecc @ecc_scheme @ecc_bytes are used only when
 * booting from NAND flash, and they are only used while fetching the payload.
 * These values are used to initialize the ECC controller. To avoid using
 * non-portable bitfields, struct spkg_hdr uses uint8_t for these fields, so
 * the user must shift the values into the correct spot.
 *
 * The payload will be loaded into memory at @payload_address.
 * Execution then jumps to @payload_address + @execution_offset.
 * The LSB of @execution_offset selects between ARM and Thumb mode,
 * as per the usual ARM interworking convention.
 */
struct spkg_hdr {
	uint8_t		marker[4];	/* aka magic */
	uint8_t		version;
	uint8_t		ecc;
	uint8_t		ecc_scheme;
	uint8_t		ecc_bytes;
	uint32_t	payload_length; /* only HIGHER 24 bits */
	uint32_t	load_address;
	uint32_t	execution_offset;
	uint32_t	crc; /* of this header */
} __packed;

/**
 * struct spkg_file - complete SPKG image
 *
 * A SPKG image consists of 8 identical copies of struct spkg_hdr, each one
 * occupying 24 bytes, for a total of 192 bytes.
 *
 * This is followed by the payload (the u-boot binary), and a 32-bit CRC.
 *
 * Optionally, the payload can be being with security header ("BLp_header").
 * This feature is not currently supported in mkimage.
 *
 * The payload is typically padded with 0xFF bytes so as to bring the total
 * image size to a multiple of the flash erase size (often 64kB).
 */
struct spkg_file {
	struct spkg_hdr	header[SPKG_HEADER_COUNT];
	uint8_t		payload[0];
	/* then the CRC */
} __packed;

#endif
