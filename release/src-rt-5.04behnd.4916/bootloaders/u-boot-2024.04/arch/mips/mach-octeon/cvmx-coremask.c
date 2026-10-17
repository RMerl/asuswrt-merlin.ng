// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2018-2020 Marvell International Ltd.
 */

#include <env.h>
#include <errno.h>

#include <linux/compat.h>
#include <linux/ctype.h>

#include <mach/cvmx-regs.h>
#include <mach/cvmx-coremask.h>
#include <mach/cvmx-fuse.h>
#include <mach/octeon-model.h>
#include <mach/octeon-feature.h>
#include <mach/cvmx-ciu-defs.h>

struct cvmx_coremask *get_coremask_override(struct cvmx_coremask *pcm)
{
	struct cvmx_coremask pcm_override = CVMX_COREMASK_MAX;
	char *cptr;

	/* The old code sets the number of cores to be to 16 in this case. */
	cvmx_coremask_set_cores(pcm, 0, 16);

	if (OCTEON_IS_OCTEON2() || OCTEON_IS_OCTEON3())
		cvmx_coremask_copy(pcm, &pcm_override);

	cptr = env_get("coremask_override");
	if (cptr) {
		if (cvmx_coremask_str2bmp(pcm, cptr) < 0)
			return NULL;
	}

	return pcm;
}

/* Validate the coremask that is passed to a boot* function. */
int validate_coremask(struct cvmx_coremask *pcm)
{
	struct cvmx_coremask coremask_override;
	struct cvmx_coremask fuse_coremask;

	if (!get_coremask_override(&coremask_override))
		return -1;

	octeon_get_available_coremask(&fuse_coremask);

	if (!cvmx_coremask_is_subset(&fuse_coremask, pcm)) {
		puts("ERROR: Can't boot cores that don't exist!\n");
		puts("Available coremask:\n");
		cvmx_coremask_print(&fuse_coremask);
		return -1;
	}

	if (!cvmx_coremask_is_subset(&coremask_override, pcm)) {
		struct cvmx_coremask print_cm;

		puts("Notice: coremask changed from:\n");
		cvmx_coremask_print(pcm);
		puts("based on coremask_override of:\n");
		cvmx_coremask_print(&coremask_override);
		cvmx_coremask_and(&print_cm, pcm, &coremask_override);
		puts("to:\n");
		cvmx_coremask_print(&print_cm);
	}

	return 0;
}

/**
 * In CIU_FUSE for the 78XX, odd and even cores are separated out.
 * For example, a CIU_FUSE value of 0xfffffefffffe indicates that bits 0 and 1
 * are set.
 * This function converts the bit number in the CIU_FUSE register to a
 * physical core number.
 */
static int convert_ciu_fuse_to_physical_core(int core, int max_cores)
{
	if (!octeon_has_feature(OCTEON_FEATURE_CIU3))
		return core;
	else if (!OCTEON_IS_MODEL(OCTEON_CN78XX))
		return core;
	else if (core < (max_cores / 2))
		return core * 2;
	else
		return ((core - (max_cores / 2)) * 2) + 1;
}

/**
 * Get the total number of fuses blown as well as the number blown per tad.
 *
 * @param	coremask	fuse coremask
 * @param[out]	tad_blown_count	number of cores blown for each tad
 * @param	num_tads	number of tads
 * @param	max_cores	maximum number of cores
 *
 * Return:	void
 */
void fill_tad_corecount(u64 coremask, int tad_blown_count[], int num_tads,
			int max_cores)
{
	int core, physical_core;

	for (core = 0; core < max_cores; core++) {
		if (!(coremask & (1ULL << core))) {
			int tad;

			physical_core =
				convert_ciu_fuse_to_physical_core(core,
								  max_cores);
			tad = physical_core % num_tads;
			tad_blown_count[tad]++;
		}
	}
}

u64 get_core_pattern(int num_tads, int max_cores)
{
	u64 pattern = 1ULL;
	int cnt;

	for (cnt = 1; cnt < (max_cores / num_tads); cnt++)
		pattern |= pattern << num_tads;

	return pattern;
}

