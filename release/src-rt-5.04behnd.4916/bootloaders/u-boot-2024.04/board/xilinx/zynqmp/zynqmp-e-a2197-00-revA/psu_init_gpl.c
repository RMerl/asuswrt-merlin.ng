// SPDX-License-Identifier: GPL-2.0+
/*
 * (c) Copyright 2015 Xilinx, Inc. All rights reserved.
 */

#include <asm/arch/psu_init_gpl.h>
#include <xil_io.h>

static int serdes_rst_seq(u32 pllsel, u32 lane3_protocol, u32 lane3_rate,
			  u32 lane2_protocol, u32 lane2_rate,
			  u32 lane1_protocol, u32 lane1_rate,
			  u32 lane0_protocol, u32 lane0_rate)
{
	Xil_Out32(0xFD410098, 0x00000000);
	Xil_Out32(0xFD401010, 0x00000040);
	Xil_Out32(0xFD405010, 0x00000040);
	Xil_Out32(0xFD409010, 0x00000040);
	Xil_Out32(0xFD40D010, 0x00000040);
	Xil_Out32(0xFD402084, 0x00000080);
	Xil_Out32(0xFD406084, 0x00000080);
	Xil_Out32(0xFD40A084, 0x00000080);
	Xil_Out32(0xFD40E084, 0x00000080);
	Xil_Out32(0xFD410098, 0x00000004);
	mask_delay(50);
	if (lane0_rate == 1)
		Xil_Out32(0xFD410098, 0x0000000E);
	Xil_Out32(0xFD410098, 0x00000006);
	if (lane0_rate == 1) {
		Xil_Out32(0xFD40000C, 0x00000004);
		Xil_Out32(0xFD40400C, 0x00000004);
		Xil_Out32(0xFD40800C, 0x00000004);
		Xil_Out32(0xFD40C00C, 0x00000004);
		Xil_Out32(0xFD410098, 0x00000007);
		mask_delay(400);
		Xil_Out32(0xFD40000C, 0x0000000C);
		Xil_Out32(0xFD40400C, 0x0000000C);
		Xil_Out32(0xFD40800C, 0x0000000C);
		Xil_Out32(0xFD40C00C, 0x0000000C);
		mask_delay(15);
		Xil_Out32(0xFD410098, 0x0000000F);
		mask_delay(100);
	}
	if (pllsel == 0)
		mask_poll(0xFD4023E4, 0x00000010U);
	if (pllsel == 1)
		mask_poll(0xFD4063E4, 0x00000010U);
	if (pllsel == 2)
		mask_poll(0xFD40A3E4, 0x00000010U);
	if (pllsel == 3)
		mask_poll(0xFD40E3E4, 0x00000010U);
	mask_delay(50);
	Xil_Out32(0xFD401010, 0x000000C0);
	Xil_Out32(0xFD405010, 0x000000C0);
	Xil_Out32(0xFD409010, 0x000000C0);
	Xil_Out32(0xFD40D010, 0x000000C0);
	Xil_Out32(0xFD401010, 0x00000080);
	Xil_Out32(0xFD405010, 0x00000080);
	Xil_Out32(0xFD409010, 0x00000080);
	Xil_Out32(0xFD40D010, 0x00000080);

	Xil_Out32(0xFD402084, 0x000000C0);
	Xil_Out32(0xFD406084, 0x000000C0);
	Xil_Out32(0xFD40A084, 0x000000C0);
	Xil_Out32(0xFD40E084, 0x000000C0);
	mask_delay(50);
	Xil_Out32(0xFD402084, 0x00000080);
	Xil_Out32(0xFD406084, 0x00000080);
	Xil_Out32(0xFD40A084, 0x00000080);
	Xil_Out32(0xFD40E084, 0x00000080);
	mask_delay(50);
	Xil_Out32(0xFD401010, 0x00000000);
	Xil_Out32(0xFD405010, 0x00000000);
	Xil_Out32(0xFD409010, 0x00000000);
	Xil_Out32(0xFD40D010, 0x00000000);
	Xil_Out32(0xFD402084, 0x00000000);
	Xil_Out32(0xFD406084, 0x00000000);
	Xil_Out32(0xFD40A084, 0x00000000);
	Xil_Out32(0xFD40E084, 0x00000000);
	mask_delay(500);
	return 1;
}

static int serdes_bist_static_settings(u32 lane_active)
{
	if (lane_active == 0) {
		Xil_Out32(0xFD403004, (Xil_In32(0xFD403004) & 0xFFFFFF1F));
		Xil_Out32(0xFD403068, 0x1);
		Xil_Out32(0xFD40306C, 0x1);
		Xil_Out32(0xFD4010AC, 0x0020);
		Xil_Out32(0xFD403008, 0x0);
		Xil_Out32(0xFD40300C, 0xF4);
		Xil_Out32(0xFD403010, 0x0);
		Xil_Out32(0xFD403014, 0x0);
		Xil_Out32(0xFD403018, 0x00);
		Xil_Out32(0xFD40301C, 0xFB);
		Xil_Out32(0xFD403020, 0xFF);
		Xil_Out32(0xFD403024, 0x0);
		Xil_Out32(0xFD403028, 0x00);
		Xil_Out32(0xFD40302C, 0x00);
		Xil_Out32(0xFD403030, 0x4A);
		Xil_Out32(0xFD403034, 0x4A);
		Xil_Out32(0xFD403038, 0x4A);
		Xil_Out32(0xFD40303C, 0x4A);
		Xil_Out32(0xFD403040, 0x0);
		Xil_Out32(0xFD403044, 0x14);
		Xil_Out32(0xFD403048, 0x02);
		Xil_Out32(0xFD403004, (Xil_In32(0xFD403004) & 0xFFFFFF1F));
	}
	if (lane_active == 1) {
		Xil_Out32(0xFD407004, (Xil_In32(0xFD407004) & 0xFFFFFF1F));
		Xil_Out32(0xFD407068, 0x1);
		Xil_Out32(0xFD40706C, 0x1);
		Xil_Out32(0xFD4050AC, 0x0020);
		Xil_Out32(0xFD407008, 0x0);
		Xil_Out32(0xFD40700C, 0xF4);
		Xil_Out32(0xFD407010, 0x0);
		Xil_Out32(0xFD407014, 0x0);
		Xil_Out32(0xFD407018, 0x00);
		Xil_Out32(0xFD40701C, 0xFB);
		Xil_Out32(0xFD407020, 0xFF);
		Xil_Out32(0xFD407024, 0x0);
		Xil_Out32(0xFD407028, 0x00);
		Xil_Out32(0xFD40702C, 0x00);
		Xil_Out32(0xFD407030, 0x4A);
		Xil_Out32(0xFD407034, 0x4A);
		Xil_Out32(0xFD407038, 0x4A);
		Xil_Out32(0xFD40703C, 0x4A);
		Xil_Out32(0xFD407040, 0x0);
		Xil_Out32(0xFD407044, 0x14);
		Xil_Out32(0xFD407048, 0x02);
		Xil_Out32(0xFD407004, (Xil_In32(0xFD407004) & 0xFFFFFF1F));
	}

	if (lane_active == 2) {
		Xil_Out32(0xFD40B004, (Xil_In32(0xFD40B004) & 0xFFFFFF1F));
		Xil_Out32(0xFD40B068, 0x1);
		Xil_Out32(0xFD40B06C, 0x1);
		Xil_Out32(0xFD4090AC, 0x0020);
		Xil_Out32(0xFD40B008, 0x0);
		Xil_Out32(0xFD40B00C, 0xF4);
		Xil_Out32(0xFD40B010, 0x0);
		Xil_Out32(0xFD40B014, 0x0);
		Xil_Out32(0xFD40B018, 0x00);
		Xil_Out32(0xFD40B01C, 0xFB);
		Xil_Out32(0xFD40B020, 0xFF);
		Xil_Out32(0xFD40B024, 0x0);
		Xil_Out32(0xFD40B028, 0x00);
		Xil_Out32(0xFD40B02C, 0x00);
		Xil_Out32(0xFD40B030, 0x4A);
		Xil_Out32(0xFD40B034, 0x4A);
		Xil_Out32(0xFD40B038, 0x4A);
		Xil_Out32(0xFD40B03C, 0x4A);
		Xil_Out32(0xFD40B040, 0x0);
		Xil_Out32(0xFD40B044, 0x14);
		Xil_Out32(0xFD40B048, 0x02);
		Xil_Out32(0xFD40B004, (Xil_In32(0xFD40B004) & 0xFFFFFF1F));
	}

	if (lane_active == 3) {
		Xil_Out32(0xFD40F004, (Xil_In32(0xFD40F004) & 0xFFFFFF1F));
		Xil_Out32(0xFD40F068, 0x1);
		Xil_Out32(0xFD40F06C, 0x1);
		Xil_Out32(0xFD40D0AC, 0x0020);
		Xil_Out32(0xFD40F008, 0x0);
		Xil_Out32(0xFD40F00C, 0xF4);
		Xil_Out32(0xFD40F010, 0x0);
		Xil_Out32(0xFD40F014, 0x0);
		Xil_Out32(0xFD40F018, 0x00);
		Xil_Out32(0xFD40F01C, 0xFB);
		Xil_Out32(0xFD40F020, 0xFF);
		Xil_Out32(0xFD40F024, 0x0);
		Xil_Out32(0xFD40F028, 0x00);
		Xil_Out32(0xFD40F02C, 0x00);
		Xil_Out32(0xFD40F030, 0x4A);
		Xil_Out32(0xFD40F034, 0x4A);
		Xil_Out32(0xFD40F038, 0x4A);
		Xil_Out32(0xFD40F03C, 0x4A);
		Xil_Out32(0xFD40F040, 0x0);
		Xil_Out32(0xFD40F044, 0x14);
		Xil_Out32(0xFD40F048, 0x02);
		Xil_Out32(0xFD40F004, (Xil_In32(0xFD40F004) & 0xFFFFFF1F));
	}
	return 1;
}

static int serdes_bist_run(u32 lane_active)
{
	if (lane_active == 0) {
		psu_mask_write(0xFD410044, 0x00000003U, 0x00000000U);
		psu_mask_write(0xFD410040, 0x00000003U, 0x00000000U);
		psu_mask_write(0xFD410038, 0x00000007U, 0x00000001U);
		Xil_Out32(0xFD4010AC, 0x0020);
		Xil_Out32(0xFD403004, (Xil_In32(0xFD403004) | 0x1));
	}
	if (lane_active == 1) {
		psu_mask_write(0xFD410044, 0x0000000CU, 0x00000000U);
		psu_mask_write(0xFD410040, 0x0000000CU, 0x00000000U);
		psu_mask_write(0xFD410038, 0x00000070U, 0x00000010U);
		Xil_Out32(0xFD4050AC, 0x0020);
		Xil_Out32(0xFD407004, (Xil_In32(0xFD407004) | 0x1));
	}
	if (lane_active == 2) {
		psu_mask_write(0xFD410044, 0x00000030U, 0x00000000U);
		psu_mask_write(0xFD410040, 0x00000030U, 0x00000000U);
		psu_mask_write(0xFD41003C, 0x00000007U, 0x00000001U);
		Xil_Out32(0xFD4090AC, 0x0020);
		Xil_Out32(0xFD40B004, (Xil_In32(0xFD40B004) | 0x1));
	}
	if (lane_active == 3) {
		psu_mask_write(0xFD410040, 0x000000C0U, 0x00000000U);
		psu_mask_write(0xFD410044, 0x000000C0U, 0x00000000U);
		psu_mask_write(0xFD41003C, 0x00000070U, 0x00000010U);
		Xil_Out32(0xFD40D0AC, 0x0020);
		Xil_Out32(0xFD40F004, (Xil_In32(0xFD40F004) | 0x1));
	}
	mask_delay(100);
	return 1;
}

static int serdes_bist_result(u32 lane_active)
{
	u32 pkt_cnt_l0, pkt_cnt_h0, err_cnt_l0, err_cnt_h0;

	if (lane_active == 0) {
		pkt_cnt_l0 = Xil_In32(0xFD40304C);
		pkt_cnt_h0 = Xil_In32(0xFD403050);
		err_cnt_l0 = Xil_In32(0xFD403054);
		err_cnt_h0 = Xil_In32(0xFD403058);
	}
	if (lane_active == 1) {
		pkt_cnt_l0 = Xil_In32(0xFD40704C);
		pkt_cnt_h0 = Xil_In32(0xFD407050);
		err_cnt_l0 = Xil_In32(0xFD407054);
		err_cnt_h0 = Xil_In32(0xFD407058);
	}
	if (lane_active == 2) {
		pkt_cnt_l0 = Xil_In32(0xFD40B04C);
		pkt_cnt_h0 = Xil_In32(0xFD40B050);
		err_cnt_l0 = Xil_In32(0xFD40B054);
		err_cnt_h0 = Xil_In32(0xFD40B058);
	}
	if (lane_active == 3) {
		pkt_cnt_l0 = Xil_In32(0xFD40F04C);
		pkt_cnt_h0 = Xil_In32(0xFD40F050);
		err_cnt_l0 = Xil_In32(0xFD40F054);
		err_cnt_h0 = Xil_In32(0xFD40F058);
	}
	if (lane_active == 0)
		Xil_Out32(0xFD403004, 0x0);
	if (lane_active == 1)
		Xil_Out32(0xFD407004, 0x0);
	if (lane_active == 2)
		Xil_Out32(0xFD40B004, 0x0);
	if (lane_active == 3)
		Xil_Out32(0xFD40F004, 0x0);
	if (err_cnt_l0 > 0 || err_cnt_h0 > 0 ||
	    (pkt_cnt_l0 == 0 && pkt_cnt_h0 == 0))
		return 0;
	return 1;
}

