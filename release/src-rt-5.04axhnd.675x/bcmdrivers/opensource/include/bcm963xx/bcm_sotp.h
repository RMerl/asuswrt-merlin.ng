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

#define SOTP_NUM_WORDS_IN_KEYSLOT 8
#define SOTP_NUM_ROWS_IN_KEYSLOT 8
#define SOTP_NUM_DATABITS_IN_KEYSLOT (SOTP_NUM_WORDS_IN_KEYSLOT*4*8)
typedef enum
{
    SOTP_ROW,
    SOTP_REGION_READLOCK,
    SOTP_REGION_FUSELOCK,
    SOTP_KEYSLOT,
    SOTP_KEYSLOT_READLOCK,
    SOTP_MAP,
    SOTP_ROLLBACK_LVL,
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

//* Rollback lvl section - Using general cfg sections */
#define    STOP_FIRST_ROLLBACK_ROW              16 /* First row used for anti-rollback */
#define    SOTP_FIRST_ROLLBACK_SECTION		4 /* First section used for anti-rollback */
#define    SOTP_LAST_ROLLBACK_SECTION		4 /* Last section used for anti-rollback */
#define    SOTP_NUM_REGIONS_IN_ROLLBACK_SECTION	1 /* Number of regions in each anti-rollback section */

#define    SOTP_LAST_ROLLBACK_ROW               (STOP_FIRST_ROLLBACK_ROW + \
                                                (SOTP_LAST_ROLLBACK_SECTION - \
						 SOTP_FIRST_ROLLBACK_SECTION+1) * \
						 SOTP_NUM_REGIONS_IN_ROLLBACK_SECTION * \
						 SOTP_ROWS_IN_REGION -1)
#define    SOTP_MAX_ROLLBACK_LVLS_PER_ROW	16 /* Number of rollback levels in a 32bit SOTP row */
#define	   SOTP_ROLLBACK_LVL_MASK		0x3 /* Mask for rollback level */
#define    SOTP_ROLLBACK_LVL_SHIFT		2 /* Shift index for rollback level */	
#define    SOTP_MAX_ROLLBACK_ROW_VALUE		0xffffffff
#define    SOTP_MAX_ROLLBACK_LVL		((SOTP_LAST_ROLLBACK_SECTION - SOTP_FIRST_ROLLBACK_SECTION + 1 ) \
						 * SOTP_NUM_REGIONS_IN_ROLLBACK_SECTION * 4 * SOTP_MAX_ROLLBACK_LVLS_PER_ROW)

#endif
