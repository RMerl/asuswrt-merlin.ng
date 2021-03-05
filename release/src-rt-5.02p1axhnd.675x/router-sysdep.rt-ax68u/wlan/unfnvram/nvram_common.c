/***********************************************************************
 *
 *  Copyright (c) 2018  Broadcom
 *  All Rights Reserved
 *
<:label-BRCM:2018:proprietary:standard

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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <bcmnvram.h>
#include "nvram_netlink.h"
#include "nvram_api.h"

#define VALIDATE_BIT(bit) do { if ((bit < 0) || (bit > 31)) return NULL; } while (0)
#define VALIDATE_BIT_INT(bit) do { if ((bit < 0) || (bit > 31)) return 0; } while (0)
#define CODE_BUFF	16
#define HEX_BASE	16

#ifdef KERNEL_NVRAM_AUTO_SET
char* kernel_nvram_prefix_table[] = {
        "pci/%d/",
        "sb/%d/",
        "%d:",
        "devpath%d",
        NULL,
};
#endif

/* Common API */

int common_nvram_kernel_set(const char *name, const char *value)
{
    int ret = -1;
    int buflen;
    t_WLCSM_NAME_VALUEPAIR *buf;
    char temp_buf[MAX_NLRCV_BUF_SIZE];

    buflen=_get_valuepair_total_len(name,value,0);
    buf=get_namevalue_buf(name,value,0);
    if(buf!=NULL) {
        memset(temp_buf, 0, MAX_NLRCV_BUF_SIZE);
        if(netlink_send_mesg(WLCSM_MSG_NVRAM_SET,(char *)buf,buflen)== 0) {
            t_WLCSM_MSG_HDR *hdr=netlink_recv_mesg(temp_buf);
            if(hdr!=NULL && hdr->type==WLCSM_MSG_NVRAM_SET) {
                ret = 0;
            }
        }
        netlink_free();
        free(buf);
    }
    return ret;
}

int common_nvram_kernel_get(const char *nvname, char *value, size_t size)
{
    int ret = -1;
    char temp_buf[MAX_NLRCV_BUF_SIZE] = {0};

    if(netlink_send_mesg(WLCSM_MSG_NVRAM_GET, (char *)nvname, strlen(nvname)+1) == 0)
    {
        t_WLCSM_MSG_HDR *hdr = netlink_recv_mesg(temp_buf);

        if(hdr != NULL && hdr->type == WLCSM_MSG_NVRAM_GET)
        {
            char *value_buf = NULL;

            if(hdr->len)
                value_buf = VALUEPAIR_VALUE((t_WLCSM_NAME_VALUEPAIR *)(hdr+1));

            if(value_buf != NULL)
            {
                if(strlen(value_buf) < size)
                {
                    ret = 0;
                    strncpy(value, value_buf, size);
                }
            }
        }
    }
    netlink_free();
    return ret;
}

int common_nvram_kernel_getall(const char *buf, size_t size)
{
    int ret = 0;
    t_WLCSM_MSG_HDR *hdr;
    int type=WLCSM_MSG_NVRAM_GETALL;
    char temp_buf[MAX_NLRCV_BUF_SIZE] = {0};
    int  msg_data[2];
    int len=0, index = 0;

    if(!buf)
        return -1;

    memset((void*)buf, '\0', size);

    msg_data[0] = size;
    while(type != WLCSM_MSG_NVRAM_GETALL_DONE)
    { 

        msg_data[1] = index;
        if(netlink_send_mesg(WLCSM_MSG_NVRAM_GETALL, (char *)msg_data, sizeof(msg_data)) == 0)
        {
            hdr = netlink_recv_mesg(temp_buf);
            if(!hdr)
            {
                ret = -1;
                break;
            }

            if(hdr->len==0) break;

            type = hdr->type;
            if((type == WLCSM_MSG_NVRAM_GETALL) || (type == WLCSM_MSG_NVRAM_GETALL_DONE))
            {
                if((size-len) > hdr->len)
                {
                    memcpy((void*)buf, (char *)(hdr+1), hdr->len);
                    buf += hdr->len;
                    len += hdr->len;
                    index++;
                }
                else 
                {
                    ret = -1;
                    fprintf(stderr,"getall buf is not big enough!\n");
                    break;
                }

            }
        }
    }
    netlink_free();
    return ret;
}

char * common_nvram_get_bitflag(const char *name, int bit)
{
    VALIDATE_BIT(bit);
    char *ptr = nvram_unf_get(name);
    unsigned long nvramvalue = 0;
    unsigned long bitflagvalue = 1;

    if (ptr) {
        bitflagvalue = bitflagvalue << bit;
        nvramvalue = strtoul(ptr, NULL, HEX_BASE);
        if (nvramvalue) {
            nvramvalue = nvramvalue & bitflagvalue;
        }
        free(ptr);
        return (nvramvalue ? "1" : "0");
    }
    return NULL;
}

int common_nvram_set_bitflag(const char *name, int bit, int value)
{
    VALIDATE_BIT_INT(bit);
    char nvram_val[CODE_BUFF];
    char *ptr = nvram_unf_get(name);
    unsigned long nvramvalue = 0;
    unsigned long bitflagvalue = 1;

    memset(nvram_val, 0, sizeof(nvram_val));

    if (ptr) {
        bitflagvalue = bitflagvalue << bit;
        nvramvalue = strtoul(ptr, NULL, HEX_BASE);
        if (value) {
            nvramvalue |= bitflagvalue;
        } else {
            nvramvalue &= (~bitflagvalue);
        }
        free(ptr);
    }
    snprintf(nvram_val, sizeof(nvram_val)-1, "%lx", nvramvalue);
    return nvram_set(name, nvram_val);
}

#ifdef KERNEL_NVRAM_AUTO_SET
int match_kernel_nvram(const char *nvname)
{
    unsigned int i = 0;
    int item1;
    int ret = 0;

    if(!nvname)
        return ret;

    while (kernel_nvram_prefix_table[i] != NULL)
    {
        if(sscanf(nvname, kernel_nvram_prefix_table[i], &item1) == 1)
        {
            //printf("kernel nvram:%s matched!!\n", nvname);
            ret = 1;
            break;
        }
        i++;
    }
    return ret;
}
#endif

