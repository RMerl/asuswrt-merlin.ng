/*
 * +--------------------------------------------------------------------------+
 * MLO_IPC: Inter Processor communication between MLO AP's
 *
 * Copyright (C) 2023, Broadcom. All Rights Reserved.
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
 * $Id: mlo_ipc.c 832812 2023-11-14 01:54:32Z $
 * +--------------------------------------------------------------------------+
 */

#include <typedefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <bcmpcie.h>
#include <mlo_ipc.h>

#include <linuxver.h>
#include <linux/dma-mapping.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0)
#include <linux/dma-contiguous.h>
#else
#include <linux/dma-map-ops.h>
#endif /* KERNEL < 5.10 */

#define MLO_IPC_WAIT_TIMEOUT		5000 /* ms */

static struct device *mlo_ipc_dev_alloc(void);
static void mlo_ipc_dev_free(struct device *mlo_ipc_dev);

// Global instance of MLO IPC context
mlo_ipc_sys_t * mlo_ipc_sys_gp = NULL;
const char * __mlo_ipc_mlc_state_str[mlo_ipc_mlc_max_e] =
{
	"MLC_RESET", "MLC_ATTACH", "MLC_BIND", "MLC_LINK", "MLC_BRIDGE",
	"MLC_SYNC", "MLC_READY", "MLC_TRAP", "MLC_HALT", "MLC_SUSPEND",
	"MLC_SUSPEND_HALT", "MLC_DOWN_REQUEST", "MLC_DOWN", "MLC_DOWN_COMPLETE",
	"MLC_BH_REQUEST", "MLC_BH_HALT_REQUEST", "MLC_BH", "MLC_BH_COMPLETE",
	"MLC_DOWN_BH_REQUEST", "MLC_DOWN_BH", "MLC_DOWN_BH_COMPLETE",
	"MLC_ACTIVE"
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
 * AP with unit number "2" is MAP and AP unit "3" (if exists) is not MLO capable.
 * nvram set wl_mlo_config="1 2 0 -1"
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
*  Cannot have a value 3, or higher
*  May have more than one value -1
*  Cannot have a repetition of any non -1 value
*/

static bool
_is_mlo_config_validated(char *mlo_conf_str, int *nvram_mlo)
{
	uint8 unit = 0, mlo_unit_bmap = 0, mlo_units = 0;
	int dummy;
	bool isMAP = FALSE, isAAP = FALSE;

	/* Must have 4 values */
	if (sscanf(mlo_conf_str, "%d %d %d %d %d",
		&nvram_mlo[0], &nvram_mlo[1], &nvram_mlo[2], &nvram_mlo[3],
		&dummy) != WL_MAX_RADIO) {

		MLO_IPC_ERROR(("%s: wl_mlo_config [%s] must have 4 values\n",
			__FUNCTION__, mlo_conf_str));
		return FALSE;
	}

	/* Cannot have a repetition of any non -1 value */
	for (unit = 0; unit < WL_MAX_RADIO; unit++) {

		 /* Cannot have a value 3, or higher */
		if (!TEST_IN_RANGE(nvram_mlo[unit], WL_MLO_MIN_UNIT,
			WL_MLO_MAX_UNIT)) {

			MLO_IPC_ERROR(("%s: Must have a value in range from %d to %d, "
				"wl_mlo_config[%s]\n", __FUNCTION__, WL_MLO_MIN_UNIT,
				WL_MLO_MAX_UNIT, mlo_conf_str));
			return FALSE;
		}

		/* -1 can be repeated */
		if (nvram_mlo[unit] == WL_MLO_MIN_UNIT) {
			continue;
		}

		/*
		* Must have at least a 0 and a 1 in the 4 values,
		*  i.e. at the minimum must be a dual AP MLO.
		*/

		/* Check MAP */
		if (nvram_mlo[unit] == WL_MLO_DUAL_AP_MAP_UNIT) {
			isMAP = TRUE;
		}

		/* Check AAP1 */
		if (nvram_mlo[unit] == WL_MLO_DUAL_AP_AAP_UNIT) {
			isAAP = TRUE;
		}

		if (mlo_unit_bmap & (1 << nvram_mlo[unit])) {
			MLO_IPC_ERROR(("%s: Cannot have a repetition of any non -1 value, "
				"wl_mlo_config [%s]\n", __FUNCTION__, mlo_conf_str));
			return FALSE;
		}

		mlo_unit_bmap |= (1 << nvram_mlo[unit]);
		mlo_units++;
	}

	if (!mlo_units) {
		return TRUE;
	}

	if (!isMAP || !isAAP) {
		MLO_IPC_ERROR(("%s: Must have at least a 0 and a 1, "
			"wl_mlo_config [%s]\n", __FUNCTION__, mlo_conf_str));
		return FALSE;
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
	return -1;
}

/* Get the MLO link count */
int mlo_ipc_get_mlink_count(void)
{
	mlo_ipc_sys_t   * mlo_ipc_sys;
	pcie_ipc_mlo_sys_t *sys;

	mlo_ipc_sys = mlo_ipc_sys_gp;

	if ((mlo_ipc_sys != NULL) && (mlo_ipc_sys->pcie_ipc_mlo != NULL)) {
		sys = &mlo_ipc_sys->pcie_ipc_mlo->sys;
		return sys->mlo_total;
	}
	return 0;
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

	MLO_IPC_DEBUG(("%s: ENTER\n", __FUNCTION__));

	// +--- Fetch and parse nvram or default
	{
		char * wl_mlo_config_str = _get_wl_mlo_config_str();

		if (!_is_mlo_config_validated(wl_mlo_config_str, nvram_mlo)) {
#if defined(BCMDBG)
			ASSERT(0);
#endif

			wl_mlo_config_str = WL_MLO_DEFAULT;
			return BCME_OK;
		}
	}

	// +--- Audit nvram: determine ap_unit_map, mlo_total
	for (ap_unit = 0; ap_unit < PCIE_IPC_AP_UNIT_MAX; ++ap_unit)
	{
		if (nvram_mlo[ap_unit] == (int)PCIE_IPC_MLO_UNIT_INV)
			continue; // AP does not participate in MLO

		if (nvram_mlo[ap_unit] == (int)PCIE_IPC_MLO_UNIT_MAP)
		{
			ap_unit_map = (int8)ap_unit; // MAP's AP unit
		}

		mlo_ap_bm |= (1 << ap_unit);
		++mlo_total; // count of total WLAN APs that participate in MLO
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
			pcie_ipc_mlo->sys.mlo_unit[ap_unit] = (int8)nvram_mlo[ap_unit];

			/* If WLAN radio is not place into a PCIE slot then per-AP MLO
			 * attributes will not be set. So Reset AP and MLO unit for sanity.
			 */
			pcie_ipc_mlo->ap[ap_unit].ap_unit = (uint8)PCIE_IPC_AP_UNIT_INV;
			pcie_ipc_mlo->ap[ap_unit].mlo_unit = (int8)PCIE_IPC_MLO_UNIT_INV;
		}

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
		evt_queue->wake_up_cb = NULL;
	}

	mlo_ipc_sys->ap_unit_map = ap_unit_map;
	mlo_ipc_sys->dump_signature = (uint16)OSL_RAND();

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

	// Free PCIE IPC for MLO
	if (mlo_ipc_sys->pcie_ipc_mlo != NULL) {
		DMA_FREE_CONSISTENT(mlo_ipc_sys->osh, (void *)mlo_ipc_sys->pcie_ipc_mlo,
			mlo_ipc_sys->pcie_ipc_mlo_alloced, mlo_ipc_sys->pcie_ipc_mlo_pa, 0);
		mlo_ipc_sys->pcie_ipc_mlo = NULL;
	}

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
mlo_ipc_map_register(int ap_unit, mlo_ipc_map_msg_enqueue_fn_t msg_enqueue_fn,
	mlo_ipc_evt_wake_up_cb_t wake_up_cb, void * data, pcie_ipc_mlo_opmode_t op_mode)
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

	mlo_ipc_sys->msg_enqueue_fn = msg_enqueue_fn;
	mlo_ipc_sys->map_data = data;
	mlo_ipc_sys->aap_data[ap_unit] = data;

	mlo_ipc_sys->evt_queue[ap_unit].wake_up_cb = wake_up_cb;
	mlo_ipc_sys->pcie_ipc_mlo->sys.op_mode[ap_unit] = op_mode;

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

	mlo_ipc_sys->msg_enqueue_fn = NULL;
	mlo_ipc_sys->map_data = NULL;
	mlo_ipc_sys->aap_data[ap_unit] = NULL;
	mlo_ipc_clear_ap_bm(ap_unit);

} /* mlo_ipc_map_deregister() */

void // Register handler for Auxillary AP
mlo_ipc_aap_register(int ap_unit, mlo_ipc_aap_state_change_cb_t state_change_cb,
	mlo_ipc_evt_wake_up_cb_t wake_up_cb, void * data, pcie_ipc_mlo_opmode_t op_mode)
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

	mlo_ipc_sys->state_change_cb[ap_unit] = state_change_cb;
	mlo_ipc_sys->aap_data[ap_unit] = data;

	mlo_ipc_sys->evt_queue[ap_unit].wake_up_cb = wake_up_cb;
	mlo_ipc_sys->pcie_ipc_mlo->sys.op_mode[ap_unit] = op_mode;

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

	mlo_ipc_sys->state_change_cb[ap_unit] = NULL;
	mlo_ipc_sys->aap_data[ap_unit] = NULL;
	mlo_ipc_clear_ap_bm(ap_unit);

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

	mlo_ipc_sys = mlo_ipc_sys_gp;

	return mlo_ipc_sys->map_data;
}

