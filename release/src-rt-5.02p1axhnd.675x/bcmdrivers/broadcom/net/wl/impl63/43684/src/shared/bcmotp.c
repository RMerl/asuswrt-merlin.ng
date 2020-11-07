/*
 * OTP support.
 *
 * Copyright (C) 2020, Broadcom. All Rights Reserved.
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
 * $Id: bcmotp.c 788953 2020-07-15 07:52:39Z $
 */

#include <bcm_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmdevs.h>
#include <bcmutils.h>
#include <siutils.h>
#include <bcmendian.h>
#include <hndsoc.h>
#include <sbchipc.h>
#include <sbgci.h>
#include <bcmotp.h>
#ifdef EVENT_LOG_COMPILE
#include <event_log.h>
#endif // endif
#include <pcicfg.h>
#if (!defined(_CFE_) && !defined(_CFEZ_)) || defined(CFG_WL)
#include <wlioctl.h>
#endif /* (!_CFE_ && !_CFEZ_) || CFG_WL */

/*
 * There are two different OTP controllers so far:
 * 	1. new IPX OTP controller:	chipc 21, >=23
 * 	2. older HND OTP controller:	chipc 12, 17, 22
 *
 * Define BCMHNDOTP to include support for the HND OTP controller.
 * Define BCMIPXOTP to include support for the IPX OTP controller.
 *
 * NOTE 1: More than one may be defined
 * NOTE 2: If none are defined, the default is to include them all.
 */

#if !defined(BCMHNDOTP) && !defined(BCMIPXOTP)
#define BCMHNDOTP	1
#define BCMIPXOTP	1
#endif // endif

#define OTPTYPE_HND(ccrev)	((ccrev) < 21 || (ccrev) == 22)
#define OTPTYPE_IPX(ccrev)	((ccrev) == 21 || (ccrev) >= 23)
#define OTPPROWMASK(ccrev)	((ccrev >= 49) ? OTPP_ROW_MASK9 : OTPP_ROW_MASK)

/* XXX debug/trace
 * ?? To enable ERR by default
 * warning: in the dongles, the printf could be very costly and slow,
 *   in BMAC dongle, print big msg can even lead to BMAC rpc timeout
 *
 */
#define OTP_ERR_VAL	0x0001
#define OTP_MSG_VAL	0x0002
#define OTP_DBG_VAL	0x0004
uint32 otp_msg_level = OTP_ERR_VAL;

#if defined(EVENT_LOG_COMPILE) && defined(BCMDBG_ERR) && defined(ERR_USE_EVENT_LOG)
#if defined(ERR_USE_EVENT_LOG_RA)
#define	OTP_ERR(args)		do { \
				if (otp_msg_level & OTP_ERR_VAL) { \
				EVENT_LOG_RA(EVENT_LOG_TAG_OTP_ERROR, args); }\
				} while (0);
#else
#define	OTP_ERR(args)		do { \
				if (otp_msg_level & OTP_ERR_VAL) { \
				EVENT_LOG_COMPACT_CAST_PAREN_ARGS(EVENT_LOG_TAG_OTP_ERROR, args); }\
				} while (0);
#endif /* ERR_USE_EVENT_LOG_RA */
#elif defined(BCMDBG) || defined(BCMDBG_ERR)
#define OTP_ERR(args)	do {if (otp_msg_level & OTP_ERR_VAL) printf args;} while (0)
#else
#define OTP_ERR(args)
#endif /* defined(BCMDBG_ERR) && defined(ERR_USE_EVENT_LOG) */

#ifdef BCMDBG
#define OTP_MSG(args)	do {if (otp_msg_level & OTP_MSG_VAL) printf args;} while (0)
#define OTP_DBG(args)	do {if (otp_msg_level & OTP_DBG_VAL) printf args;} while (0)
#else
#define OTP_MSG(args)
#define OTP_DBG(args)
#endif // endif

#define OTPP_TRIES		10000000	/* # of tries for OTPP */
#define OTP_FUSES_PER_BIT	2
#define OTP_WRITE_RETRY		16

#ifdef BCMIPXOTP
#define MAXNUMRDES		9		/* Maximum OTP redundancy entries */
#endif // endif

/* OTP common registers */
typedef struct {
	volatile uint32	*otpstatus;
	volatile uint32	*otpcontrol;
	volatile uint32	*otpprog;
	volatile uint32	*otplayout;
	volatile uint32	*otpcontrol1;
	volatile uint32	*otplayoutextension;
	volatile uint32	*otpprogdata;
	volatile uint32	*otpECCstatus;
} otpregs_t;

/* OTP common function type */
typedef int	(*otp_status_t)(void *oh);
typedef int	(*otp_size_t)(void *oh);
typedef void*	(*otp_init_t)(si_t *sih);
typedef uint16	(*otp_read_bit_t)(void *oh, chipcregs_t *cc, uint off);
typedef int	(*otp_read_region_t)(si_t *sih, int region, uint16 *data, uint *wlen);
typedef int	(*otp_nvread_t)(void *oh, char *data, uint *len);
typedef int	(*otp_write_region_t)(void *oh, int region, uint16 *data, uint wlen, uint flags);
typedef int	(*otp_cis_append_region_t)(si_t *sih, int region, char *vars, int count);
typedef int	(*otp_lock_t)(si_t *sih);
typedef int	(*otp_nvwrite_t)(void *oh, uint16 *data, uint wlen);
typedef int	(*otp_dump_t)(void *oh, int arg, char *buf, uint size);
typedef int	(*otp_write_word_t)(void *oh, uint wn, uint16 data);
typedef int	(*otp_read_word_t)(void *oh, uint wn, uint16 *data);
typedef int (*otp_write_bits_t)(void *oh, int bn, int bits, uint8* data);
typedef bool	(*otp_isunified_t)(void *oh);
typedef uint16	(*otp_avsbitslen_t)(void *oh);
typedef int (*otp_ecc_write_t)(void *oh, uint wn, uint32 data, uint32 ecc_type);
typedef int	(*otp_read_t)(void *oh, void *arg, char *buf, uint size);
typedef int	(*otp_ecc_rows_dump_t)(void *oh, int *arg, char *buf, uint size);

/* OTP function struct */
typedef struct otp_fn_s {
	otp_size_t		size;
	otp_read_bit_t		read_bit;
	otp_dump_t		dump;
	otp_status_t		status;

	otp_init_t		init;
	otp_read_region_t	read_region;
	otp_nvread_t		nvread;
	otp_write_region_t	write_region;
	otp_cis_append_region_t	cis_append_region;
	otp_lock_t		lock;
	otp_nvwrite_t		nvwrite;
	otp_write_word_t	write_word;
	otp_read_word_t		read_word;
	otp_write_bits_t	write_bits;
	otp_ecc_write_t		ecc_write;
	otp_ecc_rows_dump_t	ecc_rows_dump;
	otp_isunified_t		isunified;
	otp_avsbitslen_t	avsbitslen;
	otp_read_t		read;
} otp_fn_t;

typedef struct {
	uint		ccrev;		/* chipc revision */
	otp_fn_t	*fn;		/* OTP functions */
	si_t		*sih;		/* Saved sb handle */
	osl_t		*osh;

#ifdef BCMIPXOTP
	/* IPX OTP section */
	uint16		wsize;		/* Size of otp in words */
	uint16		rows;		/* Geometry */
	uint16		cols;		/* Geometry */
	uint32		status;		/* Flag bits (lock/prog/rv).
					 * (Reflected only when OTP is power cycled)
					 */
	uint16		hwbase;		/* hardware subregion offset */
	uint16		hwlim;		/* hardware subregion boundary */
	uint16		swbase;		/* software subregion offset */
	uint16		swlim;		/* software subregion boundary */
	uint16		fbase;		/* fuse subregion offset */
	uint16		flim;		/* fuse subregion boundary */
	int		otpgu_base;	/* offset to General Use Region */
	uint16		fusebits;	/* num of fusebits */
	bool 		buotp; 		/* Uinified OTP flag */
	uint 		usbmanfid_offset; /* Offset of the usb manfid inside the sdio CIS */
	struct {
		uint8 width;		/* entry width in bits */
		uint8 val_shift;	/* value bit offset in the entry */
		uint8 offsets;		/* # entries */
		uint8 stat_shift;	/* valid bit in otpstatus */
		uint16 offset[MAXNUMRDES];	/* entry offset in OTP */
	} rde_cb;			/* OTP redundancy control blocks */
	uint16		rde_idx;
#endif /* BCMIPXOTP */

#ifdef BCMHNDOTP
	/* HND OTP section */
	uint		size;		/* Size of otp in bytes */
	uint		hwprot;		/* Hardware protection bits */
	uint		signvalid;	/* Signature valid bits */
	int		boundary;	/* hw/sw boundary */
#endif /* BCMHNDOTP */
	volatile uint16	*otpbase;	/* Cache OTP Base address */
	uint16		avsbitslen;	/* Number of bits used for AVS in sw region */
} otpinfo_t;

static otpinfo_t otpinfo;

/*
 * ROM accessor to avoid struct in shdat. Declare BCMRAMFN() to force the accessor to be excluded
 * from ROM.
 */
static otpinfo_t *
BCMRAMFN(get_otpinfo)(void)
{
	return (otpinfo_t *)&otpinfo;
}

static void
otp_initregs(si_t *sih, volatile void *coreregs, otpregs_t *otpregs)
{
	if (AOB_ENAB(sih)) {
		gciregs_t *gciregs = (gciregs_t *)coreregs;

		otpregs->otpstatus = &gciregs->otpstatus;
		otpregs->otpcontrol = &gciregs->otpcontrol;
		otpregs->otpprog = &gciregs->otpprog;
		otpregs->otplayout = &gciregs->otplayout;
		otpregs->otpcontrol1 = &gciregs->otpcontrol1;
		otpregs->otplayoutextension = &gciregs->otplayoutextension;
		otpregs->otpprogdata = &gciregs->otpprogdata;
		otpregs->otpECCstatus = &gciregs->otpECCstatus;
	} else {
		chipcregs_t *ccregs = (chipcregs_t *)coreregs;

		otpregs->otpstatus = &ccregs->otpstatus;
		otpregs->otpcontrol = &ccregs->otpcontrol;
		otpregs->otpprog = &ccregs->otpprog;
		otpregs->otplayout = &ccregs->otplayout;
		otpregs->otpcontrol1 = &ccregs->otpcontrol1;
		otpregs->otplayoutextension = &ccregs->otplayoutextension;
		otpregs->otpprogdata = NULL;
		otpregs->otpECCstatus = NULL;
	}
}

/*
 * IPX OTP Code
 *
 *   Exported functions:
 *	ipxotp_status()
 *	ipxotp_size()
 *	ipxotp_init()
 *	ipxotp_read_bit()
 *	ipxotp_read_region()
 *	ipxotp_read_word()
 *	ipxotp_nvread()
 *	ipxotp_write_region()
 *	ipxotp_write_word()
 *	ipxotp_cis_append_region()
 *	ipxotp_lock()
 *	ipxotp_nvwrite()
 *	ipxotp_dump()
 *	ipxotp_isunified()
 *	ipxotp_avsbitslen()
 *
 *   IPX internal functions:
 *	ipxotp_otpr()
 *	_ipxotp_init()
 *	_ipxotp_read_bit()
 *	ipxotp_write_bit()
 *	ipxotp_otpwb16()
 *	ipxotp_check_otp_pmu_res()
 *	ipxotp_write_rde()
 *	ipxotp_fix_word16()
 *	ipxotp_check_word16()
 *	ipxotp_max_rgnsz()
 *	ipxotp_otprb16()
 *	ipxotp_uotp_usbmanfid_offset()
 *
 */

#ifdef BCMIPXOTP

#define	OTPWSIZE		16	/* word size */
#define HWSW_RGN(rgn)		(((rgn) == OTP_HW_RGN) ? "h/w" : "s/w")

/* OTP layout */
/* CC revs 21, 24 and 27 OTP General Use Region word offset */
#define REVA4_OTPGU_BASE	12

/* CC revs 23, 25, 26, 28 and above OTP General Use Region word offset */
#define REVB8_OTPGU_BASE	20

/* CC rev 36 OTP General Use Region word offset */
#define REV36_OTPGU_BASE	12

/* Subregion word offsets in General Use region */
#define OTPGU_HSB_OFF		0
#define OTPGU_SFB_OFF		1
#define OTPGU_CI_OFF		2
#define OTPGU_P_OFF		3
#define OTPGU_SROM_OFF		4

/* Flag bit offsets in General Use region  */
#define OTPGU_NEWCISFORMAT_OFF	59
#define OTPGU_HWP_OFF		60
#define OTPGU_SWP_OFF		61
#define OTPGU_CIP_OFF		62
#define OTPGU_FUSEP_OFF		63
#define OTPGU_CIP_MSK		0x4000
#define OTPGU_P_MSK		0xf000
#define OTPGU_P_SHIFT		(OTPGU_HWP_OFF % 16)

/* LOCK but offset */
#define OTP_LOCK_ROW1_LOC_OFF	63	/* 1st ROW lock bit */
#define OTP_LOCK_ROW2_LOC_OFF	127	/* 2nd ROW lock bit */
#define OTP_LOCK_RD_LOC_OFF	128	/* Redundnancy Region lock bit */
#define OTP_LOCK_GU_LOC_OFF	129	/* General User Region lock bit */

/* OTP Fuse Size */
#define OTP_SZ_FU_592		((ROUNDUP(592, 16))/8)
#define OTP_SZ_FU_2576          ((ROUNDUP(2576, 16))/8)
#define OTP_SZ_FU_1808          ((ROUNDUP(1808, 16))/8)
#define OTP_SZ_FU_1296          ((ROUNDUP(1296, 16))/8)
#define OTP_SZ_FU_1048          ((ROUNDUP(1048, 16))/8)
#define OTP_SZ_FU_972		((ROUNDUP(972, 16))/8)
#define OTP_SZ_FU_720		((ROUNDUP(720, 16))/8)
#define OTP_SZ_FU_468		((ROUNDUP(468, 16))/8)
#define OTP_SZ_FU_608		((ROUNDUP(608, 16))/8)
#define OTP_SZ_FU_576		((ROUNDUP(576, 16))/8)
#define OTP_SZ_FU_324		((ROUNDUP(324,8))/8)	/* 324 bits */
#define OTP_SZ_FU_792		(792/8)		/* 792 bits */
#define OTP_SZ_FU_288		(288/8)		/* 288 bits */
#define OTP_SZ_FU_216		(216/8)		/* 216 bits */
#define OTP_SZ_FU_72		(72/8)		/* 72 bits */
#define OTP_SZ_CHECKSUM		(16/8)		/* 16 bits */
#define OTP4315_SWREG_SZ	178		/* 178 bytes */
#define OTP_SZ_FU_144		(144/8)		/* 144 bits */
#define OTP_SZ_FU_180		((ROUNDUP(180,8))/8)	/* 180 bits */
#define OTP_SZ_FU_1044		((ROUNDUP(1044, 16))/8)  /* 1044 bits */
#define OTP_SZ_FU_1080		((ROUNDUP(1080, 16))/8)  /* 1080 bits */

/* OTP BT shared region (pre-allocated) */
#define OTP_BT_BASE_4350	(4384/OTPWSIZE)
#define OTP_BT_END_4350		(5408/OTPWSIZE)
#define OTP_BT_BASE_4347	(7712/OTPWSIZE)
#define OTP_BT_END_4347		(11808/OTPWSIZE)
#define OTP_BT_BASE_4364	(11216/OTPWSIZE)
#define OTP_BT_END_4364		(15312/OTPWSIZE)
#define OTP_BT_BASE_4369	(7712/OTPWSIZE)
#define OTP_BT_END_4369		(11808/OTPWSIZE)

/* AVS Regions Size */
#define AVS_BITS_LEN	32

/* OTP size read from OTPLayout Register 0x1c */
#define OTPSZ_28NM_5		5	/* Size 192x32: 6144 bits */
#define OTPSZ_28NM_5_ROWS	192
#define OTPSZ_28NM_5_COLS	32

#define OTPSZ_28NM_11		0xB	/* Size 384x32: 12288 bits */
#define OTPSZ_28NM_11_ROWS	384
#define OTPSZ_28NM_11_COLS	32

#define OTPSZ_28NM_15		0xF	/* Size 512x32: 16384 bits */
#define OTPSZ_28NM_15_ROWS	512
#define OTPSZ_28NM_15_COLS	32

/* Used for Enabing OTP Programming */
#define OTP_PROGEN_SEQCOUNT	4
#define OTP_PROGEN_IDX1		15
#define OTP_PROGEN_IDX2		4
#define OTP_PROGEN_IDX3		8
#define OTP_PROGEN_IDX4		13

/* OTP PCIE HW Header Size */
#define OTP_PCIE_HWHDR_SZ_CONV		128
#define OTP_PCIE_HWHDR_SZ_COREREV19	208
#define OTP_PCIE_HWHDR_SZ_COREREV26	224	/* 43684/6710 PCIE HW Header Size */

/* OTP unification */
#if defined(USBSDIOUNIFIEDOTP)
/** Offset in OTP from upper GUR to HNBU_UMANFID tuple value in (16-bit) words */
#define USB_MANIFID_OFFSET_4319		42
#endif /* USBSDIOUNIFIEDOTP */

#if defined(BCMNVRAMW)
/* Local */
static int ipxotp_check_otp_pmu_res(otpinfo_t *oi);
static int ipxotp_write_bit(otpinfo_t *oi, otpregs_t *otpregs, uint off);
static int ipxotp40n_read2x(void *oh, otpregs_t *otpregs, uint off);
static int ipxotp_write_rde_nopc(void *oh, otpregs_t *otpregs, int rde, uint bit, uint val);
#endif // endif

static otp_fn_t* get_ipxotp_fn(void);

static int
BCMCISDUMPATTACHFN(ipxotp_status)(void *oh)
{
	otpinfo_t *oi = (otpinfo_t *)oh;
	return (int)(oi->status);
}

/** Returns number of bits used for avs at the end of sw region */
static uint16
ipxotp_avsbitslen(void *oh)
{
	otpinfo_t *oi = (otpinfo_t *)oh;
	return oi->avsbitslen;
}

/** Returns if otp is unified */
static bool
ipxotp_isunified(void *oh)
{
	otpinfo_t *oi = (otpinfo_t *)oh;
	return oi->buotp;
}

/** Returns size in bytes */
static int
ipxotp_size(void *oh)
{
	otpinfo_t *oi = (otpinfo_t *)oh;
	return (int)oi->wsize * 2;
}

static uint16
BCMSROMCISDUMPATTACHFN(ipxotp_read_bit_common)(void *oh, otpregs_t *otpregs, uint off)
{
	otpinfo_t *oi = (otpinfo_t *)oh;
	uint k, row, col;
	uint32 otpp, st;
	uint otpwt;

	otpwt = (R_REG(oi->osh, otpregs->otplayout) & OTPL_WRAP_TYPE_MASK) >> OTPL_WRAP_TYPE_SHIFT;
	BCM_REFERENCE(otpwt);

	row = off / oi->cols;
	col = off % oi->cols;

	otpp = OTPP_START_BUSY |
		((((OTPWRTYPE(otpwt) == OTPL_WRAP_TYPE_40NM)? OTPPOC_READ_40NM :
		OTPPOC_READ) << OTPP_OC_SHIFT) & OTPP_OC_MASK) |
	        ((row << OTPP_ROW_SHIFT) & OTPPROWMASK(CCREV(oi->sih->ccrev))) |
	        ((col << OTPP_COL_SHIFT) & OTPP_COL_MASK);
	OTP_DBG(("%s: off = %d, row = %d, col = %d, otpp = 0x%x",
	         __FUNCTION__, off, row, col, otpp));
	W_REG(oi->osh, otpregs->otpprog, otpp);

	for (k = 0;
	     ((st = R_REG(oi->osh, otpregs->otpprog)) & OTPP_START_BUSY) && (k < OTPP_TRIES);
	     k ++)
		;
	if (k >= OTPP_TRIES) {
		OTP_ERR(("\n%s: BUSY stuck: st=0x%x, count=%d\n", __FUNCTION__, st, k));
		return 0xffff;
	}
	if (st & OTPP_READERR) {
		OTP_ERR(("\n%s: Could not read OTP bit %d\n", __FUNCTION__, off));
		return 0xffff;
	}
	st = (st & OTPP_VALUE_MASK) >> OTPP_VALUE_SHIFT;

	OTP_DBG((" => %d\n", st));
	return (int)st;
}

static uint16
BCMSROMCISDUMPATTACHFN(_ipxotp_read_bit)(void *oh, otpregs_t *otpregs, uint off)
{
	otpinfo_t *oi = (otpinfo_t *)oh;
	uint otpwt = 0;

	otpwt = (R_REG(oi->osh, otpregs->otplayout) & OTPL_WRAP_TYPE_MASK) >> OTPL_WRAP_TYPE_SHIFT;
	BCM_REFERENCE(otpwt);
	/* 28nm Chips doesnot have otpcontrol1 register access ,
	* so skip this register access alltogether
	*/
	if (OTPWRTYPE(otpwt) != OTPL_WRAP_TYPE_28NM) {
		W_REG(oi->osh, otpregs->otpcontrol, 0);

		if (CCREV(oi->sih->ccrev) >= 49) {
			AND_REG(oi->osh, otpregs->otpcontrol1,
				(OTPC1_CLK_EN_MASK | OTPC1_CLK_DIV_MASK));
		} else {
			W_REG(oi->osh, otpregs->otpcontrol1, 0);
		}
	}
	return ipxotp_read_bit_common(oh, otpregs, off);
}

static uint16
BCMCISDUMPATTACHFN(ipxotp_read_bit)(void *oh, chipcregs_t *cc, uint off)
{
	otpinfo_t *oi = (otpinfo_t *)oh;
	si_t *sih = oi->sih;
	uint idx;
	volatile void *regs;
	otpregs_t otpregs;
	uint16 val16;
	BCM_REFERENCE(cc);

	idx = si_coreidx(sih);
	if (AOB_ENAB(sih)) {
		regs = si_setcore(sih, GCI_CORE_ID, 0);
	} else {
		regs = si_setcoreidx(sih, SI_CC_IDX);
	}
	ASSERT(regs != NULL);
	otp_initregs(sih, regs, &otpregs);

	val16 = _ipxotp_read_bit(oh, &otpregs, off);

	si_setcoreidx(sih, idx);
	return (val16);
}

static uint16
BCMSROMCISDUMPATTACHFN(ipxotp_otprb16)(void *oh, otpregs_t *otpregs, uint wn)
{
	uint base, i;
	uint16 val;
	uint16 bit;

	base = wn * 16;

	val = 0;
	for (i = 0; i < 16; i++) {
		if ((bit = _ipxotp_read_bit(oh, otpregs, base + i)) == 0xffff)
			break;
		val = val | (bit << i);
	}
	if (i < 16)
		val = 0xffff;

	return val;
}

