/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2011 Freescale Semiconductor, Inc.
 */

#ifndef __MXC_HDMI_H__
#define __MXC_HDMI_H__

#ifdef CONFIG_IMX_HDMI
void imx_enable_hdmi_phy(void);
void imx_setup_hdmi(void);
#endif

/*
 * Hdmi controller registers
 */
struct hdmi_regs {
	/*Identification Registers */
	u8 design_id;			/* 0x000 */
	u8 revision_id;			/* 0x001 */
	u8 product_id0;			/* 0x002 */
	u8 product_id1;			/* 0x003 */
	u8 config0_id;			/* 0x004 */
	u8 config1_id;			/* 0x005 */
	u8 config2_id;			/* 0x006 */
	u8 config3_id;			/* 0x007 */
	u8 reserved1[0xf8];
	/* Interrupt Registers */
	u8 ih_fc_stat0;			/* 0x100 */
	u8 ih_fc_stat1;			/* 0x101 */
	u8 ih_fc_stat2;			/* 0x102 */
	u8 ih_as_stat0;			/* 0x103 */
	u8 ih_phy_stat0;		/* 0x104 */
	u8 ih_i2cm_stat0;		/* 0x105 */
	u8 ih_cec_stat0;		/* 0x106 */
	u8 ih_vp_stat0;			/* 0x107 */
	u8 ih_i2cmphy_stat0;		/* 0x108 */
	u8 ih_ahbdmaaud_stat0;		/* 0x109 */
	u8 reserved2[0x76];
	u8 ih_mute_fc_stat0;		/* 0x180 */
	u8 ih_mute_fc_stat1;		/* 0x181 */
	u8 ih_mute_fc_stat2;		/* 0x182 */
	u8 ih_mute_as_stat0;		/* 0x183 */
	u8 ih_mute_phy_stat0;		/* 0x184 */
	u8 ih_mute_i2cm_stat0;		/* 0x185 */
	u8 ih_mute_cec_stat0;		/* 0x186 */
	u8 ih_mute_vp_stat0;		/* 0x187 */
	u8 ih_mute_i2cmphy_stat0;	/* 0x188 */
	u8 ih_mute_ahbdmaaud_stat0;	/* 0x189 */
	u8 reserved3[0x75];
	u8 ih_mute;			/* 0x1ff */
	/* Video Sample Registers */
	u8 tx_invid0;			/* 0x200 */
	u8 tx_instuffing;		/* 0x201 */
	u8 tx_gydata0;			/* 0x202 */
	u8 tx_gydata1;			/* 0x203 */
	u8 tx_rcrdata0;			/* 0x204 */
	u8 tx_rcrdata1;			/* 0x205 */
	u8 tx_bcbdata0;			/* 0x206 */
	u8 tx_bcbdata1;			/* 0x207 */
	u8 reserved4[0x5f8];
	/* Video Packetizer Registers */
	u8 vp_status;			/* 0x800 */
	u8 vp_pr_cd;			/* 0x801 */
	u8 vp_stuff;			/* 0x802 */
	u8 vp_remap;			/* 0x803 */
	u8 vp_conf;			/* 0x804 */
	u8 vp_stat;			/* 0x805 */
	u8 vp_int;			/* 0x806 */
	u8 vp_mask;			/* 0x807 */
	u8 vp_pol;			/* 0x808 */
	u8 reserved5[0x7f7];
	/* Frame Composer Registers */
	u8 fc_invidconf;		/* 0x1000 */
	u8 fc_inhactv0;			/* 0x1001 */
	u8 fc_inhactv1;			/* 0x1002 */
	u8 fc_inhblank0;		/* 0x1003 */
	u8 fc_inhblank1;		/* 0x1004 */
	u8 fc_invactv0;			/* 0x1005 */
	u8 fc_invactv1;			/* 0x1006 */
	u8 fc_invblank;			/* 0x1007 */
	u8 fc_hsyncindelay0;		/* 0x1008 */
	u8 fc_hsyncindelay1;		/* 0x1009 */
	u8 fc_hsyncinwidth0;		/* 0x100a */
	u8 fc_hsyncinwidth1;		/* 0x100b */
	u8 fc_vsyncindelay;		/* 0x100c */
	u8 fc_vsyncinwidth;		/* 0x100d */
	u8 fc_infreq0;			/* 0x100e */
	u8 fc_infreq1;			/* 0x100f */
	u8 fc_infreq2;			/* 0x1010 */
	u8 fc_ctrldur;			/* 0x1011 */
	u8 fc_exctrldur;		/* 0x1012 */
	u8 fc_exctrlspac;		/* 0x1013 */
	u8 fc_ch0pream;			/* 0x1014 */
	u8 fc_ch1pream;			/* 0x1015 */
	u8 fc_ch2pream;			/* 0x1016 */
	u8 fc_aviconf3;			/* 0x1017 */
	u8 fc_gcp;			/* 0x1018 */
	u8 fc_aviconf0;			/* 0x1019 */
	u8 fc_aviconf1;			/* 0x101a */
	u8 fc_aviconf2;			/* 0x101b */
	u8 fc_avivid;			/* 0x101c */
	u8 fc_avietb0;			/* 0x101d */
	u8 fc_avietb1;			/* 0x101e */
	u8 fc_avisbb0;			/* 0x101f */
	u8 fc_avisbb1;			/* 0x1020 */
	u8 fc_avielb0;			/* 0x1021 */
	u8 fc_avielb1;			/* 0x1022 */
	u8 fc_avisrb0;			/* 0x1023 */
	u8 fc_avisrb1;			/* 0x1024 */
	u8 fc_audiconf0;		/* 0x1025 */
	u8 fc_audiconf1;		/* 0x1026 */
	u8 fc_audiconf2;		/* 0x1027 */
	u8 fc_audiconf3;		/* 0x1028 */
	u8 fc_vsdieeeid0;		/* 0x1029 */
	u8 fc_vsdsize;			/* 0x102a */
	u8 reserved6[5];
	u8 fc_vsdieeeid1;		/* 0x1030 */
	u8 fc_vsdieeeid2;		/* 0x1031 */
	u8 fc_vsdpayload0;		/* 0x1032 */
	u8 fc_vsdpayload1;		/* 0x1033 */
	u8 fc_vsdpayload2;		/* 0x1034 */
	u8 fc_vsdpayload3;		/* 0x1035 */
	u8 fc_vsdpayload4;		/* 0x1036 */
	u8 fc_vsdpayload5;		/* 0x1037 */
	u8 fc_vsdpayload6;		/* 0x1038 */
	u8 fc_vsdpayload7;		/* 0x1039 */
	u8 fc_vsdpayload8;		/* 0x103a */
	u8 fc_vsdpayload9;		/* 0x103b */
	u8 fc_vsdpayload10;		/* 0x103c */
	u8 fc_vsdpayload11;		/* 0x103d */
	u8 fc_vsdpayload12;		/* 0x103e */
	u8 fc_vsdpayload13;		/* 0x103f */
	u8 fc_vsdpayload14;		/* 0x1040 */
	u8 fc_vsdpayload15;		/* 0x1041 */
	u8 fc_vsdpayload16;		/* 0x1042 */
	u8 fc_vsdpayload17;		/* 0x1043 */
	u8 fc_vsdpayload18;		/* 0x1044 */
	u8 fc_vsdpayload19;		/* 0x1045 */
	u8 fc_vsdpayload20;		/* 0x1046 */
	u8 fc_vsdpayload21;		/* 0x1047 */
	u8 fc_vsdpayload22;		/* 0x1048 */
	u8 fc_vsdpayload23;		/* 0x1049 */
	u8 fc_spdvendorname0;		/* 0x104a */
	u8 fc_spdvendorname1;		/* 0x104b */
	u8 fc_spdvendorname2;		/* 0x104c */
	u8 fc_spdvendorname3;		/* 0x104d */
	u8 fc_spdvendorname4;		/* 0x104e */
	u8 fc_spdvendorname5;		/* 0x104f */
	u8 fc_spdvendorname6;		/* 0x1050 */
	u8 fc_spdvendorname7;		/* 0x1051 */
	u8 fc_sdpproductname0;		/* 0x1052 */
	u8 fc_sdpproductname1;		/* 0x1053 */
	u8 fc_sdpproductname2;		/* 0x1054 */
	u8 fc_sdpproductname3;		/* 0x1055 */
	u8 fc_sdpproductname4;		/* 0x1056 */
	u8 fc_sdpproductname5;		/* 0x1057 */
	u8 fc_sdpproductname6;		/* 0x1058 */
	u8 fc_sdpproductname7;		/* 0x1059 */
	u8 fc_sdpproductname8;		/* 0x105a */
	u8 fc_sdpproductname9;		/* 0x105b */
	u8 fc_sdpproductname10;		/* 0x105c */
	u8 fc_sdpproductname11;		/* 0x105d */
	u8 fc_sdpproductname12;		/* 0x105e */
	u8 fc_sdpproductname13;		/* 0x105f */
	u8 fc_sdpproductname14;		/* 0x1060 */
	u8 fc_spdproductname15;		/* 0x1061 */
	u8 fc_spddeviceinf;		/* 0x1062 */
	u8 fc_audsconf;			/* 0x1063 */
	u8 fc_audsstat;			/* 0x1064 */
	u8 reserved7[0xb];
	u8 fc_datach0fill;		/* 0x1070 */
	u8 fc_datach1fill;		/* 0x1071 */
	u8 fc_datach2fill;		/* 0x1072 */
	u8 fc_ctrlqhigh;		/* 0x1073 */
	u8 fc_ctrlqlow;			/* 0x1074 */
	u8 fc_acp0;			/* 0x1075 */
	u8 fc_acp28;			/* 0x1076 */
	u8 fc_acp27;			/* 0x1077 */
	u8 fc_acp26;			/* 0x1078 */
	u8 fc_acp25;			/* 0x1079 */
	u8 fc_acp24;			/* 0x107a */
	u8 fc_acp23;			/* 0x107b */
	u8 fc_acp22;			/* 0x107c */
	u8 fc_acp21;			/* 0x107d */
	u8 fc_acp20;			/* 0x107e */
	u8 fc_acp19;			/* 0x107f */
	u8 fc_acp18;			/* 0x1080 */
	u8 fc_acp17;			/* 0x1081 */
	u8 fc_acp16;			/* 0x1082 */
	u8 fc_acp15;			/* 0x1083 */
	u8 fc_acp14;			/* 0x1084 */
	u8 fc_acp13;			/* 0x1085 */
	u8 fc_acp12;			/* 0x1086 */
	u8 fc_acp11;			/* 0x1087 */
	u8 fc_acp10;			/* 0x1088 */
	u8 fc_acp9;			/* 0x1089 */
	u8 fc_acp8;			/* 0x108a */
	u8 fc_acp7;			/* 0x108b */
	u8 fc_acp6;			/* 0x108c */
	u8 fc_acp5;			/* 0x108d */
	u8 fc_acp4;			/* 0x108e */
	u8 fc_acp3;			/* 0x108f */
	u8 fc_acp2;			/* 0x1090 */
	u8 fc_acp1;			/* 0x1091 */
	u8 fc_iscr1_0;			/* 0x1092 */
	u8 fc_iscr1_16;			/* 0x1093 */
	u8 fc_iscr1_15;			/* 0x1094 */
	u8 fc_iscr1_14;			/* 0x1095 */
	u8 fc_iscr1_13;			/* 0x1096 */
	u8 fc_iscr1_12;			/* 0x1097 */
	u8 fc_iscr1_11;			/* 0x1098 */
	u8 fc_iscr1_10;			/* 0x1099 */
	u8 fc_iscr1_9;			/* 0x109a */
	u8 fc_iscr1_8;			/* 0x109b */
	u8 fc_iscr1_7;			/* 0x109c */
	u8 fc_iscr1_6;			/* 0x109d */
	u8 fc_iscr1_5;			/* 0x109e */
	u8 fc_iscr1_4;			/* 0x109f */
	u8 fc_iscr1_3;			/* 0x10a0 */
	u8 fc_iscr1_2;			/* 0x10a1 */
	u8 fc_iscr1_1;			/* 0x10a2 */
	u8 fc_iscr2_15;			/* 0x10a3 */
	u8 fc_iscr2_14;			/* 0x10a4 */
	u8 fc_iscr2_13;			/* 0x10a5 */
	u8 fc_iscr2_12;			/* 0x10a6 */
	u8 fc_iscr2_11;			/* 0x10a7 */
	u8 fc_iscr2_10;			/* 0x10a8 */
	u8 fc_iscr2_9;			/* 0x10a9 */
	u8 fc_iscr2_8;			/* 0x10aa */
	u8 fc_iscr2_7;			/* 0x10ab */
	u8 fc_iscr2_6;			/* 0x10ac */
	u8 fc_iscr2_5;			/* 0x10ad */
	u8 fc_iscr2_4;			/* 0x10ae */
	u8 fc_iscr2_3;			/* 0x10af */
	u8 fc_iscr2_2;			/* 0x10b0 */
	u8 fc_iscr2_1;			/* 0x10b1 */
	u8 fc_iscr2_0;			/* 0x10b2 */
	u8 fc_datauto0;			/* 0x10b3 */
	u8 fc_datauto1;			/* 0x10b4 */
	u8 fc_datauto2;			/* 0x10b5 */
	u8 fc_datman;			/* 0x10b6 */
	u8 fc_datauto3;			/* 0x10b7 */
	u8 fc_rdrb0;			/* 0x10b8 */
	u8 fc_rdrb1;			/* 0x10b9 */
	u8 fc_rdrb2;			/* 0x10ba */
	u8 fc_rdrb3;			/* 0x10bb */
	u8 fc_rdrb4;			/* 0x10bc */
	u8 fc_rdrb5;			/* 0x10bd */
	u8 fc_rdrb6;			/* 0x10be */
	u8 fc_rdrb7;			/* 0x10bf */
	u8 reserved8[0x10];
	u8 fc_stat0;			/* 0x10d0 */
	u8 fc_int0;			/* 0x10d1 */
	u8 fc_mask0;			/* 0x10d2 */
	u8 fc_pol0;			/* 0x10d3 */
	u8 fc_stat1;			/* 0x10d4 */
	u8 fc_int1;			/* 0x10d5 */
	u8 fc_mask1;			/* 0x10d6 */
	u8 fc_pol1;			/* 0x10d7 */
	u8 fc_stat2;			/* 0x10d8 */
	u8 fc_int2;			/* 0x10d9 */
	u8 fc_mask2;			/* 0x10da */
	u8 fc_pol2;			/* 0x10db */
	u8 reserved9[0x4];
	u8 fc_prconf;			/* 0x10e0 */
	u8 reserved10[0x1f];
	u8 fc_gmd_stat;			/* 0x1100 */
	u8 fc_gmd_en;			/* 0x1101 */
	u8 fc_gmd_up;			/* 0x1102 */
	u8 fc_gmd_conf;			/* 0x1103 */
	u8 fc_gmd_hb;			/* 0x1104 */
	u8 fc_gmd_pb0;			/* 0x1105 */
	u8 fc_gmd_pb1;			/* 0x1106 */
	u8 fc_gmd_pb2;			/* 0x1107 */
	u8 fc_gmd_pb3;			/* 0x1108 */
	u8 fc_gmd_pb4;			/* 0x1109 */
	u8 fc_gmd_pb5;			/* 0x110a */
	u8 fc_gmd_pb6;			/* 0x110b */
	u8 fc_gmd_pb7;			/* 0x110c */
	u8 fc_gmd_pb8;			/* 0x110d */
	u8 fc_gmd_pb9;			/* 0x110e */
	u8 fc_gmd_pb10;			/* 0x110f */
	u8 fc_gmd_pb11;			/* 0x1110 */
	u8 fc_gmd_pb12;			/* 0x1111 */
	u8 fc_gmd_pb13;			/* 0x1112 */
	u8 fc_gmd_pb14;			/* 0x1113 */
	u8 fc_gmd_pb15;			/* 0x1114 */
	u8 fc_gmd_pb16;			/* 0x1115 */
	u8 fc_gmd_pb17;			/* 0x1116 */
	u8 fc_gmd_pb18;			/* 0x1117 */
	u8 fc_gmd_pb19;			/* 0x1118 */
	u8 fc_gmd_pb20;			/* 0x1119 */
	u8 fc_gmd_pb21;			/* 0x111a */
	u8 fc_gmd_pb22;			/* 0x111b */
	u8 fc_gmd_pb23;			/* 0x111c */
	u8 fc_gmd_pb24;			/* 0x111d */
	u8 fc_gmd_pb25;			/* 0x111e */
	u8 fc_gmd_pb26;			/* 0x111f */
	u8 fc_gmd_pb27;			/* 0x1120 */
	u8 reserved11[0xdf];
	u8 fc_dbgforce;			/* 0x1200 */
	u8 fc_dbgaud0ch0;		/* 0x1201 */
	u8 fc_dbgaud1ch0;		/* 0x1202 */
	u8 fc_dbgaud2ch0;		/* 0x1203 */
	u8 fc_dbgaud0ch1;		/* 0x1204 */
	u8 fc_dbgaud1ch1;		/* 0x1205 */
	u8 fc_dbgaud2ch1;		/* 0x1206 */
	u8 fc_dbgaud0ch2;		/* 0x1207 */
	u8 fc_dbgaud1ch2;		/* 0x1208 */
	u8 fc_dbgaud2ch2;		/* 0x1209 */
	u8 fc_dbgaud0ch3;		/* 0x120a */
	u8 fc_dbgaud1ch3;		/* 0x120b */
	u8 fc_dbgaud2ch3;		/* 0x120c */
	u8 fc_dbgaud0ch4;		/* 0x120d */
	u8 fc_dbgaud1ch4;		/* 0x120e */
	u8 fc_dbgaud2ch4;		/* 0x120f */
	u8 fc_dbgaud0ch5;		/* 0x1210 */
	u8 fc_dbgaud1ch5;		/* 0x1211 */
	u8 fc_dbgaud2ch5;		/* 0x1212 */
	u8 fc_dbgaud0ch6;		/* 0x1213 */
	u8 fc_dbgaud1ch6;		/* 0x1214 */
	u8 fc_dbgaud2ch6;		/* 0x1215 */
	u8 fc_dbgaud0ch7;		/* 0x1216 */
	u8 fc_dbgaud1ch7;		/* 0x1217 */
	u8 fc_dbgaud2ch7;		/* 0x1218 */
	u8 fc_dbgtmds0;			/* 0x1219 */
	u8 fc_dbgtmds1;			/* 0x121a */
	u8 fc_dbgtmds2;			/* 0x121b */
	u8 reserved12[0x1de4];
	/* Hdmi Source Phy Registers */
	u8 phy_conf0;			/* 0x3000 */
	u8 phy_tst0;			/* 0x3001 */
	u8 phy_tst1;			/* 0x3002 */
	u8 phy_tst2;			/* 0x3003 */
	u8 phy_stat0;			/* 0x3004 */
	u8 phy_int0;			/* 0x3005 */
	u8 phy_mask0;			/* 0x3006 */
	u8 phy_pol0;			/* 0x3007 */
	u8 reserved13[0x18];
	/* Hdmi Master Phy Registers */
	u8 phy_i2cm_slave_addr;		/* 0x3020 */
	u8 phy_i2cm_address_addr;	/* 0x3021 */
	u8 phy_i2cm_datao_1_addr;	/* 0x3022 */
	u8 phy_i2cm_datao_0_addr;	/* 0x3023 */
	u8 phy_i2cm_datai_1_addr;	/* 0x3024 */
	u8 phy_i2cm_datai_0_addr;	/* 0x3025 */
	u8 phy_i2cm_operation_addr;	/* 0x3026 */
	u8 phy_i2cm_int_addr;		/* 0x3027 */
	u8 phy_i2cm_ctlint_addr;	/* 0x3028 */
	u8 phy_i2cm_div_addr;		/* 0x3029 */
	u8 phy_i2cm_softrstz_addr;	/* 0x302a */
	u8 phy_i2cm_ss_scl_hcnt_1_addr;	/* 0x302b */
	u8 phy_i2cm_ss_scl_hcnt_0_addr;	/* 0x302c */
	u8 phy_i2cm_ss_scl_lcnt_1_addr;	/* 0x302d */
	u8 phy_i2cm_ss_scl_lcnt_0_addr;	/* 0x302e */
	u8 phy_i2cm_fs_scl_hcnt_1_addr;	/* 0x302f */
	u8 phy_i2cm_fs_scl_hcnt_0_addr;	/* 0x3030 */
	u8 phy_i2cm_fs_scl_lcnt_1_addr;	/* 0x3031 */
	u8 phy_i2cm_fs_scl_lcnt_0_addr;	/* 0x3032 */
	u8 reserved14[0xcd];
	/* Audio Sampler Registers */
	u8 aud_conf0;			/* 0x3100 */
	u8 aud_conf1;			/* 0x3101 */
	u8 aud_int;			/* 0x3102 */
	u8 aud_conf2;			/* 0x3103 */
	u8 reserved15[0xfc];
	u8 aud_n1;			/* 0x3200 */
	u8 aud_n2;			/* 0x3201 */
	u8 aud_n3;			/* 0x3202 */
	u8 aud_cts1;			/* 0x3203 */
	u8 aud_cts2;			/* 0x3204 */
	u8 aud_cts3;			/* 0x3205 */
	u8 aud_inputclkfs;		/* 0x3206 */
	u8 reserved16[0xfb];
	u8 aud_spdifint;		/* 0x3302 */
	u8 reserved17[0xfd];
	u8 aud_conf0_hbr;		/* 0x3400 */
	u8 aud_hbr_status;		/* 0x3401 */
	u8 aud_hbr_int;			/* 0x3402 */
	u8 aud_hbr_pol;			/* 0x3403 */
	u8 aud_hbr_mask;		/* 0x3404 */
	u8 reserved18[0xfb];
	/*
	 * Generic Parallel Audio Interface Registers
	 * Not used as GPAUD interface is not enabled in hw
	 */
	u8 gp_conf0;			/* 0x3500 */
	u8 gp_conf1;			/* 0x3501 */
	u8 gp_conf2;			/* 0x3502 */
	u8 gp_stat;			/* 0x3503 */
	u8 gp_int;			/* 0x3504 */
	u8 gp_mask;			/* 0x3505 */
	u8 gp_pol;			/* 0x3506 */
	u8 reserved19[0xf9];
	/* Audio DMA Registers */
	u8 ahb_dma_conf0;		/* 0x3600 */
	u8 ahb_dma_start;		/* 0x3601 */
	u8 ahb_dma_stop;		/* 0x3602 */
	u8 ahb_dma_thrsld;		/* 0x3603 */
	u8 ahb_dma_straddr0;		/* 0x3604 */
	u8 ahb_dma_straddr1;		/* 0x3605 */
	u8 ahb_dma_straddr2;		/* 0x3606 */
	u8 ahb_dma_straddr3;		/* 0x3607 */
	u8 ahb_dma_stpaddr0;		/* 0x3608 */
	u8 ahb_dma_stpaddr1;		/* 0x3609 */
	u8 ahb_dma_stpaddr2;		/* 0x360a */
	u8 ahb_dma_stpaddr3;		/* 0x360b */
	u8 ahb_dma_bstaddr0;		/* 0x360c */
	u8 ahb_dma_bstaddr1;		/* 0x360d */
	u8 ahb_dma_bstaddr2;		/* 0x360e */
	u8 ahb_dma_bstaddr3;		/* 0x360f */
	u8 ahb_dma_mblength0;		/* 0x3610 */
	u8 ahb_dma_mblength1;		/* 0x3611 */
	u8 ahb_dma_stat;		/* 0x3612 */
	u8 ahb_dma_int;			/* 0x3613 */
	u8 ahb_dma_mask;		/* 0x3614 */
	u8 ahb_dma_pol;			/* 0x3615 */
	u8 ahb_dma_conf1;		/* 0x3616 */
	u8 ahb_dma_buffstat;		/* 0x3617 */
	u8 ahb_dma_buffint;		/* 0x3618 */
	u8 ahb_dma_buffmask;		/* 0x3619 */
	u8 ahb_dma_buffpol;		/* 0x361a */
	u8 reserved20[0x9e5];
	/* Main Controller Registers */
	u8 mc_sfrdiv;			/* 0x4000 */
	u8 mc_clkdis;			/* 0x4001 */
	u8 mc_swrstz;			/* 0x4002 */
	u8 mc_opctrl;			/* 0x4003 */
	u8 mc_flowctrl;			/* 0x4004 */
	u8 mc_phyrstz;			/* 0x4005 */
	u8 mc_lockonclock;		/* 0x4006 */
	u8 mc_heacphy_rst;		/* 0x4007 */
	u8 reserved21[0xf8];
	/* Colorspace Converter Registers */
	u8 csc_cfg;			/* 0x4100 */
	u8 csc_scale;			/* 0x4101 */
	u8 csc_coef_a1_msb;		/* 0x4102 */
	u8 csc_coef_a1_lsb;		/* 0x4103 */
	u8 csc_coef_a2_msb;		/* 0x4104 */
	u8 csc_coef_a2_lsb;		/* 0x4105 */
	u8 csc_coef_a3_msb;		/* 0x4106 */
	u8 csc_coef_a3_lsb;		/* 0x4107 */
	u8 csc_coef_a4_msb;		/* 0x4108 */
	u8 csc_coef_a4_lsb;		/* 0x4109 */
	u8 csc_coef_b1_msb;		/* 0x410a */
	u8 csc_coef_b1_lsb;		/* 0x410b */
	u8 csc_coef_b2_msb;		/* 0x410c */
	u8 csc_coef_b2_lsb;		/* 0x410d */
	u8 csc_coef_b3_msb;		/* 0x410e */
	u8 csc_coef_b3_lsb;		/* 0x410f */
	u8 csc_coef_b4_msb;		/* 0x4110 */
	u8 csc_coef_b4_lsb;		/* 0x4111 */
	u8 csc_coef_c1_msb;		/* 0x4112 */
	u8 csc_coef_c1_lsb;		/* 0x4113 */
	u8 csc_coef_c2_msb;		/* 0x4114 */
	u8 csc_coef_c2_lsb;		/* 0x4115 */
	u8 csc_coef_c3_msb;		/* 0x4116 */
	u8 csc_coef_c3_lsb;		/* 0x4117 */
	u8 csc_coef_c4_msb;		/* 0x4118 */
	u8 csc_coef_c4_lsb;		/* 0x4119 */
	u8 reserved22[0xee6];
	/* HDCP Encryption Engine Registers */
	u8 a_hdcpcfg0;			/* 0x5000 */
	u8 a_hdcpcfg1;			/* 0x5001 */
	u8 a_hdcpobs0;			/* 0x5002 */
	u8 a_hdcpobs1;			/* 0x5003 */
	u8 a_hdcpobs2;			/* 0x5004 */
	u8 a_hdcpobs3;			/* 0x5005 */
	u8 a_apiintclr;			/* 0x5006 */
	u8 a_apiintstat;		/* 0x5007 */
	u8 a_apiintmsk;			/* 0x5008 */
	u8 a_vidpolcfg;			/* 0x5009 */
	u8 a_oesswcfg;			/* 0x500a */
	u8 a_timer1setup0;		/* 0x500b */
	u8 a_timer1setup1;		/* 0x500c */
	u8 a_timer2setup0;		/* 0x500d */
	u8 a_timer2setup1;		/* 0x500e */
	u8 a_100mscfg;			/* 0x500f */
	u8 a_2scfg0;			/* 0x5010 */
	u8 a_2scfg1;			/* 0x5011 */
	u8 a_5scfg0;			/* 0x5012 */
	u8 a_5scfg1;			/* 0x5013 */
	u8 a_srmverlsb;			/* 0x5014 */
	u8 a_srmvermsb;			/* 0x5015 */
	u8 a_srmctrl;			/* 0x5016 */
	u8 a_sfrsetup;			/* 0x5017 */
	u8 a_i2chsetup;			/* 0x5018 */
	u8 a_intsetup;			/* 0x5019 */
	u8 a_presetup;			/* 0x501a */
	u8 reserved23[0x5];
	u8 a_srm_base;			/* 0x5020 */
	u8 reserved24[0x2cdf];
	/* CEC Engine Registers */
	u8 cec_ctrl;			/* 0x7d00 */
	u8 cec_stat;			/* 0x7d01 */
	u8 cec_mask;			/* 0x7d02 */
	u8 cec_polarity;		/* 0x7d03 */
	u8 cec_int;			/* 0x7d04 */
	u8 cec_addr_l;			/* 0x7d05 */
	u8 cec_addr_h;			/* 0x7d06 */
	u8 cec_tx_cnt;			/* 0x7d07 */
	u8 cec_rx_cnt;			/* 0x7d08 */
	u8 reserved25[0x7];
	u8 cec_tx_data0;		/* 0x7d10 */
	u8 cec_tx_data1;		/* 0x7d11 */
	u8 cec_tx_data2;		/* 0x7d12 */
	u8 cec_tx_data3;		/* 0x7d13 */
	u8 cec_tx_data4;		/* 0x7d14 */
	u8 cec_tx_data5;		/* 0x7d15 */
	u8 cec_tx_data6;		/* 0x7d16 */
	u8 cec_tx_data7;		/* 0x7d17 */
	u8 cec_tx_data8;		/* 0x7d18 */
	u8 cec_tx_data9;		/* 0x7d19 */
	u8 cec_tx_data10;		/* 0x7d1a */
	u8 cec_tx_data11;		/* 0x7d1b */
	u8 cec_tx_data12;		/* 0x7d1c */
	u8 cec_tx_data13;		/* 0x7d1d */
	u8 cec_tx_data14;		/* 0x7d1e */
	u8 cec_tx_data15;		/* 0x7d1f */
	u8 cec_rx_data0;		/* 0x7d20 */
	u8 cec_rx_data1;		/* 0x7d21 */
	u8 cec_rx_data2;		/* 0x7d22 */
	u8 cec_rx_data3;		/* 0x7d23 */
	u8 cec_rx_data4;		/* 0x7d24 */
	u8 cec_rx_data5;		/* 0x7d25 */
	u8 cec_rx_data6;		/* 0x7d26 */
	u8 cec_rx_data7;		/* 0x7d27 */
	u8 cec_rx_data8;		/* 0x7d28 */
	u8 cec_rx_data9;		/* 0x7d29 */
	u8 cec_rx_data10;		/* 0x7d2a */
	u8 cec_rx_data11;		/* 0x7d2b */
	u8 cec_rx_data12;		/* 0x7d2c */
	u8 cec_rx_data13;		/* 0x7d2d */
	u8 cec_rx_data14;		/* 0x7d2e */
	u8 cec_rx_data15;		/* 0x7d2f */
	u8 cec_lock;			/* 0x7d30 */
	u8 cec_wkupctrl;		/* 0x7d31 */
	u8 reserved26[0xce];
	/* I2C Master Registers (E-DDC) */
	u8 i2cm_slave;			/* 0x7e00 */
	u8 i2cmess;			/* 0x7e01 */
	u8 i2cm_datao;			/* 0x7e02 */
	u8 i2cm_datai;			/* 0x7e03 */
	u8 i2cm_operation;		/* 0x7e04 */
	u8 i2cm_int;			/* 0x7e05 */
	u8 i2cm_ctlint;			/* 0x7e06 */
	u8 i2cm_div;			/* 0x7e07 */
	u8 i2cm_segaddr;		/* 0x7e08 */
	u8 i2cm_softrstz;		/* 0x7e09 */
	u8 i2cm_segptr;			/* 0x7e0a */
	u8 i2cm_ss_scl_hcnt_1_addr;	/* 0x7e0b */
	u8 i2cm_ss_scl_hcnt_0_addr;	/* 0x7e0c */
	u8 i2cm_ss_scl_lcnt_1_addr;	/* 0x7e0d */
	u8 i2cm_ss_scl_lcnt_0_addr;	/* 0x7e0e */
	u8 i2cm_fs_scl_hcnt_1_addr;	/* 0x7e0f */
	u8 i2cm_fs_scl_hcnt_0_addr;	/* 0x7e10 */
	u8 i2cm_fs_scl_lcnt_1_addr;	/* 0x7e11 */
	u8 i2cm_fs_scl_lcnt_0_addr;	/* 0x7e12 */
	u8 reserved27[0x1ed];
	/* Random Number Generator Registers (RNG) */
	u8 rng_base;			/* 0x8000 */
};

