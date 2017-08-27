/*
 * Initialization and support routines for self-booting compressed image.
 *
 * Copyright (C) 2017, Broadcom. All Rights Reserved.
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
 * $Id: dngl_init.c 580798 2015-08-20 10:04:07Z $
 */

#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <siutils.h>
#include <wlioctl.h>
#include <hndcpu.h>
#include <bcmdevs.h>
#include <epivers.h>
#include <bcmutils.h>
#include <bcmnvram.h>
#include <hndsoc.h>
#include <sbchipc.h>
#include <hndpmu.h>
#include <dngl_dbg.h>
#include <dngl_bus.h>
#include <hndtcam.h>
#include <bcmsdpcm.h>
#include <event_log.h>
#include <bcmpcie.h>
#ifdef ATE_BUILD
#include "wl_ate.h"
#endif
#include <rte_dev.h>
#include <rte_mem.h>
#include <rte_tcam.h>
#include <rte.h>

/* entry C function from assembly */
static si_t *c_init(void);

extern hnd_dev_t bcmwl;	/* WL device struct */

extern hnd_dev_t sdpcmd_dev;
extern hnd_dev_t usbdev_dev;
extern hnd_dev_t pciedngl_dev;
extern hnd_dev_t pciedev_dev;
extern hnd_dev_t m2md_dev;

#ifdef WIFI_REFLECTOR
void wifi_reflector_init(void);
#endif

static const char BCMATTACHDATA(rstr_RNDIS)[] = "RNDIS";
static const char BCMATTACHDATA(rstr_CDC)[] = "CDC";
static const char BCMATTACHDATA(rstr_USB)[] = "USB";
static const char BCMATTACHDATA(rstr_SDIO)[] = "SDIO";
static const char BCMATTACHDATA(rstr_PCI)[] = "PCI";
static const char BCMATTACHDATA(rstr_USBSDIO)[] = "USB-SDIO";
static const char BCMATTACHDATA(rstr_SDPCMD)[] = "SDPCMD";
static const char BCMATTACHDATA(rstr_nocrc)[] = "nocrc";
static const char BCMATTACHDATA(rstr_crcchk)[] = "crcchk";
static const char BCMATTACHDATA(rstr_crcadr)[] = "crcadr";
static const char BCMATTACHDATA(rstr_crclen)[] = "crclen";
static const char BCMATTACHDATA(rstr_POLL)[] = "-POLL";
static const char BCMATTACHDATA(rstr_empty)[] = "";
static const char BCMATTACHDATA(rstr_RECLAIM)[] = "-RECLAIM";

#ifdef RNDIS
#define BUSPROTO rstr_RNDIS
#elif BCMMSGBUF
#define BUSPROTO "MSG_BUF"
#else
#define BUSPROTO rstr_CDC
#endif


#if defined(BCMUSBDEV) && defined(BCMSDIODEV) && defined(BCMPCIEDEV)
#define HBUSTYPE "USB-SDIO-PCIE"
#elif defined(BCMUSBDEV)
#define HBUSTYPE rstr_USB
#elif defined(BCMSDIODEV)
#define HBUSTYPE rstr_SDIO
#elif defined(BCM_OL_DEV)
#define HBUSTYPE rstr_PCI
#elif defined(BCMPCIEDEV)
#define HBUSTYPE "PCIE"
#elif defined(BCMM2MDEV)
#define HBUSTYPE "M2M"
#else
#error "Unknown bus type"
#endif

#ifdef BCMROMOFFLOAD

#ifdef __ARM_ARCH_7A__
extern void *sysmem_regs;
extern uint32 sysmem_rev;
#else
extern void *socram_regs;
extern uint32 socram_rev;
#endif
extern void hnd_patch_init(void *srp);


void __attribute__ ((weak))
BCMATTACHFN(hnd_patch_init)(void *srp)
{
	/* Avoid patch bits execution in roml builds */
	/* hnd_tcam_disablepatch(srp); */
}
#endif /* BCMROMOFFLOAD */

