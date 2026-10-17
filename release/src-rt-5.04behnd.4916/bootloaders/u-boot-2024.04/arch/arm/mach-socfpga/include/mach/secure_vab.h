/* SPDX-License-Identifier: GPL-2.0
 *
 * Copyright (C) 2020 Intel Corporation <www.intel.com>
 *
 */

#ifndef	_SECURE_VAB_H_
#define	_SECURE_VAB_H_

#include <linux/sizes.h>
#include <linux/stddef.h>
#include <u-boot/sha512.h>

#define VAB_DATA_SZ			64

#define SDM_CERT_MAGIC_NUM		0x25D04E7F
#define FCS_HPS_VAB_MAGIC_NUM		0xD0564142

#define MAX_CERT_SIZE			(SZ_4K)

/*
 * struct fcs_hps_vab_certificate_data
 * @vab_cert_magic_num: VAB Certificate Magic Word (0xD0564142)
 * @flags: TBD
 * @fcs_data: Data words being certificate signed.
 * @cert_sign_keychain: Certificate Signing Keychain
 */
struct fcs_hps_vab_certificate_data {
	u32 vab_cert_magic_num;		/* offset 0x10 */
	u32 flags;
	u8 rsvd0_1[8];
	u8 fcs_sha384[SHA384_SUM_LEN];	/* offset 0x20 */
};

/*
 * struct fcs_hps_vab_certificate_header
 * @cert_magic_num: Certificate Magic Word (0x25D04E7F)
 * @cert_data_sz: size of this certificate header (0x80)
 *	Includes magic number all the way to the certificate
 *      signing keychain (excludes cert. signing keychain)
 * @cert_ver: Certificate Version
 * @cert_type: Certificate Type
 * @data: VAB HPS Image Certificate data
 */
struct fcs_hps_vab_certificate_header {
	u32 cert_magic_num;		/* offset 0 */
	u32 cert_data_sz;
	u32 cert_ver;
	u32 cert_type;
	struct fcs_hps_vab_certificate_data d;	/* offset 0x10 */
	/* keychain starts at offset 0x50 */
};

#define VAB_CERT_HEADER_SIZE	sizeof(struct fcs_hps_vab_certificate_header)
#define VAB_CERT_MAGIC_OFFSET	offsetof \
				(struct fcs_hps_vab_certificate_header, d)
#define VAB_CERT_FIT_SHA384_OFFSET	offsetof \
					(struct fcs_hps_vab_certificate_data, \
					 fcs_sha384[0])

int socfpga_vendor_authentication(void **p_image, size_t *p_size);

#endif /* _SECURE_VAB_H_ */
