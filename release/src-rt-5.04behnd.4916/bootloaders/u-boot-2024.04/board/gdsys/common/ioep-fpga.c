// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014
 * Dirk Eibach,  Guntermann & Drunck GmbH, dirk.eibach@gdsys.cc
 */

#ifdef CONFIG_GDSYS_LEGACY_DRIVERS

#include <common.h>

#include <gdsys_fpga.h>
#include <linux/bitops.h>

enum pcb_video_type {
	PCB_DVI_SL,
	PCB_DP_165MPIX,
	PCB_DP_300MPIX,
	PCB_HDMI,
	PCB_DP_1_2,
	PCB_HDMI_2_0,
};

enum pcb_transmission_type {
	PCB_CAT_1G,
	PCB_FIBER_3G,
	PCB_CAT_10G,
	PCB_FIBER_10G,
};

enum carrier_speed {
	CARRIER_SPEED_1G,
	CARRIER_SPEED_3G,
	CARRIER_SPEED_2_5G = CARRIER_SPEED_3G,
	CARRIER_SPEED_10G,
};

enum ram_config {
	RAM_DDR2_32BIT_295MBPS,
	RAM_DDR3_32BIT_590MBPS,
	RAM_DDR3_48BIT_590MBPS,
	RAM_DDR3_64BIT_1800MBPS,
	RAM_DDR3_48BIT_1800MBPS,
};

enum sysclock {
	SYSCLK_147456,
};

struct fpga_versions {
	bool video_channel;
	bool con_side;
	enum pcb_video_type pcb_video_type;
	enum pcb_transmission_type pcb_transmission_type;
	unsigned int hw_version;
};

struct fpga_features {
	u8 video_channels;
	u8 carriers;
	enum carrier_speed carrier_speed;
	enum ram_config ram_config;
	enum sysclock sysclock;

	bool pcm_tx;
	bool pcm_rx;
	bool spdif_tx;
	bool spdif_rx;
	bool usb2;
	bool rs232;
	bool compression_type1;
	bool compression_type2;
	bool compression_type3;
	bool interlace;
	bool osd;
	bool compression_pipes;
};

#ifdef CONFIG_SYS_FPGA_FLAVOR_GAZERBEAM

static int get_versions(unsigned int fpga, struct fpga_versions *versions)
{
	enum {
		VERSIONS_FPGA_VIDEO_CHANNEL = BIT(12),
		VERSIONS_FPGA_CON_SIDE = BIT(13),
		VERSIONS_FPGA_SC = BIT(14),
		VERSIONS_PCB_CON = BIT(9),
		VERSIONS_PCB_SC = BIT(8),
		VERSIONS_PCB_VIDEO_MASK = 0x3 << 6,
		VERSIONS_PCB_VIDEO_DP_1_2 = 0x0 << 6,
		VERSIONS_PCB_VIDEO_HDMI_2_0 = 0x1 << 6,
		VERSIONS_PCB_TRANSMISSION_MASK = 0x3 << 4,
		VERSIONS_PCB_TRANSMISSION_FIBER_10G = 0x0 << 4,
		VERSIONS_PCB_TRANSMISSION_CAT_10G = 0x1 << 4,
		VERSIONS_PCB_TRANSMISSION_FIBER_3G = 0x2 << 4,
		VERSIONS_PCB_TRANSMISSION_CAT_1G = 0x3 << 4,
		VERSIONS_HW_VER_MASK = 0xf << 0,
	};
	u16 raw_versions;

	memset(versions, 0, sizeof(struct fpga_versions));

	FPGA_GET_REG(fpga, versions, &raw_versions);

	versions->video_channel = raw_versions & VERSIONS_FPGA_VIDEO_CHANNEL;
	versions->con_side = raw_versions & VERSIONS_FPGA_CON_SIDE;

	switch (raw_versions & VERSIONS_PCB_VIDEO_MASK) {
	case VERSIONS_PCB_VIDEO_DP_1_2:
		versions->pcb_video_type = PCB_DP_1_2;
		break;

	case VERSIONS_PCB_VIDEO_HDMI_2_0:
		versions->pcb_video_type = PCB_HDMI_2_0;
		break;
	}

	switch (raw_versions & VERSIONS_PCB_TRANSMISSION_MASK) {
	case VERSIONS_PCB_TRANSMISSION_FIBER_10G:
		versions->pcb_transmission_type = PCB_FIBER_10G;
		break;

	case VERSIONS_PCB_TRANSMISSION_CAT_10G:
		versions->pcb_transmission_type = PCB_CAT_10G;
		break;

	case VERSIONS_PCB_TRANSMISSION_FIBER_3G:
		versions->pcb_transmission_type = PCB_FIBER_3G;
		break;

	case VERSIONS_PCB_TRANSMISSION_CAT_1G:
		versions->pcb_transmission_type = PCB_CAT_1G;
		break;

	}

	versions->hw_version = raw_versions & VERSIONS_HW_VER_MASK;

	return 0;
}