static void get_FWID(void);

/** Writes to end of data section, so not to last RAM word */
static void
BCMATTACHFN(get_FWID)(void)
{
	uint8 *tagsrc = (uint8 *)_end;
	uint8 *tagdst = (uint8 *)&gFWID;

	tagdst[0] = tagsrc[27];
	tagdst[1] = tagsrc[28];
	tagdst[2] = tagsrc[29];
	tagdst[3] = tagsrc[30];
}


#if defined(RTE_CRC32_BIN) && defined(BCMSDIODEV_ENABLED) && !defined(mips)
#ifndef ATE_BUILD
/* For locations that may differ from download */
struct modlocs {
	uint32 vars;
	uint32 varsz;
	uint32 memsize;
	uint32 rstvec;
	void   *armregs;
	uint32 watermark;
	uint32 chiptype;
	uint32 armwrap;
	void  *ram_regs;
	uint32 ram_rev;
#if defined(__ARM_ARCH_7R__) || defined(__ARM_ARCH_7A__)
	uint32 rambottom;
	uint32 atcmrambase;
#endif
};

static void
BCMATTACHFN(chk_image)(void)
{
	/* Default to entire image (excluding the padded crc and fwtag) */
	uint8 *crcadr = (uint8 *)hnd_get_rambase();
	uint32 crclen = (_end - (char *)crcadr);
	uint32 crcchk = ~(*(uint32 *)_end);
	uint32 crcval;

	/* Will need to save/restore modified locations */
	extern char *_vars;
	extern uint32 _varsz, _memsize, orig_rst;
	extern uint32 arm_wrap, chiptype;
	extern void *arm_regs;
#if defined(__ARM_ARCH_7A__)
	extern uint32 sysmem_rev;
	extern void *sysmem_regs;
#else
	extern uint32 socram_rev;
	extern void *socram_regs;
#endif

	struct modlocs newvals;

	/*
	 * if (_varsz > NVRAM_ARRAY_SIZE) startarm.S has already overwritten
	 * part of memory, so there is not much we can do but fail miserably
	 */
	while (_varsz > NVRAM_ARRAY_MAXSIZE);

	/* Bail if nvram explicity suppresses CRC check */
	if (getintvar(_vars, rstr_nocrc) == 1)
		goto done;


	/* Save possibly-modified locations and reset to original values */
	newvals.vars = (uint32)_vars;
	newvals.varsz = _varsz;
	newvals.memsize = _memsize;
	newvals.rstvec = *(uint32*)0;
	newvals.armregs = arm_regs;
	newvals.watermark = __watermark;
	newvals.chiptype = chiptype;
	newvals.armwrap = arm_wrap;
#if defined(__ARM_ARCH_7A__)
	newvals.ram_regs = (void *)sysmem_regs;
	newvals.ram_rev = sysmem_rev;
#else
	newvals.ram_regs = (void *)socram_regs;
	newvals.ram_rev = socram_rev;
#endif

#if defined(__ARM_ARCH_7R__) || defined(__ARM_ARCH_7A__)
	newvals.rambottom = _rambottom;
	newvals.atcmrambase = _atcmrambase;
#endif

#if defined(BCMSDIODEV) || defined(BCMHOSTVARS)
	/* Temporarily copy NVRAM to Arena and clear the original NVRAM area for CRC purposes */
	(void) memcpy(_end + 4, _vars, _varsz);
	(void) memset(_vars, 0, _varsz);
#endif	/* BCMSDIODEV || BCMHOSTVARS */

	__watermark = 0xbbadbadd;
	_vars = NULL;
	_varsz = _memsize = 0;
	arm_wrap = 0;
	arm_regs = NULL;
#if defined(__ARM_ARCH_7R__)	/* See startarm-cr4.S */
	chiptype = 1;
	socram_rev = -1;
	socram_regs = (void *)-1;
	_rambottom = _atcmrambase = 0xbbadbadd;
#elif defined(__ARM_ARCH_7A__)	/* See startarm-ca7.S */
	chiptype = 1;
	sysmem_rev = -1;
	sysmem_regs = (void *)-1;
	_rambottom = _atcmrambase = 0xbbadbadd;
#elif defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_4T__) /* See startarm-cm3.S & \
	startarm-7s.S */
	socram_rev = chiptype = 0;
	socram_regs = NULL;
#endif


#ifdef BCMDBG_ARMRST
	*(uint32*)0 = orig_rst;
	orig_rst = 0;
#endif

	/* Generate the checksum */
	crcval = hndcrc32(crcadr, crclen, CRC32_INIT_VALUE);
	while (crcval != crcchk);

	/* Restore modified locations */
	_vars = (char*)newvals.vars;
	_varsz = newvals.varsz;
	_memsize = newvals.memsize;
	orig_rst = *(uint32*)0;
	*(uint32*)0 = newvals.rstvec;
	arm_regs = newvals.armregs;
	__watermark = newvals.watermark;
	chiptype = newvals.chiptype;
	arm_wrap = newvals.armwrap;
#if defined(__ARM_ARCH_7A__)
	sysmem_regs = newvals.ram_regs;
	sysmem_rev = newvals.ram_rev;
#else
	socram_regs = newvals.ram_regs;
	socram_rev = newvals.ram_rev;
#endif
#if defined(__ARM_ARCH_7R__) || defined(__ARM_ARCH_7A__)
	_rambottom = newvals.rambottom;
	_atcmrambase = newvals.atcmrambase;
#endif

#if defined(BCMSDIODEV) || defined(BCMHOSTVARS)
	/* Copy back NVRAM data to _vars, and clear Arena */
	(void) memcpy(_vars, _end + 4, _varsz);
	(void) memset(_end + 4, 0, _varsz);
#endif	/* BCMSDIODEV || BCMHOSTVARS */

done:;

}
#endif /* !ATE_BUILD */
#endif /* RTE_CRC32_BIN && BCMSDIODEV_ENABLED && !mips */

