/*
<:copyright-BRCM:2022:DUAL/GPL:standard

   Copyright (c) 2022 Broadcom 
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
/****************************************************************************
 * Clock RPC Service Driver
 *
 * Author: Dima Mamut <dima.mamut@broadcom.com>
*****************************************************************************/
#include "itc_rpc.h"
#include "clk_rpc_svc.h"
#include <linux/types.h>
#include <api_public.h>
#include <string.h>
#include <stdio.h>

//#define DEBUG

#define RPC_SERVICE_VER_CLK_DOMAIN_ID        (0)
#define RPC_SERVICE_VER_CLK_DOMAIN_NAME      (0)
#define RPC_SERVICE_VER_CLK_GET_DOMAIN_STATE (0)
#define RPC_SERVICE_VER_CLK_SET_DOMAIN_STATE (0)
#define CLK_SVC_RPC_REQUEST_TIMEOUT          (1) /* sec */

struct clk_msg {
    uint32_t hdr;
    struct {
        uint8_t id;
        struct {
            uint8_t enable:1;
            uint8_t rsvd0:7;
        };
        uint8_t rsvd1;
        uint8_t rc;
    };
    union {
        char name[CLK_DOMAIN_NAME_MAX_LEN];
        struct {
            uint32_t rate; /* kHz */
            uint32_t rsvd2;
        };
    };
};

enum clk_svc_func_idx {
   CLK_SVC_DOMAIN_ID,
   CLK_SVC_DOMAIN_NAME,
   CLK_SVC_GET_DOMAIN_STATE,
   CLK_SVC_SET_DOMAIN_STATE,
   CLK_SVC_FUNC_MAX
};

/* clk_svc rpc message manipulation helpers */
static inline uint8_t clk_svc_msg_get_retcode(rpc_msg *msg)
{
    struct clk_msg *clk_msg = (struct clk_msg *)msg;
    return clk_msg->rc;
}

/* CLK service helpers */
static inline int clk_svc_request(rpc_msg *msg)
{
    int ret = 0;
   
    ret = rpc_send_request_timeout(RPC_TUNNEL_ARM_SMC_NS, msg, CLK_SVC_RPC_REQUEST_TIMEOUT);
#ifdef  DEBUG
    rpc_dump_msg(msg);
#endif  
    if (ret) 
    {
        printf("%s:%d : ERROR: clk_svc: rpc_send_request failure (%d)\n",__FUNCTION__, __LINE__, ret);
        return -1;
    }

    ret = clk_svc_msg_get_retcode(msg);
    if (ret) 
        printf("%s:%d : ERROR: clk_svc: rpc_send_request failure (%d)\n",__FUNCTION__, __LINE__, ret);

    return ret;
}

/* CLK service calls */
static int clk_get_domain_id(const char *name, uint8_t name_size, uint8_t *id)
{
    struct clk_msg clk_msg;
    rpc_msg *msg = (rpc_msg *)&clk_msg;
    int ret = 0;

    if (name == NULL || name_size > CLK_DOMAIN_NAME_MAX_LEN || id == NULL)
    {
        printf("%s:%d : ERROR: input parameters NULL\n",__FUNCTION__, __LINE__);
        return -1;
    }

    rpc_msg_init(msg, RPC_SERVICE_CLK, CLK_SVC_DOMAIN_ID,RPC_SERVICE_VER_CLK_DOMAIN_ID, 0, 0, 0);

    memcpy(clk_msg.name, name, name_size);
    ret = clk_svc_request(msg);
    if (ret)
    {
        printf("%s:%d : ERROR: clk_svc: failure (%d)\n",__FUNCTION__, __LINE__, ret);
        return -1;
    }

    *id = clk_msg.id;

    return ret;
}

