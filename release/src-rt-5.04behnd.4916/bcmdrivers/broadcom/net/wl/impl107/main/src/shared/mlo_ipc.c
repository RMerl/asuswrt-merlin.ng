/*
 * +--------------------------------------------------------------------------+
 * MLO_IPC: Inter Processor communication between MLO AP's
 *
 * Copyright (C) 2025, Broadcom. All Rights Reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: mlo_ipc.c 854225 2025-04-07 03:17:46Z $
 * +--------------------------------------------------------------------------+
 */

#include <typedefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <bcmpcie.h>
#include <mlo_ipc.h>
#if defined(BCM_BTRACE)
#include <bcm_btrace.h>
#endif /* BCM_BTRACE */

#include <linuxver.h>
#include <linux/dma-mapping.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0)
#include <linux/dma-contiguous.h>
#else
#include <linux/dma-map-ops.h>
#endif /* KERNEL < 5.10 */
#if defined(MLO_IPC_PROC)
#include <linux/proc_fs.h>
#endif /* MLO_IPC_PROC */

#define MLO_IPC_WAIT_TIMEOUT		5000 /* ms */

static struct device *mlo_ipc_dev_alloc(void);
static void mlo_ipc_dev_free(struct device *mlo_ipc_dev);

#if defined(MLO_IPC_PROC)
static struct proc_dir_entry *mlo_ipc_proc_create(void);
static void mlo_ipc_proc_destroy(struct proc_dir_entry *proc_dir);
#endif /* MLO_IPC_PROC */

// Global instance of MLO IPC context
mlo_ipc_sys_t * mlo_ipc_sys_gp = NULL;
const char * __mlo_ipc_mlc_state_str[mlo_ipc_mlc_max_e] =
{
	"MLC_RESET", "MLC_ATTACH", "MLC_BIND", "MLC_LINK", "MLC_BRIDGE",
	"MLC_SYNC", "MLC_READY", "MLC_TRAP", "MLC_HALT", "MLC_SUSPEND",
	"MLC_SUSPEND_HALT", "MLC_DOWN_REQUEST", "MLC_DOWN", "MLC_DOWN_COMPLETE",
	"MLC_BH_REQUEST", "MLC_BH_HALT_REQUEST", "MLC_BH", "MLC_BH_COMPLETE",
	"MLC_DOWN_BH_REQUEST", "MLC_DOWN_BH", "MLC_DOWN_BH_COMPLETE",
	"MLC_UP_REQUEST",  "MLC_UP", "MLC_ACTIVE"
};

// #define MLO_IPC_DEBUG_BUILD

#define MLO_IPC_ERROR(args)		printk args
#if defined(MLO_IPC_DEBUG_BUILD)
#define MLO_IPC_DEBUG(args)		printk args
#else
#define MLO_IPC_DEBUG(args)
#endif /* ! MLO_IPC_DEBUG_BUILD */

#define PCIE_IPC_MLO_SIZE		(68 * 1024) /* 4KB for mutex + 64KB for PUQ */

struct device *mlo_ipc_dev_gp = NULL;

/**
 * +--------------------------------------------------------------------------+
 * NVRAM "wl_mlo_config"
 * +--------------------------------------------------------------------------+
 *
 * MLO Logical AP ID assignment Notation using an array.
 * Maximum of 4 PCIe slots are covered by the nvram. The array is indexed by
 * AP unit number 0 to 3.
 *
 * mlo_unit:   logical AP ID assigned to WLAN AP in probed PCIe slot
 *		mlo_unit =  0, implies WLAN AP takes on the MAP role
 *		mlo_unit = -1, implies WLAN AP (if exists) does NOT participate in MLO
 *
 * E.g. nvram kset wl_mlo_config="1 2 0 -1"
 *		wl0 is link 1  Aux AP (AAP)
 *		wl1 is link 2  Aux AP (AAP)
 *		wl2 is link 0  Main AP (MAP)
 *		In quad-band, wl3 is not participating in MLO, only 3 radios may participate in MLO
 * Note: in a mixed Full-Dongle (FD) and NIC interfaces, the FD radio should always be the MAP
 *
 * Default configuration: WL_MLO_DEFAULT
 * There are no MLO AP's
 * +--------------------------------------------------------------------------+
 */

#define WL_MLO_DEFAULT      "-1 -1 -1 -1"
#define WL_MLO_VERSION      (PCIE_IPC_MLO_VERSION)

#define WL_MAX_RADIO		(PCIE_IPC_AP_UNIT_MAX)
#define WL_MLO_MAX_RADIO	(WL_MAX_RADIO - 1)
#define WL_MLO_MIN_UNIT		(PCIE_IPC_MLO_UNIT_INV)
#define WL_MLO_MAX_UNIT		(WL_MLO_MAX_RADIO - 1)
#define WL_MLO_DUAL_AP_MAP_UNIT		(0)
#define WL_MLO_DUAL_AP_AAP_UNIT		(1)

extern char *wl_mlo_config;

/*
* Must have 4 values:
*  Either all 4 values are -1 (i.e. NonMLO), or the following rules apply
*  Must have at least a 0 and a 1 in the 4 values,
*	i.e. at the minimum must be a dual AP MLO.
*  Cannot have a value 1 without a value 0 (above rule #2 covers this)
*  Cannot have a value 2 without a value 1
*  Cannot have a value 3, or higher (only 3 radios may participate in MLO)
*  May have more than one value -1
*  Cannot have a repetition of any non -1 value
*/

static bool
_is_mlo_config_validated(char *mlo_conf_str, int *mlo_config, int *mlo_total_p)
{
	uint8 unit = 0, mlo_unit_bmap = 0, mlo_units = 0;
	int dummy;
	bool isMAP = FALSE, isAAP = FALSE;
	bool inited = (mlo_ipc_get_dev() != NULL) ? TRUE : FALSE;
	uint8 opmode, opmode_nic_units = 0, opmode_fd_units = 0;
	uint8 map_opmode = PCIE_IPC_MLO_OPMODE_INV;
	mlo_ipc_sys_t *mlo_ipc_sys = mlo_ipc_sys_gp;
	uint8 mlo_disable_ap_bm = 0;

	/* Must have 4 values */
	if (sscanf(mlo_conf_str, "%d %d %d %d %d",
		&mlo_config[0], &mlo_config[1], &mlo_config[2], &mlo_config[3],
		&dummy) != WL_MAX_RADIO) {

		MLO_IPC_ERROR(("%s: mlo_config [%s] must have 4 values\n",
			__FUNCTION__, mlo_conf_str));
		return FALSE;
	}

	/* Cannot have a repetition of any non -1 value */
	for (unit = 0; unit < WL_MAX_RADIO; unit++) {

		 /* Cannot have a value 3, or higher */
		if (!TEST_IN_RANGE(mlo_config[unit], WL_MLO_MIN_UNIT,
			WL_MLO_MAX_UNIT)) {

			MLO_IPC_ERROR(("%s: Must have a value in range from %d to %d, "
				"mlo_config[%s]\n", __FUNCTION__, WL_MLO_MIN_UNIT,
				WL_MLO_MAX_UNIT, mlo_conf_str));
			return FALSE;
		}

		/* -1 can be repeated */
		if (mlo_config[unit] == WL_MLO_MIN_UNIT) {
			continue;
		}

		/*
		* Must have at least a 0 and a 1 in the 4 values,
		*  i.e. at the minimum must be a dual AP MLO.
		*/

		/* Check MAP */
		if (mlo_config[unit] == WL_MLO_DUAL_AP_MAP_UNIT) {
			isMAP = TRUE;
		}

		/* Check AAP1 */
		if (mlo_config[unit] == WL_MLO_DUAL_AP_AAP_UNIT) {
			isAAP = TRUE;
		}

		if (mlo_unit_bmap & (1 << mlo_config[unit])) {
			MLO_IPC_ERROR(("%s: Cannot have a repetition of any non -1 value, "
				"mlo_config [%s]\n", __FUNCTION__, mlo_conf_str));
			return FALSE;
		}

		if (inited) {
			ASSERT(mlo_ipc_sys != NULL);
			ASSERT(mlo_ipc_sys->mlo_eligible[unit] == TRUE);
			opmode = mlo_ipc_get_ap_op_mode(unit);
			if (opmode == PCIE_IPC_MLO_OPMODE_NIC) {
				opmode_nic_units++;
			} else if (opmode == PCIE_IPC_MLO_OPMODE_FD) {
				opmode_fd_units++;
			} else {
				MLO_IPC_ERROR(("%s: unit[%d] is not MLO capable, "
					"mlo_config [%s]\n", __FUNCTION__, unit, mlo_conf_str));
				return FALSE;
			}

			if (mlo_config[unit] == WL_MLO_DUAL_AP_MAP_UNIT) {
				map_opmode = opmode;
			}
		}

		mlo_unit_bmap |= (1 << mlo_config[unit]);
		mlo_units++;
	}

	if (mlo_units == 0) {
		if (mlo_total_p != NULL) {
			*mlo_total_p = mlo_units;
		}
		return TRUE;
	}

	if (!isMAP || !isAAP) {
		MLO_IPC_ERROR(("%s: Must have at least a 0 and a 1, "
			"mlo_config [%s]\n", __FUNCTION__, mlo_conf_str));
		return FALSE;
	}

	if (!inited) {
		if (mlo_total_p != NULL) {
			*mlo_total_p = mlo_units;
		}
		return TRUE;
	}

	/* additional checks and mlo_config fixup for reconfig requests */
	if (mlo_units > mlo_ipc_sys->pcie_ipc_mlo->sys.mlo_total) {
		MLO_IPC_ERROR(("%s: Must have less than or equal to %d MLO units, "
			"mlo_config [%s]\n", __FUNCTION__,
			mlo_ipc_sys->pcie_ipc_mlo->sys.mlo_total,
			mlo_conf_str));
		return FALSE;
	}

	if ((map_opmode == PCIE_IPC_MLO_OPMODE_NIC) && (opmode_nic_units < 2)) {
		MLO_IPC_ERROR(("%s: Must have at least 2 MLO unis operated in NIC mode, "
			"mlo_config [%s]\n", __FUNCTION__, mlo_conf_str));
		return FALSE;
	}

	/* Disable MLO for the ap units:
	 * 1. Registered mlo_ipc but set MLO unit to -1
	 * 2. Operated in FD mode but MAP is operated in NIC mode
	 */
	for (unit = 0; unit < WL_MAX_RADIO; unit++) {
		if (mlo_ipc_sys->mlo_eligible[unit] != TRUE) {
			continue;
		}

		if (mlo_config[unit] == WL_MLO_MIN_UNIT) {
			MLO_IPC_ERROR(("%s: Disable MLO for unit[%d] according to new "
				"mlo_config and set MLO unit %d\n",
				__FUNCTION__, unit, mlo_units));
			mlo_disable_ap_bm |= NBITVAL(unit);
			mlo_ipc_set_sys_mlo_down(unit, 1);
			mlo_config[unit] = mlo_units++;
		} else if ((opmode_fd_units > 0) &&
			(map_opmode == PCIE_IPC_MLO_OPMODE_NIC) &&
			(mlo_ipc_get_ap_op_mode(unit) == PCIE_IPC_MLO_OPMODE_FD)) {
			MLO_IPC_ERROR(("%s: Disable MLO for unit[%d] operated in FD mode "
				"because MAP is operated in NIC mode\n",
				__FUNCTION__, unit));
			mlo_disable_ap_bm |= NBITVAL(unit);
			mlo_ipc_set_sys_mlo_down(unit, 1);
		}
	}

	if (mlo_ipc_sys->mlo_disable_ap_bm != mlo_disable_ap_bm) {
		MLO_IPC_ERROR(("%s: mlo_disable_ap_bm changed from 0x%x to 0x%x\n",
			__FUNCTION__, mlo_ipc_sys->mlo_disable_ap_bm, mlo_disable_ap_bm));
		mlo_ipc_sys->mlo_disable_ap_bm = mlo_disable_ap_bm;
	}

	ASSERT(mlo_units == mlo_ipc_sys->pcie_ipc_mlo->sys.mlo_total);

	if (mlo_total_p != NULL) {
		*mlo_total_p = mlo_units;
	}

	return TRUE;
}

