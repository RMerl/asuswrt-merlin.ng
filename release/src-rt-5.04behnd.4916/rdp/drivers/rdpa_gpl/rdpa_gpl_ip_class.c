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
 * ip_class object GPL shim file.
 * This file is generated automatically. Do not edit!
 */

bdmf_type_handle (*f_rdpa_ip_class_drv)(void);

EXPORT_SYMBOL(f_rdpa_ip_class_drv);

/** Get ip_class type handle.
 *
 * This handle should be passed to bdmf_new_and_set() function in
 * order to create an ip_class object.
 * \return ip_class type handle
 */
bdmf_type_handle rdpa_ip_class_drv(void)
{
   if (!f_rdpa_ip_class_drv)
       return NULL;
   return f_rdpa_ip_class_drv();
}

EXPORT_SYMBOL(rdpa_ip_class_drv);

int (*f_rdpa_ip_class_get)(bdmf_object_handle *pmo);
EXPORT_SYMBOL(f_rdpa_ip_class_get);

/** Get ip_class object.

 * This function returns ip_class object instance.
 * \param[out] ip_class_obj    Object handle
 * \return    0=OK or error <0
 */
int rdpa_ip_class_get(bdmf_object_handle *ip_class_obj)
{
   if (!f_rdpa_ip_class_get)
       return BDMF_ERR_STATE;
   return f_rdpa_ip_class_get(ip_class_obj);
}
EXPORT_SYMBOL(rdpa_ip_class_get);

MODULE_LICENSE("GPL");
