// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2018-2022 Marvell International Ltd.
 *
 * PKI Support.
 */

#include <time.h>
#include <log.h>
#include <linux/delay.h>

#include <mach/cvmx-regs.h>
#include <mach/cvmx-csr.h>
#include <mach/cvmx-bootmem.h>
#include <mach/octeon-model.h>
#include <mach/cvmx-fuse.h>
#include <mach/octeon-feature.h>
#include <mach/cvmx-qlm.h>
#include <mach/octeon_qlm.h>
#include <mach/cvmx-pcie.h>
#include <mach/cvmx-coremask.h>

#include <mach/cvmx-agl-defs.h>
#include <mach/cvmx-bgxx-defs.h>
#include <mach/cvmx-ciu-defs.h>
#include <mach/cvmx-gmxx-defs.h>
#include <mach/cvmx-gserx-defs.h>
#include <mach/cvmx-ilk-defs.h>
#include <mach/cvmx-ipd-defs.h>
#include <mach/cvmx-pcsx-defs.h>
#include <mach/cvmx-pcsxx-defs.h>
#include <mach/cvmx-pki-defs.h>
#include <mach/cvmx-pko-defs.h>
#include <mach/cvmx-xcv-defs.h>

#include <mach/cvmx-hwpko.h>
#include <mach/cvmx-ilk.h>
#include <mach/cvmx-pki.h>
#include <mach/cvmx-pki-cluster.h>
#include <mach/cvmx-pki-resources.h>

#include <mach/cvmx-helper.h>
#include <mach/cvmx-helper-board.h>
#include <mach/cvmx-helper-cfg.h>

/**
 * This function enables PKI
 *
 * @param node  Node to enable PKI.
 */
void cvmx_pki_enable(int node)
{
	cvmx_pki_sft_rst_t sft_rst;
	cvmx_pki_buf_ctl_t buf_ctl;

	sft_rst.u64 = csr_rd_node(node, CVMX_PKI_SFT_RST);
	while (sft_rst.s.busy != 0)
		sft_rst.u64 = csr_rd_node(node, CVMX_PKI_SFT_RST);

	buf_ctl.u64 = csr_rd_node(node, CVMX_PKI_BUF_CTL);
	if (buf_ctl.s.pki_en)
		debug("Warning: Enabling PKI when PKI already enabled.\n");

	buf_ctl.s.pki_en = 1;
	csr_wr_node(node, CVMX_PKI_BUF_CTL, buf_ctl.u64);
}

/**
 * This function sets the clusters in PKI.
 *
 * @param node  Node to set clusters.
 */
int cvmx_pki_setup_clusters(int node)
{
	int i;

	for (i = 0; i < cvmx_pki_cluster_code_length; i++)
		csr_wr_node(node, CVMX_PKI_IMEMX(i),
			    cvmx_pki_cluster_code_default[i]);

	return 0;
}

/**
 * This function reads global configuration of PKI block.
 *
 * @param node  Node number.
 * @param gbl_cfg  Pointer to struct to read global configuration.
 */
void cvmx_pki_read_global_config(int node,
				 struct cvmx_pki_global_config *gbl_cfg)
{
	cvmx_pki_stat_ctl_t stat_ctl;
	cvmx_pki_icgx_cfg_t icg_cfg;
	cvmx_pki_gbl_pen_t gbl_pen;
	cvmx_pki_tag_secret_t tag_secret;
	cvmx_pki_frm_len_chkx_t frm_len_chk;
	cvmx_pki_buf_ctl_t buf_ctl;
	unsigned int cl_grp;
	int id;

	stat_ctl.u64 = csr_rd_node(node, CVMX_PKI_STAT_CTL);
	gbl_cfg->stat_mode = stat_ctl.s.mode;

	for (cl_grp = 0; cl_grp < CVMX_PKI_NUM_CLUSTER_GROUP; cl_grp++) {
		icg_cfg.u64 = csr_rd_node(node, CVMX_PKI_ICGX_CFG(cl_grp));
		gbl_cfg->cluster_mask[cl_grp] = icg_cfg.s.clusters;
	}
	gbl_pen.u64 = csr_rd_node(node, CVMX_PKI_GBL_PEN);
	gbl_cfg->gbl_pen.virt_pen = gbl_pen.s.virt_pen;
	gbl_cfg->gbl_pen.clg_pen = gbl_pen.s.clg_pen;
	gbl_cfg->gbl_pen.cl2_pen = gbl_pen.s.cl2_pen;
	gbl_cfg->gbl_pen.l4_pen = gbl_pen.s.l4_pen;
	gbl_cfg->gbl_pen.il3_pen = gbl_pen.s.il3_pen;
	gbl_cfg->gbl_pen.l3_pen = gbl_pen.s.l3_pen;
	gbl_cfg->gbl_pen.mpls_pen = gbl_pen.s.mpls_pen;
	gbl_cfg->gbl_pen.fulc_pen = gbl_pen.s.fulc_pen;
	gbl_cfg->gbl_pen.dsa_pen = gbl_pen.s.dsa_pen;
	gbl_cfg->gbl_pen.hg_pen = gbl_pen.s.hg_pen;

	tag_secret.u64 = csr_rd_node(node, CVMX_PKI_TAG_SECRET);
	gbl_cfg->tag_secret.dst6 = tag_secret.s.dst6;
	gbl_cfg->tag_secret.src6 = tag_secret.s.src6;
	gbl_cfg->tag_secret.dst = tag_secret.s.dst;
	gbl_cfg->tag_secret.src = tag_secret.s.src;

	for (id = 0; id < CVMX_PKI_NUM_FRAME_CHECK; id++) {
		frm_len_chk.u64 = csr_rd_node(node, CVMX_PKI_FRM_LEN_CHKX(id));
		gbl_cfg->frm_len[id].maxlen = frm_len_chk.s.maxlen;
		gbl_cfg->frm_len[id].minlen = frm_len_chk.s.minlen;
	}
	buf_ctl.u64 = csr_rd_node(node, CVMX_PKI_BUF_CTL);
	gbl_cfg->fpa_wait = buf_ctl.s.fpa_wait;
}

