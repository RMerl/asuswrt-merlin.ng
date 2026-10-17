/*
   Copyright (c) 2023 Broadcom
   All Rights Reserved
   <:label-BRCM:2023:DUAL/GPL:standard

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
#ifdef OSL_CACHE_NEWAPI_AVAIL
#define OSL_CACHE_NEWAPI
#endif

#include <typedefs.h>
#include <linux/pci.h>
#include <linux/bcm_colors.h>

#include <dhd_offload_helper.h>

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
#include <dhd_offload.h>
#include <dhd_flowring.h>
#include <dhd_linux.h>
#include <dhd_proto.h>

#include <pcie_core.h>
#include <dhd_pcie.h>
#include <wlan_shared_defs.h>

#if IS_ENABLED(CONFIG_BCM_SPDSVC)
#include <bcm_spdsvc.h>
static bcmFun_t *dhdol_spdsvc_transmit = NULL;
#endif

/*
 * +---------------------------------------------------------------------
 *           Section: TESTING MACROS for local debugging
 * +---------------------------------------------------------------------
*/
//#define DOL_DBG
#ifdef DOL_DBG
#define DOLPR(fmt, args...)     printk(fmt "\n", ##args)
#define DOLPR1(fmt, args...)    printk(fmt "\n", ##args)
#define DOLPR2(fmt, args...)    printk(fmt "\n", ##args)
#define DOLOG(fmt, args...)     printk(fmt "\n", ##args)
#define DOL_ERROR(fmt, args...) printk(fmt, ##args)
#define DOL_INFO(fmt, args...)  printk(fmt, ##args)
#define DOL_TRACE(fmt, args...) printk(fmt, ##args)
#else /* !DOL_DBG */
#define DOLPR(fmt, args...)     do {} while (0)
#define DOLPR1(fmt, args...)    do {} while (0)
#define DOLPR2(fmt, args...)    do {} while (0)
#define DOLOG(fmt, args...)     printk(fmt "\n", ##args)
#define DOL_ERROR(fmt, args...) DHD_ERROR((fmt, ##args))
#define DOL_INFO(fmt, args...)  DHD_INFO((fmt, ##args))
#define DOL_TRACE(fmt, args...) DHD_TRACE((fmt, ##args))
#endif /* !DOL_DBG */

/*
 * +---------------------------------------------------------------------
 *           Section: Utility MACROS
 * +---------------------------------------------------------------------
 */
#define B2MB(bytes)           (((bytes) + 0xFFFFF) / 0x100000)

/*
 * +---------------------------------------------------------------------
 *           Section: Helper flowring Cache definitions
 *
 * hlpr_fr_cache_t flags field bit definitions
 *
 *  bits 1..0 are used and owned by Helper firmware
 *  bits 7..2 are used and owned by DHD Offload software
 *
 * +---------------------------------------------------------------------
 */
#define DOL_FR_DISABLED_FLAG     FLOW_RING_FLAG_DISABLED
#define DOL_FR_IN_HLPR_FLAG      (1 << 2)
#define DOL_FR_WME_AC_SHIFT      (4)
#define DOL_FR_WME_AC_MASK       (0xF << DOL_FR_WME_AC_SHIFT)
#define DOL_FR_WME_AC(flags)     \
	(((flags) & DOL_FR_WME_AC_MASK) >> DOL_FR_WME_AC_SHIFT)

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
#define OSL_VIRT_TO_PHYSADDR(osh, va, pa) \
	ULONGTOPHYSADDR((ulong)VIRT_TO_PHYS((va)), (pa))
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
#	define DOL_HLPR_INIT_CFG_SET_PA(field, pa, off)                    \
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
 * +---------------------------------------------------------------------
*/
extern int dhd_rxbound;
extern int dhd_txbound;
extern void dhdpcie_bus_ringbell_fast(struct dhd_bus *bus, uint32 value);
#if defined(BCMHWA)
extern void dhdpcie_bus_ringbell_2_fast(struct dhd_bus *bus, uint32 value,
	bool devwake);
extern void dhdpcie_bus_db1_ringbell_2_fast(struct dhd_bus *bus, uint32 value,
	bool devwake);
#endif /* BCMHWA */
extern void dhd_prot_runner_txstatus_process(dhd_pub_t *dhd, void* pktid);
extern char * nvram_get(const char *name);


/*
 * +---------------------------------------------------------------------
 *           Section: Local definitions
 * +---------------------------------------------------------------------
*/
#define DOL_RADIO_MAX                       4
#define DOL_BSS_MAX                         16
#define DOL_STATION_MAX                     128
#define DOL_IOV_BUFF_SIZE                   512
#define DOL_RXO_DEFAULT                     1
#define DOL_BCMC_PROFILE_WEIGHT_DEFAULT     1
#define DOL_CMP_SIZE                        32*1024  /* 32 KB */
#define DOL_CMP_ALIGN_MASK                  0x3FF

#define DOL_TXS_CFG_ACCPKT_OFFL             DHD_OFFLOAD_TXSTS_CFG_ACCPKT_OFFL
#define DOL_TXS_CFG_NONACCPKT_OFFL          DHD_OFFLOAD_TXSTS_CFG_NONACCPKT_OFFL
#define DOL_TXS_CFG_OFFL_MASK               DHD_OFFLOAD_TXSTS_CFG_OFFL_MASK

#define DOL_TXO_MODE_M                      0  /* Offload:0, n+m: disabled */
#define DOL_TXO_MODE_N_DEF                  1  /* Offload:1, n+m: default */
#define DOL_TXO_MODE_N_ONLY                 2  /* Offlaod:1, n+m: disabled */
#define DOL_TXO_MODE_NPM                    3  /* Offload:1, n+m: enabled */

/* N+M feature default, disabled from REL_5.04L.01 and REL_5.02L.07P1 */
#define DOL_NPM_DEFAULT                     0

/* When a per AC profile with -1 weight is used, use a two pass with a budget
 * for the first pass.
 */
#define DOL_PROFILE_PER_AC_BUDGET           64

#define DOL_TXO_SUPPORTED(dhdol)            ((dhdol)->sup_feat.txoffl == 1)

#define DOL_RXO_SUPPORTED(dhdol)            ((dhdol)->sup_feat.rxoffl == 1)

#define DOL_SUPPORTED(dhdol)                \
	(DOL_TXO_SUPPORTED(dhdol) && DOL_RXO_SUPPORTED(dhdol))

#define DOL_RXO_ENABLED(dhdol)              \
	((dhdol)->cfg.rxc_ring.offload == 1)
#define DOL_TXO_ENABLED(dhdol)              \
	((dhdol)->cfg.txs_ring.offload & DOL_TXS_CFG_ACCPKT_OFFL)

#define DOL_TXO_TXSTS_OFFLOAD(dhdol)        \
	((dhdol)->cfg.txs_ring.offload & DOL_TXS_CFG_NONACCPKT_OFFL)

/* DMA WR and RD index size (2/4 bytes) */
#define DOL_DMA_IDX_SZ(dhd)                 ((dhd)->bus->rw_index_sz)

#define DOL_IF_ROLE_STA_OR_WDS(dhd, ifidx)  \
	(DHD_IF_ROLE_STA((dhd), (ifidx)) || DHD_IF_ROLE_WDS((dhd), (ifidx)))

#define DOL_UNIT                            (dhdol->dhd->unit)
#define DOL_UNIT_INV                        (0xFF)
#define DOL_RADIO_IDX                       (dhdol->hlpr.radio_idx)

/*
 * +---------------------------------------------------------------------
 *           Section: platform specifc settings
 * +---------------------------------------------------------------------
*/
/* BCA_CPEROUTER */
#define DOL_SWFEAT_CPUQDPC_DEF              TRUE              /* Enable */
#define DOL_SWFEAT_CMPLNOTIF_DEF            TRUE              /* Enable */
#define DOL_TXO_BCMC_PRIORITY               1                 /* Normal */
#define DOL_INIT_PERIM_UNLOCK(dhdp)         do {} while (0)   /* no OP */
#define DOL_INIT_PERIM_LOCK(dhdp)           do {} while (0)   /* no OP */

/*
 * +---------------------------------------------------------------------
 *           Section: DMA Buffer Memory management (dol_mem)
 *
 * Enable DOL_MEM_ALLOC_AUDIT to get a summary of memory leaks
 * of dhd_offload part at the end of dhd driver unloading
 *
 * +---------------------------------------------------------------------
 */
/* #define DOL_MEM_ALLOC_AUDIT */
/* memory allocation */
#define DOL_MEM_ALLOC_TYPE_OSAL        0x0000    /* OSL DMA_ALLOC_CONSISTENT */
#define DOL_MEM_ALLOC_TYPE_COHERENT    0x0001    /* dma_alloc_coherent */
#define DOL_MEM_ALLOC_TYPE_KMALLOC     0x0002    /* kmalloc(GFP_DMA) */
#define DOL_MEM_ALLOC_TYPE_MAX         0x0003    /* delimiter */

#define DOL_MEM_TYPE_DHD               DOL_MEM_ALLOC_TYPE_OSAL     /* DHD */
#if IS_ENABLED(CONFIG_BCM_DHD_ARCHER)
#define DOL_MEM_TYPE_HLPR              DOL_MEM_ALLOC_TYPE_KMALLOC  /* HLPR */
#else
#define DOL_MEM_TYPE_HLPR              DOL_MEM_ALLOC_TYPE_COHERENT /* HLPR */
#endif
#define DOL_MEM_TYPE_HLPRVA            DOL_MEM_ALLOC_TYPE_KMALLOC  /* HLPR */

#ifdef DOL_MEM_ALLOC_AUDIT
#define DOL_MEM_ALLOC_CNTR_INC(dhdol, type, size)                     \
	(dhdol)->alloc_cnt[(type)]++; (dhdol)->alloc_size[(type)] += (size)

#define DOL_MEM_ALLOC_CNTR_DEC(dhdol, type, size)                     \
	(dhdol)->alloc_cnt[(type)]--; (dhdol)->alloc_size[(type)] -= (size)

#define DOL_MEM_ALLOC_CNTR_DUMP(dhdol)                                \
	do {                                                              \
	    int type = DOL_MEM_ALLOC_TYPE_OSAL;                           \
	    DOLOG("dhd_offload MemLeak Summary\nType: Leaks: Leak Bytes"); \
	    do {                                                          \
	        DOLOG(" %2d :  %4d : %08d", type,                         \
	            (dhdol)->alloc_cnt[type],                             \
	            (dhdol)->alloc_size[type]);                           \
	        type++;                                                   \
	    } while (type < DOL_MEM_ALLOC_TYPE_MAX);                      \
	} while (0)
#else /* !DOL_MEM_ALLOC_AUDIT */
#define DOL_MEM_ALLOC_CNTR_INC(dhdol, type, size)                     \
	do {} while (0)

#define DOL_MEM_ALLOC_CNTR_DEC(dhdol, type, size)                     \
	do {} while (0)

#define DOL_MEM_ALLOC_CNTR_DUMP(dhdol)                                \
	do {} while (0)
#endif /* !DOL_MEM_ALLOC_AUDIT */

/*
 * +---------------------------------------------------------------------
 *           Section: Flow Ring Manager (dol_frm)
 *
 *  - Reserved memory carving
 *  - Profile Management
 *  - Policy management
 *  - flow ring id management
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
#define DOL_FRM_PROFILE_MAX           8
#define DOL_FRM_PROFILE_DEFAULT       5
#define DOL_FR_MAX_SIZE              0x8000  /* 16bit & power of 2 */

/* flow ring profile setting maintained per radio */
typedef struct dol_frm_profile {
	bool nplusm;                /* only N+M profile */
	int id;                     /* Profile id, 0 - DOL_FRM_PROFILE_MAX-1 */
	int weight[wme_ac_max + 1]; /* -1 weight implies use rsvd memory */
	int items[wme_ac_max + 1];  /* flow ring max items (size) */
	int bqsize[wme_ac_max + 1]; /* backup queue max depth */
} dol_frm_profile_t;

/* Policy definitions */
#define DOL_FR_POLICY_MACLIST_MAX          6

/* flowring policy identifiers */
typedef enum dol_frm_policy_id {
	dol_frm_policy_id_global = 0,    /* ID for Global policy */
	dol_frm_policy_id_intf = 1,      /* ID for interface based policy */
	dol_frm_policy_id_clients = 2,   /* ID for clients based policy */
	dol_frm_policy_id_aclist = 3,    /* ID for ac list based policy */
	dol_frm_policy_id_maclist = 4,   /* ID for mac list based policy */
	dol_frm_policy_id_dllac = 5,     /* ID for.11 AC client based policy */
	dol_frm_policy_id_max = 6        /* delimiter */
} dol_frm_policy_id_t;

/*
 * flowring policy setting maintained per radio
 * policy specifies the preferred choice to select offload
 */
typedef struct dol_frm_policy {
	int id;                       /* policy Identifier */
	union {
	    bool all_hw;              /* all rings offload flag */
	    int  max_intf;            /* maximum interfaces to offload */
	    int  max_sta;             /* max assoc stations to offload */
	    bool aclist_hw[wme_ac_max + 1]; /* ac list to offload */
	    /* station mac address list to offload */
	    uint8 mac_addr[DOL_FR_POLICY_MACLIST_MAX][ETHER_ADDR_LEN+1];
	    bool d11ac_hw;            /* .11 AC client offload */
	};
} dol_frm_policy_t;

#define DOL_CFGSTS_FMTCHG                    (1 << 1)     /* Format Change */
#define DOL_CFGSTS_ITMCHG                    (1 << 2)     /* max_items Change */

/* DHD PCIe msg ring information maintained in dhd_offload */
typedef struct dol_ring_cfg {
	int offload;            /* enabled: 1 (default), disabled: 0 */
	int max_items;          /* max items in msg ring */
	int type;               /* legacy, cwi32, cwi64 */
	int size;               /* len of the work item */
	int cfgsts;             /* Ring configuration status */
	uint32 sup_types;       /* bit mask of types supported by Helper */
	bool initialized;       /* ring has been initialized */
} dol_ring_cfg_t;

const uint8 dhdol_wme_fifo2ac[] = { 0, 1, 2, 3 };
/* incoming priority is already AC (0-3) */
const uint8 dhdol_prio2fifo[4] = { 1, 0, 2, 3  };

#define WME_PRIO2AC(pktprio)  dhdol_wme_fifo2ac[dhdol_prio2fifo[(pktprio)]]
#define WME_AC2PRIO(ac)       dhdol_wme_fifo2ac[(ac)]
const char * dhdol_wme_ac_str[] = {"ac_bk", "ac_be", "ac_vi", "ac_vo", "bc_mc"};

const char *dhdol_frm_policy_id_str[] = {"global", "intfidx", "clients",
	"aclist", "maclist", "dot11ac" };

/* Default ring sizes for BA256 supported radios */
#define DOL_FRM_PROFILE_DEFAULT_BA256         4
#define DOL_TXP_AC_BE_BQSIZE_BA256           2048
#define DOL_RXC_RINGSZ_BA256                 4096

#define DOL_TXP_ITEM_MAX                     2048

/* Default backup queue sizes per Access Category */
#define DOL_TXP_AC_BK_BQSIZE_MAX             512
#define DOL_TXP_AC_BE_BQSIZE_MAX             1024
#define DOL_TXP_AC_VI_BQSIZE_MAX             512
#define DOL_TXP_AC_VO_BQSIZE_MAX             0
#define DOL_TXP_AC_BCMC_BQSIZE_MAX           0

/* Default flowring sizes per Access Category */
#define DOL_TXP_AC_BK_RINGSZ_MAX             512
#define DOL_TXP_AC_BE_RINGSZ_MAX             1024
#define DOL_TXP_AC_VI_RINGSZ_MAX             512
#define DOL_TXP_AC_VO_RINGSZ_MAX             512
#define DOL_TXP_AC_BCMC_RINGSZ_MAX           512
#define DOL_TXP_AC_RINGSZ_MAX                512

