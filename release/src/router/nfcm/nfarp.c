#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <shared.h>

#include "list.h"
#include "log.h"
#include "nfct.h"
#include "nfarp.h"

#include "nfsw.h"

#define ARP_CACHE       "/proc/net/arp"
#define ARP_BUFFER_LEN  512
#define IPLEN           16

#define GET_IPV6_MAC_SYS_COMMAND "ip -6 neigh show"
#define IP6_LINE_FORMAT "%s %s %s %s %s %s %s"

#define LOCAL_LINK_ADDR 0xfe80

arp_node_t* arp_node_new()
{
    arp_node_t *ar;

    ar = (arp_node_t *)calloc(1, sizeof(arp_node_t));
    if (!ar)
        return NULL;

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

//    char str[INET6_ADDRSTRLEN];

    ipstr[strlen(ipstr) - 1] = '\0';

    if (ar->is_v4) {
        inet_pton(AF_INET, ipstr, &ar->srcv4);
//        inet_ntop(AF_INET, &ar->srcv4, str, INET_ADDRSTRLEN);
    } else {
        inet_pton(AF_INET6, ipstr, &ar->srcv6);
    }
}

static void arp_set_attr_mac(arp_node_t *ar, char *value)
{
    // value is "18:31:bf:cf:5d:c5"
    strcpy(ar->mac, value);
}

static void arp_set_attr_port(arp_node_t *ar, sw_node_t *sw)
{

    ar->is_wl = sw->is_wl;
    ar->port = sw->port;

    ar->is_guest = sw->is_guest;
    memcpy(ar->ifname, sw->ifname, IFNAMESIZE);

    return;

#if 0
    nf_printf("\n--------------------------------------\n"
              "[%s]%s: arp_mac=[%s] cannot find in swlist"
              "\n--------------------------------------\n",
              __FILE__, __FUNCTION__, ar->mac);

    ar->port = -1;
    ar->is_wl = false;

    return;
#endif
}

int arp_node_parse(arp_node_t *ar, char *buff)
{
    //entry is
    //  ? (192.168.51.105) at 18:31:bf:cf:5d:c6 [ether]  on br0
    int i = 0;
    char *delim = " ";
    char *pch;
    char attrs[ARP_ATTR_MAX][128];

    buff[strlen(buff) - 1] = '\0';

    pch = strtok(buff, delim);
    while (pch != NULL) {
        strcpy(attrs[i], pch);
        i++;
        pch = strtok(NULL, delim);
    }

    printf("mac=[%s]\n", attrs[ARP_ATTR_MAC]);
    if (strcmp(attrs[ARP_ATTR_MAC], "<incomplete>") == 0) return -1;

    ar->is_v4 = find_dot_in_ip_str(attrs[ARP_ATTR_IP]);
    arp_set_attr_ip(ar, attrs[ARP_ATTR_IP]);

    if (ar->is_v4 && is_in_lanv4(&ar->srcv4) && !is_router_addr(&ar->srcv4)) {
        arp_set_attr_mac(ar, attrs[ARP_ATTR_MAC]);
    }

    return 0;
}

void arp_node_dump(arp_node_t *ar)
{
    char ipstr[INET6_ADDRSTRLEN];

    if(ar->is_v4)
    	inet_ntop(AF_INET, &ar->srcv4, ipstr, INET_ADDRSTRLEN);
    else
    	inet_ntop(AF_INET6, &ar->srcv6, ipstr, INET6_ADDRSTRLEN);
   
    nf_printf("is_v4:\t%d\n", ar->is_v4);
    nf_printf("ifname:\t%s\n", ar->ifname);
    nf_printf("ip:\t%s\n", ipstr);
    nf_printf("mac:\t%s\n", ar->mac);
    nf_printf("is_wl:\t%d\n", ar->is_wl);
    nf_printf("guest:\t%d\n", ar->is_guest);
    nf_printf("port:\t%d\n", ar->port);
    nf_printf("================\n");
}

void arp_list_dump(struct list_head *arlist)
{
    arp_node_t *ar;

    nf_printf("%s:\n", __FUNCTION__);
    list_for_each_entry(ar, arlist, list) {
        arp_node_dump(ar);
    }
}

