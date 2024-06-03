/*
   Copyright (c) 2022 Broadcom
   All Rights Reserved

   <:label-BRCM:2022:DUAL/GPL:standard

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

#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/bitops.h>

/* Linux PCI */
#include <linux/pci.h>
#include "../drivers/pci/pci.h"

/* Broadcom PCIe */
#include <pcie_hcd.h>
#include <bcm_pcie.h>

/*
 * +-----------------------------------------------------
 *  Defines
 * +-----------------------------------------------------
 */

/*
 * ***************************************************************************|
 *                 UBUS                   |              PCIE                 |
 * ***************************************************************************|
 * Win | Size |   Remap     |     Base    | Core | Size |   PCIe BAR          |
 * ***************************************************************************|
 * 0   | 16M  | 0x00C00000  | 0xBCA00000  |  0   | 16M  |  0xBC_A000_0000     |
 * ---------------------------------------|-------------|                     |
 *     |      |             |             |  1   | 16M  |                     |
 * ---------------------------------------|-------------|                     |
 * 2   | 16M  | 0xB00000    | 0xBCA20000  |  2   | 16M  |                     |
 * ---------------------------------------|-------------|                     |
 * 3   |      |             |             |  3   | 16M  |                     |
 * -----------------------------------------------------|                     |
 * 1   |      |             |             | RNR  | 64M  |                     |
 *     |      |             |             |      |      |                     |
 *     | 1M   | 0x82D000    | 0xBCA6D000  |      |      |                     |
 *     |      |             |             |      |      |                     |
 *     |      |             |             |      |      |                     |
 *     |      |             |             |      |      |                     |
 *     |      |             |             |      |      |                     |
 *     |      |             |             |      |      |  0xBC_A7FF_FFFF     |
 * ***************************************************************************|
 */

/* Address mapping entities */
#define SAWM_ID_PCIE0                  0
#define SAWM_ID_PCIE1                  1
#define SAWM_ID_PCIE2                  2
#define SAWM_ID_PCIE3                  3
#define SAWM_ID_RNR                    4
#define SAWM_ID_MAX                    5
#define SAWM_ID_INV                    (-1)

/* System Registers address range */
#define SAWM_REG_ADDR_START            0x0080000000
#define SAWM_REG_ADDR_END              0x008FFFFFFF

/* System PCIe memory windows address range */
#define SAWM_MEM_ADDR_START            0x0090000000
#define SAWM_MEM_ADDR_END              0x00EFFFFFFF

/* System address range for mapping */
#define SAWM_SYS_ADDR_START            SAWM_REG_ADDR_START
#define SAWM_SYS_ADDR_END              SAWM_MEM_ADDR_END

/* mapped BAR address window */
#define SAWM_MAP_ADDR_START            0xBCA0000000
#define SAWM_MAP_SIZE_CORE             SZ_16M
#define SAWM_MAP_SIZE_RNR              SZ_64M
#define SAWM_MAP_SIZE_TOTAL            SZ_128M
#define SAWM_MAP_ADDR_END              \
	(SAWM_MAP_ADDR_START + SAWM_MAP_SIZE_TOTAL - 1)

/* UBUS address sub-windows */
#define SAWM_SUB_WIN_SIZE_CORE         SZ_16M
#define SAWM_SUB_WIN_SIZE_RNR          SZ_1M

#define SWAM_MAX_CORES                 MAX_NUM_PHY_PCIE_CORES

/*
 * +-----------------------------------------------------
 *  Macros
 * +-----------------------------------------------------
 */
#define SAWM_REG_ADDR_RANGE(addr)     \
	(((addr) >= SAWM_REG_ADDR_START) && ((addr) <= SAWM_REG_ADDR_END))

#define SAWM_MEM_ADDR_RANGE(addr)     \
	(((addr) >= SAWM_MEM_ADDR_START) && ((addr) <= SAWM_MEM_ADDR_END))

