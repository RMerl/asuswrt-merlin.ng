#include <stdio.h>
#include <stdlib.h>

#include "log.h"
#include "nfcm.h"
#include "nffc.h"
#include "nfct.h"

// for RT-AX92U
#define SW_PORT_M   (0x07)
#define SW_PORT_S   (0)
#define SW_UNIT_M   (0x08)
#define SW_UNIT_S   (3)

#define LOGICAL_PORT_TO_UNIT_NUMBER(port)   ( ((port) & SW_UNIT_M) >> SW_UNIT_S  )
#define LOGICAL_PORT_TO_PHYSICAL_PORT(port) ( (port) & SW_PORT_M )
#define PHYSICAL_PORT_TO_LOGICAL_PORT(port, unit) ( (port) | ((unit << SW_UNIT_S) & SW_UNIT_M) )
//=======================================================================//

static void nf_set_attr_ipv4_src(nf_node_t *nn, char *value)
{
    //value is the format "<052.098.037.034:00443>"
    char *buf = (char *)value;
	int i;

	nn->isv4 = true;

    char *str = &buf[1];
    str[strlen(str)-1] = '\0';

    char *ip_str = strtok(str, ":");
    //char *port_str = strtok(NULL, ":");

    char s[4][4];
    char *p = strtok(ip_str, ".");
    for(i=0;i<4;i++) {
        strcpy(s[i], p);
        p = strtok(NULL, ".");
    }

    char ipaddr[INET_ADDRSTRLEN];
    sprintf(ipaddr, "%d.%d.%d.%d", atoi(s[0]), atoi(s[1]), atoi(s[2]), atoi(s[3]));

    // store this IP address in nn->src:
    inet_pton(AF_INET, ipaddr, &(nn->srcv4));
    //nn->src.sin_port = atoi(port_str);

    // now get it back and print it
	//char ipstr[INET_ADDRSTRLEN];
    //inet_ntop(AF_INET, &(nn->srcv4), ipstr, INET_ADDRSTRLEN);
    //printf("srcv4=[%s]\n", ipstr);
}

static void nf_set_attr_ipv6_src(nf_node_t *nn, char *value)
{
	nn->isv4 = false;
}

static void nf_set_attr_layer1_type(nf_node_t *nn, char *value)
{
    char *phy = (char *)value;

	//char ipstr[INET6_ADDRSTRLEN];
	//inet_ntop(AF_INET, &nn->srcv4, ipstr, INET_ADDRSTRLEN);
	//if (!strcmp(ipstr, "192.168.51.66")) {
	//	printf("src=[%s], phy=[%s]\n", ipstr, phy);
	//}

    if(!strcmp(phy, "EPHY")) 
		nn->layer1_info.eth_type = PHY_ETHER;
	else if(!strcmp(phy, "WPHY"))  
		nn->layer1_info.eth_type = PHY_WIRELESS;
	else  
		nn->layer1_info.eth_type = PHY_UNKNOWN;

}

static void nf_set_attr_layer1_port(nf_node_t *nn, char *value)
{
	int port = atoi(value);

	if (nn->layer1_info.eth_type == PHY_ETHER) {
		nn->layer1_info.eth_port = LOGICAL_PORT_TO_PHYSICAL_PORT(port);
	} else {
		nn->layer1_info.eth_port = port;
	}
}

int fc_node_parse(nf_node_t *nn, char *buff)
{
	int i = 0;
    char *delim = " ";
    char *pch;
    char attrs[FC_ATTR_MAX][128];

    pch = strtok(buff, delim);
    while (pch != NULL) {
        strcpy(attrs[i], pch);
        i++;
        pch = strtok(NULL, delim);
    }

	nn->isv4 = find_dot_in_ip_str(attrs[FC_ATTR_IPV4_SRC]);
	if (nn->isv4)
		nf_set_attr_ipv4_src(nn, attrs[FC_ATTR_IPV4_SRC]);
	else
		nf_set_attr_ipv6_src(nn, attrs[FC_ATTR_IPV4_SRC]);

	if (nn->isv4 && is_in_lanv4(&nn->srcv4)) {
		nf_set_attr_layer1_type(nn, attrs[FC_ATTR_LAYER1_TYPE]);
		nf_set_attr_layer1_port(nn, attrs[FC_ATTR_LAYER1_PORT]);
	}

    return 0;
}

bool fc_node_search(nf_node_t *fc, struct list_head *fclist)
{
	nf_node_t *nn;

	list_for_each_entry(nn, fclist, list) {
		if (nn->isv4) { // IPv4
			if (nn->srcv4.s_addr == fc->srcv4.s_addr) {
				//memcpy(&nn->layer1_info, &fc->layer1_info, sizeof(phy_port_t));
				//if (nn->layer1_info.eth_port != fc->layer1_info.eth_port)
				//	nn->layer1_info.eth_port = fc->layer1_info.eth_port;
				return true;
			}
		} else { // IPv6
			if (!memcmp(&nn->srcv6, &fc->srcv6, sizeof(struct in6_addr))) {
				//memcpy(&nn->layer1_info, &fc->layer1_info, sizeof(phy_port_t));
				//if (nn->layer1_info.eth_port != fc->layer1_info.eth_port)
				//	nn->layer1_info.eth_port = fc->layer1_info.eth_port;
				return true;
			}
		}
	}

	// not found in fclist
	return false;
}

int fc_list_parse(char *fname, struct list_head *fclist)
{
    char buff[1024];
	nf_node_t fc, *nn;

    FILE *fp = NULL;
    if((fp = fopen(fname, "r")) == NULL) {
        error("cannot open %s to read..", fname);
        return -1;
    }

    while (fgets(buff, 1024, fp) != NULL) {
        if(buff[0] != '0')
            continue;
		// get srcip, layer1_info from /proc/fcache/list
        fc_node_parse(&fc, buff);

		// current only process IPv4
		if (!is_in_lanv4(&fc.srcv4))
			continue;

		// find the fc in fclist, if not found, create one and add it
        if(fc_node_search(&fc, fclist) == false) {
			nn = nf_node_new();
			list_add_tail(&nn->list, fclist);
			memcpy(&nn->layer1_info, &fc.layer1_info, sizeof(phy_port_t));
			nn->isv4 = fc.isv4;
			if(fc.isv4) {
				memcpy(&nn->srcv4, &fc.srcv4, sizeof(struct in_addr));
			} else {
				memcpy(&nn->srcv6, &fc.srcv6, sizeof(struct in6_addr));
			}
		}
    }
    fclose(fp);

	return 0;
}

void fc_list_free(struct list_head *fclist)
{
	nf_node_t *nn, *nnt;

	list_for_each_entry_safe(nn, nnt, fclist, list) {
		list_del(&nn->list);
		nf_node_free(nn);
	}

	return;
}

