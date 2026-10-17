// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2023 Linaro Limited
 * Copyright (c) 2018 Bootlin
 * Author: Miquel Raynal <miquel.raynal@bootlin.com>
 */

#include <dm.h>
#include <dm/of_access.h>
#include <tpm_api.h>
#include <tpm-common.h>
#include <tpm-v2.h>
#include <u-boot/sha1.h>
#include <u-boot/sha256.h>
#include <u-boot/sha512.h>
#include <version_string.h>
#include <asm/io.h>
#include <linux/bitops.h>
#include <linux/unaligned/be_byteshift.h>
#include <linux/unaligned/generic.h>
#include <linux/unaligned/le_byteshift.h>

#include "tpm-utils.h"

const enum tpm2_algorithms tpm2_supported_algorithms[4] = {
	TPM2_ALG_SHA1,
	TPM2_ALG_SHA256,
	TPM2_ALG_SHA384,
	TPM2_ALG_SHA512,
};

int tcg2_get_active_pcr_banks(struct udevice *dev, u32 *active_pcr_banks)
{
	u32 supported = 0;
	u32 pcr_banks = 0;
	u32 active = 0;
	int rc;

	rc = tpm2_get_pcr_info(dev, &supported, &active, &pcr_banks);
	if (rc)
		return rc;

	*active_pcr_banks = active;

	return 0;
}

u32 tcg2_event_get_size(struct tpml_digest_values *digest_list)
{
	u32 len;
	size_t i;

	len = offsetof(struct tcg_pcr_event2, digests);
	len += offsetof(struct tpml_digest_values, digests);
	for (i = 0; i < digest_list->count; ++i) {
		u16 l = tpm2_algorithm_to_len(digest_list->digests[i].hash_alg);

		if (!l)
			continue;

		len += l + offsetof(struct tpmt_ha, digest);
	}
	len += sizeof(u32);

	return len;
}

int tcg2_create_digest(struct udevice *dev, const u8 *input, u32 length,
		       struct tpml_digest_values *digest_list)
{
	u8 final[sizeof(union tpmu_ha)];
	sha256_context ctx_256;
	sha512_context ctx_512;
	sha1_context ctx;
	u32 active;
	size_t i;
	u32 len;
	int rc;

	rc = tcg2_get_active_pcr_banks(dev, &active);
	if (rc)
		return rc;

	digest_list->count = 0;
	for (i = 0; i < ARRAY_SIZE(tpm2_supported_algorithms); ++i) {
		u32 mask =
			tpm2_algorithm_to_mask(tpm2_supported_algorithms[i]);

		if (!(active & mask))
			continue;

		switch (tpm2_supported_algorithms[i]) {
		case TPM2_ALG_SHA1:
			sha1_starts(&ctx);
			sha1_update(&ctx, input, length);
			sha1_finish(&ctx, final);
			len = TPM2_SHA1_DIGEST_SIZE;
			break;
		case TPM2_ALG_SHA256:
			sha256_starts(&ctx_256);
			sha256_update(&ctx_256, input, length);
			sha256_finish(&ctx_256, final);
			len = TPM2_SHA256_DIGEST_SIZE;
			break;
		case TPM2_ALG_SHA384:
			sha384_starts(&ctx_512);
			sha384_update(&ctx_512, input, length);
			sha384_finish(&ctx_512, final);
			len = TPM2_SHA384_DIGEST_SIZE;
			break;
		case TPM2_ALG_SHA512:
			sha512_starts(&ctx_512);
			sha512_update(&ctx_512, input, length);
			sha512_finish(&ctx_512, final);
			len = TPM2_SHA512_DIGEST_SIZE;
			break;
		default:
			printf("%s: unsupported algorithm %x\n", __func__,
			       tpm2_supported_algorithms[i]);
			continue;
		}

		digest_list->digests[digest_list->count].hash_alg =
			tpm2_supported_algorithms[i];
		memcpy(&digest_list->digests[digest_list->count].digest, final,
		       len);
		digest_list->count++;
	}

	return 0;
}

void tcg2_log_append(u32 pcr_index, u32 event_type,
		     struct tpml_digest_values *digest_list, u32 size,
		     const u8 *event, u8 *log)
{
	size_t len;
	size_t pos;
	u32 i;

	pos = offsetof(struct tcg_pcr_event2, pcr_index);
	put_unaligned_le32(pcr_index, log);
	pos = offsetof(struct tcg_pcr_event2, event_type);
	put_unaligned_le32(event_type, log + pos);
	pos = offsetof(struct tcg_pcr_event2, digests) +
		offsetof(struct tpml_digest_values, count);
	put_unaligned_le32(digest_list->count, log + pos);

	pos = offsetof(struct tcg_pcr_event2, digests) +
		offsetof(struct tpml_digest_values, digests);
	for (i = 0; i < digest_list->count; ++i) {
		u16 hash_alg = digest_list->digests[i].hash_alg;

		len = tpm2_algorithm_to_len(hash_alg);
		if (!len)
			continue;

		pos += offsetof(struct tpmt_ha, hash_alg);
		put_unaligned_le16(hash_alg, log + pos);
		pos += offsetof(struct tpmt_ha, digest);
		memcpy(log + pos, (u8 *)&digest_list->digests[i].digest, len);
		pos += len;
	}

	put_unaligned_le32(size, log + pos);
	pos += sizeof(u32);
	memcpy(log + pos, event, size);
}

static int tcg2_log_append_check(struct tcg2_event_log *elog, u32 pcr_index,
				 u32 event_type,
				 struct tpml_digest_values *digest_list,
				 u32 size, const u8 *event)
{
	u32 event_size;
	u8 *log;

	event_size = size + tcg2_event_get_size(digest_list);
	if (elog->log_position + event_size > elog->log_size) {
		printf("%s: log too large: %u + %u > %u\n", __func__,
		       elog->log_position, event_size, elog->log_size);
		return -ENOBUFS;
	}

	log = elog->log + elog->log_position;
	elog->log_position += event_size;

	tcg2_log_append(pcr_index, event_type, digest_list, size, event, log);

	return 0;
}

