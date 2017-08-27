#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <netinet/in.h>
#include <net/if.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/select.h>
#include <endian.h>

#include "stun.h"

 #define MAX(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

#define STUN_TYPE(method, class)            \
        ((method)&0x0f80) << 2 |            \
        ((method)&0x0070) << 1 |            \
        ((method)&0x000f) << 0 |            \
        ((class)&0x2)     << 7 |            \
        ((class)&0x1)     << 4

static int stun_hdr_encode(char *buf, const struct stun_hdr *hdr)
{
    int buf_pos=0;
    uint16_t tmp16=0;
    uint32_t tmp32=0;

    if (!buf || !hdr)
        return -1;

    tmp16 = htons(hdr->type & 0x3fff);
    memcpy(buf,&tmp16,2);
    buf_pos+=2;

    tmp16 = htons(hdr->len);
    memcpy(buf+buf_pos,&tmp16,2);
    buf_pos+=2;

    tmp32 = htonl(hdr->cookie);
    memcpy(buf+buf_pos,&tmp32,4);
    buf_pos+=4;

    memcpy(buf+buf_pos,hdr->tid,12);

    return 0;
}

static int stun_msg_vencode(char *buf, uint16_t method, uint8_t class,
                     const uint8_t *tid, uint8_t padding )
{
    struct stun_hdr hdr;
    int err = 0;

    if ( !buf || !tid )
        return -1;

    memset(&hdr,0,sizeof(hdr));

    hdr.type   = STUN_TYPE(method, class);
    hdr.cookie = STUN_MAGIC_COOKIE;
    memcpy(hdr.tid, tid, STUN_TID_SIZE);

    hdr.len = 0;

    err |= stun_hdr_encode(buf, &hdr);

    return err;
}

static uint32_t rand_u32(int i)
{
    srand(time(NULL)+i);
    return rand();
}

static int hdr_and_attr_check(uint8_t *buf,int size,uint8_t *tid,uint32_t *ipaddr)
{
    uint8_t binding_resp_success[2] = {0x01,0x01};
    uint8_t magic_cookie[4] = {0x21,0x12,0xa4,0x42};
    int success=0;
    int buf_pos=0;
    uint16_t attr_size=0,attr_type=0,attr_len=0;


    /*hdr check*/
    if( size < STUN_MESSAGETYPE_SIZE+STUN_MESSAGELEN_SIZE+STUN_MAGICCOOKIE_SIZE+STUN_TID_SIZE )
    {
        printf("Receive size is incorrect.[%d]\n",size);
        return -1;
    }

    if( memcmp(buf,binding_resp_success,STUN_MESSAGETYPE_SIZE) )
    {
        printf("Receive[%d] < Excepted[%d]\n",size,buf_pos);
        printf("The header type is not success.[0x%02x%02x]\n",buf[0],buf[1]);
        return -1;
    }
    buf_pos+=STUN_MESSAGETYPE_SIZE;

    memcpy(&attr_size,buf+buf_pos,STUN_MESSAGELEN_SIZE);
    attr_size = ntohs(attr_size);
    buf_pos+=STUN_MESSAGELEN_SIZE;

    if(  memcmp(buf+buf_pos,magic_cookie,STUN_MAGICCOOKIE_SIZE) ) {
        printf("Receive[%d] < Excepted[%d]\n",size,buf_pos);
        printf("The magic cookie is incorrect.[0x%02x%02x%02x%02x]\n",buf[4],buf[5],buf[6],buf[7]);
        return -1;
    }
    buf_pos+=STUN_MAGICCOOKIE_SIZE;

    if( memcmp( buf+buf_pos, tid, STUN_TID_SIZE ) ) {
        printf("Receive[%d] < Excepted[%d]\n",size,buf_pos);
        printf("The tid is incorrect.[0x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x]\n",buf[8],buf[9]
                                                ,buf[10],buf[11],buf[12],buf[13],buf[14],buf[15]
                                                ,buf[16],buf[17],buf[18],buf[19]);
        printf("It should be         [0x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x]\n",tid[0],tid[1]
                                                ,tid[2],tid[3],tid[4],tid[5],tid[6],tid[7]
                                                ,tid[8],tid[9],tid[10],tid[11]);
        return -1;
    }
    buf_pos+=STUN_TID_SIZE;

    /*attr check*/
    while(1)
    {
        /* get attr type*/
        if( buf_pos + STUN_ATTRTYPE_SIZE > size )
            break;
        memcpy(&attr_type,buf+buf_pos,STUN_ATTRTYPE_SIZE);
        attr_type = ntohs(attr_type);
        buf_pos+=STUN_ATTRTYPE_SIZE;

        if( attr_type == STUN_MAPPEDADDRESS )
        {
            if( buf_pos + STUN_ATTRLEN_SIZE > size )
                break;
            memcpy(&attr_len,buf+buf_pos,STUN_ATTRLEN_SIZE);
            buf_pos+=STUN_ATTRLEN_SIZE;
            attr_len = ntohs(attr_len);
            if( attr_len != MAPPEDADDRESS_SIZE || buf_pos + attr_len > size )
                break;
            success = 1;

            memcpy(ipaddr,buf+buf_pos+4,4);
            break;
        } else if ( attr_type == STUN_XORMAPPEDADDRESS ) {
            if( buf_pos + STUN_ATTRLEN_SIZE > size )
                break;
            memcpy(&attr_len,buf+buf_pos,STUN_ATTRLEN_SIZE);
            buf_pos+=STUN_ATTRLEN_SIZE;
            attr_len = ntohs(attr_len);
            if( attr_len != MAPPEDADDRESS_SIZE || buf_pos + attr_len > size )
                break;
            success = 1;

            memcpy(ipaddr,buf+buf_pos+4,4);

            *ipaddr = htonl(*ipaddr);
            *ipaddr ^= STUN_MAGIC_COOKIE;
            *ipaddr = ntohl(*ipaddr);
            break;
        } else {
            if( buf_pos + STUN_ATTRLEN_SIZE > size )
                break;
            memcpy(&attr_len,buf+buf_pos,STUN_ATTRLEN_SIZE);
            buf_pos+=STUN_ATTRLEN_SIZE;
            attr_len = ntohs(attr_len);
            printf("attr_len %d\n",attr_len);

            buf_pos+=attr_len;
        }
    }

    if(success)
        return 0;
    return -1;
}

