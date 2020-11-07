/*
    Copyright (c) 2017 Broadcom
    All Rights Reserved

    <:label-BRCM:2017:DUAL/GPL:standard

    Unless you and Broadcom execute a separate written software license
    agreement governing use of this software, this software is licensed
    to you under the terms of the GNU General Public License version 2
    (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
    with the following added to such license:

       As a special exception, the copyright holders of this software give
       you permission to link this software with independent modules, and
       to copy and distribute the resulting executable under terms of your
       choice, provided that you also meet, for each linked independent
       module, the terms and conditions of the license of that module.
       An independent module is a module which is not derived from this
       software.  The special exception does not apply to any modifications
       of the software.

    Not withstanding the above, under no circumstances may you combine
    this software in any way with any other Broadcom software provided
    under a license other than the GPL, without Broadcom's express prior
    written consent.

    :>
*/

#include <typedefs.h>
#include <linux/pci.h>
#include <linux/bcm_colors.h>

#include <rdpa_api.h>
#include <rdpa_dhd_helper.h>
#include <rdpa_ag_dhd_helper.h>
#include <rdpa_mw_cpu_queue_ids.h>
#include <rdd.h>

#include <bcm_rsvmem.h>
#include <bcmutils.h>
#include <bcmmsgbuf.h>
#include <bcmendian.h>
#include <dngl_stats.h>

#include <dhd.h>
#include <dhd_bus.h>
#include <dhd_dbg.h>
#include <dhd_runner.h>
#include <dhd_flowring.h>
#include <dhd_linux.h>
#include <dhd_proto.h>

#include <pcie_core.h>
#include <dhd_pcie.h>
#include <bcm_map_part.h>
#include <wlan_shared_defs.h>

#if (defined(CONFIG_BCM_SPDSVC) || defined(CONFIG_BCM_SPDSVC_MODULE))
#include <bcm_spdsvc.h>
static bcmFun_t *dhd_runner_spdsvc_transmit = NULL;
#endif

/*
 * +---------------------------------------------------------------------
 *           Section: TESTING MACROS for local debugging
 *
 * +---------------------------------------------------------------------
*/
#if 0
#define RPR(fmt, args...)     printk(fmt "\n", ##args)
#define RPR1(fmt, args...)    printk(fmt "\n", ##args)
#define RPR2(fmt, args...)    printk(fmt "\n", ##args)
#define RLOG(fmt, args...)    printk(fmt "\n", ##args)
#else
#define RPR(fmt, args...)     do {} while (0)
#define RPR1(fmt, args...)    do {} while (0)
#define RPR2(fmt, args...)    do {} while (0)
#define RLOG(fmt, args...)    printk(fmt "\n", ##args)
#endif

/*
 * +---------------------------------------------------------------------
 *           Section: RDPA flowring Cache definitions
 *
 * rdpa_dhd_flring_cache_t flags field bit definitions
 *
 *  bits 1..0 are used and owned by Runner firmware
 *  bits 7..2 are used and owned by DHD software
 *
 * +---------------------------------------------------------------------
 */
#define DHD_RNR_FLRING_DISABLED_FLAG     FLOW_RING_FLAG_DISABLED
#define DHD_RNR_FLRING_IN_RUNNER_FLAG    (1 << 2)
#define DHD_RNR_FLRING_WME_AC_SHIFT      (4)
#define DHD_RNR_FLRING_WME_AC_MASK       (0xF << DHD_RNR_FLRING_WME_AC_SHIFT)

/*
 * +---------------------------------------------------------------------
 *           Section: Architecture specific definitions and macros
 *
 *   Given a memory address, set a DMAable buffer's cacheable virtual address.
 *   Given a DMAable buffer with a valid virt address, set the physical address.
 *
 * +---------------------------------------------------------------------
*/
#if defined(__arm__) || defined(CONFIG_ARM64)
#	define ARCH_ALLOC_COHERENT_MEM(dhd, dma_buf)                       \
	    dma_buf->va = (void *)dma_alloc_coherent(&dhd->bus->dev->dev,  \
	        dma_buf->len, (dma_addr_t *) &dma_buf->pa, GFP_KERNEL)
#	define ARCH_FREE_COHERENT_MEM(dhd, dma_buf)                        \
	    dma_free_coherent(&dhd->bus->dev->dev, dma_buf->len,           \
	        dma_buf->va, dma_buf->pa)
#	define ARCH_SET_DMA_BUF_VA(dma_buf, addr) \
	    (dma_buf)->va = (void*)(addr);
#	define ARCH_VIRT_TO_PHYS(va) virt_to_phys(va)
#   define ARCH_FLUSH_COHERENT(addr, len)   do {} while (0)
#	define RDPA_INIT_CFG_SET_PA(field, pa)    do { field = (pa); } while (0)
#	define ARCH_WR_DMA_IDX(ptr, idx) iowrite16(idx, ptr)
#	define ARCH_RD_DMA_IDX(ptr, idx) idx = LTOH16(*(ptr))
#elif defined(__mips__)
#	define ARCH_ALLOC_COHERENT_MEM(dhd, dma_buf)                       \
	    dma_buf->va = DMA_ALLOC_CONSISTENT(dhd->osh, dma_buf->len,     \
	        4, &alloced, &dma_buf->pa, &dma_buf->dmah);                \
	    ARCH_SET_DMA_BUF_PA(dma_buf);                                  \
	    dma_buf->va = (void*)OSL_UNCACHED(dma_buf->va)
#	define ARCH_FREE_COHERENT_MEM(dhd, dma_buf)                        \
	    dma_buf->va = (void*)OSL_CACHED(dma_buf->va);                  \
	    DMA_FREE_CONSISTENT(dhd->osh, dma_buf->va, dma_buf->len,       \
	        dma_buf->pa, dma_buf->dmah)
#	define IO_ADDRESS(x)    (x)
#	define ARCH_SET_DMA_BUF_VA(dma_buf, addr) \
	    (dma_buf)->va = (void*)KSEG0ADDR(addr);
#	define ARCH_VIRT_TO_PHYS(va) CPHYSADDR(va)
#	define ARCH_FLUSH_COHERENT(addr, len) \
	    OSL_CACHE_FLUSH((void *)(addr), (len))
#	define RDPA_INIT_CFG_SET_PA(field, pa)    do { } while (0)
#	define ARCH_WR_DMA_IDX(ptr, idx)      *ptr = HTOL16(idx)
#	define ARCH_RD_DMA_IDX(ptr, idx)      idx = LTOH16(*(ptr));
#else  /* ! __arm__ && ! CONFIG_ARM64 && ! __mips__ */
#	error "revisit memory management and endian handling support"
#endif /* ! __arm__ && ! CONFIG_ARM64 && ! __mips__ */
#	define ARCH_SET_DMA_BUF_PA(dma_buf) \
	    PHYSADDRHISET((dma_buf)->pa, (ulong)ARCH_VIRT_TO_PHYS((dma_buf)->va)); \
	    PHYSADDRLOSET((dma_buf)->pa, (ulong)ARCH_VIRT_TO_PHYS((dma_buf)->va))


/*
 * +---------------------------------------------------------------------
 *           Section: External variables and functions declarations
 *
 * +---------------------------------------------------------------------
*/
extern int bdmf_global_trace_level;
extern int dhd_rxbound;
extern int dhd_txbound;
extern void dhdpcie_bus_ringbell_fast(struct dhd_bus *bus, uint32 value);
#if defined(BCMHWA)
extern void dhdpcie_bus_ringbell_2_fast(struct dhd_bus *bus, uint32 value, bool devwake);
extern void dhdpcie_bus_db1_ringbell_2_fast(struct dhd_bus *bus, uint32 value, bool devwake);
#endif /* BCMHWA */
extern void dhd_prot_runner_txstatus_process(dhd_pub_t *dhd, void* pktid);
extern int kerSysScratchPadSet(char *tokenId, char *tokBuf, int bufLen);
extern int kerSysScratchPadGet(char *tokenId, char *tokBuf, int bufLen);
extern char * nvram_get(const char *name);


/*
 * +---------------------------------------------------------------------
 *           Section: Local definitions
 *
 * +---------------------------------------------------------------------
*/
#define DHD_RNR_MAX_RADIOS                  3
#define DHD_RNR_MAX_BSS                     16
#define DHD_RNR_MAX_STATIONS                128
#define DHD_RNR_IOVAR_BUFF_SIZE             512
#define DHD_RNR_DEF_RX_OFFLOAD              1
#define DHD_RNR_DEF_MCBC_PROFILE_WEIGHT     1
#define DHD_RNR_COHERENT_MEM_POOL_SIZE      32*1024  /* 32 KB */
#define DHD_RNR_COHERENT_MEM_POOL_ALIGN_MASK 0x3FF

#define TX_DOR_MODE_M                       0  /* Offload:0, n+m: disabled */
#define TX_DOR_MODE_N_DEF                   1  /* Offload:1, n+m: default */
#define TX_DOR_MODE_N_ONLY                  2  /* Offlaod:1, n+m: disabled */
#define TX_DOR_MODE_NPM                     3  /* Offload:1, n+m: enabled */

/* When a per AC profile with -1 weight is used, use a two pass with a budget
 * for the first pass.
 */
#define DHD_RNR_PER_AC_PROFILE_BUDGET       64

#define DHD_RNR_IS_TX_OFFL_SUPPORTED(dhd_hlp)           \
	((dhd_hlp)->rnr_sup_feat.txoffl == 1)

#define DHD_RNR_IS_RX_OFFL_SUPPORTED(dhd_hlp)           \
	((dhd_hlp)->rnr_sup_feat.rxoffl == 1)

#define DHD_RNR_IS_OFFL_SUPPORTED(dhd_hlp)              \
	(DHD_RNR_IS_TX_OFFL_SUPPORTED(dhd_hlp) && DHD_RNR_IS_RX_OFFL_SUPPORTED(dhd_hlp))

#define DHD_RNR_RX_OFFLOAD(dhd_hlp)                     \
	((dhd_hlp)->rxcmpl_ring_cfg.offload == 1)
#define DHD_RNR_TXSTS_OFFLOAD(dhd_hlp)                  \
	((dhd_hlp)->txsts_ring_cfg.offload & DHD_RNR_TXSTS_CFG_ACCPKT_OFFL)
#define DHD_RNR_NONACCPKT_TXSTS_OFFLOAD(dhd_hlp)        \
	((dhd_hlp)->txsts_ring_cfg.offload & DHD_RNR_TXSTS_CFG_NONACCPKT_OFFL)
#define PCIE_WR_CONFIG(rb, off, val)                    \
	pci_bus_write_config_dword((rb), 0, (off), (val))

/*
 * +---------------------------------------------------------------------
 *           Section: platform specifc settings
 * +---------------------------------------------------------------------
*/
#if defined(BCA_HNDROUTER) && !defined(BCA_CPEROUTER)
/* N+M feature default, enabled for HND */
#define DOR_NPM_DEFAULT                     1

#define DHD_RNR_PROC_CPUQ_IN_DPC            FALSE            /* Disable */
#if DOR_NPM_DEFAULT == 1
#define DHD_RNR_BCMC_TXOFFL_PRIORITY        0                /* Force Off */
#else /* !DOR_NPM_DEFAULT */
#define DHD_RNR_BCMC_TXOFFL_PRIORITY        1                /* Normal */
#endif /* !DOR_NPM_DEFAULT */
#define DHD_RNR_INIT_PERIM_UNLOCK(dhdp)     DHD_UNLOCK(dhdp)
#define DHD_RNR_INIT_PERIM_LOCK(dhdp)       DHD_LOCK(dhdp)
#else /* !BCA_HNDROUTER */
/* N+M feature default, disabled from REL_5.04L.01 and REL_5.02L.07P1 */
#define DOR_NPM_DEFAULT                     0

#if defined(CONFIG_BCM_PON_RDP)
#define DHD_RNR_PROC_CPUQ_IN_DPC            FALSE            /* Disable */
#else /* !CONFIG_BCM_PON_RDP */
#define DHD_RNR_PROC_CPUQ_IN_DPC            TRUE              /* Enable */
#endif /* !CONFIG_BCM_PON_RDP */
#define DHD_RNR_BCMC_TXOFFL_PRIORITY        1                 /* Normal */
#define DHD_RNR_INIT_PERIM_UNLOCK(dhdp)     do {} while (0)   /* no OP */
#define DHD_RNR_INIT_PERIM_LOCK(dhdp)       do {} while (0)   /* no OP */
#endif /* !BCA_HNDROUTER */

/*
 * +---------------------------------------------------------------------
 *           Section: DHD Runner memory allocation
 *
 * Enable DHD_RNR_MEM_ALLOC_AUDIT to get a summary of memory leaks
 * of dhd_runner part at the end of dhd driver unloading
 *
 * +---------------------------------------------------------------------
 */
/* #define DHD_RNR_MEM_ALLOC_AUDIT */
/* memory allocation */
#define DHD_RNR_MEM_ALLOC_TYPE_OSAL        0x0000    /* OSL DMA_ALLOC_CONSISTENT */
#define DHD_RNR_MEM_ALLOC_TYPE_COHERENT    0x0001    /* dma_alloc_coherent */
#define DHD_RNR_MEM_ALLOC_TYPE_KMALLOC     0x0002    /* kmalloc(GFP_DMA) */
#define DHD_RNR_MAX_MEM_ALLOC_TYPES        3         /* delimiter */

#define DHD_RNR_MEM_TYPE_DHD         DHD_RNR_MEM_ALLOC_TYPE_OSAL       /* owner DHD */
#define DHD_RNR_MEM_TYPE_RNR         DHD_RNR_MEM_ALLOC_TYPE_COHERENT   /* owner RNR */
#define DHD_RNR_MEM_TYPE_RNRVA       DHD_RNR_MEM_ALLOC_TYPE_KMALLOC    /* owner RNR */

#ifdef DHD_RNR_MEM_ALLOC_AUDIT
#define DHD_RNR_INC_MEM_ALLOC_CNTRS(dhd_hlp, type, size)  \
	(dhd_hlp)->alloc_cnt[(type)]++; (dhd_hlp)->alloc_size[(type)] += (size)
#define DHD_RNR_DEC_MEM_ALLOC_CNTRS(dhd_hlp, type, size)  \
	(dhd_hlp)->alloc_cnt[(type)]--; (dhd_hlp)->alloc_size[(type)] -= (size)
#define DHD_RNR_DUMP_MEM_ALLOC_CNTRS(dhd_hlp)                                \
	do {                                                                 \
	    int type = DHD_RNR_MEM_ALLOC_TYPE_OSAL;                          \
	    RLOG("dhd_runner MemLeak Summary\nType: Leaks: Leak Bytes");     \
	    do {                                                             \
	        RLOG(" %2d :  %4d : %08d", type,                             \
	            (dhd_hlp)->alloc_cnt[type],                              \
	            (dhd_hlp)->alloc_size[type]);                            \
	        type++;                                                      \
	    } while (type < DHD_RNR_MAX_MEM_ALLOC_TYPES);                    \
	} while (0)
#else /* !DHD_RNR_MEM_ALLOC_AUDIT */
#define DHD_RNR_INC_MEM_ALLOC_CNTRS(dhd_hlp, type, size)  \
	do {} while (0)
#define DHD_RNR_DEC_MEM_ALLOC_CNTRS(dhd_hlp, type, size)  \
	do {} while (0)
#define DHD_RNR_DUMP_MEM_ALLOC_CNTRS(dhd_hlp)             \
	do {} while (0)
#endif /* !DHD_RNR_MEM_ALLOC_AUDIT */

/*
 * +---------------------------------------------------------------------
 *           Section: Flowring Policy and profile management
 *
 * +---------------------------------------------------------------------
 */
/* Flowring pool memory management. */
typedef enum dhd_wme_ac {
	wme_ac_bk = 0,
	wme_ac_be = 1,
	wme_ac_vi = 2,
	wme_ac_vo = 3,
	wme_ac_max = 4
} dhd_wme_ac_t;
#define wme_ac_bcmc (wme_ac_max)

/* Profile definitions */
#define DHD_RNR_FLOWRING_PROFILES           8
#define DHD_RNR_FLOWRING_USER_PROFILES      DHD_RNR_MAX_RADIOS
#define DHD_RNR_FLOWRING_DEFAULT_PROFILE_ID 3
#define DHD_RNR_FLOWRING_MAX_SIZE           16384

/* flow ring profile setting maintained per radio */
typedef struct dhd_flowring_profile {
	bool npm;                   /* only N+M profile */
	int id;                     /* Profile id, 0 - DHD_RNR_FLOWRING_PROFILES-1 */
	int weight[wme_ac_max + 1]; /* -1 weight implies use rsvd memory */
	int items[wme_ac_max + 1];  /* flow ring max items (size) */
} dhd_flowring_profile_t;

/* Policy definitions */
#define DHD_RNR_POLICY_MAX_MACLIST          6

/* flowring policy identifiers */
typedef enum dhd_flowring_policy_id {
	dhd_rnr_policy_global = 0,    /* ID for Global policy */
	dhd_rnr_policy_intf = 1,      /* ID for interface based policy */
	dhd_rnr_policy_clients = 2,   /* ID for clients based policy */
	dhd_rnr_policy_aclist = 3,    /* ID for ac list based policy */
	dhd_rnr_policy_maclist = 4,   /* ID for mac list based policy */
	dhd_rnr_policy_dllac = 5,     /* ID for.11 AC client based policy */
	dhd_rnr_max_policies = 6      /* delimiter */
} dhd_flowring_policy_id_t;

/*
 * flowring policy setting maintained per radio
 * policy specifies the preferred choice to select offload
 */
typedef struct dhd_flowring_policy {
	int id;                       /* policy Identifier */
	union {
	    bool all_hw;              /* all rings offload flag */
	    int  max_intf;            /* maximum interfaces to offload */
	    int  max_sta;             /* max assoc stations to offload */
	    bool aclist_hw[wme_ac_max + 1]; /* ac list to offload */
	    /* station mac address list to offload */
	    uint8 mac_addr[DHD_RNR_POLICY_MAX_MACLIST][ETHER_ADDR_LEN+1];
	    bool d11ac_hw;            /* .11 AC client offload */
	};
} dhd_flowring_policy_t;

/* DHD PCIe msg ring information maintained in dhd_runner */
typedef struct dhd_msgring_profile {
	int offload;            /* enabled: 1 (default), disabled: 0 */
	int max_items;          /* max items in msg ring (configurable for txpost, rxpost) */
	int type;               /* legacy, cwi32, cwi64 */
	int size;               /* len of the work item */
	uint32 sup_types;       /* bit mask of types supported by runner */
} dhd_runner_ring_cfg_t;

const uint8 dhd_wme_fifo2ac[] = { 0, 1, 2, 3 };
/* incoming priority is already AC (0-3) */
const uint8 dhd_prio2fifo[4] = { 1, 0, 2, 3  };

#define WME_PRIO2AC(pktprio)  dhd_wme_fifo2ac[dhd_prio2fifo[(pktprio)]]
#define WME_AC2PRIO(ac)       dhd_wme_fifo2ac[(ac)]
const char * dhd_wme_ac_str[] = {"ac_bk", "ac_be", "ac_vi", "ac_vo", "bc_mc"};

const char *dhd_flowring_policy_id_str[] = {"global", "intfidx", "clients",
	"aclist", "maclist", "dot11ac" };

#define DHD_RNR_TXPOST_MAX_ITEM              2048
/* Default physical flow ring size when runner backup queues are enabled - per access category*/
const uint16 dhd_flowring_phy_ring_size_defaults[] = { 512, 1024, 512, 512, 512 };


#define DHD_RNR_TXPOST_AC_BK_MAX_ITEM        (DHD_RNR_TXPOST_MAX_ITEM / 2)
#define DHD_RNR_TXPOST_AC_BE_MAX_ITEM        (DHD_RNR_TXPOST_MAX_ITEM)
#define DHD_RNR_TXPOST_AC_VI_MAX_ITEM        (DHD_RNR_TXPOST_MAX_ITEM / 2)
#define DHD_RNR_TXPOST_AC_VO_MAX_ITEM        (DHD_RNR_TXPOST_MAX_ITEM / 4)
#define DHD_RNR_TXPOST_BCMC_MAX_ITEM         (DHD_RNR_TXPOST_MAX_ITEM / 4)

#define DHD_RNR_TXPOST_MAX_ITEM_LIST \
{ \
	DHD_RNR_TXPOST_AC_BK_MAX_ITEM, \
	DHD_RNR_TXPOST_AC_BE_MAX_ITEM, \
	DHD_RNR_TXPOST_AC_VI_MAX_ITEM, \
	DHD_RNR_TXPOST_AC_VO_MAX_ITEM, \
	DHD_RNR_TXPOST_BCMC_MAX_ITEM \
}

/*
 * Storage area selection for key settings
 */
#define DHD_RNR_RADIO_IDX_ALL               'a'
/* Comment out below line if Scratchpad is used instead of NVRAM */
#define DHD_RNR_NVRAM_KEYS

/* key ids within dhd runner */
#define DHD_RNR_KEY_STR_LEN                 32
#define DHD_RNR_KEY_PROFILE                 0
#define DHD_RNR_KEY_POLICY                  1
#define DHD_RNR_KEY_RXOFFL                  2
#define DHD_RNR_KEY_TXOFFL                  3
#define DHD_RNR_KEY_SWFEATURES              4
#define DHD_RNR_KEY_PHY_RING_SIZE           5
#define DHD_RNR_KEY_OFFL_OVERRIDE           6
#define DHD_RNR_MAX_KEYS                    7

/* dhd_runner configurable feature bitmap */
#define DHD_RNR_SWFEATURE_CPUQINDPC         (1UL << 0)

/* Max length of key information stored in memory */
#define DHD_RNR_KEY_PPROFILE_STR_LEN        64    /* Profile information */
#define DHD_RNR_KEY_POLICY_STR_LEN          768   /* Policy information */
#define DHD_RNR_KEY_RXOFFL_STR_LEN          16    /* Rx Offload information */
#define DHD_RNR_KEY_TXOFFL_STR_LEN          16    /* Tx Offload information */
#define DHD_RNR_KEY_SWFEATURE_STR_LEN       16    /* sw features information */
#define DHD_RNR_KEY_PHYRINGSIZE_STR_LEN     64    /* physical ring size information */
#define DHD_RNR_KEY_OFFL_OVERRIDE_STR_LEN   32    /* Offload override information */

/* storage key id vs format str */
char dhd_runner_key_fmt_str[DHD_RNR_MAX_KEYS][DHD_RNR_KEY_STR_LEN] = {
	"dhd%d_rnr_flowring_profile",
	"dhd%d_rnr_flowring_policy",
	"dhd%d_rnr_rxoffl",
	"dhd%d_rnr_txoffl",
	"dhd%d_rnr_swfeatures",
	"dhd%d_rnr_flowring_physize",
	"dhd_rnr_offload_override",
};

/* flowring profiles id vs profile */
dhd_flowring_profile_t dhd_rnr_profiles[DHD_RNR_FLOWRING_PROFILES] =
{
	{ false, 0, { 1, -1, -1, -1, 1 },  DHD_RNR_TXPOST_MAX_ITEM_LIST },
	{ false, 1, { 1, -1, -1, -1, 1 },  DHD_RNR_TXPOST_MAX_ITEM_LIST },
	{ false, 2, { 1, -1, -1, -1, 1 },  DHD_RNR_TXPOST_MAX_ITEM_LIST },
	{ false, 3, { 1, -1, -1, -1, DHD_RNR_BCMC_TXOFFL_PRIORITY },
	    DHD_RNR_TXPOST_MAX_ITEM_LIST },
	{ true, 4, { -1, -1, -1, -1, 1 }, DHD_RNR_TXPOST_MAX_ITEM_LIST },
	{ true, 5, { 1, 1, 1, 1, 1 },   DHD_RNR_TXPOST_MAX_ITEM_LIST },
	{ true, 6, { 1, 2, 4, 8, 1 },   DHD_RNR_TXPOST_MAX_ITEM_LIST },

	{ false, 7, { 1, 1, 1, 1, 1 },
	    { DHD_RNR_TXPOST_MAX_ITEM, DHD_RNR_TXPOST_MAX_ITEM,
	    DHD_RNR_TXPOST_MAX_ITEM, DHD_RNR_TXPOST_MAX_ITEM,
	    DHD_RNR_TXPOST_MAX_ITEM } }
};

/* dhd radio vs flowring policy */
dhd_flowring_policy_t dhd_rnr_policies[DHD_RNR_MAX_RADIOS] =
{
	{ .id = dhd_rnr_policy_global, .all_hw = TRUE },
	{ .id = dhd_rnr_policy_global, .all_hw = TRUE },
	{ .id = dhd_rnr_policy_global, .all_hw = TRUE }
};

struct dhd_runner_flowmgr; /* forward declaration */
struct dhd_runner_hlp;

typedef uint16
(dhd_runner_flowring_selector_fn_t)(struct dhd_runner_flowmgr *flowmgr,
	int ifidx, int prio, uint8 *mac, int staidx, bool d11ac, bool *is_hw_ring);

/* flow ring information maintained by dhd runner */
typedef struct dhd_runner_flring_dhd_info
{
	void* base_va;     /* flowring descriptor base virtual address */
} dhd_rdpa_flring_cache_t;

/* flow ring manager in dhd runner */
typedef struct dhd_runner_flowmgr
{
	int max_h2d_rings;  /* total h2d rings, including common rings */
	int max_bss;        /* total bcmc tx post flowrings */
	union {
	    int max_sta;    /* total ucast txpost rings per ac */
	    int max_uc_rings;
	};

	uint8 *hw_mem_virt_base_addr;      /* Reserved memory virtual base address */
	phys_addr_t hw_mem_phys_base_addr; /* Reserved memory physical base address */
	uint8 *hw_mem_addr;      /* Reserved memory virtual address */
	uint32 hw_mem_size;      /* Reserved memory size */

	rdpa_dhd_flring_cache_t *rdpa_flring_cache; /* Runner Flowring Cache */
	dhd_rdpa_flring_cache_t *dhd_flring_cache;  /* DHD Flowring Cache */

	dhd_flowring_policy_t    *policy;   /* Flowring policy */
	dhd_flowring_profile_t   *profile;  /* Flowring profile */

	void *flow_ids_map;                 /* flow id number pool */

	/* Below per AC arrays are reused for per TID */
	void *hw_id16_map[wme_ac_max + 1];  /* id number pool for runner managed rings */
	void *sw_id16_map[wme_ac_max + 1];  /* id number pool for DHD managed rings */

	int  hw_ring_cnt[wme_ac_max + 1];   /* runner managed flow ring count */
	int  sw_ring_cnt[wme_ac_max + 1];   /* DHD managed flow ring count */
	int  phy_items[wme_ac_max + 1];     /* physical flow ring max items (size) */

	dhd_runner_flowring_selector_fn_t *select_fn;    /* ring selection (hw or sw) function */
	struct dhd_runner_hlp *dhd_hlp;                  /* dhd runner helper object */
} dhd_runner_flowmgr_t;


#define DHD_RNR_TXCMPL_AUDIT                    1

/*
 * Runner's DHD helper supported features
 */
#if defined(RDPA_DHD_HELPER_FEATURE_NPLUSM)
#define RNR_DHD_HLPR_NPM
#endif /* RDPA_DHD_HELPER_FEATURE_NPLUSM */
#if defined(RDPA_DHD_HELPER_FEATURE_TXCOMPL_SUPPORT)
#define RNR_DHD_HLPR_TXCMPL2HOST
#endif /* RDPA_DHD_HELPER_FEATURE_TXCOMPL_SUPPORT */

#if defined(RDPA_DHD_HELPER_FEATURE_LLCSNAPHDR_SUPPORT)
#define RNR_DHD_HLPR_LLCSNAPHDR
#endif /* RNR_DHD_HLPR_LLCSNAPHDR */

#if defined(RDPA_DHD_HELPER_FEATURE_LBRAGGR_SUPPORT)
#define RNR_DHD_HLPR_LBRAGGR
#endif /* RDPA_DHD_HELPER_FEATURE_LBRAGGR_SUPPORT */

#if defined(RDPA_DHD_HELPER_FEATURE_MSGFORMAT_SUPPORT)
#define RNR_DHD_HLPR_MSGRINGFRMT
#endif /* RDPA_DHD_HELPER_FEATURE_MSGFORMAT_SUPPORT */