/**
 * For CN78XX and CN68XX this function returns the logical coremask from the
 * CIU_FUSE register value. For other models there is no difference.
 *
 * @param ciu_fuse_value	fuse value from CIU_FUSE register
 * Return: logical coremask of CIU_FUSE value.
 */
u64 get_logical_coremask(u64 ciu_fuse_value)
{
	int tad_blown_count[MAX_CORE_TADS] = {0};
	int tad;
	u64 logical_coremask = 0;
	u64 tad_mask, pattern;
	int num_tads, max_cores;

	if (OCTEON_IS_MODEL(OCTEON_CN78XX)) {
		num_tads = 8;
		max_cores = 48;
	} else if (OCTEON_IS_MODEL(OCTEON_CN73XX) ||
		   OCTEON_IS_MODEL(OCTEON_CNF75XX)) {
		num_tads = 4;
		max_cores = 16;
	} else if (OCTEON_IS_MODEL(OCTEON_CN68XX)) {
		num_tads = 4;
		max_cores = 32;
	} else {
		/* Most Octeon devices don't need any mapping. */
		return ciu_fuse_value;
	}

	pattern = get_core_pattern(num_tads, max_cores);
	fill_tad_corecount(ciu_fuse_value, tad_blown_count,
			   num_tads, max_cores);

	for (tad = 0; tad < num_tads; tad++) {
		tad_mask = pattern << tad;
		logical_coremask |= tad_mask >> (tad_blown_count[tad] * num_tads);
	}
	return logical_coremask;
}

/**
 * Returns the available coremask either from env or fuses.
 * If the fuses are blown and locked, they are the definitive coremask.
 *
 * @param pcm	pointer to coremask to fill in
 * Return: pointer to coremask
 */
struct cvmx_coremask *octeon_get_available_coremask(struct cvmx_coremask *pcm)
{
	u8 node_mask = 0x01;	/* ToDo: Currently only one node is supported */
	u64 ciu_fuse;
	u64 cores;

	cvmx_coremask_clear_all(pcm);

	if (octeon_has_feature(OCTEON_FEATURE_CIU3)) {
		int node;

		cvmx_coremask_for_each_node(node, node_mask) {
			ciu_fuse = (csr_rd(CVMX_CIU_FUSE) &
				    0x0000FFFFFFFFFFFFULL);

			ciu_fuse = get_logical_coremask(ciu_fuse);
			cvmx_coremask_set64_node(pcm, node, ciu_fuse);
		}

		return pcm;
	}

	ciu_fuse = (csr_rd(CVMX_CIU_FUSE) & 0x0000FFFFFFFFFFFFULL);
	ciu_fuse = get_logical_coremask(ciu_fuse);

	if (OCTEON_IS_MODEL(OCTEON_CN68XX))
		cvmx_coremask_set64(pcm, ciu_fuse);

	/* Get number of cores from fuse register, convert to coremask */
	cores = __builtin_popcountll(ciu_fuse);

	cvmx_coremask_set_cores(pcm, 0, cores);

	return pcm;
}