static int clk_get_domain_state(uint8_t id, uint8_t *enabled, uint32_t *rate)
{
    struct clk_msg clk_msg;
    rpc_msg *msg = (rpc_msg *)&clk_msg;
    int ret = 0;

    if (enabled == NULL || rate == NULL)
    {
        printf("%s:%d : ERROR: input parameters NULL\n",__FUNCTION__, __LINE__);
        return -1;
    }

    rpc_msg_init(msg, RPC_SERVICE_CLK, CLK_SVC_GET_DOMAIN_STATE,
        RPC_SERVICE_VER_CLK_GET_DOMAIN_STATE, 0, 0, 0);

    clk_msg.id = id;
    ret = clk_svc_request(msg);
    if (ret)
    {
        printf("%s:%d : ERROR: clk_svc: failure (%d)\n",__FUNCTION__, __LINE__, ret);
        return -1;
    }

    *enabled = clk_msg.enable;
    *rate = clk_msg.rate;

    return ret;
}

static int clk_set_domain_state(uint8_t id, uint8_t enable, uint32_t rate)
{
    struct clk_msg clk_msg;
    rpc_msg *msg = (rpc_msg *)&clk_msg;

    rpc_msg_init(msg, RPC_SERVICE_CLK, CLK_SVC_SET_DOMAIN_STATE,
         RPC_SERVICE_VER_CLK_SET_DOMAIN_STATE, 0, 0, 0);

    clk_msg.id = id;
    clk_msg.enable = enable;
    clk_msg.rate = rate;

    return clk_svc_request(msg);
}

int bcm_rpc_clk_set_domain_state(char *name, uint8_t name_size, uint8_t enabled, uint32_t rate)
{
    uint8_t id = 0;
    int ret = 0;

    if (name == NULL || name_size > CLK_DOMAIN_NAME_MAX_LEN)
    {
        printf("%s:%d : ERROR: input parameters NULL\n",__FUNCTION__, __LINE__);
        return -1;
    }

    if (clk_get_domain_id(name, name_size, &id))
    {
        printf("%s:%d : ERROR: invalid domain id[0x%x] name[%s]\n",__FUNCTION__, __LINE__,id, name);
        return -1;
    }

    if (clk_set_domain_state(id, enabled, rate))
    {
        printf("%s:%d : ERROR: invalid domain enabled[0x%x] rate[0x%x]\n",__FUNCTION__, __LINE__, enabled, rate);
        return -1;
    }

#ifdef DEBUG
    printf("%s:%d : DEBUG: name[%s] enabled[0x%x] rate[0x%x]\n",__FUNCTION__, __LINE__, name, enabled, rate);
#endif      

    return ret;
}

int bcm_rpc_clk_get_domain_state(char *name, uint8_t name_size, uint8_t *enabled, uint32_t *rate)
{
    uint8_t id = 0;
    int ret = 0;

    if (name == NULL || name_size > CLK_DOMAIN_NAME_MAX_LEN || enabled == NULL || rate == NULL)
    {
       printf("%s:%d : ERROR: input parameters NULL\n",__FUNCTION__, __LINE__);
       return -1;
    }

    if (clk_get_domain_id(name, name_size, &id))
    {
        printf("%s:%d : ERROR: invalid domain id[0x%x] name[%s]\n",__FUNCTION__, __LINE__, id, name);
        return -1;
    }

    if (clk_get_domain_state(id, enabled, rate))
    {
        printf("%s:%d : ERROR: invalid domain enable[0x%x] state[0x%x] \n",__FUNCTION__, __LINE__, *enabled, *rate);
        return -1;
    }

#ifdef DEBUG
    printf("%s:%d : DEBUG: name[%s] enable[0x%x] state[0x%x]\n",__FUNCTION__, __LINE__, name, *enabled, *rate);
#endif   

    return ret;
}

int bcm_rpc_clk_get_rate(char *name, uint8_t name_size, uint32_t *rate)
{
    int ret = 0;
    uint8_t enabled;
    uint8_t id = 0;

    if (name == NULL || name_size > CLK_DOMAIN_NAME_MAX_LEN || rate == NULL)
    {
        printf("%s:%d : ERROR: input parameters NULL\n",__FUNCTION__, __LINE__);
        return -1;
    }

    if (clk_get_domain_id(name, name_size, &id))
    {
        printf("%s:%d : ERROR: invalid domain id[0x%x] name[%s]\n",__FUNCTION__, __LINE__,id, name);
        return -1;
    }

    ret = clk_get_domain_state(id, &enabled, rate);
    if (ret) 
    {
        printf("%s:%d : ERROR: clk_svc: failure (%d)\n",__FUNCTION__, __LINE__, ret);
        return -1;
    }

    *rate = (*rate / 1000); /* Rate in kHz */

#ifdef  DEBUG
    printf("%s:%d DEBUG: getting clock ID %d rate to %d kHz\n", __FUNCTION__, __LINE__,id, *rate);
#endif 

    return ret;
}