static int tcg2_log_init(struct udevice *dev, struct tcg2_event_log *elog)
{
	struct tcg_efi_spec_id_event *ev;
	struct tcg_pcr_event *log;
	u32 event_size;
	u32 count = 0;
	u32 log_size;
	u32 active;
	u32 mask;
	size_t i;
	u16 len;
	int rc;

	rc = tcg2_get_active_pcr_banks(dev, &active);
	if (rc)
		return rc;

	event_size = offsetof(struct tcg_efi_spec_id_event, digest_sizes);
	for (i = 0; i < ARRAY_SIZE(tpm2_supported_algorithms); ++i) {
		mask = tpm2_algorithm_to_mask(tpm2_supported_algorithms[i]);

		if (!(active & mask))
			continue;

		switch (tpm2_supported_algorithms[i]) {
		case TPM2_ALG_SHA1:
		case TPM2_ALG_SHA256:
		case TPM2_ALG_SHA384:
		case TPM2_ALG_SHA512:
			count++;
			break;
		default:
			continue;
		}
	}

	event_size += 1 +
		(sizeof(struct tcg_efi_spec_id_event_algorithm_size) * count);
	log_size = offsetof(struct tcg_pcr_event, event) + event_size;

	if (log_size > elog->log_size) {
		printf("%s: log too large: %u > %u\n", __func__, log_size,
		       elog->log_size);
		return -ENOBUFS;
	}

	log = (struct tcg_pcr_event *)elog->log;
	put_unaligned_le32(0, &log->pcr_index);
	put_unaligned_le32(EV_NO_ACTION, &log->event_type);
	memset(&log->digest, 0, sizeof(log->digest));
	put_unaligned_le32(event_size, &log->event_size);

	ev = (struct tcg_efi_spec_id_event *)log->event;
	strlcpy((char *)ev->signature, TCG_EFI_SPEC_ID_EVENT_SIGNATURE_03,
		sizeof(ev->signature));
	put_unaligned_le32(0, &ev->platform_class);
	ev->spec_version_minor = TCG_EFI_SPEC_ID_EVENT_SPEC_VERSION_MINOR_TPM2;
	ev->spec_version_major = TCG_EFI_SPEC_ID_EVENT_SPEC_VERSION_MAJOR_TPM2;
	ev->spec_errata = TCG_EFI_SPEC_ID_EVENT_SPEC_VERSION_ERRATA_TPM2;
	ev->uintn_size = sizeof(size_t) / sizeof(u32);
	put_unaligned_le32(count, &ev->number_of_algorithms);

	count = 0;
	for (i = 0; i < ARRAY_SIZE(tpm2_supported_algorithms); ++i) {
		mask = tpm2_algorithm_to_mask(tpm2_supported_algorithms[i]);

		if (!(active & mask))
			continue;

		len = tpm2_algorithm_to_len(tpm2_supported_algorithms[i]);
		if (!len)
			continue;

		put_unaligned_le16(tpm2_supported_algorithms[i],
				   &ev->digest_sizes[count].algorithm_id);
		put_unaligned_le16(len, &ev->digest_sizes[count].digest_size);
		count++;
	}

	*((u8 *)ev + (event_size - 1)) = 0;
	elog->log_position = log_size;

	return 0;
}

static int tcg2_replay_eventlog(struct tcg2_event_log *elog,
				struct udevice *dev,
				struct tpml_digest_values *digest_list,
				u32 log_position)
{
	const u32 offset = offsetof(struct tcg_pcr_event2, digests) +
		offsetof(struct tpml_digest_values, digests);
	u32 event_size;
	u32 count;
	u16 algo;
	u32 pcr;
	u32 pos;
	u16 len;
	u8 *log;
	int rc;
	u32 i;

	while (log_position + offset < elog->log_size) {
		log = elog->log + log_position;

		pos = offsetof(struct tcg_pcr_event2, pcr_index);
		pcr = get_unaligned_le32(log + pos);
		pos = offsetof(struct tcg_pcr_event2, event_type);
		if (!get_unaligned_le32(log + pos))
			return 0;

		pos = offsetof(struct tcg_pcr_event2, digests) +
			offsetof(struct tpml_digest_values, count);
		count = get_unaligned_le32(log + pos);
		if (count > ARRAY_SIZE(tpm2_supported_algorithms) ||
		    (digest_list->count && digest_list->count != count))
			return 0;

		pos = offsetof(struct tcg_pcr_event2, digests) +
			offsetof(struct tpml_digest_values, digests);
		for (i = 0; i < count; ++i) {
			pos += offsetof(struct tpmt_ha, hash_alg);
			if (log_position + pos + sizeof(u16) >= elog->log_size)
				return 0;

			algo = get_unaligned_le16(log + pos);
			pos += offsetof(struct tpmt_ha, digest);
			switch (algo) {
			case TPM2_ALG_SHA1:
			case TPM2_ALG_SHA256:
			case TPM2_ALG_SHA384:
			case TPM2_ALG_SHA512:
				len = tpm2_algorithm_to_len(algo);
				break;
			default:
				return 0;
			}

			if (digest_list->count) {
				if (algo != digest_list->digests[i].hash_alg ||
				    log_position + pos + len >= elog->log_size)
					return 0;

				memcpy(digest_list->digests[i].digest.sha512,
				       log + pos, len);
			}

			pos += len;
		}

		if (log_position + pos + sizeof(u32) >= elog->log_size)
			return 0;

		event_size = get_unaligned_le32(log + pos);
		pos += event_size + sizeof(u32);
		if (log_position + pos > elog->log_size)
			return 0;

		if (digest_list->count) {
			rc = tcg2_pcr_extend(dev, pcr, digest_list);
			if (rc)
				return rc;
		}

		log_position += pos;
	}

	elog->log_position = log_position;
	elog->found = true;
	return 0;
}