/*
 * Register field definitions
 */
enum {
/* IH_FC_INT2 field values */
	HDMI_IH_FC_INT2_OVERFLOW_MASK = 0x03,
	HDMI_IH_FC_INT2_LOW_PRIORITY_OVERFLOW = 0x02,
	HDMI_IH_FC_INT2_HIGH_PRIORITY_OVERFLOW = 0x01,

/* IH_FC_STAT2 field values */
	HDMI_IH_FC_STAT2_OVERFLOW_MASK = 0x03,
	HDMI_IH_FC_STAT2_LOW_PRIORITY_OVERFLOW = 0x02,
	HDMI_IH_FC_STAT2_HIGH_PRIORITY_OVERFLOW = 0x01,

/* IH_PHY_STAT0 field values */
	HDMI_IH_PHY_STAT0_RX_SENSE3 = 0x20,
	HDMI_IH_PHY_STAT0_RX_SENSE2 = 0x10,
	HDMI_IH_PHY_STAT0_RX_SENSE1 = 0x8,
	HDMI_IH_PHY_STAT0_RX_SENSE0 = 0x4,
	HDMI_IH_PHY_STAT0_TX_PHY_LOCK = 0x2,
	HDMI_IH_PHY_STAT0_HPD = 0x1,

/* IH_MUTE_I2CMPHY_STAT0 field values */
	HDMI_IH_MUTE_I2CMPHY_STAT0_I2CMPHYDONE = 0x2,
	HDMI_IH_MUTE_I2CMPHY_STAT0_I2CMPHYERROR = 0x1,

/* IH_AHBDMAAUD_STAT0 field values */
	HDMI_IH_AHBDMAAUD_STAT0_ERROR = 0x20,
	HDMI_IH_AHBDMAAUD_STAT0_LOST = 0x10,
	HDMI_IH_AHBDMAAUD_STAT0_RETRY = 0x08,
	HDMI_IH_AHBDMAAUD_STAT0_DONE = 0x04,
	HDMI_IH_AHBDMAAUD_STAT0_BUFFFULL = 0x02,
	HDMI_IH_AHBDMAAUD_STAT0_BUFFEMPTY = 0x01,

/* IH_MUTE_FC_STAT2 field values */
	HDMI_IH_MUTE_FC_STAT2_OVERFLOW_MASK = 0x03,
	HDMI_IH_MUTE_FC_STAT2_LOW_PRIORITY_OVERFLOW = 0x02,
	HDMI_IH_MUTE_FC_STAT2_HIGH_PRIORITY_OVERFLOW = 0x01,

/* IH_MUTE_AHBDMAAUD_STAT0 field values */
	HDMI_IH_MUTE_AHBDMAAUD_STAT0_ERROR = 0x20,
	HDMI_IH_MUTE_AHBDMAAUD_STAT0_LOST = 0x10,
	HDMI_IH_MUTE_AHBDMAAUD_STAT0_RETRY = 0x08,
	HDMI_IH_MUTE_AHBDMAAUD_STAT0_DONE = 0x04,
	HDMI_IH_MUTE_AHBDMAAUD_STAT0_BUFFFULL = 0x02,
	HDMI_IH_MUTE_AHBDMAAUD_STAT0_BUFFEMPTY = 0x01,

/* IH_MUTE field values */
	HDMI_IH_MUTE_MUTE_WAKEUP_INTERRUPT = 0x2,
	HDMI_IH_MUTE_MUTE_ALL_INTERRUPT = 0x1,

/* TX_INVID0 field values */
	HDMI_TX_INVID0_INTERNAL_DE_GENERATOR_MASK = 0x80,
	HDMI_TX_INVID0_INTERNAL_DE_GENERATOR_ENABLE = 0x80,
	HDMI_TX_INVID0_INTERNAL_DE_GENERATOR_DISABLE = 0x00,
	HDMI_TX_INVID0_VIDEO_MAPPING_MASK = 0x1F,
	HDMI_TX_INVID0_VIDEO_MAPPING_OFFSET = 0,

/* TX_INSTUFFING field values */
	HDMI_TX_INSTUFFING_BDBDATA_STUFFING_MASK = 0x4,
	HDMI_TX_INSTUFFING_BDBDATA_STUFFING_ENABLE = 0x4,
	HDMI_TX_INSTUFFING_BDBDATA_STUFFING_DISABLE = 0x0,
	HDMI_TX_INSTUFFING_RCRDATA_STUFFING_MASK = 0x2,
	HDMI_TX_INSTUFFING_RCRDATA_STUFFING_ENABLE = 0x2,
	HDMI_TX_INSTUFFING_RCRDATA_STUFFING_DISABLE = 0x0,
	HDMI_TX_INSTUFFING_GYDATA_STUFFING_MASK = 0x1,
	HDMI_TX_INSTUFFING_GYDATA_STUFFING_ENABLE = 0x1,
	HDMI_TX_INSTUFFING_GYDATA_STUFFING_DISABLE = 0x0,

/* VP_PR_CD field values */
	HDMI_VP_PR_CD_COLOR_DEPTH_MASK = 0xF0,
	HDMI_VP_PR_CD_COLOR_DEPTH_OFFSET = 4,
	HDMI_VP_PR_CD_DESIRED_PR_FACTOR_MASK = 0x0F,
	HDMI_VP_PR_CD_DESIRED_PR_FACTOR_OFFSET = 0,

/* VP_STUFF field values */
	HDMI_VP_STUFF_IDEFAULT_PHASE_MASK = 0x20,
	HDMI_VP_STUFF_IDEFAULT_PHASE_OFFSET = 5,
	HDMI_VP_STUFF_IFIX_PP_TO_LAST_MASK = 0x10,
	HDMI_VP_STUFF_IFIX_PP_TO_LAST_OFFSET = 4,
	HDMI_VP_STUFF_ICX_GOTO_P0_ST_MASK = 0x8,
	HDMI_VP_STUFF_ICX_GOTO_P0_ST_OFFSET = 3,
	HDMI_VP_STUFF_YCC422_STUFFING_MASK = 0x4,
	HDMI_VP_STUFF_YCC422_STUFFING_STUFFING_MODE = 0x4,
	HDMI_VP_STUFF_YCC422_STUFFING_DIRECT_MODE = 0x0,
	HDMI_VP_STUFF_PP_STUFFING_MASK = 0x2,
	HDMI_VP_STUFF_PP_STUFFING_STUFFING_MODE = 0x2,
	HDMI_VP_STUFF_PP_STUFFING_DIRECT_MODE = 0x0,
	HDMI_VP_STUFF_PR_STUFFING_MASK = 0x1,
	HDMI_VP_STUFF_PR_STUFFING_STUFFING_MODE = 0x1,
	HDMI_VP_STUFF_PR_STUFFING_DIRECT_MODE = 0x0,

/* VP_CONF field values */
	HDMI_VP_CONF_BYPASS_EN_MASK = 0x40,
	HDMI_VP_CONF_BYPASS_EN_ENABLE = 0x40,
	HDMI_VP_CONF_BYPASS_EN_DISABLE = 0x00,
	HDMI_VP_CONF_PP_EN_ENMASK = 0x20,
	HDMI_VP_CONF_PP_EN_ENABLE = 0x20,
	HDMI_VP_CONF_PP_EN_DISABLE = 0x00,
	HDMI_VP_CONF_PR_EN_MASK = 0x10,
	HDMI_VP_CONF_PR_EN_ENABLE = 0x10,
	HDMI_VP_CONF_PR_EN_DISABLE = 0x00,
	HDMI_VP_CONF_YCC422_EN_MASK = 0x8,
	HDMI_VP_CONF_YCC422_EN_ENABLE = 0x8,
	HDMI_VP_CONF_YCC422_EN_DISABLE = 0x0,
	HDMI_VP_CONF_BYPASS_SELECT_MASK = 0x4,
	HDMI_VP_CONF_BYPASS_SELECT_VID_PACKETIZER = 0x4,
	HDMI_VP_CONF_BYPASS_SELECT_PIX_REPEATER = 0x0,
	HDMI_VP_CONF_OUTPUT_SELECTOR_MASK = 0x3,
	HDMI_VP_CONF_OUTPUT_SELECTOR_BYPASS = 0x3,
	HDMI_VP_CONF_OUTPUT_SELECTOR_YCC422 = 0x1,
	HDMI_VP_CONF_OUTPUT_SELECTOR_PP = 0x0,

/* VP_REMAP field values */
	HDMI_VP_REMAP_MASK = 0x3,
	HDMI_VP_REMAP_YCC422_24bit = 0x2,
	HDMI_VP_REMAP_YCC422_20bit = 0x1,
	HDMI_VP_REMAP_YCC422_16bit = 0x0,

/* FC_INVIDCONF field values */
	HDMI_FC_INVIDCONF_HDCP_KEEPOUT_MASK = 0x80,
	HDMI_FC_INVIDCONF_HDCP_KEEPOUT_ACTIVE = 0x80,
	HDMI_FC_INVIDCONF_HDCP_KEEPOUT_INACTIVE = 0x00,
	HDMI_FC_INVIDCONF_VSYNC_IN_POLARITY_MASK = 0x40,
	HDMI_FC_INVIDCONF_VSYNC_IN_POLARITY_ACTIVE_HIGH = 0x40,
	HDMI_FC_INVIDCONF_VSYNC_IN_POLARITY_ACTIVE_LOW = 0x00,
	HDMI_FC_INVIDCONF_HSYNC_IN_POLARITY_MASK = 0x20,
	HDMI_FC_INVIDCONF_HSYNC_IN_POLARITY_ACTIVE_HIGH = 0x20,
	HDMI_FC_INVIDCONF_HSYNC_IN_POLARITY_ACTIVE_LOW = 0x00,
	HDMI_FC_INVIDCONF_DE_IN_POLARITY_MASK = 0x10,
	HDMI_FC_INVIDCONF_DE_IN_POLARITY_ACTIVE_HIGH = 0x10,
	HDMI_FC_INVIDCONF_DE_IN_POLARITY_ACTIVE_LOW = 0x00,
	HDMI_FC_INVIDCONF_DVI_MODEZ_MASK = 0x8,
	HDMI_FC_INVIDCONF_DVI_MODEZ_HDMI_MODE = 0x8,
	HDMI_FC_INVIDCONF_DVI_MODEZ_DVI_MODE = 0x0,
	HDMI_FC_INVIDCONF_R_V_BLANK_IN_OSC_MASK = 0x2,
	HDMI_FC_INVIDCONF_R_V_BLANK_IN_OSC_ACTIVE_HIGH = 0x2,
	HDMI_FC_INVIDCONF_R_V_BLANK_IN_OSC_ACTIVE_LOW = 0x0,
	HDMI_FC_INVIDCONF_IN_I_P_MASK = 0x1,
	HDMI_FC_INVIDCONF_IN_I_P_INTERLACED = 0x1,
	HDMI_FC_INVIDCONF_IN_I_P_PROGRESSIVE = 0x0,

/* FC_AUDICONF0 field values */
	HDMI_FC_AUDICONF0_CC_OFFSET = 4,
	HDMI_FC_AUDICONF0_CC_MASK = 0x70,
	HDMI_FC_AUDICONF0_CT_OFFSET = 0,
	HDMI_FC_AUDICONF0_CT_MASK = 0xF,

/* FC_AUDICONF1 field values */
	HDMI_FC_AUDICONF1_SS_OFFSET = 3,
	HDMI_FC_AUDICONF1_SS_MASK = 0x18,
	HDMI_FC_AUDICONF1_SF_OFFSET = 0,
	HDMI_FC_AUDICONF1_SF_MASK = 0x7,

/* FC_AUDICONF3 field values */
	HDMI_FC_AUDICONF3_LFEPBL_OFFSET = 5,
	HDMI_FC_AUDICONF3_LFEPBL_MASK = 0x60,
	HDMI_FC_AUDICONF3_DM_INH_OFFSET = 4,
	HDMI_FC_AUDICONF3_DM_INH_MASK = 0x10,
	HDMI_FC_AUDICONF3_LSV_OFFSET = 0,
	HDMI_FC_AUDICONF3_LSV_MASK = 0xF,

/* FC_AUDSCHNLS0 field values */
	HDMI_FC_AUDSCHNLS0_CGMSA_OFFSET = 4,
	HDMI_FC_AUDSCHNLS0_CGMSA_MASK = 0x30,
	HDMI_FC_AUDSCHNLS0_COPYRIGHT_OFFSET = 0,
	HDMI_FC_AUDSCHNLS0_COPYRIGHT_MASK = 0x01,

/* FC_AUDSCHNLS3-6 field values */
	HDMI_FC_AUDSCHNLS3_OIEC_CH0_OFFSET = 0,
	HDMI_FC_AUDSCHNLS3_OIEC_CH0_MASK = 0x0f,
	HDMI_FC_AUDSCHNLS3_OIEC_CH1_OFFSET = 4,
	HDMI_FC_AUDSCHNLS3_OIEC_CH1_MASK = 0xf0,
	HDMI_FC_AUDSCHNLS4_OIEC_CH2_OFFSET = 0,
	HDMI_FC_AUDSCHNLS4_OIEC_CH2_MASK = 0x0f,
	HDMI_FC_AUDSCHNLS4_OIEC_CH3_OFFSET = 4,
	HDMI_FC_AUDSCHNLS4_OIEC_CH3_MASK = 0xf0,

