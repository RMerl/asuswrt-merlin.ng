#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <shutils.h>
#include <wlioctl.h>
#include <wlc_types.h>

#include <rtstate.h>
#include <bcmendian.h>
#include <shared.h>
#include <bcmutils.h>

#include <libnetfilter_conntrack/libnetfilter_conntrack.h>

#include "list.h"
#include "log.h"
#include "nfct.h"
#include "nfmc.h"
#include "nfsw.h"

#define WL_STA_LIST_FILE "/tmp/sta_list"

#define	MAX_STA_COUNT		128

extern bool g_swap;

#if defined(HND) || defined(CONFIG_ET)

#if 0 
  typedef struct {
    uint16          ver;        /* version of this struct */
    uint16          len;        /* length in bytes of this structure */
    uint16          cap;        /* sta's advertised capabilities */
    uint32          flags;      /* flags defined below */
    uint32          idle;       /* time since data pkt rx'd from sta */
    struct ether_addr   ea;     /* Station address */
    wl_rateset_t        rateset;    /* rateset in use */
    uint32          in;     /* seconds elapsed since associated */
    uint32          listen_interval_inms; /* Min Listen interval in ms for this STA */
    uint32          tx_pkts;    /* # of packets transmitted */
    uint32          tx_failures;    /* # of packets failed */
    uint32          rx_ucast_pkts;  /* # of unicast packets received */
    uint32          rx_mcast_pkts;  /* # of multicast packets received */
    uint32          tx_rate;    /* Rate of last successful tx frame */
    uint32          rx_rate;    /* Rate of last successful rx frame */
    uint32          rx_decrypt_succeeds;    /* # of packet decrypted successfully */
    uint32          rx_decrypt_failures;    /* # of packet decrypted unsuccessfully */
} sta_info_t;
#endif

static sta_info_t* wl_sta_info(char *ifname, struct ether_addr *ea)
{
    static char buf[sizeof(sta_info_t)];
    sta_info_t *sta = NULL;

    strcpy(buf, "sta_info");
    memcpy(buf + strlen(buf) + 1, (void *)ea, ETHER_ADDR_LENGTH);

    if (!wl_ioctl(ifname, WLC_GET_VAR, buf, sizeof(buf))) {
        sta = (sta_info_t *)buf;
        sta->ver = dtoh16(sta->ver);

        /* Report unrecognized version */
        if (sta->ver > WL_STA_VER) {
            dbg(" ERROR: unknown driver station info version %d\n", sta->ver);
            return NULL;
        }

        sta->len = dtoh16(sta->len);
        sta->cap = dtoh16(sta->cap);
#ifdef RTCONFIG_BCMARM
        sta->aid = dtoh16(sta->aid);
#endif
        sta->flags = dtoh32(sta->flags);
        sta->in = dtoh32(sta->in);
#if 0
        sta->idle = dtoh32(sta->idle);
        sta->rateset.count = dtoh32(sta->rateset.count);
        sta->listen_interval_inms = dtoh32(sta->listen_interval_inms);
#ifdef RTCONFIG_BCMARM
        sta->ht_capabilities = dtoh16(sta->ht_capabilities);
        sta->vht_flags = dtoh16(sta->vht_flags);
#endif
#endif //#if 0
    }

    return sta;
}

