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

#define FLOW_HW_INVALID	0xFFFFFFFF
extern int fhwPktRunnerActivate(Blog_t *blog_p, uint32_t key_in);
extern int fhwPktRunnerDeactivate(uint32_t pktrunner_key, uint32_t *pktsCnt_p,
				  uint64_t *octetsCnt_p, struct blog_t *blog_p);
extern int fhwPktRunnerRefresh(uint32_t pktRunner_Key, uint32_t *pktsCnt_p,
			       uint64_t *octetsCnt_p);

static int2int_map_t fpi2rdpa_mode_map_table[] =
{
	{fpi_mode_l2_bridge, rdpa_fpi_mode_l2_bridge},
	{fpi_mode_l2, rdpa_fpi_mode_l2},
	{fpi_mode_l3l4, rdpa_fpi_mode_l3l4},
	{fpi_mode_fallback, rdpa_fpi_mode_fallback},
	{-1, -1},
};

/*
 * Setting system FPI mode
 * Return:
 * 	0 if succeeds. otherwise, fails */
int pktrunner_fpi_set_mode(fpi_mode_t mode)
{
	bdmf_object_handle rdpa_sys_obj = NULL;
	rdpa_fpi_mode_t rdpa_fpi_mode;
	int rc = 0;

	rdpa_fpi_mode = int2int_map(fpi2rdpa_mode_map_table, mode, -1);

	rdpa_system_get(&rdpa_sys_obj);
	rc = rdpa_system_fpi_mode_set(rdpa_sys_obj, rdpa_fpi_mode);
	bdmf_put(rdpa_sys_obj);

	return rc;
}

/*
 * Getting system FPI mode
 * return:
 * 	0 if succeeds. otherwise, fails */
int pktrunner_fpi_get_mode(fpi_mode_t *mode_p)
{
	bdmf_object_handle rdpa_sys_obj = NULL;
	rdpa_fpi_mode_t rdpa_fpi_mode;
	int rc = 0;

	rdpa_system_get(&rdpa_sys_obj);
	rc = rdpa_system_fpi_mode_get(rdpa_sys_obj, &rdpa_fpi_mode);
	bdmf_put(rdpa_sys_obj);

	if (rc)
		return rc;

	*mode_p = int2int_map_r(fpi2rdpa_mode_map_table, rdpa_fpi_mode, -1);

	return rc;
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
		return -1;	/* TODO! fix the return error code */

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
	Blog_t blog;
	uint32_t pkt_cnt;
	uint64_t byte_cnt;

	blog.rx.multicast = 0;
	/* do we need to report the last stat somewhere? */
	return fhwPktRunnerDeactivate(handle, &pkt_cnt, &byte_cnt, &blog);
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
int pktrunner_fpi_get_stat(int handle, uint32_t *pkt_cnt_p,
			   uint64_t *byte_cnt_p)
{
	return fhwPktRunnerRefresh(handle, pkt_cnt_p, byte_cnt_p);
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
	return pktrunner_system_add_host_mac((char *)mac_addr_p);
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
	return pktrunner_system_delete_host_mac((char *)mac_addr_p);
}

static const fpi_hw_info_t hw_info_db = {
	.set_mode = pktrunner_fpi_set_mode,
	.get_mode = pktrunner_fpi_get_mode,
	.add_flow = pktrunner_fpi_add_flow,
	.delete_flow = pktrunner_fpi_delete_flow,
	.delete_flow_by_blog = NULL,
	.get_stat = pktrunner_fpi_get_stat,
	.add_ap_mac = pktrunner_fpi_add_ap_mac,
	.delete_ap_mac = pktrunner_fpi_delete_ap_mac,
};

int __init pktrunner_fpi_init(void)
{
	int rc = 0;

	rc = fpi_register_hw(&hw_info_db);
	if (rc)
		pr_err("%s:%d:fail to register PktRunner to Flow "
		       "Provisioning API Driver!\n", __func__, __LINE__);
	pr_info("Successfully register PktRunner to Flow Provisioning "
		"API Driver\n");
	return rc;
}

void __exit pktrunner_fpi_exit(void)
{
	fpi_unregister_hw((fpi_hw_info_t *)&hw_info_db);
}