static int serdes_illcalib_pcie_gen1(u32 pllsel, u32 lane3_protocol,
				     u32 lane3_rate, u32 lane2_protocol,
				     u32 lane2_rate, u32 lane1_protocol,
				     u32 lane1_rate, u32 lane0_protocol,
				     u32 lane0_rate, u32 gen2_calib)
{
	u64 tempbistresult;
	u32 currbistresult[4];
	u32 prevbistresult[4];
	u32 itercount = 0;
	u32 ill12_val[4], ill1_val[4];
	u32 loop = 0;
	u32 iterresult[8];
	u32 meancount[4];
	u32 bistpasscount[4];
	u32 meancountalt[4];
	u32 meancountalt_bistpasscount[4];
	u32 lane0_active;
	u32 lane1_active;
	u32 lane2_active;
	u32 lane3_active;

	lane0_active = (lane0_protocol == 1);
	lane1_active = (lane1_protocol == 1);
	lane2_active = (lane2_protocol == 1);
	lane3_active = (lane3_protocol == 1);
	for (loop = 0; loop <= 3; loop++) {
		iterresult[loop] = 0;
		iterresult[loop + 4] = 0;
		meancountalt[loop] = 0;
		meancountalt_bistpasscount[loop] = 0;
		meancount[loop] = 0;
		prevbistresult[loop] = 0;
		bistpasscount[loop] = 0;
	}
	itercount = 0;
	if (lane0_active)
		serdes_bist_static_settings(0);
	if (lane1_active)
		serdes_bist_static_settings(1);
	if (lane2_active)
		serdes_bist_static_settings(2);
	if (lane3_active)
		serdes_bist_static_settings(3);
	do {
		if (gen2_calib != 1) {
			if (lane0_active == 1)
				ill1_val[0] = ((0x04 + itercount * 8) % 0x100);
			if (lane0_active == 1)
				ill12_val[0] =
				    ((0x04 + itercount * 8) >=
				     0x100) ? 0x10 : 0x00;
			if (lane1_active == 1)
				ill1_val[1] = ((0x04 + itercount * 8) % 0x100);
			if (lane1_active == 1)
				ill12_val[1] =
				    ((0x04 + itercount * 8) >=
				     0x100) ? 0x10 : 0x00;
			if (lane2_active == 1)
				ill1_val[2] = ((0x04 + itercount * 8) % 0x100);
			if (lane2_active == 1)
				ill12_val[2] =
				    ((0x04 + itercount * 8) >=
				     0x100) ? 0x10 : 0x00;
			if (lane3_active == 1)
				ill1_val[3] = ((0x04 + itercount * 8) % 0x100);
			if (lane3_active == 1)
				ill12_val[3] =
				    ((0x04 + itercount * 8) >=
				     0x100) ? 0x10 : 0x00;

			if (lane0_active == 1)
				Xil_Out32(0xFD401924, ill1_val[0]);
			if (lane0_active == 1)
				psu_mask_write(0xFD401990, 0x000000F0U,
					       ill12_val[0]);
			if (lane1_active == 1)
				Xil_Out32(0xFD405924, ill1_val[1]);
			if (lane1_active == 1)
				psu_mask_write(0xFD405990, 0x000000F0U,
					       ill12_val[1]);
			if (lane2_active == 1)
				Xil_Out32(0xFD409924, ill1_val[2]);
			if (lane2_active == 1)
				psu_mask_write(0xFD409990, 0x000000F0U,
					       ill12_val[2]);
			if (lane3_active == 1)
				Xil_Out32(0xFD40D924, ill1_val[3]);
			if (lane3_active == 1)
				psu_mask_write(0xFD40D990, 0x000000F0U,
					       ill12_val[3]);
		}
		if (gen2_calib == 1) {
			if (lane0_active == 1)
				ill1_val[0] = ((0x104 + itercount * 8) % 0x100);
			if (lane0_active == 1)
				ill12_val[0] =
				    ((0x104 + itercount * 8) >=
				     0x200) ? 0x02 : 0x01;
			if (lane1_active == 1)
				ill1_val[1] = ((0x104 + itercount * 8) % 0x100);
			if (lane1_active == 1)
				ill12_val[1] =
				    ((0x104 + itercount * 8) >=
				     0x200) ? 0x02 : 0x01;
			if (lane2_active == 1)
				ill1_val[2] = ((0x104 + itercount * 8) % 0x100);
			if (lane2_active == 1)
				ill12_val[2] =
				    ((0x104 + itercount * 8) >=
				     0x200) ? 0x02 : 0x01;
			if (lane3_active == 1)
				ill1_val[3] = ((0x104 + itercount * 8) % 0x100);
			if (lane3_active == 1)
				ill12_val[3] =
				    ((0x104 + itercount * 8) >=
				     0x200) ? 0x02 : 0x01;

			if (lane0_active == 1)
				Xil_Out32(0xFD401928, ill1_val[0]);
			if (lane0_active == 1)
				psu_mask_write(0xFD401990, 0x0000000FU,
					       ill12_val[0]);
			if (lane1_active == 1)
				Xil_Out32(0xFD405928, ill1_val[1]);
			if (lane1_active == 1)
				psu_mask_write(0xFD405990, 0x0000000FU,
					       ill12_val[1]);
			if (lane2_active == 1)
				Xil_Out32(0xFD409928, ill1_val[2]);
			if (lane2_active == 1)
				psu_mask_write(0xFD409990, 0x0000000FU,
					       ill12_val[2]);
			if (lane3_active == 1)
				Xil_Out32(0xFD40D928, ill1_val[3]);
			if (lane3_active == 1)
				psu_mask_write(0xFD40D990, 0x0000000FU,
					       ill12_val[3]);
		}

		if (lane0_active == 1)
			psu_mask_write(0xFD401018, 0x00000030U, 0x00000010U);
		if (lane1_active == 1)
			psu_mask_write(0xFD405018, 0x00000030U, 0x00000010U);
		if (lane2_active == 1)
			psu_mask_write(0xFD409018, 0x00000030U, 0x00000010U);
		if (lane3_active == 1)
			psu_mask_write(0xFD40D018, 0x00000030U, 0x00000010U);
		if (lane0_active == 1)
			currbistresult[0] = 0;
		if (lane1_active == 1)
			currbistresult[1] = 0;
		if (lane2_active == 1)
			currbistresult[2] = 0;
		if (lane3_active == 1)
			currbistresult[3] = 0;
		serdes_rst_seq(pllsel, lane3_protocol, lane3_rate,
			       lane2_protocol, lane2_rate, lane1_protocol,
			       lane1_rate, lane0_protocol, lane0_rate);
		if (lane3_active == 1)
			serdes_bist_run(3);
		if (lane2_active == 1)
			serdes_bist_run(2);
		if (lane1_active == 1)
			serdes_bist_run(1);
		if (lane0_active == 1)
			serdes_bist_run(0);
		tempbistresult = 0;
		if (lane3_active == 1)
			tempbistresult = tempbistresult | serdes_bist_result(3);
		tempbistresult = tempbistresult << 1;
		if (lane2_active == 1)
			tempbistresult = tempbistresult | serdes_bist_result(2);
		tempbistresult = tempbistresult << 1;
		if (lane1_active == 1)
			tempbistresult = tempbistresult | serdes_bist_result(1);
		tempbistresult = tempbistresult << 1;
		if (lane0_active == 1)
			tempbistresult = tempbistresult | serdes_bist_result(0);
		Xil_Out32(0xFD410098, 0x0);
		Xil_Out32(0xFD410098, 0x2);

		if (itercount < 32) {
			iterresult[0] =
			    ((iterresult[0] << 1) |
			     ((tempbistresult & 0x1) == 0x1));
			iterresult[1] =
			    ((iterresult[1] << 1) |
			     ((tempbistresult & 0x2) == 0x2));
			iterresult[2] =
			    ((iterresult[2] << 1) |
			     ((tempbistresult & 0x4) == 0x4));
			iterresult[3] =
			    ((iterresult[3] << 1) |
			     ((tempbistresult & 0x8) == 0x8));
		} else {
			iterresult[4] =
			    ((iterresult[4] << 1) |
			     ((tempbistresult & 0x1) == 0x1));
			iterresult[5] =
			    ((iterresult[5] << 1) |
			     ((tempbistresult & 0x2) == 0x2));
			iterresult[6] =
			    ((iterresult[6] << 1) |
			     ((tempbistresult & 0x4) == 0x4));
			iterresult[7] =
			    ((iterresult[7] << 1) |
			     ((tempbistresult & 0x8) == 0x8));
		}
		currbistresult[0] =
		    currbistresult[0] | ((tempbistresult & 0x1) == 1);
		currbistresult[1] =
		    currbistresult[1] | ((tempbistresult & 0x2) == 0x2);
		currbistresult[2] =
		    currbistresult[2] | ((tempbistresult & 0x4) == 0x4);
		currbistresult[3] =
		    currbistresult[3] | ((tempbistresult & 0x8) == 0x8);

		for (loop = 0; loop <= 3; loop++) {
			if (currbistresult[loop] == 1 &&
			    prevbistresult[loop] == 1)
				bistpasscount[loop] = bistpasscount[loop] + 1;
			if (bistpasscount[loop] < 4 &&
			    currbistresult[loop] == 0 && itercount > 2) {
				if (meancountalt_bistpasscount[loop] <
				    bistpasscount[loop]) {
					meancountalt_bistpasscount[loop] =
					    bistpasscount[loop];
					meancountalt[loop] =
					    ((itercount - 1) -
					     ((bistpasscount[loop] + 1) / 2));
				}
				bistpasscount[loop] = 0;
			}
			if (meancount[loop] == 0 && bistpasscount[loop] >= 4 &&
			    (currbistresult[loop] == 0 || itercount == 63) &&
			    prevbistresult[loop] == 1)
				meancount[loop] =
				    itercount - 1 -
				    ((bistpasscount[loop] + 1) / 2);
			prevbistresult[loop] = currbistresult[loop];
		}
	} while (++itercount < 64);

	for (loop = 0; loop <= 3; loop++) {
		if (lane0_active == 0 && loop == 0)
			continue;
		if (lane1_active == 0 && loop == 1)
			continue;
		if (lane2_active == 0 && loop == 2)
			continue;
		if (lane3_active == 0 && loop == 3)
			continue;

		if (meancount[loop] == 0)
			meancount[loop] = meancountalt[loop];

		if (gen2_calib != 1) {
			ill1_val[loop] = ((0x04 + meancount[loop] * 8) % 0x100);
			ill12_val[loop] =
			    ((0x04 + meancount[loop] * 8) >=
			     0x100) ? 0x10 : 0x00;
		}
		if (gen2_calib == 1) {
			ill1_val[loop] =
			    ((0x104 + meancount[loop] * 8) % 0x100);
			ill12_val[loop] =
			    ((0x104 + meancount[loop] * 8) >=
			     0x200) ? 0x02 : 0x01;
		}
	}
	if (gen2_calib != 1) {
		if (lane0_active == 1)
			Xil_Out32(0xFD401924, ill1_val[0]);
		if (lane0_active == 1)
			psu_mask_write(0xFD401990, 0x000000F0U, ill12_val[0]);
		if (lane1_active == 1)
			Xil_Out32(0xFD405924, ill1_val[1]);
		if (lane1_active == 1)
			psu_mask_write(0xFD405990, 0x000000F0U, ill12_val[1]);
		if (lane2_active == 1)
			Xil_Out32(0xFD409924, ill1_val[2]);
		if (lane2_active == 1)
			psu_mask_write(0xFD409990, 0x000000F0U, ill12_val[2]);
		if (lane3_active == 1)
			Xil_Out32(0xFD40D924, ill1_val[3]);
		if (lane3_active == 1)
			psu_mask_write(0xFD40D990, 0x000000F0U, ill12_val[3]);
	}
	if (gen2_calib == 1) {
		if (lane0_active == 1)
			Xil_Out32(0xFD401928, ill1_val[0]);
		if (lane0_active == 1)
			psu_mask_write(0xFD401990, 0x0000000FU, ill12_val[0]);
		if (lane1_active == 1)
			Xil_Out32(0xFD405928, ill1_val[1]);
		if (lane1_active == 1)
			psu_mask_write(0xFD405990, 0x0000000FU, ill12_val[1]);
		if (lane2_active == 1)
			Xil_Out32(0xFD409928, ill1_val[2]);
		if (lane2_active == 1)
			psu_mask_write(0xFD409990, 0x0000000FU, ill12_val[2]);
		if (lane3_active == 1)
			Xil_Out32(0xFD40D928, ill1_val[3]);
		if (lane3_active == 1)
			psu_mask_write(0xFD40D990, 0x0000000FU, ill12_val[3]);
	}

	if (lane0_active == 1)
		psu_mask_write(0xFD401018, 0x00000030U, 0x00000000U);
	if (lane1_active == 1)
		psu_mask_write(0xFD405018, 0x00000030U, 0x00000000U);
	if (lane2_active == 1)
		psu_mask_write(0xFD409018, 0x00000030U, 0x00000000U);
	if (lane3_active == 1)
		psu_mask_write(0xFD40D018, 0x00000030U, 0x00000000U);

	Xil_Out32(0xFD410098, 0);
	if (lane0_active == 1) {
		Xil_Out32(0xFD403004, 0);
		Xil_Out32(0xFD403008, 0);
		Xil_Out32(0xFD40300C, 0);
		Xil_Out32(0xFD403010, 0);
		Xil_Out32(0xFD403014, 0);
		Xil_Out32(0xFD403018, 0);
		Xil_Out32(0xFD40301C, 0);
		Xil_Out32(0xFD403020, 0);
		Xil_Out32(0xFD403024, 0);
		Xil_Out32(0xFD403028, 0);
		Xil_Out32(0xFD40302C, 0);
		Xil_Out32(0xFD403030, 0);
		Xil_Out32(0xFD403034, 0);
		Xil_Out32(0xFD403038, 0);
		Xil_Out32(0xFD40303C, 0);
		Xil_Out32(0xFD403040, 0);
		Xil_Out32(0xFD403044, 0);
		Xil_Out32(0xFD403048, 0);
		Xil_Out32(0xFD40304C, 0);
		Xil_Out32(0xFD403050, 0);
		Xil_Out32(0xFD403054, 0);
		Xil_Out32(0xFD403058, 0);
		Xil_Out32(0xFD403068, 1);
		Xil_Out32(0xFD40306C, 0);
		Xil_Out32(0xFD4010AC, 0);
		psu_mask_write(0xFD410044, 0x00000003U, 0x00000001U);
		psu_mask_write(0xFD410040, 0x00000003U, 0x00000001U);
		psu_mask_write(0xFD410038, 0x00000007U, 0x00000000U);
	}
	if (lane1_active == 1) {
		Xil_Out32(0xFD407004, 0);
		Xil_Out32(0xFD407008, 0);
		Xil_Out32(0xFD40700C, 0);
		Xil_Out32(0xFD407010, 0);
		Xil_Out32(0xFD407014, 0);
		Xil_Out32(0xFD407018, 0);
		Xil_Out32(0xFD40701C, 0);
		Xil_Out32(0xFD407020, 0);
		Xil_Out32(0xFD407024, 0);
		Xil_Out32(0xFD407028, 0);
		Xil_Out32(0xFD40702C, 0);
		Xil_Out32(0xFD407030, 0);
		Xil_Out32(0xFD407034, 0);
		Xil_Out32(0xFD407038, 0);
		Xil_Out32(0xFD40703C, 0);
		Xil_Out32(0xFD407040, 0);
		Xil_Out32(0xFD407044, 0);
		Xil_Out32(0xFD407048, 0);
		Xil_Out32(0xFD40704C, 0);
		Xil_Out32(0xFD407050, 0);
		Xil_Out32(0xFD407054, 0);
		Xil_Out32(0xFD407058, 0);
		Xil_Out32(0xFD407068, 1);
		Xil_Out32(0xFD40706C, 0);
		Xil_Out32(0xFD4050AC, 0);
		psu_mask_write(0xFD410044, 0x0000000CU, 0x00000004U);
		psu_mask_write(0xFD410040, 0x0000000CU, 0x00000004U);
		psu_mask_write(0xFD410038, 0x00000070U, 0x00000000U);
	}
	if (lane2_active == 1) {
		Xil_Out32(0xFD40B004, 0);
		Xil_Out32(0xFD40B008, 0);
		Xil_Out32(0xFD40B00C, 0);
		Xil_Out32(0xFD40B010, 0);
		Xil_Out32(0xFD40B014, 0);
		Xil_Out32(0xFD40B018, 0);
		Xil_Out32(0xFD40B01C, 0);
		Xil_Out32(0xFD40B020, 0);
		Xil_Out32(0xFD40B024, 0);
		Xil_Out32(0xFD40B028, 0);
		Xil_Out32(0xFD40B02C, 0);
		Xil_Out32(0xFD40B030, 0);
		Xil_Out32(0xFD40B034, 0);
		Xil_Out32(0xFD40B038, 0);
		Xil_Out32(0xFD40B03C, 0);
		Xil_Out32(0xFD40B040, 0);
		Xil_Out32(0xFD40B044, 0);
		Xil_Out32(0xFD40B048, 0);
		Xil_Out32(0xFD40B04C, 0);
		Xil_Out32(0xFD40B050, 0);
		Xil_Out32(0xFD40B054, 0);
		Xil_Out32(0xFD40B058, 0);
		Xil_Out32(0xFD40B068, 1);
		Xil_Out32(0xFD40B06C, 0);
		Xil_Out32(0xFD4090AC, 0);
		psu_mask_write(0xFD410044, 0x00000030U, 0x00000010U);
		psu_mask_write(0xFD410040, 0x00000030U, 0x00000010U);
		psu_mask_write(0xFD41003C, 0x00000007U, 0x00000000U);
	}
	if (lane3_active == 1) {
		Xil_Out32(0xFD40F004, 0);
		Xil_Out32(0xFD40F008, 0);
		Xil_Out32(0xFD40F00C, 0);
		Xil_Out32(0xFD40F010, 0);
		Xil_Out32(0xFD40F014, 0);
		Xil_Out32(0xFD40F018, 0);
		Xil_Out32(0xFD40F01C, 0);
		Xil_Out32(0xFD40F020, 0);
		Xil_Out32(0xFD40F024, 0);
		Xil_Out32(0xFD40F028, 0);
		Xil_Out32(0xFD40F02C, 0);
		Xil_Out32(0xFD40F030, 0);
		Xil_Out32(0xFD40F034, 0);
		Xil_Out32(0xFD40F038, 0);
		Xil_Out32(0xFD40F03C, 0);
		Xil_Out32(0xFD40F040, 0);
		Xil_Out32(0xFD40F044, 0);
		Xil_Out32(0xFD40F048, 0);
		Xil_Out32(0xFD40F04C, 0);
		Xil_Out32(0xFD40F050, 0);
		Xil_Out32(0xFD40F054, 0);
		Xil_Out32(0xFD40F058, 0);
		Xil_Out32(0xFD40F068, 1);
		Xil_Out32(0xFD40F06C, 0);
		Xil_Out32(0xFD40D0AC, 0);
		psu_mask_write(0xFD410044, 0x000000C0U, 0x00000040U);
		psu_mask_write(0xFD410040, 0x000000C0U, 0x00000040U);
		psu_mask_write(0xFD41003C, 0x00000070U, 0x00000000U);
	}
	return 1;
}

