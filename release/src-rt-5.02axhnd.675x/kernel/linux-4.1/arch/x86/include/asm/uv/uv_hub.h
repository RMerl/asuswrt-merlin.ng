/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * SGI UV architectural definitions
 *
 * Copyright (C) 2007-2014 Silicon Graphics, Inc. All rights reserved.
 */

#ifndef _ASM_X86_UV_UV_HUB_H
#define _ASM_X86_UV_UV_HUB_H

#ifdef CONFIG_X86_64
#include <linux/numa.h>
#include <linux/percpu.h>
#include <linux/timer.h>
#include <linux/io.h>
#include <asm/types.h>
#include <asm/percpu.h>
#include <asm/uv/uv_mmrs.h>
#include <asm/irq_vectors.h>
#include <asm/io_apic.h>


/*
 * Addressing Terminology
 *
 *	M       - The low M bits of a physical address represent the offset
 *		  into the blade local memory. RAM memory on a blade is physically
 *		  contiguous (although various IO spaces may punch holes in
 *		  it)..
 *
 *	N	- Number of bits in the node portion of a socket physical
 *		  address.
 *
 *	NASID   - network ID of a router, Mbrick or Cbrick. Nasid values of
 *		  routers always have low bit of 1, C/MBricks have low bit
 *		  equal to 0. Most addressing macros that target UV hub chips
 *		  right shift the NASID by 1 to exclude the always-zero bit.
 *		  NASIDs contain up to 15 bits.
 *
 *	GNODE   - NASID right shifted by 1 bit. Most mmrs contain gnodes instead
 *		  of nasids.
 *
 *	PNODE   - the low N bits of the GNODE. The PNODE is the most useful variant
 *		  of the nasid for socket usage.
 *
 *	GPA	- (global physical address) a socket physical address converted
 *		  so that it can be used by the GRU as a global address. Socket
 *		  physical addresses 1) need additional NASID (node) bits added
 *		  to the high end of the address, and 2) unaliased if the
 *		  partition does not have a physical address 0. In addition, on
 *		  UV2 rev 1, GPAs need the gnode left shifted to bits 39 or 40.
 *
 *
 *  NumaLink Global Physical Address Format:
 *  +--------------------------------+---------------------+
 *  |00..000|      GNODE             |      NodeOffset     |
 *  +--------------------------------+---------------------+
 *          |<-------53 - M bits --->|<--------M bits ----->
 *
 *	M - number of node offset bits (35 .. 40)
 *
 *
 *  Memory/UV-HUB Processor Socket Address Format:
 *  +----------------+---------------+---------------------+
 *  |00..000000000000|   PNODE       |      NodeOffset     |
 *  +----------------+---------------+---------------------+
 *                   <--- N bits --->|<--------M bits ----->
 *
 *	M - number of node offset bits (35 .. 40)
 *	N - number of PNODE bits (0 .. 10)
 *
 *		Note: M + N cannot currently exceed 44 (x86_64) or 46 (IA64).
 *		The actual values are configuration dependent and are set at
 *		boot time. M & N values are set by the hardware/BIOS at boot.
 *
 *
 * APICID format
 *	NOTE!!!!!! This is the current format of the APICID. However, code
 *	should assume that this will change in the future. Use functions
 *	in this file for all APICID bit manipulations and conversion.
 *
 *		1111110000000000
 *		5432109876543210
 *		pppppppppplc0cch	Nehalem-EX (12 bits in hdw reg)
 *		ppppppppplcc0cch	Westmere-EX (12 bits in hdw reg)
 *		pppppppppppcccch	SandyBridge (15 bits in hdw reg)
 *		sssssssssss
 *
 *			p  = pnode bits
 *			l =  socket number on board
 *			c  = core
 *			h  = hyperthread
 *			s  = bits that are in the SOCKET_ID CSR
 *
 *	Note: Processor may support fewer bits in the APICID register. The ACPI
 *	      tables hold all 16 bits. Software needs to be aware of this.
 *
 *	      Unless otherwise specified, all references to APICID refer to
 *	      the FULL value contained in ACPI tables, not the subset in the
 *	      processor APICID register.
 */


/*
 * Maximum number of bricks in all partitions and in all coherency domains.
 * This is the total number of bricks accessible in the numalink fabric. It
 * includes all C & M bricks. Routers are NOT included.
 *
 * This value is also the value of the maximum number of non-router NASIDs
 * in the numalink fabric.
 *
 * NOTE: a brick may contain 1 or 2 OS nodes. Don't get these confused.
 */
