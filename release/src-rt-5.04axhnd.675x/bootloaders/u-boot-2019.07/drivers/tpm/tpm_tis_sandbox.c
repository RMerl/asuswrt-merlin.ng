// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2013 Google, Inc
 */

#include <common.h>
#include <dm.h>
#include <tpm-v1.h>
#include <asm/state.h>
#include <asm/unaligned.h>
#include <linux/crc8.h>

/* TPM NVRAM location indices. */
#define FIRMWARE_NV_INDEX		0x1007
#define KERNEL_NV_INDEX			0x1008
#define BACKUP_NV_INDEX                 0x1009
#define FWMP_NV_INDEX                   0x100a
#define REC_HASH_NV_INDEX               0x100b
#define REC_HASH_NV_SIZE                VB2_SHA256_DIGEST_SIZE

#define NV_DATA_PUBLIC_PERMISSIONS_OFFSET	60

/* Kernel TPM space - KERNEL_NV_INDEX, locked with physical presence */
#define ROLLBACK_SPACE_KERNEL_VERSION	2
#define ROLLBACK_SPACE_KERNEL_UID	0x4752574C  /* 'GRWL' */

struct rollback_space_kernel {
	/* Struct version, for backwards compatibility */
	uint8_t struct_version;
	/* Unique ID to detect space redefinition */
	uint32_t uid;
	/* Kernel versions */
	uint32_t kernel_versions;
	/* Reserved for future expansion */
	uint8_t reserved[3];
	/* Checksum (v2 and later only) */
	uint8_t crc8;
} __packed rollback_space_kernel;

/*
 * These numbers derive from adding the sizes of command fields as shown in
 * the TPM commands manual.
 */
#define TPM_REQUEST_HEADER_LENGTH	10
#define TPM_RESPONSE_HEADER_LENGTH	10

/* These are the different non-volatile spaces that we emulate */
enum {
	NV_GLOBAL_LOCK,
	NV_SEQ_FIRMWARE,
	NV_SEQ_KERNEL,
	NV_SEQ_BACKUP,
	NV_SEQ_FWMP,
	NV_SEQ_REC_HASH,

	NV_SEQ_COUNT,
};

/* Size of each non-volatile space */
#define NV_DATA_SIZE		0x20

struct nvdata_state {
	bool present;
	u8 data[NV_DATA_SIZE];
};

/*
 * Information about our TPM emulation. This is preserved in the sandbox
 * state file if enabled.
 */
static struct tpm_state {
	bool valid;
	struct nvdata_state nvdata[NV_SEQ_COUNT];
} g_state;

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
	const char *prop;
	int len;
	int i;

	if (!blob)
		return 0;

	for (i = 0; i < NV_SEQ_COUNT; i++) {
		char prop_name[20];

		sprintf(prop_name, "nvdata%d", i);
		prop = fdt_getprop(blob, node, prop_name, &len);
		if (prop && len == NV_DATA_SIZE) {
			memcpy(g_state.nvdata[i].data, prop, NV_DATA_SIZE);
			g_state.nvdata[i].present = true;
		}
	}
	g_state.valid = true;

	return 0;
}

/**
 * cros_ec_write_state() - Write out our state to the state file
 *
 * The caller will ensure that there is a node ready for the state. The node
 * may already contain the old state, in which case it is overridden.
 *
 * @blob: Device tree blob holding state
 * @node: Node to write our state into
 */
static int sandbox_tpm_write_state(void *blob, int node)
{
	int i;

	/*
	 * We are guaranteed enough space to write basic properties.
	 * We could use fdt_add_subnode() to put each set of data in its
	 * own node - perhaps useful if we add access informaiton to each.
	 */
	for (i = 0; i < NV_SEQ_COUNT; i++) {
		char prop_name[20];

		if (g_state.nvdata[i].present) {
			sprintf(prop_name, "nvdata%d", i);
			fdt_setprop(blob, node, prop_name,
				    g_state.nvdata[i].data, NV_DATA_SIZE);
		}
	}

	return 0;
}

SANDBOX_STATE_IO(sandbox_tpm, "google,sandbox-tpm", sandbox_tpm_read_state,
		 sandbox_tpm_write_state);

