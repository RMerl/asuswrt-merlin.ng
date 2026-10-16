/*
 * Copyright (C) 2024 Asus Networks
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * Explicit 64-bit XFRM netlink wire-layout marshalling for 32-bit ARM userspace
 * on 64-bit AArch64 kernel (BCM4916 / GT-BN98_PRO and similar platforms).
 *
 * ── The problem ──────────────────────────────────────────────────────────────
 *
 * On AArch64 kernels, every 32-bit (ARM32/AAPCS) process has TIF_32BIT set.
 * The kernel's netlink dispatch therefore routes all XFRM messages through
 * xfrm_user_rcv_msg_compat() instead of xfrm_user_rcv_msg().
 *
 * The compat handler uses compat_xfrm_usersa_info / compat_xfrm_userpolicy_info
 * (defined in net/xfrm/xfrm_compat.c), which use:
 *
 *   typedef u64 __attribute__((aligned(4))) compat_u64;  // AArch64 compat.h
 *
 * Because compat_u64 has only 4-byte alignment, the compat structs are
 * 4 bytes SMALLER than the 64-bit native structs:
 *
 *   struct                         compat size   native (64-bit) size
 *   ─────────────────────────────  ───────────   ────────────────────
 *   compat_xfrm_usersa_info           220 B         224 B
 *   compat_xfrm_userpolicy_info       164 B         168 B
 *   compat_xfrm_userspi_info          228 B         232 B
 *
 * ARM32 (AAPCS) however requires __u64 to be 8-byte aligned, exactly like
 * AArch64.  Therefore the ARM32 cross-compiled structs have the same layout
 * as the 64-bit native structs — NOT the compat layout:
 *
 *   sizeof(struct xfrm_usersa_info)     on ARM32 gcc = 224 B  (verified)
 *   sizeof(struct xfrm_userpolicy_info) on ARM32 gcc = 168 B  (verified)
 *   sizeof(struct xfrm_userspi_info)    on ARM32 gcc = 232 B  (verified)
 *
 * The compat handler calls nlmsg_parse_deprecated() with hdrlen = 220 (for
 * NEWSA).  When ARM32 strongSwan sends 224 bytes (native size), the parser
 * skips only 220 bytes as the "header" and tries to parse bytes 220-223
 * (which are ARM32's trailing padding = four zero bytes) as netlink
 * attributes.  A zero nla_len is invalid; nla_ok() returns false immediately,
 * and real attributes that follow (crypto algorithms, marks, etc.) are never
 * parsed.  The kernel returns EINVAL and the SA install fails.
 *
 * ── The fix ──────────────────────────────────────────────────────────────────
 *
 * Two-part solution:
 *
 *   1. Kernel side (Plan B patch): disable the compat-path for XFRM on this
 *      platform so ARM32 process messages are handled by the native 64-bit
 *      xfrm_user_rcv_msg() path, which correctly expects 224-byte structs.
 *
 *   2. strongSwan side (this file): explicitly marshal XFRM structs into the
 *      64-bit native wire layout using pinned byte offsets, independent of the
 *      compiler's struct-padding decisions.  The marshal functions ensure the
 *      netlink buffer always contains the layout the native handler expects,
 *      and NLMSG_LENGTH() uses the 64-bit size constants so that netlink
 *      attributes are appended at the correct offset.
 *
 * The static assertions in this file verify at compile time that the native
 * ARM32 struct layout actually matches the 64-bit constants.  If a build uses
 * a non-AAPCS ABI where __u64 has only 4-byte alignment (e.g. -mabi=apcs-gnu),
 * the assertions will fire and alert the developer before a silent mis-match
 * reaches the kernel.
 */

#ifndef KERNEL_NETLINK_XFRM64_H_
#define KERNEL_NETLINK_XFRM64_H_

#include <stdint.h>
#include <string.h>
#include <stddef.h>
#include <linux/xfrm.h>

