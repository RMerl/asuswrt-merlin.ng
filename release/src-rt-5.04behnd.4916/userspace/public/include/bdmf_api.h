/*
<:copyright-BRCM:2019:DUAL/GPL:standard

   Copyright (c) 2019 Broadcom 
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