static int tcg2_log_parse(struct udevice *dev, struct tcg2_event_log *elog)
{
	struct tpml_digest_values digest_list;
	struct tcg_efi_spec_id_event *event;
	struct tcg_pcr_event *log;
	u32 log_active;
	u32 calc_size;
	u32 active;
	u32 count;
	u32 evsz;
	u32 mask;
	u16 algo;
	u16 len;
	int rc;
	u32 i;
	u16 j;

	if (elog->log_size <= offsetof(struct tcg_pcr_event, event))
		return 0;

	log = (struct tcg_pcr_event *)elog->log;
	if (get_unaligned_le32(&log->pcr_index) != 0 ||
	    get_unaligned_le32(&log->event_type) != EV_NO_ACTION)
		return 0;

	for (i = 0; i < sizeof(log->digest); i++) {
		if (log->digest[i])
			return 0;
	}

	evsz = get_unaligned_le32(&log->event_size);
	if (evsz < offsetof(struct tcg_efi_spec_id_event, digest_sizes) ||
	    evsz + offsetof(struct tcg_pcr_event, event) > elog->log_size)
		return 0;

	event = (struct tcg_efi_spec_id_event *)log->event;
	if (memcmp(event->signature, TCG_EFI_SPEC_ID_EVENT_SIGNATURE_03,
		   sizeof(TCG_EFI_SPEC_ID_EVENT_SIGNATURE_03)))
		return 0;

	if (event->spec_version_minor != TCG_EFI_SPEC_ID_EVENT_SPEC_VERSION_MINOR_TPM2 ||
	    event->spec_version_major != TCG_EFI_SPEC_ID_EVENT_SPEC_VERSION_MAJOR_TPM2)
		return 0;

	count = get_unaligned_le32(&event->number_of_algorithms);
	if (count > ARRAY_SIZE(tpm2_supported_algorithms))
		return 0;

	calc_size = offsetof(struct tcg_efi_spec_id_event, digest_sizes) +
		(sizeof(struct tcg_efi_spec_id_event_algorithm_size) * count) +
		1;
	if (evsz != calc_size)
		return 0;

	rc = tcg2_get_active_pcr_banks(dev, &active);
	if (rc)
		return rc;

	digest_list.count = 0;
	log_active = 0;

	for (i = 0; i < count; ++i) {
		algo = get_unaligned_le16(&event->digest_sizes[i].algorithm_id);
		mask = tpm2_algorithm_to_mask(algo);

		if (!(active & mask))
			return 0;

		switch (algo) {
		case TPM2_ALG_SHA1:
		case TPM2_ALG_SHA256:
		case TPM2_ALG_SHA384:
		case TPM2_ALG_SHA512:
			len = get_unaligned_le16(&event->digest_sizes[i].digest_size);
			if (tpm2_algorithm_to_len(algo) != len)
				return 0;
			digest_list.digests[digest_list.count++].hash_alg = algo;
			break;
		default:
			return 0;
		}

		log_active |= mask;
	}

	/* Ensure the previous firmware extended all the PCRs. */
	if (log_active != active)
		return 0;

	/* Read PCR0 to check if previous firmware extended the PCRs or not. */
	rc = tcg2_pcr_read(dev, 0, &digest_list);
	if (rc)
		return rc;

	for (i = 0; i < digest_list.count; ++i) {
		len = tpm2_algorithm_to_len(digest_list.digests[i].hash_alg);
		for (j = 0; j < len; ++j) {
			if (digest_list.digests[i].digest.sha512[j])
				break;
		}

		/* PCR is non-zero; it has been extended, so skip extending. */
		if (j != len) {
			digest_list.count = 0;
			break;
		}
	}

	return tcg2_replay_eventlog(elog, dev, &digest_list,
				    offsetof(struct tcg_pcr_event, event) +
				    evsz);
}

int tcg2_pcr_extend(struct udevice *dev, u32 pcr_index,
		    struct tpml_digest_values *digest_list)
{
	u32 rc;
	u32 i;

	for (i = 0; i < digest_list->count; i++) {
		u32 alg = digest_list->digests[i].hash_alg;

		rc = tpm2_pcr_extend(dev, pcr_index, alg,
				     (u8 *)&digest_list->digests[i].digest,
				     tpm2_algorithm_to_len(alg));
		if (rc) {
			printf("%s: error pcr:%u alg:%08x\n", __func__,
			       pcr_index, alg);
			return rc;
		}
	}

	return 0;
}

int tcg2_pcr_read(struct udevice *dev, u32 pcr_index,
		  struct tpml_digest_values *digest_list)
{
	struct tpm_chip_priv *priv;
	u32 rc;
	u32 i;

	priv = dev_get_uclass_priv(dev);
	if (!priv)
		return -ENODEV;

	for (i = 0; i < digest_list->count; i++) {
		u32 alg = digest_list->digests[i].hash_alg;
		u8 *digest = (u8 *)&digest_list->digests[i].digest;

		rc = tpm2_pcr_read(dev, pcr_index, priv->pcr_select_min, alg,
				   digest, tpm2_algorithm_to_len(alg), NULL);
		if (rc) {
			printf("%s: error pcr:%u alg:%08x\n", __func__,
			       pcr_index, alg);
			return rc;
		}
	}

	return 0;
}

int tcg2_measure_data(struct udevice *dev, struct tcg2_event_log *elog,
		      u32 pcr_index, u32 size, const u8 *data, u32 event_type,
		      u32 event_size, const u8 *event)
{
	struct tpml_digest_values digest_list;
	int rc;

	if (data)
		rc = tcg2_create_digest(dev, data, size, &digest_list);
	else
		rc = tcg2_create_digest(dev, event, event_size, &digest_list);
	if (rc)
		return rc;

	rc = tcg2_pcr_extend(dev, pcr_index, &digest_list);
	if (rc)
		return rc;

	return tcg2_log_append_check(elog, pcr_index, event_type, &digest_list,
				     event_size, event);
}

int tcg2_log_prepare_buffer(struct udevice *dev, struct tcg2_event_log *elog,
			    bool ignore_existing_log)
{
	struct tcg2_event_log log;
	int rc;

	elog->log_position = 0;
	elog->found = false;

	rc = tcg2_platform_get_log(dev, (void **)&log.log, &log.log_size);
	if (!rc) {
		log.log_position = 0;
		log.found = false;

		if (!ignore_existing_log) {
			rc = tcg2_log_parse(dev, &log);
			if (rc)
				return rc;
		}

		if (elog->log_size) {
			if (log.found) {
				if (elog->log_size < log.log_position)
					return -ENOSPC;

				/*
				 * Copy the discovered log into the user buffer
				 * if there's enough space.
				 */
				memcpy(elog->log, log.log, log.log_position);
			}

			unmap_physmem(log.log, MAP_NOCACHE);
		} else {
			elog->log = log.log;
			elog->log_size = log.log_size;
		}

		elog->log_position = log.log_position;
		elog->found = log.found;
	}

	/*
	 * Initialize the log buffer if no log was discovered and the buffer is
	 * valid. User's can pass in their own buffer as a fallback if no
	 * memory region is found.
	 */
	if (!elog->found && elog->log_size)
		rc = tcg2_log_init(dev, elog);

	return rc;
}

int tcg2_measurement_init(struct udevice **dev, struct tcg2_event_log *elog,
			  bool ignore_existing_log)
{
	int rc;

	rc = tcg2_platform_get_tpm2(dev);
	if (rc)
		return rc;

	rc = tpm_auto_start(*dev);
	if (rc)
		return rc;

	rc = tcg2_log_prepare_buffer(*dev, elog, ignore_existing_log);
	if (rc) {
		tcg2_measurement_term(*dev, elog, true);
		return rc;
	}

	rc = tcg2_measure_event(*dev, elog, 0, EV_S_CRTM_VERSION,
				strlen(version_string) + 1,
				(u8 *)version_string);
	if (rc) {
		tcg2_measurement_term(*dev, elog, true);
		return rc;
	}

	return 0;
}

void tcg2_measurement_term(struct udevice *dev, struct tcg2_event_log *elog,
			   bool error)
{
	u32 event = error ? 0x1 : 0xffffffff;
	int i;

	for (i = 0; i < 8; ++i)
		tcg2_measure_event(dev, elog, i, EV_SEPARATOR, sizeof(event),
				   (const u8 *)&event);

	if (elog->log)
		unmap_physmem(elog->log, MAP_NOCACHE);
}