/*
 * XFRM_FORCE_64BIT_LAYOUT
 *
 * Passed via -D from Makefile.am (AM_CPPFLAGS).  When defined, the explicit
 * 64-bit marshalling path in kernel_netlink_ipsec.c is compiled in:
 *   • XFRM_WIRE_SZ_* macros expand to the XFRM64_* constants below.
 *   • SA/policy structs are built into local temporaries and serialised to
 *     the netlink buffer via xfrm64_marshal_*() before each send.
 *   • Static assertions verify that the native struct layout matches the
 *     64-bit constants at compile time.
 *
 * When not defined, the original upstream strongSwan behaviour is used:
 *   • XFRM_WIRE_SZ_* macros expand to sizeof(struct xfrm_*).
 *   • Struct pointers point directly to NLMSG_DATA(hdr); no marshalling.
 *
 * To disable (restore original path), remove -DXFRM_FORCE_64BIT_LAYOUT
 * from Makefile.am or pass -UXFRM_FORCE_64BIT_LAYOUT in CPPFLAGS.
 */

/* ── Wire sizes: 64-bit (AArch64 / ARM32 AAPCS) UAPI layout ─────────────── */
/*
 * These are the sizes the AArch64 native XFRM handler expects.
 * They are also the sizes that ARM32 (AAPCS, __u64 8-byte aligned) produces
 * naturally.  They differ from the *compat* sizes by 4 bytes (the extra
 * trailing padding that 8-byte struct alignment adds on 64-bit).
 *
 * Compat sizes for reference (what the compat handler wrongly expects for
 * ARM32 on AArch64 because compat_u64 has only 4-byte alignment):
 *   compat_xfrm_usersa_info      = 220 B  (we send 224)
 *   compat_xfrm_userpolicy_info  = 164 B  (we send 168)
 *   compat_xfrm_userspi_info     = 228 B  (we send 232)
 */
#define XFRM64_SELECTOR_SIZE            56u
#define XFRM64_ID_SIZE                  24u
#define XFRM64_LIFETIME_CFG_SIZE        64u
#define XFRM64_LIFETIME_CUR_SIZE        32u
#define XFRM64_STATS_SIZE               12u
#define XFRM64_USERSA_INFO_SIZE        224u   /* compat = 220 */
#define XFRM64_USERSPI_INFO_SIZE       232u   /* compat = 228 */
#define XFRM64_USERPOLICY_INFO_SIZE    168u   /* compat = 164 */
#define XFRM64_USERSA_ID_SIZE           24u   /* same in compat */
#define XFRM64_USERPOLICY_ID_SIZE       64u   /* same in compat */
#define XFRM64_USER_TMPL_SIZE           64u   /* same in compat */

/*
 * XFRM_WIRE_SZ_* — the sizes used in NLMSG_LENGTH() and memcpy() calls.
 *
 * With XFRM_FORCE_64BIT_LAYOUT: explicit 64-bit constants above.
 * Without: native sizeof(), which is the original upstream behaviour.
 */
#ifdef XFRM_FORCE_64BIT_LAYOUT
# define XFRM_WIRE_SZ_USERSA_INFO       XFRM64_USERSA_INFO_SIZE
# define XFRM_WIRE_SZ_USERSPI_INFO      XFRM64_USERSPI_INFO_SIZE
# define XFRM_WIRE_SZ_USERPOLICY_INFO   XFRM64_USERPOLICY_INFO_SIZE
# define XFRM_WIRE_SZ_USERSA_ID         XFRM64_USERSA_ID_SIZE
# define XFRM_WIRE_SZ_USERPOLICY_ID     XFRM64_USERPOLICY_ID_SIZE
#else
# define XFRM_WIRE_SZ_USERSA_INFO       ((unsigned)sizeof(struct xfrm_usersa_info))
# define XFRM_WIRE_SZ_USERSPI_INFO      ((unsigned)sizeof(struct xfrm_userspi_info))
# define XFRM_WIRE_SZ_USERPOLICY_INFO   ((unsigned)sizeof(struct xfrm_userpolicy_info))
# define XFRM_WIRE_SZ_USERSA_ID         ((unsigned)sizeof(struct xfrm_usersa_id))
# define XFRM_WIRE_SZ_USERPOLICY_ID     ((unsigned)sizeof(struct xfrm_userpolicy_id))
#endif

/* ── Field offsets: xfrm_selector ──────────────────────────────────────── */

#define XFRM64_SEL_DADDR        0u   /* xfrm_address_t (16 B) */
#define XFRM64_SEL_SADDR       16u   /* xfrm_address_t (16 B) */
#define XFRM64_SEL_DPORT       32u   /* __be16          ( 2 B) */
#define XFRM64_SEL_DPORT_MASK  34u   /* __be16          ( 2 B) */
#define XFRM64_SEL_SPORT       36u   /* __be16          ( 2 B) */
#define XFRM64_SEL_SPORT_MASK  38u   /* __be16          ( 2 B) */
#define XFRM64_SEL_FAMILY      40u   /* __u16           ( 2 B) */
#define XFRM64_SEL_PREFIXLEN_D 42u   /* __u8            ( 1 B) */
#define XFRM64_SEL_PREFIXLEN_S 43u   /* __u8            ( 1 B) */
#define XFRM64_SEL_PROTO       44u   /* __u8            ( 1 B) */
                                     /* 3 B implicit padding   */
