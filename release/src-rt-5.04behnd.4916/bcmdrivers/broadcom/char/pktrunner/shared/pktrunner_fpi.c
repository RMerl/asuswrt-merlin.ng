/*
<:copyright-BRCM:2023:DUAL/GPL:standard

   Copyright (c) 2023 Broadcom
   All Rights Reserved

Unless you and Broadcom execute a separate written software license
agreement governing use of this software, this software is licensed
to you under the terms of the GNU General Public License version 2
(the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
with the following added to such license:

   As a special exception, the copyright holders of this software give
   you permission to link this software with independent modules, and
   to copy and distribute the resulting executable under terms of your
   choice, provided that you also meet, for each linked independent
   module, the terms and conditions of the license of that module.
   An independent module is a module which is not derived from this
   software.  The special exception does not apply to any modifications
   of the software.

Not withstanding the above, under no circumstances may you combine
this software in any way with any other Broadcom software provided
under a license other than the GPL, without Broadcom's express prior
written consent.

:>
*/

/*
 *******************************************************************************
 * File Name : pktrunner_fpi.c
 *             Pktrunner Support for Flow Provisioning Interface
 *******************************************************************************
 */

#include <linux/module.h>
#include "rdpa_types.h"
#include "rdpa_api.h"
#include "fpi.h"
#include "fpi_hw.h"
#include "pktrunner_host_common.h"
#include <net/xfrm.h>
#include "linux/bcm_log.h"
#include "bcmspumsg.h"

#define FLOW_HW_INVALID	0xFFFFFFFF
extern int fhwPktRunnerActivate(Blog_t *blog_p, uint32_t key_in);
extern int fhwPktRunnerDeactivate(uint32_t pktrunner_key, uint32_t *pktsCnt_p,
				  uint64_t *octetsCnt_p, struct blog_t *blog_p);
extern int fhwPktRunnerRefresh(uint32_t pktRunner_Key, uint32_t *pktsCnt_p,
			       uint64_t *octetsCnt_p);

static int2int_map_t fpi2rdpa_mode_map_table[] =
{
	{fpi_mode_l2, rdpa_fpi_mode_l2},
	{fpi_mode_l3l4, rdpa_fpi_mode_l3l4},
	{-1, -1},
};

bdmf_object_handle rdpa_fpi_obj = NULL;

/*
 * Setting system FPI mode
 * Return:
 * 	0 if succeeds. otherwise, fails */
int pktrunner_fpi_set_mode(fpi_mode_t mode)
{
	rdpa_fpi_mode_t rdpa_fpi_mode;
	int rc = 0;

	rdpa_fpi_mode = int2int_map(fpi2rdpa_mode_map_table, mode, -1);

	rc = rdpa_fpi_mode_set(rdpa_fpi_obj, rdpa_fpi_mode);

	return rc? -EINVAL : 0;
}

/*
 * Getting system FPI mode
 * return:
 * 	0 if succeeds. otherwise, fails */
int pktrunner_fpi_get_mode(fpi_mode_t *mode_p)
{
	rdpa_fpi_mode_t rdpa_fpi_mode;
	int rc = 0;

	rc = rdpa_fpi_mode_get(rdpa_fpi_obj, &rdpa_fpi_mode);

	rc = rc? -EINVAL : 0;
	if (rc)
		return rc;

	*mode_p = int2int_map_r(fpi2rdpa_mode_map_table, rdpa_fpi_mode, -1);

	return rc;
}

/*
 * Setting default priority 
 * Return:
 * 	0 if succeeds. otherwise, fails */
int pktrunner_fpi_set_def_prio(uint8_t prio)
{
	int rc = 0;

	rc = rdpa_fpi_default_priority_set(rdpa_fpi_obj, (bdmf_number)prio);

	return rc? -EINVAL : 0;
}

/*
 * Getting default priority
 * return:
 * 	0 if succeeds. otherwise, fails */
int pktrunner_fpi_get_def_prio(uint8_t *prio_p)
{
	bdmf_number rdpa_prio;
	int rc = 0;

	rc = rdpa_fpi_default_priority_get(rdpa_fpi_obj, &rdpa_prio);

	rc = rc? -EINVAL : 0;
	if (rc)
		return rc;

	*prio_p = (uint8_t)rdpa_prio;
	return rc;
}

/*
 * Getting FPI system statistics
 * Return:
 * 	0 if succeeds. otherwise, fails */
