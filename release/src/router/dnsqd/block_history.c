#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>


#include "list.h"
#include "block_history.h"

block_history_node_t* block_history_node_new()
{
    block_history_node_t *node;

    node = (block_history_node_t *)calloc(1, sizeof(block_history_node_t));
    if (!node) return NULL;

    INIT_LIST_HEAD(&node->list);

    return node;
}

void block_history_node_free(block_history_node_t *node)
{
    if (node) free(node);

    return;
}

void block_history_list_dump(struct list_head *list)
{
    block_history_node_t *node;
    char ip_str[INET6_ADDRSTRLEN];

    printf("%s:\n", __FUNCTION__);
    list_for_each_entry(node, list, list) {
        if(node->isv4)
        {
          inet_ntop(AF_INET, (struct in_addr*)&node->client_ip, ip_str, INET_ADDRSTRLEN);
          dnsdbg("v4 src_ip:%u", ip_str);
        } else {
          inet_ntop(AF_INET6, &node->srcv6, ip_str, INET6_ADDRSTRLEN);
          dnsdbg("v6 src_ip:%u", ip_str);
        }
        dnsdbg("timestamp:%d", node->timestamp);
        dnsdbg("name:%s", node->name);     
    }
}

void block_history_list_free(struct list_head *list)
{
    block_history_node_t *node,*nodet;

    list_for_each_entry_safe(node, nodet, list, list) {
        list_del(&node->list);
        block_history_node_free(node);
    }

    return;
}
