/***********************************************************************
 *
 *  Copyright (c) 2021  Broadcom Corporation
 *  All Rights Reserved
 *
 * <:label-BRCM:2012:DUAL/GPL:standard
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as published by
 * the Free Software Foundation (the "GPL").
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * 
 * A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
 * writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
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