static uint16
BCMSROMCISDUMPATTACHFN(ipxotp_otpr)(void *oh, otpregs_t *otpregs, uint wn)
{
	otpinfo_t *oi;
	si_t *sih;
	uint16 val;
	osl_t *osh;

	oi = (otpinfo_t *)oh;

	ASSERT(wn < oi->wsize);
	ASSERT(otpregs != NULL);

	sih = oi->sih;
	osh = si_osh(sih);

	ASSERT(sih != NULL);
	/* If sprom is available use indirect access(as cc->sromotp maps to srom),
	 * else use random-access.
	 */
	if (si_is_sprom_available(sih))
		val = ipxotp_otprb16(oi, otpregs, wn);
	else {
		if (BUSTYPE(sih->bustype) == PCI_BUS && AOB_ENAB(sih)) {
			uint32 bar0win, otpaddr;

			/* Save BAR0WIN */
			bar0win = OSL_PCI_READ_CONFIG(osh, PCI_BAR0_WIN, 4);
			otpaddr = si_addrspace(sih, CORE_SLAVE_PORT_0, CORE_BASE_ADDR_0)
					+ SI_CORE_SIZE;
			OSL_PCI_WRITE_CONFIG(osh, PCI_BAR0_WIN, 4, otpaddr);

			val = R_REG(oi->osh, &oi->otpbase[wn]);

			/* Restore BAR0WIN */
			OSL_PCI_WRITE_CONFIG(osh, PCI_BAR0_WIN, 4, bar0win);
		} else
			val = R_REG(oi->osh, &oi->otpbase[wn]);
	}

	return val;
}

/** OTP BT region size */
static void
BCMSROMCISDUMPATTACHFN(ipxotp_bt_region_get)(otpinfo_t *oi, uint16 *start, uint16 *end)
{
	*start = *end = 0;
	switch (CHIPID(oi->sih->chip)) {
	CASE_BCM43602_CHIP:	/* 43602 does not contain a BT core, only GCI/SECI interface. */
		break;
	case BCM4363_CHIP_ID:
	case BCM4365_CHIP_ID:
	case BCM4366_CHIP_ID:
	case BCM43664_CHIP_ID:
	case BCM43666_CHIP_ID:
	CASE_BCM43684_CHIP:
	CASE_BCM6710_CHIP:
	CASE_BCM6715_CHIP:
		break; /* these (router) chips do not use the BT coex interface */
	case BCM43570_CHIP_ID:
		*start = OTP_BT_BASE_4350;
		*end = OTP_BT_END_4350;
		break;
	case BCM4347_CHIP_GRPID:
		*start = OTP_BT_BASE_4347;
		*end = OTP_BT_END_4347;
		break;
	case BCM4369_CHIP_GRPID:
		*start = OTP_BT_BASE_4369;
		*end = OTP_BT_END_4369;
		break;
	}
}

/**
 * Calculate max HW/SW region byte size by substracting fuse region and checksum size,
 * osizew is oi->wsize (OTP size - GU size) in words.
 */
static int
BCMSROMCISDUMPATTACHFN(ipxotp_max_rgnsz)(otpinfo_t *oi)
{
	int osizew = oi->wsize;
	int ret = 0;
	uint16 checksum;
	si_t *sih = oi->sih;
	uint idx;
	volatile void *regs;
	otpregs_t otpregs;

	idx = si_coreidx(sih);
	if (AOB_ENAB(sih)) {
		regs = si_setcore(sih, GCI_CORE_ID, 0);
	} else {
		regs = si_setcoreidx(sih, SI_CC_IDX);
	}
	ASSERT(regs != NULL);
	otp_initregs(sih, regs, &otpregs);

	checksum = OTP_SZ_CHECKSUM;

	/* for new chips, fusebit is available from cc register */
	if (CCREV(sih->ccrev) >= 35) {
		oi->fusebits = R_REG(oi->osh, otpregs.otplayoutextension)
			& OTPLAYOUTEXT_FUSE_MASK;
		oi->fusebits = ROUNDUP(oi->fusebits, 8);
		oi->fusebits >>= 3; /* bytes */
	}

	si_setcoreidx(sih, idx);

	switch (CHIPID(sih->chip)) {
	case BCM43217_CHIP_ID:
	case BCM43428_CHIP_ID:
		oi->fusebits = OTP_SZ_FU_72;
		break;
	case BCM43602_CHIP_ID:
		oi->fusebits = OTP_SZ_FU_972;
		break;
	case BCM4363_CHIP_ID:
	case BCM4365_CHIP_ID:
	case BCM4366_CHIP_ID:
	case BCM43664_CHIP_ID:
	case BCM43666_CHIP_ID:
		if (CHIPREV(sih->chiprev) >= 4) {
			oi->fusebits = OTP_SZ_FU_1808;
		}
		else {
			oi->fusebits = OTP_SZ_FU_1296;
		}
		break;
	CASE_BCM43684_CHIP:
		oi->fusebits = OTP_SZ_FU_2576;
		break;
	CASE_BCM6710_CHIP:
		oi->fusebits = OTP_SZ_FU_592;
		break;
	case BCM43570_CHIP_ID:
		oi->fusebits = OTP_SZ_FU_720;
		break;
	case BCM4360_CHIP_ID:
		oi->fusebits = OTP_SZ_FU_792;
		break;
	case BCM4347_CHIP_GRPID:
	case BCM4369_CHIP_GRPID:
		oi->fusebits = OTP_SZ_FU_1080;
		break;
	default:
		if (oi->fusebits == 0)
			ASSERT(0);	/* Don't konw about this chip */
	}

	ret = osizew*2 - oi->fusebits - checksum;

	OTP_MSG(("max region size %d bytes\n", ret));
	return ret;
}

/** OTP sizes for 65nm and 130nm */
static int
BCMCISDUMPATTACHFN(ipxotp_otpsize_set_65nm)(otpinfo_t *oi, uint otpsz)
{
	/* Check for otp size */
	switch (otpsz) {
	case 1:	/* 32x64 */
		oi->rows = 32;
		oi->cols = 64;
		oi->wsize = 128;
		break;
	case 2:	/* 64x64 */
		oi->rows = 64;
		oi->cols = 64;
		oi->wsize = 256;
		break;
	case 5:	/* 96x64 */
		oi->rows = 96;
		oi->cols = 64;
		oi->wsize = 384;
		break;
	case 7:	/* 16x64 */ /* 1024 bits */
		oi->rows = 16;
		oi->cols = 64;
		oi->wsize = 64;
		break;
	default:
		/* Don't know the geometry */
		OTP_ERR(("%s: unknown OTP geometry\n", __FUNCTION__));
	}

	return 0;
}

/**  OTP sizes for 40nm */
static int
BCMSROMCISDUMPATTACHFN(ipxotp_otpsize_set_40nm)(otpinfo_t *oi, uint otpsz)
{
	/* Check for otp size */
	switch (otpsz) {
	case 1:	/* 64x32: 2048 bits */
		oi->rows = 64;
		oi->cols = 32;
		break;
	case 2:	/* 96x32: 3072 bits */
		oi->rows = 96;
		oi->cols = 32;
		break;
	case 3:	/* 128x32: 4096 bits */
		oi->rows = 128;
		oi->cols = 32;
		break;
	case 4:	/* 160x32: 5120 bits */
		oi->rows = 160;
		oi->cols = 32;
		break;
	case 5:	/* 192x32: 6144 bits */
		oi->rows = 192;
		oi->cols = 32;
		break;
	case 7:	/* 256x32: 8192 bits */
		oi->rows = 256;
		oi->cols = 32;
		break;
	case 11: /* 384x32: 12288 bits */
		oi->rows = 384;
		oi->cols = 32;
		break;
	case 15: /* 512x32: 16384 bits */
		oi->rows = 512;
		oi->cols = 32;
		break;
	default:
		/* Don't know the geometry */
		OTP_ERR(("%s: unknown OTP geometry\n", __FUNCTION__));
	}

	oi->wsize = (oi->cols * oi->rows)/OTPWSIZE;
	return 0;
}

/**  OTP sizes for 28nm */
static int
BCMSROMCISDUMPATTACHFN(ipxotp_otpsize_set_28nm)(otpinfo_t *oi, uint otpsz)
{
	/* Check for otp size */
	switch (otpsz) {
	case OTPSZ_28NM_5:	/* 192x32: 6144 bits */
		oi->rows = OTPSZ_28NM_5_ROWS;
		oi->cols = OTPSZ_28NM_5_COLS;
		break;
	case OTPSZ_28NM_11:	/* 384x32: 12288 bits */
		oi->rows = OTPSZ_28NM_11_ROWS;
		oi->cols = OTPSZ_28NM_11_COLS;
		break;
	case OTPSZ_28NM_15:	/* 512x32: 16384 bits */
		oi->rows = OTPSZ_28NM_15_ROWS;
		oi->cols = OTPSZ_28NM_15_COLS;
		break;
	default:
		/* Don't know the geometry */
		OTP_ERR(("%s: unknown OTP geometry\n", __FUNCTION__));
	}

	oi->wsize = (oi->cols * oi->rows)/OTPWSIZE;
	return 0;
}

/** OTP unification */
#if defined(USBSDIOUNIFIEDOTP) && defined(BCMNVRAMW)
static void
BCMNMIATTACHFN(ipxotp_uotp_usbmanfid_offset)(otpinfo_t *oi)
{
	OTP_DBG(("%s: chip=0x%x\n", __FUNCTION__, CHIPID(oi->sih->chip)));
	switch (CHIPID(oi->sih->chip)) {
		/* Add cases for supporting chips */
		default:
			OTP_ERR(("chip=0x%x does not support Unified OTP.\n",
				CHIPID(oi->sih->chip)));
			break;
	}
}
#endif /* USBSDIOUNIFIEDOTP && BCMNVRAMW */

static void
BCMSROMCISDUMPATTACHFN(_ipxotp_init)(otpinfo_t *oi, otpregs_t *otpregs)
{
	uint	k;
	uint32 otpp, st;
	uint16 btsz, btbase = 0, btend = 0;
	uint   otpwt;

	/* record word offset of General Use Region for various chipcommon revs */
	if (CCREV(oi->sih->ccrev) >= 40) {
		/* FIX: Available in rev >= 23; Verify before applying to others */
		oi->otpgu_base = (R_REG(oi->osh, otpregs->otplayout) & OTPL_HWRGN_OFF_MASK)
			>> OTPL_HWRGN_OFF_SHIFT;
		ASSERT((oi->otpgu_base - (OTPGU_SROM_OFF * OTPWSIZE)) > 0);
		oi->otpgu_base >>= 4; /* words */
		oi->otpgu_base -= OTPGU_SROM_OFF;
	} else if (CCREV(oi->sih->ccrev) == 21 || CCREV(oi->sih->ccrev) == 24 ||
		CCREV(oi->sih->ccrev) == 27) {
		oi->otpgu_base = REVA4_OTPGU_BASE;
	} else if ((CCREV(oi->sih->ccrev) == 36) || (CCREV(oi->sih->ccrev) == 39)) {
		/* OTP size greater than equal to 2KB (128 words), otpgu_base is similar to rev23 */
		if (oi->wsize >= 128)
			oi->otpgu_base = REVB8_OTPGU_BASE;
		else
			oi->otpgu_base = REV36_OTPGU_BASE;
	} else if (CCREV(oi->sih->ccrev) == 23 || CCREV(oi->sih->ccrev) >= 25) {
		oi->otpgu_base = REVB8_OTPGU_BASE;
	} else {
		OTP_ERR(("%s: chipc rev %d not supported\n", __FUNCTION__, CCREV(oi->sih->ccrev)));
	}

	otpwt = (R_REG(oi->osh, otpregs->otplayout) & OTPL_WRAP_TYPE_MASK)
		>> OTPL_WRAP_TYPE_SHIFT;
	BCM_REFERENCE(otpwt);

	if ((OTPWRTYPE(otpwt) != OTPL_WRAP_TYPE_40NM) &&
		(OTPWRTYPE(otpwt) != OTPL_WRAP_TYPE_28NM)) {
		/* First issue an init command so the status is up to date */
		otpp = OTPP_START_BUSY | ((OTPPOC_INIT << OTPP_OC_SHIFT) & OTPP_OC_MASK);

		OTP_DBG(("%s: otpp = 0x%x", __FUNCTION__, otpp));
		W_REG(oi->osh, otpregs->otpprog, otpp);
		for (k = 0;
			((st = R_REG(oi->osh, otpregs->otpprog)) & OTPP_START_BUSY) &&
				(k < OTPP_TRIES);
			k ++)
			;
			if (k >= OTPP_TRIES) {
			OTP_ERR(("\n%s: BUSY stuck: st=0x%x, count=%d\n", __FUNCTION__, st, k));
			return;
		}
	}

	/* Read OTP lock bits and subregion programmed indication bits */
	oi->status = R_REG(oi->osh, otpregs->otpstatus);

	/* XXX WAR for PR 65487: OTP status is not updated before power-cycle, so we need
	 * to read the subregion programmed bit from OTP directly
	 */
	if ((CHIPID(oi->sih->chip) == BCM4360_CHIP_ID) ||
		(CHIPID(oi->sih->chip) == BCM43460_CHIP_ID) ||
		(CHIPID(oi->sih->chip) == BCM43526_CHIP_ID) ||
		(CHIPID(oi->sih->chip) == BCM43602_CHIP_ID) ||
		(CHIPID(oi->sih->chip) == BCM4365_CHIP_ID) ||
		BCM43684_CHIP(oi->sih->chip) ||
		BCM6710_CHIP(oi->sih->chip) ||
		(CHIPID(oi->sih->chip) == BCM4366_CHIP_ID) ||
		BCM43602_CHIP(oi->sih->chip) ||
		BCM4365_CHIP(oi->sih->chip) ||
		BCM4350_CHIP(oi->sih->chip) ||
		BCM4347_CHIP(oi->sih->chip) ||
		BCM4369_CHIP(oi->sih->chip)) {
		uint32 p_bits;
		p_bits = (ipxotp_otpr(oi, otpregs, oi->otpgu_base + OTPGU_P_OFF) & OTPGU_P_MSK)
			>> OTPGU_P_SHIFT;
		oi->status |= (p_bits << OTPS_GUP_SHIFT);
	}
	OTP_DBG(("%s: status 0x%x\n", __FUNCTION__, oi->status));

	/* OTP unification */
	oi->buotp = FALSE; /* Initialize it to false, until its explicitely set true. */
#if defined(USBSDIOUNIFIEDOTP) && defined(BCMNVRAMW)
	ipxotp_uotp_usbmanfid_offset(oi);
#endif /* USBSDIOUNIFIEDOTP && BCMNVRAMW */
	if ((oi->status & (OTPS_GUP_HW | OTPS_GUP_SW)) == (OTPS_GUP_HW | OTPS_GUP_SW)) {
		switch (CHIPID(oi->sih->chip)) {
			/* Add cases for supporting chips */
			case BCM4363_CHIP_ID:
			case BCM4365_CHIP_ID:
			case BCM4366_CHIP_ID:
			case BCM43664_CHIP_ID:
			case BCM43666_CHIP_ID:
			CASE_BCM43684_CHIP:
			CASE_BCM6710_CHIP:
				oi->buotp = TRUE;
				break;
			default:
				OTP_ERR(("chip=0x%x does not support Unified OTP.\n",
					CHIPID(oi->sih->chip)));
				break;
		}
	}

	/* if AVS is part of s/w region, update how many bits are used for AVS */
	switch (CHIPID(oi->sih->chip)) {
		default:
			oi->avsbitslen = 0;
			break;
	}

	/*
	 * h/w region base and fuse region limit are fixed to the top and
	 * the bottom of the general use region. Everything else can be flexible.
	 */
	oi->hwbase = oi->otpgu_base + OTPGU_SROM_OFF;
	oi->hwlim = oi->wsize;
	oi->flim = oi->wsize;

	ipxotp_bt_region_get(oi, &btbase, &btend);
	btsz = btend - btbase;
	if (btsz > 0) {
		/* default to not exceed BT base */
		oi->hwlim = btbase;

		/* With BT shared region, swlim and fbase are fixed */
		oi->swlim = btbase;
		oi->fbase = btend;
		/* if avs bits are part of swregion, subtract that from the sw/hw limit */
		oi->hwlim -= oi->avsbitslen / OTPWSIZE;
		oi->swlim -= oi->avsbitslen / OTPWSIZE;
	}

	/* Update hwlim and swbase */
	if (oi->status & OTPS_GUP_HW) {
		uint16 swbase;
		OTP_DBG(("%s: hw region programmed\n", __FUNCTION__));
		swbase = ipxotp_otpr(oi, otpregs, oi->otpgu_base + OTPGU_HSB_OFF) / 16;
		if (swbase) {
			oi->hwlim =  swbase;
		}
		oi->swbase = oi->hwlim;
	} else {
		oi->swbase = oi->hwbase;
	}

	/* Update swlim and fbase only if no BT region */
	if (btsz == 0) {
		/* subtract fuse and checksum from beginning */
		oi->swlim = ipxotp_max_rgnsz(oi) / 2;

		if (oi->status & OTPS_GUP_SW) {
			OTP_DBG(("%s: sw region programmed\n", __FUNCTION__));
			oi->swlim = ipxotp_otpr(oi, otpregs, oi->otpgu_base + OTPGU_SFB_OFF) / 16;
			oi->fbase = oi->swlim;
		}
		else
			oi->fbase = oi->swbase;
		/* if avs bits are part of swregion, subtract that from the sw limit */
		oi->swlim -= oi->avsbitslen;
	}

	OTP_DBG(("%s: OTP limits---\n"
		"hwbase %d/%d hwlim %d/%d\n"
		"swbase %d/%d swlim %d/%d\n"
		"fbase %d/%d flim %d/%d\n", __FUNCTION__,
		oi->hwbase, oi->hwbase * 16, oi->hwlim, oi->hwlim * 16,
		oi->swbase, oi->swbase * 16, oi->swlim, oi->swlim * 16,
		oi->fbase, oi->fbase * 16, oi->flim, oi->flim * 16));
}

static void *
BCMSROMCISDUMPATTACHFN(ipxotp_init)(si_t *sih)
{
	uint idx, otpsz, otpwt;
	volatile void *regs;
	otpregs_t otpregs;
	otpinfo_t *oi = NULL;

	OTP_MSG(("%s: Use IPX OTP controller\n", __FUNCTION__));

	/* Make sure we're running IPX OTP */
	ASSERT(OTPTYPE_IPX(CCREV(sih->ccrev)));
	if (!OTPTYPE_IPX(CCREV(sih->ccrev)))
		return NULL;

	/* Make sure OTP is not disabled */
	if (si_is_otp_disabled(sih)) {
		OTP_MSG(("%s: OTP is disabled\n", __FUNCTION__));
#if !defined(WLTEST)
		return NULL;
#endif // endif
	}

	/* Make sure OTP is powered up */
	if (!si_is_otp_powered(sih)) {
		OTP_ERR(("%s: OTP is powered down\n", __FUNCTION__));
		return NULL;
	}

	/* Retrieve OTP region info */
	/* XXX: Change to use the "fast" access method in chips that support it.
	 *	see nicpci.c and siutils.c for guidance.
	 */
	idx = si_coreidx(sih);
	if (AOB_ENAB(sih)) {
		regs = si_setcore(sih, GCI_CORE_ID, 0);
	} else {
		regs = si_setcoreidx(sih, SI_CC_IDX);
	}
	ASSERT(regs != NULL);
	otp_initregs(sih, regs, &otpregs);

	oi = get_otpinfo();

	if (BCM6710_CHIP(oi->sih->chip) && CHIPREV(sih->chiprev) == 0) {
		otpsz = OTPSZ_28NM_11;
	} else if (CCREV(sih->ccrev) >= 49) {
		otpsz = (R_REG(oi->osh, otpregs.otplayout) & OTPL_ROW_SIZE_MASK)
		    >> OTPL_ROW_SIZE_SHIFT;
	} else {
		otpsz = (sih->cccaps & CC_CAP_OTPSIZE) >> CC_CAP_OTPSIZE_SHIFT;
	}

	if (otpsz == 0) {
		OTP_ERR(("%s: No OTP\n", __FUNCTION__));
		oi = NULL;
		goto exit;
	}

	otpwt = (R_REG(oi->osh, otpregs.otplayout) & OTPL_WRAP_TYPE_MASK) >> OTPL_WRAP_TYPE_SHIFT;
	BCM_REFERENCE(otpwt);

	if (OTPWRTYPE(otpwt) == OTPL_WRAP_TYPE_40NM) {
		ipxotp_otpsize_set_40nm(oi, otpsz);
	} else if (OTPWRTYPE(otpwt) == OTPL_WRAP_TYPE_65NM) {
		ipxotp_otpsize_set_65nm(oi, otpsz);
	} else if (OTPWRTYPE(otpwt) == OTPL_WRAP_TYPE_28NM) {
		ipxotp_otpsize_set_28nm(oi, otpsz);
	} else {
		OTP_ERR(("%s: Unknown wrap type: %d\n", __FUNCTION__, OTPWRTYPE(otpwt)));
		oi = NULL;
		goto exit;
	}

	OTP_MSG(("%s: rows %u cols %u wsize %u\n", __FUNCTION__, oi->rows, oi->cols, oi->wsize));

	/* check if GCI core is exist */
	if (CCREV(sih->ccrev) >= 49 && si_findcoreidx(sih, GCI_CORE_ID, 0) != BADIDX) {
		if (AOB_ENAB(sih)) {
			uint32 otpaddr;
			otpaddr = si_addrspace(sih, CORE_SLAVE_PORT_0, CORE_BASE_ADDR_0) +
					SI_CORE_SIZE;
			if (BUSTYPE(sih->bustype) != PCI_BUS) {
				oi->otpbase = (uint16 *)REG_MAP(otpaddr, SI_CORE_SIZE);
			} else {
				/* Need to do si_set_win to
				 * (ai_addrspace(sih, CORE_SLAVE_PORT_0, CORE_BASE_ADDR_0) +
					SI_CORE_SIZE)
				 * whenever access through the otpbase ptr.
				 */
				 oi->otpbase = (volatile uint16 *)regs;
			}
			OTP_ERR(("%s: mapping otpbase at 0x%08x to 0x%p\n", __FUNCTION__,
				otpaddr, oi->otpbase));
		} else {
			uint idx2;
			/* Take offset of OTP Base address from GCI CORE */
			idx2 = si_coreidx(sih);
			oi->otpbase = (volatile uint16 *)si_setcore(sih, GCI_CORE_ID, 0);
			/* Reset si handler curidx to CC */
			si_setcoreidx(sih, idx2);
		}
	} else {
		chipcregs_t *cc = (chipcregs_t *)regs;
		oi->otpbase = (volatile uint16 *)cc->sromotp;
	}

#ifdef BCMNVRAMW
	/* Initialize OTP redundancy control blocks */
	if (CCREV(sih->ccrev) >= 49) {
		uint16 offset[] = {266, 284, 302, 330, 348, 366, 394, 412, 430};
		bcopy(offset, &oi->rde_cb.offset, sizeof(offset));
		oi->rde_cb.offsets = ARRAYSIZE(offset);
		oi->rde_cb.width = 18;
		oi->rde_cb.val_shift = 14;
		oi->rde_cb.stat_shift = 16;
	} else if (CCREV(sih->ccrev) >= 40) {
		uint16 offset[] = {269, 286, 303, 333, 350, 367, 397, 414, 431};
		bcopy(offset, &oi->rde_cb.offset, sizeof(offset));
		oi->rde_cb.offsets = ARRAYSIZE(offset);
		oi->rde_cb.width = 17;
		oi->rde_cb.val_shift = 13;
		oi->rde_cb.stat_shift = 16;
	} else if (CCREV(sih->ccrev) == 36) {
		uint16 offset[] = {141, 158, 175};
		bcopy(offset, &oi->rde_cb.offset, sizeof(offset));
		oi->rde_cb.offsets = ARRAYSIZE(offset);
		oi->rde_cb.width = 15;
		oi->rde_cb.val_shift = 13;
		oi->rde_cb.stat_shift = 16;
	} else if (CCREV(sih->ccrev) == 21 || CCREV(sih->ccrev) == 24) {
		uint16 offset[] = {64, 79, 94, 109, 128, 143, 158, 173};
		bcopy(offset, &oi->rde_cb.offset, sizeof(offset));
		oi->rde_cb.offsets = ARRAYSIZE(offset);
		oi->rde_cb.width = 15;
		oi->rde_cb.val_shift = 11;
		oi->rde_cb.stat_shift = 16;
	}
	else if (CCREV(sih->ccrev) == 27) {
		uint16 offset[] = {128, 143, 158, 173};
		bcopy(offset, &oi->rde_cb.offset, sizeof(offset));
		oi->rde_cb.offsets = ARRAYSIZE(offset);
		oi->rde_cb.width = 15;
		oi->rde_cb.val_shift = 11;
		oi->rde_cb.stat_shift = 20;
	}
	else {
		uint16 offset[] = {141, 158, 175, 205, 222, 239, 269, 286, 303};
		bcopy(offset, &oi->rde_cb.offset, sizeof(offset));
		oi->rde_cb.offsets = ARRAYSIZE(offset);
		oi->rde_cb.width = 17;
		oi->rde_cb.val_shift = 13;
		oi->rde_cb.stat_shift = 16;
	}
	ASSERT(oi->rde_cb.offsets <= MAXNUMRDES);
	/* Initialize global rde index */
	oi->rde_idx = 0;
#endif	/* BCMNVRAMW */

	_ipxotp_init(oi, &otpregs);

exit:
	si_setcoreidx(sih, idx);

	return (void *)oi;
}