#define XFRM64_SEL_IFINDEX     48u   /* int             ( 4 B) */
#define XFRM64_SEL_USER        52u   /* __kernel_uid32_t( 4 B) */

/* ── Field offsets: xfrm_id ─────────────────────────────────────────────── */

#define XFRM64_ID_DADDR         0u   /* xfrm_address_t (16 B) */
#define XFRM64_ID_SPI          16u   /* __be32          ( 4 B) */
#define XFRM64_ID_PROTO        20u   /* __u8            ( 1 B) */
                                     /* 3 B implicit padding   */

/* ── Field offsets: xfrm_lifetime_cfg ──────────────────────────────────── */

#define XFRM64_LFT_SOFT_BYTE_LIMIT    0u   /* __u64 */
#define XFRM64_LFT_HARD_BYTE_LIMIT    8u
#define XFRM64_LFT_SOFT_PKT_LIMIT    16u
#define XFRM64_LFT_HARD_PKT_LIMIT    24u
#define XFRM64_LFT_SOFT_ADD_EXPIRES  32u
#define XFRM64_LFT_HARD_ADD_EXPIRES  40u
#define XFRM64_LFT_SOFT_USE_EXPIRES  48u
#define XFRM64_LFT_HARD_USE_EXPIRES  56u

/* ── Field offsets: xfrm_usersa_info (native 64-bit layout, 224 B) ──────── */
/*
 * All field positions are identical between ARM32-native and AArch64-native
 * (both ABIs have 8-byte aligned __u64).  The difference from the compat
 * layout (220 B) is only the trailing padding:
 *   native:  7 B pad after flags byte (offset 216) → total 224 B (align 8)
 *   compat:  3 B pad after flags byte (offset 216) → total 220 B (align 4)
 */
#define XFRM64_SA_SEL           0u   /* xfrm_selector        (56 B) */
#define XFRM64_SA_ID           56u   /* xfrm_id              (24 B) */
#define XFRM64_SA_SADDR        80u   /* xfrm_address_t       (16 B) */
#define XFRM64_SA_LFT          96u   /* xfrm_lifetime_cfg    (64 B) */
#define XFRM64_SA_CURLFT      160u   /* xfrm_lifetime_cur    (32 B) */
#define XFRM64_SA_STATS       192u   /* xfrm_stats           (12 B) */
#define XFRM64_SA_SEQ         204u   /* __u32 */
#define XFRM64_SA_REQID       208u   /* __u32 */
#define XFRM64_SA_FAMILY      212u   /* __u16 */
#define XFRM64_SA_MODE        214u   /* __u8  */
#define XFRM64_SA_REPLAY_WIN  215u   /* __u8  */
#define XFRM64_SA_FLAGS       216u   /* __u8  */
                                     /* 7 B implicit padding → total 224 B */

/* ── Field offsets: xfrm_userspi_info (232 B) ───────────────────────────── */

#define XFRM64_SPI_INFO         0u   /* xfrm_usersa_info    (224 B) */
#define XFRM64_SPI_MIN        224u   /* __u32 */
#define XFRM64_SPI_MAX        228u   /* __u32 */

/* ── Field offsets: xfrm_userpolicy_info (native 64-bit layout, 168 B) ──── */
/*
 * compat size = 164 B (4 B trailing padding at align 4)
 * native size = 168 B (4 B trailing padding at align 8, same field values)
 */
#define XFRM64_POL_SEL          0u   /* xfrm_selector        (56 B) */
#define XFRM64_POL_LFT         56u   /* xfrm_lifetime_cfg    (64 B) */
#define XFRM64_POL_CURLFT     120u   /* xfrm_lifetime_cur    (32 B) */
#define XFRM64_POL_PRIORITY   152u   /* __u32 */
#define XFRM64_POL_INDEX      156u   /* __u32 */
#define XFRM64_POL_DIR        160u   /* __u8  */
#define XFRM64_POL_ACTION     161u   /* __u8  */
#define XFRM64_POL_FLAGS      162u   /* __u8  */
#define XFRM64_POL_SHARE      163u   /* __u8  */
                                     /* 4 B implicit padding → total 168 B */

