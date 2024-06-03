/*
 * Misc utility routines for accessing the SOC Interconnects
 * of Broadcom HNBU chips.
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
 * $Id: siutils.h 831472 2023-10-18 16:05:45Z $
 */

#ifndef	_siutils_h_
#define	_siutils_h_

#if (defined(CMWIFI) && defined(BCMEXTNVM))
#include "bcmsrom_fmt.h"
#endif

#define WARM_BOOT	0xA0B0C0D0

#ifdef BCM_BACKPLANE_TIMEOUT

#define SI_MAX_ERRLOG_SIZE	4
typedef struct si_axi_error
{
	uint32 error;
	uint32 coreid;
	uint32 errlog_lo;
	uint32 errlog_hi;
	uint32 errlog_id;
	uint32 errlog_flags;
	uint32 errlog_status;
} si_axi_error_t;

typedef struct si_axi_error_info
{
	uint32 count;
	si_axi_error_t axi_error[SI_MAX_ERRLOG_SIZE];
} si_axi_error_info_t;
#endif /* BCM_BACKPLANE_TIMEOUT */

enum sprom_bus_e {
	SPROM_BUS_NO_SPROM = 0, /* no SPROM attached */
	SPROM_BUS_UWIRE,
	SPROM_BUS_SPI
};

/**
 * Data structure to export all chip specific common variables
 *   public (read-only) portion of siutils handle returned by si_attach()/si_kattach()
 */
struct si_pub {
	uint	socitype;		/**< SOCI_SB, SOCI_AI */

	enum bustype_e	bustype;	/**< SI_BUS, PCI_BUS */
	uint	buscoretype;		/**< PCI_CORE_ID, PCIE_CORE_ID, PCMCIA_CORE_ID */
	uint	buscorerev;		/**< buscore rev */
	uint	buscoreidx;		/**< buscore index */
	int	ccrev;			/**< chip common core rev */
	uint32	cccaps;			/**< chip common capabilities */
	uint32  cccaps_ext;			/**< chip common capabilities extension */
	int	pmurev;			/**< pmu core rev */
	uint32	pmucaps;		/**< pmu capabilities */
	uint	boardtype;		/**< board type */
	uint    boardrev;               /* board rev */
	uint	boardvendor;		/**< board vendor */
	uint	boardflags;		/**< board flags */
	uint	boardflags2;		/**< board flags2 */
	uint	boardflags4;		/**< board flags4 */
	uint	chip;			/**< chip number */
	uint	chiprev;		/**< chip revision */
	uint	chippkg;		/**< chip package option */
	uint32	chipst;			/**< chip status */
	uint32	otpflag;		/**< chip specific OTP info */
	bool	issim;			/**< chip is in simulation or emulation */
	uint    socirev;		/**< SOC interconnect rev */
	int	gcirev;			/**< gci core rev */
	int	lpflags;		/**< low power flags */
	/** physical backplane address where the cc core resides from CPU perspective: */
	union {
		uint32	enum_base;	/**< to be obsoleted struct member */
		uint32	enum_base_pa;	/**< the new name for this struct member */
	};
	/** virtual address where the cc core resides from CPU perspective: */
	uintptr	enum_base_va; /**< only non-zero for NIC build && bustype==SI_BUS */
	/** physical backplane address where the cc core resides from WLAN core point-of-view: */
	uint32	enum_base_bp_pov_pa;
#ifdef BCM_BACKPLANE_TIMEOUT
	si_axi_error_info_t * err_info;
#endif /* BCM_BACKPLANE_TIMEOUT */
	bool	srvars_inited;		/**< SROM vars initialized */
	enum sprom_bus_e sprom_bus;
};

/* for HIGH_ONLY driver, the si_t must be writable to allow states sync from BMAC to HIGH driver
 * for monolithic driver, it is readonly to prevent accident change
 */
typedef struct si_pub si_t;

/*
 * Many of the routines below take an 'sih' handle as their first arg.
 * Allocate this by calling si_attach().  Free it by calling si_detach().
 * At any one time, the sih is logically focused on one particular si core
 * (the "current core").
 * Use si_setcore() or si_setcoreidx() to change the association to another core.
 */
#define	SI_OSH		NULL	/**< Use for si_kattach when no osh is available */

#define	BADIDX		(SI_MAXCORES + 1)

/* clkctl xtal what flags */
#define	XTAL			0x1	/**< primary crystal oscillator (2050) */
#define	PLL			0x2	/**< main chip pll */

/* clkctl clk mode */
#define	CLK_FAST		0	/**< force fast (pll) clock */
#define	CLK_DYNAMIC		2	/**< enable dynamic clock control */

/* GPIO usage priorities */
#define GPIO_DRV_PRIORITY	0	/**< Driver */
#define GPIO_APP_PRIORITY	1	/**< Application */
#define GPIO_HI_PRIORITY	2	/**< Highest priority. Ignore GPIO reservation */

/* GPIO pull up/down */
#define GPIO_PULLUP		0
#define GPIO_PULLDN		1

/* GPIO event regtype */
#define GPIO_REGEVT		0	/**< GPIO register event */
#define GPIO_REGEVT_INTMSK	1	/**< GPIO register event int mask */
#define GPIO_REGEVT_INTPOL	2	/**< GPIO register event int polarity */

/* device path */
#define SI_DEVPATH_BUFSZ	16	/**< min buffer size in bytes */

/* SI routine enumeration: to be used by update function with multiple hooks */
#define	SI_DOATTACH	1
#define SI_PCIDOWN	2	/**< wireless interface is down */
#define SI_PCIUP	3	/**< wireless interface is up */

/* "access" param defines for si_seci_access() below */
#define SECI_ACCESS_STATUSMASK_SET	0
#define SECI_ACCESS_INTRS			1
#define SECI_ACCESS_UART_CTS		2
#define SECI_ACCESS_UART_RTS		3
#define SECI_ACCESS_UART_RXEMPTY	4
#define SECI_ACCESS_UART_GETC		5
#define SECI_ACCESS_UART_TXFULL		6
#define SECI_ACCESS_UART_PUTC		7
#define SECI_ACCESS_STATUSMASK_GET	8

