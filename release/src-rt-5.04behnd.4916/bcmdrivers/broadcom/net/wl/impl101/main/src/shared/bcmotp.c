/*
 * OTP support.
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
 * $Id: bcmotp.c 829575 2023-09-01 03:46:56Z $
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
#include <pcicfg.h>
#if (!defined(_CFE_) && !defined(_CFEZ_)) || defined(CFG_WL)
#include <wlioctl.h>
#endif /* (!_CFE_ && !_CFEZ_) || CFG_WL */

/*
 * There are two different OTP controllers so far:
 *	1. new IPX OTP controller:	chipc 21, >=23
 *	2. older HND OTP controller:	chipc 12, 17, 22
 *
 * The older HND OTP controller is no longer supported and code has been removed.
 * Define BCMIPXOTP to include support for the IPX OTP controller always
 */

#if !defined(BCMIPXOTP)
#define BCMIPXOTP	1
#endif

#define OTPPROWMASK		OTPP_ROW_MASK9

#define OTP_ERR_VAL	0x0001
#define OTP_MSG_VAL	0x0002
#define OTP_DBG_VAL	0x0004
uint32 otp_msg_level = OTP_ERR_VAL;

#if defined(BCMDBG) || defined(BCMDBG_ERR)
#define OTP_ERR(args)	do {if (otp_msg_level & OTP_ERR_VAL) printf args;} while (0)
#else
#define OTP_ERR(args)
#endif /* defined(BCMDBG) || defined(BCMDBG_ERR) */

#ifdef BCMDBG
#define OTP_MSG(args)	do {if (otp_msg_level & OTP_MSG_VAL) printf args;} while (0)
#define OTP_DBG(args)	do {if (otp_msg_level & OTP_DBG_VAL) printf args;} while (0)
#else
#define OTP_MSG(args)
#define OTP_DBG(args)
#endif

#define OTPP_TRIES		10000000	/* # of tries for OTPP */
#define OTP_FUSES_PER_BIT	2
#define OTP_WRITE_RETRY		16

#ifdef BCMIPXOTP
#define MAXNUMRDES		9		/* Maximum OTP redundancy entries */
#endif

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
typedef int	(*otp_write_bits_t)(void *oh, int bn, int bits, uint8* data);
typedef bool	(*otp_isunified_t)(void *oh);
typedef uint16	(*otp_avsbitslen_t)(void *oh);
typedef int	(*otp_ecc_write_t)(void *oh, uint wn, uint32 data, uint32 ecc_type);
typedef int	(*otp_read_t)(void *oh, void *arg, char *buf, uint size);
typedef int	(*otp_ecc_rows_dump_t)(void *oh, int *arg, char *buf, uint size);

