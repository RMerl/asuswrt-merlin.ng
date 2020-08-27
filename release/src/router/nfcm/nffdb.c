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
#include "nffdb.h"

#define FDB_WL_FILE "/tmp/wlan_list"

fdb_node_t *fdb_node_new()
{
    fdb_node_t *fb;

    fb = (fdb_node_t *)calloc(1, sizeof(fdb_node_t));
    if (!fb) return NULL;

    INIT_LIST_HEAD(&fb->list);

    return fb;
}

void fdb_node_free(fdb_node_t *fb)
{
	if (fb)
		free(fb);

	return;
}

static void fdb_sw_set_port(fdb_node_t *fb, char *value)
{
	// value is "[dest_port]:4"
    char pstr[8];

    char *p = strtok(value, ":");
    p = strtok(NULL, ":");

    if (p) {
        strcpy(pstr, p);
        fb->port = atoi(pstr);
    } else {
        fb->port = 0;
    }

}

static void fdb_sw_set_mac(fdb_node_t *fb, char *value)
{
	// value is "[addr]:00-0c-29-da-52-78"
    char pstr[MAC_STR_LEN];

    char *p = strtok(value, ":");
    p = strtok(NULL, ":");

    if (p) {
        int i;
        strcpy(pstr, p);
        for (i=0;i<strlen(pstr);i++) {
            if (pstr[i] == '-') 
                pstr[i] = ':';
        }
    } else {
        strcpy(pstr, "00:00:00:00:00:00");
    }
    strcpy(fb->mac, pstr);
}

int fdb_node_sw_parse(fdb_node_t *fb, char *buff)
{
    int i = 0, j;
    char *delim = " ";
    char *pch;
    char attrs[FDB_ATTR_SW_MAX][128];
    fdb_node_t *fbt;

    //buff[strlen(buff)-1] = '\0';

    pch = strtok(buff, delim);
    while (pch != NULL) {
        strcpy(attrs[i], pch);
        i++;
        pch = strtok(NULL, delim);
    }

    fb->iswl = false;
    fdb_sw_set_port(fb, attrs[FDB_ATTR_SW_PORT]);
	fdb_sw_set_mac(fb, attrs[FDB_ATTR_SW_MAC]);

	return 0;
}

void fdb_list_dump(struct list_head *list)
{
    fdb_node_t *fb;

    list_for_each_entry(fb, list, list) {
        printf("mac:\t%s\n", fb->mac);
        printf("iswl:\t%d\n", fb->iswl);
        printf("port:\t%d\n", fb->port);
        printf("--------------\n");
    }
}

#if defined(RTAX89U) || defined(GTAXY16000)
fdb_node_t *fdb_list_search(fdb_node_t *fb, struct list_head *list)
{
    fdb_node_t *fbt;

    list_for_each_entry(fbt, list, list) {
        if(fbt->sw[0] != 1)
            continue;
        if (!strcmp(fbt->mac, fb->mac)) {
            return fbt;
        }
    }
    return NULL;
}
#endif // defined(RTAX89U) || defined(GTAXY16000)

#if defined(RTAX89U)
static int rtax89u_get_phy_port(fdb_node_t *fb)
{
    switch (fb->sw[0]) {
    case 1:
        switch (fb->sw[1]) {
        case 1: return 7;
        case 2: return 6;
        case 3: return 5;
        case 4: return 4;
        case 5: return 3;
        case 6: return 8;
        default: return 0;
        }
        break;

    case 2: return 2;
    case 3: return 1;
    case 4: return 0; //wan
    case 5: return 5; // 10G SFP+
    case 6: return 6; // 10G RJ45
    default:
        return 0;
    }

    return 0;
}
#endif // defined(RTAX89U)

#if defined(GTAXY16000)
static int gtaxy16000_get_phy_port(fdb_node_t *fb)
{
    switch (fb->sw[0]) {
    case 1:
        switch (fb->sw[1]) {
        case 1: return 7;
        case 2: return 5;
        case 3: return 6;
        case 4: return 3;
        case 5: return 4;
        case 6: return 8;
        default: return 0;
        }
        break;

    case 2: return 2;
    case 3: return 1;
    case 4: return 0; //wan
    case 5: return 5; // 10G SFP+
    case 6: return 6; // 10G RJ45
    default:
        return 0;
    }

    return 0;
}
#endif // defined(GTAXY16000)


static void fdb_wl_set_port(fdb_node_t *fb, char *value)
{
	// value is "6"
    int chann = atoi(value);

    fb->port = (chann <= 14) ? 2 : 5;

}

static void fdb_wl_set_mac(fdb_node_t *fb, char *value)
{
	// value is "cc:66:0a:cf:66:28"
    strcpy(fb->mac, value);
}

