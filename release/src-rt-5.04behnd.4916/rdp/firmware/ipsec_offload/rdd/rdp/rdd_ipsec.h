/*
   Copyright (c) 2013 Broadcom Corporation
   All Rights Reserved

    <:label-BRCM:2013:DUAL/GPL:standard
    
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

#ifndef _RDD_IPSEC_H
#define _RDD_IPSEC_H


void rdd_ipsec_sa_desc_table_address(uint32_t sa_table_addr, uint16_t sa_entry_size);
BL_LILAC_RDD_ERROR_DTE rdd_ipsec_sa_desc_read(rdpa_traffic_dir dir, uint32_t index, void *val);
BL_LILAC_RDD_ERROR_DTE rdd_ipsec_sa_desc_write(rdpa_traffic_dir dir, uint32_t index, const void *val);
BL_LILAC_RDD_ERROR_DTE rdd_ipsec_sa_desc_cam_tbl_read(rdpa_traffic_dir dir, uint32_t index, void *val);


#endif /* _RDD_IPSEC_H */

