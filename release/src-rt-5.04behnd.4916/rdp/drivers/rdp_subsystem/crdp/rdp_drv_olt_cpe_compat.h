/*
    <:copyright-BRCM:2025:DUAL/GPL:standard
    
       Copyright (c) 2025 Broadcom 
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
#ifndef RDP_OLT_CPE_COMPAT__H
#define RDP_OLT_CPE_COMPAT__H

#define TARGET_LINUX
#define BCM_CPE_DEVICE
#include "rdp_common.h"
#include "access_macros.h"
#include "bdmf_interface.h"
#include "xrdp_drv_drivers_common_ag.h"
#include "xrdp_autogens.h"
#include "xrdp_drv_tcam_ag.h"
#include "rdp_platform.h"
#ifdef BDMF_SYSTEM_SIM
#define SIMULATION_BUILD
#endif
/* Error mapping */
#define bcmos_errno             bdmf_error_t
#define BCM_ERR_OK              BDMF_ERR_OK
#define BCM_ERR_PARM            BDMF_ERR_PARM
#define BCM_ERR_STATE           BDMF_ERR_STATE
#define BCM_ERR_OVERFLOW        BDMF_ERR_OVERFLOW
#define BCM_ERR_ALREADY         BDMF_ERR_ALREADY
#define BCM_ERR_INTERNAL        BDMF_ERR_INTERNAL
#define BCM_ERR_NOENT           BDMF_ERR_NOENT
#define BCM_ERR_IO              BDMF_ERR_IO
#define BCM_ERR_NOT_SUPPORTED   BDMF_ERR_NOT_SUPPORTED
/* Other common types mapping */
typedef char bcmos_bool;
#define BCMOS_TRUE  1
#define BCMOS_FALSE 0
#define BCMOS_ROUND_UP(n, m) (((n) + (m) - 1) & ~((m) - 1))
#define POSSIBLY_UNUSED(_v) ((void)_v)
#define BCMOS_TRACE_ERR BDMF_TRACE_ERR
#define BCMOS_TRACE_CHECK_RETURN(cond,rc,fmt,args...)    \
    do {                                        \
        if (cond)                                 \
        {\
            BDMF_TRACE_ERR( #cond ": status:%s :" fmt, bdmf_strerror(rc), ## args); \
            return rc;                              \
        }\
    } while (0)

/* CLI compatibility */
#define bcmcli_session                  bdmf_session
#define bcmcli_cmd_parm                 bdmfmon_cmd_parm_t
#define bcmcli_print                    bdmf_session_print
#define bcmcli_session_print            bdmf_session_print
#define bcmcli_parm_get                 bdmfmon_parm_get
#define bcmcli_enum_val                 bdmfmon_enum_val_t
#define bcmcli_dir_find(_dir, _name)   bdmfmon_dir_find(_dir, _name)
#define bcmcli_dir_add(_dir, _name, _help, _access, _extras) bdmfmon_dir_add(_dir, _name, _help, _access, _extras)
#define bcmcli_token_destroy(_token)    bdmfmon_token_destroy(_token)
typedef struct bdmfmon_entry            bcmcli_entry;
#define BCMCLI_MAKE_CMD                 BDMFMON_MAKE_CMD
#define BCMCLI_MAKE_CMD_NOPARM          BDMFMON_MAKE_CMD_NOPARM
#define BCMCLI_MAKE_PARM                BDMFMON_MAKE_PARM
#define BCMCLI_MAKE_PARM_RANGE          BDMFMON_MAKE_PARM_RANGE
#define BCMCLI_MAKE_PARM_DEFVAL         BDMFMON_MAKE_PARM_DEFVAL
#define BCMCLI_MAKE_PARM_ENUM           BDMFMON_MAKE_PARM_ENUM
#define BCMCLI_MAKE_PARM_ENUM_DEFVAL    BDMFMON_MAKE_PARM_ENUM_DEFVAL
#define BCMCLI_PARM_FLAG_NONE           0
#define BCMCLI_PARM_FLAG_OPTIONAL       BDMFMON_PARM_FLAG_OPTIONAL
#define BCMCLI_PARM_NUMBER              BDMFMON_PARM_NUMBER
#define BCMCLI_PARM_STRING              BDMFMON_PARM_STRING
#define BCMCLI_ACCESS_DEBUG             BDMF_ACCESS_DEBUG
#define BCMCLI_ENUM_LAST                BDMFMON_ENUM_LAST
/* Common utilities mapping */
extern int bdmf_strhex(const char *src, uint8_t *dst, uint16_t dst_len);
#define bcmolt_strhex                   bdmf_strhex
static inline void bcmcli_session_hexdump(bcmcli_session *session, uint8_t *buf, uint32_t offset, uint32_t size, const char *prefix)
{
    bdmf_session_hexdump(session, buf, offset, size);
}
#define bcmos_printf bdmf_print

#define bcmos_calloc   bdmf_calloc
#define bcmos_free     bdmf_free
#define bcmos_usleep   bdmf_usleep


/* Endian swapping */
/** Swaps the endianness of a 32-bit integer
 * \param[in] n the original number
 * \return number with swapped endianness
 */
static inline uint32_t bcmos_endian_swap_u32(uint32_t n)
{
    return ((n << 24) & 0xFF000000U)|((n << 8 ) & 0x00FF0000U)|((n >> 8) & 0x0000FF00U)|((n >> 24) & 0x000000FFU);
}
#define BCMOS_ENDIAN_CPU_TO_BIG_U32(n) (bcmos_endian_swap_u32(n))
#define BCMOS_ENDIAN_BIG_TO_CPU_U32(n) (bcmos_endian_swap_u32(n))

#endif