static int
BCMSROMCISDUMPATTACHFN(ipxotp_read_region)(void *oh, int region, uint16 *data, uint *wlen)
{
	otpinfo_t *oi = (otpinfo_t *)oh;
	uint idx;
	uint base, i, sz;
	si_t *sih = oi->sih;
	volatile void *regs;
	otpregs_t otpregs;

	/* Validate region selection */
	switch (region) {
	case OTP_HW_RGN:
		/* OTP unification: For unified OTP sz=flim-hwbase */
		if (oi->buotp)
			sz = (uint)oi->flim - oi->hwbase;
		else
			sz = (uint)oi->hwlim - oi->hwbase;
		if (!(oi->status & OTPS_GUP_HW)) {
			OTP_ERR(("%s: h/w region not programmed\n", __FUNCTION__));
			*wlen = sz;
			return BCME_NOTFOUND;
		}
		if (*wlen < sz) {
			OTP_ERR(("%s: buffer too small, should be at least %u\n",
			         __FUNCTION__, oi->hwlim - oi->hwbase));
			*wlen = sz;
			return BCME_BUFTOOSHORT;
		}
		base = oi->hwbase;
		break;
	case OTP_SW_RGN:
		/* OTP unification: For unified OTP sz=flim-swbase */
		if (oi->buotp)
			sz = ((uint)oi->flim - oi->swbase);
		else
			sz = ((uint)oi->swlim - oi->swbase);
		if (!(oi->status & OTPS_GUP_SW)) {
			OTP_ERR(("%s: s/w region not programmed\n", __FUNCTION__));
			*wlen = sz;
			return BCME_NOTFOUND;
		}
		if (*wlen < sz) {
			OTP_ERR(("%s: buffer too small should be at least %u\n",
			         __FUNCTION__, oi->swlim - oi->swbase));
			*wlen = sz;
			return BCME_BUFTOOSHORT;
		}
		base = oi->swbase;
		break;
	case OTP_CI_RGN:
		sz = OTPGU_CI_SZ;
		if (!(oi->status & OTPS_GUP_CI)) {
			OTP_ERR(("%s: chipid region not programmed\n", __FUNCTION__));
			*wlen = sz;
			return BCME_NOTFOUND;
		}
		if (*wlen < sz) {
			OTP_ERR(("%s: buffer too small, should be at least %u\n",
			         __FUNCTION__, OTPGU_CI_SZ));
			*wlen = sz;
			return BCME_BUFTOOSHORT;
		}
		base = oi->otpgu_base + OTPGU_CI_OFF;
		break;
	case OTP_FUSE_RGN:
		sz = (uint)oi->flim - oi->fbase;
		if (!(oi->status & OTPS_GUP_FUSE)) {
			OTP_ERR(("%s: fuse region not programmed\n", __FUNCTION__));
			*wlen = sz;
			return BCME_NOTFOUND;
		}
		if (*wlen < sz) {
			OTP_ERR(("%s: buffer too small, should be at least %u\n",
			         __FUNCTION__, oi->flim - oi->fbase));
			*wlen = sz;
			return BCME_BUFTOOSHORT;
		}
		base = oi->fbase;
		break;
	case OTP_NW_RGN:
		sz = OTP_NW_SZ;
		base = OTP_NW_BASE;
		break;
	case OTP_ALL_RGN:
		sz = ((uint)oi->flim - oi->hwbase);
		if (!(oi->status & (OTPS_GUP_HW | OTPS_GUP_SW))) {
			OTP_ERR(("%s: h/w & s/w region not programmed\n", __FUNCTION__));
			*wlen = sz;
			return BCME_NOTFOUND;
		}
		if (*wlen < sz) {
			OTP_ERR(("%s: buffer too small, should be at least %u\n",
				__FUNCTION__, oi->hwlim - oi->hwbase));
			*wlen = sz;
			return BCME_BUFTOOSHORT;
		}
		base = oi->hwbase;
		break;
	default:
		OTP_ERR(("%s: reading region %d is not supported\n", __FUNCTION__, region));
		return BCME_BADARG;
	}

	idx = si_coreidx(sih);
	if (AOB_ENAB(sih)) {
		regs = si_setcore(sih, GCI_CORE_ID, 0);
	} else {
		regs = si_setcoreidx(sih, SI_CC_IDX);
	}
	ASSERT(regs != NULL);
	otp_initregs(sih, regs, &otpregs);

	/* Read the data */
	for (i = 0; i < sz; i ++)
		data[i] = ipxotp_otpr(oh, &otpregs, base + i);

	si_setcoreidx(sih, idx);
	*wlen = sz;
	return 0;
}

static int
BCMCISDUMPATTACHFN(ipxotp_read_word)(void *oh, uint wn, uint16 *data)
{
	otpinfo_t *oi = (otpinfo_t *)oh;
	uint idx;
	volatile void *regs;
	otpregs_t otpregs;
	si_t *sih = oi->sih;

	idx = si_coreidx(sih);
	if (AOB_ENAB(sih)) {
		regs = si_setcore(sih, GCI_CORE_ID, 0);
	} else {
		regs = si_setcoreidx(sih, SI_CC_IDX);
	}
	ASSERT(regs != NULL);
	otp_initregs(sih, regs, &otpregs);

	/* Read the data */
	*data = ipxotp_otpr(oh, &otpregs, wn);

	si_setcoreidx(sih, idx);
	return 0;
}

static int
BCMCISDUMPATTACHFN(ipxotp_nvread)(void *oh, char *data, uint *len)
{
	BCM_REFERENCE(oh);
	BCM_REFERENCE(data);
	BCM_REFERENCE(len);

	return BCME_UNSUPPORTED;
}

#ifdef BCMNVRAMW
static int
BCMNMIATTACHFN(ipxotp_writable)(otpinfo_t *oi, otpregs_t *otpregs)
{
	uint otpwt;

	otpwt = (R_REG(oi->osh, otpregs->otplayout) & OTPL_WRAP_TYPE_MASK) >> OTPL_WRAP_TYPE_SHIFT;
	BCM_REFERENCE(otpwt);

	if (OTPWRTYPE(otpwt) == OTPL_WRAP_TYPE_40NM) {
		uint i, k, row, col;
		uint32 otpp, st;
		uint cols[OTP_PROGEN_SEQCOUNT] = {OTP_PROGEN_IDX1, OTP_PROGEN_IDX2,
			OTP_PROGEN_IDX3, OTP_PROGEN_IDX4};

		row = 0;
		for (i = 0; i < OTP_PROGEN_SEQCOUNT; i++) {
			col = cols[i];

			otpp = OTPP_START_BUSY |
				((OTPPOC_PROG_ENABLE_40NM << OTPP_OC_SHIFT) & OTPP_OC_MASK) |
				((row << OTPP_ROW_SHIFT) & OTPPROWMASK(CCREV(oi->sih->ccrev))) |
				((col << OTPP_COL_SHIFT) & OTPP_COL_MASK);
			OTP_DBG(("%s: row = %d, col = %d, otpp = 0x%x\n",
				__FUNCTION__, row, col, otpp));

			W_REG(oi->osh, otpregs->otpprog, otpp);

			for (k = 0;
				((st = R_REG(oi->osh, otpregs->otpprog)) & OTPP_START_BUSY) &&
				(k < OTPP_TRIES); k ++)
				;
			if (k >= OTPP_TRIES) {
				OTP_ERR(("\n%s: BUSY stuck: st=0x%x, count=%d\n",
					__FUNCTION__, st, k));
				return -1;
			}
		}

		/* wait till OTP Program mode is unlocked */
		for (k = 0;
			(!((st = R_REG(oi->osh, otpregs->otpstatus)) & OTPS_PROGOK)) &&
			(k < OTPP_TRIES); k ++)
			;
		OTP_MSG(("\n%s: OTP Program status: %x\n", __FUNCTION__, st));

		if (k >= OTPP_TRIES) {
			OTP_ERR(("\n%s: OTP Program mode is still locked, OTP is unwritable\n",
				__FUNCTION__));
			return -1;
		}
	}
	else if (OTPWRTYPE(otpwt) == OTPL_WRAP_TYPE_28NM) {
		uint i, k, row, col;
		uint32 otpp, st;
		uint cols[OTP_PROGEN_SEQCOUNT] = {OTP_PROGEN_IDX1, OTP_PROGEN_IDX2,
			OTP_PROGEN_IDX3, OTP_PROGEN_IDX4};

		/* This sequence is required to enable Programming in OTP */
		row = 0;
		for (i = 0; i < OTP_PROGEN_SEQCOUNT; i++) {
			col = cols[i];
			W_REG(oi->osh, otpregs->otpprogdata, col);

			otpp = OTPP_START_BUSY |
				((OTPPOC_PROG_ENABLE_28NM << OTPP_OC_SHIFT_28NM) &
				OTPP_OC_MASK_28NM) |
				((row << OTPP_ROW_SHIFT) & OTPPROWMASK(CCREV(oi->sih->ccrev)));
			OTP_DBG(("%s: row = %d, col = %d, otpp = 0x%x\n",
				__FUNCTION__, row, col, otpp));

			W_REG(oi->osh, otpregs->otpprog, otpp);

			for (k = 0;
				((st = R_REG(oi->osh, otpregs->otpprog)) & OTPP_START_BUSY) &&
				(k < OTPP_TRIES); k ++)
				;
			if (k >= OTPP_TRIES) {
				OTP_ERR(("\n%s: BUSY stuck: st=0x%x, count=%d\n",
					__FUNCTION__, st, k));
				return -1;
			}
		}

		/* wait till OTP Program mode is unlocked */
		for (k = 0;
			(!((st = R_REG(oi->osh, otpregs->otpstatus)) & OTPS_PROGOK)) &&
			(k < OTPP_TRIES); k ++)
			;
		OTP_MSG(("\n%s: OTP Program status: %x\n", __FUNCTION__, st));

		if (k >= OTPP_TRIES) {
			OTP_ERR(("\n%s: OTP Program mode is still locked, OTP is unwritable\n",
				__FUNCTION__));
			return -1;
		}
	}
	OR_REG(oi->osh, otpregs->otpcontrol,
		((OTPWRTYPE(otpwt) == OTPL_WRAP_TYPE_28NM)? OTPC_PROGEN_28NM :
		OTPC_PROGEN));
	return 0;
}

static int
BCMNMIATTACHFN(ipxotp_unwritable)(otpinfo_t *oi, otpregs_t *otpregs)
{
	uint otpwt;

	otpwt = (R_REG(oi->osh, otpregs->otplayout) & OTPL_WRAP_TYPE_MASK) >> OTPL_WRAP_TYPE_SHIFT;
	BCM_REFERENCE(otpwt);

	if (OTPWRTYPE(otpwt) == OTPL_WRAP_TYPE_40NM) {
		uint k, row, col;
		uint32 otpp, st;

		row = 0;
		col = 0;

		otpp = OTPP_START_BUSY |
			((OTPPOC_PROG_DISABLE_40NM << OTPP_OC_SHIFT) & OTPP_OC_MASK) |
			((row << OTPP_ROW_SHIFT) & OTPPROWMASK(CCREV(oi->sih->ccrev))) |
			((col << OTPP_COL_SHIFT) & OTPP_COL_MASK);
		OTP_DBG(("%s: row = %d, col = %d, otpp = 0x%x\n",
			__FUNCTION__, row, col, otpp));

		W_REG(oi->osh, otpregs->otpprog, otpp);

		for (k = 0;
			((st = R_REG(oi->osh, otpregs->otpprog))
				& OTPP_START_BUSY) && (k < OTPP_TRIES);
			k ++)
			;
		if (k >= OTPP_TRIES) {
			OTP_ERR(("\n%s: BUSY stuck: st=0x%x, count=%d\n", __FUNCTION__, st, k));
			return -1;
		}

		/* wait till OTP Program mode is unlocked */
		for (k = 0;
			((st = R_REG(oi->osh, otpregs->otpstatus))
				& OTPS_PROGOK) && (k < OTPP_TRIES);
			k ++)
			;
		OTP_MSG(("\n%s: OTP Program status: %x\n", __FUNCTION__, st));

		if (k >= OTPP_TRIES) {
			OTP_ERR(("\n%s: OTP Program mode is still unlocked, OTP is writable\n",
				__FUNCTION__));
			return -1;
		}
	}
	AND_REG(oi->osh, otpregs->otpcontrol,
		((OTPWRTYPE(otpwt) == OTPL_WRAP_TYPE_28NM)? ~OTPC_PROGEN_28NM :
		~OTPC_PROGEN));
	return 0;
}

static int
BCMNMIATTACHFN(ipxotp_write_bit_common)(otpinfo_t *oi, otpregs_t *otpregs, uint off)
{
	uint k, row, col;
	uint32 otpp, st;
	uint otpwt;
	uint32 progdata = 0;

	otpwt = (R_REG(oi->osh, otpregs->otplayout) & OTPL_WRAP_TYPE_MASK) >> OTPL_WRAP_TYPE_SHIFT;
	BCM_REFERENCE(otpwt);

	row = off / oi->cols;
	col = off % oi->cols;

	if (OTPWRTYPE(otpwt) != OTPL_WRAP_TYPE_28NM) {
		otpp = OTPP_START_BUSY |
		        ((1 << OTPP_VALUE_SHIFT) & OTPP_VALUE_MASK) |
			((((OTPWRTYPE(otpwt) == OTPL_WRAP_TYPE_40NM)? OTPPOC_BIT_PROG_40NM :
			OTPPOC_BIT_PROG) << OTPP_OC_SHIFT) & OTPP_OC_MASK) |
		        ((row << OTPP_ROW_SHIFT) & OTPPROWMASK(CCREV(oi->sih->ccrev))) |
		        ((col << OTPP_COL_SHIFT) & OTPP_COL_MASK);
		OTP_DBG(("%s: off = %d, row = %d, col = %d, otpp = 0x%x\n",
		         __FUNCTION__, off, row, col, otpp));
	}
	else {
		progdata = 1 << col;
		OTP_DBG(("%s: progdata = 0x%x, off = %d, row = %d, col = %d \n",
		         __FUNCTION__, progdata, off, row, col));

		/* Program the data to GCI Program data register(OTPProgData 0x328) */
		W_REG(oi->osh, otpregs->otpprogdata, progdata);

		/* Program OTP opcode in OTPProg Register (OTPProg 0x318) */
		otpp = OTPP_START_BUSY |
			(((OTPPOC_PROG_28NM) << OTPP_OC_SHIFT_28NM) & OTPP_OC_MASK_28NM) |
		        ((row << OTPP_ROW_SHIFT) & OTPPROWMASK(CCREV(oi->sih->ccrev)));
	}
	W_REG(oi->osh, otpregs->otpprog, otpp);

	for (k = 0;
	     ((st = R_REG(oi->osh, otpregs->otpprog)) & OTPP_START_BUSY) && (k < OTPP_TRIES);
	     k ++)
		;
	if (k >= OTPP_TRIES) {
		OTP_ERR(("\n%s: BUSY stuck: st=0x%x, count=%d\n", __FUNCTION__, st, k));
		return -1;
	}

	return 0;
}

static int
BCMNMIATTACHFN(ipxotp40n_read2x)(void *oh, otpregs_t *otpregs, uint off)
{
	otpinfo_t *oi;
	uint32 otpcontrol1 = 0;

	oi = (otpinfo_t *)oh;

	W_REG(oi->osh, otpregs->otpcontrol,
		(OTPC_40NM_PCOUNT_V1X << OTPC_40NM_PCOUNT_SHIFT) |
		(OTPC_40NM_REGCSEL_DEF << OTPC_40NM_REGCSEL_SHIFT) |
		(1 << OTPC_40NM_PROGIN_SHIFT) |
		(1 << OTPC_40NM_R2X_SHIFT) |
		(1 << OTPC_40NM_ODM_SHIFT) |
		(1 << OTPC_40NM_DF_SHIFT) |
		(OTPC_40NM_VSEL_R1X << OTPC_40NM_VSEL_SHIFT) |
		(1 << OTPC_40NM_COFAIL_SHIFT));

	if (CCREV(oi->sih->ccrev) >= 49) {
		otpcontrol1 = R_REG(oi->osh, otpregs->otpcontrol1);
		otpcontrol1 &= (OTPC1_CLK_EN_MASK | OTPC1_CLK_DIV_MASK);
	}

	W_REG(oi->osh, otpregs->otpcontrol1, otpcontrol1 |
		(OTPC1_CPCSEL_DEF << OTPC1_CPCSEL_SHIFT) |
		(OTPC1_TM_R1X << OTPC1_TM_SHIFT));

	return ipxotp_read_bit_common(oh, otpregs, off);
}

static int
BCMNMIATTACHFN(ipxotp40n_read1x)(void *oh, otpregs_t *otpregs, uint off, uint fuse)
{
	otpinfo_t *oi;
	uint32 otpcontrol1 = 0;

	oi = (otpinfo_t *)oh;

	W_REG(oi->osh, otpregs->otpcontrol,
		(fuse << OTPC_40NM_PROGSEL_SHIFT) |
		(OTPC_40NM_PCOUNT_V1X << OTPC_40NM_PCOUNT_SHIFT) |
		(OTPC_40NM_REGCSEL_DEF << OTPC_40NM_REGCSEL_SHIFT) |
		(1 << OTPC_40NM_PROGIN_SHIFT) |
		(0 << OTPC_40NM_R2X_SHIFT) |
		(1 << OTPC_40NM_ODM_SHIFT) |
		(1 << OTPC_40NM_DF_SHIFT) |
		(OTPC_40NM_VSEL_R1X << OTPC_40NM_VSEL_SHIFT) |
		(1 << OTPC_40NM_COFAIL_SHIFT));
	if (CCREV(oi->sih->ccrev) >= 49) {
		otpcontrol1 = R_REG(oi->osh, otpregs->otpcontrol1);
		otpcontrol1 &= (OTPC1_CLK_EN_MASK | OTPC1_CLK_DIV_MASK);
	}
	W_REG(oi->osh, otpregs->otpcontrol1, otpcontrol1 |
		(OTPC1_CPCSEL_DEF << OTPC1_CPCSEL_SHIFT) |
		(OTPC1_TM_R1X << OTPC1_TM_SHIFT));

	return ipxotp_read_bit_common(oh, otpregs, off);
}

static int
BCMNMIATTACHFN(ipxotp40n_verify1x)(void *oh, otpregs_t *otpregs, uint off, uint fuse)
{
	otpinfo_t *oi;
	uint32 otpcontrol1 = 0;

	oi = (otpinfo_t *)oh;

	W_REG(oi->osh, otpregs->otpcontrol,
		(fuse << OTPC_40NM_PROGSEL_SHIFT) |
		(OTPC_40NM_PCOUNT_V1X << OTPC_40NM_PCOUNT_SHIFT) |
		(OTPC_40NM_REGCSEL_DEF << OTPC_40NM_REGCSEL_SHIFT) |
		(1 << OTPC_40NM_PROGIN_SHIFT) |
		(0 << OTPC_40NM_R2X_SHIFT) |
		(1 << OTPC_40NM_ODM_SHIFT) |
		(1 << OTPC_40NM_DF_SHIFT) |
		(OTPC_40NM_VSEL_V1X << OTPC_40NM_VSEL_SHIFT) |
		(1 << OTPC_40NM_COFAIL_SHIFT));

	if (CCREV(oi->sih->ccrev) >= 49) {
		otpcontrol1 = R_REG(oi->osh, otpregs->otpcontrol1);
		otpcontrol1 &= (OTPC1_CLK_EN_MASK | OTPC1_CLK_DIV_MASK);
	}
	W_REG(oi->osh, otpregs->otpcontrol1, otpcontrol1 |
		(OTPC1_CPCSEL_DEF << OTPC1_CPCSEL_SHIFT) |
		(OTPC1_TM_V1X << OTPC1_TM_SHIFT));

	return ipxotp_read_bit_common(oh, otpregs, off);
}

static int
BCMNMIATTACHFN(ipxotp40n_write_fuse)(otpinfo_t *oi, otpregs_t *otpregs, uint off, uint fuse)
{
	uint32 otpcontrol1 = 0;

	W_REG(oi->osh, otpregs->otpcontrol,
		(fuse << OTPC_40NM_PROGSEL_SHIFT) |
		(OTPC_40NM_PCOUNT_WR << OTPC_40NM_PCOUNT_SHIFT) |
		(OTPC_40NM_REGCSEL_DEF << OTPC_40NM_REGCSEL_SHIFT) |
		(1 << OTPC_40NM_PROGIN_SHIFT) |
		(0 << OTPC_40NM_R2X_SHIFT) |
		(1 << OTPC_40NM_ODM_SHIFT) |
		(0 << OTPC_40NM_DF_SHIFT) |
		(OTPC_40NM_VSEL_WR << OTPC_40NM_VSEL_SHIFT) |
		(1 << OTPC_40NM_COFAIL_SHIFT) |
		OTPC_PROGEN);

	if (CCREV(oi->sih->ccrev) >= 49) {
		otpcontrol1 = R_REG(oi->osh, otpregs->otpcontrol1);
		otpcontrol1 &= (OTPC1_CLK_EN_MASK | OTPC1_CLK_DIV_MASK);
	}
	W_REG(oi->osh, otpregs->otpcontrol1, otpcontrol1 |
		(OTPC1_CPCSEL_DEF << OTPC1_CPCSEL_SHIFT) |
		(OTPC1_TM_WR << OTPC1_TM_SHIFT));

	return ipxotp_write_bit_common(oi, otpregs, off);
}

