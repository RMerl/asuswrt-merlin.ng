/*
 * DHD Bus Module for PCIE
 *
 * Copyright (C) 2022, Broadcom. All Rights Reserved.
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
 * $Id: dhd_pcie.c 807970 2022-02-07 06:44:58Z $
 */

/* include files */
#include <typedefs.h>
#include <bcmutils.h>
#include <bcmdevs.h>
#include <siutils.h>
#include <hndsoc.h>
#include <hndpmu.h>
#include <hnd_debug.h>
#include <sbchipc.h>
#include <hnd_armtrap.h>
#if defined(DHD_DEBUG)
#include <hnd_cons.h>
#endif /* defined(DHD_DEBUG) */
#include <dngl_stats.h>
#include <pcie_core.h>
#include <dhd.h>
#include <wl_core.h>
#include <dhd_bus.h>
#include <dhd_flowring.h>
#include <dhd_proto.h>
#include <dhd_dbg.h>
#include <dhdioctl.h>
#include <sdiovar.h>
#include <bcmmsgbuf.h>
#include <pcicfg.h>
#include <dhd_pcie.h>
#include <bcmpcie.h>
#include <bcmendian.h>
#ifdef DHDTCPACK_SUPPRESS
#include <dhd_ip.h>
#endif /* DHDTCPACK_SUPPRESS */
#if (defined(BCM_ROUTER_DHD) && defined(BCM_GMAC3))
#include <hndfwd.h>
#endif /* BCM_ROUTER_DHD && BCM_GMAC3 */

#if defined(BCM_BLOG)
#include <dhd_blog.h>
#endif /* BCM_BLOG */

#if defined(BCM_DHD_RUNNER)
#include <dhd_runner.h>
#endif /* BCM_DHD_RUNNER */
#if defined(BCM_AWL)
#include <dhd_awl.h>
#endif /* BCM_AWL */
#ifdef DHD_IFE
#include <dhd_ife.h>
#endif

#ifdef BCM_ROUTER_DHD
#define STR_END		"END\0\0"
#define BOARDREV_PROMOTABLE_STR	"0xff"
#endif
#ifdef BCMEMBEDIMAGE
#include BCMEMBEDIMAGE
#endif /* BCMEMBEDIMAGE */

#ifdef PCIE_OOB
#include "ftdi_sio_external.h"
#endif /* PCIE_OOB */

#include <dhd_macdbg.h> // for dhd_macdbg_dump_dongle()

#if defined(DEBUGGER) || defined(DHD_DSCOPE)
#include <debugger.h>
#endif /* DEBUGGER || DHD_DSCOPE */

#if defined(BCA_SROMMAP)
#include <bcmsromio.h>
#endif

#define MEMBLOCK	2048		/* Block size used for downloading of dongle image */

/* dhdpcie_bus_membytes */
#define PCIE_SYSMEM_READ   (FALSE)
#define PCIE_SYSMEM_WRITE  (TRUE)

/* define trap types for AXI related issues */
#define TR_BUS		5	/* v7-M ARM Arch */
#define TR_DAB		4	/* other ARM Archs */

#define ARMCR4REG_BANKIDX	(0x40/sizeof(uint32))
#define ARMCR4REG_BANKPDA	(0x4C/sizeof(uint32))
/* Temporary war to fix precommit till sync issue between trunk & precommit branch is resolved */

#if defined(SUPPORT_MULTIPLE_BOARD_REV)
	extern unsigned int system_rev;
#endif /* SUPPORT_MULTIPLE_BOARD_REV */

#ifdef BCA_HNDROUTER
extern int is_reboot;
#endif

int dhd_dongle_memsize;
int dhd_dongle_ramsize;
static int dhdpcie_checkdied(dhd_bus_t *bus, char *data, uint size, uint32 *tr_type);
#ifdef DHD_DEBUG
static int dhdpcie_bus_readconsole(dhd_bus_t *bus);
#endif /* DHD_DEBUG */
#if defined(DHD_FW_COREDUMP)
static int dhdpcie_mem_dump(dhd_bus_t *bus);
#endif /* DHD_FW_COREDUMP */

static int dhd_hme_buf_alloc_try(dhd_bus_t *bus, dhd_dma_buf_t *user_dma_buf,
	pcie_ipc_hme_user_t * pcie_ipc_hme_user);
static void dhdpcie_bus_membytes(dhd_bus_t *bus, uint32 pcie_sysmem_access,
	daddr32_t daddr32, uint8 *data, uint size);
static int dhdpcie_bus_doiovar(dhd_bus_t *bus, const bcm_iovar_t *vi, uint32 actionid,
	const char *name, void *params,
	int plen, void *arg, int len, int val_size);
static int dhdpcie_bus_lpback_req(struct  dhd_bus *bus, uint32 intval);
static int dhdpcie_bus_dmaxfer_req(struct  dhd_bus *bus,
	uint32 len, uint32 srcdelay, uint32 destdelay);
static int dhdpcie_bus_download_state(dhd_bus_t *bus, bool enter);
static int _dhdpcie_download_firmware(struct dhd_bus *bus);
static int dhdpcie_download_firmware(dhd_bus_t *bus, osl_t *osh);
static int dhdpcie_bus_write_vars(dhd_bus_t *bus);
static bool dhdpcie_bus_process_mailbox_intr(dhd_bus_t *bus, uint32 intstatus);
static bool dhdpci_bus_read_frames(dhd_bus_t *bus);
static void dhdpcie_bus_clear_pcie_ipc_addr(dhd_bus_t *bus);
static int dhdpcie_bus_read_pcie_ipc(dhd_bus_t *bus, daddr32_t *pcie_ipc_daddr32,
	pcie_ipc_t *pcie_ipc, pcie_ipc_rings_t *pcie_ipc_rings,
	pcie_ipc_hme_t **pcie_ipc_hme_p);
static int dhdpcie_bus_init_pcie_ipc(dhd_bus_t *bus);
static void dhdpcie_bus_init_pcie_ipc_rings(dhd_bus_t *bus);
static bool dhdpcie_ipc_rev_compatible(uint32 host_rev, uint32 dngl_rev);
static bool dhdpcie_dongle_attach(dhd_bus_t *bus);
static void dhdpcie_bus_dongle_setmemsize(dhd_bus_t *bus, int mem_size);
static void dhdpcie_bus_release_dongle(dhd_bus_t *bus, osl_t *osh,
	bool dongle_isolation, bool reset_flag);
static void dhdpcie_bus_release_malloc(dhd_bus_t *bus, osl_t *osh);
int dhdpcie_downloadvars(dhd_bus_t *bus, void *arg, int len);
static void dhdpcie_bus_write_u8(dhd_bus_t *bus, uint32 daddr32, uint8 data);
static uint8 dhdpcie_bus_read_u8(dhd_bus_t *bus, uint32 daddr32);
static void dhdpcie_bus_write_u16(dhd_bus_t *bus, uint32 daddr32, uint16 data);
static uint16 dhdpcie_bus_read_u16(dhd_bus_t *bus, uint32 daddr32);
static void dhdpcie_bus_write_u32(dhd_bus_t *bus, uint32 daddr32, uint32 data);
static uint32 dhdpcie_bus_read_u32(dhd_bus_t *bus, uint32 daddr32);
#ifndef BCMQT
static void dhdpcie_bus_write_u64(dhd_bus_t *bus, uint32 daddr32, uint64 data);
static uint64 dhdpcie_bus_read_u64(dhd_bus_t *bus, uint32 daddr32);
#endif /* !BCMQT */
static void dhdpcie_bus_cfg_set_bar0_win(dhd_bus_t *bus, uint32 data);
static void dhdpcie_bus_reg_unmap(osl_t *osh, ulong addr, int size);
static int dhdpcie_cc_nvmshadow(dhd_bus_t *bus, struct bcmstrbuf *b);
static void dhdpcie_fw_trap(dhd_bus_t *bus);
static void dhdpcie_send_mb_data(dhd_bus_t *bus, uint32 h2d_mb_data);
#ifdef OEM_ANDROID
extern void dhd_dpc_kill(dhd_pub_t *dhdp);
#endif /* OEM_ANDROID */
#if defined(STB) && !defined(STBAP)
static void dhdpcie_bus_set_wowl(struct dhd_bus *bus, int state);
#endif /* STB && STBAP */
#ifdef BCMEMBEDIMAGE
static int dhdpcie_download_code_array(dhd_bus_t *bus);
#endif /* BCMEMBEDIMAGE */
#ifdef BCM_ROUTER_DHD
extern char * getvar(char *vars, const char *name);
#endif
#if defined(BCMEMBEDIMAGE) && defined(BCM_ROUTER_DHD)
static void  select_fd_image(
		struct dhd_bus *bus, unsigned char **p_dlarray,
		char **p_dlimagename, char **p_dlimagever,
		char **p_dlimagedate, int *image_size);
#endif /* defined(BCMEMBEDIMAGE) && defined (BCM_ROUTER_DHD) */

#if defined(BCM_ROUTER_DHD) && defined(EXTFDIMGPATH)
static int select_chipidverstr(struct dhd_bus *bus, char *idverstr);
static int concate_extfdimg_name(dhd_bus_t *bus, char *fw_path);
#endif /* defined(BCM_ROUTER_DHD) && defined(EXTFDIMGPATH) */

#ifdef BCM_ROUTER_DHD
int dbushost_initvars_flash(si_t *sih, osl_t *osh, char **base, uint len);
#endif

#define PCI_VENDOR_ID_BROADCOM		0x14e4
#define PCI_DEVICE_REMOVED		(uint32)-1

static void dhd_bus_set_device_wake(struct dhd_bus *bus, bool val);
extern void wl_nddbg_wpp_log(const char *format, ...);
#ifdef PCIE_OOB
static void dhd_bus_doorbell_timeout_reset(struct dhd_bus *bus);

#define DHD_DEFAULT_DOORBELL_TIMEOUT 200	/* ms */
static uint dhd_doorbell_timeout = DHD_DEFAULT_DOORBELL_TIMEOUT;

#define HOST_WAKE 4   /* GPIO_0 (HOST_WAKE) - Output from WLAN */
#define DEVICE_WAKE 5  /* GPIO_1 (DEVICE_WAKE) - Input to WLAN */
#define BIT_WL_REG_ON 6
#define BIT_BT_REG_ON 7

int gpio_handle_val = 0;
unsigned char gpio_port = 0;
unsigned char gpio_direction = 0;
#define OOB_PORT "ttyUSB0"
#endif /* PCIE_OOB */

static void hnd_hw_coherent_disable(dhd_bus_t *bus);

/* IOVar table */
enum {
	IOV_INTR = 1,
	IOV_MEMBYTES,
	IOV_MEMSIZE,
	IOV_SET_DOWNLOAD_STATE,
	IOV_DEVRESET,
	IOV_VARS,
	IOV_MSI_SIM,
	IOV_PCIE_LPBK,
	IOV_CC_NVMSHADOW,
	IOV_RAMSIZE,
	IOV_RAMSTART,
	IOV_SLEEP_ALLOWED,
	IOV_PCIE_DMAXFER,
	IOV_PCIE_SUSPEND,
	IOV_PCIEREG,
	IOV_PCIECFGREG,
	IOV_PCIECOREREG,
	IOV_PCIESERDESREG,
	IOV_PCIEASPM,
	IOV_BAR0_SECWIN_REG,
	IOV_SBREG,
	IOV_DONGLEISOLATION,
	IOV_LTRSLEEPON_UNLOOAD,
	IOV_METADATA_DBG,
	IOV_RX_METADATALEN,
	IOV_TX_METADATALEN,
	IOV_TXP_THRESHOLD,
	IOV_BUZZZ_DUMP,
	IOV_BUZZZ_FILE,
	IOV_CSIMON,
	IOV_DUMP_RINGUPD_BLOCK,
	IOV_DMA_RINGINDICES,
	IOV_FORCE_FW_TRAP,
	IOV_DB1_FOR_MB,
	IOV_FLOW_PRIO_MAP,
	IOV_RXBOUND,
	IOV_TXBOUND,
	IOV_HANGREPORT,
#ifdef PCIE_OOB
	IOV_OOB_BT_REG_ON,
	IOV_OOB_ENABLE,
#endif /* PCIE_OOB */
	IOV_DUMP_DONGLE,
	IOV_HALT_DONGLE,
	IOV_IDMA_ENABLE,
	IOV_DNGL_CAPS,   /**< returns string with dongle capabilities */
#if defined(DEBUGGER) || defined(DHD_DSCOPE)
	IOV_GDB_SERVER,  /**< starts gdb server on given interface */
#endif /* DEBUGGER || DHD_DSCOPE */
	IOV_HYBRIDFW,
	IOV_PCIE_LAST    /**< unused IOVAR */
};

const bcm_iovar_t dhdpcie_iovars[] = {
	{"intr",	IOV_INTR,	0,	0,  IOVT_BOOL,	0 },
	{"membytes",	IOV_MEMBYTES,	0,	0,  IOVT_BUFFER,	2 * sizeof(int) },
	{"memsize",	IOV_MEMSIZE,	0,	0,  IOVT_UINT32,	0 },
	{"dwnldstate",	IOV_SET_DOWNLOAD_STATE,	0,	0,  IOVT_BOOL,	0 },
	{"vars",	IOV_VARS,	0,	0,  IOVT_BUFFER,	0 },
	{"devreset",	IOV_DEVRESET,	0,	0,  IOVT_BOOL,	0 },
	{"pcie_device_trap", IOV_FORCE_FW_TRAP, 0,	0,  0,	0 },
	{"pcie_lpbk",	IOV_PCIE_LPBK,	0,	0,  IOVT_UINT32,	0 },
	{"cc_nvmshadow", IOV_CC_NVMSHADOW, 0, 0,    IOVT_BUFFER, 0 },
	{"ramsize",	IOV_RAMSIZE,	0,	0,  IOVT_UINT32,	0 },
	{"ramstart",	IOV_RAMSTART,	0,	0,  IOVT_UINT32,	0 },
	{"pciereg",	IOV_PCIEREG,	0,	0,  IOVT_BUFFER,	2 * sizeof(int32) },
	{"pciecfgreg",	IOV_PCIECFGREG,	0,	0,  IOVT_BUFFER,	2 * sizeof(int32) },
	{"pciecorereg",	IOV_PCIECOREREG,	0,	0,  IOVT_BUFFER,	2 * sizeof(int32) },
	{"pcieserdesreg",	IOV_PCIESERDESREG,	0,	0,
	IOVT_BUFFER,	3 * sizeof(int32) },
	{"bar0secwinreg",	IOV_BAR0_SECWIN_REG,	0,	0,
	IOVT_BUFFER,	sizeof(sdreg_t) },
	{"sbreg",	IOV_SBREG,	0,	0,  IOVT_BUFFER,	sizeof(uint8) },
	{"pcie_dmaxfer",	IOV_PCIE_DMAXFER,	0,	0,
	IOVT_BUFFER,	3 * sizeof(int32) },
	{"pcie_suspend", IOV_PCIE_SUSPEND,	0,	0,  IOVT_UINT32,	0 },
#ifdef PCIE_OOB
	{"oob_bt_reg_on", IOV_OOB_BT_REG_ON,    0,  0,  IOVT_UINT32,    0 },
	{"oob_enable",   IOV_OOB_ENABLE,    0,  0,  IOVT_UINT32,    0 },
#endif /* PCIE_OOB */
	{"sleep_allowed",	IOV_SLEEP_ALLOWED,	0,	0,  IOVT_BOOL,	0 },
	{"dngl_isolation", IOV_DONGLEISOLATION,	0,	0,  IOVT_UINT32,	0 },
	{"ltrsleep_on_unload", IOV_LTRSLEEPON_UNLOOAD,	0,	0,  IOVT_UINT32,	0 },
	{"dump_ringupdblk", IOV_DUMP_RINGUPD_BLOCK,	0,	0,  IOVT_BUFFER,	0 },
	{"dma_ring_indices", IOV_DMA_RINGINDICES,	0,	0,  IOVT_UINT32,	0},
	{"metadata_dbg", IOV_METADATA_DBG,	0,	0,  IOVT_BOOL,	0 },
	{"rx_metadata_len", IOV_RX_METADATALEN,	0,	0,  IOVT_UINT32,	0 },
	{"tx_metadata_len", IOV_TX_METADATALEN,	0,	0,  IOVT_UINT32,	0 },
	{"db1_for_mb", IOV_DB1_FOR_MB,	0,	0,  IOVT_UINT32,	0 },
	{"txp_thresh", IOV_TXP_THRESHOLD,	0,	0,  IOVT_UINT32,	0 },
	{"buzzz_dump", IOV_BUZZZ_DUMP,		0,	0,  IOVT_UINT32,	0 },
	{"buzzz_file", IOV_BUZZZ_FILE,		0,	0,  IOVT_UINT32,	0 },
	{"csimon", IOV_CSIMON,		0,	0,  IOVT_UINT32,	0 },
	{"flow_prio_map", IOV_FLOW_PRIO_MAP,	0,	0,  IOVT_UINT32,	0 },
	{"rxbound",     IOV_RXBOUND,    0,      0,  IOVT_UINT32,    0 },
	{"txbound",     IOV_TXBOUND,    0,      0,  IOVT_UINT32,    0 },
	{"aspm", IOV_PCIEASPM, 0, 0,    IOVT_INT32, 0 },
	{"fw_hang_report", IOV_HANGREPORT,	0,	0,  IOVT_BOOL,	0 },
	{"dump_dongle", IOV_DUMP_DONGLE, 0, 0,  IOVT_BUFFER,
	MAX(sizeof(dump_dongle_in_t), sizeof(dump_dongle_out_t))},
	{"halt_dongle", IOV_HALT_DONGLE,    0,      0,  IOVT_UINT32,    0 },
	{"idma_enable",   IOV_IDMA_ENABLE,    0, 0,  IOVT_UINT32,    0 },
	{"cap", IOV_DNGL_CAPS,    0,      0,    IOVT_BUFFER,    0},
#if defined(DEBUGGER) || defined(DHD_DSCOPE)
	{"gdb_server", IOV_GDB_SERVER,    0,      0,    IOVT_UINT32,    0 },
#endif /* DEBUGGER || DHD_DSCOPE */
	{"hybridfw",   IOV_HYBRIDFW,    0, 0,  IOVT_BUFFER,    0 },
	{NULL, 0, 0, 0, 0, 0 }
};

#ifdef BCMQT
#define MAX_READ_TIMEOUT	5 * 5 * 1000 * 1000
#else
#define MAX_READ_TIMEOUT	10 * 1000 * 1000
#endif

#ifndef DHD_RXBOUND
#define DHD_RXBOUND		64
#endif
#ifndef DHD_TXBOUND
#define DHD_TXBOUND		64
#endif
uint dhd_rxbound = DHD_RXBOUND;
uint dhd_txbound = DHD_TXBOUND;

#if defined(DEBUGGER) || defined(DHD_DSCOPE)
/** the GDB debugger layer will call back into this (bus) layer to read/write dongle memory */
static struct dhd_gdb_bus_ops_s  bus_ops = {
	.read_u16 = dhdpcie_bus_read_u16,
	.read_u32 = dhdpcie_bus_read_u32,
	.write_u32 = dhdpcie_bus_write_u32,
};
#endif /* DEBUGGER || DHD_DSCOPE */

/* Register/Unregister functions are called by the main DHD entry
 * point (e.g. module insertion) to link with the bus driver, in
 * order to look for or await the device.
 */

int
dhd_bus_register(void)
{
	DHD_TRACE(("%s: Enter\n", __FUNCTION__));

	return dhdpcie_bus_register();
}

void
dhd_bus_unregister(void)
{
	DHD_TRACE(("%s: Enter\n", __FUNCTION__));

	dhdpcie_bus_unregister();
}

/** returns a host virtual address */
uint32 *
dhdpcie_bus_reg_map(osl_t *osh, ulong addr, int size)
{
	return (uint32 *)REG_MAP(addr, size);
}

void
dhdpcie_bus_reg_unmap(osl_t *osh, ulong addr, int size)
{
	REG_UNMAP((void*)(uintptr)addr);
}

/**
 * Called once for each hardware (dongle) instance that this DHD manages.
 *
 * 'regs' is the host virtual address that maps to the start of the PCIe BAR0 window. The first 4096
 * bytes in this window are mapped to the backplane address in the PCIEBAR0Window register. The
 * precondition is that the PCIEBAR0Window register 'points' at the PCIe core.
 *
 * 'tcm' is the *host* virtual address at which tcm is mapped.
 */
dhd_bus_t* dhdpcie_bus_attach(osl_t *osh,
	volatile char *regs, volatile char *tcm, void *pci_dev)
{
	dhd_bus_t *bus;

	DHD_TRACE(("%s: ENTER\n", __FUNCTION__));

	do {
		if (!(bus = MALLOCZ(osh, sizeof(dhd_bus_t)))) {
			DHD_ERROR(("%s: MALLOC of dhd_bus_t failed\n", __FUNCTION__));
			break;
		}

		bus->regs = regs;
		bus->tcm = tcm;
		bus->osh = osh;

		/* Save pci_dev into dhd_bus, as it may be needed in dhd_attach */
		bus->dev = (struct pci_dev *)pci_dev;

#if defined(BCM_ROUTER_DHD) && defined(BCM_GMAC3)
		dll_init(&bus->txqueue_pend);
		dll_init(&bus->txqueue_done);
#endif /* BCM_ROUTER_DHD && BCM_GMAC3 */
		dll_init(&bus->const_flowring);

		/* dhd_common_init(osh); */

		if (dhdpcie_dongle_attach(bus)) {
			DHD_ERROR(("%s: dhdpcie_probe_attach failed\n", __FUNCTION__));
			break;
		}

		/* software resources */
		if (!(bus->dhd = dhd_attach(osh, bus, PCMSGBUF_HDRLEN))) {
			DHD_ERROR(("%s: dhd_attach failed\n", __FUNCTION__));

			break;
		}
		bus->dhd->busstate = DHD_BUS_DOWN;
		bus->db1_for_mb = TRUE;
		bus->dhd->hang_report = TRUE;

		DHD_TRACE(("%s: EXIT SUCCESS\n",
			__FUNCTION__));

		return bus;
	} while (0);

	DHD_TRACE(("%s: EXIT FAILURE\n", __FUNCTION__));

	if (bus) {
		MFREE(osh, bus, sizeof(dhd_bus_t));
	}

	return NULL;
}

uint
dhd_bus_chip(struct dhd_bus *bus)
{
	ASSERT(bus->sih != NULL);

	return bus->sih->chip;
}

uint
dhd_bus_chiprev(struct dhd_bus *bus)
{
	ASSERT(bus);
	ASSERT(bus->sih != NULL);

	return bus->sih->chiprev;
}

void *
dhd_bus_pub(struct dhd_bus *bus)
{
	return bus->dhd;
}

void *
dhd_bus_sih(struct dhd_bus *bus)
{
	return (void *)bus->sih;
}

void *
dhd_bus_txq(struct dhd_bus *bus)
{
	return &bus->txq;
}

/** Get Chip ID version */
uint dhd_bus_chip_id(dhd_pub_t *dhdp)
{
	dhd_bus_t *bus = dhdp->bus;

	return  bus->sih->chip;
}

/** Get Chip Rev ID version */
uint dhd_bus_chiprev_id(dhd_pub_t *dhdp)
{
	dhd_bus_t *bus = dhdp->bus;

	return bus->sih->chiprev;
}

/** Get Chip Pkg ID version */
uint dhd_bus_chippkg_id(dhd_pub_t *dhdp)
{
	dhd_bus_t *bus = dhdp->bus;

	return bus->sih->chippkg;
}

/**
 * Read and clear intstatus. This should be called with interupts disabled or inside isr.
 * Return PCI_DEVICE_REMOVED if device removal was detected.
 */
uint32
dhdpcie_bus_intstatus(dhd_bus_t *bus)
{
	uint32 intstatus = 0;
	uint32 intmask = 0;

	if ((bus->sih->buscorerev == 6) || (bus->sih->buscorerev == 4) ||
		(bus->sih->buscorerev == 2)) {
		intstatus = dhdpcie_bus_cfg_read_dword(bus, PCIIntstatus, 4);
		dhdpcie_bus_cfg_write_dword(bus, PCIIntstatus, 4, intstatus);
		intstatus &= I_MB;
	} else {
		/* this is a PCIE core register..not a config register... */
		intstatus = si_corereg(bus->sih, bus->sih->buscoreidx, PCIMailBoxInt, 0, 0);

		/* this is a PCIE core register..not a config register... */
		intmask = si_corereg(bus->sih, bus->sih->buscoreidx, PCIMailBoxMask, 0, 0);

		/* If device was removed. intstatus & intmask read (0xffffffff) */
		/* In that case, return PCI_DEVICE_REMOVED to the caller */
		if (intstatus != PCI_DEVICE_REMOVED) {

			/*
			 * The fourth argument to si_corereg is the "mask" fields of the register
			 * to update
			 * and the fifth field is the "value" to update. Now if we are interested
			 * in only
			 * few fields of the "mask" bit map, we should not be writing back what we
			 * read
			 * By doing so, we might clear/ack interrupts that are not handled yet.
			 */
			si_corereg(bus->sih, bus->sih->buscoreidx, PCIMailBoxInt, bus->def_intmask,
				intstatus);

			intstatus &= intmask;
			intstatus &= bus->def_intmask;
		}
	}

	return intstatus;
}

/**
 * Interrupt Service routine checks for the status register, disable interrupt and queue DPC if
 * mail box interrupts are raised.
 *
 * @par[in]  irq      interrupt vector
 * @par[in]  arg      handle to private data structure
 * @return   Status (TRUE or FALSE)
 *
 * Description:
 * Interrupt Service routine checks for the status register,
 * disable interrupt and queue DPC if mail box interrupts are raised.
 *
 * Note: the interrupt line can be 'shared', which means that this function can get called because
 *       of a non-Broadcom device generating an interrupt.
 */
int32
dhdpcie_bus_isr(dhd_bus_t *bus)
{
	uint32 intstatus = 0;

	do {
		DHD_TRACE(("%s: Enter\n", __FUNCTION__));
		/* verify argument */
		if (!bus) {
			DHD_ERROR(("%s : bus is null pointer, exit \n", __FUNCTION__));
			break;
		}

		if (bus->dhd->dongle_reset) {
			break;
		}

		if (bus->dhd->busstate == DHD_BUS_DOWN ||
		    bus->dhd->busstate == DHD_BUS_SUSPENDED) {
			DHD_INFO(("%s: BUS is down/suspended, not processing the interrupt \r\n",
				__FUNCTION__));
			break;
		}

		intstatus = dhdpcie_bus_intstatus(bus);

		/* Check for device removal */
		if (intstatus == PCI_DEVICE_REMOVED) {
			DHD_ERROR(("%s: !!!!!!Device Removed or dead chip.\n", __FUNCTION__));
			break;
		}

		/* Check if the interrupt is ours or not */
		if (intstatus == 0) {
			break;
		}

		/* save the intstatus */
		bus->intstatus = intstatus;

		/*  Overall operation:
		 *    - Mask further interrupts
		 *    - Read/ack intstatus
		 *    - Take action based on bits and state
		 *    - Reenable interrupts (as per state)
		 */

		/* Count the interrupt call */
		bus->intrcount++;

		/* read interrupt status register!! Status bits will be cleared in DPC !! */
		bus->ipend = TRUE;
		dhdpcie_bus_intr_disable(bus); /* Disable interrupt!! */
		bus->intdis = TRUE;

#if defined(PCIE_ISR_THREAD)

		DHD_TRACE(("Calling dhd_bus_dpc() from %s\n", __FUNCTION__));
		DHD_OS_WAKE_LOCK(bus->dhd);
		while (dhd_bus_dpc(bus));
		DHD_OS_WAKE_UNLOCK(bus->dhd);
#else
		bus->dpc_sched = TRUE;
		dhd_sched_dpc(bus->dhd);     /* queue DPC now!! */
#endif /* defined(SDIO_ISR_THREAD) */

		DHD_TRACE(("%s: Exit Success DPC Queued\n", __FUNCTION__));
		return TRUE;

	} while (0);

	DHD_TRACE(("%s: Exit Failure\n", __FUNCTION__));
	return FALSE;
}

static uint32
dhdpcie_bus_get_htreq(dhd_bus_t *bus)
{
	uint32 chip = bus->sih->chip;

	if (BCM4365_CHIP(chip))
		return (1 << RES4365_HT_AVAIL | 1 << RES4365_MACPHY_CLK_AVAIL);
	if (BCM43684_CHIP(chip))
		return (1 << RES43684_HT_AVAIL | 1 << RES43684_MACPHY_CLK_AVAIL);
	return 0;
}

/*
 * This function is to switch Backplane clock from HT to ALP and vice versa.
 */
static void
dhdpcie_bus_switch_ht_alp(dhd_bus_t *bus, uint32 bp_clk)
{
	uint32 pmu_idx = si_findcoreidx(bus->sih, PMU_CORE_ID, 0);
	uint32 ht_req = dhdpcie_bus_get_htreq(bus);
	uint32 force_clk = (bp_clk == CCS_BP_ON_HT) ? CCS_FORCEHT : CCS_FORCEALP;
	uint32 clk_ctl_st;

	ASSERT(bp_clk == CCS_BP_ON_HT || bp_clk == CCS_BP_ON_ALP);

	if (!ht_req)
		return;

	clk_ctl_st = si_corereg(bus->sih, SI_CC_IDX, OFFSETOF(chipcregs_t, clk_ctl_st), 0, 0);

	DHD_ERROR(("%s: BP is currently on %s clock (0x%08x), force BP on %s clock\n", __FUNCTION__,
		(clk_ctl_st & CCS_BP_ON_HT) ? "HT" : "ALP", clk_ctl_st,
		(bp_clk & CCS_BP_ON_HT) ? "HT" : "ALP"));

	if (bp_clk == CCS_BP_ON_HT) {
		/* enable HT clock resources */
		si_corereg(bus->sih, pmu_idx, PMU_MIN_RES_MASK, ht_req, ht_req);
		OSL_DELAY(100);
		si_corereg(bus->sih, pmu_idx, PMU_MAX_RES_MASK, ht_req, ht_req);
	} else {
		/* disable HT clock resources */
		si_corereg(bus->sih, pmu_idx, PMU_MIN_RES_MASK, ht_req, 0);
		OSL_DELAY(100);
		si_corereg(bus->sih, pmu_idx, PMU_MAX_RES_MASK, ht_req, 0);
	}
	OSL_DELAY(100);

	si_corereg(bus->sih, SI_CC_IDX, OFFSETOF(chipcregs_t, clk_ctl_st),
		CCS_FORCEHT | CCS_FORCEALP, force_clk);

	SPINWAIT(((clk_ctl_st & bp_clk) == 0 &&	(clk_ctl_st = si_corereg(bus->sih, SI_CC_IDX,
		OFFSETOF(chipcregs_t, clk_ctl_st), 0, 0))), PMU_MAX_TRANSITION_DLY);
	ASSERT(clk_ctl_st & bp_clk);

	clk_ctl_st = si_corereg(bus->sih, SI_CC_IDX, OFFSETOF(chipcregs_t, clk_ctl_st), 0, 0);
	DHD_ERROR(("%s: BP is on %s clock now (0x%08x)\n", __FUNCTION__,
		(clk_ctl_st & CCS_BP_ON_HT) ? "HT" : "ALP", clk_ctl_st));
}

static int
dhdpcie_prep_enum_scan(osl_t *osh)
{
	uint32 corr_err_stat, revid;

	corr_err_stat = OSL_PCI_READ_CONFIG(osh, PCI_CORR_ERR_STATUS, sizeof(uint32));

	if (corr_err_stat) {
		revid = OSL_PCI_READ_CONFIG(osh, PCI_REVID, sizeof(uint32));
		if ((revid & PCI_REVISION_MASK) >= 16) {
			DHD_ERROR(("%s: correctable error detected (0x%08x), backplane reset\n",
				__FUNCTION__, corr_err_stat));
			OSL_PCI_WRITE_CONFIG(osh, PCI_SPROM_CONTROL,
				sizeof(uint32), SPROM_CFG_TO_SB_RST);

			SPINWAIT((OSL_PCI_READ_CONFIG(osh, PCI_SPROM_CONTROL,
				sizeof(uint32)) & SPROM_CFG_TO_SB_RST), 500);

			OSL_DELAY(2000);

			OSL_PCI_WRITE_CONFIG(osh, PCI_CORR_ERR_STATUS,
				sizeof(uint32), corr_err_stat);

			return BCME_OK;
		}
		return BCME_NODEVICE;
	}
	return BCME_OK;
}

#ifdef CUSTOMER_HW4
#if defined(CONFIG_MACH_UNIVERSAL5433) || defined(CONFIG_MACH_UNIVERSAL7420)
dhd_pub_t *link_recovery = NULL;
#endif /* CONFIG_MACH_UNIVERSAL5433 || CONFIG_MACH_UNIVERSAL7420 */
#endif /* CUSTOMER_HW4 */

static bool
dhdpcie_dongle_attach(dhd_bus_t *bus)
{
	osl_t *osh = bus->osh;
	volatile void *regsva = (volatile void*)bus->regs;
	uint16 devid = bus->cl_devid;
	uint32 val;
	sbpcieregs_t *sbpcieregs;

	DHD_TRACE(("%s: ENTER\n", __FUNCTION__));

#ifdef CUSTOMER_HW4
#if defined(CONFIG_MACH_UNIVERSAL5433) || defined(CONFIG_MACH_UNIVERSAL7420)
	link_recovery = bus->dhd;
#endif /* CONFIG_MACH_UNIVERSAL5433 || CONFIG_MACH_UNIVERSAL7420 */
#endif /* CUSTOMER_HW4 */

	bus->sih = NULL;

	/* Checking PCIe bus status with reading configuration space */
	val = OSL_PCI_READ_CONFIG(osh, PCI_CFG_VID, sizeof(uint32));
	if ((val & 0xFFFF) != VENDOR_BROADCOM) {
		DHD_ERROR(("%s : failed to read PCI configuration space!\n", __FUNCTION__));
		goto fail;
	}
	devid = (val >> 16) & 0xFFFF;
	bus->cl_devid = devid;

	/* Set bar0 window to si_enum_base */
	dhdpcie_bus_cfg_set_bar0_win(bus, si_enum_base(devid));

	if (dhdpcie_prep_enum_scan(osh) != BCME_OK) {
		DHD_ERROR(("%s: device not accessible\n", __FUNCTION__));
		goto fail;
	}

	/* si_attach() will provide an SI handle and scan the backplane */
	if (!(bus->sih = si_attach((uint)devid, osh, regsva, PCI_BUS, bus,
	                           &bus->vars, &bus->varsz))) {
		DHD_ERROR(("%s: si_attach failed!\n", __FUNCTION__));
		goto fail;
	}

	dhdpcie_bus_switch_ht_alp(bus, CCS_BP_ON_ALP);

	/*
	 * Issue CC watchdog to reset all the cores on the chip - similar to rmmod dhd
	 * This is required to avoid spurious interrupts to the Host and bring back
	 * dongle to a sane state (on host soft-reboot / watchdog-reboot).
	 */
	si_corereg(bus->sih, SI_CC_IDX, OFFSETOF(chipcregs_t, watchdog), ~0, 4);
	OSL_DELAY(300000);

	si_setcore(bus->sih, PCIE2_CORE_ID, 0);
	sbpcieregs = (sbpcieregs_t*)(bus->regs);

	/* WAR where the BAR1 window may not be sized properly */
	W_REG(osh, &sbpcieregs->configaddr, 0x4e0);
	val = R_REG(osh, &sbpcieregs->configdata);
	W_REG(osh, &sbpcieregs->configdata, val);

	/* Get info on the ARM and SOCRAM cores... */
	/* Should really be qualified by device id */
	if ((si_setcore(bus->sih, ARM7S_CORE_ID, 0)) ||
	    (si_setcore(bus->sih, ARMCM3_CORE_ID, 0)) ||
	    (si_setcore(bus->sih, ARMCR4_CORE_ID, 0)) ||
	    (si_setcore(bus->sih, ARMCA7_CORE_ID, 0))) {
		bus->armrev = si_corerev(bus->sih);
	} else {
		DHD_ERROR(("%s: failed to find ARM core!\n", __FUNCTION__));
		goto fail;
	}

	if (si_setcore(bus->sih, SYSMEM_CORE_ID, 0)) {
		if (!(bus->orig_ramsize = si_sysmem_size(bus->sih))) { // may reset sysmem core
			DHD_ERROR(("%s: failed to find SYSMEM memory!\n", __FUNCTION__));
			goto fail;
		}
		switch ((uint16)bus->sih->chip) {
			CASE_BCM43684_CHIP:
				bus->dongle_ram_base = CA7_43684_RAM_BASE;
				bus->orig_ramsize = 0x600000; /* 6MB */
				break;
			CASE_BCM6715_CHIP:
				bus->dongle_ram_base = CA7_6715_RAM_BASE;
				bus->orig_ramsize = 0x500000; /* 5MB */
				break;

			default: /* 4365/6 family */
				/* also populate base address */
				bus->dongle_ram_base = CA7_4365_RAM_BASE;
				bus->orig_ramsize = 0x1c0000; /* Reserve 1.75MB for CA7 */
				break;
		}
	} else if (!si_setcore(bus->sih, ARMCR4_CORE_ID, 0)) {
		if (!(bus->orig_ramsize = si_socram_size(bus->sih))) {
			DHD_ERROR(("%s: failed to find SOCRAM memory!\n", __FUNCTION__));
			goto fail;
		}
	} else {
		/* cr4 has a different way to find the RAM size from TCM's */
		if (!(bus->orig_ramsize = si_tcm_size(bus->sih))) {
			DHD_ERROR(("%s: failed to find CR4-TCM memory!\n", __FUNCTION__));
			goto fail;
		}
		/* also populate base address */
		switch ((uint16)bus->sih->chip) {
		case BCM4339_CHIP_ID:
		case BCM4335_CHIP_ID:
			bus->dongle_ram_base = CR4_4335_RAM_BASE;
			break;
		case BCM4358_CHIP_ID:
		case BCM4356_CHIP_ID:
		case BCM4354_CHIP_ID:
		case BCM43567_CHIP_ID:
		case BCM43569_CHIP_ID:
		case BCM4350_CHIP_ID:
		case BCM43570_CHIP_ID:
			bus->dongle_ram_base = CR4_4350_RAM_BASE;
			break;
#ifdef UNRELEASEDCHIP
		case BCM4347_CHIP_ID:
			bus->dongle_ram_base = CR4_4347_RAM_BASE;
			break;
#endif
			break;
		case BCM4360_CHIP_ID:
			bus->dongle_ram_base = CR4_4360_RAM_BASE;
			break;
#ifdef UNRELEASEDCHIP
		case BCM4364_CHIP_ID:
			bus->dongle_ram_base = CR4_4364_RAM_BASE;
			break;
#endif
		CASE_BCM4345_CHIP:
			bus->dongle_ram_base = (bus->sih->chiprev < 6)  /* changed at 4345C0 */
				? CR4_4345_LT_C0_RAM_BASE : CR4_4345_GE_C0_RAM_BASE;
			break;
		CASE_BCM43602_CHIP:
			bus->dongle_ram_base = CR4_43602_RAM_BASE;
			break;
		case BCM4349_CHIP_GRPID:
			bus->dongle_ram_base = CR4_4349_RAM_BASE;
			break;
		default:
			bus->dongle_ram_base = 0;
			DHD_ERROR(("%s: WARNING: Using default ram base at 0x%x\n",
			           __FUNCTION__, bus->dongle_ram_base));
		}
	}
	bus->ramsize = bus->orig_ramsize;
	if (dhd_dongle_memsize)
		dhdpcie_bus_dongle_setmemsize(bus, dhd_dongle_memsize);

	DHD_ERROR(("DHD: dongle ram size is set to %d(orig %d) at 0x%x\n",
	           bus->ramsize, bus->orig_ramsize, bus->dongle_ram_base));

	bus->srmemsize = si_socram_srmem_size(bus->sih);

	bus->def_intmask = PCIE_MB_D2H_MB_MASK | PCIE_MB_TOPCIE_FN0_0 | PCIE_MB_TOPCIE_FN0_1;

	/* Set the poll and/or interrupt flags */
	bus->intr = (bool)dhd_intr;

	bus->wait_for_d3_ack = 1;
	bus->suspended = FALSE;

#ifdef PCIE_OOB
	gpio_handle_val = get_handle(OOB_PORT);
	if (gpio_handle_val < 0)
	{
		DHD_ERROR(("%s: Could not get GPIO handle.\n", __FUNCTION__));
		ASSERT(FALSE);
	}

	gpio_direction = 0;
	ftdi_set_bitmode(gpio_handle_val, 0, BITMODE_BITBANG);

	/* Note BT core is also enabled here */
	gpio_port = 1 << BIT_WL_REG_ON | 1 << BIT_BT_REG_ON | 1 << DEVICE_WAKE;
	gpio_write_port(gpio_handle_val, gpio_port);

	gpio_direction = 1 << BIT_WL_REG_ON | 1 << BIT_BT_REG_ON | 1 << DEVICE_WAKE;
	ftdi_set_bitmode(gpio_handle_val, gpio_direction, BITMODE_BITBANG);

	bus->oob_enabled = TRUE;

	/* drive the Device_Wake GPIO low on startup */
	bus->device_wake_state = TRUE;
	dhd_bus_set_device_wake(bus, FALSE);
	dhd_bus_doorbell_timeout_reset(bus);
#endif /* PCIE_OOB */

#ifdef BCM_ROUTER_DHD
	if (BCM4365_CHIP(bus->sih->chip)) {
		if (bus->sih->buscoretype == PCIE2_CORE_ID)
			OSL_PCIE_MPS_LIMIT(osh, PCIECFGREG_DEVCONTROL, 256);
		else if (bus->sih->buscoretype == PCIE_CORE_ID)
			OSL_PCIE_MPS_LIMIT(osh, PCI_CFG_DEVCTRL, 256);
	}
#endif /* BCM_ROUTER_DHD */

#if defined(BCM_DHD_RUNNER)
	/* XXX, need Runner to support iDMA (DoorBell per set/group on Chn2).
	 * A flowring without Runner offload is working but it doesn't work on a Runner
	 * offload flowring.
	*/
	bus->idma_enabled = TRUE;
#else
	bus->idma_enabled = TRUE;
#endif

	DHD_TRACE(("%s: EXIT: SUCCESS\n", __FUNCTION__));
	return 0;

fail:
	if (bus->sih != NULL) {
		si_detach(bus->sih);
		bus->sih = NULL;
	}

	DHD_TRACE(("%s: EXIT: FAILURE\n", __FUNCTION__));
	return -1;
}

int
dhpcie_bus_unmask_interrupt(dhd_bus_t *bus)
{
	dhdpcie_bus_cfg_write_dword(bus, PCIIntmask, 4, I_MB);

	return 0;
}

int
dhpcie_bus_mask_interrupt(dhd_bus_t *bus)
{
	dhdpcie_bus_cfg_write_dword(bus, PCIIntmask, 4, 0x0);

	return 0;
}

/**
 * Called only from task context
 */
void
dhdpcie_bus_intr_enable(dhd_bus_t *bus)
{
	DHD_TRACE(("enable interrupts\n"));
	if (bus && bus->sih) {
		if ((bus->sih->buscorerev == 2) || (bus->sih->buscorerev == 6) ||
			(bus->sih->buscorerev == 4)) {
			dhpcie_bus_unmask_interrupt(bus);
		} else {
		si_corereg(bus->sih, bus->sih->buscoreidx, PCIMailBoxMask,
			bus->def_intmask, bus->def_intmask);
		}
	}
}

/**
 * Called from both isr and task context
 *
 * Prerequisite: a spinlock should have been acquired
 */
void
dhdpcie_bus_intr_disable(dhd_bus_t *bus)
{

	DHD_TRACE(("%s Enter\n", __FUNCTION__));

	if (bus && bus->sih) {
		if ((bus->sih->buscorerev == 2) || (bus->sih->buscorerev == 6) ||
			(bus->sih->buscorerev == 4)) {
			dhpcie_bus_mask_interrupt(bus);
		} else {
			si_corereg(bus->sih, bus->sih->buscoreidx, PCIMailBoxMask,
				bus->def_intmask, 0);
		}
	}
	DHD_TRACE(("%s Exit\n", __FUNCTION__));
}

static void
dhdpcie_bus_watchdog_reset(dhd_bus_t *bus)
{
	uint32 chip_id = si_chipid(bus->sih);

	if (BCM4365_CHIP(chip_id)) {
	    /* XXX: Sometimes watchdog reset causes host stall if run
	     * with HT clock. So turn off HT clock and switch to ALP
	     * clock before performing watchdog reset. We will again
	     * switch to HT clock once the reset is over.
	     */
	    dhdpcie_bus_switch_ht_alp(bus, CCS_BP_ON_ALP);
	}

	/* Turn off CA7 coherence to avoid system access error */
	if (BCM4365_CHIP(bus->sih->chip) ||
	    BCM43684_CHIP(bus->sih->chip) ||
	    BCM6715_CHIP(bus->sih->chip)) {
		hnd_hw_coherent_disable(bus);
	}

	pcie_watchdog_reset(bus->osh, bus->sih, (sbpcieregs_t *)(bus->regs));

	if (BCM4365_CHIP(chip_id)) {
	    /* reset done.. turn on HT clock */
	    dhdpcie_bus_switch_ht_alp(bus, CCS_BP_ON_HT);
	}
}

static void
dhdpcie_bus_remove_prep(dhd_bus_t *bus)
{
	DHD_TRACE(("%s Enter\n", __FUNCTION__));

	dhd_os_sdlock(bus->dhd);

	bus->dhd->busstate = DHD_BUS_DOWN;

	if (bus->intr) {
		dhdpcie_bus_intr_disable(bus);
	}

	if (bus->dhd->is_pcie_watchdog_reset == FALSE) {
		dhdpcie_bus_watchdog_reset(bus);
		bus->dhd->is_pcie_watchdog_reset = TRUE;
	}

	dhd_os_sdunlock(bus->dhd);

	DHD_TRACE(("%s Exit\n", __FUNCTION__));
}

/** Detach and free everything */
void
dhdpcie_bus_release(dhd_bus_t *bus)
{
	bool dongle_isolation = FALSE;
	osl_t *osh = NULL;
#ifdef BCMQT
	uint buscorerev = 0;
#endif /* BCMQT */

	DHD_TRACE(("%s: Enter\n", __FUNCTION__));

	if (bus) {
		osh = bus->osh;
		ASSERT(osh);

		if (bus->dhd) {
#if defined(DEBUGGER) || defined(DHD_DSCOPE)
			debugger_close();
#endif /* DEBUGGER || DHD_DSCOPE */

			/* stop watchdog timer and thread */
			dhd_os_wd_timer(bus->dhd, 0);
			dhd_os_stop_wd_thread(bus->dhd);

			dongle_isolation = bus->dhd->dongle_isolation;

			dhdpcie_bus_remove_prep(bus);

			if (bus->intr) {
				dhdpcie_free_irq(bus);
			}
#ifdef BCMQT
			if (IDMA_ACTIVE(bus->dhd)) {
			/**
			 * On FPGA during exit path force set "IDMA Control Register"
			 * to default value 0x0. Otherwise host dongle syc for IDMA fails
			 * during next IDMA initilization(without system reboot)
			 */
				buscorerev = bus->sih->buscorerev;
				si_corereg(bus->sih, bus->sih->buscoreidx,
					IDMAControl, ~0, 0);
			}
#endif /* BCMQT */
			dhdpcie_bus_release_dongle(bus, osh, dongle_isolation, TRUE);
			/* free host fw buffer if there is any */
			dhd_dma_buf_free(bus->dhd, &bus->hybridfw_dma_buf, "hybridfw");
			dhd_detach(bus->dhd);
			dhd_free(bus->dhd);
			bus->dhd = NULL;
		}

		/* unmap the regs and tcm here!! */
		if (bus->regs) {
			dhdpcie_bus_reg_unmap(osh, (ulong)bus->regs, DONGLE_REG_MAP_SIZE);
			bus->regs = NULL;
		}
		if (bus->tcm) {
			dhdpcie_bus_reg_unmap(osh, (ulong)bus->tcm, DONGLE_TCM_MAP_SIZE);
			bus->tcm = NULL;
		}

		dhdpcie_bus_release_malloc(bus, osh);

#ifdef DHD_DEBUG
		if (bus->console.buf != NULL)
			MFREE(osh, bus->console.buf, bus->console.bufsize);
#endif

		/* free pcie_ipc_hme if there is any */
		if (bus->pcie_ipc_hme != NULL) {
			MFREE(osh, bus->pcie_ipc_hme, bus->pcie_ipc_hme->size);
			bus->pcie_ipc_hme = NULL;
		}

		/* Finally free bus info */
		MFREE(osh, bus, sizeof(dhd_bus_t));
	}

	DHD_TRACE(("%s: Exit\n", __FUNCTION__));
} /* dhdpcie_bus_release */

void
dhdpcie_bus_release_dongle(dhd_bus_t *bus, osl_t *osh, bool dongle_isolation, bool reset_flag)
{
	if (bus->dhd == NULL) {
		DHD_ERROR(("%s: bus->dhd is NULL\n", __FUNCTION__));
		return;
	}

	DHD_TRACE(("%s: Enter bus->dhd %p bus->dhd->dongle_reset %d \n", __FUNCTION__,
		bus->dhd, bus->dhd->dongle_reset));

	if (bus->dhd->dongle_reset && reset_flag) {
		DHD_TRACE(("%s Exit\n", __FUNCTION__));
		return;
	}

	if (bus->sih) {

		if (!dongle_isolation && bus->dhd->is_pcie_watchdog_reset == FALSE) {
			dhdpcie_bus_watchdog_reset(bus);
			bus->dhd->is_pcie_watchdog_reset = TRUE;
		}

		if (bus->ltrsleep_on_unload) {
			si_corereg(bus->sih, bus->sih->buscoreidx,
				OFFSETOF(sbpcieregs_t, u.pcie2.ltr_state), ~0, 0);
		}

		if (bus->sih->buscorerev == 13)
			 pcie_serdes_iddqdisable(bus->osh, bus->sih, (sbpcieregs_t *)(bus->regs));

		if (bus->sih != NULL) {
			si_detach(bus->sih);
			bus->sih = NULL;
		}
		if (bus->vars && bus->varsz)
			MFREE(osh, bus->vars, bus->varsz);
		bus->vars = NULL;
	}

	DHD_TRACE(("%s Exit\n", __FUNCTION__));
}

uint32
dhdpcie_bus_cfg_read_dword(dhd_bus_t *bus, uint32 addr, uint32 size)
{
	uint32 data = OSL_PCI_READ_CONFIG(bus->osh, addr, size);

	return data;
}

/** 32 bit config write */
void
dhdpcie_bus_cfg_write_dword(dhd_bus_t *bus, uint32 addr, uint32 size, uint32 data)
{
	OSL_PCI_WRITE_CONFIG(bus->osh, addr, size, data);
}

void
dhdpcie_bus_cfg_set_bar0_win(dhd_bus_t *bus, uint32 data)
{
	OSL_PCI_WRITE_CONFIG(bus->osh, PCI_BAR0_WIN, 4, data);
}

void
dhdpcie_bus_dongle_setmemsize(struct dhd_bus *bus, int mem_size)
{
	int32 min_size =  DONGLE_MIN_MEMSIZE;
	/* Restrict the memsize to user specified limit */
	DHD_ERROR(("user: Restrict the dongle ram size to %d, min accepted %d\n",
		dhd_dongle_memsize, min_size));
	if ((dhd_dongle_memsize > min_size) &&
		(dhd_dongle_memsize < (int32)bus->orig_ramsize))
		bus->ramsize = dhd_dongle_memsize;
}

void
dhdpcie_bus_release_malloc(dhd_bus_t *bus, osl_t *osh)
{
	DHD_TRACE(("%s: Enter\n", __FUNCTION__));

	if (bus->dhd && bus->dhd->dongle_reset)
		return;

	if (bus->vars && bus->varsz) {
		MFREE(osh, bus->vars, bus->varsz);
		bus->vars = NULL;
	}

	DHD_TRACE(("%s: Exit\n", __FUNCTION__));
}

/** Stop bus module: clear pending frames, disable data flow */
void
dhd_bus_stop(struct dhd_bus *bus, bool enforce_mutex)
{
	uint32 status;

	DHD_TRACE(("%s: Enter\n", __FUNCTION__));

	if (!bus->dhd)
		return;

	if (bus->dhd->busstate == DHD_BUS_DOWN) {
		DHD_ERROR(("%s: already down by net_dev_reset\n", __FUNCTION__));
		goto done;
	}

	bus->dhd->busstate = DHD_BUS_DOWN;
	dhdpcie_bus_intr_disable(bus);
	status =  dhdpcie_bus_cfg_read_dword(bus, PCIIntstatus, 4);
	dhdpcie_bus_cfg_write_dword(bus, PCIIntstatus, 4, status);

#ifdef OEM_ANDROID
	if (!dhd_download_fw_on_driverload) {
		dhd_dpc_kill(bus->dhd);
	}
#endif /* OEM_ANDROID */

	/* Clear rx control and wake any waiters */
	dhd_os_set_ioctl_resp_timeout(IOCTL_DISABLE_TIMEOUT);
	dhd_os_ioctl_resp_wake(bus->dhd);

done:
	return;
}

/**
 * Watchdog timer function.
 * @param dhd   Represents a specific hardware (dongle) instance that this DHD manages
 */
bool dhd_bus_watchdog(dhd_pub_t *dhd)
{
#ifdef DHD_DEBUG
	dhd_bus_t *bus;
	bus = dhd->bus;

	/* Poll for console output periodically */
	if (dhd->dhd_console_ms != 0) {
		if (dhd->busstate == DHD_BUS_DATA || dhd->dongle_trap_occured == TRUE) {
			bus->console.count += dhd_watchdog_ms;
			if (bus->console.count >= dhd->dhd_console_ms) {
				bus->console.count -= dhd->dhd_console_ms;
				/* Make sure backplane clock is on */
				if (dhdpcie_bus_readconsole(bus) < 0)
					dhd->dhd_console_ms = 0; /* On error, stop trying */
			}
		}
	}
#endif /* DHD_DEBUG */

#ifdef PCIE_OOB
	/* If haven't communicated with device for a while, deassert the Device_Wake GPIO */
	if (dhd_doorbell_timeout != 0 &&
	    !(bus->dhd->busstate == DHD_BUS_SUSPENDING ||
	      bus->dhd->busstate == DHD_BUS_SUSPENDED) &&
	    dhd_timeout_expired(&bus->doorbell_timer)) {
		dhd_bus_set_device_wake(bus, FALSE);
	}
#endif /* PCIE_OOB */

	/* Call the msgbuf module watchdog */
#if defined(STB) && !defined(STBAP)
	if ((dhd->busstate == DHD_BUS_DATA) && (dhd->iswl) && dhd->up) {
#else
	if ((dhd->busstate == DHD_BUS_DATA) && (dhd->iswl)) {
#endif /* STB && !STBAP */
		dhd_msgbuf_watchdog(dhd);
	}

	return FALSE;
} /* dhd_bus_watchdog */

#if defined(SUPPORT_MULTIPLE_REVISION)
static int
concate_revision_bcm4358(dhd_bus_t *bus, char *fw_path, char *nv_path)
{
	uint32 chip_id, chip_ver;
#if defined(SUPPORT_MULTIPLE_CHIPS)
	char chipver_tag[20] = "_4358";
#else
	char chipver_tag[10] = {0, };
#endif /* SUPPORT_MULTIPLE_CHIPS */

	chip_id = si_chipid(bus->sih);
	chip_ver = bus->sih->chiprev;
	if (chip_ver == 0) {
		DHD_ERROR(("----- CHIP 4358 A0 -----\n"));
		strcat(chipver_tag, "_a0");
	} else if (chip_ver == 1) {
		DHD_ERROR(("----- CHIP 4358 A1 -----\n"));
#if defined(SUPPORT_MULTIPLE_CHIPS) || defined(SUPPORT_MULTIPLE_MODULE_CIS)
		strcat(chipver_tag, "_a1");
#endif /* defined(SUPPORT_MULTIPLE_CHIPS) || defined(SUPPORT_MULTIPLE_MODULE_CIS) */
	} else if (chip_ver == 3) {
		DHD_ERROR(("----- CHIP 4358 A3 -----\n"));
#if defined(SUPPORT_MULTIPLE_CHIPS)
		strcat(chipver_tag, "_a3");
#endif /* SUPPORT_MULTIPLE_CHIPS */
	} else {
		DHD_ERROR(("----- Unknown chip version, ver=%x -----\n", chip_ver));
	}

	strcat(fw_path, chipver_tag);

#if defined(SUPPORT_MULTIPLE_MODULE_CIS) && defined(USE_CID_CHECK)
	if (chip_ver == 1 || chip_ver == 3) {
		int ret = dhd_check_module_b85a(bus->dhd);
		if ((chip_ver == 1) && (ret < 0)) {
			memset(chipver_tag, 0x00, sizeof(chipver_tag));
			strcat(chipver_tag, "_b85");
			strcat(chipver_tag, "_a1");
		}
	}

	DHD_ERROR(("%s: chipver_tag %s \n", __FUNCTION__, chipver_tag));
#endif /* defined(SUPPORT_MULTIPLE_MODULE_CIS) && defined(USE_CID_CHECK) */

#if defined(SUPPORT_MULTIPLE_BOARD_REV)
	if (system_rev >= 10) {
		DHD_ERROR(("----- Board Rev  [%d]-----\n", system_rev));
		strcat(chipver_tag, "_r10");
	}
#endif /* SUPPORT_MULTIPLE_BOARD_REV */
	strcat(nv_path, chipver_tag);

	return 0;
}

static int
concate_revision_bcm4359(dhd_bus_t *bus, char *fw_path, char *nv_path)
{
	uint32 chip_id, chip_ver;
	char chipver_tag[10] = { 0 };

	chip_id = si_chipid(bus->sih);
	chip_ver = bus->sih->chiprev;
	if (chip_ver == 4) {
		DHD_ERROR(("----- CHIP 4359 B0 -----\n"));
		memcpy(chipver_tag + strlen(chipver_tag), "_b0", strlen("_b0"));
	} else if (chip_ver == 5) {
		DHD_ERROR(("----- CHIP 4359 B1 -----\n"));
		memcpy(chipver_tag + strlen(chipver_tag), "_b1", strlen("_b1"));
	} else {
		DHD_ERROR(("----- Unknown chip version, ver=%x -----\n", chip_ver));
		return -1;
	}

	strcat(fw_path, chipver_tag);
	strcat(nv_path, chipver_tag);

	return 0;
}

int
concate_revision(dhd_bus_t *bus, char *fw_path, char *nv_path)
{
	int res = 0;

	if (!bus || !bus->sih) {
		DHD_ERROR(("%s:Bus is Invalid\n", __FUNCTION__));
		return -1;
	}

	DHD_ERROR(("concate_revision \n"));

	if (!fw_path || !nv_path) {
		DHD_ERROR(("fw_path or nv_path is null.\n"));
		return res;
	}

	switch (si_chipid(bus->sih)) {

	case BCM43569_CHIP_ID:
	case BCM4358_CHIP_ID:
		res = concate_revision_bcm4358(bus, fw_path, nv_path);
		break;
	case BCM4355_CHIP_ID:
	case BCM4359_CHIP_ID:
		res = concate_revision_bcm4359(bus, fw_path, nv_path);
		break;
	default:
		DHD_ERROR(("REVISION SPECIFIC feature is not required\n"));
		return res;
	}

	return res;
}
#endif /* SUPPORT_MULTIPLE_REVISION */

/**
 * Loads firmware given by caller supplied path and nvram image into PCIe dongle.
 *
 * BCM_REQUEST_FW specific :
 * Given the chip type, determines the to be used file paths within /lib/firmware/brcm/ containing
 * firmware and nvm for that chip. If the download fails, retries download with a different nvm file
 *
 * BCMEMBEDIMAGE specific:
 * If bus->fw_path is empty, or if the download of bus->fw_path failed, firmware contained in header
 * file will be used instead.
 *
 * @return BCME_OK on success
 */
int
dhd_bus_download_firmware(struct dhd_bus *bus, osl_t *osh,
                          char *pfw_path, char *pnv_path)
{
	int ret;

	bus->fw_path = pfw_path;
	bus->nv_path = pnv_path;

#if defined(SUPPORT_MULTIPLE_REVISION) && !defined(OEM_ANDROID)
	if (concate_revision(bus, bus->fw_path, bus->nv_path) != 0) {
		DHD_ERROR(("%s: fail to concatnate revison \n",
			__FUNCTION__));
		return BCME_BADARG;
	}
#endif /* SUPPORT_MULTIPLE_REVISION */

#if defined(BCM_ROUTER_DHD) && defined(EXTFDIMGPATH)
	if (concate_extfdimg_name(bus, bus->fw_path) != BCME_OK) {
		DHD_ERROR(("%s: fail to concatnate chip specific file name \n",
			__FUNCTION__));
		return BCME_BADARG;
	}
#endif /* defined(BCM_ROUTER_DHD) && defined(EXTFDIMGPATH) */

	DHD_ERROR(("%s: firmware path=%s, nvram path=%s\n",
		__FUNCTION__, bus->fw_path, bus->nv_path));

	ret = dhdpcie_download_firmware(bus, osh);

	return ret;
}

/**
 * Loads firmware given by 'bus->fw_path' into PCIe dongle.
 *
 * BCM_REQUEST_FW specific :
 * Given the chip type, determines the to be used file paths within /lib/firmware/brcm/ containing
 * firmware and nvm for that chip. If the download fails, retries download with a different nvm file
 *
 * BCMEMBEDIMAGE specific:
 * If bus->fw_path is empty, or if the download of bus->fw_path failed, firmware contained in header
 * file will be used instead.
 *
 * @return BCME_OK on success
 */
static int
dhdpcie_download_firmware(struct dhd_bus *bus, osl_t *osh)
{
	int ret = 0;
#if defined(BCM_REQUEST_FW)
#define CHIP_43602_CHIPREV_A0   0
#define CHIP_43602_CHIPREV_A1   1
#define CHIP_43602_CHIPREV_A3   3
#define CHIP_4366_CHIPREV_B1    3
#define CHIP_4366_CHIPREV_C0    4
#define CHIP_43570_CHIPREV_A0   0
#define CHIP_43570_CHIPREV_A2   2
#define CHIP_43684_CHIPREV_B0   2
#define CHIP_43684_CHIPREV_B1   3
#define CHIP_43684_CHIPREV_C0   4
#define CHIP_43694_CHIPREV_B1   3
#define CHIP_43794_CHIPREV_A0   0
#define CHIP_43794_CHIPREV_A1   1
#define CHIP_43794_CHIPREV_B0   2
#define CHIP_6715_CHIPREV_A0    0
#define CHIP_6715_CHIPREV_A1    1
#define CHIP_6715_CHIPREV_B0    2

#define CHIPID_NONE            -1
	struct chip_reqfw_map {
		uint32 chipid;
		uint32 chiprev;
		char *chip_name;
		char *fw_name;
		char *nv_name;
	} static chip_reqfw_map_table [] =
	{
		{BCM43602_CHIP_ID, CHIP_43602_CHIPREV_A0,
		"bcm43602a0", "bcm43602a0-firmware.bin", "bcm43602a0.nvm"},
		{BCM43602_CHIP_ID, CHIP_43602_CHIPREV_A1,
		"bcm43602a1", "bcm43602a1-firmware.bin", "bcm43602a1.nvm"},
#if defined(STB) && (!defined(STBAP) || !defined(BCM_ROUTER_DHD))
		{BCM43602_CHIP_ID, CHIP_43602_CHIPREV_A3,
		"bcm43602a3", "bcm43602a3-firmware.bin", "bcm43602a3.nvm"},
#endif
		{BCM4366_CHIP_ID, CHIP_4366_CHIPREV_C0,
		"bcm4366c0", "bcm4366c0-firmware.bin", "bcm4366c0.nvm"},
		{BCM43664_CHIP_ID, CHIP_4366_CHIPREV_C0,
		"bcm4366c0", "bcm4366c0-firmware.bin", "bcm4366c0.nvm"},
		{BCM4363_CHIP_ID, CHIP_4366_CHIPREV_C0,
		"bcm4363c0", "bcm4363c0-firmware.bin", "bcm4363c0.nvm"},
		{BCM43570_CHIP_ID, CHIP_43570_CHIPREV_A0,
		"bcm43570a0", "bcm43570a0-firmware.bin", "bcm43570a0.nvm"},
		{BCM43570_CHIP_ID, CHIP_43570_CHIPREV_A2,
		"bcm43570a2", "bcm43570a2-firmware.bin", "bcm43570a2.nvm"},
		{BCM43684_CHIP_ID, CHIP_43684_CHIPREV_B0,
		"bcm43684b0", "bcm43684b0-firmware.bin", "bcm43684b0.nvm"},
		{BCM43684_CHIP_ID, CHIP_43684_CHIPREV_B1,
		"bcm43684b1", "bcm43684b0-firmware.bin", "bcm43684b1.nvm"},
		{BCM43684_CHIP_ID, CHIP_43684_CHIPREV_C0,
		"bcm43684c0", "bcm43684c0-firmware.bin", "bcm43684c0.nvm"},
		{BCM43694_CHIP_ID, CHIP_43694_CHIPREV_B1,
		"bcm43694b1", "bcm43694b0-firmware.bin", "bcm43694b1.nvm"},
		{BCM43794_CHIP_ID, CHIP_43794_CHIPREV_B0,
		"bcm43794b0", "bcm43794b0-firmware.bin", "bcm43794b0.nvm"},
		{BCM6715_CHIP_ID, CHIP_6715_CHIPREV_B0,
		"bcm6715b0", "bcm6715b0-firmware.bin", "bcm6715b0.nvm"},
		/* {for a given chipid, chiprev, what is the index (the above enum) */
		{CHIPID_NONE, 0, 0, 0, 0} /* CHIPID_NONE is -1, used to mark end of list */
	};

#if defined(OEM_ANDROID)
	char ini_path[64] = "/vendor/firmware/broadcom/dhd/firmware/brcm/";      /* initial path */
#else
	char ini_path[64] = "/lib/firmware/brcm/";	/* initial path */
#endif
	char fw_path[128];				/* path to firmware image */
	char nv_path[64];				/* path to nvram vars file */
	uint32 chipid, chiprev;
	struct chip_reqfw_map *p_reqfw_index;

	bus->fw_path = fw_path;
	bus->nv_path = nv_path;
	chipid = 0;
	p_reqfw_index = &chip_reqfw_map_table[0];
	while (chipid != CHIPID_NONE) {
		chipid = p_reqfw_index->chipid;
		chiprev = p_reqfw_index->chiprev;

		if ((chipid == bus->sih->chip) && (chiprev == bus->sih->chiprev)) {
			break;
		}
		p_reqfw_index++;
	}

	if (chipid != CHIPID_NONE) {
		/* load generic nvram file */
		snprintf(bus->nv_path, sizeof(nv_path), "%s%s", ini_path, p_reqfw_index->nv_name);
		/* load firmware */
		snprintf(bus->fw_path, sizeof(fw_path), "%s%s", ini_path, p_reqfw_index->fw_name);
	} else {
		DHD_ERROR(("\n\n\n\033[1m\033[34m"
			"##################################################################"
			"\n# %s: reqfw not available for chipid = 0x%x chiprev = %d\n"
			"##################################################################"
			"\033[0m\n",
			__FUNCTION__, bus->sih->chip, bus->sih->chiprev));
		return 0;
	}
#endif /* BCM_REQUEST_FW */

	DHD_OS_WAKE_LOCK(bus->dhd);
	ret = _dhdpcie_download_firmware(bus);

#if defined(BCM_REQUEST_FW) && !defined(OEM_ANDROID)
	if (ret) {
		uint boardtype = bus->sih->boardtype;
		uint boardrev = bus->sih->boardrev;
		char *ptr, dev_name[64] = { 0 };

		/* generic nvram file missing - load the board specific nvram file */
		sprintf(dev_name, "%s", dhdpcie_bus_get_device_name(bus));
		/* remove special character for pci device name */
		for (ptr = dev_name; *ptr != '\0'; ptr++) {
			if ((*ptr == ':') || (*ptr == '.')) {
				*ptr = '_';
				continue;
			}
		}
		snprintf(bus->nv_path, sizeof(nv_path), "%s%s-%02x-%02x-%s.nvm",
			ini_path, p_reqfw_index->chip_name, boardtype, boardrev, dev_name);
		DHD_INFO(("%s: DHD_NVFILE FW=%s\n", __FUNCTION__, bus->nv_path));
		ret = _dhdpcie_download_firmware(bus);
	}
#endif /* BCM_REQUEST_FW */

	DHD_OS_WAKE_UNLOCK(bus->dhd);
	return ret;
} /* dhdpcie_download_firmware */

/*
 * Allocate a DMA-able buffer for the host resident firmware using PCIe IPC
 * PCIE_IPC_HYBRIDFW HME attributes. As the dongle PCIe IPC handshake has not
 * commenced, bcmpcie.h defined HME attributes for HYBRIDFW (ie HME_USER_HMOSWP)
 * are used.
 */
static int
dhdpcie_hybridfw_dma_buf_alloc(struct dhd_bus *bus, uint32 len)
{
	dhd_dma_buf_t * hybridfw_dma_buf = &bus->hybridfw_dma_buf;

	/* XXX If dongle is in reset state, and this is a re-download of a new
	 * hybridfw, then the earlier dma_buf would have moved from
	 * dhd_bus::hybridfw_dma_buf into the dhd_proto::hme::user_dma_buf[]
	 * Consequently, a double hybridfw worth of memory is used, or re-download.
	 * The next test for hybridfw_dma_buf exists does not check into dhd_proto.
	 */

	/* Check whether current hybridfw_dma_buf exists and can be re-used */
	if ((hybridfw_dma_buf->va != NULL) && (hybridfw_dma_buf->len < len)) {
		/* Cannot re-use, so free and reset the hybridfw_dma_buf */
		dhd_dma_buf_free(bus->dhd, hybridfw_dma_buf, "hybridfw");
	} // else hybridfw_dma_buf::len is NOT downsized, under reuse case

	if (hybridfw_dma_buf->va == NULL)
	{
		/* Compose HME memory attributes using PCIe IPC defaults */
		pcie_ipc_hme_user_t pcie_ipc_hybridfw_hme; // on stack

		pcie_ipc_hybridfw_hme.user_id    = PCIE_IPC_HYBRIDFW_HME_USER;
		pcie_ipc_hybridfw_hme.align_bits = PCIE_IPC_HYBRIDFW_ALIGN_BITS;
		pcie_ipc_hybridfw_hme.bound_bits = PCIE_IPC_HYBRIDFW_BOUND_BITS;
		pcie_ipc_hybridfw_hme.flags      = PCIE_IPC_HYBRIDFW_MEM_FLAGS;
		pcie_ipc_hybridfw_hme.pages      = PCIE_IPC_HME_PAGES(len);
		strncpy(pcie_ipc_hybridfw_hme.name, "HYBRID",
		        PCIE_IPC_HME_USER_NAME_SIZE - 1);

		/* Try to allocate a PCIe IPC HYBRIDFW HME compliant DMA-able buffer */
		dhd_hme_buf_alloc_try(bus, hybridfw_dma_buf, &pcie_ipc_hybridfw_hme);
	}

	return (hybridfw_dma_buf->va == NULL) ? BCME_NOMEM : BCME_OK;
}

/* Get length of each portion of hybrid fw binary from the header */
static int
dhdpcie_hybridfw_get_next_block(char * fptr, int *fsize, uint32 *type, uint32 *len)
{
	struct portion_hdr {
		uint32 type;
		uint32 len;
	} hdr;
	int ret;

	/* read and verify header */
	if (*fsize <= sizeof(hdr)) {
		return BCME_BADLEN;
	}

	ret = dhd_os_get_image_block((char *)&hdr, sizeof(hdr), fptr);
	if (ret <= 0) {
		return BCME_ERROR;
	}

	*fsize -= sizeof(hdr);
	*type = ltoh32(hdr.type);
	*len = ltoh32(hdr.len);

	if ((*len > (uint32)*fsize) || ((int)*len < 0)) {
		return BCME_BADLEN;
	}

	DHD_ERROR(("%s Found section %d with length %d\n", __FUNCTION__, hdr.type, hdr.len));

	return BCME_OK;
}

/**
 * hybrid firmware download handler
 *
 * Parse, prepare and download a hybrid firmware
 * - Identify a hybrid firmware
 * - Place the host offload portion in an allocated DMA consistent buffer
 * - Modifying the host portion function pointers according to info table
 */
static int
dhdpcie_hybridfw_download(struct dhd_bus *bus, char *fp)
{
	uint32 magic_num;
	int ret = BCME_OK;
	char *dnglfw = NULL;
	int dnglfw_sz = 0;
	int fsize;
	int offset = 0;
	uint32 type = 0, len = 0;
	void *ptr = NULL;
#if defined(STB) || defined(STBAP) || (0 && (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, \
	0)))
	/* dhd is required for OSL_CACHE_FLUSH in STB builds */
	dhd_pub_t *dhd = bus->dhd;
#endif

	fsize = dhd_os_get_image_size(fp);

	/* Verify magic number */
	if (fsize < sizeof(magic_num)) {
		return BCME_UNSUPPORTED;
	}
	ret = dhd_os_get_image_block((char *)&magic_num, sizeof(magic_num), fp);
	if (ret <= 0) {
		return BCME_ERROR;
	}
	magic_num = ltoh32(magic_num);
	if (magic_num != PCIE_IPC_HYBRIDFW_MAGICNUM) {
		return BCME_UNSUPPORTED;
	}
	fsize -= sizeof(magic_num);

	do {
		ret = dhdpcie_hybridfw_get_next_block(fp, &fsize, &type, &len);
		if (ret != BCME_OK) {
			break;
		}

		if (len == 0) {
			continue;
		}

		if ((ptr = MALLOC(bus->dhd->osh, len)) == NULL) {
			ret = BCME_NOMEM;
			break;
		}

		len = dhd_os_get_image_block(ptr, len, fp);
		if (len <= 0) {
			MFREE(bus->dhd->osh, ptr, len);
			ret = BCME_ERROR;
			break;
		}
		fsize -= len;

		switch (type) {
			case PCIE_IPC_HYBRIDFW_TYPE_DNGL:
				/* cannot have more than one RAM image blocks */
				if (dnglfw_sz) {
					MFREE(bus->dhd->osh, ptr, len);
					ret = BCME_ERROR;
					break;
				}
				/* RAM portion of the FW image */
				dnglfw = ptr;
				dnglfw_sz = len;
				if ((uint32)len > bus->ramsize) {
					ret = BCME_BADLEN;
					break;
				}
				break;

			case PCIE_IPC_HYBRIDFW_TYPE_HOST:
			{
				dhd_dma_buf_t * hybridfw_dma_buf;

				/* Allocate into bus::hybridfw_dma_buf a PCIe IPC HYBRIDFW
				 * compliant DMA-able buffer
				 */
				ret = dhdpcie_hybridfw_dma_buf_alloc(bus, len);
				if (ret != BCME_OK) {
					MFREE(bus->dhd->osh, ptr, len);
					break;
				}

				hybridfw_dma_buf = &bus->hybridfw_dma_buf;
				ASSERT(hybridfw_dma_buf->va != (void*)NULL);

				memcpy(hybridfw_dma_buf->va, ptr, len);
				MFREE(bus->dhd->osh, ptr, len);

				OSL_CACHE_FLUSH((void *)hybridfw_dma_buf->va, len);

				DHD_ERROR(("%s: hybridfw" DHD_DMA_BUF_FMT "\n",
					__FUNCTION__, DHD_DMA_BUF_VAL(bus->hybridfw_dma_buf)));
				break;
			}

			default:
				ret = BCME_ERROR;
				break;
		}

	} while (!ret && (fsize > 0));

	if (ret != BCME_OK) {
		DHD_ERROR(("%s: err:%d, fsize:%d, t:%d, l:%d\n",
			__FUNCTION__, ret, fsize, type, len));
		goto exit;
	}

	if ((uint32*)dnglfw == NULL) {
		DHD_ERROR(("%s: Dongle image should be present in combo file\n",
			__FUNCTION__));
		ret = BCME_ERROR;
		goto exit;
	}

	/* Store the reset instruction to be written in 0 */
	bus->resetinstr = *(((uint32*)dnglfw));
	/* Add start of RAM address to the address given by user */
	offset += bus->dongle_ram_base;

	dhdpcie_bus_membytes(bus, PCIE_SYSMEM_WRITE, offset,
	                     (uint8 *)dnglfw, dnglfw_sz);

#ifdef DHD_DEBUG
	{
		unsigned char *ularray = NULL;
		unsigned int uploaded_len;
		daddr32_t daddr32 = 0U;
		unsigned int remaining_len;

		uploaded_len = 0;
		ularray = MALLOC(bus->dhd->osh, dnglfw_sz);
		if (ularray == NULL) {
			DHD_ERROR(("%s: skipping image upload compare\n", __FUNCTION__));
			goto upload_err;
		}
		/* Upload image to verify downloaded contents. */
		daddr32 = bus->dongle_ram_base;
		memset(ularray, 0xaa, dnglfw_sz);
		while (uploaded_len  < dnglfw_sz) {
			remaining_len = dnglfw_sz - uploaded_len;
			len = (remaining_len >= MEMBLOCK) ? MEMBLOCK : remaining_len;
			dhdpcie_bus_membytes(bus, PCIE_SYSMEM_READ, daddr32,
			                     (uint8 *)(ularray + uploaded_len), len);
			uploaded_len += len;
			daddr32 += MEMBLOCK;
		}

		if (memcmp(dnglfw, ularray, dnglfw_sz)) {
			DHD_ERROR(("%s: Downloaded image is corrupted.\n",
				__FUNCTION__));
			goto upload_err;

		} else {
			DHD_ERROR(("%s: Download, Upload and compare succeeded.\n",
				__FUNCTION__));
		}
upload_err:
		if (ularray) {
			MFREE(bus->dhd->osh, ularray, dnglfw_sz);
		}
	}
#endif /* DHD_DEBUG */

exit:
	if (dnglfw) {
		MFREE(bus->dhd->osh, dnglfw, dnglfw_sz);
	}

	if (ret) {
		dhd_dma_buf_free(bus->dhd, &bus->hybridfw_dma_buf, "hybridfw");
	}

	return ret;
}

/**
 * Downloads a file containing firmware into dongle memory. In case of a .bea file, the DHD
 * is updated with the event logging partitions within that file as well.
 *
 * @param pfw_path    Path to .bin or .bea file
 */
static int
dhdpcie_download_code_file(struct dhd_bus *bus, char *pfw_path)
{
	int bcmerror = BCME_ERROR;
	daddr32_t daddr32 = 0U;
	int len = 0;
	char *imgbuf = NULL;
	uint8 *memblock = NULL, *memptr;

	DHD_ERROR(("%s: download firmware %s\n", __FUNCTION__, pfw_path));

	/* Should succeed in opening image if it is actually given through registry
	 * entry or in module param.
	 */
	imgbuf = dhd_os_open_image(bus->dhd, pfw_path);
	if (imgbuf == NULL)
		goto err;

	/* dhdpcie_hybridfw_download return BCME_UNSUPPORTED if the binary
	 * doesn't have a recognizable format. Continue to previous routine
	 * in such case. Return and propagate the result for BCME_OK or
	 * other errors
	 */
	bcmerror = dhdpcie_hybridfw_download(bus, imgbuf);
	if (bcmerror != BCME_UNSUPPORTED) {
		goto err;
	}
	bcmerror = BCME_OK;

	/* Close and re-open the image file to reset the file pointer.
	 * Needed because dhdpcie_hybridfw_download() already read 4 bytes from the file.
	 */
	dhd_os_close_image(bus->dhd, imgbuf);
	imgbuf = dhd_os_open_image(bus->dhd, pfw_path);
	if (imgbuf == NULL) {
		goto err;
	}

	memptr = memblock = MALLOC(bus->dhd->osh, MEMBLOCK + DHD_SDALIGN);
	if (memblock == NULL) {
		DHD_ERROR(("%s: Failed to allocate memory %d bytes\n", __FUNCTION__, MEMBLOCK));
		goto err;
	}
	if ((uint32)(uintptr)memblock % DHD_SDALIGN)
		memptr += (DHD_SDALIGN - ((uint32)(uintptr)memblock % DHD_SDALIGN));

	/* Download image with MEMBLOCK size */
	while ((len = dhd_os_get_image_block((char*)memptr, MEMBLOCK, imgbuf))) {
		if (len < 0) {
			DHD_ERROR(("%s: dhd_os_get_image_block failed (%d)\n", __FUNCTION__, len));
			bcmerror = BCME_ERROR;
			goto err;
		}
		/* check if CR4/CA7 */
		if (si_setcore(bus->sih, ARMCR4_CORE_ID, 0) ||
			si_setcore(bus->sih, ARMCA7_CORE_ID, 0)) {
			/* if address is 0, store the reset instruction to be written in 0 */
			if (daddr32 == 0U) {
				bus->resetinstr = *(((uint32*)memptr));
				/* Add start of RAM address to the address given by user */
				daddr32 += bus->dongle_ram_base;
			}
		}
		dhdpcie_bus_membytes(bus, PCIE_SYSMEM_WRITE, daddr32,
		                     (uint8 *)memptr, len);
		daddr32 += MEMBLOCK;
	}

err:
	if (memblock)
		MFREE(bus->dhd->osh, memblock, MEMBLOCK + DHD_SDALIGN);

	if (imgbuf)
		dhd_os_close_image(bus->dhd, imgbuf);

	return bcmerror;
} /* dhdpcie_download_code_file */

static int
dhdpcie_download_nvram(struct dhd_bus *bus)
{
	int bcmerror = BCME_ERROR;
	uint len;
	char * memblock = NULL;
	char *bufp;
	char *pnv_path;
	bool nvram_file_exists;
	bool nvram_uefi_exists = FALSE;
	bool local_alloc = FALSE;
	pnv_path = bus->nv_path;

	nvram_file_exists = ((pnv_path != NULL) && (pnv_path[0] != '\0'));

	/* First try UEFI */
	len = MAX_NVRAMBUF_SIZE;
	dhd_get_download_buffer(bus->dhd, NULL, download_type_NVRAM, &memblock, &len);

	/* If UEFI empty, then read from file system */
	if ((len == 0) || (memblock[0] == '\0')) {

		if (nvram_file_exists) {
			len = MAX_NVRAMBUF_SIZE;
			dhd_get_download_buffer(bus->dhd, pnv_path, download_type_NVRAM, &memblock,
				&len);
			if ((len <= 0 || len > MAX_NVRAMBUF_SIZE)) {
				goto err;
			}
#ifdef BCM_ROUTER_DHD
			if (bus->nvram_params_len) {
				if ((len + bus->nvram_params_len) < MAX_NVRAMBUF_SIZE) {
					/* append to the existing params */
					memcpy(memblock + len, bus->nvram_params,
						bus->nvram_params_len);
					len += bus->nvram_params_len;
				} else {
					DHD_ERROR(("%s: Failed to append nvram params %d bytes\n",
						__FUNCTION__, bus->nvram_params_len));
					ASSERT((len + bus->nvram_params_len) < MAX_NVRAMBUF_SIZE);
				}
			}
#endif /* BCM_ROUTER_DHD */
		}
#ifdef BCM_ROUTER_DHD
		else if (bus->nvram_params_len) {
			memblock = MALLOCZ(bus->dhd->osh, MAX_NVRAMBUF_SIZE);
			if (memblock == NULL) {
				DHD_ERROR(("%s: Failed to allocate memory %d bytes\n",
					__FUNCTION__, MAX_NVRAMBUF_SIZE));
				goto err;
			}
			local_alloc = TRUE;
			/* nvram is string with null terminated. cannot use strlen */
			len = bus->nvram_params_len;
			ASSERT(len < MAX_NVRAMBUF_SIZE);
			memcpy(memblock, bus->nvram_params, len);
		}
#endif /* BCM_ROUTER_DHD */
		else {
			/* For SROM OTP no external file or UEFI required */
			bcmerror = BCME_OK;
		}
	} else {
		nvram_uefi_exists = TRUE;
	}

	if (len > 0 && len < MAX_NVRAMBUF_SIZE) {
		bufp = (char *) memblock;

#ifdef CACHE_FW_IMAGES
		if (bus->processed_nvram_params_len) {
			len = bus->processed_nvram_params_len;
		}

		if (!bus->processed_nvram_params_len) {
			bufp[len] = 0;
			if (nvram_uefi_exists || nvram_file_exists) {
				len = process_nvram_vars(bufp, len);
				bus->processed_nvram_params_len = len;
			}
		} else
#else
		{
			bufp[len] = 0;
			if (nvram_uefi_exists || nvram_file_exists) {
				len = process_nvram_vars(bufp, len);
			}
		}
#endif /* CACHE_FW_IMAGES */

#if defined(BCA_HNDROUTER)
		osl_nvram_vars_adjust_mac(bus->dhd->unit, memblock, &len);
#endif
		if (len % 4) {
			len += 4 - (len % 4);
		}
		bufp += len;
		*bufp++ = 0;
		if (len)
			bcmerror = dhdpcie_downloadvars(bus, memblock, len + 1);
		if (bcmerror) {
			DHD_ERROR(("%s: error downloading vars: %d\n",
				__FUNCTION__, bcmerror));
		}
	}

err:
	if (memblock) {
		if (local_alloc) {
			MFREE(bus->dhd->osh, memblock, MAX_NVRAMBUF_SIZE);
		} else {
			dhd_free_download_buffer(bus->dhd, memblock, MAX_NVRAMBUF_SIZE);
		}
	}

	return bcmerror;
}

#if defined(BCM_ROUTER_DHD) && (defined(BCMEMBEDIMAGE) || defined(EXTFDIMGPATH))

/*
 * Chipset 4366
 */
#define CHIPID_4365             BCM4365_CHIP_ID
#define CHIPID_4366             BCM4366_CHIP_ID
#define CHIPID_43664            BCM43664_CHIP_ID
#define CHIPID_43666            BCM43666_CHIP_ID
#define CHIPID_43465            BCM43465_CHIP_ID
#define CHIPID_43525            BCM43525_CHIP_ID
#define CHIP_4366_CHIPREV_C0    4
#define CHIP_4366_PKG_OPT       0
#define CHIP_4366_PKG_OPT4	4
#define CHIPIDSTR_4366          "4366"

/*
 * Chipset 4363
 */
#define CHIPID_4363             BCM4363_CHIP_ID
#define CHIP_4363_CHIPREV_C0    4
#define CHIP_4363_PKG_OPT       0
#define CHIPIDSTR_4363          "4363"

/*
 * Chipset 43684
 */
#define CHIPID_43684            BCM43684_CHIP_ID
#define CHIP_43684_CHIPREV_B0   2
#define CHIP_43684_CHIPREV_B1   3
#define CHIP_43684_CHIPREV_C0   4
#define CHIP_43684_PKG_OPT      0
#define CHIPIDSTR_43684         "43684"

#if defined(DHD_EAP)
/*
 * Chipset 43694
 */
#define CHIPID_43694            BCM43694_CHIP_ID
#define CHIP_43694_CHIPREV_B0   2
#define CHIP_43694_CHIPREV_B1   3
#define CHIP_43694_CHIPREV_C0   4
#define CHIP_43694_PKG_OPT      0
#define CHIPIDSTR_43694         "43694"

/*
 * Chipset 43794
 */
#define CHIPID_43794            BCM43794_CHIP_ID
#define CHIP_43794_CHIPREV_B0   2
#define CHIP_43794_PKG_OPT      0
#define CHIPIDSTR_43794         "43794"
#endif /* DHD_EAP */

/*
 * Chipset 6715
 */
#define CHIPID_6715            BCM6715_CHIP_ID
#define CHIP_6715_CHIPREV_B0   2
#define CHIP_6715_PKG_OPT      0
#define CHIPIDSTR_6715         "6715"

#define CHIPID_NONE            -1
#define CHIPREVSTR_A0           "a0"
#define CHIPREVSTR_A1           "a1"
#define CHIPREVSTR_A3           "a3"
#define CHIPREVSTR_B0           "b0"
#define CHIPREVSTR_C0           "c0"

enum chip_image_rev
{
	CHIP_4366_C0_CHIP_IMAGE,
	CHIP_43684_B0_CHIP_IMAGE,
	CHIP_43684_C0_CHIP_IMAGE,
#if defined(DHD_EAP)
	CHIP_43694_B0_CHIP_IMAGE,
	CHIP_43794_B0_CHIP_IMAGE,
#endif
	CHIP_4363_C0_CHIP_IMAGE,
	CHIP_6715_B0_CHIP_IMAGE,

	/* index in the above array */
	CHIP_NONE_CHIP_IMAGE
};

struct chip_image_map
{
	uint32 chipid;
	uint32 chiprev;
	uint32 chippkg;
	uint32 image_idx;
	char*  chipidname;
	char*  chiprevname;
} static chip_image_index_map_table [] =
{
/*
 * Chipset 4366c0
 */
	/* Unprogrammed */
	{CHIPID_4365, CHIP_4366_CHIPREV_C0, CHIP_4366_PKG_OPT, CHIP_4366_C0_CHIP_IMAGE,
	CHIPIDSTR_4366, CHIPREVSTR_C0},
	/* 4X4 */
	{CHIPID_4366, CHIP_4366_CHIPREV_C0, CHIP_4366_PKG_OPT, CHIP_4366_C0_CHIP_IMAGE,
	CHIPIDSTR_4366, CHIPREVSTR_C0},
	{CHIPID_4366, CHIP_4366_CHIPREV_C0, CHIP_4366_PKG_OPT4, CHIP_4366_C0_CHIP_IMAGE,
	CHIPIDSTR_4366, CHIPREVSTR_C0},
	/* 43664 use 4366 image */
	{CHIPID_43664, CHIP_4366_CHIPREV_C0, CHIP_4366_PKG_OPT, CHIP_4366_C0_CHIP_IMAGE,
	CHIPIDSTR_4366, CHIPREVSTR_C0},
	{CHIPID_43666, CHIP_4366_CHIPREV_C0, CHIP_4366_PKG_OPT, CHIP_4366_C0_CHIP_IMAGE,
	CHIPIDSTR_4366, CHIPREVSTR_C0},
	{CHIPID_43465, CHIP_4366_CHIPREV_C0, CHIP_4366_PKG_OPT, CHIP_4366_C0_CHIP_IMAGE,
	CHIPIDSTR_4366, CHIPREVSTR_C0},
	{CHIPID_43525, CHIP_4366_CHIPREV_C0, CHIP_4366_PKG_OPT, CHIP_4366_C0_CHIP_IMAGE,
	CHIPIDSTR_4366, CHIPREVSTR_C0},

/*
 * Chipset 43684b0
 */
	{CHIPID_43684, CHIP_43684_CHIPREV_B0, CHIP_43684_PKG_OPT, CHIP_43684_B0_CHIP_IMAGE,
	CHIPIDSTR_43684, CHIPREVSTR_B0},
	{CHIPID_43684, CHIP_43684_CHIPREV_B1, CHIP_43684_PKG_OPT, CHIP_43684_B0_CHIP_IMAGE,
	CHIPIDSTR_43684, CHIPREVSTR_B0},

/*
 * Chipset 43684c0
 */
	{CHIPID_43684, CHIP_43684_CHIPREV_C0, CHIP_43684_PKG_OPT, CHIP_43684_C0_CHIP_IMAGE,
	CHIPIDSTR_43684, CHIPREVSTR_C0},

#if defined(DHD_EAP)
/*
 * Chipset 43694b0
 */
	{CHIPID_43694, CHIP_43694_CHIPREV_B1, CHIP_43694_PKG_OPT, CHIP_43694_B0_CHIP_IMAGE,
	CHIPIDSTR_43694, CHIPREVSTR_B0},

/*
 * Chipset 43794b0
 */
	{CHIPID_43794, CHIP_43794_CHIPREV_B0, CHIP_43794_PKG_OPT, CHIP_43794_B0_CHIP_IMAGE,
	CHIPIDSTR_43794, CHIPREVSTR_B0},
#endif /* DHD_EAP */

/*
 * Chipset 4363c0
 */
	{CHIPID_4363, CHIP_4363_CHIPREV_C0, CHIP_4363_PKG_OPT, CHIP_4363_C0_CHIP_IMAGE,
	CHIPIDSTR_4363, CHIPREVSTR_C0},

/*
 * Chipset 6715b0
 */
	{CHIPID_6715, CHIP_6715_CHIPREV_B0, CHIP_6715_PKG_OPT, CHIP_6715_B0_CHIP_IMAGE,
	CHIPIDSTR_6715, CHIPREVSTR_B0},

	/* {for a given chipid, chiprev, chippkg, what is the index (the above enum) */
	{CHIPID_NONE, 0, 0, 0} /* CHIPID_NONE is -1, used to mark end of list */
};

#if defined(BCMEMBEDIMAGE)
struct fd_chip_image
{
	unsigned char *dlarray;
	int dlimagesize;
	char *dlimagename;
	char *dlimagever;
	char *dliamgedate;
} static chip_dl_image_array[] __initdata =
{
#ifdef DLIMAGE_43602a1
	{dlarray_43602a1, sizeof(dlarray_43602a1), dlimagename_43602a1,
	dlimagever_43602a1, dlimagedate_43602a1},
#endif
#ifdef DLIMAGE_43602a3
	{dlarray_43602a3, sizeof(dlarray_43602a3), dlimagename_43602a3,
	dlimagever_43602a3, dlimagedate_43602a3},
#endif
#ifdef DLIMAGE_4366c0
	{dlarray_4366c0, sizeof(dlarray_4366c0), dlimagename_4366c0,
	dlimagever_4366c0, dlimagedate_4366c0},
#endif
#ifdef DLIMAGE_43684b0
	{dlarray_43684b0, sizeof(dlarray_43684b0), dlimagename_43684b0,
	dlimagever_43684b0, dlimagedate_43684b0},
#endif
#ifdef DLIMAGE_43684c0
	{dlarray_43684c0, sizeof(dlarray_43684c0), dlimagename_43684c0,
	dlimagever_43684c0, dlimagedate_43684c0},
#endif
#ifdef DLIMAGE_43694b0
	{dlarray_43694b0, sizeof(dlarray_43694b0), dlimagename_43694b0,
	dlimagever_43694b0, dlimagedate_43694b0},
#endif
#ifdef DLIMAGE_43794b0
	{dlarray_43794b0, sizeof(dlarray_43794b0), dlimagename_43794b0,
	dlimagever_43794b0, dlimagedate_43794b0},
#endif
#ifdef DLIMAGE_4363c0
	{dlarray_4363c0, sizeof(dlarray_4363c0), dlimagename_4363c0,
	dlimagever_4363c0, dlimagedate_4363c0},
#endif
#ifdef DLIMAGE_6715b0
	{dlarray_6715b0, sizeof(dlarray_6715b0), dlimagename_6715b0,
	dlimagever_6715b0, dlimagedate_6715b0},
#endif
	/* {image attributes for other chips, only if image is compiled} */
};

static void
__init select_fd_image(struct dhd_bus *bus, unsigned char **p_dlarray,
		char **p_dlimagename, char **p_dlimagever,
		char **p_dlimagedate, int *image_size) {

	uint32 chipid, chiprev, chippkg_opt;
	int image_index;
	struct chip_image_map *p_image_index;

	chipid = 0;
	image_index = -1;
	p_image_index = &chip_image_index_map_table[0];
	while (chipid != CHIPID_NONE) {
		chipid = p_image_index->chipid;
		chiprev = p_image_index->chiprev;
		chippkg_opt = p_image_index->chippkg;

		if ((chipid == bus->sih->chip) && (chiprev == bus->sih->chiprev) &&
			(chippkg_opt == bus->sih->chippkg)) {
			image_index = p_image_index->image_idx;
			break;
		}
		p_image_index++;
	}

	if (image_index != -1) {
		*p_dlarray     = chip_dl_image_array[image_index].dlarray;
		*p_dlimagename = chip_dl_image_array[image_index].dlimagename;
		*p_dlimagever  = chip_dl_image_array[image_index].dlimagever;
		*p_dlimagedate = chip_dl_image_array[image_index].dliamgedate;
		*image_size    = chip_dl_image_array[image_index].dlimagesize;
	} else {
		*p_dlarray     = 0;
		DHD_ERROR(("\n\n\n\033[1m\033[34m"
			"##################################################################"
			"\n# %s: Dongle image not available for:"
			"\n#\t\tchipid = 0x%x  chiprev = %d  chippkg = %d\n"
			"##################################################################"
			"\033[0m\n",
			__FUNCTION__, bus->sih->chip, bus->sih->chiprev, bus->sih->chippkg));
	}
}
#endif /* BCMEMBEDIMAGE */

#if defined(EXTFDIMGPATH)

static int
select_chipidverstr(struct dhd_bus *bus, char *idverstr)
{
	int res = BCME_OK;
	uint32 chipid, chiprev, chippkg_opt;
	struct chip_image_map *p_img_idx;

	chipid = 0;
	p_img_idx = &chip_image_index_map_table[0];
	while (chipid != CHIPID_NONE) {
		chipid = p_img_idx->chipid;
		chiprev = p_img_idx->chiprev;
		chippkg_opt = p_img_idx->chippkg;

		if ((chipid == bus->sih->chip) && (chiprev == bus->sih->chiprev) &&
			(chippkg_opt == bus->sih->chippkg)) {
			break;
		}
		p_img_idx++;
	}

	if (chipid != CHIPID_NONE) {
		if (idverstr) {
			memcpy(idverstr + strlen(idverstr), p_img_idx->chipidname,
				strlen(p_img_idx->chipidname));
			memcpy(idverstr + strlen(idverstr), p_img_idx->chiprevname,
				strlen(p_img_idx->chiprevname));
		}
	} else {
		DHD_ERROR(("\n\n\n\033[1m\033[34m"
			"##################################################################"
			"\n# %s: DHD not supported for"
			"\n#\t\t chipid = 0x%x  chiprev = %d  chippkg = %d\n"
			"##################################################################"
			"\033[0m\n",
			__FUNCTION__, bus->sih->chip, bus->sih->chiprev, bus->sih->chippkg));
		res = BCME_UNSUPPORTED;
	}

	return res;
}

/*
 * If the fw_path is a directory, append the firmware binary file name
 * to the path based on dongle's chip id and version
 *
 */
static int
concate_extfdimg_name(dhd_bus_t *bus, char *fw_path)
{
	mm_segment_t old_fs;
	struct kstat stat;
	char chipidver[16] = "";

	/* Sanity Check */
	if (!bus || !bus->sih) {
		DHD_ERROR(("%s:Bus is Invalid\n", __FUNCTION__));
		return BCME_ERROR;
	}

	/* Check if the path is empty */
	if ((!fw_path) || (fw_path[0] == '\0')) {
		DHD_ERROR(("%s: fw path is empty\n", __FUNCTION__));
		return BCME_BADARG;
	}

	/* Check if the path already hs a .bin file */
	if (strstr(fw_path, ".bin") != NULL) {
		return BCME_OK;
	}

	/* Get the path information */
	old_fs = get_fs();
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 1, 0))
	set_fs(KERNEL_DS);
#else
	set_fs(get_ds());
#endif /* LINUX_VERSION_CODE >= 5.1.0 */
	if (vfs_stat(fw_path, &stat)) {
		/* non-existant or invalid path */
		DHD_ERROR(("%s: Failed to stat path %s \n", __FUNCTION__, fw_path));
		set_fs(old_fs);
		return BCME_BADARG;
	}
	set_fs(old_fs);

	/* Check if the path is a directory or not */
	if ((stat.mode & S_IFMT) != S_IFDIR) {
		/* path is not a directory, nothing to update */
		return BCME_OK;
	}

	/*
	 * given path is a directory, concatinate path with file name based on supported
	 * dongle connected on this PCIe slot.
	 *
	 * format of fw_path after concatination
	 *
	 * <dir>/<id><ver>/rtecdc.bin
	 */
	if (select_chipidverstr(bus, chipidver) == BCME_OK) {
		if (fw_path[strlen(fw_path)-1] != '/') {
			memcpy(fw_path + strlen(fw_path), "/", strlen("/") + 1);
		}
		memcpy(fw_path + strlen(fw_path), chipidver, strlen(chipidver) + 1);
		memcpy(fw_path + strlen(fw_path), "/rtecdc.bin", strlen("/rtecdc.bin")+1);
		DHD_ERROR(("DHD  %s: Final FW path is %s \n", __FUNCTION__, fw_path));
	}

	return BCME_OK;
}
#endif /* EXTFDIMGPATH */
#endif /* BCM_ROUTER_DHD && (BCMEMBEDIMAGE || EXTFDIMGPATH) */

#ifdef BCMEMBEDIMAGE
int
dhdpcie_download_code_array(struct dhd_bus *bus)
{
	int ret = BCME_OK;
	daddr32_t daddr32 = 0U;
	unsigned char *p_dlarray  = NULL;
	unsigned int dlarray_size = 0;
	unsigned int downloaded_len, remaining_len, len;
	char *p_dlimagename, *p_dlimagever, *p_dlimagedate;
	uint8 *memblock = NULL, *memptr;

	downloaded_len = 0;
	remaining_len = 0;
	len = 0;

#ifndef BCM_ROUTER_DHD
	p_dlarray = dlarray;
	dlarray_size = sizeof(dlarray);
	p_dlimagename = dlimagename;
	p_dlimagever  = dlimagever;
	p_dlimagedate = dlimagedate;
#else
	select_fd_image(bus, &p_dlarray, &p_dlimagename, &p_dlimagever,
		&p_dlimagedate, &dlarray_size);
#endif /* endif for BCM_ROUTER_DHD */

	if ((p_dlarray == 0) ||	(dlarray_size == 0) || (dlarray_size > bus->ramsize) ||
		(p_dlimagename == 0) ||	(p_dlimagever == 0) ||	(p_dlimagedate == 0))
	{
		DHD_ERROR(("%s: bad arguments\n", __FUNCTION__));
		ret = BCME_BADARG;
		goto err;
	}

	memptr = memblock = MALLOC(bus->dhd->osh, MEMBLOCK + DHD_SDALIGN);
	if (memblock == NULL) {
		DHD_ERROR(("%s: Failed to allocate memory %d bytes\n", __FUNCTION__, MEMBLOCK));
		ret = BCME_NOMEM;
		goto err;
	}
	if ((uint32)(uintptr)memblock % DHD_SDALIGN)
		memptr += (DHD_SDALIGN - ((uint32)(uintptr)memblock % DHD_SDALIGN));

	while (downloaded_len  < dlarray_size) {
		remaining_len = dlarray_size - downloaded_len;
		len = (remaining_len >= MEMBLOCK) ? MEMBLOCK : remaining_len;
		memcpy(memptr, (p_dlarray + downloaded_len), len);
		/* check if CR4/CA7 */
		if (si_setcore(bus->sih, ARMCR4_CORE_ID, 0) ||
			si_setcore(bus->sih, SYSMEM_CORE_ID, 0)) {
			/* if address is 0, store the reset instruction to be written in 0 */
			if (daddr32 == 0U) {
				bus->resetinstr = *(((uint32*)memptr));
				/* Add start of RAM address to the address given by user */
				daddr32 += bus->dongle_ram_base;
			}
		}
		dhdpcie_bus_membytes(bus, PCIE_SYSMEM_WRITE, daddr32,
		                    (uint8 *)memptr, len);
		downloaded_len += len;
		daddr32 += MEMBLOCK;
	}

#ifdef DHD_DEBUG
	/* Upload and compare the downloaded code */
	{
		unsigned char *ularray = NULL;
		unsigned int uploaded_len;
		uploaded_len = 0;
		ularray = MALLOC(bus->dhd->osh, dlarray_size);
		if (ularray == NULL) {
			DHD_ERROR(("%s: Upload malloc error\n", __FUNCTION__));
			ret = BCME_NOMEM;
			goto upload_err;
		}
		/* Upload image to verify downloaded contents. */
		daddr32 = bus->dongle_ram_base;
		memset(ularray, 0xaa, dlarray_size);
		while (uploaded_len  < dlarray_size) {
			remaining_len = dlarray_size - uploaded_len;
			len = (remaining_len >= MEMBLOCK) ? MEMBLOCK : remaining_len;
			dhdpcie_bus_membytes(bus, PCIE_SYSMEM_READ, daddr32,
			                     (uint8 *)(ularray + uploaded_len), len);
			uploaded_len += len;
			daddr32 += MEMBLOCK;
		}

		if (memcmp(p_dlarray, ularray, dlarray_size)) {
			DHD_ERROR(("%s: Downloaded image is corrupted (%s, %s, %s).\n",
				__FUNCTION__, p_dlimagename, p_dlimagever, p_dlimagedate));
			goto upload_err;
		} else {
			DHD_ERROR(("%s: Download, Upload and compare succeeded (%s, %s, %s).\n",
				__FUNCTION__, p_dlimagename, p_dlimagever, p_dlimagedate));
		}

upload_err:
		if (ularray)
			MFREE(bus->dhd->osh, ularray, dlarray_size);
	}
#endif /* DHD_DEBUG */
err:

	if (memblock)
		MFREE(bus->dhd->osh, memblock, MEMBLOCK + DHD_SDALIGN);

	return ret;
} /* dhdpcie_download_code_array */
#endif /* BCMEMBEDIMAGE */

#ifdef BCM_ROUTER_DHD
static int
_dhdpcie_get_nvram_params(struct dhd_bus *bus)
{
	int nvram_len = MAX_NVRAMBUF_SIZE;
	int tmp_nvram_len, boardrev_str_len;
	char *boardrev_str;
	char *boardtype_str;
	char *ptr;
	char *var;

	bus->nvram_params = MALLOC(bus->dhd->osh, nvram_len);
	if (!bus->nvram_params) {
		DHD_ERROR(("%s: fail to get nvram buffer to download.\n", __FUNCTION__));
		return -1;
	}

	bus->nvram_params[0] = 0;
	ptr = bus->nvram_params;
	/*
	 * For full dongle router platforms, we would have two dhd instances running,
	 * serving two radios, one for 5G and another for 2G. But, both dongle instances
	 * would come up as wl0, as one is not aware of the other. In order to avoid
	 * this situation, we pass the dhd instance number through nvram parameter
	 * wlunit=0 and wlunit=1 to the dongle and make sure the two dongle instances
	 * come up as wl0 and wl1.
	 */

	tmp_nvram_len = strlen("wlunit=xx\n\n") + 1;
	tmp_nvram_len =
		snprintf(ptr, tmp_nvram_len, "wlunit=%d", dhd_get_instance(bus->dhd));
	ptr += (tmp_nvram_len + 1); /* leave NULL */
	tmp_nvram_len++;

	if ((boardrev_str = si_getdevpathvar(bus->sih, "boardrev")) == NULL)
		boardrev_str = getvar(NULL, "boardrev");

	boardrev_str_len = strlen("boardrev=0xXXXX") + 1;
	boardrev_str_len = snprintf(ptr, boardrev_str_len, "boardrev=%s",
		boardrev_str? boardrev_str : BOARDREV_PROMOTABLE_STR);
	ptr += (boardrev_str_len + 1); /* leave NULL */
	tmp_nvram_len += (boardrev_str_len + 1);

	/* If per device boardtype is not available, use global boardtype */
	if ((boardtype_str = si_getdevpathvar(bus->sih, "boardtype")) == NULL) {
		if ((boardtype_str = getvar(NULL, "boardtype")) != NULL) {
			int boardtype_str_len = 0;

			boardtype_str_len = strlen("boardtype=0xXXXX") + 1;
			boardtype_str_len = snprintf(ptr, boardtype_str_len,
					"boardtype=%s", boardtype_str);
			ptr += (boardtype_str_len + 1); /* leave NULL */
			tmp_nvram_len += (boardtype_str_len + 1);
		}
	}

	/* If host is configured for pcie gen1 setting, let's explicitly
	 * set mutxmax=0. Setting this unconditionally for all radios,
	 * but this will affect only 4366.
	 */
	if ((var = getvar(NULL, "forcegen1rc")) != NULL) {
		int forcegen1 = bcm_strtoul(var, NULL, 0);
		if (forcegen1) {
			int mutx_str_len;
			mutx_str_len = sprintf(ptr, "mutxmax=0");
			ptr += (mutx_str_len + 1); /* leave NULL */
			tmp_nvram_len += (mutx_str_len + 1);
		}
	}

	if (dbushost_initvars_flash(bus->sih,
		bus->osh, &ptr,
		(nvram_len - tmp_nvram_len)) != 0) {
		DHD_ERROR(("%s: fail to read nvram from flash.\n", __FUNCTION__));
	}

	tmp_nvram_len = (int)(ptr - bus->nvram_params);

	bcopy(STR_END, ptr, sizeof(STR_END));
	tmp_nvram_len += sizeof(STR_END);
	bus->nvram_params_len  = tmp_nvram_len;

	return 0;
}

static void
_dhdpcie_free_nvram_params(struct dhd_bus *bus)
{
	if (bus->nvram_params) {
		MFREE(bus->dhd->osh, bus->nvram_params, MAX_NVRAMBUF_SIZE);
		bus->nvram_params = NULL;
	}
}
#endif /* BCM_ROUTER_DHD */

/**
 * Opens the file given by bus->fw_path, reads part of the file into a buffer and closes the file.
 *
 * @return BCME_OK on success
 */
static int
dhdpcie_ramsize_read_image(struct dhd_bus *bus, char *buf, int len)
{
	int bcmerror = BCME_ERROR;
	char *imgbuf = NULL;
	int fsize;
	uint32 magic_num;
	struct portion_hdr {
		uint32 type;
		uint32 len;
	} hdr;
	int ret;

	if (buf == NULL || len == 0)
		goto err;

	/* External image takes precedence if specified */
	if ((bus->fw_path != NULL) && (bus->fw_path[0] != '\0')) {
		/* opens and seeks to correct file offset: */
		imgbuf = dhd_os_open_image(bus->dhd, bus->fw_path);
		if (imgbuf == NULL) {
			DHD_ERROR(("%s: Failed to open firmware file\n", __FUNCTION__));
			goto err;
		}

		/* test for hybrid firmware */
		fsize = dhd_os_get_image_size(imgbuf);
		if (fsize < sizeof(magic_num)) {
			DHD_ERROR(("%s: image size too small: %u bytes\n", __FUNCTION__, fsize));
			goto err;
		}
		ret = dhd_os_get_image_block((char *)&magic_num, sizeof(magic_num), imgbuf);
		if (ret <= 0) {
			goto err;
		}
		magic_num = ltoh32(magic_num);
		if (magic_num == PCIE_IPC_HYBRIDFW_MAGICNUM) {
			ret = dhd_os_get_image_block((char *)&hdr, sizeof(hdr), imgbuf);
			if (ret <= 0) {
				goto err;
			}
			if (ltoh32(hdr.type) == PCIE_IPC_HYBRIDFW_TYPE_DNGL) {
				goto ramsize_continue;
			}
			DHD_ERROR(("%s: unexpected file type: %u\n",
				__FUNCTION__, ltoh32(hdr.type)));
		}
		/* Close and re-open the image file to reset the file pointer. */
		dhd_os_close_image(bus->dhd, imgbuf);
		imgbuf = dhd_os_open_image(bus->dhd, bus->fw_path);
		if (imgbuf == NULL) {
			DHD_ERROR(("%s: reopen %s failed\n", __FUNCTION__, bus->fw_path));
			goto err;
		}
ramsize_continue:

		/* Read it */
		if (len != dhd_os_get_image_block(buf, len, imgbuf)) {
			DHD_ERROR(("%s: Failed to read %d bytes data\n", __FUNCTION__, len));
			goto err;
		}

		bcmerror = BCME_OK;
	}

err:
	if (imgbuf)
		dhd_os_close_image(bus->dhd, imgbuf);

	return bcmerror;
}

/* The ramsize can be changed in the dongle image, for example 4365 chip share the sysmem
 * with BMC and we can adjust how many sysmem belong to CA7 during dongle compilation.
 * So in DHD we need to detect this case and update the correct dongle RAMSIZE as well.
 */
static void
dhdpcie_ramsize_adj(struct dhd_bus *bus)
{
	int i, search_len = 0;
	uint8 *memptr = NULL;
	uint8 *ramsizeptr = NULL;
	uint ramsizelen;
	uint32 ramsize_ptr_ptr[] = {RAMSIZE_PTR_PTR_LIST};
	hnd_ramsize_ptr_t ramsize_info;

	DHD_ERROR(("%s: Enter\n", __FUNCTION__));

	/* Not handle if user restrict dongle ram size enabled */
	if (dhd_dongle_memsize) {
		DHD_ERROR(("%s: user restrict dongle ram size to %d.\n", __FUNCTION__,
			dhd_dongle_memsize));
		return;
	}

#ifndef BCMEMBEDIMAGE
	/* Out immediately if no image to download */
	if ((bus->fw_path == NULL) || (bus->fw_path[0] == '\0')) {
		DHD_ERROR(("%s: no fimrware file\n", __FUNCTION__));
		return;
	}
#endif /* !BCMEMBEDIMAGE */

	/* Get maximum RAMSIZE info search length */
	for (i = 0; ; i++) {
		if (ramsize_ptr_ptr[i] == RAMSIZE_PTR_PTR_END)
			break;

		if (search_len < ramsize_ptr_ptr[i])
			search_len = ramsize_ptr_ptr[i];
	}

	if (!search_len)
		return;

	search_len += sizeof(hnd_ramsize_ptr_t);

	memptr = MALLOC(bus->dhd->osh, search_len);
	if (memptr == NULL) {
		DHD_ERROR(("%s: Failed to allocate memory %d bytes\n", __FUNCTION__, search_len));
		return;
	}

	/* External image takes precedence if specified */
	if (dhdpcie_ramsize_read_image(bus, memptr, search_len) != BCME_OK) {
#ifdef BCMEMBEDIMAGE
		unsigned char *p_dlarray  = NULL;
		unsigned int dlarray_size = 0;
		char *p_dlimagename, *p_dlimagever, *p_dlimagedate;

#ifndef BCM_ROUTER_DHD
		p_dlarray = dlarray;
		dlarray_size = sizeof(dlarray);
		p_dlimagename = dlimagename;
		p_dlimagever  = dlimagever;
		p_dlimagedate = dlimagedate;
#else
		select_fd_image(bus, &p_dlarray, &p_dlimagename, &p_dlimagever,
			&p_dlimagedate, &dlarray_size);
#endif /* endif for BCM_ROUTER_DHD */

		if ((p_dlarray == 0) ||	(dlarray_size == 0) || (p_dlimagename == 0) ||
			(p_dlimagever  == 0) ||	(p_dlimagedate == 0))
			goto err;

		ramsizeptr = p_dlarray;
		ramsizelen = dlarray_size;
#else
		goto err;
#endif /* BCMEMBEDIMAGE */
	}
	else {
		ramsizeptr = memptr;
		ramsizelen = search_len;
	}

	if (ramsizeptr) {
		/* Check Magic */
		for (i = 0; ; i++) {
			if (ramsize_ptr_ptr[i] == RAMSIZE_PTR_PTR_END)
				break;

			if (ramsize_ptr_ptr[i] + sizeof(hnd_ramsize_ptr_t) > ramsizelen)
				continue;

			memcpy((char *)&ramsize_info, ramsizeptr + ramsize_ptr_ptr[i],
				sizeof(hnd_ramsize_ptr_t));

			if (ramsize_info.magic == HTOL32(HND_RAMSIZE_PTR_MAGIC)) {
				bus->orig_ramsize = LTOH32(ramsize_info.ram_size);
				bus->ramsize = LTOH32(ramsize_info.ram_size);
				DHD_ERROR(("%s: Adjust dongle RAMSIZE to 0x%x\n", __FUNCTION__,
					bus->ramsize));
				break;
			}
		}
	}

err:
	if (memptr)
		MFREE(bus->dhd->osh, memptr, search_len);

	return;
} /* dhdpcie_ramsize_adj */

/**
 * Downloads firmware file given by 'bus->fw_path' into PCIe dongle
 *
 * BCMEMBEDIMAGE specific:
 * If bus->fw_path is empty, or if the download of bus->fw_path failed, firmware contained in header
 * file will be used instead.
 *
 */
static int
_dhdpcie_download_firmware(struct dhd_bus *bus)
{
	int bcmerror = -1;

	bool embed = FALSE;	/* download embedded firmware */
	bool dlok = FALSE;	/* download firmware succeeded */

	/* Out immediately if no image to download */
	if ((bus->fw_path == NULL) || (bus->fw_path[0] == '\0')) {
#ifdef BCMEMBEDIMAGE
		embed = TRUE;
#else
		DHD_ERROR(("%s: no fimrware file\n", __FUNCTION__));
		return 0;
#endif
	}
#ifdef BCM_ROUTER_DHD
	if (_dhdpcie_get_nvram_params(bus) < 0) {
		DHD_ERROR(("%s: fail to get nvram from system.\n", __FUNCTION__));
		return 0;
	}
#endif /* BCM_ROUTER_DHD */
	/* Adjust ram size */
	dhdpcie_ramsize_adj(bus);

	/* Turn off CA7 coherence to avoid system access error */
	if (BCM4365_CHIP(bus->sih->chip) ||
	    BCM43684_CHIP(bus->sih->chip) ||
	    BCM6715_CHIP(bus->sih->chip)) {
		hnd_hw_coherent_disable(bus);
	}

	/* Keep arm in reset */
	if (dhdpcie_bus_download_state(bus, TRUE)) {
		DHD_ERROR(("%s: error placing ARM core in reset\n", __FUNCTION__));
		goto err;
	}

	/* External image takes precedence if specified */
	if ((bus->fw_path != NULL) && (bus->fw_path[0] != '\0')) {
		if (dhdpcie_download_code_file(bus, bus->fw_path)) {
			DHD_ERROR(("%s:%d dongle image file download failed\n",
				__FUNCTION__, __LINE__));
#ifdef BCMEMBEDIMAGE
			embed = TRUE;
#else
			goto err;
#endif
		} else {
			embed = FALSE;
			dlok = TRUE;
		}
	}

#ifdef BCMEMBEDIMAGE
	if (embed) {
		if (dhdpcie_download_code_array(bus)) {
			DHD_ERROR(("%s: dongle image array download failed\n", __FUNCTION__));
			goto err;
		} else {
			dlok = TRUE;
		}
	}
#else
	BCM_REFERENCE(embed);
#endif
	if (!dlok) {
		DHD_ERROR(("%s:%d dongle image download failed\n", __FUNCTION__, __LINE__));
		goto err;
	}

	/* EXAMPLE: nvram_array */
	/* If a valid nvram_arry is specified as above, it can be passed down to dongle */
	/* dhd_bus_set_nvram_params(bus, (char *)&nvram_array); */

	/* External nvram takes precedence if specified */
	if (dhdpcie_download_nvram(bus)) {
		DHD_ERROR(("%s:%d dongle nvram file download failed\n", __FUNCTION__, __LINE__));
		goto err;
	}

#ifdef BCA_SROMMAP
	if (dhdpcie_download_map_bin(bus)) {
		DHD_ERROR((" %s:%d dongle nvram file download from internal failed.\n",
			__FUNCTION__, __LINE__));
		goto err;
	}
#endif

	/* Take arm out of reset */
	if (dhdpcie_bus_download_state(bus, FALSE)) {
		DHD_ERROR(("%s: error getting out of ARM core reset\n", __FUNCTION__));
		goto err;
	}

	bcmerror = 0;

err:
#ifdef BCM_ROUTER_DHD
	_dhdpcie_free_nvram_params(bus);
#endif /* BCM_ROUTER_DHD */
	return bcmerror;
} /* _dhdpcie_download_firmware */

#define CONSOLE_LINE_MAX	192

#ifdef CMWIFI_RDKB
#define CONSOLE_MSG		"BRCM-WIFI: CONSOLE[wl%d]: %s\n"
#else
#define CONSOLE_MSG		"CONSOLE[wl%d]: %s\n"
#endif /* CMWIFI_RDKB */

#ifdef DHD_DEBUG

static int
dhdpcie_bus_readconsole(dhd_bus_t *bus)
{
	dhd_console_t *c = &bus->console;
	uint8 line[CONSOLE_LINE_MAX], ch;
	uint32 n, idx;
	daddr32_t daddr32;
	uint readlen = 0;
	int i = 0;

	/* Don't do anything until FWREADY updates console address */
	if (bus->console_daddr32 == 0)
		return -1;

	/* Read console log struct */
	daddr32 = bus->console_daddr32 + OFFSETOF(hnd_cons_t, log);

	dhdpcie_bus_membytes(bus, PCIE_SYSMEM_READ, daddr32,
	                     (uint8 *)&c->log, sizeof(c->log));

	/* Allocate console buffer (one time only) */
	if (c->buf == NULL) {
		c->bufsize = ltoh32(c->log.buf_size);
		if ((c->buf = MALLOC(bus->dhd->osh, c->bufsize)) == NULL)
			return BCME_NOMEM;
		DHD_INFO(("conlog: bufsize=0x%x\n", c->bufsize));
	}
	idx = ltoh32(c->log.idx);

	/* Protect against corrupt value */
	if (idx > c->bufsize)
		return BCME_ERROR;

	/* Skip reading the console buffer if the index pointer has not moved */
	if (idx == c->last)
		return BCME_OK;

	DHD_INFO(("conlog: addr=0x%x, idx=0x%x, last=0x%x \n", c->log.buf, idx, c->last));

	/* Read the console buffer data to a local buffer */
	/* optimize and read only the portion of the buffer needed, but
	 * important to handle wrap-around.
	*/
	daddr32 = ltoh32(c->log.buf);

	/* wrap around case - write ptr < read ptr */
	if (idx < c->last) {
		/* from read ptr to end of buffer */
		readlen = c->bufsize - c->last;
		dhdpcie_bus_membytes(bus, PCIE_SYSMEM_READ,
			(ulong)daddr32 + (ulong)c->last, c->buf, readlen);
		/* from beginning of buffer to write ptr */
		dhdpcie_bus_membytes(bus, PCIE_SYSMEM_READ, (ulong)daddr32,
			(uint8*)((ulong)c->buf + (ulong)readlen), idx);
		readlen += idx;
	} else {
		/* non-wraparound case, write ptr > read ptr */
		readlen = (uint)idx - c->last;
		dhdpcie_bus_membytes(bus, PCIE_SYSMEM_READ,
			(ulong)daddr32 + (ulong)c->last, c->buf, readlen);
	}
	/* update read ptr */
	c->last = idx;

	/* now output the read data from the local buffer to the host console */
#if defined(LINUX) || defined(linux)
	preempt_disable();
#endif /* LINUX || linux */
	while (i < readlen) {
		for (n = 0; n < CONSOLE_LINE_MAX - 2 && i < readlen; n++) {
			ch = c->buf[i];
			++i;
			if (ch == '\n')
				break;
			line[n] = ch;
		}

		if (n > 0) {
			if (line[n - 1] == '\r')
				n--;
			line[n] = 0;
			if (dhd_msg_level & DHD_ERROR_VAL)
			printf(CONSOLE_MSG, bus->dhd->unit, line);
		}
	}
#if defined(LINUX) || defined(linux)
	preempt_enable();
#endif /* LINUX || linux */

	return BCME_OK;
} /* dhdpcie_bus_readconsole */
#endif /* DHD_DEBUG */

static int
dhdpcie_checkdied(dhd_bus_t *bus, char *data, uint size, uint32 *tr_type)
{
	int bcmerror = BCME_OK;
	uint msize = 512;
	char *mbuffer = NULL;
	char *console_buffer = NULL;
	uint maxstrlen = 256;
	char *str = NULL;
	trap_t tr;
	struct bcmstrbuf strbuf;
	uint32 console_ptr, console_size, console_index;
	uint8 line[CONSOLE_LINE_MAX], ch;
	uint32 n, i;
	daddr32_t daddr32;

	pcie_ipc_t assert_pcie_ipc; /* local copy on stack */
	pcie_ipc_t *pcie_ipc = &assert_pcie_ipc;

	DHD_TRACE(("%s: Enter\n", __FUNCTION__));

	if (DHD_NOCHECKDIED_ON()) {
		return 0;
	}

	if (data == NULL) {
		/*
		 * Called after a rx ctrl timeout. "data" is NULL.
		 * allocate memory to trace the trap or assert.
		 */
		size = msize;
		mbuffer = data = MALLOC(bus->dhd->osh, msize);
		if (mbuffer == NULL) {
			DHD_ERROR(("%s: MALLOC(%d) failed \n", __FUNCTION__, msize));
			bcmerror = BCME_NOMEM;
			goto done;
		}
	}

	if ((str = MALLOC(bus->dhd->osh, maxstrlen)) == NULL) {
		DHD_ERROR(("%s: MALLOC(%d) failed \n", __FUNCTION__, maxstrlen));
		bcmerror = BCME_NOMEM;
		goto done;
	}

	/*
	 * Read the pcie_ipc structure: fetch assert / trap info ...
	 * Do not need to read pcie_ipc_rings and pcie_ipc_hme
	 */
	bcmerror = dhdpcie_bus_read_pcie_ipc(bus, &daddr32, pcie_ipc, NULL, NULL);
	if (bcmerror < 0) {
		DHD_ERROR(("%s: dhdpcie_bus_read_pcie_ipc FAILURE %d\n",
			__FUNCTION__, bcmerror));
		goto done;
	}

	/* pcie_ipc is now in host order */

	bcm_binit(&strbuf, data, size);

	bcm_bprintf(&strbuf, "console address : 0x%08X\n", pcie_ipc->console_daddr32);

	if ((pcie_ipc->flags & PCIE_IPC_FLAGS_ASSERT_BUILT) == 0) {
		/* NOTE: Misspelled assert is intentional - DO NOT FIX.
		 * (Avoids conflict with real asserts for programmatic parsing of output.)
		 */
		bcm_bprintf(&strbuf, "Assrt not built in dongle\n");
	}

	if ((pcie_ipc->flags & (PCIE_IPC_FLAGS_ASSERT | PCIE_IPC_FLAGS_TRAP)) == 0) {
		/* NOTE: Misspelled assert is intentional - DO NOT FIX.
		 * (Avoids conflict with real asserts for programmatic parsing of output.)
		 */
		bcm_bprintf(&strbuf, "No trap%s in dongle",
		          (pcie_ipc->flags & PCIE_IPC_FLAGS_ASSERT_BUILT)
		          ? "/assrt" : "");
	} else {

		if (pcie_ipc->flags & PCIE_IPC_FLAGS_ASSERT) {
			/* Upload assert */
			bcm_bprintf(&strbuf, "Dongle assert");

			/* Upload the assert expression text */
			if (pcie_ipc->assert_exp_daddr32 != 0) {
				str[0] = '\0';
				dhdpcie_bus_membytes(bus, PCIE_SYSMEM_READ,
					pcie_ipc->assert_exp_daddr32, (uint8 *)str, maxstrlen);
				str[maxstrlen - 1] = '\0';
				bcm_bprintf(&strbuf, " expr \"%s\"", str);
			}

			/* Upload source filename text */
			if (pcie_ipc->assert_file_daddr32 != 0) {
				str[0] = '\0';
				dhdpcie_bus_membytes(bus, PCIE_SYSMEM_READ,
					pcie_ipc->assert_file_daddr32, (uint8 *)str, maxstrlen);
				str[maxstrlen - 1] = '\0';
				bcm_bprintf(&strbuf, " file \"%s\"", str);
			}

			bcm_bprintf(&strbuf, " line %d ", pcie_ipc->assert_line);
		}

		/* Check if a trap occurred in dongle */
		if (pcie_ipc->flags & PCIE_IPC_FLAGS_TRAP) {
			bus->dhd->dongle_trap_occured = TRUE;

			if (pcie_ipc->trap_daddr32 == 0) {
				DHD_ERROR(("%s: invalid trap daddr32 = 0\n", __FUNCTION__));
				bcmerror = BCME_NOTFOUND;
				goto done;
			}

			dhdpcie_bus_membytes(bus, PCIE_SYSMEM_READ,
				pcie_ipc->trap_daddr32, (uint8*)&tr, sizeof(trap_t));

			bcm_bprintf(&strbuf,
			"\nTRAP type 0x%x @ epc 0x%x, cpsr 0x%x, spsr 0x%x, sp 0x%x,"
			" lp 0x%x, rpc 0x%x"
			"\nTrap offset 0x%x, r0 0x%x, r1 0x%x, r2 0x%x, r3 0x%x, "
			"r4 0x%x, r5 0x%x, r6 0x%x, r7 0x%x\n\n",
			ltoh32(tr.type), ltoh32(tr.epc), ltoh32(tr.cpsr), ltoh32(tr.spsr),
			ltoh32(tr.r13), ltoh32(tr.r14), ltoh32(tr.pc),
			ltoh32(pcie_ipc->trap_daddr32),
			ltoh32(tr.r0), ltoh32(tr.r1), ltoh32(tr.r2), ltoh32(tr.r3),
			ltoh32(tr.r4), ltoh32(tr.r5), ltoh32(tr.r6), ltoh32(tr.r7));

			if (tr_type)
				*tr_type = ltoh32(tr.type);
		} /* trap */

		/* Try to upload console log from dongle, on either assert or trap */

		/* Upload the location of hnd_log_t in dongle's memory */
		daddr32 = pcie_ipc->console_daddr32 + OFFSETOF(hnd_cons_t, log);
		dhdpcie_bus_membytes(bus, PCIE_SYSMEM_READ, daddr32,
			(uint8 *)&console_ptr, sizeof(console_ptr));

		/* Upload the console log buffer size */
		daddr32 = pcie_ipc->console_daddr32 + OFFSETOF(hnd_cons_t, log.buf_size);
		dhdpcie_bus_membytes(bus, PCIE_SYSMEM_READ, daddr32,
			(uint8 *)&console_size, sizeof(console_size));

		/* Upload the current console log index */
		daddr32 = pcie_ipc->console_daddr32 + OFFSETOF(hnd_cons_t, log.idx);
		dhdpcie_bus_membytes(bus, PCIE_SYSMEM_READ, daddr32,
			(uint8 *)&console_index, sizeof(console_index));

		console_ptr = ltoh32(console_ptr);
		console_size = ltoh32(console_size);
		console_index = ltoh32(console_index);

		/* Allocate local memory to store the console log */
		if (console_size > CONSOLE_BUFFER_MAX ||
		        !(console_buffer = MALLOC(bus->dhd->osh, console_size))) {
			goto printbuf;
		}

		/* Upload the entire console log */
		dhdpcie_bus_membytes(bus, PCIE_SYSMEM_READ, console_ptr,
			(uint8 *)console_buffer, console_size);

		for (i = 0, n = 0; i < console_size; i += n + 1) {
			for (n = 0; n < CONSOLE_LINE_MAX - 2; n++) {
				ch = console_buffer[(console_index + i + n) % console_size];
				if (ch == '\n')
					break;
				line[n] = ch;
			}

			if (n > 0) {
				if (line[n - 1] == '\r')
					n--;
				line[n] = 0;
				/* Don't use DHD_ERROR macro since we print
				 * a lot of information quickly. The macro
				 * will truncate a lot of the printfs
				 */

				if (dhd_msg_level & DHD_ERROR_VAL)
				printf(CONSOLE_MSG, bus->dhd->unit, line);
			}
		}
	}

printbuf:

	if (pcie_ipc->flags & (PCIE_IPC_FLAGS_ASSERT | PCIE_IPC_FLAGS_TRAP)) {
		printf("%s: %s\n", __FUNCTION__, strbuf.origbuf);

#if defined(DHD_FW_COREDUMP)
		/* save core dump or write to a file */
		if (bus->dhd->memdump_enabled) {
			dhdpcie_mem_dump(bus);
		}
#endif /* DHD_FW_COREDUMP */
	}

done:
	if (mbuffer)
		MFREE(bus->dhd->osh, mbuffer, msize);
	if (str)
		MFREE(bus->dhd->osh, str, maxstrlen);

	if (console_buffer)
		MFREE(bus->dhd->osh, console_buffer, console_size);

	return bcmerror;
} /* dhdpcie_checkdied */

/* Custom copy of dhdpcie_mem_dump() that can be called at interrupt level */
void
dhdpcie_mem_dump_bugcheck(dhd_bus_t *bus, uint8 *buf)
{
	int size; /* Full mem size */
	int start; /* Start address */
	int read_size = 0; /* Read size of each iteration */
	uint8 *databuf = buf;

	if (bus == NULL) {
		return;
	}

	start = bus->dongle_ram_base;
	/* Get full mem size */
	size = bus->ramsize;
	/* Read mem content */
	while (size)
	{
		read_size = MIN(MEMBLOCK, size);
		dhdpcie_bus_membytes(bus, PCIE_SYSMEM_READ, start, databuf, read_size);

		/* Decrement size and increment start address */
		size -= read_size;
		start += read_size;
		databuf += read_size;
	}
	bus->dhd->soc_ram = buf;
	bus->dhd->soc_ram_length = bus->ramsize;
}

#if defined(DHD_FW_COREDUMP)
/** Has several callers within this file */
static int
dhdpcie_mem_dump(dhd_bus_t *bus)
{
	int size; /* Full mem size */
	int start = bus->dongle_ram_base; /* Start address */
	int read_size = 0; /* Read size of each iteration */
	uint8 *buf = NULL, *databuf = NULL;

	/* Get full mem size */
	size = bus->ramsize;
#if defined(CONFIG_DHD_USE_STATIC_BUF) && defined(DHD_USE_STATIC_MEMDUMP)
	buf = DHD_OS_PREALLOC(bus->dhd, DHD_PREALLOC_MEMDUMP_BUF, size);
	bzero(buf, size);
#else
	buf = MALLOC(bus->dhd->osh, size);
#endif /* CONFIG_DHD_USE_STATIC_BUF && DHD_USE_STATIC_MEMDUMP */
	if (buf == NULL) {
		DHD_ERROR(("%s: Out of memory (%d bytes), retrying dump.\n", __FUNCTION__, size));
		/* Retry in a lower execution context by writing directly to file, without having
		 * to allocate a large buffer.
		 */
		dhd_schedule_memdump(bus->dhd, NULL, 0);
		return BCME_OK;
	}

	/* Read mem content */
	DHD_TRACE_HW4(("Dump dongle memory"));
	databuf = buf;
	while (size)
	{
		read_size = MIN(MEMBLOCK, size);
		dhdpcie_bus_membytes(bus, PCIE_SYSMEM_READ, start, databuf, read_size);
		DHD_TRACE(("."));

		/* Decrement size and increment start address */
		size -= read_size;
		start += read_size;
		databuf += read_size;
	}

	DHD_TRACE_HW4(("%s FUNC: Copy fw image to the embedded buffer \n", __FUNCTION__));

	dhd_save_fwdump(bus->dhd, buf, bus->ramsize);
	dhd_schedule_memdump(bus->dhd, buf, bus->ramsize);

	return BCME_OK;
}

/** called by higher dhd layer, in irq context */
int
dhd_bus_mem_dump(dhd_pub_t *dhdp)
{
	dhd_bus_t *bus = dhdp->bus;

	return dhdpcie_mem_dump(bus);
}

#endif /* DHD_FW_COREDUMP */

int
dhd_socram_dump(dhd_bus_t *bus)
{
#if defined(DHD_FW_COREDUMP)
	return (dhdpcie_mem_dump(bus));
#else
	return -1;
#endif
}

/**
 * Transfers bytes from host to dongle using pio mode.
 * Parameter 'daddr32' is a backplane address.
 * Access over PCIe cannot fail (no return value).
 */
static void
dhdpcie_bus_membytes(dhd_bus_t *bus, uint32 pcie_sysmem_access,
	daddr32_t daddr32, uint8 *data, uint size)
{
	uint dsize;
	int detect_endian_flag = 0x01;
	bool little_endian;

	/* Detect endianness. */
	little_endian = *(char *)&detect_endian_flag;

	/* In remap mode, adjust daddr32 beyond socram and redirect
	 * to devram at SOCDEVRAM_BP_ADDR since remap daddr32 > orig_ramsize
	 * is not backplane accessible
	 */

	/* Determine initial transfer parameters */
#ifdef BCMQT
	dsize = sizeof(uint32);
#else
	dsize = sizeof(uint64);
#endif /* BCMQT */

	/* Do the transfer(s) */
	if (pcie_sysmem_access == PCIE_SYSMEM_WRITE) {
		while (size) {
#ifdef BCMQT
			if (size >= sizeof(uint32)) {
				uint32 u32 = (uint32) HTOL32(*(uint32*)data);
				dhdpcie_bus_write_u32(bus, daddr32, u32);
#else
			if (size >= sizeof(uint64) && little_endian &&
#ifdef CONFIG_64BIT
				!(daddr32 % 8) &&
#endif /* CONFIG_64BIT */
				1) {
				dhdpcie_bus_write_u64(bus, daddr32, *((uint64 *)data));
#endif /* BCMQT */
			} else {
				dsize = sizeof(uint8);
				dhdpcie_bus_write_u8(bus, daddr32, *data);
			}

			/* Adjust for next transfer (if any) */
			if ((size -= dsize)) {
				data += dsize;
				daddr32 += dsize;
			}
		}
	} else { // pcie_sysmem_access == PCIE_SYSMEM_READ
		while (size) {
#ifdef BCMQT
			if (size >= sizeof(uint32)) {
				*(uint32*)data = LTOH32(dhdpcie_bus_read_u32(bus, daddr32));
#else
			if (size >= sizeof(uint64) && little_endian &&
#ifdef CONFIG_64BIT
				!(daddr32 % 8) &&
#endif /* CONFIG_64BIT */
				1) {
				*(uint64 *)data = dhdpcie_bus_read_u64(bus, daddr32);
#endif /* BCMQT */
			} else {
				dsize = sizeof(uint8);
				*data = dhdpcie_bus_read_u8(bus, daddr32);
			}

			/* Adjust for next transfer (if any) */
			if ((size -= dsize) > 0) {
				data += dsize;
				daddr32 += dsize;
			}
		}
	}

} /* dhdpcie_bus_membytes */

/**
 * Transfers one transmit (ethernet) packet that was queued in the (flow controlled) flow ring queue
 * to the (non flow controlled) flow ring.
 */
int BCMFASTPATH
dhd_bus_schedule_queue(struct dhd_bus  *bus, uint16 flow_id, bool txs)
{
	flow_ring_node_t *flow_ring_node;
	int ret = BCME_OK;

	DHD_INFO(("%s: flow_id is %d\n", __FUNCTION__, flow_id));

	/* ASSERT on flow_id */
	if (flow_id >= bus->max_h2d_rings) {
		DHD_ERROR(("%s: flow_id is invalid %d, max %d\n", __FUNCTION__,
			flow_id, bus->max_h2d_rings));
		return 0;
	}

	flow_ring_node = DHD_FLOW_RING(bus->dhd, flow_id);

#if defined(BCM_DHD_RUNNER)
	if (DHD_FLOWRING_RNR_OFFL(flow_ring_node))
	{
		unsigned long flags;
		void *txp = NULL;
#ifdef BCM_NBUFF_WLMCAST
		uint8 b_fkb_wmf_ucast;
		uint8 b_fkb_clone;
#endif
		flow_queue_t *queue;

		queue = &flow_ring_node->queue; /* queue associated with flow ring */

		DHD_FLOWRING_LOCK(flow_ring_node->lock, flags);

		if (flow_ring_node->status != FLOW_RING_STATUS_OPEN) {
			DHD_FLOWRING_UNLOCK(flow_ring_node->lock, flags);
			return BCME_NOTREADY;
		}

		while ((txp = dhd_flow_queue_dequeue(bus->dhd, queue)) != NULL) {
			PKTORPHAN(txp);

			dhd_shortpktpad(bus->dhd, txp);

#ifdef BCM_NBUFF_WLMCAST
			b_fkb_wmf_ucast = IS_FKB_WMF_UCAST(txp);
			b_fkb_clone = IS_FKBUFF_PTR(txp) &
				_is_fkb_cloned_pool_(PNBUFF_2_FKBUFF(txp));
			if (b_fkb_wmf_ucast && (FKB_IS_FKBPOOL(txp))) {
					char *t_pktdata_p = PKTDATA(bus->dhd->osh, txp);
					if (DHDHDR_SUPPORT(bus->dhd) && FKB_HAS_DHDHDR(txp)) {
						/* restore it back to original */
						memcpy(t_pktdata_p + ETHER_ADDR_LEN,
						(t_pktdata_p + ETHER_ADDR_LEN -
						DOT11_LLC_SNAP_HDR_LEN),
						ETHER_ADDR_LEN);

						memcpy(t_pktdata_p,
						t_pktdata_p-DOT11_LLC_SNAP_HDR_LEN, ETHER_ADDR_LEN);
						DHD_PKT_CLR_DATA_DHDHDR(txp);
					}
					memcpy(t_pktdata_p, DHD_PKT_GET_MAC(txp), ETHER_ADDR_LEN);

			} else if ((b_fkb_clone && DHDHDR_SUPPORT(bus->dhd)) || b_fkb_wmf_ucast) {
				void *clone_txp = DHD_FKB_CLONE2UNICAST(bus->dhd->osh, txp,
					b_fkb_wmf_ucast ? DHD_PKT_GET_MAC(txp):NULL);
					if (clone_txp) {
						if (IS_FKBUFF_PTR(clone_txp))
							PKTFREE(bus->dhd->osh, txp, FALSE);
						txp = clone_txp;
					} else {
						PKTFREE(bus->dhd->osh, txp, FALSE);
						continue;
				}
			}

#endif /* BCM_NBUFF_WLMCAST */
			if (dhd_runner_notify(bus->dhd->runner_hlp, H2R_TXPOST_NOTIF,
				(uintptr) txp, flow_ring_node->flow_info.ifindex)) {

				DHD_INFO(("%s: Reinsert %d\n", __FUNCTION__, ret));
				/* reinsert at head */
				dhd_flow_queue_reinsert(bus->dhd, queue, txp);
				return BCME_OK;
			}
		}

		DHD_FLOWRING_UNLOCK(flow_ring_node->lock, flags);

		return ret;
	}
#endif /* BCM_DHD_RUNNER */
#if defined(BCM_ROUTER_DHD) && defined(BCM_GMAC3)
	if (DHD_FLOW_QUEUE_LEN(&flow_ring_node->queue))
		ret = dhd_prot_txqueue_flush(bus->dhd, flow_ring_node);
#else
	{
		unsigned long flags;
		void *txp = NULL;
		flow_queue_t *queue;

		queue = &flow_ring_node->queue; /* queue associated with flow ring */

		DHD_FLOWRING_LOCK(flow_ring_node->lock, flags);

		if (flow_ring_node->status != FLOW_RING_STATUS_OPEN) {
			DHD_FLOWRING_UNLOCK(flow_ring_node->lock, flags);
			return BCME_NOTREADY;
		}

		while ((txp = dhd_flow_queue_dequeue(bus->dhd, queue)) != NULL) {
			PKTORPHAN(txp);

			/*
			 * Modifying the packet length caused P2P cert failures.
			 * Specifically on test cases where a packet of size 52 bytes
			 * was injected, the sniffer capture showed 62 bytes because of
			 * which the cert tests failed. So making the below change
			 * only Router specific.
			 */
#if defined(BCM_ROUTER_DHD)
			dhd_shortpktpad(bus->dhd, txp);
#endif /* BCM_ROUTER_DHD */

#ifdef DHDTCPACK_SUPPRESS
			if (bus->dhd->tcpack_sup_mode != TCPACK_SUP_HOLD) {
				ret = dhd_tcpack_check_xmit(bus->dhd, txp);
				if (ret != BCME_OK) {
					DHD_ERROR(("%s: dhd_tcpack_check_xmit() error.\n",
						__FUNCTION__));
				}
			}
#endif /* DHDTCPACK_SUPPRESS */

			/* Attempt to transfer packet over flow ring */
			ret = dhd_prot_txdata(bus->dhd, txp, flow_ring_node->flow_info.ifindex);
			if (ret != BCME_OK) { /* may not have resources in flow ring */
				DHD_INFO(("%s: Reinserrt %d\n", __FUNCTION__, ret));
				/* reinsert at head */
				dhd_flow_queue_reinsert(bus->dhd, queue, txp);
				dhd_prot_txdata_write_flush(bus->dhd, flow_id, FALSE);
				DHD_FLOWRING_UNLOCK(flow_ring_node->lock, flags);

				/* If we are able to requeue back, return success */
				return BCME_OK;
			}
		}

		dhd_prot_txdata_write_flush(bus->dhd, flow_id, FALSE);

		DHD_FLOWRING_UNLOCK(flow_ring_node->lock, flags);
	}
#endif /* ! (BCM_ROUTER_DHD && BCM_GMAC3) */

	return ret;
} /* dhd_bus_schedule_queue */

/** Sends an (ethernet) data frame (in 'txp') to the dongle. Callee disposes of txp. */
int BCMFASTPATH
dhd_bus_txdata(struct dhd_bus *bus, void *txp, uint8 ifidx)
{
	uint16 flowid;
	flow_queue_t *queue;
	flow_ring_node_t *flow_ring_node;
	unsigned long flags;
	int ret = BCME_OK;
	void *txp_pend = NULL;
#if (defined(BCM_ROUTER_DHD) && defined(PKTC))
	void *ntxp = NULL;
	uint8 prio = PKTPRIO(txp);
#endif

	if (!bus->dhd->flowid_allocator) {
		DHD_ERROR(("%s: Flow ring not intited yet  \n", __FUNCTION__));
		goto toss;
	}

	flowid = DHD_PKT_GET_FLOWID(txp);

	flow_ring_node = DHD_FLOW_RING(bus->dhd, flowid);

	DHD_TRACE(("%s: pkt flowid %d, status %d active %d\n",
		__FUNCTION__, flowid, flow_ring_node->status,
		flow_ring_node->active));

	DHD_FLOWRING_LOCK(flow_ring_node->lock, flags);
	if ((flowid >= bus->dhd->num_flow_rings) ||
		(!flow_ring_node->active) ||
		((flow_ring_node->status == FLOW_RING_STATUS_DELETE_PENDING) &&
#ifdef DHD_IFE
		/* Continue queuing in case delete initiated by IFE */
		!flow_ring_node->evict_inprogress &&
#endif /* DHD_IFE */
		TRUE) || (flow_ring_node->status == FLOW_RING_STATUS_STA_FREEING)) {
		DHD_FLOWRING_UNLOCK(flow_ring_node->lock, flags);
		DHD_INFO(("%s: Dropping pkt flowid %d, status %d active %d\n",
			__FUNCTION__, flowid, flow_ring_node->status,
			flow_ring_node->active));
		ret = BCME_ERROR;
		goto toss;
	}

	queue = &flow_ring_node->queue; /* queue associated with flow ring */

#if (defined(BCM_ROUTER_DHD) && defined(PKTC))

	FOREACH_CHAINED_PKT(txp, ntxp) {
		/* Tag the packet with flowid - Remember, only the head packet */
		/* of the chain has been tagged with the FlowID in dhd_sendpkt */
		/* Also set the priority */
		DHD_PKT_SET_FLOWID(txp, flowid);
		PKTSETPRIO(txp, prio);

		if ((ret = dhd_flow_queue_enqueue(bus->dhd, queue, txp)) != BCME_OK) {
			txp_pend = txp;
			PKTSETCLINK((txp), ntxp);
			break;
		}
	}
#else  /* !((BCM_ROUTER_DHD && PKTC) || (CMWIFI && PKTC)) */
	if ((ret = dhd_flow_queue_enqueue(bus->dhd, queue, txp)) != BCME_OK) {
		txp_pend = txp;
	}
#endif

	DHD_FLOWRING_UNLOCK(flow_ring_node->lock, flags);

	if (flow_ring_node->status) {
		DHD_INFO(("%s: Enq pkt flowid %d, status %d active %d\n",
			__FUNCTION__, flowid, flow_ring_node->status,
			flow_ring_node->active));
		if (txp_pend) {
			txp = txp_pend;
			goto toss;
		}
		return BCME_OK;
	}
	ret = dhd_bus_schedule_queue(bus, flowid, FALSE); /* from queue to flowring */

	/* If we have anything pending, try to push into q */
	if (txp_pend) {
		DHD_FLOWRING_LOCK(flow_ring_node->lock, flags);

#if (defined(BCM_ROUTER_DHD) && defined(PKTC))
		FOREACH_CHAINED_PKT(txp_pend, ntxp) {
			/* Tag the packet with flowid and set packet priority */
			DHD_PKT_SET_FLOWID(txp_pend, flowid);
			PKTSETPRIO(txp_pend, prio);

			if ((ret = dhd_flow_queue_enqueue(bus->dhd, queue, txp_pend))
				!= BCME_OK) {
				DHD_FLOWRING_UNLOCK(flow_ring_node->lock, flags);
				PKTSETCLINK((txp_pend), ntxp);
				txp = txp_pend;
				goto toss;
			}
		}
#else  /* !(BCM_ROUTER_DHD && PKTC) */
		if ((ret = dhd_flow_queue_enqueue(bus->dhd, queue, txp_pend)) != BCME_OK) {
			DHD_FLOWRING_UNLOCK(flow_ring_node->lock, flags);
			txp = txp_pend;
			goto toss;
		}
#endif /* BCM_ROUTER_DHD && PKTC */

		DHD_FLOWRING_UNLOCK(flow_ring_node->lock, flags);
	}

	return ret;

toss:
	DHD_INFO(("%s: Toss %d\n", __FUNCTION__, ret));
	PKTCFREE(bus->dhd->osh, txp, TRUE);
	return ret;
} /* dhd_bus_txdata */

#if defined(BCM_ROUTER_DHD) && defined(BCM_GMAC3)
/**
 * Bulk packet enqueue to flowring queue and single flush optimization.
 * Packets dispatched to the pcie bus, are placed into the flowring's queue,
 * and the flowring queue is placed into the pcie bus's txqueue_pend list.
 * When the OS layer invokes txqueue_flush, all flowring queues, that are in the
 * txqueue_pend list are flushed by moving the packets from the queue to the
 * flowring (if space is available) and the flowring queue is moved to the
 * txqueue_done list.
 *
 * Perimeter lock must be taken, protecting the flow_queue and the txqueue_xxx.
 */
int BCMFASTPATH
dhd_bus_txqueue_enqueue(struct dhd_bus *bus, void *pkt, uint16 flowid)
{
	int ret;
	flow_ring_node_t *flow_ring_node;
	flow_queue_t *queue;
	dhd_pub_t *dhdp = bus->dhd;

	ASSERT(flowid < dhdp->num_flow_rings);

	flow_ring_node = DHD_FLOW_RING(dhdp, flowid);

	DHD_TRACE(("%s: pkt flowid %d, status %d active %d\n",
		__FUNCTION__, flowid, flow_ring_node->status,
		flow_ring_node->active));

	queue = &flow_ring_node->queue; /* queue associated with flow ring */

	ret = dhd_flow_queue_enqueue(dhdp, queue, pkt);

	/* move the flowring queue (from txqueue_xxx) to txqueue_pend, ignore ret */
	dll_delete(&queue->list);
	dll_prepend(&bus->txqueue_pend, &queue->list);

	return ret;
}

void BCMFASTPATH
dhd_bus_txqueue_flush(struct dhd_bus *bus)
{
	dll_t *item, *next;
	flow_queue_t *txqueue;
	flow_ring_node_t *flow_ring_node;
	dhd_pub_t *dhdp = bus->dhd;

	/* Attempt to flush each flowring queue in bus' txqueue_pend list */
	for (item = dll_head_p(&bus->txqueue_pend);
	         !dll_end(&bus->txqueue_pend, item); item = next) {
		next = dll_next_p(item);

		txqueue = container_of(item, flow_queue_t, list);
		flow_ring_node = container_of(txqueue, flow_ring_node_t, queue);

		if ((!flow_ring_node->active) ||
		    (flow_ring_node->status == FLOW_RING_STATUS_DELETE_PENDING)) {
			DHD_INFO(("%s: Flowring %d, status %d active %d\n", __FUNCTION__,
			          flow_ring_node->flowid, flow_ring_node->status,
			          flow_ring_node->active));
		} else {
			/* schedule the flushing of all packets from flow_queue */
			dhd_prot_txqueue_flush(dhdp, flow_ring_node);

			/* move the flowring queue (from txqueue_pend) to txqueue_done */
			dll_delete(&txqueue->list);
			dll_prepend(&bus->txqueue_done, &txqueue->list);
		}
	}
}
#endif /* BCM_ROUTER_DHD && BCM_GMAC3 */

void
dhd_bus_stop_queue(struct dhd_bus *bus)
{
	dhd_txflowcontrol(bus->dhd, ALL_INTERFACES, ON);
	bus->bus_flowctrl = TRUE;
}

void
dhd_bus_start_queue(struct dhd_bus *bus)
{
	dhd_txflowcontrol(bus->dhd, ALL_INTERFACES, OFF);
	bus->bus_flowctrl = TRUE;
}

#if defined(DHD_DEBUG)
/* Device console input function */
int
dhd_bus_console_in(dhd_pub_t *dhd, uchar *msg, uint msglen)
{
	dhd_bus_t *bus = dhd->bus;
	daddr32_t daddr32;
	uint32 val;
	/* Address could be zero if CONSOLE := 0 in dongle Makefile */
	if (bus->console_daddr32 == 0)
		return BCME_UNSUPPORTED;

	/* Don't allow input if dongle is in reset */
	if (bus->dhd->dongle_reset) {
		dhd_os_sdunlock(bus->dhd);
		return BCME_NOTREADY;
	}

	/* Zero cbuf_index */
	daddr32 = bus->console_daddr32 + OFFSETOF(hnd_cons_t, cbuf_idx);
	val = htol32(0);
	dhdpcie_bus_membytes(bus, PCIE_SYSMEM_WRITE, daddr32,
		(uint8 *)&val, sizeof(val));

	/* Write message into cbuf */
	daddr32 = bus->console_daddr32 + OFFSETOF(hnd_cons_t, cbuf);
	dhdpcie_bus_membytes(bus, PCIE_SYSMEM_WRITE, daddr32,
		(uint8 *)msg, msglen);

	/* Write length into vcons_in */
	daddr32 = bus->console_daddr32 + OFFSETOF(hnd_cons_t, vcons_in);
	val = htol32(msglen);
	dhdpcie_bus_membytes(bus, PCIE_SYSMEM_WRITE, daddr32,
		(uint8 *)&val, sizeof(val));

	/* generate an interrupt to dongle to indicate that it needs to process cons command */
	dhdpcie_send_mb_data(bus, PCIE_IPC_H2DMB_HOST_CONS_INT);

	return BCME_OK;
} /* dhd_bus_console_in */
#endif /* defined(DHD_DEBUG) */

/**
 * Called on frame reception, the frame was received from the dongle on interface 'ifidx' and is
 * contained in 'pkt'. Processes rx frame, forwards up the layer to netif.
 */
void BCMFASTPATH
dhd_bus_rx_frame(struct dhd_bus *bus, void* pkt, int ifidx, uint pkt_count)
{
	dhd_rx_frame(bus->dhd, ifidx, pkt, pkt_count, 0);
}

/** 'daddr32' is a backplane address */
void
dhdpcie_bus_write_u8(dhd_bus_t *bus, uint32 daddr32, uint8 data)
{
	*(volatile uint8 *)(bus->tcm + daddr32) = (uint8)data;
}

uint8
dhdpcie_bus_read_u8(dhd_bus_t *bus, uint32 daddr32)
{
	volatile uint8 data;

#ifdef BCM_ROUTER_DHD
	if (OSL_ACP_WAR_ENAB() || OSL_ARCH_IS_COHERENT())
		data = R_REG(bus->dhd->osh, (volatile uint8 *)(bus->tcm + daddr32));
	else
#endif /* BCM_ROUTER_DHD */
		data = *(volatile uint8 *)(bus->tcm + daddr32);

	return data;
}

void
dhdpcie_bus_write_u32(dhd_bus_t *bus, uint32 daddr32, uint32 data)
{
	*(volatile uint32 *)(bus->tcm + daddr32) = (uint32)data;
}

void
dhdpcie_bus_write_u16(dhd_bus_t *bus, uint32 daddr32, uint16 data)
{
	*(volatile uint16 *)(bus->tcm + daddr32) = (uint16)data;
}

#ifndef BCMQT
void
dhdpcie_bus_write_u64(dhd_bus_t *bus, uint32 daddr32, uint64 data)
{
	*(volatile uint64 *)(bus->tcm + daddr32) = (uint64)data;
}
#endif /* !BCMQT */

uint16
dhdpcie_bus_read_u16(dhd_bus_t *bus, uint32 daddr32)
{
	volatile uint16 data;

#ifdef BCM_ROUTER_DHD
	if (OSL_ACP_WAR_ENAB() || OSL_ARCH_IS_COHERENT())
		data = R_REG(bus->dhd->osh, (volatile uint16 *)(bus->tcm + daddr32));
	else
#endif /* BCM_ROUTER_DHD */
		data = *(volatile uint16 *)(bus->tcm + daddr32);

	return data;
}

uint32
dhdpcie_bus_read_u32(dhd_bus_t *bus, uint32 daddr32)
{
	volatile uint32 data;

#ifdef BCM_ROUTER_DHD
	if (OSL_ACP_WAR_ENAB() || OSL_ARCH_IS_COHERENT())
		data = R_REG(bus->dhd->osh, (volatile uint32 *)(bus->tcm + daddr32));
	else
#endif /* BCM_ROUTER_DHD */
		data = *(volatile uint32 *)(bus->tcm + daddr32);

	return data;
}

#ifndef BCMQT
uint64
dhdpcie_bus_read_u64(dhd_bus_t *bus, uint32 daddr32)
{
	volatile uint64 data;

#ifdef BCM_ROUTER_DHD
	if (OSL_ACP_WAR_ENAB() || OSL_ARCH_IS_COHERENT())
#ifdef CONFIG_64BIT
		data = R_REG(bus->dhd->osh, (volatile uint64 *)(bus->tcm + daddr32));
#else /* !CONFIG_64BIT */
	{
		volatile uint64 datah;

		data = R_REG(bus->dhd->osh, (volatile uint32 *)(bus->tcm + daddr32));
		datah = R_REG(bus->dhd->osh,
			(volatile uint32 *)(bus->tcm + daddr32 + sizeof(uint32)));
		data |= (datah << 32);
	}
#endif /* !CONFIG_64BIT */
	else
#endif /* BCM_ROUTER_DHD */
		data = *(volatile uint64 *)(bus->tcm + daddr32);

	return data;
}
#endif /* !BCMQT */

/**
 * Write to a specific memory location in dongle's memory, that is a field of
 * the shared structures pcie_ipc_t, pcie_ipc_rings_t or pcie_ipc_ring_mem_t
 *
 * All data is written in LittleEndian format.
 * Size of data is determined by type.
 */
void
dhd_bus_cmn_writeshared(dhd_bus_t *bus, void *data, uint8 type, uint16 ringid)
{
	uint64 long_data;
	daddr32_t daddr32; /* address in dongle's memory */

	switch (type) {
		case HOST_MEM_BUF_ADDR: /* Address of table of HME addresses */
			long_data = HTOL64(*(uint64 *)data);
			daddr32   = bus->pcie_ipc_daddr32;
			daddr32  += OFFSETOF(pcie_ipc_t, host_mem_haddr64);
			dhdpcie_bus_membytes(bus, PCIE_SYSMEM_WRITE, daddr32,
			                     (uint8 *) &long_data, 8);
			break;

		case HOST_MEM_BUF_LEN: /* Total bytes of all HME regions */
			daddr32   = bus->pcie_ipc_daddr32;
			daddr32  += OFFSETOF(pcie_ipc_t, host_mem_len);
			dhdpcie_bus_write_u32(bus, daddr32, (uint32) HTOL32(*(uint32 *)data));
			break;

		case H2D_DMA_INDX_WR_BUF:
			long_data = HTOL64(*(uint64 *)data);
			daddr32   = bus->pcie_ipc_rings_daddr32;
			daddr32  += OFFSETOF(pcie_ipc_rings_t, h2d_wr_haddr64);
			dhdpcie_bus_membytes(bus, PCIE_SYSMEM_WRITE, daddr32,
			                     (uint8 *) &long_data, 8);
			break;

		case H2D_DMA_INDX_RD_BUF:
			long_data = HTOL64(*(uint64 *)data);
			daddr32   = bus->pcie_ipc_rings_daddr32;
			daddr32  += OFFSETOF(pcie_ipc_rings_t, h2d_rd_haddr64);
			dhdpcie_bus_membytes(bus, PCIE_SYSMEM_WRITE, daddr32,
			                     (uint8 *) &long_data, 8);
			break;

		case D2H_DMA_INDX_WR_BUF:
			long_data = HTOL64(*(uint64 *)data);
			daddr32   = bus->pcie_ipc_rings_daddr32;
			daddr32  += OFFSETOF(pcie_ipc_rings_t, d2h_wr_haddr64);
			dhdpcie_bus_membytes(bus, PCIE_SYSMEM_WRITE, daddr32,
			                     (uint8 *) &long_data, 8);
			break;

		case D2H_DMA_INDX_RD_BUF:
			long_data = HTOL64(*(uint64 *)data);
			daddr32   = bus->pcie_ipc_rings_daddr32;
			daddr32  += OFFSETOF(pcie_ipc_rings_t, d2h_rd_haddr64);
			dhdpcie_bus_membytes(bus, PCIE_SYSMEM_WRITE, daddr32,
			                     (uint8 *) &long_data, 8);
			break;

		case D2H_MB_DATA:
			daddr32  = bus->pcie_ipc_d2h_mb_daddr32;
			dhdpcie_bus_write_u32(bus, daddr32, (uint32) HTOL32(*(uint32 *)data));
			break;

		case H2D_MB_DATA:
			daddr32  = bus->pcie_ipc_h2d_mb_daddr32;
			dhdpcie_bus_write_u32(bus, daddr32, (uint32) HTOL32(*(uint32 *)data));
			break;

		case RING_MEM_ID:
			daddr32   = bus->ring_mem[ringid].daddr32;
			daddr32  += OFFSETOF(pcie_ipc_ring_mem_t, id);
			dhdpcie_bus_write_u8(bus, daddr32, *(uint8 *)data);
			break;

		case RING_ITEM_TYPE:
			daddr32   = bus->ring_mem[ringid].daddr32;
			daddr32  += OFFSETOF(pcie_ipc_ring_mem_t, item_type);
			dhdpcie_bus_write_u8(bus, daddr32, *(uint8 *)data);
			break;

		case RING_MAX_ITEMS:
			daddr32   = bus->ring_mem[ringid].daddr32;
			daddr32  += OFFSETOF(pcie_ipc_ring_mem_t, max_items);
			dhdpcie_bus_write_u16(bus, daddr32, (uint16) HTOL16(*(uint16 *)data));
			break;

		case RING_ITEM_SIZE:
			daddr32   = bus->ring_mem[ringid].daddr32;
			daddr32  += OFFSETOF(pcie_ipc_ring_mem_t, item_size);
			dhdpcie_bus_write_u16(bus, daddr32, (uint16) HTOL16(*(uint16 *)data));
			break;

		case RING_BASE_ADDR:
			long_data = HTOL64(*(uint64 *)data);
			daddr32   = bus->ring_mem[ringid].daddr32;
			daddr32  += OFFSETOF(pcie_ipc_ring_mem_t, haddr64);
			dhdpcie_bus_membytes(bus, PCIE_SYSMEM_WRITE, daddr32,
			                     (uint8 *) &long_data, 8);
			break;

		case RING_RD_UPD:
			daddr32   = bus->ring_mem[ringid].rd_daddr32;
			dhdpcie_bus_write_u16(bus, daddr32, (uint16) HTOL16(*(uint16 *)data));
			break;

		case RING_WR_UPD:
			daddr32   = bus->ring_mem[ringid].wr_daddr32;
			dhdpcie_bus_write_u16(bus, daddr32, (uint16) HTOL16(*(uint16 *)data));
			break;

		case RING_QDWR_UPD:
			daddr32   = bus->ring_mem[ringid].wr_daddr32;
			dhdpcie_bus_write_u32(bus, daddr32, (uint32) HTOL32(*(uint32 *)data));
			break;

		case IPC_HOST_CAP1:
			daddr32   = bus->pcie_ipc_daddr32;
			daddr32  += OFFSETOF(pcie_ipc_t, hcap1);
			dhdpcie_bus_write_u32(bus, daddr32, (uint32) HTOL32(*(uint32 *)data));
			break;

		case IPC_HOST_CAP2:
			daddr32   = bus->pcie_ipc_daddr32;
			daddr32  += OFFSETOF(pcie_ipc_t, hcap2);
			dhdpcie_bus_write_u32(bus, daddr32, (uint32) HTOL32(*(uint32 *)data));
			break;

		case IPC_HOST_PHYSADDRHI:
			daddr32   = bus->pcie_ipc_daddr32;
			daddr32  += OFFSETOF(pcie_ipc_t, host_physaddrhi);
			dhdpcie_bus_write_u32(bus, daddr32, (uint32) HTOL32(*(uint32 *)data));
			break;

		case IPC_HOST_RXBUFLEN:
			daddr32   = bus->pcie_ipc_rings_daddr32;
			daddr32  += OFFSETOF(pcie_ipc_rings_t, rxpost_data_buf_len);
			dhdpcie_bus_write_u16(bus, daddr32, (uint16) HTOL16(*(uint16 *)data));
			break;

		case BUZZZ_HOSTMEM_SEGMENTS:
			daddr32   = bus->pcie_ipc.buzzz_daddr32;
			if (daddr32 == 0U) {
				DHD_ERROR(("BUZZZ not enabled in dongle firmware.\n"));
				break;
			}
#if defined(BCM_BUZZZ_STREAMING_BUILD)
			daddr32  += OFFSETOF(bcm_buzzz_t, seg_haddr32);
			dhdpcie_bus_membytes(bus, PCIE_SYSMEM_WRITE, daddr32, (uint8*)data,
				BCM_BUZZZ_SEGMENTS * sizeof(uint32));
#endif /* BCM_BUZZZ_STREAMING_BUILD */
			break;

		default:
			daddr32 = 0;
			ASSERT(0);
			break;
	}

	DHD_INFO(("DHD WR daddr32 0x%08x type %d\n", (int)daddr32, type));

} /* dhd_bus_cmn_writeshared */

/**
 * Read from a specific memory location in dongle's memory, that is a field of
 * the shared structures pcie_ipc_t, pcie_ipc_rings_t or pcie_ipc_ring_mem_t
 *
 * All data is read in LittleEndian format and converted to host endian format.
 */
void
dhd_bus_cmn_readshared(dhd_bus_t *bus, void* data, uint8 type, uint16 ringid)
{
	daddr32_t daddr32;

	switch (type) {

		case RING_RD_UPD:
			daddr32  = bus->ring_mem[ringid].rd_daddr32;
			*(uint16*)data = LTOH16(dhdpcie_bus_read_u16(bus, daddr32));
			break;

		case RING_WR_UPD:
			daddr32  = bus->ring_mem[ringid].wr_daddr32;
			*(uint16*)data = LTOH16(dhdpcie_bus_read_u16(bus, daddr32));
			break;

		case RING_QDWR_UPD:
			daddr32  = bus->ring_mem[ringid].wr_daddr32;
			*(uint32*)data = LTOH32(dhdpcie_bus_read_u32(bus, daddr32));
			break;

		case H2D_MB_DATA:
			daddr32  = bus->pcie_ipc_h2d_mb_daddr32;
			*(uint32*)data = LTOH32(dhdpcie_bus_read_u32(bus, daddr32));
			break;

		case D2H_MB_DATA:
			daddr32  = bus->pcie_ipc_d2h_mb_daddr32;
			*(uint32*)data = LTOH32(dhdpcie_bus_read_u32(bus, daddr32));
			break;

		case IPC_REVISION:
			daddr32  = bus->pcie_ipc_daddr32;
			daddr32 += OFFSETOF(pcie_ipc_t, flags);
			*(uint32*)data = LTOH32(dhdpcie_bus_read_u32(bus, daddr32));
			*(uint32*)data = *(uint32*)data & PCIE_IPC_FLAGS_REVISION_MASK;
			break;

		case MAX_TX_PKTS:
			daddr32  = bus->pcie_ipc_daddr32;
			daddr32 += OFFSETOF(pcie_ipc_t, max_tx_pkts);
			*(uint16*)data = LTOH16(dhdpcie_bus_read_u16(bus, daddr32));
			break;

		case MAX_RX_PKTS:
			daddr32  = bus->pcie_ipc_daddr32;
			daddr32 += OFFSETOF(pcie_ipc_t, max_rx_pkts);
			*(uint16*)data = LTOH16(dhdpcie_bus_read_u16(bus, daddr32));
			break;

		case BUZZZ_STATE:
			daddr32  = bus->pcie_ipc_daddr32;
			daddr32 += OFFSETOF(pcie_ipc_t, buzzz_daddr32);
			*(uint32*)data = LTOH32(dhdpcie_bus_read_u32(bus, daddr32));
			break;

		default :
			daddr32 = 0;
			ASSERT(0);
			break;
	}

	DHD_INFO(("DHD RD daddr32 0x%08x type %d\n", (int)daddr32, type));

} /* dhd_bus_cmn_readshared */

void
dhd_bus_clearcounts(dhd_pub_t *dhdp)
{
	uint16 flowid;
	flow_ring_node_t *flow_ring_node;

	for (flowid = 0; flowid < dhdp->num_flow_rings; flowid++) {
		flow_ring_node = DHD_FLOW_RING(dhdp, flowid);
		if (!flow_ring_node->active)
			continue;
		DHD_FLOW_QUEUE_FAILURES(&flow_ring_node->queue) = 0;
	}
} /* dhd_bus_clearcounts */

/**
 * @param params    input buffer, NULL for 'set' operation.
 * @param plen      length of 'params' buffer, 0 for 'set' operation.
 * @param arg       output buffer
 */
int
dhd_bus_iovar_op(dhd_pub_t *dhdp, const char *name,
                 void *params, int plen, void *arg, int len, bool set)
{
	dhd_bus_t *bus = dhdp->bus;
	const bcm_iovar_t *vi = NULL;
	int bcmerror = 0;
	int val_size;
	uint32 actionid;

	DHD_TRACE(("%s: Enter\n", __FUNCTION__));

	ASSERT(name);
	ASSERT(len >= 0);

	/* Get MUST have return space */
	ASSERT(set || (arg && len));

	/* Set does NOT take qualifiers */
	ASSERT(!set || (!params && !plen));

	DHD_INFO(("%s: %s %s, len %d plen %d\n", __FUNCTION__,
	         name, (set ? "set" : "get"), len, plen));

	/* Look up var locally; if not found pass to host driver */
	if ((vi = bcm_iovar_lookup(dhdpcie_iovars, name)) == NULL) {
		goto exit;
	}

	/* set up 'params' pointer in case this is a set command so that
	 * the convenience int and bool code can be common to set and get
	 */
	if (params == NULL) {
		params = arg;
		plen = len;
	}

	if (vi->type == IOVT_VOID)
		val_size = 0;
	else if (vi->type == IOVT_BUFFER)
		val_size = len;
	else
		/* all other types are integer sized */
		val_size = sizeof(int);

	actionid = set ? IOV_SVAL(vi->varid) : IOV_GVAL(vi->varid);
	bcmerror = dhdpcie_bus_doiovar(bus, vi, actionid, name, params, plen, arg, len, val_size);

exit:
	return bcmerror;
} /* dhd_bus_iovar_op */

/* Ring DoorBell1 to indicate Hostready */
void
dhd_bus_hostready(struct dhd_bus *bus)
{
	/* Request dongle to resume into pciedev_init_sharedbufs */
	if ((bus->pcie_ipc.flags & PCIE_IPC_FLAGS_HOSTRDY_SUPPORT) == 0) {
		return;
	}

	/*
	 * XXX
	 *
	 * KUDU pciedev uses PD_DEV0_DB1_INTMASK -> pciedev_handle_hostready_intr
	 * to resume pciedev_init_sharedbufs when common_rings_attached = FALSE
	 *
	 * 10.10 and 7.35 firmware uses the first ioctl (Ctrl submit). PCIH2D_DB1
	 * is never used.
	 */
	si_corereg(bus->sih, bus->sih->buscoreidx, PCIH2D_DB1, ~0, 0x12345678);
	DHD_INFO(("%s: Ring Hostready\n", __FUNCTION__));
}

#if defined(BCM_BUZZZ)
#include <bcm_buzzz.h>

static int dhd_buzzz_dump_indent(char *page_p, uint32 indent);

/* BCM_BUZZZ_FUNC: Function Entry/Exit pretty print indentation */
#define BCM_BUZZZ_FUNC_INDENT_STR   "  "

static int
dhd_buzzz_dump_indent(char *page_p, uint32 indent)
{
	int i, bytes = 0;
	for (i = 0; i < indent; i++)
		bytes += sprintf(page_p + bytes, "%s", BCM_BUZZZ_FUNC_INDENT_STR);
	return bytes;
}

#if !defined(BCM_BUZZZ_STREAMING_BUILD)
static int dhd_buzzz_dump_cntrs(char *p, uint32 *core, uint32 *log, const int num_counters);
static int dhd_bcm_buzzz_dump_cntrs6(char *p, uint32 *core, uint32 *log);
static int dhd_buzzz_dump_log(bcm_buzzz_t *buzzz_p, uint32 *u32_p, char *page_p,
	uint32 *core, uint32 *indent_p);
static void _dhd_buzzz_dump(bcm_buzzz_t *buzzz_p, void *buffer_p, char *page_p);

static int
dhd_buzzz_dump_cntrs(char *p, uint32 *core, uint32 *log,
	const int num_counters)
{
	int bytes = 0;
	uint32 ctr;
	uint32 curr[BCM_BUZZZ_COUNTERS_MAX], prev[BCM_BUZZZ_COUNTERS_MAX];
	uint32 delta[BCM_BUZZZ_COUNTERS_MAX];

	/* Compute elapsed counter values per counter event type */
	for (ctr = 0U; ctr < num_counters; ctr++) {
		prev[ctr] = core[ctr];
		curr[ctr] = *log++;
		core[ctr] = curr[ctr];  /* saved for next log */

		if (curr[ctr] < prev[ctr])
			delta[ctr] = curr[ctr] + (~0U - prev[ctr]);
		else
			delta[ctr] = (curr[ctr] - prev[ctr]);

		bytes += sprintf(p + bytes, "%12u ", delta[ctr]);
	}

	return bytes;
} /* dhd_buzzz_dump_cntrs */

int /* Cortex M3 counters handling */
dhd_bcm_buzzz_dump_cntrs6(char *p, uint32 *core, uint32 *log)
{
	int bytes = 0;

	uint32 cyccnt, instrcnt;
	bcm_buzzz_cm3_cnts_t cm3_cnts;
	uint8 foldcnt;

	{   /* 32bit cyccnt */
		uint32 curr, prev, delta;
		prev = core[0]; curr = *log++; core[0] = curr;
		if (curr < prev)
			delta = curr + (~0U - prev);
		else
			delta = (curr - prev);

		bytes += sprintf(p + bytes, "%12u ", delta);
		cyccnt = delta;
	}

	{	/* Extract the 4 cnts: cpi, exc, sleep and lsu */
		int i;
		uint8 max8 = ~0;
		bcm_buzzz_cm3_cnts_t curr, prev, delta;
		prev.u32 = core[1]; curr.u32 = * log++; core[1] = curr.u32;
		for (i = 0; i < 4; i++) {
			if (curr.u8[i] < prev.u8[i])
				delta.u8[i] = curr.u8[i] + (max8 - prev.u8[i]);
			else
				delta.u8[i] = (curr.u8[i] - prev.u8[i]);
			bytes += sprintf(p + bytes, "%4u ", delta.u8[i]);
		}
		cm3_cnts.u32 = delta.u32;
	}

	{   /* Extract the foldcnt from arg0 */
		uint8 curr, prev, delta, max8 = ~0;
		bcm_buzzz_arg0_t arg0; arg0.u32 = *log;
		prev = core[2]; curr = arg0.klog.cnt; core[2] = curr;
		if (curr < prev)
			delta = curr + (max8 - prev);
		else
			delta = (curr - prev);
		bytes += sprintf(p + bytes, "%4u ", delta);
		foldcnt = delta;
	}

	instrcnt = cyccnt - (cm3_cnts.u8[0] + cm3_cnts.u8[1] + cm3_cnts.u8[2]
		                 + cm3_cnts.u8[3]) + foldcnt;
	if (instrcnt > 0xFFFFFF00)
		bytes += sprintf(p + bytes, "[%10s] ", "~");
	else
		bytes += sprintf(p + bytes, "[%10u] ", instrcnt);
	return bytes;
} /* dhd_bcm_buzzz_dump_cntrs6 */

/* Dump a buzzz log: func or kevt mode. */
static int /* Convert log to formatted output using event id to format string */
dhd_buzzz_dump_log(bcm_buzzz_t *buzzz_p, uint32 *u32_p, char *page_p,
	uint32 *core, uint32 *indent_p)
{
	int bytes = 0;
	bcm_buzzz_arg0_t arg0;
	static uint8 * fmt[] = BCM_BUZZZ_FMT_STRINGS;

	if (buzzz_p->mode == BCM_BUZZZ_MODE_EVENT) {
		if (buzzz_p->counters == 6) {
			bytes += dhd_bcm_buzzz_dump_cntrs6(page_p + bytes, core, u32_p);
			u32_p += 2; /* 32bit cyccnt + (4 x 8bit) CM3 */
		} else {
			bytes += dhd_buzzz_dump_cntrs(page_p + bytes, core, u32_p,
				buzzz_p->counters);
			u32_p += buzzz_p->counters; /* (N x 32bit) CR4=3, CA7=4 */
		}
	}

	/* Dump the logged arguments using the registered formats */
	arg0.u32 = *u32_p++;

	if (arg0.klog.id == BUZZZ_KLOG__FUNC_ENT) {
		uint32 called = *u32_p;
		bytes += dhd_buzzz_dump_indent(page_p + bytes, *indent_p);
		bytes += sprintf(page_p + bytes, fmt[arg0.klog.id], called);
		*indent_p = *indent_p + 1;
	} else if (arg0.klog.id == BUZZZ_KLOG__FUNC_EXT) {
		uint32 called = *u32_p;
		if (*indent_p > 0U) *indent_p = *indent_p - 1;
		bytes += dhd_buzzz_dump_indent(page_p + bytes, *indent_p);
		bytes += sprintf(page_p + bytes, fmt[arg0.klog.id], called);
	} else {

		switch (arg0.klog.args) {
			case 0:
			{
				bytes += sprintf(page_p + bytes, fmt[arg0.klog.id]);
				break;
			}
			case 1:
			{
				uint32 arg1 = *u32_p;
				bytes += sprintf(page_p + bytes, fmt[arg0.klog.id], arg1);
				break;
			}
			case 2:
			{
				uint32 arg1, arg2;
				arg1 = *u32_p++; arg2 = *u32_p;
				bytes += sprintf(page_p + bytes, fmt[arg0.klog.id], arg1, arg2);
				break;
			}
			case 3:
			{
				uint32 arg1, arg2, arg3;
				arg1 = *u32_p++; arg2 = *u32_p++; arg3 = *u32_p;
				bytes += sprintf(page_p + bytes,
					fmt[arg0.klog.id], arg1, arg2, arg3);
				break;
			}
			case 4:
			{
				uint32 arg1, arg2, arg3, arg4;
				arg1 = *u32_p++; arg2 = *u32_p++; arg3 = *u32_p++; arg4 = *u32_p;
				bytes += sprintf(page_p + bytes,
					fmt[arg0.klog.id], arg1, arg2, arg3, arg4);
				break;
			}
			default:
				printf("Maximum 4 arguments supported\n");
				break;
		}
	}

	bytes += sprintf(page_p + bytes, "\n");

	return bytes;
} /* dhd_buzzz_dump_log */

static void /* Given dongle buzzz state, dump starting from cur with wrapover */
_dhd_buzzz_dump(bcm_buzzz_t *buzzz_p, void *buffer_p, char *page_p)
{
	int i;
	uint32 indent, total, part1, part2, log_sz, core[BCM_BUZZZ_COUNTERS_MAX];
	uint32 * u32_p;

	indent = 0U;
	for (i = 0; i < BCM_BUZZZ_COUNTERS_MAX; i++) {
		core[i] = 0;
	}

	log_sz = buzzz_p->log_sz;

	part1 = (buzzz_p->cur - buzzz_p->log) / log_sz;

	if (buzzz_p->wrap == TRUE) {
		part2 = (buzzz_p->end - buzzz_p->cur) / log_sz;
		total = (buzzz_p->buffer_sz - BCM_BUZZZ_LOGENTRY_MAXSZ) / log_sz;
	} else {
		part2 = 0U;
		total = buzzz_p->count;
	}

	if (total == 0U) {
		printf("bcm_buzzz_dump total<%u> done\n", total);
		return;
	} else {
		printf("bcm_buzzz_dump total<%u> : part2<%u> + part1<%u>\n",
		       total, part2, part1);
	}

	if (part2) {   /* with wrap */
		u32_p = (uint32 *)((uintptr)buffer_p + (buzzz_p->cur - buzzz_p->log));
		while (part2--) {   /* from cur to end : part2 */
			page_p[0] = '\0';
			dhd_buzzz_dump_log(buzzz_p, u32_p, page_p, core, &indent);
			printf("%s", page_p);
			u32_p = (uint32 *)((uintptr)u32_p + buzzz_p->log_sz);
		}
	}

	u32_p = (uint32 *)buffer_p;
	while (part1--) {
		page_p[0] = '\0';
		dhd_buzzz_dump_log(buzzz_p, u32_p, page_p, core, &indent);
		printf("%s", page_p);
		u32_p = (uint32 *)((uintptr)u32_p + buzzz_p->log_sz);
	}

	printf("dhd_buzzz_dump done.\n");
} /* dhd_buzzz_dump() */

static int /* IOV_BUZZZ_DUMP: Fetch dongle buzzz log, parse and pretty print */
dhd_buzzz_dump_bus(dhd_bus_t *bus)
{
	uint32 buzzz_daddr32 = 0U;
	bcm_buzzz_t * buzzz_p = NULL;
	void * buffer_p = NULL;
	char * page_p = NULL;

	if (bus->dhd->busstate != DHD_BUS_DATA) {
		return BCME_UNSUPPORTED;
	}
	if ((page_p = (char *)MALLOC(bus->dhd->osh, 4096)) == NULL) {
		printf("Page memory allocation failure\n");
		goto done;
	}
	if ((buzzz_p = MALLOC(bus->dhd->osh, sizeof(bcm_buzzz_t))) == NULL) {
		printf("BCM BUZZZ memory allocation failure\n");
		goto done;
	}

	/* Fetch the location of buzzz state in dongle */
	buzzz_daddr32 = bus->pcie_ipc.buzzz_daddr32;
	if (buzzz_daddr32 == 0U) {
		dhd_bus_cmn_readshared(bus, &buzzz_daddr32, BUZZZ_STATE, 0);
		bus->pcie_ipc.buzzz_daddr32 = buzzz_daddr32;
	}

	DHD_INFO(("%s buzzz state daddr32  %08x\n", __FUNCTION__, buzzz_daddr32));

	/* Fetch and display dongle BUZZZ Trace */
	if (buzzz_daddr32 != 0U) {

		dhdpcie_bus_membytes(bus, PCIE_SYSMEM_READ, buzzz_daddr32,
			(uint8 *)buzzz_p, sizeof(bcm_buzzz_t));

		printf("BUZZZ[0x%08x]: log<0x%08x> cur<0x%08x> end<0x%08x> "
			"count<%u> status<%u> wrap<%u> mode<%u> skip<%u>\n"
			"cpu<0x%02X> counters<%u> group<%u> buffer_sz<%u> log_sz<%u>\n",
			(int)buzzz_daddr32,
			(int)buzzz_p->log, (int)buzzz_p->cur, (int)buzzz_p->end,
			buzzz_p->count, buzzz_p->status, buzzz_p->wrap,
			buzzz_p->mode, buzzz_p->skip,
			buzzz_p->cpu_idcode, buzzz_p->counters, buzzz_p->group,
			buzzz_p->buffer_sz, buzzz_p->log_sz);

		if (buzzz_p->count == 0) {
			printf("Empty dongle BUZZZ trace\n\n");
			goto done;
		}

		/* Allocate memory for trace buffer strings */
		buffer_p = MALLOC(bus->dhd->osh, buzzz_p->buffer_sz);
		if (buffer_p == NULL) {
			printf("Buffer memory allocation failure\n");
			goto done;
		}

		/* Fetch trace buffer. format strings are exported via bcm_buzzz.h */
		dhdpcie_bus_membytes(bus, PCIE_SYSMEM_READ, (uint32)buzzz_p->log,
			(uint8 *)buffer_p, buzzz_p->buffer_sz);

		/* Process and display the trace using formatted output */

		if (buzzz_p->mode == BCM_BUZZZ_MODE_EVENT) {
			int ctr;
			for (ctr = 0; ctr < buzzz_p->counters; ctr++) {
				printf("<Evt[%02X]> ", buzzz_p->eventid[ctr]);
			}
			printf("<code execution point>\n");
		}

		_dhd_buzzz_dump(buzzz_p, buffer_p, page_p);

		printf("----- End of dongle BCM BUZZZ Trace -----\n\n");

		MFREE(bus->dhd->osh, buffer_p, buzzz_p->buffer_sz); buffer_p = NULL;
	}

done:

	if (page_p)   MFREE(bus->dhd->osh, page_p, 4096);
	if (buzzz_p)  MFREE(bus->dhd->osh, buzzz_p, sizeof(bcm_buzzz_t));
	if (buffer_p) MFREE(bus->dhd->osh, buffer_p, buzzz_p->buffer_sz);

	return BCME_OK;

} /* dhd_buzzz_dump_bus() */

static int /* IOV_BUZZZ_FILE: Unsupported. available only for streaming mode */
dhd_buzzz_file_bus(dhd_bus_t *bus)
{
	printk("BUZZZ save log to file unsupported. Use buzzz_dump\n");

	return BCME_UNSUPPORTED;
} /* dhd_buzzz_file_bus() */

#else  /* BCM_BUZZZ_STREAMING_BUILD */

static void dhd_buzzz_dump_log(char *p, uint64 *tcnt, uint32 *ccnt, uint32 cusec,
	bcm_buzzz_log_t *log_p, uint32 *indent_p);

static void /* Dump log to a formatted output */
dhd_buzzz_dump_log(char *p, uint64 *tcnt, uint32 *ccnt, uint32 cusec,
	bcm_buzzz_log_t *log_p, uint32 *indent_p)
{
	int bytes = 0;
	uint8 id = log_p->id;
	static uint8 * fmt[] = BCM_BUZZZ_FMT_STRINGS;
#if !defined(BCM_BUZZZ_FUNC)
	uint32 prev, diff; /* counter manipulation */

	prev = *ccnt;
	*ccnt = log_p->cycctr;

	/* compute diff based on previous */
	diff = (*ccnt < prev) ?  (*ccnt + (~0U - prev)) : (*ccnt - prev);

	*tcnt += diff; /* add to running total */

	bytes += sprintf(p, "%16llu %12u : ", (uint64)((uint32)*tcnt / cusec), diff);
#endif /* ! BCM_BUZZZ_FUNC */

	if (log_p->id == BUZZZ_KLOG__FUNC_ENT) { /* Function entry */
		bytes += dhd_buzzz_dump_indent(p + bytes, *indent_p);
		bytes += sprintf(p + bytes, fmt[log_p->id], log_p->arg1);
		*indent_p = *indent_p + 1;
	} else if (log_p->id == BUZZZ_KLOG__FUNC_EXT) { /* Function exit */
		if (*indent_p > 0U) *indent_p = *indent_p - 1;
		bytes += dhd_buzzz_dump_indent(p + bytes, *indent_p);
		bytes += sprintf(p + bytes, fmt[log_p->id], log_p->arg1);
	} else if (log_p->id == BUZZZ_KLOG__FUNC_LINE) {
		bytes += dhd_buzzz_dump_indent(p + bytes, *indent_p);
		bytes += sprintf(p + bytes, fmt[log_p->id], log_p->arg1);
	} else {
		switch (log_p->args) {
			case 0:
				sprintf(p + bytes, fmt[id]);
				break;
			case 1:
				sprintf(p + bytes, fmt[id], log_p->arg1);
				break;
			case 2:
				sprintf(p + bytes, fmt[id], log_p->arg1, log_p->arg2);
				break;
#if !defined(BCM_BUZZZ_FUNC)
			case 3:
				sprintf(p + bytes, fmt[id], log_p->arg1, log_p->arg2, log_p->arg3);
				break;
#endif /* ! BCM_BUZZZ_FUNC */
		}
	}

	printf("%s\n", p);
}

static int /* IOV_BUZZZ_DUMP: Fetch dongle buzzz log, parse and pretty print */
dhd_buzzz_dump_bus(dhd_bus_t *bus)
{
	uint32 indent = 0U;
	uint64 tcnt = 0;
	uint32 seg, logs, ccnt, cusec;
	char * page_p = NULL;
	bcm_buzzz_log_t *log_p;
	const uint32 logs_max = BCM_BUZZZ_HOSTMEM_SEGSZ / sizeof(bcm_buzzz_log_t);

	if (bus->dhd == NULL) return BCME_NOTUP;
	if (bus->dhd->busstate != DHD_BUS_DATA) return BCME_UNSUPPORTED;
	if (bus->dhd->bcm_buzzz_va[BCM_BUZZZ_SEGMENTS-1] == NULL) return BCME_NOTUP;

	if ((page_p = (char *)MALLOC(bus->dhd->osh, 4096)) == NULL) {
		printf("Page memory allocation failure\n");
		goto done;
	}

	log_p = (bcm_buzzz_log_t *)(bus->dhd->bcm_buzzz_va[0]);

	/* peek into first log entry */
#if !defined(BCM_BUZZZ_FUNC)
	ccnt  = log_p->cycctr; /* ignore elapsed diff for first as no previous */
#else
	ccnt  = 0U; /* no cycle counter in function streaming */
#endif
	cusec = log_p->arg1; /* Cycles per usec ... Mhz of dongle */

	printf("BUZZZ Streaming[%s] FWID<0x%x,0x%x> IPC rev %u, chip 0x%x %u\n"
		"    Cycles Per Usec = %u\n"
		"    Size bcm_buzzz_log_t = %d\n",
#if defined(BCM_BUZZZ_FUNC)
		"FUNC",
#else
		"KEVT",
#endif
		bus->pcie_ipc.fwid, log_p->arg2,
		bus->pcie_ipc.flags & PCIE_IPC_FLAGS_REVISION_MASK,
		bus->sih->chip, bus->sih->chiprev, log_p->arg1,
		(int)sizeof(bcm_buzzz_log_t));

#if !defined(BCM_BUZZZ_FUNC)
	printf("%16s %12s : Event Description\n\n", "Time (Usecs)", "Diff(cycles)");
#endif

	for (seg = 0; seg < BCM_BUZZZ_SEGMENTS; seg++) {
		log_p = (bcm_buzzz_log_t *)bus->dhd->bcm_buzzz_va[seg];

		for (logs = 0; logs < logs_max; logs++) {
			if (log_p->u32[0] == 0U) break;
			dhd_buzzz_dump_log(page_p, &tcnt, &ccnt, cusec, log_p, &indent);
			log_p++;
		} /* for all logs in a segment */

		/* Prepare this segment buffer for next run. Zero out entire segment. */
		memset(bus->dhd->bcm_buzzz_va[seg], 0, BCM_BUZZZ_HOSTMEM_SEGSZ);
		OSL_CACHE_FLUSH(bus->dhd->bcm_buzzz_va[seg], BCM_BUZZZ_HOSTMEM_SEGSZ);

		if (logs < logs_max)
			break; /* current segment not full, no need to check next segment */
	} /* for all segments */

done:

	if (page_p)   MFREE(bus->dhd->osh, page_p, 4096);

	return BCME_OK;

} /* dhd_buzzz_dump_bus() */

static int /* IOV_BUZZZ_FILE: Transfer buzzz log from host memory to file */
dhd_buzzz_file_bus(dhd_bus_t *bus)
{
	void *buf;
	uint32 seg;
	loff_t pos = 0;
	ssize_t ret, len = BCM_BUZZZ_HOSTMEM_SEGSZ;
	const char *fname = BCM_BUZZZ_STREAMING_FILE;
	int flags = O_RDWR | O_CREAT | O_LARGEFILE | O_SYNC;   // O_DSYNC

	mm_segment_t old_fs;
	struct file *buzzz_file;

	if (bus->dhd == NULL) return BCME_NOTUP;
	if (bus->dhd->busstate != DHD_BUS_DATA) return BCME_UNSUPPORTED;
	if (bus->dhd->bcm_buzzz_va[BCM_BUZZZ_SEGMENTS-1] == NULL) return BCME_NOTUP;

	old_fs = get_fs(); // current addr_limit: user|lernel space
	set_fs(KERNEL_DS); // change to "kernel data segment" address limit

	buzzz_file = filp_open(fname, flags, 0600);
	if (IS_ERR(buzzz_file)) {
		printk("%s filp_open(%s) failed\n", __FUNCTION__, fname);
		ret = PTR_ERR(buzzz_file);
		goto exit;
	}

	printk("\nCreating %s from %u size %u segments\n",
		fname, BCM_BUZZZ_HOSTMEM_SEGSZ, BCM_BUZZZ_SEGMENTS);
	printk("\tSEG\tADDRESS\tOFFSET\n");

	for (seg = 0; seg < BCM_BUZZZ_SEGMENTS; seg++)
	{
		buf = bus->dhd->bcm_buzzz_va[seg];
		printk("\t%u.\t0x%p\t%u\n", seg, buf, (uint32)pos);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 0, 0))
		ret = kernel_write(buzzz_file, buf, len, &pos);
		if (ret != len) {
			printk("%s kernel_write %s failed: %zd\n", __FUNCTION__, fname, ret);
			if (ret > 0)
				ret = -EIO;
			goto exit;
		}
#else /* LINUX_VERSION_CODE < 4 */
		buzzz_file->f_op->write(buzzz_file, buf, len, &pos); // not tested ...
#endif
		memset(buf, 0, len);
		OSL_CACHE_FLUSH(buf, len);
	}

	printk("Created %s size %u\n\n\n", fname, (uint32)pos);
	printk("\tlabPC]$ nc -l 80 > %s\n", fname);
	printk("\t# cat %s | /usr/bin/nc <lab.pc.ip.addr> 80\n\n", fname);

	ret = BCME_OK;

exit:
	filp_close(buzzz_file, NULL);
	set_fs(old_fs); // restore saved address limit

	return (int)ret;

}   /* dhd_buzzz_file_bus() */

#endif /* BCM_BUZZZ_STREAMING_BUILD */
#endif /* BCM_BUZZZ */

#define PCIE_GEN2(sih) ((BUSTYPE((sih)->bustype) == PCI_BUS) &&	\
	((sih)->buscoretype == PCIE2_CORE_ID))

static bool
pcie2_mdiosetblock(dhd_bus_t *bus, uint blk)
{
	uint mdiodata, mdioctrl, i = 0;
	uint pcie_serdes_spinwait = 200;

	mdioctrl = MDIOCTL2_DIVISOR_VAL | (0x1F << MDIOCTL2_REGADDR_SHF);
	mdiodata = (blk << MDIODATA2_DEVADDR_SHF) | MDIODATA2_DONE;

	si_corereg(bus->sih, bus->sih->buscoreidx, PCIE2_MDIO_CONTROL, ~0, mdioctrl);
	si_corereg(bus->sih, bus->sih->buscoreidx, PCIE2_MDIO_WR_DATA, ~0, mdiodata);

	OSL_DELAY(10);
	/* retry till the transaction is complete */
	while (i < pcie_serdes_spinwait) {
		uint mdioctrl_read = si_corereg(bus->sih, bus->sih->buscoreidx, PCIE2_MDIO_WR_DATA,
			0, 0);
		if (!(mdioctrl_read & MDIODATA2_DONE)) {
			break;
		}
		OSL_DELAY(1000);
		i++;
	}

	if (i >= pcie_serdes_spinwait) {
		DHD_ERROR(("pcie_mdiosetblock: timed out\n"));
		return FALSE;
	}

	return TRUE;
}

int
dhd_bus_devreset(dhd_pub_t *dhdp, uint8 flag)
{
	dhd_bus_t *bus = dhdp->bus;
	int bcmerror = 0;
#ifdef OEM_ANDROID
#ifdef CONFIG_ARCH_MSM
	int retry = POWERUP_MAX_RETRY;
#endif /* CONFIG_ARCH_MSM */

	if (dhd_download_fw_on_driverload) {
		bcmerror = dhd_bus_start(dhdp);
	} else {
		if (flag == TRUE) { /* Turn off WLAN */
			/* Removing Power */
			DHD_ERROR(("%s: == Power OFF ==\n", __FUNCTION__));
			bus->dhd->up = FALSE;
			if (bus->intr) {
				dhdpcie_bus_intr_disable(bus);
				dhdpcie_free_irq(bus);
			}
#ifdef BCMPCIE_OOB_HOST_WAKE
			/* Clean up any pending host wake IRQ */
			dhd_bus_oob_intr_set(bus->dhd, FALSE);
			dhd_bus_oob_intr_unregister(bus->dhd);
#endif /* BCMPCIE_OOB_HOST_WAKE */
			if (bus->dhd->busstate != DHD_BUS_DOWN) {
				dhd_os_wd_timer(dhdp, 0);
				dhd_bus_stop(bus, TRUE);
			}
			dhd_prot_reset(dhdp);
			/* XXX Reset dhd_pub_t instance to initial status
			 * for built-in type driver
			 */
			dhd_clear(dhdp);
			dhd_bus_release_dongle(bus);
			dhdpcie_bus_free_resource(bus);
			bcmerror = dhdpcie_bus_disable_device(bus);
			if (bcmerror) {
				DHD_ERROR(("%s: dhdpcie_bus_disable_device: %d\n",
					__FUNCTION__, bcmerror));
				goto done;
			}
#ifdef CONFIG_ARCH_MSM
			bcmerror = dhdpcie_bus_clock_stop(bus);
			if (bcmerror) {
				DHD_ERROR(("%s: host clock stop failed: %d\n",
					__FUNCTION__, bcmerror));
				goto done;
			}
#endif /* CONFIG_ARCH_MSM */
			bus->dhd->busstate = DHD_BUS_DOWN;
			bus->dhd->dongle_reset = TRUE;
			DHD_ERROR(("%s:  WLAN OFF Done\n", __FUNCTION__));

		} else { /* Turn on WLAN */
			if (bus->dhd->busstate == DHD_BUS_DOWN) {
				/* Powering On */
				DHD_ERROR(("%s: == Power ON ==\n", __FUNCTION__));
#ifdef CONFIG_ARCH_MSM
				while (--retry) {
					bcmerror = dhdpcie_bus_clock_start(bus);
					if (!bcmerror) {
						DHD_ERROR(("%s: dhdpcie_bus_clock_start OK\n",
							__FUNCTION__));
						break;
					} else {
						OSL_SLEEP(10);
					}
				}

				if (bcmerror && !retry) {
					DHD_ERROR(("%s: host pcie clock enable failed: %d\n",
						__FUNCTION__, bcmerror));
					goto done;
				}
#endif /* CONFIG_ARCH_MSM */
				bcmerror = dhdpcie_bus_enable_device(bus);
				if (bcmerror) {
					DHD_ERROR(("%s: host configuration restore failed: %d\n",
						__FUNCTION__, bcmerror));
					goto done;
				}

				bcmerror = dhdpcie_bus_alloc_resource(bus);
				if (bcmerror) {
					DHD_ERROR(("%s: dhdpcie_bus_resource_alloc failed: %d\n",
						__FUNCTION__, bcmerror));
					goto done;
				}

				bcmerror = dhdpcie_bus_dongle_attach(bus);
				if (bcmerror) {
					DHD_ERROR(("%s: dhdpcie_bus_dongle_attach failed: %d\n",
						__FUNCTION__, bcmerror));
					goto done;
				}

				bcmerror = dhd_bus_request_irq(bus);
				if (bcmerror) {
					DHD_ERROR(("%s: dhd_bus_request_irq failed: %d\n",
						__FUNCTION__, bcmerror));
					goto done;
				}

				bus->dhd->dongle_reset = FALSE;

				bcmerror = dhd_bus_start(dhdp);
				if (bcmerror) {
					DHD_ERROR(("%s: dhd_bus_start: %d\n",
						__FUNCTION__, bcmerror));
					goto done;
				}

				bus->dhd->up = TRUE;
				DHD_ERROR(("%s: WLAN Power On Done\n", __FUNCTION__));
			} else {
				DHD_ERROR(("%s: what should we do here\n", __FUNCTION__));
				goto done;
			}
		}
	}

done:
	if (bcmerror) {
		bus->dhd->busstate = DHD_BUS_DOWN;
	}

#else /* OEM_ANDROID */

	if (flag == TRUE) {
		/* Removing Power */
		if (!dhdp->dongle_reset) {
			DHD_ERROR(("%s: == Power OFF ==\n", __FUNCTION__));
			dhd_os_sdlock(dhdp);
			dhd_os_wd_timer(dhdp, 0);

			dhd_bus_stop(bus, FALSE);
			dhd_flow_rings_deinit(dhdp);

			dhdpcie_bus_release_dongle(bus, bus->dhd->osh,
				bus->dhd->dongle_isolation, TRUE);
			bus->dhd->dongle_reset = TRUE;
			bus->dhd->up = FALSE;

			dhd_os_sdunlock(dhdp);

			DHD_ERROR(("%s:  WLAN OFF Done\n", __FUNCTION__));
		} else {
			DHD_ERROR(("%s: Dongle is already in RESET!\n", __FUNCTION__));
			bcmerror = BCME_DONGLE_DOWN;
		}
	} else {
		/* Powering On */
		DHD_ERROR(("%s: == Power ON ==\n", __FUNCTION__));

		if (bus->dhd->dongle_reset) {
			dhd_os_sdlock(dhdp); /* Turn on WLAN */

			if (dhdpcie_dongle_attach(bus)) {
				DHD_ERROR(("%s: dhdpcie_probe_attach failed\n", __FUNCTION__));
				dhd_os_sdunlock(dhdp);
				return BCME_DONGLE_DOWN;
			}
			bus->dhd->busstate = DHD_BUS_DOWN;

			DHD_INFO(("%s: About to download firmware\n", __FUNCTION__));
			if (dhd_bus_download_firmware(bus, bus->dhd->osh,
				bus->fw_path, bus->nv_path) == 0) {

				bcmerror = dhd_bus_init((dhd_pub_t *) bus->dhd, FALSE);
				if (bcmerror == BCME_OK) {
					bus->dhd->dongle_reset = FALSE;
					bus->dhd->up = TRUE;

					dhd_os_wd_timer(dhdp, dhd_watchdog_ms);

					DHD_ERROR(("%s: WLAN Power On Done\n", __FUNCTION__));
				} else {
					DHD_ERROR(("%s: dhd_bus_init FAILed\n", __FUNCTION__));
					dhd_bus_stop(bus, FALSE);
				}
			} else {
				DHD_ERROR(("%s: dhd_bus_download_firmware FAILed\n", __FUNCTION__));
				bcmerror = BCME_DONGLE_DOWN;
			}

			dhd_os_sdunlock(dhdp);
		} else {
			bcmerror = BCME_DONGLE_DOWN;
			DHD_ERROR(("%s called when dongle is not in reset\n", __FUNCTION__));
		}
	}
#endif /* OEM_ANDROID */

	return bcmerror;
}

static int
pcie2_mdioop(dhd_bus_t *bus, uint physmedia, uint regaddr, bool write, uint *val,
	bool slave_bypass)
{
	uint pcie_serdes_spinwait = 200, i = 0, mdio_ctrl;
	uint32 reg32;

	pcie2_mdiosetblock(bus, physmedia);

	/* enable mdio access to SERDES */
	mdio_ctrl = MDIOCTL2_DIVISOR_VAL;
	mdio_ctrl |= (regaddr << MDIOCTL2_REGADDR_SHF);

	if (slave_bypass)
		mdio_ctrl |= MDIOCTL2_SLAVE_BYPASS;

	if (!write)
		mdio_ctrl |= MDIOCTL2_READ;

	si_corereg(bus->sih, bus->sih->buscoreidx, PCIE2_MDIO_CONTROL, ~0, mdio_ctrl);

	if (write) {
		reg32 =  PCIE2_MDIO_WR_DATA;
		si_corereg(bus->sih, bus->sih->buscoreidx, PCIE2_MDIO_WR_DATA, ~0,
			*val | MDIODATA2_DONE);
	} else
		reg32 =  PCIE2_MDIO_RD_DATA;

	/* retry till the transaction is complete */
	while (i < pcie_serdes_spinwait) {
		uint done_val =  si_corereg(bus->sih, bus->sih->buscoreidx, reg32, 0, 0);
		if (!(done_val & MDIODATA2_DONE)) {
			if (!write) {
				*val = si_corereg(bus->sih, bus->sih->buscoreidx,
					PCIE2_MDIO_RD_DATA, 0, 0);
				*val = *val & MDIODATA2_MASK;
			}
			return 0;
		}
		OSL_DELAY(1000);
		i++;
	}

	return -1;
}

/** Get the fetched local copy of a pcie_ipc_t structure */
pcie_ipc_t *
dhdpcie_get_ipc(dhd_bus_t *bus)
{
	ASSERT(bus->pcie_ipc.flags != 0U);

	return &bus->pcie_ipc;
}

/** Get the fetched local copy of a pcie_ipc_rings_t structure */
pcie_ipc_rings_t *
dhdpcie_get_ipc_rings(dhd_bus_t *bus)
{
	ASSERT(bus->pcie_ipc.flags != 0U);
	ASSERT(bus->pcie_ipc_rings.ring_mem_daddr32 != 0U);

	return &bus->pcie_ipc_rings;
}

/** Use PCIe IPC HME User configuration to allocate a compatible DMA buf */
#define DHD_HME_COMPLIANCY_TRY_MAX 16 // Try to allocate 16 times

static int
dhdpcie_set_regval(dhd_bus_t *bus, uint32 regoffset, uint32 regval)
{
	si_corereg(bus->sih, bus->sih->buscoreidx, OFFSETOF(sbpcieregs_t, configaddr), ~0,
		regoffset);
	si_corereg(bus->sih, bus->sih->buscoreidx, OFFSETOF(sbpcieregs_t, configdata), ~0,
		regval);
	return BCME_OK;
} /* dhdpcie_set_regval */

static uint32
dhdpcie_get_regval(dhd_bus_t *bus, uint32 regoffset)
{
	uint32 regval = 0;

	si_corereg(bus->sih, bus->sih->buscoreidx, OFFSETOF(sbpcieregs_t, configaddr), ~0,
		regoffset);
	regval = si_corereg(bus->sih, bus->sih->buscoreidx,
		OFFSETOF(sbpcieregs_t, configdata), 0, 0);
	return regval;
} /* dhdpcie_get_regval */

/**
 * Allocate a DMA-able buffer compliant with PCIe IPC HME memory attributes.
 * Returns Number of HME pages allocated (can be 0) or BCME_NOMEM on error.
 */
static int
dhd_hme_buf_alloc_try(dhd_bus_t *bus,
	dhd_dma_buf_t *user_dma_buf, pcie_ipc_hme_user_t * pcie_ipc_hme_user)
{
	uint32          try, boundary_mask;
	const uint32    try_max = DHD_HME_COMPLIANCY_TRY_MAX;
	size_t          dma_buf_list_size;
	dhd_dma_buf_t * dma_buf_list; // temporary dma_buf table

	if (pcie_ipc_hme_user->pages == 0)
		return ((int)0);

	/* Size of the table of try_max number of dhd_dma_buf_t */
	dma_buf_list_size = try_max * sizeof(dhd_dma_buf_t);
	/* Temporary table of try_max number of dhd_dma_buf_t */
	dma_buf_list = MALLOCZ(bus->dhd->osh, dma_buf_list_size);
	if (dma_buf_list == (dhd_dma_buf_t*)NULL) {
		DHD_ERROR(("%s: PCIe IPC MEMORY FAILURE: "
			"HME User %s dma_buf_list allocation\n",
			__FUNCTION__, pcie_ipc_hme_user->name));
		return BCME_NOMEM;
	}

	ASSERT(user_dma_buf->va == (void*)NULL);
	memset(user_dma_buf, 0, sizeof(*user_dma_buf));

	boundary_mask = ((1 << pcie_ipc_hme_user->bound_bits) - 1);

	ASSERT(PCIE_IPC_HME_BYTES(pcie_ipc_hme_user->pages) <=
		(1 << pcie_ipc_hme_user->bound_bits));
	/* Repeat try_max times, to aligned allocate a dma_buf until a dma_buf
	 * is found that does not overlap the specified boundary.
	 */
	for (try = 0; try < try_max; ++try)
	{
		uint32 pa_base, pa_end;
		dhd_dma_buf_t * cur_dma_buf = (dma_buf_list + try);

		if (dhd_dma_buf_alloc(bus->dhd, cur_dma_buf, pcie_ipc_hme_user->name,
		        PCIE_IPC_HME_BYTES(pcie_ipc_hme_user->pages),
		        pcie_ipc_hme_user->align_bits))
		{
			DHD_ERROR(("%s: PCIe IPC MEMORY FAILURE: "
				"HME User %s pages %u align %u try %u\n",
				__FUNCTION__, pcie_ipc_hme_user->name,
				pcie_ipc_hme_user->pages,
				(1 << pcie_ipc_hme_user->align_bits), try));
			break;
		}

		// Check whether the aligned dma_buf also meets boundary requirement
		pa_base = (uint32)PHYSADDRLO(cur_dma_buf->pa) & boundary_mask;
		pa_end  = (pa_base + cur_dma_buf->_alloced) & boundary_mask;
		if (pa_base <= pa_end) // dma_buf is compliant with boundary
		{
			*user_dma_buf = *cur_dma_buf; // struct dhd_dma_buf copy into user
			/* Clear the 'try' array entry so that it will not be freed below */
			memset((void*)cur_dma_buf, 0, sizeof(*cur_dma_buf));
			break; // found, no further tries
		} else {
			DHD_PCIE_IPC(("\t\tTry-%d fail (boundary): Addr=0x%x len=0x%x mask=0x%x\n",
				try, (uint32)PHYSADDRLO(cur_dma_buf->pa),
				cur_dma_buf->_alloced, boundary_mask));
		}
	}   // try_max loop

	// Release all dma_buf(s) in failed tries
	for (try = 0; try < try_max; ++try) {
		dhd_dma_buf_t * cur_dma_buf = (dma_buf_list + try);
		if (cur_dma_buf->va != (void*)NULL) {
			dhd_dma_buf_free(bus->dhd, cur_dma_buf, "bus_try");
		}
	}

	// free the table of dhd_dma_buf
	MFREE(bus->dhd->osh, dma_buf_list, dma_buf_list_size);

	if (user_dma_buf->va != (void*)NULL) { // success case
		DHD_INFO((PCIE_IPC_HME_USER_FMT DHD_DMA_BUF_FMT "\n",
			PCIE_IPC_HME_USER_VAL(*pcie_ipc_hme_user),
			DHD_DMA_BUF_VAL(*user_dma_buf)));
		return (int)(pcie_ipc_hme_user->pages);
	} else { // failure case
		DHD_ERROR(("%s:" PCIE_IPC_HME_USER_FMT "\n",
			__FUNCTION__, PCIE_IPC_HME_USER_VAL(*pcie_ipc_hme_user)));
		return BCME_ERROR;
	}
}   /* dhd_hme_buf_alloc_try */

/** Use PCIe IPC HME User attributes to allocate a compliant HME DMA buffer */
int
dhd_hme_buf_alloc(dhd_bus_t *bus, dhd_dma_buf_t *user_dma_buf, uint32 hme_user_id)
{
	uint32 bytes;
	pcie_ipc_hme_t      * pcie_ipc_hme      = bus->pcie_ipc_hme;
	pcie_ipc_hme_user_t * pcie_ipc_hme_user = &pcie_ipc_hme->user[hme_user_id];

	/* Assert that a PCIe IPC HME user request is available in the bus layer */
	ASSERT(pcie_ipc_hme != (pcie_ipc_hme_t*)NULL);
	ASSERT(pcie_ipc_hme->users >= hme_user_id);
	ASSERT(pcie_ipc_hme_user->user_id == hme_user_id);

	if (pcie_ipc_hme_user->pages == 0)
		return BCME_OK; // HME user not configured

	bytes = PCIE_IPC_HME_BYTES(pcie_ipc_hme_user->pages);
	ASSERT(bytes < PCIE_IPC_HME_BYTES_MAX);

	if (hme_user_id == PCIE_IPC_HME_USER_HMOSWP) // PCIE_IPC_HYBRIDFW_HME_USER
	{
		dhd_dma_buf_t * hybridfw_dma_buf = &bus->hybridfw_dma_buf;

		ASSERT(hybridfw_dma_buf->va != (void*)NULL);
		ASSERT(hybridfw_dma_buf->len >= bytes);

		/* Transfer the bus layer HYBRIDFW DMA buffer - couled be empty */
		*user_dma_buf = *hybridfw_dma_buf; // struct dhd_dma_buf copy

		memset((void*)hybridfw_dma_buf, 0, sizeof(*hybridfw_dma_buf));

		return pcie_ipc_hme_user->pages;
	}

	/* Allocate HME compliant DMA-able buffer using HME User memory profile */
	return dhd_hme_buf_alloc_try(bus, user_dma_buf, pcie_ipc_hme_user);
}

/**
 * IOVAR handler of the DHD bus layer (in this case, the PCIe bus).
 *
 * @param actionid  e.g. IOV_SVAL(IOV_PCIEREG)
 * @param params    input buffer
 * @param plen      length in [bytes] of input buffer 'params'
 * @param arg       output buffer
 * @param len       length in [bytes] of output buffer 'arg'
 */
static int
dhdpcie_bus_doiovar(dhd_bus_t *bus, const bcm_iovar_t *vi, uint32 actionid, const char *name,
                void *params, int plen, void *arg, int len, int val_size)
{
	int bcmerror = BCME_OK;
	int32 int_val = 0;
	int32 int_val2 = 0;
	int32 int_val3 = 0;
	bool bool_val = 0;

	DHD_TRACE(("%s: Enter, action %d name %s params %p plen %d arg %p len %d val_size %d\n",
	           __FUNCTION__, actionid, name, params, plen, arg, len, val_size));

	if ((bcmerror = bcm_iovar_lencheck(vi, arg, len, IOV_ISSET(actionid))) != 0)
		goto exit;

	if (plen >= (int)sizeof(int_val))
		bcopy(params, &int_val, sizeof(int_val));

	if (plen >= (int)sizeof(int_val) * 2)
		bcopy((void*)((uintptr)params + sizeof(int_val)), &int_val2, sizeof(int_val2));

	if (plen >= (int)sizeof(int_val) * 3)
		bcopy((void*)((uintptr)params + 2 * sizeof(int_val)), &int_val3, sizeof(int_val3));

	bool_val = (int_val != 0) ? TRUE : FALSE;

	/* Check if dongle is in reset. If so, only allow DEVRESET iovars */
	if (bus->dhd->dongle_reset && !(actionid == IOV_SVAL(IOV_DEVRESET) ||
	                                actionid == IOV_GVAL(IOV_DEVRESET))) {
		bcmerror = BCME_NOTREADY;
		goto exit;
	}

	switch (actionid) {

	case IOV_SVAL(IOV_VARS):
		bcmerror = dhdpcie_downloadvars(bus, arg, len);
		break;

	case IOV_SVAL(IOV_PCIEREG):
		dhdpcie_set_regval(bus, (uint32) int_val, (uint32) int_val2);
		break;

	case IOV_GVAL(IOV_PCIEREG):
		int_val = (int32) (dhdpcie_get_regval(bus, (uint32) int_val));
		bcopy(&int_val, arg, sizeof(int_val));
		break;

	case IOV_SVAL(IOV_PCIECOREREG):
		si_corereg(bus->sih, bus->sih->buscoreidx, int_val, ~0, int_val2);
		break;
	case IOV_GVAL(IOV_BAR0_SECWIN_REG):
	{
		sdreg_t sdreg;
		uint32 addr, size;

		bcopy(params, &sdreg, sizeof(sdreg));

		addr = sdreg.offset;
		size = sdreg.func;

		if (si_backplane_access(bus->sih, addr, size, &int_val, TRUE) != BCME_OK) {
			DHD_ERROR(("Invalid size/addr combination \n"));
			bcmerror = BCME_ERROR;
			break;
		}
		bcopy(&int_val, arg, sizeof(int32));
		break;
	}

	case IOV_SVAL(IOV_BAR0_SECWIN_REG):
	{
		sdreg_t sdreg;
		uint32 addr, size;

		bcopy(params, &sdreg, sizeof(sdreg));

		addr = sdreg.offset;
		size = sdreg.func;
		if (si_backplane_access(bus->sih, addr, size, &sdreg.value, FALSE) != BCME_OK) {
			DHD_ERROR(("Invalid size/addr combination \n"));
			bcmerror = BCME_ERROR;
		}
		break;
	}

	case IOV_GVAL(IOV_SBREG):
	{
		sdreg_t sdreg;
		uint32 addr, size;

		bcopy(params, &sdreg, sizeof(sdreg));

		addr = sdreg.offset | SI_ENUM_BASE(bus->sih);
		size = sdreg.func;

		if (si_backplane_access(bus->sih, addr, size, &int_val, TRUE) != BCME_OK) {
			DHD_ERROR(("Invalid size/addr combination \n"));
			bcmerror = BCME_ERROR;
			break;
		}
		bcopy(&int_val, arg, size);
		break;
	}

	case IOV_SVAL(IOV_SBREG):
	{
		sdreg_t sdreg;
		uint32 addr, size;

		bcopy(params, &sdreg, sizeof(sdreg));

		addr = sdreg.offset | SI_ENUM_BASE(bus->sih);
		size = sdreg.func;
		if (si_backplane_access(bus->sih, addr, size, &sdreg.value, FALSE) != BCME_OK) {
			DHD_ERROR(("Invalid size/addr combination \n"));
			bcmerror = BCME_ERROR;
		}
		break;
	}

	case IOV_GVAL(IOV_PCIESERDESREG):
	{
		uint val;
		if (!PCIE_GEN2(bus->sih)) {
			DHD_ERROR(("supported only in pcie gen2\n"));
			bcmerror = BCME_ERROR;
			break;
		}

		if (!pcie2_mdioop(bus, int_val, int_val2, FALSE, &val, FALSE)) {
			bcopy(&val, arg, sizeof(int32));
		} else {
			DHD_ERROR(("pcie2_mdioop failed.\n"));
			bcmerror = BCME_ERROR;
		}
		break;
	}

	case IOV_SVAL(IOV_PCIESERDESREG):
		if (!PCIE_GEN2(bus->sih)) {
			DHD_ERROR(("supported only in pcie gen2\n"));
			bcmerror = BCME_ERROR;
			break;
		}
		if (pcie2_mdioop(bus, int_val, int_val2, TRUE, &int_val3, FALSE)) {
			DHD_ERROR(("pcie2_mdioop failed.\n"));
			bcmerror = BCME_ERROR;
		}
		break;
	case IOV_GVAL(IOV_PCIECOREREG):
		int_val = si_corereg(bus->sih, bus->sih->buscoreidx, int_val, 0, 0);
		bcopy(&int_val, arg, sizeof(int_val));
		break;

	case IOV_SVAL(IOV_PCIECFGREG):
		OSL_PCI_WRITE_CONFIG(bus->osh, int_val, 4, int_val2);
		break;

	case IOV_GVAL(IOV_PCIECFGREG):
		int_val = OSL_PCI_READ_CONFIG(bus->osh, int_val, 4);
		bcopy(&int_val, arg, sizeof(int_val));
		break;

	case IOV_SVAL(IOV_PCIE_LPBK):
		bcmerror = dhdpcie_bus_lpback_req(bus, int_val);
		break;

	case IOV_SVAL(IOV_PCIE_DMAXFER):
		bcmerror = dhdpcie_bus_dmaxfer_req(bus, int_val, int_val2, int_val3);
		break;

	case IOV_GVAL(IOV_PCIE_SUSPEND):
		int_val = (bus->dhd->busstate == DHD_BUS_SUSPENDING ||
		           bus->dhd->busstate == DHD_BUS_SUSPENDED) ? 1 : 0;
		bcopy(&int_val, arg, val_size);
		break;

	case IOV_SVAL(IOV_PCIE_SUSPEND):
		dhdpcie_bus_suspend(bus, bool_val);
		break;

	case IOV_GVAL(IOV_MEMSIZE):
		int_val = (int32)bus->ramsize;
		bcopy(&int_val, arg, val_size);
		break;
	case IOV_SVAL(IOV_MEMBYTES):
	case IOV_GVAL(IOV_MEMBYTES):
	{
		uint32 address;		/* absolute backplane address */
		uint size, dsize;
		uint8 *data;

		bool set = (actionid == IOV_SVAL(IOV_MEMBYTES));

		ASSERT(plen >= 2*sizeof(int));

		address = (uint32)int_val;
		bcopy((char *)params + sizeof(int_val), &int_val, sizeof(int_val));
		size = (uint)int_val;

		/* Do some validation */
		dsize = set ? plen - (2 * sizeof(int)) : len;
		if (dsize < size) {
			DHD_ERROR(("%s: error on %s membytes, addr 0x%08x size %d dsize %d\n",
			           __FUNCTION__, (set ? "set" : "get"), address, size, dsize));
			bcmerror = BCME_BADARG;
			break;
		}

		DHD_INFO(("%s: Request to %s %d bytes at address 0x%08x\n dsize %d ", __FUNCTION__,
		          (set ? "write" : "read"), size, address, dsize));

		/* check if CR4 */
		if (si_setcore(bus->sih, ARMCR4_CORE_ID, 0) ||
		    si_setcore(bus->sih, SYSMEM_CORE_ID, 0)) {
			/* if address is 0, store the reset instruction to be written in 0 */
			if (set && address == bus->dongle_ram_base) {
				bus->resetinstr = *(((uint32*)params) + 2);
			}
		} else {
		/* If we know about SOCRAM, check for a fit */
		if ((bus->orig_ramsize) &&
		    ((address > bus->orig_ramsize) || (address + size > bus->orig_ramsize)))
		{
			uint8 enable, protect, remap;
			si_socdevram(bus->sih, FALSE, &enable, &protect, &remap);
			if (!enable || protect) {
				DHD_ERROR(("%s: ramsize 0x%08x doesn't have %d bytes at 0x%08x\n",
					__FUNCTION__, bus->orig_ramsize, size, address));
				DHD_ERROR(("%s: socram enable %d, protect %d\n",
					__FUNCTION__, enable, protect));
				bcmerror = BCME_BADARG;
				break;
			}

			if (!REMAP_ENAB(bus) && (address >= SOCDEVRAM_ARM_ADDR)) {
				uint32 devramsize = si_socdevram_size(bus->sih);
				if ((address < SOCDEVRAM_ARM_ADDR) ||
					(address + size > (SOCDEVRAM_ARM_ADDR + devramsize))) {
					DHD_ERROR(("%s: bad address 0x%08x, size 0x%08x\n",
						__FUNCTION__, address, size));
					DHD_ERROR(("%s: socram range 0x%08x,size 0x%08x\n",
						__FUNCTION__, SOCDEVRAM_ARM_ADDR, devramsize));
					bcmerror = BCME_BADARG;
					break;
				}
				/* move it such that address is real now */
				address -= SOCDEVRAM_ARM_ADDR;
				address += SOCDEVRAM_BP_ADDR;
				DHD_INFO(("%s: Request to %s %d bytes @ Mapped address 0x%08x\n",
					__FUNCTION__, (set ? "write" : "read"), size, address));
			} else if (REMAP_ENAB(bus) && REMAP_ISADDR(bus, address) && remap) {
				/* Can not access remap region while devram remap bit is set
				 * ROM content would be returned in this case
				 */
				DHD_ERROR(("%s: Need to disable remap for address 0x%08x\n",
					__FUNCTION__, address));
				bcmerror = BCME_ERROR;
				break;
			}
		}
		}

		/* Generate the actual data pointer */
		data = set ? (uint8*)params + 2 * sizeof(int): (uint8*)arg;

		/* Call to do the transfer */
		dhdpcie_bus_membytes(bus, set, address, data, size);

		break;
	}

	case IOV_SVAL(IOV_HALT_DONGLE): /* halts ARM and PSM(s) for debug inspection */
	{
		int cpu_type = 0;
		uint32 zero_int = 0;

		bcmerror = BCME_ERROR;
		if (si_setcore(bus->sih, ARMCA7_CORE_ID, 0)) {
			cpu_type = ARMCA7_CORE_ID;
		} else if (si_setcore(bus->sih, ARMCR4_CORE_ID, 0)) {
			cpu_type = ARMCR4_CORE_ID;
		} else if (si_setcore(bus->sih, ARMCM3_CORE_ID, 0)) {
			cpu_type = ARMCM3_CORE_ID;
		}

		if (cpu_type != 0 && si_iscoreup(bus->sih) && int_val != 0) {
			switch (cpu_type) {
			case ARMCA7_CORE_ID:
				/* write ca7 reset vector with NULL to prevent firmware reboot */
				dhdpcie_bus_membytes(bus, PCIE_SYSMEM_WRITE, bus->dongle_ram_base,
					(uint8 *)&zero_int, sizeof(zero_int));
				si_core_reset(bus->sih, 0, 0); /* ARM reset and go */
				break;
			case ARMCR4_CORE_ID:
			case ARMCM3_CORE_ID:
				si_core_reset(bus->sih, SICF_CPUHALT, SICF_CPUHALT);
				break;
			}
			/* disable hardware watchdog */
			si_watchdog(bus->sih, 0);

			bcmerror = dhd_macdbg_halt_psms(bus->sih);
			bus->dhd->busstate = DHD_BUS_DOWN;
		}
		break;
	}

	/* Debug related. Dumps core registers or one of the dongle memory */
	case IOV_GVAL(IOV_DUMP_DONGLE):
	{
		dump_dongle_in_t ddi = *(dump_dongle_in_t *)params;
		dump_dongle_out_t *ddo = (dump_dongle_out_t *)arg;

		bcmerror = dhd_macdbg_dump_dongle(bus->sih, ddi, len, ddo);
		break;
	}

	/* Debug related. Returns a string with dongle capabilities */
	case IOV_GVAL(IOV_DNGL_CAPS):
	{
		memcpy(arg, bus->dhd->wlcore->fw_capabilities,
				MIN(strlen(bus->dhd->wlcore->fw_capabilities), len));
		((char*)arg)[len - 1] = '\0';
		break;
	}

#if defined(DEBUGGER) || defined(DHD_DSCOPE)
	case IOV_SVAL(IOV_GDB_SERVER):
		/* debugger_*() functions may sleep, so cannot hold spinlock */
		DHD_UNLOCK(bus->dhd);
		if (int_val > 0) {
			debugger_init((void *) bus, &bus_ops, int_val, SI_ENUM_BASE(bus->sih));
		} else {
			debugger_close();
		}
		DHD_LOCK(bus->dhd);
		break;
#endif /* DEBUGGER || DHD_DSCOPE */

#ifdef BCM_BUZZZ
	/* Dump dongle side buzzz trace to console */
	case IOV_GVAL(IOV_BUZZZ_DUMP):
		DHD_UNLOCK(bus->dhd);
		bcmerror = dhd_buzzz_dump_bus(bus);
		DHD_LOCK(bus->dhd);
		break;
	/* Dump dongle side buzzz trace to file */
	case IOV_GVAL(IOV_BUZZZ_FILE):
		DHD_UNLOCK(bus->dhd);
		bcmerror = dhd_buzzz_file_bus(bus);
		DHD_LOCK(bus->dhd);
		break;
#endif /* BCM_BUZZZ */

	case IOV_GVAL(IOV_CSIMON):
		bcmerror = dhd_csimon_dump(bus->dhd);
		break;

	case IOV_SVAL(IOV_SET_DOWNLOAD_STATE):
		bcmerror = dhdpcie_bus_download_state(bus, bool_val);
		break;

	case IOV_GVAL(IOV_RAMSIZE):
		int_val = (int32)bus->ramsize;
		bcopy(&int_val, arg, val_size);
		break;

	case IOV_SVAL(IOV_RAMSIZE):
		bus->ramsize = int_val;
		bus->orig_ramsize = int_val;
		break;

	case IOV_GVAL(IOV_RAMSTART):
		int_val = (int32)bus->dongle_ram_base;
		bcopy(&int_val, arg, val_size);
		break;

	case IOV_GVAL(IOV_CC_NVMSHADOW):
	{
		struct bcmstrbuf dump_b;

		bcm_binit(&dump_b, arg, len);
		bcmerror = dhdpcie_cc_nvmshadow(bus, &dump_b);
		break;
	}

	case IOV_GVAL(IOV_SLEEP_ALLOWED):
		int_val = (int32)bus->sleep_allowed;
		bcopy(&int_val, arg, val_size);
		break;

	case IOV_SVAL(IOV_SLEEP_ALLOWED):
		bus->sleep_allowed = bool_val;
		break;

	case IOV_GVAL(IOV_DONGLEISOLATION):
		int_val = bus->dhd->dongle_isolation;
		bcopy(&int_val, arg, val_size);
		break;

	case IOV_SVAL(IOV_DONGLEISOLATION):
		bus->dhd->dongle_isolation = bool_val;
		break;

	case IOV_GVAL(IOV_LTRSLEEPON_UNLOOAD):
		int_val = bus->ltrsleep_on_unload;
		bcopy(&int_val, arg, val_size);
		break;

	case IOV_SVAL(IOV_LTRSLEEPON_UNLOOAD):
		bus->ltrsleep_on_unload = bool_val;
		break;

	case IOV_GVAL(IOV_DUMP_RINGUPD_BLOCK):
	{
		struct bcmstrbuf dump_b;
		bcm_binit(&dump_b, arg, len);
		bcmerror = dhd_prot_ringupd_dump(bus->dhd, &dump_b);
		break;
	}
	case IOV_GVAL(IOV_DMA_RINGINDICES):
	{	int h2d_support, d2h_support;

		d2h_support = DMA_INDX_ENAB(bus->dhd->dma_d2h_ring_upd_support) ? 1 : 0;
		h2d_support = DMA_INDX_ENAB(bus->dhd->dma_h2d_ring_upd_support) ? 1 : 0;
		int_val = d2h_support | (h2d_support << 1);
		bcopy(&int_val, arg, sizeof(int_val));
		break;
	}
	case IOV_SVAL(IOV_DMA_RINGINDICES):
		/* Can change it only during initialization/FW download */
		if (bus->dhd->busstate == DHD_BUS_DOWN) {
			if ((int_val > 3) || (int_val < 0)) {
				DHD_ERROR(("Bad argument. Possible values: 0, 1, 2 & 3\n"));
				bcmerror = BCME_BADARG;
			} else {
				bus->dhd->dma_d2h_ring_upd_support = (int_val & 1) ? TRUE : FALSE;
				bus->dhd->dma_h2d_ring_upd_support = (int_val & 2) ? TRUE : FALSE;
			}
		} else {
			DHD_ERROR(("%s: Can change only when bus down (before FW download)\n",
				__FUNCTION__));
			bcmerror = BCME_NOTDOWN;
		}
		break;

	case IOV_GVAL(IOV_METADATA_DBG):
		int_val = dhd_prot_metadata_dbg_get(bus->dhd);
		bcopy(&int_val, arg, val_size);
		break;
	case IOV_SVAL(IOV_METADATA_DBG):
		dhd_prot_metadata_dbg_set(bus->dhd, (int_val != 0));
		break;

	case IOV_GVAL(IOV_RX_METADATALEN):
		int_val = dhd_prot_metadatalen_get(bus->dhd, TRUE);
		bcopy(&int_val, arg, val_size);
		break;

	case IOV_SVAL(IOV_RX_METADATALEN):
#if !defined(BCM_ROUTER_DHD)
		if (int_val > 64) {
			bcmerror = BCME_BUFTOOLONG;
			break;
		}
		dhd_prot_metadatalen_set(bus->dhd, int_val, TRUE);
#else
		bcmerror = BCME_UNSUPPORTED;
#endif /* BCM_ROUTER_DHD */
		break;

	case IOV_SVAL(IOV_TXP_THRESHOLD):
		dhd_prot_txp_threshold(bus->dhd, TRUE, int_val);
		break;

	case IOV_GVAL(IOV_TXP_THRESHOLD):
		int_val = dhd_prot_txp_threshold(bus->dhd, FALSE, int_val);
		bcopy(&int_val, arg, val_size);
		break;

	case IOV_SVAL(IOV_DB1_FOR_MB):
		if (int_val)
			bus->db1_for_mb = TRUE;
		else
			bus->db1_for_mb = FALSE;
		break;

	case IOV_GVAL(IOV_DB1_FOR_MB):
		if (bus->db1_for_mb)
			int_val = 1;
		else
			int_val = 0;
		bcopy(&int_val, arg, val_size);
		break;

	case IOV_GVAL(IOV_TX_METADATALEN):
		int_val = dhd_prot_metadatalen_get(bus->dhd, FALSE);
		bcopy(&int_val, arg, val_size);
		break;

	case IOV_SVAL(IOV_TX_METADATALEN):
#if !defined(BCM_ROUTER_DHD)
		if (int_val > 64) {
			bcmerror = BCME_BUFTOOLONG;
			break;
		}
		dhd_prot_metadatalen_set(bus->dhd, int_val, FALSE);
#else
		bcmerror = BCME_UNSUPPORTED;
#endif /* BCM_ROUTER_DHD */
		break;

	case IOV_SVAL(IOV_DEVRESET):
		dhd_bus_devreset(bus->dhd, (uint8)bool_val);
		break;
	case IOV_SVAL(IOV_FORCE_FW_TRAP):
		if (bus->dhd->busstate == DHD_BUS_DATA)
			dhdpcie_fw_trap(bus);
		else {
			DHD_ERROR(("%s: Bus is NOT up\n", __FUNCTION__));
			bcmerror = BCME_NOTUP;
		}
		break;
	case IOV_GVAL(IOV_FLOW_PRIO_MAP):
		int_val = bus->dhd->flow_prio_map_type;
		bcopy(&int_val, arg, val_size);
		break;

	case IOV_SVAL(IOV_FLOW_PRIO_MAP):
		/* Attempt IOVAR override ONLY IF dongle does not restrict */
		if (bus->pcie_ipc.hcap1 & PCIE_IPC_HCAP1_FLOWRING_TID) {
			int_val = (int_val == DHD_FLOW_PRIO_TID_MAP) ? BCME_OK : BCME_EPERM;
		} else {
			/* check whether setting may be applied, inform dongle and set */
			int_val = dhd_update_flow_prio_map(bus->dhd, (uint8)int_val);
		}
		bcopy(&int_val, arg, val_size);
		break;

	case IOV_GVAL(IOV_TXBOUND):
		int_val = (int32)dhd_txbound;
		bcopy(&int_val, arg, val_size);
		break;

	case IOV_SVAL(IOV_TXBOUND):
		dhd_txbound = (uint)int_val;
		break;

	case IOV_GVAL(IOV_RXBOUND):
		int_val = (int32)dhd_rxbound;
		bcopy(&int_val, arg, val_size);
		break;

	case IOV_SVAL(IOV_RXBOUND):
		dhd_rxbound = (uint)int_val;
		break;

	case IOV_GVAL(IOV_PCIEASPM): {
		uint8 clkreq = 0;
		uint32 aspm = 0;

		/* this command is to hide the details, but match the lcreg
		#define PCIE_CLKREQ_ENAB		0x100
		#define PCIE_ASPM_L1_ENAB		2
		#define PCIE_ASPM_L0s_ENAB		1
		*/

		clkreq = dhdpcie_clkreq(bus->dhd->osh, 0, 0);
		aspm = dhdpcie_lcreg(bus->dhd->osh, 0, 0);

		int_val = ((clkreq & 0x1) << 8) | (aspm & PCIE_ASPM_ENAB);
		bcopy(&int_val, arg, val_size);
		break;
	}

	case IOV_SVAL(IOV_PCIEASPM): {
		uint32 tmp;

		tmp = dhdpcie_lcreg(bus->dhd->osh, 0, 0);
		dhdpcie_lcreg(bus->dhd->osh, PCIE_ASPM_ENAB,
			(tmp & ~PCIE_ASPM_ENAB) | (int_val & PCIE_ASPM_ENAB));

		dhdpcie_clkreq(bus->dhd->osh, 1, ((int_val & 0x100) >> 8));
		break;
	}

	case IOV_SVAL(IOV_HANGREPORT):
		bus->dhd->hang_report = bool_val;
		DHD_ERROR(("%s: Set hang_report as %d\n",
			__FUNCTION__, bus->dhd->hang_report));
		break;

	case IOV_GVAL(IOV_HANGREPORT):
		int_val = (int32)bus->dhd->hang_report;
		bcopy(&int_val, arg, val_size);
		break;

	case IOV_GVAL(IOV_IDMA_ENABLE):
		int_val = bus->idma_enabled;
		bcopy(&int_val, arg, val_size);
		break;

	case IOV_SVAL(IOV_IDMA_ENABLE):
		bus->idma_enabled = (bool)int_val;
		break;

	case IOV_SVAL(IOV_HYBRIDFW):
		{
			char *fp;
			fp = dhd_os_open_image(bus->dhd, params);
			if (fp == NULL) {
				bcmerror = BCME_ERROR;
				break;
			}
			bcmerror = dhdpcie_hybridfw_download(bus, fp);
			dhd_os_close_image(bus->dhd, fp);
			break;
		}

	default:
		bcmerror = BCME_UNSUPPORTED;
		break;
	}

exit:
	return bcmerror;
} /* dhdpcie_bus_doiovar */

/** Transfers bytes from host to dongle using pio mode */
static int
dhdpcie_bus_lpback_req(struct  dhd_bus *bus, uint32 len)
{
	if (bus->dhd == NULL) {
		DHD_ERROR(("bus not inited\n"));
		return 0;
	}

	if (bus->dhd->prot == NULL) {
		DHD_ERROR(("prot is not inited\n"));
		return 0;
	}

	if (bus->dhd->busstate != DHD_BUS_DATA) {
		DHD_ERROR(("not in a readystate to LPBK  is not inited\n"));
		return 0;
	}

	dhdmsgbuf_lpbk_req(bus->dhd, len);

	return 0;
}

/**
 * Invoked by the OS or via iovar "pcie_suspend". DHD will disable interrupt poll and watchdog
 * timers during './dhd pcie_suspend 1' (bringing dongle to D3 state) and re-enable them on
 * './dhd pcie_suspend 0' (bringing dongle to D0 state). During D3 state, dhd/wl iovars will not
 * work.
 *
 * @param[in] state    1 for suspend, 0 for resume.
 *
 * Note: this function may sleep. Meanwhile other functions (eg, the dpc) may be executed.
 */
int
dhdpcie_bus_suspend(struct dhd_bus *bus, bool state)
{
	int timeleft;
	unsigned long flags;
	int rc = 0;
	int bcmerror = 0;

	if (bus->dhd == NULL) {
		DHD_ERROR(("bus not inited\n"));
		return BCME_ERROR;
	}
	if (bus->dhd->prot == NULL) {
		DHD_ERROR(("prot is not inited\n"));
		return BCME_ERROR;
	}
	DHD_GENERAL_LOCK(bus->dhd, flags);
	if (bus->dhd->busstate != DHD_BUS_DATA && bus->dhd->busstate != DHD_BUS_SUSPENDING &&
	    bus->dhd->busstate != DHD_BUS_SUSPENDED) {
		DHD_ERROR(("not in a readystate to LPBK  is not inited\n"));
		DHD_GENERAL_UNLOCK(bus->dhd, flags);
		return BCME_ERROR;
	}
	DHD_GENERAL_UNLOCK(bus->dhd, flags);
	if (bus->dhd->dongle_reset) {
		DHD_ERROR(("Dongle is in reset state.\n"));
		return -EIO;
	}

	if (bus->suspended == state) { /* Set to same state */
		DHD_ERROR(("Bus is already in this state.\n"));
		return BCME_OK;
	}

	if (state) {
#if defined(STB) && !defined(STBAP)
		dhdpcie_bus_set_wowl(bus, state);
#endif /* STB && STBAP */
		/* Suspend */
		DHD_ERROR(("%s: Entering suspend state\n", __FUNCTION__));
		bus->wait_for_d3_ack = 0;
		bus->suspended = TRUE;
		DHD_GENERAL_LOCK(bus->dhd, flags);
		bus->dhd->busstate = DHD_BUS_SUSPENDING;
#if defined(LINUX) || defined(linux)
		if (bus->dhd->tx_in_progress) {
			DHD_ERROR(("Tx Request is not ended\n"));
			bus->dhd->busstate = DHD_BUS_DATA;
			DHD_GENERAL_UNLOCK(bus->dhd, flags);
			bus->suspended = FALSE;
			return -EBUSY;
		}
#endif /* LINUX || linux */
		DHD_GENERAL_UNLOCK(bus->dhd, flags);
		DHD_OS_WAKE_LOCK_WAIVE(bus->dhd);
		dhd_os_set_ioctl_resp_timeout(DEFAULT_IOCTL_RESP_TIMEOUT);
		dhdpcie_send_mb_data(bus, PCIE_IPC_H2DMB_HOST_D3_INFORM);
		timeleft = dhd_os_d3ack_wait(bus->dhd, &bus->wait_for_d3_ack);
		dhd_os_set_ioctl_resp_timeout(IOCTL_RESP_TIMEOUT);
		DHD_OS_WAKE_LOCK_RESTORE(bus->dhd);
		if (bus->wait_for_d3_ack) {
			DHD_ERROR(("%s: Got D3 Ack \n", __FUNCTION__));

			/* Got D3 Ack. Suspend the bus */
			if (dhd_os_check_wakelock_all(bus->dhd)) {
				DHD_ERROR(("Suspend failed because of wakelock\n"));
				bus->suspended = FALSE;
				DHD_GENERAL_LOCK(bus->dhd, flags);
				bus->dhd->busstate = DHD_BUS_DATA;
				DHD_GENERAL_UNLOCK(bus->dhd, flags);
				rc = BCME_ERROR;
			} else {
				DHD_OS_WAKE_LOCK_WAIVE(bus->dhd);
				dhdpcie_send_mb_data(bus, PCIE_IPC_H2DMB_HOST_D0_INFORM_IN_USE);
				DHD_OS_WAKE_LOCK_RESTORE(bus->dhd);
				dhdpcie_bus_intr_disable(bus);
				bus->dhd->busstate = DHD_BUS_SUSPENDED;
				// writes D3hot to PMCSR, can cause isr of a neigboring device.
				rc = dhdpcie_pci_suspend_resume(bus, state);
				dhd_bus_set_device_wake(bus, FALSE);
				dhd_prot_reset(bus->dhd);
				bus->dhd->up = FALSE;
			}
			bus->dhd->d3ackcnt_timeout = 0;
		} else if (timeleft == 0) {
			bus->dhd->d3ackcnt_timeout++;
			DHD_ERROR(("%s: resumed on timeout for D3 ACK d3_inform_cnt %d \n",
				__FUNCTION__, bus->dhd->d3ackcnt_timeout));
#if defined(DHD_FW_COREDUMP) && defined(CUSTOMER_HW4)
			if (bus->dhd->memdump_enabled) {
				/* write core dump to file */
				dhdpcie_mem_dump(bus);
			}
#endif /* DHD_FW_COREDUMP && CUSTOMER_HW4 */
			bus->suspended = FALSE;
			DHD_GENERAL_LOCK(bus->dhd, flags);
			bus->dhd->busstate = DHD_BUS_DATA;
			DHD_GENERAL_UNLOCK(bus->dhd, flags);
#ifdef OEM_ANDROID
			if (bus->dhd->d3ackcnt_timeout >= MAX_CNTL_D3ACK_TIMEOUT) {
				DHD_ERROR(("%s: Event HANG send up "
					"due to PCIe linkdown\n", __FUNCTION__));
#ifdef SUPPORT_LINKDOWN_RECOVERY
#ifdef CONFIG_ARCH_MSM
				bus->islinkdown = 1;
#endif /* CONFIG_ARCH_MSM */
#endif /* SUPPORT_LINKDOWN_RECOVERY */
				dhd_os_check_hang(bus->dhd, 0, -ETIMEDOUT);
			}
#endif /* OEM_ANDROID */
			rc = -ETIMEDOUT;

		}
		bus->wait_for_d3_ack = 1;
	} else {
		/* Resume */
#if defined(linux) && defined(OEM_ANDROID)
#if defined(BCMPCIE_OOB_HOST_WAKE)
		DHD_OS_OOB_IRQ_WAKE_UNLOCK(bus->dhd);
#endif /* BCMPCIE_OOB_HOST_WAKE */
#endif /* linux && OEM_ANDROID */
		rc = dhdpcie_pci_suspend_resume(bus, state);
		if (bus->dhd->busstate == DHD_BUS_SUSPENDING ||
		    bus->dhd->busstate == DHD_BUS_SUSPENDED) {
			DHD_OS_WAKE_LOCK_WAIVE(bus->dhd);
			dhdpcie_send_mb_data(bus, PCIE_IPC_H2DMB_HOST_D0_INFORM);
			DHD_OS_WAKE_LOCK_RESTORE(bus->dhd);
			dhd_bus_set_device_wake(bus, TRUE);
		}
		bcmerror = dhd_bus_start(bus->dhd);
		if (bcmerror) {
		      DHD_ERROR(("%s: dhd_bus_start: %d\n", __FUNCTION__, bcmerror));
		}
		bus->dhd->up = TRUE;
		bus->suspended = FALSE;
		DHD_GENERAL_LOCK(bus->dhd, flags);
		bus->dhd->busstate = DHD_BUS_DATA;
		DHD_GENERAL_UNLOCK(bus->dhd, flags);
		dhdpcie_bus_intr_enable(bus);
#if defined(STB) && !defined(STBAP)
		dhdpcie_bus_set_wowl(bus, state);
#endif /* STB && STBAP */
	}

	return rc;
} /* dhdpcie_bus_suspend */

/** Transfers bytes from host to dongle and to host again using DMA */
static int
dhdpcie_bus_dmaxfer_req(struct  dhd_bus *bus, uint32 len, uint32 srcdelay, uint32 destdelay)
{
	if (bus->dhd == NULL) {
		DHD_ERROR(("bus not inited\n"));
		return BCME_ERROR;
	}

	if (bus->dhd->prot == NULL) {
		DHD_ERROR(("prot is not inited\n"));
		return BCME_ERROR;
	}

	if (bus->dhd->busstate != DHD_BUS_DATA) {
		DHD_ERROR(("not in a readystate to LPBK  is not inited\n"));
		return BCME_ERROR;
	}

	if (len < 5 || len > 4194296) {
		DHD_ERROR(("len is too small or too large\n"));
		return BCME_ERROR;
	}

	return dhdmsgbuf_dmaxfer_req(bus->dhd, len, srcdelay, destdelay);
}

static int
dhdpcie_bus_download_state(dhd_bus_t *bus, bool enter)
{
	int bcmerror = 0;
	volatile uint32 *cr4_regs;

	if (!bus->sih) {
		DHD_ERROR(("%s: NULL sih!!\n", __FUNCTION__));
		return BCME_ERROR;
	}
	/* To enter download state, disable ARM and reset SOCRAM.
	 * To exit download state, simply reset ARM (default is RAM boot).
	 */
	if (enter) {
		dhdpcie_bus_switch_ht_alp(bus, CCS_BP_ON_ALP);

		/* some chips (e.g. 43602) have two ARM cores, the CR4 is receives the firmware. */
		cr4_regs = si_setcore(bus->sih, ARMCR4_CORE_ID, 0);

		if (cr4_regs == NULL && !(si_setcore(bus->sih, ARM7S_CORE_ID, 0)) &&
		    !(si_setcore(bus->sih, ARMCM3_CORE_ID, 0)) &&
		    !(si_setcore(bus->sih, ARMCA7_CORE_ID, 0))) {
			DHD_ERROR(("%s: Failed to find ARM core!\n", __FUNCTION__));
			bcmerror = BCME_ERROR;
			goto fail;
		}

		if (si_setcore(bus->sih, ARMCA7_CORE_ID, 0)) {
			/* Halt ARM & remove reset */
			si_core_reset(bus->sih, SICF_CPUHALT, SICF_CPUHALT);
			if (!(si_setcore(bus->sih, SYSMEM_CORE_ID, 0))) {
				DHD_ERROR(("%s: Failed to find SYSMEM core!\n", __FUNCTION__));
				bcmerror = BCME_ERROR;
				goto fail;
			}
			si_core_reset(bus->sih, 0, 0);
			/* reset last 4 bytes of RAM address. to be used for shared area */
			dhdpcie_bus_clear_pcie_ipc_addr(bus);
		} else if (cr4_regs == NULL) { /* no CR4 present on chip */
			si_core_disable(bus->sih, 0);

			if (!(si_setcore(bus->sih, SOCRAM_CORE_ID, 0))) {
				DHD_ERROR(("%s: Failed to find SOCRAM core!\n", __FUNCTION__));
				bcmerror = BCME_ERROR;
				goto fail;
			}

			si_core_reset(bus->sih, 0, 0);

			/* Clear the top bit of memory */
			if (bus->ramsize) {
				uint32 zeros = 0;
				dhdpcie_bus_membytes(bus, PCIE_SYSMEM_WRITE, bus->ramsize - 4,
					(uint8 *)&zeros, sizeof(zeros));
			}
		} else {
			/* For CR4,
			 * Halt ARM
			 * Remove ARM reset
			 * Read RAM base address [0x18_0000]
			 * [next] Download firmware
			 * [done at else] Populate the reset vector
			 * [done at else] Remove ARM halt
			*/
			/* Halt ARM & remove reset */
			si_core_reset(bus->sih, SICF_CPUHALT, SICF_CPUHALT);
			if (BCM43602_CHIP(bus->sih->chip)) {
				W_REG(bus->pcie_mb_intr_osh, cr4_regs + ARMCR4REG_BANKIDX, 5);
				W_REG(bus->pcie_mb_intr_osh, cr4_regs + ARMCR4REG_BANKPDA, 0);
				W_REG(bus->pcie_mb_intr_osh, cr4_regs + ARMCR4REG_BANKIDX, 7);
				W_REG(bus->pcie_mb_intr_osh, cr4_regs + ARMCR4REG_BANKPDA, 0);
			}
			/* reset last 4 bytes of RAM address. to be used for shared area */
			dhdpcie_bus_clear_pcie_ipc_addr(bus);
		}
	} else {
		if (si_setcore(bus->sih, ARMCA7_CORE_ID, 0)) {
			/* write vars */
			if ((bcmerror = dhdpcie_bus_write_vars(bus))) {
				DHD_ERROR(("%s: could not write vars to RAM\n", __FUNCTION__));
				goto fail;
			}

			if (bus->hybridfw_dma_buf.va) {
				/* Share the location of the host memory
				 * location where pageable FW binary is located.
				 */
				host_location_info_t host_location;
				host_location.tlv_signature =
					htol32(BCM_HOST_PAGE_LOCATION_SIGNATURE);
				host_location.tlv_size = htol32(sizeof(host_location)) -
					sizeof(host_location.tlv_size) -
					sizeof(host_location.tlv_signature);
				host_location.binary_size = htol32(bus->hybridfw_dma_buf.len);
				host_location.addr_hi = PHYSADDRHI(bus->hybridfw_dma_buf.pa);
				host_location.addr_lo = PHYSADDRLO(bus->hybridfw_dma_buf.pa);
				bus->ramtop_addr -= sizeof(host_location);
				dhdpcie_bus_membytes(bus, PCIE_SYSMEM_WRITE, bus->ramtop_addr,
					(uint8*)&host_location, sizeof(host_location));
				DHD_ERROR(("%s: HYBRIDFW Host page location info:"
					" %08x-%08x Len:0x%08x,%u!\n", __FUNCTION__,
					host_location.addr_hi, host_location.addr_lo,
					host_location.binary_size, host_location.binary_size));
			}

			/* switch back to arm core again */
			if (!(si_setcore(bus->sih, ARMCA7_CORE_ID, 0))) {
				DHD_ERROR(("%s: Failed to find ARM CA7 core!\n", __FUNCTION__));
				bcmerror = BCME_ERROR;
				goto fail;
			}
#ifndef VELOCE_DL_BACKDOOR
			/* write address 0 with reset instruction */
			dhdpcie_bus_membytes(bus, PCIE_SYSMEM_WRITE, 0,
				(uint8 *)&bus->resetinstr, sizeof(bus->resetinstr));
#endif
			/* now remove reset and halt and continue to run CA7 */
		} else if (!si_setcore(bus->sih, ARMCR4_CORE_ID, 0)) {
			if (!(si_setcore(bus->sih, SOCRAM_CORE_ID, 0))) {
				DHD_ERROR(("%s: Failed to find SOCRAM core!\n", __FUNCTION__));
				bcmerror = BCME_ERROR;
				goto fail;
			}

			if (!si_iscoreup(bus->sih)) {
				DHD_ERROR(("%s: SOCRAM core is down after reset?\n", __FUNCTION__));
				bcmerror = BCME_ERROR;
				goto fail;
			}

			/* Enable remap before ARM reset but after vars.
			 * No backplane access in remap mode
			 */
			if (!si_setcore(bus->sih, PCMCIA_CORE_ID, 0) &&
			    !si_setcore(bus->sih, SDIOD_CORE_ID, 0)) {
				DHD_ERROR(("%s: Can't change back to SDIO core?\n", __FUNCTION__));
				bcmerror = BCME_ERROR;
				goto fail;
			}

			if (!(si_setcore(bus->sih, ARM7S_CORE_ID, 0)) &&
			    !(si_setcore(bus->sih, ARMCM3_CORE_ID, 0))) {
				DHD_ERROR(("%s: Failed to find ARM core!\n", __FUNCTION__));
				bcmerror = BCME_ERROR;
				goto fail;
			}
		} else {
			if (BCM43602_CHIP(bus->sih->chip)) {
				/* Firmware crashes on SOCSRAM access when core is in reset */
				if (!(si_setcore(bus->sih, SOCRAM_CORE_ID, 0))) {
					DHD_ERROR(("%s: Failed to find SOCRAM core!\n",
						__FUNCTION__));
					bcmerror = BCME_ERROR;
					goto fail;
				}
				si_core_reset(bus->sih, 0, 0);
				si_setcore(bus->sih, ARMCR4_CORE_ID, 0);
			}

			/* write vars */
			if ((bcmerror = dhdpcie_bus_write_vars(bus))) {
				DHD_ERROR(("%s: could not write vars to RAM\n", __FUNCTION__));
				goto fail;
			}

			/* switch back to arm core again */
			if (!(si_setcore(bus->sih, ARMCR4_CORE_ID, 0))) {
				DHD_ERROR(("%s: Failed to find ARM CR4 core!\n", __FUNCTION__));
				bcmerror = BCME_ERROR;
				goto fail;
			}

			/* write address 0 with reset instruction */
			dhdpcie_bus_membytes(bus, PCIE_SYSMEM_WRITE, 0,
				(uint8 *)&bus->resetinstr, sizeof(bus->resetinstr));
			{
				uint32 tmp;
				dhdpcie_bus_membytes(bus, PCIE_SYSMEM_READ, 0,
					(uint8 *)&tmp, sizeof(tmp));
				if (tmp != bus->resetinstr) {
					DHD_ERROR(("%s: Failed to write 0x%08x to addr 0\n",
					          __FUNCTION__, bus->resetinstr));
					DHD_ERROR(("%s: contents of addr 0 is 0x%08x\n",
					          __FUNCTION__, tmp));
					bcmerror = BCME_ERROR;
					goto fail;
				}
			}

			/* now remove reset and halt and continue to run CR4 */
		}

		si_core_reset(bus->sih, 0, 0);

		/* Allow HT Clock now that the ARM is running. */
		dhdpcie_bus_switch_ht_alp(bus, CCS_BP_ON_HT);

		bus->dhd->busstate = DHD_BUS_LOAD;
	}

fail:
	/* Always return to PCIE core */
	si_setcore(bus->sih, PCIE2_CORE_ID, 0);

	return bcmerror;
} /* dhdpcie_bus_download_state */

static int
dhdpcie_bus_write_vars(dhd_bus_t *bus)
{
	uint32 varsize, phys_size;
	uint32 varaddr;
	uint8 *vbuffer;
	uint32 varsizew;
#ifdef DHD_DEBUG
	uint8 *nvram_ularray;
#endif /* DHD_DEBUG */

	/* Even if there are no vars are to be written, we still need to set the ramsize. */
	varsize = bus->varsz ? ROUNDUP(bus->varsz, 4) : 0;
	varaddr = (bus->ramsize - 4) - varsize;

	varaddr += bus->dongle_ram_base;
	bus->ramtop_addr = varaddr;

	if (bus->vars) {

		vbuffer = (uint8 *)MALLOC(bus->dhd->osh, varsize);
		if (!vbuffer)
			return BCME_NOMEM;

		bzero(vbuffer, varsize);
		bcopy(bus->vars, vbuffer, bus->varsz);
		/* Write the vars list */
		dhdpcie_bus_membytes(bus, PCIE_SYSMEM_WRITE, varaddr, vbuffer, varsize);

		/* Implement read back and verify later */
#ifdef DHD_DEBUG
		/* Verify NVRAM bytes */
		DHD_INFO(("Compare NVRAM dl & ul; varsize=%d\n", varsize));
		nvram_ularray = (uint8*)MALLOC(bus->dhd->osh, varsize);
		if (!nvram_ularray) {
			DHD_ERROR(("%s: Upload NVRAM ilarray malloc failure\n",
				__FUNCTION__));
			return BCME_NOMEM;
		}

		/* Upload image to verify downloaded contents. */
		memset(nvram_ularray, 0xaa, varsize);

		/* Read the vars list to temp buffer for comparison */
		dhdpcie_bus_membytes(bus, PCIE_SYSMEM_READ, varaddr,
			nvram_ularray, varsize);

		/* Compare the org NVRAM with the one read from RAM */
		if (memcmp(vbuffer, nvram_ularray, varsize)) {
			DHD_ERROR(("%s: Downloaded NVRAM image is corrupted.\n",
				__FUNCTION__));
		} else {
			DHD_ERROR(("%s: Download, Upload and compare of NVRAM succeeded.\n",
				__FUNCTION__));
		}

		MFREE(bus->dhd->osh, nvram_ularray, varsize);
#endif /* DHD_DEBUG */

		MFREE(bus->dhd->osh, vbuffer, varsize);
	}

	phys_size = REMAP_ENAB(bus) ? bus->ramsize : bus->orig_ramsize;

	phys_size += bus->dongle_ram_base;

	/* adjust to the user specified RAM */
	DHD_INFO(("Physical memory size: %d, usable memory size: %d\n",
		phys_size, bus->ramsize));
	DHD_INFO(("Vars are at %d, orig varsize is %d\n",
		varaddr, varsize));
	varsize = ((phys_size - 4) - varaddr);

	/*
	 * Determine the length token:
	 * Varsize, converted to words, in lower 16-bits, checksum in upper 16-bits.
	 */
	varsizew = varsize / 4;
	varsizew = (~varsizew << 16) | (varsizew & 0x0000FFFF);
	bus->nvram_csm = varsizew;
	varsizew = htol32(varsizew);

	DHD_INFO(("New varsize is %d, length token=0x%08x\n", varsize, varsizew));

	/* Write the length token to the last word */
	dhdpcie_bus_membytes(bus, PCIE_SYSMEM_WRITE, (phys_size - 4),
		(uint8*)&varsizew, sizeof(varsizew));

	return BCME_OK;
} /* dhdpcie_bus_write_vars */

/**
 * @param[in] state           1 = suspend
 */
int
dhdpcie_downloadvars(dhd_bus_t *bus, void *arg, int len)
{
	int bcmerror = BCME_OK;
#ifdef KEEP_JP_REGREV
	char *tmpbuf;
	uint tmpidx;
#endif /* KEEP_JP_REGREV */

	DHD_TRACE(("%s: Enter\n", __FUNCTION__));

	/* Basic sanity checks */
	if (bus->dhd->up) {
		bcmerror = BCME_NOTDOWN;
		goto err;
	}
	if (!len) {
		bcmerror = BCME_BUFTOOSHORT;
		goto err;
	}

	/* Free the old ones and replace with passed variables */
	if (bus->vars)
		MFREE(bus->dhd->osh, bus->vars, bus->varsz);

	bus->vars = MALLOC(bus->dhd->osh, len);
	bus->varsz = bus->vars ? len : 0;
	if (bus->vars == NULL) {
		bcmerror = BCME_NOMEM;
		goto err;
	}

	/* Copy the passed variables, which should include the terminating double-null */
	bcopy(arg, bus->vars, bus->varsz);

#ifdef KEEP_JP_REGREV
	if (bus->vars != NULL && bus->varsz > 0) {
		char *pos = NULL;
		tmpbuf = MALLOCZ(bus->dhd->osh, bus->varsz + 1);
		if (tmpbuf == NULL) {
			goto err;
		}
		memcpy(tmpbuf, bus->vars, bus->varsz);
		for (tmpidx = 0; tmpidx < bus->varsz; tmpidx++) {
			if (tmpbuf[tmpidx] == 0) {
				tmpbuf[tmpidx] = '\n';
			}
		}
		bus->dhd->vars_ccode[0] = 0;
		bus->dhd->vars_regrev = 0;
		if ((pos = strstr(tmpbuf, "ccode"))) {
			sscanf(pos, "ccode=%s\n", bus->dhd->vars_ccode);
		}
		if ((pos = strstr(tmpbuf, "regrev"))) {
			sscanf(pos, "regrev=%u\n", &(bus->dhd->vars_regrev));
		}
		MFREE(bus->dhd->osh, tmpbuf, bus->varsz + 1);
	}
#endif /* KEEP_JP_REGREV */

err:
	return bcmerror;
}

/* loop through the capability list and see if the pcie capabilty exists */
uint8
dhdpcie_find_pci_capability(osl_t *osh, uint8 req_cap_id)
{
	uint8 cap_id;
	uint8 cap_ptr = 0;
	uint8 byte_val;

	/* check for Header type 0 */
	byte_val = read_pci_cfg_byte(PCI_CFG_HDR);
	if ((byte_val & 0x7f) != PCI_HEADER_NORMAL) {
		DHD_ERROR(("%s : PCI config header not normal.\n", __FUNCTION__));
		goto end;
	}

	/* check if the capability pointer field exists */
	byte_val = read_pci_cfg_byte(PCI_CFG_STAT);
	if (!(byte_val & PCI_CAPPTR_PRESENT)) {
		DHD_ERROR(("%s : PCI CAP pointer not present.\n", __FUNCTION__));
		goto end;
	}

	cap_ptr = read_pci_cfg_byte(PCI_CFG_CAPPTR);
	/* check if the capability pointer is 0x00 */
	if (cap_ptr == 0x00) {
		DHD_ERROR(("%s : PCI CAP pointer is 0x00.\n", __FUNCTION__));
		goto end;
	}

	/* loop thr'u the capability list and see if the pcie capabilty exists */

	cap_id = read_pci_cfg_byte(cap_ptr);

	while (cap_id != req_cap_id) {
		cap_ptr = read_pci_cfg_byte((cap_ptr + 1));
		if (cap_ptr == 0x00) break;
		cap_id = read_pci_cfg_byte(cap_ptr);
	}

end:
	return cap_ptr;
}

void
dhdpcie_pme_active(osl_t *osh, bool enable)
{
	uint8 cap_ptr;
	uint32 pme_csr;

	cap_ptr = dhdpcie_find_pci_capability(osh, PCI_CAP_POWERMGMTCAP_ID);

	if (!cap_ptr) {
		DHD_ERROR(("%s : Power Management Capability not present\n", __FUNCTION__));
		return;
	}

	pme_csr = OSL_PCI_READ_CONFIG(osh, cap_ptr + PME_CSR_OFFSET, sizeof(uint32));
	DHD_ERROR(("%s : pme_sts_ctrl 0x%x\n", __FUNCTION__, pme_csr));

	pme_csr |= PME_CSR_PME_STAT;
	if (enable) {
		pme_csr |= PME_CSR_PME_EN;
	} else {
		pme_csr &= ~PME_CSR_PME_EN;
	}

	OSL_PCI_WRITE_CONFIG(osh, cap_ptr + PME_CSR_OFFSET, sizeof(uint32), pme_csr);
}

bool
dhdpcie_pme_cap(osl_t *osh)
{
	uint8 cap_ptr;
	uint32 pme_cap;

	cap_ptr = dhdpcie_find_pci_capability(osh, PCI_CAP_POWERMGMTCAP_ID);

	if (!cap_ptr) {
		DHD_ERROR(("%s : Power Management Capability not present\n", __FUNCTION__));
		return FALSE;
	}

	pme_cap = OSL_PCI_READ_CONFIG(osh, cap_ptr, sizeof(uint32));

	DHD_ERROR(("%s : pme_cap 0x%x\n", __FUNCTION__, pme_cap));

	return ((pme_cap & PME_CAP_PM_STATES) != 0);
}

uint32
dhdpcie_lcreg(osl_t *osh, uint32 mask, uint32 val)
{
	uint8	pcie_cap;
	uint8	lcreg_offset;	/* PCIE capability LCreg offset in the config space */
	uint32	reg_val;

	pcie_cap = dhdpcie_find_pci_capability(osh, PCI_CAP_PCIECAP_ID);

	if (!pcie_cap) {
		DHD_ERROR(("%s : PCIe Capability not present\n", __FUNCTION__));
		return 0;
	}

	lcreg_offset = pcie_cap + PCIE_CAP_LINKCTRL_OFFSET;

	/* set operation */
	if (mask) {
		/* read */
		reg_val = OSL_PCI_READ_CONFIG(osh, lcreg_offset, sizeof(uint32));

		/* modify */
		reg_val &= ~mask;
		reg_val |= (mask & val);

		/* write */
		OSL_PCI_WRITE_CONFIG(osh, lcreg_offset, sizeof(uint32), reg_val);
	}

	return OSL_PCI_READ_CONFIG(osh, lcreg_offset, sizeof(uint32));
}

/* set min res mask to highest value, preventing sleep */
void
dhdpcie_set_pmu_min_res_mask(struct dhd_bus *bus, uint min_res_mask)
{
	si_pmu_set_min_res_mask(bus->sih, bus->osh, min_res_mask);
}

uint8
dhdpcie_clkreq(osl_t *osh, uint32 mask, uint32 val)
{
	uint8	pcie_cap;
	uint32	reg_val;
	uint8	lcreg_offset;	/* PCIE capability LCreg offset in the config space */

	pcie_cap = dhdpcie_find_pci_capability(osh, PCI_CAP_PCIECAP_ID);

	if (!pcie_cap) {
		DHD_ERROR(("%s : PCIe Capability not present\n", __FUNCTION__));
		return 0;
	}

	lcreg_offset = pcie_cap + PCIE_CAP_LINKCTRL_OFFSET;

	reg_val = OSL_PCI_READ_CONFIG(osh, lcreg_offset, sizeof(uint32));
	/* set operation */
	if (mask) {
		if (val)
			reg_val |= PCIE_CLKREQ_ENAB;
		else
			reg_val &= ~PCIE_CLKREQ_ENAB;
		OSL_PCI_WRITE_CONFIG(osh, lcreg_offset, sizeof(uint32), reg_val);
		reg_val = OSL_PCI_READ_CONFIG(osh, lcreg_offset, sizeof(uint32));
	}
	if (reg_val & PCIE_CLKREQ_ENAB)
		return 1;
	else
		return 0;
}

/** Add bus dump output to a buffer */
void
dhd_bus_dump(dhd_pub_t *dhdp, struct bcmstrbuf *strbuf)
{
	uint16 flowid;
	int ix = 0;
	flow_ring_node_t *flow_ring_node;
	flow_info_t *flow_info;
	char eabuf[ETHER_ADDR_STR_LEN];

	if (dhdp->busstate != DHD_BUS_DATA)
		return;

	dhd_prot_print_info(dhdp, strbuf);
	bcm_bprintf(strbuf, "dhd cumm_ctr %d\n", DHD_CUMM_CTR_READ(&dhdp->cumm_ctr));
	bcm_bprintf(strbuf, "%s %4s %4s %2s %4s %17s %4s %4s %6s %10s ",
		"Num:", "Offl", "Flow", "If", "Prio", ":Dest_MacAddress:", "Qlen",
		"CLen", "L2CLen", "Overflows");
	if (PCIE_HBQD(dhdp))
		bcm_bprintf(strbuf, "%9s %19s ", "RD", "QD,  WR");
	else
		bcm_bprintf(strbuf, "%9s %9s ", "RD", "WR");
	bcm_bprintf(strbuf, "%5s %6s %5s \n", "Acked", "tossed", "noack");

	for (flowid = 0; flowid < dhdp->num_flow_rings; flowid++) {
		flow_ring_node = DHD_FLOW_RING(dhdp, flowid);
		if (!flow_ring_node->active)
			continue;

		flow_info = &flow_ring_node->flow_info;
		bcm_bprintf(strbuf,
			"%3d. %4d %4d %2d %4d %17s %4d %4d %6d %10u ", ix++,
#if defined(BCM_DHD_RUNNER)
			DHD_FLOWRING_RNR_OFFL(flow_ring_node),
#else /* !BCM_DHD_RUNNER */
			0,
#endif /* !BCM_DHD_RUNNER */
			flow_ring_node->flowid, flow_info->ifindex, flow_info->tid,
			bcm_ether_ntoa((struct ether_addr *)&flow_info->da, eabuf),
			DHD_FLOW_QUEUE_LEN(&flow_ring_node->queue),
			DHD_CUMM_CTR_READ(DHD_FLOW_QUEUE_CLEN_PTR(&flow_ring_node->queue)),
			DHD_CUMM_CTR_READ(DHD_FLOW_QUEUE_L2CLEN_PTR(&flow_ring_node->queue)),
			DHD_FLOW_QUEUE_FAILURES(&flow_ring_node->queue));
		if (PCIE_HBQD(dhdp))
			dhd_prot_print_flow_ring(dhdp, flow_ring_node->prot_info, strbuf,
				"%4d:%4d %4d,%4d:%4d,%4d ");
		else
			dhd_prot_print_flow_ring(dhdp, flow_ring_node->prot_info, strbuf,
				"%4d:%4d %4d:%4d ");
#ifdef BCMDBG
		if (!dhdp->d2h_sync_mode) {
			flow_info_t *flow = &flow_ring_node->flow_info;
			bcm_bprintf(strbuf, "%5d %6d %5d\n",
			flow->tx_status[WLFC_CTL_PKTFLAG_DISCARD],
			flow->tx_status[WLFC_CTL_PKTFLAG_TOSSED_BYWLC],
			flow->tx_status[WLFC_CTL_PKTFLAG_DISCARD_NOACK]);
		} else {
			bcm_bprintf(strbuf,
				"%5s %6s %5s\n", "NA", "NA", "NA");
		}
#else
		bcm_bprintf(strbuf,
			"%5s %6s %5s\n", "NA", "NA", "NA");
#endif
	}
	bcm_bprintf(strbuf, "D3 inform cnt %d\n", dhdp->bus->d3_inform_cnt);
	bcm_bprintf(strbuf, "D0 inform cnt %d\n", dhdp->bus->d0_inform_cnt);
	bcm_bprintf(strbuf, "D0 inform in use cnt %d\n", dhdp->bus->d0_inform_in_use_cnt);
}

/**
 * Brings transmit packets on all flow rings closer to the dongle, by moving (a subset) from their
 * flow queue to their flow ring.
 */
static void
dhd_update_txflowrings(dhd_pub_t *dhd)
{
	dll_t *item, *next;
	flow_ring_node_t *flow_ring_node;
	struct dhd_bus *bus = dhd->bus;

	for (item = dll_head_p(&bus->const_flowring);
		(!dhd_is_device_removed(dhd) && !dll_end(&bus->const_flowring, item));
		item = next) {
		if (dhd->hang_was_sent) {
			break;
		}

		next = dll_next_p(item);
		flow_ring_node = dhd_constlist_to_flowring(item);

		/* Ensure that flow_ring_node in the list is Not Null */
		ASSERT(flow_ring_node != NULL);

		/* Ensure that the flowring node has valid contents */
		ASSERT(flow_ring_node->prot_info != NULL);

#if defined(BCM_DHD_RUNNER)
		/* Don't process runner based flow rings here */
		if (!(DHD_FLOWRING_RNR_OFFL(flow_ring_node)))
#endif /* BCM_DHD_RUNNER */
		dhd_prot_update_txflowring(dhd, flow_ring_node->flowid, flow_ring_node->prot_info);
	}
}

/** Mailbox ringbell Function */
static void
dhd_bus_gen_devmb_intr(struct dhd_bus *bus)
{
	if ((bus->sih->buscorerev == 2) || (bus->sih->buscorerev == 6) ||
		(bus->sih->buscorerev == 4)) {
		DHD_ERROR(("mailbox communication not supported\n"));
		return;
	}
	if (bus->db1_for_mb)  {
		/* this is a pcie core register, not the config register */
		DHD_INFO(("writing a mail box interrupt to the device, through doorbell 1\n"));
		si_corereg(bus->sih, bus->sih->buscoreidx, PCIH2D_DB1, ~0, 0x12345678);
	} else {
		DHD_INFO(("writing a mail box interrupt to the device, through config space\n"));
		dhdpcie_bus_cfg_write_dword(bus, PCISBMbx, 4, (1 << 0));
		dhdpcie_bus_cfg_write_dword(bus, PCISBMbx, 4, (1 << 0));
	}
}

static void
dhd_bus_set_device_wake(struct dhd_bus *bus, bool val)
{
	if (bus->device_wake_state != val)
	{
		DHD_INFO(("Set Device_Wake to %d\n", val));
#ifdef PCIE_OOB
		if (bus->oob_enabled)
		{
			if (val)
			{
				gpio_port = gpio_port | (1 << DEVICE_WAKE);
				gpio_write_port_non_block(gpio_handle_val, gpio_port);
			} else {
				gpio_port = gpio_port & (0xff ^ (1 << DEVICE_WAKE));
				gpio_write_port_non_block(gpio_handle_val, gpio_port);
			}
		}
#endif /* PCIE_OOB */
		bus->device_wake_state = val;
	}
}

#ifdef PCIE_OOB
void
dhd_oob_set_bt_reg_on(struct dhd_bus *bus, bool val)
{
	DHD_INFO(("Set Device_Wake to %d\n", val));
	if (val)
	{
		gpio_port = gpio_port | (1 << BIT_BT_REG_ON);
		gpio_write_port(gpio_handle_val, gpio_port);
	} else {
		gpio_port = gpio_port & (0xff ^ (1 << BIT_BT_REG_ON));
		gpio_write_port(gpio_handle_val, gpio_port);
	}
}

int
dhd_oob_get_bt_reg_on(struct dhd_bus *bus)
{
	int ret;
	uint8 val;
	ret = gpio_read_port(gpio_handle_val, &val);

	if (ret < 0) {
		DHD_ERROR(("gpio_read_port returns %d\n", ret));
		return ret;
	}

	if (val & (1 << BIT_BT_REG_ON))
	{
		ret = 1;
	} else {
		ret = 0;
	}

	return ret;
}

static void
dhd_bus_doorbell_timeout_reset(struct dhd_bus *bus)
{
	if (dhd_doorbell_timeout)
		dhd_timeout_start(&bus->doorbell_timer,
			(dhd_doorbell_timeout * 1000) / dhd_watchdog_ms);
	else if (!(bus->dhd->busstate == DHD_BUS_SUSPENDING ||
	           bus->dhd->busstate == DHD_BUS_SUSPENDED))
	         dhd_bus_set_device_wake(bus, FALSE);
}
#endif /* PCIE_OOB */

/** mailbox doorbell ring function */
void
dhd_bus_ringbell(struct dhd_bus *bus, uint32 value)
{
	if ((bus->sih->buscorerev == 2) || (bus->sih->buscorerev == 6) ||
		(bus->sih->buscorerev == 4)) {
		si_corereg(bus->sih, bus->sih->buscoreidx, PCIMailBoxInt, PCIE_INTB, PCIE_INTB);
	} else {
		/* this is a pcie core register, not the config regsiter */
		DHD_INFO(("writing a door bell to the device\n"));
		if (IDMA_ACTIVE(bus->dhd)) {
			si_corereg(bus->sih, bus->sih->buscoreidx, PCIH2D_MailBox_2,
				~0, value);
		} else {
			si_corereg(bus->sih, bus->sih->buscoreidx, PCIH2D_MailBox,
				~0, 0x12345678);
		}
	}
}

/** mailbox doorbell ring function for IDMA using dma channel2 */
void
dhd_bus_ringbell_2(struct dhd_bus *bus, uint32 value, bool devwake)
{
	/* this is a pcie core register, not the config regsiter */
	DHD_INFO(("writing a door bell 2 to the device\n"));
	si_corereg(bus->sih, bus->sih->buscoreidx, PCIH2D_MailBox_2, ~0, value);
}

/**
 * HWA iDMA specific doorbell ring function (HostToDev2Doorbell1)
 * Seperate DMA_TYPE::HWA_RXPOST, HWA_TXPOST and HWA_RXPOST to use this one rather than
 * HostToDev2Doorbell0, so that dongle can ignore this signal for these HWA iDMA types.
 * Otherwise if we use HostToDev2Doorbell0 for HWA iDMA which can also be use by IDMA too
 * then dongle will be signaled unnecessary for HWA iDMA type.
 */
void
dhd_bus_db1_ringbell_2(struct dhd_bus *bus, uint32 value, bool devwake)
{
	/* this is a pcie core register, not the config regsiter */
	DHD_INFO(("writing a Doorbell1 through channel2 to the device\n"));
	si_corereg(bus->sih, bus->sih->buscoreidx, PCIH2D_DB1_2, ~0, value);
}

void
dhdpcie_bus_ringbell_fast(struct dhd_bus *bus, uint32 value)
{
#ifdef PCIE_OOB
	dhd_bus_set_device_wake(bus, TRUE);
	dhd_bus_doorbell_timeout_reset(bus);
#endif
	W_REG(bus->pcie_mb_intr_osh, bus->pcie_mb_intr_addr, value);
}

/** doorbell ring function (HostToDev2Doorbell0) for IDMA using dma channel2 */
void
dhdpcie_bus_ringbell_2_fast(struct dhd_bus *bus, uint32 value, bool devwake)
{
#ifdef PCIE_OOB
	if (devwake) {
		dhd_bus_set_device_wake(bus, TRUE);
		dhd_bus_doorbell_timeout_reset(bus);
	}
#endif
	W_REG(bus->pcie_mb_intr_osh, bus->pcie_mb_intr_2_addr, value);
}

/**
 * HWA iDMA specific doorbell ring function (HostToDev2Doorbell1)
 * Seperate DMA_TYPE::HWA_RXPOST, HWA_TXPOST and HWA_RXPOST to use this one rather than
 * HostToDev2Doorbell0, so that dongle can ignore this signal for these HWA iDMA types.
 * Otherwise if we use HostToDev2Doorbell0 for HWA iDMA which can also be use by IDMA too
 * then dongle will be signaled unnecessary for HWA iDMA type.
 */
void
dhdpcie_bus_db1_ringbell_2_fast(struct dhd_bus *bus, uint32 value, bool devwake)
{
#ifdef PCIE_OOB
	if (devwake) {
		dhd_bus_set_device_wake(bus, TRUE);
		dhd_bus_doorbell_timeout_reset(bus);
	}
#endif
	W_REG(bus->pcie_mb_intr_osh, bus->pcie_db1_intr_2_addr, value);
}

static void
dhd_bus_ringbell_oldpcie(struct dhd_bus *bus, uint32 value)
{
	uint32 w;
	w = (R_REG(bus->pcie_mb_intr_osh, bus->pcie_mb_intr_addr) & ~PCIE_INTB) | PCIE_INTB;
	W_REG(bus->pcie_mb_intr_osh, bus->pcie_mb_intr_addr, w);
}

dhd_mb_ring_t
dhd_bus_get_mbintr_fn(struct dhd_bus *bus)
{
	if ((bus->sih->buscorerev == 2) || (bus->sih->buscorerev == 6) ||
		(bus->sih->buscorerev == 4)) {
		bus->pcie_mb_intr_addr = si_corereg_addr(bus->sih, bus->sih->buscoreidx,
			PCIMailBoxInt);
		if (bus->pcie_mb_intr_addr) {
			bus->pcie_mb_intr_osh = si_osh(bus->sih);
			return dhd_bus_ringbell_oldpcie;
		}
	} else {
		bus->pcie_mb_intr_addr = si_corereg_addr(bus->sih, bus->sih->buscoreidx,
			PCIH2D_MailBox);
		if (bus->pcie_mb_intr_addr) {
			bus->pcie_mb_intr_osh = si_osh(bus->sih);
			return dhdpcie_bus_ringbell_fast;
		}
	}

	return dhd_bus_ringbell;
}

dhd_mb_ring_2_t
dhd_bus_get_mbintr_2_fn(struct dhd_bus *bus)
{
	bus->pcie_mb_intr_2_addr = si_corereg_addr(bus->sih, bus->sih->buscoreidx,
		PCIH2D_MailBox_2);
	if (bus->pcie_mb_intr_2_addr) {
		bus->pcie_mb_intr_osh = si_osh(bus->sih);
		return dhdpcie_bus_ringbell_2_fast;
	}

	return dhd_bus_ringbell_2;
}

dhd_mb_ring_2_t
dhd_bus_get_db1intr_2_fn(struct dhd_bus *bus)
{
	bus->pcie_db1_intr_2_addr = si_corereg_addr(bus->sih, bus->sih->buscoreidx,
		PCIH2D_DB1_2);
	if (bus->pcie_db1_intr_2_addr) {
		bus->pcie_mb_intr_osh = si_osh(bus->sih);
		return dhdpcie_bus_db1_ringbell_2_fast;
	}

	return dhd_bus_db1_ringbell_2;
}

bool BCMFASTPATH
dhd_bus_dpc(struct dhd_bus *bus)
{
	bool resched = FALSE;	  /* Flag indicating resched wanted */

	DHD_TRACE(("%s: Enter\n", __FUNCTION__));

	if (bus->dhd->busstate == DHD_BUS_DOWN) {
		DHD_ERROR(("%s: Bus down, ret\n", __FUNCTION__));
		bus->intstatus = 0;
		return 0;
	}

	if (bus->intstatus) {
		resched = dhdpcie_bus_process_mailbox_intr(bus, bus->intstatus);

		if (!resched) {
			bus->intstatus = 0;
			dhdpcie_bus_intr_enable(bus);
		}
	}

#if defined(BCM_DHD_RUNNER)
	/*
	 * Process Dongle -> Runner -> Host CPU Queue messages in dpc context
	 */
	resched |= dhd_runner_process_cpu_queue(bus->dhd->runner_hlp, resched);
#endif /* !BCM_DHD_RUNNER */

#if defined(BCM_AWL)
#if defined(DHD_AWL_TX) && defined(AWL_TX_DPC)
	/*
	 * Process LAN/WLAN -> Archer -> PKTFWD (flow-hit) packets in dpc context
	 */
	resched |= dhd_awl_process_fastpath_txpkts(bus->dhd, dhd_txbound);
#endif /* DHD_AWL_TX && AWL_TX_DPC */

#if defined(DHD_AWL_RX)
	/*
	 * Process Dongle -> DHD -> Archer (flow-miss)  packets in dpc context
	 */
	resched |= dhd_awl_process_slowpath_rxpkts(bus->dhd, dhd_rxbound);
#endif /* DHD_AWL_RX */
#endif /* BCM_AWL */

	return resched;

}

/* Upon receiving a mailbox interrupt,
 * if PCIE_IPC_H2DMB_FW_TRAP bit is set in mailbox location
 * device traps
 */
static void
dhdpcie_fw_trap(dhd_bus_t *bus)
{
	/* Send the mailbox data and generate mailbox intr. */
	dhdpcie_send_mb_data(bus, PCIE_IPC_H2DMB_FW_TRAP);
	/* For FWs that cannot interprete PCIE_IPC_H2DMB_FW_TRAP */
	(void)dhd_wl_ioctl_set_intiovar(bus->dhd, "bus:disconnect", 99, WLC_SET_VAR, TRUE, 0);
}

static void
dhdpcie_send_mb_data(dhd_bus_t *bus, uint32 h2d_mb_data)
{
	uint32 cur_h2d_mb_data = 0;

	DHD_INFO_HW4(("%s: H2D_MB_DATA: 0x%08X\n", __FUNCTION__, h2d_mb_data));
	dhd_bus_cmn_readshared(bus, &cur_h2d_mb_data, H2D_MB_DATA, 0);

	if (cur_h2d_mb_data != 0) {
		uint32 i = 0;
		DHD_INFO(("GRRRRRRR: MB transaction is already pending 0x%04x\n", cur_h2d_mb_data));
		while ((i++ < 100) && cur_h2d_mb_data) {
			OSL_DELAY(10);
			dhd_bus_cmn_readshared(bus, &cur_h2d_mb_data, H2D_MB_DATA, 0);
		}
		if (i >= 100) {
			DHD_ERROR(("%s : waited 1ms for the dngl "
				"to ack the previous mb transaction\n", __FUNCTION__));
			DHD_ERROR(("%s : MB transaction is still pending 0x%04x\n",
				__FUNCTION__, cur_h2d_mb_data));
		}
	}

	dhd_bus_cmn_writeshared(bus, &h2d_mb_data, H2D_MB_DATA, 0);
	dhd_bus_gen_devmb_intr(bus);

	if (h2d_mb_data == PCIE_IPC_H2DMB_HOST_D3_INFORM) {
		DHD_INFO_HW4(("%s: send PCIE_IPC_H2DMB_HOST_D3_INFORM to dongle\n",
			__FUNCTION__));
		bus->d3_inform_cnt++;
	}
	if (h2d_mb_data == PCIE_IPC_H2DMB_HOST_D0_INFORM_IN_USE) {
		DHD_INFO_HW4(("%s: send PCIE_IPC_H2DMB_HOST_D0_INFORM_IN_USE to dongle\n",
			__FUNCTION__));
		bus->d0_inform_in_use_cnt++;
	}
	if (h2d_mb_data == PCIE_IPC_H2DMB_HOST_D0_INFORM) {
		DHD_INFO_HW4(("%s: send PCIE_IPC_H2DMB_HOST_D0_INFORM to dongle\n",
			__FUNCTION__));
		bus->d0_inform_cnt++;
	}
}

static bool dhdpcie_is_dab_trap(dhd_bus_t *bus, uint32 tr_type)
{
	uint arm_idx;

	arm_idx = si_findcoreidx(bus->sih, ARMCM3_CORE_ID, 0);
	if (arm_idx == BADIDX || BCM43602_CHIP(bus->sih->chip)) {
		return (tr_type == TR_DAB);
	}

	return (tr_type == TR_BUS);
}

static void
dhdpcie_handle_mb_data(dhd_bus_t *bus)
{
	uint32 d2h_mb_data = 0;
	uint32 zero = 0;

	dhd_bus_cmn_readshared(bus, &d2h_mb_data, D2H_MB_DATA, 0);
	if (!d2h_mb_data) {
		DHD_INFO_HW4(("%s: Invalid D2H_MB_DATA: 0x%08x\n",
			__FUNCTION__, d2h_mb_data));
		return;
	}

	dhd_bus_cmn_writeshared(bus, &zero, D2H_MB_DATA, 0);

	DHD_INFO_HW4(("D2H_MB_DATA: 0x%08x\n", d2h_mb_data));
	if (d2h_mb_data & PCIE_IPC_D2HMB_DEV_DS_ENTER_REQ)  {
		/* what should we do */
		DHD_INFO(("D2H_MB_DATA: DEEP SLEEP REQ\n"));
		dhdpcie_send_mb_data(bus, PCIE_IPC_H2DMB_HOST_DS_ACK);
		DHD_INFO(("D2H_MB_DATA: sent DEEP SLEEP ACK\n"));
	}
	if (d2h_mb_data & PCIE_IPC_D2HMB_DEV_DS_EXIT_NOTE)  {
		/* what should we do */
		DHD_INFO(("D2H_MB_DATA: DEEP SLEEP EXIT\n"));
	}
	if (d2h_mb_data & PCIE_IPC_D2HMB_DEV_D3_ACK)  {
		/* what should we do */
		DHD_INFO_HW4(("D2H_MB_DATA: D3 ACK\n"));
		if (!bus->wait_for_d3_ack) {
			bus->wait_for_d3_ack = 1;
			dhd_os_d3ack_wake(bus->dhd);
		}
	}
	if ((d2h_mb_data & PCIE_IPC_D2HMB_DEV_FWHALT) &&
#ifdef BCA_HNDROUTER
		(!is_reboot) &&
#endif
		TRUE) {

		uint32 tr_type;

		DHD_ERROR(("FW trap has happened\n"));
		dhdpcie_checkdied(bus, NULL, 0, &tr_type);
		/* not ready yet dhd_os_ind_firmware_stall(bus->dhd); */
		bus->dhd->busstate = DHD_BUS_DOWN;

		/* Call the bus module watchdog */
		dhd_bus_watchdog(bus->dhd);

		/* Get dump and signal to the debug_monitor process to notify of firmware traps */
		DHD_ERROR(("%s: Schedule trap log dump\n", __FUNCTION__));
		dhd_schedule_trap_log_dump(bus->dhd, dhdpcie_is_dab_trap(bus, tr_type));
	}
}

/** @return TRUE if dpc needs to be rescheduled */
static bool
dhdpcie_bus_process_mailbox_intr(dhd_bus_t *bus, uint32 intstatus)
{
	bool resched = FALSE;

	if ((bus->sih->buscorerev == 2) || (bus->sih->buscorerev == 6) ||
		(bus->sih->buscorerev == 4)) {
		/* Msg stream interrupt */
		if (intstatus & I_BIT1) {
			resched = dhdpci_bus_read_frames(bus);
		} else if (intstatus & I_BIT0) {
			/* do nothing for Now */
		}
	} else {
		if (intstatus & (PCIE_MB_TOPCIE_FN0_0 | PCIE_MB_TOPCIE_FN0_1))
			dhdpcie_handle_mb_data(bus);

		if (bus->dhd->busstate == DHD_BUS_SUSPENDING ||
		    bus->dhd->busstate == DHD_BUS_SUSPENDED) {
			goto exit;
		}

		if (intstatus & PCIE_MB_D2H_MB_MASK) {
			resched = dhdpci_bus_read_frames(bus);
		}
	}

exit:
	return resched;
}

static bool
dhdpci_bus_read_frames(dhd_bus_t *bus)
{
	bool more = FALSE;

	/* There may be frames in both ctrl buf and data buf; check ctrl buf first */
	DHD_LOCK(bus->dhd);
	dhd_prot_process_ctrlbuf(bus->dhd);
	/* Unlock to give chance for resp to be handled */
	DHD_UNLOCK(bus->dhd);

	DHD_LOCK(bus->dhd);

#if defined(BCM_DHD_RUNNER)
	dhd_prot_schedule_runner(bus->dhd);
#endif /* BCM_DHD_RUNNER */

	/* update the flow ring cpls */
	dhd_update_txflowrings(bus->dhd);

	/* With heavy TX traffic, we could get a lot of TxStatus
	 * so add bound
	 */
#if defined(BCM_DHD_RUNNER)
	if (!DHD_RNR_OFFL_TXSTS(bus->dhd))
#endif /* ! BCM_DHD_RUNNER */
	/* TODO: Do we need is_hw_ring check for this function ? */
	more |= dhd_prot_process_msgbuf_txcpl(bus->dhd, dhd_txbound);

#if defined(BCM_DHD_RUNNER)
	if (!DHD_RNR_OFFL_RXCMPL(bus->dhd))
#endif /* ! BCM_DHD_RUNNER */
	/* With heavy RX traffic, this routine potentially could spend some time
	 * processing RX frames without RX bound
	 */
	more |= dhd_prot_process_msgbuf_rxcpl(bus->dhd, dhd_rxbound);

	/* don't talk to the dongle if fw is about to be reloaded */
	if (bus->dhd->hang_was_sent) {
		more = FALSE;
	}
	DHD_UNLOCK(bus->dhd);

	return more;
}

/*
 * Read the last word in dongle's memory, to retrieve the location of the
 * pcie_ipc_t structure. Upload the pcie_ipc_t structure (not including the
 * extension). Compare one field, e.g. pcie_ipc_t::rings_daddr32.
 */
bool
dhdpcie_tcm_valid(dhd_bus_t *bus)
{
	daddr_t last_word_daddr32; /* Address of last word in dongle memory */
	daddr32_t pcie_ipc_daddr32 = 0; /* Location of pcie_ipc_t in dongle memory */
	pcie_ipc_t pcie_ipc; /* local stack copy of pcie_ipc_t::base */

	last_word_daddr32 = bus->dongle_ram_base + bus->ramsize - 4;

	/* Read last word in memory to determine address of pcie_ipc structure */
	pcie_ipc_daddr32 = LTOH32(dhdpcie_bus_read_u32(bus, last_word_daddr32));

	if ((pcie_ipc_daddr32 == 0) || (pcie_ipc_daddr32 == bus->nvram_csm) ||
		(pcie_ipc_daddr32 < bus->dongle_ram_base) ||
		(pcie_ipc_daddr32 > last_word_daddr32))
	{
		DHD_ERROR(("%s: PCIE IPC address (0x%08x) invalid\n",
			__FUNCTION__, pcie_ipc_daddr32));
		return FALSE;
	}

	/* Read pcie_ipc_t structure (rev5 base portion only) */
	dhdpcie_bus_membytes(bus, PCIE_SYSMEM_READ, pcie_ipc_daddr32,
		(uint8 *)&pcie_ipc, PCIE_IPC_REV5_SZ);

	/* Compare any field in pcie_ipc_t */
	if (pcie_ipc.rings_daddr32 != bus->pcie_ipc.rings_daddr32) {
		DHD_ERROR(("Contents of pcie_ipc_t structure are not matching.\n"));
		return FALSE;
	}

	return (bus->pcie_ipc_rings_daddr32 != 0U);
} /* dhdpcie_tcm_valid */

/**
 * Determine whether the Host driver is compatible with the dongle PCIE IP rev.
 */
static bool
dhdpcie_ipc_rev_compatible(uint32 host_rev, uint32 dngl_rev)
{
	DHD_INFO(("PCIe IPC Revision compatibility: host 0x%02x, dngl 0x%02x\n",
		host_rev, dngl_rev));

	ASSERT(PCIE_IPC_REV_IS_BCA(host_rev));

	/* Firmware is BCA PCIE IPC revision */
	if (PCIE_IPC_REV_IS_BCA(dngl_rev)) {
		if (host_rev >= dngl_rev) {
			return TRUE;
		} else {
			DHD_ERROR(("\n\n\n\033[1m\033[34m"
				"################################################\n"
				"###### BCA PCIe IPC REVISION INCOMPATIBLE ######\n"
				"###### UPGRADE DHD TO PCIe IPC REV [0x%02x] ######\n"
				"################################################\033[0m\n\n\n",
				dngl_rev));
			return FALSE;
		}
	} else {
		if (dngl_rev == PCIE_IPC_DEFAULT_HOST_REVISION) {
			return TRUE;
		} else {
			DHD_ERROR(("Legacy IPC Rev failure:"
				" host_rev 0x%02x, dngl_rev %d != legacy rev %d\n",
				host_rev, dngl_rev, PCIE_IPC_DEFAULT_HOST_REVISION));
			return FALSE;
		}
	}

	return FALSE;

} /* dhdpcie_ipc_rev_compatible */

/* LTOH Helper macros used in dhdpcie_bus_read_pcie_ipc() */
#define _PCIE_IPC_LTOH16(field) \
	pcie_ipc->field = LTOH16(pcie_ipc->field)
#define _PCIE_IPC_LTOH32(field) \
	pcie_ipc->field = LTOH32(pcie_ipc->field)

#define _PCIE_IPC_RINGS_LTOH16(field) \
	pcie_ipc_rings->field = LTOH16(pcie_ipc_rings->field)
#define _PCIE_IPC_RINGS_LTOH32(field) \
	pcie_ipc_rings->field = LTOH32(pcie_ipc_rings->field)

#define _PCIE_IPC_HME_LTOH16(field) \
	pcie_ipc_hme->field = LTOH16(pcie_ipc_hme->field)
#define _PCIE_IPC_HME_LTOH32(field) \
	pcie_ipc_hme->field = LTOH32(pcie_ipc_hme->field)

#define _PCIE_IPC_HME_USER_LTOH16(field) \
	pcie_ipc_hme_user->field = LTOH16(pcie_ipc_hme_user->field)
#define _PCIE_IPC_HME_USER_LTOH32(field) \
	pcie_ipc_hme_user->field = LTOH32(pcie_ipc_hme_user->field)

/**
 * Fetch the PCIE IPC and Rings structures from dongle's memory.
 * Caller will provide the local storage for these structures. Local storage
 * must be large enough to carry the entire structures, including extensions.
 *
 * The returned structure fields will be in Host Order.
 */
static int
dhdpcie_bus_read_pcie_ipc(dhd_bus_t *bus, daddr32_t *pcie_ipc_daddr32,
	pcie_ipc_t *pcie_ipc, pcie_ipc_rings_t *pcie_ipc_rings,
	pcie_ipc_hme_t **pcie_ipc_hme_p)
{
	uint32 val32, user_id;
	daddr32_t daddr32 = 0U; /* pcie_ipc_t location in dongle memory */
	uint32 pcie_ipc_size, host_ipc_revision, dngl_ipc_revision;

	pcie_ipc_hme_t      * pcie_ipc_hme;
	pcie_ipc_hme_user_t * pcie_ipc_hme_user;

	ASSERT(pcie_ipc != (pcie_ipc_t*)NULL);
	*pcie_ipc_daddr32 = 0U;

	DHD_PCIE_IPC(("PCIe IPC UPLOAD COMMENCE\n"));

	{   /* Locate the pcie_ipc_t structure in dongle's memory */
		dhd_timeout_t tmo;
		daddr32_t last_word_daddr32;

		last_word_daddr32 = bus->dongle_ram_base + bus->ramsize - 4;
		dhd_timeout_start(&tmo, MAX_READ_TIMEOUT); /* 5 second timer */

		while (((daddr32 == 0U) || (daddr32 == bus->nvram_csm)) &&
		       !dhd_timeout_expired(&tmo))
		{
			/* Location of pcie_ipc_t is placed in last word of dongle memory */
			val32   = dhdpcie_bus_read_u32(bus, last_word_daddr32);
			daddr32 = LTOH32(val32);
		}

		/* Validate the address written to last word in dongle memory */
		if ((daddr32 == 0U) || (daddr32 == bus->nvram_csm) ||
			(daddr32 < bus->dongle_ram_base) || (daddr32 > last_word_daddr32))
		{
			DHD_ERROR(("%s: PCIe IPC LOCATION FAILURE: "
				"daddr32 0x%08x invalid. ram[0x%08x,0x%08x]\n", __FUNCTION__,
				daddr32, bus->dongle_ram_base, last_word_daddr32));
			return BCME_NOTREADY;
		}

		DHD_PCIE_IPC(("PCIe IPC LOCATED: "
			"read_u32 daddr32 0x%08x took %u usec. ram[0x%08x,0x%08x]\n",
			daddr32, tmo.elapsed, bus->dongle_ram_base, last_word_daddr32));
	}

	/* Fetch the dongle's PCIe IPC revision from the first word in pcie_ipc_t */
	val32 = dhdpcie_bus_read_u32(bus, daddr32);
	val32 = LTOH32(val32);
	dngl_ipc_revision = PCIE_IPC_REV_GET(val32);
	host_ipc_revision = PCIE_IPC_REVISION;

	/* Verify compatibility of host and dongle IPC Revisions */
	if (!(dhdpcie_ipc_rev_compatible(host_ipc_revision, dngl_ipc_revision))) {
		DHD_ERROR(("%s: PCIe IPC REVISION FAILURE: "
			" host 0x%02x incompatible with dngl 0x%02x\n", __FUNCTION__,
			host_ipc_revision, dngl_ipc_revision));
		return BCME_VERSION;
	}

	/* Copy pcie_ipc_t into local memory */

	/* Given the dongle IPC revision, determine the size of pcie_ipc_t */
	pcie_ipc_size = PCIE_IPC_EXTN_PRESENT(dngl_ipc_revision) ?
		PCIE_IPC_SZ : PCIE_IPC_REV5_SZ;

	dhdpcie_bus_membytes(bus, PCIE_SYSMEM_READ, daddr32,
		(uint8 *)pcie_ipc, pcie_ipc_size);

	/* Convert entire pcie_ipc_t structure to Host Endian Order */
	_PCIE_IPC_LTOH32(flags);
	_PCIE_IPC_LTOH32(trap_daddr32);
	_PCIE_IPC_LTOH32(assert_exp_daddr32);
	_PCIE_IPC_LTOH32(assert_file_daddr32);
	_PCIE_IPC_LTOH32(assert_line);
	_PCIE_IPC_LTOH32(console_daddr32);
	// _PCIE_IPC_LTOH32(msgtrace_daddr32); deprecated
	_PCIE_IPC_LTOH32(fwid);
	_PCIE_IPC_LTOH16(max_tx_pkts);
	_PCIE_IPC_LTOH16(max_rx_pkts);
	_PCIE_IPC_LTOH32(dma_rxoffset); // deprecated
	_PCIE_IPC_LTOH32(h2d_mb_daddr32);
	_PCIE_IPC_LTOH32(d2h_mb_daddr32);
	_PCIE_IPC_LTOH32(rings_daddr32);
	_PCIE_IPC_LTOH32(host_mem_len); // filled by DHD, should be 0
	_PCIE_IPC_LTOH32(host_mem_haddr64.lo); // filled by DHD, should be 0
	_PCIE_IPC_LTOH32(host_mem_haddr64.hi); // filled by DHD, should be 0

	_PCIE_IPC_LTOH16(host_mem_users);
	_PCIE_IPC_LTOH16(host_mem_size);
	_PCIE_IPC_LTOH32(host_mem_daddr32);

	_PCIE_IPC_LTOH32(buzzz_daddr32);

	DHD_PCIE_IPC(("PCIe IPC HEADER: "
		"daddr32 0x%08x size %u revision: host 0x%02x dngl 0x%02x\n"
		"\t FWID 0x%08x, flags 0x%08x, dcap1 0x%08x dcap2 0x%08x\n"
		"\t trap 0x%08x assrt 0x%08x,0x%08x,%u cons 0x%08x\n"
		"\t mbox H2D 0x%08x D2H 0x%08x rings 0x%08x\n",
		daddr32, pcie_ipc_size, host_ipc_revision, dngl_ipc_revision,
		pcie_ipc->fwid, pcie_ipc->flags, pcie_ipc->dcap1, pcie_ipc->dcap2,
		pcie_ipc->trap_daddr32, pcie_ipc->assert_exp_daddr32,
		pcie_ipc->assert_file_daddr32, pcie_ipc->assert_line,
		pcie_ipc->console_daddr32,
		pcie_ipc->h2d_mb_daddr32, pcie_ipc->d2h_mb_daddr32,
		pcie_ipc->rings_daddr32));

	if (PCIE_IPC_EXTN_PRESENT(dngl_ipc_revision)) {
		_PCIE_IPC_LTOH32(dcap1);
		_PCIE_IPC_LTOH32(dcap2);
		_PCIE_IPC_LTOH32(hcap1);
		_PCIE_IPC_LTOH32(hcap2);
		_PCIE_IPC_LTOH32(host_physaddrhi);

		if (dngl_ipc_revision < 0x84) {
			for (user_id = 0; user_id < PCIE_IPC_HME_USERS_MAX; ++user_id) {
				_PCIE_IPC_LTOH16(hme_pages[user_id]);
			}
		}
	}

	/* Check whether Caller requested the fetching of pcie_ipc_hme_t, too */
	if (pcie_ipc_hme_p == (pcie_ipc_hme_t**)NULL)
		goto skip_pcie_ipc_hme;
	if ((pcie_ipc->dcap1 & PCIE_IPC_DCAP1_HOST_MEM_EXTN) == 0)
		goto skip_pcie_ipc_hme;

	ASSERT(*pcie_ipc_hme_p == (pcie_ipc_hme_t*)NULL);

	/* Compose pcie_ipc_hme_t
	 * - PCIe IPC Rev >= 0x84: Fetch pcie_ipc_hme_t from dongle
	 * - PCIe IPC Rev <  0x84: Use hme_pages and default attributes
	 */
	if (dngl_ipc_revision >= 0x84) // Discrete pcie_ipc_hme_t
	{
		/* Discrete pcie_ipc_hme_t linked into pcie_ipc_t */
		uint32 pcie_ipc_hme_users   = (uint32)pcie_ipc->host_mem_users;
		uint32 pcie_ipc_hme_size    = (uint32)pcie_ipc->host_mem_size;
		uint32 pcie_ipc_hme_daddr32 = pcie_ipc->host_mem_daddr32;

		/* Audit pcie_ipc_hme_t linkage */
		ASSERT(pcie_ipc_hme_size != 0U);
		ASSERT(pcie_ipc_hme_size == PCIE_IPC_HOST_MEM_SIZE(pcie_ipc_hme_users));

		/* Allocate a pcie_ipc_hme_t object for the dhd_bus */
		pcie_ipc_hme = (pcie_ipc_hme_t*)MALLOCZ(bus->dhd->osh, pcie_ipc_hme_size);
		if (pcie_ipc_hme == (pcie_ipc_hme_t*)NULL) {
			DHD_ERROR(("%s: PCIe IPC MEMORY FAILURE: "
				"malloc %u bytes. [pcie_ipc_hme users %u]\n",
				__FUNCTION__, pcie_ipc_hme_size, pcie_ipc_hme_users));
			return BCME_NOMEM;
		}

		/* Upload the pcie_ipc_hme_t from dongle */
		dhdpcie_bus_membytes(bus, PCIE_SYSMEM_READ, pcie_ipc_hme_daddr32,
			(uint8 *)pcie_ipc_hme, pcie_ipc_hme_size);

		_PCIE_IPC_HME_LTOH16(size); /* size of array of pcie_ipc_hme_user(s) */

		ASSERT(pcie_ipc_hme->size == pcie_ipc_hme_size);
		/* pcie_ipc_hme::<bytes, haddr64> set later by dhd_prot */
		ASSERT(pcie_ipc_hme->bytes == 0U);
		ASSERT(HADDR64_IS_ZERO(pcie_ipc_hme->haddr64));

		for (user_id = 0; user_id < (uint32)pcie_ipc_hme->users; ++user_id)
		{
			pcie_ipc_hme_user = &pcie_ipc_hme->user[user_id];
			_PCIE_IPC_HME_USER_LTOH16(flags);
			_PCIE_IPC_HME_USER_LTOH16(pages);
		}

	} else { // pre 0x84: PCIE_IPC_HME_USERS_MAX

		uint32 pcie_ipc_hme_size;

		pcie_ipc_hme_size = PCIE_IPC_HOST_MEM_SIZE(PCIE_IPC_HME_USERS_MAX);
		pcie_ipc_hme = (pcie_ipc_hme_t*)MALLOCZ(bus->dhd->osh, pcie_ipc_hme_size);
		if (pcie_ipc_hme == (pcie_ipc_hme_t*)NULL) {
			DHD_ERROR(("%s: PCIe IPC MEMORY FAILURE: "
				"malloc %u bytes. [pcie_ipc_hme users %u]\n",
				__FUNCTION__, pcie_ipc_hme_size, PCIE_IPC_HME_USERS_MAX));
			return BCME_NOMEM;
		}

		pcie_ipc_hme->version = 1; // Legacy HME service
		pcie_ipc_hme->users   = PCIE_IPC_HME_USERS_MAX;
		pcie_ipc_hme->size    = pcie_ipc_hme_size;

		for (user_id = 0; user_id < PCIE_IPC_HME_USERS_MAX; ++user_id)
		{
			/* Store into bus as-if PCIe IPC Rev 0x84 form */
			pcie_ipc_hme_user = &pcie_ipc_hme->user[user_id];

			pcie_ipc_hme_user->user_id     = user_id;
			/* Apply default HME memory attributes */
			pcie_ipc_hme_user->align_bits  = PCIE_IPC_HME_ALIGN_BITS; // 256 B
			pcie_ipc_hme_user->bound_bits  = PCIE_IPC_HME_BOUND_BITS; // 32 MB
			pcie_ipc_hme_user->flags       = PCIE_IPC_HME_FLAGS;
			pcie_ipc_hme_user->pages       = pcie_ipc->hme_pages[user_id];
			snprintf(pcie_ipc_hme_user->name, PCIE_IPC_HME_USER_NAME_SIZE,
			                                  "USR_%02u", user_id);
		}

	} // Compose pcie_ipc_hme_t

	*pcie_ipc_hme_p = pcie_ipc_hme; // store into bus

	DHD_PCIE_IPC(("PCIe IPC HEADER HME: version %u users %u size %u\n",
		pcie_ipc_hme->version, pcie_ipc_hme->users, pcie_ipc_hme->size));

	DHD_PCIE_IPC(("\t ID HMEUSR PAGES ALIGN   BOUNDARY  FLAGS\n"));
	for (user_id = 0; user_id < pcie_ipc_hme->users; ++user_id)
	{
		pcie_ipc_hme_user = &pcie_ipc_hme->user[user_id];
		DHD_PCIE_IPC(("\t %2u %6s %5u %5u 0x%08x 0x%04x\n",
			pcie_ipc_hme_user->user_id, pcie_ipc_hme_user->name,
			pcie_ipc_hme_user->pages,
			(1 << pcie_ipc_hme_user->align_bits),
			(1 <<pcie_ipc_hme_user->bound_bits) - 1,
			pcie_ipc_hme_user->flags));
	}

skip_pcie_ipc_hme:

	/* Caller requested the fetching of pcie_ipc_rings_t, too */
	if (pcie_ipc_rings != (pcie_ipc_rings_t*)NULL)
	{
		uint32 pcie_ipc_rings_sz;

		/* Given the dongle IPC revision, determine size of pcie_ipc_rings_t */
		pcie_ipc_rings_sz = PCIE_IPC_EXTN_PRESENT(dngl_ipc_revision) ?
			PCIE_IPC_RINGS_SZ : PCIE_IPC_RINGS_REV5_SZ;

		dhdpcie_bus_membytes(bus, PCIE_SYSMEM_READ, pcie_ipc->rings_daddr32,
			(uint8 *)pcie_ipc_rings, pcie_ipc_rings_sz);

		/* Convert entire pcie_ipc_rings_t structure to Host Endian Order */
		_PCIE_IPC_RINGS_LTOH32(ring_mem_daddr32);
		_PCIE_IPC_RINGS_LTOH32(h2d_wr_daddr32);
		_PCIE_IPC_RINGS_LTOH32(h2d_rd_daddr32);
		_PCIE_IPC_RINGS_LTOH32(d2h_wr_daddr32);
		_PCIE_IPC_RINGS_LTOH32(d2h_rd_daddr32);
		_PCIE_IPC_RINGS_LTOH32(h2d_wr_haddr64.lo);
		_PCIE_IPC_RINGS_LTOH32(h2d_wr_haddr64.hi);
		_PCIE_IPC_RINGS_LTOH32(h2d_rd_haddr64.lo);
		_PCIE_IPC_RINGS_LTOH32(h2d_rd_haddr64.hi);
		_PCIE_IPC_RINGS_LTOH32(d2h_wr_haddr64.lo);
		_PCIE_IPC_RINGS_LTOH32(d2h_wr_haddr64.hi);
		_PCIE_IPC_RINGS_LTOH32(d2h_rd_haddr64.lo);
		_PCIE_IPC_RINGS_LTOH32(h2d_rd_haddr64.hi);
		_PCIE_IPC_RINGS_LTOH16(max_h2d_rings);
		_PCIE_IPC_RINGS_LTOH16(max_d2h_rings);

		if (PCIE_IPC_EXTN_PRESENT(dngl_ipc_revision))
		{
			_PCIE_IPC_RINGS_LTOH16(max_flowrings);
			_PCIE_IPC_RINGS_LTOH16(max_interfaces);
			// wi_formats: accessed as uint8 units
			_PCIE_IPC_RINGS_LTOH16(rxpost_data_buf_len);
			_PCIE_IPC_RINGS_LTOH16(rxcpln_dataoffset);
		}

		if (dngl_ipc_revision <= 5) {
			/* Fillup values for older firmware revisions */
			pcie_ipc_rings->max_d2h_rings  = BCMPCIE_D2H_COMMON_MSGRINGS;
			pcie_ipc_rings->max_flowrings  = pcie_ipc_rings->max_h2d_rings -
				BCMPCIE_H2D_COMMON_MSGRINGS;

			pcie_ipc_rings->max_interfaces = DHD_MAX_IFS;

			pcie_ipc_rings->txpost_format = MSGBUF_WI_LEGACY;
			pcie_ipc_rings->rxpost_format = MSGBUF_WI_LEGACY;
			pcie_ipc_rings->txcpln_format = MSGBUF_WI_LEGACY;
			pcie_ipc_rings->rxcpln_format = MSGBUF_WI_LEGACY;

			pcie_ipc_rings->rxcpln_dataoffset = ~0; /* undefined */
		}

		/* ring_mem array size is only BCMPCIE_COMMON_MSGRINGS + MAX_DHD_TX_FLOWS,
		 * so reduce max_flowrings if we cannot hold all of them.
		 */
		if (pcie_ipc_rings->max_h2d_rings > (BCMPCIE_H2D_COMMON_MSGRINGS +
			MAX_DHD_TX_FLOWS)) {
			uint16 h2drings, flowrings, dbgrings;

			ASSERT(pcie_ipc_rings->max_flowrings <=
				pcie_ipc_rings->max_h2d_rings - BCMPCIE_H2D_COMMON_MSGRINGS);

			/* Only reduce tx flow rings */
			dbgrings = pcie_ipc_rings->max_h2d_rings -
				BCMPCIE_H2D_COMMON_MSGRINGS -
				pcie_ipc_rings->max_flowrings;
			flowrings = MAX_DHD_TX_FLOWS - dbgrings;
			h2drings = pcie_ipc_rings->max_h2d_rings -
				(pcie_ipc_rings->max_flowrings - flowrings);

			DHD_ERROR(("%s: PCIe IPC OVERRIDE WARNING: "
				"DHD reduces h2d_ring::flowrings from max %u::%u to %u::%u\n",
				__FUNCTION__,
				pcie_ipc_rings->max_h2d_rings, pcie_ipc_rings->max_flowrings,
				h2drings, flowrings));

			pcie_ipc_rings->max_h2d_rings = h2drings;
			pcie_ipc_rings->max_flowrings = flowrings;
		}

		DHD_PCIE_IPC(("PCIe IPC RINGS: size %u Ring Mem daddr32 0x%08x\n"
			"\t H2D Indices Array daddr32: WR 0x%08x RD 0x%08x\n"
			"\t D2H Indices Array daddr32: WR 0x%08x RD 0x%08x\n"
			"\t Total: H2D rings %u D2H rings %u flowrings %u interfaces %u\n",
			pcie_ipc_rings_sz, pcie_ipc_rings->ring_mem_daddr32,
			pcie_ipc_rings->h2d_wr_daddr32, pcie_ipc_rings->h2d_rd_daddr32,
			pcie_ipc_rings->d2h_wr_daddr32, pcie_ipc_rings->d2h_rd_daddr32,
			pcie_ipc_rings->max_h2d_rings, pcie_ipc_rings->max_d2h_rings,
			pcie_ipc_rings->max_flowrings, pcie_ipc_rings->max_interfaces));

		// bcm_print_bytes("\tRing: ", (uchar*)pcie_ipc_rings, pcie_ipc_rings_sz);

	} /* read pcie_ipc_rings_t from dongle */

	if (pcie_ipc_daddr32)
		*pcie_ipc_daddr32 = daddr32;

	DHD_PCIE_IPC(("PCIe IPC UPLOAD COMPLETED\n\n"));

	return BCME_OK;

}   /* dhdpcie_bus_read_pcie_ipc */

#ifdef BCMDBG_PCIE
/* Get, Set pcie registers - for use during initialization or debug
 * - equivalent of dhd -i wl0 pciereg <addr> using IOV_PCIEREG
 * Following is based on a 3390 - offset from 0x21080000;
 * Debug Registers of interest: B4, 104, 110,  TLP Header: 11C, 120, 124, 128
 */
#define DHD_PCIE_RC_CFG_PCIE_OFFSET	0x00AC
#define DHD_PCIE_RC_CFG_AER_OFFSET	0x0100
#define DHD_MAX_REG_OFFSET		0x3FFC

static void
dhdpcie_dumpregs_range(dhd_bus_t *bus, const char *name, uint32 offset_st, int num_reg)
{
	uint32 offset_curr = offset_st;
	int i;

	/* collect values to print */
	printk("PCIE%d %s: num_reg=%d \n", bus->dhd->unit, name ? name : "", num_reg);
	for (i = 0; (i < num_reg) && (offset_curr < (uint32)DHD_MAX_REG_OFFSET); i++) {
		printk(" [0x%04X] : %08X\n", offset_curr, dhdpcie_get_regval(bus, offset_curr));
		offset_curr += sizeof(offset_curr);
	}
	printk("\n");
} /* dhdpcie_dumpregs_range */

void
dhdpcie_dumpregs(dhd_bus_t *bus, char *dbgstr)
{
	uint32 offset_st;

	printk("-----Unit:%d dhdpcie_dumpregs %s-------\n", bus->dhd->unit, dbgstr);
	offset_st = (uint32)(DHD_PCIE_RC_CFG_PCIE_OFFSET);
	dhdpcie_dumpregs_range(bus, "RC_CFG_PCIE ", offset_st, 16);
	offset_st = (uint32)(DHD_PCIE_RC_CFG_AER_OFFSET);
	dhdpcie_dumpregs_range(bus, "RC_CFG_AER ", offset_st, 16);
	printk("------------\n");
} /* dhdpcie_dumpregs */
#endif /* BCMDBG_PCIE */

/**
 * Host Dongle PCIe IPC training
 * Read the pcie_ipc_t and pcie_ipc_rings_t from dongle memory and settup bus.
 */
static int
dhdpcie_bus_init_pcie_ipc(dhd_bus_t *bus)
{
	int ret;
	int dma_indx_wr_buf, dma_indx_rd_buf;
	bool use_haddr64 = TRUE;
	uint32 host_physaddrhi = 0U;
#ifdef BCMHWA
	uint32 hwa_caps;
#endif
	uint8 max_rxcpln_rings = 1;

	/* local host pcie_ipc_t structure */
	pcie_ipc_t *pcie_ipc = &bus->pcie_ipc;
	pcie_ipc_rings_t *pcie_ipc_rings = &bus->pcie_ipc_rings;

#ifdef BCMDBG_PCIE
	dhdpcie_dumpregs(bus, "from-dhdpcie_bus_init_pcie_ipc Enter");
	/*  XXX For example set DHD_PCIE_REG_DEVICE_STATUS_CONTROL (0x00B4)
	 * dhdpcie_set_regval(bus, (uint32)DHD_PCIE_REG_DEVICE_STATUS_CONTROL), 0x00102C50);
	 * val = dhdpcie_get_regval(bus, (uint32)(DHD_PCIE_REG_DEVICE_STATUS_CONTROL));
	 */
#endif /* BCMDBG_PCIE */

	if (bus->pcie_ipc_hme != NULL) {
		MFREE(bus->dhd->osh, bus->pcie_ipc_hme, bus->pcie_ipc_hme->size);
		bus->pcie_ipc_hme = NULL;
	}

	DHD_PCIE_IPC(("\nPCIe IPC Host Dongle Training Commences ...\n\n"));
	ret = dhdpcie_bus_read_pcie_ipc(bus, &bus->pcie_ipc_daddr32,
	           &bus->pcie_ipc, &bus->pcie_ipc_rings, &bus->pcie_ipc_hme);

	if (ret < 0) {
		DHD_ERROR(("%s: PCIe IPC FAILURE: error %d\n", __FUNCTION__, ret));
		return ret;
	}

	/* Setup bus layer for IPC */
	bus->pcie_ipc_revision         = PCIE_IPC_REV_GET(pcie_ipc->flags);

	/* Setup the well known dongle addresses in the bus layer */
	ASSERT(bus->pcie_ipc_daddr32  != 0U);
	bus->pcie_ipc_rings_daddr32    = pcie_ipc->rings_daddr32;
	bus->pcie_ipc_ring_mem_daddr32 = pcie_ipc_rings->ring_mem_daddr32;
	bus->pcie_ipc_h2d_mb_daddr32   = pcie_ipc->h2d_mb_daddr32;
	bus->pcie_ipc_d2h_mb_daddr32   = pcie_ipc->d2h_mb_daddr32;
	bus->console_daddr32           = pcie_ipc->console_daddr32;

	DHD_PCIE_IPC(("PCIe IPC REVISION %u Configuration and Features\n",
		bus->pcie_ipc_revision));

	/* Initialize bus layer, process dongle advertized features */
	bus->dhd->d2h_sync_mode = pcie_ipc->flags & PCIE_IPC_FLAGS_D2H_SYNC_MODE_MASK;
	DHD_PCIE_IPC(("\t Config  : D2H Sync Mode: 0x%08x\n",
		bus->dhd->d2h_sync_mode));

	ASSERT(pcie_ipc->dma_rxoffset == 0U);
	dhd_prot_dma_rxoffset(bus->dhd, pcie_ipc->dma_rxoffset);

	/* PCIe IPC Rings supported in H2D and D2H directions */
	bus->max_h2d_rings = pcie_ipc_rings->max_h2d_rings;
	bus->max_d2h_rings = pcie_ipc_rings->max_d2h_rings;
	bus->max_interfaces = pcie_ipc_rings->max_interfaces;
	ASSERT(bus->max_d2h_rings == BCMPCIE_D2H_COMMON_MSGRINGS);
	DHD_PCIE_IPC(("\t Config  : Max Rings H2D %u D2H %u\n",
		bus->max_h2d_rings, bus->max_d2h_rings));

	/* Ring RD and WR index size */
	bus->rw_index_sz = (pcie_ipc->flags & PCIE_IPC_FLAGS_2BYTE_INDICES) ?
		sizeof(uint16) : sizeof(uint32);
	DHD_PCIE_IPC(("\t Config  : Ring Index Size %u bytes\n", bus->rw_index_sz));

	PCIE_HBQD(bus->dhd) = FALSE;
	if (pcie_ipc->dcap2 & PCIE_IPC_DCAP2_PCIE_HBQD) {
		ASSERT(bus->rw_index_sz == sizeof(uint32));
		pcie_ipc->hcap2 |= PCIE_IPC_HCAP2_PCIE_HBQD;
		PCIE_HBQD(bus->dhd) = TRUE;
		DHD_PCIE_IPC(("\t Feature : Host Backup Queue Depth\n"));
	}

	/* Check if host support/enable iDMA/iFRM */
	/* iDMA */
	if (IDMA_CAPABLE(bus) && bus->idma_enabled) {
		if (pcie_ipc->dcap1 & PCIE_IPC_DCAP1_IDMA) {
			bus->dhd->idma_enable = TRUE;
			pcie_ipc->hcap1 |= PCIE_IPC_HCAP1_IDMA;
			DHD_PCIE_IPC(("\t Feature : Host iDMA\n"));
		} else {
			bus->dhd->idma_enable = FALSE;
		}
	} else {
		bus->dhd->idma_enable = FALSE;
	}
	bus->dhd->idma_inited = TRUE;

	/* Dongle firmware is built with SQS, requiring per TID ucast flowrings */
	if (pcie_ipc->dcap1 & PCIE_IPC_DCAP1_FLOWRING_TID) {
		/* See dhd_flow_rings_init() : where DHD_FLOW_PRIO_TID_MAP is applied */
		pcie_ipc->hcap1 |= PCIE_IPC_HCAP1_FLOWRING_TID; /* ACK fw */
		DHD_PCIE_IPC(("\t Config  : Flowring per TID\n"));
	}

	/* Does the FW support DMA'ing r/w indices */
	if (pcie_ipc->flags & PCIE_IPC_FLAGS_DMA_INDEX) {
		/* If dongle requests DMA INDEX support, DHD must support it! */
		bus->dhd->dma_d2h_ring_upd_support = TRUE;
		bus->dhd->dma_h2d_ring_upd_support = TRUE;
		DHD_PCIE_IPC(("\t Feature : Dongle DMA Indices H2D %d D2H %d\n",
			(DMA_INDX_ENAB(bus->dhd->dma_h2d_ring_upd_support) ? 1 : 0),
			(DMA_INDX_ENAB(bus->dhd->dma_d2h_ring_upd_support) ? 1 : 0)));
	}
	else if (DMA_INDX_ENAB(bus->dhd->dma_d2h_ring_upd_support) ||
	         DMA_INDX_ENAB(bus->dhd->dma_h2d_ring_upd_support)) {
#ifdef BCM_INDX_DMA
		DHD_ERROR(("%s: PCIe IPC SUPPORT FAILURE: "
			"FW does not support DMAing indices\n" __FUNCTION__));
		return BCME_ERROR;
#endif
		bus->dhd->dma_d2h_ring_upd_support = FALSE;
		bus->dhd->dma_h2d_ring_upd_support = FALSE;
		DHD_ERROR(("%s: PCIe IPC SUPPORT WARNING: "
			"Host supports DMAing indices but FW does not\n", __FUNCTION__));
	}

	/* Allocate memory and notify FW */
	if (DMA_INDX_ENAB(bus->dhd->dma_h2d_ring_upd_support) || IDMA_ENAB(bus->dhd))
	{
		dma_indx_wr_buf = dhd_prot_dma_indx_init(bus->dhd, bus->rw_index_sz,
			H2D_DMA_INDX_WR_BUF, bus->max_h2d_rings);
		dma_indx_rd_buf = dhd_prot_dma_indx_init(bus->dhd, bus->rw_index_sz,
			D2H_DMA_INDX_RD_BUF, bus->max_d2h_rings);

		if ((dma_indx_wr_buf != BCME_OK) || (dma_indx_rd_buf != BCME_OK)) {
			bus->dhd->dma_h2d_ring_upd_support = FALSE;
			bus->dhd->idma_enable = FALSE;
			DHD_ERROR(("%s: PCIe IPC MEMORY WARNING: "
				"H2D WR %u D2H RD %u Indices Array. "
				"Host will xfer Indices to Dongle\n",
				__FUNCTION__, dma_indx_wr_buf, dma_indx_rd_buf));
		}
	}

	if (DMA_INDX_ENAB(bus->dhd->dma_d2h_ring_upd_support))
	{
		dma_indx_wr_buf = dhd_prot_dma_indx_init(bus->dhd, bus->rw_index_sz,
			D2H_DMA_INDX_WR_BUF, bus->max_d2h_rings);
		dma_indx_rd_buf = dhd_prot_dma_indx_init(bus->dhd, bus->rw_index_sz,
			H2D_DMA_INDX_RD_BUF, bus->max_h2d_rings);

		if ((dma_indx_wr_buf != BCME_OK) || (dma_indx_rd_buf != BCME_OK)) {
			bus->dhd->dma_d2h_ring_upd_support = FALSE;
			DHD_ERROR(("%s: PCIe IPC MEMORY WARNING: "
				"D2H WR %u H2D RD %u Indices Array. "
				"Host will xfer Indices from Dongle\n",
				__FUNCTION__, dma_indx_wr_buf, dma_indx_rd_buf));
		}
	}

#if defined(BCM_DHDHDR)
	bus->dhd->dhdhdr_support = (pcie_ipc->flags & PCIE_IPC_FLAGS_DHDHDR) ?
		TRUE : FALSE;
	DHD_PCIE_IPC(("\t Feature : SFH Insertion %u\n", bus->dhd->dhdhdr_support));
#endif /* BCM_ROUTER_DHD && BCM_DHDHDR */

	/* Does the firmware support csi monitor? */
	if (pcie_ipc->dcap2 & PCIE_IPC_DCAP2_CSI_MONITOR) {
		bus->dhd->csi_monitor = TRUE;
		/* Enable CSI monitor support in firmware if supported */
		pcie_ipc->hcap2 |= PCIE_IPC_HCAP2_CSI_MONITOR;
		DHD_PCIE_IPC(("\t Feature : CSI Monitor\n"));
	} else {
		bus->dhd->csi_monitor = FALSE;
		DHD_PCIE_IPC(("\t Feature : CSI Monitor disabled\n"));
	}

	/* Does the firmware support fast delete ring? */
	if (pcie_ipc->dcap1 & PCIE_IPC_DCAP1_FAST_DELETE_RING) {
		bus->dhd->fast_delete_ring_support = TRUE;
		/* Enable fast delete ring in firmware if supported */
		pcie_ipc->hcap1 |= PCIE_IPC_HCAP1_FAST_DELETE_RING;
		DHD_PCIE_IPC(("\t Feature : Fast Delete Ring\n"));
	} else {
		bus->dhd->fast_delete_ring_support = FALSE;
		DHD_PCIE_IPC(("\t Feature : Fast Delete Ring disabled\n"));
	}

	/* Is DHD configured to enable fast delete ring? */
	if (bus->dhd->fast_delete_ring_support == TRUE) {
		int ffrd = 0;

#if defined(BCM_DHD_DNGL_FFRD)
		char *var;

		var = getvar(NULL, "dhd_dngl_ffrd");
		if (var && bcm_strtoul(var, NULL, 0) == 0) {
			ffrd = 0;
		} else {
			ffrd = 1;
		}
#endif /* BCM_DHD_DNGL_FFRD */

		if (ffrd == 0) {
			bus->dhd->fast_delete_ring_support = FALSE;
			/* Disable fast delete ring in firmware if supported */
			pcie_ipc->hcap1 &= (~PCIE_IPC_HCAP1_FAST_DELETE_RING);
			DHD_PCIE_IPC(("\t Feature : Fast Delete Ring disabled\n"));
		}
	}

#if defined(BCM_DHD_RUNNER)
	/* Does the runner support fast delete ring? */
	if (bus->dhd->fast_delete_ring_support == TRUE) {
		uint32 rcap1 = PCIE_IPC_HCAP1_FAST_DELETE_RING;
		int rc;

		/* Notify and get back dor cap1 for fast delete ring */
		rc = dhd_runner_notify(bus->dhd->runner_hlp, H2R_PCIE_IPC_CAP1_NOTIF,
			(unsigned long)PCIE_IPC_HCAP1_FAST_DELETE_RING, (unsigned long)&rcap1);
		if ((rc != BCME_OK) || (rcap1 != PCIE_IPC_HCAP1_FAST_DELETE_RING))  {
			bus->dhd->fast_delete_ring_support = FALSE;
			/* Disable fast delete ring in firmware if supported */
			pcie_ipc->hcap1 &= (~PCIE_IPC_HCAP1_FAST_DELETE_RING);
			DHD_PCIE_IPC(("\t Feature : Fast Delete Ring disabled\n"));
		}
	}

	/* Does runner support Host Backup Queue Depth? */
	if (PCIE_HBQD(bus->dhd) == TRUE) {
#if defined(H2R_PCIE_IPC_CAP_HBQD)
		uint32 rcap = H2R_PCIE_IPC_CAP_HBQD;
		int rc;

		/* Notify and get back dor cap for hbqd */
		rc = dhd_runner_notify(bus->dhd->runner_hlp, H2R_PCIE_IPC_CAP_NOTIF,
			(unsigned long)H2R_PCIE_IPC_CAP_HBQD, (unsigned long)&rcap);

		/* hbqd can not be negotiated, fail if not supported by runner */
		if ((rc != BCME_OK) || (!(rcap & H2R_PCIE_IPC_CAP_HBQD)))  {
			DHD_PCIE_IPC(("\t Feature : Host Backup Queue not supported\n"));
			return BCME_UNSUPPORTED;
		}
#else /* !H2R_PCIE_IPC_CAP_HBQD */
		DHD_PCIE_IPC(("\t Feature : Host Backup Queue not supported\n"));
		return BCME_UNSUPPORTED;
#endif /* !H2R_PCIE_IPC_CAP_HBQD */
	}
#endif /* BCM_DHD_RUNNER */

	/* read pcie_ipc_ring_mem_t from shared area and store in host variables */
	dhdpcie_bus_init_pcie_ipc_rings(bus);

	/* Process extended PCIE IPC using prefilled defaults for older fw */

	/* Advertize Host IPC Version */
	PCIE_IPC_REV_SET(pcie_ipc->hcap1, PCIE_IPC_REVISION);

	/* Dongle's RxPost, TxPost, RxCpln and TxCpln WI formats */

#ifdef BCM_ROUTER_DHD
	/* All Routers: haddr64.hi = 0 even on 64bit hosts */
	host_physaddrhi = 0U;
	use_haddr64 = FALSE;
#if defined(BCM_DHD_RUNNER) && defined(BCMDMA64OSL)
	{
		dhd_helper_status_t rnr_status;
		int rc;

		/*
		 * Runner Offload does not support 64bit WI for Rx path.
		 * DHD can not support 32bit WI in Rx path (skb data could be 33bit)
		 * Use 64bit address only when RX runner offload is disabled
		 */
		rc = dhd_runner_do_iovar(bus->dhd->runner_hlp, DHD_RNR_IOVAR_STATUS,
				FALSE, (char*)&rnr_status, sizeof(rnr_status));
		if ((rc == BCME_OK) && (rnr_status.en_features.rxoffl == 0)) {
			use_haddr64 = TRUE;
		}
	}
#endif /* BCM_DHD_RUNNER && BCMDMA64OSL */
#else /* ! BCM_ROUTER_DHD */
	if (sizeof(void*) == 4) {
	/* For All ARM STB Platform considering the 8 byte DMA address */
#if defined(STBLINUX)
		use_haddr64 = TRUE;
		host_physaddrhi = ~0U;
#else
		use_haddr64 = FALSE;
		host_physaddrhi = 0U;
#endif /* STBLINUX */
	} else {
		/* XXX
		 * Selectively exclude 64bit hosts with fixed pkt databuf::hiaddr
		 * 64bit hosts (including STA mode BU Linux PCs) may not use the entire
		 * 64bit address space (i.e. fixed physaddrhi = 0x0).
		 */
		use_haddr64 = TRUE;
		host_physaddrhi = ~0U;
		DHD_PCIE_IPC(("\t Config  : Host Limits BurstLen\n"));
		pcie_ipc->hcap1 |= PCIE_IPC_HCAP1_LIMIT_BL;
	}
#endif /* ! BCM_ROUTER_DHD */

	DHD_PCIE_IPC(("\t Config  : WI Formats TxP %u RxP %u TxC %u RxC %u\n",
		pcie_ipc_rings->txpost_format, pcie_ipc_rings->rxpost_format,
		pcie_ipc_rings->txcpln_format, pcie_ipc_rings->rxcpln_format));

	/* CWI and ACWI formats do not include message level d2h sync */
	if (pcie_ipc->dcap1 & PCIE_IPC_DCAP1_ACWI) {
		if ((bus->dhd->d2h_sync_mode & PCIE_IPC_FLAGS_D2H_SYNC_MODE_MASK) != 0) {
			ASSERT((pcie_ipc_rings->txcpln_format == MSGBUF_WI_LEGACY) &&
			(pcie_ipc_rings->rxcpln_format == MSGBUF_WI_LEGACY));
		}
	}

#ifdef BCMHWA
	/* Host acknowledges index update through IDMA */
	hwa_caps = (PCIE_IPC_DCAP1_HWA_RXPOST_IDMA | PCIE_IPC_DCAP1_HWA_TXCPL_IDMA |
		PCIE_IPC_DCAP1_HWA_RXCPL_IDMA | PCIE_IPC_DCAP1_HWA_TXPOST);
	if (pcie_ipc->dcap1 & hwa_caps) {
		hwa_caps = 0;
		if (pcie_ipc->dcap1 & PCIE_IPC_DCAP1_HWA_RXPOST_IDMA) {
			hwa_caps |= PCIE_IPC_HCAP1_HWA_RXPOST_IDMA;
			DHD_PCIE_IPC(("\t Feature : HWA iDMA RxPost\n"));
		}
		if (pcie_ipc->dcap1 & PCIE_IPC_DCAP1_HWA_TXCPL_IDMA) {
			hwa_caps |= PCIE_IPC_HCAP1_HWA_TXCPL_IDMA;
			DHD_PCIE_IPC(("\t Feature : HWA iDMA TxCpl\n"));
		}
		if (pcie_ipc->dcap1 & PCIE_IPC_DCAP1_HWA_RXCPL_IDMA) {
			hwa_caps |= PCIE_IPC_HCAP1_HWA_RXCPL_IDMA;
			DHD_PCIE_IPC(("\t Feature : HWA iDMA RxCpl\n"));
		}
		if (pcie_ipc->dcap1 & PCIE_IPC_DCAP1_HWA_TXPOST) {
			hwa_caps |= PCIE_IPC_HCAP1_HWA_TXPOST;
			DHD_PCIE_IPC(("\t Feature : HWA TxPost\n"));
		}
		if (pcie_ipc->dcap1 & PCIE_IPC_DCAP1_HWA_RXCPL4) {
			hwa_caps |= PCIE_IPC_HCAP1_HWA_RXCPL4;
			max_rxcpln_rings = 4;
			DHD_PCIE_IPC(("\t Config  : HWA RxCpl rings %u\n",
				max_rxcpln_rings));
		}
		pcie_ipc->hcap1 |= hwa_caps;
		dhd_prot_set_hwa_caps(bus->dhd, hwa_caps);
	}
#endif /* BCMHWA */

	if (dhd_prot_preinit(bus->dhd, use_haddr64, host_physaddrhi,
	        pcie_ipc_rings->rxcpln_dataoffset, max_rxcpln_rings,
	        pcie_ipc_rings->txpost_format, pcie_ipc_rings->rxpost_format,
	        pcie_ipc_rings->txcpln_format, pcie_ipc_rings->rxcpln_format,
	        pcie_ipc->flags))
	{
		DHD_ERROR(("%s: PCIe IPC FEATURE FAILURE: "
			"Host not capable of ACWI or CWI formats\n", __FUNCTION__));
		ASSERT(0);
		return BCME_ERROR;
	}

#ifdef BCMHWA
	/* Setup HWA attribute for RxPost, TxCpl and RxCpl rings */
	dhd_prot_set_hwa_attributes(bus->dhd);
#endif

	/* Host acknowledges WI formats requested */
	if (pcie_ipc->dcap1 & PCIE_IPC_DCAP1_ACWI) {
		pcie_ipc->hcap1 |= PCIE_IPC_HCAP1_ACWI;
	}

	if (use_haddr64) { /* When use_haddr64 is FALSE, ACWI ==> CWI */
		pcie_ipc->hcap1 |= PCIE_IPC_HCAP1_ADDR64;
	}
	pcie_ipc->hcap1 |= PCIE_IPC_HCAP1_HW_COHERENCY;

	pcie_ipc->host_physaddrhi = host_physaddrhi;

	pcie_ipc_rings->rxpost_data_buf_len = DHD_RX_PKT_BUFSZ - DHD_RX_PKT_ALIGN;
#ifdef BCM_ROUTER_DHD
	/* Routers reduce the posted buffer by an extra headroom space */
	pcie_ipc_rings->rxpost_data_buf_len -= BCMEXTRAHDROOM;
#endif

	DHD_PCIE_IPC(("\t Config  : RxPost Data buffer length %u\n",
		pcie_ipc_rings->rxpost_data_buf_len));
	DHD_PCIE_IPC(("\t Config  : Host PhysAddr hi32 %u\n",
		pcie_ipc->host_physaddrhi));

	/* Initialize Host Memory Extension service */
	dhd_prot_hme_reset(bus->dhd);
	if (pcie_ipc->dcap1 & PCIE_IPC_DCAP1_HOST_MEM_EXTN) {
		/**
		 * dhd_prot_hme_init() allocates all HME segments and records their
		 * physical addresses into a table of LE-formatted haddr64_t.
		 */
		ASSERT(bus->pcie_ipc_hme != (pcie_ipc_hme_t*)NULL);
		ret = dhd_prot_hme_init(bus->dhd, bus->pcie_ipc_hme->users);
		ASSERT(ret == BCME_OK); /* Dongle cannot exist without HME sections */
		/* All HME segments successfully allocated */

		/* Acknowledge firmware of HME service capability in DHD */
		pcie_ipc->hcap1 |= PCIE_IPC_HCAP1_HOST_MEM_EXTN;

		/* Later in dhd_prot_init(),
		 * the address of the table of HME physical addresses and the sum
		 * total length of all HME regions will be transferred to dongle's
		 * pcie_ipc fields host_mem_haddr64 and host_mem_len respectively,
		 * using dhd_bus_cmn_writeshared().
		 */
	}

	if (PCIE_IPC_EXTN_PRESENT(bus->pcie_ipc_revision)) {
		/* Write out to dongle's pcie_ipc_t host information */
		dhd_bus_cmn_writeshared(bus, &pcie_ipc->hcap1, IPC_HOST_CAP1, 0);
		dhd_bus_cmn_writeshared(bus, &pcie_ipc->hcap2, IPC_HOST_CAP2, 0);

		dhd_bus_cmn_writeshared(bus, &pcie_ipc->host_physaddrhi,
			IPC_HOST_PHYSADDRHI, 0);

		dhd_bus_cmn_writeshared(bus, &pcie_ipc_rings->rxpost_data_buf_len,
			IPC_HOST_RXBUFLEN, 0);
	}

	DHD_PCIE_IPC(("PCIe IPC Host Dongle Training Completed!\n\n"));

#ifdef BCMDBG_PCIE
	dhdpcie_dumpregs(bus, "from-dhdpcie_bus_init_pcie_ipc Return");
#endif /* BCMDBG_PCIE */

	return BCME_OK;

} /* dhdpcie_bus_init_pcie_ipc */

/** Initialize the bus rings contexts based on PCIe IPC rings structure */
static void
dhdpcie_bus_init_pcie_ipc_rings(dhd_bus_t *bus)
{
	uint16 i = 0;
	uint16 j = 0;
	daddr32_t daddr32;
	daddr32_t d2h_wr_daddr32, d2h_rd_daddr32, h2d_wr_daddr32, h2d_rd_daddr32;
	pcie_ipc_rings_t *pcie_ipc_rings = &bus->pcie_ipc_rings;

	/* pcie_ipc_ring_mem_t array is indexed by ringid */

	{
		/* ring_mem_daddr32 points to array of pcie_ipc_ring_mem_t */
		daddr32 = pcie_ipc_rings->ring_mem_daddr32;

		/* Determine address of each pcie_ipc_ring_mem_t in dongle memory */
		for (i = 0; i <= BCMPCIE_COMMON_MSGRING_MAX_ID; i++) {
			bus->ring_mem[i].daddr32 = daddr32;
			daddr32  = daddr32  + sizeof(pcie_ipc_ring_mem_t);
			DHD_INFO(("ring id %d ring mem addr 0x%04x \n",
				i, bus->ring_mem[i].daddr32));
		}
	}

	{
		/* Compute per ring's address of RD/WR indices in dongle memory */
		d2h_wr_daddr32 = pcie_ipc_rings->d2h_wr_daddr32;
		d2h_rd_daddr32 = pcie_ipc_rings->d2h_rd_daddr32;
		h2d_wr_daddr32 = pcie_ipc_rings->h2d_wr_daddr32;
		h2d_rd_daddr32 = pcie_ipc_rings->h2d_rd_daddr32;

		/* Populate address of each H2D common ring's RD/WR index */
		for (i = 0; i < BCMPCIE_H2D_COMMON_MSGRINGS; i++) {
			bus->ring_mem[i].wr_daddr32 = h2d_wr_daddr32;
			bus->ring_mem[i].rd_daddr32 = h2d_rd_daddr32;

			h2d_wr_daddr32 = h2d_wr_daddr32 + bus->rw_index_sz;
			h2d_rd_daddr32 = h2d_rd_daddr32 + bus->rw_index_sz;

			DHD_INFO(("h2d w/r : idx %d daddr wr %x rd %x \n", i,
				bus->ring_mem[i].wr_daddr32, bus->ring_mem[i].rd_daddr32));
		}

		/* Populate address of each D2H common ring's RD/WR index */
		for (j = 0; j < BCMPCIE_D2H_COMMON_MSGRINGS; j++, i++) {
			bus->ring_mem[i].wr_daddr32 = d2h_wr_daddr32;
			bus->ring_mem[i].rd_daddr32 = d2h_rd_daddr32;

			d2h_wr_daddr32 = d2h_wr_daddr32 + bus->rw_index_sz;
			d2h_rd_daddr32 = d2h_rd_daddr32 + bus->rw_index_sz;

			DHD_INFO(("d2h w/r : idx %d daddr wr %x rd %x \n", i,
				bus->ring_mem[i].wr_daddr32, bus->ring_mem[i].rd_daddr32));
		}

		/* Populate address of each H2D TxPost Flowring's RD/WR index */
		for (j = 0; j < (bus->max_h2d_rings - BCMPCIE_H2D_COMMON_MSGRINGS);
			i++, j++)
		{
			bus->ring_mem[i].wr_daddr32 = h2d_wr_daddr32;
			bus->ring_mem[i].rd_daddr32 = h2d_rd_daddr32;

			h2d_wr_daddr32 = h2d_wr_daddr32 + bus->rw_index_sz;
			h2d_rd_daddr32 = h2d_rd_daddr32 + bus->rw_index_sz;

			DHD_INFO(("FLOW Rings h2d w/r : idx %d daddr wr %x rd %x \n", i,
				bus->ring_mem[i].wr_daddr32, bus->ring_mem[i].rd_daddr32));
		}
	}

} /* dhdpcie_bus_init_pcie_ipc_rings */

/**
 * Initialize bus module: prepare for communication with the dongle. Called after downloading
 * firmware into the dongle.
 */
int
dhd_bus_init(dhd_pub_t *dhdp, bool enforce_mutex)
{
	dhd_bus_t *bus = dhdp->bus;
	int  ret = 0;

	DHD_TRACE(("%s: Enter\n", __FUNCTION__));

	ASSERT(bus->dhd);
	if (!bus->dhd)
		return 0;

	/* Make sure we're talking to the core. */
	bus->reg = si_setcore(bus->sih, PCIE2_CORE_ID, 0);
	ASSERT(bus->reg != NULL);

	/* before opening up bus for data transfer, check if shared are is intact */
	ret = dhdpcie_bus_init_pcie_ipc(bus);
	if (ret < 0) {
		DHD_ERROR(("%s: PCIe IPC INITIALIZATION FAILURE\n", __FUNCTION__));
		return ret;
	}

	/* Make sure we're talking to the core. */
	bus->reg = si_setcore(bus->sih, PCIE2_CORE_ID, 0);
	ASSERT(bus->reg != NULL);

	/* Set bus state according to enable result */
	dhdp->busstate = DHD_BUS_DATA;

	if (!dhd_download_fw_on_driverload)
		dhd_dpc_enable(bus->dhd);

	/* Enable the interrupt after device is up */
	dhdpcie_bus_intr_enable(bus);

	/* bcmsdh_intr_unmask(bus->sdh); */

	return ret;
} /* dhd_bus_init */

/**
 * Zeros out the last 4 bytes in dongle memory. Dongle will place the address
 * of the pcie_ipc_t structure in this location.
 */
static void
dhdpcie_bus_clear_pcie_ipc_addr(dhd_bus_t *bus)
{
	daddr32_t last_word_daddr32;
	uint32 val32 = 0U;
	last_word_daddr32 = bus->dongle_ram_base + bus->ramsize - 4;
	dhdpcie_bus_membytes(bus, PCIE_SYSMEM_WRITE, last_word_daddr32,
		(uint8 *)&val32, sizeof(val32));
}

bool
dhdpcie_chipmatch(uint16 vendor, uint16 device)
{
	if (vendor != PCI_VENDOR_ID_BROADCOM) {
		DHD_INFO(("%s: Unsupported vendor %x device %x\n", __FUNCTION__,
			vendor, device));
		return (-ENODEV);
	}

	if ((device == BCM4350_D11AC_ID) || (device == BCM4350_D11AC2G_ID) ||
		(device == BCM4350_D11AC5G_ID) || (device == BCM4350_CHIP_ID) ||
		(device == BCM43569_CHIP_ID))
		return 0;

	if ((device == BCM4354_D11AC_ID) || (device == BCM4354_D11AC2G_ID) ||
		(device == BCM4354_D11AC5G_ID) || (device == BCM4354_CHIP_ID))
		return 0;

	if ((device == BCM4356_D11AC_ID) || (device == BCM4356_D11AC2G_ID) ||
		(device == BCM4356_D11AC5G_ID) || (device == BCM4356_CHIP_ID))
		return 0;

	if ((device == BCM4345_D11AC_ID) || (device == BCM4345_D11AC2G_ID) ||
		(device == BCM4345_D11AC5G_ID) || BCM4345_CHIP(device))
		return 0;

	if ((device == BCM4335_D11AC_ID) || (device == BCM4335_D11AC2G_ID) ||
		(device == BCM4335_D11AC5G_ID) || (device == BCM4335_CHIP_ID))
		return 0;

#if defined(STB) && (!defined(STBAP) || !defined(BCM_ROUTER_DHD))
	if ((device == BCM43602_D11AC_ID) || (device == BCM43602_D11AC2G_ID) ||
			(device == BCM43602_D11AC5G_ID) || (device == BCM43602_CHIP_ID))
		return 0;
#endif

	if ((device == BCM43569_D11AC_ID) || (device == BCM43569_D11AC2G_ID) ||
		(device == BCM43569_D11AC5G_ID) || (device == BCM43569_CHIP_ID))
		return 0;

	/* XXX: For 4358, BCM4358_CHIP_ID is not checked intentionally as
	 * this is not a real chip id, but propagated from the OTP.
	 */
	if ((device == BCM4358_D11AC_ID) || (device == BCM4358_D11AC2G_ID) ||
		(device == BCM4358_D11AC5G_ID))
		return 0;

	if ((device == BCM4349_D11AC_ID) || (device == BCM4349_D11AC2G_ID) ||
		(device == BCM4349_D11AC5G_ID) || (device == BCM4349_CHIP_ID))
		return 0;

	if ((device == BCM4355_D11AC_ID) || (device == BCM4355_D11AC2G_ID) ||
		(device == BCM4355_D11AC5G_ID) || (device == BCM4355_CHIP_ID))
		return 0;

	/* XXX: BCM4359_CHIP_ID is not checked intentionally as this is
	 * not a real chip id, but propogated from the OTP.
	 */
	if ((device == BCM4359_D11AC_ID) || (device == BCM4359_D11AC2G_ID) ||
		(device == BCM4359_D11AC5G_ID))
		return 0;

#ifdef UNRELEASEDCHIP
	if ((device == BCM4364_D11AC_ID) || (device == BCM4364_D11AC2G_ID) ||
		(device == BCM4364_D11AC5G_ID) || (device == BCM4364_CHIP_ID))
		return 0;

	if ((device == BCM4347_D11AC_ID) || (device == BCM4347_D11AC2G_ID) ||
		(device == BCM4347_D11AC5G_ID) || (device == BCM4347_CHIP_ID))
		return 0;
#endif

	if ((device == BCM4365_D11AC_ID) || (device == BCM4365_D11AC2G_ID) ||
		(device == BCM4365_D11AC5G_ID) || (device == BCM4365_CHIP_ID))
		return 0;

	if ((device == BCM4366_D11AC_ID) || (device == BCM4366_D11AC2G_ID) ||
		(device == BCM4366_D11AC5G_ID) || (device == BCM4366_CHIP_ID) ||
		(device == BCM43664_CHIP_ID) || (device == BCM43666_CHIP_ID))
		return 0;

	if ((device == BCM43684_D11AX_ID) || (device == BCM43684_D11AX2G_ID) ||
		(device == BCM43684_D11AX5G_ID) || (device == BCM43684_D11AX6G_ID) ||
		(device == BCM43684_D11AX2G6G_ID) || (device == BCM43684_D11AX5G6G_ID) ||
		(device == BCM43684_D11AX2G5G6G_ID) ||
		(device == BCM43684_CHIP_ID) ||	(device == BCM43694_CHIP_ID) ||
		(device == BCM4363_CHIP_ID))
		return 0;

	if ((device == BCM6715_D11AX_ID) || (device == BCM6715_D11AX2G_ID) ||
		(device == BCM6715_D11AX5G_ID) || (device == BCM6715_D11AX6G_ID) ||
		(device == BCM6715_D11AX2G6G_ID) || (device == BCM6715_D11AX5G6G_ID) ||
		(device == BCM6715_D11AX2G5G6G_ID) ||
		(device == BCM6715_CHIP_ID) || (device == BCM43794_CHIP_ID))
		return 0;

	DHD_INFO(("%s: Unsupported vendor %x device %x\n", __FUNCTION__, vendor, device));
	return (-ENODEV);
} /* dhdpcie_chipmatch */

/**
 * Name:  dhdpcie_cc_nvmshadow
 *
 * Description:
 * A shadow of OTP/SPROM exists in ChipCommon Region
 * betw. 0x800 and 0xBFF (Backplane Addr. 0x1800_0800 and 0x1800_0BFF).
 * Strapping option (SPROM vs. OTP), presence of OTP/SPROM and its size
 * can also be read from ChipCommon Registers.
 */
static int
dhdpcie_cc_nvmshadow(dhd_bus_t *bus, struct bcmstrbuf *b)
{
	uint16 dump_offset = 0;
	uint32 dump_size = 0, otp_size = 0, sprom_size = 0;

	/* Table for 65nm OTP Size (in bits) */
	int  otp_size_65nm[8] = {0, 2048, 4096, 8192, 4096, 6144, 512, 1024};

	volatile uint16 *nvm_shadow;

	uint cur_coreid;
	uint chipc_corerev;
	chipcregs_t *chipcregs;

	/* Save the current core */
	cur_coreid = si_coreid(bus->sih);
	/* Switch to ChipC */
	chipcregs = (chipcregs_t *)si_setcore(bus->sih, CC_CORE_ID, 0);
	ASSERT(chipcregs != NULL);

	chipc_corerev = si_corerev(bus->sih);

	/* Check ChipcommonCore Rev */
	if (chipc_corerev < 44) {
		DHD_ERROR(("%s: ChipcommonCore Rev %d < 44\n", __FUNCTION__, chipc_corerev));
		return BCME_UNSUPPORTED;
	}

	/* Check ChipID */
	if (((uint16)bus->sih->chip != BCM4350_CHIP_ID) && !BCM4345_CHIP((uint16)bus->sih->chip)) {
		DHD_ERROR(("%s: cc_nvmdump cmd. supported for 4350/4345 only\n",
			__FUNCTION__));
		return BCME_UNSUPPORTED;
	}

	/* Check if SRC_PRESENT in SpromCtrl(0x190 in ChipCommon Regs) is set */
	if (chipcregs->sromcontrol & SRC_PRESENT) {
		/* SPROM Size: 1Kbits (0x0), 4Kbits (0x1), 16Kbits(0x2) */
		sprom_size = (1 << (2 * ((chipcregs->sromcontrol & SRC_SIZE_MASK)
					>> SRC_SIZE_SHIFT))) * 1024;
		bcm_bprintf(b, "\nSPROM Present (Size %d bits)\n", sprom_size);
	}

	/* XXX Check if OTP exists. 2 possible approaches:
	 * 1) Check if OtpPresent in SpromCtrl (0x190 in ChipCommon Regs) is set OR
	 * 2) Check if OtpSize > 0
	 */
	if (chipcregs->sromcontrol & SRC_OTPPRESENT) {
		bcm_bprintf(b, "\nOTP Present");

		if (((chipcregs->otplayout & OTPL_WRAP_TYPE_MASK) >> OTPL_WRAP_TYPE_SHIFT)
			== OTPL_WRAP_TYPE_40NM) {
			/* 40nm OTP: Size = (OtpSize + 1) * 1024 bits */
			otp_size =  (((chipcregs->capabilities & CC_CAP_OTPSIZE)
				        >> CC_CAP_OTPSIZE_SHIFT) + 1) * 1024;
			bcm_bprintf(b, "(Size %d bits)\n", otp_size);
		} else {
			/* This part is untested since newer chips have 40nm OTP */
			otp_size = otp_size_65nm[(chipcregs->capabilities & CC_CAP_OTPSIZE)
				        >> CC_CAP_OTPSIZE_SHIFT];
			bcm_bprintf(b, "(Size %d bits)\n", otp_size);
			DHD_INFO(("%s: 65nm/130nm OTP Size not tested. \n",
				__FUNCTION__));
		}
	}

	if (((chipcregs->sromcontrol & SRC_PRESENT) == 0) &&
		((chipcregs->capabilities & CC_CAP_OTPSIZE) == 0)) {
		DHD_ERROR(("%s: SPROM and OTP could not be found \n",
			__FUNCTION__));
		return BCME_NOTFOUND;
	}

	/* Check the strapping option in SpromCtrl: Set = OTP otherwise SPROM */
	if ((chipcregs->sromcontrol & SRC_OTPSEL) &&
		(chipcregs->sromcontrol & SRC_OTPPRESENT)) {

		bcm_bprintf(b, "OTP Strap selected.\n"
		               "\nOTP Shadow in ChipCommon:\n");

		dump_size = otp_size / 16 ; /* 16bit words */

	} else if (((chipcregs->sromcontrol & SRC_OTPSEL) == 0) &&
		(chipcregs->sromcontrol & SRC_PRESENT)) {

		bcm_bprintf(b, "SPROM Strap selected\n"
				"\nSPROM Shadow in ChipCommon:\n");

		/* If SPROM > 8K only 8Kbits is mapped to ChipCommon (0x800 - 0xBFF) */
		/* dump_size in 16bit words */
		dump_size = sprom_size > 8 ? (8 * 1024) / 16 : sprom_size / 16;
	} else {
		DHD_ERROR(("%s: NVM Shadow does not exist in ChipCommon\n",
			__FUNCTION__));
		return BCME_NOTFOUND;
	}

	if (bus->regs == NULL) {
		DHD_ERROR(("ChipCommon Regs. not initialized\n"));
		return BCME_NOTREADY;
	} else {
	    bcm_bprintf(b, "\n OffSet:");

	    /* Point to the SPROM/OTP shadow in ChipCommon */
	    nvm_shadow = chipcregs->sromotp;

	   /*
	    * Read 16 bits / iteration.
	    * dump_size & dump_offset in 16-bit words
	    */
	    while (dump_offset < dump_size) {
		if (dump_offset % 2 == 0)
			/* Print the offset in the shadow space in Bytes */
			bcm_bprintf(b, "\n 0x%04x", dump_offset * 2);

		bcm_bprintf(b, "\t0x%04x", *(nvm_shadow + dump_offset));
		dump_offset += 0x1;
	    }
	}

	/* Switch back to the original core */
	si_setcore(bus->sih, cur_coreid, 0);

	return BCME_OK;
} /* dhdpcie_cc_nvmshadow */

/** Flow rings are dynamically created and destroyed */
void
dhd_bus_clean_flow_ring(dhd_bus_t *bus, void *node)
{
	void *pkt;
	flow_queue_t *queue;
	flow_ring_node_t *flow_ring_node = (flow_ring_node_t *)node;
	unsigned long flags;

	queue = &flow_ring_node->queue;

#ifdef DHDTCPACK_SUPPRESS
	/* Clean tcp_ack_info_tbl in order to prevent access to flushed pkt,
	 * when there is a newly coming packet from network stack.
	 */
	dhd_tcpack_info_tbl_clean(bus->dhd);
#endif /* DHDTCPACK_SUPPRESS */

	/* clean up BUS level info */
	DHD_FLOWRING_LOCK(flow_ring_node->lock, flags);

	/* Flush all pending packets in the queue, if any */
	while ((pkt = dhd_flow_queue_dequeue(bus->dhd, queue)) != NULL) {
		PKTFREE(bus->dhd->osh, pkt, TRUE);
	}
	ASSERT(DHD_FLOW_QUEUE_EMPTY(queue));

	/* Reinitialise flowring's queue */
	dhd_flow_queue_reinit(bus->dhd, queue, FLOW_RING_QUEUE_THRESHOLD);

	flow_ring_node->status = FLOW_RING_STATUS_CLOSED;
	flow_ring_node->active = FALSE;

#ifdef DHD_IFE
	dhd_ife_flowring_delete(bus->dhd, flow_ring_node);
#endif /* DHD_IFE */

	if (!dll_empty(&flow_ring_node->list)) {
	    dll_delete(&flow_ring_node->list);
	    dll_init(&flow_ring_node->list);
	}

	DHD_FLOWRING_UNLOCK(flow_ring_node->lock, flags);

	/* Release the flowring object back into the pool */
	dhd_prot_flowrings_pool_release(bus->dhd,
		flow_ring_node->flowid, flow_ring_node->prot_info);

	/* Free the flowid back to the flowid allocator */
	dhd_flowid_free(bus->dhd, flow_ring_node->flow_info.ifindex,
		flow_ring_node->flowid);
}

/**
 * Allocate a Flow ring buffer,
 * Init Ring buffer, send Msg to device about flow ring creation
*/
int
dhd_bus_flow_ring_create_request(dhd_bus_t *bus, void *arg)
{
	flow_ring_node_t *flow_ring_node = (flow_ring_node_t *)arg;

	DHD_INFO(("%s :Flow create\n", __FUNCTION__));

	/* Send Msg to device about flow ring creation */
	if (dhd_prot_flow_ring_create(bus->dhd, flow_ring_node) != BCME_OK)
		return BCME_NOMEM;

	return BCME_OK;
}

/** Handle response from dongle on a 'flow ring create' request */
void
dhd_bus_flow_ring_create_response(dhd_bus_t *bus, uint16 flowid, int32 status)
{
	flow_ring_node_t *flow_ring_node;
	unsigned long flags;

	DHD_INFO(("%s :Flow Response %d \n", __FUNCTION__, flowid));

	flow_ring_node = DHD_FLOW_RING(bus->dhd, flowid);
	ASSERT(flow_ring_node->flowid == flowid);

	if (status != BCME_OK) {
		DHD_ERROR(("%s Flow create Response failure error status = %d \n",
		     __FUNCTION__, status));
#if defined(BCM_BLOG)
#if defined(BCM_DHD_RUNNER)
		if (DHD_FLOWRING_RNR_OFFL(flow_ring_node)) {
			/*
			 * Flow ring is already enabled in the runner
			 * Disable it and flush any outstanding packets on that ring
			 */
			dhd_runner_notify(bus->dhd->runner_hlp, H2R_FLRING_DISAB_NOTIF, flowid, 0);
			dhd_runner_notify(bus->dhd->runner_hlp, H2R_FLRING_FLUSH_NOTIF, flowid, 0);
		}
#endif /* BCM_DHD_RUNNER */

		dhd_blog_flush_flowring(bus->dhd, flowid);
#endif /* BCM_BLOG */

		/* Call Flow clean up */
		dhd_bus_clean_flow_ring(bus, flow_ring_node);
		return;
	}

	DHD_FLOWRING_LOCK(flow_ring_node->lock, flags);
	flow_ring_node->status = FLOW_RING_STATUS_OPEN;
	DHD_FLOWRING_UNLOCK(flow_ring_node->lock, flags);

	/* Now add the Flow ring node into the active list
	 * Note that this code to add the newly created node to the active
	 * list was living in dhd_flowid_lookup. But note that after
	 * adding the node to the active list the contents of node is being
	 * filled in dhd_prot_flow_ring_create.
	 * If there is a D2H interrupt after the node gets added to the
	 * active list and before the node gets populated with values
	 * from the Bottom half dhd_update_txflowrings would be called.
	 * which will then try to walk through the active flow ring list,
	 * pickup the nodes and operate on them. Now note that since
	 * the function dhd_prot_flow_ring_create is not finished yet
	 * the contents of flow_ring_node can still be NULL leading to
	 * crashes. Hence the flow_ring_node should be added to the
	 * active list only after its truely created, which is after
	 * receiving the create response message from the Host.
	 */

	DHD_FLOWID_LOCK(bus->dhd->flowid_lock, flags);
	dll_prepend(&bus->const_flowring, &flow_ring_node->list);
	DHD_FLOWID_UNLOCK(bus->dhd->flowid_lock, flags);

#ifdef DHD_IFE
	dhd_ife_flowring_create(bus->dhd, flow_ring_node);
#endif /* DHD_IFE */
	dhd_bus_schedule_queue(bus, flowid, FALSE); /* from queue to flowring */
}

int
dhd_bus_flow_ring_delete_request(dhd_bus_t *bus, void *arg)
{
	void * pkt;
	flow_queue_t *queue;
	flow_ring_node_t *flow_ring_node;
	unsigned long flags;

	DHD_INFO(("%s :Flow Delete\n", __FUNCTION__));

	flow_ring_node = (flow_ring_node_t *)arg;

	DHD_FLOWRING_LOCK(flow_ring_node->lock, flags);
	if (flow_ring_node->status == FLOW_RING_STATUS_DELETE_PENDING) {
		DHD_FLOWRING_UNLOCK(flow_ring_node->lock, flags);
		DHD_ERROR(("%s :Delete Pending\n", __FUNCTION__));
		return BCME_ERROR;
	}
	flow_ring_node->status = FLOW_RING_STATUS_DELETE_PENDING;

	queue = &flow_ring_node->queue; /* queue associated with flow ring */

#ifdef DHDTCPACK_SUPPRESS
	/* Clean tcp_ack_info_tbl in order to prevent access to flushed pkt,
	 * when there is a newly coming packet from network stack.
	 */
	dhd_tcpack_info_tbl_clean(bus->dhd);
#endif /* DHDTCPACK_SUPPRESS */
	/* Flush all pending packets in the queue, if any */
	while ((pkt = dhd_flow_queue_dequeue(bus->dhd, queue)) != NULL) {
		PKTFREE(bus->dhd->osh, pkt, TRUE);
	}
	ASSERT(DHD_FLOW_QUEUE_EMPTY(queue));

	DHD_FLOWRING_UNLOCK(flow_ring_node->lock, flags);

	/* Send Msg to device about flow ring deletion */
	dhd_prot_flow_ring_delete(bus->dhd, flow_ring_node);

	return BCME_OK;
}

void
dhd_bus_flow_ring_delete_response(dhd_bus_t *bus, uint16 flowid, uint32 status)
{
	flow_ring_node_t *flow_ring_node;

	DHD_INFO(("%s :Flow Delete Response %d \n", __FUNCTION__, flowid));

	flow_ring_node = DHD_FLOW_RING(bus->dhd, flowid);
	ASSERT(flow_ring_node->flowid == flowid);

	if (status != BCME_OK) {
		DHD_ERROR(("%s Flow Delete Response failure error status = %d \n",
		    __FUNCTION__, status));
		return;
	}

#ifdef DHD_IFE
	/* Do not flush queues if delete initiated from IFE Module
	 * and there are packets pending
	 */
	if (flow_ring_node->evict_inprogress &&
		!(DHD_FLOW_QUEUE_EMPTY(&flow_ring_node->queue))) {
		unsigned long flags;

		flow_ring_node->evict_inprogress = FALSE;
		/* Remove node from the list otherwise it will
		 * be added again from create Response
		 */
		DHD_FLOWRING_LOCK(flow_ring_node->lock, flags);
		dll_delete(&flow_ring_node->list);
		dll_init(&flow_ring_node->list);
		flow_ring_node->status = FLOW_RING_STATUS_PENDING;
		DHD_FLOWRING_UNLOCK(flow_ring_node->lock, flags);

		if (dhd_bus_flow_ring_create_request(bus, flow_ring_node)
			!= BCME_OK) {
			DHD_ERROR(("%s: Flowring create error\n", __FUNCTION__));
			dhd_bus_clean_flow_ring(bus, flow_ring_node);
			return;
		}
	} else
#endif /* DHD_IFE */
	{
		/* Call Flow clean up */
		dhd_bus_clean_flow_ring(bus, flow_ring_node);
	}

	return;

}

/** This function is not called. Obsolete ? */
int
dhd_bus_flow_ring_flush_request(dhd_bus_t *bus, void *arg)
{
	void *pkt;
	flow_queue_t *queue;
	flow_ring_node_t *flow_ring_node;
	unsigned long flags;

	DHD_INFO(("%s :Flow Delete\n", __FUNCTION__));

	flow_ring_node = (flow_ring_node_t *)arg;
	queue = &flow_ring_node->queue; /* queue associated with flow ring */

	DHD_FLOWRING_LOCK(flow_ring_node->lock, flags);

#ifdef DHDTCPACK_SUPPRESS
	/* Clean tcp_ack_info_tbl in order to prevent access to flushed pkt,
	 * when there is a newly coming packet from network stack.
	 */
	dhd_tcpack_info_tbl_clean(bus->dhd);
#endif /* DHDTCPACK_SUPPRESS */

	/* Flush all pending packets in the queue, if any */
	while ((pkt = dhd_flow_queue_dequeue(bus->dhd, queue)) != NULL) {
		PKTFREE(bus->dhd->osh, pkt, TRUE);
	}
	ASSERT(DHD_FLOW_QUEUE_EMPTY(queue));

	DHD_FLOWRING_UNLOCK(flow_ring_node->lock, flags);

	/* Send Msg to device about flow ring flush */
	dhd_prot_flow_ring_flush(bus->dhd, flow_ring_node);

	flow_ring_node->status = FLOW_RING_STATUS_FLUSH_PENDING;
	return BCME_OK;
}

void
dhd_bus_flow_ring_flush_response(dhd_bus_t *bus, uint16 flowid, uint32 status)
{
	flow_ring_node_t *flow_ring_node;

	if (status != BCME_OK) {
		DHD_ERROR(("%s Flow flush Response failure error status = %d \n",
		    __FUNCTION__, status));
		return;
	}

	flow_ring_node = DHD_FLOW_RING(bus->dhd, flowid);
	ASSERT(flow_ring_node->flowid == flowid);

	flow_ring_node->status = FLOW_RING_STATUS_OPEN;
}

uint32
dhd_bus_max_h2d_rings(struct dhd_bus *bus)
{
	return bus->max_h2d_rings;
}

uint32
dhd_bus_max_interfaces(struct dhd_bus *bus)
{
	return bus->max_interfaces;
}

/* To be symmetric with SDIO */
void
dhd_bus_pktq_flush(dhd_pub_t *dhdp)
{
	return;
}

#ifdef OEM_ANDROID
int
dhdpcie_bus_clock_start(struct dhd_bus *bus)
{
	return dhdpcie_start_host_pcieclock(bus);
}

int
dhdpcie_bus_clock_stop(struct dhd_bus *bus)
{
	return dhdpcie_stop_host_pcieclock(bus);
}

int
dhdpcie_bus_disable_device(struct dhd_bus *bus)
{
	return dhdpcie_disable_device(bus);
}

int
dhdpcie_bus_enable_device(struct dhd_bus *bus)
{
	return dhdpcie_enable_device(bus);
}

int
dhdpcie_bus_alloc_resource(struct dhd_bus *bus)
{
	return dhdpcie_alloc_resource(bus);
}

void
dhdpcie_bus_free_resource(struct dhd_bus *bus)
{
	dhdpcie_free_resource(bus);
}

int
dhd_bus_request_irq(struct dhd_bus *bus)
{
	return dhdpcie_bus_request_irq(bus);
}

bool
dhdpcie_bus_dongle_attach(struct dhd_bus *bus)
{
	return dhdpcie_dongle_attach(bus);
}

int
dhd_bus_release_dongle(struct dhd_bus *bus)
{
	bool dongle_isolation;
	osl_t *osh;

	DHD_TRACE(("%s: Enter\n", __FUNCTION__));

	if (bus) {
		osh = bus->osh;
		ASSERT(osh);

		if (bus->dhd) {
			dongle_isolation = bus->dhd->dongle_isolation;
			dhdpcie_bus_release_dongle(bus, osh, dongle_isolation, TRUE);
		}
	}

	return 0;
}
#endif /* OEM_ANDROID */

#if defined(STB) && !defined(STBAP)
/* Enable or disable Wake-on-wireless LAN */
static void
dhdpcie_bus_set_wowl(struct dhd_bus *bus, int state)
{
	dhd_pub_t *dhdp = bus->dhd;
	dhd_bus_set_wowl((void *)dhdp->info, state);
}
#endif /* STB && STBAP */

const char *
dhdpcie_bus_get_device_name(struct dhd_bus *bus)
{
	return dhdpcie_get_device_name(bus);
}

#ifdef BCMPCIE_OOB_HOST_WAKE

int
dhd_bus_oob_intr_register(dhd_pub_t *dhdp)
{
	return dhdpcie_oob_intr_register(dhdp->bus);
}

void
dhd_bus_oob_intr_unregister(dhd_pub_t *dhdp)
{
	dhdpcie_oob_intr_unregister(dhdp->bus);
}

void
dhd_bus_oob_intr_set(dhd_pub_t *dhdp, bool enable)
{
	dhdpcie_oob_intr_set(dhdp->bus, enable);
}
#endif /* BCMPCIE_OOB_HOST_WAKE */

#ifdef BCMDBG
void
dhd_bus_flow_ring_cnt_update(dhd_bus_t *bus, uint16 flowid, uint32 txstatus)
{
	flow_ring_node_t *flow_ring_node;
	/* If we have d2h sync enabled due to marker overloading, we cannot update this. */
	if (bus->dhd->d2h_sync_mode)
		return;
	if (txstatus >= DHD_FLOWRING_MAXSTATUS_MSGS) {
		DHD_INFO(("%s Unknown txtstatus = %d \n",
		    __FUNCTION__, txstatus));
		return;
	}
	flow_ring_node = DHD_FLOW_RING(bus->dhd, flowid);
	ASSERT(flow_ring_node->flowid == flowid);
	flow_ring_node->flow_info.tx_status[txstatus]++;
	return;
}
#endif /* BCMDBG */

static void
hnd_hw_coherent_disable(dhd_bus_t *bus)
{
	uint savecore;
	volatile void *regs;

	savecore = si_coreidx(bus->sih);

	if (BCM6715_CHIP(bus->sih->chip)) {
		OSL_PCI_WRITE_CONFIG(bus->osh, PCI_BAR0_WIN, 4,
			(SI_ENUM_BASE(bus->sih) + SI_CCI500_S1_CTL_OFFSET));
	} else {
		OSL_PCI_WRITE_CONFIG(bus->osh, PCI_BAR0_WIN, 4,
			(SI_ENUM_BASE(bus->sih) + SI_CCI400_S3_CTL_OFFSET));
	}
	regs = (void *)bus->regs;
	W_REG(bus->osh, (uint32 *)regs, 0);

	si_setcoreidx(bus->sih, savecore);
}