static char *
_get_wl_mlo_config_str(void)
{
	char	* wl_mlo_config_str;

#if defined(BCM_ROUTER)
	char	nvram_str[64];

	/* Override with NVRAM setting */
	snprintf(nvram_str, sizeof(nvram_str), "wl_mlo_config");
	if ((wl_mlo_config_str = getvar(NULL, nvram_str)) == NULL) {
		wl_mlo_config_str = WL_MLO_DEFAULT;
	}
#else
	wl_mlo_config_str = wl_mlo_config;
#endif /* BCM_ROUTER */

	MLO_IPC_ERROR(("WL MLO AP IDS assignment %s\n", wl_mlo_config_str));
	return wl_mlo_config_str;
} // _get_wl_mlo_config_str()

struct device *mlo_ipc_get_dev(void)
{
	return mlo_ipc_dev_gp;
}

/* Get the mlo link_id */
int mlo_ipc_get_mlo_unit(int ap_unit)
{
	mlo_ipc_sys_t   * mlo_ipc_sys;
	pcie_ipc_mlo_sys_t *sys;

	mlo_ipc_sys = mlo_ipc_sys_gp;

	if ((mlo_ipc_sys != NULL) && (mlo_ipc_sys->pcie_ipc_mlo != NULL)) {
		sys = &mlo_ipc_sys->pcie_ipc_mlo->sys;
		return sys->mlo_unit[ap_unit];
	}
	return PCIE_IPC_MLO_UNIT_INV;
}

/* Get the MLO link count */
int mlo_ipc_get_mlink_count(void)
{
	mlo_ipc_sys_t   * mlo_ipc_sys;
	pcie_ipc_mlo_sys_t *sys;
	uint8 mlo_unit, mlo_active_total = 0;

	mlo_ipc_sys = mlo_ipc_sys_gp;

	if ((mlo_ipc_sys != NULL) && (mlo_ipc_sys->pcie_ipc_mlo != NULL)) {
		sys = &mlo_ipc_sys->pcie_ipc_mlo->sys;
		for (mlo_unit = 0; mlo_unit < PCIE_IPC_MLO_UNIT_MAX; ++mlo_unit) {
			if (sys->mlo_active_bm & (1 << mlo_unit)) {
				mlo_active_total++;
			}
		}
		return mlo_active_total;
	}
	return 0;
}

/*
 * Read wl_mlo_eligible from nvram.
 * The value is either 1(MLO eligibility) or 0(non MLO eligibility) for each unit.
 */
bool
nvram_mlo_eligible_is_valid(int * mlo_eligible)
{
	char * wl_mlo_eligible_str = getvar(NULL, "wl_mlo_eligible");
	int dummy;
	uint8 unit;

	if (wl_mlo_eligible_str == NULL) {
		return FALSE;
	}

	/* Must have 4 values */
	if (sscanf(wl_mlo_eligible_str, "%d %d %d %d %d",
		&mlo_eligible[0], &mlo_eligible[1], &mlo_eligible[2], &mlo_eligible[3],
		&dummy) != WL_MAX_RADIO) {
		MLO_IPC_ERROR(("%s: mlo_eligible [%s] must have 4 values\n",
			__FUNCTION__, wl_mlo_eligible_str));
		return FALSE;
	} else {
		for (unit = 0; unit < WL_MAX_RADIO; unit++) {
			if ((mlo_eligible[unit] != 0) && (mlo_eligible[unit] != 1)) {
				MLO_IPC_ERROR(("%s: mlo_eligible[%d]=%d is invalid\n",
					__FUNCTION__, unit, mlo_eligible[unit]));
				return FALSE;
			}
		}
	}

	return TRUE;
}