int pktrunner_fpi_get_system_stat(fpi_global_stat_t *stat_p)
{
	rdpa_fpi_stat_t rdpa_fpi_stat;
	int rc = 0;

	rc = rdpa_fpi_stat_get(rdpa_fpi_obj, &rdpa_fpi_stat);
	if (rc)
		return -EINVAL;

	/* IP Frag stat */
	stat_p->ip_frag.packets_received =
			rdpa_fpi_stat.ip_frag_packets_received;
	stat_p->ip_frag.fragments_transmitted =
			rdpa_fpi_stat.ip_frag_fragments_transmitted;
	stat_p->ip_frag.no_buffer_drop = rdpa_fpi_stat.ip_frag_no_buffers_drop;

	/* IP Reassembly stat */
	stat_p->ip_reasm.fragments_received =
			rdpa_fpi_stat.ip_reasm_fragments_received;
	stat_p->ip_reasm.packets_reassembled =
			rdpa_fpi_stat.ip_reasm_packets_reassembled;
	stat_p->ip_reasm.too_many_fragments_trap =
			rdpa_fpi_stat.ip_reasm_too_many_fragments_trap;
	stat_p->ip_reasm.packet_length_too_big_trap =
			rdpa_fpi_stat.ip_reasm_packet_length_too_big_trap;
	stat_p->ip_reasm.buffer_type_mismatch_trap =
			rdpa_fpi_stat.ip_reasm_psram_ddr_mismatch_trap;
	stat_p->ip_reasm.no_buffer_trap =
			rdpa_fpi_stat.ip_reasm_no_buffers_trap;
	stat_p->ip_reasm.no_available_ctx_trap =
			rdpa_fpi_stat.ip_reasm_no_available_ctx_trap;
	stat_p->ip_reasm.flow_miss_trap =
			rdpa_fpi_stat.ip_reasm_flow_miss_trap;
	stat_p->ip_reasm.timeout_trap = rdpa_fpi_stat.ip_reasm_timeout_trap;

	return 0;
}

/*
 * Adding flow.
 * Input:
 * 	blog_p: pointer to blog
 * Output:
 * 	handle_p: memory to store returned handle
 * Return:
 * 	0 if succeeds. otherwise, fails
 */
int pktrunner_fpi_add_flow(Blog_t *blog_p, int *handle_p)
{
	*handle_p = FLOW_HW_INVALID;

	/* using pktrunner to add the blog */
	*handle_p = fhwPktRunnerActivate(blog_p, 1);

	if (*handle_p == FLOW_HW_INVALID)
		return -EKEYREJECTED;

	return 0;
}

/*
 * Deleting flow.
 * Input:
 * 	handle: handle for the flow
 * Return:
 * 	0 if succeeds. otherwise, fails
 */
int pktrunner_fpi_delete_flow(int handle)
{
	Blog_t blog = {};
	uint32_t pkt_cnt;
	uint64_t byte_cnt;
	int rc;

	blog.rx.multicast = 0;
	/* do we need to report the last stat somewhere? */
	rc = fhwPktRunnerDeactivate(handle, &pkt_cnt, &byte_cnt, &blog);
	return rc? -EINVAL : 0;
}

/*
 * Allocating IP->ID counter
 * If no more counter available, rdpa will return the default counter which
 * is usually the last counter.  In this case, also return error code, so
 * FPI can handle it differently if needed.
 * Input:
 * Output:
 * 	ipid_cntr_p: memory to store 8-bit IP->ID counter ID
 * Return:
 * 	0 if succeeds. otherwise, fails
 */
int pktrunner_fpi_alloc_ipid_cntr(uint8_t *ipid_cntr_p)
{
	int rc;

	rc = rdpa_fpi_ipid_cntr_alloc(ipid_cntr_p);
	if (rc)
		return -ENOSPC;

	return rc;
}

/*
 * Freeing IP->ID counter
 * Even if counter ID provided is default counter, user can still call the
 * API.  RDPA will make sure the default counter is not freed.
 * Input:
 * 	ipid_cntr: 8-bit IP->ID counter ID to be freed
 * Output:
 * Return:
 */
void pktrunner_fpi_free_ipid_cntr(uint8_t ipid_cntr)
{
	rdpa_fpi_ipid_cntr_free(ipid_cntr);
}