#if defined(RDPA_DHD_HELPER_FEATURE_FAST_FLOWRING_DELETE_SUPPORT)
#define RNR_DHD_HLPR_FFRD
#endif /*  RDPA_DHD_HELPER_FEATURE_FAST_FLOWRING_DELETE_SUPPORT */

#if defined(MSGBUF_WI_COMPACT) && defined(RNR_DHD_HLPR_MSGRINGFRMT)
/*
 * Runner supports only two formats
 *   0: Legacy
 *   1: CWI (TxPost::CWI64, RxPost::CWI32, TXCPL::CWI32, RXCPL::CWI32)
 */
#define DOR_RINGTYPE_TO_RNR_RINGFMT(type)   (((type) == MSGBUF_WI_WI64) ? 0 : 1)
#else /* !MSGBUF_WI_COMPACT || !RNR_DHD_HLPR_MSGRINGFRMT */
#define DOR_RINGTYPE_TO_RNR_RINGFMT(type)    0
#endif /* !MSGBUF_WI_COMPACT || !RNR_DHD_HLPR_MSGRINGFRMT */

#if defined(RDPA_DHD_HELPER_FEATURE_BACKUP_QUEUE_SUPPORT)
#define RNR_DHD_HLPR_BKUPQUEUE
#define DHD_RNR_DEF_BKUPQ                   1
#define DHD_RNR_BKUPQ(dhd_hlp)              ((dhd_hlp)->rnr_en_feat.bkupq == 1)
#define DHD_RNR_PHY_RING_SIZE(cache)        (cache)->phy_ring_size
#else /* !RDPA_DHD_HELPER_FEATURE_BACKUP_QUEUE_SUPPORT */
#define DHD_RNR_DEF_BKUPQ                   0
#define DHD_RNR_BKUPQ(dhd_hlp)              0
#define DHD_RNR_PHY_RING_SIZE(cache)        (cache)->items
#endif /* !RDPA_DHD_HELPER_FEATURE_BACKUP_QUEUE_SUPPORT */

#if defined(RDPA_DHD_HELPER_FEATURE_HWA_WAKEUP_SUPPORT)
#define RNR_DHD_HLPR_HWA_WAKEUP
#endif /* RDPA_DHD_HELPER_FEATURE_HWA_WAKEUP_SUPPORT */

