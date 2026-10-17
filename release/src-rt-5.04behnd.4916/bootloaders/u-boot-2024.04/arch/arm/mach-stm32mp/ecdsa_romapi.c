// SPDX-License-Identifier: GPL-2.0+
/*
 * STM32MP ECDSA verification via the ROM API
 *
 * Implements ECDSA signature verification via the STM32MP ROM.
 */
#include <asm/system.h>
#include <asm/arch/sys_proto.h>
#include <dm/device.h>
#include <linux/types.h>
#include <u-boot/ecdsa.h>
#include <crypto/ecdsa-uclass.h>
#include <linux/libfdt.h>
#include <dm/platdata.h>

#define ROM_API_SUCCESS				0x77
#define ROM_API_ECDSA_ALGO_PRIME_256V1		1
#define ROM_API_ECDSA_ALGO_BRAINPOOL_256	2

#define ROM_API_OFFSET_ECDSA_VERIFY		0x60

struct ecdsa_rom_api {
	uint32_t (*ecdsa_verify_signature)(const void *hash, const void *pubkey,
					   const void *signature,
					   uint32_t ecc_algo);
};

static void stm32mp_rom_get_ecdsa_functions(struct ecdsa_rom_api *rom)
{
	uintptr_t verify_ptr = get_stm32mp_rom_api_table() +
			       ROM_API_OFFSET_ECDSA_VERIFY;

	rom->ecdsa_verify_signature = *(void **)verify_ptr;
}

static int ecdsa_key_algo(const char *curve_name)
{
	if (!strcmp(curve_name, "prime256v1"))
		return ROM_API_ECDSA_ALGO_PRIME_256V1;
	else if (!strcmp(curve_name, "brainpool256"))
		return ROM_API_ECDSA_ALGO_BRAINPOOL_256;
	else
		return -ENOPROTOOPT;
}

static int romapi_ecdsa_verify(struct udevice *dev,
			       const struct ecdsa_public_key *pubkey,
			       const void *hash, size_t hash_len,
			       const void *signature, size_t sig_len)
{
	struct ecdsa_rom_api rom;
	uint8_t raw_key[64];
	uint32_t rom_ret;
	int algo;

	/* The ROM API can only handle 256-bit ECDSA keys. */
	if (sig_len != 64 || hash_len != 32 || pubkey->size_bits != 256)
		return -EINVAL;

	algo = ecdsa_key_algo(pubkey->curve_name);
	if (algo < 0)
		return algo;

	/* The ROM API wants the (X, Y) coordinates concatenated. */
	memcpy(raw_key, pubkey->x, 32);
	memcpy(raw_key + 32, pubkey->y, 32);

	stm32mp_rom_get_ecdsa_functions(&rom);

	/* Mark BootROM region as executable. */
	mmu_set_region_dcache_behaviour(0, SZ_2M, DCACHE_DEFAULT_OPTION);

	rom_ret = rom.ecdsa_verify_signature(hash, raw_key, signature, algo);

	return rom_ret == ROM_API_SUCCESS ? 0 : -EPERM;
}

static const struct ecdsa_ops rom_api_ops = {
	.verify = romapi_ecdsa_verify,
};

U_BOOT_DRIVER(stm32mp_rom_api_ecdsa) = {
	.name	= "stm32mp_rom_api_ecdsa",
	.id	= UCLASS_ECDSA,
	.ops	= &rom_api_ops,
	.flags	= DM_FLAG_PRE_RELOC,
};

U_BOOT_DRVINFO(stm32mp_rom_api_ecdsa) = {
	.name = "stm32mp_rom_api_ecdsa",
};
