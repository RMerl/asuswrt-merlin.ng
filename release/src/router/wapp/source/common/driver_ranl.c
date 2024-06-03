/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * Copyright  (C) 2019-2020  MediaTek Inc. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <net/if.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/family.h>
#include <netlink/genl/ctrl.h>
#include <pthread.h>
#include "hotspot.h"
#include "rt_nl_copy.h"
#include "driver_ranl.h"

#define CONFIG_LIBNL20

static int ack_handler(struct nl_msg *msg, void *arg)
{
    int *err = arg; 

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
    *err = 0; 
    return NL_STOP;
}

static int finish_handler(struct nl_msg *msg, void *arg)
{
    int *ret = arg; 

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
    *ret = 0; 
    return NL_SKIP;
}

static int error_handler(struct sockaddr_nl *nla, struct nlmsgerr *err,
             void *arg)
{
    int *ret = arg; 

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
    *ret = err->error;
    return NL_SKIP;
}

static int send_and_recv(struct driver_ranl_data *drv,
             struct nl_handle *nl_handle, struct nl_msg *msg,
             int (*valid_handler)(struct nl_msg *, void *),
             void *valid_data)
{
    struct nl_cb *cb;
    int err = -ENOMEM;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
    cb = nl_cb_clone(drv->nl_cb);
    if (!cb)
        goto out;

    err = nl_send_auto_complete(nl_handle, msg);
    if (err < 0)
        goto out;

    err = 1;

    nl_cb_err(cb, NL_CB_CUSTOM, error_handler, &err);
    nl_cb_set(cb, NL_CB_FINISH, NL_CB_CUSTOM, finish_handler, &err);
    nl_cb_set(cb, NL_CB_ACK, NL_CB_CUSTOM, ack_handler, &err);

    if (valid_handler)
        nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM,
              valid_handler, valid_data);

    while (err > 0)
        nl_recvmsgs(nl_handle, cb);
 out:
    nl_cb_put(cb);
    nlmsg_free(msg);
    return err;
}

static int send_and_recv_msgs(struct driver_ranl_data *drv,
                  struct nl_msg *msg,
                  int (*valid_handler)(struct nl_msg *, void *),
                  void *valid_data)
{
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
    return send_and_recv(drv, drv->nl_handle, msg, valid_handler,
                 valid_data);
}


struct family_data {
    const char *group;
    int id;
};

static int family_handler(struct nl_msg *msg, void *arg)
{
    struct family_data *res = arg;
    struct nlattr *tb[CTRL_ATTR_MAX + 1];
    struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
    struct nlattr *mcgrp;
    int i;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
    nla_parse(tb, CTRL_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
          genlmsg_attrlen(gnlh, 0), NULL);
    if (!tb[CTRL_ATTR_MCAST_GROUPS])
        return NL_SKIP;

    nla_for_each_nested(mcgrp, tb[CTRL_ATTR_MCAST_GROUPS], i) {
        struct nlattr *tb2[CTRL_ATTR_MCAST_GRP_MAX + 1];
        nla_parse(tb2, CTRL_ATTR_MCAST_GRP_MAX, nla_data(mcgrp),
              nla_len(mcgrp), NULL);
        if (!tb2[CTRL_ATTR_MCAST_GRP_NAME] ||
            !tb2[CTRL_ATTR_MCAST_GRP_ID] ||
            strncmp(nla_data(tb2[CTRL_ATTR_MCAST_GRP_NAME]),
                   res->group,
                   nla_len(tb2[CTRL_ATTR_MCAST_GRP_NAME])) != 0)
            continue;
        res->id = nla_get_u32(tb2[CTRL_ATTR_MCAST_GRP_ID]);
        break;
    };

    return NL_SKIP;
}

