// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2020 Google LLC
 *
 * Modified from coreboot nhlt.c
 */

#define LOG_CATEGORY	LOGC_ACPI

#include <common.h>
#include <binman.h>
#include <dm.h>
#include <log.h>
#include <malloc.h>
#include <tables_csum.h>
#include <acpi/acpi_table.h>
#include <asm/acpi_nhlt.h>
#include <asm/unaligned.h>
#include <dm/acpi.h>

#define NHLT_RID		1
#define NHLT_SSID		1
#define WAVEFORMAT_TAG		0xfffe
#define DEFAULT_VIRTUAL_BUS_ID	0

static const struct sub_format pcm_subformat = {
	.data1 = 0x00000001,
	.data2 = 0x0000,
	.data3 = 0x0010,
	.data4 = { 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71 },
};

struct nhlt *nhlt_init(void)
{
	struct nhlt *nhlt;

	nhlt = malloc(sizeof(*nhlt));

	if (!nhlt)
		return NULL;

	memset(nhlt, 0, sizeof(*nhlt));
	nhlt->subsystem_id = NHLT_SSID;

	return nhlt;
}

struct nhlt_endpoint *nhlt_add_endpoint(struct nhlt *nhlt, int link_type,
					int device_type, int dir,
					u16 vid, u16 did)
{
	struct nhlt_endpoint *endp;

	if (link_type < NHLT_LINK_HDA || link_type >= NHLT_MAX_LINK_TYPES)
		return NULL;

	if (nhlt->num_endpoints >= MAX_ENDPOINTS)
		return NULL;

	endp = &nhlt->endpoints[nhlt->num_endpoints];

	endp->link_type = link_type;
	endp->instance_id = nhlt->current_instance_id[link_type];
	endp->vendor_id = vid;
	endp->device_id = did;
	endp->revision_id = NHLT_RID;
	endp->subsystem_id = nhlt->subsystem_id;
	endp->device_type = device_type;
	endp->direction = dir;
	endp->virtual_bus_id = DEFAULT_VIRTUAL_BUS_ID;
	endp->num_formats = 0;

	nhlt->num_endpoints++;

	return endp;
}

static int append_specific_config(struct nhlt_specific_config *spec_cfg,
				  const void *config, size_t config_sz)
{
	size_t new_sz;
	void *new_cfg;

	new_sz = spec_cfg->size + config_sz;
	new_cfg = malloc(new_sz);
	if (!new_cfg)
		return -ENOMEM;

	/* Append new config */
	memcpy(new_cfg, spec_cfg->capabilities, spec_cfg->size);
	memcpy(new_cfg + spec_cfg->size, config, config_sz);

	free(spec_cfg->capabilities);

	/* Update with new config data */
	spec_cfg->size = new_sz;
	spec_cfg->capabilities = new_cfg;

	return 0;
}

int nhlt_endpoint_append_config(struct nhlt_endpoint *endp, const void *config,
				size_t config_sz)
{
	return append_specific_config(&endp->config, config, config_sz);
}

struct nhlt_format *nhlt_add_format(struct nhlt_endpoint *endp,
				    int num_channels, int sample_freq_khz,
				    int container_bits_per_sample,
				    int valid_bits_per_sample,
				    uint32_t speaker_mask)
{
	struct nhlt_format *fmt;
	struct nhlt_waveform *wave;

	if (endp->num_formats >= MAX_FORMATS)
		return NULL;

	fmt = &endp->formats[endp->num_formats];
	wave = &fmt->waveform;

	wave->tag = WAVEFORMAT_TAG;
	wave->num_channels = num_channels;
	wave->samples_per_second = sample_freq_khz * 1000;
	wave->bits_per_sample = container_bits_per_sample;
	wave->extra_size = sizeof(wave->valid_bits_per_sample);
	wave->extra_size += sizeof(wave->channel_mask);
	wave->extra_size += sizeof(wave->sub_format);
	wave->valid_bits_per_sample = valid_bits_per_sample;
	wave->channel_mask = speaker_mask;
	memcpy(&wave->sub_format, &pcm_subformat, sizeof(wave->sub_format));

	/* Calculate the dervied fields */
	wave->block_align = wave->num_channels * wave->bits_per_sample / 8;
	wave->bytes_per_second = wave->block_align * wave->samples_per_second;

	endp->num_formats++;

	return fmt;
}

