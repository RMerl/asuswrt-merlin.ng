#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <time.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <netinet/in.h>
#include <linux/types.h>
#include <linux/netfilter.h>
#include <libnetfilter_queue/libnetfilter_queue.h>
#include <libnetfilter_queue/libnetfilter_queue_ipv4.h>
#include <libnetfilter_queue/libnetfilter_queue_udp.h>
#include <libnetfilter_queue/pktbuff.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <ev.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>



#include "dns.h"
#include "dnssql.h"
#include "appjs.h"
#include "dnsarp.h"
#include "nfcfg.h"


#include "bw_mon_ct.h"
#include <bcmnvram.h>
#include <shared.h>

#ifdef RTCONFIG_MULTILAN_CFG
#include "mtlan_utils.h"
#endif

#define BUFSIZE 2048

int dq_db_vacuum_flag = 0;
int dq_app_client_aggregator_flag = 0;
int dq_app_sum_worker_flag = 0;
int dq_dev_identify_worker_flag = 0;
int dq_bw_mon_flag = 0;
int dq_env_update_flag = 0;



#define DNSQD_VACUUM_DB_TIMER (5*60)
#define DNSQD_APP_CLIENT_AGGREGATOR_TIMER (60)
#define DNSQD_APP_SUM_WORKER_TIMER (2*60)
#define DNSQD_DEV_IDENTIFY_WORKER_TIMER (3*60) // TODO: set prod env value
#define DNSQD_BW_MON_TIMER (1)
#define DNSQD_ENV_UPDATE_TIMER (10)
#define DNSQD_MAIN_LOOPER_TIMER (1)

#define DNSQD_APP_SUM_WORKER_HOURLY_TIMER (60*60)
#define DNSQD_DEV_WORKER_WORKER_HOURLY_TIMER (60*60)

#define APP_CLIENT_DB_SIZE 4 
#define DNS_QUERY_DB_SIZE 4 
#define APP_SUM_DB_SIZE 2 

#define APP_CLIENT_DB_DAY 0.1  // 2.4 hour , at least keep 1 hour since app_sum calculate 1 hour ago
#define DNS_QUERY_DB_DAY 2 
#define APP_SUM_DB_DAY 30

typedef enum
{
  PKT_ERR = -1,
  PKT_ACCEPT = 0,
  PKT_DROP = 1,
  PKT_BLOCK_REDIRECT =2
}pkt_decision_enum;

sqlite3 *dns_db = NULL;
sqlite3 *app_client_db = NULL;
sqlite3 *app_sum_db = NULL;

int dns_dpi_trf_db_enable = 0;

LIST_HEAD(arlist);
LIST_HEAD(clilist);
LIST_HEAD(clipoollist);
LIST_HEAD(iplist);
LIST_HEAD(bklist);
LIST_HEAD(smlist);


long gLastBWReq = 0;
void dump_packet(unsigned char *buff, int len)
{
        int i=0, j=0, maxlen;

        if (len <= 0 || !buff)
                return;

        // maxlen = (len > 64) ? 64 : len;
        maxlen = len;
        for (i=0;i<maxlen;i++) {
                printf("%02x ", buff[i]&0xFF);
                j++;
                if (j >= 16) {
                        printf("\n");
                        j=0;
                }
        }
        printf("\n");
}


/*
* Deconding/Encoding functions.
*/

// 3foo3bar3com0 => foo.bar.com (No full validation is done!)
char *decode_domain_name(const uint8_t **buf, size_t len)
{
  char domain[DNS_MAX_NAME_LEN];
  for (int i = 1; i < MIN(DNS_MAX_NAME_LEN, len); i += 1) {
    uint8_t c = (*buf)[i];
    if (c == 0) {
      domain[i - 1] = 0;
      *buf += i + 1;
      return strdup(domain);
    } else if (c <= 63) {
      domain[i - 1] = '.';
    } else {
      domain[i - 1] = c;
    }
  }

  return NULL;
}

u_char* ReadName(unsigned char* reader,unsigned char* buffer, int max_len, int* count)
{
    unsigned char *name;
    unsigned int p=0,jumped=0,offset;
    int i , j;
 
    *count = 1;
    name = (unsigned char*)malloc(256);
 
    name[0]='\0';
 
    //read the names in 3www6google3com format
    while(*reader!=0)
    {
        if(*reader>=192)
        {
            offset = (*reader)*256 + *(reader+1) - 49152; //49152 = 11000000 00000000 ;)
            if( offset > max_len)
            { 
               dnsdbg("Bad dns response packet - Compress Name offset over data buff range");
               free(name);
               return NULL;
            }
            reader = buffer + offset - 1;
            jumped = 1; //we have jumped to another location so counting wont go up!
        }
        else
        {
            name[p++]=*reader;
        }
 
        reader = reader+1;
 
        if(jumped==0)
        {
            *count = *count + 1; //if we havent jumped to another location then we can count up
        }
    }
 
    name[p]='\0'; //string complete
    if(jumped==1)
    {
        *count = *count + 1; //number of steps we actually moved forward in the packet
    }
 
    //now convert 3www6google3com0 to www.google.com
    for(i=0;i<(int)strlen((const char*)name);i++) 
    {
        p=name[i];
        for(j=0;j<(int)p;j++) 
        {
            name[i]=name[i+1];
            i=i+1;
        }
        name[i]='.';
    }
    name[i-1]='\0'; //remove the last dot
    return name;
}    

static int updateOffset(uint8_t isIPv4, const char *data_p)
{
	const struct tcphdr *tcp;
  	const struct udphdr *udp;
	int payload_offset;

#ifdef DNSQD_DEBUG
	dnsdbg("isIPv4<%d>", isIPv4);
#endif

	if (isIPv4)
	{
		const struct iphdr *iph;
		iph = (const struct iphdr *)data_p;
		udp = (const struct udphdr *)(data_p + (iph->ihl<<2));
		payload_offset = ((iph->ihl)<<2) + sizeof(struct udphdr);
#ifdef DNSQD_DEBUG
	dnsdbg("offset<%d>", payload_offset);
#endif
		return payload_offset;
  } else { 
      const struct ip6_hdr *ip6h;
      const struct ip6_ext *ip_ext_p;
      uint8_t nextHdr;
      int count = 8;

      ip6h = (const struct ip6_hdr *)data_p;
      nextHdr = ip6h->ip6_nxt;
      ip_ext_p = (const struct ip6_ext *)(ip6h + 1);
      payload_offset = sizeof(struct ip6_hdr);

      do
      {
        if ( nextHdr == IPPROTO_UDP )
        {
          udp = (struct udphdr *)ip_ext_p;
          payload_offset += sizeof(struct udphdr);
          return payload_offset;
        }
        if (nextHdr == IPPROTO_NONE)
        {
          dnsdbg("hit null nexthdr before dup\n");
          return -1;
        }
        payload_offset += (ip_ext_p->ip6e_len + 1) << 3;
        nextHdr = ip_ext_p->ip6e_nxt;
        ip_ext_p = (struct ip6_ext *)(data_p + payload_offset);
        count--; /* at most 8 extension headers */
      } while(count);
  }
	
  return -1;
}