static int index_to_seq(uint32_t index)
{
	switch (index) {
	case FIRMWARE_NV_INDEX:
		return NV_SEQ_FIRMWARE;
	case KERNEL_NV_INDEX:
		return NV_SEQ_KERNEL;
	case BACKUP_NV_INDEX:
		return NV_SEQ_BACKUP;
	case FWMP_NV_INDEX:
		return NV_SEQ_FWMP;
	case REC_HASH_NV_INDEX:
		return NV_SEQ_REC_HASH;
	case 0:
		return NV_GLOBAL_LOCK;
	}

	printf("Invalid nv index %#x\n", index);
	return -1;
}

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
			data = recvbuf + TPM_RESPONSE_HEADER_LENGTH +
					sizeof(uint32_t);
			switch (index) {
			case FIRMWARE_NV_INDEX:
				break;
			case KERNEL_NV_INDEX:
				handle_cap_flag_space(&data, index);
				*recv_len = data - recvbuf -
					TPM_RESPONSE_HEADER_LENGTH -
					sizeof(uint32_t);
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
			put_unaligned_be32(*recv_len,
					   recvbuf +
					   TPM_RESPONSE_HEADER_LENGTH);
			break;
		case TPM_CAP_NV_INDEX:
			index = get_unaligned_be32(sendbuf + 18);
			printf("Get cap nv index %#02x\n", index);
			put_unaligned_be32(22, recvbuf +
					   TPM_RESPONSE_HEADER_LENGTH);
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
		seq = index_to_seq(index);
		if (seq < 0)
			return -EINVAL;
		printf("tpm: nvwrite index=%#02x, len=%#02x\n", index, length);
		memcpy(&tpm->nvdata[seq].data, sendbuf + 22, length);
		tpm->nvdata[seq].present = true;
		*recv_len = 12;
		memset(recvbuf, '\0', *recv_len);
		break;
	case TPM_CMD_NV_READ_VALUE: /* nvread */
		index = get_unaligned_be32(sendbuf + 10);
		length = get_unaligned_be32(sendbuf + 18);
		seq = index_to_seq(index);
		if (seq < 0)
			return -EINVAL;
		printf("tpm: nvread index=%#02x, len=%#02x, seq=%#02x\n", index,
		       length, seq);
		*recv_len = TPM_RESPONSE_HEADER_LENGTH + sizeof(uint32_t) +
					length;
		memset(recvbuf, '\0', *recv_len);
		put_unaligned_be32(length, recvbuf +
				   TPM_RESPONSE_HEADER_LENGTH);
		if (seq == NV_SEQ_KERNEL) {
			struct rollback_space_kernel rsk;

			data = recvbuf + TPM_RESPONSE_HEADER_LENGTH +
					sizeof(uint32_t);
			memset(&rsk, 0, sizeof(struct rollback_space_kernel));
			rsk.struct_version = 2;
			rsk.uid = ROLLBACK_SPACE_KERNEL_UID;
			rsk.crc8 = crc8(0, (unsigned char *)&rsk,
					offsetof(struct rollback_space_kernel,
						 crc8));
			memcpy(data, &rsk, sizeof(rsk));
		} else if (!tpm->nvdata[seq].present) {
			put_unaligned_be32(TPM_BADINDEX, recvbuf +
					   sizeof(uint16_t) + sizeof(uint32_t));
		} else {
			memcpy(recvbuf + TPM_RESPONSE_HEADER_LENGTH +
			       sizeof(uint32_t), &tpm->nvdata[seq].data,
			       length);
		}
		break;
	case TPM_CMD_EXTEND:
		*recv_len = 30;
		memset(recvbuf, '\0', *recv_len);
		break;
	case TPM_CMD_NV_DEFINE_SPACE:
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

	memcpy(tpm, &g_state, sizeof(*tpm));

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

U_BOOT_DRIVER(sandbox_tpm) = {
	.name   = "sandbox_tpm",
	.id     = UCLASS_TPM,
	.of_match = sandbox_tpm_ids,
	.ops    = &sandbox_tpm_ops,
	.probe	= sandbox_tpm_probe,
	.priv_auto_alloc_size = sizeof(struct tpm_state),
};