int nhlt_format_append_config(struct nhlt_format *fmt, const void *config,
			      size_t config_sz)
{
	return append_specific_config(&fmt->config, config, config_sz);
}

int nhlt_endpoint_add_formats(struct nhlt_endpoint *endp,
			      const struct nhlt_format_config *formats,
			      size_t num_formats)
{
	ofnode node;
	size_t i;

	node = binman_section_find_node("private-files");

	for (i = 0; i < num_formats; i++) {
		const struct nhlt_format_config *cfg = &formats[i];
		struct nhlt_format *fmt;
		void *data;
		int size;
		int ret;

		fmt = nhlt_add_format(endp, cfg->num_channels,
				      cfg->sample_freq_khz,
				      cfg->container_bits_per_sample,
				      cfg->valid_bits_per_sample,
				      cfg->speaker_mask);
		if (!fmt)
			return -ENOSPC;

		if (!cfg->settings_file)
			continue;

		ret = binman_entry_map(node, cfg->settings_file, &data, &size);
		if (ret) {
			log_warning("Failed to find settings file %s\n",
				    cfg->settings_file);
			return log_msg_ret("settings", ret);
		}

		ret = nhlt_format_append_config(fmt, data, size);
		if (ret)
			return log_msg_ret("append", ret);
	}

	return 0;
}

void nhlt_next_instance(struct nhlt *nhlt, int link_type)
{
	if (link_type < NHLT_LINK_HDA || link_type >= NHLT_MAX_LINK_TYPES)
		return;

	nhlt->current_instance_id[link_type]++;
}

static size_t calc_specific_config_size(struct nhlt_specific_config *cfg)
{
	return sizeof(cfg->size) + cfg->size;
}

static size_t calc_format_size(struct nhlt_format *fmt)
{
	size_t sz = 0;

	/* Wave format first */
	sz += sizeof(fmt->waveform.tag);
	sz += sizeof(fmt->waveform.num_channels);
	sz += sizeof(fmt->waveform.samples_per_second);
	sz += sizeof(fmt->waveform.bytes_per_second);
	sz += sizeof(fmt->waveform.block_align);
	sz += sizeof(fmt->waveform.bits_per_sample);
	sz += sizeof(fmt->waveform.extra_size);
	sz += sizeof(fmt->waveform.valid_bits_per_sample);
	sz += sizeof(fmt->waveform.channel_mask);
	sz += sizeof(fmt->waveform.sub_format);

	sz += calc_specific_config_size(&fmt->config);

	return sz;
}

static size_t calc_endpoint_size(struct nhlt_endpoint *endp)
{
	int i;
	size_t sz = 0;

	sz += sizeof(endp->length) + sizeof(endp->link_type);
	sz += sizeof(endp->instance_id) + sizeof(endp->vendor_id);
	sz += sizeof(endp->device_id) + sizeof(endp->revision_id);
	sz += sizeof(endp->subsystem_id) + sizeof(endp->device_type);
	sz += sizeof(endp->direction) + sizeof(endp->virtual_bus_id);
	sz += calc_specific_config_size(&endp->config);
	sz += sizeof(endp->num_formats);

	for (i = 0; i < endp->num_formats; i++)
		sz += calc_format_size(&endp->formats[i]);

	/* Adjust endpoint length to reflect current configuration */
	endp->length = sz;

	return sz;
}

