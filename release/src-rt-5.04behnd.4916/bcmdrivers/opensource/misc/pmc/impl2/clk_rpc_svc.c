/*
<:copyright-BRCM:2022:DUAL/GPL:standard

   Copyright (c) 2022 Broadcom
   All Rights Reserved

Unless you and Broadcom execute a separate written software license
agreement governing use of this software, this software is licensed
to you under the terms of the GNU General Public License version 2
(the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
with the following added to such license:

   As a special exception, the copyright holders of this software give
   you permission to link this software with independent modules, and
   to copy and distribute the resulting executable under terms of your
   choice, provided that you also meet, for each linked independent
   module, the terms and conditions of the license of that module.
   An independent module is a module which is not derived from this
   software.  The special exception does not apply to any modifications
   of the software.

Not withstanding the above, under no circumstances may you combine
this software in any way with any other Broadcom software provided
under a license other than the GPL, without Broadcom's express prior
written consent.

:>
*/
/****************************************************************************
 * Clock RPC Service Driver
 *
 * Author: Dima Mamut <dima.mamut@broadcom.com>
*****************************************************************************/
#include <linux/platform_device.h>
#include <itc_rpc.h>
#include <clk_rpc_svc.h>
#include <pmc_rpc.h>

//#define DEBUG

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
static uint8_t clk_svc_msg_get_retcode(rpc_msg *msg)
{
    struct clk_msg *clk_msg = (struct clk_msg *)msg;
    return clk_msg->rc;
}

/* CLK service calls */
static int clk_get_domain_id(const char *name, uint8_t name_size, uint8_t *id)
{
    struct clk_msg clk_msg;
    rpc_msg *msg = (rpc_msg *)&clk_msg;
    int ret = 0;

    if (name == NULL || name_size > CLK_DOMAIN_NAME_MAX_LEN || id == NULL)
    {
        printk("%s:%d : ERROR: input parameters NULL\n",__FUNCTION__, __LINE__);
        return -1;
    }

    rpc_msg_init(msg, RPC_SERVICE_CLK, CLK_SVC_DOMAIN_ID,RPC_SERVICE_VER_CLK_DOMAIN_ID, 0, 0, 0);

    memcpy(clk_msg.name, name, name_size);
    ret = pmc_svc_request(msg, clk_svc_msg_get_retcode);
    if (ret)
    {
        printk("%s:%d : ERROR: clk_svc: failure (%d)\n",__FUNCTION__, __LINE__, ret);
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
        printk("%s:%d : ERROR: input parameters NULL\n",__FUNCTION__, __LINE__);
        return -1; 
    }    

   rpc_msg_init(msg, RPC_SERVICE_CLK, CLK_SVC_GET_DOMAIN_STATE,
         RPC_SERVICE_VER_CLK_GET_DOMAIN_STATE, 0, 0, 0);

    clk_msg.id = id;
    ret = pmc_svc_request(msg, clk_svc_msg_get_retcode);
    if (ret)
    {
        printk("%s:%d : ERROR: clk_svc: failure (%d)\n",__FUNCTION__, __LINE__, ret);
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

    return pmc_svc_request(msg, clk_svc_msg_get_retcode);
}

int bcm_rpc_clk_set_domain_state(char *name, uint8_t name_size, uint8_t enabled, uint32_t rate)
{
    uint8_t id = 0;
    int ret = 0;

    if (name == NULL || name_size > CLK_DOMAIN_NAME_MAX_LEN)
    {
        printk("%s:%d : ERROR: input parameters NULL\n",__FUNCTION__, __LINE__);
        return -1;
    }

    if (clk_get_domain_id(name, name_size, &id))
    {
        printk("%s:%d : ERROR: invalid domain id[0x%x] name[%s]\n",__FUNCTION__, __LINE__,id, name);
        return -1;
    }

    if (clk_set_domain_state(id, enabled, rate))
    {
        printk("%s:%d : ERROR: invalid domain enabled[0x%x] rate[0x%x]\n",__FUNCTION__, __LINE__,enabled, rate);
        return -1;
    }

#ifdef DEBUG
    printk("%s:%d : DEBUG: name[%s] enabled[0x%x] rate[0x%x]\n",__FUNCTION__, __LINE__, name, enabled, rate);
#endif      

    return ret;
}


int bcm_rpc_clk_get_domain_state(char *name, uint8_t name_size, uint8_t *enabled, uint32_t *rate)
{
    uint8_t id = 0;
    int ret = 0;

    if (name == NULL || name_size > CLK_DOMAIN_NAME_MAX_LEN || enabled == NULL || rate == NULL)
    {
        printk("%s:%d : ERROR: input parameters NULL\n",__FUNCTION__, __LINE__);
        return -1;
    }

    if (clk_get_domain_id(name, name_size, &id))
    {
        printk("%s:%d : ERROR: invalid domain id[0x%x] name[%s]\n",__FUNCTION__, __LINE__,id, name);
        return -1;
    }

    if (clk_get_domain_state(id, enabled, rate))
    {
        printk("%s:%d : ERROR: invalid domain enable[0x%x] state[0x%x] \n",__FUNCTION__, __LINE__,*enabled, *rate);
        return -1;
    }

#ifdef DEBUG
    printk("%s:%d : DEBUG: name[%s] enable[0x%x] state[0x%x]\n",__FUNCTION__, __LINE__, name, *enabled, *rate);
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
        printk("%s:%d : ERROR: input parameters NULL\n",__FUNCTION__, __LINE__);
        return -1;
    }

    if (clk_get_domain_id(name, name_size, &id))
    {
        printk("%s:%d : ERROR: invalid domain id[0x%x] name[%s]\n",__FUNCTION__, __LINE__, id, name);
        return -1;
    }

    ret = clk_get_domain_state(id, &enabled, rate);
    if (ret) 
    {
        printk("%s:%d : ERROR: clk_svc: failure (%d)\n",__FUNCTION__, __LINE__, ret);
        return -1;
    }

    *rate = (*rate / 1000); /* Rate in kHz */

#ifdef  DEBUG
      printk("%s:%d DEBUG: getting clock ID %d rate to %d kHz\n", __FUNCTION__, __LINE__,id, *rate);
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
        printk("%s:%d : ERROR: input parameters NULL\n",__FUNCTION__, __LINE__);
        return -1;
    }

    if (clk_get_domain_id(name, name_size, &id))
    {
        printk("%s:%d : ERROR: invalid domain id[0x%x] name[%s]\n",__FUNCTION__, __LINE__,id, name);
        return -1;
    }

   ret = clk_get_domain_state(id, &enabled, &temp_rate);
   if (ret) 
   {
        printk(" %s:%d: ERROR: retrieving state of clock ID %d\n",__FUNCTION__, __LINE__, id);
        return -1;
   }

   temp_rate = (rate * 1000); /* Rate in kHz */
   ret = clk_set_domain_state(id, enabled, rate);
   if (ret) 
   {
        printk("%s:%d : ERROR: clk_svc: failure (%d)\n",__FUNCTION__, __LINE__, ret);
        return -1;
   }

