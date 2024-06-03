/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * Copyright  (C) 2019-2020  MediaTek Inc. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <asm/types.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <linux/if_arp.h>
#include <arpa/inet.h>  //htons etc
#include <os.h>

#define PROTO_ARP 0x0806
#define ETH2_HEADER_LEN 14
#define HW_TYPE 1
#define MAC_LENGTH 6
#define IPV4_LENGTH 4
#define ARP_REQUEST 0x01
#define ARP_REPLY 0x02
#define BUF_SIZE 60

#define debug(x...) printf(x);printf("\n");
#define info(x...) printf(x);printf("\n");
#define warn(x...) printf(x);printf("\n");
#define err(x...) printf(x);printf("\n");
#define PRINT_MAC(addr)	\
	addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]

struct arp_header {
    unsigned short hardware_type;
    unsigned short protocol_type;
    unsigned char hardware_len;
    unsigned char protocol_len;
    unsigned short opcode;
    unsigned char sender_mac[MAC_LENGTH];
    unsigned char sender_ip[IPV4_LENGTH];
    unsigned char target_mac[MAC_LENGTH];
    unsigned char target_ip[IPV4_LENGTH];
};

extern unsigned int if_nametoindex (const char *__ifname);

/*
 * Converts struct sockaddr with an IPv4 address to network byte order uin32_t.
 * Returns 0 on success.
 */
int int_ip4(struct sockaddr *addr, uint32_t *ip)
{
    if (addr->sa_family == AF_INET) {
        struct sockaddr_in *i = (struct sockaddr_in *) addr;
        *ip = i->sin_addr.s_addr;
        return 0;
    } else {
        err("Not AF_INET");
        return 1;
    }
}

/*
 * Formats sockaddr containing IPv4 address as human readable string.
 * Returns 0 on success.
 */
int format_ip4(struct sockaddr *addr, char *out)
{
    if (addr->sa_family == AF_INET) {
        struct sockaddr_in *i = (struct sockaddr_in *) addr;
        const char *ip = inet_ntoa(i->sin_addr);
        if (!ip) {
            return -2;
        } else {
            os_snprintf(out, 20, "%s", ip); /* ip4 string addr len is less than 15 bytes */
            return 0;
        }
    } else {
        return -1;
    }
}

/*
 * Writes interface IPv4 address as network byte order to ip.
 * Returns 0 on success.
 */
int get_if_ip4(int fd, const char *ifname, uint32_t *ip) {
    int err = -1;
    struct ifreq ifr;
    memset(&ifr, 0, sizeof(struct ifreq));
    if (strlen(ifname) > (IFNAMSIZ - 1)) {
        err("Too long interface name");
        goto out;
    }

    os_snprintf(ifr.ifr_name, IFNAMSIZ, "%s", ifname);
    if (ioctl(fd, SIOCGIFADDR, &ifr) == -1) {
        perror("SIOCGIFADDR");
        goto out;
    }

    if (int_ip4(&ifr.ifr_addr, ip)) {
        goto out;
    }
    err = 0;
out:
    return err;
}

/*
 * Sends an ARP who-has request to dst_ip
 * on interface ifindex, using source mac src_mac and source ip src_ip.
 */
