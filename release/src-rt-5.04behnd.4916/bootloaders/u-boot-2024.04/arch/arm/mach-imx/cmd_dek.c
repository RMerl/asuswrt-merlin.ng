// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2008-2015 Freescale Semiconductor, Inc.
 * Copyright 2022 NXP
 *
 * Command for encapsulating DEK blob
 */

#include <common.h>
#include <command.h>
#include <log.h>
#include <malloc.h>
#include <memalign.h>
#include <asm/byteorder.h>
#include <linux/compiler.h>
#include <fsl_sec.h>
#include <asm/arch/clock.h>
#include <mapmem.h>
#include <tee.h>
#ifdef CONFIG_IMX_SECO_DEK_ENCAP
#include <imx_container.h>
#include <firmware/imx/sci/sci.h>
#endif
#ifdef CONFIG_IMX_ELE_DEK_ENCAP
#include <imx_container.h>
#include <asm/mach-imx/ele_api.h>
#endif

#include <cpu_func.h>

/**
* blob_dek() - Encapsulate the DEK as a blob using CAM's Key
* @src: - Address of data to be encapsulated
* @dst: - Desination address of encapsulated data
* @len: - Size of data to be encapsulated
*
* Returns zero on success,and negative on error.
*/
#ifdef CONFIG_IMX_CAAM_DEK_ENCAP
static int blob_encap_dek(u32 src_addr, u32 dst_addr, u32 len)
{
	u8 *src_ptr, *dst_ptr;

	src_ptr = map_sysmem(src_addr, len / 8);
	dst_ptr = map_sysmem(dst_addr, BLOB_SIZE(len / 8));

	hab_caam_clock_enable(1);

	u32 out_jr_size = sec_in32(CFG_SYS_FSL_JR0_ADDR +
				   FSL_CAAM_ORSR_JRa_OFFSET);
	if (out_jr_size != FSL_CAAM_MAX_JR_SIZE)
		sec_init();

	if (!((len == 128) | (len == 192) | (len == 256))) {
		debug("Invalid DEK size. Valid sizes are 128, 192 and 256b\n");
		return -1;
	}

	len /= 8;
	return blob_dek(src_ptr, dst_ptr, len);
}
#endif /* CONFIG_IMX_CAAM_DEK_ENCAP */

#ifdef CONFIG_IMX_OPTEE_DEK_ENCAP

#define PTA_DEK_BLOB_PTA_UUID {0xef477737, 0x0db1, 0x4a9d, \
	{0x84, 0x37, 0xf2, 0xf5, 0x35, 0xc0, 0xbd, 0x92} }

#define OPTEE_BLOB_HDR_SIZE		8

static int blob_encap_dek(u32 src_addr, u32 dst_addr, u32 len)
{
	struct udevice *dev = NULL;
	struct tee_shm *shm_input, *shm_output;
	struct tee_open_session_arg arg = {0};
	struct tee_invoke_arg arg_func = {0};
	const struct tee_optee_ta_uuid uuid = PTA_DEK_BLOB_PTA_UUID;
	struct tee_param param[4] = {0};
	int ret;

	/* Get tee device */
	dev = tee_find_device(NULL, NULL, NULL, NULL);
	if (!dev) {
		printf("Cannot get OP-TEE device\n");
		return -1;
	}

	/* Set TA UUID */
	tee_optee_ta_uuid_to_octets(arg.uuid, &uuid);

	/* Open TA session */
	ret = tee_open_session(dev, &arg, 0, NULL);
	if (ret < 0) {
		printf("Cannot open session with PTA Blob 0x%X\n", ret);
		return -1;
	}

	/* Allocate shared input and output buffers for TA */
	ret = tee_shm_register(dev, (void *)(ulong)src_addr, len / 8, 0x0, &shm_input);
	if (ret < 0) {
		printf("Cannot register input shared memory 0x%X\n", ret);
		goto error;
	}

	ret = tee_shm_register(dev, (void *)(ulong)dst_addr,
			       BLOB_SIZE(len / 8) + OPTEE_BLOB_HDR_SIZE,
			       0x0, &shm_output);
	if (ret < 0) {
		printf("Cannot register output shared memory 0x%X\n", ret);
		tee_shm_free(shm_input);
		goto error;
	}

	param[0].u.memref.shm	= shm_input;
	param[0].u.memref.size	= shm_input->size;
	param[0].attr		= TEE_PARAM_ATTR_TYPE_MEMREF_INPUT;
	param[1].u.memref.shm	= shm_output;
	param[1].u.memref.size	= shm_output->size;
	param[1].attr		= TEE_PARAM_ATTR_TYPE_MEMREF_OUTPUT;
	param[2].attr		= TEE_PARAM_ATTR_TYPE_NONE;
	param[3].attr		= TEE_PARAM_ATTR_TYPE_NONE;

	arg_func.func = 0;
	arg_func.session = arg.session;

	/* Generate DEK blob */
	arg_func.session = arg.session;
	ret = tee_invoke_func(dev, &arg_func, 4, param);
	if (ret < 0)
		printf("Cannot generate Blob with PTA DEK Blob 0x%X\n", ret);

	/* Free shared memory */
	tee_shm_free(shm_input);
	tee_shm_free(shm_output);

error:
	/* Close session */
	ret = tee_close_session(dev, arg.session);
	if (ret < 0)
		printf("Cannot close session with PTA DEK Blob 0x%X\n", ret);

	return ret;
}
#endif /* CONFIG_IMX_OPTEE_DEK_ENCAP */
#ifdef CONFIG_IMX_SECO_DEK_ENCAP