#ifdef  DEBUG
    printk("%s:%d DEBUG: setting clock ID %d rate to %d kHz\n", __FUNCTION__, __LINE__,id, rate);
#endif   

    return ret;
}

char bcm_rpc_clk_is_enabled(char *name, uint8_t name_size)
{
    int ret = 0;
    char enabled;
    uint32_t rate;
    uint8_t id = 0;

    if (name == NULL || name_size > CLK_DOMAIN_NAME_MAX_LEN )
    {
        printk("%s:%d : ERROR: input parameters NULL\n",__FUNCTION__, __LINE__);
        return -1;
    }

    if (clk_get_domain_id(name, name_size, &id))
    {
        printk("%s:%d : ERROR: invalid domain id[0x%x] name[%s]\n",__FUNCTION__, __LINE__,id, name);
        return -1;
    }

    ret = clk_get_domain_state(id, &enabled, &rate);
    if (ret) 
    {
        printk("%s:%d retrieving state of clock ID %d",__FUNCTION__, __LINE__, id);
        return -1;
    } 

#ifdef  DEBUG
    printk("%s:%d DEBUG: setting clock ID %d is %s",__FUNCTION__, __LINE__, id, enabled ? "enabled" : "disabled");
#endif

    return enabled;
}

int bcm_rpc_clk_enable_disable(char *name, uint8_t name_size, char enable)
{
    int ret;
    uint32_t rate;
    uint8_t id = 0;
    char enabled;

    if (name == NULL || name_size > CLK_DOMAIN_NAME_MAX_LEN )
    {
        printk("%s:%d : ERROR: input parameters NULL\n",__FUNCTION__,__LINE__);
        return -1;
    }

    if (clk_get_domain_id(name, name_size, &id))
    {
        printk("%s:%d : ERROR: invalid domain id[0x%x] name[%s]\n",__FUNCTION__, __LINE__,id, name);
        return -1;
    }

    ret = clk_get_domain_state(id, &enabled, &rate);
    if (ret) 
    {
        printk("%s:%d retrieving state of clock ID %d",__FUNCTION__, __LINE__, id);
        return -1;
    } 
   
    ret = clk_set_domain_state(id, enable, rate);
    if (ret) 
    {
        printk("%s:%d : ERROR: clk_svc: failure (%d)\n",__FUNCTION__,__LINE__, ret);
        return -1;
    }

#ifdef  DEBUG
    printk("%s:%d %s DEBUG: clock ID[%d]\n",__FUNCTION__,__LINE__, enable ? "enabled" : "disabled", id);
#endif   

    return ret;
}