/**
 * This function writes max and min frame lengths to hardware which can be used
 * to check the size of frame arrived.There are 2 possible combination which are
 * indicated by id field.
 *
 * @param node  Node number.
 * @param id  Choose which frame len register to write to
 * @param len_chk  Struct containing byte count for max-sized/min-sized frame check.
 */
static void cvmx_pki_write_frame_len(int node, int id,
				     struct cvmx_pki_frame_len len_chk)
{
	cvmx_pki_frm_len_chkx_t frm_len_chk;

	frm_len_chk.u64 = csr_rd_node(node, CVMX_PKI_FRM_LEN_CHKX(id));
	frm_len_chk.s.maxlen = len_chk.maxlen;
	frm_len_chk.s.minlen = len_chk.minlen;
	csr_wr_node(node, CVMX_PKI_FRM_LEN_CHKX(id), frm_len_chk.u64);
}

/**
 * This function writes global configuration of PKI into hw.
 *
 * @param node  Node number.
 * @param gbl_cfg  Pointer to struct to global configuration.
 */
void cvmx_pki_write_global_config(int node,
				  struct cvmx_pki_global_config *gbl_cfg)
{
	cvmx_pki_stat_ctl_t stat_ctl;
	cvmx_pki_buf_ctl_t buf_ctl;
	unsigned int cl_grp;

	for (cl_grp = 0; cl_grp < CVMX_PKI_NUM_CLUSTER_GROUP; cl_grp++)
		cvmx_pki_attach_cluster_to_group(node, cl_grp,
						 gbl_cfg->cluster_mask[cl_grp]);

	stat_ctl.u64 = 0;
	stat_ctl.s.mode = gbl_cfg->stat_mode;
	csr_wr_node(node, CVMX_PKI_STAT_CTL, stat_ctl.u64);

	buf_ctl.u64 = csr_rd_node(node, CVMX_PKI_BUF_CTL);
	buf_ctl.s.fpa_wait = gbl_cfg->fpa_wait;
	csr_wr_node(node, CVMX_PKI_BUF_CTL, buf_ctl.u64);

	cvmx_pki_write_global_parse(node, gbl_cfg->gbl_pen);
	cvmx_pki_write_tag_secret(node, gbl_cfg->tag_secret);
	cvmx_pki_write_frame_len(node, 0, gbl_cfg->frm_len[0]);
	cvmx_pki_write_frame_len(node, 1, gbl_cfg->frm_len[1]);
}

/**
 * This function reads per pkind parameters in hardware which defines how
 * the incoming packet is processed.
 *
 * @param node  Node number.
 * @param pkind  PKI supports a large number of incoming interfaces and packets
 *     arriving on different interfaces or channels may want to be processed
 *     differently. PKI uses the pkind to determine how the incoming packet
 *     is processed.
 * @param pkind_cfg  Pointer to struct conatining pkind configuration read
 *     from the hardware.
 */
int cvmx_pki_read_pkind_config(int node, int pkind,
			       struct cvmx_pki_pkind_config *pkind_cfg)
{
	int cluster = 0;
	u64 cl_mask;
	cvmx_pki_pkindx_icgsel_t icgsel;
	cvmx_pki_clx_pkindx_style_t pstyle;
	cvmx_pki_icgx_cfg_t icg_cfg;
	cvmx_pki_clx_pkindx_cfg_t pcfg;
	cvmx_pki_clx_pkindx_skip_t skip;
	cvmx_pki_clx_pkindx_l2_custom_t l2cust;
	cvmx_pki_clx_pkindx_lg_custom_t lgcust;

	icgsel.u64 = csr_rd_node(node, CVMX_PKI_PKINDX_ICGSEL(pkind));
	icg_cfg.u64 = csr_rd_node(node, CVMX_PKI_ICGX_CFG(icgsel.s.icg));
	pkind_cfg->cluster_grp = (uint8_t)icgsel.s.icg;
	cl_mask = (uint64_t)icg_cfg.s.clusters;
	cluster = __builtin_ffsll(cl_mask) - 1;

	pstyle.u64 =
		csr_rd_node(node, CVMX_PKI_CLX_PKINDX_STYLE(pkind, cluster));
	pkind_cfg->initial_parse_mode = pstyle.s.pm;
	pkind_cfg->initial_style = pstyle.s.style;

	pcfg.u64 = csr_rd_node(node, CVMX_PKI_CLX_PKINDX_CFG(pkind, cluster));
	pkind_cfg->fcs_pres = pcfg.s.fcs_pres;
	pkind_cfg->parse_en.inst_hdr = pcfg.s.inst_hdr;
	pkind_cfg->parse_en.mpls_en = pcfg.s.mpls_en;
	pkind_cfg->parse_en.lg_custom = pcfg.s.lg_custom;
	pkind_cfg->parse_en.fulc_en = pcfg.s.fulc_en;
	pkind_cfg->parse_en.dsa_en = pcfg.s.dsa_en;
	pkind_cfg->parse_en.hg2_en = pcfg.s.hg2_en;
	pkind_cfg->parse_en.hg_en = pcfg.s.hg_en;

	skip.u64 = csr_rd_node(node, CVMX_PKI_CLX_PKINDX_SKIP(pkind, cluster));
	pkind_cfg->fcs_skip = skip.s.fcs_skip;
	pkind_cfg->inst_skip = skip.s.inst_skip;

	l2cust.u64 = csr_rd_node(node,
				 CVMX_PKI_CLX_PKINDX_L2_CUSTOM(pkind, cluster));
	pkind_cfg->l2_scan_offset = l2cust.s.offset;

