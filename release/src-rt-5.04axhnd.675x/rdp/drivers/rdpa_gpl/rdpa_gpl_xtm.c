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
 * xtm object GPL shim file.
 * This file is generated automatically. Do not edit!
 */

bdmf_type_handle (*f_rdpa_xtm_drv)(void);

EXPORT_SYMBOL(f_rdpa_xtm_drv);

/** Get xtm type handle.
 *
 * This handle should be passed to bdmf_new_and_set() function in
 * order to create a xtm object.
 * \return xtm type handle
 */
bdmf_type_handle rdpa_xtm_drv(void)
{
   if (!f_rdpa_xtm_drv)
       return NULL;
   return f_rdpa_xtm_drv();
}

EXPORT_SYMBOL(rdpa_xtm_drv);

int (*f_rdpa_xtm_get)(bdmf_object_handle *pmo);
EXPORT_SYMBOL(f_rdpa_xtm_get);

/** Get xtm object.

 * This function returns xtm object instance.
 * \param[out] xtm_obj    Object handle
 * \return    0=OK or error <0
 */
int rdpa_xtm_get(bdmf_object_handle *xtm_obj)
{
   if (!f_rdpa_xtm_get)
       return BDMF_ERR_STATE;
   return f_rdpa_xtm_get(xtm_obj);
}
EXPORT_SYMBOL(rdpa_xtm_get);

bdmf_type_handle (*f_rdpa_xtmchannel_drv)(void);

EXPORT_SYMBOL(f_rdpa_xtmchannel_drv);

/** Get xtmchannel type handle.
 *
 * This handle should be passed to bdmf_new_and_set() function in
 * order to create a xtmchannel object.
 * \return xtmchannel type handle
 */
bdmf_type_handle rdpa_xtmchannel_drv(void)
{
   if (!f_rdpa_xtmchannel_drv)
       return NULL;
   return f_rdpa_xtmchannel_drv();
}

EXPORT_SYMBOL(rdpa_xtmchannel_drv);

int (*f_rdpa_xtmchannel_get)(bdmf_number index_, bdmf_object_handle *pmo);
EXPORT_SYMBOL(f_rdpa_xtmchannel_get);

/** Get xtmchannel object by key.

 * This function returns xtmchannel object instance by key.
 * \param[in] index_    Object key
 * \param[out] xtmchannel_obj    Object handle
 * \return    0=OK or error <0
 */
int rdpa_xtmchannel_get(bdmf_number index_, bdmf_object_handle *xtmchannel_obj)
{
   if (!f_rdpa_xtmchannel_get)
       return BDMF_ERR_STATE;
   return f_rdpa_xtmchannel_get(index_, xtmchannel_obj);
}
EXPORT_SYMBOL(rdpa_xtmchannel_get);

bdmf_type_handle (*f_rdpa_xtmflow_drv)(void);

EXPORT_SYMBOL(f_rdpa_xtmflow_drv);

/** Get xtmflow type handle.
 *
 * This handle should be passed to bdmf_new_and_set() function in
 * order to create a xtmflow object.
 * \return xtmflow type handle
 */
bdmf_type_handle rdpa_xtmflow_drv(void)
{
   if (!f_rdpa_xtmflow_drv)
       return NULL;
   return f_rdpa_xtmflow_drv();
}

EXPORT_SYMBOL(rdpa_xtmflow_drv);

int (*f_rdpa_xtmflow_get)(bdmf_number index_, bdmf_object_handle *pmo);
EXPORT_SYMBOL(f_rdpa_xtmflow_get);

/** Get xtmflow object by key.

 * This function returns xtmflow object instance by key.
 * \param[in] index_    Object key
 * \param[out] xtmflow_obj    Object handle
 * \return    0=OK or error <0
 */
int rdpa_xtmflow_get(bdmf_number index_, bdmf_object_handle *xtmflow_obj)
{
   if (!f_rdpa_xtmflow_get)
       return BDMF_ERR_STATE;
   return f_rdpa_xtmflow_get(index_, xtmflow_obj);
}
EXPORT_SYMBOL(rdpa_xtmflow_get);

MODULE_LICENSE("GPL");