#define DEK_BLOB_KEY_ID				0x0

#define AHAB_PRIVATE_KEY			0x81
#define AHAB_VERSION				0x00
#define AHAB_MODE_CBC				0x67
#define AHAB_ALG_AES				0x55
#define AHAB_128_AES_KEY			0x10
#define AHAB_192_AES_KEY			0x18
#define AHAB_256_AES_KEY			0x20
#define AHAB_FLAG_KEK				0x80
#define AHAB_DEK_BLOB				0x01

#define DEK_BLOB_HDR_SIZE			8
#define SECO_PT					2U

static int blob_encap_dek(u32 src_addr, u32 dst_addr, u32 len)
{
	int err;
	sc_rm_mr_t mr_input, mr_output;
	struct generate_key_blob_hdr hdr;
	u8 in_size, out_size;
	u8 *src_ptr, *dst_ptr;
	int ret = 0;
	int i;

	/* Set sizes */
	in_size = sizeof(struct generate_key_blob_hdr) + len / 8;
	out_size = BLOB_SIZE(len / 8) + DEK_BLOB_HDR_SIZE;

	/* Get src and dst virtual addresses */
	src_ptr = map_sysmem(src_addr, in_size);
	dst_ptr = map_sysmem(dst_addr, out_size);

	/* Check addr input */
	if (!(src_ptr && dst_ptr)) {
		debug("src_addr or dst_addr invalid\n");
		return -1;
	}

	/* Build key header */
	hdr.version = AHAB_VERSION;
	hdr.length_lsb = sizeof(struct generate_key_blob_hdr) + len / 8;
	hdr.length_msb = 0x00;
	hdr.tag = AHAB_PRIVATE_KEY;
	hdr.flags = AHAB_DEK_BLOB;
	hdr.algorithm = AHAB_ALG_AES;
	hdr.mode = AHAB_MODE_CBC;

	switch (len) {
	case 128:
		hdr.size = AHAB_128_AES_KEY;
		break;
	case 192:
		hdr.size = AHAB_192_AES_KEY;
		break;
	case 256:
		hdr.size = AHAB_256_AES_KEY;
		break;
	default:
		/* Not supported */
		debug("Invalid DEK size. Valid sizes are 128, 192 and 256b\n");
		return -1;
	}

	/* Build input message */
	memmove((void *)(src_ptr + sizeof(struct generate_key_blob_hdr)),
		(void *)src_ptr, len / 8);
	memcpy((void *)src_ptr, (void *)&hdr,
	       sizeof(struct generate_key_blob_hdr));

	/* Flush the cache before triggering the CAAM DMA */
	flush_dcache_range(src_addr, src_addr + in_size);

	/* Find input memory region */
	err = sc_rm_find_memreg((-1), &mr_input, src_addr & ~(CONFIG_SYS_CACHELINE_SIZE - 1),
				ALIGN(src_addr + in_size, CONFIG_SYS_CACHELINE_SIZE));
	if (err) {
		printf("Error: find memory region 0x%X\n", src_addr);
		return -ENOMEM;
	}

	/* Find output memory region */
	err = sc_rm_find_memreg((-1), &mr_output, dst_addr & ~(CONFIG_SYS_CACHELINE_SIZE - 1),
				ALIGN(dst_addr + out_size, CONFIG_SYS_CACHELINE_SIZE));
	if (err) {
		printf("Error: find memory region 0x%X\n", dst_addr);
		return -ENOMEM;
	}

	/* Set memory region permissions for SECO */
	err = sc_rm_set_memreg_permissions(-1, mr_input, SECO_PT,
					   SC_RM_PERM_FULL);
	if (err) {
		printf("Set permission failed for input memory region\n");
		ret = -EPERM;
		goto error;
	}

	err = sc_rm_set_memreg_permissions(-1, mr_output, SECO_PT,
					   SC_RM_PERM_FULL);
	if (err) {
		printf("Set permission failed for output memory region\n");
		ret = -EPERM;
		goto error;
	}

	/* Flush output data before SECO operation */
	flush_dcache_range((ulong)dst_ptr, (ulong)(dst_ptr +
			roundup(out_size, ARCH_DMA_MINALIGN)));

	/* Generate DEK blob */
	err = sc_seco_gen_key_blob((-1), 0x0, src_addr, dst_addr, out_size);
	if (err) {
		ret = -EPERM;
		goto error;
	}

	/* Invalidate output buffer */
	invalidate_dcache_range((ulong)dst_ptr, (ulong)(dst_ptr +
			roundup(out_size, ARCH_DMA_MINALIGN)));

	printf("DEK Blob\n");
	for (i = 0; i < DEK_BLOB_HDR_SIZE + BLOB_SIZE(len / 8); i++)
		printf("%02X", dst_ptr[i]);
	printf("\n");

error:
	/* Remove memory region permission to SECO */
	err = sc_rm_set_memreg_permissions(-1, mr_input, SECO_PT,
					   SC_RM_PERM_NONE);
	if (err) {
		printf("Error: remove permission failed for input\n");
		ret = -EPERM;
	}

	err = sc_rm_set_memreg_permissions(-1, mr_output, SECO_PT,
					   SC_RM_PERM_NONE);
	if (err) {
		printf("Error: remove permission failed for output\n");
		ret = -EPERM;
	}

	return ret;
}
#endif /* CONFIG_IMX_SECO_DEK_ENCAP */