int // Initialize MLO IPC context
mlo_ipc_init(void)
{
	osl_t		* osh;
	mlo_ipc_sys_t	* mlo_ipc_sys;
	pcie_ipc_mlo_t	* pcie_ipc_mlo;			// PCIE IPC for MLO:
	int	nvram_mlo[PCIE_IPC_AP_UNIT_MAX];	// parsed from nvram or module param
	int	mlo_total      = 0;
	int	ret;
	uint8	ap_unit;
	int8	ap_unit_map    = (int8)PCIE_IPC_AP_UNIT_INV;
	uint8	mlo_ap_bm = 0;
	int	nvram_mlo_eligible[PCIE_IPC_AP_UNIT_MAX];
	uint8	mlo_disable_ap_bm = 0;

	MLO_IPC_DEBUG(("%s: ENTER\n", __FUNCTION__));

	// +--- Fetch and parse nvram or default
	{
		char * wl_mlo_config_str = _get_wl_mlo_config_str();

		if (!_is_mlo_config_validated(wl_mlo_config_str, nvram_mlo, &mlo_total)) {
#if defined(BCMDBG)
			ASSERT(0);
#endif

			wl_mlo_config_str = WL_MLO_DEFAULT;
			return BCME_OK;
		}
	}

	/* Init mlo_eligible with mlo_config if nvram wl_mlo_eligible is not found or not valid */
	if (!nvram_mlo_eligible_is_valid(nvram_mlo_eligible)) {
		for (ap_unit = 0; ap_unit < PCIE_IPC_AP_UNIT_MAX; ++ap_unit) {
			/* The unit is MLO eligible if it has non -1 mlo_unit */
			if (nvram_mlo[ap_unit] != (int)PCIE_IPC_MLO_UNIT_INV) {
				nvram_mlo_eligible[ap_unit] = 1;
			} else {
				nvram_mlo_eligible[ap_unit] = 0;
			}
		}
	}

	// +--- Audit nvram: determine ap_unit_map, mlo_total
	for (ap_unit = 0; ap_unit < PCIE_IPC_AP_UNIT_MAX; ++ap_unit)
	{
		if (nvram_mlo[ap_unit] == (int)PCIE_IPC_MLO_UNIT_INV) {
			if (nvram_mlo_eligible[ap_unit] == 1) {
				mlo_disable_ap_bm |= NBITVAL(ap_unit);
				nvram_mlo[ap_unit] = mlo_total++;
			} else {
				continue; // AP does not participate in MLO and not MLO eligible
			}
		}

		if (nvram_mlo_eligible[ap_unit] == 0) {
			MLO_IPC_ERROR(("%s: nvram_mlo[%d]=%d and mlo_eligible[%d]=%d mismatched\n",
				__FUNCTION__, ap_unit, nvram_mlo[ap_unit],
				ap_unit, nvram_mlo_eligible[ap_unit]));
			ASSERT(0);
		}

		if (nvram_mlo[ap_unit] == (int)PCIE_IPC_MLO_UNIT_MAP)
		{
			ap_unit_map = (int8)ap_unit; // MAP's AP unit
		}

		mlo_ap_bm |= (1 << ap_unit);
	}

	// Continue only if MLO AP's are present
	if (mlo_total == 0) {
		return BCME_OK;
	}

	// mlo_ipc_dev_gp is a pseudo device for the purpose of DMA memory allocation
	mlo_ipc_dev_gp = mlo_ipc_dev_alloc();
	if (mlo_ipc_dev_gp == NULL) {
		return BCME_NORESOURCE;
	}

	pcie_ipc_mlo = NULL;
	// Allocate osh with a NULL pdev so that we can use the DMA_ALLOC_CONSISTENT macro
	osh = osl_attach(NULL, PCI_BUS, FALSE);

	mlo_ipc_sys = MALLOCZ(osh, sizeof(mlo_ipc_sys_t));
	if (mlo_ipc_sys == NULL) {
		MLO_IPC_ERROR(("%s: mlo_ipc_sys malloc [%u] failed\n",
			__FUNCTION__, (uint)sizeof(mlo_ipc_sys_t)));
		ret = BCME_NORESOURCE;
		goto mlo_ipc_init_fail;
	}

#if defined(BCM_DMA_MALLOC)
	dma_malloc_init(osh);
#endif /* BCM_DMA_MALLOC */

	mlo_ipc_sys->osh = osh;

	// Set Global access pointer
	mlo_ipc_sys_gp = mlo_ipc_sys;

	ASSERT(ap_unit_map != (int8)PCIE_IPC_AP_UNIT_INV);
	MLO_IPC_DEBUG(("%s: AP unit for MAP %d\n", __FUNCTION__, ap_unit_map));

	{ // Allocate PCIE IPC MLO
		/* Place PCIE_IPC_MLO into the first 4K page.
		 * The trailing DDR space is used by the LMAC for mutex and PUQ.
		 */
		pcie_ipc_mlo = (pcie_ipc_mlo_t *)DMA_ALLOC_CONSISTENT(osh,
			PCIE_IPC_MLO_SIZE,	/* 4KB for mutex + 36KB for PUQ */
			0,			/* 0 is no alignment; 12 if 4KB aligned */
			&mlo_ipc_sys->pcie_ipc_mlo_alloced, &mlo_ipc_sys->pcie_ipc_mlo_pa, 0);
		if (pcie_ipc_mlo == NULL) {
			MLO_IPC_ERROR(("%s: PCIE_IPC_MLO alloc failed\n", __FUNCTION__));
			ret = BCME_NORESOURCE;
			goto mlo_ipc_init_fail;
		}

		MLO_IPC_ERROR(("%s: PCIE_IPC_MLO req %u alloc %u bytes pa %08x-%08x\n",
			__FUNCTION__, PCIE_IPC_MLO_SIZE,
			mlo_ipc_sys->pcie_ipc_mlo_alloced,
			(uint32)PHYSADDRHI(mlo_ipc_sys->pcie_ipc_mlo_pa),
			(uint32)PHYSADDRLO(mlo_ipc_sys->pcie_ipc_mlo_pa)));
		pcie_ipc_mlo->sys.mlo_version   = WL_MLO_VERSION;
		pcie_ipc_mlo->sys.mlo_total     = mlo_total;
		pcie_ipc_mlo->sys.ap_unit_map   = ap_unit_map;
		pcie_ipc_mlo->sys.mlo_ap_bm     = mlo_ap_bm;

		for (ap_unit = 0; ap_unit < PCIE_IPC_AP_UNIT_MAX; ++ap_unit) {
			if (nvram_mlo[ap_unit] != (int)PCIE_IPC_MLO_UNIT_INV) {
				pcie_ipc_mlo->sys.mlo_active_bm |= (1 << nvram_mlo[ap_unit]);
			}
			pcie_ipc_mlo->sys.mlo_unit[ap_unit] = (int8)nvram_mlo[ap_unit];
			mlo_ipc_sys->mlo_config[ap_unit] = nvram_mlo[ap_unit];
			mlo_ipc_sys->mlo_eligible[ap_unit] = (nvram_mlo_eligible[ap_unit] == 1) ?
				TRUE : FALSE;
			if ((mlo_disable_ap_bm & NBITVAL(ap_unit)) != 0) {
				pcie_ipc_mlo->sys.mlo_down[ap_unit] = 1;
			}

			/* If WLAN radio is not place into a PCIE slot then per-AP MLO
			 * attributes will not be set. So Reset AP and MLO unit for sanity.
			 */
			pcie_ipc_mlo->ap[ap_unit].ap_unit = (uint8)PCIE_IPC_AP_UNIT_INV;
			pcie_ipc_mlo->ap[ap_unit].mlo_unit = (int8)PCIE_IPC_MLO_UNIT_INV;
			pcie_ipc_mlo->sys.op_mode[ap_unit] = (uint8)PCIE_IPC_MLO_OPMODE_INV;
		}

		mlo_ipc_sys->mlo_disable_ap_bm = mlo_disable_ap_bm;
		mlo_ipc_sys->pcie_ipc_mlo = pcie_ipc_mlo;
	}

	{ // Allocate MLO IPC MSGQUEUE
		mlo_ipc_msg_queue_t * msg_queue;
		mlo_ipc_msg_t	* msg;
		int		msg_id;

		msg_queue = MLO_IPC_MSG_QUEUE();

		// Initialize system wide msg_queue lock
#if defined(CONFIG_SMP) || defined(CONFIG_PREEMPT)
		spin_lock_init(&msg_queue->lock);
#endif
		dll_init(&msg_queue->active_list);
		dll_init(&msg_queue->free_list);

		// Add messages to free list
		for (msg_id = 0; msg_id < MLO_IPC_MSG_ID_LAST; msg_id++) {
			msg = MLO_IPC_MSG(msg_id);

			msg->id = msg_id; // Valid for lifetime; Dont reset on free
			msg->prio = 0;
			msg->req_ap_bm = 0;

			dll_init(&msg->node);
			dll_append(&msg_queue->free_list, &msg->node);
		}
	}

	for (ap_unit = 0; ap_unit < PCIE_IPC_AP_UNIT_MAX; ++ap_unit)
	{
		mlo_ipc_evt_queue_t	*evt_queue;

		if (nvram_mlo[ap_unit] == (int)PCIE_IPC_MLO_UNIT_INV)
			continue; // AP does not participate in MLO

		evt_queue = MLO_IPC_EVT_QUEUE(ap_unit);
#if defined(CONFIG_SMP) || defined(CONFIG_PREEMPT)
		spin_lock_init(&evt_queue->lock);
#endif
		dll_init(&evt_queue->active_list);
	}
	mlo_ipc_sys->ap_unit_dflt_map = ap_unit_map;
	mlo_ipc_sys->ap_unit_map = ap_unit_map;
	mlo_ipc_sys->dump_signature = (uint16)OSL_RAND();
#if defined(CONFIG_SMP) || defined(CONFIG_PREEMPT)
	spin_lock_init(&mlo_ipc_sys->lock);
#endif

#if defined(MLO_IPC_PROC)
	if ((mlo_ipc_sys->proc_dir = mlo_ipc_proc_create()) == NULL) {
		MLO_IPC_ERROR(("%s: mlo_ipc_proc_init failed\n", __FUNCTION__));
		goto mlo_ipc_init_fail;
	}
#endif /* MLO_IPC_PROC */

	MLO_IPC_ERROR(("%s: SUCCESS; pcie_ipc_mlo - %p ap_unit_map %d mlo_total %d\n",
		__FUNCTION__, mlo_ipc_sys->pcie_ipc_mlo, ap_unit_map, mlo_total));

	return BCME_OK;

mlo_ipc_init_fail:

	MLO_IPC_ERROR(("%s: Failure\n", __FUNCTION__));
	mlo_ipc_exit();
	return ret;

} // mlo_ipc_init()

void // Destruct MLO IPC context
mlo_ipc_exit(void)
{
	mlo_ipc_sys_t	* mlo_ipc_sys;
	osl_t *osh;

	mlo_ipc_sys = mlo_ipc_sys_gp;

	if (mlo_ipc_sys == NULL) {
		return;
	}

	osh = mlo_ipc_sys->osh;

	// +++ Destruct Global access objects +++
#if defined(MLO_IPC_PROC)
	if (mlo_ipc_sys->proc_dir != NULL) {
		mlo_ipc_proc_destroy(mlo_ipc_sys->proc_dir);
		mlo_ipc_sys->proc_dir = NULL;
	}
#endif /* MLO_IPC_PROC */

	// Free PCIE IPC for MLO
	if (mlo_ipc_sys->pcie_ipc_mlo != NULL) {
		DMA_FREE_CONSISTENT(mlo_ipc_sys->osh, (void *)mlo_ipc_sys->pcie_ipc_mlo,
			mlo_ipc_sys->pcie_ipc_mlo_alloced, mlo_ipc_sys->pcie_ipc_mlo_pa, 0);
		mlo_ipc_sys->pcie_ipc_mlo = NULL;
	}

	if (mlo_ipc_sys->bhm_map_mem != NULL) {
		DMA_FREE_CONTIGUOUS(osh, mlo_ipc_sys->bhm_map_mem, mlo_ipc_sys->bhm_map_alloced,
			mlo_ipc_sys->bhm_map_pa, 0);
		mlo_ipc_sys->bhm_map_mem = NULL;
	}

#if defined(BCM_DMA_MALLOC)
	dma_malloc_deinit(osh);
#endif /* BCM_DMA_MALLOC */
	MFREE(mlo_ipc_sys->osh, mlo_ipc_sys, sizeof(mlo_ipc_sys_t));
	osl_detach(osh);
	mlo_ipc_sys_gp = NULL;

	if (mlo_ipc_dev_gp) {
		mlo_ipc_dev_free(mlo_ipc_dev_gp);
		mlo_ipc_dev_gp = NULL;
	}

	MLO_IPC_DEBUG(("Destructed MLO_IPC\n"));

} // mlo_ipc_exit()

/* ------------------------------------------------------------------------------------------------
 * Each MLO AP during PROBE, will join MLO_IPC star topology using register API.
 * ------------------------------------------------------------------------------------------------
 */
void // Register handler for Main AP
mlo_ipc_map_register(int ap_unit, mlo_ipc_generic_cb_t generic_cb,
	void *data, pcie_ipc_mlo_opmode_t op_mode)
{
	mlo_ipc_sys_t	* mlo_ipc_sys;

	mlo_ipc_sys = mlo_ipc_sys_gp;

	// Validate ap_unit
	ASSERT(ap_unit < PCIE_IPC_AP_UNIT_MAX);
	ASSERT(op_mode < PCIE_IPC_MLO_OPMODE_MAX);
	ASSERT(mlo_ipc_sys->pcie_ipc_mlo != NULL);
	ASSERT(mlo_ipc_sys->pcie_ipc_mlo->sys.mlo_total > 0);
	ASSERT(mlo_ipc_sys->pcie_ipc_mlo->sys.ap_unit_map == ap_unit);
	ASSERT(mlo_ipc_sys->pcie_ipc_mlo->sys.mlo_unit[ap_unit] != (int8)PCIE_IPC_MLO_UNIT_INV);

	mlo_ipc_sys->generic_cb[ap_unit] = generic_cb;
	mlo_ipc_sys->mlo_ap_data[ap_unit] = data;
	mlo_ipc_sys->pcie_ipc_mlo->sys.op_mode[ap_unit] = op_mode;
	mlo_ipc_sync_global_sys_state(MLO_IPC_CB_COMMIT_SYS_STATE);
} /* mlo_ipc_map_register() */

