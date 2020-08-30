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

#ifndef DRV_FPM_H_INCLUDED
#define DRV_FPM_H_INCLUDED

#ifdef __cplusplus
extern "C"
{
#endif

/*****************************************************************************/
/*                                                                           */
/* Include files                                                             */
/*                                                                           */
/*****************************************************************************/
#include "xrdp_drv_fpm_ag.h"

#define FPM_BUF_SIZE_0                      512
#define FPM_BUF_SIZE_1                      256
#define FPM_BUF_SIZE_DEFAULT                FPM_BUF_SIZE_0

#define FPM_BUF_SIZE_256                    FPM_BUF_SIZE_1
#define FPM_BUF_SIZE_512                    FPM_BUF_SIZE_0
#define FPM_BUF_SIZE_1K                     1024
#define FPM_BUF_SIZE_2K                     2048
#define FPM_BUF_MAX_BASE_BUFFS              8
#define TOTAL_FPM_TOKENS                    (64*1024)

/* All following definitions must be 100*k since they are percent-wise divided */
#define TOTAL_DYNAMIC_FPM                   61000           /* ( 2 (FPM per DQM) * ( 287 (no. of queues) + 16 (prefetch) ) + 
                                                               ( 64K (maximal number of PDs in Queue) / (2K / 16) number of PDs in FPM) ) * 4(2K allocation )
                                                               = 4472 FPMs (basic size of 512). Worst case DQM FPM allocation.  */
#define FPM_EXTRA_TOKENS_FOR_DQM            (4 * 1024)


void drv_fpm_init(void *virt_base, unsigned int fpm_buf_size);

bdmf_error_t drv_fpm_alloc_buffer(uint32_t packet_len, uint32_t *buff_num);
bdmf_error_t drv_fpm_free_buffer(uint32_t packet_len, uint32_t buff_num);
void drv_fpm_copy_from_host_buffer(void *data, uint32_t fpm_bn, uint32_t packet_len, uint16_t offset);
void *drv_fpm_buffer_get_address(uint32_t fpm_bn);

#ifdef USE_BDMF_SHELL
int drv_fpm_cli_debug_get(bdmf_session_handle session, bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
int drv_fpm_cli_sanity_check(bdmf_session_handle session, bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
int drv_fpm_cli_config_get(bdmf_session_handle session, bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
void drv_fpm_cli_init(bdmfmon_handle_t driver_dir);
void drv_fpm_cli_exit(bdmfmon_handle_t driver_dir);
#endif

#ifdef __cplusplus
}
#endif

#endif