#if defined(BCMQT)
#define	ISSIM_ENAB(sih)	TRUE
#else
#define	ISSIM_ENAB(sih)	FALSE
#endif

#define INVALID_ADDR (~0)

/* chipcommon clock/power control (exclusive with PMU's) */
#if defined(BCMPMUCTL) && BCMPMUCTL
#define CCCTL_ENAB(sih)		(0)
#define CCPLL_ENAB(sih)		(0)
#else
#define CCCTL_ENAB(sih)		((sih)->cccaps & CC_CAP_PWR_CTL)
#define CCPLL_ENAB(sih)		((sih)->cccaps & CC_CAP_PLL_MASK)
#endif

typedef void (*gci_gpio_handler_t)(uint32 stat, void *arg);

/* External BT Coex enable mask */
#define CC_BTCOEX_EN_MASK  0x01
/* External PA enable mask */
#define GPIO_CTRL_EPA_EN_MASK 0x40
/* WL/BT control enable mask */
#define GPIO_CTRL_5_6_EN_MASK 0x60
#define GPIO_CTRL_7_6_EN_MASK 0xC0
#define GPIO_OUT_7_EN_MASK 0x80

#if defined(BCMDONGLEHOST)

/* CR4 specific defines used by the host driver */
#define SI_CR4_CAP			(0x04)
#define SI_CR4_BANKIDX		(0x40)
#define SI_CR4_BANKINFO		(0x44)
#define SI_CR4_BANKPDA		(0x4C)

#define	ARMCR4_TCBBNB_MASK	0xf0
#define	ARMCR4_TCBBNB_SHIFT	4
#define	ARMCR4_TCBANB_MASK	0xf
#define	ARMCR4_TCBANB_SHIFT	0

#define	SICF_CPUHALT		(0x0020)
#define	ARMCR4_BSZ_MASK		0x3f
#define	ARMCR4_BSZ_MULT		8192

/* armcb53 = armca53+bca enhancement */
#define	SICF_ARMCA53_CPU0HALT	(0x0010)
#define	SICF_ARMCA53_CPU1HALT	(0x0020)
#define	SICF_ARMCA53_CPUHALT	(SICF_ARMCA53_CPU0HALT | SICF_ARMCA53_CPU1HALT)

/* ARMCA7 REV8
 * ForceCoreReset bits[7:4] For each bit 3:0, when this field is set to 1,
 * the corresponding ARM processor is held in the reset state.
 * When cleared to 0, the processor reset is the same as the core reset.
 */
#define	SICF_CPUHALT_CPU0	(0x0010)
#endif /* BCMDONGLEHOST */
#define	SI_BPIND_1BYTE		0x1
#define	SI_BPIND_2BYTE		0x3
#define	SI_BPIND_4BYTE		0xF

#define GET_GCI_OFFSET(sih, gci_reg)	\
	OFFSETOF(gciregs_t, gci_reg)

#define GET_GCI_CORE(sih)	\
	si_findcoreidx(sih, GCI_CORE_ID, 0)

#include <osl_decl.h>
/* === exported functions === */
extern si_t *si_attach(uint pcidev, osl_t *osh, volatile void *regs, enum bustype_e bustype,
                       void *sdh, char **vars, uint *varsz);
extern si_t *si_kattach(osl_t *osh);
extern void si_detach(si_t *sih);
extern volatile void *
si_d11_switch_addrbase(si_t *sih, uint coreunit);
extern uint si_corelist(si_t *sih, uint coreid[]);
extern uint si_coreid(si_t *sih);
extern uint si_flag(si_t *sih);
extern uint si_flag_alt(si_t *sih);
extern uint si_intflag(si_t *sih);
extern uint si_coreidx(si_t *sih);
extern uint si_coreunit(si_t *sih);
extern uint si_corevendor(si_t *sih);
extern uint si_corerev(si_t *sih);
extern uint si_corerev_minor(si_t *sih);
extern void *si_osh(si_t *sih);
extern void si_setosh(si_t *sih, osl_t *osh);
extern uint si_backplane_access(si_t *sih, uint addr, uint size,
	uint *val, bool read);
extern uint si_backplane_pci_config_bar0_core2_win2(si_t *sih, uint32 addr);
#if !defined(BCMDONGLEHOST)
extern uint si_backplane_pci_config_bar0_ext_win(si_t *sih, uint32 addr, uint32 ext_idx);
#endif /* !BCMDONGLEHOST */
extern uint si_corereg(si_t *sih, uint coreidx, uint regoff, uint mask, uint val);
extern uint si_pmu_corereg(si_t *sih, uint32 idx, uint regoff, uint mask, uint val);
extern volatile uint32 *si_corereg_addr(si_t *sih, uint coreidx, uint regoff);
extern volatile void *si_coreregs(si_t *sih);
extern uint si_wrapperreg(si_t *sih, uint32 offset, uint32 mask, uint32 val);
extern uint si_core_wrapperreg(si_t *sih, uint32 coreidx, uint32 offset, uint32 mask, uint32 val);
extern void *si_wrapperregs(si_t *sih);
extern uint32 si_core_cflags(si_t *sih, uint32 mask, uint32 val);
extern void si_core_cflags_wo(si_t *sih, uint32 mask, uint32 val);
extern uint32 si_core_sflags(si_t *sih, uint32 mask, uint32 val);
extern void si_commit(si_t *sih);
extern bool si_iscoreup(si_t *sih);
extern uint si_numcoreunits(si_t *sih, uint coreid);
extern uint si_numd11coreunits(si_t *sih);
extern uint si_findcoreidx(si_t *sih, uint coreid, uint coreunit);
extern volatile void *si_setcoreidx(si_t *sih, uint coreidx);
extern volatile void *si_setcore(si_t *sih, uint coreid, uint coreunit);
extern volatile void *si_switch_core(si_t *sih, uint coreid, uint *origidx, uint *intr_val);
extern volatile void *si_switch_coreunit(si_t *sih, uint coreid, uint coreunit, uint *origidx,
	uint *intr_val);
