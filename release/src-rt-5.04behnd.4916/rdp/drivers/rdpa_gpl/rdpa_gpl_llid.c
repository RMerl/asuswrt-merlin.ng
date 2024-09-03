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
 * llid object GPL shim file.
 * This file is generated automatically. Do not edit!
 */

bdmf_type_handle (*f_rdpa_llid_drv)(void);

EXPORT_SYMBOL(f_rdpa_llid_drv);

/** Get llid type handle.
 *
 * This handle should be passed to bdmf_new_and_set() function in
 * order to create a llid object.
 * \return llid type handle
 */
bdmf_type_handle rdpa_llid_drv(void)
{
   if (!f_rdpa_llid_drv)
       return NULL;
   return f_rdpa_llid_drv();
}

EXPORT_SYMBOL(rdpa_llid_drv);

int (*f_rdpa_llid_get)(bdmf_number index_, bdmf_object_handle *pmo);
EXPORT_SYMBOL(f_rdpa_llid_get);

/** Get llid object by key.

 * This function returns llid object instance by key.
 * \param[in] index_    Object key
 * \param[out] llid_obj    Object handle
 * \return    0=OK or error <0
 */
int rdpa_llid_get(bdmf_number index_, bdmf_object_handle *llid_obj)
{
   if (!f_rdpa_llid_get)
       return BDMF_ERR_STATE;
   return f_rdpa_llid_get(index_, llid_obj);
}
EXPORT_SYMBOL(rdpa_llid_get);

MODULE_LICENSE("GPL");
