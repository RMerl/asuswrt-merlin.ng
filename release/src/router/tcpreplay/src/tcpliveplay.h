

/*
 * Main Author & Publisher: Yazan Siam (tcpliveplay@gmail.com)
 * File: tcpliveplay.h
 * Started as a Senior Design project @ North Carolina State University
 * Last Updated Date: September 5, 2012
 * Past Contributors (Last contributed May 4, 2012): Andrew Leonard & Beau Luck
 */

#pragma once

#include "defines.h"
#include "config.h"

#define SIZE_ETHERNET 14
#define LOCAL_IP_MATCH 1
#define REMOTE_IP_MATCH 2
#define NO_MATCH 0
#define PCAP_OPEN_ERROR (-1)
#define TIMEOUT_ms 10000
#define PROMISC_OFF 0
#define BUFSIZ_PLUS BUFSIZ
#define ALARM_TIMEOUT 10
#define SUCCESS 1
#define ERROR (-1)

/***********From tcpedit.h*****/
#define TCPEDIT_SOFT_ERROR (-2)
#define TCPEDIT_ERROR (-1)
#define TCPEDIT_OK 0
#define TCPEDIT_WARN 1

/************From checksum.h******/
#define CHECKSUM_CARRY(x) (x = (x >> 16) + (x & 0xffff), (~(x + (x >> 16)) & 0xffff))

#include <stdbool.h>

// 6 byte MAC Address
struct mac_addr {
    unsigned char byte1;
    unsigned char byte2;
    unsigned char byte3;
    unsigned char byte4;
    unsigned char byte5;
    unsigned char byte6;
};

typedef struct ip_addr input_addr;
// 4 bytes IP address
struct ip_addr {
    unsigned char byte1;
    unsigned char byte2;
    unsigned char byte3;
    unsigned char byte4;
};

typedef struct ether_hdr ether_hdr;
/* Ethernet header */
struct ether_hdr {
    u_char ether_dhost[ETHER_ADDR_LEN]; /* Destination host address */
    u_char ether_shost[ETHER_ADDR_LEN]; /* Source host address */
    u_short ether_type;                 /* IP? ARP? RARP? etc */
};

typedef struct ipv4_hdr ipv4_hdr;

struct ipv4_hdr {
#if defined(WORDS_BIGENDIAN)
    u_int8_t ip_v:4;
    u_int8_t ip_hl:4;
#else
    u_int8_t ip_hl:4;
    u_int8_t ip_v:4;
#endif
    u_int8_t ip_tos;
    u_int16_t ip_len;
    u_int16_t ip_id;
    u_int16_t ip_off;
    u_int8_t ip_ttl;
    u_int8_t ip_p;
    u_int16_t ip_sum;
    input_addr ip_src, ip_dst;
};

typedef struct tcpheader tcp_hdr;

/* for easy reference ************ */
struct tcpheader {
    u_int16_t th_sport; // source port
    u_int16_t th_dport;
    u_int32_t th_seq;
    u_int32_t th_ack;
#if defined(WORDS_BIGENDIAN)
    u_int8_t th_off:4;
    u_int8_t th_x2:4;
#else
    u_int8_t th_x2:4;
    u_int8_t th_off:4;
#endif
    u_int8_t th_flags;
    u_int16_t th_win;
    u_int16_t th_sum;
    u_int16_t th_urp;
};

#define TH_FIN 0x01
#define TH_SYN 0x02
#define TH_RST 0x04
#define TH_PUSH 0x08
#define TH_ACK 0x10
#define TH_URG 0x20
#define TH_ECE 0x40
#define TH_CWR 0x80

struct tcp_sched {
    u_int32_t exp_rseq;             /* Expected Remote SEQ */
    u_int32_t exp_rack;             /* Expected Remote ACK */
    u_int32_t calc_curr_rseq;       /* Calculated Current Remote SEQ (not used at the moment) */
    u_int32_t calc_curr_rack;       /* Calculated Current Remote ACK (not used at the moment) */
    u_int32_t calc_curr_lseq;       /* Calculated Current Local SEQ (not used at the moment) */
    u_int32_t calc_curr_lack;       /* Calculated Current Local ACK (not used at the moment) */
    u_int32_t curr_lseq;            /* Current Local SEQ */
    u_int32_t curr_lack;            /* Current Local ACK */
    unsigned int length_curr_ldata; /* Data Length of Currently seen local data */
    unsigned int length_last_ldata; /* Data Length of last locally seen data */
    unsigned int length_curr_rdata; /* Length of currently seen remote data */
    unsigned int length_last_rdata; /* Length of last remote seen data*/
    u_char *packet_ptr;             /* The entire packet data to be sent */
    struct pcap_pkthdr pkthdr;      /* Packet header */
    ether_hdr *etherhdr;            /* Ethernet Header */
    tcp_hdr *tcphdr;                /* TCP Header */
    ipv4_hdr *iphdr;                /* IP Header */
    unsigned int size_ip;           /* Keep track of each packet's IP Size */
    unsigned int size_tcp;          /* Keep track of each packet's TCP Size */
    unsigned int size_payload;      /* Keep tack of each packet's Payload size, if any */
    unsigned int sent_counter;      /* Keep track of each packet's sent attempts*/
    bool remote;                    /* Flag to signify this is a remote packet */
    bool local;                     /* Flag to signify this is a local packet */
};