void *
mlo_ipc_get_aap_data(int ap_unit)
{
	mlo_ipc_sys_t	* mlo_ipc_sys;

	mlo_ipc_sys = mlo_ipc_sys_gp;

	return mlo_ipc_sys->aap_data[ap_unit];
}

mlo_ipc_map_msg_enqueue_fn_t
mlo_ipc_get_map_msg_enqueue_fn(void)
{
	mlo_ipc_sys_t	* mlo_ipc_sys;

	mlo_ipc_sys = mlo_ipc_sys_gp;

	return mlo_ipc_sys->msg_enqueue_fn;
}

mlo_ipc_aap_state_change_cb_t
mlo_ipc_get_state_change_cb(int ap_unit)
{
	mlo_ipc_sys_t	* mlo_ipc_sys;

	mlo_ipc_sys = mlo_ipc_sys_gp;

	return mlo_ipc_sys->state_change_cb[ap_unit];
}

uint8
mlo_ipc_get_sys_ap_bm(void)
{
	mlo_ipc_sys_t	* mlo_ipc_sys;

	mlo_ipc_sys = mlo_ipc_sys_gp;

	return mlo_ipc_sys->pcie_ipc_mlo->sys.mlo_ap_bm;
}

/**
 * -----------------------------------------------------------------------------------------------
 *  Section: MLO_IPC_EVENT
 * -----------------------------------------------------------------------------------------------
 */