static void dump_dns_header(dns_header *dns_hdr)
{
  dnsdbg("  DNS HDR id: [%X]\n", dns_hdr->id);
  dnsdbg("  DNS HDR flags/codes: [%X]\n", dns_hdr->fc);
  dnsdbg("  DNS HDR qd_count: [%d]\n", dns_hdr->q_count);
  dnsdbg("  DNS HDR an_count: [%d]\n", dns_hdr->ans_count);
  dnsdbg("  DNS HDR ns_count: [%d]\n", dns_hdr->ns_count);
  dnsdbg("  DNS HDR ar_count: [%d]\n", dns_hdr->addrec_count);
}


/*
* Basic memory operations.
*/

size_t get16bits(const uint8_t **buffer)
{
  uint16_t value;

  memcpy(&value, *buffer, 2);
  *buffer += 2;

  return ntohs(value);
}

size_t get32bits(const uint8_t **buffer)
{
  uint32_t value;
  memcpy(&value, *buffer, 4);
  *buffer += 4;
  
  return ntohl(value);
}

void put8bits(uint8_t **buffer, uint8_t value)
{
  memcpy(*buffer, &value, 1);
  *buffer += 1;
}

void put16bits(uint8_t **buffer, uint16_t value)
{
  value = htons(value);
  memcpy(*buffer, &value, 2);
  *buffer += 2;
}

void put32bits(uint8_t **buffer, uint32_t value)
{
  value = htonl(value);
  memcpy(*buffer, &value, 4);
  *buffer += 4;
}

void printDnsQuestion(dns_question dns_question)
{
  dnsdbg("Question: %s type: %d class: %d", dns_question.name,\
  dns_question.q_type, dns_question.q_class);
  dnsdbg("n_rec:%d", dns_question.ans.n_rec );
  for (int i = 0 ; i < dns_question.ans.n_rec ; i++) {
    if(dns_question.ans.isv4[i])
    	dnsdbg("IP V4: %s", dns_question.ans.recs[i]);
    else
    	dnsdbg("IP V6: %s", dns_question.ans.recs[i]);
  }
}

static int parse_dns_header(unsigned char *data, dns_msg *dns_msg, int max_len)
{
  int ret = 0, stop = 0;
  unsigned char *pQuestion, *pAnswer, *pAnsName;;
  char *qname = NULL;
  struct in_addr addr4;
  struct in6_addr addr6;
  
  if (!dns_msg || !data) {
    ret = DNS_DATA_GENERIC_ERR;
    return ret;
  }
  dns_header *pHdr = (dns_header *)data;
  dns_msg->header.id = ntohs(pHdr->id);
  dns_msg->header.fc = ntohs(pHdr->fc);
  dns_msg->header.q_count = ntohs(pHdr->q_count);
  dns_msg->header.ans_count = ntohs(pHdr->ans_count);
  dns_msg->header.ns_count = ntohs(pHdr->ns_count);
  dns_msg->header.addrec_count = ntohs(pHdr->addrec_count);

#ifdef DNSQD_DEBUG
  dnsdbg("dns_header: id=%x fc=%x q_c=%d a_c=%d ns_c=%d ar_c=%d", dns_msg->header.id,
      dns_msg->header.fc, dns_msg->header.q_count, dns_msg->header.ans_count, dns_msg->header.ns_count,
      dns_msg->header.addrec_count);
#endif
 
  if (!(dns_msg->header.fc & QR_MASK ) || (dns_msg->header.fc & OPCODE_MASK) != OPCODE_Query)
  {
    ret = DNS_DATA_RESPONSE_TYPE_ERR;
#ifdef DNSQD_DEBUG
    dnsdbg("parse_dns_header: %d", ret);
#endif
    return ret;
  } 

  if ( (dns_msg->header.fc & RCODE_MASK) != Ok_ResponseType || (dns_msg->header.fc & TC_MASK))
  {
    ret = DNS_DATA_REPLY_ERR;
#ifdef DNSQD_DEBUG
    dnsdbg("parse_dns_header: %d", ret);
#endif
    return ret;
  } 

  if (!dns_msg->header.ans_count) 
  {
    ret = DNS_DATA_NO_ANSWER_ERR;
#ifdef DNSQD_DEBUG
    dnsdbg("parse_dns_header: %d", ret);
#endif
    return ret;
  }

  pQuestion = data + sizeof(dns_header);
  
  // question
  for (int i = 0; i< dns_msg->header.q_count; i++)
  {
    stop = 0;
    qname =  ReadName(pQuestion, data, max_len, &stop);
    if (!qname) 
    {
      ret = DNS_DATA_NO_QUESTION_ERR;
      return ret;
    }
    strcpy(dns_msg->question.name, qname);
    pQuestion = pQuestion + stop;
    dns_msg->question.q_type = get16bits(&pQuestion);
    dns_msg->question.q_class = get16bits(&pQuestion);
    free(qname);
    qname = NULL;
  }
  
  // answer
  stop = 0;
  dns_msg->question.ans.n_rec = 0;
  for (int i = 0; i < dns_msg->header.ans_count && i< DNS_MAX_ANS_RR_NUM; i++)
  {
    pAnsName  = ReadName(pQuestion, data, max_len, &stop);
    pQuestion = pQuestion + stop;
    unsigned short ans_type = get16bits(&pQuestion);
    unsigned short ans_class = get16bits(&pQuestion);
    unsigned int ans_ttl = get32bits(&pQuestion);
    unsigned short ans_len = get16bits(&pQuestion);
    
    if ( ans_type == DNS_RR_TYPE_A ) {
    
      dns_msg->question.ans.isv4[dns_msg->question.ans.n_rec] = true;
      memcpy(&addr4, pQuestion, ans_len);
      if(inet_ntop(AF_INET, &addr4, dns_msg->question.ans.recs[dns_msg->question.ans.n_rec], INET_ADDRSTRLEN) >0)
      {
        dns_msg->question.ans.n_rec++;
      }
    
    } else if ( ans_type == DNS_RR_TYPE_AAAA ) {      
    
      dns_msg->question.ans.isv4[dns_msg->question.ans.n_rec] = false;
      memcpy(&addr6, pQuestion, ans_len);
      if(inet_ntop(AF_INET6, &addr6, dns_msg->question.ans.recs[dns_msg->question.ans.n_rec], INET6_ADDRSTRLEN)>0)
      {
        dns_msg->question.ans.n_rec++;
      }
    }
 
    pQuestion = pQuestion + ans_len;
    free(pAnsName);
    pAnsName = NULL;
  
  } 
  
  if(dns_msg->question.ans.n_rec == 0) {
   /* ipv6 client , ipv6 dns server, win10 nslookup use inverse query w/o iq opcode 
      and then get bad answer type/content 
   */
      ret = DNS_DATA_BAD_IQUERY_ERR;
  }
  return ret;
}

static inline
char* ip_str(unsigned char *ip, int ip_ver)
{
#ifndef INET6_ADDRSTRLEN
#define INET6_ADDRSTRLEN        48
#endif

  static int iter = 0;
  static unsigned char ipstr_buf[4][INET6_ADDRSTRLEN];

  unsigned char *ipstr = ipstr_buf[iter++ & 3];
  *ipstr = 0;

  if (ip_ver == 4) {
    snprintf(ipstr, INET6_ADDRSTRLEN,
                  "%d.%d.%d.%d",
                  ip[0], ip[1], ip[2], ip[3]);
  } else if (ip_ver == 6) {
    /* No support for address abbrevation */
    snprintf(ipstr, INET6_ADDRSTRLEN,
                  "%.2x:%.2x:%.2x:%.2x:%.2x:%.2x:%.2x:%.2x:"
                  "%.2x:%.2x:%.2x:%.2x:%.2x:%.2x:%.2x:%.2x",
                  ip[0], ip[1], ip[2], ip[3],
                  ip[4], ip[5], ip[6], ip[7],
                  ip[8], ip[9], ip[10], ip[11],
                  ip[12], ip[13], ip[14], ip[15]);
  }
  return ipstr;
}