void
mlo_ipc_map_deregister(int ap_unit)
{
	mlo_ipc_sys_t	* mlo_ipc_sys;

	mlo_ipc_sys = mlo_ipc_sys_gp;

	// Validate ap_unit
	ASSERT(ap_unit < PCIE_IPC_AP_UNIT_MAX);
	ASSERT(mlo_ipc_sys->pcie_ipc_mlo != NULL);
	ASSERT(mlo_ipc_sys->pcie_ipc_mlo->sys.ap_unit_map == ap_unit);
	ASSERT(mlo_ipc_sys->pcie_ipc_mlo->sys.mlo_unit[ap_unit] != (int8)PCIE_IPC_MLO_UNIT_INV);

	mlo_ipc_sys->generic_cb[ap_unit] = NULL;
	mlo_ipc_sys->mlo_ap_data[ap_unit] = NULL;
	mlo_ipc_clear_ap_bm(ap_unit);
	mlo_ipc_set_mlo_down_ap_bm(MAXBITVAL(PCIE_IPC_AP_UNIT_MAX));
	mlo_ipc_set_mlc_state(ap_unit, mlo_ipc_mlc_reset_e);
	mlo_ipc_sync_global_sys_state(MLO_IPC_CB_COMMIT_SYS_STATE);
} /* mlo_ipc_map_deregister() */

void // Register handler for Auxillary AP
mlo_ipc_aap_register(int ap_unit, mlo_ipc_generic_cb_t generic_cb,
	void *data, pcie_ipc_mlo_opmode_t op_mode)
{
	mlo_ipc_sys_t	* mlo_ipc_sys;

	mlo_ipc_sys = mlo_ipc_sys_gp;

	// Validate ap_unit
	ASSERT(ap_unit < PCIE_IPC_AP_UNIT_MAX);
	ASSERT(op_mode < PCIE_IPC_MLO_OPMODE_MAX);
	ASSERT(mlo_ipc_sys->pcie_ipc_mlo != NULL);
	ASSERT(mlo_ipc_sys->pcie_ipc_mlo->sys.mlo_total > 1);
	ASSERT(mlo_ipc_sys->pcie_ipc_mlo->sys.ap_unit_map != ap_unit);
	ASSERT(mlo_ipc_sys->pcie_ipc_mlo->sys.mlo_unit[ap_unit] != (int8)PCIE_IPC_MLO_UNIT_INV);

	mlo_ipc_sys->generic_cb[ap_unit] = generic_cb;
	mlo_ipc_sys->mlo_ap_data[ap_unit] = data;
	mlo_ipc_sys->pcie_ipc_mlo->sys.op_mode[ap_unit] = op_mode;
	mlo_ipc_set_sys_mlo_active_bm(ap_unit);
	mlo_ipc_sync_global_sys_state(MLO_IPC_CB_COMMIT_SYS_STATE);
} /* mlo_ipc_aap_register() */

void
mlo_ipc_aap_deregister(int ap_unit)
{
	mlo_ipc_sys_t	* mlo_ipc_sys;

	mlo_ipc_sys = mlo_ipc_sys_gp;

	// Validate ap_unit
	ASSERT(ap_unit < PCIE_IPC_AP_UNIT_MAX);
	ASSERT(mlo_ipc_sys->pcie_ipc_mlo != NULL);
	ASSERT(mlo_ipc_sys->pcie_ipc_mlo->sys.ap_unit_map != ap_unit);
	ASSERT(mlo_ipc_sys->pcie_ipc_mlo->sys.mlo_unit[ap_unit] != (int8)PCIE_IPC_MLO_UNIT_INV);

	mlo_ipc_sys->generic_cb[ap_unit] = NULL;
	mlo_ipc_sys->mlo_ap_data[ap_unit] = NULL;
	mlo_ipc_clear_ap_bm(ap_unit);
	mlo_ipc_clear_sys_mlo_active_bm(ap_unit);
	mlo_ipc_set_sys_mlo_down(ap_unit, 1);
	mlo_ipc_set_mlc_state(ap_unit, mlo_ipc_mlc_reset_e);
	mlo_ipc_sync_global_sys_state(MLO_IPC_CB_COMMIT_SYS_STATE);
} /* mlo_ipc_aap_deregister() */

uint8
mlo_ipc_get_ap_bm(void)
{
	mlo_ipc_sys_t	* mlo_ipc_sys;

	mlo_ipc_sys = mlo_ipc_sys_gp;

	return mlo_ipc_sys->mlo_ap_bm;
}

uint8
mlo_ipc_set_ap_bm(int ap_unit)
{
	mlo_ipc_sys_t	* mlo_ipc_sys;

	// Validate ap_unit
	ASSERT(ap_unit < PCIE_IPC_AP_UNIT_MAX);
	mlo_ipc_sys = mlo_ipc_sys_gp;
	mlo_ipc_sys->mlo_ap_bm |= (1 << ap_unit); // Set MLO AP bitmap

	MLO_IPC_DEBUG(("%s: mlo_ap_bm %x\n", __FUNCTION__, mlo_ipc_sys->mlo_ap_bm));

	return mlo_ipc_sys->mlo_ap_bm;
}

uint8
mlo_ipc_clear_ap_bm(int ap_unit)
{
	mlo_ipc_sys_t	* mlo_ipc_sys;

	// Validate ap_unit
	ASSERT(ap_unit < PCIE_IPC_AP_UNIT_MAX);
	mlo_ipc_sys = mlo_ipc_sys_gp;
	mlo_ipc_sys->mlo_ap_bm &= ~(1 << ap_unit);

	return mlo_ipc_sys->mlo_ap_bm;
}

uint8
mlo_ipc_get_ap_unit_map(void)
{
	mlo_ipc_sys_t	* mlo_ipc_sys;

	mlo_ipc_sys = mlo_ipc_sys_gp;

	return mlo_ipc_sys->ap_unit_map;
}

void *
mlo_ipc_get_map_data(void)
{
	mlo_ipc_sys_t	* mlo_ipc_sys;
	uint8 ap_unit_map = mlo_ipc_get_ap_unit_map();

	mlo_ipc_sys = mlo_ipc_sys_gp;

	return mlo_ipc_sys->mlo_ap_data[ap_unit_map];
}

void *
mlo_ipc_get_mlo_ap_data(int ap_unit)
{
	mlo_ipc_sys_t	* mlo_ipc_sys;

	mlo_ipc_sys = mlo_ipc_sys_gp;

	if (ap_unit == PCIE_IPC_AP_UNIT_MAX) {
		return (void *)mlo_ipc_sys;
	}

	return mlo_ipc_sys->mlo_ap_data[ap_unit];
}

mlo_ipc_generic_cb_t
mlo_ipc_get_generic_cb(int ap_unit)
{
	mlo_ipc_sys_t	* mlo_ipc_sys;

	mlo_ipc_sys = mlo_ipc_sys_gp;

	return mlo_ipc_sys->generic_cb[ap_unit];
}

uint8
mlo_ipc_get_mlo_disable_ap_bm(void)
{
	mlo_ipc_sys_t	* mlo_ipc_sys;

	mlo_ipc_sys = mlo_ipc_sys_gp;

	return mlo_ipc_sys->mlo_disable_ap_bm;
}

void
mlo_ipc_set_mlc_state(int ap_unit, mlo_ipc_mlc_state_t mlc_state)
{
	mlo_ipc_sys_t	* mlo_ipc_sys;

	mlo_ipc_sys = mlo_ipc_sys_gp;

	mlo_ipc_sys->mlc_state[ap_unit] = mlc_state;
}

uint8
mlo_ipc_get_sys_ap_bm(void)
{
	mlo_ipc_sys_t	* mlo_ipc_sys;

	mlo_ipc_sys = mlo_ipc_sys_gp;

	return mlo_ipc_sys->pcie_ipc_mlo->sys.mlo_ap_bm;
}

uint8
mlo_ipc_get_sys_mlo_down(int ap_unit)
{
	mlo_ipc_sys_t	* mlo_ipc_sys;

	mlo_ipc_sys = mlo_ipc_sys_gp;

	return mlo_ipc_sys->pcie_ipc_mlo->sys.mlo_down[ap_unit];
}

void
mlo_ipc_set_sys_mlo_down(int ap_unit, uint8 mlo_down)
{
	mlo_ipc_sys_t	* mlo_ipc_sys;

	ASSERT(ap_unit < PCIE_IPC_AP_UNIT_MAX);

	mlo_ipc_sys = mlo_ipc_sys_gp;

	/* ignore mlo_down setting for non-MLO eligible unit */
	if (mlo_ipc_sys->mlo_eligible[ap_unit] == FALSE) {
		return;
	}

	if ((mlo_down == 0) &&
		((mlo_ipc_sys->mlo_disable_ap_bm & NBITVAL(ap_unit)) != 0)) {
		MLO_IPC_ERROR(("%s: clearing ap[%d] mlo_down is not allowed\n",
			__FUNCTION__, ap_unit));
		ASSERT(0);
	}
	mlo_ipc_sys->pcie_ipc_mlo->sys.mlo_down[ap_unit] = mlo_down;
	mlo_ipc_sync_global_sys_state(MLO_IPC_CB_COMMIT_SYS_STATE);
}

void
mlo_ipc_set_mlo_down_ap_bm(uint8 mlo_down_ap_bm)
{
	int apix;

	for (apix = 0; apix < PCIE_IPC_AP_UNIT_MAX; apix++) {
		if (mlo_down_ap_bm & NBITVAL(apix)) {
			mlo_ipc_set_sys_mlo_down(apix, 1);
		} else {
			mlo_ipc_set_sys_mlo_down(apix, 0);
		}
	}
}

uint8
mlo_ipc_get_mlo_down_ap_bm(void)
{
	int apix;
	uint8 mlo_down_ap_bm = 0;

	for (apix = 0; apix < PCIE_IPC_AP_UNIT_MAX; apix++) {
		if (mlo_ipc_get_sys_mlo_down(apix) != 0) {
			mlo_down_ap_bm |= NBITVAL(apix);
		}
	}

	return mlo_down_ap_bm;
}