#define SAWM_SYS_ADDR_RANGE(addr)     \
	(((addr) >= SAWM_SYS_ADDR_START) && ((addr) <= SAWM_SYS_ADDR_END))

#define SAWM_MAP_ADDR_RANGE(addr)     \
	(((addr) >= SAWM_MAP_ADDR_START) && ((addr) <= SAWM_MAP_ADDR_END))

#define SAWM_ID_MAP_ADDR(id)          \
	(SAWM_MAP_ADDR_START + ((id) * SAWM_MAP_SIZE_CORE))

#define SAWM_MAP_ADDR_ID(addr)          \
	(((addr) - SAWM_MAP_ADDR_START) / SAWM_MAP_SIZE_CORE)

#define SAWM_ID_MAP_SIZE(id)          \
	(((id) == SAWM_ID_RNR) ? SAWM_MAP_SIZE_RNR : SAWM_MAP_SIZE_CORE)

#define SAWM_ID_SUB_WIN_SIZE(id)      \
	(((id) == SAWM_ID_RNR) ? SAWM_SUB_WIN_SIZE_RNR : SAWM_SUB_WIN_SIZE_CORE)

#define SAWM_ID_TO_SUB_WIN(phc, id)   \
	((id) >= SAWM_ID_RNR) ? phc->info.id : (id)

#define SAWM_SUB_WIN_TO_ID(subwin)    (subwin)
/*
 * +-----------------------------------------------------
 *  Structures
 * +-----------------------------------------------------
 */
/* UBUS Sub Window Map */
typedef struct bpcie_swm {
	phys_addr_t sys_addr;   /* UBUS Sub-Window system address */
	phys_addr_t pci_addr;   /* UBUS incoming address from PCIe */
	uint32      size;       /* UBUS Sub-Window size */
} bpcie_swm_t;

/* PCIe Sub Window Address Manager */
typedef struct bpcie_sawm {
	struct bpcie_swm swm[SAWM_ID_MAX];  /* UBUS Mapped Address windows */
	int    num_swms[SWAM_MAX_CORES];    /* Number of active sub-windows */
} bpcie_sawm_t;

/* bcm_pcie control block */
typedef struct bpcie {
	struct bpcie_sawm swa_mgr; /* address window manager */
} bpcie_t;

/*
 * +-----------------------------------------------------
 *  Local Function prototype
 * +-----------------------------------------------------
 */
static int bcm_pcie_unconfig_bar_addr(struct pcie_hc_core *phc, int bar,
	phys_addr_t addr);

/*
 * +-----------------------------------------------------
 *  external Function prototype
 * +-----------------------------------------------------
 */
extern void bcm963xx_hc_config_bar(struct pcie_hc_core *phc, int num,
	uint32 base_hi, uint32 base_lo, uint32 sizekb, int icm);

extern void bcm963xx_hc_get_bar(struct pcie_hc_core *phc, int num,
	uint32 *base_hi, uint32 *base_lo, uint32 *sizekb);

extern int ubus_master_decode_pcie_wnd_cfg(int core, int win,
	phys_addr_t sys_addr, phys_addr_t remap_addr, unsigned int size);


/*
 * +-----------------------------------------------------
 *  Global variables
 * +-----------------------------------------------------
 */
struct bpcie bpcie_cb = { 0 };

/*
 * +-----------------------------------------------------
 *  Local inline functions
 * +-----------------------------------------------------
 */
static inline int bpcie_sys_addr_swm_id(struct pcie_hc_core *phc,
	phys_addr_t addr)
{
	int map_id = SAWM_ID_INV;

	if (SAWM_REG_ADDR_RANGE(addr)) {
	    map_id = SAWM_ID_RNR;
	} else if (SAWM_MEM_ADDR_RANGE(addr)) {
	    map_id = phc->info.id;
	}

	return map_id;
}

