// <:copyright-BRCM:2013:DUAL/GPL:standard
// 
//    Copyright (c) 2013 Broadcom 
//    All Rights Reserved
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License, version 2, as published by
// the Free Software Foundation (the "GPL").
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// 
// A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
// writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
// Boston, MA 02111-1307, USA.
// 
// :>
/*
 * mcast_whitelist object GPL shim file.
 * This file is generated automatically. Do not edit!
 */

bdmf_type_handle (*f_rdpa_mcast_whitelist_drv)(void);

EXPORT_SYMBOL(f_rdpa_mcast_whitelist_drv);

/** Get mcast_whitelist type handle.
 *
 * This handle should be passed to bdmf_new_and_set() function in
 * order to create a mcast_whitelist object.
 * \return mcast_whitelist type handle
 */
bdmf_type_handle rdpa_mcast_whitelist_drv(void)
{
   if (!f_rdpa_mcast_whitelist_drv)
       return NULL;
   return f_rdpa_mcast_whitelist_drv();
}

EXPORT_SYMBOL(rdpa_mcast_whitelist_drv);

int (*f_rdpa_mcast_whitelist_get)(bdmf_object_handle *pmo);
EXPORT_SYMBOL(f_rdpa_mcast_whitelist_get);

/** Get mcast_whitelist object.

 * This function returns mcast_whitelist object instance.
 * \param[out] mcast_whitelist_obj    Object handle
 * \return    0=OK or error <0
 */
int rdpa_mcast_whitelist_get(bdmf_object_handle *mcast_whitelist_obj)
{
   if (!f_rdpa_mcast_whitelist_get)
       return BDMF_ERR_STATE;
   return f_rdpa_mcast_whitelist_get(mcast_whitelist_obj);
}
EXPORT_SYMBOL(rdpa_mcast_whitelist_get);

MODULE_LICENSE("GPL");