int send_arp(int fd, int ifindex, char *src_mac, uint32_t src_ip, uint32_t dst_ip)
{
    int err = -1;
    unsigned char buffer[BUF_SIZE];
    memset(buffer, 0, sizeof(buffer));

    struct sockaddr_ll socket_address;
    socket_address.sll_family = AF_PACKET;
    socket_address.sll_protocol = htons(ETH_P_ARP);
    socket_address.sll_ifindex = ifindex;
    socket_address.sll_hatype = htons(ARPHRD_ETHER);
    socket_address.sll_pkttype = (PACKET_BROADCAST);
    socket_address.sll_halen = MAC_LENGTH;
    socket_address.sll_addr[6] = 0x00;
    socket_address.sll_addr[7] = 0x00;

    struct ethhdr *send_req = (struct ethhdr *) buffer;
    struct arp_header *arp_req = (struct arp_header *) (buffer + ETH2_HEADER_LEN);
    uint32_t ret;

    //Broadcast
    memset(send_req->h_dest, 0xff, MAC_LENGTH);

    //Target MAC zero
    memset(arp_req->target_mac, 0x00, MAC_LENGTH);

    //Set source mac to our MAC address
    memcpy(send_req->h_source, src_mac, MAC_LENGTH);
    memcpy(arp_req->sender_mac, src_mac, MAC_LENGTH);
    memcpy(socket_address.sll_addr, src_mac, MAC_LENGTH);

    /* Setting protocol of the packet */
    send_req->h_proto = htons(ETH_P_ARP);

    /* Creating ARP request */
    arp_req->hardware_type = htons(HW_TYPE);
    arp_req->protocol_type = htons(ETH_P_IP);
    arp_req->hardware_len = MAC_LENGTH;
    arp_req->protocol_len = IPV4_LENGTH;
    arp_req->opcode = htons(ARP_REQUEST);

    //debug("Copy IP address to arp_req");
    memcpy(arp_req->sender_ip, &src_ip, sizeof(uint32_t));
    memcpy(arp_req->target_ip, &dst_ip, sizeof(uint32_t));

    ret = sendto(fd, buffer, 42, 0, (struct sockaddr *) &socket_address, sizeof(socket_address));
    if (ret == -1) {
        perror("sendto():");
        goto out;
    }
    err = 0;
out:
    return err;
}

/*
 * Sends an ARP who-has request to dst_ip
 * on interface ifindex, using source mac src_mac and source ip src_ip.
 */
int send_garp(int fd, int ifindex, char *src_mac, uint32_t src_ip)
{
    int err = -1;
    unsigned char buffer[BUF_SIZE];
    memset(buffer, 0, sizeof(buffer));

    struct sockaddr_ll socket_address;
    socket_address.sll_family = AF_PACKET;
    socket_address.sll_protocol = htons(ETH_P_ARP);
    socket_address.sll_ifindex = ifindex;
    socket_address.sll_hatype = htons(ARPHRD_ETHER);
    socket_address.sll_pkttype = (PACKET_BROADCAST);
    socket_address.sll_halen = MAC_LENGTH;
    socket_address.sll_addr[6] = 0x00;
    socket_address.sll_addr[7] = 0x00;

    struct ethhdr *send_req = (struct ethhdr *) buffer;
    struct arp_header *arp_req = (struct arp_header *) (buffer + ETH2_HEADER_LEN);
    uint32_t ret;

    //Broadcast
    memset(send_req->h_dest, 0xff, MAC_LENGTH);

    //Target MAC zero
    memset(arp_req->target_mac, 0x00, MAC_LENGTH);

    //Set source mac to our MAC address
    memcpy(send_req->h_source, src_mac, MAC_LENGTH);
    memcpy(arp_req->sender_mac, src_mac, MAC_LENGTH);
    memcpy(socket_address.sll_addr, src_mac, MAC_LENGTH);

    /* Setting protocol of the packet */
    send_req->h_proto = htons(ETH_P_ARP);

    /* Creating ARP request */
    arp_req->hardware_type = htons(HW_TYPE);
    arp_req->protocol_type = htons(ETH_P_IP);
    arp_req->hardware_len = MAC_LENGTH;
    arp_req->protocol_len = IPV4_LENGTH;
    arp_req->opcode = htons(ARP_REQUEST);

    debug("Copy IP address to arp_req");
    memcpy(arp_req->sender_ip, &src_ip, sizeof(uint32_t));
    memset(arp_req->target_ip, 0, sizeof(uint32_t));

    ret = sendto(fd, buffer, 42, 0, (struct sockaddr *) &socket_address, sizeof(socket_address));
    if (ret == -1) {
        perror("sendto():");
        goto out;
    }
    err = 0;
out:
    return err;
}

