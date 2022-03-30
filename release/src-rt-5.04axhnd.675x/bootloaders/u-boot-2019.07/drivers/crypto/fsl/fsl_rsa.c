// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014 Freescale Semiconductor, Inc.
 * Author: Ruchika Gupta <ruchika.gupta@freescale.com>
 */

#include <config.h>
#include <common.h>
#include <dm.h>
#include <asm/types.h>
#include <malloc.h>
#include "jobdesc.h"
#include "desc.h"
#include "jr.h"
#include "rsa_caam.h"
#include <u-boot/rsa-mod-exp.h>

int fsl_mod_exp(struct udevice *dev, const uint8_t *sig, uint32_t sig_len,
		struct key_prop *prop, uint8_t *out)
{
	uint32_t keylen;
	struct pk_in_params pkin;
	uint32_t desc[MAX_CAAM_DESCSIZE];
	int ret;

	/* Length in bytes */
	keylen = prop->num_bits / 8;

	pkin.a = sig;
	pkin.a_siz = sig_len;
	pkin.n = prop->modulus;
	pkin.n_siz = keylen;
	pkin.e = prop->public_exponent;
	pkin.e_siz = prop->exp_len;

	inline_cnstr_jobdesc_pkha_rsaexp(desc, &pkin, out, sig_len);

	ret = run_descriptor_jr(desc);
	if (ret) {
		debug("%s: RSA failed to verify: %d\n", __func__, ret);
		return -EFAULT;
	}

	return 0;
}

static const struct mod_exp_ops fsl_mod_exp_ops = {
	.mod_exp	= fsl_mod_exp,
};

U_BOOT_DRIVER(fsl_rsa_mod_exp) = {
	.name	= "fsl_rsa_mod_exp",
	.id	= UCLASS_MOD_EXP,
	.ops	= &fsl_mod_exp_ops,
	.flags  = DM_FLAG_PRE_RELOC,
};

U_BOOT_DEVICE(fsl_rsa) = {
	.name = "fsl_rsa_mod_exp",
};
