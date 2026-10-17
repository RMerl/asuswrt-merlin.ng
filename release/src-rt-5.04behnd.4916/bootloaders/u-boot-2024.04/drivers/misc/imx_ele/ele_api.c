// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2020, 2023 NXP
 *
 */

#include <common.h>
#include <hang.h>
#include <malloc.h>
#include <asm/io.h>
#include <dm.h>
#include <asm/mach-imx/ele_api.h>
#include <misc.h>

DECLARE_GLOBAL_DATA_PTR;

static u32 compute_crc(const struct ele_msg *msg)
{
	u32 crc = 0;
	size_t i = 0;
	u32 *data = (u32 *)msg;

	for (i = 0; i < (msg->size - 1); i++)
		crc ^= data[i];

	return crc;
}

int ele_release_rdc(u8 core_id, u8 xrdc, u32 *response)
{
	struct udevice *dev = gd->arch.ele_dev;
	int size = sizeof(struct ele_msg);
	struct ele_msg msg;
	int ret;

	if (!dev) {
		printf("ele dev is not initialized\n");
		return -ENODEV;
	}

	msg.version = ELE_VERSION;
	msg.tag = ELE_CMD_TAG;
	msg.size = 2;
	msg.command = ELE_RELEASE_RDC_REQ;
	switch (xrdc) {
	case 0:
		msg.data[0] = (0x74 << 8) | core_id;
		break;
	case 1:
		msg.data[0] = (0x78 << 8) | core_id;
		break;
	case 2:
		msg.data[0] = (0x82 << 8) | core_id;
		break;
	case 3:
		msg.data[0] = (0x86 << 8) | core_id;
		break;
	default:
		printf("Error: wrong xrdc index %u\n", xrdc);
		return -EINVAL;
	}

	ret = misc_call(dev, false, &msg, size, &msg, size);
	if (ret)
		printf("Error: %s: ret %d, core id %u, response 0x%x\n",
		       __func__, ret, core_id, msg.data[0]);

	if (response)
		*response = msg.data[0];

	return ret;
}

int ele_auth_oem_ctnr(ulong ctnr_addr, u32 *response)
{
	struct udevice *dev = gd->arch.ele_dev;
	int size = sizeof(struct ele_msg);
	struct ele_msg msg;
	int ret;

	if (!dev) {
		printf("ele dev is not initialized\n");
		return -ENODEV;
	}

	msg.version = ELE_VERSION;
	msg.tag = ELE_CMD_TAG;
	msg.size = 3;
	msg.command = ELE_OEM_CNTN_AUTH_REQ;
	msg.data[0] = upper_32_bits(ctnr_addr);
	msg.data[1] = lower_32_bits(ctnr_addr);

	ret = misc_call(dev, false, &msg, size, &msg, size);
	if (ret)
		printf("Error: %s: ret %d, cntr_addr 0x%lx, response 0x%x\n",
		       __func__, ret, ctnr_addr, msg.data[0]);

	if (response)
		*response = msg.data[0];

	return ret;
}

int ele_release_container(u32 *response)
{
	struct udevice *dev = gd->arch.ele_dev;
	int size = sizeof(struct ele_msg);
	struct ele_msg msg;
	int ret;

	if (!dev) {
		printf("ele dev is not initialized\n");
		return -ENODEV;
	}

	msg.version = ELE_VERSION;
	msg.tag = ELE_CMD_TAG;
	msg.size = 1;
	msg.command = ELE_RELEASE_CONTAINER_REQ;

	ret = misc_call(dev, false, &msg, size, &msg, size);
	if (ret)
		printf("Error: %s: ret %d, response 0x%x\n",
		       __func__, ret, msg.data[0]);

	if (response)
		*response = msg.data[0];

	return ret;
}

int ele_verify_image(u32 img_id, u32 *response)
{
	struct udevice *dev = gd->arch.ele_dev;
	int size = sizeof(struct ele_msg);
	struct ele_msg msg;
	int ret;

	if (!dev) {
		printf("ele dev is not initialized\n");
		return -ENODEV;
	}

	msg.version = ELE_VERSION;
	msg.tag = ELE_CMD_TAG;
	msg.size = 2;
	msg.command = ELE_VERIFY_IMAGE_REQ;
	msg.data[0] = 1 << img_id;

	ret = misc_call(dev, false, &msg, size, &msg, size);
	if (ret)
		printf("Error: %s: ret %d, img_id %u, response 0x%x\n",
		       __func__, ret, img_id, msg.data[0]);

	if (response)
		*response = msg.data[0];

	return ret;
}

