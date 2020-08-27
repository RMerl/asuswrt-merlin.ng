#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <arpa/inet.h>
#include <json.h>

#include "log.h"
#include "nfcm.h"
#include "nfct.h"
#include "nfarp.h"
#include "nfjs.h"

#define JSON_OUTPUT_APP_FILE "/jffs/nfcm_app.json"
#define JSON_OUTPUT_SUM_FILE "/jffs/nfcm_sum.json"

int get_eth_type(char *ethtype, PHY_TYPE etype)
{
	switch (etype) {
	case PHY_ETHER:
		strcpy(ethtype, "EPHY");
		break;
	case PHY_WIRELESS:
		strcpy(ethtype, "WPHY");
		break;
	default:
		strcpy(ethtype, "UNKNOWN");
		break;
	}

	return 0;
}

int get_eth_type_from_arp(nf_node_t *nn, struct list_head *arlist)
{
    arp_node_t *ar;

    list_for_each_entry(ar, arlist, list) {
        if (ar->srcv4.s_addr == nn->srcv4.s_addr) {
            nn->layer1_info.eth_type = (ar->iswl) ? PHY_WIRELESS : PHY_ETHER;
            nn->layer1_info.eth_port = ar->port;
            return 0;
        }
    }

    return 0;
}

struct json_object *nf_node_to_json(nf_node_t *nn, struct list_head *arlist)
{
	char ipstr[INET6_ADDRSTRLEN];
	char ethtype[10];

	struct json_object *node_obj = json_object_new_object();
	if (node_obj == NULL)  {
		printf("new json object failed.\n");
		return NULL;
	}

	struct json_object *phy_obj = json_object_new_object();
	if (phy_obj == NULL)  {
		json_object_put(node_obj); //free
		printf("new json object failed.\n");
		return NULL;
	}

#if defined(NFCMDBG)
	nf_node_dump(nn);
#endif

    json_object_object_add(node_obj, "proto", json_object_new_int(nn->proto));
	if (nn->isv4) {
		inet_ntop(AF_INET, &nn->srcv4, ipstr, INET_ADDRSTRLEN);
	} else {
		inet_ntop(AF_INET6, &nn->srcv6, ipstr, INET6_ADDRSTRLEN);
	}
	json_object_object_add(node_obj, "src_ip", json_object_new_string(ipstr));
    json_object_object_add(node_obj, "src_port", json_object_new_int(nn->src_port));

    if (nn->isv4) {
        inet_ntop(AF_INET, &nn->dstv4, ipstr, INET_ADDRSTRLEN);
    } else {
        inet_ntop(AF_INET6, &nn->dstv6, ipstr, INET6_ADDRSTRLEN);
    }
    json_object_object_add(node_obj, "dst_ip", json_object_new_string(ipstr));
    json_object_object_add(node_obj, "dst_port", json_object_new_int(nn->dst_port));

    json_object_object_add(node_obj, "up_pkts", json_object_new_int64(nn->up_pkts));
	json_object_object_add(node_obj, "up_bytes", json_object_new_int64(nn->up_bytes));
	json_object_object_add(node_obj, "up_speed", json_object_new_int64(nn->up_speed));

	json_object_object_add(node_obj, "dn_pkts", json_object_new_int64(nn->dn_pkts));
	json_object_object_add(node_obj, "dn_bytes", json_object_new_int64(nn->dn_bytes));
	json_object_object_add(node_obj, "dn_speed", json_object_new_int64(nn->dn_speed));

    if (nn->layer1_info.eth_type == PHY_UNKNOWN) 
        get_eth_type_from_arp(nn, arlist);
	get_eth_type(ethtype, nn->layer1_info.eth_type);
	json_object_object_add(phy_obj, "type", json_object_new_string(ethtype));
	json_object_object_add(phy_obj, "port", json_object_new_int(nn->layer1_info.eth_port));
	json_object_object_add(node_obj, "L1-info", phy_obj);

	return node_obj;
}

int nf_list_to_json(struct list_head *iplist, struct list_head *arlist)
{
	nf_node_t *nn;
	struct json_object *json_obj;

	struct json_object *ary_obj = json_object_new_array();
	if (ary_obj == NULL) {
		return -1;
	}

	list_for_each_entry(nn, iplist, list) {
		if (nn->isv4 && !is_in_lanv4(&nn->srcv4))
				continue;
		json_obj = nf_node_to_json(nn, arlist);
		json_object_array_add(ary_obj, json_obj);
	}

	/* print it */
	json_object_to_file(JSON_OUTPUT_APP_FILE, ary_obj);

	// free the root object will also free its descending tree nodes
	json_object_put(ary_obj);

	return 0;
}

struct json_object *nf_node_statistics_to_json(nf_node_t *nn, struct list_head *arlist)
{
	char ipstr[INET6_ADDRSTRLEN];
	char ethtype[10];

	struct json_object *node_obj = json_object_new_object();
	if (node_obj == NULL)  {
		printf("new json object failed.\n");
		return NULL;
	}

	struct json_object *phy_obj = json_object_new_object();
	if (phy_obj == NULL)  {
		json_object_put(node_obj); //free
		printf("new json object failed.\n");
		return NULL;
	}

#if defined(NFCMDBG)
	nf_node_dump(nn);
#endif

	if (nn->isv4) {
		inet_ntop(AF_INET, &nn->srcv4, ipstr, INET_ADDRSTRLEN);
	} else {
		inet_ntop(AF_INET6, &nn->srcv6, ipstr, INET6_ADDRSTRLEN);
	}
	json_object_object_add(node_obj, "src_ip", json_object_new_string(ipstr));

    json_object_object_add(node_obj, "up_pkts", json_object_new_int64(nn->up_pkts));
	json_object_object_add(node_obj, "up_bytes", json_object_new_int64(nn->up_bytes));
    json_object_object_add(node_obj, "up_ttl_bytes", json_object_new_int64(nn->up_ttl_bytes));
    json_object_object_add(node_obj, "up_speed", json_object_new_int64(nn->up_speed));

	json_object_object_add(node_obj, "dn_pkts", json_object_new_int64(nn->dn_pkts));
	json_object_object_add(node_obj, "dn_bytes", json_object_new_int64(nn->dn_bytes));
    json_object_object_add(node_obj, "dn_ttl_bytes", json_object_new_int64(nn->dn_ttl_bytes));
    json_object_object_add(node_obj, "dn_speed", json_object_new_int64(nn->dn_speed));

    if (nn->layer1_info.eth_type == PHY_UNKNOWN) 
        get_eth_type_from_arp(nn, arlist);
    get_eth_type(ethtype, nn->layer1_info.eth_type);
	json_object_object_add(phy_obj, "type", json_object_new_string(ethtype));
	json_object_object_add(phy_obj, "port", json_object_new_int(nn->layer1_info.eth_port));
	json_object_object_add(node_obj, "L1-info", phy_obj);

	return node_obj;
}


int nf_list_statistics_to_json(struct list_head *smlist, struct list_head *arlist)
{
	nf_node_t *nn;
	struct json_object *json_obj;

    struct json_object *ary_obj = json_object_new_array();
    if (ary_obj == NULL) 
        return -1;

    // dump to json file based on smlist
    list_for_each_entry(nn, smlist, list) {
        json_obj = nf_node_statistics_to_json(nn, arlist);
        json_object_array_add(ary_obj, json_obj);
    }

    /* print it */
    json_object_to_file(JSON_OUTPUT_SUM_FILE, ary_obj);

    // free the root object will also free its descending tree nodes
    json_object_put(ary_obj);

	return 0;
}
