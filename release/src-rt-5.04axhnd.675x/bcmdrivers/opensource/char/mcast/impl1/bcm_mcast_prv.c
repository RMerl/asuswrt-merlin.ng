#include <net/ip.h>
#include <linux/blog.h>
#include <linux/blog_rule.h>
#include "bcm_mcast_priv.h"
#include "rdpa_api.h"
#include "rdpa_mw_blog_parse.h"
#include "bcm_mcast_whitelist.h"

static bdmf_object_handle iptv = NULL;

#define FLOW_HW_INVALID         0xFFFFFFFF
#define MAX_NUM_PRV_VLAN_TAG    1

#define REQUEST_KEY_TAG_NUM_VALIDATE(vtag_num, ret) do      \
{                                                           \
    if (vtag_num > MAX_NUM_PRV_VLAN_TAG)                    \
    {                                                       \
        __logError("vtag_num<%d> out of range!", vtag_num); \
        return ret;                                         \
    }                                                       \
                                                            \
} while(0)

typedef struct mcast_key_entry
{
    DLIST_ENTRY(mcast_key_entry) list;
    uint16_t key;
    uint32_t index; /* Channel index in RDD */
    int ref_cnt[MAX_NUM_PRV_VLAN_TAG + 1]; /* ref cnt per tag */
    Blog_t *blog_p;
} mcast_key_entry_t;

DLIST_HEAD(mcast_key_list_t, mcast_key_entry);

static uint16_t key_last_used = 0;
static int max_num_of_mcast_flows = 0;

struct mcast_key_list_t mcast_key_list;

static mcast_key_entry_t *mcast_key_find(uint16_t key)
{
    mcast_key_entry_t *entry, *tmp_entry;

    DLIST_FOREACH_SAFE(entry, &mcast_key_list, list, tmp_entry)
    {
        if (entry->key == key)
            return entry;
    }

    return NULL;
}

static bdmf_index mcast_key_del(uint16_t key, uint8_t vtag_num)
{
    mcast_key_entry_t *entry = mcast_key_find(key);
    bdmf_index index = BDMF_INDEX_UNASSIGNED;
    int i = 0, ref_cnt_total = 0;

    REQUEST_KEY_TAG_NUM_VALIDATE(vtag_num, BDMF_INDEX_UNASSIGNED);

    if (entry)
    {
        index = entry->index;
        entry->ref_cnt[vtag_num]--;

        for (i = 0; i < (MAX_NUM_PRV_VLAN_TAG + 1); i++)
        {
            ref_cnt_total += entry->ref_cnt[i];
        }

        if (ref_cnt_total == 0)
        {
            DLIST_REMOVE(entry, list);
            bdmf_free(entry);
        }
    }

    return index;
}

static uint16_t mcast_key_add(uint32_t index, uint8_t vtag_num, Blog_t *blog_p)
{
    mcast_key_entry_t *entry, *tmp_entry;
    uint16_t try_num = 0;

    REQUEST_KEY_TAG_NUM_VALIDATE(vtag_num, BDMF_INDEX_UNASSIGNED);

    /* search if request is already recorded, and return its key */
    DLIST_FOREACH_SAFE(entry, &mcast_key_list, list, tmp_entry)
    {
        if (entry->index == index && entry->blog_p == blog_p)
        {
            entry->ref_cnt[vtag_num]++;
            return entry->key;
        }
    }

find_next_free:
    if (try_num++ == max_num_of_mcast_flows)
        return BDMF_INDEX_UNASSIGNED;
	
    if (++key_last_used == max_num_of_mcast_flows)
        key_last_used = 0;

    /* find next availible key */
    DLIST_FOREACH_SAFE(entry, &mcast_key_list, list, tmp_entry)
    {
        if (entry->key == key_last_used)
	    goto find_next_free;
    }

    entry = bdmf_alloc(sizeof(mcast_key_entry_t));
    if (!entry)
        return BDMF_INDEX_UNASSIGNED;

    memset(entry, 0, sizeof(mcast_key_entry_t));
    entry->index = index;
    entry->key = key_last_used;
    entry->blog_p = blog_p;
    entry->ref_cnt[vtag_num] = 1;
    DLIST_INSERT_HEAD(&mcast_key_list, entry, list);

    return key_last_used;
}