__weak int tcg2_platform_get_log(struct udevice *dev, void **addr, u32 *size)
{
	const __be32 *addr_prop;
	const __be32 *size_prop;
	int asize;
	int ssize;

	*addr = NULL;
	*size = 0;

	addr_prop = dev_read_prop(dev, "tpm_event_log_addr", &asize);
	if (!addr_prop)
		addr_prop = dev_read_prop(dev, "linux,sml-base", &asize);

	size_prop = dev_read_prop(dev, "tpm_event_log_size", &ssize);
	if (!size_prop)
		size_prop = dev_read_prop(dev, "linux,sml-size", &ssize);

	if (addr_prop && size_prop) {
		u64 a = of_read_number(addr_prop, asize / sizeof(__be32));
		u64 s = of_read_number(size_prop, ssize / sizeof(__be32));

		*addr = map_physmem(a, s, MAP_NOCACHE);
		*size = (u32)s;
	} else {
		struct ofnode_phandle_args args;
		phys_addr_t a;
		fdt_size_t s;

		if (dev_read_phandle_with_args(dev, "memory-region", NULL, 0,
					       0, &args))
			return -ENODEV;

		a = ofnode_get_addr_size(args.node, "reg", &s);
		if (a == FDT_ADDR_T_NONE)
			return -ENOMEM;

		*addr = map_physmem(a, s, MAP_NOCACHE);
		*size = (u32)s;
	}

	return 0;
}

__weak int tcg2_platform_get_tpm2(struct udevice **dev)
{
	for_each_tpm_device(*dev) {
		if (tpm_get_version(*dev) == TPM_V2)
			return 0;
	}

	return -ENODEV;
}

__weak void tcg2_platform_startup_error(struct udevice *dev, int rc) {}

u32 tpm2_startup(struct udevice *dev, enum tpm2_startup_types mode)
{
	const u8 command_v2[12] = {
		tpm_u16(TPM2_ST_NO_SESSIONS),
		tpm_u32(12),
		tpm_u32(TPM2_CC_STARTUP),
		tpm_u16(mode),
	};
	int ret;

	/*
	 * Note TPM2_Startup command will return RC_SUCCESS the first time,
	 * but will return RC_INITIALIZE otherwise.
	 */
	ret = tpm_sendrecv_command(dev, command_v2, NULL, NULL);
	if (ret && ret != TPM2_RC_INITIALIZE)
		return ret;

	return 0;
}

u32 tpm2_self_test(struct udevice *dev, enum tpm2_yes_no full_test)
{
	const u8 command_v2[12] = {
		tpm_u16(TPM2_ST_NO_SESSIONS),
		tpm_u32(11),
		tpm_u32(TPM2_CC_SELF_TEST),
		full_test,
	};

	return tpm_sendrecv_command(dev, command_v2, NULL, NULL);
}

u32 tpm2_auto_start(struct udevice *dev)
{
	u32 rc;

	rc = tpm2_self_test(dev, TPMI_YES);

	if (rc == TPM2_RC_INITIALIZE) {
		rc = tpm2_startup(dev, TPM2_SU_CLEAR);
		if (rc)
			return rc;

		rc = tpm2_self_test(dev, TPMI_YES);
	}

	return rc;
}

u32 tpm2_clear(struct udevice *dev, u32 handle, const char *pw,
	       const ssize_t pw_sz)
{
	/* Length of the message header, up to start of password */
	uint offset = 27;
	u8 command_v2[COMMAND_BUFFER_SIZE] = {
		tpm_u16(TPM2_ST_SESSIONS),	/* TAG */
		tpm_u32(offset + pw_sz),	/* Length */
		tpm_u32(TPM2_CC_CLEAR),		/* Command code */

		/* HANDLE */
		tpm_u32(handle),		/* TPM resource handle */

		/* AUTH_SESSION */
		tpm_u32(9 + pw_sz),		/* Authorization size */
		tpm_u32(TPM2_RS_PW),		/* Session handle */
		tpm_u16(0),			/* Size of <nonce> */
						/* <nonce> (if any) */
		0,				/* Attributes: Cont/Excl/Rst */
		tpm_u16(pw_sz),			/* Size of <hmac/password> */
		/* STRING(pw)			   <hmac/password> (if any) */
	};
	int ret;

	/*
	 * Fill the command structure starting from the first buffer:
	 *     - the password (if any)
	 */
	ret = pack_byte_string(command_v2, sizeof(command_v2), "s",
			       offset, pw, pw_sz);
	offset += pw_sz;
	if (ret)
		return TPM_LIB_ERROR;

	return tpm_sendrecv_command(dev, command_v2, NULL, NULL);
}

u32 tpm2_nv_define_space(struct udevice *dev, u32 space_index,
			 size_t space_size, u32 nv_attributes,
			 const u8 *nv_policy, size_t nv_policy_size)
{
	/*
	 * Calculate the offset of the nv_policy piece by adding each of the
	 * chunks below.
	 */
	const int platform_len = sizeof(u32);
	const int session_hdr_len = 13;
	const int message_len = 14;
	uint offset = TPM2_HDR_LEN + platform_len + session_hdr_len +
		message_len;
	u8 command_v2[COMMAND_BUFFER_SIZE] = {
		/* header 10 bytes */
		tpm_u16(TPM2_ST_SESSIONS),	/* TAG */
		tpm_u32(offset + nv_policy_size + 2),/* Length */
		tpm_u32(TPM2_CC_NV_DEFINE_SPACE),/* Command code */

		/* handles 4 bytes */
		tpm_u32(TPM2_RH_PLATFORM),	/* Primary platform seed */

		/* session header 13 bytes */
		tpm_u32(9),			/* Header size */
		tpm_u32(TPM2_RS_PW),		/* Password authorisation */
		tpm_u16(0),			/* nonce_size */
		0,				/* session_attrs */
		tpm_u16(0),			/* auth_size */

		/* message 14 bytes + policy */
		tpm_u16(message_len + nv_policy_size),	/* size */
		tpm_u32(space_index),
		tpm_u16(TPM2_ALG_SHA256),
		tpm_u32(nv_attributes),
		tpm_u16(nv_policy_size),
		/*
		 * nv_policy
		 * space_size
		 */
	};
	int ret;

	/*
	 * Fill the command structure starting from the first buffer:
	 *     - the password (if any)
	 */
	ret = pack_byte_string(command_v2, sizeof(command_v2), "sw",
			       offset, nv_policy, nv_policy_size,
			       offset + nv_policy_size, space_size);
	if (ret)
		return TPM_LIB_ERROR;

	return tpm_sendrecv_command(dev, command_v2, NULL, NULL);
}

