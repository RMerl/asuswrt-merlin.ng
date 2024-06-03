// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2009-2012 Freescale Semiconductor, Inc.
 *
 * This file is derived from arch/powerpc/cpu/mpc85xx/cpu.c and
 * arch/powerpc/cpu/mpc86xx/cpu.c. Basically this file contains
 * cpu specific common code for 85xx/86xx processors.
 */

#include <config.h>
#include <common.h>
#include <command.h>
#include <tsec.h>
#include <fm_eth.h>
#include <netdev.h>
#include <asm/cache.h>
#include <asm/io.h>
#include <vsc9953.h>

DECLARE_GLOBAL_DATA_PTR;

static struct cpu_type cpu_type_list[] = {
#if defined(CONFIG_MPC85xx)
	CPU_TYPE_ENTRY(8533, 8533, 1),
	CPU_TYPE_ENTRY(8535, 8535, 1),
	CPU_TYPE_ENTRY(8536, 8536, 1),
	CPU_TYPE_ENTRY(8540, 8540, 1),
	CPU_TYPE_ENTRY(8541, 8541, 1),
	CPU_TYPE_ENTRY(8543, 8543, 1),
	CPU_TYPE_ENTRY(8544, 8544, 1),
	CPU_TYPE_ENTRY(8545, 8545, 1),
	CPU_TYPE_ENTRY(8547, 8547, 1),
	CPU_TYPE_ENTRY(8548, 8548, 1),
	CPU_TYPE_ENTRY(8555, 8555, 1),
	CPU_TYPE_ENTRY(8560, 8560, 1),
	CPU_TYPE_ENTRY(8567, 8567, 1),
	CPU_TYPE_ENTRY(8568, 8568, 1),
	CPU_TYPE_ENTRY(8569, 8569, 1),
	CPU_TYPE_ENTRY(8572, 8572, 2),
	CPU_TYPE_ENTRY(P1010, P1010, 1),
	CPU_TYPE_ENTRY(P1011, P1011, 1),
	CPU_TYPE_ENTRY(P1012, P1012, 1),
	CPU_TYPE_ENTRY(P1013, P1013, 1),
	CPU_TYPE_ENTRY(P1014, P1014, 1),
	CPU_TYPE_ENTRY(P1017, P1017, 1),
	CPU_TYPE_ENTRY(P1020, P1020, 2),
	CPU_TYPE_ENTRY(P1021, P1021, 2),
	CPU_TYPE_ENTRY(P1022, P1022, 2),
	CPU_TYPE_ENTRY(P1023, P1023, 2),
	CPU_TYPE_ENTRY(P1024, P1024, 2),
	CPU_TYPE_ENTRY(P1025, P1025, 2),
	CPU_TYPE_ENTRY(P2010, P2010, 1),
	CPU_TYPE_ENTRY(P2020, P2020, 2),
	CPU_TYPE_ENTRY(P2040, P2040, 4),
	CPU_TYPE_ENTRY(P2041, P2041, 4),
	CPU_TYPE_ENTRY(P3041, P3041, 4),
	CPU_TYPE_ENTRY(P4040, P4040, 4),
	CPU_TYPE_ENTRY(P4080, P4080, 8),
	CPU_TYPE_ENTRY(P5010, P5010, 1),
	CPU_TYPE_ENTRY(P5020, P5020, 2),
	CPU_TYPE_ENTRY(P5021, P5021, 2),
	CPU_TYPE_ENTRY(P5040, P5040, 4),
	CPU_TYPE_ENTRY(T4240, T4240, 0),
	CPU_TYPE_ENTRY(T4120, T4120, 0),
	CPU_TYPE_ENTRY(T4160, T4160, 0),
	CPU_TYPE_ENTRY(T4080, T4080, 4),
	CPU_TYPE_ENTRY(B4860, B4860, 0),
	CPU_TYPE_ENTRY(G4860, G4860, 0),
	CPU_TYPE_ENTRY(B4440, B4440, 0),
	CPU_TYPE_ENTRY(B4460, B4460, 0),
	CPU_TYPE_ENTRY(G4440, G4440, 0),
	CPU_TYPE_ENTRY(B4420, B4420, 0),
	CPU_TYPE_ENTRY(B4220, B4220, 0),
	CPU_TYPE_ENTRY(T1040, T1040, 0),
	CPU_TYPE_ENTRY(T1041, T1041, 0),
	CPU_TYPE_ENTRY(T1042, T1042, 0),
	CPU_TYPE_ENTRY(T1020, T1020, 0),
	CPU_TYPE_ENTRY(T1021, T1021, 0),
	CPU_TYPE_ENTRY(T1022, T1022, 0),
	CPU_TYPE_ENTRY(T1024, T1024, 0),
	CPU_TYPE_ENTRY(T1023, T1023, 0),
	CPU_TYPE_ENTRY(T1014, T1014, 0),
	CPU_TYPE_ENTRY(T1013, T1013, 0),
	CPU_TYPE_ENTRY(T2080, T2080, 0),
	CPU_TYPE_ENTRY(T2081, T2081, 0),
	CPU_TYPE_ENTRY(BSC9130, 9130, 1),
	CPU_TYPE_ENTRY(BSC9131, 9131, 1),
	CPU_TYPE_ENTRY(BSC9132, 9132, 2),
	CPU_TYPE_ENTRY(BSC9232, 9232, 2),
	CPU_TYPE_ENTRY(C291, C291, 1),
	CPU_TYPE_ENTRY(C292, C292, 1),
	CPU_TYPE_ENTRY(C293, C293, 1),
#elif defined(CONFIG_MPC86xx)
	CPU_TYPE_ENTRY(8610, 8610, 1),
	CPU_TYPE_ENTRY(8641, 8641, 2),
	CPU_TYPE_ENTRY(8641D, 8641D, 2),
#endif
};

