/*************************************************
* <:copyright-BRCM:2013:proprietary:standard
*
*    Copyright (c) 2013 Broadcom
*    All Rights Reserved
*
*  This program is the proprietary software of Broadcom and/or its
*  licensors, and may only be used, duplicated, modified or distributed pursuant
*  to the terms and conditions of a separate, written license agreement executed
*  between you and Broadcom (an "Authorized License").  Except as set forth in
*  an Authorized License, Broadcom grants no license (express or implied), right
*  to use, or waiver of any kind with respect to the Software, and Broadcom
*  expressly reserves all rights in and to the Software and all intellectual
*  property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
*  NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
*  BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
*  Except as expressly set forth in the Authorized License,
*
*  1. This program, including its structure, sequence and organization,
*     constitutes the valuable trade secrets of Broadcom, and you shall use
*     all reasonable efforts to protect the confidentiality thereof, and to
*     use this information only in connection with your use of Broadcom
*     integrated circuit products.
*
*  2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*     AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*     WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
*     RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
*     ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
*     FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
*     COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
*     TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
*     PERFORMANCE OF THE SOFTWARE.
*
*  3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
*     ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
*     INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
*     WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
*     IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
*     OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
*     SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
*     SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
*     LIMITED REMEDY.
:>
*/

#include <linux/socket.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/rtnetlink.h>
#include <linux/netlink.h>
#include <net/sock.h>
#include <linux/in.h>
#include <linux/if.h>
#include <linux/if_vlan.h>
#include <linux/rbtree.h>
#include <wlcsm_linux.h>
#include <wlcsm_nvram.h>
#include <linux/mutex.h>

#ifdef WLCSM_DEBUG
//unsigned int g_WLCSM_TRACE_LEVEL=WLCSM_TRACE_DBG|WLCSM_TRACE_PKT;           /**< Debug time trace level setting value */
unsigned int g_WLCSM_TRACE_LEVEL=0;           /**< Debug time trace level setting value */
#endif

static DEFINE_SPINLOCK(nvram_spinlock);              /**< spinlock for nvram read/write protection */
struct rb_root wlcsm_nvram_tree= RB_ROOT;       /**< redblack binary tree root for nvram index */

#define WLCSM_NVRAM_LOCK() spin_lock_bh(&nvram_spinlock);
#define WLCSM_NVRAM_UNLOCK() spin_unlock_bh(&nvram_spinlock);

static void _valuepair_set(t_WLCSM_NAME_VALUEPAIR *v,char *name,char *value,int len)
{
    t_WLCSM_NAME_VALUEPAIR *vp=v;
    /*  init to 0 */
    memset((void *)vp,0,_get_valuepair_total_len(name,value,len));
    /*  set name first */
    vp->len=strlen(name)+1;
    strcpy(vp->value,name);

    /*  move pointer to  */
    vp = _get_valuepair_value(v);
    if(value) {
        vp->len=(len?len:(strlen(value)+1));
        memcpy(&(vp->value),value,vp->len);
    } else
        vp->len=0;
}

static t_WLCSM_NVRAM_TUPLE *_wlcsm_nvram_tuple_search(char *name)
{
    struct rb_node *node = wlcsm_nvram_tree.rb_node;
    int result;

    while (node) {
        t_WLCSM_NVRAM_TUPLE *data = container_of(node, t_WLCSM_NVRAM_TUPLE, node);

        result = strcmp(name, data->tuple->value);

        if (result < 0)
            node = node->rb_left;
        else if (result > 0)
            node = node->rb_right;
        else
            return data;
    }
    return NULL;
}

static int _wlcsm_nvram_tuple_insert(char *buf, int len)
{
    struct rb_node **new = &(wlcsm_nvram_tree.rb_node), *parent = NULL;
    int result=0;
    t_WLCSM_NVRAM_TUPLE *data=(t_WLCSM_NVRAM_TUPLE *)kmalloc(sizeof(t_WLCSM_NVRAM_TUPLE),GFP_ATOMIC);
    if(data) {
        data->tuple=(t_WLCSM_NAME_VALUEPAIR *)kmalloc(len,GFP_ATOMIC);
        if(!(data->tuple)) {
            printk("nvram:%s did not set correctly due to no mem\n", (VALUEPAIR_NAME(buf)));
            kfree(data);
            return WLCSM_GEN_ERR;
        }
        memcpy(data->tuple,buf,len);
        /*  Figure out where to put new node */
        while (*new) {
            t_WLCSM_NVRAM_TUPLE  *this = container_of(*new, t_WLCSM_NVRAM_TUPLE, node);
            result = strcmp(data->tuple->value, this->tuple->value);
            parent = *new;
            if (result < 0)
                new = &((*new)->rb_left);
            else if (result > 0)
                new = &((*new)->rb_right);
            else {
                /* should not come in here in any case as same nvram will not in store when
                 * it hit this function */
                kfree(data->tuple);
                kfree(data);
                return WLCSM_GEN_ERR;
            }
        }

        /*  Add new node and rebalance tree. */
        rb_link_node(&data->node, parent, new);
        rb_insert_color(&data->node, &wlcsm_nvram_tree);
        return WLCSM_SUCCESS;
    } else {
        printk("nvram:%s did not set correctly due to no mem\n", (VALUEPAIR_NAME(buf)));
        return WLCSM_GEN_ERR;
    }
}