uint8
mlo_ipc_get_sys_mlo_active_bm(void)
{
	mlo_ipc_sys_t	* mlo_ipc_sys;

	mlo_ipc_sys = mlo_ipc_sys_gp;

	return mlo_ipc_sys->pcie_ipc_mlo->sys.mlo_active_bm;
}

uint8
mlo_ipc_set_sys_mlo_active_bm(int ap_unit)
{
	mlo_ipc_sys_t	* mlo_ipc_sys;
	int8 mlo_unit;

	// Validate ap_unit
	ASSERT(ap_unit < PCIE_IPC_AP_UNIT_MAX);
	mlo_ipc_sys = mlo_ipc_sys_gp;

	mlo_unit = mlo_ipc_sys->pcie_ipc_mlo->sys.mlo_unit[ap_unit];
	ASSERT(mlo_unit != PCIE_IPC_MLO_UNIT_INV);

	mlo_ipc_sys->pcie_ipc_mlo->sys.mlo_active_bm |= (1 << mlo_unit); // Set MLO unit bitmap

	MLO_IPC_DEBUG(("%s: mlo_active_bm %x\n", __FUNCTION__,
		mlo_ipc_sys->pcie_ipc_mlo->sys.mlo_active_bm));

	mlo_ipc_sync_global_sys_state(MLO_IPC_CB_COMMIT_SYS_STATE);
	return mlo_ipc_sys->pcie_ipc_mlo->sys.mlo_active_bm;
}

uint8
mlo_ipc_clear_sys_mlo_active_bm(int ap_unit)
{
	mlo_ipc_sys_t	* mlo_ipc_sys;
	int8 mlo_unit;

	// Validate ap_unit
	ASSERT(ap_unit < PCIE_IPC_AP_UNIT_MAX);
	mlo_ipc_sys = mlo_ipc_sys_gp;

	mlo_unit = mlo_ipc_sys->pcie_ipc_mlo->sys.mlo_unit[ap_unit];
	ASSERT(mlo_unit != PCIE_IPC_MLO_UNIT_INV);

	mlo_ipc_sys->pcie_ipc_mlo->sys.mlo_active_bm &= ~(1 << mlo_unit);

	mlo_ipc_sync_global_sys_state(MLO_IPC_CB_COMMIT_SYS_STATE);
	return mlo_ipc_sys->pcie_ipc_mlo->sys.mlo_active_bm;
}

/* make update to System-wide MLO attributes visible: pcie_ipc_mlo_sys */
/* For SoCs that don't have coherent caches, do an explicit cache operations */
void
mlo_ipc_sync_global_sys_state(mlo_ipc_cb_t cb_event)
{
#if !defined(OSL_CACHE_COHERENT)
	mlo_ipc_sys_t	*mlo_ipc_sys = mlo_ipc_sys_gp;
	mlo_ipc_generic_cb_t generic_cb;
	int ap_unit;
	if (mlo_ipc_sys == NULL) {
		return;
	}
	ap_unit = mlo_ipc_sys->ap_unit_map;
	generic_cb = mlo_ipc_get_generic_cb(ap_unit);
	if (generic_cb == NULL) {
		return;
	}
	generic_cb(cb_event,
		mlo_ipc_get_map_data(),
		ap_unit,
		lower_32_bits((dma_addr_t)&mlo_ipc_sys->pcie_ipc_mlo->sys),
		upper_32_bits((dma_addr_t)&mlo_ipc_sys->pcie_ipc_mlo->sys),
		sizeof(pcie_ipc_mlo_sys_t));
#endif /* OSL_CACHE_COHERENT */
}

/**
 * -----------------------------------------------------------------------------------------------
 *  Section: MLO_IPC_EVENT
 * -----------------------------------------------------------------------------------------------
 */

#define MLO_IPC_EVT_QUEUE_MAX_CNT	1024

/* Enqueues an event message to consumer link identified by AP unit number.
 * and informs consumer using registered wakeup callback.
 */
int
mlo_ipc_evt_queue_enqueue(osl_t *osh, uint16 dest_ap_unit, mlo_ipc_evt_msg_t * evt_msg)
{
	mlo_ipc_evt_queue_t * evt_queue;
	int	ret = BCME_OK;
	mlo_ipc_generic_cb_t generic_cb;
	void * mlo_ap_data;

	evt_queue = MLO_IPC_EVT_QUEUE(dest_ap_unit);

	MLO_IPC_EVT_QUEUE_LOCK(dest_ap_unit); // +++++++++++++++++++++++++++++++++++++++++++++++++

	if (evt_queue->active_cnt < MLO_IPC_EVT_QUEUE_MAX_CNT) {

#if defined(BCM_BTRACE)
		btrace(osh, BTRACE_EVT_CAT_MLO_MLC, BTRACE_EVT_TYPE_MLO_EVT_ENQUEUE,
			(evt_msg->ap_unit & 0xff) | (dest_ap_unit << 8) | (evt_msg->evt_id << 16));
#endif /* BCM_BTRACE */

		dll_append(&evt_queue->active_list, &evt_msg->node);

		evt_queue->enqueue_cnt++;
		evt_queue->active_cnt++;
		evt_queue->last_enq_time = jiffies;
	} else {
		if (time_after(jiffies, evt_queue->last_enq_time +
			msecs_to_jiffies(MLO_IPC_WAIT_TIMEOUT))) {
			MLO_IPC_ERROR(("%s: ap_unit %d dequeue schedule timeout %u (ms)\n",
				__FUNCTION__, dest_ap_unit,
				jiffies_to_msecs(jiffies - evt_queue->last_enq_time)));
		}

		evt_queue->failure_cnt++;
		ret = BCME_NORESOURCE;
	}

	MLO_IPC_EVT_QUEUE_UNLK(dest_ap_unit); // -------------------------------------------------
	mlo_ap_data = mlo_ipc_get_mlo_ap_data(dest_ap_unit);
	generic_cb = mlo_ipc_get_generic_cb(dest_ap_unit);

	ASSERT(mlo_ap_data != NULL);
	ASSERT(generic_cb != NULL);

	generic_cb(MLO_IPC_CB_EVT_WAKE_UP, mlo_ap_data, (int)dest_ap_unit, 0, 0, 0);

	return ret;
} /* mlo_ipc_evt_queue_enqueue() */

/* Dequeues an event message from consumer link event queue identified by AP unit number. */
mlo_ipc_evt_msg_t *
mlo_ipc_evt_queue_dequeue(osl_t *osh, uint16 ap_unit)
{
	mlo_ipc_evt_queue_t * evt_queue;
	mlo_ipc_evt_msg_t * evt_msg = NULL;
	dll_t	* item;

	evt_queue = MLO_IPC_EVT_QUEUE(ap_unit);

	MLO_IPC_EVT_QUEUE_LOCK(ap_unit); // +++++++++++++++++++++++++++++++++++++++++++++++++

	if (evt_queue->active_cnt) {
		item = dll_head_p(&evt_queue->active_list);
		evt_msg = container_of(item, mlo_ipc_evt_msg_t, node);

		// Remove the msg node from active list
		dll_delete(&evt_msg->node);

		evt_queue->dequeue_cnt++;
		evt_queue->active_cnt--;

#if defined(BCM_BTRACE)
		btrace(osh, BTRACE_EVT_CAT_MLO_MLC, BTRACE_EVT_TYPE_MLO_EVT_DEQUEUE,
			(evt_msg->ap_unit & 0xff) | (ap_unit << 8) | (evt_msg->evt_id << 16));
#endif /* BCM_BTRACE */
	}

	MLO_IPC_EVT_QUEUE_UNLK(ap_unit); // ------------------------------------------------

	return evt_msg;
} /* mlo_ipc_evt_queue_dequeue() */

bool
mlo_ipc_evt_queue_available(uint16 ap_unit)
{
	mlo_ipc_evt_queue_t *evt_queue = MLO_IPC_EVT_QUEUE(ap_unit);

	/* No locking, we assume there is only a single consumer */
	return evt_queue->active_cnt != 0;
}

/* Allocate an event message in HND context (osh) */
mlo_ipc_evt_msg_t *
mlo_ipc_evt_msg_alloc(int bytes)
{
	mlo_ipc_evt_msg_t * evt_msg;

	evt_msg = (mlo_ipc_evt_msg_t *) MALLOC(mlo_ipc_sys_gp->osh, bytes);

	if (evt_msg)
		dll_init(&evt_msg->node);

	return evt_msg;
}

/* Deallocate an event message from HND context (osh) */
void
mlo_ipc_evt_msg_free(mlo_ipc_evt_msg_t * evt_msg)
{
	int bytes;
	ASSERT(evt_msg != NULL);

	bytes = evt_msg->bytes;
	if (bytes) {
		MFREE(mlo_ipc_sys_gp->osh, evt_msg, bytes);
	}
}

static struct device*
mlo_ipc_dev_alloc(void)
{
	struct device *mlo_ipc_dev = kzalloc(sizeof(struct device), GFP_KERNEL);
	if (!mlo_ipc_dev) {
		return NULL;
	}
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 19, 0)
#ifndef CMWIFI
	arch_setup_dma_ops(mlo_ipc_dev, 0, 0, NULL, true);
#endif /* CMWIFI */
	dma_coerce_mask_and_coherent(mlo_ipc_dev, DMA_BIT_MASK(64));
#else
	mlo_ipc_dev->coherent_dma_mask = DMA_BIT_MASK(64);
	mlo_ipc_dev->dma_mask = &mlo_ipc_dev->coherent_dma_mask;
#endif /* KERNEL >= 3.19 */

	return mlo_ipc_dev;
}

static void
mlo_ipc_dev_free(struct device *mlo_ipc_dev)
{
	kfree(mlo_ipc_dev);
}

bool mlo_ipc_bh_request(void)
{
	MLO_IPC_LOCK();
	if (!mlo_ipc_sys_gp->mlo_bh_in_progress) {
		mlo_ipc_sys_gp->mlo_bh_in_progress = TRUE;
		MLO_IPC_UNLK();
		return TRUE;
	}
	MLO_IPC_UNLK();
	return FALSE;
}

