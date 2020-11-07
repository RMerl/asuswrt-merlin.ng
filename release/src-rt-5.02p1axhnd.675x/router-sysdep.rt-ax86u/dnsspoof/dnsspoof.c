/***********************************************************************
 *
 *  Copyright (c) 2003-2010  Broadcom Corporation
 *  All Rights Reserved
 *
<:label-BRCM:2012:proprietary:standard

 This program is the proprietary software of Broadcom and/or its
 licensors, and may only be used, duplicated, modified or distributed pursuant
 to the terms and conditions of a separate, written license agreement executed
 between you and Broadcom (an "Authorized License").  Except as set forth in
 an Authorized License, Broadcom grants no license (express or implied), right
 to use, or waiver of any kind with respect to the Software, and Broadcom
 expressly reserves all rights in and to the Software and all intellectual
 property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
 NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
 BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.

 Except as expressly set forth in the Authorized License,

 1. This program, including its structure, sequence and organization,
    constitutes the valuable trade secrets of Broadcom, and you shall use
    all reasonable efforts to protect the confidentiality thereof, and to
    use this information only in connection with your use of Broadcom
    integrated circuit products.

 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
    RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
    ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
    FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
    COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
    TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
    PERFORMANCE OF THE SOFTWARE.

 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
    ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
    INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
    WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
    IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
    OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
    SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
    SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
    LIMITED REMEDY.
:>
 *
************************************************************************/

/**************************************************************************
 * File Name  : dnsspoof
 *
 * Description: This program receives DNS query packet requests on the DNS
 *              socket and always responds with the local LAN IP address.
 *              It is used when a WAN connection cannot be established in
 *              order to force Web access to the modem.
 *
 * Updates    : 10/06/2003  Created.
 ***************************************************************************/


/** Includes. **/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <errno.h>
#include <syslog.h>

#include <net/if.h>
#include <linux/sockios.h>
#include <sys/uio.h>


/** Defines. **/

#define ERROR                           -1
#define DNS_PORT                        53
#define DNSHDR_FLAG_RSP                 0x8000
#define DNSQA_NAME_NO_AUTH_NO_CONF      (0xc000 + sizeof(DNSHDR))
#define DNSQA_TYPE_HOST_ADDR            0x0001
#define DNSQA_CLASS_INET                0x0001
#define MIN_DNS_QUESTION_SIZE           5

#define DNS_MK_SHORT(x) \
    (((unsigned short)(*(unsigned char *) (x))<<8) | (*((unsigned char *)(x)+1)))


/** Typedefs. **/

typedef struct
{
    unsigned short dnshdr_query_id;
    unsigned short dnshdr_flags;
    unsigned short dnshdr_questions;
    unsigned short dnshdr_answers;
    unsigned short dnshdr_authority;
    unsigned short dnshdr_resource;
} DNSHDR;

typedef struct
{
    unsigned short dnsans_name;
    unsigned short dnsans_type;
    unsigned short dnsans_class;
    unsigned short dnsans_ttl_high;
    unsigned short dnsans_ttl_low;
    unsigned short dnsans_data_len;
    unsigned char dnsans_data[4];
} DNSANS;


/** Globals. **/

static int s = -1;


/** Prototypes. **/

void done(int sig);
void extract_ip_addr( char *ip_addr_str, unsigned char *ip_addr_buf );

/***************************************************************************
 * Function Name: main
 * Description  : Main program function.
 * Returns      : 0 - success, non-0 - error.
 ***************************************************************************/
