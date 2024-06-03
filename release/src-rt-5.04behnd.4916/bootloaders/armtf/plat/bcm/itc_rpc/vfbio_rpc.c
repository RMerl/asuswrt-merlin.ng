/* SPDX-License-Identifier: GPL-2.0+
 *
 *  Copyright 2019 Broadcom Ltd.
 */

#include <stdio.h>
#include <memalign.h>

#include "itc_rpc.h"
#include "vfbio_rpc.h"

enum vfbio_func_idx {
	VFBIO_ALLOC = 0,
	VFBIO_FREE,
	VFBIO_LUN_ID,
	VFBIO_LUN_INFO,
	VFBIO_READ,
	VFBIO_WRITE,
	VFBIO_DISCARD,
	VFBIO_FLUSH,
	VFBIO_SG_READ,
	VFBIO_SG_WRITE,
	VFBIO_ASYNC_DISCARD,
	VFBIO_ASYNC_FLUSH,
	VFBIO_ASYNC_COMPLETE,
	VFBIO_DEVICE_HEALTH,
	VFBIO_FINISH_FIRST_BOOT,
	VFBIO_LVM_LUN_CREATE,
	VFBIO_LVM_LUN_DELETE,
	VFBIO_LVM_LUN_RESIZE,
	VFBIO_LVM_LUN_RENAME,
	VFBIO_LVM_LUN_CHMOD,
	VFBIO_LVM_DEFRAGMENT,
	VFBIO_LVM_DEFRAGMENT_INFO,
	VFBIO_LVM_DEVICE_INFO,
	VFBIO_FUNC_MAX
};

struct vfbio_rename_request {
	uint8_t cnt;
	uint8_t reserved[3];
	/* cnt entries below */
	struct ent {
		uint8_t id;
		uint8_t reserved[3];
		char name[VFBIO_LUN_INFO_NAME_MAX];  /* 0-terminated unique LUN name */
	} ent[16];
};

struct vfbio_lun_create_request
{
    char lun_name[VFBIO_LUN_INFO_NAME_MAX];  /* Unique lun name */
    uint32_t lun_size;  /* Volume size in 4KB blocks (in Little Endian format) */
};

/*
 * __vfbio_lun_info
 * request encoding
 * W1: |       31..24        | 23..8 | 7..0 |
 *     | LUN_INFO_BUF[39:32] | RSVD  |  LUN |
 *
 * W2: |         31..0       |
 *     |  LUN_INFO_BUF[31:0] |
 *
 * W3: | 31..0 |
 *     |  RSVD |
 *
 * reply encoding
 * W1: | 31..24 | 23..8  | 7..0 |
 *     |   RC   |  RSVD  |  LUN |
 *
 * W2: | 31..0  |
 *     |  RSVD  |
 *
 * W3: | 31..0 |
 *     |  RSVD |
 *
 * LUN_INFO_BUF[39..32]
 * 	Upper 8 bits of 40-bit reply buffer physical address
 * LUN_INFO_BUF[31..0]
 * 	Lower 32 bits of 40-bit reply buffer physical address
 * RSVD:	Reserved
 * LUN:		Logical unit #
 */
struct __vfbio_lun_info {
	/*
	 * W0: |  31..8 |  7..0  |
	 *     |  RSVD  | BLK_SZ |
	 *
	 * W1: | 31..0  |
	 *     | N_BLKS |
	 *
	 * W2..W5: | 31..24  | 23..16  | 15..8   |  7..0   |
	 *         | NAME[3] | NAME[2] | NAME[1] | NAME[0] |
	 */
	/*
	 * blk_sz:
	 * 0 = 512B
	 * 1 = 1KB
	 * 2 = 2KB
	 * 3 = 4KB
	 * 4..7 = RSVD
	 */
	uint8_t blk_sz;
	u8 flags;
#define VFBIO_LUN_INFO_FLAG_READ_ONLY  0x01
#define VFBIO_LUN_INFO_FLAG_DYNAMIC    0x02
	uint8_t rsvd1[2];
	uint32_t n_blks;
	uint8_t name[VFBIO_LUN_INFO_NAME_MAX]; /* NULL-terminated ASCII string */
};

static inline uint8_t vfbio_lun_info_get_blk_sz(struct __vfbio_lun_info *info)
{
	return info->blk_sz & 0xff;
}

/*
 * vfbio_read, vfbio_write
 * request encoding
 * W1: |       31..24     | 23..8  | 7..0 |
 *     | DMA ADDR[39..32] | N_BLKS |  LUN |
 *
 * W2: |      31..0      |
 *     | DMA ADDR[31..0] |
 *
 * W3: | 31..0 |
 *     |  BLK  |
 *
 * N_BLKS:		# of blocks to read/write
 * DMA ADDR[39..32]:	Upper 8 bits of 40-bit DMA address
 * LUN:			Logical unit #
 * DMA ADDR[31..0]:	Lower 32 bits of 40-bit DMA address
 * BLK:			Starting block in LUN
 *
 * reply encoding same as request except
 * N_BLKS:	# of blocks actually read/written
 */