static int serdes_illcalib(u32 lane3_protocol, u32 lane3_rate,
			   u32 lane2_protocol, u32 lane2_rate,
			   u32 lane1_protocol, u32 lane1_rate,
			   u32 lane0_protocol, u32 lane0_rate)
{
	unsigned int rdata = 0;
	unsigned int sata_gen2 = 1;
	unsigned int temp_ill12 = 0;
	unsigned int temp_PLL_REF_SEL_OFFSET;
	unsigned int temp_TM_IQ_ILL1;
	unsigned int temp_TM_E_ILL1;
	unsigned int temp_tx_dig_tm_61;
	unsigned int temp_tm_dig_6;
	unsigned int temp_pll_fbdiv_frac_3_msb_offset;

	if (lane0_protocol == 2 || lane0_protocol == 1) {
		Xil_Out32(0xFD401910, 0xF3);
		Xil_Out32(0xFD40193C, 0xF3);
		Xil_Out32(0xFD401914, 0xF3);
		Xil_Out32(0xFD401940, 0xF3);
	}
	if (lane1_protocol == 2 || lane1_protocol == 1) {
		Xil_Out32(0xFD405910, 0xF3);
		Xil_Out32(0xFD40593C, 0xF3);
		Xil_Out32(0xFD405914, 0xF3);
		Xil_Out32(0xFD405940, 0xF3);
	}
	if (lane2_protocol == 2 || lane2_protocol == 1) {
		Xil_Out32(0xFD409910, 0xF3);
		Xil_Out32(0xFD40993C, 0xF3);
		Xil_Out32(0xFD409914, 0xF3);
		Xil_Out32(0xFD409940, 0xF3);
	}
	if (lane3_protocol == 2 || lane3_protocol == 1) {
		Xil_Out32(0xFD40D910, 0xF3);
		Xil_Out32(0xFD40D93C, 0xF3);
		Xil_Out32(0xFD40D914, 0xF3);
		Xil_Out32(0xFD40D940, 0xF3);
	}

	if (sata_gen2 == 1) {
		if (lane0_protocol == 2) {
			temp_pll_fbdiv_frac_3_msb_offset = Xil_In32(0xFD402360);
			Xil_Out32(0xFD402360, 0x0);
			temp_PLL_REF_SEL_OFFSET = Xil_In32(0xFD410000);
			psu_mask_write(0xFD410000, 0x0000001FU, 0x0000000DU);
			temp_TM_IQ_ILL1 = Xil_In32(0xFD4018F8);
			temp_TM_E_ILL1 = Xil_In32(0xFD401924);
			Xil_Out32(0xFD4018F8, 0x78);
			temp_tx_dig_tm_61 = Xil_In32(0xFD4000F4);
			temp_tm_dig_6 = Xil_In32(0xFD40106C);
			psu_mask_write(0xFD4000F4, 0x0000000BU, 0x00000000U);
			psu_mask_write(0xFD40106C, 0x0000000FU, 0x00000000U);
			temp_ill12 = Xil_In32(0xFD401990) & 0xF0;

			serdes_illcalib_pcie_gen1(0, 0, 0, 0, 0, 0, 0, 1, 0, 0);

			Xil_Out32(0xFD402360, temp_pll_fbdiv_frac_3_msb_offset);
			Xil_Out32(0xFD410000, temp_PLL_REF_SEL_OFFSET);
			Xil_Out32(0xFD4018F8, temp_TM_IQ_ILL1);
			Xil_Out32(0xFD4000F4, temp_tx_dig_tm_61);
			Xil_Out32(0xFD40106C, temp_tm_dig_6);
			Xil_Out32(0xFD401928, Xil_In32(0xFD401924));
			temp_ill12 =
			    temp_ill12 | (Xil_In32(0xFD401990) >> 4 & 0xF);
			Xil_Out32(0xFD401990, temp_ill12);
			Xil_Out32(0xFD401924, temp_TM_E_ILL1);
		}
		if (lane1_protocol == 2) {
			temp_pll_fbdiv_frac_3_msb_offset = Xil_In32(0xFD406360);
			Xil_Out32(0xFD406360, 0x0);
			temp_PLL_REF_SEL_OFFSET = Xil_In32(0xFD410004);
			psu_mask_write(0xFD410004, 0x0000001FU, 0x0000000DU);
			temp_TM_IQ_ILL1 = Xil_In32(0xFD4058F8);
			temp_TM_E_ILL1 = Xil_In32(0xFD405924);
			Xil_Out32(0xFD4058F8, 0x78);
			temp_tx_dig_tm_61 = Xil_In32(0xFD4040F4);
			temp_tm_dig_6 = Xil_In32(0xFD40506C);
			psu_mask_write(0xFD4040F4, 0x0000000BU, 0x00000000U);
			psu_mask_write(0xFD40506C, 0x0000000FU, 0x00000000U);
			temp_ill12 = Xil_In32(0xFD405990) & 0xF0;

			serdes_illcalib_pcie_gen1(1, 0, 0, 0, 0, 1, 0, 0, 0, 0);

			Xil_Out32(0xFD406360, temp_pll_fbdiv_frac_3_msb_offset);
			Xil_Out32(0xFD410004, temp_PLL_REF_SEL_OFFSET);
			Xil_Out32(0xFD4058F8, temp_TM_IQ_ILL1);
			Xil_Out32(0xFD4040F4, temp_tx_dig_tm_61);
			Xil_Out32(0xFD40506C, temp_tm_dig_6);
			Xil_Out32(0xFD405928, Xil_In32(0xFD405924));
			temp_ill12 =
			    temp_ill12 | (Xil_In32(0xFD405990) >> 4 & 0xF);
			Xil_Out32(0xFD405990, temp_ill12);
			Xil_Out32(0xFD405924, temp_TM_E_ILL1);
		}
		if (lane2_protocol == 2) {
			temp_pll_fbdiv_frac_3_msb_offset = Xil_In32(0xFD40A360);
			Xil_Out32(0xFD40A360, 0x0);
			temp_PLL_REF_SEL_OFFSET = Xil_In32(0xFD410008);
			psu_mask_write(0xFD410008, 0x0000001FU, 0x0000000DU);
			temp_TM_IQ_ILL1 = Xil_In32(0xFD4098F8);
			temp_TM_E_ILL1 = Xil_In32(0xFD409924);
			Xil_Out32(0xFD4098F8, 0x78);
			temp_tx_dig_tm_61 = Xil_In32(0xFD4080F4);
			temp_tm_dig_6 = Xil_In32(0xFD40906C);
			psu_mask_write(0xFD4080F4, 0x0000000BU, 0x00000000U);
			psu_mask_write(0xFD40906C, 0x0000000FU, 0x00000000U);
			temp_ill12 = Xil_In32(0xFD409990) & 0xF0;

			serdes_illcalib_pcie_gen1(2, 0, 0, 1, 0, 0, 0, 0, 0, 0);

			Xil_Out32(0xFD40A360, temp_pll_fbdiv_frac_3_msb_offset);
			Xil_Out32(0xFD410008, temp_PLL_REF_SEL_OFFSET);
			Xil_Out32(0xFD4098F8, temp_TM_IQ_ILL1);
			Xil_Out32(0xFD4080F4, temp_tx_dig_tm_61);
			Xil_Out32(0xFD40906C, temp_tm_dig_6);
			Xil_Out32(0xFD409928, Xil_In32(0xFD409924));
			temp_ill12 =
			    temp_ill12 | (Xil_In32(0xFD409990) >> 4 & 0xF);
			Xil_Out32(0xFD409990, temp_ill12);
			Xil_Out32(0xFD409924, temp_TM_E_ILL1);
		}
		if (lane3_protocol == 2) {
			temp_pll_fbdiv_frac_3_msb_offset = Xil_In32(0xFD40E360);
			Xil_Out32(0xFD40E360, 0x0);
			temp_PLL_REF_SEL_OFFSET = Xil_In32(0xFD41000C);
			psu_mask_write(0xFD41000C, 0x0000001FU, 0x0000000DU);
			temp_TM_IQ_ILL1 = Xil_In32(0xFD40D8F8);
			temp_TM_E_ILL1 = Xil_In32(0xFD40D924);
			Xil_Out32(0xFD40D8F8, 0x78);
			temp_tx_dig_tm_61 = Xil_In32(0xFD40C0F4);
			temp_tm_dig_6 = Xil_In32(0xFD40D06C);
			psu_mask_write(0xFD40C0F4, 0x0000000BU, 0x00000000U);
			psu_mask_write(0xFD40D06C, 0x0000000FU, 0x00000000U);
			temp_ill12 = Xil_In32(0xFD40D990) & 0xF0;

			serdes_illcalib_pcie_gen1(3, 1, 0, 0, 0, 0, 0, 0, 0, 0);

			Xil_Out32(0xFD40E360, temp_pll_fbdiv_frac_3_msb_offset);
			Xil_Out32(0xFD41000C, temp_PLL_REF_SEL_OFFSET);
			Xil_Out32(0xFD40D8F8, temp_TM_IQ_ILL1);
			Xil_Out32(0xFD40C0F4, temp_tx_dig_tm_61);
			Xil_Out32(0xFD40D06C, temp_tm_dig_6);
			Xil_Out32(0xFD40D928, Xil_In32(0xFD40D924));
			temp_ill12 =
			    temp_ill12 | (Xil_In32(0xFD40D990) >> 4 & 0xF);
			Xil_Out32(0xFD40D990, temp_ill12);
			Xil_Out32(0xFD40D924, temp_TM_E_ILL1);
		}
		rdata = Xil_In32(0xFD410098);
		rdata = (rdata & 0xDF);
		Xil_Out32(0xFD410098, rdata);
	}

	if (lane0_protocol == 2 && lane0_rate == 3) {
		psu_mask_write(0xFD40198C, 0x000000F0U, 0x00000020U);
		psu_mask_write(0xFD40192C, 0x000000FFU, 0x00000094U);
	}
	if (lane1_protocol == 2 && lane1_rate == 3) {
		psu_mask_write(0xFD40598C, 0x000000F0U, 0x00000020U);
		psu_mask_write(0xFD40592C, 0x000000FFU, 0x00000094U);
	}
	if (lane2_protocol == 2 && lane2_rate == 3) {
		psu_mask_write(0xFD40998C, 0x000000F0U, 0x00000020U);
		psu_mask_write(0xFD40992C, 0x000000FFU, 0x00000094U);
	}
	if (lane3_protocol == 2 && lane3_rate == 3) {
		psu_mask_write(0xFD40D98C, 0x000000F0U, 0x00000020U);
		psu_mask_write(0xFD40D92C, 0x000000FFU, 0x00000094U);
	}

	if (lane0_protocol == 1) {
		if (lane0_rate == 0) {
			serdes_illcalib_pcie_gen1(0, lane3_protocol, lane3_rate,
						  lane2_protocol, lane2_rate,
						  lane1_protocol, lane1_rate,
						  lane0_protocol, 0, 0);
		} else {
			serdes_illcalib_pcie_gen1(0, lane3_protocol, lane3_rate,
						  lane2_protocol, lane2_rate,
						  lane1_protocol, lane1_rate,
						  lane0_protocol, 0, 0);
			serdes_illcalib_pcie_gen1(0, lane3_protocol, lane3_rate,
						  lane2_protocol, lane2_rate,
						  lane1_protocol, lane1_rate,
						  lane0_protocol, lane0_rate,
						  1);
		}
	}

	if (lane0_protocol == 3)
		Xil_Out32(0xFD401914, 0xF3);
	if (lane0_protocol == 3)
		Xil_Out32(0xFD401940, 0xF3);
	if (lane0_protocol == 3)
		Xil_Out32(0xFD401990, 0x20);
	if (lane0_protocol == 3)
		Xil_Out32(0xFD401924, 0x37);

	if (lane1_protocol == 3)
		Xil_Out32(0xFD405914, 0xF3);
	if (lane1_protocol == 3)
		Xil_Out32(0xFD405940, 0xF3);
	if (lane1_protocol == 3)
		Xil_Out32(0xFD405990, 0x20);
	if (lane1_protocol == 3)
		Xil_Out32(0xFD405924, 0x37);

	if (lane2_protocol == 3)
		Xil_Out32(0xFD409914, 0xF3);
	if (lane2_protocol == 3)
		Xil_Out32(0xFD409940, 0xF3);
	if (lane2_protocol == 3)
		Xil_Out32(0xFD409990, 0x20);
	if (lane2_protocol == 3)
		Xil_Out32(0xFD409924, 0x37);

	if (lane3_protocol == 3)
		Xil_Out32(0xFD40D914, 0xF3);
	if (lane3_protocol == 3)
		Xil_Out32(0xFD40D940, 0xF3);
	if (lane3_protocol == 3)
		Xil_Out32(0xFD40D990, 0x20);
	if (lane3_protocol == 3)
		Xil_Out32(0xFD40D924, 0x37);

	return 1;
}

