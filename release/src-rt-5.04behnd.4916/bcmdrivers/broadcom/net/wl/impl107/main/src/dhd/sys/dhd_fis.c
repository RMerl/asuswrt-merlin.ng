/*
 * FIS (Force Immediate Save) Functions for DHD
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
 * $Id$
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sched.h>

#include <typedefs.h>
#include <linuxver.h>
#include <siutils.h>
#include <bcmutils.h>
#include <bcmdevs.h>
#include <bcmendian.h>
#include <hndpmu.h>
#include <sreng_regs.h>
#include <dhdioctl.h>
#include <pcie_core.h>
#include <dhd.h>
#include <dhd_fis.h>
#include <sdiovar.h>
#include <sbchipc.h>
#include <dhd_linux.h>
#include <dhd_pcie.h>
#include <dhd_bus.h>
#include <sbsysmem.h>
#include <dhd_dbg.h>
#include <wl_core.h>
#include <pcicfg.h>
#include <osl.h>
#include <dhd_fis_asm.h>

#if defined(MLO_IPC)
#include <dhd_mlo_ipc.h>
#include <mlo_ipc.h>
#include <bcmpcie.h>
#endif /* MLO_IPC */

#define ONE_MS_DELAY		1000u

#define ONE_SEC_DELAY		(1000u * 1000u)

#define FIS_TRIG_DEFER_DELAY	(500u * ONE_MS_DELAY)
#define FIS_TRIG_DEFER_LOOP	14u

#define PCIE_SAVE_BARS		6u

#define FIS_DONE_DELAY_MIN	3u
#define FIS_DONE_DELAY		FIS_DONE_DELAY_MIN

#define DATA_BUFSZ              128u
#define DUMP_BASE_PATH          "/tmp"
#define IFNAME_STR              "wl%d"
#define ASM_STR_BUF_SZ          16u
#define DUMP_PREFIX_STR_LEN     8u

void dhd_fis_dump(dhd_pub_t *dhd, bool asm_dump, bool is_iovar);
int dhd_fis_reset(dhd_pub_t *dhd, bool force_reset);
int dhd_fis_trigger(dhd_pub_t *dhd, int trigger_type, bool is_iovar);
int dhd_fis_sr_init(dhd_pub_t *dhd);
int dhd_fis_asm_load(dhd_pub_t *dhd, int core, bool is_iovar);
int dhd_fis_enable_sreng(dhd_pub_t *dhd);
int dhd_fis_trigger_and_dump(dhd_pub_t *dhd, int core, int trigger_type);

char *asm_core_type_str[] = {
	"NA", "cpusub", "dig", "phy12",
	"rx1", "rx2", "rx4", "d11mac1",
	"d11mac2", "chip_all", "chip_wo_dig",
	"chip_mac_phy"
};

typedef struct _save_pcie_regs {
	uint32 cfgcmd;
	uint32 bar0;
	uint32 bar1;
	uint32 bar2;
	uint32 bar3;
	uint32 bar4;
	uint32 bar5;
} save_pcie_regs_t;

save_pcie_regs_t save_pcie_regs;

static void
dhd_fis_save_pcie_regs(dhd_pub_t *dhd)
{
	osl_t *osh = dhd->bus->osh;

	save_pcie_regs.cfgcmd = OSL_PCI_READ_CONFIG(osh, PCI_CFG_CMD, sizeof(uint32));
	save_pcie_regs.bar0 = OSL_PCI_READ_CONFIG(osh, PCI_CFG_BAR0, sizeof(uint32));
	save_pcie_regs.bar1 = OSL_PCI_READ_CONFIG(osh, PCI_CFG_BAR1, sizeof(uint32));
	save_pcie_regs.bar2 = OSL_PCI_READ_CONFIG(osh, PCI_CFG_BAR2, sizeof(uint32));
	save_pcie_regs.bar3 = OSL_PCI_READ_CONFIG(osh, PCI_CFG_BAR3, sizeof(uint32));
	save_pcie_regs.bar4 = OSL_PCI_READ_CONFIG(osh, PCI_CFG_BAR4, sizeof(uint32));
	save_pcie_regs.bar5 = OSL_PCI_READ_CONFIG(osh, PCI_CFG_BAR5, sizeof(uint32));
}