static size_t calc_endpoints_size(struct nhlt *nhlt)
{
	size_t sz = 0;
	int i;

	for (i = 0; i < nhlt->num_endpoints; i++)
		sz += calc_endpoint_size(&nhlt->endpoints[i]);

	return sz;
}

static size_t calc_size(struct nhlt *nhlt)
{
	return sizeof(nhlt->num_endpoints) + calc_endpoints_size(nhlt);
}

size_t nhlt_current_size(struct nhlt *nhlt)
{
	return calc_size(nhlt) + sizeof(struct acpi_table_header);
}

static void nhlt_free_resources(struct nhlt *nhlt)
{
	int i, j;

	/* Free all specific configs */
	for (i = 0; i < nhlt->num_endpoints; i++) {
		struct nhlt_endpoint *endp = &nhlt->endpoints[i];

		free(endp->config.capabilities);
		for (j = 0; j < endp->num_formats; j++) {
			struct nhlt_format *fmt = &endp->formats[j];

			free(fmt->config.capabilities);
		}
	}

	/* Free nhlt object proper */
	free(nhlt);
}

struct cursor {
	u8 *start;
	u8 *buf;
};

static void ser8(struct cursor *cur, uint val)
{
	*cur->buf = val;
	cur->buf += sizeof(u8);
}

static void ser16(struct cursor *cur, uint val)
{
	put_unaligned_le16(val, cur->buf);
	cur->buf += sizeof(u16);
}

static void ser32(struct cursor *cur, uint val)
{
	put_unaligned_le32(val, cur->buf);
	cur->buf += sizeof(u32);
}

static void serblob(struct cursor *cur, void *from, size_t sz)
{
	memcpy(cur->buf, from, sz);
	cur->buf += sz;
}

static void serialise_specific_config(struct nhlt_specific_config *cfg,
				      struct cursor *cur)
{
	log_debug("%zx\n", cur->buf - cur->start);
	ser32(cur, cfg->size);
	serblob(cur, cfg->capabilities, cfg->size);
}

static void serialise_waveform(struct nhlt_waveform *wave, struct cursor *cur)
{
	log_debug("%zx\n", cur->buf - cur->start);
	ser16(cur, wave->tag);
	ser16(cur, wave->num_channels);
	ser32(cur, wave->samples_per_second);
	ser32(cur, wave->bytes_per_second);
	ser16(cur, wave->block_align);
	ser16(cur, wave->bits_per_sample);
	ser16(cur, wave->extra_size);
	ser16(cur, wave->valid_bits_per_sample);
	ser32(cur, wave->channel_mask);
	ser32(cur, wave->sub_format.data1);
	ser16(cur, wave->sub_format.data2);
	ser16(cur, wave->sub_format.data3);
	serblob(cur, wave->sub_format.data4, sizeof(wave->sub_format.data4));
}

static void serialise_format(struct nhlt_format *fmt, struct cursor *cur)
{
	log_debug("%zx\n", cur->buf - cur->start);
	serialise_waveform(&fmt->waveform, cur);
	serialise_specific_config(&fmt->config, cur);
}

static void serialise_endpoint(struct nhlt_endpoint *endp, struct cursor *cur)
{
	int i;

	log_debug("%zx\n", cur->buf - cur->start);
	ser32(cur, endp->length);
	ser8(cur, endp->link_type);
	ser8(cur, endp->instance_id);
	ser16(cur, endp->vendor_id);
	ser16(cur, endp->device_id);
	ser16(cur, endp->revision_id);
	ser32(cur, endp->subsystem_id);
	ser8(cur, endp->device_type);
	ser8(cur, endp->direction);
	ser8(cur, endp->virtual_bus_id);
	serialise_specific_config(&endp->config, cur);
	ser8(cur, endp->num_formats);

	for (i = 0; i < endp->num_formats; i++)
		serialise_format(&endp->formats[i], cur);
}

