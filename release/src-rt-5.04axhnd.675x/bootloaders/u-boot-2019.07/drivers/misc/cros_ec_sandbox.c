// SPDX-License-Identifier: GPL-2.0+
/*
 * Chromium OS cros_ec driver - sandbox emulation
 *
 * Copyright (c) 2013 The Chromium OS Authors.
 */

#include <common.h>
#include <cros_ec.h>
#include <dm.h>
#include <ec_commands.h>
#include <errno.h>
#include <hash.h>
#include <malloc.h>
#include <os.h>
#include <u-boot/sha256.h>
#include <spi.h>
#include <asm/state.h>
#include <asm/sdl.h>
#include <linux/input.h>

/*
 * Ultimately it shold be possible to connect an Chrome OS EC emulation
 * to U-Boot and remove all of this code. But this provides a test
 * environment for bringing up chromeos_sandbox and demonstrating its
 * utility.
 *
 * This emulation includes the following:
 *
 * 1. Emulation of the keyboard, by converting keypresses received from SDL
 * into key scan data, passed back from the EC as key scan messages. The
 * key layout is read from the device tree.
 *
 * 2. Emulation of vboot context - so this can be read/written as required.
 *
 * 3. Save/restore of EC state, so that the vboot context, flash memory
 * contents and current image can be preserved across boots. This is important
 * since the EC is supposed to continue running even if the AP resets.
 *
 * 4. Some event support, in particular allowing Escape to be pressed on boot
 * to enter recovery mode. The EC passes this to U-Boot through the normal
 * event message.
 *
 * 5. Flash read/write/erase support, so that software sync works. The
 * protect messages are supported but no protection is implemented.
 *
 * 6. Hashing of the EC image, again to support software sync.
 *
 * Other features can be added, although a better path is probably to link
 * the EC image in with U-Boot (Vic has demonstrated a prototype for this).
 */

#define KEYBOARD_ROWS	8
#define KEYBOARD_COLS	13

/* A single entry of the key matrix */
struct ec_keymatrix_entry {
	int row;	/* key matrix row */
	int col;	/* key matrix column */
	int keycode;	/* corresponding linux key code */
};

/**
 * struct ec_state - Information about the EC state
 *
 * @vbnv_context: Vboot context data stored by EC
 * @ec_config: FDT config information about the EC (e.g. flashmap)
 * @flash_data: Contents of flash memory
 * @flash_data_len: Size of flash memory
 * @current_image: Current image the EC is running
 * @matrix_count: Number of keys to decode in matrix
 * @matrix: Information about keyboard matrix
 * @keyscan: Current keyscan information (bit set for each row/column pressed)
 * @recovery_req: Keyboard recovery requested
 */
struct ec_state {
	u8 vbnv_context[EC_VBNV_BLOCK_SIZE_V2];
	struct fdt_cros_ec ec_config;
	uint8_t *flash_data;
	int flash_data_len;
	enum ec_current_image current_image;
	int matrix_count;
	struct ec_keymatrix_entry *matrix;	/* the key matrix info */
	uint8_t keyscan[KEYBOARD_COLS];
	bool recovery_req;
} s_state, *g_state;

/**
 * cros_ec_read_state() - read the sandbox EC state from the state file
 *
 * If data is available, then blob and node will provide access to it. If
 * not this function sets up an empty EC.
 *
 * @param blob: Pointer to device tree blob, or NULL if no data to read
 * @param node: Node offset to read from
 */
static int cros_ec_read_state(const void *blob, int node)
{
	struct ec_state *ec = &s_state;
	const char *prop;
	int len;

	/* Set everything to defaults */
	ec->current_image = EC_IMAGE_RO;
	if (!blob)
		return 0;

	/* Read the data if available */
	ec->current_image = fdtdec_get_int(blob, node, "current-image",
					   EC_IMAGE_RO);
	prop = fdt_getprop(blob, node, "vbnv-context", &len);
	if (prop && len == sizeof(ec->vbnv_context))
		memcpy(ec->vbnv_context, prop, len);

	prop = fdt_getprop(blob, node, "flash-data", &len);
	if (prop) {
		ec->flash_data_len = len;
		ec->flash_data = os_malloc(len);
		if (!ec->flash_data)
			return -ENOMEM;
		memcpy(ec->flash_data, prop, len);
		debug("%s: Loaded EC flash data size %#x\n", __func__, len);
	}

	return 0;
}