static inline int bpcie_addr_in_bar_range(struct pcie_hc_core *phc, int bar,
	uint32 addr_hi, uint32 addr_lo, uint32 sizekb)
{
	uint32 bar_lo, bar_hi, bar_sizekb;
	int rc = -EBUSY;

	bcm963xx_hc_get_bar(phc, bar, &bar_hi, &bar_lo, &bar_sizekb);
	if (bar_sizekb == 0) {
	    /* Free */
	    rc = 0;
	} else {
	    uint32 bar_end = bar_lo + bar_sizekb * SZ_1K - 1;
	    uint32 addr_end = addr_lo + sizekb * SZ_1K - 1;

	    if ((bar_hi == addr_hi) && (bar_lo <= addr_lo) &&
	        (bar_end >= addr_end)) {
	        /* An existing window will work */
	        rc = 1;
	    }
	}

	return rc;
}

/*
 * +-----------------------------------------------------
 *  Local Functions
 * +-----------------------------------------------------
 */
/*
 * Function bcm_pcie_unconfig_bar_addr ()
 *
 *   Parameters:
 *    phc:   pci host controller handle
 *    bar:   pci BAR register to use
 *            0: Search for existing BAR
 *    addr:  BAR incoming physical address
 *
 *   Description:
 *    unconfigure the PCIe incoming address window.
 *
 *   Return:
 *     0 on success,
 *     1 on success with address mapped
 *   -ve on failure
 */
static int bcm_pcie_unconfig_bar_addr(struct pcie_hc_core *phc, int bar,
	phys_addr_t addr)
{
	uint32 addr_lo, addr_hi, sizekb;
	int max_bars;
	int rc;

	HCD_INFO("bpcie: >>%s(0x%px, %d, 0x%pa)\n", __FUNCTION__, phc, bar, &addr);

	/* Get number of bars supported by the host controller */
	max_bars = hweight8(phc->cfg.barmask);

	/* Prepare parameters for configuration */
	addr_hi = ((u64)addr) >> 32;
	addr_lo = (uint32)(addr);
	sizekb = SZ_64K/SZ_1K; /* Use minimum Size */

	/* Check if BAR space already configured ? */
	/* Get first available BAR */
	if (bar == 0) {

	    while (bar++ < max_bars) {
	        rc = bpcie_addr_in_bar_range(phc, bar, addr_hi, addr_lo, sizekb);
	        if (rc == 1) {
	            break;
	        }
	    }
	} else {
	    rc = bpcie_addr_in_bar_range(phc, bar, addr_hi, addr_lo, sizekb);
	}

	/* Check the search status */
	if (rc != 1) {
	    /* no existing BAR for the given address */
	    HCD_ERROR("bpcie%d: No existing BAR's available\n", phc->info.id);
	    return -EINVAL;
	}

#if defined(CONFIG_BCM_PCIE_SWAM)
	if (SAWM_MAP_ADDR_RANGE(addr)) {
	    struct bpcie_swm *swm;
	    int map_id = SAWM_MAP_ADDR_ID(addr);
	    int rc;

	    /* Remove the UBUS decode window */
	    if (map_id > SAWM_ID_RNR) map_id = SAWM_ID_RNR;
	    swm = &bpcie_cb.swa_mgr.swm[map_id];
	    rc = ubus_master_decode_pcie_wnd_cfg(phc->info.id, map_id,
	        0x0, 0x0, 0x0);

	    HCD_LOG("bpcie%d: UBUS SWAM [%d] Delete PCI " HCD_ADDR_SIZE_LOG_FMT
	        " System Addr: 0x%llx\n",
	        phc->info.id, map_id, HCD_ADDR_SIZE_LOG(swm->pci_addr, swm->size),
	        swm->sys_addr);

	    bpcie_cb.swa_mgr.num_swms[phc->info.id]--;

	    if (bpcie_cb.swa_mgr.num_swms[phc->info.id]) {
	        /* There are still active sub-windows in use */
	        return 0;
	    }

	    /* Update the iwin address and size for Logging */
	    addr_lo = (SAWM_MAP_ADDR_START & 0xFFFFFFFF);
	    addr_hi = ((SAWM_MAP_ADDR_START >> 32) & 0xFFFFFFFF);
	    sizekb = (SAWM_MAP_SIZE_TOTAL / SZ_1K);

	    HCD_INFO("bpcie%d: updated BAR%d addr: 0x%x_%x, size 0x%x\n",
	        phc->info.id, bar, addr_hi, addr_lo, sizekb * SZ_1K);
	}
#endif /* CONFIG_BCM_PCIE_SWAM */

	/* This BAR is configured, un-configure it */
	HCD_INFO("bpcie%d: un-configuring BAR%d addr: 0x%x_%x, size 0x%x\n",
	    phc->info.id, bar, addr_hi, addr_lo, sizekb * SZ_1K);

	bcm963xx_hc_config_bar(phc, bar, 0x0, 0x0, 0x0, 0x0);

	return 0;
}