void _wlcsm_nvram_tuple_del (char *name )
{

    t_WLCSM_NVRAM_TUPLE *data=_wlcsm_nvram_tuple_search(name);
    if ( data ) {
        rb_erase(&data->node,&wlcsm_nvram_tree);
        kfree(data->tuple);
        kfree(data);
    }
    return;
}


int wlcsm_nvram_set(char *buf, int len)
{
    int ret=0;
    t_WLCSM_NVRAM_TUPLE *data=NULL;
    char *value;
    if(VALUEPAIR_LEN(buf)>=WLCSM_NAMEVALUEPAIR_MAX) return WLCSM_GEN_ERR;
    value=VALUEPAIR_VALUE(buf);
    if(!value)
        return wlcsm_nvram_unset(buf);

    WLCSM_NVRAM_LOCK(); /* aquire the lock before searching */
    data=_wlcsm_nvram_tuple_search(VALUEPAIR_NAME(buf));
    if (data) {
        if(ksize(data->tuple)>=len)  {
            memcpy(data->tuple,buf,len);
            WLCSM_NVRAM_UNLOCK();
            return WLCSM_SUCCESS;
        } else {
            rb_erase(&data->node,&wlcsm_nvram_tree);
            kfree(data->tuple);
            kfree(data);
        }
    }
    ret=_wlcsm_nvram_tuple_insert(buf,len);
    WLCSM_NVRAM_UNLOCK();
    return ret;
}

int wlcsm_nvram_unset (char *buf )
{
    WLCSM_NVRAM_LOCK();

    _wlcsm_nvram_tuple_del(VALUEPAIR_NAME(buf));

    WLCSM_NVRAM_UNLOCK();

    return 0;
}

int wlcsm_nvram_get(char *name,char *result)
{
    int len=0;
    t_WLCSM_NVRAM_TUPLE *data=NULL;

    WLCSM_NVRAM_LOCK();
    data=_wlcsm_nvram_tuple_search(name);
    if (data) {
        /* result is copied to result to make sure no correuption after unlock
         * and value is get released by del from other process
         */
        len=VALUEPAIR_LEN(data->tuple);
        memcpy(result,data->tuple,len);
    } else {
        len=0;
    }
    WLCSM_NVRAM_UNLOCK();
    return len;
}

/*  API used by kernel, these NVRAM's are static NVRAM which are not supposed to change
 *  and detele on runtime, so it is safe to return the address directly. */

char *wlcsm_nvram_k_get(char*name)
{
    t_WLCSM_NVRAM_TUPLE *data=NULL;

    WLCSM_NVRAM_LOCK();
    data=_wlcsm_nvram_tuple_search(name);
    WLCSM_NVRAM_UNLOCK();
    if (data)  {
        return (char *)VALUEPAIR_VALUE(data->tuple);
    } else {
        WLCSM_TRACE(WLCSM_TRACE_DBG," %s get null?\r\n",name);
        return NULL;
    }
}


int wlcsm_nvram_k_set(char *name, char *value)
{
    int ret=0;
    t_WLCSM_NVRAM_TUPLE *data=NULL;
    int len=_get_valuepair_total_len(name,value,0);
    if(len>=WLCSM_NAMEVALUEPAIR_MAX) return WLCSM_GEN_ERR;

    WLCSM_NVRAM_LOCK(); /* aquire the lock before searching */

    data=_wlcsm_nvram_tuple_search(name);
    if (data) {
        if(ksize(data->tuple)>=len)  {
            _valuepair_set((t_WLCSM_NAME_VALUEPAIR *)(data->tuple),name,value,0);
            WLCSM_NVRAM_UNLOCK();
            return WLCSM_SUCCESS;
        } else {
            rb_erase(&data->node,&wlcsm_nvram_tree);
            kfree(data->tuple);
            kfree(data);
        }
    } else {
        char *buf=kmalloc(len,GFP_ATOMIC);
        if(buf)  {
            _valuepair_set((t_WLCSM_NAME_VALUEPAIR *)buf,name,value,0);
            ret=_wlcsm_nvram_tuple_insert(buf,len);
            kfree(buf);
        }
    }
    WLCSM_NVRAM_UNLOCK();

    return ret;
}