/**
 * cros_ec_write_state() - Write out our state to the state file
 *
 * The caller will ensure that there is a node ready for the state. The node
 * may already contain the old state, in which case it is overridden.
 *
 * @param blob: Device tree blob holding state
 * @param node: Node to write our state into
 */
static int cros_ec_write_state(void *blob, int node)
{
	struct ec_state *ec = g_state;

	/* We are guaranteed enough space to write basic properties */
	fdt_setprop_u32(blob, node, "current-image", ec->current_image);
	fdt_setprop(blob, node, "vbnv-context", ec->vbnv_context,
		    sizeof(ec->vbnv_context));
	return state_setprop(node, "flash-data", ec->flash_data,
			     ec->ec_config.flash.length);
}

SANDBOX_STATE_IO(cros_ec, "google,cros-ec", cros_ec_read_state,
		 cros_ec_write_state);

/**
 * Return the number of bytes used in the specified image.
 *
 * This is the actual size of code+data in the image, as opposed to the
 * amount of space reserved in flash for that image. This code is similar to
 * that used by the real EC code base.
 *
 * @param ec	Current emulated EC state
 * @param entry	Flash map entry containing the image to check
 * @return actual image size in bytes, 0 if the image contains no content or
 * error.
 */
static int get_image_used(struct ec_state *ec, struct fmap_entry *entry)
{
	int size;

	/*
	 * Scan backwards looking for 0xea byte, which is by definition the
	 * last byte of the image.  See ec.lds.S for how this is inserted at
	 * the end of the image.
	 */
	for (size = entry->length - 1;
	     size > 0 && ec->flash_data[entry->offset + size] != 0xea;
	     size--)
		;

	return size ? size + 1 : 0;  /* 0xea byte IS part of the image */
}

/**
 * Read the key matrix from the device tree
 *
 * Keymap entries in the fdt take the form of 0xRRCCKKKK where
 * RR=Row CC=Column KKKK=Key Code
 *
 * @param ec	Current emulated EC state
 * @param node	Keyboard node of device tree containing keyscan information
 * @return 0 if ok, -1 on error
 */
static int keyscan_read_fdt_matrix(struct ec_state *ec, ofnode node)
{
	const u32 *cell;
	int upto;
	int len;

	cell = ofnode_get_property(node, "linux,keymap", &len);
	ec->matrix_count = len / 4;
	ec->matrix = calloc(ec->matrix_count, sizeof(*ec->matrix));
	if (!ec->matrix) {
		debug("%s: Out of memory for key matrix\n", __func__);
		return -1;
	}

	/* Now read the data */
	for (upto = 0; upto < ec->matrix_count; upto++) {
		struct ec_keymatrix_entry *matrix = &ec->matrix[upto];
		u32 word;

		word = fdt32_to_cpu(*cell++);
		matrix->row = word >> 24;
		matrix->col = (word >> 16) & 0xff;
		matrix->keycode = word & 0xffff;

		/* Hard-code some sanity limits for now */
		if (matrix->row >= KEYBOARD_ROWS ||
		    matrix->col >= KEYBOARD_COLS) {
			debug("%s: Matrix pos out of range (%d,%d)\n",
			      __func__, matrix->row, matrix->col);
			return -1;
		}
	}

	if (upto != ec->matrix_count) {
		debug("%s: Read mismatch from key matrix\n", __func__);
		return -1;
	}

	return 0;
}