static int
BCMNMIATTACHFN(ipxotp40n_write_bit)(otpinfo_t *oi, otpregs_t *otpregs, uint off)
{
	uint32 oc_orig, oc1_orig;
	uint8 i, j, err = 0;
	int verr0, verr1, rerr0, rerr1, retry, val;

	oc_orig = R_REG(oi->osh, otpregs->otpcontrol);
	oc1_orig = R_REG(oi->osh, otpregs->otpcontrol1);

	for (i = 0; i < OTP_FUSES_PER_BIT; i++) {
		retry = 0;
		for (j = 0; j < OTP_WRITE_RETRY; j++) {
			/* program fuse */
			ipxotp40n_write_fuse(oi, otpregs, off, i);

			/* verify fuse */
			val = ipxotp40n_verify1x(oi, otpregs, off, i);
			if (val == 1)
				break;

			retry++;
		}

		if ((val != 1) && (j == OTP_WRITE_RETRY)) {
			OTP_ERR(("ERROR: New write failed max attempts fuse:%d @ off:%d\n",
				i, off));
		} else if (retry > 0)
			OTP_MSG(("Verify1x multi retries:%d fuse:%d @ off:%d\n",
				retry, i, off));
	}

	/* Post screen */
	verr0 = (ipxotp40n_verify1x(oi, otpregs, off, 0) == 1) ? TRUE : FALSE;
	verr1 = (ipxotp40n_verify1x(oi, otpregs, off, 1) == 1) ? TRUE : FALSE;
	rerr0 = (ipxotp40n_read1x(oi, otpregs, off, 0) == 1) ? TRUE : FALSE;
	rerr1 = (ipxotp40n_read1x(oi, otpregs, off, 1) == 1) ? TRUE : FALSE;

	if (verr0 && verr1) {
		OTP_MSG(("V0:%d and V1:%d ok off:%d\n", verr0, verr1, off));
	} else if (verr0 && rerr1) {
		OTP_MSG(("V0:%d and R1:%d ok off:%d\n", verr0, rerr1, off));
	} else if (rerr0 && verr1) {
		OTP_MSG(("R0:%d and V1:%d ok off:%d\n", rerr0, verr1, off));
	} else {
		OTP_ERR(("Bit failed post screen v0:%d v1:%d r0:%d r1:%d off:%d\n",
			verr0, verr1, rerr0, rerr1, off));
		err = -1;
	}

	W_REG(oi->osh, otpregs->otpcontrol, oc_orig);
	W_REG(oi->osh, otpregs->otpcontrol1, oc1_orig);

	return err;
}

#ifdef OTP_DEBUG
int
BCMNMIATTACHFN(otp_read1x)(void *oh, uint off, uint fuse)
{
	otpinfo_t *oi = (otpinfo_t *)oh;
	chipcregs_t *cc;
	uint idx, otpwt;
	int val = 0;
	si_t *sih = oi->sih;
	void *regs;
	otpregs_t otpregs;

	idx = si_coreidx(sih);
	if (AOB_ENAB(sih)) {
		regs = si_setcore(sih, GCI_CORE_ID, 0);
	} else {
		regs = si_setcoreidx(sih, SI_CC_IDX);
	}
	ASSERT(regs != NULL);
	otp_initregs(sih, regs, &otpregs);

	otpwt = (R_REG(oi->osh, otpregs.otplayout) & OTPL_WRAP_TYPE_MASK)
		>> OTPL_WRAP_TYPE_SHIFT;
	BCM_REFERENCE(otpwt);
	if ((OTPWRTYPE(otpwt) != OTPL_WRAP_TYPE_40NM) || (CCREV(sih->ccrev) < 40))
		goto exit;

	val = ipxotp40n_read1x(oi, &otpregs, off, fuse);

exit:
	si_setcoreidx(sih, idx);
	return val;
}

int
BCMNMIATTACHFN(otp_verify1x)(void *oh, uint off, uint fuse)
{
	otpinfo_t *oi = (otpinfo_t *)oh;
	int err = 0;
	chipcregs_t *cc;
	uint idx, otpwt;
	si_t *sih = oi->sih;
	void *regs;
	otpregs_t otpregs;

	idx = si_coreidx(sih);
	if (AOB_ENAB(sih)) {
		regs = si_setcore(sih, GCI_CORE_ID, 0);
	} else {
		regs = si_setcoreidx(sih, SI_CC_IDX);
	}
	ASSERT(regs != NULL);
	otp_initregs(sih, regs, &otpregs);

	otpwt = (R_REG(oi->osh, otpregs.otplayout) & OTPL_WRAP_TYPE_MASK)
		>> OTPL_WRAP_TYPE_SHIFT;
	BCM_REFERENCE(otpwt);
	if ((OTPWRTYPE(otpwt) != OTPL_WRAP_TYPE_40NM) || (CCREV(oi->sih->ccrev) < 40))
		goto exit;

	err = ipxotp40n_verify1x(oi, &otpregs, off, fuse);
	if (err != 1)
		OTP_ERR(("v1x failed fuse:%d @ off:%d\n", fuse, off));
exit:
	si_setcoreidx(sih, idx);
	return err;
}

int
BCMNMIATTACHFN(otp_write_ones_old)(void *oh, uint off, uint bits)
{
	otpinfo_t *oi = (otpinfo_t *)oh;
	uint idx;
	void *regs;
	otpregs_t otpregs;
	uint32 i;
	uint32 min_res_mask = 0;
	uint32 otpcontrol1;
	si_t *sih = oi->sih;

	if (off < 0 || off + bits > oi->rows * oi->cols)
		return BCME_RANGE;

	idx = si_coreidx(sih);
	if (AOB_ENAB(sih)) {
		regs = si_setcore(sih, GCI_CORE_ID, 0);
	} else {
		regs = si_setcoreidx(sih, SI_CC_IDX);
	}
	ASSERT(regs != NULL);
	otp_initregs(sih, regs, &otpregs);

	W_REG(oi->osh, otpregs.otpcontrol, 0);
	if (CCREV(sih->ccrev) >= 49) {
		otpcontrol1 = R_REG(oi->osh, otpregs.otpcontrol1);
		otpcontrol1 &= (OTPC1_CLK_EN_MASK | OTPC1_CLK_DIV_MASK);
	}
	W_REG(oi->osh, otpregs.otpcontrol1, otpcontrol1);

	ipxotp_writable(oi, &otpregs);
	for (i = 0; i < bits; i++) {
		ipxotp_write_bit_common(oi, &otpregs, off++);
	}
	ipxotp_unwritable(oi, &otpregs);

	si_otp_power(sih, FALSE, &min_res_mask);
	si_otp_power(sih, TRUE, &min_res_mask);
	_ipxotp_init(oi, &otpregs);

	si_setcoreidx(sih, idx);
	return BCME_OK;
}

int
BCMNMIATTACHFN(otp_write_ones)(void *oh, uint off, uint bits)
{
	otpinfo_t *oi = (otpinfo_t *)oh;
	si_t *sih = oi->sih;
	uint idx;
	void *regs;
	otpregs_t otpregs;
	uint32 i;
	int err;
	uint32 min_res_mask = 0;

	if (off < 0 || off + bits > oi->rows * oi->cols)
		return BCME_RANGE;

	idx = si_coreidx(sih);
	if (AOB_ENAB(sih)) {
		regs = si_setcore(sih, GCI_CORE_ID, 0);
	} else {
		regs = si_setcoreidx(sih, SI_CC_IDX);
	}
	ASSERT(regs != NULL);
	otp_initregs(sih, regs, &otpregs);

	ipxotp_writable(oi, &otpregs);
	for (i = 0; i < bits; i++) {
		err = ipxotp_write_bit(oi, &otpregs, off);
		if (err != 0) {
			OTP_ERR(("%s: write bit failed: %d\n", __FUNCTION__, off));

			err = ipxotp_write_rde_nopc(oi, &otpregs,
				ipxotp_check_otp_pmu_res(oi), off, 1);
			if (err != 0)
				OTP_ERR(("%s: repair bit failed: %d\n", __FUNCTION__, off));
			else
				OTP_ERR(("%s: repair bit ok: %d\n", __FUNCTION__, off));
		}

		off++;
	}
	ipxotp_unwritable(oi, &otpregs);

	si_otp_power(oi->sih, FALSE, &min_res_mask);
	si_otp_power(oi->sih, TRUE, &min_res_mask);
	_ipxotp_init(oi, &otpregs);

	si_setcoreidx(oi->sih, idx);
	return BCME_OK;
}

#endif /* OTP_DEBUG */

static int
BCMNMIATTACHFN(ipxotp_write_bit)(otpinfo_t *oi, otpregs_t *otpregs, uint off)
{
	uint otpwt;
	int status = 0;

	otpwt = (R_REG(oi->osh, otpregs->otplayout) & OTPL_WRAP_TYPE_MASK) >> OTPL_WRAP_TYPE_SHIFT;
	BCM_REFERENCE(otpwt);

	if (OTPWRTYPE(otpwt) == OTPL_WRAP_TYPE_40NM) {
		/* Can damage fuse in 40nm so safeguard against reprogramming */
		if (ipxotp40n_read2x(oi, otpregs, off) != 1) {
			status = ipxotp40n_write_bit(oi, otpregs, off);
		} else {
			OTP_MSG(("Bit already programmed: %d\n", off));
		}
	} else if (OTPWRTYPE(otpwt) == OTPL_WRAP_TYPE_28NM) {
		/* Handle the bit Write specially for 28nm Chips as they support only byte write
		* Each bit will be handled like byte write masking off all the remaining bits to
		* ensure no fuse damage
		*/
		status = ipxotp_write_bit_common(oi, otpregs, off);
	} else {
		AND_REG(oi->osh, otpregs->otpcontrol, OTPC_PROGEN);
		if (CCREV(oi->sih->ccrev) >= 49) {
			AND_REG(oi->osh, otpregs->otpcontrol1,
				(OTPC1_CLK_EN_MASK | OTPC1_CLK_DIV_MASK));
		} else {
		    W_REG(oi->osh, otpregs->otpcontrol1, 0);
		}
		status = ipxotp_write_bit_common(oi, otpregs, off);
	}

	return status;
}

static int
BCMNMIATTACHFN(ipxotp_write_bits)(void *oh, int bn, int bits, uint8* data)
{
	otpinfo_t *oi = (otpinfo_t *)oh;
	uint idx;
	volatile void *regs;
	otpregs_t otpregs;
	int i, j;
	uint8 temp;
	int err;
	uint32 min_res_mask = 0;
	uint otpwt;
	si_t *sih = oi->sih;

	if (bn < 0 || bn + bits > oi->rows * oi->cols)
		return BCME_RANGE;

	idx = si_coreidx(sih);
	if (AOB_ENAB(sih)) {
		regs = si_setcore(sih, GCI_CORE_ID, 0);
	} else {
		regs = si_setcoreidx(sih, SI_CC_IDX);
	}
	ASSERT(regs != NULL);
	otp_initregs(sih, regs, &otpregs);

	ipxotp_writable(oi, &otpregs);

	otpwt = (R_REG(oi->osh, otpregs.otplayout) & OTPL_WRAP_TYPE_MASK) >> OTPL_WRAP_TYPE_SHIFT;
	BCM_REFERENCE(otpwt);

	{
		for (i = 0; i < bits; ) {
			temp = *data++;
			for (j = 0; j < 8; j++, i++) {
				if (i >= bits)
					break;
				if (temp & 0x01)
				{
					if (ipxotp_write_bit(oi, &otpregs, (uint)(i + bn)) != 0) {
						OTP_ERR(("%s: write bit failed: %d\n",
							__FUNCTION__, i + bn));

						err = ipxotp_write_rde_nopc(oi, &otpregs,
							ipxotp_check_otp_pmu_res(oi), i + bn, 1);
						if (err != 0) {
							OTP_ERR(("%s: repair bit failed: %d\n",
								__FUNCTION__, i + bn));
							AND_REG(oi->osh, otpregs.otpcontrol,
								~OTPC_PROGEN);
							return -1;
						} else
							OTP_ERR(("%s: repair bit ok: %d\n",
								__FUNCTION__, i + bn));
					}
				}
				temp >>= 1;
			}
		}
	}

	ipxotp_unwritable(oi, &otpregs);

	si_otp_power(sih, FALSE, &min_res_mask);
	si_otp_power(sih, TRUE, &min_res_mask);
	_ipxotp_init(oi, &otpregs);

	si_setcoreidx(sih, idx);
	return BCME_OK;
}

static int
BCMNMIATTACHFN(ipxotp_write_lock_bit)(otpinfo_t *oi, otpregs_t *otpregs, uint off)
{
	uint k, row, col;
	uint32 otpp, st;
	uint otpwt;

	otpwt = (R_REG(oi->osh, otpregs->otplayout) & OTPL_WRAP_TYPE_MASK) >> OTPL_WRAP_TYPE_SHIFT;
	BCM_REFERENCE(otpwt);

	row = off / oi->cols;
	col = off % oi->cols;

	if (OTPWRTYPE(otpwt) == OTPL_WRAP_TYPE_28NM) {
		otpp = OTPP_START_BUSY |
			((OTPPOC_PROGRAM_LOCK_28NM << OTPP_OC_SHIFT_28NM) &
			OTPP_OC_MASK_28NM) |
		    ((row << OTPP_ROW_SHIFT) & OTPPROWMASK(CCREV(oi->sih->ccrev)));
		OTP_DBG(("%s: off = %d, row = %d, otpp = 0x%x\n",
		         __FUNCTION__, off, row, otpp));
	} else {
		otpp = OTPP_START_BUSY |
			((((OTPWRTYPE(otpwt) == OTPL_WRAP_TYPE_40NM)? OTPPOC_ROW_LOCK_40NM :
			OTPPOC_ROW_LOCK) << OTPP_OC_SHIFT) & OTPP_OC_MASK) |
		    ((row << OTPP_ROW_SHIFT) & OTPPROWMASK(CCREV(oi->sih->ccrev))) |
		    ((col << OTPP_COL_SHIFT) & OTPP_COL_MASK);
		OTP_DBG(("%s: off = %d, row = %d, col = %d, otpp = 0x%x\n",
		         __FUNCTION__, off, row, col, otpp));
	}
	W_REG(oi->osh, otpregs->otpprog, otpp);

	for (k = 0;
	     ((st = R_REG(oi->osh, otpregs->otpprog)) & OTPP_START_BUSY) && (k < OTPP_TRIES);
	     k ++)
		;
	if (k >= OTPP_TRIES) {
		OTP_ERR(("\n%s: BUSY stuck: st=0x%x, count=%d\n", __FUNCTION__, st, k));
		return -1;
	}

	return 0;
}

static int
BCMNMIATTACHFN(ipxotp_otpwb16)(otpinfo_t *oi, otpregs_t *otpregs, int wn, uint16 data)
{
	uint base, i;
	int rc = 0;

	base = wn * 16;
	for (i = 0; i < 16; i++) {
		if (data & (1 << i)) {
			rc = ipxotp_write_bit(oi, otpregs, base + i);
			if (rc != 0) {
				OTP_ERR(("%s: write bit failed:%d\n", __FUNCTION__, base + i));

				rc = ipxotp_write_rde_nopc(oi, otpregs,
					ipxotp_check_otp_pmu_res(oi), base + i, 1);
				if (rc != 0) {
					OTP_ERR(("%s: repair bit failed:%d\n",
						__FUNCTION__, base + i));
					break;
				} else
					OTP_ERR(("%s: repair bit ok:%d\n", __FUNCTION__, base + i));
			}
		}
	}

	return rc;
}

/* Write OTP redundancy entry:
 *  rde - redundancy entry index (-ve for "next")
 *  bit - bit offset
 *  val - bit value
 */

/** Check if for a particular chip OTP PMU resource is available */
static int
BCMNMIATTACHFN(ipxotp_check_otp_pmu_res)(otpinfo_t *oi)
{
	switch (CHIPID(oi->sih->chip)) {
		case BCM43217_CHIP_ID:
			/* OTP PMU resource not available, hence use global rde index */
			return OTP_GLOBAL_RDE_IDX;
		default:
			/* OTP PMU resource available, hence calculate rde index */
			return -1;
	}
	return -1;
}

/** Assumes already writable and bypasses power-cycling */
static int
BCMNMIATTACHFN(ipxotp_write_rde_nopc)(void *oh, otpregs_t *otpregs, int rde, uint bit, uint val)
{
	otpinfo_t *oi = (otpinfo_t *)oh;
	uint i, temp;
	int err = BCME_OK;

#ifdef BCMDBG
	if ((rde >= (int)oi->rde_cb.offsets) || (bit >= (uint)(oi->rows * oi->cols)) || (val > 1))
		return BCME_RANGE;
#endif // endif

	if (rde < 0) {
		for (rde = 0; rde < oi->rde_cb.offsets - 1; rde++) {
			if ((oi->status & (1 << (oi->rde_cb.stat_shift + rde))) == 0)
				break;
		}
		OTP_ERR(("%s: Auto rde index %d\n", __FUNCTION__, rde));
	}
	else if (rde == OTP_GLOBAL_RDE_IDX) {
		/* Chips which do not have a OTP PMU res, OTP can't be pwr cycled from the drv. */
		/* Hence we need to have a count of the global rde, and populate accordingly. */

		/* Find the next available rde location */
		while (oi->status & (1 << (oi->rde_cb.stat_shift + oi->rde_idx))) {
			OTP_MSG(("%s: rde %d already in use, status 0x%08x\n", __FUNCTION__,
				rde, oi->status));
			oi->rde_idx++;
		}
		rde = oi->rde_idx++;

		if (rde >= MAXNUMRDES) {
			OTP_MSG(("%s: No rde location available to fix.\n", __FUNCTION__));
			return BCME_ERROR;
		}
	}

	if (oi->status & (1 << (oi->rde_cb.stat_shift + rde))) {
		OTP_ERR(("%s: rde %d already in use, status 0x%08x\n", __FUNCTION__,
		         rde, oi->status));
		return BCME_ERROR;
	}

	temp = ~(~0 << oi->rde_cb.width) &
	        ((~0 << (oi->rde_cb.val_shift + 1)) | (val << oi->rde_cb.val_shift) | bit);

	OTP_MSG(("%s: rde %d bit %d val %d bmp 0x%08x\n", __FUNCTION__, rde, bit, val, temp));

	for (i = 0; i < oi->rde_cb.width; i ++) {
		if (!(temp & (1 << i)))
			continue;
		if (ipxotp_write_bit(oi, otpregs, oi->rde_cb.offset[rde] + i) != 0)
			err = BCME_ERROR;
	}

	/* no power-cyclying to just set status */
	oi->status |= (1 << (oi->rde_cb.stat_shift + rde));

	return err;
}

int
BCMNMIATTACHFN(ipxotp_write_rde)(void *oh, int rde, uint bit, uint val)
{
	otpinfo_t *oi = (otpinfo_t *)oh;
	si_t *sih = oi->sih;
	uint idx;
	volatile void *regs;
	otpregs_t otpregs;
	int err;
	uint32 min_res_mask = 0;

	idx = si_coreidx(oi->sih);
	if (AOB_ENAB(sih)) {
		regs = si_setcore(sih, GCI_CORE_ID, 0);
	} else {
		regs = si_setcoreidx(sih, SI_CC_IDX);
	}
	ASSERT(regs != NULL);
	otp_initregs(sih, regs, &otpregs);

	/* Enable Write */
	ipxotp_writable(oi, &otpregs);

	err = ipxotp_write_rde_nopc(oh, &otpregs, rde, bit, val);

	/* Disable Write */
	ipxotp_unwritable(oi, &otpregs);

	si_otp_power(oi->sih, FALSE, &min_res_mask);
	si_otp_power(oi->sih, TRUE, &min_res_mask);
	_ipxotp_init(oi, &otpregs);

	si_setcoreidx(oi->sih, idx);
	return err;
}

/** Set up redundancy entries for the specified bits */
static int
BCMNMIATTACHFN(ipxotp_fix_word16)(void *oh, uint wn, uint16 mask, uint16 val, otpregs_t *otpregs)
{
	otpinfo_t *oi;
	uint bit;
	int rc = 0;
	BCM_REFERENCE(otpregs);

	oi = (otpinfo_t *)oh;

	ASSERT(oi != NULL);
	ASSERT(wn < oi->wsize);

	for (bit = wn * 16; mask; bit++, mask >>= 1, val >>= 1) {
		if (mask & 1) {
			if ((rc = ipxotp_write_rde(oi, ipxotp_check_otp_pmu_res(oi), bit, val & 1)))
				break;
		}
	}

	return rc;
}

static int
BCMNMIATTACHFN(ipxotp_check_word16)(void *oh, otpregs_t *otpregs, uint wn, uint16 val)
{
	otpinfo_t *oi = (otpinfo_t *)oh;
	uint16 word = ipxotp_otpr(oi, otpregs, wn);
	int rc = 0;

	if ((word ^= val)) {
		OTP_MSG(("%s: word %d is 0x%04x, wanted 0x%04x, fixing...\n",
			__FUNCTION__, wn, (word ^ val), val));

		if ((rc = ipxotp_fix_word16(oi, wn, word, val, otpregs))) {
			OTP_ERR(("FAILED, ipxotp_fix_word16 returns %d\n", rc));
			/* Fatal error, unfixable. MFGC will have to fail. Board
			 * needs to be discarded!!
			 */
			return BCME_NORESOURCE;
		}
	}

	return BCME_OK;
}