#ifdef CONFIG_JFFS_NVRAM
static char *large_nvram_list[] = {
	"MULTIFILTER_MAC",
	"MULTIFILTER_DEVICENAME",
	"MULTIFILTER_MACFILTER_DAYTIME",
	"MULTIFILTER_MACFILTER_DAYTIME_V2",
	"MULTIFILTER_TMP",
	"Tor_redir_list",
	"asus_device_list",
	"autofw_rulelist",
	"captive_portal",
	"captive_portal_adv_local_clientlist",
	"captive_portal_adv_profile",
	"cloud_sync",
	"custom_clientlist",
	"custom_usericon",
	"custom_usericon_del",
	"dhcp1_staticlist",
	"dhcp_staticlist",
	"dsltmp_cfg_iptv_pvclist",
	"fb_comment",
	"filter_lwlist",
	"game_vts_rulelist",
	"gvlan_rulelist",
	"ipsec_client_list_1",
	"ipsec_client_list_2",
	"ipsec_client_list_3",
	"ipsec_client_list_4",
	"ipsec_client_list_5",
	"ipsec_profile_1",
	"ipsec_profile_1_ext",
	"ipsec_profile_2",
	"ipsec_profile_2_ext",
	"ipsec_profile_3",
	"ipsec_profile_3_ext",
	"ipsec_profile_4",
	"ipsec_profile_4_ext",
	"ipsec_profile_5",
	"ipsec_profile_5_ext",
	"ipsec_profile_client_1",
	"ipsec_profile_client_1_ext",
	"ipsec_profile_client_2",
	"ipsec_profile_client_2_ext",
	"ipsec_profile_client_3",
	"ipsec_profile_client_3_ext",
	"ipsec_profile_client_4",
	"ipsec_profile_client_4_ext",
	"ipsec_profile_client_5",
	"ipsec_profile_client_5_ext",
	"ig_client_list",
	"ipv6_fw_rulelist",
	"keyword_rulelist",
	"keyword_sched",
	"kg_devicename",
	"kg_mac",
	"lb_skip_port",
	"lp55xx_lp5523_sch",
	"nc_setting_conf",
	"pptpd_clientlist",
	"pptpd_sr_rulelist",
	"qos_bw_rulelist",
	"qos_orates",
	"qos_rulelist",
	"share_link_host",
	"share_link_param",
	"share_link_result",
	"sr_rulelist",
	"sshd_authkeys",
	"sshd_hostkey",
	"sshd_dsskey",
	"sshd_ecdsakey",
	"subnet_rulelist",
	"tl_cycle",
	"tr_ca_cert",
	"tr_client_cert",
	"tr_client_key",
	"url_rulelist",
	"url_sched",
	"vlan_pvid_list",
	"vlan_rulelist",
	"vpn_client1_custom",
	"vpn_client2_custom",
	"vpn_client3_custom",
	"vpn_client4_custom",
	"vpn_client5_custom",
	"vpn_crt_client_ca",
	"vpn_crt_client_crl",
	"vpn_crt_client_crt",
	"vpn_crt_client_key",
	"vpn_crt_client_static",
	"vpn_crt_server_ca",
	"vpn_crt_server_client_crt",
	"vpn_crt_server_client_key",
	"vpn_crt_server_crl",
	"vpn_crt_server_crt",
	"vpn_crt_server_dh",
	"vpn_crt_server_key",
	"vpn_crt_server_static",
	"vpn_server_ccd_val",
	"vpn_server_custom",
	"vpn_server1_ccd_val",
	"vpn_server1_custom",
	"vpn_server2_ccd_val",
	"vpn_server2_custom",
	"vpn_serverx_clientlist",
	"vpnc_clientlist",
	"vpnc_dev_policy_list",
	"vpnc_dev_policy_list_tmp",
	"vpnc_pptp_options_x_list",
	"vts1_rulelist",
	"vts_rulelist",
	"wans_routing_rulelist",
	"wl0.1_maclist_x",
	"wl0.2_maclist_x",
	"wl0.3_maclist_x",
	"wl0.4_maclist_x",
	"wl0_maclist_x",
	"wl0_rast_static_client",
    "wl0_sched",
    "wl0_sched_v2",
	"wl1.1_maclist_x",
	"wl1.2_maclist_x",
	"wl1.3_maclist_x",
	"wl1.4_maclist_x",
	"wl1_maclist_x",
	"wl1_rast_static_client",
	"wl1_sched",
    "wl1_sched_v2",
	"wl2.1_maclist_x",
	"wl2.2_maclist_x",
	"wl2.3_maclist_x",
	"wl2.4_maclist_x",
	"wl2_maclist_x",
	"wl2_rast_static_client",
	"wl2_sched",
    "wl2_sched_v2",
	"wl_maclist_x",
	"wl_rast_static_client",
	"wl_sched",
    "wl_sched_v2",
	"wollist",
	"wrs_app_rulelist",
	"wrs_rulelist",
	"wtf_rulelist",
	"yadns_rulelist",
	"amas_dbsta",
	"amas_dbsta_all",
	"wl0_chansps",
	"wl1_chansps",
	"wl2_chansps",
	"wl0.2_gn_wbl_rule",
	"bwdpi_game_list",
	"bwdpi_stream_list",
	NULL
};