	lgcust.u64 = csr_rd_node(node,
				 CVMX_PKI_CLX_PKINDX_LG_CUSTOM(pkind, cluster));
	pkind_cfg->lg_scan_offset = lgcust.s.offset;
	return 0;
}

/**
 * This function writes per pkind parameters in hardware which defines how
 * the incoming packet is processed.
 *
 * @param node  Node number.
 * @param pkind  PKI supports a large number of incoming interfaces and packets
 *     arriving on different interfaces or channels may want to be processed
 *     differently. PKI uses the pkind to determine how the incoming
 *     packet is processed.
 * @param pkind_cfg  Pointer to struct conatining pkind configuration need
 *     to be written in the hardware.
 */
int cvmx_pki_write_pkind_config(int node, int pkind,
				struct cvmx_pki_pkind_config *pkind_cfg)
{
	unsigned int cluster = 0;
	u64 cluster_mask;
	cvmx_pki_pkindx_icgsel_t icgsel;
	cvmx_pki_clx_pkindx_style_t pstyle;
	cvmx_pki_icgx_cfg_t icg_cfg;
	cvmx_pki_clx_pkindx_cfg_t pcfg;
	cvmx_pki_clx_pkindx_skip_t skip;
	cvmx_pki_clx_pkindx_l2_custom_t l2cust;
	cvmx_pki_clx_pkindx_lg_custom_t lgcust;

	if (pkind >= CVMX_PKI_NUM_PKIND ||
	    pkind_cfg->cluster_grp >= CVMX_PKI_NUM_CLUSTER_GROUP ||
	    pkind_cfg->initial_style >= CVMX_PKI_NUM_FINAL_STYLE) {
		debug("ERROR: Configuring PKIND pkind = %d cluster_group = %d style = %d\n",
		      pkind, pkind_cfg->cluster_grp, pkind_cfg->initial_style);
		return -1;
	}
	icgsel.u64 = csr_rd_node(node, CVMX_PKI_PKINDX_ICGSEL(pkind));
	icgsel.s.icg = pkind_cfg->cluster_grp;
	csr_wr_node(node, CVMX_PKI_PKINDX_ICGSEL(pkind), icgsel.u64);

	icg_cfg.u64 =
		csr_rd_node(node, CVMX_PKI_ICGX_CFG(pkind_cfg->cluster_grp));
	cluster_mask = (uint64_t)icg_cfg.s.clusters;
	while (cluster < CVMX_PKI_NUM_CLUSTER) {
		if (cluster_mask & (0x01L << cluster)) {
			pstyle.u64 = csr_rd_node(
				node,
				CVMX_PKI_CLX_PKINDX_STYLE(pkind, cluster));
			pstyle.s.pm = pkind_cfg->initial_parse_mode;
			pstyle.s.style = pkind_cfg->initial_style;
			csr_wr_node(node,
				    CVMX_PKI_CLX_PKINDX_STYLE(pkind, cluster),
				    pstyle.u64);

			pcfg.u64 = csr_rd_node(
				node, CVMX_PKI_CLX_PKINDX_CFG(pkind, cluster));
			pcfg.s.fcs_pres = pkind_cfg->fcs_pres;
			pcfg.s.inst_hdr = pkind_cfg->parse_en.inst_hdr;
			pcfg.s.mpls_en = pkind_cfg->parse_en.mpls_en;
			pcfg.s.lg_custom = pkind_cfg->parse_en.lg_custom;
			pcfg.s.fulc_en = pkind_cfg->parse_en.fulc_en;
			pcfg.s.dsa_en = pkind_cfg->parse_en.dsa_en;
			pcfg.s.hg2_en = pkind_cfg->parse_en.hg2_en;
			pcfg.s.hg_en = pkind_cfg->parse_en.hg_en;
			csr_wr_node(node,
				    CVMX_PKI_CLX_PKINDX_CFG(pkind, cluster),
				    pcfg.u64);

			skip.u64 = csr_rd_node(
				node, CVMX_PKI_CLX_PKINDX_SKIP(pkind, cluster));
			skip.s.fcs_skip = pkind_cfg->fcs_skip;
			skip.s.inst_skip = pkind_cfg->inst_skip;
			csr_wr_node(node,
				    CVMX_PKI_CLX_PKINDX_SKIP(pkind, cluster),
				    skip.u64);

			l2cust.u64 = csr_rd_node(
				node,
				CVMX_PKI_CLX_PKINDX_L2_CUSTOM(pkind, cluster));
			l2cust.s.offset = pkind_cfg->l2_scan_offset;
			csr_wr_node(node,
				    CVMX_PKI_CLX_PKINDX_L2_CUSTOM(pkind,
								  cluster),
				    l2cust.u64);

			lgcust.u64 = csr_rd_node(
				node,
				CVMX_PKI_CLX_PKINDX_LG_CUSTOM(pkind, cluster));
			lgcust.s.offset = pkind_cfg->lg_scan_offset;
			csr_wr_node(node,
				    CVMX_PKI_CLX_PKINDX_LG_CUSTOM(pkind,
								  cluster),
				    lgcust.u64);
		}
		cluster++;
	}
	return 0;
}

/**
 * This function reads parameters associated with tag configuration in hardware.
 * Only first cluster in the group is used.
 *
 * @param node  Node number.
 * @param style  Style to configure tag for.
 * @param cluster_mask	Mask of clusters to configure the style for.
 * @param tag_cfg  Pointer to tag configuration struct.
 */
void cvmx_pki_read_tag_config(int node, int style, uint64_t cluster_mask,
			      struct cvmx_pki_style_tag_cfg *tag_cfg)
{
	int mask, tag_idx, index;
	cvmx_pki_clx_stylex_cfg2_t style_cfg2;
	cvmx_pki_clx_stylex_alg_t style_alg;
	cvmx_pki_stylex_tag_sel_t tag_sel;
	cvmx_pki_tag_incx_ctl_t tag_ctl;
	cvmx_pki_tag_incx_mask_t tag_mask;
	int cluster = __builtin_ffsll(cluster_mask) - 1;