static inline uint32_t vfbio_msg_get_n_blks(rpc_msg *msg)
{
	return (msg->data[0] >> 8) & 0xffff;
}

static inline void vfbio_msg_set_n_blks(rpc_msg *msg, uint32_t v)
{
	msg->data[0] = (msg->data[0] & ~(0xffff << 8)) | ((v & 0xffff) << 8);
}

static inline void vfbio_msg_set_addr(rpc_msg *msg, ulong v)
{
	msg->data[0] = (msg->data[0] & ~(0xff << 24));
	msg->data[1] = (uint32_t)(v & 0xffffffff);
}

static inline void vfbio_msg_set_blk(rpc_msg *msg, uint32_t v)
{
	msg->data[2] = v;
}

static inline uint8_t vfbio_msg_get_lun(rpc_msg *msg)
{
	return (msg->data[0] & 0xff);
}

static inline void vfbio_msg_set_lun(rpc_msg *msg, uint8_t v)
{
	msg->data[0] = (msg->data[0] & ~0xff) | (v & 0xff);
}

static inline uint8_t vfbio_msg_get_retcode(rpc_msg *msg)
{
	return ((msg->data[0] >> 24) & 0xff);
}

static inline void vfbio_msg_set_size(rpc_msg *msg, uint32_t v)
{
	msg->data[1] = v;
}

static inline uint32_t vfbio_lvm_msg_get_device_info_total_blocks(rpc_msg *msg)
{
	return msg->data[1];
}

static inline uint32_t vfbio_lvm_msg_get_device_info_free_blocks(rpc_msg *msg)
{
	return msg->data[2];
}

#define VFBIO_LVM_DEFAULT_NAME_FORMAT   "dynlun-%u"
#define VFBIO_LVM_ALLOC_BLOCK_SIZE      4096    /* lun size is always defined in these units. DO NOT CHANGE! */
#define RPC_TIMEOUT  5     /* secs */

static int vfbio_request(rpc_msg *msg)
{
	int rc = 0;

	rc = rpc_send_request_timeout(RPC_TUNNEL_VFLASH_SMC_NS, msg, RPC_TIMEOUT);
	if (rc) {
		printf("vfbio: rpc_send_request failure (%d)", rc);
		goto done;
	}
	rc = vfbio_msg_get_retcode(msg);
done:
	return rc;
}

static int vfbio_rw(int lun, bool write, ulong addr, uint32_t blk, uint32_t *n_blks)
{
	int rc = 0;
	rpc_msg msg;

	rpc_msg_init(&msg, RPC_SERVICE_FLASH_BIO, write ? VFBIO_WRITE : VFBIO_READ, 0, 0, 0, 0);
	vfbio_msg_set_addr(&msg, addr);
	vfbio_msg_set_blk(&msg, blk);
	vfbio_msg_set_n_blks(&msg, *n_blks);
	vfbio_msg_set_lun(&msg, lun);
	rc = vfbio_request(&msg);
	*n_blks = vfbio_msg_get_n_blks(&msg);

	return rc;
}

static int vfbio_flush(int lun)
{
	rpc_msg msg;

	rpc_msg_init(&msg, RPC_SERVICE_FLASH_BIO, VFBIO_FLUSH, 0, 0, 0, 0);
	vfbio_msg_set_lun(&msg, lun);
	return vfbio_request(&msg);
}

int vfbio_rpc_read(int lun, ulong addr, uint32_t blk, uint32_t blk_sz, uint32_t *n_blks)
{
	invalidate_dcache_range(addr, addr + *n_blks * blk_sz - 1);
	return vfbio_rw(lun, false, addr, blk, n_blks);
}

int vfbio_rpc_write(int lun, ulong addr, uint32_t blk, uint32_t blk_sz, uint32_t *n_blks)
{
	int ret;
	
	flush_dcache_range(addr, addr + *n_blks * blk_sz - 1);
	ret = vfbio_rw(lun, true, addr, blk, n_blks);
	vfbio_flush(lun);
	
	return ret;
}

int vfbio_rpc_lun_info(int lun, struct vfbio_lun_info *info)
{
	ALLOC_CACHE_ALIGN_BUFFER(struct __vfbio_lun_info, lun_info, 1);
	rpc_msg msg;
	int rc = 0;

	rpc_msg_init(&msg, RPC_SERVICE_FLASH_BIO, VFBIO_LUN_INFO, 0, 0, 0, 0);
	vfbio_msg_set_addr(&msg, (ulong)lun_info);
	vfbio_msg_set_lun(&msg, lun);
	invalidate_dcache_range((ulong)lun_info, (ulong)lun_info + sizeof(struct __vfbio_lun_info) - 1);
	rc = vfbio_request(&msg);
	if(rc)
		goto done;
	memcpy(info->name, lun_info->name, VFBIO_LUN_INFO_NAME_MAX);
	info->name[VFBIO_LUN_INFO_NAME_MAX - 1] = '\0';
	info->blk_sz = (1 << vfbio_lun_info_get_blk_sz(lun_info)) << 9;
	info->n_blks = lun_info->n_blks;
        info->read_only = (lun_info->flags & VFBIO_LUN_INFO_FLAG_READ_ONLY) != 0;
        info->dynamic = (lun_info->flags & VFBIO_LUN_INFO_FLAG_DYNAMIC) != 0;

done:
	return rc;
}