/**
 * Return the next keyscan message contents
 *
 * @param ec	Current emulated EC state
 * @param scan	Place to put keyscan bytes for the keyscan message (must hold
 *		enough space for a full keyscan)
 * @return number of bytes of valid scan data
 */
static int cros_ec_keyscan(struct ec_state *ec, uint8_t *scan)
{
	const struct ec_keymatrix_entry *matrix;
	int bytes = KEYBOARD_COLS;
	int key[8];	/* allow up to 8 keys to be pressed at once */
	int count;
	int i;

	memset(ec->keyscan, '\0', bytes);
	count = sandbox_sdl_scan_keys(key, ARRAY_SIZE(key));

	/* Look up keycode in matrix */
	for (i = 0, matrix = ec->matrix; i < ec->matrix_count; i++, matrix++) {
		bool found;
		int j;

		for (found = false, j = 0; j < count; j++) {
			if (matrix->keycode == key[j])
				found = true;
		}

		if (found) {
			debug("%d: %d,%d\n", matrix->keycode, matrix->row,
			      matrix->col);
			ec->keyscan[matrix->col] |= 1 << matrix->row;
		}
	}

	memcpy(scan, ec->keyscan, bytes);
	return bytes;
}

/**
 * Process an emulated EC command
 *
 * @param ec		Current emulated EC state
 * @param req_hdr	Pointer to request header
 * @param req_data	Pointer to body of request
 * @param resp_hdr	Pointer to place to put response header
 * @param resp_data	Pointer to place to put response data, if any
 * @return length of response data, or 0 for no response data, or -1 on error
 */
static int process_cmd(struct ec_state *ec,
		       struct ec_host_request *req_hdr, const void *req_data,
		       struct ec_host_response *resp_hdr, void *resp_data)
{
	int len;

	/* TODO(sjg@chromium.org): Check checksums */
	debug("EC command %#0x\n", req_hdr->command);

