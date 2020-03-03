#ifndef __TARGET_CORE_USER_H
#define __TARGET_CORE_USER_H

/* This header will be used by application too */

#include <linux/types.h>
#include <linux/uio.h>

#define TCMU_VERSION "2.0"

/*
 * Ring Design
 * -----------
 *
 * The mmaped area is divided into three parts:
 * 1) The mailbox (struct tcmu_mailbox, below)
 * 2) The command ring
 * 3) Everything beyond the command ring (data)
 *
 * The mailbox tells userspace the offset of the command ring from the
 * start of the shared memory region, and how big the command ring is.
 *
 * The kernel passes SCSI commands to userspace by putting a struct
 * tcmu_cmd_entry in the ring, updating mailbox->cmd_head, and poking
 * userspace via uio's interrupt mechanism.
 *
 * tcmu_cmd_entry contains a header. If the header type is PAD,
 * userspace should skip hdr->length bytes (mod cmdr_size) to find the
 * next cmd_entry.
 *
 * Otherwise, the entry will contain offsets into the mmaped area that
 * contain the cdb and data buffers -- the latter accessible via the
 * iov array. iov addresses are also offsets into the shared area.
 *
 * When userspace is completed handling the command, set
 * entry->rsp.scsi_status, fill in rsp.sense_buffer if appropriate,
 * and also set mailbox->cmd_tail equal to the old cmd_tail plus
 * hdr->length, mod cmdr_size. If cmd_tail doesn't equal cmd_head, it
 * should process the next packet the same way, and so on.
 */

#define TCMU_MAILBOX_VERSION 2
#define ALIGN_SIZE 64 /* Should be enough for most CPUs */

/* See https://gcc.gnu.org/onlinedocs/cpp/Stringification.html */
#define xstr(s) str(s)
#define str(s) #s

struct tcmu_mailbox {
	__u16 version;
	__u16 flags;
	__u32 cmdr_off;
	__u32 cmdr_size;

	__u32 cmd_head;

	/* Updated by user. On its own cacheline */
	__u32 cmd_tail __attribute__((__aligned__(ALIGN_SIZE)));

} __packed;

enum tcmu_opcode {
	TCMU_OP_PAD = 0,
	TCMU_OP_CMD,
};

/*
 * Only a few opcodes, and length is 8-byte aligned, so use low bits for opcode.
 */
struct tcmu_cmd_entry_hdr {
	__u32 len_op;
	__u16 cmd_id;
	__u8 kflags;
#define TCMU_UFLAG_UNKNOWN_OP 0x1
	__u8 uflags;

} __packed;

#define TCMU_OP_MASK 0x7

static inline enum tcmu_opcode tcmu_hdr_get_op(__u32 len_op)
{
	return len_op & TCMU_OP_MASK;
}

static inline void tcmu_hdr_set_op(__u32 *len_op, enum tcmu_opcode op)
{
	*len_op &= ~TCMU_OP_MASK;
	*len_op |= (op & TCMU_OP_MASK);
}

static inline __u32 tcmu_hdr_get_len(__u32 len_op)
{
	return len_op & ~TCMU_OP_MASK;
}

static inline void tcmu_hdr_set_len(__u32 *len_op, __u32 len)
{
	*len_op &= TCMU_OP_MASK;
	*len_op |= len;
}

/* Currently the same as SCSI_SENSE_BUFFERSIZE */
#define TCMU_SENSE_BUFFERSIZE 96

struct tcmu_cmd_entry {
	struct tcmu_cmd_entry_hdr hdr;

	union {
		struct {
			uint32_t iov_cnt;
			uint32_t iov_bidi_cnt;
			uint32_t iov_dif_cnt;
			uint64_t cdb_off;
			uint64_t __pad1;
			uint64_t __pad2;
			struct iovec iov[0];
		} req;
		struct {
			uint8_t scsi_status;
			uint8_t __pad1;
			uint16_t __pad2;
			uint32_t __pad3;
			char sense_buffer[TCMU_SENSE_BUFFERSIZE];
		} rsp;
	};

} __packed;

#define TCMU_OP_ALIGN_SIZE sizeof(uint64_t)

enum tcmu_genl_cmd {
	TCMU_CMD_UNSPEC,
	TCMU_CMD_ADDED_DEVICE,
	TCMU_CMD_REMOVED_DEVICE,
	__TCMU_CMD_MAX,
};
#define TCMU_CMD_MAX (__TCMU_CMD_MAX - 1)

enum tcmu_genl_attr {
	TCMU_ATTR_UNSPEC,
	TCMU_ATTR_DEVICE,
	TCMU_ATTR_MINOR,
	__TCMU_ATTR_MAX,
};
#define TCMU_ATTR_MAX (__TCMU_ATTR_MAX - 1)

#endif
