// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2018, Bootlin
 * Author: Miquel Raynal <miquel.raynal@bootlin.com>
 */

#include <common.h>
#include <dm.h>
#include <tpm-v2.h>
#include <asm/state.h>
#include <asm/unaligned.h>
#include <linux/crc8.h>

/* Hierarchies */
enum tpm2_hierarchy {
	TPM2_HIERARCHY_LOCKOUT = 0,
	TPM2_HIERARCHY_ENDORSEMENT,
	TPM2_HIERARCHY_PLATFORM,
	TPM2_HIERARCHY_NB,
};

/* Subset of supported capabilities */
enum tpm2_capability {
	TPM_CAP_TPM_PROPERTIES = 0x6,
};

/* Subset of supported properties */
#define TPM2_PROPERTIES_OFFSET 0x0000020E

enum tpm2_cap_tpm_property {
	TPM2_FAIL_COUNTER = 0,
	TPM2_PROP_MAX_TRIES,
	TPM2_RECOVERY_TIME,
	TPM2_LOCKOUT_RECOVERY,
	TPM2_PROPERTY_NB,
};

#define SANDBOX_TPM_PCR_NB 1

static const u8 sandbox_extended_once_pcr[] = {
	0xf5, 0xa5, 0xfd, 0x42, 0xd1, 0x6a, 0x20, 0x30,
	0x27, 0x98, 0xef, 0x6e, 0xd3, 0x09, 0x97, 0x9b,
	0x43, 0x00, 0x3d, 0x23, 0x20, 0xd9, 0xf0, 0xe8,
	0xea, 0x98, 0x31, 0xa9, 0x27, 0x59, 0xfb, 0x4b,
};

struct sandbox_tpm2 {
	/* TPM internal states */
	bool init_done;
	bool startup_done;
	bool tests_done;
	/* TPM password per hierarchy */
	char pw[TPM2_HIERARCHY_NB][TPM2_DIGEST_LEN + 1];
	int pw_sz[TPM2_HIERARCHY_NB];
	/* TPM properties */
	u32 properties[TPM2_PROPERTY_NB];
	/* TPM PCRs */
	u8 pcr[SANDBOX_TPM_PCR_NB][TPM2_DIGEST_LEN];
	/* TPM PCR extensions */
	u32 pcr_extensions[SANDBOX_TPM_PCR_NB];
};

/*
 * Check the tag validity depending on the command (authentication required or
 * not). If authentication is required, check it is valid. Update the auth
 * pointer to point to the next chunk of data to process if needed.
 */
static int sandbox_tpm2_check_session(struct udevice *dev, u32 command, u16 tag,
				      const u8 **auth,
				      enum tpm2_hierarchy *hierarchy)
{
	struct sandbox_tpm2 *tpm = dev_get_priv(dev);
	u32 handle, auth_sz, session_handle;
	u16 nonce_sz, pw_sz;
	const char *pw;

	switch (command) {
	case TPM2_CC_STARTUP:
	case TPM2_CC_SELF_TEST:
	case TPM2_CC_GET_CAPABILITY:
	case TPM2_CC_PCR_READ:
		if (tag != TPM2_ST_NO_SESSIONS) {
			printf("No session required for command 0x%x\n",
			       command);
			return TPM2_RC_BAD_TAG;
		}

		return 0;

	case TPM2_CC_CLEAR:
	case TPM2_CC_HIERCHANGEAUTH:
	case TPM2_CC_DAM_RESET:
	case TPM2_CC_DAM_PARAMETERS:
	case TPM2_CC_PCR_EXTEND:
		if (tag != TPM2_ST_SESSIONS) {
			printf("Session required for command 0x%x\n", command);
			return TPM2_RC_AUTH_CONTEXT;
		}

		handle = get_unaligned_be32(*auth);
		*auth += sizeof(handle);

		/*
		 * PCR_Extend had a different protection mechanism and does not
		 * use the same standards as other commands.
		 */
		if (command == TPM2_CC_PCR_EXTEND)
			break;

		switch (handle) {
		case TPM2_RH_LOCKOUT:
			*hierarchy = TPM2_HIERARCHY_LOCKOUT;
			break;
		case TPM2_RH_ENDORSEMENT:
			if (command == TPM2_CC_CLEAR) {
				printf("Endorsement hierarchy unsupported\n");
				return TPM2_RC_AUTH_MISSING;
			}
			*hierarchy = TPM2_HIERARCHY_ENDORSEMENT;
			break;
		case TPM2_RH_PLATFORM:
			*hierarchy = TPM2_HIERARCHY_PLATFORM;
			break;
		default:
			printf("Wrong handle 0x%x\n", handle);
			return TPM2_RC_VALUE;
		}

		break;

	default:
		printf("Command code not recognized: 0x%x\n", command);
		return TPM2_RC_COMMAND_CODE;
	}