extern void si_restore_core(si_t *sih, uint coreid, uint intr_val);
extern int si_numaddrspaces(si_t *sih);
extern uint32 si_addrspace(si_t *sih, uint spidx, uint baidx);
extern uint32 si_addrspacesize(si_t *sih, uint spidx, uint baidx);
extern void si_coreaddrspaceX(si_t *sih, uint asidx, uint32 *addr, uint32 *size);
extern int si_corebist(si_t *sih);
extern void si_core_reset(si_t *sih, uint32 bits, uint32 resetbits);
extern void si_core_disable(si_t *sih, uint32 bits);
extern uint32 si_clock_rate(uint32 pll_type, uint32 n, uint32 m);
extern uint si_chip_hostif(si_t *sih);
extern uint32 si_clock(si_t *sih);
extern uint32 si_alp_clock(si_t *sih); /* returns [Hz] units */
extern uint32 si_xtal_freq(si_t *sih); /* returns [Hz] units */
extern uint32 si_ilp_clock(si_t *sih); /* returns [Hz] units */
extern void si_pci_setup(si_t *sih, uint coremask);
extern void si_setint(si_t *sih, int siflag);
extern bool si_backplane64(si_t *sih);
extern void si_register_intr_callback(si_t *sih, void *intrsoff_fn, void *intrsrestore_fn,
	void *intrsenabled_fn, void *intr_arg);
extern void si_deregister_intr_callback(si_t *sih);
extern void si_clkctl_init(si_t *sih);
extern uint16 si_clkctl_fast_pwrup_delay(si_t *sih);
extern bool si_clkctl_cc(si_t *sih, uint mode);
extern int si_clkctl_xtal(si_t *sih, uint what, bool on);
extern uint32 si_gpiotimerval(si_t *sih, uint32 mask, uint32 val);
extern void si_btcgpiowar(si_t *sih);
extern bool si_deviceremoved(si_t *sih);
extern void si_set_device_removed(si_t *sih, bool status);
extern uint32 si_sysmem_size(si_t *sih);
extern uint32 si_socram_size(si_t *sih);
extern uint32 si_socdevram_size(si_t *sih);
extern uint32 si_socram_srmem_size(si_t *sih);
extern void si_socram_set_bankpda(si_t *sih, uint32 bankidx, uint32 bankpda);
extern void si_socdevram(si_t *sih, bool set, uint8 *ennable, uint8 *protect, uint8 *remap);
extern bool si_socdevram_pkg(si_t *sih);
extern bool si_socdevram_remap_isenb(si_t *sih);
extern uint32 si_socdevram_remap_size(si_t *sih);

extern void si_watchdog(si_t *sih, uint ticks);
extern void si_watchdog_ms(si_t *sih, uint32 ms);
extern uint32 si_watchdog_msticks(void);
extern volatile void *si_gpiosetcore(si_t *sih);
extern uint32 si_gpiocontrol(si_t *sih, uint32 mask, uint32 val, uint8 priority);
extern uint32 si_gpioouten(si_t *sih, uint32 mask, uint32 val, uint8 priority);
extern uint32 si_gpioout(si_t *sih, uint32 mask, uint32 val, uint8 priority);
extern uint32 si_gpioin(si_t *sih);
extern uint32 si_gpiointpolarity(si_t *sih, uint32 mask, uint32 val, uint8 priority);
extern uint32 si_gpiointmask(si_t *sih, uint32 mask, uint32 val, uint8 priority);
extern uint32 si_gpioeventintmask(si_t *sih, uint32 mask, uint32 val, uint8 priority);
extern uint32 si_gpioled(si_t *sih, uint32 mask, uint32 val);
extern uint32 si_gpioreserve(si_t *sih, uint32 gpio_num, uint8 priority);
extern uint32 si_gpiorelease(si_t *sih, uint32 gpio_num, uint8 priority);
extern uint32 si_gpiopull(si_t *sih, bool updown, uint32 mask, uint32 val);
extern uint32 si_gpioevent(si_t *sih, uint regtype, uint32 mask, uint32 val);
extern uint32 si_gpio_int_enable(si_t *sih, bool enable);
extern void si_gci_uart_init(si_t *sih, osl_t *osh, uint8 seci_mode);
extern void si_gci_enable_gpio(si_t *sih, uint8 gpio, uint32 mask, uint32 value);

extern void si_invalidate_second_bar0win(si_t *sih);

extern void si_gci_shif_config_wake_pin(si_t *sih, uint8 gpio_n,
		uint8 wake_events, bool gci_gpio);
extern void si_shif_int_enable(si_t *sih, uint8 gpio_n, uint8 wake_events, bool enable);

/* GCI interrupt handlers */
extern void si_gci_handler_process(si_t *sih);

extern void si_enable_gpio_wake(si_t *sih, uint8 *wake_mask, uint8 *cur_status, uint8 gci_gpio,
	uint32 pmu_cc2_mask, uint32 pmu_cc2_value);

/* GCI GPIO event handlers */
extern void *si_gci_gpioint_handler_register(si_t *sih, uint8 gpio, uint8 sts,
	gci_gpio_handler_t cb, void *arg);
extern void si_gci_gpioint_handler_unregister(si_t *sih, void* gci_i);

extern uint8 si_gci_gpio_status(si_t *sih, uint8 gci_gpio, uint8 mask, uint8 value);
extern void si_gci_config_wake_pin(si_t *sih, uint8 gpio_n, uint8 wake_events,
	bool gci_gpio);
extern void si_gci_free_wake_pin(si_t *sih, uint8 gpio_n);
#if !defined(BCMDONGLEHOST)
#if (!defined(_CFE_) && !defined(_CFEZ_)) || defined(CFG_WL)
extern uint8 si_gci_gpio_wakemask(si_t *sih, uint8 gpio, uint8 mask, uint8 value);
extern uint8 si_gci_gpio_intmask(si_t *sih, uint8 gpio, uint8 mask, uint8 value);
#endif /* (!_CFE_ && !_CFEZ_) || CFG_WL */
#endif /* !defined(BCMDONGLEHOST) */

extern uint si_pcie_readreg(void *sih, uint addrtype, uint offset);
extern uint si_pcie_writereg(void *sih, uint addrtype, uint offset, uint val);