void encode_header(dns_msg *msg, uint8_t **buffer)
{
  /*
                                  1  1  1  1  1  1
    0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5
  +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
  |                      ID                       |
  +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
  |QR|   Opcode  |AA|TC|RD|RA|   Z    |   RCODE   |
  +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
  |                    QDCOUNT                    |
  +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
  |                    ANCOUNT                    |
  +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
  |                    NSCOUNT                    |
  +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
  |                    ARCOUNT                    |
  +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
  */
  put16bits(buffer, msg->header.id);
  put16bits(buffer, msg->header.fc);
  put16bits(buffer, 1);
  put16bits(buffer, 1);
  put16bits(buffer, 0);
  put16bits(buffer, 0);
}

void encode_domain_name(const char *domain, uint8_t **buffer)
{
  uint8_t *buf = *buffer;
  const char *beg = domain;
  const char *pos;
  int len = 0;
  int i = 0;

  while ((pos = strchr(beg, '.'))) {
    len = pos - beg;
    buf[i] = len;
    i += 1;
    memcpy(buf+i, beg, len);
    i += len;

    beg = pos + 1;
  }

  len = strlen(domain) - (beg - domain);
  buf[i] = len;
  i += 1;

  memcpy(buf + i, beg, len);
  i += len;
  buf[i] = 0;
  i += 1;
  *buffer += i;
}

void encode_question(dns_msg *msg, uint8_t **buffer)
{
  /*
                                  1  1  1  1  1  1
    0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5
  +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
  |                                               |
  /                     QNAME                     /
  /                                               /
  +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
  |                     QTYPE                     |
  +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
  |                     QCLASS                    |
  +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
  */
  encode_domain_name(msg->question.name, buffer);
  put16bits(buffer, msg->question.q_type);
  put16bits(buffer, msg->question.q_class);
}

void encode_answer(dns_msg *msg, uint8_t **buffer)
{
  /*
                                  1  1  1  1  1  1
    0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5
  +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
  |                                               |
  /                                               /
  /                      NAME                     /
  |                                               |
  +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
  |                      TYPE                     |
  +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
  |                     CLASS                     |
  +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
  |                      TTL                      |
  |                                               |
  +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
  |                   RDLENGTH                    |
  +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--|
  /                     RDATA                     /
  /                                               /
  +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
  */

  // have to use optimization name, otherwise, mangle packet can't resolved by client 
  //encode_domain_name(msg->question.name, buffer);
  put16bits(buffer, 0xc00c);
 
  put16bits(buffer, msg->question.q_type);
  put16bits(buffer, msg->question.q_class);
  put32bits(buffer, 600);
  put16bits(buffer, 4);
  /* 0.0.0.0 as dns sinkhole */
  put8bits(buffer, 0);
  put8bits(buffer, 0);
  put8bits(buffer, 0);
  put8bits(buffer, 0);
}

void encode_sinkhole_payload(dns_msg *msg, uint8_t *payload, int *payload_len)
{
  // prepare and encode a single question and single answer header
  uint8_t *p = payload;
  encode_header(msg, &p);
  encode_question(msg, &p);
  encode_answer(msg, &p);

  *payload_len = p - payload;
}

static int block_response(struct nfq_q_handle *qh, u_int32_t id, struct nfq_data * payload, dns_msg *msg)
{
  char *data;
	int payload_offset, data_len, dns_len;
	struct pkt_buff *pkt_buf = NULL;
  uint8_t buffer[BUF_SIZE];
  int replace_len = 0;
  int rc;
	data_len = nfq_get_payload(payload, &data);
  if ( data_len == -1 )
  {
  #ifdef DNSQD_DEBUG
    dnsdbg("data_len == -1!!!!!!!!!!!!!!!, EXIT\n");
  #endif
     return nfq_set_verdict(qh, id, NF_ACCEPT, 0, NULL);
  }
  pkt_buf = pktb_alloc(AF_INET, data, data_len, 0x1000);
  
  struct iphdr *ip = nfq_ip_get_hdr(pkt_buf);
  
  nfq_ip_set_transport_header(pkt_buf, ip);
  
  if (ip->protocol == IPPROTO_UDP) 
  { 
 
    struct udphdr *udp = nfq_udp_get_hdr(pkt_buf);
    unsigned char *payload = nfq_udp_get_payload(udp, pkt_buf);
    unsigned int payload_len = nfq_udp_get_payload_len(udp, pkt_buf);  

    // mangle udp packet
    encode_sinkhole_payload(msg, buffer, &replace_len);
  
    nfq_udp_mangle_ipv4(pkt_buf, 0, payload_len, buffer, replace_len);
    // recalculate checksum
    //nfq_udp_compute_checksum_ipv4(udp, ip);
  
    rc = nfq_set_verdict(qh, id, NF_ACCEPT, data_len, pktb_data(pkt_buf));
    pktb_free(pkt_buf);
    return rc;  
  }

  return nfq_set_verdict(qh, id, NF_ACCEPT, 0, NULL);
}

void save_dns_packet(char *name, char *name2, unsigned char *data, int len, unsigned char *in, int in_len)
{
  FILE *f;
  int count = 0;
  int nfd;

 
  sprintf(name, "/tmp/%ld-%d.dat", time(NULL), count);
  sprintf(name2, "/tmp/%ld-%d_orig.dat", time(NULL), count);
 
  while(isFileExist(name))
  {
     count++;
     sprintf(name, "/tmp/%ld-%d.dat", time(NULL), count);
     sprintf(name2, "/tmp/%ld-%d_orig.dat", time(NULL), count);
 
  }

  nfd = open(name, O_WRONLY | O_CREAT);  
  if(nfd >= 0)
        {
                if((f = fdopen(nfd, "w")) != NULL)
                {
                     fwrite(data, 1, len, f);   
                     fclose(f);
                }
        }

   nfd = open(name2, O_WRONLY | O_CREAT);  
  if(nfd >= 0)
        {
                if((f = fdopen(nfd, "w")) != NULL)
                {
                     fwrite(in, 1, in_len, f);   
                     fclose(f);
                }
        } 

}

