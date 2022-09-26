#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <shared.h>

#include <libnetfilter_conntrack/libnetfilter_conntrack.h>

#include "list.h"
#include "log.h"
#include "nfct.h"
#include "nfssdk.h"
#include "nffdb.h"
#include "nfsw.h"

// copy from qca-wifi/source/include/_ieee80211.h
enum wlan_band_chann_id {
    WLAN_BAND_CHANN_UNSPECIFIED = 0,
    WLAN_BAND_CHANN_2GHZ = 1,
    WLAN_BAND_CHANN_5GHZ = 2,
    WLAN_BAND_CHANN_6GHZ = 3,
    /* Add new band definitions here */
    WLAN_BAND_CHANN_MAX,
};

static int qca_wlan_get_band(int band)
{
    switch (band) {
    case WLAN_BAND_CHANN_2GHZ:
        return 2;
    case WLAN_BAND_CHANN_5GHZ:
        return 5;
    case WLAN_BAND_CHANN_6GHZ:
        return 6;
    default:
        return 0;
    }
}

#define LINE_FORMAT "%d %17s %d"
static int qca_stainfo(const char *ifname, bool is_guest, struct list_head *list) 
{
    sw_node_t swt, *sw;
    ethsw_mac_table tbl;

#if 0
    char macstr[ETHER_ADDR_LENGTH];
    int i;

    //defined in qca-wifi/source/os/linux/tools/qcatools_lib.c
    qca_wlan_mac_table_get(ifname, &tbl);

#if defined(_MACDBG_)
    printf("================================\nQCA wifi table.count=[%d]\n", tbl.count);
    for (i=0;i < tbl.count; i++) {
        printf("%2d: %3d %18s %4d %4d\n", i + 1, tbl.entry[i].vid,
               mac2str(tbl.entry[i].mac, macstr), tbl.entry[i].port, 
               qca_wlan_get_band(tbl.entry[i].port));
    }
#endif
    
    for (i=0; i<tbl.count; i++) {
        sw = sw_node_new();
        list_add_tail(&sw->list, list);

        sw->iswl = 1;
        sw->port = qca_wlan_get_band(tbl.entry[i].port);
        memcpy(sw->mac, mac2str(tbl.entry[i].mac, macstr), ETHER_ADDR_LENGTH);
    }
#else
    char cmd[128];
    unsigned int vid, port;
    char mac[18];
    int cnt=1;
    FILE *fp;

    sprintf(cmd, "wlanconfig %s list mactbl", ifname);
    fp = popen(cmd, "r");
    if (!fp) return -1;

    while (fscanf(fp, LINE_FORMAT, &vid, mac, &port) == 3) {
        sw = sw_node_new();
        list_add_tail(&sw->list, list);

        sw->is_wl = 1;
        sw->is_guest = is_guest;
        memcpy(sw->ifname, ifname, IFNAMESIZE);
        sw->port = qca_wlan_get_band(port);
        memcpy(sw->mac, mac, ETHER_ADDR_LENGTH);
#if defined(_MACDBG_)
        nf_printf("%2d: %16s %3d %18s %2d %5d %4d %4d\n", cnt++, ifname, 0, sw->mac,
                  sw->is_wl, sw->is_guest, sw->port, sw->port);
#endif
    }

    pclose(fp);
#endif

    return 0;
}

static void qca_sta_info_list_get(struct list_head *list)
{
    int i = 0, j;
    char word[256], *next;
    char tmp[128], prefix[] = "wlXXXXXXXXXX_";
    char *name;
    char name_vif[] = "wlX.Y_XXXXXXXXXX";

    foreach(word, nvram_safe_get("wl_ifnames"), next) {
        SKIP_ABSENT_BAND_AND_INC_UNIT(i);
        qca_stainfo(word, false, list);

        // get guest network AP's sta_list
        for (j = 1; j < 4; j++) {
#ifdef RTCONFIG_WIRELESSREPEATER
            if ((nvram_get_int("sw_mode") == SW_MODE_REPEATER) && (i == nvram_get_int("wlc_band")) && (j == 1)) 
                break;
#endif
            sprintf(prefix, "wl%d.%d_", i, j);
            if (nvram_match(strcat_r(prefix, "bss_enabled", tmp), "1")) {
                sprintf(tmp, "%s%s", prefix, "ifname");
                sprintf(name_vif, "%s", nvram_safe_get(tmp));
                if (!strlen(name_vif)) 
                    continue;

                qca_stainfo(name_vif, true, list);
            }
        }
        i++;
    }
}

int sw_list_parse(struct list_head *list)
{
    sw_node_t *sw;
    ethsw_mac_table tbl;
    int i;
    char macstr[ETHER_ADDR_LENGTH];
    char *ifname = nvram_get("lan_ifname");

#if defined(_MACDBG_)
    nf_printf("No.      ifname      vid        MAC         wl guest  phy  port\n");
    nf_printf("=== ================ ===  ================= == =====  ==== ====\n");
#endif

    int ret = qca_fdb_mac_table_get(&tbl);
    for (i = 0; i < tbl.count; i++) {
#if defined(RTAC95U)
        if (tbl.entry[i].port <= 0 || tbl.entry[i].port >= 5)
            continue;
#elif defined(PLAX56_XP4)
        if (tbl.entry[i].port <= 2 || tbl.entry[i].port >= 5)
            continue;
#endif
#if defined(_MACDBG_)
        nf_printf("%2d: %16s %3d %18s %2d %5d %4d %4d\n", i + 1, ifname, tbl.entry[i].vid,
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

    qca_sta_info_list_get(list);

#if defined(NFCMDBG)
    sw_list_dump(list);
#endif

    return 0;
}