static int get_features(unsigned int fpga, struct fpga_features *features)
{
	enum {
		FEATURE_SPDIF_RX = BIT(15),
		FEATURE_SPDIF_TX = BIT(14),
		FEATURE_PCM_RX = BIT(13),
		FEATURE_PCM_TX = BIT(12),
		FEATURE_RAM_MASK = GENMASK(11, 8),
		FEATURE_RAM_DDR2_32BIT_295MBPS = 0x0 << 8,
		FEATURE_RAM_DDR3_32BIT_590MBPS = 0x1 << 8,
		FEATURE_RAM_DDR3_48BIT_590MBPS = 0x2 << 8,
		FEATURE_RAM_DDR3_64BIT_1800MBPS = 0x3 << 8,
		FEATURE_RAM_DDR3_48BIT_1800MBPS = 0x4 << 8,
		FEATURE_CARRIER_SPEED_MASK = GENMASK(7, 6),
		FEATURE_CARRIER_SPEED_1G = 0x0 << 6,
		FEATURE_CARRIER_SPEED_2_5G = 0x1 << 6,
		FEATURE_CARRIER_SPEED_10G = 0x2 << 6,
		FEATURE_CARRIERS_MASK = GENMASK(5, 4),
		FEATURE_CARRIERS_0 = 0x0 << 4,
		FEATURE_CARRIERS_1 = 0x1 << 4,
		FEATURE_CARRIERS_2 = 0x2 << 4,
		FEATURE_CARRIERS_4 = 0x3 << 4,
		FEATURE_USB2 = BIT(3),
		FEATURE_VIDEOCHANNELS_MASK = GENMASK(2, 0),
		FEATURE_VIDEOCHANNELS_0 = 0x0 << 0,
		FEATURE_VIDEOCHANNELS_1 = 0x1 << 0,
		FEATURE_VIDEOCHANNELS_1_1 = 0x2 << 0,
		FEATURE_VIDEOCHANNELS_2 = 0x3 << 0,
	};

	enum {
		EXT_FEATURE_OSD = BIT(15),
		EXT_FEATURE_ETHERNET = BIT(9),
		EXT_FEATURE_INTERLACE = BIT(8),
		EXT_FEATURE_RS232 = BIT(7),
		EXT_FEATURE_COMPRESSION_PERF_MASK = GENMASK(6, 4),
		EXT_FEATURE_COMPRESSION_PERF_1X = 0x0 << 4,
		EXT_FEATURE_COMPRESSION_PERF_2X = 0x1 << 4,
		EXT_FEATURE_COMPRESSION_PERF_4X = 0x2 << 4,
		EXT_FEATURE_COMPRESSION_TYPE1 = BIT(0),
		EXT_FEATURE_COMPRESSION_TYPE2 = BIT(1),
		EXT_FEATURE_COMPRESSION_TYPE3 = BIT(2),
	};

	u16 raw_features;
	u16 raw_extended_features;

	memset(features, 0, sizeof(struct fpga_features));

	FPGA_GET_REG(fpga, fpga_features, &raw_features);
	FPGA_GET_REG(fpga, fpga_ext_features, &raw_extended_features);

	switch (raw_features & FEATURE_VIDEOCHANNELS_MASK) {
	case FEATURE_VIDEOCHANNELS_0:
		features->video_channels = 0;
		break;

	case FEATURE_VIDEOCHANNELS_1:
		features->video_channels = 1;
		break;

	case FEATURE_VIDEOCHANNELS_1_1:
	case FEATURE_VIDEOCHANNELS_2:
		features->video_channels = 2;
		break;
	};

	switch (raw_features & FEATURE_CARRIERS_MASK) {
	case FEATURE_CARRIERS_0:
		features->carriers = 0;
		break;

	case FEATURE_CARRIERS_1:
		features->carriers = 1;
		break;

	case FEATURE_CARRIERS_2:
		features->carriers = 2;
		break;

	case FEATURE_CARRIERS_4:
		features->carriers = 4;
		break;
	}

	switch (raw_features & FEATURE_CARRIER_SPEED_MASK) {
	case FEATURE_CARRIER_SPEED_1G:
		features->carrier_speed = CARRIER_SPEED_1G;
		break;
	case FEATURE_CARRIER_SPEED_2_5G:
		features->carrier_speed = CARRIER_SPEED_2_5G;
		break;
	case FEATURE_CARRIER_SPEED_10G:
		features->carrier_speed = CARRIER_SPEED_10G;
		break;
	}

	switch (raw_features & FEATURE_RAM_MASK) {
	case FEATURE_RAM_DDR2_32BIT_295MBPS:
		features->ram_config = RAM_DDR2_32BIT_295MBPS;
		break;

	case FEATURE_RAM_DDR3_32BIT_590MBPS:
		features->ram_config = RAM_DDR3_32BIT_590MBPS;
		break;

	case FEATURE_RAM_DDR3_48BIT_590MBPS:
		features->ram_config = RAM_DDR3_48BIT_590MBPS;
		break;

	case FEATURE_RAM_DDR3_64BIT_1800MBPS:
		features->ram_config = RAM_DDR3_64BIT_1800MBPS;
		break;

	case FEATURE_RAM_DDR3_48BIT_1800MBPS:
		features->ram_config = RAM_DDR3_48BIT_1800MBPS;
		break;
	}

	features->pcm_tx = raw_features & FEATURE_PCM_TX;
	features->pcm_rx = raw_features & FEATURE_PCM_RX;
	features->spdif_tx = raw_features & FEATURE_SPDIF_TX;
	features->spdif_rx = raw_features & FEATURE_SPDIF_RX;
	features->usb2 = raw_features & FEATURE_USB2;
	features->rs232 = raw_extended_features & EXT_FEATURE_RS232;
	features->compression_type1 = raw_extended_features & EXT_FEATURE_COMPRESSION_TYPE1;
	features->compression_type2 = raw_extended_features & EXT_FEATURE_COMPRESSION_TYPE2;
	features->compression_type3 = raw_extended_features & EXT_FEATURE_COMPRESSION_TYPE3;
	features->interlace = raw_extended_features & EXT_FEATURE_INTERLACE;
	features->osd = raw_extended_features & EXT_FEATURE_OSD;
	features->compression_pipes = raw_extended_features & EXT_FEATURE_COMPRESSION_PERF_MASK;

	return 0;
}