	auth_sz = get_unaligned_be32(*auth);
	*auth += sizeof(auth_sz);

	session_handle = get_unaligned_be32(*auth);
	*auth += sizeof(session_handle);
	if (session_handle != TPM2_RS_PW) {
		printf("Wrong session handle 0x%x\n", session_handle);
		return TPM2_RC_VALUE;
	}

	nonce_sz = get_unaligned_be16(*auth);
	*auth += sizeof(nonce_sz);
	if (nonce_sz) {
		printf("Nonces not supported in Sandbox, aborting\n");
		return TPM2_RC_HANDLE;
	}

	/* Ignore attributes */
	*auth += sizeof(u8);

	pw_sz = get_unaligned_be16(*auth);
	*auth += sizeof(pw_sz);
	if (auth_sz != (9 + nonce_sz + pw_sz)) {
		printf("Authentication size (%d) do not match %d\n",
		       auth_sz, 9 + nonce_sz + pw_sz);
		return TPM2_RC_SIZE;
	}

	/* No passwork is acceptable */
	if (!pw_sz && !tpm->pw_sz[*hierarchy])
		return TPM2_RC_SUCCESS;

	/* Password is too long */
	if (pw_sz > TPM2_DIGEST_LEN) {
		printf("Password should not be more than %dB\n",
		       TPM2_DIGEST_LEN);
		return TPM2_RC_AUTHSIZE;
	}

	pw = (const char *)*auth;
	*auth += pw_sz;

	/* Password is wrong */
	if (pw_sz != tpm->pw_sz[*hierarchy] ||
	    strncmp(pw, tpm->pw[*hierarchy], tpm->pw_sz[*hierarchy])) {
		printf("Authentication failed: wrong password.\n");
		return TPM2_RC_BAD_AUTH;
	}

	return TPM2_RC_SUCCESS;
}

static int sandbox_tpm2_check_readyness(struct udevice *dev, int command)
{
	struct sandbox_tpm2 *tpm = dev_get_priv(dev);

	switch (command) {
	case TPM2_CC_STARTUP:
		if (!tpm->init_done || tpm->startup_done)
			return TPM2_RC_INITIALIZE;

		break;
	case TPM2_CC_GET_CAPABILITY:
		if (!tpm->init_done || !tpm->startup_done)
			return TPM2_RC_INITIALIZE;

		break;
	case TPM2_CC_SELF_TEST:
		if (!tpm->startup_done)
			return TPM2_RC_INITIALIZE;

		break;
	default:
		if (!tpm->tests_done)
			return TPM2_RC_NEEDS_TEST;

		break;
	}

	return 0;
}

static int sandbox_tpm2_fill_buf(u8 *recv, size_t *recv_len, u16 tag, u32 rc)
{
	*recv_len = sizeof(tag) + sizeof(u32) + sizeof(rc);

	/* Write tag */
	put_unaligned_be16(tag, recv);
	recv += sizeof(tag);

	/* Write length */
	put_unaligned_be32(*recv_len, recv);
	recv += sizeof(u32);

	/* Write return code */
	put_unaligned_be32(rc, recv);
	recv += sizeof(rc);

	/* Add trailing \0 */
	*recv = '\0';

	return 0;
}

static int sandbox_tpm2_extend(struct udevice *dev, int pcr_index,
			       const u8 *extension)
{
	struct sandbox_tpm2 *tpm = dev_get_priv(dev);
	int i;

	/* Only simulate the first extensions from all '0' with only '0' */
	for (i = 0; i < TPM2_DIGEST_LEN; i++)
		if (tpm->pcr[pcr_index][i] || extension[i])
			return TPM2_RC_FAILURE;

	memcpy(tpm->pcr[pcr_index], sandbox_extended_once_pcr,
	       TPM2_DIGEST_LEN);
	tpm->pcr_extensions[pcr_index]++;

	return 0;
};

