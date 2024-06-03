// SPDX-License-Identifier: GPL-2.0+
/*
 * MSMC controller utilities
 *
 * (C) Copyright 2012-2014
 *     Texas Instruments Incorporated, <www.ti.com>
 */

#include <common.h>
#include <asm/arch/msmc.h>

struct mpax {
	u32	mpaxl;
	u32	mpaxh;
};

struct msms_regs {
	u32	pid;
	u32	_res_04;
	u32	smcerrar;
	u32	smcerrxr;
	u32	smedcc;
	u32	smcea;
	u32	smsecc;
	u32	smpfar;
	u32	smpfxr;
	u32	smpfr;
	u32	smpfcr;
	u32	_res_2c;
	u32	sbndc[8];
	u32	sbndm;
	u32	sbnde;
	u32	_res_58;
	u32	cfglck;
	u32	cfgulck;
	u32	cfglckstat;
	u32	sms_mpax_lck;
	u32	sms_mpax_ulck;
	u32	sms_mpax_lckstat;
	u32	ses_mpax_lck;
	u32	ses_mpax_ulck;
	u32	ses_mpax_lckstat;
	u32	smestat;
	u32	smirstat;
	u32	smirc;
	u32	smiestat;
	u32	smiec;
	u32	_res_94_c0[12];
	u32	smncerrar;
	u32	smncerrxr;
	u32	smncea;
	u32	_res_d0_1fc[76];
	struct mpax sms[16][8];
	struct mpax ses[16][8];
};


void msmc_share_all_segments(int priv_id)
{
	struct msms_regs *msmc = (struct msms_regs *)KS2_MSMC_CTRL_BASE;
	int j;

	for (j = 0; j < 8; j++) {
		msmc->sms[priv_id][j].mpaxh &= 0xffffff7ful;
		msmc->ses[priv_id][j].mpaxh &= 0xffffff7ful;
	}
}

void msmc_map_ses_segment(int priv_id, int ses_pair,
			  u32 src_pfn, u32 dst_pfn, enum mpax_seg_size size)
{
	struct msms_regs *msmc = (struct msms_regs *)KS2_MSMC_CTRL_BASE;

	msmc->ses[priv_id][ses_pair].mpaxh = src_pfn << 12 |
					     (size & 0x1f) | 0x80;
	msmc->ses[priv_id][ses_pair].mpaxl = dst_pfn << 8 | 0x3f;
}

void msmc_get_ses_mpax(int priv_id, int ses_pair, u32 *mpax)
{
	struct msms_regs *msmc = (struct msms_regs *)KS2_MSMC_CTRL_BASE;

	*mpax++ = msmc->ses[priv_id][ses_pair].mpaxl;
	*mpax = msmc->ses[priv_id][ses_pair].mpaxh;
}

void msmc_set_ses_mpax(int priv_id, int ses_pair, u32 *mpax)
{
	struct msms_regs *msmc = (struct msms_regs *)KS2_MSMC_CTRL_BASE;

	msmc->ses[priv_id][ses_pair].mpaxl = *mpax++;
	msmc->ses[priv_id][ses_pair].mpaxh = *mpax;
}