int bcm_rpc_clk_set_rate(char *name, uint8_t name_size, uint32_t rate)
{
    int ret;
    uint8_t enabled;
    uint32_t temp_rate;
    uint8_t id = 0;

    if (name == NULL || name_size > CLK_DOMAIN_NAME_MAX_LEN)
    {
        printf("%s:%d : ERROR: input parameters NULL\n",__FUNCTION__, __LINE__);
        return -1;
    }

    if (clk_get_domain_id(name, name_size, &id))
    {
        printf("%s:%d : ERROR: invalid domain id[0x%x] name[%s]\n",__FUNCTION__, __LINE__, id, name);
        return -1;
    }

    ret = clk_get_domain_state(id, &enabled, &temp_rate);
    if (ret) 
    {
        printf(" %s:%d: ERROR: retrieving state of clock ID %d\n",__FUNCTION__, __LINE__, id);
        return -1;
    }

    temp_rate = (rate * 1000); /* Rate in kHz */
    ret = clk_set_domain_state(id, enabled, rate);
    if (ret) 
    {
        printf("%s:%d : ERROR: clk_svc: failure (%d)\n",__FUNCTION__, __LINE__, ret);
        return -1;
    }

#ifdef  DEBUG
    printf("%s:%d DEBUG: setting clock ID %d rate to %d kHz\n", __FUNCTION__, __LINE__, id, rate);
#endif   

   return ret;
}

char bcm_rpc_clk_is_enabled(char *name, uint8_t name_size)
{
    int ret = 0;
    uint8_t enabled;
    uint32_t rate;
    uint8_t id = 0;
 
    if (name == NULL || name_size > CLK_DOMAIN_NAME_MAX_LEN )
    {
        printf("%s:%d : ERROR: input parameters NULL\n",__FUNCTION__, __LINE__);
        return -1;
    }

    if (clk_get_domain_id(name, name_size, &id))
    {
        printf("%s:%d : ERROR: invalid domain id[0x%x] name[%s]\n",__FUNCTION__, __LINE__, id, name);
        return -1;
    }

    ret = clk_get_domain_state(id, &enabled, &rate);
    if (ret) 
    {
        printf("%s:%d retrieving state of clock ID %d",__FUNCTION__, __LINE__, id);
        return -1;
    } 

#ifdef  DEBUG
    printf("%s:%d DEBUG: setting clock ID %d is %s",__FUNCTION__, __LINE__, id, enabled ? "enabled" : "disabled");
#endif

    return enabled;
}

int bcm_rpc_clk_enable_disable(char *name, uint8_t name_size, char enable)
{
    int ret;
    uint32_t rate;
    uint8_t id = 0;
    uint8_t enabled;

    if (name == NULL || name_size > CLK_DOMAIN_NAME_MAX_LEN )
    {
        printf("%s:%d : ERROR: input parameters NULL\n",__FUNCTION__, __LINE__);
        return -1;
    }

    if (clk_get_domain_id(name, name_size, &id))
    {
        printf("%s:%d : ERROR: invalid domain id[0x%x] name[%s]\n",__FUNCTION__, __LINE__,id, name);
        return -1;
    }

    ret = clk_get_domain_state(id, &enabled, &rate);
    if (ret) 
    {
        printf("%s:%d retrieving state of clock ID %d",__FUNCTION__, __LINE__, id);
        return -1;
    } 
   
    ret = clk_set_domain_state(id, enable, rate);
    if (ret) 
    {
        printf("%s:%d : ERROR: clk_svc: failure (%d)\n",__FUNCTION__, __LINE__, ret);
        return -1;
    }

#ifdef  DEBUG
    printf("%s:%d %s DEBUG: clock ID[%d]\n",__FUNCTION__, __LINE__, enable ? "enabled" : "disabled", id);
#endif   

    return ret;
}

