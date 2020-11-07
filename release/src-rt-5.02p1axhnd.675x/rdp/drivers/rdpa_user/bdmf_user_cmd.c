// <:copyright-BRCM:2013:DUAL/GPL:standard
// 
//    Copyright (c) 2013 Broadcom 
//    All Rights Reserved
// 
// Unless you and Broadcom execute a separate written software license
// agreement governing use of this software, this software is licensed
// to you under the terms of the GNU General Public License version 2
// (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
// with the following added to such license:
// 
//    As a special exception, the copyright holders of this software give
//    you permission to link this software with independent modules, and
//    to copy and distribute the resulting executable under terms of your
//    choice, provided that you also meet, for each linked independent
//    module, the terms and conditions of the license of that module.
//    An independent module is a module which is not derived from this
//    software.  The special exception does not apply to any modifications
//    of the software.
// 
// Not withstanding the above, under no circumstances may you combine
// this software in any way with any other Broadcom software provided
// under a license other than the GPL, without Broadcom's express prior
// written consent.
// 
// :>

#include "bdmf_user_interface.h"
#include "rdpa_user_int.h"

int bdmf_user_new_and_set(bdmf_ioctl_t *ba)
{
    BDMF_TRACE_DBG("ENTER bdmf_user_new_and_set");

    if ((ba->ret = bdmf_new_and_set(ba->drv, ba->owner_or_ds, ba->mattr, &ba->mo_or_us)))
    {
        BDMF_TRACE_ERR("bdmf_new_and_set failed, ret:%d\n", ba->ret);
    }

    return 0;
}

int bdmf_user_destroy(bdmf_ioctl_t *ba)
{
    BDMF_TRACE_DBG("ENTER bdmf_user_destroy");

    if ((ba->ret = bdmf_destroy(ba->mo_or_us)))
    {
        BDMF_TRACE_ERR("bdmf_destroy failed, ret:%d\n", ba->ret);
    }

    return 0;
}

int bdmf_user_mattr_alloc(bdmf_ioctl_t *ba)
{
    BDMF_TRACE_DBG("ENTER bdmf_user_mattr_alloc");
 
    if ((ba->mattr = bdmf_mattr_alloc(ba->drv)) == NULL)
    {
        BDMF_TRACE_ERR("bdmf_user_mattr_alloc failed\n");
    }

    return 0;
}

int bdmf_user_mattr_free(bdmf_ioctl_t *ba)
{
    BDMF_TRACE_DBG("ENTER bdmf_user_mattr_free");
    
    bdmf_mattr_free(ba->mattr);

    return 0;
}

int bdmf_user_get(bdmf_ioctl_t *ba)
{
    BDMF_TRACE_DBG("ENTER bdmf_user_get");
  
    bdmf_get(ba->mo_or_us);

    return 0;
}

int bdmf_user_put(bdmf_ioctl_t *ba)
{
    BDMF_TRACE_DBG("ENTER bdmf_user_put");
    
    bdmf_put(ba->mo_or_us);

    return 0;
}

int bdmf_user_get_next(bdmf_ioctl_t *ba)
{
    BDMF_TRACE_DBG("ENTER bdmf_user_get_next");
    
    ba->owner_or_ds = bdmf_get_next(ba->drv, ba->mo_or_us, ba->str);

    return 0;
}

int bdmf_user_link(bdmf_ioctl_t *ba)
{
    BDMF_TRACE_DBG("ENTER bdmf_user_link");
      
    if ((ba->ret = bdmf_link(ba->owner_or_ds, ba->mo_or_us, ba->str)) != 0)
    {
        BDMF_TRACE_ERR("bdmf_user_link failed, ret=%d\n", ba->ret);
    }

    return 0;
}

int bdmf_user_unlink(bdmf_ioctl_t *ba)
{
    BDMF_TRACE_DBG("ENTER bdmf_user_unlink");
    
    if ((ba->ret = bdmf_unlink(ba->owner_or_ds, ba->mo_or_us)) != 0)
    {
        BDMF_TRACE_ERR("bdmf_user_unlink failed, ret=%d\n", ba->ret);
    }

    return 0;
}

int bdmf_user_get_next_us_link(bdmf_ioctl_t *ba)
{
    BDMF_TRACE_DBG("ENTER bdmf_user_get_next_us_link");
    
    if ((ba->link = bdmf_get_next_us_link(ba->owner_or_ds, ba->link)) == NULL)
    {
        BDMF_TRACE_DBG("bdmf_user_get_next_us_link NULL\n");
    }

    return 0;
}

int bdmf_user_get_next_ds_link(bdmf_ioctl_t *ba)
{
    BDMF_TRACE_DBG("ENTER bdmf_user_get_next_ds_link");
    
    if ((ba->link = bdmf_get_next_ds_link(ba->mo_or_us, ba->link)) == NULL)
    {
        BDMF_TRACE_DBG("bdmf_user_get_next_ds_link NULL\n");
    }

    return 0;
}

int bdmf_user_us_link_to_object(bdmf_ioctl_t *ba)
{
    BDMF_TRACE_DBG("ENTER bdmf_user_us_link_to_object");
    
    if ((ba->mo_or_us = bdmf_us_link_to_object(ba->link)) == NULL)
    {
        BDMF_TRACE_ERR("bdmf_user_lus_link_to_object failed\n");
    }

    return 0;
}

int bdmf_user_ds_link_to_object(bdmf_ioctl_t *ba)
{
    BDMF_TRACE_DBG("ENTER bdmf_user_us_link_to_object");
    
    if ((ba->owner_or_ds = bdmf_us_link_to_object(ba->link)) == NULL)
    {
        BDMF_TRACE_ERR("bdmf_user_lus_link_to_object failed\n");
    }

    return 0;
}

int bdmf_user_get_owner(bdmf_ioctl_t *ba)
{
    BDMF_TRACE_DBG("ENTER bdmf_user_get_owner");
    
    bdmf_get_owner(ba->mo_or_us, &ba->owner_or_ds);

    return 0;
}