#if defined(CONFIG_RUNNER_IPTV_LKUP_KEY_INCLUDE_SRC_PORT)
static int rx_if_get_by_blog(Blog_t *blog_p, rdpa_if *rx_if)
{
    *rx_if = rdpa_if_wan0;

    if (blog_p->rx.info.phyHdrType == BLOG_GPONPHY)
    {
        __logDebug("source.phy GPON\n");
        *rx_if = rdpa_wan_type_to_if(rdpa_wan_gpon);
    }
    else if (blog_p->rx.info.phyHdrType == BLOG_EPONPHY)
    {
        __logDebug("source.phy EPON\n");
        *rx_if = rdpa_wan_type_to_if(rdpa_wan_epon);
    }
    else if (is_netdev_wan((struct net_device *)blog_p->rx_dev_p))
    {
        __logDebug("source.phy ETH WAN\n");
        *rx_if = rdpa_wan_type_to_if(rdpa_wan_gbe);
    }
    else
    {
        /* LAN */ 
        __logError("LAN flow is not supported: Rx %u, Tx %u", blog_p->rx.info.phyHdrType,
            blog_p->tx.info.phyHdrType);
        return -1;
    }

    return 0;
}
#endif

static int blog_parse_mcast_group(Blog_t *blog_p, int is_l2, int include_vid, rdpa_iptv_channel_key_t *key)
{
    uint32_t vlan_id;

    if (is_l2)
    {
        uint32_t is_host = 0; 

        if (blog_p->rx.info.bmap.PLD_IPv4)
        {
            is_host = blog_p->rx.tuple.daddr & htonl(0xff000000);

            if (is_host == 0)
            {
                uint32_t daddr = htonl(blog_p->rx.tuple.daddr);
				
                key->mcast_group.mac.b[0] = 0x1;         
                key->mcast_group.mac.b[1] = 0x0;         
                key->mcast_group.mac.b[2] = 0x5e;         
                key->mcast_group.mac.b[3] = (daddr&0x7f0000)>>16; 
                key->mcast_group.mac.b[4] = (daddr&0xff00)>>8; 
                key->mcast_group.mac.b[5] = daddr&0xff; 
            }
            else
            {
                __logDebug("L2 multicast supported for Host control mode only");
                return BDMF_ERR_NOT_SUPPORTED;
            }
        }
        else if (blog_p->rx.info.bmap.PLD_IPv6)
        {
            is_host = blog_p->tupleV6.daddr.p8[0] & 0xff;

            if (is_host == 0)
            {
                key->mcast_group.mac.b[0] = 0x33;         
                key->mcast_group.mac.b[1] = 0x33;         
                key->mcast_group.mac.b[2] = blog_p->tupleV6.daddr.p8[12];
                key->mcast_group.mac.b[3] = blog_p->tupleV6.daddr.p8[13];
                key->mcast_group.mac.b[4] = blog_p->tupleV6.daddr.p8[14];
                key->mcast_group.mac.b[5] = blog_p->tupleV6.daddr.p8[15];
            }
            else
            {
                __logDebug("L2 multicast supported for Host control mode only");
                return BDMF_ERR_NOT_SUPPORTED;
            }
        }
    }
    else
    {
        /* Retrieve group and source IP addresses. */
        if (blog_p->rx.info.bmap.PLD_IPv4)
        {
            /* IGMP */
            key->mcast_group.l3.gr_ip.family = bdmf_ip_family_ipv4;
            key->mcast_group.l3.gr_ip.addr.ipv4 = htonl(blog_p->rx.tuple.daddr);
            key->mcast_group.l3.src_ip.family = bdmf_ip_family_ipv4;
            key->mcast_group.l3.src_ip.addr.ipv4 = htonl(blog_p->rx.tuple.saddr);
        }
        else
        {
            /* MLD. */
            key->mcast_group.l3.gr_ip.family = bdmf_ip_family_ipv6;
            memcpy(&key->mcast_group.l3.gr_ip.addr.ipv6, &blog_p->tupleV6.daddr, sizeof(bdmf_ipv6_t));
            key->mcast_group.l3.src_ip.family = bdmf_ip_family_ipv6;
            memcpy(&key->mcast_group.l3.src_ip.addr.ipv6, &blog_p->tupleV6.saddr, sizeof(bdmf_ipv6_t));
        }
    }
    if (include_vid)
    {
        vlan_id = blog_p->vtag[0];
        key->vid = vlan_id & RDPA_VID_MASK;
    }

    return 0;
}