void mlo_ipc_bh_complete(void)
{
	MLO_IPC_LOCK();
	if (!mlo_ipc_sys_gp->mlo_bh_in_progress) {
		MLO_IPC_ERROR(("%s: MLO BH is not in progress\n", __FUNCTION__));
	}
	mlo_ipc_sys_gp->mlo_bh_in_progress = FALSE;
	MLO_IPC_UNLK();
}

void
mlo_ipc_dump(uint16 ap_unit, struct bcmstrbuf *b)
{
	mlo_ipc_msg_queue_t * msg_queue;
	mlo_ipc_evt_queue_t * evt_queue;

	msg_queue = MLO_IPC_MSG_QUEUE();

	bcm_bprintf(b, "msg_queue stats: enqueue %d dequeue %d\n",
		msg_queue->enqueue_cnt, msg_queue->dequeue_cnt);

	evt_queue = MLO_IPC_EVT_QUEUE(ap_unit);

	bcm_bprintf(b, "evt_queue stats: ap_unit[%d]\n", ap_unit);
	bcm_bprintf(b, "\t active %d enqueue %d dequeue %d failure %d last_enq_time %ld\n",
		evt_queue->active_cnt, evt_queue->enqueue_cnt, evt_queue->dequeue_cnt,
		evt_queue->failure_cnt, evt_queue->last_enq_time);
} /* mlo_ipc_dump() */

#if defined(BCM_DMA_MALLOC)
void
mlo_ipc_dma_malloc_dump(struct bcmstrbuf *b)
{
	osl_t *osh = mlo_ipc_sys_gp->osh;

	if (b) {
		bcm_bprintf(b, "MLO IPC DMA malloc dump:\n");
	}
#if defined(BCM_DMA_MALLOC_DEBUG)
	else {
		printf("MLO IPC DMA malloc dump:\n");
	}
#endif /* BCM_DMA_MALLOC_DEBUG */

	dma_malloc_dump(osh, b);
}
#endif /* BCM_DMA_MALLOC */

bool
mlo_ipc_get_ap_trap_complete(uint16 ap_unit)
{
	return mlo_ipc_sys_gp->ap_trap[ap_unit];
}

void
mlo_ipc_set_ap_trap_complete(uint16 ap_unit)
{
	mlo_ipc_sys_gp->ap_trap[ap_unit] = TRUE;
}

int
mlo_ipc_get_ap_op_mode(uint16 ap_unit)
{
	return mlo_ipc_sys_gp->pcie_ipc_mlo->sys.op_mode[ap_unit];
}

static bool
mlo_ipc_role_change(uint8 ap_unit, int8 *new_mlo_unit)
{
	mlo_ipc_sys_t *mlo_ipc_sys;
	mlo_ipc_sys = mlo_ipc_sys_gp;

	if (mlo_ipc_sys->mlo_config[ap_unit] ==
		mlo_ipc_sys->pcie_ipc_mlo->sys.mlo_unit[ap_unit]) {
		*new_mlo_unit = mlo_ipc_sys->pcie_ipc_mlo->sys.mlo_unit[ap_unit];
		return FALSE;
	} else {
		MLO_IPC_ERROR(("%s: ap_unit %d mlo_unit changes from %d to %d\n",
			__FUNCTION__, ap_unit, mlo_ipc_sys->pcie_ipc_mlo->sys.mlo_unit[ap_unit],
			mlo_ipc_sys->mlo_config[ap_unit]));
		*new_mlo_unit = (int8)mlo_ipc_sys->mlo_config[ap_unit];
		return TRUE;
	}
}

void
mlo_ipc_mlo_unit_reconfig(void)
{
	mlo_ipc_sys_t *mlo_ipc_sys;
	uint8 ap_unit;
	int8 mlo_unit;
	uint8 mlo_active_bm = 0;

	mlo_ipc_sys = mlo_ipc_sys_gp;

	for (ap_unit = 0; ap_unit < PCIE_IPC_AP_UNIT_MAX; ++ap_unit) {
		mlo_unit = (int8)mlo_ipc_sys->mlo_config[ap_unit];
		mlo_ipc_sys->pcie_ipc_mlo->sys.mlo_unit[ap_unit] = mlo_unit;

		if (mlo_unit == PCIE_IPC_MLO_UNIT_INV) {
			continue;
		}

		if (mlo_unit == PCIE_IPC_MLO_UNIT_MAP) {
			mlo_ipc_sys->ap_unit_map = ap_unit;
			mlo_ipc_sys->pcie_ipc_mlo->sys.ap_unit_map = ap_unit;
		}

		/* do not set mlo active bitmap for the AAPs which are in power-down */
		if ((mlo_unit != PCIE_IPC_MLO_UNIT_MAP) &&
			!(mlo_ipc_sys->mlo_ap_bm & NBITVAL(ap_unit))) {
			continue;
		}

		mlo_active_bm |= NBITVAL(mlo_unit);
	}

	mlo_ipc_sys->pcie_ipc_mlo->sys.mlo_active_bm = mlo_active_bm;

	mlo_ipc_sync_global_sys_state(MLO_IPC_CB_COMMIT_SYS_STATE);
	MLO_IPC_ERROR(("%s: ap_unit_map %u mlo_active_bm 0x%x\n",
		__FUNCTION__, mlo_ipc_sys->ap_unit_map, mlo_active_bm));
}

void *
mlo_ipc_get_bhm_map(uint32 size, uint32 align_bits, dmaaddr_t *pa)
{
	mlo_ipc_sys_t	* mlo_ipc_sys;
	osl_t *osh;
	uint _alloced;
	dmaaddr_t _pa;
	uint32 pa_align, readjust;

	mlo_ipc_sys = mlo_ipc_sys_gp;

	if (mlo_ipc_sys == NULL) {
		MLO_IPC_ERROR(("%s: mlo_ipc_sys is NULL\n", __FUNCTION__));
		goto fail;
	}

	if (mlo_ipc_sys->bhm_map_mem) {
		goto done;
	}

	osh = mlo_ipc_sys->osh;

	/* Do allocation without alignment to avoid memory waste,
	 * expect linux return at 4KB alignment already
	 */
	if ((mlo_ipc_sys->bhm_map_mem = DMA_ALLOC_CONTIGUOUS(osh, size,
		0, &_alloced, &_pa, 0)) == NULL) {
		MLO_IPC_ERROR(("%s: DMA_ALLOC_CONTIGUOUS failed\n", __FUNCTION__));
		goto fail;
	}

	/* Check alignment */
	if (ISALIGNED(PHYSADDRLO(_pa), NBITVAL(align_bits))) {
		mlo_ipc_sys->bhm_map_va = mlo_ipc_sys->bhm_map_mem;
		mlo_ipc_sys->bhm_map_pa = _pa;
		mlo_ipc_sys->bhm_map_alloced = _alloced;
		goto done;
	}

	/* Free it and do allocation with SW alignment */
	DMA_FREE_CONTIGUOUS(osh, mlo_ipc_sys->bhm_map_mem, _alloced, _pa, 0);

	if ((mlo_ipc_sys->bhm_map_mem = DMA_ALLOC_CONTIGUOUS(osh, size,
		align_bits, &_alloced, &_pa, 0)) == NULL) {
		MLO_IPC_ERROR(("%s: DMA_ALLOC_CONTIGUOUS failed\n", __FUNCTION__));
		goto fail;
	}

	/* Perform alignment by rounding up to alignment boundary */
	/* Compute bytes to skip from front to align physical memory */
	pa_align = ROUNDUP(PHYSADDRLO(_pa), NBITVAL(align_bits));
	readjust = pa_align - PHYSADDRLO(_pa);
	ASSERT(pa_align >= PHYSADDRLO(_pa));
	PHYSADDRLOSET(_pa, pa_align);

	mlo_ipc_sys->bhm_map_va = (void *)((uintptr)mlo_ipc_sys->bhm_map_mem + readjust);
	mlo_ipc_sys->bhm_map_pa = _pa;
	mlo_ipc_sys->bhm_map_alloced = _alloced;

	MLO_IPC_ERROR(("%s: PCIe IPC BHM MAP ALLOC SUCCESS: size %dMB "
		"DMA Buf va %px pa hi 0x%08x lo 0x%08x alloced %7u\n",
		__FUNCTION__, size, mlo_ipc_sys->bhm_map_va,
		(uint)PHYSADDRHI(_pa), (uint)PHYSADDRLO(_pa), _alloced));

done:
	*pa = mlo_ipc_sys->bhm_map_pa;

	return mlo_ipc_sys->bhm_map_va;

fail:
	MLO_IPC_ERROR(("%s: PCIe IPC BHM MAP ALLOC FAILURE: size %u\n",
		__FUNCTION__, size));

	return NULL;
}

uint8
mlo_ipc_get_ap_unit_default_map(void)
{
	mlo_ipc_sys_t	* mlo_ipc_sys;

	mlo_ipc_sys = mlo_ipc_sys_gp;

	return mlo_ipc_sys->ap_unit_dflt_map;
}

static bool
_is_mlo_ipc_mlc_synced(uint8 mlo_ap_bm)
{
	mlo_ipc_sys_t	* mlo_ipc_sys;
	int ap_unit;
	bool synced = FALSE;

	ASSERT(mlo_ap_bm != 0);
	mlo_ipc_sys = mlo_ipc_sys_gp;

	for (ap_unit = 0; ap_unit < PCIE_IPC_AP_UNIT_MAX; ap_unit++) {
		if ((mlo_ap_bm & NBITVAL(ap_unit)) == 0) {
			continue;
		}

		if (mlo_ipc_sys->mlc_state[ap_unit] != mlo_ipc_mlc_sync_e) {
			synced = FALSE;
			break;
		} else {
			synced = TRUE;
		}
	}

	return synced;
}

static int
mlo_ipc_restart(uint8 ap_unit)
{
	void *mlo_ap_data;
	mlo_ipc_generic_cb_t generic_cb;
	int ret;

	MLO_IPC_ERROR(("%s: enter\n", __FUNCTION__));

	mlo_ap_data = mlo_ipc_get_mlo_ap_data(ap_unit);
	generic_cb = mlo_ipc_get_generic_cb(ap_unit);

	ASSERT(mlo_ap_data != NULL);
	ASSERT(generic_cb != NULL);

	ret = generic_cb(MLO_IPC_CB_RESTART, mlo_ap_data, ap_unit, 0, 0, 0);

	MLO_IPC_ERROR(("%s: exit\n", __FUNCTION__));

	return ret;
}