	style_cfg2.u64 =
		csr_rd_node(node, CVMX_PKI_CLX_STYLEX_CFG2(style, cluster));
	style_alg.u64 =
		csr_rd_node(node, CVMX_PKI_CLX_STYLEX_ALG(style, cluster));

	/* 7-Tuple Tag: */
	tag_cfg->tag_fields.layer_g_src = style_cfg2.s.tag_src_lg;
	tag_cfg->tag_fields.layer_f_src = style_cfg2.s.tag_src_lf;
	tag_cfg->tag_fields.layer_e_src = style_cfg2.s.tag_src_le;
	tag_cfg->tag_fields.layer_d_src = style_cfg2.s.tag_src_ld;
	tag_cfg->tag_fields.layer_c_src = style_cfg2.s.tag_src_lc;
	tag_cfg->tag_fields.layer_b_src = style_cfg2.s.tag_src_lb;
	tag_cfg->tag_fields.layer_g_dst = style_cfg2.s.tag_dst_lg;
	tag_cfg->tag_fields.layer_f_dst = style_cfg2.s.tag_dst_lf;
	tag_cfg->tag_fields.layer_e_dst = style_cfg2.s.tag_dst_le;
	tag_cfg->tag_fields.layer_d_dst = style_cfg2.s.tag_dst_ld;
	tag_cfg->tag_fields.layer_c_dst = style_cfg2.s.tag_dst_lc;
	tag_cfg->tag_fields.layer_b_dst = style_cfg2.s.tag_dst_lb;
	tag_cfg->tag_fields.tag_vni = style_alg.s.tag_vni;
	tag_cfg->tag_fields.tag_gtp = style_alg.s.tag_gtp;
	tag_cfg->tag_fields.tag_spi = style_alg.s.tag_spi;
	tag_cfg->tag_fields.tag_sync = style_alg.s.tag_syn;
	tag_cfg->tag_fields.ip_prot_nexthdr = style_alg.s.tag_pctl;
	tag_cfg->tag_fields.second_vlan = style_alg.s.tag_vs1;
	tag_cfg->tag_fields.first_vlan = style_alg.s.tag_vs0;
	tag_cfg->tag_fields.mpls_label = style_alg.s.tag_mpls0;
	tag_cfg->tag_fields.input_port = style_alg.s.tag_prt;

	/* Custom-Mask Tag: */
	tag_sel.u64 = csr_rd_node(node, CVMX_PKI_STYLEX_TAG_SEL(style));
	for (mask = 0; mask < 4; mask++) {
		tag_cfg->mask_tag[mask].enable =
			(style_cfg2.s.tag_inc & (1 << mask)) != 0;
		switch (mask) {
		case 0:
			tag_idx = tag_sel.s.tag_idx0;
			break;
		case 1:
			tag_idx = tag_sel.s.tag_idx1;
			break;
		case 2:
			tag_idx = tag_sel.s.tag_idx2;
			break;
		case 3:
			tag_idx = tag_sel.s.tag_idx3;
			break;
		}
		index = tag_idx * 4 + mask;
		tag_mask.u64 = csr_rd_node(node, CVMX_PKI_TAG_INCX_MASK(index));
		tag_cfg->mask_tag[mask].val = tag_mask.s.en;
		tag_ctl.u64 = csr_rd_node(node, CVMX_PKI_TAG_INCX_CTL(index));
		tag_cfg->mask_tag[mask].base = tag_ctl.s.ptr_sel;
		tag_cfg->mask_tag[mask].offset = tag_ctl.s.offset;
	}
}

/**
 * This function writes/configures parameters associated with tag configuration in
 * hardware. In Custom-Mask Tagging, all four masks use the same base index
 * to access Tag Control and Tag Mask registers.
 *
 * @param node  Node number.
 * @param style  Style to configure tag for.
 * @param cluster_mask  Mask of clusters to configure the style for.
 * @param tag_cfg  Pointer to taf configuration struct.
 */
void cvmx_pki_write_tag_config(int node, int style, uint64_t cluster_mask,
			       struct cvmx_pki_style_tag_cfg *tag_cfg)
{
	int mask, index, tag_idx, mtag_en = 0;
	unsigned int cluster = 0;
	cvmx_pki_clx_stylex_cfg2_t scfg2;
	cvmx_pki_clx_stylex_alg_t style_alg;
	cvmx_pki_tag_incx_ctl_t tag_ctl;
	cvmx_pki_tag_incx_mask_t tag_mask;
	cvmx_pki_stylex_tag_sel_t tag_sel;

