/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2020 Google LLC
 *
 * Modified from coreboot nhlt.h
 */

#ifndef _NHLT_H_
#define _NHLT_H_

struct acpi_ctx;
struct nhlt;
struct nhlt_endpoint;
struct nhlt_format;
struct nhlt_format_config;

/*
 * Non HD Audio ACPI support. This table is typically used for Intel Smart
 * Sound Technology DSP. It provides a way to encode opaque settings in
 * the ACPI tables.
 *
 * While the structure fields of the NHLT structs are exposed below
 * the SoC/chipset code should be the only other user manipulating the
 * fields directly aside from the library itself.
 *
 * The NHLT table consists of endpoints which in turn contain different
 * supporting stream formats. Each endpoint may contain a device specific
 * configuration payload as well as each stream format.
 *
 * Most code should use the SoC variants of the functions because
 * there is required logic needed to be performed by the SoC. The SoC
 * code should be abstracting the inner details of these functions that
 * specically apply to NHLT objects for that SoC.
 *
 * An example sequence:
 *
 * nhlt = nhlt_init()
 * ep = nhlt_add_endpoint()
 * nhlt_endpoint_append_config(ep)
 * nhlt_endpoint_add_formats(ep)
 * nhlt_soc_serialise()
 */

/* Obtain an nhlt object for adding endpoints. Returns NULL on error. */
struct nhlt *nhlt_init(void);

/* Return the size of the NHLT table including ACPI header. */
size_t nhlt_current_size(struct nhlt *nhlt);

/*
 * Helper functions for adding NHLT devices utilizing an nhlt_endp_descriptor
 * to drive the logic.
 */

struct nhlt_endp_descriptor {
	/* NHLT endpoint types. */
	int link;
	int device;
	int direction;
	u16 vid;
	u16 did;
	/* Optional endpoint specific configuration data. */
	const void *cfg;
	size_t cfg_size;
	/* Formats supported for endpoint. */
	const struct nhlt_format_config *formats;
	size_t num_formats;
};

/*
 * Add the number of endpoints described by each descriptor. The virtual bus
 * id for each descriptor is the default value of 0.
 * Returns < 0 on error, 0 on success.
 */
int nhlt_add_endpoints(struct nhlt *nhlt,
		       const struct nhlt_endp_descriptor *epds,
		       size_t num_epds);

/*
 * Add the number of endpoints associated with a single NHLT SSP instance id.
 * Each endpoint described in the endpoint descriptor array uses the provided
 * virtual bus id. Returns < 0 on error, 0 on success.
 */
int nhlt_add_ssp_endpoints(struct nhlt *nhlt, int virtual_bus_id,
			   const struct nhlt_endp_descriptor *epds,
			   size_t num_epds);

/*
 * Add endpoint to NHLT object. Returns NULL on error.
 *
 * generic nhlt_add_endpoint() is called by the SoC code to provide
 * the specific assumptions/uses for NHLT for that platform. All fields
 * are the NHLT enumerations found within this header file.
 */
struct nhlt_endpoint *nhlt_add_endpoint(struct nhlt *nhlt, int link_type,
					int device_type, int dir,
					u16 vid, u16 did);

/*
 * Append blob of configuration to the endpoint proper. Returns 0 on
 * success, < 0 on error. A copy of the configuration is made so any
 * resources pointed to by config can be freed after the call.
 */
int nhlt_endpoint_append_config(struct nhlt_endpoint *endpoint,
				const void *config, size_t config_sz);

/* Add a format type to the provided endpoint. Returns NULL on error. */
struct nhlt_format *nhlt_add_format(struct nhlt_endpoint *endpoint,
				    int num_channels, int sample_freq_khz,
				    int container_bits_per_sample,
				    int valid_bits_per_sample,
				    u32 speaker_mask);

/*
 * Append blob of configuration to the format proper. Returns 0 on
 * success, < 0 on error. A copy of the configuration is made so any
 * resources pointed to by config can be freed after the call.
 */
int nhlt_format_append_config(struct nhlt_format *format, const void *config,
			      size_t config_sz);

/*
 * Add num_formats described by formats to the endpoint. This function
 * effectively wraps nhlt_add_format() and nhlt_format_config() using the
 * data found in each nhlt_format_config object. Returns 0 on success, < 0
 * on error.
 */
int nhlt_endpoint_add_formats(struct nhlt_endpoint *endpoint,
			      const struct nhlt_format_config *formats,
			      size_t num_formats);

/*
 * Increment the instance id for a given link type. This function is
 * used for marking a device being completely added to the NHLT object.
 * Subsequent endpoints added to the nhlt object with the same link type
 * will use incremented instance id.
 */
void nhlt_next_instance(struct nhlt *nhlt, int link_type);

/*
 * Serialize NHLT object to ACPI table. Take in the beginning address of where
 * the table will reside oem_id and oem_table_id and return the address of the
 * next ACPI table. On error 0 will be returned. The NHLT object is no longer
 * valid after this function is called.
 */
int nhlt_serialise_oem_overrides(struct acpi_ctx *ctx, struct nhlt *nhlt,
				 const char *oem_id, const char *oem_table_id,
				 u32 oem_revision);