#define UV_MAX_NUMALINK_BLADES	16384

/*
 * Maximum number of C/Mbricks within a software SSI (hardware may support
 * more).
 */
#define UV_MAX_SSI_BLADES	256

/*
 * The largest possible NASID of a C or M brick (+ 2)
 */
#define UV_MAX_NASID_VALUE	(UV_MAX_NUMALINK_BLADES * 2)

struct uv_scir_s {
	struct timer_list timer;
	unsigned long	offset;
	unsigned long	last;
	unsigned long	idle_on;
	unsigned long	idle_off;
	unsigned char	state;
	unsigned char	enabled;
};

/*
 * The following defines attributes of the HUB chip. These attributes are
 * frequently referenced and are kept in the per-cpu data areas of each cpu.
 * They are kept together in a struct to minimize cache misses.
 */
struct uv_hub_info_s {
	unsigned long		global_mmr_base;
	unsigned long		gpa_mask;
	unsigned int		gnode_extra;
	unsigned char		hub_revision;
	unsigned char		apic_pnode_shift;
	unsigned char		m_shift;
	unsigned char		n_lshift;
	unsigned long		gnode_upper;
	unsigned long		lowmem_remap_top;
	unsigned long		lowmem_remap_base;
	unsigned short		pnode;
	unsigned short		pnode_mask;
	unsigned short		coherency_domain_number;
	unsigned short		numa_blade_id;
	unsigned char		blade_processor_id;
	unsigned char		m_val;
	unsigned char		n_val;
	struct uv_scir_s	scir;
};

DECLARE_PER_CPU(struct uv_hub_info_s, __uv_hub_info);
#define uv_hub_info		this_cpu_ptr(&__uv_hub_info)
#define uv_cpu_hub_info(cpu)	(&per_cpu(__uv_hub_info, cpu))

/*
 * Hub revisions less than UV2_HUB_REVISION_BASE are UV1 hubs. All UV2
 * hubs have revision numbers greater than or equal to UV2_HUB_REVISION_BASE.
 * This is a software convention - NOT the hardware revision numbers in
 * the hub chip.
 */
#define UV1_HUB_REVISION_BASE		1
#define UV2_HUB_REVISION_BASE		3
#define UV3_HUB_REVISION_BASE		5

static inline int is_uv1_hub(void)
{
	return uv_hub_info->hub_revision < UV2_HUB_REVISION_BASE;
}

static inline int is_uv2_hub(void)
{
	return ((uv_hub_info->hub_revision >= UV2_HUB_REVISION_BASE) &&
		(uv_hub_info->hub_revision < UV3_HUB_REVISION_BASE));
}

static inline int is_uv3_hub(void)
{
	return uv_hub_info->hub_revision >= UV3_HUB_REVISION_BASE;
}

static inline int is_uv_hub(void)
{
	return uv_hub_info->hub_revision;
}

/* code common to uv2 and uv3 only */
static inline int is_uvx_hub(void)
{
	return uv_hub_info->hub_revision >= UV2_HUB_REVISION_BASE;
}

union uvh_apicid {
    unsigned long       v;
    struct uvh_apicid_s {
        unsigned long   local_apic_mask  : 24;
        unsigned long   local_apic_shift :  5;
        unsigned long   unused1          :  3;
        unsigned long   pnode_mask       : 24;
        unsigned long   pnode_shift      :  5;
        unsigned long   unused2          :  3;
    } s;
};

/*
 * Local & Global MMR space macros.
 *	Note: macros are intended to be used ONLY by inline functions
 *	in this file - not by other kernel code.
 *		n -  NASID (full 15-bit global nasid)
 *		g -  GNODE (full 15-bit global nasid, right shifted 1)
 *		p -  PNODE (local part of nsids, right shifted 1)
 */
#define UV_NASID_TO_PNODE(n)		(((n) >> 1) & uv_hub_info->pnode_mask)
#define UV_PNODE_TO_GNODE(p)		((p) |uv_hub_info->gnode_extra)
#define UV_PNODE_TO_NASID(p)		(UV_PNODE_TO_GNODE(p) << 1)

