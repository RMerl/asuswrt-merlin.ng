/*
   Copyright (c) 2015 Broadcom
   All Rights Reserved

    <:label-BRCM:2015:DUAL/GPL:standard
    
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



/* This is an automated file. Do not edit its contents. */


#include "rdd.h"

#include "rdd_ag_update_fifo.h"

int rdd_ag_update_fifo_update_fifo_table_set(uint32_t _entry, bdmf_boolean valid, uint16_t pd_fifo_write_ptr, uint16_t queue_number)
{
    if(_entry >= RDD_UPDATE_FIFO_TABLE_SIZE || pd_fifo_write_ptr >= 16384 || queue_number >= 512)
          return BDMF_ERR_PARM;

    RDD_UPDATE_FIFO_ENTRY_VALID_WRITE_G(valid, RDD_UPDATE_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_UPDATE_FIFO_ENTRY_PD_FIFO_WRITE_PTR_WRITE_G(pd_fifo_write_ptr, RDD_UPDATE_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_UPDATE_FIFO_ENTRY_QUEUE_NUMBER_WRITE_G(queue_number, RDD_UPDATE_FIFO_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_update_fifo_update_fifo_table_set_core(uint32_t _entry, bdmf_boolean valid, uint16_t pd_fifo_write_ptr, uint16_t queue_number, int core_id)
{
    if(_entry >= RDD_UPDATE_FIFO_TABLE_SIZE || pd_fifo_write_ptr >= 16384 || queue_number >= 512)
          return BDMF_ERR_PARM;

    RDD_UPDATE_FIFO_ENTRY_VALID_WRITE_CORE(valid, RDD_UPDATE_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_UPDATE_FIFO_ENTRY_PD_FIFO_WRITE_PTR_WRITE_CORE(pd_fifo_write_ptr, RDD_UPDATE_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_UPDATE_FIFO_ENTRY_QUEUE_NUMBER_WRITE_CORE(queue_number, RDD_UPDATE_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_update_fifo_update_fifo_table_get(uint32_t _entry, bdmf_boolean *valid, uint16_t *pd_fifo_write_ptr, uint16_t *queue_number)
{
    if(_entry >= RDD_UPDATE_FIFO_TABLE_SIZE)
          return BDMF_ERR_PARM;

    RDD_UPDATE_FIFO_ENTRY_VALID_READ_G(*valid, RDD_UPDATE_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_UPDATE_FIFO_ENTRY_PD_FIFO_WRITE_PTR_READ_G(*pd_fifo_write_ptr, RDD_UPDATE_FIFO_TABLE_ADDRESS_ARR, _entry);
    RDD_UPDATE_FIFO_ENTRY_QUEUE_NUMBER_READ_G(*queue_number, RDD_UPDATE_FIFO_TABLE_ADDRESS_ARR, _entry);

    return BDMF_ERR_OK;
}

int rdd_ag_update_fifo_update_fifo_table_get_core(uint32_t _entry, bdmf_boolean *valid, uint16_t *pd_fifo_write_ptr, uint16_t *queue_number, int core_id)
{
    if(_entry >= RDD_UPDATE_FIFO_TABLE_SIZE)
          return BDMF_ERR_PARM;

    RDD_UPDATE_FIFO_ENTRY_VALID_READ_CORE(*valid, RDD_UPDATE_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_UPDATE_FIFO_ENTRY_PD_FIFO_WRITE_PTR_READ_CORE(*pd_fifo_write_ptr, RDD_UPDATE_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);
    RDD_UPDATE_FIFO_ENTRY_QUEUE_NUMBER_READ_CORE(*queue_number, RDD_UPDATE_FIFO_TABLE_ADDRESS_ARR, _entry, core_id);

    return BDMF_ERR_OK;
}