static void
dhd_fis_restore_pcie_regs(dhd_pub_t *dhd)
{
	osl_t *osh = dhd->bus->osh;

	OSL_PCI_WRITE_CONFIG(osh, PCI_CFG_CMD, sizeof(uint32), save_pcie_regs.cfgcmd);
	OSL_PCI_WRITE_CONFIG(osh, PCI_CFG_BAR0, sizeof(uint32), save_pcie_regs.bar0);
	OSL_PCI_WRITE_CONFIG(osh, PCI_CFG_BAR1, sizeof(uint32), save_pcie_regs.bar1);
	OSL_PCI_WRITE_CONFIG(osh, PCI_CFG_BAR2, sizeof(uint32), save_pcie_regs.bar2);
	OSL_PCI_WRITE_CONFIG(osh, PCI_CFG_BAR3, sizeof(uint32), save_pcie_regs.bar3);
	OSL_PCI_WRITE_CONFIG(osh, PCI_CFG_BAR4, sizeof(uint32), save_pcie_regs.bar4);
	OSL_PCI_WRITE_CONFIG(osh, PCI_CFG_BAR5, sizeof(uint32), save_pcie_regs.bar5);
}

static bool
dhd_fis_is_cap(dhd_pub_t *dhd)
{
	char *fis_cap = NULL;

	if (!strlen(dhd->wlcore->fw_capabilities)) {
		return FALSE;
	}

	fis_cap = bcmstrstr(dhd->wlcore->fw_capabilities, "fisdump");
	if (fis_cap) {
		return TRUE;
	}

	DHD_ERROR(("%s: " IFNAME_STR ": fis capability isn't in fw cap\n", __FUNCTION__,
		dhd->unit));
	return FALSE;
}

static void
dhd_fis_asm_core_type_to_str(int core_type, char *str, int len)
{
	strlcpy(str, asm_core_type_str[core_type], len - 1);
}

static void
dhd_fis_write(dhd_pub_t *dhd, char *fname, uint32 addr, uint32 size)
{
	uint32 offset, val, len = 0;
	unsigned long tot_size = 0u;
	char buf[DATA_BUFSZ] = {0, };

	/* safe to do UNLOCK/LOCK dance */
	DHD_PERIM_ASSERT(dhd);
	for (offset = 0; offset <= size; offset += sizeof(uint32)) {
		if (dhd->fis.deinit) {
			return;
		}

		ASSERT(OSL_PCI_READ_CONFIG(dhd->bus->osh, PCI_BAR1_WIN, sizeof(uint32)) == 0);
		dhdpcie_bus_read_sysmem(dhd->bus, (addr + offset), (uint8 *)&val,
			sizeof(uint32));

		snprintf(buf, sizeof(buf), "Address 0x%06x :: 0x%08x\n", (addr + offset), val);
		len = strlen(buf);
		tot_size += len;
		DHD_UNLOCK(dhd);
		dhd_write_file(dhd, fname, buf, len);
		DHD_LOCK(dhd);
	}
	DHD_ERROR(("%s: " IFNAME_STR ":%s (%lu bytes) created done\n", __FUNCTION__, dhd->unit,
		fname, tot_size));
}

