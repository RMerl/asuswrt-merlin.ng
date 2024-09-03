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
#ifndef _PMD_OP_H_
#define _PMD_OP_H_

#include "reg/ru.h"
#include "pmd.h"

#define PMD_READ_OP 1
#define PMD_WRITE_OP 0

int pmd_op_i2c(pmd_dev_client client, uint16_t offset, unsigned char *buf, uint16_t len, int read_op);
int ru_reg_read(uint16_t addr, uint32_t * val);
int ru_reg_write(uint16_t addr, uint32_t val);
int pmd_op_sw_download(uint32_t *crc);
int pmd_op_file(pmd_dev_client client, unsigned char * buf, int len, bool read_op);
int pmd_op_get_file_len(pmd_dev_client client, uint16_t * len);
int pmd_op_temp_conv_table_download(const uint32_t *ptr);
int pmd_op_temp_apd_conv_table_download(const uint16_t *ptr);
int pmd_dump_data(void);
#endif /* _PMD_OP_H_ */
