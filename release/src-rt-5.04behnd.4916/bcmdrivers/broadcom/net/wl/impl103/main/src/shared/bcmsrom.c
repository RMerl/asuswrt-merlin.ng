/*
 * Routines to access SPROM and to parse SROM/CIS variables.
 *
 * Despite its file name, OTP contents is also parsed in this file.
 *
 * Copyright (C) 2024, Broadcom. All Rights Reserved.
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
 * $Id: bcmsrom.c 833398 2023-11-27 02:49:31Z $
 */

/*
 * List of non obvious preprocessor defines used in this file and their meaning:
 * DONGLEBUILD    : building firmware that runs on the dongle's CPU
 * BCM_DONGLEVARS : NVRAM variables can be read from OTP/S(P)ROM.
 * When host may supply nvram vars in addition to the ones in OTP/SROM:
 * 	BCMHOSTVARS    		: full nic / full dongle
 * BCMDONGLEHOST  : defined when building DHD, code executes on the host in a dongle environment.
 * DHD_SPROM      : defined when building a DHD that supports reading/writing to SPROM
 */

#include <bcm_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmstdarg.h>
#include <bcmutils.h>
#include <hndsoc.h>
#include <sbchipc.h>
#include <bcmdevs.h>
#include <bcmendian.h>
#include <pcicfg.h>
#include <siutils.h>
#include <bcmsrom.h>
#include <bcmsrom_tbl.h>
#ifdef BCMSPI
#include <spid.h>
#endif

#include <bcmnvram.h>
#include <bcmotp.h>

#if defined(WLTEST) || defined(DHD_SPROM) || defined(BCMDBG)
#include <sbsprom.h>
#endif
#include <ethernet.h>	/* for sprom content groking */

#include <sbgci.h>

#if defined(BCMDBG_ERR) || defined(WLTEST)
#define BS_ERROR(args)	printf args
#else
#define BS_ERROR(args)
#endif	/* defined(BCMDBG_ERR) || defined(WLTEST) */

/** a set of primitive operations both applicable to uWire and SPI SPROMs */
enum sprom_op_e {
	SPROM_OP_RD = 0,	/** read a 16 bit word from SPROM */
	SPROM_OP_WR,		/** write a 16 bit word to SPROM */
	SPROM_OP_WRLOCK		/** set or reset SPROM write lock */
};

#if defined(BCMPCIEDEV_SROM_FORMAT) && defined(WLC_TXCAL)
bool BCMATTACHDATA(is_caldata_prsnt) = FALSE;
uint16 BCMATTACHDATA(caldata_array)[SROM_MAX / 2];
#endif

#if !defined(BCMDONGLEHOST) && defined(CMWIFI) && defined(BCMEXTNVM)
extern int BCMATTACHFN(cmwifi_sromvars_map)(si_t *sih, uint chipId, void *buf, uint nbytes);
#endif /* !defined(BCMDONGLEHOST) && defined(CMWIFI) && defined(BCMEXTNVM) */

/**
 * At HW start of day, the lower part of the SPROM is copied to a 512 words (8192 bits) large
 * shadow RAM in the ChipCommon core.
 *
 * @param[in] curmap  Contains host start address of PCI BAR0 window
 *
 * @return  A byte pointer to the start of the 'SROM/OTP shadow area'
 */
uint8 *
srom_offset(si_t *sih, void *curmap)
{
	if ((sih->cccaps & CC_CAP_SROM) == 0)
		return NULL;

#if !defined(DSLCPE) && !defined(DSLCPE_WOMBO)
	if (BUSTYPE(sih->bustype) == SI_BUS)
		return (uint8 *)((uintptr)SI_ENUM_BASE_PA(sih) + CC_SROM_OTP);
#endif /* !DSLCPE && !DSLCPE_WOMBO */

	return (uint8 *)curmap + PCI_16KB0_CCREGS_OFFSET + CC_SROM_OTP;
}

#if defined(WLTEST) || defined(DHD_SPROM) || defined(BCMDBG)
#define WRITE_ENABLE_DELAY	500	/* 500 ms after write enable/disable toggle */
#define WRITE_WORD_DELAY	20	/* 20 ms between each word write */
#endif

extern char *_vars;
extern uint _varsz;
#ifdef DONGLEBUILD
char * BCMATTACHDATA(_vars_otp) = NULL;
#define DONGLE_STORE_VARS_OTP_PTR(v)	(_vars_otp = (v))
#else
#define DONGLE_STORE_VARS_OTP_PTR(v)
#endif

#define SROM_CIS_SINGLE	1

/* srom version check: Current valid versions are: 1-5, 8-11, 12, 13, 15, 16, 17, 18, 19, 20
 * This is a bit mask of all valid SROM versions.
 */
#define VALID_SROMREV   0x001fbf3e

void varbuf_init(varbuf_t *b, char *buf, uint size);
int varbuf_append(varbuf_t *b, const char *fmt, ...);

#if !defined(BCMDONGLEHOST)
#if defined(BCMPCIEDEV)
static int initvars_srom_si_pciedev(si_t *sih, osl_t *osh, void *curmap, char **vars, uint *varsz);
#endif /* BCMPCIEDEV */

void _initvars_srom_pci(uint8 sromrev, uint16 *srom, uint off, varbuf_t *b);
int initvars_srom_pci(si_t *sih, void *curmap, char **vars, uint *count);
static int initvars_cis_pci(si_t *sih, osl_t *osh, void *curmap, char **vars, uint *count);
#if !defined(DONGLEBUILD)
static bool flash_nvram_present(si_t *sih, osl_t *osh, char **vars);
#endif
#endif /* !defined(BCMDONGLEHOST) */

#if !defined(BCMSDIODEV_ENABLED) && !defined(BCMDONGLEHOST) && \
	!defined(BCMPCIEDEV_ENABLED)
static int initvars_si(si_t *sih, char **vars, uint *count);
#endif
#if !defined(BCMDONGLEHOST)
#ifdef BCMSPI
static int initvars_cis_spi(si_t *sih, osl_t *osh, char **vars, uint *count);
#endif /* BCMSPI */
#endif /* !defined(BCMDONGLEHOST) */
int sprom_read_pci(osl_t *osh, si_t *sih, uint16 *sprom, uint wordoff, uint16 *buf,
                          uint nwords, bool check_crc);
#if !defined(BCMDONGLEHOST)
#if defined(BCMNVRAMW) || defined(BCMNVRAMR)
static int otp_read_pci(osl_t *osh, si_t *sih, uint16 *buf, uint bufsz);
#endif /* defined(BCMNVRAMW) || defined(BCMNVRAMR) */
#endif /* !defined(BCMDONGLEHOST) */

static uint16 srom_cc_cmd(si_t *sih, osl_t *osh, chipcregs_t *cc,
                          enum sprom_op_e op, uint wordoff, uint16 data);

#if !defined(BCMDONGLEHOST)
static int initvars_table(osl_t *osh, char *start, char *end, char **vars, uint *count);
static int initvars_only_prefixed(si_t *sih, osl_t *osh, char **vp, uint len);
int dbushost_initvars_flash(si_t *sih, osl_t *osh, char **base, uint len);
static int get_max_cis_size(si_t *sih);
#endif /* !defined(BCMDONGLEHOST) */
uint srom_vars_len(char *vars);

uint16 BCMATTACHDATA(caldata_array)[SROM_MAX/2];

/* BCMHOSTVARS is enabled only if WLTEST is enabled or BCMEXTNVM is enabled */
#if defined(BCMHOSTVARS)
/* Also used by wl_readconfigdata for vars download */
char BCMATTACHDATA(mfgsromvars)[VARS_MAX];
int BCMATTACHDATA(defvarslen) = 0;
#endif

#if !defined(BCMDONGLEHOST)
/* BCMHOSTVARS is enabled only if WLTEST is enabled or BCMEXTNVM is enabled */
#if defined(BCMHOSTVARS)
static char BCMATTACHDATA(defaultsromvars_wltest)[] =
	"macaddr=00:90:4c:f8:00:01\0"
	"et0macaddr=00:11:22:33:44:52\0"
	"et0phyaddr=30\0"
	"et0mdcport=0\0"
	"gpio2=robo_reset\0"
	"boardvendor=0x14e4\0"
	"boardflags=0x210\0"
	"boardflags2=0\0"
	"boardtype=0x04c3\0"
	"boardrev=0x1100\0"
	"sromrev=8\0"
	"devid=0x432c\0"
	"ccode=0\0"
	"regrev=0\0"
	"ledbh0=255\0"
	"ledbh1=255\0"
	"ledbh2=255\0"
	"ledbh3=255\0"
	"leddc=0xffff\0"
	"aa2g=3\0"
	"ag0=2\0"
	"ag1=2\0"
	"aa5g=3\0"
	"aa0=2\0"
	"aa1=2\0"
	"txchain=3\0"
	"rxchain=3\0"
	"antswitch=0\0"
	"itt2ga0=0x20\0"
	"maxp2ga0=0x48\0"
	"pa2gw0a0=0xfe9e\0"
	"pa2gw1a0=0x15d5\0"
	"pa2gw2a0=0xfae9\0"
	"itt2ga1=0x20\0"
	"maxp2ga1=0x48\0"
	"pa2gw0a1=0xfeb3\0"
	"pa2gw1a1=0x15c9\0"
	"pa2gw2a1=0xfaf7\0"
	"tssipos2g=1\0"
	"extpagain2g=0\0"
	"pdetrange2g=0\0"
	"triso2g=3\0"
	"antswctl2g=0\0"
	"tssipos5g=1\0"
	"extpagain5g=0\0"
	"pdetrange5g=0\0"
	"triso5g=3\0"
	"antswctl5g=0\0"
	"cck2gpo=0\0"
	"ofdm2gpo=0\0"
	"mcs2gpo0=0\0"
	"mcs2gpo1=0\0"
	"mcs2gpo2=0\0"
	"mcs2gpo3=0\0"
	"mcs2gpo4=0\0"
	"mcs2gpo5=0\0"
	"mcs2gpo6=0\0"
	"mcs2gpo7=0\0"
	"cddpo=0\0"
	"stbcpo=0\0"
	"bw40po=4\0"
	"bwduppo=0\0"
	"END\0";

/**
 * The contents of this array is a first attempt, was copied from 4365/4366, needs to be edited in
 * a later stage.
 */
static char BCMATTACHDATA(defaultsromvars_43684)[] =
	"sromrev=18\0"
	"subvid=0x14e4\0"
	"vendid=0x14e4\0"
	"devid=0x4429\0" /* 43684 802.11ax 2G/5G dualband device */
	"boardtype=0x84d\0"
	"boardrev=0x1101\0"
	"boardflags=0x10001000\0"
	"boardflags2=0x0\0"
	"boardflags3=0x0\0"
	"boardflags4=0x0\0"
	"boardnum=1\0"
	"macaddr=00:90:4c:1b:00:01\0"
	"ccode=0x0\0"
	"regrev=0\0"
	"aa2g=0xf\0"
	"aa5g=0xf\0"
	"agbg0=133\0"
	"agbg1=133\0"
	"agbg2=133\0"
	"agbg3=133\0"
	"aga0=71\0"
	"aga1=71\0"
	"aga2=71\0"
	"aga3=71\0"
	"txchain=0xf\0"
	"rxchain=0xf\0"
	"antswitch=0\0"
	"tssiposslope2g=1\0"
	"epagain2g=0\0"
	"pdgain2g=0\0"
	"tworangetssi2g=0\0"
	"papdcap2g=0\0"
	"femctrl=2\0"
	"tssiposslope5g=1\0"
	"epagain5g=0\0"
	"pdgain5g=0\0"
	"tworangetssi5g=0\0"
	"papdcap5g=0\0"
	"gainctrlsph=0\0"
	"tempthresh=255\0"
	"tempoffset=255\0"
	"rawtempsense=0x1ff\0"
	"measpower=0\0"
	"tempsense_slope=0xff\0"
	"tempcorrx=0x3f\0"
	"tempsense_option=0x3\0"
	"xtalfreq=40000\0"
	"phycal_tempdelta=40\0"
	"temps_period=1\0"
	"temps_hysteresis=5\0"
	"measpower1=0\0"
	"measpower2=0\0"
	"pdoffset2gcck=0\0"
	"pdoffsetcck20m=0\0"
	"pdoffset20in40m2g=0\0"
	"pdoffset20in40m5gb0=0\0"
	"pdoffset20in40m5gb1=0\0"
	"pdoffset20in40m5gb2=0\0"
	"pdoffset20in40m5gb3=0\0"
	"pdoffset20in40m5gb4=0\0"
	"pdoffset40in80m5gb0=0\0"
	"pdoffset40in80m5gb1=0\0"
	"pdoffset40in80m5gb2=0\0"
	"pdoffset40in80m5gb3=0\0"
	"pdoffset40in80m5gb4=0\0"
	"pdoffset20in80m5gb0=0\0"
	"pdoffset20in80m5gb1=0\0"
	"pdoffset20in80m5gb2=0\0"
	"pdoffset20in80m5gb3=0\0"
	"pdoffset20in80m5gb4=0\0"
	"pdoffset20in40m2gcore3=0\0"
	"pdoffset20in40m5gcore3=0\0"
	"pdoffset20in40m5gcore3_1=0\0"
	"pdoffset40in80m5gcore3=0\0"
	"pdoffset40in80m5gcore3_1=0\0"
	"pdoffset20in80m5gcore3=0\0"
	"pdoffset20in80m5gcore3_1=0\0"
	"subband5gver=0x5\0"
	"mcs1024qam2gpo=0\0"
	"mcs1024qam5glpo=0\0"
	"mcs1024qam5gmpo=0\0"
	"mcs1024qam5ghpo=0\0"
	"mcs1024qam5gx1po=0\0"
	"mcs1024qam5gx2po=0\0"
	"mcs8poexp=0\0"
	"mcs9poexp=0\0"
	"mcs10poexp=0\0"
	"mcs11poexp=0\0"
	"cckbw202gpo=0\0"
	"cckbw20ul2gpo=0\0"
	"mcsbw202gpo=0\0"
	"mcsbw402gpo=0\0"
	"dot11agofdmhrbw202gpo=0\0"
	"ofdmlrbw202gpo=0\0"
	"mcsbw205glpo=0\0"
	"mcsbw405glpo=0\0"
	"mcsbw805glpo=0\0"
	"mcsbw1605glpo=0\0"
	"mcsbw205gmpo=0\0"
	"mcsbw405gmpo=0\0"
	"mcsbw805gmpo=0\0"
	"mcsbw1605gmpo=0\0"
	"mcsbw205ghpo=0\0"
	"mcsbw405ghpo=0\0"
	"mcsbw805ghpo=0\0"
	"mcsbw1605ghpo=0\0"
	"mcsbw205gx1po=0\0"
	"mcsbw405gx1po=0\0"
	"mcsbw805gx1po=0\0"
	"mcsbw1605gx1po=0\0"
	"mcsbw205gx2po=0\0"
	"mcsbw405gx2po=0\0"
	"mcsbw805gx2po=0\0"
	"mcsbw1605gx2po=0\0"
	"mcslr5glpo=0\0"
	"mcslr5gmpo=0\0"
	"mcslr5ghpo=0\0"
	"mcslr5gx1po=0\0"
	"mcslr5gx2po=0\0"
	"sb20in40hrpo=0\0"
	"sb20in40lrpo=0\0"
	"sb20in40hrlrpox=0\0"
	"dot11agduphrpo=0\0"
	"dot11agduplrpo=0\0"
	"dot11agduphrlrpox=0\0"
	"sb40and80hr5glpo=0\0"
	"sb40and80hr5gmpo=0\0"
	"sb40and80hr5ghpo=0\0"
	"sb40and80hr5gx1po=0\0"
	"sb40and80hr5gx2po=0\0"
	"sb20in80and160hr5glpo=0\0"
	"sb20in80and160hr5gmpo=0\0"
	"sb20in80and160hr5ghpo=0\0"
	"sb20in80and160hr5gx1po=0\0"
	"sb20in80and160hr5gx2po=0\0"
	"sb40and80lr5glpo=0\0"
	"sb40and80lr5gmpo=0\0"
	"sb40and80lr5ghpo=0\0"
	"sb40and80lr5gx1po=0\0"
	"sb40and80lr5gx2po=0\0"
	"sb20in80and160lr5glpo=0\0"
	"sb20in80and160lr5gmpo=0\0"
	"sb20in80and160lr5ghpo=0\0"
	"sb20in80and160lr5gx1po=0\0"
	"sb20in80and160lr5gx2po=0\0"
	"pcieingress_war=15\0"
	"sar2g=18\0"
	"sar5g=15\0"
	"noiselvl2ga0=31\0"
	"noiselvl2ga1=31\0"
	"noiselvl2ga2=31\0"
	"noiselvl2ga3=31\0"
	"noiselvl5ga0=31,31,31,31\0"
	"noiselvl5ga1=31,31,31,31\0"
	"noiselvl5ga2=31,31,31,31\0"
	"noiselvl5ga3=31,31,31,31\0"
	"rxgainerr2ga0=63\0"
	"rxgainerr2ga1=31\0"
	"rxgainerr2ga2=31\0"
	"rxgainerr2ga3=31\0"
	"rxgainerr5ga0=63,63,63,63\0"
	"rxgainerr5ga1=31,31,31,31\0"
	"rxgainerr5ga2=31,31,31,31\0"
	"rxgainerr5ga3=31,31,31,31\0"
	"rxgainerr6ga0=63,63,63,63,63,63,63\0"
	"rxgainerr6ga1=31,31,31,31,31,31,31\0"
	"rxgainerr6ga2=31,31,31,31,31,31,31\0"
	"rxgainerr6ga3=31,31,31,31,31,31,31\0"
	"rxgains5gmelnagaina0=7\0"
	"rxgains5gmelnagaina1=7\0"
	"rxgains5gmelnagaina2=7\0"
	"rxgains5gmelnagaina3=7\0"
	"rxgains5gmtrisoa0=15\0"
	"rxgains5gmtrisoa1=15\0"
	"rxgains5gmtrisoa2=15\0"
	"rxgains5gmtrisoa3=15\0"
	"rxgains5gmtrelnabypa0=1\0"
	"rxgains5gmtrelnabypa1=1\0"
	"rxgains5gmtrelnabypa2=1\0"
	"rxgains5gmtrelnabypa3=1\0"
	"rxgains5ghelnagaina0=7\0"
	"rxgains5ghelnagaina1=7\0"
	"rxgains5ghelnagaina2=7\0"
	"rxgains5ghelnagaina3=7\0"
	"rxgains5ghtrisoa0=15\0"
	"rxgains5ghtrisoa1=15\0"
	"rxgains5ghtrisoa2=15\0"
	"rxgains5ghtrisoa3=15\0"
	"rxgains5ghtrelnabypa0=1\0"
	"rxgains5ghtrelnabypa1=1\0"
	"rxgains5ghtrelnabypa2=1\0"
	"rxgains5ghtrelnabypa3=1\0"
	"rxgains6gband0elnagaina0=7\0"
	"rxgains6gband0elnagaina1=7\0"
	"rxgains6gband0elnagaina2=7\0"
	"rxgains6gband0elnagaina3=7\0"
	"rxgains6gband0trisoa0=15\0"
	"rxgains6gband0trisoa1=15\0"
	"rxgains6gband0trisoa2=15\0"
	"rxgains6gband0trisoa3=15\0"
	"rxgains6gband0trelnabypa0=1\0"
	"rxgains6gband0trelnabypa1=1\0"
	"rxgains6gband0trelnabypa2=1\0"
	"rxgains6gband0trelnabypa3=1\0"
	"rxgains6gband1elnagaina0=7\0"
	"rxgains6gband1elnagaina1=7\0"
	"rxgains6gband1elnagaina2=7\0"
	"rxgains6gband1elnagaina3=7\0"
	"rxgains6gband1trisoa0=15\0"
	"rxgains6gband1trisoa1=15\0"
	"rxgains6gband1trisoa2=15\0"
	"rxgains6gband1trisoa3=15\0"
	"rxgains6gband1trelnabypa0=1\0"
	"rxgains6gband1trelnabypa1=1\0"
	"rxgains6gband1trelnabypa2=1\0"
	"rxgains6gband1trelnabypa3=1\0"
	"rxgains6gband2elnagaina0=7\0"
	"rxgains6gband2elnagaina1=7\0"
	"rxgains6gband2elnagaina2=7\0"
	"rxgains6gband2elnagaina3=7\0"
	"rxgains6gband2trisoa0=15\0"
	"rxgains6gband2trisoa1=15\0"
	"rxgains6gband2trisoa2=15\0"
	"rxgains6gband2trisoa3=15\0"
	"rxgains6gband2trelnabypa0=1\0"
	"rxgains6gband2trelnabypa1=1\0"
	"rxgains6gband2trelnabypa2=1\0"
	"rxgains6gband2trelnabypa3=1\0"
	"rxgains6gband3elnagaina0=7\0"
	"rxgains6gband3elnagaina1=7\0"
	"rxgains6gband3elnagaina2=7\0"
	"rxgains6gband3elnagaina3=7\0"
	"rxgains6gband3trisoa0=15\0"
	"rxgains6gband3trisoa1=15\0"
	"rxgains6gband3trisoa2=15\0"
	"rxgains6gband3trisoa3=15\0"
	"rxgains6gband3trelnabypa0=1\0"
	"rxgains6gband3trelnabypa1=1\0"
	"rxgains6gband3trelnabypa2=1\0"
	"rxgains6gband3trelnabypa3=1\0"
	"rxgains6gband4elnagaina0=7\0"
	"rxgains6gband4elnagaina1=7\0"
	"rxgains6gband4elnagaina2=7\0"
	"rxgains6gband4elnagaina3=7\0"
	"rxgains6gband4trisoa0=15\0"
	"rxgains6gband4trisoa1=15\0"
	"rxgains6gband4trisoa2=15\0"
	"rxgains6gband4trisoa3=15\0"
	"rxgains6gband4trelnabypa0=1\0"
	"rxgains6gband4trelnabypa1=1\0"
	"rxgains6gband4trelnabypa2=1\0"
	"rxgains6gband4trelnabypa3=1\0"
	"rxgains6gband5elnagaina0=7\0"
	"rxgains6gband5elnagaina1=7\0"
	"rxgains6gband5elnagaina2=7\0"
	"rxgains6gband5elnagaina3=7\0"
	"rxgains6gband5trisoa0=15\0"
	"rxgains6gband5trisoa1=15\0"
	"rxgains6gband5trisoa2=15\0"
	"rxgains6gband5trisoa3=15\0"
	"rxgains6gband5trelnabypa0=1\0"
	"rxgains6gband5trelnabypa1=1\0"
	"rxgains6gband5trelnabypa2=1\0"
	"rxgains6gband5trelnabypa3=1\0"
	"rxgains6gband6elnagaina0=7\0"
	"rxgains6gband6elnagaina1=7\0"
	"rxgains6gband6elnagaina2=7\0"
	"rxgains6gband6elnagaina3=7\0"
	"rxgains6gband6trisoa0=15\0"
	"rxgains6gband6trisoa1=15\0"
	"rxgains6gband6trisoa2=15\0"
	"rxgains6gband6trisoa3=15\0"
	"rxgains6gband6trelnabypa0=1\0"
	"rxgains6gband6trelnabypa1=1\0"
	"rxgains6gband6trelnabypa2=1\0"
	"rxgains6gband6trelnabypa3=1\0"
	"maxp2ga0=84\0"
	"maxp5gb0a0=72\0"
	"maxp5gb1a0=72\0"
	"maxp5gb2a0=72\0"
	"maxp5gb3a0=72\0"
	"maxp5gb4a0=72\0"
	"maxp2ga1=84\0"
	"maxp5gb0a1=72\0"
	"maxp5gb1a1=72\0"
	"maxp5gb2a1=72\0"
	"maxp5gb3a1=72\0"
	"maxp5gb4a1=72\0"
	"maxp2ga2=84\0"
	"maxp5gb0a2=72\0"
	"maxp5gb1a2=72\0"
	"maxp5gb2a2=72\0"
	"maxp5gb3a2=72\0"
	"maxp5gb4a2=72\0"
	"maxp2ga3=84\0"
	"maxp5gb0a3=72\0"
	"maxp5gb1a3=72\0"
	"maxp5gb2a3=72\0"
	"maxp5gb3a3=72\0"
	"maxp5gb4a3=72\0"
	"rxgains2gelnagaina0=4\0"
	"rxgains2gtrisoa0=7\0"
	"rxgains2gtrelnabypa0=1\0"
	"rxgains5gelnagaina0=2\0"
	"rxgains5gtrisoa0=6\0"
	"rxgains5gtrelnabypa0=1\0"
	"rxgains2gelnagaina1=4\0"
	"rxgains2gtrisoa1=7\0"
	"rxgains2gtrelnabypa1=1\0"
	"rxgains5gelnagaina1=2\0"
	"rxgains5gtrisoa1=6\0"
	"rxgains5gtrelnabypa1=1\0"
	"rxgains2gelnagaina2=4\0"
	"rxgains2gtrisoa2=7\0"
	"rxgains2gtrelnabypa2=1\0"
	"rxgains5gelnagaina2=2\0"
	"rxgains5gtrisoa2=6\0"
	"rxgains5gtrelnabypa2=1\0"
	"rxgains2gelnagaina3=4\0"
	"rxgains2gtrisoa3=7\0"
	"rxgains2gtrelnabypa3=1\0"
	"rxgains5gelnagaina3=2\0"
	"rxgains5gtrisoa3=6\0"
	"rxgains5gtrelnabypa3=1\0"
	"pa5ga0=0x1ab9,0xc3bb,0xfe1a,0x0,0x1af7,0xc324,0xfe74,0x0,0x1b39,0xc23c,"
	"0xff2b,0x0,0x1b72,0xc198,0xff9d,0x0,0x1b8c,0xc2ad,0xff24,0x0\0"
	"pa5g40a0=0x1b7b,0xd218,0xf58a,0x0,0x1b98,0xd17b,0xf5e4,0x0,0x1bfb,0xcfee,"
	"0xf6d2,0x0,0x1c01,0xd08d,0xf6c3,0x0,0x1bf7,0xd26a,0xf5ff,0x0\0"
	"pa5g80a0=0x1ae9,0xcddb,0xf8a0,0x0,0x1afd,0xcdf8,0xf883,0x0,0x1b45,0xccdc,"
	"0xf956,0x0,0x1b44,0xcea2,0xf8c3,0x0,0x1b95,0xcd47,0xf95e,0x0\0"
	"pa5ga1=0x1a80,0xc2d0,0xff08,0x0,0x1aa0,0xc27f,0xff42,0x0,0x1afb,0xc208,"
	"0xffdd,0x0,0x1b24,0xc255,0xffcd,0x0,0x1b5c,0xc214,0x0,0x0\0"
	"pa5g40a1=0x1b31,0xd1ab,0xf630,0x0,0x1b52,0xd095,0xf6ce,0x0,0x1ba3,0xd071,"
	"0xf735,0x0,0x1bd1,0xd066,0xf746,0x0,0x1bf2,0xd0e2,0xf711,0x0\0"
	"pa5g80a1=0x1aa6,0xcd0b,0xf97e,0x0,0x1abb,0xcc7d,0xf9e5,0x0,0x1af5,0xccdb,"
	"0xf9e0,0x0,0x1b32,0xcd75,0xf9b0,0x0,0x1b45,0xcde6,0xf977,0x0\0"
	"pa5ga2=0x1b1e,0xc1ba,0xff54,0x0,0x1b2c,0xc18c,0xff76,0x0,0x1b51,0xc204,"
	"0xff7f,0x0,0x1b79,0xc1ed,0xffa1,0x0,0x1bc2,0xc17b,0xffe0,0x0\0"
	"pa5g40a2=0x1bba,0xd163,0xf602,0x0,0x1be0,0xd020,0xf6ac,0x0,0x1c0f,0xd04b,"
	"0xf6da,0x0,0x1c25,0xd07b,0xf6db,0x0,0x1c28,0xd232,0xf623,0x0\0"
	"pa5g80a2=0x1b16,0xcd7d,0xf8f5,0x0,0x1b07,0xce18,0xf8c8,0x0,0x1b50,0xcd30,"
	"0xf963,0x0,0x1b72,0xce09,0xf931,0x0,0x1ba0,0xcd76,0xf969,0x0\0"
	"pa5ga3=0x1b1e,0xc1ba,0xff54,0x0,0x1b2c,0xc18c,0xff76,0x0,0x1b51,0xc204,"
	"0xff7f,0x0,0x1b79,0xc1ed,0xffa1,0x0,0x1bc2,0xc17b,0xffe0,0x0\0"
	"pa5g40a3=0x1bba,0xd163,0xf602,0x0,0x1be0,0xd020,0xf6ac,0x0,0x1c0f,0xd04b,"
	"0xf6da,0x0,0x1c25,0xd07b,0xf6db,0x0,0x1c28,0xd232,0xf623,0x0\0"
	"pa5g80a3=0x1b16,0xcd7d,0xf8f5,0x0,0x1b07,0xce18,0xf8c8,0x0,0x1b50,0xcd30,"
	"0xf963,0x0,0x1b72,0xce09,0xf931,0x0,0x1ba0,0xcd76,0xf969,0x0\0"
	"pa2ga0=0x1c9b,0xbef2,0x0,0xba7\0"
	"pa2g40a0=0x1c9b,0xbef2,0x0,0xba7\0"
	"pa2ga1=0x1c3f,0xbec6,0x0,0x0\0"
	"pa2g40a1=0xffff,0xffff,0xffff,0xffff\0"
	"pa2ga2=0x1d05,0xbe32,0x0,0xaf2\0"
	"pa2g40a2=0xffff,0xffff,0xffff,0xffff\0"
	"pa2ga3=0x1d05,0xbe32,0x0,0xaf2\0"
	"pa2g40a3=0xffff,0xffff,0xffff,0xffff\0"
	"rpcal2g=0\0"
	"rpcal5gb0=0\0"
	"rpcal5gb1=0\0"
	"rpcal5gb2=0\0"
	"rpcal5gb3=0\0"
	"rpcal5gb4=0\0"
	"rpcal2gcore3=0\0"
	"rpcal5gb0core3=0\0"
	"rpcal5gb1core3=0\0"
	"rpcal5gb2core3=0\0"
	"rpcal5gb3core3=0\0"
	"swctrlmap4_cfg=0x1\0"
	"swctrlmap4_TX2g_fem3to0=0x36bd\0"
	"swctrlmap4_RX2g_fem3to0=0x0080\0"
	"swctrlmap4_RXByp2g_fem3to0=0x24a4\0"
	"swctrlmap4_misc2g_fem3to0=0x24a4\0"
	"swctrlmap4_TX5g_fem3to0=0x7ebd\0"
	"swctrlmap4_RX5g_fem3to0=0x4880\0"
	"swctrlmap4_RXByp5g_fem3to0=0x6ca4\0"
	"swctrlmap4_misc5g_fem3to0=0x6ca4\0"
	"swctrlmap4_TX2g_fem7to4=0\0"
	"swctrlmap4_RX2g_fem7to4=0\0"
	"swctrlmap4_RXByp2g_fem7to4=0\0"
	"swctrlmap4_misc2g_fem7to4=0\0"
	"swctrlmap4_TX5g_fem7to4=0\0"
	"swctrlmap4_RX5g_fem7to4=0\0"
	"swctrlmap4_RXByp5g_fem7to4=0\0"
	"swctrlmap4_misc5g_fem7to4=0\0"
	"END\0";

/**
 * The contents of this array is a first attempt, is likely incorrect for 6710, needs to be
 * edited in a later stage.
 */
static char BCMATTACHDATA(defaultsromvars_6710)[] =
	"sromrev=18\0"
	"subvid=0x14e4\0"
	"vendid=0x14e4\0"
	"devid=0x6710\0" /* 43684 802.11ax 2G/5G dualband device */
	"boardtype=0x0\0"
	"boardrev=0x1101\0"
	"boardflags=0x10001000\0"
	"boardflags2=0x0\0"
	"boardflags3=0x0\0"
	"boardflags4=0x0\0"
	"boardnum=1\0"
	"macaddr=00:90:4c:1b:00:01\0"
	"ccode=0x0\0"
	"regrev=0\0"
	"aa2g=0x7\0"
	"aa5g=0x7\0"
	"agbg0=133\0"
	"agbg1=133\0"
	"agbg2=133\0"
	"aga0=71\0"
	"aga1=71\0"
	"aga2=71\0"
	"txchain=0x7\0"
	"rxchain=0x7\0"
	"antswitch=0\0"
	"tssiposslope2g=1\0"
	"epagain2g=0\0"
	"pdgain2g=0\0"
	"tworangetssi2g=0\0"
	"papdcap2g=0\0"
	"femctrl=2\0"
	"tssiposslope5g=1\0"
	"epagain5g=0\0"
	"pdgain5g=0\0"
	"tworangetssi5g=0\0"
	"papdcap5g=0\0"
	"gainctrlsph=0\0"
	"tempthresh=255\0"
	"tempoffset=255\0"
	"rawtempsense=0x1ff\0"
	"measpower=0\0"
	"tempsense_slope=0xff\0"
	"tempcorrx=0x3f\0"
	"tempsense_option=0x3\0"
	"xtalfreq=54000\0"
	"phycal_tempdelta=40\0"
	"temps_period=1\0"
	"temps_hysteresis=5\0"
	"measpower1=0\0"
	"measpower2=0\0"
	"pdoffset2gcck=0\0"
	"pdoffsetcck20m=0\0"
	"pdoffset20in40m2g=0\0"
	"pdoffset20in40m5gb0=0\0"
	"pdoffset20in40m5gb1=0\0"
	"pdoffset20in40m5gb2=0\0"
	"pdoffset20in40m5gb3=0\0"
	"pdoffset20in40m5gb4=0\0"
	"pdoffset40in80m5gb0=0\0"
	"pdoffset40in80m5gb1=0\0"
	"pdoffset40in80m5gb2=0\0"
	"pdoffset40in80m5gb3=0\0"
	"pdoffset40in80m5gb4=0\0"
	"pdoffset20in80m5gb0=0\0"
	"pdoffset20in80m5gb1=0\0"
	"pdoffset20in80m5gb2=0\0"
	"pdoffset20in80m5gb3=0\0"
	"pdoffset20in80m5gb4=0\0"
	"pdoffset20in40m2gcore3=0\0"
	"pdoffset20in40m5gcore3=0\0"
	"pdoffset20in40m5gcore3_1=0\0"
	"pdoffset40in80m5gcore3=0\0"
	"pdoffset40in80m5gcore3_1=0\0"
	"pdoffset20in80m5gcore3=0\0"
	"pdoffset20in80m5gcore3_1=0\0"
	"subband5gver=0x5\0"
	"mcs1024qam2gpo=0\0"
	"mcs1024qam5glpo=0\0"
	"mcs1024qam5gmpo=0\0"
	"mcs1024qam5ghpo=0\0"
	"mcs1024qam5gx1po=0\0"
	"mcs1024qam5gx2po=0\0"
	"mcs8poexp=0\0"
	"mcs9poexp=0\0"
	"mcs10poexp=0\0"
	"mcs11poexp=0\0"
	"cckbw202gpo=0\0"
	"cckbw20ul2gpo=0\0"
	"mcsbw202gpo=0\0"
	"mcsbw402gpo=0\0"
	"dot11agofdmhrbw202gpo=0\0"
	"ofdmlrbw202gpo=0\0"
	"mcsbw205glpo=0\0"
	"mcsbw405glpo=0\0"
	"mcsbw805glpo=0\0"
	"mcsbw1605glpo=0\0"
	"mcsbw205gmpo=0\0"
	"mcsbw405gmpo=0\0"
	"mcsbw805gmpo=0\0"
	"mcsbw1605gmpo=0\0"
	"mcsbw205ghpo=0\0"
	"mcsbw405ghpo=0\0"
	"mcsbw805ghpo=0\0"
	"mcsbw1605ghpo=0\0"
	"mcsbw205gx1po=0\0"
	"mcsbw405gx1po=0\0"
	"mcsbw805gx1po=0\0"
	"mcsbw1605gx1po=0\0"
	"mcsbw205gx2po=0\0"
	"mcsbw405gx2po=0\0"
	"mcsbw805gx2po=0\0"
	"mcsbw1605gx2po=0\0"
	"mcslr5glpo=0\0"
	"mcslr5gmpo=0\0"
	"mcslr5ghpo=0\0"
	"mcslr5gx1po=0\0"
	"mcslr5gx2po=0\0"
	"sb20in40hrpo=0\0"
	"sb20in40lrpo=0\0"
	"sb20in40hrlrpox=0\0"
	"dot11agduphrpo=0\0"
	"dot11agduplrpo=0\0"
	"dot11agduphrlrpox=0\0"
	"sb40and80hr5glpo=0\0"
	"sb40and80hr5gmpo=0\0"
	"sb40and80hr5ghpo=0\0"
	"sb40and80hr5gx1po=0\0"
	"sb40and80hr5gx2po=0\0"
	"sb20in80and160hr5glpo=0\0"
	"sb20in80and160hr5gmpo=0\0"
	"sb20in80and160hr5ghpo=0\0"
	"sb20in80and160hr5gx1po=0\0"
	"sb20in80and160hr5gx2po=0\0"
	"sb40and80lr5glpo=0\0"
	"sb40and80lr5gmpo=0\0"
	"sb40and80lr5ghpo=0\0"
	"sb40and80lr5gx1po=0\0"
	"sb40and80lr5gx2po=0\0"
	"sb20in80and160lr5glpo=0\0"
	"sb20in80and160lr5gmpo=0\0"
	"sb20in80and160lr5ghpo=0\0"
	"sb20in80and160lr5gx1po=0\0"
	"sb20in80and160lr5gx2po=0\0"
	"pcieingress_war=15\0"
	"sar2g=18\0"
	"sar5g=15\0"
	"noiselvl2ga0=31\0"
	"noiselvl2ga1=31\0"
	"noiselvl2ga2=31\0"
	"noiselvl2ga3=31\0"
	"noiselvl5ga0=31,31,31,31\0"
	"noiselvl5ga1=31,31,31,31\0"
	"noiselvl5ga2=31,31,31,31\0"
	"noiselvl5ga3=31,31,31,31\0"
	"rxgainerr2ga0=63\0"
	"rxgainerr2ga1=31\0"
	"rxgainerr2ga2=31\0"
	"rxgainerr2ga3=31\0"
	"rxgainerr5ga0=63,63,63,63\0"
	"rxgainerr5ga1=31,31,31,31\0"
	"rxgainerr5ga2=31,31,31,31\0"
	"rxgainerr5ga3=31,31,31,31\0"
	"rxgains5gmelnagaina0=7\0"
	"rxgains5gmelnagaina1=7\0"
	"rxgains5gmelnagaina2=7\0"
	"rxgains5gmelnagaina3=7\0"
	"rxgains5gmtrisoa0=15\0"
	"rxgains5gmtrisoa1=15\0"
	"rxgains5gmtrisoa2=15\0"
	"rxgains5gmtrisoa3=15\0"
	"rxgains5gmtrelnabypa0=1\0"
	"rxgains5gmtrelnabypa1=1\0"
	"rxgains5gmtrelnabypa2=1\0"
	"rxgains5gmtrelnabypa3=1\0"
	"rxgains5ghelnagaina0=7\0"
	"rxgains5ghelnagaina1=7\0"
	"rxgains5ghelnagaina2=7\0"
	"rxgains5ghelnagaina3=7\0"
	"rxgains5ghtrisoa0=15\0"
	"rxgains5ghtrisoa1=15\0"
	"rxgains5ghtrisoa2=15\0"
	"rxgains5ghtrisoa3=15\0"
	"rxgains5ghtrelnabypa0=1\0"
	"rxgains5ghtrelnabypa1=1\0"
	"rxgains5ghtrelnabypa2=1\0"
	"rxgains5ghtrelnabypa3=1\0"
	"maxp2ga0=84\0"
	"maxp5gb0a0=72\0"
	"maxp5gb1a0=72\0"
	"maxp5gb2a0=72\0"
	"maxp5gb3a0=72\0"
	"maxp5gb4a0=72\0"
	"maxp2ga1=84\0"
	"maxp5gb0a1=72\0"
	"maxp5gb1a1=72\0"
	"maxp5gb2a1=72\0"
	"maxp5gb3a1=72\0"
	"maxp5gb4a1=72\0"
	"maxp2ga2=84\0"
	"maxp5gb0a2=72\0"
	"maxp5gb1a2=72\0"
	"maxp5gb2a2=72\0"
	"maxp5gb3a2=72\0"
	"maxp5gb4a2=72\0"
	"maxp2ga3=84\0"
	"maxp5gb0a3=72\0"
	"maxp5gb1a3=72\0"
	"maxp5gb2a3=72\0"
	"maxp5gb3a3=72\0"
	"maxp5gb4a3=72\0"
	"rxgains2gelnagaina0=4\0"
	"rxgains2gtrisoa0=7\0"
	"rxgains2gtrelnabypa0=1\0"
	"rxgains5gelnagaina0=2\0"
	"rxgains5gtrisoa0=6\0"
	"rxgains5gtrelnabypa0=1\0"
	"rxgains2gelnagaina1=4\0"
	"rxgains2gtrisoa1=7\0"
	"rxgains2gtrelnabypa1=1\0"
	"rxgains5gelnagaina1=2\0"
	"rxgains5gtrisoa1=6\0"
	"rxgains5gtrelnabypa1=1\0"
	"rxgains2gelnagaina2=4\0"
	"rxgains2gtrisoa2=7\0"
	"rxgains2gtrelnabypa2=1\0"
	"rxgains5gelnagaina2=2\0"
	"rxgains5gtrisoa2=6\0"
	"rxgains5gtrelnabypa2=1\0"
	"rxgains2gelnagaina3=4\0"
	"rxgains2gtrisoa3=7\0"
	"rxgains2gtrelnabypa3=1\0"
	"rxgains5gelnagaina3=2\0"
	"rxgains5gtrisoa3=6\0"
	"rxgains5gtrelnabypa3=1\0"
	"pa5ga0=0x1ab9,0xc3bb,0xfe1a,0x0,0x1af7,0xc324,0xfe74,0x0,0x1b39,0xc23c,"
	"0xff2b,0x0,0x1b72,0xc198,0xff9d,0x0,0x1b8c,0xc2ad,0xff24,0x0\0"
	"pa5g40a0=0x1b7b,0xd218,0xf58a,0x0,0x1b98,0xd17b,0xf5e4,0x0,0x1bfb,0xcfee,"
	"0xf6d2,0x0,0x1c01,0xd08d,0xf6c3,0x0,0x1bf7,0xd26a,0xf5ff,0x0\0"
	"pa5g80a0=0x1ae9,0xcddb,0xf8a0,0x0,0x1afd,0xcdf8,0xf883,0x0,0x1b45,0xccdc,"
	"0xf956,0x0,0x1b44,0xcea2,0xf8c3,0x0,0x1b95,0xcd47,0xf95e,0x0\0"
	"pa5ga1=0x1a80,0xc2d0,0xff08,0x0,0x1aa0,0xc27f,0xff42,0x0,0x1afb,0xc208,"
	"0xffdd,0x0,0x1b24,0xc255,0xffcd,0x0,0x1b5c,0xc214,0x0,0x0\0"
	"pa5g40a1=0x1b31,0xd1ab,0xf630,0x0,0x1b52,0xd095,0xf6ce,0x0,0x1ba3,0xd071,"
	"0xf735,0x0,0x1bd1,0xd066,0xf746,0x0,0x1bf2,0xd0e2,0xf711,0x0\0"
	"pa5g80a1=0x1aa6,0xcd0b,0xf97e,0x0,0x1abb,0xcc7d,0xf9e5,0x0,0x1af5,0xccdb,"
	"0xf9e0,0x0,0x1b32,0xcd75,0xf9b0,0x0,0x1b45,0xcde6,0xf977,0x0\0"
	"pa5ga2=0x1b1e,0xc1ba,0xff54,0x0,0x1b2c,0xc18c,0xff76,0x0,0x1b51,0xc204,"
	"0xff7f,0x0,0x1b79,0xc1ed,0xffa1,0x0,0x1bc2,0xc17b,0xffe0,0x0\0"
	"pa5g40a2=0x1bba,0xd163,0xf602,0x0,0x1be0,0xd020,0xf6ac,0x0,0x1c0f,0xd04b,"
	"0xf6da,0x0,0x1c25,0xd07b,0xf6db,0x0,0x1c28,0xd232,0xf623,0x0\0"
	"pa5g80a2=0x1b16,0xcd7d,0xf8f5,0x0,0x1b07,0xce18,0xf8c8,0x0,0x1b50,0xcd30,"
	"0xf963,0x0,0x1b72,0xce09,0xf931,0x0,0x1ba0,0xcd76,0xf969,0x0\0"
	"pa5ga3=0x1b1e,0xc1ba,0xff54,0x0,0x1b2c,0xc18c,0xff76,0x0,0x1b51,0xc204,"
	"0xff7f,0x0,0x1b79,0xc1ed,0xffa1,0x0,0x1bc2,0xc17b,0xffe0,0x0\0"
	"pa5g40a3=0x1bba,0xd163,0xf602,0x0,0x1be0,0xd020,0xf6ac,0x0,0x1c0f,0xd04b,"
	"0xf6da,0x0,0x1c25,0xd07b,0xf6db,0x0,0x1c28,0xd232,0xf623,0x0\0"
	"pa5g80a3=0x1b16,0xcd7d,0xf8f5,0x0,0x1b07,0xce18,0xf8c8,0x0,0x1b50,0xcd30,"
	"0xf963,0x0,0x1b72,0xce09,0xf931,0x0,0x1ba0,0xcd76,0xf969,0x0\0"
	"pa2ga0=0x1c9b,0xbef2,0x0,0xba7\0"
	"pa2g40a0=0x1c9b,0xbef2,0x0,0xba7\0"
	"pa2ga1=0x1c3f,0xbec6,0x0,0x0\0"
	"pa2g40a1=0xffff,0xffff,0xffff,0xffff\0"
	"pa2ga2=0x1d05,0xbe32,0x0,0xaf2\0"
	"pa2g40a2=0xffff,0xffff,0xffff,0xffff\0"
	"pa2ga3=0x1d05,0xbe32,0x0,0xaf2\0"
	"pa2g40a3=0xffff,0xffff,0xffff,0xffff\0"
	"rpcal2g=0\0"
	"rpcal5gb0=0\0"
	"rpcal5gb1=0\0"
	"rpcal5gb2=0\0"
	"rpcal5gb3=0\0"
	"rpcal5gb4=0\0"
	"rpcal2gcore3=0\0"
	"rpcal5gb0core3=0\0"
	"rpcal5gb1core3=0\0"
	"rpcal5gb2core3=0\0"
	"rpcal5gb3core3=0\0"
	"swctrlmap4_cfg=0x1\0"
	"swctrlmap4_TX2g_fem3to0=0x36bd\0"
	"swctrlmap4_RX2g_fem3to0=0x0080\0"
	"swctrlmap4_RXByp2g_fem3to0=0x24a4\0"
	"swctrlmap4_misc2g_fem3to0=0x24a4\0"
	"swctrlmap4_TX5g_fem3to0=0x7ebd\0"
	"swctrlmap4_RX5g_fem3to0=0x4880\0"
	"swctrlmap4_RXByp5g_fem3to0=0x6ca4\0"
	"swctrlmap4_misc5g_fem3to0=0x6ca4\0"
	"swctrlmap4_TX2g_fem7to4=0\0"
	"swctrlmap4_RX2g_fem7to4=0\0"
	"swctrlmap4_RXByp2g_fem7to4=0\0"
	"swctrlmap4_misc2g_fem7to4=0\0"
	"swctrlmap4_TX5g_fem7to4=0\0"
	"swctrlmap4_RX5g_fem7to4=0\0"
	"swctrlmap4_RXByp5g_fem7to4=0\0"
	"swctrlmap4_misc5g_fem7to4=0\0"
	"END\0";

/**
 * The contents of this array is a first attempt, is likely incorrect for 6715, needs to be
 * edited in a later stage.
 */
static char BCMATTACHDATA(defaultsromvars_6715)[] =
	"sromrev=18\0"
	"subvid=0x14e4\0"
	"vendid=0x14e4\0"
	"devid=0x6017\0" /* 6715 802.11ax 2G/5G/6G triband device */
	"boardtype=0x0\0"
	"boardrev=0x1101\0"
	"boardflags=0x10001000\0"
	"boardflags2=0x0\0"
	"boardflags3=0x0\0"
	"boardflags4=0x0\0"
	"boardnum=1\0"
	"macaddr=00:90:4c:1b:00:01\0"
	"ccode=0x0\0"
	"regrev=0\0"
	"aa2g=0xf\0"
	"aa5g=0xf\0"
	"agbg0=133\0"
	"agbg1=133\0"
	"agbg2=133\0"
	"aga0=71\0"
	"aga1=71\0"
	"aga2=71\0"
	"txchain=0xf\0"
	"rxchain=0xf\0"
	"antswitch=0\0"
	"tssiposslope2g=1\0"
	"epagain2g=0\0"
	"pdgain2g=0\0"
	"tworangetssi2g=0\0"
	"papdcap2g=0\0"
	"femctrl=2\0"
	"tssiposslope5g=1\0"
	"epagain5g=0\0"
	"pdgain5g=0\0"
	"tworangetssi5g=0\0"
	"papdcap5g=0\0"
	"gainctrlsph=0\0"
	"tempthresh=255\0"
	"tempoffset=255\0"
	"rawtempsense=0x1ff\0"
	"measpower=0\0"
	"tempsense_slope=0xff\0"
	"tempcorrx=0x3f\0"
	"tempsense_option=0x3\0"
	"xtalfreq=50000\0"
	"phycal_tempdelta=40\0"
	"temps_period=1\0"
	"temps_hysteresis=5\0"
	"measpower1=0\0"
	"measpower2=0\0"
	"pdoffset2gcck=0\0"
	"pdoffsetcck20m=0\0"
	"pdoffset20in40m2g=0\0"
	"pdoffset20in40m5gb0=0\0"
	"pdoffset20in40m5gb1=0\0"
	"pdoffset20in40m5gb2=0\0"
	"pdoffset20in40m5gb3=0\0"
	"pdoffset20in40m5gb4=0\0"
	"pdoffset40in80m5gb0=0\0"
	"pdoffset40in80m5gb1=0\0"
	"pdoffset40in80m5gb2=0\0"
	"pdoffset40in80m5gb3=0\0"
	"pdoffset40in80m5gb4=0\0"
	"pdoffset20in80m5gb0=0\0"
	"pdoffset20in80m5gb1=0\0"
	"pdoffset20in80m5gb2=0\0"
	"pdoffset20in80m5gb3=0\0"
	"pdoffset20in80m5gb4=0\0"
	"pdoffset20in40m2gcore3=0\0"
	"pdoffset20in40m5gcore3=0\0"
	"pdoffset20in40m5gcore3_1=0\0"
	"pdoffset40in80m5gcore3=0\0"
	"pdoffset40in80m5gcore3_1=0\0"
	"pdoffset20in80m5gcore3=0\0"
	"pdoffset20in80m5gcore3_1=0\0"
	"subband5gver=0x5\0"
	"mcs1024qam2gpo=0\0"
	"mcs1024qam5glpo=0\0"
	"mcs1024qam5gmpo=0\0"
	"mcs1024qam5ghpo=0\0"
	"mcs1024qam5gx1po=0\0"
	"mcs1024qam5gx2po=0\0"
	"mcs8poexp=0\0"
	"mcs9poexp=0\0"
	"mcs10poexp=0\0"
	"mcs11poexp=0\0"
	"wb_rxattn=0x0101\0"
	"wb_tia_gain_mode=0x0404\0"
	"wb_txbuf_offset=0x2222\0"
	"cckbw202gpo=0\0"
	"cckbw20ul2gpo=0\0"
	"mcsbw202gpo=0\0"
	"mcsbw402gpo=0\0"
	"dot11agofdmhrbw202gpo=0\0"
	"ofdmlrbw202gpo=0\0"
	"mcsbw205glpo=0\0"
	"mcsbw405glpo=0\0"
	"mcsbw805glpo=0\0"
	"mcsbw1605glpo=0\0"
	"mcsbw205gmpo=0\0"
	"mcsbw405gmpo=0\0"
	"mcsbw805gmpo=0\0"
	"mcsbw1605gmpo=0\0"
	"mcsbw205ghpo=0\0"
	"mcsbw405ghpo=0\0"
	"mcsbw805ghpo=0\0"
	"mcsbw1605ghpo=0\0"
	"mcsbw205gx1po=0\0"
	"mcsbw405gx1po=0\0"
	"mcsbw805gx1po=0\0"
	"mcsbw1605gx1po=0\0"
	"mcsbw205gx2po=0\0"
	"mcsbw405gx2po=0\0"
	"mcsbw805gx2po=0\0"
	"mcsbw1605gx2po=0\0"
	"mcslr5glpo=0\0"
	"mcslr5gmpo=0\0"
	"mcslr5ghpo=0\0"
	"mcslr5gx1po=0\0"
	"mcslr5gx2po=0\0"
	"sb20in40hrpo=0\0"
	"sb20in40lrpo=0\0"
	"sb20in40hrlrpox=0\0"
	"dot11agduphrpo=0\0"
	"dot11agduplrpo=0\0"
	"dot11agduphrlrpox=0\0"
	"sb40and80hr5glpo=0\0"
	"sb40and80hr5gmpo=0\0"
	"sb40and80hr5ghpo=0\0"
	"sb40and80hr5gx1po=0\0"
	"sb40and80hr5gx2po=0\0"
	"sb20in80and160hr5glpo=0\0"
	"sb20in80and160hr5gmpo=0\0"
	"sb20in80and160hr5ghpo=0\0"
	"sb20in80and160hr5gx1po=0\0"
	"sb20in80and160hr5gx2po=0\0"
	"sb40and80lr5glpo=0\0"
	"sb40and80lr5gmpo=0\0"
	"sb40and80lr5ghpo=0\0"
	"sb40and80lr5gx1po=0\0"
	"sb40and80lr5gx2po=0\0"
	"sb20in80and160lr5glpo=0\0"
	"sb20in80and160lr5gmpo=0\0"
	"sb20in80and160lr5ghpo=0\0"
	"sb20in80and160lr5gx1po=0\0"
	"sb20in80and160lr5gx2po=0\0"
	"pcieingress_war=15\0"
	"sar2g=18\0"
	"sar5g=15\0"
	"noiselvl2ga0=31\0"
	"noiselvl2ga1=31\0"
	"noiselvl2ga2=31\0"
	"noiselvl2ga3=31\0"
	"noiselvl5ga0=31,31,31,31\0"
	"noiselvl5ga1=31,31,31,31\0"
	"noiselvl5ga2=31,31,31,31\0"
	"noiselvl5ga3=31,31,31,31\0"
	"rxgainerr2ga0=63\0"
	"rxgainerr2ga1=31\0"
	"rxgainerr2ga2=31\0"
	"rxgainerr2ga3=31\0"
	"rxgainerr5ga0=63,63,63,63\0"
	"rxgainerr5ga1=31,31,31,31\0"
	"rxgainerr5ga2=31,31,31,31\0"
	"rxgainerr5ga3=31,31,31,31\0"
	"rxgains5gmelnagaina0=7\0"
	"rxgains5gmelnagaina1=7\0"
	"rxgains5gmelnagaina2=7\0"
	"rxgains5gmelnagaina3=7\0"
	"rxgains5gmtrisoa0=15\0"
	"rxgains5gmtrisoa1=15\0"
	"rxgains5gmtrisoa2=15\0"
	"rxgains5gmtrisoa3=15\0"
	"rxgains5gmtrelnabypa0=1\0"
	"rxgains5gmtrelnabypa1=1\0"
	"rxgains5gmtrelnabypa2=1\0"
	"rxgains5gmtrelnabypa3=1\0"
	"rxgains5ghelnagaina0=7\0"
	"rxgains5ghelnagaina1=7\0"
	"rxgains5ghelnagaina2=7\0"
	"rxgains5ghelnagaina3=7\0"
	"rxgains5ghtrisoa0=15\0"
	"rxgains5ghtrisoa1=15\0"
	"rxgains5ghtrisoa2=15\0"
	"rxgains5ghtrisoa3=15\0"
	"rxgains5ghtrelnabypa0=1\0"
	"rxgains5ghtrelnabypa1=1\0"
	"rxgains5ghtrelnabypa2=1\0"
	"rxgains5ghtrelnabypa3=1\0"
	"maxp2ga0=84\0"
	"maxp5gb0a0=72\0"
	"maxp5gb1a0=72\0"
	"maxp5gb2a0=72\0"
	"maxp5gb3a0=72\0"
	"maxp5gb4a0=72\0"
	"maxp2ga1=84\0"
	"maxp5gb0a1=72\0"
	"maxp5gb1a1=72\0"
	"maxp5gb2a1=72\0"
	"maxp5gb3a1=72\0"
	"maxp5gb4a1=72\0"
	"maxp2ga2=84\0"
	"maxp5gb0a2=72\0"
	"maxp5gb1a2=72\0"
	"maxp5gb2a2=72\0"
	"maxp5gb3a2=72\0"
	"maxp5gb4a2=72\0"
	"maxp2ga3=84\0"
	"maxp5gb0a3=72\0"
	"maxp5gb1a3=72\0"
	"maxp5gb2a3=72\0"
	"maxp5gb3a3=72\0"
	"maxp5gb4a3=72\0"
	"rxgains2gelnagaina0=4\0"
	"rxgains2gtrisoa0=7\0"
	"rxgains2gtrelnabypa0=1\0"
	"rxgains5gelnagaina0=2\0"
	"rxgains5gtrisoa0=6\0"
	"rxgains5gtrelnabypa0=1\0"
	"rxgains2gelnagaina1=4\0"
	"rxgains2gtrisoa1=7\0"
	"rxgains2gtrelnabypa1=1\0"
	"rxgains5gelnagaina1=2\0"
	"rxgains5gtrisoa1=6\0"
	"rxgains5gtrelnabypa1=1\0"
	"rxgains2gelnagaina2=4\0"
	"rxgains2gtrisoa2=7\0"
	"rxgains2gtrelnabypa2=1\0"
	"rxgains5gelnagaina2=2\0"
	"rxgains5gtrisoa2=6\0"
	"rxgains5gtrelnabypa2=1\0"
	"rxgains2gelnagaina3=4\0"
	"rxgains2gtrisoa3=7\0"
	"rxgains2gtrelnabypa3=1\0"
	"rxgains5gelnagaina3=2\0"
	"rxgains5gtrisoa3=6\0"
	"rxgains5gtrelnabypa3=1\0"
	"pa5ga0=0x1ab9,0xc3bb,0xfe1a,0x0,0x1af7,0xc324,0xfe74,0x0,0x1b39,0xc23c,"
	"0xff2b,0x0,0x1b72,0xc198,0xff9d,0x0,0x1b8c,0xc2ad,0xff24,0x0\0"
	"pa5g40a0=0x1b7b,0xd218,0xf58a,0x0,0x1b98,0xd17b,0xf5e4,0x0,0x1bfb,0xcfee,"
	"0xf6d2,0x0,0x1c01,0xd08d,0xf6c3,0x0,0x1bf7,0xd26a,0xf5ff,0x0\0"
	"pa5g80a0=0x1ae9,0xcddb,0xf8a0,0x0,0x1afd,0xcdf8,0xf883,0x0,0x1b45,0xccdc,"
	"0xf956,0x0,0x1b44,0xcea2,0xf8c3,0x0,0x1b95,0xcd47,0xf95e,0x0\0"
	"pa5ga1=0x1a80,0xc2d0,0xff08,0x0,0x1aa0,0xc27f,0xff42,0x0,0x1afb,0xc208,"
	"0xffdd,0x0,0x1b24,0xc255,0xffcd,0x0,0x1b5c,0xc214,0x0,0x0\0"
	"pa5g40a1=0x1b31,0xd1ab,0xf630,0x0,0x1b52,0xd095,0xf6ce,0x0,0x1ba3,0xd071,"
	"0xf735,0x0,0x1bd1,0xd066,0xf746,0x0,0x1bf2,0xd0e2,0xf711,0x0\0"
	"pa5g80a1=0x1aa6,0xcd0b,0xf97e,0x0,0x1abb,0xcc7d,0xf9e5,0x0,0x1af5,0xccdb,"
	"0xf9e0,0x0,0x1b32,0xcd75,0xf9b0,0x0,0x1b45,0xcde6,0xf977,0x0\0"
	"pa5ga2=0x1b1e,0xc1ba,0xff54,0x0,0x1b2c,0xc18c,0xff76,0x0,0x1b51,0xc204,"
	"0xff7f,0x0,0x1b79,0xc1ed,0xffa1,0x0,0x1bc2,0xc17b,0xffe0,0x0\0"
	"pa5g40a2=0x1bba,0xd163,0xf602,0x0,0x1be0,0xd020,0xf6ac,0x0,0x1c0f,0xd04b,"
	"0xf6da,0x0,0x1c25,0xd07b,0xf6db,0x0,0x1c28,0xd232,0xf623,0x0\0"
	"pa5g80a2=0x1b16,0xcd7d,0xf8f5,0x0,0x1b07,0xce18,0xf8c8,0x0,0x1b50,0xcd30,"
	"0xf963,0x0,0x1b72,0xce09,0xf931,0x0,0x1ba0,0xcd76,0xf969,0x0\0"
	"pa5ga3=0x1b1e,0xc1ba,0xff54,0x0,0x1b2c,0xc18c,0xff76,0x0,0x1b51,0xc204,"
	"0xff7f,0x0,0x1b79,0xc1ed,0xffa1,0x0,0x1bc2,0xc17b,0xffe0,0x0\0"
	"pa5g40a3=0x1bba,0xd163,0xf602,0x0,0x1be0,0xd020,0xf6ac,0x0,0x1c0f,0xd04b,"
	"0xf6da,0x0,0x1c25,0xd07b,0xf6db,0x0,0x1c28,0xd232,0xf623,0x0\0"
	"pa5g80a3=0x1b16,0xcd7d,0xf8f5,0x0,0x1b07,0xce18,0xf8c8,0x0,0x1b50,0xcd30,"
	"0xf963,0x0,0x1b72,0xce09,0xf931,0x0,0x1ba0,0xcd76,0xf969,0x0\0"
	"pa2ga0=0x1c9b,0xbef2,0x0,0xba7\0"
	"pa2g40a0=0x1c9b,0xbef2,0x0,0xba7\0"
	"pa2ga1=0x1c3f,0xbec6,0x0,0x0\0"
	"pa2g40a1=0xffff,0xffff,0xffff,0xffff\0"
	"pa2ga2=0x1d05,0xbe32,0x0,0xaf2\0"
	"pa2g40a2=0xffff,0xffff,0xffff,0xffff\0"
	"pa2ga3=0x1d05,0xbe32,0x0,0xaf2\0"
	"pa2g40a3=0xffff,0xffff,0xffff,0xffff\0"
	"rpcal2g=0\0"
	"rpcal5gb0=0\0"
	"rpcal5gb1=0\0"
	"rpcal5gb2=0\0"
	"rpcal5gb3=0\0"
	"rpcal5gb4=0\0"
	"rpcal2gcore3=0\0"
	"rpcal5gb0core3=0\0"
	"rpcal5gb1core3=0\0"
	"rpcal5gb2core3=0\0"
	"rpcal5gb3core3=0\0"
	"swctrlmap4_cfg=0x1\0"
	"swctrlmap4_TX2g_fem3to0=0x36bd\0"
	"swctrlmap4_RX2g_fem3to0=0x0080\0"
	"swctrlmap4_RXByp2g_fem3to0=0x24a4\0"
	"swctrlmap4_misc2g_fem3to0=0x24a4\0"
	"swctrlmap4_TX5g_fem3to0=0x7ebd\0"
	"swctrlmap4_RX5g_fem3to0=0x4880\0"
	"swctrlmap4_RXByp5g_fem3to0=0x6ca4\0"
	"swctrlmap4_misc5g_fem3to0=0x6ca4\0"
	"swctrlmap4_TX2g_fem7to4=0\0"
	"swctrlmap4_RX2g_fem7to4=0\0"
	"swctrlmap4_RXByp2g_fem7to4=0\0"
	"swctrlmap4_misc2g_fem7to4=0\0"
	"swctrlmap4_TX5g_fem7to4=0\0"
	"swctrlmap4_RX5g_fem7to4=0\0"
	"swctrlmap4_RXByp5g_fem7to4=0\0"
	"swctrlmap4_misc5g_fem7to4=0\0"
	"hwrssioffset_cmn_2g_0=0\0"
	"hwrssioffset_trt_2g_0=0\0"
	"hwrssioffset_cmn_2g_1=0\0"
	"hwrssioffset_trt_2g_1=0\0"
	"hwrssioffset_cmn_2g_2=0\0"
	"hwrssioffset_trt_2g_2=0\0"
	"hwrssioffset_cmn_2g_3=0\0"
	"hwrssioffset_trt_2g_3=0\0"
	"hwrssioffset_cmn_5g_6g_0=0,0,0,0,0,0,0\0"
	"hwrssioffset_cmn_5g_6g_1=0,0,0,0,0,0,0\0"
	"hwrssioffset_cmn_5g_6g_2=0,0,0,0,0,0,0\0"
	"hwrssioffset_cmn_5g_6g_3=0,0,0,0,0,0,0\0"
	"hwrssioffset_trt_5g_6g_0=0,0,0,0,0,0,0\0"
	"hwrssioffset_trt_5g_6g_1=0,0,0,0,0,0,0\0"
	"hwrssioffset_trt_5g_6g_2=0,0,0,0,0,0,0\0"
	"hwrssioffset_trt_5g_6g_3=0,0,0,0,0,0,0\0"
	"END\0";

static char BCMATTACHDATA(defaultsromvars_6717)[] =
	"sromrev=18\0"
	"boardrev=0x1101\0"
	"boardflags=0x10001000\0"
	"boardflags2=0x0\0"
	"boardtype=0xa36\0"
	"devid=0x602a\0"
	"subvid=0x14e4\0"
	"boardflags3=0x0\0"
	"boardnum=9999\0"
	"macaddr=00:90:4c:3d:83:18\0"
	"ccode=0x0\0"
	"regrev=0\0"
	"aa2g=15\0"
	"aa5g=15\0"
	"agbg0=0\0"
	"agbg1=0\0"
	"agbg2=0\0"
	"aga0=0\0"
	"aga1=0\0"
	"aga2=0\0"
	"txchain=15\0"
	"rxchain=15\0"
	"antswitch=0\0"
	"tssiposslope2g=1\0"
	"epagain2g=0\0"
	"pdgain2g=0\0"
	"tworangetssi2g=0\0"
	"papdcap2g=0\0"
	"femctrl=2\0"
	"tssiposslope5g=1\0"
	"epagain5g=0\0"
	"pdgain5g=0\0"
	"tworangetssi5g=0\0"
	"papdcap5g=0\0"
	"gainctrlsph=0\0"
	"tempthresh=110\0"
	"tempoffset=255\0"
	"rawtempsense=0x20\0"
	"measpower=0x0\0"
	"tempsense_slope=0xff\0"
	"tempcorrx=0x3f\0"
	"tempsense_option=0x3\0"
	"xtalfreq=80000\0"
	"pa5gbw4080a1=0x0,0x739c,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0\0"
	"phycal_tempdelta=40\0"
	"temps_period=10\0"
	"temps_hysteresis=5\0"
	"measpower1=0x0\0"
	"measpower2=0x0\0"
	"tssifloor2g=0x1c\0"
	"tssifloor5g=0x0,0x300,0x35a,0x35a\0"
	"pdoffset2g40ma0=0\0"
	"pdoffset2g40ma1=0\0"
	"pdoffset2g40ma2=0\0"
	"pdoffset2g40mvalid=0\0"
	"pdoffset40ma0=0\0"
	"pdoffset40ma1=0\0"
	"pdoffset40ma2=0\0"
	"pdoffset80ma0=65535\0"
	"pdoffset80ma1=0\0"
	"pdoffset80ma2=0\0"
	"subband5gver=0x5\0"
	"paparambwver=0\0"
	"rx5ggainwar=0\0"
	"pa5gbw4080a0=0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x14a5\0"
	"pa5gbw40a0=0x0,0x0,0x0,0x2020,0x3820,0x2020,0x3820,0x0,0x0,0x0,0x0,0x0\0"
	"pa5gbw80a0=0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x14a5\0"
	"cckbw202gpo=8738\0"
	"cckbw20ul2gpo=8738\0"
	"mcsbw202gpo=3433731650\0"
	"mcsbw402gpo=3433731650\0"
	"dot11agofdmhrbw202gpo=43686\0"
	"ofdmlrbw202gpo=8770\0"
	"mcsbw205glpo=2558944322\0"
	"mcsbw405glpo=2575721538\0"
	"mcsbw805glpo=2844156994\0"
	"mcsbw205gmpo=2558944322\0"
	"mcsbw405gmpo=2558944322\0"
	"mcsbw805gmpo=2844156994\0"
	"mcsbw205ghpo=2288411714\0"
	"mcsbw405ghpo=2288411714\0"
	"mcsbw805ghpo=2826331202\0"
	"mcslr5glpo=0\0"
	"mcslr5gmpo=0\0"
	"mcslr5ghpo=0\0"
	"sb20in40hrpo=0\0"
	"sb20in80and160hr5glpo=0\0"
	"sb40and80hr5glpo=0\0"
	"sb20in80and160hr5gmpo=0\0"
	"sb40and80hr5gmpo=0\0"
	"sb20in80and160hr5ghpo=0\0"
	"sb40and80hr5ghpo=0\0"
	"sb20in40lrpo=0\0"
	"sb20in80and160lr5glpo=0\0"
	"sb40and80lr5glpo=0\0"
	"sb20in80and160lr5gmpo=0\0"
	"sb40and80lr5gmpo=0\0"
	"sb20in80and160lr5ghpo=0\0"
	"sb40and80lr5ghpo=0\0"
	"dot11agduphrpo=14\0"
	"dot11agduplrpo=0\0"
	"sar2g=18\0"
	"sar5g=15\0"
	"noiselvl2ga0=31\0"
	"noiselvl2ga1=31\0"
	"noiselvl2ga2=31\0"
	"noiselvl5ga0=31,31,31,31\0"
	"noiselvl5ga1=31,31,31,31\0"
	"noiselvl5ga2=31,31,31,31\0"
	"eu_edthresh2g=255\0"
	"eu_edthresh5g=255\0"
	"rxgainerr2ga0=8\0"
	"rxgainerr2ga1=0\0"
	"rxgainerr2ga2=4\0"
	"rxgainerr5ga0=4,8,6,10\0"
	"rxgainerr5ga1=0,30,30,26\0"
	"rxgainerr5ga2=2,0,2,30\0"
	"rpcal2g=0\0"
	"rpcal5gb0=0\0"
	"rpcal5gb1=0\0"
	"rpcal5gb2=0\0"
	"rpcal5gb3=0\0"
	"txidxcap2g=0\0"
	"txidxcap5g=0\0"
	"pdoffsetcckma0=0\0"
	"pdoffsetcckma1=0\0"
	"pdoffsetcckma2=0\0"
	"boardflags4=0x12230008\0"
	"pdoffsetcck=165\0"
	"pdoffset20in40m5gb0=27482\0"
	"pdoffset20in40m5gb1=27482\0"
	"pdoffset20in40m5gb2=27482\0"
	"pdoffset20in40m5gb3=27482\0"
	"pdoffset20in40m5gb4=27482\0"
	"pdoffset40in80m5gb0=0\0"
	"pdoffset40in80m5gb1=0\0"
	"pdoffset40in80m5gb2=0\0"
	"pdoffset40in80m5gb3=0\0"
	"pdoffset40in80m5gb4=0\0"
	"pdoffset20in80m5gb0=27482\0"
	"pdoffset20in80m5gb1=27482\0"
	"pdoffset20in80m5gb2=27482\0"
	"pdoffset20in80m5gb3=27482\0"
	"pdoffset20in80m5gb4=29530\0"
	"mcsbw205gx1po=2288411714\0"
	"mcsbw405gx1po=2288411714\0"
	"mcsbw805gx1po=2825282626\0"
	"mcsbw205gx2po=2825282626\0"
	"mcsbw405gx2po=2825282626\0"
	"mcsbw805gx2po=2825282626\0"
	"sb20in80and160hr5gx1po=0\0"
	"sb20in80and160lr5gx1po=0\0"
	"sb40and80hr5gx1po=0\0"
	"sb20in80and160hr5gx2po=0\0"
	"sb20in80and160lr5gx2po=0\0"
	"sb40and80hr5gx2po=0\0"
	"rxgains5gmelnagaina0=0\0"
	"rxgains5gmelnagaina1=0\0"
	"rxgains5gmelnagaina2=0\0"
	"rxgains5gmtrisoa0=0\0"
	"rxgains5gmtrisoa1=0\0"
	"rxgains5gmtrisoa2=0\0"
	"rxgains5gmtrelnabypa0=0\0"
	"rxgains5gmtrelnabypa1=0\0"
	"rxgains5gmtrelnabypa2=0\0"
	"rxgains5ghelnagaina0=7\0"
	"rxgains5ghelnagaina1=7\0"
	"rxgains5ghelnagaina2=7\0"
	"rxgains5ghtrisoa0=15\0"
	"rxgains5ghtrisoa1=15\0"
	"rxgains5ghtrisoa2=15\0"
	"rxgains5ghtrelnabypa0=1\0"
	"rxgains5ghtrelnabypa1=1\0"
	"rxgains5ghtrelnabypa2=1\0"
	"gpdn=0x0\0"
	"rpcal2gcore3=0\0"
	"rpcal5gb0core3=0\0"
	"rpcal5gb1core3=0\0"
	"rpcal5gb2core3=0\0"
	"rpcal5gb3core3=0\0"
	"sw_txchain_mask=15\0"
	"sw_rxchain_mask=15\0"
	"agbg3=0\0"
	"aga3=0\0"
	"noiselvl2ga3=31\0"
	"noiselvl5ga3=31,31,31,31\0"
	"rxgainerr2ga3=28\0"
	"rxgainerr5ga3=28,26,0,26\0"
	"rxgains5gmelnagaina3=0\0"
	"rxgains5gmtrisoa3=0\0"
	"rxgains5gmtrelnabypa3=0\0"
	"rxgains5ghelnagaina3=7\0"
	"rxgains5ghtrisoa3=15\0"
	"rxgains5ghtrelnabypa3=1\0"
	"wb_rxattn=0x0\0"
	"wb_tia_gain_mode=0x0\0"
	"wb_txbuf_offset=0x0\0"
	"peak_psd_limit_6G2G=0x0\0"
	"peak_psd_limit_5G0=0x0\0"
	"peak_psd_limit_5G1=0x0\0"
	"peak_psd_limit_5G2=0x0\0"
	"pdoffset20in40m5gb5=65535\0"
	"pdoffset20in40m5gb6=65535\0"
	"pdoffset20in80m5gb5=65535\0"
	"pdoffset20in80m5gb6=65535\0"
	"pdoffset40in80m5gb5=65535\0"
	"pdoffset40in80m5gb6=65535\0"
	"pdoffset20in40m5gcore3_2=65535\0"
	"pdoffset20in80m5gcore3_2=65535\0"
	"pdoffset40in80m5gcore3_2=65535\0"
	"pdoffset20in160m5gc4=65535\0"
	"pdoffset20in160m5gc5=65535\0"
	"pdoffset20in160m5gc6=65535\0"
	"pdoffset20in160m5gcore3_2=65535\0"
	"pdoffset40in160m5gc4=65535\0"
	"pdoffset40in160m5gc5=65535\0"
	"pdoffset40in160m5gc6=65535\0"
	"pdoffset40in160m5gcore3_2=65535\0"
	"pdoffset80in160m5gc4=65535\0"
	"pdoffset80in160m5gc5=65535\0"
	"pdoffset80in160m5gc6=65535\0"
	"pdoffset80in160m5gcore3_2=65535\0"
	"maxp5gb5a0=255\0"
	"maxp5gb6a0=255\0"
	"maxp5gb5a1=255\0"
	"maxp5gb6a1=255\0"
	"maxp5gb5a2=255\0"
	"maxp5gb6a2=255\0"
	"maxp5gb5a3=255\0"
	"maxp5gb6a3=255\0"
	"rxgains6gband0elnagaina0=7\0"
	"rxgains6gband0elnagaina1=7\0"
	"rxgains6gband0elnagaina2=7\0"
	"rxgains6gband0elnagaina3=7\0"
	"rxgains6gband0trisoa0=15\0"
	"rxgains6gband0trisoa1=15\0"
	"rxgains6gband0trisoa2=15\0"
	"rxgains6gband0trisoa3=15\0"
	"rxgains6gband0trelnabypa0=1\0"
	"rxgains6gband0trelnabypa1=1\0"
	"rxgains6gband0trelnabypa2=1\0"
	"rxgains6gband0trelnabypa3=1\0"
	"rxgains6gband1elnagaina0=7\0"
	"rxgains6gband1elnagaina1=7\0"
	"rxgains6gband1elnagaina2=7\0"
	"rxgains6gband1elnagaina3=7\0"
	"rxgains6gband1trisoa0=15\0"
	"rxgains6gband1trisoa1=15\0"
	"rxgains6gband1trisoa2=15\0"
	"rxgains6gband1trisoa3=15\0"
	"rxgains6gband1trelnabypa0=1\0"
	"rxgains6gband1trelnabypa1=1\0"
	"rxgains6gband1trelnabypa2=1\0"
	"rxgains6gband1trelnabypa3=1\0"
	"rxgains6gband2elnagaina0=7\0"
	"rxgains6gband2elnagaina1=7\0"
	"rxgains6gband2elnagaina2=7\0"
	"rxgains6gband2elnagaina3=7\0"
	"rxgains6gband2trisoa0=15\0"
	"rxgains6gband2trisoa1=15\0"
	"rxgains6gband2trisoa2=15\0"
	"rxgains6gband2trisoa3=15\0"
	"rxgains6gband2trelnabypa0=1\0"
	"rxgains6gband2trelnabypa1=1\0"
	"rxgains6gband2trelnabypa2=1\0"
	"rxgains6gband2trelnabypa3=1\0"
	"rxgains6gband3elnagaina0=7\0"
	"rxgains6gband3elnagaina1=7\0"
	"rxgains6gband3elnagaina2=7\0"
	"rxgains6gband3elnagaina3=7\0"
	"rxgains6gband3trisoa0=15\0"
	"rxgains6gband3trisoa1=15\0"
	"rxgains6gband3trisoa2=15\0"
	"rxgains6gband3trisoa3=15\0"
	"rxgains6gband3trelnabypa0=1\0"
	"rxgains6gband3trelnabypa1=1\0"
	"rxgains6gband3trelnabypa2=1\0"
	"rxgains6gband3trelnabypa3=1\0"
	"rxgains6gband4elnagaina0=7\0"
	"rxgains6gband4elnagaina1=7\0"
	"rxgains6gband4elnagaina2=7\0"
	"rxgains6gband4elnagaina3=7\0"
	"rxgains6gband4trisoa0=15\0"
	"rxgains6gband4trisoa1=15\0"
	"rxgains6gband4trisoa2=15\0"
	"rxgains6gband4trisoa3=15\0"
	"rxgains6gband4trelnabypa0=1\0"
	"rxgains6gband4trelnabypa1=1\0"
	"rxgains6gband4trelnabypa2=1\0"
	"rxgains6gband4trelnabypa3=1\0"
	"rxgains6gband5elnagaina0=7\0"
	"rxgains6gband5elnagaina1=7\0"
	"rxgains6gband5elnagaina2=7\0"
	"rxgains6gband5elnagaina3=7\0"
	"rxgains6gband5trisoa0=15\0"
	"rxgains6gband5trisoa1=15\0"
	"rxgains6gband5trisoa2=15\0"
	"rxgains6gband5trisoa3=15\0"
	"rxgains6gband5trelnabypa0=1\0"
	"rxgains6gband5trelnabypa1=1\0"
	"rxgains6gband5trelnabypa2=1\0"
	"rxgains6gband5trelnabypa3=1\0"
	"rxgains6gband6elnagaina0=7\0"
	"rxgains6gband6elnagaina1=7\0"
	"rxgains6gband6elnagaina2=7\0"
	"rxgains6gband6elnagaina3=7\0"
	"rxgains6gband6trisoa0=15\0"
	"rxgains6gband6trisoa1=15\0"
	"rxgains6gband6trisoa2=15\0"
	"rxgains6gband6trisoa3=15\0"
	"rxgains6gband6trelnabypa0=1\0"
	"rxgains6gband6trelnabypa1=1\0"
	"rxgains6gband6trelnabypa2=1\0"
	"rxgains6gband6trelnabypa3=1\0"
	"rxgainerr6ga0=63,63,63,63,63,63,63\0"
	"rxgainerr6ga1=31,31,31,31,31,31,31\0"
	"rxgainerr6ga2=31,31,31,31,31,31,31\0"
	"rxgainerr6ga3=31,31,31,31,31,31,31\0"
	"pdoffset20in40m5gcore3=27482\0"
	"pdoffset20in40m5gcore3_1=858\0"
	"pdoffset20in80m5gcore3=27485\0"
	"pdoffset20in80m5gcore3_1=922\0"
	"pdoffset40in80m5gcore3=0\0"
	"pdoffset40in80m5gcore3_1=0\0"
	"pdoffset20in40m2g=29596\0"
	"pdoffset20in40m2gcore3=28\0"
	"pdoffsetcck20m=5285\0"
	"pdoffsetcckch14=0\0"
	"mcs1024qam2gpo=65278\0"
	"mcs1024qam5glpo=4006399180\0"
	"mcs1024qam5gmpo=4006399180\0"
	"mcs1024qam5ghpo=13421772\0"
	"mcs1024qam5gx1po=13421772\0"
	"mcs1024qam5gx2po=0\0"
	"mcsbw1605glpo=3397936742\0"
	"mcsbw1605gmpo=3397936196\0"
	"mcsbw1605ghpo=3397936196\0"
	"mcsbw1605gx1po=3397936196\0"
	"mcsbw1605gx2po=3433718852\0"
	"mcs8poexp=0\0"
	"mcs9poexp=0\0"
	"mcs10poexp=941629472\0"
	"mcs11poexp=941629472\0"
	"cckbw202gpodelta=0\0"
	"sb20in40hrlrpox=0\0"
	"mcslr5gx1po=0\0"
	"mcslr5gx2po=0\0"
	"mcslr5gx3po=65535\0"
	"mcslr5gx4po=65535\0"
	"mcsbw205gx3po=4294967295\0"
	"mcsbw405gx3po=4294967295\0"
	"mcsbw805gx3po=4294967295\0"
	"mcsbw205gx4po=4294967295\0"
	"mcsbw405gx4po=4294967295\0"
	"mcsbw805gx4po=4294967295\0"
	"sb20in80and160hr5gx3po=65535\0"
	"sb40and80hr5gx3po=65535\0"
	"sb20in80and160lr5gx3po=65535\0"
	"sb20in80and160hr5gx4po=65535\0"
	"sb40and80hr5gx4po=65535\0"
	"sb20in80and160lr5gx4po=65535\0"
	"mcsbw1605gx3po=0\0"
	"mcsbw1605gx4po=0\0"
	"mcs1024qam5gx3po=4294967295\0"
	"mcs1024qam5gx4po=4294967295\0"
	"sb40and80lr5gx1po=0\0"
	"sb40and80lr5gx2po=0\0"
	"sb40and80lr5gx3po=65535\0"
	"sb40and80lr5gx4po=65535\0"
	"swctrlmap4_cfg=1793\0"
	"swctrlmap4_TX2g_fem3to0=26214\0"
	"swctrlmap4_RX2g_fem3to0=4369\0"
	"swctrlmap4_RXmid2g_fem3to0=17476\0"
	"swctrlmap4_RXByp2g_fem3to0=13107\0"
	"swctrlmap4_misc2g_fem3to0=0\0"
	"swctrlmap4_TX5g_fem3to0=26214\0"
	"swctrlmap4_RX5g_fem3to0=4369\0"
	"swctrlmap4_RXmid5g_fem3to0=17476\0"
	"swctrlmap4_RXByp5g_fem3to0=13107\0"
	"swctrlmap4_misc5g_fem3to0=0\0"
	"swctrlmap4_TX2g_fem7to4=0\0"
	"swctrlmap4_RX2g_fem7to4=0\0"
	"swctrlmap4_RXByp2g_fem7to4=0\0"
	"swctrlmap4_misc2g_fem7to4=0\0"
	"swctrlmap4_TX5g_fem7to4=0\0"
	"swctrlmap4_RX5g_fem7to4=0\0"
	"swctrlmap4_RXByp5g_fem7to4=0\0"
	"swctrlmap4_misc5g_fem7to4=0\0"
	"rxmid2g_elnagain=13107\0"
	"rxmid5g_elnagain=13107\0"
	"papd_swctrl=65535\0"
	"papd_swctrl_5g6g=65535\0"
	"papd_swctrlmap_fem2g=65535\0"
	"papd_swctrlmap_fem5g=65535\0"
	"papd_swctrlmap_fem6g=65535\0"
	"papd_swctrlmap_femlp2g=65535\0"
	"papd_swctrlmap_femlp5g=65535\0"
	"papd_swctrlmap_femlp6g=65535\0"
	"pdoffset20in160m5gc0=31710\0"
	"pdoffset20in160m5gc1=31710\0"
	"pdoffset20in160m5gc2=29628\0"
	"pdoffset20in160m5gc3=29628\0"
	"pdoffset20in160m5gcore3=924\0"
	"pdoffset20in160m5gcore3_1=924\0"
	"pdoffset40in160m5gc0=4294\0"
	"pdoffset40in160m5gc1=4294\0"
	"pdoffset40in160m5gc2=2178\0"
	"pdoffset40in160m5gc3=2178\0"
	"pdoffset40in160m5gcore3=98\0"
	"pdoffset40in160m5gcore3_1=0\0"
	"pdoffset80in160m5gc0=30750\0"
	"pdoffset80in160m5gc1=30750\0"
	"pdoffset80in160m5gc2=30750\0"
	"pdoffset80in160m5gc3=30750\0"
	"pdoffset80in160m5gcore3=924\0"
	"pdoffset80in160m5gcore3_1=924\0"
	"low_adc_rate_en=1\0"
	"ppr_backoff_2g=3\0"
	"ppr_backoff_5g=3\0"
	"dssf_dis_ch138=1\0"
	"olpc_2g_th=10\0"
	"olpc_5g_th=10\0"
	"coex_gpioctrl_0=65535\0"
	"coex_gpioctrl_1=65535\0"
	"coex_gpioctrl_2=65535\0"
	"coex_gpioctrl_3=65535\0"
	"ru106ppr_2g_0=38741\0"
	"ru106ppr_2g_1=15788\0"
	"ru106ppr_2g_2=17968\0"
	"ru242ppr_2g_0=38485\0"
	"ru242ppr_2g_1=15787\0"
	"ru242ppr_2g_2=16912\0"
	"ru106ppr_5g_0=38741\0"
	"ru106ppr_5g_1=15788\0"
	"ru106ppr_5g_2=17968\0"
	"ru242ppr_5g_0=38485\0"
	"ru242ppr_5g_1=15787\0"
	"ru242ppr_5g_2=16912\0"
	"ru484ppr_5g_0=38741\0"
	"ru484ppr_5g_1=15788\0"
	"ru484ppr_5g_2=17968\0"
	"ru996ppr_5g_0=38485\0"
	"ru996ppr_5g_1=15787\0"
	"ru996ppr_5g_2=16912\0"
	"txs_shaper_en_2g=1\0"
	"txs_shaper_en_5g=1\0"
	"txs_shaper_en_6g=1\0"
	"txs_shaper_bypass=0\0"
	"txs_chan_rate_en=103\0"
	"txs_shaper_dis_2g_nonbndg=1\0"
	"txs_shaper_dis_5g_nonbndg=1\0"
	"txs_shaper_en_unii3=1\0"
	"txs_shaper_en_unii4=1\0"
	"subband_ed_adj=65535\0"
	"avsflags=0xffff\0"
	"hwrssioffset_cmn_2g_0=15\0"
	"hwrssioffset_trt_2g_0=15\0"
	"hwrssioffset_cmn_2g_1=15\0"
	"hwrssioffset_trt_2g_1=15\0"
	"hwrssioffset_cmn_2g_2=15\0"
	"hwrssioffset_trt_2g_2=15\0"
	"hwrssioffset_cmn_2g_3=15\0"
	"hwrssioffset_trt_2g_3=15\0"
	"hwrssioffset_cmn_5g_6g_0=15,15,15,15,15,15,15\0"
	"hwrssioffset_cmn_5g_6g_1=15,15,15,15,15,15,15\0"
	"hwrssioffset_cmn_5g_6g_2=15,15,15,15,15,15,15\0"
	"hwrssioffset_cmn_5g_6g_3=15,15,15,15,15,15,15\0"
	"hwrssioffset_trt_5g_6g_0=15,15,15,15,15,15,15\0"
	"hwrssioffset_trt_5g_6g_1=15,15,15,15,15,15,15\0"
	"hwrssioffset_trt_5g_6g_2=15,15,15,15,15,15,15\0"
	"hwrssioffset_trt_5g_6g_3=15,15,15,15,15,15,15\0"
	"maxp2ga0=108\0"
	"maxp5gb4a0=100\0"
	"pa2ga0=0x1d2c,0xeaad,0xf1bd,0xc21\0"
	"pa2g40a0=0x1c68,0xfca6,0xe855,0x0\0"
	"maxp5gb0a0=100\0"
	"maxp5gb1a0=100\0"
	"maxp5gb2a0=100\0"
	"maxp5gb3a0=100\0"
	"pa5ga0=0x22ab,0xbf39,0x0,0x0,0x22fd,0xbef1,0x0,0x0,0x227d,0xc18e,0x0,0x0,0x223f,0xc1b2,"
	"0xffff,0x6b,0x21cd,0xc260,0x0,0x0\0"
	"pa5g40a0=0x22e6,0xc820,0xfb1c,0x844,0x2346,0xc5aa,0xfbd2,0x0,0x227c,0xcc82,0xf9c4,0x0,"
	"0x225d,0xcba8,0xfa6e,0x0,0x2207,0xc966,0xfbb7,0x0\0"
	"pa5g80a0=0x2344,0xbf25,0x0,0xac6,0x23c0,0xbdc7,0x0,0x0,0x234e,0xc0e6,0xffff,0x9b0,0x2310,"
	"0xc16a,0x0,0x835,0x22b3,0xc13a,0x0,0x0\0"
	"rxgains2gelnagaina0=4\0"
	"rxgains2gtrisoa0=7\0"
	"rxgains2gtrelnabypa0=1\0"
	"rxgains5gelnagaina0=4\0"
	"rxgains5gtrisoa0=7\0"
	"rxgains5gtrelnabypa0=1\0"
	"pa2g20ccka0=0x1dc8,0xfb13,0xe8fe,0x0\0"
	"pa5g160a0=0x234a,0xc071,0x0,0x1935,0x22db,0xcce7,0xfb55,0x1fab,0x2348,0xc360,0x0,0x210a,"
	"0x227f,0xc360,0x0,0x210a\0"
	"pa5gexta0=0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff\0"
	"pa5gext40a0=0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff\0"
	"pa5gext80a0=0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff\0"
	"pa5gext160a0=0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,"
	"0xffff,0xffff\0"
	"maxp2ga1=108\0"
	"maxp5gb4a1=100\0"
	"pa2ga1=0x1d0a,0xec65,0xf0ac,0xcf6\0"
	"pa2g40a1=0x1d5f,0xf2ec,0xeceb,0xe04\0"
	"maxp5gb0a1=100\0"
	"maxp5gb1a1=100\0"
	"maxp5gb2a1=100\0"
	"maxp5gb3a1=100\0"
	"pa5ga1=0x223b,0xbf49,0x0,0x0,0x227c,0xbf2f,0x0,0x0,0x2291,0xbf6b,0x0,0x0,0x2280,0xc03f,"
	"0x0,0x0,0x2210,0xc099,0x0,0x0\0"
	"pa5g40a1=0x22f9,0xc456,0xfd0a,0xee3,0x23ad,0xc44e,0xfbfe,0x0,0x22d0,0xca54,0xfa2e,0x0,"
	"0x22d3,0xc810,0xfb61,0x0,0x229d,0xc6d6,0xfc41,0x0\0"
	"pa5g80a1=0x23ae,0xbd6c,0x0,0x0,0x23e3,0xbd94,0x0,0x0,0x23c9,0xbe39,0x0,0x0,0x2323,0xbf97,"
	"0x0,0x8bd,0x2363,0xbf4e,0x0,0x0\0"
	"rxgains2gelnagaina1=4\0"
	"rxgains2gtrisoa1=7\0"
	"rxgains2gtrelnabypa1=1\0"
	"rxgains5gelnagaina1=4\0"
	"rxgains5gtrisoa1=7\0"
	"rxgains5gtrelnabypa1=1\0"
	"pa2g20ccka1=0x1da9,0xfe92,0xe6f5,0x0\0"
	"pa5g160a1=0x2399,0xbf1c,0x0,0xdd0,0x2371,0xc76f,0xfde0,0x242e,0x2388,0xc2d2,0xfe84,0xc30,"
	"0x22f1,0xc2d2,0xfe84,0xc30\0"
	"pa5gexta1=0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff\0"
	"pa5gext40a1=0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff\0"
	"pa5gext80a1=0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff\0"
	"pa5gext160a1=0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,"
	"0xffff,0xffff\0"
	"maxp2ga2=108\0"
	"maxp5gb4a2=100\0"
	"pa2ga2=0x1dad,0xead1,0xf17b,0x857\0"
	"pa2g40a2=0x1cb7,0xfd7c,0xe80c,0x0\0"
	"maxp5gb0a2=100\0"
	"maxp5gb1a2=100\0"
	"maxp5gb2a2=100\0"
	"maxp5gb3a2=100\0"
	"pa5ga2=0x229b,0xbefd,0x0,0x0,0x22db,0xbe81,0x0,0x0,0x22b5,0xbefc,0x0,0x0,0x225d,0xbf9d,"
	"0x0,0x0,0x21da,0xc0cc,0x0,0x0\0"
	"pa5g40a2=0x239b,0xc268,0xfdc2,0x1188,0x23b5,0xc2db,0xfd50,0xa39,0x231d,0xc663,0xfba9,0x0,"
	"0x23ad,0xc180,0xfe86,0x953,0x239b,0xc026,0xffd5,0x12b3\0"
	"pa5g80a2=0x2418,0xbcea,0x0,0x0,0x2412,0xbcd4,0x0,0x0,0x23f2,0xbd94,0x0,0x0,0x2380,0xbe25,"
	"0x0,0x0,0x2332,0xbec5,0x0,0x0\0"
	"rxgains2gelnagaina2=4\0"
	"rxgains2gtrisoa2=7\0"
	"rxgains2gtrelnabypa2=1\0"
	"rxgains5gelnagaina2=4\0"
	"rxgains5gtrisoa2=7\0"
	"rxgains5gtrelnabypa2=1\0"
	"pa2g20ccka2=0x1dc3,0xf7f7,0xea7e,0x0\0"
	"pa5g160a2=0x23ef,0xbed7,0x0,0x11f6,0x220a,0xcbf5,0xfcc9,0x246c,0x237f,0xc036,0xfffe,"
	"0x1770,0x231c,0xc036,0xfffe,0x1770\0"
	"pa5gexta2=0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff\0"
	"pa5gext40a2=0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff\0"
	"pa5gext80a2=0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff\0"
	"pa5gext160a2=0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,"
	"0xffff,0xffff\0"
	"maxp2ga3=108\0"
	"maxp5gb4a3=100\0"
	"pa2ga3=0x1e6b,0xe643,0xf401,0xeae\0"
	"pa2g40a3=0x1e99,0xf084,0xee82,0xb88\0"
	"maxp5gb0a3=100\0"
	"maxp5gb1a3=100\0"
	"maxp5gb2a3=100\0"
	"maxp5gb3a3=100\0"
	"pa5ga3=0x2322,0xbf46,0x0,0x0,0x2364,0xbeef,0x0,0x0,0x2369,0xbf5d,0x0,0x0,0x229b,0xc0d7,"
	"0xfffe,0x0,0x2230,0xc1d0,0x0,0x0\0"
	"pa5g40a3=0x22b0,0xcbc1,0xf92b,0x4a7,0x241e,0xc707,0xfaed,0x0,0x23ee,0xc64d,0xfbc4,0x0,"
	"0x240b,0xc311,0xfded,0xa17,0x23f6,0xbfad,0xffff,0xb91\0"
	"pa5g80a3=0x23f4,0xbeee,0x0,0xdbf,0x248f,0xbcec,0x0,0x0,0x2493,0xbdb6,0x0,0x0,0x242e,"
	"0xbe61,0x0,0x0,0x23c4,0xbf0c,0x0,0x0\0"
	"rxgains2gelnagaina3=4\0"
	"rxgains2gtrisoa3=7\0"
	"rxgains2gtrelnabypa3=1\0"
	"rxgains5gelnagaina3=4\0"
	"rxgains5gtrisoa3=7\0"
	"rxgains5gtrelnabypa3=1\0"
	"pa2g20ccka3=0x1e1f,0xf493,0xebac,0x0\0"
	"pa5g160a3=0x2413,0xc0c4,0x0,0x1f51,0x223e,0xcb79,0xfc72,0x2340,0x2476,0xbfc2,0xff71,"
	"0x3f9,0x23d0,0xbfc2,0xff71,0x3f9\0"
	"pa5gexta3=0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff\0"
	"pa5gext40a3=0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff\0"
	"pa5gext80a3=0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff\0"
	"pa5gext160a3=0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,"
	"0xffff,0xffff\0"
	"END\0";

static char BCMATTACHDATA(defaultsromvars_6726)[] =
	"sromrev=18\0"
	"boardrev=0x1101\0"
	"boardflags=0x10001000\0"
	"boardflags2=0x0\0"
	"boardtype=0xa36\0"
	"devid=0x6031\0"
#if defined(BCMQT) && defined(BCMQT_PHYLESS)
	"phy_qt_radioid=0x50e7\0"
	"phy_qt_phyrev=0x84\0"
#endif /* BCMQT && BCMQT_PHYLESS */
	"subvid=0x14e4\0"
	"boardflags3=0x0\0"
	"boardnum=9999\0"
	"macaddr=00:90:4c:3d:83:18\0"
	"ccode=0x0\0"
	"regrev=0\0"
	"aa2g=15\0"
	"aa5g=15\0"
	"agbg0=0\0"
	"agbg1=0\0"
	"agbg2=0\0"
	"aga0=0\0"
	"aga1=0\0"
	"aga2=0\0"
	"txchain=15\0"
	"rxchain=15\0"
	"antswitch=0\0"
	"tssiposslope2g=1\0"
	"epagain2g=0\0"
	"pdgain2g=0\0"
	"tworangetssi2g=0\0"
	"papdcap2g=0\0"
	"femctrl=2\0"
	"tssiposslope5g=1\0"
	"epagain5g=0\0"
	"pdgain5g=0\0"
	"tworangetssi5g=0\0"
	"papdcap5g=0\0"
	"gainctrlsph=0\0"
	"tempthresh=110\0"
	"tempoffset=255\0"
	"rawtempsense=0x20\0"
	"measpower=0x0\0"
	"tempsense_slope=0xff\0"
	"tempcorrx=0x3f\0"
	"tempsense_option=0x3\0"
	"xtalfreq=80000\0"
	"pa5gbw4080a1=0x0,0x739c,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0\0"
	"phycal_tempdelta=40\0"
	"temps_period=10\0"
	"temps_hysteresis=5\0"
	"measpower1=0x0\0"
	"measpower2=0x0\0"
	"tssifloor2g=0x1c\0"
	"tssifloor5g=0x0,0x300,0x35a,0x35a\0"
	"pdoffset2g40ma0=0\0"
	"pdoffset2g40ma1=0\0"
	"pdoffset2g40ma2=0\0"
	"pdoffset2g40mvalid=0\0"
	"pdoffset40ma0=0\0"
	"pdoffset40ma1=0\0"
	"pdoffset40ma2=0\0"
	"pdoffset80ma0=65535\0"
	"pdoffset80ma1=0\0"
	"pdoffset80ma2=0\0"
	"subband5gver=0x5\0"
	"paparambwver=0\0"
	"rx5ggainwar=0\0"
	"pa5gbw4080a0=0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x14a5\0"
	"pa5gbw40a0=0x0,0x0,0x0,0x2020,0x3820,0x2020,0x3820,0x0,0x0,0x0,0x0,0x0\0"
	"pa5gbw80a0=0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x14a5\0"
	"cckbw202gpo=8738\0"
	"cckbw20ul2gpo=8738\0"
	"mcsbw202gpo=3433731650\0"
	"mcsbw402gpo=3433731650\0"
	"dot11agofdmhrbw202gpo=43686\0"
	"ofdmlrbw202gpo=8770\0"
	"mcsbw205glpo=2558944322\0"
	"mcsbw405glpo=2575721538\0"
	"mcsbw805glpo=2844156994\0"
	"mcsbw205gmpo=2558944322\0"
	"mcsbw405gmpo=2558944322\0"
	"mcsbw805gmpo=2844156994\0"
	"mcsbw205ghpo=2288411714\0"
	"mcsbw405ghpo=2288411714\0"
	"mcsbw805ghpo=2826331202\0"
	"mcslr5glpo=0\0"
	"mcslr5gmpo=0\0"
	"mcslr5ghpo=0\0"
	"sb20in40hrpo=0\0"
	"sb20in80and160hr5glpo=0\0"
	"sb40and80hr5glpo=0\0"
	"sb20in80and160hr5gmpo=0\0"
	"sb40and80hr5gmpo=0\0"
	"sb20in80and160hr5ghpo=0\0"
	"sb40and80hr5ghpo=0\0"
	"sb20in40lrpo=0\0"
	"sb20in80and160lr5glpo=0\0"
	"sb40and80lr5glpo=0\0"
	"sb20in80and160lr5gmpo=0\0"
	"sb40and80lr5gmpo=0\0"
	"sb20in80and160lr5ghpo=0\0"
	"sb40and80lr5ghpo=0\0"
	"dot11agduphrpo=14\0"
	"dot11agduplrpo=0\0"
	"sar2g=18\0"
	"sar5g=15\0"
	"noiselvl2ga0=31\0"
	"noiselvl2ga1=31\0"
	"noiselvl2ga2=31\0"
	"noiselvl5ga0=31,31,31,31\0"
	"noiselvl5ga1=31,31,31,31\0"
	"noiselvl5ga2=31,31,31,31\0"
	"eu_edthresh2g=255\0"
	"eu_edthresh5g=255\0"
	"rxgainerr2ga0=8\0"
	"rxgainerr2ga1=0\0"
	"rxgainerr2ga2=4\0"
	"rxgainerr5ga0=4,8,6,10\0"
	"rxgainerr5ga1=0,30,30,26\0"
	"rxgainerr5ga2=2,0,2,30\0"
	"rpcal2g=0\0"
	"rpcal5gb0=0\0"
	"rpcal5gb1=0\0"
	"rpcal5gb2=0\0"
	"rpcal5gb3=0\0"
	"txidxcap2g=0\0"
	"txidxcap5g=0\0"
	"pdoffsetcckma0=0\0"
	"pdoffsetcckma1=0\0"
	"pdoffsetcckma2=0\0"
	"boardflags4=0x12230008\0"
	"pdoffsetcck=165\0"
	"pdoffset20in40m5gb0=27482\0"
	"pdoffset20in40m5gb1=27482\0"
	"pdoffset20in40m5gb2=27482\0"
	"pdoffset20in40m5gb3=27482\0"
	"pdoffset20in40m5gb4=27482\0"
	"pdoffset40in80m5gb0=0\0"
	"pdoffset40in80m5gb1=0\0"
	"pdoffset40in80m5gb2=0\0"
	"pdoffset40in80m5gb3=0\0"
	"pdoffset40in80m5gb4=0\0"
	"pdoffset20in80m5gb0=27482\0"
	"pdoffset20in80m5gb1=27482\0"
	"pdoffset20in80m5gb2=27482\0"
	"pdoffset20in80m5gb3=27482\0"
	"pdoffset20in80m5gb4=29530\0"
	"mcsbw205gx1po=2288411714\0"
	"mcsbw405gx1po=2288411714\0"
	"mcsbw805gx1po=2825282626\0"
	"mcsbw205gx2po=2825282626\0"
	"mcsbw405gx2po=2825282626\0"
	"mcsbw805gx2po=2825282626\0"
	"sb20in80and160hr5gx1po=0\0"
	"sb20in80and160lr5gx1po=0\0"
	"sb40and80hr5gx1po=0\0"
	"sb20in80and160hr5gx2po=0\0"
	"sb20in80and160lr5gx2po=0\0"
	"sb40and80hr5gx2po=0\0"
	"rxgains5gmelnagaina0=0\0"
	"rxgains5gmelnagaina1=0\0"
	"rxgains5gmelnagaina2=0\0"
	"rxgains5gmtrisoa0=0\0"
	"rxgains5gmtrisoa1=0\0"
	"rxgains5gmtrisoa2=0\0"
	"rxgains5gmtrelnabypa0=0\0"
	"rxgains5gmtrelnabypa1=0\0"
	"rxgains5gmtrelnabypa2=0\0"
	"rxgains5ghelnagaina0=7\0"
	"rxgains5ghelnagaina1=7\0"
	"rxgains5ghelnagaina2=7\0"
	"rxgains5ghtrisoa0=15\0"
	"rxgains5ghtrisoa1=15\0"
	"rxgains5ghtrisoa2=15\0"
	"rxgains5ghtrelnabypa0=1\0"
	"rxgains5ghtrelnabypa1=1\0"
	"rxgains5ghtrelnabypa2=1\0"
	"gpdn=0x0\0"
	"rpcal2gcore3=0\0"
	"rpcal5gb0core3=0\0"
	"rpcal5gb1core3=0\0"
	"rpcal5gb2core3=0\0"
	"rpcal5gb3core3=0\0"
	"sw_txchain_mask=15\0"
	"sw_rxchain_mask=15\0"
	"agbg3=0\0"
	"aga3=0\0"
	"noiselvl2ga3=31\0"
	"noiselvl5ga3=31,31,31,31\0"
	"rxgainerr2ga3=28\0"
	"rxgainerr5ga3=28,26,0,26\0"
	"rxgains5gmelnagaina3=0\0"
	"rxgains5gmtrisoa3=0\0"
	"rxgains5gmtrelnabypa3=0\0"
	"rxgains5ghelnagaina3=7\0"
	"rxgains5ghtrisoa3=15\0"
	"rxgains5ghtrelnabypa3=1\0"
	"wb_rxattn=0x0\0"
	"wb_tia_gain_mode=0x0\0"
	"wb_txbuf_offset=0x0\0"
	"peak_psd_limit_6G2G=0x0\0"
	"peak_psd_limit_5G0=0x0\0"
	"peak_psd_limit_5G1=0x0\0"
	"peak_psd_limit_5G2=0x0\0"
	"pdoffset20in40m5gb5=65535\0"
	"pdoffset20in40m5gb6=65535\0"
	"pdoffset20in80m5gb5=65535\0"
	"pdoffset20in80m5gb6=65535\0"
	"pdoffset40in80m5gb5=65535\0"
	"pdoffset40in80m5gb6=65535\0"
	"pdoffset20in40m5gcore3_2=65535\0"
	"pdoffset20in80m5gcore3_2=65535\0"
	"pdoffset40in80m5gcore3_2=65535\0"
	"pdoffset20in160m5gc4=65535\0"
	"pdoffset20in160m5gc5=65535\0"
	"pdoffset20in160m5gc6=65535\0"
	"pdoffset20in160m5gcore3_2=65535\0"
	"pdoffset40in160m5gc4=65535\0"
	"pdoffset40in160m5gc5=65535\0"
	"pdoffset40in160m5gc6=65535\0"
	"pdoffset40in160m5gcore3_2=65535\0"
	"pdoffset80in160m5gc4=65535\0"
	"pdoffset80in160m5gc5=65535\0"
	"pdoffset80in160m5gc6=65535\0"
	"pdoffset80in160m5gcore3_2=65535\0"
	"maxp5gb5a0=255\0"
	"maxp5gb6a0=255\0"
	"maxp5gb5a1=255\0"
	"maxp5gb6a1=255\0"
	"maxp5gb5a2=255\0"
	"maxp5gb6a2=255\0"
	"maxp5gb5a3=255\0"
	"maxp5gb6a3=255\0"
	"rxgains6gband0elnagaina0=7\0"
	"rxgains6gband0elnagaina1=7\0"
	"rxgains6gband0elnagaina2=7\0"
	"rxgains6gband0elnagaina3=7\0"
	"rxgains6gband0trisoa0=15\0"
	"rxgains6gband0trisoa1=15\0"
	"rxgains6gband0trisoa2=15\0"
	"rxgains6gband0trisoa3=15\0"
	"rxgains6gband0trelnabypa0=1\0"
	"rxgains6gband0trelnabypa1=1\0"
	"rxgains6gband0trelnabypa2=1\0"
	"rxgains6gband0trelnabypa3=1\0"
	"rxgains6gband1elnagaina0=7\0"
	"rxgains6gband1elnagaina1=7\0"
	"rxgains6gband1elnagaina2=7\0"
	"rxgains6gband1elnagaina3=7\0"
	"rxgains6gband1trisoa0=15\0"
	"rxgains6gband1trisoa1=15\0"
	"rxgains6gband1trisoa2=15\0"
	"rxgains6gband1trisoa3=15\0"
	"rxgains6gband1trelnabypa0=1\0"
	"rxgains6gband1trelnabypa1=1\0"
	"rxgains6gband1trelnabypa2=1\0"
	"rxgains6gband1trelnabypa3=1\0"
	"rxgains6gband2elnagaina0=7\0"
	"rxgains6gband2elnagaina1=7\0"
	"rxgains6gband2elnagaina2=7\0"
	"rxgains6gband2elnagaina3=7\0"
	"rxgains6gband2trisoa0=15\0"
	"rxgains6gband2trisoa1=15\0"
	"rxgains6gband2trisoa2=15\0"
	"rxgains6gband2trisoa3=15\0"
	"rxgains6gband2trelnabypa0=1\0"
	"rxgains6gband2trelnabypa1=1\0"
	"rxgains6gband2trelnabypa2=1\0"
	"rxgains6gband2trelnabypa3=1\0"
	"rxgains6gband3elnagaina0=7\0"
	"rxgains6gband3elnagaina1=7\0"
	"rxgains6gband3elnagaina2=7\0"
	"rxgains6gband3elnagaina3=7\0"
	"rxgains6gband3trisoa0=15\0"
	"rxgains6gband3trisoa1=15\0"
	"rxgains6gband3trisoa2=15\0"
	"rxgains6gband3trisoa3=15\0"
	"rxgains6gband3trelnabypa0=1\0"
	"rxgains6gband3trelnabypa1=1\0"
	"rxgains6gband3trelnabypa2=1\0"
	"rxgains6gband3trelnabypa3=1\0"
	"rxgains6gband4elnagaina0=7\0"
	"rxgains6gband4elnagaina1=7\0"
	"rxgains6gband4elnagaina2=7\0"
	"rxgains6gband4elnagaina3=7\0"
	"rxgains6gband4trisoa0=15\0"
	"rxgains6gband4trisoa1=15\0"
	"rxgains6gband4trisoa2=15\0"
	"rxgains6gband4trisoa3=15\0"
	"rxgains6gband4trelnabypa0=1\0"
	"rxgains6gband4trelnabypa1=1\0"
	"rxgains6gband4trelnabypa2=1\0"
	"rxgains6gband4trelnabypa3=1\0"
	"rxgains6gband5elnagaina0=7\0"
	"rxgains6gband5elnagaina1=7\0"
	"rxgains6gband5elnagaina2=7\0"
	"rxgains6gband5elnagaina3=7\0"
	"rxgains6gband5trisoa0=15\0"
	"rxgains6gband5trisoa1=15\0"
	"rxgains6gband5trisoa2=15\0"
	"rxgains6gband5trisoa3=15\0"
	"rxgains6gband5trelnabypa0=1\0"
	"rxgains6gband5trelnabypa1=1\0"
	"rxgains6gband5trelnabypa2=1\0"
	"rxgains6gband5trelnabypa3=1\0"
	"rxgains6gband6elnagaina0=7\0"
	"rxgains6gband6elnagaina1=7\0"
	"rxgains6gband6elnagaina2=7\0"
	"rxgains6gband6elnagaina3=7\0"
	"rxgains6gband6trisoa0=15\0"
	"rxgains6gband6trisoa1=15\0"
	"rxgains6gband6trisoa2=15\0"
	"rxgains6gband6trisoa3=15\0"
	"rxgains6gband6trelnabypa0=1\0"
	"rxgains6gband6trelnabypa1=1\0"
	"rxgains6gband6trelnabypa2=1\0"
	"rxgains6gband6trelnabypa3=1\0"
	"rxgainerr6ga0=63,63,63,63,63,63,63\0"
	"rxgainerr6ga1=31,31,31,31,31,31,31\0"
	"rxgainerr6ga2=31,31,31,31,31,31,31\0"
	"rxgainerr6ga3=31,31,31,31,31,31,31\0"
	"pdoffset20in40m5gcore3=27482\0"
	"pdoffset20in40m5gcore3_1=858\0"
	"pdoffset20in80m5gcore3=27485\0"
	"pdoffset20in80m5gcore3_1=922\0"
	"pdoffset40in80m5gcore3=0\0"
	"pdoffset40in80m5gcore3_1=0\0"
	"pdoffset20in40m2g=29596\0"
	"pdoffset20in40m2gcore3=28\0"
	"pdoffsetcck20m=5285\0"
	"pdoffsetcckch14=0\0"
	"mcs1024qam2gpo=65278\0"
	"mcs1024qam5glpo=4006399180\0"
	"mcs1024qam5gmpo=4006399180\0"
	"mcs1024qam5ghpo=13421772\0"
	"mcs1024qam5gx1po=13421772\0"
	"mcs1024qam5gx2po=0\0"
	"mcsbw1605glpo=3397936742\0"
	"mcsbw1605gmpo=3397936196\0"
	"mcsbw1605ghpo=3397936196\0"
	"mcsbw1605gx1po=3397936196\0"
	"mcsbw1605gx2po=3433718852\0"
	"mcs8poexp=0\0"
	"mcs9poexp=0\0"
	"mcs10poexp=941629472\0"
	"mcs11poexp=941629472\0"
	"cckbw202gpodelta=0\0"
	"sb20in40hrlrpox=0\0"
	"mcslr5gx1po=0\0"
	"mcslr5gx2po=0\0"
	"mcslr5gx3po=65535\0"
	"mcslr5gx4po=65535\0"
	"mcsbw205gx3po=4294967295\0"
	"mcsbw405gx3po=4294967295\0"
	"mcsbw805gx3po=4294967295\0"
	"mcsbw205gx4po=4294967295\0"
	"mcsbw405gx4po=4294967295\0"
	"mcsbw805gx4po=4294967295\0"
	"sb20in80and160hr5gx3po=65535\0"
	"sb40and80hr5gx3po=65535\0"
	"sb20in80and160lr5gx3po=65535\0"
	"sb20in80and160hr5gx4po=65535\0"
	"sb40and80hr5gx4po=65535\0"
	"sb20in80and160lr5gx4po=65535\0"
	"mcsbw1605gx3po=0\0"
	"mcsbw1605gx4po=0\0"
	"mcs1024qam5gx3po=4294967295\0"
	"mcs1024qam5gx4po=4294967295\0"
	"sb40and80lr5gx1po=0\0"
	"sb40and80lr5gx2po=0\0"
	"sb40and80lr5gx3po=65535\0"
	"sb40and80lr5gx4po=65535\0"
	"swctrlmap4_cfg=1793\0"
	"swctrlmap4_TX2g_fem3to0=26214\0"
	"swctrlmap4_RX2g_fem3to0=4369\0"
	"swctrlmap4_RXmid2g_fem3to0=17476\0"
	"swctrlmap4_RXByp2g_fem3to0=13107\0"
	"swctrlmap4_misc2g_fem3to0=0\0"
	"swctrlmap4_TX5g_fem3to0=26214\0"
	"swctrlmap4_RX5g_fem3to0=4369\0"
	"swctrlmap4_RXmid5g_fem3to0=17476\0"
	"swctrlmap4_RXByp5g_fem3to0=13107\0"
	"swctrlmap4_misc5g_fem3to0=0\0"
	"swctrlmap4_TX2g_fem7to4=0\0"
	"swctrlmap4_RX2g_fem7to4=0\0"
	"swctrlmap4_RXByp2g_fem7to4=0\0"
	"swctrlmap4_misc2g_fem7to4=0\0"
	"swctrlmap4_TX5g_fem7to4=0\0"
	"swctrlmap4_RX5g_fem7to4=0\0"
	"swctrlmap4_RXByp5g_fem7to4=0\0"
	"swctrlmap4_misc5g_fem7to4=0\0"
	"rxmid2g_elnagain=13107\0"
	"rxmid5g_elnagain=13107\0"
	"papd_swctrl=65535\0"
	"papd_swctrl_5g6g=65535\0"
	"papd_swctrlmap_fem2g=65535\0"
	"papd_swctrlmap_fem5g=65535\0"
	"papd_swctrlmap_fem6g=65535\0"
	"papd_swctrlmap_femlp2g=65535\0"
	"papd_swctrlmap_femlp5g=65535\0"
	"papd_swctrlmap_femlp6g=65535\0"
	"pdoffset20in160m5gc0=31710\0"
	"pdoffset20in160m5gc1=31710\0"
	"pdoffset20in160m5gc2=29628\0"
	"pdoffset20in160m5gc3=29628\0"
	"pdoffset20in160m5gcore3=924\0"
	"pdoffset20in160m5gcore3_1=924\0"
	"pdoffset40in160m5gc0=4294\0"
	"pdoffset40in160m5gc1=4294\0"
	"pdoffset40in160m5gc2=2178\0"
	"pdoffset40in160m5gc3=2178\0"
	"pdoffset40in160m5gcore3=98\0"
	"pdoffset40in160m5gcore3_1=0\0"
	"pdoffset80in160m5gc0=30750\0"
	"pdoffset80in160m5gc1=30750\0"
	"pdoffset80in160m5gc2=30750\0"
	"pdoffset80in160m5gc3=30750\0"
	"pdoffset80in160m5gcore3=924\0"
	"pdoffset80in160m5gcore3_1=924\0"
	"low_adc_rate_en=1\0"
	"ppr_backoff_2g=3\0"
	"ppr_backoff_5g=3\0"
	"dssf_dis_ch138=1\0"
	"olpc_2g_th=10\0"
	"olpc_5g_th=10\0"
	"coex_gpioctrl_0=65535\0"
	"coex_gpioctrl_1=65535\0"
	"coex_gpioctrl_2=65535\0"
	"coex_gpioctrl_3=65535\0"
	"ru106ppr_2g_0=38741\0"
	"ru106ppr_2g_1=15788\0"
	"ru106ppr_2g_2=17968\0"
	"ru242ppr_2g_0=38485\0"
	"ru242ppr_2g_1=15787\0"
	"ru242ppr_2g_2=16912\0"
	"ru106ppr_5g_0=38741\0"
	"ru106ppr_5g_1=15788\0"
	"ru106ppr_5g_2=17968\0"
	"ru242ppr_5g_0=38485\0"
	"ru242ppr_5g_1=15787\0"
	"ru242ppr_5g_2=16912\0"
	"ru484ppr_5g_0=38741\0"
	"ru484ppr_5g_1=15788\0"
	"ru484ppr_5g_2=17968\0"
	"ru996ppr_5g_0=38485\0"
	"ru996ppr_5g_1=15787\0"
	"ru996ppr_5g_2=16912\0"
	"txs_shaper_en_2g=1\0"
	"txs_shaper_en_5g=1\0"
	"txs_shaper_en_6g=1\0"
	"txs_shaper_bypass=0\0"
	"txs_chan_rate_en=103\0"
	"txs_shaper_dis_2g_nonbndg=1\0"
	"txs_shaper_dis_5g_nonbndg=1\0"
	"txs_shaper_en_unii3=1\0"
	"txs_shaper_en_unii4=1\0"
	"subband_ed_adj=65535\0"
	"avsflags=0xffff\0"
	"hwrssioffset_cmn_2g_0=15\0"
	"hwrssioffset_trt_2g_0=15\0"
	"hwrssioffset_cmn_2g_1=15\0"
	"hwrssioffset_trt_2g_1=15\0"
	"hwrssioffset_cmn_2g_2=15\0"
	"hwrssioffset_trt_2g_2=15\0"
	"hwrssioffset_cmn_2g_3=15\0"
	"hwrssioffset_trt_2g_3=15\0"
	"hwrssioffset_cmn_5g_6g_0=15,15,15,15,15,15,15\0"
	"hwrssioffset_cmn_5g_6g_1=15,15,15,15,15,15,15\0"
	"hwrssioffset_cmn_5g_6g_2=15,15,15,15,15,15,15\0"
	"hwrssioffset_cmn_5g_6g_3=15,15,15,15,15,15,15\0"
	"hwrssioffset_trt_5g_6g_0=15,15,15,15,15,15,15\0"
	"hwrssioffset_trt_5g_6g_1=15,15,15,15,15,15,15\0"
	"hwrssioffset_trt_5g_6g_2=15,15,15,15,15,15,15\0"
	"hwrssioffset_trt_5g_6g_3=15,15,15,15,15,15,15\0"
	"maxp2ga0=108\0"
	"maxp5gb4a0=100\0"
	"pa2ga0=0x1d2c,0xeaad,0xf1bd,0xc21\0"
	"pa2g40a0=0x1c68,0xfca6,0xe855,0x0\0"
	"maxp5gb0a0=100\0"
	"maxp5gb1a0=100\0"
	"maxp5gb2a0=100\0"
	"maxp5gb3a0=100\0"
	"pa5ga0=0x22ab,0xbf39,0x0,0x0,0x22fd,0xbef1,0x0,0x0,0x227d,0xc18e,0x0,0x0,0x223f,0xc1b2,"
	"0xffff,0x6b,0x21cd,0xc260,0x0,0x0\0"
	"pa5g40a0=0x22e6,0xc820,0xfb1c,0x844,0x2346,0xc5aa,0xfbd2,0x0,0x227c,0xcc82,0xf9c4,0x0,"
	"0x225d,0xcba8,0xfa6e,0x0,0x2207,0xc966,0xfbb7,0x0\0"
	"pa5g80a0=0x2344,0xbf25,0x0,0xac6,0x23c0,0xbdc7,0x0,0x0,0x234e,0xc0e6,0xffff,0x9b0,0x2310,"
	"0xc16a,0x0,0x835,0x22b3,0xc13a,0x0,0x0\0"
	"rxgains2gelnagaina0=4\0"
	"rxgains2gtrisoa0=7\0"
	"rxgains2gtrelnabypa0=1\0"
	"rxgains5gelnagaina0=4\0"
	"rxgains5gtrisoa0=7\0"
	"rxgains5gtrelnabypa0=1\0"
	"pa2g20ccka0=0x1dc8,0xfb13,0xe8fe,0x0\0"
	"pa5g160a0=0x234a,0xc071,0x0,0x1935,0x22db,0xcce7,0xfb55,0x1fab,0x2348,0xc360,0x0,0x210a,"
	"0x227f,0xc360,0x0,0x210a\0"
	"pa5gexta0=0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff\0"
	"pa5gext40a0=0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff\0"
	"pa5gext80a0=0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff\0"
	"pa5gext160a0=0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,"
	"0xffff,0xffff\0"
	"maxp2ga1=108\0"
	"maxp5gb4a1=100\0"
	"pa2ga1=0x1d0a,0xec65,0xf0ac,0xcf6\0"
	"pa2g40a1=0x1d5f,0xf2ec,0xeceb,0xe04\0"
	"maxp5gb0a1=100\0"
	"maxp5gb1a1=100\0"
	"maxp5gb2a1=100\0"
	"maxp5gb3a1=100\0"
	"pa5ga1=0x223b,0xbf49,0x0,0x0,0x227c,0xbf2f,0x0,0x0,0x2291,0xbf6b,0x0,0x0,0x2280,0xc03f,"
	"0x0,0x0,0x2210,0xc099,0x0,0x0\0"
	"pa5g40a1=0x22f9,0xc456,0xfd0a,0xee3,0x23ad,0xc44e,0xfbfe,0x0,0x22d0,0xca54,0xfa2e,0x0,"
	"0x22d3,0xc810,0xfb61,0x0,0x229d,0xc6d6,0xfc41,0x0\0"
	"pa5g80a1=0x23ae,0xbd6c,0x0,0x0,0x23e3,0xbd94,0x0,0x0,0x23c9,0xbe39,0x0,0x0,0x2323,0xbf97,"
	"0x0,0x8bd,0x2363,0xbf4e,0x0,0x0\0"
	"rxgains2gelnagaina1=4\0"
	"rxgains2gtrisoa1=7\0"
	"rxgains2gtrelnabypa1=1\0"
	"rxgains5gelnagaina1=4\0"
	"rxgains5gtrisoa1=7\0"
	"rxgains5gtrelnabypa1=1\0"
	"pa2g20ccka1=0x1da9,0xfe92,0xe6f5,0x0\0"
	"pa5g160a1=0x2399,0xbf1c,0x0,0xdd0,0x2371,0xc76f,0xfde0,0x242e,0x2388,0xc2d2,0xfe84,0xc30,"
	"0x22f1,0xc2d2,0xfe84,0xc30\0"
	"pa5gexta1=0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff\0"
	"pa5gext40a1=0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff\0"
	"pa5gext80a1=0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff\0"
	"pa5gext160a1=0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,"
	"0xffff,0xffff\0"
	"maxp2ga2=108\0"
	"maxp5gb4a2=100\0"
	"pa2ga2=0x1dad,0xead1,0xf17b,0x857\0"
	"pa2g40a2=0x1cb7,0xfd7c,0xe80c,0x0\0"
	"maxp5gb0a2=100\0"
	"maxp5gb1a2=100\0"
	"maxp5gb2a2=100\0"
	"maxp5gb3a2=100\0"
	"pa5ga2=0x229b,0xbefd,0x0,0x0,0x22db,0xbe81,0x0,0x0,0x22b5,0xbefc,0x0,0x0,0x225d,0xbf9d,"
	"0x0,0x0,0x21da,0xc0cc,0x0,0x0\0"
	"pa5g40a2=0x239b,0xc268,0xfdc2,0x1188,0x23b5,0xc2db,0xfd50,0xa39,0x231d,0xc663,0xfba9,0x0,"
	"0x23ad,0xc180,0xfe86,0x953,0x239b,0xc026,0xffd5,0x12b3\0"
	"pa5g80a2=0x2418,0xbcea,0x0,0x0,0x2412,0xbcd4,0x0,0x0,0x23f2,0xbd94,0x0,0x0,0x2380,0xbe25,"
	"0x0,0x0,0x2332,0xbec5,0x0,0x0\0"
	"rxgains2gelnagaina2=4\0"
	"rxgains2gtrisoa2=7\0"
	"rxgains2gtrelnabypa2=1\0"
	"rxgains5gelnagaina2=4\0"
	"rxgains5gtrisoa2=7\0"
	"rxgains5gtrelnabypa2=1\0"
	"pa2g20ccka2=0x1dc3,0xf7f7,0xea7e,0x0\0"
	"pa5g160a2=0x23ef,0xbed7,0x0,0x11f6,0x220a,0xcbf5,0xfcc9,0x246c,0x237f,0xc036,0xfffe,"
	"0x1770,0x231c,0xc036,0xfffe,0x1770\0"
	"pa5gexta2=0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff\0"
	"pa5gext40a2=0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff\0"
	"pa5gext80a2=0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff\0"
	"pa5gext160a2=0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,"
	"0xffff,0xffff\0"
	"maxp2ga3=108\0"
	"maxp5gb4a3=100\0"
	"pa2ga3=0x1e6b,0xe643,0xf401,0xeae\0"
	"pa2g40a3=0x1e99,0xf084,0xee82,0xb88\0"
	"maxp5gb0a3=100\0"
	"maxp5gb1a3=100\0"
	"maxp5gb2a3=100\0"
	"maxp5gb3a3=100\0"
	"pa5ga3=0x2322,0xbf46,0x0,0x0,0x2364,0xbeef,0x0,0x0,0x2369,0xbf5d,0x0,0x0,0x229b,0xc0d7,"
	"0xfffe,0x0,0x2230,0xc1d0,0x0,0x0\0"
	"pa5g40a3=0x22b0,0xcbc1,0xf92b,0x4a7,0x241e,0xc707,0xfaed,0x0,0x23ee,0xc64d,0xfbc4,0x0,"
	"0x240b,0xc311,0xfded,0xa17,0x23f6,0xbfad,0xffff,0xb91\0"
	"pa5g80a3=0x23f4,0xbeee,0x0,0xdbf,0x248f,0xbcec,0x0,0x0,0x2493,0xbdb6,0x0,0x0,0x242e,"
	"0xbe61,0x0,0x0,0x23c4,0xbf0c,0x0,0x0\0"
	"rxgains2gelnagaina3=4\0"
	"rxgains2gtrisoa3=7\0"
	"rxgains2gtrelnabypa3=1\0"
	"rxgains5gelnagaina3=4\0"
	"rxgains5gtrisoa3=7\0"
	"rxgains5gtrelnabypa3=1\0"
	"pa2g20ccka3=0x1e1f,0xf493,0xebac,0x0\0"
	"pa5g160a3=0x2413,0xc0c4,0x0,0x1f51,0x223e,0xcb79,0xfc72,0x2340,0x2476,0xbfc2,0xff71,"
	"0x3f9,0x23d0,0xbfc2,0xff71,0x3f9\0"
	"pa5gexta3=0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff\0"
	"pa5gext40a3=0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff\0"
	"pa5gext80a3=0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff\0"
	"pa5gext160a3=0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,"
	"0xffff,0xffff\0"
	"END\0";

static char BCMATTACHDATA(defaultsromvars_6711)[] =
	"sromrev=18\0"
	"boardrev=0x1101\0"
	"boardflags=0x10001000\0"
	"boardflags2=0x0\0"
	"boardtype=0xa36\0"
	"devid=0x6711\0" /* BCM6711_D11BE2G_ID */
#if defined(BCMQT) && defined(BCMQT_PHYLESS)
	/* Temporary use phyrev133 and radioid20712 (6726B0 used) instead of
	 * phyrev137 and radioid20714 to bypass phy assertions
	 */
	"phy_qt_radioid=0x50e8\0"
	"phy_qt_phyrev=0x85\0"
	"phy_qt_caps_no_5g=1\0"
	"phy_qt_caps_no_6g=1\0"
#endif /* BCMQT && BCMQT_PHYLESS */
	"subvid=0x14e4\0"
	"boardflags3=0x0\0"
	"boardnum=9999\0"
	"macaddr=00:90:4c:3d:83:18\0"
	"ccode=0x0\0"
	"regrev=0\0"
	"aa2g=15\0"
	"agbg0=0\0"
	"agbg1=0\0"
	"agbg2=0\0"
	"aga0=0\0"
	"aga1=0\0"
	"aga2=0\0"
	"txchain=7\0"
	"rxchain=7\0"
	"antswitch=0\0"
	"tssiposslope2g=1\0"
	"epagain2g=0\0"
	"pdgain2g=0\0"
	"tworangetssi2g=0\0"
	"papdcap2g=0\0"
	"femctrl=2\0"
	"gainctrlsph=0\0"
	"tempthresh=110\0"
	"tempoffset=255\0"
	"rawtempsense=0x20\0"
	"measpower=0x0\0"
	"tempsense_slope=0xff\0"
	"tempcorrx=0x3f\0"
	"tempsense_option=0x3\0"
	"xtalfreq=80000\0"
	"phycal_tempdelta=40\0"
	"temps_period=10\0"
	"temps_hysteresis=5\0"
	"measpower1=0x0\0"
	"measpower2=0x0\0"
	"tssifloor2g=0x1c\0"
	"pdoffset2g40ma0=0\0"
	"pdoffset2g40ma1=0\0"
	"pdoffset2g40ma2=0\0"
	"pdoffset2g40mvalid=0\0"
	"pdoffset40ma0=0\0"
	"pdoffset40ma1=0\0"
	"pdoffset40ma2=0\0"
	"pdoffset80ma0=65535\0"
	"pdoffset80ma1=0\0"
	"pdoffset80ma2=0\0"
	"paparambwver=0\0"
	"cckbw202gpo=8738\0"
	"cckbw20ul2gpo=8738\0"
	"mcsbw202gpo=3433731650\0"
	"mcsbw402gpo=3433731650\0"
	"dot11agofdmhrbw202gpo=43686\0"
	"ofdmlrbw202gpo=8770\0"
	"sb20in40hrpo=0\0"
	"sb20in40lrpo=0\0"
	"dot11agduphrpo=14\0"
	"dot11agduplrpo=0\0"
	"sar2g=18\0"
	"noiselvl2ga0=31\0"
	"noiselvl2ga1=31\0"
	"noiselvl2ga2=31\0"
	"eu_edthresh2g=255\0"
	"rxgainerr2ga0=8\0"
	"rxgainerr2ga1=0\0"
	"rxgainerr2ga2=4\0"
	"rpcal2g=0\0"
	"txidxcap2g=0\0"
	"pdoffsetcckma0=0\0"
	"pdoffsetcckma1=0\0"
	"pdoffsetcckma2=0\0"
	"boardflags4=0x12230008\0"
	"pdoffsetcck=165\0"
	"gpdn=0x0\0"
	"rpcal2gcore3=0\0"
	"sw_txchain_mask=15\0"
	"sw_rxchain_mask=15\0"
	"agbg3=0\0"
	"aga3=0\0"
	"noiselvl2ga3=31\0"
	"rxgainerr2ga3=28\0"
	"wb_rxattn=0x0\0"
	"wb_tia_gain_mode=0x0\0"
	"wb_txbuf_offset=0x0\0"
	"peak_psd_limit_6G2G=0x0\0"
	"pdoffset20in40m2g=29596\0"
	"pdoffset20in40m2gcore3=28\0"
	"pdoffsetcck20m=5285\0"
	"pdoffsetcckch14=0\0"
	"mcs1024qam2gpo=65278\0"
	"mcs8poexp=0\0"
	"mcs9poexp=0\0"
	"mcs10poexp=941629472\0"
	"mcs11poexp=941629472\0"
	"cckbw202gpodelta=0\0"
	"sb20in40hrlrpox=0\0"
	"swctrlmap4_cfg=1793\0"
	"swctrlmap4_TX2g_fem3to0=26214\0"
	"swctrlmap4_RX2g_fem3to0=4369\0"
	"swctrlmap4_RXmid2g_fem3to0=17476\0"
	"swctrlmap4_RXByp2g_fem3to0=13107\0"
	"swctrlmap4_misc2g_fem3to0=0\0"
	"swctrlmap4_TX2g_fem7to4=0\0"
	"swctrlmap4_RX2g_fem7to4=0\0"
	"swctrlmap4_RXByp2g_fem7to4=0\0"
	"swctrlmap4_misc2g_fem7to4=0\0"
	"rxmid2g_elnagain=13107\0"
	"papd_swctrl=65535\0"
	"papd_swctrlmap_fem2g=65535\0"
	"papd_swctrlmap_femlp2g=65535\0"
	"low_adc_rate_en=1\0"
	"ppr_backoff_2g=3\0"
	"dssf_dis_ch138=1\0"
	"olpc_2g_th=10\0"
	"coex_gpioctrl_0=65535\0"
	"coex_gpioctrl_1=65535\0"
	"coex_gpioctrl_2=65535\0"
	"coex_gpioctrl_3=65535\0"
	"ru106ppr_2g_0=38741\0"
	"ru106ppr_2g_1=15788\0"
	"ru106ppr_2g_2=17968\0"
	"ru242ppr_2g_0=38485\0"
	"ru242ppr_2g_1=15787\0"
	"ru242ppr_2g_2=16912\0"
	"txs_shaper_en_2g=1\0"
	"txs_shaper_bypass=0\0"
	"txs_chan_rate_en=103\0"
	"txs_shaper_dis_2g_nonbndg=1\0"
	"txs_shaper_en_unii3=1\0"
	"txs_shaper_en_unii4=1\0"
	"subband_ed_adj=65535\0"
	"avsflags=0xffff\0"
	"hwrssioffset_cmn_2g_0=15\0"
	"hwrssioffset_trt_2g_0=15\0"
	"hwrssioffset_cmn_2g_1=15\0"
	"hwrssioffset_trt_2g_1=15\0"
	"hwrssioffset_cmn_2g_2=15\0"
	"hwrssioffset_trt_2g_2=15\0"
	"hwrssioffset_cmn_2g_3=15\0"
	"hwrssioffset_trt_2g_3=15\0"
	"maxp2ga0=108\0"
	"pa2ga0=0x1d2c,0xeaad,0xf1bd,0xc21\0"
	"pa2g40a0=0x1c68,0xfca6,0xe855,0x0\0"
	"rxgains2gelnagaina0=4\0"
	"rxgains2gtrisoa0=7\0"
	"rxgains2gtrelnabypa0=1\0"
	"pa2g20ccka0=0x1dc8,0xfb13,0xe8fe,0x0\0"
	"maxp2ga1=108\0"
	"pa2ga1=0x1d0a,0xec65,0xf0ac,0xcf6\0"
	"pa2g40a1=0x1d5f,0xf2ec,0xeceb,0xe04\0"
	"rxgains2gelnagaina1=4\0"
	"rxgains2gtrisoa1=7\0"
	"rxgains2gtrelnabypa1=1\0"
	"pa2g20ccka1=0x1da9,0xfe92,0xe6f5,0x0\0"
	"maxp2ga2=108\0"
	"pa2ga2=0x1dad,0xead1,0xf17b,0x857\0"
	"pa2g40a2=0x1cb7,0xfd7c,0xe80c,0x0\0"
	"rxgains2gelnagaina2=4\0"
	"rxgains2gtrisoa2=7\0"
	"rxgains2gtrelnabypa2=1\0"
	"pa2g20ccka2=0x1dc3,0xf7f7,0xea7e,0x0\0"
	"maxp2ga3=108\0"
	"pa2ga3=0x1e6b,0xe643,0xf401,0xeae\0"
	"pa2g40a3=0x1e99,0xf084,0xee82,0xb88\0"
	"rxgains2gelnagaina3=4\0"
	"rxgains2gtrisoa3=7\0"
	"rxgains2gtrelnabypa3=1\0"
	"pa2g20ccka3=0x1e1f,0xf493,0xebac,0x0\0"
	"END\0";

#endif
#endif /* !defined(BCMDONGLEHOST) */

#if defined(BCMQT) && defined(CONFIG_BCM96756)
static char BCMATTACHDATA(defaultsromvars_6756)[] =
	"boardflags=0x00000000\0"
	"boardflags2=0xc0000004\0"
	"boardflags3=0x40000000\0"
	"boardflags4=0x10008\0"
	"sromrev=18\0"
	"boardtype=0x08a3\0"
	"boardrev=0x1201\0"
	"subvid=0x106b\0"
	"vendid=0x14e4\0"
	"boardnum=${serno}\0"
	"devid=0x6019\0"
	"macaddr=00:90:4c:be:20:01\0"
	"ccode=ALL\0"
	"regrev=0\0"
	"aa2g=0x3\0"
	"aa5g=0x3\0"
	"agbg0=0x83\0"
	"agbg1=0x83\0"
	"aga0=0x43\0"
	"aga1=0x43\0"
	"txchain=0x3\0"
	"rxchain=0x3\0"
	"tssiposslope2g=1\0"
	"epacal2g=1\0"
	"epagain2g=2\0"
	"pdgain2g=0\0"
	"tworangetssi2g=0\0"
	"papdcap2g=0\0"
	"tssiposslope5g=1\0"
	"epacal5g=0\0"
	"epagain5g=0\0"
	"pdgain5g=0\0"
	"tworangetssi5g=0\0"
	"papdcap5g=0\0"
	"tempthresh=125\0"
	"tempoffset=255\0"
	"rawtempsense=0x1ff\0"
	"measpower=0\0"
	"tempsense_slope=0xff\0"
	"tempcorrx=0x3f\0"
	"tempsense_option=0x3\0"
	"xtalfreq=50000\0"
	"phycal_tempdelta=40\0"
	"temps_hysteresis=5\0"
	"measpower1=0\0"
	"measpower2=0\0"
	"pdoffsetcck=0x42\0"
	"pdoffsetcck20m=0=0x42\0"
	"pdoffset20in40m2g=0x3bd\0"
	"pdoffset20in40m5gb0=0x35a\0"
	"pdoffset20in40m5gb1=0x35a\0"
	"pdoffset20in40m5gb2=0x35a\0"
	"pdoffset20in40m5gb3=0x35a\0"
	"pdoffset20in40m5gb4=0x35a\0"
	"pdoffset40in80m5gb0=0x63\0"
	"pdoffset40in80m5gb1=0x63\0"
	"pdoffset40in80m5gb2=0x63\0"
	"pdoffset40in80m5gb3=0x63\0"
	"pdoffset40in80m5gb4=0x63\0"
	"pdoffset20in80m5gb0=0x39c\0"
	"pdoffset20in80m5gb1=0x39c\0"
	"pdoffset20in80m5gb2=0x39c\0"
	"pdoffset20in80m5gb3=0x39c\0"
	"pdoffset20in80m5gb4=0x39c\0"
	"subband5gver=0x5\0"
	"mcs1024qam2gpo=0x00009999\0"
	"mcs1024qam5glpo=0x00bbbbbb\0"
	"mcs1024qam5gmpo=0x00bbbbbb\0"
	"mcs1024qam5ghpo=0x00bbbbbb\0"
	"mcs1024qam5gx1po=0x00bbbbbb\0"
	"mcs1024qam5gx2po=0x00bbbbbb\0"
	"mcs8poexp=0\0"
	"mcs9poexp=0\0"
	"mcs10poexp=0\0"
	"mcs11poexp=0\0"
	"cckbw202gpo=0x0000\0"
	"cckbw20ul2gpo=0x0000\0"
	"mcsbw202gpo=0x77555442\0"
	"mcsbw402gpo=0x88555442\0"
	"dot11agofdmhrbw202gpo=0x0000\0"
	"ofdmlrbw202gpo=0x0000\0"
	"mcsbw205glpo=0xbb777442\0"
	"mcsbw405glpo=0x99777442\0"
	"mcsbw805glpo=0x99666442\0"
	"mcsbw205gmpo=0xbb777442\0"
	"mcsbw405gmpo=0x99777442\0"
	"mcsbw805gmpo=0x99666442\0"
	"mcsbw205ghpo=0xbb777442\0"
	"mcsbw405ghpo=0x99777442\0"
	"mcsbw805ghpo=0x99666442\0"
	"mcsbw205gx1po=0xbb777442\0"
	"mcsbw405gx1po=0x99777442\0"
	"mcsbw805gx1po=0x99666442\0"
	"mcsbw205gx2po=0xbb777442\0"
	"mcsbw405gx2po=0x99777442\0"
	"mcsbw805gx2po=0x99666442\0"
	"mcslr5glpo=0x0000\0"
	"mcslr5gmpo=0x0000\0"
	"mcslr5ghpo=0x0000\0"
	"mcslr5gx1po=0x0000\0"
	"mcslr5gx2po=0x0000\0"
	"sb20in40hrpo=0\0"
	"sb20in40lrpo=0\0"
	"sb20in40hrlrpox=0\0"
	"dot11agduphrpo=0\0"
	"dot11agduplrpo=0\0"
	"dot11agduphrlrpox=0\0"
	"sb40and80hr5glpo=0\0"
	"sb40and80hr5gmpo=0\0"
	"sb40and80hr5ghpo=0\0"
	"sb40and80hr5gx1po=0\0"
	"sb40and80hr5gx2po=0\0"
	"sb20in80and160hr5glpo=0\0"
	"sb20in80and160hr5gmpo=0\0"
	"sb20in80and160hr5ghpo=0\0"
	"sb20in80and160hr5gx1po=0\0"
	"sb20in80and160hr5gx2po=0\0"
	"sb40and80lr5glpo=0\0"
	"sb40and80lr5gmpo=0\0"
	"sb40and80lr5ghpo=0\0"
	"sb40and80lr5gx1po=0\0"
	"sb40and80lr5gx2po=0\0"
	"sb20in80and160lr5glpo=0\0"
	"sb20in80and160lr5gmpo=0\0"
	"sb20in80and160lr5ghpo=0\0"
	"sb20in80and160lr5gx1po=0\0"
	"sb20in80and160lr5gx2po=0\0"
	"noiselvl2ga0=31\0"
	"noiselvl2ga1=31\0"
	"noiselvl5ga0=31,31,31,31\0"
	"noiselvl5ga1=31,31,31,31\0"
	"rxgainerr2ga0=63\0"
	"rxgainerr2ga1=31\0"
	"rxgainerr5ga0=63,63,63,63\0"
	"rxgainerr5ga1=31,31,31,31\0"
	"rxgains5gmelnagaina0=3\0"
	"rxgains5gmelnagaina1=3\0"
	"rxgains5gmtrisoa0=9\0"
	"rxgains5gmtrisoa1=9\0"
	"rxgains5gmtrelnabypa0=1\0"
	"rxgains5gmtrelnabypa1=1\0"
	"rxgains5ghelnagaina0=3\0"
	"rxgains5ghelnagaina1=3\0"
	"rxgains5ghtrisoa0=7\0"
	"rxgains5ghtrisoa1=7\0"
	"rxgains5ghtrelnabypa0=1\0"
	"rxgains5ghtrelnabypa1=1\0"
	"maxp2ga0=0x58\0"
	"maxp5gb0a0=0x64\0"
	"maxp5gb1a0=0x64\0"
	"maxp5gb2a0=0x64\0"
	"maxp5gb3a0=0x64\0"
	"maxp5gb4a0=0x64\0"
	"maxp2ga1=0x58\0"
	"maxp5gb0a1=0x64\0"
	"maxp5gb1a1=0x64\0"
	"maxp5gb2a1=0x64\0"
	"maxp5gb3a1=0x64\0"
	"maxp5gb4a1=0x64\0"
	"rxgains5gelnagaina0=3\0"
	"rxgains5gtrisoa0=8\0"
	"rxgains5gtrelnabypa0=1\0"
	"rxgains5gelnagaina1=3\0"
	"rxgains5gtrisoa1=8\0"
	"rxgains5gtrelnabypa1=1\0"
	"pa2ga0=0x1d05,0xd6e2,0x0,0x2b14\0"
	"pa2ga1=0x1f20,0xd56f,0x0,0x2b93\0"
	"pa2g40a0=0x1f93,0xd1a6,0x0,0x2df9\0"
	"pa2g40a1=0x21d6,0xcfa9,0x0,0x2cd1\0"
	"pa5ga0=0x26E4,0xBA9B,0x0000,0x050A,0x2653,0xBC80,0x0000,0x0EFE,0x2685,0xBCF1,0x0000,"
	"0x11BD,0x2647,0xBDD5,0x0000,0x1094,0x268E,0xBC66,0x0000,0x08F4\0"
	"pa5ga1=0x2480,0xBF36,0x0000,0x0539,0x2653,0xBC80,0x0000,0x0EFE,0x2453,0xC06C,0x0000,"
	"0x0B7D,0x2427,0xC1AF,0x0000,0x1681,0x2422,0xC203,0xFFFC,0x169D\0"
	"pa5g40a0=0x292B,0xB7B2,0x0000,0x17DF,0x28E4,0xB8EE,0x0000,0x1B45,0x2756,0xC3CD,0xFB5F,"
	"0x0D42,0x2823,0xBCAA,0xFFF8,0x2684,0x2722,0xC542,0xFBD7,0x1F21\0"
	"pa5g40a1=0x2699,0xBCDB,0x0000,0x1969,0x25BD,0xC295,0xFE1D,0x1D96,0x26BD,0xBD2F,0x0000,"
	"0x17C0,0x25DE,0xC13B,0xFF3C,0x1FEF,0x267F,0xBDFA,0xFFAC,0x1714\0"
	"pa5g80a0=0x273E,0xBD16,0x0000,0x27D2,0x253C,0xCC51,0xF7AF,0x0000,0x26B5,0xC1FE,0xFD24,"
	"0x122D,0x2749,0xBD2F,0x0000,0x1E51,0x27C0,0xBAB5,0x0000,0x11A1\0"
	"pa5g80a1=0x2544,0xBFD7,0x0000,0x1F1D,0x24A2,0xC356,0xFF21,0x2113,0x2526,0xC04D,0x0000,"
	"0x1972,0x2571,0xBF07,0x0000,0x122E,0x23C1,0xCCBF,0xF983,0x0859\0"
	"pa5g160a0=0x2544,0xBFD7,0x0000,0x1F1D,0x24A2,0xC356,0xFF21,0x2113,0x2526,0xC04D,0x0000,"
	"0x1972,0x2571,0xBF07,0x0000,0x122E,0x23C1,0xCCBF,0xF983,0x0859\0"
	"pa5g160a1=0x2544,0xBFD7,0x0000,0x1F1D,0x24A2,0xC356,0xFF21,0x2113,0x2526,0xC04D,0x0000,"
	"0x1972,0x2571,0xBF07,0x0000,0x122E,0x23C1,0xCCBF,0xF983,0x0859\0"
	"pa5gb2w0a0=0xff29\0"
	"pa5gb2w1a0=0x1615\0"
	"pa5gb2w2a0=0xfd2e\0"
	"pa5gb2w0a1=0xff2b\0"
	"pa5gb2w1a1=0x15c9\0"
	"pa5gb2w2a1=0xfd3a\0"
	"pa2g20ccka0=0x1eff,0xd270,0x0,0x2a3d\0"
	"pa2g20ccka1=0x2112,0xced0,0x0,0x27c8\0"
	"AvVmid_c0=3,85,4,70,4,70,4,70,4,75,4,70\0"
	"AvVmid_c1=3,85,4,70,4,80,4,70,4,75,4,70\0"
	"rpcal2g=0\0"
	"rpcal5gb0=0\0"
	"rpcal5gb1=0\0"
	"rpcal5gb2=0\0"
	"rpcal5gb3=0\0"
	"rpcal5gb4=0\0"
	"swctrlmap4_cfg=0x7\0"
	"swctrlmap4_TX2g_fem3to0=0x11\0"
	"swctrlmap4_RX2g_fem3to0=0x22\0"
	"swctrlmap4_RXByp2g_fem3to0=0x22\0"
	"swctrlmap4_misc2g_fem3to0=0x0\0"
	"swctrlmap4_TX5g_fem3to0=0x88\0"
	"swctrlmap4_RX5g_fem3to0=0x44\0"
	"swctrlmap4_RXByp5g_fem3to0=0x44\0"
	"swctrlmap4_misc5g_fem3to0=0x0\0"
	"swctrlmap4_TX2g_fem7to4=0x11\0"
	"swctrlmap4_RX2g_fem7to4=0x0\0"
	"swctrlmap4_RXByp2g_fem7to4=0x0\0"
	"swctrlmap4_misc2g_fem7to4=0x0\0"
	"swctrlmap4_TX5g_fem7to4=0x22\0"
	"swctrlmap4_RX5g_fem7to4=0x0\0"
	"swctrlmap4_RXByp5g_fem7to4=0x22\0"
	"swctrlmap4_misc5g_fem7to4=0x0\0"
	"paprdis=1\0"
	"low_adc_rate_en=1\0"
	"nb_rxattn=0x0303\0"
	"nb_txattn=0x0303\0"
	"nb_papdcalidx=0x0a1a\0"
	"nb_bbmult=0x5740\0"
	"nb_tia_gain_mode=0x0502\0"
	"pacalshift2g=-2,-2\0"
	"spurcan_CoreMask=2\0"
	"END\0";
#endif /* BCMQT && CONFIG_BCM96756 */

/* It must end with pattern of "END" */

/**
 * @return The length in [bytes] of a string of concatenated name/value nvram pairs.
 */
uint
BCMATTACHFN(srom_vars_len)(char *vars)
{
	uint pos = 0;
	uint len;
	char *s;
	char *emark = "END";
	uint emark_len = strlen(emark) + 1;

	for (s = vars; s && *s;) {
		if (strcmp(s, emark) == 0)
			break;

		len = strlen(s);
		s += strlen(s) + 1;
		pos += len + 1;
		/* BS_ERROR(("len %d vars[pos] %s\n", pos, s)); */
#if (defined(CMWIFI) && defined(BCMEXTNVM))
		if (pos >= (MAXSZ_NVRAM_VARS - emark_len)) {
#else
		if (pos >= (VARS_MAX - emark_len)) {
#endif /* (defined(CMWIFI) && defined(BCMEXTNVM)) */
			return 0;
		}
	}

	return pos + emark_len;	/* include the "END\0" */
}

#if !defined(BCMDONGLEHOST)
/** Initialization of varbuf structure */
void
BCMATTACHFN(varbuf_init)(varbuf_t *b, char *buf, uint size)
{
	b->size = size;
	b->base = b->buf = buf;
}

/** append a null terminated var=value string */
int
BCMATTACHFN(varbuf_append)(varbuf_t *b, const char *fmt, ...)
{
	va_list ap;
	int r;
	size_t len;
	char *s;

	if (b->size < 2)
	  return 0;

	va_start(ap, fmt);
	r = vsnprintf(b->buf, b->size, fmt, ap);
	va_end(ap);

	/* C99 snprintf behavior returns r >= size on overflow,
	 * others return -1 on overflow.
	 * All return -1 on format error.
	 * We need to leave room for 2 null terminations, one for the current var
	 * string, and one for final null of the var table. So check that the
	 * strlen written, r, leaves room for 2 chars.
	 */
	if ((r == -1) || (r > (int)(b->size - 2))) {
		b->size = 0;
		return 0;
	}

	/* Remove any earlier occurrence of the same variable */
	if ((s = strchr(b->buf, '=')) != NULL) {
		len = (size_t)(s - b->buf);
		for (s = b->base; s < b->buf;) {
			if ((bcmp(s, b->buf, len) == 0) && s[len] == '=') {
				len = strlen(s) + 1;
				memmove(s, (s + len), ((b->buf + r + 1) - (s + len)));
				b->buf -= len;
				b->size += (unsigned int)len;
				break;
			}

			while (*s++)
				;
		}
	}

	/* skip over this string's null termination */
	r++;
	b->size -= r;
	b->buf += r;

	return r;
}

/**
 * Initialize local vars from the right source for this platform. Called from siutils.c.
 *
 * @param[out] vars pointer to a to-be created pointer area for nvram variable name/value pairs.
 *             Some callers of this function set 'vars' to NULL, in that case this function will
 *             prematurely return.
 *
 * @return    0 on success, nonzero on error.
 *
 * Prerequisites:
 * - The CC core has been selected
 */
int
BCMATTACHFN(srom_var_init)(si_t *sih, enum bustype_e bustype, void *curmap, osl_t *osh,
	char **vars, uint *count)
{
#ifdef BCMBUSTYPE
	ASSERT(bustype == BUSTYPE(bustype));
#endif
	if (vars == NULL || count == NULL)
		return (0);

	*vars = NULL;
	*count = 0;

	switch (BUSTYPE(bustype)) {
	case SI_BUS:
#if defined(DSLCPE) && defined(DSLCPE_WOMBO)
		ASSERT(curmap != NULL);
		return initvars_srom_pci(sih, curmap, vars, count);
#endif /* defined(DSLCPE) && defined(DSLCPE_WOMBO) */
	/* deliberate fall through */
	case JTAG_BUS:
#ifdef BCMPCIEDEV
		if (BCMPCIEDEV_ENAB()) {
			int ret;

			ret = initvars_cis_pci(sih, osh, curmap, vars, count);

#ifdef BCMPCIEDEV_SROM_FORMAT
			if (ret)
				ret = initvars_srom_pci(sih, curmap, vars, count);
#endif
			if (ret)
				ret = initvars_srom_si_pciedev(sih, osh, curmap, vars, count);

			return ret;
		} else
#endif /* BCMPCIEDEV */
		{
#ifdef ATE_BUILD
			/* Skip the process of srom for ATE since it is not needed */
			return BCME_OK;
#elif defined(BCMPCIEDEV)
			return initvars_srom_si_pciedev(sih, osh, curmap, vars, count);
#else /* !BCMPCIEDEV */
			return initvars_si(sih, vars, count);
#endif /* PCIEDEV */
		}
	case PCI_BUS: {
		int ret;
		ASSERT(curmap != NULL);
		if (curmap == NULL)
			return (-1);

		/* First check for CIS format. if not CIS, try SROM format */
		if ((ret = initvars_cis_pci(sih, osh, curmap, vars, count)))
			return initvars_srom_pci(sih, curmap, vars, count);
		return ret;
	}

#ifdef BCMSPI
	case SPI_BUS:
		return initvars_cis_spi(sih, osh, vars, count);
#endif /* BCMSPI */

	default:
		ASSERT(0);
	}

	return (-1);
}
#endif /* !defined(BCMDONGLEHOST) */

/** support only 16-bit word read from srom */
int
srom_read(si_t *sih, enum bustype_e bustype, void *curmap, osl_t *osh,
          uint byteoff, uint nbytes, uint16 *buf, bool check_crc)
{
	uint off, nw;

#ifdef BCMBUSTYPE
	ASSERT(bustype == BUSTYPE(bustype));
#endif

	/* check input - 16-bit access only */
	if (byteoff & 1 || nbytes & 1 || (byteoff + nbytes) > SROM_MAX)
		return 1;

	off = byteoff / 2;
	nw = nbytes / 2;

#ifdef BCMPCIEDEV
	if ((BUSTYPE(bustype) == SI_BUS) &&
	    (BCM43684_CHIP(sih->chip) ||
	     BCM6715_CHIP(sih->chip) ||
	     BCM6717_CHIP(sih->chip) ||
	     BCM6726_CHIP(sih->chip))) {
		/* building firmware for chips with a PCIe interface and internal SI bus */
#else
	if (BUSTYPE(bustype) == PCI_BUS) {
#endif /* BCMPCIEDEV */
		if (!curmap)
			return 1;

		if (si_is_sprom_available(sih)) {
			uint16 *shadow;

			shadow = (uint16 *)srom_offset(sih, curmap);
			if (shadow == NULL)
				return 1;

			if (sprom_read_pci(osh, sih, shadow, off, buf, nw, check_crc))
				return 1;
		}
#if !defined(BCMDONGLEHOST) && (defined(BCMNVRAMW) || defined(BCMNVRAMR))
		else if (!((BUSTYPE(bustype) == SI_BUS) &&
			(BCM43684_CHIP(sih->chip)))) {
			if (otp_read_pci(osh, sih, buf, nbytes))
				return 1;
		}
#endif /* !BCMDONGLEHOST && (BCMNVRAMW||BCMNVRAMR) */
#ifdef BCMSPI
	} else if (BUSTYPE(bustype) == SPI_BUS) {
	                if (bcmsdh_cis_read(NULL, SDIO_FUNC_1, (uint8 *)buf, byteoff + nbytes) != 0)
				return 1;
#endif /* BCMSPI */
	} else if (BUSTYPE(bustype) == SI_BUS) {
		return 1;
	} else {
		return 1;
	}

	return 0;
} /* srom_read */

#if defined(WLTEST) || defined(DHD_SPROM) || defined(BCMDBG)
/** support only 16-bit word write into srom */
int
srom_write(si_t *sih, enum bustype_e bustype, void *curmap, osl_t *osh,
           uint byteoff, uint nbytes, uint16 *buf)
{
	uint i, nw, crc_range;
	uint16 *old, *new;
	uint8 crc;
	int rc = 1;

#ifdef BCMBUSTYPE
	ASSERT(bustype == BUSTYPE(bustype));
#endif
	ASSERT(sih->sprom_bus != SPROM_BUS_NO_SPROM);

	/* freed in same function */
	old = MALLOC_NOPERSIST(osh, SROM_MAXW * sizeof(uint16));
	new = MALLOC_NOPERSIST(osh, SROM_MAXW * sizeof(uint16));

	if (old == NULL || new == NULL)
		goto done;

	/* check input - 16-bit access only. use byteoff 0x55aa to indicate
	 * srclear
	 */
	if ((byteoff != 0x55aa) && ((byteoff & 1) || (nbytes & 1)))
		goto done;

	if ((byteoff != 0x55aa) && ((byteoff + nbytes) > SROM_MAX))
		goto done;

	else {
		crc_range = srom_size(sih, osh);
	}

	nw = crc_range / 2;
	/* read first small number words from srom, then adjust the length, read all */
	if (srom_read(sih, bustype, curmap, osh, 0, crc_range, old, FALSE))
		goto done;

	BS_ERROR(("%s: old[SROM4_SIGN] 0x%x, old[SROM8_SIGN] 0x%x\n",
	          __FUNCTION__, old[SROM4_SIGN], old[SROM8_SIGN]));
	/* Deal with blank srom */
	if (old[0] == 0xffff) {
		/* Do nothing to blank srom when it's srclear */
		if (byteoff == 0x55aa) {
			rc = 0;
			goto done;
		}

		/* see if the input buffer is valid SROM image or not */
		if (buf[SROM11_SIGN] == SROM11_SIGNATURE) {
			BS_ERROR(("%s: buf[SROM11_SIGN] 0x%x\n",
				__FUNCTION__, buf[SROM11_SIGN]));

			/* block invalid buffer size */
			if (nbytes < SROM11_WORDS * 2) {
				rc = BCME_BUFTOOSHORT;
				goto done;
			} else if (nbytes > SROM11_WORDS * 2) {
				rc = BCME_BUFTOOLONG;
				goto done;
			}

			nw = SROM11_WORDS;

		} else if (buf[SROM12_SIGN] == SROM12_SIGNATURE) {
			BS_ERROR(("%s: buf[SROM12_SIGN] 0x%x\n",
				__FUNCTION__, buf[SROM12_SIGN]));

			/* block invalid buffer size */
			if (nbytes < SROM12_WORDS * 2) {
				rc = BCME_BUFTOOSHORT;
				goto done;
			} else if (nbytes > SROM12_WORDS * 2) {
				rc = BCME_BUFTOOLONG;
				goto done;
			}

			nw = SROM12_WORDS;

		} else if (buf[SROM13_SIGN] == SROM13_SIGNATURE) {
			BS_ERROR(("%s: buf[SROM13_SIGN] 0x%x\n",
				__FUNCTION__, buf[SROM13_SIGN]));

			/* block invalid buffer size */
			if (nbytes < SROM13_WORDS * 2) {
				rc = BCME_BUFTOOSHORT;
				goto done;
			} else if (nbytes > SROM13_WORDS * 2) {
				rc = BCME_BUFTOOLONG;
				goto done;
			}

			nw = SROM13_WORDS;

		} else if (buf[SROM16_SIGN] == SROM16_SIGNATURE) {
			BS_ERROR(("%s: buf[SROM16_SIGN] 0x%x\n",
				__FUNCTION__, buf[SROM16_SIGN]));

			/* block invalid buffer size */
			if (nbytes < SROM16_WORDS * 2) {
				rc = BCME_BUFTOOSHORT;
				goto done;
			} else if (nbytes > SROM16_WORDS * 2) {
				rc = BCME_BUFTOOLONG;
				goto done;
			}

			nw = SROM16_WORDS;

		} else if (buf[SROM17_SIGN] == SROM17_SIGNATURE) {
			BS_ERROR(("%s: buf[SROM17_SIGN] 0x%x\n",
				__FUNCTION__, buf[SROM17_SIGN]));

			/* block invalid buffer size */
			if (nbytes < SROM17_WORDS * 2) {
				rc = BCME_BUFTOOSHORT;
				goto done;
			} else if (nbytes > SROM17_WORDS * 2) {
				rc = BCME_BUFTOOLONG;
				goto done;
			}

			nw = SROM17_WORDS;

		} else if (buf[SROM18_SIGN] == SROM18_SIGNATURE) {
			BS_ERROR(("%s: buf[SROM18_SIGN] 0x%x\n",
				__FUNCTION__, buf[SROM18_SIGN]));

			/* nbytes can be < SROM18 bytes since host limits transfer chunk size
			 * to 1518 Bytes
			 */
			if (nbytes > SROM18_WORDS * 2) {
				rc = BCME_BUFTOOLONG;
				goto done;
			}

			nw = SROM18_WORDS;
		} else if (buf[SROM19_SIGN] == SROM19_SIGNATURE) {
			BS_ERROR(("%s: buf[SROM19_SIGN] 0x%x\n", __FUNCTION__, buf[SROM19_SIGN]));
			/* nbytes can be < SROM19 bytes since host limits transfer chunk size
			 * to 1518 Bytes
			 */
			if (nbytes > SROM19_WORDS * sizeof(uint16)) {
				rc = BCME_BUFTOOLONG;
				goto done;
			}

			nw = SROM19_WORDS;
		} else if (buf[SROM20_SIGN] == SROM20_SIGNATURE) {
			BS_ERROR(("%s: buf[SROM20_SIGN] 0x%x\n", __FUNCTION__, buf[SROM20_SIGN]));
			/* nbytes can be < SROM20 bytes since host limits transfer chunk size
			 * to 1518 Bytes
			 */
			if (nbytes > SROM20_WORDS * sizeof(uint16)) {
				rc = BCME_BUFTOOLONG;
				goto done;
			}

			nw = SROM20_WORDS;
		} else if (buf[SROM11_SIGN] == SROM15_SIGNATURE) {
			BS_ERROR(("%s: buf[SROM15_SIGN] 0x%x\n",
				__FUNCTION__, buf[SROM11_SIGN]));
			 /* nbytes can be < SROM15 bytes since host limits transfer chunk size
			 * to 1518 Bytes
			 */
			 if (nbytes > SROM15_WORDS * 2) {
				rc = BCME_BUFTOOLONG;
				goto done;
			}
			nw = SROM15_WORDS;
		} else if ((buf[SROM4_SIGN] == SROM4_SIGNATURE) ||
			(buf[SROM8_SIGN] == SROM4_SIGNATURE)) {
			BS_ERROR(("%s: buf[SROM4_SIGN] 0x%x, buf[SROM8_SIGN] 0x%x\n",
				__FUNCTION__, buf[SROM4_SIGN], buf[SROM8_SIGN]));

			/* block invalid buffer size */
			if (nbytes < SROM4_WORDS * 2) {
				rc = BCME_BUFTOOSHORT;
				goto done;
			} else if (nbytes > SROM4_WORDS * 2) {
				rc = BCME_BUFTOOLONG;
				goto done;
			}

			nw = SROM4_WORDS;
		} else if (nbytes == SROM_WORDS * 2){ /* the other possible SROM format */
			BS_ERROR(("%s: Not SROM4 or SROM8.\n", __FUNCTION__));

			nw = SROM_WORDS;
		} else {
			BS_ERROR(("%s: Invalid input file signature\n", __FUNCTION__));
			rc = BCME_BADARG;
			goto done;
		}
		crc_range = nw * 2;
		if (srom_read(sih, bustype, curmap, osh, 0, crc_range, old, FALSE)) {
			goto done;
		}
	} else if (old[SROM20_SIGN] == SROM20_SIGNATURE) {
		nw = SROM20_WORDS;
		crc_range = nw * sizeof(uint16);
		if (srom_read(sih, bustype, curmap, osh, 0, crc_range, old, FALSE)) {
			goto done;
		}
	} else if (old[SROM19_SIGN] == SROM19_SIGNATURE) {
		nw = SROM19_WORDS;
		crc_range = nw * sizeof(uint16);
		if (srom_read(sih, bustype, curmap, osh, 0, crc_range, old, FALSE)) {
			goto done;
		}
	} else if (old[SROM18_SIGN] == SROM18_SIGNATURE) {
		nw = SROM18_WORDS;
		crc_range = nw * 2;
		if (srom_read(sih, bustype, curmap, osh, 0, crc_range, old, FALSE)) {
			goto done;
		}
	} else if (old[SROM17_SIGN] == SROM17_SIGNATURE) {
		nw = SROM17_WORDS;
		crc_range = nw * 2;
		if (srom_read(sih, bustype, curmap, osh, 0, crc_range, old, FALSE)) {
			goto done;
		}
	} else if (old[SROM16_SIGN] == SROM16_SIGNATURE) {
		nw = SROM16_WORDS;
		crc_range = nw * 2;
		if (srom_read(sih, bustype, curmap, osh, 0, crc_range, old, FALSE)) {
			goto done;
		}
	} else if (old[SROM15_SIGN] == SROM15_SIGNATURE) {
		nw = SROM15_WORDS;
		crc_range = nw * 2;
		if (srom_read(sih, bustype, curmap, osh, 0, crc_range, old, FALSE)) {
			goto done;
		}
	} else if (old[SROM13_SIGN] == SROM13_SIGNATURE) {
		nw = SROM13_WORDS;
		crc_range = nw * 2;
		if (srom_read(sih, bustype, curmap, osh, 0, crc_range, old, FALSE)) {
			goto done;
		}
	} else if (old[SROM12_SIGN] == SROM12_SIGNATURE) {
		nw = SROM12_WORDS;
		crc_range = nw * 2;
		if (srom_read(sih, bustype, curmap, osh, 0, crc_range, old, FALSE)) {
			goto done;
		}
	} else if (old[SROM11_SIGN] == SROM11_SIGNATURE) {
		nw = SROM11_WORDS;
		crc_range = nw * 2;
		if (srom_read(sih, bustype, curmap, osh, 0, crc_range, old, FALSE)) {
			goto done;
		}
	} else if ((old[SROM4_SIGN] == SROM4_SIGNATURE) ||
	           (old[SROM8_SIGN] == SROM4_SIGNATURE)) {
		nw = SROM4_WORDS;
		crc_range = nw * 2;
		if (srom_read(sih, bustype, curmap, osh, 0, crc_range, old, FALSE)) {
			goto done;
		}
	} else {
		/* Assert that we have already read enough for sromrev 2 */
		ASSERT(crc_range >= SROM_WORDS * 2);
		nw = SROM_WORDS;
		crc_range = nw * 2;
	}

	if (byteoff == 0x55aa) {
		/* Erase request */
		crc_range = 0;
		memset((void *)new, 0xff, nw * 2);
	} else {
		/* Copy old contents */
		bcopy((void *)old, (void *)new, nw * 2);
		/* make changes */
		bcopy((void *)buf, (void *)&new[byteoff / 2], nbytes);
	}

	if (crc_range) {
		/* calculate crc */
		htol16_buf(new, crc_range);
		crc = ~hndcrc8((uint8 *)new, crc_range - 1, CRC8_INIT_VALUE);
		ltoh16_buf(new, crc_range);
		new[nw - 1] = (crc << 8) | (new[nw - 1] & 0xff);
	}

#ifdef BCMPCIEDEV
	if ((BUSTYPE(bustype) == SI_BUS) &&
	    (BCM43684_CHIP(sih->chip) ||
	     BCM6715_CHIP(sih->chip) ||
	     BCM6717_CHIP(sih->chip) ||
	     BCM6726_CHIP(sih->chip))) {
#else
	if (BUSTYPE(bustype) == PCI_BUS) {
#endif /* BCMPCIEDEV */
		chipcregs_t *ccregs = NULL;

		if (BCM43684_CHIP(sih->chip) ||
			BCM6710_CHIP(sih->chip) ||
			BCM6715_CHIP(sih->chip) ||
			BCM6717_CHIP(sih->chip) ||
			BCM6726_CHIP(sih->chip) ||
			BCM6711_CHIP(sih->chip)) {
			si_srom_clk_set(sih); /* corrects srom clock frequency */
		}

		/* enable writes to the SPROM */
#if !defined(DSLCPE) && !defined(DSLCPE_WOMBO)
		if (BUSTYPE(sih->bustype) == SI_BUS) {
			ccregs = (chipcregs_t *)(uintptr)SI_ENUM_BASE_PA(sih);
		} else
#endif /* !DSLCPE && !DSLCPE_WOMBO */
		{
			ccregs =
				(chipcregs_t *)((uint8 *)curmap + PCI_16KB0_CCREGS_OFFSET);
		}

		(void)srom_cc_cmd(sih, osh, ccregs, SPROM_OP_WRLOCK, 0, FALSE);

		bcm_mdelay(WRITE_ENABLE_DELAY);
		/* write srom */
		for (i = 0; i < nw; i++) {
			if (old[i] != new[i]) {
				if ((sih->cccaps & CC_CAP_SROM) == 0) {
					/* No srom support in this chip */
					BS_ERROR(("srom_write, invalid srom, skip\n"));
				} else {
					(void)srom_cc_cmd(sih, osh, ccregs, SPROM_OP_WR, i, new[i]);
				}
				bcm_mdelay(WRITE_WORD_DELAY);
			}
		}

		/* disable writes to the SPROM */
		(void)srom_cc_cmd(sih, osh, ccregs, SPROM_OP_WRLOCK, 0, TRUE);

	} else if (BUSTYPE(bustype) == SI_BUS) {
		goto done;
	} else {
		goto done;
	}

	bcm_mdelay(WRITE_ENABLE_DELAY);
	rc = 0;

done:
	if (old != NULL)
		MFREE(osh, old, SROM_MAXW * sizeof(uint16));
	if (new != NULL)
		MFREE(osh, new, SROM_MAXW * sizeof(uint16));

	return rc;
} /* srom_write */

/** support only 16-bit word write into srom */
int
srom_write_short(si_t *sih, enum bustype_e bustype, void *curmap, osl_t *osh,
                 uint byteoff, uint16 value)
{
	int rc = 1;

#ifdef BCMBUSTYPE
	ASSERT(bustype == BUSTYPE(bustype));
#endif

	if (byteoff & 1)
		goto done;

#ifdef BCMPCIEDEV
	if ((BUSTYPE(bustype) == SI_BUS) &&
	    (BCM43684_CHIP(sih->chip) ||
	     BCM6715_CHIP(sih->chip) ||
	     BCM6717_CHIP(sih->chip) ||
	     BCM6726_CHIP(sih->chip))) {
#else
	if (BUSTYPE(bustype) == PCI_BUS) {
#endif /* BCMPCIEDEV */
		void *ccregs = NULL;

		if (BCM43684_CHIP(sih->chip) ||
			BCM6710_CHIP(sih->chip) ||
			BCM6715_CHIP(sih->chip) ||
			BCM6717_CHIP(sih->chip) ||
			BCM6726_CHIP(sih->chip) ||
			BCM6711_CHIP(sih->chip)) {
			si_srom_clk_set(sih); /* corrects srom clock frequency */
		}

		/* enable writes to the SPROM */
#if !defined(DSLCPE) && !defined(DSLCPE_WOMBO)
		if (BUSTYPE(sih->bustype) == SI_BUS)
			ccregs = (void *)(uintptr)SI_ENUM_BASE_PA(sih);
		else
#endif /* !DSLCPE && !DSLCPE_WOMBO */
			ccregs = (void *)((uint8 *)curmap + PCI_16KB0_CCREGS_OFFSET);
		(void)srom_cc_cmd(sih, osh, ccregs, SPROM_OP_WRLOCK, 0, FALSE);

		bcm_mdelay(WRITE_ENABLE_DELAY);
		/* write srom */
		if ((sih->cccaps & CC_CAP_SROM) == 0) {
			/* No srom support in this chip */
			BS_ERROR(("srom_write, invalid srom, skip\n"));
		} else {
			(void)srom_cc_cmd(sih, osh, ccregs, SPROM_OP_WR, byteoff / 2, value);
		}

		bcm_mdelay(WRITE_WORD_DELAY);

		/* disable writes to the SPROM */
		(void)srom_cc_cmd(sih, osh, ccregs, SPROM_OP_WRLOCK, 0, TRUE);

	} else if (BUSTYPE(bustype) == SI_BUS) {
		goto done;
	} else {
		goto done;
	}

	bcm_mdelay(WRITE_ENABLE_DELAY);
	rc = 0;

done:
	return rc;
} /* srom_write_short */

#endif

/**
 * These 'vstr_*' definitions are used to convert from CIS format to a 'NVRAM var=val' format, the
 * NVRAM format is used throughout the rest of the firmware.
 */
#if !defined(BCMDONGLEHOST)
static const char BCMATTACHDATA(vstr_manf)[] = "manf=%s";
static const char BCMATTACHDATA(vstr_productname)[] = "productname=%s";
static const char BCMATTACHDATA(vstr_manfid)[] = "manfid=0x%x";
static const char BCMATTACHDATA(vstr_prodid)[] = "prodid=0x%x";
static const char BCMATTACHDATA(vstr_regwindowsz)[] = "regwindowsz=%d";
static const char BCMATTACHDATA(vstr_sromrev)[] = "sromrev=%d";
static const char BCMATTACHDATA(vstr_chiprev)[] = "chiprev=%d";
static const char BCMATTACHDATA(vstr_subvendid)[] = "subvendid=0x%x";
static const char BCMATTACHDATA(vstr_subdevid)[] = "subdevid=0x%x";
static const char BCMATTACHDATA(vstr_boardrev)[] = "boardrev=0x%x";
static const char BCMATTACHDATA(vstr_aa2g)[] = "aa2g=0x%x";
static const char BCMATTACHDATA(vstr_aa5g)[] = "aa5g=0x%x";
static const char BCMATTACHDATA(vstr_ag)[] = "ag%d=0x%x";
static const char BCMATTACHDATA(vstr_cc)[] = "cc=%d";
static const char BCMATTACHDATA(vstr_opo)[] = "opo=%d";
static const char BCMATTACHDATA(vstr_pa0b)[][9] = { "pa0b0=%d", "pa0b1=%d", "pa0b2=%d" };
static const char BCMATTACHDATA(vstr_pa0b_lo)[][12] =
	{ "pa0b0_lo=%d", "pa0b1_lo=%d", "pa0b2_lo=%d" };
static const char BCMATTACHDATA(vstr_pa0itssit)[] = "pa0itssit=%d";
static const char BCMATTACHDATA(vstr_pa0maxpwr)[] = "pa0maxpwr=%d";
static const char BCMATTACHDATA(vstr_pa1b)[][9] = { "pa1b0=%d", "pa1b1=%d", "pa1b2=%d" };
static const char BCMATTACHDATA(vstr_pa1lob)[][11] =
	{ "pa1lob0=%d", "pa1lob1=%d", "pa1lob2=%d" };
static const char BCMATTACHDATA(vstr_pa1hib)[][11] =
	{ "pa1hib0=%d", "pa1hib1=%d", "pa1hib2=%d" };
static const char BCMATTACHDATA(vstr_pa1itssit)[] = "pa1itssit=%d";
static const char BCMATTACHDATA(vstr_pa1maxpwr)[] = "pa1maxpwr=%d";
static const char BCMATTACHDATA(vstr_pa1lomaxpwr)[] = "pa1lomaxpwr=%d";
static const char BCMATTACHDATA(vstr_pa1himaxpwr)[] = "pa1himaxpwr=%d";
static const char BCMATTACHDATA(vstr_oem)[] = "oem=%02x%02x%02x%02x%02x%02x%02x%02x";
static const char BCMATTACHDATA(vstr_boardflags)[] = "boardflags=0x%x";
static const char BCMATTACHDATA(vstr_boardflags2)[] = "boardflags2=0x%x";
static const char BCMATTACHDATA(vstr_boardflags3)[] = "boardflags3=0x%x";
static const char BCMATTACHDATA(vstr_boardflags4)[] = "boardflags4=0x%x";
static const char BCMATTACHDATA(vstr_boardflags5)[] = "boardflags5=0x%x";
static const char BCMATTACHDATA(vstr_ledbh)[] = "ledbh%d=0x%x";
static const char BCMATTACHDATA(vstr_noccode)[] = "ccode=0x0";
static const char BCMATTACHDATA(vstr_ccodelock)[] = "ccodelock=1";
static const char BCMATTACHDATA(vstr_ccode)[] = "ccode=%c%c";
static const char BCMATTACHDATA(vstr_cctl)[] = "cctl=0x%x";
static const char BCMATTACHDATA(vstr_cckpo)[] = "cckpo=0x%x";
static const char BCMATTACHDATA(vstr_ofdmpo)[] = "ofdmpo=0x%x";
static const char BCMATTACHDATA(vstr_rdlid)[] = "rdlid=0x%x";
#ifdef BCM_BOOTLOADER
static const char BCMATTACHDATA(vstr_rdlrwu)[] = "rdlrwu=%d";
static const char BCMATTACHDATA(vstr_rdlsn)[] = "rdlsn=%d";
#endif /* BCM_BOOTLOADER */
static const char BCMATTACHDATA(vstr_usbfs)[] = "usbfs=%d";
static const char BCMATTACHDATA(vstr_wpsgpio)[] = "wpsgpio=%d";
static const char BCMATTACHDATA(vstr_wpsled)[] = "wpsled=%d";
static const char BCMATTACHDATA(vstr_rssismf2g)[] = "rssismf2g=%d";
static const char BCMATTACHDATA(vstr_rssismc2g)[] = "rssismc2g=%d";
static const char BCMATTACHDATA(vstr_rssisav2g)[] = "rssisav2g=%d";
static const char BCMATTACHDATA(vstr_bxa2g)[] = "bxa2g=%d";
static const char BCMATTACHDATA(vstr_rssismf5g)[] = "rssismf5g=%d";
static const char BCMATTACHDATA(vstr_rssismc5g)[] = "rssismc5g=%d";
static const char BCMATTACHDATA(vstr_rssisav5g)[] = "rssisav5g=%d";
static const char BCMATTACHDATA(vstr_bxa5g)[] = "bxa5g=%d";
static const char BCMATTACHDATA(vstr_tri2g)[] = "tri2g=%d";
static const char BCMATTACHDATA(vstr_tri5gl)[] = "tri5gl=%d";
static const char BCMATTACHDATA(vstr_tri5g)[] = "tri5g=%d";
static const char BCMATTACHDATA(vstr_tri5gh)[] = "tri5gh=%d";
static const char BCMATTACHDATA(vstr_rxpo2g)[] = "rxpo2g=%d";
static const char BCMATTACHDATA(vstr_rxpo5g)[] = "rxpo5g=%d";
static const char BCMATTACHDATA(vstr_boardtype)[] = "boardtype=0x%x";
static const char BCMATTACHDATA(vstr_leddc)[] = "leddc=0x%04x";
static const char BCMATTACHDATA(vstr_vendid)[] = "vendid=0x%x";
static const char BCMATTACHDATA(vstr_devid)[] = "devid=0x%x";
static const char BCMATTACHDATA(vstr_xtalfreq)[] = "xtalfreq=%d";
static const char BCMATTACHDATA(vstr_txchain)[] = "txchain=0x%x";
static const char BCMATTACHDATA(vstr_rxchain)[] = "rxchain=0x%x";
static const char BCMATTACHDATA(vstr_elna2g)[] = "elna2g=0x%x";
static const char BCMATTACHDATA(vstr_elna5g)[] = "elna5g=0x%x";
static const char BCMATTACHDATA(vstr_antswitch)[] = "antswitch=0x%x";
static const char BCMATTACHDATA(vstr_regrev)[] = "regrev=0x%x";
static const char BCMATTACHDATA(vstr_antswctl2g)[] = "antswctl2g=0x%x";
static const char BCMATTACHDATA(vstr_triso2g)[] = "triso2g=0x%x";
static const char BCMATTACHDATA(vstr_pdetrange2g)[] = "pdetrange2g=0x%x";
static const char BCMATTACHDATA(vstr_extpagain2g)[] = "extpagain2g=0x%x";
static const char BCMATTACHDATA(vstr_tssipos2g)[] = "tssipos2g=0x%x";
static const char BCMATTACHDATA(vstr_antswctl5g)[] = "antswctl5g=0x%x";
static const char BCMATTACHDATA(vstr_triso5g)[] = "triso5g=0x%x";
static const char BCMATTACHDATA(vstr_pdetrange5g)[] = "pdetrange5g=0x%x";
static const char BCMATTACHDATA(vstr_extpagain5g)[] = "extpagain5g=0x%x";
static const char BCMATTACHDATA(vstr_tssipos5g)[] = "tssipos5g=0x%x";
static const char BCMATTACHDATA(vstr_maxp2ga)[] = "maxp2ga%d=0x%x";
static const char BCMATTACHDATA(vstr_itt2ga0)[] = "itt2ga0=0x%x";
static const char BCMATTACHDATA(vstr_pa)[] = "pa%dgw%da%d=0x%x";
static const char BCMATTACHDATA(vstr_pahl)[] = "pa%dg%cw%da%d=0x%x";
static const char BCMATTACHDATA(vstr_maxp5ga0)[] = "maxp5ga0=0x%x";
static const char BCMATTACHDATA(vstr_itt5ga0)[] = "itt5ga0=0x%x";
static const char BCMATTACHDATA(vstr_maxp5gha0)[] = "maxp5gha0=0x%x";
static const char BCMATTACHDATA(vstr_maxp5gla0)[] = "maxp5gla0=0x%x";
static const char BCMATTACHDATA(vstr_itt2ga1)[] = "itt2ga1=0x%x";
static const char BCMATTACHDATA(vstr_maxp5ga1)[] = "maxp5ga1=0x%x";
static const char BCMATTACHDATA(vstr_itt5ga1)[] = "itt5ga1=0x%x";
static const char BCMATTACHDATA(vstr_maxp5gha1)[] = "maxp5gha1=0x%x";
static const char BCMATTACHDATA(vstr_maxp5gla1)[] = "maxp5gla1=0x%x";
static const char BCMATTACHDATA(vstr_cck2gpo)[] = "cck2gpo=0x%x";
static const char BCMATTACHDATA(vstr_ofdm2gpo)[] = "ofdm2gpo=0x%x";
static const char BCMATTACHDATA(vstr_ofdm5gpo)[] = "ofdm5gpo=0x%x";
static const char BCMATTACHDATA(vstr_ofdm5glpo)[] = "ofdm5glpo=0x%x";
static const char BCMATTACHDATA(vstr_ofdm5ghpo)[] = "ofdm5ghpo=0x%x";
static const char BCMATTACHDATA(vstr_cddpo)[] = "cddpo=0x%x";
static const char BCMATTACHDATA(vstr_stbcpo)[] = "stbcpo=0x%x";
static const char BCMATTACHDATA(vstr_bw40po)[] = "bw40po=0x%x";
static const char BCMATTACHDATA(vstr_bwduppo)[] = "bwduppo=0x%x";
static const char BCMATTACHDATA(vstr_mcspo)[] = "mcs%dgpo%d=0x%x";
static const char BCMATTACHDATA(vstr_mcspohl)[] = "mcs%dg%cpo%d=0x%x";
static const char BCMATTACHDATA(vstr_custom)[] = "customvar%d=0x%x";
static const char BCMATTACHDATA(vstr_cckdigfilttype)[] = "cckdigfilttype=%d";
static const char BCMATTACHDATA(vstr_usbflags)[] = "usbflags=0x%x";
#ifdef BCM_BOOTLOADER
static const char BCMATTACHDATA(vstr_mdio)[] = "mdio%d=0x%%x";
static const char BCMATTACHDATA(vstr_mdioex)[] = "mdioex%d=0x%%x";
static const char BCMATTACHDATA(vstr_brmin)[] = "brmin=0x%x";
static const char BCMATTACHDATA(vstr_brmax)[] = "brmax=0x%x";
static const char BCMATTACHDATA(vstr_pllreg)[] = "pll%d=0x%x";
static const char BCMATTACHDATA(vstr_ccreg)[] = "chipc%d=0x%x";
static const char BCMATTACHDATA(vstr_regctrl)[] = "reg%d=0x%x";
static const char BCMATTACHDATA(vstr_time)[] = "r%dt=0x%x";
static const char BCMATTACHDATA(vstr_depreg)[] = "r%dd=0x%x";
static const char BCMATTACHDATA(vstr_usbpredly)[] = "usbpredly=0x%x";
static const char BCMATTACHDATA(vstr_usbpostdly)[] = "usbpostdly=0x%x";
static const char BCMATTACHDATA(vstr_usbrdy)[] = "usbrdy=0x%x";
static const char BCMATTACHDATA(vstr_usbdevctrl)[] = "usbdevctrl=0x%x";
static const char BCMATTACHDATA(vstr_bldr_reset_timeout)[] = "bldr_to=0x%x";
static const char BCMATTACHDATA(vstr_muxenab)[] = "muxenab=0x%x";
static const char BCMATTACHDATA(vstr_pubkey)[] = "pubkey=%s";
#endif /* BCM_BOOTLOADER */
static const char BCMATTACHDATA(vstr_boardnum)[] = "boardnum=%d";
static const char BCMATTACHDATA(vstr_macaddr)[] = "macaddr=%s";
static const char BCMATTACHDATA(vstr_macaddr2)[] = "macaddr2=%s";
static const char BCMATTACHDATA(vstr_usbepnum)[] = "usbepnum=0x%x";
static const char BCMATTACHDATA(vstr_usbutmi_ctl)[] = "usbutmi_ctl=0x%x";
static const char BCMATTACHDATA(vstr_usbssphy_utmi_ctl0)[] = "usbssphy_utmi_ctl0=0x%x";
static const char BCMATTACHDATA(vstr_usbssphy_utmi_ctl1)[] = "usbssphy_utmi_ctl1=0x%x";
static const char BCMATTACHDATA(vstr_usbssphy_utmi_ctl2)[] = "usbssphy_utmi_ctl2=0x%x";
static const char BCMATTACHDATA(vstr_usbssphy_sleep0)[] = "usbssphy_sleep0=0x%x";
static const char BCMATTACHDATA(vstr_usbssphy_sleep1)[] = "usbssphy_sleep1=0x%x";
static const char BCMATTACHDATA(vstr_usbssphy_sleep2)[] = "usbssphy_sleep2=0x%x";
static const char BCMATTACHDATA(vstr_usbssphy_sleep3)[] = "usbssphy_sleep3=0x%x";
static const char BCMATTACHDATA(vstr_usbssphy_mdio)[] = "usbssmdio%d=0x%x,0x%x,0x%x,0x%x";
static const char BCMATTACHDATA(vstr_usb30phy_noss)[] = "usbnoss=0x%x";
static const char BCMATTACHDATA(vstr_usb30phy_u1u2)[] = "usb30u1u2=0x%x";
static const char BCMATTACHDATA(vstr_usb30phy_regs)[] = "usb30regs%d=0x%x,0x%x,0x%x,0x%x";

/* Power per rate for SROM V9 */
static const char BCMATTACHDATA(vstr_cckbw202gpo)[][21] =
	{ "cckbw202gpo=0x%x", "cckbw20ul2gpo=0x%x", "cckbw20in802gpo=0x%x" };
static const char BCMATTACHDATA(vstr_legofdmbw202gpo)[][23] =
	{ "legofdmbw202gpo=0x%x", "legofdmbw20ul2gpo=0x%x" };
static const char BCMATTACHDATA(vstr_legofdmbw205gpo)[][24] =
	{ "legofdmbw205glpo=0x%x", "legofdmbw20ul5glpo=0x%x",
	"legofdmbw205gmpo=0x%x", "legofdmbw20ul5gmpo=0x%x",
	"legofdmbw205ghpo=0x%x", "legofdmbw20ul5ghpo=0x%x" };

static const char BCMATTACHDATA(vstr_mcs2gpo)[][19] =
{ "mcsbw202gpo=0x%x", "mcsbw20ul2gpo=0x%x", "mcsbw402gpo=0x%x", "mcsbw802gpo=0x%x" };

static const char BCMATTACHDATA(vstr_mcs5glpo)[][20] =
	{ "mcsbw205glpo=0x%x", "mcsbw20ul5glpo=0x%x", "mcsbw405glpo=0x%x" };

static const char BCMATTACHDATA(vstr_mcs5gmpo)[][20] =
	{ "mcsbw205gmpo=0x%x", "mcsbw20ul5gmpo=0x%x", "mcsbw405gmpo=0x%x" };

static const char BCMATTACHDATA(vstr_mcs5ghpo)[][20] =
	{ "mcsbw205ghpo=0x%x", "mcsbw20ul5ghpo=0x%x", "mcsbw405ghpo=0x%x" };

static const char BCMATTACHDATA(vstr_mcs32po)[] = "mcs32po=0x%x";
static const char BCMATTACHDATA(vstr_legofdm40duppo)[] = "legofdm40duppo=0x%x";

/* SROM V11 */
static const char BCMATTACHDATA(vstr_tempthresh)[] = "tempthresh=%d";	/* HNBU_TEMPTHRESH */
static const char BCMATTACHDATA(vstr_temps_period)[] = "temps_period=%d";
static const char BCMATTACHDATA(vstr_temps_hysteresis)[] = "temps_hysteresis=%d";
static const char BCMATTACHDATA(vstr_tempoffset)[] = "tempoffset=%d";
static const char BCMATTACHDATA(vstr_tempsense_slope)[] = "tempsense_slope=%d";
static const char BCMATTACHDATA(vstr_temp_corrx)[] = "tempcorrx=%d";
static const char BCMATTACHDATA(vstr_tempsense_option)[] = "tempsense_option=%d";
static const char BCMATTACHDATA(vstr_phycal_tempdelta)[] = "phycal_tempdelta=%d";
static const char BCMATTACHDATA(vstr_tssiposslopeg)[] = "tssiposslope%dg=%d";	/* HNBU_FEM_CFG */
static const char BCMATTACHDATA(vstr_epagaing)[] = "epagain%dg=%d";
static const char BCMATTACHDATA(vstr_pdgaing)[] = "pdgain%dg=%d";
static const char BCMATTACHDATA(vstr_tworangetssi)[] = "tworangetssi%dg=%d";
static const char BCMATTACHDATA(vstr_papdcap)[] = "papdcap%dg=%d";
static const char BCMATTACHDATA(vstr_femctrl)[] = "femctrl=%d";
static const char BCMATTACHDATA(vstr_gainctrlsph)[] = "gainctrlsph=%d";
static const char BCMATTACHDATA(vstr_subband5gver)[] = "subband5gver=%d";	/* HNBU_ACPA_CX */
static const char BCMATTACHDATA(vstr_pa2ga)[] = "pa2ga%d=0x%x,0x%x,0x%x";
static const char BCMATTACHDATA(vstr_maxp5ga)[] = "maxp5ga%d=0x%x,0x%x,0x%x,0x%x";
static const char BCMATTACHDATA(vstr_pa5ga)[] = "pa5ga%d=0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,"
	"0x%x,0x%x,0x%x,0x%x,0x%x,0x%x";
static const char BCMATTACHDATA(vstr_pa2gccka)[] = "pa2gccka%d=0x%x,0x%x,0x%x";
static const char BCMATTACHDATA(vstr_pa5gbw40a)[] = "pa5gbw40a%d=0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,"
	"0x%x,0x%x,0x%x,0x%x,0x%x,0x%x";
static const char BCMATTACHDATA(vstr_pa5gbw80a)[] = "pa5gbw80a%d=0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,"
	"0x%x,0x%x,0x%x,0x%x,0x%x,0x%x";
static const char BCMATTACHDATA(vstr_pa5gbw4080a)[] = "pa5gbw4080a%d=0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,"
	"0x%x,0x%x,0x%x,0x%x,0x%x,0x%x";
static const char BCMATTACHDATA(vstr_rxgainsgelnagaina)[] = "rxgains%dgelnagaina%d=%d";
static const char BCMATTACHDATA(vstr_rxgainsgtrisoa)[] = "rxgains%dgtrisoa%d=%d";
static const char BCMATTACHDATA(vstr_rxgainsgtrelnabypa)[] = "rxgains%dgtrelnabypa%d=%d";
static const char BCMATTACHDATA(vstr_rxgainsgxelnagaina)[] = "rxgains%dg%celnagaina%d=%d";
static const char BCMATTACHDATA(vstr_rxgainsgxtrisoa)[] = "rxgains%dg%ctrisoa%d=%d";
static const char BCMATTACHDATA(vstr_rxgainsgxtrelnabypa)[] = "rxgains%dg%ctrelnabypa%d=%d";
static const char BCMATTACHDATA(vstr_measpower)[] = "measpower=0x%x";	/* HNBU_MEAS_PWR */
static const char BCMATTACHDATA(vstr_measpowerX)[] = "measpower%d=0x%x";
static const char BCMATTACHDATA(vstr_pdoffsetma)[] = "pdoffset%dma%d=0x%x";	/* HNBU_PDOFF */
static const char BCMATTACHDATA(vstr_pdoffset2gma)[] = "pdoffset2g%dma%d=0x%x";	/* HNBU_PDOFF_2G */
static const char BCMATTACHDATA(vstr_pdoffset2gmvalid)[] = "pdoffset2g%dmvalid=0x%x";
static const char BCMATTACHDATA(vstr_rawtempsense)[] = "rawtempsense=0x%x";
/* HNBU_ACPPR_2GPO */
static const char BCMATTACHDATA(vstr_dot11agofdmhrbw202gpo)[] = "dot11agofdmhrbw202gpo=0x%x";
static const char BCMATTACHDATA(vstr_ofdmlrbw202gpo)[] = "ofdmlrbw202gpo=0x%x";
static const char BCMATTACHDATA(vstr_mcsbw805gpo)[] = "mcsbw805g%cpo=0x%x"; /* HNBU_ACPPR_5GPO */
static const char BCMATTACHDATA(vstr_mcsbw1605gpo)[] = "mcsbw1605g%cpo=0x%x";
/* TODO:WL11BE:SROM add bw320 */
static const char BCMATTACHDATA(vstr_mcsbw80p805gpo)[] = "mcsbw80p805g%cpo=0x%x";
static const char BCMATTACHDATA(vstr_mcsbw80p805g1po)[] = "mcsbw80p805g%c1po=0x%x";
static const char BCMATTACHDATA(vstr_mcsbw1605g1po)[] = "mcsbw1605g%c1po=0x%x";
static const char BCMATTACHDATA(vstr_mcsbw805g1po)[] = "mcsbw805g%c1po=0x%x";
static const char BCMATTACHDATA(vstr_mcsbw405g1po)[] = "mcsbw405g%c1po=0x%x";
static const char BCMATTACHDATA(vstr_mcsbw205g1po)[] = "mcsbw205g%c1po=0x%x";
static const char BCMATTACHDATA(vstr_mcslr5gpo)[] = "mcslr5g%cpo=0x%x";
static const char BCMATTACHDATA(vstr_mcslr5g1po)[] = "mcslr5g%c1po=0x%x";
static const char BCMATTACHDATA(vstr_mcslr5g80p80po)[] = "mcslr5g80p80po=0x%x";
/* HNBU_ACPPR_SBPO */
static const char BCMATTACHDATA(vstr_sb20in40rpo)[] = "sb20in40%crpo=0x%x";
/* HNBU_ACPPR_SBPO */
static const char BCMATTACHDATA(vstr_sb20in40and80rpo)[] = "sb20in40and80%crpo=0x%x";
static const char BCMATTACHDATA(vstr_sb20in80and160r5gpo)[] = "sb20in80and160%cr5g%cpo=0x%x";
static const char BCMATTACHDATA(vstr_sb20in80and160r5g1po)[] = "sb20in80and160%cr5g%c1po=0x%x";
static const char BCMATTACHDATA(vstr_sb2040and80in80p80r5gpo)[] =
	"sb2040and80in80p80%cr5g%cpo=0x%x";
static const char BCMATTACHDATA(vstr_sb2040and80in80p80r5g1po)[] =
	"sb2040and80in80p80%cr5g%c1po=0x%x";
static const char BCMATTACHDATA(vstr_sb20in40dot11agofdm2gpo)[] = "sb20in40dot11agofdm2gpo=0x%x";
static const char BCMATTACHDATA(vstr_sb20in80dot11agofdm2gpo)[] = "sb20in80dot11agofdm2gpo=0x%x";
static const char BCMATTACHDATA(vstr_sb20in40ofdmlrbw202gpo)[] = "sb20in40ofdmlrbw202gpo=0x%x";
static const char BCMATTACHDATA(vstr_sb20in80ofdmlrbw202gpo)[] = "sb20in80ofdmlrbw202gpo=0x%x";
static const char BCMATTACHDATA(vstr_sb20in80p80r5gpo)[] = "sb20in80p80%cr5gpo=0x%x";
static const char BCMATTACHDATA(vstr_sb40and80r5gpo)[] = "sb40and80%cr5g%cpo=0x%x";
static const char BCMATTACHDATA(vstr_sb40and80r5g1po)[] = "sb40and80%cr5g%c1po=0x%x";
static const char BCMATTACHDATA(vstr_dot11agduprpo)[] = "dot11agdup%crpo=0x%x";
static const char BCMATTACHDATA(vstr_dot11agduppo)[] = "dot11agduppo=0x%x";
static const char BCMATTACHDATA(vstr_noiselvl2ga)[] = "noiselvl2ga%d=%d";	/* HNBU_NOISELVL */
static const char BCMATTACHDATA(vstr_noiselvl5ga)[] = "noiselvl5ga%d=%d,%d,%d,%d";
/* HNBU_RXGAIN_ERR */
static const char BCMATTACHDATA(vstr_rxgainerr2ga)[] = "rxgainerr2ga%d=0x%x";
static const char BCMATTACHDATA(vstr_rxgainerr5ga)[] = "rxgainerr5ga%d=0x%x,0x%x,0x%x,0x%x";
static const char BCMATTACHDATA(vstr_agbg)[] = "agbg%d=0x%x";	/* HNBU_AGBGA */
static const char BCMATTACHDATA(vstr_aga)[] = "aga%d=0x%x";
static const char BCMATTACHDATA(vstr_txduty_ofdm)[] = "tx_duty_cycle_ofdm_%d_5g=%d";
static const char BCMATTACHDATA(vstr_txduty_thresh)[] = "tx_duty_cycle_thresh_%d_5g=%d";
static const char BCMATTACHDATA(vstr_paparambwver)[] = "paparambwver=%d";

static const char BCMATTACHDATA(vstr_uuid)[] = "uuid=%s";

static const char BCMATTACHDATA(vstr_wowlgpio)[] = "wowl_gpio=%d";
static const char BCMATTACHDATA(vstr_wowlgpiopol)[] = "wowl_gpiopol=%d";

static const char BCMATTACHDATA(rstr_ag0)[] = "ag0";
static const char BCMATTACHDATA(rstr_sromrev)[] = "sromrev";

static const char BCMATTACHDATA(vstr_paparamrpcalvars)[][20] =
	{"rpcal2g=0x%x", "rpcal5gb0=0x%x", "rpcal5gb1=0x%x",
	"rpcal5gb2=0x%x", "rpcal5gb3=0x%x"};

static const char BCMATTACHDATA(vstr_gpdn)[] = "gpdn=0x%x";

/* SROM V13 PA */
static const char BCMATTACHDATA(vstr_sr13pa2ga)[] = "pa2ga%d=0x%x,0x%x,0x%x,0x%x";
static const char BCMATTACHDATA(vstr_maxp5gba)[] = "maxp5gb%da%d=0x%x";
static const char BCMATTACHDATA(vstr_sr13pa5ga)[] = "pa5ga%d=%s";
static const char BCMATTACHDATA(vstr_sr13pa5gbwa)[] = "pa5g%da%d=%s";
static const char BCMATTACHDATA(vstr_pa2g40a)[] = "pa2g40a%d=0x%x,0x%x,0x%x,0x%x";

uint8 patch_pair = 0;

/* For dongle HW, accept partial calibration parameters */
#if defined(BCMDONGLEHOST)
#define BCMDONGLECASE(n) case n:
#else
#define BCMDONGLECASE(n)
#endif

#ifdef BCM_BOOTLOADER
/* The format of the PMUREGS OTP Tuple ->
 * 1 byte -> Lower 5 bits has the address of the register
 *                 Higher 3 bits has the mode of the register like
 *                 PLL, ChipCtrl, RegCtrl, UpDwn or Dependency mask
 * 4 bytes -> Value of the register to be updated.
 */
#define PMUREGS_MODE_MASK	0xE0
#define PMUREGS_MODE_SHIFT	5
#define PMUREGS_ADDR_MASK	0x1F
#define PMUREGS_TPL_SIZE	5

enum {
	PMU_PLLREG_MODE,
	PMU_CCREG_MODE,
	PMU_VOLTREG_MODE,
	PMU_RES_TIME_MODE,
	PMU_RESDEPEND_MODE
};

#define USBREGS_TPL_SIZE	5
enum {
	USB_DEV_CTRL_REG,
	HSIC_PHY_CTRL1_REG,
	HSIC_PHY_CTRL2_REG
};

#define USBRDY_DLY_TYPE	0x8000	/* Bit indicating if the byte is pre or post delay value */
#define USBRDY_DLY_MASK	0x7FFF	/* Bits indicating the amount of delay */
#define USBRDY_MAXOTP_SIZE	5	/* Max size of the OTP parameter */

#endif /* BCM_BOOTLOADER */

static int
BCMATTACHFN(get_max_cis_size)(si_t *sih)
{
	int max_cis_size = CIS_SIZE;
	void *oh;

	if (sih) {
		if (sih->ccrev >= 129)
			max_cis_size = CIS_SIZE_SW_1172B;
		else if (sih->ccrev >= 66)
			max_cis_size = CIS_SIZE_SW_1436B;
	}

	if (sih && (oh = otp_init(sih)) != NULL) {
		max_cis_size -= otp_avsbitslen(oh);
	}
	return max_cis_size;
}

#ifndef BCM_BOOTLOADER
static uint32
BCMATTACHFN(srom_data2value)(uint8 *p, uint8 len)
{
	uint8 pos = 0;
	uint32 value = 0;

	ASSERT(len <= 4);

	while (pos < len) {
		value += (p[pos] << (pos * 8));
		pos++;
	}

	return value;
}
#endif /* BCM_BOOTLOADER */

/**
 * Both SROM and OTP contain variables in 'CIS' format, whereas the rest of the firmware works with
 * 'variable/value' string pairs.
 */
int
BCMATTACHFN(srom_parsecis)(si_t *sih, osl_t *osh, uint8 *pcis[], uint ciscnt, char **vars,
	uint *count)
{
	char eabuf[32];
	char eabuf2[32];
	char *base;
	varbuf_t b;
	uint8 *cis, tup, tlen, sromrev = 1;
	int i, j;
#ifndef BCM_BOOTLOADER
	bool ag_init = FALSE;
#endif
	uint32 w32;
	uint funcid;
	uint cisnum;
	int32 boardnum;
	int err;
	bool standard_cis;
	int max_cis_size;
	bool ccodelock = FALSE;

	ASSERT(count != NULL);

	if (vars == NULL) {
		ASSERT(0);	/* crash debug images for investigation */
		return BCME_BADARG;
	}

	boardnum = -1;

	/* freed in same function */
	base = MALLOC_NOPERSIST(osh, MAXSZ_NVRAM_VARS);
	ASSERT(base != NULL);
	if (!base)
		return -2;

	varbuf_init(&b, base, MAXSZ_NVRAM_VARS);
	bzero(base, MAXSZ_NVRAM_VARS);
	/* Append from vars if there's already something inside */
	if (*vars && **vars && (*count >= 3)) {
		/* back off \0 at the end, leaving only one \0 for the last param */
		while (((*vars)[(*count)-1] == '\0') && ((*vars)[(*count)-2] == '\0'))
			(*count)--;

		bcopy(*vars, base, *count);
		b.buf += *count;
	}

	eabuf[0] = '\0';
	eabuf2[0] = '\0';
	max_cis_size = get_max_cis_size(sih);
	for (cisnum = 0; cisnum < ciscnt; cisnum++) {
		cis = *pcis++;
		i = 0;
		funcid = 0;
		standard_cis = TRUE;
		do {
			if (standard_cis) {
				tup = cis[i++];
				if (tup == CISTPL_NULL || tup == CISTPL_END)
					tlen = 0;
				else
					tlen = cis[i++];
			} else {
				if (cis[i] == CISTPL_NULL || cis[i] == CISTPL_END) {
					tlen = 0;
					tup = cis[i];
				} else {
					tlen = cis[i];
					tup = CISTPL_BRCM_HNBU;
				}

				++i;
			}

			if ((i + tlen) >= max_cis_size)
				break;

			switch (tup) {
			case CISTPL_VERS_1:
				/* assume the strings are good if the version field checks out */
				if (((cis[i + 1] << 8) + cis[i]) >= 0x0008) {
					varbuf_append(&b, vstr_manf, &cis[i + 2]);
					varbuf_append(&b, vstr_productname,
					              &cis[i + 3 + strlen((char *)&cis[i + 2])]);
				}
				break;

			case CISTPL_MANFID:
				varbuf_append(&b, vstr_manfid, (cis[i + 1] << 8) + cis[i]);
				varbuf_append(&b, vstr_prodid, (cis[i + 3] << 8) + cis[i + 2]);
				break;

			case CISTPL_FUNCID:
				funcid = cis[i];
				break;

			case CISTPL_FUNCE:
				switch (funcid) {
				case CISTPL_FID_SDIO:
					funcid = 0;
					break;
				default:
					/* set macaddr if HNBU_MACADDR not seen yet */
					if (eabuf[0] == '\0' && cis[i] == LAN_NID &&
						!(ETHER_ISNULLADDR(&cis[i + 2])) &&
						!(ETHER_ISMULTI(&cis[i + 2]))) {
						ASSERT(cis[i + 1] == ETHER_ADDR_LEN);
						bcm_ether_ntoa((struct ether_addr *)&cis[i + 2],
						               eabuf);

						/* set boardnum if HNBU_BOARDNUM not seen yet */
						if (boardnum == -1)
							boardnum = (cis[i + 6] << 8) + cis[i + 7];
					}
					break;
				}
				break;

			case CISTPL_CFTABLE:
				varbuf_append(&b, vstr_regwindowsz, (cis[i + 7] << 8) | cis[i + 6]);
				break;

			case CISTPL_BRCM_HNBU:
				switch (cis[i]) {
				case HNBU_SROMREV:
					sromrev = cis[i + 1];
					varbuf_append(&b, vstr_sromrev, sromrev);
					break;

				case HNBU_XTALFREQ:
					varbuf_append(&b, vstr_xtalfreq,
					              (cis[i + 4] << 24) |
					              (cis[i + 3] << 16) |
					              (cis[i + 2] << 8) |
					              cis[i + 1]);
					break;

				case HNBU_CHIPID:
					varbuf_append(&b, vstr_vendid, (cis[i + 2] << 8) +
					              cis[i + 1]);
					varbuf_append(&b, vstr_devid, (cis[i + 4] << 8) +
					              cis[i + 3]);
					if (tlen >= 7) {
						varbuf_append(&b, vstr_chiprev,
						              (cis[i + 6] << 8) + cis[i + 5]);
					}

					if (tlen >= 9) {
						varbuf_append(&b, vstr_subvendid,
						              (cis[i + 8] << 8) + cis[i + 7]);
					}

					if (tlen >= 11) {
						varbuf_append(&b, vstr_subdevid,
						              (cis[i + 10] << 8) + cis[i + 9]);
						/* subdevid doubles for boardtype */
						varbuf_append(&b, vstr_boardtype,
						              (cis[i + 10] << 8) + cis[i + 9]);
					}

					break;

				case HNBU_BOARDNUM:
					boardnum = (cis[i + 2] << 8) + cis[i + 1];
					break;

				case HNBU_PATCH:
					{
						char vstr_paddr[16];
						char vstr_pdata[16];

						/* retrieve the patch pairs
						 * from tlen/6; where 6 is
						 * sizeof(patch addr(2)) +
						 * sizeof(patch data(4)).
						 */
						patch_pair = tlen/6;

						for (j = 0; j < patch_pair; j++) {
							snprintf(vstr_paddr, sizeof(vstr_paddr),
								"pa%d=0x%%x", j);
							snprintf(vstr_pdata, sizeof(vstr_pdata),
								"pd%d=0x%%x", j);

							varbuf_append(&b, vstr_paddr,
								(cis[i + (j*6) + 2] << 8) |
								cis[i + (j*6) + 1]);

							varbuf_append(&b, vstr_pdata,
								(cis[i + (j*6) + 6] << 24) |
								(cis[i + (j*6) + 5] << 16) |
								(cis[i + (j*6) + 4] << 8) |
								cis[i + (j*6) + 3]);
						}
					}
					break;

				case HNBU_BOARDREV:
					if (tlen == 2)
						varbuf_append(&b, vstr_boardrev, cis[i + 1]);
					else
						varbuf_append(&b, vstr_boardrev,
							(cis[i + 2] << 8) + cis[i + 1]);
					break;

				case HNBU_BOARDFLAGS:
					w32 = (cis[i + 2] << 8) + cis[i + 1];
					if (tlen >= 5)
						w32 |= ((cis[i + 4] << 24) + (cis[i + 3] << 16));
					varbuf_append(&b, vstr_boardflags, w32);

					if (tlen >= 7) {
						w32 = (cis[i + 6] << 8) + cis[i + 5];
						if (tlen >= 9)
							w32 |= ((cis[i + 8] << 24) +
								(cis[i + 7] << 16));
						varbuf_append(&b, vstr_boardflags2, w32);
					}

					if (tlen >= 11) {
						w32 = (cis[i + 10] << 8) + cis[i + 9];
						if (tlen >= 13)
							w32 |= ((cis[i + 12] << 24) +
								(cis[i + 11] << 16));
						varbuf_append(&b, vstr_boardflags3, w32);
					}

					if (tlen >= 15) {
						w32 = (cis[i + 14] << 8) + cis[i + 13];
						if (tlen >= 17)
							w32 |= ((cis[i + 16] << 24) +
								(cis[i + 15] << 16));
						varbuf_append(&b, vstr_boardflags4, w32);
					}

					if (tlen >= 19) {
						w32 = (cis[i + 18] << 8) + cis[i + 17];
						if (tlen >= 21)
							w32 |= ((cis[i + 20] << 24) +
								(cis[i + 19] << 16));
						varbuf_append(&b, vstr_boardflags5, w32);
					}
					break;

				case HNBU_USBFS:
					varbuf_append(&b, vstr_usbfs, cis[i + 1]);
					break;

				case HNBU_BOARDTYPE:
					varbuf_append(&b, vstr_boardtype,
					              (cis[i + 2] << 8) + cis[i + 1]);
					break;

				case HNBU_HNBUCIS:
					/*
					 * what follows is a nonstandard HNBU CIS
					 * that lacks CISTPL_BRCM_HNBU tags
					 *
					 * skip 0xff (end of standard CIS)
					 * after this tuple
					 */
					tlen++;
					standard_cis = FALSE;
					break;

				case HNBU_USBEPNUM:
					varbuf_append(&b, vstr_usbepnum,
						(cis[i + 2] << 8) | cis[i + 1]);
					break;

				case HNBU_PATCH_AUTOINC: {
						char vstr_paddr[16];
						char vstr_pdata[16];
						uint32 addr_inc;
						uint8 pcnt;

						addr_inc = (cis[i + 4] << 24) |
							(cis[i + 3] << 16) |
							(cis[i + 2] << 8) |
							(cis[i + 1]);

						pcnt = (tlen - 5)/4;
						for (j = 0; j < pcnt; j++) {
							snprintf(vstr_paddr, sizeof(vstr_paddr),
								"pa%d=0x%%x", j + patch_pair);
							snprintf(vstr_pdata, sizeof(vstr_pdata),
								"pd%d=0x%%x", j + patch_pair);

							varbuf_append(&b, vstr_paddr, addr_inc);
							varbuf_append(&b, vstr_pdata,
								(cis[i + (j*4) + 8] << 24) |
								(cis[i + (j*4) + 7] << 16) |
								(cis[i + (j*4) + 6] << 8) |
								cis[i + (j*4) + 5]);
							addr_inc += 4;
						}

						patch_pair += pcnt;
					}
					break;
				case HNBU_PATCH2:
					{
						char vstr_paddr[16];
						char vstr_pdata[16];

						/* retrieve the patch pairs
						 * from tlen/8; where 8 is
						 * sizeof(patch addr(4)) +
						 * sizeof(patch data(4)).
						 */
						patch_pair = tlen/8;

						for (j = 0; j < patch_pair; j++) {
							snprintf(vstr_paddr, sizeof(vstr_paddr),
								"pa%d=0x%%x", j);
							snprintf(vstr_pdata, sizeof(vstr_pdata),
								"pd%d=0x%%x", j);

							varbuf_append(&b, vstr_paddr,
								(cis[i + (j*8) + 4] << 24) |
								(cis[i + (j*8) + 3] << 16) |
								(cis[i + (j*8) + 2] << 8) |
								cis[i + (j*8) + 1]);

							varbuf_append(&b, vstr_pdata,
								(cis[i + (j*8) + 8] << 24) |
								(cis[i + (j*8) + 7] << 16) |
								(cis[i + (j*8) + 6] << 8) |
								cis[i + (j*8) + 5]);
						}
					}
					break;
				case HNBU_PATCH_AUTOINC8: {
						char vstr_paddr[16];
						char vstr_pdatah[16];
						char vstr_pdatal[16];
						uint32 addr_inc;
						uint8 pcnt;

						addr_inc = (cis[i + 4] << 24) |
							(cis[i + 3] << 16) |
							(cis[i + 2] << 8) |
							(cis[i + 1]);

						pcnt = (tlen - 5)/8;
						for (j = 0; j < pcnt; j++) {
							snprintf(vstr_paddr, sizeof(vstr_paddr),
								"pa%d=0x%%x", j + patch_pair);
							snprintf(vstr_pdatah, sizeof(vstr_pdatah),
								"pdh%d=0x%%x", j + patch_pair);
							snprintf(vstr_pdatal, sizeof(vstr_pdatal),
								"pdl%d=0x%%x", j + patch_pair);

							varbuf_append(&b, vstr_paddr, addr_inc);
							varbuf_append(&b, vstr_pdatal,
								(cis[i + (j*8) + 8] << 24) |
								(cis[i + (j*8) + 7] << 16) |
								(cis[i + (j*8) + 6] << 8) |
								cis[i + (j*8) + 5]);
							varbuf_append(&b, vstr_pdatah,
								(cis[i + (j*8) + 12] << 24) |
								(cis[i + (j*8) + 11] << 16) |
								(cis[i + (j*8) + 10] << 8) |
								cis[i + (j*8) + 9]);
							addr_inc += 8;
						}

						patch_pair += pcnt;
					}
					break;

				case HNBU_PATCH8:
					{
						char vstr_paddr[16];
						char vstr_pdatah[16];
						char vstr_pdatal[16];

						/* retrieve the patch pairs
						 * from tlen/8; where 8 is
						 * sizeof(patch addr(4)) +
						 * sizeof(patch data(4)).
						 */
						patch_pair = tlen/12;

						for (j = 0; j < patch_pair; j++) {
							snprintf(vstr_paddr, sizeof(vstr_paddr),
								"pa%d=0x%%x", j);
							snprintf(vstr_pdatah, sizeof(vstr_pdatah),
								"pdh%d=0x%%x", j);
							snprintf(vstr_pdatal, sizeof(vstr_pdatal),
								"pdl%d=0x%%x", j);

							varbuf_append(&b, vstr_paddr,
								(cis[i + (j*12) + 4] << 24) |
								(cis[i + (j*12) + 3] << 16) |
								(cis[i + (j*12) + 2] << 8) |
								cis[i + (j*12) + 1]);

							varbuf_append(&b, vstr_pdatal,
								(cis[i + (j*12) + 8] << 24) |
								(cis[i + (j*12) + 7] << 16) |
								(cis[i + (j*12) + 6] << 8) |
								cis[i + (j*12) + 5]);

							varbuf_append(&b, vstr_pdatah,
								(cis[i + (j*12) + 12] << 24) |
								(cis[i + (j*12) + 11] << 16) |
								(cis[i + (j*12) + 10] << 8) |
								cis[i + (j*12) + 9]);
						}
					}
					break;
				case HNBU_USBFLAGS:
					varbuf_append(&b, vstr_usbflags,
					              (cis[i + 4] << 24) |
					              (cis[i + 3] << 16) |
					              (cis[i + 2] << 8) |
					              cis[i + 1]);
					break;
#ifdef BCM_BOOTLOADER
				case HNBU_MDIOEX_REGLIST:
				case HNBU_MDIO_REGLIST: {
					/* Format: addr (8 bits) | val (16 bits) */
					const uint8 msize = 3;
					char mdiostr[24];
					const char *mdiodesc;
					uint8 *st;

					mdiodesc = (cis[i] == HNBU_MDIO_REGLIST) ?
						vstr_mdio : vstr_mdioex;

					ASSERT(((tlen - 1) % msize) == 0);

					st = &cis[i + 1]; /* start of reg list */
					for (j = 0; j < (tlen - 1); j += msize, st += msize) {
						snprintf(mdiostr, sizeof(mdiostr),
							mdiodesc, st[0]);
						varbuf_append(&b, mdiostr, (st[2] << 8) | st[1]);
					}
				}
					break;
				case HNBU_BRMIN:
					varbuf_append(&b, vstr_brmin,
					              (cis[i + 4] << 24) |
					              (cis[i + 3] << 16) |
					              (cis[i + 2] << 8) |
					              cis[i + 1]);
					break;

				case HNBU_BRMAX:
					varbuf_append(&b, vstr_brmax,
					              (cis[i + 4] << 24) |
					              (cis[i + 3] << 16) |
					              (cis[i + 2] << 8) |
					              cis[i + 1]);
					break;
#endif /* BCM_BOOTLOADER */

				case HNBU_RDLID:
					varbuf_append(&b, vstr_rdlid,
					              (cis[i + 2] << 8) | cis[i + 1]);
					break;

				case HNBU_GCI_CCR:
					{
						/* format:
						* |0x80 | 	<== brcm
						* |len|		<== variable, multiple of 5
						* |tup|		<== tupletype
						* |ccreg_ix0|<== ix of ccreg [1byte]
						* |ccreg_val0|<= corr value [4bytes]
						*	---
						* Multiple registers are possible. for eg: we
						*	can specify reg_ix3val3 and reg_ix5val5, etc
						*/
						char vstr_gci_ccreg_entry[16];
						int num_entries = 0;

						/* retrieve the index-value pairs
						 * from tlen/5; where 5 is
						 * sizeof(ccreg_ix(1)) +
						 * sizeof(ccreg_val(4)).
						 */
						num_entries = tlen/5;

						for (j = 0; j < num_entries; j++) {
							snprintf(vstr_gci_ccreg_entry,
								sizeof(vstr_gci_ccreg_entry),
								"gcr%d=0x%%x", cis[i + (j*5) + 1]);

							varbuf_append(&b, vstr_gci_ccreg_entry,
								(cis[i + (j*5) + 5] << 24) |
								(cis[i + (j*5) + 4] << 16) |
								(cis[i + (j*5) + 3] << 8) |
								cis[i + (j*5) + 2]);
						}
					}
					break;

#ifdef BCM_BOOTLOADER
				case HNBU_RDLRNDIS:
					break;

				case HNBU_RDLRWU:
					varbuf_append(&b, vstr_rdlrwu, cis[i + 1]);
					break;

				case HNBU_RDLSN:
					if (tlen >= 5)
						varbuf_append(&b, vstr_rdlsn,
						              (cis[i + 4] << 24) |
						              (cis[i + 3] << 16) |
						              (cis[i + 2] << 8) |
						              cis[i + 1]);
					else
						varbuf_append(&b, vstr_rdlsn,
						              (cis[i + 2] << 8) |
						              cis[i + 1]);
					break;

				case HNBU_PMUREGS:
					{
						uint8 offset = 1, mode_addr, mode, addr;
						const char *fmt;

						do {
							mode_addr = cis[i+offset];

							mode = (mode_addr & PMUREGS_MODE_MASK)
								>> PMUREGS_MODE_SHIFT;
							addr = mode_addr & PMUREGS_ADDR_MASK;

							switch (mode) {
								case PMU_PLLREG_MODE:
									fmt = vstr_pllreg;
									break;
								case PMU_CCREG_MODE:
									fmt = vstr_ccreg;
									break;
								case PMU_VOLTREG_MODE:
									fmt = vstr_regctrl;
									break;
								case PMU_RES_TIME_MODE:
									fmt = vstr_time;
									break;
								case PMU_RESDEPEND_MODE:
									fmt = vstr_depreg;
									break;
								default:
									fmt = NULL;
									break;
							}

							if (fmt != NULL) {
								varbuf_append(&b, fmt, addr,
								(cis[i + offset + 4] << 24) |
								(cis[i + offset + 3] << 16) |
								(cis[i + offset + 2] << 8) |
								cis[i + offset + 1]);
							}

							offset += PMUREGS_TPL_SIZE;
						} while (offset < tlen);
					}
					break;

				case HNBU_USBREGS:
					{
						uint8 offset = 1, usb_reg;
						const char *fmt;

						do {
							usb_reg = cis[i+offset];

							switch (usb_reg) {
								case USB_DEV_CTRL_REG:
									fmt = vstr_usbdevctrl;
									break;
								case HSIC_PHY_CTRL1_REG:
									fmt = vstr_hsicphyctrl1;
									break;
								case HSIC_PHY_CTRL2_REG:
									fmt = vstr_hsicphyctrl2;
									break;
								default:
									fmt = NULL;
									break;
							}

							if (fmt != NULL) {
								varbuf_append(&b, fmt,
								(cis[i + offset + 4] << 24) |
								(cis[i + offset + 3] << 16) |
								(cis[i + offset + 2] << 8) |
								cis[i + offset + 1]);
							}

							offset += USBREGS_TPL_SIZE;
						} while (offset < tlen);
					}
					break;

				case HNBU_USBRDY:
					/* The first byte of this tuple indicate if the host
					 * needs to be informed about the readiness of
					 * the HSIC/USB for enumeration on which GPIO should
					 * the device assert this event.
					 */
					varbuf_append(&b, vstr_usbrdy, cis[i + 1]);

					/* The following fields in this OTP are optional.
					 * The remaining bytes will indicate the delay required
					 * before and/or after the ch_init(). The delay is defined
					 * using 16-bits of this the MSB(bit15 of 15:0) will be
					 * used indicate if the parameter is for Pre or Post delay.
					 */
					for (j = 2; j < USBRDY_MAXOTP_SIZE && j < tlen;
						j += 2) {
						uint16 usb_delay;

						usb_delay = cis[i + j] | (cis[i + j + 1] << 8);

						/* The bit-15 of the delay field will indicate the
						 * type of delay (pre or post).
						 */
						if (usb_delay & USBRDY_DLY_TYPE) {
							varbuf_append(&b, vstr_usbpostdly,
							(usb_delay & USBRDY_DLY_MASK));
						} else {
							varbuf_append(&b, vstr_usbpredly,
							(usb_delay & USBRDY_DLY_MASK));
						}
					}
					break;

				case HNBU_BLDR_TIMEOUT:
					/* The Delay after USBConnect for timeout till dongle
					 * receives get_descriptor request.
					 */
					varbuf_append(&b, vstr_bldr_reset_timeout,
						(cis[i + 1] | (cis[i + 2] << 8)));
					break;

				case HNBU_MUXENAB:
					varbuf_append(&b, vstr_muxenab, cis[i + 1]);
					break;

				case HNBU_PUBKEY:
					/* The public key is in binary format in OTP,
					 * convert to string format before appending
					 * buffer string.
					 *  public key(12 bytes) + crc (1byte) = 129
					 */
					{
						unsigned char a[300];
						int k;

						for (k = 1, j = 0; k < 129; k++)
							j += snprintf((char *)(a + j),
								sizeof(a) - j,
								"%02x", cis[i + k]);

						a[256] = 0;

						varbuf_append(&b, vstr_pubkey, a);
					}
					break;
#else
				case HNBU_AA:
					varbuf_append(&b, vstr_aa2g, cis[i + 1]);
					if (tlen >= 3)
						varbuf_append(&b, vstr_aa5g, cis[i + 2]);
					break;

				case HNBU_AG:
					varbuf_append(&b, vstr_ag, 0, cis[i + 1]);
					if (tlen >= 3)
						varbuf_append(&b, vstr_ag, 1, cis[i + 2]);
					if (tlen >= 4)
						varbuf_append(&b, vstr_ag, 2, cis[i + 3]);
					if (tlen >= 5)
						varbuf_append(&b, vstr_ag, 3, cis[i + 4]);
					ag_init = TRUE;
					break;

				case HNBU_ANT5G:
					varbuf_append(&b, vstr_aa5g, cis[i + 1]);
					varbuf_append(&b, vstr_ag, 1, cis[i + 2]);
					break;

				case HNBU_CC:
					ASSERT(sromrev == 1);
					varbuf_append(&b, vstr_cc, cis[i + 1]);
					break;

				case HNBU_PAPARMS:
				{
					uint8 pa0_lo_offset = 0;
					switch (tlen) {
					case 2:
						ASSERT(sromrev == 1);
						varbuf_append(&b, vstr_pa0maxpwr, cis[i + 1]);
						break;
					/* case 16:
						ASSERT(sromrev >= 11);
						for (j = 0; j < 3; j++) {
						varbuf_append(&b, vstr_pa0b_lo[j],
								(cis[i + (j * 2) + 11] << 8) +
								cis[i + (j * 2) + 10]);
						}
						 FALLTHROUGH
					*/
					case 10:
					case 16:
						ASSERT(sromrev >= 2);
						varbuf_append(&b, vstr_opo, cis[i + 9]);
						if (tlen >= 13 && pa0_lo_offset == 0)
							pa0_lo_offset = 9;
						/* FALLTHROUGH */
					case 9:
					case 15:
						varbuf_append(&b, vstr_pa0maxpwr, cis[i + 8]);
						if (tlen >= 13 && pa0_lo_offset == 0)
							pa0_lo_offset = 8;
						/* FALLTHROUGH */
					BCMDONGLECASE(8)
					BCMDONGLECASE(14)
						varbuf_append(&b, vstr_pa0itssit, cis[i + 7]);
						varbuf_append(&b, vstr_maxp2ga, 0, cis[i + 7]);
						if (tlen >= 13 && pa0_lo_offset == 0)
							pa0_lo_offset = 7;
						/* FALLTHROUGH */
					BCMDONGLECASE(7)
					BCMDONGLECASE(13)
					        for (j = 0; j < 3; j++) {
							varbuf_append(&b, vstr_pa0b[j],
							              (cis[i + (j * 2) + 2] << 8) +
							              cis[i + (j * 2) + 1]);
						}
						if (tlen >= 13 && pa0_lo_offset == 0)
							pa0_lo_offset = 6;

						if (tlen >= 13 && pa0_lo_offset != 0) {
							for (j = 0; j < 3; j++) {
								varbuf_append(&b, vstr_pa0b_lo[j],
								 (cis[pa0_lo_offset+i+(j*2)+2]<<8)+
								 cis[pa0_lo_offset+i+(j*2)+1]);
							}
						}
						break;
					default:
						ASSERT((tlen == 2) || (tlen == 9) || (tlen == 10) ||
							(tlen == 15) || (tlen == 16));
						break;
					}
					break;
				}

				case HNBU_PAPARMS5G:
					ASSERT((sromrev == 2) || (sromrev == 3));
					switch (tlen) {
					case 23:
						varbuf_append(&b, vstr_pa1himaxpwr, cis[i + 22]);
						varbuf_append(&b, vstr_pa1lomaxpwr, cis[i + 21]);
						varbuf_append(&b, vstr_pa1maxpwr, cis[i + 20]);
						/* FALLTHROUGH */
					case 20:
						varbuf_append(&b, vstr_pa1itssit, cis[i + 19]);
						/* FALLTHROUGH */
					case 19:
						for (j = 0; j < 3; j++) {
							varbuf_append(&b, vstr_pa1b[j],
							              (cis[i + (j * 2) + 2] << 8) +
							              cis[i + (j * 2) + 1]);
						}

						for (j = 3; j < 6; j++) {
							varbuf_append(&b, vstr_pa1lob[j - 3],
							              (cis[i + (j * 2) + 2] << 8) +
							              cis[i + (j * 2) + 1]);
						}

						for (j = 6; j < 9; j++) {
							varbuf_append(&b, vstr_pa1hib[j - 6],
							              (cis[i + (j * 2) + 2] << 8) +
							              cis[i + (j * 2) + 1]);
						}

						break;
					default:
						ASSERT((tlen == 19) ||
						       (tlen == 20) || (tlen == 23));
						break;
					}
					break;

				case HNBU_OEM:
					ASSERT(sromrev == 1);
					varbuf_append(&b, vstr_oem,
					              cis[i + 1], cis[i + 2],
					              cis[i + 3], cis[i + 4],
					              cis[i + 5], cis[i + 6],
					              cis[i + 7], cis[i + 8]);
					break;

				case HNBU_LEDS:
					for (j = 1; j <= 4; j++) {
						if (cis[i + j] != 0xff) {
							varbuf_append(&b, vstr_ledbh, j-1,
							cis[i + j]);
						}
					}

					if (tlen < 13) break;

					for (j = 5; j <= 12; j++) {
						if (cis[i + j] != 0xff) {
							varbuf_append(&b, vstr_ledbh, j-1,
							cis[i + j]);
						}
					}

					if (tlen < 17) break;

					for (j = 13; j <= 16; j++) {
						if (cis[i + j] != 0xff) {
							varbuf_append(&b, vstr_ledbh, j-1,
							cis[i + j]);
						}
					}

					break;

				case HNBU_CCODE:
					ASSERT(sromrev > 1);
					if ((cis[i + 1] == 0) || (cis[i + 2] == 0)) {
						varbuf_append(&b, vstr_noccode);
						ccodelock = FALSE;
					} else {
						varbuf_append(&b, vstr_ccode,
						              cis[i + 1], cis[i + 2]);
						ccodelock = TRUE;
					}

					varbuf_append(&b, vstr_cctl, cis[i + 3]);
					break;

				case HNBU_CCKPO:
					ASSERT(sromrev > 2);
					varbuf_append(&b, vstr_cckpo,
					              (cis[i + 2] << 8) | cis[i + 1]);
					break;

				case HNBU_OFDMPO:
					ASSERT(sromrev > 2);
					varbuf_append(&b, vstr_ofdmpo,
					              (cis[i + 4] << 24) |
					              (cis[i + 3] << 16) |
					              (cis[i + 2] << 8) |
					              cis[i + 1]);
					break;

				case HNBU_WPS:
					varbuf_append(&b, vstr_wpsgpio, cis[i + 1]);
					if (tlen >= 3)
						varbuf_append(&b, vstr_wpsled, cis[i + 2]);
					break;

				case HNBU_RSSISMBXA2G:
					ASSERT(sromrev == 3);
					varbuf_append(&b, vstr_rssismf2g, cis[i + 1] & 0xf);
					varbuf_append(&b, vstr_rssismc2g, (cis[i + 1] >> 4) & 0xf);
					varbuf_append(&b, vstr_rssisav2g, cis[i + 2] & 0x7);
					varbuf_append(&b, vstr_bxa2g, (cis[i + 2] >> 3) & 0x3);
					break;

				case HNBU_RSSISMBXA5G:
					ASSERT(sromrev == 3);
					varbuf_append(&b, vstr_rssismf5g, cis[i + 1] & 0xf);
					varbuf_append(&b, vstr_rssismc5g, (cis[i + 1] >> 4) & 0xf);
					varbuf_append(&b, vstr_rssisav5g, cis[i + 2] & 0x7);
					varbuf_append(&b, vstr_bxa5g, (cis[i + 2] >> 3) & 0x3);
					break;

				case HNBU_TRI2G:
					ASSERT(sromrev == 3);
					varbuf_append(&b, vstr_tri2g, cis[i + 1]);
					break;

				case HNBU_TRI5G:
					ASSERT(sromrev == 3);
					varbuf_append(&b, vstr_tri5gl, cis[i + 1]);
					varbuf_append(&b, vstr_tri5g, cis[i + 2]);
					varbuf_append(&b, vstr_tri5gh, cis[i + 3]);
					break;

				case HNBU_RXPO2G:
					ASSERT(sromrev == 3);
					varbuf_append(&b, vstr_rxpo2g, cis[i + 1]);
					break;

				case HNBU_RXPO5G:
					ASSERT(sromrev == 3);
					varbuf_append(&b, vstr_rxpo5g, cis[i + 1]);
					break;

				case HNBU_MACADDR:
					if (!(ETHER_ISNULLADDR(&cis[i+1])) &&
					    !(ETHER_ISMULTI(&cis[i+1]))) {
						bcm_ether_ntoa((struct ether_addr *)&cis[i + 1],
						               eabuf);

						/* set boardnum if HNBU_BOARDNUM not seen yet */
						if (boardnum == -1)
							boardnum = (cis[i + 5] << 8) + cis[i + 6];
					}
					break;

				case HNBU_LEDDC:
					/* CIS leddc only has 16bits, convert it to 32bits */
					w32 = ((cis[i + 2] << 24) | /* oncount */
					       (cis[i + 1] << 8)); /* offcount */
					varbuf_append(&b, vstr_leddc, w32);
					break;

				case HNBU_CHAINSWITCH:
					varbuf_append(&b, vstr_txchain, cis[i + 1]);
					varbuf_append(&b, vstr_rxchain, cis[i + 2]);
					varbuf_append(&b, vstr_antswitch,
					      (cis[i + 4] << 8) + cis[i + 3]);
					break;

				case HNBU_ELNA2G:
					varbuf_append(&b, vstr_elna2g, cis[i + 1]);
					break;

				case HNBU_ELNA5G:
					varbuf_append(&b, vstr_elna5g, cis[i + 1]);
					break;

				case HNBU_REGREV:
					varbuf_append(&b, vstr_regrev,
						srom_data2value(&cis[i + 1], tlen - 1));
					break;

				case HNBU_FEM: {
					uint16 fem = (cis[i + 2] << 8) + cis[i + 1];
					varbuf_append(&b, vstr_antswctl2g, (fem &
						SROM8_FEM_ANTSWLUT_MASK) >>
						SROM8_FEM_ANTSWLUT_SHIFT);
					varbuf_append(&b, vstr_triso2g, (fem &
						SROM8_FEM_TR_ISO_MASK) >>
						SROM8_FEM_TR_ISO_SHIFT);
					varbuf_append(&b, vstr_pdetrange2g, (fem &
						SROM8_FEM_PDET_RANGE_MASK) >>
						SROM8_FEM_PDET_RANGE_SHIFT);
					varbuf_append(&b, vstr_extpagain2g, (fem &
						SROM8_FEM_EXTPA_GAIN_MASK) >>
						SROM8_FEM_EXTPA_GAIN_SHIFT);
					varbuf_append(&b, vstr_tssipos2g, (fem &
						SROM8_FEM_TSSIPOS_MASK) >>
						SROM8_FEM_TSSIPOS_SHIFT);
					if (tlen < 5) break;

					fem = (cis[i + 4] << 8) + cis[i + 3];
					varbuf_append(&b, vstr_antswctl5g, (fem &
						SROM8_FEM_ANTSWLUT_MASK) >>
						SROM8_FEM_ANTSWLUT_SHIFT);
					varbuf_append(&b, vstr_triso5g, (fem &
						SROM8_FEM_TR_ISO_MASK) >>
						SROM8_FEM_TR_ISO_SHIFT);
					varbuf_append(&b, vstr_pdetrange5g, (fem &
						SROM8_FEM_PDET_RANGE_MASK) >>
						SROM8_FEM_PDET_RANGE_SHIFT);
					varbuf_append(&b, vstr_extpagain5g, (fem &
						SROM8_FEM_EXTPA_GAIN_MASK) >>
						SROM8_FEM_EXTPA_GAIN_SHIFT);
					varbuf_append(&b, vstr_tssipos5g, (fem &
						SROM8_FEM_TSSIPOS_MASK) >>
						SROM8_FEM_TSSIPOS_SHIFT);
					break;
					}

				case HNBU_PAPARMS_C0:
					varbuf_append(&b, vstr_maxp2ga, 0, cis[i + 1]);
					varbuf_append(&b, vstr_itt2ga0, cis[i + 2]);
					varbuf_append(&b, vstr_pa, 2, 0, 0,
						(cis[i + 4] << 8) + cis[i + 3]);
					varbuf_append(&b, vstr_pa, 2, 1, 0,
						(cis[i + 6] << 8) + cis[i + 5]);
					varbuf_append(&b, vstr_pa, 2, 2, 0,
						(cis[i + 8] << 8) + cis[i + 7]);
					if (tlen < 31) break;

					varbuf_append(&b, vstr_maxp5ga0, cis[i + 9]);
					varbuf_append(&b, vstr_itt5ga0, cis[i + 10]);
					varbuf_append(&b, vstr_maxp5gha0, cis[i + 11]);
					varbuf_append(&b, vstr_maxp5gla0, cis[i + 12]);
					varbuf_append(&b, vstr_pa, 5, 0, 0,
						(cis[i + 14] << 8) + cis[i + 13]);
					varbuf_append(&b, vstr_pa, 5, 1, 0,
						(cis[i + 16] << 8) + cis[i + 15]);
					varbuf_append(&b, vstr_pa, 5, 2, 0,
						(cis[i + 18] << 8) + cis[i + 17]);
					varbuf_append(&b, vstr_pahl, 5, 'l', 0, 0,
						(cis[i + 20] << 8) + cis[i + 19]);
					varbuf_append(&b, vstr_pahl, 5, 'l', 1, 0,
						(cis[i + 22] << 8) + cis[i + 21]);
					varbuf_append(&b, vstr_pahl, 5, 'l', 2, 0,
						(cis[i + 24] << 8) + cis[i + 23]);
					varbuf_append(&b, vstr_pahl, 5, 'h', 0, 0,
						(cis[i + 26] << 8) + cis[i + 25]);
					varbuf_append(&b, vstr_pahl, 5, 'h', 1, 0,
						(cis[i + 28] << 8) + cis[i + 27]);
					varbuf_append(&b, vstr_pahl, 5, 'h', 2, 0,
						(cis[i + 30] << 8) + cis[i + 29]);
					break;

				case HNBU_PAPARMS_C1:
					varbuf_append(&b, vstr_maxp2ga, 1, cis[i + 1]);
					varbuf_append(&b, vstr_itt2ga1, cis[i + 2]);
					varbuf_append(&b, vstr_pa, 2, 0, 1,
						(cis[i + 4] << 8) + cis[i + 3]);
					varbuf_append(&b, vstr_pa, 2, 1, 1,
						(cis[i + 6] << 8) + cis[i + 5]);
					varbuf_append(&b, vstr_pa, 2, 2, 1,
						(cis[i + 8] << 8) + cis[i + 7]);
					if (tlen < 31) break;

					varbuf_append(&b, vstr_maxp5ga1, cis[i + 9]);
					varbuf_append(&b, vstr_itt5ga1, cis[i + 10]);
					varbuf_append(&b, vstr_maxp5gha1, cis[i + 11]);
					varbuf_append(&b, vstr_maxp5gla1, cis[i + 12]);
					varbuf_append(&b, vstr_pa, 5, 0, 1,
						(cis[i + 14] << 8) + cis[i + 13]);
					varbuf_append(&b, vstr_pa, 5, 1, 1,
						(cis[i + 16] << 8) + cis[i + 15]);
					varbuf_append(&b, vstr_pa, 5, 2, 1,
						(cis[i + 18] << 8) + cis[i + 17]);
					varbuf_append(&b, vstr_pahl, 5, 'l', 0, 1,
						(cis[i + 20] << 8) + cis[i + 19]);
					varbuf_append(&b, vstr_pahl, 5, 'l', 1, 1,
						(cis[i + 22] << 8) + cis[i + 21]);
					varbuf_append(&b, vstr_pahl, 5, 'l', 2, 1,
						(cis[i + 24] << 8) + cis[i + 23]);
					varbuf_append(&b, vstr_pahl, 5, 'h', 0, 1,
						(cis[i + 26] << 8) + cis[i + 25]);
					varbuf_append(&b, vstr_pahl, 5, 'h', 1, 1,
						(cis[i + 28] << 8) + cis[i + 27]);
					varbuf_append(&b, vstr_pahl, 5, 'h', 2, 1,
						(cis[i + 30] << 8) + cis[i + 29]);
					break;

				case HNBU_PO_CCKOFDM:
					varbuf_append(&b, vstr_cck2gpo,
						(cis[i + 2] << 8) + cis[i + 1]);
					varbuf_append(&b, vstr_ofdm2gpo,
						(cis[i + 6] << 24) + (cis[i + 5] << 16) +
						(cis[i + 4] << 8) + cis[i + 3]);
					if (tlen < 19) break;

					varbuf_append(&b, vstr_ofdm5gpo,
						(cis[i + 10] << 24) + (cis[i + 9] << 16) +
						(cis[i + 8] << 8) + cis[i + 7]);
					varbuf_append(&b, vstr_ofdm5glpo,
						(cis[i + 14] << 24) + (cis[i + 13] << 16) +
						(cis[i + 12] << 8) + cis[i + 11]);
					varbuf_append(&b, vstr_ofdm5ghpo,
						(cis[i + 18] << 24) + (cis[i + 17] << 16) +
						(cis[i + 16] << 8) + cis[i + 15]);
					break;

				case HNBU_PO_MCS2G:
					for (j = 0; j <= (tlen/2); j++) {
						varbuf_append(&b, vstr_mcspo, 2, j,
							(cis[i + 2 + 2*j] << 8) + cis[i + 1 + 2*j]);
					}
					break;

				case HNBU_PO_MCS5GM:
					for (j = 0; j <= (tlen/2); j++) {
						varbuf_append(&b, vstr_mcspo, 5, j,
							(cis[i + 2 + 2*j] << 8) + cis[i + 1 + 2*j]);
					}
					break;

				case HNBU_PO_MCS5GLH:
					for (j = 0; j <= (tlen/4); j++) {
						varbuf_append(&b, vstr_mcspohl, 5, 'l', j,
							(cis[i + 2 + 2*j] << 8) + cis[i + 1 + 2*j]);
					}

					for (j = 0; j <= (tlen/4); j++) {
						varbuf_append(&b, vstr_mcspohl, 5, 'h', j,
							(cis[i + ((tlen/2)+2) + 2*j] << 8) +
							cis[i + ((tlen/2)+1) + 2*j]);
					}

					break;

				case HNBU_PO_CDD:
					varbuf_append(&b, vstr_cddpo,
					              (cis[i + 2] << 8) + cis[i + 1]);
					break;

				case HNBU_PO_STBC:
					varbuf_append(&b, vstr_stbcpo,
					              (cis[i + 2] << 8) + cis[i + 1]);
					break;

				case HNBU_PO_40M:
					varbuf_append(&b, vstr_bw40po,
					              (cis[i + 2] << 8) + cis[i + 1]);
					break;

				case HNBU_PO_40MDUP:
					varbuf_append(&b, vstr_bwduppo,
					              (cis[i + 2] << 8) + cis[i + 1]);
					break;

				case HNBU_OFDMPO5G:
					varbuf_append(&b, vstr_ofdm5gpo,
						(cis[i + 4] << 24) + (cis[i + 3] << 16) +
						(cis[i + 2] << 8) + cis[i + 1]);
					varbuf_append(&b, vstr_ofdm5glpo,
						(cis[i + 8] << 24) + (cis[i + 7] << 16) +
						(cis[i + 6] << 8) + cis[i + 5]);
					varbuf_append(&b, vstr_ofdm5ghpo,
						(cis[i + 12] << 24) + (cis[i + 11] << 16) +
						(cis[i + 10] << 8) + cis[i + 9]);
					break;
				/* Power per rate for SROM V9 */
				case HNBU_CCKBW202GPO:
					varbuf_append(&b, vstr_cckbw202gpo[0],
						((cis[i + 2] << 8) + cis[i + 1]));
					if (tlen > 4)
						varbuf_append(&b, vstr_cckbw202gpo[1],
							((cis[i + 4] << 8) + cis[i + 3]));
					if (tlen > 6)
						varbuf_append(&b, vstr_cckbw202gpo[2],
							((cis[i + 6] << 8) + cis[i + 5]));
					break;

				case HNBU_LEGOFDMBW202GPO:
					varbuf_append(&b, vstr_legofdmbw202gpo[0],
						((cis[i + 4] << 24) + (cis[i + 3] << 16) +
						(cis[i + 2] << 8) + cis[i + 1]));
					if (tlen > 6)  {
						varbuf_append(&b, vstr_legofdmbw202gpo[1],
							((cis[i + 8] << 24) + (cis[i + 7] << 16) +
							(cis[i + 6] << 8) + cis[i + 5]));
					}
					break;

				case HNBU_LEGOFDMBW205GPO:
					for (j = 0; j < 6; j++) {
						if (tlen < (2 + 4 * j))
							break;
						varbuf_append(&b, vstr_legofdmbw205gpo[j],
							((cis[4 * j + i + 4] << 24)
							+ (cis[4 * j + i + 3] << 16)
							+ (cis[4 * j + i + 2] << 8)
							+ cis[4 * j + i + 1]));
					}
					break;

				case HNBU_MCS2GPO:
					for (j = 0; j < 4; j++) {
						if (tlen < (2 + 4 * j))
							break;
						varbuf_append(&b, vstr_mcs2gpo[j],
							((cis[4 * j + i + 4] << 24)
							+ (cis[4 * j + i + 3] << 16)
							+ (cis[4 * j + i + 2] << 8)
							+ cis[4 * j + i + 1]));
					}
					break;

				case HNBU_MCS5GLPO:
					for (j = 0; j < 3; j++) {
						if (tlen < (2 + 4 * j))
							break;
						varbuf_append(&b, vstr_mcs5glpo[j],
							((cis[4 * j + i + 4] << 24)
							+ (cis[4 * j + i + 3] << 16)
							+ (cis[4 * j + i + 2] << 8)
							+ cis[4 * j + i + 1]));
					}
					break;

				case HNBU_MCS5GMPO:
					for (j = 0; j < 3; j++) {
						if (tlen < (2 + 4 * j))
							break;
						varbuf_append(&b, vstr_mcs5gmpo[j],
							((cis[4 * j + i + 4] << 24)
							+ (cis[4 * j + i + 3] << 16)
							+ (cis[4 * j + i + 2] << 8)
							+ cis[4 * j + i + 1]));
					}
					break;

				case HNBU_MCS5GHPO:
					for (j = 0; j < 3; j++) {
						if (tlen < (2 + 4 * j))
							break;
						varbuf_append(&b, vstr_mcs5ghpo[j],
							((cis[4 * j + i + 4] << 24)
							+ (cis[4 * j + i + 3] << 16)
							+ (cis[4 * j + i + 2] << 8)
							+ cis[4 * j + i + 1]));
					}
					break;

				case HNBU_MCS32PO:
					varbuf_append(&b, vstr_mcs32po,
						(cis[i + 2] << 8) + cis[i + 1]);
					break;

				case HNBU_LEG40DUPPO:
					varbuf_append(&b, vstr_legofdm40duppo,
						(cis[i + 2] << 8) + cis[i + 1]);
					break;

				case HNBU_CUSTOM1:
					varbuf_append(&b, vstr_custom, 1, ((cis[i + 4] << 24) +
						(cis[i + 3] << 16) + (cis[i + 2] << 8) +
						cis[i + 1]));
					break;

#if defined(BCMCCISSR3)
				case HNBU_SROM3SWRGN:
					if (tlen >= 73) {
						uint16 srom[35];
						uint8 srev = cis[i + 1 + 70];
						ASSERT(srev == 3);
						/* make tuple value 16-bit aligned and parse it */
						bcopy(&cis[i + 1], srom, sizeof(srom));
						_initvars_srom_pci(srev, srom, SROM3_SWRGN_OFF, &b);
						/* 2.4G antenna gain is included in SROM */
						ag_init = TRUE;
						/* Ethernet MAC address is included in SROM */
						eabuf[0] = 0;
						boardnum = -1;
					}

					/* create extra variables */
					if (tlen >= 75)
						varbuf_append(&b, vstr_vendid,
						              (cis[i + 1 + 73] << 8) +
						              cis[i + 1 + 72]);
					if (tlen >= 77)
						varbuf_append(&b, vstr_devid,
						              (cis[i + 1 + 75] << 8) +
						              cis[i + 1 + 74]);
					if (tlen >= 79)
						varbuf_append(&b, vstr_xtalfreq,
						              (cis[i + 1 + 77] << 8) +
						              cis[i + 1 + 76]);
					break;
#endif

				case HNBU_CCKFILTTYPE:
					varbuf_append(&b, vstr_cckdigfilttype,
						(cis[i + 1]));
					break;

				case HNBU_TEMPTHRESH:
					varbuf_append(&b, vstr_tempthresh,
						(cis[i + 1]));
					/* period in msb nibble */
					varbuf_append(&b, vstr_temps_period,
						(cis[i + 2] & SROM11_TEMPS_PERIOD_MASK) >>
						SROM11_TEMPS_PERIOD_SHIFT);
					/* hysterisis in lsb nibble */
					varbuf_append(&b, vstr_temps_hysteresis,
						(cis[i + 2] & SROM11_TEMPS_HYSTERESIS_MASK) >>
						SROM11_TEMPS_HYSTERESIS_SHIFT);
					if (tlen >= 4) {
						varbuf_append(&b, vstr_tempoffset,
						(cis[i + 3]));
						varbuf_append(&b, vstr_tempsense_slope,
						(cis[i + 4]));
						varbuf_append(&b, vstr_temp_corrx,
						(cis[i + 5] & SROM11_TEMPCORRX_MASK) >>
						SROM11_TEMPCORRX_SHIFT);
						varbuf_append(&b, vstr_tempsense_option,
						(cis[i + 5] & SROM11_TEMPSENSE_OPTION_MASK) >>
						SROM11_TEMPSENSE_OPTION_SHIFT);
						varbuf_append(&b, vstr_phycal_tempdelta,
						(cis[i + 6]));
					}
					break;

				case HNBU_FEM_CFG: {
					/* fem_cfg1 */
					uint16 fem_cfg = (cis[i + 2] << 8) + cis[i + 1];
					varbuf_append(&b, vstr_femctrl,
						(fem_cfg & SROM11_FEMCTRL_MASK) >>
						SROM11_FEMCTRL_SHIFT);
					varbuf_append(&b, vstr_papdcap, 2,
						(fem_cfg & SROM11_PAPDCAP_MASK) >>
						SROM11_PAPDCAP_SHIFT);
					varbuf_append(&b, vstr_tworangetssi, 2,
						(fem_cfg & SROM11_TWORANGETSSI_MASK) >>
						SROM11_TWORANGETSSI_SHIFT);
					varbuf_append(&b, vstr_pdgaing, 2,
						(fem_cfg & SROM11_PDGAIN_MASK) >>
						SROM11_PDGAIN_SHIFT);
					varbuf_append(&b, vstr_epagaing, 2,
						(fem_cfg & SROM11_EPAGAIN_MASK) >>
						SROM11_EPAGAIN_SHIFT);
					varbuf_append(&b, vstr_tssiposslopeg, 2,
						(fem_cfg & SROM11_TSSIPOSSLOPE_MASK) >>
						SROM11_TSSIPOSSLOPE_SHIFT);
					/* fem_cfg2 */
					fem_cfg = (cis[i + 4] << 8) + cis[i + 3];
					varbuf_append(&b, vstr_gainctrlsph,
						(fem_cfg & SROM11_GAINCTRLSPH_MASK) >>
						SROM11_GAINCTRLSPH_SHIFT);
					varbuf_append(&b, vstr_papdcap, 5,
						(fem_cfg & SROM11_PAPDCAP_MASK) >>
						SROM11_PAPDCAP_SHIFT);
					varbuf_append(&b, vstr_tworangetssi, 5,
						(fem_cfg & SROM11_TWORANGETSSI_MASK) >>
						SROM11_TWORANGETSSI_SHIFT);
					varbuf_append(&b, vstr_pdgaing, 5,
						(fem_cfg & SROM11_PDGAIN_MASK) >>
						SROM11_PDGAIN_SHIFT);
					varbuf_append(&b, vstr_epagaing, 5,
						(fem_cfg & SROM11_EPAGAIN_MASK) >>
						SROM11_EPAGAIN_SHIFT);
					varbuf_append(&b, vstr_tssiposslopeg, 5,
						(fem_cfg & SROM11_TSSIPOSSLOPE_MASK) >>
						SROM11_TSSIPOSSLOPE_SHIFT);
					break;
				}

				case HNBU_ACPA_C0:
				{
					const int a = 0;

					varbuf_append(&b, vstr_subband5gver,
					              (cis[i + 2] << 8) + cis[i + 1]);
					varbuf_append(&b, vstr_maxp2ga, a,
					              (cis[i + 4] << 8) + cis[i + 3]);
					/* pa2g */
					varbuf_append(&b, vstr_pa2ga, a,
						(cis[i + 6] << 8) + cis[i + 5],
						(cis[i + 8] << 8) + cis[i + 7],
						(cis[i + 10] << 8) + cis[i + 9]);
					/* maxp5g */
					varbuf_append(&b, vstr_maxp5ga, a,
						cis[i + 11],
						cis[i + 12],
						cis[i + 13],
						cis[i + 14]);
					/* pa5g */
					varbuf_append(&b, vstr_pa5ga, a,
						(cis[i + 16] << 8) + cis[i + 15],
						(cis[i + 18] << 8) + cis[i + 17],
						(cis[i + 20] << 8) + cis[i + 19],
						(cis[i + 22] << 8) + cis[i + 21],
						(cis[i + 24] << 8) + cis[i + 23],
						(cis[i + 26] << 8) + cis[i + 25],
						(cis[i + 28] << 8) + cis[i + 27],
						(cis[i + 30] << 8) + cis[i + 29],
						(cis[i + 32] << 8) + cis[i + 31],
						(cis[i + 34] << 8) + cis[i + 33],
						(cis[i + 36] << 8) + cis[i + 35],
						(cis[i + 38] << 8) + cis[i + 37]);
					break;
				}

				case HNBU_ACPA_C1:
				{
					const int a = 1;

					varbuf_append(&b, vstr_maxp2ga, a,
					              (cis[i + 2] << 8) + cis[i + 1]);
					/* pa2g */
					varbuf_append(&b, vstr_pa2ga, a,
						(cis[i + 4] << 8) + cis[i + 3],
						(cis[i + 6] << 8) + cis[i + 5],
						(cis[i + 8] << 8) + cis[i + 7]);
					/* maxp5g */
					varbuf_append(&b, vstr_maxp5ga, a,
						cis[i + 9],
						cis[i + 10],
						cis[i + 11],
						cis[i + 12]);
					/* pa5g */
					varbuf_append(&b, vstr_pa5ga, a,
						(cis[i + 14] << 8) + cis[i + 13],
						(cis[i + 16] << 8) + cis[i + 15],
						(cis[i + 18] << 8) + cis[i + 17],
						(cis[i + 20] << 8) + cis[i + 19],
						(cis[i + 22] << 8) + cis[i + 21],
						(cis[i + 24] << 8) + cis[i + 23],
						(cis[i + 26] << 8) + cis[i + 25],
						(cis[i + 28] << 8) + cis[i + 27],
						(cis[i + 30] << 8) + cis[i + 29],
						(cis[i + 32] << 8) + cis[i + 31],
						(cis[i + 34] << 8) + cis[i + 33],
						(cis[i + 36] << 8) + cis[i + 35]);
					break;
				}

				case HNBU_ACPA_C2:
				{
					const int a = 2;

					varbuf_append(&b, vstr_maxp2ga, a,
					              (cis[i + 2] << 8) + cis[i + 1]);
					/* pa2g */
					varbuf_append(&b, vstr_pa2ga, a,
						(cis[i + 4] << 8) + cis[i + 3],
						(cis[i + 6] << 8) + cis[i + 5],
						(cis[i + 8] << 8) + cis[i + 7]);
					/* maxp5g */
					varbuf_append(&b, vstr_maxp5ga, a,
						cis[i + 9],
						cis[i + 10],
						cis[i + 11],
						cis[i + 12]);
					/* pa5g */
					varbuf_append(&b, vstr_pa5ga, a,
						(cis[i + 14] << 8) + cis[i + 13],
						(cis[i + 16] << 8) + cis[i + 15],
						(cis[i + 18] << 8) + cis[i + 17],
						(cis[i + 20] << 8) + cis[i + 19],
						(cis[i + 22] << 8) + cis[i + 21],
						(cis[i + 24] << 8) + cis[i + 23],
						(cis[i + 26] << 8) + cis[i + 25],
						(cis[i + 28] << 8) + cis[i + 27],
						(cis[i + 30] << 8) + cis[i + 29],
						(cis[i + 32] << 8) + cis[i + 31],
						(cis[i + 34] << 8) + cis[i + 33],
						(cis[i + 36] << 8) + cis[i + 35]);
					break;
				}

				case HNBU_MEAS_PWR:
					varbuf_append(&b, vstr_measpower, cis[i + 1]);
					varbuf_append(&b, vstr_measpowerX, 1, (cis[i + 2]));
					varbuf_append(&b, vstr_measpowerX, 2, (cis[i + 3]));
					varbuf_append(&b, vstr_rawtempsense,
						((cis[i + 5] & 0x1) << 8) + cis[i + 4]);
					break;

				case HNBU_PDOFF:
					varbuf_append(&b, vstr_pdoffsetma, 40, 0,
					      (cis[i + 2] << 8) + cis[i + 1]);
					varbuf_append(&b, vstr_pdoffsetma, 40, 1,
					      (cis[i + 4] << 8) + cis[i + 3]);
					varbuf_append(&b, vstr_pdoffsetma, 40, 2,
					      (cis[i + 6] << 8) + cis[i + 5]);
					varbuf_append(&b, vstr_pdoffsetma, 80, 0,
					      (cis[i + 8] << 8) + cis[i + 7]);
					varbuf_append(&b, vstr_pdoffsetma, 80, 1,
					      (cis[i + 10] << 8) + cis[i + 9]);
					varbuf_append(&b, vstr_pdoffsetma, 80, 2,
					      (cis[i + 12] << 8) + cis[i + 11]);
					break;

				case HNBU_ACPPR_2GPO:
					varbuf_append(&b, vstr_dot11agofdmhrbw202gpo,
					              (cis[i + 2] << 8) + cis[i + 1]);
					varbuf_append(&b, vstr_ofdmlrbw202gpo,
					              (cis[i + 4] << 8) + cis[i + 3]);

					if (tlen < 13) break;
					varbuf_append(&b, vstr_sb20in40dot11agofdm2gpo,
					              (cis[i + 6] << 8) + cis[i + 5]);
					varbuf_append(&b, vstr_sb20in80dot11agofdm2gpo,
					              (cis[i + 8] << 8) + cis[i + 7]);
					varbuf_append(&b, vstr_sb20in40ofdmlrbw202gpo,
					              (cis[i + 10] << 8) + cis[i + 9]);
					varbuf_append(&b, vstr_sb20in80ofdmlrbw202gpo,
					              (cis[i + 12] << 8) + cis[i + 11]);
					break;

				case HNBU_ACPPR_5GPO:
					varbuf_append(&b, vstr_mcsbw805gpo, 'l',
						(cis[i + 4] << 24) + (cis[i + 3] << 16) +
						(cis[i + 2] << 8) + cis[i + 1]);
					varbuf_append(&b, vstr_mcsbw1605gpo, 'l',
						(cis[i + 8] << 24) + (cis[i + 7] << 16) +
						(cis[i + 6] << 8) + cis[i + 5]);
					varbuf_append(&b, vstr_mcsbw805gpo, 'm',
						(cis[i + 12] << 24) + (cis[i + 11] << 16) +
						(cis[i + 10] << 8) + cis[i + 9]);
					varbuf_append(&b, vstr_mcsbw1605gpo, 'm',
						(cis[i + 16] << 24) + (cis[i + 15] << 16) +
						(cis[i + 14] << 8) + cis[i + 13]);
					varbuf_append(&b, vstr_mcsbw805gpo, 'h',
						(cis[i + 20] << 24) + (cis[i + 19] << 16) +
						(cis[i + 18] << 8) + cis[i + 17]);
					varbuf_append(&b, vstr_mcsbw1605gpo, 'h',
						(cis[i + 24] << 24) + (cis[i + 23] << 16) +
						(cis[i + 22] << 8) + cis[i + 21]);
					varbuf_append(&b, vstr_mcslr5gpo, 'l',
					              (cis[i + 26] << 8) + cis[i + 25]);
					varbuf_append(&b, vstr_mcslr5gpo, 'm',
					              (cis[i + 28] << 8) + cis[i + 27]);
					varbuf_append(&b, vstr_mcslr5gpo, 'h',
					              (cis[i + 30] << 8) + cis[i + 29]);

					if (tlen < 51) break;
					varbuf_append(&b, vstr_mcsbw80p805gpo, 'l',
						(cis[i + 34] << 24) + (cis[i + 33] << 16) +
						(cis[i + 32] << 8) + cis[i + 31]);
					varbuf_append(&b, vstr_mcsbw80p805gpo, 'm',
						(cis[i + 38] << 24) + (cis[i + 37] << 16) +
						(cis[i + 36] << 8) + cis[i + 35]);
					varbuf_append(&b, vstr_mcsbw80p805gpo, 'h',
						(cis[i + 42] << 24) + (cis[i + 41] << 16) +
						(cis[i + 40] << 8) + cis[i + 39]);
					varbuf_append(&b, vstr_mcsbw80p805g1po, 'x',
						(cis[i + 46] << 24) + (cis[i + 45] << 16) +
						(cis[i + 44] << 8) + cis[i + 43]);
					varbuf_append(&b, vstr_mcslr5g1po, 'x',
					              (cis[i + 48] << 8) + cis[i + 47]);
					varbuf_append(&b, vstr_mcslr5g80p80po,
					              (cis[i + 50] << 8) + cis[i + 49]);
					varbuf_append(&b, vstr_mcsbw805g1po, 'x',
						(cis[i + 54] << 24) + (cis[i + 53] << 16) +
						(cis[i + 52] << 8) + cis[i + 51]);
					varbuf_append(&b, vstr_mcsbw1605g1po, 'x',
						(cis[i + 58] << 24) + (cis[i + 57] << 16) +
						(cis[i + 56] << 8) + cis[i + 55]);

					break;

				case HNBU_MCS5Gx1PO:
					varbuf_append(&b, vstr_mcsbw205g1po, 'x',
						(cis[i + 4] << 24) + (cis[i + 3] << 16) +
						(cis[i + 2] << 8) + cis[i + 1]);
					varbuf_append(&b, vstr_mcsbw405g1po, 'x',
						(cis[i + 8] << 24) + (cis[i + 7] << 16) +
						(cis[i + 6] << 8) + cis[i + 5]);
					break;

				case HNBU_ACPPR_SBPO:
					varbuf_append(&b, vstr_sb20in40rpo, 'h',
					              (cis[i + 2] << 8) + cis[i + 1]);
					varbuf_append(&b, vstr_sb20in80and160r5gpo, 'h', 'l',
					              (cis[i + 4] << 8) + cis[i + 3]);
					varbuf_append(&b, vstr_sb40and80r5gpo, 'h', 'l',
					              (cis[i + 6] << 8) + cis[i + 5]);
					varbuf_append(&b, vstr_sb20in80and160r5gpo, 'h', 'm',
					              (cis[i + 8] << 8) + cis[i + 7]);
					varbuf_append(&b, vstr_sb40and80r5gpo, 'h', 'm',
					              (cis[i + 10] << 8) + cis[i + 9]);
					varbuf_append(&b, vstr_sb20in80and160r5gpo, 'h', 'h',
					              (cis[i + 12] << 8) + cis[i + 11]);
					varbuf_append(&b, vstr_sb40and80r5gpo, 'h', 'h',
					              (cis[i + 14] << 8) + cis[i + 13]);
					varbuf_append(&b, vstr_sb20in40rpo, 'l',
					              (cis[i + 16] << 8) + cis[i + 15]);
					varbuf_append(&b, vstr_sb20in80and160r5gpo, 'l', 'l',
					              (cis[i + 18] << 8) + cis[i + 17]);
					varbuf_append(&b, vstr_sb40and80r5gpo, 'l', 'l',
					              (cis[i + 20] << 8) + cis[i + 19]);
					varbuf_append(&b, vstr_sb20in80and160r5gpo, 'l', 'm',
					              (cis[i + 22] << 8) + cis[i + 21]);
					varbuf_append(&b, vstr_sb40and80r5gpo, 'l', 'm',
					              (cis[i + 24] << 8) + cis[i + 23]);
					varbuf_append(&b, vstr_sb20in80and160r5gpo, 'l', 'h',
					              (cis[i + 26] << 8) + cis[i + 25]);
					varbuf_append(&b, vstr_sb40and80r5gpo, 'l', 'h',
					              (cis[i + 28] << 8) + cis[i + 27]);
					varbuf_append(&b, vstr_dot11agduprpo, 'h',
						(cis[i + 32] << 24) + (cis[i + 31] << 16) +
						(cis[i + 30] << 8) + cis[i + 29]);
					varbuf_append(&b, vstr_dot11agduprpo, 'l',
						(cis[i + 36] << 24) + (cis[i + 35] << 16) +
						(cis[i + 34] << 8) + cis[i + 33]);

					if (tlen < 49) break;
					varbuf_append(&b, vstr_sb20in40and80rpo, 'h',
						(cis[i + 38] << 8) + cis[i + 37]);
					varbuf_append(&b, vstr_sb20in40and80rpo, 'l',
						(cis[i + 40] << 8) + cis[i + 39]);
					varbuf_append(&b, vstr_sb20in80and160r5g1po, 'h', 'x',
						(cis[i + 42] << 8) + cis[i + 41]);
					varbuf_append(&b, vstr_sb20in80and160r5g1po, 'l', 'x',
						(cis[i + 44] << 8) + cis[i + 43]);
					varbuf_append(&b, vstr_sb40and80r5g1po, 'h', 'x',
						(cis[i + 46] << 8) + cis[i + 45]);
					varbuf_append(&b, vstr_sb40and80r5g1po, 'l', 'x',
						(cis[i + 48] << 8) + cis[i + 47]);
					break;

				case HNBU_ACPPR_SB8080_PO:
					varbuf_append(&b, vstr_sb2040and80in80p80r5gpo, 'h', 'l',
						(cis[i + 2] << 8) + cis[i + 1]);
					varbuf_append(&b, vstr_sb2040and80in80p80r5gpo, 'l', 'l',
						(cis[i + 4] << 8) + cis[i + 3]);
					varbuf_append(&b, vstr_sb2040and80in80p80r5gpo, 'h', 'm',
						(cis[i + 6] << 8) + cis[i + 5]);
					varbuf_append(&b, vstr_sb2040and80in80p80r5gpo, 'l', 'm',
						(cis[i + 8] << 8) + cis[i + 7]);
					varbuf_append(&b, vstr_sb2040and80in80p80r5gpo, 'h', 'h',
						(cis[i + 10] << 8) + cis[i + 9]);
					varbuf_append(&b, vstr_sb2040and80in80p80r5gpo, 'l', 'h',
						(cis[i + 12] << 8) + cis[i + 11]);
					varbuf_append(&b, vstr_sb2040and80in80p80r5g1po, 'h', 'x',
						(cis[i + 14] << 8) + cis[i + 13]);
					varbuf_append(&b, vstr_sb2040and80in80p80r5g1po, 'l', 'x',
						(cis[i + 16] << 8) + cis[i + 15]);
					varbuf_append(&b, vstr_sb20in80p80r5gpo, 'h',
						(cis[i + 18] << 8) + cis[i + 17]);
					varbuf_append(&b, vstr_sb20in80p80r5gpo, 'l',
						(cis[i + 20] << 8) + cis[i + 19]);
					varbuf_append(&b, vstr_dot11agduppo,
						(cis[i + 22] << 8) + cis[i + 21]);
					break;

				case HNBU_NOISELVL:
					/* noiselvl2g */
					varbuf_append(&b, vstr_noiselvl2ga, 0,
					              (cis[i + 1] & 0x1f));
					varbuf_append(&b, vstr_noiselvl2ga, 1,
					              (cis[i + 2] & 0x1f));
					varbuf_append(&b, vstr_noiselvl2ga, 2,
					              (cis[i + 3] & 0x1f));
					/* noiselvl5g */
					varbuf_append(&b, vstr_noiselvl5ga, 0,
					              (cis[i + 4] & 0x1f),
					              (cis[i + 5] & 0x1f),
					              (cis[i + 6] & 0x1f),
					              (cis[i + 7] & 0x1f));
					varbuf_append(&b, vstr_noiselvl5ga, 1,
					              (cis[i + 8] & 0x1f),
					              (cis[i + 9] & 0x1f),
					              (cis[i + 10] & 0x1f),
					              (cis[i + 11] & 0x1f));
					varbuf_append(&b, vstr_noiselvl5ga, 2,
					              (cis[i + 12] & 0x1f),
					              (cis[i + 13] & 0x1f),
					              (cis[i + 14] & 0x1f),
					              (cis[i + 15] & 0x1f));
					break;

				case HNBU_RXGAIN_ERR:
					varbuf_append(&b, vstr_rxgainerr2ga, 0,
					              (cis[i + 1] & 0x3f));
					varbuf_append(&b, vstr_rxgainerr2ga, 1,
					              (cis[i + 2] & 0x1f));
					varbuf_append(&b, vstr_rxgainerr2ga, 2,
					              (cis[i + 3] & 0x1f));
					varbuf_append(&b, vstr_rxgainerr5ga, 0,
					              (cis[i + 4] & 0x3f),
					              (cis[i + 5] & 0x3f),
					              (cis[i + 6] & 0x3f),
					              (cis[i + 7] & 0x3f));
					varbuf_append(&b, vstr_rxgainerr5ga, 1,
					              (cis[i + 8] & 0x1f),
					              (cis[i + 9] & 0x1f),
					              (cis[i + 10] & 0x1f),
					              (cis[i + 11] & 0x1f));
					varbuf_append(&b, vstr_rxgainerr5ga, 2,
					              (cis[i + 12] & 0x1f),
					              (cis[i + 13] & 0x1f),
					              (cis[i + 14] & 0x1f),
					              (cis[i + 15] & 0x1f));
					break;

				case HNBU_AGBGA:
					varbuf_append(&b, vstr_agbg, 0, cis[i + 1]);
					varbuf_append(&b, vstr_agbg, 1, cis[i + 2]);
					varbuf_append(&b, vstr_agbg, 2, cis[i + 3]);
					varbuf_append(&b, vstr_aga, 0, cis[i + 4]);
					varbuf_append(&b, vstr_aga, 1, cis[i + 5]);
					varbuf_append(&b, vstr_aga, 2, cis[i + 6]);
					break;

				case HNBU_ACRXGAINS_C0: {
					int a = 0;

					/* rxgains */
					uint16 rxgains = (cis[i + 2] << 8) + cis[i + 1];
					varbuf_append(&b, vstr_rxgainsgtrelnabypa, 5, a,
						(rxgains & SROM11_RXGAINS5GTRELNABYPA_MASK) >>
						SROM11_RXGAINS5GTRELNABYPA_SHIFT);
					varbuf_append(&b, vstr_rxgainsgtrisoa, 5, a,
						(rxgains & SROM11_RXGAINS5GTRISOA_MASK) >>
						SROM11_RXGAINS5GTRISOA_SHIFT);
					varbuf_append(&b, vstr_rxgainsgelnagaina, 5, a,
						(rxgains & SROM11_RXGAINS5GELNAGAINA_MASK) >>
						SROM11_RXGAINS5GELNAGAINA_SHIFT);
					varbuf_append(&b, vstr_rxgainsgtrelnabypa, 2, a,
						(rxgains & SROM11_RXGAINS2GTRELNABYPA_MASK) >>
						SROM11_RXGAINS2GTRELNABYPA_SHIFT);
					varbuf_append(&b, vstr_rxgainsgtrisoa, 2, a,
						(rxgains & SROM11_RXGAINS2GTRISOA_MASK) >>
						SROM11_RXGAINS2GTRISOA_SHIFT);
					varbuf_append(&b, vstr_rxgainsgelnagaina, 2, a,
						(rxgains & SROM11_RXGAINS2GELNAGAINA_MASK) >>
						SROM11_RXGAINS2GELNAGAINA_SHIFT);
					/* rxgains1 */
					rxgains = (cis[i + 4] << 8) + cis[i + 3];
					varbuf_append(&b, vstr_rxgainsgxtrelnabypa, 5, 'h', a,
						(rxgains & SROM11_RXGAINS5GTRELNABYPA_MASK) >>
						SROM11_RXGAINS5GTRELNABYPA_SHIFT);
					varbuf_append(&b, vstr_rxgainsgxtrisoa, 5, 'h', a,
						(rxgains & SROM11_RXGAINS5GTRISOA_MASK) >>
						SROM11_RXGAINS5GTRISOA_SHIFT);
					varbuf_append(&b, vstr_rxgainsgxelnagaina, 5, 'h', a,
						(rxgains & SROM11_RXGAINS5GELNAGAINA_MASK) >>
						SROM11_RXGAINS5GELNAGAINA_SHIFT);
					varbuf_append(&b, vstr_rxgainsgxtrelnabypa, 5, 'm', a,
						(rxgains & SROM11_RXGAINS5GTRELNABYPA_MASK) >>
						SROM11_RXGAINS5GTRELNABYPA_SHIFT);
					varbuf_append(&b, vstr_rxgainsgxtrisoa, 5, 'm', a,
						(rxgains & SROM11_RXGAINS5GTRISOA_MASK) >>
						SROM11_RXGAINS5GTRISOA_SHIFT);
					varbuf_append(&b, vstr_rxgainsgxelnagaina, 5, 'm', a,
						(rxgains & SROM11_RXGAINS5GELNAGAINA_MASK) >>
						SROM11_RXGAINS5GELNAGAINA_SHIFT);
					break;
				}

				case HNBU_ACRXGAINS_C1: {
					int a = 1;

					/* rxgains */
					uint16 rxgains = (cis[i + 2] << 8) + cis[i + 1];
					varbuf_append(&b, vstr_rxgainsgtrelnabypa, 5, a,
						(rxgains & SROM11_RXGAINS5GTRELNABYPA_MASK) >>
						SROM11_RXGAINS5GTRELNABYPA_SHIFT);
					varbuf_append(&b, vstr_rxgainsgtrisoa, 5, a,
						(rxgains & SROM11_RXGAINS5GTRISOA_MASK) >>
						SROM11_RXGAINS5GTRISOA_SHIFT);
					varbuf_append(&b, vstr_rxgainsgelnagaina, 5, a,
						(rxgains & SROM11_RXGAINS5GELNAGAINA_MASK) >>
						SROM11_RXGAINS5GELNAGAINA_SHIFT);
					varbuf_append(&b, vstr_rxgainsgtrelnabypa, 2, a,
						(rxgains & SROM11_RXGAINS2GTRELNABYPA_MASK) >>
						SROM11_RXGAINS2GTRELNABYPA_SHIFT);
					varbuf_append(&b, vstr_rxgainsgtrisoa, 2, a,
						(rxgains & SROM11_RXGAINS2GTRISOA_MASK) >>
						SROM11_RXGAINS2GTRISOA_SHIFT);
					varbuf_append(&b, vstr_rxgainsgelnagaina, 2, a,
						(rxgains & SROM11_RXGAINS2GELNAGAINA_MASK) >>
						SROM11_RXGAINS2GELNAGAINA_SHIFT);
					/* rxgains1 */
					rxgains = (cis[i + 4] << 8) + cis[i + 3];
					varbuf_append(&b, vstr_rxgainsgxtrelnabypa, 5, 'h', a,
						(rxgains & SROM11_RXGAINS5GTRELNABYPA_MASK) >>
						SROM11_RXGAINS5GTRELNABYPA_SHIFT);
					varbuf_append(&b, vstr_rxgainsgxtrisoa, 5, 'h', a,
						(rxgains & SROM11_RXGAINS5GTRISOA_MASK) >>
						SROM11_RXGAINS5GTRISOA_SHIFT);
					varbuf_append(&b, vstr_rxgainsgxelnagaina, 5, 'h', a,
						(rxgains & SROM11_RXGAINS5GELNAGAINA_MASK) >>
						SROM11_RXGAINS5GELNAGAINA_SHIFT);
					varbuf_append(&b, vstr_rxgainsgxtrelnabypa, 5, 'm', a,
						(rxgains & SROM11_RXGAINS5GTRELNABYPA_MASK) >>
						SROM11_RXGAINS5GTRELNABYPA_SHIFT);
					varbuf_append(&b, vstr_rxgainsgxtrisoa, 5, 'm', a,
						(rxgains & SROM11_RXGAINS5GTRISOA_MASK) >>
						SROM11_RXGAINS5GTRISOA_SHIFT);
					varbuf_append(&b, vstr_rxgainsgxelnagaina, 5, 'm', a,
						(rxgains & SROM11_RXGAINS5GELNAGAINA_MASK) >>
						SROM11_RXGAINS5GELNAGAINA_SHIFT);
					break;
				}

				case HNBU_ACRXGAINS_C2: {
					int a = 2;

					/* rxgains */
					uint16 rxgains = (cis[i + 2] << 8) + cis[i + 1];
					varbuf_append(&b, vstr_rxgainsgtrelnabypa, 5, a,
						(rxgains & SROM11_RXGAINS5GTRELNABYPA_MASK) >>
						SROM11_RXGAINS5GTRELNABYPA_SHIFT);
					varbuf_append(&b, vstr_rxgainsgtrisoa, 5, a,
						(rxgains & SROM11_RXGAINS5GTRISOA_MASK) >>
						SROM11_RXGAINS5GTRISOA_SHIFT);
					varbuf_append(&b, vstr_rxgainsgelnagaina, 5, a,
						(rxgains & SROM11_RXGAINS5GELNAGAINA_MASK) >>
						SROM11_RXGAINS5GELNAGAINA_SHIFT);
					varbuf_append(&b, vstr_rxgainsgtrelnabypa, 2, a,
						(rxgains & SROM11_RXGAINS2GTRELNABYPA_MASK) >>
						SROM11_RXGAINS2GTRELNABYPA_SHIFT);
					varbuf_append(&b, vstr_rxgainsgtrisoa, 2, a,
						(rxgains & SROM11_RXGAINS2GTRISOA_MASK) >>
						SROM11_RXGAINS2GTRISOA_SHIFT);
					varbuf_append(&b, vstr_rxgainsgelnagaina, 2, a,
						(rxgains & SROM11_RXGAINS2GELNAGAINA_MASK) >>
						SROM11_RXGAINS2GELNAGAINA_SHIFT);
					/* rxgains1 */
					rxgains = (cis[i + 4] << 8) + cis[i + 3];
					varbuf_append(&b, vstr_rxgainsgxtrelnabypa, 5, 'h', a,
						(rxgains & SROM11_RXGAINS5GTRELNABYPA_MASK) >>
						SROM11_RXGAINS5GTRELNABYPA_SHIFT);
					varbuf_append(&b, vstr_rxgainsgxtrisoa, 5, 'h', a,
						(rxgains & SROM11_RXGAINS5GTRISOA_MASK) >>
						SROM11_RXGAINS5GTRISOA_SHIFT);
					varbuf_append(&b, vstr_rxgainsgxelnagaina, 5, 'h', a,
						(rxgains & SROM11_RXGAINS5GELNAGAINA_MASK) >>
						SROM11_RXGAINS5GELNAGAINA_SHIFT);
					varbuf_append(&b, vstr_rxgainsgxtrelnabypa, 5, 'm', a,
						(rxgains & SROM11_RXGAINS5GTRELNABYPA_MASK) >>
						SROM11_RXGAINS5GTRELNABYPA_SHIFT);
					varbuf_append(&b, vstr_rxgainsgxtrisoa, 5, 'm', a,
						(rxgains & SROM11_RXGAINS5GTRISOA_MASK) >>
						SROM11_RXGAINS5GTRISOA_SHIFT);
					varbuf_append(&b, vstr_rxgainsgxelnagaina, 5, 'm', a,
						(rxgains & SROM11_RXGAINS5GELNAGAINA_MASK) >>
						SROM11_RXGAINS5GELNAGAINA_SHIFT);
					break;
				}

				case HNBU_TXDUTY: {
					varbuf_append(&b, vstr_txduty_ofdm, 40,
					              (cis[i + 2] << 8) + cis[i + 1]);
					varbuf_append(&b, vstr_txduty_thresh, 40,
					              (cis[i + 4] << 8) + cis[i + 3]);
					varbuf_append(&b, vstr_txduty_ofdm, 80,
					              (cis[i + 6] << 8) + cis[i + 5]);
					varbuf_append(&b, vstr_txduty_thresh, 80,
					              (cis[i + 8] << 8) + cis[i + 7]);
					break;
				}

				case HNBU_UUID:
					{
					/* uuid format 12345678-1234-5678-1234-567812345678 */

					char uuidstr[37]; /* 32 ids, 4 '-', 1 Null */

					snprintf(uuidstr, sizeof(uuidstr),
						"%02X%02X%02X%02X-%02X%02X-%02X%02X-"
						"%02X%02X-%02X%02X%02X%02X%02X%02X",
						cis[i + 1], cis[i + 2], cis[i + 3], cis[i + 4],
						cis[i + 5], cis[i + 6], cis[i + 7], cis[i + 8],
						cis[i + 9], cis[i + 10], cis[i + 11], cis[i + 12],
						cis[i + 13], cis[i + 14], cis[i + 15], cis[i + 16]);

					varbuf_append(&b, vstr_uuid, uuidstr);
					break;

					}
				case HNBU_WOWLGPIO:
					varbuf_append(&b, vstr_wowlgpio, ((cis[i + 1]) & 0x7F));
					varbuf_append(&b, vstr_wowlgpiopol,
						(((cis[i + 1]) >> 7) & 0x1));
					break;

#endif /* !BCM_BOOTLOADER */
				case HNBU_USBUTMI_CTL:
					varbuf_append(&b, vstr_usbutmi_ctl,
						(cis[i + 2] << 8) | cis[i + 1]);
					break;

				case HNBU_USBSSPHY_UTMI_CTL0:
					varbuf_append(&b, vstr_usbssphy_utmi_ctl0,
						(cis[i + 4] << 24) | (cis[i + 3] << 16) |
						(cis[i + 2] << 8) | cis[i + 1]);
					break;

				case HNBU_USBSSPHY_UTMI_CTL1:
					varbuf_append(&b, vstr_usbssphy_utmi_ctl1,
						(cis[i + 4] << 24) | (cis[i + 3] << 16) |
						(cis[i + 2] << 8) | cis[i + 1]);
					break;

				case HNBU_USBSSPHY_UTMI_CTL2:
					varbuf_append(&b, vstr_usbssphy_utmi_ctl2,
						(cis[i + 4] << 24) | (cis[i + 3] << 16) |
						(cis[i + 2] << 8) | cis[i + 1]);
					break;

				case HNBU_USBSSPHY_SLEEP0:
					varbuf_append(&b, vstr_usbssphy_sleep0,
						(cis[i + 2] << 8) | cis[i + 1]);
					break;

				case HNBU_USBSSPHY_SLEEP1:
					varbuf_append(&b, vstr_usbssphy_sleep1,
						(cis[i + 2] << 8) | cis[i + 1]);
					break;

				case HNBU_USBSSPHY_SLEEP2:
					varbuf_append(&b, vstr_usbssphy_sleep2,
						(cis[i + 2] << 8) | cis[i + 1]);
					break;

				case HNBU_USBSSPHY_SLEEP3:
					varbuf_append(&b, vstr_usbssphy_sleep3,
						(cis[i + 2] << 8) | cis[i + 1]);
					break;
				case HNBU_USBSSPHY_MDIO:
					{
					int setnum, k;

					setnum = (cis[i + 1])/4;
					if (setnum == 0)
						break;
					for (j = 0; j < setnum; j++) {
						k = j*12;
						varbuf_append(&b, vstr_usbssphy_mdio, j,
						(cis[i+4+k]<<16) | (cis[i+3+k]<<8) | cis[i+2+k],
						(cis[i+7+k]<<16) | (cis[i+6+k]<<8) | cis[i+5+k],
						(cis[i+10+k]<<16) | (cis[i+9+k]<<8) | cis[i+8+k],
						(cis[i+13+k]<<16) | (cis[i+12+k]<<8) | cis[i+11+k]);
						}
					}
					break;
				case HNBU_USB30PHY_NOSS:
					{
						varbuf_append(&b, vstr_usb30phy_noss, cis[i + 1]);
					}
					break;
				case HNBU_USB30PHY_U1U2:
					{
						varbuf_append(&b, vstr_usb30phy_u1u2, cis[i + 1]);
					}
					break;
				case HNBU_USB30PHY_REGS:
					{
						varbuf_append(&b, vstr_usb30phy_regs, 0,
							cis[i+4]|cis[i+3]|cis[i+2]|cis[i+1],
							cis[i+8]|cis[i+7]|cis[i+6]|cis[i+5],
							cis[i+12]|cis[i+11]|cis[i+10]|cis[i+9],
							cis[i+16]|cis[i+15]|cis[i+14]|cis[i+13]);
						varbuf_append(&b, vstr_usb30phy_regs, 1,
							cis[i+20]|cis[i+19]|cis[i+18]|cis[i+17],
							cis[i+24]|cis[i+23]|cis[i+22]|cis[i+21],
							cis[i+28]|cis[i+27]|cis[i+26]|cis[i+25],
							cis[i+32]|cis[i+31]|cis[i+30]|cis[i+29]);

					}
					break;

				case HNBU_PDOFF_2G:
					{
					uint16 pdoff_2g = (cis[i + 2] << 8) + cis[i + 1];
					varbuf_append(&b, vstr_pdoffset2gma, 40, 0,
						(pdoff_2g & SROM11_PDOFF_2G_40M_A0_MASK) >>
						SROM11_PDOFF_2G_40M_A0_SHIFT);
					varbuf_append(&b, vstr_pdoffset2gma, 40, 1,
						(pdoff_2g & SROM11_PDOFF_2G_40M_A1_MASK) >>
						SROM11_PDOFF_2G_40M_A1_SHIFT);
					varbuf_append(&b, vstr_pdoffset2gma, 40, 2,
						(pdoff_2g & SROM11_PDOFF_2G_40M_A2_MASK) >>
						SROM11_PDOFF_2G_40M_A2_SHIFT);
					varbuf_append(&b, vstr_pdoffset2gmvalid, 40,
						(pdoff_2g & SROM11_PDOFF_2G_40M_VALID_MASK) >>
						SROM11_PDOFF_2G_40M_VALID_SHIFT);
					break;
					}

				case HNBU_ACPA_CCK_C0:
					varbuf_append(&b, vstr_pa2gccka, 0,
					        (cis[i + 2] << 8) + cis[i + 1],
						(cis[i + 4] << 8) + cis[i + 3],
						(cis[i + 6] << 8) + cis[i + 5]);
					break;

				case HNBU_ACPA_CCK_C1:
					varbuf_append(&b, vstr_pa2gccka, 1,
						(cis[i + 2] << 8) + cis[i + 1],
						(cis[i + 4] << 8) + cis[i + 3],
						(cis[i + 6] << 8) + cis[i + 5]);
					break;

				case HNBU_ACPA_40:
					varbuf_append(&b, vstr_pa5gbw40a, 0,
					        (cis[i + 2] << 8) + cis[i + 1],
						(cis[i + 4] << 8) + cis[i + 3],
						(cis[i + 6] << 8) + cis[i + 5],
					        (cis[i + 8] << 8) + cis[i + 7],
						(cis[i + 10] << 8) + cis[i + 9],
						(cis[i + 12] << 8) + cis[i + 11],
					        (cis[i + 14] << 8) + cis[i + 13],
						(cis[i + 16] << 8) + cis[i + 15],
						(cis[i + 18] << 8) + cis[i + 17],
					        (cis[i + 20] << 8) + cis[i + 19],
						(cis[i + 22] << 8) + cis[i + 21],
						(cis[i + 24] << 8) + cis[i + 23]);
					break;

				case HNBU_ACPA_80:
					varbuf_append(&b, vstr_pa5gbw80a, 0,
					        (cis[i + 2] << 8) + cis[i + 1],
						(cis[i + 4] << 8) + cis[i + 3],
						(cis[i + 6] << 8) + cis[i + 5],
					        (cis[i + 8] << 8) + cis[i + 7],
						(cis[i + 10] << 8) + cis[i + 9],
						(cis[i + 12] << 8) + cis[i + 11],
					        (cis[i + 14] << 8) + cis[i + 13],
						(cis[i + 16] << 8) + cis[i + 15],
						(cis[i + 18] << 8) + cis[i + 17],
					        (cis[i + 20] << 8) + cis[i + 19],
						(cis[i + 22] << 8) + cis[i + 21],
						(cis[i + 24] << 8) + cis[i + 23]);
					break;

				case HNBU_ACPA_4080:
					varbuf_append(&b, vstr_pa5gbw4080a, 0,
					        (cis[i + 2] << 8) + cis[i + 1],
						(cis[i + 4] << 8) + cis[i + 3],
						(cis[i + 6] << 8) + cis[i + 5],
					        (cis[i + 8] << 8) + cis[i + 7],
						(cis[i + 10] << 8) + cis[i + 9],
						(cis[i + 12] << 8) + cis[i + 11],
					        (cis[i + 14] << 8) + cis[i + 13],
						(cis[i + 16] << 8) + cis[i + 15],
						(cis[i + 18] << 8) + cis[i + 17],
					        (cis[i + 20] << 8) + cis[i + 19],
						(cis[i + 22] << 8) + cis[i + 21],
						(cis[i + 24] << 8) + cis[i + 23]);
					varbuf_append(&b, vstr_pa5gbw4080a, 1,
					        (cis[i + 26] << 8) + cis[i + 25],
						(cis[i + 28] << 8) + cis[i + 27],
						(cis[i + 30] << 8) + cis[i + 29],
					        (cis[i + 32] << 8) + cis[i + 31],
						(cis[i + 34] << 8) + cis[i + 33],
						(cis[i + 36] << 8) + cis[i + 35],
					        (cis[i + 38] << 8) + cis[i + 37],
						(cis[i + 40] << 8) + cis[i + 39],
						(cis[i + 42] << 8) + cis[i + 41],
					        (cis[i + 44] << 8) + cis[i + 43],
						(cis[i + 46] << 8) + cis[i + 45],
						(cis[i + 48] << 8) + cis[i + 47]);
					break;

				case HNBU_ACPA_4X4C0:
				case HNBU_ACPA_4X4C1:
				case HNBU_ACPA_4X4C2:
				case HNBU_ACPA_4X4C3:
				{
					int core_num = 0;
					uint8 tuple = cis[i];

					if (tuple == HNBU_ACPA_4X4C1) {
						core_num = 1;
					} else if (tuple == HNBU_ACPA_4X4C2) {
						core_num = 2;
					} else if (tuple == HNBU_ACPA_4X4C3) {
						core_num = 3;
					}

					varbuf_append(&b, vstr_maxp2ga, core_num, cis[i + 1]);
					/* pa2g */
					varbuf_append(&b, vstr_sr13pa2ga, core_num,
						(cis[i + 3] << 8) + cis[i + 2],
						(cis[i + 5] << 8) + cis[i + 4],
						(cis[i + 7] << 8) + cis[i + 6],
						(cis[i + 9] << 8) + cis[i + 8]);
					/* pa2g40 */
					varbuf_append(&b, vstr_pa2g40a, core_num,
						(cis[i + 11] << 8) + cis[i + 10],
						(cis[i + 13] << 8) + cis[i + 12],
						(cis[i + 15] << 8) + cis[i + 14],
						(cis[i + 17] << 8) + cis[i + 16]);
					for (j = 0; j < 5; j++) {
						varbuf_append(&b, vstr_maxp5gba, j, core_num,
							cis[i + j + 18]);
					}
					break;
				}

				case HNBU_ACPA_BW20_4X4C0:
				case HNBU_ACPA_BW40_4X4C0:
				case HNBU_ACPA_BW80_4X4C0:
				case HNBU_ACPA_BW20_4X4C1:
				case HNBU_ACPA_BW40_4X4C1:
				case HNBU_ACPA_BW80_4X4C1:
				case HNBU_ACPA_BW20_4X4C2:
				case HNBU_ACPA_BW40_4X4C2:
				case HNBU_ACPA_BW80_4X4C2:
				case HNBU_ACPA_BW20_4X4C3:
				case HNBU_ACPA_BW40_4X4C3:
				case HNBU_ACPA_BW80_4X4C3:
				{
					int k = 0;
					char pabuf[140]; /* max: 20 '0x????'s + 19 ','s + 1 Null */
					int core_num = 0, buflen = 0;
					uint8 tuple = cis[i];

					if (tuple == HNBU_ACPA_BW20_4X4C1 ||
						tuple == HNBU_ACPA_BW40_4X4C1 ||
						tuple == HNBU_ACPA_BW80_4X4C1) {
						core_num = 1;
					} else if (tuple == HNBU_ACPA_BW20_4X4C2 ||
						tuple == HNBU_ACPA_BW40_4X4C2 ||
						tuple == HNBU_ACPA_BW80_4X4C2) {
						core_num = 2;
					} else if (tuple == HNBU_ACPA_BW20_4X4C3 ||
						tuple == HNBU_ACPA_BW40_4X4C3 ||
						tuple == HNBU_ACPA_BW80_4X4C3) {
						core_num = 3;
					}

					buflen = sizeof(pabuf);
					for (j = 0; j < 20; j++) { /* cis[i+1] - cis[i+40] */
						k += snprintf(pabuf+k, buflen-k, "0x%x",
							((cis[i + (2*j) + 2] << 8) +
							cis[i + (2*j) + 1]));
						if (j < 19) {
							k += snprintf(pabuf+k, buflen-k,
								",");
						}
					}

					if (tuple == HNBU_ACPA_BW20_4X4C0 ||
						tuple == HNBU_ACPA_BW20_4X4C1 ||
						tuple == HNBU_ACPA_BW20_4X4C2 ||
						tuple == HNBU_ACPA_BW20_4X4C3) {
						varbuf_append(&b, vstr_sr13pa5ga, core_num, pabuf);
					} else {
						int bw = 40;

						if (tuple == HNBU_ACPA_BW80_4X4C0 ||
							tuple == HNBU_ACPA_BW80_4X4C1 ||
							tuple == HNBU_ACPA_BW80_4X4C2 ||
							tuple == HNBU_ACPA_BW80_4X4C3) {
							bw = 80;
						}
						varbuf_append(&b, vstr_sr13pa5gbwa, bw,
							core_num, pabuf);
					}
					break;
				}

				case HNBU_SUBBAND5GVER:
					varbuf_append(&b, vstr_subband5gver,
					        (cis[i + 2] << 8) + cis[i + 1]);
					break;

				case HNBU_PAPARAMBWVER:
					varbuf_append(&b, vstr_paparambwver, cis[i + 1]);
					break;

				case HNBU_TXBFRPCALS:
				/* note: all 5 rpcal parameters are expected to be */
				/* inside one tuple record, i.e written with one  */
				/* wl wrvar cmd as follows: */
				/* wl wrvar rpcal2g=0x1211 ... rpcal5gb3=0x0  */
					if (tlen != 11 ) { /* sanity check */
						BS_ERROR(("%s:incorrect length:%d for"
							" HNBU_TXBFRPCALS tuple\n",
							__FUNCTION__, tlen));
						break;
					}

					varbuf_append(&b, vstr_paparamrpcalvars[0],
						(cis[i + 1] + (cis[i + 2] << 8)));
					varbuf_append(&b, vstr_paparamrpcalvars[1],
						(cis[i + 3]  +  (cis[i + 4] << 8)));
					varbuf_append(&b, vstr_paparamrpcalvars[2],
						(cis[i + 5]  +  (cis[i + 6] << 8)));
					varbuf_append(&b, vstr_paparamrpcalvars[3],
						(cis[i + 7]  +  (cis[i + 8] << 8)));
					varbuf_append(&b, vstr_paparamrpcalvars[4],
						(cis[i + 9]  +  (cis[i + 10] << 8)));
					break;

				case HNBU_GPIO_PULL_DOWN:
					varbuf_append(&b, vstr_gpdn,
					              (cis[i + 4] << 24) |
					              (cis[i + 3] << 16) |
					              (cis[i + 2] << 8) |
					              cis[i + 1]);
					break;

				case HNBU_MACADDR2:
					if (!(ETHER_ISNULLADDR(&cis[i+1])) &&
					    !(ETHER_ISMULTI(&cis[i+1]))) {
						bcm_ether_ntoa((struct ether_addr *)&cis[i + 1],
						               eabuf2);
					}
					break;
				} /* CISTPL_BRCM_HNBU */
				break;
			} /* switch (tup) */

			i += tlen;
		} while (tup != CISTPL_END);
	}

	if (boardnum != -1) {
		varbuf_append(&b, vstr_boardnum, boardnum);
	}

	if (eabuf[0]) {
		varbuf_append(&b, vstr_macaddr, eabuf);
	}

	if (eabuf2[0]) {
		varbuf_append(&b, vstr_macaddr2, eabuf2);
	}

	if (ccodelock) {
		varbuf_append(&b, vstr_ccodelock);
	}

#ifndef BCM_BOOTLOADER
	/* if there is no antenna gain field, set default */
	sromrev = (sromrev == 1) ? (uint8)getintvar(NULL, rstr_sromrev) : sromrev;
	if (sromrev <= 10 && getvar(NULL, rstr_ag0) == NULL && ag_init == FALSE) {
		varbuf_append(&b, vstr_ag, 0, 0xff);
	}
#endif

	/* final nullbyte terminator */
	ASSERT(b.size >= 1);
#if !defined(DONGLEBUILD)
	if (!flash_nvram_present(sih, osh, &b.buf)) {
		*b.buf++ = '\0';
	}
#else
	*b.buf++ = '\0';
#endif

	ASSERT(b.buf - base <= MAXSZ_NVRAM_VARS);

	/* initvars_table() MALLOCs, copies and assigns the MALLOCed buffer to '*vars' */
	err = initvars_table(osh, base /* start */, b.buf /* end */, vars, count);

	MFREE(osh, base, MAXSZ_NVRAM_VARS);
	return err;
} /* srom_parsecis */
#endif /* !defined(BCMDONGLEHOST) */

/**
 * Read/write/unlock uWire SPROM
 */
static uint16
srom_cc_cmd_uwire(si_t *sih, osl_t *osh, chipcregs_t *cc, enum sprom_op_e op,
                  uint wordoff, uint16 data)
{
	uint wait_cnt = 1000;
	uint32 byteoff = 0;
	uint32 sprom_size = 0;				/**< in [bytes] */
	uint32 cmd;

	BCM_REFERENCE(sih);
	byteoff = wordoff * 2;
	switch (op) {
	case SPROM_OP_RD:
		cmd = UWIRE_OP_READ;
		break;
	case SPROM_OP_WR:
		cmd = UWIRE_OP_WRITE;
		break;
	case SPROM_OP_WRLOCK:
		switch (data) {
		case FALSE:
			cmd = UWIRE_OP_WREN;
			break;
		case TRUE:
			cmd = UWIRE_OP_WRDIS;
			break;
		default:
			ASSERT(0);
			return 0xffff;
		}
		break;
	default:
		ASSERT(0);
		return 0xffff;
	}

	sprom_size = ((R_REG(osh, &cc->sromcontrol) & UWIRE_SIZE_MASK) >> UWIRE_SIZE_SHIFT);
	if (sprom_size == UWIRE_SIZE_8192BITS) {
		sprom_size = 8192;
	} else if (sprom_size == UWIRE_SIZE_2048BITS) {
		sprom_size = 2048;
	} else if (sprom_size == UWIRE_SIZE_512BITS) {
		sprom_size = 512;
	} else if (sprom_size == UWIRE_SIZE_128BITS) {
		sprom_size = 128;
	} else {
		ASSERT(0);
		return 0xffff;
	}

	if (byteoff >= sprom_size) {
		return 0xffff;
	}

	if ((cmd == UWIRE_OP_READ) || (cmd == UWIRE_OP_WRITE)) {
		W_REG(osh, &cc->chipcontrol, (byteoff & SROM16K_BANK_SEL_MASK) >>
			SROM16K_BANK_SHFT_MASK);
		W_REG(osh, &cc->sromaddress, (byteoff  & SROM16K_ADDR_SEL_MASK));
		if (cmd == UWIRE_OP_WRITE) {
			W_REG(osh, &cc->sromdata, (uint32)data);
		}
	}

	W_REG(osh, &cc->sromcontrol, UWIRE_START | cmd);

	while (wait_cnt--) {
		if ((R_REG(osh, &cc->sromcontrol) & UWIRE_BUSY) == 0)
			break;
	}

	if (!wait_cnt) {
		BS_ERROR(("%s: Command 0x%x timed out\n", __FUNCTION__, cmd));
		return 0xffff;
	}

	if (cmd == UWIRE_OP_READ) {
		return (uint16)R_REG(osh, &cc->sromdata);
	} else {
		return 0xffff;
	}
} /* srom_cc_cmd_uwire */

/**
 * Read/write/unlock SPI SPROM
 */
static uint16
srom_cc_cmd_spi(si_t *sih, osl_t *osh, chipcregs_t *cc, enum sprom_op_e op,
                  uint wordoff, uint16 data)
{
	uint32 ctrl = 0;
	uint32 cmd;
	uint32 sprom_size = 0;				/**< in [bytes] */
	uint16 byteoff = 0;
	uint wait_cnt = 1000;

	ASSERT(CCREV(sih->ccrev) >= 132);

	ctrl = R_REG(osh, &cc->sprom_spi_ctrl);
	if (!(ctrl & SPROM_SPICTL_SPROM_PRESENT) ||
	    (ctrl & (SPROM_SPICTL_OTP_SEL | SPROM_SPICTL_START_BUSY))) {
		ASSERT(ctrl & SPROM_SPICTL_SPROM_PRESENT);
		ASSERT((ctrl & SPROM_SPICTL_OTP_SEL) == 0);
		ASSERT((ctrl & SPROM_SPICTL_START_BUSY) == 0);
		BS_ERROR(("%s %x fail\n", __FUNCTION__, ctrl));
		return 0xFFFF;
	}

	switch (op) {
	case SPROM_OP_WRLOCK:
		switch (data) {
		case FALSE:
			cmd = SPROM_SPI_CMD_WREN;
			break;
		case TRUE:
			cmd = SPROM_SPI_CMD_WRDIS;
			break;
		default:
			ASSERT(0);
			return 0xffff;
		}
		break;
	case SPROM_OP_RD:
		cmd = SPROM_SPI_CMD_READ;
		break;
	case SPROM_OP_WR:
		cmd = SPROM_SPI_CMD_WRITE;
		break;
	default:
		ASSERT(0);
		return 0xffff;
	}

	if (cmd == SPROM_SPI_CMD_READ || cmd == SPROM_SPI_CMD_WRITE) {
		switch ((R_REG(osh, &cc->sprom_spi_ctrl) & SPROM_SPICTL_SPROM_SIZE_MASK) >>
		        SPROM_SPICTL_SPROM_SIZE_SHIFT) {
		case SPROM_SPI_SZ_RSVD:
			sprom_size = 4096;
			break;
		case SPROM_SPI_SZ_16KBITS:
			sprom_size = 16384;
			break;
		case SPROM_SPI_SZ_32KBITS:
			sprom_size = 32768;
			break;
		case SPROM_SPI_SZ_64KBITS:
			sprom_size = 65536;
			break;
		default:
			ASSERT(0);
			return 0xFFFF;
		}

		if ((R_REG(osh, &cc->sprom_spi_ctrl) & SPROM_SPICTL_ADDR_SIZE) == 0) {
			sprom_size = MIN(sprom_size, 4096);
		}

		sprom_size /= NBBY; /* from [bits] to [bytes] */
		byteoff = wordoff * sizeof(uint16);
		if (byteoff >= sprom_size) {
			return 0xFFFF;
		}

		W_REG(osh, &cc->sprom_spi_addr, byteoff);
		if (cmd == SPROM_SPI_CMD_WRITE) {
			W_REG(osh, &cc->sprom_spi_data, (uint32)data);
		}
	}

	ctrl &= ~(SPROM_SPICTL_SPI_CMD_MASK | SPROM_SPICTL_SPI_DATASZ_MASK);
	ctrl |= (cmd << SPROM_SPICTL_SPI_CMD_SHIFT);
	ctrl |= (SPROM_SPI_ACCESS_SZ_2B << SPROM_SPICTL_SPI_DATASZ_SHIFT);
	W_REG(osh, &cc->sprom_spi_ctrl, SPROM_SPICTL_START_BUSY | ctrl);
	while (wait_cnt--) {
		if ((R_REG(osh, &cc->sprom_spi_ctrl) & SPROM_SPICTL_START_BUSY) == 0) {
			break;
		}
	}

	if (wait_cnt == 0) {
		BS_ERROR(("%s: Command 0x%x timed out\n", __FUNCTION__, ctrl));
		return 0xFFFF;
	}

	if (cmd == SPROM_SPI_CMD_READ) {
		return (R_REG(osh, &cc->sprom_spi_data) & 0xFFFF);
	}

	return 0xFFFF;
} /* srom_cc_cmd_spi */

/**
 * Read/write/unlock SPI or uWire SPROM
 */
static uint16
srom_cc_cmd(si_t *sih, osl_t *osh, chipcregs_t *cc, enum sprom_op_e op,
            uint wordoff, uint16 data)
{
	switch (sih->sprom_bus) {
	case SPROM_BUS_UWIRE:
		return srom_cc_cmd_uwire(sih, osh, cc, op, wordoff, data);
	case SPROM_BUS_SPI:
		if (op == SPROM_OP_WR) {
			/* Set write enable for each write transaction is necessary */
			srom_cc_cmd_spi(sih, osh, cc, SPROM_OP_WRLOCK, 0, FALSE);
		}
		return srom_cc_cmd_spi(sih, osh, cc, op, wordoff, data);
		break;
	default:
		ASSERT(0);
		break;
	}

	return 0xFFFF;
} /* srom_cc_cmd */

#define CC_SROM_SHADOW_WSIZE	512	/* 0x800 - 0xC00 */

int
sprom_read_pci(osl_t *osh, si_t *sih, uint16 *shadow, uint wordoff, uint16 *buf, uint nwords,
	bool check_crc)
{
	int err = 0;
	uint i;
	void *ccregs = NULL;
	uint32 sprom_size = 0;					/**< in [bytes] */
	uint32 sprom_num_words;					/**< # of 16 bits words */
	chipcregs_t *cc;

	if (BCM43684_CHIP(sih->chip) ||
		BCM6710_CHIP(sih->chip) ||
		BCM6715_CHIP(sih->chip) ||
		BCM6717_CHIP(sih->chip) ||
		BCM6726_CHIP(sih->chip) ||
		BCM6711_CHIP(sih->chip)) {
		si_srom_clk_set(sih); /* corrects srom clock frequency */
	}

	ccregs = (void *)((uint8 *)shadow - CC_SROM_OTP);
	cc = (chipcregs_t *)ccregs;
	sprom_size = ((R_REG(osh, &cc->sromcontrol) & UWIRE_SIZE_MASK) >> UWIRE_SIZE_SHIFT);
	if (sprom_size == UWIRE_SIZE_8192BITS)
		sprom_size = 8192;
	else if (sprom_size == UWIRE_SIZE_2048BITS)
		sprom_size = 2048;
	else if (sprom_size == UWIRE_SIZE_512BITS)
		sprom_size = 512;
	else if (sprom_size == UWIRE_SIZE_128BITS)
		sprom_size = 128;
	else {
		ASSERT(0);
		sprom_size = 0;
	}

	sprom_num_words = sprom_size / 2;

	/* read the sprom */
	for (i = 0; i < nwords; i++) {
		if (ISSIM_ENAB(sih)) {
			/* use indirect since direct is too slow on QT */
			/* Also enable indirect access with 4365 for SROMREV13 */
			if ((sih->cccaps & CC_CAP_SROM) == 0) {
				err = 1;
				goto error;
			}

			buf[i] = srom_cc_cmd(sih, osh, ccregs, SPROM_OP_RD, wordoff + i, 0);
		} else {
			if (ISSIM_ENAB(sih)) {
				buf[i] = R_REG(osh, &shadow[wordoff + i]);
			}

			if ((wordoff + i) >= sprom_num_words) {
				buf[i] = 0xffff;
			} else if (((wordoff + i) >= CC_SROM_SHADOW_WSIZE) ||
#ifndef DONGLEBUILD
					/*
					 * In NIC mode, the read needs to be done in indirect mode.
					 * A direct read can take over 60,000 cycles which can cause
					 * the UBUS transaction ID to roll over if there are 256
					 * other requests that come in in the meantime. This hangs
					 * the bus at bootup.
					 */
					TRUE) {
#else
					FALSE) {
#endif
				/* Srom shadow region in chipcommon is only 512 words
				 * use indirect access for Srom beyond 512 words
				 */
				buf[i] = srom_cc_cmd(sih, osh, ccregs, SPROM_OP_RD,
				                     wordoff + i, 0);
			} else {
				buf[i] = R_REG(osh, &shadow[wordoff + i]);
			}
		}

		if (i == SROM13_SIGN) {
			if ((buf[SROM13_SIGN] !=  SROM13_SIGNATURE) && (nwords == SROM13_WORDS)) {
				err = 1;
				goto error;
			}
		}
	}

	/* bypass crc checking for simulation to allow srom hack */
	if (ISSIM_ENAB(sih)) {
		goto error;
	}

	if (check_crc) {
		if (buf[0] == 0xffff) {
			/* The hardware thinks that an srom that starts with 0xffff
			 * is blank, regardless of the rest of the content, so declare
			 * it bad.
			 */
			BS_ERROR(("%s: buf[0] = 0x%x, returning bad-crc\n", __FUNCTION__, buf[0]));
			err = 1;
			goto error;
		}

		/* fixup the endianness so crc8 will pass */
		htol16_buf(buf, nwords * 2);
		if (hndcrc8((uint8 *)buf, nwords * 2, CRC8_INIT_VALUE) != CRC8_GOOD_VALUE) {
			/* DBG only pci always read srom4 first, then srom8/9 */
			/* BS_ERROR(("%s: bad crc\n", __FUNCTION__)); */
			err = 1;
		}

		/* now correct the endianness of the byte array */
		ltoh16_buf(buf, nwords * 2);
	}

error:
	return err;
} /* sprom_read_pci */

#if !defined(BCMDONGLEHOST)
#if defined(BCMNVRAMW) || defined(BCMNVRAMR)
static int
BCMSROMATTACHFN(otp_read_pci)(osl_t *osh, si_t *sih, uint16 *buf, uint bufsz)
{
	uint8 *otp;
	uint sz = OTP_SZ_MAX / 2; /* size in words */
	int err = 0;

	if (bufsz > OTP_SZ_MAX) {
		return BCME_ERROR;
	}

	/* freed in same function */
	if ((otp = MALLOC_NOPERSIST(osh, OTP_SZ_MAX)) == NULL) {
		return BCME_ERROR;
	}

	bzero(otp, OTP_SZ_MAX);

	err = otp_read_region(sih, OTP_HW_RGN, (uint16 *)otp, &sz);

	if (err) {
		MFREE(osh, otp, OTP_SZ_MAX);
		return err;
	}

	bcopy(otp, buf, bufsz);

	/* Check CRC */
	if (((uint16 *)otp)[0] == 0xffff) {
		/* The hardware thinks that an srom that starts with 0xffff
		 * is blank, regardless of the rest of the content, so declare
		 * it bad.
		 */
		BS_ERROR(("%s: otp[0] = 0x%x, returning bad-crc\n",
		          __FUNCTION__, ((uint16 *)otp)[0]));
		MFREE(osh, otp, OTP_SZ_MAX);
		return 1;
	}

	/* fixup the endianness so crc8 will pass */
	htol16_buf(otp, OTP_SZ_MAX);
	if (hndcrc8(otp, SROM4_WORDS * 2, CRC8_INIT_VALUE) != CRC8_GOOD_VALUE &&
		hndcrc8(otp, SROM10_WORDS * 2, CRC8_INIT_VALUE) != CRC8_GOOD_VALUE &&
		hndcrc8(otp, SROM11_WORDS * 2, CRC8_INIT_VALUE) != CRC8_GOOD_VALUE &&
		hndcrc8(otp, SROM12_WORDS * 2, CRC8_INIT_VALUE) != CRC8_GOOD_VALUE &&
		hndcrc8(otp, SROM13_WORDS * 2, CRC8_INIT_VALUE) != CRC8_GOOD_VALUE &&
		hndcrc8(otp, SROM18_WORDS * 2, CRC8_INIT_VALUE) != CRC8_GOOD_VALUE) {
		BS_ERROR(("%s: bad crc\n", __FUNCTION__));
		err = 1;
	}

	MFREE(osh, otp, OTP_SZ_MAX);

	return err;
} /* otp_read_pci */
#endif /* defined(BCMNVRAMW) || defined(BCMNVRAMR) */
#endif /* !defined(BCMDONGLEHOST) */

int
srom_otp_write_region_crc(si_t *sih, uint nbytes, uint16* buf16, bool write)
{
#if defined(WLTEST) || defined(BCMDBG)
	int err = 0, crc = 0;
#if !defined(BCMDONGLEHOST)
	uint8 *buf8;

	/* Check nbytes is not odd or too big */
	if ((nbytes & 1) || (nbytes > SROM_MAX))
		return 1;

	/* block invalid buffer size */
	if (nbytes < SROM4_WORDS * 2)
		return BCME_BUFTOOSHORT;
	else if (nbytes > SROM20_WORDS * 2)
		return BCME_BUFTOOLONG;

	/* Verify signatures */
	if (!((buf16[SROM4_SIGN] == SROM4_SIGNATURE) ||
		(buf16[SROM8_SIGN] == SROM4_SIGNATURE) ||
		(buf16[SROM10_SIGN] == SROM4_SIGNATURE) ||
		(buf16[SROM11_SIGN] == SROM11_SIGNATURE) ||
		(buf16[SROM12_SIGN] == SROM12_SIGNATURE) ||
		(buf16[SROM13_SIGN] == SROM13_SIGNATURE) ||
		(buf16[SROM18_SIGN] == SROM18_SIGNATURE) ||
		(buf16[SROM19_SIGN] == SROM19_SIGNATURE) ||
		(buf16[SROM20_SIGN] == SROM20_SIGNATURE))) {
		BS_ERROR(("%s: wrong signature SROM4_SIGN %x SROM8_SIGN %x SROM10_SIGN %x\n",
			__FUNCTION__, buf16[SROM4_SIGN], buf16[SROM8_SIGN], buf16[SROM10_SIGN]));
		return BCME_ERROR;
	}

	/* Check CRC */
	if (buf16[0] == 0xffff) {
		/* The hardware thinks that an srom that starts with 0xffff
		 * is blank, regardless of the rest of the content, so declare
		 * it bad.
		 */
		BS_ERROR(("%s: invalid buf16[0] = 0x%x\n", __FUNCTION__, buf16[0]));
		goto out;
	}

	buf8 = (uint8*)buf16;
	/* fixup the endianness and then calculate crc */
	htol16_buf(buf8, nbytes);
	crc = ~hndcrc8(buf8, nbytes - 1, CRC8_INIT_VALUE);
	/* now correct the endianness of the byte array */
	ltoh16_buf(buf8, nbytes);
	if (nbytes == SROM20_WORDS * 2)
		buf16[SROM20_CRCREV] = (crc << 8) | (buf16[SROM20_CRCREV] & 0xff);
	else if (nbytes == SROM19_WORDS * 2)
		buf16[SROM19_CRCREV] = (crc << 8) | (buf16[SROM19_CRCREV] & 0xff);
	else if (nbytes == SROM18_WORDS * 2)
		buf16[SROM18_CRCREV] = (crc << 8) | (buf16[SROM18_CRCREV] & 0xff);
	else if (nbytes == SROM13_WORDS * 2)
		buf16[SROM13_CRCREV] = (crc << 8) | (buf16[SROM13_CRCREV] & 0xff);
	else if (nbytes == SROM12_WORDS * 2)
		buf16[SROM12_CRCREV] = (crc << 8) | (buf16[SROM12_CRCREV] & 0xff);
	else if (nbytes == SROM11_WORDS * 2)
		buf16[SROM11_CRCREV] = (crc << 8) | (buf16[SROM11_CRCREV] & 0xff);
	else if (nbytes == SROM10_WORDS * 2)
		buf16[SROM10_CRCREV] = (crc << 8) | (buf16[SROM10_CRCREV] & 0xff);
	else
		buf16[SROM4_CRCREV] = (crc << 8) | (buf16[SROM4_CRCREV] & 0xff);

#ifdef BCMNVRAMW
	/* Write the CRC back */
	if (write)
		err = otp_write_region(sih, OTP_HW_RGN, buf16, nbytes/2, 0);
#endif /* BCMNVRAMW */

out:
#endif /* !defined(BCMDONGLEHOST) */
	return write ? err : crc;
#else
	BCM_REFERENCE(sih);
	BCM_REFERENCE(nbytes);
	BCM_REFERENCE(buf16);
	BCM_REFERENCE(write);
	return 0;
#endif
} /* srom_otp_write_region_crc */

#if !defined(BCMDONGLEHOST)
/**
 * Create variable table from memory.
 * Return 0 on success, nonzero on error.
 */
static int
BCMATTACHFN(initvars_table)(osl_t *osh, char *start, char *end, char **vars, uint *count)
{
	int c = (int)(end - start);

	/* do it only when there is more than just the null string */
	if (c > 1) {
		char *vp = MALLOC_NOPERSIST(osh, c);
		ASSERT(vp != NULL);
		if (!vp)
			return BCME_NOMEM;
		bcopy(start, vp, c);
		*vars = vp;
		*count = c;
	} else {
		*vars = NULL;
		*count = 0;
	}

	return 0;
}

int
BCMATTACHFN(dbushost_initvars_flash)(si_t *sih, osl_t *osh, char **base, uint len)
{
	return initvars_only_prefixed(sih, osh, base, len);
}

/**
 * Copies a subset of all nvram variables from a source (which can be flash, or an external nvram
 * store (such as scm on a router) to a caller-allocated buffer. Assumes that every nvram variable
 * in this source is prefixed, either with a 'long' or 'compact' prefix. Only nvram variables
 * whoes prefix matches the current device/slice are copied into the caller-allocated buffer.
 *
 *
 * @param[inout] base  Caller allocated buffer that will contain the filtered nvram variable/value
 *                     pairs. At function exit, the *base pointer has been moved forward to point at
 *                     the end of the buffer.
 *
 * @return       0 on success, nonzero on error.
 *
 * Prerequisites:
 * - The CC core has been selected
 */
static int
BCMATTACHFN(initvars_only_prefixed)(si_t *sih, osl_t *osh, char **base, uint len)
{
	char *vp = *base;
	char *flash; /** temporary malloc'ed buffer */
	int err;
	char *s;
	uint l, dl, copy_len;
	char devpath[SI_DEVPATH_BUFSZ], devpath_pcie[SI_DEVPATH_BUFSZ];
	char compact_prefix[SI_DEVPATH_BUFSZ] = {0};
	int path_len, compact_len, devid_len, pcie_path_len;

#if defined(BCMQT) && defined(CONFIG_BCM96756)
	memcpy(*base, defaultsromvars_6756, sizeof(defaultsromvars_6756));
	*base += sizeof(defaultsromvars_6756);
	return BCME_OK;
#endif /* BCMQT && CONFIG_BCM96756 */

	/* allocate memory and read in flash */
	/* freed in same function */
	if (!(flash = MALLOC_NOPERSIST(osh, MAX_NVRAM_SPACE)))
		return BCME_NOMEM;

	if ((err = nvram_getall(flash, MAX_NVRAM_SPACE))) {
		goto exit;
	}

	/* create legacy devpath prefix */
	si_devpath(sih, devpath, sizeof(devpath));
	path_len = strlen(devpath);

	if (BUSTYPE(sih->bustype) == PCI_BUS) {
		si_devpath_pcie(sih, devpath_pcie, sizeof(devpath_pcie));
		pcie_path_len = strlen(devpath_pcie);
	} else {
		pcie_path_len = 0;
	}

	/* create compact devpath prefix (eg '0:varname=...' instead of 'sb/0/varname=...') */
	si_compact_devpathvar(sih, compact_prefix, sizeof(compact_prefix), "devid");

	/* compact_prefix now is 'xx:devid, eat ending 'devid' */
	/* to be 'xx:' */
	devid_len = strlen("devid");
	compact_len = strlen(compact_prefix);
	if (compact_len > devid_len) {
		compact_prefix[compact_len - devid_len] = '\0';
		compact_len -= devid_len;
	}
	else
		compact_len = 0;

	/* make sure strlen never reads beyond the buffer */
	flash[MAX_NVRAM_SPACE - 1] = 0;

	/* grab vars with the <devpath> prefix or <compact_prefix> prefix in name */
	for (s = flash; (s < flash + MAX_NVRAM_SPACE) && *s; s += l + 1) {
		l = strlen(s);

		if (strncmp(s, devpath, path_len) == 0)
			dl = path_len; /* nvram variable uses 'long' prefix */
		else if (pcie_path_len && strncmp(s, devpath_pcie, pcie_path_len) == 0)
			dl = pcie_path_len;
		else if (compact_len && strncmp(s, compact_prefix, compact_len) == 0)
			dl = compact_len; /* nvram variable uses 'compact' prefix */
		else
			continue; /* skip non-matching nvram variable */

		/* is there enough room to copy? */
		copy_len = l - dl + 1;
		if (len < copy_len) {
			err = BCME_BUFTOOSHORT;
			goto exit;
		}

		/* no prefix, just the name=value */
		strncpy(vp, &s[dl], copy_len);
		vp += copy_len;
		len -= copy_len;
	}

	if (s >= flash + MAX_NVRAM_SPACE) {
		BS_ERROR(("Nvram was not terminated with double zeroes.\n"));
	}

	/* add null string as terminator */
	if (len < 1) {
		err = BCME_BUFTOOSHORT;
		goto exit;
	}
	*vp++ = '\0';

	*base = vp;

exit:
	MFREE(osh, flash, MAX_NVRAM_SPACE);
	return err;
} /* initvars_only_prefixed */
#endif /* !defined(BCMDONGLEHOST) */

#if !defined(BCMSDIODEV_ENABLED) && !defined(BCMPCIEDEV_ENABLED)
#if !defined(BCMDONGLEHOST)
/**
 * Initialize nonvolatile variable table from an nvram source (like flash, or external nvram).
 * Return 0 on success, nonzero on error.
 *
 * Prerequisites:
 * - The CC core has been selected
 */
static int
BCMATTACHFN(initvars_si)(si_t *sih, char **vars, uint *count)
{
	osl_t *osh = si_osh(sih);
	char *vp, *base;
	int err;

	ASSERT(vars != NULL);
	ASSERT(count != NULL);

	/* freed in same function */
	base = vp = MALLOC_NOPERSIST(osh, MAXSZ_NVRAM_VARS);
	ASSERT(vp != NULL);
	if (!vp)
		return BCME_NOMEM;

	if ((err = initvars_only_prefixed(sih, osh, &vp, MAXSZ_NVRAM_VARS)) == 0) {
		err = initvars_table(osh, base, vp, vars, count);
	}

	MFREE(osh, base, MAXSZ_NVRAM_VARS);

	return err;
}
#endif /* !BCMDONGLEHOST */
#endif	/* !BCMSDIODEV_ENABLED && !BCMPCIEDEV_ENABLED */

#if !defined(BCMDONGLEHOST)

/** returns position of rightmost bit that was set in caller supplied mask */
static uint
mask_shift(uint16 mask)
{
	uint i;
	for (i = 0; i < (sizeof(mask) << 3); i ++) {
		if (mask & (1 << i))
			return i;
	}
	ASSERT(mask);
	return 0;
}

static uint
mask_width(uint16 mask)
{
	int i;
	for (i = (sizeof(mask) << 3) - 1; i >= 0; i --) {
		if (mask & (1 << i))
			return (uint)(i - mask_shift(mask) + 1);
	}
	ASSERT(mask);
	return 0;
}

#ifdef BCMASSERT_SUPPORT
static bool
mask_valid(uint16 mask)
{
	uint shift = mask_shift(mask);
	uint width = mask_width(mask);
	return mask == ((~0 << shift) & ~(~0 << (shift + width)));
}
#endif

/**
 * Parses caller supplied SROM contents into name=value pairs. Global array pci_sromvars[] contains
 * the link between a word offset in SROM and the corresponding NVRAM variable name.
 *
 * @param[in]  srom  Points at the SROM word array. In the case of !BCMSDIO, this word array
 *                   starts at the PCIe header in SPROM.
 * @param[in]  off   Specifies the offset of the first word 'srom' points to, which should
 *                   be either 0 or SROM3_SWRG_OFF (full SROM or software region).
 * @param[out] b     The name/value pairs are written into this called provided buffer.
 */
void
BCMATTACHFN(_initvars_srom_pci)(uint8 sromrev, uint16 *srom, uint off, varbuf_t *b)
{
	uint16 w;
	uint32 val;
	const sromvar_t *srv;
	uint width;
	uint flags;
	uint32 sr = (1 << sromrev);
	bool in_array = FALSE;
	static char array_temp[256];
	uint array_curr = 0;
	const char* array_name = NULL;
	uint pcie_inc = 0;

	varbuf_append(b, "sromrev=%d", sromrev);
#if !defined(SROM15_MEMOPT) && !defined(SROM17_MEMOPT)
	if (sromrev == 15)
		srv = pci_srom15vars;
	else if (sromrev == 16)
		srv = pci_srom16vars;
	else if (sromrev == 17)
		srv = pci_srom17vars;
	else
		srv = pci_sromvars;
#else
#if defined(SROM15_MEMOPT)
	srv = pci_srom15vars;
#endif /* defined(SROM15_MEMOPT) */
#if defined(SROM17_MEMOPT)
	srv = pci_srom17vars;
#endif /* defined(SROM17_MEMOPT) */
#endif /* !defined(SROM15_MEMOPT) && !defined(SROM17_MEMOPT) */

	for (; srv->name != NULL; srv ++) {
		const char *name;
		static bool in_array2 = FALSE;
		static char array_temp2[256];
		static uint array_curr2 = 0;
		static const char* array_name2 = NULL;

		if ((srv->revmask & sr) == 0)
		    continue;

		if (sromrev >= 18) {
			/*
			 * SROM18 is extension version of SROM13. PCIE portion of SROM18 increase
			 * to 112 words from 64 words, so the body portion of SROM18 need to be
			 * offset with (112 - 64). SROM19 has same offset
			 */
			if (srv->off < SROM13_PCIE_WORDS) {
				if (srv->off == SROM_DEVID_PCIE) {
					/* Adjust devid's offset for sromrev 18 */
					pcie_inc = SROM18_PCIE_INC;
				} else {
					pcie_inc = 0;
				}
			} else {
				/* Adjust srom body portion's offset for sromrev 18 */
				pcie_inc = SROM18_PCIE_INC;
			}
		} else {
			pcie_inc = 0;
		}

		if ((srv->off + pcie_inc) < off)
			continue;

		flags = srv->flags;
		name = srv->name;

		/* This entry is for mfgc only. Don't generate param for it, */
		if (flags & SRFL_NOVAR)
			continue;

		if (flags & SRFL_ETHADDR) {
			char eabuf[ETHER_ADDR_STR_LEN];
			struct ether_addr ea;

			ea.octet[0] = (srom[srv->off + pcie_inc - off] >> 8) & 0xff;
			ea.octet[1] = srom[srv->off + pcie_inc - off] & 0xff;
			ea.octet[2] = (srom[srv->off + pcie_inc + 1 - off] >> 8) & 0xff;
			ea.octet[3] = srom[srv->off + pcie_inc + 1 - off] & 0xff;
			ea.octet[4] = (srom[srv->off + pcie_inc + 2 - off] >> 8) & 0xff;
			ea.octet[5] = srom[srv->off + pcie_inc + 2 - off] & 0xff;
			bcm_ether_ntoa(&ea, eabuf);

			varbuf_append(b, "%s=%s", name, eabuf);
		} else {
			ASSERT(mask_valid(srv->mask));
			ASSERT(mask_width(srv->mask));

			/* Start of an array */
			if (sromrev >= 10 && (srv->flags & SRFL_ARRAY) && !in_array2) {
				array_curr2 = 0;
				array_name2 = (const char*)srv->name;
				memset((void*)array_temp2, 0, sizeof(array_temp2));
				in_array2 = TRUE;
			}

			w = srom[srv->off + pcie_inc - off];
			val = (w & srv->mask) >> mask_shift(srv->mask);
			width = mask_width(srv->mask);

			while (srv->flags & SRFL_MORE) {
				srv ++;
				ASSERT(srv->name != NULL);

				if (((srv->off + pcie_inc) == 0) || ((srv->off + pcie_inc) < off))
					continue;

				ASSERT(mask_valid(srv->mask));
				ASSERT(mask_width(srv->mask));

				w = srom[srv->off + pcie_inc - off];
				val += ((w & srv->mask) >> mask_shift(srv->mask)) << width;
				width += mask_width(srv->mask);
			}

			if ((flags & SRFL_NOFFS) && ((int)val == (1 << width) - 1))
				continue;

			/* Array support starts in sromrev 10. Skip arrays for sromrev <= 9 */
			if (sromrev <= 9 && srv->flags & SRFL_ARRAY) {
				while (srv->flags & SRFL_ARRAY)
					srv ++;
				srv ++;
			}

			if (in_array2) {
				int ret;

				if (flags & SRFL_PRHEX) {
					ret = snprintf(array_temp2 + array_curr2,
						sizeof(array_temp2) - array_curr2, "0x%x,", val);
				} else if ((flags & SRFL_PRSIGN) &&
					(val & (1 << (width - 1)))) {
					ret = snprintf(array_temp2 + array_curr2,
						sizeof(array_temp2) - array_curr2, "%d,",
						(int)(val | (~0 << width)));
				} else {
					ret = snprintf(array_temp2 + array_curr2,
						sizeof(array_temp2) - array_curr2, "%u,", val);
				}

				if (ret > 0) {
					array_curr2 += ret;
				} else {
					BS_ERROR(("%s: array %s parsing error. buffer too short.\n",
						__FUNCTION__, array_name2));
					ASSERT(0);

					/* buffer too small, skip this param */
					while (srv->flags & SRFL_ARRAY)
						srv ++;
					srv ++;
					in_array2 = FALSE;
					continue;
				}

				if (!(srv->flags & SRFL_ARRAY)) { /* Array ends */
					/* Remove the last ',' */
					array_temp2[array_curr2-1] = '\0';
					in_array2 = FALSE;
					varbuf_append(b, "%s=%s", array_name2, array_temp2);
				}
			} else if (flags & SRFL_CCODE) {
				if (val == 0)
					varbuf_append(b, "ccode=");
				else
					varbuf_append(b, "ccode=%c%c", (val >> 8), (val & 0xff));
			} else if (flags & SRFL_LEDDC) {
				/* LED Powersave duty cycle has to be scaled:
				*(oncount >> 24) (offcount >> 8)
				*/
				uint32 w32 = (((val >> 8) & 0xff) << 24) | /* oncount */
					     (((val & 0xff)) << 8); /* offcount */
				varbuf_append(b, "leddc=%d", w32);
			} else if (flags & SRFL_PRHEX) {
				varbuf_append(b, "%s=0x%x", name, val);
			} else if ((flags & SRFL_PRSIGN) && (val & (1 << (width - 1)))) {
				varbuf_append(b, "%s=%d", name, (int)(val | (~0 << width)));
			} else {
				varbuf_append(b, "%s=%u", name, val);
			}
		}
	}

	if (sromrev >= 4 && sromrev != 16) {
		/* Do per-path variables */
		uint p, pb, psz, path_num;
		uint bw160_shift = 0;
		uint bw160_antoff = 0;

		if ((sromrev == 17) || (sromrev == 15)) {
			pb = psz = 0;
			path_num = 0;
		} else if (sromrev >= 18) {
			pb = SROM18_PATH0;
			psz = SROM18_PATH1 - SROM18_PATH0;
			path_num = MAX_PATH_SROM_18;
		} else if (sromrev >= 13) {
			pb = SROM13_PATH0;
			psz = SROM13_PATH1 - SROM13_PATH0;
			path_num = MAX_PATH_SROM_13;
		} else if (sromrev >= 12) {
			pb = SROM12_PATH0;
			psz = SROM12_PATH1 - SROM12_PATH0;
			path_num = MAX_PATH_SROM_12;
		} else if (sromrev >= 11) {
			pb = SROM11_PATH0;
			psz = SROM11_PATH1 - SROM11_PATH0;
			path_num = MAX_PATH_SROM_11;
		} else if (sromrev >= 8) {
			pb = SROM8_PATH0;
			psz = SROM8_PATH1 - SROM8_PATH0;
			path_num = MAX_PATH_SROM;
		} else {
			pb = SROM4_PATH0;
			psz = SROM4_PATH1 - SROM4_PATH0;
			path_num = MAX_PATH_SROM;
		}

		for (p = 0; p < path_num; p++) {
			for (srv = perpath_pci_sromvars; srv->name != NULL; srv ++) {

				if ((srv->revmask & sr) == 0)
					continue;

				if (pb + srv->off < off)
					continue;

				/* This entry is for mfgc only. Don't generate param for it, */
				if (srv->flags & SRFL_NOVAR)
					continue;

				/* Start of an array */
				if (sromrev >= 10 && (srv->flags & SRFL_ARRAY) && !in_array) {
					array_curr = 0;
					array_name = (const char*)srv->name;
					memset((void*)array_temp, 0, sizeof(array_temp));
					in_array = TRUE;
				}

				if (sromrev >= 18 && (srv->off > LEGACY_PA_OFFSET_END) &&
					(srv->off < SROM18_PA_OFFSET_END)) {
					/* 2G20CCK and BW160 PA offset */
					if (p == 1) {	/* a1 */
						bw160_shift = SROM18_PATH1 - SROM18_PATH0;
					} else if (p == 2) {	/* a2 */
						bw160_shift = SROM18_PATH2 - SROM18_PATH0;
					} else if (p == 3) {	/* a3 */
						bw160_shift = SROM18_PATH3 - SROM18_PATH0;
					} else {	/* a0 */
						bw160_shift = 0;
					}
					if ((srv->off >= SROM18_5GB5_PA)) {
						bw160_antoff = (p * SROM18_6GEXT_PA_OFFSET_ANT);
					} else {
						bw160_antoff = (p * SROM18_PA_OFFSET_ANT);
					}
				} else if (sromrev >= 19 && (srv->off >= SROM18_PA_OFFSET_END)) {
					if (p == 1) {	/* a1 */
						bw160_shift = SROM18_PATH1 - SROM18_PATH0;
					} else if (p == 2) {	/* a2 */
						bw160_shift = SROM18_PATH2 - SROM18_PATH0;
					} else if (p == 3) {	/* a3 */
						bw160_shift = SROM18_PATH3 - SROM18_PATH0;
					} else {	/* a0 */
						bw160_shift = 0;
					}
					bw160_antoff = (p * SROM19_6G_PA_OFFSET_ANT);
				} else {
					bw160_shift = 0;
					bw160_antoff = 0;
				}

				ASSERT(pb + srv->off - off - bw160_shift + bw160_antoff > 0);
				w = srom[pb + srv->off - off - bw160_shift + bw160_antoff];

				ASSERT(mask_valid(srv->mask));
				val = (w & srv->mask) >> mask_shift(srv->mask);
				width = mask_width(srv->mask);

				flags = srv->flags;

				/* Cheating: no per-path var is more than 1 word */

				if ((srv->flags & SRFL_NOFFS) && ((int)val == (1 << width) - 1))
					continue;

				if (in_array) {
					int ret;

					if (flags & SRFL_PRHEX) {
						ret = snprintf(array_temp + array_curr,
						  sizeof(array_temp) - array_curr, "0x%x,", val);
					} else if ((flags & SRFL_PRSIGN) &&
						(val & (1 << (width - 1)))) {
						ret = snprintf(array_temp + array_curr,
							sizeof(array_temp) - array_curr, "%d,",
							(int)(val | (~0 << width)));
					} else {
						ret = snprintf(array_temp + array_curr,
						  sizeof(array_temp) - array_curr, "%u,", val);
					}

					if (ret > 0) {
						array_curr += ret;
					} else {
						BS_ERROR(
						("%s: array %s parsing error. buffer too short.\n",
						__FUNCTION__, array_name));
						ASSERT(0);

						/* buffer too small, skip this param */
						while (srv->flags & SRFL_ARRAY)
							srv ++;
						srv ++;
						in_array = FALSE;
						continue;
					}

					if (!(srv->flags & SRFL_ARRAY)) { /* Array ends */
						/* Remove the last ',' */
						array_temp[array_curr-1] = '\0';
						in_array = FALSE;
						varbuf_append(b, "%s%d=%s",
							array_name, p, array_temp);
					}
				} else if (srv->flags & SRFL_PRHEX)
					varbuf_append(b, "%s%d=0x%x", srv->name, p, val);
				else
					varbuf_append(b, "%s%d=%d", srv->name, p, val);
			}

			if (sromrev >= 13 && (p == (MAX_PATH_SROM_13 - 2))) {
				psz = SROM13_PATH3 - SROM13_PATH2;
			}

			pb += psz;
		}
	} /* per path variables */
} /* _initvars_srom_pci */

int
BCMATTACHFN(get_srom_pci_caldata_size)(uint32 sromrev)
{
	uint32 caldata_size;

	switch (sromrev) {
		case 15:
			caldata_size = (SROM15_CALDATA_WORDS * 2);
			break;
		case 17:
			caldata_size = (SROM17_CALDATA_WORDS * 2);
			break;
		default:
			caldata_size = 0;
			break;
	}

	return caldata_size;
}

uint32
BCMATTACHFN(get_srom_size)(uint32 sromrev)
{
	uint32 size;

	switch (sromrev) {
		case 15:
			size = (SROM15_WORDS * 2);
			break;
		case 17:
			size = (SROM17_WORDS * 2);
			break;
		default:
			size = 0;
			break;
	}

	return size;
}

int
BCMATTACHFN(_initvars_srom_pci_caldata)(si_t *sih, uint16 *srom, uint32 sromrev)
{
	int err = BCME_ERROR;

	if (si_is_sprom_available(sih)) {
		uint32 caldata_size;

		caldata_size = get_srom_pci_caldata_size(sromrev);
		memcpy(srom, caldata_array, caldata_size);
		err = BCME_OK;
	}

	return err;
}

#if defined(BCMHOSTVARS)
/**
 * If no OTP nor SPROM nvrams could be found, then as a last resort for early chip development,
 * in-driver default nvram values can be used.
 */
static void
BCMATTACHFN(initvars_defaultsrom)(si_t *sih, char *vp)
{
	if (BCM43684_CHIP(sih->chip)) {
		defvarslen = srom_vars_len(defaultsromvars_43684);
		bcopy(defaultsromvars_43684, vp, defvarslen);
	} else if (BCM6710_CHIP(sih->chip)) {
		defvarslen = srom_vars_len(defaultsromvars_6710);
		bcopy(defaultsromvars_6710, vp, defvarslen);
	} else if (BCM6715_CHIP(sih->chip)) {
		defvarslen = srom_vars_len(defaultsromvars_6715);
		bcopy(defaultsromvars_6715, vp, defvarslen);
	} else if (BCM6717_CHIP(sih->chip)) {
		defvarslen = srom_vars_len(defaultsromvars_6717);
		bcopy(defaultsromvars_6717, vp, defvarslen);
	} else if (BCM6726_CHIP(sih->chip)) { /* re-use 6717 NVRAM for now */
		defvarslen = srom_vars_len(defaultsromvars_6726);
		bcopy(defaultsromvars_6726, vp, defvarslen);
	} else if (BCM6711_CHIP(sih->chip)) {
		defvarslen = srom_vars_len(defaultsromvars_6711);
		bcopy(defaultsromvars_6711, vp, defvarslen);
	} else {
		defvarslen = srom_vars_len(defaultsromvars_wltest);
		bcopy(defaultsromvars_wltest, vp, defvarslen);
	}
}
#endif

/**
 * Initialize nonvolatile variable table from sprom, or OTP when SPROM is not available, or
 * optionally a set of 'defaultsromvars' (compiled-in) variables when both OTP and SPROM bear no
 * contents.
 *
 * On success, a buffer containing var/val pairs is allocated and returned in params vars and count.
 *
 * Return 0 on success, nonzero on error.
 *
 * Prerequisites:
 * - The CC core has been selected
 */
int
BCMATTACHFN(initvars_srom_pci)(si_t *sih, void *curmap, char **vars, uint *count)
{
	uint16 *srom;			/**< srom words, starting with the PCIe header */
	uint16 *sromwindow;
	uint8 sromrev = 0;
	uint32 sr;
	varbuf_t b;
	char *vp, *base = NULL;
#if !defined(BCMDONGLEHOST) && defined(CMWIFI) && defined(BCMEXTNVM)
	char *sromvars_map = NULL;
#endif /* !defined(BCMDONGLEHOST) && defined(CMWIFI) && defined(BCMEXTNVM) */
	osl_t *osh = si_osh(sih);
	bool flash = FALSE;
	int err = 0;
#if defined(BCMPCIEDEV_SROM_FORMAT) && defined(WLC_TXCAL)
	uint16 cal_wordoffset;
#endif

	/*
	 * Apply CRC over SROM content regardless SROM is present or not, and use variable
	 * <devpath>sromrev's existance in flash to decide if we should return an error when CRC
	 * fails or read SROM variables from flash.
	 */

	/* freed in same function */
	srom = MALLOC_NOPERSIST(osh, SROM_MAX);
	ASSERT(srom != NULL);
	if (!srom)
		return -2;

	sromwindow = (uint16 *)srom_offset(sih, curmap);
	if (si_is_sprom_available(sih)) {
		if (ISSIM_ENAB(sih) && (BCM43684_CHIP(sih->chip) ||
			BCM6710_CHIP(sih->chip) || BCM6715_CHIP(sih->chip) ||
			BCM6717_CHIP(sih->chip) || BCM6726_CHIP(sih->chip) ||
			BCM6711_CHIP(sih->chip))) {
			/* Skip srom read in Veloce to save time */
			BS_ERROR(("skip srom read in Veloce development\n"));
			err = 1;
		} else {
			err = sprom_read_pci(osh, sih, sromwindow, 0, srom, SROM_SIGN_MINWORDS + 1,
				FALSE);
		}

		if (err == 0) {
			if (srom[SROM20_SIGN] == SROM20_SIGNATURE) {
				err = sprom_read_pci(osh, sih, sromwindow,
						0, srom, SROM20_WORDS, TRUE);
				sromrev = srom[SROM20_CRCREV] & 0xff;
			} else if (srom[SROM19_SIGN] == SROM19_SIGNATURE) {
				err = sprom_read_pci(osh, sih, sromwindow,
						0, srom, SROM19_WORDS, TRUE);
				sromrev = srom[SROM19_CRCREV] & 0xff;
			} else if (srom[SROM18_SIGN] == SROM18_SIGNATURE) {
				err = sprom_read_pci(osh, sih, sromwindow,
						0, srom, SROM18_WORDS, TRUE);
				sromrev = srom[SROM18_CRCREV] & 0xff;
			} else if (srom[SROM17_SIGN] == SROM17_SIGNATURE) {
				err = sprom_read_pci(osh, sih, sromwindow,
					0, srom, SROM17_WORDS, TRUE);
				sromrev = srom[SROM17_CRCREV] & 0xff;
			} else if (srom[SROM16_SIGN] == SROM16_SIGNATURE) {
				err = sprom_read_pci(osh, sih, sromwindow,
						0, srom, SROM16_WORDS, TRUE);
				sromrev = srom[SROM16_CRCREV] & 0xff;
			} else if (srom[SROM15_SIGN] == SROM15_SIGNATURE) { /* srom 15  */
				err = sprom_read_pci(osh, sih, sromwindow,
						0, srom, SROM15_WORDS, TRUE);
				sromrev = srom[SROM15_CRCREV] & 0xff;
			} else if (srom[SROM11_SIGN] == SROM13_SIGNATURE) {
				err = sprom_read_pci(osh, sih, sromwindow,
						0, srom, SROM13_WORDS, TRUE);
				sromrev = srom[SROM13_CRCREV] & 0xff;
			} else if (srom[SROM11_SIGN] == SROM12_SIGNATURE) {
				err = sprom_read_pci(osh, sih, sromwindow,
						0, srom, SROM12_WORDS, TRUE);
				sromrev = srom[SROM12_CRCREV] & 0xff;
			} else if (srom[SROM11_SIGN] == SROM11_SIGNATURE) {
				err = sprom_read_pci(osh, sih, sromwindow,
						0, srom, SROM11_WORDS, TRUE);
				sromrev = srom[SROM11_CRCREV] & 0xff;
			} else  if ((srom[SROM4_SIGN] == SROM4_SIGNATURE) || /* srom 4  */
				(srom[SROM8_SIGN] == SROM4_SIGNATURE)) { /* srom 8,9  */
				err = sprom_read_pci(osh, sih, sromwindow,
						0, srom, SROM4_WORDS, TRUE);
				sromrev = srom[SROM4_CRCREV] & 0xff;
			} else {
				err = sprom_read_pci(osh, sih, sromwindow, 0,
						srom, SROM_WORDS, TRUE);
				if (err == 0) {
					/* srom is good and is rev < 4 */
					/* top word of sprom contains version and crc8 */
					sromrev = srom[SROM_CRCREV] & 0xff;
					/* bcm4401 sroms misprogrammed */
					if (sromrev == 0x10)
						sromrev = 1;
				}
			}

			if (err) {
				BS_ERROR(("srom read failed\n"));
			}
		} else {
			BS_ERROR(("srom read failed\n"));
		}
	}
#if defined(BCMNVRAMW) || defined(BCMNVRAMR)
	/* Use OTP if SPROM not available */
	else if ((err = otp_read_pci(osh, sih, srom, SROM_MAX)) == 0) {
		/* OTP only contain SROM rev8/rev9/rev10/Rev11/rev12/rev13/rev18 for now */
		ASSERT(srom[SROM19_SIGN] != SROM19_SIGNATURE);  /* SROM19 is too  large for OTP */
		ASSERT(srom[SROM20_SIGN] != SROM20_SIGNATURE);  /* SROM20 is too  large for OTP */
		if (srom[SROM18_SIGN] == SROM18_SIGNATURE)
			sromrev = srom[SROM18_CRCREV] & 0xff;
		else if (srom[SROM13_SIGN] == SROM13_SIGNATURE)
			sromrev = srom[SROM13_CRCREV] & 0xff;
		else if (srom[SROM12_SIGN] == SROM12_SIGNATURE)
			sromrev = srom[SROM12_CRCREV] & 0xff;
		else if (srom[SROM11_SIGN] == SROM11_SIGNATURE)
			sromrev = srom[SROM11_CRCREV] & 0xff;
		else if (srom[SROM10_SIGN] == SROM10_SIGNATURE)
			sromrev = srom[SROM10_CRCREV] & 0xff;
		else
			sromrev = srom[SROM4_CRCREV] & 0xff;
	}
#endif /* defined(BCMNVRAMW) || defined(BCMNVRAMR) */
	else {
		err = 1;
		BS_ERROR(("Neither SPROM nor OTP has valid image\n"));
	}

	BS_ERROR(("srom rev:%d\n", sromrev));

#if !defined(BCMDONGLEHOST) && defined(CMWIFI) && defined(BCMEXTNVM)
	if (err || (sromrev == 0)) {
		BS_ERROR(("wl:srom/otp not programmed, using external nvram file\n"));
		sromvars_map = (char *)MALLOCZ_NOPERSIST(osh, MAXSZ_NVRAM_VARS);
		if (sromvars_map == NULL) {
			BS_ERROR(("wl: Failed to allocate memory\n"));
			err = -2;
			goto errout;
		}

		/* read from vars file */
		if (cmwifi_sromvars_map(sih, sih->chip,
			(void *)sromvars_map, MAXSZ_NVRAM_VARS-1) == BCME_OK) {
			unsigned int len;
			base = vp = (char *)sromvars_map;
			len = srom_vars_len((char *)sromvars_map);
			vp += len;
			*vp++ = '\0';
			goto varsdone;
		}
	}
#endif /* !defined(BCMDONGLEHOST) && defined(CMWIFI) && defined(BCMEXTNVM) */

	/* We want internal/wltest driver to come up with default sromvars so we can
	 * program a blank SPROM/OTP.
	 */
	if (err || sromrev == 0) {
		char *value;
#if defined(BCMHOSTVARS)
		uint32 val;
#endif

		if ((value = si_getdevpathvar(sih, "sromrev"))) {
			sromrev = (uint8)bcm_strtoul(value, NULL, 0);
			flash = TRUE;
			goto varscont;
		}

		BS_ERROR(("%s, SROM CRC Error\n", __FUNCTION__));

#if !defined(DONGLEBUILD) || defined(BCMPCIEDEV_SROM_FORMAT)
		/* NIC build or PCIe FD using SROM format shouldn't load driver
		 * default when external nvram exists.
		 */
		if ((value = getvar(NULL, "sromrev"))) {
			BS_ERROR(("%s, Using external nvram\n", __FUNCTION__));
			err = 0;
			goto errout;
		}
#endif /* !DONGLEBUILD || BCMPCIEDEV_SROM_FORMAT */

/* BCMHOSTVARS is enabled only if WLTEST is enabled or BCMEXTNVM is enabled */
#if defined(BCMHOSTVARS)
		val = OSL_PCI_READ_CONFIG(osh, PCI_SPROM_CONTROL, sizeof(uint32));
		if ((si_is_sprom_available(sih) && srom[0] == 0xffff) ||
#ifdef BCMQT
			(si_is_sprom_available(sih) && sromrev == 0) ||
#endif
			(val & SPROM_OTPIN_USE)) {
			vp = base = mfgsromvars;

			if (defvarslen == 0) {
				BS_ERROR(("No nvm file, use generic default (for programming"
					" SPROM/OTP only)\n"));
				initvars_defaultsrom(sih, vp);
			} else {
				BS_ERROR(("Use nvm file as default\n"));
			}

			vp += defvarslen;
			/* add final null terminator */
			*vp++ = '\0';

			BS_ERROR(("Used %d bytes of defaultsromvars\n", defvarslen));
			goto varsdone;
		} else if (BCM43684_CHIP(sih->chip) ||
			BCM6710_CHIP(sih->chip) ||
			BCM6715_CHIP(sih->chip) ||
			BCM6717_CHIP(sih->chip) ||
			BCM6726_CHIP(sih->chip) ||
			BCM6711_CHIP(sih->chip)) {

			base = vp = mfgsromvars;

			BS_ERROR(("BOOT w/o SPROM or OTP\n"));

			if (defvarslen == 0) {
				BS_ERROR(("Use generic default SROM.\n"));
				initvars_defaultsrom(sih, vp);
			}
			vp += defvarslen;
			*vp++ = '\0';
			goto varsdone;
		} else
#endif
		{
			err = -1;
			goto errout;
		}
	}
#if defined(BCM_ONE_NVRAM_SRC)
	/* Discard hostvars if SROM parsing is successful, so only one nvram source
	 * will be used.
	 * Routers use combined srom/host nvram so shouldn't define BCM_ONE_NVRAM_SRC.
	 */
	else {
		nvram_exit((void *)sih); /* free up global vars */
	}
#endif /* BCM_ONE_NVRAM_SRC */

varscont:
	/* Bitmask for the sromrev */
	sr = 1 << sromrev;
	if ((sr & VALID_SROMREV) == 0) {
		BS_ERROR(("Invalid SROM rev %d\n", sromrev));
		err = -2;
		goto errout;
	}

	ASSERT(vars != NULL);
	ASSERT(count != NULL);

	/* freed in same function */
	base = vp = MALLOC_NOPERSIST(osh, MAXSZ_NVRAM_VARS);
	ASSERT(vp != NULL);
	if (!vp) {
		err = -2;
		goto errout;
	}

	/* read variables from flash */
	if (flash) {
		if ((err = initvars_only_prefixed(sih, osh, &vp, MAXSZ_NVRAM_VARS)))
			goto errout;
		goto varsdone;
	}

	varbuf_init(&b, base, MAXSZ_NVRAM_VARS);

	/* parse SROM into name=value pairs. */
	_initvars_srom_pci(sromrev, srom, 0, &b);

#if defined(DONGLEBUILD) && defined(BCMHOSTVARS) && defined(BCMHOSTVARS_OVERRIDE_SROM)
	if ((_varsz != 0) && (_vars != NULL)) {
		int i = 0, off = 0, len;

		BS_ERROR(("%s: override SROM by provisioned vars from host(size %d).\n",
			__FUNCTION__, _varsz));
		for (; (len = strlen(_vars + off)) > 0 && (off < _varsz); off += len + 1, i++) {
			varbuf_append(&b, "%s", _vars + off);
		}
		BS_ERROR(("%s: overrode SROM by %d host vars.\n", __FUNCTION__, i));
	}
#endif /* DONGLEBUILD && BCMHOSTVARS && BCMHOSTVARS_OVERRIDE_SROM */

	/* final nullbyte terminator */
	ASSERT(b.size >= 1);
	vp = b.buf;
#if !defined(DONGLEBUILD)
	if (!flash_nvram_present(sih, osh, &vp)) {
		*vp++ = '\0';
	}
#else
	*vp++ = '\0';
#endif

	ASSERT((vp - base) <= MAXSZ_NVRAM_VARS);

varsdone:
	err = initvars_table(osh, base, vp, vars, count); /* allocates buffer in 'vars' */

#if defined(BCMPCIEDEV_SROM_FORMAT) && defined(WLC_TXCAL)
	if (sromrev == 16) {
		int caldata_wordoffset = srom[SROM16_CALDATA_OFFSET_LOC] / 2;

		if ((caldata_wordoffset != 0) &&
			(caldata_wordoffset + SROM_CALDATA_WORDS < SROM16_WORDS)) {
			memcpy(caldata_array, srom + caldata_wordoffset, SROM_CALDATA_WORDS * 2);
			is_caldata_prsnt = TRUE;
		}
	}
#endif /* defined(BCMPCIEDEV_SROM_FORMAT) && defined(WLC_TXCAL) */

#if defined(BCMPCIEDEV_SROM_FORMAT) && defined(WLC_TXCAL)
	if ((sromrev == 15) || (sromrev == 17)) {
		uint32 caldata_size = get_srom_pci_caldata_size(sromrev);

		cal_wordoffset = getintvar(NULL, "caldata_offset")/2;
		memcpy(caldata_array, srom + cal_wordoffset, caldata_size);
	}
#endif

errout:
/* BCMHOSTVARS are enabled only if WLTEST is enabled or BCMEXTNVM is enabled */
#if defined(BCMHOSTVARS)
	if (base && (base != mfgsromvars))
#else
	if (base)
#endif
		MFREE(osh, base, MAXSZ_NVRAM_VARS);

	MFREE(osh, srom, SROM_MAX);

	return err;
} /* initvars_srom_pci */

/**
 * initvars_cis_pci() parses OTP CIS. This is specifically for PCIe full dongle that has SROM
 * header plus CIS tuples programmed in OTP.
 * Return error if the content is not in CIS format or OTP is not present.
 */
static int
BCMATTACHFN(initvars_cis_pci)(si_t *sih, osl_t *osh, void *curmap, char **vars, uint *count)
{
	uint wsz = 0, sz = 0, base_len = 0;
	void *oh = NULL;
	int rc = BCME_OK;
	uint16 *cisbuf = NULL;
	uint8 *cis = NULL;
#if defined(BCMHOSTVARS)
	char *vp = NULL;
#endif
	char *base = NULL;
	bool wasup;
	uint32 min_res_mask = 0;
	BCM_REFERENCE(curmap);

	/* Bail out if we've dealt with OTP/SPROM before! */
	if (sih->srvars_inited)
		goto exit;

	/* Turn on OTP if it's not already on */
	if (!(wasup = si_is_otp_powered(sih)))
		si_otp_power(sih, TRUE, &min_res_mask);

	if (si_cis_source(sih) != CIS_OTP)
		rc = BCME_NOTFOUND;
	else if ((oh = otp_init(sih)) == NULL)
		rc = BCME_ERROR;
	else if (!(((BUSCORETYPE(sih->buscoretype) == PCIE2_CORE_ID) || otp_newcis(oh)) &&
		(otp_status(oh) & OTPS_GUP_HW))) {
		/* OTP bit CIS format (507) not used by pcie core - only needed for sdio core */
		rc = BCME_NOTFOUND;
	} else if ((sz = otp_size(oh)) != 0) {
		if ((cisbuf = (uint16*)MALLOC_NOPERSIST(osh, sz))) {
			/* otp_size() returns bytes, not words. */
			wsz = sz >> 1;
			rc = otp_read_region(sih, OTP_HW_RGN, cisbuf, &wsz);

			/* Bypass the HW header and signature */
			cis = (uint8*)(cisbuf + (otp_pcie_hwhdr_sz(sih) / 2));
			BS_ERROR(("%s: Parsing CIS in OTP.\n", __FUNCTION__));
		} else
			rc = BCME_NOMEM;
	}

	/* Restore original OTP state */
	if (!wasup)
		si_otp_power(sih, FALSE, &min_res_mask);

	if (rc != BCME_OK) {
		BS_ERROR(("%s: Not CIS format\n", __FUNCTION__));
		goto exit;
	}

#if defined(BCMHOSTVARS)
	if (defvarslen) {
		vp = mfgsromvars;
		vp += defvarslen;

		/* allocates buffer in 'vars' */
		rc = initvars_table(osh, mfgsromvars, vp, &base, &base_len);
		if (rc)
			goto exit;

		*vars = base;
		*count = base_len;

		BS_ERROR(("%s external nvram %d bytes\n", __FUNCTION__, defvarslen));
	}

#endif

	/* Parse the CIS and allocate a(nother) buffer in 'vars' */
	rc = srom_parsecis(sih, osh, &cis, SROM_CIS_SINGLE, vars, count);

	sih->srvars_inited = TRUE;
exit:
	/* Clean up */
	if (base)
		MFREE(osh, base, base_len);
	if (cisbuf)
		MFREE(osh, cisbuf, sz);

	/* return OK so the driver will load & use defaults if bad srom/otp */
	return rc;
} /* initvars_cis_pci */

#if !defined(DONGLEBUILD)
/**
 * For OTP/SROM + nvram on NIC mode driver
 * If prefixed nvram variable exist on FLASH
 * Remove prefix and append nvram variable to vars
 * Finally nvram variable will be appended to wlc->pub->vars
 */
static bool
BCMATTACHFN(flash_nvram_present)(si_t *sih, osl_t *osh, char **vars)
{
	char *value;
	uint32 sr;
	uint8 sromrev = 0;

	if ((value = si_getdevpathvar(sih, "sromrev"))) {
		sromrev = (uint8)bcm_strtoul(value, NULL, 0);
		sr = 1 << sromrev;
		if ((sr & VALID_SROMREV) != 0) {
			if ((initvars_only_prefixed(sih, osh, vars, MAXSZ_NVRAM_VARS)) == BCME_OK) {
				return TRUE;
			}
		}
	}
	return FALSE;
}
#endif /* !DONGLEBUILD */
#endif /* !defined(BCMDONGLEHOST) */

#if !defined(BCMDONGLEHOST)
#ifdef BCMSPI
/**
 * Read the SPI cis and call parsecis to allocate and initialize the NVRAM vars buffer.
 * Return 0 on success, nonzero on error.
 */
static int
BCMATTACHFN(initvars_cis_spi)(si_t *sih, osl_t *osh, char **vars, uint *count)
{
	uint8 *cis;
	int rc;

	/* freed in same function */
	if ((cis = MALLOC_NOPERSIST(osh, SBSDIO_CIS_SIZE_LIMIT)) == NULL) {
		return -1;
	}

	bzero(cis, SBSDIO_CIS_SIZE_LIMIT);

	if (bcmsdh_cis_read(NULL, SDIO_FUNC_1, cis, SBSDIO_CIS_SIZE_LIMIT) != 0) {
		MFREE(osh, cis, SBSDIO_CIS_SIZE_LIMIT);
		return -2;
	}

	rc = srom_parsecis(sih, osh, &cis, SDIO_FUNC_1, vars, count);

	MFREE(osh, cis, SBSDIO_CIS_SIZE_LIMIT);

	return (rc);
}
#endif /* BCMSPI */
#endif /* !defined(BCMDONGLEHOST) */

/** Return a fixed sprom size of 258 [bytes] */
uint
srom_size(si_t *sih, osl_t *osh)
{
	uint size = 0;

	size = (SROM16_SIGN + 1) * sizeof(uint16); /* must big enough for SROM16 */

	return size;
}

#if defined(BCMPCIEDEV_ENABLED)

static int
BCMATTACHFN(initvars_srom_si_pciedev)(si_t *sih, osl_t *osh, void *curmap, char **vars, uint *varsz)
{
#ifdef BCM_DONGLEVARS
	void *oh = NULL;
	uint8 *cis;
	uint sz = 0;
	int rc;

	if (si_cis_source(sih) !=  CIS_OTP)
		return BCME_OK;

	if (((oh = otp_init(sih)) != NULL) && (otp_status(oh) & OTPS_GUP_HW))
		sz = otp_size(oh);
	if (sz == 0)
		return BCME_OK;

	/* nvram data is freed in si_detach */
	if ((cis = MALLOC_NOPERSIST(osh, sz)) == NULL)
		return BCME_NOMEM;
	sz >>= 1;
	rc = otp_read_region(sih, OTP_HW_RGN, (uint16 *)cis, &sz);
	sz <<= 1;

	/* account for the Hardware header */
	if (sz == otp_pcie_hwhdr_sz(sih))
		return BCME_OK;

	cis += otp_pcie_hwhdr_sz(sih);

	if (*(uint16 *)cis == SROM11_SIGNATURE) {
		return BCME_OK;
	}

	if ((rc = srom_parsecis(sih, osh, &cis, SROM_CIS_SINGLE, vars, varsz)) == BCME_OK)
		nvram_append((void *)sih, *vars, *varsz);

	return rc;
#else /* BCM_DONGLEVARS */
	*vars = NULL;
	*varsz = 0;
	return BCME_OK;
#endif /* BCM_DONGLEVARS */
} /* initvars_srom_si_pciedev */

#endif	/* BCMPCIEDEV_ENABLED */

void
BCMATTACHFN(srom_var_deinit)(si_t *sih)
{
	BCM_REFERENCE(sih);

	sih->srvars_inited = FALSE;
}

#if defined(BCMPCIEDEV_SROM_FORMAT) && defined(WLC_TXCAL)
bool
BCMATTACHFN(srom_caldata_prsnt)(si_t *sih)
{
	return is_caldata_prsnt;
}

int
BCMATTACHFN(srom_get_caldata)(si_t *sih, uint16 *srom)
{
	if (!is_caldata_prsnt) {
		return BCME_ERROR;
	}

	memcpy(srom, caldata_array, SROM_CALDATA_WORDS * 2);
	return BCME_OK;
}
#endif /* defined(BCMPCIEDEV_SROM_FORMAT) && defined(WLC_TXCAL) */

enum nvram_line_devpath_type_e
srom_get_nvram_line_devpath_type(const char *nvram_line)
{
	if (strlen(nvram_line) < 2) {
		return NVRAM_DEVPATH_NONE;
	}

	if (strncmp(nvram_line, DEVPATH_SB, sizeof(DEVPATH_SB) - 1) == 0) {
		return NVRAM_DEVPATH_SB;
	}

	if (strncmp(nvram_line, DEVPATH_PCIE, sizeof(DEVPATH_PCIE) - 1) == 0) {
		return NVRAM_DEVPATH_PCIE;
	}

	/**
	 * With compact devpaths, the nvram file contains a line such as:
	 *    devpath0=pci/1/1/
	 * Following that, there can be multiple lines that reference that devpath, like:
	 *    0:venid=0x14e4
	 *    0:tri2g=0x0
	 */
	if (nvram_line[0] >= '0' && nvram_line[0] <= '9' && nvram_line[1] == ':') {
		return TRUE;
	}

	return NVRAM_DEVPATH_NONE;
}