/*
typedef struct maclist {
    uint32 count;
    struct ether_addr ea[1];
} maclist_t;
*/
static void brcm_stainfo(int unit, struct list_head *list)
{
    /* initial */
    char tmp[128], prefix[] = "wlXXXXXXXXXX_";
    char *name;
    struct maclist *auth = NULL;
    int mac_list_size;
    char name_vif[] = "wlX.Y_XXXXXXXXXX";
    int i, j;
    sta_info_t *sta;
    int chspec;
    char macstr[ETHER_ADDR_LENGTH];
    sw_node_t *sw;

    /* get wireless stainfo */
    snprintf(prefix, sizeof(prefix), "wl%d_", unit);
    name = nvram_safe_get(strcat_r(prefix, "ifname", tmp));

#ifdef RTCONFIG_WIRELESSREPEATER
    if ((nvram_get_int("sw_mode") == SW_MODE_REPEATER) && (nvram_get_int("wlc_band") == unit)) {
        sprintf(name_vif, "wl%d.%d", unit, 1);
        name = name_vif;
    }
#endif

    if (!strlen(name)) goto exit;

    if (wl_iovar_getint(name, "chanspec", &chspec) < 0) goto exit;

    //printf("name=[%s], chspec=[%04X], channel=[%d], is2G=[%d], is5G=[%d]\n",
    //       name, chspec, CHSPEC_CHANNEL(chspec), CHSPEC_IS2G(chspec), CHSPEC_IS5G(chspec));

    /* buffers and length */
    mac_list_size = sizeof(auth->count) + MAX_STA_COUNT * sizeof(struct ether_addr);
    auth = malloc(mac_list_size);

    if (!auth) goto exit;

    memset(auth, 0, mac_list_size);

    /* query wl for authenticated sta list */
    strcpy((char *)auth, "authe_sta_list");
    if (wl_ioctl(name, WLC_GET_VAR, auth, mac_list_size)) goto exit;

    /* build authenticated sta list */
    for (i = 0; i < auth->count; ++i) {
        sta = wl_sta_info(name, &auth->ea[i]);
        if (!sta) continue;
        if (!(sta->flags & WL_STA_ASSOC) && !sta->in) continue;

        // add to swlist
        sw = sw_node_new();
        list_add_tail(&sw->list, list);

        sw->is_wl = true;
        sw->is_guest = false;
        memcpy(sw->ifname, name, IFNAMESIZE);
        sw->port = CHSPEC_IS2G(chspec) ? 2 : 5;
        memcpy(sw->mac, mac2str((unsigned char *)&auth->ea[i], macstr), ETHER_ADDR_LENGTH);
#if defined(_MACDBG_)
        printf("%2d: %16s %3d %18s %2d %5d %4d %4d\n", i + 1, name, 0, sw->mac,
               sw->is_wl, sw->is_guest, sw->port, sw->port);
#endif
    }

    // get guest network AP's sta_list
    for (i = 1; i < 4; i++) {
#ifdef RTCONFIG_WIRELESSREPEATER
        if ((nvram_get_int("sw_mode") == SW_MODE_REPEATER) && (unit == nvram_get_int("wlc_band")) && (i == 1)) break;
#endif
        sprintf(prefix, "wl%d.%d_", unit, i);
        if (nvram_match(strcat_r(prefix, "bss_enabled", tmp), "1")) {
            sprintf(name_vif, "wl%d.%d", unit, i);

            if (wl_iovar_getint(name_vif, "chanspec", &chspec) < 0) goto exit;

            //printf("name_vif=[%s], chspec=[%04X], channel=[%d], is2G=[%d], is5G=[%d]\n",
            //       name_vif, chspec, CHSPEC_CHANNEL(chspec), CHSPEC_IS2G(chspec), CHSPEC_IS5G(chspec));

            memset(auth, 0, mac_list_size);

            /* query wl for authenticated sta list */
            strcpy((char *)auth, "authe_sta_list");
            if (wl_ioctl(name_vif, WLC_GET_VAR, auth, mac_list_size)) goto exit;

            for (j = 0; j < auth->count; j++) {
                sta = wl_sta_info(name_vif, &auth->ea[j]);
                if (!sta) continue;

                // add to mclist
                sw = sw_node_new();
                list_add_tail(&sw->list, list);

                sw->is_wl = true;
                sw->is_guest = true;
                memcpy(sw->ifname, name_vif, IFNAMESIZE);
                sw->port = CHSPEC_IS2G(chspec) ? 2 : 5;
                memcpy(sw->mac, mac2str((unsigned char *)&auth->ea[j], macstr), ETHER_ADDR_LENGTH);
#if defined(_MACDBG_)
                printf("%2d: %16s %3d %18s %2d %5d %4d %4d\n", i+1, name_vif, 0, sw->mac,
                       sw->is_wl, sw->is_guest, sw->port, sw->port);
#endif
            }
        }
    }

    /* error/exit */
exit:
    if (auth) free(auth);
}
#endif // defined(HND) || defined(CONFIG_ET)