#else

static int get_versions(unsigned int fpga, struct fpga_versions *versions)
{
	enum {
		/* HW version encoding is a mess, leave it for the moment */
		VERSIONS_HW_VER_MASK = 0xf << 0,
		VERSIONS_PIX_CLOCK_GEN_IDT8N3QV01 = BIT(4),
		VERSIONS_SFP = BIT(5),
		VERSIONS_VIDEO_MASK = 0x7 << 6,
		VERSIONS_VIDEO_DVI = 0x0 << 6,
		VERSIONS_VIDEO_DP_165 = 0x1 << 6,
		VERSIONS_VIDEO_DP_300 = 0x2 << 6,
		VERSIONS_VIDEO_HDMI = 0x3 << 6,
		VERSIONS_UT_MASK = 0xf << 12,
		VERSIONS_UT_MAIN_SERVER = 0x0 << 12,
		VERSIONS_UT_MAIN_USER = 0x1 << 12,
		VERSIONS_UT_VIDEO_SERVER = 0x2 << 12,
		VERSIONS_UT_VIDEO_USER = 0x3 << 12,
	};
	u16 raw_versions;

	memset(versions, 0, sizeof(struct fpga_versions));

	FPGA_GET_REG(fpga, versions, &raw_versions);

	switch (raw_versions & VERSIONS_UT_MASK) {
	case VERSIONS_UT_MAIN_SERVER:
		versions->video_channel = false;
		versions->con_side = false;
		break;

	case VERSIONS_UT_MAIN_USER:
		versions->video_channel = false;
		versions->con_side = true;
		break;

	case VERSIONS_UT_VIDEO_SERVER:
		versions->video_channel = true;
		versions->con_side = false;
		break;

	case VERSIONS_UT_VIDEO_USER:
		versions->video_channel = true;
		versions->con_side = true;
		break;

	}

	switch (raw_versions & VERSIONS_VIDEO_MASK) {
	case VERSIONS_VIDEO_DVI:
		versions->pcb_video_type = PCB_DVI_SL;
		break;

	case VERSIONS_VIDEO_DP_165:
		versions->pcb_video_type = PCB_DP_165MPIX;
		break;

	case VERSIONS_VIDEO_DP_300:
		versions->pcb_video_type = PCB_DP_300MPIX;
		break;

	case VERSIONS_VIDEO_HDMI:
		versions->pcb_video_type = PCB_HDMI;
		break;
	}

	versions->hw_version = raw_versions & VERSIONS_HW_VER_MASK;

	if (raw_versions & VERSIONS_SFP)
		versions->pcb_transmission_type = PCB_FIBER_3G;
	else
		versions->pcb_transmission_type = PCB_CAT_1G;

	return 0;
}

