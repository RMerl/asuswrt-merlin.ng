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
 * bridge object GPL shim file.
 * This file is generated automatically. Do not edit!
 */

bdmf_type_handle (*f_rdpa_bridge_drv)(void);

EXPORT_SYMBOL(f_rdpa_bridge_drv);

/** Get bridge type handle.
 *
 * This handle should be passed to bdmf_new_and_set() function in
 * order to create a bridge object.
 * \return bridge type handle
 */
bdmf_type_handle rdpa_bridge_drv(void)
{
   if (!f_rdpa_bridge_drv)
       return NULL;
   return f_rdpa_bridge_drv();
}

EXPORT_SYMBOL(rdpa_bridge_drv);

int (*f_rdpa_bridge_get)(bdmf_number index_, bdmf_object_handle *pmo);
EXPORT_SYMBOL(f_rdpa_bridge_get);

/** Get bridge object by key.

 * This function returns bridge object instance by key.
 * \param[in] index_    Object key
 * \param[out] bridge_obj    Object handle
 * \return    0=OK or error <0
 */
int rdpa_bridge_get(bdmf_number index_, bdmf_object_handle *bridge_obj)
{
   if (!f_rdpa_bridge_get)
       return BDMF_ERR_STATE;
   return f_rdpa_bridge_get(index_, bridge_obj);
}
EXPORT_SYMBOL(rdpa_bridge_get);

MODULE_LICENSE("GPL");