#ifdef USB_XDCI
static bool  usbd_is30(si_t *sih);

static bool  usbd_is30(si_t *sih)
{
	bool usb30d = FALSE;
	uint32 cs;

	cs = sih->chipst;

	dbg("cc status %x", cs);

	/* need to check IFC since CST4350_USB30D_MODE is not reliable */
	if (CST4350_CHIPMODE_USB30D(cs) || CST4350_CHIPMODE_USB30D_WL(cs) ||
		CST4350_CHIPMODE_HSIC30D(cs)) {
		usb30d = TRUE;
	}

#ifdef BCM4350_FPGA
	usb30d = TRUE;
#endif

	return usb30d;
}
#endif /* USB_XDCI */


#define CLK_MPT(clk)		(((clk) + 50000) / 1000000)
#define CLK_KPT(clk)		((((clk) + 50000) - CLK_MPT(clk) * 1000000) / 100000)

static char BCMATTACHDATA(rstr_banner)[] =
	"\nRTE (%s-%s%s%s) %s on BCM%s r%d @ %d.%d/%d.%d/%d.%dMHz\n";

static si_t *
BCMATTACHFN(c_init)(void)
{
	chipcregs_t *cc;
#if defined(BCMDBG) || defined(BCMDBG_ERR)
	const char *busname = NULL;
#endif /* BCMDBG || BCMDBG_ERR */
	uint16 id;
	si_t *sih;
	char chn[8];
	hnd_dev_t *netdev = NULL;
	hnd_dev_t *busdev = NULL;

#ifdef BCMROMOFFLOAD
#ifndef __ARM_ARCH_7A__
	if ((socram_rev == 4) || (socram_rev == 5)) {
		/* "Patch hardware" initialization */
		hnd_patch_init(socram_regs);
	}
#endif
#endif /* BCMROMOFFLOAD */

	/* Basic initialization */
	sih = hnd_init();

	/* clear the watchdog counter which may have been set by the bootrom download code */
	si_watchdog(sih, 0);

#ifdef mips
	if (MFC0(C0_STATUS, 0) & ST0_NMI)
		err("NMI bit set, ErrorPC=0x%x", MFC0(C0_ERREPC, 0));
#elif defined(__arm__) || defined(__thumb__) || defined(__thumb2__)
	/* TODO: Check flags in the arm's resetlog */
#endif

	if (((cc = si_setcoreidx(sih, SI_CC_IDX)) != NULL) &&
	    (R_REG(si_osh(sih), &cc->intstatus) & CI_WDRESET)) {
		err("Watchdog reset bit set, clearing");
		W_REG(si_osh(sih), &cc->intstatus, CI_WDRESET);
	}

	/* Initialize and turn caches on */
	caches_on();

#ifdef EVENT_LOG_COMPILE
	/* Init the event logger */
	event_log_init(sih);
	  /* BUS LOG */
	event_log_set_init(sih, EVENT_LOG_SET_BUS, EVENT_LOG_BUS_BLOCK);
	event_log_set_expand(sih, EVENT_LOG_SET_BUS, EVENT_LOG_BUS_BLOCK);
	event_log_set_init(sih, EVENT_LOG_SET_DBG, EVENT_LOG_DBG_BLOCK);
	event_log_set_expand(sih, EVENT_LOG_SET_DBG, EVENT_LOG_DBG_BLOCK);

#ifdef PCI_TRACE_BY_DEFAULT
	event_log_tag_start(EVENT_LOG_TAG_PCI_TRACE, EVENT_LOG_SET_BUS,
		EVENT_LOG_TAG_FLAG_LOG);
#endif /* PCI_TRACE_BY_DEFAULT */
	event_log_tag_start(EVENT_LOG_TAG_PCI_ERROR, EVENT_LOG_SET_BUS,
		EVENT_LOG_TAG_FLAG_LOG);
	event_log_tag_start(EVENT_LOG_TAG_PCI_DBG, EVENT_LOG_SET_DBG,
		EVENT_LOG_TAG_FLAG_LOG);
#endif /* EVENT_LOG_COMPILE */

#ifdef USB_XDCI
	if (usbd_is30(sih))
		usbdev_dev.flags = (1 << RTEDEVFLAG_USB30);
#endif
	/* Print the banner */
	printf(rstr_banner,
	       HBUSTYPE, BUSPROTO,
#ifdef RTE_POLL
	       rstr_POLL,
#else
	       rstr_empty,
#endif
#ifdef BCMRECLAIM
	       rstr_RECLAIM,
#else
	       rstr_empty,
#endif
	       EPI_VERSION_STR,
	       bcm_chipname(si_chipid(sih), chn, sizeof(chn)), sih->chiprev,
	       CLK_MPT(si_alp_clock(sih)), CLK_KPT(si_alp_clock(sih)),
	       CLK_MPT(si_clock(sih)), CLK_KPT(si_clock(sih)),
	       CLK_MPT(si_cpu_clock(sih)), CLK_KPT(si_cpu_clock(sih)));

	/* Add the USB or SDIO/PCMCIA/PCIE device.  Only one may be defined
	 * at a time, except during a GENROMTBL build where both are
	 * defined to make sure all the symbols are pulled into ROM.
	 */
#if !defined(BCMUSBDEV) && !defined(BCMSDIODEV) && !defined(BCM_OL_DEV) && \
	!defined(BCMPCIEDEV) && !defined(BCMM2MDEV)
#error "Bus type undefined"
#endif

#ifdef DONGLEOVERLAYS
	hnd_overlay_prep();
#endif

#if defined(BCMUSBDEV) && defined(BCMUSBDEV_ENABLED)
#if defined(BCMDBG) || defined(BCMDBG_ERR)
	busname = rstr_USB;
#endif /* BCMDBG || BCMDBG_ERR */
	trace("add USB device");
	if (hnd_add_device(sih, &usbdev_dev, USB_CORE_ID, BCM47XX_USBD_ID) == 0 ||
#ifdef USB_XDCI
	    ((usbdev_dev.flags & (1 << RTEDEVFLAG_USB30)) &&
	     (hnd_add_device(sih, &usbdev_dev, USB30D_CORE_ID, BCM47XX_USB30D_ID) == 0)) ||
#endif
	    hnd_add_device(sih, &usbdev_dev, USB20D_CORE_ID, BCM47XX_USB20D_ID) == 0 ||
	    hnd_add_device(sih, &usbdev_dev, USB11D_CORE_ID, BCM47XX_USBD_ID) == 0) {
		busdev = &usbdev_dev;
		bus_ops = &usbdev_bus_ops;
		proto_ops = &cdc_proto_ops;
	}
#endif /* BCMUSBDEV && BCMUSBDEV_ENABLED */

#if defined(BCMSDIODEV) && defined(BCMSDIODEV_ENABLED)
#if defined(BCMDBG) || defined(BCMDBG_ERR)
	busname = rstr_SDPCMD;
#endif /* BCMDBG || BCMDBG_ERR */
	trace("add SDIO device");
	if (hnd_add_device(sih, &sdpcmd_dev, PCMCIA_CORE_ID, SDIOD_FPGA_ID) == 0 ||
	    hnd_add_device(sih, &sdpcmd_dev, SDIOD_CORE_ID, SDIOD_FPGA_ID) == 0) {
		busdev = &sdpcmd_dev;
		bus_ops = &sdpcmd_bus_ops;
		proto_ops = &cdc_proto_ops;
	}
#endif /* BCMSDIODEV && BCMSDIODEV_ENABLED */

#if defined(BCMPCIEDEV)
	if (BCMPCIEDEV_ENAB()) {
#if defined(BCMDBG) || defined(BCMDBG_ERR)
		busname = "PCIE";
#endif /* BCMDBG || BCMDBG_ERR */
		trace("  c_init: add PCIE device\n");
		if (hnd_add_device(sih, &pciedev_dev, PCIE2_CORE_ID, 0x43ff) == 0) {
			busdev = &pciedev_dev;
			bus_ops = &pciedev_bus_ops;
			proto_ops = &msgbuf_proto_ops;
		}
	}
#endif /* BCMPCIEDEV */

#if defined(BCMM2MDEV) && defined(BCMM2MDEV_ENABLED)
#if defined(BCMDBG) || defined(BCMDBG_ERR)
	busname = "M2M";
#endif /* BCMDBG || BCMDBG_ERR */
	trace("  c_init: add M2M device\n");
	if (hnd_add_device(sih, &m2md_dev, M2MDMA_CORE_ID, 0x4999) == 0) {
		busdev = &m2md_dev;
		bus_ops = &m2md_bus_ops;
		proto_ops = &cdc_proto_ops;
	}
#endif /* BCMM2MDEV && BCMM2MDEV_ENABLED */

#ifndef BCM4350_FPGA
#ifdef BCM_OL_DEV
#if defined(BCMDBG) || defined(BCMDBG_ERR)
	busname = "PCIDEV";
#endif /* BCMDBG || BCMDBG_ERR */
	trace("  c_init: add PCI device\n");
	if (hnd_add_device(sih, &pciedngl_dev, PCIE2_CORE_ID, 0x4999) == 0) {
		busdev = &pciedngl_dev;
		bus_ops = &pciedngl_bus_ops;
		proto_ops = &cdc_proto_ops;
	}
#endif /* BCM_OL_DEV */

	/* Add the WL device, they are exclusive */
	if ((id = si_d11_devid(sih)) == 0xffff)
		id = BCM4318_D11G_ID;
	trace("add WL device 0x%x", id);
	if (hnd_add_d11device(sih, &bcmwl, id) == 0)
		netdev = &bcmwl;
#else
	id = 0;
	netdev = &bcmwl;
#endif /* BCM4350_FPGA */

#ifndef ATE_BUILD
	ASSERT(busdev);
#endif
	ASSERT(netdev);
	/* If USB/SDIO/PCMCIA/PCI device is there then open it */
	if (busdev != NULL && netdev != NULL) {
#ifndef BCM4350_FPGA
		if (bus_ops->binddev(busdev, netdev,
			si_numd11coreunits(sih)) < 0)
			err("%s%s device binddev failed", busname, BUSPROTO);
#endif
		if (busdev->ops->open(busdev))
			err("%s%s device open failed", busname, BUSPROTO);

#ifdef BCM_OL_DEV
		/* Call wl_open here for now */
		if (netdev->ops->open(netdev))
			err("netdev:  device open failed");
#endif /* BCM_OL_DEV */
	}

	return sih;
}