/* OTP function struct */
typedef struct otp_fn_s {
	otp_size_t		size;
	otp_read_bit_t		read_bit;
	otp_dump_t		dump;
	otp_status_t		status;

#if !defined(BCMDONGLEHOST)
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
#endif /* !BCMDONGLEHOST */
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
	struct {
		uint8 width;		/* entry width in bits */
		uint8 val_shift;	/* value bit offset in the entry */
		uint8 offsets;		/* # entries */
		uint8 stat_shift;	/* valid bit in otpstatus */
		uint16 offset[MAXNUMRDES];	/* entry offset in OTP */
	} rde_cb;			/* OTP redundancy control blocks */
#endif /* BCMIPXOTP */

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
	gciregs_t *gciregs = (gciregs_t *)coreregs;

	otpregs->otpstatus = &gciregs->otpstatus;
	otpregs->otpcontrol = &gciregs->otpcontrol;
	otpregs->otpprog = &gciregs->otpprog;
	otpregs->otplayout = &gciregs->otplayout;
	otpregs->otpcontrol1 = &gciregs->otpcontrol1;
	otpregs->otplayoutextension = &gciregs->otplayoutextension;
	otpregs->otpprogdata = &gciregs->otpprogdata;
	otpregs->otpECCstatus = &gciregs->otpECCstatus;
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
 *	ipxotp_write_rde()
 *	ipxotp_fix_word16()
 *	ipxotp_check_word16()
 *	ipxotp_max_rgnsz()
 *	ipxotp_otprb16()
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
#define OTP_SZ_FU_2576          ((ROUNDUP(2576, 16))/8)
#define OTP_SZ_FU_2224		((ROUNDUP(2224, 16))/8)
#define OTP_SZ_FU_1808          ((ROUNDUP(1808, 16))/8)
#define OTP_SZ_FU_1744          ((ROUNDUP(1744, 16))/8) /* 6717: Equal to BISR area */
#define OTP_SZ_FU_1296          ((ROUNDUP(1296, 16))/8)
#define OTP_SZ_FU_1080		((ROUNDUP(1080, 16))/8)
#define OTP_SZ_FU_972		((ROUNDUP(972, 16))/8)
#define OTP_SZ_FU_720		((ROUNDUP(720, 16))/8)
#define OTP_SZ_FU_592		((ROUNDUP(592, 16))/8)
#define OTP_SZ_FU_792		(792/8)		/* 792 bits */
#define OTP_SZ_FU_72		(72/8)		/* 72 bits */
#define OTP_SZ_CHECKSUM		(16/8)		/* 16 bits */

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

#define OTPSZ_16NM_15		0xF	/* Size 512x32: 16384 bits */
#define OTPSZ_16NM_15_ROWS	512
#define OTPSZ_16NM_15_COLS	32

#define OTPSZ_7NM_15		0xF	/* Size 512x32: 16384 bits */
#define OTPSZ_7NM_15_ROWS	512
#define OTPSZ_7NM_15_COLS	32

/* Used for Enabing OTP Programming */
#define OTP_PROGEN_SEQCOUNT	4
#define OTP_PROGEN_IDX1		15
#define OTP_PROGEN_IDX2		4
#define OTP_PROGEN_IDX3		8
#define OTP_PROGEN_IDX4		13

/* OTP PCIE HW Header Size */
#define OTP_PCIE_HWHDR_SZ_CONV		128
#define OTP_PCIE_HWHDR_SZ_COREREV19	208
#define OTP_PCIE_HWHDR_SZ_COREREV26	224	/* 43684/6710/6715/6717 PCIE HW Header Size */

#if !defined(BCMDONGLEHOST)
#if defined(BCMNVRAMW)
/* Local */
static int ipxotp_write_bit(otpinfo_t *oi, otpregs_t *otpregs, uint off);
static int ipxotp40n_read2x(void *oh, otpregs_t *otpregs, uint off);
static int ipxotp_write_rde_nopc(void *oh, otpregs_t *otpregs, uint bit, uint val);
#endif
#endif /* BCMDONGLEHOST */

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
	        ((row << OTPP_ROW_SHIFT) & OTPPROWMASK) |
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
	/* <= 28nm chips do not have otpcontrol1 register access ,
	* so skip this register access alltogether
	*/
	if ((OTPWRTYPE(otpwt) != OTPL_WRAP_TYPE_28NM) &&
	    (OTPWRTYPE(otpwt) != OTPL_WRAP_TYPE_16NM) &&
	    (OTPWRTYPE(otpwt) != OTPL_WRAP_TYPE_7NM)) {
		W_REG(oi->osh, otpregs->otpcontrol, 0);
		AND_REG(oi->osh, otpregs->otpcontrol1, (OTPC1_CLK_EN_MASK | OTPC1_CLK_DIV_MASK));
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
	regs = si_setcore(sih, GCI_CORE_ID, 0);
	ASSERT(regs != NULL);
	otp_initregs(sih, regs, &otpregs);

	val16 = _ipxotp_read_bit(oh, &otpregs, off);

	si_setcoreidx(sih, idx);
	return (val16);
}

#if !defined(BCMDONGLEHOST) || defined(WLTEST)
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
	ASSERT(sih != NULL);
	osh = si_osh(sih);

	/* If sprom is available use indirect access(as cc->sromotp maps to srom),
	 * else use random-access.
	 */
	if (si_is_sprom_available(sih))
		val = ipxotp_otprb16(oi, otpregs, wn);
	else {
		if (BUSTYPE(sih->bustype) == PCI_BUS) {
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
#endif

#if !defined(BCMDONGLEHOST)
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
	regs = si_setcore(sih, GCI_CORE_ID, 0);
	ASSERT(regs != NULL);
	otp_initregs(sih, regs, &otpregs);

	checksum = OTP_SZ_CHECKSUM;

	/* for new chips, fusebit is available from cc register */
	oi->fusebits = R_REG(oi->osh, otpregs.otplayoutextension) & OTPLAYOUTEXT_FUSE_MASK;
	oi->fusebits = ROUNDUP(oi->fusebits, 8);
	oi->fusebits >>= 3; /* bytes */

	si_setcoreidx(sih, idx);

	switch (CHIPID(sih->chip)) {
	CASE_BCM43684_CHIP:
		oi->fusebits = OTP_SZ_FU_2576;
		break;
	CASE_BCM6710_CHIP:
		oi->fusebits = OTP_SZ_FU_592;
		break;
	CASE_BCM6715_CHIP:
	CASE_BCM6726_CHIP:
	CASE_BCM6711_CHIP:
		oi->fusebits = OTP_SZ_FU_2224;
		break;
	CASE_BCM6717_CHIP:
		oi->fusebits = OTP_SZ_FU_1744;
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
		ASSERT(0);
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
		ASSERT(0);
	}

	oi->wsize = (oi->cols * oi->rows) / OTPWSIZE;
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
		ASSERT(0);
	}

	oi->wsize = (oi->cols * oi->rows) / OTPWSIZE;
	return 0;
}

/**  OTP sizes for 16nm */
static int
BCMSROMCISDUMPATTACHFN(ipxotp_otpsize_set_16nm)(otpinfo_t *oi, uint otpsz)
{
	/* Check for otp size */
	switch (otpsz) {
	case OTPSZ_16NM_15:	/* 512x32: 16384 bits */
		oi->rows = OTPSZ_16NM_15_ROWS;
		oi->cols = OTPSZ_16NM_15_COLS;
		break;
	default:
		/* Don't know the geometry */
		OTP_ERR(("%s: unknown OTP geometry\n", __FUNCTION__));
		ASSERT(0);
	}

	oi->wsize = (oi->cols * oi->rows) / OTPWSIZE;
	return 0;
}

/**  OTP sizes for 7nm */
static int
BCMSROMCISDUMPATTACHFN(ipxotp_otpsize_set_7nm)(otpinfo_t *oi, uint otpsz)
{
	/* Check for otp size */
	switch (otpsz) {
	case OTPSZ_7NM_15:	/* 512x32: 16384 bits */
		oi->rows = OTPSZ_7NM_15_ROWS;
		oi->cols = OTPSZ_7NM_15_COLS;
		break;
	default:
		/* Don't know the geometry */
		OTP_ERR(("%s: unknown OTP geometry\n", __FUNCTION__));
		ASSERT(0);
	}

	oi->wsize = (oi->cols * oi->rows) / OTPWSIZE;
	return 0;
}

static void
BCMSROMCISDUMPATTACHFN(_ipxotp_init)(otpinfo_t *oi, otpregs_t *otpregs)
{
	uint	k;
	uint32 otpp, st;
	uint   otpwt;

	/* record word offset of General Use Region for various chipcommon revs */
	/* FIX: Available in rev >= 23; Verify before applying to others */
	oi->otpgu_base = (R_REG(oi->osh, otpregs->otplayout) & OTPL_HWRGN_OFF_MASK)
		>> OTPL_HWRGN_OFF_SHIFT;
	OTP_MSG(("%s: HwOffset: bit %d\n", __FUNCTION__, oi->otpgu_base));
	ASSERT((oi->otpgu_base - (OTPGU_SROM_OFF * OTPWSIZE)) > 0);
	oi->otpgu_base >>= 4; /* words */
	oi->otpgu_base -= OTPGU_SROM_OFF;

	otpwt = (R_REG(oi->osh, otpregs->otplayout) & OTPL_WRAP_TYPE_MASK)
		>> OTPL_WRAP_TYPE_SHIFT;
	BCM_REFERENCE(otpwt);

	if ((OTPWRTYPE(otpwt) != OTPL_WRAP_TYPE_40NM) &&
	    (OTPWRTYPE(otpwt) != OTPL_WRAP_TYPE_28NM) &&
	    (OTPWRTYPE(otpwt) != OTPL_WRAP_TYPE_16NM) &&
	    (OTPWRTYPE(otpwt) != OTPL_WRAP_TYPE_7NM)) {
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

	if (BCM43684_CHIP(oi->sih->chip) ||
		BCM6710_CHIP(oi->sih->chip) ||
		BCM6715_CHIP(oi->sih->chip) ||
		BCM6717_CHIP(oi->sih->chip) ||
		BCM6726_CHIP(oi->sih->chip) ||
		BCM6711_CHIP(oi->sih->chip)) {
		uint32 p_bits;
		p_bits = (ipxotp_otpr(oi, otpregs, oi->otpgu_base + OTPGU_P_OFF) & OTPGU_P_MSK)
			>> OTPGU_P_SHIFT;
		oi->status |= (p_bits << OTPS_GUP_SHIFT);
	}
	OTP_DBG(("%s: status 0x%x\n", __FUNCTION__, oi->status));

	/* OTP unification */
	oi->buotp = FALSE; /* Initialize it to false, until its explicitely set true. */
	if ((oi->status & (OTPS_GUP_HW | OTPS_GUP_SW)) == (OTPS_GUP_HW | OTPS_GUP_SW)) {
		switch (CHIPID(oi->sih->chip)) {
			/* Add cases for supporting chips */
			CASE_BCM43684_CHIP:
			CASE_BCM6710_CHIP:
			CASE_BCM6715_CHIP:
			CASE_BCM6717_CHIP:
			CASE_BCM6726_CHIP:
			CASE_BCM6711_CHIP:
				oi->buotp = TRUE;
				break;
			default:
				OTP_ERR(("chip=0x%x does not support Unified OTP.\n",
					CHIPID(oi->sih->chip)));
				break;
		}
	}

	/* if AVS is part of s/w region, update how many bits are used for AVS */
	oi->avsbitslen = 0;

	/*
	 * h/w region base and fuse region limit are fixed to the top and
	 * the bottom of the general use region. Everything else can be flexible.
	 */
	oi->hwbase = oi->otpgu_base + OTPGU_SROM_OFF;
	oi->hwlim = oi->wsize;
	oi->flim = oi->wsize;

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

	/* Update swlim and fbase */
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
#ifdef BCMNVRAMW
	uint16 offset[] = {266, 284, 302, 330, 348, 366, 394, 412, 430};
#endif /* BCMNVRAMW */

	OTP_MSG(("%s: Use IPX OTP controller\n", __FUNCTION__));

	/* Make sure OTP is not disabled */
	if (si_is_otp_disabled(sih)) {
		OTP_MSG(("%s: OTP is disabled\n", __FUNCTION__));
#if !defined(WLTEST)
		return NULL;
#endif
	}

	/* Make sure OTP is powered up */
	if (!si_is_otp_powered(sih)) {
		OTP_ERR(("%s: OTP is powered down\n", __FUNCTION__));
		return NULL;
	}

	/* Retrieve OTP region info */
	idx = si_coreidx(sih);
	regs = si_setcore(sih, GCI_CORE_ID, 0);
	ASSERT(regs != NULL);
	otp_initregs(sih, regs, &otpregs);

	oi = get_otpinfo();

	if (BCM6710_CHIP(oi->sih->chip) && CHIPREV(sih->chiprev) == 0) {
		otpsz = OTPSZ_28NM_11;
	} else {
		otpsz = (R_REG(oi->osh, otpregs.otplayout) & OTPL_ROW_SIZE_MASK)
			>> OTPL_ROW_SIZE_SHIFT;
	}
	OTP_MSG(("%s: cap_otpsz 0x%x\n", __FUNCTION__, otpsz));

	if (otpsz == 0) {
		OTP_ERR(("%s: No OTP\n", __FUNCTION__));
		oi = NULL;
		goto exit;
	}

	otpwt = (R_REG(oi->osh, otpregs.otplayout) & OTPL_WRAP_TYPE_MASK) >> OTPL_WRAP_TYPE_SHIFT;
	BCM_REFERENCE(otpwt);
	OTP_MSG(("%s: wrap type %d\n", __FUNCTION__, OTPWRTYPE(otpwt)));

	if (OTPWRTYPE(otpwt) == OTPL_WRAP_TYPE_40NM) {
		ipxotp_otpsize_set_40nm(oi, otpsz);
	} else if (OTPWRTYPE(otpwt) == OTPL_WRAP_TYPE_65NM) {
		ipxotp_otpsize_set_65nm(oi, otpsz);
	} else if (OTPWRTYPE(otpwt) == OTPL_WRAP_TYPE_28NM) {
		ipxotp_otpsize_set_28nm(oi, otpsz);
	} else if (OTPWRTYPE(otpwt) == OTPL_WRAP_TYPE_16NM) {
		ipxotp_otpsize_set_16nm(oi, otpsz);
	} else if (OTPWRTYPE(otpwt) == OTPL_WRAP_TYPE_7NM) {
		ipxotp_otpsize_set_7nm(oi, otpsz);
	} else {
		OTP_ERR(("%s: Unknown wrap type: %d\n", __FUNCTION__, OTPWRTYPE(otpwt)));
		oi = NULL;
		goto exit;
	}

	OTP_MSG(("%s: rows %u cols %u wsize %u\n", __FUNCTION__, oi->rows, oi->cols, oi->wsize));

	/* check if GCI core is exist */
	if (si_findcoreidx(sih, GCI_CORE_ID, 0) != BADIDX) {
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
		OTP_MSG(("%s: mapping otpbase at 0x%08x to 0x%p\n", __FUNCTION__,
			otpaddr, oi->otpbase));
	} else {
		chipcregs_t *cc = (chipcregs_t *)regs;
		oi->otpbase = (volatile uint16 *)cc->sromotp;
	}

#ifdef BCMNVRAMW
	bcopy(offset, &oi->rde_cb.offset, sizeof(offset));
	oi->rde_cb.offsets = ARRAYSIZE(offset);
	oi->rde_cb.width = 18;
	oi->rde_cb.val_shift = 14;
	oi->rde_cb.stat_shift = 16;
	ASSERT(oi->rde_cb.offsets <= MAXNUMRDES);
#endif /* BCMNVRAMW */

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
	regs = si_setcore(sih, GCI_CORE_ID, 0);
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
	regs = si_setcore(sih, GCI_CORE_ID, 0);
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
				((row << OTPP_ROW_SHIFT) & OTPPROWMASK) |
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
	else if ((OTPWRTYPE(otpwt) == OTPL_WRAP_TYPE_28NM) ||
		(OTPWRTYPE(otpwt) == OTPL_WRAP_TYPE_16NM) ||
		(OTPWRTYPE(otpwt) == OTPL_WRAP_TYPE_7NM)) {
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
				((row << OTPP_ROW_SHIFT) & OTPPROWMASK);
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
		(((OTPWRTYPE(otpwt) == OTPL_WRAP_TYPE_28NM) ||
		(OTPWRTYPE(otpwt) == OTPL_WRAP_TYPE_16NM) ||
		(OTPWRTYPE(otpwt) == OTPL_WRAP_TYPE_7NM)) ?
		OTPC_PROGEN_28NM : OTPC_PROGEN));
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
			((row << OTPP_ROW_SHIFT) & OTPPROWMASK) |
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
		(((OTPWRTYPE(otpwt) == OTPL_WRAP_TYPE_28NM) ||
		(OTPWRTYPE(otpwt) == OTPL_WRAP_TYPE_16NM) ||
		(OTPWRTYPE(otpwt) == OTPL_WRAP_TYPE_7NM)) ?
		~OTPC_PROGEN_28NM : ~OTPC_PROGEN));
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

	if ((OTPWRTYPE(otpwt) != OTPL_WRAP_TYPE_28NM) &&
	    (OTPWRTYPE(otpwt) != OTPL_WRAP_TYPE_16NM) &&
	    (OTPWRTYPE(otpwt) != OTPL_WRAP_TYPE_7NM)) {
		otpp = OTPP_START_BUSY |
		        ((1 << OTPP_VALUE_SHIFT) & OTPP_VALUE_MASK) |
			((((OTPWRTYPE(otpwt) == OTPL_WRAP_TYPE_40NM)? OTPPOC_BIT_PROG_40NM :
			OTPPOC_BIT_PROG) << OTPP_OC_SHIFT) & OTPP_OC_MASK) |
		        ((row << OTPP_ROW_SHIFT) & OTPPROWMASK) |
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
		        ((row << OTPP_ROW_SHIFT) & OTPPROWMASK);
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

#if !defined(BCMDONGLEHOST)
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

	otpcontrol1 = R_REG(oi->osh, otpregs->otpcontrol1);
	otpcontrol1 &= (OTPC1_CLK_EN_MASK | OTPC1_CLK_DIV_MASK);

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

	otpcontrol1 = R_REG(oi->osh, otpregs->otpcontrol1);
	otpcontrol1 &= (OTPC1_CLK_EN_MASK | OTPC1_CLK_DIV_MASK);

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

	otpcontrol1 = R_REG(oi->osh, otpregs->otpcontrol1);
	otpcontrol1 &= (OTPC1_CLK_EN_MASK | OTPC1_CLK_DIV_MASK);

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

	otpcontrol1 = R_REG(oi->osh, otpregs->otpcontrol1);
	otpcontrol1 &= (OTPC1_CLK_EN_MASK | OTPC1_CLK_DIV_MASK);

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
	regs = si_setcore(sih, GCI_CORE_ID, 0);
	ASSERT(regs != NULL);
	otp_initregs(sih, regs, &otpregs);

	otpwt = (R_REG(oi->osh, otpregs.otplayout) & OTPL_WRAP_TYPE_MASK)
		>> OTPL_WRAP_TYPE_SHIFT;
	BCM_REFERENCE(otpwt);
	if (OTPWRTYPE(otpwt) != OTPL_WRAP_TYPE_40NM)
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
	regs = si_setcore(sih, GCI_CORE_ID, 0);
	ASSERT(regs != NULL);
	otp_initregs(sih, regs, &otpregs);

	otpwt = (R_REG(oi->osh, otpregs.otplayout) & OTPL_WRAP_TYPE_MASK)
		>> OTPL_WRAP_TYPE_SHIFT;
	BCM_REFERENCE(otpwt);
	if (OTPWRTYPE(otpwt) != OTPL_WRAP_TYPE_40NM)
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
	regs = si_setcore(sih, GCI_CORE_ID, 0);
	ASSERT(regs != NULL);
	otp_initregs(sih, regs, &otpregs);

	W_REG(oi->osh, otpregs.otpcontrol, 0);

	otpcontrol1 = R_REG(oi->osh, otpregs.otpcontrol1);
	otpcontrol1 &= (OTPC1_CLK_EN_MASK | OTPC1_CLK_DIV_MASK);

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
	regs = si_setcore(sih, GCI_CORE_ID, 0);
	ASSERT(regs != NULL);
	otp_initregs(sih, regs, &otpregs);

	ipxotp_writable(oi, &otpregs);
	for (i = 0; i < bits; i++) {
		err = ipxotp_write_bit(oi, &otpregs, off);
		if (err != 0) {
			OTP_ERR(("%s: write bit failed: %d\n", __FUNCTION__, off));

			err = ipxotp_write_rde_nopc(oi, &otpregs, off, 1);
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
	} else if ((OTPWRTYPE(otpwt) == OTPL_WRAP_TYPE_28NM) ||
		(OTPWRTYPE(otpwt) == OTPL_WRAP_TYPE_16NM) ||
		(OTPWRTYPE(otpwt) == OTPL_WRAP_TYPE_7NM)) {
		/* Handle the bit Write specially for 28nm Chips as they support only byte write
		* Each bit will be handled like byte write masking off all the remaining bits to
		* ensure no fuse damage
		*/
		status = ipxotp_write_bit_common(oi, otpregs, off);
	} else {
		AND_REG(oi->osh, otpregs->otpcontrol, OTPC_PROGEN);
		AND_REG(oi->osh, otpregs->otpcontrol1, (OTPC1_CLK_EN_MASK | OTPC1_CLK_DIV_MASK));
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
	regs = si_setcore(sih, GCI_CORE_ID, 0);
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
							i + bn, 1);
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
#endif /* !BCMDONGLEHOST */

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

	if ((OTPWRTYPE(otpwt) == OTPL_WRAP_TYPE_28NM) ||
	    (OTPWRTYPE(otpwt) == OTPL_WRAP_TYPE_16NM) ||
	    (OTPWRTYPE(otpwt) == OTPL_WRAP_TYPE_7NM)) {
		otpp = OTPP_START_BUSY |
			((OTPPOC_PROGRAM_LOCK_28NM << OTPP_OC_SHIFT_28NM) &
			OTPP_OC_MASK_28NM) |
		    ((row << OTPP_ROW_SHIFT) & OTPPROWMASK);
		OTP_DBG(("%s: off = %d, row = %d, otpp = 0x%x\n",
		         __FUNCTION__, off, row, otpp));
	} else {
		otpp = OTPP_START_BUSY |
			((((OTPWRTYPE(otpwt) == OTPL_WRAP_TYPE_40NM)? OTPPOC_ROW_LOCK_40NM :
			OTPPOC_ROW_LOCK) << OTPP_OC_SHIFT) & OTPP_OC_MASK) |
		    ((row << OTPP_ROW_SHIFT) & OTPPROWMASK) |
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

				rc = ipxotp_write_rde_nopc(oi, otpregs, base + i, 1);
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

/** Assumes already writable and bypasses power-cycling */
static int
BCMNMIATTACHFN(ipxotp_write_rde_nopc)(void *oh, otpregs_t *otpregs, uint bit, uint val)
{
	otpinfo_t *oi = (otpinfo_t *)oh;
	uint i, temp;
	int err = BCME_OK;
	int rde;

#ifdef BCMDBG
	if ((bit >= (uint)(oi->rows * oi->cols)) || (val > 1)) {
		return BCME_RANGE;
	}
#endif

	for (rde = 0; rde < oi->rde_cb.offsets - 1; rde++) {
		if ((oi->status & (1 << (oi->rde_cb.stat_shift + rde))) == 0)
			break;
	}
	OTP_ERR(("%s: Auto rde index %d\n", __FUNCTION__, rde));

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

static int
BCMNMIATTACHFN(ipxotp_write_rde)(void *oh, uint bit, uint val)
{
	otpinfo_t *oi = (otpinfo_t *)oh;
	si_t *sih = oi->sih;
	uint idx;
	volatile void *regs;
	otpregs_t otpregs;
	int err;
	uint32 min_res_mask = 0;

	idx = si_coreidx(oi->sih);
	regs = si_setcore(sih, GCI_CORE_ID, 0);
	ASSERT(regs != NULL);
	otp_initregs(sih, regs, &otpregs);

	/* Enable Write */
	ipxotp_writable(oi, &otpregs);

	err = ipxotp_write_rde_nopc(oh, &otpregs, bit, val);

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
			if ((rc = ipxotp_write_rde(oi, bit, val & 1)))
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
	regs = si_setcore(sih, GCI_CORE_ID, 0);
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
					base * 16);
				ipxotp_write_bit(oi, &otpregs, otpgu_bit_base + OTPGU_SWP_OFF);
			} else
				ipxotp_otpwb16(oi, &otpregs, oi->otpgu_base + OTPGU_HSB_OFF,
				(base + i) * 16);
			ipxotp_write_bit(oi, &otpregs, otpgu_bit_base + OTPGU_HWP_OFF);
#if (!defined(_CFE_) && !defined(_CFEZ_)) || defined(CFG_WL)
			/* Set new CIS format bit when the ciswrite command ask us
			 * to do so, or for listed chips
			 */
			if (flags & CISH_FLAG_PCIECIS) {
				ipxotp_write_bit(oi, &otpregs, otpgu_bit_base +
					OTPGU_NEWCISFORMAT_OFF);
			}
#endif /* (!_CFE_ && !_CFEZ_) || CFG_WL */
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
				boundary_val = (base * 16);
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
			if ((rc = ipxotp_write_rde(oi, otpgu_bit_base + programmed_off, 1))) {
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
	regs = si_setcore(sih, GCI_CORE_ID, 0);
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

				if (i >= (int)((sz*2) - 2))
					i = 0;
			} else {
				/* move pass the hardware header */
				if (BUSCORETYPE(sih->buscoretype) == PCIE2_CORE_ID) {
					/* PCIE GEN2 */
					i += otp_pcie_hwhdr_sz(sih);
				} else {
					uint32 otp_layout;

					otp_layout = si_corereg(sih, GCI_CORE_IDX(sih),
						GCI_OFFSETOF(sih, otplayout), 0, 0);

					if (otp_layout & OTP_CISFORMAT_NEW) {
						i += 12; /* new sdio header format, 6 half words */
					} else {
						i += 8; /* old sdio header format */
					}
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
	regs = si_setcore(sih, GCI_CORE_ID, 0);
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
			((row << OTPP_ROW_SHIFT) & OTPPROWMASK);

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
	regs = si_setcore(sih, GCI_CORE_ID, 0);
	ASSERT(regs != NULL);

	otp_initregs(sih, regs, &otpregs);
	/* Check the OTP Wrapper Type and return Error for cases other than 28nm Chips */
	otpwt = (R_REG(oi->osh, otpregs.otplayout) & OTPL_WRAP_TYPE_MASK) >> OTPL_WRAP_TYPE_SHIFT;
	BCM_REFERENCE(otpwt);
	if ((OTPWRTYPE(otpwt) != OTPL_WRAP_TYPE_28NM) &&
	    (OTPWRTYPE(otpwt) != OTPL_WRAP_TYPE_16NM) &&
	    (OTPWRTYPE(otpwt) != OTPL_WRAP_TYPE_7NM)) {
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
	regs = si_setcore(sih, GCI_CORE_ID, 0);
	otp_initregs(sih, regs, &otpregs);

	/* Check the OTP Wrapper Type and return Error for cases other than 28nm Chips */
	otpwt = (R_REG(oi->osh, otpregs.otplayout) & OTPL_WRAP_TYPE_MASK) >> OTPL_WRAP_TYPE_SHIFT;
	BCM_REFERENCE(otpwt);
	if ((OTPWRTYPE(otpwt) != OTPL_WRAP_TYPE_28NM) &&
	    (OTPWRTYPE(otpwt) != OTPL_WRAP_TYPE_16NM) &&
	    (OTPWRTYPE(otpwt) != OTPL_WRAP_TYPE_7NM)) {
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
#endif /* !defined(BCMDONGLEHOST) */

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
	regs = si_setcore(sih, GCI_CORE_ID, 0);
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
	regs = si_setcore(sih, GCI_CORE_ID, 0);
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
#endif

static otp_fn_t BCMCISDUMPATTACHDATA(ipxotp_fn) = {
	(otp_size_t)ipxotp_size,
	(otp_read_bit_t)ipxotp_read_bit,
	(otp_dump_t)NULL,		/* Assigned in otp_init */
	(otp_status_t)ipxotp_status,

#if !defined(BCMDONGLEHOST)
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
#endif /* !defined(BCMDONGLEHOST) */
#if !defined(BCMDONGLEHOST)
#ifdef BCMNVRAMW
	(otp_write_bits_t)ipxotp_write_bits,
	(otp_ecc_write_t)ipxotp_ecc_write,	/* ECC related code */
	(otp_ecc_rows_dump_t)ipxotp_ecc_rows_dump,
#else
	(otp_write_bits_t)NULL,
	(otp_ecc_write_t)NULL,
	(otp_ecc_rows_dump_t)NULL,
#endif /* BCMNVRAMW */
#endif /* !BCMDONGLEHOST */
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
 * Common Code: Compiled for IPX / AUTO
 *	otp_status()
 *	otp_size()
 *	otp_read_bit()
 *	otp_init()
 *	otp_read_region()
 *	otp_read_word()
 *	otp_nvread()
 *	otp_write_region()
 *	otp_write_word()
 *	otp_cis_append_region()
 *	otp_lock()
 *	otp_nvwrite()
 *	otp_dump()
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

#if !defined(BCMDONGLEHOST) && defined(BCMNVRAMW)
int
BCMNMIATTACHFN(otp_write_bits)(void *oh, uint offset, int bits, uint8* data)
{
	otpinfo_t *oi = (otpinfo_t *)oh;
	return oi->fn->write_bits(oh, offset, bits, data);
}
#endif /* BCMNVRAMW && !BCMDONGLEHOST */

void *
BCMSROMCISDUMPATTACHFN(otp_init)(si_t *sih)
{
	otpinfo_t *oi;
	void *ret = NULL;
#if !defined(BCMDONGLEHOST)
	uint32 min_res_mask = 0;
	bool wasup = FALSE;
	uint i;
	uint16 tmp;
#endif /* !BCMDONGLEHOST */

	sih->otpflag = 0;

	oi = get_otpinfo();
	bzero(oi, sizeof(otpinfo_t));

	oi->ccrev = CCREV(sih->ccrev);

#ifdef BCMIPXOTP
#if defined(WLTEST)
	/* Dump function is excluded from ROM */
	get_ipxotp_fn()->dump = ipxotp_dump;
	get_ipxotp_fn()->read = ipxotp_read;
#endif
	oi->fn = get_ipxotp_fn();
#endif /* BCMIPXOTP */

	if (EMBEDDED_2x2AX_CORE(sih->chip)) {
		sih->otpflag = ((CHIPC_REG_RD(sih, chipstatus) & 0x8) ? 1: 0);
	}

#if defined BCM_ROUTER && !defined(BCMDONGLEHOST) && !defined(DONGLEBUILD) && \
	!defined(CMWIFI)
	if (EMBEDDED_2x2AX_160MHZ_CORE(sih->chip) && CCREV(sih->ccrev) >= 131) {
		if (otp_fout_128bit_read(sih, 84, 84) == 1) {
			sih->otpflag |= WIFI0_HWCFG_OPT0_IND;
		}
	}
	if (EMBEDDED_WLAN_CORE(sih->chip)) {
		bcm_soc_info_t bcm_soc_info;
		int ret = bcm_get_soc_info(&bcm_soc_info);
		if (ret == BCME_OK) {
			/* combine retrieved info in one 15bits value */
			uint32 soc_info = ((bcm_soc_info.process & 0x1F) |
				((bcm_soc_info.substrate & 0x1F) << 5) |
				((bcm_soc_info.foundry & 0x1F) << 10));
			/* determine whether we lost bits with above masking */
			uint32 overflow = ((bcm_soc_info.process & ~0x1F) |
				(bcm_soc_info.substrate & ~0x1F) |
				(bcm_soc_info.foundry & ~0x1F));
			/* push soc_info into otpflag and mark it valid with bit#28  */
			sih->otpflag |= ((soc_info << 12) | (1 << 28));
			/* indicate potential overflow in bit#27 */
			if (overflow != 0) {
				sih->otpflag |= (1 << 27);
			}
			OTP_DBG(("%s SOC info: proc:0x%x, sub: 0x%x, fab: 0x%x\n",
				__FUNCTION__, bcm_soc_info.process, bcm_soc_info.substrate,
				bcm_soc_info.foundry));
		} else {
			OTP_ERR(("###### %s SOC info not available (%d) ######\n",
				__FUNCTION__, ret));
		}
		/* mark bit#29 regardless to indicate driver support for this soc info */
		sih->otpflag |= (1 << 29);
	}
#endif /* BCM_ROUTER && !BCMDONGLEHOST && !DONGLEBUILD && !CMWIFI */
	if (ISSIM_ENAB(sih)) {
		/* Skip srom read in Veloce to save time */
		OTP_ERR(("skip otp_init in veloce development\n"));
		oi->fn = NULL;
	}

	if (oi->fn == NULL) {
		OTP_ERR(("otp_init: unsupported OTP type\n"));
		return NULL;
	}

	oi->sih = sih;
	oi->osh = si_osh(oi->sih);

#if !defined(BCMDONGLEHOST)
	if (!(wasup = si_is_otp_powered(sih)))
		si_otp_power(sih, TRUE, &min_res_mask);

	ret = (oi->fn->init)(sih);

	if (!wasup)
		si_otp_power(sih, FALSE, &min_res_mask);

	/* Specific OTP info */
	if (BCM43684_CHIP(sih->chip)) {
		for (i = 0; i < 11; i++) {
			/* read HW config options */
			tmp = otp_read_bit(ret, OTP43684_HW_CONFIG_OPTIONS_0 + i);
			sih->otpflag |= (tmp << i);
		}
		sih->otpflag |= (1 << i); /* always set bit 11 to mark driver has this loop */

	} else if (BCM6715_CHIP(sih->chip)) {
		for (i = 0; i < OTP_HW_CONFIG_OPTIONS_BITS; i++) {
			/* read HW config options and map to otpflag bit[6:0] */
			tmp = otp_read_bit(ret, OTP6715_HW_CONFIG_OPTIONS_0 + i);
			sih->otpflag |= (tmp << i);
		}
		for (i = 0; i < OTP_FOUNDRY_FAB_BITS; i++) {
			/* read foundry FABs and map to otpflag bit[11:7] */
			tmp = otp_read_bit(ret, OTP_FOUNDRY_FAB_0 + i);
			sih->otpflag |= (tmp << (OTP_HW_CONFIG_OPTIONS_BITS + i));
		}
	} else if (BCM6726_CHIP(sih->chip)) {
		for (i = 0; i < OTP_HW_CONFIG_OPTIONS_BITS; i++) {
			/* read HW config options and map to otpflag bit[6:0] */
			tmp = otp_read_bit(ret, OTP6726_HW_CONFIG_OPTIONS_0 + i);
			sih->otpflag |= (tmp << i);
		}
	} else if (BCM6711_CHIP(sih->chip)) {
		for (i = 0; i < OTP_PACKAGE_OPTIONS_BITS; i++) {
			/* read package options and map to otpflag bit[10:0] */
			tmp = otp_read_bit(ret, OTP6711_PACKAGE_OPTIONS_0 + i);
			sih->otpflag |= (tmp << i);
		}
	}
#endif /* !BCMDONGLEHOST */

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

#if !defined(BCMDONGLEHOST)
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
#endif

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
	regs = si_setcore(sih, GCI_CORE_ID, 0);
	ASSERT(regs != NULL);

	otp_initregs(sih, regs, &otpregs);

	/* Check the OTP Wrapper Type and return Error for cases other than 28nm Chips */
	otpwt = (R_REG(oi->osh, otpregs.otplayout) & OTPL_WRAP_TYPE_MASK) >> OTPL_WRAP_TYPE_SHIFT;
	BCM_REFERENCE(otpwt);
	if ((OTPWRTYPE(otpwt) != OTPL_WRAP_TYPE_28NM) &&
	    (OTPWRTYPE(otpwt) != OTPL_WRAP_TYPE_16NM) &&
	    (OTPWRTYPE(otpwt) != OTPL_WRAP_TYPE_7NM)) {
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
	regs = si_setcore(sih, GCI_CORE_ID, 0);
	ASSERT(regs != NULL);
	otp_initregs(sih, regs, &otpregs);

	/* Check the OTP Wrapper Type and return Error for cases other than 28nm Chips */
	otpwt = (R_REG(oi->osh, otpregs.otplayout) & OTPL_WRAP_TYPE_MASK) >> OTPL_WRAP_TYPE_SHIFT;
	BCM_REFERENCE(otpwt);
	if ((OTPWRTYPE(otpwt) != OTPL_WRAP_TYPE_28NM) &&
	    (OTPWRTYPE(otpwt) != OTPL_WRAP_TYPE_16NM) &&
	    (OTPWRTYPE(otpwt) != OTPL_WRAP_TYPE_7NM)) {
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
			((21 << OTPP_ROW_SHIFT) & OTPPROWMASK);

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
	regs = si_setcore(sih, GCI_CORE_ID, 0);
	ASSERT(regs != NULL);
	otp_initregs(sih, regs, &otpregs);

	/* Check the OTP Wrapper Type and return Error for cases other than 28nm Chips */
	otpwt = (R_REG(oi->osh, otpregs.otplayout) & OTPL_WRAP_TYPE_MASK) >> OTPL_WRAP_TYPE_SHIFT;
	BCM_REFERENCE(otpwt);
	if ((OTPWRTYPE(otpwt) != OTPL_WRAP_TYPE_28NM) &&
	    (OTPWRTYPE(otpwt) != OTPL_WRAP_TYPE_16NM) &&
	    (OTPWRTYPE(otpwt) != OTPL_WRAP_TYPE_7NM)) {
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
#endif
	bcm_bprintf(&b, "\n");

	return 200;	/* real buf length, pick one to cover above print */
}

#endif

#if defined BCM_ROUTER
/**
 * Chipcommon rev >=131 on router SoCs offer a mechanism to read out the 128 bits wide 'fout' shift
 * register within the WLAN core. This offers WLAN software the opportunity to interpret more OTP
 * bits than was previously possible. When the SoC's OTP has been programmed, this shift register
 * will be populated at PoR by hardware with WIFI data that was read from the SoC OTP.
 *
 * @param[in] lsb, msb : data to read: fout128[msb:lsb]
 *
 * @return               data that was read.
 */
uint32
otp_fout_128bit_read(si_t *sih, uint msb, uint lsb)
{
	uint i_word32;		/**< index of 32 bits word within the 128 bits shift register */
	uint32 ret;
	const uint nbits = msb - lsb + 1;

	ASSERT(CCREV(sih->ccrev) >= 131);
	ASSERT(msb <= 127);
	ASSERT(msb >= lsb);
	ASSERT(nbits <= sizeof(uint32) * NBBY - 1); /* max 31 bits wide return value */
	i_word32 = msb / (sizeof(uint32) * NBBY);
	ASSERT(i_word32 == lsb / (sizeof(uint32) * NBBY)); /* bitfield should not straddle */

	if (CHIPC_REG_RD(sih, wotp_rx_cmd) & WOTP_RX_CMD_FOUT_ERR_MASK) {
		return 0; /* likely SoC OTP has not been programmed */
	}

	CHIPC_REG_WR(sih, wotp_rx_cmd, WOTP_RX_CMD_DAT_SEL_MASK, i_word32);
	ASSERT(CHIPC_REG_RD(sih, wotp_rx_cmd) & WOTP_RX_CMD_DAT_AV_MASK);
	ret = CHIPC_REG_RD(sih, wotp_rx_data);
	ret >>= (lsb % sizeof(uint32));
	ret &= (1 << nbits) - 1;

	return ret;
} /* otp_fout_128bit_read */
#endif /* BCM_ROUTER */

#endif /* !defined(BCMDONGLEHOST) */