#define UV1_LOCAL_MMR_BASE		0xf4000000UL
#define UV1_GLOBAL_MMR32_BASE		0xf8000000UL
#define UV1_LOCAL_MMR_SIZE		(64UL * 1024 * 1024)
#define UV1_GLOBAL_MMR32_SIZE		(64UL * 1024 * 1024)

#define UV2_LOCAL_MMR_BASE		0xfa000000UL
#define UV2_GLOBAL_MMR32_BASE		0xfc000000UL
#define UV2_LOCAL_MMR_SIZE		(32UL * 1024 * 1024)
#define UV2_GLOBAL_MMR32_SIZE		(32UL * 1024 * 1024)

#define UV3_LOCAL_MMR_BASE		0xfa000000UL
#define UV3_GLOBAL_MMR32_BASE		0xfc000000UL
#define UV3_LOCAL_MMR_SIZE		(32UL * 1024 * 1024)
#define UV3_GLOBAL_MMR32_SIZE		(32UL * 1024 * 1024)

#define UV_LOCAL_MMR_BASE		(is_uv1_hub() ? UV1_LOCAL_MMR_BASE : \
					(is_uv2_hub() ? UV2_LOCAL_MMR_BASE : \
							UV3_LOCAL_MMR_BASE))
#define UV_GLOBAL_MMR32_BASE		(is_uv1_hub() ? UV1_GLOBAL_MMR32_BASE :\
					(is_uv2_hub() ? UV2_GLOBAL_MMR32_BASE :\
							UV3_GLOBAL_MMR32_BASE))
#define UV_LOCAL_MMR_SIZE		(is_uv1_hub() ? UV1_LOCAL_MMR_SIZE : \
					(is_uv2_hub() ? UV2_LOCAL_MMR_SIZE : \
							UV3_LOCAL_MMR_SIZE))
#define UV_GLOBAL_MMR32_SIZE		(is_uv1_hub() ? UV1_GLOBAL_MMR32_SIZE :\
					(is_uv2_hub() ? UV2_GLOBAL_MMR32_SIZE :\
							UV3_GLOBAL_MMR32_SIZE))
#define UV_GLOBAL_MMR64_BASE		(uv_hub_info->global_mmr_base)

#define UV_GLOBAL_GRU_MMR_BASE		0x4000000

#define UV_GLOBAL_MMR32_PNODE_SHIFT	15
#define UV_GLOBAL_MMR64_PNODE_SHIFT	26

#define UV_GLOBAL_MMR32_PNODE_BITS(p)	((p) << (UV_GLOBAL_MMR32_PNODE_SHIFT))

#define UV_GLOBAL_MMR64_PNODE_BITS(p)					\
	(((unsigned long)(p)) << UV_GLOBAL_MMR64_PNODE_SHIFT)

#define UVH_APICID		0x002D0E00L
#define UV_APIC_PNODE_SHIFT	6

#define UV_APICID_HIBIT_MASK	0xffff0000

/* Local Bus from cpu's perspective */
#define LOCAL_BUS_BASE		0x1c00000
#define LOCAL_BUS_SIZE		(4 * 1024 * 1024)

/*
 * System Controller Interface Reg
 *
 * Note there are NO leds on a UV system.  This register is only
 * used by the system controller to monitor system-wide operation.
 * There are 64 regs per node.  With Nahelem cpus (2 cores per node,
 * 8 cpus per core, 2 threads per cpu) there are 32 cpu threads on
 * a node.
 *
 * The window is located at top of ACPI MMR space
 */
#define SCIR_WINDOW_COUNT	64
#define SCIR_LOCAL_MMR_BASE	(LOCAL_BUS_BASE + \
				 LOCAL_BUS_SIZE - \
				 SCIR_WINDOW_COUNT)

#define SCIR_CPU_HEARTBEAT	0x01	/* timer interrupt */
#define SCIR_CPU_ACTIVITY	0x02	/* not idle */
#define SCIR_CPU_HB_INTERVAL	(HZ)	/* once per second */

/* Loop through all installed blades */
#define for_each_possible_blade(bid)		\
	for ((bid) = 0; (bid) < uv_num_possible_blades(); (bid)++)

/*
 * Macros for converting between kernel virtual addresses, socket local physical
 * addresses, and UV global physical addresses.
 *	Note: use the standard __pa() & __va() macros for converting
 *	      between socket virtual and socket physical addresses.
 */