static void dpll_prog(int div2, int ddr_pll_fbdiv, int d_lock_dly,
		      int d_lock_cnt, int d_lfhf, int d_cp, int d_res)
{
	unsigned int pll_ctrl_regval;
	unsigned int pll_status_regval;

	pll_ctrl_regval = Xil_In32(((0xFD1A0000U) + 0x0000002C));
	pll_ctrl_regval = pll_ctrl_regval & (~0x00010000U);
	pll_ctrl_regval = pll_ctrl_regval | (div2 << 16);
	Xil_Out32(((0xFD1A0000U) + 0x0000002C), pll_ctrl_regval);

	pll_ctrl_regval = Xil_In32(((0xFD1A0000U) + 0x00000030));
	pll_ctrl_regval = pll_ctrl_regval & (~0xFE000000U);
	pll_ctrl_regval = pll_ctrl_regval | (d_lock_dly << 25);
	Xil_Out32(((0xFD1A0000U) + 0x00000030), pll_ctrl_regval);

	pll_ctrl_regval = Xil_In32(((0xFD1A0000U) + 0x00000030));
	pll_ctrl_regval = pll_ctrl_regval & (~0x007FE000U);
	pll_ctrl_regval = pll_ctrl_regval | (d_lock_cnt << 13);
	Xil_Out32(((0xFD1A0000U) + 0x00000030), pll_ctrl_regval);

	pll_ctrl_regval = Xil_In32(((0xFD1A0000U) + 0x00000030));
	pll_ctrl_regval = pll_ctrl_regval & (~0x00000C00U);
	pll_ctrl_regval = pll_ctrl_regval | (d_lfhf << 10);
	Xil_Out32(((0xFD1A0000U) + 0x00000030), pll_ctrl_regval);

	pll_ctrl_regval = Xil_In32(((0xFD1A0000U) + 0x00000030));
	pll_ctrl_regval = pll_ctrl_regval & (~0x000001E0U);
	pll_ctrl_regval = pll_ctrl_regval | (d_cp << 5);
	Xil_Out32(((0xFD1A0000U) + 0x00000030), pll_ctrl_regval);

	pll_ctrl_regval = Xil_In32(((0xFD1A0000U) + 0x00000030));
	pll_ctrl_regval = pll_ctrl_regval & (~0x0000000FU);
	pll_ctrl_regval = pll_ctrl_regval | (d_res << 0);
	Xil_Out32(((0xFD1A0000U) + 0x00000030), pll_ctrl_regval);

	pll_ctrl_regval = Xil_In32(((0xFD1A0000U) + 0x0000002C));
	pll_ctrl_regval = pll_ctrl_regval & (~0x00007F00U);
	pll_ctrl_regval = pll_ctrl_regval | (ddr_pll_fbdiv << 8);
	Xil_Out32(((0xFD1A0000U) + 0x0000002C), pll_ctrl_regval);

	pll_ctrl_regval = Xil_In32(((0xFD1A0000U) + 0x0000002C));
	pll_ctrl_regval = pll_ctrl_regval & (~0x00000008U);
	pll_ctrl_regval = pll_ctrl_regval | (1 << 3);
	Xil_Out32(((0xFD1A0000U) + 0x0000002C), pll_ctrl_regval);

	pll_ctrl_regval = Xil_In32(((0xFD1A0000U) + 0x0000002C));
	pll_ctrl_regval = pll_ctrl_regval & (~0x00000001U);
	pll_ctrl_regval = pll_ctrl_regval | (1 << 0);
	Xil_Out32(((0xFD1A0000U) + 0x0000002C), pll_ctrl_regval);

	pll_ctrl_regval = Xil_In32(((0xFD1A0000U) + 0x0000002C));
	pll_ctrl_regval = pll_ctrl_regval & (~0x00000001U);
	pll_ctrl_regval = pll_ctrl_regval | (0 << 0);
	Xil_Out32(((0xFD1A0000U) + 0x0000002C), pll_ctrl_regval);

	pll_status_regval = 0x00000000;
	while ((pll_status_regval & 0x00000002U) != 0x00000002U)
		pll_status_regval = Xil_In32(((0xFD1A0000U) + 0x00000044));

	pll_ctrl_regval = Xil_In32(((0xFD1A0000U) + 0x0000002C));
	pll_ctrl_regval = pll_ctrl_regval & (~0x00000008U);
	pll_ctrl_regval = pll_ctrl_regval | (0 << 3);
	Xil_Out32(((0xFD1A0000U) + 0x0000002C), pll_ctrl_regval);
}

static unsigned long psu_pll_init_data(void)
{
	psu_mask_write(0xFF5E0034, 0xFE7FEDEFU, 0x7E4B0C62U);
	psu_mask_write(0xFF5E0030, 0x00717F00U, 0x00014600U);
	psu_mask_write(0xFF5E0030, 0x00000008U, 0x00000008U);
	psu_mask_write(0xFF5E0030, 0x00000001U, 0x00000001U);
	psu_mask_write(0xFF5E0030, 0x00000001U, 0x00000000U);
	mask_poll(0xFF5E0040, 0x00000002U);
	psu_mask_write(0xFF5E0030, 0x00000008U, 0x00000000U);
	psu_mask_write(0xFF5E0048, 0x00003F00U, 0x00000300U);
	psu_mask_write(0xFF5E0038, 0x8000FFFFU, 0x00000000U);
	psu_mask_write(0xFF5E0108, 0x013F3F07U, 0x01012300U);
	psu_mask_write(0xFF5E0024, 0xFE7FEDEFU, 0x7E672C6CU);
	psu_mask_write(0xFF5E0020, 0x00717F00U, 0x00002D00U);
	psu_mask_write(0xFF5E0020, 0x00000008U, 0x00000008U);
	psu_mask_write(0xFF5E0020, 0x00000001U, 0x00000001U);
	psu_mask_write(0xFF5E0020, 0x00000001U, 0x00000000U);
	mask_poll(0xFF5E0040, 0x00000001U);
	psu_mask_write(0xFF5E0020, 0x00000008U, 0x00000000U);
	psu_mask_write(0xFF5E0044, 0x00003F00U, 0x00000300U);
	psu_mask_write(0xFF5E0028, 0x8000FFFFU, 0x00000000U);
	psu_mask_write(0xFD1A0024, 0xFE7FEDEFU, 0x7E4B0C62U);
	psu_mask_write(0xFD1A0020, 0x00717F00U, 0x00014800U);
	psu_mask_write(0xFD1A0020, 0x00000008U, 0x00000008U);
	psu_mask_write(0xFD1A0020, 0x00000001U, 0x00000001U);
	psu_mask_write(0xFD1A0020, 0x00000001U, 0x00000000U);
	mask_poll(0xFD1A0044, 0x00000001U);
	psu_mask_write(0xFD1A0020, 0x00000008U, 0x00000000U);
	psu_mask_write(0xFD1A0048, 0x00003F00U, 0x00000300U);
	psu_mask_write(0xFD1A0028, 0x8000FFFFU, 0x00000000U);
	psu_mask_write(0xFD1A0030, 0xFE7FEDEFU, 0x7E4B0C62U);
	psu_mask_write(0xFD1A002C, 0x00717F00U, 0x00014000U);
	psu_mask_write(0xFD1A002C, 0x00000008U, 0x00000008U);
	psu_mask_write(0xFD1A002C, 0x00000001U, 0x00000001U);
	psu_mask_write(0xFD1A002C, 0x00000001U, 0x00000000U);
	mask_poll(0xFD1A0044, 0x00000002U);
	psu_mask_write(0xFD1A002C, 0x00000008U, 0x00000000U);
	psu_mask_write(0xFD1A004C, 0x00003F00U, 0x00000300U);
	psu_mask_write(0xFD1A0034, 0x8000FFFFU, 0x00000000U);
	psu_mask_write(0xFD1A003C, 0xFE7FEDEFU, 0x7E4B0C62U);
	psu_mask_write(0xFD1A0038, 0x00717F00U, 0x00014700U);
	psu_mask_write(0xFD1A0038, 0x00000008U, 0x00000008U);
	psu_mask_write(0xFD1A0038, 0x00000001U, 0x00000001U);
	psu_mask_write(0xFD1A0038, 0x00000001U, 0x00000000U);
	mask_poll(0xFD1A0044, 0x00000004U);
	psu_mask_write(0xFD1A0038, 0x00000008U, 0x00000000U);
	psu_mask_write(0xFD1A0050, 0x00003F00U, 0x00000300U);
	psu_mask_write(0xFD1A0040, 0x8000FFFFU, 0x00000000U);

	return 1;
}

static unsigned long psu_clock_init_data(void)
{
	psu_mask_write(0xFF5E0050, 0x063F3F07U, 0x06010C00U);
	psu_mask_write(0xFF180360, 0x00000003U, 0x00000001U);
	psu_mask_write(0xFF180308, 0x00000006U, 0x00000006U);
	psu_mask_write(0xFF5E0100, 0x013F3F07U, 0x01010600U);
	psu_mask_write(0xFF5E0070, 0x013F3F07U, 0x01010800U);
	psu_mask_write(0xFF18030C, 0x00020000U, 0x00000000U);
	psu_mask_write(0xFF5E0074, 0x013F3F07U, 0x01010F00U);
	psu_mask_write(0xFF5E0120, 0x013F3F07U, 0x01010F00U);
	psu_mask_write(0xFF5E0124, 0x013F3F07U, 0x01010F00U);
	psu_mask_write(0xFF5E0090, 0x01003F07U, 0x01000302U);
	psu_mask_write(0xFF5E009C, 0x01003F07U, 0x01000602U);
	psu_mask_write(0xFF5E00A4, 0x01003F07U, 0x01000800U);
	psu_mask_write(0xFF5E00A8, 0x01003F07U, 0x01000302U);
	psu_mask_write(0xFF5E00AC, 0x01003F07U, 0x01000F02U);
	psu_mask_write(0xFF5E00B0, 0x01003F07U, 0x01000602U);
	psu_mask_write(0xFF5E00B8, 0x01003F07U, 0x01000302U);
	psu_mask_write(0xFF5E00C0, 0x013F3F07U, 0x01010F00U);
	psu_mask_write(0xFF5E00C4, 0x013F3F07U, 0x01040F00U);
	psu_mask_write(0xFF5E00C8, 0x013F3F07U, 0x01010500U);
	psu_mask_write(0xFF5E00CC, 0x013F3F07U, 0x01010400U);
	psu_mask_write(0xFF5E0108, 0x013F3F07U, 0x01011D02U);
	psu_mask_write(0xFF5E0104, 0x00000007U, 0x00000000U);
	psu_mask_write(0xFF5E0128, 0x01003F07U, 0x01000F00U);
	psu_mask_write(0xFD1A0060, 0x03003F07U, 0x03000100U);
	psu_mask_write(0xFD1A0068, 0x01003F07U, 0x01000200U);
	psu_mask_write(0xFD1A0080, 0x00003F07U, 0x00000200U);
	psu_mask_write(0xFD1A0084, 0x07003F07U, 0x07000100U);
	psu_mask_write(0xFD1A00B8, 0x01003F07U, 0x01000200U);
	psu_mask_write(0xFD1A00BC, 0x01003F07U, 0x01000200U);
	psu_mask_write(0xFD1A00C0, 0x01003F07U, 0x01000203U);
	psu_mask_write(0xFD1A00C4, 0x01003F07U, 0x01000502U);
	psu_mask_write(0xFD1A00F8, 0x00003F07U, 0x00000200U);
	psu_mask_write(0xFF180380, 0x000000FFU, 0x00000000U);
	psu_mask_write(0xFD610100, 0x00000001U, 0x00000000U);
	psu_mask_write(0xFF180300, 0x00000001U, 0x00000000U);
	psu_mask_write(0xFF410050, 0x00000001U, 0x00000000U);

	return 1;
}

