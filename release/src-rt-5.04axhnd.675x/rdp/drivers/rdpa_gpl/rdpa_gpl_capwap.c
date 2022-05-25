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
/*
 * capwap object GPL shim file.
 * This file is generated automatically. Do not edit!
 */

bdmf_type_handle (*f_rdpa_capwap_drv)(void);

EXPORT_SYMBOL(f_rdpa_capwap_drv);

/** Get capwap type handle.
 *
 * This handle should be passed to bdmf_new_and_set() function in
 * order to create a capwap object.
 * \return capwap type handle
 */
bdmf_type_handle rdpa_capwap_drv(void)
{
   if (!f_rdpa_capwap_drv)
       return NULL;
   return f_rdpa_capwap_drv();
}

EXPORT_SYMBOL(rdpa_capwap_drv);

int (*f_rdpa_capwap_get)(bdmf_object_handle *pmo);
EXPORT_SYMBOL(f_rdpa_capwap_get);

/** Get capwap object.

 * This function returns capwap object instance.
 * \param[out] capwap_obj    Object handle
 * \return    0=OK or error <0
 */
int rdpa_capwap_get(bdmf_object_handle *capwap_obj)
{
   if (!f_rdpa_capwap_get)
       return BDMF_ERR_STATE;
   return f_rdpa_capwap_get(capwap_obj);
}
EXPORT_SYMBOL(rdpa_capwap_get);

bdmf_type_handle (*f_rdpa_capwap_reassembly_drv)(void);

EXPORT_SYMBOL(f_rdpa_capwap_reassembly_drv);

/** Get capwap_reassembly type handle.
 *
 * This handle should be passed to bdmf_new_and_set() function in
 * order to create a capwap_reassembly object.
 * \return capwap_reassembly type handle
 */
bdmf_type_handle rdpa_capwap_reassembly_drv(void)
{
   if (!f_rdpa_capwap_reassembly_drv)
       return NULL;
   return f_rdpa_capwap_reassembly_drv();
}

EXPORT_SYMBOL(rdpa_capwap_reassembly_drv);

int (*f_rdpa_capwap_reassembly_get)(bdmf_object_handle *pmo);
EXPORT_SYMBOL(f_rdpa_capwap_reassembly_get);

/** Get capwap_reassembly object.

 * This function returns capwap_reassembly object instance.
 * \param[out] capwap_reassembly_obj    Object handle
 * \return    0=OK or error <0
 */
int rdpa_capwap_reassembly_get(bdmf_object_handle *capwap_reassembly_obj)
{
   if (!f_rdpa_capwap_reassembly_get)
       return BDMF_ERR_STATE;
   return f_rdpa_capwap_reassembly_get(capwap_reassembly_obj);
}
EXPORT_SYMBOL(rdpa_capwap_reassembly_get);

bdmf_type_handle (*f_rdpa_capwap_fragmentation_drv)(void);

EXPORT_SYMBOL(f_rdpa_capwap_fragmentation_drv);

/** Get capwap_fragmentation type handle.
 *
 * This handle should be passed to bdmf_new_and_set() function in
 * order to create a capwap_fragmentation object.
 * \return capwap_fragmentation type handle
 */
bdmf_type_handle rdpa_capwap_fragmentation_drv(void)
{
   if (!f_rdpa_capwap_fragmentation_drv)
       return NULL;
   return f_rdpa_capwap_fragmentation_drv();
}

EXPORT_SYMBOL(rdpa_capwap_fragmentation_drv);

int (*f_rdpa_capwap_fragmentation_get)(bdmf_object_handle *pmo);
EXPORT_SYMBOL(f_rdpa_capwap_fragmentation_get);

/** Get capwap_fragmentation object.

 * This function returns capwap_fragmentation object instance.
 * \param[out] capwap_fragmentation_obj    Object handle
 * \return    0=OK or error <0
 */
int rdpa_capwap_fragmentation_get(bdmf_object_handle *capwap_fragmentation_obj)
{
   if (!f_rdpa_capwap_fragmentation_get)
       return BDMF_ERR_STATE;
   return f_rdpa_capwap_fragmentation_get(capwap_fragmentation_obj);
}
EXPORT_SYMBOL(rdpa_capwap_fragmentation_get);

MODULE_LICENSE("GPL");
