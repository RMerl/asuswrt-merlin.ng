#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//#include <bcmnvram.h>
#include <shutils.h>
#include <wlioctl.h>
#include <wlc_types.h>

#include <rtstate.h>
#include <bcmendian.h>
#include <shared.h>
#include <bcmutils.h>

#include <libnetfilter_conntrack/libnetfilter_conntrack.h>
// userspace/private/include/ethswctl_api.h
#include "ethswctl_api.h"

#include "list.h"
#include "log.h"
#include "nfct.h"
#include "nfmc.h"

#define WL_STA_LIST_FILE "/tmp/sta_list"

#define	MAX_STA_COUNT		128

extern bool g_swap;

mc_node_t *mc_node_new()
{
    mc_node_t *mn;

    mn = (mc_node_t *)calloc(1, sizeof(mc_node_t));
    if (!mn) return NULL;

    INIT_LIST_HEAD(&mn->list);

    return mn;
}

void mc_node_free(mc_node_t *mn)
{
	if (mn)
		free(mn);

	return;
}

void mc_list_dump(struct list_head *list)
{
    mc_node_t *mn;

    printf("%s:\n", __FUNCTION__);
    list_for_each_entry(mn, list, list) {
        printf("iswl:\t%d\n", mn->iswl);
        printf("mac:\t%s\n", mn->mac);
        printf("port:\t%d\n", mn->port);
        printf("--------------\n");
    }
}

void mc_list_free(struct list_head *list)
{
	mc_node_t *mn, *mnt;

	list_for_each_entry_safe(mn, mnt, list, list) {
		list_del(&mn->list);
		mc_node_free(mn);
	}

	return;
}

static sta_info_t *wl_sta_info(char *ifname, struct ether_addr *ea)
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
    mc_node_t *mn;

	/* get wireless stainfo */
	snprintf(prefix, sizeof(prefix), "wl%d_", unit);
	name = nvram_safe_get(strcat_r(prefix, "ifname", tmp));

#ifdef RTCONFIG_WIRELESSREPEATER
	if ((nvram_get_int("sw_mode") == SW_MODE_REPEATER) && (nvram_get_int("wlc_band") == unit)) {
		sprintf(name_vif, "wl%d.%d", unit, 1);
		name = name_vif;
	}
#endif

	if (!strlen(name))
		goto exit;

    if(wl_iovar_getint(name, "chanspec", &chspec) < 0) 
        goto exit;

    //printf("name=[%s], chspec=[%d], CHSPEC_CHANNEL(chspec)=[%d]\n", name, chspec, CHSPEC_CHANNEL(chspec));

	/* buffers and length */
	mac_list_size = sizeof(auth->count) + MAX_STA_COUNT * sizeof(struct ether_addr);
	auth = malloc(mac_list_size);

	if (!auth)
		goto exit;

	memset(auth, 0, mac_list_size);

	/* query wl for authenticated sta list */
	strcpy((char*)auth, "authe_sta_list");
	if (wl_ioctl(name, WLC_GET_VAR, auth, mac_list_size))
		goto exit;

	/* build authenticated sta list */
	for(i = 0; i < auth->count; ++i) {
		sta = wl_sta_info(name, &auth->ea[i]);
		if (!sta) 
            continue;
		if (!(sta->flags & WL_STA_ASSOC) && !sta->in) 
            continue;

        // add to mclist
        mn = mc_node_new();
        list_add_tail(&mn->list, list);

        mn->iswl = true;
        mn->port = (CHSPEC_CHANNEL(chspec) < 15) ? 2 : 5;
        strcpy(mn->mac, mac2str((unsigned char *)&auth->ea[i], macstr));
	}

    // get guest network AP's sta_list
	for (i = 1; i < 4; i++) {
#ifdef RTCONFIG_WIRELESSREPEATER
		if ((nvram_get_int("sw_mode") == SW_MODE_REPEATER) && (unit == nvram_get_int("wlc_band")) && (i == 1))
			break;
#endif
		sprintf(prefix, "wl%d.%d_", unit, i);
		if (nvram_match(strcat_r(prefix, "bss_enabled", tmp), "1")) {
			sprintf(name_vif, "wl%d.%d", unit, i);

            if(wl_iovar_getint(name_vif, "chanspec", &chspec) < 0) 
                goto exit;

            //printf("name_vif=[%s], chspec=[%d], CHSPEC_CHANNEL(chspec)=[%d]\n", 
            //       name_vif, chspec, CHSPEC_CHANNEL(chspec));

            memset(auth, 0, mac_list_size);

			/* query wl for authenticated sta list */
			strcpy((char*)auth, "authe_sta_list");
			if (wl_ioctl(name_vif, WLC_GET_VAR, auth, mac_list_size))
				goto exit;

			for(j = 0; j < auth->count; j++) {
				sta = wl_sta_info(name_vif, &auth->ea[j]);
				if (!sta) continue;

                // add to mclist
                mn = mc_node_new();
                list_add_tail(&mn->list, list);

                mn->iswl = true;
                mn->port = (CHSPEC_CHANNEL(chspec) < 15) ? 2 : 5;
                strcpy(mn->mac, mac2str((unsigned char *)&auth->ea[j], macstr));
			}
		}
	}

	/* error/exit */
exit:
	if (auth) free(auth);
}

int mc_list_parse(char *fname, struct list_head *list)
{
    int i=0;
	mc_node_t *mn;
    ethsw_mac_table tbl;
    char macstr[ETHER_ADDR_LENGTH];
    char word[256], *next;

    foreach(word, nvram_safe_get("wl_ifnames"), next) {
        brcm_stainfo(i++, list);
    }

    bcm_arl_dump_us(2, (char *)&tbl);
    for (i=0;i < tbl.count;i++) {
        //printf("%d: %s %d\n", i, mac2str(tbl.entry[i].mac, macstr), tbl.entry[i].port);
        mn = mc_node_new();
        list_add_tail(&mn->list, list);

        mn->iswl = false;
        mn->port = tbl.entry[i].port;
        strcpy(mn->mac, mac2str(tbl.entry[i].mac, macstr));
    }

#if defined(NFCMDBG)
    mc_list_dump(list);
#endif

	return 0;
}