	switch (req_hdr->command) {
	case EC_CMD_HELLO: {
		const struct ec_params_hello *req = req_data;
		struct ec_response_hello *resp = resp_data;

		resp->out_data = req->in_data + 0x01020304;
		len = sizeof(*resp);
		break;
	}
	case EC_CMD_GET_VERSION: {
		struct ec_response_get_version *resp = resp_data;

		strcpy(resp->version_string_ro, "sandbox_ro");
		strcpy(resp->version_string_rw, "sandbox_rw");
		resp->current_image = ec->current_image;
		debug("Current image %d\n", resp->current_image);
		len = sizeof(*resp);
		break;
	}
	case EC_CMD_VBNV_CONTEXT: {
		const struct ec_params_vbnvcontext *req = req_data;
		struct ec_response_vbnvcontext *resp = resp_data;

		switch (req->op) {
		case EC_VBNV_CONTEXT_OP_READ:
			/* TODO(sjg@chromium.org): Support full-size context */
			memcpy(resp->block, ec->vbnv_context,
			       EC_VBNV_BLOCK_SIZE);
			len = 16;
			break;
		case EC_VBNV_CONTEXT_OP_WRITE:
			/* TODO(sjg@chromium.org): Support full-size context */
			memcpy(ec->vbnv_context, req->block,
			       EC_VBNV_BLOCK_SIZE);
			len = 0;
			break;
		default:
			printf("   ** Unknown vbnv_context command %#02x\n",
			       req->op);
			return -1;
		}
		break;
	}
	case EC_CMD_REBOOT_EC: {
		const struct ec_params_reboot_ec *req = req_data;

		printf("Request reboot type %d\n", req->cmd);
		switch (req->cmd) {
		case EC_REBOOT_DISABLE_JUMP:
			len = 0;
			break;
		case EC_REBOOT_JUMP_RW:
			ec->current_image = EC_IMAGE_RW;
			len = 0;
			break;
		default:
			puts("   ** Unknown type");
			return -1;
		}
		break;
	}
	case EC_CMD_HOST_EVENT_GET_B: {
		struct ec_response_host_event_mask *resp = resp_data;

		resp->mask = 0;
		if (ec->recovery_req) {
			resp->mask |= EC_HOST_EVENT_MASK(
					EC_HOST_EVENT_KEYBOARD_RECOVERY);
		}

		len = sizeof(*resp);
		break;
	}
	case EC_CMD_VBOOT_HASH: {
		const struct ec_params_vboot_hash *req = req_data;
		struct ec_response_vboot_hash *resp = resp_data;
		struct fmap_entry *entry;
		int ret, size;

		entry = &ec->ec_config.region[EC_FLASH_REGION_ACTIVE];

		switch (req->cmd) {
		case EC_VBOOT_HASH_RECALC:
		case EC_VBOOT_HASH_GET:
			size = SHA256_SUM_LEN;
			len = get_image_used(ec, entry);
			ret = hash_block("sha256",
					 ec->flash_data + entry->offset,
					 len, resp->hash_digest, &size);
			if (ret) {
				printf("   ** hash_block() failed\n");
				return -1;
			}
			resp->status = EC_VBOOT_HASH_STATUS_DONE;
			resp->hash_type = EC_VBOOT_HASH_TYPE_SHA256;
			resp->digest_size = size;
			resp->reserved0 = 0;
			resp->offset = entry->offset;
			resp->size = len;
			len = sizeof(*resp);
			break;
		default:
			printf("   ** EC_CMD_VBOOT_HASH: Unknown command %d\n",
			       req->cmd);
			return -1;
		}
		break;
	}
	case EC_CMD_FLASH_PROTECT: {
		const struct ec_params_flash_protect *req = req_data;
		struct ec_response_flash_protect *resp = resp_data;
		uint32_t expect = EC_FLASH_PROTECT_ALL_NOW |
				EC_FLASH_PROTECT_ALL_AT_BOOT;

		printf("mask=%#x, flags=%#x\n", req->mask, req->flags);
		if (req->flags == expect || req->flags == 0) {
			resp->flags = req->flags ? EC_FLASH_PROTECT_ALL_NOW :
								0;
			resp->valid_flags = EC_FLASH_PROTECT_ALL_NOW;
			resp->writable_flags = 0;
			len = sizeof(*resp);
		} else {
			puts("   ** unexpected flash protect request\n");
			return -1;
		}
		break;
	}
	case EC_CMD_FLASH_REGION_INFO: {
		const struct ec_params_flash_region_info *req = req_data;
		struct ec_response_flash_region_info *resp = resp_data;
		struct fmap_entry *entry;

		switch (req->region) {
		case EC_FLASH_REGION_RO:
		case EC_FLASH_REGION_ACTIVE:
		case EC_FLASH_REGION_WP_RO:
			entry = &ec->ec_config.region[req->region];
			resp->offset = entry->offset;
			resp->size = entry->length;
			len = sizeof(*resp);
			printf("EC flash region %d: offset=%#x, size=%#x\n",
			       req->region, resp->offset, resp->size);
			break;
		default:
			printf("** Unknown flash region %d\n", req->region);
			return -1;
		}
		break;
	}
	case EC_CMD_FLASH_ERASE: {
		const struct ec_params_flash_erase *req = req_data;

		memset(ec->flash_data + req->offset,
		       ec->ec_config.flash_erase_value,
		       req->size);
		len = 0;
		break;
	}
	case EC_CMD_FLASH_WRITE: {
		const struct ec_params_flash_write *req = req_data;

		memcpy(ec->flash_data + req->offset, req + 1, req->size);
		len = 0;
		break;
	}
	case EC_CMD_MKBP_STATE:
		len = cros_ec_keyscan(ec, resp_data);
		break;
	case EC_CMD_ENTERING_MODE:
		len = 0;
		break;
	default:
		printf("   ** Unknown EC command %#02x\n", req_hdr->command);
		return -1;
	}

	return len;
}

