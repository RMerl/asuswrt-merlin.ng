// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2020 B&R Industrial Automation GmbH - http://www.br-automation.com
 */

#include <common.h>
#include <dm.h>
#include <log.h>
#include <asm/arch/fsp_bindings.h>

/**
 * read_u8_prop() - Read an u8 property from devicetree (scalar or array)
 * @node:  Valid node reference to read property from
 * @name:  Name of the property to read from
 * @count: If the property is expected to be an array, this is the
 *         number of expected elements
 *         Set to 0 if the property is expected to be a scalar
 * @dst:   Pointer to destination of where to save the value(s) read
 *         from devicetree
 */
static void read_u8_prop(ofnode node, char *name, size_t count, u8 *dst)
{
	u32 tmp;
	const u8 *buf;
	int ret;

	if (count == 0) {
		ret = ofnode_read_u32(node, name, &tmp);
		if (ret == 0)
			*dst = tmp;
	} else {
		buf = ofnode_read_u8_array_ptr(node, name, count);
		if (buf)
			memcpy(dst, buf, count);
	}
}

/**
 * read_u16_prop() - Read an u16 property from devicetree (scalar or array)
 * @node:  Valid node reference to read property from
 * @name:  Name of the property to read from
 * @count: If the property is expected to be an array, this is the
 *         number of expected elements
 *         Set to 0 if the property is expected to be a scalar
 * @dst:   Pointer to destination of where to save the value(s) read
 *         from devicetree
 * Return: 0 on success, -ve on error
 */
static int read_u16_prop(ofnode node, char *name, size_t count, u16 *dst)
{
	u32 tmp;
	u32 buf[32];
	int ret;

	if (ARRAY_SIZE(buf) < count) {
		debug("ERROR: %s buffer to small!\n", __func__);
		return -ENOSPC;
	}

	if (count == 0) {
		ret = ofnode_read_u32(node, name, &tmp);
		if (ret == 0)
			*dst = tmp;
	} else {
		ret = ofnode_read_u32_array(node, name, buf, count);
		if (ret == 0)
			for (int i = 0; i < count; i++)
				dst[i] = buf[i];
	}

	return 0;
}

/**
 * read_u32_prop() - Read an u32 property from devicetree (scalar or array)
 * @node:  Valid node reference to read property from
 * @name:  Name of the property to read from
 * @count: If the property is expected to be an array, this is the
 *         number of expected elements
 *         set to 0 if the property is expected to be a scalar
 * @dst:   Pointer to destination of where to save the value(s) read
 *         from devicetree
 */
static void read_u32_prop(ofnode node, char *name, size_t count, u32 *dst)
{
	if (count == 0)
		ofnode_read_u32(node, name, dst);
	else
		ofnode_read_u32_array(node, name, dst, count);
}

/**
 * read_u64_prop() - Read an u64 property from devicetree (scalar or array)
 * @node:  Valid node reference to read property from
 * @name:  Name of the property to read from
 * @count: If the property is expected to be an array, this is the
 *         number of expected elements
 *         set to 0 if the property is expected to be a scalar
 * @dst:   Pointer to destination of where to save the value(s) read
 *         from devicetree
 */
static int read_u64_prop(ofnode node, char *name, size_t count, u64 *dst)
{
	if (count == 0) {
		ofnode_read_u64(node, name, dst);
	} else {
		debug("ERROR: %s u64 arrays not supported!\n", __func__);
		return -EINVAL;
	}

	return 0;
}

/**
 * read_string_prop() - Read a string property from devicetree
 * @node:  Valid node reference to read property from
 * @name:  Name of the property to read from
 * @count: Size of the destination buffer
 * @dst:   Pointer to destination of where to save the values read
 *         from devicetree
 */
static void read_string_prop(ofnode node, char *name, size_t count, char *dst)
{
	const char *string_buf;

	if (count > 0) {
		string_buf = ofnode_read_string(node, name);
		if (string_buf)
			strlcpy(dst, string_buf, count);
	}
}

/**
 * read_swizzle_prop() - Read a swizzle property from devicetree
 * @node:  Valid node reference to read property from
 * @name:  Name of the property to read from
 * @count: Number of elements in the swizzle configuration
 * @dst:   pointer to destination of where to save the values read
 *         from devicetree
 */
static void read_swizzle_prop(ofnode node, char *name, size_t count, u8 *dst)
{
	const struct lpddr4_chan_swizzle_cfg *sch;
	/* Number of bytes to copy per DQS */
	const size_t sz = DQ_BITS_PER_DQS;
	const struct lpddr4_swizzle_cfg *swizzle_cfg;

	swizzle_cfg = (const struct lpddr4_swizzle_cfg *)
			ofnode_read_u8_array_ptr(node, name, count);

	if (!swizzle_cfg)
		return;
	/*
	 * CH0_DQB byte lanes in the bit swizzle configuration field are
	 * not 1:1. The mapping within the swizzling field is:
	 *   indices [0:7]   - byte lane 1 (DQS1) DQ[8:15]
	 *   indices [8:15]  - byte lane 0 (DQS0) DQ[0:7]
	 *   indices [16:23] - byte lane 3 (DQS3) DQ[24:31]
	 *   indices [24:31] - byte lane 2 (DQS2) DQ[16:23]
	 */
	sch = &swizzle_cfg->phys[LP4_PHYS_CH0B];
	memcpy(&dst[0 * DQ_BITS_PER_DQS], &sch->dqs[LP4_DQS1], sz);
	memcpy(&dst[1 * DQ_BITS_PER_DQS], &sch->dqs[LP4_DQS0], sz);
	memcpy(&dst[2 * DQ_BITS_PER_DQS], &sch->dqs[LP4_DQS3], sz);
	memcpy(&dst[3 * DQ_BITS_PER_DQS], &sch->dqs[LP4_DQS2], sz);

	/*
	 * CH0_DQA byte lanes in the bit swizzle configuration field are 1:1.
	 */
	sch = &swizzle_cfg->phys[LP4_PHYS_CH0A];
	memcpy(&dst[4 * DQ_BITS_PER_DQS], &sch->dqs[LP4_DQS0], sz);
	memcpy(&dst[5 * DQ_BITS_PER_DQS], &sch->dqs[LP4_DQS1], sz);
	memcpy(&dst[6 * DQ_BITS_PER_DQS], &sch->dqs[LP4_DQS2], sz);
	memcpy(&dst[7 * DQ_BITS_PER_DQS], &sch->dqs[LP4_DQS3], sz);

	sch = &swizzle_cfg->phys[LP4_PHYS_CH1B];
	memcpy(&dst[8 * DQ_BITS_PER_DQS], &sch->dqs[LP4_DQS1], sz);
	memcpy(&dst[9 * DQ_BITS_PER_DQS], &sch->dqs[LP4_DQS0], sz);
	memcpy(&dst[10 * DQ_BITS_PER_DQS], &sch->dqs[LP4_DQS3], sz);
	memcpy(&dst[11 * DQ_BITS_PER_DQS], &sch->dqs[LP4_DQS2], sz);

	/*
	 * CH0_DQA byte lanes in the bit swizzle configuration field are 1:1.
	 */
	sch = &swizzle_cfg->phys[LP4_PHYS_CH1A];
	memcpy(&dst[12 * DQ_BITS_PER_DQS], &sch->dqs[LP4_DQS0], sz);
	memcpy(&dst[13 * DQ_BITS_PER_DQS], &sch->dqs[LP4_DQS1], sz);
	memcpy(&dst[14 * DQ_BITS_PER_DQS], &sch->dqs[LP4_DQS2], sz);
	memcpy(&dst[15 * DQ_BITS_PER_DQS], &sch->dqs[LP4_DQS3], sz);
}

