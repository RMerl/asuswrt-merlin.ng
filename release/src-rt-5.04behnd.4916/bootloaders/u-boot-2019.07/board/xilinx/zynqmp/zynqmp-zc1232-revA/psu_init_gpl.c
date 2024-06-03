// SPDX-License-Identifier: GPL-2.0+
/*
 * (c) Copyright 2015 Xilinx, Inc. All rights reserved.
 */

#include <asm/arch/psu_init_gpl.h>
#include <xil_io.h>

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
	psu_mask_write(0xFF5E0038, 0x8000FFFFU, 0x8000C76CU);
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
	psu_mask_write(0xFD1A0040, 0x8000FFFFU, 0x8000497FU);

	return 1;
}

static unsigned long psu_clock_init_data(void)
{
	psu_mask_write(0xFF5E0068, 0x013F3F07U, 0x01010C00U);
	psu_mask_write(0xFF5E0074, 0x013F3F07U, 0x01010F00U);
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
	psu_mask_write(0xFD1A00A0, 0x01003F07U, 0x01000200U);
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
	psu_mask_write(0xFD070000, 0xE30FBE3DU, 0x81040001U);
	psu_mask_write(0xFD070010, 0x8000F03FU, 0x00000030U);
	psu_mask_write(0xFD070020, 0x000003F3U, 0x00000200U);
	psu_mask_write(0xFD070024, 0xFFFFFFFFU, 0x00800000U);
	psu_mask_write(0xFD070030, 0x0000007FU, 0x00000000U);
	psu_mask_write(0xFD070034, 0x00FFFF1FU, 0x00408410U);
	psu_mask_write(0xFD070050, 0x00F1F1F4U, 0x00210000U);
	psu_mask_write(0xFD070054, 0x0FFF0FFFU, 0x00000000U);
	psu_mask_write(0xFD070060, 0x00000073U, 0x00000001U);
	psu_mask_write(0xFD070064, 0x0FFF83FFU, 0x0081808BU);
	psu_mask_write(0xFD070070, 0x00000017U, 0x00000010U);
	psu_mask_write(0xFD070074, 0x00000003U, 0x00000000U);
	psu_mask_write(0xFD0700C4, 0x3F000391U, 0x10000200U);
	psu_mask_write(0xFD0700C8, 0x01FF1F3FU, 0x0030051FU);
	psu_mask_write(0xFD0700D0, 0xC3FF0FFFU, 0x00020106U);
	psu_mask_write(0xFD0700D4, 0x01FF7F0FU, 0x006A0000U);
	psu_mask_write(0xFD0700D8, 0x0000FF0FU, 0x00002305U);
	psu_mask_write(0xFD0700DC, 0xFFFFFFFFU, 0x01240004U);
	psu_mask_write(0xFD0700E0, 0xFFFFFFFFU, 0x00280000U);
	psu_mask_write(0xFD0700E4, 0x00FF03FFU, 0x00110004U);
	psu_mask_write(0xFD0700E8, 0xFFFFFFFFU, 0x00000000U);
	psu_mask_write(0xFD0700EC, 0xFFFF0000U, 0x00000000U);
	psu_mask_write(0xFD0700F0, 0x0000003FU, 0x00000010U);
	psu_mask_write(0xFD0700F4, 0x00000FFFU, 0x0000066FU);
	psu_mask_write(0xFD070100, 0x7F3F7F3FU, 0x0F0E2412U);
	psu_mask_write(0xFD070104, 0x001F1F7FU, 0x000D0419U);
	psu_mask_write(0xFD070108, 0x3F3F3F3FU, 0x0507070BU);
	psu_mask_write(0xFD07010C, 0x3FF3F3FFU, 0x00502008U);
	psu_mask_write(0xFD070110, 0x1F0F0F1FU, 0x07020408U);
	psu_mask_write(0xFD070114, 0x0F0F3F1FU, 0x06060403U);
	psu_mask_write(0xFD070118, 0x0F0F000FU, 0x01010004U);
	psu_mask_write(0xFD07011C, 0x00000F0FU, 0x00000606U);
	psu_mask_write(0xFD070120, 0x7F7F7F7FU, 0x03030909U);
	psu_mask_write(0xFD070124, 0x40070F3FU, 0x0004040DU);
	psu_mask_write(0xFD07012C, 0x7F1F031FU, 0x440C011CU);
	psu_mask_write(0xFD070130, 0x00030F1FU, 0x00020608U);
	psu_mask_write(0xFD070180, 0xF7FF03FFU, 0x80AB002BU);
	psu_mask_write(0xFD070184, 0x3FFFFFFFU, 0x020196E5U);
	psu_mask_write(0xFD070190, 0x1FBFBF3FU, 0x048A8207U);
	psu_mask_write(0xFD070194, 0xF31F0F0FU, 0x00020304U);
	psu_mask_write(0xFD070198, 0x0FF1F1F1U, 0x07000101U);
	psu_mask_write(0xFD07019C, 0x000000F1U, 0x00000021U);
	psu_mask_write(0xFD0701A0, 0xC3FF03FFU, 0x00400003U);
	psu_mask_write(0xFD0701A4, 0x00FF00FFU, 0x004100E1U);
	psu_mask_write(0xFD0701B0, 0x00000007U, 0x00000000U);
	psu_mask_write(0xFD0701B4, 0x00003F3FU, 0x00000802U);
	psu_mask_write(0xFD0701C0, 0x00000007U, 0x00000000U);
	psu_mask_write(0xFD070200, 0x0000001FU, 0x0000001FU);
	psu_mask_write(0xFD070204, 0x001F1F1FU, 0x00080808U);
	psu_mask_write(0xFD070208, 0x0F0F0F0FU, 0x00000000U);
	psu_mask_write(0xFD07020C, 0x0F0F0F0FU, 0x00000000U);
	psu_mask_write(0xFD070210, 0x00000F0FU, 0x00000F0FU);
	psu_mask_write(0xFD070214, 0x0F0F0F0FU, 0x070F0707U);
	psu_mask_write(0xFD070218, 0x8F0F0F0FU, 0x0F070707U);
	psu_mask_write(0xFD07021C, 0x00000F0FU, 0x00000F0FU);
	psu_mask_write(0xFD070220, 0x00001F1FU, 0x00000000U);
	psu_mask_write(0xFD070224, 0x0F0F0F0FU, 0x07070707U);
	psu_mask_write(0xFD070228, 0x0F0F0F0FU, 0x07070707U);
	psu_mask_write(0xFD07022C, 0x0000000FU, 0x00000007U);
	psu_mask_write(0xFD070240, 0x0F1F0F7CU, 0x06000610U);
	psu_mask_write(0xFD070244, 0x00003333U, 0x00000001U);
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
	psu_mask_write(0xFD080010, 0xFFFFFFFFU, 0x07001E00U);
	psu_mask_write(0xFD080018, 0xFFFFFFFFU, 0x00F10010U);
	psu_mask_write(0xFD08001C, 0xFFFFFFFFU, 0x55AA5480U);
	psu_mask_write(0xFD080024, 0xFFFFFFFFU, 0x010100F4U);
	psu_mask_write(0xFD080040, 0xFFFFFFFFU, 0x0AC85590U);
	psu_mask_write(0xFD080044, 0xFFFFFFFFU, 0x41540B00U);
	psu_mask_write(0xFD080068, 0xFFFFFFFFU, 0x01100000U);
	psu_mask_write(0xFD080090, 0xFFFFFFFFU, 0x02A04161U);
	psu_mask_write(0xFD080100, 0xFFFFFFFFU, 0x0800040BU);
	psu_mask_write(0xFD080110, 0xFFFFFFFFU, 0x08240E08U);
	psu_mask_write(0xFD080114, 0xFFFFFFFFU, 0x281C0404U);
	psu_mask_write(0xFD080118, 0xFFFFFFFFU, 0x00070200U);
	psu_mask_write(0xFD08011C, 0xFFFFFFFFU, 0x82000800U);
	psu_mask_write(0xFD080120, 0xFFFFFFFFU, 0x01162B1AU);
	psu_mask_write(0xFD080124, 0xFFFFFFFFU, 0x00320E08U);
	psu_mask_write(0xFD080128, 0xFFFFFFFFU, 0x00000A0EU);
	psu_mask_write(0xFD080140, 0xFFFFFFFFU, 0x08400020U);
	psu_mask_write(0xFD080144, 0xFFFFFFFFU, 0x00000C80U);
	psu_mask_write(0xFD080150, 0xFFFFFFFFU, 0x00000000U);
	psu_mask_write(0xFD080154, 0xFFFFFFFFU, 0x00000000U);
	psu_mask_write(0xFD080180, 0xFFFFFFFFU, 0x00000124U);
	psu_mask_write(0xFD080184, 0xFFFFFFFFU, 0x00000004U);
	psu_mask_write(0xFD080188, 0xFFFFFFFFU, 0x00000028U);
	psu_mask_write(0xFD08018C, 0xFFFFFFFFU, 0x00000000U);
	psu_mask_write(0xFD080190, 0xFFFFFFFFU, 0x00000000U);
	psu_mask_write(0xFD080194, 0xFFFFFFFFU, 0x00000000U);
	psu_mask_write(0xFD080198, 0xFFFFFFFFU, 0x00000000U);
	psu_mask_write(0xFD0801AC, 0xFFFFFFFFU, 0x00000000U);
	psu_mask_write(0xFD0801B0, 0xFFFFFFFFU, 0x0000004DU);
	psu_mask_write(0xFD0801B4, 0xFFFFFFFFU, 0x00000008U);
	psu_mask_write(0xFD0801B8, 0xFFFFFFFFU, 0x0000004DU);
	psu_mask_write(0xFD0801D8, 0xFFFFFFFFU, 0x00000000U);
	psu_mask_write(0xFD080200, 0xFFFFFFFFU, 0x800081C7U);
	psu_mask_write(0xFD080204, 0xFFFFFFFFU, 0x00010236U);
	psu_mask_write(0xFD080240, 0xFFFFFFFFU, 0x00141054U);
	psu_mask_write(0xFD080250, 0xFFFFFFFFU, 0x00088000U);
	psu_mask_write(0xFD080414, 0xFFFFFFFFU, 0x12340800U);
	psu_mask_write(0xFD0804F4, 0xFFFFFFFFU, 0x00000005U);
	psu_mask_write(0xFD080500, 0xFFFFFFFFU, 0x30000028U);
	psu_mask_write(0xFD080508, 0xFFFFFFFFU, 0x0A000000U);
	psu_mask_write(0xFD08050C, 0xFFFFFFFFU, 0x00000009U);
	psu_mask_write(0xFD080510, 0xFFFFFFFFU, 0x0A000000U);
	psu_mask_write(0xFD080520, 0xFFFFFFFFU, 0x0300B0B0U);
	psu_mask_write(0xFD080528, 0xFFFFFFFFU, 0xF1032019U);
	psu_mask_write(0xFD08052C, 0xFFFFFFFFU, 0x07F001E3U);
	psu_mask_write(0xFD080544, 0xFFFFFFFFU, 0x00000000U);
	psu_mask_write(0xFD080548, 0xFFFFFFFFU, 0x00000000U);
	psu_mask_write(0xFD080558, 0xFFFFFFFFU, 0x00000000U);
	psu_mask_write(0xFD08055C, 0xFFFFFFFFU, 0x00000000U);
	psu_mask_write(0xFD080560, 0xFFFFFFFFU, 0x00000000U);
	psu_mask_write(0xFD080564, 0xFFFFFFFFU, 0x00000000U);
	psu_mask_write(0xFD080680, 0xFFFFFFFFU, 0x008AA858U);
	psu_mask_write(0xFD080684, 0xFFFFFFFFU, 0x000077BBU);
	psu_mask_write(0xFD080694, 0xFFFFFFFFU, 0x01E10210U);
	psu_mask_write(0xFD080698, 0xFFFFFFFFU, 0x01E10000U);
	psu_mask_write(0xFD0806A4, 0xFFFFFFFFU, 0x000076BBU);
	psu_mask_write(0xFD080700, 0xFFFFFFFFU, 0x40800604U);
	psu_mask_write(0xFD080710, 0xFFFFFFFFU, 0x0E00B00CU);
	psu_mask_write(0xFD080714, 0xFFFFFFFFU, 0x09093030U);
	psu_mask_write(0xFD080718, 0xFFFFFFFFU, 0x09092B2BU);
	psu_mask_write(0xFD080800, 0xFFFFFFFFU, 0x40800604U);
	psu_mask_write(0xFD080810, 0xFFFFFFFFU, 0x0E00B00CU);
	psu_mask_write(0xFD080814, 0xFFFFFFFFU, 0x09093030U);
	psu_mask_write(0xFD080818, 0xFFFFFFFFU, 0x09092B2BU);
	psu_mask_write(0xFD080900, 0xFFFFFFFFU, 0x40800604U);
	psu_mask_write(0xFD080904, 0xFFFFFFFFU, 0x00007FFFU);
	psu_mask_write(0xFD080910, 0xFFFFFFFFU, 0x0E00B00CU);
	psu_mask_write(0xFD080914, 0xFFFFFFFFU, 0x09093030U);
	psu_mask_write(0xFD080918, 0xFFFFFFFFU, 0x09092B2BU);
	psu_mask_write(0xFD080A00, 0xFFFFFFFFU, 0x40800604U);
	psu_mask_write(0xFD080A04, 0xFFFFFFFFU, 0x00007FFFU);
	psu_mask_write(0xFD080A10, 0xFFFFFFFFU, 0x0E00B00CU);
	psu_mask_write(0xFD080A14, 0xFFFFFFFFU, 0x09093030U);
	psu_mask_write(0xFD080A18, 0xFFFFFFFFU, 0x09092B2BU);
	psu_mask_write(0xFD080B00, 0xFFFFFFFFU, 0x40800604U);
	psu_mask_write(0xFD080B04, 0xFFFFFFFFU, 0x00007FFFU);
	psu_mask_write(0xFD080B10, 0xFFFFFFFFU, 0x0E00B00CU);
	psu_mask_write(0xFD080B14, 0xFFFFFFFFU, 0x09093030U);
	psu_mask_write(0xFD080B18, 0xFFFFFFFFU, 0x09092B2BU);
	psu_mask_write(0xFD080C00, 0xFFFFFFFFU, 0x40800604U);
	psu_mask_write(0xFD080C04, 0xFFFFFFFFU, 0x00007FFFU);
	psu_mask_write(0xFD080C10, 0xFFFFFFFFU, 0x0E00B00CU);
	psu_mask_write(0xFD080C14, 0xFFFFFFFFU, 0x09093030U);
	psu_mask_write(0xFD080C18, 0xFFFFFFFFU, 0x09092B2BU);
	psu_mask_write(0xFD080D00, 0xFFFFFFFFU, 0x40800604U);
	psu_mask_write(0xFD080D04, 0xFFFFFFFFU, 0x00007FFFU);
	psu_mask_write(0xFD080D10, 0xFFFFFFFFU, 0x0E00B00CU);
	psu_mask_write(0xFD080D14, 0xFFFFFFFFU, 0x09093030U);
	psu_mask_write(0xFD080D18, 0xFFFFFFFFU, 0x09092B2BU);
	psu_mask_write(0xFD080E00, 0xFFFFFFFFU, 0x40800604U);
	psu_mask_write(0xFD080E04, 0xFFFFFFFFU, 0x00007FFFU);
	psu_mask_write(0xFD080E10, 0xFFFFFFFFU, 0x0E00B00CU);
	psu_mask_write(0xFD080E14, 0xFFFFFFFFU, 0x09093030U);
	psu_mask_write(0xFD080E18, 0xFFFFFFFFU, 0x09092B2BU);
	psu_mask_write(0xFD080F00, 0xFFFFFFFFU, 0x40800624U);
	psu_mask_write(0xFD080F04, 0xFFFFFFFFU, 0x00007F00U);
	psu_mask_write(0xFD080F10, 0xFFFFFFFFU, 0x0E00B00CU);
	psu_mask_write(0xFD080F14, 0xFFFFFFFFU, 0x09093030U);
	psu_mask_write(0xFD080F18, 0xFFFFFFFFU, 0x09092B2BU);
	psu_mask_write(0xFD081400, 0xFFFFFFFFU, 0x2A019FFEU);
	psu_mask_write(0xFD081404, 0xFFFFFFFFU, 0x01100000U);
	psu_mask_write(0xFD08141C, 0xFFFFFFFFU, 0x01264300U);
	psu_mask_write(0xFD08142C, 0xFFFFFFFFU, 0x00041800U);
	psu_mask_write(0xFD081430, 0xFFFFFFFFU, 0x70000000U);
	psu_mask_write(0xFD081440, 0xFFFFFFFFU, 0x2A019FFEU);
	psu_mask_write(0xFD081444, 0xFFFFFFFFU, 0x01100000U);
	psu_mask_write(0xFD08145C, 0xFFFFFFFFU, 0x01264300U);
	psu_mask_write(0xFD08146C, 0xFFFFFFFFU, 0x00041800U);
	psu_mask_write(0xFD081470, 0xFFFFFFFFU, 0x70000000U);
	psu_mask_write(0xFD081480, 0xFFFFFFFFU, 0x2A019FFEU);
	psu_mask_write(0xFD081484, 0xFFFFFFFFU, 0x01100000U);
	psu_mask_write(0xFD08149C, 0xFFFFFFFFU, 0x01264300U);
	psu_mask_write(0xFD0814AC, 0xFFFFFFFFU, 0x00041800U);
	psu_mask_write(0xFD0814B0, 0xFFFFFFFFU, 0x70000000U);
	psu_mask_write(0xFD0814C0, 0xFFFFFFFFU, 0x2A019FFEU);
	psu_mask_write(0xFD0814C4, 0xFFFFFFFFU, 0x01100000U);
	psu_mask_write(0xFD0814DC, 0xFFFFFFFFU, 0x01264300U);
	psu_mask_write(0xFD0814EC, 0xFFFFFFFFU, 0x00041800U);
	psu_mask_write(0xFD0814F0, 0xFFFFFFFFU, 0x70000000U);
	psu_mask_write(0xFD081500, 0xFFFFFFFFU, 0x2A019FFEU);
	psu_mask_write(0xFD081504, 0xFFFFFFFFU, 0x01100000U);
	psu_mask_write(0xFD08151C, 0xFFFFFFFFU, 0x01264300U);
	psu_mask_write(0xFD08152C, 0xFFFFFFFFU, 0x00041800U);
	psu_mask_write(0xFD081530, 0xFFFFFFFFU, 0x70000000U);
	psu_mask_write(0xFD0817C4, 0xFFFFFFFFU, 0x01100000U);
	psu_mask_write(0xFD0817DC, 0xFFFFFFFFU, 0x012643C4U);
	psu_mask_write(0xFD080004, 0xFFFFFFFFU, 0x00040073U);

	return 1;
}