int ele_forward_lifecycle(u16 life_cycle, u32 *response)
{
	struct udevice *dev = gd->arch.ele_dev;
	int size = sizeof(struct ele_msg);
	struct ele_msg msg;
	int ret;

	if (!dev) {
		printf("ele dev is not initialized\n");
		return -ENODEV;
	}

	msg.version = ELE_VERSION;
	msg.tag = ELE_CMD_TAG;
	msg.size = 2;
	msg.command = ELE_FWD_LIFECYCLE_UP_REQ;
	msg.data[0] = life_cycle;

	ret = misc_call(dev, false, &msg, size, &msg, size);
	if (ret)
		printf("Error: %s: ret %d, life_cycle 0x%x, response 0x%x\n",
		       __func__, ret, life_cycle, msg.data[0]);

	if (response)
		*response = msg.data[0];

	return ret;
}

int ele_read_common_fuse(u16 fuse_id, u32 *fuse_words, u32 fuse_num, u32 *response)
{
	struct udevice *dev = gd->arch.ele_dev;
	int size = sizeof(struct ele_msg);
	struct ele_msg msg;
	int ret;

	if (!dev) {
		printf("ele dev is not initialized\n");
		return -ENODEV;
	}

	if (!fuse_words) {
		printf("Invalid parameters for fuse read\n");
		return -EINVAL;
	}

	if ((fuse_id != 1 && fuse_num != 1) ||
	    (fuse_id == 1 && fuse_num != 4)) {
		printf("Invalid fuse number parameter\n");
		return -EINVAL;
	}

	msg.version = ELE_VERSION;
	msg.tag = ELE_CMD_TAG;
	msg.size = 2;
	msg.command = ELE_READ_FUSE_REQ;
	msg.data[0] = fuse_id;

	ret = misc_call(dev, false, &msg, size, &msg, size);
	if (ret)
		printf("Error: %s: ret %d, fuse_id 0x%x, response 0x%x\n",
		       __func__, ret, fuse_id, msg.data[0]);

	if (response)
		*response = msg.data[0];

	fuse_words[0] = msg.data[1];
	if (fuse_id == 1) {
		/* OTP_UNIQ_ID */
		fuse_words[1] = msg.data[2];
		fuse_words[2] = msg.data[3];
		fuse_words[3] = msg.data[4];
	}

	return ret;
}

int ele_write_fuse(u16 fuse_id, u32 fuse_val, bool lock, u32 *response)
{
	struct udevice *dev = gd->arch.ele_dev;
	int size = sizeof(struct ele_msg);
	struct ele_msg msg;
	int ret;

	if (!dev) {
		printf("ele dev is not initialized\n");
		return -ENODEV;
	}

	msg.version = ELE_VERSION;
	msg.tag = ELE_CMD_TAG;
	msg.size = 3;
	msg.command = ELE_WRITE_FUSE_REQ;
	msg.data[0] = (32 << 16) | (fuse_id << 5);
	if (lock)
		msg.data[0] |= (1 << 31);

	msg.data[1] = fuse_val;

	ret = misc_call(dev, false, &msg, size, &msg, size);
	if (ret)
		printf("Error: %s: ret %d, fuse_id 0x%x, response 0x%x\n",
		       __func__, ret, fuse_id, msg.data[0]);

	if (response)
		*response = msg.data[0];

	return ret;
}

int ele_release_caam(u32 core_did, u32 *response)
{
	struct udevice *dev = gd->arch.ele_dev;
	int size = sizeof(struct ele_msg);
	struct ele_msg msg;
	int ret;

	if (!dev) {
		printf("ele dev is not initialized\n");
		return -ENODEV;
	}

	msg.version = ELE_VERSION;
	msg.tag = ELE_CMD_TAG;
	msg.size = 2;
	msg.command = ELE_RELEASE_CAAM_REQ;
	msg.data[0] = core_did;

	ret = misc_call(dev, false, &msg, size, &msg, size);
	if (ret)
		printf("Error: %s: ret %d, response 0x%x\n",
		       __func__, ret, msg.data[0]);

	if (response)
		*response = msg.data[0];

	return ret;
}