static int large_nvram(const char *name)
{
	int i;

	for (i = 0; large_nvram_list[i] != NULL; i++)
		if (!strcmp(name, large_nvram_list[i]))
			return 1;

	return 0;
}
#endif

/* function to avoid copy-pasting its function body */
static int wlcsm_double_null_terminate(char *buf, int count, int len)
{
	/* external caller expects buf to be double-NULL character terminated */
	if (len + 1 < count) {
		buf[len++] = '\0';
	} else {
		buf[count - 2] = '\0';
		buf[count - 1] = '\0';
	}

	return len;
}

/*
 * As well as its 'WLAN-driver-internal' buffer of nvram variables, the WLAN driver relies on
 * nvram variables that are stored outside of the WLAN driver. This function gets called by the WLAN driver
 * to retrieve those 'external' variables.
 */
int wlcsm_nvram_getall(char *buf,int count)
{
    struct rb_node *node;
    t_WLCSM_NAME_VALUEPAIR *data;
    char *name,*value;
    int len=0;

    WLCSM_NVRAM_LOCK();

    for(node=rb_first(&wlcsm_nvram_tree); node; node=rb_next(node)) {
        data = rb_entry(node,t_WLCSM_NVRAM_TUPLE, node)->tuple;
        name=VALUEPAIR_NAME(data);
        value=VALUEPAIR_VALUE(data);

#ifdef CONFIG_JFFS_NVRAM
	if (large_nvram(name))
		continue;
#endif

        WLCSM_TRACE(WLCSM_TRACE_LOG,"---:%s:%d  name:%s value:%s\r\n",__FUNCTION__,__LINE__,name,value );
        if((count-len) >(strlen(name)+1+strlen(value)+1)) {
            len+=sprintf(buf+len,"%s=%s",name,value)+1;
        } else
            break;
    }

    len = wlcsm_double_null_terminate(buf, count, len);
    
    WLCSM_NVRAM_UNLOCK();
    return len;

}

/*
 * This function gets called when kernel-space nvrams have to be transferred to user space. The
 * kernel nvram buffer can be too large to fit into one ioctl buffer. This function allows reading
 * of the kernel nvram buffer in multiple 'chunks', by calling this function multiple times.
 *
 * @param[in] buf     buffer to write nvram strings into
 * @param[in] count   *half* the size of buf in [bytes]
 * @param[in] pos     byte position in a (virtual) bytestream of nvram strings to start reading
 *                    from
 * @return            if ret < count : means that subsequent chunks are available
 *                    if ret >= count: means that final chunk was returned
 */
int wlcsm_nvram_getall_pos(char *buf,int count,int pos)
{
    struct rb_node *node;
    t_WLCSM_NAME_VALUEPAIR *data;
    char *name,*value;
    int len=0,tcount=0,first=1;
    char tbuf[WLCSM_NAMEVALUEPAIR_MAX];

    WLCSM_NVRAM_LOCK();

    for(node=rb_first(&wlcsm_nvram_tree); node; node=rb_next(node)) {
        data = rb_entry(node,t_WLCSM_NVRAM_TUPLE, node)->tuple;
        name=VALUEPAIR_NAME(data);
        value=VALUEPAIR_VALUE(data);
        len+=sprintf(tbuf,"%s=%s",name,value)+1;
        if(len>pos && tcount<count) {
            if(first)  {
                tcount+=sprintf(buf+tcount,"%s",tbuf+strlen(tbuf)+1-(len-pos))+1;
                first=0;
            }
            else
                tcount+=sprintf(buf+tcount,"%s",tbuf)+1;
        }
    
    	if(tcount>count) break;
    }

    WLCSM_NVRAM_UNLOCK();
    return tcount;
}