#define RNR_DHD_HLPR_FEATURE_STS_STRING(dhd_hlp, feat) \
	(dhd_hlp->rnr_sup_feat.feat ? \
	    (dhd_hlp->rnr_en_feat.feat ? #feat" 1" : #feat" 0") :"")

/*
 * +---------------------------------------------------------------------
 *           Section: Instance of a dhd runner helper object per radio
 *
 * +---------------------------------------------------------------------
 */
/* dhd runner helper object structure */
typedef struct dhd_runner_hlp {
	dhd_pub_t *dhd;                       /* DHD public object */

	int cpu_queue_id;                     /* Runner -> CPU Queue id */
	int cpu_port;                         /* CPU port */
	rdpa_cpu_reason trap_reason;          /* Runner flow miss reason */
	struct tasklet_struct dhd_rx_tasklet; /* Tasklet for processing RXCMPL and TXSTS */
	rdpa_dhd_init_cfg_t dhd_init_cfg;     /* init config object */
	bdmf_object_handle dhd_helper_obj;    /* dhd helper object in runner */
	bdmf_object_handle dhd_mcast_obj;     /* multicast object in runner */
	bdmf_object_handle cpu_obj;           /* cpu object in runner */

	struct pci_dev *pci_dev;               /* PCI device pointer */
	bool proc_cpuq_in_dpc;                 /* process cpu queue in DPC instead of Tasklet */
	bool ipend;                            /* runner cpuq interrupt pending */
	dhd_dma_buf_t flring_cache_dma_buf;    /* dma buffer object for flow ring cache */
	dhd_runner_flowmgr_t flowmgr;          /* Flow manager object */
	dhd_runner_ring_cfg_t rxpost_ring_cfg; /* RXPOST ring configuration object */
	dhd_runner_ring_cfg_t txpost_ring_cfg; /* TXPOST ring configuration object */
	dhd_runner_ring_cfg_t rxcmpl_ring_cfg; /* RXCMPL ring configuration object */
	dhd_runner_ring_cfg_t txsts_ring_cfg;  /* TXSTS ring configuration object */
	dhd_dma_buf_t coherent_mem_pool;       /* Coherent memory pool object */
	dhd_helper_feat_t rnr_sup_feat;        /* Features supported by Runner dhd helper */
	dhd_helper_feat_t rnr_en_feat;         /* Features enabled in Runner dhd helper */

	/* local counters */
	ulong h2r_txpost_notif;        /* Host notifies Runner to post tx packets */
	ulong h2r_rx_compl_notif;      /* Host notifies Runner to process D2H RxCompl */
	ulong h2r_tx_compl_notif;      /* Host notifies Runner to process D2H TxCompl */
	ulong r2h_rx_compl_req;        /* Runner requests Host to receive a packet */
	ulong r2h_tx_compl_req;        /* Runner requests Host to free a packet */
	ulong r2h_wake_dngl_req;       /* Runner requests Host to wake dongle */

#ifdef DHD_RNR_MEM_ALLOC_AUDIT
	uint32 alloc_cnt[DHD_RNR_MAX_MEM_ALLOC_TYPES];   /* memory allocation count */
	uint32 alloc_size[DHD_RNR_MAX_MEM_ALLOC_TYPES];  /* allocated memory size */
#endif /* DHD_RNR_MEM_ALLOC_AUDIT */
} dhd_runner_hlp_t;


/*
 * +---------------------------------------------------------------------
 *           Section: Local Function Prototypes
 *
 * +---------------------------------------------------------------------
 */
static int  dhd_runner_wake_dongle_isr(int irq, void *data);

/* Runner MISS Rx Path via CPU queue */
static bool dhd_runner_process_all_cpu_queues(
	dhd_runner_hlp_t *dhd_hlp, bool resched);
static bool dhd_runner_process_cpu_queue_rxpkt(
	dhd_runner_hlp_t *dhd_hlp, int bound);
static bool dhd_runner_process_cpu_queue_txcmpl(
	dhd_runner_hlp_t *dhd_hlp, int bound);
static void dhd_runner_rx_tasklet_handler(unsigned long data);
static void dhd_runner_cpu_queue_isr(long data);
static int  dhd_runner_cfg_cpu_queue(dhd_runner_hlp_t *dhd_hlp,
	int cpu_queue_size);

/* DHD dma buffer allocation and base address configuration in RDPA */
static int  dhd_runner_dma_buf_init(dhd_runner_hlp_t *dhd_hlp, void *osh,
	dhd_runner_dma_buf_t buf_type, dhd_dma_buf_t *dma_buf);

/* Setup the PCIE RC match and Ubus remap registers using PCI#_BASE */
static int dhd_runner_pcie_init(dhd_runner_hlp_t *dhd_hlp,
	struct pci_dev *pci_dev);

/* Initialized dongle wakeup registter information for runner/rdpa/rdd */
static int dhd_runner_dongle_wakeup_init(dhd_runner_hlp_t *dhd_hlp);

/* Wakeup dongle using the wakeup register and value provided */
static int dhd_runner_wakeup_dongle(dhd_runner_hlp_t *dhd_hlp, uint32 db_reg, uint32 db_val);

/* Setup llcsnaphdr runner feature */
static int dhd_runner_llcsnaphdr_init(dhd_runner_hlp_t *dhd_hlp);

/* Setup txcmpl2host runner feature */
static int dhd_runner_txcmpl2host_init(dhd_runner_hlp_t *dhd_hlp,
	bdmf_object_handle mo);

/* Initialize dhd_runner sw features */
static int dhd_runner_swfeatures_init(struct dhd_runner_hlp *dhd_hlp);

/* DHD proto layer init completed, configure Runner helper object */
static int  dhd_runner_init(dhd_runner_hlp_t *dhd_hlp, struct pci_dev *pci_dev);

/* DHD insmod and rmmod callbacks */
static int  dhd_helper_attach(dhd_runner_hlp_t *dhd_hlp, void *dhd);
static void dhd_helper_detach(dhd_runner_hlp_t *dhd_hlp);

static INLINE int dhd_runner_wakeup_init(bcmpcie_soft_doorbell_t *soft_doorbell,
	phys_addr_t wakeup_paddr, uint32 wakeup_val32);

/* Allocate a flowring buffer in carved or cached memory */
static int dhd_runner_flowring_alloc(dhd_runner_flowmgr_t *flowmgr,
	dhd_wme_ac_t wme_ac, bool force_dhd);

static void dhd_runner_flring_cache_enable(dhd_runner_hlp_t *dhd_hlp,
	uint32_t ringid, int enable);


/* Persistent Scratch Pad area  access */
static int dhd_runner_key_get(int radio_idx, int key_id, char *buff, int len);
static int dhd_runner_key_set(int radio_idx, int key_id, char *buff, int len);

/* Flow ring profile management */
static dhd_flowring_profile_t* dhd_runner_profile_init(struct dhd_runner_hlp *dhd_hlp);

/* Flow ring policy management */
static dhd_flowring_policy_t* dhd_runner_policy_init(struct dhd_runner_hlp *dhd_hlp);
static void dhd_runner_get_policy_str(dhd_flowring_policy_t *policy,
	char    *buf, int len);
static INLINE int dhd_runner_str_to_idx(char *str, const char **list, int max);


/* memory allocation */
static void dhd_runner_free_mem(dhd_runner_hlp_t *dhd_hlp,
	dhd_dma_buf_t *dma_buf, uint32 type);
static void* dhd_runner_alloc_mem(dhd_runner_hlp_t *dhd_hlp,
	dhd_dma_buf_t *dma_buf, uint32 type);
static uint32 dhd_runner_get_ring_mem_alloc_type(dhd_runner_hlp_t *dhd_hlp,
	uint16 ringid);

/* Tx Offload Setting */
static int dhd_runner_txoffl_init(struct dhd_runner_hlp *dhd_hlp,
	dhd_runner_ring_cfg_t *ring_cfg);

/* Rx Offload Setting */
static int dhd_runner_rxoffl_init(struct dhd_runner_hlp *dhd_hlp,
	dhd_runner_ring_cfg_t *ring_cfg);

/* dhd runner IOVAR processing */
static int dhd_runner_iovar_get_profile(struct dhd_runner_hlp *dhd_hlp,
	char *buf, int buflen);
static int dhd_runner_iovar_get_policy(struct dhd_runner_hlp *dhd_hlp,
	char *buf, int buflen);
static int dhd_runner_iovar_set_profile(dhd_runner_hlp_t *dhd_hlp,
	char *buf, int buflen);
static int dhd_runner_iovar_set_policy(dhd_runner_hlp_t *dhd_hlp,
	char *buf, int buflen);
static int dhd_runner_iovar_get_rxoffl(dhd_runner_hlp_t *dhd_hlp,
	char *buf, int bufflen);
static int dhd_runner_iovar_set_rxoffl(dhd_runner_hlp_t *dhd_hlp,
	char *buf, int buflen);
static int dhd_runner_iovar_dump(dhd_runner_hlp_t *dhd_hlp, char *buff,
	int bufflen);
static int dhd_runner_iovar_get_rnr_status(dhd_runner_hlp_t *dhd_hlp,
	char *buf, int buflen);
static int dhd_runner_iovar_get_rnr_stats(dhd_runner_hlp_t *dhd_hlp,
	char *buf, int buflen);

typedef int
(dhd_runner_iovar_fn_t)(dhd_runner_hlp_t *dhd_hlp, char *buf, int buflen);


dhd_runner_iovar_fn_t * dhd_rnr_iovar_table[DHD_RNR_MAX_IOVARS][2] = {
	{
	    dhd_runner_iovar_get_profile,    /* DHD_RNR_IOVAR_PROFILE, GET */
	    dhd_runner_iovar_set_profile,    /* DHD_RNR_IOVAR_PROFILE, SET */
	},
	{
	    dhd_runner_iovar_get_policy,    /* DHD_RNR_IOVAR_POLICY, GET */
	    dhd_runner_iovar_set_policy,    /* DHD_RNR_IOVAR_POLICY, SET */
	},
	{
	    dhd_runner_iovar_get_rxoffl,    /* DHD_RNR_IOVAR_RXOFFL, GET */
	    dhd_runner_iovar_set_rxoffl,    /* DHD_RNR_IOVAR_RXOFFL, SET */
	},
	{
	    dhd_runner_iovar_get_rnr_stats, /* DHD_RNR_IOVAR_RNR_STATS, GET */
	    NULL,                           /* DHD_RNR_IOVAR_RNR_STATS, SET */
	},
	{
	    dhd_runner_iovar_dump,          /* DHD_RNR_IOVAR_DUMP, GET */
	    NULL,                           /* DHD_RNR_IOVAR_DUMP, SET */
	},
	{
	    dhd_runner_iovar_get_rnr_status,     /* DHD_RNR_IOVAR_STATUS, GET */
	    NULL,                                /* DHD_RNR_IOVAR_STATUS, SET */
	},
};

static int dhd_rnr_mcast_obj_ref_cnt = -1;

/*
 * +----------------------------------------------------------------------------
 *           Section: DHD Runner memory allocation
 *
 * memory allocation functions for dma idx, ring descriptor memories
 *
 * memory types
 *   DHD: cached memory used by DHD driver only. Not shared by Runner
 *        uses DMA_ALLOC_CONSISTENT/DMA_FREE_CONSISTENT osl functions
 *        this internally uses kmalloc/kfree with flush_cache operations
 *        sw TX flowrings, ctrlpost, ctrlcmpl rings
 *
 *   RNR: un cached memory used only by Runner.
 *        uses dma_alloc_coherent/dma_free_coherent functions
 *        cached flowrings, ring DMA index buffers
 *        applicable for arm/arm64 platforms
 *
 *   RNRVA: Used by Runner only (address <256MB).
 *           rdp uses virt_to_phys on this memory to get physical address
 *           which dma_alloc_coherent can not provide, so a different type
 *           uses kmalloc/kfree with GFP_DMA flag
 *           ring descriptor memory for txcmpl,rxcmpl,rxpost
 *           applicable for arm/arm64 platforms
 *
 * +----------------------------------------------------------------------------
 */

/**
 * Allocate memory for the ring descriptor and indices buffers
 */
void*
dhd_runner_alloc_mem(dhd_runner_hlp_t *dhd_hlp, dhd_dma_buf_t *dma_buf,
	uint32 type)
{
	int alloced;
	dhd_pub_t *dhd;

	dhd = dhd_hlp->dhd;
	dma_buf->va = NULL;
	dma_buf->dmah = NULL;

	if (type == DHD_RNR_MEM_ALLOC_TYPE_COHERENT) {
	    dhd_dma_buf_t* coherent_pool = &dhd_hlp->coherent_mem_pool;
	    int aligned_len;
	    uint32 pa_low;

	    aligned_len = (dma_buf->len + DHD_RNR_COHERENT_MEM_POOL_ALIGN_MASK);
	    aligned_len &= ~(DHD_RNR_COHERENT_MEM_POOL_ALIGN_MASK);

	    if ((coherent_pool->va == NULL) ||
	        (coherent_pool->len < (coherent_pool->_alloced + aligned_len))) {
	        /* No Memory in the Local Coherent Pool */
	        ARCH_ALLOC_COHERENT_MEM(dhd, dma_buf);
	    } else {
	        /* Get from Local Coherent Pool */
	        dma_buf->va = coherent_pool->va + coherent_pool->_alloced;
	        pa_low = PHYSADDRLO(coherent_pool->pa) + coherent_pool->_alloced;
	        PHYSADDRLOSET((dma_buf)->pa, pa_low);
	        dma_buf->_alloced = aligned_len;
	        coherent_pool->_alloced += aligned_len;
	        DHD_INFO(("[COHERENT_POOL_%d] Allocated 0x%x size 0x%p, 0x%lx from pool 0x%p\r\n",
	            dhd_hlp->dhd->unit, dma_buf->_alloced, dma_buf->va, PHYSADDRLO(dma_buf->pa),
	            coherent_pool->va));
	    }
	} else if (type == DHD_RNR_MEM_ALLOC_TYPE_KMALLOC) {
	    dma_buf->va = kmalloc(dma_buf->len, GFP_ATOMIC | __GFP_ZERO | GFP_DMA);
	    ARCH_SET_DMA_BUF_PA(dma_buf);
	} else	{
	    dma_buf->va = DMA_ALLOC_CONSISTENT(dhd->osh, dma_buf->len,
	        4, &alloced, &dma_buf->pa, &dma_buf->dmah);
	}

	if (dma_buf->va != NULL) {
	    memset(dma_buf->va, 0, dma_buf->len);
	    if (type != DHD_RNR_MEM_ALLOC_TYPE_COHERENT)
	        OSL_CACHE_FLUSH(dma_buf->va, dma_buf->len);

	    DHD_RNR_INC_MEM_ALLOC_CNTRS(dhd_hlp, type, dma_buf->len);
	}

	return dma_buf->va;
}

/**
 * Free the memory allocated for the ring descriptor and indices buffers
 */
void
dhd_runner_free_mem(dhd_runner_hlp_t *dhd_hlp, dhd_dma_buf_t *dma_buf,
	uint32 type)
{

	if (dma_buf->va != NULL) {
	    dhd_pub_t *dhd;

	    dhd = dhd_hlp->dhd;
	    if (type == DHD_RNR_MEM_ALLOC_TYPE_COHERENT) {
	        dhd_dma_buf_t* coherent_pool = &dhd_hlp->coherent_mem_pool;

	        if ((dma_buf == coherent_pool) ||
	            ((dma_buf->va <  coherent_pool->va) &&
	            (dma_buf->va >= (coherent_pool->va + coherent_pool->len)))) {
	            /* Local Coherent pool itself or Outside of Pool */
	            ARCH_FREE_COHERENT_MEM(dhd, dma_buf);
	        }
	    } else if (type == DHD_RNR_MEM_ALLOC_TYPE_KMALLOC) {
	        kfree(dma_buf->va);
	    } else {
	        DMA_FREE_CONSISTENT(dhd->osh, dma_buf->va, dma_buf->len,
	            dma_buf->pa, dma_buf->dmah);
	    }
	    DHD_RNR_DEC_MEM_ALLOC_CNTRS(dhd_hlp, type, dma_buf->len);
	    memset(dma_buf, 0, sizeof(dhd_dma_buf_t));
	}

	return;
}

/**
 * Returns the memory type to be allocated based on ring id
 */
uint32
dhd_runner_get_ring_mem_alloc_type(dhd_runner_hlp_t *dhd_hlp,
	uint16 ringid)
{
	uint32 type = DHD_RNR_MEM_TYPE_DHD;

	if (DHD_RNR_RX_OFFLOAD(dhd_hlp)) {
	    if ((ringid == BCMPCIE_H2D_MSGRING_RXPOST_SUBMIT) ||
	        (ringid == BCMPCIE_D2H_MSGRING_RX_COMPLETE))
	        type = DHD_RNR_MEM_TYPE_RNRVA;
	}

	if (DHD_RNR_TXSTS_OFFLOAD(dhd_hlp)) {
	    if (ringid == BCMPCIE_D2H_MSGRING_TX_COMPLETE)
	        type = DHD_RNR_MEM_TYPE_RNRVA;
	}

	return type;

}
/*
 * +----------------------------------------------------------------------------
 *           Section: DHD requests Runner to post a packet descriptor
 * +----------------------------------------------------------------------------
 */
int
dhd_runner_txpost(dhd_pub_t *dhd, void *txp, uint32 ifindex)
{
	int rc;
	int pktlen = PKTLEN(dhd->osh, txp);
	rdpa_dhd_tx_post_info_t info = {};
#if (defined(CONFIG_BCM_SPDSVC) || defined(CONFIG_BCM_SPDSVC_MODULE))
	spdsvcHook_transmit_t spdsvc_transmit;
#endif

	info.radio_idx = dhd->unit;
	info.flow_ring_id = DHD_PKT_GET_FLOWID(txp);
	info.ssid_if_idx = ifindex;

#if (defined(CONFIG_BCM_SPDSVC) || defined(CONFIG_BCM_SPDSVC_MODULE))
	spdsvc_transmit.pNBuff = txp;
	spdsvc_transmit.dev = NULL;
	spdsvc_transmit.header_type = SPDSVC_HEADER_TYPE_ETH;
	spdsvc_transmit.phy_overhead = WL_SPDSVC_OVERHEAD;

	info.is_spdsvc_setup_packet = dhd_runner_spdsvc_transmit(&spdsvc_transmit);
	if (info.is_spdsvc_setup_packet < 0)
	{
	    /* In case of error, NBuff will be free by spdsvc */
	    return 0;
	}
#else
	info.is_spdsvc_setup_packet = 0;
#endif

	rc = rdpa_dhd_helper_send_packet_to_dongle(txp, pktlen, &info);

	if (rc != 0) {
	    DHD_TRACE(("dor%d %s (0x%p, %d, 0x%p) returned %d\r\n",
	        dhd->unit, "rdpa_dhd_helper_send_packet_to_dongle",
	        txp, pktlen, &info, rc));
	}

	return rc;
}

/*
 * +----------------------------------------------------------------------------
 *           Section: Runner wakes Dongle after updating WR index in DDR
 * +----------------------------------------------------------------------------
 */
static int
dhd_runner_wake_dongle_isr(int irq, void *data)
{
	dhd_runner_hlp_t *dhd_hlp = (dhd_runner_hlp_t *)data;

	rdpa_dhd_helper_doorbell_interrupt_clear(dhd_hlp->dhd->unit);

	if (dhd_hlp->dhd->busstate == DHD_BUS_DOWN) {
	    DHD_INFO(("%s: dhd%d_rnr: PCIe bus down, ret\n",
	        __FUNCTION__, dhd_hlp->dhd->unit));
	} else {
	    dhd_runner_request(dhd_hlp, R2H_WAKE_DNGL_REQUEST, 0, 0);
	}

	return BDMF_IRQ_HANDLED;
}


#ifdef XRDP
static struct sk_buff *skb_alloc(rdpa_cpu_rx_info_t *info)
{
	struct sk_buff *skb;

	skb = skb_header_alloc();
	if (unlikely(!skb))
	    return NULL;

	skb_headerinit(BCM_PKT_HEADROOM + info->data_offset,
#if defined(CC_NBUFF_FLUSH_OPTIMIZATION)
	    SKB_DATA_ALIGN(info->size + BCM_SKB_TAILROOM + info->data_offset),
#else /* CC_NBUFF_FLUSH_OPTIMIZATION */
	    BCM_MAX_PKT_LEN - info->data_offset,
#endif /* CC_NBUFF_FLUSH_OPTIMIZATION */
	    skb, info->data + info->data_offset, bdmf_sysb_recycle, 0, NULL);

	skb_trim(skb, info->size);
	skb->recycle_flags &= SKB_NO_RECYCLE; /* no skb recycle,just do data recyle */

	return (bdmf_sysb)SKBUFF_2_PNBUFF(skb);
}
#endif /* XRDP */


/*
 * +----------------------------------------------------------------------------
 *
 *         Section: Dongle to Host Rx Completion processing Runner.
 *
 * A miss in Runner will result in Runner forwarding the packet to DHD via a
 * dedicated CPU queue and raising an interrupt.
 *
 * dhd_runner_isr() will be invoked and a tasklet will be scheduled to
 * process all packets in this Runner to CPU queue in the context of the dhd.
 *
 * +----------------------------------------------------------------------------
 */

static int dhd_runner_skb_get(dhd_runner_hlp_t *dhd_hlp, rdpa_cpu_port port,
	bdmf_index queue, bdmf_sysb *sysb, rdpa_cpu_rx_info_t *info)
{
	int rc;

#if defined(CONFIG_BCM_FCACHE_CLASSIFICATION_BYPASS) && defined(XRDP) && !defined(CONFIG_BCM963158)
	uint32_t flow_key = 0;
	fc_class_ctx_t fc_key;
#endif

	rc = rdpa_cpu_packet_get(port, queue, info);
	if (rc)
	    return rc;

#ifdef XRDP
#if defined(CONFIG_BCM_FCACHE_CLASSIFICATION_BYPASS) && !defined(CONFIG_BCM963158)
	if (!info->is_exception)
	{
	    flow_key = *(uint32_t*)((uint8_t*)info->data + info->data_offset);
	    fc_key.word = flow_key;
	    info->src_port = fc_key.id.src_port;
	    fc_key.id.src_port = 0x0;
	    flow_key = fc_key.word;
	    info->data_offset += L2_FLOW_P_LEN;
	    info->size -= L2_FLOW_P_LEN;
	}
#endif

	*sysb = (bdmf_sysb*)skb_alloc(info);
	info->reason_data = info->dest_ssid;
#else /* XRDP */
	/* Extract the ssid ifidx from mapped hw_port */
	info->reason_data = WLAN_NETDEVPATH_SSID(info->reason_data);

	/* create a sysb and initialize it with packet data & len */
	*sysb = bdmf_sysb_header_alloc(bdmf_sysb_skb, (uint8_t *)info->data,
	    info->size, 0,
	    info->rx_csum_verified?BDMF_RX_CSUM_VERIFIED_MASK:0);
#endif /* XRDP */

	if (!*sysb) {
	    DHD_ERROR(("%s: Failed to allocate skb header\n", __FUNCTION__));
	    bdmf_sysb_databuf_free((uint8_t *)info->data, 0);
	    return -1;
	}

	*sysb = bdmf_sysb_2_fkb_or_skb(*sysb);

#if defined(CONFIG_BCM_FCACHE_CLASSIFICATION_BYPASS) && defined(XRDP) && !defined(CONFIG_BCM963158)
	if (flow_key)
	    ((struct sk_buff *)(*sysb))->fc_ctxt = flow_key;
#endif

	return 0;
}

/**
 * function to processes RX packet from the Runner to DHD cpu queue
 * When Runner posts buffers in the H2D RxPost common rings, these buffers are
 * not in cache and hence there is no need to cache invalidate again before
 * invoking R2H_RX_COMPL_REQUEST ops
 */
static bool
dhd_runner_process_cpu_queue_rxpkt(dhd_runner_hlp_t *dhd_hlp, int bound)
{
	int rc = 0;
	struct sk_buff *skb = NULL;
	rdpa_cpu_rx_info_t info = {};
	int pkts = 0;

	ASSERT(dhd_hlp != NULL);

	/* If RxOffload is not enabled, Runner will not send rxpkt messages on this queue */
	if (!DHD_RNR_RX_OFFLOAD(dhd_hlp))
	    return FALSE;

	while (!bound || (pkts < bound)) {

	    /* Fetch each packet from Runner to Host cpu queue */
	    rc = dhd_runner_skb_get(dhd_hlp, dhd_hlp->cpu_port,
	        dhd_hlp->cpu_queue_id, (bdmf_sysb *)&skb, &info);
	    if (rc)
	        break;

	    /* Hand off to DHD to process packets missed in Runner */
	    dhd_runner_request(dhd_hlp, R2H_RX_COMPL_REQUEST,
	                       (unsigned long)skb, (unsigned long)info.reason_data);

	    pkts++;
	}

	return (bound && (pkts >= bound));
}

/**
 * Function that processes Tx Complete packets from the dongle coming through
 * Runner to DHD cpu queue
 */
static bool
dhd_runner_process_cpu_queue_txcmpl(dhd_runner_hlp_t *dhd_hlp, int bound)
{
	int rc = 0;
	host_txbuf_cmpl_t *txsts;
	dhd_runner_txsts_t rnrtxsts;
	rdpa_dhd_complete_data_t dhd_complete_data = {dhd_hlp->dhd->unit, 0, 0, 0};
	int pkts = 0;

	ASSERT(dhd_hlp != NULL);

	/* If TxOffload is not enabled, Runner will not send txcmpl messages on this queue */
	if (!DHD_RNR_TXSTS_OFFLOAD(dhd_hlp))
	    return FALSE;

	while (!bound || (pkts < bound)) {

	    /* Fetch each tx complete packet from Runner to Host DHD complete queue */
	    rc = rdpa_dhd_helper_dhd_complete_message_get(&dhd_complete_data);

	    if (rc)
	        break;

	    rnrtxsts.pkt = NULL;
	    txsts = &rnrtxsts.dngl_txsts;
	    txsts->cmn_hdr.request_id = htonl(dhd_complete_data.request_id);

#if defined(RNR_DHD_HLPR_TXCMPL2HOST)
	    if ((dhd_complete_data.buf_type == RDPA_DHD_TX_POST_HOST_BUFFER_VALUE) &&
	        (dhd_complete_data.txp)) {
	        rnrtxsts.pkt = dhd_complete_data.txp;
	    }
#endif /* RNR_DHD_HLPR_TXCMPL2HOST */

	    if (dhd_hlp->txsts_ring_cfg.type) {

#if defined(DHD_RNR_TXCMPL_AUDIT)
	        /*
	         * Filter out the spurious/unwanted Tx Complete messages from Runner
	         */
	        if (dhd_complete_data.buf_type == RDPA_DHD_TX_POST_HOST_BUFFER_VALUE) {
	            if ((dhd_complete_data.txp == (void*)-1) || (dhd_complete_data.txp == NULL)) {
	                DHD_ERROR(("dhd%d_rnr Discard TxComplete: pkt[0x%p], req_id[0x%x]\r\n",
	                    dhd_hlp->dhd->unit, dhd_complete_data.txp,
	                    ltoh32(txsts->cmn_hdr.request_id)));
	                continue;
	            }
	        } else if (ltoh32(txsts->cmn_hdr.request_id) == 0) {
	            DHD_ERROR(("dhd%d_rnr Discard TxComplete: req_id[0x%x], pkt[0x%p]\r\n",
	                dhd_hlp->dhd->unit, ltoh32(txsts->cmn_hdr.request_id),
	                dhd_complete_data.txp));
	            continue;
	        }
#endif /* DHD_RNR_TXCMPL_AUDIT */

	        dhd_runner_request(dhd_hlp, R2H_TX_COMPL_REQUEST, (unsigned long)&rnrtxsts, 0);
	    } else {
	        /* legacy message formats */
	        txsts->tx_status = htons(dhd_complete_data.status);
	        txsts->compl_hdr.flow_ring_id = htons(dhd_complete_data.flow_ring_id);
	        txsts->cmn_hdr.msg_type =  MSG_TYPE_TX_STATUS;
	        txsts->cmn_hdr.if_id = 0;
	        RPR2("req_id = 0x%08x, status = 0x%04x flow_ring_id = 0x%04x\r\n",
	            txsts->cmn_hdr.request_id, txsts->tx_status,
	            txsts->compl_hdr.flow_ring_id);


#if defined(DHD_RNR_TXCMPL_AUDIT)
	        /*
	         * Filter out the spurious/unwanted Tx Complete messages
	         *
	         * Note: RNR might send them during re-load of DHD driver as RNR doesn't support
	         *           DHD unload and reload yet
	         */
	        if ((ltoh32(txsts->cmn_hdr.request_id) == 0)||
	            (ltoh16(txsts->compl_hdr.flow_ring_id) < FLOW_RING_COMMON)||
	            (ltoh16(txsts->compl_hdr.flow_ring_id) > dhd_hlp->flowmgr.max_h2d_rings) ||
#if defined(RNR_DHD_HLPR_TXCMPL2HOST)
	            ((dhd_complete_data.buf_type == RDPA_DHD_TX_POST_HOST_BUFFER_VALUE) &&
	             ((dhd_complete_data.txp == (void*)-1) || (dhd_complete_data.txp == NULL)))||
#endif /* RNR_DHD_HLPR_TXCMPL2HOST */
	            (ltoh16(txsts->tx_status) != 0)) {
	            DHD_ERROR(("dhd%d_rnr Discard TxComplete: req_id[0x%x], flow_ring_id[0x%x]",
	                dhd_hlp->dhd->unit, ltoh32(txsts->cmn_hdr.request_id),
	                ltoh16(txsts->compl_hdr.flow_ring_id)));
	            DHD_ERROR((" status [0x%x] buf_type [%d] txp [0x%p]\r\n",
	                ltoh16(txsts->tx_status),
	                dhd_complete_data.buf_type, dhd_complete_data.txp));
	            continue;
	        }
#endif /* DHD_RNR_TXCMPL_AUDIT */
	        dhd_runner_request(dhd_hlp, R2H_TX_COMPL_REQUEST, (unsigned long)&rnrtxsts, 0);
	    }
	    pkts++;
	}

	return (bound && (pkts >= bound));
}

/**
 * dpc thread function that processes packets from the Runner to DHD cpu queue.
 * Processes available packets to the max bound limits and enables interrupts
 * if there is no more packets to process
 * Input: resched is ignored currently
 */
static bool
dhd_runner_process_all_cpu_queues(struct dhd_runner_hlp *dhd_hlp, bool resched)
{

	/* Process max of rxbound RxComplete packets */
	resched = dhd_runner_process_cpu_queue_rxpkt(dhd_hlp, dhd_rxbound);

	/* Process max of txbound TxComplete packets */
	resched |= dhd_runner_process_cpu_queue_txcmpl(dhd_hlp, dhd_txbound);

	if (resched == FALSE) {
	    /* Re-enable interrupts on Runner to Host cpu queue */
	    dhd_hlp->ipend = FALSE;
	    rdpa_cpu_int_enable(dhd_hlp->cpu_port, dhd_hlp->cpu_queue_id);
	}

	return resched;
}

/**
 * dpc function that processes packets from the Runner to DHD cpu queue.
 */
bool dhd_runner_process_cpu_queue(struct dhd_runner_hlp *dhd_hlp, bool resched)
{

	ASSERT(dhd_hlp != NULL);

	if ((dhd_hlp->proc_cpuq_in_dpc == TRUE) && (dhd_hlp->ipend == TRUE)) {

	    resched = dhd_runner_process_all_cpu_queues(dhd_hlp, resched);
	}

	return resched;
}

/**
 * tasklet that processes packets from the Runner to DHD cpu queue
 * Processes all available packets before enabling the interrupts
 */
static void
dhd_runner_rx_tasklet_handler(unsigned long data)
{
	dhd_runner_hlp_t *dhd_hlp = (dhd_runner_hlp_t *)data;

	ASSERT(dhd_hlp != NULL);

	if (dhd_hlp->dhd->busstate == DHD_BUS_DOWN) {
	    DHD_INFO(("%s: dhd%d_rnr: PCIe bus down, ret\n",
	        __FUNCTION__, dhd_hlp->dhd->unit));
	    return;
	}

	if (dhd_runner_process_all_cpu_queues(dhd_hlp, FALSE)) {
	    /* There may be more packets pending, reschedule */
	    tasklet_schedule(&dhd_hlp->dhd_rx_tasklet);
	}

	return;
}

/**
 * ISR handler attached to the Runner to Host cpu queue
 */
static void
dhd_runner_cpu_queue_isr(long data)
{
	dhd_runner_hlp_t *dhd_hlp = (dhd_runner_hlp_t *)data;
	int cpu_queue_id;

	ASSERT(dhd_hlp != NULL);

	cpu_queue_id = dhd_hlp->cpu_queue_id;

	/* Disable and acknowledge the interrupt */
	rdpa_cpu_int_disable(dhd_hlp->cpu_port, cpu_queue_id);
	rdpa_cpu_int_clear(dhd_hlp->cpu_port, cpu_queue_id);

	dhd_hlp->ipend = TRUE;

	if (dhd_hlp->proc_cpuq_in_dpc == TRUE) {
	    /* Process packets in the dhd dpc context */
	    dhd_sched_dpc(dhd_hlp->dhd);
	} else {
	    tasklet_schedule(&dhd_hlp->dhd_rx_tasklet);
	}
}

static int cpu_queue_id_get(bdmf_object_handle cpu_obj, int radio_idx)
{
#ifdef XRDP
	bdmf_number num_queues;
	int rc;

	/* In XRDP, we use dedicated cpu objects per radio, as allocated by WFD. When
	 * create cpu object, WFD will allow to configure 1 extra queue for DHD
	 * exception traffic, which will be the last queue in it's range. If WFD is not
	 * configured, use same logic as in RDP.
	 */
	rc = rdpa_cpu_num_queues_get(cpu_obj, &num_queues);

	if (rc != 0) {
	    DHD_ERROR(("dor%d rdpa_cpu_num_queues_get(0x%p, 0x%p) returned %d\r\n",
	        radio_idx, cpu_obj, &num_queues, rc));
	    num_queues = 0;
	}
	return (int)num_queues - 1;
#else
	int cpu_queues[] = {RDPA_DHD_HELPER_1_CPU_QUEUE,
	    RDPA_DHD_HELPER_2_CPU_QUEUE,
	    RDPA_DHD_HELPER_3_CPU_QUEUE};

	return cpu_queues[radio_idx];
#endif /* XRDP */
}

/**
 * Configure the Runner to DHD cpu queue
 */
static int
dhd_runner_cfg_cpu_queue(struct dhd_runner_hlp *dhd_hlp, int init)
{
	int rc;
#ifndef XRDP
	int i;
#endif
	rdpa_cpu_rxq_cfg_t rxq_cfg = {};

	int cpu_reasons[] = {rdpa_cpu_rx_reason_pci_ip_flow_miss_1,
	    rdpa_cpu_rx_reason_pci_ip_flow_miss_2,
	    rdpa_cpu_rx_reason_pci_ip_flow_miss_3};
	rdpa_cpu_port cpu_port;
	int cpu_queue_size = init ? RDPA_DHD_HELPER_CPU_QUEUE_SIZE : 0;

	if (!cpu_queue_size)
	{
	    /* Cpu object is not configured yet, nothing to do */
	    if (!dhd_hlp->cpu_obj)
	        return 0;

	    rc = rdpa_cpu_index_get(dhd_hlp->cpu_obj, &cpu_port);
	    if (rc != 0) {
	        DHD_ERROR(("dor%d rdpa_cpu_index_get(0x%p, 0x%p) returned %d\r\n",
	            dhd_hlp->dhd->unit, dhd_hlp->cpu_obj, &cpu_port, rc));
	    }

	    rdpa_cpu_int_disable(dhd_hlp->cpu_port, dhd_hlp->cpu_queue_id);
	    rdpa_cpu_int_clear(dhd_hlp->cpu_port, dhd_hlp->cpu_queue_id);
	    tasklet_kill(&dhd_hlp->dhd_rx_tasklet);

	    rc = rdpa_cpu_rxq_flush_set(dhd_hlp->cpu_obj, dhd_hlp->cpu_queue_id, 1);
	    if (rc != 0) {
	        DHD_ERROR(("dor%d rdpa_cpu_rxq_flush_set(0x%p, %d, 1) returned %d\r\n",
	            dhd_hlp->dhd->unit, dhd_hlp->cpu_obj, dhd_hlp->cpu_queue_id, rc));
	    }
	}
	else
	{
#ifdef XRDP
	    cpu_port = rdpa_cpu_wlan0 + dhd_hlp->dhd->unit;
#else
	    cpu_port = rdpa_cpu_host;
#endif
	    rc = rdpa_cpu_get(cpu_port, &dhd_hlp->cpu_obj);
	    if (rc) {
	        DHD_ERROR(("dor%d rdpa_cpu_get(%d, 0x%p) returned %d\r\n",
	            dhd_hlp->dhd->unit, cpu_port, &dhd_hlp->cpu_obj, rc));
	        return rc;
	    }

	    dhd_hlp->cpu_port = cpu_port;
	    dhd_hlp->cpu_queue_id = cpu_queue_id_get(dhd_hlp->cpu_obj, dhd_hlp->dhd->unit);
	    if (dhd_hlp->cpu_queue_id < 0)
	        goto exit;
	    dhd_hlp->trap_reason = cpu_reasons[dhd_hlp->dhd->unit];

	    tasklet_init(&dhd_hlp->dhd_rx_tasklet, dhd_runner_rx_tasklet_handler,
	        (unsigned long)dhd_hlp);
	}

	rc = rdpa_cpu_rxq_cfg_get(dhd_hlp->cpu_obj, dhd_hlp->cpu_queue_id, &rxq_cfg);
	if (rc != 0) {
	    DHD_ERROR(("dor%d rdpa_cpu_rxq_cfg_get(0x%p, %d, 0x%p) returned %d\r\n",
	        dhd_hlp->dhd->unit, dhd_hlp->cpu_obj, dhd_hlp->cpu_queue_id, &rxq_cfg, rc));
	}

	rxq_cfg.size = cpu_queue_size;
	rxq_cfg.isr_priv = (long)dhd_hlp;
	rxq_cfg.rx_isr = cpu_queue_size ? dhd_runner_cpu_queue_isr : NULL;
	if (!cpu_queue_size)
	    rxq_cfg.ring_head = NULL; /* Destroy the ring */

	rc = rdpa_cpu_rxq_cfg_set(dhd_hlp->cpu_obj, dhd_hlp->cpu_queue_id, &rxq_cfg);
	if (rc < 0) {
	    DHD_ERROR(("dor%d rdpa_cpu_rxq_cfg_set(0x%p, %d, 0x%p) returned %d\r\n",
	        dhd_hlp->dhd->unit, dhd_hlp->cpu_obj, dhd_hlp->cpu_queue_id, &rxq_cfg, rc));
	    goto exit;
	}

	if (cpu_queue_size) {
	    rc = rdpa_dhd_helper_dhd_complete_ring_create(dhd_hlp->dhd->unit,
	        D2HRING_TXCMPLT_MAX_ITEM);
	    DHD_ERROR(("rdpa_dhd_helper_dhd_complete_ring_create(%d, %d) returned %d\r\n",
	        dhd_hlp->dhd->unit, D2HRING_TXCMPLT_MAX_ITEM, rc));
	} else {
	    rc = rdpa_dhd_helper_dhd_complete_ring_destroy(dhd_hlp->dhd->unit,
	        D2HRING_TXCMPLT_MAX_ITEM);
	    DHD_ERROR(("rdpa_dhd_helper_dhd_complete_ring_destroy(%d, %d) returned %d\r\n",
	        dhd_hlp->dhd->unit, D2HRING_TXCMPLT_MAX_ITEM, rc));
	}
	if (rc < 0)
	    goto exit;

	if (!cpu_queue_size)
	{
#ifdef XRDP
	    rdpa_dhd_cpu_data_t cpu_data;

	    cpu_data.cpu_port = rdpa_cpu_none;
	    cpu_data.exception_rxq = 0xff;
	    rc = rdpa_dhd_helper_cpu_data_set(dhd_hlp->dhd_helper_obj, NULL);
	    if (rc != 0) {
	        DHD_ERROR(("dor%d rdpa_dhd_helper_cpu_data_set(0x%p, NULL) returned %d\r\n",
	            dhd_hlp->dhd->unit, dhd_hlp->cpu_obj, rc));
	    }
#endif
	    goto exit;
	}

#ifdef XRDP
	{
	    bdmf_object_handle system_obj = NULL;
	    rdpa_dhd_cpu_data_t cpu_data;

	    cpu_data.cpu_port = cpu_port;
	    cpu_data.exception_rxq = dhd_hlp->cpu_queue_id;

	    /* Since same vport will be used for both WDF and DHD offload interface,
	     * for exception we need to use different reasons to map them to different
	     * cpu queues. For that purpose, we will use pci_ip_flow_miss reason.
	     */
	    rc = rdpa_system_get(&system_obj);
	    if (rc != 0) {
	        DHD_ERROR(("dor%d rdpa_system_get(0x%p) returned %d\r\n",
	            dhd_hlp->dhd->unit, &system_obj, rc));
	    }

	    rc = rdpa_system_cpu_reason_to_tc_set(system_obj,
	        rdpa_cpu_rx_reason_pci_ip_flow_miss_1 + dhd_hlp->dhd->unit,
	        dhd_hlp->cpu_queue_id);
	    bdmf_put(system_obj);
	    if (rc != 0) {
	        DHD_ERROR(("dor%d rdpa_system_cpu_reason_to_tc_set(0x%p, %d, %d) returned %d\r\n",
	            dhd_hlp->dhd->unit, &system_obj,
	            rdpa_cpu_rx_reason_pci_ip_flow_miss_1 + dhd_hlp->dhd->unit,
	        dhd_hlp->cpu_queue_id, rc));
	        return rc;
	    }

	    rc = rdpa_cpu_tc_to_rxq_set(dhd_hlp->cpu_obj, dhd_hlp->cpu_queue_id,
	        dhd_hlp->cpu_queue_id);
	    if (rc != 0) {
	        DHD_ERROR(("dor%d rdpa_cpu_tc_to_rxq_set(0x%p, %d, %d) returned %d\r\n",
	            dhd_hlp->dhd->unit, dhd_hlp->cpu_obj,
	            dhd_hlp->cpu_queue_id, dhd_hlp->cpu_queue_id, rc));
	    }

	    rc = rc ? rc : rdpa_dhd_helper_cpu_data_set(dhd_hlp->dhd_helper_obj, &cpu_data);
	    if (rc) {
	        DHD_ERROR(("dor%d rdpa_dhd_helper_cpu_data_set(0x%p, 0x%p) returned %d\r\n",
	            dhd_hlp->dhd->unit, dhd_hlp->dhd_helper_obj,  &cpu_data, rc));
	        return rc;
	    }
	}
#else /* RDP */
	for (i = 0; i < 2; i++) {
	    rdpa_cpu_reason_cfg_t reason_cfg = {};
	    rdpa_cpu_reason_index_t cpu_reason = {};

	    cpu_reason.reason = dhd_hlp->trap_reason;
	    cpu_reason.dir = i ? rdpa_dir_us : rdpa_dir_ds;
	    reason_cfg.queue = dhd_hlp->cpu_queue_id;
	    reason_cfg.meter = BDMF_INDEX_UNASSIGNED;
	    reason_cfg.meter_ports = 0;
	    rc = rdpa_cpu_reason_cfg_set(dhd_hlp->cpu_obj, &cpu_reason, &reason_cfg);
	    if (rc < 0) {
	        DHD_ERROR(("dor%d rdpa_cpu_reason_cfg_set(0x%p, 0x%p, 0x%p) returned %d\r\n",
	            dhd_hlp->dhd->unit, dhd_hlp->cpu_obj,  &cpu_reason, &reason_cfg, rc));
	        goto exit;
	    }
	}
#endif /* RDP */

	rdpa_cpu_int_enable(dhd_hlp->cpu_port, dhd_hlp->cpu_queue_id);

exit:
	return rc;
}

/*
 * +----------------------------------------------------------------------------
 *       Section: Layer 2 forwarding (Port to bridge)
 *
 * Initialize the Runner with the base address of various DHD DMA buffers.
 * In the case of DMA RD/WR Indices arrays, allocate/free in noncacheable memory
 * +----------------------------------------------------------------------------
 */
static int
dhd_runner_dma_buf_init(dhd_runner_hlp_t *dhd_hlp, void *osh,
	dhd_runner_dma_buf_t buf_type, dhd_dma_buf_t *dma_buf)
{
	rdpa_dhd_init_cfg_t *dhd_init_cfg = &dhd_hlp->dhd_init_cfg;

	ASSERT(dma_buf != NULL);

	/*
	 * Allocate and attach a DMA buffer for RD and WR indices in uncached memory
	 * Use ARM/MIPS platform specific uncached memory allocation API
	 */
	if (buf_type & ATTACH_DMA_INDX_BUF) {

	    /* Round up to cache line size */
	    dma_buf->len = ALIGN_SIZE(dma_buf->len, L1_CACHE_BYTES);

	    dma_buf->va = dhd_runner_alloc_mem(dhd_hlp, dma_buf, DHD_RNR_MEM_TYPE_RNR);

	    if (!dma_buf->va) {
	        DHD_ERROR(("%s: ATTACH_DMA_INDX_BUF<0x%08x> alloc failure\n",
	            __FUNCTION__, buf_type));
	        return BCME_NOMEM;
	    }

	    DHD_INFO(("%s: ATTACH_DMA_INDX_BUF<0x%08x> dma_buf<0x%p,0x%lx,%d>\n",
	        __FUNCTION__, buf_type, dma_buf->va, PHYSADDRLO(dma_buf->pa),
	        dma_buf->len));

	    switch (buf_type) {
	        case ATTACH_R2D_WR_BUF:
	            dhd_init_cfg->r2d_wr_arr_base_addr = dma_buf->va;
	            RDPA_INIT_CFG_SET_PA(dhd_init_cfg->r2d_wr_arr_base_phys_addr,
	                (uint32_t)dma_buf->pa);
	            break;
	        case ATTACH_R2D_RD_BUF:
	            dhd_init_cfg->r2d_rd_arr_base_addr = dma_buf->va;
	            RDPA_INIT_CFG_SET_PA(dhd_init_cfg->r2d_rd_arr_base_phys_addr,
	                (uint32_t) dma_buf->pa);
	            break;
	        case ATTACH_D2R_WR_BUF:
	            dhd_init_cfg->d2r_wr_arr_base_addr = (void*) ((unsigned long)dma_buf->va + 2);
	            RDPA_INIT_CFG_SET_PA(dhd_init_cfg->d2r_wr_arr_base_phys_addr,
	                (uint32_t) ((unsigned long)dma_buf->pa + 2));
	            break;
	        case ATTACH_D2R_RD_BUF:
	            dhd_init_cfg->d2r_rd_arr_base_addr = (void*) ((unsigned long)dma_buf->va + 2);
	            RDPA_INIT_CFG_SET_PA(dhd_init_cfg->d2r_rd_arr_base_phys_addr,
	                (uint32_t) ((unsigned long)dma_buf->pa + 2));
	            break;
	        default:
	            DHD_ERROR(("%s: invalid DMA_INDX_BUF type<%d>\n",
	                __FUNCTION__, buf_type));
	            ASSERT(0);
	            return BCME_BADOPTION;
	    }

	    DHD_INFO(("%s: ATTACH_DMA_INDX_BUF<0x%08x> dma_buf<0x%p,0x%lx,%d>\n",
	        __FUNCTION__, buf_type, dma_buf->va, PHYSADDRLO(dma_buf->pa),
	        dma_buf->len));


	} else if (buf_type & DETACH_DMA_INDX_BUF) {

	    DHD_INFO(("%s: DETACH_DMA_INDX_BUF<0x%08x> dma_buf<0x%p,0x%lx,%d>\n",
	        __FUNCTION__, buf_type, dma_buf->va, PHYSADDRLO(dma_buf->pa),
	        dma_buf->len));

	    dhd_runner_free_mem(dhd_hlp, dma_buf, DHD_RNR_MEM_TYPE_RNR);

	} else if (buf_type & ATTACH_RING_BUF) {
	    uint32_t ringid = buf_type & 0xFFFF;

	    if (ringid >= BCMPCIE_COMMON_MSGRINGS) { /* DHD_IS_FLOWRING */
	        uint16 flow_id;
	        rdpa_dhd_flring_cache_t *rdpa_cache;
	        dhd_runner_flowmgr_t *flowmgr;
	        dhd_rdpa_flring_cache_t *dhd_cache;

	        /* DHD_RINGID_TO_FLOWID */
	        flow_id = (ringid - BCMPCIE_COMMON_MSGRINGS) + BCMPCIE_H2D_COMMON_MSGRINGS;
	        flowmgr = &dhd_hlp->flowmgr;
	        rdpa_cache = flowmgr->rdpa_flring_cache + flow_id;
	        dhd_cache = flowmgr->dhd_flring_cache + flow_id;

	        dma_buf->va = dhd_cache->base_va;
	        PHYSADDRLOSET(dma_buf->pa, ntohl(rdpa_cache->base_addr_low));
	        PHYSADDRHISET(dma_buf->pa, ntohl(rdpa_cache->base_addr_high));

	        dma_buf->len = ntohs(DHD_RNR_PHY_RING_SIZE(rdpa_cache));
	        dma_buf->len *= dhd_hlp->txpost_ring_cfg.size;
	        dma_buf->dmah = (void*)NULL;
	        dma_buf->secdma = (void*)NULL;

	        return BCME_OK;
	    }

	    /* Allocate DMA-able buffers for the ring, if not done so already */
	    if (dma_buf->va == NULL) {

	        dma_buf->va = dhd_runner_alloc_mem(dhd_hlp, dma_buf,
	            dhd_runner_get_ring_mem_alloc_type(dhd_hlp, ringid));

	        if (!dma_buf->va) {
	            DHD_ERROR(("%s: ATTACH_RING_BUF ringid<%d> alloc failure\n",
	                __FUNCTION__, ringid));
	            return BCME_NOMEM;
	        }
	    }

	    DHD_INFO(("%s: ATTACH_RING_BUF<0x%08x> dma_buf<0x%p,0x%lx,%d>\n",
	        __FUNCTION__, buf_type, dma_buf->va, PHYSADDRLO(dma_buf->pa),
	        dma_buf->len));

	    /* Setup the allocation variables  */
	    switch (ringid) {
	        case BCMPCIE_H2D_MSGRING_RXPOST_SUBMIT:
	            dhd_init_cfg->rx_post_flow_ring_base_addr = dma_buf->va;
	            break;
	        case BCMPCIE_D2H_MSGRING_TX_COMPLETE:
	            if (DHD_RNR_TXSTS_OFFLOAD(dhd_hlp))
	                dhd_init_cfg->tx_complete_flow_ring_base_addr = dma_buf->va;
	            break;
	        case BCMPCIE_D2H_MSGRING_RX_COMPLETE:
	            dhd_init_cfg->rx_complete_flow_ring_base_addr = dma_buf->va;
	            break;

	        case BCMPCIE_H2D_MSGRING_CONTROL_SUBMIT:
	        case BCMPCIE_D2H_MSGRING_CONTROL_COMPLETE:
	            /* Runner does not need to registers control subn/cmplt rings */
	            return BCME_OK;

	        default:
	            DHD_ERROR(("%s: ringid %d not supported in Runner\n",
	                __FUNCTION__, buf_type));
	            ASSERT(0);
	            return BCME_BADOPTION;
	    }
	} else if (buf_type & DETACH_RING_BUF) {
	    uint32_t ringid = buf_type & 0xFFFF;

	    DHD_INFO(("%s: DETACH_RING_BUF<0x%08x> dma_buf<0x%p,0x%lx,%d>\n",
	        __FUNCTION__, buf_type, dma_buf->va, PHYSADDRLO(dma_buf->pa),
	        dma_buf->len));

	    if (ringid < BCMPCIE_COMMON_MSGRINGS) { /* DHD_IS_NOT_FLOWRING */

	        dhd_runner_free_mem(dhd_hlp, dma_buf,
	            dhd_runner_get_ring_mem_alloc_type(dhd_hlp, ringid));

	    } else {
	    /* Nothing needs to be done here as all the flowring buffers are freed
	     * as part of dhd_runner_flowmgr_fini() call later
	     */
	        RPR("DETACH_RING_BUF No Action");
	    }
	}

	return BCME_OK;
}

/* Update flag in flring rdpa_cache entry for specified flring */
static int
dhd_runner_flring_cache_flag_update(rdpa_dhd_flring_cache_t *flring,
	uint16 flag, int set)
{
	if (set)
	    flring->flags |= htons(flag);
	else
	    flring->flags &= htons(~flag);

	ARCH_FLUSH_COHERENT((void *)flring, sizeof(rdpa_dhd_flring_cache_t));

	return BCME_OK;
}


/**
 * Setup the PCIE RC match and Ubus remap registers
 *
 * Mechanism using translation in dongle sbtopcie transl 0 and PCIE1 remap:
 * ========================================================================
 * In 436XX, the PCIE core defines 3 system backplane address spaces:
 * enumeration, a 128-MB PCIE access space and a full 64-bit addressable
 * PCIE space. Using the SBTOPCIE translation 0 and 1, the smaller 128-MB of
 * of backplane address space may be mapped onto two 64-MB each of external
 * PCIE address space. Here, a 64-MB is managed using sbtopcie0. A single
 * 64-MB region suffices to cover the Runner's register space. If two
 * separate regions are needed, then the BAR3 and sbtopcie transl1 may
 * be used.
 *
 * Dongle side:
 * Any address in the 64-MB (sbtopcie region) 0x08000000..0x0BFFFFFF will
 * be directed to the PCIE core, using the sbtopcie translation 0.
 * The base 0x08000000 will be subtracted when delivered to the host side
 * system bus.
 *
 * So if we have dongle accessing address 0x0bXXXXXX, it will be directed
 * to the PCIE core as it falls in the range 0x08000000..0x0BFFFFFF, and the
 * base 0x08000000 will be subtracted, to get 0x03XXXXXX. Now whatever we
 * program into sbtopcie0 will be added to it. So, if the remap sbtopcie0
 * had 0x10000000, we would get 0x13XXXXXX into the PCIE root complex.
 *
 * Now we simply use the PCIE RC BAR2 match address to trap this address.
 * To trap we use a match address of 0x10000000 with a range of 64-MB. If
 * we needed dongle to access two 64-MB regions, we could use each of BAR2
 * and BAR3 match address regions configured as 64-MB each.
 *
 * The match address would get subtracted, so we are back to 0x03XXXXXX. The
 * trapped address will be directed to the UBUS. Using the UBUS BAR2 remap
 * we place back the 0x10000000, to get back to 0x13XXXXXX.
 *
 * Here is a walk through of dongle accessing Runner register 0x13099004:
 * - dongle performs access using address 0x0B099004.
 * - as 0x0B099004 is in the range 0x08000000..0x0BFFFFFF, it will be
 *   directed to the PCIE core and the sbtopcie trans0, will be added, and
 *   base 0x08000000 will be subtracted.
 *   0x0B099004 -> 0x0B099004 + sbtopcie0[0x10000000] - 0x08000000
 *              -> 0x13099004 (delivered to PCIE RC).
 * - PCIE RC BAR2 match address 0x10000000 will trap and redirect to UBUS
 *   after subtracting the match address:
 *   0x13099004 -> 0x13099004 - PCIE_RC_BAR2_MATCH[0x10000000]
 *              -> 0x03099004 to be delivered to UBUS with BAR2 remapping
 * - UBUS BAR2 remap =[0x10000000], will be added before sent out on UBUS
 *   0x03099004 -> 0x03099004 + 0x10000000
 *              -> 0x13099004 and voila original runner register phys-address
 *                 after two match translations ...
 *
 * NOTE: We could have 2 regions in host memory, each of 64M region that may
 * be addressed using sbtopcie transl 0 and transl 1, as described above.
 *
 * PcieMiscRegs::rc_bar2_config_lo      +0x4034 (64-MB = 0xB)
 * PcieMiscRegs::rc_bar2_config_hi      +0x4038
 * PcieMiscRegs::ubus_bar2_config_remap +0x40B4 (AccessEn = 0x1)
 *
 */

static int
dhd_runner_pcie_init(dhd_runner_hlp_t *dhd_hlp, struct pci_dev *pci_dev)
{
	struct pci_bus *root_bus = pci_dev->bus;
	uint32 map_addr;
	uint32 rev;

	dhd_hlp->pci_dev = pci_dev;

	RLOG(CLRyk "Runner DHD PCIE: vendor<0x%04x> device<0x%04x> bus<%d> slot<%d>" CLRnl,
	    pci_dev->vendor, pci_dev->device,
	    pci_dev->bus->number, PCI_SLOT(pci_dev->devfn));

	/* Get the handle to the Root Bus of the PCIe device */
	while (NULL != root_bus->parent) {
	    root_bus = root_bus->parent;
	}

	/* Get PCIe MISC_REVISION information */
	if (pci_bus_read_config_dword(root_bus, 0, 0x406C, &rev)) {
	    DHD_ERROR(("%s: Failed to get pci revision informatonpci_dev\n",
	        __FUNCTION__));
	    return BCME_ERROR;
	}

#if defined(__mips__)
	map_addr = 0x10000000;
#else /* !__mips__ */
	map_addr = 0x80000000;
#endif /* __mips__ */
	/*
	 * Configure MISC_RC_BAR2_CONFIG register with
	 * 64MB map size (0x0b) from map_addr
	 */
	PCIE_WR_CONFIG(root_bus, 0x4034, (map_addr|0x0B));
	PCIE_WR_CONFIG(root_bus, 0x4038, 0x00000000);

	/*
	 * Configure MISC_UBUS_BAR2_CONFIG_REMAP register with
	 * Enable USB2 access to BAR2 address space and offset(map_addr)
	 */
	if (rev < 0x0300) {
	    PCIE_WR_CONFIG(root_bus, 0x408C, (map_addr|0x01));
	} else {
	    PCIE_WR_CONFIG(root_bus, 0x40B4, (map_addr|0x01));
	    PCIE_WR_CONFIG(root_bus, 0x40B8, 0x00000000);
	}

	return BCME_OK;
}

/**
 * initialize dongle wakeup register information based on iDMA
 *
 */
static int
dhd_runner_dongle_wakeup_init(dhd_runner_hlp_t *dhd_hlp)
{
	dhd_bus_t *bus = dhd_hlp->dhd->bus;
	rdpa_dhd_init_cfg_t *init_cfg = &dhd_hlp->dhd_init_cfg;
	phys_addr_t bar0_phys = pci_resource_start(dhd_hlp->pci_dev, 0);
	char *regs_base = (char*)bus->regs;
	char *db_reg1;
	char *db_reg2;


	/*
	 * Set the dongle wake up register
	 *
	 * DHD maps PCIe bar#0 to dongle registers
	 *
	 * dongle addr = mail box VA - registers base VA + register base PA
	 */

#if defined(BCMHWA)
	if (IDMA_ENAB(dhd_hlp->dhd)) {
	    if (dhd_hlp->rnr_sup_feat.hwawkup == 0) {
	        DHD_ERROR(("dhd%d_rnr: Runner does not support iDMA\r\n", dhd_hlp->dhd->unit));
	        return BCME_UNSUPPORTED;
	    }
	    /* Use PCIH2D_MailBox_2 (0x160) to wakeup dongle ARM (TXPOST) */
	    db_reg1 = (char*)bus->pcie_mb_intr_2_addr;

	    /* Use PCIH2D_DB1_2 (0x164) to wakeup dongle HWA (RXPOST, RXCPL, TXCPL) */
	    db_reg2 = (char*)bus->pcie_db1_intr_2_addr;

	    dhd_hlp->rnr_en_feat.hwawkup = 1;
	} else
#endif /* BCMHWA */
	{
	    /* Use PCIMailBoxInt (0x140) for all rings to wakeup dongle SW */
	    db_reg1 = (char*)bus->pcie_mb_intr_addr;
	    db_reg2 = db_reg1;
	}

	/* Fill the wakeup information in init_cfg structure */
	/* Wakeup through DHD */
	init_cfg->doorbell_isr = dhd_runner_wake_dongle_isr;
	init_cfg->doorbell_ctx = dhd_hlp;

	/* Direct wakeup by runner using door bell registers */
	init_cfg->dongle_wakeup_register = (uint32)(bar0_phys + (uint32)(db_reg1 - regs_base));
	RPR("dongle_wakeup_addr = 0x%08x\r\n",
	    dhd_hlp->dhd_init_cfg.dongle_wakeup_register);
#if defined(RNR_DHD_HLPR_HWA_WAKEUP)
	init_cfg->dongle_wakeup_register_2 = (uint32)(bar0_phys + (uint32)(db_reg2 - regs_base));
	init_cfg->dongle_wakeup_register_virt = (void*)db_reg1;
	init_cfg->dongle_wakeup_register_2_virt = (void*)db_reg2;
#if defined(BCMHWA)
	init_cfg->dongle_wakeup_hwa = IDMA_ENAB(dhd_hlp->dhd);
#else /* !BCMHWA */
	init_cfg->dongle_wakeup_hwa = 0;
#endif /* !BCMHWA */
	RPR("dongle_wakeup_addr_2 = 0x%08x\r\n",
	    dhd_hlp->dhd_init_cfg.dongle_wakeup_register_2);
#endif /* RNR_DHD_HLPR_HWA_WAKEUP */

	return BCME_OK;
}


/**
 * wakeup dongle based on the doorbell register and value
 *
 */
static int
dhd_runner_wakeup_dongle(dhd_runner_hlp_t *dhd_hlp, uint32 db_reg, uint32 db_val)
{
	dhd_pub_t	*dhdp = dhd_hlp->dhd;

#if defined(BCMHWA)
	if (IDMA_ENAB(dhdp)) {
	    if (db_reg == dhd_hlp->dhd_init_cfg.dongle_wakeup_register) {
	        /* Use PCIH2D_MailBox_2 to wakeup dongle ARM */
	        dhdpcie_bus_ringbell_2_fast(dhdp->bus, db_val, TRUE);
	    } else if (db_reg == dhd_hlp->dhd_init_cfg.dongle_wakeup_register_2) {
	        /* Use PCIH2D_DB1_2 to wakeup dongle HWA */
	        dhdpcie_bus_db1_ringbell_2_fast(dhdp->bus, db_val, TRUE);
	    } else
	        return BCME_ERROR;
	} else
#endif /* BCMHWA */
	{
	    /* Use PCIMailBoxInt to wake up dongle */
	    dhdpcie_bus_ringbell_fast(dhdp->bus, 0xdeadbeef);
	}

	return BCME_OK;
}

/**
 * enable adding llcsnap header feature in the runner based on dhd and dongle support.
 */
static int
dhd_runner_llcsnaphdr_init(dhd_runner_hlp_t *dhd_hlp)
{
#if defined(BCM_DHDHDR)
	uint8 llcsnaphdr = DHDHDR_SUPPORT(dhd_hlp->dhd);

	/* Check if runner has support for llcsnap header */
	if (llcsnaphdr && dhd_hlp->rnr_sup_feat.dhdhdr == 0) {
	    DHD_ERROR(("%s: Add LLCSNAP_HDR is not supported by runner\n",
	        __FUNCTION__));
	    return BCME_UNSUPPORTED;
	}

#if defined(RNR_DHD_HLPR_LLCSNAPHDR)
	dhd_hlp->dhd_init_cfg.add_llcsnap_header = llcsnaphdr;
	dhd_hlp->rnr_en_feat.dhdhdr = llcsnaphdr;
#endif /* RNR_DHD_HLPR_LLCSNAPHDR */
#endif /* BCM_DHDHDR */

	return BCME_OK;
}

/**
 * enable txcmpl2host feature in the runner.
 */
static int
dhd_runner_txcmpl2host_init(dhd_runner_hlp_t *dhd_hlp, bdmf_object_handle mo)
{
	int rc = BCME_OK;

	if (DHD_RNR_TXSTS_OFFLOAD(dhd_hlp) &&
	    !(DHD_RNR_NONACCPKT_TXSTS_OFFLOAD(dhd_hlp))) {

	    /* fail if Runner does not support this feature */
	    if (dhd_hlp->rnr_sup_feat.txcmpl2host == 0) {
	        return BCME_UNSUPPORTED;
	    }

#if defined(RNR_DHD_HLPR_TXCMPL2HOST)
	    rc = rdpa_dhd_helper_tx_complete_send2host_set(mo, TRUE);

	    if (rc < 0) {
	        DHD_ERROR(("dhd%d_rnr: set tx_complete_send2host failed %d\r\n",
	            dhd_hlp->dhd->unit, rc));
	        return BCME_ERROR;
	    }
	    dhd_hlp->rnr_en_feat.txcmpl2host = 1;
	    RPR2("rdpa_dhd_helper_tx_complete_send2host_set=TRUE\r\n");
#endif /* RNR_DHD_HLPR_TXCMPL2HOST */
	}

	return rc;
}

/**
 * Initialize dhd_runner sw features
 *
 * check nvram, if not present, return use default value
 *
 *
*/
static int
dhd_runner_swfeatures_init(struct dhd_runner_hlp *dhd_hlp)
{
	char buff[DHD_RNR_KEY_SWFEATURE_STR_LEN];
	int  length = 0;
	uint32 features;

	/* Set the default values */
	dhd_hlp->proc_cpuq_in_dpc = DHD_RNR_PROC_CPUQ_IN_DPC;
	dhd_hlp->ipend = FALSE;

	/* Fetch the SW feature Configuration information of the radio, if present */
	memset(buff, 0, sizeof(buff));
	length = dhd_runner_key_get(dhd_hlp->dhd->unit, DHD_RNR_KEY_SWFEATURES, buff,
	    sizeof(buff));

	if (length == 0) {
	    /* sw features setting is not present, return default */
	    goto done;
	}

	sscanf(buff, "%d", &features);

	if (features & DHD_RNR_SWFEATURE_CPUQINDPC)
	    dhd_hlp->proc_cpuq_in_dpc = TRUE;
	else
	    dhd_hlp->proc_cpuq_in_dpc = FALSE;

done:
	return 0;
}

/**
 * DHD requests Runner to complete the configuration of the dhd_hlp object
 * once all dhd DMA-able buffers for common rings, flowrings and H2D/D2H indices
 * is completed.
 */
static int
dhd_runner_init(dhd_runner_hlp_t *dhd_hlp, struct pci_dev *pci_dev)
{
	int rc = BCME_OK;
	dhd_pub_t *dhdp = dhd_hlp->dhd;
	BDMF_MATTR(dhd_helper_attr, rdpa_dhd_helper_drv());
	BDMF_MATTR(wlan_mcast_attrs, rdpa_wlan_mcast_drv());

	/* Initialization is not needed, if offload is not supported for this radio */
	if (!DHD_RNR_IS_OFFL_SUPPORTED(dhd_hlp)) {
	    DHD_INFO(("dhd%d_rnr offload not supported, skip initialization\r\n",
	        dhd_hlp->dhd->unit));
	    return BCME_OK;
	}

	if (!pci_dev) {
	    DHD_ERROR(("%s: unknown pci_dev\n", __FUNCTION__));
	    return BCME_NODEVICE;
	}

	if ((rc = dhd_runner_pcie_init(dhd_hlp, pci_dev)) != BCME_OK)
	    return rc;

	if ((rc = dhd_runner_llcsnaphdr_init(dhd_hlp)) != BCME_OK)
	    return rc;

	if ((rc = dhd_runner_txcmpl2host_init(dhd_hlp, dhd_helper_attr)) == BCME_ERROR)
	    return rc;

	if ((rc = dhd_runner_dongle_wakeup_init(dhd_hlp)) != BCME_OK)
	    return rc;

	rc = rdpa_dhd_helper_radio_idx_set(dhd_helper_attr, dhdp->unit);
	if (rc != 0) {
	    DHD_ERROR(("dor%d rdpa_dhd_helper_radio_idx_set(0x%p, %d) returned %d\r\n",
	        dhdp->unit, &dhd_helper_attr, dhdp->unit, rc));
	}
	rc = rdpa_dhd_helper_init_cfg_set(dhd_helper_attr, &dhd_hlp->dhd_init_cfg);
	if (rc != 0) {
	    DHD_ERROR(("dor%d rdpa_dhd_helper_init_cfg_set(0x%p, 0x%p) returned %d\r\n",
	        dhdp->unit, &dhd_helper_attr, &dhd_hlp->dhd_init_cfg, rc));
	}

	DHD_RNR_INIT_PERIM_UNLOCK(dhdp);

	rc = bdmf_new_and_set(rdpa_dhd_helper_drv(), NULL,
	                        dhd_helper_attr, &dhd_hlp->dhd_helper_obj);

	if (rc) {
	    DHD_ERROR(("%s: bdmf_new_and_set error %d\n", __FUNCTION__, rc));
	    DHD_RNR_INIT_PERIM_LOCK(dhdp);
	    return BCME_ERROR;
	}


	rc = rdpa_dhd_helper_int_connect_set(dhd_hlp->dhd_helper_obj, true);

	if (rc) {
	    DHD_ERROR(("%s: Failed to connect interrupts, error %d\n", __FUNCTION__, rc));
	    DHD_RNR_INIT_PERIM_LOCK(dhdp);
	    return BCME_ERROR;
	}

	dhd_hlp->dhd_mcast_obj = NULL;
	rc = rdpa_wlan_mcast_get(&(dhd_hlp->dhd_mcast_obj));
	if (rc && dhd_rnr_mcast_obj_ref_cnt < 0)
	{
	    /* Create one mcast object for all radios */
	    rc = bdmf_new_and_set(rdpa_wlan_mcast_drv(), NULL, wlan_mcast_attrs,
	        &dhd_hlp->dhd_mcast_obj);
	    if (rc)
	        DHD_ERROR(("%s: MCAST HANDLER FAILURE  %d\n", __FUNCTION__, rc));
	    else
	        dhd_rnr_mcast_obj_ref_cnt = 0;
	}

	if (!rc && (dhd_rnr_mcast_obj_ref_cnt >= 0)) {
	    dhd_rnr_mcast_obj_ref_cnt++;
	}

	if (!rc) {
	    rc = dhd_runner_cfg_cpu_queue(dhd_hlp, 1);
	    if (rc)
	        DHD_ERROR(("%s: dhd_runner_cfg_cpu_queue failed, rc = %d\n",
	            __FUNCTION__, rc));
	}

	DHD_RNR_INIT_PERIM_LOCK(dhdp);

#if (defined(CONFIG_BCM_SPDSVC) || defined(CONFIG_BCM_SPDSVC_MODULE))
	dhd_runner_spdsvc_transmit = bcmFun_get(BCM_FUN_ID_SPDSVC_TRANSMIT);
	BCM_ASSERT(dhd_runner_spdsvc_transmit != NULL);
#endif

	RLOG(CLRyk "Runner DHD Offload initialization complete" CLRnl);

	return rc;
}

/*
 * +----------------------------------------------------------------------------
 *      Section: Attach and Detach
 * +----------------------------------------------------------------------------
 */
static INLINE int
dhd_runner_wakeup_init(bcmpcie_soft_doorbell_t *soft_doorbell,
	phys_addr_t wakeup_paddr, uint32 wakeup_val32)
{
	if (soft_doorbell == NULL) {
	    /* If softdoorbells are not supported, runner can not get interrupts */
	    return BDMF_ERR_PARM;
	}

#if defined(PHYS_ADDR_64BIT) || defined(CONFIG_PHYS_ADDR_T_64BIT)
	soft_doorbell->haddr.low_addr = (uint32_t)(wakeup_paddr & 0xffffffff);
	soft_doorbell->haddr.high_addr = (uint32_t)(wakeup_paddr >> 32);
#else /* !PHYS_ADDR_64BIT && !CONFIG_PHYS_ADDR_T_64BIT */
	soft_doorbell->haddr.low_addr = (uint32_t)wakeup_paddr;
	soft_doorbell->haddr.high_addr = 0U; /* 32bit host */
#endif /* !PHYS_ADDR_64BIT && !CONFIG_PHYS_ADDR_T_64BIT */
	soft_doorbell->value = htol32(wakeup_val32);

	if (soft_doorbell->haddr.high_addr != 0) {
	    /* Dongle does not support address > 32bit */
	    return BDMF_ERR_PARM;
	}

	return BDMF_ERR_OK;
}

static int
dhd_helper_attach(dhd_runner_hlp_t *dhd_hlp, void *dhd)
{
	char buff[DHD_RNR_KEY_OFFL_OVERRIDE_STR_LEN];
	int  length = 0;
	uint32 en_mask = 0;
	uint32 dhd_mask;

	DHD_INFO(("%s: hlp<%p> dhd<%p>\n", __FUNCTION__, dhd_hlp, dhd));

	dhd_hlp->dhd = dhd;
	dhd_mask = 1UL << dhd_hlp->dhd->unit;

	/* Fetch the offlaod override Configuration information of the radio, if present */
	memset(buff, 0, sizeof(buff));
	length = dhd_runner_key_get(DHD_RNR_RADIO_IDX_ALL, DHD_RNR_KEY_OFFL_OVERRIDE, buff,
	    sizeof(buff));

	if (length == 0) {
	    /* Offload override setting is not present, Check max radios */
	    if (dhd_hlp->dhd->unit < RDPA_MAX_RADIOS) {
	        en_mask = dhd_mask;
	    }
	} else {
	    /* Offload override setting is present, Check the setting */
	    int unit = 0;
	    uint32 radio_mask, bit_mask;

	    sscanf(buff, "%d", &radio_mask);
	    bit_mask = 1UL;
	    while ((unit < RDPA_MAX_RADIOS) && (bit_mask <= dhd_mask)) {
	        if (radio_mask & bit_mask) unit++;
	        if ((radio_mask & bit_mask) == dhd_mask) {
	            en_mask = dhd_mask;
	            break;
	        }
	        bit_mask <<= 1;
	    }
	}

	if (!en_mask) {
	    /*
	     * runner support is not enabled for this radio
	     * dhd_rnr instance still exists, to support api's
	     * but runner offload is disabled completely.
	     */
	    DHD_ERROR(("dhd%d_rnr offload not supported by runner\r\n",
	        dhd_hlp->dhd->unit));
	    memset(&dhd_hlp->rnr_sup_feat, 0, sizeof(dhd_helper_feat_t));
	    memset(&dhd_hlp->rnr_en_feat, 0, sizeof(dhd_helper_feat_t));
	    return 0;
	}

	/* Base features are enabled by default */
	dhd_hlp->rnr_sup_feat.txoffl = 1;
	dhd_hlp->rnr_sup_feat.rxoffl = 1;

	/* Extra features are enabled conditionally */
#if defined(RNR_DHD_HLPR_TXCMPL2HOST)
	dhd_hlp->rnr_sup_feat.txcmpl2host = 1;
#endif /* RNR_DHD_HLPR_TXCMPL2HOST */

#if defined(RNR_DHD_HLPR_LLCSNAPHDR)
	dhd_hlp->rnr_sup_feat.dhdhdr = 1;
#endif /* RNR_DHD_HLPR_LLCSNAPHDR */

#if defined(RNR_DHD_HLPR_LBRAGGR)
	dhd_hlp->rnr_sup_feat.lbraggr = 1;
#endif /* RNR_DHD_HLPR_LBRAGGR */

	dhd_hlp->rnr_sup_feat.npm = DOR_NPM_DEFAULT;
#if defined(RNR_DHD_HLPR_NPM)
	dhd_hlp->rnr_sup_feat.npm = 1;
#endif /* RNR_DHD_HLPR_NPM */

	/* Enable legacy type for all rings MSGBUF_WI_WI64 */
	dhd_hlp->txpost_ring_cfg.sup_types = 0x1;
	dhd_hlp->rxpost_ring_cfg.sup_types = 0x1;
	dhd_hlp->txsts_ring_cfg.sup_types = 0x1;
	dhd_hlp->rxcmpl_ring_cfg.sup_types = 0x1;

#if defined(RNR_DHD_HLPR_MSGRINGFRMT)
	dhd_hlp->rnr_sup_feat.msgringformat = 1;

#if defined(MSGBUF_WI_COMPACT)
	/* For TxPost add CWI32/CWI64 based on the runner support flag */
#if defined(RDPA_DHD_HELPER_MIXED_CWI64_CWI32_MSGFORMAT_SUPPORT)
	dhd_hlp->txpost_ring_cfg.sup_types |= (1ul << MSGBUF_WI_CWI64);
#else /* !RDPA_DHD_HELPER_MIXED_CWI64_CWI32_MSGFORMAT_SUPPORT */
	dhd_hlp->txpost_ring_cfg.sup_types |= (1ul << MSGBUF_WI_CWI32);
#endif /* !RDPA_DHD_HELPER_MIXED_CWI64_CWI32_MSGFORMAT_SUPPORT */

	/* For rest, add CWI32 format support */
	dhd_hlp->rxpost_ring_cfg.sup_types |= (1ul << MSGBUF_WI_CWI32);
	dhd_hlp->rxcmpl_ring_cfg.sup_types |= (1ul << MSGBUF_WI_CWI32);
	dhd_hlp->txsts_ring_cfg.sup_types |= (1ul << MSGBUF_WI_CWI32);
#endif /* MSGBUF_WI_COMPACT */

#endif /* RNR_DHD_HLPR_MSGRINGFRMT */

	RLOG("dor%d: runner supported ring format types TxP 0x%x, RxP 0x%x TxC 0x%x RxC 0x%x\r\n",
	    dhd_hlp->dhd->unit,
	    dhd_hlp->txpost_ring_cfg.sup_types, dhd_hlp->rxpost_ring_cfg.sup_types,
	    dhd_hlp->txsts_ring_cfg.sup_types, dhd_hlp->rxcmpl_ring_cfg.sup_types);

#if defined(RNR_DHD_HLPR_BKUPQUEUE)
	dhd_hlp->rnr_sup_feat.bkupq = 1;
#endif /* RNR_DHD_HLPR_BKUPQUEUE */

#if defined(RNR_DHD_HLPR_HWA_WAKEUP)
	dhd_hlp->rnr_sup_feat.hwawkup = 1;
#endif /* RNR_DHD_HLPR_HWA_WAKEUP */

#if defined(RNR_DHD_HLPR_FFRD)
	dhd_hlp->rnr_sup_feat.ffrd = 1;
#endif /* RNR_DHD_HLPR_FFRD */

	return 0;
}

static void
dhd_helper_detach(dhd_runner_hlp_t *dhd_hlp)
{
	DHD_INFO(("%s: dhd_hlp<%p>\n", __FUNCTION__, dhd_hlp));

	dhd_runner_cfg_cpu_queue(dhd_hlp, 0); /* Unconfigure CPU Queue */

	if (dhd_hlp->dhd_helper_obj)
	    bdmf_destroy(dhd_hlp->dhd_helper_obj);

	if (dhd_hlp->dhd_mcast_obj) {
	    if (dhd_rnr_mcast_obj_ref_cnt > 0) {
	        if (dhd_rnr_mcast_obj_ref_cnt == 1) {
	            bdmf_destroy(dhd_hlp->dhd_mcast_obj);
	        }
	        dhd_rnr_mcast_obj_ref_cnt--;
	    }
	    dhd_hlp->dhd_mcast_obj = NULL;
	}
}

struct dhd_runner_hlp *
dhd_runner_attach(dhd_pub_t *dhd, bcmpcie_soft_doorbell_t *soft_doorbells)
{
	int rc;
	int old_trace_level = bdmf_global_trace_level;
	dhd_runner_hlp_t *dhd_hlp = NULL;
	bcmpcie_soft_doorbell_t *tx_soft_doorbell, *rx_soft_doorbell;
	rdpa_dhd_wakeup_info_t wakeup_info = {dhd->unit, 0, 0, 0, 0};

	DHD_INFO(("%s: dhd<%p>\n", __FUNCTION__, dhd));

	dhd_hlp = (dhd_runner_hlp_t *)MALLOCZ(dhd->osh, sizeof(dhd_runner_hlp_t));
	if (dhd_hlp == NULL) {
	    DHD_ERROR(("%s: failed DHD helper context malloc\n", __FUNCTION__));
	    return NULL;
	}

	rc = dhd_helper_attach(dhd_hlp, dhd);
	if (rc) {
	    DHD_ERROR(("%s: Failed DHD Helper context attach\n", __FUNCTION__));
	    goto exit;
	}

	dhd_hlp->coherent_mem_pool.len = DHD_RNR_COHERENT_MEM_POOL_SIZE;
	dhd_hlp->coherent_mem_pool.va = dhd_runner_alloc_mem(dhd_hlp,
	    &dhd_hlp->coherent_mem_pool, DHD_RNR_MEM_ALLOC_TYPE_COHERENT);

	if (dhd_hlp->coherent_mem_pool.va == NULL) {
	    DHD_ERROR(("%s: Failed to Allocate coherent memory \n", __FUNCTION__));
	    rc = BDMF_ERR_NOMEM;
	    goto exit;
	}
	/*
	 * Use len to store total allocated and alloced to get the total
	 * allocated in the local pool
	 */
	dhd_hlp->coherent_mem_pool._alloced = 0;
	DHD_INFO(("[COHERENT_POOL_%d] Allocated 0x%x size pool 0x%p, 0x%lx\r\n",
	    dhd_hlp->dhd->unit, dhd_hlp->coherent_mem_pool.len, dhd_hlp->coherent_mem_pool.va,
	    PHYSADDRLO(dhd_hlp->coherent_mem_pool.pa)));

	/*
	 * Lets tell the dongle where the tx complete and rx complete runner wake up
	 * registers are and the value to write so as to directly wake up the
	 * corresponding threads in Runner without DHD intervention.
	 */

	if (soft_doorbells) {
	    /* Control Complete d2h doorbell defaults to interrupt host i.e. DHD */
	    tx_soft_doorbell = soft_doorbells + BCMPCIE_D2H_MSGRING_TX_COMPLETE_IDX;
	    rx_soft_doorbell = soft_doorbells + BCMPCIE_D2H_MSGRING_RX_COMPLETE_IDX;
	} else {
	    tx_soft_doorbell = rx_soft_doorbell = NULL;
	}

	/*
	 * Initialize all ring configurations to legacy formats
	 * DHD should update the type and size based on dongle support in prot-pre-init
	 */
	dhd_hlp->txpost_ring_cfg.max_items = H2DRING_TXPOST_MAX_ITEM;
	dhd_hlp->txpost_ring_cfg.size = H2DRING_TXPOST_ITEMSIZE;
	dhd_hlp->txpost_ring_cfg.type = 0;    /* Legacy */

	dhd_hlp->rxpost_ring_cfg.max_items = H2DRING_RXPOST_MAX_ITEM;
	dhd_hlp->rxpost_ring_cfg.size = H2DRING_RXPOST_ITEMSIZE;
	dhd_hlp->rxpost_ring_cfg.type = 0;    /* Legacy */

	dhd_hlp->txsts_ring_cfg.max_items = D2HRING_TXCMPLT_MAX_ITEM;
	dhd_hlp->txsts_ring_cfg.size = D2HRING_TXCMPLT_ITEMSIZE;
	dhd_hlp->txsts_ring_cfg.type = 0;    /* Legacy */

	dhd_hlp->rxcmpl_ring_cfg.max_items = D2HRING_RXCMPLT_MAX_ITEM;
	dhd_hlp->rxcmpl_ring_cfg.size = D2HRING_RXCMPLT_ITEMSIZE;
	dhd_hlp->rxcmpl_ring_cfg.type = 0;    /* Legacy */

	/* Initialize tx ring offload setting */
	dhd_runner_txoffl_init(dhd_hlp, &dhd_hlp->txsts_ring_cfg);
	RLOG("%s: Tx Offload - %s, Ring Size = %d", __FUNCTION__,
	    (dhd_hlp->txsts_ring_cfg.offload) ? "Enabled" : "Disabled",
	    dhd_hlp->txsts_ring_cfg.max_items);

	/* Initialize rx ring offload setting */
	dhd_runner_rxoffl_init(dhd_hlp, &dhd_hlp->rxcmpl_ring_cfg);
	RLOG("%s: Rx Offload - %s, Ring Size = %d", __FUNCTION__,
	    (dhd_hlp->rxcmpl_ring_cfg.offload) ? "Enabled" : "Disabled",
	    dhd_hlp->rxcmpl_ring_cfg.max_items);

	if (DHD_RNR_IS_OFFL_SUPPORTED(dhd_hlp)) {
	    /* Get wakeup register information from runner */
	    rdpa_dhd_helper_wakeup_information_get(&wakeup_info);
	}

	if (DHD_RNR_TXSTS_OFFLOAD(dhd_hlp)) {
	    /* Setup Tx complete thread wake up doorbell */
	    rc = dhd_runner_wakeup_init(tx_soft_doorbell,
	        wakeup_info.tx_complete_wakeup_register,
	        HTOL32(wakeup_info.tx_complete_wakeup_value));
	    if (rc) {
	        DHD_ERROR(("dhd%d_rnr: failed to initialize runner tx wakeup",
	            dhd_hlp->dhd->unit));
	        DHD_ERROR((" doorbell = <0x%p> reg = <0x%llx>, val = <0x%08x>\r\n",
	            tx_soft_doorbell,
	            (uint64_t)wakeup_info.tx_complete_wakeup_register,
	            HTOL32(wakeup_info.tx_complete_wakeup_value)));
			goto exit;
	    }
	    dhd_hlp->rnr_en_feat.txoffl = 1;
	    RLOG("TX wakeup info: reg = <0x%llx>, val = <0x%08x>",
	        (uint64_t)wakeup_info.tx_complete_wakeup_register,
	        HTOL32(wakeup_info.tx_complete_wakeup_value));
	}

	if (DHD_RNR_RX_OFFLOAD(dhd_hlp)) {
	    /* Setup Rx complete thread wake up doorbell */
	    rc = dhd_runner_wakeup_init(rx_soft_doorbell,
	        wakeup_info.rx_complete_wakeup_register,
	        HTOL32(wakeup_info.rx_complete_wakeup_value));
	    if (rc) {
	        DHD_ERROR(("dhd%d_rnr: failed to initialize runner rx wakeup",
	            dhd_hlp->dhd->unit));
	        DHD_ERROR((" doorbell = <0x%p> reg = <0x%llx>, val = <0x%08x>\r\n",
	            rx_soft_doorbell,
	            (uint64_t)wakeup_info.rx_complete_wakeup_register,
	            HTOL32(wakeup_info.rx_complete_wakeup_value)));
			goto exit;
	    }
	    dhd_hlp->rnr_en_feat.rxoffl = 1;
	    RLOG("RX wakeup info: reg = <0x%llx>, val = <0x%08x>",
	        (uint64_t)wakeup_info.rx_complete_wakeup_register,
	        HTOL32(wakeup_info.rx_complete_wakeup_value));
	}

	/* enable/disable swfeatures from default/storage settings */
	rc = dhd_runner_swfeatures_init(dhd_hlp);
	if (rc) {
		DHD_ERROR(("dhd%d_rnr: Failed to initialize swfeatures\n", dhd_hlp->dhd->unit));
		goto exit;
	}


	return dhd_hlp;

exit:
	if (rc < 0)
	    dhd_runner_detach(dhd, dhd_hlp);
	bdmf_global_trace_level = old_trace_level;

	return (dhd_runner_hlp_t*)NULL;
}


void
dhd_runner_detach(dhd_pub_t *dhd, struct dhd_runner_hlp *dhd_hlp)
{
	DHD_INFO(("%s: dhd_hlp<%p dhd<%p>\n", __FUNCTION__, dhd_hlp, dhd));

	if (dhd_hlp == NULL)
	    return;

	dhd_helper_detach(dhd_hlp);

	if (dhd_hlp->coherent_mem_pool.va != NULL) {
	    dhd_runner_free_mem(dhd_hlp, &dhd_hlp->coherent_mem_pool,
	        DHD_RNR_MEM_ALLOC_TYPE_COHERENT);
	    DHD_INFO(("[COHERENT_POOL_%d] Freed 0x%x size pool 0x%p\r\n",
	        dhd_hlp->dhd->unit, dhd_hlp->coherent_mem_pool.len,
	        dhd_hlp->coherent_mem_pool.va));
	    dhd_hlp->coherent_mem_pool.va = NULL;
	}

	DHD_RNR_DUMP_MEM_ALLOC_CNTRS(dhd_hlp);

	if (dhd_hlp->cpu_obj) {
	    bdmf_put(dhd_hlp->cpu_obj);
	    dhd_hlp->cpu_obj = NULL;
	}

	dhd_hlp->dhd = NULL;
	MFREE(dhd->osh, dhd_hlp, sizeof(struct dhd_runner_hlp));
}

/*
 * +----------------------------------------------------------------------------
 *           Section: N:M Flowring Split Manager
 * +----------------------------------------------------------------------------
 */
/*
 * Select a HW or SW managed flowring. Currently STA-id is not available and if
 * required, it may be passed. Several select algorithms may be implemented and
 * one may be picked and placed into the flowmgr.
 */
uint16
dhd_runner_flowmgr_select(dhd_runner_flowmgr_t *flowmgr,
	int prio, bool bcmc, bool prefer_hw_ring, bool *is_hw_ring)
{
	int select, wme_ac;
	void *id_map[2];
	bool hw_ring[2];
	const bool is_hw_ring_c = TRUE;
	const bool is_sw_ring_c = FALSE;
	uint16 flow_id = ID16_INVALID;

	if (bcmc)
	    wme_ac = wme_ac_bcmc;
	else
	    wme_ac = WME_PRIO2AC(prio);

	hw_ring[!prefer_hw_ring] = is_hw_ring_c;
	id_map[!prefer_hw_ring] = flowmgr->hw_id16_map[wme_ac];

	hw_ring[prefer_hw_ring] = is_sw_ring_c;
	id_map[prefer_hw_ring] = flowmgr->sw_id16_map[wme_ac];


	for (select = 0; select < 2; select++) {
	    flow_id = id16_map_alloc(id_map[select]);
	    if (flow_id != ID16_INVALID) {
	        *is_hw_ring = hw_ring[select];
	        break;
	    }
	}

	RPR2("%s: (0x%p, %d, %s, %s, %s) = %d", __FUNCTION__, flowmgr, prio,
	    (bcmc)?"true":"false", (prefer_hw_ring)?"true":"false",
	    (*is_hw_ring)?"true":"false", flow_id);


	return flow_id;
}

/*
 * returns whether hw flow ring is preferred or sw flow ring is preferred
 * based on the current policy set for the radio
*/
uint16
dhd_runner_flowring_select_policy(dhd_runner_flowmgr_t *flowmgr,
	int ifidx, int prio, uint8 *mac, int staidx, bool d11ac, bool *is_hw_ring)
{
	dhd_flowring_policy_t* policy;
	bool is_hw, bcmc;
	dhd_wme_ac_t ac;
	int index;
	uint16 flow_id;

	is_hw = FALSE;
	policy = flowmgr->policy;

	/* parse and update the user profile, if present */
	switch (policy->id) {

	    case dhd_rnr_policy_global:
	        is_hw = policy->all_hw;
	        break;

	    case dhd_rnr_policy_intf:
	        if (ifidx <= policy->max_sta) is_hw = TRUE;
	        break;

	    case dhd_rnr_policy_clients:
	        if (staidx <= policy->max_sta) is_hw = TRUE;
	        break;

	    case dhd_rnr_policy_aclist:
	        ac = WME_PRIO2AC(prio);
	        is_hw = policy->aclist_hw[ac];
	        break;

	    case dhd_rnr_policy_maclist:
	        for (index = 0; index < DHD_RNR_POLICY_MAX_MACLIST; index++)
	        {
	            if (policy->mac_addr[index][6])
	            {
	                if (!memcmp(mac, policy->mac_addr[index], ETHER_ADDR_LEN))
	                {
	                    is_hw = TRUE;
	                    break;
	                }
	             }
	             else
	                break;

	        }
	        break;

	    case dhd_rnr_policy_dllac:
	        /*     d11ac_hw        d11ac        is_hw
	         *         1               1          1
	         *         1               0          0
	         *         0               1          0
	         *         0               0          0
	         */
	        is_hw = policy->d11ac_hw & d11ac;
	        break;
	}

	DHD_TRACE(("%s: flow ring preference %s\r\n", __FUNCTION__,
	    (is_hw) ? "HW" : "SW"));
	RPR("%s: flow ring preference %s\r\n", __FUNCTION__,
	    (is_hw) ? "HW" : "SW");

	bcmc = ETHER_ISMULTI(mac);
	/*
	 * For STA dest and WDS dest we allocate entry based on prio only,
	 * not based on mac address
	 */
	if (DHD_IF_ROLE_STA(flowmgr->dhd_hlp->dhd, ifidx) ||
	    DHD_IF_ROLE_WDS(flowmgr->dhd_hlp->dhd, ifidx)) {
	    bcmc = FALSE;
	}
	flow_id = dhd_runner_flowmgr_select(flowmgr, prio, bcmc, is_hw,
	    is_hw_ring);

	/*
	 * If flow_id is invalid, dongle is supporting additional clients than advertised,
	 * re-use id's from other ac's to support them
	 */
	if (flow_id == ID16_INVALID) {
	    ac = wme_ac_bk;
	    while ((flow_id == ID16_INVALID) && (ac < wme_ac_max)) {
	        flow_id = dhd_runner_flowmgr_select(flowmgr, WME_AC2PRIO(ac), FALSE,
	            is_hw, is_hw_ring);
	        ac++;
	    }
	}

	return flow_id;
}


static int
dhd_runner_flowring_alloc(dhd_runner_flowmgr_t *flowmgr, dhd_wme_ac_t wme_ac,
	bool force_dhd)
{
	int ring_sz;
	uint16 flags, flow_id;
	void *id16_map_hndl;
	rdpa_dhd_flring_cache_t *rdpa_cache;
	dhd_rdpa_flring_cache_t *dhd_cache;


	flow_id = id16_map_alloc(flowmgr->flow_ids_map);
	if (flow_id == FLOWID_INVALID) {
	    DHD_ERROR(("%s: Invalid flowid", __FUNCTION__));
	    return BCME_ERROR;
	}

	rdpa_cache = flowmgr->rdpa_flring_cache + flow_id;
	dhd_cache = flowmgr->dhd_flring_cache + flow_id;
	rdpa_cache->base_addr_high = 0U;
	flags = wme_ac << DHD_RNR_FLRING_WME_AC_SHIFT;

	ring_sz = flowmgr->phy_items[wme_ac] * flowmgr->dhd_hlp->txpost_ring_cfg.size;


	if ((flowmgr->hw_mem_size >= ring_sz) && (force_dhd == FALSE)) {
	    /* Runner managed txpost ring */
	    rdpa_cache->base_addr_low = (uint32)(RDD_RSV_VIRT_TO_PHYS(
	        flowmgr->hw_mem_virt_base_addr, flowmgr->hw_mem_phys_base_addr,
	        flowmgr->hw_mem_addr));
	    dhd_cache->base_va = (void*)flowmgr->hw_mem_addr;
	    flags |= DHD_RNR_FLRING_IN_RUNNER_FLAG | DHD_RNR_FLRING_DISABLED_FLAG;
	    flowmgr->hw_mem_size -= ring_sz;
	    flowmgr->hw_mem_addr += ring_sz;
	    flowmgr->hw_ring_cnt[wme_ac]++;
	    id16_map_hndl = flowmgr->hw_id16_map[wme_ac];
	} else if (!(flowmgr->dhd_hlp->rnr_en_feat.txoffl ^ flowmgr->dhd_hlp->rnr_en_feat.npm)) {
	    /* DHD managed txpost ring */
	    dhd_dma_buf_t dma_buf;

	    dma_buf.len = ring_sz;
	    dma_buf.va = dhd_runner_alloc_mem(flowmgr->dhd_hlp, &dma_buf, DHD_RNR_MEM_TYPE_DHD);

	    if (!dma_buf.va) {
	        DHD_ERROR(("%s: dma_buf alloc for flow_id %d failure\n",
	            __FUNCTION__, flow_id));
	        return BCME_ERROR;
	    }

	    rdpa_cache->base_addr_low = PHYSADDRLO(dma_buf.pa);
	    rdpa_cache->base_addr_high = PHYSADDRHI(dma_buf.pa);
	    dhd_cache->base_va = dma_buf.va;
	    flowmgr->sw_ring_cnt[wme_ac]++;
	    id16_map_hndl = flowmgr->sw_id16_map[wme_ac];
	} else {
	    DHD_ERROR(("dor%d: Not enough Reserved memory", flowmgr->dhd_hlp->dhd->unit));
	    DHD_ERROR(("dor%d: Not enough Reserved memory\n", flowmgr->dhd_hlp->dhd->unit));
	    return BCME_ERROR;
	}

#if defined(RNR_DHD_HLPR_BKUPQUEUE)
	rdpa_cache->phy_ring_size = htons(flowmgr->phy_items[wme_ac]);
#endif /* RNR_DHD_HLPR_BKUPQUEUE */
	/* items = physical ring size + backup queues if supported */
	rdpa_cache->items = htons(flowmgr->profile->items[wme_ac]);
	rdpa_cache->base_addr_low = htonl(rdpa_cache->base_addr_low);
	rdpa_cache->flags = htons(flags);
	id16_map_free(id16_map_hndl, flow_id);

	ARCH_FLUSH_COHERENT((void *)rdpa_cache, sizeof(rdpa_dhd_flring_cache_t));

	RPR2("%s: allocated %s flowring [%d], base low address [%x] \r\n",
	    __FUNCTION__, (flags & DHD_RNR_FLRING_IN_RUNNER_FLAG) ? "HW" : "SW",
	    flow_id, rdpa_cache->base_addr_low);

	return BCME_OK;
}

void *
dhd_runner_flowmgr_init(dhd_runner_hlp_t *dhd_hlp, int max_h2d_rings,
	int max_bss)
{
	int ac;
	uint16 total_ids;
	int rings_to_allocate[wme_ac_max + 1];  /* reused for per tid, too */
	int weighted_rings_to_allocate = 0;
	dhd_runner_flowmgr_t *flowmgr;
	dhd_pub_t *dhdp;
	dhd_dma_buf_t *dma_buf;
	rdpa_dhd_init_cfg_t *dhd_init_cfg;
	dhdp = dhd_hlp->dhd;
	flowmgr = &dhd_hlp->flowmgr;
	dhd_init_cfg = &dhd_hlp->dhd_init_cfg;

	/* Determine the flring split for max_bss = bcmc and ucast per ac|tid */
	flowmgr->max_h2d_rings = max_h2d_rings;
	flowmgr->max_bss = max_bss;

	/* When per TID ucast flowrings are used, the per ac profile/policy will
	 * be applied for carving runner memory. As such the rings_to_allocate[]
	 * will count twice as much rings for per TID (as compared to per AC).
	 */
	flowmgr->max_uc_rings = /* per ac or per tid, ucast tx post rings */
	    (max_h2d_rings - BCMPCIE_H2D_COMMON_MSGRINGS - max_bss) / wme_ac_max;

	ASSERT(max_h2d_rings == ((flowmgr->max_uc_rings * wme_ac_max)
	                         + BCMPCIE_H2D_COMMON_MSGRINGS + max_bss));

	/* Pick the flow ring selector. */
	flowmgr->select_fn = dhd_runner_flowring_select_policy;
	flowmgr->dhd_hlp = dhd_hlp;

	/* Setup the DMA-able buffer for flring cache in uncached memory. */
	dma_buf = &dhd_hlp->flring_cache_dma_buf;
	if (dma_buf->va == NULL) {
	    dma_buf->len = sizeof(rdpa_dhd_flring_cache_t) * max_h2d_rings;
	    dma_buf->len += sizeof(dhd_rdpa_flring_cache_t) * max_h2d_rings;
	    dma_buf->len = ALIGN_SIZE(dma_buf->len, L1_CACHE_BYTES);

	    dma_buf->va = dhd_runner_alloc_mem(dhd_hlp, dma_buf, DHD_RNR_MEM_TYPE_RNR);

	    if (!dma_buf->va) {
	        DHD_ERROR(("%s: flring_cache_dma_buf len<%d> alloc failure\n",
	            __FUNCTION__, dma_buf->len));
	        goto error_rtn;
	    }

	    /* Register with runner */
	    dhd_init_cfg->tx_post_mgmt_arr_base_addr =
	        (void *)PHYSADDRLO(dma_buf->va);
	    dhd_init_cfg->tx_post_mgmt_arr_base_phys_addr =
	        (uint32_t)PHYSADDRLO(dma_buf->pa);
	    dhd_init_cfg->tx_post_mgmt_arr_entry_count =
	        (uint32_t) flowmgr->max_h2d_rings;

	    /* Register with flowmgr */
	    flowmgr->rdpa_flring_cache = (rdpa_dhd_flring_cache_t*)dma_buf->va;
	    flowmgr->dhd_flring_cache =
	        (dhd_rdpa_flring_cache_t*)(flowmgr->rdpa_flring_cache + max_h2d_rings);
	}

	if (DHD_RNR_TXSTS_OFFLOAD(dhd_hlp)) {
	    dhd_runner_ring_cfg_t *ring;

	    memset(flowmgr->hw_mem_addr, 0, flowmgr->hw_mem_size);
	    RLOG("%s: dhd%d_rnr bootmem addr<%p> size<%u>", __FUNCTION__,
	        dhdp->unit, flowmgr->hw_mem_addr, flowmgr->hw_mem_size);

	    /* Initialize the base address */
	    ring = &dhd_hlp->txpost_ring_cfg;
	    dhd_hlp->dhd_init_cfg.tx_post_flow_ring_base_addr = (void *)
	        ((unsigned long)flowmgr->hw_mem_addr -
	        (BCMPCIE_H2D_COMMON_MSGRINGS * ring->max_items * ring->size));
	}

	/* Get flowring profile and policy from NVRAM area, if present */
	flowmgr->profile = dhd_runner_profile_init(dhd_hlp);
	ASSERT(flowmgr->profile != NULL);

	flowmgr->policy = dhd_runner_policy_init(dhd_hlp);
	ASSERT(flowmgr->policy != NULL);

	/* Populate the flowids that will be used from 2..max */
	total_ids = flowmgr->max_h2d_rings - FLOW_RING_COMMON;
	flowmgr->flow_ids_map = id16_map_init(dhdp->osh, total_ids, FLOWID_RESERVED);

#if defined(BCM_DBG_ID16)
#error "remove ASSERT val16 or use total_ids in id16_map_init below"
#endif

	/* id16 map allocators for ucast rings per access category for N stations */
	for (ac = wme_ac_bk; ac < wme_ac_max; ac++) {
	    rings_to_allocate[ac] = flowmgr->max_uc_rings;
	    flowmgr->hw_id16_map[ac] =
	        id16_map_init(dhdp->osh, flowmgr->max_uc_rings, ID16_INVALID);
	    flowmgr->sw_id16_map[ac] =
	        id16_map_init(dhdp->osh, flowmgr->max_uc_rings, ID16_INVALID);
	}
	/* id16 map allocators for BCMC rings (one per BSS) */
	rings_to_allocate[wme_ac_bcmc] = flowmgr->max_bss;

	/* Check if full dor memory available if n+m is not enabled */
	if (dhd_hlp->rnr_en_feat.txoffl == 1) {
	    uint32 ring_sz = 0;
	    uint32 full_dor_sz = 0;
	    bool force_m = FALSE;

	    for (ac = wme_ac_bk; ac <= wme_ac_max; ac++) {
	        ring_sz = flowmgr->phy_items[ac] * dhd_hlp->txpost_ring_cfg.size;
	        full_dor_sz += ring_sz * rings_to_allocate[ac];
	        if (flowmgr->profile->weight[ac] == 0) force_m = TRUE;
	    }

	    if (dhd_hlp->rnr_en_feat.npm == 0) {
	        if (flowmgr->hw_mem_size < full_dor_sz) {
	            RLOG(CLRerr "\n========================================================");
	            RLOG("dor%d Reserved memory [%d] < full offload [%d], aborting",
	                dhd_hlp->dhd->unit, flowmgr->hw_mem_size, full_dor_sz);
	            RLOG("========================================================" CLRnl);
	            goto error_rtn;
	        }
	        if (force_m == TRUE) {
	            RLOG(CLRerr "\n========================================================");
	            RLOG("dor%d profile N+M settings not supported, aborting",
	                dhd_hlp->dhd->unit);
	            RLOG("========================================================" CLRnl);
	            goto error_rtn;
	        }
	    } else if (flowmgr->hw_mem_size >= full_dor_sz) {
	    } else if ((flowmgr->hw_mem_size >= full_dor_sz) && (force_m == FALSE)) {
	        dhd_hlp->rnr_en_feat.npm = 0;
	    }
	}

	flowmgr->hw_id16_map[wme_ac_bcmc] =
	    id16_map_init(dhdp->osh, flowmgr->max_bss, ID16_INVALID);
	flowmgr->sw_id16_map[wme_ac_bcmc] =
	    id16_map_init(dhdp->osh, flowmgr->max_bss, ID16_INVALID);

	/* Allocate all rings with 0 weight (forced non-offload) in DHD memory */
	for (ac = wme_ac_bk; ac <= wme_ac_max; ac++)
	{
	    if (flowmgr->profile->weight[ac] == 0) {
	        while (rings_to_allocate[ac]) {
	            if (dhd_runner_flowring_alloc(flowmgr, ac, TRUE) == BCME_ERROR) {
	                DHD_ERROR(("Failed to allocate %d %s rings",
	                    rings_to_allocate[ac], dhd_wme_ac_str[ac]));
	                        goto error_rtn;
	                    }
	            rings_to_allocate[ac]--;
	        }
	    }
	}

	/* Carve out ucast rings with -1 weights from reserved memory */

	/* First pass, allows all -1 weighted ACs to consume a max of
	 * DHD_RNR_PER_AC_PROFILE_BUDGET number of flowrings.
	 * This is done so that the per TID based ucast flowrings do not consume
	 * the entire Runner memory.
	 */
	for (ac = wme_ac_bk; ac < wme_ac_max; ac++)
	{
	    int ac_budget = DHD_RNR_PER_AC_PROFILE_BUDGET;

	    if (flowmgr->profile->weight[ac] == -1) {
	        while (rings_to_allocate[ac]) {
	            if (dhd_runner_flowring_alloc(flowmgr, ac, FALSE) == BCME_ERROR) {
	                DHD_ERROR(("Failed to allocate %d %s ucast rings",
	                    rings_to_allocate[ac], dhd_wme_ac_str[ac]));
	                goto error_rtn;
	            }
	            rings_to_allocate[ac]--;
	            if (--ac_budget == 0)
	                break; /* rest of rings_to_allocate[ac] in second pass */
	        }
	    } else {
	        weighted_rings_to_allocate += rings_to_allocate[ac];
	    }
	}

	/* Second pass: -1 weight AC leftover from DHD_RNR_PER_AC_PROFILE_BUDGET */
	for (ac = wme_ac_bk; ac < wme_ac_max; ac++)
	{
	    if (flowmgr->profile->weight[ac] == -1) {
	        while (rings_to_allocate[ac]) {
	            if (dhd_runner_flowring_alloc(flowmgr, ac, FALSE) == BCME_ERROR) {
	                DHD_ERROR(("Failed to allocate %d %s ucast rings",
	                    rings_to_allocate[ac], dhd_wme_ac_str[ac]));
	                goto error_rtn;
	            }
	            rings_to_allocate[ac]--;
	        }
	    }
	}

	/* Carve out ucast rings using round robin profile */
	while (weighted_rings_to_allocate) {
	    for (ac = wme_ac_bk; ac < wme_ac_max; ac++)
	    {
	        int weight;
	        weight = flowmgr->profile->weight[ac];
	        if ((weight == -1) || (weight == 0))
	            continue;
	        weight = LIMIT_TO_MAX(weight, rings_to_allocate[ac]);
	        rings_to_allocate[ac] -= weight;
	        weighted_rings_to_allocate -= weight;

	        while (weight) {
	            if (dhd_runner_flowring_alloc(flowmgr, ac, FALSE) == BCME_ERROR) {
	                DHD_ERROR(("Failed to allocate %d %s ucast rings",
	                    rings_to_allocate[ac], dhd_wme_ac_str[ac]));
	                goto error_rtn;
	            }
	            weight--;
	        }
	    }
	}

	/* Carve out bcmc rings */
	while (rings_to_allocate[wme_ac_bcmc]) {
	    if (dhd_runner_flowring_alloc(flowmgr, wme_ac_bcmc, FALSE) == BCME_ERROR) {
	        DHD_ERROR(("Failed to allocate %d bcmc rings",
	            rings_to_allocate[wme_ac_bcmc]));
	        goto error_rtn;
	    }
	    rings_to_allocate[wme_ac_bcmc]--;
	}

	/* By now the flow_ids_map should be empty */
	flowmgr->flow_ids_map = id16_map_fini(dhdp->osh, flowmgr->flow_ids_map);

	return flowmgr;

error_rtn:

	dhd_runner_flowmgr_fini(dhd_hlp, flowmgr);
	return NULL;
}

void *
dhd_runner_flowmgr_fini(dhd_runner_hlp_t *dhd_hlp, void *mgr)
{
	int ac, flring;
	dhd_pub_t *dhdp;
	rdpa_dhd_init_cfg_t *dhd_init_cfg;
	dhd_runner_flowmgr_t *flowmgr;
	rdpa_dhd_flring_cache_t *rdpa_cache;
	dhd_rdpa_flring_cache_t *dhd_cache;

	if (!dhd_hlp)
	    goto done;

	dhdp = dhd_hlp->dhd;
	dhd_init_cfg = &dhd_hlp->dhd_init_cfg;
	flowmgr = (dhd_runner_flowmgr_t*)mgr;

	/* walk all entries in flowring cache (skip 1st 2) */
	if (flowmgr->rdpa_flring_cache) {
	    for (flring = FLOWID_RESERVED; flring < flowmgr->max_h2d_rings; flring++)
	    {
	        void *va;
	        bool is_hw_ring;

	        rdpa_cache = flowmgr->rdpa_flring_cache + flring;
	        dhd_cache = flowmgr->dhd_flring_cache + flring;
	        va = dhd_cache->base_va;
	        is_hw_ring = ntohs(rdpa_cache->flags) & DHD_RNR_FLRING_IN_RUNNER_FLAG;
	        if (va && !is_hw_ring) {
	            dhd_dma_buf_t dma_buf;

	            dma_buf.va = va;
	            PHYSADDRLOSET(dma_buf.pa, ntohl(rdpa_cache->base_addr_low));
	            PHYSADDRHISET(dma_buf.pa, ntohl(rdpa_cache->base_addr_high));
	            dma_buf.len = ntohs(DHD_RNR_PHY_RING_SIZE(rdpa_cache));
	            dma_buf.len *= dhd_hlp->txpost_ring_cfg.size;
	            dma_buf.dmah = NULL;

	            rdpa_cache->base_addr_low = 0U;
	            dhd_cache->base_va = (void*)NULL;
	            rdpa_cache->items = 0;
#if defined(RNR_DHD_HLPR_BKUPQUEUE)
	            rdpa_cache->phy_ring_size = 0;
#endif /* RNR_DHD_HLPR_BKUPQUEUE */
	            rdpa_cache->flags = htons(DHD_RNR_FLRING_DISABLED_FLAG);
	            ARCH_FLUSH_COHERENT((void *)rdpa_cache, sizeof(rdpa_dhd_flring_cache_t));

	            dhd_runner_free_mem(dhd_hlp, &dma_buf, DHD_RNR_MEM_TYPE_DHD);
	        } else if (va && is_hw_ring) {
	            dhd_runner_flring_cache_enable(dhd_hlp, flring, 0);
	        }
	    }
	}

	if (dhd_hlp->flring_cache_dma_buf.va) {
	    dhd_dma_buf_t *dma_buf = &dhd_hlp->flring_cache_dma_buf;
	    dhd_runner_free_mem(dhd_hlp, dma_buf, DHD_RNR_MEM_TYPE_RNR);

	    /* Runner should not use tx_post_mgmt_arr_base_addr */
	    dhd_init_cfg->tx_post_mgmt_arr_base_addr = (void *)NULL;
	    dhd_init_cfg->tx_post_mgmt_arr_base_phys_addr = 0;
	    /* SASHAP: Do we need a call into runner rdpa? */
	}

	flowmgr->flow_ids_map = id16_map_fini(dhdp->osh, flowmgr->flow_ids_map);

	for (ac = wme_ac_bk; ac <= wme_ac_max; ac++) {
	    flowmgr->hw_id16_map[ac] =
	        id16_map_fini(dhdp->osh, flowmgr->hw_id16_map[ac]);
	    flowmgr->sw_id16_map[ac] =
	        id16_map_fini(dhdp->osh, flowmgr->sw_id16_map[ac]);
	}

done:
	return NULL;
}


uint16
dhd_runner_flowmgr_alloc(void *mgr,
	int ifidx, int prio, uint8 *mac, bool d11ac, bool *is_hw_ring)
{
	dhd_runner_flowmgr_t *flowmgr = (dhd_runner_flowmgr_t *)mgr;
	uint16 staid = ID16_INVALID;
	uint16 ringid = ID16_INVALID;

	/* Get the dhd helper object from the flowmgr address */
	ASSERT(mgr != NULL);

	staid = dhd_get_sta_cnt(flowmgr->dhd_hlp->dhd, ifidx, mac);

	ringid = flowmgr->select_fn(flowmgr, ifidx, prio, mac, staid, d11ac,
	    is_hw_ring);

#if defined(FLOW_RING_FLAG_SSID_MASK)
	/* update ssid ifidx in flowring cache for offloaded ring */
	if ((ringid != ID16_INVALID) && (*is_hw_ring == TRUE)) {
	    rdpa_dhd_flring_cache_t *rdpa_cache;
	    uint16 flags;

	    rdpa_cache = flowmgr->rdpa_flring_cache + ringid;
	    flags = ntohs(rdpa_cache->flags);
	    if (flags & DHD_RNR_FLRING_IN_RUNNER_FLAG) {
	        flags &= ~FLOW_RING_FLAG_SSID_MASK;
	        flags |= (ifidx << FLOW_RING_FLAG_SSID_SHIFT);
	        rdpa_cache->flags = htons(flags);
	        ARCH_FLUSH_COHERENT((void *)rdpa_cache, sizeof(rdpa_dhd_flring_cache_t));
	    }
	}
#endif /* FLOW_RING_FLAG_SSID_MASK */

	return ringid;
}

int
dhd_runner_flowmgr_free(void *mgr, uint16 flow_id)
{
	int wme_ac;
	void * id_map;
	dhd_runner_flowmgr_t *flowmgr;
	rdpa_dhd_flring_cache_t *rdpa_cache;
	uint16 flags;

	flowmgr = (dhd_runner_flowmgr_t *)mgr;
	rdpa_cache = flowmgr->rdpa_flring_cache + flow_id;

	flags = ntohs(rdpa_cache->flags);
	wme_ac = (flags & DHD_RNR_FLRING_WME_AC_MASK)
	          >> DHD_RNR_FLRING_WME_AC_SHIFT;
	id_map = (flags & DHD_RNR_FLRING_IN_RUNNER_FLAG) ?
	    flowmgr->hw_id16_map[wme_ac] : flowmgr->sw_id16_map[wme_ac];

	DHD_TRACE(("Free %s Flowid %d map = 0x%p, ac=%d\r\n",
	    (flags & DHD_RNR_FLRING_IN_RUNNER_FLAG) ? "HW" : "SW",
	    flow_id, id_map, wme_ac));
	RPR("Free %s Flowid %d map = 0x%p, ac=%d\r\n",
	    (flags & DHD_RNR_FLRING_IN_RUNNER_FLAG) ? "HW" : "SW",
	    flow_id, id_map, wme_ac);

	id16_map_free(id_map, flow_id);
	return BCME_OK;
}

/*
 * +----------------------------------------------------------------------------
 *      Section: Interface between DHD and Runner
 * +----------------------------------------------------------------------------
 */

static void
dhd_runner_flring_cache_enable(dhd_runner_hlp_t *dhd_hlp, uint32_t ringid,
	int enable)
{
	rdpa_dhd_flring_cache_t *flring;
	int rc;

	if (!dhd_hlp->flring_cache_dma_buf.va) {
	    DHD_ERROR(("%s: rdpa_flring_cache is not initialized\n", __FUNCTION__));
	    return;
	}

	flring = dhd_hlp->flowmgr.rdpa_flring_cache;
	if (!flring) {
	    DHD_ERROR(("%s: rdpa_flring_cache flowid<%d> not initialized\n",
	        __FUNCTION__, ringid));
	    return;
	}

	flring += ringid;
	if (!(ntohs(flring->flags) & DHD_RNR_FLRING_IN_RUNNER_FLAG))
	{
	    DHD_INFO(("%s: SW ring %d, no need to update runner\n", __FUNCTION__,
	        ringid));
	   return;
	}

	dhd_runner_flring_cache_flag_update(flring, DHD_RNR_FLRING_DISABLED_FLAG,
	    !enable);
	rc = rdpa_dhd_helper_flow_ring_enable_set(dhd_hlp->dhd_helper_obj, ringid,
	    enable);

	if (rc != 0) {
	    DHD_ERROR(("dor%d rdpa_dhd_helper_flow_ring_enable_set(0x%p, %d, %d) returned %d\r\n",
	        dhd_hlp->dhd->unit, dhd_hlp->dhd_helper_obj, ringid, enable, rc));
	}
}

/**
 * Host DHD notifies Runner to perform an operation.
 */
int
dhd_runner_notify(struct dhd_runner_hlp *dhd_hlp,
	dhd_runner_ops_t ops, unsigned long arg1, unsigned long arg2)
{
	dhd_pub_t *dhd;
	int	rc;

	DHD_INFO(("%s: H2R dhd_hlp<%p> ops<%d> <0x%lx> <0x%lx>\n",
	    __FUNCTION__, dhd_hlp, ops, arg1, arg2));

	if ((dhd_hlp == NULL) || (dhd_hlp->dhd == NULL)) {
	    DHD_ERROR(("%s: invalid arg dhd_hlp<%p>\n", __FUNCTION__, dhd_hlp));
	    return BCME_BADARG;
	}

	dhd = dhd_hlp->dhd;

	switch (ops) {

	    /* Host requests Runner to read DMA Index buffer */
	    case H2R_IDX_BUF_RD_REQUEST: /* arg1:buffer ptr, arg2:read value ptr */
	        DHD_TRACE(("H2R_IDX_BUF_RD_REQUEST ptr<0x%p> valptr<0x%p>\n",
	            (uint16*)arg1, (uint16*)arg2));
	        {
	            uint16* ptr = (uint16*)arg1;
	            uint16 val;

	            if (DHD_RNR_MEM_TYPE_RNR == DHD_RNR_MEM_ALLOC_TYPE_COHERENT) {
	                ARCH_RD_DMA_IDX(ptr, val);
	            } else {
	                OSL_CACHE_INV((void *)ptr, 2);
	                val = LTOH16(*(ptr));
	            }
	            *(uint16*)arg2 = val;
	        }
	        break;

	    /* Host requests Runner to write DMA Index buffer */
	    case H2R_IDX_BUF_WR_REQUEST: /* arg1:buffer ptr, arg2:write value */
	        DHD_TRACE(("H2R_IDX_BUF_WR_REQUEST ptr<0x%p> val<0x%d>\n",
	            (uint16*)arg1, (uint16)arg2));
	        {
	            uint16* ptr = (uint16*)arg1;
	            uint16  val = (uint16)arg2;

	            if (DHD_RNR_MEM_TYPE_RNR == DHD_RNR_MEM_ALLOC_TYPE_COHERENT) {
	                ARCH_WR_DMA_IDX(ptr, val);
	            } else {
	                *ptr = htol16(val);
	                OSL_CACHE_FLUSH((void *)ptr, 2);
	            }
	        }
	        break;

	    /* Host notifies Runner to post initial buffers */
	    case H2R_RXPOST_NOTIF:
	        DHD_TRACE(("H2R_RXPOST_NOTIF\n"));

	        if (!DHD_RNR_IS_RX_OFFL_SUPPORTED(dhd_hlp))
	            return BCME_UNSUPPORTED;

	        if (rdpa_dhd_helper_rx_post_init(dhd_hlp->dhd_helper_obj)) {
	            DHD_ERROR(("rdpa_dhd_helper_rx_post_init(0x%p) failed\r\n",
	                dhd_hlp->dhd_helper_obj));
	            return BCME_NOMEM;
	        }

	        break;

	    /* Host notifies Runner to free buffers in RxPost */
	    case H2R_RXPOST_FREE_NOTIF:
	        DHD_TRACE(("H2R_RXPOST_FREE_NOTIF\n"));

	        if (!DHD_RNR_IS_RX_OFFL_SUPPORTED(dhd_hlp))
	            return BCME_UNSUPPORTED;

	        if (rdpa_dhd_helper_rx_post_uninit(dhd_hlp->dhd_helper_obj)) {
	            DHD_ERROR(("rdpa_dhd_helper_rx_post_uninit(0x%p) failed\r\n",
	                dhd_hlp->dhd_helper_obj));
	            return BCME_ERROR;
	        }
	        break;

	    /* Host notifies Runner to re-initialize buffers in RxPost */
	    case H2R_RXPOST_REINIT_NOTIF:
	        DHD_TRACE(("H2R_RXPOST_REINIT_NOTIF\n"));

	        if (!DHD_RNR_IS_RX_OFFL_SUPPORTED(dhd_hlp))
	            return BCME_UNSUPPORTED;

	        if (rdpa_dhd_helper_rx_post_reinit(dhd_hlp->dhd_helper_obj)) {
	            DHD_ERROR(("rdpa_dhd_helper_rx_post_uninit(0x%p) failed\r\n",
	                dhd_hlp->dhd_helper_obj));
	            return BCME_ERROR;
	        }
	        break;

	    /* Host notifies Runner to process D2H RxCompl */
	    case H2R_RX_COMPL_NOTIF:
	        DHD_TRACE(("H2R_RX_COMPL_NOTIF, unit %d\n", dhd_hlp->dhd->unit));
	        RPR2("H2R_RX_COMPL_NOTIF");

	        if (dhd_hlp->ipend == FALSE) {
	            rdpa_dhd_helper_complete_wakeup(dhd_hlp->dhd->unit, FALSE);
	            dhd_hlp->h2r_rx_compl_notif++;
	        }
	        break;


	    /* Host notifies Runner to transmit a packet */
	    case H2R_TXPOST_NOTIF: /* arg1:pktptr arg2:ifid */
	        DHD_TRACE(("H2R_TXPOST_NOTIF pkt<0x%p> len<%d> flowring<%d> ifidx<%d>\n",
	            (void*)arg1, PKTLEN(dhd->osh, (void*)arg1),
	            DHD_PKT_GET_FLOWID((void*)arg1), (uint32)arg2));
	        RPR1("H2R_TXPOST_NOTIF pkt<0x%p> ifid<%d>", (void*)arg1, (int)arg2);

	        rc = dhd_runner_txpost(dhd, (void*)arg1, (uint32)arg2);
	        dhd_hlp->h2r_txpost_notif++;
	        return rc;

	    /* Host notifies Runner to process D2H TxCompl */
	    case H2R_TX_COMPL_NOTIF:
	        DHD_TRACE(("H2R_TX_COMPL_NOTIF\n"));
	        RPR1("H2R_TX_COMPL_NOTIF");

	        if (dhd_hlp->ipend == FALSE) {
	            rdpa_dhd_helper_complete_wakeup(dhd_hlp->dhd->unit, TRUE);
	            dhd_hlp->h2r_tx_compl_notif++;
	        }
	        break;


	    /* Host notifies Runner to attach/detach/register a DMA Buf */
	    case H2R_DMA_BUF_NOTIF:
	        DHD_TRACE(("H2R_DMA_BUF_NOTIF type<0x%08x> dma_buf<0x%p>\n",
	            (int)arg1, (dhd_dma_buf_t*)arg2));

	        return dhd_runner_dma_buf_init(dhd_hlp, dhd->osh,
	                 (dhd_runner_dma_buf_t)arg1, (dhd_dma_buf_t*)arg2);


	    case H2R_FLRING_INIT_NOTIF:
	        DHD_TRACE(("H2R_FLRING_CACHE_NOTIF arg1<%d> arg2<0x%08x>\n",
	            (int)arg1, (int)arg2));
	        ASSERT(arg1 == dhd_hlp->flowmgr.max_h2d_rings);
	        *((rdpa_dhd_flring_cache_t**)arg2) = dhd_hlp->flowmgr.rdpa_flring_cache;
	        break;

	    /* Host notifies Runner to complete configuration */
	    case H2R_INIT_NOTIF:
	        DHD_TRACE(("H2R_INIT_NOTIF dhd_hlp<0x%p> pci_dev<0x%p>\n",
	                dhd_hlp, (struct pci_dev*)arg1));

	        return dhd_runner_init(dhd_hlp, (struct pci_dev*)arg1);

	    /* Host notifies Runner to enable a flowring */
	    case H2R_FLRING_ENAB_NOTIF: /* arg1:flowid [2..N] */
	        DHD_TRACE(("H2R_FLRING_ENAB_NOTIF flowring<%d>\n", (int)arg1));

	        dhd_runner_flring_cache_enable(dhd_hlp, (int)arg1, 1);
	        break;

	    /* Host notifies Runner to disable a flowring */
	    case H2R_FLRING_DISAB_NOTIF: /* arg1:flowid [2..N] */
	        DHD_TRACE(("H2R_FLRING_DISAB_NOTIF flowring<%d>\n", (int)arg1));

	        dhd_runner_flring_cache_enable(dhd_hlp, (int)arg1, 0);
	        break;

	    /* Host notifies Runner to flush a flowring */
	    case H2R_FLRING_FLUSH_NOTIF: /* arg1:flowid [2..N] arg2:rd_idx [0..0xfffe, 0xffff] */
	        DHD_TRACE(("H2R_FLRING_FLUSH_NOTIF flowring<%d> data<0x%x>\n",
	            (int)arg1, (uint16)arg2));

#if defined(RNR_DHD_HLPR_FFRD)
	        {
	            rdpa_dhd_ffd_data_t data;

	            data.flowring_idx = arg1;
	            data.read_idx = arg2;
	            data.read_idx_valid = (arg2 < 0xFFFF) ? 1 : 0;
	            rc = rdpa_dhd_helper_flush_set(dhd_hlp->dhd_helper_obj, (bdmf_number)data.u32);
	        }
#else /* !RNR_DHD_HLPR_FFRD */
	        rc = rdpa_dhd_helper_flush_set(dhd_hlp->dhd_helper_obj, arg1);
#endif /* !RNR_DHD_HLPR_FFRD */

	        if (rc != 0) {
	            DHD_ERROR(("dor%d rdpa_dhd_helper_flush_set(0x%p, %d) returned %d\r\n",
	                dhd_hlp->dhd->unit, &dhd_hlp->dhd_helper_obj, (int)arg1, rc));
	            return BCME_ERROR;
	        }
	        break;

	    /* Host notifies Runner to configure aggregation */
	    case H2R_AGGR_CONFIG_NOTIF: /* arg1:dhd_runner_aggr_config_t* */
	        DHD_TRACE(("H2R_AGGR_CONFIG_NOTIF aggr_config<0x%p>\n", (void*)arg1));

	        /* Check if any tx flow rings are offloaded */
	        if (DHD_RNR_TXSTS_OFFLOAD(dhd_hlp)) {
	            dhd_runner_aggr_config_t* aggr_cfg = (dhd_runner_aggr_config_t*)arg1;
#if defined(RNR_DHD_HLPR_LBRAGGR)
	            bdmf_number aggr_size;
	            bdmf_number aggr_timer;
	            bdmf_index wme_ac = 0;
	            int prio = 0;
	            int rc = 0;
#endif /* RNR_DHD_HLPR_LBRAGGR */

	            /* Check if Runner supports LBR aggregation */
	            if (dhd_hlp->rnr_sup_feat.lbraggr == 0) {
	                DHD_ERROR(("dhd%d_rnr: LBR Aggregation is not supported by Runner\n",
	                    dhd_hlp->dhd->unit));
	                return BCME_UNSUPPORTED;
	            }

	            /* Sanity Check input parameters */
	            if ((aggr_cfg == NULL) || (dhd_hlp->dhd_helper_obj == NULL)) {
	                DHD_ERROR(("dhd%d_rnr: NULL Config or runner DHD helper obj\n",
	                    dhd_hlp->dhd->unit));
	                return BCME_ERROR;
	            }

	            DHD_TRACE(("dhd%d_rnr: aggr mask<0x%x>, len<%d pkts>, timeout<%d msec>\n",
	                dhd_hlp->dhd->unit, aggr_cfg->en_mask, aggr_cfg->len,
	                aggr_cfg->timeout));

#if defined(RNR_DHD_HLPR_LBRAGGR)
	            /*
	             * Convert DHD parameters to Runner parameters
	             * DHD (aggr_en + aggr_len) to  Runner (aggr_size)
	             * DHD (prio) to Runner (wme_ac)
	             */
	            while (prio < wme_ac_max) {
	                if ((1 << prio) & aggr_cfg->en_mask) {
	                    aggr_size = (bdmf_number)aggr_cfg->len;
	                } else {
	                    aggr_size = 1;
	                }
	                wme_ac = (bdmf_index)WME_PRIO2AC(prio);
	                rc = rdpa_dhd_helper_aggregation_size_set(
	                    dhd_hlp->dhd_helper_obj, wme_ac, aggr_size);

	                if (rc < 0) {
	                    DHD_ERROR(("dhd%d_rnr: set aggr size failed %d\r\n",
	                        dhd_hlp->dhd->unit, rc));
	                }
	                prio++;
	            }

	            dhd_hlp->rnr_en_feat.lbraggr = (aggr_cfg->en_mask) ? 1 : 0;

	            /* Set the aggregation timeout in the runner */
	            aggr_timer = (bdmf_number)aggr_cfg->timeout;
	            rc = rdpa_dhd_helper_aggregation_timer_set(
	                dhd_hlp->dhd_helper_obj, aggr_timer);

	            if (rc < 0) {
	                DHD_ERROR(("dhd%d_rnr: set aggr timeout failed %d\r\n",
	                    dhd_hlp->dhd->unit, rc));
	            }
#endif /* RNR_DHD_HLPR_LBRAGGR */
	        }
	        break;

	    /* Host notifies Runner to set/unset sending nonacceleted packet txstatus to dhd */
	    case H2R_TXSTS_CONFIG_NOTIF: /* arg1: enable */
	        DHD_TRACE(("H2R_TXSTS_CONFIG_NOTIF enable <%d>\n", (int)arg1));

	        if (DHD_RNR_TXSTS_OFFLOAD(dhd_hlp)) {

	            dhd_hlp->txsts_ring_cfg.offload = arg1&DHD_RNR_TXSTS_CFG_OFFL_MASK;

	            if (!(arg1 & DHD_RNR_TXSTS_CFG_NONACCPKT_OFFL) &&
	                (dhd_hlp->rnr_sup_feat.txcmpl2host == 0)) {
	                /*
	                 * dhd requested to disable offload of txsts but runner can not support
	                 * sending nonacceleted packet txstatus to dhd
	                 */
	                dhd_hlp->txsts_ring_cfg.offload |= DHD_RNR_TXSTS_CFG_NONACCPKT_OFFL;
	                rc = BCME_UNSUPPORTED;
	                DHD_ERROR(("dhd%d_rnr: TXSTS2DHD not supported by Runner f/w\r\n",
	                    dhd_hlp->dhd->unit));
	            }
	        }
	        break;

	    /*
	     * Host notifies Runner to configure PCIe MSGRING format and size
	     *
	     * Runner only supports one format for all ring types.
	     * Currently TX_POST ring format setting is used for all ring types.
	     *
	     * Note: This API supports format setting for each ring type seperately for future use
	     */
	    case H2R_MSGRING_FORMAT_NOTIF:
	        DHD_TRACE(("H2R_MSGRING_FORMAT_NOTIF msgring<0x%x> val<0x%x>\n",
	            (uint8)arg1, (uint32)arg2));
	        {
	            uint16 ring = (uint16)arg1;
	            uint16 type = (uint16)(arg2 & 0xFFFF);
	            uint16 size = (uint16)((arg2 >> 16) & 0xFFFF);
	            bool tx_dor_en = TRUE;
	            bool rx_dor_en = TRUE;

	            switch (ring) {
	                case BCMPCIE_H2D_MSGRING_RXPOST_SUBMIT:
	                    dhd_hlp->rxpost_ring_cfg.size = size;
	                    dhd_hlp->rxpost_ring_cfg.type = type;
	                    if (!(dhd_hlp->rxpost_ring_cfg.sup_types & (1ul << type)))
	                        rx_dor_en = FALSE;
	                    break;

	                case BCMPCIE_D2H_MSGRING_TX_COMPLETE:
	                    dhd_hlp->txsts_ring_cfg.size = size;
	                    dhd_hlp->txsts_ring_cfg.type = type;
	                    if (!(dhd_hlp->txsts_ring_cfg.sup_types & (1ul << type)))
	                        tx_dor_en = FALSE;
	                    break;

	                case BCMPCIE_D2H_MSGRING_RX_COMPLETE:
	                    dhd_hlp->rxcmpl_ring_cfg.size = size;
	                    dhd_hlp->rxcmpl_ring_cfg.type = type;
	                    if (!(dhd_hlp->rxcmpl_ring_cfg.sup_types & (1ul << type)))
	                        rx_dor_en = FALSE;
	                    break;

	                case BCMPCIE_H2D_MSGRING_CONTROL_SUBMIT:
	                case BCMPCIE_D2H_MSGRING_CONTROL_COMPLETE:
	                    /* Runner does not need to registers control subn/cmplt rings */
	                    DHD_ERROR(("dor%d: MSG Ring [%d] not supported by Runner\n",
	                        dhd_hlp->dhd->unit, ring));
	                    return BCME_BADARG;

	                default:
	                    /* BCMPCIE_H2D_MSGRING_TXPOST_SUBMIT */
	                    dhd_hlp->txpost_ring_cfg.size = size;
	                    dhd_hlp->txpost_ring_cfg.type = type;
	                    if (!(dhd_hlp->txpost_ring_cfg.sup_types & (1ul << type)))
	                        tx_dor_en = FALSE;
	                    break;
	            }

	            /*
	             * Check Runner support for msgring formats (for old release compatibility)
	             */
	            if (!tx_dor_en) {
	                DHD_ERROR(("dor%d: %s ring format type [%d] not supported by Runner\n",
	                    dhd_hlp->dhd->unit,
	                    (ring == BCMPCIE_D2H_MSGRING_TX_COMPLETE) ? "TxC" : "TxP", type));

	                /*
	                 * WAR for runner platforms without CWI message formats support
	                 *
	                 * Disable runner offload if new message formats are not supported by
	                 * the runner platforms
	                 */
	                if (DHD_RNR_TXSTS_OFFLOAD(dhd_hlp)) {
	                    DHD_ERROR(("Force disabling Runner Offload for TX\n"));
	                    dhd_hlp->rnr_en_feat.txoffl = 0;
	                    dhd_hlp->rnr_en_feat.npm = 0;
	                    dhd_hlp->txsts_ring_cfg.offload = 0;
	                    dhd_hlp->txpost_ring_cfg.offload = 0;
	                    dhd_hlp->flowmgr.hw_mem_addr = (void*)NULL;
	                    dhd_hlp->flowmgr.hw_mem_size = 0;
	                    dhd_hlp->dhd_init_cfg.tx_post_flow_ring_base_addr = (void *)NULL;
	                    dhd_hlp->dhd_init_cfg.tx_complete_flow_ring_base_addr = (void*)NULL;
	               }
	            }

	            if (!rx_dor_en) {
	                DHD_ERROR(("dor%d: %s ring format type [%d] not supported by Runner\n",
	                    dhd_hlp->dhd->unit,
	                    (ring == BCMPCIE_H2D_MSGRING_RXPOST_SUBMIT) ? "RxP" : "RxC", type));
	               if (DHD_RNR_RX_OFFLOAD(dhd_hlp)) {
	                    DHD_ERROR(("Force disabling Runner Offload for TX\n"));
	                    dhd_hlp->rnr_en_feat.rxoffl = 0;
	                    dhd_hlp->rxcmpl_ring_cfg.offload = 0;
	                    dhd_hlp->rxpost_ring_cfg.offload = 0;
	                }
	            }

#if defined(RNR_DHD_HLPR_MSGRINGFRMT)
	            if (dhd_hlp->txpost_ring_cfg.type != dhd_hlp->rnr_en_feat.msgringformat) {
	                dhd_hlp->dhd_init_cfg.flow_ring_format = DOR_RINGTYPE_TO_RNR_RINGFMT(type);
	                dhd_hlp->rnr_en_feat.msgringformat = type;
	                RLOG("dor%d: MSG Ring format set to %d",
	                    dhd_hlp->dhd->unit, dhd_hlp->dhd_init_cfg.flow_ring_format);
	            }
#endif /* RNR_DHD_HLPR_MSGRINGFRMT */
	        }
	        break;

	    /*
	     * Host notifies Runner of PCIE IPC Capabilities (DHD) for negotiation
	     *
	     * arg1 (in):     Host Capabilities1 mask (uint32)
	     * arg2 (in/out): Pointer to DHD capabilities1 (in) DoR capabilities1 (out)
	     *
	     * Parse for the dependant capabilities and enable corresponding features if runner
	     * support them. Send back all the enabled capabilities supported by DoR
	     *
	     * Note: Currently fast flow ring delete cap is supported, can be exteneded to others
	     *       in future.
	     */
	    case H2R_PCIE_IPC_CAP1_NOTIF:
	        DHD_TRACE(("H2R_PCIE_IPC_CAP1_NOTIF cap_mask <0x%x> cap <0x%x>\n",
	            (uint32)arg1, *(uint32*)arg2));
	        if (arg1) {
	            uint32 hcap = arg1 & (*(uint32*)arg2);
	            uint32 rcap = 0;

	            if (hcap) {
#if defined(PCIE_IPC_HCAP1_FAST_DELETE_RING)
	                if (hcap & PCIE_IPC_HCAP1_FAST_DELETE_RING) {
	                    if (dhd_hlp->rnr_sup_feat.ffrd) {
	                        dhd_hlp->rnr_en_feat.ffrd = 1;
	                        rcap |= PCIE_IPC_HCAP1_FAST_DELETE_RING;
	                    }
	                }
#endif /* PCIE_IPC_HCAP1_FAST_DELETE_RING */
	            }

	            /* return the dor capabilities back */
	            *(uint32*)arg2 = rcap;
	        }
	        break;

	    default:
	        DHD_ERROR(("%s: Invalid H2R ops<%d> <0x%lx> <0x%lx>\n",
	                  __FUNCTION__, ops, arg1, arg2));
	        ASSERT(0);
	        return BCME_BADOPTION;
	}

	return BCME_OK;
}

/**
 * Runner requests Host DHD to handle an operation.
 */
int
dhd_runner_request(struct dhd_runner_hlp *dhd_hlp,
	dhd_runner_ops_t ops, unsigned long arg1, unsigned long arg2)
{
	dhd_pub_t *dhd;

	DHD_INFO(("R2H dhd_hlp<%p> ops<%d> <0x%lx> <0x%lx>\n",
	          dhd_hlp, ops, arg1, arg2));

	if ((dhd_hlp == NULL) || (dhd_hlp->dhd == NULL)) {
	    DHD_ERROR(("%s: invalid arg dhd_hlp<%p>\n", __FUNCTION__, dhd_hlp));
	    return BCME_BADARG;
	}

	dhd = dhd_hlp->dhd;

	switch (ops) {

	    /* Request Host to receive a packet that missed in Runner */
	    case R2H_RX_COMPL_REQUEST:
	        /* arg1:pkt ptr, arg2:info.reason_data:ifid */
	        DHD_TRACE(("R2H_RX_COMPL_REQUEST pkt<0x%p> ifid<%d>\n",
	            (void*)arg1, (int)arg2));
	        if (DHD_RNR_RX_OFFLOAD(dhd_hlp)) {
	            RPR1("dhd_bus_rx_frame pkt<0x%p> if<%d>", (void*)arg1, (int)arg2);
	            DHD_LOCK(dhd);
	            dhd_hlp->r2h_rx_compl_req++;
	            dhd_bus_rx_frame(dhd->bus, (void *)arg1, (int)arg2, 1);
	            DHD_UNLOCK(dhd);
	        } else {
	            DHD_ERROR(("dhd%d: unexpected R2H_RX_COMPL_REQUEST <0x%lx> <0x%lx>\n",
	                  dhd->unit, arg1, arg2));
	        }
	        break;

	    /* Runner requests Host to free a packet - unused */
	    case R2H_TX_COMPL_REQUEST:
	        /* arg1:pkt ptr */
	        DHD_TRACE(("R2H_TX_COMPL_REQUEST pkt<0x%p>\n", (void*)arg1));
	        if (DHD_RNR_TXSTS_OFFLOAD(dhd_hlp)) {
	            RPR1("R2H_TX_COMPL_REQUEST pkt<0x%p>", (void*)arg1);
	            dhd_hlp->r2h_tx_compl_req++;
	            dhd_prot_runner_txstatus_process(dhd, (void*)arg1);
	        } else {
	            DHD_ERROR(("dhd%d: unexpected R2H_TX_COMPL_REQUEST <0x%lx>\n",
	                  dhd->unit, arg2));
	        }
	        break;

	    /* Runner requests Host to wake dongle */
	    case R2H_WAKE_DNGL_REQUEST:
	        DHD_TRACE(("R2H_WAKE_DNGL_REQUEST\n"));
	        RPR1("R2H_WAKE_DNGL_REQUEST");
	        dhd_hlp->r2h_wake_dngl_req++;
	        dhd_runner_wakeup_dongle(dhd_hlp, (uint32)arg1, (uint32)arg2);
	        break;

	    default:
	        DHD_ERROR(("%s: invalid R2H ops<%d> <0x%lx> <0x%lx>\n",
	                  __FUNCTION__, ops, arg1, arg2));
	        ASSERT(0);
	        return BCME_BADOPTION;
	}

	return BCME_OK;
}


/*
 * +----------------------------------------------------------------------------
 *           Section: key read/write access
 * +----------------------------------------------------------------------------
 */

/**
 * Read key information from the persistent storage area
 *
 * input:    radio_idx: WLAN radio index
 *           buff:      Pointer to the buffer to store the profile
 *           len:       Length of the buffer
 *
 * output:   Actual length of the profile read.
 *           Length of 0 means, key doesn't exists
 *
*/
int
dhd_runner_key_get(int radio_idx, int key_id, char *buff, int len)
{
#if defined(DHD_RNR_NVRAM_KEYS)
	char key[DHD_RNR_KEY_STR_LEN];
	char *key_str = NULL;

	/* Prepare the nvram key */
	if (radio_idx != DHD_RNR_RADIO_IDX_ALL) {
	    snprintf(key, sizeof(key), dhd_runner_key_fmt_str[key_id], radio_idx);
	} else {
	    snprintf(key, sizeof(key), dhd_runner_key_fmt_str[key_id]);
	}
	key_str = nvram_get(key);

	RPR2("dhd%d_rnr nvram_get(%s) returned %s\r\n", radio_idx, key, key_str);

	if (key_str) {
	    /* Copy the key value */
	    snprintf(buff, len, "%s", key_str);
	    len = strlen(key_str);
		if (len < strlen(key_str)) {
	        DHD_ERROR(("dhd%d_rnr NVRAM param [%s] exceeds local buffer len [%d]\r\n",
	            radio_idx, key_str, len));
		}
	} else {
	    /* No key present in the NVRAM */
	    len = 0;
	}
#else /* !DHD_RNR_NVRAM_KEYS */
	char key[DHD_RNR_KEY_STR_LEN];

	/* Prepare the PSP key information */
	if (radio_idx != DHD_RNR_RADIO_IDX_ALL) {
	    snprintf(key, sizeof(key), dhd_runner_key_fmt_str[key_id], radio_idx);
	} else {
	    snprintf(key, sizeof(key), dhd_runner_key_fmt_str[key_id]);
	}
	key[sizeof(dhd_runner_key_fmt_str[key_id])-1] = '\0';

	/* Get the key value from PSP */
	len = kerSysScratchPadGet(key, buff, len);

	if (len > 0) {
	    buff[len-1] = '\0';
	} else {
	    /* key doesn't exists, Display warning */
	    RPR2("%s key not present, using built-in id [%d]\r\n",
	        key, radio_idx);
	}
#endif /* !DHD_RNR_NVRAM_KEYS */

	return len;
}

/**
 * Store key information to the persistent storage area
 *
 * input:    radio_idx: WLAN radio index
 *           key_id:    PSP key id used locally
 *           buff:      Pointer to the buffer profile
 *           len:       Length of the buffer
 *
 * output:   status of write
 *           >=0 success, <0 failure
 *
*/
int
dhd_runner_key_set(int radio_idx, int key_id, char *buff, int len)
{
#if defined(DHD_RNR_NVRAM_KEYS)
	/* NVRAM write from the kernel driver is not supported */
	return BCME_UNSUPPORTED;
#else /* !DHD_RNR_NVRAM_KEYS */
	char key[DHD_RNR_KEY_STR_LEN];

	/* Prepare the PSP key information */
	if (radio_idx != DHD_RNR_RADIO_IDX_ALL) {
	    snprintf(key, sizeof(key), dhd_runner_key_fmt_str[key_id], radio_idx);
	} else {
	    snprintf(key, sizeof(key), dhd_runner_key_fmt_str[key_id]);
	}
	key[sizeof(dhd_runner_key_fmt_str[key_id])-1] = '\0';

	return kerSysScratchPadSet(key, buff, len);
#endif /* !DHD_RNR_NVRAM_KEYS */
}

/*
 * +----------------------------------------------------------------------------
 *           Section: Flow ring profile management
 * +----------------------------------------------------------------------------
 */

/**
 * Read the profile information from the persistent storage area
 *
 * Read nvram variable radio%d_profile id,
 * if present and valid, override parameter profile_id
 *
 * if present,
 * parse and data fill the global table: dhd_rnr_profiles[radio_idx]
 *
*/
static dhd_flowring_profile_t*
dhd_runner_profile_init(struct dhd_runner_hlp *dhd_hlp)
{
	char buff[DHD_RNR_KEY_PPROFILE_STR_LEN];
	int  length = 0;
	int  profile_id;
	char *token, *profilestr;
	dhd_wme_ac_t ac;
	int radio_idx;
	int phy_ring_size;
	dhd_flowring_profile_t* profile;
	dhd_runner_flowmgr_t *flowmgr = &dhd_hlp->flowmgr;

	/* Update the user profiles if present */
	if (dhd_hlp->dhd->unit == 0) {
	    /* Use unit number to limit running this code only one time */
	    for (radio_idx = 0; radio_idx < DHD_RNR_MAX_RADIOS; radio_idx++) {
	        length = dhd_runner_key_get(radio_idx, DHD_RNR_KEY_PROFILE,
	            buff, sizeof(buff));

	        if (length == 0) {
	            /* radio Profile not present, continue to next radio profiles */
	            continue;
	        }
	        /* parse and update user profiles */
	        profilestr = buff;
	        token = bcmstrtok(&profilestr, " ", NULL);
	        sscanf(buff, "%d", &profile_id);

	        /* parse and update the user profiles  */
	        ac = wme_ac_bk;
	        token = bcmstrtok(&profilestr, " ", NULL);
	        while (token && (ac <= wme_ac_max)) {
	            if (dhd_hlp->rnr_sup_feat.npm) {
	                sscanf(token, "%d:%d", &dhd_rnr_profiles[radio_idx].weight[ac],
	                    &dhd_rnr_profiles[radio_idx].items[ac]);
	            } else {
	                int val;
	                char *token1;

	                sscanf(token, "%d", &val);
	                token1  = bcmstrstr(token, ":");
	                if (token1) {
	                    /* Old format with wieight. Discard weight as it is not used */
	                    sscanf(token1, ":%d", &dhd_rnr_profiles[radio_idx].items[ac]);
	                } else {
	                    dhd_rnr_profiles[radio_idx].items[ac] = val;
	                }
	            }
	            token = bcmstrtok(&profilestr, " ", NULL);
	            ac++;
	        }
	    }
	}

	/* Fetch the profile information of the radio, if present */
	radio_idx = dhd_hlp->dhd->unit;
	length = dhd_runner_key_get(radio_idx, DHD_RNR_KEY_PROFILE, buff,
	    sizeof(buff));

	if (length == 0) {
	    /* Profile not present, return default radio profile */
	    profile_id = DHD_RNR_FLOWRING_DEFAULT_PROFILE_ID;
	    goto bkupq;
	}

	/* parse profile_id from profile string */
	profilestr = buff;
	token = bcmstrtok(&profilestr, " ", NULL);
	sscanf(buff, "%d", &profile_id);

	if ((!dhd_hlp->rnr_sup_feat.npm) && dhd_rnr_profiles[profile_id].npm) {
	    RLOG("profile id %d key is not supported, using built-in profile id %d",
	        profile_id, DHD_RNR_FLOWRING_DEFAULT_PROFILE_ID);
	    profile_id = DHD_RNR_FLOWRING_DEFAULT_PROFILE_ID;
	}

	if (profile_id >= DHD_RNR_FLOWRING_PROFILES) {
	    RLOG("profile id %d key is not valid, using built-in profile id %d",
	        profile_id, radio_idx);
	    profile_id = DHD_RNR_FLOWRING_DEFAULT_PROFILE_ID;
	}

bkupq:
	profile = &dhd_rnr_profiles[profile_id];
	dhd_hlp->rnr_en_feat.bkupq = DHD_RNR_DEF_BKUPQ;
	phy_ring_size = 0;

#if defined(RNR_DHD_HLPR_BKUPQUEUE)
	{
        /* If nvram profile set dhd?_rnr_flowring_physize, then that overrides the defaults */
        /* Exit from this section with phy_ring_size set to 0 means use the defaults */
	    int size;
	    length = dhd_runner_key_get(radio_idx, DHD_RNR_KEY_PHY_RING_SIZE, buff, sizeof(buff));
	    if (length != 0) {

	        sscanf(buff, "%d", &size);
	        if ((size > 0) && (size <= DHD_RNR_FLOWRING_MAX_SIZE)) {
	            /* valid physical ring size setting in nvram */
	            /* Enable backup queues */
	            dhd_hlp->rnr_en_feat.bkupq = 1;
	            phy_ring_size = size;
	        } else {
	            dhd_hlp->rnr_en_feat.bkupq = 0;
	        }
	    }
	}
#endif /* RNR_DHD_HLPR_BKUPQUEUE */

	for (ac = wme_ac_bk; ac <= wme_ac_max; ac++) {
		if (DHD_RNR_BKUPQ(dhd_hlp)) {
            int local_ring_phy_size = phy_ring_size ? phy_ring_size : dhd_flowring_phy_ring_size_defaults[ac];
            if (local_ring_phy_size <= profile->items[ac]) {
                flowmgr->phy_items[ac] = local_ring_phy_size;
            }
            else {
                flowmgr->phy_items[ac] = profile->items[ac];
                RLOG("%s: Adjust phy_ring_size for %s to %d\r\n", __FUNCTION__, dhd_wme_ac_str[ac], profile->items[ac]);
            }
		}
		else {
			flowmgr->phy_items[ac] = profile->items[ac];
		}
	}

	if (dhd_hlp->rnr_sup_feat.npm) {
	    RLOG("%s: N+M profile = %1d %02d:%04d/%04d %02d:%04d/%04d"
	        " %02d:%04d/%04d %02d:%04d/%04d %02d:%04d/%04d",
	        __FUNCTION__, profile_id,
	        profile->weight[0], profile->items[0], flowmgr->phy_items[0],
	        profile->weight[1], profile->items[1], flowmgr->phy_items[1],
	        profile->weight[2], profile->items[2], flowmgr->phy_items[2],
	        profile->weight[3], profile->items[3], flowmgr->phy_items[3],
	        profile->weight[4], profile->items[4], flowmgr->phy_items[4]);
	} else {
	    RLOG("%s: profile = %1d %04d/%04d %04d/%04d"
	        " %04d/%04d %04d/%04d %04d/%04d",
	        __FUNCTION__, profile_id,
	        profile->items[0], flowmgr->phy_items[0],
	        profile->items[1], flowmgr->phy_items[1],
	        profile->items[2], flowmgr->phy_items[2],
	        profile->items[3], flowmgr->phy_items[3],
	        profile->items[4], flowmgr->phy_items[4]);
	}

	return &dhd_rnr_profiles[profile_id];
}

/*
 * +----------------------------------------------------------------------------
 *           Section: Flow ring Policy management
 * +----------------------------------------------------------------------------
 */

/**
 * Display Policy Information
 *
 * if present,
 * parse and data fill the global table: dhd_rnr_profiles[radio_idx]
 *
*/
static void
dhd_runner_get_policy_str(dhd_flowring_policy_t *policy, char    *buf,
	int len)
{
	int index, ac;

	/* parse and update the user profile, if present */
	switch (policy->id) {

	    case dhd_rnr_policy_global:
	        snprintf(buf, len, "%d (%s)", policy->all_hw,
	            (policy->all_hw) ? "HW" : "SW");
	        break;

	    case dhd_rnr_policy_intf:
	        snprintf(buf, len, "%d", policy->max_intf);
	        break;

	    case dhd_rnr_policy_clients:
	        snprintf(buf, len, "%d", policy->max_sta);
	        break;

	    case dhd_rnr_policy_aclist:
	        for (ac = wme_ac_bk; ac <= wme_ac_max; ac++) {
	            snprintf(buf, (len-(ac*9)), "%s:%s ", dhd_wme_ac_str[ac],
	                (policy->aclist_hw[ac]) ? "HW" : "SW");
	            buf += 9;
	        }
	        break;

	    case dhd_rnr_policy_maclist:
	        for (index = 0; index < DHD_RNR_POLICY_MAX_MACLIST; index++)
	        {
	            if (policy->mac_addr[index][6])
	            {
	                snprintf(buf, (len-(index*18)),
	                    "%02x:%02x:%02x:%02x:%02x:%02x ",
	                    policy->mac_addr[index][0],
	                    policy->mac_addr[index][1],
	                    policy->mac_addr[index][2],
	                    policy->mac_addr[index][3],
	                    policy->mac_addr[index][4],
	                    policy->mac_addr[index][5]);
	                buf += 18;
	            }
	            else
	                break;
	        }
	        break;

	    case dhd_rnr_policy_dllac:
	        snprintf(buf, len, "%d (%s)", policy->d11ac_hw,
	            (policy->d11ac_hw) ? "HW" : "SW");
	        break;

	    default:
	        DHD_ERROR(("Unexpected flowring policy <%d>\r\n", policy->id));
	        break;
	}

	return;
}

/**
 * Get index of the matched string from string list
 *
 *
*/
static INLINE int dhd_runner_str_to_idx(char *str, const char **list, int max)
{
	int idx = 0;
	for (idx = 0; idx < max; idx++) {
	    if (strncmp(str, list[idx], strlen(list[idx])) == 0)
	        break;
	}

	return idx;
}

/**
 * Read the policy information from the persistent storage area
 *
 * Read nvram variable radio%d_policy,
 * if present and valid, override parameter policy_id
 *
 * if present,
 * parse and data fill the global table: dhd_rnr_profiles[radio_idx]
 *
*/
static dhd_flowring_policy_t*
dhd_runner_policy_init(struct dhd_runner_hlp *dhd_hlp)
{
	char buff[DHD_RNR_KEY_POLICY_STR_LEN];
	int  length = 0;
	int  policy_id;
	char *token, *policy_str;
	int  value;
	int mac_addr[ETHER_ADDR_LEN];
	int ac;
	dhd_flowring_policy_t* policy;

	policy = &dhd_rnr_policies[dhd_hlp->dhd->unit];

	if (!dhd_hlp->rnr_sup_feat.npm) {
	    /* N+M disabled, policy has no effect */
	    return policy;
	}

	/* Fetch the Policy information of the radio, if present */
	memset(buff, 0, sizeof(buff));
	length = dhd_runner_key_get(dhd_hlp->dhd->unit, DHD_RNR_KEY_POLICY, buff,
	    sizeof(buff));

	if (length == 0) {
	    /* Policy not present, return default radio policy */
	    goto done;
	}

	/* parse policy_id from profile string */
	policy_str = buff;
	token = bcmstrtok(&policy_str, " ", NULL);
	sscanf(buff, "%d", &policy_id);

	if (policy_id >= dhd_rnr_max_policies) {
	    RLOG("policy id %d is not valid, using built-in policy id %d",
	        policy_id, dhd_rnr_policies[dhd_hlp->dhd->unit].id);

	    goto done;
	}

	policy->id = policy_id;

	/* parse and update the user profile, if present */
	switch (policy_id) {

	    case dhd_rnr_policy_global:
	        token = bcmstrtok(&policy_str, " ", NULL);
	        if (token) sscanf(token, "%d", &value);
	        else value = 1;
	        if (value) policy->all_hw = TRUE;
	        else policy->all_hw = FALSE;

	        break;

	    case dhd_rnr_policy_intf:
	        token = bcmstrtok(&policy_str, " ", NULL);
	        if (token) sscanf(token, "%d", &value);
	        else value = 0;
	        value = LIMIT_TO_MAX(value, 15);
	        policy->max_intf = value;

	        break;

	    case dhd_rnr_policy_clients:
	        token = bcmstrtok(&policy_str, " ", NULL);
	        if (token) sscanf(token, "%d", &value);
	        else value = DHD_RNR_MAX_STATIONS;
	        value = LIMIT_TO_MAX(value, DHD_RNR_MAX_STATIONS);
	        policy->max_sta = value;

	        break;

	    case dhd_rnr_policy_aclist:
	        for (ac = wme_ac_bk; ac <= wme_ac_max; ac++)
	            policy->aclist_hw[ac] = FALSE;

	        token = bcmstrtok(&policy_str, " ", NULL);
	        while (token) {
	            sscanf(token, "%d:%d", &ac, &value);
	            if (ac <= wme_ac_max)
	                policy->aclist_hw[ac] = (value) ? TRUE : FALSE;
	            token = bcmstrtok(&policy_str, " ", NULL);
	        }

	        break;

	    case dhd_rnr_policy_maclist:
	        for (value = 0; value < DHD_RNR_POLICY_MAX_MACLIST; value++)
	            policy->mac_addr[value][6] = 0;

	        value = 0;
	        token = bcmstrtok(&policy_str, " ", NULL);
	        while (token) {
	            sscanf(token, "%02x:%02x:%02x:%02x:%02x:%02x",
	                &mac_addr[0], &mac_addr[1], &mac_addr[2],
	                &mac_addr[3], &mac_addr[4], &mac_addr[5]);
	            policy->mac_addr[value][0] = mac_addr[0];
	            policy->mac_addr[value][1] = mac_addr[1];
	            policy->mac_addr[value][2] = mac_addr[2];
	            policy->mac_addr[value][3] = mac_addr[3];
	            policy->mac_addr[value][4] = mac_addr[4];
	            policy->mac_addr[value][5] = mac_addr[5];
	            policy->mac_addr[value][6] = 1;
	            token = bcmstrtok(&policy_str, " ", NULL);
	            value++;
	        }

	        break;

	    case dhd_rnr_policy_dllac:
	        token = bcmstrtok(&policy_str, " ", NULL);
	        sscanf(token, "%d", &value);
	        if (value) policy->d11ac_hw = TRUE;
	        else policy->d11ac_hw = FALSE;

	        break;

	    default:
	        DHD_ERROR(("Unexpected flowring policy <%d>\r\n", policy->id));
	        policy = NULL;
	        return policy;
	}

done:
	buff[0] = '\0';
	dhd_runner_get_policy_str(policy, buff, sizeof(buff));
	RLOG("%s:  N+M Policy = %d %s", __FUNCTION__, policy->id, buff);

	return policy;
}


/*
 * +----------------------------------------------------------------------------
 *           Section: DHD Rx Offload Setting
 * +----------------------------------------------------------------------------
 */

/**
 * Initialize Rx Offload setting from PSP
 *
 * if not present, return the default value
 *
 *
*/
static int
dhd_runner_rxoffl_init(struct dhd_runner_hlp *dhd_hlp,
	dhd_runner_ring_cfg_t *ring_cfg)
{
	char buff[DHD_RNR_KEY_RXOFFL_STR_LEN];
	int  length = 0;
	int offload, size;

	/* Set the default values */
	ring_cfg->offload = DHD_RNR_DEF_RX_OFFLOAD;
	ring_cfg->max_items = D2HRING_RXCMPLT_MAX_ITEM;

	/* Fetch the Rx Ring Configuration information of the radio, if present */
	memset(buff, 0, sizeof(buff));
	length = dhd_runner_key_get(dhd_hlp->dhd->unit, DHD_RNR_KEY_RXOFFL, buff,
	    sizeof(buff));

	if (length == 0) {
	    /* Rx Offload setting is not present, return default */
	    goto done;
	}

	sscanf(buff, "%d:%d", &offload, &size);

	if (offload <= 0)
	    ring_cfg->offload = 0;
	else
	    ring_cfg->offload = 1;

	if ((size > 0) && (size <= DHD_RNR_FLOWRING_MAX_SIZE)) {
	    ring_cfg->max_items = size;
	}

done:
	if (!DHD_RNR_IS_RX_OFFL_SUPPORTED(dhd_hlp)) {
	    DHD_INFO(("dhd%d_rnr force disable rx offlaod due to no runner support\r\n",
	        dhd_hlp->dhd->unit));
	    ring_cfg->offload = 0;
	}

	return 0;
}

/*
 * +----------------------------------------------------------------------------
 *           Section: DHD Tx Offload Setting
 * +----------------------------------------------------------------------------
 */

/**
 * Initialize Tx Offload setting from reserved memory and PSP
 * if not present, return the default value
 * Order:
 *  - rc    (for HND only)
 *  - CFE   (override option for HND only)
 *  - NVRAM (for disabling only)
 *
*/
static int
dhd_runner_txoffl_init(struct dhd_runner_hlp *dhd_hlp,
	dhd_runner_ring_cfg_t *ring_cfg)
{
	char buff[DHD_RNR_KEY_TXOFFL_STR_LEN];
	int  length = 0;
	int offload = 1, size;
	char dhd_name[8];
	dhd_pub_t *dhd = dhd_hlp->dhd;
	dhd_runner_flowmgr_t *flowmgr = &dhd_hlp->flowmgr;

	/*
	 * Check if we have any reserved memory allocated for this radio
	 * If no reserved memory is allocated, we can route all TXConf messages
	 * to DHD only instead of Runner
	 */
	snprintf(dhd_name, sizeof(dhd_name), "dhd%d", dhd->unit);
	dhd_name[4] = '\0';

	/* Initialize txsts ring offload if reserved memory is available */
	if (BcmMemReserveGetByName(dhd_name,
	    (void **)&flowmgr->hw_mem_virt_base_addr, &flowmgr->hw_mem_phys_base_addr, &flowmgr->hw_mem_size) == 0) {
	    flowmgr->hw_mem_addr = flowmgr->hw_mem_virt_base_addr;
	    ring_cfg->offload = DHD_RNR_TXSTS_CFG_OFFL_MASK;
	} else {
	    DHD_INFO(("BcmMemReserveGetByName returned no memory\n"));
	    goto disable;
	}

	/* Fetch the Tx Offload setting from nvram, if present */
	memset(buff, 0, sizeof(buff));
	length = dhd_runner_key_get(dhd->unit, DHD_RNR_KEY_TXOFFL, buff,
	    sizeof(buff));

	if (length != 0) {
	    /* Tx Offload setting is present, get the current setting */
	    sscanf(buff, "%d:%d", &offload, &size);

	    if ((size > 0) && (size <= DHD_RNR_FLOWRING_MAX_SIZE)) {
	        ring_cfg->max_items = size;
	    }
	}

	/* If offload is enabled, nvram setting can be used to force disable offload */
	if ((DHD_RNR_IS_TX_OFFL_SUPPORTED(dhd_hlp)) && (offload > TX_DOR_MODE_M)) {
	    /*
	     * Offload    0   - Disable offload, N+M
	     *            1   - Enable offload, default N+M
	     *            2   - Enable offload, disable N+M
	     *            3   - Enable offload, enable N+M
	     *            *   - Same as 1
	     */
	    if (offload == TX_DOR_MODE_NPM) {
	        dhd_hlp->rnr_en_feat.npm = 1;
	    } else if (offload == TX_DOR_MODE_N_ONLY) {
	        dhd_hlp->rnr_en_feat.npm = 0;
	    } else {
	        /* TX_DOR_MODE_N_DEF */
	        dhd_hlp->rnr_en_feat.npm = DOR_NPM_DEFAULT;
	    }
	    goto done;
	}

disable:
	DHD_INFO(("dhd%d_rnr disable tx offlaod \r\n", dhd_hlp->dhd->unit));
	ring_cfg->offload = 0;
	flowmgr->hw_mem_addr = (void*)NULL;
	flowmgr->hw_mem_size = 0;
	dhd_hlp->rnr_en_feat.txoffl = 0;
	dhd_hlp->rnr_en_feat.npm = 0;

done:
	return 0;
}


/*
 * +----------------------------------------------------------------------------
 *           Section: DHD Flow ring IOVAR processing
 * +----------------------------------------------------------------------------
 */

/**
 * Fills the Iovar string with the list of available profiles supported by the DHD Runner layer
 *
 * Also indicates the current profile of the radio
 */
static int
dhd_runner_iovar_get_profile(dhd_runner_hlp_t *dhd_hlp, char *buf,
	int buflen)
{
	int id;
	dhd_wme_ac_t ac;
	struct bcmstrbuf b;
	struct bcmstrbuf *strbuf = &b;
	dhd_flowring_profile_t* profile;
	dhd_pub_t *dhdp;

	DHD_TRACE(("%s: dhd_hlp=<0x%p>, buff=<0x%p>, bufflen=<%d>\r\n",
	    __FUNCTION__, dhd_hlp, buf, buflen));

	dhdp = dhd_hlp->dhd;

	bcm_binit(strbuf, buf, buflen);

	bcm_bprintf(strbuf, "[id] [ ac_bk ] [ ac_be ] [ ac_vi ] [ ac_vo ] [ bc_mc ]\n");
	for (id = 0; id < DHD_RNR_FLOWRING_PROFILES; id++) {
	    profile = &dhd_rnr_profiles[id];
	    if ((!dhd_hlp->rnr_sup_feat.npm) && profile->npm) {
	        continue;
	    }

	    if (profile == dhd_hlp->flowmgr.profile)
	        bcm_bprintf(strbuf, " *%d ", profile->id);
	    else
	        bcm_bprintf(strbuf, "  %d ", profile->id);

	    for (ac = wme_ac_bk; ac <= wme_ac_max; ac++) {
	        if (dhd_hlp->rnr_sup_feat.npm) {
	            bcm_bprintf(strbuf, "  %02d:%04d ", profile->weight[ac],
	                profile->items[ac]);
	        } else {
	            bcm_bprintf(strbuf, "    %04d  ", profile->items[ac]);
	        }
	    }

	    bcm_bprintf(strbuf, "\n");
	}

	return (!strbuf->size ? BCME_BUFTOOSHORT : 0);
}

/**
 * Parse and set the profile information
 *
 * Parse the flowring_profile IOVAR buffer
 * Set the profile information into persistent storage area
 *
 */
static int
dhd_runner_iovar_set_profile(dhd_runner_hlp_t *dhd_hlp, char *buff,
	int bufflen)
{
	char *token, *profile_str;
	dhd_wme_ac_t ac;
	dhd_flowring_profile_t profile;
	char pspbuf[DHD_RNR_IOVAR_BUFF_SIZE];
	int radio_idx;

	DHD_TRACE(("%s: dhd_hlp=<0x%p>, buff=<%s>, bufflen=<%d>\r\n",
	    __FUNCTION__, dhd_hlp, buff, bufflen));

#if defined(DHD_RNR_NVRAM_KEYS)
	DHD_ERROR(("    Setting through DHD is deprecated\r\n"));
	DHD_ERROR(("    Insetead use nvram set, commit and reboot\r\n"));

	return BCME_UNSUPPORTED;
#endif /* DHD_RNR_NVRAM_KEYS */

	if ((bufflen == 0) || (bufflen > DHD_RNR_IOVAR_BUFF_SIZE)) {
	    DHD_ERROR(("No profile id specified\r\n"));
	    return BCME_BADARG;
	}

	/* Get the current radio profile from non-volatile memory */
	radio_idx = dhd_hlp->dhd->unit;
	memset(pspbuf, 0, DHD_RNR_KEY_PPROFILE_STR_LEN);
	dhd_runner_key_get(radio_idx, DHD_RNR_KEY_PROFILE,
	    pspbuf, sizeof(pspbuf));

	/* parse profile keys from profile string */
	profile_str = buff;
	token = bcmstrtok(&profile_str, " ", NULL);
	if ((!token) || (strncmp(token, "-id", strlen("-id")) != 0)) {
	    DHD_ERROR(("No profile id specified\r\n"));
	    return BCME_BADARG;
	}

	/* Get the profile id */
	token = bcmstrtok(&profile_str, " ", NULL);
	if (!token) {
	    DHD_ERROR(("No profile id specified\r\n"));
	    return BCME_BADARG;
	}
	sscanf(token, "%d", &profile.id);

	token = bcmstrtok(&profile_str, " ", NULL);

	if (!token) {
	    /* Only profile id is specified */
	    if (profile.id >= DHD_RNR_FLOWRING_PROFILES) {
	        DHD_ERROR(("Invalid Profile id [%d]\"\r\n", profile.id));
	        return BCME_BADARG;
	    }

	    /* Keep the user profile settings, just update the profile id */
	    pspbuf[0] = '0' + profile.id;

	    /* Update the profile information of the radio */
	    if (dhd_runner_key_set(radio_idx, DHD_RNR_KEY_PROFILE,
	        pspbuf, DHD_RNR_KEY_PPROFILE_STR_LEN) < 0)
	        return BCME_ERROR;

	    dhd_hlp->flowmgr.profile = &dhd_rnr_profiles[profile.id];
	} else {
	    char key[32];

	    /* profile id with profile settings specified */
	    if (profile.id >= DHD_RNR_FLOWRING_USER_PROFILES) {
	        DHD_ERROR(("Can not change Built-in Profile id [%d] settings \"%s\"\r\n",
	            profile.id, token));
	        return BCME_BADARG;
	    }

	    /* Get the profile settings */
	    radio_idx = profile.id;
	    memcpy(&profile, &dhd_rnr_profiles[radio_idx], sizeof(profile));

	    do {
	        int idx;
	        key[0] = 0;

	        if ((token[0] == '-') && (strlen(token) >= 5))
	            sscanf(token, "-%s", key);
	        idx = dhd_runner_str_to_idx(key, dhd_wme_ac_str, (wme_ac_bcmc+1));
	        if (idx > wme_ac_bcmc) {
	            DHD_ERROR(("Invalid AC name\r\n"));
	            return BCME_BADARG;
	        }

	        token = bcmstrtok(&profile_str, " ", NULL);
	        if (token) {
	            sscanf(token, "%d:%d", &profile.weight[idx], &profile.items[idx]);
	            token = bcmstrtok(&profile_str, " ", NULL);
	        }
	    } while (token);

	    /* Sanity check the profile setting values */
	    for (ac = wme_ac_bk; ac <= wme_ac_max; ac++) {
	        if ((profile.items[ac] > DHD_RNR_FLOWRING_MAX_SIZE) ||
	            (profile.weight[ac] < -1) ||
	            (profile.weight[ac] > profile.items[ac])) {
	            DHD_ERROR(("Settings out of range\r\n"));
	                return BCME_BADARG;
	        }
	    }
	    snprintf(pspbuf, sizeof(pspbuf),
	        "%d %d:%d %d:%d %d:%d %d:%d %d:%d",
	        (pspbuf[0] ? (pspbuf[0] - '0') : radio_idx),
	        profile.weight[wme_ac_bk], profile.items[wme_ac_bk],
	        profile.weight[wme_ac_be], profile.items[wme_ac_be],
	        profile.weight[wme_ac_vi], profile.items[wme_ac_vi],
	        profile.weight[wme_ac_vo], profile.items[wme_ac_vo],
	        profile.weight[wme_ac_bcmc], profile.items[wme_ac_bcmc]);

	    /* Update the profile information of the radio */
	    if (dhd_runner_key_set(radio_idx, DHD_RNR_KEY_PROFILE,
	        pspbuf, DHD_RNR_KEY_PPROFILE_STR_LEN) < 0)
	        return BCME_ERROR;

	    memcpy(&dhd_rnr_profiles[profile.id], &profile,
	        sizeof(dhd_flowring_profile_t));
	}

	return BCME_OK;
}

/**
 * Fills the Iovar string with the list of available policies supported by the DHD Runner layer
 *
 * Also indicates the current policy and policy parameters of the radio
 */
static int
dhd_runner_iovar_get_policy(dhd_runner_hlp_t *dhd_hlp, char *buf,
	int buflen)
{
	int id;
	struct bcmstrbuf b;
	struct bcmstrbuf *strbuf = &b;
	dhd_flowring_policy_t* policy;
	dhd_pub_t *dhdp;
	char policy_str[DHD_RNR_IOVAR_BUFF_SIZE];

	DHD_TRACE(("%s: dhd_hlp=<0x%p>, buff=<0x%p>, bufflen=<%d>\r\n",
	    __FUNCTION__, dhd_hlp, buf, buflen));

	if (!dhd_hlp->rnr_sup_feat.npm) {
	    DHD_ERROR(("    N+M Not Supported\r\n"));
	    return BCME_UNSUPPORTED;
	}

	dhdp = dhd_hlp->dhd;

	bcm_binit(strbuf, buf, buflen);

	bcm_bprintf(strbuf, "[ Name  ] [id] [Policy]\n");
	policy = &dhd_rnr_policies[dhdp->unit];

	for (id = dhd_rnr_policy_global; id < dhd_rnr_max_policies; id++) {
	    bcm_bprintf(strbuf, " %7s ", dhd_flowring_policy_id_str[id]);
	    if (id == policy->id) {
	        policy_str[0] = '\0';
	        dhd_runner_get_policy_str(policy, policy_str, sizeof(policy_str));
	        bcm_bprintf(strbuf, "  *%d   %s\n", id, policy_str);
	    }
	    else
	        bcm_bprintf(strbuf, "   %d \n", id);
	}

	return (!strbuf->size ? BCME_BUFTOOSHORT : 0);
}

/**
 * Parse and set the policy information
 *
 * Parse the flowring_policy IOVAR buffer
 * Set the policy information into persistent storage area
 * Update current radio policy to reflect the new changes
 *
 */
static int
dhd_runner_iovar_set_policy(dhd_runner_hlp_t *dhd_hlp, char *buff,
	int bufflen)
{
	char pspbuf[DHD_RNR_KEY_POLICY_STR_LEN];
	char *token, *policy_str;
	int  value;
	int mac_addr[ETHER_ADDR_LEN];
	int ac;
	dhd_flowring_policy_t policy;
	char key[32];

	DHD_TRACE(("%s: dhd_hlp=<0x%p>, buff=<%s>, bufflen=<%d>\r\n",
	    __FUNCTION__, dhd_hlp, buff, bufflen));

#if defined(DHD_RNR_NVRAM_KEYS)
	DHD_ERROR(("    Setting through DHD is deprecated\r\n"));
	DHD_ERROR(("    Insetead use nvram set, commit and reboot\r\n"));

	return BCME_UNSUPPORTED;
#endif /* DHD_RNR_NVRAM_KEYS */

	if ((bufflen == 0) || (bufflen > DHD_RNR_IOVAR_BUFF_SIZE)) {
	    DHD_ERROR(("No policy name or buffer too large\r\n"));
	    return BCME_BADARG;
	}

	/* Fill the structure with default current policy */
	memcpy(&policy, dhd_hlp->flowmgr.policy, sizeof(dhd_flowring_policy_t));
	memset(pspbuf, 0, sizeof(pspbuf));

	policy_str = buff;
	token = bcmstrtok(&policy_str, " ", NULL);

	if (!token) {
	    DHD_ERROR(("No policy name specified\r\n"));
	    return BCME_BADARG;
	}

	key[0] = 0;
	if ((token[0] == '-') && (strlen(token) >= 5))
	    sscanf(token, "-%s", key);

	policy.id = dhd_runner_str_to_idx(key, dhd_flowring_policy_id_str,
	    dhd_rnr_max_policies);

	if (policy.id >= dhd_rnr_max_policies) {
	    DHD_ERROR(("No valid policy name\r\n"));
	    return BCME_BADARG;
	}
	token = bcmstrtok(&policy_str, " ", NULL);

	if (!token) {
	    DHD_ERROR(("No policy parameters specified\r\n"));
	    return BCME_BADARG;
	}
	switch (policy.id) {
	    case dhd_rnr_policy_global:
	        sscanf(token, "%d", &value);

	        if ((value < 0) || (value > 1)) {
	            DHD_ERROR(("global policy value [%d] is not valid\r\n", value));
	            return BCME_BADARG;
	        }
	        policy.all_hw = value ? TRUE : FALSE;
	        snprintf(pspbuf, sizeof(pspbuf), "%d %d", policy.id,
	            policy.all_hw);
	        break;

	    case dhd_rnr_policy_intf:
	        sscanf(token, "%d", &value);

	        if ((value < 0) || (value >= DHD_RNR_MAX_BSS)) {
	            DHD_ERROR(("intfidx policy value [%d] is not valid\r\n",
	                value));
	            return BCME_BADARG;
	        }
	        policy.max_intf = value;
	        snprintf(pspbuf, sizeof(pspbuf), "%d %d", policy.id,
	            policy.max_intf);

	        break;

	    case dhd_rnr_policy_clients:
	        sscanf(token, "%d", &value);
	        if ((value < 0) || (value >= DHD_RNR_MAX_STATIONS)) {
	            DHD_ERROR(("clients policy value [%d] is not valid\r\n", value));
	            return BCME_BADARG;
	        }
	        policy.max_sta = value;
	        snprintf(pspbuf, sizeof(pspbuf), "%d %d", policy.id,
	            policy.max_sta);

	        break;

	    case dhd_rnr_policy_aclist:
	        for (ac = wme_ac_bk; ac <= wme_ac_max; ac++)
	            policy.aclist_hw[ac] = FALSE;

	        while (token) {
	            int idx;

	            if ((strlen(token) > 5) && (token[5] == ':')) {
	                token[5] = 0;
	                sscanf(token+6, "%d", &value);
	                idx = dhd_runner_str_to_idx(token, dhd_wme_ac_str, (wme_ac_bcmc+1));
	                if ((idx > wme_ac_max) || (value < 0) || (value > 1)) {
	                    DHD_ERROR(("aclist policy value [%s:%d] is not valid\r\n",
	                        key, value));
	                    return BCME_BADARG;
	                }
	                policy.aclist_hw[idx] = (value) ? TRUE : FALSE;
	            }
	            token = bcmstrtok(&policy_str, " ", NULL);
	        }
	        snprintf(pspbuf, sizeof(pspbuf), "%d %d:%d %d:%d %d:%d %d:%d %d:%d",
	            policy.id, wme_ac_bk, policy.aclist_hw[wme_ac_bk],
	            wme_ac_be, policy.aclist_hw[wme_ac_be],
	            wme_ac_vi, policy.aclist_hw[wme_ac_vi],
	            wme_ac_vo, policy.aclist_hw[wme_ac_vo],
	            wme_ac_bcmc, policy.aclist_hw[wme_ac_bcmc]);

	        break;

	    case dhd_rnr_policy_maclist:
	        for (value = 0; value < DHD_RNR_POLICY_MAX_MACLIST; value++)
	            policy.mac_addr[value][6] = 0;

	        value = 0;
	        snprintf(pspbuf, sizeof(pspbuf), "%d", policy.id);
	        while (token && (value < DHD_RNR_POLICY_MAX_MACLIST)) {
	            sscanf(token, "%02x:%02x:%02x:%02x:%02x:%02x",
	                &mac_addr[0], &mac_addr[1], &mac_addr[2],
	                &mac_addr[3], &mac_addr[4], &mac_addr[5]);
	            policy.mac_addr[value][0] = mac_addr[0];
	            policy.mac_addr[value][1] = mac_addr[1];
	            policy.mac_addr[value][2] = mac_addr[2];
	            policy.mac_addr[value][3] = mac_addr[3];
	            policy.mac_addr[value][4] = mac_addr[4];
	            policy.mac_addr[value][5] = mac_addr[5];
	            policy.mac_addr[value][6] = 1;            /* mac address valid */
	            strncat(pspbuf, " ", sizeof(pspbuf) - strlen(pspbuf));
	            strncat(pspbuf, token, sizeof(pspbuf) - strlen(pspbuf));
	            token = bcmstrtok(&policy_str, " ", NULL);
	            value++;
	        }
	        break;

	    case dhd_rnr_policy_dllac:
	        sscanf(token, "%d", &value);
	        if ((value < 0) || (value > 1)) {
	            DHD_ERROR(("d11ac policy value [%d] is not valid\r\n", value));
	            return BCME_BADARG;
	        }
	        policy.d11ac_hw = value ? TRUE : FALSE;
	        snprintf(pspbuf, sizeof(pspbuf), "%d %d", policy.id,
	            policy.d11ac_hw);

	        break;
	}


	/* set the policy information of the radio to PSP */
	if (dhd_runner_key_set(dhd_hlp->dhd->unit, DHD_RNR_KEY_POLICY,
	    pspbuf, DHD_RNR_KEY_POLICY_STR_LEN) < 0) {
	    DHD_ERROR(("policy id %d for radio set failed \r\n",
	        dhd_hlp->dhd->unit));
	    return BCME_ERROR;
	}

	/*
	 * Do we need to update the local policy ?
	 *
	 * These settings can not be used until a driver unload or system reboot
	 * is performed
	 */
	memcpy(&dhd_rnr_policies[dhd_hlp->dhd->unit], &policy,
	    sizeof(dhd_flowring_policy_t));

	return BCME_OK;
}

/**
 * Fills the Iovar string with the Rx Offload setting
 *
 */
static int
dhd_runner_iovar_get_rxoffl(dhd_runner_hlp_t *dhd_hlp, char *buf,
	int buflen)
{
	struct bcmstrbuf b;
	struct bcmstrbuf *strbuf = &b;
	dhd_runner_ring_cfg_t ring_cfg;

	DHD_TRACE(("%s: dhd_hlp=<0x%p>, buff=<0x%p>, bufflen=<%d>\r\n",
	    __FUNCTION__, dhd_hlp, buf, buflen));

	if (dhd_runner_rxoffl_init(dhd_hlp, &ring_cfg) != 0) {
	    DHD_ERROR(("dhd%d: Failed to get Rx Offload setting\n",
	        dhd_hlp->dhd->unit));
	    return BCME_ERROR;
	}

	bcm_binit(strbuf, buf, buflen);

	bcm_bprintf(strbuf, "%d\r\n", ring_cfg.offload);

	return (!strbuf->size ? BCME_BUFTOOSHORT : 0);
}

/**
 * Parse and set the Rx Offload setting
 *
 */
static int
dhd_runner_iovar_set_rxoffl(dhd_runner_hlp_t *dhd_hlp, char *buff,
	int buflen)
{
	char pspbuf[DHD_RNR_KEY_RXOFFL_STR_LEN];
	int offload;
	int size = D2HRING_RXCMPLT_MAX_ITEM;

	DHD_TRACE(("%s: dhd_hlp=<0x%p>, buff=<%s>, bufflen=<%d>\r\n",
	    __FUNCTION__, dhd_hlp, buff, buflen));

#if defined(DHD_RNR_NVRAM_KEYS)
	DHD_ERROR(("    Setting through DHD is deprecated\r\n"));
	DHD_ERROR(("    Insetead use nvram set, commit and reboot\r\n"));

	return BCME_UNSUPPORTED;
#endif /* DHD_RNR_NVRAM_KEYS */

	if ((buflen == 0) || (buflen > DHD_RNR_IOVAR_BUFF_SIZE)) {
	    DHD_ERROR(("No setting specified\r\n"));
	    return BCME_BADARG;
	}

	/* Fill the structure with default current rxoffl */
	memset(pspbuf, 0, sizeof(pspbuf));

	/* Get the user Rx Offload setting */
	sscanf(buff, "%d", &offload);

	offload = (offload <= 0) ? 0 : 1;

	if ((offload < 0) || (offload > 1)) {
	    DHD_ERROR(("Rx Offload setting %d should be 0 or 1\n", offload));
	    return BCME_BADARG;
	}

	snprintf(pspbuf, sizeof(pspbuf), "%d:%d", offload, size);

	/* set the rxoffload setting of the radio to PSP */
	if (dhd_runner_key_set(dhd_hlp->dhd->unit, DHD_RNR_KEY_RXOFFL,
	    pspbuf, DHD_RNR_KEY_RXOFFL_STR_LEN) < 0) {
	    DHD_ERROR(("Rx Offload setting %d for radio %d failed \r\n",
	        offload, dhd_hlp->dhd->unit));
	    return BCME_ERROR;
	}

	return BCME_OK;
}


/**
 * Dump DHD runner status, configuration and counters
 *
 * Display DHD runner offload status
 * Display flowring policy and profile configuration
 * parse and Display number of hw/sw flowrings per wme_ac
 * Display R2H request and H2R notification counters
 *
 */
static int
dhd_runner_iovar_dump(dhd_runner_hlp_t *dhd_hlp, char *buff,
	int bufflen)
{
	struct bcmstrbuf *b = (struct bcmstrbuf *)buff;
	dhd_runner_flowmgr_t *flowmgr;
	enum dhd_wme_ac wme_ac;
	char policy_str[DHD_RNR_IOVAR_BUFF_SIZE];

	flowmgr = &dhd_hlp->flowmgr;

	/* Status */
	bcm_bprintf(b, "\nDHD Runner: \n");
	bcm_bprintf(b, "  Status     : %s %s %s %s %s %s %s %s %s %s\n",
	    RNR_DHD_HLPR_FEATURE_STS_STRING(dhd_hlp, txoffl),
	    RNR_DHD_HLPR_FEATURE_STS_STRING(dhd_hlp, rxoffl),
	    RNR_DHD_HLPR_FEATURE_STS_STRING(dhd_hlp, txcmpl2host),
	    RNR_DHD_HLPR_FEATURE_STS_STRING(dhd_hlp, dhdhdr),
	    RNR_DHD_HLPR_FEATURE_STS_STRING(dhd_hlp, lbraggr),
	    RNR_DHD_HLPR_FEATURE_STS_STRING(dhd_hlp, msgringformat),
	    RNR_DHD_HLPR_FEATURE_STS_STRING(dhd_hlp, bkupq),
	    RNR_DHD_HLPR_FEATURE_STS_STRING(dhd_hlp, hwawkup),
	    RNR_DHD_HLPR_FEATURE_STS_STRING(dhd_hlp, ffrd),
	    RNR_DHD_HLPR_FEATURE_STS_STRING(dhd_hlp, npm));
	bcm_bprintf(b, "             : cpuqdpc %d\n", dhd_hlp->proc_cpuq_in_dpc);

	if (dhd_hlp->rnr_sup_feat.npm) {
	    /* Profile */
	    bcm_bprintf(b, "  Profile    : prfl_id %d id_valu", flowmgr->profile->id);
	    for (wme_ac = wme_ac_bk; wme_ac <= wme_ac_max; wme_ac++)
	        bcm_bprintf(b, "  %02d:%04d/%04d ", flowmgr->profile->weight[wme_ac],
	            flowmgr->profile->items[wme_ac], flowmgr->phy_items[wme_ac]);
	    bcm_bprintf(b, "\n");

	    /* Policy */
	    policy_str[0] = '\0';
	    dhd_runner_get_policy_str(&dhd_rnr_policies[dhd_hlp->dhd->unit], policy_str,
	        sizeof(policy_str));
	    bcm_bprintf(b, "  Policy     : plcy_id %d id_valu %s\n",
	        dhd_rnr_policies[dhd_hlp->dhd->unit].id, policy_str);


	    /* TX flowrings */
	    bcm_bprintf(b, "  tx_flowring:", flowmgr->profile->id);
	    for (wme_ac = wme_ac_bk; wme_ac <= wme_ac_max; wme_ac++)
	        bcm_bprintf(b, " [%s] sw %d hw %d", dhd_wme_ac_str[wme_ac],
	            dhd_hlp->flowmgr.sw_ring_cnt[wme_ac], dhd_hlp->flowmgr.hw_ring_cnt[wme_ac]);
	    bcm_bprintf(b, "\n");
	} else {
	    /* Profile */
	    bcm_bprintf(b, "  Profile    : prfl_id %d id_valu", flowmgr->profile->id);
	    for (wme_ac = wme_ac_bk; wme_ac <= wme_ac_max; wme_ac++)
	        bcm_bprintf(b, "  %04d/%04d ", flowmgr->profile->items[wme_ac],
	            flowmgr->phy_items[wme_ac]);
	    bcm_bprintf(b, "\n");
	}


	/*
	  * local counters
	  *
	  * tx_post          Host notifies Runner to post tx packets
	  * rx_compl         Host notifies Runner to process D2H RxCompl
	  * tx_compl         Host notifies Runner to process D2H TxCompl
	  * rx_compl         Runner requests Host to receive a packet
	  * tx_compl         Runner requests Host to free a packet
	  * wake_dngl        Runner requests Host to wake dongle
	  */
	bcm_bprintf(b, "  h2r_notif  : tx_post %lu rx_cmpl %lu tx_cmpl %lu\n",
	    dhd_hlp->h2r_txpost_notif, dhd_hlp->h2r_rx_compl_notif,
	    dhd_hlp->h2r_tx_compl_notif);
	bcm_bprintf(b, "  r2h_req    : rx_cmpl %lu tx_cmpl %lu wk_dngl %lu\n",
	    dhd_hlp->r2h_rx_compl_req, dhd_hlp->r2h_tx_compl_req,
	    dhd_hlp->r2h_wake_dngl_req);

	return BCME_OK;
}

/**
 * Fills the Iovar structure with the interface stats queried
 * from Runner
 *
 */
static int
dhd_runner_iovar_get_rnr_stats(dhd_runner_hlp_t *dhd_hlp, char *buf,
	int buflen)
{
	dhd_helper_rnr_stats_t *pstats = (dhd_helper_rnr_stats_t*)buf;
	rdpa_wlan_mcast_ssid_stats_t rnr_stats = {0, 0};
	bdmf_number rnr_drop_cnt;
	bdmf_index idx;
	int rc;

	if ((buf == NULL) || (buflen < sizeof(dhd_helper_rnr_stats_t))) {
	    DHD_ERROR(("dhd%d_rnr iovar [%d] bad arguments", dhd_hlp->dhd->unit,
	        DHD_RNR_IOVAR_RNR_STATS));
	    DHD_ERROR(("buf 0x%p, buflen %d\r\n", buf, buflen));
	    return BCME_BADARG;
	}

	pstats->mcast_pkts = 0;
	pstats->mcast_bytes = 0;
	pstats->dropped_pkts = 0;

	if (!DHD_RNR_IS_OFFL_SUPPORTED(dhd_hlp)) {
	    /* Runner does not support this radio, return 0 stats */
	    return BCME_OK;
	}

	idx = RDPA_WLAN_MCAST_SSID_STATS_ENTRY_INDEX(dhd_hlp->dhd->unit,
	    pstats->ifidx);
	rc = rdpa_wlan_mcast_ssid_stats_get(dhd_hlp->dhd_mcast_obj, idx,
	    &rnr_stats);

	/* FIXME: why not BDMF_ERR_PARM ?? */
	if ((rc < 0) && (rc != BDMF_ERR_PARM) && (rc != BDMF_ERR_NOENT)) {
	    DHD_ERROR(("dhd%d_rnr:%d iovar [%d] Runner Error %d\r\n",
	        dhd_hlp->dhd->unit, __LINE__, DHD_RNR_IOVAR_RNR_STATS, rc));
	    return BCME_ERROR;
	}

	if (rc == BDMF_ERR_OK) {
	    pstats->mcast_pkts = rnr_stats.packets;
	    pstats->mcast_bytes = rnr_stats.bytes;
	}

	rc = rdpa_dhd_helper_ssid_tx_dropped_packets_get(dhd_hlp->dhd_helper_obj,
	    pstats->ifidx, &rnr_drop_cnt);

	if (rc < 0) {
	    DHD_ERROR(("dhd%d_rnr:%d iovar [%d] Runner Error %d\r\n",
	        dhd_hlp->dhd->unit, __LINE__, DHD_RNR_IOVAR_RNR_STATS, rc));
	    return BCME_ERROR;
	}

	pstats->dropped_pkts = rnr_drop_cnt;

	return BCME_OK;
}

/**
 * Fills the Iovar structure with the dhd-runner status information
 *
 */
static int
dhd_runner_iovar_get_rnr_status(dhd_runner_hlp_t *dhd_hlp, char *buf,
	int buflen)
{
	dhd_helper_status_t *pstatus = (dhd_helper_status_t*)buf;
	int ac = wme_ac_bk;

	if ((buf == NULL) || (buflen < sizeof(dhd_helper_status_t))) {
	    DHD_ERROR(("dhd%d_rnr iovar [%d] bad arguments", dhd_hlp->dhd->unit,
	        DHD_RNR_IOVAR_STATUS));
	    DHD_ERROR(("buf 0x%p, buflen %d\r\n", buf, buflen));
	    return BCME_BADARG;
	}

	/* Copy the supported and enabled features */
	memcpy(&pstatus->sup_features, &dhd_hlp->rnr_sup_feat, sizeof(dhd_helper_feat_t));
	memcpy(&pstatus->en_features, &dhd_hlp->rnr_en_feat, sizeof(dhd_helper_feat_t));

	pstatus->hw_flowrings = 0;
	pstatus->sw_flowrings = 0;
	while (ac <= wme_ac_max) {
	    pstatus->hw_flowrings += dhd_hlp->flowmgr.hw_ring_cnt[ac];
	    pstatus->sw_flowrings += dhd_hlp->flowmgr.sw_ring_cnt[ac];
	    ac++;
	}

	return BCME_OK;
}


/**
 * Process dhd_runner related IOVAR's
 */
int
dhd_runner_do_iovar(struct dhd_runner_hlp *dhd_hlp, dhd_runner_iovar_t iovar,
	bool set, char *buf, int buflen)
{
	if (iovar >= DHD_RNR_MAX_IOVARS) {
	    return BCME_UNSUPPORTED;
	}

	ASSERT(dhd_hlp != NULL);
	return dhd_rnr_iovar_table[iovar][set](dhd_hlp, buf, buflen);
}
