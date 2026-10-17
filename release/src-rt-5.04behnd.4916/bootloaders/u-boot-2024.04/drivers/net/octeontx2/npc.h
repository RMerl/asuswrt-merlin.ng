/* SPDX-License-Identifier:    GPL-2.0
 *
 * Copyright (C) 2018 Marvell International Ltd.
 */

#ifndef __NPC_H__
#define __NPC_H__

#define RSVD_MCAM_ENTRIES_PER_PF	2	/** Ucast and Bcast */
#define RSVD_MCAM_ENTRIES_PER_NIXLF	1	/** Ucast for VFs */

struct npc_kpu_profile_cam {
	u8 state;
	u8 state_mask;
	u16 dp0;
	u16 dp0_mask;
	u16 dp1;
	u16 dp1_mask;
	u16 dp2;
	u16 dp2_mask;
};

struct npc_kpu_profile_action {
	u8 errlev;
	u8 errcode;
	u8 dp0_offset;
	u8 dp1_offset;
	u8 dp2_offset;
	u8 bypass_count;
	u8 parse_done;
	u8 next_state;
	u8 ptr_advance;
	u8 cap_ena;
	u8 lid;
	u8 ltype;
	u8 flags;
	u8 offset;
	u8 mask;
	u8 right;
	u8 shift;
};

struct npc_kpu_profile {
	int cam_entries;
	int action_entries;
	struct npc_kpu_profile_cam *cam;
	struct npc_kpu_profile_action *action;
};

struct npc_pkind {
	struct rsrc_bmap rsrc;
	u32	*pfchan_map;
};

struct npc_mcam {
	struct rsrc_bmap rsrc;
	u16	*pfvf_map;
	u16	total_entries; /* Total number of MCAM entries */
	u16	entries;  /* Total - reserved for NIX LFs */
	u8	banks_per_entry;  /* Number of keywords in key */
	u8	keysize;
	u8	banks;    /* Number of MCAM banks */
	u16	banksize; /* Number of MCAM entries in each bank */
	u16	counters; /* Number of match counters */
	u16	nixlf_offset;
	u16	pf_offset;
};

struct nix_af_handle;
struct nix_handle;
struct rvu_hwinfo;

struct npc_af {
	struct nix_af_handle	*nix_af;
	struct npc_pkind	pkind;
	void __iomem		*npc_af_base;
	u8			npc_kpus;	/** Number of parser units */
	struct npc_mcam		mcam;
	struct rvu_block	block;
	struct rvu_hwinfo	*hw;
};

struct npc {
	struct npc_af		*npc_af;
	void __iomem		*npc_base;
	struct nix_handle	*nix;
}

#endif /* __NPC_H__ */