extern uint16 si_d11_devid(si_t *sih);
extern int si_corepciid(si_t *sih, uint func, uint16 *pcivendor, uint16 *pcidevice,
	uint8 *pciclass, uint8 *pcisubclass, uint8 *pciprogif, uint8 *pciheader);

extern uint32 si_seci_access(si_t *sih, uint32 val, int access);
extern volatile void* si_seci_init(si_t *sih, uint8 seci_mode);
#if !defined(BCMDONGLEHOST)
extern void si_seci_clkreq(si_t *sih, bool enable);
#endif
extern void si_seci_clk_force(si_t *sih, bool val);
extern bool si_seci_clk_force_status(si_t *sih);

#if (defined(BCMECICOEX) && !defined(BCMDONGLEHOST))
extern bool si_eci(si_t *sih);
extern int si_eci_init(si_t *sih);
extern void si_eci_notify_bt(si_t *sih, uint32 mask, uint32 val, bool interrupt);
extern bool si_seci(si_t *sih);
extern void* si_gci_init(si_t *sih);
extern void si_seci_down(si_t *sih);
extern bool si_gci(si_t *sih);
#else
#define si_eci(sih) 0
static INLINE void * si_eci_init(si_t *sih) {return NULL;}
#define si_eci_notify_bt(sih, type, val)  (0)
#define si_seci(sih) 0
static INLINE void * si_gci_init(si_t *sih) {return NULL;}
#define si_seci_down(sih) do {} while (0)
#define si_gci(sih) 0
#endif /* BCMECICOEX */

#if !defined(BCMDONGLEHOST) && !defined(DONGLEBUILD)
extern void si_gci_hw_semaphore_request(si_t *sih);
extern void si_gci_hw_semaphore_finish(si_t *sih);
#endif /* !(BCMDONGLEHOST) && !defined(DONGLEBUILD) */

/* OTP status */
extern bool si_is_otp_disabled(si_t *sih);
extern bool si_is_otp_powered(si_t *sih);
extern void si_otp_power(si_t *sih, bool on, uint32* min_res_mask);

/* SPROM availability */
extern bool si_is_sprom_available(si_t *sih);
#ifdef SI_SPROM_PROBE
extern void si_sprom_init(si_t *sih);
#endif /* SI_SPROM_PROBE */

/* OTP/SROM CIS stuff */
extern int si_cis_source(si_t *sih);
#define CIS_DEFAULT	0
#define CIS_SROM	1
#define CIS_OTP		2

/* Fab-id information */
#define	DEFAULT_FAB	0x0	/**< Original/first fab used for this chip */
#define	CSM_FAB7	0x1	/**< CSM Fab7 chip */
#define	TSMC_FAB12	0x2	/**< TSMC Fab12/Fab14 chip */
#define	SMIC_FAB4	0x3	/**< SMIC Fab4 chip */

/* nvram prefixes (so-called devpaths) */
#define DEVPATH_SB	"sb/"
#define DEVPATH_PCIE	"pcie/"

extern uint16 BCMATTACHFN(si_fabid)(si_t *sih);
extern uint16 BCMINITFN(si_chipid)(si_t *sih);

/*
 * Build device path. Path size must be >= SI_DEVPATH_BUFSZ.
 * The returned path is NULL terminated and has trailing '/'.
 * Return 0 on success, nonzero otherwise.
 */
extern int si_devpath(si_t *sih, char *path, int size);
extern int si_devpath_pcie(si_t *sih, char *path, int size);
/* Read variable with prepending the devpath to the name */
extern char *si_getdevpathvar(si_t *sih, const char *name);
extern int si_getdevpathintvar(si_t *sih, const char *name);
extern char *si_compact_devpathvar(si_t *sih, char *varname, int var_len, const char *name);

extern uint8 si_pcieclkreq(si_t *sih, uint32 mask, uint32 val);
extern uint32 si_pcielcreg(si_t *sih, uint32 mask, uint32 val);
extern uint8 si_pcieltrenable(si_t *sih, uint32 mask, uint32 val);
extern uint8 si_pcieobffenable(si_t *sih, uint32 mask, uint32 val);
extern uint32 si_pcieltr_reg(si_t *sih, uint32 reg, uint32 mask, uint32 val);
extern void si_pcie_set_error_injection(si_t *sih, uint32 mode);
extern void si_pci_down(si_t *sih);
extern void si_pci_up(si_t *sih);
extern void si_pci_sleep(si_t *sih);
extern int si_pci_fixcfg(si_t *sih);
extern void si_chippkg_set(si_t *sih, uint);

extern void si_chipcontrl_restore(si_t *sih, uint32 val);
extern uint32 si_chipcontrl_read(si_t *sih);
extern void si_srom_clk_set(si_t *sih); /**< for chips with fast BP clock */
extern void si_btc_enable_chipcontrol(si_t *sih);
extern void si_pmu_avb_clk_set(si_t *sih, osl_t *osh, bool set_flag);
/* === debug routines === */

extern bool si_taclear(si_t *sih, bool details);

#ifdef BCMDBG
extern void si_view(si_t *sih, bool verbose);
extern void si_viewall(si_t *sih, bool verbose);
#endif /* BCMDBG */
struct bcmstrbuf;
#if defined(BCMDBG) || defined(BCMDBG_PHYDUMP) || defined(WLTEST)
extern int si_dump_pcieinfo(si_t *sih, struct bcmstrbuf *b);
extern void si_dump_pmuregs(si_t *sih, struct bcmstrbuf *b);
extern int si_dump_pcieregs(si_t *sih, struct bcmstrbuf *b);
extern int si_gpiodump(si_t *sih, struct bcmstrbuf *b);
#endif

#if defined(BCMDBG) || defined(BCMDBG_PHYDUMP)
extern void si_dumpregs(si_t *sih, struct bcmstrbuf *b);
#endif

extern uint32 si_ccreg(si_t *sih, uint32 offset, uint32 mask, uint32 val);
extern uint32 si_pciereg(si_t *sih, uint32 offset, uint32 mask, uint32 val, uint type);
extern int si_bpind_access(si_t *sih, uint32 addr_high, uint32 addr_low,
	int32* data, bool read);

