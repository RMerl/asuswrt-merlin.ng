#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>

#include "list.h"
#include "nfapp.h"

nfapp_node_t* nfapp_node_new()
{
    nfapp_node_t *ap;

    ap = (nfapp_node_t *)calloc(1, sizeof(nfapp_node_t));
    if (!ap) return NULL;

    INIT_LIST_HEAD(&ap->list);

    return ap;
}

void nfapp_node_free(nfapp_node_t *ap)
{
    if (ap) free(ap);

    return;
}

void nfapp_list_dump(struct list_head *list)
{
    nfapp_node_t *ap;
    char ip_str[INET6_ADDRSTRLEN];

    printf("%s:\n", __FUNCTION__);
    list_for_each_entry(ap, list, list) {
        dnsdbg("data_id:%d", ap->data_id);
        dnsdbg("timestamp:%ld", ap->timestamp);
        dnsdbg("ipv4: %d", ap->isv4);
        if(ap->isv4)
        {
          inet_ntop(AF_INET, (struct in_addr *)&ap->src_ip, ip_str, INET_ADDRSTRLEN);
          dnsdbg("v4 src_ip:%s", ip_str);
          inet_ntop(AF_INET, (struct in_addr *)&ap->dst_ip, ip_str, INET_ADDRSTRLEN);
          dnsdbg("v4 dst_ip:%s", ip_str);
        } else {
          inet_ntop(AF_INET6, &ap->srcv6, ip_str, INET6_ADDRSTRLEN);
          dnsdbg("v6 src_ip:%s", ip_str);
          inet_ntop(AF_INET6, &ap->dstv6, ip_str, INET6_ADDRSTRLEN);
          dnsdbg("v6 dst_ip:%s", ip_str);
        }
        dnsdbg("src_port:%u", ap->src_port);
        dnsdbg("dst_port:%u", ap->dst_port);
        dnsdbg("up_bytes:%llu", ap->up_bytes);
        dnsdbg("up_dif_bytes:%llu", ap->up_dif_bytes);
        dnsdbg("dn_bytes:%llu", ap->dn_bytes);
        dnsdbg("dn_dif_bytes:%llu", ap->dn_dif_bytes);
        dnsdbg("tx:%llu", ap->tx);
        dnsdbg("rx:%llu", ap->rx);
        dnsdbg("name:%s", ap->name);
        dnsdbg("mac:%s", ap->mac);
        dnsdbg("app_name:%s", ap->app_name);
    }
}

void nfapp_list_free(struct list_head *list)
{
    nfapp_node_t *ap,*apt;

    list_for_each_entry_safe(ap, apt, list, list) {
        list_del(&ap->list);
        nfapp_node_free(ap);
    }

    return;
}