#ifdef WIFI_REFLECTOR
void
wifi_reflector_init(void)
{
	int i;
	char buf[32];
	struct {
		uint32  cmd;
		uint32  len;
		char    *data;
		int     value;
	} wifi_init_cmds[] = {
		{WLC_UP, 0x0, NULL, 0x0},
		{WLC_SET_VAR, 0x8, "mpc", 0},
		{WLC_SET_WSEC, 0x4, NULL, 0x0},
		{WLC_SET_VAR, 0xf, "slow_timer", 999999},
		{WLC_SET_VAR, 0xf, "fast_timer", 999999},
		{WLC_SET_VAR, 0x12, "glacial_timer", 999999},
		{WLC_LEGACY_LINK_BEHAVIOR, 0x04, NULL, 0x1},
		{WLC_SET_MONITOR, 0x4, NULL, 0x1}
	};

	for (i = 0; i < 8; ++i) {
		if (wifi_init_cmds[i].data != NULL) {
			strncpy(buf, wifi_init_cmds[i].data, sizeof(buf));
			memcpy(&buf[strlen(wifi_init_cmds[i].data) + 1],
			       (char*)&wifi_init_cmds[i].value, sizeof(int));
		} else
			memcpy(&buf[0], (char*)&wifi_init_cmds[i].value, sizeof(int));

		bcmwl.ops->ioctl(&bcmwl, wifi_init_cmds[i].cmd,
		                   buf, wifi_init_cmds[i].len, NULL, NULL, FALSE);
	}
}
#endif /* WIFI_REFLECTOR */