static int process_dns_response(struct nfq_data * payload, dns_msg *dns_msg)
{
	char *data;
	char *dns = NULL;

  int payload_offset, data_len, dns_len, ret = 0;
	struct iphdr *iph;
  struct ip6_hdr *ip6h;
	uint8_t isIPv4;
  char dns_cap_file[50];
  char dns_cap_file2[50];
      
	data_len = nfq_get_payload(payload, &data);
	if ( data_len == -1 )
	{
	  dnsdbg("error!!!!! got error data_len=%d kill and wait for restart", data_len);
	  exit(1);
	}

	iph = (struct iphdr *)data;
	isIPv4 = (iph->version == 4)?1:0;
	payload_offset = updateOffset(isIPv4, data);

	if (payload_offset < 0)
	{
		/* return PKT_ERR then system take care this pkt as NF_ACCEPT */
		return PKT_ERR;
	}

  if (isIPv4)
  {
      dns_msg->isv4 = 1;
      dns_msg->src_ip = iph->saddr;
      dns_msg->dst_ip = iph->daddr;
      inet_ntop(AF_INET, &iph->saddr, dns_msg->sip, INET_ADDRSTRLEN);
      inet_ntop(AF_INET, &iph->daddr, dns_msg->dip, INET_ADDRSTRLEN);
  }
  else
  {
      dns_msg->isv4 = 0;
      ip6h = (struct ip6_hdr *)data;
      memcpy(&dns_msg->srcv6, &ip6h->ip6_src, sizeof(struct in6_addr));
      memcpy(&dns_msg->dstv6, &ip6h->ip6_dst, sizeof(struct in6_addr));
      inet_ntop(AF_INET6, &dns_msg->srcv6, dns_msg->sip, INET6_ADDRSTRLEN);
      inet_ntop(AF_INET6, &dns_msg->dstv6, dns_msg->dip, INET6_ADDRSTRLEN);
  }

	dns = (char *)(data + payload_offset);
  dns_len = data_len - payload_offset;
#ifdef SAVE_DNS_PACKET
  save_dns_packet(dns_cap_file, dns_cap_file2, (unsigned char*) dns, dns_len, data, data_len);
#endif

  ret = parse_dns_header(dns, dns_msg, dns_len);
#ifdef SAVE_DNS_PACKET
  //unlink(dns_cap_file);
#endif 	
  if (ret < 0)
  {
    return PKT_ERR;
  }

#ifdef DNSQD_DEBUG   
	  dump_dns_header(&dns_msg->header);
#endif

#if ENABLE_DNS_BLOCK 
	if(checkInBlockList(dns_msg->question.name, dns_msg->src_ip, time(NULL)) == DNS_BLOCK_BLACK_LIST_FOUND)
  {
    dnsdbg("got block server %s", dns_msg->question.name);
    return PKT_BLOCK_REDIRECT;
  }
#endif 

  return PKT_ACCEPT;
}

int checkInBlockList(char *host, uint32_t client_ip, int timestamp)
{ 
  int type = -1;
  int res = sqlite_block_list_has_name(app_client_db, host, &type);
  if (res == DNS_BLOCK_BLACK_LIST_FOUND)
  {
    sqlite_block_history_insert(app_client_db, client_ip, host, timestamp);
  } 
  return res;
}


static int cb(struct nfq_q_handle *qh, struct nfgenmsg *nfmsg,
        struct nfq_data *nfa, void *data)
{
    int lock;
    u_int32_t id = 0;
    struct nfqnl_msg_packet_hdr *ph;
    int ret = 0, app_id, cat_id;
    char app_name[128] = {0};
    char mac[ETHER_ADDR_LENGTH];
    dns_msg msg = {};
#ifdef DNSQD_DEBUG
    dnsdbg("cb recevied");
#endif
    ph = nfq_get_msg_packet_hdr(nfa);
    if (ph) {
        id = ntohl(ph->packet_id);
    }
#if 0 //dig out memory leak

      return nfq_set_verdict(qh, id, NF_ACCEPT, 0, NULL);
#endif
    /* process DNS query response packet */
    ret = process_dns_response(nfa, &msg);
    if (ret == PKT_BLOCK_REDIRECT) 
    {
#if ENABLE_DNS_BLOCK    
      return block_response(qh, id, nfa, &msg);
#else
      return nfq_set_verdict(qh, id, NF_ACCEPT, 0, NULL);
#endif      
    } else {
      if (ret == PKT_ACCEPT)
      {
//      lock = file_lock(DNS_MAC_LIST_LOCK_NAME);
        int rc = 0;
/*        
        arp_node_t *arp = arp_list_search(msg.isv4, msg.dip, &arlist);
        if (!arp) {
          memcpy(mac, DEFAULT_MAC, ETHER_ADDR_LENGTH);
  #ifdef DNSQD_DEBUG
            dnsdbg("can't find arp since msg is v4:%d", msg.isv4);
  #endif
        } else {
          memcpy(mac, arp->mac, ETHER_ADDR_LENGTH);
        }

        file_unlock(lock);
*/  
        memcpy(mac, DEFAULT_MAC, ETHER_ADDR_LENGTH);
        if(arp_list_search_and_update_mac(msg.isv4, msg.dip, mac)<0)
        {
            if(!cli_list_find_mac_by_ip(&clipoollist, msg.isv4, msg.dip, mac))
            {
              cli_list_free(&clipoollist);
              cli_list_file_parse(&clipoollist);
              // try again with updated info
              cli_list_find_mac_by_ip(&clipoollist, msg.isv4, msg.dip, mac);
            }
        }
#ifdef DNSQD_DEBUG   
        // map app_id and app_name
        dnsdbg("msg question name: %s\n", msg.question.name);
        printDnsQuestion(msg.question);
#endif	
        sqlite_dns_lookup(dns_db, msg.question.name, &app_id, &cat_id, app_name);
        // insert query log
	      lock = file_lock(DNS_QUERY_DB_LOCK_NAME);
        sqlite_dns_insert(dns_db, &msg, mac, app_id);
	      file_unlock(lock);
      }

      return nfq_set_verdict(qh, id, NF_ACCEPT, 0, NULL);
    }
}

// fill all nfapp_node data which query from nfcm initially
void map_resolve(struct list_head *list)
{
  nfapp_node_t *ap;
  list_for_each_entry(ap, list, list) {
     sqlite_dns_map_ip(dns_db, ap);    
  }
}


void do_app_stats_json(sqlite3 *app_client, int start, int end)
{
  int ret;
  LIST_HEAD(appstatslist); // app_client list 
  ret = sqlite_app_client_statistics(app_client, start, end, &appstatslist);
  if (ret == DNSSQL_OK) 
  {
    appstats_list_dump(&appstatslist);
  }
  appstats_list_to_json(&appstatslist);
  appstats_list_free(&appstatslist);
}

void aggregator_worker()
{
#ifdef DNSQD_DEBUG
  dnsdbg("++++aggregator_worker ......");
#endif
  int ret;
  int old_status, old_time, new_time, new_status;
  int lock;
  char sql[256] = {0};
   
  sqlite3 *nfcm_db = NULL;
  LIST_HEAD(nfaplist); // app_client list 
  
  ret = sqlite_task_status_get_last(app_client_db, APP_CLIENT_TABLE, &old_status, &old_time);
  if (ret == DNSSQL_ERROR)
  {
    old_time = 0;
  }
  new_time = time(NULL);
  new_status = 0;
  
  sprintf(sql, "select data_id, timestamp, is_v4, src_ip, src6_ip, src_port, dst_ip, dst6_ip, dst_port, up_bytes, up_dif_bytes, dn_bytes, dn_dif_bytes, src_mac from DATA_INFO where timestamp between %ld and %ld and (up_dif_bytes<>0 or dn_dif_bytes<>0);",
  old_time, new_time);

  lock = file_lock(NFCM_APP_DB_LOCK_NAME);
  nfcm_db = sqlite_open(NFCM_APP, nfcm_db, NFCM_APP_DB_FILE);
  
  sqlite_nfcm_select(nfcm_db, sql, &nfaplist);
  sqlite_close(nfcm_db);
  file_unlock(lock);
  
  map_resolve(&nfaplist);
  
  lock = file_lock(APP_CLIENT_DB_LOCK_NAME);
  sqlite_app_client_insert(app_client_db, &nfaplist);
  file_unlock(lock);
  sqlite_task_status_insert_or_update(app_client_db, APP_CLIENT_TABLE, new_time, new_status);
  
  nfapp_list_free(&nfaplist);
#ifdef DNSQD_DEBUG  
  dnsdbg("---aggregator_worker ......");
#endif
  //sqlite_close(nfcm_db);
}