static int nl_get_multicast_id(struct driver_ranl_data *drv,
                   const char *family, const char *group)
{
    struct nl_msg *msg;
    int ret = -1;
    struct family_data res = { group, -ENOENT };

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
    msg = nlmsg_alloc();
    if (!msg)
        return -ENOMEM;
    genlmsg_put(msg, 0, 0, genl_ctrl_resolve(drv->nl_handle, "nlctrl"),
            0, 0, CTRL_CMD_GETFAMILY, 0);
    NLA_PUT_STRING(msg, CTRL_ATTR_FAMILY_NAME, family);

    ret = send_and_recv_msgs(drv, msg, family_handler, &res);
    msg = NULL;
    if (ret == 0)
        ret = res.id;

nla_put_failure:
    nlmsg_free(msg);
    return ret;
}

static int no_seq_check(struct nl_msg *msg, void *arg)
{
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	return NL_OK;
}

static int event_anqp_req(struct driver_ranl_data *drv_data, struct nl_msg *msg)
{
	struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
	struct nlattr *tb[RANL_ATTR_MAX + 1];
	char ifname[IFNAMSIZ];
	int ret;
	struct hotspot *hs = (struct hotspot *)drv_data->priv;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	nla_parse(tb, RANL_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
			genlmsg_attrlen(gnlh, 0), NULL);

	if (!tb[RANL_ATTR_IFINDEX]) {
		DBGPRINT(RT_DEBUG_ERROR, "%s:RANL_ATTR_IFINDEX missing\n", __FUNCTION__);
		return NL_SKIP;
	}

	if (!tb[RANL_ATTR_PEER_MAC_ADDR]) {
		DBGPRINT(RT_DEBUG_ERROR, "%s:RANL_ATTR_PEER_MAC_ADDR missing\n", __FUNCTION__);
		return NL_SKIP;
	}

	if (!tb[RANL_ATTR_ANQP_DATA]) {
		DBGPRINT(RT_DEBUG_ERROR, "%s:RANL_ATTR_ANQP_DATA\n", __FUNCTION__);
		return NL_SKIP;
	}

	if_indextoname(nla_get_u32(tb[RANL_ATTR_IFINDEX]), ifname);

	ret = hs->event_ops->event_anqp_req(hs,
							 		    ifname,						  
							 		    nla_data(tb[RANL_ATTR_PEER_MAC_ADDR]),
							 		    nla_data(tb[RANL_ATTR_ANQP_DATA]),	
							 		    nla_len(tb[RANL_ATTR_ANQP_DATA]));
	if (ret)
		return NL_SKIP;

	return NL_OK;
}

static int event_anqp_rsp(struct driver_ranl_data *drv_data, struct nl_msg *msg)
{

	struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
	struct nlattr *tb[RANL_ATTR_MAX + 1];
	char ifname[IFNAMSIZ];
	int ret;
	struct hotspot *hs = (struct hotspot *)drv_data->priv;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	nla_parse(tb, RANL_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
			genlmsg_attrlen(gnlh, 0), NULL);
	
	if (!tb[RANL_ATTR_IFINDEX]) {
		DBGPRINT(RT_DEBUG_ERROR, "%s:RANL_ATTR_IFINDEX missing\n", __FUNCTION__);
		return NL_SKIP;
	}

	if (!tb[RANL_ATTR_PEER_MAC_ADDR]) {
		DBGPRINT(RT_DEBUG_ERROR, "%s:RANL_ATTR_PEER_MAC_ADDR missing\n", __FUNCTION__);
		return NL_SKIP;
	}

	if (!tb[RANL_ATTR_STATUS]) {
		DBGPRINT(RT_DEBUG_ERROR, "%s:RANL_ATTR_STATUS missing\n", __FUNCTION__);
		return NL_SKIP;
	}

	if (!tb[RANL_ATTR_ANQP_DATA]) {
		DBGPRINT(RT_DEBUG_ERROR, "%s:RANL_ATTR_ANQP_DATA\n", __FUNCTION__);
		return NL_SKIP;
	}

	if_indextoname(nla_get_u32(tb[RANL_ATTR_IFINDEX]), ifname);
	
	ret = hs->event_ops->event_anqp_rsp(hs,
							 		    ifname,						  
							 		    nla_data(tb[RANL_ATTR_PEER_MAC_ADDR]),
							 		    nla_get_u16(tb[RANL_ATTR_STATUS]),
							 		    nla_data(tb[RANL_ATTR_ANQP_DATA]),	
							 		    nla_len(tb[RANL_ATTR_ANQP_DATA]));
	if (ret)
		return NL_SKIP;
}