/*
 * +-----------------------------------------------------
 *  Global Functions
 * +-----------------------------------------------------
 */

/*
 * +-----------------------------------------------------
 *  External exported Functions
 * +-----------------------------------------------------
 */
/*
 * Function bcm_pcie_config_bar_addr ()
 *
 *   Parameters:
 *    pdev:  pci device handle
 *    bar:   pci BAR register to use
 *            0: Search for available/existing BAR
 *    addr:  BAR incoming physical address
 *    size: incoming address size
 *            0: will unconfig the BAR
 *
 *   Description:
 *    External interface to configure/unconfigure the PCIe incoming address
 *    window.
 *
 *   Return:
 *     0 on success,
 *     1 on success with address mapped
 *   -ve on failure
 */
int bcm_pcie_config_bar_addr(struct pci_dev *pdev, int bar, phys_addr_t addr,
	u32 size)
{
	uint32 addr_lo, addr_hi, sizekb;
	struct pcie_hc_core *phc;
	int max_bars;
	int free_bar = 0;
	int existing_bar = 0;
	int icm = 0;
	int rc;

	HCD_INFO("bpcie: >>%s(0x%px, %d, 0x%pa, 0x%x)\n", __FUNCTION__, pdev, bar,
	    &addr, size);

	/* Sanity check */
	if (!pdev) {
	    HCD_ERROR("bpcie: Invalid pdev\n");
	    return -EINVAL;
	}

	/* Get the RC device handle */
	pdev = pcie_find_root_port(pdev);
	if (!pdev) {
	    HCD_ERROR("bpcie: Unable to get root pdev\n");
	    return -EINVAL;
	}

	/* Get the PCIe host core handle */
	phc = (struct pcie_hc_core*)(pdev->bus->sysdata);

	/* size=0 is for unconfig the BAR */
	if (size == 0) {
	    return bcm_pcie_unconfig_bar_addr(phc, bar, addr);
	}

	/* Make the size aligned to the nearest power of 2 */
	size = __roundup_pow_of_two(size);

	/* Get number of bars supported by the host controller */
	max_bars = hweight8(phc->cfg.barmask);

	/* Prepare parameters for configuration */
	addr_hi = ((u64)addr) >> 32;
	addr_lo = (uint32)(addr);
	sizekb = size/SZ_1K; /* Convert to 1K size number */

	/* Check if BAR space already configured ? */
	/* Get first available BAR */
	if (bar == 0) {

	    while (bar++ < max_bars) {
	        rc = bpcie_addr_in_bar_range(phc, bar, addr_hi, addr_lo, sizekb);
	        if (rc == 0) {
	            /* This BAR is not configured */
	            if (!free_bar)
	                free_bar = bar;
	            continue;
	        } else if (rc == 1) {
	            /* An existing window will work */
	            HCD_INFO("bpcie%d: Found existing BAR%d\n", phc->info.id, bar);
	            existing_bar = bar;
	            break;
	        }
	    }
	} else {
	    rc = bpcie_addr_in_bar_range(phc, bar, addr_hi, addr_lo, sizekb);
	    if (rc < 0) {
	        HCD_INFO("bpcie%d: BAR%d already in use\n", phc->info.id, bar);
	        return rc;
	    } else if (rc == 1) {
	        /* An existing window will work */
	        HCD_INFO("bpcie%d: BAR%d already configured \n", phc->info.id, bar);
	        existing_bar = bar;
	    }
	}

	HCD_INFO("bpcie%d: bar [%d], Existing [%d], free [%d], max [%d]\n",
	    phc->info.id, bar, existing_bar, free_bar, max_bars);

	/* Check the search status */
	if (existing_bar) {
	    /* Existing BAR for this address, use it */
	    bar = existing_bar;
	} else if (free_bar) {
	    /* no existing and a free bar available, use it */
	    bar = free_bar;
	} else if (bar > max_bars) {
	    /* Used up all BAR's, No space available */
	    HCD_ERROR("bpcie%d: No free BAR's available\n", phc->info.id);
	    return -ENOSPC;
	}

#if defined(CONFIG_BCM_PCIE_SWAM)
	if (SAWM_MAP_ADDR_RANGE(addr)) {
	    struct bpcie_swm *swm;
	    int map_id = SAWM_MAP_ADDR_ID(addr);
	    int rc;

	    /* Update the UBUS decode window */
	    if (map_id > SAWM_ID_RNR) map_id = SAWM_ID_RNR;
	    else icm = 1;
	    swm = &bpcie_cb.swa_mgr.swm[map_id];
	    rc = ubus_master_decode_pcie_wnd_cfg(phc->info.id, map_id,
	        swm->pci_addr, swm->sys_addr, swm->size);

	    HCD_LOG("bpcie%d: UBUS SWAM [%d] Add PCI " HCD_ADDR_SIZE_LOG_FMT
	        " System Addr: 0x%llx\n",
	        phc->info.id, map_id, HCD_ADDR_SIZE_LOG(swm->pci_addr, swm->size),
	        swm->sys_addr);

	    /* Update the iwin address and size if need to be configured */
	    addr_lo = (SAWM_MAP_ADDR_START & 0xFFFFFFFF);
	    addr_hi = ((SAWM_MAP_ADDR_START >> 32) & 0xFFFFFFFF);
	    sizekb = (SAWM_MAP_SIZE_TOTAL / SZ_1K);

	    HCD_INFO("bpcie%d: updated BAR%d addr: 0x%x_%x, size 0x%x\n",
	        phc->info.id, bar, addr_hi, addr_lo, sizekb * SZ_1K);

	    bpcie_cb.swa_mgr.num_swms[phc->info.id]++;
	}
#else /* !CONFIG_BCM_PCIE_SWAM */
	if (SAWM_MEM_ADDR_RANGE(addr)) {
	    /* This is used for inter pcie messaging */
	    icm = 1;
	}
#endif /* CONFIG_BCM_PCIE_SWAM */

	if ((existing_bar) && (icm == 0)) return 0;

	/* This BAR is not configured, configure it */
	HCD_INFO("bpcie%d: Configuring BAR%d addr: 0x%x_%x, size 0x%x\n",
	    phc->info.id, bar, addr_hi, addr_lo, sizekb * SZ_1K);

	bcm963xx_hc_config_bar(phc, bar, addr_hi, addr_lo, sizekb, icm);

	return 0;
}
EXPORT_SYMBOL(bcm_pcie_config_bar_addr);