/*
 * Gets interface information by name:
 * IPv4
 * MAC
 * ifindex
 */
int get_if_info(const char *ifname, int *ip, char *mac, int *ifindex)
{
    int err = -1;
    struct ifreq ifr;
    int sd = -1;

    debug("get_if_info for %s", ifname);
	sd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ARP));
    if (sd <= 0) {
        perror("socket()");
        goto out;
    }
    if (strlen(ifname) > (IFNAMSIZ - 1)) {
        printf("Too long interface name, MAX=%i\n", IFNAMSIZ - 1);
        goto out;
    }

    os_snprintf(ifr.ifr_name, IFNAMSIZ, "%s", ifname);

    //Get interface index using name
    if (ioctl(sd, SIOCGIFINDEX, &ifr) == -1) {
        perror("SIOCGIFINDEX");
        goto out;
    }
    *ifindex = ifr.ifr_ifindex;
    printf("interface index is %d\n", *ifindex);

    //Get MAC address of the interface
    if (ioctl(sd, SIOCGIFHWADDR, &ifr) == -1) {
        perror("SIOCGIFHWADDR");
        goto out;
    }

    //Copy mac address to output
    memcpy(mac, ifr.ifr_hwaddr.sa_data, MAC_LENGTH);

    if (get_if_ip4(sd, (const char *)ifname, (uint32_t *)ip)) {
	err = -2;
        goto out;
    }
    debug("get_if_info OK");

    err = 0;
out:
    if (sd >= 0) {
        debug("Clean up temporary socket");
        close(sd);
    }
    return err;
}

/*
 * Creates a raw socket that listens for ARP traffic on specific ifindex.
 * Writes out the socket's FD.
 * Return 0 on success.
 */
int bind_arp(int ifindex, int *fd)
{
    int ret = -1;

    debug("bind_arp: ifindex=%i", ifindex);
    // Submit request for a raw socket descriptor.
    *fd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ARP));
    if (*fd < 1) {
        perror("socket()");
        goto out;
    }

    debug("Binding to ifindex %i", ifindex);
    struct sockaddr_ll sll;
    memset(&sll, 0, sizeof(struct sockaddr_ll));
    sll.sll_family = AF_PACKET;
    sll.sll_ifindex = ifindex;
    if (bind(*fd, (struct sockaddr*) &sll, sizeof(struct sockaddr_ll)) < 0) {
        perror("bind");
        goto out;
    }

    ret = 0;
out:
    if (ret && *fd > 0) {
        debug("Cleanup socket");
        close(*fd);
    }
    return ret;
}


/*
 *
 * Sample code that send packet on air can be used for debugging
 * interface <ifname> to IPv4 address <ip>.
 * Returns 0 on success.
 */
int test_my_garping(const char *ifname) {
    int ret = -1;

    int src;
    int ifindex;
    char mac[MAC_LENGTH];
    if (get_if_info(ifname, &src, mac, &ifindex)) {
        goto out;
    }
    int arp_fd;
    if (bind_arp(ifindex, &arp_fd)) {
        err("Failed to bind_arp()");
        goto out;
    }
	mac[0] = 0xaa;
	mac[1] = 0xaa;
	mac[2] = 0xaa;
	mac[3] = 0xaa;
	mac[4] = 0xaa;
	mac[5] = 0xaa;

	src = 0x0b0b0b23;
    if (send_garp(arp_fd, ifindex, mac, src)) {
        err("Failed to send_arp");
        goto out;
    }

    ret = 0;
out:
    if (arp_fd) {
        close(arp_fd);
        arp_fd = 0;
    }
    return ret;
}
/*
 *
 * Sample code that sends an ARP who-has request on
 * interface <ifname> to IPv4 address <ip>.
 * Returns 0 on success.
 */
int test_arping(int sock, int ifindex, char *mac, uint32_t srcip, uint32_t dstip) {
    int ret = 0;

	ret = send_arp(sock, ifindex, mac, srcip, dstip);
    return ret;
}

