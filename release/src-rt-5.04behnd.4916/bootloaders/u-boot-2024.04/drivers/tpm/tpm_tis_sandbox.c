// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2013 Google, Inc
 */

#include <common.h>
#include <display_options.h>
#include <dm.h>
#include <tpm-v1.h>
#include <asm/state.h>
#include <asm/unaligned.h>
#include <u-boot/crc.h>
#include "sandbox_common.h"

#define NV_DATA_PUBLIC_PERMISSIONS_OFFSET	60

/*
 * Information about our TPM emulation. This is preserved in the sandbox
 * state file if enabled.
 */
static struct tpm_state {
	bool valid;
	struct nvdata_state nvdata[NV_SEQ_COUNT];
} s_state, *g_state;

/**
 * sandbox_tpm_read_state() - read the sandbox EC state from the state file
 *
 * If data is available, then blob and node will provide access to it. If
 * not this function sets up an empty TPM.
 *
 * @blob: Pointer to device tree blob, or NULL if no data to read
 * @node: Node offset to read from
 */
static int sandbox_tpm_read_state(const void *blob, int node)
{
	struct tpm_state *state = &s_state;
	const char *prop;
	int len;
	int i;

	if (!blob)
		return 0;

	for (i = 0; i < NV_SEQ_COUNT; i++) {
		struct nvdata_state *nvd = &state->nvdata[i];
		char prop_name[20];

		sprintf(prop_name, "nvdata%d", i);
		prop = fdt_getprop(blob, node, prop_name, &len);
		if (len >= NV_DATA_SIZE)
			return log_msg_ret("nvd", -E2BIG);
		if (prop) {
			memcpy(nvd->data, prop, len);
			nvd->length = len;
			nvd->present = true;
		}
	}

	s_state.valid = true;

	return 0;
}

/**
 * sandbox_tpm_write_state() - Write out our state to the state file
 *
 * The caller will ensure that there is a node ready for the state. The node
 * may already contain the old state, in which case it is overridden.
 *
 * @blob: Device tree blob holding state
 * @node: Node to write our state into
 */
static int sandbox_tpm_write_state(void *blob, int node)
{
	const struct tpm_state *state = g_state;
	int i;

	if (!state)
		return 0;

	/*
	 * We are guaranteed enough space to write basic properties.
	 * We could use fdt_add_subnode() to put each set of data in its
	 * own node - perhaps useful if we add access informaiton to each.
	 */
	for (i = 0; i < NV_SEQ_COUNT; i++) {
		const struct nvdata_state *nvd = &state->nvdata[i];
		char prop_name[20];

		if (nvd->present) {
			snprintf(prop_name, sizeof(prop_name), "nvdata%d", i);
			fdt_setprop(blob, node, prop_name, nvd->data,
				    nvd->length);
		}
	}

	return 0;
}

SANDBOX_STATE_IO(sandbox_tpm, "google,sandbox-tpm", sandbox_tpm_read_state,
		 sandbox_tpm_write_state);

static void handle_cap_flag_space(u8 **datap, uint index)
{
	struct tpm_nv_data_public pub;

	/* TPM_NV_PER_PPWRITE */
	memset(&pub, '\0', sizeof(pub));
	pub.nv_index = __cpu_to_be32(index);
	pub.pcr_info_read.pcr_selection.size_of_select = __cpu_to_be16(
		sizeof(pub.pcr_info_read.pcr_selection.pcr_select));
	pub.permission.attributes = __cpu_to_be32(1);
	pub.pcr_info_write = pub.pcr_info_read;
	memcpy(*datap, &pub, sizeof(pub));
	*datap += sizeof(pub);
}