/* socket phys RAM --> UV global physical address */
static inline unsigned long uv_soc_phys_ram_to_gpa(unsigned long paddr)
{
	if (paddr < uv_hub_info->lowmem_remap_top)
		paddr |= uv_hub_info->lowmem_remap_base;
	paddr |= uv_hub_info->gnode_upper;
	paddr = ((paddr << uv_hub_info->m_shift) >> uv_hub_info->m_shift) |
		((paddr >> uv_hub_info->m_val) << uv_hub_info->n_lshift);
	return paddr;
}


/* socket virtual --> UV global physical address */
static inline unsigned long uv_gpa(void *v)
{
	return uv_soc_phys_ram_to_gpa(__pa(v));
}

/* Top two bits indicate the requested address is in MMR space.  */
static inline int
uv_gpa_in_mmr_space(unsigned long gpa)
{
	return (gpa >> 62) == 0x3UL;
}

/* UV global physical address --> socket phys RAM */
static inline unsigned long uv_gpa_to_soc_phys_ram(unsigned long gpa)
{
	unsigned long paddr;
	unsigned long remap_base = uv_hub_info->lowmem_remap_base;
	unsigned long remap_top =  uv_hub_info->lowmem_remap_top;

	gpa = ((gpa << uv_hub_info->m_shift) >> uv_hub_info->m_shift) |
		((gpa >> uv_hub_info->n_lshift) << uv_hub_info->m_val);
	paddr = gpa & uv_hub_info->gpa_mask;
	if (paddr >= remap_base && paddr < remap_base + remap_top)
		paddr -= remap_base;
	return paddr;
}


/* gpa -> pnode */
static inline unsigned long uv_gpa_to_gnode(unsigned long gpa)
{
	return gpa >> uv_hub_info->n_lshift;
}

/* gpa -> pnode */
static inline int uv_gpa_to_pnode(unsigned long gpa)
{
	unsigned long n_mask = (1UL << uv_hub_info->n_val) - 1;

	return uv_gpa_to_gnode(gpa) & n_mask;
}

/* gpa -> node offset*/
static inline unsigned long uv_gpa_to_offset(unsigned long gpa)
{
	return (gpa << uv_hub_info->m_shift) >> uv_hub_info->m_shift;
}

/* pnode, offset --> socket virtual */
static inline void *uv_pnode_offset_to_vaddr(int pnode, unsigned long offset)
{
	return __va(((unsigned long)pnode << uv_hub_info->m_val) | offset);
}


/*
 * Extract a PNODE from an APICID (full apicid, not processor subset)
 */
static inline int uv_apicid_to_pnode(int apicid)
{
	return (apicid >> uv_hub_info->apic_pnode_shift);
}

/*
 * Convert an apicid to the socket number on the blade
 */
static inline int uv_apicid_to_socket(int apicid)
{
	if (is_uv1_hub())
		return (apicid >> (uv_hub_info->apic_pnode_shift - 1)) & 1;
	else
		return 0;
}

/*
 * Access global MMRs using the low memory MMR32 space. This region supports
 * faster MMR access but not all MMRs are accessible in this space.
 */
static inline unsigned long *uv_global_mmr32_address(int pnode, unsigned long offset)
{
	return __va(UV_GLOBAL_MMR32_BASE |
		       UV_GLOBAL_MMR32_PNODE_BITS(pnode) | offset);
}

static inline void uv_write_global_mmr32(int pnode, unsigned long offset, unsigned long val)
{
	writeq(val, uv_global_mmr32_address(pnode, offset));
}

static inline unsigned long uv_read_global_mmr32(int pnode, unsigned long offset)
{
	return readq(uv_global_mmr32_address(pnode, offset));
}

/*
 * Access Global MMR space using the MMR space located at the top of physical
 * memory.
 */
static inline volatile void __iomem *uv_global_mmr64_address(int pnode, unsigned long offset)
{
	return __va(UV_GLOBAL_MMR64_BASE |
		    UV_GLOBAL_MMR64_PNODE_BITS(pnode) | offset);
}

static inline void uv_write_global_mmr64(int pnode, unsigned long offset, unsigned long val)
{
	writeq(val, uv_global_mmr64_address(pnode, offset));
}

static inline unsigned long uv_read_global_mmr64(int pnode, unsigned long offset)
{
	return readq(uv_global_mmr64_address(pnode, offset));
}

/*
 * Global MMR space addresses when referenced by the GRU. (GRU does
 * NOT use socket addressing).
 */