static void
mlo_ipc_mlo_role_reconfig_cb(void *data, uint8 ap_unit)
{
	mlo_ipc_sys_t	*mlo_ipc_sys = (mlo_ipc_sys_t*)data;

	ASSERT(mlo_ipc_sys == mlo_ipc_sys_gp);
	ASSERT(mlo_ipc_sys->reconfig_in_progress);

	MLO_IPC_LOCK();

	ASSERT(mlo_ipc_sys->req_reconfig_ap_bm & NBITVAL(ap_unit));
	mlo_ipc_sys->req_reconfig_ap_bm &= ~NBITVAL(ap_unit);
	MLO_IPC_ERROR(("%s: req_reconfig_ap_bm 0x%x cleared for ap_unit %d\n",
		__FUNCTION__, mlo_ipc_sys->req_reconfig_ap_bm, ap_unit));

	MLO_IPC_UNLK();
}

static int
mlo_ipc_mlo_role_update(mlo_ipc_sys_t * mlo_ipc_sys, int8 *ap_unit_restart)
{
	uint8 iter;
	int8 new_mlo_unit;
	uint8 mlo_ap_bm = mlo_ipc_get_ap_bm();
	bool config_changed = FALSE;
	uint16 evt_msg_bytes;
	mlo_ipc_evt_msg_t *evt_msg;
	mlo_ipc_role_reconfig_msg_t *role_reconfig_msg;
	int ret = BCME_OK;

	MLO_IPC_LOCK();

	mlo_ipc_sys->req_reconfig_ap_bm = 0;
	for (iter = 0; iter < PCIE_IPC_AP_UNIT_MAX; iter++) {
		if (!mlo_ipc_role_change(iter, &new_mlo_unit)) {
			continue;
		}

		config_changed = TRUE;

		if (!(mlo_ap_bm & NBITVAL(iter))) {
			continue;
		}

		ASSERT(new_mlo_unit != PCIE_IPC_MLO_UNIT_INV);

		*ap_unit_restart = (int8)iter;
		mlo_ipc_sys->req_reconfig_ap_bm |= NBITVAL(iter);
		MLO_IPC_ERROR(("%s: req_reconfig_ap_bm 0x%x set for ap_unit %d\n",
			__FUNCTION__, mlo_ipc_sys->req_reconfig_ap_bm, iter));

		evt_msg_bytes = sizeof(mlo_ipc_evt_msg_t) + sizeof(mlo_ipc_role_reconfig_msg_t);
		evt_msg = mlo_ipc_evt_msg_alloc(evt_msg_bytes);
		if (evt_msg == NULL) {
			MLO_IPC_ERROR(("%s: MLO IPC EVENT MSG alloc failed\n", __FUNCTION__));
			ASSERT(evt_msg);
			MLO_IPC_UNLK();
			return BCME_NOMEM;
		}
		evt_msg->ap_unit	= PCIE_IPC_AP_UNIT_MAX;
		evt_msg->evt_id		= MLO_IPC_EVT_ID_ROLE_RECONFIG;
		evt_msg->evt_flags	= 0;
		evt_msg->bytes		= evt_msg_bytes;
		role_reconfig_msg	= (mlo_ipc_role_reconfig_msg_t *)evt_msg->data;
		role_reconfig_msg->mlo_role_reconfig_cb = mlo_ipc_mlo_role_reconfig_cb;
		role_reconfig_msg->new_mlo_unit = new_mlo_unit;

		ret = mlo_ipc_evt_queue_enqueue(mlo_ipc_sys->osh, iter, evt_msg);
		if (ret	!= BCME_OK) {
			MLO_IPC_ERROR(("%s: ROLE RECONFIG EVT enqueue to ap[%d] failed ret %d\n",
				__FUNCTION__, iter, ret));
			/* Release event message */
			mlo_ipc_evt_msg_free(evt_msg);
		}
	}

	if (config_changed && (mlo_ipc_sys->req_reconfig_ap_bm == 0)) {
		MLO_IPC_ERROR(("%s: reconfigure powered down MLO APs\n",
			__FUNCTION__));
		mlo_ipc_mlo_unit_reconfig();
	}

	MLO_IPC_UNLK();

	return ret;
}

#if defined(MLO_IPC_PROC)
#define MLO_IPC_PROC_DIR_NAME                     "mlo_ipc"
#define MLO_IPC_PROC_MLO_CONFIG_NAME              "mlo_config"
#define MLO_IPC_PROC_MAIN_NAME                    "main"
#define MLO_IPC_PROC_STATUS_NAME                  "status"
#define MLO_IPC_PROC_DISABLE_RECONFIG_NAME        "disable_reconfig"

static ssize_t
mlo_ipc_proc_mlo_config_read(struct file *file, char __user *ubuf,
	size_t cnt, loff_t *ppos)
{
	char buf[32] = {0};
	int len = 0;
	mlo_ipc_sys_t * mlo_ipc_sys = mlo_ipc_sys_gp;
	int ap_unit;

	if ((*ppos > 0) || (cnt < sizeof(buf))) {
		return 0;
	}

	for (ap_unit = 0; ap_unit < PCIE_IPC_AP_UNIT_MAX; ++ap_unit) {
		len += sprintf(buf + len, "%d ", mlo_ipc_sys->mlo_config[ap_unit]);
	}

	len += sprintf(buf + len, "\n\n");

	if (copy_to_user(ubuf, buf, len)) {
		return -EFAULT;
	}

	*ppos = len;

	return len;
}

static ssize_t
mlo_ipc_proc_mlo_config_write(struct file *file,
	const char __user *ubuf, size_t cnt, loff_t *ppos)
{
	mlo_ipc_sys_t * mlo_ipc_sys = mlo_ipc_sys_gp;
	char mlo_config_str[32];
	int len;
	int mlo_config[PCIE_IPC_AP_UNIT_MAX];
	int8 ap_unit_restart;
	unsigned long wait_timeout;

	if (mlo_ipc_sys == NULL) {
		MLO_IPC_ERROR(("%s: mlo_ipc_sys is NULL\n", __FUNCTION__));
		return cnt;
	}

	if (mlo_ipc_sys->disable_reconfig) {
		MLO_IPC_ERROR(("%s: MLO reconfiguration is disabled\n", __FUNCTION__));
		return cnt;
	}

	if (mlo_ipc_sys->reconfig_in_progress) {
		MLO_IPC_ERROR(("%s: MLO role migration is in progress\n", __FUNCTION__));
		goto out;
	}

	mlo_ipc_sys->reconfig_in_progress = TRUE;

	if (mlo_ipc_get_sys_mlo_down(mlo_ipc_sys->ap_unit_map) == 0) {
		MLO_IPC_ERROR(("%s: MLO not down\n", __FUNCTION__));
		goto out;
	}

	if (cnt >= sizeof(mlo_config_str)) {
		MLO_IPC_ERROR(("%s: cnt %u >= %u\n", __FUNCTION__,
			(uint)cnt, (uint)sizeof(mlo_config_str)));
		goto out;
	}

	len = min((unsigned long)cnt, (unsigned long)(sizeof(mlo_config_str) - 1));
	if (copy_from_user(mlo_config_str, ubuf, len)) {
		MLO_IPC_ERROR(("%s: copy from user failed\n", __FUNCTION__));
		goto out;
	}

	mlo_config_str[len] = '\0';
	if (len > 0) {
		mlo_config_str[len - 1] = '\0';
	}

	if (!_is_mlo_config_validated(mlo_config_str, mlo_config, NULL)) {
		MLO_IPC_ERROR(("%s: invalid mlo config\n", __FUNCTION__));
		goto out;
	}

	if (memcmp(mlo_ipc_sys->mlo_config, mlo_config, sizeof(mlo_ipc_sys->mlo_config)) == 0) {
		MLO_IPC_ERROR(("%s: mlo role not changed\n", __FUNCTION__));
		goto out;
	}

	memcpy(mlo_ipc_sys->mlo_config, mlo_config, sizeof(mlo_ipc_sys->mlo_config));

	ap_unit_restart = PCIE_IPC_MLO_UNIT_INV;
	if (mlo_ipc_mlo_role_update(mlo_ipc_sys, &ap_unit_restart) != BCME_OK) {
		MLO_IPC_ERROR(("%s: failed to update mlo role\n", __FUNCTION__));
		goto out;
	}

	if (ap_unit_restart != PCIE_IPC_MLO_UNIT_INV) {
		wait_timeout = jiffies + msecs_to_jiffies(MLO_IPC_WAIT_TIMEOUT);
		while (mlo_ipc_sys->req_reconfig_ap_bm &&
			time_after(wait_timeout, jiffies)) {
			schedule();
		}

		if (mlo_ipc_sys->req_reconfig_ap_bm) {
			MLO_IPC_ERROR(("%s: reconfig callbacks timed out, reqs 0x%x\n",
				__FUNCTION__, mlo_ipc_sys->req_reconfig_ap_bm));
		}
		else {
			MLO_IPC_ERROR(("%s: reconfig callbacks done\n", __FUNCTION__));
			if (mlo_ipc_restart(ap_unit_restart) == BCME_OK) {
				uint8 mlo_ap_bm = mlo_ipc_get_ap_bm();
				wait_timeout = jiffies + msecs_to_jiffies(MLO_IPC_WAIT_TIMEOUT);
				while (!_is_mlo_ipc_mlc_synced(mlo_ap_bm) &&
					time_after(wait_timeout, jiffies)) {
					schedule();
				}
				MLO_IPC_ERROR(("%s: mlo_ipc_mlc_synced: %d\n",
					__FUNCTION__, _is_mlo_ipc_mlc_synced(mlo_ap_bm)));
			}
		}
	}

	MLO_IPC_ERROR(("\nmlo reconfig finished:\n%d %d %d %d\n\n",
		mlo_config[0], mlo_config[1], mlo_config[2], mlo_config[3]));

out:
	mlo_ipc_sys->reconfig_in_progress = FALSE;

	return cnt;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0))