static unsigned long psu_ddr_init_data(void)
{
	psu_mask_write(0xFD1A0108, 0x00000008U, 0x00000008U);
	psu_mask_write(0xFD070000, 0xE30FBE3DU, 0xC1081020U);
	psu_mask_write(0xFD070010, 0x8000F03FU, 0x00000030U);
	psu_mask_write(0xFD070020, 0x000003F3U, 0x00000202U);
	psu_mask_write(0xFD070024, 0xFFFFFFFFU, 0x00516120U);
	psu_mask_write(0xFD070030, 0x0000007FU, 0x00000000U);
	psu_mask_write(0xFD070034, 0x00FFFF1FU, 0x00408410U);
	psu_mask_write(0xFD070050, 0x00F1F1F4U, 0x00210000U);
	psu_mask_write(0xFD070054, 0x0FFF0FFFU, 0x00000000U);
	psu_mask_write(0xFD070060, 0x00000073U, 0x00000001U);
	psu_mask_write(0xFD070064, 0x0FFF83FFU, 0x00418096U);
	psu_mask_write(0xFD070070, 0x00000017U, 0x00000010U);
	psu_mask_write(0xFD070074, 0x00000003U, 0x00000000U);
	psu_mask_write(0xFD0700C4, 0x3F000391U, 0x10000200U);
	psu_mask_write(0xFD0700C8, 0x01FF1F3FU, 0x0030051FU);
	psu_mask_write(0xFD0700D0, 0xC3FF0FFFU, 0x00030413U);
	psu_mask_write(0xFD0700D4, 0x01FF7F0FU, 0x006A0000U);
	psu_mask_write(0xFD0700D8, 0x0000FF0FU, 0x00002305U);
	psu_mask_write(0xFD0700DC, 0xFFFFFFFFU, 0x00440024U);
	psu_mask_write(0xFD0700E0, 0xFFFFFFFFU, 0x00310008U);
	psu_mask_write(0xFD0700E4, 0x00FF03FFU, 0x00210004U);
	psu_mask_write(0xFD0700E8, 0xFFFFFFFFU, 0x00000000U);
	psu_mask_write(0xFD0700EC, 0xFFFF0000U, 0x00000000U);
	psu_mask_write(0xFD0700F0, 0x0000003FU, 0x00000010U);
	psu_mask_write(0xFD0700F4, 0x00000FFFU, 0x0000077FU);
	psu_mask_write(0xFD070100, 0x7F3F7F3FU, 0x15161117U);
	psu_mask_write(0xFD070104, 0x001F1F7FU, 0x00040422U);
	psu_mask_write(0xFD070108, 0x3F3F3F3FU, 0x060C1A10U);
	psu_mask_write(0xFD07010C, 0x3FF3F3FFU, 0x00F08000U);
	psu_mask_write(0xFD070110, 0x1F0F0F1FU, 0x0A04060CU);
	psu_mask_write(0xFD070114, 0x0F0F3F1FU, 0x01040808U);
	psu_mask_write(0xFD070118, 0x0F0F000FU, 0x01010005U);
	psu_mask_write(0xFD07011C, 0x00000F0FU, 0x00000401U);
	psu_mask_write(0xFD070120, 0x7F7F7F7FU, 0x04040606U);
	psu_mask_write(0xFD070124, 0x40070F3FU, 0x0004040DU);
	psu_mask_write(0xFD07012C, 0x7F1F031FU, 0x440C011CU);
	psu_mask_write(0xFD070130, 0x00030F1FU, 0x00020608U);
	psu_mask_write(0xFD070180, 0xF7FF03FFU, 0x82160010U);
	psu_mask_write(0xFD070184, 0x3FFFFFFFU, 0x01B65B96U);
	psu_mask_write(0xFD070190, 0x1FBFBF3FU, 0x0495820AU);
	psu_mask_write(0xFD070194, 0xF31F0F0FU, 0x00030304U);
	psu_mask_write(0xFD070198, 0x0FF1F1F1U, 0x07000101U);
	psu_mask_write(0xFD07019C, 0x000000F1U, 0x00000021U);
	psu_mask_write(0xFD0701A0, 0xC3FF03FFU, 0x83FF0003U);
	psu_mask_write(0xFD0701A4, 0x00FF00FFU, 0x00C800FFU);
	psu_mask_write(0xFD0701B0, 0x00000007U, 0x00000004U);
	psu_mask_write(0xFD0701B4, 0x00003F3FU, 0x00001308U);
	psu_mask_write(0xFD0701C0, 0x00000007U, 0x00000001U);
	psu_mask_write(0xFD070200, 0x0000001FU, 0x0000001FU);
	psu_mask_write(0xFD070204, 0x001F1F1FU, 0x00070707U);
	psu_mask_write(0xFD070208, 0x0F0F0F0FU, 0x00000000U);
	psu_mask_write(0xFD07020C, 0x0F0F0F0FU, 0x0F000000U);
	psu_mask_write(0xFD070210, 0x00000F0FU, 0x00000F0FU);
	psu_mask_write(0xFD070214, 0x0F0F0F0FU, 0x060F0606U);
	psu_mask_write(0xFD070218, 0x8F0F0F0FU, 0x06060606U);
	psu_mask_write(0xFD07021C, 0x00000F0FU, 0x00000F0FU);
	psu_mask_write(0xFD070220, 0x00001F1FU, 0x00000000U);
	psu_mask_write(0xFD070224, 0x0F0F0F0FU, 0x06060606U);
	psu_mask_write(0xFD070228, 0x0F0F0F0FU, 0x06060606U);
	psu_mask_write(0xFD07022C, 0x0000000FU, 0x00000006U);
	psu_mask_write(0xFD070240, 0x0F1F0F7CU, 0x04000400U);
	psu_mask_write(0xFD070244, 0x00003333U, 0x00000000U);
	psu_mask_write(0xFD070250, 0x7FFF3F07U, 0x01002001U);
	psu_mask_write(0xFD070264, 0xFF00FFFFU, 0x08000040U);
	psu_mask_write(0xFD07026C, 0xFF00FFFFU, 0x08000040U);
	psu_mask_write(0xFD070280, 0xFFFFFFFFU, 0x00000000U);
	psu_mask_write(0xFD070284, 0xFFFFFFFFU, 0x00000000U);
	psu_mask_write(0xFD070288, 0xFFFFFFFFU, 0x00000000U);
	psu_mask_write(0xFD07028C, 0xFFFFFFFFU, 0x00000000U);
	psu_mask_write(0xFD070290, 0x0000FFFFU, 0x00000000U);
	psu_mask_write(0xFD070294, 0x00000001U, 0x00000001U);
	psu_mask_write(0xFD070300, 0x00000011U, 0x00000000U);
	psu_mask_write(0xFD07030C, 0x80000033U, 0x00000000U);
	psu_mask_write(0xFD070320, 0x00000001U, 0x00000000U);
	psu_mask_write(0xFD070400, 0x00000111U, 0x00000001U);
	psu_mask_write(0xFD070404, 0x000073FFU, 0x0000200FU);
	psu_mask_write(0xFD070408, 0x000073FFU, 0x0000200FU);
	psu_mask_write(0xFD070490, 0x00000001U, 0x00000001U);
	psu_mask_write(0xFD070494, 0x0033000FU, 0x0020000BU);
	psu_mask_write(0xFD070498, 0x07FF07FFU, 0x00000000U);
	psu_mask_write(0xFD0704B4, 0x000073FFU, 0x0000200FU);
	psu_mask_write(0xFD0704B8, 0x000073FFU, 0x0000200FU);
	psu_mask_write(0xFD070540, 0x00000001U, 0x00000001U);
	psu_mask_write(0xFD070544, 0x03330F0FU, 0x02000B03U);
	psu_mask_write(0xFD070548, 0x07FF07FFU, 0x00000000U);
	psu_mask_write(0xFD070564, 0x000073FFU, 0x0000200FU);
	psu_mask_write(0xFD070568, 0x000073FFU, 0x0000200FU);
	psu_mask_write(0xFD0705F0, 0x00000001U, 0x00000001U);
	psu_mask_write(0xFD0705F4, 0x03330F0FU, 0x02000B03U);
	psu_mask_write(0xFD0705F8, 0x07FF07FFU, 0x00000000U);
	psu_mask_write(0xFD070614, 0x000073FFU, 0x0000200FU);
	psu_mask_write(0xFD070618, 0x000073FFU, 0x0000200FU);
	psu_mask_write(0xFD0706A0, 0x00000001U, 0x00000001U);
	psu_mask_write(0xFD0706A4, 0x0033000FU, 0x00100003U);
	psu_mask_write(0xFD0706A8, 0x07FF07FFU, 0x0000004FU);
	psu_mask_write(0xFD0706AC, 0x0033000FU, 0x00100003U);
	psu_mask_write(0xFD0706B0, 0x000007FFU, 0x0000004FU);
	psu_mask_write(0xFD0706C4, 0x000073FFU, 0x0000200FU);
	psu_mask_write(0xFD0706C8, 0x000073FFU, 0x0000200FU);
	psu_mask_write(0xFD070750, 0x00000001U, 0x00000001U);
	psu_mask_write(0xFD070754, 0x0033000FU, 0x00100003U);
	psu_mask_write(0xFD070758, 0x07FF07FFU, 0x0000004FU);
	psu_mask_write(0xFD07075C, 0x0033000FU, 0x00100003U);
	psu_mask_write(0xFD070760, 0x000007FFU, 0x0000004FU);
	psu_mask_write(0xFD070774, 0x000073FFU, 0x0000200FU);
	psu_mask_write(0xFD070778, 0x000073FFU, 0x0000200FU);
	psu_mask_write(0xFD070800, 0x00000001U, 0x00000001U);
	psu_mask_write(0xFD070804, 0x0033000FU, 0x00100003U);
	psu_mask_write(0xFD070808, 0x07FF07FFU, 0x0000004FU);
	psu_mask_write(0xFD07080C, 0x0033000FU, 0x00100003U);
	psu_mask_write(0xFD070810, 0x000007FFU, 0x0000004FU);
	psu_mask_write(0xFD070F04, 0x000001FFU, 0x00000000U);
	psu_mask_write(0xFD070F08, 0x000000FFU, 0x00000000U);
	psu_mask_write(0xFD070F0C, 0x000001FFU, 0x00000010U);
	psu_mask_write(0xFD070F10, 0x000000FFU, 0x0000000FU);
	psu_mask_write(0xFD072190, 0x1FBFBF3FU, 0x07828002U);
	psu_mask_write(0xFD1A0108, 0x0000000CU, 0x00000000U);
	psu_mask_write(0xFD080010, 0xFFFFFFFFU, 0x87001E00U);
	psu_mask_write(0xFD080018, 0xFFFFFFFFU, 0x00F07E38U);
	psu_mask_write(0xFD08001C, 0xFFFFFFFFU, 0x55AA5480U);
	psu_mask_write(0xFD080024, 0xFFFFFFFFU, 0x010100F4U);
	psu_mask_write(0xFD080040, 0xFFFFFFFFU, 0x42C21590U);
	psu_mask_write(0xFD080044, 0xFFFFFFFFU, 0xD05512C0U);
	psu_mask_write(0xFD080068, 0xFFFFFFFFU, 0x01100000U);
	psu_mask_write(0xFD080090, 0xFFFFFFFFU, 0x02A04161U);
	psu_mask_write(0xFD0800C0, 0xFFFFFFFFU, 0x00000000U);
	psu_mask_write(0xFD0800C4, 0xFFFFFFFFU, 0x000000E4U);
	psu_mask_write(0xFD080100, 0xFFFFFFFFU, 0x0000040DU);
	psu_mask_write(0xFD080110, 0xFFFFFFFFU, 0x0B2E1708U);
	psu_mask_write(0xFD080114, 0xFFFFFFFFU, 0x282B0711U);
	psu_mask_write(0xFD080118, 0xFFFFFFFFU, 0x000F0133U);
	psu_mask_write(0xFD08011C, 0xFFFFFFFFU, 0x82000501U);
	psu_mask_write(0xFD080120, 0xFFFFFFFFU, 0x012B2B0BU);
	psu_mask_write(0xFD080124, 0xFFFFFFFFU, 0x0044260BU);
	psu_mask_write(0xFD080128, 0xFFFFFFFFU, 0x00000C18U);
	psu_mask_write(0xFD080140, 0xFFFFFFFFU, 0x08400020U);
	psu_mask_write(0xFD080144, 0xFFFFFFFFU, 0x00000C80U);
	psu_mask_write(0xFD080150, 0xFFFFFFFFU, 0x00000000U);
	psu_mask_write(0xFD080154, 0xFFFFFFFFU, 0x00000000U);
	psu_mask_write(0xFD080180, 0xFFFFFFFFU, 0x00000000U);
	psu_mask_write(0xFD080184, 0xFFFFFFFFU, 0x00000044U);
	psu_mask_write(0xFD080188, 0xFFFFFFFFU, 0x00000024U);
	psu_mask_write(0xFD08018C, 0xFFFFFFFFU, 0x00000031U);
	psu_mask_write(0xFD080190, 0xFFFFFFFFU, 0x00000008U);
	psu_mask_write(0xFD080194, 0xFFFFFFFFU, 0x00000000U);
	psu_mask_write(0xFD080198, 0xFFFFFFFFU, 0x00000000U);
	psu_mask_write(0xFD0801AC, 0xFFFFFFFFU, 0x00000056U);
	psu_mask_write(0xFD0801B0, 0xFFFFFFFFU, 0x00000056U);
	psu_mask_write(0xFD0801B4, 0xFFFFFFFFU, 0x00000008U);
	psu_mask_write(0xFD0801B8, 0xFFFFFFFFU, 0x00000019U);
	psu_mask_write(0xFD0801D8, 0xFFFFFFFFU, 0x00000016U);
	psu_mask_write(0xFD080200, 0xFFFFFFFFU, 0x800091C7U);
	psu_mask_write(0xFD080204, 0xFFFFFFFFU, 0x00010236U);
	psu_mask_write(0xFD080240, 0xFFFFFFFFU, 0x00141054U);
	psu_mask_write(0xFD080250, 0xFFFFFFFFU, 0x00088000U);
	psu_mask_write(0xFD080414, 0xFFFFFFFFU, 0x12340800U);
	psu_mask_write(0xFD0804F4, 0xFFFFFFFFU, 0x0000000AU);
	psu_mask_write(0xFD080500, 0xFFFFFFFFU, 0x30000028U);
	psu_mask_write(0xFD080508, 0xFFFFFFFFU, 0x00000000U);
	psu_mask_write(0xFD08050C, 0xFFFFFFFFU, 0x00000005U);
	psu_mask_write(0xFD080510, 0xFFFFFFFFU, 0x00000000U);
	psu_mask_write(0xFD080520, 0xFFFFFFFFU, 0x0300BD99U);
	psu_mask_write(0xFD080528, 0xFFFFFFFFU, 0xF1032019U);
	psu_mask_write(0xFD08052C, 0xFFFFFFFFU, 0x07F001E3U);
	psu_mask_write(0xFD080544, 0xFFFFFFFFU, 0x00000000U);
	psu_mask_write(0xFD080548, 0xFFFFFFFFU, 0x00000000U);
	psu_mask_write(0xFD080558, 0xFFFFFFFFU, 0x00000000U);
	psu_mask_write(0xFD08055C, 0xFFFFFFFFU, 0x00000000U);
	psu_mask_write(0xFD080560, 0xFFFFFFFFU, 0x00000000U);
	psu_mask_write(0xFD080564, 0xFFFFFFFFU, 0x00000000U);
	psu_mask_write(0xFD080680, 0xFFFFFFFFU, 0x008AAC58U);
	psu_mask_write(0xFD080684, 0xFFFFFFFFU, 0x0001B39BU);
	psu_mask_write(0xFD080694, 0xFFFFFFFFU, 0x01E10210U);
	psu_mask_write(0xFD080698, 0xFFFFFFFFU, 0x01E10000U);
	psu_mask_write(0xFD0806A4, 0xFFFFFFFFU, 0x0001BB9BU);
	psu_mask_write(0xFD080700, 0xFFFFFFFFU, 0x40800604U);
	psu_mask_write(0xFD080704, 0xFFFFFFFFU, 0x00007FFFU);
	psu_mask_write(0xFD08070C, 0xFFFFFFFFU, 0x3F000008U);
	psu_mask_write(0xFD080710, 0xFFFFFFFFU, 0x0E00F50CU);
	psu_mask_write(0xFD080714, 0xFFFFFFFFU, 0x09091616U);
	psu_mask_write(0xFD080718, 0xFFFFFFFFU, 0x09092B2BU);
	psu_mask_write(0xFD080800, 0xFFFFFFFFU, 0x40800604U);
	psu_mask_write(0xFD080804, 0xFFFFFFFFU, 0x00007FFFU);
	psu_mask_write(0xFD08080C, 0xFFFFFFFFU, 0x3F000008U);
	psu_mask_write(0xFD080810, 0xFFFFFFFFU, 0x0E00F50CU);
	psu_mask_write(0xFD080814, 0xFFFFFFFFU, 0x09091616U);
	psu_mask_write(0xFD080818, 0xFFFFFFFFU, 0x09092B2BU);
	psu_mask_write(0xFD080900, 0xFFFFFFFFU, 0x40800604U);
	psu_mask_write(0xFD080904, 0xFFFFFFFFU, 0x00007FFFU);
	psu_mask_write(0xFD08090C, 0xFFFFFFFFU, 0x3F000008U);
	psu_mask_write(0xFD080910, 0xFFFFFFFFU, 0x0E00F504U);
	psu_mask_write(0xFD080914, 0xFFFFFFFFU, 0x09091616U);
	psu_mask_write(0xFD080918, 0xFFFFFFFFU, 0x09092B2BU);
	psu_mask_write(0xFD080A00, 0xFFFFFFFFU, 0x40800604U);
	psu_mask_write(0xFD080A04, 0xFFFFFFFFU, 0x00007FFFU);
	psu_mask_write(0xFD080A0C, 0xFFFFFFFFU, 0x3F000008U);
	psu_mask_write(0xFD080A10, 0xFFFFFFFFU, 0x0E00F504U);
	psu_mask_write(0xFD080A14, 0xFFFFFFFFU, 0x09091616U);
	psu_mask_write(0xFD080A18, 0xFFFFFFFFU, 0x09092B2BU);
	psu_mask_write(0xFD080B00, 0xFFFFFFFFU, 0x80803660U);
	psu_mask_write(0xFD080B04, 0xFFFFFFFFU, 0x55556000U);
	psu_mask_write(0xFD080B08, 0xFFFFFFFFU, 0xAAAAAAAAU);
	psu_mask_write(0xFD080B0C, 0xFFFFFFFFU, 0x0029A4A4U);
	psu_mask_write(0xFD080B10, 0xFFFFFFFFU, 0x0C00BD00U);
	psu_mask_write(0xFD080B14, 0xFFFFFFFFU, 0x09091616U);
	psu_mask_write(0xFD080B18, 0xFFFFFFFFU, 0x09092B2BU);
	psu_mask_write(0xFD080C00, 0xFFFFFFFFU, 0x80803660U);
	psu_mask_write(0xFD080C04, 0xFFFFFFFFU, 0x55556000U);
	psu_mask_write(0xFD080C08, 0xFFFFFFFFU, 0xAAAAAAAAU);
	psu_mask_write(0xFD080C0C, 0xFFFFFFFFU, 0x0029A4A4U);
	psu_mask_write(0xFD080C10, 0xFFFFFFFFU, 0x0C00BD00U);
	psu_mask_write(0xFD080C14, 0xFFFFFFFFU, 0x09091616U);
	psu_mask_write(0xFD080C18, 0xFFFFFFFFU, 0x09092B2BU);
	psu_mask_write(0xFD080D00, 0xFFFFFFFFU, 0x80803660U);
	psu_mask_write(0xFD080D04, 0xFFFFFFFFU, 0x55556000U);
	psu_mask_write(0xFD080D08, 0xFFFFFFFFU, 0xAAAAAAAAU);
	psu_mask_write(0xFD080D0C, 0xFFFFFFFFU, 0x0029A4A4U);
	psu_mask_write(0xFD080D10, 0xFFFFFFFFU, 0x0C00BD00U);
	psu_mask_write(0xFD080D14, 0xFFFFFFFFU, 0x09091616U);
	psu_mask_write(0xFD080D18, 0xFFFFFFFFU, 0x09092B2BU);
	psu_mask_write(0xFD080E00, 0xFFFFFFFFU, 0x80803660U);
	psu_mask_write(0xFD080E04, 0xFFFFFFFFU, 0x55556000U);
	psu_mask_write(0xFD080E08, 0xFFFFFFFFU, 0xAAAAAAAAU);
	psu_mask_write(0xFD080E0C, 0xFFFFFFFFU, 0x0029A4A4U);
	psu_mask_write(0xFD080E10, 0xFFFFFFFFU, 0x0C00BD00U);
	psu_mask_write(0xFD080E14, 0xFFFFFFFFU, 0x09091616U);
	psu_mask_write(0xFD080E18, 0xFFFFFFFFU, 0x09092B2BU);
	psu_mask_write(0xFD080F00, 0xFFFFFFFFU, 0x80803660U);
	psu_mask_write(0xFD080F04, 0xFFFFFFFFU, 0x55556000U);
	psu_mask_write(0xFD080F08, 0xFFFFFFFFU, 0xAAAAAAAAU);
	psu_mask_write(0xFD080F0C, 0xFFFFFFFFU, 0x0029A4A4U);
	psu_mask_write(0xFD080F10, 0xFFFFFFFFU, 0x0C00BD00U);
	psu_mask_write(0xFD080F14, 0xFFFFFFFFU, 0x09091616U);
	psu_mask_write(0xFD080F18, 0xFFFFFFFFU, 0x09092B2BU);
	psu_mask_write(0xFD081400, 0xFFFFFFFFU, 0x2A019FFEU);
	psu_mask_write(0xFD081404, 0xFFFFFFFFU, 0x01100000U);
	psu_mask_write(0xFD08141C, 0xFFFFFFFFU, 0x01264300U);
	psu_mask_write(0xFD08142C, 0xFFFFFFFFU, 0x000C1800U);
	psu_mask_write(0xFD081430, 0xFFFFFFFFU, 0x71000000U);
	psu_mask_write(0xFD081440, 0xFFFFFFFFU, 0x2A019FFEU);
	psu_mask_write(0xFD081444, 0xFFFFFFFFU, 0x01100000U);
	psu_mask_write(0xFD08145C, 0xFFFFFFFFU, 0x01264300U);
	psu_mask_write(0xFD08146C, 0xFFFFFFFFU, 0x000C1800U);
	psu_mask_write(0xFD081470, 0xFFFFFFFFU, 0x71000000U);
	psu_mask_write(0xFD081480, 0xFFFFFFFFU, 0x15019FFEU);
	psu_mask_write(0xFD081484, 0xFFFFFFFFU, 0x21100000U);
	psu_mask_write(0xFD08149C, 0xFFFFFFFFU, 0x01266300U);
	psu_mask_write(0xFD0814AC, 0xFFFFFFFFU, 0x000C1800U);
	psu_mask_write(0xFD0814B0, 0xFFFFFFFFU, 0x70400000U);
	psu_mask_write(0xFD0814C0, 0xFFFFFFFFU, 0x15019FFEU);
	psu_mask_write(0xFD0814C4, 0xFFFFFFFFU, 0x21100000U);
	psu_mask_write(0xFD0814DC, 0xFFFFFFFFU, 0x01266300U);
	psu_mask_write(0xFD0814EC, 0xFFFFFFFFU, 0x000C1800U);
	psu_mask_write(0xFD0814F0, 0xFFFFFFFFU, 0x70400000U);
	psu_mask_write(0xFD081500, 0xFFFFFFFFU, 0x15019FFEU);
	psu_mask_write(0xFD081504, 0xFFFFFFFFU, 0x21100000U);
	psu_mask_write(0xFD08151C, 0xFFFFFFFFU, 0x01266300U);
	psu_mask_write(0xFD08152C, 0xFFFFFFFFU, 0x000C1800U);
	psu_mask_write(0xFD081530, 0xFFFFFFFFU, 0x70400000U);
	psu_mask_write(0xFD0817DC, 0xFFFFFFFFU, 0x012643C4U);

	return 1;
}