int nhlt_setup(struct nhlt *nhlt, ofnode node);

/* Link and device types. */
enum {
	NHLT_LINK_HDA,
	NHLT_LINK_DSP,
	NHLT_LINK_PDM,
	NHLT_LINK_SSP,
	NHLT_MAX_LINK_TYPES,
};

enum {
	NHLT_SSP_DEV_BT, /* Bluetooth */
	NHLT_SSP_DEV_MODEM,
	NHLT_SSP_DEV_FM,
	NHLT_SSP_DEV_RESERVED,
	NHLT_SSP_DEV_I2S = 4,
};

enum {
	NHLT_PDM_DEV,
};

/* Endpoint direction. */
enum {
	NHLT_DIR_RENDER,
	NHLT_DIR_CAPTURE,
	NHLT_DIR_BIDIRECTIONAL,
};

/*
 * Channel mask for an endpoint. While they are prefixed with 'SPEAKER' the
 * channel masks are also used for capture devices
 */
enum {
	SPEAKER_FRONT_LEFT =		BIT(0),
	SPEAKER_FRONT_RIGHT =		BIT(1),
	SPEAKER_FRONT_CENTER =		BIT(2),
	SPEAKER_LOW_FREQUENCY =		BIT(3),
	SPEAKER_BACK_LEFT =		BIT(4),
	SPEAKER_BACK_RIGHT =		BIT(5),
	SPEAKER_FRONT_LEFT_OF_CENTER =	BIT(6),
	SPEAKER_FRONT_RIGHT_OF_CENTER =	BIT(7),
	SPEAKER_BACK_CENTER =		BIT(8),
	SPEAKER_SIDE_LEFT =		BIT(9),
	SPEAKER_SIDE_RIGHT =		BIT(10),
	SPEAKER_TOP_CENTER =		BIT(11),
	SPEAKER_TOP_FRONT_LEFT =	BIT(12),
	SPEAKER_TOP_FRONT_CENTER =	BIT(13),
	SPEAKER_TOP_FRONT_RIGHT =	BIT(14),
	SPEAKER_TOP_BACK_LEFT =		BIT(15),
	SPEAKER_TOP_BACK_CENTER =	BIT(16),
	SPEAKER_TOP_BACK_RIGHT =	BIT(17),
};

/*
 * Supporting structures. Only SoC/chipset and the library code directly should
 * be manipulating these structures
 */
struct sub_format {
	u32 data1;
	u16 data2;
	u16 data3;
	u8 data4[8];
};

struct nhlt_specific_config {
	u32 size;
	void *capabilities;
};

struct nhlt_waveform {
	u16 tag;
	u16 num_channels;
	u32 samples_per_second;
	u32 bytes_per_second;
	u16 block_align;
	u16 bits_per_sample;
	u16 extra_size;
	u16 valid_bits_per_sample;
	u32 channel_mask;
	struct sub_format sub_format;
};

struct nhlt_format {
	struct nhlt_waveform waveform;
	struct nhlt_specific_config config;
};

/*
 * This struct is used by nhlt_endpoint_add_formats() for easily adding
 * waveform formats with associated settings file.
 */
struct nhlt_format_config {
	int num_channels;
	int sample_freq_khz;
	int container_bits_per_sample;
	int valid_bits_per_sample;
	u32 speaker_mask;
	const char *settings_file;
};

/* Arbitrary max number of formats per endpoint. */
#define MAX_FORMATS 2
struct nhlt_endpoint {
	u32 length;
	u8 link_type;
	u8 instance_id;
	u16 vendor_id;
	u16 device_id;
	u16 revision_id;
	u32 subsystem_id;
	u8 device_type;
	u8 direction;
	u8 virtual_bus_id;
	struct nhlt_specific_config config;
	u8 num_formats;
	struct nhlt_format formats[MAX_FORMATS];
};

#define MAX_ENDPOINTS 8
struct nhlt {
	u32 subsystem_id;
	u8 num_endpoints;
	struct nhlt_endpoint endpoints[MAX_ENDPOINTS];
	u8 current_instance_id[NHLT_MAX_LINK_TYPES];
};

struct nhlt_tdm_config {
	u8 virtual_slot;
	u8 config_type;
};

enum {
	NHLT_TDM_BASIC,
	NHLT_TDM_MIC_ARRAY,
};

struct nhlt_dmic_array_config {
	struct nhlt_tdm_config tdm_config;
	u8 array_type;
};

/*
 * Microphone array definitions may be found here:
 * https://msdn.microsoft.com/en-us/library/windows/hardware/dn613960%28v=vs.85%29.aspx
 */
enum {
	NHLT_MIC_ARRAY_2CH_SMALL = 0xa,
	NHLT_MIC_ARRAY_2CH_BIG = 0xb,
	NHLT_MIC_ARRAY_4CH_1ST_GEOM = 0xc,
	NHLT_MIC_ARRAY_4CH_L_SHAPED = 0xd,
	NHLT_MIC_ARRAY_4CH_2ND_GEOM = 0xe,
	NHLT_MIC_ARRAY_VENDOR_DEFINED = 0xf,
};

#endif