	while (cluster < CVMX_PKI_NUM_CLUSTER) {
		if (cluster_mask & (0x01L << cluster)) {
			/* 7-Tuple Tag: */
			scfg2.u64 = csr_rd_node(
				node, CVMX_PKI_CLX_STYLEX_CFG2(style, cluster));
			scfg2.s.tag_src_lg = tag_cfg->tag_fields.layer_g_src;
			scfg2.s.tag_src_lf = tag_cfg->tag_fields.layer_f_src;
			scfg2.s.tag_src_le = tag_cfg->tag_fields.layer_e_src;
			scfg2.s.tag_src_ld = tag_cfg->tag_fields.layer_d_src;
			scfg2.s.tag_src_lc = tag_cfg->tag_fields.layer_c_src;
			scfg2.s.tag_src_lb = tag_cfg->tag_fields.layer_b_src;
			scfg2.s.tag_dst_lg = tag_cfg->tag_fields.layer_g_dst;
			scfg2.s.tag_dst_lf = tag_cfg->tag_fields.layer_f_dst;
			scfg2.s.tag_dst_le = tag_cfg->tag_fields.layer_e_dst;
			scfg2.s.tag_dst_ld = tag_cfg->tag_fields.layer_d_dst;
			scfg2.s.tag_dst_lc = tag_cfg->tag_fields.layer_c_dst;
			scfg2.s.tag_dst_lb = tag_cfg->tag_fields.layer_b_dst;
			csr_wr_node(node,
				    CVMX_PKI_CLX_STYLEX_CFG2(style, cluster),
				    scfg2.u64);

			style_alg.u64 = csr_rd_node(
				node, CVMX_PKI_CLX_STYLEX_ALG(style, cluster));
			style_alg.s.tag_vni = tag_cfg->tag_fields.tag_vni;
			style_alg.s.tag_gtp = tag_cfg->tag_fields.tag_gtp;
			style_alg.s.tag_spi = tag_cfg->tag_fields.tag_spi;
			style_alg.s.tag_syn = tag_cfg->tag_fields.tag_sync;
			style_alg.s.tag_pctl =
				tag_cfg->tag_fields.ip_prot_nexthdr;
			style_alg.s.tag_vs1 = tag_cfg->tag_fields.second_vlan;
			style_alg.s.tag_vs0 = tag_cfg->tag_fields.first_vlan;
			style_alg.s.tag_mpls0 = tag_cfg->tag_fields.mpls_label;
			style_alg.s.tag_prt = tag_cfg->tag_fields.input_port;
			csr_wr_node(node,
				    CVMX_PKI_CLX_STYLEX_ALG(style, cluster),
				    style_alg.u64);

			/* Custom-Mask Tag (Part 1): */
			for (mask = 0; mask < 4; mask++) {
				if (tag_cfg->mask_tag[mask].enable)
					mtag_en++;
			}
			if (mtag_en) {
				scfg2.u64 = csr_rd_node(
					node, CVMX_PKI_CLX_STYLEX_CFG2(
						      style, cluster));
				scfg2.s.tag_inc = 0;
				for (mask = 0; mask < 4; mask++) {
					if (tag_cfg->mask_tag[mask].enable)
						scfg2.s.tag_inc |= 1 << mask;
				}
				csr_wr_node(node,
					    CVMX_PKI_CLX_STYLEX_CFG2(style,
								     cluster),
					    scfg2.u64);
			}
		}
		cluster++;
	}
	/* Custom-Mask Tag (Part 2): */
	if (mtag_en) {
		tag_idx = cvmx_pki_mtag_idx_alloc(node, -1);
		if (tag_idx < 0)
			return;

		tag_sel.u64 = csr_rd_node(node, CVMX_PKI_STYLEX_TAG_SEL(style));
		for (mask = 0; mask < 4; mask++) {
			if (tag_cfg->mask_tag[mask].enable) {
				switch (mask) {
				case 0:
					tag_sel.s.tag_idx0 = tag_idx;
					break;
				case 1:
					tag_sel.s.tag_idx1 = tag_idx;
					break;
				case 2:
					tag_sel.s.tag_idx2 = tag_idx;
					break;
				case 3:
					tag_sel.s.tag_idx3 = tag_idx;
					break;
				}
				index = tag_idx * 4 + mask;
				tag_mask.u64 = csr_rd_node(
					node, CVMX_PKI_TAG_INCX_MASK(index));
				tag_mask.s.en = tag_cfg->mask_tag[mask].val;
				csr_wr_node(node, CVMX_PKI_TAG_INCX_MASK(index),
					    tag_mask.u64);

				tag_ctl.u64 = csr_rd_node(
					node, CVMX_PKI_TAG_INCX_CTL(index));
				tag_ctl.s.ptr_sel =
					tag_cfg->mask_tag[mask].base;
				tag_ctl.s.offset =
					tag_cfg->mask_tag[mask].offset;
				csr_wr_node(node, CVMX_PKI_TAG_INCX_CTL(index),
					    tag_ctl.u64);
			}
		}
		csr_wr_node(node, CVMX_PKI_STYLEX_TAG_SEL(style), tag_sel.u64);
	}
}

/**
 * This function reads parameters associated with style in hardware.
 *
 * @param node  Node number.
 * @param style	Style to read from.
 * @param cluster_mask	Mask of clusters style belongs to.
 * @param style_cfg	 Pointer to style config struct.
 */
void cvmx_pki_read_style_config(int node, int style, uint64_t cluster_mask,
				struct cvmx_pki_style_config *style_cfg)
{
	cvmx_pki_clx_stylex_cfg_t scfg;
	cvmx_pki_clx_stylex_cfg2_t scfg2;
	cvmx_pki_clx_stylex_alg_t style_alg;
	cvmx_pki_stylex_buf_t style_buf;
	int cluster = __builtin_ffsll(cluster_mask) - 1;

	scfg.u64 = csr_rd_node(node, CVMX_PKI_CLX_STYLEX_CFG(style, cluster));
	scfg2.u64 = csr_rd_node(node, CVMX_PKI_CLX_STYLEX_CFG2(style, cluster));
	style_alg.u64 =
		csr_rd_node(node, CVMX_PKI_CLX_STYLEX_ALG(style, cluster));
	style_buf.u64 = csr_rd_node(node, CVMX_PKI_STYLEX_BUF(style));

	style_cfg->parm_cfg.ip6_udp_opt = scfg.s.ip6_udp_opt;
	style_cfg->parm_cfg.lenerr_en = scfg.s.lenerr_en;
	style_cfg->parm_cfg.lenerr_eqpad = scfg.s.lenerr_eqpad;
	style_cfg->parm_cfg.maxerr_en = scfg.s.maxerr_en;
	style_cfg->parm_cfg.minerr_en = scfg.s.minerr_en;
	style_cfg->parm_cfg.fcs_chk = scfg.s.fcs_chk;
	style_cfg->parm_cfg.fcs_strip = scfg.s.fcs_strip;
	style_cfg->parm_cfg.minmax_sel = scfg.s.minmax_sel;
	style_cfg->parm_cfg.qpg_base = scfg.s.qpg_base;
	style_cfg->parm_cfg.qpg_dis_padd = scfg.s.qpg_dis_padd;
	style_cfg->parm_cfg.qpg_dis_aura = scfg.s.qpg_dis_aura;
	style_cfg->parm_cfg.qpg_dis_grp = scfg.s.qpg_dis_grp;
	style_cfg->parm_cfg.qpg_dis_grptag = scfg.s.qpg_dis_grptag;
	style_cfg->parm_cfg.rawdrp = scfg.s.rawdrp;
	style_cfg->parm_cfg.force_drop = scfg.s.drop;
	style_cfg->parm_cfg.nodrop = scfg.s.nodrop;