/** expects the caller to disable interrupts before calling this routine */
static int
BCMNMIATTACHFN(ipxotp_write_region)(void *oh, int region, uint16 *data, uint wlen, uint flags)
{
	otpinfo_t *oi = (otpinfo_t *)oh;
	si_t *sih = oi->sih;
	uint idx;
	volatile void *regs;
	otpregs_t otpregs;
	uint base, i;
	int otpgu_bit_base;
	bool rewrite = FALSE;
	int rc = 0;
	uint32 min_res_mask = 0;
#if defined(DONGLEBUILD)
	uint16 *origdata = NULL;
#endif /* DONGLEBUILD */

	otpgu_bit_base = oi->otpgu_base * 16;

	/* Validate region selection */
	switch (region) {
	case OTP_HW_RGN:
		if (wlen > (uint)(oi->hwlim - oi->hwbase)) {
			OTP_ERR(("%s: wlen %u exceeds OTP h/w region limit %u\n",
			         __FUNCTION__, wlen, oi->hwlim - oi->hwbase));
			return BCME_BUFTOOLONG;
		}
		rewrite = !!(oi->status & OTPS_GUP_HW);
		base = oi->hwbase;
		break;
	case OTP_SW_RGN:
		if (wlen > (uint)(oi->swlim - oi->swbase)) {
			OTP_ERR(("%s: wlen %u exceeds OTP s/w region limit %u\n",
			         __FUNCTION__, wlen, oi->swlim - oi->swbase));
			return BCME_BUFTOOLONG;
		}
		rewrite = !!(oi->status & OTPS_GUP_SW);
		base = oi->swbase;
		break;
	case OTP_CI_RGN:
		if (oi->status & OTPS_GUP_CI) {
			OTP_ERR(("%s: chipid region has been programmed\n", __FUNCTION__));
			return BCME_ERROR;
		}
		if (wlen > OTPGU_CI_SZ) {
			OTP_ERR(("%s: wlen %u exceeds OTP ci region limit %u\n",
			         __FUNCTION__, wlen, OTPGU_CI_SZ));
			return BCME_BUFTOOLONG;
		}
		if ((wlen == OTPGU_CI_SZ) && (data[OTPGU_CI_SZ - 1] & OTPGU_P_MSK) != 0) {
			OTP_ERR(("%s: subregion programmed bits not zero\n", __FUNCTION__));
			return BCME_BADARG;
		}
		base = oi->otpgu_base + OTPGU_CI_OFF;
		break;
	case OTP_FUSE_RGN:
		if (oi->status & OTPS_GUP_FUSE) {
			OTP_ERR(("%s: fuse region has been programmed\n", __FUNCTION__));
			return BCME_ERROR;
		}
		if (wlen > (uint)(oi->flim - oi->fbase)) {
			OTP_ERR(("%s: wlen %u exceeds OTP ci region limit %u\n",
			         __FUNCTION__, wlen, oi->flim - oi->fbase));
			return BCME_BUFTOOLONG;
		}
		base = oi->flim - wlen;
		break;
	case OTP_NW_RGN:
		if (wlen > (uint)(oi->hwlim - oi->hwbase)) {
			OTP_ERR(("%s: wlen %u exceeds OTP h/w region limit %u\n",
			         __FUNCTION__, wlen, oi->hwlim - oi->hwbase));
			return BCME_BUFTOOLONG;
		}
		rewrite = TRUE;
		base = OTP_NW_BASE;
		wlen = OTP_NW_SZ; /* write only 40 bytes */
		break;
	default:
		OTP_ERR(("%s: writing region %d is not supported\n", __FUNCTION__, region));
		return BCME_ERROR;
	}

	idx = si_coreidx(sih);
	if (AOB_ENAB(sih)) {
		regs = si_setcore(sih, GCI_CORE_ID, 0);
	} else {
		regs = si_setcoreidx(sih, SI_CC_IDX);
	}
	ASSERT(regs != NULL);
	otp_initregs(sih, regs, &otpregs);

#if defined(DONGLEBUILD)
	/* Check for conflict; Since some bits might be programmed at ATE time, we need to
	 * avoid redundancy by clearing already written bits, but copy original for verification.
	 */
	/* freed in same function */
	if ((origdata = (uint16*)MALLOC_NOPERSIST(oi->osh, wlen * 2)) == NULL) {
		rc = BCME_NOMEM;
		goto exit;
	}
	for (i = 0; i < wlen; i++) {
		origdata[i] = data[i];
		data[i] = ipxotp_otpr(oi, &otpregs, base + i);
		if (data[i] & ~origdata[i]) {
			OTP_ERR(("%s: %s region: word %d incompatible (0x%04x->0x%04x)\n",
				__FUNCTION__, HWSW_RGN(region), i, data[i], origdata[i]));
			rc = BCME_BADARG;
			goto exit;
		}
		data[i] ^= origdata[i];
	}
#endif /* DONGLEBUILD */

	OTP_MSG(("%s: writing new bits in %s region\n", __FUNCTION__, HWSW_RGN(region)));

	/* Enable Write */
	ipxotp_writable(oi, &otpregs);

	/* Write the data */
	for (i = 0; i < wlen; i++) {
		rc = ipxotp_otpwb16(oi, &otpregs, base + i, data[i]);
		if (rc != 0) {
			OTP_ERR(("%s: otpwb16 failed: %d 0x%x\n", __FUNCTION__, base + i, data[i]));
			ipxotp_unwritable(oi, &otpregs);
			goto exit;
		}
	}

	/* One time set region flag: Update boundary/flag in memory and in OTP */
	if (!rewrite) {
		switch (region) {
		case OTP_HW_RGN:
			/* OTP unification */
			if (oi->buotp) {
				ipxotp_otpwb16(oi, &otpregs, oi->otpgu_base + OTPGU_HSB_OFF,
					((base + oi->usbmanfid_offset) * 16));
				ipxotp_write_bit(oi, &otpregs, otpgu_bit_base + OTPGU_SWP_OFF);
			} else
				ipxotp_otpwb16(oi, &otpregs, oi->otpgu_base + OTPGU_HSB_OFF,
				(base + i) * 16);
			ipxotp_write_bit(oi, &otpregs, otpgu_bit_base + OTPGU_HWP_OFF);
			/* Set new CIS format bit when the ciswrite command ask us
			 * to do so, or for listed chips
			 */
			if (0 ||
#if (!defined(_CFE_) && !defined(_CFEZ_)) || defined(CFG_WL)
				(flags & CISH_FLAG_PCIECIS) ||
#endif /* (!_CFE_ && !_CFEZ_) || CFG_WL */
				((BCM4350_CHIP(oi->sih->chip)) &&
				CST4350_CHIPMODE_SDIOD(oi->sih->chipst)) ||
				((BCM4347_CHIP(oi->sih->chip)) &&
				CST4347_CHIPMODE_SDIOD(oi->sih->chipst)) ||
				0)
				ipxotp_write_bit(oi, &otpregs, otpgu_bit_base +
					OTPGU_NEWCISFORMAT_OFF);
			break;
		case OTP_SW_RGN:
			/* Write HW region limit as well */
			ipxotp_otpwb16(oi, &otpregs, oi->otpgu_base + OTPGU_HSB_OFF, base * 16);
			/* write max swlim(covert to bits) to the sw/fuse boundary */
			ipxotp_otpwb16(oi, &otpregs, oi->otpgu_base + OTPGU_SFB_OFF,
				oi->swlim * 16);
			ipxotp_write_bit(oi, &otpregs, otpgu_bit_base + OTPGU_SWP_OFF);
			break;
		case OTP_CI_RGN:
			ipxotp_write_bit(oi, &otpregs, otpgu_bit_base + OTPGU_CIP_OFF);
			/* Also set the OTPGU_CIP_MSK bit in the input so verification
			 * doesn't fail
			 */
			if (wlen >= OTPGU_CI_SZ)
				data[OTPGU_CI_SZ - 1] |= OTPGU_CIP_MSK;
			break;
		case OTP_FUSE_RGN:
			ipxotp_otpwb16(oi, &otpregs, oi->otpgu_base + OTPGU_SFB_OFF, base * 16);
			ipxotp_write_bit(oi, &otpregs, otpgu_bit_base + OTPGU_FUSEP_OFF);
			break;
		}
	}

	/* Disable Write */
	ipxotp_unwritable(oi, &otpregs);

	/* Sync region info by retrieving them again (use PMU bit to power cycle OTP) */
	si_otp_power(oi->sih, FALSE, &min_res_mask);
	si_otp_power(oi->sih, TRUE, &min_res_mask);

	/* Check and fix for region size and region programmed bits */
	if (!rewrite) {
		uint16	boundary_off = 0, boundary_val = 0;
		uint16	programmed_off = 0;
		uint16	bit = 0;

		switch (region) {
		case OTP_HW_RGN:
			boundary_off = OTPGU_HSB_OFF;
			/* OTP unification */
			if (oi->buotp) {
				boundary_val = ((base + oi->usbmanfid_offset) * 16);
			} else
				boundary_val = (base + i) * 16;
			programmed_off = OTPGU_HWP_OFF;
			break;
		case OTP_SW_RGN:
			/* Also write 0 to HW region boundary */
			if ((rc = ipxotp_check_word16(oi, &otpregs, oi->otpgu_base + OTPGU_HSB_OFF,
				base * 16)))
				goto exit;
			boundary_off = OTPGU_SFB_OFF;
			boundary_val = oi->swlim * 16;
			programmed_off = OTPGU_SWP_OFF;
			break;
		case OTP_CI_RGN:
			/* No CI region boundary */
			programmed_off = OTPGU_CIP_OFF;
			break;
		case OTP_FUSE_RGN:
			boundary_off = OTPGU_SFB_OFF;
			boundary_val = base * 16;
			programmed_off = OTPGU_FUSEP_OFF;
			break;
		}

		/* Do the actual checking and return BCME_NORESOURCE if we cannot fix */
		if ((region != OTP_CI_RGN) &&
			(rc = ipxotp_check_word16(oi, &otpregs, oi->otpgu_base + boundary_off,
			boundary_val))) {
			goto exit;
		}

		if ((bit = _ipxotp_read_bit(oh, &otpregs, otpgu_bit_base + programmed_off))
			== 0xffff) {
			OTP_ERR(("\n%s: FAILED bit %d reads %d\n", __FUNCTION__, otpgu_bit_base +
				programmed_off, bit));
			goto exit;
		} else if (bit == 0) {	/* error detected, fix it */
			OTP_ERR(("\n%s: FAILED bit %d reads %d, fixing\n", __FUNCTION__,
				otpgu_bit_base + programmed_off, bit));
			if ((rc = ipxotp_write_rde(oi, ipxotp_check_otp_pmu_res(oi),
				otpgu_bit_base + programmed_off, 1))) {
				OTP_ERR(("\n%s: cannot fix, ipxotp_write_rde returns %d\n",
					__FUNCTION__, rc));
				goto exit;
			}
		}
	}

	/* Update status, apply WAR */
	_ipxotp_init(oi, &otpregs);

#if defined(DONGLEBUILD)
	/* Recover original data... */
	if (origdata)
		bcopy(origdata, data, wlen * 2);
#endif /* DONGLEBUILD */

	/* ...Check again so we can verify and fix where possible */
	for (i = 0; i < wlen; i++) {
		if ((rc = ipxotp_check_word16(oi, &otpregs, base + i, data[i])))
			goto exit;
	}

exit:
#if defined(DONGLEBUILD)
	if (origdata)
		MFREE(oi->osh, origdata, wlen * 2);
#endif /* DONGLEBUILD */
	si_setcoreidx(sih, idx);
	return rc;
}

static int
BCMNMIATTACHFN(ipxotp_write_word)(void *oh, uint wn, uint16 data)
{
	otpinfo_t *oi = (otpinfo_t *)oh;
	si_t *sih = oi->sih;
	int rc = 0;
	uint16 origdata;
	uint idx;
	volatile void *regs;
	otpregs_t otpregs;

	idx = si_coreidx(sih);
	if (AOB_ENAB(sih)) {
		regs = si_setcore(sih, GCI_CORE_ID, 0);
	} else {
		regs = si_setcoreidx(sih, SI_CC_IDX);
	}
	ASSERT(regs != NULL);
	otp_initregs(sih, regs, &otpregs);

	/* Check for conflict */
	origdata = data;
	data = ipxotp_otpr(oi, &otpregs, wn);
	if (data & ~origdata) {
		OTP_ERR(("%s: word %d incompatible (0x%04x->0x%04x)\n",
			__FUNCTION__, wn, data, origdata));
		rc = BCME_BADARG;
		goto exit;
	}
	data ^= origdata;

	/* Enable Write */
	ipxotp_writable(oi, &otpregs);

	rc = ipxotp_otpwb16(oi, &otpregs, wn, data);

	/* Disable Write */
	ipxotp_unwritable(oi, &otpregs);

	data = origdata;
	if ((rc = ipxotp_check_word16(oi, &otpregs, wn, data)))
		goto exit;
exit:
	si_setcoreidx(sih, idx);
	return rc;
}

/**
 * Appends the caller supplied CIS tuple(s) to OTP. This function is able to deal with both
 * unprogrammed and (partially) programmed OTP.
 *
 * XXX Write (append) OTP CIS
 * For blank OTP, add 0x2 0x1 0xff 0xff 0x00 0xff 0x00 at the end of region then write
 * For rewrite, append the content
 * http://hwnbu-twiki.broadcom.com/bin/view/Mwgroup/BootLoader4322usb#Ending_pattern_is_not_reliabl
 */
static int
BCMNMIATTACHFN(_ipxotp_cis_append_region)(si_t *sih, int region, char *vars, int count)
{
#define TUPLE_MATCH(_s1, _s2, _s3, _v1, _v2, _v3) \
	(((_s1) == 0x80) ? \
		(((_s1) == (_v1)) && ((_s2) == (_v2)) && ((_s3) == (_v3))) \
		: (((_s1) == (_v1)) && ((_s2) == (_v2))))

	uint8 *cis;
	osl_t *osh;
	uint sz = OTP_SZ_MAX/2; /* size in words */
	int rc = 0;
	bool newchip = FALSE;
	uint overwrite = 0;
	uint overwrite_check = 0;

	ASSERT(region == OTP_HW_RGN || region == OTP_SW_RGN);

	osh = si_osh(sih);
	if ((cis = MALLOC(osh, OTP_SZ_MAX)) == NULL) {
		return BCME_ERROR;
	}

	bzero(cis, OTP_SZ_MAX);

	rc = otp_read_region(sih, region, (uint16 *)cis, &sz);
	newchip = (rc == BCME_NOTFOUND) ? TRUE : FALSE;
	if ((rc != 0) && (rc != BCME_NOTFOUND)) {
		return BCME_ERROR;
	}
	rc = 0;

	/* zero count for read, non-zero count for write */
	if (count) {
		int i = 0, newlen = 0;

		if (newchip) {
			int termw_len = 0;	/* length of termination word */

			/* convert halfwords to bytes offset */
			newlen = sz * 2;

			cis[newlen-1] = 0xff;
			cis[newlen-2] = 0xff;
			termw_len = 2;

			if (count >= newlen - termw_len) {
				OTP_MSG(("OTP left %x bytes; buffer %x bytes\n", newlen, count));
				rc = BCME_BUFTOOLONG;
			}
		} else {
			int end = 0;
			if (region == OTP_SW_RGN) {
				/* Walk through the leading zeros (could be 0 or 8 bytes for now) */
				for (i = 0; i < (int)sz*2; i++)
					if (cis[i] != 0)
						break;

				/* PR 115832: OTP programmed but no tuples
				 *  Minus 2 accounts for terminator of 0xff or 0xffff
				 */
				if (i >= (int)((sz*2) - 2))
					i = 0;
			} else {
				/* move pass the hardware header */
				if ((CCREV(sih->ccrev) >= 45) &&
					(BUSCORETYPE(sih->buscoretype) == PCIE2_CORE_ID)) {
					/* PCIE GEN2 */
					i += otp_pcie_hwhdr_sz(sih);
				} else if (CCREV(sih->ccrev) >= 36) {
					uint32 otp_layout;

					otp_layout = si_corereg(sih, GCI_CORE_IDX(sih),
						GCI_OFFSETOF(sih, otplayout), 0, 0);

					if (otp_layout & OTP_CISFORMAT_NEW) {
						i += 12; /* new sdio header format, 6 half words */
					} else {
						i += 8; /* old sdio header format */
					}
				} else {
					return BCME_ERROR; /* old chip, not suppported */
				}
			}

			/* Find the place to append */
			for (; i < (int)sz*2; i++) {
				if (cis[i] == 0)
					break;

				/* find the last tuple with same BMCM HNBU */
				if (TUPLE_MATCH(cis[i], cis[i+1], cis[i+2],
					vars[0], vars[1], vars[2])) {
					overwrite = i;
					/* New variable is used to make sure even if i=0, overwrite
					* functionality will work properly
					*/
					overwrite_check = 1;
				}
				i += ((int)cis[i+1] + 1);
			}
			for (end = i; end < (int)sz*2; end++) {
				if (cis[end] != 0)
					break;
			}

			/* If the tuple exist, check if it can be overwritten */
			if (overwrite_check)
			{
				int j;

				/* found, check if it is compiatable for fix */
				for (j = 0; j < cis[overwrite+1] + 2; j++) {
					if ((cis[overwrite+j] ^ vars[j]) & cis[overwrite+j]) {
						break;
					}
				}
				if (j == cis[overwrite+1] + 2) {
					i = overwrite;
				}
			}

			newlen = i + count;
			if (newlen & 1)		/* make it even-sized buffer */
				newlen++;

			if (newlen >= (end - 1)) {
				OTP_MSG(("OTP left %x bytes; buffer %x bytes\n", end-i, count));
				rc = BCME_BUFTOOLONG;
			}
		}

		/* copy the buffer */
		memcpy(&cis[i], vars, count);
#ifdef BCMNVRAMW
		/* Write the buffer back */
		if (!rc)
			rc = otp_write_region(sih, region, (uint16*)cis, newlen/2, 0);

		/* Print the buffer */
		OTP_MSG(("Buffer of size %d bytes to write:\n", newlen));
		for (i = 0; i < newlen; i++) {
			OTP_DBG(("%02x ", cis[i] & 0xff));
			if ((i % 16) == 15) {
				OTP_DBG(("\n"));
			}
		}
		OTP_MSG(("\n"));
#endif /* BCMNVRAMW */
	}
	if (cis)
		MFREE(osh, cis, OTP_SZ_MAX);

	return (rc);
}

/**
 * given a caller supplied CIS (in *vars), appends the tuples in the CIS to the existing CIS in
 * OTP. Tuples are appended to extend the CIS, or to overrule prior written tuples.
 */
static int
BCMNMIATTACHFN(ipxotp_cis_append_region)(si_t *sih, int region, char *vars, int count)
{
	int result;
	char *tuple;
	int tuplePos;
	int tupleCount;
	int remainingCount;

	result = BCME_ERROR;

	/* zero count for read, non-zero count for write */
	if (count) {
		if (count < (int)(*(vars + 1) + 2)) {
			OTP_MSG(("vars (count): %d bytes; tuple len: %d bytes\n",
				count, (int)(*(vars + 1))));
			return BCME_ERROR;
		}

		/* get first tuple from vars. */
		tuplePos = 0;
		remainingCount = count;

		/* separate vars into tuples and write tuples one by one to OTP. */
		do {
			tupleCount = (int)(*(vars + tuplePos + 1) + 2);
			if (remainingCount < tupleCount) {
				OTP_MSG(("vars (remainingCount): %d bytes; tuple len: %d bytes\n",
					remainingCount, tupleCount));
				return (result);
			}
			tuple = vars + tuplePos;

			result = _ipxotp_cis_append_region(sih, region, tuple, tupleCount);
			if (result != BCME_OK)
				return (result);

			/* get next tuple from vars. */
			remainingCount -= tupleCount;
			tuplePos += (((int)*(tuple + 1)) + 2);
		} while (remainingCount > 0);
	} else {
		return (_ipxotp_cis_append_region(sih, region, vars, count));
	}

	return (result);
}

/* No need to lock for IPXOTP */
static int
BCMNMIATTACHFN(ipxotp_lock)(void *oh)
{
	uint idx;
	volatile void *regs;
	otpregs_t otpregs;
	otpinfo_t *oi = (otpinfo_t *)oh;
	si_t *sih = oi->sih;
	int err = 0, rc = 0;
	uint32 min_res_mask = 0;

	idx = si_coreidx(sih);
	if (AOB_ENAB(sih)) {
		regs = si_setcore(sih, GCI_CORE_ID, 0);
	} else {
		regs = si_setcoreidx(sih, SI_CC_IDX);
	}
	ASSERT(regs != NULL);
	otp_initregs(sih, regs, &otpregs);

	/* Enable Write */
	ipxotp_writable(oi, &otpregs);

	err = ipxotp_write_lock_bit(oi, &otpregs, OTP_LOCK_ROW1_LOC_OFF);
	if (err) {
		OTP_ERR(("fail to lock ROW1\n"));
		rc = -1;
	}
	err =  ipxotp_write_lock_bit(oi, &otpregs, OTP_LOCK_ROW2_LOC_OFF);
	if (err) {
		OTP_ERR(("fail to lock ROW2\n"));
		rc = -2;
	}
	err = ipxotp_write_lock_bit(oi, &otpregs, OTP_LOCK_RD_LOC_OFF);
	if (err) {
		OTP_ERR(("fail to lock RD\n"));
		rc = -3;
	}
	err = ipxotp_write_lock_bit(oi, &otpregs, OTP_LOCK_GU_LOC_OFF);
	if (err) {
		OTP_ERR(("fail to lock GU\n"));
		rc = -4;
	}

	/* Disable Write */
	ipxotp_unwritable(oi, &otpregs);

	/* Sync region info by retrieving them again (use PMU bit to power cycle OTP) */
	si_otp_power(sih, FALSE, &min_res_mask);
	si_otp_power(sih, TRUE, &min_res_mask);

	/* Update status, apply WAR */
	_ipxotp_init(oi, &otpregs);

	si_setcoreidx(sih, idx);

	return rc;
}

static int
BCMNMIATTACHFN(ipxotp_nvwrite)(void *oh, uint16 *data, uint wlen)
{
	BCM_REFERENCE(oh);
	BCM_REFERENCE(data);
	BCM_REFERENCE(wlen);

	return -1;
}

/* ECC related functions for 28nm -start  */
/* This function programs the OTP row depending on the ECC type
*  Data will be written into GCI Progdata
*/
static int
BCMNMIATTACHFN(_ipxotp_ecc_write)(void *oh, otpregs_t *otpregs, uint wn, uint32 data,
	uint32 ecc_type)
{
	uint k, row = 0;
	uint32 otpp, st;
	otpinfo_t *oi = (otpinfo_t *)oh;
	uint32 val = 0;
	uint32 origdata = 0;
	uint32 progdata = 0;
	int ret = 0;

	/* Make sure row offset is within OTP size */
	if (wn >= oi->rows) {
		ret = BCME_ERROR;
		goto out;
	}
	row = wn;
	/* Depending on the ECC type , program OTP Differently */
	if ((ecc_type != OTPPOC_PROG_28NM) && (ecc_type != OTPPOC_PROG_ECC_28NM) &&
		(ecc_type != OTPPOC_PROG_ECC_READ_28NM)) {
		ret = BCME_ERROR;
		goto out;
	}
	val = R_REG(oi->osh, &oi->otpbase[row * 2]);
	val = val | (R_REG(oi->osh, &oi->otpbase[row * 2 + 1]) << 16);
	origdata = val;

	/* XOR the Existing Row data with the incoming write data to properly
	* update the OTP bits without damaging it
	*/
	progdata = data ^ val;

	/* Program the data to GCI Program data register(OTPProgData 0x328) */
	W_REG(oi->osh, otpregs->otpprogdata, progdata);

	/* Program OTP opcode in OTPProg Register (OTPProg 0x318) */
	otpp = OTPP_START_BUSY |
		((ecc_type << OTPP_OC_SHIFT_28NM) & OTPP_OC_MASK_28NM) |
			((row << OTPP_ROW_SHIFT) & OTPPROWMASK(CCREV(oi->sih->ccrev)));

	OTP_DBG(("%s:row = %d, otpp = 0x%x ecc_type = %d origdata = [0x%x]"
		"progdata = [0x%x] data =[0x%x]\n",
		__FUNCTION__, row, otpp, ecc_type, origdata, progdata, data));

	W_REG(oi->osh, otpregs->otpprog, otpp);

	for (k = 0;
	     ((st = R_REG(oi->osh, otpregs->otpprog)) & OTPP_START_BUSY) && (k < OTPP_TRIES);
	     k ++)
		;
	if (k >= OTPP_TRIES) {
		OTP_ERR(("\n%s: BUSY stuck: st=0x%x, count=%d\n", __FUNCTION__, st, k));
		ret = BCME_ERROR;
		goto out;
	}
	/* Read back the value to make sure , OTP is Programmed Properly */
	val = R_REG(oi->osh, &oi->otpbase[row * 2]);
	val = val | (R_REG(oi->osh, &oi->otpbase[row * 2 + 1]) << 16);

	if (val != (origdata | data)) {
		OTP_DBG(("%s: row = %d, origdata = 0x%x Existing data = 0x%x Write data = 0x%x"
			" otpprog =[0x%x]is not matching --> OTP ECC Error happened \n",
			__FUNCTION__, row, origdata, val, data, *otpregs->otpprog));
		ret = BCME_ERROR;
		goto out;
	}
out:
	return ret;
}