static int event_test(struct driver_ranl_data *drv_data, struct nl_msg *msg)
{
	int ret;
	struct hotspot *hs = (struct hotspot *)drv_data->priv;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	ret = hs->event_ops->event_test(hs);

	if (ret)
		return NL_SKIP;

	return NL_OK;
}

static int event_hs_onoff(struct driver_ranl_data *drv_data, struct nl_msg *msg)
{
	struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
	struct nlattr *tb[RANL_ATTR_MAX + 1];
	char ifname[IFNAMSIZ];
	int ret;
	struct hotspot *hs = (struct hotspot *)drv_data->priv;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	nla_parse(tb, RANL_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
			genlmsg_attrlen(gnlh, 0), NULL);
	
	if (!tb[RANL_ATTR_IFINDEX]) {
		DBGPRINT(RT_DEBUG_ERROR, "%s:RANL_ATTR_IFINDEX missing\n", __FUNCTION__);
		return NL_SKIP;
	}


	if(!tb[RANL_ATTR_HOTSPOT_ONOFF]) {
		DBGPRINT(RT_DEBUG_ERROR, "%s:RANL_ATTR_HOTSPOT_ONOFF missing\n", __FUNCTION__);
		return NL_SKIP;
	}

	if_indextoname(nla_get_u32(tb[RANL_ATTR_IFINDEX]), ifname);
	
	ret = hs->event_ops->event_hs_onoff(hs,
							 		    ifname,
							 		    nla_get_u8(tb[RANL_ATTR_HOTSPOT_ONOFF]));

	if (ret)
		return NL_SKIP;

	return NL_OK;
}

static int event_handler(struct nl_msg *msg, void *arg)
{
	struct driver_ranl_data *drv_data = (struct driver_ranl_data *)arg;
	struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
	int ret = NL_OK;	

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	switch (gnlh->cmd) {
	case RANL_CMD_ANQP_REQ:
		ret = event_anqp_req(drv_data, msg);
		break;
	case RANL_CMD_ANQP_RSP:
		ret = event_anqp_rsp(drv_data, msg);
		break;
	case RANL_CMD_TEST:
		ret = event_test(drv_data, msg);
		break;
	case RANL_CMD_HOTSPOT_ONOFF:
		ret = event_hs_onoff(drv_data, msg);
		break;
	default:
		DBGPRINT(RT_DEBUG_ERROR, "%s: Unknown event cmd=%d\n", __FUNCTION__, gnlh->cmd);
		break;
	}	

	return ret;
}

static void ranl_event_receive(int sock, void *eloop_ctx, void *sock_ctx)
{
	struct nl_cb *cb;
	struct driver_ranl_data *drv_data = (struct driver_ranl_data *)eloop_ctx;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	cb = nl_cb_clone(drv_data->nl_cb);

	if (!cb)
		return;
	
	nl_cb_set(cb, NL_CB_SEQ_CHECK, NL_CB_CUSTOM, no_seq_check, NULL);
	nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM, event_handler, drv_data);

	nl_recvmsgs(drv_data->nl_handle_event, cb);

	nl_cb_put(cb);

	return;
}