u32 tpm2_pcr_extend(struct udevice *dev, u32 index, u32 algorithm,
		    const u8 *digest, u32 digest_len)
{
	/* Length of the message header, up to start of digest */
	uint offset = 33;
	u8 command_v2[COMMAND_BUFFER_SIZE] = {
		tpm_u16(TPM2_ST_SESSIONS),	/* TAG */
		tpm_u32(offset + digest_len),	/* Length */
		tpm_u32(TPM2_CC_PCR_EXTEND),	/* Command code */

		/* HANDLE */
		tpm_u32(index),			/* Handle (PCR Index) */

		/* AUTH_SESSION */
		tpm_u32(9),			/* Authorization size */
		tpm_u32(TPM2_RS_PW),		/* Session handle */
		tpm_u16(0),			/* Size of <nonce> */
						/* <nonce> (if any) */
		0,				/* Attributes: Cont/Excl/Rst */
		tpm_u16(0),			/* Size of <hmac/password> */
						/* <hmac/password> (if any) */

		/* hashes */
		tpm_u32(1),			/* Count (number of hashes) */
		tpm_u16(algorithm),	/* Algorithm of the hash */
		/* STRING(digest)		   Digest */
	};
	int ret;

	if (!digest)
		return -EINVAL;
	/*
	 * Fill the command structure starting from the first buffer:
	 *     - the digest
	 */
	ret = pack_byte_string(command_v2, sizeof(command_v2), "s",
			       offset, digest, digest_len);
	if (ret)
		return TPM_LIB_ERROR;

	return tpm_sendrecv_command(dev, command_v2, NULL, NULL);
}

u32 tpm2_nv_read_value(struct udevice *dev, u32 index, void *data, u32 count)
{
	u8 command_v2[COMMAND_BUFFER_SIZE] = {
		/* header 10 bytes */
		tpm_u16(TPM2_ST_SESSIONS),	/* TAG */
		tpm_u32(10 + 8 + 4 + 9 + 4),	/* Length */
		tpm_u32(TPM2_CC_NV_READ),	/* Command code */

		/* handles 8 bytes */
		tpm_u32(TPM2_RH_PLATFORM),	/* Primary platform seed */
		tpm_u32(HR_NV_INDEX + index),	/* Password authorisation */

		/* AUTH_SESSION */
		tpm_u32(9),			/* Authorization size */
		tpm_u32(TPM2_RS_PW),		/* Session handle */
		tpm_u16(0),			/* Size of <nonce> */
						/* <nonce> (if any) */
		0,				/* Attributes: Cont/Excl/Rst */
		tpm_u16(0),			/* Size of <hmac/password> */
						/* <hmac/password> (if any) */

		tpm_u16(count),			/* Number of bytes */
		tpm_u16(0),			/* Offset */
	};
	size_t response_len = COMMAND_BUFFER_SIZE;
	u8 response[COMMAND_BUFFER_SIZE];
	int ret;
	u16 tag;
	u32 size, code;

	ret = tpm_sendrecv_command(dev, command_v2, response, &response_len);
	if (ret)
		return log_msg_ret("read", ret);
	if (unpack_byte_string(response, response_len, "wdds",
			       0, &tag, 2, &size, 6, &code,
			       16, data, count))
		return TPM_LIB_ERROR;

	return 0;
}

u32 tpm2_nv_write_value(struct udevice *dev, u32 index, const void *data,
			u32 count)
{
	struct tpm_chip_priv *priv = dev_get_uclass_priv(dev);
	uint offset = 10 + 8 + 4 + 9 + 2;
	uint len = offset + count + 2;
	/* Use empty password auth if platform hierarchy is disabled */
	u32 auth = priv->plat_hier_disabled ? HR_NV_INDEX + index :
		TPM2_RH_PLATFORM;
	u8 command_v2[COMMAND_BUFFER_SIZE] = {
		/* header 10 bytes */
		tpm_u16(TPM2_ST_SESSIONS),	/* TAG */
		tpm_u32(len),			/* Length */
		tpm_u32(TPM2_CC_NV_WRITE),	/* Command code */

		/* handles 8 bytes */
		tpm_u32(auth),			/* Primary platform seed */
		tpm_u32(HR_NV_INDEX + index),	/* Password authorisation */

		/* AUTH_SESSION */
		tpm_u32(9),			/* Authorization size */
		tpm_u32(TPM2_RS_PW),		/* Session handle */
		tpm_u16(0),			/* Size of <nonce> */
						/* <nonce> (if any) */
		0,				/* Attributes: Cont/Excl/Rst */
		tpm_u16(0),			/* Size of <hmac/password> */
						/* <hmac/password> (if any) */

		tpm_u16(count),
	};
	size_t response_len = COMMAND_BUFFER_SIZE;
	u8 response[COMMAND_BUFFER_SIZE];
	int ret;

	ret = pack_byte_string(command_v2, sizeof(command_v2), "sw",
			       offset, data, count,
			       offset + count, 0);
	if (ret)
		return TPM_LIB_ERROR;

	return tpm_sendrecv_command(dev, command_v2, response, &response_len);
}

u32 tpm2_pcr_read(struct udevice *dev, u32 idx, unsigned int idx_min_sz,
		  u16 algorithm, void *data, u32 digest_len,
		  unsigned int *updates)
{
	u8 idx_array_sz = max(idx_min_sz, DIV_ROUND_UP(idx, 8));
	u8 command_v2[COMMAND_BUFFER_SIZE] = {
		tpm_u16(TPM2_ST_NO_SESSIONS),	/* TAG */
		tpm_u32(17 + idx_array_sz),	/* Length */
		tpm_u32(TPM2_CC_PCR_READ),	/* Command code */

		/* TPML_PCR_SELECTION */
		tpm_u32(1),			/* Number of selections */
		tpm_u16(algorithm),		/* Algorithm of the hash */
		idx_array_sz,			/* Array size for selection */
		/* bitmap(idx)			   Selected PCR bitmap */
	};
	size_t response_len = COMMAND_BUFFER_SIZE;
	u8 response[COMMAND_BUFFER_SIZE];
	unsigned int pcr_sel_idx = idx / 8;
	u8 pcr_sel_bit = BIT(idx % 8);
	unsigned int counter = 0;
	int ret;

	if (pack_byte_string(command_v2, COMMAND_BUFFER_SIZE, "b",
			     17 + pcr_sel_idx, pcr_sel_bit))
		return TPM_LIB_ERROR;

	ret = tpm_sendrecv_command(dev, command_v2, response, &response_len);
	if (ret)
		return ret;

	if (digest_len > response_len)
		return TPM_LIB_ERROR;

	if (unpack_byte_string(response, response_len, "ds",
			       10, &counter,
			       response_len - digest_len, data,
			       digest_len))
		return TPM_LIB_ERROR;

	if (updates)
		*updates = counter;

	return 0;
}