static unsigned long psu_ddr_qos_init_data(void)
{
	psu_mask_write(0xFD360008, 0x0000000FU, 0x00000000U);
	psu_mask_write(0xFD36001C, 0x0000000FU, 0x00000000U);
	psu_mask_write(0xFD370008, 0x0000000FU, 0x00000000U);
	psu_mask_write(0xFD37001C, 0x0000000FU, 0x00000000U);
	psu_mask_write(0xFD380008, 0x0000000FU, 0x00000000U);
	psu_mask_write(0xFD38001C, 0x0000000FU, 0x00000000U);
	psu_mask_write(0xFD390008, 0x0000000FU, 0x00000000U);
	psu_mask_write(0xFD39001C, 0x0000000FU, 0x00000000U);
	psu_mask_write(0xFD3A0008, 0x0000000FU, 0x00000000U);
	psu_mask_write(0xFD3A001C, 0x0000000FU, 0x00000000U);
	psu_mask_write(0xFD3B0008, 0x0000000FU, 0x00000000U);
	psu_mask_write(0xFD3B001C, 0x0000000FU, 0x00000000U);
	psu_mask_write(0xFF9B0008, 0x0000000FU, 0x00000000U);
	psu_mask_write(0xFF9B001C, 0x0000000FU, 0x00000000U);

	return 1;
}

static unsigned long psu_mio_init_data(void)
{
	psu_mask_write(0xFF180000, 0x000000FEU, 0x00000000U);
	psu_mask_write(0xFF180004, 0x000000FEU, 0x00000000U);
	psu_mask_write(0xFF180008, 0x000000FEU, 0x00000000U);
	psu_mask_write(0xFF18000C, 0x000000FEU, 0x00000000U);
	psu_mask_write(0xFF180010, 0x000000FEU, 0x00000000U);
	psu_mask_write(0xFF180014, 0x000000FEU, 0x00000000U);
	psu_mask_write(0xFF180018, 0x000000FEU, 0x00000000U);
	psu_mask_write(0xFF18001C, 0x000000FEU, 0x00000000U);
	psu_mask_write(0xFF180020, 0x000000FEU, 0x00000000U);
	psu_mask_write(0xFF180024, 0x000000FEU, 0x00000000U);
	psu_mask_write(0xFF180028, 0x000000FEU, 0x00000000U);
	psu_mask_write(0xFF18002C, 0x000000FEU, 0x00000000U);
	psu_mask_write(0xFF180030, 0x000000FEU, 0x00000000U);
	psu_mask_write(0xFF180034, 0x000000FEU, 0x00000000U);
	psu_mask_write(0xFF180038, 0x000000FEU, 0x00000000U);
	psu_mask_write(0xFF18003C, 0x000000FEU, 0x00000000U);
	psu_mask_write(0xFF180040, 0x000000FEU, 0x00000000U);
	psu_mask_write(0xFF180044, 0x000000FEU, 0x00000000U);
	psu_mask_write(0xFF180048, 0x000000FEU, 0x00000000U);
	psu_mask_write(0xFF18004C, 0x000000FEU, 0x00000000U);
	psu_mask_write(0xFF180050, 0x000000FEU, 0x00000000U);
	psu_mask_write(0xFF180054, 0x000000FEU, 0x00000000U);
	psu_mask_write(0xFF180058, 0x000000FEU, 0x00000000U);
	psu_mask_write(0xFF18005C, 0x000000FEU, 0x00000000U);
	psu_mask_write(0xFF180060, 0x000000FEU, 0x00000000U);
	psu_mask_write(0xFF180064, 0x000000FEU, 0x00000000U);
	psu_mask_write(0xFF180068, 0x000000FEU, 0x00000000U);
	psu_mask_write(0xFF18006C, 0x000000FEU, 0x00000000U);
	psu_mask_write(0xFF180070, 0x000000FEU, 0x00000000U);
	psu_mask_write(0xFF180074, 0x000000FEU, 0x00000000U);
	psu_mask_write(0xFF180078, 0x000000FEU, 0x00000000U);
	psu_mask_write(0xFF18007C, 0x000000FEU, 0x00000000U);
	psu_mask_write(0xFF180080, 0x000000FEU, 0x00000000U);
	psu_mask_write(0xFF180084, 0x000000FEU, 0x00000000U);
	psu_mask_write(0xFF180088, 0x000000FEU, 0x00000040U);
	psu_mask_write(0xFF18008C, 0x000000FEU, 0x00000040U);
	psu_mask_write(0xFF180090, 0x000000FEU, 0x00000040U);
	psu_mask_write(0xFF180094, 0x000000FEU, 0x00000040U);
	psu_mask_write(0xFF180098, 0x000000FEU, 0x000000C0U);
	psu_mask_write(0xFF18009C, 0x000000FEU, 0x000000C0U);
	psu_mask_write(0xFF1800A0, 0x000000FEU, 0x00000000U);
	psu_mask_write(0xFF1800A4, 0x000000FEU, 0x00000000U);
	psu_mask_write(0xFF1800A8, 0x000000FEU, 0x00000000U);
	psu_mask_write(0xFF1800AC, 0x000000FEU, 0x00000000U);
	psu_mask_write(0xFF1800B0, 0x000000FEU, 0x00000000U);
	psu_mask_write(0xFF1800B4, 0x000000FEU, 0x00000010U);
	psu_mask_write(0xFF1800B8, 0x000000FEU, 0x00000010U);
	psu_mask_write(0xFF1800BC, 0x000000FEU, 0x00000010U);
	psu_mask_write(0xFF1800C0, 0x000000FEU, 0x00000010U);
	psu_mask_write(0xFF1800C4, 0x000000FEU, 0x00000010U);
	psu_mask_write(0xFF1800C8, 0x000000FEU, 0x00000010U);
	psu_mask_write(0xFF1800CC, 0x000000FEU, 0x00000010U);
	psu_mask_write(0xFF1800D0, 0x000000FEU, 0x00000000U);
	psu_mask_write(0xFF1800D4, 0x000000FEU, 0x00000000U);
	psu_mask_write(0xFF1800D8, 0x000000FEU, 0x00000000U);
	psu_mask_write(0xFF1800DC, 0x000000FEU, 0x00000000U);
	psu_mask_write(0xFF1800E0, 0x000000FEU, 0x00000000U);
	psu_mask_write(0xFF1800E4, 0x000000FEU, 0x00000000U);
	psu_mask_write(0xFF1800E8, 0x000000FEU, 0x00000000U);
	psu_mask_write(0xFF1800EC, 0x000000FEU, 0x00000000U);
	psu_mask_write(0xFF1800F0, 0x000000FEU, 0x00000000U);
	psu_mask_write(0xFF1800F4, 0x000000FEU, 0x00000000U);
	psu_mask_write(0xFF1800F8, 0x000000FEU, 0x00000000U);
	psu_mask_write(0xFF1800FC, 0x000000FEU, 0x00000000U);
	psu_mask_write(0xFF180100, 0x000000FEU, 0x00000000U);
	psu_mask_write(0xFF180104, 0x000000FEU, 0x00000000U);
	psu_mask_write(0xFF180108, 0x000000FEU, 0x00000000U);
	psu_mask_write(0xFF18010C, 0x000000FEU, 0x00000000U);
	psu_mask_write(0xFF180110, 0x000000FEU, 0x00000000U);
	psu_mask_write(0xFF180114, 0x000000FEU, 0x00000000U);
	psu_mask_write(0xFF180118, 0x000000FEU, 0x00000000U);
	psu_mask_write(0xFF18011C, 0x000000FEU, 0x00000000U);
	psu_mask_write(0xFF180120, 0x000000FEU, 0x00000000U);
	psu_mask_write(0xFF180124, 0x000000FEU, 0x00000000U);
	psu_mask_write(0xFF180128, 0x000000FEU, 0x00000000U);
	psu_mask_write(0xFF18012C, 0x000000FEU, 0x00000000U);
	psu_mask_write(0xFF180130, 0x000000FEU, 0x00000060U);
	psu_mask_write(0xFF180134, 0x000000FEU, 0x00000060U);
	psu_mask_write(0xFF180204, 0xFFFFFFFFU, 0x00000000U);
	psu_mask_write(0xFF180208, 0xFFFFFFFFU, 0x00002040U);
	psu_mask_write(0xFF18020C, 0x00003FFFU, 0x00000000U);
	psu_mask_write(0xFF180138, 0x03FFFFFFU, 0x03FFFFFFU);
	psu_mask_write(0xFF18013C, 0x03FFFFFFU, 0x03FFFFFFU);
	psu_mask_write(0xFF180140, 0x03FFFFFFU, 0x00000000U);
	psu_mask_write(0xFF180144, 0x03FFFFFFU, 0x03FFFFFFU);
	psu_mask_write(0xFF180148, 0x03FFFFFFU, 0x03FFFFFFU);
	psu_mask_write(0xFF18014C, 0x03FFFFFFU, 0x00000000U);
	psu_mask_write(0xFF180154, 0x03FFFFFFU, 0x03FFFFFFU);
	psu_mask_write(0xFF180158, 0x03FFFFFFU, 0x03FFFFFFU);
	psu_mask_write(0xFF18015C, 0x03FFFFFFU, 0x00000000U);
	psu_mask_write(0xFF180160, 0x03FFFFFFU, 0x03FFFFFFU);
	psu_mask_write(0xFF180164, 0x03FFFFFFU, 0x03FFFFFFU);
	psu_mask_write(0xFF180168, 0x03FFFFFFU, 0x00000000U);
	psu_mask_write(0xFF180170, 0x03FFFFFFU, 0x03FFFFFFU);
	psu_mask_write(0xFF180174, 0x03FFFFFFU, 0x03FFFFFFU);
	psu_mask_write(0xFF180178, 0x03FFFFFFU, 0x00000000U);
	psu_mask_write(0xFF18017C, 0x03FFFFFFU, 0x03FFFFFFU);
	psu_mask_write(0xFF180180, 0x03FFFFFFU, 0x03FFFFFFU);
	psu_mask_write(0xFF180184, 0x03FFFFFFU, 0x00000000U);
	psu_mask_write(0xFF180200, 0x0000000FU, 0x00000000U);

	return 1;
}