static int
BCMNMIATTACHFN(ipxotp_ecc_write)(void *oh, uint wn, uint32 data, uint32 ecc_type)
{
	otpinfo_t *oi = (otpinfo_t *)oh;
	si_t *sih = oi->sih;
	int rc = 0;
	uint idx;
	volatile void *regs;
	otpregs_t otpregs;
	uint otpwt;

	idx = si_coreidx(sih);
	if (AOB_ENAB(sih)) {
		regs = si_setcore(sih, GCI_CORE_ID, 0);
	} else {
		regs = si_setcoreidx(sih, SI_CC_IDX);
	}
	ASSERT(regs != NULL);

	otp_initregs(sih, regs, &otpregs);
	/* Check the OTP Wrapper Type and return Error for cases other than 28nm Chips */
	otpwt = (R_REG(oi->osh, otpregs.otplayout) & OTPL_WRAP_TYPE_MASK) >> OTPL_WRAP_TYPE_SHIFT;
	BCM_REFERENCE(otpwt);
	if (OTPWRTYPE(otpwt) != OTPL_WRAP_TYPE_28NM) {
		return BCME_UNSUPPORTED;
	}
	/* Enable Write */
	ipxotp_writable(oi, &otpregs);

	rc = _ipxotp_ecc_write(oi, &otpregs, wn, data, ecc_type);

	/* Disable Write */
	ipxotp_unwritable(oi, &otpregs);

	si_setcoreidx(sih, idx);
	return rc;
}

static int
BCMNMIATTACHFN(ipxotp_ecc_rows_dump)(void *oh, int *arg, char *buf, uint size)
{
	otpinfo_t *oi = (otpinfo_t *)oh;
	si_t *sih = oi->sih;
	volatile void *regs;
	otpregs_t otpregs;
	uint idx, i;
	uint otpwt;
	uint16 *outbuf = (uint16 *)buf;
	wl_otpecc_rows_t read_cmd;

	memcpy(&read_cmd, (uint8 *)arg, sizeof(read_cmd));

	idx = si_coreidx(sih);
	if (AOB_ENAB(sih)) {
		regs = si_setcore(sih, GCI_CORE_ID, 0);
	} else {
		regs = si_setcoreidx(sih, SI_CC_IDX);
	}
	otp_initregs(sih, regs, &otpregs);

	/* Check the OTP Wrapper Type and return Error for cases other than 28nm Chips */
	otpwt = (R_REG(oi->osh, otpregs.otplayout) & OTPL_WRAP_TYPE_MASK) >> OTPL_WRAP_TYPE_SHIFT;
	BCM_REFERENCE(otpwt);
	if (OTPWRTYPE(otpwt) != OTPL_WRAP_TYPE_28NM) {
		return BCME_UNSUPPORTED;
	}

	OTP_DBG(("ipxotp_ecc_rows_dump, count %d, offset %d\n",
		read_cmd.numrows, read_cmd.rowoffset));

	for (i = 0; i < read_cmd.numrows; i++) {
		outbuf[WL_ECCDUMP_ROW_SIZE_WORD * i] =
			ipxotp_otpr(oh, &otpregs, (i + read_cmd.rowoffset) * 2);
		outbuf[WL_ECCDUMP_ROW_SIZE_WORD * i + 1] =
			ipxotp_otpr(oh, &otpregs, (i + read_cmd.rowoffset) * 2 + 1);
		outbuf[WL_ECCDUMP_ROW_SIZE_WORD * i + 2] =
			(uint16)R_REG(oi->osh, otpregs.otpECCstatus);
	}

	si_setcoreidx(oi->sih, idx);

	return (WL_ECCDUMP_ROW_SIZE_WORD * i);
}

/* ECC related functions for 28nm -end  */

#endif /* BCMNVRAMW */

#if defined(WLTEST)
static int
BCMNMIATTACHFN(ipxotp_dump)(void *oh, int arg, char *buf, uint size)
{
	otpinfo_t *oi = (otpinfo_t *)oh;
	si_t *sih = oi->sih;
	volatile void *regs;
	otpregs_t otpregs;
	uint idx, i, count;
	uint16 val;
	struct bcmstrbuf b;

	idx = si_coreidx(sih);
	if (AOB_ENAB(sih)) {
		regs = si_setcore(sih, GCI_CORE_ID, 0);
	} else {
		regs = si_setcoreidx(sih, SI_CC_IDX);
	}
	otp_initregs(sih, regs, &otpregs);

	count = ipxotp_size(oh);

	bcm_binit(&b, buf, size);
	for (i = 0; i < count / 2; i++) {
		if (!(i % 4))
			bcm_bprintf(&b, "\n0x%04x:", 2 * i);
		if (arg == 0)
			val = ipxotp_otpr(oh, &otpregs, i);
		else
			val = ipxotp_otprb16(oi, &otpregs, i);
		bcm_bprintf(&b, " 0x%04x", val);
	}
	bcm_bprintf(&b, "\n");

	si_setcoreidx(oi->sih, idx);

	return ((int)(b.buf - b.origbuf));
}

static int
BCMNMIATTACHFN(ipxotp_read)(void *oh, void *arg, char *buf, uint size)
{
#if (!defined(_CFE_) && !defined(_CFEZ_)) || defined(CFG_WL)
	otpinfo_t *oi = (otpinfo_t *)oh;
	si_t *sih = oi->sih;
	volatile void *regs;
	otpregs_t otpregs;
	uint idx, i, count, offset, version;
	wl_otpread_cmd_t read_cmd;
	uint16 *outbuf = (uint16 *)buf;

	memcpy(&read_cmd, (uint8 *)arg, sizeof(read_cmd));

	version = read_cmd.version;
	count = read_cmd.rdsize;
	offset = read_cmd.rdoffset;

	if (version > WL_OTPREAD_VER) {
		return BCME_VERSION;
	}
	idx = si_coreidx(sih);
	if (AOB_ENAB(sih)) {
		regs = si_setcore(sih, GCI_CORE_ID, 0);
	} else {
		regs = si_setcoreidx(sih, SI_CC_IDX);
	}
	otp_initregs(sih, regs, &otpregs);

	for (i = 0; i < count / 2; i++) {
		outbuf[i] = ipxotp_otpr(oh, &otpregs, i + offset / 2);
	}

	si_setcoreidx(oi->sih, idx);

	return count;
#else
	return 0;
#endif /* (!_CFE_  || !_CFEZ_) || CFG_WL */
}
#endif // endif

static otp_fn_t BCMCISDUMPATTACHDATA(ipxotp_fn) = {
	(otp_size_t)ipxotp_size,
	(otp_read_bit_t)ipxotp_read_bit,
	(otp_dump_t)NULL,		/* Assigned in otp_init */
	(otp_status_t)ipxotp_status,

	(otp_init_t)ipxotp_init,
	(otp_read_region_t)ipxotp_read_region,
	(otp_nvread_t)ipxotp_nvread,
#ifdef BCMNVRAMW
	(otp_write_region_t)ipxotp_write_region,
	(otp_cis_append_region_t)ipxotp_cis_append_region,
	(otp_lock_t)ipxotp_lock,
	(otp_nvwrite_t)ipxotp_nvwrite,
	(otp_write_word_t)ipxotp_write_word,
#else /* BCMNVRAMW */
	(otp_write_region_t)NULL,
	(otp_cis_append_region_t)NULL,
	(otp_lock_t)NULL,
	(otp_nvwrite_t)NULL,
	(otp_write_word_t)NULL,
#endif /* BCMNVRAMW */
	(otp_read_word_t)ipxotp_read_word,
#ifdef BCMNVRAMW
	(otp_write_bits_t)ipxotp_write_bits,
	(otp_ecc_write_t)ipxotp_ecc_write,	/* ECC related code */
	(otp_ecc_rows_dump_t)ipxotp_ecc_rows_dump,
#else
	(otp_write_bits_t)NULL,
	(otp_ecc_write_t)NULL,
	(otp_ecc_rows_dump_t)NULL,
#endif /* BCMNVRAMW */
	(otp_isunified_t)ipxotp_isunified,
	(otp_avsbitslen_t)ipxotp_avsbitslen,
	(otp_read_t)NULL			/* Assigned in otp_init */
};

/*
 * ROM accessor to avoid struct in shdat. Declare BCMRAMFN() to force the accessor to be excluded
 * from ROM.
 */
static otp_fn_t*
BCMRAMFN(get_ipxotp_fn)(void)
{
	return &ipxotp_fn;
}
#endif /* BCMIPXOTP */

/*
 * HND OTP Code
 *
 *   Exported functions:
 *	hndotp_status()
 *	hndotp_size()
 *	hndotp_init()
 *	hndotp_read_bit()
 *	hndotp_read_region()
 *	hndotp_read_word()
 *	hndotp_nvread()
 *	hndotp_write_region()
 *	hndotp_cis_append_region()
 *	hndotp_lock()
 *	hndotp_nvwrite()
 *	hndotp_dump()
 *	hndotp_isunified()
 *
 *   HND internal functions:
 *	hndotp_otpr()
 *	hndotp_write_bit()
 *	hndotp_write_word()
 *	hndotp_valid_rce()
 *	hndotp_write_rce()
 *	hndotp_write_row()
 *	hndotp_otprb16()
 *
 */

#ifdef BCMHNDOTP

/* Fields in otpstatus */
#define	OTPS_PROGFAIL		0x80000000
#define	OTPS_PROTECT		0x00000007
#define	OTPS_HW_PROTECT		0x00000001
#define	OTPS_SW_PROTECT		0x00000002
#define	OTPS_CID_PROTECT	0x00000004
#define	OTPS_RCEV_MSK		0x00003f00
#define	OTPS_RCEV_SHIFT		8

/* Fields in the otpcontrol register */
#define	OTPC_RECWAIT		0xff000000
#define	OTPC_PROGWAIT		0x00ffff00
#define	OTPC_PRW_SHIFT		8
#define	OTPC_MAXFAIL		0x00000038
#define	OTPC_VSEL		0x00000006
#define	OTPC_SELVL		0x00000001

/* OTP regions (Word offsets from otp size) */
#define	OTP_SWLIM_OFF	(-4)
#define	OTP_CIDBASE_OFF	0
#define	OTP_CIDLIM_OFF	4

/* Predefined OTP words (Word offset from otp size) */
#define	OTP_BOUNDARY_OFF (-4)
#define	OTP_HWSIGN_OFF	(-3)
#define	OTP_SWSIGN_OFF	(-2)
#define	OTP_CIDSIGN_OFF	(-1)
#define	OTP_CID_OFF	0
#define	OTP_PKG_OFF	1
#define	OTP_FID_OFF	2
#define	OTP_RSV_OFF	3
#define	OTP_LIM_OFF	4
#define	OTP_RD_OFF	4	/* Redundancy row starts here */
#define	OTP_RC0_OFF	28	/* Redundancy control word 1 */
#define	OTP_RC1_OFF	32	/* Redundancy control word 2 */
#define	OTP_RC_LIM_OFF	36	/* Redundancy control word end */

#define	OTP_HW_REGION	OTPS_HW_PROTECT
#define	OTP_SW_REGION	OTPS_SW_PROTECT
#define	OTP_CID_REGION	OTPS_CID_PROTECT

#if OTP_HW_REGION != OTP_HW_RGN
#error "incompatible OTP_HW_RGN"
#endif // endif
#if OTP_SW_REGION != OTP_SW_RGN
#error "incompatible OTP_SW_RGN"
#endif // endif
#if OTP_CID_REGION != OTP_CI_RGN
#error "incompatible OTP_CI_RGN"
#endif // endif

/* Redundancy entry definitions */
#define	OTP_RCE_ROW_SZ		6
#define	OTP_RCE_SIGN_MASK	0x7fff
#define	OTP_RCE_ROW_MASK	0x3f
#define	OTP_RCE_BITS		21
#define	OTP_RCE_SIGN_SZ		15
#define	OTP_RCE_BIT0		1

#define	OTP_WPR		4
#define	OTP_SIGNATURE	0x578a
#define	OTP_MAGIC	0x4e56

static int
BCMNMIATTACHFN(hndotp_status)(void *oh)
{
	otpinfo_t *oi = (otpinfo_t *)oh;
	return ((int)(oi->hwprot | oi->signvalid));
}

static uint16
hndotp_avsbitslen(void *oh)
{
	BCM_REFERENCE(oh);
	return 0;
}

static bool
BCMNMIATTACHFN(hndotp_isunified)(void *oh)
{
	BCM_REFERENCE(oh);
	return FALSE;
}

static int
BCMNMIATTACHFN(hndotp_size)(void *oh)
{
	otpinfo_t *oi = (otpinfo_t *)oh;
	return ((int)(oi->size));
}

static uint16
BCMNMIATTACHFN(hndotp_otpr)(void *oh, chipcregs_t *cc, uint wn)
{
	otpinfo_t *oi = (otpinfo_t *)oh;
	osl_t *osh;
	volatile uint16 *ptr;

	ASSERT(wn < ((oi->size / 2) + OTP_RC_LIM_OFF));
	ASSERT(cc != NULL);

	osh = si_osh(oi->sih);

	ptr = (volatile uint16 *)((volatile char *)cc + CC_SROM_OTP);
	return (R_REG(osh, &ptr[wn]));
}

static uint16
BCMNMIATTACHFN(hndotp_read_bit)(void *oh, chipcregs_t *cc, uint idx)
{
	otpinfo_t *oi = (otpinfo_t *)oh;
	uint k, row, col;
	uint32 otpp, st;
	osl_t *osh;

	osh = si_osh(oi->sih);
	row = idx / 65;
	col = idx % 65;

	otpp = OTPP_START_BUSY | OTPP_READ |
	        ((row << OTPP_ROW_SHIFT) & OTPP_ROW_MASK) |
	        (col & OTPP_COL_MASK);

	OTP_DBG(("%s: idx = %d, row = %d, col = %d, otpp = 0x%x", __FUNCTION__,
	         idx, row, col, otpp));

	W_REG(osh, &cc->otpprog, otpp);
	st = R_REG(osh, &cc->otpprog);
	for (k = 0; ((st & OTPP_START_BUSY) == OTPP_START_BUSY) && (k < OTPP_TRIES); k++)
		st = R_REG(osh, &cc->otpprog);

	if (k >= OTPP_TRIES) {
		OTP_ERR(("\n%s: BUSY stuck: st=0x%x, count=%d\n", __FUNCTION__, st, k));
		return 0xffff;
	}
	if (st & OTPP_READERR) {
		OTP_ERR(("\n%s: Could not read OTP bit %d\n", __FUNCTION__, idx));
		return 0xffff;
	}
	st = (st & OTPP_VALUE_MASK) >> OTPP_VALUE_SHIFT;
	OTP_DBG((" => %d\n", st));
	return (uint16)st;
}

static void *
BCMNMIATTACHFN(hndotp_init)(si_t *sih)
{
	uint idx;
	chipcregs_t *cc;
	otpinfo_t *oi;
	void *ret = NULL;
	osl_t *osh;

	OTP_MSG(("%s: Use HND OTP controller\n", __FUNCTION__));

	oi = get_otpinfo();

	idx = si_coreidx(sih);
	osh = si_osh(oi->sih);

	/* Check for otp */
	if ((cc = si_setcoreidx(sih, SI_CC_IDX)) != NULL) {
		uint32 cap = R_REG(osh, &cc->capabilities);

		if ((cap & CC_CAP_OTPSIZE) == 0) {
			/* Nothing there */
			goto out;
		}

		return NULL;
	}

	OTP_MSG(("%s: ccrev %d\tsize %d bytes\thwprot %x\tsignvalid %x\tboundary %x\n",
		__FUNCTION__, CCREV(oi->ccrev), oi->size, oi->hwprot, oi->signvalid,
		oi->boundary));

out:	/* All done */
	si_setcoreidx(sih, idx);

	return ret;
}

static int
BCMNMIATTACHFN(hndotp_read_region)(void *oh, int region, uint16 *data, uint *wlen)
{
	otpinfo_t *oi = (otpinfo_t *)oh;
	uint32 idx, st;
	chipcregs_t *cc;
	int i;

	/* Only support HW region (no active chips use HND OTP SW region) */
	ASSERT(region == OTP_HW_REGION);

	OTP_MSG(("%s: region %x wlen %d\n", __FUNCTION__, region, *wlen));

	/* Region empty? */
	st = oi->hwprot | oi-> signvalid;
	if ((st & region) == 0)
		return BCME_NOTFOUND;

	*wlen = ((int)*wlen < oi->boundary/2) ? *wlen : (uint)oi->boundary/2;

	idx = si_coreidx(oi->sih);
	cc = si_setcoreidx(oi->sih, SI_CC_IDX);
	ASSERT(cc != NULL);

	for (i = 0; i < (int)*wlen; i++)
		data[i] = hndotp_otpr(oh, cc, i);

	si_setcoreidx(oi->sih, idx);

	return 0;
}

static int
BCMNMIATTACHFN(hndotp_read_word)(void *oh, uint wn, uint16 *data)
{
	otpinfo_t *oi = (otpinfo_t *)oh;
	uint32 idx;
	chipcregs_t *cc;

	idx = si_coreidx(oi->sih);
	cc = si_setcoreidx(oi->sih, SI_CC_IDX);
	ASSERT(cc != NULL);

	*data = hndotp_otpr(oh, cc, wn);

	si_setcoreidx(oi->sih, idx);
	return 0;
}

static int
BCMNMIATTACHFN(hndotp_nvread)(void *oh, char *data, uint *len)
{
	int rc = 0;
	otpinfo_t *oi = (otpinfo_t *)oh;
	uint32 base, bound, lim = 0, st;
	int i, chunk, gchunks, tsz = 0;
	uint32 idx;
	chipcregs_t *cc;
	uint offset;
	uint16 *rawotp = NULL;

	/* save the orig core */
	idx = si_coreidx(oi->sih);
	cc = si_setcoreidx(oi->sih, SI_CC_IDX);
	ASSERT(cc != NULL);

	st = hndotp_status(oh);
	if (!(st & (OTP_HW_REGION | OTP_SW_REGION))) {
		OTP_ERR(("OTP not programmed\n"));
		rc = -1;
		goto out;
	}

	/* Read the whole otp so we can easily manipulate it */
	lim = hndotp_size(oh);
	if (lim == 0) {
		OTP_ERR(("OTP size is 0\n"));
		rc = -1;
		goto out;
	}
	/* freed in same function */
	if ((rawotp = MALLOC_NOPERSIST(si_osh(oi->sih), lim)) == NULL) {
		OTP_ERR(("Out of memory for rawotp\n"));
		rc = -2;
		goto out;
	}
	for (i = 0; i < (int)(lim / 2); i++)
		rawotp[i] = hndotp_otpr(oh, cc,  i);

	if ((st & OTP_HW_REGION) == 0) {
		OTP_ERR(("otp: hw region not written (0x%x)\n", st));

		/* This could be a programming failure in the first
		 * chunk followed by one or more good chunks
		 */
		for (i = 0; i < (int)(lim / 2); i++)
			if (rawotp[i] == OTP_MAGIC)
				break;

		if (i < (int)(lim / 2)) {
			base = i;
			bound = (i * 2) + rawotp[i + 1];
			OTP_MSG(("otp: trying chunk at 0x%x-0x%x\n", i * 2, bound));
		} else {
			OTP_MSG(("otp: unprogrammed\n"));
			rc = -3;
			goto out;
		}
	} else {
		bound = rawotp[(lim / 2) + OTP_BOUNDARY_OFF];

		/* There are two cases: 1) The whole otp is used as nvram
		 * and 2) There is a hardware header followed by nvram.
		 */
		if (rawotp[0] == OTP_MAGIC) {
			base = 0;
			if (bound != rawotp[1])
				OTP_MSG(("otp: Bound 0x%x != chunk0 len 0x%x\n", bound,
				         rawotp[1]));
		} else
			base = bound;
	}

	/* Find and copy the data */

	chunk = 0;
	gchunks = 0;
	i = base / 2;
	offset = 0;
	while ((i < (int)(lim / 2)) && (rawotp[i] == OTP_MAGIC)) {
		int dsz, rsz = rawotp[i + 1];

		if (((i * 2) + rsz) >= (int)lim) {
			OTP_MSG(("  bad chunk size, chunk %d, base 0x%x, size 0x%x\n",
			         chunk, i * 2, rsz));
			/* Bad length, try to find another chunk anyway */
			rsz = 6;
		}
		if (hndcrc16((uint8 *)&rawotp[i], rsz,
		             CRC16_INIT_VALUE) == CRC16_GOOD_VALUE) {
			/* Good crc, copy the vars */
			OTP_MSG(("  good chunk %d, base 0x%x, size 0x%x\n",
			         chunk, i * 2, rsz));
			gchunks++;
			dsz = rsz - 6;
			tsz += dsz;
			if (offset + dsz >= *len) {
				OTP_MSG(("Out of memory for otp\n"));
				goto out;
			}
			bcopy((char *)&rawotp[i + 2], &data[offset], dsz);
			offset += dsz;
			/* Remove extra null characters at the end */
			while (offset > 1 &&
			       data[offset - 1] == 0 && data[offset - 2] == 0)
				offset --;
			i += rsz / 2;
		} else {
			/* bad length or crc didn't check, try to find the next set */
			OTP_MSG(("  chunk %d @ 0x%x size 0x%x: bad crc, ",
			         chunk, i * 2, rsz));
			if (rawotp[i + (rsz / 2)] == OTP_MAGIC) {
				/* Assume length is good */
				i += rsz / 2;
			} else {
				while (++i < (int)(lim / 2))
					if (rawotp[i] == OTP_MAGIC)
						break;
			}
			if (i < (int)(lim / 2))
				OTP_MSG(("trying next base 0x%x\n", i * 2));
			else
				OTP_MSG(("no more chunks\n"));
		}
		chunk++;
	}

	OTP_MSG(("  otp size = %d, boundary = 0x%x, nv base = 0x%x\n", lim, bound, base));
	if (tsz != 0) {
		OTP_MSG(("  Found %d bytes in %d good chunks out of %d\n", tsz, gchunks, chunk));
	} else {
		OTP_MSG(("  No good chunks found out of %d\n", chunk));
	}

	*len = offset;

out:
	if (rawotp)
		MFREE(si_osh(oi->sih), rawotp, lim);
	si_setcoreidx(oi->sih, idx);

	return rc;
}

#ifdef BCMNVRAMW
#if defined(BCMDBG) || defined(WLTEST)
static	uint st_n, st_s, st_hwm, pp_hwm;
#ifdef	OTP_FORCEFAIL
static	uint forcefail_bitcount = 0;
#endif /* OTP_FORCEFAIL */
#endif /* BCMDBG || WLTEST */

