/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020 Marvell International Ltd.
 *
 * Helper functions for PKI
 */

#ifndef __CVMX_HELPER_PKI_H__
#define __CVMX_HELPER_PKI_H__

#include "cvmx-pki.h"

/* Modify this if more than 8 ilk channels need to be supported */
#define CVMX_MAX_PORT_PER_INTERFACE  64
#define CVMX_MAX_QOS_PRIORITY	     64
#define CVMX_PKI_FIND_AVAILABLE_RSRC (-1)

struct cvmx_pki_qos_schd {
	cvmx_fpa3_gaura_t _aura;
	cvmx_fpa3_pool_t _pool;
	bool pool_per_qos;
	int pool_num;
	char *pool_name;
	u64 pool_buff_size;
	u64 pool_max_buff;
	bool aura_per_qos;
	int aura_num;
	char *aura_name;
	u64 aura_buff_cnt;
	bool sso_grp_per_qos;
	int sso_grp;
	u16 port_add;
	int qpg_base;
};

struct cvmx_pki_prt_schd {
	cvmx_fpa3_pool_t _pool;
	cvmx_fpa3_gaura_t _aura;
	bool cfg_port;
	int style;
	bool pool_per_prt;
	int pool_num;
	char *pool_name;
	u64 pool_buff_size;
	u64 pool_max_buff;
	bool aura_per_prt;
	int aura_num;
	char *aura_name;
	u64 aura_buff_cnt;
	bool sso_grp_per_prt;
	int sso_grp;
	enum cvmx_pki_qpg_qos qpg_qos;
	int qpg_base;
	struct cvmx_pki_qos_schd qos_s[CVMX_MAX_QOS_PRIORITY];
};

struct cvmx_pki_intf_schd {
	cvmx_fpa3_pool_t _pool;
	cvmx_fpa3_gaura_t _aura;
	bool style_per_intf;
	int style;
	bool pool_per_intf;
	int pool_num;
	char *pool_name;
	u64 pool_buff_size;
	u64 pool_max_buff;
	bool aura_per_intf;
	int aura_num;
	char *aura_name;
	u64 aura_buff_cnt;
	bool sso_grp_per_intf;
	int sso_grp;
	bool qos_share_aura;
	bool qos_share_grp;
	int qpg_base;
	struct cvmx_pki_prt_schd prt_s[CVMX_MAX_PORT_PER_INTERFACE];
};

struct cvmx_pki_global_schd {
	bool setup_pool;
	int pool_num;
	char *pool_name;
	u64 pool_buff_size;
	u64 pool_max_buff;
	bool setup_aura;
	int aura_num;
	char *aura_name;
	u64 aura_buff_cnt;
	bool setup_sso_grp;
	int sso_grp;
	cvmx_fpa3_pool_t _pool;
	cvmx_fpa3_gaura_t _aura;
};

struct cvmx_pki_legacy_qos_watcher {
	bool configured;
	enum cvmx_pki_term field;
	u32 data;
	u32 data_mask;
	u8 advance;
	u8 sso_grp;
};

extern bool cvmx_pki_dflt_init[CVMX_MAX_NODES];

extern struct cvmx_pki_pool_config pki_dflt_pool[CVMX_MAX_NODES];
extern struct cvmx_pki_aura_config pki_dflt_aura[CVMX_MAX_NODES];
extern struct cvmx_pki_style_config pki_dflt_style[CVMX_MAX_NODES];
extern struct cvmx_pki_pkind_config pki_dflt_pkind[CVMX_MAX_NODES];
extern u64 pkind_style_map[CVMX_MAX_NODES][CVMX_PKI_NUM_PKIND];
extern struct cvmx_pki_sso_grp_config pki_dflt_sso_grp[CVMX_MAX_NODES];
extern struct cvmx_pki_legacy_qos_watcher qos_watcher[8];

/**
 * This function Enabled the PKI hardware to
 * start accepting/processing packets.
 * @param node    node number
 */
void cvmx_helper_pki_enable(int node);

/**
 * This function frees up PKI resources consumed by that port.
 * This function should only be called if port resources
 * (fpa pools aura, style qpg entry pcam entry etc.) are not shared
 * @param xipd_port     ipd port number for which resources need to
 *                      be freed.
 */
