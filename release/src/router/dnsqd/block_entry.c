#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>

#include "list.h"
#include "block_entry.h"

block_entry_node_t* block_entry_node_new()
{
    block_entry_node_t *node;

    node = (block_entry_node_t *)calloc(1, sizeof(block_entry_node_t));
    if (!node) return NULL;

    INIT_LIST_HEAD(&node->list);

    return node;
}

void block_entry_node_free(block_entry_node_t *node)
{
    if (node) free(node);

    return;
}

void block_entry_list_dump(struct list_head *list)
{
    block_entry_node_t *node;

    printf("%s:\n", __FUNCTION__);
    list_for_each_entry(node, list, list) {
        dnsdbg("name:%s", node->name);     
        dnsdbg("type:%d", node->type);       
    }
}

void block_entry_list_free(struct list_head *list)
{
    block_entry_node_t *node,*nodet;

    list_for_each_entry_safe(node, nodet, list, list) {
        list_del(&node->list);
        block_entry_node_free(node);
    }

    return;
}