	HDMI_FC_AUDSCHNLS5_OIEC_CH0_OFFSET = 0,
	HDMI_FC_AUDSCHNLS5_OIEC_CH0_MASK = 0x0f,
	HDMI_FC_AUDSCHNLS5_OIEC_CH1_OFFSET = 4,
	HDMI_FC_AUDSCHNLS5_OIEC_CH1_MASK = 0xf0,
	HDMI_FC_AUDSCHNLS6_OIEC_CH2_OFFSET = 0,
	HDMI_FC_AUDSCHNLS6_OIEC_CH2_MASK = 0x0f,
	HDMI_FC_AUDSCHNLS6_OIEC_CH3_OFFSET = 4,
	HDMI_FC_AUDSCHNLS6_OIEC_CH3_MASK = 0xf0,

/* HDMI_FC_AUDSCHNLS7 field values */
	HDMI_FC_AUDSCHNLS7_ACCURACY_OFFSET = 4,
	HDMI_FC_AUDSCHNLS7_ACCURACY_MASK = 0x30,

/* HDMI_FC_AUDSCHNLS8 field values */
	HDMI_FC_AUDSCHNLS8_ORIGSAMPFREQ_MASK = 0xf0,
	HDMI_FC_AUDSCHNLS8_ORIGSAMPFREQ_OFFSET = 4,
	HDMI_FC_AUDSCHNLS8_WORDLEGNTH_MASK = 0x0f,
	HDMI_FC_AUDSCHNLS8_WORDLEGNTH_OFFSET = 0,

/* FC_AUDSCONF field values */
	HDMI_FC_AUDSCONF_AUD_PACKET_SAMPFIT_MASK = 0xF0,
	HDMI_FC_AUDSCONF_AUD_PACKET_SAMPFIT_OFFSET = 4,
	HDMI_FC_AUDSCONF_AUD_PACKET_LAYOUT_MASK = 0x1,
	HDMI_FC_AUDSCONF_AUD_PACKET_LAYOUT_OFFSET = 0,
	HDMI_FC_AUDSCONF_AUD_PACKET_LAYOUT_LAYOUT1 = 0x1,
	HDMI_FC_AUDSCONF_AUD_PACKET_LAYOUT_LAYOUT0 = 0x0,

/* FC_STAT2 field values */
	HDMI_FC_STAT2_OVERFLOW_MASK = 0x03,
	HDMI_FC_STAT2_LOW_PRIORITY_OVERFLOW = 0x02,
	HDMI_FC_STAT2_HIGH_PRIORITY_OVERFLOW = 0x01,

/* FC_INT2 field values */
	HDMI_FC_INT2_OVERFLOW_MASK = 0x03,
	HDMI_FC_INT2_LOW_PRIORITY_OVERFLOW = 0x02,
	HDMI_FC_INT2_HIGH_PRIORITY_OVERFLOW = 0x01,

/* FC_MASK2 field values */
	HDMI_FC_MASK2_OVERFLOW_MASK = 0x03,
	HDMI_FC_MASK2_LOW_PRIORITY_OVERFLOW = 0x02,
	HDMI_FC_MASK2_HIGH_PRIORITY_OVERFLOW = 0x01,

/* FC_PRCONF field values */
	HDMI_FC_PRCONF_INCOMING_PR_FACTOR_MASK = 0xF0,
	HDMI_FC_PRCONF_INCOMING_PR_FACTOR_OFFSET = 4,
	HDMI_FC_PRCONF_OUTPUT_PR_FACTOR_MASK = 0x0F,
	HDMI_FC_PRCONF_OUTPUT_PR_FACTOR_OFFSET = 0,

/* FC_AVICONF0-FC_AVICONF3 field values */
	HDMI_FC_AVICONF0_PIX_FMT_MASK = 0x03,
	HDMI_FC_AVICONF0_PIX_FMT_RGB = 0x00,
	HDMI_FC_AVICONF0_PIX_FMT_YCBCR422 = 0x01,
	HDMI_FC_AVICONF0_PIX_FMT_YCBCR444 = 0x02,
	HDMI_FC_AVICONF0_ACTIVE_FMT_MASK = 0x40,
	HDMI_FC_AVICONF0_ACTIVE_FMT_INFO_PRESENT = 0x40,
	HDMI_FC_AVICONF0_ACTIVE_FMT_NO_INFO = 0x00,
	HDMI_FC_AVICONF0_BAR_DATA_MASK = 0x0C,
	HDMI_FC_AVICONF0_BAR_DATA_NO_DATA = 0x00,
	HDMI_FC_AVICONF0_BAR_DATA_VERT_BAR = 0x04,
	HDMI_FC_AVICONF0_BAR_DATA_HORIZ_BAR = 0x08,
	HDMI_FC_AVICONF0_BAR_DATA_VERT_HORIZ_BAR = 0x0C,
	HDMI_FC_AVICONF0_SCAN_INFO_MASK = 0x30,
	HDMI_FC_AVICONF0_SCAN_INFO_OVERSCAN = 0x10,
	HDMI_FC_AVICONF0_SCAN_INFO_UNDERSCAN = 0x20,
	HDMI_FC_AVICONF0_SCAN_INFO_NODATA = 0x00,