void app_sum_worker()
{

  int ret;
  int lock;
  int old_status, old_time, new_time, new_status, delta_time, start_time, end_time;
  new_time = time(NULL);
  delta_time = new_time % DNSQD_APP_SUM_WORKER_HOURLY_TIMER;
  new_time -= delta_time;
  
  LIST_HEAD(sumlist); // app_client list 
 
  ret = sqlite_task_status_get_last(app_client_db, APP_SUM_TABLE, &old_status, &old_time);
  if (ret == DNSSQL_ERROR)
  {
    old_time = new_time - (APP_SUM_DB_DAY * DAY_SEC);
  }
  
  start_time = old_time;
  end_time = start_time + DNSQD_APP_SUM_WORKER_HOURLY_TIMER;
  
  int count;
 
  while (start_time < new_time)
  {
    ret = sqlite_app_sum_aggregate(app_client_db, start_time, end_time, &sumlist);
    if ( ret != DNSSQL_OK)
      break;
    start_time += DNSQD_APP_SUM_WORKER_HOURLY_TIMER;
    end_time += DNSQD_APP_SUM_WORKER_HOURLY_TIMER;
  }
 
  new_status = 0;
  lock = file_lock(APP_SUM_DB_LOCK_NAME);
  sqlite_app_sum_insert(app_sum_db, &sumlist);
  file_unlock(lock);
  sqlite_task_status_insert_or_update(app_client_db, APP_SUM_TABLE, start_time, new_status);
  nfapp_list_free(&sumlist);
}

void dev_exclusive_check()
{
  int ret;
  int old_status, old_time, new_time, new_status, delta_time, start_time, end_time;
  new_time = time(NULL);
  delta_time = new_time % DNSQD_DEV_WORKER_WORKER_HOURLY_TIMER;
  new_time -= delta_time;
  
  LIST_HEAD(dev_list); // nfdev_node_t list 
  
  ret = sqlite_task_status_get_last(app_client_db, EXCLUSIVE_HISTORY_TABLE, &old_status, &old_time);
  if (ret == DNSSQL_ERROR)
  {
    old_time = new_time - (APP_SUM_DB_DAY * DAY_SEC);
  }
  
  start_time = old_time;
  end_time = start_time + DNSQD_DEV_WORKER_WORKER_HOURLY_TIMER;
  

  while (start_time < new_time)
  {
    ret = sqlite_app_exclusive_statistics(app_client_db, start_time, end_time, &dev_list);
    if ( ret != DNSSQL_OK)
      break;
      
    start_time += DNSQD_DEV_WORKER_WORKER_HOURLY_TIMER;
    end_time += DNSQD_DEV_WORKER_WORKER_HOURLY_TIMER;
    
  }
   
  new_status = 0;

#if 0  
  nfdev_list_dump(&dev_list);
#endif   

  sqlite_app_exclusive_history_insert(app_sum_db, &dev_list);
  sqlite_task_status_insert_or_update(app_client_db, EXCLUSIVE_HISTORY_TABLE, start_time, new_status);
  nfdev_list_free(&dev_list);

}

void update_device_type_file()
{
  LIST_HEAD(devices);
  sqlite_dev_type_query(app_sum_db, &devices);
#if 0
  nfdev_list_dump(&devices);
#endif
  device_query_list_to_json(&devices);
  nfdev_list_free(&devices);
}

void dev_identify_worker()
{
#ifdef DNSQD_DEBUG  
  dnsdbg("dev_identify_worker.....\n");
#endif
  dev_exclusive_check();
  update_device_type_file();
}

int g_bw_mon_inited = 0;
int g_disable_bw_mon = 1;


LIST_HEAD(lanlist);
lan_info_t* lan_info_new()
{
    lan_info_t *li = (lan_info_t *)calloc(1, sizeof(lan_info_t));
    if (!li) return NULL;

    INIT_LIST_HEAD(&li->list);

    return li;

}

void lan_info_free(lan_info_t *li)
{
    if (li) free(li);

    return;
}

void lan_info_list_dump(struct list_head *list)
{
    lan_info_t *li;
    char ipstr[INET6_ADDRSTRLEN];
    //inet_ntop(AF_INET, src, ipstr, INET_ADDRSTRLEN);

    printf("%s:\n", __FUNCTION__);
    list_for_each_entry(li, list, list) {
        inet_ntop(AF_INET, &li->addr, ipstr, INET_ADDRSTRLEN);
        printf("addr=\t%s\n", ipstr);
        inet_ntop(AF_INET, &li->subnet, ipstr, INET_ADDRSTRLEN);
        printf("subnet=\t%s\n", ipstr);
    }
}

#ifdef RTCONFIG_MULTILAN_CFG
int lan_info_add_sdn(struct list_head *list)
{
    lan_info_t *li;
    MTLAN_T *pmtl = (MTLAN_T *)INIT_MTLAN(sizeof(MTLAN_T));
    size_t mtl_sz = 0;
    int i;
    if (pmtl) {
      get_mtlan(pmtl, &mtl_sz);
      for (i = 0; i < mtl_sz; i++) {
        if (pmtl[i].enable) {

          li = lan_info_new();
          list_add_tail(&li->list, list);

          strcpy(li->ifname, pmtl[i].nw_t.ifname);
          inet_pton(AF_INET, pmtl[i].nw_t.addr, &li->addr);
          inet_pton(AF_INET, pmtl[i].nw_t.netmask, &li->subnet);
          li->enabled = true;
        }
      }
      FREE_MTLAN((void *)pmtl);
    }

#if defined(NFCMDBG)
    lan_info_list_dump(list);
#endif

    return 0;
}
#endif

int lan_info_init(struct list_head *list)
{
    lan_info_t *li;
    char word[64], *next;
    char *b, *nv, *nvp;
    char *ifname, *laninfo;
    char *addr, *mask;
    char *rulelist;

    li = lan_info_new();
    list_add_tail(&li->list, list);

    strcpy(li->ifname, "br0");
    li->enabled = true;
    inet_pton(AF_INET, nvram_get("lan_ipaddr"), &li->addr);
    inet_pton(AF_INET, nvram_get("lan_netmask"), &li->subnet);

    // get wireless guest network status
    if (!nvram_get_int("wgn_enabled"))
        goto out_f;

    rulelist = nvram_get("wgn_brif_rulelist");
    if (!rulelist)
        goto out_f;

    nv = nvp = strdup(rulelist);
    if (!nv)
        goto out_free;

    while ((b = strsep(&nvp, "<")) != NULL) {
        if (!b || !strlen(b)) continue;
        //fields = vstrsep(b, ">", &ifname, &laninfo);
        vstrsep(b, ">", &ifname, &laninfo);

        addr = strtok(laninfo, "/");
        mask = strtok(NULL, "/");

        li = lan_info_new();
        list_add_tail(&li->list, list);

        strcpy(li->ifname, ifname);
        inet_pton(AF_INET, addr, &li->addr); // lan_ipaddr
        li->subnet.s_addr = htonl(0xFFFFFFFF << (32 - atoi(mask)));
    }

    foreach(word, nvram_get("wgn_ifnames"), next) {
        list_for_each_entry(li, list, list) {
            if (!strcmp(word, li->ifname)) {
                li->enabled = true;
                break;
            }
        }
    }

out_free:
    free(nv);

out_f:

#if defined(NFCMDBG)
    lan_info_list_dump(list);
#endif
    return 0;
}


