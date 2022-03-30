// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2003
 * Steven Scholz, imc Measurement & Control, steven.scholz@imc-berlin.de
 *
 * (C) Copyright 2002
 * Rich Ireland, Enterasys Networks, rireland@enterasys.com.
 */

/*
 *  Altera FPGA support
 */
#include <common.h>
#include <errno.h>
#include <ACEX1K.h>
#include <stratixII.h>

/* Define FPGA_DEBUG to 1 to get debug printf's */
#define FPGA_DEBUG	0

static const struct altera_fpga {
	enum altera_family	family;
	const char		*name;
	int			(*load)(Altera_desc *, const void *, size_t);
	int			(*dump)(Altera_desc *, const void *, size_t);
	int			(*info)(Altera_desc *);
} altera_fpga[] = {
#if defined(CONFIG_FPGA_ACEX1K)
	{ Altera_ACEX1K, "ACEX1K", ACEX1K_load, ACEX1K_dump, ACEX1K_info },
	{ Altera_CYC2,   "ACEX1K", ACEX1K_load, ACEX1K_dump, ACEX1K_info },
#elif defined(CONFIG_FPGA_CYCLON2)
	{ Altera_ACEX1K, "CycloneII", CYC2_load, CYC2_dump, CYC2_info },
	{ Altera_CYC2,   "CycloneII", CYC2_load, CYC2_dump, CYC2_info },
#endif
#if defined(CONFIG_FPGA_STRATIX_II)
	{ Altera_StratixII, "StratixII", StratixII_load,
	  StratixII_dump, StratixII_info },
#endif
#if defined(CONFIG_FPGA_STRATIX_V)
	{ Altera_StratixV, "StratixV", stratixv_load, NULL, NULL },
#endif
#if defined(CONFIG_FPGA_STRATIX10)
	{ Intel_FPGA_Stratix10, "Stratix10", stratix10_load, NULL, NULL },
#endif
#if defined(CONFIG_FPGA_SOCFPGA)
	{ Altera_SoCFPGA, "SoC FPGA", socfpga_load, NULL, NULL },
#endif
};

static int altera_validate(Altera_desc *desc, const char *fn)
{
	if (!desc) {
		printf("%s: NULL descriptor!\n", fn);
		return -EINVAL;
	}

	if ((desc->family < min_altera_type) ||
	    (desc->family > max_altera_type)) {
		printf("%s: Invalid family type, %d\n", fn, desc->family);
		return -EINVAL;
	}

	if ((desc->iface < min_altera_iface_type) ||
	    (desc->iface > max_altera_iface_type)) {
		printf("%s: Invalid Interface type, %d\n", fn, desc->iface);
		return -EINVAL;
	}

	if (!desc->size) {
		printf("%s: NULL part size\n", fn);
		return -EINVAL;
	}

	return 0;
}

static const struct altera_fpga *
altera_desc_to_fpga(Altera_desc *desc, const char *fn)
{
	int i;

	if (altera_validate(desc, fn)) {
		printf("%s: Invalid device descriptor\n", fn);
		return NULL;
	}

	for (i = 0; i < ARRAY_SIZE(altera_fpga); i++) {
		if (desc->family == altera_fpga[i].family)
			break;
	}

	if (i == ARRAY_SIZE(altera_fpga)) {
		printf("%s: Unsupported family type, %d\n", fn, desc->family);
		return NULL;
	}

	return &altera_fpga[i];
}

int altera_load(Altera_desc *desc, const void *buf, size_t bsize)
{
	const struct altera_fpga *fpga = altera_desc_to_fpga(desc, __func__);

	if (!fpga)
		return FPGA_FAIL;

	debug_cond(FPGA_DEBUG, "%s: Launching the %s Loader...\n",
		   __func__, fpga->name);
	if (fpga->load)
		return fpga->load(desc, buf, bsize);
	return 0;
}

int altera_dump(Altera_desc *desc, const void *buf, size_t bsize)
{
	const struct altera_fpga *fpga = altera_desc_to_fpga(desc, __func__);

	if (!fpga)
		return FPGA_FAIL;

	debug_cond(FPGA_DEBUG, "%s: Launching the %s Reader...\n",
		   __func__, fpga->name);
	if (fpga->dump)
		return fpga->dump(desc, buf, bsize);
	return 0;
}

int altera_info(Altera_desc *desc)
{
	const struct altera_fpga *fpga = altera_desc_to_fpga(desc, __func__);

	if (!fpga)
		return FPGA_FAIL;

	printf("Family:        \t%s\n", fpga->name);

	printf("Interface type:\t");
	switch (desc->iface) {
	case passive_serial:
		printf("Passive Serial (PS)\n");
		break;
	case passive_parallel_synchronous:
		printf("Passive Parallel Synchronous (PPS)\n");
		break;
	case passive_parallel_asynchronous:
		printf("Passive Parallel Asynchronous (PPA)\n");
		break;
	case passive_serial_asynchronous:
		printf("Passive Serial Asynchronous (PSA)\n");
		break;
	case altera_jtag_mode:		/* Not used */
		printf("JTAG Mode\n");
		break;
	case fast_passive_parallel:
		printf("Fast Passive Parallel (FPP)\n");
		break;
	case fast_passive_parallel_security:
		printf("Fast Passive Parallel with Security (FPPS)\n");
		break;
	case secure_device_manager_mailbox:
		puts("Secure Device Manager (SDM) Mailbox\n");
		break;
		/* Add new interface types here */
	default:
		printf("Unsupported interface type, %d\n", desc->iface);
	}

	printf("Device Size:   \t%zd bytes\n"
	       "Cookie:        \t0x%x (%d)\n",
	       desc->size, desc->cookie, desc->cookie);

	if (desc->iface_fns) {
		printf("Device Function Table @ 0x%p\n", desc->iface_fns);
		if (fpga->info)
			fpga->info(desc);
	} else {
		printf("No Device Function Table.\n");
	}

	return FPGA_SUCCESS;
}
