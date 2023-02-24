#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>

#include "list.h"
#include "nfdev.h"


nfdev_node_t* nfdev_node_new()
{
    nfdev_node_t *dev;

    dev = (nfdev_node_t *)calloc(1, sizeof(nfdev_node_t));
    if (!dev) return NULL;

    INIT_LIST_HEAD(&dev->list);

    return dev;
}

void nfdev_node_free(nfdev_node_t *dev)
{
    if (dev) free(dev);

    return;
}

void nfdev_list_dump(struct list_head *list)
{
    nfdev_node_t *dev;
    char ip_str[INET6_ADDRSTRLEN];

    printf("%s:\n", __FUNCTION__);
    list_for_each_entry(dev, list, list) {
        if(dev->isv4)
        {
          inet_ntop(AF_INET, (struct in_addr *)&dev->client_ip, ip_str, INET_ADDRSTRLEN);
          dnsdbg("v4 client_ip:%s", ip_str);

        } else {
          inet_ntop(AF_INET6, &dev->srcv6, ip_str, INET6_ADDRSTRLEN);
          dnsdbg("v6 client_ip:%s", ip_str);
        }
        dnsdbg("timestamp:%d", dev->timestamp);
        dnsdbg("tx:%llu", dev->tx);
        dnsdbg("rx:%llu", dev->rx);
        dnsdbg("domain_name:%s", dev->domain_name);
        dnsdbg("mac:%s", dev->mac);
        dnsdbg("dev_type:%s", dev->dev_type);
        dnsdbg("count:%llu", dev->count);        
    }
}

void nfdev_list_free(struct list_head *list)
{
    nfdev_node_t *dev,*devt;

    list_for_each_entry_safe(dev, devt, list, list) {
        list_del(&dev->list);
        nfdev_node_free(dev);
    }

    return;
}