#define MLO_IPC_EVT_QUEUE_MAX_CNT	512

/* Enqueues an event message to consumer link identified by AP unit number.
 * and informs consumer using registered wakeup callback.
 */
int
mlo_ipc_evt_queue_enqueue(uint16 dest_ap_unit, mlo_ipc_evt_msg_t * evt_msg)
{
	mlo_ipc_evt_queue_t * evt_queue;
	int	ret = BCME_OK;

	evt_queue = MLO_IPC_EVT_QUEUE(dest_ap_unit);

	MLO_IPC_EVT_QUEUE_LOCK(dest_ap_unit); // +++++++++++++++++++++++++++++++++++++++++++++++++

	if (evt_queue->active_cnt < MLO_IPC_EVT_QUEUE_MAX_CNT) {
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
		ret = BCME_NORESOURCE;
	}

	MLO_IPC_EVT_QUEUE_UNLK(dest_ap_unit); // -------------------------------------------------

	evt_queue->wake_up_cb(mlo_ipc_sys_gp->aap_data[dest_ap_unit]);

	return ret;
} /* mlo_ipc_evt_queue_enqueue() */

/* Dequeues an event message from consumer link event queue identified by AP unit number. */
mlo_ipc_evt_msg_t *
mlo_ipc_evt_queue_dequeue(uint16 ap_unit)
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
	}

	MLO_IPC_EVT_QUEUE_UNLK(ap_unit); // ------------------------------------------------

	return evt_msg;
} /* mlo_ipc_evt_queue_dequeue() */

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
