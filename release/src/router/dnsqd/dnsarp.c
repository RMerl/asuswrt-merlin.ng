#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>


#include "list.h"
#include "log.h"
#include "dnsarp.h"


#define NET_ADDRSTRLEN 46
#define IFNAMESIZE 16
#define ARP_CACHE "/proc/net/arp"
#define ARP_BUFFER_LEN 512
#define ARP_LINE_FORMAT "%s %s %s %s %s %s"

#define GET_IPV6_MAC_SYS_COMMAND "ip -6 neigh show"
#define IP6_LINE_FORMAT "%s %s %s %s %s %s %s"

#define LOCAL_LINK_ADDR 0xfe80
arp_node_t *arp_node_new()
{
  arp_node_t *ar;

  ar = (arp_node_t *)calloc(1, sizeof(arp_node_t));
  if (!ar)
    return NULL;
  // init
  memset(ar->mac, 0, ETHER_ADDR_LENGTH);
  INIT_LIST_HEAD(&ar->list);

  return ar;
}

void arp_node_free(arp_node_t *ar)
{
  if (ar)
    free(ar);

  return;
}

void arp_node_dump(arp_node_t *ar)
{
  char ipstr[INET6_ADDRSTRLEN];
  if(ar->isv4)
    inet_ntop(AF_INET, &ar->srcv4, ipstr, INET_ADDRSTRLEN);
  else
    inet_ntop(AF_INET6, &ar->srcv6, ipstr, INET6_ADDRSTRLEN);

  dnsdbg("ipv4=%d  ip:\t%s\n", ar->isv4, ipstr);
  dnsdbg("mac:\t%s\n", ar->mac);
}

void arp_list_dump(struct list_head *arlist)
{
  arp_node_t *ar;

  dnsdbg("%s:\n", __FUNCTION__);
  list_for_each_entry(ar, arlist, list)
  {
    arp_node_dump(ar);
  }
}

arp_node_t *arp_list_find_node(arp_node_t *node, struct list_head *arlist)
{
  arp_node_t *arp;

  list_for_each_entry(arp, arlist, list)
  {
    if (!strncmp(arp->mac, node->mac, ETHER_ADDR_LENGTH))
    {  if ((arp->isv4 && (arp->srcv4.s_addr == node->srcv4.s_addr)) || 
           (!arp->isv4 && !memcmp(&arp->srcv6, &node->srcv6, sizeof(struct in6_addr))))
         return arp;
    }
  }

  return NULL;
}

void to_upper(char* string)
{
    const char OFFSET = 'a' - 'A';
    while (*string)
    {
        *string = (*string >= 'a' && *string <= 'z') ? *string -= OFFSET : *string;
        string++;
    }
}