extern uint32 si_pcieserdesreg(si_t *sih, uint32 mdioslave, uint32 offset, uint32 mask, uint32 val);
extern void si_pcie_set_request_size(si_t *sih, uint16 size);
extern uint16 si_pcie_get_request_size(si_t *sih);
extern void si_pcie_set_maxpayload_size(si_t *sih, uint16 size);
extern uint16 si_pcie_get_maxpayload_size(si_t *sih);
extern uint16 si_pcie_get_ssid(si_t *sih);
extern uint32 si_pcie_get_bar0(si_t *sih);

#ifndef BCMDONGLEHOST
extern void si_muxenab(si_t *sih, uint32 w);
extern uint32 si_clear_backplane_to(si_t *sih);
extern void si_slave_wrapper_add(si_t *sih);
#endif /* !BCMDONGLEHOST */

#ifdef BCM_BACKPLANE_TIMEOUT
extern const si_axi_error_info_t * si_get_axi_errlog_info(si_t *sih);
extern void si_reset_axi_errlog_info(si_t * sih);
#endif /* BCM_BACKPLANE_TIMEOUT */

extern void si_update_backplane_timeouts(si_t *sih, bool enable, uint32 timeout, uint32 cid);

#if defined(BCMDONGLEHOST)
extern uint32 si_tcm_size(si_t *sih);
extern bool si_has_flops(si_t *sih);
#endif /* BCMDONGLEHOST */

extern int si_set_sromctl(si_t *sih, uint32 value);
extern uint32 si_get_sromctl(si_t *sih);

extern uint32 si_gci_direct(si_t *sih, uint offset, uint32 mask, uint32 val);
extern uint32 si_gci_indirect(si_t *sih, uint regidx, uint offset, uint32 mask, uint32 val);
extern uint32 si_gci_output(si_t *sih, uint reg, uint32 mask, uint32 val);
extern uint32 si_gci_input(si_t *sih, uint reg);
extern uint32 si_gci_int_enable(si_t *sih, bool enable);
extern void si_gci_reset(si_t *sih);
#ifdef BCMLTECOEX
extern void si_ercx_init(si_t *sih, uint32 ltecx_mux, uint32 ltecx_padnum,
	uint32 ltecx_fnsel, uint32 ltecx_gcigpio);
#endif /* BCMLTECOEX */
extern void si_gci_seci_init(si_t *sih);
extern void si_gci_3wire_init(si_t *sih, uint8 btc_wlan_gpio, uint8 btc_bt_gpio,
	uint8 btc_stat_gpio);
extern int si_gci_seci_write_byte(si_t *sih, uint32 write_byte);
extern void si_gci_exchange_eci_info(si_t *sih, uint32 btc_mode);
extern void si_wci2_init(si_t *sih, uint8 baudrate, uint32 ltecx_mux, uint32 ltecx_padnum,
	uint32 ltecx_fnsel, uint32 ltecx_gcigpio, uint32 xtalfreq);

extern void si_gci_set_functionsel(si_t *sih, uint32 pin, uint8 fnsel);
extern uint32 si_gci_get_functionsel(si_t *sih, uint32 pin);
extern void si_gci_clear_functionsel(si_t *sih, uint8 fnsel);
extern uint8 si_gci_get_chipctrlreg_idx(uint32 pin, uint32 *regidx, uint32 *pos);
extern uint32 si_gci_chipcontrol(si_t *sih, uint reg, uint32 mask, uint32 val);
extern uint32 si_gci_chipstatus(si_t *sih, uint reg);
extern uint8 si_enable_device_wake(si_t *sih, uint8 *wake_status, uint8 *cur_status);
extern uint8 si_get_device_wake_opt(si_t *sih);
extern uint8 si_enable_perst_wake(si_t *sih, uint8 *perst_wake_mask, uint8 *perst_cur_status);

extern uint32 si_get_pmu_reg_addr(si_t *sih, uint32 offset);
#define CHIPCTRLREG1 0x1
#define CHIPCTRLREG2 0x2
#define CHIPCTRLREG3 0x3
#define CHIPCTRLREG4 0x4
#define CHIPCTRLREG5 0x5
#define MINRESMASKREG 0x618
#define MAXRESMASKREG 0x61c
#define CHIPCTRLADDR 0x650
#define CHIPCTRLDATA 0x654
#define RSRCTABLEADDR 0x620
#define RSRCUPDWNTIME 0x628
#define PMUREG_RESREQ_MASK 0x68c

extern void si_pcie_prep_D3(si_t *sih, bool enter_D3);
extern void si_pcie_disable_oobselltr(si_t *sih);

/* Macro to enable clock gating changes in different cores */
#define MEM_CLK_GATE_BIT	5
#define GCI_CLK_GATE_BIT	18

#define USBAPP_CLK_BIT		0
#define PCIE_CLK_BIT		3
#define ARMCR4_DBG_CLK_BIT	4
#define SAMPLE_SYNC_CLK_BIT	17
#define PCIE_TL_CLK_BIT		18
#define HQ_REQ_BIT		24
#define PLL_DIV2_BIT_START	9
#define PLL_DIV2_MASK		(0x37 << PLL_DIV2_BIT_START)
#define PLL_DIV2_DIS_OP		(0x37 << PLL_DIV2_BIT_START)

#define pmu_corereg(si, cc_idx, member, mask, val) \
	si_pmu_corereg(si, si_findcoreidx(si, PMU_CORE_ID, 0), \
	               OFFSETOF(pmuregs_t, member), mask, val)

/* Used only for the regs present in the pmu core and not present in the old cc core */
#define PMU_REG_NEW(si, member, mask, val) \
	si_corereg(si, si_findcoreidx(si, PMU_CORE_ID, 0), \
	           OFFSETOF(pmuregs_t, member), mask, val)

#define PMU_REG(si, member, mask, val) \
	si_corereg(si, si_findcoreidx(si, PMU_CORE_ID, 0), \
	           OFFSETOF(pmuregs_t, member), mask, val)

#define GCI_REG(si, offset, mask, val) \
	si_corereg(si, si_findcoreidx(si, GCI_CORE_ID, 0), \
	           offset, mask, val)

/* Used only for the regs present in the gci core and not present in the old cc core */
#define GCI_REG_NEW(si, member, mask, val) \
		si_corereg(si, si_findcoreidx(si, GCI_CORE_ID, 0), \
			OFFSETOF(gciregs_t, member), mask, val)

