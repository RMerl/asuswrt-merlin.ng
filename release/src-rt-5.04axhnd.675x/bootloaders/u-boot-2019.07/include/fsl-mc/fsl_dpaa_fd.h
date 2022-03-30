/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2014 Freescale Semiconductor
 */
#ifndef __FSL_DPAA_FD_H
#define __FSL_DPAA_FD_H

/* Place-holder for FDs, we represent it via the simplest form that we need for
 * now. Different overlays may be needed to support different options, etc. (It
 * is impractical to define One True Struct, because the resulting encoding
 * routines (lots of read-modify-writes) would be worst-case performance whether
 * or not circumstances required them.) */
struct dpaa_fd {
	union {
		u32 words[8];
		struct dpaa_fd_simple {
			u32 addr_lo;
			u32 addr_hi;
			u32 len;
			/* offset in the MS 16 bits, BPID in the LS 16 bits */
			u32 bpid_offset;
			u32 frc; /* frame context */
			/* "err", "va", "cbmt", "asal", [...] */
			u32 ctrl;
			/* flow context */
			u32 flc_lo;
			u32 flc_hi;
		} simple;
	};
};

enum dpaa_fd_format {
	dpaa_fd_single = 0,
	dpaa_fd_list,
	dpaa_fd_sg
};

static inline u64 ldpaa_fd_get_addr(const struct dpaa_fd *fd)
{
	return (u64)((((uint64_t)fd->simple.addr_hi) << 32)
				+ fd->simple.addr_lo);
}

static inline void ldpaa_fd_set_addr(struct dpaa_fd *fd, u64 addr)
{
	fd->simple.addr_hi = upper_32_bits(addr);
	fd->simple.addr_lo = lower_32_bits(addr);
}

static inline u32 ldpaa_fd_get_len(const struct dpaa_fd *fd)
{
	return fd->simple.len;
}

static inline void ldpaa_fd_set_len(struct dpaa_fd *fd, u32 len)
{
	fd->simple.len = len;
}

static inline uint16_t ldpaa_fd_get_offset(const struct dpaa_fd *fd)
{
	return (uint16_t)(fd->simple.bpid_offset >> 16) & 0x0FFF;
}

static inline void ldpaa_fd_set_offset(struct dpaa_fd *fd, uint16_t offset)
{
	fd->simple.bpid_offset &= 0xF000FFFF;
	fd->simple.bpid_offset |= (u32)offset << 16;
}

static inline uint16_t ldpaa_fd_get_bpid(const struct dpaa_fd *fd)
{
	return (uint16_t)(fd->simple.bpid_offset & 0xFFFF);
}

static inline void ldpaa_fd_set_bpid(struct dpaa_fd *fd, uint16_t bpid)
{
	fd->simple.bpid_offset &= 0xFFFF0000;
	fd->simple.bpid_offset |= (u32)bpid;
}

/* When frames are dequeued, the FDs show up inside "dequeue" result structures
 * (if at all, not all dequeue results contain valid FDs). This structure type
 * is intentionally defined without internal detail, and the only reason it
 * isn't declared opaquely (without size) is to allow the user to provide
 * suitably-sized (and aligned) memory for these entries. */
struct ldpaa_dq {
	uint32_t dont_manipulate_directly[16];
};

/* Parsing frame dequeue results */
#define LDPAA_DQ_STAT_FQEMPTY       0x80
#define LDPAA_DQ_STAT_HELDACTIVE    0x40
#define LDPAA_DQ_STAT_FORCEELIGIBLE 0x20
#define LDPAA_DQ_STAT_VALIDFRAME    0x10
#define LDPAA_DQ_STAT_ODPVALID      0x04
#define LDPAA_DQ_STAT_VOLATILE      0x02
#define LDPAA_DQ_STAT_EXPIRED       0x01
uint32_t ldpaa_dq_flags(const struct ldpaa_dq *);
static inline int ldpaa_dq_is_pull(const struct ldpaa_dq *dq)
{
	return (int)(ldpaa_dq_flags(dq) & LDPAA_DQ_STAT_VOLATILE);
}
static inline int ldpaa_dq_is_pull_complete(
					const struct ldpaa_dq *dq)
{
	return (int)(ldpaa_dq_flags(dq) & LDPAA_DQ_STAT_EXPIRED);
}
/* seqnum/odpid are valid only if VALIDFRAME and ODPVALID flags are TRUE */
uint16_t ldpaa_dq_seqnum(const struct ldpaa_dq *);
uint16_t ldpaa_dq_odpid(const struct ldpaa_dq *);
uint32_t ldpaa_dq_fqid(const struct ldpaa_dq *);
uint32_t ldpaa_dq_byte_count(const struct ldpaa_dq *);
uint32_t ldpaa_dq_frame_count(const struct ldpaa_dq *);
uint32_t ldpaa_dq_fqd_ctx_hi(const struct ldpaa_dq *);
uint32_t ldpaa_dq_fqd_ctx_lo(const struct ldpaa_dq *);
/* get the Frame Descriptor */
const struct dpaa_fd *ldpaa_dq_fd(const struct ldpaa_dq *);

#endif /* __FSL_DPAA_FD_H */
