/***********************************************************************
 *
 *  Copyright (c) 2010  Broadcom Corporation
 *  All Rights Reserved
 *
 * <:label-BRCM:2011:DUAL/GPL:standard
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
:>
 *
************************************************************************/

#ifndef UUID_H
#define UUID_H


/* from rfc4122.txt */
typedef struct _uuid {
    UINT32  time_low;
    UINT16  time_mid;
    UINT16  time_hi_and_version;
    UINT8   clock_seq_hi_and_reserved;
    UINT8   clock_seq_low;
    char    node[6];
} UUID;

#define UUID_LEN       16
#define MAX_ADJUSTMENT 10
#define MAXFDS	128

#endif /* UUID_H */