int vfbio_rpc_finish_first_boot(void)
{
	rpc_msg msg;

	rpc_msg_init(&msg, RPC_SERVICE_FLASH_BIO, VFBIO_FINISH_FIRST_BOOT, 0, 0, 0, 0);
	return vfbio_request(&msg);
}

int vfbio_rpc_lun_create( int lun, const char *name, uint32_t size)
{
	ALLOC_CACHE_ALIGN_BUFFER(struct vfbio_lun_create_request, req, 1);
	rpc_msg msg;

	/* Validation */
	if(!size)
		return -1;

	/* Automatic name assignment */
	if(!name || !*name)
	{
		if(lun < 0)
		{
			printf("automatic lun name and lun id assignment can be used simultaneously\n");
			return -1;
		}
	    snprintf(req->lun_name, VFBIO_LUN_INFO_NAME_MAX, VFBIO_LVM_DEFAULT_NAME_FORMAT, lun);
	}
	else
	{
		strncpy(req->lun_name, name, VFBIO_LUN_INFO_NAME_MAX);
	}
	req->lun_name[VFBIO_LUN_INFO_NAME_MAX - 1] = 0;

	req->lun_size = (size + VFBIO_LVM_ALLOC_BLOCK_SIZE - 1)/VFBIO_LVM_ALLOC_BLOCK_SIZE;
    
	rpc_msg_init(&msg, RPC_SERVICE_FLASH_BIO, VFBIO_LVM_LUN_CREATE, 0, 0, 0, 0);
	vfbio_msg_set_addr(&msg, (ulong)req);
	if(lun < 0)
		 lun = 0xff;
	vfbio_msg_set_lun(&msg, lun);

	flush_dcache_range((ulong)req, (ulong)req + sizeof(struct vfbio_lun_create_request) - 1);
	if(vfbio_request(&msg))
		return -1;
	
	return vfbio_msg_get_lun(&msg);
}

int vfbio_rpc_lun_delete(int lun)
{
	rpc_msg msg;

	rpc_msg_init(&msg, RPC_SERVICE_FLASH_BIO, VFBIO_LVM_LUN_DELETE, 0, 0, 0, 0);
	vfbio_msg_set_lun(&msg, lun);
	return vfbio_request(&msg);
}

int vfbio_rpc_lun_resize(int lun, uint32_t size)
{
	rpc_msg msg;
	
	rpc_msg_init(&msg, RPC_SERVICE_FLASH_BIO, VFBIO_LVM_LUN_RESIZE, 0, 0, 0, 0);
	vfbio_msg_set_size(&msg, (size + VFBIO_LVM_ALLOC_BLOCK_SIZE - 1)/VFBIO_LVM_ALLOC_BLOCK_SIZE);
	vfbio_msg_set_lun(&msg, lun);
	return vfbio_request(&msg);
}

int vfbio_rpc_lun_rename(uint8_t num_luns, struct vfbio_lun_id_name id_name[])
{
	ALLOC_CACHE_ALIGN_BUFFER(struct vfbio_rename_request, req, 1);
	rpc_msg msg;
	int i;
	
	if(num_luns > 16)
		return -1;
	
	for(i=0; i<num_luns; i++)
	{
		req->ent[i].id = id_name[i].id;
		strncpy(req->ent[i].name, id_name[i].name, VFBIO_LUN_INFO_NAME_MAX);
		req->ent[i].name[VFBIO_LUN_INFO_NAME_MAX - 1] = 0;
	}
	
	rpc_msg_init(&msg, RPC_SERVICE_FLASH_BIO, VFBIO_LVM_LUN_RENAME, 0, 0, 0, 0);
	vfbio_msg_set_addr(&msg, (ulong)req);
	flush_dcache_range((ulong)req, (ulong)req + sizeof(struct vfbio_rename_request));
	return vfbio_request(&msg);
}

int vfbio_rpc_device_get_info(uint64_t *total_size, uint64_t *free_size)
{
	rpc_msg msg;
	int rc;

	rpc_msg_init(&msg, RPC_SERVICE_FLASH_BIO, VFBIO_LVM_DEVICE_INFO, 0, 0, 0, 0);

	rc = vfbio_request(&msg);
	
	*total_size = (uint64_t)vfbio_lvm_msg_get_device_info_total_blocks(&msg) * VFBIO_LVM_ALLOC_BLOCK_SIZE;
	*free_size = (uint64_t)vfbio_lvm_msg_get_device_info_free_blocks(&msg) * VFBIO_LVM_ALLOC_BLOCK_SIZE;
	return rc;
}