static int sandbox_tpm2_xfer(struct udevice *dev, const u8 *sendbuf,
			     size_t send_size, u8 *recvbuf,
			     size_t *recv_len)
{
	struct sandbox_tpm2 *tpm = dev_get_priv(dev);
	enum tpm2_hierarchy hierarchy = 0;
	const u8 *sent = sendbuf;
	u8 *recv = recvbuf;
	u32 length, command, rc = 0;
	u16 tag, mode, new_pw_sz;
	u8 yes_no;
	int i, j;

	/* TPM2_GetProperty */
	u32 capability, property, property_count;

	/* TPM2_PCR_Read/Extend variables */
	int pcr_index = 0;
	u64 pcr_map = 0;
	u32 selections, pcr_nb;
	u16 alg;
	u8 pcr_array_sz;

	tag = get_unaligned_be16(sent);
	sent += sizeof(tag);

	length = get_unaligned_be32(sent);
	sent += sizeof(length);
	if (length != send_size) {
		printf("TPM2: Unmatching length, received: %ld, expected: %d\n",
		       send_size, length);
		rc = TPM2_RC_SIZE;
		sandbox_tpm2_fill_buf(recv, recv_len, tag, rc);
		return 0;
	}

	command = get_unaligned_be32(sent);
	sent += sizeof(command);
	rc = sandbox_tpm2_check_readyness(dev, command);
	if (rc) {
		sandbox_tpm2_fill_buf(recv, recv_len, tag, rc);
		return 0;
	}

	rc = sandbox_tpm2_check_session(dev, command, tag, &sent, &hierarchy);
	if (rc) {
		sandbox_tpm2_fill_buf(recv, recv_len, tag, rc);
		return 0;
	}

	switch (command) {
	case TPM2_CC_STARTUP:
		mode = get_unaligned_be16(sent);
		sent += sizeof(mode);
		switch (mode) {
		case TPM2_SU_CLEAR:
		case TPM2_SU_STATE:
			break;
		default:
			rc = TPM2_RC_VALUE;
		}

		tpm->startup_done = true;

		sandbox_tpm2_fill_buf(recv, recv_len, tag, rc);
		break;

	case TPM2_CC_SELF_TEST:
		yes_no = *sent;
		sent += sizeof(yes_no);
		switch (yes_no) {
		case TPMI_YES:
		case TPMI_NO:
			break;
		default:
			rc = TPM2_RC_VALUE;
		}

		tpm->tests_done = true;

		sandbox_tpm2_fill_buf(recv, recv_len, tag, rc);
		break;

	case TPM2_CC_CLEAR:
		/* Reset this hierarchy password */
		tpm->pw_sz[hierarchy] = 0;

		/* Reset all password if thisis the PLATFORM hierarchy */
		if (hierarchy == TPM2_HIERARCHY_PLATFORM)
			for (i = 0; i < TPM2_HIERARCHY_NB; i++)
				tpm->pw_sz[i] = 0;

		/* Reset the properties */
		for (i = 0; i < TPM2_PROPERTY_NB; i++)
			tpm->properties[i] = 0;

		/* Reset the PCRs and their number of extensions */
		for (i = 0; i < SANDBOX_TPM_PCR_NB; i++) {
			tpm->pcr_extensions[i] = 0;
			for (j = 0; j < TPM2_DIGEST_LEN; j++)
				tpm->pcr[i][j] = 0;
		}

		sandbox_tpm2_fill_buf(recv, recv_len, tag, rc);
		break;

	case TPM2_CC_HIERCHANGEAUTH:
		new_pw_sz = get_unaligned_be16(sent);
		sent += sizeof(new_pw_sz);
		if (new_pw_sz > TPM2_DIGEST_LEN) {
			rc = TPM2_RC_SIZE;
		} else if (new_pw_sz) {
			tpm->pw_sz[hierarchy] = new_pw_sz;
			memcpy(tpm->pw[hierarchy], sent, new_pw_sz);
			sent += new_pw_sz;
		}

		sandbox_tpm2_fill_buf(recv, recv_len, tag, rc);
		break;

	case TPM2_CC_GET_CAPABILITY:
		capability = get_unaligned_be32(sent);
		sent += sizeof(capability);
		if (capability != TPM_CAP_TPM_PROPERTIES) {
			printf("Sandbox TPM only support TPM_CAPABILITIES\n");
			return TPM2_RC_HANDLE;
		}

		property = get_unaligned_be32(sent);
		sent += sizeof(property);
		property -= TPM2_PROPERTIES_OFFSET;

		property_count = get_unaligned_be32(sent);
		sent += sizeof(property_count);
		if (!property_count ||
		    property + property_count > TPM2_PROPERTY_NB) {
			rc = TPM2_RC_HANDLE;
			return sandbox_tpm2_fill_buf(recv, recv_len, tag, rc);
		}

		/* Write tag */
		put_unaligned_be16(tag, recv);
		recv += sizeof(tag);

		/* Ignore length for now */
		recv += sizeof(u32);

		/* Write return code */
		put_unaligned_be32(rc, recv);
		recv += sizeof(rc);

		/* Tell there is more data to read */
		*recv = TPMI_YES;
		recv += sizeof(yes_no);

		/* Repeat the capability */
		put_unaligned_be32(capability, recv);
		recv += sizeof(capability);

		/* Give the number of properties that follow */
		put_unaligned_be32(property_count, recv);
		recv += sizeof(property_count);

		/* Fill with the properties */
		for (i = 0; i < property_count; i++) {
			put_unaligned_be32(TPM2_PROPERTIES_OFFSET + property +
					   i, recv);
			recv += sizeof(property);
			put_unaligned_be32(tpm->properties[property + i],
					   recv);
			recv += sizeof(property);
		}

		/* Add trailing \0 */
		*recv = '\0';

		/* Write response length */
		*recv_len = recv - recvbuf;
		put_unaligned_be32(*recv_len, recvbuf + sizeof(tag));

		break;

	case TPM2_CC_DAM_PARAMETERS:
		tpm->properties[TPM2_PROP_MAX_TRIES] = get_unaligned_be32(sent);
		sent += sizeof(*tpm->properties);
		tpm->properties[TPM2_RECOVERY_TIME] = get_unaligned_be32(sent);
		sent += sizeof(*tpm->properties);
		tpm->properties[TPM2_LOCKOUT_RECOVERY] = get_unaligned_be32(sent);
		sent += sizeof(*tpm->properties);

		sandbox_tpm2_fill_buf(recv, recv_len, tag, rc);
		break;

	case TPM2_CC_PCR_READ:
		selections = get_unaligned_be32(sent);
		sent += sizeof(selections);
		if (selections != 1) {
			printf("Sandbox cannot handle more than one PCR\n");
			rc = TPM2_RC_VALUE;
			return sandbox_tpm2_fill_buf(recv, recv_len, tag, rc);
		}

		alg = get_unaligned_be16(sent);
		sent += sizeof(alg);
		if (alg != TPM2_ALG_SHA256) {
			printf("Sandbox TPM only handle SHA256 algorithm\n");
			rc = TPM2_RC_VALUE;
			return sandbox_tpm2_fill_buf(recv, recv_len, tag, rc);
		}

		pcr_array_sz = *sent;
		sent += sizeof(pcr_array_sz);
		if (!pcr_array_sz || pcr_array_sz > 8) {
			printf("Sandbox TPM cannot handle so much PCRs\n");
			rc = TPM2_RC_VALUE;
			return sandbox_tpm2_fill_buf(recv, recv_len, tag, rc);
		}

		for (i = 0; i < pcr_array_sz; i++)
			pcr_map += (u64)sent[i] << (i * 8);

		if (pcr_map >> SANDBOX_TPM_PCR_NB) {
			printf("Sandbox TPM handles up to %d PCR(s)\n",
			       SANDBOX_TPM_PCR_NB);
			rc = TPM2_RC_VALUE;
			return sandbox_tpm2_fill_buf(recv, recv_len, tag, rc);
		}

		if (!pcr_map) {
			printf("Empty PCR map.\n");
			rc = TPM2_RC_VALUE;
			return sandbox_tpm2_fill_buf(recv, recv_len, tag, rc);
		}

		for (i = 0; i < SANDBOX_TPM_PCR_NB; i++)
			if (pcr_map & BIT(i))
				pcr_index = i;

		/* Write tag */
		put_unaligned_be16(tag, recv);
		recv += sizeof(tag);

		/* Ignore length for now */
		recv += sizeof(u32);

		/* Write return code */
		put_unaligned_be32(rc, recv);
		recv += sizeof(rc);

		/* Number of extensions */
		put_unaligned_be32(tpm->pcr_extensions[pcr_index], recv);
		recv += sizeof(u32);

		/* Copy the PCR */
		memcpy(recv, tpm->pcr[pcr_index], TPM2_DIGEST_LEN);
		recv += TPM2_DIGEST_LEN;

		/* Add trailing \0 */
		*recv = '\0';

		/* Write response length */
		*recv_len = recv - recvbuf;
		put_unaligned_be32(*recv_len, recvbuf + sizeof(tag));

		break;

	case TPM2_CC_PCR_EXTEND:
		/* Get the PCR index */
		pcr_index = get_unaligned_be32(sendbuf + sizeof(tag) +
					       sizeof(length) +
					       sizeof(command));
		if (pcr_index > SANDBOX_TPM_PCR_NB) {
			printf("Sandbox TPM handles up to %d PCR(s)\n",
			       SANDBOX_TPM_PCR_NB);
			rc = TPM2_RC_VALUE;
		}

		/* Check the number of hashes */
		pcr_nb = get_unaligned_be32(sent);
		sent += sizeof(pcr_nb);
		if (pcr_nb != 1) {
			printf("Sandbox cannot handle more than one PCR\n");
			rc = TPM2_RC_VALUE;
			return sandbox_tpm2_fill_buf(recv, recv_len, tag, rc);
		}

		/* Check the hash algorithm */
		alg = get_unaligned_be16(sent);
		sent += sizeof(alg);
		if (alg != TPM2_ALG_SHA256) {
			printf("Sandbox TPM only handle SHA256 algorithm\n");
			rc = TPM2_RC_VALUE;
			return sandbox_tpm2_fill_buf(recv, recv_len, tag, rc);
		}

		/* Extend the PCR */
		rc = sandbox_tpm2_extend(dev, pcr_index, sent);

		sandbox_tpm2_fill_buf(recv, recv_len, tag, rc);
		break;

	default:
		printf("TPM2 command %02x unknown in Sandbox\n", command);
		rc = TPM2_RC_COMMAND_CODE;
		sandbox_tpm2_fill_buf(recv, recv_len, tag, rc);
	}

	return 0;
}

