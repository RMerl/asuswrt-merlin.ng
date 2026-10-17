/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Structures used by the ChromiumOS bootmeth
 *
 * See docs at:
 * https://www.chromium.org/chromium-os/chromiumos-design-docs/verified-boot-data-structures/
 *
 * Original code at:
 * https://chromium.googlesource.com/chromiumos/platform/vboot_reference/+/refs/heads/main/firmware/2lib/include/2struct.h
 *
 * Code taken from vboot_reference commit 5b8596ce file 2struct.h
 *
 * Copyright 2023 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#ifndef __BOOTMETH_CROS_H
#define __BOOTMETH_CROS_H

/* Signature data (a secure hash, possibly signed) */
struct vb2_signature {
	/* Offset of signature data from start of this struct */
	uint32_t sig_offset;
	uint32_t reserved0;

	/* Size of signature data in bytes */
	uint32_t sig_size;
	uint32_t reserved1;

	/* Size of the data block which was signed in bytes */
	uint32_t data_size;
	uint32_t reserved2;
} __attribute__((packed));

#define EXPECTED_VB2_SIGNATURE_SIZE 24

/* Packed public key data */
struct vb2_packed_key {
	/* Offset of key data from start of this struct */
	uint32_t key_offset;
	uint32_t reserved0;

	/* Size of key data in bytes (NOT strength of key in bits) */
	uint32_t key_size;
	uint32_t reserved1;

	/* Signature algorithm used by the key (enum vb2_crypto_algorithm) */
	uint32_t algorithm;
	uint32_t reserved2;

	/* Key version */
	uint32_t key_version;
	uint32_t reserved3;

	/* TODO: when redoing this struct, add a text description of the key */
} __attribute__((packed));

#define EXPECTED_VB2_PACKED_KEY_SIZE 32

#define VB2_KEYBLOCK_MAGIC "CHROMEOS"
#define VB2_KEYBLOCK_MAGIC_SIZE 8

/*
 * Keyblock, containing the public key used to sign some other chunk of data.
 *
 * This should be followed by:
 *   1) The data_key key data, pointed to by data_key.key_offset.
 *   2) The checksum data for (vb2_keyblock + data_key data), pointed to
 *      by keyblock_checksum.sig_offset.
 *   3) The signature data for (vb2_keyblock + data_key data), pointed to
 *      by keyblock_signature.sig_offset.
 */
struct vb2_keyblock {
	/* Magic number */
	uint8_t magic[VB2_KEYBLOCK_MAGIC_SIZE];

	/* Version of this header format */
	uint32_t header_version_major;
	uint32_t header_version_minor;

	/*
	 * Length of this entire keyblock, including keys, signatures, and
	 * padding, in bytes
	 */
	uint32_t keyblock_size;
	uint32_t reserved0;

	/*
	 * Signature for this keyblock (header + data pointed to by data_key)
	 * For use with signed data keys
	 */
	struct vb2_signature keyblock_signature;

	/*
	 * SHA-512 hash for this keyblock (header + data pointed to by
	 * data_key) For use with unsigned data keys.
	 *
	 * Only supported for kernel keyblocks, not firmware keyblocks.
	 */
	struct vb2_signature keyblock_hash;

	/* Flags for key (VB2_KEYBLOCK_FLAG_*) */
	uint32_t keyblock_flags;
	uint32_t reserved1;

	/* Key to verify the chunk of data */
	struct vb2_packed_key data_key;
} __attribute__((packed));

#define EXPECTED_VB2_KEYBLOCK_SIZE 112

/*
 * Preamble block for kernel, version 2.2
 *
 * This should be followed by:
 *   1) The signature data for the kernel body, pointed to by
 *      body_signature.sig_offset.
 *   2) The signature data for (vb2_kernel_preamble + body signature data),
 *       pointed to by preamble_signature.sig_offset.
 *   3) The 16-bit vmlinuz header, which is used for reconstruction of
 *      vmlinuz image.
 */
struct vb2_kernel_preamble {
	/*
	 * Size of this preamble, including keys, signatures, vmlinuz header,
	 * and padding, in bytes
	 */
	uint32_t preamble_size;
	uint32_t reserved0;

	/* Signature for this preamble (header + body signature) */
	struct vb2_signature preamble_signature;

	/* Version of this header format */
	uint32_t header_version_major;
	uint32_t header_version_minor;

	/* Kernel version */
	uint32_t kernel_version;
	uint32_t reserved1;

	/* Load address for kernel body */
	uint64_t body_load_address;
	/* TODO (vboot 2.1): we never used that */

	/* Address of bootloader, after body is loaded at body_load_address */
	uint64_t bootloader_address;
	/* TODO (vboot 2.1): should be a 32-bit offset */

	/* Size of bootloader in bytes */
	uint32_t bootloader_size;
	uint32_t reserved2;

	/* Signature for the kernel body */
	struct vb2_signature body_signature;

	/*
	 * TODO (vboot 2.1): fields for kernel offset and size.  Right now the
	 * size is implicitly the same as the size of data signed by the body
	 * signature, and the offset is implicitly at the end of the preamble.
	 * But that forces us to pad the preamble to 64KB rather than just
	 * having a tiny preamble and an offset field.
	 */

	/*
	 * Fields added in header version 2.1.  You must verify the header
	 * version before reading these fields!
	 */

	/*
	 * Address of 16-bit header for vmlinuz reassembly.  Readers should
	 * return 0 for header version < 2.1.
	 */
	uint64_t vmlinuz_header_address;

	/* Size of 16-bit header for vmlinuz in bytes.  Readers should return 0
	   for header version < 2.1 */
	uint32_t vmlinuz_header_size;
	uint32_t reserved3;

	/*
	 * Fields added in header version 2.2.  You must verify the header
	 * version before reading these fields!
	 */

	/*
	 * Flags; see VB2_KERNEL_PREAMBLE_*.  Readers should return 0 for
	 * header version < 2.2.  Flags field is currently defined as:
	 * [31:2] - Reserved (for future use)
	 * [1:0]  - Kernel image type (0b00 - CrOS,
	 *                             0b01 - bootimg,
	 *                             0b10 - multiboot)
	 */
	uint32_t flags;
} __attribute__((packed));

#endif /* __BOOTMETH_CROS_H */
