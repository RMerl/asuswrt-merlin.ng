/*
    <:copyright-BRCM:2015:DUAL/GPL:standard
    
       Copyright (c) 2015 Broadcom 
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
#include "rdp_common.h"
#include "rdp_drv_tcam.h"

/*
 * external helpers
 */

bdmf_error_t drv_tcam_mode_set(rdp_tcam_key_type key_type)
{
    return 0;
}

bdmf_error_t drv_tcam_rule_add(
    rdp_tcam_priority                       priority,
    const rdp_tcam_key_area_t              *key,
    const rdp_tcam_key_area_t              *mask,
    const rdp_tcam_context_t               *context)
{
    return 0;
}

bdmf_error_t drv_tcam_rule_delete(
    const rdp_tcam_key_area_t              *key,
    const rdp_tcam_key_area_t              *mask)
{
    return 0;
}

bdmf_error_t drv_tcam_rule_lkup(
    const rdp_tcam_key_area_t              *key,
    const rdp_tcam_key_area_t              *mask,
    uint16_t  *rule_index)
{
    return 0;
}

bdmf_error_t drv_tcam_rule_modify(
    uint16_t                               rule_index,
    rdp_tcam_context_t                     *context)
{
    return 0;
}