void lan_list_free(struct list_head *list)
{
	lan_info_t *lan, *tmp;

	list_for_each_entry_safe(lan, tmp, list, list) {
		list_del(&lan->list);
		lan_info_free(lan);
	}

	return;
}

void lan_list_parse(struct list_head *list)
{
  lan_info_init(list);
#ifdef RTCONFIG_MULTILAN_CFG
  lan_info_add_sdn(list);
#endif
}



bool is_in_lanv4(struct in_addr *src)
{
    lan_info_t *li;
    list_for_each_entry(li, &lanlist, list) {
        if ((src->s_addr & li->subnet.s_addr) == (li->addr.s_addr & li->subnet.s_addr)) {
            return true;
        }
    }

    return false;
}

bool is_in_lanv6(struct in6_addr *src)
{

    struct in6_addr prefix;
    int prefix_len = nvram_get_int("ipv6_prefix_length"); 
    int tail_bits;
    //passthrough don't have ipv6_prefix_length, ipv6_prefix, ipv6_rtr_addr, don't care lan
    if(!strcmp("ipv6pt", nvram_get("ipv6_service"))) {
       return true;
    }

    if(prefix_len < 1 || prefix_len > 127) {
       return false;
    } 
    if(!inet_pton(AF_INET6, nvram_get("ipv6_prefix"), &prefix)) {
   
      return false;
    }
    for(int i = 0; i< prefix_len/8; i++)
    {    
        if (src->s6_addr[i] != prefix.s6_addr[i]) {
            return false;
        }
    }
    tail_bits = prefix_len % 8;
    if ( tail_bits > 0 &&  src->s6_addr[prefix_len] >> (8 - tail_bits) != prefix.s6_addr[prefix_len] >> (8 - tail_bits) ) {
            return false;
    }
    return true;
}

bool is_local_link_addr(struct in6_addr *addr)
{

    if (addr->s6_addr[0] == 0xfe && (addr->s6_addr[1] & 0x80) == 0x80) 
            return true;
    return false;
}

bool is_router_v6addr(struct in6_addr *addr)
{
    struct in6_addr router;
    //nf_printf("router addr: %s\n", nvram_get("ipv6_rtr_addr"));
    if(!strcmp("ipv6pt", nvram_get("ipv6_service")))
            return false;
    if(!inet_pton(AF_INET6, nvram_get("ipv6_rtr_addr"), &router)) 
            return false;
    if (!memcmp(addr, &router, sizeof(struct in6_addr)))
            return true;

    return false;
}

bool is_router_addr(struct in_addr *addr)
{
    lan_info_t *li;

    //char ipstr[INET6_ADDRSTRLEN];
    //inet_ntop(AF_INET, src, ipstr, INET_ADDRSTRLEN);

    list_for_each_entry(li, &lanlist, list) {
        if (addr->s_addr == li->addr.s_addr) {
            return true;
        }
    }

    return false;
}


bool is_broadcast_addr(struct in_addr *addr)
{
    lan_info_t *li;

    //char ipstr[INET6_ADDRSTRLEN];
    //inet_ntop(AF_INET, src, ipstr, INET_ADDRSTRLEN);

    list_for_each_entry(li, &lanlist, list) {
        if (addr->s_addr == li->subnet.s_addr) {
            return true;
        }
    }

    return false;
}

bool is_multi_addr(struct in_addr *addr)
{
    //in_addr_t stored in network order
    uint32_t address = ntohl(addr->s_addr);

    return (address & 0xF0000000) == 0xE0000000;
}


int bw_mon_ct_cb(enum nf_conntrack_msg_type type,
                        struct nf_conntrack *ct,
                        void *data)
{
 // int lock;
 // lock = file_lock(DNS_MAC_LIST_LOCK_NAME);
  bw_nf_conntrack_process(ct, &iplist, &clilist, &arlist);
 // file_unlock(lock);
  return NFCT_CB_CONTINUE;
}

void bw_mon_worker()
{
 // printf("bw_mon_worker\n");
  if(time(NULL) - gLastBWReq > 30) {
    //printf("bw_mon_worker shutdown since now %ld old than %d \n", time(NULL), gLastBWReq);
    g_disable_bw_mon = 1;
  }

  if(!g_bw_mon_inited && !g_disable_bw_mon)
  {
    //printf("bw_mon_worker active  %d \n");
    g_bw_mon_inited = init_bw_mon_ct();
  }
  if(g_bw_mon_inited) {    
    nf_list_move(&bklist, &iplist);

    //printf("bw_mon_worker  init=%d disable=%d t=%lu\n", g_bw_mon_inited, g_disable_bw_mon, time(NULL));
    bw_mon_ct_invoke();

    bw_nf_list_diff_calc(&iplist, &bklist);

    bw_nf_list_statistics_calc(&iplist, &smlist);

#if 0
    nf_list_dump("bw_mon", &iplist);
#endif    
    //bw_ct_list_to_json(&iplist);
    bw_ct_list_to_json(&smlist);
    //nf_list_free(&iplist);
    nf_list_free(&bklist);
   
  }
  
  if(g_disable_bw_mon)
  {
   // printf("bw_mon_worker shutdown  %d \n");
 
    if(g_bw_mon_inited) {
      deinit_bw_mon_ct();
      g_bw_mon_inited = 0;
      nf_list_free(&iplist);
      nf_list_free(&smlist);
 
    }
  }
}

void env_update_worker()
{
  lan_list_free(&lanlist);
  lan_list_parse(&lanlist);
  arp_list_free(&arlist);
  arp_list_parse(&arlist);
  cli_list_free(&clilist);
  cli_list_file_parse(&clilist);

#if 0
  lan_info_list_dump(&lanlist);
  arp_list_dump(&arlist);
  cli_list_dump("clilist", &clilist);
#endif

}

void app_client_aggregator()
{
  aggregator_worker();
}


/*
 *  check filesize is over or not, size is Mbytes
 *  if over size, return 1, else return 0
 */
int check_file_size_over(char *fname, long int size)
{
    struct stat st;
    long max_size = size * ONE_M;
    int is_oversize = false;

    stat(fname, &st);

    is_oversize = (st.st_size >= max_size) ? 1 : 0;
    //printf("================================================\n"
    //   "[%s]%s: leave.. file[%s] file_size=[%ld], size=[%d], is_oversize=[%d]\n",
    //      __FILE__, __FUNCTION__, fname, st.st_size, max_size, is_oversize);

    return is_oversize;
}