static void
dhd_fis_create_dump(dhd_pub_t *dhd, bool is_asm_dump)
{
	uint32 data_addr, data_size, asm_size;
	char asm_str[ASM_STR_BUF_SZ] = {0, };
	char dumpfilename[DUMP_FILENAME_SZ + 1] = {0, };
	char asmdumpfname[DUMP_FILENAME_SZ + 1] = {0, };
	char prefix[DUMP_PREFIX_STR_LEN] = {0, };
	uint16 dump_signature = dhd_get_dump_signature(dhd);

	if (dhd->fis.fis_status > SR_ENABLED) {
		strlcpy(prefix, "after", DUMP_PREFIX_STR_LEN - 1);
	} else {
		strlcpy(prefix, "before", DUMP_PREFIX_STR_LEN - 1);
	}

	dhd_fis_asm_core_type_to_str(dhd->fis.cur_asm, asm_str, sizeof(asm_str));

#ifdef BCM_ROUTER
	if (!dhd->fis.fis_trig_iovar) {
		snprintf(dumpfilename, DUMP_FILENAME_SZ,
			DUMP_BASE_PATH "/" IFNAME_STR "/dump_fis_data_" IFNAME_STR "_%04x.txt",
			dhd->unit, dhd->unit, dump_signature);

		snprintf(asmdumpfname, DUMP_FILENAME_SZ,
			DUMP_BASE_PATH "/" IFNAME_STR "/dump_fis_asm_" IFNAME_STR "_%04x.txt",
			dhd->unit, dhd->unit, dump_signature);
	} else {
		snprintf(dumpfilename, DUMP_FILENAME_SZ,
			DUMP_BASE_PATH "/" IFNAME_STR "/dump_%s_fis_%s_" IFNAME_STR "_%04x.txt",
			dhd->unit, prefix, asm_str, dhd->unit, dump_signature);
	}
#else
	if (!dhd->fis.fis_trig_iovar) {
		snprintf(dumpfilename, DUMP_FILENAME_SZ,
			DUMP_BASE_PATH "/dump_fis_data_" IFNAME_STR "_%04x.txt", dhd->unit,
			dump_signature);

		snprintf(asmdumpfname, DUMP_FILENAME_SZ,
			DUMP_BASE_PATH "/dump_fis_asm_" IFNAME_STR "_%04x.txt", dhd->unit,
			dump_signature);
	} else {
		snprintf(dumpfilename, DUMP_FILENAME_SZ,
			DUMP_BASE_PATH "/dump_%s_fis_%s_" IFNAME_STR "_%04x.txt", dhd->unit, prefix,
			asm_str, dump_signature);
	}
#endif /* BCM_ROUTER */

	if (is_asm_dump) {
		if (dhd->fis.cur_asm >= CHIP_ALL) {
			data_addr =  ALL_DATA_START_ADDR;

			if (dhd->fis.cur_asm == CHIP_ALL) {
				data_size = ALL_DATA_SIZE;
			} else {
				data_size = ALL_DATA_SIZE_WO_DIG;
			}

			asm_size = ASM_ALL_SIZE;
		} else {
			data_addr = CORE_DATA_START_ADDR;
			data_size = CORE_DATA_SIZE;
			asm_size = ASM_CORE_SIZE;
		}

		DHD_ERROR(("%s: " IFNAME_STR ": fis asm dump creating from 0x%x to 0x%x "
			"(size %d bytes) for %s core\n", __FUNCTION__, dhd->unit,
			ASM_START_ADDR,	(ASM_START_ADDR + asm_size), asm_size, asm_str));

		dhd_fis_write(dhd, asmdumpfname, ASM_START_ADDR, asm_size);

	} else {
		if (dhd->fis.cur_asm >= CHIP_ALL) {
			data_addr = ALL_DATA_START_ADDR;

			if (dhd->fis.cur_asm == CHIP_ALL) {
				data_size = ALL_DATA_SIZE;
			} else {
				data_size = ALL_DATA_SIZE_WO_DIG;
			}
		} else {
			data_addr = CORE_DATA_START_ADDR;
			data_size = CORE_DATA_SIZE;
		}
	}

	DHD_ERROR(("%s: " IFNAME_STR ": fis dump creating from 0x%x to 0x%x (size %d bytes) "
		"for %s core\n", __FUNCTION__, dhd->unit, data_addr, (data_addr + data_size),
		data_size, asm_str));

	dhd_fis_write(dhd, dumpfilename, data_addr, data_size);

}

void
dhd_fis_init(dhd_pub_t *dhd)
{
	dhd->fis.fis_status = SR_NOT_INIT_YET;
	dhd->fis.delay = FIS_DONE_DELAY;
	dhd->fis.cur_asm = CHIP_MAC_PHY;
	dhd->fis.deinit = FALSE;
}

void
dhd_fis_reset_status(dhd_pub_t *dhd)
{
	if (dhd->fis.fis_status == SR_NOT_INIT_YET) {
		return;
	}

	dhd->fis.fis_status = SR_NOT_INIT_YET;
	dhd->fis.delay = FIS_DONE_DELAY;
	dhd->fis.cur_asm = CHIP_MAC_PHY;
	dhd->fis.deinit = FALSE;
	dhd->fis.fis_trig_iovar = FALSE;
}

void
dhd_fis_deinit(dhd_pub_t *dhd)
{
	char *val = getvar(NULL, "fis_dump_disable");
	if (val && (bcm_atoi(val) == 1)) {
		return;
	}

	DHD_PERIM_ASSERT(dhd);

	dhd->fis.deinit = TRUE;

	if (dhd->fis.fis_status == SR_FIS_TRIG) {
		dhd_fis_reset(dhd, TRUE);
	}
}

