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

#include "rdd_ag_cpu_tx.h"

int rdd_ag_cpu_tx_ddr_latency_dbg_usec_set(uint32_t bits)
{
    RDD_BYTES_4_BITS_WRITE_G(bits, RDD_DDR_LATENCY_DBG_USEC_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_cpu_tx_ddr_latency_dbg_usec_set_core(uint32_t bits, int core_id)
{
    RDD_BYTES_4_BITS_WRITE_CORE(bits, RDD_DDR_LATENCY_DBG_USEC_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_cpu_tx_ddr_latency_dbg_usec_get(uint32_t *bits)
{
    RDD_BYTES_4_BITS_READ_G(*bits, RDD_DDR_LATENCY_DBG_USEC_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_cpu_tx_ddr_latency_dbg_usec_get_core(uint32_t *bits, int core_id)
{
    RDD_BYTES_4_BITS_READ_CORE(*bits, RDD_DDR_LATENCY_DBG_USEC_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_cpu_tx_ddr_latency_dbg_usec_max_set(uint32_t bits)
{
    RDD_BYTES_4_BITS_WRITE_G(bits, RDD_DDR_LATENCY_DBG_USEC_MAX_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_cpu_tx_ddr_latency_dbg_usec_max_set_core(uint32_t bits, int core_id)
{
    RDD_BYTES_4_BITS_WRITE_CORE(bits, RDD_DDR_LATENCY_DBG_USEC_MAX_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

int rdd_ag_cpu_tx_ddr_latency_dbg_usec_max_get(uint32_t *bits)
{
    RDD_BYTES_4_BITS_READ_G(*bits, RDD_DDR_LATENCY_DBG_USEC_MAX_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ag_cpu_tx_ddr_latency_dbg_usec_max_get_core(uint32_t *bits, int core_id)
{
    RDD_BYTES_4_BITS_READ_CORE(*bits, RDD_DDR_LATENCY_DBG_USEC_MAX_ADDRESS_ARR, 0, core_id);

    return BDMF_ERR_OK;
}