int cros_ec_sandbox_packet(struct udevice *udev, int out_bytes, int in_bytes)
{
	struct cros_ec_dev *dev = dev_get_uclass_priv(udev);
	struct ec_state *ec = dev_get_priv(dev->dev);
	struct ec_host_request *req_hdr = (struct ec_host_request *)dev->dout;
	const void *req_data = req_hdr + 1;
	struct ec_host_response *resp_hdr = (struct ec_host_response *)dev->din;
	void *resp_data = resp_hdr + 1;
	int len;

	len = process_cmd(ec, req_hdr, req_data, resp_hdr, resp_data);
	if (len < 0)
		return len;

	resp_hdr->struct_version = 3;
	resp_hdr->result = EC_RES_SUCCESS;
	resp_hdr->data_len = len;
	resp_hdr->reserved = 0;
	len += sizeof(*resp_hdr);
	resp_hdr->checksum = 0;
	resp_hdr->checksum = (uint8_t)
		-cros_ec_calc_checksum((const uint8_t *)resp_hdr, len);

	return in_bytes;
}

void cros_ec_check_keyboard(struct udevice *dev)
{
	struct ec_state *ec = dev_get_priv(dev);
	ulong start;

	printf("Press keys for EC to detect on reset (ESC=recovery)...");
	start = get_timer(0);
	while (get_timer(start) < 1000)
		;
	putc('\n');
	if (!sandbox_sdl_key_pressed(KEY_ESC)) {
		ec->recovery_req = true;
		printf("   - EC requests recovery\n");
	}
}

int cros_ec_probe(struct udevice *dev)
{
	struct ec_state *ec = dev->priv;
	struct cros_ec_dev *cdev = dev->uclass_priv;
	struct udevice *keyb_dev;
	ofnode node;
	int err;

	memcpy(ec, &s_state, sizeof(*ec));
	err = cros_ec_decode_ec_flash(dev, &ec->ec_config);
	if (err) {
		debug("%s: Cannot device EC flash\n", __func__);
		return err;
	}

	node = ofnode_null();
	for (device_find_first_child(dev, &keyb_dev);
	     keyb_dev;
	     device_find_next_child(&keyb_dev)) {
		if (device_get_uclass_id(keyb_dev) == UCLASS_KEYBOARD) {
			node = dev_ofnode(keyb_dev);
			break;
		}
	}
	if (!ofnode_valid(node)) {
		debug("%s: No cros_ec keyboard found\n", __func__);
	} else if (keyscan_read_fdt_matrix(ec, node)) {
		debug("%s: Could not read key matrix\n", __func__);
		return -1;
	}

	/* If we loaded EC data, check that the length matches */
	if (ec->flash_data &&
	    ec->flash_data_len != ec->ec_config.flash.length) {
		printf("EC data length is %x, expected %x, discarding data\n",
		       ec->flash_data_len, ec->ec_config.flash.length);
		os_free(ec->flash_data);
		ec->flash_data = NULL;
	}

	/* Otherwise allocate the memory */
	if (!ec->flash_data) {
		ec->flash_data_len = ec->ec_config.flash.length;
		ec->flash_data = os_malloc(ec->flash_data_len);
		if (!ec->flash_data)
			return -ENOMEM;
	}

	cdev->dev = dev;
	g_state = ec;
	return cros_ec_register(dev);
}

struct dm_cros_ec_ops cros_ec_ops = {
	.packet = cros_ec_sandbox_packet,
};

static const struct udevice_id cros_ec_ids[] = {
	{ .compatible = "google,cros-ec-sandbox" },
	{ }
};

U_BOOT_DRIVER(cros_ec_sandbox) = {
	.name		= "cros_ec_sandbox",
	.id		= UCLASS_CROS_EC,
	.of_match	= cros_ec_ids,
	.probe		= cros_ec_probe,
	.priv_auto_alloc_size = sizeof(struct ec_state),
	.ops		= &cros_ec_ops,
};
