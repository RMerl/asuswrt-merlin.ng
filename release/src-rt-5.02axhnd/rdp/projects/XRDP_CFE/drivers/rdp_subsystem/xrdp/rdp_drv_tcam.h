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

#ifndef _RDP_DRV_TCAM_H
#define _RDP_DRV_TCAM_H

#include "access_macros.h"
#include "bdmf_interface.h"
#include "rdp_common.h"
#include "xrdp_drv_drivers_common_ag.h"
#include "xrdp_drv_tcam_ag.h"
#include "rdp_platform.h"

#define RDP_TCAM_CONTEXT_SIZE           4

/* Total number of entries in all TCAM tables */
#define RDP_TCAM_ALL_TABLE_SIZE         (RDP_TCAM_TABLE_SIZE * RDP_TCAM_NUM_ENGINES)

/* Max TCAM key size in bytes */
#define RDP_TCAM_MAX_KEY_SIZE           64

/* Max number of TCAM priorities.
 * TCAM rules are sorted in order of priorities.
 * Lesser number means higher priority
 */
#define RDP_TCAM_MAX_PRIORITIES         64

/* TCAM key size */
typedef enum
{
    RDP_TCAM_KEY_256,   /**< 256 bit key */
    RDP_TCAM_KEY_512    /**< 512 bit key */
} rdp_tcam_key_type;

/* TCAM rule priority. The value range is 0..RDP_TCAM_MAX_PRIORITIES-1 */
typedef uint8_t rdp_tcam_priority;

/* TCAM key / mask area */
typedef union rdp_tcam_key_area
{
    uint8_t b[RDP_TCAM_MAX_KEY_SIZE];
    uint32_t w[RDP_TCAM_MAX_KEY_SIZE / 4];
} rdp_tcam_key_area_t;

/* TCAM context */
typedef struct rdp_tcam_context
{
    uint32_t word[RDP_TCAM_CONTEXT_SIZE];   /**< Context words in CPU byte order */
} rdp_tcam_context_t;

/* TCAM commands */
typedef enum
{
    TCAM_CMD_READ       = 0,
    TCAM_CMD_WRITE      = 1,
    TCAM_CMD_COMPARE    = 2,
    TCAM_CMD_INVALIDATE = 3,
} tcam_cmd;

/* Set tcam mode
 * \param[in]   key_type             Key size
 * \returns: 0=0 or error < 0
 */
bdmf_error_t drv_tcam_mode_set(rdp_tcam_key_type key_type);

/* Get tcam mode
 * \param[out]  key_type             Key size
 * \returns: 0=0 or error < 0
 */
bdmf_error_t drv_tcam_mode_get(rdp_tcam_key_type *key_type);

/* Get TCAM key size
 * \param[out]  key_size             Key size (bytes)
 * \returns: 0=0 or error < 0
 */
static inline bdmf_error_t drv_tcam_keysize_get(uint32_t *key_size)
{
    rdp_tcam_key_type key_type;
    bdmf_error_t err = drv_tcam_mode_get(&key_type);
    if (err)
        return err;
    *key_size = (key_type == RDP_TCAM_KEY_256) ? 256 / 8 : 512 / 8;
    return BDMF_ERR_OK;
}

/* Add classification rule
 *
 * The function inserts new entry into the correct place in TCAM table(s)
 * based on the entry's priority.
 * The tables are updated safely. Firmware can perform searches while
 * the tables are being updated.
 *
 * The function must be called under lock!
 *
 * \param[in]   priority             Rule priority
 * \param[in]   key                  Key area
 * \param[in]   mask                 Mask area
 * \param[in]   context              Rule context
 * \returns: 0=0 or error < 0
 */
bdmf_error_t drv_tcam_rule_add(
    rdp_tcam_priority                       priority,
    const rdp_tcam_key_area_t              *key,
    const rdp_tcam_key_area_t              *mask,
    const rdp_tcam_context_t               *context);

/* Modify classification context
 *
 * The function inserts new context into the correct place in TCAM table(s)
 * based on the entry's index.
 *
 * The function must be called under lock!
 *
 * \param[in]   rule_index           Index in TCAM table
 * \param[in]   context              Rule context
 * \returns: 0=0 or error < 0
 */
bdmf_error_t drv_tcam_rule_modify(
    uint16_t                               rule_index,
    rdp_tcam_context_t                     *context);

/* Delete classification rule
 *
 * The function deletes existing entry from TCAM table(s).
 * The tables are updated safely. Firmware can perform searches while
 * the tables are being updated.
 *
 * The function must be called under lock!
 *
 * \param[in]   key                  Key area
 * \param[in]   mask                 Mask area
 * \returns: 0=0 or error < 0
 */
bdmf_error_t drv_tcam_rule_delete(
    const rdp_tcam_key_area_t              *key,
    const rdp_tcam_key_area_t              *mask);

/* Find classification rule
 *
 * The function must be called under lock!
 *
 * \param[in]   key                  Key area
 * \param[in]   rule_index           Index in TCAM table
 * \param[in]   context              Rule context
 * \returns: 0=0 or error < 0
 */
bdmf_error_t drv_tcam_rule_find(
    const rdp_tcam_key_area_t              *key,
    uint16_t                               *rule_index,
    rdp_tcam_context_t                     *context);

/* Find TCAM rule and return index
 *
 * \param[in]   key                  Key area
 * \param[in]   mask                 Mask area
 * \param[in]   rule_index           Index in TCAM table
 * \returns: 0=0 or error < 0
 */
bdmf_error_t drv_tcam_rule_lkup(
    const rdp_tcam_key_area_t              *key,
    const rdp_tcam_key_area_t              *mask,
    uint16_t  *rule_index);

/*
 * CLI support
 */
#ifdef USE_BDMF_SHELL
void drv_tcam_cli_init(bdmfmon_handle_t driver_dir);
void drv_tcam_cli_exit(bdmfmon_handle_t driver_dir);
bdmf_error_t drv_tcam_mem_dump(bdmf_session_handle session, const char *filename);
#endif

#endif /* _RDP_DRV_TCAM_H */

