/*
<:copyright-BRCM:2019:DUAL/GPL:standard

   Copyright (c) 2019 Broadcom 
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

#include <sys/ioctl.h>
#include <stdint.h>
#include "bdmf_user_interface.h"

static int call_ioctl(bdmf_ioctl_t *ba, int cmd)
{
    int fd, ret = 0;

    fd = open(RDPA_USR_DEV_NAME, O_RDWR);
    if (fd < 0)
    {
        rdpa_usr_error("%s: %s\n", RDPA_USR_DEV_NAME, strerror(errno));
        return -EINVAL;
    }

    ret = ioctl(fd, cmd, (uintptr_t)ba);
    if (ret)
        rdpa_usr_error("ioctl failed, ret=%d\n", ret);

    close(fd);
    return ret;             
}

int bdmf_new_and_set(bdmf_type_handle drv,
                    bdmf_object_handle owner, 
                    const bdmf_mattr_handle hmattr,
                    bdmf_object_handle *pmo)
{
    bdmf_ioctl_t ba;
    int ret = 0;

    memset(&ba, 0x0, sizeof(bdmf_ioctl_t));
    ba.drv = drv;
    ba.owner_or_ds = owner;
    ba.mattr = hmattr;

    ret = call_ioctl(&ba, BDMF_NEW_AND_SET);
    if (ret)
        return -EINVAL;

    *pmo = ba.mo_or_us;
    return ba.ret;
}

int bdmf_destroy(bdmf_object_handle mo)
{
    bdmf_ioctl_t ba;
    int ret = 0;
    
    memset(&ba, 0x0, sizeof(bdmf_ioctl_t));
    ba.mo_or_us = mo;
    
    ret = call_ioctl(&ba, BDMF_DESTROY);
    if (ret)
        return -EINVAL;    
            
    return ba.ret;
}

bdmf_mattr_handle bdmf_mattr_alloc(bdmf_type_handle drv)
{
    bdmf_ioctl_t ba;
    int ret = 0;

    memset(&ba, 0x0, sizeof(bdmf_ioctl_t));
    ba.drv = drv;

    ret = call_ioctl(&ba, BDMF_MATTR_ALLOC);
    if (ret)
        return 0; 
        
    return ba.mattr; 
}

void bdmf_mattr_free(bdmf_mattr_handle mattr)
{
    bdmf_ioctl_t ba;

    memset(&ba, 0x0, sizeof(bdmf_ioctl_t));
    ba.mattr = mattr;
    
    call_ioctl(&ba, BDMF_MATTR_FREE);
}

void bdmf_get(bdmf_object_handle mo)
{
    bdmf_ioctl_t ba;

    memset(&ba, 0x0, sizeof(bdmf_ioctl_t));
    ba.mo_or_us = mo;
    
    call_ioctl(&ba, BDMF_GET);
}

void bdmf_put(bdmf_object_handle mo)
{
    bdmf_ioctl_t ba;

    memset(&ba, 0x0, sizeof(bdmf_ioctl_t));
    ba.mo_or_us = mo;
    
    call_ioctl(&ba, BDMF_PUT);
}

bdmf_object_handle bdmf_get_next(bdmf_type_handle drv,
                                 bdmf_object_handle mo, const char *filter)
{
    bdmf_ioctl_t ba;
    int ret = 0;

    memset(&ba, 0x0, sizeof(bdmf_ioctl_t));
    ba.drv = drv;
    ba.mo_or_us = mo;
    ba.str = filter;
    

    ret = call_ioctl(&ba, BDMF_GET_NEXT);
    if (ret)
        return 0;

    return ba.owner_or_ds;
}

int bdmf_link(bdmf_object_handle ds,
              bdmf_object_handle us, const char *attrs)
{
    bdmf_ioctl_t ba;
    int ret = 0;

    memset(&ba, 0x0, sizeof(bdmf_ioctl_t));
    ba.owner_or_ds = ds;
    ba.mo_or_us = us;
    ba.str = attrs;
    
    ret = call_ioctl(&ba, BDMF_LINK);
    if (ret)
        return -EINVAL; 

    return ba.ret;
}

int bdmf_unlink(bdmf_object_handle ds, bdmf_object_handle us)
{
    bdmf_ioctl_t ba;
    int ret = 0;

    memset(&ba, 0x0, sizeof(bdmf_ioctl_t));
    ba.owner_or_ds = ds;
    ba.mo_or_us = us;

    ret = call_ioctl(&ba, BDMF_UNLINK);
    if (ret)
        return -EINVAL;     
    
    return ba.ret;
}


bdmf_link_handle bdmf_get_next_us_link(bdmf_object_handle mo,
                                          bdmf_link_handle prev)
{
    bdmf_ioctl_t ba;
    int ret = 0;

    memset(&ba, 0x0, sizeof(bdmf_ioctl_t));
    ba.owner_or_ds = mo;
    ba.link = prev;
    
    ret = call_ioctl(&ba, BDMF_GET_NEXT_US_LINK);
    if (ret)
        return 0;

    return ba.link;
}

bdmf_link_handle bdmf_get_next_ds_link(bdmf_object_handle mo,
                                        bdmf_link_handle prev)
{
    bdmf_ioctl_t ba;
    int ret = 0;

    memset(&ba, 0x0, sizeof(bdmf_ioctl_t));
    ba.mo_or_us = mo;
    ba.link = prev;
    
    ret = call_ioctl(&ba, BDMF_GET_NEXT_DS_LINK);
    if (ret)
        return 0;

    return ba.link;
}

bdmf_object_handle bdmf_us_link_to_object(bdmf_link_handle us_link)
{
    bdmf_ioctl_t ba;
    int ret = 0;

    memset(&ba, 0x0, sizeof(bdmf_ioctl_t));
    ba.link = us_link;
    
    ret = call_ioctl(&ba, BDMF_US_LINK_TO_OBJECT);
    if (ret)
        return 0;     
    
    return ba.mo_or_us;
}

bdmf_object_handle bdmf_ds_link_to_object(bdmf_link_handle ds_link)
{
    bdmf_ioctl_t ba;
    int ret = 0;

    memset(&ba, 0x0, sizeof(bdmf_ioctl_t));
    ba.link = ds_link;
    
    ret = call_ioctl(&ba, BDMF_DS_LINK_TO_OBJECT);
    if (ret)
        return 0;     
    
    return ba.owner_or_ds;
}

void bdmf_get_owner(const bdmf_object_handle mo,
                    bdmf_object_handle *owner)
{
    bdmf_ioctl_t ba;

    memset(&ba, 0x0, sizeof(bdmf_ioctl_t));
    ba.mo_or_us = mo;

    call_ioctl(&ba, BDMF_GET_OWNER);
    *owner = ba.owner_or_ds;
}