/*
 * Getting the statistics for flow
 * Input:
 * 	handle: handle for the flow
 * Output:
 * 	pkt_cnt_p: memory to store 32-bit packet counter
 * 	byte_cnt_p: memory to store 64-bit byte counter
 * Return:
 * 	0 if succeeds. otherwise, fails
 */
int pktrunner_fpi_get_flow_stat(int handle, uint32_t *pkt_cnt_p,
				uint64_t *byte_cnt_p)
{
	int rc = fhwPktRunnerRefresh(handle, pkt_cnt_p, byte_cnt_p);
	return rc? -EINVAL : 0;
}

/*
 * Adding the AP MAC address
 * Input:
 * 	mac_addr_p: pointer to 6-byte array for MAC address
 * Output:
 * Return:
 * 	0 if succeeds. otherwise, fails
 */
int pktrunner_fpi_add_ap_mac(uint8_t *mac_addr_p)
{
	int rc = pktrunner_system_add_host_mac((char *)mac_addr_p);
	if (rc == BDMF_ERR_NORES)
		return -ENOSPC;
	else if (rc != 0)
		return -EINVAL;
	else
		return 0;
}

/*
 * Deleting the AP MAC address
 * Input:
 * 	mac_addr_p: pointer to 6-byte array for MAC address
 * Output:
 * Return:
 * 	0 if succeeds. otherwise, fails
 */
int pktrunner_fpi_delete_ap_mac(uint8_t *mac_addr_p)
{
	int rc = pktrunner_system_delete_host_mac((char *)mac_addr_p);
	return rc? -EINVAL : 0;
}

/*
 * Setting GRE FPI mode
 * Return:
 * 	0 if succeeds. otherwise, fails */
int pktrunner_fpi_set_gre_mode(fpi_gre_mode_t mode)
{
	int rc = 0;

	rc = rdpa_fpi_special_gre_set(rdpa_fpi_obj, mode);

	return rc? -EINVAL : 0;
}

/*
 * Setting L2 lookup based on EtherType
 * Return:
 * 	0 if succeeds. otherwise, fails */
int pktrunner_fpi_set_l2lkp_on_etype(bool enable, uint16_t etype)
{
	int rc = 0;

	rc = rdpa_fpi_l2lkp_on_ethertype_set(rdpa_fpi_obj, (bdmf_boolean)enable);
	rc = rc? rc: rdpa_fpi_ethertype_for_l2lkp_set(rdpa_fpi_obj, etype);

	return rc? -EINVAL : 0;
}

/*
 * Setting enable HW lookup for FPI
 * Return:
 * 	0 if succeeds. otherwise, fails */
int pktrunner_fpi_set_lkp_enable(bool enable)
{
	int rc = 0;

	rc = rdpa_fpi_lkp_enable_set(rdpa_fpi_obj, (bdmf_boolean)enable);

	return rc? -EINVAL : 0;
}

/*
 * Setting enable IP fragmentation for FPI
 * Return:
 * 	0 if succeeds. otherwise, fails */
int pktrunner_fpi_set_ip_frag_enable(bool enable)
{
	int rc = 0;

	rc = rdpa_fpi_ip_frag_enable_set(rdpa_fpi_obj, (bdmf_boolean)enable);

	return rc? -EINVAL : 0;
}

/*
 * Setting enable IP reassembly for FPI
 * Return:
 * 	0 if succeeds. otherwise, fails */
int pktrunner_fpi_set_ip_reasm_enable(bool enable)
{
	int rc = 0;

	rc = rdpa_fpi_ip_reasm_enable_set(rdpa_fpi_obj, (bdmf_boolean)enable);

	return rc? -EINVAL : 0;
}

/*
 * Adding IPsec tunnel
 * Return:
 * 	0 if succeeds. otherwise, fails */
int pktrunner_fpi_add_ipsec_tunnel(struct xfrm_state *x, uint32_t tun_id, uint8_t is_us)
{
	int rc = -1;
	bcmFun_t *spu_add_tunnel = bcmFun_get(BCM_FUN_ID_SPU_FPI_TUNNEL_CREATE);

	if (spu_add_tunnel)
	{
		struct spu_fpi_tunnel_args a = {};
		a.add.tun_id = tun_id;
		a.add.x = x;
		a.add.is_us = is_us;
		rc = spu_add_tunnel(&a);
	}
	return rc;
}

/*
 * Delete IPsec tunnel
 * Return:
 * 	0 if succeeds. otherwise, fails */
