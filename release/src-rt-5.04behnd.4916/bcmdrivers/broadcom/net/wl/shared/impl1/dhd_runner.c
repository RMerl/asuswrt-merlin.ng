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

#if IS_ENABLED(CONFIG_BCM_DHD_ARCHER)
#include <archer_dhd_helper.h>
#else
#include <rdpa_api.h>
#include <rdpa_dhd_helper.h>
#include <rdpa_ag_dhd_helper.h>
#include <rdpa_mw_cpu_queue_ids.h>
#include <rdd.h>
#endif

#include <board.h>
#include <flash_common.h>
#include <bcm_rsvmem.h>
#include <bcm_pcie.h>
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
#include <wlan_shared_defs.h>

#if !IS_ENABLED(CONFIG_BCM_DHD_ARCHER)
#define DOL_HELPER "Runner"
#else
#define DOL_HELPER "Archer"
#endif

#if IS_ENABLED(CONFIG_BCM_SPDSVC)
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
 *           Section: Utility MACROS
 *
 * +---------------------------------------------------------------------
 */
#define B2MB(bytes)           (((bytes) + 0xFFFFF) / 0x100000)

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
#define DHD_RNR_FLRING_WME_AC(flags)     \
	(((flags) & DHD_RNR_FLRING_WME_AC_MASK) >> DHD_RNR_FLRING_WME_AC_SHIFT)

/*
 * +---------------------------------------------------------------------
 *           Section: WLAN impl specific missing define/macros
 * +---------------------------------------------------------------------
*/
#ifndef BCMDMA64OSL
#ifndef PHYSADDRTOULONG
#define PHYSADDRTOULONG(_pa, _ulong) \
	do { \
	    _ulong = (_pa); \
	} while (0)
#endif /* PHYSADDRTOULONG */

#ifndef ULONGTOPHYSADDR
#define ULONGTOPHYSADDR(_ulong, _pa) \
	do { \
	    (_pa) = (_ulong);		\
	} while (0)
#endif /* ULONGTOPHYSADDR */
#endif /* !BCMDMA64OSL */

#ifndef OSL_VIRT_TO_PHYSADDR
#define OSL_VIRT_TO_PHYSADDR(osh, va, pa) ULONGTOPHYSADDR((ulong)VIRT_TO_PHYS((va)), (pa))
#endif
/*
 * +---------------------------------------------------------------------
 *           Section: Definitions added to handle dynamic rings
 *           wl/impl - Provides actual values with dynamic rings
 *           wl/shared - For backward compatibility
 * +---------------------------------------------------------------------
*/

#ifndef MAX_TX_FLOWRINGS
#define MAX_TX_FLOWRINGS(dhdp, max_h2d_rings)	((max_h2d_rings) - BCMPCIE_H2D_COMMON_MSGRINGS)
#endif /* MAX_TX_FLOWRINGS */

#ifndef MAX_DYN_FLOWRINGS
#define MAX_DYN_FLOWRINGS(dhdp)	0
#endif /* MAX_DYN_FLOWRINGS */

#ifndef	DHD_IS_TX_FLOWRING
#define	DHD_IS_TX_FLOWRING(dhdp, ringid)	((ringid) >= BCMPCIE_COMMON_MSGRINGS)
#endif /* DHD_IS_TX_FLOWRING */

#ifndef DHD_IS_NON_TX_FLOWRING
#define	DHD_IS_NON_TX_FLOWRING(dhdp, ringid)	((ringid) < BCMPCIE_COMMON_MSGRINGS)
#endif /* DHD_IS_NON_TX_FLOWRING */

#ifndef	DHD_IS_DYN_FLOWRING
#define	DHD_IS_DYN_FLOWRING(dhdp, ring)	FALSE
#endif /* DHD_IS_DYN_FLOWRING */

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
	        dma_buf->len, (dma_addr_t *) &dma_buf->pa,                 \
	        (GFP_DMA32 | (in_interrupt() ? GFP_ATOMIC : GFP_KERNEL)))
#	define ARCH_FREE_COHERENT_MEM(dhd, dma_buf)                        \
	do {                                                               \
	    unsigned long pa_addr;                                         \
	    PHYSADDRTOULONG(dma_buf->pa, pa_addr);                         \
	    dma_free_coherent(&dhd->bus->dev->dev, dma_buf->len,           \
	        dma_buf->va, (dma_addr_t)pa_addr);                         \
	} while (0)
#	define ARCH_SET_DMA_BUF_VA(dma_buf, addr) \
	    (dma_buf)->va = (void*)(addr);
#	define ARCH_SET_DMA_BUF_PA(osh, dma_buf)       \
	    OSL_VIRT_TO_PHYSADDR((osh), (dma_buf)->va, (dma_buf)->pa)
#   define ARCH_FLUSH_COHERENT(addr, len)   do {} while (0)
#	define RDPA_INIT_CFG_SET_PA(field, pa, off)                        \
	do {                                                               \
	    field = (uint32_t) PHYSADDRLO(pa) + off;                       \
	} while (0)
#	define ARCH_WR_DMA_IDX(ptr, idx) *(volatile u16*)(ptr) = (idx); wmb()
#	define ARCH_RD_DMA_IDX(ptr, idx) idx = LTOH16(*(ptr))
#else  /* ! __arm__ && ! CONFIG_ARM64 */
#	error "revisit memory management and endian handling support"
#endif /* ! __arm__ && ! CONFIG_ARM64 */


/*
 * +---------------------------------------------------------------------
 *           Section: External variables and functions declarations
 *
 * +---------------------------------------------------------------------
*/
#if IS_ENABLED(CONFIG_BCM_DHD_ARCHER)
static int bdmf_global_trace_level = 0;
#else
extern int bdmf_global_trace_level;
#endif
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
#define DHD_RNR_MAX_RADIOS                  4
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

/* N+M feature default, disabled from REL_5.04L.01 and REL_5.02L.07P1 */
#define DOR_NPM_DEFAULT                     0

/* When a per AC profile with -1 weight is used, use a two pass with a budget
 * for the first pass.
 */
#define DHD_RNR_PER_AC_PROFILE_BUDGET       64

#define DHD_RNR_IS_TX_OFFL_SUPPORTED(dhd_hlp)           \
	((dhd_hlp)->sup_feat.txoffl == 1)

#define DHD_RNR_IS_RX_OFFL_SUPPORTED(dhd_hlp)           \
	((dhd_hlp)->sup_feat.rxoffl == 1)

#define DHD_RNR_IS_OFFL_SUPPORTED(dhd_hlp)              \
	(DHD_RNR_IS_TX_OFFL_SUPPORTED(dhd_hlp) && DHD_RNR_IS_RX_OFFL_SUPPORTED(dhd_hlp))

#define DHD_RNR_RX_OFFLOAD(dhd_hlp)                     \
	((dhd_hlp)->rxcmpl_ring_cfg.offload == 1)
#define DHD_RNR_TXSTS_OFFLOAD(dhd_hlp)                  \
	((dhd_hlp)->txsts_ring_cfg.offload & DHD_RNR_TXSTS_CFG_ACCPKT_OFFL)
#define DOL_TXOL_EN                         DHD_RNR_TXSTS_OFFLOAD
#define DOL_RXOL_EN                         DHD_RNR_RX_OFFLOAD
#define DHD_RNR_NONACCPKT_TXSTS_OFFLOAD(dhd_hlp)        \
	((dhd_hlp)->txsts_ring_cfg.offload & DHD_RNR_TXSTS_CFG_NONACCPKT_OFFL)
#define PCIE_WR_CONFIG(rb, off, val)                    \
	pci_bus_write_config_dword((rb), 0, (off), (val))


/* DMA WR and RD index size (2/4 bytes) */
#define DHD_RNR_DMA_IDX_SZ(dhd)             ((dhd)->bus->rw_index_sz)

#define DHD_RNR_IF_ROLE_STA_OR_WDS(dhd, ifidx)          \
	(DHD_IF_ROLE_STA((dhd), (ifidx)) || DHD_IF_ROLE_WDS((dhd), (ifidx)))
/*
 * +---------------------------------------------------------------------
 *           Section: platform specifc settings
 * +---------------------------------------------------------------------
*/
/* BCA_CPEROUTER */
#define DOL_SWFEAT_CPUQDPC_DEF              TRUE              /* Enable */
#define DOL_SWFEAT_CMPLNOTIF_DEF            TRUE              /* Enable */
#define DHD_RNR_BCMC_TXOFFL_PRIORITY        1                 /* Normal */
#define DHD_RNR_INIT_PERIM_UNLOCK(dhdp)     do {} while (0)   /* no OP */
#define DHD_RNR_INIT_PERIM_LOCK(dhdp)       do {} while (0)   /* no OP */

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
#if IS_ENABLED(CONFIG_BCM_DHD_ARCHER)
#define DHD_RNR_MEM_TYPE_RNR         DHD_RNR_MEM_ALLOC_TYPE_KMALLOC    /* owner RNR */
#else
#define DHD_RNR_MEM_TYPE_RNR         DHD_RNR_MEM_ALLOC_TYPE_COHERENT   /* owner RNR */
#endif
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
#define DHD_RNR_FLOWRING_DEFAULT_PROFILE_ID 5
#define DHD_RNR_FLOWRING_MAX_SIZE           0x8000  /* 16bit & power of 2 */

