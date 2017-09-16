/*
 * bl-device-discovery.h
 *
 * Copyright (c) 2010 EMC Corporation, Haiying Tang <Tang_Haiying@emc.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef BL_DEVICE_DISCOVERY_H
#define BL_DEVICE_DISCOVERY_H

#include <stdint.h>

enum blk_vol_type {
	BLOCK_VOLUME_SIMPLE = 0,	/* maps to a single LU */
	BLOCK_VOLUME_SLICE = 1,		/* slice of another volume */
	BLOCK_VOLUME_CONCAT = 2,	/* concatenation of multiple volumes */
	BLOCK_VOLUME_STRIPE = 3,	/* striped across multiple volumes */
	BLOCK_VOLUME_PSEUDO = 4,
};

/* All disk offset/lengths are stored in 512-byte sectors */
struct bl_volume {
	uint32_t bv_type;
	off_t bv_size;
	struct bl_volume **bv_vols;
	int bv_vol_n;
	union {
		dev_t bv_dev;		/* for BLOCK_VOLUME_SIMPLE(PSEUDO) */
		off_t bv_stripe_unit;	/* for BLOCK_VOLUME_STRIPE(CONCAT) */
		off_t bv_offset;	/* for BLOCK_VOLUME_SLICE */
	} param;
};

struct bl_sig_comp {
	int64_t bs_offset;		/* In bytes */
	uint32_t bs_length;		/* In bytes */
	char *bs_string;
};

/* Maximum number of signatures components in a simple volume */
# define BLOCK_MAX_SIG_COMP 16

struct bl_sig {
	int si_num_comps;
	struct bl_sig_comp si_comps[BLOCK_MAX_SIG_COMP];
};

/*
 * Multipath support: ACTIVE or PSEUDO device is valid,
 *		      PASSIVE is a standby for ACTIVE.
 */
enum bl_path_state_e {
	BL_PATH_STATE_PASSIVE = 1,
	BL_PATH_STATE_ACTIVE = 2,
	BL_PATH_STATE_PSEUDO = 3,
};

struct bl_serial {
	int len;
	char *data;
};

struct bl_disk_path {
	struct bl_disk_path *next;
	char *full_path;
	enum bl_path_state_e state;
};

struct bl_disk {
	struct bl_disk *next;
	struct bl_serial *serial;
	dev_t dev;
	off_t size;			/* in 512-byte sectors */
	struct bl_disk_path *valid_path;
	struct bl_disk_path *paths;
};

struct bl_dev_id {
	unsigned char type;
	unsigned char ids;
	unsigned char reserve;
	unsigned char len;
	char data[0];
};

struct bl_dev_msg {
	int status;
	uint32_t major, minor;
};

struct bl_pipemsg_hdr {
	uint8_t type;
	uint16_t totallen;		/* length of message excluding hdr */
};

#define BL_DEVICE_UMOUNT                0x0	/* Umount--delete devices */
#define BL_DEVICE_MOUNT                 0x1	/* Mount--create devices */
#define BL_DEVICE_REQUEST_INIT          0x0	/* Start request */
#define BL_DEVICE_REQUEST_PROC          0x1	/* User process succeeds */
#define BL_DEVICE_REQUEST_ERR           0x2	/* User process fails */

uint32_t *blk_overflow(uint32_t * p, uint32_t * end, size_t nbytes);

#define BLK_READBUF(p, e, nbytes)  do { \
	p = blk_overflow(p, e, nbytes); \
	if (!p) {\
		goto out_err;\
	} \
} while (0)

#define READ32(x)         (x) = ntohl(*p++)

#define READ64(x)         do {                  \
	(x) = (uint64_t)ntohl(*p++) << 32;           \
	(x) |= ntohl(*p++);                     \
} while (0)

#define READ_SECTOR(x)     do { \
	READ64(tmp); \
	if (tmp & 0x1ff) { \
		goto out_err; \
	} \
	(x) = tmp >> 9; \
} while (0)

extern struct bl_disk *visible_disk_list;
uint64_t dm_device_create(struct bl_volume *vols, int num_vols);
int dm_device_remove_all(uint64_t *dev);
uint64_t process_deviceinfo(const char *dev_addr_buf,
			    unsigned int dev_addr_len,
			    uint32_t *major, uint32_t *minor);

extern ssize_t atomicio(ssize_t(*f) (int, void *, size_t),
			int fd, void *_s, size_t n);
extern struct bl_serial *bldev_read_serial(int fd, const char *filename);
extern enum bl_path_state_e bldev_read_ap_state(int fd);
extern int bl_discover_devices(void);

#define BL_LOG_INFO(fmt...)		syslog(LOG_INFO, fmt)
#define BL_LOG_WARNING(fmt...)		syslog(LOG_WARNING, fmt)
#define BL_LOG_ERR(fmt...)		syslog(LOG_ERR, fmt)
#define BL_LOG_DEBUG(fmt...)		syslog(LOG_DEBUG, fmt)
#endif