static void *driver_ranl_init(struct hotspot *hs, 
						     const int opmode,
							 const int drv_mode)
{
	int ret;
	struct driver_ranl_data *nldrv_data;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	nldrv_data = calloc(1, sizeof(*nldrv_data));
	if (!nldrv_data) {
		DBGPRINT(RT_DEBUG_ERROR, "No avaliable memory for driver_ranl_data\n");
		goto err1;

	}

	printf("Initialize ralink netlink interface\n");
	
	nldrv_data->nl_cb = nl_cb_alloc(NL_CB_DEFAULT);
	if (!nldrv_data->nl_cb) {
		DBGPRINT(RT_DEBUG_ERROR, "failed to allocate netlink default callbacks\n");
		goto err2;
	}

	nldrv_data->nl_handle = nl_handle_alloc_cb(nldrv_data->nl_cb);
	if (!nldrv_data->nl_handle) {
		DBGPRINT(RT_DEBUG_ERROR, "failed to allocate netlink socket for nl_handle\n");
		goto err3;
	}

	nldrv_data->nl_handle_event = nl_handle_alloc_cb(nldrv_data->nl_cb);
	if (!nldrv_data->nl_handle) {
		DBGPRINT(RT_DEBUG_ERROR, "failed to allocate netlink socket for nl_handle_event\n");
		goto err4;
	}

	if (genl_connect(nldrv_data->nl_handle)) {
		DBGPRINT(RT_DEBUG_ERROR, "failed to connect generic netlink for nl_handle\n");
		goto err5;
	}

	if (genl_connect(nldrv_data->nl_handle_event)) {
		DBGPRINT(RT_DEBUG_ERROR, "failed to connect generic netlink for nl_handle_event\n");
		goto err5;
	}

#ifdef CONFIG_LIBNL20
	if (genl_ctrl_alloc_cache(nldrv_data->nl_handle, &nldrv_data->nl_cache) < 0) {
		DBGPRINT(RT_DEBUG_ERROR, "failed to allocate generic netlink cache for nl_handle\n");
		goto err5;
	}

	if (genl_ctrl_alloc_cache(nldrv_data->nl_handle_event, &nldrv_data->nl_cache_event) < 0) {
		DBGPRINT(RT_DEBUG_ERROR, "failed to allocate generic netlink cache for nl_handle_event\n");
		goto err6;
	}
#else
	nldrv_data->nl_cache = genl_ctrl_alloc_cache(nldrv_data->nl_handle);
	if (!nldrv_data->nl_cache) {
		DBGPRINT(RT_DEBUG_ERROR, "failed to allocate generic netlink cache for nl_handle\n");
		goto err5;
	}
	
	nldrv_data->nl_cache_event = genl_ctrl_alloc_cache(nldrv_data->nl_handle_event);
	if (!nldrv_data->nl_cache) {
		DBGPRINT(RT_DEBUG_ERROR, "failed to allocate generic netlink cache for nl_handle_event\n");
		goto err6;
	}
#endif /* CONFIG_LIBNL20 */

	nldrv_data->ranl = genl_ctrl_search_by_name(nldrv_data->nl_cache, "ranl");
	if (!nldrv_data->ranl) {
		DBGPRINT(RT_DEBUG_ERROR, " 'ranl' generic netlink family not found\n");
		goto err7;
	}

	ret = nl_get_multicast_id(nldrv_data, "ranl", "hotspot");

	if (ret >= 0) 
		ret = nl_socket_add_membership(nldrv_data->nl_handle_event, ret);
	
	if (ret < 0) {
		DBGPRINT(RT_DEBUG_ERROR, "Can not add multicast membership for hotspot events\n");
		goto err7;
	}

	nldrv_data->opmode = opmode;

	nldrv_data->drv_mode = drv_mode;

	nldrv_data->priv = (void *)hs;
	
	eloop_register_read_sock(nl_socket_get_fd(nldrv_data->nl_handle_event), 
								ranl_event_receive, nldrv_data, NULL);

	return (void *)nldrv_data;

err7:
	nl_cache_free(nldrv_data->nl_cache_event);
err6:
	nl_cache_free(nldrv_data->nl_cache);
err5:
	nl_handle_destroy(nldrv_data->nl_handle_event);
err4:
	nl_handle_destroy(nldrv_data->nl_handle);
err3:
	nl_cb_put(nldrv_data->nl_cb);
err2:
	os_free(nldrv_data);
err1:
	return NULL;

}

