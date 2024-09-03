/*
 <:copyright-BRCM:2020:DUAL/GPL:standard
 
    Copyright (c) 2020 Broadcom 
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

#ifndef _PMD_IND_H_
#define _PMD_IND_H_

typedef struct
{
    uint32_t esc_be                :1 ;
    uint32_t esc_rogue             :1 ;
    uint32_t esc_mod_over_current  :1 ;
    uint32_t esc_bias_over_current :1 ;
    uint32_t esc_mpd_fault         :1 ;
    uint32_t esc_eye_safety        :1 ;
} PMD_ALARM_INDICATION_PARAMETERS_DTE;

#endif