/* ── Compile-time layout verification ──────────────────────────────────── */
/*
 * On ARM32 (AAPCS), __u64 is 8-byte aligned, matching AArch64.  These
 * assertions verify that the native struct sizes match the 64-bit constants
 * documented above.  A failure indicates a non-AAPCS ABI is in use (e.g.
 * -mabi=apcs-gnu where long long is only 4-byte aligned), in which case the
 * ARM32 process would send the compat-sized structs and the kernel Plan B
 * path would need adaptation.
 */
#define XFRM64_ASSERT_SIZE(type, expected) \
	typedef char xfrm64_sz_##type \
		[((int)sizeof(struct type) == (int)(expected)) ? 1 : -1]

#define XFRM64_ASSERT_OFF(type, field, expected) \
	typedef char xfrm64_off_##type##_##field \
		[((int)offsetof(struct type, field) == (int)(expected)) ? 1 : -1]

XFRM64_ASSERT_SIZE(xfrm_selector,        XFRM64_SELECTOR_SIZE);
XFRM64_ASSERT_SIZE(xfrm_id,              XFRM64_ID_SIZE);
XFRM64_ASSERT_SIZE(xfrm_lifetime_cfg,    XFRM64_LIFETIME_CFG_SIZE);
XFRM64_ASSERT_SIZE(xfrm_lifetime_cur,    XFRM64_LIFETIME_CUR_SIZE);
XFRM64_ASSERT_SIZE(xfrm_stats,           XFRM64_STATS_SIZE);
XFRM64_ASSERT_SIZE(xfrm_usersa_info,     XFRM64_USERSA_INFO_SIZE);
XFRM64_ASSERT_SIZE(xfrm_userspi_info,    XFRM64_USERSPI_INFO_SIZE);
XFRM64_ASSERT_SIZE(xfrm_userpolicy_info, XFRM64_USERPOLICY_INFO_SIZE);
XFRM64_ASSERT_SIZE(xfrm_usersa_id,       XFRM64_USERSA_ID_SIZE);
XFRM64_ASSERT_SIZE(xfrm_userpolicy_id,   XFRM64_USERPOLICY_ID_SIZE);
XFRM64_ASSERT_SIZE(xfrm_user_tmpl,       XFRM64_USER_TMPL_SIZE);

XFRM64_ASSERT_OFF(xfrm_usersa_info, sel,           XFRM64_SA_SEL);
XFRM64_ASSERT_OFF(xfrm_usersa_info, id,            XFRM64_SA_ID);
XFRM64_ASSERT_OFF(xfrm_usersa_info, saddr,         XFRM64_SA_SADDR);
XFRM64_ASSERT_OFF(xfrm_usersa_info, lft,           XFRM64_SA_LFT);
XFRM64_ASSERT_OFF(xfrm_usersa_info, curlft,        XFRM64_SA_CURLFT);
XFRM64_ASSERT_OFF(xfrm_usersa_info, stats,         XFRM64_SA_STATS);
XFRM64_ASSERT_OFF(xfrm_usersa_info, seq,           XFRM64_SA_SEQ);
XFRM64_ASSERT_OFF(xfrm_usersa_info, reqid,         XFRM64_SA_REQID);
XFRM64_ASSERT_OFF(xfrm_usersa_info, family,        XFRM64_SA_FAMILY);
XFRM64_ASSERT_OFF(xfrm_usersa_info, mode,          XFRM64_SA_MODE);
XFRM64_ASSERT_OFF(xfrm_usersa_info, replay_window, XFRM64_SA_REPLAY_WIN);
XFRM64_ASSERT_OFF(xfrm_usersa_info, flags,         XFRM64_SA_FLAGS);
XFRM64_ASSERT_OFF(xfrm_userpolicy_info, sel,       XFRM64_POL_SEL);
XFRM64_ASSERT_OFF(xfrm_userpolicy_info, lft,       XFRM64_POL_LFT);
XFRM64_ASSERT_OFF(xfrm_userpolicy_info, curlft,    XFRM64_POL_CURLFT);
XFRM64_ASSERT_OFF(xfrm_userpolicy_info, priority,  XFRM64_POL_PRIORITY);
XFRM64_ASSERT_OFF(xfrm_userpolicy_info, index,     XFRM64_POL_INDEX);
XFRM64_ASSERT_OFF(xfrm_userpolicy_info, dir,       XFRM64_POL_DIR);
XFRM64_ASSERT_OFF(xfrm_userpolicy_info, action,    XFRM64_POL_ACTION);
XFRM64_ASSERT_OFF(xfrm_userpolicy_info, flags,     XFRM64_POL_FLAGS);
XFRM64_ASSERT_OFF(xfrm_userpolicy_info, share,     XFRM64_POL_SHARE);