int ele_get_fw_version(u32 *fw_version, u32 *sha1, u32 *response)
{
	struct udevice *dev = gd->arch.ele_dev;
	int size = sizeof(struct ele_msg);
	struct ele_msg msg;
	int ret;

	if (!dev) {
		printf("ele dev is not initialized\n");
		return -ENODEV;
	}

	if (!fw_version) {
		printf("Invalid parameters for f/w version read\n");
		return -EINVAL;
	}

	if (!sha1) {
		printf("Invalid parameters for commit sha1\n");
		return -EINVAL;
	}

	msg.version = ELE_VERSION;
	msg.tag = ELE_CMD_TAG;
	msg.size = 1;
	msg.command = ELE_GET_FW_VERSION_REQ;

	ret = misc_call(dev, false, &msg, size, &msg, size);
	if (ret)
		printf("Error: %s: ret %d, response 0x%x\n",
		       __func__, ret, msg.data[0]);

	if (response)
		*response = msg.data[0];

	*fw_version = msg.data[1];
	*sha1 = msg.data[2];

	return ret;
}

int ele_dump_buffer(u32 *buffer, u32 buffer_length)
{
	struct udevice *dev = gd->arch.ele_dev;
	int size = sizeof(struct ele_msg);
	struct ele_msg msg;
	int ret, i = 0;

	if (!dev) {
		printf("ele dev is not initialized\n");
		return -ENODEV;
	}

	msg.version = ELE_VERSION;
	msg.tag = ELE_CMD_TAG;
	msg.size = 1;
	msg.command = ELE_DUMP_DEBUG_BUFFER_REQ;

	ret = misc_call(dev, false, &msg, size, &msg, size);
	if (ret) {
		printf("Error: %s: ret %d, response 0x%x\n",
		       __func__, ret, msg.data[0]);

		return ret;
	}

	if (buffer) {
		buffer[i++] = *(u32 *)&msg; /* Need dump the response header */
		for (; i < buffer_length && i < msg.size; i++)
			buffer[i] = msg.data[i - 1];
	}

	return i;
}

int ele_get_info(struct ele_get_info_data *info, u32 *response)
{
	struct udevice *dev = gd->arch.ele_dev;
	int size = sizeof(struct ele_msg);
	struct ele_msg msg;
	int ret;

	if (!dev) {
		printf("ele dev is not initialized\n");
		return -ENODEV;
	}

	msg.version = ELE_VERSION;
	msg.tag = ELE_CMD_TAG;
	msg.size = 4;
	msg.command = ELE_GET_INFO_REQ;
	msg.data[0] = upper_32_bits((ulong)info);
	msg.data[1] = lower_32_bits((ulong)info);
	msg.data[2] = sizeof(struct ele_get_info_data);

	ret = misc_call(dev, false, &msg, size, &msg, size);
	if (ret)
		printf("Error: %s: ret %d, response 0x%x\n",
		       __func__, ret, msg.data[0]);

	if (response)
		*response = msg.data[0];

	return ret;
}

int ele_get_fw_status(u32 *status, u32 *response)
{
	struct udevice *dev = gd->arch.ele_dev;
	int size = sizeof(struct ele_msg);
	struct ele_msg msg;
	int ret;

	if (!dev) {
		printf("ele dev is not initialized\n");
		return -ENODEV;
	}

	msg.version = ELE_VERSION;
	msg.tag = ELE_CMD_TAG;
	msg.size = 1;
	msg.command = ELE_GET_FW_STATUS_REQ;

	ret = misc_call(dev, false, &msg, size, &msg, size);
	if (ret)
		printf("Error: %s: ret %d, response 0x%x\n",
		       __func__, ret, msg.data[0]);

	if (response)
		*response = msg.data[0];

	*status = msg.data[1] & 0xF;

	return ret;
}

int ele_release_m33_trout(void)
{
	struct udevice *dev = gd->arch.ele_dev;
	int size = sizeof(struct ele_msg);
	struct ele_msg msg;
	int ret;

	if (!dev) {
		printf("ele dev is not initialized\n");
		return -ENODEV;
	}

	msg.version = ELE_VERSION;
	msg.tag = ELE_CMD_TAG;
	msg.size = 1;
	msg.command = ELE_ENABLE_RTC_REQ;

	ret = misc_call(dev, false, &msg, size, &msg, size);
	if (ret)
		printf("Error: %s: ret %d, response 0x%x\n",
		       __func__, ret, msg.data[0]);

	return ret;
}