#define CHIPC_REG_RD(si, member) \
		si_corereg(si, SI_CC_IDX, OFFSETOF(chipcregs_t, member), 0, 0)

#define CHIPC_REG_WR(si, member, mask, val) \
		si_corereg(si, SI_CC_IDX, OFFSETOF(chipcregs_t, member), mask, val)

/* GCI Macros */
#define ALLONES_32				0xFFFFFFFF
#define GCI_CCTL_SECIRST_OFFSET			0 /**< SeciReset */
#define GCI_CCTL_RSTSL_OFFSET			1 /**< ResetSeciLogic */
#define GCI_CCTL_SECIEN_OFFSET			2 /**< EnableSeci  */
#define GCI_CCTL_FSL_OFFSET			3 /**< ForceSeciOutLow */
#define GCI_CCTL_SMODE_OFFSET			4 /**< SeciOpMode, 6:4 */
#define GCI_CCTL_US_OFFSET			7 /**< UpdateSeci */
#define GCI_CCTL_BRKONSLP_OFFSET		8 /**< BreakOnSleep */
#define GCI_CCTL_SILOWTOUT_OFFSET		9 /**< SeciInLowTimeout, 10:9 */
#define GCI_CCTL_RSTOCC_OFFSET			11 /**< ResetOffChipCoex */
#define GCI_CCTL_ARESEND_OFFSET			12 /**< AutoBTSigResend */
#define GCI_CCTL_FGCR_OFFSET			16 /**< ForceGciClkReq */
#define GCI_CCTL_FHCRO_OFFSET			17 /**< ForceHWClockReqOff */
#define GCI_CCTL_FREGCLK_OFFSET			18 /**< ForceRegClk */
#define GCI_CCTL_FSECICLK_OFFSET		19 /**< ForceSeciClk */
#define GCI_CCTL_FGCA_OFFSET			20 /**< ForceGciClkAvail */
#define GCI_CCTL_FGCAV_OFFSET			21 /**< ForceGciClkAvailValue */
#define GCI_CCTL_SCS_OFFSET			24 /**< SeciClkStretch, 31:24 */
#define GCI_CCTL_SCS				25 /* SeciClkStretch */

#define GCI_MODE_UART				0x0
#define GCI_MODE_SECI				0x1
#define GCI_MODE_BTSIG				0x2
#define GCI_MODE_GPIO				0x3
#define GCI_MODE_MASK				0x7

#define GCI_CCTL_LOWTOUT_DIS			0x0
#define GCI_CCTL_LOWTOUT_10BIT			0x1
#define GCI_CCTL_LOWTOUT_20BIT			0x2
#define GCI_CCTL_LOWTOUT_30BIT			0x3
#define GCI_CCTL_LOWTOUT_MASK			0x3

#define GCI_CCTL_SCS_DEF			0x19
#define GCI_CCTL_SCS_MASK			0xFF

#define GCI_SECIIN_MODE_OFFSET			0
#define GCI_SECIIN_GCIGPIO_OFFSET		4
#define GCI_SECIIN_RXID2IP_OFFSET		8

#define GCI_SECIIN_MODE_MASK                    0x7
#define GCI_SECIIN_GCIGPIO_MASK                 0xF

#define GCI_SECIOUT_MODE_OFFSET			0
#define GCI_SECIOUT_GCIGPIO_OFFSET		4
#define	GCI_SECIOUT_LOOPBACK_OFFSET		8
#define GCI_SECIOUT_SECIINRELATED_OFFSET	16

#define GCI_SECIOUT_MODE_MASK                   0x7
#define GCI_SECIOUT_GCIGPIO_MASK                0xF
#define GCI_SECIOUT_SECIINRELATED_MASK          0x1

#define GCI_SECIOUT_SECIINRELATED               0x1

#define GCI_SECIAUX_RXENABLE_OFFSET		0
#define GCI_SECIFIFO_RXENABLE_OFFSET		16

#define GCI_SECITX_ENABLE_OFFSET		0

#define GCI_GPIOCTL_INEN_OFFSET			0
#define GCI_GPIOCTL_OUTEN_OFFSET		1
#define GCI_GPIOCTL_PDN_OFFSET			4

#define GCI_GPIOIDX_OFFSET			16

#define GCI_LTECX_SECI_ID			0 /**< SECI port for LTECX */
#define GCI_LTECX_TXCONF_EN_OFFSET		2
#define GCI_LTECX_PRISEL_EN_OFFSET		3

/* To access per GCI bit registers */
#define GCI_REG_WIDTH				32

/* number of event summary bits */
#define GCI_EVENT_NUM_BITS			32

/* gci event bits per core */
#define GCI_EVENT_BITS_PER_CORE	4
#define GCI_EVENT_HWBIT_1			1
#define GCI_EVENT_HWBIT_2			2
#define GCI_EVENT_SWBIT_1			3
#define GCI_EVENT_SWBIT_2			4

#define GCI_MBDATA_TOWLAN_POS	96
#define GCI_MBACK_TOWLAN_POS	104
#define GCI_WAKE_TOWLAN_PO		112
#define GCI_SWREADY_POS			120

/* GCI bit positions */
/* GCI [127:000] = WLAN [127:0] */
#define GCI_WLAN_IP_ID				0
#define GCI_WLAN_BEGIN				0
#define GCI_WLAN_PRIO_POS			(GCI_WLAN_BEGIN + 4)
#define GCI_WLAN_PERST_POS			(GCI_WLAN_BEGIN + 15)

/* GCI [255:128] = BT [127:0] */
#define GCI_BT_IP_ID					1
#define GCI_BT_BEGIN					128
#define GCI_BT_MBDATA_TOWLAN_POS	(GCI_BT_BEGIN + GCI_MBDATA_TOWLAN_POS)
#define GCI_BT_MBACK_TOWLAN_POS	(GCI_BT_BEGIN + GCI_MBACK_TOWLAN_POS)
#define GCI_BT_WAKE_TOWLAN_POS	(GCI_BT_BEGIN + GCI_WAKE_TOWLAN_PO)
#define GCI_BT_SWREADY_POS			(GCI_BT_BEGIN + GCI_SWREADY_POS)