static inline unsigned long uv_global_gru_mmr_address(int pnode, unsigned long offset)
{
	return UV_GLOBAL_GRU_MMR_BASE | offset |
		((unsigned long)pnode << uv_hub_info->m_val);
}

static inline void uv_write_global_mmr8(int pnode, unsigned long offset, unsigned char val)
{
	writeb(val, uv_global_mmr64_address(pnode, offset));
}

static inline unsigned char uv_read_global_mmr8(int pnode, unsigned long offset)
{
	return readb(uv_global_mmr64_address(pnode, offset));
}

/*
 * Access hub local MMRs. Faster than using global space but only local MMRs
 * are accessible.
 */
static inline unsigned long *uv_local_mmr_address(unsigned long offset)
{
	return __va(UV_LOCAL_MMR_BASE | offset);
}

static inline unsigned long uv_read_local_mmr(unsigned long offset)
{
	return readq(uv_local_mmr_address(offset));
}

static inline void uv_write_local_mmr(unsigned long offset, unsigned long val)
{
	writeq(val, uv_local_mmr_address(offset));
}

static inline unsigned char uv_read_local_mmr8(unsigned long offset)
{
	return readb(uv_local_mmr_address(offset));
}

static inline void uv_write_local_mmr8(unsigned long offset, unsigned char val)
{
	writeb(val, uv_local_mmr_address(offset));
}

/*
 * Structures and definitions for converting between cpu, node, pnode, and blade
 * numbers.
 */
struct uv_blade_info {
	unsigned short	nr_possible_cpus;
	unsigned short	nr_online_cpus;
	unsigned short	pnode;
	short		memory_nid;
	spinlock_t	nmi_lock;	/* obsolete, see uv_hub_nmi */
	unsigned long	nmi_count;	/* obsolete, see uv_hub_nmi */
};
extern struct uv_blade_info *uv_blade_info;
extern short *uv_node_to_blade;
extern short *uv_cpu_to_blade;
extern short uv_possible_blades;

/* Blade-local cpu number of current cpu. Numbered 0 .. <# cpus on the blade> */
static inline int uv_blade_processor_id(void)
{
	return uv_hub_info->blade_processor_id;
}

/* Blade number of current cpu. Numnbered 0 .. <#blades -1> */
static inline int uv_numa_blade_id(void)
{
	return uv_hub_info->numa_blade_id;
}

/* Convert a cpu number to the the UV blade number */
static inline int uv_cpu_to_blade_id(int cpu)
{
	return uv_cpu_to_blade[cpu];
}

/* Convert linux node number to the UV blade number */
static inline int uv_node_to_blade_id(int nid)
{
	return uv_node_to_blade[nid];
}

/* Convert a blade id to the PNODE of the blade */
static inline int uv_blade_to_pnode(int bid)
{
	return uv_blade_info[bid].pnode;
}

/* Nid of memory node on blade. -1 if no blade-local memory */
static inline int uv_blade_to_memory_nid(int bid)
{
	return uv_blade_info[bid].memory_nid;
}

/* Determine the number of possible cpus on a blade */
static inline int uv_blade_nr_possible_cpus(int bid)
{
	return uv_blade_info[bid].nr_possible_cpus;
}

/* Determine the number of online cpus on a blade */
static inline int uv_blade_nr_online_cpus(int bid)
{
	return uv_blade_info[bid].nr_online_cpus;
}

/* Convert a cpu id to the PNODE of the blade containing the cpu */
static inline int uv_cpu_to_pnode(int cpu)
{
	return uv_blade_info[uv_cpu_to_blade_id(cpu)].pnode;
}

/* Convert a linux node number to the PNODE of the blade */
static inline int uv_node_to_pnode(int nid)
{
	return uv_blade_info[uv_node_to_blade_id(nid)].pnode;
}

/* Maximum possible number of blades */
static inline int uv_num_possible_blades(void)
{
	return uv_possible_blades;
}

/* Per Hub NMI support */
extern void uv_nmi_setup(void);

/* BMC sets a bit this MMR non-zero before sending an NMI */
#define UVH_NMI_MMR		UVH_SCRATCH5
#define UVH_NMI_MMR_CLEAR	UVH_SCRATCH5_ALIAS
#define UVH_NMI_MMR_SHIFT	63
#define	UVH_NMI_MMR_TYPE	"SCRATCH5"

