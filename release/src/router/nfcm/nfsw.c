#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <utils.h>

#include <shutils.h>
#include <shared.h>
#include <errno.h>

#include "nfsw.h"

sw_node_t* sw_node_new()
{
    sw_node_t *sw;

    sw = (sw_node_t *)calloc(1, sizeof(sw_node_t));
    if (!sw) return NULL;

    INIT_LIST_HEAD(&sw->list);

    return sw;
}

void sw_node_free(sw_node_t *sw)
{
    if (sw) free(sw);

    return;
}

void sw_list_dump(struct list_head *list)
{
    sw_node_t *sw;

    nf_printf("%s:\n", __FUNCTION__);
    list_for_each_entry(sw, list, list) {
        nf_printf("ifname:\t%s\n", sw->ifname);
        nf_printf("is_wl:\t%d\n", sw->is_wl);
        nf_printf("is_guest:\t%d\n", sw->is_guest);
        nf_printf("mac:\t%s\n", sw->mac);
        nf_printf("port:\t%d\n", sw->port);
        nf_printf("--------------\n");
    }
}

void sw_list_free(struct list_head *list)
{
    sw_node_t *sw,*swt;

    list_for_each_entry_safe(sw, swt, list, list) {
        list_del(&sw->list);
        sw_node_free(sw);
    }

    return;
}

#define MAX_LAN_PORTS 16
int nfcm_get_lan_ports(int port)
{
    int ports[MAX_LAN_PORTS];
    //int sw = get_switch(); // final call get_switch_model()
    int model = get_model(); //model was defined in router/shared/model.h

    switch (model) {
    case MODEL_GTAC5300:
        /*       BRCM_LAN          |           BCM53134        */
        ports[1] = 1; ports[0] = 2; ports[13] = 3; ports[12] = 4;
        ports[3] = 5; ports[2] = 6; ports[11] = 7; ports[10] = 8;
        break;

    case MODEL_RTAC5300:
        ports[1] = 1; ports[2] = 2; ports[3] = 3; ports[4] = 4;
        break;

    case MODEL_RTAC88U:
        ports[0] = 4; ports[1] = 3; ports[2] = 2; ports[3] = 1;
        //RLK
        ports[10] = 5; ports[11] = 6; ports[12] = 7; ports[13] = 8;
        break;

    case MODEL_RTAC3100:
        ports[0] = 4; ports[1] = 3; ports[2] = 2; ports[3] = 1;
        break;

    case MODEL_RTAC68U:
        //port[0] is WAN
        ports[1] = 1; ports[2] = 2; ports[3] = 3; ports[4] = 4;
        break;

    case MODEL_RTAX88U:
        // internal switch
        ports[0] = 4; ports[1] = 3; ports[2] = 2; ports[3] = 1;
        // external switch BCM53134
        ports[13] = 5; ports[12] = 6; ports[11] = 7; ports[10] = 8;
        break;

    case MODEL_GTAX11000:
        ports[0] = 4; ports[1] = 3; ports[2] = 2; ports[3] = 1;
        ports[7] = 5; // 2.5G LAN port
        break;

    case MODEL_RTAX95Q:
        ports[0] = 1; ports[1] = 2; ports[2] = 3; //ports[3] = 3;
        break;

    case MODEL_RTAX92U:
        ports[0] = 4; ports[1] = 3; ports[2] = 2; ports[3] = 1;
        break;

    case MODEL_RTAX56_XD4:  // have problem
        /*
            0 1 W0 L1
         */
        if (nvram_match("HwId", "A") || nvram_match("HwId", "C")) {
            ports[0] = 0; ports[1] = 1;
        } else {
            ports[0] = 0;;
        }
        break;

    case MODEL_GTAXE11000:
        ports[0] = 1; ports[1] = 3; ports[2] = 4; ports[3] = 2;
        ports[7] = 5; // 2.5G LAN port
        break;

    case MODEL_RTAC95U: //QCA
        ports[2] = 3; ports[3] = 2; ports[4] = 1;
        break;

    case MODEL_RTAX89U: //QCA
        return port;

    case MODEL_PLAX56XP4: //QCA
        ports[3] = 1; ports[4] = 2;
        break;

    case MODEL_RTAX86U:
        ports[0] = 4; ports[1] = 3; ports[2] = 2; ports[3] = 1;
        break;

    case MODEL_RTAX58U:
        ports[0] = 4; ports[1] = 3; ports[2] = 2; ports[3] = 1;
        break;

    case MODEL_RTAX55:
        ports[0] = 1; ports[1] = 2; ports[2] = 3; ports[3] = 4;
        break;

    case MODEL_RTAX56U:
        ports[0] = 4; ports[1] = 3; ports[2] = 2; ports[3] = 1;
        break;

    default:
        break;
    }

    return ports[port];
}

