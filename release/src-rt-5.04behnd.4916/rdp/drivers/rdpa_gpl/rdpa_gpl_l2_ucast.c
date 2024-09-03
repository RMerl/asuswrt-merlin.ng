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
 * l2_ucast object GPL shim file.
 * This file is generated automatically. Do not edit!
 */

bdmf_type_handle (*f_rdpa_l2_ucast_drv)(void);

EXPORT_SYMBOL(f_rdpa_l2_ucast_drv);

/** Get l2_ucast type handle.
 *
 * This handle should be passed to bdmf_new_and_set() function in
 * order to create a l2_ucast object.
 * \return l2_ucast type handle
 */
bdmf_type_handle rdpa_l2_ucast_drv(void)
{
   if (!f_rdpa_l2_ucast_drv)
       return NULL;
   return f_rdpa_l2_ucast_drv();
}

EXPORT_SYMBOL(rdpa_l2_ucast_drv);

int (*f_rdpa_l2_ucast_get)(bdmf_object_handle *pmo);
EXPORT_SYMBOL(f_rdpa_l2_ucast_get);

/** Get l2_ucast object.

 * This function returns l2_ucast object instance.
 * \param[out] l2_ucast_obj    Object handle
 * \return    0=OK or error <0
 */
int rdpa_l2_ucast_get(bdmf_object_handle *l2_ucast_obj)
{
   if (!f_rdpa_l2_ucast_get)
       return BDMF_ERR_STATE;
   return f_rdpa_l2_ucast_get(l2_ucast_obj);
}
EXPORT_SYMBOL(rdpa_l2_ucast_get);

MODULE_LICENSE("GPL");
