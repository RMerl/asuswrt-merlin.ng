#if defined(CONFIG_BCM_KF_VLANCTL_BIND) && defined(CONFIG_BLOG)
/*
*    Copyright (c) 2003-2014 Broadcom Corporation
*    All Rights Reserved
*
<:label-BRCM:2014:DUAL/GPL:standard 

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
#include "bcm_OS_Deps.h"
#include <linux/bcm_log.h>
#include <linux/blog.h>

#include <linux/kernel.h>
#include <linux/vlanctl_bind.h>

static vlanctl_bind_SnHook_t vlanctl_bind_sn_hook_g[VLANCTL_BIND_CLIENT_MAX] = { (vlanctl_bind_SnHook_t)NULL };
static vlanctl_bind_ScHook_t vlanctl_bind_sc_hook_g[VLANCTL_BIND_CLIENT_MAX] = { (vlanctl_bind_ScHook_t)NULL };
static vlanctl_bind_SdHook_t vlanctl_bind_sd_hook_g[VLANCTL_BIND_CLIENT_MAX] = { (vlanctl_bind_SdHook_t)NULL };

#if defined(CC_VLANCTL_BIND_SUPPORT_DEBUG)
#define vlanctl_assertr(cond, rtn)                                              \
    if ( !cond ) {                                                              \
        printk( CLRerr "VLANCTL_BIND ASSERT %s : " #cond CLRnl, __FUNCTION__ ); \
        return rtn;                                                             \
    }
#else
#define vlanctl_assertr(cond, rtn) NULL_STMT
#endif

/*------------------------------------------------------------------------------
 *  Function    : vlanctl_bind_config
 *  Description : Override default config and deconf hook.
 *  vlanctl_sc  : Function pointer to be invoked in blog_activate()
 *  client      : configuration client
 *------------------------------------------------------------------------------
 */                      
void vlanctl_bind_config(vlanctl_bind_ScHook_t vlanctl_bind_sc, 
	                     vlanctl_bind_SdHook_t vlanctl_bind_sd,  
	                     vlanctl_bind_SnHook_t vlanctl_bind_sn,  
	                     vlanctl_bind_client_t client, 
                         vlanctl_bind_t bind)
{
    BCM_LOG_DEBUG(BCM_LOG_ID_VLAN,  "vlanctl Bind Sc[<%p>] Sd[<%p>] Sn[<%p>] Client[<%u>] bind[<%u>]",
                vlanctl_bind_sc, vlanctl_bind_sd, vlanctl_bind_sn, client, (uint8_t)bind.hook_info);

    if ( bind.bmap.SC_HOOK )
        vlanctl_bind_sc_hook_g[client] = vlanctl_bind_sc;   /* config hook */
    if ( bind.bmap.SD_HOOK )
        vlanctl_bind_sd_hook_g[client] = vlanctl_bind_sd;   /* deconf hook */
    if ( bind.bmap.SN_HOOK )
        vlanctl_bind_sn_hook_g[client] = vlanctl_bind_sn;   /* notify hook */
}



/*
 *------------------------------------------------------------------------------
 * Function     : vlanctl_activate
 * Description  : This function invokes vlanctl configuration hook
 * Parameters   :
 *  blog_p      : pointer to a blog with configuration information
 *  client      : configuration client
 *
 * Returns      :
 *  ActivateKey : If the configuration is successful, a key is returned.
 *                Otherwise, BLOG_KEY_INVALID is returned
 *------------------------------------------------------------------------------
 */
uint32_t vlanctl_activate( Blog_t * blog_p, vlanctl_bind_client_t client )
{
    uint32_t     key;

    key = BLOG_KEY_INVALID;
    
    if ( blog_p == BLOG_NULL || client >= VLANCTL_BIND_CLIENT_MAX )
    {
        vlanctl_assertr((blog_p != BLOG_NULL), key);
        goto bypass;
    }

    if (unlikely(vlanctl_bind_sc_hook_g[client] == (vlanctl_bind_ScHook_t)NULL))
        goto bypass;


    BLOG_LOCK_BH();
    key = vlanctl_bind_sc_hook_g[client](blog_p, BlogTraffic_Layer2_Flow);
    BLOG_UNLOCK_BH();

bypass:
    return key;
}

/*
 *------------------------------------------------------------------------------
 * Function     : vlanctl_deactivate
 * Description  : This function invokes a deconfiguration hook
 * Parameters   :
 *  key         : blog key information
 *  client      : configuration client
 *
 * Returns      :
 *  blog_p      : If the deconfiguration is successful, the associated blog 
 *                pointer is returned to the caller
 *------------------------------------------------------------------------------
 */
Blog_t * vlanctl_deactivate( uint32_t key, vlanctl_bind_client_t client )
{
    Blog_t * blog_p = NULL;

    if ( key == BLOG_KEY_INVALID || client >= VLANCTL_BIND_CLIENT_MAX )
    {
        vlanctl_assertr( (key != BLOG_KEY_INVALID), blog_p );
        goto bypass;
    }

    if ( unlikely(vlanctl_bind_sd_hook_g[client] == (vlanctl_bind_SdHook_t)NULL) )
        goto bypass;

    BLOG_LOCK_BH();
    blog_p = vlanctl_bind_sd_hook_g[client](key, BlogTraffic_Layer2_Flow);
    BLOG_UNLOCK_BH();

bypass:
    return blog_p;
}


int	vlanctl_notify(vlanctl_bind_Notify_t event, void *ptr, vlanctl_bind_client_t client)
{

   if (client >= VLANCTL_BIND_CLIENT_MAX)
       goto bypass;

    BCM_LOG_DEBUG(BCM_LOG_ID_VLAN, "client<%u>" "event<%u>", client, event);

    if (unlikely(vlanctl_bind_sn_hook_g[client] == (vlanctl_bind_SnHook_t)NULL))
        goto bypass;

	BLOG_LOCK_BH();
    vlanctl_bind_sn_hook_g[client](event, ptr);
    BLOG_UNLOCK_BH();

bypass:
    return 0;
}


EXPORT_SYMBOL(vlanctl_bind_config); 
EXPORT_SYMBOL(vlanctl_activate); 
EXPORT_SYMBOL(vlanctl_deactivate); 
EXPORT_SYMBOL(vlanctl_notify);

#endif /* defined(BCM_KF_VLANCTL_BIND && defined(CONFIG_BLOG) */