static int driver_ranl_exit(struct hotspot *hs)
{
	struct driver_ranl_data *nldrv_data = (struct driver_ranl_data *)hs->drv_data;
	int ret = -1;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	genl_family_put(nldrv_data->ranl);
	nl_cache_free(nldrv_data->nl_cache_event);
	nl_cache_free(nldrv_data->nl_cache);
	nl_handle_destroy(nldrv_data->nl_handle_event);
	nl_handle_destroy(nldrv_data->nl_handle);
	nl_cb_put(nldrv_data->nl_cb);
	eloop_unregister_read_sock(nl_socket_get_fd(nldrv_data->nl_handle_event));
	os_free(nldrv_data);
	
	return 0;
}

/*
 * Set information element to driver 
 */
static int driver_ranl_set_ie(void *drv_data, const char *ifname, const char *ie, 
				       size_t ie_len)
{
	struct driver_ranl_data *nldrv_data = (struct driver_ranl_data *)drv_data;
	struct nl_msg *msg;
	int ret = -1;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);
	
	msg = nlmsg_alloc();

	if (!msg)
		return -1;
	
	genlmsg_put(msg, 0, 0, genl_family_get_id(nldrv_data->ranl), 0, 0,
				RANL_CMD_IE, 0);

	NLA_PUT_U32(msg, RANL_ATTR_IFINDEX, if_nametoindex(ifname));
	NLA_PUT_U32(msg, RANL_ATTR_OPMODE, nldrv_data->opmode);
	NLA_PUT(msg, RANL_ATTR_IE, ie_len, ie);

	ret = send_and_recv_msgs(nldrv_data, msg, NULL, NULL);
	msg = NULL;

	if (ret)
		DBGPRINT(RT_DEBUG_ERROR, "%s fail\n", __FUNCTION__);

	return ret;

nla_put_failure:
	nlmsg_free(msg);
	return -1;
}

/*
 * Send ANQP request to driver
 */
static int driver_ranl_send_anqp_req(void *drv_data, const char *ifname, 
							  const char *peer_mac_addr, const char *anqp_req, 
							  size_t anqp_req_len)
{
	struct driver_ranl_data *nldrv_data = (struct driver_ranl_data *)drv_data;
	struct nl_msg *msg;
	int ret = -1;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);
	
	msg = nlmsg_alloc();

	if (!msg) {
		DBGPRINT(RT_DEBUG_ERROR,"%s Not available memory\n", __FUNCTION__);
		return ret;
	}
	
	genlmsg_put(msg, 0, 0, genl_family_get_id(nldrv_data->ranl), 0, 0,
				RANL_CMD_ANQP_REQ, 0);

	NLA_PUT_U32(msg, RANL_ATTR_IFINDEX, if_nametoindex(ifname));
	NLA_PUT_U32(msg, RANL_ATTR_OPMODE, nldrv_data->opmode);
	NLA_PUT(msg, RANL_ATTR_PEER_MAC_ADDR, 6, peer_mac_addr);
	NLA_PUT(msg, RANL_ATTR_ANQP_DATA, anqp_req_len, anqp_req);

	ret = send_and_recv_msgs(nldrv_data, msg, NULL, NULL);
	msg = NULL;

	if (ret) {
		DBGPRINT(RT_DEBUG_ERROR, "%s fail\n", __FUNCTION__);
	}

	return ret;

nla_put_failure:
	nlmsg_free(msg);
	return -1;
}

/*
 * Send ANQP response to driver
 */
static int driver_ranl_send_anqp_rsp(void *drv_data, const char *ifname, 
							  const char *peer_mac_addr, const char *anqp_rsp, 
							  size_t anqp_rsp_len)
{
	struct driver_ranl_data *nldrv_data = (struct driver_ranl_data *)drv_data;
	struct nl_msg *msg;
	int ret = -1;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);
	
	msg = nlmsg_alloc();

	if (!msg) {
		DBGPRINT(RT_DEBUG_ERROR, "%s Not available memory\n", __FUNCTION__);
		return ret;
	}
		
	genlmsg_put(msg, 0, 0, genl_family_get_id(nldrv_data->ranl), 0, 0,
				RANL_CMD_ANQP_RSP, 0);

	NLA_PUT_U32(msg, RANL_ATTR_IFINDEX, if_nametoindex(ifname));
	NLA_PUT_U32(msg, RANL_ATTR_OPMODE, nldrv_data->opmode);
	NLA_PUT(msg, RANL_ATTR_PEER_MAC_ADDR, 6, peer_mac_addr);
	NLA_PUT(msg, RANL_ATTR_ANQP_DATA, anqp_rsp_len, anqp_rsp);

	ret = send_and_recv_msgs(nldrv_data, msg, NULL, NULL);
	msg = NULL;
	
	if (ret) {
		DBGPRINT(RT_DEBUG_ERROR, "%s fail\n", __FUNCTION__);
		goto nla_put_failure;
	}
		
	ret = 0;

