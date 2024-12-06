/***********************************************************************
 *
 *  Copyright (c) 2021  Broadcom Corporation
 *  All Rights Reserved
 *
 * <:label-BRCM:2012:DUAL/GPL:standard
 * 
 * Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2
 * (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
 * with the following added to such license:
 * 
 *    As a special exception, the copyright holders of this software give
 *    you permission to link this software with independent modules, and
 *    to copy and distribute the resulting executable under terms of your
 *    choice, provided that you also meet, for each linked independent
 *    module, the terms and conditions of the license of that module.
 *    An independent module is a module which is not derived from this
 *    software.  The special exception does not apply to any modifications
 *    of the software.
 * 
 * Not withstanding the above, under no circumstances may you combine
 * this software in any way with any other Broadcom software provided
 * under a license other than the GPL, without Broadcom's express prior
 * written consent.
 * 
 * :>
 *
 ************************************************************************/

#include "xflow_macsec_defs.h"
#include "xflow_macsec.h"
#include "xflow_macsec_esw_defs.h"
#include "xflow_macsec_common.h"
#include "xflow_macsec_firelight.h"
#include "macsec_dev.h"

#ifndef MACSEC_DEBUG_H
#define MACSEC_DEBUG_H

#define DEBUG_LEVEL_ALWAYS              0x00
#define DEBUG_LEVEL_ERROR               0x01
#define DEBUG_LEVEL_WARN                0x02
#define DEBUG_LEVEL_INFO                0x04
#define DEBUG_LEVEL_VERBOSE             0x08
#define DEBUG_LEVEL_FILESYS             0x10
#define DEBUG_LEVEL_SOCKS               0x20
#define DEBUG_LEVEL_ALL                 0x2f

#define DEBUG_LEVEL_ERROR_STR           "ERROR"
#define DEBUG_LEVEL_WARN_STR            "WARN"
#define DEBUG_LEVEL_INFO_STR            "INFO"
#define DEBUG_LEVEL_VERBOSE_STR         "VERBOSE"
#define DEBUG_LEVEL_FILESYS_STR         "FILESYS"
#define DEBUG_LEVEL_SOCKS_STR           "SOCKS"
#define DEBUG_LEVEL_ALL_STR             "ALL"

int xflow_macsec_debug_mode_handler(macsec_dev_t *mdev, int type, char *name, int mem_idx);

void debug_dump_table(macsec_dev_t *mdev, unsigned char *mem_name, int index);

#endif

