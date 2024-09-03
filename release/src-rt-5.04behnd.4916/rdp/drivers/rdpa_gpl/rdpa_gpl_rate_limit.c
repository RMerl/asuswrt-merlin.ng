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
 * rate_limit object GPL shim file.
 * This file is generated automatically. Do not edit!
 */

bdmf_type_handle (*f_rdpa_rate_limit_drv)(void);

EXPORT_SYMBOL(f_rdpa_rate_limit_drv);

/** Get rate_limit type handle.
 *
 * This handle should be passed to bdmf_new_and_set() function in
 * order to create a rate_limit object.
 * \return rate_limit type handle
 */
bdmf_type_handle rdpa_rate_limit_drv(void)
{
   if (!f_rdpa_rate_limit_drv)
       return NULL;
   return f_rdpa_rate_limit_drv();
}

EXPORT_SYMBOL(rdpa_rate_limit_drv);

int (*f_rdpa_rate_limit_get)(bdmf_number index_, bdmf_object_handle *pmo);
EXPORT_SYMBOL(f_rdpa_rate_limit_get);

/** Get rate_limit object by key.

 * This function returns rate_limit object instance by key.
 * \param[in] index_    Object key
 * \param[out] rate_limit_obj    Object handle
 * \return    0=OK or error <0
 */
int rdpa_rate_limit_get(bdmf_number index_, bdmf_object_handle *rate_limit_obj)
{
   if (!f_rdpa_rate_limit_get)
       return BDMF_ERR_STATE;
   return f_rdpa_rate_limit_get(index_, rate_limit_obj);
}
EXPORT_SYMBOL(rdpa_rate_limit_get);

MODULE_LICENSE("GPL");