nla_put_failure:
	nlmsg_free(msg);
	return ret;
}

/*
 * Test Hotspot cmd
 */
static int driver_ranl_test(void *drv_data, const char *ifname)
{
	struct driver_ranl_data *nldrv_data = (struct driver_ranl_data *)drv_data;
	struct nl_msg *msg;
	int ret = -1;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);

	msg = nlmsg_alloc();

	if (!msg) {
		DBGPRINT(RT_DEBUG_ERROR, "%s Not available memory\n", __FUNCTION__);
		return ret;
	}

	genlmsg_put(msg, 0, 0, genl_family_get_id(nldrv_data->ranl), 0, 0,
				RANL_CMD_TEST, 0);

	NLA_PUT_U32(msg, RANL_ATTR_IFINDEX, if_nametoindex(ifname));
	NLA_PUT_U32(msg, RANL_ATTR_OPMODE, nldrv_data->opmode);
	NLA_PUT_U32(msg, RANL_ATTR_TEST, 100);

	ret = send_and_recv_msgs(nldrv_data, msg, NULL, NULL);
	msg = NULL;

	if (ret) 
		DBGPRINT(RT_DEBUG_ERROR, "%s fail\n", __FUNCTION__);
	
	return ret;
	
nla_put_failure:
	nlmsg_free(msg);
	return -1;
}


/*
 * Enable/Disable hotspot feature
 */
static int driver_ranl_hotspot_onoff(void *drv_data, const char *ifname, 
					                 u8 enable)
{
	struct driver_ranl_data *nldrv_data = (struct driver_ranl_data *)drv_data;
	struct nl_msg *msg;
	int ret = -1;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);

	msg = nlmsg_alloc();
	
	if(!msg) {
		DBGPRINT(RT_DEBUG_ERROR, "%s Not available memory\n", __FUNCTION__);
		return ret;
	}

	genlmsg_put(msg, 0, 0, genl_family_get_id(nldrv_data->ranl), 0, 0,
			    RANL_CMD_HOTSPOT_ONOFF, 0);

	NLA_PUT_U32(msg, RANL_ATTR_IFINDEX, if_nametoindex(ifname));
	NLA_PUT_U32(msg, RANL_ATTR_OPMODE, nldrv_data->opmode);
	NLA_PUT_U8(msg, RANL_ATTR_HOTSPOT_IPCTYPE, nldrv_data->drv_mode);
	NLA_PUT_U8(msg, RANL_ATTR_HOTSPOT_ONOFF, enable);

	ret = send_and_recv_msgs(nldrv_data, msg, NULL, NULL);
	msg  = NULL;

	if (ret)
		DBGPRINT(RT_DEBUG_ERROR, "%s fail\n", __FUNCTION__);

	return ret;

nla_put_failure:
	nlmsg_free(msg);
	return -1;
}

const struct hotspot_drv_ops hotspot_drv_ranl_ops = {
	.drv_inf_init = driver_ranl_init,
	.drv_inf_exit = driver_ranl_exit,
	.drv_test = driver_ranl_test,
	.drv_set_ie = driver_ranl_set_ie,
	.drv_send_anqp_req = driver_ranl_send_anqp_req,
	.drv_send_anqp_rsp = driver_ranl_send_anqp_rsp,
	.drv_hotspot_onoff = driver_ranl_hotspot_onoff,
};
