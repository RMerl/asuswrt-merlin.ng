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
 * Power RPC Service Driver
 *
 * Author: Dima Mamut <dima.mamut@broadcom.com>
*****************************************************************************/

#include <asm/cacheflush.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/dma-mapping.h>
#include <linux/platform_device.h>
#include <itc_rpc.h>
#include <power_rpc_svc.h>
#include <pmc_rpc.h>

//#define DEBUG 

enum {
    PWR_SVC_GET_DOMAIN_ID,
    PWR_SVC_GET_DOMAIN_NAME,
    PWR_SVC_GET_DOMAIN_STATE,
    PWR_SVC_SET_DOMAIN_STATE,
    MAX_FUNC,
};

struct pwr_msg {
    uint32_t hdr;
    struct {
        uint8_t id;
        uint8_t state;
        struct {
            uint8_t reset:2;
            uint8_t rsvd:6;
        };
        uint8_t rc;
    };
    char name[8];
};

/* pwr_svc rpc message manipulation helpers */
static uint8_t power_msg_get_retcode(rpc_msg *msg)
{
    struct pwr_msg *pwr_msg = (struct pwr_msg *)msg;
    return pwr_msg->rc;
}

static int pwr_get_domain_id(char *name, uint8_t name_size, uint8_t *id)
{
    struct pwr_msg pwr_msg;
    rpc_msg *msg = (rpc_msg *)&pwr_msg;
    int ret = 0;

    if (name == NULL || name_size > PWR_DOMAIN_NAME_MAX_LEN || id == NULL)
    {
        printk("%s:%d : ERROR: input parameters NULL\n",__FUNCTION__, __LINE__);
        return -1;
    }

    rpc_msg_init(msg, RPC_SERVICE_PWR, PWR_SVC_GET_DOMAIN_ID, RPC_SERVICE_VER_PWR_DOMAIN_ID, 0, 0, 0);

    memcpy(pwr_msg.name, name, name_size);
    ret = pmc_svc_request(msg, power_msg_get_retcode);   
    if (ret)
    {
        printk("%s:%d : ERROR: power_svc: rpc_send_request failure (%d)\n",__FUNCTION__, __LINE__, ret);
        return -1;
    }   

    *id = pwr_msg.id;
#ifdef DEBUG
    printk("%s:%d Debug: id[0x%x]\n",__FUNCTION__, __LINE__, *id);
#endif  

    return ret;
}

static int pwr_get_domain_state(uint8_t id, pwr_dom_state *state, pwr_dom_reset *reset)
{
    struct pwr_msg pwr_msg;
    rpc_msg *msg = (rpc_msg *)&pwr_msg;
    int ret = 0;

    rpc_msg_init(msg, RPC_SERVICE_PWR, PWR_SVC_GET_DOMAIN_STATE, RPC_SERVICE_VER_PWR_GET_DOMAIN_STATE, 0, 0, 0);

    pwr_msg.id = id;
    ret = pmc_svc_request(msg, power_msg_get_retcode); 
    if (ret)
    {
        printk("%s:%d : ERROR: power_svc: rpc_send_request failure (%d)\n",__FUNCTION__, __LINE__, ret);
        return -1;
    }

    *state = pwr_msg.state;
    *reset = pwr_msg.state;

    return ret;
}

static int pwr_set_domain_state(uint8_t id, pwr_dom_state state, pwr_dom_reset reset)
{
    struct pwr_msg pwr_msg;
    rpc_msg *msg = (rpc_msg *)&pwr_msg;
    int ret = 0;

    rpc_msg_init(msg, RPC_SERVICE_PWR, PWR_SVC_SET_DOMAIN_STATE, RPC_SERVICE_VER_PWR_SET_DOMAIN_STATE, 0, 0, 0);
    pwr_msg.id = id;
    pwr_msg.state = state;
    pwr_msg.reset = reset;

    ret = pmc_svc_request(msg, power_msg_get_retcode); 
    if (ret)
    {
        printk("%s:%d : ERROR: power_svc: rpc_send_request failure (%d)\n",__FUNCTION__, __LINE__, ret);
        return -1;
    }

    return ret;
}

int bcm_rpc_pwr_get_domain_state(char *name, uint8_t name_size, pwr_dom_state *state, pwr_dom_reset *reset)
{
    uint8_t id = 0;
    int ret = 0;

    if (name == NULL || name_size > PWR_DOMAIN_NAME_MAX_LEN || state == NULL || reset == NULL)
    {
        printk("%s:%d : ERROR: input parameters NULL\n",__FUNCTION__, __LINE__);
        return -1;
    }

    if (pwr_get_domain_id(name, name_size, &id))
    {
        printk("%s:%d : ERROR: invalid domain id[0x%x] name[%s]\n",__FUNCTION__, __LINE__,id, name);
        return -1;
    }

    if (pwr_get_domain_state(id, state, reset))
    {
        printk("%s:%d : ERROR: invalid domain state[0x%x] \n",__FUNCTION__, __LINE__,*state);
        return -1;
    }

#ifdef DEBUG
    printk("%s:%d : DEBUG: name[%s] state[0x%x] reset[0x%x]\n",__FUNCTION__, __LINE__, name, *state, *reset);
#endif   

    return ret;
}

int bcm_rpc_pwr_set_domain_state(char *name, uint8_t name_size, pwr_dom_state state, pwr_dom_reset reset)
{
    uint8_t id = 0;
    int ret = 0;

    if (name == NULL || name_size > PWR_DOMAIN_NAME_MAX_LEN)
    {
        printk("%s:%d : ERROR: input parameters NULL\n",__FUNCTION__, __LINE__);
        return -1;
    }

    if (pwr_get_domain_id(name, name_size, &id))
    {
        printk("%s:%d : ERROR: invalid domain id[0x%x] name[%s]\n",__FUNCTION__, __LINE__,id, name);
        return -1;
    }

    if (pwr_set_domain_state(id, state, reset))
    {
        printk("%s:%d : ERROR: invalid domain state[0x%x]\n",__FUNCTION__, __LINE__,state);
        return -1;
    }

#ifdef DEBUG
    printk("%s:%d : DEBUG: name[%s] state[0x%x] reset[0x%x]\n",__FUNCTION__, __LINE__, name, state, reset);
#endif      

    return ret;
}


