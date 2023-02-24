#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <shared.h>
#include <json.h>

#include "nfarp.h"
#include "nfcfg.h"

char* str2lower(char *mac)
{
	char *p = mac;

	for ( ; *p; ++p) 
		*p = tolower(*p);

	return mac;
}

char* str2upper(char *p)
{
	for ( ; *p; ++p) *p = toupper(*p);

	return p;
}

cli_node_t* cli_node_new()
{
	cli_node_t *cli;

	cli = (cli_node_t *)calloc(1, sizeof(cli_node_t));
	if (!cli) return NULL;

	INIT_LIST_HEAD(&cli->list);

	return cli;
}

void cli_node_free(cli_node_t *cli)
{
	if (cli)
		free(cli);

	return;
}

void cli_list_free(struct list_head *list)
{
	cli_node_t *cli, *tmp;

	list_for_each_entry_safe(cli, tmp, list, list) {
		list_del(&cli->list);
		cli_node_free(cli);
	}

	return;
}

int cli_list_size(struct list_head *list)
{
	cli_node_t *cli;
	int cnt = 0;

	list_for_each_entry(cli, list, list) {
		cnt++;
	}

	return cnt;
}

void cli_node_dump(cli_node_t *cli)
{
	char ipstr[INET6_ADDRSTRLEN];

	if (cli->isv4) {
		inet_ntop(AF_INET, &cli->ipv4, ipstr, INET_ADDRSTRLEN);
		printf("ipv4: %s[%u]\n", ipstr, cli->ipv4.s_addr);
	} else {
        inet_ntop(AF_INET6, &cli->ipv6, ipstr, INET6_ADDRSTRLEN);
		printf("ipv6: %s\n", ipstr);
        //printf("ipv6: %s[%u]\n", ipstr, cli->ipv6.s_addr);
	}
	printf("mac:%s\n", cli->mac);
	printf("type:%s\n", cli->type);

	return;
}

void cli_list_dump(char *title, struct list_head *list)
{
	cli_node_t *cli;

	printf("[%s]%s: %s, count=[%d]\n", __FILE__, __FUNCTION__, title, cli_list_size(list));
	list_for_each_entry(cli, list, list) {
		cli_node_dump(cli);
	}
	printf("=======================\n");

	return;
}

bool cli_node_compare_mac(cli_node_t *cli, char *mac)
{
    
    if (!strcasecmp(cli->mac, mac)) {
        return true;
    }

    return false;
}

cli_node_t *cli_list_find_mac(struct list_head *list, char *mac)
{
    bool ret = false;
    cli_node_t *cli;

    list_for_each_entry(cli, list, list) {
        ret = cli_node_compare_mac(cli, mac);
        if (ret) {
            return cli;
        }
    }

    return NULL;
}

void cli_list_file_parse(struct list_head *cli_list)
{
	int lock;
	json_object *clietListObj = NULL;
	json_object *brMacObj = NULL;
	json_object *clientObj = NULL;
	json_object *infoObj = NULL;
	char mac[ETHER_ADDR_LENGTH] = {0};
	char type[ETHER_ADDR_LENGTH] = {0};
	char ip[INET_ADDRSTRLEN] = {0};
	char dut[ETHER_ADDR_LENGTH] = {0};
	cli_node_t *cli = NULL;

	cli_list_free(cli_list);

	// dut mac address
	snprintf(dut, sizeof(dut), "%s", get_lan_hwaddr());

	lock = file_lock(CLIENTLIST_FILE_LOCK);
	clietListObj = json_object_from_file(CLIENTLIST_JSON_PATH);
	if (clietListObj) {
		json_object_object_foreach(clietListObj, key, val) {
			// skip CAP's device
			if (!strcasecmp(key, dut))
				continue;

			brMacObj = val;
			//printf(" 1. clientListObj key=%s\n", key);
			json_object_object_foreach(brMacObj, key, val) {
				clientObj = val;
				memset(type, 0, sizeof(type));
				snprintf(type, sizeof(type), "%s", key);
				//printf(" 2. brMacObj key=%s\n", key);
				json_object_object_foreach(clientObj, key, val) {
					infoObj = val;
					//printf(" 3. clientObj key=%s\n", key);
					memset(mac, 0, sizeof(mac));
					snprintf(mac, sizeof(mac), "%s", key);
					str2lower(mac);
					json_object_object_foreach(infoObj, key, val) {
						if (!strcmp(key, "ip")) {
							memset(ip, 0, sizeof(ip));
							snprintf(ip, sizeof(ip), "%s", json_object_get_string(val));
							if (strcmp(ip, "")) {
								//printf(" mac=%s, ip=%s\n", mac, ip);
								cli = cli_node_new();
								if (cli) {
									list_add_tail(&cli->list, cli_list);
									cli->isv4 = is_v4_addr(ip);
									strncpy(cli->mac, mac, sizeof(cli->mac));
									if (cli->isv4) {
										inet_pton(AF_INET, ip, &cli->ipv4);
										//cli->ipv6 = 0;
									} else {
										inet_pton(AF_INET6, ip, &cli->ipv6);
										//cli->ipv4 = 0;
									}
									strncpy(cli->type, type, sizeof(cli->type));
								}
								//is_re_device_full(mac, ip);
							}
						}
					}
				}
			}
		}
#if defined(NFCMDBG)
		cli_list_dump("ClientList", cli_list);
#endif
	}

	json_object_put(clietListObj);
	file_unlock(lock);

#if defined(NFCMDBG)
	printf(" Leave.\n");
#endif
	return;
}