static int
BCMNMIATTACHFN(hndotp_write_bit)(void *oh, chipcregs_t *cc, int bn, bool bit, int no_retry)
{
	otpinfo_t *oi = (otpinfo_t *)oh;
	uint row, col, j, k;
	uint32 pwait, init_pwait, otpc, otpp, pst, st;
	osl_t *osh;

	osh = si_osh(oi->sih);
	ASSERT((bit >> 1) == 0);

#ifdef	OTP_FORCEFAIL
	OTP_MSG(("%s: [0x%x] = 0x%x\n", __FUNCTION__, wn * 2, data));
#endif // endif

	/* This is bit-at-a-time writing, future cores may do word-at-a-time */
	if (CCREV(oi->ccrev) == 12) {
		otpc = 0x20000001;
		init_pwait = 0x00000200;
	} else if (CCREV(oi->ccrev) == 22) {
		otpc = 0x20000001;
		init_pwait = 0x00000400;
	} else {
		otpc = 0x20000001;
		init_pwait = 0x00004000;
	}

	pwait = init_pwait;
	row = bn / 65;
	col = bn % 65;
	otpp = OTPP_START_BUSY |
		((bit << OTPP_VALUE_SHIFT) & OTPP_VALUE_MASK) |
		((row << OTPP_ROW_SHIFT) & OTPP_ROW_MASK) |
		(col & OTPP_COL_MASK);
	j = 0;
	while (1) {
		j++;
		if (j > 1) {
			OTP_DBG(("row %d, col %d, val %d, otpc 0x%x, otpp 0x%x\n",
				row, col, bit, (otpc | pwait), otpp));
		}
		W_REG(osh, &cc->otpcontrol, otpc | pwait);
		W_REG(osh, &cc->otpprog, otpp);
		pst = R_REG(osh, &cc->otpprog);
		for (k = 0; ((pst & OTPP_START_BUSY) == OTPP_START_BUSY) && (k < OTPP_TRIES); k++)
			pst = R_REG(osh, &cc->otpprog);
#if defined(BCMDBG) || defined(WLTEST)
		if (k > pp_hwm)
			pp_hwm = k;
#endif /* BCMDBG || WLTEST */
		if (k >= OTPP_TRIES) {
			OTP_ERR(("BUSY stuck: pst=0x%x, count=%d\n", pst, k));
			st = OTPS_PROGFAIL;
			break;
		}
		st = R_REG(osh, &cc->otpstatus);
		if (((st & OTPS_PROGFAIL) == 0) || (pwait == OTPC_PROGWAIT) || (no_retry)) {
			break;
		} else {
			if ((CCREV(oi->ccrev) == 12) || (CCREV(oi->ccrev) == 22))
				pwait = (pwait << 3) & OTPC_PROGWAIT;
			else
				pwait = (pwait << 1) & OTPC_PROGWAIT;
			if (pwait == 0)
				pwait = OTPC_PROGWAIT;
		}
	}
#if defined(BCMDBG) || defined(WLTEST)
	st_n++;
	st_s += j;
	if (j > st_hwm)
		 st_hwm = j;
#ifdef	OTP_FORCEFAIL
	if (forcefail_bitcount++ == OTP_FORCEFAIL * 16) {
		OTP_DBG(("Forcing PROGFAIL on bit %d (FORCEFAIL = %d/0x%x)\n",
			forcefail_bitcount, OTP_FORCEFAIL, OTP_FORCEFAIL));
		st = OTPS_PROGFAIL;
	}
#endif // endif
#endif /* BCMDBG || WLTEST */
	if (st & OTPS_PROGFAIL) {
		OTP_ERR(("After %d tries: otpc = 0x%x, otpp = 0x%x/0x%x, otps = 0x%x\n",
		       j, otpc | pwait, otpp, pst, st));
		OTP_ERR(("otp prog failed. bit=%d, ppret=%d, ret=%d\n", bit, k, j));
		return 1;
	}

	return 0;
}

static int
BCMNMIATTACHFN(hndotp_write_bits)(void *oh, int bn, int bits, uint8* data)
{
	otpinfo_t *oi = (otpinfo_t *)oh;
	uint idx;
	chipcregs_t *cc;
	int i, j;
	uint8 temp;

	if (bn < 0 || bn + bits >= oi->rows * oi->cols)
		return BCME_RANGE;

	idx = si_coreidx(oi->sih);
	cc = si_setcoreidx(oi->sih, SI_CC_IDX);
	ASSERT(cc != NULL);

	for (i = 0; i < bits; ) {
		temp = *data++;
		for (j = 0; j < 8; j++, i++) {
			if (i >= bits)
				break;
			if (hndotp_write_bit(oh, cc, i + bn, (temp & 0x01), 0) != 0) {
				return -1;
			}
			temp >>= 1;
		}
	}

	si_setcoreidx(oi->sih, idx);
	return BCME_OK;
}

static int
BCMNMIATTACHFN(hndotp_write_word)(void *oh, chipcregs_t *cc, int wn, uint16 data)
{
	uint base, i;
	int err = 0;

	OTP_MSG(("%s: wn %d data %x\n", __FUNCTION__, wn, data));

	/* There is one test bit for each row */
	base = (wn * 16) + (wn / 4);

	for (i = 0; i < 16; i++) {
		err += hndotp_write_bit(oh, cc, base + i, data & 1, 0);
		data >>= 1;
		/* abort write after first error to avoid stress the charge-pump */
		if (err) {
			OTP_DBG(("%s: wn %d fail on bit %d\n", __FUNCTION__, wn, i));
			break;
		}
	}

	return err;
}

static int
BCMNMIATTACHFN(hndotp_valid_rce)(void *oh, chipcregs_t *cc, int i)
{
	otpinfo_t *oi = (otpinfo_t *)oh;
	osl_t *osh;
	uint32 hwv, fw, rce, e, sign, row, st;

	ASSERT(CCREV(oi->ccrev) >= 18);

	/* HW valid bit */
	osh = si_osh(oi->sih);
	st = R_REG(osh, &cc->otpstatus);
	hwv = (st & OTPS_RCEV_MSK) & (1 << (OTPS_RCEV_SHIFT + i));
	BCM_REFERENCE(hwv);

	if (i < 3) {
		e = i;
		fw = hndotp_size(oh)/2 + OTP_RC0_OFF + e;
	} else {
		e = i - 3;
		fw = hndotp_size(oh)/2 + OTP_RC1_OFF + e;
	}

	rce = hndotp_otpr(oh, cc, fw+1) << 16 | hndotp_otpr(oh, cc, fw);
	rce >>= ((e * OTP_RCE_BITS) + OTP_RCE_BIT0 - (e * 16));
	row = rce & OTP_RCE_ROW_MASK;
	sign = (rce >> OTP_RCE_ROW_SZ) & OTP_RCE_SIGN_MASK;

	OTP_MSG(("rce %d sign %x row %d hwv %x\n", i, sign, row, hwv));

	return (sign == OTP_SIGNATURE) ? row : -1;
}

static int
BCMNMIATTACHFN(hndotp_write_rce)(void *oh, chipcregs_t *cc, int r, uint16* data)
{
	int i, rce = -1;
	uint32	sign;

	ASSERT(((otpinfo_t *)oh)->ccrev >= 18);
	ASSERT(r >= 0 && r < hndotp_size(oh)/(2*OTP_WPR));
	ASSERT(data);

	for (rce = OTP_RCE_ROW_SZ -1; rce >= 0; rce--) {
		int e, rt, rcr, bit, err = 0;

		int rr = hndotp_valid_rce(oh, cc, rce);
		/* redundancy row in use already */
		if (rr != -1) {
			if (rr == r) {
				OTP_MSG(("%s: row %d already replaced by RCE %d",
					__FUNCTION__, r, rce));
				return 0;
			}

			continue; /* If row used, go for the next row */
		}

		/*
		 * previously used bad rce entry maybe treaed as valid rce and used again, abort on
		 * first bit error to avoid stress the charge pump
		 */

		/* Write the data to the redundant row */
		for (i = 0; i < OTP_WPR; i++) {
			err += hndotp_write_word(oh, cc, hndotp_size(oh)/2+OTP_RD_OFF+rce*4+i,
				data[i]);
			if (err) {
				OTP_MSG(("fail to write redundant row %d\n", rce));
				break;
			}
		}

		/* Now write the redundant row index */
		if (rce < 3) {
			e = rce;
			rcr = hndotp_size(oh)/2 + OTP_RC0_OFF;
		} else {
			e = rce - 3;
			rcr = hndotp_size(oh)/2 + OTP_RC1_OFF;
		}

		/* Write row numer bit-by-bit */
		bit = (rcr * 16 + rcr / 4) + e * OTP_RCE_BITS + OTP_RCE_BIT0;
		rt = r;
		for (i = 0; i < OTP_RCE_ROW_SZ; i++) {
			/* If any timeout happened, invalidate the subsequent bits with 0 */
			if (hndotp_write_bit(oh, cc, bit, (rt & (err ? 0 : 1)), err)) {
				OTP_MSG(("%s: timeout fixing row %d with RCE %d - at row"
					" number bit %x\n", __FUNCTION__, r, rce, i));
				err++;
			}
			rt >>= 1;
			bit ++;
		}

		/* Write the RCE signature bit-by-bit */
		sign = OTP_SIGNATURE;
		for (i = 0; i < OTP_RCE_SIGN_SZ; i++) {
			/* If any timeout happened, invalidate the subsequent bits with 0 */
			if (hndotp_write_bit(oh, cc, bit, (sign & (err ? 0 : 1)), err)) {
				OTP_MSG(("%s: timeout fixing row %d with RCE %d - at row"
					" number bit %x\n", __FUNCTION__, r, rce, i));
				err++;
			}
			sign >>= 1;
			bit ++;
		}

		if (err) {
			OTP_ERR(("%s: row %d not fixed by RCE %d due to %d timeouts. try next"
				" RCE\n", __FUNCTION__, r, rce, err));
			continue;
		} else {
			OTP_MSG(("%s: Fixed row %d by RCE %d\n", __FUNCTION__, r, rce));
			return BCME_OK;
		}
	}

	OTP_ERR(("All RCE's are in use. Failed fixing OTP.\n"));
	/* Fatal error, unfixable. MFGC will have to fail. Board needs to be discarded!!  */
	return BCME_NORESOURCE;
}

/** Write a row and fix it with RCE if any error detected */
static int
BCMNMIATTACHFN(hndotp_write_row)(void *oh, chipcregs_t *cc, int wn, uint16* data, bool rewrite)
{
	otpinfo_t *oi = (otpinfo_t *)oh;
	int err = 0, i;

	ASSERT(wn % OTP_WPR == 0);

	/* Write the data */
	for (i = 0; i < OTP_WPR; i++) {
		if (rewrite && (data[i] == hndotp_otpr(oh, cc, wn+i)))
			continue;

		err += hndotp_write_word(oh, cc, wn + i, data[i]);
	}

	/* Fix this row if any error */
	if (err && (CCREV(oi->ccrev) >= 18)) {
		OTP_DBG(("%s: %d write errors in row %d. Fixing...\n", __FUNCTION__, err, wn/4));
		if ((err = hndotp_write_rce(oh, cc, wn / OTP_WPR, data)))
			OTP_MSG(("%s: failed to fix row %d\n", __FUNCTION__, wn/4));
	}

	return err;
}

/** expects the caller to disable interrupts before calling this routine */
static int
BCMNMIATTACHFN(hndotp_write_region)(void *oh, int region, uint16 *data, uint wlen, uint flags)
{
	otpinfo_t *oi = (otpinfo_t *)oh;
	uint32 st;
	uint wn, base = 0, lim;
	int ret = BCME_OK;
	uint idx;
	chipcregs_t *cc;
	bool rewrite = FALSE;
	uint32	save_clk;
	BCM_REFERENCE(flags);

	ASSERT(wlen % OTP_WPR == 0);

	idx = si_coreidx(oi->sih);
	cc = si_setcoreidx(oi->sih, SI_CC_IDX);
	ASSERT(cc != NULL);

	/* Check valid region */
	if ((region != OTP_HW_REGION) &&
	    (region != OTP_SW_REGION) &&
	    (region != OTP_CID_REGION)) {
		ret = BCME_BADARG;
		goto out;
	}

	/* Region already written? */
	st = oi->hwprot | oi-> signvalid;
	if ((st & region) != 0)
		rewrite = TRUE;

	/* HW and CID have to be written before SW */
	if ((((st & (OTP_HW_REGION | OTP_CID_REGION)) == 0) &&
		(st & OTP_SW_REGION) != 0)) {
		OTP_ERR(("%s: HW/CID region should be programmed first\n", __FUNCTION__));
		ret = BCME_BADARG;
		goto out;
	}

	/* Bounds for the region */
	lim = (oi->size / 2) + OTP_SWLIM_OFF;
	if (region == OTP_HW_REGION) {
		base = 0;
	} else if (region == OTP_SW_REGION) {
		base = oi->boundary / 2;
	} else if (region == OTP_CID_REGION) {
		base = (oi->size / 2) + OTP_CID_OFF;
		lim = (oi->size / 2) + OTP_LIM_OFF;
	}

	if (wlen > (lim - base)) {
		ret = BCME_BUFTOOLONG;
		goto out;
	}
	lim = base + wlen;

#if defined(BCMDBG) || defined(WLTEST)
	st_n = st_s = st_hwm = pp_hwm = 0;
#endif /* BCMDBG || WLTEST */

	/* force ALP for progrramming stability */
	save_clk = R_REG(oi->osh, &cc->clk_ctl_st);
	OR_REG(oi->osh, &cc->clk_ctl_st, CCS_FORCEALP);
	OSL_DELAY(10);

	/* Write the data row by row */
	for (wn = base; wn < lim; wn += OTP_WPR, data += OTP_WPR) {
		if ((ret = hndotp_write_row(oh, cc, wn, data, rewrite)) != 0) {
			if (ret == BCME_NORESOURCE) {
				OTP_ERR(("%s: Abort at word %x\n", __FUNCTION__, wn));
				break;
			}
		}
	}

	/* Don't need to update signature & boundary if rewrite */
	if (rewrite)
		goto out_rclk;

	/* Done with the data, write the signature & boundary if needed */
	if (region == OTP_HW_REGION) {
		if (hndotp_write_word(oh, cc, (oi->size / 2) + OTP_BOUNDARY_OFF, lim * 2) != 0) {
			ret = BCME_NORESOURCE;
			goto out_rclk;
		}
		if (hndotp_write_word(oh, cc, (oi->size / 2) + OTP_HWSIGN_OFF,
			OTP_SIGNATURE) != 0) {
			ret = BCME_NORESOURCE;
			goto out_rclk;
		}
		oi->boundary = lim * 2;
		oi->signvalid |= OTP_HW_REGION;
	} else if (region == OTP_SW_REGION) {
		if (hndotp_write_word(oh, cc, (oi->size / 2) + OTP_SWSIGN_OFF,
			OTP_SIGNATURE) != 0) {
			ret = BCME_NORESOURCE;
			goto out_rclk;
		}
		oi->signvalid |= OTP_SW_REGION;
	} else if (region == OTP_CID_REGION) {
		if (hndotp_write_word(oh, cc, (oi->size / 2) + OTP_CIDSIGN_OFF,
			OTP_SIGNATURE) != 0) {
			ret = BCME_NORESOURCE;
			goto out_rclk;
		}
		oi->signvalid |= OTP_CID_REGION;
	}

out_rclk:
	/* Restore clock */
	W_REG(oi->osh, &cc->clk_ctl_st, save_clk);

out:
#if defined(BCMDBG) || defined(WLTEST)
	OTP_MSG(("bits written: %d, average (%d/%d): %d, max retry: %d, pp max: %d\n",
		st_n, st_s, st_n, st_n?(st_s / st_n):0, st_hwm, pp_hwm));
#endif // endif

	si_setcoreidx(oi->sih, idx);

	return ret;
}

/** For HND OTP, there's no space for appending after filling in SROM image */
static int
BCMNMIATTACHFN(hndotp_cis_append_region)(si_t *sih, int region, char *vars, int count)
{
	return otp_write_region(sih, region, (uint16*)vars, count/2, 0);
}

/**
 * Fill all unwritten RCE signature with 0 and return the number of them.
 * HNDOTP needs lock due to the randomness of unprogrammed content.
 */
static int
BCMNMIATTACHFN(hndotp_lock)(void *oh)
{
	otpinfo_t *oi = (otpinfo_t *)oh;
	int i, j, e, rcr, bit, ret = 0;
	uint32 st, idx;
	chipcregs_t *cc;

	ASSERT(CCREV(oi->ccrev) >= 18);

	idx = si_coreidx(oi->sih);
	cc = si_setcoreidx(oi->sih, SI_CC_IDX);
	ASSERT(cc != NULL);

	/* Region already written? */
	st = oi->hwprot | oi-> signvalid;
	if ((st & (OTP_HW_REGION | OTP_SW_REGION)) == 0) {
		si_setcoreidx(oi->sih, idx);
		return BCME_NOTREADY;	/* Don't lock unprogrammed OTP */
	}

	/* Find the highest valid RCE */
	for (i = 0; i < OTP_RCE_ROW_SZ -1; i++) {
		if ((hndotp_valid_rce(oh, cc, i) != -1))
			break;
	}
	i--;	/* Start invalidating from the next RCE */

	for (; i >= 0; i--) {
		if ((hndotp_valid_rce(oh, cc, i) == -1)) {

			ret++;	/* This is a unprogrammed row */

			/* Invalidate the row with 0 */
			if (i < 3) {
				e = i;
				rcr = hndotp_size(oh)/2 + OTP_RC0_OFF;
			} else {
				e = i - 3;
				rcr = hndotp_size(oh)/2 + OTP_RC1_OFF;
			}

			/* Fill row numer and signature with 0 bit-by-bit */
			bit = (rcr * 16 + rcr / 4) + e * OTP_RCE_BITS + OTP_RCE_BIT0;
			for (j = 0; j < (OTP_RCE_ROW_SZ + OTP_RCE_SIGN_SZ); j++) {
				hndotp_write_bit(oh, cc, bit, 0, 1);
				bit ++;
			}

			OTP_MSG(("locking rce %d\n", i));
		}
	}

	si_setcoreidx(oi->sih, idx);

	return ret;
}

/* expects the caller to disable interrupts before calling this routine */
static int
BCMNMIATTACHFN(hndotp_nvwrite)(void *oh, uint16 *data, uint wlen)
{
	otpinfo_t *oi = (otpinfo_t *)oh;
	uint32 st;
	uint16 crc, clen, *p, hdr[2];
	uint wn, base = 0, lim;
	int err, gerr = 0;
	uint idx;
	chipcregs_t *cc;

	/* otp already written? */
	st = oi->hwprot | oi-> signvalid;
	if ((st & (OTP_HW_REGION | OTP_SW_REGION)) == (OTP_HW_REGION | OTP_SW_REGION))
		return BCME_EPERM;

	/* save the orig core */
	idx = si_coreidx(oi->sih);
	cc = si_setcoreidx(oi->sih, SI_CC_IDX);
	ASSERT(cc != NULL);

	/* Bounds for the region */
	lim = (oi->size / 2) + OTP_SWLIM_OFF;
	base = 0;

	/* Look for possible chunks from the end down */
	wn = lim;
	while (wn > 0) {
		wn--;
		if (hndotp_otpr(oh, cc, wn) == OTP_MAGIC) {
			base = wn + (hndotp_otpr(oh, cc, wn + 1) / 2);
			break;
		}
	}
	if (base == 0) {
		OTP_MSG(("Unprogrammed otp\n"));
	} else {
		OTP_MSG(("Found some chunks, skipping to 0x%x\n", base * 2));
	}
	if ((wlen + 3) > (lim - base)) {
		err =  BCME_NORESOURCE;
		goto out;
	}

#if defined(BCMDBG) || defined(WLTEST)
	st_n = st_s = st_hwm = pp_hwm = 0;
#endif /* BCMDBG || WLTEST */

	/* Prepare the header and crc */
	hdr[0] = OTP_MAGIC;
	hdr[1] = (wlen + 3) * 2;
	crc = hndcrc16((uint8 *)hdr, sizeof(hdr), CRC16_INIT_VALUE);
	crc = hndcrc16((uint8 *)data, wlen * 2, crc);
	crc = ~crc;

	do {
		p = data;
		wn = base + 2;
		lim = base + wlen + 2;

		OTP_MSG(("writing chunk, 0x%x bytes @ 0x%x-0x%x\n", wlen * 2,
		         base * 2, (lim + 1) * 2));

		/* Write the header */
		err = hndotp_write_word(oh, cc, base, hdr[0]);

		/* Write the data */
		while (wn < lim) {
			err += hndotp_write_word(oh, cc, wn++, *p++);

			/* If there has been an error, close this chunk */
			if (err != 0) {
				OTP_MSG(("closing early @ 0x%x\n", wn * 2));
				break;
			}
		}

		/* If we wrote the whole chunk, write the crc */
		if (wn == lim) {
			OTP_MSG(("  whole chunk written, crc = 0x%x\n", crc));
			err += hndotp_write_word(oh, cc, wn++, crc);
			clen = hdr[1];
		} else {
			/* If there was an error adjust the count to point to
			 * the word after the error so we can start the next
			 * chunk there.
			 */
			clen = (wn - base) * 2;
			OTP_MSG(("  partial chunk written, chunk len = 0x%x\n", clen));
		}
		/* And now write the chunk length */
		err += hndotp_write_word(oh, cc, base + 1, clen);

		if (base == 0) {
			/* Write the signature and boundary if this is the HW region,
			 * but don't report failure if either of these 2 writes fail.
			 */
			if (hndotp_write_word(oh, cc, (oi->size / 2) + OTP_BOUNDARY_OFF,
			    wn * 2) == 0)
				gerr += hndotp_write_word(oh, cc, (oi->size / 2) + OTP_HWSIGN_OFF,
				                       OTP_SIGNATURE);
			else
				gerr++;
			oi->boundary = wn * 2;
			oi->signvalid |= OTP_HW_REGION;
		}

		if (err != 0) {
			gerr += err;
			/* Errors, do it all over again if there is space left */
			if ((wlen + 3) <= ((oi->size / 2) + OTP_SWLIM_OFF - wn)) {
				base = wn;
				lim = base + wlen + 2;
				OTP_ERR(("Programming errors, retry @ 0x%x\n", wn * 2));
			} else {
				OTP_ERR(("Programming errors, no space left ( 0x%x)\n", wn * 2));
				break;
			}
		}
	} while (err != 0);

	OTP_MSG(("bits written: %d, average (%d/%d): %d, max retry: %d, pp max: %d\n",
		st_n, st_s, st_n, (st_n ? (st_s / st_n) : 0), st_hwm, pp_hwm));

	if (gerr != 0)
		OTP_MSG(("programming %s after %d errors\n", (err == 0) ? "succedded" : "failed",
		         gerr));
out:
	/* done */
	si_setcoreidx(oi->sih, idx);

	if (err)
		return BCME_ERROR;
	else
		return 0;
}
#endif /* BCMNVRAMW */

#if defined(WLTEST)
static uint16
BCMNMIATTACHFN(hndotp_otprb16)(void *oh, chipcregs_t *cc, uint wn)
{
	uint base, i;
	uint16 val, bit;

	base = (wn * 16) + (wn / 4);
	val = 0;
	for (i = 0; i < 16; i++) {
		if ((bit = hndotp_read_bit(oh, cc, base + i)) == 0xffff)
			break;
		val = val | (bit << i);
	}
	if (i < 16)
		val = 0xaaaa;
	return val;
}

static int
BCMNMIATTACHFN(hndotp_dump)(void *oh, int arg, char *buf, uint size)
{
	otpinfo_t *oi = (otpinfo_t *)oh;
	chipcregs_t *cc;
	uint idx, i, count, lil;
	uint16 val;
	struct bcmstrbuf b;

	idx = si_coreidx(oi->sih);
	cc = si_setcoreidx(oi->sih, SI_CC_IDX);
	ASSERT(cc != NULL);

	if (arg >= 16)
		arg -= 16;

	if (arg == 2) {
		count = 66 * 4;
		lil = 3;
	} else {
		count = (oi->size / 2) + OTP_RC_LIM_OFF;
		lil = 7;
	}

	OTP_MSG(("%s: arg %d, size %d, words %d\n", __FUNCTION__, arg, size, count));
	bcm_binit(&b, buf, size);
	for (i = 0; i < count; i++) {
		if ((i & lil) == 0)
			bcm_bprintf(&b, "0x%04x:", 2 * i);

		if (arg == 0)
			val = hndotp_otpr(oh, cc, i);
		else
			val = hndotp_otprb16(oi, cc, i);
		bcm_bprintf(&b, " 0x%04x", val);
		if ((i & lil) == lil) {
			if (arg == 2) {
				bcm_bprintf(&b, " %d\n",
				            hndotp_read_bit(oh, cc, ((i / 4) * 65) + 64) & 1);
			} else {
				bcm_bprintf(&b, "\n");
			}
		}
	}
	if ((i & lil) != lil)
		bcm_bprintf(&b, "\n");

	OTP_MSG(("%s: returning %d, left %d, wn %d\n",
		__FUNCTION__, (int)(b.buf - b.origbuf), b.size, i));

	si_setcoreidx(oi->sih, idx);

	return ((int)(b.buf - b.origbuf));
}
#endif // endif

