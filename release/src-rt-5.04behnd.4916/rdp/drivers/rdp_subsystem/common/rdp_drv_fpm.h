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
#include "bcm_rdp_arch.h"

/* default values for FPM */
#define FPM_POOL_BASE_ADDRESS               0
#define FPM_CLEAR_IRQ_STATUS                0xffffffff
#define FPM_DDR_0_POOL_ID                   0
#define FPM_ALLOC_WEIGHT                    1
#define FPM_FREE_WEIGHT                     1
#define FPM_POLL_SLEEP_TIME                 200

#if CHIP_VER >= RDP_GEN_62
#define FPM_MAX_PREFETCH_TOKEN (32*32) /*take worst case( 32*32 contain) 20 * 32 + 8 * 32 + 2 * 32 + 1 * 21, all small ones in the holes of 20 * 32 */
#else
#define FPM_MAX_PREFETCH_TOKEN (16*32) /*take worst case( 16*32 contain) */
#endif
#define FPM_MIN_POOL_SIZE 3


#if defined(RDP_ARCH_SIM) || defined(RDP_ARCH_QEMU_SIM) || defined(CONFIG_BRCM_QEMU_NO_INDIRECT)
#define FPM_INIT_TIMEOUT                    0
#define FPM_INIT_LOOP for (fpmini_init = 0; fpmini_init;)
#elif defined(RDP_ARCH_BOARD) || defined (RDP_ARCH_BOOT)
#if defined(BCM6856)
#define FPM_INIT_TIMEOUT                    0x7800
#else
#define FPM_INIT_TIMEOUT                    0xf000
#endif
#define FPM_INIT_LOOP for (fpmini_init = 1; fpmini_init; )
#else
#error "no arch defined"
#endif /* RDP_ARCH */

#define FPM_INTERRUPT_STATUS                {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
#define FPM_INTERRUPT_MASK                  {0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 0}


#define FPM_INTERRUPT_MASK_OFF              {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
#define FPM_INTERRUPT_TIMER_DELAY           1000  /* msec */


#define FPM_BUF_MAX_BASE_BUFFS              8


#define FPM_XON_THRESHOLD                   (TOTAL_FPM_TOKENS - TOTAL_DYNAMIC_FPM)
#define FPM_XOFF_THRESHOLD                  (7*FPM_XON_THRESHOLD/8)
#define FPM_EXTRA_TOKENS_FOR_DQM            (4 * 1024)

int drv_fpm_init(dpi_params_t *p_dpi_cfg);

void drv_fpm_select_pools(int *pool_size, uint32_t pool_set);
int drv_fpm_pool0_encode(int tokens);
int drv_fpm_pool1_encode(int tokens);

bdmf_error_t drv_fpm_alloc_buffer(uint32_t packet_len, uint32_t *buff_num);
bdmf_error_t drv_fpm_free_buffer(uint32_t packet_len, uint32_t buff_num, void *info_in);
extern int drv_xpm_ddr_token_info_clear(uint32_t token_idx);

bdmf_error_t drv_fpm_check_xoff(uint32_t num_of_token);

#if CHIP_VER < RDP_GEN_60
int drv_fpm_data_path_init(dpi_params_t *p_dpi_cfg, fpm_pool_cfg *pool_cfg, int extra_dqm_tokens_set);
uint32_t drv_fpm_dqm_extra_fpm_tokens_get(void);
#endif

#ifdef DQM_MINI_FPM_MODE
#define FPM_FPMINI_2K_TOKENS    0xFFFFFFFC
#define FPM_FPMINI_4K_TOKENS    0xFFFFFFF0
#define FPM_FPMINI_8K_TOKENS    0xFFFFFF00
#define FPM_FPMINI_16K_TOKENS   0xFFFF0000
void drv_fpm_data_path_mini_fpm_init(uint32_t fpm_mini_tokens);
#endif

#ifdef USE_BDMF_SHELL
int drv_fpm_cli_debug_get(bdmf_session_handle session, bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
int drv_fpm_cli_sanity_check(bdmf_session_handle session, bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
int drv_fpm_cli_config_get(bdmf_session_handle session, bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
void drv_fpm_cli_init(bdmfmon_handle_t driver_dir);
void drv_fpm_cli_exit(bdmfmon_handle_t driver_dir);
#endif

#if (CHIP_VER >= RDP_GEN_100)
#define POOL1_STAT1 STAT1
#define POOL1_STAT2 STAT2
#define POOL1_STAT3 STAT3
#define POOL1_STAT4 STAT4
#define POOL1_STAT5 STAT5
#define POOL1_STAT6 STAT6
#define POOL1_STAT7 STAT7
#define POOL1_STAT8 STAT8
#define POOL1_INTR_STS INTR_STS
#define ag_drv_fpm_pool1_xon_xoff_cfg_get ag_drv_fpm_xon_xoff_cfg_get
#endif
#ifdef __cplusplus
}
#endif

#endif