/* Newer SMM NMI handler, not present in all systems */
#define UVH_NMI_MMRX		UVH_EVENT_OCCURRED0
#define UVH_NMI_MMRX_CLEAR	UVH_EVENT_OCCURRED0_ALIAS
#define UVH_NMI_MMRX_SHIFT	(is_uv1_hub() ? \
					UV1H_EVENT_OCCURRED0_EXTIO_INT0_SHFT :\
					UVXH_EVENT_OCCURRED0_EXTIO_INT0_SHFT)
#define	UVH_NMI_MMRX_TYPE	"EXTIO_INT0"

/* Non-zero indicates newer SMM NMI handler present */
#define UVH_NMI_MMRX_SUPPORTED	UVH_EXTIO_INT0_BROADCAST

/* Indicates to BIOS that we want to use the newer SMM NMI handler */
#define UVH_NMI_MMRX_REQ	UVH_SCRATCH5_ALIAS_2
#define UVH_NMI_MMRX_REQ_SHIFT	62

struct uv_hub_nmi_s {
	raw_spinlock_t	nmi_lock;
	atomic_t	in_nmi;		/* flag this node in UV NMI IRQ */
	atomic_t	cpu_owner;	/* last locker of this struct */
	atomic_t	read_mmr_count;	/* count of MMR reads */
	atomic_t	nmi_count;	/* count of true UV NMIs */
	unsigned long	nmi_value;	/* last value read from NMI MMR */
};

struct uv_cpu_nmi_s {
	struct uv_hub_nmi_s	*hub;
	int			state;
	int			pinging;
	int			queries;
	int			pings;
};

DECLARE_PER_CPU(struct uv_cpu_nmi_s, uv_cpu_nmi);

#define uv_hub_nmi			(uv_cpu_nmi.hub)
#define uv_cpu_nmi_per(cpu)		(per_cpu(uv_cpu_nmi, cpu))
#define uv_hub_nmi_per(cpu)		(uv_cpu_nmi_per(cpu).hub)

/* uv_cpu_nmi_states */
#define	UV_NMI_STATE_OUT		0
#define	UV_NMI_STATE_IN			1
#define	UV_NMI_STATE_DUMP		2
#define	UV_NMI_STATE_DUMP_DONE		3

/* Update SCIR state */
static inline void uv_set_scir_bits(unsigned char value)
{
	if (uv_hub_info->scir.state != value) {
		uv_hub_info->scir.state = value;
		uv_write_local_mmr8(uv_hub_info->scir.offset, value);
	}
}

static inline unsigned long uv_scir_offset(int apicid)
{
	return SCIR_LOCAL_MMR_BASE | (apicid & 0x3f);
}

static inline void uv_set_cpu_scir_bits(int cpu, unsigned char value)
{
	if (uv_cpu_hub_info(cpu)->scir.state != value) {
		uv_write_global_mmr8(uv_cpu_to_pnode(cpu),
				uv_cpu_hub_info(cpu)->scir.offset, value);
		uv_cpu_hub_info(cpu)->scir.state = value;
	}
}

extern unsigned int uv_apicid_hibits;
static unsigned long uv_hub_ipi_value(int apicid, int vector, int mode)
{
	apicid |= uv_apicid_hibits;
	return (1UL << UVH_IPI_INT_SEND_SHFT) |
			((apicid) << UVH_IPI_INT_APIC_ID_SHFT) |
			(mode << UVH_IPI_INT_DELIVERY_MODE_SHFT) |
			(vector << UVH_IPI_INT_VECTOR_SHFT);
}

static inline void uv_hub_send_ipi(int pnode, int apicid, int vector)
{
	unsigned long val;
	unsigned long dmode = dest_Fixed;

	if (vector == NMI_VECTOR)
		dmode = dest_NMI;

	val = uv_hub_ipi_value(apicid, vector, dmode);
	uv_write_global_mmr64(pnode, UVH_IPI_INT, val);
}

/*
 * Get the minimum revision number of the hub chips within the partition.
 *     1 - UV1 rev 1.0 initial silicon
 *     2 - UV1 rev 2.0 production silicon
 *     3 - UV2 rev 1.0 initial silicon
 *     5 - UV3 rev 1.0 initial silicon
 */
static inline int uv_get_min_hub_revision_id(void)
{
	return uv_hub_info->hub_revision;
}

#endif /* CONFIG_X86_64 */
#endif /* _ASM_X86_UV_UV_HUB_H */
