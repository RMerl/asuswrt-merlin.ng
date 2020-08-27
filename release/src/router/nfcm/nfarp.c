#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libnetfilter_conntrack/libnetfilter_conntrack.h>

#include "list.h"
#include "log.h"
#include "nfct.h"
#include "nfarp.h"
#ifdef QCA
  #include "nffdb.h"
#elif HND
  #include "nfmc.h"
#else // for RT-AC68U
  #include "nfrob.h"
#endif

arp_node_t *arp_node_new()
{
    arp_node_t *ar;

    ar = (arp_node_t *)calloc(1, sizeof(arp_node_t));
    if (!ar) return NULL;

    INIT_LIST_HEAD(&ar->list);

    return ar;
}

void arp_node_free(arp_node_t *ar)
{
	if (ar)
		free(ar);

	return;
}

static void arp_set_attr_ip(arp_node_t *ar, char *value)
{
	// value is "(192.168.1.104)"
	char *ipstr = &value[1];

    char str[INET6_ADDRSTRLEN];

	ipstr[strlen(ipstr)-1] = '\0';

    if (ar->isv4) {
        inet_pton(AF_INET, ipstr, &ar->srcv4);
        inet_ntop(AF_INET, &ar->srcv4, str, INET_ADDRSTRLEN);
    } else {
        inet_pton(AF_INET6, ipstr, &ar->srcv6);
    }
}

static void arp_set_attr_mac(arp_node_t *ar, char *value)
{
    // value is "18:31:bf:cf:5d:c5"
    strcpy(ar->mac, value);
}

static void arp_set_attr_port(arp_node_t *ar, struct list_head *list)
{
#ifdef QCA
    fdb_node_t *node;
#elif HND
    mc_node_t *node;
#else
    rob_node_t *node;
#endif

    list_for_each_entry(node, list, list) {
        if (!strcasecmp(node->mac, ar->mac)) {
            ar->iswl = node->iswl;
            ar->port = node->port;
            return;
        }
    }

    ar->port = -1;
    return;
}

int arp_node_parse(arp_node_t *ar, char *buff)
{
	//entry is
	//  ? (192.168.51.105) at 18:31:bf:cf:5d:c6 [ether]  on br0
	int i = 0;
	char *delim = " ";
	char *pch;
	char attrs[ARP_ATTR_MAX][128];

	buff[strlen(buff)-1] = '\0';

	pch = strtok(buff, delim);
	while (pch != NULL) {
		strcpy(attrs[i], pch);
		i++;
		pch = strtok(NULL, delim);
	}

    if (strcmp(attrs[ARP_ATTR_MAC], "<incomplete>") == 0) 
        return -1;

	ar->isv4 = find_dot_in_ip_str(attrs[ARP_ATTR_IP]);
	arp_set_attr_ip(ar, attrs[ARP_ATTR_IP]);

	if (ar->isv4 && is_in_lanv4(&ar->srcv4)) {
		arp_set_attr_mac(ar, attrs[ARP_ATTR_MAC]);
	}

    return 0;
}

void arp_list_dump(struct list_head *arlist)
{
    arp_node_t *ar;
    char ipstr[INET6_ADDRSTRLEN];

    printf("%s:\n", __FUNCTION__);
    list_for_each_entry(ar, arlist, list) {
        inet_ntop(AF_INET, &ar->srcv4, ipstr, INET_ADDRSTRLEN);
        printf("ip:\t%s\n", ipstr);
        printf("mac:\t%s\n", ar->mac);
        printf("iswl:\t%d\n", ar->iswl);
        printf("port:\t%d\n", ar->port);
        printf("================\n");
    }
}

void arp_list_parse(char *fname, struct list_head *arlist, struct list_head *list)
{
	char cmd[64];
	char buff[1024];
	arp_node_t ar, *art;
	FILE *fp = NULL;

    char ipstr[INET6_ADDRSTRLEN];

	sprintf(cmd, "arp -n > %s", fname);
    //info("%s", cmd);
	system(cmd);

    if((fp = fopen(fname, "r")) == NULL) {
        error("cannot open %s to read..", fname);
        return;
    }

    while (fgets(buff, 1024, fp) != NULL) {
        if(buff[0] != '?')
            continue;

        if(arp_node_parse(&ar, buff) < 0) 
            continue;

        inet_ntop(AF_INET, &ar.srcv4, ipstr, INET_ADDRSTRLEN);

		// current only process IPv4
		if (!is_in_lanv4(&ar.srcv4))
			continue;

		art = arp_node_new();
		list_add_tail(&art->list, arlist);
        art->isv4 = ar.isv4;
        strcpy(art->mac, ar.mac);
        if (art->isv4) {
            memcpy(&art->srcv4, &ar.srcv4, sizeof(struct in_addr));
        } else {
            memcpy(&art->srcv6, &ar.srcv6, sizeof(struct in6_addr));
        }
        arp_set_attr_port(art, list);
    }
    fclose(fp);

#if defined(NFCMDBG)
    arp_list_dump(arlist);
#endif

	return;
}

void arp_list_free(struct list_head *list)
{
	arp_node_t *ar, *art;

	list_for_each_entry_safe(ar, art, list, list) {
		list_del(&ar->list);
		arp_node_free(ar);
	}

	return;
}