static unsigned long psu_peripherals_pre_init_data(void)
{
	psu_mask_write(0xFF5E0108, 0x013F3F07U, 0x01012302U);

	return 1;
}

static unsigned long psu_peripherals_init_data(void)
{
	psu_mask_write(0xFD1A0100, 0x0000007CU, 0x00000000U);
	psu_mask_write(0xFF5E0238, 0x001A0000U, 0x00000000U);
	psu_mask_write(0xFF5E023C, 0x0093C018U, 0x00000000U);
	psu_mask_write(0xFF5E0230, 0x00000001U, 0x00000000U);
	psu_mask_write(0xFF5E0238, 0x00000040U, 0x00000000U);
	psu_mask_write(0xFF180310, 0x00008000U, 0x00000000U);
	psu_mask_write(0xFF180320, 0x33840000U, 0x00800000U);
	psu_mask_write(0xFF18031C, 0x7FFE0000U, 0x64500000U);
	psu_mask_write(0xFF180358, 0x00000008U, 0x00000008U);
	psu_mask_write(0xFF180324, 0x03C00000U, 0x00000000U);
	psu_mask_write(0xFF5E0238, 0x00000600U, 0x00000000U);
	psu_mask_write(0xFF5E0238, 0x00000002U, 0x00000000U);
	psu_mask_write(0xFF5E0238, 0x00040000U, 0x00000000U);
	psu_mask_write(0xFF4B0024, 0x000000FFU, 0x000000FFU);
	psu_mask_write(0xFFCA5000, 0x00001FFFU, 0x00000000U);
	psu_mask_write(0xFD5C0060, 0x000F000FU, 0x00000000U);
	psu_mask_write(0xFFA60040, 0x80000000U, 0x80000000U);
	psu_mask_write(0xFF260020, 0xFFFFFFFFU, 0x05F5DD18U);
	psu_mask_write(0xFF260000, 0x00000001U, 0x00000001U);
	return 1;
}

static unsigned long psu_serdes_init_data(void)
{
	psu_mask_write(0xFD410000, 0x0000001FU, 0x0000000FU);
	psu_mask_write(0xFD402860, 0x00000080U, 0x00000080U);
	psu_mask_write(0xFD40106C, 0x0000000FU, 0x0000000FU);
	psu_mask_write(0xFD4000F4, 0x0000000BU, 0x0000000BU);
	psu_mask_write(0xFD401074, 0x00000010U, 0x00000010U);
	psu_mask_write(0xFD405074, 0x00000010U, 0x00000010U);
	psu_mask_write(0xFD409074, 0x00000010U, 0x00000010U);
	psu_mask_write(0xFD40D074, 0x00000010U, 0x00000010U);
	psu_mask_write(0xFD40189C, 0x00000080U, 0x00000080U);
	psu_mask_write(0xFD4018F8, 0x000000FFU, 0x0000007DU);
	psu_mask_write(0xFD4018FC, 0x000000FFU, 0x0000007DU);
	psu_mask_write(0xFD401990, 0x000000FFU, 0x00000000U);
	psu_mask_write(0xFD401924, 0x000000FFU, 0x00000082U);
	psu_mask_write(0xFD401928, 0x000000FFU, 0x00000000U);
	psu_mask_write(0xFD401900, 0x000000FFU, 0x00000064U);
	psu_mask_write(0xFD40192C, 0x000000FFU, 0x00000000U);
	psu_mask_write(0xFD401980, 0x000000FFU, 0x000000FFU);
	psu_mask_write(0xFD401914, 0x000000FFU, 0x000000FFU);
	psu_mask_write(0xFD401918, 0x00000001U, 0x00000001U);
	psu_mask_write(0xFD401940, 0x000000FFU, 0x000000FFU);
	psu_mask_write(0xFD401944, 0x00000001U, 0x00000001U);
	psu_mask_write(0xFD401994, 0x00000007U, 0x00000007U);
	psu_mask_write(0xFD405994, 0x00000007U, 0x00000007U);
	psu_mask_write(0xFD409994, 0x00000007U, 0x00000007U);
	psu_mask_write(0xFD40D994, 0x00000007U, 0x00000007U);
	psu_mask_write(0xFD40107C, 0x0000000FU, 0x00000001U);
	psu_mask_write(0xFD40507C, 0x0000000FU, 0x00000001U);
	psu_mask_write(0xFD40907C, 0x0000000FU, 0x00000001U);
	psu_mask_write(0xFD40D07C, 0x0000000FU, 0x00000001U);
	psu_mask_write(0xFD4019A4, 0x000000FFU, 0x000000FFU);
	psu_mask_write(0xFD401038, 0x00000040U, 0x00000040U);
	psu_mask_write(0xFD40102C, 0x00000040U, 0x00000040U);
	psu_mask_write(0xFD4059A4, 0x000000FFU, 0x000000FFU);
	psu_mask_write(0xFD405038, 0x00000040U, 0x00000040U);
	psu_mask_write(0xFD40502C, 0x00000040U, 0x00000040U);
	psu_mask_write(0xFD4099A4, 0x000000FFU, 0x000000FFU);
	psu_mask_write(0xFD409038, 0x00000040U, 0x00000040U);
	psu_mask_write(0xFD40902C, 0x00000040U, 0x00000040U);
	psu_mask_write(0xFD40D9A4, 0x000000FFU, 0x000000FFU);
	psu_mask_write(0xFD40D038, 0x00000040U, 0x00000040U);
	psu_mask_write(0xFD40D02C, 0x00000040U, 0x00000040U);
	psu_mask_write(0xFD4019AC, 0x00000003U, 0x00000000U);
	psu_mask_write(0xFD4059AC, 0x00000003U, 0x00000000U);
	psu_mask_write(0xFD4099AC, 0x00000003U, 0x00000000U);
	psu_mask_write(0xFD40D9AC, 0x00000003U, 0x00000000U);
	psu_mask_write(0xFD401978, 0x00000010U, 0x00000010U);
	psu_mask_write(0xFD405978, 0x00000010U, 0x00000010U);
	psu_mask_write(0xFD409978, 0x00000010U, 0x00000010U);
	psu_mask_write(0xFD40D978, 0x00000010U, 0x00000010U);

	serdes_illcalib(0, 0, 0, 0, 0, 0, 5, 0);
	psu_mask_write(0xFD410010, 0x00000007U, 0x00000005U);
	psu_mask_write(0xFD410040, 0x00000003U, 0x00000000U);
	psu_mask_write(0xFD410044, 0x00000003U, 0x00000000U);

	return 1;
}

static unsigned long psu_resetout_init_data(void)
{
	psu_mask_write(0xFF5E0230, 0x00000001U, 0x00000000U);
	psu_mask_write(0xFD480064, 0x00000200U, 0x00000200U);
	mask_poll(0xFD4023E4, 0x00000010U);

	return 1;
}

static unsigned long psu_resetin_init_data(void)
{
	psu_mask_write(0xFF5E0230, 0x00000001U, 0x00000001U);

	return 1;
}

static unsigned long psu_afi_config(void)
{
	psu_mask_write(0xFD1A0100, 0x00001F80U, 0x00000000U);
	psu_mask_write(0xFF5E023C, 0x00080000U, 0x00000000U);
	psu_mask_write(0xFF419000, 0x00000300U, 0x00000000U);

	return 1;
}

