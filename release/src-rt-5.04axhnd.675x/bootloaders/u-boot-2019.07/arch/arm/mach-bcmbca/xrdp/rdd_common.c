// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2014 Broadcom
 */
/*
    
*/

#include "rdd.h"
#include "rdd_runner_proj_defs.h"

#define LAYER2_HEADER_COPY_DST_OFFSET_ARRAY(var, offset) \
	uint16_t var[] = { \
		[0] = offset - 2,   \
		[1] = offset - 10,  \
		[2] = offset - 10,  \
		[3] = offset - 18,  \
		[4] = offset - 18,  \
		[5] = offset - 2,   \
		[6] = offset - 2,   \
		[7] = offset - 10,  \
		[8] = offset - 10,  \
		[9] = offset - 18,  \
		[10] = offset + 6,  \
		[11] = offset - 2,  \
		[12] = offset - 2,  \
		[13] = offset - 10, \
		[14] = offset - 10, \
		[15] = offset + 6,  \
		[16] = offset + 6,  \
		[17] = offset - 2,  \
		[18] = offset - 2,  \
		[19] = offset - 10, \
		[20] = offset + 14, \
		[21] = offset + 6,  \
		[22] = offset + 6,  \
		[23] = offset - 2,  \
		[24] = offset - 2,  \
		[25] = offset + 14, \
		[26] = offset + 14, \
		[27] = offset + 6,  \
		[28] = offset + 6,  \
		[29] = offset - 2,  \
	}


static void __rdd_rx_flow_cfg(uint32_t flow_index, rdd_flow_dest destination,
			      rdd_rdd_vport vport, uint32_t counter_id)
{
	RDD_RX_FLOW_ENTRY_VIRTUAL_PORT_WRITE_G(vport,
				RDD_RX_FLOW_TABLE_ADDRESS_ARR, flow_index);
}

void rdd_rx_flow_cfg(uint32_t flow_index, rdd_flow_dest destination,
		     rdd_rdd_vport vport, uint32_t counter_id)
{

	RDD_BTRACE("flow_index = %d, destination = %d, vport = %d, counter_id "
		   "= %d, first_time %d\n", flow_index,	destination, vport,
		   counter_id, first_time);

	__rdd_rx_flow_cfg(flow_index, destination, vport, counter_id);
}