#ifdef CONFIG_SYS_FSL_QORIQ_CHASSIS2
static inline u32 init_type(u32 cluster, int init_id)
{
	ccsr_gur_t *gur = (void __iomem *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	u32 idx = (cluster >> (init_id * 8)) & TP_CLUSTER_INIT_MASK;
	u32 type = in_be32(&gur->tp_ityp[idx]);

	if (type & TP_ITYP_AV)
		return type;

	return 0;
}

u32 compute_ppc_cpumask(void)
{
	ccsr_gur_t *gur = (void __iomem *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	int i = 0, count = 0;
	u32 cluster, type, mask = 0;

	do {
		int j;
		cluster = in_be32(&gur->tp_cluster[i].lower);
		for (j = 0; j < TP_INIT_PER_CLUSTER; j++) {
			type = init_type(cluster, j);
			if (type) {
				if (TP_ITYP_TYPE(type) == TP_ITYP_TYPE_PPC)
					mask |= 1 << count;
				count++;
			}
		}
		i++;
	} while ((cluster & TP_CLUSTER_EOC) != TP_CLUSTER_EOC);

	return mask;
}

#ifdef CONFIG_HETROGENOUS_CLUSTERS
u32 compute_dsp_cpumask(void)
{
	ccsr_gur_t *gur = (void __iomem *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	int i = CONFIG_DSP_CLUSTER_START, count = 0;
	u32 cluster, type, dsp_mask = 0;

	do {
		int j;
		cluster = in_be32(&gur->tp_cluster[i].lower);
		for (j = 0; j < TP_INIT_PER_CLUSTER; j++) {
			type = init_type(cluster, j);
			if (type) {
				if (TP_ITYP_TYPE(type) == TP_ITYP_TYPE_SC)
					dsp_mask |= 1 << count;
				count++;
			}
		}
		i++;
	} while ((cluster & TP_CLUSTER_EOC) != TP_CLUSTER_EOC);

	return dsp_mask;
}

int fsl_qoriq_dsp_core_to_cluster(unsigned int core)
{
	ccsr_gur_t *gur = (void __iomem *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	int count = 0, i = CONFIG_DSP_CLUSTER_START;
	u32 cluster;

	do {
		int j;
		cluster = in_be32(&gur->tp_cluster[i].lower);
		for (j = 0; j < TP_INIT_PER_CLUSTER; j++) {
			if (init_type(cluster, j)) {
				if (count == core)
					return i;
				count++;
			}
		}
		i++;
	} while ((cluster & TP_CLUSTER_EOC) != TP_CLUSTER_EOC);

	return -1;	/* cannot identify the cluster */
}
#endif

int fsl_qoriq_core_to_cluster(unsigned int core)
{
	ccsr_gur_t *gur = (void __iomem *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	int i = 0, count = 0;
	u32 cluster;

	do {
		int j;
		cluster = in_be32(&gur->tp_cluster[i].lower);
		for (j = 0; j < TP_INIT_PER_CLUSTER; j++) {
			if (init_type(cluster, j)) {
				if (count == core)
					return i;
				count++;
			}
		}
		i++;
	} while ((cluster & TP_CLUSTER_EOC) != TP_CLUSTER_EOC);

	return -1;	/* cannot identify the cluster */
}

#else /* CONFIG_SYS_FSL_QORIQ_CHASSIS2 */
/*
 * Before chassis genenration 2, the cpumask should be hard-coded.
 * In case of cpu type unknown or cpumask unset, use 1 as fail save.
 */
#define compute_ppc_cpumask()	1
#define fsl_qoriq_core_to_cluster(x) x
#endif /* CONFIG_SYS_FSL_QORIQ_CHASSIS2 */

static struct cpu_type cpu_type_unknown = CPU_TYPE_ENTRY(Unknown, Unknown, 0);

struct cpu_type *identify_cpu(u32 ver)
{
	int i;
	for (i = 0; i < ARRAY_SIZE(cpu_type_list); i++) {
		if (cpu_type_list[i].soc_ver == ver)
			return &cpu_type_list[i];
	}
	return &cpu_type_unknown;
}

#define MPC8xxx_PICFRR_NCPU_MASK  0x00001f00
#define MPC8xxx_PICFRR_NCPU_SHIFT 8

/*
 * Return a 32-bit mask indicating which cores are present on this SOC.
 */
__weak u32 cpu_mask(void)
{
	ccsr_pic_t __iomem *pic = (void *)CONFIG_SYS_MPC8xxx_PIC_ADDR;
	struct cpu_type *cpu = gd->arch.cpu;

	/* better to query feature reporting register than just assume 1 */
	if (cpu == &cpu_type_unknown)
	return ((in_be32(&pic->frr) & MPC8xxx_PICFRR_NCPU_MASK) >>
			MPC8xxx_PICFRR_NCPU_SHIFT) + 1;

	if (cpu->num_cores == 0)
		return compute_ppc_cpumask();

	return cpu->mask;
}

#ifdef CONFIG_HETROGENOUS_CLUSTERS
__weak u32 cpu_dsp_mask(void)
{
	ccsr_pic_t __iomem *pic = (void *)CONFIG_SYS_MPC8xxx_PIC_ADDR;
	struct cpu_type *cpu = gd->arch.cpu;

	/* better to query feature reporting register than just assume 1 */
	if (cpu == &cpu_type_unknown)
		return ((in_be32(&pic->frr) & MPC8xxx_PICFRR_NCPU_MASK) >>
			 MPC8xxx_PICFRR_NCPU_SHIFT) + 1;

	if (cpu->dsp_num_cores == 0)
		return compute_dsp_cpumask();

	return cpu->dsp_mask;
}

/*
 * Return the number of SC/DSP cores on this SOC.
 */
__weak int cpu_num_dspcores(void)
{
	struct cpu_type *cpu = gd->arch.cpu;

	/*
	 * Report # of cores in terms of the cpu_mask if we haven't
	 * figured out how many there are yet
	 */
	if (cpu->dsp_num_cores == 0)
		return hweight32(cpu_dsp_mask());

	return cpu->dsp_num_cores;
}
#endif

/*
 * Return the number of PPC cores on this SOC.
 */
__weak int cpu_numcores(void)
{
	struct cpu_type *cpu = gd->arch.cpu;

	/*
	 * Report # of cores in terms of the cpu_mask if we haven't
	 * figured out how many there are yet
	 */
	if (cpu->num_cores == 0)
		return hweight32(cpu_mask());

	return cpu->num_cores;
}


/*
 * Check if the given core ID is valid
 *
 * Returns zero if it isn't, 1 if it is.
 */
int is_core_valid(unsigned int core)
{
	return !!((1 << core) & cpu_mask());
}

int arch_cpu_init(void)
{
	uint svr;
	uint ver;

	svr = get_svr();
	ver = SVR_SOC_VER(svr);

	gd->arch.cpu = identify_cpu(ver);

	return 0;
}

/* Once in memory, compute mask & # cores once and save them off */
int fixup_cpu(void)
{
	struct cpu_type *cpu = gd->arch.cpu;

	if (cpu->num_cores == 0) {
		cpu->mask = cpu_mask();
		cpu->num_cores = cpu_numcores();
	}

#ifdef CONFIG_HETROGENOUS_CLUSTERS
	if (cpu->dsp_num_cores == 0) {
		cpu->dsp_mask = cpu_dsp_mask();
		cpu->dsp_num_cores = cpu_num_dspcores();
	}
#endif
	return 0;
}

/*
 * Initializes on-chip ethernet controllers.
 * to override, implement board_eth_init()
 */
int cpu_eth_init(bd_t *bis)
{
#if defined(CONFIG_ETHER_ON_FCC)
	fec_initialize(bis);
#endif

#if defined(CONFIG_UEC_ETH)
	uec_standard_init(bis);
#endif

#if defined(CONFIG_TSEC_ENET) || defined(CONFIG_MPC85XX_FEC)
	tsec_standard_init(bis);
#endif

#ifdef CONFIG_FMAN_ENET
	fm_standard_init(bis);
#endif

#ifdef CONFIG_VSC9953
	vsc9953_init(bis);
#endif
	return 0;
}