int cvmx_helper_pki_port_shutdown(int xipd_port);

/**
 * This function shuts down complete PKI hardware
 * and software resources.
 * @param node          node number where PKI needs to shutdown.
 */
void cvmx_helper_pki_shutdown(int node);

/**
 * This function calculates how mant qpf entries will be needed for
 * a particular QOS.
 * @param qpg_qos       qos value for which entries need to be calculated.
 */
int cvmx_helper_pki_get_num_qpg_entry(enum cvmx_pki_qpg_qos qpg_qos);

/**
 * This function setups the qos table by allocating qpg entry and writing
 * the provided parameters to that entry (offset).
 * @param node          node number.
 * @param qpg_cfg       pointer to struct containing qpg configuration
 */
int cvmx_helper_pki_set_qpg_entry(int node, struct cvmx_pki_qpg_config *qpg_cfg);

/**
 * This function sets up aura QOS for RED, backpressure and tail-drop.
 *
 * @param node       node number.
 * @param aura       aura to configure.
 * @param ena_red       enable RED based on [DROP] and [PASS] levels
 *			1: enable 0:disable
 * @param pass_thresh   pass threshold for RED.
 * @param drop_thresh   drop threshold for RED
 * @param ena_bp        enable backpressure based on [BP] level.
 *			1:enable 0:disable
 * @param bp_thresh     backpressure threshold.
 * @param ena_drop      enable tail drop.
 *			1:enable 0:disable
 * Return: Zero on success. Negative on failure
 */
int cvmx_helper_setup_aura_qos(int node, int aura, bool ena_red, bool ena_drop, u64 pass_thresh,
			       u64 drop_thresh, bool ena_bp, u64 bp_thresh);

/**
 * This function maps specified bpid to all the auras from which it can receive bp and
 * then maps that bpid to all the channels, that bpid can asserrt bp on.
 *
 * @param node          node number.
 * @param aura          aura number which will back pressure specified bpid.
 * @param bpid          bpid to map.
 * @param chl_map       array of channels to map to that bpid.
 * @param chl_cnt	number of channel/ports to map to that bpid.
 * Return: Zero on success. Negative on failure
 */
int cvmx_helper_pki_map_aura_chl_bpid(int node, u16 aura, u16 bpid, u16 chl_map[], u16 chl_cnt);

/**
 * This function sets up the global pool, aura and sso group
 * resources which application can use between any interfaces
 * and ports.
 * @param node          node number
 * @param gblsch        pointer to struct containing global
 *                      scheduling parameters.
 */
int cvmx_helper_pki_set_gbl_schd(int node, struct cvmx_pki_global_schd *gblsch);

/**
 * This function sets up scheduling parameters (pool, aura, sso group etc)
 * of an ipd port.
 * @param xipd_port     ipd port number
 * @param prtsch        pointer to struct containing port's
 *                      scheduling parameters.
 */
int cvmx_helper_pki_init_port(int xipd_port, struct cvmx_pki_prt_schd *prtsch);

/**
 * This function sets up scheduling parameters (pool, aura, sso group etc)
 * of an interface (all ports/channels on that interface).
 * @param xiface        interface number with node.
 * @param intfsch      pointer to struct containing interface
 *                      scheduling parameters.
 * @param gblsch       pointer to struct containing global scheduling parameters
 *                      (can be NULL if not used)
 */
int cvmx_helper_pki_init_interface(const int xiface, struct cvmx_pki_intf_schd *intfsch,
				   struct cvmx_pki_global_schd *gblsch);
/**
 * This function gets all the PKI parameters related to that
 * particular port from hardware.
 * @param xipd_port     ipd port number to get parameter of
 * @param port_cfg      pointer to structure where to store read parameters
 */
void cvmx_pki_get_port_config(int xipd_port, struct cvmx_pki_port_config *port_cfg);

/**
 * This function sets all the PKI parameters related to that
 * particular port in hardware.
 * @param xipd_port     ipd port number to get parameter of
 * @param port_cfg      pointer to structure containing port parameters
 */
void cvmx_pki_set_port_config(int xipd_port, struct cvmx_pki_port_config *port_cfg);