int fdb_node_wl_parse(fdb_node_t *fb, char *buff)
{
    int i=0, j=0;
    char *delim = " ";
    char *pch;
    char attrs[FDB_ATTR_WL_MAX][128];
    fdb_node_t *fbt;

    pch = strtok(buff, delim);
    while (pch != NULL) {
        strcpy(attrs[i], pch);
        i++;
        if (++j > FDB_ATTR_WL_MAX) 
            break;
        pch = strtok(NULL, delim);
    }

    fb->iswl = true;
    fdb_wl_set_port(fb, attrs[FDB_ATTR_WL_CHAN]);
	fdb_wl_set_mac(fb, attrs[FDB_ATTR_WL_MAC]);

	return 0;
}

static int qca_stainfo(const char *ifname, char *fname, struct list_head *list)
{
    char cmd[64];
    char buff[1024];
    fdb_node_t fbt, *fb;
    FILE *fp = NULL;

    sprintf(cmd, "wlanconfig %s list > %s 2> /dev/null", ifname, fname);
    //info("%s", cmd);
    system(cmd);

    if((fp = fopen(fname, "r")) == NULL) {
        error("cannot open %s to read..", fname);
        return;
    }
    while (fgets(buff, 1024, fp) != NULL) {
        if(buff[0] == 'A' || buff[0] == 'w') 
            continue;

        fdb_node_wl_parse(&fbt, buff);

        fb = fdb_node_new();
        list_add_tail(&fb->list, list);

        fb->port = fbt.port;
        fb->iswl = fbt.iswl;
        strcpy(fb->mac, fbt.mac);
    }
    fclose(fp);

    return 0;
}

static void get_qca_sta_info_list(char *fname, struct list_head *list)
{
	int i = 0, j;
	char word[256], *next;
	char tmp[128], prefix[] = "wlXXXXXXXXXX_";
    char *name;
    char name_vif[] = "wlX.Y_XXXXXXXXXX";

	foreach(word, nvram_safe_get("wl_ifnames"), next) {
		SKIP_ABSENT_BAND_AND_INC_UNIT(i);
        qca_stainfo(word, fname, list);

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

                qca_stainfo(name_vif, fname, list);
            }
        }
        i++;
	}
}

void fdb_list_parse(char *fname, struct list_head *list)
{
    char cmd[64];
    char buff[1024];
    fdb_node_t fbt, *fb;
    FILE *fp = NULL;

    // "wlanconfig ifname list" to get wireless sta info
    get_qca_sta_info_list(fname, list);

    // "ssdk_sh fdb entry show" is the same as "ssdk_sh sw0 fdb entry show"
    sprintf(cmd, "ssdk_sh fdb entry show > %s", fname);
    //info("%s", cmd);
    system(cmd);

    if((fp = fopen(fname, "r")) == NULL) {
        error("cannot open %s to read..", fname);
        return;
    }

    while (fgets(buff, 1024, fp) != NULL) {
        if(buff[0] != '[' || buff[1] != 'a')
            continue;
        fdb_node_sw_parse(&fbt, buff);
        fb = fdb_node_new();
        list_add_tail(&fb->list, list);
#if defined(RTAX89U)
        fb->sw[0] = fbt.port;
        fb->sw[1] = 0;
        fb->port = rtax89u_get_phy_port(fb);
#elif defined(GTAXY16000)
        fb->sw[0] = fbt.port;
        fb->sw[1] = 0;
        fb->port = gtaxy16000_get_phy_port(fb);
#else
        fb->port = fbt.port;
#endif
        fb->iswl = fbt.iswl;
        strcpy(fb->mac, fbt.mac);
    }
    fclose(fp);

#if defined(RTAX89U) || defined(GTAXY16000)
    sprintf(cmd, "ssdk_sh sw1 fdb entry show > %s", fname);
    //info("%s", cmd);
    system(cmd);

    if((fp = fopen(fname, "r")) == NULL) {
        error("cannot open %s to read..", fname);
        return;
    }

    while (fgets(buff, 1024, fp) != NULL) {
        if(buff[0] != '[' || buff[1] != 'a')
            continue;
        fdb_node_sw_parse(&fbt, buff);
        if (fbt.port == 0) 
            continue;

        fb = fdb_list_search(&fbt, list);
        if (fb) {
            fb->sw[1] = fbt.port;
#if defined(RTAX89U)
            fb->port = rtax89u_get_phy_port(fb);
#elif defined(GTAXY16000)
            fb->port = gtaxy16000_get_phy_port(fb);
#endif
        }
    }
    fclose(fp);
#endif // defined(RTAX89U) || defined(GTAXY16000)

#if defined(NFCMDBG)
    fdb_list_dump(list);
#endif

}

void fdb_list_free(struct list_head *list)
{
	fdb_node_t *fb, *fbt;

	list_for_each_entry_safe(fb, fbt, list, list) {
		list_del(&fb->list);
		fdb_node_free(fb);
	}

	return;
}