static otp_fn_t hndotp_fn = {
	(otp_size_t)hndotp_size,
	(otp_read_bit_t)hndotp_read_bit,
	(otp_dump_t)NULL,		/* Assigned in otp_init */
	(otp_status_t)hndotp_status,

	(otp_init_t)hndotp_init,
	(otp_read_region_t)hndotp_read_region,
	(otp_nvread_t)hndotp_nvread,
#ifdef BCMNVRAMW
	(otp_write_region_t)hndotp_write_region,
	(otp_cis_append_region_t)hndotp_cis_append_region,
	(otp_lock_t)hndotp_lock,
	(otp_nvwrite_t)hndotp_nvwrite,
	(otp_write_word_t)NULL,
#else /* BCMNVRAMW */
	(otp_write_region_t)NULL,
	(otp_cis_append_region_t)NULL,
	(otp_lock_t)NULL,
	(otp_nvwrite_t)NULL,
	(otp_write_word_t)NULL,
#endif /* BCMNVRAMW */
	(otp_read_word_t)hndotp_read_word,
#ifdef BCMNVRAMW
	(otp_write_bits_t)hndotp_write_bits,
	(otp_ecc_write_t)NULL,
#else
	(otp_write_bits_t)NULL,
	(otp_ecc_write_t)NULL,
#endif /* BCMNVRAMW */
	(otp_ecc_rows_dump_t)NULL,
	(otp_isunified_t)hndotp_isunified,
	(otp_avsbitslen_t)hndotp_avsbitslen,
	(otp_read_t)NULL
};

#endif /* BCMHNDOTP */

/*
 * Common Code: Compiled for IPX / HND / AUTO
 *	otp_status()
 *	otp_size()
 *	otp_read_bit()
 *	otp_init()
 * 	otp_read_region()
 * 	otp_read_word()
 * 	otp_nvread()
 * 	otp_write_region()
 * 	otp_write_word()
 * 	otp_cis_append_region()
 * 	otp_lock()
 * 	otp_nvwrite()
 * 	otp_dump()
 */

int
BCMNMIATTACHFN(otp_status)(void *oh)
{
	otpinfo_t *oi = (otpinfo_t *)oh;

	return oi->fn->status(oh);
}

uint16
otp_avsbitslen(void *oh)
{
	otpinfo_t *oi = (otpinfo_t *)oh;

	return oi->fn->avsbitslen(oh);
}

bool
BCMNMIATTACHFN(otp_isunified)(void *oh)
{
	otpinfo_t *oi = (otpinfo_t *)oh;

	return oi->fn->isunified(oh);
}

int
BCMNMIATTACHFN(otp_size)(void *oh)
{
	otpinfo_t *oi = (otpinfo_t *)oh;

	return oi->fn->size(oh);
}

uint16
BCMNMIATTACHFN(otp_read_bit)(void *oh, uint offset)
{
	otpinfo_t *oi = (otpinfo_t *)oh;
	uint idx = si_coreidx(oi->sih);
	chipcregs_t *cc = si_setcoreidx(oi->sih, SI_CC_IDX);
	uint16 readBit = (uint16)oi->fn->read_bit(oh, cc, offset);
	si_setcoreidx(oi->sih, idx);
	return readBit;
}

#if defined(BCMNVRAMW)
int
BCMNMIATTACHFN(otp_write_bits)(void *oh, uint offset, int bits, uint8* data)
{
	otpinfo_t *oi = (otpinfo_t *)oh;
	return oi->fn->write_bits(oh, offset, bits, data);
}
#endif // endif

void *
BCMSROMCISDUMPATTACHFN(otp_init)(si_t *sih)
{
	otpinfo_t *oi;
	void *ret = NULL;
	uint32 min_res_mask = 0;
	bool wasup = FALSE;

#ifdef EVENT_LOG_COMPILE
	/* First thing to do.. initialize OTP ERROR tag's attributes. */
	/* All errors are directed to log set error. */
	event_log_tag_start(EVENT_LOG_TAG_OTP_ERROR, EVENT_LOG_SET_ERROR, EVENT_LOG_TAG_FLAG_LOG);
#endif // endif

	oi = get_otpinfo();
	bzero(oi, sizeof(otpinfo_t));

	oi->ccrev = CCREV(sih->ccrev);

#ifdef BCMIPXOTP
	if (OTPTYPE_IPX(CCREV(oi->ccrev))) {
#if defined(WLTEST)
		/* Dump function is excluded from ROM */
		get_ipxotp_fn()->dump = ipxotp_dump;
		get_ipxotp_fn()->read = ipxotp_read;
#endif // endif
		oi->fn = get_ipxotp_fn();
	}
#endif /* BCMIPXOTP */

#ifdef BCMHNDOTP
	if (OTPTYPE_HND(CCREV(oi->ccrev))) {
#if defined(WLTEST)
		/* Dump function is excluded from ROM */
		hndotp_fn.dump = hndotp_dump;
#endif // endif
		oi->fn = &hndotp_fn;
	}
#endif /* BCMHNDOTP */

	if (EMBEDDED_2x2AX_CORE(sih->chip)) {
		sih->otpflag = ((CHIPC_REG(sih, chipstatus, 0, 0) & 0x8) ? 1: 0);
	}

	if (ISSIM_ENAB(sih) && (BCM43684_CHIP(sih->chip) || BCM6710_CHIP(sih->chip) ||
		BCM6715_CHIP(sih->chip))) {
		/* Skip srom read in Veloce to save time */
		OTP_ERR(("skip otp_init in 43684/6710/6715 veloce development\n"));
		oi->fn = NULL;
	}

	if (oi->fn == NULL) {
		OTP_ERR(("otp_init: unsupported OTP type\n"));
		return NULL;
	}

	oi->sih = sih;
	oi->osh = si_osh(oi->sih);

	if (!(wasup = si_is_otp_powered(sih)))
		si_otp_power(sih, TRUE, &min_res_mask);

	ret = (oi->fn->init)(sih);

	if (!wasup)
		si_otp_power(sih, FALSE, &min_res_mask);

	/* 43684 specific OTP info */
	if (BCM43684_CHIP(sih->chip)) {
		uint i;
		uint16 tmp;
		oi->sih->otpflag = 0;
		for (i = 0; i < 11; i++) {
			/* read HW config options */
			tmp = otp_read_bit(ret, OTP_BIT_500 + i);
			oi->sih->otpflag |= (tmp << i);
		}
		oi->sih->otpflag |= (1 << i); /* always set bit 11 to mark driver has this loop */
	}

	return ret;
}

int
BCMNMIATTACHFN(otp_pcie_hwhdr_sz)(si_t *sih)
{
	if (BUSCORETYPE(sih->buscoretype) != PCIE2_CORE_ID)
		return OTP_PCIE_HWHDR_SZ_CONV;

	if (PCIECOREREV(sih->buscorerev) >= 26)
		return OTP_PCIE_HWHDR_SZ_COREREV26;
	else
		return OTP_PCIE_HWHDR_SZ_COREREV19;
}

int
BCMNMIATTACHFN(otp_newcis)(void *oh)
{
	int ret = 0;
#if defined(BCMIPXOTP)
	otpinfo_t *oi = (otpinfo_t *)oh;
	int otpgu_bit_base = oi->otpgu_base * 16;

	ret = otp_read_bit(oh, otpgu_bit_base + OTPGU_NEWCISFORMAT_OFF);
	OTP_MSG(("New Cis format bit %d value: %x\n", otpgu_bit_base + OTPGU_NEWCISFORMAT_OFF,
		ret));
#endif // endif

	return ret;
}

int
BCMSROMCISDUMPATTACHFN(otp_read_region)(si_t *sih, int region, uint16 *data, uint *wlen)
{
	bool wasup = FALSE;
	void *oh;
	int err = 0;
	uint32 min_res_mask = 0;

	if (!(wasup = si_is_otp_powered(sih)))
		si_otp_power(sih, TRUE, &min_res_mask);

	if (!si_is_otp_powered(sih) || si_is_otp_disabled(sih)) {
		err = BCME_NOTREADY;
		goto out;
	}

	oh = otp_init(sih);
	if (oh == NULL) {
		OTP_ERR(("otp_init failed.\n"));
		err = BCME_ERROR;
		goto out;
	}

	err = (((otpinfo_t*)oh)->fn->read_region)(oh, region, data, wlen);

out:
	if (!wasup)
		si_otp_power(sih, FALSE, &min_res_mask);

	return err;
}

int
BCMNMIATTACHFN(otp_read_word)(si_t *sih, uint wn, uint16 *data)
{
	bool wasup = FALSE;
	void *oh;
	int err = 0;
	uint32 min_res_mask = 0;

	if (!(wasup = si_is_otp_powered(sih)))
		si_otp_power(sih, TRUE, &min_res_mask);

	if (!si_is_otp_powered(sih) || si_is_otp_disabled(sih)) {
		err = BCME_NOTREADY;
		goto out;
	}

	oh = otp_init(sih);
	if (oh == NULL) {
		OTP_ERR(("otp_init failed.\n"));
		err = BCME_ERROR;
		goto out;
	}

	if (((otpinfo_t*)oh)->fn->read_word == NULL) {
		err = BCME_UNSUPPORTED;
		goto out;
	}
	err = (((otpinfo_t*)oh)->fn->read_word)(oh, wn, data);

out:
	if (!wasup)
		si_otp_power(sih, FALSE, &min_res_mask);

	return err;
}

int
BCMNMIATTACHFN(otp_nvread)(void *oh, char *data, uint *len)
{
	otpinfo_t *oi = (otpinfo_t *)oh;

	return oi->fn->nvread(oh, data, len);
}

#ifdef BCMNVRAMW
int
BCMNMIATTACHFN(otp_write_region)(si_t *sih, int region, uint16 *data, uint wlen, uint flags)
{
	bool wasup = FALSE;
	void *oh;
	int err = 0;
	uint32 min_res_mask = 0;

	if (!(wasup = si_is_otp_powered(sih)))
		si_otp_power(sih, TRUE, &min_res_mask);

	if (!si_is_otp_powered(sih) || si_is_otp_disabled(sih)) {
		err = BCME_NOTREADY;
		goto out;
	}

	oh = otp_init(sih);
	if (oh == NULL) {
		OTP_ERR(("otp_init failed.\n"));
		err = BCME_ERROR;
		goto out;
	}

	err = (((otpinfo_t*)oh)->fn->write_region)(oh, region, data, wlen, flags);

out:
	if (!wasup)
		si_otp_power(sih, FALSE, &min_res_mask);

	return err;
}

int
BCMNMIATTACHFN(otp_write_word)(si_t *sih, uint wn, uint16 data)
{
	bool wasup = FALSE;
	void *oh;
	int err = 0;
	uint32 min_res_mask = 0;

	if (!(wasup = si_is_otp_powered(sih)))
		si_otp_power(sih, TRUE, &min_res_mask);

	if (!si_is_otp_powered(sih) || si_is_otp_disabled(sih)) {
		err = BCME_NOTREADY;
		goto out;
	}

	oh = otp_init(sih);
	if (oh == NULL) {
		OTP_ERR(("otp_init failed.\n"));
		err = BCME_ERROR;
		goto out;
	}

	if (((otpinfo_t*)oh)->fn->write_word == NULL) {
		err = BCME_UNSUPPORTED;
		goto out;
	}
	err = (((otpinfo_t*)oh)->fn->write_word)(oh, wn, data);

out:
	if (!wasup)
		si_otp_power(sih, FALSE, &min_res_mask);

	return err;
}

int
BCMNMIATTACHFN(otp_cis_append_region)(si_t *sih, int region, char *vars, int count)
{
	void *oh = otp_init(sih);

	if (oh == NULL) {
		OTP_ERR(("otp_init failed.\n"));
		return -1;
	}
	return (((otpinfo_t*)oh)->fn->cis_append_region)(sih, region, vars, count);
}

int
BCMNMIATTACHFN(otp_lock)(si_t *sih)
{
	bool wasup = FALSE;
	void *oh;
	int ret = 0;
	uint32 min_res_mask = 0;

	if (!(wasup = si_is_otp_powered(sih)))
		si_otp_power(sih, TRUE, &min_res_mask);

	if (!si_is_otp_powered(sih) || si_is_otp_disabled(sih)) {
		ret = BCME_NOTREADY;
		goto out;
	}

	oh = otp_init(sih);
	if (oh == NULL) {
		OTP_ERR(("otp_init failed.\n"));
		ret = BCME_ERROR;
		goto out;
	}

	ret = (((otpinfo_t*)oh)->fn->lock)(oh);

out:
	if (!wasup)
		si_otp_power(sih, FALSE, &min_res_mask);

	return ret;
}

int
BCMNMIATTACHFN(otp_nvwrite)(void *oh, uint16 *data, uint wlen)
{
	otpinfo_t *oi = (otpinfo_t *)oh;

	return oi->fn->nvwrite(oh, data, wlen);
}

/* ECC Related interface code to write data into OTP row - 4 bytes
* Depending on the type given in the iovar, OTP will be programming in three ways.
* Type = 8 - Prog Data with ECC (should be done only once for a row -4 bytes)
* Type = 9 - Prog Data with Read (should be done after row data is finalised -
*			creates ECC based on current contents on the row)
* Type = 10 - Prog Data without ECC
*/
int
BCMNMIATTACHFN(otp_ecc_write)(void *oh, uint offset, uint32 data, uint32 type)
{
	otpinfo_t *oi = (otpinfo_t *)oh;
	return oi->fn->ecc_write(oh, offset, data, type);
}
#endif /* BCMNVRAMW */

/* Function to read rows data and ECC status */
int
BCMNMIATTACHFN(otp_ecc_rows_dump)(void *oh, int *arg, char *buf, uint size)
{
	otpinfo_t *oi = (otpinfo_t *)oh;

	ASSERT(oi->fn->ecc_rows_dump);
	return oi->fn->ecc_rows_dump(oh, arg, buf, size);
}

/* Function to read ECC status Register */
uint32
BCMNMIATTACHFN(otp_ecc_status)(si_t *sih, uint offset)
{
	bool wasup = FALSE;
	void *oh;
	int ret = 0;
	uint32 min_res_mask = 0;
	otpinfo_t *oi = NULL;
	uint idx;
	volatile void *regs;
	otpregs_t otpregs;
	uint otpwt;
	uint32 val = 0;

	if (!(wasup = si_is_otp_powered(sih)))
		si_otp_power(sih, TRUE, &min_res_mask);

	if (!si_is_otp_powered(sih) || si_is_otp_disabled(sih)) {
		ret = BCME_NOTREADY;
		goto out;
	}

	oh = otp_init(sih);
	if (oh == NULL) {
		OTP_ERR(("otp_init failed.\n"));
		ret = BCME_ERROR;
		goto out;
	}

	oi = (otpinfo_t *)oh;

	idx = si_coreidx(sih);
	if (AOB_ENAB(sih)) {
		regs = si_setcore(sih, GCI_CORE_ID, 0);
	} else {
		regs = si_setcoreidx(sih, SI_CC_IDX);
	}
	ASSERT(regs != NULL);

	otp_initregs(sih, regs, &otpregs);

	/* Check the OTP Wrapper Type and return Error for cases other than 28nm Chips */
	otpwt = (R_REG(oi->osh, otpregs.otplayout) & OTPL_WRAP_TYPE_MASK) >> OTPL_WRAP_TYPE_SHIFT;
	BCM_REFERENCE(otpwt);
	if (OTPWRTYPE(otpwt) != OTPL_WRAP_TYPE_28NM) {
		ret = BCME_UNSUPPORTED;
		goto out;
	}
	/* Make sure offset value is within OTP size */
	if (offset >= oi->rows) {
		ret = BCME_BADARG;
		goto out;
	}
	val = R_REG(oi->osh, &oi->otpbase[offset * 2]);
	val = val | (R_REG(oi->osh, &oi->otpbase[offset * 2 + 1]) << 16);
	ret = val;

	OTP_DBG(("%s: offset = [%d] val = [0x%x]", __FUNCTION__, offset, val));

	/* Set the core back to original idx */
	si_setcoreidx(sih, idx);
out:
	if (!wasup)
		si_otp_power(sih, FALSE, &min_res_mask);

	return ret;
}

/* Function to enable ECC on OTP */
int
BCMNMIATTACHFN(otp_ecc_enable)(si_t *sih, uint enab)
{
	bool wasup = FALSE;
	void *oh;
	int ret = 0;
	uint32 min_res_mask = 0;
	otpinfo_t *oi = NULL;
	uint idx;
	volatile void *regs;
	otpregs_t otpregs;
	uint32 otpp;
	uint otpwt;

	if (!(wasup = si_is_otp_powered(sih)))
		si_otp_power(sih, TRUE, &min_res_mask);

	if (!si_is_otp_powered(sih) || si_is_otp_disabled(sih)) {
		ret = BCME_NOTREADY;
		goto out;
	}

	oh = otp_init(sih);
	if (oh == NULL) {
		OTP_ERR(("otp_init failed.\n"));
		ret = BCME_ERROR;
		goto out;
	}

	oi = (otpinfo_t *)oh;

	idx = si_coreidx(sih);
	if (AOB_ENAB(sih)) {
		regs = si_setcore(sih, GCI_CORE_ID, 0);
	} else {
		regs = si_setcoreidx(sih, SI_CC_IDX);
	}
	ASSERT(regs != NULL);
	otp_initregs(sih, regs, &otpregs);

	/* Check the OTP Wrapper Type and return Error for cases other than 28nm Chips */
	otpwt = (R_REG(oi->osh, otpregs.otplayout) & OTPL_WRAP_TYPE_MASK) >> OTPL_WRAP_TYPE_SHIFT;
	BCM_REFERENCE(otpwt);
	if (OTPWRTYPE(otpwt) != OTPL_WRAP_TYPE_28NM) {
		ret = BCME_UNSUPPORTED;
		goto out;
	}

	/* Program the data to GCI Program data register(OTPProgData 0x328) */
	if (enab)
		W_REG(oi->osh, otpregs.otpprogdata, 0x0);
	else
		W_REG(oi->osh, otpregs.otpprogdata, 0xEC);

	/* Program OTP opcode in OTPProg Register (OTPProg 0x318) */
	otpp = OTPP_START_BUSY |
		((OTPPOC_CTRL_WR_28NM<< OTPP_OC_SHIFT_28NM) & OTPP_OC_MASK_28NM) |
			((21 << OTPP_ROW_SHIFT) & OTPPROWMASK(CCREV(oi->sih->ccrev)));

	OTP_DBG(("%s:otpp = 0x%x", __FUNCTION__, otpp));

	W_REG(oi->osh, otpregs.otpprog, otpp);

	/* Set the core back to original idx */
	si_setcoreidx(sih, idx);
out:
	if (!wasup)
		si_otp_power(sih, FALSE, &min_res_mask);

	return ret;
}

/* Function to clear ECC DBLerr bit on OTP Control Register */
int
BCMNMIATTACHFN(otp_ecc_clear_dblerrbit)(si_t *sih)
{
	bool wasup = FALSE;
	void *oh;
	int ret = 0;
	uint32 min_res_mask = 0;
	otpinfo_t *oi = NULL;
	uint idx;
	volatile void *regs;
	otpregs_t otpregs;
	uint otpwt;

	if (!(wasup = si_is_otp_powered(sih)))
		si_otp_power(sih, TRUE, &min_res_mask);

	if (!si_is_otp_powered(sih) || si_is_otp_disabled(sih)) {
		ret = BCME_NOTREADY;
		goto out;
	}

	oh = otp_init(sih);
	if (oh == NULL) {
		OTP_ERR(("otp_init failed.\n"));
		ret = BCME_ERROR;
		goto out;
	}

	oi = (otpinfo_t *)oh;

	idx = si_coreidx(sih);
	if (AOB_ENAB(sih)) {
		regs = si_setcore(sih, GCI_CORE_ID, 0);
	} else {
		regs = si_setcoreidx(sih, SI_CC_IDX);
	}
	ASSERT(regs != NULL);
	otp_initregs(sih, regs, &otpregs);

	/* Check the OTP Wrapper Type and return Error for cases other than 28nm Chips */
	otpwt = (R_REG(oi->osh, otpregs.otplayout) & OTPL_WRAP_TYPE_MASK) >> OTPL_WRAP_TYPE_SHIFT;
	BCM_REFERENCE(otpwt);
	if (OTPWRTYPE(otpwt) != OTPL_WRAP_TYPE_28NM) {
		ret = BCME_UNSUPPORTED;
		goto out;
	}

	/* Clear the ECC DBL error bit in OTP control Register 0x314 */
	OR_REG(oi->osh, otpregs.otpcontrol, OTPC_DBLERRCLR);

	ret = R_REG(oi->osh, otpregs.otpcontrol);

	/* Set the core back to original idx */
	si_setcoreidx(sih, idx);

out:
	if (!wasup)
		si_otp_power(sih, FALSE, &min_res_mask);

	return ret;
}

#if defined(WLTEST)
int
BCMNMIATTACHFN(otp_dump)(void *oh, int arg, char *buf, uint size)
{
	otpinfo_t *oi = (otpinfo_t *)oh;

	if (oi->fn->dump == NULL)
		return BCME_UNSUPPORTED;
	else
		return oi->fn->dump(oh, arg, buf, size);
}

int
BCMNMIATTACHFN(otp_read)(void *oh, void *arg, char *buf, uint size)
{
	otpinfo_t *oi = (otpinfo_t *)oh;

	if (oi->fn->read == NULL)
		return BCME_UNSUPPORTED;
	else
		return oi->fn->read(oh, arg, buf, size);
}

int
BCMNMIATTACHFN(otp_dumpstats)(void *oh, int arg, char *buf, uint size)
{
	otpinfo_t *oi = (otpinfo_t *)oh;
	struct bcmstrbuf b;

	BCM_REFERENCE(arg);

	bcm_binit(&b, buf, size);

	bcm_bprintf(&b, "\nOTP, ccrev 0x%04x\n", CCREV(oi->ccrev));
#if defined(BCMIPXOTP)
	bcm_bprintf(&b, "wsize %d rows %d cols %d\n", oi->wsize, oi->rows, oi->cols);
	bcm_bprintf(&b, "hwbase %d hwlim %d swbase %d swlim %d fbase %d flim %d fusebits %d\n",
		oi->hwbase, oi->hwlim, oi->swbase, oi->swlim, oi->fbase, oi->flim, oi->fusebits);
	bcm_bprintf(&b, "otpgu_base %d status %d\n", oi->otpgu_base, oi->status);
#endif // endif
#if defined(BCMHNDOTP)
	bcm_bprintf(&b, "OLD OTP, size %d hwprot 0x%x signvalid 0x%x boundary %d\n",
		oi->size, oi->hwprot, oi->signvalid, oi->boundary);
#endif // endif
	bcm_bprintf(&b, "\n");

	return 200;	/* real buf length, pick one to cover above print */
}

#endif // endif