/* ── Primitive write helpers ─────────────────────────────────────────────── */
/*
 * Use memcpy to avoid strict-aliasing and unaligned-access UB regardless of
 * buf's alignment.
 */

static inline void xfrm64_put8(uint8_t *buf, uint8_t v)
{
	buf[0] = v;
}

static inline void xfrm64_put16(uint8_t *buf, uint16_t v)
{
	memcpy(buf, &v, 2);
}

static inline void xfrm64_put32(uint8_t *buf, uint32_t v)
{
	memcpy(buf, &v, 4);
}

static inline void xfrm64_put64(uint8_t *buf, uint64_t v)
{
	memcpy(buf, &v, 8);
}

/* ── Marshal functions ───────────────────────────────────────────────────── */
/*
 * Each function writes the struct into the netlink send buffer at the exact
 * 64-bit (native/ARM32-AAPCS) byte offsets listed above.  The buffer must be
 * pre-allocated to at least the corresponding XFRM64_*_SIZE bytes.
 *
 * On ARM32 (AAPCS) the native struct layout is identical to AArch64 native,
 * so these functions are effectively a memcpy — but they remain correct even
 * if the build flags or compiler version change the native layout, because
 * every field is written at its documented offset explicitly.
 */

/**
 * Marshal an xfrm_selector into 64-bit wire layout at buf.
 */
static inline void xfrm64_marshal_selector(uint8_t *buf,
										   const struct xfrm_selector *sel)
{
	memset(buf, 0, XFRM64_SELECTOR_SIZE);
	memcpy(buf + XFRM64_SEL_DADDR,        &sel->daddr,       sizeof(sel->daddr));
	memcpy(buf + XFRM64_SEL_SADDR,        &sel->saddr,       sizeof(sel->saddr));
	xfrm64_put16(buf + XFRM64_SEL_DPORT,       sel->dport);
	xfrm64_put16(buf + XFRM64_SEL_DPORT_MASK,  sel->dport_mask);
	xfrm64_put16(buf + XFRM64_SEL_SPORT,       sel->sport);
	xfrm64_put16(buf + XFRM64_SEL_SPORT_MASK,  sel->sport_mask);
	xfrm64_put16(buf + XFRM64_SEL_FAMILY,      sel->family);
	xfrm64_put8 (buf + XFRM64_SEL_PREFIXLEN_D, sel->prefixlen_d);
	xfrm64_put8 (buf + XFRM64_SEL_PREFIXLEN_S, sel->prefixlen_s);
	xfrm64_put8 (buf + XFRM64_SEL_PROTO,       sel->proto);
	xfrm64_put32(buf + XFRM64_SEL_IFINDEX,     (uint32_t)sel->ifindex);
	xfrm64_put32(buf + XFRM64_SEL_USER,        sel->user);
}

/**
 * Marshal an xfrm_id into 64-bit wire layout at buf.
 */
static inline void xfrm64_marshal_id(uint8_t *buf, const struct xfrm_id *id)
{
	memset(buf, 0, XFRM64_ID_SIZE);
	memcpy(buf + XFRM64_ID_DADDR, &id->daddr, sizeof(id->daddr));
	xfrm64_put32(buf + XFRM64_ID_SPI,   id->spi);
	xfrm64_put8 (buf + XFRM64_ID_PROTO, id->proto);
}

/**
 * Marshal an xfrm_lifetime_cfg into 64-bit wire layout at buf.
 */
