/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright 2021 Linaro Limited
 *		Author: AKASHI Takahiro
 *
 * derived from efi.h and efi_api.h to make the file POSIX-compliant
 */

#ifndef _EFI_CAPSULE_H
#define _EFI_CAPSULE_H

#include <stdint.h>

/*
 * Gcc's predefined attributes are not recognized by clang.
 */
#ifndef __packed
#define __packed	__attribute__((__packed__))
#endif

#ifndef __aligned
#define __aligned(x)	__attribute__((__aligned__(x)))
#endif

#define ARRAY_SIZE(x)		(sizeof(x) / sizeof((x)[0]))

typedef struct {
	uint8_t b[16];
} efi_guid_t __aligned(8);

#define EFI_GUID(a, b, c, d0, d1, d2, d3, d4, d5, d6, d7) \
	{{ (a) & 0xff, ((a) >> 8) & 0xff, ((a) >> 16) & 0xff, \
		((a) >> 24) & 0xff, \
		(b) & 0xff, ((b) >> 8) & 0xff, \
		(c) & 0xff, ((c) >> 8) & 0xff, \
		(d0), (d1), (d2), (d3), (d4), (d5), (d6), (d7) } }

#define EFI_FIRMWARE_MANAGEMENT_CAPSULE_ID_GUID \
	EFI_GUID(0x6dcbd5ed, 0xe82d, 0x4c44, 0xbd, 0xa1, \
		 0x71, 0x94, 0x19, 0x9a, 0xd9, 0x2a)

#define EFI_CERT_TYPE_PKCS7_GUID \
	EFI_GUID(0x4aafd29d, 0x68df, 0x49ee, 0x8a, 0xa9, \
		 0x34, 0x7d, 0x37, 0x56, 0x65, 0xa7)

#define FW_ACCEPT_OS_GUID \
	EFI_GUID(0x0c996046, 0xbcc0, 0x4d04, 0x85, 0xec, \
		 0xe1, 0xfc, 0xed, 0xf1, 0xc6, 0xf8)

#define FW_REVERT_OS_GUID \
	EFI_GUID(0xacd58b4b, 0xc0e8, 0x475f, 0x99, 0xb5, \
		 0x6b, 0x3f, 0x7e, 0x07, 0xaa, 0xf0)

/* flags */
#define CAPSULE_FLAGS_PERSIST_ACROSS_RESET      0x00010000

struct efi_capsule_header {
	efi_guid_t capsule_guid;
	uint32_t header_size;
	uint32_t flags;
	uint32_t capsule_image_size;
} __packed;

struct efi_firmware_management_capsule_header {
	uint32_t version;
	uint16_t embedded_driver_count;
	uint16_t payload_item_count;
	uint64_t item_offset_list[];
} __packed;

/* image_capsule_support */
#define CAPSULE_SUPPORT_AUTHENTICATION          0x0000000000000001

struct efi_firmware_management_capsule_image_header {
	uint32_t version;
	efi_guid_t update_image_type_id;
	uint8_t update_image_index;
	uint8_t reserved[3];
	uint32_t update_image_size;
	uint32_t update_vendor_code_size;
	uint64_t update_hardware_instance;
	uint64_t image_capsule_support;
} __packed;

/**
 * win_certificate_uefi_guid - A certificate that encapsulates
 * a GUID-specific signature
 *
 * @hdr:	Windows certificate header, cf. WIN_CERTIFICATE
 * @cert_type:	Certificate type
 */
struct win_certificate_uefi_guid {
	struct {
		uint32_t dwLength;
		uint16_t wRevision;
		uint16_t wCertificateType;
	} hdr;
	efi_guid_t cert_type;
} __packed;

/**
 * efi_firmware_image_authentication - Capsule authentication method
 * descriptor
 *
 * This structure describes an authentication information for
 * a capsule with IMAGE_ATTRIBUTE_AUTHENTICATION_REQUIRED set
 * and should be included as part of the capsule.
 * Only EFI_CERT_TYPE_PKCS7_GUID is accepted.
 *
 * @monotonic_count: Count to prevent replay
 * @auth_info: Authentication info
 */
struct efi_firmware_image_authentication {
	uint64_t monotonic_count;
	struct win_certificate_uefi_guid auth_info;
} __packed;

/* fmp payload header */
#define SIGNATURE_16(A, B)	((A) | ((B) << 8))
#define SIGNATURE_32(A, B, C, D)	\
	(SIGNATURE_16(A, B) | (SIGNATURE_16(C, D) << 16))

#define FMP_PAYLOAD_HDR_SIGNATURE	SIGNATURE_32('M', 'S', 'S', '1')

/**
 * struct fmp_payload_header - EDK2 header for the FMP payload
 *
 * This structure describes the header which is preprended to the
 * FMP payload by the edk2 capsule generation scripts.
 *
 * @signature:			Header signature used to identify the header
 * @header_size:		Size of the structure
 * @fw_version:			Firmware versions used
 * @lowest_supported_version:	Lowest supported version (not used)
 */
struct fmp_payload_header {
	uint32_t signature;
	uint32_t header_size;
	uint32_t fw_version;
	uint32_t lowest_supported_version;
};

struct fmp_payload_header_params {
	bool have_header;
	uint32_t fw_version;
};

#endif /* _EFI_CAPSULE_H */