int pktrunner_fpi_del_ipsec_tunnel(uint32_t tun_id, uint8_t is_us, uint32_t esp_spi)
{
	int rc = -1;
	bcmFun_t *spu_del_tunnel = bcmFun_get(BCM_FUN_ID_SPU_FPI_TUNNEL_DELETE);

	if (spu_del_tunnel)
	{
		struct spu_fpi_tunnel_args a = {};
		a.del.tun_id = tun_id;
		a.del.is_us = is_us;
		a.del.esp_spi = esp_spi;
		rc = spu_del_tunnel(&a);
	}
	
	return rc;
}

/*
 * Remap IPsec tunnel
 * Return:
 * 	0 if succeeds. otherwise, fails */
int pktrunner_fpi_remap_ipsec_tunnel(uint32_t tun_id1, uint32_t esp_spi1,
				     uint32_t tun_id2, uint32_t esp_spi2)
{
	int rc = -1;
	bcmFun_t *spu_remap_tunnel = bcmFun_get(BCM_FUN_ID_SPU_FPI_TUNNEL_REMAP);

	if (spu_remap_tunnel)
	{
		struct spu_fpi_tunnel_args a = {};
		a.remap.tun_id1 = tun_id1;
		a.remap.esp_spi1 = esp_spi1;
		a.remap.tun_id2 = tun_id2;
		a.remap.esp_spi2 = esp_spi2;
		rc = spu_remap_tunnel(&a);
	}
	return rc;
}

static const fpi_hw_info_t hw_info_db = {
	.set_mode = pktrunner_fpi_set_mode,
	.get_mode = pktrunner_fpi_get_mode,
	.set_default_priority = pktrunner_fpi_set_def_prio,
	.get_default_priority = pktrunner_fpi_get_def_prio,
	.get_system_stat = pktrunner_fpi_get_system_stat,
	.add_flow = pktrunner_fpi_add_flow,
	.delete_flow = pktrunner_fpi_delete_flow,
	.delete_flow_by_blog = NULL,
	.get_flow_stat = pktrunner_fpi_get_flow_stat,
	.alloc_ipid_cntr = pktrunner_fpi_alloc_ipid_cntr,
	.free_ipid_cntr = pktrunner_fpi_free_ipid_cntr,
	.add_ap_mac = pktrunner_fpi_add_ap_mac,
	.delete_ap_mac = pktrunner_fpi_delete_ap_mac,
	.set_gre_mode = pktrunner_fpi_set_gre_mode,
	.set_l2lkp_on_etype = pktrunner_fpi_set_l2lkp_on_etype,
	.set_lkp_enable = pktrunner_fpi_set_lkp_enable,
	.set_ip_frag_enable = pktrunner_fpi_set_ip_frag_enable,
	.set_ip_reasm_enable = pktrunner_fpi_set_ip_reasm_enable,
	.add_ipsec_tunnel = pktrunner_fpi_add_ipsec_tunnel,
	.del_ipsec_tunnel = pktrunner_fpi_del_ipsec_tunnel,
	.remap_ipsec_tunnel = pktrunner_fpi_remap_ipsec_tunnel,
};

int pktrunner_fpi_init(void)
{
	BDMF_MATTR_ALLOC(fpi_attr, rdpa_fpi_drv());
	int rc = 0;

	rc = bdmf_new_and_set(rdpa_fpi_drv(), NULL, fpi_attr, &rdpa_fpi_obj);
	if (rc) {
		pr_err("Fail to add RDPA FPI object\n");
		BDMF_MATTR_FREE(fpi_attr);
		return rc;
	}

	rc = rdpa_fpi_ipv6_exthdr_force_trap_set(rdpa_fpi_obj,
		(bdmf_boolean)true);
	if (rc) {
		pr_err("Fail to enable IPv6 exthdr force trap\n");
		bdmf_destroy(rdpa_fpi_obj);
		BDMF_MATTR_FREE(fpi_attr);
		return rc;
	}

	rc = fpi_register_hw(&hw_info_db);
	if (rc)
		pr_err("%s:%d:Fail to register PktRunner to Flow "
		       "Provisioning API Driver!\n", __func__, __LINE__);

	pr_info("Successfully register PktRunner to Flow Provisioning "
		"API Driver\n");

	return rc;
}

void pktrunner_fpi_exit(void)
{
	fpi_unregister_hw((fpi_hw_info_t *)&hw_info_db);
	bdmf_destroy(rdpa_fpi_obj);
}