u32 tpm2_get_capability(struct udevice *dev, u32 capability, u32 property,
			void *buf, size_t prop_count)
{
	u8 command_v2[COMMAND_BUFFER_SIZE] = {
		tpm_u16(TPM2_ST_NO_SESSIONS),		/* TAG */
		tpm_u32(22),				/* Length */
		tpm_u32(TPM2_CC_GET_CAPABILITY),	/* Command code */

		tpm_u32(capability),			/* Capability */
		tpm_u32(property),			/* Property */
		tpm_u32(prop_count),			/* Property count */
	};
	u8 response[COMMAND_BUFFER_SIZE];
	size_t response_len = COMMAND_BUFFER_SIZE;
	unsigned int properties_off;
	int ret;

	ret = tpm_sendrecv_command(dev, command_v2, response, &response_len);
	if (ret)
		return ret;

	/*
	 * In the response buffer, the properties are located after the:
	 * tag (u16), response size (u32), response code (u32),
	 * YES/NO flag (u8), TPM_CAP (u32).
	 */
	properties_off = sizeof(u16) + sizeof(u32) + sizeof(u32) +
			 sizeof(u8) + sizeof(u32);
	memcpy(buf, &response[properties_off], response_len - properties_off);

	return 0;
}

static int tpm2_get_num_pcr(struct udevice *dev, u32 *num_pcr)
{
	u8 response[(sizeof(struct tpms_capability_data) -
		offsetof(struct tpms_capability_data, data))];
	u32 properties_offset =
		offsetof(struct tpml_tagged_tpm_property, tpm_property) +
		offsetof(struct tpms_tagged_property, value);
	u32 ret;

	memset(response, 0, sizeof(response));
	ret = tpm2_get_capability(dev, TPM2_CAP_TPM_PROPERTIES,
				  TPM2_PT_PCR_COUNT, response, 1);
	if (ret)
		return ret;

	*num_pcr = get_unaligned_be32(response + properties_offset);
	if (*num_pcr > TPM2_MAX_PCRS) {
		printf("%s: too many pcrs: %u\n", __func__, *num_pcr);
		return -E2BIG;
	}

	return 0;
}

static bool tpm2_is_active_pcr(struct tpms_pcr_selection *selection)
{
	int i;

	/*
	 * check the pcr_select. If at least one of the PCRs supports the
	 * algorithm add it on the active ones
	 */
	for (i = 0; i < selection->size_of_select; i++) {
		if (selection->pcr_select[i])
			return true;
	}

	return false;
}

int tpm2_get_pcr_info(struct udevice *dev, u32 *supported_pcr, u32 *active_pcr,
		      u32 *pcr_banks)
{
	u8 response[(sizeof(struct tpms_capability_data) -
		offsetof(struct tpms_capability_data, data))];
	struct tpml_pcr_selection pcrs;
	u32 num_pcr;
	size_t i;
	u32 ret;

	*supported_pcr = 0;
	*active_pcr = 0;
	*pcr_banks = 0;
	memset(response, 0, sizeof(response));
	ret = tpm2_get_capability(dev, TPM2_CAP_PCRS, 0, response, 1);
	if (ret)
		return ret;

	pcrs.count = get_unaligned_be32(response);
	/*
	 * We only support 5 algorithms for now so check against that
	 * instead of TPM2_NUM_PCR_BANKS
	 */
	if (pcrs.count > ARRAY_SIZE(tpm2_supported_algorithms) ||
	    pcrs.count < 1) {
		printf("%s: too many pcrs: %u\n", __func__, pcrs.count);
		return -EMSGSIZE;
	}

	ret = tpm2_get_num_pcr(dev, &num_pcr);
	if (ret)
		return ret;

	for (i = 0; i < pcrs.count; i++) {
		/*
		 * Definition of TPMS_PCR_SELECTION Structure
		 * hash: u16
		 * size_of_select: u8
		 * pcr_select: u8 array
		 *
		 * The offsets depend on the number of the device PCRs
		 * so we have to calculate them based on that
		 */
		u32 hash_offset = offsetof(struct tpml_pcr_selection, selection) +
			i * offsetof(struct tpms_pcr_selection, pcr_select) +
			i * ((num_pcr + 7) / 8);
		u32 size_select_offset =
			hash_offset + offsetof(struct tpms_pcr_selection,
					       size_of_select);
		u32 pcr_select_offset =
			hash_offset + offsetof(struct tpms_pcr_selection,
					       pcr_select);

		pcrs.selection[i].hash =
			get_unaligned_be16(response + hash_offset);
		pcrs.selection[i].size_of_select =
			__get_unaligned_be(response + size_select_offset);
		if (pcrs.selection[i].size_of_select > TPM2_PCR_SELECT_MAX) {
			printf("%s: pcrs selection too large: %u\n", __func__,
			       pcrs.selection[i].size_of_select);
			return -ENOBUFS;
		}
		/* copy the array of pcr_select */
		memcpy(pcrs.selection[i].pcr_select, response + pcr_select_offset,
		       pcrs.selection[i].size_of_select);
	}

	for (i = 0; i < pcrs.count; i++) {
		u32 hash_mask = tpm2_algorithm_to_mask(pcrs.selection[i].hash);

		if (hash_mask) {
			*supported_pcr |= hash_mask;
			if (tpm2_is_active_pcr(&pcrs.selection[i]))
				*active_pcr |= hash_mask;
		} else {
			printf("%s: unknown algorithm %x\n", __func__,
			       pcrs.selection[i].hash);
		}
	}

	*pcr_banks = pcrs.count;

	return 0;
}

u32 tpm2_dam_reset(struct udevice *dev, const char *pw, const ssize_t pw_sz)
{
	u8 command_v2[COMMAND_BUFFER_SIZE] = {
		tpm_u16(TPM2_ST_SESSIONS),	/* TAG */
		tpm_u32(27 + pw_sz),		/* Length */
		tpm_u32(TPM2_CC_DAM_RESET),	/* Command code */

		/* HANDLE */
		tpm_u32(TPM2_RH_LOCKOUT),	/* TPM resource handle */

		/* AUTH_SESSION */
		tpm_u32(9 + pw_sz),		/* Authorization size */
		tpm_u32(TPM2_RS_PW),		/* Session handle */
		tpm_u16(0),			/* Size of <nonce> */
						/* <nonce> (if any) */
		0,				/* Attributes: Cont/Excl/Rst */
		tpm_u16(pw_sz),			/* Size of <hmac/password> */
		/* STRING(pw)			   <hmac/password> (if any) */
	};
	unsigned int offset = 27;
	int ret;

	/*
	 * Fill the command structure starting from the first buffer:
	 *     - the password (if any)
	 */
	ret = pack_byte_string(command_v2, sizeof(command_v2), "s",
			       offset, pw, pw_sz);
	offset += pw_sz;
	if (ret)
		return TPM_LIB_ERROR;

	return tpm_sendrecv_command(dev, command_v2, NULL, NULL);
}