	style_cfg->parm_cfg.len_lg = scfg2.s.len_lg;
	style_cfg->parm_cfg.len_lf = scfg2.s.len_lf;
	style_cfg->parm_cfg.len_le = scfg2.s.len_le;
	style_cfg->parm_cfg.len_ld = scfg2.s.len_ld;
	style_cfg->parm_cfg.len_lc = scfg2.s.len_lc;
	style_cfg->parm_cfg.len_lb = scfg2.s.len_lb;
	style_cfg->parm_cfg.csum_lg = scfg2.s.csum_lg;
	style_cfg->parm_cfg.csum_lf = scfg2.s.csum_lf;
	style_cfg->parm_cfg.csum_le = scfg2.s.csum_le;
	style_cfg->parm_cfg.csum_ld = scfg2.s.csum_ld;
	style_cfg->parm_cfg.csum_lc = scfg2.s.csum_lc;
	style_cfg->parm_cfg.csum_lb = scfg2.s.csum_lb;

	style_cfg->parm_cfg.qpg_qos = style_alg.s.qpg_qos;
	style_cfg->parm_cfg.tag_type = style_alg.s.tt;
	style_cfg->parm_cfg.apad_nip = style_alg.s.apad_nip;
	style_cfg->parm_cfg.qpg_port_sh = style_alg.s.qpg_port_sh;
	style_cfg->parm_cfg.qpg_port_msb = style_alg.s.qpg_port_msb;
	style_cfg->parm_cfg.wqe_vs = style_alg.s.wqe_vs;

	style_cfg->parm_cfg.pkt_lend = style_buf.s.pkt_lend;
	style_cfg->parm_cfg.wqe_hsz = style_buf.s.wqe_hsz;
	style_cfg->parm_cfg.wqe_skip = style_buf.s.wqe_skip * 128;
	style_cfg->parm_cfg.first_skip = style_buf.s.first_skip * 8;
	style_cfg->parm_cfg.later_skip = style_buf.s.later_skip * 8;
	style_cfg->parm_cfg.cache_mode = style_buf.s.opc_mode;
	style_cfg->parm_cfg.mbuff_size = style_buf.s.mb_size * 8;
	style_cfg->parm_cfg.dis_wq_dat = style_buf.s.dis_wq_dat;

	cvmx_pki_read_tag_config(node, style, cluster_mask,
				 &style_cfg->tag_cfg);
}

/**
 * This function writes/configures parameters associated with style in hardware.
 *
 * @param node  Node number.
 * @param style  Style to configure.
 * @param cluster_mask  Mask of clusters to configure the style for.
 * @param style_cfg	 Pointer to style config struct.
 */
void cvmx_pki_write_style_config(int node, uint64_t style, u64 cluster_mask,
				 struct cvmx_pki_style_config *style_cfg)
{
	cvmx_pki_clx_stylex_cfg_t scfg;
	cvmx_pki_clx_stylex_cfg2_t scfg2;
	cvmx_pki_clx_stylex_alg_t style_alg;
	cvmx_pki_stylex_buf_t style_buf;
	unsigned int cluster = 0;

