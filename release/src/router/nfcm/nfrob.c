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

#include "list.h"
#include "log.h"
#include "nfct.h"
#include "nfrob.h"

extern bool g_swap;

rob_node_t *rob_node_new()
{
    rob_node_t *rb;

    rb = (rob_node_t *)calloc(1, sizeof(rob_node_t));
    if (!rb) return NULL;

    INIT_LIST_HEAD(&rb->list);

    return rb;
}

void rob_node_free(rob_node_t *rb)
{
	if (rb)
		free(rb);

	return;
}

static void rob_set_state(rob_node_t *rb, char *value)
{
    // value is DOWN, 1000FD, 100FD, 10FD...
	if (!strncmp(value, "DOWN", 4)) {
		rb->state = false;
        return;
	}
	rb->state = true;
}

static void rob_set_port(rob_node_t *rb, char *value)
{
	// value is "4:"
	value[strlen(value)-1] = '\0';

	rb->port = atoi(value);
}

static void rob_set_vlan(rob_node_t *rb, char *value)
{
	// value is "1"
	rb->vlan = atoi(value);
}

static void rob_set_mac(rob_node_t *rb, char *value)
{
	// value is "18:31:bf:cf:5d:c5"
	strcpy(rb->mac, value);
}

int rob_node_parse(rob_node_t *rb, char *buff)
{
    int i = 0, j;
    char *delim = " ";
    char *pch;
    char attrs[ROB_ATTR_MAX][128];

    buff[strlen(buff)-1] = '\0';

    pch = strtok(buff, delim);
    while (pch != NULL) {
        strcpy(attrs[i], pch);
        i++;
        pch = strtok(NULL, delim);
    }

	rob_set_state(rb, attrs[ROB_ATTR_STATE]);
	if (rb->state) {
		rob_set_port(rb, attrs[ROB_ATTR_PORT]);
		rob_set_vlan(rb, attrs[ROB_ATTR_VLAN]);
		rob_set_mac(rb, attrs[ROB_ATTR_MAC]);
		rb->enabled = true;
	}

	return 1;
}

void rob_list_dump(struct list_head *list)
{
    rob_node_t *rb;

    printf("%s:\n", __FUNCTION__);
    list_for_each_entry(rb, list, list) {
        printf("mac:\t%s\n", rb->mac);
		printf("iswl:\t%d\n", rb->iswl);
        printf("port:\t%d\n", rb->port);
        printf("state:\t%d\n", rb->state);
        printf("enabled:%d\n", rb->enabled);
        printf("--------------\n");
    }
}

static sta_info_t *wl_sta_info(char *ifname, struct ether_addr *ea)
{
	static char buf[sizeof(sta_info_t)];
	sta_info_t *sta = NULL;

	strcpy(buf, "sta_info");
	memcpy(buf + strlen(buf) + 1, (void *)ea, ETHER_ADDR_LEN);

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
    rob_node_t *rb;

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
	for(i = 0; i < auth->count; i++) {
		sta = wl_sta_info(name, &auth->ea[i]);
		if (!sta) 
            continue;
		if (!(sta->flags & WL_STA_ASSOC) && !sta->in) 
            continue;

        // add to mclist
        rb = rob_node_new();
        list_add_tail(&rb->list, list);

        rb->iswl = true;
        rb->port = (CHSPEC_CHANNEL(chspec) < 15) ? 2 : 5;
        strcpy(rb->mac, mac2str((unsigned char *)&auth->ea[i], macstr));
		rb->enabled = true;
		rb->state = true;
		rb->vlan = 0;
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
				if (!sta) 
					continue;

                // add to mclist
                rb = rob_node_new();
                list_add_tail(&rb->list, list);
                rb->iswl = true;
                rb->port = (CHSPEC_CHANNEL(chspec) < 15) ? 2 : 5;
                strcpy(rb->mac, mac2str((unsigned char *)&auth->ea[j], macstr));
				rb->enabled = true;
				rb->state = true;
				rb->vlan = 0;
			}
		}
	}

	/* error/exit */
exit:
	if (auth) free(auth);
}

void rob_list_parse(char *fname, struct list_head *list)
{
	char cmd[64];
	char buff[1024];
	rob_node_t rbt, *rb;
	FILE *fp = NULL;

	char word[256], *next;
	int i=0;
	foreach(word, nvram_safe_get("wl_ifnames"), next) {
		brcm_stainfo(i++, list);
	}

	sprintf(cmd, "robocfg show > %s", fname);
    //info("%s", cmd);
	system(cmd);

	if((fp = fopen(fname, "r")) == NULL) {
		error("cannot open %s to read..", fname);
		return;
	}

	while (fgets(buff, 1024, fp) != NULL) {
		if(buff[0] != 'P')
			continue;
		rob_node_parse(&rbt, buff);
		if (rbt.state == false)
			continue;
		rb = rob_node_new();
		list_add_tail(&rb->list, list);

		rb->iswl = false;
        rb->port = rbt.port;
        rb->state = rbt.state;
        rb->enabled = rbt.enabled;
        rb->vlan = rbt.vlan;
        strcpy(rb->mac, rbt.mac);
	}
	fclose(fp);

#if defined(NFCMDBG)
    rob_list_dump(list);
#endif

}

void rob_list_free(struct list_head *list)
{
	rob_node_t *rb, *rbt;

	list_for_each_entry_safe(rb, rbt, list, list) {
		list_del(&rb->list);
		rob_node_free(rb);
	}

	return;
}