void
dhd_fis_dump(dhd_pub_t *dhd, bool asm_dump, bool is_iovar)
{
	dhd->fis.fis_trig_iovar = is_iovar;

	if (dhd->fis.fis_status > SR_ENABLED) {
		DHD_ERROR(("%s: " IFNAME_STR ": create a dump after FIS\n", __FUNCTION__,
			dhd->unit));
		dhd_fis_create_dump(dhd, asm_dump);
		/* reset all status after dump */
		dhd_fis_reset_status(dhd);
	} else {
		DHD_ERROR(("%s: " IFNAME_STR ": create a dump before FIS\n", __FUNCTION__,
			dhd->unit));
		dhd_fis_create_dump(dhd, asm_dump);
	}
}

int
dhd_fis_reset(dhd_pub_t *dhd, bool force_reset)
{
	uint32 save_idx, fis_ctrl_status = 0;
	int ret = BCME_OK;
	si_t *sih = dhd->bus->sih;
	save_idx = si_coreid(sih);

	if (!force_reset &&
		(dhd->fis.fis_status != SR_FIS_TRIG)) {
		DHD_ERROR(("%s: " IFNAME_STR ": failed to reset, status = %d\n", __FUNCTION__,
			dhd->unit, dhd->fis.fis_status));
		return BCME_ERROR;
	}

	/*
	 * For DIG or CHIP_ALL,
	 * a kernel panic can occur in the case of access to a register
	 * without chip reset.
	 */
	if (dhd->fis.cur_asm != DIG && dhd->fis.cur_asm != CHIP_ALL) {
		fis_ctrl_status = PMU_REG(dhd->bus->sih, fis_ctrl_status, 0, 0);
		if (fis_ctrl_status & PMU_CLEAR_FIS_DONE_MASK) {
			DHD_ERROR(("%s: " IFNAME_STR ": FIS Done bit set\n", __FUNCTION__,
				dhd->unit));
		} else {
			DHD_ERROR(("%s: " IFNAME_STR ": FIS Done bit not set\n", __FUNCTION__,
				dhd->unit));
			return BCME_ERROR;
		}
	}

	/*
	 * DIG  needs to reset PCIE to get a dump
	 * Due to HW limitation, hard reset is required after FIS dump.
	 * If not, system crash happens because ChipId can't be read.
	 * In the case of CHIP_ALL, it triggers all the cores including DIG.
	 * For CHIP_ALL, hard reset is required (toggle power switch).
	 */
	if (dhd->fis.cur_asm == DIG || dhd->fis.cur_asm == CHIP_ALL) {
		dhd_bus_fis_pcie_perstb_reset(dhd);

		/* CFG CMD and BAR0~5 registers are required to be restored. */
		dhd_fis_restore_pcie_regs(dhd);

		DHD_ERROR(("%s: " IFNAME_STR ": Hard reset (power cycle) is required "
			"after FIS dump\n", __FUNCTION__, dhd->unit));
	}

	si_pmu_fis_reset_fis_status(sih);

	if (si_setcore(sih, SYSMEM_CORE_ID, 0)) {
		si_core_reset(sih, 0, 0);
	} else {
		DHD_ERROR(("%s: " IFNAME_STR ": failed to find sysmem core\n", __FUNCTION__,
			dhd->unit));
		ret = BCME_ERROR;
	}

	si_setcore(sih, save_idx, 0);

	dhd->fis.fis_status = SR_FIS_RESET;

	DHD_ERROR(("%s: " IFNAME_STR ": Done with recovery from FIS\n", __FUNCTION__,
		dhd->unit));

	return ret;
}

