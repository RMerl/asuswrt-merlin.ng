// SPDX-License-Identifier: GPL-2.0+
/*
 *
 * Command for encapsulating/decapsulating blob of memory.
 */

#include <common.h>
#include <command.h>
#include <malloc.h>
#include <asm/byteorder.h>
#include <linux/compiler.h>
#if defined(CONFIG_ARCH_MX6) || defined(CONFIG_ARCH_MX7) || \
	defined(CONFIG_ARCH_MX7ULP) || defined(CONFIG_ARCH_IMX8M)
#include <fsl_sec.h>
#include <asm/arch/clock.h>
#endif

/**
 * blob_decap() - Decapsulate the data as a blob
 * @key_mod:	- Pointer to key modifier/key
 * @src:	- Address of data to be decapsulated
 * @dst:	- Address of data to be decapsulated
 * @len:	- Size of data to be decapsulated
 *
 * Returns zero on success,and negative on error.
 */
__weak int blob_decap(u8 *key_mod, u8 *src, u8 *dst, u32 len)
{
	return 0;
}

/**
 * blob_encap() - Encapsulate the data as a blob
 * @key_mod:	- Pointer to key modifier/key
 * @src:	- Address of data to be encapsulated
 * @dst:	- Address of data to be encapsulated
 * @len:	- Size of data to be encapsulated
 *
 * Returns zero on success,and negative on error.
 */
__weak int blob_encap(u8 *key_mod, u8 *src, u8 *dst, u32 len)
{
	return 0;
}

/**
 * do_blob() - Handle the "blob" command-line command
 * @cmdtp:	Command data struct pointer
 * @flag:	Command flag
 * @argc:	Command-line argument count
 * @argv:	Array of command-line arguments
 *
 * Returns zero on success, CMD_RET_USAGE in case of misuse and negative
 * on error.
 */
static int do_blob(struct cmd_tbl *cmdtp, int flag, int argc,
		   char *const argv[])
{
	ulong key_addr, src_addr, dst_addr, len;
	uint8_t *km_ptr, *src_ptr, *dst_ptr;
	int enc, ret = 0;

	if (argc != 6)
		return CMD_RET_USAGE;

	if (!strncmp(argv[1], "enc", 3))
		enc = 1;
	else if (!strncmp(argv[1], "dec", 3))
		enc = 0;
	else
		return CMD_RET_USAGE;

	src_addr = hextoul(argv[2], NULL);
	dst_addr = hextoul(argv[3], NULL);
	len = hextoul(argv[4], NULL);
	key_addr = hextoul(argv[5], NULL);

	km_ptr = (uint8_t *)(uintptr_t)key_addr;
	src_ptr = (uint8_t *)(uintptr_t)src_addr;
	dst_ptr = (uint8_t *)(uintptr_t)dst_addr;

#if defined(CONFIG_ARCH_MX6) || defined(CONFIG_ARCH_MX7) || \
	defined(CONFIG_ARCH_MX7ULP) || defined(CONFIG_ARCH_IMX8M)

	hab_caam_clock_enable(1);

	u32 out_jr_size = sec_in32(CFG_SYS_FSL_JR0_ADDR +
				   FSL_CAAM_ORSR_JRa_OFFSET);
	if (out_jr_size != FSL_CAAM_MAX_JR_SIZE)
		sec_init();
#endif

	if (enc)
		ret = blob_encap(km_ptr, src_ptr, dst_ptr, len);
	else
		ret = blob_decap(km_ptr, src_ptr, dst_ptr, len);

	return ret;
}

/***************************************************/
static char blob_help_text[] =
	"enc src dst len km - Encapsulate and create blob of data\n"
	"                          $len bytes long at address $src and\n"
	"                          store the result at address $dst.\n"
	"                          $km is the address where the key\n"
	"                          modifier is stored.\n"
	"                          The modifier is required for generation\n"
	"                          /use as key for cryptographic operation.\n"
	"                          Key modifier should be 16 byte long.\n"
	"blob dec src dst len km - Decapsulate the  blob of data at address\n"
	"                          $src and store result of $len byte at\n"
	"                          addr $dst.\n"
	"                          $km is the address where the key\n"
	"                          modifier is stored.\n"
	"                          The modifier is required for generation\n"
	"                          /use as key for cryptographic operation.\n"
	"                          Key modifier should be 16 byte long.\n";

U_BOOT_CMD(
	blob, 6, 1, do_blob,
	"Blob encapsulation/decryption",
	blob_help_text
);
