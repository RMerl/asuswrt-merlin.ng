/*
 *
 * Copyright (C) 2019-2024, Broadband Forum
 * Copyright (C) 2007-2024  CommScope, Inc
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

/**
 * @file nu_macaddr.c
 *
 * Network MAC utilty functions.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ioctl.h>

#include <string.h>
#include <net/if.h>
#include <unistd.h>
#include <errno.h>

#include "common_defs.h"
#include "nu_macaddr.h"
#include "usp_api.h"
#include "text_utils.h"

//------------------------------------------------------------------------------
// String, set by '-i' command line option to specify the network interface to be used by USP communications
char *usp_interface = NULL;

/*********************************************************************//**
**
** nu_macaddr_wan_ifname
**
** Returns the name of the WAN interface.
** This should be the interface that's used to connect to
** remote network services such as CWMP and XMPP.
** For example this may be 'eth0'
**
** \param   None
**
** \return  pointer to string containing name of the WAN interface
**
**************************************************************************/
char *nu_macaddr_wan_ifname(void)
{
    char *e;

    // Exit if the network interface was specified by the '-i command line option when invoking this executable
    if (usp_interface != NULL)
    {
        return usp_interface;
    }

    // Exit if an environment variable is used to override the interface name
    // NOTE: This may be used with docker or with embedded systems still using the 'eth0' style naming convention
    e = getenv("USP_BOARD_IFNAME");
    if ((e != NULL) && (*e != '\0'))
    {
        return e;
    }

    return DEFAULT_WAN_IFNAME;
}

/*********************************************************************//**
**
** nu_macaddr_wan_macaddr
**
** Returns the MAC address of the WAN interface
**
** \param   buf - pointer to buffer in which to return MAC address (6 bytes raw)
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int nu_macaddr_wan_macaddr(unsigned char *buf)
{
    int err;
    int sock;
    struct ifreq ifr;

    // Set default return MAC address
    memset(buf, 0, MAC_ADDR_LEN);

    // Set up which interface we want to get the MAC address of
    memset(&ifr, 0x00, sizeof(ifr));
    strcpy(ifr.ifr_name, nu_macaddr_wan_ifname());

    // Attempt to get the MAC address of the interface
    sock = socket(PF_INET, SOCK_DGRAM, 0);
    if (sock == -1)
    {
        USP_ERR_SetMessage("%s: socket() failed. Unable to open a UDP socket (errno=%d, %m)", __FUNCTION__, errno);
        return USP_ERR_INTERNAL_ERROR;
    }

    err = ioctl(sock, SIOCGIFHWADDR, &ifr);
    close(sock);

    // Exit if unable to get the MAC address of the interface
    if (err == -1)
    {
        USP_ERR_SetMessage("%s: ioctl() failed. Unable to get MAC address for interface %s (errno=%d, %m)", __FUNCTION__, ifr.ifr_name, errno);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Copy MAC address into return buffer
    memcpy(buf, ifr.ifr_hwaddr.sa_data, MAC_ADDR_LEN);

    return USP_ERR_OK;
}
