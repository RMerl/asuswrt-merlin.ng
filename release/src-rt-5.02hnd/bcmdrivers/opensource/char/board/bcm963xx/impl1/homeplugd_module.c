/*
<:copyright-BRCM:2014:GPL/GPL:standard

   Copyright (c) 2014 Broadcom 
   All Rights Reserved

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License, version 2, as published by
the Free Software Foundation (the "GPL").

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.


A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.

:>
*/

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/version.h>
#include <linux/workqueue.h>
#include <net/sock.h>
#include <linux/udp.h>
#include <linux/net.h>
#include <homeplugd_module.h>
#include <board.h>

void homeplugd_send_msg(unsigned long time, void* param)
{
    struct socket *sk;
    struct sockaddr_in to;
    struct msghdr msg;
    struct iovec iov;
    homeplugd_msg_header udp_msg;
    int sendLen;

    if (sock_create(PF_INET, SOCK_DGRAM, IPPROTO_UDP, &sk) < 0) {
        printk("board driver: Error creating udp socket\n");
        return;
    }

    if ((homeplugd_event)param == PLC_UKE)
        udp_msg.type = HOMEPLUGD_UKE_CMD_START;
    else
        udp_msg.type = HOMEPLUGD_UKE_CMD_RANDOMIZE;
    udp_msg.len = 0;

    memset(&to, 0, sizeof(to));
    to.sin_family = AF_INET;
    to.sin_addr.s_addr = htonl(0x7f000001);
    to.sin_port = htons((unsigned short)HOMEPLUGD_MESSAGE_PORT);

    iov.iov_base = &udp_msg;
    iov.iov_len = sizeof(udp_msg);

    memset(&msg, 0, sizeof(msg));
    msg.msg_name = &to;
    msg.msg_namelen = sizeof(to);
    msg.msg_control    = NULL;
    msg.msg_controllen = 0;
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 0, 0)
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
#else
    iov_iter_init(&msg.msg_iter, WRITE, &iov, 1, iov.iov_len);
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 0, 0)
    sendLen = sock_sendmsg(sk, &msg, sizeof(udp_msg));
#else
    sendLen = sock_sendmsg(sk, &msg);
#endif
    if (sendLen != sizeof(udp_msg)){
        printk("board driver: Error sending message to homeplugd\n");
    }

    sock_release(sk);
    return;
}

static int __init brcm_board_homeplugd_init(void)
{
    kerSysRegisterPlcUkeCallback(homeplugd_send_msg, (void*)PLC_UKE);
    kerSysRegisterPlcRandomizeCallback(homeplugd_send_msg, (void*)PLC_RANDOMIZE_KEY);
    return 0;
}

module_init(brcm_board_homeplugd_init);