	while (cluster < CVMX_PKI_NUM_CLUSTER) {
		if (cluster_mask & (0x01L << cluster)) {
			scfg.u64 = csr_rd_node(
				node, CVMX_PKI_CLX_STYLEX_CFG(style, cluster));
			scfg.s.ip6_udp_opt = style_cfg->parm_cfg.ip6_udp_opt;
			scfg.s.lenerr_en = style_cfg->parm_cfg.lenerr_en;
			scfg.s.lenerr_eqpad = style_cfg->parm_cfg.lenerr_eqpad;
			scfg.s.maxerr_en = style_cfg->parm_cfg.maxerr_en;
			scfg.s.minerr_en = style_cfg->parm_cfg.minerr_en;
			scfg.s.fcs_chk = style_cfg->parm_cfg.fcs_chk;
			scfg.s.fcs_strip = style_cfg->parm_cfg.fcs_strip;
			scfg.s.minmax_sel = style_cfg->parm_cfg.minmax_sel;
			scfg.s.qpg_base = style_cfg->parm_cfg.qpg_base;
			scfg.s.qpg_dis_padd = style_cfg->parm_cfg.qpg_dis_padd;
			scfg.s.qpg_dis_aura = style_cfg->parm_cfg.qpg_dis_aura;
			scfg.s.qpg_dis_grp = style_cfg->parm_cfg.qpg_dis_grp;
			scfg.s.qpg_dis_grptag =
				style_cfg->parm_cfg.qpg_dis_grptag;
			scfg.s.rawdrp = style_cfg->parm_cfg.rawdrp;
			scfg.s.drop = style_cfg->parm_cfg.force_drop;
			scfg.s.nodrop = style_cfg->parm_cfg.nodrop;
			csr_wr_node(node,
				    CVMX_PKI_CLX_STYLEX_CFG(style, cluster),
				    scfg.u64);

			scfg2.u64 = csr_rd_node(
				node, CVMX_PKI_CLX_STYLEX_CFG2(style, cluster));
			scfg2.s.len_lg = style_cfg->parm_cfg.len_lg;
			scfg2.s.len_lf = style_cfg->parm_cfg.len_lf;
			scfg2.s.len_le = style_cfg->parm_cfg.len_le;
			scfg2.s.len_ld = style_cfg->parm_cfg.len_ld;
			scfg2.s.len_lc = style_cfg->parm_cfg.len_lc;
			scfg2.s.len_lb = style_cfg->parm_cfg.len_lb;
			scfg2.s.csum_lg = style_cfg->parm_cfg.csum_lg;
			scfg2.s.csum_lf = style_cfg->parm_cfg.csum_lf;
			scfg2.s.csum_le = style_cfg->parm_cfg.csum_le;
			scfg2.s.csum_ld = style_cfg->parm_cfg.csum_ld;
			scfg2.s.csum_lc = style_cfg->parm_cfg.csum_lc;
			scfg2.s.csum_lb = style_cfg->parm_cfg.csum_lb;
			csr_wr_node(node,
				    CVMX_PKI_CLX_STYLEX_CFG2(style, cluster),
				    scfg2.u64);

			style_alg.u64 = csr_rd_node(
				node, CVMX_PKI_CLX_STYLEX_ALG(style, cluster));
			style_alg.s.qpg_qos = style_cfg->parm_cfg.qpg_qos;
			style_alg.s.tt = style_cfg->parm_cfg.tag_type;
			style_alg.s.apad_nip = style_cfg->parm_cfg.apad_nip;
			style_alg.s.qpg_port_sh =
				style_cfg->parm_cfg.qpg_port_sh;
			style_alg.s.qpg_port_msb =
				style_cfg->parm_cfg.qpg_port_msb;
			style_alg.s.wqe_vs = style_cfg->parm_cfg.wqe_vs;
			csr_wr_node(node,
				    CVMX_PKI_CLX_STYLEX_ALG(style, cluster),
				    style_alg.u64);
		}
		cluster++;
	}
	style_buf.u64 = csr_rd_node(node, CVMX_PKI_STYLEX_BUF(style));
	style_buf.s.pkt_lend = style_cfg->parm_cfg.pkt_lend;
	style_buf.s.wqe_hsz = style_cfg->parm_cfg.wqe_hsz;
	style_buf.s.wqe_skip = (style_cfg->parm_cfg.wqe_skip) / 128;
	style_buf.s.first_skip = (style_cfg->parm_cfg.first_skip) / 8;
	style_buf.s.later_skip = style_cfg->parm_cfg.later_skip / 8;
	style_buf.s.opc_mode = style_cfg->parm_cfg.cache_mode;
	style_buf.s.mb_size = (style_cfg->parm_cfg.mbuff_size) / 8;
	style_buf.s.dis_wq_dat = style_cfg->parm_cfg.dis_wq_dat;
	csr_wr_node(node, CVMX_PKI_STYLEX_BUF(style), style_buf.u64);

	cvmx_pki_write_tag_config(node, style, cluster_mask,
				  &style_cfg->tag_cfg);
}

/**
 * This function reads qpg entry at specified offset from qpg table.
 *
 * @param node  Node number.
 * @param offset  Offset in qpg table to read from.
 * @param qpg_cfg  Pointer to structure containing qpg values.
 */
int cvmx_pki_read_qpg_entry(int node, int offset,
			    struct cvmx_pki_qpg_config *qpg_cfg)
{
	cvmx_pki_qpg_tblx_t qpg_tbl;

	if (offset >= CVMX_PKI_NUM_QPG_ENTRY) {
		debug("ERROR: qpg offset %d is >= 2048\n", offset);
		return -1;
	}
	qpg_tbl.u64 = csr_rd_node(node, CVMX_PKI_QPG_TBLX(offset));
	qpg_cfg->aura_num = qpg_tbl.s.laura;
	qpg_cfg->port_add = qpg_tbl.s.padd;
	qpg_cfg->grp_ok = qpg_tbl.s.grp_ok;
	qpg_cfg->grp_bad = qpg_tbl.s.grp_bad;
	qpg_cfg->grptag_ok = qpg_tbl.s.grptag_ok;
	qpg_cfg->grptag_bad = qpg_tbl.s.grptag_bad;
	return 0;
}

/**
 * This function writes qpg entry at specified offset in qpg table.
 *
 * @param node  Node number.
 * @param offset  Offset in qpg table to read from.
 * @param qpg_cfg  Pointer to structure containing qpg values.
 */
void cvmx_pki_write_qpg_entry(int node, int offset,
			      struct cvmx_pki_qpg_config *qpg_cfg)
{
	cvmx_pki_qpg_tblx_t qpg_tbl;

	qpg_tbl.u64 = csr_rd_node(node, CVMX_PKI_QPG_TBLX(offset));
	qpg_tbl.s.padd = qpg_cfg->port_add;
	qpg_tbl.s.laura = qpg_cfg->aura_num;
	qpg_tbl.s.grp_ok = qpg_cfg->grp_ok;
	qpg_tbl.s.grp_bad = qpg_cfg->grp_bad;
	qpg_tbl.s.grptag_ok = qpg_cfg->grptag_ok;
	qpg_tbl.s.grptag_bad = qpg_cfg->grptag_bad;
	csr_wr_node(node, CVMX_PKI_QPG_TBLX(offset), qpg_tbl.u64);
}

/**
 * This function writes pcam entry at given offset in pcam table in hardware
 *
 * @param node  Node number.
 * @param index  Offset in pcam table.
 * @param cluster_mask	Mask of clusters in which to write pcam entry.
 * @param input  Input keys to pcam match passed as struct.
 * @param action  PCAM match action passed as struct.
 */
int cvmx_pki_pcam_write_entry(int node, int index, uint64_t cluster_mask,
			      struct cvmx_pki_pcam_input input,
			      struct cvmx_pki_pcam_action action)
{
	int bank;
	unsigned int cluster = 0;
	cvmx_pki_clx_pcamx_termx_t term;
	cvmx_pki_clx_pcamx_matchx_t match;
	cvmx_pki_clx_pcamx_actionx_t act;