static int blog_parse_mcast_request_key(Blog_t *blog_p, rdpa_iptv_channel_key_t *key)
{
    rdpa_iptv_lookup_method lookup_method;
    int is_l2 = 0, include_vid = 0;

    rdpa_iptv_lookup_method_get(iptv, &lookup_method);

    /* Get VID if necessary */
    if (lookup_method == iptv_lookup_method_group_ip_src_ip_vid ||
        lookup_method == iptv_lookup_method_mac_vid)
    {
        if (blog_p->vtag_num != 1)
        {
            __logDebug("%d tags set in blog_filter. Only a single VID must be specified, required by lookup method",
                blog_p->vtag_num);
            return BDMF_ERR_PARSE;
        }
        include_vid = 1;
    }

    /* L2 multicast */
    if (lookup_method == iptv_lookup_method_mac_vid ||
        lookup_method == iptv_lookup_method_mac)
    {
        is_l2 = 1;
    }

    /* L3 multicast */
    return blog_parse_mcast_group(blog_p, is_l2, include_vid, key);
}

static uint32_t runner_iptv_channel_create(Blog_t *blog_p)
{
    int rc;
    rdpa_iptv_channel_request_t request;
    uint16_t map_idx;
    rdpa_channel_req_key_t request_key;
#if defined(CONFIG_RUNNER_IPTV_LKUP_KEY_INCLUDE_SRC_PORT)
    rdpa_if rx_if;
#endif

    /* Get the multicast group, multicast source and vid */
    memset(&request, 0, sizeof(rdpa_iptv_channel_request_t));
    rc = blog_parse_mcast_request_key(blog_p, &request.key);
    if (rc < 0)
        return FLOW_HW_INVALID;

#if defined(CONFIG_RUNNER_IPTV_LKUP_KEY_INCLUDE_SRC_PORT)
    rc = rx_if_get_by_blog(blog_p, &rx_if);
    if (rc)
        return FLOW_HW_INVALID;
    request.key.rx_if = rx_if;
#endif

    /* Get mcast classification result from blog commands */
    rc = blog_parse_mcast_result_get(blog_p, &request.mcast_result);
    if (rc < 0)
        goto Exit;

    rc = rdpa_iptv_channel_request_add(iptv, &request_key, &request);
    if (rc < 0)
        goto Exit;

    map_idx = mcast_key_add(request_key.channel_index, blog_p->vtag_num, blog_p);
    if (map_idx == BDMF_INDEX_UNASSIGNED)
        rc = -1;

Exit:
    blog_parse_mcast_result_put(&request.mcast_result);
    if (rc < 0)
    {
        if (rc == BDMF_ERR_ALREADY)
            __logError("Failed to add channel request, BDMF_ERR_ALREADY");
        else
            __logError("Failed to add channel request, rc=%d", rc);

        return FLOW_HW_INVALID;
    }

    return map_idx;
}

static int runner_iptv_channel_delete(uint32_t key, struct blog_t *blog_p)
{
    rdpa_channel_req_key_t request_key;
    int rc;

    request_key.port = blog_parse_egress_port_get(blog_p);
    request_key.channel_index = mcast_key_del(key, blog_p->vtag_num);

    rc = rdpa_iptv_channel_request_delete(iptv, &request_key);
    if (rc)
    {
        __logError("Failed to remove flow key=%d, rc=%d", key, rc);
        return rc;
    }

    return 0;
}