#ifdef CONFIG_IMX_ELE_DEK_ENCAP

#define DEK_BLOB_HDR_SIZE 8
#define AHAB_PRIVATE_KEY 0x81
#define AHAB_DEK_BLOB	 0x01
#define AHAB_ALG_AES	 0x03
#define AHAB_128_AES_KEY 0x10
#define AHAB_192_AES_KEY 0x18
#define AHAB_256_AES_KEY 0x20

static int blob_encap_dek(u32 src_addr, u32 dst_addr, u32 len)
{
	u8 in_size, out_size;
	u8 *src_ptr, *dst_ptr;
	struct generate_key_blob_hdr hdr;

	/* Set sizes */
	in_size = sizeof(struct generate_key_blob_hdr) + len / 8;
	out_size = BLOB_SIZE(len / 8) + DEK_BLOB_HDR_SIZE;

	/* Get src and dst virtual addresses */
	src_ptr = map_sysmem(src_addr, in_size);
	dst_ptr = map_sysmem(dst_addr, out_size);

	/* Check addr input */
	if (!(src_ptr && dst_ptr)) {
		debug("src_addr or dst_addr invalid\n");
		return -1;
	}

	/* Build key header */
	hdr.version = 0x0;
	hdr.length_lsb = in_size;
	hdr.length_msb = 0x00;
	hdr.tag = AHAB_PRIVATE_KEY;
	hdr.flags = AHAB_DEK_BLOB;
	hdr.algorithm = AHAB_ALG_AES;
	hdr.mode = 0x0; /* Not used by the ELE */

	switch (len) {
	case 128:
		hdr.size = AHAB_128_AES_KEY;
		break;
	case 192:
		hdr.size = AHAB_192_AES_KEY;
		break;
	case 256:
		hdr.size = AHAB_256_AES_KEY;
		break;
	default:
		/* Not supported */
		debug("Invalid DEK size. Valid sizes are 128, 192 and 256b\n");
		return -1;
	}

	/* Move input key and append blob header */
	memmove((void *)(src_ptr + sizeof(struct generate_key_blob_hdr)),
		(void *)src_ptr, len / 8);
	memcpy((void *)src_ptr, (void *)&hdr,
	       sizeof(struct generate_key_blob_hdr));

	/* Flush the cache */
	flush_dcache_range(src_addr, src_addr + in_size);
	flush_dcache_range((ulong)dst_ptr, (ulong)(dst_ptr +
			roundup(out_size, ARCH_DMA_MINALIGN)));

	/* Call ELE */
	if (ele_generate_dek_blob(0x00, src_addr, dst_addr, out_size))
		return -1;

	/* Invalidate output buffer */
	invalidate_dcache_range((ulong)dst_ptr, (ulong)(dst_ptr +
			roundup(out_size, ARCH_DMA_MINALIGN)));

	return 0;
}
#endif /* CONFIG_IMX_ELE_DEK_ENCAP */

/**
 * do_dek_blob() - Handle the "dek_blob" command-line command
 * @cmdtp:  Command data struct pointer
 * @flag:   Command flag
 * @argc:   Command-line argument count
 * @argv:   Array of command-line arguments
 *
 * Returns zero on success, CMD_RET_USAGE in case of misuse and negative
 * on error.
 */
static int do_dek_blob(struct cmd_tbl *cmdtp, int flag, int argc,
		       char *const argv[])
{
	uint32_t src_addr, dst_addr, len;

	if (argc != 4)
		return CMD_RET_USAGE;

	src_addr = hextoul(argv[1], NULL);
	dst_addr = hextoul(argv[2], NULL);
	len = dectoul(argv[3], NULL);

	return blob_encap_dek(src_addr, dst_addr, len);
}

/***************************************************/
static char dek_blob_help_text[] =
	"src dst len            - Encapsulate and create blob of data\n"
	"                         $len bits long at address $src and\n"
	"                         store the result at address $dst.\n";

U_BOOT_CMD(
	dek_blob, 4, 1, do_dek_blob,
	"Data Encryption Key blob encapsulation",
	dek_blob_help_text
);