static unsigned long psu_mio_init_data(void)
{
	psu_mask_write(0xFF180000, 0x000000FEU, 0x00000002U);
	psu_mask_write(0xFF180004, 0x000000FEU, 0x00000002U);
	psu_mask_write(0xFF180008, 0x000000FEU, 0x00000002U);
	psu_mask_write(0xFF18000C, 0x000000FEU, 0x00000002U);
	psu_mask_write(0xFF180010, 0x000000FEU, 0x00000002U);
	psu_mask_write(0xFF180014, 0x000000FEU, 0x00000002U);
	psu_mask_write(0xFF180088, 0x000000FEU, 0x000000C0U);
	psu_mask_write(0xFF18008C, 0x000000FEU, 0x000000C0U);
	psu_mask_write(0xFF180204, 0x0000003FU, 0x00000000U);
	psu_mask_write(0xFF180208, 0x0000000CU, 0x00000004U);
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

static unsigned long psu_peripherals_init_data(void)
{
	psu_mask_write(0xFD1A0100, 0x0000807EU, 0x00000000U);
	psu_mask_write(0xFF5E0238, 0x001A0000U, 0x00000000U);
	psu_mask_write(0xFF5E023C, 0x0093C018U, 0x00000000U);
	psu_mask_write(0xFF5E0238, 0x00000001U, 0x00000000U);
	psu_mask_write(0xFF180390, 0x00000004U, 0x00000004U);
	psu_mask_write(0xFF5E0238, 0x00008000U, 0x00000000U);
	psu_mask_write(0xFF5E0238, 0x00007800U, 0x00000000U);
	psu_mask_write(0xFF5E0238, 0x00000002U, 0x00000000U);
	psu_mask_write(0xFF000034, 0x000000FFU, 0x00000005U);
	psu_mask_write(0xFF000018, 0x0000FFFFU, 0x0000008FU);
	psu_mask_write(0xFF000000, 0x000001FFU, 0x00000017U);
	psu_mask_write(0xFF000004, 0x000003FFU, 0x00000020U);
	psu_mask_write(0xFF5E0238, 0x00040000U, 0x00000000U);
	psu_mask_write(0xFF4B0024, 0x000000FFU, 0x000000FFU);
	psu_mask_write(0xFFCA5000, 0x00001FFFU, 0x00000000U);
	psu_mask_write(0xFD5C0060, 0x000F000FU, 0x00000000U);
	psu_mask_write(0xFFA60040, 0x80000000U, 0x80000000U);
	psu_mask_write(0xFF260020, 0xFFFFFFFFU, 0x05F5DD17U);
	psu_mask_write(0xFF260000, 0x00000001U, 0x00000001U);

	return 1;
}

static unsigned long psu_serdes_init_data(void)
{
	psu_mask_write(0xFD410000, 0x0000001FU, 0x0000000FU);
	psu_mask_write(0xFD410004, 0x0000001FU, 0x0000000FU);
	psu_mask_write(0xFD402860, 0x00000080U, 0x00000080U);
	psu_mask_write(0xFD402864, 0x00000081U, 0x00000001U);
	psu_mask_write(0xFD402368, 0x000000FFU, 0x000000E0U);
	psu_mask_write(0xFD40236C, 0x00000007U, 0x00000003U);
	psu_mask_write(0xFD406368, 0x000000FFU, 0x000000E0U);
	psu_mask_write(0xFD40636C, 0x00000007U, 0x00000003U);
	psu_mask_write(0xFD402370, 0x000000FFU, 0x000000C9U);
	psu_mask_write(0xFD402374, 0x000000FFU, 0x000000D2U);
	psu_mask_write(0xFD402378, 0x000000FFU, 0x00000001U);
	psu_mask_write(0xFD40237C, 0x000000B3U, 0x000000B0U);
	psu_mask_write(0xFD406370, 0x000000FFU, 0x000000C9U);
	psu_mask_write(0xFD406374, 0x000000FFU, 0x000000D2U);
	psu_mask_write(0xFD406378, 0x000000FFU, 0x00000001U);
	psu_mask_write(0xFD40637C, 0x000000B3U, 0x000000B0U);
	psu_mask_write(0xFD402360, 0x00000040U, 0x00000040U);
	psu_mask_write(0xFD40106C, 0x0000000FU, 0x0000000FU);
	psu_mask_write(0xFD4000F4, 0x0000000BU, 0x0000000BU);
	psu_mask_write(0xFD406360, 0x00000040U, 0x00000040U);
	psu_mask_write(0xFD40506C, 0x0000000FU, 0x0000000FU);
	psu_mask_write(0xFD4040F4, 0x0000000BU, 0x0000000BU);
	psu_mask_write(0xFD401074, 0x00000010U, 0x00000010U);
	psu_mask_write(0xFD405074, 0x00000010U, 0x00000010U);
	psu_mask_write(0xFD409074, 0x00000010U, 0x00000010U);
	psu_mask_write(0xFD40D074, 0x00000010U, 0x00000010U);
	psu_mask_write(0xFD40189C, 0x00000080U, 0x00000080U);
	psu_mask_write(0xFD4018F8, 0x000000FFU, 0x0000007DU);
	psu_mask_write(0xFD4018FC, 0x000000FFU, 0x0000007DU);
	psu_mask_write(0xFD401990, 0x000000FFU, 0x00000001U);
	psu_mask_write(0xFD401924, 0x000000FFU, 0x0000009CU);
	psu_mask_write(0xFD401928, 0x000000FFU, 0x00000039U);
	psu_mask_write(0xFD40198C, 0x000000F0U, 0x00000020U);
	psu_mask_write(0xFD401900, 0x000000FFU, 0x0000007DU);
	psu_mask_write(0xFD40192C, 0x000000FFU, 0x00000064U);
	psu_mask_write(0xFD401980, 0x000000FFU, 0x000000FFU);
	psu_mask_write(0xFD401914, 0x000000FFU, 0x000000F7U);
	psu_mask_write(0xFD401918, 0x00000001U, 0x00000001U);
	psu_mask_write(0xFD401940, 0x000000FFU, 0x000000F7U);
	psu_mask_write(0xFD401944, 0x00000001U, 0x00000001U);
	psu_mask_write(0xFD401994, 0x00000007U, 0x00000007U);
	psu_mask_write(0xFD40589C, 0x00000080U, 0x00000080U);
	psu_mask_write(0xFD4058F8, 0x000000FFU, 0x0000007DU);
	psu_mask_write(0xFD4058FC, 0x000000FFU, 0x0000007DU);
	psu_mask_write(0xFD405990, 0x000000FFU, 0x00000001U);
	psu_mask_write(0xFD405924, 0x000000FFU, 0x0000009CU);
	psu_mask_write(0xFD405928, 0x000000FFU, 0x00000039U);
	psu_mask_write(0xFD40598C, 0x000000F0U, 0x00000020U);
	psu_mask_write(0xFD405900, 0x000000FFU, 0x0000007DU);
	psu_mask_write(0xFD40592C, 0x000000FFU, 0x00000064U);
	psu_mask_write(0xFD405980, 0x000000FFU, 0x000000FFU);
	psu_mask_write(0xFD405914, 0x000000FFU, 0x000000F7U);
	psu_mask_write(0xFD405918, 0x00000001U, 0x00000001U);
	psu_mask_write(0xFD405940, 0x000000FFU, 0x000000F7U);
	psu_mask_write(0xFD405944, 0x00000001U, 0x00000001U);
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
	psu_mask_write(0xFD410010, 0x00000077U, 0x00000022U);
	psu_mask_write(0xFD4001D8, 0x00000001U, 0x00000001U);
	psu_mask_write(0xFD4041D8, 0x00000001U, 0x00000001U);
	psu_mask_write(0xFD401C14, 0x000000FFU, 0x000000E6U);
	psu_mask_write(0xFD401C40, 0x0000001FU, 0x0000000CU);
	psu_mask_write(0xFD40194C, 0x00000020U, 0x00000020U);
	psu_mask_write(0xFD401950, 0x00000007U, 0x00000006U);
	psu_mask_write(0xFD405C14, 0x000000FFU, 0x000000E6U);
	psu_mask_write(0xFD405C40, 0x0000001FU, 0x0000000CU);
	psu_mask_write(0xFD40594C, 0x00000020U, 0x00000020U);
	psu_mask_write(0xFD405950, 0x00000007U, 0x00000006U);
	psu_mask_write(0xFD404048, 0x000000FFU, 0x00000001U);
	psu_mask_write(0xFD400048, 0x000000FFU, 0x00000001U);

	return 1;
}

static unsigned long psu_resetout_init_data(void)
{
	psu_mask_write(0xFD3D0100, 0x00000003U, 0x00000001U);
	psu_mask_write(0xFD1A0100, 0x00000002U, 0x00000000U);
	psu_mask_write(0xFD480064, 0x00000200U, 0x00000200U);
	mask_poll(0xFD4023E4, 0x00000010U);
	mask_poll(0xFD4063E4, 0x00000010U);
	psu_mask_write(0xFD0C00AC, 0xFFFFFFFFU, 0x28184018U);
	psu_mask_write(0xFD0C00B0, 0xFFFFFFFFU, 0x0E081406U);
	psu_mask_write(0xFD0C00B4, 0xFFFFFFFFU, 0x064A0813U);
	psu_mask_write(0xFD0C00B8, 0xFFFFFFFFU, 0x3FFC96A4U);

	return 1;
}

static unsigned long psu_resetin_init_data(void)
{
	psu_mask_write(0xFD1A0100, 0x00000002U, 0x00000002U);

	return 1;
}

static unsigned long psu_afi_config(void)
{
	psu_mask_write(0xFD1A0100, 0x00001F80U, 0x00000000U);
	psu_mask_write(0xFF5E023C, 0x00080000U, 0x00000000U);
	psu_mask_write(0xFD615000, 0x00000300U, 0x00000200U);
	psu_mask_write(0xFD360000, 0x00000003U, 0x00000000U);
	psu_mask_write(0xFD370000, 0x00000003U, 0x00000000U);
	psu_mask_write(0xFD390000, 0x00000003U, 0x00000000U);
	psu_mask_write(0xFD3A0000, 0x00000003U, 0x00000000U);
	psu_mask_write(0xFF9B0000, 0x00000003U, 0x00000000U);
	psu_mask_write(0xFD360014, 0x00000003U, 0x00000000U);
	psu_mask_write(0xFD370014, 0x00000003U, 0x00000000U);
	psu_mask_write(0xFD390014, 0x00000003U, 0x00000000U);
	psu_mask_write(0xFD3A0014, 0x00000003U, 0x00000000U);
	psu_mask_write(0xFF9B0014, 0x00000003U, 0x00000000U);

	return 1;
}

static unsigned long psu_ddr_phybringup_data(void)
{
	unsigned int regval = 0;

	while ((Xil_In32(0xFD080030U) & 0x0000000FU) != 0x0000000FU)
		;
	prog_reg(0xFD080004U, 0x00000001U, 0x00000000U, 0x00000001U);

	while ((Xil_In32(0xFD080030U) & 0x000000FFU) != 0x0000001FU)
		;
	Xil_Out32(0xFD0701B0U, 0x00000001U);
	Xil_Out32(0xFD070320U, 0x00000001U);
	while ((Xil_In32(0xFD070004U) & 0x0000000FU) != 0x00000001U)
		;
	prog_reg(0xFD080014U, 0x00000040U, 0x00000006U, 0x00000001U);
	Xil_Out32(0xFD080004, 0x0004FE01);
	regval = Xil_In32(0xFD080030);
	while (regval != 0x80000FFF)
		regval = Xil_In32(0xFD080030);
	Xil_Out32(0xFD070180U, 0x00AB002BU);
	Xil_Out32(0xFD070060U, 0x00000000U);
	prog_reg(0xFD080014U, 0x00000040U, 0x00000006U, 0x00000000U);

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
			/* xil_printf("#SERDES initialization timed out\n\r");*/
			return maskstatus;
		}

		p_code = mask_read(0xFD40EF18, 0xFFFFFFFF);
		n_code = mask_read(0xFD40EF1C, 0xFFFFFFFF);

		i_code = mask_read(0xFD40EF24, 0xFFFFFFFF);
		r_code = mask_read(0xFD40EF28, 0xFFFFFFFF);

		if ((p_code >= 0x26) && (p_code <= 0x3C))
			match_pmos_code[p_code - 0x26] += 1;

		if ((n_code >= 0x26) && (n_code <= 0x3C))
			match_nmos_code[n_code - 0x26] += 1;

		if ((i_code >= 0xC) && (i_code <= 0x12))
			match_ical_code[i_code - 0xC] += 1;

		if ((r_code >= 0x6) && (r_code <= 0xC))
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
	L3_TM_CALIB_DIG19 =
		L3_TM_CALIB_DIG19 | ((p_code & 0x3) << 6) | 0x20 | 0x4 |
		((n_code >> 3) & 0x3);

	L3_TM_CALIB_DIG18 = mask_read(0xFD40EC48, 0xFFFFFF0F);
	L3_TM_CALIB_DIG18 = L3_TM_CALIB_DIG18 | ((n_code & 0x7) << 5) | 0x10;

	L3_TM_CALIB_DIG16 = mask_read(0xFD40EC40, 0xFFFFFFF8);
	L3_TM_CALIB_DIG16 = L3_TM_CALIB_DIG16 | ((r_code >> 1) & 0x7);

	L3_TM_CALIB_DIG15 = mask_read(0xFD40EC3C, 0xFFFFFF30);
	L3_TM_CALIB_DIG15 =
		L3_TM_CALIB_DIG15 | ((r_code & 0x1) << 7) | 0x40 | 0x8 |
		((i_code >> 1) & 0x7);

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
	unsigned int tmp_regval;

	tmp_regval = Xil_In32(0xFD690040);
	tmp_regval &= ~0x00000001;
	Xil_Out32(0xFD690040, tmp_regval);

	tmp_regval = Xil_In32(0xFD690030);
	tmp_regval &= ~0x00000001;
	Xil_Out32(0xFD690030, tmp_regval);
}

int psu_init(void)
{
	int status = 1;

	status &= psu_mio_init_data();
	status &= psu_pll_init_data();
	status &= psu_clock_init_data();
	status &= psu_ddr_init_data();
	status &= psu_ddr_phybringup_data();
	status &= psu_peripherals_init_data();
	status &= init_serdes();
	init_peripheral();

	status &= psu_afi_config();

	if (status == 0)
		return 1;
	return 0;
}