static int mcast_config(Blog_t *blog_p, BlogTraffic_t traffic, bcm_mcast_flowkey_t *hdl_p)
{
    uint32_t key = runner_iptv_channel_create(blog_p);

    if (key == FLOW_HW_INVALID)
        return -1;

    hdl_p->blog_key.fc.word = key;
    hdl_p->blog_key.mc.word = key;

    return 0;
}

static Blog_t *mcast_deconfig(bcm_mcast_flowkey_t activate_key, BlogTraffic_t traffic)
{
    uint32_t key = activate_key.blog_key.mc.word;
    Blog_t *blog_p;
    mcast_key_entry_t *entry = mcast_key_find(key);

    if (!entry)
    {
        __logError("mcast_deconfig mcast_key_entry not found: key=%d traffic=%d", key, traffic);
        return NULL;
    }

    blog_p = entry->blog_p;
    runner_iptv_channel_delete(key, blog_p);

    return blog_p;
}

static int mcast_operation_mode_get(rdpa_operation_mode *operation_mode)
{
    int rc;
    bdmf_object_handle system = NULL;
    rdpa_system_init_cfg_t init_cfg;

    rc = rdpa_system_get(&system);
    if (rc)
    {
        __logError("Failed to get RDPA System object, rc=%d", rc);
        goto Exit;
    }

    rc = rdpa_system_init_cfg_get(system, &init_cfg);
    if (rc)
    {
        __logError("Failed to get RDPA System init cfg, rc=%d", rc);
        goto Exit;
    }

   *operation_mode = init_cfg.operation_mode; 

Exit:
    if (system)
        bdmf_put(system);

    return rc;
}

static int mcast_max_flows_get(int *num_flows)
{
    int rc;
    bdmf_object_handle system = NULL;
    rdpa_system_resources_t system_resources = {};

    rc = rdpa_system_get(&system);
    if (rc)
    {
        __logError("Failed to get RDPA System object, rc=%d", rc);
        goto Exit;
    }

    rc = rdpa_system_system_resources_get(system, &system_resources);
    if (rc)
    {
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "rdpa_system_system_resources_get() failed, rc=%d", rc);
        goto Exit;
    }

    *num_flows = system_resources.num_iptv_entries;

Exit:
    if (system)
        bdmf_put(system);

    return rc;
}

static int bcm_mcast_runner_init_iptv(void)
{
    int rc;
    BDMF_MATTR(iptv_attrs, rdpa_iptv_drv());
    rdpa_iptv_lookup_method method = iptv_lookup_method_group_ip_src_ip;

    rc = rdpa_iptv_get(&iptv);
    if (rc)
    {
        rc = bdmf_new_and_set(rdpa_iptv_drv(), NULL, iptv_attrs, &iptv);
        if (rc)
        {
            __logError("rdpa iptv object and can't be created, rc=%d", rc);
            goto Exit;
        }
    }

    rdpa_iptv_lookup_method_set(iptv, method);
    if (rc)
    {
        __logError("Cannot set iptv lookup method, rc=%d", rc);
        goto Exit;
    }

Exit:
    return rc;
}

int bcm_mcast_prv_init(void)
{
    rdpa_operation_mode operation_mode;

    if (mcast_operation_mode_get(&operation_mode))
        return -1;

    if (operation_mode != rdpa_method_prv)
        return 0;

    if (bcm_mcast_runner_init_iptv())
        return -1;

    if (mcast_max_flows_get(&max_num_of_mcast_flows))
        return -1;

    /* PRV mode -- if we reach here */
    bcm_mcast_mode_prv = 1;
    /* PRV mode does not go through blog_activate/deactivate */
    bcm_mcast_flow_add_hook = mcast_config;
    bcm_mcast_flow_delete_hook = mcast_deconfig;

    /* Whitelist functions are used only in fcache mode and not needed in PRV mode.
       Mark them NULL */
    bcm_mcast_whitelist_add_fn    = NULL;
    bcm_mcast_whitelist_delete_fn = NULL;

    return 0;
}