/* flow ring profile setting maintained per radio */
typedef struct dhd_flowring_profile {
	bool nplusm;                /* only N+M profile */
	int id;                     /* Profile id, 0 - DHD_RNR_FLOWRING_PROFILES-1 */
	int weight[wme_ac_max + 1]; /* -1 weight implies use rsvd memory */
	int items[wme_ac_max + 1];  /* flow ring max items (size) */
	int bqsize[wme_ac_max + 1]; /* backup queue max depth */
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

#define DOR_CFGSTS_FMTCHG                    (1 << 1)     /* Format Change */
#define DOR_CFGSTS_ITMCHG                    (1 << 2)     /* max_items Change */

/* DHD PCIe msg ring information maintained in dhd_runner */
typedef struct dhd_msgring_profile {
	int offload;            /* enabled: 1 (default), disabled: 0 */
	int max_items;          /* max items in msg ring (configurable for txpost, rxpost) */
	int type;               /* legacy, cwi32, cwi64 */
	int size;               /* len of the work item */
	int cfgsts;             /* Ring configuration status */
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

/* Default ring sizes for BA256 supported radios */
#define DOR_DEFAULT_PROFILE_ID_BA256         4
#define DOR_TXP_AC_BE_BQSIZE_BA256           2048
#define DOR_RXC_RINGSZ_BA256                 4096

#define DHD_RNR_TXPOST_MAX_ITEM              2048

/* Default backup queue sizes per Access Category */
#define DOR_TXP_AC_BK_BQSIZE_MAX             512
#define DOR_TXP_AC_BE_BQSIZE_MAX             1024
#define DOR_TXP_AC_VI_BQSIZE_MAX             512
#define DOR_TXP_AC_VO_BQSIZE_MAX             0
#define DOR_TXP_AC_BCMC_BQSIZE_MAX           0

/* Default flowring sizes per Access Category */
#define DOR_TXP_AC_BK_RINGSZ_MAX             512
#define DOR_TXP_AC_BE_RINGSZ_MAX             1024
#define DOR_TXP_AC_VI_RINGSZ_MAX             512
#define DOR_TXP_AC_VO_RINGSZ_MAX             512
#define DOR_TXP_AC_BCMC_RINGSZ_MAX           512
#define DOR_TXP_AC_RINGSZ_MAX                512

/* Default TxPost items size per Access Category */
#define DOR_TXP_ITEMS_MAX(ac) \
	(DOR_TXP_##ac##_RINGSZ_MAX + DOR_TXP_##ac##_BQSIZE_MAX)

/* Same TxPost items size per Access Category */
#define DOR_TXP_ITEMS_SAME(ac) \
	(DOR_TXP_AC_RINGSZ_MAX + DOR_TXP_##ac##_BQSIZE_MAX)

/*
 * profile items and backup queue size lists
 */

/* Default profile items list (ring size + backup queue depth) */
#define DOR_TXP_ITEMS_LIST_DEF \
{ \
	DOR_TXP_ITEMS_MAX(AC_BK), DOR_TXP_ITEMS_MAX(AC_BE), \
	DOR_TXP_ITEMS_MAX(AC_VI), DOR_TXP_ITEMS_MAX(AC_VO), \
	DOR_TXP_ITEMS_MAX(AC_BCMC) \
}

/* Profile items list (same ring size + default backup queue depth) */
#define DOR_TXP_ITEMS_LIST_SAME \
{ \
	DOR_TXP_ITEMS_SAME(AC_BK), DOR_TXP_ITEMS_SAME(AC_BE), \
	DOR_TXP_ITEMS_SAME(AC_VI), DOR_TXP_ITEMS_SAME(AC_VO), \
	DOR_TXP_ITEMS_SAME(AC_BCMC) \
}

/* Profile with Ring Size 512 for all ac and default backup queue depth */
#define DOR_TXP_ITEMS_LIST_RING_512 \
{ \
	DOR_TXP_ITEMS_MAX(AC_BK), DOR_TXP_ITEMS_MAX(AC_BE), \
	DOR_TXP_ITEMS_MAX(AC_VI), DOR_TXP_ITEMS_MAX(AC_VO), \
	DOR_TXP_ITEMS_MAX(AC_BCMC) \
}

/* Default profile backup queue depth list */
#define DOR_TXP_BQSIZE_LIST_DEF \
{ \
	DOR_TXP_AC_BK_BQSIZE_MAX, DOR_TXP_AC_BE_BQSIZE_MAX, \
	DOR_TXP_AC_VI_BQSIZE_MAX, DOR_TXP_AC_VO_BQSIZE_MAX, \
	DOR_TXP_AC_BCMC_BQSIZE_MAX \
}

/* Max profile items list */
#define DOR_TXP_ITEMS_LIST_MAX \
{ \
	DHD_RNR_TXPOST_MAX_ITEM, DHD_RNR_TXPOST_MAX_ITEM, \
	DHD_RNR_TXPOST_MAX_ITEM, DHD_RNR_TXPOST_MAX_ITEM, \
	DHD_RNR_TXPOST_MAX_ITEM \
}

/* Max profile backup queue depth list */
#define DOR_TXP_BQSIZE_LIST_MAX \
{ \
	(DHD_RNR_TXPOST_MAX_ITEM - DOR_TXP_AC_BK_RINGSZ_MAX), \
	(DHD_RNR_TXPOST_MAX_ITEM - DOR_TXP_AC_BE_RINGSZ_MAX), \
	(DHD_RNR_TXPOST_MAX_ITEM - DOR_TXP_AC_VI_RINGSZ_MAX), \
	(DHD_RNR_TXPOST_MAX_ITEM - DOR_TXP_AC_VO_RINGSZ_MAX), \
	(DHD_RNR_TXPOST_MAX_ITEM - DOR_TXP_AC_BCMC_RINGSZ_MAX) \
}

/* BA256 configuration profile BE Category TxPost items */
#define DOR_TXP_AC_BE_ITEMS_BA256 \
	(DOR_TXP_AC_BE_RINGSZ_MAX + DOR_TXP_AC_BE_BQSIZE_BA256)

/* BA256 profile items list */
#define DOR_TXP_ITEMS_LIST_BA256 \
{ \
	DOR_TXP_ITEMS_MAX(AC_BK), DOR_TXP_AC_BE_ITEMS_BA256, \
	DOR_TXP_ITEMS_MAX(AC_VI), DOR_TXP_ITEMS_MAX(AC_VO), \
	DOR_TXP_ITEMS_MAX(AC_BCMC) \
}

/* BA256 profile backup queue depth list */
#define DOR_TXP_BQSIZE_LIST_BA256 \
{ \
	DOR_TXP_AC_BK_BQSIZE_MAX, DOR_TXP_AC_BE_BQSIZE_BA256, \
	DOR_TXP_AC_VI_BQSIZE_MAX, DOR_TXP_AC_VO_BQSIZE_MAX, \
	DOR_TXP_AC_BCMC_BQSIZE_MAX \
}

/* TxPost flow ring size given profile and Access Category */
#define DOR_PROFILE_TXP_RING_SIZE(profile, ac) \
	((profile)->items[(ac)] - (profile)->bqsize[(ac)])

/*
 * Storage area selection for key settings
 */
#define DHD_RNR_RADIO_IDX_ALL               'a'
/* Comment out below line if Scratchpad is used instead of NVRAM */
#define DHD_RNR_NVRAM_KEYS

/* key ids within dhd runner */
#define DHD_RNR_KEY_STR_LEN                 32
#define DHD_RNR_VAL_STR_LEN                 128
#define DHD_RNR_KEY_PROFILE                 0
#define DHD_RNR_KEY_POLICY                  1
#define DHD_RNR_KEY_RXOFFL                  2
#define DHD_RNR_KEY_TXOFFL                  3
#define DHD_RNR_KEY_PHY_RING_SIZE           4
#define DHD_RNR_KEY_OFFL_OVERRIDE           5
#define DHD_RNR_KEY_CODEL                   6
#define DHD_RNR_KEY_CAP_OVERRIDE            7
#define DHD_RNR_MAX_KEYS                    8

/* Max length of key information stored in memory */
#define DHD_RNR_KEY_PPROFILE_STR_LEN        128   /* Profile information */
#define DHD_RNR_KEY_POLICY_STR_LEN          768   /* Policy information */
#define DHD_RNR_KEY_RXOFFL_STR_LEN          16    /* Rx Offload information */
#define DHD_RNR_KEY_TXOFFL_STR_LEN          16    /* Tx Offload information */
#define DHD_RNR_KEY_PHYRINGSIZE_STR_LEN     64    /* physical ring size information */
#define DHD_RNR_KEY_OFFL_OVERRIDE_STR_LEN   32    /* Offload override information */
#define DHD_RNR_KEY_CODEL_STR_LEN           16    /* Runner Codel information */
#define DHD_RNR_KEY_CAP_ORIDE_STR_LEN       32    /* Helper Capabilities override key */
#define DHD_RNR_KEY_UINT32_STR_LEN          16    /* key holding uint32 information */

/* storage key id vs format str */
char dhd_runner_key_fmt_str[DHD_RNR_MAX_KEYS][DHD_RNR_KEY_STR_LEN] = {
	"dhd%d_rnr_flowring_profile",
	"dhd%d_rnr_flowring_policy",
	"dhd%d_rnr_rxoffl",
	"dhd%d_rnr_txoffl",
	"dhd%d_rnr_flowring_physize",
	"dhd_rnr_offload_override",
	"dhd%d_rnr_codel",
	"dol%s_cap_%s_override",
};

/* flowring profiles id vs profile */
dhd_flowring_profile_t dhd_rnr_profiles[DHD_RNR_FLOWRING_PROFILES] =
{
	/* User defined Profiles */
	{ false, 0, { 1, -1, -1, -1, 1 },                   /* Radio#0 */
	    DOR_TXP_ITEMS_LIST_DEF, DOR_TXP_BQSIZE_LIST_DEF },
	{ false, 1, { 1, -1, -1, -1, 1 },                   /* Radio#1 */
	    DOR_TXP_ITEMS_LIST_DEF, DOR_TXP_BQSIZE_LIST_DEF },
	{ false, 2, { 1, -1, -1, -1, 1 },                   /* Radio#2 */
	    DOR_TXP_ITEMS_LIST_DEF, DOR_TXP_BQSIZE_LIST_DEF },
	{ false, 3, { 1, -1, -1, -1, 1 },                   /* Radio#3 */
	    DOR_TXP_ITEMS_LIST_DEF, DOR_TXP_BQSIZE_LIST_DEF },

	/* Built-in fixed profiles */
	{ false, 4, { 1, -1, -1, -1, 1 },                   /* BA256 */
	    DOR_TXP_ITEMS_LIST_BA256, DOR_TXP_BQSIZE_LIST_BA256 },
	{ false, 5, { 1, -1, -1, -1, DHD_RNR_BCMC_TXOFFL_PRIORITY }, /* Default */
	    DOR_TXP_ITEMS_LIST_DEF, DOR_TXP_BQSIZE_LIST_DEF },
	{ false, 6, { 1, 1, 1, 1, 1 },                      /* same ring size for all ac */
	    DOR_TXP_ITEMS_LIST_SAME, DOR_TXP_BQSIZE_LIST_DEF },
	{ false, 7, { 1, 1, 1, 1, 1 },                       /* 2K ring+bq */
	    DOR_TXP_ITEMS_LIST_MAX, DOR_TXP_BQSIZE_LIST_MAX }
};

/* dhd radio vs flowring policy */
dhd_flowring_policy_t dhd_rnr_policies[DHD_RNR_MAX_RADIOS] =
{
	{ .id = dhd_rnr_policy_global, .all_hw = TRUE },
	{ .id = dhd_rnr_policy_global, .all_hw = TRUE },
	{ .id = dhd_rnr_policy_global, .all_hw = TRUE },
	{ .id = dhd_rnr_policy_global, .all_hw = TRUE }
};

/*
 * Default physical flow ring size when runner backup queues are enabled
 * - per access category
 */
const uint16 dhd_rnr_txp_ringsz_defaults[] = {
	DOR_TXP_AC_BK_RINGSZ_MAX,
	DOR_TXP_AC_BE_RINGSZ_MAX,
	DOR_TXP_AC_VI_RINGSZ_MAX,
	DOR_TXP_AC_VO_RINGSZ_MAX,
	DOR_TXP_AC_BCMC_RINGSZ_MAX
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

/* flow ring id allocation mode */
#define FLOWID_ALLOC_LEGACY                 0  /* based on id16 (Legacy) */
#define FLOWID_ALLOC_IDMA                   1  /* Based on iDMA group */
#define FLOWID_ALLOC_WMEAC                  2  /* Based on WME AC */
#define FLOWID_ALLOC_MAX_MODES              3  /* Delimiter */
#define FLOWID_ALLOC_MODE_STR_LEN           12 /* mode string size */

#define FLOWID_ALLOC_DEFAULT                FLOWID_ALLOC_WMEAC
static char
flowid_alloc_mode_str[FLOWID_ALLOC_MAX_MODES][FLOWID_ALLOC_MODE_STR_LEN] = {
	"Flat id16",
	"iDMA Group",
	"WME AC"
};

/* flow ring manager in dhd runner */
typedef struct dhd_runner_flowmgr
{
	int alloc_mode;     /* flowid allocation mode */
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
	int def_profile_id;                 /* Default profile id */

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
 * DHD Offload helper supported features
 */
#if defined(RDPA_DHD_HELPER_FEATURE_NPLUSM)
#define DOL_HLPR_CAP_NPLUSM
#endif /* RDPA_DHD_HELPER_FEATURE_NPLUSM */
#if defined(RDPA_DHD_HELPER_FEATURE_TXCOMPL_SUPPORT)
#define DOL_HLPR_CAP_TXCMPL2HOST
#endif /* RDPA_DHD_HELPER_FEATURE_TXCOMPL_SUPPORT */

#if defined(RDPA_DHD_HELPER_FEATURE_LLCSNAPHDR_SUPPORT)
#define DOL_HLPR_CAP_LLCSNAPHDR
#endif /* DOL_HLPR_CAP_LLCSNAPHDR */

#if defined(RDPA_DHD_HELPER_FEATURE_LBRAGGR_SUPPORT)
#define DOL_HLPR_CAP_LBRAGGR
#endif /* RDPA_DHD_HELPER_FEATURE_LBRAGGR_SUPPORT */

#if defined(RDPA_DHD_HELPER_FEATURE_MSGFORMAT_SUPPORT)
#define DOL_HLPR_CAP_MSGRINGFRMT
#endif /* RDPA_DHD_HELPER_FEATURE_MSGFORMAT_SUPPORT */

#if defined(RDPA_DHD_HELPER_FEATURE_FAST_FLOWRING_DELETE_SUPPORT)
#define DOL_HLPR_CAP_FFRD
#endif /*  RDPA_DHD_HELPER_FEATURE_FAST_FLOWRING_DELETE_SUPPORT */

#if defined(RDPA_DHD_HELPER_FEATURE_IDMA_GROUP_CFG_SUPPORT)
#define DOL_HLPR_CAP_IDMAGRPCFG
#endif /*  RDPA_DHD_HELPER_FEATURE_IDMA_GROUP_CFG_SUPPORT */

#if defined(RDPA_DHD_HELPER_FEATURE_IDMA64_SUPPORT)
#define DOL_HLPR_CAP_IDMA64
#endif /*  RDPA_DHD_HELPER_FEATURE_IDMA64_SUPPORT */

#if defined(MSGBUF_WI_COMPACT) && defined(DOL_HLPR_CAP_MSGRINGFRMT)
/*
 * Runner supports only two formats
 *   0: Legacy
 *   1: CWI (TxPost::CWI64, RxPost::CWI32, TXCPL::CWI32, RXCPL::CWI32)
 */
#define DOR_RINGTYPE_TO_RNR_RINGFMT(type)   (((type) == MSGBUF_WI_WI64) ? 0 : 1)
#else /* !MSGBUF_WI_COMPACT || !DOL_HLPR_CAP_MSGRINGFRMT */
#define DOR_RINGTYPE_TO_RNR_RINGFMT(type)    0
#endif /* !MSGBUF_WI_COMPACT || !DOL_HLPR_CAP_MSGRINGFRMT */

#if defined(RDPA_DHD_HELPER_FEATURE_BACKUP_QUEUE_SUPPORT)
#define DOL_HLPR_CAP_BKUPQUEUE
#define DHD_RNR_DEF_BKUPQ                   1
#define DHD_RNR_BKUPQ(dhd_hlp)              ((dhd_hlp)->en_feat.bkupq == 1)
#define DHD_RNR_PHY_RING_SIZE(cache)        (cache)->phy_ring_size
#else /* !RDPA_DHD_HELPER_FEATURE_BACKUP_QUEUE_SUPPORT */
#define DHD_RNR_DEF_BKUPQ                   0
#define DHD_RNR_BKUPQ(dhd_hlp)              0
#define DHD_RNR_PHY_RING_SIZE(cache)        (cache)->items
#endif /* !RDPA_DHD_HELPER_FEATURE_BACKUP_QUEUE_SUPPORT */

#if defined(RDPA_DHD_HELPER_FEATURE_HWA_WAKEUP_SUPPORT)
#define DOL_HLPR_CAP_HWA_WAKEUP
#endif /* RDPA_DHD_HELPER_FEATURE_HWA_WAKEUP_SUPPORT */

#if defined(RDPA_DHD_HELPER_FEATURE_CODEL_SUPPORT)
#define DOL_HLPR_CAP_CODEL
#endif /* RDPA_DHD_HELPER_FEATURE_CODEL_SUPPORT */

#if defined(RDPA_DHD_HELPER_FEATURE_HBQD_SUPPORT)
#define DOL_HLPR_CAP_HBQD
#endif /* RDPA_DHD_HELPER_FEATURE_HBQD_SUPPORT */

#if defined(RDPA_DHD_HELPER_FEATURE_FLOWRING_UPDATE_SUPPORT)
#define DOL_HLPR_CAP_DYNBKUPQ
#endif /* RDPA_DHD_HELPER_FEATURE_FLOWRING_UPDATE_SUPPORT */

#if defined(RDPA_DHD_HELPER_FEATURE_COMMON_RING_SIZES_CFG_SUPPORT)
#define DOL_HLPR_CAP_CMNRNGSZ
#endif /* RDPA_DHD_HELPER_FEATURE_COMMON_RING_SIZES_CFG_SUPPORT */

#if defined(RDPA_DHD_HELPER_FEATURE_MSCS_SUPPORT)
#define DOL_HLPR_CAP_RXCMPLV2
#endif /*  RDPA_DHD_HELPER_FEATURE_MSCS_SUPPORT */

#if defined(RDPA_DHD_HELPER_FEATURE_RCH_SUPPORT)
#define DOL_HLPR_CAP_RXCMPLRCH
#endif /*  RDPA_DHD_HELPER_FEATURE_RCH_SUPPORT */

#if defined(RDPA_DHD_HELPER_FEATURE_BA256_CFG_SUPPORT)
#define DOL_HLPR_CAP_BA256CFG
#endif /*  RDPA_DHD_HELPER_FEATURE_BA256_CFG_SUPPORT */

#define DOL_HLPR_CAP_EN_STS_STR(dol, feat)                             \
	(dol->sup_feat.feat ?                                              \
	    (dol->en_feat.feat ? #feat":1" : #feat":0") : #feat":0")

#define DOL_HLPR_CAP_STS_STR(dol, gfeat1, gfeat2, feat)                \
	(dol->sup_feat.feat ?                                              \
	    ((dol->en_feat.feat && \
	     (dol->en_feat.gfeat1 || dol->en_feat.gfeat2)) ?               \
	        #feat":1" : #feat":0") : #feat":X")

typedef dhd_helper_feat_t dol_hlpr_caps_t;

#define DOL_NV_CAP_SUPPORT(dol, feat)                                  \
({                                                                     \
	char buff[DHD_RNR_KEY_UINT32_STR_LEN];                             \
	int  supported = 1;                                                \
	snprintf(buff, sizeof(buff), "%s", #feat);                         \
	if (dhd_runner_key_get(dol->dhd->unit,                             \
	    DHD_RNR_KEY_CAP_OVERRIDE, buff, sizeof(buff)) != 0) {          \
	    sscanf(buff, "%d", &supported);                                \
	} else  if (dhd_runner_key_get(DHD_RNR_RADIO_IDX_ALL,              \
	    DHD_RNR_KEY_CAP_OVERRIDE, buff, sizeof(buff)) != 0) {          \
	    sscanf(buff, "%d", &supported);                                \
	}                                                                  \
	((supported > 0) ? 1 : 0);                                         \
})

#define DOL_HLPR_CAP_SUPPORT_INIT(dol, feat)                           \
	dol->sup_feat.feat = DOL_NV_CAP_SUPPORT(dol, feat)

#define DOL_SWFEAT_ENABLE_INIT(dol, feat)                              \
	dol->sw_feat.feat = DOL_NV_CAP_SUPPORT(dol, feat)

#define DOL_SWFEAT_STS_STR(dol, feat)                                  \
	(dol->sw_feat.feat ? #feat":1" : #feat":0")

typedef struct dol_swfeat {
	bool cpuqdpc;                   /* process cpu queue in DHD DPC */
	bool cmplnotif;                 /* Tx-Rx Complete notifications to helper */
} dol_swfeat_t;

#define DOL_MAX_DHD_OFFL_RADIOS             RDPA_MAX_DHD_OFFL_RADIOS
#if defined(RDPA_MAX_CONCUR_OFFL_RADIOS)
#define DOL_MAX_CONCUR_OFFL_RADIOS          RDPA_MAX_CONCUR_OFFL_RADIOS
#else /* !RDPA_MAX_CONCUR_OFFL_RADIOS */
#define DOL_MAX_CONCUR_OFFL_RADIOS          RDPA_MAX_DHD_OFFL_RADIOS
#endif /* !RDPA_MAX_CONCUR_OFFL_RADIOS */

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
#ifndef XRDP
	rdpa_cpu_reason trap_reason;          /* Runner flow miss reason */
#endif
	struct tasklet_struct dhd_rx_tasklet; /* Tasklet for processing RXCMPL and TXSTS */
	rdpa_dhd_init_cfg_t dhd_init_cfg;     /* init config object */
	bdmf_object_handle dhd_helper_obj;    /* dhd helper object in runner */
	bdmf_object_handle dhd_mcast_obj;     /* multicast object in runner */
	bdmf_object_handle cpu_obj;           /* cpu object in runner */

	struct pci_dev *pci_dev;               /* PCI device pointer */
	phys_addr_t    pci_bar;                /* Runner Wakeup Window Address */
	bool ipend;                            /* runner cpuq interrupt pending */
	dhd_dma_buf_t flring_cache_dma_buf;    /* dma buffer object for flow ring cache */
	dhd_runner_flowmgr_t flowmgr;          /* Flow manager object */
	dhd_runner_ring_cfg_t rxpost_ring_cfg; /* RXPOST ring configuration object */
	dhd_runner_ring_cfg_t txpost_ring_cfg; /* TXPOST ring configuration object */
	dhd_runner_ring_cfg_t rxcmpl_ring_cfg; /* RXCMPL ring configuration object */
	dhd_runner_ring_cfg_t txsts_ring_cfg;  /* TXSTS ring configuration object */
	dhd_dma_buf_t coherent_mem_pool;       /* Coherent memory pool object */
	dol_hlpr_caps_t sup_feat;      /* Features supported by Offload helper */
	dol_hlpr_caps_t en_feat;       /* Features enabled in Offload helper */
	dol_swfeat_t    sw_feat;       /* Features enabled in Offload (software) */

	/* local counters */
	ulong h2r_txpost_notif;        /* Host notifies Runner to post tx packets */
	ulong h2r_txp_fail;            /* Number of Tx Post Notification failures */
	ulong h2r_rx_compl_notif;      /* Host notifies Runner to process D2H RxCompl */
	ulong h2r_tx_compl_notif;      /* Host notifies Runner to process D2H TxCompl */
	ulong r2h_rx_compl_req;        /* Runner requests Host to receive a packet */
	ulong r2h_tx_compl_req;        /* Runner requests Host to free a packet */
	ulong r2h_wake_dngl_req;       /* Runner requests Host to wake dongle */
	ulong o2h_sq_txp_notif;        /* TxPost packet through Service Queue */

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
static void dhd_runner_pcie_deinit(dhd_runner_hlp_t *dhd_hlp);

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

/* BA256 configuration feature */
static int dhd_runner_ba256cfg_init(dhd_runner_hlp_t *dhd_hlp);

/* Initialize Runner Codel feature */
static int dhd_runner_codel_init(struct dhd_runner_hlp *dhd_hlp);

/* Force disable Offload */
static int
dhd_runner_force_disable_offload(struct dhd_runner_hlp *dhd_hlp, bool tx_dor,
	bool rx_dor);

/* DHD proto layer init completed, configure Runner helper object */
static int  dhd_runner_init(dhd_runner_hlp_t *dhd_hlp, struct pci_dev *pci_dev);

/* DHD insmod and rmmod callbacks */
static int  dhd_helper_attach(dhd_runner_hlp_t *dhd_hlp, void *dhd);
static void dhd_helper_detach(dhd_runner_hlp_t *dhd_hlp);

static INLINE int dhd_runner_wakeup_init(dhd_runner_hlp_t *dhd_hlp,
	bcmpcie_soft_doorbell_t *soft_doorbell, phys_addr_t wakeup_paddr,
	uint32 wakeup_val32);

/* Allocate a flowring id */
static int dhd_runner_flowring_alloc(dhd_runner_flowmgr_t *flowmgr,
	dhd_wme_ac_t wme_ac, bool force_dhd);

/* Initialize a flowring buffer in carved or cached memory */
static int dhd_runner_flowring_init(dhd_runner_flowmgr_t *flowmgr,
	uint16 flow_id);

static void dhd_runner_flring_cache_enable(dhd_runner_hlp_t *dhd_hlp,
	uint32_t ringid, int enable);


/* Persistent Scratch Pad area  access */
static int dhd_runner_key_get(int radio_idx, int key_id, char *buff, int len);
#if !defined(DHD_RNR_NVRAM_KEYS)
static int dhd_runner_key_set(int radio_idx, int key_id, char *buff, int len);
#endif /* !DHD_RNR_NVRAM_KEYS */

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
	dhd_runner_ring_cfg_t *rxc_ring_cfg, dhd_runner_ring_cfg_t *rxp_ring_cfg);

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

/*
 * DHD Offload layer structure - Global for all radios
 */
typedef struct dhdol {
	bool up_inited;                    /* user profiles initializion status */
} dhdol_t;

static struct dhdol dhdol_cb = { 0 };

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

	        /* Offload engine supports 32bit physical address only */
	        if (PHYSADDRHI(dma_buf->pa)) {
	            DHD_ERROR(("[COHERENT_POOL_%d] Allocated 0x%x size 0x%px, "
	                "[0x%x_%x] 0x%px from upper 2GB\r\n",
	                dhd_hlp->dhd->unit, dma_buf->_alloced, dma_buf->va,
	                (uint32)PHYSADDRHI(dma_buf->pa),
	                (uint32)PHYSADDRLO(dma_buf->pa), coherent_pool->va));
	            ARCH_FREE_COHERENT_MEM(dhd, dma_buf);
	            memset(dma_buf, 0, sizeof(dhd_dma_buf_t));
	        }
	    } else {
	        /* Get from Local Coherent Pool */
	        dma_buf->va = coherent_pool->va + coherent_pool->_alloced;
	        pa_low = PHYSADDRLO(coherent_pool->pa) + coherent_pool->_alloced;
	        PHYSADDRLOSET((dma_buf)->pa, pa_low);
	        PHYSADDRHISET((dma_buf)->pa, PHYSADDRHI(coherent_pool->pa));
	        dma_buf->_alloced = aligned_len;
	        coherent_pool->_alloced += aligned_len;
	        DHD_INFO(("[COHERENT_POOL_%d] Allocated 0x%x size 0x%px, 0x%x_%x "
	            "from pool 0x%px\r\n",
	            dhd_hlp->dhd->unit, dma_buf->_alloced, dma_buf->va,
	            (uint32)PHYSADDRHI(dma_buf->pa),
	            (uint32)PHYSADDRLO(dma_buf->pa), coherent_pool->va));
	    }
	} else if (type == DHD_RNR_MEM_ALLOC_TYPE_KMALLOC) {
	    dma_buf->va = kmalloc(dma_buf->len, GFP_ATOMIC | __GFP_ZERO | GFP_DMA);
	    ARCH_SET_DMA_BUF_PA(dhd->osh, dma_buf);
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

#if IS_ENABLED(CONFIG_BCM_DHD_ARCHER)
static inline int
dhd_runner_txpost_spdsvc(dhd_runner_hlp_t *dhd_hlp, void *txp, rdpa_dhd_tx_post_info_t *info)
{
#if IS_ENABLED(CONFIG_BCM_SPDSVC)
	if (IS_SKBUFF_PTR(txp)) {
	    struct sk_buff *skb = PNBUFF_2_SKBUFF(txp);
	    spdsvcHook_transmit_t spdsvc_transmit = {};
	    dhd_pub_t *dhd = dhd_hlp->dhd;
	    union {
	        BlogRnr_t rnr;
	        uint32_t u32;
	    } tag = { .u32 = 0 };
	    int rc;

	    spdsvc_transmit.pNBuff = txp;
	    spdsvc_transmit.dev = dhd_linux_get_primary_netdev(dhd);
	    spdsvc_transmit.header_type = SPDSVC_HEADER_TYPE_ETH;
	    spdsvc_transmit.phy_overhead = WL_SPDSVC_OVERHEAD;
	    spdsvc_transmit.egress_type = SPDSVC_EGRESS_TYPE_DHD_OFFLOAD;
	    spdsvc_transmit.transmit_helper = NULL;

	    tag.rnr.radio_idx = info->radio_idx;
	    tag.rnr.flowring_idx = info->flow_ring_id;
	    tag.rnr.priority = LINUX_GET_PRIO_MARK(skb->mark);
	    tag.rnr.llcsnap_flag = dhd_hlp->dhd_init_cfg.add_llcsnap_header;

	    spdsvc_transmit.tag = tag.u32;

	    rc = dhd_runner_spdsvc_transmit(&spdsvc_transmit);
	    if (unlikely(rc > 0)) {
	        // Buffer was hijacked by spdsvc, abort transmission
	        rc = -1;
	    }

	    return rc;
	}
#endif /* CONFIG_BCM_SPDSVC */
	return 0;
}
#else /* !CONFIG_BCM_DHD_ARCHER */
static inline int
dhd_runner_txpost_spdsvc(dhd_runner_hlp_t *dhd_hlp, void *txp, rdpa_dhd_tx_post_info_t *info)
{
#if IS_ENABLED(CONFIG_BCM_SPDSVC)
	spdsvcHook_transmit_t spdsvc_transmit = {};

	spdsvc_transmit.pNBuff = txp;
	spdsvc_transmit.dev = NULL;
	spdsvc_transmit.header_type = SPDSVC_HEADER_TYPE_ETH;
	spdsvc_transmit.phy_overhead = WL_SPDSVC_OVERHEAD;
	spdsvc_transmit.so_mark = 0;
	spdsvc_transmit.egress_type = SPDSVC_EGRESS_TYPE_DHD_OFFLOAD;
	spdsvc_transmit.transmit_helper = NULL;

	info->is_spdsvc_setup_packet = dhd_runner_spdsvc_transmit(&spdsvc_transmit);

	info->spdt_so_mark = spdsvc_transmit.so_mark;
#else /* !CONFIG_BCM_SPDSVC */
	info->is_spdsvc_setup_packet = 0;
#endif /* !CONFIG_BCM_SPDSVC */

	return info->is_spdsvc_setup_packet;
}
#endif /* !CONFIG_BCM_DHD_ARCHER */

int
dhd_runner_txpost(dhd_runner_hlp_t *dhd_hlp, void *txp, uint32 ifindex)
{
	int rc;
	dhd_pub_t *dhd = dhd_hlp->dhd;
	int pktlen = PKTLEN(dhd->osh, txp);
	rdpa_dhd_tx_post_info_t info = {};

	info.radio_idx = dhd->unit;
	info.flow_ring_id = DHD_PKT_GET_FLOWID(txp);
	info.ssid_if_idx = ifindex;
#if defined(MLO_BCMC)
	if (DHD_PKT_GET_MLO_BCMC(txp)) {
	    uint16 bcmc_seqno = DHD_MLO_BCMC_PKTTAG_SEQ(txp);
	    DHD_INFO(("dhd%d BCMCSEQ seq 0x%X\n",
	        dhd->unit, bcmc_seqno));
	    ASSERT(MLO_BCMC_SEQNO_ISVALID(bcmc_seqno));
	    /* Assign bcmc_seqno to info.bcmc_seqno */
	    info.seqnum = bcmc_seqno;
	}
#endif /* MLO_BCMC */
	rc = dhd_runner_txpost_spdsvc(dhd_hlp, txp, &info);

	if (unlikely(rc < 0)) {
	    /* In case of error, NBuff will be free by spdsvc */
	    return 0;
	}

	rc = rdpa_dhd_helper_send_packet_to_dongle(txp, pktlen, &info);

	if (rc > 0) {
	    /*
	     * Sucessful return from service queue (RC_PKT_SENT_TO_SQ)
	     * Offload engine will own the Txp buffer and will not send
	     * TxComplete message back
	     */
	    rc = 0;
	    dhd_hlp->o2h_sq_txp_notif++;
	}

	if (rc != 0) {
	    DHD_TRACE(("dor%d %s (0x%px, %d, 0x%px) returned %d\r\n",
	        dhd->unit, "rdpa_dhd_helper_send_packet_to_dongle",
	        txp, pktlen, &info, rc));
	    dhd_hlp->h2r_txp_fail++;
	} else {
	    dhd_hlp->h2r_txpost_notif++;
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
#endif

	rc = rdpa_cpu_packet_get(port, queue, info);
	if (rc)
	    return rc;

#ifdef XRDP
#if defined(CONFIG_BCM_FCACHE_CLASSIFICATION_BYPASS) && !defined(CONFIG_BCM963158)
	if (!info->is_exception)
	{
	    flow_key = *(uint32_t*)((uint8_t*)info->data + info->data_offset);
	    info->data_offset += sizeof(flow_key);
	    info->size -= sizeof(flow_key);
	}
#endif

	*sysb = (bdmf_sysb*)skb_alloc(info);
	info->reason_data = info->dest_ssid;
#else /* XRDP */
	/* Extract the ssid ifidx from mapped hw_port */
	info->reason_data = WLAN_NETDEVPATH_SSID(info->reason_data);

	/* create a sysb and initialize it with packet data & len */
	*sysb = bdmf_sysb_header_alloc(bdmf_sysb_skb, (uint8_t *)info->data, info->data_offset,
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
	    skbuff_bcm_ext_fc_ctxt_set(((struct sk_buff *)(*sysb)), flow_key);
#endif

#if defined(DOL_HLPR_CAP_RXCMPLV2)
	if (dhd_hlp->en_feat.rxcmplv2 == 1) {
	    uint8 prio;

	    /* get and set the TID information */
	    prio = (info->wl_metadata >> RDPA_CPU_RX_INFO_WL_METADATA_TID_SHIFT);
	    prio &= RDPA_CPU_RX_INFO_WL_METADATA_TID_MASK;
	    ((struct sk_buff*)(*sysb))->wl.ucast.dhd.wl_prio = prio;
	}
#endif /* DOL_HLPR_CAP_RXCMPLV2 */
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

#if IS_ENABLED(CONFIG_BCM_DHD_ARCHER)
	    skb->ip_summed = info.csum_verified;
#endif
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
	rdpa_dhd_complete_data_t dhd_complete_data = { 0 };
	int pkts = 0;
	dhd_pub_t *dhdp;

	ASSERT(dhd_hlp != NULL);
	ASSERT(dhd_hlp->dhd != NULL);

	dhdp = dhd_hlp->dhd;
	/* If TxOffload is not enabled, Runner will not send txcmpl messages on this queue */
	if (!DHD_RNR_TXSTS_OFFLOAD(dhd_hlp))
	    return FALSE;

	dhd_complete_data.radio_idx = dhd_hlp->dhd->unit;

	while (!bound || (pkts < bound)) {

	    /* Fetch each tx complete packet from Runner to Host DHD complete queue */
	    rc = rdpa_dhd_helper_dhd_complete_message_get(&dhd_complete_data);

	    if (rc)
	        break;

	    rnrtxsts.pkt = NULL;
	    txsts = &rnrtxsts.dngl_txsts;
	    txsts->cmn_hdr.request_id = htonl(dhd_complete_data.request_id);

#if defined(DOL_HLPR_CAP_TXCMPL2HOST)
	    if ((dhd_complete_data.buf_type == RDPA_DHD_TX_POST_HOST_BUFFER_VALUE) &&
	        (dhd_complete_data.txp)) {
	        rnrtxsts.pkt = dhd_complete_data.txp;
	    }
#endif /* DOL_HLPR_CAP_TXCMPL2HOST */

	    if (dhd_hlp->txsts_ring_cfg.type) {

#if defined(DHD_RNR_TXCMPL_AUDIT)
	        /*
	         * Filter out the spurious/unwanted Tx Complete messages from Runner
	         */
	        if (dhd_complete_data.buf_type == RDPA_DHD_TX_POST_HOST_BUFFER_VALUE) {
	            if ((dhd_complete_data.txp == (void*)-1) || (dhd_complete_data.txp == NULL)) {
	                DHD_ERROR(("dhd%d_rnr Discard TxComplete: pkt[0x%px], req_id[0x%x]\r\n",
	                    dhd_hlp->dhd->unit, dhd_complete_data.txp,
	                    ltoh32(txsts->cmn_hdr.request_id)));
	                continue;
	            }
	        } else if (ltoh32(txsts->cmn_hdr.request_id) == 0) {
	            DHD_ERROR(("dhd%d_rnr Discard TxComplete: req_id[0x%x], pkt[0x%px]\r\n",
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
	            (ltoh16(txsts->compl_hdr.flow_ring_id) >
	            (dhd_hlp->flowmgr.max_h2d_rings - MAX_DYN_FLOWRINGS(dhdp))) ||
#if defined(DOL_HLPR_CAP_TXCMPL2HOST)
	            ((dhd_complete_data.buf_type == RDPA_DHD_TX_POST_HOST_BUFFER_VALUE) &&
	             ((dhd_complete_data.txp == (void*)-1) || (dhd_complete_data.txp == NULL)))||
#endif /* DOL_HLPR_CAP_TXCMPL2HOST */
	            (ltoh16(txsts->tx_status) != 0)) {
	            DHD_ERROR(("dhd%d_rnr Discard TxComplete: req_id[0x%x], flow_ring_id[0x%x]",
	                dhd_hlp->dhd->unit, ltoh32(txsts->cmn_hdr.request_id),
	                ltoh16(txsts->compl_hdr.flow_ring_id)));
	            DHD_ERROR((" status [0x%x] buf_type [%d] txp [0x%px]\r\n",
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

#if defined(CONFIG_BCM_DHD_CROSSBOW) && defined(CC_CROSSBOW_SWBCACPE_60718)
	resched |= !rdpa_cpu_rxq_empty(dhd_hlp->cpu_port, dhd_hlp->cpu_queue_id);
#endif
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

	if ((dhd_hlp->sw_feat.cpuqdpc == TRUE) && (dhd_hlp->ipend == TRUE)) {
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

	if (dhd_hlp->sw_feat.cpuqdpc == TRUE) {
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
	    DHD_ERROR(("dor%d rdpa_cpu_num_queues_get(0x%px, 0x%px) returned %d\r\n",
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
	int cpu_reasons[] = {rdpa_cpu_rx_reason_pci_ip_flow_miss_1,
	    rdpa_cpu_rx_reason_pci_ip_flow_miss_2,
	    rdpa_cpu_rx_reason_pci_ip_flow_miss_3};
#endif
	rdpa_cpu_rxq_cfg_t rxq_cfg = {};
	rdpa_cpu_port cpu_port;
	int cpu_queue_size = init ? RDPA_DHD_HELPER_CPU_QUEUE_SIZE : 0;

	if (!cpu_queue_size)
	{
	    /* Cpu object is not configured yet, nothing to do */
	    if (!dhd_hlp->cpu_obj)
	        return 0;

	    rc = rdpa_cpu_index_get(dhd_hlp->cpu_obj, &cpu_port);
	    if (rc != 0) {
	        DHD_ERROR(("dor%d rdpa_cpu_index_get(0x%px, 0x%px) returned %d\r\n",
	            dhd_hlp->dhd->unit, dhd_hlp->cpu_obj, &cpu_port, rc));
	    }

	    rdpa_cpu_int_disable(dhd_hlp->cpu_port, dhd_hlp->cpu_queue_id);
	    rdpa_cpu_int_clear(dhd_hlp->cpu_port, dhd_hlp->cpu_queue_id);
	    tasklet_kill(&dhd_hlp->dhd_rx_tasklet);

	    rc = rdpa_cpu_rxq_flush_set(dhd_hlp->cpu_obj, dhd_hlp->cpu_queue_id, 1);
	    if (rc != 0) {
	        DHD_ERROR(("dor%d rdpa_cpu_rxq_flush_set(0x%px, %d, 1) returned %d\r\n",
	            dhd_hlp->dhd->unit, dhd_hlp->cpu_obj, dhd_hlp->cpu_queue_id, rc));
	    }
	}
	else
	{
#ifdef XRDP
	    cpu_port = rdpa_cpu_wlan0 + dhd_hlp->dhd->unit;
#else
	    cpu_port = rdpa_cpu_host;
	    dhd_hlp->trap_reason = cpu_reasons[dhd_hlp->dhd->unit];
#endif
	    rc = rdpa_cpu_get(cpu_port, &dhd_hlp->cpu_obj);
	    if (rc) {
	        DHD_ERROR(("dor%d rdpa_cpu_get(%d, 0x%px) returned %d\r\n",
	            dhd_hlp->dhd->unit, cpu_port, &dhd_hlp->cpu_obj, rc));
	        return rc;
	    }

	    dhd_hlp->cpu_port = cpu_port;
	    dhd_hlp->cpu_queue_id = cpu_queue_id_get(dhd_hlp->cpu_obj, dhd_hlp->dhd->unit);
	    if (dhd_hlp->cpu_queue_id < 0)
	        goto exit;

	    tasklet_init(&dhd_hlp->dhd_rx_tasklet, dhd_runner_rx_tasklet_handler,
	        (unsigned long)dhd_hlp);
	}

	rc = rdpa_cpu_rxq_cfg_get(dhd_hlp->cpu_obj, dhd_hlp->cpu_queue_id, &rxq_cfg);
	if (rc != 0) {
	    DHD_ERROR(("dor%d rdpa_cpu_rxq_cfg_get(0x%px, %d, 0x%px) returned %d\r\n",
	        dhd_hlp->dhd->unit, dhd_hlp->cpu_obj, dhd_hlp->cpu_queue_id, &rxq_cfg, rc));
	}

	rxq_cfg.size = cpu_queue_size;
	rxq_cfg.isr_priv = (long)dhd_hlp;
	rxq_cfg.rx_isr = cpu_queue_size ? dhd_runner_cpu_queue_isr : NULL;
	if (!cpu_queue_size)
	    rxq_cfg.ring_head = NULL; /* Destroy the ring */

	rc = rdpa_cpu_rxq_cfg_set(dhd_hlp->cpu_obj, dhd_hlp->cpu_queue_id, &rxq_cfg);
	if (rc < 0) {
	    DHD_ERROR(("dor%d rdpa_cpu_rxq_cfg_set(0x%px, %d, 0x%px) returned %d\r\n",
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

#if !IS_ENABLED(CONFIG_BCM_DHD_ARCHER)
	if (!cpu_queue_size)
	{
#ifdef XRDP
	    rc = rdpa_dhd_helper_cpu_data_set(dhd_hlp->dhd_helper_obj, NULL);
	    if (rc != 0) {
	        DHD_ERROR(("dor%d rdpa_dhd_helper_cpu_data_set(0x%px, NULL) returned %d\r\n",
	            dhd_hlp->dhd->unit, dhd_hlp->cpu_obj, rc));
	    }
#endif
	    goto exit;
	}

#ifdef XRDP
	{
	    rdpa_dhd_cpu_data_t cpu_data;

	    cpu_data.cpu_port = cpu_port;
	    cpu_data.exception_rxq = dhd_hlp->cpu_queue_id;

	    if (DHD_RNR_RX_OFFLOAD(dhd_hlp))
	    {
	        rc = rdpa_cpu_tc_to_rxq_set(dhd_hlp->cpu_obj,
	                dhd_hlp->cpu_queue_id, dhd_hlp->cpu_queue_id);
	        if (rc != 0) {
	            DHD_ERROR(("dor%d rdpa_cpu_tc_to_rxq_set(0x%px, %d, %d) returned %d\r\n",
	            dhd_hlp->dhd->unit, dhd_hlp->cpu_obj,
	            dhd_hlp->cpu_queue_id, dhd_hlp->cpu_queue_id, rc));
	        }

	        cpu_data.set_exception_tc_to_rxq = true;
	        rc = rc ? rc : rdpa_dhd_helper_cpu_data_set(dhd_hlp->dhd_helper_obj, &cpu_data);
	    }
	    else
	    {
	        /* do not touch tc to queue configuration, just setup interrupt for TxComplete */
	        cpu_data.set_exception_tc_to_rxq = false;
	        rc = rc ? rc : rdpa_dhd_helper_cpu_data_set(dhd_hlp->dhd_helper_obj, &cpu_data);
	    }

	    if (rc) {
	        DHD_ERROR(("dor%d %s(0x%px, 0x%px) returned %d\r\n", dhd_hlp->dhd->unit,
	             "rdpa_dhd_helper_cpu_data_set", dhd_hlp->dhd_helper_obj,  &cpu_data, rc));
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
	    rc = rdpa_cpu_reason_cfg_set(dhd_hlp->cpu_obj, &cpu_reason, &reason_cfg);
	    if (rc < 0) {
	        DHD_ERROR(("dor%d rdpa_cpu_reason_cfg_set(0x%px, 0x%px, 0x%px) returned %d\r\n",
	            dhd_hlp->dhd->unit, dhd_hlp->cpu_obj,  &cpu_reason, &reason_cfg, rc));
	        goto exit;
	    }
	}
#endif /* RDP */
#endif /* !CONFIG_BCM_DHD_ARCHER */

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
	dhd_pub_t* dhdp = dhd_hlp->dhd;

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

	    DHD_INFO(("%s: ATTACH_DMA_INDX_BUF<0x%08x> dma_buf<0x%px,0x%x_%x,%d>\n",
	        __FUNCTION__, buf_type, dma_buf->va, (uint32)PHYSADDRHI(dma_buf->pa),
	        (uint32)PHYSADDRLO(dma_buf->pa), dma_buf->len));

	    switch (buf_type) {
	        case ATTACH_R2D_WR_BUF:
	            dhd_init_cfg->r2d_wr_arr_base_addr = dma_buf->va;
	            RDPA_INIT_CFG_SET_PA(dhd_init_cfg->r2d_wr_arr_base_phys_addr,
	                dma_buf->pa, 0);
	            break;
	        case ATTACH_R2D_RD_BUF:
	            dhd_init_cfg->r2d_rd_arr_base_addr = dma_buf->va;
	            RDPA_INIT_CFG_SET_PA(dhd_init_cfg->r2d_rd_arr_base_phys_addr,
	                dma_buf->pa, 0);
	            break;
	        case ATTACH_D2R_WR_BUF:
	            {
	                ulong addr = (ulong)dma_buf->va + DHD_RNR_DMA_IDX_SZ(dhdp);

	                dhd_init_cfg->d2r_wr_arr_base_addr = (void*)addr;
	                RDPA_INIT_CFG_SET_PA(
	                    dhd_init_cfg->d2r_wr_arr_base_phys_addr,
	                    dma_buf->pa, dhdp->bus->rw_index_sz);
	            }
	            break;
	        case ATTACH_D2R_RD_BUF:
	            {
	                ulong addr = (ulong)dma_buf->va + DHD_RNR_DMA_IDX_SZ(dhdp);

	                dhd_init_cfg->d2r_rd_arr_base_addr = (void*)addr;
	                RDPA_INIT_CFG_SET_PA(
	                    dhd_init_cfg->d2r_rd_arr_base_phys_addr,
	                    dma_buf->pa, dhdp->bus->rw_index_sz);
	            }
	            break;
	        default:
	            DHD_ERROR(("%s: invalid DMA_INDX_BUF type<%d>\n",
	                __FUNCTION__, buf_type));
	            ASSERT(0);
	            return BCME_BADOPTION;
	    }

	    DHD_INFO(("%s: ATTACH_DMA_INDX_BUF<0x%08x> dma_buf<0x%px,0x%x_%x,%d>\n",
	        __FUNCTION__, buf_type, dma_buf->va, (uint32)PHYSADDRHI(dma_buf->pa),
	        (uint32)PHYSADDRLO(dma_buf->pa), dma_buf->len));


	} else if (buf_type & DETACH_DMA_INDX_BUF) {

	    DHD_INFO(("%s: DETACH_DMA_INDX_BUF<0x%08x> dma_buf<0x%px,0x%x_%x,%d>\n",
	        __FUNCTION__, buf_type, dma_buf->va, (uint32)PHYSADDRHI(dma_buf->pa),
	        (uint32)PHYSADDRLO(dma_buf->pa), dma_buf->len));

	    dhd_runner_free_mem(dhd_hlp, dma_buf, DHD_RNR_MEM_TYPE_RNR);

	} else if (buf_type & ATTACH_RING_BUF) {
	    uint32_t ringid = buf_type & 0xFFFF;
	    uint16_t def_max_items;
	    dhd_runner_ring_cfg_t *ring_cfg = NULL;

	    if (DHD_IS_TX_FLOWRING(dhdp, ringid)) {
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

	    /* Update dma_buf length information, if configured differently */
	    if (ringid == BCMPCIE_H2D_MSGRING_RXPOST_SUBMIT) {
	        ring_cfg = &dhd_hlp->rxpost_ring_cfg;
	        def_max_items = H2DRING_RXPOST_MAX_ITEM;
	    } else if (ringid == BCMPCIE_D2H_MSGRING_TX_COMPLETE) {
	        ring_cfg = &dhd_hlp->txsts_ring_cfg;
	        def_max_items = D2HRING_TXCMPLT_MAX_ITEM;
	    } else if (ringid == BCMPCIE_D2H_MSGRING_RX_COMPLETE) {
	        ring_cfg = &dhd_hlp->rxcmpl_ring_cfg;
	        def_max_items = D2HRING_RXCMPLT_MAX_ITEM;
	    }

	    /*
	     * Inform DHD of any update in the max_items using dma length
	     */
	    if (ring_cfg) {
	        if (((ring_cfg->cfgsts & DOR_CFGSTS_FMTCHG) == 0) ||
	            (dhd_hlp->sup_feat.cmnrngsz == 0)) {
	            /*
	             * Switch back to default max items as DHD or Runner does not
	             * support ring max items configuraion
	             */
	            if (ring_cfg->max_items != def_max_items) {
	                RLOG("dor%d ringid [%d] Using default max_items [%d]",
	                    dhd_hlp->dhd->unit, ringid, def_max_items);
	                ring_cfg->max_items = def_max_items;
	            }
	        } else if (dma_buf->len != ring_cfg->max_items * ring_cfg->size) {
	            /* max_items are different, update dma buffer length */
	            dma_buf->len = ring_cfg->max_items * ring_cfg->size;
	            dhd_hlp->en_feat.cmnrngsz = 1;
	        }
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

	    DHD_INFO(("%s: ATTACH_RING_BUF<0x%08x> dma_buf<0x%px,0x%x_%x,%d>\n",
	        __FUNCTION__, buf_type, dma_buf->va, (uint32)PHYSADDRHI(dma_buf->pa),
	        (uint32)PHYSADDRLO(dma_buf->pa), dma_buf->len));

	    /* Setup the allocation variables  */
	    switch (ringid) {
	        case BCMPCIE_H2D_MSGRING_RXPOST_SUBMIT:
	            dhd_init_cfg->rx_post_flow_ring_base_addr = dma_buf->va;
#if defined(DOL_HLPR_CAP_CMNRNGSZ)
	            dhd_init_cfg->rx_post_ring_size = ring_cfg->max_items;
#endif /* DOL_HLPR_CAP_CMNRNGSZ */
	            break;
	        case BCMPCIE_D2H_MSGRING_TX_COMPLETE:
	            if (DHD_RNR_TXSTS_OFFLOAD(dhd_hlp))
	                dhd_init_cfg->tx_complete_flow_ring_base_addr = dma_buf->va;
#if defined(DOL_HLPR_CAP_CMNRNGSZ)
	            dhd_init_cfg->tx_cmpl_ring_size = ring_cfg->max_items;
#endif /* DOL_HLPR_CAP_CMNRNGSZ */
	            break;
	        case BCMPCIE_D2H_MSGRING_RX_COMPLETE:
	            dhd_init_cfg->rx_complete_flow_ring_base_addr = dma_buf->va;
#if defined(DOL_HLPR_CAP_CMNRNGSZ)
	            dhd_init_cfg->rx_cmpl_ring_size = ring_cfg->max_items;
#endif /* DOL_HLPR_CAP_CMNRNGSZ */
	            break;

	        case BCMPCIE_H2D_MSGRING_CONTROL_SUBMIT:
	        case BCMPCIE_D2H_MSGRING_CONTROL_COMPLETE:
	            /* Runner does not need to registers control subn/cmplt rings */
	            return BCME_OK;

	        default:
		    if (DHD_IS_DYN_FLOWRING(dhdp, (uint16_t)ringid)) {
			return BCME_OK;
		    }
	            DHD_ERROR(("%s: ringid %d not supported in Runner\n",
	                __FUNCTION__, buf_type));
	            ASSERT(0);
	            return BCME_BADOPTION;
	    }
	} else if (buf_type & DETACH_RING_BUF) {
	    uint32_t ringid = buf_type & 0xFFFF;

	    DHD_INFO(("%s: DETACH_RING_BUF<0x%08x> dma_buf<0x%px,0x%x_%x,%d>\n",
	        __FUNCTION__, buf_type, dma_buf->va, (uint32)PHYSADDRHI(dma_buf->pa),
	        (uint32)PHYSADDRLO(dma_buf->pa), dma_buf->len));

	    if (DHD_IS_NON_TX_FLOWRING(dhdp, ringid)) {

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
	dhd_hlp->pci_dev = pci_dev;

	RLOG(CLRyk "%s DHD PCIE: vendor<0x%04x> device<0x%04x> bus<%d> slot<%d>" CLRnl,
	        DOL_HELPER,
	    pci_dev->vendor, pci_dev->device,
	    pci_dev->bus->number, PCI_SLOT(pci_dev->devfn));

	if (bcm_pcie_config_bar_addr(pci_dev, 0, dhd_hlp->pci_bar, SZ_64M) == 0) {
	    return BCME_OK;
	} else {
	    DHD_ERROR(("dol%d: failed to set PCIe Offload incoming address\n",
	        dhd_hlp->dhd->unit));
	    return BCME_ERROR;
	}
}

/**
 * Un-configure the PCIe BAR for Runner wakeup incoming address
 */
static void
dhd_runner_pcie_deinit(dhd_runner_hlp_t *dhd_hlp)
{
	if (bcm_pcie_config_bar_addr(dhd_hlp->pci_dev, 0, dhd_hlp->pci_bar, 0)) {
	    DHD_ERROR(("dol%d: failed to unconfig PCIe Offload incoming address\n",
	        dhd_hlp->dhd->unit));
	}

	return;
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
	    if (dhd_hlp->sup_feat.hwawkup == 0) {
	        DHD_ERROR(("dhd%d_rnr: Runner does not support iDMA\r\n",
	            dhd_hlp->dhd->unit));
	        return BCME_UNSUPPORTED;
	    }
	    /* Use PCIH2D_MailBox_2 (0x160) to wakeup dongle ARM (TXPOST) */
	    db_reg1 = (char*)bus->pcie_mb_intr_2_addr;

	    /* Use PCIH2D_DB1_2 (0x164) to wakeup dongle HWA (RXPOST, RXCPL, TXCPL) */
	    db_reg2 = (char*)bus->pcie_db1_intr_2_addr;

	    dhd_hlp->en_feat.hwawkup = 1;
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
#if defined(DOL_HLPR_CAP_HWA_WAKEUP)
	init_cfg->dongle_wakeup_register_2 = (uint32)(bar0_phys + (uint32)(db_reg2 - regs_base));
	init_cfg->dongle_wakeup_register_virt = (void*)db_reg1;
	init_cfg->dongle_wakeup_register_2_virt = (void*)db_reg2;
#if defined(BCMHWA)
	init_cfg->dongle_wakeup_hwa = IDMA_ENAB(dhd_hlp->dhd);

#if defined(DOL_HLPR_CAP_IDMAGRPCFG)
	init_cfg->idma_group_shift = 0;
	if (init_cfg->dongle_wakeup_hwa) {
	    uint16_t igs = 0;
	    uint16 flows = IDMA_FLOWS_PER_SET(dhd_hlp->dhd);

	    while (flows > 1) { igs++; flows >>= 1; }

	    init_cfg->idma_group_shift = igs;
	}
#endif /* DOL_HLPR_CAP_IDMAGRPCFG */

#if defined(DOL_HLPR_CAP_IDMA64)
	init_cfg->idma_num_of_groups = 0;
#if defined(IDMA_SETS)
	if (dhd_hlp->en_feat.idma64 == 1) {
	    init_cfg->idma_num_of_groups = IDMA_SETS(dhd_hlp->dhd);
	}
#endif /* IDMA_SETS */
#endif /* DOL_HLPR_CAP_IDMA64 */
#else /* !BCMHWA */
	init_cfg->dongle_wakeup_hwa = 0;
#endif /* !BCMHWA */
	RPR("dongle_wakeup_addr_2 = 0x%08x\r\n",
	    init_cfg->dongle_wakeup_register_2);
#endif /* DOL_HLPR_CAP_HWA_WAKEUP */

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
	if (llcsnaphdr && dhd_hlp->sup_feat.dhdhdr == 0) {
	    DHD_ERROR(("%s: Add LLCSNAP_HDR is not supported by runner\n",
	        __FUNCTION__));
	    return BCME_UNSUPPORTED;
	}

#if defined(DOL_HLPR_CAP_LLCSNAPHDR)
	dhd_hlp->dhd_init_cfg.add_llcsnap_header = llcsnaphdr;
	dhd_hlp->en_feat.dhdhdr = llcsnaphdr;
#endif /* DOL_HLPR_CAP_LLCSNAPHDR */
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
	    if (dhd_hlp->sup_feat.txcmpl2host == 0) {
	        return BCME_UNSUPPORTED;
	    }

#if defined(DOL_HLPR_CAP_TXCMPL2HOST)
	    rc = rdpa_dhd_helper_tx_complete_send2host_set(mo, TRUE);

	    if (rc < 0) {
	        DHD_ERROR(("dhd%d_rnr: set tx_complete_send2host failed %d\r\n",
	            dhd_hlp->dhd->unit, rc));
	        return BCME_ERROR;
	    }
	    dhd_hlp->en_feat.txcmpl2host = 1;
	    RPR2("rdpa_dhd_helper_tx_complete_send2host_set=TRUE\r\n");
#endif /* DOL_HLPR_CAP_TXCMPL2HOST */
	}

	return rc;
}

/**
 * Initialize ba256 configuration parameters
 *
 * Update RxP/RxC/TxP-BE-BQ sizes for Dongle BA256 mode if
 *  Supported by runner platform,
 *  enabled in dongle
 *  corresponding ring sizes in nvram are not updated by user
 *  override nvram setting is enabled if present
 *
 */
static int
dhd_runner_ba256cfg_init(dhd_runner_hlp_t *dhd_hlp)
{
	dhd_runner_flowmgr_t *flowmgr = &dhd_hlp->flowmgr;
	int unit = dhd_hlp->dhd->unit;
	int rc = BCME_OK;

	/*
	 * For non-Offload, DHD always supports BA256 configuration. For Offload
	 * check if all dependant features are supported by offload hardware,
	 * if offload is enabled
	 */
	if ((DOL_TXOL_EN(dhd_hlp) || DOL_RXOL_EN(dhd_hlp)) &&
	    ((dhd_hlp->sup_feat.ba256cfg == 0) ||
	    (dhd_hlp->sup_feat.cmnrngsz == 0) ||
	    (dhd_hlp->sup_feat.bkupq == 0))) {
	    rc = BCME_UNSUPPORTED;
	    RLOG("dor%d: BA256 can not support\n", unit);
	    goto done;
	}

	/*
	 * BA256 can be supported by runner, Update the ring configuration,
	 * if still default and is not set by user using NVRAM
	 */

	/* RX Complete ring size */
	if ((dhd_hlp->rxcmpl_ring_cfg.cfgsts & DOR_CFGSTS_ITMCHG) == 0) {
	    dhd_hlp->rxcmpl_ring_cfg.max_items = DOR_RXC_RINGSZ_BA256;
	}

	/* TxPost BE Backup queue size */
	if (flowmgr->def_profile_id == DHD_RNR_FLOWRING_DEFAULT_PROFILE_ID) {
	    flowmgr->def_profile_id = DOR_DEFAULT_PROFILE_ID_BA256;
	}

	rc = BCME_OK;
	RLOG("dor%d: BA256 updated Profile ID [%d], max_items RxP/RxC [%d]/[%d]\n",
	    unit, dhd_hlp->flowmgr.def_profile_id,
	    dhd_hlp->rxpost_ring_cfg.max_items, dhd_hlp->rxcmpl_ring_cfg.max_items);

done:
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
	/* Set the default values */
	dhd_hlp->sw_feat.cpuqdpc = DOL_SWFEAT_CPUQDPC_DEF;
	dhd_hlp->sw_feat.cmplnotif = DOL_SWFEAT_CMPLNOTIF_DEF;
	dhd_hlp->ipend = FALSE;

	/*
	 * Fetch the SW feature Configuration information of the radio,
	 * if present
	 */

	/* Process Rx,Tx complete Queue in dhd DPC context */
	DOL_SWFEAT_ENABLE_INIT(dhd_hlp, cpuqdpc);

	/* Notify Rx,Tx complete to runner */
	DOL_SWFEAT_ENABLE_INIT(dhd_hlp, cmplnotif);

	return 0;
}

/**
 * Initialize Runner Codel setting
*/
static int
dhd_runner_codel_init(struct dhd_runner_hlp *dhd_hlp)
{
#if defined(DOL_HLPR_CAP_CODEL)
	char buff[DHD_RNR_KEY_CODEL_STR_LEN];
	bdmf_boolean enable = false;
	int length = 0;
	int codel = -1;
	int rc = BDMF_ERR_OK;

	/* Fetch the Codel Configuration information of the radio, if present */
	memset(buff, 0, sizeof(buff));
	length = dhd_runner_key_get(dhd_hlp->dhd->unit, DHD_RNR_KEY_CODEL, buff,
	    sizeof(buff));

	if (length == 0) {
	    /* Codel nvram configuration is not present */
	    if (dhd_hlp->dhd_init_cfg.hbqd_mode == 1) {
	        /*
	         * When Codel is enabled, runner might trim the backup queue depth
	         * which is not expected by dongle when backup queue depth is
	         * exposed to dongle.
	         *
	         * disable codel in runner, until backup queue depth trimming is
	         * supported by dongle
	         */
	        codel = 0;
	    }
	} else {
	    sscanf(buff, "%d", &codel);
	}

	if (codel >= 0) {
	    enable = (codel) ? true : false;
	    rc = rdpa_dhd_helper_codel_enable_set(dhd_hlp->dhd_helper_obj, enable);
	    if (rc == 0) {
	        RLOG("dor%d: codel turned %s", dhd_hlp->dhd->unit,
	            (codel) ? "ON" : "OFF");
	    } else {
	        DHD_ERROR(("dor%d rdpa_dhd_helper_codel_enable_set failed rc: %d\n",
	            dhd_hlp->dhd->unit, rc));
	    }
	}

	rc = rdpa_dhd_helper_codel_enable_get(dhd_hlp->dhd_helper_obj, &enable);
	if (rc == BDMF_ERR_OK) {
	    dhd_hlp->en_feat.codel = enable;
	} else {
	    DHD_ERROR(("dor%d rdpa_dhd_helper_codel_enable_get failed rc: %d\n",
	        dhd_hlp->dhd->unit, rc));
	}

	return rc;
#else /* !DOL_HLPR_CAP_CODEL */
	return BDMF_ERR_OK;
#endif /* !DOL_HLPR_CAP_CODEL */
}


/**
 * Disable Offload (tx/rx) due to missing/unsupported features
 *
*/
static int
dhd_runner_force_disable_offload(struct dhd_runner_hlp *dhd_hlp, bool tx_dor,
	bool rx_dor)
{
	if ((tx_dor) && (DHD_RNR_TXSTS_OFFLOAD(dhd_hlp))) {

	    RLOG(CLRerr "Force disabling Runner Offload for TX" CLRnl);

	    dhd_hlp->en_feat.txoffl = 0;
	    dhd_hlp->en_feat.nplusm = 0;
	    dhd_hlp->txsts_ring_cfg.offload = 0;
	    dhd_hlp->txpost_ring_cfg.offload = 0;
	    dhd_hlp->flowmgr.hw_mem_addr = (void*)NULL;
	    dhd_hlp->flowmgr.hw_mem_size = 0;
	    dhd_hlp->dhd_init_cfg.tx_post_flow_ring_base_addr = (void *)NULL;
	    dhd_hlp->dhd_init_cfg.tx_complete_flow_ring_base_addr = (void*)NULL;
	}

	if ((rx_dor) && (DHD_RNR_RX_OFFLOAD(dhd_hlp))) {

	    RLOG(CLRerr "Force disabling Runner Offload for RX" CLRnl);

	    dhd_hlp->en_feat.rxoffl = 0;
	    dhd_hlp->rxcmpl_ring_cfg.offload = 0;
	    dhd_hlp->rxpost_ring_cfg.offload = 0;
	}

	return BCME_OK;
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
	BDMF_MATTR_ALLOC(dhd_helper_attr, rdpa_dhd_helper_drv());

	/* Initialization is not needed, if offload is not supported for this radio */
	if (!DHD_RNR_IS_OFFL_SUPPORTED(dhd_hlp)) {
	    DHD_INFO(("dhd%d_rnr offload not supported, skip initialization\r\n",
	        dhd_hlp->dhd->unit));
	    rc = BCME_OK;
	    goto exit;
	}

	if (!pci_dev) {
	    DHD_ERROR(("%s: unknown pci_dev\n", __FUNCTION__));
	    rc = BCME_NODEVICE;
	    goto exit;
	}

	if (!dhd_helper_attr) {
	    DHD_ERROR(("%s: failed to allocate dhd_helper_attr\n", __FUNCTION__));
	    rc = BCME_NOMEM;
	    goto exit;
	}

	if ((rc = dhd_runner_pcie_init(dhd_hlp, pci_dev)) != BCME_OK)
	    goto exit;

	if ((rc = dhd_runner_llcsnaphdr_init(dhd_hlp)) != BCME_OK)
	    goto exit;

	if ((rc = dhd_runner_txcmpl2host_init(dhd_hlp, dhd_helper_attr)) == BCME_ERROR)
	    goto exit;

	if ((rc = dhd_runner_dongle_wakeup_init(dhd_hlp)) != BCME_OK)
	    goto exit;

#if defined(DOL_HLPR_CAP_RXCMPLRCH) && defined(BCM_RXCPLE_RCH)
	dhd_hlp->dhd_init_cfg.rx_cmpl_rch_max_seg = dhd_hlp->dhd->max_rch_sdu_cnt;
#endif /* DOL_HLPR_CAP_RXCMPLRCH && BCM_RXCPLE_RCH */

#if IS_ENABLED(CONFIG_BCM_DHD_ARCHER)
	if (DHD_RNR_RX_OFFLOAD(dhd_hlp)) {
	    dhd_hlp->dhd_init_cfg.rxoffl = 1;
	} else {
	    dhd_hlp->dhd_init_cfg.rxoffl = 0;
	}

	if (DHD_RNR_TXSTS_OFFLOAD(dhd_hlp)) {
	    dhd_hlp->dhd_init_cfg.txoffl = 1;
	} else {
	    dhd_hlp->dhd_init_cfg.txoffl = 0;
	}
#endif
	rc = rdpa_dhd_helper_radio_idx_set(dhd_helper_attr, dhdp->unit);
	if (rc != 0) {
	    DHD_ERROR(("dor%d rdpa_dhd_helper_radio_idx_set(0x%px, %d) returned %d\r\n",
	        dhdp->unit, &dhd_helper_attr, dhdp->unit, rc));
	}
	rc = rdpa_dhd_helper_init_cfg_set(dhd_helper_attr, &dhd_hlp->dhd_init_cfg);
	if (rc != 0) {
	    DHD_ERROR(("dor%d rdpa_dhd_helper_init_cfg_set(0x%px, 0x%px) returned %d\r\n",
	        dhdp->unit, &dhd_helper_attr, &dhd_hlp->dhd_init_cfg, rc));
	}

	DHD_RNR_INIT_PERIM_UNLOCK(dhdp);

	rc = bdmf_new_and_set(rdpa_dhd_helper_drv(), NULL,
	                        dhd_helper_attr, &dhd_hlp->dhd_helper_obj);

	if (rc) {
	    DHD_ERROR(("%s: bdmf_new_and_set error %d\n", __FUNCTION__, rc));
	    DHD_RNR_INIT_PERIM_LOCK(dhdp);
	    rc = BCME_ERROR;
	    goto exit;
	}

	if ((rc = dhd_runner_codel_init(dhd_hlp)) != BDMF_ERR_OK) {
	    DHD_RNR_INIT_PERIM_LOCK(dhdp);
	    rc = BCME_ERROR;
	    goto exit;
	}

	rc = rdpa_dhd_helper_int_connect_set(dhd_hlp->dhd_helper_obj, true);

	if (rc) {
	    DHD_ERROR(("%s: Failed to connect interrupts, error %d\n", __FUNCTION__, rc));
	    DHD_RNR_INIT_PERIM_LOCK(dhdp);
	    rc = BCME_ERROR;
	    goto exit;
	}

	dhd_hlp->dhd_mcast_obj = NULL;

	if (!rc) {
	    rc = dhd_runner_cfg_cpu_queue(dhd_hlp, 1);
	    if (rc)
	        DHD_ERROR(("%s: dhd_runner_cfg_cpu_queue failed, rc = %d\n",
	            __FUNCTION__, rc));
	}

	DHD_RNR_INIT_PERIM_LOCK(dhdp);

#if IS_ENABLED(CONFIG_BCM_SPDSVC)
	dhd_runner_spdsvc_transmit = bcmFun_get(BCM_FUN_ID_SPDSVC_TRANSMIT);
	BCM_ASSERT(dhd_runner_spdsvc_transmit != NULL);
#endif

exit:
	if (dhd_helper_attr) {
	    BDMF_MATTR_FREE(dhd_helper_attr);
	}

	RLOG(CLRyk "%s DHD Offload initialization %s" CLRnl, DOL_HELPER,
	    (rc == 0) ? "Successful" : "Failed");
	return rc;
}

/*
 * +----------------------------------------------------------------------------
 *      Section: Attach and Detach
 * +----------------------------------------------------------------------------
 */
static INLINE int
dhd_runner_wakeup_init(dhd_runner_hlp_t *dhd_hlp,
	bcmpcie_soft_doorbell_t *soft_doorbell, phys_addr_t wakeup_paddr,
	uint32 wakeup_val32)
{
	struct pci_dev *pdev;

	if (soft_doorbell == NULL) {
	    /* If softdoorbells are not supported, runner can not get interrupts */
	    return BDMF_ERR_PARM;
	}

	pdev = osl_pci_device(dhd_hlp->dhd->osh);
	if (pdev != NULL) {
	    wakeup_paddr = bcm_pcie_map_bar_addr(pdev, wakeup_paddr, SZ_1M);
	}

	if (wakeup_paddr == BCM_PCIE_MAP_ADDR_INVALID) {
	    return BDMF_ERR_PARM;
	}

	/* Use 64MB incoming window for PCI */
	dhd_hlp->pci_bar = wakeup_paddr & ~(SZ_64M - 1);

#if defined(PHYS_ADDR_64BIT) || defined(CONFIG_PHYS_ADDR_T_64BIT)
	soft_doorbell->haddr.low_addr = (uint32_t)(wakeup_paddr & 0xffffffff);
	soft_doorbell->haddr.high_addr = (uint32_t)(wakeup_paddr >> 32);
#else /* !PHYS_ADDR_64BIT && !CONFIG_PHYS_ADDR_T_64BIT */
	soft_doorbell->haddr.low_addr = (uint32_t)wakeup_paddr;
	soft_doorbell->haddr.high_addr = 0U; /* 32bit host */
#endif /* !PHYS_ADDR_64BIT && !CONFIG_PHYS_ADDR_T_64BIT */
	soft_doorbell->value = htol32(wakeup_val32);

	return BDMF_ERR_OK;
}

static int
dhd_helper_attach(dhd_runner_hlp_t *dhd_hlp, void *dhd)
{
	char   buff[DHD_RNR_KEY_OFFL_OVERRIDE_STR_LEN];
	int    length = 0;
	uint32 sup_mask = 0;
	uint32 ovrd_mask = 0;
	uint32 radio_mask = 0;

	DHD_INFO(("%s: hlp<%px> dhd<%px>\n", __FUNCTION__, dhd_hlp, dhd));

	dhd_hlp->dhd = dhd;
	radio_mask = (1UL << dhd_hlp->dhd->unit);

	/*
	 * Create bitmask of default supported radios configuration by the
	 * Offload engine
	 *
	 * Radio# 0 .. DOL_MAX_CONCUR_OFFL_RADIOS-1
	 */
	{
	    int unit = 0;
	    while (unit < DOL_MAX_DHD_OFFL_RADIOS) {
	        ovrd_mask |= (1 << unit);
	        if (unit < DOL_MAX_CONCUR_OFFL_RADIOS) {
	            sup_mask |= (1 << unit);
	        }
	        unit++;
	    }
	}

	/*
	 * Fetch the offload override configuration information of the radio,
	 * if present
	 */
	memset(buff, 0, sizeof(buff));
	length = dhd_runner_key_get(DHD_RNR_RADIO_IDX_ALL,
	    DHD_RNR_KEY_OFFL_OVERRIDE, buff, sizeof(buff));

	if (length > 0) {
	    /* Offload override setting is present, Check the setting */

	    sscanf(buff, "%d", &ovrd_mask);

	    /*
	     * If Offload engine supports less than Max radios, the following two
	     * configurations are supported
	     *
	     * Radio# 0 .. DOL_MAX_CONCUR_OFFL_RADIOS-1 (existing sup_mask)
	     * Radio# 1 .. DOL_MAX_CONCUR_OFFL_RADIOS
	     */
#if (DOL_MAX_CONCUR_OFFL_RADIOS < DOL_MAX_DHD_OFFL_RADIOS)
	    if ((ovrd_mask & 0x1) == 0) {
	        /*
	         * switch to non-default configuration
	         * Radio# 1 .. DOL_MAX_CONCUR_OFFL_RADIOS
	         */
	        sup_mask <<= 1;
	    }
#endif /* (DOL_MAX_CONCUR_OFFL_RADIOS < DOL_MAX_DHD_OFFL_RADIOS) */
	}

	if (!(sup_mask & ovrd_mask & radio_mask)) {
	    /*
	     * Offload engine support is not enabled for this radio
	     * dhd dol instance still exists, to support api's
	     * but runner offload is disabled completely.
	     */
	    DHD_ERROR(("dol%d not supported, hw [0x%1x] nv [0x%1x]\n",
	        dhd_hlp->dhd->unit, sup_mask, ovrd_mask));
	    memset(&dhd_hlp->sup_feat, 0, sizeof(dol_hlpr_caps_t));
	    memset(&dhd_hlp->en_feat, 0, sizeof(dol_hlpr_caps_t));
	    return 0;
	}

	/* Base features are enabled by default */
	dhd_hlp->sup_feat.txoffl = 1;
	dhd_hlp->sup_feat.rxoffl = 1;

	/* Extra features are enabled conditionally */
#if defined(DOL_HLPR_CAP_TXCMPL2HOST)
	DOL_HLPR_CAP_SUPPORT_INIT(dhd_hlp, txcmpl2host);
#endif /* DOL_HLPR_CAP_TXCMPL2HOST */

#if defined(DOL_HLPR_CAP_LLCSNAPHDR)
	DOL_HLPR_CAP_SUPPORT_INIT(dhd_hlp, dhdhdr);
#endif /* DOL_HLPR_CAP_LLCSNAPHDR */

#if defined(DOL_HLPR_CAP_LBRAGGR)
	DOL_HLPR_CAP_SUPPORT_INIT(dhd_hlp, lbraggr);
#endif /* DOL_HLPR_CAP_LBRAGGR */

	dhd_hlp->sup_feat.nplusm = DOR_NPM_DEFAULT;
#if defined(DOL_HLPR_CAP_NPLUSM)
	DOL_HLPR_CAP_SUPPORT_INIT(dhd_hlp, nplusm);
#endif /* DOL_HLPR_CAP_NPLUSM */

	/* Enable legacy type for all rings MSGBUF_WI_WI64 */
	dhd_hlp->txpost_ring_cfg.sup_types = 0x1;
	dhd_hlp->rxpost_ring_cfg.sup_types = 0x1;
	dhd_hlp->txsts_ring_cfg.sup_types = 0x1;
	dhd_hlp->rxcmpl_ring_cfg.sup_types = 0x1;

#if defined(DOL_HLPR_CAP_MSGRINGFRMT)
	DOL_HLPR_CAP_SUPPORT_INIT(dhd_hlp, msgringformat);

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

#endif /* DOL_HLPR_CAP_MSGRINGFRMT */

	RLOG("dor%d: runner supported ring format types TxP 0x%x, RxP 0x%x TxC 0x%x RxC 0x%x",
	    dhd_hlp->dhd->unit,
	    dhd_hlp->txpost_ring_cfg.sup_types, dhd_hlp->rxpost_ring_cfg.sup_types,
	    dhd_hlp->txsts_ring_cfg.sup_types, dhd_hlp->rxcmpl_ring_cfg.sup_types);

#if defined(DOL_HLPR_CAP_BKUPQUEUE)
	DOL_HLPR_CAP_SUPPORT_INIT(dhd_hlp, bkupq);
#endif /* DOL_HLPR_CAP_BKUPQUEUE */

#if defined(DOL_HLPR_CAP_HWA_WAKEUP)
	DOL_HLPR_CAP_SUPPORT_INIT(dhd_hlp, hwawkup);
#endif /* DOL_HLPR_CAP_HWA_WAKEUP */

#if defined(DOL_HLPR_CAP_FFRD)
	DOL_HLPR_CAP_SUPPORT_INIT(dhd_hlp, ffrd);
#endif /* DOL_HLPR_CAP_FFRD */

#if defined(DOL_HLPR_CAP_HBQD)
	DOL_HLPR_CAP_SUPPORT_INIT(dhd_hlp, hbqd);
#endif /* DOL_HLPR_CAP_HBQD */

#if defined(DOL_HLPR_CAP_DYNBKUPQ)
	DOL_HLPR_CAP_SUPPORT_INIT(dhd_hlp, dynbkupq);
#endif /* DOL_HLPR_CAP_DYNBKUPQ */

#if defined(DOL_HLPR_CAP_CODEL)
	DOL_HLPR_CAP_SUPPORT_INIT(dhd_hlp, codel);
#endif /* DOL_HLPR_CAP_CODEL */

#if defined(DOL_HLPR_CAP_CMNRNGSZ)
	DOL_HLPR_CAP_SUPPORT_INIT(dhd_hlp, cmnrngsz);
#endif /* DOL_HLPR_CAP_CMNRNGSZ */

#if defined(DOL_HLPR_CAP_RXCMPLV2)
	DOL_HLPR_CAP_SUPPORT_INIT(dhd_hlp, rxcmplv2);
#endif /* DOL_HLPR_CAP_RXCMPLV2 */

#if defined(DOL_HLPR_CAP_RXCMPLRCH)
	DOL_HLPR_CAP_SUPPORT_INIT(dhd_hlp, rxcmplrch);
#endif /* DOL_HLPR_CAP_RXCMPLRCH */

#if defined(DOL_HLPR_CAP_BA256CFG)
	DOL_HLPR_CAP_SUPPORT_INIT(dhd_hlp, ba256cfg);
#endif /* DOL_HLPR_CAP_BA256CFG */

#if defined(DOL_HLPR_CAP_IDMA64)
	DOL_HLPR_CAP_SUPPORT_INIT(dhd_hlp, idma64);
#endif /* DOL_HLPR_CAP_IDMA64 */

#if defined(DOL_HLPR_CAP_IDMAGRPCFG)
	DOL_HLPR_CAP_SUPPORT_INIT(dhd_hlp, idmagrpcfg);
	if (dhd_hlp->sup_feat.idmagrpcfg == 1) {
	    dhd_hlp->en_feat.idmagrpcfg = 1;
	}
#endif /* DOL_HLPR_CAP_IDMAGRPCFG */

	return 0;
}

static void
dhd_helper_detach(dhd_runner_hlp_t *dhd_hlp)
{
	DHD_INFO(("%s: dhd_hlp<%px>\n", __FUNCTION__, dhd_hlp));

	dhd_runner_cfg_cpu_queue(dhd_hlp, 0); /* Unconfigure CPU Queue */

	if (dhd_hlp->dhd_helper_obj)
	    bdmf_destroy(dhd_hlp->dhd_helper_obj);
}

struct dhd_runner_hlp *
dhd_runner_attach(dhd_pub_t *dhd, bcmpcie_soft_doorbell_t *soft_doorbells)
{
	int rc;
	int old_trace_level = bdmf_global_trace_level;
	dhd_runner_hlp_t *dhd_hlp = NULL;
	bcmpcie_soft_doorbell_t *tx_soft_doorbell, *rx_soft_doorbell;
	rdpa_dhd_wakeup_info_t wakeup_info = {dhd->unit, 0, 0, 0, 0};
	DHD_INFO(("%s: dhd<%px>\n", __FUNCTION__, dhd));

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
	DHD_INFO(("[COHERENT_POOL_%d] Allocated 0x%x size pool 0x%px, 0x%x_%x\r\n",
	    dhd_hlp->dhd->unit, dhd_hlp->coherent_mem_pool.len,
	    dhd_hlp->coherent_mem_pool.va,
	    (uint32)PHYSADDRHI(dhd_hlp->coherent_mem_pool.pa),
	    (uint32)PHYSADDRLO(dhd_hlp->coherent_mem_pool.pa)));

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
	dhd_runner_rxoffl_init(dhd_hlp, &dhd_hlp->rxcmpl_ring_cfg,
	    &dhd_hlp->rxpost_ring_cfg);
	RLOG("%s: Rx Offload - %s, Ring Size = %d", __FUNCTION__,
	    (dhd_hlp->rxcmpl_ring_cfg.offload) ? "Enabled" : "Disabled",
	    dhd_hlp->rxcmpl_ring_cfg.max_items);

	if (DHD_RNR_IS_OFFL_SUPPORTED(dhd_hlp)) {
	    /* Get wakeup register information from runner */
	    rdpa_dhd_helper_wakeup_information_get(&wakeup_info);
	}

	if (DHD_RNR_TXSTS_OFFLOAD(dhd_hlp)) {
	    /* Setup Tx complete thread wake up doorbell */
	    rc = dhd_runner_wakeup_init(dhd_hlp, tx_soft_doorbell,
	        wakeup_info.tx_complete_wakeup_register,
	        HTOL32(wakeup_info.tx_complete_wakeup_value));
	    if (rc) {
	        DHD_ERROR(("dhd%d_rnr: failed to initialize runner tx wakeup",
	            dhd_hlp->dhd->unit));
	        DHD_ERROR((" doorbell = <0x%px> reg = <0x%llx>, val = <0x%08x>\r\n",
	            tx_soft_doorbell,
	            (uint64_t)wakeup_info.tx_complete_wakeup_register,
	            HTOL32(wakeup_info.tx_complete_wakeup_value)));
	        goto exit;
	    }
	    dhd_hlp->en_feat.txoffl = 1;
	    RLOG("TX wakeup info: reg = <0x%llx>, val = <0x%08x>",
	        (uint64_t)wakeup_info.tx_complete_wakeup_register,
	        HTOL32(wakeup_info.tx_complete_wakeup_value));
	}

	if (DHD_RNR_RX_OFFLOAD(dhd_hlp)) {
	    /* Setup Rx complete thread wake up doorbell */
	    rc = dhd_runner_wakeup_init(dhd_hlp, rx_soft_doorbell,
	        wakeup_info.rx_complete_wakeup_register,
	        HTOL32(wakeup_info.rx_complete_wakeup_value));
	    if (rc) {
	        DHD_ERROR(("dhd%d_rnr: failed to initialize runner rx wakeup",
	            dhd_hlp->dhd->unit));
	        DHD_ERROR((" doorbell = <0x%px> reg = <0x%llx>, val = <0x%08x>\r\n",
	            rx_soft_doorbell,
	            (uint64_t)wakeup_info.rx_complete_wakeup_register,
	            HTOL32(wakeup_info.rx_complete_wakeup_value)));
	        goto exit;
	    }
	    dhd_hlp->en_feat.rxoffl = 1;
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
	DHD_INFO(("%s: dhd_hlp<%px dhd<%px>\n", __FUNCTION__, dhd_hlp, dhd));

	if (dhd_hlp == NULL)
	    return;

	bdmf_set_nonblocking_log();
	dhd_helper_detach(dhd_hlp);

	dhd_runner_pcie_deinit(dhd_hlp);

	if (dhd_hlp->coherent_mem_pool.va != NULL) {
	    dhd_runner_free_mem(dhd_hlp, &dhd_hlp->coherent_mem_pool,
	        DHD_RNR_MEM_ALLOC_TYPE_COHERENT);
	    DHD_INFO(("[COHERENT_POOL_%d] Freed 0x%x size pool 0x%px\r\n",
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
	bdmf_clear_nonblocking_log();
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

	RPR2("%s: (0x%px, %d, %s, %s, %s) = %d", __FUNCTION__, flowmgr, prio,
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
	if (DHD_RNR_IF_ROLE_STA_OR_WDS(flowmgr->dhd_hlp->dhd, ifidx)) {
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

/**
 * Allocate access category based flowring and mark it offloaded or not
 *
 */
static int
dhd_runner_flowring_alloc(dhd_runner_flowmgr_t *flowmgr, dhd_wme_ac_t wme_ac,
	bool force_dhd)
{
	int ring_sz;
	uint16 flags, flow_id;
	void *id16_map_hndl;
	rdpa_dhd_flring_cache_t *rdpa_cache;
	dhd_runner_hlp_t *dhd_hlp;

	flow_id = id16_map_alloc(flowmgr->flow_ids_map);
	if (flow_id == FLOWID_INVALID) {
	    DHD_ERROR(("%s: Invalid flowid", __FUNCTION__));
	    return BCME_ERROR;
	}

	rdpa_cache = flowmgr->rdpa_flring_cache + flow_id;
	rdpa_cache->base_addr_high = 0U;
	flags = wme_ac << DHD_RNR_FLRING_WME_AC_SHIFT;

	dhd_hlp = flowmgr->dhd_hlp;
	ring_sz = flowmgr->phy_items[wme_ac] * dhd_hlp->txpost_ring_cfg.size;

	if ((flowmgr->hw_mem_size >= ring_sz) && (force_dhd == FALSE)) {
	    /* Runner managed txpost ring */
	    flags |= DHD_RNR_FLRING_IN_RUNNER_FLAG | DHD_RNR_FLRING_DISABLED_FLAG;
	    flowmgr->hw_mem_size -= ring_sz;
	    flowmgr->hw_ring_cnt[wme_ac]++;
	    id16_map_hndl = flowmgr->hw_id16_map[wme_ac];
	} else if (!(dhd_hlp->en_feat.txoffl ^ dhd_hlp->en_feat.nplusm)) {
	    flowmgr->sw_ring_cnt[wme_ac]++;
	    id16_map_hndl = flowmgr->sw_id16_map[wme_ac];
	} else {
	    DHD_ERROR(("dor%d: Not enough Reserved memory\n", dhd_hlp->dhd->unit));
	    return BCME_ERROR;
	}

	rdpa_cache->flags = htons(flags);
	id16_map_free(id16_map_hndl, flow_id);

	RPR2("%s: allocated %s flowring [%d], base address [%x_%8x] \r\n",
	    __FUNCTION__, (flags & DHD_RNR_FLRING_IN_RUNNER_FLAG) ? "HW" : "SW",
	    flow_id, rdpa_cache->base_addr_high, rdpa_cache->base_addr_low);

	return BCME_OK;
}

/**
 * Initialize the rdpa and dhd cahche information for the flowring
 *
 */
static int
dhd_runner_flowring_init(dhd_runner_flowmgr_t *flowmgr, uint16 flow_id)
{
	int ring_sz;
	uint16 flags;
	dhd_wme_ac_t wme_ac;
	rdpa_dhd_flring_cache_t *rdpa_cache;
	dhd_rdpa_flring_cache_t *dhd_cache;
	dhd_runner_hlp_t *dhd_hlp;

	dhd_hlp = flowmgr->dhd_hlp;
	rdpa_cache = flowmgr->rdpa_flring_cache + flow_id;
	dhd_cache = flowmgr->dhd_flring_cache + flow_id;

	flags = ntohs(rdpa_cache->flags);
	wme_ac = DHD_RNR_FLRING_WME_AC(flags);
	ring_sz = flowmgr->phy_items[wme_ac] * dhd_hlp->txpost_ring_cfg.size;

	/* Allocation flowring buffer */
	if (flags & DHD_RNR_FLRING_IN_RUNNER_FLAG) {
	    /*
	     * Runner managed txpost ring, use reserved memory
	     * Reserved memory address is alwasys in lower 2GByte range
	     */
#if IS_ENABLED(CONFIG_BCM_DHD_ARCHER)
	        rdpa_cache->base_ptr = (void*)flowmgr->hw_mem_addr;
#if !IS_ENABLED(CONFIG_BCM_DHD_CROSSBOW)
	        rdpa_cache->wr_idx = 0;
	        rdpa_cache->rd_idx = 0;
#endif /* !CONFIG_BCM_DHD_CROSSBOW */
#endif /* CONFIG_BCM_DHD_ARCHER */
	    rdpa_cache->base_addr_high = 0U;
	    rdpa_cache->base_addr_low = (uint32)(RDD_RSV_VIRT_TO_PHYS(
	        flowmgr->hw_mem_virt_base_addr, flowmgr->hw_mem_phys_base_addr,
	        flowmgr->hw_mem_addr));
	    dhd_cache->base_va = (void*)flowmgr->hw_mem_addr;
	    flowmgr->hw_mem_addr += ring_sz;
	} else {
	    /* DHD managed txpost ring, allocate dma buffer */
	    dhd_dma_buf_t dma_buf;

	    dma_buf.len = ring_sz;
	    dma_buf.va = dhd_runner_alloc_mem(dhd_hlp, &dma_buf,
	        DHD_RNR_MEM_TYPE_DHD);

	    if (!dma_buf.va) {
	        DHD_ERROR(("%s: dma_buf alloc for flow_id %d failure\n",
	            __FUNCTION__, flow_id));
	        return BCME_ERROR;
	    }

	    rdpa_cache->base_addr_low = PHYSADDRLO(dma_buf.pa);
	    rdpa_cache->base_addr_high = PHYSADDRHI(dma_buf.pa);
	    dhd_cache->base_va = dma_buf.va;
	}

#if defined(DOL_HLPR_CAP_BKUPQUEUE)
	rdpa_cache->phy_ring_size = htons(flowmgr->phy_items[wme_ac]);
#endif /* DOL_HLPR_CAP_BKUPQUEUE */

#if defined(DOL_HLPR_CAP_CODEL)
	rdpa_cache->window_ts = 0;
	rdpa_cache->drop_interval = 0;
	rdpa_cache->codel_flags = 0;
	rdpa_cache->codel_drop_counter = 0;
#endif /* DOL_HLPR_CAP_CODEL */

	/* items = physical ring size + backup queues if supported */
	rdpa_cache->items = htons(flowmgr->profile->items[wme_ac]);
	rdpa_cache->base_addr_low = htonl(rdpa_cache->base_addr_low);
	rdpa_cache->base_addr_high = htonl(rdpa_cache->base_addr_high);
	rdpa_cache->flags = htons(flags);

	ARCH_FLUSH_COHERENT((void *)rdpa_cache, sizeof(rdpa_dhd_flring_cache_t));

	RPR2("%s: allocated %s flowring [%d], base address [%x_%8x] \r\n",
	    __FUNCTION__, (flags & DHD_RNR_FLRING_IN_RUNNER_FLAG) ? "HW" : "SW",
	    flow_id, rdpa_cache->base_addr_high, rdpa_cache->base_addr_low);

	return BCME_OK;
}

/**
 * Initialize flowring id map based on the flowmgr allocation type
 *
 */
int
dhd_runner_flowring_map_init(dhd_runner_flowmgr_t *flowmgr)
{
	dhd_runner_hlp_t *dhd_hlp = flowmgr->dhd_hlp;
	dhd_pub_t *dhdp;
	uint16 flowid;
	dhd_wme_ac_t ac;
	void *flow_ids_map;
	int max_h2d_rings;
	int rc;

	DHD_TRACE(("%s\n", __FUNCTION__));

	dhdp = dhd_hlp->dhd;
	max_h2d_rings = flowmgr->max_h2d_rings;


	if (flowmgr->alloc_mode != FLOWID_ALLOC_WMEAC) {
	    rdpa_dhd_flring_cache_t *rdpa_cache;
	    uint16 flags;

	    /* Non access category based flowring map initialization */
	    if (flowmgr->alloc_mode == FLOWID_ALLOC_IDMA) {
	        flow_ids_map = dhd_idma_flowmgr_init(dhdp, max_h2d_rings);
	    } else {
	        flow_ids_map = id16_map_init(dhdp->osh,
	            max_h2d_rings - FLOW_RING_COMMON - MAX_DYN_FLOWRINGS(dhdp), FLOWID_RESERVED);
	    }

	    /* If failed to allocate flowid map, bail out */
	    if (flow_ids_map == NULL) {
	        DHD_ERROR(("dor%d: failed to allocate flow-id map\n", dhdp->unit));
	        goto error_rtn;
	    }
	    flowmgr->flow_ids_map = flow_ids_map;

	    /* Mark offloaded/non-offloaded in the rdpa flowring cache memory */
	    if (DHD_RNR_TXSTS_OFFLOAD(dhd_hlp)) {
	        flags = DHD_RNR_FLRING_DISABLED_FLAG;
	        flags |= DHD_RNR_FLRING_IN_RUNNER_FLAG;
	        for (flowid = FLOWID_RESERVED; flowid < (max_h2d_rings - MAX_DYN_FLOWRINGS(dhdp));
			flowid++)
	        {
	            rdpa_cache = flowmgr->rdpa_flring_cache + flowid;
	            rdpa_cache->flags = htons(flags);
	        }
	    }
	} else {
	    /* Access Category based flowring map initialization */
	    uint16 total_ids;
	    int rings_to_allocate[wme_ac_max + 1];  /* reused for per tid, too */
	    int weighted_rings_to_allocate = 0;

	    /* Populate the flowids that will be used from 2..(max - dynamic rings) */
	    total_ids = max_h2d_rings - FLOW_RING_COMMON - MAX_DYN_FLOWRINGS(dhdp);
	    flowmgr->flow_ids_map = id16_map_init(dhdp->osh, total_ids,
	        FLOWID_RESERVED);
	    if (flowmgr->flow_ids_map == NULL) {
	        DHD_ERROR(("dor%d: flow_ids_map init failure\n", dhdp->unit));
	        goto error_rtn;
	    }

#if defined(BCM_DBG_ID16)
#error "remove ASSERT val16 or use total_ids in id16_map_init below"
#endif

	    DHD_INFO(("dor%d: allocated flow_ids_map %p, ids %d\n", dhdp->unit,
	        flowmgr->flow_ids_map, total_ids));

	    /*
	     * id16 map allocators for ucast rings per access category
	     * for N stations
	     */
	    for (ac = wme_ac_bk; ac < wme_ac_max; ac++) {
	        rings_to_allocate[ac] = flowmgr->max_uc_rings;
	        flowmgr->hw_id16_map[ac] =
	            id16_map_init(dhdp->osh, flowmgr->max_uc_rings, ID16_INVALID);
	        flowmgr->sw_id16_map[ac] =
	            id16_map_init(dhdp->osh, flowmgr->max_uc_rings, ID16_INVALID);
	        if (dhd_hlp->en_feat.nplusm == 0) {
	            flowmgr->profile->weight[ac] = DHD_RNR_PER_AC_PROFILE_BUDGET;
	        }
	    }

	    /* id16 map allocators for BCMC rings (one per BSS) */
	    rings_to_allocate[wme_ac_bcmc] = flowmgr->max_bss;

	    flowmgr->hw_id16_map[wme_ac_bcmc] =
	        id16_map_init(dhdp->osh, flowmgr->max_bss, ID16_INVALID);
	    flowmgr->sw_id16_map[wme_ac_bcmc] =
	        id16_map_init(dhdp->osh, flowmgr->max_bss, ID16_INVALID);

	    /* Allocate rings with 0 weight (forced non-offload) in DHD memory */
	    for (ac = wme_ac_bk; ac <= wme_ac_max; ac++)
	    {
	        if (flowmgr->profile->weight[ac] == 0) {
	            while (rings_to_allocate[ac]) {
	                rc = dhd_runner_flowring_alloc(flowmgr, ac, TRUE);
	                if (rc == BCME_ERROR) {
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
	                rc = dhd_runner_flowring_alloc(flowmgr, ac, FALSE);
	                if (rc == BCME_ERROR) {
	                    DHD_ERROR(("Failed to allocate %d %s ucast rings",
	                        rings_to_allocate[ac], dhd_wme_ac_str[ac]));
	                    goto error_rtn;
	                }
	                rings_to_allocate[ac]--;
	                if (--ac_budget == 0)
	                    break; /* rest of rings_to_allocate[ac] in 2nd pass */
	            }
	        } else {
	            weighted_rings_to_allocate += rings_to_allocate[ac];
	        }
	    }

	    /* 2nd pass: -1 weight AC leftover from DHD_RNR_PER_AC_PROFILE_BUDGET */
	    for (ac = wme_ac_bk; ac < wme_ac_max; ac++)
	    {
	        if (flowmgr->profile->weight[ac] == -1) {
	            while (rings_to_allocate[ac]) {
	                rc = dhd_runner_flowring_alloc(flowmgr, ac, FALSE);
	                if (rc == BCME_ERROR) {
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
	                rc = dhd_runner_flowring_alloc(flowmgr, ac, FALSE);
	                if (rc == BCME_ERROR) {
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
	        rc = dhd_runner_flowring_alloc(flowmgr, wme_ac_bcmc, FALSE);
	        if (rc == BCME_ERROR) {
	            DHD_ERROR(("Failed to allocate %d bcmc rings",
	                rings_to_allocate[wme_ac_bcmc]));
	            goto error_rtn;
	        }
	        rings_to_allocate[wme_ac_bcmc]--;
	    }

	    /* By now the flow_ids_map should be empty */
	    flowmgr->flow_ids_map = id16_map_fini(dhdp->osh, flowmgr->flow_ids_map);
	}

	return BCME_OK;

error_rtn:
	return BCME_ERROR;
}

void *
dhd_runner_flowmgr_init(dhd_runner_hlp_t *dhd_hlp, int max_h2d_rings,
	int max_bss)
{
	int ac;
	int rings_to_allocate[wme_ac_max + 1];  /* reused for per tid, too */
	dhd_runner_flowmgr_t *flowmgr;
	dhd_pub_t *dhdp;
	dhd_dma_buf_t *dma_buf;
	rdpa_dhd_init_cfg_t *dhd_init_cfg;
	uint16 flow_id;

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
		(MAX_TX_FLOWRINGS(dhdp, max_h2d_rings) - max_bss) / wme_ac_max;

	ASSERT(max_h2d_rings == ((flowmgr->max_uc_rings * wme_ac_max)
			+ BCMPCIE_H2D_COMMON_MSGRINGS + max_bss + MAX_DYN_FLOWRINGS(dhdp)));

	/* Pick the flow ring selector. */
	flowmgr->select_fn = dhd_runner_flowring_select_policy;
	flowmgr->dhd_hlp = dhd_hlp;

	/* Setup the DMA-able buffer for flring cache in uncached memory. */
	dma_buf = &dhd_hlp->flring_cache_dma_buf;
	if (dma_buf->va == NULL) {
	    dma_buf->len = sizeof(rdpa_dhd_flring_cache_t) * max_h2d_rings;
	    dma_buf->len += sizeof(dhd_rdpa_flring_cache_t) * max_h2d_rings;
	    dma_buf->len = ALIGN_SIZE(dma_buf->len, L1_CACHE_BYTES);

	    dma_buf->va = dhd_runner_alloc_mem(dhd_hlp, dma_buf,
	        DHD_RNR_MEM_TYPE_RNR);

	    if (!dma_buf->va) {
	        DHD_ERROR(("%s: flring_cache_dma_buf len<%d> alloc failure\n",
	            __FUNCTION__, dma_buf->len));
	        goto error_rtn;
	    }

	    /* Register with runner */
	    dhd_init_cfg->tx_post_mgmt_arr_base_addr = (void *)(dma_buf->va);
	    RDPA_INIT_CFG_SET_PA(dhd_init_cfg->tx_post_mgmt_arr_base_phys_addr,
	        dma_buf->pa, 0);
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
	    RLOG("%s: dhd%d_rnr bootmem addr<%px> size<%u>", __FUNCTION__,
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

	/* id16 map allocators for ucast rings per access category for N stations */
	for (ac = wme_ac_bk; ac < wme_ac_max; ac++) {
	    rings_to_allocate[ac] = flowmgr->max_uc_rings;
	}
	/* id16 map allocators for BCMC rings (one per BSS) */
	rings_to_allocate[wme_ac_bcmc] = flowmgr->max_bss;

	/* Check if full dor memory available if n+m is not enabled */
	if (dhd_hlp->en_feat.txoffl == 1) {
	    uint32 ring_sz = 0;
	    uint32 full_dor_sz = 0;
	    bool force_m = FALSE;

	    for (ac = wme_ac_bk; ac <= wme_ac_max; ac++) {
	        ring_sz = flowmgr->phy_items[ac] * dhd_hlp->txpost_ring_cfg.size;
	        full_dor_sz += ring_sz * rings_to_allocate[ac];
	        if (flowmgr->profile->weight[ac] == 0) force_m = TRUE;
	    }

	    RLOG(CLRyk "dor%d TxOffl memory: Reserved<%d Mb> Needed<%d Mb>" CLRnl,
	        dhd_hlp->dhd->unit, B2MB(flowmgr->hw_mem_size), B2MB(full_dor_sz));

	    if (dhd_hlp->en_feat.nplusm == 0) {
	        if (flowmgr->hw_mem_size < full_dor_sz) {
	            RLOG(CLRerr "================================================");
	            RLOG("dor%d TxOffl memory < Full Offload memory, aborting",
	                dhd_hlp->dhd->unit);
	            RLOG("=================================================" CLRnl);
	            goto error_rtn;
	        }
	        if (force_m == TRUE) {
	            RLOG(CLRerr "================================================");
	            RLOG("dor%d profile N+M settings not supported, aborting",
	                dhd_hlp->dhd->unit);
	            RLOG("=================================================" CLRnl);
	            goto error_rtn;
	        }
	    } else if ((flowmgr->hw_mem_size >= full_dor_sz) && (force_m == FALSE)) {
	        dhd_hlp->en_feat.nplusm = 0;
	    }
	}

	/*
	 * Determine which flow ring id allocation mode to be used.
	 *
	 * Use AC based allocation if N+M or flowrings have different sizes
	 * Otherwise use iDMA mode if dongle suppots iDMA
	 * Else use legacy flat mode
	 */
	flowmgr->alloc_mode = FLOWID_ALLOC_DEFAULT;
	if (dhd_hlp->en_feat.nplusm == 0) {
	    uint16 ring_phy_size;

	    flowmgr->alloc_mode = FLOWID_ALLOC_LEGACY;
	    if (IDMA_ACTIVE(dhdp)) {
	        flowmgr->alloc_mode = FLOWID_ALLOC_IDMA;
	    }

	    ring_phy_size = flowmgr->phy_items[wme_ac_bk];
	    for (ac = wme_ac_bk; ac <= wme_ac_max; ac++) {
	        if (ring_phy_size != flowmgr->phy_items[ac]) {
	            flowmgr->alloc_mode = FLOWID_ALLOC_WMEAC;
	            break;
	        }
	    }
	}
	RLOG("dor%d: using %s based flowid allocation", dhdp->unit,
	    flowid_alloc_mode_str[flowmgr->alloc_mode]);

	/* Initialize flowring id map with offload status */
	if (dhd_runner_flowring_map_init(flowmgr) != BCME_OK)
	    goto error_rtn;

	/*
	 * Allocate dma buffers for flow rings and fill information needed
	 *  - DHD needs dma buffer virtual address
	 *  - Runner needs physical address, AC, Offloaded, Enabled
	 */
	for (flow_id = FLOWID_RESERVED;
		 flow_id < (flowmgr->max_h2d_rings - MAX_DYN_FLOWRINGS(dhdp)); flow_id++)
	{
	    if (dhd_runner_flowring_init(flowmgr, flow_id) != BCME_OK)
	        goto error_rtn;
	}

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
	    for (flring = FLOWID_RESERVED;
		flring < (flowmgr->max_h2d_rings - MAX_DYN_FLOWRINGS(dhdp)); flring++)
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
#if defined(DOL_HLPR_CAP_BKUPQUEUE)
	            rdpa_cache->phy_ring_size = 0;
#endif /* DOL_HLPR_CAP_BKUPQUEUE */
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

	if (flowmgr->alloc_mode == FLOWID_ALLOC_IDMA) {
	    flowmgr->flow_ids_map =
	        dhd_idma_flowmgr_fini(dhdp, flowmgr->flow_ids_map);
	} else if (flowmgr->alloc_mode == FLOWID_ALLOC_LEGACY) {
	    flowmgr->flow_ids_map =
	        id16_map_fini(dhdp->osh, flowmgr->flow_ids_map);
	} else {
	    flowmgr->flow_ids_map = id16_map_fini(dhdp->osh, flowmgr->flow_ids_map);

	    for (ac = wme_ac_bk; ac <= wme_ac_max; ac++) {
	        flowmgr->hw_id16_map[ac] =
	            id16_map_fini(dhdp->osh, flowmgr->hw_id16_map[ac]);
	        flowmgr->sw_id16_map[ac] =
	            id16_map_fini(dhdp->osh, flowmgr->sw_id16_map[ac]);
	    }
	}

done:
	return NULL;
}

uint16
dhd_runner_flowmgr_alloc(void *mgr,
	int ifidx, int prio, uint8 *mac, bool d11ac, bool *is_hw_ring)
{
	dhd_runner_flowmgr_t *flowmgr = (dhd_runner_flowmgr_t *)mgr;
	dhd_pub_t *dhdp;
	uint16 staid = ID16_INVALID;
	uint16 ringid = ID16_INVALID;

	/* Get the dhd helper object from the flowmgr address */
	ASSERT(mgr != NULL);
	ASSERT(flowmgr->dhd_hlp != NULL);
	ASSERT(flowmgr->dhd_hlp->dhd != NULL);

	dhdp = flowmgr->dhd_hlp->dhd;

	if (flowmgr->alloc_mode == FLOWID_ALLOC_IDMA) {
	    ringid = dhd_idma_flowmgr_alloc(flowmgr->flow_ids_map, mac);
	    *is_hw_ring = (DHD_RNR_TXSTS_OFFLOAD(flowmgr->dhd_hlp)) ? TRUE : FALSE;
	} else if (flowmgr->alloc_mode == FLOWID_ALLOC_LEGACY) {
	    ringid = id16_map_alloc(flowmgr->flow_ids_map);
	    *is_hw_ring = (DHD_RNR_TXSTS_OFFLOAD(flowmgr->dhd_hlp)) ? TRUE : FALSE;
	} else {
	    staid = dhd_get_sta_cnt(dhdp, ifidx, mac);

	    ringid = flowmgr->select_fn(flowmgr, ifidx, prio, mac, staid, d11ac,
	        is_hw_ring);
	}

	/* update flags in flowring cache for offloaded ring */
	if ((ringid != ID16_INVALID) && (*is_hw_ring == TRUE)) {
	    rdpa_dhd_flring_cache_t *rdpa_cache;
	    uint16 flags;

	    rdpa_cache = flowmgr->rdpa_flring_cache + ringid;
	    flags = ntohs(rdpa_cache->flags);
	    if (flags & DHD_RNR_FLRING_IN_RUNNER_FLAG) {
#if defined(FLOW_RING_FLAG_SSID_MASK)
	        /* update ssid ifidx in flowring cache for offloaded ring */
	        flags &= ~FLOW_RING_FLAG_SSID_MASK;
	        flags |= (ifidx << FLOW_RING_FLAG_SSID_SHIFT);
#endif /* FLOW_RING_FLAG_SSID_MASK */
	        /* update total items and wme_ac in flowring cache for the ring */
	        {
	            dhd_wme_ac_t wme_ac;

	            /* WMEAC allocation with STA/WDS use prio instead of mac */
	            if (ETHER_ISMULTI(mac) &&
	                !((flowmgr->alloc_mode == FLOWID_ALLOC_WMEAC) &&
	                (DHD_RNR_IF_ROLE_STA_OR_WDS(dhdp, ifidx))))
	                wme_ac = wme_ac_bcmc;
	            else
	                wme_ac = WME_PRIO2AC(prio);
	            flags &= ~DHD_RNR_FLRING_WME_AC_MASK;
	            flags |= (wme_ac << DHD_RNR_FLRING_WME_AC_SHIFT);
	            rdpa_cache->items = htons(flowmgr->profile->items[wme_ac]);
	        }
	        rdpa_cache->flags = htons(flags);
	        ARCH_FLUSH_COHERENT((void *)rdpa_cache,
	            sizeof(rdpa_dhd_flring_cache_t));
	    }
	}

	return ringid;
}

int
dhd_runner_flowmgr_free(void *mgr, uint16 flow_id)
{
	dhd_runner_flowmgr_t *flowmgr = (dhd_runner_flowmgr_t *)mgr;

	if (flowmgr->alloc_mode == FLOWID_ALLOC_IDMA) {
	    dhd_idma_flowmgr_free(flowmgr->flow_ids_map, flow_id);
	} else if (flowmgr->alloc_mode == FLOWID_ALLOC_LEGACY) {
	    id16_map_free(flowmgr->flow_ids_map, flow_id);
	} else {
	    int wme_ac;
	    void * id_map;
	    rdpa_dhd_flring_cache_t *rdpa_cache;
	    uint16 flags;

	    rdpa_cache = flowmgr->rdpa_flring_cache + flow_id;
	    flags = ntohs(rdpa_cache->flags);
	    wme_ac = (flags & DHD_RNR_FLRING_WME_AC_MASK)
	          >> DHD_RNR_FLRING_WME_AC_SHIFT;
	    id_map = (flags & DHD_RNR_FLRING_IN_RUNNER_FLAG) ?
	        flowmgr->hw_id16_map[wme_ac] : flowmgr->sw_id16_map[wme_ac];

	    DHD_TRACE(("Free %s Flowid %d map = 0x%px, ac=%d\r\n",
	        (flags & DHD_RNR_FLRING_IN_RUNNER_FLAG) ? "HW" : "SW",
	        flow_id, id_map, wme_ac));
	    RPR("Free %s Flowid %d map = 0x%px, ac=%d\r\n",
	        (flags & DHD_RNR_FLRING_IN_RUNNER_FLAG) ? "HW" : "SW",
	        flow_id, id_map, wme_ac);

	    id16_map_free(id_map, flow_id);
	}

	return BCME_OK;
}

/*
 * +----------------------------------------------------------------------------
 *      Section: Interface between DHD and Runner
 * +----------------------------------------------------------------------------
 */

static void
dhd_runner_update_flring_bqsize(dhd_runner_hlp_t *dhd_hlp, uint16_t ringid,
	uint32_t bqsize)
{
#if defined(DOL_HLPR_CAP_DYNBKUPQ)
	rdpa_dhd_flring_cache_t *flring;
	dhd_wme_ac_t wme_ac;
	uint16_t flags;
	bool notify = true;
	bool direct = false;
	uint32_t rnr_bqsize;

	if (bqsize & FLRING_BQSIZE_NOTIF_GROUP) {
	    /* More notify coming on the way, defer notifying runner */
	    notify = false;
	    bqsize &= ~FLRING_BQSIZE_NOTIF_GROUP;
	}

	if (bqsize & FLRING_BQSIZE_NOTIF_DIRECT) {
	    /* No normalization use direct values */
	    direct = true;
	    bqsize &= ~FLRING_BQSIZE_NOTIF_DIRECT;
	}


	if (ringid < ID16_INVALID) {
	    /* Notification for single flow-ring with backup queue size */
	    if (!dhd_hlp->flring_cache_dma_buf.va) {
	        DHD_ERROR(("%s: rdpa_flring_cache is not initialized\n",
	            __FUNCTION__));
	        return;
	    }

	    flring = dhd_hlp->flowmgr.rdpa_flring_cache;
	    if (!flring) {
	        DHD_ERROR(("%s: rdpa_flring_cache flowid<%d> not initialized\n",
	            __FUNCTION__, ringid));
	        return;
	    }

	    flring += ringid;
	    flags = ntohs(flring->flags);

	    if (!(flags & DHD_RNR_FLRING_IN_RUNNER_FLAG)) {
	        DHD_INFO(("%s: SW ring %d, no need to update runner\n",
	            __FUNCTION__, ringid));
	        return;
	    }

	    wme_ac = DHD_RNR_FLRING_WME_AC(flags);
	    rnr_bqsize = bqsize;

	    if (direct == false) {
	        /* Use the same ratio of backup queue size reduction for each AC
	         *
	         * items = [(items - phy_items) * (bqsize)]/max_bqsize + phy_items
	         *
	         * items = runner backup queue size  + physical ring size (phy_items)
	         * bqsize = station backup queue size in DHD
	         * max_bqsize = maximum stations backup queue size in DHD
	         */
	        rnr_bqsize *= dhd_hlp->flowmgr.profile->bqsize[wme_ac];
	        rnr_bqsize /= DHD_RNR_TXPOST_MAX_ITEM;
	    }
	    rnr_bqsize += dhd_hlp->flowmgr.phy_items[wme_ac];
	    flring->items = htons(rnr_bqsize);

	    ARCH_FLUSH_COHERENT((void *)flring, sizeof(rdpa_dhd_flring_cache_t));
	}

	/* Inform Runner of the backup queue update */
	if (notify == true) {
	    rdpa_dhd_msg_data_t data;
	    int rc;

	    if (ringid < ID16_INVALID) {
	        data.flowring_idx = ringid;
	        data.idx_valid = 1;
	    } else {
	        data.flowring_idx = 0;
	        data.idx_valid = 0;
	    }
	    rc = rdpa_dhd_helper_flow_ring_update_set(
	            dhd_hlp->dhd_helper_obj, (bdmf_number)data.u32);

	    if (rc != 0) {
	        DHD_ERROR(("dor%d %s (0x%px, 0x%x) returned %d\r\n",
	            dhd_hlp->dhd->unit, "rdpa_dhd_helper_flow_ring_BQSIZE_set",
	            &dhd_hlp->dhd_helper_obj, data.u32, rc));
	    }
	}
#endif /* DOL_HLPR_CAP_DYNBKUPQ */

	return;
}

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
	    DHD_ERROR(("dor%d rdpa_dhd_helper_flow_ring_enable_set(0x%px, %d, %d) returned %d\r\n",
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

	DHD_INFO(("%s: H2R dhd_hlp<%px> ops<%d> <0x%lx> <0x%lx>\n",
	    __FUNCTION__, dhd_hlp, ops, arg1, arg2));

	if ((dhd_hlp == NULL) || (dhd_hlp->dhd == NULL)) {
	    DHD_ERROR(("%s: invalid arg dhd_hlp<%px>\n", __FUNCTION__, dhd_hlp));
	    return BCME_BADARG;
	}

	dhd = dhd_hlp->dhd;

	switch (ops) {

	    /* Host requests Runner to read DMA Index buffer */
	    case H2R_IDX_BUF_RD_REQUEST: /* arg1:buffer ptr, arg2:read value ptr */
	        DHD_TRACE(("H2R_IDX_BUF_RD_REQUEST ptr<0x%px> valptr<0x%px>\n",
	            (uint16*)arg1, (uint32*)arg2));
	        if (DHD_RNR_DMA_IDX_SZ(dhd) == sizeof(uint32)) {
	            uint32* ptr = (uint32*)arg1;
	            uint32 val;

	            if (DHD_RNR_MEM_TYPE_RNR != DHD_RNR_MEM_ALLOC_TYPE_COHERENT) {
	                OSL_CACHE_INV((void *)ptr, sizeof(uint32));
	            }
	            val = LTOH32(*(ptr));
	            *(uint32*)arg2 = val;
	        } else {
	            uint16* ptr = (uint16*)arg1;
	            uint16 val;

	            if (DHD_RNR_MEM_TYPE_RNR == DHD_RNR_MEM_ALLOC_TYPE_COHERENT) {
	                ARCH_RD_DMA_IDX(ptr, val);
	            } else {
	                OSL_CACHE_INV((void *)ptr, sizeof(uint16));
	                val = LTOH16(*(ptr));
	            }
	            *(uint16*)arg2 = val;
	        }
	        break;

	    /* Host requests Runner to write DMA Index buffer */
	    case H2R_IDX_BUF_WR_REQUEST: /* arg1:buffer ptr, arg2:write value */
	        DHD_TRACE(("H2R_IDX_BUF_WR_REQUEST ptr<0x%px> val<0x%d>\n",
	            (uint16*)arg1, (uint32)arg2));
	        if (DHD_RNR_DMA_IDX_SZ(dhd) == sizeof(uint32)) {
	            uint32* ptr = (uint32*)arg1;
	            uint32  val = (uint32)arg2;

	            if (DHD_RNR_MEM_TYPE_RNR == DHD_RNR_MEM_ALLOC_TYPE_COHERENT) {
	                iowrite32(val, ptr);
	            } else {
	                *ptr = htol32(val);
	                OSL_CACHE_FLUSH((void *)ptr, sizeof(uint32));
	            }
	        } else {
	            uint16* ptr = (uint16*)arg1;
	            uint16  val = (uint16)arg2;

	            if (DHD_RNR_MEM_TYPE_RNR == DHD_RNR_MEM_ALLOC_TYPE_COHERENT) {
	                ARCH_WR_DMA_IDX(ptr, val);
	            } else {
	                *ptr = htol16(val);
	                OSL_CACHE_FLUSH((void *)ptr, sizeof(uint16));
	            }
	        }
	        break;

	    /* Host notifies Runner to post initial buffers */
	    case H2R_RXPOST_NOTIF:
	        DHD_TRACE(("H2R_RXPOST_NOTIF\n"));

	        if (!DHD_RNR_IS_RX_OFFL_SUPPORTED(dhd_hlp))
	            return BCME_UNSUPPORTED;

	        bdmf_set_nonblocking_log();
	        if (rdpa_dhd_helper_rx_post_init(dhd_hlp->dhd_helper_obj)) {
	            DHD_ERROR(("rdpa_dhd_helper_rx_post_init(0x%px) failed\r\n",
	                dhd_hlp->dhd_helper_obj));
	            rc = BCME_NOMEM;
	        }
		else
	            rc = BCME_OK;

	        bdmf_clear_nonblocking_log();
		return rc;

	        break;

	    /* Host notifies Runner to free buffers in RxPost */
	    case H2R_RXPOST_FREE_NOTIF:
	        DHD_TRACE(("H2R_RXPOST_FREE_NOTIF\n"));

	        if (!DHD_RNR_IS_RX_OFFL_SUPPORTED(dhd_hlp))
	            return BCME_UNSUPPORTED;

	        bdmf_set_nonblocking_log();
	        if (rdpa_dhd_helper_rx_post_uninit(dhd_hlp->dhd_helper_obj)) {
	            DHD_ERROR(("rdpa_dhd_helper_rx_post_uninit(0x%px) failed\r\n",
	                dhd_hlp->dhd_helper_obj));
	            rc = BCME_ERROR;
	        }
		else
	            rc = BCME_OK;

	        bdmf_clear_nonblocking_log();
		return rc;

	        break;

	    /* Host notifies Runner to re-initialize buffers in RxPost */
	    case H2R_RXPOST_REINIT_NOTIF:
	        DHD_TRACE(("H2R_RXPOST_REINIT_NOTIF\n"));

	        if (!DHD_RNR_IS_RX_OFFL_SUPPORTED(dhd_hlp))
	            return BCME_UNSUPPORTED;

	        if (rdpa_dhd_helper_rx_post_reinit(dhd_hlp->dhd_helper_obj)) {
	            DHD_ERROR(("rdpa_dhd_helper_rx_post_uninit(0x%px) failed\r\n",
	                dhd_hlp->dhd_helper_obj));
	            return BCME_ERROR;
	        }
	        break;

	    /* Host notifies Runner to process D2H RxCompl */
	    case H2R_RX_COMPL_NOTIF:
	        DHD_TRACE(("H2R_RX_COMPL_NOTIF, unit %d\n", dhd_hlp->dhd->unit));
	        RPR2("H2R_RX_COMPL_NOTIF");

	        if ((dhd_hlp->ipend == FALSE) && (dhd_hlp->sw_feat.cmplnotif == TRUE)) {
	            rdpa_dhd_helper_complete_wakeup(dhd_hlp->dhd->unit, FALSE);
	            dhd_hlp->h2r_rx_compl_notif++;
	        }
	        break;


	    /* Host notifies Runner to transmit a packet */
	    case H2R_TXPOST_NOTIF: /* arg1:pktptr arg2:ifid */
	        DHD_TRACE(("H2R_TXPOST_NOTIF pkt<0x%px> len<%d> flowring<%d> ifidx<%d>\n",
	            (void*)arg1, PKTLEN(dhd->osh, (void*)arg1),
	            DHD_PKT_GET_FLOWID((void*)arg1), (uint32)arg2));
	        RPR1("H2R_TXPOST_NOTIF pkt<0x%px> ifid<%d>", (void*)arg1, (int)arg2);

	        rc = dhd_runner_txpost(dhd_hlp, (void*)arg1, (uint32)arg2);
	        return rc;

	    /* Host notifies Runner to process D2H TxCompl */
	    case H2R_TX_COMPL_NOTIF:
	        DHD_TRACE(("H2R_TX_COMPL_NOTIF\n"));
	        RPR1("H2R_TX_COMPL_NOTIF");

	        if ((dhd_hlp->ipend == FALSE) && (dhd_hlp->sw_feat.cmplnotif == TRUE)) {
	            rdpa_dhd_helper_complete_wakeup(dhd_hlp->dhd->unit, TRUE);
	            dhd_hlp->h2r_tx_compl_notif++;
	        }
	        break;


	    /* Host notifies Runner to attach/detach/register a DMA Buf */
	    case H2R_DMA_BUF_NOTIF:
	        DHD_TRACE(("H2R_DMA_BUF_NOTIF type<0x%08x> dma_buf<0x%px>\n",
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
	        DHD_TRACE(("H2R_INIT_NOTIF dhd_hlp<0x%px> pci_dev<0x%px>\n",
	                dhd_hlp, (struct pci_dev*)arg1));

		bdmf_set_nonblocking_log();
	        rc = dhd_runner_init(dhd_hlp, (struct pci_dev*)arg1);
		bdmf_clear_nonblocking_log();
	        return rc;
		break;

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

#if defined(DOL_HLPR_CAP_FFRD)
	        {
	            rdpa_dhd_msg_data_t data;

	            data.flowring_idx = arg1;
	            data.read_idx = arg2;
	            data.idx_valid = (arg2 < 0xFFFF) ? 1 : 0;
	            rc = rdpa_dhd_helper_flush_set(dhd_hlp->dhd_helper_obj, (bdmf_number)data.u32);
	        }
#else /* !DOL_HLPR_CAP_FFRD */
	        rc = rdpa_dhd_helper_flush_set(dhd_hlp->dhd_helper_obj, arg1);
#endif /* !DOL_HLPR_CAP_FFRD */

	        if (rc != 0) {
	            DHD_ERROR(("dor%d rdpa_dhd_helper_flush_set(0x%px, %d) returned %d\r\n",
	                dhd_hlp->dhd->unit, &dhd_hlp->dhd_helper_obj, (int)arg1, rc));
	            return BCME_ERROR;
	        }
	        break;

	    /* Host notifies Runner to update backup queue size */
	    case H2R_FLRING_BQSIZE_NOTIF: /* arg1:flowid [2.N] arg2:qd [0.0xffff] */
	        DHD_TRACE(("H2R_FLRING_BQSIZE_NOTIF flowring<%d> bq size <0x%x>\n",
	            (int)arg1, (uint16)arg2));

	        if (DHD_RNR_BKUPQ(dhd_hlp)) {
	            dhd_runner_update_flring_bqsize(dhd_hlp, arg1, arg2);
	        }
	        break;

	    /* Host notifies Runner to configure aggregation */
	    case H2R_AGGR_CONFIG_NOTIF: /* arg1:dhd_runner_aggr_config_t* */
	        DHD_TRACE(("H2R_AGGR_CONFIG_NOTIF aggr_config<0x%px>\n", (void*)arg1));

	        /* Check if any tx flow rings are offloaded */
	        if (DHD_RNR_TXSTS_OFFLOAD(dhd_hlp)) {
	            dhd_runner_aggr_config_t* aggr_cfg = (dhd_runner_aggr_config_t*)arg1;
#if defined(DOL_HLPR_CAP_LBRAGGR)
	            bdmf_number aggr_size;
	            bdmf_number aggr_timer;
	            bdmf_index wme_ac = 0;
	            int prio = 0;
	            int rc = 0;
#endif /* DOL_HLPR_CAP_LBRAGGR */

	            /* Check if Runner supports LBR aggregation */
	            if (dhd_hlp->sup_feat.lbraggr == 0) {
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

#if defined(DOL_HLPR_CAP_LBRAGGR)
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

	            dhd_hlp->en_feat.lbraggr = (aggr_cfg->en_mask) ? 1 : 0;

	            /* Set the aggregation timeout in the runner */
	            aggr_timer = (bdmf_number)aggr_cfg->timeout;
	            rc = rdpa_dhd_helper_aggregation_timer_set(
	                dhd_hlp->dhd_helper_obj, aggr_timer);

	            if (rc < 0) {
	                DHD_ERROR(("dhd%d_rnr: set aggr timeout failed %d\r\n",
	                    dhd_hlp->dhd->unit, rc));
	            }
#endif /* DOL_HLPR_CAP_LBRAGGR */
	        }
	        break;

	    /* Host notifies Runner to set/unset sending nonacceleted packet txstatus to dhd */
	    case H2R_TXSTS_CONFIG_NOTIF: /* arg1: enable */
	        DHD_TRACE(("H2R_TXSTS_CONFIG_NOTIF enable <%d>\n", (int)arg1));

	        if (DHD_RNR_TXSTS_OFFLOAD(dhd_hlp)) {

	            dhd_hlp->txsts_ring_cfg.offload = arg1&DHD_RNR_TXSTS_CFG_OFFL_MASK;

	            if (!(arg1 & DHD_RNR_TXSTS_CFG_NONACCPKT_OFFL) &&
	                (dhd_hlp->sup_feat.txcmpl2host == 0)) {
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
	                    dhd_hlp->rxpost_ring_cfg.cfgsts |= DOR_CFGSTS_FMTCHG;
	                    if (!(dhd_hlp->rxpost_ring_cfg.sup_types & (1ul << type)))
	                        rx_dor_en = FALSE;
	                    break;

	                case BCMPCIE_D2H_MSGRING_TX_COMPLETE:
	                    dhd_hlp->txsts_ring_cfg.size = size;
	                    dhd_hlp->txsts_ring_cfg.type = type;
	                    dhd_hlp->txsts_ring_cfg.cfgsts |= DOR_CFGSTS_FMTCHG;
	                    if (!(dhd_hlp->txsts_ring_cfg.sup_types & (1ul << type)))
	                        tx_dor_en = FALSE;
	                    break;

	                case BCMPCIE_D2H_MSGRING_RX_COMPLETE:
	                    dhd_hlp->rxcmpl_ring_cfg.size = size;
	                    dhd_hlp->rxcmpl_ring_cfg.type = type;
	                    dhd_hlp->rxcmpl_ring_cfg.cfgsts |= DOR_CFGSTS_FMTCHG;
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
	                    dhd_hlp->txpost_ring_cfg.cfgsts |= DOR_CFGSTS_FMTCHG;
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
	            }

	            if (!rx_dor_en) {
	                DHD_ERROR(("dor%d: %s ring format type [%d] not supported by Runner\n",
	                    dhd_hlp->dhd->unit,
	                    (ring == BCMPCIE_H2D_MSGRING_RXPOST_SUBMIT) ? "RxP" : "RxC", type));
	            }
	            rc = dhd_runner_force_disable_offload(dhd_hlp, !tx_dor_en,
	                    !rx_dor_en);

#if defined(DOL_HLPR_CAP_MSGRINGFRMT)
	            if (dhd_hlp->txpost_ring_cfg.type != dhd_hlp->en_feat.msgringformat) {
	                dhd_hlp->dhd_init_cfg.flow_ring_format = DOR_RINGTYPE_TO_RNR_RINGFMT(type);
	                dhd_hlp->en_feat.msgringformat = type;
	                DHD_TRACE(("dor%d: MSG Ring format set to %d",
	                    dhd_hlp->dhd->unit,
	                    dhd_hlp->dhd_init_cfg.flow_ring_format));
	            }
#endif /* DOL_HLPR_CAP_MSGRINGFRMT */
	        }
	        break;

	    /*
	     * Host notifies Runner of PCIE IPC Capabilities (DHD) for negotiation
	     *
	     * arg1 (in):     Host Capabilities mask (uint32)
	     * arg2 (in/out): Pointer to DHD capabilities (in)
	     *                           DoR capabilities (out)
	     *
	     * Parse for the dependant capabilities and enable corresponding
	     * features if runner support them. Send back all the enabled
	     * capabilities supported by DoR
	     *
	     * Note: Currently fast flow ring delete cap is supported, can be
	     * exteneded to others in future.
	     */
	    case H2R_PCIE_IPC_CAP_NOTIF:
	        DHD_TRACE(("H2R_PCIE_IPC_CAP_NOTIF cap_mask <0x%x> cap <0x%x>\n",
	            (uint32)arg1, *(uint32*)arg2));
	        if (arg1) {
	            uint32 hcap = arg1 & (*(uint32*)arg2);
	            uint32 rcap = 0;

	            if (hcap) {
	                /* Fast Flow Ring Delete */
	                if (hcap & H2R_PCIE_IPC_CAP_FFRD) {
	                    if (dhd_hlp->sup_feat.ffrd) {
	                        dhd_hlp->en_feat.ffrd = 1;
	                        rcap |= H2R_PCIE_IPC_CAP_FFRD;
	                    }
	                }

	                /* Host Backup Queue Depth */
	                if (hcap & H2R_PCIE_IPC_CAP_HBQD) {
	                    if (dhd_hlp->sup_feat.hbqd) {
	                        dhd_hlp->en_feat.hbqd = 1;
	                        dhd_hlp->dhd_init_cfg.hbqd_mode = 1;
	                        rcap |= H2R_PCIE_IPC_CAP_HBQD;
	                    } else {
	                        DHD_ERROR(("dor%d: HBQD not supported by Runner\n",
	                            dhd_hlp->dhd->unit));
	                        rc = dhd_runner_force_disable_offload(dhd_hlp,
	                                true, true);
	                        if (rc == BCME_OK) {
	                            rcap |= H2R_PCIE_IPC_CAP_HBQD;
	                        }
	                    }
	                }

	                /* Rx Complete V2 (tid support) */
	                if (hcap & H2R_PCIE_IPC_CAP_RX_CMPL_V2) {
	                    if (dhd_hlp->sup_feat.rxcmplv2) {
	                        dhd_hlp->en_feat.rxcmplv2 = 1;
	                        rcap |= H2R_PCIE_IPC_CAP_RX_CMPL_V2;
	                    }
	                }

	                /* Rx Complete RCH (RxCompletion in Hedroom) */
	                if (hcap & H2R_PCIE_IPC_CAP_RX_CMPL_RCH) {
	                    if (dhd_hlp->sup_feat.rxcmplrch) {
	                        dhd_hlp->en_feat.rxcmplrch = 1;
	                        rcap |= H2R_PCIE_IPC_CAP_RX_CMPL_RCH;
	                    }
	                }

	                /* BA256 support capability */
	                if (hcap & H2R_PCIE_IPC_CAP_BA256CFG) {
	                    if (dhd_runner_ba256cfg_init(dhd_hlp) == BCME_OK) {
	                        dhd_hlp->en_feat.ba256cfg = 1;
	                        rcap |= H2R_PCIE_IPC_CAP_BA256CFG;
	                    }
	                }

#if defined(DOL_HLPR_CAP_IDMA64)
	                /* iDMA64 support capability */
	                if (hcap & H2R_PCIE_IPC_CAP_IDMA64) {
	                    dhd_hlp->en_feat.idma64 = 1;
	                    rcap |= H2R_PCIE_IPC_CAP_IDMA64;
	                }
#endif /* DOL_HLPR_CAP_IDMA64 */
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

	DHD_INFO(("R2H dhd_hlp<%px> ops<%d> <0x%lx> <0x%lx>\n",
	          dhd_hlp, ops, arg1, arg2));

	if ((dhd_hlp == NULL) || (dhd_hlp->dhd == NULL)) {
	    DHD_ERROR(("%s: invalid arg dhd_hlp<%px>\n", __FUNCTION__, dhd_hlp));
	    return BCME_BADARG;
	}

	dhd = dhd_hlp->dhd;

	switch (ops) {

	    /* Request Host to receive a packet that missed in Runner */
	    case R2H_RX_COMPL_REQUEST:
	        /* arg1:pkt ptr, arg2:info.reason_data:ifid */
	        DHD_TRACE(("R2H_RX_COMPL_REQUEST pkt<0x%px> ifid<%d>\n",
	            (void*)arg1, (int)arg2));
	        if (DHD_RNR_RX_OFFLOAD(dhd_hlp)) {
	            RPR1("dhd_bus_rx_frame pkt<0x%px> if<%d>", (void*)arg1, (int)arg2);
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
	        DHD_TRACE(("R2H_TX_COMPL_REQUEST pkt<0x%px>\n", (void*)arg1));
	        if (DHD_RNR_TXSTS_OFFLOAD(dhd_hlp)) {
	            RPR1("R2H_TX_COMPL_REQUEST pkt<0x%px>", (void*)arg1);
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
	char val[DHD_RNR_VAL_STR_LEN];
	char *key_str = NULL;

	/* Prepare the nvram key */
	if (key_id == DHD_RNR_KEY_CAP_OVERRIDE) {
	    char radio[2] = {0};

	    if (radio_idx != DHD_RNR_RADIO_IDX_ALL) {
	        snprintf(radio, sizeof(radio), "%d", radio_idx);
	    }
	    snprintf(key, sizeof(key), dhd_runner_key_fmt_str[key_id], radio, buff);
	} else if (radio_idx != DHD_RNR_RADIO_IDX_ALL) {
	    snprintf(key, sizeof(key), dhd_runner_key_fmt_str[key_id], radio_idx);
	} else {
	    snprintf(key, sizeof(key), dhd_runner_key_fmt_str[key_id]);
	}

	/* uboot environment first */
	if (eNvramGet(key, val, DHD_RNR_VAL_STR_LEN) > 0) {
	    key_str = val;
	} else {
	    key_str = nvram_get(key);
	}

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

#if !defined(DHD_RNR_NVRAM_KEYS)
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
	char key[DHD_RNR_KEY_STR_LEN];

	/* Prepare the PSP key information */
	if (radio_idx != DHD_RNR_RADIO_IDX_ALL) {
	    snprintf(key, sizeof(key), dhd_runner_key_fmt_str[key_id], radio_idx);
	} else {
	    snprintf(key, sizeof(key), dhd_runner_key_fmt_str[key_id]);
	}
	key[sizeof(dhd_runner_key_fmt_str[key_id])-1] = '\0';

	return kerSysScratchPadSet(key, buff, len);
}
#endif /* !DHD_RNR_NVRAM_KEYS */

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
	dhd_flowring_profile_t* profile;
	dhd_runner_flowmgr_t *flowmgr = &dhd_hlp->flowmgr;

	/* Update the user profiles if present, only once */
	if (dhdol_cb.up_inited == false) {
	     int val1, val2;

	    /* Walk through all radios user profiles */
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
	            if (dhd_hlp->sup_feat.nplusm) {
	                sscanf(token, "%d:%d", &val2, &val1);
	                dhd_rnr_profiles[radio_idx].weight[ac] = val2;
	                if (val1 >= dhd_rnr_txp_ringsz_defaults[ac]) {
	                    val2 = val1 - dhd_rnr_txp_ringsz_defaults[ac];
	                    dhd_rnr_profiles[radio_idx].items[ac] = val1;
	                    dhd_rnr_profiles[radio_idx].bqsize[ac] = val2;
	                } else {
	                    RLOG("dhd%d discarding %d item size for %s\n",
	                        dhd_hlp->dhd->unit, val1, dhd_wme_ac_str[ac]);
	                }
	            } else {
	                char *token1;

	                sscanf(token, "%d", &val1);
	                token1  = bcmstrstr(token, ":");
	                if (token1) {
	                    /* New format with ring_size and bq_size */
	                    sscanf(token1, ":%d", &val2);
	                    /* Adjust ring_size to the nearest power of 2 */
	                    val1 = next_larger_power2(val1);
	                    dhd_rnr_profiles[radio_idx].items[ac] = val1 + val2;
	                    dhd_rnr_profiles[radio_idx].bqsize[ac] = val2;
	                } else {
	                    if (val1 >= dhd_rnr_txp_ringsz_defaults[ac]) {
	                        val2 = val1 - dhd_rnr_txp_ringsz_defaults[ac];
	                        dhd_rnr_profiles[radio_idx].items[ac] = val1;
	                        dhd_rnr_profiles[radio_idx].bqsize[ac] = val2;
	                    } else {
	                        RLOG("dhd%d discarding %d item size for %s\n",
	                            dhd_hlp->dhd->unit, val1, dhd_wme_ac_str[ac]);
	                    }
	                }
	            }
	            token = bcmstrtok(&profilestr, " ", NULL);
	            ac++;
	        }
	    }
	    dhdol_cb.up_inited = true;
	}

	/* Fetch the profile information of the radio, if present */
	radio_idx = dhd_hlp->dhd->unit;
	length = dhd_runner_key_get(radio_idx, DHD_RNR_KEY_PROFILE, buff,
	    sizeof(buff));

	if (length == 0) {
	    /* Profile not present, return default radio profile */
	    profile_id = flowmgr->def_profile_id;
	    goto bkupq;
	}

	/* parse profile_id from profile string */
	profilestr = buff;
	token = bcmstrtok(&profilestr, " ", NULL);
	sscanf(buff, "%d", &profile_id);

	if (profile_id >= DHD_RNR_FLOWRING_PROFILES) {
	    RLOG("profile id %d key is not valid, using built-in profile id %d",
	        profile_id, radio_idx);
	    profile_id = flowmgr->def_profile_id;
	}

	if ((!dhd_hlp->sup_feat.nplusm) && dhd_rnr_profiles[profile_id].nplusm) {
	    RLOG("profile id %d key is not supported, using built-in profile id %d",
	        profile_id, flowmgr->def_profile_id);
	    profile_id = flowmgr->def_profile_id;
	}

bkupq:
	profile = &dhd_rnr_profiles[profile_id];
	dhd_hlp->en_feat.bkupq = DHD_RNR_DEF_BKUPQ;

#if defined(DOL_HLPR_CAP_BKUPQUEUE)
	{
	    /* If nvram profile set dhd?_rnr_flowring_physize, then that overrides the defaults */
	    /* Exit from this section with phy_ring_size set to 0 means use the defaults */
	    length = dhd_runner_key_get(radio_idx, DHD_RNR_KEY_PHY_RING_SIZE, buff, sizeof(buff));
	    if (length != 0) {
	        RLOG("dor%d: dhd%d_rnr_flowring_physize setting deprecated"
	            "use dhd%d_rnr_flowring_profile instead\n",
	            dhd_hlp->dhd->unit, dhd_hlp->dhd->unit, dhd_hlp->dhd->unit);
	    }
	}
#endif /* DOL_HLPR_CAP_BKUPQUEUE */

#if defined(DOL_HLPR_CAP_DYNBKUPQ)
	/* Enable back up queue update feature if backup queue feature is enabled */
	dhd_hlp->en_feat.dynbkupq = dhd_hlp->en_feat.bkupq;
#endif /* DOL_HLPR_CAP_DYNBKUPQ */

	for (ac = wme_ac_bk; ac <= wme_ac_max; ac++) {
	    if (DHD_RNR_BKUPQ(dhd_hlp)) {
	        flowmgr->phy_items[ac] = DOR_PROFILE_TXP_RING_SIZE(profile, ac);
	    } else {
	        flowmgr->phy_items[ac] = profile->items[ac];
	    }
	}

	if (dhd_hlp->sup_feat.nplusm) {
	    RLOG("%s: N+M profile = %1d %02d:%04d/%04d %02d:%04d/%04d"
	        " %02d:%04d/%04d %02d:%04d/%04d %02d:%04d/%04d",
	        __FUNCTION__, profile_id,
	        profile->weight[0], flowmgr->phy_items[0], profile->bqsize[0],
	        profile->weight[1], flowmgr->phy_items[1], profile->bqsize[1],
	        profile->weight[2], flowmgr->phy_items[2], profile->bqsize[2],
	        profile->weight[3], flowmgr->phy_items[3], profile->bqsize[3],
	        profile->weight[4], flowmgr->phy_items[4], profile->bqsize[4]);
	} else {
	    RLOG("%s: profile = %1d %04d/%04d %04d/%04d"
	        " %04d/%04d %04d/%04d %04d/%04d",
	        __FUNCTION__, profile_id,
	        flowmgr->phy_items[0], profile->bqsize[0],
	        flowmgr->phy_items[1], profile->bqsize[1],
	        flowmgr->phy_items[2], profile->bqsize[2],
	        flowmgr->phy_items[3], profile->bqsize[3],
	        flowmgr->phy_items[4], profile->bqsize[4]);
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

	if (!dhd_hlp->sup_feat.nplusm) {
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
	dhd_runner_ring_cfg_t *rxc_ring_cfg, dhd_runner_ring_cfg_t *rxp_ring_cfg)
{
	char buff[DHD_RNR_KEY_RXOFFL_STR_LEN];
	int  length = 0;
	int offload, rxc_size = -1, rxp_size = -1;

	/* Set the default values */
	rxc_ring_cfg->offload = DHD_RNR_DEF_RX_OFFLOAD;
	rxc_ring_cfg->max_items = D2HRING_RXCMPLT_MAX_ITEM;

	if (rxp_ring_cfg) {
	    rxp_ring_cfg->offload = DHD_RNR_DEF_RX_OFFLOAD;
	    rxp_ring_cfg->max_items = H2DRING_RXPOST_MAX_ITEM;
	}

	/* Fetch the Rx Ring Configuration information of the radio, if present */
	memset(buff, 0, sizeof(buff));
	length = dhd_runner_key_get(dhd_hlp->dhd->unit, DHD_RNR_KEY_RXOFFL, buff,
	    sizeof(buff));

	if (length == 0) {
	    /* Rx Offload setting is not present, return default */
	    goto done;
	}

	sscanf(buff, "%d:%d:%d", &offload, &rxc_size, &rxp_size);

	if (offload <= 0)
	    rxc_ring_cfg->offload = 0;
	else
	    rxc_ring_cfg->offload = 1;

	if ((rxc_size > 0) && (rxc_size <= DHD_RNR_FLOWRING_MAX_SIZE)) {
	    rxc_ring_cfg->max_items = next_larger_power2(rxc_size);
	    rxc_ring_cfg->cfgsts |= DOR_CFGSTS_ITMCHG;
	}

	if ((rxp_ring_cfg) && (rxp_size > 0) &&
	    (rxp_size <= DHD_RNR_FLOWRING_MAX_SIZE)) {
	    rxp_ring_cfg->max_items = next_larger_power2(rxp_size);
	    rxp_ring_cfg->cfgsts |= DOR_CFGSTS_ITMCHG;
	}

done:
	if (!DHD_RNR_IS_RX_OFFL_SUPPORTED(dhd_hlp)) {
	    DHD_INFO(("dhd%d_rnr force disable rx offlaod due to no runner support\r\n",
	        dhd_hlp->dhd->unit));
	    rxc_ring_cfg->offload = 0;
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
	int offload = 1, size = 0;
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
	    (void **)&flowmgr->hw_mem_virt_base_addr,
	    &flowmgr->hw_mem_phys_base_addr,
	    &flowmgr->hw_mem_size) == 0) {
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
	        ring_cfg->max_items = next_larger_power2(size);
	        ring_cfg->cfgsts |= DOR_CFGSTS_ITMCHG;
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
	        dhd_hlp->en_feat.nplusm = 1;
	    } else if (offload == TX_DOR_MODE_N_ONLY) {
	        dhd_hlp->en_feat.nplusm = 0;
	    } else {
	        /* TX_DOR_MODE_N_DEF */
	        dhd_hlp->en_feat.nplusm = DOR_NPM_DEFAULT;
	    }
	    goto done;
	}

disable:
	DHD_INFO(("dhd%d_rnr disable tx offlaod \r\n", dhd_hlp->dhd->unit));
	ring_cfg->offload = 0;
	flowmgr->hw_mem_addr = (void*)NULL;
	flowmgr->hw_mem_size = 0;
	dhd_hlp->en_feat.txoffl = 0;
	dhd_hlp->en_feat.nplusm = 0;

done:
	flowmgr->def_profile_id = DHD_RNR_FLOWRING_DEFAULT_PROFILE_ID;
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

	DHD_TRACE(("%s: dhd_hlp=<0x%px>, buff=<0x%px>, bufflen=<%d>\r\n",
	    __FUNCTION__, dhd_hlp, buf, buflen));

	dhdp = dhd_hlp->dhd;

	bcm_binit(strbuf, buf, buflen);

	bcm_bprintf(strbuf, "[id] [   ac_bk   ] [   ac_be   ] [   ac_vi   ] "
	    "[   ac_vo   ] [   bc_mc   ]\n");
	if (dhd_hlp->sup_feat.nplusm) {
	    bcm_bprintf(strbuf, "     pr:ring:queue pr:ring:queue pr:ring:queue "
	        "pr:ring:queue pr:ring:queue\n");
	} else {
	    bcm_bprintf(strbuf, "     [ ring:queue] [ ring:queue] [ ring:queue] "
	        "[ ring:queue] [ ring:queue]\n");
	}

	for (id = 0; id < DHD_RNR_FLOWRING_PROFILES; id++) {
	    profile = &dhd_rnr_profiles[id];
	    if ((!dhd_hlp->sup_feat.nplusm) && profile->nplusm) {
	        continue;
	    }

	    if (profile == dhd_hlp->flowmgr.profile)
	        bcm_bprintf(strbuf, " *%d ", profile->id);
	    else
	        bcm_bprintf(strbuf, "  %d ", profile->id);

	    for (ac = wme_ac_bk; ac <= wme_ac_max; ac++) {
	        if (dhd_hlp->sup_feat.nplusm) {
	            bcm_bprintf(strbuf, "%02d:%04d:%04d ", profile->weight[ac],
	                DOR_PROFILE_TXP_RING_SIZE(profile, ac),
	                profile->bqsize[ac]);
	        } else {
	            bcm_bprintf(strbuf, "   %04d:%04d  ",
	                DOR_PROFILE_TXP_RING_SIZE(profile, ac),
	                profile->bqsize[ac]);
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
#if !defined(DHD_RNR_NVRAM_KEYS)
	char *token, *profile_str;
	dhd_wme_ac_t ac;
	dhd_flowring_profile_t profile;
	char pspbuf[DHD_RNR_IOVAR_BUFF_SIZE];
	int radio_idx;

	DHD_TRACE(("%s: dhd_hlp=<0x%px>, buff=<%s>, bufflen=<%d>\r\n",
	    __FUNCTION__, dhd_hlp, buff, bufflen));

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
#else /* DHD_RNR_NVRAM_KEYS */
	DHD_ERROR(("    Setting through DHD is deprecated\r\n"));
	DHD_ERROR(("    Instead use nvram kset, kcommit and reboot\r\n"));

	return BCME_UNSUPPORTED;
#endif /* DHD_RNR_NVRAM_KEYS */
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

	DHD_TRACE(("%s: dhd_hlp=<0x%px>, buff=<0x%px>, bufflen=<%d>\r\n",
	    __FUNCTION__, dhd_hlp, buf, buflen));

	if (!dhd_hlp->sup_feat.nplusm) {
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
#if !defined(DHD_RNR_NVRAM_KEYS)
	char pspbuf[DHD_RNR_KEY_POLICY_STR_LEN];
	char *token, *policy_str;
	int  value;
	int mac_addr[ETHER_ADDR_LEN];
	int ac;
	dhd_flowring_policy_t policy;
	char key[32];

	DHD_TRACE(("%s: dhd_hlp=<0x%px>, buff=<%s>, bufflen=<%d>\r\n",
	    __FUNCTION__, dhd_hlp, buff, bufflen));

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
#else /* DHD_RNR_NVRAM_KEYS */
	DHD_ERROR(("    Setting through DHD is deprecated\r\n"));
	DHD_ERROR(("    Instead use nvram kset, kcommit and reboot\r\n"));

	return BCME_UNSUPPORTED;
#endif /* DHD_RNR_NVRAM_KEYS */
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

	DHD_TRACE(("%s: dhd_hlp=<0x%px>, buff=<0x%px>, bufflen=<%d>\r\n",
	    __FUNCTION__, dhd_hlp, buf, buflen));

	if (dhd_runner_rxoffl_init(dhd_hlp, &ring_cfg, NULL) != 0) {
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
#if !defined(DHD_RNR_NVRAM_KEYS)
	char pspbuf[DHD_RNR_KEY_RXOFFL_STR_LEN];
	int offload;
	int size = D2HRING_RXCMPLT_MAX_ITEM;

	DHD_TRACE(("%s: dhd_hlp=<0x%px>, buff=<%s>, bufflen=<%d>\r\n",
	    __FUNCTION__, dhd_hlp, buff, buflen));

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
#else /* DHD_RNR_NVRAM_KEYS */
	DHD_ERROR(("    Setting through DHD is deprecated\r\n"));
	DHD_ERROR(("    Instead use nvram kset, kcommit and reboot\r\n"));

	return BCME_UNSUPPORTED;
#endif /* DHD_RNR_NVRAM_KEYS */
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
dhd_runner_iovar_dump(dhd_runner_hlp_t *dhd_hlp, char *buff, int bufflen)
{
	struct bcmstrbuf *b = (struct bcmstrbuf *)buff;
	dhd_runner_flowmgr_t *flowmgr;
	enum dhd_wme_ac wme_ac;
	char policy_str[DHD_RNR_IOVAR_BUFF_SIZE];

	flowmgr = &dhd_hlp->flowmgr;

	/* Status */
	bcm_bprintf(b, "\n");
	bcm_bprintf(b, "dol_Helper  : %s (radio %d)\n", DOL_HELPER,
	    dhd_hlp->dhd->unit);

	bcm_bprintf(b, " dol_cap Hlp: %s  %s  %s  %s  %s  %s  %s  %s  %s  %s  %s  "
	    "%s  %s  %s  %s  %s  %s  %s  %s\n",
	    DOL_HLPR_CAP_EN_STS_STR(dhd_hlp, txoffl),
	    DOL_HLPR_CAP_EN_STS_STR(dhd_hlp, rxoffl),
	    DOL_HLPR_CAP_STS_STR(dhd_hlp, txoffl, txoffl, txcmpl2host),
	    DOL_HLPR_CAP_STS_STR(dhd_hlp, txoffl, txoffl, dhdhdr),
	    DOL_HLPR_CAP_STS_STR(dhd_hlp, txoffl, txoffl, lbraggr),
	    DOL_HLPR_CAP_STS_STR(dhd_hlp, txoffl, rxoffl, msgringformat),
	    DOL_HLPR_CAP_STS_STR(dhd_hlp, txoffl, txoffl, bkupq),
	    DOL_HLPR_CAP_STS_STR(dhd_hlp, txoffl, rxoffl, hwawkup),
	    DOL_HLPR_CAP_STS_STR(dhd_hlp, txoffl, txoffl, ffrd),
	    DOL_HLPR_CAP_STS_STR(dhd_hlp, txoffl, txoffl, nplusm),
	    DOL_HLPR_CAP_STS_STR(dhd_hlp, txoffl, txoffl, hbqd),
	    DOL_HLPR_CAP_STS_STR(dhd_hlp, txoffl, txoffl, dynbkupq),
	    DOL_HLPR_CAP_STS_STR(dhd_hlp, txoffl, txoffl, codel),
	    DOL_HLPR_CAP_STS_STR(dhd_hlp, txoffl, rxoffl, cmnrngsz),
	    DOL_HLPR_CAP_STS_STR(dhd_hlp, rxoffl, rxoffl, rxcmplv2),
	    DOL_HLPR_CAP_STS_STR(dhd_hlp, rxoffl, rxoffl, rxcmplrch),
	    DOL_HLPR_CAP_STS_STR(dhd_hlp, txoffl, rxoffl, ba256cfg),
	    DOL_HLPR_CAP_STS_STR(dhd_hlp, txoffl, txoffl, idmagrpcfg),
	    DOL_HLPR_CAP_STS_STR(dhd_hlp, txoffl, txoffl, idma64));

	bcm_bprintf(b, " dol_cap SW : %s  %s\n",
		DOL_SWFEAT_STS_STR(dhd_hlp, cpuqdpc),
		DOL_SWFEAT_STS_STR(dhd_hlp, cmplnotif));

	if (dhd_hlp->sup_feat.nplusm) {
	    /* Profile */
	    bcm_bprintf(b, " dol_prf TxP: prfl_id %d id_valu", flowmgr->profile->id);
	    for (wme_ac = wme_ac_bk; wme_ac <= wme_ac_max; wme_ac++)
	        bcm_bprintf(b, "  %02d:%04d:%04d ",
	            flowmgr->profile->weight[wme_ac], flowmgr->phy_items[wme_ac],
	            flowmgr->profile->bqsize[wme_ac]);
	    bcm_bprintf(b, "\n");

	    /* Policy */
	    policy_str[0] = '\0';
	    dhd_runner_get_policy_str(&dhd_rnr_policies[dhd_hlp->dhd->unit],
	        policy_str, sizeof(policy_str));
	    bcm_bprintf(b, " dol_pol TxP: plcy_id %d id_valu %s\n",
	        dhd_rnr_policies[dhd_hlp->dhd->unit].id, policy_str);


	    /* TX flowrings */
	    bcm_bprintf(b, " dol_flr TxP:");
	    for (wme_ac = wme_ac_bk; wme_ac <= wme_ac_max; wme_ac++)
	        bcm_bprintf(b, " [%s] sw %d hw %d", dhd_wme_ac_str[wme_ac],
	            dhd_hlp->flowmgr.sw_ring_cnt[wme_ac], dhd_hlp->flowmgr.hw_ring_cnt[wme_ac]);
	    bcm_bprintf(b, "\n");
	} else {
	    /* Profile */
	    bcm_bprintf(b, " dol_prf TxP: prfl_id %d id_valu", flowmgr->profile->id);
	    for (wme_ac = wme_ac_bk; wme_ac <= wme_ac_max; wme_ac++)
	        bcm_bprintf(b, "  %04d:%04d ", flowmgr->phy_items[wme_ac],
	            flowmgr->profile->bqsize[wme_ac]);
	    bcm_bprintf(b, "\n");
	}

	bcm_bprintf(b, " dol_prf CmN: TxC %d RxC %d RxP %d\n",
	    dhd_hlp->txsts_ring_cfg.max_items, dhd_hlp->rxcmpl_ring_cfg.max_items,
	    dhd_hlp->rxpost_ring_cfg.max_items);

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
	bcm_bprintf(b, " dol_cnt h2o: tx_post %lu rx_cmpl %lu tx_cmpl %lu txp_fail %lu\n",
	    dhd_hlp->h2r_txpost_notif, dhd_hlp->h2r_rx_compl_notif,
	    dhd_hlp->h2r_tx_compl_notif, dhd_hlp->h2r_txp_fail);
	bcm_bprintf(b, " dol_cnt o2h: rx_cmpl %lu tx_cmpl %lu wk_dngl %lu "
	    "sq_txp %lu (tx_cmpl+sq_txp) %lu\n",
	    dhd_hlp->r2h_rx_compl_req, dhd_hlp->r2h_tx_compl_req,
	    dhd_hlp->r2h_wake_dngl_req, dhd_hlp->o2h_sq_txp_notif,
	    (dhd_hlp->r2h_tx_compl_req + dhd_hlp->o2h_sq_txp_notif));

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
	bdmf_number rnr_drop_cnt;
	int rc;

	if ((buf == NULL) || (buflen < sizeof(dhd_helper_rnr_stats_t))) {
	    DHD_ERROR(("dhd%d_rnr iovar [%d] bad arguments", dhd_hlp->dhd->unit,
	        DHD_RNR_IOVAR_RNR_STATS));
	    DHD_ERROR(("buf 0x%px, buflen %d\r\n", buf, buflen));
	    return BCME_BADARG;
	}

	pstats->mcast_pkts = 0;
	pstats->mcast_bytes = 0;
	pstats->dropped_pkts = 0;

	if (!DHD_RNR_IS_OFFL_SUPPORTED(dhd_hlp)) {
	    /* Runner does not support this radio, return 0 stats */
	    return BCME_OK;
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
	    DHD_ERROR(("buf 0x%px, buflen %d\r\n", buf, buflen));
	    return BCME_BADARG;
	}

	/* Copy the supported and enabled features */
	memcpy(&pstatus->sup_features, &dhd_hlp->sup_feat, sizeof(dhd_helper_feat_t));
	memcpy(&pstatus->en_features, &dhd_hlp->en_feat, sizeof(dhd_helper_feat_t));

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