static int sandbox_tpm_xfer(struct udevice *dev, const uint8_t *sendbuf,
			    size_t send_size, uint8_t *recvbuf,
			    size_t *recv_len)
{
	struct tpm_state *tpm = dev_get_priv(dev);
	uint32_t code, index, length, type;
	uint8_t *data;
	int seq;

	code = get_unaligned_be32(sendbuf + sizeof(uint16_t) +
				  sizeof(uint32_t));
#ifdef DEBUG
	printf("tpm: %zd bytes, recv_len %zd, cmd = %x\n", send_size,
	       *recv_len, code);
	print_buffer(0, sendbuf, 1, send_size, 0);
#endif
	switch (code) {
	case TPM_CMD_GET_CAPABILITY:
		type = get_unaligned_be32(sendbuf + 14);
		switch (type) {
		case TPM_CAP_FLAG:
			index = get_unaligned_be32(sendbuf + 18);
			printf("Get flags index %#02x\n", index);
			*recv_len = 22;
			memset(recvbuf, '\0', *recv_len);
			data = recvbuf + TPM_HDR_LEN + sizeof(uint32_t);
			switch (index) {
			case FIRMWARE_NV_INDEX:
				break;
			case KERNEL_NV_INDEX:
				handle_cap_flag_space(&data, index);
				*recv_len = data - recvbuf;
				break;
			case TPM_CAP_FLAG_PERMANENT: {
				struct tpm_permanent_flags *pflags;

				pflags = (struct tpm_permanent_flags *)data;
				memset(pflags, '\0', sizeof(*pflags));
				put_unaligned_be32(TPM_TAG_PERMANENT_FLAGS,
						   &pflags->tag);
				*recv_len = TPM_HEADER_SIZE + 4 +
						sizeof(*pflags);
				break;
			}
			default:
				printf("   ** Unknown flags index %x\n", index);
				return -ENOSYS;
			}
			put_unaligned_be32(*recv_len, recvbuf + TPM_HDR_LEN);
			break;
		case TPM_CAP_NV_INDEX:
			index = get_unaligned_be32(sendbuf + 18);
			printf("Get cap nv index %#02x\n", index);
			put_unaligned_be32(22, recvbuf + TPM_HDR_LEN);
			break;
		default:
			printf("   ** Unknown 0x65 command type %#02x\n",
			       type);
			return -ENOSYS;
		}
		break;
	case TPM_CMD_NV_WRITE_VALUE:
		index = get_unaligned_be32(sendbuf + 10);
		length = get_unaligned_be32(sendbuf + 18);
		seq = sb_tpm_index_to_seq(index);
		if (seq < 0)
			return -EINVAL;
		printf("tpm: nvwrite index=%#02x, len=%#02x\n", index, length);
		sb_tpm_write_data(tpm->nvdata, seq, sendbuf, 22, length);
		break;
	case TPM_CMD_NV_READ_VALUE: /* nvread */
		index = get_unaligned_be32(sendbuf + 10);
		length = get_unaligned_be32(sendbuf + 18);
		seq = sb_tpm_index_to_seq(index);
		if (seq < 0)
			return -EINVAL;
		printf("tpm: nvread index=%#02x, len=%#02x, seq=%#02x\n", index,
		       length, seq);
		*recv_len = TPM_HDR_LEN + sizeof(uint32_t) + length;
		memset(recvbuf, '\0', *recv_len);
		put_unaligned_be32(length, recvbuf + TPM_HDR_LEN);
		sb_tpm_read_data(tpm->nvdata, seq, recvbuf, TPM_HDR_LEN + 4,
				 length);
		break;
	case TPM_CMD_EXTEND:
		*recv_len = 30;
		memset(recvbuf, '\0', *recv_len);
		break;
	case TPM_CMD_NV_DEFINE_SPACE:
		index = get_unaligned_be32(sendbuf + 12);
		length = get_unaligned_be32(sendbuf + 77);
		seq = sb_tpm_index_to_seq(index);
		if (seq < 0)
			return -EINVAL;
		printf("tpm: define_space index=%#02x, len=%#02x, seq=%#02x\n",
		       index, length, seq);
		sb_tpm_define_data(tpm->nvdata, seq, length);
		*recv_len = 12;
		memset(recvbuf, '\0', *recv_len);
		break;
	case 0x15: /* pcr read */
	case 0x5d: /* force clear */
	case 0x6f: /* physical enable */
	case 0x72: /* physical set deactivated */
	case 0x99: /* startup */
	case 0x50: /* self test full */
	case 0x4000000a:  /* assert physical presence */
		*recv_len = 12;
		memset(recvbuf, '\0', *recv_len);
		break;
	default:
		printf("Unknown tpm command %02x\n", code);
		return -ENOSYS;
	}
#ifdef DEBUG
	printf("tpm: rx recv_len %zd\n", *recv_len);
	print_buffer(0, recvbuf, 1, *recv_len, 0);
#endif

	return 0;
}

static int sandbox_tpm_get_desc(struct udevice *dev, char *buf, int size)
{
	if (size < 15)
		return -ENOSPC;

	return snprintf(buf, size, "sandbox TPM");
}

static int sandbox_tpm_probe(struct udevice *dev)
{
	struct tpm_state *tpm = dev_get_priv(dev);

	if (s_state.valid)
		memcpy(tpm, &s_state, sizeof(*tpm));
	g_state = tpm;

	return 0;
}

static int sandbox_tpm_open(struct udevice *dev)
{
	return 0;
}

static int sandbox_tpm_close(struct udevice *dev)
{
	return 0;
}

static const struct tpm_ops sandbox_tpm_ops = {
	.open		= sandbox_tpm_open,
	.close		= sandbox_tpm_close,
	.get_desc	= sandbox_tpm_get_desc,
	.xfer		= sandbox_tpm_xfer,
};

static const struct udevice_id sandbox_tpm_ids[] = {
	{ .compatible = "google,sandbox-tpm" },
	{ }
};

U_BOOT_DRIVER(google_sandbox_tpm) = {
	.name   = "google_sandbox_tpm",
	.id     = UCLASS_TPM,
	.of_match = sandbox_tpm_ids,
	.ops    = &sandbox_tpm_ops,
	.probe	= sandbox_tpm_probe,
	.priv_auto	= sizeof(struct tpm_state),
};