static int get_features(unsigned int fpga, struct fpga_features *features)
{
	enum {
		FEATURE_CARRIER_SPEED_2_5 = BIT(4),
		FEATURE_RAM_MASK = 0x7 << 5,
		FEATURE_RAM_DDR2_32BIT = 0x0 << 5,
		FEATURE_RAM_DDR3_32BIT = 0x1 << 5,
		FEATURE_RAM_DDR3_48BIT = 0x2 << 5,
		FEATURE_PCM_AUDIO_TX = BIT(9),
		FEATURE_PCM_AUDIO_RX = BIT(10),
		FEATURE_OSD = BIT(11),
		FEATURE_USB20 = BIT(12),
		FEATURE_COMPRESSION_MASK = 7 << 13,
		FEATURE_COMPRESSION_TYPE1 = 0x1 << 13,
		FEATURE_COMPRESSION_TYPE1_TYPE2 = 0x3 << 13,
		FEATURE_COMPRESSION_TYPE1_TYPE2_TYPE3 = 0x7 << 13,
	};

	enum {
		EXTENDED_FEATURE_SPDIF_AUDIO_TX = BIT(0),
		EXTENDED_FEATURE_SPDIF_AUDIO_RX = BIT(1),
		EXTENDED_FEATURE_RS232 = BIT(2),
		EXTENDED_FEATURE_COMPRESSION_PIPES = BIT(3),
		EXTENDED_FEATURE_INTERLACE = BIT(4),
	};

	u16 raw_features;
#ifdef GDSYS_LEGACY_DRIVERS
	u16 raw_extended_features;
#endif

	memset(features, 0, sizeof(struct fpga_features));

	FPGA_GET_REG(fpga, fpga_features, &raw_features);
#ifdef GDSYS_LEGACY_DRIVERS
	FPGA_GET_REG(fpga, fpga_ext_features, &raw_extended_features);
#endif

	features->video_channels = raw_features & 0x3;
	features->carriers = (raw_features >> 2) & 0x3;

	features->carrier_speed = (raw_features & FEATURE_CARRIER_SPEED_2_5)
		? CARRIER_SPEED_2_5G : CARRIER_SPEED_1G;

	switch (raw_features & FEATURE_RAM_MASK) {
	case FEATURE_RAM_DDR2_32BIT:
		features->ram_config = RAM_DDR2_32BIT_295MBPS;
		break;

	case FEATURE_RAM_DDR3_32BIT:
		features->ram_config = RAM_DDR3_32BIT_590MBPS;
		break;

	case FEATURE_RAM_DDR3_48BIT:
		features->ram_config = RAM_DDR3_48BIT_590MBPS;
		break;
	}

	features->pcm_tx = raw_features & FEATURE_PCM_AUDIO_TX;
	features->pcm_rx = raw_features & FEATURE_PCM_AUDIO_RX;
#ifdef GDSYS_LEGACY_DRIVERS
	features->spdif_tx = raw_extended_features & EXTENDED_FEATURE_SPDIF_AUDIO_TX;
	features->spdif_rx = raw_extended_features & EXTENDED_FEATURE_SPDIF_AUDIO_RX;
#endif

	features->usb2 = raw_features & FEATURE_USB20;
#ifdef GDSYS_LEGACY_DRIVERS
	features->rs232 = raw_extended_features & EXTENDED_FEATURE_RS232;
#endif

	features->compression_type1 = false;
	features->compression_type2 = false;
	features->compression_type3 = false;
	switch (raw_features & FEATURE_COMPRESSION_MASK) {
	case FEATURE_COMPRESSION_TYPE1_TYPE2_TYPE3:
		features->compression_type3 = true;
	case FEATURE_COMPRESSION_TYPE1_TYPE2:
		features->compression_type2 = true;
	case FEATURE_COMPRESSION_TYPE1:
		features->compression_type1 = true;
		break;
	}

#ifdef GDSYS_LEGACY_DRIVERS
	features->interlace = raw_extended_features & EXTENDED_FEATURE_INTERLACE;
#endif
	features->osd = raw_features & FEATURE_OSD;
#ifdef GDSYS_LEGACY_DRIVERS
	features->compression_pipes = raw_extended_features & EXTENDED_FEATURE_COMPRESSION_PIPES;
#endif

	return 0;
}

