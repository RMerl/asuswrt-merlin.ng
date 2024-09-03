/*
<:copyright-BRCM:2018:DUAL/GPL:standard

   Copyright (c) 2018 Broadcom 
   All Rights Reserved

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License, version 2, as published by
the Free Software Foundation (the "GPL").

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.


A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.

:>
*/

#ifndef SOTP_IOCTL_H_DEFINED
#define SOTP_IOCTL_H_DEFINED

#include <bcmtypes.h>

#define SOTP_IOCTL_MAGIC       'S'

typedef enum sotpVersion
{
    SOTP_VERSION_SOTP = 1,
    SOTP_VERSION_SKP
} SOTP_VERSION;

typedef struct sotpInfoBlk
{
    uint32_t version;
    uint32_t max_rollback_lvl;
    uint32_t num_words_in_keyslot;
} SOTP_INFO_BLK;

typedef enum
{
    SOTP_ROW,
    SOTP_REGION_READLOCK,
    SOTP_REGION_FUSELOCK,
    SOTP_KEYSLOT,
    SOTP_KEYSLOT_READLOCK,
    SOTP_MAP,
    SOTP_ROLLBACK_LVL,
    SOTP_INFO,
} SOTP_ELEMENT;

typedef struct sotpIoctlParms
{
    SOTP_ELEMENT element;
    uint32_t row_addr;              /* To be used when accessing rows */ 
    uint32_t region_num;            /* To be used when locking regions */
    uint32_t keyslot_section_num;   /* To be used when accessing keyslots 
                                     * See sotp_base_defs.h for section numbers
                                     * for keyslots */
                                
    BCM_IOC_PTR(void *, inout_data);
    uint32_t data_len;
    uint32_t raw_access;
    uint32_t result;
} SOTP_IOCTL_PARMS;


#define SOTP_IOCTL_GET             _IOWR(SOTP_IOCTL_MAGIC, 0, SOTP_IOCTL_PARMS)
#define SOTP_IOCTL_SET             _IOWR(SOTP_IOCTL_MAGIC, 1, SOTP_IOCTL_PARMS)

#endif