static unsigned long psu_ddr_phybringup_data(void)
{
	unsigned int regval = 0;

	for (int tp = 0; tp < 20; tp++)
		regval = Xil_In32(0xFD070018);
	int cur_PLLCR0;

	cur_PLLCR0 = (Xil_In32(0xFD080068U) & 0xFFFFFFFFU) >> 0x00000000U;
	int cur_DX8SL0PLLCR0;

	cur_DX8SL0PLLCR0 = (Xil_In32(0xFD081404U) & 0xFFFFFFFFU) >> 0x00000000U;
	int cur_DX8SL1PLLCR0;

	cur_DX8SL1PLLCR0 = (Xil_In32(0xFD081444U) & 0xFFFFFFFFU) >> 0x00000000U;
	int cur_DX8SL2PLLCR0;

	cur_DX8SL2PLLCR0 = (Xil_In32(0xFD081484U) & 0xFFFFFFFFU) >> 0x00000000U;
	int cur_DX8SL3PLLCR0;

	cur_DX8SL3PLLCR0 = (Xil_In32(0xFD0814C4U) & 0xFFFFFFFFU) >> 0x00000000U;
	int cur_DX8SL4PLLCR0;

	cur_DX8SL4PLLCR0 = (Xil_In32(0xFD081504U) & 0xFFFFFFFFU) >> 0x00000000U;
	int cur_DX8SLBPLLCR0;

	cur_DX8SLBPLLCR0 = (Xil_In32(0xFD0817C4U) & 0xFFFFFFFFU) >> 0x00000000U;
	Xil_Out32(0xFD080068, 0x02120000);
	Xil_Out32(0xFD081404, 0x02120000);
	Xil_Out32(0xFD081444, 0x02120000);
	Xil_Out32(0xFD081484, 0x02120000);
	Xil_Out32(0xFD0814C4, 0x02120000);
	Xil_Out32(0xFD081504, 0x02120000);
	Xil_Out32(0xFD0817C4, 0x02120000);
	int cur_div2;

	cur_div2 = (Xil_In32(0xFD1A002CU) & 0x00010000U) >> 0x00000010U;
	int cur_fbdiv;

	cur_fbdiv = (Xil_In32(0xFD1A002CU) & 0x00007F00U) >> 0x00000008U;
	dpll_prog(1, 49, 63, 625, 3, 3, 2);
	for (int tp = 0; tp < 20; tp++)
		regval = Xil_In32(0xFD070018);
	unsigned int pll_retry = 10;
	unsigned int pll_locked = 0;

	while ((pll_retry > 0) && (!pll_locked)) {
		Xil_Out32(0xFD080004, 0x00040010);
		Xil_Out32(0xFD080004, 0x00040011);

		while ((Xil_In32(0xFD080030) & 0x1) != 1)
			;
		pll_locked = (Xil_In32(0xFD080030) & 0x80000000)
		    >> 31;
		pll_locked &= (Xil_In32(0xFD0807E0) & 0x10000)
		    >> 16;
		pll_locked &= (Xil_In32(0xFD0809E0) & 0x10000) >> 16;
		pll_retry--;
	}
	Xil_Out32(0xFD0800C4, Xil_In32(0xFD0800C4) | (pll_retry << 16));
	if (!pll_locked)
		return 0;

	Xil_Out32(0xFD080004U, 0x00040063U);
	Xil_Out32(0xFD0800C0U, 0x00000001U);

	while ((Xil_In32(0xFD080030U) & 0x0000000FU) != 0x0000000FU)
		;
	prog_reg(0xFD080004U, 0x00000001U, 0x00000000U, 0x00000001U);

	while ((Xil_In32(0xFD080030U) & 0x000000FFU) != 0x0000001FU)
		;
	Xil_Out32(0xFD070010U, 0x80000018U);
	Xil_Out32(0xFD0701B0U, 0x00000005U);
	regval = Xil_In32(0xFD070018);
	while ((regval & 0x1) != 0x0)
		regval = Xil_In32(0xFD070018);

	regval = Xil_In32(0xFD070018);
	regval = Xil_In32(0xFD070018);
	regval = Xil_In32(0xFD070018);
	regval = Xil_In32(0xFD070018);
	regval = Xil_In32(0xFD070018);
	regval = Xil_In32(0xFD070018);
	regval = Xil_In32(0xFD070018);
	regval = Xil_In32(0xFD070018);
	regval = Xil_In32(0xFD070018);
	regval = Xil_In32(0xFD070018);
	Xil_Out32(0xFD070014U, 0x00000331U);
	Xil_Out32(0xFD070010U, 0x80000018U);
	regval = Xil_In32(0xFD070018);
	while ((regval & 0x1) != 0x0)
		regval = Xil_In32(0xFD070018);

	regval = Xil_In32(0xFD070018);
	regval = Xil_In32(0xFD070018);
	regval = Xil_In32(0xFD070018);
	regval = Xil_In32(0xFD070018);
	regval = Xil_In32(0xFD070018);
	regval = Xil_In32(0xFD070018);
	regval = Xil_In32(0xFD070018);
	regval = Xil_In32(0xFD070018);
	regval = Xil_In32(0xFD070018);
	regval = Xil_In32(0xFD070018);
	Xil_Out32(0xFD070014U, 0x00000B36U);
	Xil_Out32(0xFD070010U, 0x80000018U);
	regval = Xil_In32(0xFD070018);
	while ((regval & 0x1) != 0x0)
		regval = Xil_In32(0xFD070018);

	regval = Xil_In32(0xFD070018);
	regval = Xil_In32(0xFD070018);
	regval = Xil_In32(0xFD070018);
	regval = Xil_In32(0xFD070018);
	regval = Xil_In32(0xFD070018);
	regval = Xil_In32(0xFD070018);
	regval = Xil_In32(0xFD070018);
	regval = Xil_In32(0xFD070018);
	regval = Xil_In32(0xFD070018);
	regval = Xil_In32(0xFD070018);
	Xil_Out32(0xFD070014U, 0x00000C56U);
	Xil_Out32(0xFD070010U, 0x80000018U);
	regval = Xil_In32(0xFD070018);
	while ((regval & 0x1) != 0x0)
		regval = Xil_In32(0xFD070018);

	regval = Xil_In32(0xFD070018);
	regval = Xil_In32(0xFD070018);
	regval = Xil_In32(0xFD070018);
	regval = Xil_In32(0xFD070018);
	regval = Xil_In32(0xFD070018);
	regval = Xil_In32(0xFD070018);
	regval = Xil_In32(0xFD070018);
	regval = Xil_In32(0xFD070018);
	regval = Xil_In32(0xFD070018);
	regval = Xil_In32(0xFD070018);
	Xil_Out32(0xFD070014U, 0x00000E19U);
	Xil_Out32(0xFD070010U, 0x80000018U);
	regval = Xil_In32(0xFD070018);
	while ((regval & 0x1) != 0x0)
		regval = Xil_In32(0xFD070018);

	regval = Xil_In32(0xFD070018);
	regval = Xil_In32(0xFD070018);
	regval = Xil_In32(0xFD070018);
	regval = Xil_In32(0xFD070018);
	regval = Xil_In32(0xFD070018);
	regval = Xil_In32(0xFD070018);
	regval = Xil_In32(0xFD070018);
	regval = Xil_In32(0xFD070018);
	regval = Xil_In32(0xFD070018);
	regval = Xil_In32(0xFD070018);
	Xil_Out32(0xFD070014U, 0x00001616U);
	Xil_Out32(0xFD070010U, 0x80000018U);
	Xil_Out32(0xFD070010U, 0x80000010U);
	Xil_Out32(0xFD0701B0U, 0x00000005U);
	Xil_Out32(0xFD070320U, 0x00000001U);
	while ((Xil_In32(0xFD070004U) & 0x0000000FU) != 0x00000001U)
		;
	prog_reg(0xFD0701B0U, 0x00000001U, 0x00000000U, 0x00000000U);
	prog_reg(0xFD080014U, 0x00000040U, 0x00000006U, 0x00000001U);
	prog_reg(0xFD080028U, 0x00000001U, 0x00000000U, 0x00000001U);
	prog_reg(0xFD080004U, 0x20000000U, 0x0000001DU, 0x00000001U);
	prog_reg(0xFD08016CU, 0x00000004U, 0x00000002U, 0x00000001U);
	prog_reg(0xFD080168U, 0x000000F0U, 0x00000004U, 0x00000007U);
	prog_reg(0xFD080168U, 0x00000F00U, 0x00000008U, 0x00000002U);
	prog_reg(0xFD080168U, 0x0000000FU, 0x00000000U, 0x00000001U);
	for (int tp = 0; tp < 20; tp++)
		regval = Xil_In32(0xFD070018);

	Xil_Out32(0xFD080068, cur_PLLCR0);
	Xil_Out32(0xFD081404, cur_DX8SL0PLLCR0);
	Xil_Out32(0xFD081444, cur_DX8SL1PLLCR0);
	Xil_Out32(0xFD081484, cur_DX8SL2PLLCR0);
	Xil_Out32(0xFD0814C4, cur_DX8SL3PLLCR0);
	Xil_Out32(0xFD081504, cur_DX8SL4PLLCR0);
	Xil_Out32(0xFD0817C4, cur_DX8SLBPLLCR0);
	for (int tp = 0; tp < 20; tp++)
		regval = Xil_In32(0xFD070018);

	dpll_prog(cur_div2, cur_fbdiv, 63, 625, 3, 3, 2);
	for (int tp = 0; tp < 2000; tp++)
		regval = Xil_In32(0xFD070018);

	prog_reg(0xFD080004U, 0x20000000U, 0x0000001DU, 0x00000000U);
	prog_reg(0xFD080004U, 0x00040000U, 0x00000012U, 0x00000001U);
	prog_reg(0xFD080004U, 0x00000040U, 0x00000006U, 0x00000001U);
	prog_reg(0xFD080004U, 0x00000020U, 0x00000005U, 0x00000001U);
	prog_reg(0xFD080004U, 0x00000010U, 0x00000004U, 0x00000001U);
	prog_reg(0xFD080004U, 0x00000001U, 0x00000000U, 0x00000001U);

	while ((Xil_In32(0xFD080030U) & 0x0000000FU) != 0x0000000FU)
		;
	prog_reg(0xFD080004U, 0x00000001U, 0x00000000U, 0x00000001U);

	while ((Xil_In32(0xFD080030U) & 0x000000FFU) != 0x0000001FU)
		;
	for (int tp = 0; tp < 2000; tp++)
		regval = Xil_In32(0xFD070018);

	prog_reg(0xFD080028U, 0x00000001U, 0x00000000U, 0x00000000U);
	prog_reg(0xFD08016CU, 0x00000004U, 0x00000002U, 0x00000001U);
	prog_reg(0xFD080168U, 0x000000F0U, 0x00000004U, 0x00000007U);
	prog_reg(0xFD080168U, 0x00000F00U, 0x00000008U, 0x00000003U);
	prog_reg(0xFD080168U, 0x0000000FU, 0x00000000U, 0x00000001U);
	for (int tp = 0; tp < 2000; tp++)
		regval = Xil_In32(0xFD070018);

	prog_reg(0xFD080014U, 0x00000040U, 0x00000006U, 0x00000001U);
	Xil_Out32(0xFD080004, 0x0014FE01);

	regval = Xil_In32(0xFD080030);
	while (regval != 0x8000007E)
		regval = Xil_In32(0xFD080030);

	Xil_Out32(0xFD080200U, 0x000091C7U);
	regval = Xil_In32(0xFD080030);
	while (regval != 0x80008FFF)
		regval = Xil_In32(0xFD080030);

	Xil_Out32(0xFD080200U, 0x800091C7U);
	regval = ((Xil_In32(0xFD080030) & 0x1FFF0000) >> 18);
	if (regval != 0)
		return 0;
	prog_reg(0xFD070320U, 0x00000001U, 0x00000000U, 0x00000000U);
	prog_reg(0xFD0701B0U, 0x00000001U, 0x00000000U, 0x00000001U);
	prog_reg(0xFD0701A0U, 0x80000000U, 0x0000001FU, 0x00000000U);
	prog_reg(0xFD070320U, 0x00000001U, 0x00000000U, 0x00000001U);
	Xil_Out32(0xFD070180U, 0x02160010U);
	Xil_Out32(0xFD070060U, 0x00000000U);
	prog_reg(0xFD080014U, 0x00000040U, 0x00000006U, 0x00000000U);
	for (int tp = 0; tp < 4000; tp++)
		regval = Xil_In32(0xFD070018);

	prog_reg(0xFD080090U, 0x00000FC0U, 0x00000006U, 0x00000007U);
	prog_reg(0xFD080090U, 0x00000004U, 0x00000002U, 0x00000001U);
	prog_reg(0xFD08070CU, 0x02000000U, 0x00000019U, 0x00000000U);
	prog_reg(0xFD08080CU, 0x02000000U, 0x00000019U, 0x00000000U);
	prog_reg(0xFD08090CU, 0x02000000U, 0x00000019U, 0x00000000U);
	prog_reg(0xFD080A0CU, 0x02000000U, 0x00000019U, 0x00000000U);
	prog_reg(0xFD080F0CU, 0x02000000U, 0x00000019U, 0x00000000U);
	prog_reg(0xFD080200U, 0x00000010U, 0x00000004U, 0x00000001U);
	prog_reg(0xFD080250U, 0x00000002U, 0x00000001U, 0x00000000U);
	prog_reg(0xFD080250U, 0x0000000CU, 0x00000002U, 0x00000001U);
	prog_reg(0xFD080250U, 0x000000F0U, 0x00000004U, 0x00000000U);
	prog_reg(0xFD080250U, 0x00300000U, 0x00000014U, 0x00000001U);
	prog_reg(0xFD080250U, 0xF0000000U, 0x0000001CU, 0x00000002U);
	prog_reg(0xFD08070CU, 0x08000000U, 0x0000001BU, 0x00000000U);
	prog_reg(0xFD08080CU, 0x08000000U, 0x0000001BU, 0x00000000U);
	prog_reg(0xFD08090CU, 0x08000000U, 0x0000001BU, 0x00000000U);
	prog_reg(0xFD080A0CU, 0x08000000U, 0x0000001BU, 0x00000000U);
	prog_reg(0xFD080B0CU, 0x08000000U, 0x0000001BU, 0x00000000U);
	prog_reg(0xFD080C0CU, 0x08000000U, 0x0000001BU, 0x00000000U);
	prog_reg(0xFD080D0CU, 0x08000000U, 0x0000001BU, 0x00000000U);
	prog_reg(0xFD080E0CU, 0x08000000U, 0x0000001BU, 0x00000000U);
	prog_reg(0xFD080F0CU, 0x08000000U, 0x0000001BU, 0x00000000U);
	prog_reg(0xFD080254U, 0x000000FFU, 0x00000000U, 0x00000001U);
	prog_reg(0xFD080254U, 0x000F0000U, 0x00000010U, 0x0000000AU);
	prog_reg(0xFD080250U, 0x00000001U, 0x00000000U, 0x00000001U);

	return 1;
}

static int serdes_enb_coarse_saturation(void)
{
	Xil_Out32(0xFD402094, 0x00000010);
	Xil_Out32(0xFD406094, 0x00000010);
	Xil_Out32(0xFD40A094, 0x00000010);
	Xil_Out32(0xFD40E094, 0x00000010);
	return 1;
}

static int serdes_fixcal_code(void)
{
	int maskstatus = 1;
	unsigned int rdata = 0;
	unsigned int match_pmos_code[23];
	unsigned int match_nmos_code[23];
	unsigned int match_ical_code[7];
	unsigned int match_rcal_code[7];
	unsigned int p_code = 0;
	unsigned int n_code = 0;
	unsigned int i_code = 0;
	unsigned int r_code = 0;
	unsigned int repeat_count = 0;
	unsigned int L3_TM_CALIB_DIG20 = 0;
	unsigned int L3_TM_CALIB_DIG19 = 0;
	unsigned int L3_TM_CALIB_DIG18 = 0;
	unsigned int L3_TM_CALIB_DIG16 = 0;
	unsigned int L3_TM_CALIB_DIG15 = 0;
	unsigned int L3_TM_CALIB_DIG14 = 0;
	int i = 0;
	int count = 0;

	rdata = Xil_In32(0xFD40289C);
	rdata = rdata & ~0x03;
	rdata = rdata | 0x1;
	Xil_Out32(0xFD40289C, rdata);

	do {
		if (count == 1100000)
			break;
		rdata = Xil_In32(0xFD402B1C);
		count++;
	} while ((rdata & 0x0000000E) != 0x0000000E);

	for (i = 0; i < 23; i++) {
		match_pmos_code[i] = 0;
		match_nmos_code[i] = 0;
	}
	for (i = 0; i < 7; i++) {
		match_ical_code[i] = 0;
		match_rcal_code[i] = 0;
	}

	do {
		Xil_Out32(0xFD410010, 0x00000000);
		Xil_Out32(0xFD410014, 0x00000000);

		Xil_Out32(0xFD410010, 0x00000001);
		Xil_Out32(0xFD410014, 0x00000000);

		maskstatus = mask_poll(0xFD40EF14, 0x2);
		if (maskstatus == 0) {
			xil_printf("#SERDES initialization timed out\n\r");
			return maskstatus;
		}

		p_code = mask_read(0xFD40EF18, 0xFFFFFFFF);
		n_code = mask_read(0xFD40EF1C, 0xFFFFFFFF);
		;
		i_code = mask_read(0xFD40EF24, 0xFFFFFFFF);
		r_code = mask_read(0xFD40EF28, 0xFFFFFFFF);
		;

		if (p_code >= 0x26 && p_code <= 0x3C)
			match_pmos_code[p_code - 0x26] += 1;

		if (n_code >= 0x26 && n_code <= 0x3C)
			match_nmos_code[n_code - 0x26] += 1;

		if (i_code >= 0xC && i_code <= 0x12)
			match_ical_code[i_code - 0xc] += 1;

		if (r_code >= 0x6 && r_code <= 0xC)
			match_rcal_code[r_code - 0x6] += 1;

	} while (repeat_count++ < 10);

	for (i = 0; i < 23; i++) {
		if (match_pmos_code[i] >= match_pmos_code[0]) {
			match_pmos_code[0] = match_pmos_code[i];
			p_code = 0x26 + i;
		}
		if (match_nmos_code[i] >= match_nmos_code[0]) {
			match_nmos_code[0] = match_nmos_code[i];
			n_code = 0x26 + i;
		}
	}

	for (i = 0; i < 7; i++) {
		if (match_ical_code[i] >= match_ical_code[0]) {
			match_ical_code[0] = match_ical_code[i];
			i_code = 0xC + i;
		}
		if (match_rcal_code[i] >= match_rcal_code[0]) {
			match_rcal_code[0] = match_rcal_code[i];
			r_code = 0x6 + i;
		}
	}

	L3_TM_CALIB_DIG20 = mask_read(0xFD40EC50, 0xFFFFFFF0);
	L3_TM_CALIB_DIG20 = L3_TM_CALIB_DIG20 | 0x8 | ((p_code >> 2) & 0x7);

	L3_TM_CALIB_DIG19 = mask_read(0xFD40EC4C, 0xFFFFFF18);
	L3_TM_CALIB_DIG19 = L3_TM_CALIB_DIG19 | ((p_code & 0x3) << 6)
	    | 0x20 | 0x4 | ((n_code >> 3) & 0x3);

	L3_TM_CALIB_DIG18 = mask_read(0xFD40EC48, 0xFFFFFF0F);
	L3_TM_CALIB_DIG18 = L3_TM_CALIB_DIG18 | ((n_code & 0x7) << 5) | 0x10;

	L3_TM_CALIB_DIG16 = mask_read(0xFD40EC40, 0xFFFFFFF8);
	L3_TM_CALIB_DIG16 = L3_TM_CALIB_DIG16 | ((r_code >> 1) & 0x7);

	L3_TM_CALIB_DIG15 = mask_read(0xFD40EC3C, 0xFFFFFF30);
	L3_TM_CALIB_DIG15 = L3_TM_CALIB_DIG15 | ((r_code & 0x1) << 7)
	    | 0x40 | 0x8 | ((i_code >> 1) & 0x7);

	L3_TM_CALIB_DIG14 = mask_read(0xFD40EC38, 0xFFFFFF3F);
	L3_TM_CALIB_DIG14 = L3_TM_CALIB_DIG14 | ((i_code & 0x1) << 7) | 0x40;

	Xil_Out32(0xFD40EC50, L3_TM_CALIB_DIG20);
	Xil_Out32(0xFD40EC4C, L3_TM_CALIB_DIG19);
	Xil_Out32(0xFD40EC48, L3_TM_CALIB_DIG18);
	Xil_Out32(0xFD40EC40, L3_TM_CALIB_DIG16);
	Xil_Out32(0xFD40EC3C, L3_TM_CALIB_DIG15);
	Xil_Out32(0xFD40EC38, L3_TM_CALIB_DIG14);
	return maskstatus;
}

static int init_serdes(void)
{
	int status = 1;

	status &= psu_resetin_init_data();

	status &= serdes_fixcal_code();
	status &= serdes_enb_coarse_saturation();

	status &= psu_serdes_init_data();
	status &= psu_resetout_init_data();

	return status;
}

static void init_peripheral(void)
{
	psu_mask_write(0xFD5F0018, 0x8000001FU, 0x8000001FU);
}

int psu_init(void)
{
	int status = 1;

	status &= psu_mio_init_data();
	status &= psu_peripherals_pre_init_data();
	status &= psu_pll_init_data();
	status &= psu_clock_init_data();
	status &= psu_ddr_init_data();
	status &= psu_ddr_phybringup_data();
	status &= psu_peripherals_init_data();
	status &= init_serdes();
	init_peripheral();

	status &= psu_afi_config();
	psu_ddr_qos_init_data();

	if (status == 0)
		return 1;
	return 0;
}