	HDMI_FC_AVICONF1_ACTIVE_ASPECT_RATIO_MASK = 0x0F,
	HDMI_FC_AVICONF1_ACTIVE_ASPECT_RATIO_USE_CODED = 0x08,
	HDMI_FC_AVICONF1_ACTIVE_ASPECT_RATIO_4_3 = 0x09,
	HDMI_FC_AVICONF1_ACTIVE_ASPECT_RATIO_16_9 = 0x0A,
	HDMI_FC_AVICONF1_ACTIVE_ASPECT_RATIO_14_9 = 0x0B,
	HDMI_FC_AVICONF1_CODED_ASPECT_RATIO_MASK = 0x30,
	HDMI_FC_AVICONF1_CODED_ASPECT_RATIO_NO_DATA = 0x00,
	HDMI_FC_AVICONF1_CODED_ASPECT_RATIO_4_3 = 0x10,
	HDMI_FC_AVICONF1_CODED_ASPECT_RATIO_16_9 = 0x20,
	HDMI_FC_AVICONF1_COLORIMETRY_MASK = 0xC0,
	HDMI_FC_AVICONF1_COLORIMETRY_NO_DATA = 0x00,
	HDMI_FC_AVICONF1_COLORIMETRY_SMPTE = 0x40,
	HDMI_FC_AVICONF1_COLORIMETRY_ITUR = 0x80,
	HDMI_FC_AVICONF1_COLORIMETRY_EXTENDED_INFO = 0xC0,