/**
 * This function displays all the PKI parameters related to that
 * particular port.
 * @param xipd_port      ipd port number to display parameter of
 */
void cvmx_pki_show_port_config(int xipd_port);

/**
 * Modifies maximum frame length to check.
 * It modifies the global frame length set used by this port, any other
 * port using the same set will get affected too.
 * @param xipd_port	ipd port for which to modify max len.
 * @param max_size	maximum frame length
 */
void cvmx_pki_set_max_frm_len(int xipd_port, u32 max_size);

/**
 * This function sets up all the ports of particular interface
 * for chosen fcs mode. (only use for backward compatibility).
 * New application can control it via init_interface calls.
 * @param node          node number.
 * @param interface     interface number.
 * @param nports        number of ports
 * @param has_fcs       1 -- enable fcs check and fcs strip.
 *                      0 -- disable fcs check.
 */
void cvmx_helper_pki_set_fcs_op(int node, int interface, int nports, int has_fcs);

/**
 * This function sets the wqe buffer mode of all ports. First packet data buffer can reside
 * either in same buffer as wqe OR it can go in separate buffer. If used the later mode,
 * make sure software allocate enough buffers to now have wqe separate from packet data.
 * @param node	                node number.
 * @param pkt_outside_wqe	0 = The packet link pointer will be at word [FIRST_SKIP]
 *				    immediately followed by packet data, in the same buffer
 *				    as the work queue entry.
 *				1 = The packet link pointer will be at word [FIRST_SKIP] in a new
 *				    buffer separate from the work queue entry. Words following the
 *				    WQE in the same cache line will be zeroed, other lines in the
 *				    buffer will not be modified and will retain stale data (from the
 *				    buffer’s previous use). This setting may decrease the peak PKI
 *				    performance by up to half on small packets.
 */
void cvmx_helper_pki_set_wqe_mode(int node, bool pkt_outside_wqe);

/**
 * This function sets the Packet mode of all ports and styles to little-endian.
 * It Changes write operations of packet data to L2C to
 * be in little-endian. Does not change the WQE header format, which is
 * properly endian neutral.
 * @param node	                node number.
 */
void cvmx_helper_pki_set_little_endian(int node);

void cvmx_helper_pki_set_dflt_pool(int node, int pool, int buffer_size, int buffer_count);
void cvmx_helper_pki_set_dflt_aura(int node, int aura, int pool, int buffer_count);
void cvmx_helper_pki_set_dflt_pool_buffer(int node, int buffer_count);

void cvmx_helper_pki_set_dflt_aura_buffer(int node, int buffer_count);

void cvmx_helper_pki_set_dflt_pkind_map(int node, int pkind, int style);

void cvmx_helper_pki_get_dflt_style(int node, struct cvmx_pki_style_config *style_cfg);
void cvmx_helper_pki_set_dflt_style(int node, struct cvmx_pki_style_config *style_cfg);

void cvmx_helper_pki_get_dflt_qpg(int node, struct cvmx_pki_qpg_config *qpg_cfg);
void cvmx_helper_pki_set_dflt_qpg(int node, struct cvmx_pki_qpg_config *qpg_cfg);

void cvmx_helper_pki_no_dflt_init(int node);

void cvmx_helper_pki_set_dflt_bp_en(int node, bool bp_en);

void cvmx_pki_dump_wqe(const cvmx_wqe_78xx_t *wqp);

int __cvmx_helper_pki_port_setup(int node, int xipd_port);

int __cvmx_helper_pki_global_setup(int node);
void cvmx_helper_pki_show_port_config(int xipd_port);

int __cvmx_helper_pki_install_dflt_vlan(int node);
void __cvmx_helper_pki_set_dflt_ltype_map(int node);
int cvmx_helper_pki_route_dmac(int node, int style, u64 mac_addr, u64 mac_addr_mask,
			       int final_style);
int cvmx_pki_clone_style(int node, int style, u64 cluster_mask);
void cvmx_helper_pki_modify_prtgrp(int xipd_port, int grp_ok, int grp_bad);
int cvmx_helper_pki_route_prt_dmac(int xipd_port, u64 mac_addr, u64 mac_addr_mask, int grp);

void cvmx_helper_pki_errata(int node);

#endif /* __CVMX_HELPER_PKI_H__ */