/* GCI [639:512] = LTE [127:0] */
#define GCI_LTE_IP_ID				4
#define GCI_LTE_BEGIN				512
#define GCI_LTE_FRAMESYNC_POS			(GCI_LTE_BEGIN + 0)
#define GCI_LTE_RX_POS				(GCI_LTE_BEGIN + 1)
#define GCI_LTE_TX_POS				(GCI_LTE_BEGIN + 2)
#define GCI_LTE_WCI2TYPE_POS			(GCI_LTE_BEGIN + 48)
#define GCI_LTE_WCI2TYPE_MASK			7
#define GCI_LTE_AUXRXDVALID_POS			(GCI_LTE_BEGIN + 56)

/* Reg Index corresponding to ECI bit no x of ECI space */
#define GCI_REGIDX(x)				((x)/GCI_REG_WIDTH)
/* Bit offset of ECI bit no x in 32-bit words */
#define GCI_BITOFFSET(x)			((x)%GCI_REG_WIDTH)

/* BT SMEM Control Register 0 */
#define GCI_BT_SMEM_CTRL0_SUBCORE_ENABLE_PKILL	(1 << 28)

/* End - GCI Macros */

#define AXI_OOB		0x7

/*
 * REV133/134 CM Cluster Address
 * DOT11MAC AXI Slave region: 0x2880_0000 - 0x2C7F_FFFF 64MB
 * "AXI Address - 0x2A00_0000" = "CM Host Address"
 * Size(KB)  Host AXI Address
 * VASIP ITCM      64	      0x2A00_0000
 * PSMX ITCM       192	      0x2A04_0000
 * VASIP DTCM      32	      0x2A08_0000
 * PSMX DTCM       256	      0x2A0C_0000
 * CM4 IMEM        64	      0x2A10_0000
 * CM4 DMEM        64	      0x2A14_0000
 * CPUCLUSTER_REGS           0x2A40_0000
 * DMAC_0_REGS               0x2A40_2000
 * DMAC_1_REGS               0x2A40_3000
 * DMAC_2_REGS               0x2A40_4000
 * IPC                       0x2A41_0000
 * Shared Mem      128       0x2A50_0000
 * VASIP CM7 Debug           0x2A70_0000
 * PSMX CM7 Debug            0x2A71_0000
 * CM4 Debug                 0x2A72_0000
 */
#define CM_CLUSTER_ADDR_OFFST		0x01800000 // based on DOT11MAC AXI Slave region
#define CM_CLUSTER_REGS_OFFST       0x00400000 // DMAC_0_REGS
#define CM_IPC_REGS_OFFST           0x00410000 // IPC regs base
#define CM_SHMEM_REGS_OFFST         0x00500000 // Shared Mem
#define CM_SMAC_INTNL_REGS_OFFSET   0x00710000 // SMAC CM7 internal registers
#define CM_MLO_AMGR_REGS_OFFSET     0x00800000 // MLO_AMGR Regs base
#define CM_CPU_DBG_REGS_OFFSET      0x00A00000 // CPU DBG Regs base
#define CM_ICC_OUT_REGS_OFFSET      0x00B00000 // ICC outgoing Regs base

/* for REV 135 (6765): SLAVE region == 0x9400_0000, CM cluster == 0x9600_0000 */
#define CM_CLUSTER_ADDR_OFFST_REV135	0x02000000 // based on DOT11MAC AXI Slave region
#define CM_CLUSTER_HADDR_BASE		0x2A000000 // based on DOT11MAC AXI Slave region - host addr
#define CM_CLUSTER_HADDR_BASE_REV135	0x96000000 // based on DOT11MAC AXI Slave region - host addr
/* all other offsets listed above are correct, but wrt 0x9600_0000 iso 0x2A00_0000 for rev 135 */

/* Common ARM CMx processor xxx_CONTROL_x bit definitions */
#define CM_CONTROL_CORE_EN		(1u << 0) // CORE_ENABLE
#define CM_CONTROL_CORE_RST		(1u << 1) // CORE_RESET
#define CM_CONTROL_CORE_CLK_EN		(1u << 2) // CORE_CLOCK_ENABLE

#define SMAC_INSTR_SEL			0x00040000 // PSMX_ITCM_SEL
#define SMAC_DATA_SEL			0x000c0000 // PSMX_DTCM_SEL
#define SMAC_DATA_SIZE			(64 * 1024)
#define SMAC_DATA_SIZE_REV133		(256 * 1024)
#define SMAC_CONTROL_1			0x4c	   // CM7_CONTROL_1

#define PHYDSP_INSTR_SEL		0x00000000 // PHYDSP_ITCM_SEL
#define PHYDSP_DATA_SEL			0x00080000 // PHYDSP_DTCM_SEL
#define	CM7_CONTROL_0			0x48       // CM7_CONTROL_0

#define MLO_INSTR_SEL			0x00100000 // CM4_ITCM_SEL
#define MLO_DATA_SEL			0x00140000 // CM4_DTCM_SEL
#define MLO_DATA_SIZE			(64 * 1024)
#define MLO_CONTROL_0			0x38	   // CM4_CONTROL_0

#define CM_RATE_MEM_SEL			0x01000000 // Rate Mem - CM
#define CM_IHR1_REGS_OFFSET		0x01900000 // IHR1 - CM
#define CM_IHRX_REGS_OFFSET		0x01a00000 // IHRX - CM

#define CM_REGION_SIZE_1MB		0x00100000 // 1MB region size

#define CM_SRAM_SIZE			(96 * 1024)
#define CM_MLOAMGR_SIZE			(1024 * 1024)
#define CM_MLOICC_OUT_SIZE		(64 * 1024)
#define CM_MLOICC_REGS_SIZE		(512)
#define CM_MLOICC_IN_SIZE		(16 * 1024)