	HDMI_FC_AVICONF2_SCALING_MASK = 0x03,
	HDMI_FC_AVICONF2_SCALING_NONE = 0x00,
	HDMI_FC_AVICONF2_SCALING_HORIZ = 0x01,
	HDMI_FC_AVICONF2_SCALING_VERT = 0x02,
	HDMI_FC_AVICONF2_SCALING_HORIZ_VERT = 0x03,
	HDMI_FC_AVICONF2_RGB_QUANT_MASK = 0x0C,
	HDMI_FC_AVICONF2_RGB_QUANT_DEFAULT = 0x00,
	HDMI_FC_AVICONF2_RGB_QUANT_LIMITED_RANGE = 0x04,
	HDMI_FC_AVICONF2_RGB_QUANT_FULL_RANGE = 0x08,
	HDMI_FC_AVICONF2_EXT_COLORIMETRY_MASK = 0x70,
	HDMI_FC_AVICONF2_EXT_COLORIMETRY_XVYCC601 = 0x00,
	HDMI_FC_AVICONF2_EXT_COLORIMETRY_XVYCC709 = 0x10,
	HDMI_FC_AVICONF2_EXT_COLORIMETRY_SYCC601 = 0x20,
	HDMI_FC_AVICONF2_EXT_COLORIMETRY_ADOBE_YCC601 = 0x30,
	HDMI_FC_AVICONF2_EXT_COLORIMETRY_ADOBE_RGB = 0x40,
	HDMI_FC_AVICONF2_IT_CONTENT_MASK = 0x80,
	HDMI_FC_AVICONF2_IT_CONTENT_NO_DATA = 0x00,
	HDMI_FC_AVICONF2_IT_CONTENT_VALID = 0x80,