/**
 * fsp_update_config_from_dtb() - Read FSP config from devicetree node
 * @node: Valid node reference to read property from
 * @cfg:  Pointer to FSP config structure
 * @fsp_bindings: Binding describing which devicetree properties should
 *                be stored where in the FSP configuration structure
 *                The end of the list is declared by a NULL pointer in propname
 * Return: 0 on success, -ve on error
 *
 * This function reads the configuration for FSP from the provided
 * devicetree node and saves it in the FSP configuration structure.
 * Configuration options that are not present in the devicetree are
 * left at their current value.
 */
__maybe_unused
static int fsp_update_config_from_dtb(ofnode node, u8 *cfg,
				      const struct fsp_binding *fsp_bindings)
{
	const struct fsp_binding *fspb;
	int ret;

	for (int i = 0; fsp_bindings[i].propname; i++) {
		fspb = &fsp_bindings[i];

		switch (fspb->type) {
		case FSP_UINT8:
			read_u8_prop(node, fspb->propname, fspb->count,
				     &cfg[fspb->offset]);
		break;
		case FSP_UINT16:
			ret = read_u16_prop(node, fspb->propname, fspb->count,
					    (u16 *)&cfg[fspb->offset]);
			if (ret)
				return ret;
		break;
		case FSP_UINT32:
			read_u32_prop(node, fspb->propname, fspb->count,
				      (u32 *)&cfg[fspb->offset]);
		break;
		case FSP_UINT64:
			ret = read_u64_prop(node, fspb->propname, fspb->count,
				      (u64 *)&cfg[fspb->offset]);
			if (ret)
				return ret;
		break;
		case FSP_STRING:
			read_string_prop(node, fspb->propname, fspb->count,
					 (char *)&cfg[fspb->offset]);
		break;
		case FSP_LPDDR4_SWIZZLE:
			read_swizzle_prop(node, fspb->propname, fspb->count,
					  &cfg[fspb->offset]);
		break;
		}
	}

	return 0;
}