u32 tpm2_dam_parameters(struct udevice *dev, const char *pw,
			const ssize_t pw_sz, unsigned int max_tries,
			unsigned int recovery_time,
			unsigned int lockout_recovery)
{
	u8 command_v2[COMMAND_BUFFER_SIZE] = {
		tpm_u16(TPM2_ST_SESSIONS),	/* TAG */
		tpm_u32(27 + pw_sz + 12),	/* Length */
		tpm_u32(TPM2_CC_DAM_PARAMETERS), /* Command code */

		/* HANDLE */
		tpm_u32(TPM2_RH_LOCKOUT),	/* TPM resource handle */

		/* AUTH_SESSION */
		tpm_u32(9 + pw_sz),		/* Authorization size */
		tpm_u32(TPM2_RS_PW),		/* Session handle */
		tpm_u16(0),			/* Size of <nonce> */
						/* <nonce> (if any) */
		0,				/* Attributes: Cont/Excl/Rst */
		tpm_u16(pw_sz),			/* Size of <hmac/password> */
		/* STRING(pw)			   <hmac/password> (if any) */

		/* LOCKOUT PARAMETERS */
		/* tpm_u32(max_tries)		   Max tries (0, always lock) */
		/* tpm_u32(recovery_time)	   Recovery time (0, no lock) */
		/* tpm_u32(lockout_recovery)	   Lockout recovery */
	};
	unsigned int offset = 27;
	int ret;

	/*
	 * Fill the command structure starting from the first buffer:
	 *     - the password (if any)
	 *     - max tries
	 *     - recovery time
	 *     - lockout recovery
	 */
	ret = pack_byte_string(command_v2, sizeof(command_v2), "sddd",
			       offset, pw, pw_sz,
			       offset + pw_sz, max_tries,
			       offset + pw_sz + 4, recovery_time,
			       offset + pw_sz + 8, lockout_recovery);
	offset += pw_sz + 12;
	if (ret)
		return TPM_LIB_ERROR;

	return tpm_sendrecv_command(dev, command_v2, NULL, NULL);
}

int tpm2_change_auth(struct udevice *dev, u32 handle, const char *newpw,
		     const ssize_t newpw_sz, const char *oldpw,
		     const ssize_t oldpw_sz)
{
	unsigned int offset = 27;
	u8 command_v2[COMMAND_BUFFER_SIZE] = {
		tpm_u16(TPM2_ST_SESSIONS),	/* TAG */
		tpm_u32(offset + oldpw_sz + 2 + newpw_sz), /* Length */
		tpm_u32(TPM2_CC_HIERCHANGEAUTH), /* Command code */

		/* HANDLE */
		tpm_u32(handle),		/* TPM resource handle */

		/* AUTH_SESSION */
		tpm_u32(9 + oldpw_sz),		/* Authorization size */
		tpm_u32(TPM2_RS_PW),		/* Session handle */
		tpm_u16(0),			/* Size of <nonce> */
						/* <nonce> (if any) */
		0,				/* Attributes: Cont/Excl/Rst */
		tpm_u16(oldpw_sz)		/* Size of <hmac/password> */
		/* STRING(oldpw)		   <hmac/password> (if any) */

		/* TPM2B_AUTH (TPM2B_DIGEST) */
		/* tpm_u16(newpw_sz)		   Digest size, new pw length */
		/* STRING(newpw)		   Digest buffer, new pw */
	};
	int ret;

	/*
	 * Fill the command structure starting from the first buffer:
	 *     - the old password (if any)
	 *     - size of the new password
	 *     - new password
	 */
	ret = pack_byte_string(command_v2, sizeof(command_v2), "sws",
			       offset, oldpw, oldpw_sz,
			       offset + oldpw_sz, newpw_sz,
			       offset + oldpw_sz + 2, newpw, newpw_sz);
	offset += oldpw_sz + 2 + newpw_sz;
	if (ret)
		return TPM_LIB_ERROR;

	return tpm_sendrecv_command(dev, command_v2, NULL, NULL);
}

u32 tpm2_pcr_setauthpolicy(struct udevice *dev, const char *pw,
			   const ssize_t pw_sz, u32 index, const char *key)
{
	u8 command_v2[COMMAND_BUFFER_SIZE] = {
		tpm_u16(TPM2_ST_SESSIONS),	/* TAG */
		tpm_u32(35 + pw_sz + TPM2_DIGEST_LEN), /* Length */
		tpm_u32(TPM2_CC_PCR_SETAUTHPOL), /* Command code */

		/* HANDLE */
		tpm_u32(TPM2_RH_PLATFORM),	/* TPM resource handle */

		/* AUTH_SESSION */
		tpm_u32(9 + pw_sz),		/* Authorization size */
		tpm_u32(TPM2_RS_PW),		/* session handle */
		tpm_u16(0),			/* Size of <nonce> */
						/* <nonce> (if any) */
		0,				/* Attributes: Cont/Excl/Rst */
		tpm_u16(pw_sz)			/* Size of <hmac/password> */
		/* STRING(pw)			   <hmac/password> (if any) */

		/* TPM2B_AUTH (TPM2B_DIGEST) */
		/* tpm_u16(TPM2_DIGEST_LEN)	   Digest size length */
		/* STRING(key)			   Digest buffer (PCR key) */

		/* TPMI_ALG_HASH */
		/* tpm_u16(TPM2_ALG_SHA256)   Algorithm of the hash */

		/* TPMI_DH_PCR */
		/* tpm_u32(index),		   PCR Index */
	};
	unsigned int offset = 27;
	int ret;

	/*
	 * Fill the command structure starting from the first buffer:
	 *     - the password (if any)
	 *     - the PCR key length
	 *     - the PCR key
	 *     - the hash algorithm
	 *     - the PCR index
	 */
	ret = pack_byte_string(command_v2, sizeof(command_v2), "swswd",
			       offset, pw, pw_sz,
			       offset + pw_sz, TPM2_DIGEST_LEN,
			       offset + pw_sz + 2, key, TPM2_DIGEST_LEN,
			       offset + pw_sz + 2 + TPM2_DIGEST_LEN,
			       TPM2_ALG_SHA256,
			       offset + pw_sz + 4 + TPM2_DIGEST_LEN, index);
	offset += pw_sz + 2 + TPM2_DIGEST_LEN + 2 + 4;
	if (ret)
		return TPM_LIB_ERROR;

	return tpm_sendrecv_command(dev, command_v2, NULL, NULL);
}