	HDMI_FC_AVICONF3_IT_CONTENT_TYPE_MASK = 0x03,
	HDMI_FC_AVICONF3_IT_CONTENT_TYPE_GRAPHICS = 0x00,
	HDMI_FC_AVICONF3_IT_CONTENT_TYPE_PHOTO = 0x01,
	HDMI_FC_AVICONF3_IT_CONTENT_TYPE_CINEMA = 0x02,
	HDMI_FC_AVICONF3_IT_CONTENT_TYPE_GAME = 0x03,
	HDMI_FC_AVICONF3_QUANT_RANGE_MASK = 0x0C,
	HDMI_FC_AVICONF3_QUANT_RANGE_LIMITED = 0x00,
	HDMI_FC_AVICONF3_QUANT_RANGE_FULL = 0x04,

/* FC_DBGFORCE field values */
	HDMI_FC_DBGFORCE_FORCEAUDIO = 0x10,
	HDMI_FC_DBGFORCE_FORCEVIDEO = 0x1,

/* PHY_CONF0 field values */
	HDMI_PHY_CONF0_PDZ_MASK = 0x80,
	HDMI_PHY_CONF0_PDZ_OFFSET = 7,
	HDMI_PHY_CONF0_ENTMDS_MASK = 0x40,
	HDMI_PHY_CONF0_ENTMDS_OFFSET = 6,
	HDMI_PHY_CONF0_SPARECTRL = 0x20,
	HDMI_PHY_CONF0_GEN2_PDDQ_MASK = 0x10,
	HDMI_PHY_CONF0_GEN2_PDDQ_OFFSET = 4,
	HDMI_PHY_CONF0_GEN2_TXPWRON_MASK = 0x8,
	HDMI_PHY_CONF0_GEN2_TXPWRON_OFFSET = 3,
	HDMI_PHY_CONF0_GEN2_ENHPDRXSENSE_MASK = 0x4,
	HDMI_PHY_CONF0_GEN2_ENHPDRXSENSE_OFFSET = 2,
	HDMI_PHY_CONF0_SELDATAENPOL_MASK = 0x2,
	HDMI_PHY_CONF0_SELDATAENPOL_OFFSET = 1,
	HDMI_PHY_CONF0_SELDIPIF_MASK = 0x1,
	HDMI_PHY_CONF0_SELDIPIF_OFFSET = 0,

/* PHY_TST0 field values */
	HDMI_PHY_TST0_TSTCLR_MASK = 0x20,
	HDMI_PHY_TST0_TSTCLR_OFFSET = 5,
	HDMI_PHY_TST0_TSTEN_MASK = 0x10,
	HDMI_PHY_TST0_TSTEN_OFFSET = 4,
	HDMI_PHY_TST0_TSTCLK_MASK = 0x1,
	HDMI_PHY_TST0_TSTCLK_OFFSET = 0,

/* PHY_STAT0 field values */
	HDMI_PHY_RX_SENSE3 = 0x80,
	HDMI_PHY_RX_SENSE2 = 0x40,
	HDMI_PHY_RX_SENSE1 = 0x20,
	HDMI_PHY_RX_SENSE0 = 0x10,
	HDMI_PHY_HPD = 0x02,
	HDMI_PHY_TX_PHY_LOCK = 0x01,

/* Convenience macro RX_SENSE | HPD */
	HDMI_DVI_STAT = 0xF2,

/* PHY_I2CM_SLAVE_ADDR field values */
	HDMI_PHY_I2CM_SLAVE_ADDR_PHY_GEN2 = 0x69,
	HDMI_PHY_I2CM_SLAVE_ADDR_HEAC_PHY = 0x49,

/* PHY_I2CM_OPERATION_ADDR field values */
	HDMI_PHY_I2CM_OPERATION_ADDR_WRITE = 0x10,
	HDMI_PHY_I2CM_OPERATION_ADDR_READ = 0x1,

/* HDMI_PHY_I2CM_INT_ADDR */
	HDMI_PHY_I2CM_INT_ADDR_DONE_POL = 0x08,
	HDMI_PHY_I2CM_INT_ADDR_DONE_MASK = 0x04,

/* HDMI_PHY_I2CM_CTLINT_ADDR */
	HDMI_PHY_I2CM_CTLINT_ADDR_NAC_POL = 0x80,
	HDMI_PHY_I2CM_CTLINT_ADDR_NAC_MASK = 0x40,
	HDMI_PHY_I2CM_CTLINT_ADDR_ARBITRATION_POL = 0x08,
	HDMI_PHY_I2CM_CTLINT_ADDR_ARBITRATION_MASK = 0x04,

/* AUD_CTS3 field values */
	HDMI_AUD_CTS3_N_SHIFT_OFFSET = 5,
	HDMI_AUD_CTS3_N_SHIFT_MASK = 0xe0,
	HDMI_AUD_CTS3_N_SHIFT_1 = 0,
	HDMI_AUD_CTS3_N_SHIFT_16 = 0x20,
	HDMI_AUD_CTS3_N_SHIFT_32 = 0x40,
	HDMI_AUD_CTS3_N_SHIFT_64 = 0x60,
	HDMI_AUD_CTS3_N_SHIFT_128 = 0x80,
	HDMI_AUD_CTS3_N_SHIFT_256 = 0xa0,
	/* note that the CTS3 MANUAL bit has been removed
	   from our part. Can't set it, will read as 0. */
	HDMI_AUD_CTS3_CTS_MANUAL = 0x10,
	HDMI_AUD_CTS3_AUDCTS19_16_MASK = 0x0f,

/* AHB_DMA_CONF0 field values */
	HDMI_AHB_DMA_CONF0_SW_FIFO_RST_OFFSET = 7,
	HDMI_AHB_DMA_CONF0_SW_FIFO_RST_MASK = 0x80,
	HDMI_AHB_DMA_CONF0_HBR = 0x10,
	HDMI_AHB_DMA_CONF0_EN_HLOCK_OFFSET = 3,
	HDMI_AHB_DMA_CONF0_EN_HLOCK_MASK = 0x08,
	HDMI_AHB_DMA_CONF0_INCR_TYPE_OFFSET = 1,
	HDMI_AHB_DMA_CONF0_INCR_TYPE_MASK = 0x06,
	HDMI_AHB_DMA_CONF0_INCR4 = 0x0,
	HDMI_AHB_DMA_CONF0_INCR8 = 0x2,
	HDMI_AHB_DMA_CONF0_INCR16 = 0x4,
	HDMI_AHB_DMA_CONF0_BURST_MODE = 0x1,

/* HDMI_AHB_DMA_START field values */
	HDMI_AHB_DMA_START_START_OFFSET = 0,
	HDMI_AHB_DMA_START_START_MASK = 0x01,

/* HDMI_AHB_DMA_STOP field values */
	HDMI_AHB_DMA_STOP_STOP_OFFSET = 0,
	HDMI_AHB_DMA_STOP_STOP_MASK = 0x01,

/* AHB_DMA_STAT, AHB_DMA_INT, AHB_DMA_MASK, AHB_DMA_POL field values */
	HDMI_AHB_DMA_DONE = 0x80,
	HDMI_AHB_DMA_RETRY_SPLIT = 0x40,
	HDMI_AHB_DMA_LOSTOWNERSHIP = 0x20,
	HDMI_AHB_DMA_ERROR = 0x10,
	HDMI_AHB_DMA_FIFO_THREMPTY = 0x04,
	HDMI_AHB_DMA_FIFO_FULL = 0x02,
	HDMI_AHB_DMA_FIFO_EMPTY = 0x01,

/* AHB_DMA_BUFFSTAT, AHB_DMA_BUFFINT, AHB_DMA_BUFFMASK, AHB_DMA_BUFFPOL field values */
	HDMI_AHB_DMA_BUFFSTAT_FULL = 0x02,
	HDMI_AHB_DMA_BUFFSTAT_EMPTY = 0x01,

/* MC_CLKDIS field values */
	HDMI_MC_CLKDIS_HDCPCLK_DISABLE = 0x40,
	HDMI_MC_CLKDIS_CECCLK_DISABLE = 0x20,
	HDMI_MC_CLKDIS_CSCCLK_DISABLE = 0x10,
	HDMI_MC_CLKDIS_AUDCLK_DISABLE = 0x8,
	HDMI_MC_CLKDIS_PREPCLK_DISABLE = 0x4,
	HDMI_MC_CLKDIS_TMDSCLK_DISABLE = 0x2,
	HDMI_MC_CLKDIS_PIXELCLK_DISABLE = 0x1,

/* MC_SWRSTZ field values */
	HDMI_MC_SWRSTZ_TMDSSWRST_REQ = 0x02,

/* MC_FLOWCTRL field values */
	HDMI_MC_FLOWCTRL_FEED_THROUGH_OFF_MASK = 0x1,
	HDMI_MC_FLOWCTRL_FEED_THROUGH_OFF_CSC_IN_PATH = 0x1,
	HDMI_MC_FLOWCTRL_FEED_THROUGH_OFF_CSC_BYPASS = 0x0,

/* MC_PHYRSTZ field values */
	HDMI_MC_PHYRSTZ_ASSERT = 0x0,
	HDMI_MC_PHYRSTZ_DEASSERT = 0x1,

/* MC_HEACPHY_RST field values */
	HDMI_MC_HEACPHY_RST_ASSERT = 0x1,
	HDMI_MC_HEACPHY_RST_DEASSERT = 0x0,

/* CSC_CFG field values */
	HDMI_CSC_CFG_INTMODE_MASK = 0x30,
	HDMI_CSC_CFG_INTMODE_OFFSET = 4,
	HDMI_CSC_CFG_INTMODE_DISABLE = 0x00,
	HDMI_CSC_CFG_INTMODE_CHROMA_INT_FORMULA1 = 0x10,
	HDMI_CSC_CFG_INTMODE_CHROMA_INT_FORMULA2 = 0x20,
	HDMI_CSC_CFG_DECMODE_MASK = 0x3,
	HDMI_CSC_CFG_DECMODE_OFFSET = 0,
	HDMI_CSC_CFG_DECMODE_DISABLE = 0x0,
	HDMI_CSC_CFG_DECMODE_CHROMA_INT_FORMULA1 = 0x1,
	HDMI_CSC_CFG_DECMODE_CHROMA_INT_FORMULA2 = 0x2,
	HDMI_CSC_CFG_DECMODE_CHROMA_INT_FORMULA3 = 0x3,

/* CSC_SCALE field values */
	HDMI_CSC_SCALE_CSC_COLORDE_PTH_MASK = 0xF0,
	HDMI_CSC_SCALE_CSC_COLORDE_PTH_24BPP = 0x00,
	HDMI_CSC_SCALE_CSC_COLORDE_PTH_30BPP = 0x50,
	HDMI_CSC_SCALE_CSC_COLORDE_PTH_36BPP = 0x60,
	HDMI_CSC_SCALE_CSC_COLORDE_PTH_48BPP = 0x70,
	HDMI_CSC_SCALE_CSCSCALE_MASK = 0x03,

/* A_HDCPCFG0 field values */
	HDMI_A_HDCPCFG0_ELVENA_MASK = 0x80,
	HDMI_A_HDCPCFG0_ELVENA_ENABLE = 0x80,
	HDMI_A_HDCPCFG0_ELVENA_DISABLE = 0x00,
	HDMI_A_HDCPCFG0_I2CFASTMODE_MASK = 0x40,
	HDMI_A_HDCPCFG0_I2CFASTMODE_ENABLE = 0x40,
	HDMI_A_HDCPCFG0_I2CFASTMODE_DISABLE = 0x00,
	HDMI_A_HDCPCFG0_BYPENCRYPTION_MASK = 0x20,
	HDMI_A_HDCPCFG0_BYPENCRYPTION_ENABLE = 0x20,
	HDMI_A_HDCPCFG0_BYPENCRYPTION_DISABLE = 0x00,
	HDMI_A_HDCPCFG0_SYNCRICHECK_MASK = 0x10,
	HDMI_A_HDCPCFG0_SYNCRICHECK_ENABLE = 0x10,
	HDMI_A_HDCPCFG0_SYNCRICHECK_DISABLE = 0x00,
	HDMI_A_HDCPCFG0_AVMUTE_MASK = 0x8,
	HDMI_A_HDCPCFG0_AVMUTE_ENABLE = 0x8,
	HDMI_A_HDCPCFG0_AVMUTE_DISABLE = 0x0,
	HDMI_A_HDCPCFG0_RXDETECT_MASK = 0x4,
	HDMI_A_HDCPCFG0_RXDETECT_ENABLE = 0x4,
	HDMI_A_HDCPCFG0_RXDETECT_DISABLE = 0x0,
	HDMI_A_HDCPCFG0_EN11FEATURE_MASK = 0x2,
	HDMI_A_HDCPCFG0_EN11FEATURE_ENABLE = 0x2,
	HDMI_A_HDCPCFG0_EN11FEATURE_DISABLE = 0x0,
	HDMI_A_HDCPCFG0_HDMIDVI_MASK = 0x1,
	HDMI_A_HDCPCFG0_HDMIDVI_HDMI = 0x1,
	HDMI_A_HDCPCFG0_HDMIDVI_DVI = 0x0,

/* A_HDCPCFG1 field values */
	HDMI_A_HDCPCFG1_DISSHA1CHECK_MASK = 0x8,
	HDMI_A_HDCPCFG1_DISSHA1CHECK_DISABLE = 0x8,
	HDMI_A_HDCPCFG1_DISSHA1CHECK_ENABLE = 0x0,
	HDMI_A_HDCPCFG1_PH2UPSHFTENC_MASK = 0x4,
	HDMI_A_HDCPCFG1_PH2UPSHFTENC_ENABLE = 0x4,
	HDMI_A_HDCPCFG1_PH2UPSHFTENC_DISABLE = 0x0,
	HDMI_A_HDCPCFG1_ENCRYPTIONDISABLE_MASK = 0x2,
	HDMI_A_HDCPCFG1_ENCRYPTIONDISABLE_DISABLE = 0x2,
	HDMI_A_HDCPCFG1_ENCRYPTIONDISABLE_ENABLE = 0x0,
	HDMI_A_HDCPCFG1_SWRESET_MASK = 0x1,
	HDMI_A_HDCPCFG1_SWRESET_ASSERT = 0x0,

/* A_VIDPOLCFG field values */
	HDMI_A_VIDPOLCFG_UNENCRYPTCONF_MASK = 0x60,
	HDMI_A_VIDPOLCFG_UNENCRYPTCONF_OFFSET = 5,
	HDMI_A_VIDPOLCFG_DATAENPOL_MASK = 0x10,
	HDMI_A_VIDPOLCFG_DATAENPOL_ACTIVE_HIGH = 0x10,
	HDMI_A_VIDPOLCFG_DATAENPOL_ACTIVE_LOW = 0x0,
	HDMI_A_VIDPOLCFG_VSYNCPOL_MASK = 0x8,
	HDMI_A_VIDPOLCFG_VSYNCPOL_ACTIVE_HIGH = 0x8,
	HDMI_A_VIDPOLCFG_VSYNCPOL_ACTIVE_LOW = 0x0,
	HDMI_A_VIDPOLCFG_HSYNCPOL_MASK = 0x2,
	HDMI_A_VIDPOLCFG_HSYNCPOL_ACTIVE_HIGH = 0x2,
	HDMI_A_VIDPOLCFG_HSYNCPOL_ACTIVE_LOW = 0x0,
};

#endif /* __MXC_HDMI_H__ */
