/*
   Copyright (c) 2013 Broadcom Corporation
   All Rights Reserved

    <:label-BRCM:2013:DUAL/GPL:standard

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