int cvmx_coremask_str2bmp(struct cvmx_coremask *pcm, char *hexstr)
{
	int i, j;
	int l;		/* length of the hexstr in characters */
	int lb;		/* number of bits taken by hexstr */
	int hldr_offset;/* holder's offset within the coremask */
	int hldr_xsz;	/* holder's size in the number of hex digits */
	u64 h;
	char c;

#define MINUS_ONE (hexstr[0] == '-' && hexstr[1] == '1' && hexstr[2] == 0)
	if (MINUS_ONE) {
		cvmx_coremask_set_all(pcm);
		return 0;
	}

	/* Skip '0x' from hexstr */
	if (hexstr[0] == '0' && (hexstr[1] == 'x' || hexstr[1] == 'X'))
		hexstr += 2;

	if (!strlen(hexstr)) {
		printf("%s: Error: hex string is empty\n", __func__);
		return -2;
	}

	/* Trim leading zeros */
	while (*hexstr == '0')
		hexstr++;

	cvmx_coremask_clear_all(pcm);
	l = strlen(hexstr);

	/* If length is 0 then the hex string must be all zeros */
	if (l == 0)
		return 0;

	for (i = 0; i < l; i++) {
		if (isxdigit((int)hexstr[i]) == 0) {
			printf("%s: Non-hex digit within hexstr\n", __func__);
			return -2;
		}
	}

	lb = (l - 1) * 4;
	if (hexstr[0] > '7')
		lb += 4;
	else if (hexstr[0] > '3')
		lb += 3;
	else if (hexstr[0] > '1')
		lb += 2;
	else
		lb += 1;
	if (lb > CVMX_MIPS_MAX_CORES) {
		printf("%s: hexstr (%s) is too long\n", __func__, hexstr);
		return -1;
	}

	hldr_offset = 0;
	hldr_xsz = 2 * sizeof(u64);
	for (i = l; i > 0; i -= hldr_xsz) {
		c = hexstr[i];
		hexstr[i] = 0;
		j = i - hldr_xsz;
		if (j < 0)
			j = 0;
		h = simple_strtoull(&hexstr[j], NULL, 16);
		if (errno == EINVAL) {
			printf("%s: strtou returns w/ EINVAL\n", __func__);
			return -2;
		}
		pcm->coremask_bitmap[hldr_offset] = h;
		hexstr[i] = c;
		hldr_offset++;
	}

	return 0;
}

void cvmx_coremask_print(const struct cvmx_coremask *pcm)
{
	int i, j;
	int start;
	int found = 0;

	/*
	 * Print one node per line. Since the bitmap is stored LSB to MSB
	 * we reverse the order when printing.
	 */
	if (!octeon_has_feature(OCTEON_FEATURE_MULTINODE)) {
		start = 0;
		for (j = CVMX_COREMASK_MAX_CORES_PER_NODE -
			     CVMX_COREMASK_HLDRSZ;
		     j >= 0; j -= CVMX_COREMASK_HLDRSZ) {
			if (pcm->coremask_bitmap[j / CVMX_COREMASK_HLDRSZ] != 0)
				start = 1;
			if (start) {
				printf(" 0x%llx",
				       (u64)pcm->coremask_bitmap[j /
						CVMX_COREMASK_HLDRSZ]);
			}
		}

		if (start)
			found = 1;

		/*
		 * If the coremask is empty print <EMPTY> so it is not
		 * confusing
		 */
		if (!found)
			printf("<EMPTY>");
		printf("\n");

		return;
	}

	for (i = 0; i < CVMX_MAX_USED_CORES_BMP;
	     i += CVMX_COREMASK_MAX_CORES_PER_NODE) {
		printf("%s  node %d:", i > 0 ? "\n" : "",
		       cvmx_coremask_core_to_node(i));
		start = 0;

		for (j = i + CVMX_COREMASK_MAX_CORES_PER_NODE -
			     CVMX_COREMASK_HLDRSZ;
		     j >= i;
		     j -= CVMX_COREMASK_HLDRSZ) {
			/* Don't start printing until we get a non-zero word. */
			if (pcm->coremask_bitmap[j / CVMX_COREMASK_HLDRSZ] != 0)
				start = 1;

			if (start) {
				printf(" 0x%llx", (u64)pcm->coremask_bitmap[j /
							CVMX_COREMASK_HLDRSZ]);
			}
		}

		if (start)
			found = 1;
	}

	i /= CVMX_COREMASK_HLDRSZ;
	for (; i < CVMX_COREMASK_BMPSZ; i++) {
		if (pcm->coremask_bitmap[i]) {
			printf("  EXTRA GARBAGE[%i]: %016llx\n", i,
			       (u64)pcm->coremask_bitmap[i]);
		}
	}

	/* If the coremask is empty print <EMPTY> so it is not confusing */
	if (!found)
		printf("<EMPTY>");

	printf("\n");
}
