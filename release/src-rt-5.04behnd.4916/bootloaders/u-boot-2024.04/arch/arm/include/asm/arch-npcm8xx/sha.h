/* SPDX-License-Identifier: GPL-2.0+ */

#ifndef _NPCM_SHA_H_
#define _NPCM_SHA_H_

#define HASH_DIG_H_NUM        8

/* SHA type */
enum npcm_sha_type {
	npcm_sha_type_sha2 = 0,
	npcm_sha_type_sha1,
	npcm_sha_type_num
};

struct npcm_sha_regs {
	unsigned int hash_data_in;
	unsigned char hash_ctr_sts;
	unsigned char reserved_0[0x03];
	unsigned char hash_cfg;
	unsigned char reserved_1[0x03];
	unsigned char hash_ver;
	unsigned char reserved_2[0x13];
	unsigned int hash_dig[HASH_DIG_H_NUM];
};

#define HASH_CTR_STS_SHA_EN             BIT(0)
#define HASH_CTR_STS_SHA_BUSY           BIT(1)
#define HASH_CTR_STS_SHA_RST            BIT(2)
#define HASH_CFG_SHA1_SHA2              BIT(0)

int npcm_sha_calc(u8 type, const u8 *buf, u32 len, u8 *digest);
int npcm_sha_selftest(u8 type);

#endif