void db_vacuum(sqlite3 *db, char *lock_name, char *db_file, char *table_name,  int max_db_size, double max_days)
{
    time_t now;
    time(&now);

    int ret;
    char *err = NULL;
    char sql[1024];
    time_t timestamp;
    int compact;
    int lock; // file lock

    compact = check_file_size_over(db_file, max_db_size);
    if (compact) {
        // step1. get timestamp
        timestamp = time(NULL) - max_days * DAY_SEC;
    } else
        return;
    lock = file_lock(lock_name);
    /* if *-journal exists, remove it !! */
    sqlite_remove_journal(db_file);

    /* integrity check */
    if (sqlite_integrity_check(db, db_file) == 0) {
        file_unlock(lock);
        return;
    }

//    while (compact) 
    {
        // step2. execute sql to delete nfcm_sum
        snprintf(sql, sizeof(sql), "DELETE from %s WHERE timestamp < %ld;", table_name, timestamp);
        //info("sql=[%s]\n", sql);
        ret = sqlite3_exec(db, sql,  NULL, NULL, &err);
        if (ret != SQLITE_OK && err != NULL) {
            printf("SQL error: %s\n", err);
            sqlite3_free(err);
            file_unlock(lock);
            return;
        }

        // step3. compact file
        //info("sql=[%s]\n", "VACUUM;");
        ret = sqlite3_exec(db, "VACUUM;",  NULL, NULL, &err);
        if (ret != SQLITE_OK && err != NULL) {
            printf("SQL error: %s\n", err);
            sqlite3_free(err);
            file_unlock(lock);
            return;
        }
//        compact = check_file_size_over(app_client_db_file, app_client_db_size);
//        timestamp += DAY_SEC * 1;
    }
    file_unlock(lock);
    return;
}

void func_db_vacuum()
{
  time_t now;
  time(&now);
  dq_db_vacuum_flag = 1;
}

void func_app_client_aggregator()
{
  time_t now;
  time(&now);
  dq_app_client_aggregator_flag = 1;

}

void func_app_sum_worker()
{
  time_t now;
  time(&now);
  dq_app_sum_worker_flag = 1;
}

void func_dev_identify_worker()
{
  time_t now;
  time(&now);
  dq_dev_identify_worker_flag = 1;
}

void func_bw_mon_worker()
{
  time_t now;
  time(&now);
  dq_bw_mon_flag = 1;
}

void func_env_update_worker()
{
  time_t now;
  time(&now);
  dq_env_update_flag = 1;
}

void func_main_looper()
{
  time_t now;
  time(&now);
  int lock;
  
#if 1

  if(dq_bw_mon_flag)
  {
    bw_mon_worker();
    dq_bw_mon_flag = 0;
  }

  if(dq_env_update_flag)
  {
    env_update_worker();
    dq_env_update_flag = 0;
  }


  if(dq_app_client_aggregator_flag)
  {
    app_client_aggregator();
    dq_app_client_aggregator_flag = 0;
  }

  if(dq_app_sum_worker_flag)
  {
    app_sum_worker();
    dq_app_sum_worker_flag = 0;
  }

  if(dq_dev_identify_worker_flag)
  {
    dev_identify_worker();
    dq_dev_identify_worker_flag = 0;
  }

  
  if(dq_db_vacuum_flag)
  {
    //dns_query
    db_vacuum(dns_db, DNS_QUERY_DB_LOCK_NAME, DNS_QUERY_DB_FILE, DNS_QUERY_TABLE,  DNS_QUERY_DB_SIZE, DNS_QUERY_DB_DAY);
    //app_client
    db_vacuum(app_client_db, APP_CLIENT_DB_LOCK_NAME, APP_CLIENT_DB_FILE, APP_CLIENT_TABLE,
		    APP_CLIENT_DB_SIZE, APP_CLIENT_DB_DAY);
    //app_sum
    db_vacuum(app_sum_db, APP_SUM_DB_LOCK_NAME, APP_SUM_DB_FILE, APP_SUM_TABLE, APP_SUM_DB_SIZE, APP_SUM_DB_DAY);
    db_vacuum(app_sum_db, APP_SUM_DB_LOCK_NAME, APP_SUM_DB_FILE, EXCLUSIVE_HISTORY_TABLE, APP_SUM_DB_SIZE, APP_SUM_DB_DAY);
    dq_db_vacuum_flag = 0;
  }
  
#endif

#if 0 
  arp_list_dump(&arlist);
#endif

  return;
}

int ev_timer_vacuum_db(struct ev_loop *loop, ev_timer *timer)
{
    // initialize a timer watcher, then start it
    // simple non-repeating TIMER_DURATION second timeout
    ev_init(timer, func_db_vacuum);
    timer->repeat = DNSQD_VACUUM_DB_TIMER;
    ev_timer_again(loop, timer);

    return 0;
}

int ev_timer_app_client_aggregator(struct ev_loop *loop, ev_timer *timer)
{
    // initialize a timer watcher, then start it
    // simple non-repeating TIMER_DURATION second timeout
    ev_init(timer, func_app_client_aggregator);
    timer->repeat = DNSQD_APP_CLIENT_AGGREGATOR_TIMER;
    ev_timer_again(loop, timer);

    return 0;
}

int ev_timer_app_sum_worker(struct ev_loop *loop, ev_timer *timer)
{
    // initialize a timer watcher, then start it
    // simple non-repeating TIMER_DURATION second timeout
    ev_init(timer, func_app_sum_worker);
    timer->repeat = DNSQD_APP_SUM_WORKER_TIMER;
    ev_timer_again(loop, timer);

    return 0;
}

int ev_timer_dev_identify_worker(struct ev_loop *loop, ev_timer *timer)
{
    // initialize a timer watcher, then start it
    // simple non-repeating TIMER_DURATION second timeout
    ev_init(timer, func_dev_identify_worker);
    timer->repeat = DNSQD_DEV_IDENTIFY_WORKER_TIMER;
    ev_timer_again(loop, timer);

    return 0;
}

int ev_timer_bw_mon_worker(struct ev_loop *loop, ev_timer *timer)
{
    // initialize a timer watcher, then start it
    // simple non-repeating TIMER_DURATION second timeout
    ev_init(timer, func_bw_mon_worker);
    timer->repeat = DNSQD_BW_MON_TIMER;
    ev_timer_again(loop, timer);

    return 0;
}

int ev_timer_env_update_worker(struct ev_loop *loop, ev_timer *timer)
{
    // initialize a timer watcher, then start it
    // simple non-repeating TIMER_DURATION second timeout
    ev_init(timer, func_env_update_worker);
    timer->repeat = DNSQD_ENV_UPDATE_TIMER;
    ev_timer_again(loop, timer);

    return 0;
}


int ev_timer_main_looper(struct ev_loop *loop, ev_timer *timer)
{
    // initialize a timer watcher, then start it
    // simple non-repeating TIMER_DURATION second timeout
    ev_init(timer, func_main_looper);
    timer->repeat = DNSQD_MAIN_LOOPER_TIMER;
    ev_timer_again(loop, timer);

    return 0;
}