static inline void xfrm64_marshal_lifetime_cfg(uint8_t *buf,
											   const struct xfrm_lifetime_cfg *lft)
{
	xfrm64_put64(buf + XFRM64_LFT_SOFT_BYTE_LIMIT,  lft->soft_byte_limit);
	xfrm64_put64(buf + XFRM64_LFT_HARD_BYTE_LIMIT,  lft->hard_byte_limit);
	xfrm64_put64(buf + XFRM64_LFT_SOFT_PKT_LIMIT,   lft->soft_packet_limit);
	xfrm64_put64(buf + XFRM64_LFT_HARD_PKT_LIMIT,   lft->hard_packet_limit);
	xfrm64_put64(buf + XFRM64_LFT_SOFT_ADD_EXPIRES, lft->soft_add_expires_seconds);
	xfrm64_put64(buf + XFRM64_LFT_HARD_ADD_EXPIRES, lft->hard_add_expires_seconds);
	xfrm64_put64(buf + XFRM64_LFT_SOFT_USE_EXPIRES, lft->soft_use_expires_seconds);
	xfrm64_put64(buf + XFRM64_LFT_HARD_USE_EXPIRES, lft->hard_use_expires_seconds);
}

/**
 * Marshal an xfrm_usersa_info into 64-bit native wire layout (224 B) at buf.
 *
 * The buf must be pre-zeroed or the caller must not rely on curlft/stats
 * (which are kernel read-only counters and are zeroed here).
 *
 * This function is called after ALL field modifications on the local sa struct,
 * including late ones like sa->flags |= XFRM_STATE_ESN and sa->replay_window,
 * so that nothing is missed.  Netlink attributes appended by netlink_reserve()
 * reside at offsets >= XFRM64_USERSA_INFO_SIZE (224) and are not affected.
 */
static inline void xfrm64_marshal_usersa_info(uint8_t *buf,
											  const struct xfrm_usersa_info *sa)
{
	memset(buf, 0, XFRM64_USERSA_INFO_SIZE);
	xfrm64_marshal_selector    (buf + XFRM64_SA_SEL,  &sa->sel);
	xfrm64_marshal_id          (buf + XFRM64_SA_ID,   &sa->id);
	memcpy(buf + XFRM64_SA_SADDR, &sa->saddr, sizeof(sa->saddr));
	xfrm64_marshal_lifetime_cfg(buf + XFRM64_SA_LFT,  &sa->lft);
	/* curlft and stats are kernel-side counters; leave zeroed */
	xfrm64_put32(buf + XFRM64_SA_SEQ,        sa->seq);
	xfrm64_put32(buf + XFRM64_SA_REQID,      sa->reqid);
	xfrm64_put16(buf + XFRM64_SA_FAMILY,     sa->family);
	xfrm64_put8 (buf + XFRM64_SA_MODE,       sa->mode);
	xfrm64_put8 (buf + XFRM64_SA_REPLAY_WIN, sa->replay_window);
	xfrm64_put8 (buf + XFRM64_SA_FLAGS,      sa->flags);
}

/**
 * Marshal an xfrm_userspi_info (xfrm_usersa_info + SPI range) into 64-bit
 * native wire layout (232 B) at buf.
 */
static inline void xfrm64_marshal_userspi_info(uint8_t *buf,
											   const struct xfrm_userspi_info *spi)
{
	xfrm64_marshal_usersa_info(buf + XFRM64_SPI_INFO, &spi->info);
	xfrm64_put32(buf + XFRM64_SPI_MIN, spi->min);
	xfrm64_put32(buf + XFRM64_SPI_MAX, spi->max);
}

/**
 * Marshal an xfrm_userpolicy_info into 64-bit native wire layout (168 B)
 * at buf.
 */
static inline void xfrm64_marshal_userpolicy_info(uint8_t *buf,
												  const struct xfrm_userpolicy_info *pol)
{
	memset(buf, 0, XFRM64_USERPOLICY_INFO_SIZE);
	xfrm64_marshal_selector    (buf + XFRM64_POL_SEL,  &pol->sel);
	xfrm64_marshal_lifetime_cfg(buf + XFRM64_POL_LFT,  &pol->lft);
	/* curlft is kernel-side counter; leave zeroed */
	xfrm64_put32(buf + XFRM64_POL_PRIORITY, pol->priority);
	xfrm64_put32(buf + XFRM64_POL_INDEX,    pol->index);
	xfrm64_put8 (buf + XFRM64_POL_DIR,      pol->dir);
	xfrm64_put8 (buf + XFRM64_POL_ACTION,   pol->action);
	xfrm64_put8 (buf + XFRM64_POL_FLAGS,    pol->flags);
	xfrm64_put8 (buf + XFRM64_POL_SHARE,    pol->share);
}

#endif /* KERNEL_NETLINK_XFRM64_H_ */