/** Gather Server Reflexive address */
int send_binding_request(unsigned int stun_ip,unsigned short stun_port,unsigned int *dieve_public_ip)
{
    uint8_t tid[STUN_TID_SIZE];
    int sockfd=0,n=0,maxfd=0;
    struct sockaddr_in servaddr;
    uint8_t sendline[32] = {0};
    uint8_t recvline[256] = {0};
    int i=0,err=0,ret=0,success=0,retry_times=0;
    struct timeval timeout;
    fd_set fds;

    *dieve_public_ip = 0;

    for (i=0; i<STUN_TID_SIZE; i++)
        tid[i] = rand_u32(i);

    err = stun_msg_vencode( (char *) sendline,
                            STUN_METHOD_BINDING,
                            STUN_CLASS_REQUEST,
                            tid,
                            0x00);
    if(err)
        goto error_out;

    sockfd=socket(AF_INET,SOCK_DGRAM,0);
    if(!sockfd)
        goto error_out;

    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr=stun_ip;
    servaddr.sin_port=htons(stun_port);

    n = sendto(sockfd,sendline,BINDING_REQUEST_SIZE,0,(struct sockaddr *)&servaddr,sizeof(servaddr));
    if( n != BINDING_REQUEST_SIZE )
        printf("[WARNING]sendto size error\n");

    while(1)
    {
        retry_times++;
        timeout.tv_sec = 5;
        timeout.tv_usec = 0;
        FD_ZERO(&fds);
        FD_SET(sockfd,&fds);
        maxfd=sockfd+1;
        switch( select(maxfd,&fds,NULL,NULL,&timeout) )
        {
            case -1:
                if(MAX_STUN_RETRY < retry_times)
                    goto error_out;
                break;

            case 0:
                if(MAX_STUN_RETRY < retry_times)
                    goto error_out;

                n = sendto(sockfd,sendline,BINDING_REQUEST_SIZE,0,(struct sockaddr *)&servaddr,sizeof(servaddr));
                if( n != BINDING_REQUEST_SIZE )
                    printf("[WARNING]sendto size error\n");
                break;

            default:
                if(MAX_STUN_RETRY < retry_times)
                    goto error_out;

                if( FD_ISSET(sockfd,&fds) )
                {
                    n = recvfrom(sockfd,recvline,256,0,NULL,NULL);
                    if( n > 0 )
                    {
                        ret = hdr_and_attr_check(recvline,n,tid,dieve_public_ip);
                        if(!ret)
                           success = 1;
                    }
                }
        }

        if(success)
            break;
    }

    if(sockfd)
        close(sockfd);
    return 0;

error_out:

    if(sockfd)
        close(sockfd);

    printf("Send STUN binding request error\n");

    return -1;

}