void sig_str(int signum, char *sigstr, size_t sigstr_sz)
{
    char sig_default[16];
    char *sig;

    switch (signum) {
    case SIGHUP:
        sig = "SIGHUP";      break;
    case SIGINT:
        sig = "SIGINT";      break;
    case SIGQUIT:
        sig = "SIGQUIT";     break;
    case SIGILL:
        sig = "SIGILL";      break;
    case SIGTRAP:
        sig = "SIGTRAP";     break;
    case SIGABRT:
        sig = "SIGABRT";     break;
    case SIGBUS:
        sig = "SIGBUS";      break;
    case SIGFPE:
        sig = "SIGFPE";      break;
    case SIGKILL:
        sig = "SIGKILL";     break;
    case SIGUSR1:
        sig = "SIGUSR1";     break;
    case SIGSEGV:
        sig = "SIGSEGV";     break;
    case SIGUSR2:
        sig = "SIGUSR2";     break;
    case SIGPIPE:
        sig = "SIGPIPE";     break;
    case SIGALRM:
        sig = "SIGALRM";     break;
    case SIGTERM:
        sig = "SIGTERM";     break;
    case SIGCHLD:
        sig = "SIGCHLD";     break;
    case SIGCONT:
        sig = "SIGCONT";     break;
    case SIGSTOP:
        sig = "SIGSTOP";     break;
    case SIGTSTP:
        sig = "SIGTSTP";     break;
    case SIGTTIN:
        sig = "SIGTTIN";     break;
    case SIGTTOU:
        sig = "SIGTTOU";     break;
    case SIGURG:
        sig = "SIGURG";      break;
    case SIGXCPU:
        sig = "SIGXCPU";     break;
    case SIGXFSZ:
        sig = "SIGXFSZ";     break;
    case SIGVTALRM:
        sig = "SIGVTALRM";   break;
    case SIGPROF:
        sig = "SIGPROF";     break;
    case SIGWINCH:
        sig = "SIGWINCH";    break;
    case SIGIO:
        sig = "SIGIO";       break;
    case SIGPWR:
        sig = "SIGPWR";      break;
    case SIGSYS:
        sig = "SIGSYS";      break;

    default:
        snprintf(sig_default, sizeof(sig_default), "%d", signum);
        sig = sig_default;
        break;
    }

    snprintf(sigstr, sigstr_sz, "%s (%s)", sig, strsignal(signum));
}

void signal_action(struct ev_loop *loop, ev_signal *w, int e)
{
    char sigstr[64];
    int var;

    sig_str(w->signum, sigstr, sizeof(sigstr));
    //printf("Signal %s received.\n", sigstr);

    switch (w->signum) {
    case SIGUSR1:
      //  printf("++g_disable_bw_mon=%d\n", g_disable_bw_mon);
        g_disable_bw_mon = 1;
      //  printf("--g_disable_bw_mon=%d\n", g_disable_bw_mon);
        break;
    case SIGUSR2:
      //  printf("++g_disable_bw_mon=%d\n", g_disable_bw_mon);
        g_disable_bw_mon = 0;
        gLastBWReq = time(NULL);
      //  printf("--g_disable_bw_mon=%d\n", g_disable_bw_mon);
        break;

    default:
        /* At this point the handler for this signal was reset
           (except for SIGUSR2) due to the SA_RESETHAND flag;
           so re-send the signal to ourselves in order to properly crash */
        raise(w->signum);
    }

}

int ev_signal_sigusr1(struct ev_loop *loop, ev_signal *signaler)
{
    ev_signal_init(signaler, signal_action, SIGUSR1);
    ev_signal_start(loop, signaler);

    return 0;
}
int ev_signal_sigusr2(struct ev_loop *loop, ev_signal *signaler)
{
    ev_signal_init(signaler, signal_action, SIGUSR2);
    ev_signal_start(loop, signaler);

    return 0;
}

void ev_main_loop_thread(void * data)
{
  struct ev_loop *loop = EV_DEFAULT;
  ev_timer timer_main_looper;
  ev_timer timer_vacuum_db;
  ev_timer timer_app_client_aggregator;
  ev_timer timer_app_sum_worker;
  ev_timer timer_dev_identify_worker;
  ev_timer timer_bw_mon_worker;
  ev_timer timer_env_update_worker;
  ev_signal signal_sigusr1;
  ev_signal signal_sigusr2;
    
  ev_signal_sigusr1(loop, &signal_sigusr1); // re-parse
  ev_signal_sigusr2(loop, &signal_sigusr2); // re-parse
   

  ev_timer_vacuum_db(loop, &timer_vacuum_db);
  if(dns_dpi_trf_db_enable) {
    ev_timer_app_client_aggregator(loop, &timer_app_client_aggregator);
    ev_timer_app_sum_worker(loop, &timer_app_sum_worker);
  }
  ev_timer_dev_identify_worker(loop, &timer_dev_identify_worker);
  ev_timer_bw_mon_worker(loop, &timer_bw_mon_worker);
  ev_timer_env_update_worker(loop, &timer_env_update_worker);
  ev_timer_main_looper(loop, &timer_main_looper);
  
  ev_run(loop, 0);  
 
  ev_loop_destroy(loop);
  pthread_exit(NULL);
}

static bool dnsqd_write_pid_file(char *fname)
{
    char pidstr[32];

    FILE *fp = fopen(fname, "w+");
    if (!fp)  return false;

    sprintf(pidstr, "%d", getpid());
    fputs(pidstr, fp);
    fclose(fp);

    return true;
}

int main(int argc, char **argv)
{
    struct nfq_handle *h;
    struct nfq_q_handle *qh;
    struct nfnl_handle *nh;
    int fd;
    int rv;
    char buf[4096];
    pthread_t tid;
    int status;
    
    dns_dpi_trf_db_enable = nvram_get_int("dns_dpi_trf_analysis");
    
    dns_db = sqlite_open(DNS_QUERY, dns_db, DNS_QUERY_DB_FILE);
    app_client_db = sqlite_open(APP_CLIENT, app_client_db, APP_CLIENT_DB_FILE);
    app_sum_db = sqlite_open(APP_SUM, app_sum_db, APP_SUM_DB_FILE);
  

    assert((h = nfq_open()) != NULL);
    assert(nfq_unbind_pf(h, AF_INET) == 0);
    assert(nfq_bind_pf(h, AF_INET) == 0);

    assert(nfq_unbind_pf(h, AF_INET6) == 0);
    assert(nfq_bind_pf(h, AF_INET6) == 0);
    
    assert((qh = nfq_create_queue(h, 0, &cb, NULL)) != NULL);
    assert(nfq_set_mode(qh, NFQNL_COPY_PACKET, 0xffff) == 0);

    fd = nfq_fd(h);

    //init ar list
    arp_list_parse(&arlist);

    // init lan info according to br0/wifi intf
    lan_list_parse(&lanlist);

    dnsqd_write_pid_file(DNSQD_PID_FILE);

    status = pthread_create(&tid, NULL, ev_main_loop_thread, NULL);
    if(status !=0)
      dnsdbg("thread creat fail=%d at %d", status);
    pthread_detach(tid);
    while ((rv = recv(fd, buf, sizeof(buf), 0)) && rv >= 0) {
#ifdef DNSQD_DEBUG
    	dnsdbg("handle - received rv=%d", rv);
#endif
	nfq_handle_packet(h, buf, rv);
    }

    nfq_destroy_queue(qh);
    arp_list_free(&arlist);
    lan_list_free(&lanlist);

    nfq_close(h);
    sqlite_close(dns_db);
    sqlite_close(app_client_db);
    sqlite_close(app_sum_db);
    
    return 0;
}