static const struct proc_ops mlo_ipc_proc_mlo_config_ops = {
	.proc_read	= mlo_ipc_proc_mlo_config_read,
	.proc_write	= mlo_ipc_proc_mlo_config_write,
};
#else /* KERNEL < 5.6.0 */
static const struct file_operations mlo_ipc_proc_mlo_config_ops = {
	.owner		= THIS_MODULE,
	.read		= mlo_ipc_proc_mlo_config_read,
	.write		= mlo_ipc_proc_mlo_config_write,
};
#endif /* KERNEL < 5.6.0 */

static ssize_t
mlo_ipc_proc_main_read(struct file *file, char __user *ubuf,
	size_t cnt, loff_t *ppos)
{
	char buf[32] = {0};
	int len = 0;
	mlo_ipc_sys_t * mlo_ipc_sys = mlo_ipc_sys_gp;

	if ((*ppos > 0) || (cnt < sizeof(buf))) {
		return 0;
	}

	len += snprintf(buf, sizeof(buf), "%d", mlo_ipc_sys->ap_unit_map);

	if (copy_to_user(ubuf, buf, len)) {
		return -EFAULT;
	}

	*ppos = len;

	return len;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0))
static const struct proc_ops mlo_ipc_proc_main_ops = {
	.proc_read	= mlo_ipc_proc_main_read,
};
#else /* KERNEL < 5.6.0 */
static const struct file_operations mlo_ipc_proc_main_ops = {
	.owner		= THIS_MODULE,
	.read		= mlo_ipc_proc_main_read,
};
#endif /* KERNEL < 5.6.0 */

static int mlo_ipc_proc_status_show(struct seq_file *m, void *v)
{
	int prec = 12;
	uint8 ap_unit;
	mlo_ipc_sys_t * mlo_ipc_sys = mlo_ipc_sys_gp;

	seq_printf(m, "%*s", prec + 21, "");
	for (ap_unit = 0; ap_unit < PCIE_IPC_AP_UNIT_MAX; ++ap_unit) {
		seq_printf(m, "Unit%-20d", ap_unit);
	}
	seq_putc(m, '\n');

	seq_printf(m, "%*s: ", prec, "MLO Eligible");
	for (ap_unit = 0; ap_unit < PCIE_IPC_AP_UNIT_MAX; ++ap_unit) {
		seq_printf(m, "%24s",
			(mlo_ipc_sys->mlo_eligible[ap_unit] == TRUE) ? "Yes" : "No");
	}
	seq_putc(m, '\n');

	seq_printf(m, "%*s: ", prec, "MLO Unit");
	for (ap_unit = 0; ap_unit < PCIE_IPC_AP_UNIT_MAX; ++ap_unit) {
		seq_printf(m, "%24d", mlo_ipc_sys->pcie_ipc_mlo->sys.mlo_unit[ap_unit]);
	}
	seq_putc(m, '\n');

	seq_printf(m, "%*s: ", prec, "Op Mode");
	for (ap_unit = 0; ap_unit < PCIE_IPC_AP_UNIT_MAX; ++ap_unit) {
		uint8 op_mode = mlo_ipc_get_ap_op_mode(ap_unit);
		char *op_mode_str;
		if (op_mode == PCIE_IPC_MLO_OPMODE_NIC) {
			op_mode_str = "NIC";
		} else if (op_mode == PCIE_IPC_MLO_OPMODE_FD) {
			op_mode_str = "FD";
		} else {
			op_mode_str = "INV";
		}
		seq_printf(m, "%24s", op_mode_str);
	}
	seq_putc(m, '\n');

	seq_printf(m, "%*s: ", prec, "Registered");
	for (ap_unit = 0; ap_unit < PCIE_IPC_AP_UNIT_MAX; ++ap_unit) {
		seq_printf(m, "%24s", (mlo_ipc_get_mlo_ap_data(ap_unit) != NULL) ? "Yes" : "No");
	}
	seq_putc(m, '\n');

	seq_printf(m, "%*s: ", prec, "MLO Down");
	for (ap_unit = 0; ap_unit < PCIE_IPC_AP_UNIT_MAX; ++ap_unit) {
		seq_printf(m, "%24s", (mlo_ipc_get_sys_mlo_down(ap_unit) == 1) ? "Yes" : "No");
	}
	seq_putc(m, '\n');

	seq_printf(m, "%*s: ", prec, "Disabled");
	for (ap_unit = 0; ap_unit < PCIE_IPC_AP_UNIT_MAX; ++ap_unit) {
		seq_printf(m, "%24s",
			((mlo_ipc_sys->mlo_disable_ap_bm & NBITVAL(ap_unit)) != 0) ? "Yes" : "No");
	}
	seq_putc(m, '\n');

	seq_printf(m, "%*s: ", prec, "MLC STATE");
	for (ap_unit = 0; ap_unit < PCIE_IPC_AP_UNIT_MAX; ++ap_unit) {
		mlo_ipc_mlc_state_t mlc_state;
		mlc_state = mlo_ipc_sys->mlc_state[ap_unit];
		ASSERT(mlc_state < mlo_ipc_mlc_max_e);
		seq_printf(m, "%24s", __mlo_ipc_mlc_state_str[mlc_state]);
	}
	seq_putc(m, '\n');

	return 0;
}

static ssize_t
mlo_ipc_proc_disable_reconfig_read(struct file *file, char __user *ubuf,
	size_t cnt, loff_t *ppos)
{
	char buf[16] = {0};
	int len = 0;
	mlo_ipc_sys_t * mlo_ipc_sys = mlo_ipc_sys_gp;

	if (mlo_ipc_sys == NULL) {
		MLO_IPC_ERROR(("%s: mlo_ipc_sys is NULL\n", __FUNCTION__));
		return 0;
	}

	if ((*ppos > 0) || (cnt < sizeof(buf))) {
		return 0;
	}

	len += snprintf(buf, sizeof(buf), "%d", mlo_ipc_sys->disable_reconfig);

	if (copy_to_user(ubuf, buf, len)) {
		return -EFAULT;
	}

	*ppos = len;

	return len;
}

static ssize_t
mlo_ipc_proc_disable_reconfig_write(struct file *file,
	const char __user *ubuf, size_t cnt, loff_t *ppos)
{
	mlo_ipc_sys_t * mlo_ipc_sys = mlo_ipc_sys_gp;
	char buf[16];
	int len;
	unsigned long val;

	if (mlo_ipc_sys == NULL) {
		MLO_IPC_ERROR(("%s: mlo_ipc_sys is NULL\n", __FUNCTION__));
		return cnt;
	}

	len = min((unsigned long)cnt, (unsigned long)(sizeof(buf) - 1));
	if (copy_from_user(buf, ubuf, len)) {
		MLO_IPC_ERROR(("%s: copy from user failed\n", __FUNCTION__));
		return cnt;
	}

	buf[len] = '\0';
	if (kstrtoul(buf, 0, &val))
		goto fail;

	if ((val != 0) && (val != 1))
		goto fail;

	mlo_ipc_sys->disable_reconfig = (val == 0) ? FALSE : TRUE;

	return cnt;

fail:
	MLO_IPC_ERROR(("%s: invalid input value\n", __FUNCTION__));

	return cnt;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0))
static const struct proc_ops mlo_ipc_proc_disable_reconfig_ops = {
	.proc_read	= mlo_ipc_proc_disable_reconfig_read,
	.proc_write	= mlo_ipc_proc_disable_reconfig_write,
};
#else /* KERNEL < 5.6.0 */
static const struct file_operations mlo_ipc_proc_disable_reconfig_ops = {
	.owner		= THIS_MODULE,
	.read		= mlo_ipc_proc_disable_reconfig_read,
	.write		= mlo_ipc_proc_disable_reconfig_write,
};
#endif /* KERNEL < 5.6.0 */

static struct proc_dir_entry *
mlo_ipc_proc_create(void)
{
	struct proc_dir_entry *proc_dir;

	if ((proc_dir = proc_mkdir(MLO_IPC_PROC_DIR_NAME, NULL)) == NULL) {
		MLO_IPC_ERROR(("%s: created proc dir %s failed\n",
			__FUNCTION__, MLO_IPC_PROC_DIR_NAME));
		goto fail;
	}

	if (proc_create(MLO_IPC_PROC_MLO_CONFIG_NAME, S_IRUGO | S_IWUGO, proc_dir,
		&mlo_ipc_proc_mlo_config_ops) == NULL) {
		MLO_IPC_ERROR(("%s: created proc entry %s failed\n",
			__FUNCTION__, MLO_IPC_PROC_MLO_CONFIG_NAME));
		goto fail;
	}

	if (proc_create(MLO_IPC_PROC_MAIN_NAME, S_IRUGO, proc_dir,
		&mlo_ipc_proc_main_ops) == NULL) {
		MLO_IPC_ERROR(("%s: created proc entry %s failed\n",
			__FUNCTION__, MLO_IPC_PROC_MAIN_NAME));
		goto fail;
	}

	if (proc_create_single(MLO_IPC_PROC_STATUS_NAME, S_IRUGO, proc_dir,
		mlo_ipc_proc_status_show) == NULL) {
		MLO_IPC_ERROR(("%s: created proc entry %s failed\n",
			__FUNCTION__, MLO_IPC_PROC_STATUS_NAME));
		goto fail;
	}

	if (proc_create(MLO_IPC_PROC_DISABLE_RECONFIG_NAME, S_IRUGO | S_IWUGO, proc_dir,
		&mlo_ipc_proc_disable_reconfig_ops) == NULL) {
		MLO_IPC_ERROR(("%s: created proc entry %s failed\n",
			__FUNCTION__, MLO_IPC_PROC_DISABLE_RECONFIG_NAME));
		goto fail;
	}

	return proc_dir;

fail:
	mlo_ipc_proc_destroy(proc_dir);

	return NULL;
}

static void
mlo_ipc_proc_destroy(struct proc_dir_entry *proc_dir)
{
	if (proc_dir == NULL) {
		return;
	}

	proc_remove(proc_dir);
}
#endif /* MLO_IPC_PROC */