void arp_list_parse(struct list_head *arlist)
{
  arp_node_t *ar, *ar_tmp;
  struct in_addr srcv4;
  struct in6_addr srcv6;
  char ipstr[NET_ADDRSTRLEN], flag[10];
  char mac[ETHER_ADDR_LENGTH], device[IFNAMESIZE];
  char hw_type[20], mask[20];

  char dev[20], br[20],lladdr[20], status[20], status_ext[20];
  char ip6str[INET6_ADDRSTRLEN]; 
  int tokens;
  char line[ARP_BUFFER_LEN];
  
  FILE *fp = fopen(ARP_CACHE, "r");
  if (!fp)
  {
    perror("cannot open arp cache");
    return;
  }

  char header[ARP_BUFFER_LEN];
  if (!fgets(header, sizeof(header), fp))
  {
    fclose(fp);
    return;
  }
  // handle /proc/net/arp for ipv4 only  
  while (fscanf(fp, ARP_LINE_FORMAT, ipstr, hw_type, flag, mac, mask, device) == 6)
  {
    inet_pton(AF_INET, ipstr, &srcv4);

    ar = arp_node_new();

    ar->isv4 = true;  
    //memcpy(&ar->srcv4, &srcv4, INET_ADDRSTRLEN);
    memcpy(&ar->srcv4, &srcv4, sizeof(struct in_addr));
    to_upper(mac);
    memcpy(ar->mac, mac, ETHER_ADDR_LENGTH);
   
    ar_tmp = arp_list_find_node(ar, arlist);
    if (ar_tmp) {
      arp_node_free(ar);
      continue;
    }
    list_add_tail(&ar->list, arlist);
  }
  
  fclose(fp);
  
  //TODO: ipv6 from $ip -6 neigh show
  /*
     admin@RT-AX58U_V2-0000:/proc/4848/net# ip -6 neigh show
     fe80::22cf:30ff:fe00:0 dev br0 lladdr 20:cf:30:00:00:00 router STALE
     2001:b011:400b:f4fa:8120:3ec2:3e7d:5266 dev br0 lladdr 00:28:f8:e3:cf:38 REACHABLE
     fe80::9c79:e1fb:7603:ba18 dev br0 lladdr 00:28:f8:e3:cf:38 STALE
     admin@RT-AX58U_V2-0000:/proc/4848/net#
  */
  fp = popen(GET_IPV6_MAC_SYS_COMMAND, "r");
  if (!fp)
  {
    dnsdbg("cannot open ip -6 neigh show");
    return;
  }
 

  while (fgets(line, sizeof(line), fp) != NULL)
  {
    tokens = sscanf(line, IP6_LINE_FORMAT, ip6str, dev, br, lladdr, mac, status, status_ext);
    //dnsdbg("tokens=%d ip6str=%s dev=%s br=%s lladdr=%s mac=%s status:%s status_ext:%s", tokens, ip6str, dev, br, lladdr, mac, status, status_ext);
    
    if(tokens < 6) 
    {
       //clear
       memset(ip6str, 0, INET6_ADDRSTRLEN);
       memset(mac, 0, sizeof(mac));  
       memset(status, 0, sizeof(status));
       memset(status_ext, 0, sizeof(status_ext));
       continue;
    }
   // dnsdbg("tokens=%d ip6str=%s dev=%s br=%s lladdr=%s mac=%s status:%s", tokens, ip6str, dev, br, lladdr, mac, status);
    inet_pton(AF_INET6, ip6str, &srcv6);
     
    if(srcv6.s6_addr[0] == 0xfe && (srcv6.s6_addr[1] & 0x80) == 0x80) 
    {
      //dnsdbg("got local link address --skip ..."); 
      continue;
    }
    ar = arp_node_new();

    ar->isv4 = false;  
    memcpy(&ar->srcv6, &srcv6, sizeof(struct in6_addr));
    to_upper(mac);
    memcpy(ar->mac, mac, ETHER_ADDR_LENGTH);
    
 
    ar_tmp = arp_list_find_node(ar, arlist);
    if (ar_tmp) {
      arp_node_free(ar);
      continue;
    }
    list_add_tail(&ar->list, arlist);
  }
  
  pclose(fp);

  return;
}

void arp_list_free(struct list_head *list)
{
  arp_node_t *ar, *art;

  list_for_each_entry_safe(ar, art, list, list)
  {
    list_del(&ar->list);
    arp_node_free(ar);
  }

  return;
}

arp_node_t*  arp_list_search(bool isv4, char* ipstr, struct list_head *arlist)
{
  arp_node_t *arp;
  struct in_addr srcv4;
  struct in6_addr srcv6;
  
  if(isv4)
    inet_pton(AF_INET, ipstr, &srcv4);
  else
   {
     inet_pton(AF_INET6, ipstr, &srcv6);
#if 0
     dnsdbg("arp serach isv4:%d ipstr:%s v6:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x", isv4, ipstr,
       srcv6.s6_addr[0], srcv6.s6_addr[1], srcv6.s6_addr[2], srcv6.s6_addr[3],
       srcv6.s6_addr[4], srcv6.s6_addr[5], srcv6.s6_addr[6], srcv6.s6_addr[7],
       srcv6.s6_addr[8], srcv6.s6_addr[9], srcv6.s6_addr[10], srcv6.s6_addr[11],
       srcv6.s6_addr[12], srcv6.s6_addr[13], srcv6.s6_addr[14], srcv6.s6_addr[15]
      );
#endif
   }

  list_for_each_entry(arp, arlist, list)
  {
    if ((isv4 && arp->isv4 && (arp->srcv4.s_addr == srcv4.s_addr)) 
         || (!isv4 && !arp->isv4 &&!memcmp(&arp->srcv6, &srcv6, sizeof(struct in6_addr))))
      return arp;

  }
  return NULL;
}