static int sandbox_tpm2_get_desc(struct udevice *dev, char *buf, int size)
{
	if (size < 15)
		return -ENOSPC;

	return snprintf(buf, size, "Sandbox TPM2.x");
}

static int sandbox_tpm2_open(struct udevice *dev)
{
	struct sandbox_tpm2 *tpm = dev_get_priv(dev);

	if (tpm->init_done)
		return -EIO;

	tpm->init_done = true;

	return 0;
}

static int sandbox_tpm2_probe(struct udevice *dev)
{
	struct sandbox_tpm2 *tpm = dev_get_priv(dev);
	struct tpm_chip_priv *priv = dev_get_uclass_priv(dev);

	/* Use the TPM v2 stack */
	priv->version = TPM_V2;

	memset(tpm, 0, sizeof(*tpm));

	priv->pcr_count = 32;
	priv->pcr_select_min = 2;

	return 0;
}

static int sandbox_tpm2_close(struct udevice *dev)
{
	return 0;
}

static const struct tpm_ops sandbox_tpm2_ops = {
	.open		= sandbox_tpm2_open,
	.close		= sandbox_tpm2_close,
	.get_desc	= sandbox_tpm2_get_desc,
	.xfer		= sandbox_tpm2_xfer,
};

static const struct udevice_id sandbox_tpm2_ids[] = {
	{ .compatible = "sandbox,tpm2" },
	{ }
};

U_BOOT_DRIVER(sandbox_tpm2) = {
	.name   = "sandbox_tpm2",
	.id     = UCLASS_TPM,
	.of_match = sandbox_tpm2_ids,
	.ops    = &sandbox_tpm2_ops,
	.probe	= sandbox_tpm2_probe,
	.priv_auto_alloc_size = sizeof(struct sandbox_tpm2),
};