int
dhd_fis_trigger(dhd_pub_t *dhd, int trigger_type, bool is_iovar)
{
	uint32 delay, cnt = 0;
	uint32 fis_delay = FIS_DONE_DELAY * ONE_SEC_DELAY;
	uint32 fis_cnt_delay = 100u * ONE_MS_DELAY;
	char *nv_val = NULL;
	si_t *sih = dhd->bus->sih;

	if (!sih) {
		DHD_ERROR(("%s: " IFNAME_STR ": sih is null\n", __FUNCTION__, dhd->unit));
		return BCME_ERROR;
	}

	if (dhd->fis.fis_status != SR_ENABLED) {
		DHD_ERROR(("%s: " IFNAME_STR ": Failed to trigger FIS, status = %d\n", __FUNCTION__,
			dhd->unit, dhd->fis.fis_status));
		return BCME_ERROR;
	}

	if (dhd->fis.deinit) {
		DHD_ERROR(("%s: Don't trigger FIS during deinit on " IFNAME_STR "\n",
			__FUNCTION__, dhd->unit));
		return BCME_ERROR;
	}

	dhd->fis.fis_trig_iovar = is_iovar;

	if (dhd->fis.cur_asm == DIG || dhd->fis.cur_asm == CHIP_ALL) {
	/* Before fis, CFGCMD and BAR0~5 are needed to be saved */
		dhd_fis_save_pcie_regs(dhd);
	}

	si_pmu_fis_res_table_setup(sih);

	/* at least 3 sec is required to complete FIS */
	nv_val = getvar(NULL, "fis_done_delay");
	if (nv_val) {
		delay = bcm_atoi(nv_val);
		if (delay >= FIS_DONE_DELAY_MIN) {
			/* msec unit */
			fis_delay = delay * ONE_SEC_DELAY;
		}
	} else {
		if (dhd->fis.delay >= FIS_DONE_DELAY_MIN) {
			fis_delay = dhd->fis.delay * ONE_SEC_DELAY;
		}
	}

	switch (trigger_type) {
		case PMU_FIS_TRIGGER:
			DHD_ERROR(("%s: PMU FIS Trigger to " IFNAME_STR "\n", __FUNCTION__,
				dhd->unit));
			si_pmu_fis_trigger_by_pmu(sih);
			break;

		case PCIE_FIS_TRIGGER:
			DHD_ERROR(("%s: PCIE FIS trigger to " IFNAME_STR "\n", __FUNCTION__,
				dhd->unit));

			if (si_fis_trigger_by_pcie(sih)) {
				DHD_ERROR(("%s: " IFNAME_STR ": Failed to find PCIE core\n",
					__FUNCTION__, dhd->unit));
				return BCME_ERROR;
			}
			break;

		case MAC_FIS_TRIGGER:
			DHD_ERROR(("%s: MAC FIS trigger to " IFNAME_STR "\n", __FUNCTION__,
				dhd->unit));

			if (si_fis_trigger_by_mac_phy(sih, TRUE)) {
				DHD_ERROR(("%s: " IFNAME_STR ": Failed to find D11 core\n",
					__FUNCTION__, dhd->unit));
				return BCME_ERROR;
			}
			break;

		case PHY_FIS_TRIGGER:
			DHD_ERROR(("%s: PHY FIS trigger to " IFNAME_STR "\n", __FUNCTION__,
				dhd->unit));

			if (si_fis_trigger_by_mac_phy(sih, FALSE)) {
				DHD_ERROR(("%s: " IFNAME_STR ": Failed to find D11 core\n",
					__FUNCTION__, dhd->unit));
				return BCME_ERROR;
			}
			break;

		default:
			DHD_ERROR(("%s: Invalid trigger type %d to " IFNAME_STR "\n", __FUNCTION__,
				trigger_type, dhd->unit));
			return BCME_ERROR;
	}

	dhd->fis.fis_status = SR_FIS_TRIG;

	DHD_ERROR(("%s: fis is in progress on " IFNAME_STR "\n", __FUNCTION__, dhd->unit));

	while ((cnt * fis_cnt_delay) <= fis_delay) {
		OSL_DELAY(fis_cnt_delay);

		if (++cnt % 10) {
			DHD_ERROR(("........"));
		} else {
			DHD_ERROR(("\n"));
		}
	}

	DHD_ERROR(("%s: fis trigger done to " IFNAME_STR "\n", __FUNCTION__, dhd->unit));
	return BCME_OK;
}

int
dhd_fis_sr_init(dhd_pub_t *dhd)
{
	si_t *sih = dhd->bus->sih;

	if (!sih) {
		DHD_ERROR(("%s: " IFNAME_STR ": sih is null\n", __FUNCTION__, dhd->unit));
		return BCME_ERROR;
	}

	if (dhd->fis.deinit) {
		DHD_ERROR(("%s: Don't trigger FIS during deinit on " IFNAME_STR "\n",
			__FUNCTION__, dhd->unit));
		return BCME_ERROR;
	}

	if (!dhd_fis_is_cap(dhd)) {
		return BCME_ERROR;
	}

	if (si_sreng_reg_init(sih)) {
		DHD_ERROR(("%s: " IFNAME_STR ": Failed to find SR core\n", __FUNCTION__,
			dhd->unit));
		return BCME_ERROR;
	}

	si_pmu_fis_setup(sih);

	dhd->fis.fis_status = SR_DISABLED;

	return BCME_OK;
}

