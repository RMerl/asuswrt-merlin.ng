// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2016 Freescale Semiconductor, Inc.
 * Copyright 2017 NXP
 *
 * These commands enable the use of the CAAM MPPubK-generation and MPSign
 * functions in supported i.MX devices.
 */

#include <asm/byteorder.h>
#include <asm/arch/clock.h>
#include <linux/compiler.h>
#include <command.h>
#include <common.h>
#include <env.h>
#include <fsl_sec.h>
#include <mapmem.h>
#include <memalign.h>

/**
 * do_mfgprot() - Handle the "mfgprot" command-line command
 * @cmdtp:  Command data struct pointer
 * @flag:   Command flag
 * @argc:   Command-line argument count
 * @argv:   Array of command-line arguments
 *
 * Returns zero on success, CMD_RET_USAGE in case of misuse and negative
 * on error.
 */
static int do_mfgprot(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	u8 *m_ptr, *dgst_ptr, *c_ptr, *d_ptr, *dst_ptr;
	char *pubk, *sign, *sel;
	int m_size, i, ret;
	u32 m_addr;

	pubk = "pubk";
	sign = "sign";
	sel = argv[1];

	/* Enable HAB clock */
	hab_caam_clock_enable(1);

	u32 out_jr_size = sec_in32(CFG_SYS_FSL_JR0_ADDR +
				   FSL_CAAM_ORSR_JRa_OFFSET);

	if (out_jr_size != FSL_CAAM_MAX_JR_SIZE)
		sec_init();

	if (strcmp(sel, pubk) == 0) {
		dst_ptr = malloc_cache_aligned(FSL_CAAM_MP_PUBK_BYTES);
		if (!dst_ptr)
			return -ENOMEM;

		ret = gen_mppubk(dst_ptr);
		if (ret) {
			free(dst_ptr);
			return ret;
		}

		/* Output results */
		puts("Public key:\n");
		for (i = 0; i < FSL_CAAM_MP_PUBK_BYTES; i++)
			printf("%02X", (dst_ptr)[i]);
		puts("\n");
		free(dst_ptr);

	} else if (strcmp(sel, sign) == 0) {
		if (argc != 4)
			return CMD_RET_USAGE;

		m_addr = hextoul(argv[2], NULL);
		m_size = dectoul(argv[3], NULL);
		m_ptr = map_physmem(m_addr, m_size, MAP_NOCACHE);
		if (!m_ptr)
			return -ENOMEM;

		dgst_ptr = malloc_cache_aligned(FSL_CAAM_MP_MES_DGST_BYTES);
		if (!dgst_ptr) {
			ret = -ENOMEM;
			goto free_m;
		}

		c_ptr = malloc_cache_aligned(FSL_CAAM_MP_PRVK_BYTES);
		if (!c_ptr) {
			ret = -ENOMEM;
			goto free_dgst;
		}

		d_ptr = malloc_cache_aligned(FSL_CAAM_MP_PRVK_BYTES);
		if (!d_ptr) {
			ret = -ENOMEM;
			goto free_c;
		}

		ret = sign_mppubk(m_ptr, m_size, dgst_ptr, c_ptr, d_ptr);
		if (ret)
			goto free_d;

		/* Output results */
		puts("Message: ");
		for (i = 0; i < m_size; i++)
			printf("%02X ", (m_ptr)[i]);
		puts("\n");

		puts("Message Representative Digest(SHA-256):\n");
		for (i = 0; i < FSL_CAAM_MP_MES_DGST_BYTES; i++)
			printf("%02X", (dgst_ptr)[i]);
		puts("\n");

		puts("Signature:\n");
		puts("C:\n");
		for (i = 0; i < FSL_CAAM_MP_PRVK_BYTES; i++)
			printf("%02X", (c_ptr)[i]);
		puts("\n");

		puts("d:\n");
		for (i = 0; i < FSL_CAAM_MP_PRVK_BYTES; i++)
			printf("%02X", (d_ptr)[i]);
		puts("\n");
free_d:
	free(d_ptr);
free_c:
	free(c_ptr);
free_dgst:
	free(dgst_ptr);
free_m:
	unmap_sysmem(m_ptr);

	} else {
		return CMD_RET_USAGE;
	}
	return ret;
}

/***************************************************/
static char mfgprot_help_text[] =
	"Usage:\n"
	 "Print the public key for Manufacturing Protection\n"
	 "\tmfgprot pubk\n"
	 "Generates a Manufacturing Protection signature\n"
	 "\tmfgprot sign <data_addr> <size>";

U_BOOT_CMD(
	mfgprot, 4, 1, do_mfgprot,
	"Manufacturing Protection\n",
	mfgprot_help_text
);
