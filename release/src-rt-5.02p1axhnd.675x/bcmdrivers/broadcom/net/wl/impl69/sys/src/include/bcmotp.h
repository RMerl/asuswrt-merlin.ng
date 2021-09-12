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
 * $Id: bcmotp.h 765714 2018-07-13 08:30:53Z $
 */

#ifndef	_bcmotp_h_
#define	_bcmotp_h_

/* OTP regions */
#define OTP_HW_RGN	1
#define OTP_SW_RGN	2
#define OTP_CI_RGN	4
#define OTP_FUSE_RGN	8
#define OTP_NW_RGN	9
#define OTP_ALL_RGN	0xf	/* From h/w region to end of OTP including checksum */

/* OTP Size */
#define OTP_SZ_MAX		(16384/8)	/* maximum bytes in one CIS */

/* Fixed size subregions sizes in words */
#define OTPGU_CI_SZ		2
#define OTP_NW_SZ		0x14
#define OTP_NW_BASE		0x9c

/* OTP bit offset */
#define OTP_BIT_500	500

/* OTP usage */
#define OTP4325_FM_DISABLED_OFFSET	188

#ifdef BCMNVRAMW
/* Global RDE index for chips not having an OTP PMU resource. */
#define OTP_GLOBAL_RDE_IDX 0xFF
#endif // endif

#ifdef BCMOTPWRTYPE
#define OTPWRTYPE(owt)  (BCMOTPWRTYPE)
#else
#define OTPWRTYPE(owt)  (owt)
#endif // endif

/* Exported functions */
extern int	otp_status(void *oh);
extern int	otp_size(void *oh);
extern bool	otp_isunified(void *oh);
extern uint16	otp_avsbitslen(void *oh);
extern uint16	otp_read_bit(void *oh, uint offset);
extern void*	otp_init(si_t *sih);
#if !defined(BCMDONGLEHOST)
extern int	otp_newcis(void *oh);
extern int	otp_read_region(si_t *sih, int region, uint16 *data, uint *wlen);
extern int	otp_read_word(si_t *sih, uint wn, uint16 *data);
extern int	otp_nvread(void *oh, char *data, uint *len);
#ifdef BCMNVRAMW
extern int	otp_write_region(si_t *sih, int region, uint16 *data, uint wlen, uint flags);
extern int	otp_write_word(si_t *sih, uint wn, uint16 data);
extern int	otp_cis_append_region(si_t *sih, int region, char *vars, int count);
extern int	otp_lock(si_t *sih);
extern int	otp_nvwrite(void *oh, uint16 *data, uint wlen);
#endif /* BCMNVRAMW */
#endif /* !defined(BCMDONGLEHOST) */

#if defined(WLTEST)
extern int	otp_dump(void *oh, int arg, char *buf, uint size);
extern int	otp_read(void *oh, void *arg, char *buf, uint size);
extern int	otp_dumpstats(void *oh, int arg, char *buf, uint size);
#endif // endif

#if !defined(BCMDONGLEHOST) && defined(BCMNVRAMW)
#define otp_write_rde(oh, rde, bit, val)	ipxotp_write_rde(oh, rde, bit, val)
extern int	ipxotp_write_rde(void *oh, int rde, uint bit, uint val);
extern int otp_write_bits(void *oh, uint offset, int bits, uint8* data);
extern int otp_ecc_write(void *oh, uint offset, uint32 data, uint32 type);

#ifdef OTP_DEBUG
extern int otp_verify1x(void *oh, uint off, uint fuse);
extern int otp_read1x(void *oh, uint off, uint fuse);
extern int otp_write_ones(void *oh, uint off, uint bits);
extern int otp_write_ones_old(void *oh, uint off, uint bits);
#endif // endif
#endif /* !defined(BCMDONGLEHOST) && defined(BCMNVRAMW) */

extern int	otp_ecc_rows_dump(void *oh, int *arg, char *buf, uint size);

extern uint32 otp_ecc_status(si_t *sih, uint offset);
extern int otp_ecc_enable(si_t *sih, uint enab);
extern int otp_ecc_clear_dblerrbit(si_t *sih);

extern int otp_pcie_hwhdr_sz(si_t *sih);

#endif /* _bcmotp_h_ */