sw_node_t *arp_list_find_sw_node(char *mac, struct list_head *swlist)
{
    sw_node_t *sw;

    list_for_each_entry(sw, swlist, list) {
        if (!strncmp(sw->mac, mac, ETHER_ADDR_LENGTH))
            return sw;
    }

    return NULL;
}

bool is_v4_addr(char *ip)
{  
   bool is_v4 = false;
   if(strstr(ip, ":") == NULL && strstr(ip, ".") != NULL)
	is_v4 = true;
  
   return is_v4;	
}

/* need 1/3/4/6 fields */
// 192.168.89.189 0x1 0x2 00:0e:c7:9e:27:93 * br0
#define ARP_LINE_FORMAT "%s %s %s %s %s %s"

void arp_list_parse(struct list_head *arlist, struct list_head *swlist)
{
    arp_node_t *ar;
    sw_node_t *sw;
    struct in_addr srcv4;
    struct in6_addr srcv6;
    char ipstr[INET6_ADDRSTRLEN], flag[10];
    char mac[ETHER_ADDR_LENGTH], device[IFNAMESIZE];
    char hw_type[20], mask[20];
    bool is_v4;
    char dev[20], br[20], lladdr[20], status[20], status_ext[20], ip6str[INET6_ADDRSTRLEN];
    int tokens;
    char line[ARP_BUFFER_LEN]; 

    FILE *fp = fopen(ARP_CACHE, "r");
    if (!fp) {
        perror("cannot open arp cache");
        return;
    }

    char header[ARP_BUFFER_LEN];
    if (!fgets(header, sizeof(header), fp)) {
        fclose(fp);
        return;
    }

    while (fscanf(fp, ARP_LINE_FORMAT, ipstr, hw_type, flag, mac, mask, device) == 6) {
        if (strtoul(flag, NULL, 0) == 0x00) // not complete
            continue;
        is_v4 = is_v4_addr(ipstr);
        if(is_v4)
        {
	  inet_pton(AF_INET, ipstr, &srcv4);
          if (!is_in_lanv4(&srcv4))
             continue;
        }
        else  
        {
	  if(!inet_pton(AF_INET6, ipstr, &srcv6))
             continue;
        
        }

        ar = arp_node_new();
        list_add_tail(&ar->list, arlist);

        ar->is_v4 = is_v4;
        memcpy(ar->mac, mac, ETHER_ADDR_LENGTH);
        if(ar->is_v4)
	  memcpy(&ar->srcv4, &srcv4, sizeof(struct in_addr));
        else
          memcpy(&ar->srcv6, &srcv6, sizeof(struct in6_addr));

        sw = arp_list_find_sw_node(mac, swlist);
        if (!sw) continue;

        arp_set_attr_port(ar, sw);
    }

    fclose(fp);


    //TODO: ipv6
    fp = popen(GET_IPV6_MAC_SYS_COMMAND, "r");
    if (!fp)
    {
      perror("cannot open arp cache");
      return;
    }


    while (fgets(line, sizeof(line), fp) != NULL) {
        
        tokens = sscanf(line, IP6_LINE_FORMAT, ip6str, dev, br, lladdr, mac, status, status_ext);
        if(tokens < 6)
        {
            memset(ip6str, 0, INET6_ADDRSTRLEN);
            memset(mac, 0, sizeof(mac));
            memset(status, 0, sizeof(status));
            memset(status_ext, 0, sizeof(status_ext));
            continue;
        } 

	if(!inet_pton(AF_INET6, ip6str, &srcv6))
             continue;
        if(srcv6.s6_addr[0] == 0xfe && (srcv6.s6_addr[1] & 0x80) == 0x80) 
             continue;
        ar = arp_node_new();
        list_add_tail(&ar->list, arlist);

        ar->is_v4 = false;
        memcpy(ar->mac, mac, ETHER_ADDR_LENGTH);
        memcpy(&ar->srcv6, &srcv6, sizeof(struct in6_addr));

        sw = arp_list_find_sw_node(mac, swlist);
        if (!sw) continue;

        arp_set_attr_port(ar, sw);
    }

    pclose(fp);

#if defined(NFCMDBG)
    arp_list_dump(arlist);
#endif

    return;
}

void arp_list_free(struct list_head *list)
{
    arp_node_t * ar,*art;

    list_for_each_entry_safe(ar, art, list, list) {
        list_del(&ar->list);
        arp_node_free(ar);
    }

    return;
}