int main(int argc, char **argv)
{
    struct sockaddr_in sa;
    struct sockaddr_in da;
    int da_len;
    int num;
    char dns_buf[256];
    unsigned char ip_addr[4] = {255, 255, 255, 255};

    if( argc == 2 )
        extract_ip_addr( argv[1], ip_addr );
    else
        printf( "dnsspoof: error - missing local IP address parameter\n" );

    /* Proceed if the local IP address was successfully exctracted from the
     * command line.
     */
    if( ip_addr[0] != 255 )
    {
        signal(SIGABRT, done);

        /* Create a socket. */
        if( (s = socket(AF_INET, SOCK_DGRAM, 0)) != ERROR )
        {
            memset( &sa, 0x00, sizeof(sa) );
            sa.sin_family = AF_INET;
            sa.sin_addr.s_addr = INADDR_ANY;
            sa.sin_port = DNS_PORT;

            /* Bind it to the DNS port. */
            if(bind(s, (struct sockaddr *)&sa, sizeof(struct sockaddr)) != ERROR)
            {
                for( ; ; )
                {
                    /* Wait to receive a DNS query. */
                    da_len = sizeof(struct sockaddr_in);
                    num = recvfrom(s, (caddr_t) dns_buf, sizeof(dns_buf), 0,
                        (struct sockaddr *) &da, (socklen_t *)&da_len);
                    if( num >= sizeof(DNSHDR) + MIN_DNS_QUESTION_SIZE )
                    {
                        DNSHDR *hdr = (DNSHDR *) dns_buf;
                        char *p = dns_buf + sizeof(DNSHDR);

                        if( (hdr->dnshdr_flags & DNSHDR_FLAG_RSP) == 0 &&
                            hdr->dnshdr_questions >= 1 )
                        {
                            /* A DNS query request has been received.  Skip past
                             * the DNS query name.
                             */
                            while( *p++ )
                                ;

                            /* Verify the DNS request type and class. */
                            if( DNS_MK_SHORT(p) == DNSQA_TYPE_HOST_ADDR &&
                                DNS_MK_SHORT(&p[2]) == DNSQA_CLASS_INET )
                            {
                                /* Create the DNS response specifying the
                                 * local IP address.
                                 */
                                DNSANS rsp = {DNSQA_NAME_NO_AUTH_NO_CONF,
                                    DNSQA_TYPE_HOST_ADDR, DNSQA_CLASS_INET, 0,
                                    0, 4, {0}};

                                rsp.dnsans_data[0] = ip_addr[0];
                                rsp.dnsans_data[1] = ip_addr[1];
                                rsp.dnsans_data[2] = ip_addr[2];
                                rsp.dnsans_data[3] = ip_addr[3];

                                hdr->dnshdr_flags = DNSHDR_FLAG_RSP;
                                hdr->dnshdr_answers = 1;

                                memcpy(dns_buf + num, (char *)&rsp, sizeof(rsp));

                                /* Send the DNS response. */
                                sendto( s, dns_buf, num + sizeof(rsp), 0,
                                    (struct sockaddr *) &da, da_len);
                            }
                        }
                    }
                    else
                        break;
                }

                close(s);
                s = -1;
            }
            else
            {
                printf("dnsspoof: Socket error %d.\n", errno);
                close(s);
                s = -1;
            }
        }
        else
            printf("dnsspoof: Bind error %d.\n", errno);
    }

    return( 0 );
} /* main */


/***************************************************************************
 * Function Name: done
 * Description  : Called when the process is killed.
 * Returns      : 0
 ***************************************************************************/
void done(int sig)
{
    if( s != -1 )
        close(s);
    exit(0);
} /* done */


/***************************************************************************
 * Function Name: extract_ip_addr
 * Description  : Converts a string ip addr, AAA.BBB.CCC.DDD to a binary
 *                array of four bytes.
 * Returns      : 0
 ***************************************************************************/
void extract_ip_addr( char *ip_addr_str, unsigned char *ip_addr_buf )
{
    char *p1 = ip_addr_str;
    char *p2;
    int i;

    for( i = 0, p2 = p1; i < 3 && *p1; i++, p2 = p1 )
    {
        while( *p1 && *p1 != '.' )
            p1++;

        if( *p1 == '.' )
        {
            ip_addr_buf[i] = (unsigned char) strtol( p2, (char **) NULL, 10 );
            p1++;
        }
    }

    if( *p1 ) /* success */
        ip_addr_buf[3] = (unsigned char) strtol( p2, (char **) NULL, 10 );
    else /* error */
        ip_addr_buf[0] = ip_addr_buf[1] = ip_addr_buf[2] = ip_addr_buf[3] = 255;
} /* extract_ip_addr */