int sw_list_parse(struct list_head *list)
{
    int i = 0;
    sw_node_t *sw;
    ethsw_mac_table tbl;
    char macstr[ETHER_ADDR_LENGTH];
    char word[256], *next;
    char *ifname = nvram_get("lan_ifname");

#if 1//defined(_MACDBG_)
    printf("===============================================================\n");
    printf("No.      ifname      vid        MAC         wl guest  phy  port\n");
    printf("=== ================ ===  ================= == =====  ==== ====\n");
#endif

    foreach(word, nvram_safe_get("wl_ifnames"), next) {
        brcm_stainfo(i++, list);
    }

#if defined(HND)
    bcm_arl_dump_us(2, (char *)&tbl);
    for (i = 0; i < (int)tbl.count; i++) {
#if defined(GTAXE11000) || defined(GTAX11000)
        if (tbl.entry[i].port >= 8) // port 7 is 2.5G
#else
        if (tbl.entry[i].port >= 7)
#endif
            continue;

#if 1//defined(_MACDBG_)
        printf("%2d: %16s %3d %18s %2d %5d %4d %4d\n", i + 1, ifname, tbl.entry[i].vid,
               mac2str(tbl.entry[i].mac, macstr), false, false, tbl.entry[i].port,
               nfcm_get_lan_ports(tbl.entry[i].port));
#endif
        sw = sw_node_new();
        list_add_tail(&sw->list, list);

        sw->is_wl = false;
        sw->is_guest = false;
        memcpy(sw->ifname, ifname, IFNAMESIZE);
        sw->port = nfcm_get_lan_ports(tbl.entry[i].port);
        memcpy(sw->mac, mac2str(tbl.entry[i].mac, macstr), ETHER_ADDR_LENGTH);
    }

#if defined(RTCONFIG_EXT_BCM53134) && (defined(RTAX88U) || defined(GTAC5300))
    bcm_arl_dump_53134(2, (char *)&tbl);
    for (i = 0; i < (int)tbl.count; i++) {
#if 1//defined(_MACDBG_)
        printf("%2d: %16s %3d %18s %2d %5d %4d %4d\n", i + 1, ifname, tbl.entry[i].vid,
               mac2str(tbl.entry[i].mac, macstr), false, false, tbl.entry[i].port,
               nfcm_get_lan_ports(tbl.entry[i].port + 10));
#endif
        sw = sw_node_new();
        list_add_tail(&sw->list, list);

        sw->is_wl = false;
        sw->is_guest = false;
        memcpy(sw->ifname, ifname, IFNAMESIZE);
        sw->port = nfcm_get_lan_ports(tbl.entry[i].port + 10);
        memcpy(sw->mac, mac2str(tbl.entry[i].mac, macstr), ETHER_ADDR_LENGTH);
    }
#endif //#if defined(RTCONFIG_EXT_BCM53134)

#else //!defined(HND)

#if defined(CONFIG_ET)
    bcm_arl_dump_5301x(&tbl);
    for (i = 0; i < (int)tbl.count; i++) {
#if defined(RTAC5300)
        //if (tbl.entry[i].port == 0 || tbl.entry[i].port > 4) continue;
        if (tbl.entry[i].port == 0) continue;
#endif
#if 1//defined(_MACDBG_)
        printf("%2d: %16s %3d %18s %2d %5d %4d %4d\n", i + 1, ifname, tbl.entry[i].vid,
               mac2str(tbl.entry[i].mac, macstr), false, false, tbl.entry[i].port,
               nfcm_get_lan_ports(tbl.entry[i].port));
#endif
        sw = sw_node_new();
        list_add_tail(&sw->list, list);

        sw->is_wl = false;
        sw->is_guest = false;
        memcpy(sw->ifname, ifname, IFNAMESIZE);
        sw->port = nfcm_get_lan_ports(tbl.entry[i].port);
        memcpy(sw->mac, mac2str(tbl.entry[i].mac, macstr), ETHER_ADDR_LENGTH);
    }
#endif
#endif //!defined(HND)

#if defined(RTCONFIG_EXT_RTL8365MB) || defined(RTCONFIG_EXT_RTL8370MB)
    for(i = 1; i <=4; i++) {
        rtkswitch_arl_dump_port_mac(i, list);
    }
#endif

#if 1//defined(NFCMDBG)
    sw_list_dump(list);
#endif

    return 0;
}
