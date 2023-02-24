#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>

#include "list.h"
#include "appstats.h"

appstats_node_t* appstats_node_new()
{
    appstats_node_t *ap;

    ap = (appstats_node_t *)calloc(1, sizeof(appstats_node_t));
    if (!ap) return NULL;

    INIT_LIST_HEAD(&ap->list);

    return ap;
}

void appstats_node_free(appstats_node_t *ap)
{
    if (ap) free(ap);

    return;
}

void appstats_list_dump(struct list_head *list)
{
    appstats_node_t *ap;
    char ip_str[INET6_ADDRSTRLEN];

    printf("%s:\n", __FUNCTION__);
    list_for_each_entry(ap, list, list) {
#if 1 
        if(ap->isv4)
        {
          inet_ntop(AF_INET, (struct in_addr *)&ap->client_ip, ip_str, INET_ADDRSTRLEN);
          dnsdbg("v4 client_ip:%u", ip_str);
        } else {

          inet_ntop(AF_INET6, &ap->srcv6, ip_str, INET6_ADDRSTRLEN);
          dnsdbg("v6 client_ip:%u", ip_str);
        }
        dnsdbg("up_bytes:%llu", ap->up_bytes);
        dnsdbg("dn_bytes:%llu", ap->dn_bytes);
        dnsdbg("up_dif_bytes:%llu", ap->up_dif_bytes);
        dnsdbg("dn_dif_bytes:%llu", ap->dn_dif_bytes);
        dnsdbg("name:%s", ap->name);
#endif        
    }
}

void appstats_list_free(struct list_head *list)
{
    appstats_node_t *ap,*apt;

    list_for_each_entry_safe(ap, apt, list, list) {
        list_del(&ap->list);
        appstats_node_free(ap);
    }    
    return;

}
