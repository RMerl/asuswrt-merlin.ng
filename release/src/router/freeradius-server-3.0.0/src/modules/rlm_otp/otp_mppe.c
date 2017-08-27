/*
 * $Id$
 *
 *	 This program is free software; you can redistribute it and/or modify
 *	 it under the terms of the GNU General Public License as published by
 *	 the Free Software Foundation; either version 2 of the License, or
 *	 (at your option) any later version.
 *
 *	 This program is distributed in the hope that it will be useful,
 *	 but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
 *	 GNU General Public License for more details.
 *
 *	 You should have received a copy of the GNU General Public License
 *	 along with this program; if not, write to the Free Software
 *	 Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 *
 * Copyright 2001,2002	Google, Inc.
 * Copyright 2005,2006 TRI-D Systems, Inc.
 */

RCSID("$Id$")
USES_APPLE_DEPRECATED_API	/* OpenSSL API has been deprecated by Apple */

/* avoid inclusion of these FR headers which conflict w/ OpenSSL */
#define _FR_MD4_H
#define _FR_SHA1_H

#include <freeradius-devel/rad_assert.h>

#include "extern.h"
#include "otp.h"
#include "otp_mppe.h"

#include <openssl/des.h>
#include <openssl/md4.h>
#include <openssl/md5.h>
#include <openssl/sha.h>

#include <string.h>

/*
 * Add MPPE attributes to a request, if required.
 */