#if defined(CONFIG_SPL_BUILD)
const struct fsp_binding fsp_m_bindings[] = {
	{
	.type = FSP_UINT32,
	.offset = offsetof(struct fsp_m_config, serial_debug_port_address),
	.propname = "fspm,serial-debug-port-address",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, serial_debug_port_type),
	.propname = "fspm,serial-debug-port-type",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, serial_debug_port_device),
	.propname = "fspm,serial-debug-port-device",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, serial_debug_port_stride_size),
	.propname = "fspm,serial-debug-port-stride-size",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, mrc_fast_boot),
	.propname = "fspm,mrc-fast-boot",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, igd),
	.propname = "fspm,igd",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, igd_dvmt50_pre_alloc),
	.propname = "fspm,igd-dvmt50-pre-alloc",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, igd_aperture_size),
	.propname = "fspm,igd-aperture-size",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, gtt_size),
	.propname = "fspm,gtt-size",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, primary_video_adaptor),
	.propname = "fspm,primary-video-adaptor",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, package),
	.propname = "fspm,package",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, profile),
	.propname = "fspm,profile",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, memory_down),
	.propname = "fspm,memory-down",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, ddr3_l_page_size),
	.propname = "fspm,ddr3-l-page-size",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, ddr3_lasr),
	.propname = "fspm,ddr3-lasr",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, scrambler_support),
	.propname = "fspm,scrambler-support",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, interleaved_mode),
	.propname = "fspm,interleaved-mode",
	}, {
	.type = FSP_UINT16,
	.offset = offsetof(struct fsp_m_config, channel_hash_mask),
	.propname = "fspm,channel-hash-mask",
	}, {
	.type = FSP_UINT16,
	.offset = offsetof(struct fsp_m_config, slice_hash_mask),
	.propname = "fspm,slice-hash-mask",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, channels_slices_enable),
	.propname = "fspm,channels-slices-enable",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, min_ref_rate2x_enable),
	.propname = "fspm,min-ref-rate2x-enable",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, dual_rank_support_enable),
	.propname = "fspm,dual-rank-support-enable",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, rmt_mode),
	.propname = "fspm,rmt-mode",
	}, {
	.type = FSP_UINT16,
	.offset = offsetof(struct fsp_m_config, memory_size_limit),
	.propname = "fspm,memory-size-limit",
	}, {
	.type = FSP_UINT16,
	.offset = offsetof(struct fsp_m_config, low_memory_max_value),
	.propname = "fspm,low-memory-max-value",
	}, {
	.type = FSP_UINT16,
	.offset = offsetof(struct fsp_m_config, high_memory_max_value),
	.propname = "fspm,high-memory-max-value",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, disable_fast_boot),
	.propname = "fspm,disable-fast-boot",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, dimm0_spd_address),
	.propname = "fspm,dimm0-spd-address",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, dimm1_spd_address),
	.propname = "fspm,dimm1-spd-address",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, chan[0].rank_enable),
	.propname = "fspm,ch0-rank-enable",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, chan[0].device_width),
	.propname = "fspm,ch0-device-width",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, chan[0].dram_density),
	.propname = "fspm,ch0-dram-density",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, chan[0].option),
	.propname = "fspm,ch0-option",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, chan[0].odt_config),
	.propname = "fspm,ch0-odt-config",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, chan[0].tristate_clk1),
	.propname = "fspm,ch0-tristate-clk1",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, chan[0].mode2_n),
	.propname = "fspm,ch0-mode2-n",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, chan[0].odt_levels),
	.propname = "fspm,ch0-odt-levels",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, chan[1].rank_enable),
	.propname = "fspm,ch1-rank-enable",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, chan[1].device_width),
	.propname = "fspm,ch1-device-width",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, chan[1].dram_density),
	.propname = "fspm,ch1-dram-density",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, chan[1].option),
	.propname = "fspm,ch1-option",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, chan[1].odt_config),
	.propname = "fspm,ch1-odt-config",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, chan[1].tristate_clk1),
	.propname = "fspm,ch1-tristate-clk1",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, chan[1].mode2_n),
	.propname = "fspm,ch1-mode2-n",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, chan[1].odt_levels),
	.propname = "fspm,ch1-odt-levels",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, chan[2].rank_enable),
	.propname = "fspm,ch2-rank-enable",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, chan[2].device_width),
	.propname = "fspm,ch2-device-width",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, chan[2].dram_density),
	.propname = "fspm,ch2-dram-density",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, chan[2].option),
	.propname = "fspm,ch2-option",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, chan[2].odt_config),
	.propname = "fspm,ch2-odt-config",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, chan[2].tristate_clk1),
	.propname = "fspm,ch2-tristate-clk1",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, chan[2].mode2_n),
	.propname = "fspm,ch2-mode2-n",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, chan[2].odt_levels),
	.propname = "fspm,ch2-odt-levels",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, chan[3].rank_enable),
	.propname = "fspm,ch3-rank-enable",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, chan[3].device_width),
	.propname = "fspm,ch3-device-width",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, chan[3].dram_density),
	.propname = "fspm,ch3-dram-density",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, chan[3].option),
	.propname = "fspm,ch3-option",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, chan[3].odt_config),
	.propname = "fspm,ch3-odt-config",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, chan[3].tristate_clk1),
	.propname = "fspm,ch3-tristate-clk1",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, chan[3].mode2_n),
	.propname = "fspm,ch3-mode2-n",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, chan[3].odt_levels),
	.propname = "fspm,ch3-odt-levels",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, rmt_check_run),
	.propname = "fspm,rmt-check-run",
	}, {
	.type = FSP_UINT16,
	.offset = offsetof(struct fsp_m_config,
			   rmt_margin_check_scale_high_threshold),
	.propname = "fspm,rmt-margin-check-scale-high-threshold",
	}, {
	.type = FSP_LPDDR4_SWIZZLE,
	.offset = offsetof(struct fsp_m_config, ch_bit_swizzling),
	.propname = "fspm,ch-bit-swizzling",
	.count = SIZE_OF_MEMBER(struct fsp_m_config, ch_bit_swizzling) /
		 SIZE_OF_MEMBER(struct fsp_m_config, ch_bit_swizzling[0][0])
	}, {
	.type = FSP_UINT32,
	.offset = offsetof(struct fsp_m_config, msg_level_mask),
	.propname = "fspm,msg-level-mask",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, pre_mem_gpio_table_pin_num),
	.propname = "fspm,pre-mem-gpio-table-pin-num",
	.count = ARRAY_SIZE_OF_MEMBER(struct fsp_m_config,
				      pre_mem_gpio_table_pin_num),
	}, {
	.type = FSP_UINT32,
	.offset = offsetof(struct fsp_m_config, pre_mem_gpio_table_ptr),
	.propname = "fspm,pre-mem-gpio-table-ptr",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, pre_mem_gpio_table_entry_num),
	.propname = "fspm,pre-mem-gpio-table-entry-num",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, enhance_port8xh_decoding),
	.propname = "fspm,enhance-port8xh-decoding",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, spd_write_enable),
	.propname = "fspm,spd-write-enable",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, mrc_data_saving),
	.propname = "fspm,mrc-data-saving",
	}, {
	.type = FSP_UINT32,
	.offset = offsetof(struct fsp_m_config, oem_loading_base),
	.propname = "fspm,oem-loading-base",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, oem_file_name),
	.propname = "fspm,oem-file-name",
	.count = ARRAY_SIZE_OF_MEMBER(struct fsp_m_config, oem_file_name),
	}, {
	.type = FSP_UINT32,
	.offset = offsetof(struct fsp_m_config, mrc_boot_data_ptr),
	.propname = "fspm,mrc-boot-data-ptr",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, e_mmc_trace_len),
	.propname = "fspm,emmc-trace-len",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, skip_cse_rbp),
	.propname = "fspm,skip-cse-rbp",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, npk_en),
	.propname = "fspm,npk-en",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, fw_trace_en),
	.propname = "fspm,fw-trace-en",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, fw_trace_destination),
	.propname = "fspm,fw-trace-destination",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, recover_dump),
	.propname = "fspm,recover-dump",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, msc0_wrap),
	.propname = "fspm,msc0-wrap",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, msc1_wrap),
	.propname = "fspm,msc1-wrap",
	}, {
	.type = FSP_UINT32,
	.offset = offsetof(struct fsp_m_config, msc0_size),
	.propname = "fspm,msc0-size",
	}, {
	.type = FSP_UINT32,
	.offset = offsetof(struct fsp_m_config, msc1_size),
	.propname = "fspm,msc1-size",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, pti_mode),
	.propname = "fspm,pti-mode",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, pti_training),
	.propname = "fspm,pti-training",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, pti_speed),
	.propname = "fspm,pti-speed",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, punit_mlvl),
	.propname = "fspm,punit-mlvl",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, pmc_mlvl),
	.propname = "fspm,pmc-mlvl",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, sw_trace_en),
	.propname = "fspm,sw-trace-en",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, periodic_retraining_disable),
	.propname = "fspm,periodic-retraining-disable",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, enable_reset_system),
	.propname = "fspm,enable-reset-system",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, enable_s3_heci2),
	.propname = "fspm,enable-s3-heci2",
	}, {
	.type = FSP_UINT32,
	.offset = offsetof(struct fsp_m_config, variable_nvs_buffer_ptr),
	.propname = "fspm,variable-nvs-buffer-ptr",
	}, {
	.type = FSP_UINT64,
	.offset = offsetof(struct fsp_m_config, start_timer_ticker_of_pfet_assert),
	.propname = "fspm,start-timer-ticker-of-pfet-assert",
	}, {
	.type = FSP_UINT8, .offset = offsetof(struct fsp_m_config, rt_en),
	.propname = "fspm,rt-en",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_m_config, skip_pcie_power_sequence),
	.propname = "fspm,skip-pcie-power-sequence",
	}, {
	.propname = NULL
	}
};

int fsp_m_update_config_from_dtb(ofnode node, struct fsp_m_config *cfg)
{
	return fsp_update_config_from_dtb(node, (u8 *)cfg, fsp_m_bindings);
}
#endif