static void nhlt_serialise_endpoints(struct nhlt *nhlt, struct cursor *cur)
{
	int i;

	ser8(cur, nhlt->num_endpoints);

	for (i = 0; i < nhlt->num_endpoints; i++)
		serialise_endpoint(&nhlt->endpoints[i], cur);
}

int nhlt_serialise_oem_overrides(struct acpi_ctx *ctx, struct nhlt *nhlt,
				 const char *oem_id, const char *oem_table_id,
				 uint32_t oem_revision)
{
	struct cursor cur;
	struct acpi_table_header *header;
	size_t sz;
	size_t oem_id_len;
	size_t oem_table_id_len;
	int ret;

	log_debug("ACPI:    * NHLT\n");
	sz = nhlt_current_size(nhlt);

	/* Create header */
	header = (void *)ctx->current;
	memset(header, '\0', sizeof(struct acpi_table_header));
	memcpy(header->signature, "NHLT", 4);
	header->length = sz;
	header->revision = acpi_get_table_revision(ACPITAB_NHLT);

	if (oem_id) {
		oem_id_len = min((int)strlen(oem_id), 6);
		memcpy(header->oem_id, oem_id, oem_id_len);
	}
	if (oem_table_id) {
		oem_table_id_len = min((int)strlen(oem_table_id), 8);
		memcpy(header->oem_table_id, oem_table_id, oem_table_id_len);
	}
	header->oem_revision = oem_revision;
	memcpy(header->aslc_id, ASLC_ID, 4);

	cur.buf = (void *)(header + 1);
	cur.start = (void *)header;
	nhlt_serialise_endpoints(nhlt, &cur);

	header->checksum = table_compute_checksum(header, sz);
	nhlt_free_resources(nhlt);
	assert(cur.buf - cur.start == sz);

	ret = acpi_add_table(ctx, ctx->current);
	if (ret)
		return log_msg_ret("add", ret);
	acpi_inc_align(ctx, sz);

	return 0;
}

static int _nhlt_add_single_endpoint(struct nhlt *nhlt, int virtual_bus_id,
				     const struct nhlt_endp_descriptor *epd)
{
	struct nhlt_endpoint *endp;
	int ret;

	endp = nhlt_add_endpoint(nhlt, epd->link, epd->device, epd->direction,
				 epd->vid, epd->did);
	if (!endp)
		return -EINVAL;

	endp->virtual_bus_id = virtual_bus_id;

	ret = nhlt_endpoint_append_config(endp, epd->cfg, epd->cfg_size);
	if (ret)
		return ret;

	ret = nhlt_endpoint_add_formats(endp, epd->formats, epd->num_formats);
	if (ret)
		return log_msg_ret("formats", ret);

	return 0;
}

static int _nhlt_add_endpoints(struct nhlt *nhlt, int virtual_bus_id,
			       const struct nhlt_endp_descriptor *epds,
			       size_t num_epds)
{
	size_t i;
	int ret;

	for (i = 0; i < num_epds; i++) {
		ret = _nhlt_add_single_endpoint(nhlt, virtual_bus_id, &epds[i]);
		if (ret)
			return log_ret(ret);
	}

	return 0;
}

int nhlt_add_endpoints(struct nhlt *nhlt,
		       const struct nhlt_endp_descriptor *epds, size_t num_epds)
{
	int ret;

	ret = _nhlt_add_endpoints(nhlt, DEFAULT_VIRTUAL_BUS_ID, epds, num_epds);

	return ret;
}

int nhlt_add_ssp_endpoints(struct nhlt *nhlt, int virtual_bus_id,
			   const struct nhlt_endp_descriptor *epds,
			   size_t num_epds)
{
	int ret;

	ret = _nhlt_add_endpoints(nhlt, virtual_bus_id, epds, num_epds);
	if (!ret)
		nhlt_next_instance(nhlt, NHLT_LINK_SSP);

	return ret;
}