void otp_mppe(REQUEST *request, otp_pwe_t pwe, rlm_otp_t const *opt, char const *passcode)
{
	VALUE_PAIR *cvp, *rvp;

	cvp = pairfind(request->packet->vps, pwattr[pwe - 1]->attr, pwattr[pwe - 1]->vendor, TAG_ANY);
	rvp = pairfind(request->packet->vps, pwattr[pwe]->attr, pwattr[pwe]->vendor, TAG_ANY);
	if (!cvp || !rvp) {
		return;
	}

	switch (pwe) {
	case PWE_PAP:
	case PWE_CHAP:
		return;

	case PWE_MSCHAP:
		/* First, set some related attributes. */
		pairmake_reply("MS-MPPE-Encryption-Policy", otp_mppe_policy[opt->mschap_mppe_policy], T_OP_EQ);
		pairmake_reply("MS-MPPE-Encryption-Types", otp_mppe_types[opt->mschap_mppe_types], T_OP_EQ);

		/* If no MPPE, we're done. */
		if (!opt->mschap_mppe_policy) {
			return;
		}

		/*
		 * Generate the MS-CHAP-MPPE-Keys attribute.	This is not specified
		 * anywhere -- RFC 2548, par. 2.4.1 is the authority but it has
		 * typos and omissions that make this unimplementable.	The
		 * code here is based on experimental results provided by
		 * Takahiro Wagatsuma <waga@sic.shibaura-it.ac.jp>.
		 * We only support 128-bit keys derived from the NT hash; 40-bit
		 * and 56-bit keys are derived from the LM hash, which besides
		 * being deprecated, has severe security problems.
		 */
		{
			size_t i, passcode_len;
			uint8_t password_unicode[2 * OTP_MAX_PASSCODE_LEN];
			uint8_t password_md[MD4_DIGEST_LENGTH];
			uint8_t mppe_keys[32];

			/* 0x ASCII(mppe_keys)	'\0' */
			char mppe_keys_string[2 + (2 * sizeof(mppe_keys)) + 1];

			/* Zero the LM-Key sub-field (and padding). */
			(void) memset(mppe_keys, 0, sizeof(mppe_keys));

			/*
			 * The NT-Key sub-field is MD4(MD4(unicode(password))).
			 * Start by hashing the unicode passcode.
			 * This is broken because unicode chars are machine-ordered,
			 * but the spec (RFC 2433) doesn't say how to prepare
			 * the password for md4 (other than by example values).
			 */
			passcode_len = strlen(passcode);
			for (i = 0; i < passcode_len; ++i) {
				/* Set the high order 8 bits to 0 (little-endian) */
				password_unicode[i * 2] = *passcode++;
				password_unicode[i * 2 + 1] = 0;
			}

			/* first md4 */
			(void) MD4(password_unicode, 2 * passcode_len, password_md);
			/* second md4 */
			(void) MD4(password_md, MD4_DIGEST_LENGTH, &mppe_keys[8]);

			/* Whew. Now stringify it for pairmake(). */
			mppe_keys_string[0] = '0';
			mppe_keys_string[1] = 'x';

			for (i = 0; i < 32; ++i) {
				(void) sprintf(&mppe_keys_string[i*2+2], "%02X", mppe_keys[i]);
			}

			pairmake_reply("MS-CHAP-MPPE-Keys", mppe_keys_string, T_OP_EQ);
		} /* (doing mppe) */
	break; /* PWE_MSCHAP */

	case PWE_MSCHAP2:
	{
		size_t i;
		uint8_t password_md_md[MD4_DIGEST_LENGTH];

		/*
		 * MS-CHAPv2 requires mutual authentication; we must prove
		 * that we know the secret.	This is a bit circuitous: set
		 * MD1 = SHA(MD4(MD4(unicode(password)))|NT_RESPONSE|MAGIC1),
		 * MD2 = MSB8(SHA(PEER_CHALLENGE|MS_CHAP_CHALLENGE|USERNAME)),
		 * and finally use SHA(MD1|MD2|MAGIC2) as the authenticator.
		 * The authenticator is returned as the string "S=<auth>",
		 * <auth> is the authenticator expressed as [uppercase] ASCII.
		 * See RFC 2759.
		 */
		{
			size_t passcode_len;
			uint8_t password_unicode[2 * OTP_MAX_PASSCODE_LEN];
			uint8_t password_md[MD4_DIGEST_LENGTH];

			SHA_CTX ctx;
			uint8_t md1[SHA_DIGEST_LENGTH];
			uint8_t md2[SHA_DIGEST_LENGTH];
			uint8_t auth_md[SHA_DIGEST_LENGTH];
			/* S=(ASCII(auth_md))\0 */
			char auth_md_string[2 + (2 * sizeof(auth_md)) + 1];
			/*
			 * ugh.	The ASCII authenticator (auth_md_string) is sent
			 * along with a single (useless) binary byte (the ID).
			 * So we must "stringify" it again (for pairmake()) since the
			 * binary byte requires the attribute to be of type "octets".
			 */
			/* 0x(ID)(ASCII("S="ASCII(auth_md))) */
			char auth_octet_string[2 + 2 + (2 * sizeof(auth_md_string))];

			char const *username = request->username->vp_strvalue;
			int username_len = request->username->length;

			/* "Magic server to client signing constant" */
			uint8_t magic1[39] = {
				0x4D, 0x61, 0x67, 0x69, 0x63, 0x20, 0x73, 0x65, 0x72, 0x76,
				0x65, 0x72, 0x20, 0x74, 0x6F, 0x20, 0x63, 0x6C, 0x69, 0x65,
				0x6E, 0x74, 0x20, 0x73, 0x69, 0x67, 0x6E, 0x69, 0x6E, 0x67,
				0x20, 0x63, 0x6F, 0x6E, 0x73, 0x74, 0x61, 0x6E, 0x74
			};
			/* "Pad to make it do more than one iteration" */
			uint8_t magic2[41] = {
				0x50, 0x61, 0x64, 0x20, 0x74, 0x6F, 0x20, 0x6D, 0x61, 0x6B,
				0x65, 0x20, 0x69, 0x74, 0x20, 0x64, 0x6F, 0x20, 0x6D, 0x6F,
				0x72, 0x65, 0x20, 0x74, 0x68, 0x61, 0x6E, 0x20, 0x6F, 0x6E,
				0x65, 0x20, 0x69, 0x74, 0x65, 0x72, 0x61, 0x74, 0x69, 0x6F,
				0x6E
			};

			/*
			 * Start by hashing the unicode passcode.
			 * This is broken because unicode chars are machine-ordered,
			 * but the spec (RFC 2759) doesn't say how to prepare
			 * the password for md4 (other than by example values).
			 */
			passcode_len = strlen(passcode);
			for (i = 0; i < passcode_len; ++i) {
				/* Set the high order 8 bits to 0 (little-endian) */
				password_unicode[i * 2] = *passcode++;
				password_unicode[i * 2 + 1] = 0;
			}
			/* first md4 */
			(void) MD4(password_unicode, 2 * passcode_len, password_md);
			/* second md4 */
			(void) MD4(password_md, MD4_DIGEST_LENGTH, password_md_md);

			/* MD1 */
			SHA1_Init(&ctx);
			SHA1_Update(&ctx, password_md_md, MD4_DIGEST_LENGTH);
			SHA1_Update(&ctx, rvp->vp_strvalue + 26, 24);
			SHA1_Update(&ctx, magic1, sizeof(magic1));
			SHA1_Final(md1, &ctx);

			/* MD2 */
			SHA1_Init(&ctx);
			SHA1_Update(&ctx, rvp->vp_strvalue + 2, 16);
			SHA1_Update(&ctx, cvp->vp_strvalue, 16);
			SHA1_Update(&ctx, username, username_len);
			SHA1_Final(md2, &ctx);

			/* The Authenticator */
			SHA1_Init(&ctx);
			SHA1_Update(&ctx, md1, SHA_DIGEST_LENGTH);
			SHA1_Update(&ctx, md2, 8);
			SHA1_Update(&ctx, magic2, sizeof(magic2));
			SHA1_Final(auth_md, &ctx);

			/* String conversion. */
			auth_md_string[0] = 'S';
			auth_md_string[1] = '=';
			for (i = 0; i < sizeof(auth_md); ++i) {
				(void) sprintf(&auth_md_string[i * 2 + 2], "%02X", auth_md[i]);
			}

			/* And then octet conversion.	Ugh! */
			auth_octet_string[0] = '0';
			auth_octet_string[1] = 'x';
			(void) sprintf(&auth_octet_string[2], "%02X", rvp->vp_strvalue[0]);
			for (i = 0; i < sizeof(auth_md_string) - 1; ++i) {
				(void) sprintf(&auth_octet_string[i * 2 +4], "%02X", auth_md_string[i]);
			}

			pairmake_reply("MS-CHAP2-Success", auth_octet_string, T_OP_EQ);
		} /* Generate mutual auth info. */

		/*
		 * Now, set some MPPE related attributes.
		 */
		pairmake_reply("MS-MPPE-Encryption-Policy", otp_mppe_policy[opt->mschapv2_mppe_policy], T_OP_EQ);
		pairmake_reply("MS-MPPE-Encryption-Types", otp_mppe_types[opt->mschapv2_mppe_types], T_OP_EQ);

		/* If no MPPE, we're done. */
		if (!opt->mschapv2_mppe_policy) {
			return;
		}

		/*
		 * Generate the MPPE initial session key, per RFC 3079.
		 * (Although, RFC 2548 leaves us guessing at how to generate this.)
		 * For MS-CHAPv2 we support all key lengths (40-, 56- and 128-bit),
		 * although MPPE via RADIUS supports only 40- and 128-bit keys.
		 * This is a bit more complicated than MS-CHAP.	Start by generating
		 * a "master session key"
		 *		MSB16(SHA(NTPasswordHashHash|NT_RESPONSE|MAGIC1)), where
		 * NTPasswordHashHash is MD4(MD4(unicode(password))), NT_RESPONSE
		 * is from the MS-CHAP2-Response attribute, and MAGIC1 is a
		 * constant from RFC 3079.	Then, we derive asymmetric send/receive
		 * keys from the master session key.	The "master send key" is
		 *		 MSBx(SHA(MASTERKEY|SHSPAD1|MAGIC3|SHSPAD2)),
		 * and the "master receive key" is
		 *		 MSBx(SHA(MASTERKEY|SHSPAD1|MAGIC2|SHSPAD2)), where
		 * MASTERKEY is the "master session key" generated above, and the
		 * other values are constants from RFC 3079.	MSBx is the x-most
		 * significant bytes, where x is 5, 7, or 16 as appropriate for
		 * the desired key length.	We always generate 16 byte (128-bit)
		 * keys, the NAS is required to truncate as needed.
		 */
		{
			/* These constants and key vars are named from RFC 3079. */
			/* "This is the MPPE Master Key" */
			uint8_t Magic1[27] = {
				0x54, 0x68, 0x69, 0x73, 0x20, 0x69, 0x73, 0x20, 0x74,
				0x68, 0x65, 0x20, 0x4d, 0x50, 0x50, 0x45, 0x20, 0x4d,
				0x61, 0x73, 0x74, 0x65, 0x72, 0x20, 0x4b, 0x65, 0x79
			};
			/* "On the client side, this is the send key; on the server side, it is the receive key." */
			uint8_t Magic2[84] = {
				0x4f, 0x6e, 0x20, 0x74, 0x68, 0x65, 0x20, 0x63, 0x6c, 0x69,
				0x65, 0x6e, 0x74, 0x20, 0x73, 0x69, 0x64, 0x65, 0x2c, 0x20,
				0x74, 0x68, 0x69, 0x73, 0x20, 0x69, 0x73, 0x20, 0x74, 0x68,
				0x65, 0x20, 0x73, 0x65, 0x6e, 0x64, 0x20, 0x6b, 0x65, 0x79,
				0x3b, 0x20, 0x6f, 0x6e, 0x20, 0x74, 0x68, 0x65, 0x20, 0x73,
				0x65, 0x72, 0x76, 0x65, 0x72, 0x20, 0x73, 0x69, 0x64, 0x65,
				0x2c, 0x20, 0x69, 0x74, 0x20, 0x69, 0x73, 0x20, 0x74, 0x68,
				0x65, 0x20, 0x72, 0x65, 0x63, 0x65, 0x69, 0x76, 0x65, 0x20,
				0x6b, 0x65, 0x79, 0x2e
			};
			/* "On the client side, this is the receive key; on the server side, it is the send key." */
			uint8_t Magic3[84] = {
				0x4f, 0x6e, 0x20, 0x74, 0x68, 0x65, 0x20, 0x63, 0x6c, 0x69,
				0x65, 0x6e, 0x74, 0x20, 0x73, 0x69, 0x64, 0x65, 0x2c, 0x20,
				0x74, 0x68, 0x69, 0x73, 0x20, 0x69, 0x73, 0x20, 0x74, 0x68,
				0x65, 0x20, 0x72, 0x65, 0x63, 0x65, 0x69, 0x76, 0x65, 0x20,
				0x6b, 0x65, 0x79, 0x3b, 0x20, 0x6f, 0x6e, 0x20, 0x74, 0x68,
				0x65, 0x20, 0x73, 0x65, 0x72, 0x76, 0x65, 0x72, 0x20, 0x73,
				0x69, 0x64, 0x65, 0x2c, 0x20, 0x69, 0x74, 0x20, 0x69, 0x73,
				0x20, 0x74, 0x68, 0x65, 0x20, 0x73, 0x65, 0x6e, 0x64, 0x20,
				0x6b, 0x65, 0x79, 0x2e
			};
			uint8_t SHSpad1[40] = {
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
			};

			uint8_t SHSpad2[40] = {
				0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2,
				0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2,
				0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2,
				0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2
			};
			uint8_t MasterKey[16];
			uint8_t MasterSendKey[16];
			uint8_t MasterReceiveKey[16];

			SHA_CTX ctx;
			uint8_t sha_md[SHA_DIGEST_LENGTH];

			/* 0x(ASCII(mppe_key))\0 */
			char mppe_key_string[2 + (2 * sizeof(MasterKey)) + 1];

			/* Generate the master session key. */
			SHA1_Init(&ctx);
			SHA1_Update(&ctx, password_md_md, MD4_DIGEST_LENGTH);
			SHA1_Update(&ctx, rvp->vp_strvalue + 26, 24);
			SHA1_Update(&ctx, Magic1, sizeof(Magic1));
			SHA1_Final(sha_md, &ctx);
			(void) memcpy(MasterKey, sha_md, 16);

			/* Generate the master send key. */
			SHA1_Init(&ctx);
			SHA1_Update(&ctx, MasterKey, 16);
			SHA1_Update(&ctx, SHSpad1, 40);
			SHA1_Update(&ctx, Magic3, sizeof(Magic3));
			SHA1_Update(&ctx, SHSpad2, 40);
			SHA1_Final(sha_md, &ctx);
			(void) memcpy(MasterSendKey, sha_md, 16);

			/* Generate the master receive key. */
			SHA1_Init(&ctx);
			SHA1_Update(&ctx, MasterKey, 16);
			SHA1_Update(&ctx, SHSpad1, 40);
			SHA1_Update(&ctx, Magic2, sizeof(Magic3));
			SHA1_Update(&ctx, SHSpad2, 40);
			SHA1_Final(sha_md, &ctx);
			(void) memcpy(MasterReceiveKey, sha_md, 16);

			/*
			 * Now, generate the MS-MPPE-Send-Key attribute.
			 */
			mppe_key_string[0] = '0';
			mppe_key_string[1] = 'x';
			for (i = 0; i < sizeof(MasterSendKey); ++i) {
				(void) sprintf(&mppe_key_string[i*2+2], "%02X", MasterSendKey[i]);
			}

			pairmake_reply("MS-MPPE-Send-Key", mppe_key_string, T_OP_EQ);

			/*
			 * Generate the MS-MPPE-Recv-Key attribute.
			 */
			mppe_key_string[0] = '0';
			mppe_key_string[1] = 'x';
			for (i = 0; i < sizeof(MasterReceiveKey); ++i) {
				(void) sprintf(&mppe_key_string[i*2+2], "%02X", MasterReceiveKey[i]);
			}
			pairmake_reply("MS-MPPE-Recv-Key", mppe_key_string, T_OP_EQ);
		} /* (doing mppe) */

		break; /* PWE_MSCHAP2 */
	} /* PWE_MSCHAP2 */

	} /* switch (pwe) */

	return;
}