/* Default TxPost items size per Access Category */
#define DOL_TXP_ITEMS_MAX(ac) \
	(DOL_TXP_##ac##_RINGSZ_MAX + DOL_TXP_##ac##_BQSIZE_MAX)

/* Same TxPost items size per Access Category */
#define DOL_TXP_ITEMS_SAME(ac) \
	(DOL_TXP_AC_RINGSZ_MAX + DOL_TXP_##ac##_BQSIZE_MAX)

/*
 * profile items and backup queue size lists
 */

/* Default profile items list (ring size + backup queue depth) */
#define DOL_TXP_ITEMS_LIST_DEF \
{ \
	DOL_TXP_ITEMS_MAX(AC_BK), DOL_TXP_ITEMS_MAX(AC_BE), \
	DOL_TXP_ITEMS_MAX(AC_VI), DOL_TXP_ITEMS_MAX(AC_VO), \
	DOL_TXP_ITEMS_MAX(AC_BCMC) \
}

/* Profile items list (same ring size + default backup queue depth) */
#define DOL_TXP_ITEMS_LIST_SAME \
{ \
	DOL_TXP_ITEMS_SAME(AC_BK), DOL_TXP_ITEMS_SAME(AC_BE), \
	DOL_TXP_ITEMS_SAME(AC_VI), DOL_TXP_ITEMS_SAME(AC_VO), \
	DOL_TXP_ITEMS_SAME(AC_BCMC) \
}

/* Profile with Ring Size 512 for all ac and default backup queue depth */
#define DOL_TXP_ITEMS_LIST_RING_512 \
{ \
	DOL_TXP_ITEMS_MAX(AC_BK), DOL_TXP_ITEMS_MAX(AC_BE), \
	DOL_TXP_ITEMS_MAX(AC_VI), DOL_TXP_ITEMS_MAX(AC_VO), \
	DOL_TXP_ITEMS_MAX(AC_BCMC) \
}

/* Default profile backup queue depth list */
#define DOL_TXP_BQSIZE_LIST_DEF \
{ \
	DOL_TXP_AC_BK_BQSIZE_MAX, DOL_TXP_AC_BE_BQSIZE_MAX, \
	DOL_TXP_AC_VI_BQSIZE_MAX, DOL_TXP_AC_VO_BQSIZE_MAX, \
	DOL_TXP_AC_BCMC_BQSIZE_MAX \
}

/* Max profile items list */
#define DOL_TXP_ITEMS_LIST_MAX \
{ \
	DOL_TXP_ITEM_MAX, DOL_TXP_ITEM_MAX, \
	DOL_TXP_ITEM_MAX, DOL_TXP_ITEM_MAX, \
	DOL_TXP_ITEM_MAX \
}

/* Max profile backup queue depth list */
#define DOL_TXP_BQSIZE_LIST_MAX \
{ \
	(DOL_TXP_ITEM_MAX - DOL_TXP_AC_BK_RINGSZ_MAX), \
	(DOL_TXP_ITEM_MAX - DOL_TXP_AC_BE_RINGSZ_MAX), \
	(DOL_TXP_ITEM_MAX - DOL_TXP_AC_VI_RINGSZ_MAX), \
	(DOL_TXP_ITEM_MAX - DOL_TXP_AC_VO_RINGSZ_MAX), \
	(DOL_TXP_ITEM_MAX - DOL_TXP_AC_BCMC_RINGSZ_MAX) \
}

/* BA256 configuration profile BE Category TxPost items */
#define DOL_TXP_AC_BE_ITEMS_BA256 \
	(DOL_TXP_AC_BE_RINGSZ_MAX + DOL_TXP_AC_BE_BQSIZE_BA256)

/* BA256 profile items list */
#define DOL_TXP_ITEMS_LIST_BA256 \
{ \
	DOL_TXP_ITEMS_MAX(AC_BK), DOL_TXP_AC_BE_ITEMS_BA256, \
	DOL_TXP_ITEMS_MAX(AC_VI), DOL_TXP_ITEMS_MAX(AC_VO), \
	DOL_TXP_ITEMS_MAX(AC_BCMC) \
}

/* BA256 profile backup queue depth list */
#define DOL_TXP_BQSIZE_LIST_BA256 \
{ \
	DOL_TXP_AC_BK_BQSIZE_MAX, DOL_TXP_AC_BE_BQSIZE_BA256, \
	DOL_TXP_AC_VI_BQSIZE_MAX, DOL_TXP_AC_VO_BQSIZE_MAX, \
	DOL_TXP_AC_BCMC_BQSIZE_MAX \
}

/* TxPost flow ring size given profile and Access Category */
#define DOL_PROFILE_TXP_RING_SIZE(profile, ac) \
	((profile)->items[(ac)] - (profile)->bqsize[(ac)])

/*
 * Storage area selection for key settings
 */
#define DOL_NV_UNIT_ALL                'a'

/*
 * +---------------------------------------------------------------------
 *           Section: Non-Volatile Settings (dol_nv)
 *
 *  - nvram settings management
 * +---------------------------------------------------------------------
 */
#define DOL_NV_VAR_STR_LEN             32
#define DOL_NV_VAL_STR_LEN             128

/* key ids within dhd offload */
#define DOL_NV_VAR_PROFILE             0
#define DOL_NV_VAR_POLICY              1
#define DOL_NV_VAR_RXOFFL              2
#define DOL_NV_VAR_TXOFFL              3
#define DOL_NV_VAR_PHY_RING_SIZE       4
#define DOL_NV_VAR_OFFL_OVERRIDE       5
#define DOL_NV_VAR_CODEL               6
#define DOL_NV_VAR_MAX_RNR             7 /* End of old format variables */
#define DOL_NV_VAR_CAP_OVERRIDE        7
#define DOL_NV_VAR_MAX                 8

/* Max length of key information stored in memory */
#define DOL_NV_PROFILE_STR_LEN         128   /* Profile information */
#define DOL_NV_POLICY_STR_LEN          768   /* Policy information */
#define DOL_NV_RXOFFL_STR_LEN          16    /* Rx Offload information */
#define DOL_NV_TXOFFL_STR_LEN          16    /* Tx Offload information */
#define DOL_NV_PHYRINGSIZE_STR_LEN     64    /* physical ring size information */
#define DOL_NV_OFFL_OVERRIDE_STR_LEN   32    /* Offload override information */
#define DOL_NV_CODEL_STR_LEN           16    /* Runner Codel information */
#define DOL_NV_CAP_ORIDE_STR_LEN       32    /* Helper Capabilities override key */
#define DOL_NV_UINT32_STR_LEN          16    /* key holding uint32 information */

/* storage key id vs format str */
char dhdol_nv_var_fmt_str[DOL_NV_VAR_MAX][DOL_NV_VAR_STR_LEN] = {
	"dol%s_flowring_profile",
	"dol%s_flowring_policy",
	"dol%s_rxoffl",
	"dol%s_txoffl",
	"dol%s_flowring_physize",
	"dol%s_cap_offload_override",
	"dol%s_codel",
	"dol%s_cap_%s_override",
};

/* For backward compatibility, still query old string */
char dolrnr_nv_var_fmt_str[DOL_NV_VAR_MAX_RNR][DOL_NV_VAR_STR_LEN] = {
	"dhd%s_rnr_flowring_profile",
	"dhd%s_rnr_flowring_policy",
	"dhd%s_rnr_rxoffl",
	"dhd%s_rnr_txoffl",
	"dhd%s_rnr_flowring_physize",
	"dhd%s_rnr_offload_override",
	"dhd%s_rnr_codel",
	/* DO NOT ADD any new var's here, add them in the new format above */
};

/* flowring profiles id vs profile */
dol_frm_profile_t dhdol_frm_profiles[DOL_FRM_PROFILE_MAX] = {
	/* User defined Profiles */
	{
	    false, 0, { 1, -1, -1, -1, 1 },                   /* Radio#0 */
	    DOL_TXP_ITEMS_LIST_DEF, DOL_TXP_BQSIZE_LIST_DEF
	}, {
	    false, 1, { 1, -1, -1, -1, 1 },                   /* Radio#1 */
	    DOL_TXP_ITEMS_LIST_DEF, DOL_TXP_BQSIZE_LIST_DEF
	}, {
	    false, 2, { 1, -1, -1, -1, 1 },                   /* Radio#2 */
	    DOL_TXP_ITEMS_LIST_DEF, DOL_TXP_BQSIZE_LIST_DEF
	}, { false, 3, { 1, -1, -1, -1, 1 },                   /* Radio#3 */
	    DOL_TXP_ITEMS_LIST_DEF, DOL_TXP_BQSIZE_LIST_DEF
	}, /* Built-in fixed profiles */ {
	    false, 4, { 1, -1, -1, -1, 1 },                   /* BA256 */
	    DOL_TXP_ITEMS_LIST_BA256, DOL_TXP_BQSIZE_LIST_BA256
	}, {
	    false, 5, { 1, -1, -1, -1, DOL_TXO_BCMC_PRIORITY }, /* Default */
	    DOL_TXP_ITEMS_LIST_DEF, DOL_TXP_BQSIZE_LIST_DEF
	}, {
	    false, 6, { 1, 1, 1, 1, 1 },            /* same ring size for all ac */
	    DOL_TXP_ITEMS_LIST_SAME, DOL_TXP_BQSIZE_LIST_DEF
	}, {
	    false, 7, { 1, 1, 1, 1, 1 },                       /* 2K ring+bq */
	    DOL_TXP_ITEMS_LIST_MAX, DOL_TXP_BQSIZE_LIST_MAX
	}
};

/* dhd radio vs flowring policy */
dol_frm_policy_t dhdol_frm_policies[DOL_RADIO_MAX] = {
	{ .id = dol_frm_policy_id_global, .all_hw = TRUE },
	{ .id = dol_frm_policy_id_global, .all_hw = TRUE },
	{ .id = dol_frm_policy_id_global, .all_hw = TRUE },
	{ .id = dol_frm_policy_id_global, .all_hw = TRUE }
};

/*
 * Default physical flow ring size when Helper backup queues are enabled
 * - per access category
 */
const uint16 dhdol_frm_txp_ringsz_defaults[] = {
	DOL_TXP_AC_BK_RINGSZ_MAX,
	DOL_TXP_AC_BE_RINGSZ_MAX,
	DOL_TXP_AC_VI_RINGSZ_MAX,
	DOL_TXP_AC_VO_RINGSZ_MAX,
	DOL_TXP_AC_BCMC_RINGSZ_MAX
};

struct dhd_offload_frm; /* forward declaration */
struct dhd_offload_obj;

typedef uint16
(dol_frm_selector_fn_t)(struct dhd_offload_frm *frm, int ifidx, int prio,
	uint8 *mac, int staidx, bool d11ac, bool *is_hw_ring);

/* flow ring information maintained by dhd offload */
typedef struct dol_flring_dhd_info
{
	void* base_va;     /* flowring descriptor base virtual address */
} dol_fr_cache_t;

/* flow ring id allocation mode */
#define DOL_FR_ALLOC_LEGACY                 0  /* based on id16 (Legacy) */
#define DOL_FR_ALLOC_IDMA                   1  /* Based on iDMA group */
#define DOL_FR_ALLOC_WMEAC                  2  /* Based on WME AC */
#define DOL_FR_ALLOC_MAX_MODES              3  /* Delimiter */
#define DOL_FR_ALLOC_MODE_STR_LEN           12 /* mode string size */

#define DOL_FR_ALLOC_DEFAULT                DOL_FR_ALLOC_WMEAC
static char
dhdol_frm_alloc_mode_str[DOL_FR_ALLOC_MAX_MODES][DOL_FR_ALLOC_MODE_STR_LEN] = {
	"Flat id16",
	"iDMA Group",
	"WME AC"
};

/* flow ring manager in dhd offload */
typedef struct dhd_offload_frm {
	struct dhd_offload_obj *dhdol;     /* DHD Offload object */

	/* Configuration information from DHD */
	int max_h2d_rings;                 /* total h2d rings, including common */
	int max_bss;                       /* total bcmc tx post flowrings */
	union {
	    int max_sta;                   /* total ucast txpost rings per ac */
	    int max_uc_rings;
	};

	/* Flow ring Reserved Memory */
	uint8       *rsv_mem_addr_base;    /* virtual address base */
	phys_addr_t rsv_mem_paddr_base;    /* physical address base */
	uint8       *rsv_mem_addr;         /* virtual address for allocation */
	uint32      rsv_mem_size;          /* memory size for allocation */

	/* Flow ring Cache */
	hlpr_fr_cache_t *hlpr_fr_cache;    /* Offload Helper Cache */
	dol_fr_cache_t *dhd_flring_cache;  /* DHD Cache */

	/* Flow ring allocation/selection Method */
	dol_frm_policy_t    *policy;       /* Flowring policy */
	dol_frm_profile_t   *profile;      /* Flowring profile */
	int                def_profile_id; /* Default profile id */

	/* Flow ring allocation/selection Management */
	int alloc_mode;                    /* flowid allocation mode */
	void *flow_ids_map;                /* flow id number pool */

	/* Below per AC arrays are reused for per TID */
	dol_frm_selector_fn_t *select_fn;  /* ring selection (hw/sw) function */
	void *hw_id16_map[wme_ac_max + 1]; /* id pool for Helper managed rings */
	void *sw_id16_map[wme_ac_max + 1]; /* id pool for DHD managed rings */

	int   hw_ring_cnt[wme_ac_max + 1]; /* Helper managed flow ring count */
	int   sw_ring_cnt[wme_ac_max + 1]; /* DHD managed flow ring count */
	int   phy_items[wme_ac_max + 1];   /* physical flow ring max size */

} dol_frm_t;


#define DOL_HLPR_TXS_AUDIT                    1

#if defined(MSGBUF_WI_COMPACT) && defined(DOL_HLPR_CAP_MSGRINGFRMT)
/*
 * Offload Helper supports only two formats
 *   0: Legacy
 *   1: CWI (TxPost::CWI64, RxPost::CWI32, TXCPL::CWI32, RXCPL::CWI32)
 */
#define DOL_HLPR_TXPOST_FMT(type) (((type) == MSGBUF_WI_WI64) ? \
					RDPA_DHD_TX_POST_MSGRING_FORMAT_WI64 : \
					RDPA_DHD_TX_POST_MSGRING_FORMAT_CWI32)

#ifdef DOL_HLPR_CAP_TXCPL_EWI
#define DOL_HLPR_TXCPL_FMT(ext)     (((ext) == MSGBUF_WI_EWI32) ? RDPA_DHD_FLOWRING_FORMAT_EXT : 0)
#else
#define DOL_HLPR_TXCPL_FMT(ext)     0
#endif

#define DOL_HLPR_FLOWRING_FMT(type, ext) \
	((((type) == MSGBUF_WI_WI64) ? RDPA_DHD_FLOWRING_FORMAT_WI64 : \
	RDPA_DHD_FLOWRING_FORMAT_CWI32) | DOL_HLPR_TXCPL_FMT(ext))

#else /* !MSGBUF_WI_COMPACT || !DOL_HLPR_CAP_MSGRINGFRMT */
#define DOL_HLPR_RINGFMT(txp_type, txc_type)    0
#endif /* !MSGBUF_WI_COMPACT || !DOL_HLPR_CAP_MSGRINGFRMT */

#if defined(DOL_HLPR_CAP_BKUPQUEUE)
#define DOL_BKUPQ_DEFAULT                   1
#define DOL_BKUPQ(dhdol)                    ((dhdol)->en_feat.bkupq == 1)
#define DOL_PHY_RING_SIZE(cache)            (cache)->phy_ring_size
#else /* !DOL_HLPR_CAP_BKUPQUEUE */
#define DOL_BKUPQ_DEFAULT                   0
#define DOL_BKUPQ(dhdol)                    0
#define DOL_PHY_RING_SIZE(cache)            (cache)->items
#endif /* !DOL_HLPR_CAP_BKUPQUEUE */

#define DOL_HLPR_CAP_EN_STS_STR(dol, feat)                             \
	(dol->sup_feat.feat ?                                              \
	    (dol->en_feat.feat ? #feat":1" : #feat":0") : #feat":0")

#define DOL_HLPR_CAP_STS_STR(dol, gfeat1, gfeat2, feat)                \
	(dol->sup_feat.feat ?                                              \
	    ((dol->en_feat.feat && \
	     (dol->en_feat.gfeat1 || dol->en_feat.gfeat2)) ?               \
	        #feat":1" : #feat":0") : #feat":X")

typedef dhd_offload_feat_t dol_hlpr_caps_t;

#define DOL_NV_CAP_SUPPORT(dol, feat)                                  \
({                                                                     \
	char buff[DOL_NV_UINT32_STR_LEN];                                  \
	int  supported = 1;                                                \
	snprintf(buff, sizeof(buff), "%s", #feat);                         \
	if (dol_nv_get(dol->dhd->unit,                                     \
	    DOL_NV_VAR_CAP_OVERRIDE, buff, sizeof(buff)) != 0) {           \
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


/*
 * +---------------------------------------------------------------------
 *           Section: Instance of a dhd offload object per radio
 * +---------------------------------------------------------------------
 */
/* DHD Offload local counters structure */
typedef struct dol_counters {
	/* From DHD Offload to Helper */
	ulong h2o_txp_notif;            /* #of Tx Posts */
	ulong h2o_txp_fail;             /* #of Tx Post failures */
	ulong h2o_rxc_notif;            /* #of process D2H RxCompl notifications */
	ulong h2o_txc_notif;            /* #of process D2H TxCompl notifications */

	/* From Helper to DHD offload */
	ulong o2h_rxc_req;              /* #of Process Rx Complete requests */
	ulong o2h_txc_req;              /* #of Process Tx Complete requests */
	ulong o2h_wkdngl_req;           /* #of Process Dongle Wakeup requests */
	ulong o2h_sq_txp_notif;         /* #of TxPost packets through Service Q */
} dol_cntr_t;

/* Message buffer ring configuration object structure */
typedef struct dol_cfg {
	dol_ring_cfg_t rxp_ring;        /* RXPOST ring configuration */
	dol_ring_cfg_t txp_ring;        /* TXPOST ring configuration */
	dol_ring_cfg_t rxc_ring;        /* RXCMPL ring configuration */
	dol_ring_cfg_t txs_ring;        /* TXSTS ring configuration */
} dol_cfg_t;

/* DHD offload object structure */
typedef struct dhd_offload_obj {
	dhd_pub_t       *dhd;           /* DHD public object */
	struct pci_dev  *pci_dev;       /* PCI device pointer */
	dol_cfg_t       cfg;            /* msgring configuration */

	phys_addr_t     pci_bar;        /* Offload Helper Wakeup Window Address */

	dol_hlpr_caps_t sup_feat;       /* Features supported by Offload helper */
	dol_hlpr_caps_t en_feat;        /* Features enabled in Offload helper */
	dol_swfeat_t    sw_feat;        /* Features enabled in Offload (software) */

	dhd_dma_buf_t   cmp;            /* Coherent memory pool object */
	dol_frm_t       frm;            /* Flow Ring Manager object */
	dhd_dma_buf_t   fr_cache_base;  /* DMA buffer object for flow ring cache */
	dol_hlpr_t      hlpr;           /* Offload helper hardware */

	bool            ipend;          /* Offload Helper cpuq interrupt pending */
	struct tasklet_struct dol_cpuq_tasklet; /* Tasklet for RXCMPL and TXSTS */
	dol_cntr_t      cntr;           /* local counters */

#ifdef DOL_MEM_ALLOC_AUDIT
	uint32 alloc_cnt[DOL_MEM_ALLOC_TYPE_MAX];   /* memory allocation count */
	uint32 alloc_size[DOL_MEM_ALLOC_TYPE_MAX];  /* allocated memory size */
#endif /* DOL_MEM_ALLOC_AUDIT */
} dhd_offload_t;


/**
 * +---------------------------------------------------------------------
 *           Section: Local Function Prototypes
 * +---------------------------------------------------------------------
 */

/**
 * +----------------------------------------------------------------------------
 *           Section: DMA Buffer Memory management (dol_mem)
 * +----------------------------------------------------------------------------
 */
static void* dol_mem_alloc_dmabuf(dhd_offload_t *dhdol, dhd_dma_buf_t *dma_buf,
	uint32 type);
static void dol_mem_free_dmabuf(dhd_offload_t *dhdol, dhd_dma_buf_t *dma_buf,
	uint32 type);
static uint32 dol_mem_get_ring_alloc_type(dhd_offload_t *dhdol,
	uint16 ringid);
static INLINE void* dol_mem_alloc_dmabuf_ring(dhd_offload_t *dhdol,
	dhd_dma_buf_t *dma_buf, uint32 ringid);
static INLINE void dol_mem_free_dmabuf_ring(dhd_offload_t *dhdol,
	dhd_dma_buf_t *dma_buf, uint32 ringid);

/**
 * +----------------------------------------------------------------------------
 *           Section: Non-Volatile Settings (dol_nv)
 * +----------------------------------------------------------------------------
 */
static int dol_nv_read_fmt_var(int unit, char *var_fmt, char *buff, int len);
static int dol_nv_read_var(char *var, char *buff, int len);
static int dol_nv_get(int radio_idx, int var_id, char *buff, int len);
static int dol_nv_get_uint(int unit, int var_id, uint32 *pvalue);

/**
 * +----------------------------------------------------------------------------
 *           Section: Offload Helper (dol_hlpr)
 * +----------------------------------------------------------------------------
 */
static int dol_hlpr_isr_wake_dngl(int irq, void *data);
static void dol_hlpr_isr_cpuq(long data);
static int dol_hlpr_txpost(dhd_offload_t *dhdol, void *txp, uint32 ifindex);
static int dol_hlpr_txpost_spdsvc(dhd_offload_t *dhdol, void *txp,
	hlpr_txp_info_t *info);
static dol_hlpr_t* dol_hlpr_attach(dhd_offload_t *dhdol);
static void dol_hlpr_detach(dhd_offload_t *dhdol);

/**
 * +----------------------------------------------------------------------------
 *           Section: Offload Initialization (dol_init)
 * +----------------------------------------------------------------------------
 */
static int dol_init_dngl_wakeup(dhd_offload_t *dhdol);
static int dol_init_llcsnaphdr(dhd_offload_t *dhdol);
static int dol_init_txc2host(dhd_offload_t *dhdol, hlpr_obj_handle mo);
static int dol_init_ba256cfg(dhd_offload_t *dhdol);
static int dol_init_swfeatures(struct dhd_offload_obj *dhdol);
static int dol_init_codel(struct dhd_offload_obj *dhdol);
static int dol_init_hlpr_wakeup(dhd_offload_t *dhdol,
	bcmpcie_soft_doorbell_t *soft_doorbell, phys_addr_t wakeup_paddr,
	uint32 wakeup_val32);
static int dol_init_pcie(dhd_offload_t *dhdol, struct pci_dev *pci_dev);
static void dol_deinit_pcie(dhd_offload_t *dhdol);
static int dol_init_rxoffl(struct dhd_offload_obj *dhdol,
	dol_ring_cfg_t *rxc_ring_cfg, dol_ring_cfg_t *rxp_ring_cfg);
static int dol_init_txoffl(struct dhd_offload_obj *dhdol,
	dol_ring_cfg_t *ring_cfg);

static int dol_disable_offload(struct dhd_offload_obj *dhdol, bool tx_dor,
	bool rx_dor);
/**
 * +----------------------------------------------------------------------------
 *           Section: Offload Operations (Notification/Requests) (dol_ops)
 * +----------------------------------------------------------------------------
 */
static int  dol_ops_dma_buf_notif(dhd_offload_t *dhdol, void *osh,
	dhd_offload_dma_buf_t buf_type, dhd_dma_buf_t *dma_buf);
static int  dol_ops_init_notif(dhd_offload_t *dhdol, struct pci_dev *pci_dev);
static int dol_ops_fr_enab_notif(dhd_offload_t *dhdol, uint32_t ringid,
	int enable);
static int dol_ops_msgring_fmt_notif(dhd_offload_t *dhdol, unsigned long arg1,
	unsigned long arg2);
static int dol_ops_aggr_config_notif(dhd_offload_t *dhdol, unsigned long arg1,
	unsigned long arg2);
static int dol_ops_fr_bqsize_notif(dhd_offload_t *dhdol, uint16_t ringid,
	uint32_t bqsize);
static int dol_ops_pcie_ipc_cap_notif(dhd_offload_t *dhdol, unsigned long arg1,
	unsigned long arg2);
static int dol_ops_wake_dngl_req(dhd_offload_t *dhdol, uint32 db_reg,
	uint32 db_val);

/**
 * +----------------------------------------------------------------------------
 *           Section: Flow Ring Manager (dol_frm)
 * +----------------------------------------------------------------------------
 */
static void dol_frm_get_policy_str(dol_frm_policy_t *policy, char *buf, int len);
static uint16 dol_frm_select_fr(dol_frm_t *frm, int prio, bool bcmc,
	bool prefer_hw_ring, bool *is_hw_ring);
static uint16 dol_frm_select_policy(dol_frm_t *frm, int ifidx, int prio,
	uint8 *mac, int staidx, bool d11ac, bool *is_hw_ring);
static int dol_frm_alloc_fr_id(dol_frm_t *frm, dhd_wme_ac_t wme_ac,
	bool force_dhd);
static int dol_frm_init_fr(dol_frm_t *frm, uint16 flow_id);
static int dol_frm_init_fr_map(dol_frm_t *frm);
static dol_frm_profile_t* dol_frm_init_profile(struct dhd_offload_obj *dhdol);
static dol_frm_policy_t* dol_frm_init_policy(struct dhd_offload_obj *dhdol);

/**
 * +----------------------------------------------------------------------------
 *           Section: Offload IOVAR processing (dol_iov)
 * +----------------------------------------------------------------------------
 */
static int dol_iov_get_profile(dhd_offload_t *dhdol, char *buf, int buflen);
static int dol_iov_get_policy(dhd_offload_t *dhdol, char *buf, int buflen);
static int dol_iov_get_rxoffl(dhd_offload_t *dhdol, char *buf, int bufflen);
static int dol_iov_get_hlpr_stats(dhd_offload_t *dhdol, char *buf, int buflen);
static int dol_iov_dump(dhd_offload_t *dhdol, char *buff, int bufflen);
static int dol_iov_get_status(dhd_offload_t *dhdol, char *buf, int buflen);
static int dol_iov_set_unsupported(dhd_offload_t *dhdol, char *buf, int buflen);

typedef int
(dhdol_iov_fn_t)(dhd_offload_t *dhdol, char *buf, int buflen);

dhdol_iov_fn_t * dhdol_iov_fn_tbl[DHD_OFFLOAD_IOV_MAX+1] = {
		/* IOV_GET */
	    dol_iov_get_profile,           /* DHD_OFFLOAD_IOV_PROFILE */
	    dol_iov_get_policy,            /* DHD_OFFLOAD_IOV_POLICY */
	    dol_iov_get_rxoffl,            /* DHD_OFFLOAD_IOV_RXOFFL */
	    dol_iov_get_hlpr_stats,        /* DHD_OFFLOAD_IOV_STATS */
	    dol_iov_dump,                  /* DHD_OFFLOAD_IOV_DUMP */
	    dol_iov_get_status,            /* DHD_OFFLOAD_IOV_STATUS */

		/* IOV_SET */
		dol_iov_set_unsupported,       /* DHD_OFFLOAD_IOV_XXXX */
};

/**
 * +----------------------------------------------------------------------------
 *           Section: Offload Data Path processing (dol_dp)
 * Helper CPU queue processing for non-accelerated (flow-miss) packets
 * +----------------------------------------------------------------------------
 */
static bool dol_dp_process_cpuq_rxc(dhd_offload_t *dhdol, int bound);
static bool dol_dp_process_cpuq_txs(dhd_offload_t *dhdol, int bound);

#if defined(DOL_HLPR_CAP_PCAP)
static bool dol_dp_process_cpuq_pcap(dhd_offload_t *dhdol, int type, int bound);
#endif /* DOL_HLPR_CAP_PCAP */

static bool dol_dp_process_cpuq(dhd_offload_t *dhdol, bool resched);
static void dol_dp_cpuq_tasklet(unsigned long data);

/**
 * +----------------------------------------------------------------------------
 *           Section: Global Structures
 * +----------------------------------------------------------------------------
 */
typedef struct dhdol {
	bool up_inited;                    /* user profiles initializion status */
} dhdol_t;

static struct dhdol dhdol_cb = { 0 };

/*
 * +----------------------------------------------------------------------------
 *           Section: DMA Buffer Memory management (dol_mem)
 *
 * memory allocation functions for dma idx, ring descriptor memories
 *
 * memory types
 *   DHD: cached memory used by DHD driver only. Not shared by Offload Helper
 *        uses DMA_ALLOC_CONSISTENT/DMA_FREE_CONSISTENT osl functions
 *        this internally uses kmalloc/kfree with flush_cache operations
 *        sw TX flowrings, ctrlpost, ctrlcmpl rings
 *
 *   HLPR: un cached memory used only by Offload Helper.
 *        uses dma_alloc_coherent/dma_free_coherent functions
 *        cached flowrings, ring DMA index buffers
 *        applicable for arm/arm64 platforms
 *
 *   HLPRVA: Used by Offload Helper only (address <256MB).
 *           rdp uses virt_to_phys on this memory to get physical address
 *           which dma_alloc_coherent can not provide, so a different type
 *           uses kmalloc/kfree with GFP_DMA flag
 *           ring descriptor memory for txcmpl,rxcmpl,rxpost
 *           applicable for arm/arm64 platforms
 *
 * +----------------------------------------------------------------------------
 */
void*
dol_mem_alloc_dmabuf(dhd_offload_t *dhdol, dhd_dma_buf_t *dma_buf, uint32 type)
{
	int alloced;
	dhd_pub_t *dhd;

	dhd = dhdol->dhd;
	dma_buf->va = NULL;
	dma_buf->dmah = NULL;

	if (type == DOL_MEM_ALLOC_TYPE_COHERENT) {
	    dhd_dma_buf_t* coherent_pool = &dhdol->cmp;
	    int aligned_len;
	    uint32 pa_low;

	    aligned_len = (dma_buf->len + DOL_CMP_ALIGN_MASK);
	    aligned_len &= ~(DOL_CMP_ALIGN_MASK);

	    if ((coherent_pool->va == NULL) ||
	        (coherent_pool->len < (coherent_pool->_alloced + aligned_len))) {
	        /* No Memory in the Local Coherent Pool */
	        ARCH_ALLOC_COHERENT_MEM(dhd, dma_buf);

	        /* Offload engine supports 32bit physical address only */
	        if (PHYSADDRHI(dma_buf->pa)) {
	            DOL_ERROR("[COHERENT_POOL_%d] Allocated 0x%x size 0x%px, "
	                "[0x%x_%x] 0x%px from upper 2GB\n",
	                DOL_UNIT, dma_buf->_alloced, dma_buf->va,
	                (uint32)PHYSADDRHI(dma_buf->pa),
	                (uint32)PHYSADDRLO(dma_buf->pa), coherent_pool->va);
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
	        DOL_INFO("[COHERENT_POOL_%d] Allocated 0x%x size 0x%px, 0x%x_%x "
	            "from pool 0x%px\n",
	            DOL_UNIT, dma_buf->_alloced, dma_buf->va,
	            (uint32)PHYSADDRHI(dma_buf->pa),
	            (uint32)PHYSADDRLO(dma_buf->pa), coherent_pool->va);
	    }
	} else if (type == DOL_MEM_ALLOC_TYPE_KMALLOC) {
	    dma_buf->va = kmalloc(dma_buf->len, GFP_ATOMIC | __GFP_ZERO | GFP_DMA);
	    ARCH_SET_DMA_BUF_PA(dhd->osh, dma_buf);
	} else	{
	    dma_buf->va = DMA_ALLOC_CONSISTENT(dhd->osh, dma_buf->len,
	        4, &alloced, &dma_buf->pa, &dma_buf->dmah);
	}

	if (dma_buf->va != NULL) {
	    memset(dma_buf->va, 0, dma_buf->len);
	    if (type != DOL_MEM_ALLOC_TYPE_COHERENT)
	        WLAN_OSL_CACHE_FLUSH(dhd->osh, dma_buf->va, dma_buf->len);

	    DOL_MEM_ALLOC_CNTR_INC(dhdol, type, dma_buf->len);
	}

	return dma_buf->va;
}

/**
 * Free the memory allocated for the ring descriptor and indices buffers
 */
void
dol_mem_free_dmabuf(dhd_offload_t *dhdol, dhd_dma_buf_t *dma_buf, uint32 type)
{
	if (dma_buf->va != NULL) {
	    dhd_pub_t *dhd;

	    dhd = dhdol->dhd;
	    if (type == DOL_MEM_ALLOC_TYPE_COHERENT) {
	        dhd_dma_buf_t* coherent_pool = &dhdol->cmp;

	        if ((dma_buf == coherent_pool) ||
	            ((dma_buf->va <  coherent_pool->va) &&
	            (dma_buf->va >= (coherent_pool->va + coherent_pool->len)))) {
	            /* Local Coherent pool itself or Outside of Pool */
	            ARCH_FREE_COHERENT_MEM(dhd, dma_buf);
	        }
	    } else if (type == DOL_MEM_ALLOC_TYPE_KMALLOC) {
	        kfree(dma_buf->va);
	    } else {
	        DMA_FREE_CONSISTENT(dhd->osh, dma_buf->va, dma_buf->len,
	            dma_buf->pa, dma_buf->dmah);
	    }
	    DOL_MEM_ALLOC_CNTR_DEC(dhdol, type, dma_buf->len);
	    memset(dma_buf, 0, sizeof(dhd_dma_buf_t));
	}

	return;
}

/**
 * Returns the memory type to be allocated based on ring id
 */
uint32
dol_mem_get_ring_alloc_type(dhd_offload_t *dhdol, uint16 ringid)
{
	uint32 type = DOL_MEM_TYPE_DHD;

	if (DOL_RXO_ENABLED(dhdol)) {
	    if ((ringid == BCMPCIE_H2D_MSGRING_RXPOST_SUBMIT) ||
	        (ringid == BCMPCIE_D2H_MSGRING_RX_COMPLETE))
	        type = DOL_MEM_TYPE_HLPRVA;
	}

	if (DOL_TXO_ENABLED(dhdol)) {
	    if (ringid == BCMPCIE_D2H_MSGRING_TX_COMPLETE)
	        type = DOL_MEM_TYPE_HLPRVA;
	}

	return type;

}

/**
 * Allocate dma buffer for the specifed ring
 * internally determines the dma buffer type to be used
 */
INLINE void*
dol_mem_alloc_dmabuf_ring(dhd_offload_t *dhdol, dhd_dma_buf_t *dma_buf,
	uint32 ringid)
{
	uint32 type = dol_mem_get_ring_alloc_type(dhdol, ringid);

	return dol_mem_alloc_dmabuf(dhdol, dma_buf, type);
}

/**
 * Free dma buffer for the specifed ring
 * internally determines the dma buffer type to be used
 */
INLINE void
dol_mem_free_dmabuf_ring(dhd_offload_t *dhdol, dhd_dma_buf_t *dma_buf,
	uint32 ringid)
{
	uint32 type = dol_mem_get_ring_alloc_type(dhdol, ringid);
	return dol_mem_free_dmabuf(dhdol, dma_buf, type);
}

/*
 * +----------------------------------------------------------------------------
 *           Section: Non-Volatile Settings (dol_nv)
 * +----------------------------------------------------------------------------
 */
/**
 * Read nvram setting from the persistent storage area given and copy the value
 * in the given buffer if exists with the below hierarchy
 *     - uboot setting
 *     - nvram setting
 *
 * input
 *   var:       nvram setting string
 *   buff:      pointer where setting value to be written
 *   len:       size of 'buff'
 *
 * output
 *   0:  nvram setting is not present in storage area
 *   >0: length of the nvram setting value in storage area
*/
int
dol_nv_read_var(char *var, char *buff, int len)
{
	char val_str[DOL_NV_VAL_STR_LEN];
	char *var_val = NULL;

	/* Read uboot environment first */
	memset(val_str, 0, sizeof(val_str));
	if (eNvramGet(var, val_str, DOL_NV_VAL_STR_LEN) > 0) {
	    var_val = val_str;
	} else {
	    var_val = nvram_get(var);
	}

	DHD_INFO(("%s(%s)=[%s]\n", __FUNCTION__, var, (var_val) ? var_val : ""));

	if (var_val) {
	    /* Copy the nvram setting value */
	    snprintf(buff, len, "%s", var_val);
	    len = strlen(var_val);
	    if (len < strlen(var_val)) {
	        DHD_ERROR(("setting value [%s] exceeds buffer len [%d]\r\n",
	            var_val, len));
	    }
	} else {
	    /* No setting present in the NVRAM */
	    len = 0;
	}

	return len;
}

/**
 * Read nvram setting from the persistent storage area given the format
 *   - Radio Specific dol%d_xxx
 *     - uboot setting
 *     - nvram setting
 *   - All Radio dol_xxx
 *     - uboot setting
 *     - nvram setting
 *
 * input
 *   unit:      WLAN radio index
 *   var_fmt:   nvram setting format
 *   buff:      pointer where setting value to be written
 *   len:       size of 'buff'
 *
 * output
 *   0:  nvram setting is not present in storage area
 *   >0: lenght of the nvram setting value in storage area
*/
int
dol_nv_read_fmt_var(int unit, char *var_fmt, char *buff, int len)
{
	char var[DOL_NV_VAR_STR_LEN];
	char unit_str[2];
	int length = 0;

	/* Prepare the setting string first with unit, if valid */
	if (unit != DOL_NV_UNIT_ALL) {
	    snprintf(unit_str, sizeof(unit_str), "%d", unit);
	    if (len && buff[0] != 0) {
	        snprintf(var, sizeof(var), var_fmt, unit_str, buff);
	    } else {
	        snprintf(var, sizeof(var), var_fmt, unit_str);
	    }

	    /* Check if radio specific nvram setting is present */
	    length = dol_nv_read_var(var, buff, len);
	}

	/* if not, check if all radio setting is present */
	if (length == 0) {
	    if (len && buff[0] != 0) {
	        snprintf(var, sizeof(var), var_fmt, "", buff);
	    } else {
	        snprintf(var, sizeof(var), var_fmt, "");
	    }

	    /* Check if all radio nvram setting is present */
	    length = dol_nv_read_var(var, buff, len);
	}

	return length;
}

/**
 * Read nvram setting from the persistent storage area using below hierarchey
 * - New format dol%d_xxx
 *   - Radio Specific dol%d_xxx
 *     - uboot setting
 *     - nvram setting
 *   - All Radio dol_xxx
 *     - uboot setting
 *     - nvram setting
 * - Fall back to old format dhd%d_rnr_xxx
 *   - Radio Specific dhd%d_rnr
 *     - uboot setting
 *     - nvram setting
 *   - All Radio dhd_rnr_xxx
 *     - uboot setting
 *     - nvram setting
 *
 * input
 *   unit:      WLAN radio index
 *   var_id:    nvram setting id
 *   buff:      pointer where setting value to be written
 *   len:       size of 'buff'
 *
 * output
 *   0:  setting is not present in storage area
 *   >0: lenght of the setting in storage area
*/
int
dol_nv_get(int unit, int var_id, char *buff, int len)
{
	char *var_fmt_str;
	int length = 0;

	if (var_id >= DOL_NV_VAR_MAX)
	    return length;

	/* Check if setting is present in new dol%d_xxxxx format */
	var_fmt_str = dhdol_nv_var_fmt_str[var_id];
	length = dol_nv_read_fmt_var(unit, var_fmt_str, buff, len);

	/* if not, check if setting is present in old format (dhd%d_rnr_xxxxx */
	if ((length == 0) && (var_id < DOL_NV_VAR_MAX_RNR)) {
	    var_fmt_str = dolrnr_nv_var_fmt_str[var_id];
	    length = dol_nv_read_fmt_var(unit, var_fmt_str, buff, len);
	}

	return length;
}

/**
 * Read nvram setting of type 'uint'
 *
 * input
 *   unit:      WLAN radio index
 *   var_id:    nvram variable identifier
 *   pvalue:    pointer where uint value to be written
 *
 * output
 *   BCME_OK: on successful read read
 *   BCME_NOTFOUND: on setting not available
*/
static int
dol_nv_get_uint(int unit, int var_id, uint32 *pvalue)
{
	char buff[DOL_NV_UINT32_STR_LEN];
	int  ret = BCME_NOTFOUND;

	/* Fetch the setting information, if present */
	memset(buff, 0, sizeof(buff));
	if (dol_nv_get(unit, var_id, buff, sizeof(buff)) != 0) {
	    /* Convert string to uint value */
	    sscanf(buff, "%d", pvalue);
	    ret = BCME_OK;
	}

	return ret;
}

/*
 * +----------------------------------------------------------------------------
 *           Section: Offload Helper (dol_hlpr)
 * +----------------------------------------------------------------------------
 */

/**
 * ISR handler attached to the Offload Helper to wakeup dongle
 */
static int
dol_hlpr_isr_wake_dngl(int irq, void *data)
{
	dhd_offload_t *dhdol = (dhd_offload_t *)data;

	dol_hlpr_clear_db_int(&dhdol->hlpr);

	if (dhdol->dhd->busstate == DHD_BUS_DOWN) {
	    DOL_INFO("%s: dol%d: PCIe bus down, ret\n", __FUNCTION__, DOL_UNIT);
	} else {
	    dhd_offload_request(dhdol, O2D_WAKE_DNGL_REQUEST, 0, 0);
	}

	return BDMF_IRQ_HANDLED;
}

/**
 * ISR handler attached to the Offload Helper to Host cpu queue
 *
 * A miss in Helper will result in Helper forwarding the packet to DHD via a
 * dedicated CPU queue and raising an interrupt.
 *
 * dol_hlpr_isr_cpuq() will be invoked and a tasklet will be scheduled to
 * process all packets in this Offload Helper to CPU queue in the context of
 * the dhd.
 *
 */
static void
dol_hlpr_isr_cpuq(long data)
{
	dhd_offload_t *dhdol = (dhd_offload_t *)data;
	int cpuq_id;

	ASSERT(dhdol != NULL);

	cpuq_id = dhdol->hlpr.cpuq_id;

	/* Disable and acknowledge the interrupt */
	dol_hlpr_disable_cpuq_int(&dhdol->hlpr);
	dol_hlpr_clear_cpuq_int(&dhdol->hlpr);

	dhdol->ipend = TRUE;

	if (dhdol->sw_feat.cpuqdpc == TRUE) {
	    /* Process packets in the dhd dpc context */
	    dhd_sched_dpc(dhdol->dhd);
	} else {
	    tasklet_schedule(&dhdol->dol_cpuq_tasklet);
	}
}

/**
 * Send TxPost Speed Service Packet to dongle through Offload Helper
 *
 */
static inline int
dol_hlpr_txpost_spdsvc(dhd_offload_t *dhdol, void *txp, hlpr_txp_info_t *info)
{
#if IS_ENABLED(CONFIG_BCM_DHD_ARCHER)
#if IS_ENABLED(CONFIG_BCM_SPDSVC)
	if (IS_SKBUFF_PTR(txp)) {
	    struct sk_buff *skb = PNBUFF_2_SKBUFF(txp);
	    spdsvcHook_transmit_t spdsvc_transmit = {};
	    dhd_pub_t *dhd = dhdol->dhd;
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
	    tag.rnr.llcsnap_flag = dhdol->hlpr.init_cfg.add_llcsnap_header;

	    spdsvc_transmit.tag = tag.u32;

	    rc = dhdol_spdsvc_transmit(&spdsvc_transmit);
	    if (unlikely(rc > 0)) {
	        // Buffer was hijacked by spdsvc, abort transmission
	        rc = -1;
	    }

	    return rc;
	}
#endif /* CONFIG_BCM_SPDSVC */
	return 0;
#else /* !CONFIG_BCM_DHD_ARCHER */
#if IS_ENABLED(CONFIG_BCM_SPDSVC)
	spdsvcHook_transmit_t spdsvc_transmit = {};

	spdsvc_transmit.pNBuff = txp;
	spdsvc_transmit.dev = NULL;
	spdsvc_transmit.header_type = SPDSVC_HEADER_TYPE_ETH;
	spdsvc_transmit.phy_overhead = WL_SPDSVC_OVERHEAD;
	spdsvc_transmit.so_mark = 0;
	spdsvc_transmit.egress_type = SPDSVC_EGRESS_TYPE_DHD_OFFLOAD;
	spdsvc_transmit.transmit_helper = NULL;

	info->is_spdsvc_setup_packet = dhdol_spdsvc_transmit(&spdsvc_transmit);

	info->spdt_so_mark = spdsvc_transmit.so_mark;
#else /* !CONFIG_BCM_SPDSVC */
	info->is_spdsvc_setup_packet = 0;
#endif /* !CONFIG_BCM_SPDSVC */

	return info->is_spdsvc_setup_packet;
#endif /* !CONFIG_BCM_DHD_ARCHER */
}

/**
 * Send TxPost Packet to dongle through Offload Helper
 *
 */
int
dol_hlpr_txpost(dhd_offload_t *dhdol, void *txp, uint32 ifindex)
{
	int rc;
	dhd_pub_t *dhd = dhdol->dhd;
	int pktlen = PKTLEN(dhd->osh, txp);
	hlpr_txp_info_t info = {};

	info.radio_idx = DOL_RADIO_IDX;
	info.flow_ring_id = DHD_PKT_GET_FLOWID(txp);
	info.ssid_if_idx = ifindex;

#if defined(MLO_BCMC)
	if (DHD_PKT_GET_MLO_BCMC(txp)) {
	    uint16 bcmc_seqno = DHD_MLO_BCMC_PKTTAG_SEQ(txp);

	    DOL_INFO("dol%d: BCMCSEQ seq 0x%X\n", DOL_UNIT, bcmc_seqno);
	    ASSERT(MLO_BCMC_SEQNO_ISVALID(bcmc_seqno));

	    /* Assign bcmc_seqno to info.bcmc_seqno */
	    info.seqnum = bcmc_seqno;
	}
#endif /* MLO_BCMC */

	rc = dol_hlpr_txpost_spdsvc(dhdol, txp, &info);
	if (unlikely(rc < 0)) {
	    /* In case of error, NBuff will be free by spdsvc */
	    return 0;
	}

	rc = dol_hlpr_send_txp(&dhdol->hlpr, txp, pktlen, &info);
	if (rc > 0) {
	    /*
	     * Sucessful return from service queue (RC_PKT_SENT_TO_SQ)
	     * Offload engine will own the Txp buffer and will not send
	     * TxComplete message back
	     */
	    rc = 0;
	    dhdol->cntr.o2h_sq_txp_notif++;
	}

	if (rc != 0) {
	    DOL_TRACE("dor%d: dol_hlpr_send_txp (0x%px, %d, 0x%px) returned %d\n",
	        DOL_UNIT, txp, pktlen, &info, rc);
	    dhdol->cntr.h2o_txp_fail++;
	} else {
	    dhdol->cntr.h2o_txp_notif++;
	}

	return rc;
}

/**
 * Attach Offload Helper
 *
 */
static dol_hlpr_t*
dol_hlpr_attach(dhd_offload_t *dhdol)
{
	uint32_t radio_idx;
	uint32 sup_mask = 0;
	uint32 ovrd_mask = 0;
	uint32 radio_mask = 0;
	dol_hlpr_t *hlpr;

	DOL_INFO("%s: dhdol<%px>\n", __FUNCTION__, dhdol);

	/* DHD unit is radio index */
	radio_idx = dol_hlpr_get_radio_idx(DOL_UNIT);

	hlpr = &dhdol->hlpr;
	hlpr->radio_idx = radio_idx;
	hlpr->dhdol = dhdol;

	radio_mask = (1UL << radio_idx);

	/*
	 * Create bitmask of default supported radios configuration by the
	 * Offload engine
	 *
	 * Radio# 0 .. DOL_RADIO_CONCUR_OFFL_MAX-1
	 */
	{
	    int unit = 0;
	    while (unit < DOL_RADIO_OFFL_MAX) {
	        ovrd_mask |= (1 << unit);
	        if (unit < DOL_RADIO_CONCUR_OFFL_MAX) {
	            sup_mask |= (1 << unit);
	        }
	        unit++;
	    }
	}

	/*
	 * Fetch the offload override configuration information of the radio,
	 * if present
	 */
	if (dol_nv_get_uint(DOL_NV_UNIT_ALL, DOL_NV_VAR_OFFL_OVERRIDE, &ovrd_mask)
	    != BCME_NOTFOUND) {
	    /* Offload override setting is present, Check the setting */
	    /*
	     * If Offload engine supports less than Max radios, the following two
	     * configurations are supported
	     *
	     * Radio# 0 .. DOL_RADIO_CONCUR_OFFL_MAX-1 (existing sup_mask)
	     * Radio# 1 .. DOL_RADIO_CONCUR_OFFL_MAX
	     */
#if (DOL_RADIO_CONCUR_OFFL_MAX < DOL_RADIO_OFFL_MAX)
	    if ((ovrd_mask & 0x1) == 0) {
	        /*
	         * switch to non-default configuration
	         * Radio# 1 .. DOL_RADIO_CONCUR_OFFL_MAX
	         */
	        sup_mask <<= 1;
	    }
#endif /* (DOL_RADIO_CONCUR_OFFL_MAX < DOL_RADIO_OFFL_MAX) */
	}

	if (!(sup_mask & ovrd_mask & radio_mask)) {
	    /*
	     * Offload engine support is not enabled for this radio
	     * dhd dol instance still exists, to support api's
	     * but offload is disabled completely.
	     */
	    DOL_ERROR("dol%d: not supported, hw [0x%1x] nv [0x%1x] radio[0x%1x]\n",
	        radio_idx, sup_mask, ovrd_mask, radio_mask);
	    memset(&dhdol->sup_feat, 0, sizeof(dol_hlpr_caps_t));
	    memset(&dhdol->en_feat, 0, sizeof(dol_hlpr_caps_t));
	    return hlpr;
	}

	/* Base features are enabled by default */
	dhdol->sup_feat.txoffl = 1;
	dhdol->sup_feat.rxoffl = 1;

	/* Extra features are enabled conditionally */
#if defined(DOL_HLPR_CAP_TXCMPL2HOST)
	DOL_HLPR_CAP_SUPPORT_INIT(dhdol, txcmpl2host);
#endif /* DOL_HLPR_CAP_TXCMPL2HOST */

#if defined(DOL_HLPR_CAP_LLCSNAPHDR)
	DOL_HLPR_CAP_SUPPORT_INIT(dhdol, dhdhdr);
#endif /* DOL_HLPR_CAP_LLCSNAPHDR */

#if defined(DOL_HLPR_CAP_LBRAGGR)
	DOL_HLPR_CAP_SUPPORT_INIT(dhdol, lbraggr);
#endif /* DOL_HLPR_CAP_LBRAGGR */

	dhdol->sup_feat.nplusm = DOL_NPM_DEFAULT;
#if defined(DOL_HLPR_CAP_NPLUSM)
	DOL_HLPR_CAP_SUPPORT_INIT(dhdol, nplusm);
#endif /* DOL_HLPR_CAP_NPLUSM */

	/* Enable legacy type for all rings MSGBUF_WI_WI64 */
	dhdol->cfg.txp_ring.sup_types = 0x1;
	dhdol->cfg.rxp_ring.sup_types = 0x1;
	dhdol->cfg.txs_ring.sup_types = 0x1;
	dhdol->cfg.rxc_ring.sup_types = 0x1;

#if defined(DOL_HLPR_CAP_MSGRINGFRMT)
	DOL_HLPR_CAP_SUPPORT_INIT(dhdol, msgringformat);

#if defined(MSGBUF_WI_COMPACT)
	/* For TxPost add CWI32/CWI64 based on the helper support flag */
#if defined(RDPA_DHD_HELPER_MIXED_CWI64_CWI32_MSGFORMAT_SUPPORT)
	dhdol->cfg.txp_ring.sup_types |= (1ul << MSGBUF_WI_CWI64);
#else /* !RDPA_DHD_HELPER_MIXED_CWI64_CWI32_MSGFORMAT_SUPPORT */
	dhdol->cfg.txp_ring.sup_types |= (1ul << MSGBUF_WI_CWI32);
#endif /* !RDPA_DHD_HELPER_MIXED_CWI64_CWI32_MSGFORMAT_SUPPORT */

	/* For rest, add CWI32 format support */
	dhdol->cfg.rxp_ring.sup_types |= (1ul << MSGBUF_WI_CWI32);
	dhdol->cfg.rxc_ring.sup_types |= (1ul << MSGBUF_WI_CWI32);
	dhdol->cfg.txs_ring.sup_types |= (1ul << MSGBUF_WI_CWI32);
#endif /* MSGBUF_WI_COMPACT */

#if defined(DOL_HLPR_CAP_TXCPL_EWI)
	dhdol->cfg.txs_ring.sup_types |= (1ul << MSGBUF_WI_EWI32);
#endif /* DOL_HLPR_CAP_TXCPL_EWI */
#endif /* DOL_HLPR_CAP_MSGRINGFRMT */

	DOLOG("dol%d: %s supported types TxP 0x%x, RxP 0x%x TxC 0x%x RxC 0x%x",
	    radio_idx, DOL_HLPR,
	    dhdol->cfg.txp_ring.sup_types, dhdol->cfg.rxp_ring.sup_types,
	    dhdol->cfg.txs_ring.sup_types, dhdol->cfg.rxc_ring.sup_types);

#if defined(DOL_HLPR_CAP_BKUPQUEUE)
	DOL_HLPR_CAP_SUPPORT_INIT(dhdol, bkupq);
#endif /* DOL_HLPR_CAP_BKUPQUEUE */

#if defined(DOL_HLPR_CAP_HWA_WAKEUP)
	DOL_HLPR_CAP_SUPPORT_INIT(dhdol, hwawkup);
#endif /* DOL_HLPR_CAP_HWA_WAKEUP */

#if defined(DOL_HLPR_CAP_FFRD)
	DOL_HLPR_CAP_SUPPORT_INIT(dhdol, ffrd);
#endif /* DOL_HLPR_CAP_FFRD */

#if defined(DOL_HLPR_CAP_HBQD)
	DOL_HLPR_CAP_SUPPORT_INIT(dhdol, hbqd);
#endif /* DOL_HLPR_CAP_HBQD */

#if defined(DOL_HLPR_CAP_DYNBKUPQ)
	DOL_HLPR_CAP_SUPPORT_INIT(dhdol, dynbkupq);
#endif /* DOL_HLPR_CAP_DYNBKUPQ */

#if defined(DOL_HLPR_CAP_CODEL)
	DOL_HLPR_CAP_SUPPORT_INIT(dhdol, codel);
#endif /* DOL_HLPR_CAP_CODEL */

#if defined(DOL_HLPR_CAP_CMNRNGSZ)
	DOL_HLPR_CAP_SUPPORT_INIT(dhdol, cmnrngsz);
#endif /* DOL_HLPR_CAP_CMNRNGSZ */

#if defined(DOL_HLPR_CAP_RXCMPLV2)
	DOL_HLPR_CAP_SUPPORT_INIT(dhdol, rxcmplv2);
#endif /* DOL_HLPR_CAP_RXCMPLV2 */

#if defined(DOL_HLPR_CAP_RXCMPLRCH)
	DOL_HLPR_CAP_SUPPORT_INIT(dhdol, rxcmplrch);
#endif /* DOL_HLPR_CAP_RXCMPLRCH */

#if defined(DOL_HLPR_CAP_BA256CFG)
	DOL_HLPR_CAP_SUPPORT_INIT(dhdol, ba256cfg);
#endif /* DOL_HLPR_CAP_BA256CFG */

#if defined(DOL_HLPR_CAP_IDMA64)
	DOL_HLPR_CAP_SUPPORT_INIT(dhdol, idma64);
#endif /* DOL_HLPR_CAP_IDMA64 */

#if defined(DOL_HLPR_CAP_IDMAGRPCFG)
	DOL_HLPR_CAP_SUPPORT_INIT(dhdol, idmagrpcfg);
	if (dhdol->sup_feat.idmagrpcfg == 1) {
	    dhdol->en_feat.idmagrpcfg = 1;
	}
#endif /* DOL_HLPR_CAP_IDMAGRPCFG */

#if defined(DOL_HLPR_CAP_PCAP)
	DOL_HLPR_CAP_SUPPORT_INIT(dhdol, pcap);
#endif /* DOL_HLPR_CAP_PCAP */

#if defined(DOL_HLPR_CAP_DUMMYTXC)
	DOL_HLPR_CAP_SUPPORT_INIT(dhdol, dummytxc);
#endif /* DOL_HLPR_CAP_DUMMYTXC */

#if defined(DOL_HLPR_CAP_TXCPL_EWI)
	DOL_HLPR_CAP_SUPPORT_INIT(dhdol, txcpl_ewi);
#endif /* DOL_HLPR_CAP_TXCPL_EWI */

	return hlpr;
}

/**
 * Detach Offload Helper object
 *
 */
static void
dol_hlpr_detach(dhd_offload_t *dhdol)
{
	dol_hlpr_t *hlpr = &dhdol->hlpr;

	DOL_INFO("%s: dhdol<%px> hlpr<0x%px>\n", __FUNCTION__, dhdol, hlpr);

	/* Un configure CPU Queues */
	dol_hlpr_unconfig_cpuq(hlpr, NULL);

	/* kill cpuq processing tasklet handle */
	tasklet_kill(&dhdol->dol_cpuq_tasklet);

	/* Free up CPU Queues */
	dol_hlpr_uninit_cpuq(hlpr);

	/* Free up radio object in the Helper */
	dol_hlpr_uninit_radio(hlpr);

	return;
}

/*
 * +----------------------------------------------------------------------------
 *           Section: Offload Initialization (dol_init)
 * +----------------------------------------------------------------------------
 */

/**
 * initialize dongle wakeup register information based on iDMA
 *
 */
static int
dol_init_dngl_wakeup(dhd_offload_t *dhdol)
{
	dhd_bus_t *bus = dhdol->dhd->bus;
	hlpr_init_cfg_t *init_cfg = &dhdol->hlpr.init_cfg;
	phys_addr_t bar0_phys = pci_resource_start(dhdol->pci_dev, 0);
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
	if (IDMA_ENAB(dhdol->dhd)) {
	    if (dhdol->sup_feat.hwawkup == 0) {
	        DOL_ERROR("dol%d: %s iDMA not supported\n", DOL_UNIT, DOL_HLPR);
	        return BCME_UNSUPPORTED;
	    }
	    /* Use PCIH2D_MailBox_2 (0x160) to wakeup dongle ARM (TXPOST) */
	    db_reg1 = (char*)bus->pcie_mb_intr_2_addr;

	    /* Use PCIH2D_DB1_2 (0x164) to wakeup dongle HWA (RXP, RXC, TXS) */
	    db_reg2 = (char*)bus->pcie_db1_intr_2_addr;

	    dhdol->en_feat.hwawkup = 1;
	} else
#endif /* BCMHWA */
	{
	    /* Use PCIMailBoxInt (0x140) for all rings to wakeup dongle SW */
	    db_reg1 = (char*)bus->pcie_mb_intr_addr;
	    db_reg2 = db_reg1;
	}

	/* Fill the wakeup information in init_cfg structure */
	/* Wakeup through DHD */
	init_cfg->doorbell_isr = dol_hlpr_isr_wake_dngl;
	init_cfg->doorbell_ctx = dhdol;

	/* Direct wakeup by Helper using door bell registers */
	init_cfg->dongle_wakeup_register =
	    (uint32)(bar0_phys + (uint32)(db_reg1 - regs_base));
	DOLPR("dongle_wakeup_addr = 0x%08x\n",
	    dhdol->hlpr.init_cfg.dongle_wakeup_register);
#if defined(DOL_HLPR_CAP_HWA_WAKEUP)
	init_cfg->dongle_wakeup_register_2 =
	    (uint32)(bar0_phys + (uint32)(db_reg2 - regs_base));
	init_cfg->dongle_wakeup_register_virt = (void*)db_reg1;
	init_cfg->dongle_wakeup_register_2_virt = (void*)db_reg2;
#if defined(BCMHWA)
	init_cfg->dongle_wakeup_hwa = IDMA_ENAB(dhdol->dhd);

#if defined(DOL_HLPR_CAP_IDMAGRPCFG)
	init_cfg->idma_group_shift = 0;
	if (init_cfg->dongle_wakeup_hwa) {
	    uint16_t igs = 0;
	    uint16 flows = IDMA_FLOWS_PER_SET(dhdol->dhd);

	    while (flows > 1) { igs++; flows >>= 1; }

	    init_cfg->idma_group_shift = igs;
	}
#endif /* DOL_HLPR_CAP_IDMAGRPCFG */

#if defined(DOL_HLPR_CAP_IDMA64)
	init_cfg->idma_num_of_groups = 0;
#if defined(IDMA_SETS)
	if (dhdol->en_feat.idma64 == 1) {
	    init_cfg->idma_num_of_groups = IDMA_SETS(dhdol->dhd);
	}
#endif /* IDMA_SETS */
#endif /* DOL_HLPR_CAP_IDMA64 */
#else /* !BCMHWA */
	init_cfg->dongle_wakeup_hwa = 0;
#endif /* !BCMHWA */
	DOLPR("dongle_wakeup_addr_2 = 0x%08x\n",
	    init_cfg->dongle_wakeup_register_2);
#endif /* DOL_HLPR_CAP_HWA_WAKEUP */

	return BCME_OK;
}

/**
 * enable adding llcsnap header feature in the Helper based on dhd and
 * dongle support.
 */
static int
dol_init_llcsnaphdr(dhd_offload_t *dhdol)
{
#if defined(BCM_DHDHDR)
	uint8 llcsnaphdr = DHDHDR_SUPPORT(dhdol->dhd);

	/* Check if Offload Helper has support for llcsnap header */
	if (llcsnaphdr && dhdol->sup_feat.dhdhdr == 0) {
	    DOL_ERROR("%s: Add LLCSNAP_HDR is not supported by %s\n",
	        __FUNCTION__, DOL_HLPR);
	    return BCME_UNSUPPORTED;
	}

#if defined(DOL_HLPR_CAP_LLCSNAPHDR)
	dhdol->hlpr.init_cfg.add_llcsnap_header = llcsnaphdr;
	dhdol->en_feat.dhdhdr = llcsnaphdr;
#endif /* DOL_HLPR_CAP_LLCSNAPHDR */
#endif /* BCM_DHDHDR */

	return BCME_OK;
}

/**
 * enable txcmpl2host feature in the Offload Helper.
 */
static int
dol_init_txc2host(dhd_offload_t *dhdol, hlpr_obj_handle mo)
{
	int rc = BCME_OK;

	if (DOL_TXO_ENABLED(dhdol) && !(DOL_TXO_TXSTS_OFFLOAD(dhdol))) {

	    /* fail if Helper does not support this feature */
	    if (dhdol->sup_feat.txcmpl2host == 0) {
	        return BCME_UNSUPPORTED;
	    }

#if defined(DOL_HLPR_CAP_TXCMPL2HOST)
	    rc = dol_hlpr_set_tx_complete_send2host(&dhdol->hlpr, mo, TRUE);

	    if (rc < 0) {
	        return BCME_ERROR;
	    }
	    dhdol->en_feat.txcmpl2host = 1;
#endif /* DOL_HLPR_CAP_TXCMPL2HOST */
	}

	return rc;
}

/**
 * Initialize ba256 configuration parameters
 *
 * Update RxP/RxC/TxP-BE-BQ sizes for Dongle BA256 mode if
 *  Supported by Offload Helper,
 *  enabled in dongle
 *  corresponding ring sizes in nvram are not updated by user
 *  override nvram setting is enabled if present
 *
 */
static int
dol_init_ba256cfg(dhd_offload_t *dhdol)
{
	dol_frm_t *frm = &dhdol->frm;
	int unit = DOL_UNIT;
	int rc = BCME_OK;

	/*
	 * For non-Offload, DHD always supports BA256 configuration. For Offload
	 * check if all dependant features are supported by offload hardware,
	 * if offload is enabled
	 */
	if ((DOL_TXO_ENABLED(dhdol) || DOL_RXO_ENABLED(dhdol)) &&
	    ((dhdol->sup_feat.ba256cfg == 0) ||
	    (dhdol->sup_feat.cmnrngsz == 0) ||
	    (dhdol->sup_feat.bkupq == 0))) {
	    rc = BCME_UNSUPPORTED;
	    DOLOG("dol%d: BA256 can not support\n", unit);
	    goto done;
	}

	/*
	 * BA256 can be supported by helper, Update the ring configuration,
	 * if still default and is not set by user using NVRAM
	 */

	/* RX Complete ring size */
	if ((dhdol->cfg.rxc_ring.cfgsts & DOL_CFGSTS_ITMCHG) == 0) {
	    dhdol->cfg.rxc_ring.max_items = DOL_RXC_RINGSZ_BA256;
	}

	/* TxPost BE Backup queue size */
	if (frm->def_profile_id == DOL_FRM_PROFILE_DEFAULT) {
	    frm->def_profile_id = DOL_FRM_PROFILE_DEFAULT_BA256;
	}

	rc = BCME_OK;
	DOLOG("dol%d: BA256 updated Profile [%d], max_items RxP/RxC [%d]/[%d]\n",
	    unit, dhdol->frm.def_profile_id,
	    dhdol->cfg.rxp_ring.max_items, dhdol->cfg.rxc_ring.max_items);

done:
	return rc;
}

/**
 * Initialize dhd_offload sw features
 *
 * check nvram, if not present, return use default value
 *
 *
*/
static int
dol_init_swfeatures(struct dhd_offload_obj *dhdol)
{
	/* Set the default values */
	dhdol->sw_feat.cpuqdpc = DOL_SWFEAT_CPUQDPC_DEF;
	dhdol->sw_feat.cmplnotif = DOL_SWFEAT_CMPLNOTIF_DEF;
	dhdol->ipend = FALSE;

	/*
	 * Fetch the SW feature Configuration information of the radio,
	 * if present
	 */

	/* Process Rx,Tx complete Queue in dhd DPC context */
	DOL_SWFEAT_ENABLE_INIT(dhdol, cpuqdpc);

	/* Notify Rx,Tx complete to Helper */
	DOL_SWFEAT_ENABLE_INIT(dhdol, cmplnotif);

	return 0;
}

/**
 * Initialize Runner Codel setting
*/
static int
dol_init_codel(struct dhd_offload_obj *dhdol)
{
#if defined(DOL_HLPR_CAP_CODEL)
	bool enable = false;
	int codel = -1;
	int rc = BCME_OK;

	/* Fetch the Codel Configuration information of the radio, if present */
	if (dol_nv_get_uint(DOL_UNIT, DOL_NV_VAR_CODEL, &codel) == BCME_NOTFOUND) {
	    /* Codel nvram configuration is not present */
	    if (dhdol->hlpr.init_cfg.hbqd_mode == 1) {
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
	}

	if (codel >= 0) {
	    enable = (codel) ? true : false;
	    rc = dol_hlpr_set_codel_enable(&dhdol->hlpr, enable);
	    if (rc == 0) {
	        DOLOG("dol%d: codel turned %s", DOL_UNIT, (codel) ? "ON" : "OFF");
	    }
	}

	rc = dol_hlpr_get_codel_enable(&dhdol->hlpr, &enable);
	if (rc == DOL_HLPR_OK) {
	    dhdol->en_feat.codel = enable;
	}

	return rc;
#else /* !DOL_HLPR_CAP_CODEL */
	return BCME_OK;
#endif /* !DOL_HLPR_CAP_CODEL */
}

/**
 * Initialize Offload Helper Wakeup information for Dongle
*/
static int
dol_init_hlpr_wakeup(dhd_offload_t *dhdol,
	bcmpcie_soft_doorbell_t *soft_doorbell, phys_addr_t wakeup_paddr,
	uint32 wakeup_val32)
{
	struct pci_dev *pdev;

	if (soft_doorbell == NULL) {
	    /*
		 * If softdoorbells are not supported, Offload Helper can not get
		 * interrupts
		 */
	    return BCME_BADARG;
	}

	pdev = osl_pci_device(dhdol->dhd->osh);
	if (pdev != NULL) {
	    wakeup_paddr = bcm_pcie_map_bar_addr(pdev, wakeup_paddr, SZ_1M);
	}

	if (wakeup_paddr == BCM_PCIE_MAP_ADDR_INVALID) {
	    return BCME_BADARG;
	}

	/* Use 64MB incoming window for PCI */
	dhdol->pci_bar = wakeup_paddr & ~(SZ_64M - 1);

#if defined(PHYS_ADDR_64BIT) || defined(CONFIG_PHYS_ADDR_T_64BIT)
	soft_doorbell->haddr.low_addr = (uint32_t)(wakeup_paddr & 0xffffffff);
	soft_doorbell->haddr.high_addr = (uint32_t)(wakeup_paddr >> 32);
#else /* !PHYS_ADDR_64BIT && !CONFIG_PHYS_ADDR_T_64BIT */
	soft_doorbell->haddr.low_addr = (uint32_t)wakeup_paddr;
	soft_doorbell->haddr.high_addr = 0U; /* 32bit host */
#endif /* !PHYS_ADDR_64BIT && !CONFIG_PHYS_ADDR_T_64BIT */
	soft_doorbell->value = htol32(wakeup_val32);

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
 * 64-MB region suffices to cover the Helper's register space. If two
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
 * Here is a walk through of dongle accessing Helper register 0x13099004:
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
 *              -> 0x13099004 and voila original Helper register phys-address
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
dol_init_pcie(dhd_offload_t *dhdol, struct pci_dev *pci_dev)
{
	int rc;

	dhdol->pci_dev = pci_dev;

	DOLOG(CLRyk "%s DHD PCIE: vendor<0x%04x> device<0x%04x> bus<%d> slot<%d>"
	    CLRnl, DOL_HLPR, pci_dev->vendor, pci_dev->device,
	    pci_dev->bus->number, PCI_SLOT(pci_dev->devfn));

	if (bcm_pcie_config_bar_addr(pci_dev, 0, dhdol->pci_bar, SZ_64M) == 0) {
	    rc = BCME_OK;
	} else {
	    DOL_ERROR("dol%d: failed to set PCIe Offload incoming address\n",
	        DOL_UNIT);
	    rc = BCME_ERROR;
	}

	return rc;
}

/**
 * Un-configure the PCIe BAR for Offload Helper wakeup incoming address
 */
static void
dol_deinit_pcie(dhd_offload_t *dhdol)
{
	if (bcm_pcie_config_bar_addr(dhdol->pci_dev, 0, dhdol->pci_bar, 0)) {
	    DOL_ERROR("dol%d: failed to unconfig PCIe Offload incoming address\n",
	        DOL_UNIT);
	}

	return;
}

/**
 * Initialize Rx Offload setting from PSP
 *
 * if not present, return the default value
 *
*/
static int
dol_init_rxoffl(struct dhd_offload_obj *dhdol,
	dol_ring_cfg_t *rxc_ring_cfg, dol_ring_cfg_t *rxp_ring_cfg)
{
	char buff[DOL_NV_RXOFFL_STR_LEN];
	int  length = 0;
	int offload, rxc_size = -1, rxp_size = -1;

	/* Set the default values */
	rxc_ring_cfg->offload = DOL_RXO_DEFAULT;
	rxc_ring_cfg->max_items = D2HRING_RXCMPLT_MAX_ITEM;

	if (rxp_ring_cfg) {
	    rxp_ring_cfg->offload = DOL_RXO_DEFAULT;
	    rxp_ring_cfg->max_items = H2DRING_RXPOST_MAX_ITEM;
	}

	/* Fetch the Rx Ring Configuration information of the radio, if present */
	memset(buff, 0, sizeof(buff));
	length = dol_nv_get(DOL_UNIT, DOL_NV_VAR_RXOFFL, buff, sizeof(buff));

	if (length == 0) {
	    /* Rx Offload setting is not present, return default */
	    goto done;
	}

	sscanf(buff, "%d:%d:%d", &offload, &rxc_size, &rxp_size);

	if (offload <= 0)
	    rxc_ring_cfg->offload = 0;
	else
	    rxc_ring_cfg->offload = 1;

	if ((rxc_size > 0) && (rxc_size <= DOL_FR_MAX_SIZE)) {
	    rxc_ring_cfg->max_items = next_larger_power2(rxc_size);
	    rxc_ring_cfg->cfgsts |= DOL_CFGSTS_ITMCHG;
	}

	if ((rxp_ring_cfg) && (rxp_size > 0) &&
	    (rxp_size <= DOL_FR_MAX_SIZE)) {
	    rxp_ring_cfg->max_items = next_larger_power2(rxp_size);
	    rxp_ring_cfg->cfgsts |= DOL_CFGSTS_ITMCHG;
	}

done:
	if (!DOL_RXO_SUPPORTED(dhdol)) {
	    DOL_INFO("dol%d: force disable rx offlaod due to no %s support\n",
	        DOL_UNIT, DOL_HLPR);
	    rxc_ring_cfg->offload = 0;
	}

	return 0;
}

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
dol_init_txoffl(struct dhd_offload_obj *dhdol, dol_ring_cfg_t *ring_cfg)
{
	char buff[DOL_NV_TXOFFL_STR_LEN];
	int  length = 0;
	int offload = 1, size = 0;
	char dhd_name[8];
	dhd_pub_t *dhd = dhdol->dhd;
	dol_frm_t *frm = &dhdol->frm;

	/*
	 * Check if we have any reserved memory allocated for this radio
	 * If no reserved memory is allocated, we can route all TXConf messages
	 * to DHD only instead of Offload Helper
	 */
	snprintf(dhd_name, sizeof(dhd_name), "dhd%d", dhd->unit);
	dhd_name[4] = '\0';

	/* Initialize txsts ring offload if reserved memory is available */
	if (BcmMemReserveGetByName(dhd_name,
	    (void **)&frm->rsv_mem_addr_base,
	    &frm->rsv_mem_paddr_base,
	    &frm->rsv_mem_size) == 0) {
	    frm->rsv_mem_addr = frm->rsv_mem_addr_base;
	    ring_cfg->offload = DOL_TXS_CFG_OFFL_MASK;
	} else {
	    DOL_INFO("BcmMemReserveGetByName returned no memory\n");
	    goto disable;
	}

	/* Fetch the Tx Offload setting from nvram, if present */
	memset(buff, 0, sizeof(buff));
	length = dol_nv_get(dhd->unit, DOL_NV_VAR_TXOFFL, buff, sizeof(buff));

	if (length != 0) {
	    /* Tx Offload setting is present, get the current setting */
	    sscanf(buff, "%d:%d", &offload, &size);

	    if ((size > 0) && (size <= DOL_FR_MAX_SIZE)) {
	        ring_cfg->max_items = next_larger_power2(size);
	        ring_cfg->cfgsts |= DOL_CFGSTS_ITMCHG;
	    }
	}

	/* If offload is enabled, nv setting can be used to force disable offload */
	if ((DOL_TXO_SUPPORTED(dhdol)) && (offload > DOL_TXO_MODE_M)) {
	    /*
	     * Offload    0   - Disable offload, N+M
	     *            1   - Enable offload, default N+M
	     *            2   - Enable offload, disable N+M
	     *            3   - Enable offload, enable N+M
	     *            *   - Same as 1
	     */
	    if (offload == DOL_TXO_MODE_NPM) {
	        dhdol->en_feat.nplusm = 1;
	    } else if (offload == DOL_TXO_MODE_N_ONLY) {
	        dhdol->en_feat.nplusm = 0;
	    } else {
	        /* DOL_TXO_MODE_N_DEF */
	        dhdol->en_feat.nplusm = DOL_NPM_DEFAULT;
	    }
	    goto done;
	}

disable:
	DOL_INFO("dol%d: disable tx offlaod \n", DOL_UNIT);
	ring_cfg->offload = 0;
	frm->rsv_mem_addr = (void*)NULL;
	frm->rsv_mem_size = 0;
	dhdol->en_feat.txoffl = 0;
	dhdol->en_feat.nplusm = 0;

done:
	frm->def_profile_id = DOL_FRM_PROFILE_DEFAULT;
	return 0;
}

/**
 * Disable Offload (tx/rx) due to missing/unsupported features
 *
*/
static int
dol_disable_offload(struct dhd_offload_obj *dhdol, bool tx_dor, bool rx_dor)
{
	if ((tx_dor) && (DOL_TXO_ENABLED(dhdol))) {

	    DOLOG(CLRerr "Force disabling %s Offload for TX" CLRnl, DOL_HLPR);

	    dhdol->en_feat.txoffl = 0;
	    dhdol->en_feat.nplusm = 0;
	    dhdol->cfg.txs_ring.offload = 0;
	    dhdol->cfg.txp_ring.offload = 0;
	    dhdol->frm.rsv_mem_addr = (void*)NULL;
	    dhdol->frm.rsv_mem_size = 0;
	    dhdol->hlpr.init_cfg.tx_post_flow_ring_base_addr = (void *)NULL;
	    dhdol->hlpr.init_cfg.tx_complete_flow_ring_base_addr = (void*)NULL;
	}

	if ((rx_dor) && (DOL_RXO_ENABLED(dhdol))) {

	    DOLOG(CLRerr "Force disabling %s Offload for RX" CLRnl, DOL_HLPR);

	    dhdol->en_feat.rxoffl = 0;
	    dhdol->cfg.rxc_ring.offload = 0;
	    dhdol->cfg.rxp_ring.offload = 0;
	}

	return BCME_OK;
}

/**
 * +----------------------------------------------------------------------------
 *           Section: Offload Operations (Notification/Requests) (dol_ops)
 * +----------------------------------------------------------------------------
 */
/*
 *
 * Initialize the Offload with the base address of various DHD DMA buffers.
 * In the case of DMA RD/WR Indices arrays, allocate/free in noncacheable memory
 */
static int
dol_ops_dma_buf_notif(dhd_offload_t *dhdol, void *osh,
	dhd_offload_dma_buf_t buf_type, dhd_dma_buf_t *dma_buf)
{
	hlpr_init_cfg_t *init_cfg = &dhdol->hlpr.init_cfg;
	dhd_pub_t* dhdp = dhdol->dhd;

	ASSERT(dma_buf != NULL);

	/*
	 * Allocate and attach a DMA buffer for RD and WR indices in uncached memory
	 * Use ARM/MIPS platform specific uncached memory allocation API
	 */
	if (buf_type & ATTACH_DMA_INDX_BUF) {

	    /* Round up to cache line size */
	    dma_buf->len = ALIGN_SIZE(dma_buf->len, L1_CACHE_BYTES);

	    dma_buf->va = dol_mem_alloc_dmabuf(dhdol, dma_buf, DOL_MEM_TYPE_HLPR);

	    if (!dma_buf->va) {
	        DOL_ERROR("%s: ATTACH_DMA_INDX_BUF<0x%08x> alloc failure\n",
	            __FUNCTION__, buf_type);
	        return BCME_NOMEM;
	    }

	    DOL_INFO("%s: ATTACH_DMA_INDX_BUF<0x%08x> dma_buf<0x%px,0x%x_%x,%d>\n",
	        __FUNCTION__, buf_type, dma_buf->va,
	        (uint32)PHYSADDRHI(dma_buf->pa), (uint32)PHYSADDRLO(dma_buf->pa),
	        dma_buf->len);

	    switch (buf_type) {
	        case ATTACH_O2D_WR_BUF:
	            init_cfg->r2d_wr_arr_base_addr = dma_buf->va;
	            DOL_HLPR_INIT_CFG_SET_PA(init_cfg->r2d_wr_arr_base_phys_addr,
	                dma_buf->pa, 0);
	            break;
	        case ATTACH_O2D_RD_BUF:
	            init_cfg->r2d_rd_arr_base_addr = dma_buf->va;
	            DOL_HLPR_INIT_CFG_SET_PA(init_cfg->r2d_rd_arr_base_phys_addr,
	                dma_buf->pa, 0);
	            break;
	        case ATTACH_D2O_WR_BUF:
	            {
	                ulong addr = (ulong)dma_buf->va + DOL_DMA_IDX_SZ(dhdp);

	                init_cfg->d2r_wr_arr_base_addr = (void*)addr;
	                DOL_HLPR_INIT_CFG_SET_PA(
	                    init_cfg->d2r_wr_arr_base_phys_addr,
	                    dma_buf->pa, dhdp->bus->rw_index_sz);
	            }
	            break;
	        case ATTACH_D2O_RD_BUF:
	            {
	                ulong addr = (ulong)dma_buf->va + DOL_DMA_IDX_SZ(dhdp);

	                init_cfg->d2r_rd_arr_base_addr = (void*)addr;
	                DOL_HLPR_INIT_CFG_SET_PA(
	                    init_cfg->d2r_rd_arr_base_phys_addr,
	                    dma_buf->pa, dhdp->bus->rw_index_sz);
	            }
	            break;
	        default:
	            DOL_ERROR("%s: invalid DMA_INDX_BUF type<%d>\n",
	                __FUNCTION__, buf_type);
	            ASSERT(0);
	            return BCME_BADOPTION;
	    }

	    DOL_INFO("%s: ATTACH_DMA_INDX_BUF<0x%08x> dma_buf<0x%px,0x%x_%x,%d>\n",
	        __FUNCTION__, buf_type, dma_buf->va,
	        (uint32)PHYSADDRHI(dma_buf->pa), (uint32)PHYSADDRLO(dma_buf->pa),
	        dma_buf->len);

	} else if (buf_type & DETACH_DMA_INDX_BUF) {

	    DOL_INFO("%s: DETACH_DMA_INDX_BUF<0x%08x> dma_buf<0x%px,0x%x_%x,%d>\n",
	        __FUNCTION__, buf_type, dma_buf->va, (uint32)PHYSADDRHI(dma_buf->pa),
	        (uint32)PHYSADDRLO(dma_buf->pa), dma_buf->len);

	    dol_mem_free_dmabuf(dhdol, dma_buf, DOL_MEM_TYPE_HLPR);

	} else if (buf_type & ATTACH_RING_BUF) {
	    uint32_t ringid = buf_type & 0xFFFF;
	    uint16_t def_max_items;
	    dol_ring_cfg_t *ring_cfg = NULL;

	    if (DHD_IS_TX_FLOWRING(dhdp, ringid)) {
	        uint16 flow_id;
	        hlpr_fr_cache_t *hlpr_cache;
	        dol_frm_t *frm;
	        dol_fr_cache_t *dol_cache;

	        /* DHD_RINGID_TO_FLOWID */
	        flow_id = (ringid - BCMPCIE_COMMON_MSGRINGS);
	        flow_id += BCMPCIE_H2D_COMMON_MSGRINGS;

	        frm = &dhdol->frm;
	        hlpr_cache = frm->hlpr_fr_cache + flow_id;
	        dol_cache = frm->dhd_flring_cache + flow_id;

	        dma_buf->va = dol_cache->base_va;
	        PHYSADDRLOSET(dma_buf->pa, ntohl(hlpr_cache->base_addr_low));
	        PHYSADDRHISET(dma_buf->pa, ntohl(hlpr_cache->base_addr_high));

	        dma_buf->len = ntohs(DOL_PHY_RING_SIZE(hlpr_cache));
	        dma_buf->len *= dhdol->cfg.txp_ring.size;
	        dma_buf->dmah = (void*)NULL;
	        dma_buf->secdma = (void*)NULL;

	        return BCME_OK;
	    }

	    /* Update dma_buf length information, if configured differently */
	    if (ringid == BCMPCIE_H2D_MSGRING_RXPOST_SUBMIT) {
	        ring_cfg = &dhdol->cfg.rxp_ring;
	        def_max_items = H2DRING_RXPOST_MAX_ITEM;
	    } else if (ringid == BCMPCIE_D2H_MSGRING_TX_COMPLETE) {
	        ring_cfg = &dhdol->cfg.txs_ring;
	        def_max_items = D2HRING_TXCMPLT_MAX_ITEM;
	    } else if (ringid == BCMPCIE_D2H_MSGRING_RX_COMPLETE) {
	        ring_cfg = &dhdol->cfg.rxc_ring;
	        def_max_items = D2HRING_RXCMPLT_MAX_ITEM;
	    }

	    /*
	     * Inform DHD of any update in the max_items using dma length
	     */
	    if (ring_cfg) {
	        if (((ring_cfg->cfgsts & DOL_CFGSTS_FMTCHG) == 0) ||
	            (dhdol->sup_feat.cmnrngsz == 0)) {
	            /*
	             * Switch back to default max items as DHD or Helper does not
	             * support ring max items configuraion
	             */
	            if (ring_cfg->max_items != def_max_items) {
	                DOLOG("dol%d: ringid [%d] Using default max_items [%d]",
	                    DOL_UNIT, ringid, def_max_items);
	                ring_cfg->max_items = def_max_items;
	            }
	        } else if (dma_buf->len != ring_cfg->max_items * ring_cfg->size) {
	            /* max_items are different, update dma buffer length */
	            dma_buf->len = ring_cfg->max_items * ring_cfg->size;
	            dhdol->en_feat.cmnrngsz = 1;
	        }
	    }

	    /* Allocate DMA-able buffers for the ring, if not done so already */
	    if (dma_buf->va == NULL) {

	        dma_buf->va = dol_mem_alloc_dmabuf_ring(dhdol, dma_buf, ringid);

	        if (!dma_buf->va) {
	            DOL_ERROR("%s: ATTACH_RING_BUF ringid<%d> alloc failure\n",
	                __FUNCTION__, ringid);
	            return BCME_NOMEM;
	        }
	    }

	    DOL_INFO("%s: ATTACH_RING_BUF<0x%08x> dma_buf<0x%px,0x%x_%x,%d>\n",
	        __FUNCTION__, buf_type, dma_buf->va,
	        (uint32)PHYSADDRHI(dma_buf->pa), (uint32)PHYSADDRLO(dma_buf->pa),
	        dma_buf->len);

	    /* Setup the allocation variables  */
	    switch (ringid) {
	        case BCMPCIE_H2D_MSGRING_RXPOST_SUBMIT:
	            init_cfg->rx_post_flow_ring_base_addr = dma_buf->va;
#if defined(DOL_HLPR_CAP_CMNRNGSZ)
	            init_cfg->rx_post_ring_size = ring_cfg->max_items;
#endif /* DOL_HLPR_CAP_CMNRNGSZ */
	            break;
	        case BCMPCIE_D2H_MSGRING_TX_COMPLETE:
	            if (DOL_TXO_ENABLED(dhdol))
	                init_cfg->tx_complete_flow_ring_base_addr = dma_buf->va;
#if defined(DOL_HLPR_CAP_CMNRNGSZ)
	            init_cfg->tx_cmpl_ring_size = ring_cfg->max_items;
#endif /* DOL_HLPR_CAP_CMNRNGSZ */
	            break;
	        case BCMPCIE_D2H_MSGRING_RX_COMPLETE:
	            if (DOL_RXO_ENABLED(dhdol))
	                init_cfg->rx_complete_flow_ring_base_addr = dma_buf->va;
#if defined(DOL_HLPR_CAP_CMNRNGSZ)
	            init_cfg->rx_cmpl_ring_size = ring_cfg->max_items;
#endif /* DOL_HLPR_CAP_CMNRNGSZ */
	            break;

	        case BCMPCIE_H2D_MSGRING_CONTROL_SUBMIT:
	        case BCMPCIE_D2H_MSGRING_CONTROL_COMPLETE:
	            /* Helper does not need to registers control subn/cmplt rings */
	            return BCME_OK;

	        default:
		    if (DHD_IS_DYN_FLOWRING(dhdp, (uint16_t)ringid)) {
			return BCME_OK;
		    }
	            DOL_ERROR("%s: ringid %d not supported in %s\n",
	                __FUNCTION__, buf_type, DOL_HLPR);
	            ASSERT(0);
	            return BCME_BADOPTION;
	    }
	} else if (buf_type & DETACH_RING_BUF) {
	    uint32_t ringid = buf_type & 0xFFFF;

	    DOL_INFO("%s: DETACH_RING_BUF<0x%08x> dma_buf<0x%px,0x%x_%x,%d>\n",
	        __FUNCTION__, buf_type, dma_buf->va,
	        (uint32)PHYSADDRHI(dma_buf->pa), (uint32)PHYSADDRLO(dma_buf->pa),
	        dma_buf->len);

	    if (DHD_IS_NON_TX_FLOWRING(dhdp, ringid)) {

	        dol_mem_free_dmabuf_ring(dhdol, dma_buf, ringid);

	    } else {
	        /*
			 * Nothing needs to be done here as all the flowring buffers are
			 * freed as part of dhd_offload_flowmgr_fini() call later
	         */
	        DOLPR("DETACH_RING_BUF No Action");
	    }
	}

	return BCME_OK;
}

/**
 * DHD requests Offlaod to complete the configuration of the dhdol object
 * once all dhd DMA-able buffers for common rings, flowrings and H2D/D2H indices
 * is completed.
 */
static int
dol_ops_init_notif(dhd_offload_t *dhdol, struct pci_dev *pci_dev)
{
	int rc = BCME_OK;
	DOL_HLPR_ATTR_ALLOC(dol_hlpr_attr);

	/* no need to Initialize, if offload is not supported for this radio */
	if (!DOL_SUPPORTED(dhdol)) {
	    DOL_INFO("dol%d: offload not supported, skip initialization\n",
	        DOL_UNIT);
	    rc = BCME_OK;
	    goto exit;
	}

	if (!pci_dev) {
	    DOL_ERROR("%s: unknown pci_dev\n", __FUNCTION__);
	    rc = BCME_NODEVICE;
	    goto exit;
	}

	if (!dol_hlpr_attr) {
	    DOL_ERROR("%s: failed to allocate dol_hlpr_attr\n", __FUNCTION__);
	    rc = BCME_NOMEM;
	    goto exit;
	}

	if ((rc = dol_init_pcie(dhdol, pci_dev)) != BCME_OK)
	    goto exit;

	if ((rc = dol_init_llcsnaphdr(dhdol)) != BCME_OK)
	    goto exit;

	rc = dol_init_txc2host(dhdol, dol_hlpr_attr);
	if (rc == BCME_ERROR)
	    goto exit;

	if ((rc = dol_init_dngl_wakeup(dhdol)) != BCME_OK)
	    goto exit;

#if defined(DOL_HLPR_CAP_RXCMPLRCH) && defined(BCM_RXCPLE_RCH)
	dhdol->hlpr.init_cfg.rx_cmpl_rch_max_seg = dhdol->dhd->max_rch_sdu_cnt;
#endif /* DOL_HLPR_CAP_RXCMPLRCH && BCM_RXCPLE_RCH */


	DOL_INIT_PERIM_UNLOCK(dhdp);

	rc = dol_hlpr_init_radio(&dhdol->hlpr, dol_hlpr_attr);
	if (rc) {
	    DOL_INIT_PERIM_LOCK(dhdp);
	    rc = BCME_ERROR;
	    goto exit;
	}

	if ((rc = dol_init_codel(dhdol)) != BCME_OK) {
	    DOL_INIT_PERIM_LOCK(dhdp);
	    rc = BCME_ERROR;
	    goto exit;
	}

	rc = dol_hlpr_set_init_connect(&dhdol->hlpr, dol_hlpr_attr);
	if (rc) {
	    DOL_INIT_PERIM_LOCK(dhdp);
	    rc = BCME_ERROR;
	    goto exit;
	}

	rc = dol_hlpr_init_cpuq(&dhdol->hlpr);
	if (rc) {
	    DOL_INIT_PERIM_LOCK(dhdp);
	    rc = BCME_ERROR;
	    goto exit;
	} else {
	    hlpr_cpuq_cfg_t cfg = { 0 };

	    cfg.size = (DOL_RXO_ENABLED(dhdol)) ? DOL_HLPR_CPUQ_RXC_SIZE : 0;
	    cfg.isr_priv = (long)dhdol;
	    cfg.rx_isr = dol_hlpr_isr_cpuq;

	    rc = dol_hlpr_config_cpuq(&dhdol->hlpr, &cfg);
	    if (!rc) {
	        tasklet_init(&dhdol->dol_cpuq_tasklet, dol_dp_cpuq_tasklet,
	            (unsigned long)dhdol);
	        dol_hlpr_enable_cpuq_int(&dhdol->hlpr);
	    }
	}

	DOL_INIT_PERIM_LOCK(dhdp);

#if IS_ENABLED(CONFIG_BCM_SPDSVC)
	dhdol_spdsvc_transmit = bcmFun_get(BCM_FUN_ID_SPDSVC_TRANSMIT);
	BCM_ASSERT(dhdol_spdsvc_transmit != NULL);
#endif

exit:
	DOL_HLPR_ATTR_FREE(dol_hlpr_attr);

	DOLOG(CLRyk "%s DHD Offload initialization %s" CLRnl, DOL_HLPR,
	    (rc == 0) ? "Successful" : "Failed");

	return rc;
}


static int
dol_ops_fr_enab_notif(dhd_offload_t *dhdol, uint32_t ringid, int enable)
{
	hlpr_fr_cache_t *flring;
	int rc;

	if (!dhdol->fr_cache_base.va) {
	    DOL_ERROR("%s: hlpr_fr_cache is not initialized\n", __FUNCTION__);
	    return BCME_NOTREADY;
	}

	flring = dhdol->frm.hlpr_fr_cache;
	if (!flring) {
	    DOL_ERROR("%s: hlpr_fr_cache flowid<%d> not initialized\n",
	        __FUNCTION__, ringid);
	    return BCME_NOTREADY;
	}

	flring += ringid;
	if (!(ntohs(flring->flags) & DOL_FR_IN_HLPR_FLAG)) {
	    DOL_INFO("%s: SW ring %d, no need to update %s\n", __FUNCTION__,
	        ringid, DOL_HLPR);
	   return BCME_OK;
	}

	if (enable)
	    flring->flags &= htons(~DOL_FR_DISABLED_FLAG);
	else
	    flring->flags |= htons(DOL_FR_DISABLED_FLAG);

	ARCH_FLUSH_COHERENT((void *)flring, sizeof(hlpr_fr_cache_t));

	rc = dol_hlpr_set_flow_ring_enable(&dhdol->hlpr, ringid, enable);

	return rc;
}

static int
dol_ops_aggr_config_notif(dhd_offload_t *dhdol, unsigned long arg1,
	unsigned long arg2)
{
	dhd_offload_aggr_config_t* aggr_cfg = (dhd_offload_aggr_config_t*)arg1;

#if defined(DOL_HLPR_CAP_LBRAGGR)
	uint32_t aggr_size;
	uint32_t aggr_timer;
	uint32_t wme_ac = 0;
	int prio = 0;
	int rc = 0;
#endif /* DOL_HLPR_CAP_LBRAGGR */

	/* Check if Offload Helper supports LBR aggregation */
	if (dhdol->sup_feat.lbraggr == 0) {
	    DOL_ERROR("dol%d: LBR Aggregation is not supported by %s\n",
	        DOL_UNIT, DOL_HLPR);
	    return BCME_UNSUPPORTED;
	}

	/* Sanity Check input parameters */
	if ((aggr_cfg == NULL) || (dhdol->hlpr.radio_obj == NULL)) {
	    DOL_ERROR("dol%d: NULL Config or %s DHD helper obj\n",
	        DOL_UNIT, DOL_HLPR);
	    return BCME_ERROR;
	}

	DOL_TRACE("dol%d: aggr mask<0x%x>, len<%d pkts>, timeout<%d msec>\n",
	    DOL_UNIT, aggr_cfg->en_mask, aggr_cfg->len, aggr_cfg->timeout);

#if defined(DOL_HLPR_CAP_LBRAGGR)
	/*
	 * Convert DHD parameters to Offload Helper parameters
	 * DHD (aggr_en + aggr_len) to  Offload Helper (aggr_size)
	 * DHD (prio) to Offload Helper (wme_ac)
	 */
	while (prio < wme_ac_max) {
	    if ((1 << prio) & aggr_cfg->en_mask) {
	        aggr_size = aggr_cfg->len;
	    } else {
	        aggr_size = 1;
	    }
	    wme_ac = WME_PRIO2AC(prio);
	    rc = dol_hlpr_set_aggregation_size(&dhdol->hlpr, wme_ac, aggr_size);
	    prio++;
	}

	dhdol->en_feat.lbraggr = (aggr_cfg->en_mask) ? 1 : 0;

	/* Set the aggregation timeout in the Offload Helper */
	aggr_timer = aggr_cfg->timeout;
	rc = dol_hlpr_aggregation_timer_set(&dhdol->hlpr, aggr_timer);
#endif /* DOL_HLPR_CAP_LBRAGGR */

	return BCME_OK;
}

/*
 * DHD notifies Offload Helper to configure PCIe MSGRING format and size
 *
 * Offload Helper only supports one format for all ring types.
 * Currently TX_POST ring format setting is used for all ring types.
 *
 * Note: This API supports format setting for each ring type seperately
 * for future use
 */
static int
dol_ops_msgring_fmt_notif(dhd_offload_t *dhdol, unsigned long arg1,
	unsigned long arg2)
{
	int rc = BCME_OK;
	uint16 ring = (uint16)arg1;
	uint16 type = (uint16)(arg2 & 0xFFFF);
	uint16 size = (uint16)((arg2 >> 16) & 0xFFFF);
	bool tx_dor_en = TRUE;
	bool rx_dor_en = TRUE;

	switch (ring) {
	    case BCMPCIE_H2D_MSGRING_RXPOST_SUBMIT:
	        dhdol->cfg.rxp_ring.size = size;
	        dhdol->cfg.rxp_ring.type = type;
	        dhdol->cfg.rxp_ring.cfgsts |= DOL_CFGSTS_FMTCHG;
		dhdol->cfg.rxp_ring.initialized = TRUE;
	        if (!(dhdol->cfg.rxp_ring.sup_types & (1ul << type)))
	            rx_dor_en = FALSE;
	        break;

	    case BCMPCIE_D2H_MSGRING_TX_COMPLETE:
	        dhdol->cfg.txs_ring.size = size;
	        dhdol->cfg.txs_ring.type = type;
	        dhdol->cfg.txs_ring.cfgsts |= DOL_CFGSTS_FMTCHG;
		dhdol->cfg.txs_ring.initialized = TRUE;
	        if (!(dhdol->cfg.txs_ring.sup_types & (1ul << type)))
	            tx_dor_en = FALSE;

		if (type == MSGBUF_WI_EWI32 && dhdol->sup_feat.txcpl_ewi) {
			dhdol->en_feat.txcpl_ewi = 1;
		}
	        break;

	    case BCMPCIE_D2H_MSGRING_RX_COMPLETE:
	        dhdol->cfg.rxc_ring.size = size;
	        dhdol->cfg.rxc_ring.type = type;
	        dhdol->cfg.rxc_ring.cfgsts |= DOL_CFGSTS_FMTCHG;
		dhdol->cfg.rxc_ring.initialized = TRUE;
	        if (!(dhdol->cfg.rxc_ring.sup_types & (1ul << type)))
	            rx_dor_en = FALSE;
	        break;

	    case BCMPCIE_H2D_MSGRING_CONTROL_SUBMIT:
	    case BCMPCIE_D2H_MSGRING_CONTROL_COMPLETE:
	        /* Helper does not need to registers control subn/cmplt rings */
	        DOL_ERROR("dol%d: MSG Ring [%d] not supported by %s\n",
	            DOL_UNIT, ring, DOL_HLPR);
	        return BCME_BADARG;

	    default:
	        /* BCMPCIE_H2D_MSGRING_TXPOST_SUBMIT */
	        dhdol->cfg.txp_ring.size = size;
	        dhdol->cfg.txp_ring.type = type;
	        dhdol->cfg.txp_ring.cfgsts |= DOL_CFGSTS_FMTCHG;
		dhdol->cfg.txp_ring.initialized = TRUE;
	        if (!(dhdol->cfg.txp_ring.sup_types & (1ul << type)))
	            tx_dor_en = FALSE;
	        break;
	}

	/*
	 * Check Offload Helper support for msgring formats
	 * (for old release compatibility)
	 */
	if (!tx_dor_en) {
	    DOL_ERROR("dol%d: %s ring format type [%d] not supported by %s\n",
	        DOL_UNIT,
	        (ring == BCMPCIE_D2H_MSGRING_TX_COMPLETE) ? "TxC" : "TxP", type,
	        DOL_HLPR);

	    /*
	     * WAR for Offload Helper without CWI message formats support
	     *
	     * Disable Offload if new message formats are not supported by
	     * the Offload Helper
	     */
	}

	if (!rx_dor_en) {
	    DOL_ERROR("dol%d: %s ring format type [%d] not supported by %s\n",
	        DOL_UNIT,
	        (ring == BCMPCIE_H2D_MSGRING_RXPOST_SUBMIT) ? "RxP" : "RxC", type,
	        DOL_HLPR);
	}
	rc = dol_disable_offload(dhdol, !tx_dor_en, !rx_dor_en);

#if defined(DOL_HLPR_CAP_MSGRINGFRMT)
	if (dhdol->cfg.txp_ring.initialized && dhdol->cfg.txs_ring.initialized) {
		dhdol->hlpr.init_cfg.flow_ring_format =
			DOL_HLPR_FLOWRING_FMT(dhdol->cfg.txp_ring.type,
				dhdol->cfg.txs_ring.type);

		dhdol->en_feat.msgringformat = dhdol->hlpr.init_cfg.flow_ring_format;

		DOL_TRACE("dol%d: MSG Ring format set to %d", DOL_UNIT,
				dhdol->hlpr.init_cfg.flow_ring_format);
	}
#endif /* DOL_HLPR_CAP_MSGRINGFRMT */

	return BCME_OK;
}

/* DHD notifies Offload Helper to update backup queue size */
static int
dol_ops_fr_bqsize_notif(dhd_offload_t *dhdol, uint16_t ringid, uint32_t bqsize)
{
#if defined(DOL_HLPR_CAP_DYNBKUPQ)
	hlpr_fr_cache_t *flring;
	dhd_wme_ac_t wme_ac;
	uint16_t flags;
	bool notify = true;
	bool direct = false;
	uint32_t rnr_bqsize;


	if (bqsize & FLRING_BQSIZE_NOTIF_GROUP) {
	    /* More notify coming on the way, defer notifying Helper */
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
	    if (!dhdol->fr_cache_base.va) {
	        DOL_ERROR("%s: hlpr_fr_cache is not initialized\n",
	            __FUNCTION__);
	        return BCME_OK;
	    }

	    flring = dhdol->frm.hlpr_fr_cache;
	    if (!flring) {
	        DOL_ERROR("%s: hlpr_fr_cache flowid<%d> not initialized\n",
	            __FUNCTION__, ringid);
	        return BCME_OK;
	    }

	    flring += ringid;
	    flags = ntohs(flring->flags);

	    if (!(flags & DOL_FR_IN_HLPR_FLAG)) {
	        DOL_INFO("%s: SW ring %d, no need to update %s\n",
	            __FUNCTION__, ringid, DOL_HLPR);
	        return BCME_OK;
	    }

	    wme_ac = DOL_FR_WME_AC(flags);
	    rnr_bqsize = bqsize;

	    if (direct == false) {
	        /* Use the same ratio of backup queue size reduction for each AC
	         *
	         * items = [(items - phy_items) * (bqsize)]/max_bqsize + phy_items
	         *
	         * items = Helper backup queue size  + physical ring size (phy_items)
	         * bqsize = station backup queue size in DHD
	         * max_bqsize = maximum stations backup queue size in DHD
	         */
	        rnr_bqsize *= dhdol->frm.profile->bqsize[wme_ac];
	        rnr_bqsize /= DOL_TXP_ITEM_MAX;
	    }
	    rnr_bqsize += dhdol->frm.phy_items[wme_ac];
	    flring->items = htons(rnr_bqsize);

	    ARCH_FLUSH_COHERENT((void *)flring, sizeof(hlpr_fr_cache_t));
	}

	/* Inform Offload Helper of the backup queue update */
	if (notify == true) {
	    hlpr_msgring_data_t data;
	    int rc;

	    if (ringid < ID16_INVALID) {
	        data.flowring_idx = ringid;
	        data.idx_valid = 1;
	    } else {
	        data.flowring_idx = 0;
	        data.idx_valid = 0;
	    }
	    rc = dol_hlpr_set_flow_ring_update(&dhdol->hlpr, data.u32);

	}
#endif /* DOL_HLPR_CAP_DYNBKUPQ */

	return BCME_OK;
}

/*
 * DHD notifies Offload Helper of PCIE IPC Capabilities (DHD) for negotiation
 *
 * arg1 (in):     Host Capabilities mask (uint32)
 * arg2 (in/out): Pointer to DHD capabilities (in)
 *                           DoL capabilities (out)
 *
 * Parse for the dependant capabilities and enable corresponding
 * features if helper support them. Send back all the enabled
 * capabilities supported by DoL
 *
 * Note: Currently fast flow ring delete cap is supported, can be
 * exteneded to others in future.
 */
static int
dol_ops_pcie_ipc_cap_notif(dhd_offload_t *dhdol, unsigned long arg1,
	unsigned long arg2)
{
	int rc = BCME_OK;
	uint32 hcap = arg1 & (*(uint32*)arg2);
	uint32 rcap = 0;

	if (!arg1) {
	    return rc;
	}

	if (hcap) {
	    /* Fast Flow Ring Delete */
	    if (hcap & D2O_PCIE_IPC_CAP_FFRD) {
	        if (dhdol->sup_feat.ffrd) {
	            dhdol->en_feat.ffrd = 1;
	            rcap |= D2O_PCIE_IPC_CAP_FFRD;
	        }
	    }

	    /* Host Backup Queue Depth */
	    if (hcap & D2O_PCIE_IPC_CAP_HBQD) {
	        if (dhdol->sup_feat.hbqd) {
	            dhdol->en_feat.hbqd = 1;
	            dhdol->hlpr.init_cfg.hbqd_mode = 1;
	            rcap |= D2O_PCIE_IPC_CAP_HBQD;
	        } else {
	            DOL_ERROR("dol%d: HBQD not supported by %s\n",
	                DOL_UNIT, DOL_HLPR);
	            rc = dol_disable_offload(dhdol, true, true);
	            if (rc == BCME_OK) {
	                rcap |= D2O_PCIE_IPC_CAP_HBQD;
	            }
	        }
	    }

	    /* Rx Complete V2 (tid support) */
	    if (hcap & D2O_PCIE_IPC_CAP_RX_CMPL_V2) {
	        if (dhdol->sup_feat.rxcmplv2) {
	            dhdol->en_feat.rxcmplv2 = 1;
	            rcap |= D2O_PCIE_IPC_CAP_RX_CMPL_V2;
	        }
	    }

	    /* Rx Complete RCH (RxCompletion in Hedroom) */
	    if (hcap & D2O_PCIE_IPC_CAP_RX_CMPL_RCH) {
	        if (dhdol->sup_feat.rxcmplrch) {
	            dhdol->en_feat.rxcmplrch = 1;
	            rcap |= D2O_PCIE_IPC_CAP_RX_CMPL_RCH;
	        }
	    }

	    /* BA256 support capability */
	    if (hcap & D2O_PCIE_IPC_CAP_BA256CFG) {
	        if (dol_init_ba256cfg(dhdol) == BCME_OK) {
	            dhdol->en_feat.ba256cfg = 1;
	            rcap |= D2O_PCIE_IPC_CAP_BA256CFG;
	        }
	    }

#if defined(DOL_HLPR_CAP_IDMA64)
	    /* iDMA64 support capability */
	    if (hcap & D2O_PCIE_IPC_CAP_IDMA64) {
	        dhdol->en_feat.idma64 = 1;
	        rcap |= D2O_PCIE_IPC_CAP_IDMA64;
	    }
#endif /* DOL_HLPR_CAP_IDMA64 */

	    /* Dummy TxComplete WI handling Capability */
	    if ((hcap & D2O_PCIE_IPC_CAP_DUMMYTXC) &&
	        (dhdol->sup_feat.dummytxc)) {
	        dhdol->en_feat.dummytxc = 1;
	        rcap |= D2O_PCIE_IPC_CAP_DUMMYTXC;
	    }
	}

	/* return the dol capabilities back */
	*(uint32*)arg2 = rcap;

	return rc;
}

/**
 * wakeup dongle based on the doorbell register and value
 *
 */
static int
dol_ops_wake_dngl_req(dhd_offload_t *dhdol, uint32 db_reg, uint32 db_val)
{
	dhd_pub_t	*dhdp = dhdol->dhd;

#if defined(BCMHWA)
	if (IDMA_ENAB(dhdp)) {
	    if (db_reg == dhdol->hlpr.init_cfg.dongle_wakeup_register) {
	        /* Use PCIH2D_MailBox_2 to wakeup dongle ARM */
	        dhdpcie_bus_ringbell_2_fast(dhdp->bus, db_val, TRUE);
	    } else if (db_reg == dhdol->hlpr.init_cfg.dongle_wakeup_register_2) {
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
 * +----------------------------------------------------------------------------
 *           Section: Flow Ring Manager (dol_frm)
 * +----------------------------------------------------------------------------
 */
/**
 * Display Policy Information
 *
 * if present,
 * parse and data fill the global table: dhdol_frm_profiles[radio_idx]
 *
*/
static void
dol_frm_get_policy_str(dol_frm_policy_t *policy, char *buf, int len)
{
	int index, ac;

	/* parse and update the user profile, if present */
	switch (policy->id) {

	    case dol_frm_policy_id_global:
	        snprintf(buf, len, "%d (%s)", policy->all_hw,
	            (policy->all_hw) ? "HW" : "SW");
	        break;

	    case dol_frm_policy_id_intf:
	        snprintf(buf, len, "%d", policy->max_intf);
	        break;

	    case dol_frm_policy_id_clients:
	        snprintf(buf, len, "%d", policy->max_sta);
	        break;

	    case dol_frm_policy_id_aclist:
	        for (ac = wme_ac_bk; ac <= wme_ac_max; ac++) {
	            snprintf(buf, (len-(ac*9)), "%s:%s ", dhdol_wme_ac_str[ac],
	                (policy->aclist_hw[ac]) ? "HW" : "SW");
	            buf += 9;
	        }
	        break;

	    case dol_frm_policy_id_maclist:
	        for (index = 0; index < DOL_FR_POLICY_MACLIST_MAX; index++)
	        {
	            if (policy->mac_addr[index][6])
	            {
	                snprintf(buf, (len-(index*18)),
	                    "%02x:%02x:%02x:%02x:%02x:%02x ",
	                    policy->mac_addr[index][0], policy->mac_addr[index][1],
	                    policy->mac_addr[index][2], policy->mac_addr[index][3],
	                    policy->mac_addr[index][4], policy->mac_addr[index][5]);
	                buf += 18;
	            }
	            else
	                break;
	        }
	        break;

	    case dol_frm_policy_id_dllac:
	        snprintf(buf, len, "%d (%s)", policy->d11ac_hw,
	            (policy->d11ac_hw) ? "HW" : "SW");
	        break;

	    default:
	        DOL_ERROR("Unexpected flowring policy <%d>\n", policy->id);
	        break;
	}

	return;
}

/*
 * Select a HW or SW managed flowring. Currently STA-id is not available and if
 * required, it may be passed. Several select algorithms may be implemented and
 * one may be picked and placed into the flowmgr.
 */
uint16
dol_frm_select_fr(dol_frm_t *frm, int prio, bool bcmc, bool prefer_hw_ring,
	bool *is_hw_ring)
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
	id_map[!prefer_hw_ring] = frm->hw_id16_map[wme_ac];

	hw_ring[prefer_hw_ring] = is_sw_ring_c;
	id_map[prefer_hw_ring] = frm->sw_id16_map[wme_ac];


	for (select = 0; select < 2; select++) {
	    flow_id = id16_map_alloc(id_map[select]);
	    if (flow_id != ID16_INVALID) {
	        *is_hw_ring = hw_ring[select];
	        break;
	    }
	}

	DOLPR2("%s: (0x%px, %d, %s, %s, %s) = %d", __FUNCTION__, frm, prio,
	    (bcmc)?"true":"false", (prefer_hw_ring)?"true":"false",
	    (*is_hw_ring)?"true":"false", flow_id);


	return flow_id;
}

/*
 * returns whether hw flow ring is preferred or sw flow ring is preferred
 * based on the current policy set for the radio
*/
uint16
dol_frm_select_policy(dol_frm_t *frm, int ifidx, int prio, uint8 *mac,
	int staidx, bool d11ac, bool *is_hw_ring)
{
	dol_frm_policy_t* policy;
	bool is_hw, bcmc;
	dhd_wme_ac_t ac;
	int index;
	uint16 flow_id;

	is_hw = FALSE;
	policy = frm->policy;

	/* parse and update the user profile, if present */
	switch (policy->id) {

	    case dol_frm_policy_id_global:
	        is_hw = policy->all_hw;
	        break;

	    case dol_frm_policy_id_intf:
	        if (ifidx <= policy->max_sta) is_hw = TRUE;
	        break;

	    case dol_frm_policy_id_clients:
	        if (staidx <= policy->max_sta) is_hw = TRUE;
	        break;

	    case dol_frm_policy_id_aclist:
	        ac = WME_PRIO2AC(prio);
	        is_hw = policy->aclist_hw[ac];
	        break;

	    case dol_frm_policy_id_maclist:
	        for (index = 0; index < DOL_FR_POLICY_MACLIST_MAX; index++)
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

	    case dol_frm_policy_id_dllac:
	        /*     d11ac_hw        d11ac        is_hw
	         *         1               1          1
	         *         1               0          0
	         *         0               1          0
	         *         0               0          0
	         */
	        is_hw = policy->d11ac_hw & d11ac;
	        break;
	}

	DOL_TRACE("%s: flow ring preference %s\n", __FUNCTION__,
	    (is_hw) ? "HW" : "SW");
	DOLPR("%s: flow ring preference %s\n", __FUNCTION__,
	    (is_hw) ? "HW" : "SW");

	bcmc = ETHER_ISMULTI(mac);
	/*
	 * For STA dest and WDS dest we allocate entry based on prio only,
	 * not based on mac address
	 */
	if (DOL_IF_ROLE_STA_OR_WDS(frm->dhdol->dhd, ifidx)) {
	    bcmc = FALSE;
	}
	flow_id = dol_frm_select_fr(frm, prio, bcmc, is_hw,
	    is_hw_ring);

	/*
	 * If flow_id is invalid, dongle is supporting additional clients than
	 * advertised, re-use id's from other ac's to support them
	 */
	if (flow_id == ID16_INVALID) {
	    ac = wme_ac_bk;
	    while ((flow_id == ID16_INVALID) && (ac < wme_ac_max)) {
	        flow_id = dol_frm_select_fr(frm, WME_AC2PRIO(ac), FALSE,
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
dol_frm_alloc_fr_id(dol_frm_t *frm, dhd_wme_ac_t wme_ac,
	bool force_dhd)
{
	int ring_sz;
	uint16 flags, flow_id;
	void *id16_map_hndl;
	hlpr_fr_cache_t *hlpr_cache;
	dhd_offload_t *dhdol;

	flow_id = id16_map_alloc(frm->flow_ids_map);
	if (flow_id == FLOWID_INVALID) {
	    DOL_ERROR("%s: Invalid flowid", __FUNCTION__);
	    return BCME_ERROR;
	}

	hlpr_cache = frm->hlpr_fr_cache + flow_id;
	hlpr_cache->base_addr_high = 0U;
	flags = wme_ac << DOL_FR_WME_AC_SHIFT;

	dhdol = frm->dhdol;
	ring_sz = frm->phy_items[wme_ac] * dhdol->cfg.txp_ring.size;

	if ((frm->rsv_mem_size >= ring_sz) && (force_dhd == FALSE)) {
	    /* Offload Helper managed txpost ring */
	    flags |= DOL_FR_IN_HLPR_FLAG | DOL_FR_DISABLED_FLAG;
	    frm->rsv_mem_size -= ring_sz;
	    frm->hw_ring_cnt[wme_ac]++;
	    id16_map_hndl = frm->hw_id16_map[wme_ac];
	} else if (!(dhdol->en_feat.txoffl ^ dhdol->en_feat.nplusm)) {
	    frm->sw_ring_cnt[wme_ac]++;
	    id16_map_hndl = frm->sw_id16_map[wme_ac];
	} else {
	    DOL_ERROR("dol%d: Not enough Reserved memory\n", DOL_UNIT);
	    return BCME_ERROR;
	}

	hlpr_cache->flags = htons(flags);
	id16_map_free(id16_map_hndl, flow_id);

	DOLPR2("%s: allocated %s flowring [%d], base address [%x_%8x] \n",
	    __FUNCTION__, (flags & DOL_FR_IN_HLPR_FLAG) ? "HW" : "SW",
	    flow_id, hlpr_cache->base_addr_high, hlpr_cache->base_addr_low);

	return BCME_OK;
}

/**
 * Initialize the helper and dhd cahche information for the flowring
 *
 */
static int
dol_frm_init_fr(dol_frm_t *frm, uint16 flow_id)
{
	int ring_sz;
	uint16 flags;
	dhd_wme_ac_t wme_ac;
	hlpr_fr_cache_t *hlpr_cache;
	dol_fr_cache_t *dol_cache;
	dhd_offload_t *dhdol;

	dhdol = frm->dhdol;
	hlpr_cache = frm->hlpr_fr_cache + flow_id;
	dol_cache = frm->dhd_flring_cache + flow_id;

	flags = ntohs(hlpr_cache->flags);
	wme_ac = DOL_FR_WME_AC(flags);
	ring_sz = frm->phy_items[wme_ac] * dhdol->cfg.txp_ring.size;

	/* Allocation flowring buffer */
	if (flags & DOL_FR_IN_HLPR_FLAG) {
	    /*
	     * Offload Helper managed txpost ring, use reserved memory
	     * Reserved memory address is alwasys in lower 2GByte range
	     */
#if IS_ENABLED(CONFIG_BCM_DHD_ARCHER)
	    hlpr_cache->base_ptr = (void*)frm->rsv_mem_addr;
#if !IS_ENABLED(CONFIG_BCM_DHD_CROSSBOW)
	    hlpr_cache->wr_idx = 0;
	    hlpr_cache->rd_idx = 0;
#endif /* !CONFIG_BCM_DHD_CROSSBOW */
#endif /* CONFIG_BCM_DHD_ARCHER */

	    hlpr_cache->base_addr_high = 0U;
	    hlpr_cache->base_addr_low = (uint32)(RDD_RSV_VIRT_TO_PHYS(
	        frm->rsv_mem_addr_base, frm->rsv_mem_paddr_base,
	        frm->rsv_mem_addr));
	    dol_cache->base_va = (void*)frm->rsv_mem_addr;
	    frm->rsv_mem_addr += ring_sz;
	} else {
	    /* DHD managed txpost ring, allocate dma buffer */
	    dhd_dma_buf_t dma_buf;

	    dma_buf.len = ring_sz;
	    dma_buf.va = dol_mem_alloc_dmabuf(dhdol, &dma_buf, DOL_MEM_TYPE_DHD);

	    if (!dma_buf.va) {
	        DOL_ERROR("%s: dma_buf alloc for flow_id %d failure\n",
	            __FUNCTION__, flow_id);
	        return BCME_ERROR;
	    }

	    hlpr_cache->base_addr_low = PHYSADDRLO(dma_buf.pa);
	    hlpr_cache->base_addr_high = PHYSADDRHI(dma_buf.pa);
	    dol_cache->base_va = dma_buf.va;
	}

#if defined(DOL_HLPR_CAP_BKUPQUEUE)
	hlpr_cache->phy_ring_size = htons(frm->phy_items[wme_ac]);
#endif /* DOL_HLPR_CAP_BKUPQUEUE */

#if defined(DOL_HLPR_CAP_CODEL)
	hlpr_cache->window_ts = 0;
	hlpr_cache->drop_interval = 0;
	hlpr_cache->codel_flags = 0;
	hlpr_cache->codel_drop_counter = 0;
#endif /* DOL_HLPR_CAP_CODEL */

	/* items = physical ring size + backup queues if supported */
	hlpr_cache->items = htons(frm->profile->items[wme_ac]);
	hlpr_cache->base_addr_low = htonl(hlpr_cache->base_addr_low);
	hlpr_cache->base_addr_high = htonl(hlpr_cache->base_addr_high);
	hlpr_cache->flags = htons(flags);

	ARCH_FLUSH_COHERENT((void *)hlpr_cache, sizeof(hlpr_fr_cache_t));

	DOLPR2("%s: allocated %s flowring [%d], base address [%x_%8x] \n",
	    __FUNCTION__, (flags & DOL_FR_IN_HLPR_FLAG) ? "HW" : "SW",
	    flow_id, hlpr_cache->base_addr_high, hlpr_cache->base_addr_low);

	return BCME_OK;
}

/**
 * Initialize flowring id map based on the flowmgr allocation type
 *
 */
int
dol_frm_init_fr_map(dol_frm_t *frm)
{
	dhd_offload_t *dhdol = frm->dhdol;
	dhd_pub_t *dhdp;
	uint16 flowid;
	dhd_wme_ac_t ac;
	void *flow_ids_map;
	int max_h2d_rings;
	int rc;

	DOL_TRACE("%s\n", __FUNCTION__);

	dhdp = dhdol->dhd;
	max_h2d_rings = frm->max_h2d_rings;


	if (frm->alloc_mode != DOL_FR_ALLOC_WMEAC) {
	    hlpr_fr_cache_t *hlpr_cache;
	    uint16 flags;

	    /* Non access category based flowring map initialization */
	    if (frm->alloc_mode == DOL_FR_ALLOC_IDMA) {
	        flow_ids_map = dhd_idma_flowmgr_init(dhdp, max_h2d_rings);
	    } else {
	        flow_ids_map = id16_map_init(dhdp->osh,
	            max_h2d_rings - FLOW_RING_COMMON - MAX_DYN_FLOWRINGS(dhdp), FLOWID_RESERVED);
	    }

	    /* If failed to allocate flowid map, bail out */
	    if (flow_ids_map == NULL) {
	        DOL_ERROR("dol%d: failed to allocate flow-id map\n", dhdp->unit);
	        goto error_rtn;
	    }
	    frm->flow_ids_map = flow_ids_map;

	    /* Mark offloaded/non-offloaded in the offload flowring cache memory */
	    if (DOL_TXO_ENABLED(dhdol)) {
	        flags = DOL_FR_DISABLED_FLAG;
	        flags |= DOL_FR_IN_HLPR_FLAG;
	        for (flowid = FLOWID_RESERVED; flowid < (max_h2d_rings - MAX_DYN_FLOWRINGS(dhdp));
			flowid++)
	        {
	            hlpr_cache = frm->hlpr_fr_cache + flowid;
	            hlpr_cache->flags = htons(flags);
	        }
	    }
	} else {
	    /* Access Category based flowring map initialization */
	    uint16 total_ids;
	    int rings_to_allocate[wme_ac_max + 1];  /* reused for per tid, too */
	    int weighted_rings_to_allocate = 0;

	    /* Populate the flowids that will be used from 2..(max-dynamic rings) */
	    total_ids = max_h2d_rings - FLOW_RING_COMMON - MAX_DYN_FLOWRINGS(dhdp);
	    frm->flow_ids_map = id16_map_init(dhdp->osh, total_ids,
	        FLOWID_RESERVED);
	    if (frm->flow_ids_map == NULL) {
	        DOL_ERROR("dol%d: flow_ids_map init failure\n", dhdp->unit);
	        goto error_rtn;
	    }

#if defined(BCM_DBG_ID16)
#error "remove ASSERT val16 or use total_ids in id16_map_init below"
#endif

	    DOL_INFO("dol%d: allocated flow_ids_map %p, ids %d\n", dhdp->unit,
	        frm->flow_ids_map, total_ids);

	    /*
	     * id16 map allocators for ucast rings per access category
	     * for N stations
	     */
	    for (ac = wme_ac_bk; ac < wme_ac_max; ac++) {
	        rings_to_allocate[ac] = frm->max_uc_rings;
	        frm->hw_id16_map[ac] =
	            id16_map_init(dhdp->osh, frm->max_uc_rings, ID16_INVALID);
	        frm->sw_id16_map[ac] =
	            id16_map_init(dhdp->osh, frm->max_uc_rings, ID16_INVALID);
	        if (dhdol->en_feat.nplusm == 0) {
	            frm->profile->weight[ac] = DOL_PROFILE_PER_AC_BUDGET;
	        }
	    }

	    /* id16 map allocators for BCMC rings (one per BSS) */
	    rings_to_allocate[wme_ac_bcmc] = frm->max_bss;

	    frm->hw_id16_map[wme_ac_bcmc] =
	        id16_map_init(dhdp->osh, frm->max_bss, ID16_INVALID);
	    frm->sw_id16_map[wme_ac_bcmc] =
	        id16_map_init(dhdp->osh, frm->max_bss, ID16_INVALID);

	    /* Allocate rings with 0 weight (forced non-offload) in DHD memory */
	    for (ac = wme_ac_bk; ac <= wme_ac_max; ac++)
	    {
	        if (frm->profile->weight[ac] == 0) {
	            while (rings_to_allocate[ac]) {
	                rc = dol_frm_alloc_fr_id(frm, ac, TRUE);
	                if (rc == BCME_ERROR) {
	                    DOL_ERROR("Failed to allocate %d %s rings",
	                        rings_to_allocate[ac], dhdol_wme_ac_str[ac]);
	                            goto error_rtn;
	                }
	                rings_to_allocate[ac]--;
	            }
	        }
	    }

	    /* Carve out ucast rings with -1 weights from reserved memory */

	    /* First pass, allows all -1 weighted ACs to consume a max of
	     * DOL_PROFILE_PER_AC_BUDGET number of flowrings.
	     * This is done so that the per TID based ucast flowrings do not consume
	     * the entire Offload Helper memory.
	     */
	    for (ac = wme_ac_bk; ac < wme_ac_max; ac++)
	    {
	        int ac_budget = DOL_PROFILE_PER_AC_BUDGET;

	        if (frm->profile->weight[ac] == -1) {
	            while (rings_to_allocate[ac]) {
	                rc = dol_frm_alloc_fr_id(frm, ac, FALSE);
	                if (rc == BCME_ERROR) {
	                    DOL_ERROR("Failed to allocate %d %s ucast rings",
	                        rings_to_allocate[ac], dhdol_wme_ac_str[ac]);
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

	    /* 2nd pass: -1 weight AC leftover from DOL_PROFILE_PER_AC_BUDGET */
	    for (ac = wme_ac_bk; ac < wme_ac_max; ac++)
	    {
	        if (frm->profile->weight[ac] == -1) {
	            while (rings_to_allocate[ac]) {
	                rc = dol_frm_alloc_fr_id(frm, ac, FALSE);
	                if (rc == BCME_ERROR) {
	                    DOL_ERROR("Failed to allocate %d %s ucast rings",
	                        rings_to_allocate[ac], dhdol_wme_ac_str[ac]);
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
	            weight = frm->profile->weight[ac];
	            if ((weight == -1) || (weight == 0))
	                continue;
	            weight = LIMIT_TO_MAX(weight, rings_to_allocate[ac]);
	            rings_to_allocate[ac] -= weight;
	            weighted_rings_to_allocate -= weight;

	            while (weight) {
	                rc = dol_frm_alloc_fr_id(frm, ac, FALSE);
	                if (rc == BCME_ERROR) {
	                    DOL_ERROR("Failed to allocate %d %s ucast rings",
	                        rings_to_allocate[ac], dhdol_wme_ac_str[ac]);
	                    goto error_rtn;
	                }
	                weight--;
	            }
	        }
	    }

	    /* Carve out bcmc rings */
	    while (rings_to_allocate[wme_ac_bcmc]) {
	        rc = dol_frm_alloc_fr_id(frm, wme_ac_bcmc, FALSE);
	        if (rc == BCME_ERROR) {
	            DOL_ERROR("Failed to allocate %d bcmc rings",
	                rings_to_allocate[wme_ac_bcmc]);
	            goto error_rtn;
	        }
	        rings_to_allocate[wme_ac_bcmc]--;
	    }

	    /* By now the flow_ids_map should be empty */
	    frm->flow_ids_map = id16_map_fini(dhdp->osh, frm->flow_ids_map);
	}

	return BCME_OK;

error_rtn:
	return BCME_ERROR;
}

/**
 * Read the profile information from the persistent storage area
 *
 * Read nvram variable radio%d_profile id,
 * if present and valid, override parameter profile_id
 *
 * if present,
 * parse and data fill the global table: dhdol_frm_profiles[radio_idx]
 *
*/
static dol_frm_profile_t*
dol_frm_init_profile(struct dhd_offload_obj *dhdol)
{
	char buff[DOL_NV_PROFILE_STR_LEN];
	int  length = 0;
	int  profile_id;
	char *token, *profilestr;
	dhd_wme_ac_t ac;
	int radio_idx;
	dol_frm_profile_t* profile;
	dol_frm_t *frm = &dhdol->frm;

	/* Update the user profiles if present, only once */
	if (dhdol_cb.up_inited == false) {
	     int val1, val2;

	    /* Walk through all radios user profiles */
	    for (radio_idx = 0; radio_idx < DOL_RADIO_MAX; radio_idx++) {
	        length = dol_nv_get(radio_idx, DOL_NV_VAR_PROFILE, buff, sizeof(buff));

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
	            if (dhdol->sup_feat.nplusm) {
	                sscanf(token, "%d:%d", &val2, &val1);
	                dhdol_frm_profiles[radio_idx].weight[ac] = val2;
	                if (val1 >= dhdol_frm_txp_ringsz_defaults[ac]) {
	                    val2 = val1 - dhdol_frm_txp_ringsz_defaults[ac];
	                    dhdol_frm_profiles[radio_idx].items[ac] = val1;
	                    dhdol_frm_profiles[radio_idx].bqsize[ac] = val2;
	                } else {
	                    DOLOG("dol%d: discarding %d item size for %s\n",
	                        DOL_UNIT, val1, dhdol_wme_ac_str[ac]);
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
	                    dhdol_frm_profiles[radio_idx].items[ac] = val1 + val2;
	                    dhdol_frm_profiles[radio_idx].bqsize[ac] = val2;
	                } else {
	                    if (val1 >= dhdol_frm_txp_ringsz_defaults[ac]) {
	                        val2 = val1 - dhdol_frm_txp_ringsz_defaults[ac];
	                        dhdol_frm_profiles[radio_idx].items[ac] = val1;
	                        dhdol_frm_profiles[radio_idx].bqsize[ac] = val2;
	                    } else {
	                        DOLOG("dol%d: discarding %d item size for %s\n",
	                            DOL_UNIT, val1, dhdol_wme_ac_str[ac]);
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
	radio_idx = DOL_UNIT;
	length = dol_nv_get(radio_idx, DOL_NV_VAR_PROFILE, buff, sizeof(buff));

	if (length == 0) {
	    /* Profile not present, return default radio profile */
	    profile_id = frm->def_profile_id;
	    goto bkupq;
	}

	/* parse profile_id from profile string */
	profilestr = buff;
	token = bcmstrtok(&profilestr, " ", NULL);
	sscanf(buff, "%d", &profile_id);

	if (profile_id >= DOL_FRM_PROFILE_MAX) {
	    DOLOG("profile %d key is not valid, using built-in profile id %d",
	        profile_id, radio_idx);
	    profile_id = frm->def_profile_id;
	}

	if ((!dhdol->sup_feat.nplusm) && dhdol_frm_profiles[profile_id].nplusm) {
	    DOLOG("profile %d key is not supported, using built-in profile id %d",
	        profile_id, frm->def_profile_id);
	    profile_id = frm->def_profile_id;
	}

bkupq:
	profile = &dhdol_frm_profiles[profile_id];
	dhdol->en_feat.bkupq = DOL_BKUPQ_DEFAULT;

#if defined(DOL_HLPR_CAP_BKUPQUEUE)
	{
	    /*
		 * If nvram profile set dhd?_rnr_flowring_physize, then that overrides
		 * the defaults. Exit from this section with phy_ring_size set to 0
		 * means use the defaults
		 */
	    length = dol_nv_get(radio_idx, DOL_NV_VAR_PHY_RING_SIZE, buff,
	        sizeof(buff));
	    if (length != 0) {
	        DOLOG("dol%d: dol%d_flowring_physize setting deprecated"
	            "use dol%d_flowring_profile instead\n",
	            DOL_UNIT, DOL_UNIT, DOL_UNIT);
	    }
	}
#endif /* DOL_HLPR_CAP_BKUPQUEUE */

#if defined(DOL_HLPR_CAP_DYNBKUPQ)
	/* Enable back up queue update feature if backup queue feature is enabled */
	dhdol->en_feat.dynbkupq = dhdol->en_feat.bkupq;
#endif /* DOL_HLPR_CAP_DYNBKUPQ */

	for (ac = wme_ac_bk; ac <= wme_ac_max; ac++) {
	    if (DOL_BKUPQ(dhdol)) {
	        frm->phy_items[ac] = DOL_PROFILE_TXP_RING_SIZE(profile, ac);
	    } else {
	        frm->phy_items[ac] = profile->items[ac];
	    }
	}

	if (dhdol->sup_feat.nplusm) {
	    DOLOG("%s: N+M profile = %1d %02d:%04d/%04d %02d:%04d/%04d"
	        " %02d:%04d/%04d %02d:%04d/%04d %02d:%04d/%04d",
	        __FUNCTION__, profile_id,
	        profile->weight[0], frm->phy_items[0], profile->bqsize[0],
	        profile->weight[1], frm->phy_items[1], profile->bqsize[1],
	        profile->weight[2], frm->phy_items[2], profile->bqsize[2],
	        profile->weight[3], frm->phy_items[3], profile->bqsize[3],
	        profile->weight[4], frm->phy_items[4], profile->bqsize[4]);
	} else {
	    DOLOG("%s: profile = %1d %04d/%04d %04d/%04d"
	        " %04d/%04d %04d/%04d %04d/%04d",
	        __FUNCTION__, profile_id,
	        frm->phy_items[0], profile->bqsize[0],
	        frm->phy_items[1], profile->bqsize[1],
	        frm->phy_items[2], profile->bqsize[2],
	        frm->phy_items[3], profile->bqsize[3],
	        frm->phy_items[4], profile->bqsize[4]);
	}

	return &dhdol_frm_profiles[profile_id];
}

/**
 * Read the policy information from the persistent storage area
 *
 * Read nvram variable radio%d_policy,
 * if present and valid, override parameter policy_id
 *
 * if present,
 * parse and data fill the global table: dhdol_frm_profiles[radio_idx]
 *
*/
static dol_frm_policy_t*
dol_frm_init_policy(struct dhd_offload_obj *dhdol)
{
	char buff[DOL_NV_POLICY_STR_LEN];
	int  length = 0;
	int  policy_id;
	char *token, *policy_str;
	int  value;
	int mac_addr[ETHER_ADDR_LEN];
	int ac;
	dol_frm_policy_t* policy;

	policy = &dhdol_frm_policies[DOL_UNIT];

	if (!dhdol->sup_feat.nplusm) {
	    /* N+M disabled, policy has no effect */
	    return policy;
	}

	/* Fetch the Policy information of the radio, if present */
	memset(buff, 0, sizeof(buff));
	length = dol_nv_get(DOL_UNIT, DOL_NV_VAR_POLICY, buff, sizeof(buff));

	if (length == 0) {
	    /* Policy not present, return default radio policy */
	    goto done;
	}

	/* parse policy_id from profile string */
	policy_str = buff;
	token = bcmstrtok(&policy_str, " ", NULL);
	sscanf(buff, "%d", &policy_id);

	if (policy_id >= dol_frm_policy_id_max) {
	    DOLOG("policy id %d is not valid, using built-in policy id %d",
	        policy_id, dhdol_frm_policies[DOL_UNIT].id);

	    goto done;
	}

	policy->id = policy_id;

	/* parse and update the user profile, if present */
	switch (policy_id) {

	    case dol_frm_policy_id_global:
	        token = bcmstrtok(&policy_str, " ", NULL);
	        if (token) sscanf(token, "%d", &value);
	        else value = 1;
	        if (value) policy->all_hw = TRUE;
	        else policy->all_hw = FALSE;

	        break;

	    case dol_frm_policy_id_intf:
	        token = bcmstrtok(&policy_str, " ", NULL);
	        if (token) sscanf(token, "%d", &value);
	        else value = 0;
	        value = LIMIT_TO_MAX(value, 15);
	        policy->max_intf = value;

	        break;

	    case dol_frm_policy_id_clients:
	        token = bcmstrtok(&policy_str, " ", NULL);
	        if (token) sscanf(token, "%d", &value);
	        else value = DOL_STATION_MAX;
	        value = LIMIT_TO_MAX(value, DOL_STATION_MAX);
	        policy->max_sta = value;

	        break;

	    case dol_frm_policy_id_aclist:
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

	    case dol_frm_policy_id_maclist:
	        for (value = 0; value < DOL_FR_POLICY_MACLIST_MAX; value++)
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

	    case dol_frm_policy_id_dllac:
	        token = bcmstrtok(&policy_str, " ", NULL);
	        sscanf(token, "%d", &value);
	        if (value) policy->d11ac_hw = TRUE;
	        else policy->d11ac_hw = FALSE;

	        break;

	    default:
	        DOL_ERROR("Unexpected flowring policy <%d>\n", policy->id);
	        policy = NULL;
	        return policy;
	}

done:
	buff[0] = '\0';
	dol_frm_get_policy_str(policy, buff, sizeof(buff));
	DOLOG("%s:  N+M Policy = %d %s", __FUNCTION__, policy->id, buff);

	return policy;
}

/**
 * +----------------------------------------------------------------------------
 *           Section: Offload IOVAR processing (dol_iov)
 * +----------------------------------------------------------------------------
 */

/**
 * Fills the Iovar string with the list of available profiles supported by the
 * DHD Offload layer
 *
 * Also indicates the current profile of the radio
 */
static int
dol_iov_get_profile(dhd_offload_t *dhdol, char *buf, int buflen)
{
	int id;
	dhd_wme_ac_t ac;
	struct bcmstrbuf b;
	struct bcmstrbuf *strbuf = &b;
	dol_frm_profile_t* profile;
	dhd_pub_t *dhdp;

	DOL_TRACE("%s: dhdol=<0x%px>, buff=<0x%px>, bufflen=<%d>\n",
	    __FUNCTION__, dhdol, buf, buflen);

	dhdp = dhdol->dhd;

	bcm_binit(strbuf, buf, buflen);

	bcm_bprintf(strbuf, "[id] [   ac_bk   ] [   ac_be   ] [   ac_vi   ] "
	    "[   ac_vo   ] [   bc_mc   ]\n");
	if (dhdol->sup_feat.nplusm) {
	    bcm_bprintf(strbuf, "     pr:ring:queue pr:ring:queue pr:ring:queue "
	        "pr:ring:queue pr:ring:queue\n");
	} else {
	    bcm_bprintf(strbuf, "     [ ring:queue] [ ring:queue] [ ring:queue] "
	        "[ ring:queue] [ ring:queue]\n");
	}

	for (id = 0; id < DOL_FRM_PROFILE_MAX; id++) {
	    profile = &dhdol_frm_profiles[id];
	    if ((!dhdol->sup_feat.nplusm) && profile->nplusm) {
	        continue;
	    }

	    if (profile == dhdol->frm.profile)
	        bcm_bprintf(strbuf, " *%d ", profile->id);
	    else
	        bcm_bprintf(strbuf, "  %d ", profile->id);

	    for (ac = wme_ac_bk; ac <= wme_ac_max; ac++) {
	        if (dhdol->sup_feat.nplusm) {
	            bcm_bprintf(strbuf, "%02d:%04d:%04d ", profile->weight[ac],
	                DOL_PROFILE_TXP_RING_SIZE(profile, ac),
	                profile->bqsize[ac]);
	        } else {
	            bcm_bprintf(strbuf, "   %04d:%04d  ",
	                DOL_PROFILE_TXP_RING_SIZE(profile, ac),
	                profile->bqsize[ac]);
	        }
	    }

	    bcm_bprintf(strbuf, "\n");
	}

	return (!strbuf->size ? BCME_BUFTOOSHORT : 0);
}

/**
 * Fills the Iovar string with the list of available policies supported by the
 * DHD Offload layer
 *
 * Also indicates the current policy and policy parameters of the radio
 */
static int
dol_iov_get_policy(dhd_offload_t *dhdol, char *buf, int buflen)
{
	int id;
	struct bcmstrbuf b;
	struct bcmstrbuf *strbuf = &b;
	dol_frm_policy_t* policy;
	dhd_pub_t *dhdp;
	char policy_str[DOL_IOV_BUFF_SIZE];

	DOL_TRACE("%s: dhdol=<0x%px>, buff=<0x%px>, bufflen=<%d>\n",
	    __FUNCTION__, dhdol, buf, buflen);

	if (!dhdol->sup_feat.nplusm) {
	    DOL_ERROR("    N+M Not Supported\n");
	    return BCME_UNSUPPORTED;
	}

	dhdp = dhdol->dhd;

	bcm_binit(strbuf, buf, buflen);

	bcm_bprintf(strbuf, "[ Name  ] [id] [Policy]\n");
	policy = &dhdol_frm_policies[dhdp->unit];

	for (id = dol_frm_policy_id_global; id < dol_frm_policy_id_max; id++) {
	    bcm_bprintf(strbuf, " %7s ", dhdol_frm_policy_id_str[id]);
	    if (id == policy->id) {
	        policy_str[0] = '\0';
	        dol_frm_get_policy_str(policy, policy_str, sizeof(policy_str));
	        bcm_bprintf(strbuf, "  *%d   %s\n", id, policy_str);
	    }
	    else
	        bcm_bprintf(strbuf, "   %d \n", id);
	}

	return (!strbuf->size ? BCME_BUFTOOSHORT : 0);
}

/**
 * Fills the Iovar string with the Rx Offload setting
 *
 */
static int
dol_iov_get_rxoffl(dhd_offload_t *dhdol, char *buf, int buflen)
{
	struct bcmstrbuf b;
	struct bcmstrbuf *strbuf = &b;
	dol_ring_cfg_t ring_cfg;

	DOL_TRACE("%s: dhdol=<0x%px>, buff=<0x%px>, bufflen=<%d>\n",
	    __FUNCTION__, dhdol, buf, buflen);

	if (dol_init_rxoffl(dhdol, &ring_cfg, NULL) != 0) {
	    DOL_ERROR("dol%d: Failed to get Rx Offload setting\n", DOL_UNIT);
	    return BCME_ERROR;
	}

	bcm_binit(strbuf, buf, buflen);

	bcm_bprintf(strbuf, "%d\n", ring_cfg.offload);

	return (!strbuf->size ? BCME_BUFTOOSHORT : 0);
}

/**
 * Fills the Iovar structure with the interface stats queried
 * from Offload helper
 *
 */
static int
dol_iov_get_hlpr_stats(dhd_offload_t *dhdol, char *buf, int buflen)
{
	dhd_offload_stats_t *pstats = (dhd_offload_stats_t*)buf;
	uint32_t hlpr_drop_cnt;
	int rc;

	if ((buf == NULL) || (buflen < sizeof(dhd_offload_stats_t))) {
	    DOL_ERROR("dol%d: iovar [%d] bad arguments [buf 0x%px, buflen %d]\n",
	        DOL_UNIT, DHD_OFFLOAD_IOV_STATS, buf, buflen);
	    return BCME_BADARG;
	}

	pstats->mcast_pkts = 0;
	pstats->mcast_bytes = 0;
	pstats->dropped_pkts = 0;

	if (!DOL_SUPPORTED(dhdol)) {
	    /* Offload Helper does not support this radio, return 0 stats */
	    return BCME_OK;
	}

	rc = dol_hlpr_get_ssid_tx_dropped_pkts(&dhdol->hlpr, pstats->ifidx,
	        &hlpr_drop_cnt);

	if (rc < 0) {
	    return BCME_ERROR;
	}

	pstats->dropped_pkts = hlpr_drop_cnt;

	return BCME_OK;
}

/**
 * Dump DHD offload status, configuration and counters
 *
 * Display DHD offload helper status
 * Display flowring policy and profile configuration
 * parse and Display number of hw/sw flowrings per wme_ac
 * Display O2D request and D2O notification counters
 *
 */
static int
dol_iov_dump(dhd_offload_t *dhdol, char *buff, int bufflen)
{
	struct bcmstrbuf *b = (struct bcmstrbuf *)buff;
	dol_frm_t *frm;
	enum dhd_wme_ac wme_ac;
	char policy_str[DOL_IOV_BUFF_SIZE];

	frm = &dhdol->frm;

	/* Status */
	bcm_bprintf(b, "\n");
	bcm_bprintf(b, "dol_Helper  : %s (radio %d)\n", DOL_HLPR, DOL_RADIO_IDX);

	bcm_bprintf(b, " dol_cap Hlp: %s  %s  %s  %s  %s  %s  %s  %s  %s  %s  %s  "
	    "%s  %s  %s  %s  %s  %s  %s  %s  %s %s %s\n",
	    DOL_HLPR_CAP_EN_STS_STR(dhdol, txoffl),
	    DOL_HLPR_CAP_EN_STS_STR(dhdol, rxoffl),
	    DOL_HLPR_CAP_STS_STR(dhdol, txoffl, txoffl, txcmpl2host),
	    DOL_HLPR_CAP_STS_STR(dhdol, txoffl, txoffl, dhdhdr),
	    DOL_HLPR_CAP_STS_STR(dhdol, txoffl, txoffl, lbraggr),
	    DOL_HLPR_CAP_STS_STR(dhdol, txoffl, rxoffl, msgringformat),
	    DOL_HLPR_CAP_STS_STR(dhdol, txoffl, txoffl, bkupq),
	    DOL_HLPR_CAP_STS_STR(dhdol, txoffl, rxoffl, hwawkup),
	    DOL_HLPR_CAP_STS_STR(dhdol, txoffl, txoffl, ffrd),
	    DOL_HLPR_CAP_STS_STR(dhdol, txoffl, txoffl, nplusm),
	    DOL_HLPR_CAP_STS_STR(dhdol, txoffl, txoffl, hbqd),
	    DOL_HLPR_CAP_STS_STR(dhdol, txoffl, txoffl, dynbkupq),
	    DOL_HLPR_CAP_STS_STR(dhdol, txoffl, txoffl, codel),
	    DOL_HLPR_CAP_STS_STR(dhdol, txoffl, rxoffl, cmnrngsz),
	    DOL_HLPR_CAP_STS_STR(dhdol, rxoffl, rxoffl, rxcmplv2),
	    DOL_HLPR_CAP_STS_STR(dhdol, rxoffl, rxoffl, rxcmplrch),
	    DOL_HLPR_CAP_STS_STR(dhdol, txoffl, rxoffl, ba256cfg),
	    DOL_HLPR_CAP_STS_STR(dhdol, txoffl, txoffl, idmagrpcfg),
	    DOL_HLPR_CAP_STS_STR(dhdol, txoffl, txoffl, idma64),
	    DOL_HLPR_CAP_STS_STR(dhdol, txoffl, rxoffl, pcap),
	    DOL_HLPR_CAP_STS_STR(dhdol, txoffl, rxoffl, dummytxc),
	    DOL_HLPR_CAP_STS_STR(dhdol, txoffl, rxoffl, txcpl_ewi));

	bcm_bprintf(b, " dol_cap SW : %s  %s\n",
		DOL_SWFEAT_STS_STR(dhdol, cpuqdpc),
		DOL_SWFEAT_STS_STR(dhdol, cmplnotif));

	if (dhdol->sup_feat.nplusm) {
	    /* Profile */
	    bcm_bprintf(b, " dol_prf TxP: prfl_id %d id_valu", frm->profile->id);
	    for (wme_ac = wme_ac_bk; wme_ac <= wme_ac_max; wme_ac++)
	        bcm_bprintf(b, "  %02d:%04d:%04d ",
	            frm->profile->weight[wme_ac], frm->phy_items[wme_ac],
	            frm->profile->bqsize[wme_ac]);
	    bcm_bprintf(b, "\n");

	    /* Policy */
	    policy_str[0] = '\0';
	    dol_frm_get_policy_str(&dhdol_frm_policies[DOL_UNIT],
	        policy_str, sizeof(policy_str));
	    bcm_bprintf(b, " dol_pol TxP: plcy_id %d id_valu %s\n",
	        dhdol_frm_policies[DOL_UNIT].id, policy_str);


	    /* TX flowrings */
	    bcm_bprintf(b, " dol_flr TxP:");
	    for (wme_ac = wme_ac_bk; wme_ac <= wme_ac_max; wme_ac++)
	        bcm_bprintf(b, " [%s] sw %d hw %d", dhdol_wme_ac_str[wme_ac],
	            dhdol->frm.sw_ring_cnt[wme_ac], dhdol->frm.hw_ring_cnt[wme_ac]);
	    bcm_bprintf(b, "\n");
	} else {
	    /* Profile */
	    bcm_bprintf(b, " dol_prf TxP: prfl_id %d id_valu", frm->profile->id);
	    for (wme_ac = wme_ac_bk; wme_ac <= wme_ac_max; wme_ac++)
	        bcm_bprintf(b, "  %04d:%04d ", frm->phy_items[wme_ac],
	            frm->profile->bqsize[wme_ac]);
	    bcm_bprintf(b, "\n");
	}

	bcm_bprintf(b, " dol_prf CmN: TxC %d RxC %d RxP %d\n",
	    dhdol->cfg.txs_ring.max_items, dhdol->cfg.rxc_ring.max_items,
	    dhdol->cfg.rxp_ring.max_items);

	/*
	  * local counters
	  *
	  * tx_post          Host notifies Offload Helper to post tx packets
	  * rx_compl         Host notifies Offload Helper to process D2H RxCompl
	  * tx_compl         Host notifies Offload Helper to process D2H TxCompl
	  * rx_compl         Offload Helper requests Host to receive a packet
	  * tx_compl         Offload Helper requests Host to free a packet
	  * wake_dngl        Offload Helper requests Host to wake dongle
	  */
	bcm_bprintf(b, " dol_cnt h2o: tx_post %lu rx_cmpl %lu tx_cmpl %lu txp_fail %lu\n",
	    dhdol->cntr.h2o_txp_notif, dhdol->cntr.h2o_rxc_notif,
	    dhdol->cntr.h2o_txc_notif, dhdol->cntr.h2o_txp_fail);
	bcm_bprintf(b, " dol_cnt o2h: rx_cmpl %lu tx_cmpl %lu wk_dngl %lu "
	    "sq_txp %lu (tx_cmpl+sq_txp) %lu\n",
	    dhdol->cntr.o2h_rxc_req, dhdol->cntr.o2h_txc_req,
	    dhdol->cntr.o2h_wkdngl_req, dhdol->cntr.o2h_sq_txp_notif,
	    (dhdol->cntr.o2h_txc_req + dhdol->cntr.o2h_sq_txp_notif));

	return BCME_OK;
}


/**
 * Fills the Iovar structure with the Offload status information
 *
 */
static int
dol_iov_get_status(dhd_offload_t *dhdol, char *buf, int buflen)
{
	dhd_offload_status_t *pstatus = (dhd_offload_status_t*)buf;
	int ac = wme_ac_bk;

	/* Sanity Check */
	if ((buf == NULL) || (buflen < sizeof(dhd_offload_status_t))) {
	    DOL_ERROR("dol%d: iovar [%d] bad arguments", DOL_UNIT,
	        DHD_OFFLOAD_IOV_STATUS);
	    DOL_ERROR("buf 0x%px, buflen %d\n", buf, buflen);
	    return BCME_BADARG;
	}

	/* Copy the supported and enabled features */
	memcpy(&pstatus->sup_features, &dhdol->sup_feat, sizeof(dol_hlpr_caps_t));
	memcpy(&pstatus->en_features, &dhdol->en_feat, sizeof(dol_hlpr_caps_t));

	pstatus->hw_flowrings = 0;
	pstatus->sw_flowrings = 0;

	while (ac <= wme_ac_max) {
	    pstatus->hw_flowrings += dhdol->frm.hw_ring_cnt[ac];
	    pstatus->sw_flowrings += dhdol->frm.sw_ring_cnt[ac];
	    ac++;
	}

	return BCME_OK;
}

/**
 * Iovar: Offload set not supported
 *
 */
static int
dol_iov_set_unsupported(dhd_offload_t *dhdol, char *buff, int buflen)
{
	DOL_ERROR("    Setting through DHD is deprecated\n");
	DOL_ERROR("    Instead use nvram kset, kcommit and reboot\n");

	return BCME_UNSUPPORTED;
}

/**
 * +----------------------------------------------------------------------------
 *           Section: Offload Data Path processing (dol_dp)
 * +----------------------------------------------------------------------------
 */

/**
 * function to processes RX packet from the Helper to DHD cpu queue
 * When Offload Helper posts buffers in the H2D RxPost common rings, these
 * buffers are not in cache and hence there is no need to cache invalidate
 * again before invoking O2D_RX_COMPL_REQUEST ops
 */
static bool
dol_dp_process_cpuq_rxc(dhd_offload_t *dhdol, int bound)
{
	int rc = 0;
	struct sk_buff *skb = NULL;
	hlpr_rxc_info_t info = {};
	int pkts = 0;

	ASSERT(dhdol != NULL);

	/*
	 * If RxOffload is not enabled, Helper will not send rxpkt messages
	 * on this queue
	 */
	if (!DOL_RXO_ENABLED(dhdol))
	    return FALSE;

	while (!bound || (pkts < bound)) {

	    /* Fetch each packet from Offload Helper to Host cpu queue */
	    rc = dol_hlpr_get_rxc_skb(&dhdol->hlpr, &skb, &info);
	    if (rc)
	        break;

#if defined(DOL_HLPR_CAP_RXCMPLV2)
	if (dhdol->en_feat.rxcmplv2 == 1) {
	    uint8 prio;

	    /* get and set the TID information */
	    prio = (info.wl_metadata >> DOL_HLPR_WL_METADATA_TID_SHIFT);
	    prio &= DOL_HLPR_WL_METADATA_TID_MASK;
	    skb->wl.ucast.dhd.wl_prio = prio;
	}
#endif /* DOL_HLPR_CAP_RXCMPLV2 */

	    /* Hand off to DHD to process packets missed in Offload Helper */
	    dhd_offload_request(dhdol, O2D_RX_COMPL_REQUEST, (unsigned long)skb,
	        (unsigned long)info.reason_data);

	    pkts++;
	}

	return (bound && (pkts >= bound));
}

/**
 * Function that processes Tx Complete packets from the dongle coming through
 * Helper to DHD cpu queue
 */
static bool
dol_dp_process_cpuq_txs(dhd_offload_t *dhdol, int bound)
{
	int rc = 0;
	int pkts = 0;
	host_txbuf_cmpl_t   *dngl_txs;
	dhd_offload_txsts_t dol_txs;
	hlpr_txc_data_t     hlpr_txs = { 0 };
	dhd_pub_t *dhdp;

	ASSERT(dhdol != NULL);
	ASSERT(dhdol->dhd != NULL);

	dhdp = dhdol->dhd;
	/*
	 * If TxOffload is not enabled, Helper will not send txcmpl messages on
	 * this queue
	 */
	if (!DOL_TXO_ENABLED(dhdol))
	    return FALSE;

	hlpr_txs.radio_idx = DOL_RADIO_IDX;

	while (!bound || (pkts < bound)) {

	    /* Fetch each tx complete packet from Helper queue */
	    rc = dol_hlpr_get_txc_data(&dhdol->hlpr, &hlpr_txs);

	    if (rc)
	        break;

	    dol_txs.pkt = NULL;
	    dngl_txs = &dol_txs.dngl_txsts;
	    dngl_txs->cmn_hdr.request_id = htonl(hlpr_txs.request_id);
	    dngl_txs->tx_status          = hlpr_txs.status; /* already byte-swapped */

#if defined(DOL_HLPR_CAP_TXCMPL2HOST)
	    if ((hlpr_txs.buf_type == RDPA_DHD_TX_POST_HOST_BUFFER_VALUE) &&
	        (hlpr_txs.txp)) {
	        dol_txs.pkt = hlpr_txs.txp;
	    }
#endif /* DOL_HLPR_CAP_TXCMPL2HOST */

	    if (dhdol->cfg.txs_ring.type) {

#if defined(DOL_HLPR_TXS_AUDIT)
	        /*
	         * Filter out the spurious/unwanted Tx Complete messages from Helper
	         */
	        if (hlpr_txs.buf_type == DOL_HLPR_TXP_HOST_BUFF) {
	            if ((hlpr_txs.txp == (void*)-1) || (hlpr_txs.txp == NULL)) {
	                DOL_ERROR("dol%d: Discard TxS: pkt[0x%px], reqid[0x%x]\n",
	                    DOL_UNIT, hlpr_txs.txp,
	                    ltoh32(dngl_txs->cmn_hdr.request_id));
	                continue;
	            }
	        } else if (ltoh32(dngl_txs->cmn_hdr.request_id) == 0) {
	            DOL_ERROR("dol%d: Discard TxS: req_id[0x%x], pkt[0x%px]\n",
	                DOL_UNIT, ltoh32(dngl_txs->cmn_hdr.request_id),
	                hlpr_txs.txp);
	            continue;
	        }
#endif /* DOL_HLPR_TXS_AUDIT */

	        dhd_offload_request(dhdol, O2D_TX_COMPL_REQUEST,
	            (unsigned long)&dol_txs, 0);
	    } else {
	        /* legacy message formats */
	        dngl_txs->tx_status = htons(hlpr_txs.status);
	        dngl_txs->compl_hdr.flow_ring_id = htons(hlpr_txs.flow_ring_id);
	        dngl_txs->cmn_hdr.msg_type =  MSG_TYPE_TX_STATUS;
	        dngl_txs->cmn_hdr.if_id = 0;
	        DOLPR2("req_id = 0x%08x, status = 0x%04x ring_id = 0x%04x\n",
	            dngl_txs->cmn_hdr.request_id, dngl_txs->tx_status,
	            dngl_txs->compl_hdr.flow_ring_id);


#if defined(DOL_HLPR_TXS_AUDIT)
	        /*
	         * Filter out the spurious/unwanted Tx Complete messages
	         *
	         * Note: Offload Helper might send them during re-load of DHD
			 * driver as Helper doesn't support DHD unload and reload yet
	         */
	        if ((ltoh32(dngl_txs->cmn_hdr.request_id) == 0)||
	            (ltoh16(dngl_txs->compl_hdr.flow_ring_id) < FLOW_RING_COMMON)||
	            (ltoh16(dngl_txs->compl_hdr.flow_ring_id) >
	            (dhdol->frm.max_h2d_rings - MAX_DYN_FLOWRINGS(dhdp))) ||
#if defined(DOL_HLPR_CAP_TXCMPL2HOST)
	            ((hlpr_txs.buf_type == RDPA_DHD_TX_POST_HOST_BUFFER_VALUE) &&
	             ((hlpr_txs.txp == (void*)-1) || (hlpr_txs.txp == NULL)))||
#endif /* DOL_HLPR_CAP_TXCMPL2HOST */
	            (ltoh16(dngl_txs->tx_status) != 0)) {
	            DOL_ERROR("dol%d: Discard TxS: req_id[0x%x], ring_id[0x%x]",
	                DOL_UNIT, ltoh32(dngl_txs->cmn_hdr.request_id),
	                ltoh16(dngl_txs->compl_hdr.flow_ring_id));
	            DOL_ERROR(" status [0x%x] buf_type [%d] txp [0x%px]\n",
	                ltoh16(dngl_txs->tx_status),
	                hlpr_txs.buf_type, hlpr_txs.txp);
	            continue;
	        }
#endif /* DOL_HLPR_TXS_AUDIT */
	        dhd_offload_request(dhdol, O2D_TX_COMPL_REQUEST,
	            (unsigned long)&dol_txs, 0);
	    }
	    pkts++;
	}

	return (bound && (pkts >= bound));
}

#if defined(DOL_HLPR_CAP_PCAP)
/**
 * Function that processes PCAP packets from the dongle coming through
 * Runner to DHD cpu queue
 */
static bool
dol_dp_process_cpuq_pcap(dhd_offload_t *dhdol, int type, int bound)
{
#if defined(BCM_PCAP_DOL)
	int rc = 0;
	int pkts = 0;
	hlpr_pcap_data_t     hlpr_pcap = { 0 };
	dhd_pub_t *dhd;

	/* PCAP_TX_COPY_PKT=0, PCAP_RX_COPY_PKT=1 */
	void (*dhd_pcap_fn[])(dhd_pub_t *dhd, pcap_pkt_t *pkt) = { dhd_prot_runner_pcap_txcopy,
	                                                           dhd_prot_runner_pcap_rxcopy };

	ASSERT(dhdol != NULL);
	ASSERT(dhdol->dhd != NULL);

	dhd = dhdol->dhd;

	hlpr_pcap.radio_idx = DOL_RADIO_IDX;
	hlpr_pcap.pcap_type = type;

	/*
	 * If Tx/Rx Offload is not enabled, Helper will not send pcap messages on
	 * this queue
	 */
	if (((!DOL_TXO_ENABLED(dhdol)) && (hlpr_pcap.pcap_type == PCAP_TX_COPY_PKT)) ||
	   ((!DOL_RXO_ENABLED(dhdol)) && (hlpr_pcap.pcap_type == PCAP_RX_COPY_PKT)))
	    return FALSE;

	while (!bound || (pkts < bound)) {

	    /* Fetch each tx complete packet from Helper queue */
	    rc = dol_hlpr_get_pcap_data(&dhdol->hlpr, &hlpr_pcap);

	    if (rc)
	        break;

	    if (hlpr_pcap.txp) {

	        pcap_pkt_t pkt;

	        pkt.data        = hlpr_pcap.txp;
	        pkt.flags       = hlpr_pcap.flags;
	        pkt.pkt_len     = hlpr_pcap.pkt_len;
	        pkt.buf_len     = hlpr_pcap.data_buf_len;
	        pkt.buf_type    = hlpr_pcap.buf_type;
	        pkt.data_offset	= hlpr_pcap.pkt_offset;
		pkt.seq         = hlpr_pcap.seq_num;

	        dhd_pcap_fn[type](dhd, &pkt);
	        hlpr_pcap.txp = (void *)((uintptr)hlpr_pcap.txp | FKBUFF_PTR);
	        bdmf_sysb_recycle(hlpr_pcap.txp, 0, 0);
	   }
	   pkts++;
	}

	return (bound && (pkts >= bound));
#else /* ! BCM_PCAP_DOL */
	return 0;
#endif /* BCM_PCAP_DOL */
}
#endif /* DOL_HLPR_CAP_PCAP */

/**
 * dpc thread function that processes packets from the Helper to DHD cpu queue.
 * Processes available packets to the max bound limits and enables interrupts
 * if there is no more packets to process
 * Input: resched is ignored currently
 */
static bool
dol_dp_process_cpuq(dhd_offload_t *dhdol, bool resched)
{

	/* Process max of rxbound RxComplete packets */
	resched = dol_dp_process_cpuq_rxc(dhdol, dhd_rxbound);

	resched |= !dol_hlpr_empty_cpuq(&dhdol->hlpr);

	/* Process max of txbound TxComplete packets */
	resched |= dol_dp_process_cpuq_txs(dhdol, dhd_txbound);

#if defined(DOL_HLPR_CAP_PCAP)
	/* Process max of txbound TxCopy PCAP packets */
	resched |= dol_dp_process_cpuq_pcap(dhdol, PCAP_TX_COPY_PKT, dhd_txbound);

	/* Process max of txbound RxCopy PCAP packets */
	resched |= dol_dp_process_cpuq_pcap(dhdol, PCAP_RX_COPY_PKT, dhd_txbound);
#endif

	if (resched == FALSE) {
	    /* Re-enable interrupts on Offload Helper to Host cpu queue */
	    dhdol->ipend = FALSE;

	    dol_hlpr_enable_cpuq_int(&dhdol->hlpr);
	}

	return resched;
}

/**
 * tasklet that processes packets from the Helper to DHD cpu queue
 * Processes all available packets before enabling the interrupts
 */
static void
dol_dp_cpuq_tasklet(unsigned long data)
{
	dhd_offload_t *dhdol = (dhd_offload_t *)data;

	ASSERT(dhdol != NULL);

	if (dhdol->dhd->busstate == DHD_BUS_DOWN) {
	    DOL_INFO("%s: dol%d: PCIe bus down, ret\n", __FUNCTION__, DOL_UNIT);
	    return;
	}

	if (dol_dp_process_cpuq(dhdol, FALSE)) {
	    /* There may be more packets pending, reschedule */
	    tasklet_schedule(&dhdol->dol_cpuq_tasklet);
	}

	return;
}

/**
 * +----------------------------------------------------------------------------
 *           Section: DHD Interface processing (dhd_offload)
 * +----------------------------------------------------------------------------
 */

/**
 * Init: Attach offload object instance to DHD
 */
struct dhd_offload_obj *
dhd_offload_attach(dhd_pub_t *dhd, bcmpcie_soft_doorbell_t *soft_doorbells)
{
	int rc;
	int old_trace_level = bdmf_global_trace_level;
	dhd_offload_t *dhdol = NULL;
	bcmpcie_soft_doorbell_t *tx_soft_doorbell, *rx_soft_doorbell;
	hlpr_wakeup_info_t wakeup_info = {dhd->unit, 0, 0, 0, 0};

	DOL_INFO("%s: dhd<%px>\n", __FUNCTION__, dhd);

	dhdol = (dhd_offload_t *)MALLOCZ(dhd->osh, sizeof(dhd_offload_t));
	if (dhdol == NULL) {
	    DOL_ERROR("%s: failed DHD helper context malloc\n", __FUNCTION__);
	    return NULL;
	}

	dhdol->dhd = dhd;
	if (dol_hlpr_attach(dhdol) == NULL) {
	    DOL_ERROR("%s: Failed Offload Helper attach\n", __FUNCTION__);
	    rc = BCME_ERROR;
	    goto exit;
	}

	/* Initialize Coherent Memory Pool (cmp) */
	dhdol->cmp.len = DOL_CMP_SIZE;
	dhdol->cmp.va = dol_mem_alloc_dmabuf(dhdol, &dhdol->cmp,
	                    DOL_MEM_ALLOC_TYPE_COHERENT);

	if (dhdol->cmp.va == NULL) {
	    DOL_ERROR("%s: Failed to Allocate coherent memory \n", __FUNCTION__);
	    rc = BCME_NOMEM;
	    goto exit;
	}

	/*
	 * Use len to store total allocated and alloced to get the total
	 * allocated in the local pool
	 */
	dhdol->cmp._alloced = 0;
	DOL_INFO("dol%d: Allocated 0x%x size coherent mem pool 0x%px, 0x%x_%x\n",
	    DOL_UNIT, dhdol->cmp.len, dhdol->cmp.va,
	    (uint32)PHYSADDRHI(dhdol->cmp.pa), (uint32)PHYSADDRLO(dhdol->cmp.pa));

	/*
	 * Lets tell the dongle where the tx complete and rx complete helper wake up
	 * registers are and the value to write so as to directly wake up the
	 * corresponding threads in Offload Helper without DHD intervention.
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
	 * DHD should update the type and size based on dongle support in
	 * prot_pre_init
	 */
	dhdol->cfg.txp_ring.max_items = H2DRING_TXPOST_MAX_ITEM;
	dhdol->cfg.txp_ring.size = H2DRING_TXPOST_ITEMSIZE;
	dhdol->cfg.txp_ring.type = 0;    /* Legacy */

	dhdol->cfg.rxp_ring.max_items = H2DRING_RXPOST_MAX_ITEM;
	dhdol->cfg.rxp_ring.size = H2DRING_RXPOST_ITEMSIZE;
	dhdol->cfg.rxp_ring.type = 0;    /* Legacy */

	dhdol->cfg.txs_ring.max_items = D2HRING_TXCMPLT_MAX_ITEM;
	dhdol->cfg.txs_ring.size = D2HRING_TXCMPLT_ITEMSIZE;
	dhdol->cfg.txs_ring.type = 0;    /* Legacy */

	dhdol->cfg.rxc_ring.max_items = D2HRING_RXCMPLT_MAX_ITEM;
	dhdol->cfg.rxc_ring.size = D2HRING_RXCMPLT_ITEMSIZE;
	dhdol->cfg.rxc_ring.type = 0;    /* Legacy */

	/* Initialize tx ring offload setting */
	dol_init_txoffl(dhdol, &dhdol->cfg.txs_ring);
	DOLOG("%s: Tx Offload - %s, Ring Size = %d", __FUNCTION__,
	    (dhdol->cfg.txs_ring.offload) ? "Enabled" : "Disabled",
	    dhdol->cfg.txs_ring.max_items);

	/* Initialize rx ring offload setting */
	dol_init_rxoffl(dhdol, &dhdol->cfg.rxc_ring,
	    &dhdol->cfg.rxp_ring);
	DOLOG("%s: Rx Offload - %s, Ring Size = %d", __FUNCTION__,
	    (dhdol->cfg.rxc_ring.offload) ? "Enabled" : "Disabled",
	    dhdol->cfg.rxc_ring.max_items);

	wakeup_info.radio_idx = DOL_RADIO_IDX;

	if (DOL_SUPPORTED(dhdol)) {
	    /* Get wakeup register information from helper */
	    dol_hlpr_get_wakeup_info(&dhdol->hlpr, &wakeup_info);
	}

	if (DOL_TXO_ENABLED(dhdol)) {
	    /* Setup Tx complete thread wake up doorbell */
	    rc = dol_init_hlpr_wakeup(dhdol, tx_soft_doorbell,
	        wakeup_info.tx_complete_wakeup_register,
	        HTOL32(wakeup_info.tx_complete_wakeup_value));
	    if (rc) {
	        DOL_ERROR("dol%d: failed to initialize %s tx wakeup",
	            DOL_UNIT, DOL_HLPR);
	        DOL_ERROR(" doorbell = <0x%px> reg = <0x%llx>, val = <0x%08x>\n",
	            tx_soft_doorbell,
	            (uint64_t)wakeup_info.tx_complete_wakeup_register,
	            HTOL32(wakeup_info.tx_complete_wakeup_value));
	        goto exit;
	    }
	    dhdol->en_feat.txoffl = 1;
	    DOLOG("TX wakeup info: reg = <0x%llx>, val = <0x%08x>",
	        (uint64_t)wakeup_info.tx_complete_wakeup_register,
	        HTOL32(wakeup_info.tx_complete_wakeup_value));
	}

	if (DOL_RXO_ENABLED(dhdol)) {
	    /* Setup Rx complete thread wake up doorbell */
	    rc = dol_init_hlpr_wakeup(dhdol, rx_soft_doorbell,
	        wakeup_info.rx_complete_wakeup_register,
	        HTOL32(wakeup_info.rx_complete_wakeup_value));
	    if (rc) {
	        DOL_ERROR("dol%d: %s rx wakeup init failed", DOL_UNIT, DOL_HLPR);
	        DOL_ERROR(" doorbell = <0x%px> reg = <0x%llx>, val = <0x%08x>\n",
	            rx_soft_doorbell,
	            (uint64_t)wakeup_info.rx_complete_wakeup_register,
	            HTOL32(wakeup_info.rx_complete_wakeup_value));
	        goto exit;
	    }

	    dhdol->en_feat.rxoffl = 1;
	    DOLOG("RX wakeup info: reg = <0x%llx>, val = <0x%08x>",
	        (uint64_t)wakeup_info.rx_complete_wakeup_register,
	        HTOL32(wakeup_info.rx_complete_wakeup_value));
	}

	/* enable/disable swfeatures from default/storage settings */
	rc = dol_init_swfeatures(dhdol);
	if (rc) {
	    DOL_ERROR("dol%d: swfeatures init failed\n", DOL_UNIT);
	    goto exit;
	}

	return dhdol;

exit:
	if (rc < 0)
	    dhd_offload_detach(dhd, dhdol);
	bdmf_global_trace_level = old_trace_level;

	return (dhd_offload_t*)NULL;
}

/**
 * Init: Detach offload object instance from DHD
 */
void
dhd_offload_detach(dhd_pub_t *dhd, struct dhd_offload_obj *dhdol)
{
	DOL_INFO("%s: dhdol<%px> dhd<%px>\n", __FUNCTION__, dhdol, dhd);

	if (dhdol == NULL)
	    return;

	hlpr_set_nonblocking_log();

	dol_hlpr_detach(dhdol);

	dol_deinit_pcie(dhdol);

	if (dhdol->cmp.va != NULL) {
	    dol_mem_free_dmabuf(dhdol, &dhdol->cmp, DOL_MEM_ALLOC_TYPE_COHERENT);
	    DOL_INFO("[COHERENT_POOL_%d] Freed 0x%x size pool 0x%px\n",
	        DOL_UNIT, dhdol->cmp.len, dhdol->cmp.va);
	    dhdol->cmp.va = NULL;
	}

	DOL_MEM_ALLOC_CNTR_DUMP(dhdol);

	dhdol->dhd = NULL;
	MFREE(dhd->osh, dhdol, sizeof(struct dhd_offload_obj));

	hlpr_clear_nonblocking_log();
}

/**
 * Operations: Host DHD notifies Offload helper to perform an operation.
 */
int
dhd_offload_notify(struct dhd_offload_obj *dhdol, dhd_offload_ops_t ops,
	unsigned long arg1, unsigned long arg2)
{
	dhd_pub_t *dhd;
	int	rc = BCME_OK;

	DOL_INFO("%s: D2O dhdol<%px> ops<%d> <0x%lx> <0x%lx>\n",
	    __FUNCTION__, dhdol, ops, arg1, arg2);

	if ((dhdol == NULL) || (dhdol->dhd == NULL)) {
	    DOL_ERROR("%s: invalid arg dhdol<%px>\n", __FUNCTION__, dhdol);
	    return BCME_BADARG;
	}

	dhd = dhdol->dhd;

	switch (ops) {

	    /* Host requests Offload to read DMA Index buffer */
	    case D2O_IDX_BUF_RD_REQUEST: /* arg1:buffer ptr, arg2:read value ptr */
	        DOL_TRACE("D2O_IDX_BUF_RD_REQUEST ptr<0x%px> valptr<0x%px>\n",
	            (uint16*)arg1, (uint32*)arg2);

	        if (DOL_DMA_IDX_SZ(dhd) == sizeof(uint32)) {
	            uint32* ptr = (uint32*)arg1;
	            uint32 val;

	            if (DOL_MEM_TYPE_HLPR != DOL_MEM_ALLOC_TYPE_COHERENT) {
	                WLAN_OSL_CACHE_INV(dhd->osh, (void *)ptr, sizeof(uint32));
	            }
	            val = LTOH32(*(ptr));
	            *(uint32*)arg2 = val;
	        } else {
	            uint16* ptr = (uint16*)arg1;
	            uint16 val;

	            if (DOL_MEM_TYPE_HLPR == DOL_MEM_ALLOC_TYPE_COHERENT) {
	                ARCH_RD_DMA_IDX(ptr, val);
	            } else {
	                WLAN_OSL_CACHE_INV(dhd->osh, (void *)ptr, sizeof(uint16));
	                val = LTOH16(*(ptr));
	            }
	            *(uint16*)arg2 = val;
	        }
	        break;

	    /* Host requests Offload to write DMA Index buffer */
	    case D2O_IDX_BUF_WR_REQUEST: /* arg1:buffer ptr, arg2:write value */
	        DOL_TRACE("D2O_IDX_BUF_WR_REQUEST ptr<0x%px> val<0x%d>\n",
	            (uint16*)arg1, (uint32)arg2);

	        if (DOL_DMA_IDX_SZ(dhd) == sizeof(uint32)) {
	            uint32* ptr = (uint32*)arg1;
	            uint32  val = (uint32)arg2;

	            if (DOL_MEM_TYPE_HLPR == DOL_MEM_ALLOC_TYPE_COHERENT) {
	                iowrite32(val, ptr);
	            } else {
	                *ptr = htol32(val);
	                WLAN_OSL_CACHE_FLUSH(dhd->osh, (void *)ptr, sizeof(uint32));
	            }
	        } else {
	            uint16* ptr = (uint16*)arg1;
	            uint16  val = (uint16)arg2;

	            if (DOL_MEM_TYPE_HLPR == DOL_MEM_ALLOC_TYPE_COHERENT) {
	                ARCH_WR_DMA_IDX(ptr, val);
	            } else {
	                *ptr = htol16(val);
	                WLAN_OSL_CACHE_FLUSH(dhd->osh, (void *)ptr, sizeof(uint16));
	            }
	        }
	        break;

	    /* Host notifies Offload to post initial buffers */
	    case D2O_RXPOST_NOTIF:
	        DOL_TRACE("D2O_RXPOST_NOTIF\n");

	        if (!DOL_RXO_SUPPORTED(dhdol))
	            return BCME_UNSUPPORTED;

	        if (DOL_RXO_ENABLED(dhdol) &&
	            (dol_hlpr_init_rx_post(&dhdol->hlpr))) {
	            rc = BCME_NOMEM;
			}
	        break;

	    /* Host notifies Offload to free buffers in RxPost */
	    case D2O_RXPOST_FREE_NOTIF:
	        DOL_TRACE("D2O_RXPOST_FREE_NOTIF\n");

	        if (!DOL_RXO_SUPPORTED(dhdol))
	            return BCME_UNSUPPORTED;

	        if (DOL_RXO_ENABLED(dhdol) &&
	            (dol_hlpr_uninit_rx_post(&dhdol->hlpr))) {
	            rc = BCME_ERROR;
			}

	        break;

	    /* Host notifies Offload Helper to re-initialize buffers in RxPost */
	    case D2O_RXPOST_REINIT_NOTIF:
	        DOL_TRACE("D2O_RXPOST_REINIT_NOTIF\n");

	        if (!DOL_RXO_SUPPORTED(dhdol))
	            return BCME_UNSUPPORTED;

	        if (DOL_RXO_ENABLED(dhdol) &&
	            (dol_hlpr_reinit_rx_post(&dhdol->hlpr))) {
	            rc = BCME_ERROR;
			}
	        break;

	    /* Host notifies Offload Helper to process D2H RxCompl */
	    case D2O_RX_COMPL_NOTIF:
	        DOL_TRACE("D2O_RX_COMPL_NOTIF, unit %d\n", DOL_UNIT);

	        if ((dhdol->ipend == FALSE) && (dhdol->sw_feat.cmplnotif == TRUE)) {
	            dol_hlpr_wakeup_complete(&dhdol->hlpr, FALSE);
	            dhdol->cntr.h2o_rxc_notif++;
	        }
	        break;

	    /* Host notifies Offload Helper to transmit a packet */
	    case D2O_TXPOST_NOTIF: /* arg1:pktptr arg2:ifid */
	        DOL_TRACE("D2O_TXPOST_NOTIF pkt<0x%px> len<%d> fr<%d> ifidx<%d>\n",
	            (void*)arg1, PKTLEN(dhd->osh, (void*)arg1),
	            DHD_PKT_GET_FLOWID((void*)arg1), (uint32)arg2);
	        DOLPR1("D2O_TXPOST_NOTIF pkt<0x%px> ifid<%d>", (void*)arg1,
	            (int)arg2);

	        rc = dol_hlpr_txpost(dhdol, (void*)arg1, (uint32)arg2);
	        return rc;

	    /* Host notifies Offload Helper to process D2H TxCompl */
	    case D2O_TX_COMPL_NOTIF:
	        DOL_TRACE("D2O_TX_COMPL_NOTIF\n");
	        DOLPR1("D2O_TX_COMPL_NOTIF");

	        if ((dhdol->ipend == FALSE) && (dhdol->sw_feat.cmplnotif == TRUE)) {
	            dol_hlpr_wakeup_complete(&dhdol->hlpr, TRUE);
	            dhdol->cntr.h2o_txc_notif++;
	        }
	        break;

	    /* Host notifies Offload Helper to attach/detach/register a DMA Buf */
	    case D2O_DMA_BUF_NOTIF:
	        DOL_TRACE("D2O_DMA_BUF_NOTIF type<0x%08x> dma_buf<0x%px>\n",
	            (int)arg1, (dhd_dma_buf_t*)arg2);

	        return dol_ops_dma_buf_notif(dhdol, dhd->osh,
	                 (dhd_offload_dma_buf_t)arg1, (dhd_dma_buf_t*)arg2);

	    case D2O_FLRING_INIT_NOTIF:
	        DOL_TRACE("D2O_FLRING_CACHE_NOTIF arg1<%d> arg2<0x%08x>\n",
	            (int)arg1, (int)arg2);
	        ASSERT(arg1 == dhdol->frm.max_h2d_rings);
	        *((hlpr_fr_cache_t**)arg2) = dhdol->frm.hlpr_fr_cache;
	        break;

	    /* Host notifies Offload Helper to complete configuration */
	    case D2O_INIT_NOTIF:
	        DOL_TRACE("D2O_INIT_NOTIF dhdol<0x%px> pci_dev<0x%px>\n",
	                dhdol, (struct pci_dev*)arg1);

	        hlpr_set_nonblocking_log();
	        rc = dol_ops_init_notif(dhdol, (struct pci_dev*)arg1);
	        hlpr_clear_nonblocking_log();
	        return rc;
		break;

	    /* Host notifies Offload Helper to enable a flowring */
	    case D2O_FLRING_ENAB_NOTIF: /* arg1:flowid [2..N] */
	        DOL_TRACE("D2O_FLRING_ENAB_NOTIF flowring<%d>\n", (int)arg1);

	        rc = dol_ops_fr_enab_notif(dhdol, (int)arg1, 1);
	        break;

	    /* Host notifies Offload Helper to disable a flowring */
	    case D2O_FLRING_DISAB_NOTIF: /* arg1:flowid [2..N] */
	        DOL_TRACE("D2O_FLRING_DISAB_NOTIF flowring<%d>\n", (int)arg1);

	        rc = dol_ops_fr_enab_notif(dhdol, (int)arg1, 0);
	        break;

	    /* Host notifies Offload Helper to flush a flowring */
	    case D2O_FLRING_FLUSH_NOTIF:
			/* arg1:flowid [2..N] arg2:rd_idx [0..0xfffe, 0xffff] */
	        DOL_TRACE("D2O_FLRING_FLUSH_NOTIF flowring<%d> data<0x%x>\n",
	            (int)arg1, (uint16)arg2);

#if defined(DOL_HLPR_CAP_FFRD)
	        {
	            hlpr_msgring_data_t data;

	            data.flowring_idx = arg1;
	            data.read_idx = arg2;
	            data.idx_valid = (arg2 < 0xFFFF) ? 1 : 0;
	            rc = dol_hlpr_set_flush(&dhdol->hlpr, data.u32);
	        }
#else /* !DOL_HLPR_CAP_FFRD */
	        rc = dol_hlpr_set_flush(&dhdol->hlpr, arg1);
#endif /* !DOL_HLPR_CAP_FFRD */

	        if (rc != 0) {
	            return BCME_ERROR;
	        }
	        break;

	    /* Host notifies Offload Helper to update backup queue size */
	    case D2O_FLRING_BQSIZE_NOTIF: /* arg1:flowid [2.N] arg2:qd [0.0xffff] */
	        DOL_TRACE("D2O_FLRING_BQSIZE_NOTIF flowring<%d> bq size <0x%x>\n",
	            (int)arg1, (uint16)arg2);

	        if (DOL_BKUPQ(dhdol)) {
	            dol_ops_fr_bqsize_notif(dhdol, arg1, arg2);
	        }
	        break;

	    /* Host notifies Offload Helper to configure aggregation */
	    case D2O_AGGR_CONFIG_NOTIF: /* arg1:dhd_offload_aggr_config_t* */
	        DOL_TRACE("D2O_AGGR_CONFIG_NOTIF aggr_config<0x%px>\n",
	            (void*)arg1);

	        /* Check if any tx flow rings are offloaded */
	        if (DOL_TXO_ENABLED(dhdol)) {
				return dol_ops_aggr_config_notif(dhdol, arg1, arg2);
	        }
	        break;

	    /*
		 * Host notifies Offload Helper to set/unset sending nonacceleted
		 * packet txstatus to dhd
		 */
	    case D2O_TXSTS_CONFIG_NOTIF: /* arg1: enable */
	        DOL_TRACE("D2O_TXSTS_CONFIG_NOTIF enable <%d>\n", (int)arg1);

	        if (DOL_TXO_ENABLED(dhdol)) {

	            dhdol->cfg.txs_ring.offload = arg1 & DOL_TXS_CFG_OFFL_MASK;

	            if (!(arg1 & DOL_TXS_CFG_NONACCPKT_OFFL) &&
	                (dhdol->sup_feat.txcmpl2host == 0)) {
	                /*
	                 * dhd requested to disable offload of txsts but Helper
					 * can not support sending nonacceleted packet txstatus
					 * to dhd
	                 */
	                dhdol->cfg.txs_ring.offload |= DOL_TXS_CFG_NONACCPKT_OFFL;
	                rc = BCME_UNSUPPORTED;
	                DOL_ERROR("dol%d: TXSTS2DHD not supported by %s\n",
	                    DOL_UNIT, DOL_HLPR);
	            }
	        }
	        break;

	    /*
	     * Host notifies Offload to configure PCIe MSGRING format and size
	     */
	    case D2O_MSGRING_FORMAT_NOTIF:
	        DOL_TRACE("D2O_MSGRING_FORMAT_NOTIF msgring<0x%x> val<0x%x>\n",
	            (uint8)arg1, (uint32)arg2);
			rc = dol_ops_msgring_fmt_notif(dhdol, arg1, arg2);
	        break;

	    /*
	     * Host notifies Offload of PCIE IPC Capabilities (DHD) for negotiation
	     */
	    case D2O_PCIE_IPC_CAP_NOTIF:
	        DOL_TRACE("D2O_PCIE_IPC_CAP_NOTIF cap_mask <0x%x> cap <0x%x>\n",
	            (uint32)arg1, *(uint32*)arg2);
	        rc = dol_ops_pcie_ipc_cap_notif(dhdol, arg1, arg2);
	        break;
	    case D2O_PCAP_CFG_NOTIF:
#if defined(DOL_HLPR_CAP_PCAP)
	        DOL_TRACE("D2O_PCAP_CFG_NOTIF rx_en<%d> tx_en<%d> maxlen<%d>\n",
	            (int)arg1 & 1, ((int)arg1 >> 1) & 1, (int)arg2);
	        rc = dol_hlpr_pcap_cfg_set(&dhdol->hlpr, arg1 & 1, (arg1 >> 1) & 1, arg2);

		if (rc == 0) {
	                dhdol->en_feat.pcap = (arg1 & 3) ? 1: 0;
	        }
#endif /* DOL_HLPR_CAP_PCAP */
	        break;
	    default:
	        DOL_ERROR("%s: Invalid D2O ops<%d> <0x%lx> <0x%lx>\n",
	                  __FUNCTION__, ops, arg1, arg2);
	        ASSERT(0);
	        return BCME_BADOPTION;
	}

	return rc;
}

/**
 * Operations: Offload helper requests Host DHD to handle an operation.
 */
int
dhd_offload_request(struct dhd_offload_obj *dhdol, dhd_offload_ops_t ops,
	unsigned long arg1, unsigned long arg2)
{
	dhd_pub_t *dhd;

	DOL_INFO("O2D dhdol<%px> ops<%d> <0x%lx> <0x%lx>\n",
	          dhdol, ops, arg1, arg2);

	if ((dhdol == NULL) || (dhdol->dhd == NULL)) {
	    DOL_ERROR("%s: invalid arg dhdol<%px>\n", __FUNCTION__, dhdol);
	    return BCME_BADARG;
	}

	dhd = dhdol->dhd;

	switch (ops) {

	    /* Request Host to receive a packet that missed in Offload Helper */
	    case O2D_RX_COMPL_REQUEST:
	        /* arg1:pkt ptr, arg2:info.reason_data:ifid */
	        DOL_TRACE("O2D_RX_COMPL_REQUEST pkt<0x%px> ifid<%d>\n",
	            (void*)arg1, (int)arg2);

	        if (DOL_RXO_ENABLED(dhdol)) {
	            DOLPR1("dhd_bus_rx_frame pkt<0x%px> if<%d>", (void*)arg1,
	                (int)arg2);
	            DHD_LOCK(dhd);
	            dhdol->cntr.o2h_rxc_req++;
	            dhd_bus_rx_frame(dhd->bus, (void *)arg1, (int)arg2, 1);
	            DHD_UNLOCK(dhd);
	        } else {
	            DOL_ERROR("dol%d: unexpected O2D_RX_COMPL_REQUEST <0x%lx> "
	                "<0x%lx>\n", dhd->unit, arg1, arg2);
	        }
	        break;

	    /* Offload Helper requests Host to free a packet - unused */
	    case O2D_TX_COMPL_REQUEST:
	        /* arg1:pkt ptr */
	        DOL_TRACE("O2D_TX_COMPL_REQUEST pkt<0x%px>\n", (void*)arg1);

	        if (DOL_TXO_ENABLED(dhdol)) {
	            DOLPR1("O2D_TX_COMPL_REQUEST pkt<0x%px>", (void*)arg1);
	            dhdol->cntr.o2h_txc_req++;
	            dhd_prot_runner_txstatus_process(dhd, (void*)arg1);
	        } else {
	            DOL_ERROR("dol%d: unexpected O2D_TX_COMPL_REQUEST <0x%lx>\n",
	                  dhd->unit, arg2);
	        }
	        break;

	    /* Offload Helper requests Host to wake dongle */
	    case O2D_WAKE_DNGL_REQUEST:
	        DOL_TRACE("O2D_WAKE_DNGL_REQUEST\n");
	        DOLPR1("O2D_WAKE_DNGL_REQUEST");

	        dhdol->cntr.o2h_wkdngl_req++;
	        dol_ops_wake_dngl_req(dhdol, (uint32)arg1, (uint32)arg2);
	        break;

	    default:
	        DOL_ERROR("%s: invalid O2D ops<%d> <0x%lx> <0x%lx>\n",
	                  __FUNCTION__, ops, arg1, arg2);
	        ASSERT(0);
	        return BCME_BADOPTION;
	}

	return BCME_OK;
}

/**
 * Flow ring manager: initialize
 */
void *
dhd_offload_flowmgr_init(dhd_offload_t *dhdol, int max_h2d_rings, int max_bss)
{
	int ac;
	int rings_to_allocate[wme_ac_max + 1];  /* reused for per tid, too */
	dol_frm_t *frm;
	dhd_pub_t *dhdp;
	dhd_dma_buf_t *dma_buf;
	hlpr_init_cfg_t *init_cfg;
	uint16 flow_id;

	dhdp = dhdol->dhd;
	frm = &dhdol->frm;
	init_cfg = &dhdol->hlpr.init_cfg;

	/* Determine the flring split for max_bss = bcmc and ucast per ac|tid */
	frm->max_h2d_rings = max_h2d_rings;
	frm->max_bss = max_bss;

	/*
	 * When per TID ucast flowrings are used, the per ac profile/policy will
	 * be applied for carving helper memory. As such the rings_to_allocate[]
	 * will count twice as much rings for per TID (as compared to per AC).
	 */
	frm->max_uc_rings = /* per ac or per tid, ucast tx post rings */
		(MAX_TX_FLOWRINGS(dhdp, max_h2d_rings) - max_bss) / wme_ac_max;

	ASSERT(max_h2d_rings == ((frm->max_uc_rings * wme_ac_max)
			+ BCMPCIE_H2D_COMMON_MSGRINGS + max_bss + MAX_DYN_FLOWRINGS(dhdp)));

	/* Pick the flow ring selector. */
	frm->select_fn = dol_frm_select_policy;
	frm->dhdol = dhdol;

	/* Setup the DMA-able buffer for flring cache in uncached memory. */
	dma_buf = &dhdol->fr_cache_base;
	if (dma_buf->va == NULL) {
	    dma_buf->len = sizeof(hlpr_fr_cache_t) * max_h2d_rings;
	    dma_buf->len += sizeof(dol_fr_cache_t) * max_h2d_rings;
	    dma_buf->len = ALIGN_SIZE(dma_buf->len, L1_CACHE_BYTES);

	    dma_buf->va = dol_mem_alloc_dmabuf(dhdol, dma_buf, DOL_MEM_TYPE_HLPR);

	    if (!dma_buf->va) {
	        DOL_ERROR("%s: fr_cache_base len<%d> alloc failure\n",
	            __FUNCTION__, dma_buf->len);
	        goto error_rtn;
	    }

	    /* Register with Offload Helper */
	    init_cfg->tx_post_mgmt_arr_base_addr = (void *)(dma_buf->va);
	    DOL_HLPR_INIT_CFG_SET_PA(init_cfg->tx_post_mgmt_arr_base_phys_addr,
	        dma_buf->pa, 0);
	    init_cfg->tx_post_mgmt_arr_entry_count = (uint32_t) frm->max_h2d_rings;

	    /* Register with flowmgr */
	    frm->hlpr_fr_cache = (hlpr_fr_cache_t*)dma_buf->va;
	    frm->dhd_flring_cache =
	        (dol_fr_cache_t*)(frm->hlpr_fr_cache + max_h2d_rings);
	}

	if (DOL_TXO_ENABLED(dhdol)) {
	    dol_ring_cfg_t *ring;

	    memset(frm->rsv_mem_addr, 0, frm->rsv_mem_size);
	    DOLOG("%s: dol%d: bootmem addr<%px> size<%u>", __FUNCTION__,
	        dhdp->unit, frm->rsv_mem_addr, frm->rsv_mem_size);

	    /* Initialize the base address */
	    ring = &dhdol->cfg.txp_ring;
	    dhdol->hlpr.init_cfg.tx_post_flow_ring_base_addr = (void *)
	        ((unsigned long)frm->rsv_mem_addr -
	        (BCMPCIE_H2D_COMMON_MSGRINGS * ring->max_items * ring->size));
	}

	/* Get flowring profile and policy from NVRAM area, if present */
	frm->profile = dol_frm_init_profile(dhdol);
	ASSERT(frm->profile != NULL);

	frm->policy = dol_frm_init_policy(dhdol);
	ASSERT(frm->policy != NULL);

	/* id16 map allocators for ucast rings per access category for N stations */
	for (ac = wme_ac_bk; ac < wme_ac_max; ac++) {
	    rings_to_allocate[ac] = frm->max_uc_rings;
	}

	/* id16 map allocators for BCMC rings (one per BSS) */
	rings_to_allocate[wme_ac_bcmc] = frm->max_bss;

	/* Check if full dor memory available if n+m is not enabled */
	if (dhdol->en_feat.txoffl == 1) {
	    uint32 ring_sz = 0;
	    uint32 full_dor_sz = 0;
	    bool force_m = FALSE;

	    for (ac = wme_ac_bk; ac <= wme_ac_max; ac++) {
	        ring_sz = frm->phy_items[ac] * dhdol->cfg.txp_ring.size;
	        full_dor_sz += ring_sz * rings_to_allocate[ac];
	        if (frm->profile->weight[ac] == 0) force_m = TRUE;
	    }

	    DOLOG(CLRyk "dol%d: TxOffl memory: Reserved<%d Mb> Needed<%d Mb>"
	        CLRnl, DOL_UNIT, B2MB(frm->rsv_mem_size), B2MB(full_dor_sz));

	    if (dhdol->en_feat.nplusm == 0) {
	        if (frm->rsv_mem_size < full_dor_sz) {
	            DOLOG(CLRerr "===============================================");
	            DOLOG("dol%d: TxOffl memory < Full Offload memory, aborting",
	                DOL_UNIT);
	            DOLOG("================================================" CLRnl);
	            goto error_rtn;
	        }
	        if (force_m == TRUE) {
	            DOLOG(CLRerr "===============================================");
	            DOLOG("dol%d: profile N+M settings not supported, aborting",
	                DOL_UNIT);
	            DOLOG("================================================" CLRnl);
	            goto error_rtn;
	        }
	    } else if ((frm->rsv_mem_size >= full_dor_sz) && (force_m == FALSE)) {
	        dhdol->en_feat.nplusm = 0;
	    }
	}

	/*
	 * Determine which flow ring id allocation mode to be used.
	 *
	 * Use AC based allocation if N+M or flowrings have different sizes
	 * Otherwise use iDMA mode if dongle suppots iDMA
	 * Else use legacy flat mode
	 */
	frm->alloc_mode = DOL_FR_ALLOC_DEFAULT;
	if (dhdol->en_feat.nplusm == 0) {
	    uint16 ring_phy_size;

	    frm->alloc_mode = DOL_FR_ALLOC_LEGACY;
	    if (IDMA_ACTIVE(dhdp)) {
	        frm->alloc_mode = DOL_FR_ALLOC_IDMA;
	    }

	    ring_phy_size = frm->phy_items[wme_ac_bk];
	    for (ac = wme_ac_bk; ac <= wme_ac_max; ac++) {
	        if (ring_phy_size != frm->phy_items[ac]) {
	            frm->alloc_mode = DOL_FR_ALLOC_WMEAC;
	            break;
	        }
	    }
	}
	DOLOG("dol%d: using %s based flowid allocation", dhdp->unit,
	    dhdol_frm_alloc_mode_str[frm->alloc_mode]);

	/* Initialize flowring id map with offload status */
	if (dol_frm_init_fr_map(frm) != BCME_OK)
	    goto error_rtn;

	/*
	 * Allocate dma buffers for flow rings and fill information needed
	 *  - DHD needs dma buffer virtual address
	 *  - Offload Helper needs physical address, AC, Offloaded, Enabled
	 */
	for (flow_id = FLOWID_RESERVED; flow_id < (frm->max_h2d_rings - MAX_DYN_FLOWRINGS(dhdp));
		flow_id++) {
	    if (dol_frm_init_fr(frm, flow_id) != BCME_OK)
	        goto error_rtn;
	}

	return frm;

error_rtn:

	dhd_offload_flowmgr_fini(dhdol, frm);
	return NULL;
}

/**
 * Flow ring manager: de-initialize
 */
void *
dhd_offload_flowmgr_fini(dhd_offload_t *dhdol, void *mgr)
{
	int ac, flring;
	dhd_pub_t *dhdp;
	hlpr_init_cfg_t *init_cfg;
	dol_frm_t *frm;
	hlpr_fr_cache_t *hlpr_cache;
	dol_fr_cache_t *dol_cache;

	if (!dhdol)
	    goto done;

	dhdp = dhdol->dhd;
	init_cfg = &dhdol->hlpr.init_cfg;
	frm = (dol_frm_t*)mgr;

	/* walk all entries in flowring cache (skip 1st 2) */
	if (frm->hlpr_fr_cache) {
	    for (flring = FLOWID_RESERVED; flring < (frm->max_h2d_rings - MAX_DYN_FLOWRINGS(dhdp));
			flring++)
	    {
	        void *va;
	        bool is_hw_ring;

	        hlpr_cache = frm->hlpr_fr_cache + flring;
	        dol_cache = frm->dhd_flring_cache + flring;
	        va = dol_cache->base_va;
	        is_hw_ring = ntohs(hlpr_cache->flags) & DOL_FR_IN_HLPR_FLAG;
	        if (va && !is_hw_ring) {
	            dhd_dma_buf_t dma_buf;

	            dma_buf.va = va;
	            PHYSADDRLOSET(dma_buf.pa, ntohl(hlpr_cache->base_addr_low));
	            PHYSADDRHISET(dma_buf.pa, ntohl(hlpr_cache->base_addr_high));
	            dma_buf.len = ntohs(DOL_PHY_RING_SIZE(hlpr_cache));
	            dma_buf.len *= dhdol->cfg.txp_ring.size;
	            dma_buf.dmah = NULL;

	            hlpr_cache->base_addr_low = 0U;
	            dol_cache->base_va = (void*)NULL;
	            hlpr_cache->items = 0;
#if defined(DOL_HLPR_CAP_BKUPQUEUE)
	            hlpr_cache->phy_ring_size = 0;
#endif /* DOL_HLPR_CAP_BKUPQUEUE */
	            hlpr_cache->flags = htons(DOL_FR_DISABLED_FLAG);
	            ARCH_FLUSH_COHERENT((void *)hlpr_cache, sizeof(hlpr_fr_cache_t));

	            dol_mem_free_dmabuf(dhdol, &dma_buf, DOL_MEM_TYPE_DHD);
	        } else if (va && is_hw_ring &&
	            (!(ntohs(hlpr_cache->flags) & DOL_FR_DISABLED_FLAG))) {
	            dol_ops_fr_enab_notif(dhdol, flring, 0);
	        }
	    }
	}

	if (dhdol->fr_cache_base.va) {
	    dhd_dma_buf_t *dma_buf = &dhdol->fr_cache_base;
	    dol_mem_free_dmabuf(dhdol, dma_buf, DOL_MEM_TYPE_HLPR);

	    /* Offload Helper should not use tx_post_mgmt_arr_base_addr */
	    init_cfg->tx_post_mgmt_arr_base_addr = (void *)NULL;
	    init_cfg->tx_post_mgmt_arr_base_phys_addr = 0;
	    /* SASHAP: Do we need a call into helper rdpa? */
	}

	if (frm->alloc_mode == DOL_FR_ALLOC_IDMA) {
	    frm->flow_ids_map =
	        dhd_idma_flowmgr_fini(dhdp, frm->flow_ids_map);
	} else if (frm->alloc_mode == DOL_FR_ALLOC_LEGACY) {
	    frm->flow_ids_map =
	        id16_map_fini(dhdp->osh, frm->flow_ids_map);
	} else {
	    frm->flow_ids_map = id16_map_fini(dhdp->osh, frm->flow_ids_map);

	    for (ac = wme_ac_bk; ac <= wme_ac_max; ac++) {
	        frm->hw_id16_map[ac] =
	            id16_map_fini(dhdp->osh, frm->hw_id16_map[ac]);
	        frm->sw_id16_map[ac] =
	            id16_map_fini(dhdp->osh, frm->sw_id16_map[ac]);
	    }
	}

done:
	return NULL;
}

/**
 * Flow ring manager: allocate a flow-ring id
 */
uint16
dhd_offload_flowmgr_alloc(void *mgr, int ifidx, int prio, uint8 *mac,
	bool d11ac, bool *is_hw_ring)
{
	dol_frm_t *frm = (dol_frm_t *)mgr;
	dhd_pub_t *dhdp;
	uint16 staid = ID16_INVALID;
	uint16 ringid = ID16_INVALID;

	/* Get the dhd helper object from the flowmgr address */
	ASSERT(mgr != NULL);
	ASSERT(frm->dhdol != NULL);
	ASSERT(frm->dhdol->dhd != NULL);

	dhdp = frm->dhdol->dhd;

	if (frm->alloc_mode == DOL_FR_ALLOC_IDMA) {
	    ringid = dhd_idma_flowmgr_alloc(frm->flow_ids_map, mac);
	    *is_hw_ring = (DOL_TXO_ENABLED(frm->dhdol)) ? TRUE : FALSE;
	} else if (frm->alloc_mode == DOL_FR_ALLOC_LEGACY) {
	    ringid = id16_map_alloc(frm->flow_ids_map);
	    *is_hw_ring = (DOL_TXO_ENABLED(frm->dhdol)) ? TRUE : FALSE;
	} else {
	    staid = dhd_get_sta_cnt(dhdp, ifidx, mac);

	    ringid = frm->select_fn(frm, ifidx, prio, mac, staid, d11ac,
	        is_hw_ring);
	}

	/* update flags in flowring cache for offloaded ring */
	if ((ringid != ID16_INVALID) && (*is_hw_ring == TRUE)) {
	    hlpr_fr_cache_t *hlpr_cache;
	    uint16 flags;

	    hlpr_cache = frm->hlpr_fr_cache + ringid;
	    flags = ntohs(hlpr_cache->flags);
	    if (flags & DOL_FR_IN_HLPR_FLAG) {
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
	                !((frm->alloc_mode == DOL_FR_ALLOC_WMEAC) &&
	                (DOL_IF_ROLE_STA_OR_WDS(dhdp, ifidx))))
	                wme_ac = wme_ac_bcmc;
	            else
	                wme_ac = WME_PRIO2AC(prio);
	            flags &= ~DOL_FR_WME_AC_MASK;
	            flags |= (wme_ac << DOL_FR_WME_AC_SHIFT);
	            hlpr_cache->items = htons(frm->profile->items[wme_ac]);
	        }
	        hlpr_cache->flags = htons(flags);
	        ARCH_FLUSH_COHERENT((void *)hlpr_cache, sizeof(hlpr_fr_cache_t));
	    }
	}

	return ringid;
}

/**
 * Flow ring manager: free already allocated flow-ring id
 */
int
dhd_offload_flowmgr_free(void *mgr, uint16 flow_id)
{
	dol_frm_t *frm = (dol_frm_t *)mgr;

	if (frm->alloc_mode == DOL_FR_ALLOC_IDMA) {
	    dhd_idma_flowmgr_free(frm->flow_ids_map, flow_id);
	} else if (frm->alloc_mode == DOL_FR_ALLOC_LEGACY) {
	    id16_map_free(frm->flow_ids_map, flow_id);
	} else {
	    int wme_ac;
	    void * id_map;
	    hlpr_fr_cache_t *hlpr_cache;
	    uint16 flags;

	    hlpr_cache = frm->hlpr_fr_cache + flow_id;
	    flags = ntohs(hlpr_cache->flags);
	    wme_ac = (flags & DOL_FR_WME_AC_MASK)
	          >> DOL_FR_WME_AC_SHIFT;
	    id_map = (flags & DOL_FR_IN_HLPR_FLAG) ?
	        frm->hw_id16_map[wme_ac] : frm->sw_id16_map[wme_ac];

	    DOL_TRACE("Free %s Flowid %d map = 0x%px, ac=%d\n",
	        (flags & DOL_FR_IN_HLPR_FLAG) ? "HW" : "SW",
	        flow_id, id_map, wme_ac);
	    DOLPR("Free %s Flowid %d map = 0x%px, ac=%d\n",
	        (flags & DOL_FR_IN_HLPR_FLAG) ? "HW" : "SW",
	        flow_id, id_map, wme_ac);

	    id16_map_free(id_map, flow_id);
	}

	return BCME_OK;
}

/**
 * Iovar: Process offload related IOVAR's
 */
int
dhd_offload_do_iov(struct dhd_offload_obj *dhdol, dhd_offload_iov_t iov,
	bool set, char *buf, int buflen)
{
	/* Sanity Check */
	if (iov >= DHD_OFFLOAD_IOV_MAX) {
	    return BCME_UNSUPPORTED;
	}

	ASSERT(dhdol != NULL);
	/* Use the last index for the set in the array */
	if (set == true)
		iov = DHD_OFFLOAD_IOV_MAX;

	return dhdol_iov_fn_tbl[iov](dhdol, buf, buflen);
}

/**
 * Datapath: processes packets from the Offload Helper to DHD cpu queue from
 * DHD context
 */
bool
dhd_offload_process_cpu_queue(struct dhd_offload_obj *dhdol, bool resched)
{

	ASSERT(dhdol != NULL);

	if ((dhdol->sw_feat.cpuqdpc == TRUE) && (dhdol->ipend == TRUE)) {
	    resched = dol_dp_process_cpuq(dhdol, resched);
	}

	return resched;
}