int ele_get_events(u32 *events, u32 *events_cnt, u32 *response)
{
	struct udevice *dev = gd->arch.ele_dev;
	int size = sizeof(struct ele_msg);
	struct ele_msg msg;
	int ret, i = 0;
	u32 actual_events;

	if (!dev) {
		printf("ele dev is not initialized\n");
		return -ENODEV;
	}

	if (!events || !events_cnt || *events_cnt == 0) {
		printf("Invalid parameters for %s\n", __func__);
		return -EINVAL;
	}

	msg.version = ELE_VERSION;
	msg.tag = ELE_CMD_TAG;
	msg.size = 1;
	msg.command = ELE_GET_EVENTS_REQ;

	ret = misc_call(dev, false, &msg, size, &msg, size);
	if (ret)
		printf("Error: %s: ret %d, response 0x%x\n",
		       __func__, ret, msg.data[0]);

	if (response)
		*response = msg.data[0];

	if (!ret) {
		actual_events = msg.data[1] & 0xffff;
		if (*events_cnt < actual_events)
			actual_events = *events_cnt;

		for (; i < actual_events; i++)
			events[i] = msg.data[i + 2];

		*events_cnt = actual_events;
	}

	return ret;
}

int ele_start_rng(void)
{
	struct udevice *dev = gd->arch.ele_dev;
	int size = sizeof(struct ele_msg);
	struct ele_msg msg;
	int ret;

	if (!dev) {
		printf("ele dev is not initialized\n");
		return -ENODEV;
	}

	msg.version = ELE_VERSION;
	msg.tag = ELE_CMD_TAG;
	msg.size = 1;
	msg.command = ELE_START_RNG;

	ret = misc_call(dev, false, &msg, size, &msg, size);
	if (ret)
		printf("Error: %s: ret %d, response 0x%x\n",
		       __func__, ret, msg.data[0]);

	return ret;
}

int ele_write_secure_fuse(ulong signed_msg_blk, u32 *response)
{
	struct udevice *dev = gd->arch.ele_dev;
	int size = sizeof(struct ele_msg);
	struct ele_msg msg;
	int ret;

	if (!dev) {
		printf("ele dev is not initialized\n");
		return -ENODEV;
	}

	msg.version = ELE_VERSION;
	msg.tag = ELE_CMD_TAG;
	msg.size = 3;
	msg.command = ELE_WRITE_SECURE_FUSE_REQ;

	msg.data[0] = upper_32_bits(signed_msg_blk);
	msg.data[1] = lower_32_bits(signed_msg_blk);

	ret = misc_call(dev, false, &msg, size, &msg, size);
	if (ret)
		printf("Error: %s: ret %d, response 0x%x, failed fuse row index %u\n",
		       __func__, ret, msg.data[0], msg.data[1]);

	if (response)
		*response = msg.data[0];

	return ret;
}

int ele_return_lifecycle_update(ulong signed_msg_blk, u32 *response)
{
	struct udevice *dev = gd->arch.ele_dev;
	int size = sizeof(struct ele_msg);
	struct ele_msg msg;
	int ret;

	if (!dev) {
		printf("ele dev is not initialized\n");
		return -ENODEV;
	}

	msg.version = ELE_VERSION;
	msg.tag = ELE_CMD_TAG;
	msg.size = 3;
	msg.command = ELE_RET_LIFECYCLE_UP_REQ;

	msg.data[0] = upper_32_bits(signed_msg_blk);
	msg.data[1] = lower_32_bits(signed_msg_blk);

	ret = misc_call(dev, false, &msg, size, &msg, size);
	if (ret)
		printf("Error: %s: ret %d, response 0x%x, failed fuse row index %u\n",
		       __func__, ret, msg.data[0], msg.data[1]);

	if (response)
		*response = msg.data[0];

	return ret;
}

int ele_generate_dek_blob(u32 key_id, u32 src_paddr, u32 dst_paddr, u32 max_output_size)
{
	struct udevice *dev = gd->arch.ele_dev;
	int size = sizeof(struct ele_msg);
	struct ele_msg msg;
	int ret;

	if (!dev) {
		printf("ele dev is not initialized\n");
		return -ENODEV;
	}

	msg.version = ELE_VERSION;
	msg.tag = ELE_CMD_TAG;
	msg.size = 8;
	msg.command = ELE_GENERATE_DEK_BLOB;
	msg.data[0] = key_id;
	msg.data[1] = 0x0;
	msg.data[2] = src_paddr;
	msg.data[3] = 0x0;
	msg.data[4] = dst_paddr;
	msg.data[5] = max_output_size;
	msg.data[6] = compute_crc(&msg);

	ret = misc_call(dev, false, &msg, size, &msg, size);
	if (ret)
		printf("Error: %s: ret 0x%x, response 0x%x\n",
		       __func__, ret, msg.data[0]);

	return ret;
}