#if !defined(CONFIG_SPL_BUILD) && !defined(CONFIG_TPL_BUILD)
const struct fsp_binding fsp_s_bindings[] = {
	{
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, active_processor_cores),
	.propname = "fsps,active-processor-cores",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, disable_core1),
	.propname = "fsps,disable-core1",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, disable_core2),
	.propname = "fsps,disable-core2",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, disable_core3),
	.propname = "fsps,disable-core3",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, vmx_enable),
	.propname = "fsps,vmx-enable",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, proc_trace_mem_size),
	.propname = "fsps,proc-trace-mem-size",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, proc_trace_enable),
	.propname = "fsps,proc-trace-enable",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, eist),
	.propname = "fsps,eist",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, boot_p_state),
	.propname = "fsps,boot-p-state",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, enable_cx),
	.propname = "fsps,enable-cx",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, c1e),
	.propname = "fsps,c1e",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, bi_proc_hot),
	.propname = "fsps,bi-proc-hot",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, pkg_c_state_limit),
	.propname = "fsps,pkg-c-state-limit",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, c_state_auto_demotion),
	.propname = "fsps,c-state-auto-demotion",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, c_state_un_demotion),
	.propname = "fsps,c-state-un-demotion",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, max_core_c_state),
	.propname = "fsps,max-core-c-state",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, pkg_c_state_demotion),
	.propname = "fsps,pkg-c-state-demotion",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, pkg_c_state_un_demotion),
	.propname = "fsps,pkg-c-state-un-demotion",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, turbo_mode),
	.propname = "fsps,turbo-mode",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, hda_verb_table_entry_num),
	.propname = "fsps,hda-verb-table-entry-num",
	}, {
	.type = FSP_UINT32,
	.offset = offsetof(struct fsp_s_config, hda_verb_table_ptr),
	.propname = "fsps,hda-verb-table-ptr",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, p2sb_unhide),
	.propname = "fsps,p2sb-unhide",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, ipu_en),
	.propname = "fsps,ipu-en",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, ipu_acpi_mode),
	.propname = "fsps,ipu-acpi-mode",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, force_wake),
	.propname = "fsps,force-wake",
	}, {
	.type = FSP_UINT32,
	.offset = offsetof(struct fsp_s_config, gtt_mm_adr),
	.propname = "fsps,gtt-mm-adr",
	}, {
	.type = FSP_UINT32,
	.offset = offsetof(struct fsp_s_config, gm_adr),
	.propname = "fsps,gm-adr",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, pavp_lock),
	.propname = "fsps,pavp-lock",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, graphics_freq_modify),
	.propname = "fsps,graphics-freq-modify",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, graphics_freq_req),
	.propname = "fsps,graphics-freq-req",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, graphics_video_freq),
	.propname = "fsps,graphics-video-freq",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, pm_lock),
	.propname = "fsps,pm-lock",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, dop_clock_gating),
	.propname = "fsps,dop-clock-gating",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, unsolicited_attack_override),
	.propname = "fsps,unsolicited-attack-override",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, wopcm_support),
	.propname = "fsps,wopcm-support",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, wopcm_size),
	.propname = "fsps,wopcm-size",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, power_gating),
	.propname = "fsps,power-gating",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, unit_level_clock_gating),
	.propname = "fsps,unit-level-clock-gating",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, fast_boot),
	.propname = "fsps,fast-boot",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, dyn_sr),
	.propname = "fsps,dyn-sr",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, sa_ipu_enable),
	.propname = "fsps,sa-ipu-enable",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, pm_support),
	.propname = "fsps,pm-support",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, enable_render_standby),
	.propname = "fsps,enable-render-standby",
	}, {
	.type = FSP_UINT32,
	.offset = offsetof(struct fsp_s_config, logo_size),
	.propname = "fsps,logo-size",
	}, {
	.type = FSP_UINT32,
	.offset = offsetof(struct fsp_s_config, logo_ptr),
	.propname = "fsps,logo-ptr",
	}, {
	.type = FSP_UINT32,
	.offset = offsetof(struct fsp_s_config, graphics_config_ptr),
	.propname = "fsps,graphics-config-ptr",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, pavp_enable),
	.propname = "fsps,pavp-enable",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, pavp_pr3),
	.propname = "fsps,pavp-pr3",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, cd_clock),
	.propname = "fsps,cd-clock",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, pei_graphics_peim_init),
	.propname = "fsps,pei-graphics-peim-init",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, write_protection_enable),
	.propname = "fsps,write-protection-enable",
	.count = ARRAY_SIZE_OF_MEMBER(struct fsp_s_config,
				      write_protection_enable),
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, read_protection_enable),
	.propname = "fsps,read-protection-enable",
	.count = ARRAY_SIZE_OF_MEMBER(struct fsp_s_config,
				      read_protection_enable),
	}, {
	.type = FSP_UINT16,
	.offset = offsetof(struct fsp_s_config, protected_range_limit),
	.propname = "fsps,protected-range-limit",
	.count = ARRAY_SIZE_OF_MEMBER(struct fsp_s_config,
				      protected_range_limit),
	}, {
	.type = FSP_UINT16,
	.offset = offsetof(struct fsp_s_config, protected_range_base),
	.propname = "fsps,protected-range-base",
	.count = ARRAY_SIZE_OF_MEMBER(struct fsp_s_config,
				      protected_range_base),
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, gmm),
	.propname = "fsps,gmm",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, clk_gating_pgcb_clk_trunk),
	.propname = "fsps,clk-gating-pgcb-clk-trunk",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, clk_gating_sb),
	.propname = "fsps,clk-gating-sb",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, clk_gating_sb_clk_trunk),
	.propname = "fsps,clk-gating-sb-clk-trunk",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, clk_gating_sb_clk_partition),
	.propname = "fsps,clk-gating-sb-clk-partition",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, clk_gating_core),
	.propname = "fsps,clk-gating-core",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, clk_gating_dma),
	.propname = "fsps,clk-gating-dma",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, clk_gating_reg_access),
	.propname = "fsps,clk-gating-reg-access",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, clk_gating_host),
	.propname = "fsps,clk-gating-host",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, clk_gating_partition),
	.propname = "fsps,clk-gating-partition",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, clk_gating_trunk),
	.propname = "fsps,clk-gating-trunk",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, hda_enable),
	.propname = "fsps,hda-enable",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, dsp_enable),
	.propname = "fsps,dsp-enable",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, pme),
	.propname = "fsps,pme",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, hd_audio_io_buffer_ownership),
	.propname = "fsps,hd-audio-io-buffer-ownership",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, hd_audio_io_buffer_voltage),
	.propname = "fsps,hd-audio-io-buffer-voltage",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, hd_audio_vc_type),
	.propname = "fsps,hd-audio-vc-type",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, hd_audio_link_frequency),
	.propname = "fsps,hd-audio-link-frequency",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, hd_audio_i_disp_link_frequency),
	.propname = "fsps,hd-audio-i-disp-link-frequency",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, hd_audio_i_disp_link_tmode),
	.propname = "fsps,hd-audio-i-disp-link-tmode",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, dsp_endpoint_dmic),
	.propname = "fsps,dsp-endpoint-dmic",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, dsp_endpoint_bluetooth),
	.propname = "fsps,dsp-endpoint-bluetooth",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, dsp_endpoint_i2s_skp),
	.propname = "fsps,dsp-endpoint-i2s-skp",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, dsp_endpoint_i2s_hp),
	.propname = "fsps,dsp-endpoint-i2s-hp",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, audio_ctl_pwr_gate),
	.propname = "fsps,audio-ctl-pwr-gate",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, audio_dsp_pwr_gate),
	.propname = "fsps,audio-dsp-pwr-gate",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, mmt),
	.propname = "fsps,mmt",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, hmt),
	.propname = "fsps,hmt",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, hd_audio_pwr_gate),
	.propname = "fsps,hd-audio-pwr-gate",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, hd_audio_clk_gate),
	.propname = "fsps,hd-audio-clk-gate",
	}, {
	.type = FSP_UINT32,
	.offset = offsetof(struct fsp_s_config, dsp_feature_mask),
	.propname = "fsps,dsp-feature-mask",
	}, {
	.type = FSP_UINT32,
	.offset = offsetof(struct fsp_s_config, dsp_pp_module_mask),
	.propname = "fsps,dsp-pp-module-mask",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, bios_cfg_lock_down),
	.propname = "fsps,bios-cfg-lock-down",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, hpet),
	.propname = "fsps,hpet",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, hpet_bdf_valid),
	.propname = "fsps,hpet-bdf-valid",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, hpet_bus_number),
	.propname = "fsps,hpet-bus-number",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, hpet_device_number),
	.propname = "fsps,hpet-device-number",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, hpet_function_number),
	.propname = "fsps,hpet-function-number",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, io_apic_bdf_valid),
	.propname = "fsps,io-apic-bdf-valid",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, io_apic_bus_number),
	.propname = "fsps,io-apic-bus-number",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, io_apic_device_number),
	.propname = "fsps,io-apic-device-number",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, io_apic_function_number),
	.propname = "fsps,io-apic-function-number",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, io_apic_entry24_119),
	.propname = "fsps,io-apic-entry24-119",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, io_apic_id),
	.propname = "fsps,io-apic-id",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, io_apic_range_select),
	.propname = "fsps,io-apic-range-select",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, ish_enable),
	.propname = "fsps,ish-enable",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, bios_interface),
	.propname = "fsps,bios-interface",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, bios_lock),
	.propname = "fsps,bios-lock",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, spi_eiss),
	.propname = "fsps,spi-eiss",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, bios_lock_sw_smi_number),
	.propname = "fsps,bios-lock-sw-smi-number",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, lpss_s0ix_enable),
	.propname = "fsps,lpss-s0ix-enable",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, i2c_clk_gate_cfg),
	.propname = "fsps,i2c-clk-gate-cfg",
	.count = ARRAY_SIZE_OF_MEMBER(struct fsp_s_config, i2c_clk_gate_cfg),
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, hsuart_clk_gate_cfg),
	.propname = "fsps,hsuart-clk-gate-cfg",
	.count = ARRAY_SIZE_OF_MEMBER(struct fsp_s_config, hsuart_clk_gate_cfg),
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, spi_clk_gate_cfg),
	.propname = "fsps,spi-clk-gate-cfg",
	.count = ARRAY_SIZE_OF_MEMBER(struct fsp_s_config, spi_clk_gate_cfg),
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, i2c0_enable),
	.propname = "fsps,i2c0-enable",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, i2c1_enable),
	.propname = "fsps,i2c1-enable",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, i2c2_enable),
	.propname = "fsps,i2c2-enable",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, i2c3_enable),
	.propname = "fsps,i2c3-enable",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, i2c4_enable),
	.propname = "fsps,i2c4-enable",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, i2c5_enable),
	.propname = "fsps,i2c5-enable",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, i2c6_enable),
	.propname = "fsps,i2c6-enable",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, i2c7_enable),
	.propname = "fsps,i2c7-enable",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, hsuart0_enable),
	.propname = "fsps,hsuart0-enable",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, hsuart1_enable),
	.propname = "fsps,hsuart1-enable",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, hsuart2_enable),
	.propname = "fsps,hsuart2-enable",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, hsuart3_enable),
	.propname = "fsps,hsuart3-enable",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, spi0_enable),
	.propname = "fsps,spi0-enable",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, spi1_enable),
	.propname = "fsps,spi1-enable",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, spi2_enable),
	.propname = "fsps,spi2-enable",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, os_dbg_enable),
	.propname = "fsps,os-dbg-enable",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, dci_en),
	.propname = "fsps,dci-en",
	}, {
	.type = FSP_UINT32,
	.offset = offsetof(struct fsp_s_config,
			   uart2_kernel_debug_base_address),
	.propname = "fsps,uart2-kernel-debug-base-address",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, pcie_clock_gating_disabled),
	.propname = "fsps,pcie-clock-gating-disabled",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, pcie_root_port8xh_decode),
	.propname = "fsps,pcie-root-port8xh-decode",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, pcie8xh_decode_port_index),
	.propname = "fsps,pcie8xh-decode-port-index",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config,
			   pcie_root_port_peer_memory_write_enable),
	.propname = "fsps,pcie-root-port-peer-memory-write-enable",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, pcie_aspm_sw_smi_number),
	.propname = "fsps,pcie-aspm-sw-smi-number",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, pcie_root_port_en),
	.propname = "fsps,pcie-root-port-en",
	.count = ARRAY_SIZE_OF_MEMBER(struct fsp_s_config, pcie_root_port_en),
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, pcie_rp_hide),
	.propname = "fsps,pcie-rp-hide",
	.count = ARRAY_SIZE_OF_MEMBER(struct fsp_s_config, pcie_rp_hide),
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, pcie_rp_slot_implemented),
	.propname = "fsps,pcie-rp-slot-implemented",
	.count = ARRAY_SIZE_OF_MEMBER(struct fsp_s_config,
				      pcie_rp_slot_implemented),
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, pcie_rp_hot_plug),
	.propname = "fsps,pcie-rp-hot-plug",
	.count = ARRAY_SIZE_OF_MEMBER(struct fsp_s_config, pcie_rp_hot_plug),
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, pcie_rp_pm_sci),
	.propname = "fsps,pcie-rp-pm-sci",
	.count = ARRAY_SIZE_OF_MEMBER(struct fsp_s_config, pcie_rp_pm_sci),
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, pcie_rp_ext_sync),
	.propname = "fsps,pcie-rp-ext-sync",
	.count = ARRAY_SIZE_OF_MEMBER(struct fsp_s_config, pcie_rp_ext_sync),
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config,
			   pcie_rp_transmitter_half_swing),
	.propname = "fsps,pcie-rp-transmitter-half-swing",
	.count = ARRAY_SIZE_OF_MEMBER(struct fsp_s_config,
				      pcie_rp_transmitter_half_swing),
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, pcie_rp_acs_enabled),
	.propname = "fsps,pcie-rp-acs-enabled",
	.count = ARRAY_SIZE_OF_MEMBER(struct fsp_s_config, pcie_rp_acs_enabled),
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, pcie_rp_clk_req_supported),
	.propname = "fsps,pcie-rp-clk-req-supported",
	.count = ARRAY_SIZE_OF_MEMBER(struct fsp_s_config,
				      pcie_rp_clk_req_supported),
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, pcie_rp_clk_req_number),
	.propname = "fsps,pcie-rp-clk-req-number",
	.count = ARRAY_SIZE_OF_MEMBER(struct fsp_s_config,
				      pcie_rp_clk_req_number),
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, pcie_rp_clk_req_detect),
	.propname = "fsps,pcie-rp-clk-req-detect",
	.count = ARRAY_SIZE_OF_MEMBER(struct fsp_s_config,
				      pcie_rp_clk_req_detect),
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, advanced_error_reporting),
	.propname = "fsps,advanced-error-reporting",
	.count = ARRAY_SIZE_OF_MEMBER(struct fsp_s_config,
				      advanced_error_reporting),
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, pme_interrupt),
	.propname = "fsps,pme-interrupt",
	.count = ARRAY_SIZE_OF_MEMBER(struct fsp_s_config, pme_interrupt),
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, unsupported_request_report),
	.propname = "fsps,unsupported-request-report",
	.count = ARRAY_SIZE_OF_MEMBER(struct fsp_s_config,
				      unsupported_request_report),
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, fatal_error_report),
	.propname = "fsps,fatal-error-report",
	.count = ARRAY_SIZE_OF_MEMBER(struct fsp_s_config, fatal_error_report),
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, no_fatal_error_report),
	.propname = "fsps,no-fatal-error-report",
	.count = ARRAY_SIZE_OF_MEMBER(struct fsp_s_config,
				      no_fatal_error_report),
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, correctable_error_report),
	.propname = "fsps,correctable-error-report",
	.count = ARRAY_SIZE_OF_MEMBER(struct fsp_s_config,
				      correctable_error_report),
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config,
			   system_error_on_fatal_error),
	.propname = "fsps,system-error-on-fatal-error",
	.count = ARRAY_SIZE_OF_MEMBER(struct fsp_s_config,
				      system_error_on_fatal_error),
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config,
			   system_error_on_non_fatal_error),
	.propname = "fsps,system-error-on-non-fatal-error",
	.count = ARRAY_SIZE_OF_MEMBER(struct fsp_s_config,
				      system_error_on_non_fatal_error),
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config,
			   system_error_on_correctable_error),
	.propname = "fsps,system-error-on-correctable-error",
	.count = ARRAY_SIZE_OF_MEMBER(struct fsp_s_config,
				      system_error_on_correctable_error),
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, pcie_rp_speed),
	.propname = "fsps,pcie-rp-speed",
	.count = ARRAY_SIZE_OF_MEMBER(struct fsp_s_config, pcie_rp_speed),
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, physical_slot_number),
	.propname = "fsps,physical-slot-number",
	.count = ARRAY_SIZE_OF_MEMBER(struct fsp_s_config,
				      physical_slot_number),
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, pcie_rp_completion_timeout),
	.propname = "fsps,pcie-rp-completion-timeout",
	.count = ARRAY_SIZE_OF_MEMBER(struct fsp_s_config,
				      pcie_rp_completion_timeout),
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, ptm_enable),
	.propname = "fsps,ptm-enable",
	.count = ARRAY_SIZE_OF_MEMBER(struct fsp_s_config, ptm_enable),
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, pcie_rp_aspm),
	.propname = "fsps,pcie-rp-aspm",
	.count = ARRAY_SIZE_OF_MEMBER(struct fsp_s_config, pcie_rp_aspm),
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, pcie_rp_l1_substates),
	.propname = "fsps,pcie-rp-l1-substates",
	.count = ARRAY_SIZE_OF_MEMBER(struct fsp_s_config,
				      pcie_rp_l1_substates),
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, pcie_rp_ltr_enable),
	.propname = "fsps,pcie-rp-ltr-enable",
	.count = ARRAY_SIZE_OF_MEMBER(struct fsp_s_config, pcie_rp_ltr_enable),
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, pcie_rp_ltr_config_lock),
	.propname = "fsps,pcie-rp-ltr-config-lock",
	.count = ARRAY_SIZE_OF_MEMBER(struct fsp_s_config,
				      pcie_rp_ltr_config_lock),
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, pme_b0_s5_dis),
	.propname = "fsps,pme-b0-s5-dis",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, pci_clock_run),
	.propname = "fsps,pci-clock-run",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, timer8254_clk_setting),
	.propname = "fsps,timer8254-clk-setting",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, enable_sata),
	.propname = "fsps,enable-sata",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, sata_mode),
	.propname = "fsps,sata-mode",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, sata_salp_support),
	.propname = "fsps,sata-salp-support",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, sata_pwr_opt_enable),
	.propname = "fsps,sata-pwr-opt-enable",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, e_sata_speed_limit),
	.propname = "fsps,e-sata-speed-limit",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, speed_limit),
	.propname = "fsps,speed-limit",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, sata_ports_enable),
	.propname = "fsps,sata-ports-enable",
	.count = ARRAY_SIZE_OF_MEMBER(struct fsp_s_config, sata_ports_enable),
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, sata_ports_dev_slp),
	.propname = "fsps,sata-ports-dev-slp",
	.count = ARRAY_SIZE_OF_MEMBER(struct fsp_s_config, sata_ports_dev_slp),
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, sata_ports_hot_plug),
	.propname = "fsps,sata-ports-hot-plug",
	.count = ARRAY_SIZE_OF_MEMBER(struct fsp_s_config, sata_ports_hot_plug),
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, sata_ports_interlock_sw),
	.propname = "fsps,sata-ports-interlock-sw",
	.count = ARRAY_SIZE_OF_MEMBER(struct fsp_s_config,
				      sata_ports_interlock_sw),
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, sata_ports_external),
	.propname = "fsps,sata-ports-external",
	.count = ARRAY_SIZE_OF_MEMBER(struct fsp_s_config, sata_ports_external),
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, sata_ports_spin_up),
	.propname = "fsps,sata-ports-spin-up",
	.count = ARRAY_SIZE_OF_MEMBER(struct fsp_s_config, sata_ports_spin_up),
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, sata_ports_solid_state_drive),
	.propname = "fsps,sata-ports-solid-state-drive",
	.count = ARRAY_SIZE_OF_MEMBER(struct fsp_s_config,
				      sata_ports_solid_state_drive),
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, sata_ports_enable_dito_config),
	.propname = "fsps,sata-ports-enable-dito-config",
	.count = ARRAY_SIZE_OF_MEMBER(struct fsp_s_config,
				      sata_ports_enable_dito_config),
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, sata_ports_dm_val),
	.propname = "fsps,sata-ports-dm-val",
	.count = ARRAY_SIZE_OF_MEMBER(struct fsp_s_config, sata_ports_dm_val),
	}, {
	.type = FSP_UINT16,
	.offset = offsetof(struct fsp_s_config, sata_ports_dito_val),
	.propname = "fsps,sata-ports-dito-val",
	.count = ARRAY_SIZE_OF_MEMBER(struct fsp_s_config, sata_ports_dito_val),
	}, {
	.type = FSP_UINT16,
	.offset = offsetof(struct fsp_s_config, sub_system_vendor_id),
	.propname = "fsps,sub-system-vendor-id",
	}, {
	.type = FSP_UINT16,
	.offset = offsetof(struct fsp_s_config, sub_system_id),
	.propname = "fsps,sub-system-id",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, crid_settings),
	.propname = "fsps,crid-settings",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, reset_select),
	.propname = "fsps,reset-select",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, sdcard_enabled),
	.propname = "fsps,sdcard-enabled",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, e_mmc_enabled),
	.propname = "fsps,emmc-enabled",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, e_mmc_host_max_speed),
	.propname = "fsps,emmc-host-max-speed",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, ufs_enabled),
	.propname = "fsps,ufs-enabled",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, sdio_enabled),
	.propname = "fsps,sdio-enabled",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, gpp_lock),
	.propname = "fsps,gpp-lock",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, sirq_enable),
	.propname = "fsps,sirq-enable",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, sirq_mode),
	.propname = "fsps,sirq-mode",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, start_frame_pulse),
	.propname = "fsps,start-frame-pulse",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, smbus_enable),
	.propname = "fsps,smbus-enable",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, arp_enable),
	.propname = "fsps,arp-enable",
	}, {
	.type = FSP_UINT16,
	.offset = offsetof(struct fsp_s_config, num_rsvd_smbus_addresses),
	.propname = "fsps,num-rsvd-smbus-addresses",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, rsvd_smbus_address_table),
	.propname = "fsps,rsvd-smbus-address-table",
	.count = ARRAY_SIZE_OF_MEMBER(struct fsp_s_config,
				      rsvd_smbus_address_table),
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, disable_compliance_mode),
	.propname = "fsps,disable-compliance-mode",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, usb_per_port_ctl),
	.propname = "fsps,usb-per-port-ctl",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, usb30_mode),
	.propname = "fsps,usb30-mode",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, port_usb20_enable),
	.propname = "fsps,port-usb20-enable",
	.count = ARRAY_SIZE_OF_MEMBER(struct fsp_s_config, port_usb20_enable),
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, port_us20b_over_current_pin),
	.propname = "fsps,port-us20b-over-current-pin",
	.count = ARRAY_SIZE_OF_MEMBER(struct fsp_s_config,
				      port_us20b_over_current_pin),
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, usb_otg),
	.propname = "fsps,usb-otg",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, hsic_support_enable),
	.propname = "fsps,hsic-support-enable",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, port_usb30_enable),
	.propname = "fsps,port-usb30-enable",
	.count = ARRAY_SIZE_OF_MEMBER(struct fsp_s_config, port_usb30_enable),
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, port_us30b_over_current_pin),
	.propname = "fsps,port-us30b-over-current-pin",
	.count = ARRAY_SIZE_OF_MEMBER(struct fsp_s_config,
				      port_us30b_over_current_pin),
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, ssic_port_enable),
	.propname = "fsps,ssic-port-enable",
	.count = ARRAY_SIZE_OF_MEMBER(struct fsp_s_config, ssic_port_enable),
	}, {
	.type = FSP_UINT16,
	.offset = offsetof(struct fsp_s_config, dlane_pwr_gating),
	.propname = "fsps,dlane-pwr-gating",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, vtd_enable),
	.propname = "fsps,vtd-enable",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, lock_down_global_smi),
	.propname = "fsps,lock-down-global-smi",
	}, {
	.type = FSP_UINT16,
	.offset = offsetof(struct fsp_s_config, reset_wait_timer),
	.propname = "fsps,reset-wait-timer",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, rtc_lock),
	.propname = "fsps,rtc-lock",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, sata_test_mode),
	.propname = "fsps,sata-test-mode",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, ssic_rate),
	.propname = "fsps,ssic-rate",
	.count = ARRAY_SIZE_OF_MEMBER(struct fsp_s_config, ssic_rate),
	}, {
	.type = FSP_UINT16,
	.offset = offsetof(struct fsp_s_config, dynamic_power_gating),
	.propname = "fsps,dynamic-power-gating",
	}, {
	.type = FSP_UINT16,
	.offset = offsetof(struct fsp_s_config, pcie_rp_ltr_max_snoop_latency),
	.propname = "fsps,pcie-rp-ltr-max-snoop-latency",
	.count = ARRAY_SIZE_OF_MEMBER(struct fsp_s_config,
				      pcie_rp_ltr_max_snoop_latency),
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config,
			   pcie_rp_snoop_latency_override_mode),
	.propname = "fsps,pcie-rp-snoop-latency-override-mode",
	.count = ARRAY_SIZE_OF_MEMBER(struct fsp_s_config,
				      pcie_rp_snoop_latency_override_mode),
	}, {
	.type = FSP_UINT16,
	.offset = offsetof(struct fsp_s_config,
			   pcie_rp_snoop_latency_override_value),
	.propname = "fsps,pcie-rp-snoop-latency-override-value",
	.count = ARRAY_SIZE_OF_MEMBER(struct fsp_s_config,
				      pcie_rp_snoop_latency_override_value),
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config,
			   pcie_rp_snoop_latency_override_multiplier),
	.propname = "fsps,pcie-rp-snoop-latency-override-multiplier",
	.count = ARRAY_SIZE_OF_MEMBER(struct fsp_s_config,
				      pcie_rp_snoop_latency_override_multiplier),
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, skip_mp_init),
	.propname = "fsps,skip-mp-init",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, dci_auto_detect),
	.propname = "fsps,dci-auto-detect",
	}, {
	.type = FSP_UINT16,
	.offset = offsetof(struct fsp_s_config,
			   pcie_rp_ltr_max_non_snoop_latency),
	.propname = "fsps,pcie-rp-ltr-max-non-snoop-latency",
	.count = ARRAY_SIZE_OF_MEMBER(struct fsp_s_config,
				      pcie_rp_ltr_max_non_snoop_latency),
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config,
			   pcie_rp_non_snoop_latency_override_mode),
	.propname = "fsps,pcie-rp-non-snoop-latency-override-mode",
	.count = ARRAY_SIZE_OF_MEMBER(struct fsp_s_config,
				      pcie_rp_non_snoop_latency_override_mode),
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, tco_timer_halt_lock),
	.propname = "fsps,tco-timer-halt-lock",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, pwr_btn_override_period),
	.propname = "fsps,pwr-btn-override-period",
	}, {
	.type = FSP_UINT16,
	.offset = offsetof(struct fsp_s_config,
			   pcie_rp_non_snoop_latency_override_value),
	.propname = "fsps,pcie-rp-non-snoop-latency-override-value",
	.count = ARRAY_SIZE_OF_MEMBER(struct fsp_s_config,
				      pcie_rp_non_snoop_latency_override_value),
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config,
			   pcie_rp_non_snoop_latency_override_multiplier),
	.propname = "fsps,pcie-rp-non-snoop-latency-override-multiplier",
	.count = ARRAY_SIZE_OF_MEMBER(struct fsp_s_config,
				pcie_rp_non_snoop_latency_override_multiplier),
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, pcie_rp_slot_power_limit_scale),
	.propname = "fsps,pcie-rp-slot-power-limit-scale",
	.count = ARRAY_SIZE_OF_MEMBER(struct fsp_s_config,
				      pcie_rp_slot_power_limit_scale),
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, pcie_rp_slot_power_limit_value),
	.propname = "fsps,pcie-rp-slot-power-limit-value",
	.count = ARRAY_SIZE_OF_MEMBER(struct fsp_s_config,
				      pcie_rp_slot_power_limit_value),
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, disable_native_power_button),
	.propname = "fsps,disable-native-power-button",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, power_butter_debounce_mode),
	.propname = "fsps,power-butter-debounce-mode",
	}, {
	.type = FSP_UINT32,
	.offset = offsetof(struct fsp_s_config, sdio_tx_cmd_cntl),
	.propname = "fsps,sdio-tx-cmd-cntl",
	}, {
	.type = FSP_UINT32,
	.offset = offsetof(struct fsp_s_config, sdio_tx_data_cntl1),
	.propname = "fsps,sdio-tx-data-cntl1",
	}, {
	.type = FSP_UINT32,
	.offset = offsetof(struct fsp_s_config, sdio_tx_data_cntl2),
	.propname = "fsps,sdio-tx-data-cntl2",
	}, {
	.type = FSP_UINT32,
	.offset = offsetof(struct fsp_s_config, sdio_rx_cmd_data_cntl1),
	.propname = "fsps,sdio-rx-cmd-data-cntl1",
	}, {
	.type = FSP_UINT32,
	.offset = offsetof(struct fsp_s_config, sdio_rx_cmd_data_cntl2),
	.propname = "fsps,sdio-rx-cmd-data-cntl2",
	}, {
	.type = FSP_UINT32,
	.offset = offsetof(struct fsp_s_config, sdcard_tx_cmd_cntl),
	.propname = "fsps,sdcard-tx-cmd-cntl",
	}, {
	.type = FSP_UINT32,
	.offset = offsetof(struct fsp_s_config, sdcard_tx_data_cntl1),
	.propname = "fsps,sdcard-tx-data-cntl1",
	}, {
	.type = FSP_UINT32,
	.offset = offsetof(struct fsp_s_config, sdcard_tx_data_cntl2),
	.propname = "fsps,sdcard-tx-data-cntl2",
	}, {
	.type = FSP_UINT32,
	.offset = offsetof(struct fsp_s_config, sdcard_rx_cmd_data_cntl1),
	.propname = "fsps,sdcard-rx-cmd-data-cntl1",
	}, {
	.type = FSP_UINT32,
	.offset = offsetof(struct fsp_s_config, sdcard_rx_strobe_cntl),
	.propname = "fsps,sdcard-rx-strobe-cntl",
	}, {
	.type = FSP_UINT32,
	.offset = offsetof(struct fsp_s_config, sdcard_rx_cmd_data_cntl2),
	.propname = "fsps,sdcard-rx-cmd-data-cntl2",
	}, {
	.type = FSP_UINT32,
	.offset = offsetof(struct fsp_s_config, emmc_tx_cmd_cntl),
	.propname = "fsps,emmc-tx-cmd-cntl",
	}, {
	.type = FSP_UINT32,
	.offset = offsetof(struct fsp_s_config, emmc_tx_data_cntl1),
	.propname = "fsps,emmc-tx-data-cntl1",
	}, {
	.type = FSP_UINT32,
	.offset = offsetof(struct fsp_s_config, emmc_tx_data_cntl2),
	.propname = "fsps,emmc-tx-data-cntl2",
	}, {
	.type = FSP_UINT32,
	.offset = offsetof(struct fsp_s_config, emmc_rx_cmd_data_cntl1),
	.propname = "fsps,emmc-rx-cmd-data-cntl1",
	}, {
	.type = FSP_UINT32,
	.offset = offsetof(struct fsp_s_config, emmc_rx_strobe_cntl),
	.propname = "fsps,emmc-rx-strobe-cntl",
	}, {
	.type = FSP_UINT32,
	.offset = offsetof(struct fsp_s_config, emmc_rx_cmd_data_cntl2),
	.propname = "fsps,emmc-rx-cmd-data-cntl2",
	}, {
	.type = FSP_UINT32,
	.offset = offsetof(struct fsp_s_config, emmc_master_sw_cntl),
	.propname = "fsps,emmc-master-sw-cntl",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, pcie_rp_selectable_deemphasis),
	.propname = "fsps,pcie-rp-selectable-deemphasis",
	.count = ARRAY_SIZE_OF_MEMBER(struct fsp_s_config,
				      pcie_rp_selectable_deemphasis),
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, monitor_mwait_enable),
	.propname = "fsps,monitor-mwait-enable",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, hd_audio_dsp_uaa_compliance),
	.propname = "fsps,hd-audio-dsp-uaa-compliance",
	}, {
	.type = FSP_UINT32,
	.offset = offsetof(struct fsp_s_config, ipc),
	.propname = "fsps,ipc",
	.count = ARRAY_SIZE_OF_MEMBER(struct fsp_s_config, ipc),
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, sata_ports_disable_dynamic_pg),
	.propname = "fsps,sata-ports-disable-dynamic-pg",
	.count = ARRAY_SIZE_OF_MEMBER(struct fsp_s_config,
				      sata_ports_disable_dynamic_pg),
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, init_s3_cpu),
	.propname = "fsps,init-s3-cpu",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, skip_punit_init),
	.propname = "fsps,skip-punit-init",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, port_usb20_per_port_tx_pe_half),
	.propname = "fsps,port-usb20-per-port-tx-pe-half",
	.count = ARRAY_SIZE_OF_MEMBER(struct fsp_s_config,
				      port_usb20_per_port_tx_pe_half),
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, port_usb20_per_port_pe_txi_set),
	.propname = "fsps,port-usb20-per-port-pe-txi-set",
	.count = ARRAY_SIZE_OF_MEMBER(struct fsp_s_config,
				      port_usb20_per_port_pe_txi_set),
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, port_usb20_per_port_txi_set),
	.propname = "fsps,port-usb20-per-port-txi-set",
	.count = ARRAY_SIZE_OF_MEMBER(struct fsp_s_config,
				      port_usb20_per_port_txi_set),
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, port_usb20_hs_skew_sel),
	.propname = "fsps,port-usb20-hs-skew-sel",
	.count = ARRAY_SIZE_OF_MEMBER(struct fsp_s_config,
				      port_usb20_hs_skew_sel),
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config,
			   port_usb20_i_usb_tx_emphasis_en),
	.propname = "fsps,port-usb20-i-usb-tx-emphasis-en",
	.count = ARRAY_SIZE_OF_MEMBER(struct fsp_s_config,
				      port_usb20_i_usb_tx_emphasis_en),
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config,
			   port_usb20_per_port_rxi_set),
	.propname = "fsps,port-usb20-per-port-rxi-set",
	.count = ARRAY_SIZE_OF_MEMBER(struct fsp_s_config,
				      port_usb20_per_port_rxi_set),
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, port_usb20_hs_npre_drv_sel),
	.propname = "fsps,port-usb20-hs-npre-drv-sel",
	.count = ARRAY_SIZE_OF_MEMBER(struct fsp_s_config,
				      port_usb20_hs_npre_drv_sel),
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, os_selection),
	.propname = "fsps,os-selection",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, dptf_enabled),
	.propname = "fsps,dptf-enabled",
	}, {
	.type = FSP_UINT8,
	.offset = offsetof(struct fsp_s_config, pwm_enabled),
	.propname = "fsps,pwm-enabled",
	}, {
	.propname = NULL
	}
};

int fsp_s_update_config_from_dtb(ofnode node, struct fsp_s_config *cfg)
{
	return fsp_update_config_from_dtb(node, (u8 *)cfg, fsp_s_bindings);
}
#endif