u32 tpm2_pcr_setauthvalue(struct udevice *dev, const char *pw,
			  const ssize_t pw_sz, u32 index, const char *key,
			  const ssize_t key_sz)
{
	u8 command_v2[COMMAND_BUFFER_SIZE] = {
		tpm_u16(TPM2_ST_SESSIONS),	/* TAG */
		tpm_u32(33 + pw_sz + TPM2_DIGEST_LEN), /* Length */
		tpm_u32(TPM2_CC_PCR_SETAUTHVAL), /* Command code */

		/* HANDLE */
		tpm_u32(index),			/* Handle (PCR Index) */

		/* AUTH_SESSION */
		tpm_u32(9 + pw_sz),		/* Authorization size */
		tpm_u32(TPM2_RS_PW),		/* session handle */
		tpm_u16(0),			/* Size of <nonce> */
						/* <nonce> (if any) */
		0,				/* Attributes: Cont/Excl/Rst */
		tpm_u16(pw_sz),			/* Size of <hmac/password> */
		/* STRING(pw)			   <hmac/password> (if any) */

		/* TPM2B_DIGEST */
		/* tpm_u16(key_sz)		   Key length */
		/* STRING(key)			   Key */
	};
	unsigned int offset = 27;
	int ret;

	/*
	 * Fill the command structure starting from the first buffer:
	 *     - the password (if any)
	 *     - the number of digests, 1 in our case
	 *     - the algorithm, sha256 in our case
	 *     - the digest (64 bytes)
	 */
	ret = pack_byte_string(command_v2, sizeof(command_v2), "sws",
			       offset, pw, pw_sz,
			       offset + pw_sz, key_sz,
			       offset + pw_sz + 2, key, key_sz);
	offset += pw_sz + 2 + key_sz;
	if (ret)
		return TPM_LIB_ERROR;

	return tpm_sendrecv_command(dev, command_v2, NULL, NULL);
}

u32 tpm2_get_random(struct udevice *dev, void *data, u32 count)
{
	const u8 command_v2[10] = {
		tpm_u16(TPM2_ST_NO_SESSIONS),
		tpm_u32(12),
		tpm_u32(TPM2_CC_GET_RANDOM),
	};
	u8 buf[COMMAND_BUFFER_SIZE], response[COMMAND_BUFFER_SIZE];

	const size_t data_size_offset = 10;
	const size_t data_offset = 12;
	size_t response_length = sizeof(response);
	u32 data_size;
	u8 *out = data;

	while (count > 0) {
		u32 this_bytes = min((size_t)count,
				     sizeof(response) - data_offset);
		u32 err;

		if (pack_byte_string(buf, sizeof(buf), "sw",
				     0, command_v2, sizeof(command_v2),
				     sizeof(command_v2), this_bytes))
			return TPM_LIB_ERROR;
		err = tpm_sendrecv_command(dev, buf, response,
					   &response_length);
		if (err)
			return err;
		if (unpack_byte_string(response, response_length, "w",
				       data_size_offset, &data_size))
			return TPM_LIB_ERROR;
		if (data_size > this_bytes)
			return TPM_LIB_ERROR;
		if (unpack_byte_string(response, response_length, "s",
				       data_offset, out, data_size))
			return TPM_LIB_ERROR;

		count -= data_size;
		out += data_size;
	}

	return 0;
}

u32 tpm2_write_lock(struct udevice *dev, u32 index)
{
	u8 command_v2[COMMAND_BUFFER_SIZE] = {
		/* header 10 bytes */
		tpm_u16(TPM2_ST_SESSIONS),	/* TAG */
		tpm_u32(10 + 8 + 13), /* Length */
		tpm_u32(TPM2_CC_NV_WRITELOCK), /* Command code */

		/* handles 8 bytes */
		tpm_u32(TPM2_RH_PLATFORM),	/* Primary platform seed */
		tpm_u32(HR_NV_INDEX + index),	/* Password authorisation */

		/* session header 9 bytes */
		tpm_u32(9),			/* Header size */
		tpm_u32(TPM2_RS_PW),		/* Password authorisation */
		tpm_u16(0),			/* nonce_size */
		0,				/* session_attrs */
		tpm_u16(0),			/* auth_size */
	};

	return tpm_sendrecv_command(dev, command_v2, NULL, NULL);
}

u32 tpm2_disable_platform_hierarchy(struct udevice *dev)
{
	struct tpm_chip_priv *priv = dev_get_uclass_priv(dev);
	u8 command_v2[COMMAND_BUFFER_SIZE] = {
		/* header 10 bytes */
		tpm_u16(TPM2_ST_SESSIONS),	/* TAG */
		tpm_u32(10 + 4 + 13 + 5),	/* Length */
		tpm_u32(TPM2_CC_HIER_CONTROL),	/* Command code */

		/* 4 bytes */
		tpm_u32(TPM2_RH_PLATFORM),	/* Primary platform seed */

		/* session header 9 bytes */
		tpm_u32(9),			/* Header size */
		tpm_u32(TPM2_RS_PW),		/* Password authorisation */
		tpm_u16(0),			/* nonce_size */
		0,				/* session_attrs */
		tpm_u16(0),			/* auth_size */

		/* payload 5 bytes */
		tpm_u32(TPM2_RH_PLATFORM),	/* Hierarchy to disable */
		0,				/* 0=disable */
	};
	int ret;

	ret = tpm_sendrecv_command(dev, command_v2, NULL, NULL);
	log_info("ret=%s, %x\n", dev->name, ret);
	if (ret)
		return ret;

	priv->plat_hier_disabled = true;

	return 0;
}

u32 tpm2_submit_command(struct udevice *dev, const u8 *sendbuf,
			u8 *recvbuf, size_t *recv_size)
{
	return tpm_sendrecv_command(dev, sendbuf, recvbuf, recv_size);
}

u32 tpm2_report_state(struct udevice *dev, uint vendor_cmd, uint vendor_subcmd,
		      u8 *recvbuf, size_t *recv_size)
{
	u8 command_v2[COMMAND_BUFFER_SIZE] = {
		/* header 10 bytes */
		tpm_u16(TPM2_ST_NO_SESSIONS),		/* TAG */
		tpm_u32(10 + 2),			/* Length */
		tpm_u32(vendor_cmd),	/* Command code */

		tpm_u16(vendor_subcmd),
	};
	int ret;

	ret = tpm_sendrecv_command(dev, command_v2, recvbuf, recv_size);
	log_debug("ret=%s, %x\n", dev->name, ret);
	if (ret)
		return ret;
	if (*recv_size < 12)
		return -ENODATA;
	*recv_size -= 12;
	memcpy(recvbuf, recvbuf + 12, *recv_size);

	return 0;
}

u32 tpm2_enable_nvcommits(struct udevice *dev, uint vendor_cmd,
			  uint vendor_subcmd)
{
	u8 command_v2[COMMAND_BUFFER_SIZE] = {
		/* header 10 bytes */
		tpm_u16(TPM2_ST_NO_SESSIONS),		/* TAG */
		tpm_u32(10 + 2),			/* Length */
		tpm_u32(vendor_cmd),	/* Command code */

		tpm_u16(vendor_subcmd),
	};
	int ret;

	ret = tpm_sendrecv_command(dev, command_v2, NULL, NULL);
	log_debug("ret=%s, %x\n", dev->name, ret);
	if (ret)
		return ret;

	return 0;
}
