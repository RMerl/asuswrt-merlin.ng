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

#ifndef _BDMF_API_H_
#define _BDMF_API_H_

#include "rdpa_user_types.h"

int bdmf_new_and_set(bdmf_type_handle drv, bdmf_object_handle owner, const bdmf_mattr_handle hmattr, bdmf_object_handle *pmo);
int bdmf_destroy(bdmf_object_handle mo);
bdmf_mattr_handle bdmf_mattr_alloc(bdmf_type_handle drv);
void bdmf_mattr_free(bdmf_mattr_handle mattr);
void bdmf_get(bdmf_object_handle mo);
void bdmf_put(bdmf_object_handle mo);
bdmf_object_handle bdmf_get_next(bdmf_type_handle drv,bdmf_object_handle mo, const char *filter);
int bdmf_link(bdmf_object_handle ds, bdmf_object_handle us, const char *attrs);
int bdmf_unlink(bdmf_object_handle ds, bdmf_object_handle us);
bdmf_link_handle bdmf_get_next_us_link(bdmf_object_handle mo, bdmf_link_handle prev);
bdmf_link_handle bdmf_get_next_ds_link(bdmf_object_handle mo, bdmf_link_handle prev);
bdmf_object_handle bdmf_us_link_to_object(bdmf_link_handle us_link);
bdmf_object_handle bdmf_ds_link_to_object(bdmf_link_handle ds_link);
void bdmf_get_owner(const bdmf_object_handle mo, bdmf_object_handle *owner);

#endif /* _BDMF_API_H_ */