/* c_main is non-reclaimable, as it is the one calling hnd_reclaim */
si_t *_c_main(void);

si_t *
_c_main(void)
{
	si_t *sih;

	get_FWID();

#if defined(RTE_CRC32_BIN) && defined(BCMSDIODEV_ENABLED) && !defined(mips)
#ifndef ATE_BUILD
	chk_image();
#endif /* !ATE_BUILD */
#endif /* (RTE_CRC32_BIN) && defined(BCMSDIODEV_ENABLED) && !defined(mips) */

#ifdef BCMTCAM
	/* Load patch table early */
	hnd_tcam_load_default(hnd_get_rambase());
#endif
	/* Call reclaimable init function */
	sih = c_init();

#ifdef ATE_BUILD
	/* Perform ATE init */
	wl_ate_init(sih, &bcmwl);
#endif

#if defined(RSOCK) || defined(DONGLEBUILD)
#ifdef BCMRECLAIM
	bcmreclaimed = TRUE;
#endif /* BCMRECLAIM */
#ifdef BCMTCAM
	hnd_tcam_reclaim();
#endif
#if defined(DONGLEBUILD) && defined(WLC_HIGH) && !defined(WLTEST) && \
	!defined(BCMDBG_DUMP) && !defined(ATE_BUILD)
	/* After nvram_exit(), NVRAM variables are no longer accessible */
	nvram_exit((void*)sih);
#endif 
	hnd_reclaim();
#endif /* defined(RSOCK) || defined(DONGLEBUILD) */
#ifdef WIFI_REFLECTOR
	/* Get the interface up */
	wifi_reflector_init();
#endif

	return sih;
}