/*
 * Function bcm_pcie_map_bar_addr ()
 *
 *   Parameters:
 *    pdev:  pci device handle
 *    addr:  bar2 phsical address to be mapped
 *    size:  map size
 *
 *   Description:
 *    Map BAR incoming address to UBUS decodable mapped address
 *
 *   Return:
 *     Mapped physical address on success
 *     "addr" on no support
 *     -1 ull on failure
 */
phys_addr_t bcm_pcie_map_bar_addr(struct pci_dev *pdev, phys_addr_t addr,
	u32 size)
{
	phys_addr_t pci_addr = BCM_PCIE_MAP_ADDR_INVALID;

#if defined(CONFIG_BCM_PCIE_SWAM)
	struct pcie_hc_core *phc;
	struct bpcie_swm    *swm;
	int                 map_id;
	phys_addr_t         sys_addr;
	u32                 map_size;

	HCD_INFO("bpcie: >>%s(0x%px, 0x%llx, 0x%x)\n", __FUNCTION__, pdev, addr,
	    size);

	/* Sanity Check input parameters */
	if (!pdev) {
	    HCD_ERROR("bpcie: Null pdev");
	    goto done;
	}

	/* Get the RC device handle */
	pdev = pcie_find_root_port(pdev);
	if (!pdev) {
	    HCD_ERROR("bpcie: Unable to find roort port for pdev 0x%px\n", pdev);
	    goto done;
	}

	/* Get the controller handle */
	phc = (struct pcie_hc_core*)(pdev->bus->sysdata);
	if (!phc) {
	    HCD_ERROR("bpcie: Unable to find controller handle for pdev 0x%px\n",
	        pdev);
	    goto done;
	}

	/* Get the sub-window id */
	map_id = bpcie_sys_addr_swm_id(phc, addr);
	if (map_id == SAWM_ID_INV) {
	    HCD_ERROR("bpcie: Unable to find map_id for phc 0x%px\n", phc);
	    goto done;
	}

	HCD_INFO("bpcie%d: addr [0x%llx], map_id [%d]\n", phc->info.id, addr,
	    map_id);

	/* get the sub-window handle */
	swm = &bpcie_cb.swa_mgr.swm[map_id];
	/* size=0, use default size */
	if (size == 0) {
	    size = SAWM_ID_SUB_WIN_SIZE(map_id);
	    HCD_INFO("bpcie%d: auto size updated size size to [0x%x]\n",
	        phc->info.id, size);
	}

	/* Make the size aligned to the nearest power of 2 */
	size = __roundup_pow_of_two(size);

	if ((size > SAWM_ID_MAP_SIZE(map_id)) ||
	    (size > SAWM_ID_SUB_WIN_SIZE(map_id))) {
	    HCD_ERROR("bpcie%d: size - size [%d] > supported [%d], subwin [%d]\n",
	        phc->info.id, size, SAWM_ID_MAP_SIZE(map_id),
	        SAWM_ID_SUB_WIN_SIZE(map_id));
	    goto done;
	}

	/* PCIe BAR specific */
	map_size = SAWM_ID_MAP_SIZE(map_id);
	pci_addr = SAWM_ID_MAP_ADDR(map_id);
	sys_addr = (addr & ~(size - 1));

	/* Check if size size is less than PCIe bar map size */
	if (size < map_size) {
	    pci_addr |= (sys_addr & (map_size - 1));
	}

	/* Check if previous provisioned address is same as this one */
	if ((swm->pci_addr && (swm->pci_addr != pci_addr)) ||
	    (swm->sys_addr && (swm->sys_addr != sys_addr))) {
	    HCD_ERROR("bpcie%d: Previous base [0x%llx] map [0x%llx] "
	        "differ from current base [0x%llx], map [0x%llx]\n", phc->info.id,
	        swm->sys_addr, swm->pci_addr, sys_addr, pci_addr);
	} else {
	    /* Update UBUS sub-window information */
	    swm->sys_addr = sys_addr;
	    swm->pci_addr = pci_addr;
	    swm->size = size;
	    HCD_INFO("bpcie%d: map[%d]: base [0x%llx], map [0x%llx], size [0x%x]\n",
	        phc->info.id, map_id, sys_addr, pci_addr, size);
	}

	pci_addr |= (addr & (size - 1));

done:

#else /* !CONFIG_BCM_PCIE_SWAM */
	HCD_INFO("bpcie: >> %s(0x%px, 0x%pa, 0x%x)\n", __FUNCTION__, pdev, &addr,
	    size);
	pci_addr = addr;
#endif /* CONFIG_BCM_PCIE_SWAM */

	return pci_addr;
}
EXPORT_SYMBOL(bcm_pcie_map_bar_addr);
