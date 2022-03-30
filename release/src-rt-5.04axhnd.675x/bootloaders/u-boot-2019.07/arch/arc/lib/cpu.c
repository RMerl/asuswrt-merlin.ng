// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2013-2014, 2018 Synopsys, Inc. All rights reserved.
 */

#include <common.h>
#include <malloc.h>
#include <asm/arcregs.h>
#include <asm/cache.h>

DECLARE_GLOBAL_DATA_PTR;

int arch_cpu_init(void)
{
	timer_init();

	gd->cpu_clk = CONFIG_SYS_CLK_FREQ;
	gd->ram_size = CONFIG_SYS_SDRAM_SIZE;

	cache_init();

	return 0;
}

int arch_early_init_r(void)
{
	gd->bd->bi_memstart = CONFIG_SYS_SDRAM_BASE;
	gd->bd->bi_memsize = CONFIG_SYS_SDRAM_SIZE;
	return 0;
}

/* This is a dummy function on arc */
int dram_init(void)
{
	return 0;
}

#ifdef CONFIG_DISPLAY_CPUINFO
const char *arc_700_version(int arcver, char *name, int name_len)
{
	const char *arc_ver;

	switch (arcver) {
	case 0x32:
		arc_ver = "v4.4-4.5";
		break;
	case 0x33:
		arc_ver = "v4.6-v4.9";
		break;
	case 0x34:
		arc_ver = "v4.10";
		break;
	case 0x35:
		arc_ver = "v4.11";
		break;
	default:
		arc_ver = "unknown version";
	}

	snprintf(name, name_len, "ARC 700 %s", arc_ver);

	return name;
}

struct em_template_t {
	const bool cache;
	const bool dsp;
	const bool xymem;
	const char name[8];
};

static const struct em_template_t em_versions[] = {
	{false,	false,	false,	"EM4"},
	{true,	false,	false,	"EM6"},
	{false,	true,	false,	"EM5D"},
	{true,	true,	false,	"EM7D"},
	{false,	true,	true,	"EM9D"},
	{true,	true,	true,	"EM11D"},
};

const char *arc_em_version(int arcver, char *name, int name_len)
{
	const char *arc_name = "EM";
	const char *arc_ver;
	bool cache = ARC_FEATURE_EXISTS(ARC_BCR_IC_BUILD);
	bool dsp = ARC_FEATURE_EXISTS(ARC_AUX_DSP_BUILD);
	bool xymem = ARC_FEATURE_EXISTS(ARC_AUX_XY_BUILD);
	int i;

	for (i = 0; i < sizeof(em_versions) / sizeof(struct em_template_t); i++) {
		if (em_versions[i].cache == cache &&
		    em_versions[i].dsp == dsp &&
		    em_versions[i].xymem == xymem) {
			arc_name = em_versions[i].name;
			break;
		}
	}

	switch (arcver) {
	case 0x41:
		arc_ver = "v1.1a";
		break;
	case 0x42:
		arc_ver = "v3.0";
		break;
	case 0x43:
		arc_ver = "v4.0";
		break;
	case 0x44:
		arc_ver = "v5.0";
		break;
	default:
		arc_ver = "unknown version";
	}

	snprintf(name, name_len, "ARC %s %s", arc_name, arc_ver);

	return name;
}

struct hs_template_t {
	const bool cache;
	const bool mmu;
	const bool dual_issue;
	const bool dsp;
	const char name[8];
};

static const struct hs_template_t hs_versions[] = {
	{false,	false,	false,	false,	"HS34"},
	{true,	false,	false,	false,	"HS36"},
	{true,	true,	false,	false,	"HS38"},
	{false,	false,	true,	false,	"HS44"},
	{true,	false,	true,	false,	"HS46"},
	{true,	true,	true,	false,	"HS48"},
	{false,	false,	true,	true,	"HS45D"},
	{true,	false,	true,	true,	"HS47D"},
};

const char *arc_hs_version(int arcver, char *name, int name_len)
{
	const char *arc_name = "HS";
	const char *arc_ver;
	bool cache = ARC_FEATURE_EXISTS(ARC_BCR_IC_BUILD);
	bool dsp = ARC_FEATURE_EXISTS(ARC_AUX_DSP_BUILD);
	bool mmu = !!read_aux_reg(ARC_AUX_MMU_BCR);
	bool dual_issue = arcver == 0x54 ? true : false;
	int i;

	for (i = 0; i < sizeof(hs_versions) / sizeof(struct hs_template_t); i++) {
		if (hs_versions[i].cache == cache &&
		    hs_versions[i].mmu == mmu &&
		    hs_versions[i].dual_issue == dual_issue &&
		    hs_versions[i].dsp == dsp) {
			arc_name = hs_versions[i].name;
			break;
		}
	}

	switch (arcver) {
	case 0x50:
		arc_ver = "v1.0";
		break;
	case 0x51:
		arc_ver = "v2.0";
		break;
	case 0x52:
		arc_ver = "v2.1c";
		break;
	case 0x53:
		arc_ver = "v3.0";
		break;
	case 0x54:
		arc_ver = "v4.0";
		break;
	default:
		arc_ver = "unknown version";
	}

	snprintf(name, name_len, "ARC %s %s", arc_name, arc_ver);

	return name;
}

const char *decode_identity(void)
{
#define MAX_CPU_NAME_LEN	64

	int arcver = read_aux_reg(ARC_AUX_IDENTITY) & 0xff;
	char *name = malloc(MAX_CPU_NAME_LEN);

	if (arcver >= 0x50)
		return arc_hs_version(arcver, name, MAX_CPU_NAME_LEN);
	else if (arcver >= 0x40)
		return arc_em_version(arcver, name, MAX_CPU_NAME_LEN);
	else if (arcver >= 0x30)
		return arc_700_version(arcver, name, MAX_CPU_NAME_LEN);
	else
		return "Unknown ARC core";
}

const char *decode_subsystem(void)
{
	int subsys_type = read_aux_reg(ARC_AUX_SUBSYS_BUILD) & GENMASK(3, 0);

	switch (subsys_type) {
	case 0: return NULL;
	case 2: return "ARC Sensor & Control IP Subsystem";
	case 3: return "ARC Data Fusion IP Subsystem";
	case 4: return "ARC Secure Subsystem";
	default: return "Unknown subsystem";
	};
}

__weak int print_cpuinfo(void)
{
	const char *subsys_name = decode_subsystem();
	char mhz[8];

	printf("CPU:   %s at %s MHz\n", decode_identity(),
	       strmhz(mhz, gd->cpu_clk));

	if (subsys_name)
		printf("Subsys:%s\n", subsys_name);

	return 0;
}
#endif /* CONFIG_DISPLAY_CPUINFO */