#if defined(CONFIG_ET)
// switch age time out in seconds
int bcm5301x_age_timeout(int tmo)
{
    return 0;

    //et -i eth0 robowr 0x02 0x06 0x10000A 4
    int s;
    char ifname[] = "eth0";
    struct ifreq ifr;
    int vecarg[3];

    if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket");
        return -1;
    }

    if (ifname) {
        strncpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name));
        ifr.ifr_name[sizeof(ifr.ifr_name)-1] = '\0';
    } else {
        perror("ifname");
        close(s);
        return -1;
    }

    /* only 1, 2, 4, 6, and 8 bytes are valid */
    vecarg[1] = 4;  // set to 4 bytes

    // SPTAGT, see BCM5301x SPI Register document
    vecarg[0] = 0x02 << 16;               // SPI Page
    vecarg[0] |= 0x06 & 0xffff;           // SPI Offset

    // set bit 20 to 1 means 'change aging timer'
    vecarg[2] = 1 << 20;
    // set bit 19:0 to tmo means time out set to tmo
    vecarg[2] |= tmo & 0xfffff;

    ifr.ifr_data = (caddr_t) vecarg;
    if (ioctl(s, SIOCSETCROBOWR, (caddr_t)&ifr) < 0) {
        perror("etcswmctblext");
        close(s);
        return -1;
    }

    close(s);

    return 0;
}

int bcm_arl_dump_5301x(ethsw_mac_table *emt)
{
    int s, i;
    et_var_t var;
    char ifname[] = "eth0";
    struct ifreq ifr;
    char macstr[18];

    if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket");
        return -1;
    }

    if (ifname) {
        strncpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name));
        ifr.ifr_name[sizeof(ifr.ifr_name)-1] = '\0';
    } else {
        perror("ifname");
        close(s);
        return -1;
    }

    var.set = 0;
    var.cmd = IOV_SHOW_MAC_TBL;
    var.buf = emt;
    var.len = sizeof(ethsw_mac_table);

    ifr.ifr_data = (caddr_t) &var;
    if (ioctl(s, SIOCSETGETVAR, (caddr_t)&ifr) < 0) {
        perror("etcswmctblext");
        close(s);
        return -1;
    }

#if 0
    for (i=0; i < emt->count; i++) {
        printf("B: %2d %2d %s [%2d]\n", emt->entry[i].vid,
               emt->entry[i].port, mac2str(emt->entry[i].mac, macstr),
               nfcm_get_lan_ports(emt->entry[i].port));
    }
#endif

    close(s);
    return 0;
}
#endif // defined(CONFIG_ET)

#if defined(RTCONFIG_EXT_RTL8365MB) || defined(RTCONFIG_EXT_RTL8370MB)
#define RTKSWITCH_DEV	"/dev/rtkswitch"

#if defined(RTCONFIG_EXT_RTL8370MB)
#define MAX_RTL_PORTS 8
#else
#define MAX_RTL_PORTS 4
#endif

void show_port_mactable(int port, mac_table_t *tbl, struct list_head *swlist)
{
    int i;
    char macstr[18];
    sw_node_t *sw;
    char *ifname = nvram_get("lan_ifname");

    for (i = 0; i < tbl->count; i++) {
        printf("R%1d: %16s %3d %18s %0d %5d %4d %4d\n", i + 1, ifname, tbl->vid,
               mac2str(tbl->ea[i].octet, macstr), false, false, tbl->port,
               nfcm_get_lan_ports(tbl->port + 10));

        sw = sw_node_new();
        list_add_tail(&sw->list, swlist);

        sw->is_wl = false;
        sw->is_guest = false;
        memcpy(sw->ifname, ifname, IFNAMESIZE);
        sw->port = nfcm_get_lan_ports(tbl->port + 10);
        strcpy(sw->mac, macstr);
    }
}

int rtkswitch_arl_dump_port_mac(int port, struct list_head *swlist)
{
	int fd;
	int *p = NULL;
	mac_table_t mactable;

    if ((port < 0) || (port > 4)) {
        printf("error port %d, must 1..4\n", port);
		return -1;
	}

	fd = open("/dev/rtkswitch", O_RDONLY);
	if (fd < 0) {
		perror("/dev/rtkswitch");
        return -1;
	} else {
		memset(&mactable, 0, sizeof(mac_table_t));
		//p = (int *) &mactable;
		//*p = port - 1;
		if (ioctl(fd, GET_PORT_MAC, &mactable) < 0) {
			perror("rtkswitch ioctl");
			close(fd);
            return -1;
		} else {
			show_port_mactable(port, &mactable, swlist);
        }

		close(fd);
	}

	return 0;
}
#endif //defined(RTCONFIG_EXT_RTL8365MB) || defined(RTCONFIG_EXT_RTL8370MB)