int
dhd_fis_asm_load(dhd_pub_t* dhd, int core, bool is_iovar)
{
	srreg_t *sr_regs;
	uint32 save_idx, addr, len, val;
	uint32 *asm_array;
	int ret = BCME_OK, i;
	char asm_str[ASM_STR_BUF_SZ] = {0, };
	si_t *sih = dhd->bus->sih;

	if (!sih) {
		DHD_ERROR(("%s: " IFNAME_STR ": sih is null\n", __FUNCTION__, dhd->unit));
		return BCME_ERROR;
	}

	/*
	 * If mlo is configured, all the interfaces joining mls can report traps.
	 * Then if fis is needed to be tested, please reset wl_mlo_config.
	 */
#if defined(MLO_IPC)
	if (is_iovar && dhd_is_mlo_ap(dhd)) {
		DHD_ERROR(("%s: " IFNAME_STR ": don't trigger FIS under mlo\n", __FUNCTION__,
			dhd->unit));
		return BCME_ERROR;
	}
#endif /* MLO_IPC */

	if (dhd->fis.deinit) {
		DHD_ERROR(("%s: Don't trigger FIS during deinit on " IFNAME_STR "\n",
			__FUNCTION__, dhd->unit));
		return BCME_ERROR;
	}

	if (dhd->fis.fis_status == SR_NOT_INIT_YET) {
		if (dhd_fis_sr_init(dhd) != BCME_OK) {
			DHD_ERROR(("%s: " IFNAME_STR ": failed to SR init\n", __FUNCTION__,
				dhd->unit));
			return BCME_ERROR;
		}
	}

	if (dhd->fis.fis_status >= SR_ASM_LOAD) {
		DHD_ERROR(("%s: status = %d, can't load ASM to " IFNAME_STR "\n", __FUNCTION__,
			dhd->unit, dhd->fis.fis_status));
		return BCME_ERROR;
	}

	save_idx = si_coreidx(sih);
	sr_regs = (srreg_t *)si_setcore(sih, SR_CORE_ID, 0);

	if (!sr_regs) {
		/* Return to the previous core */
		si_setcoreidx(sih, save_idx);
		DHD_ERROR(("%s: " IFNAME_STR ": Failed to set SR engine core\n", __FUNCTION__,
			dhd->unit));
		return BCME_ERROR;
	}

	if (dhd->busstate != DHD_BUS_DOWN) {
		if (is_iovar) {
			ret = dhd_wl_ioctl_set_intiovar(dhd, "fis_ucode_stop", 0, WLC_SET_VAR,
				TRUE, 0);
		} else {
			DHD_LOCK(dhd);
			ret = dhd_wl_ioctl_set_intiovar(dhd, "fis_ucode_stop", 0, WLC_SET_VAR,
				TRUE, 0);
			DHD_UNLOCK(dhd);
		}
	}

	if (ret) {
		si_setcoreidx(sih, save_idx);
		DHD_ERROR(("%s: ret = %d, failed to stop ucode for " IFNAME_STR "\n", __FUNCTION__,
			ret, dhd->unit));
		return ret;
	}

	dhd->busstate = DHD_BUS_DOWN;

	switch (core) {
		case CPUSUB:
			asm_array = asm_array_cpusub;
			len = sizeof(asm_array_cpusub) / sizeof(uint32);
			break;

		case DIG:
			asm_array = asm_array_dig;
			len = sizeof(asm_array_dig) / sizeof(uint32);
			break;

		case PHY1PHY2:
			asm_array = asm_array_phy12;
			len = sizeof(asm_array_phy12) / sizeof(uint32);
			break;

		case PHYRX1:
			asm_array = asm_array_rx1;
			len = sizeof(asm_array_rx1) / sizeof(uint32);
			break;

		case PHYRX2:
			asm_array = asm_array_rx2;
			len = sizeof(asm_array_rx2) / sizeof(uint32);
			break;

		case PHYRX4:
			asm_array = asm_array_rx4;
			len = sizeof(asm_array_rx4) / sizeof(uint32);
			break;

		case D11MAC1:
			asm_array = asm_array_d11mac1;
			len = sizeof(asm_array_d11mac1) / sizeof(uint32);
			break;

		case D11MAC2:
			asm_array = asm_array_d11mac2;
			len = sizeof(asm_array_d11mac2) / sizeof(uint32);
			break;

		case CHIP_ALL:
			asm_array = asm_array_all;
			len = sizeof(asm_array_all) / sizeof(uint32);
			break;

		case CHIP_WO_DIG:
			asm_array = asm_array_all_wo_dig;
			len = sizeof(asm_array_all_wo_dig) / sizeof(uint32);
			break;

		case CHIP_MAC_PHY:
			asm_array = asm_array_all_mac_phy_only;
			len = sizeof(asm_array_all_mac_phy_only) / sizeof(uint32);
			break;

		default:
			asm_array = asm_array_all_mac_phy_only;
			len = sizeof(asm_array_all_mac_phy_only) / sizeof(uint32);
			core = CHIP_MAC_PHY;
			break;
	}

	addr = ASM_START_ADDR;
	for (i = 0; i < len; i++) {
		val = asm_array[i];
		dhdpcie_bus_write_sysmem(dhd->bus, (addr + i * sizeof(uint32)),
			(uint8 *)&val, sizeof(uint32));
	}

	dhd->fis.cur_asm = core;
	dhd->fis.fis_status = SR_ASM_LOAD;

	si_setcoreidx(sih, save_idx);

	dhd_fis_asm_core_type_to_str(dhd->fis.cur_asm, asm_str, sizeof(asm_str));
	DHD_ERROR(("%s: " IFNAME_STR ": load ASM to 0x%x, core (%s) = %d\n", __FUNCTION__,
		dhd->unit, ASM_START_ADDR, asm_str, dhd->fis.cur_asm));

	return BCME_OK;
}