/* ICC reg offset */
#define ICC_GLOBAL_CTL			0x00010004
#define ICC_AXI_M_STCTL0		0x00010018
#define ICC_AXI_M_STCTL0_CACHE_MASK	0x00001E00 /* bits 9-12 */
#define ICC_AXI_M_STCTL0_CACHE_SHIFT	9
#define ICC_AXI_M_L0ADDRL		0x00010020
#define ICC_AXI_M_L0ADDRH		0x00010024
#define ICC_AXI_M_L1ADDRL		0x00010028
#define ICC_AXI_M_L1ADDRH		0x0001002C
#define ICC_AXI_M_L2ADDRL		0x00010030
#define ICC_AXI_M_L2ADDRH		0x00010034
#define ICC_AXI_M_L3ADDRL		0x00010038
#define ICC_AXI_M_L3ADDRH		0x0001003C
#define ICC_CPU_W0ADDRL			0x000100B8
#define ICC_CPU_W0ADDRH			0x000100BC
#define ICC_AXI_S_ADDRMASK		0x00010150

/* DOT11MAC ICC incoming (CM4) region: 0x2831_8000 - 0x2831_BFFF 16KB for 6717/6726 */
#define ICC_AXI_MEM_BASE		0x28318000
#define ICC_AXI_MEM_BASE_REV135		0x90600000
#define ICC_AXI_MEM_AAP_OFFSET		0x1000
#define ICC_AXI_MEM_AAP0_OFFSET		(0)
#define ICC_AXI_MEM_AAP1_OFFSET		(ICC_AXI_MEM_AAP_OFFSET)
#define ICC_AXI_MEM_AAP2_OFFSET		(ICC_AXI_MEM_AAP_OFFSET * 2)

/* MLO_AMGR reg offset */
#define MLO_REGSEL			0x00000000 // MLO_REGSEL
#define MLO_AMGR_SIGCFG			0x00000140 // MLO_SIGCFG
#define MLO_STDBG_CTL0			0x000001E0 // MLO_STDBG_CTL0
#define MLO_STDBG_DAT0			0x000001E8 // MLO_STDBG_DAT0

/* CPU DBG internal hw memory defines for m2mdma programming (wrt CM_CLUSTER) */
#define CM_CPU_DBG_REGS_AXI_OFFSET	0x00A00000 // AXI offset for regs
#define CM_CPU_DBG_BUF_AXI_OFFSET	0x00A10000 // AXI offset for m2mdma
#define CM_CPU_DBG_BUF_SIZE		0x00008000 // CPU DBG hw fifo size
#define CM_CPU_DBG_DMA_HDR_SIZE		16
#define CM_CPU_DBG_ENTRY_SIZE		16
/* xfer size is (1 << x): 7=128; 8=256; 9=512; 10=1024; 11=2048 */
enum cpudbg_xfer_size {
	CM_CPU_DBG_XFER_SIZE_128	= 7,
	CM_CPU_DBG_XFER_SIZE_256	= 8,
	CM_CPU_DBG_XFER_SIZE_512	= 9,
	CM_CPU_DBG_XFER_SIZE_1024	= 10,
	CM_CPU_DBG_XFER_SIZE_2048	= 11,
	CM_CPU_DBG_XFER_SIZE_MAX	= CM_CPU_DBG_XFER_SIZE_2048
};
#define CM_CPU_DBG_XFER_SIZE		CM_CPU_DBG_XFER_SIZE_128 // 128 byte xfer size
#define CPUDBG_HOSTBUF_SZ		(CM_CPU_DBG_BUF_SIZE)
#define CM_CPU_DBG_M2M_IDX		M2MDMA_CORE_1
#define CM_CPU_DBG_M2M_CH		1	   // chan 1 of 2nd m2mdma eng
#define CM_CPU_DBG_M2M_CH_REV133	7	   // chan 7 of 2nd m2mdma eng (revid 133)
#define CM_CPU_DBG_M2M_HDR_TAG_IDX	12	   // tag position in CPUDBG M2M DMA header
#define CM_CPU_DBG_M2M_HDR_SEQNUM_IDX	13	   // seq num position in CPUDBG M2M DMA header

/* CPU DBG defines for HME arena config field */
#define CM_CPU_DBG_CFG_SZ_MASK		0x000f	/* size encoded as (1 << x) */
#define CM_CPU_DBG_CFG_DMA_ENABLE	0x0010	/* DMA enable bit */

/* Would be better to get the defs directly from ucode .h files */
#define CM_CPU_DBG_UT_TYPE_MASK		0x000f
#define CM_CPU_DBG_UT_TYPE_FAULT	0x0a

#define M2MDMA1_AXI_REGS_BASE		0x2800F000
#define M2MDMA1_AXI_REGS_BASE_REV135	0x90007000
#define M2MDMA1_CPU_DBG_WRIDX_OFFSET	0xB1C

/* RNGCSI */
#define RNGCSI_M2M_IDX		1	   // 2nd m2mdma eng (index 1)
#define RNGCSI_M2M_CH		5	   // chan/engine 5 of 2nd m2mdma eng
#define RNGCSI_M2M_NO_CH	-1	   // no chan/engine of 2nd m2mdma eng (for NIC chips)

extern uint si_num_slaveports(si_t *sih, uint coreid);
extern uint32 si_get_slaveport_addr(si_t *sih, uint spidx, uint baidx,
	uint core_id, uint coreunit);
extern uint32 si_get_d11_slaveport_addr(si_t *sih, uint spidx,
	uint baidx, uint coreunit);
uint si_introff(si_t *sih);
void si_intrrestore(si_t *sih, uint intr_val);
void si_nvram_res_masks(si_t *sih, uint32 *min_mask, uint32 *max_mask);
extern uint32 si_get_openloop_dco_code(si_t *sih);
extern void si_set_openloop_dco_code(si_t *sih, uint32 openloop_dco_code);
extern uint32 si_wrapper_dump_last_timeout(si_t *sih, uint32 *error, uint32 *core, uint32 *ba,
	uchar *p);

#ifdef DONGLEBUILD
/* Set the SDIO drive strength */
int si_set_sdio_drive_strength(si_t *sih, uint32 ma);
/* Get the SDIO drive strength */
int si_get_sdio_drive_strength(si_t *sih, uint32 *ma);
extern bool si_check_enable_backplane_log(si_t *sih);
#endif /* DONGLEBUILD */

uint32 si_enum_base_pa(uint devid); /* returns a physical address */

#endif	/* _siutils_h_ */