	if (index >= CVMX_PKI_TOTAL_PCAM_ENTRY) {
		debug("\nERROR: Invalid pcam entry %d\n", index);
		return -1;
	}
	bank = (int)(input.field & 0x01);
	while (cluster < CVMX_PKI_NUM_CLUSTER) {
		if (cluster_mask & (0x01L << cluster)) {
			term.u64 = csr_rd_node(
				node,
				CVMX_PKI_CLX_PCAMX_TERMX(cluster, bank, index));
			term.s.valid = 0;
			csr_wr_node(node,
				    CVMX_PKI_CLX_PCAMX_TERMX(cluster, bank,
							     index),
				    term.u64);
			match.u64 = csr_rd_node(
				node, CVMX_PKI_CLX_PCAMX_MATCHX(cluster, bank,
								index));
			match.s.data1 = input.data & input.data_mask;
			match.s.data0 = (~input.data) & input.data_mask;
			csr_wr_node(node,
				    CVMX_PKI_CLX_PCAMX_MATCHX(cluster, bank,
							      index),
				    match.u64);

			act.u64 = csr_rd_node(
				node, CVMX_PKI_CLX_PCAMX_ACTIONX(cluster, bank,
								 index));
			act.s.pmc = action.parse_mode_chg;
			act.s.style_add = action.style_add;
			act.s.pf = action.parse_flag_set;
			act.s.setty = action.layer_type_set;
			act.s.advance = action.pointer_advance;
			csr_wr_node(node,
				    CVMX_PKI_CLX_PCAMX_ACTIONX(cluster, bank,
							       index),
				    act.u64);

			term.u64 = csr_rd_node(
				node,
				CVMX_PKI_CLX_PCAMX_TERMX(cluster, bank, index));
			term.s.term1 = input.field & input.field_mask;
			term.s.term0 = (~input.field) & input.field_mask;
			term.s.style1 = input.style & input.style_mask;
			term.s.style0 = (~input.style) & input.style_mask;
			term.s.valid = 1;
			csr_wr_node(node,
				    CVMX_PKI_CLX_PCAMX_TERMX(cluster, bank,
							     index),
				    term.u64);
		}
		cluster++;
	}
	return 0;
}

/**
 * Enables/Disables fcs check and fcs stripping on the pkind.
 *
 * @param node  Node number
 * @param pknd  PKIND to apply settings on.
 * @param fcs_chk  Enable/disable fcs check.
 *    1 = enable fcs error check.
 *    0 = disable fcs error check.
 * @param fcs_strip	 Strip L2 FCS bytes from packet, decrease WQE[LEN] by 4 bytes
 *    1 = strip L2 FCS.
 *    0 = Do not strip L2 FCS.
 */
void cvmx_pki_endis_fcs_check(int node, int pknd, bool fcs_chk, bool fcs_strip)
{
	int style;
	unsigned int cluster;
	cvmx_pki_clx_pkindx_style_t pstyle;
	cvmx_pki_clx_stylex_cfg_t style_cfg;

	/* Valudate PKIND # */
	if (pknd >= CVMX_PKI_NUM_PKIND) {
		printf("%s: PKIND %d out of range\n", __func__, pknd);
		return;
	}

	for (cluster = 0; cluster < CVMX_PKI_NUM_CLUSTER; cluster++) {
		pstyle.u64 = csr_rd_node(
			node, CVMX_PKI_CLX_PKINDX_STYLE(pknd, cluster));
		style = pstyle.s.style;
		/* Validate STYLE # */
		if (style >= CVMX_PKI_NUM_INTERNAL_STYLE)
			continue;
		style_cfg.u64 = csr_rd_node(
			node, CVMX_PKI_CLX_STYLEX_CFG(style, cluster));
		style_cfg.s.fcs_chk = fcs_chk;
		style_cfg.s.fcs_strip = fcs_strip;
		csr_wr_node(node, CVMX_PKI_CLX_STYLEX_CFG(style, cluster),
			    style_cfg.u64);
	}
}

/**
 * Enables/Disables l2 length error check and max & min frame length checks
 *
 * @param node  Node number
 * @param pknd  PKIND to disable error for.
 * @param l2len_err  L2 length error check enable.
 * @param maxframe_err  Max frame error check enable.
 * @param minframe_err  Min frame error check enable.
 *    1 = Enabel err checks
 *    0 = Disable error checks
 */
void cvmx_pki_endis_l2_errs(int node, int pknd, bool l2len_err,
			    bool maxframe_err, bool minframe_err)
{
	int style;
	unsigned int cluster;
	cvmx_pki_clx_pkindx_style_t pstyle;
	cvmx_pki_clx_stylex_cfg_t style_cfg;

	/* Valudate PKIND # */
	if (pknd >= CVMX_PKI_NUM_PKIND) {
		printf("%s: PKIND %d out of range\n", __func__, pknd);
		return;
	}

	for (cluster = 0; cluster < CVMX_PKI_NUM_CLUSTER; cluster++) {
		pstyle.u64 = csr_rd_node(
			node, CVMX_PKI_CLX_PKINDX_STYLE(pknd, cluster));
		style = pstyle.s.style;
		/* Validate STYLE # */
		if (style >= CVMX_PKI_NUM_INTERNAL_STYLE)
			continue;
		style_cfg.u64 = csr_rd_node(
			node, CVMX_PKI_CLX_STYLEX_CFG(style, cluster));
		style_cfg.s.lenerr_en = l2len_err;
		style_cfg.s.maxerr_en = maxframe_err;
		style_cfg.s.minerr_en = minframe_err;
		csr_wr_node(node, CVMX_PKI_CLX_STYLEX_CFG(style, cluster),
			    style_cfg.u64);
	}
}