int
dhd_fis_enable_sreng(dhd_pub_t* dhd)
{
	si_t *sih = dhd->bus->sih;

	if (!sih) {
		DHD_ERROR(("%s: " IFNAME_STR ": sih is null\n", __FUNCTION__, dhd->unit));
		return BCME_ERROR;
	}

	if (dhd->fis.deinit) {
		DHD_ERROR(("%s: Don't trigger FIS during deinit on " IFNAME_STR "\n",
			__FUNCTION__, dhd->unit));
		return BCME_ERROR;
	}

	if (dhd->fis.fis_status != SR_ASM_LOAD) {
		DHD_ERROR(("%s: " IFNAME_STR "asm load first\n", __FUNCTION__, dhd->unit));
		return BCME_ERROR;
	}

	if (si_sreng_enable(sih, TRUE)) {
		DHD_ERROR(("%s: " IFNAME_STR ": Failed to set SR engine core\n", __FUNCTION__,
			dhd->unit));
		return BCME_ERROR;
	}

	dhd->fis.fis_status = SR_ENABLED;

	DHD_ERROR(("%s: " IFNAME_STR ": SR enabled\n", __FUNCTION__, dhd->unit));

	return BCME_OK;
}

int
dhd_fis_trigger_and_dump(dhd_pub_t* dhd, int core, int trigger_type)
{
	char *nv_val = NULL;
	bool is_asmdump = FALSE;
	int ret = BCME_OK;
#if defined(MLO_IPC)
	int ap_unit, loop_cnt = 0, mlo_trap_ap_bm = 0;
	uint8 mlo_ap_bm = 0, mlo_active_ap_bm = 0;
#endif /* MLO_IPC */

	DHD_LOCK(dhd);

	if (dhd->fis.fis_status > SR_ENABLED) {
		DHD_ERROR(("%s: FIS already triggered to " IFNAME_STR ", status = %d\n",
			__FUNCTION__, dhd->unit, dhd->fis.fis_status));
		ret = BCME_ERROR;
		goto exit;
	}

	if (dhd->fis.deinit) {
		DHD_ERROR(("%s: Don't trigger FIS during deinit on " IFNAME_STR "\n",
			__FUNCTION__, dhd->unit));
		ret = BCME_ERROR;
		goto exit;
	}

	if (dhd->fis.fis_status != SR_NOT_INIT_YET) {
		DHD_ERROR(("%s: " IFNAME_STR ": reset status = %d\n", __FUNCTION__, dhd->unit,
			dhd->fis.fis_status));
		dhd_fis_reset_status(dhd);
	}

#if defined(MLO_IPC)
	/* We need to defer FIS until all mlo interfaces are trapped */
	if (dhd_is_mlo_ap(dhd)) {
		/* mlo_ap_bm is currently configured by driver from nvram */
		mlo_ap_bm = mlo_ipc_get_ap_bm();
		/* mlo_active_bm is expecting configuration. */
		mlo_active_ap_bm = mlo_ipc_get_sys_mlo_active_bm();

		/*
		 * If a trap happens but mlo configuration is in progress for other interfaces,
		 * it can cause external abort because FIS will be stuck with mlc layer.
		 * If mlo_ap_bm is different from mlo_active_ap_bm,
		 * it could mean that mlo configuration is in progress.
		 */
		if (mlo_ap_bm != mlo_active_ap_bm) {
			DHD_ERROR(("%s: maybe mlo ipc configuration not complete - skip fis\n",
				__FUNCTION__));
			ret = BCME_ERROR;
			goto exit;
		}

		while (TRUE) {
			/*
			 * mlc sync operation uses 1us delay, 250000 loop, and 20 retry
			 * in order that MAP syncs up with AAPs.
			 * So total wait time is about 5 sec.
			 * Add 2 sec margin to 5 sec, and FIS will defer by about 7 sec.
			 */
			OSL_DELAY(FIS_TRIG_DEFER_DELAY);
			loop_cnt++;

			for (ap_unit = 0; ap_unit < PCIE_IPC_AP_UNIT_MAX; ap_unit++) {
				if ((mlo_ap_bm & (1 << ap_unit))) {
					if (ap_unit == dhd->unit ||
						dhd_mlo_ipc_is_ap_trapped(ap_unit)) {
						DHD_INFO(("%s: ap_unit %d trapped\n", __FUNCTION__,
							ap_unit));
						mlo_trap_ap_bm |= (1 << ap_unit);
					}
				}
			}

			if (mlo_ap_bm == mlo_trap_ap_bm ||
				loop_cnt > FIS_TRIG_DEFER_LOOP) {
				DHD_ERROR(("%s: mlo_ap_bm %d, mlo_trap_ap_bm %d, loop_cnt %d\n",
					__FUNCTION__, mlo_ap_bm, mlo_trap_ap_bm, loop_cnt));
				break;
			}
		}
	}
#endif	/* MLO_IPC */

	if (dhd_fis_sr_init(dhd) != BCME_OK) {
		DHD_ERROR(("%s: " IFNAME_STR ":failed to SR init\n", __FUNCTION__, dhd->unit));
		ret = BCME_ERROR;
		goto exit;
	}

	if (dhd_fis_asm_load(dhd, core, FALSE) != BCME_OK) {
		DHD_ERROR(("%s: " IFNAME_STR ": failed to load ASM (%d)\n", __FUNCTION__,
			dhd->unit, core));
		ret = BCME_ERROR;
		goto exit;
	}

	if (si_fis_set_workaround_reg(dhd->bus->sih) != BCME_OK) {
		DHD_ERROR(("%s: " IFNAME_STR ": failed to set workaround regs\n", __FUNCTION__,
			dhd->unit));
		ret = BCME_ERROR;
		goto exit;
	}

	if (dhd_fis_enable_sreng(dhd) != BCME_OK) {
		DHD_ERROR(("%s: " IFNAME_STR ": failed to enable SR eng\n", __FUNCTION__,
			dhd->unit));
		ret = BCME_ERROR;
		goto exit;
	}

	if (dhd_fis_trigger(dhd, trigger_type, FALSE) != BCME_OK) {
		DHD_ERROR(("%s: Failed to trigger FIS to " IFNAME_STR "\n", __FUNCTION__,
			dhd->unit));
		ret = BCME_ERROR;
		goto exit;
	}

	if (dhd_fis_reset(dhd, FALSE) != BCME_OK) {
		DHD_ERROR(("%s: " IFNAME_STR ": failed to recover from FIS\n", __FUNCTION__,
			dhd->unit));
		ret = BCME_ERROR;
		goto exit;
	}

	nv_val = getvar(NULL, "fis_disable_asm_dump");
	if (!nv_val) {
		is_asmdump = TRUE;
	}
	dhd_fis_create_dump(dhd, is_asmdump);

	DHD_ERROR(("%s: " IFNAME_STR ": fis dump done\n", __FUNCTION__, dhd->unit));

exit:
	DHD_UNLOCK(dhd);
	return ret;
}