#include <linux/bitops.h>
#endif

bool ioep_fpga_has_osd(unsigned int fpga)
{
	struct fpga_features features;

	get_features(fpga, &features);

	return features.osd;
}

void ioep_fpga_print_info(unsigned int fpga)
{
	u16 fpga_version;
	struct fpga_versions versions;
	struct fpga_features features;

	FPGA_GET_REG(fpga, fpga_version, &fpga_version);
	get_versions(fpga, &versions);
	get_features(fpga, &features);

	if (versions.video_channel)
		printf("Videochannel");
	else
		printf("Mainchannel");

	if (versions.con_side)
		printf(" User");
	else
		printf(" Server");

// FIXME
#if 0
		if (versions & (1<<4))
			printf(" UC");
#endif

	switch(versions.pcb_transmission_type) {
	case PCB_CAT_1G:
	case PCB_CAT_10G:
		printf(" CAT");
		break;
	case PCB_FIBER_3G:
	case PCB_FIBER_10G:
		printf(" Fiber");
		break;
	};

	switch (versions.pcb_video_type) {
	case PCB_DVI_SL:
		printf(" DVI,");
		break;
	case PCB_DP_165MPIX:
		printf(" DP 165MPix/s,");
		break;
	case PCB_DP_300MPIX:
		printf(" DP 300MPix/s,");
		break;
	case PCB_HDMI:
		printf(" HDMI,");
		break;
	case PCB_DP_1_2:
		printf(" DP 1.2,");
		break;
	case PCB_HDMI_2_0:
		printf(" HDMI 2.0,");
		break;
	}

	printf(" FPGA V %d.%02d\n       features: ",
	       fpga_version / 100, fpga_version % 100);

	if (!features.compression_type1 &&
	    !features.compression_type2 &&
	    !features.compression_type3)
		printf("no compression, ");

	if (features.compression_type1)
		printf("type1, ");

	if (features.compression_type2)
		printf("type2, ");

	if (features.compression_type3)
		printf("type3, ");

	printf("%sosd", features.osd ? "" : "no ");

	if (features.pcm_rx && features.pcm_tx)
		printf(", pcm rx+tx");
	else if(features.pcm_rx)
		printf(", pcm rx");
	else if(features.pcm_tx)
		printf(", pcm tx");

	if (features.spdif_rx && features.spdif_tx)
		printf(", spdif rx+tx");
	else if(features.spdif_rx)
		printf(", spdif rx");
	else if(features.spdif_tx)
		printf(", spdif tx");

	puts(",\n       ");

	switch (features.sysclock) {
	case SYSCLK_147456:
		printf("clock 147.456 MHz");
		break;
	}

	switch (features.ram_config) {
	case RAM_DDR2_32BIT_295MBPS:
		printf(", RAM 32 bit DDR2");
		break;
	case RAM_DDR3_32BIT_590MBPS:
		printf(", RAM 32 bit DDR3");
		break;
	case RAM_DDR3_48BIT_590MBPS:
	case RAM_DDR3_48BIT_1800MBPS:
		printf(", RAM 48 bit DDR3");
		break;
	case RAM_DDR3_64BIT_1800MBPS:
		printf(", RAM 64 bit DDR3");
		break;
	}

	printf(", %d carrier(s)", features.carriers);

	switch(features.carrier_speed) {
	case CARRIER_SPEED_1G:
		printf(", 1Gbit/s");
		break;
	case CARRIER_SPEED_3G:
		printf(", 3Gbit/s");
		break;
	case CARRIER_SPEED_10G:
		printf(", 10Gbit/s");
		break;
	}

	printf(", %d video channel(s)\n", features.video_channels);
}

#endif /* CONFIG_GDSYS_LEGACY_DRIVERS */
