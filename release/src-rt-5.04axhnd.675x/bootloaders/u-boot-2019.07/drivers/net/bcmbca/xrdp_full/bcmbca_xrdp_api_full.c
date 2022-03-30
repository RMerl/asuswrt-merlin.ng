/* SPDX-License-Identifier: GPL-2.0+
 *
 *  Copyright 2019 Broadcom Ltd.
 */

#include "data_path_init_basic.h"
#include "access_logging.h"

static int cpu_ring_and_scheduler_init(void)
{
    int i;
#if defined(CONFIG_BCM6858)
    uint32_t flow_map[] = {BB_ID_RX_BBH_4, BB_ID_RX_BBH_1, BB_ID_RX_BBH_2, BB_ID_RX_BBH_3, BB_ID_RX_BBH_0, BB_ID_RX_BBH_5, BB_ID_RX_BBH_6};
#elif defined(CONFIG_BCM6846) || defined(CONFIG_BCM6878)
    uint32_t flow_map[] = { BB_ID_RX_BBH_0, BB_ID_RX_BBH_1, BB_ID_RX_BBH_2, BB_ID_RX_BBH_3, BB_ID_RX_BBH_4 };
#elif defined(CONFIG_BCM6856) || defined(CONFIG_BCM6855)
    uint32_t flow_map[] = { BB_ID_RX_BBH_0, BB_ID_RX_BBH_1, BB_ID_RX_BBH_2, BB_ID_RX_BBH_3, BB_ID_RX_BBH_4, BB_ID_RX_BBH_5 };
#elif defined(CONFIG_BCM63146)
	uint32_t flow_map[] = {BB_ID_RX_BBH_0, BB_ID_RX_BBH_1, BB_ID_RX_BBH_2,
			       BB_ID_RX_BBH_3, BB_ID_RX_BBH_4, BB_ID_RX_BBH_5,
			       BB_ID_RX_BBH_6, BB_ID_RX_BBH_7};
#elif defined(CONFIG_BCM4912) || defined(CONFIG_BCM6813)
	uint32_t flow_map[] = {BB_ID_RX_BBH_0, BB_ID_RX_BBH_1, BB_ID_RX_BBH_2,
			       BB_ID_RX_BBH_3, BB_ID_RX_BBH_4, BB_ID_RX_BBH_5,
			       BB_ID_RX_BBH_6, BB_ID_RX_BBH_7, BB_ID_RX_BBH_8};
#endif
    uint32_t num_of_queues = sizeof(flow_map)/sizeof(flow_map[0]);

    for (i = 0; i < num_of_queues; i++) {
        /* basic scheduler configuration */
        rdd_rx_flow_cfg((256 + flow_map[i]), 0, i, 0);
    }

    return 0;
}

int xrdp_data_path_init(void)
{
    int rc;
    dpi_params_t xrdp_init_params;

    memset(&xrdp_init_params,0,sizeof(xrdp_init_params));

    ACCESS_LOG_ENABLE_SET(1);

    xrdp_init_params.mtu_size = 1518;

#ifndef CONFIG_BCMBCA_IKOS
    rc = get_rdp_freq(&xrdp_init_params.runner_freq);
    if (rc) {
        printf("Failed to get runner frequency %d\n", rc);
        goto exit;
    }
#endif

    rc = data_path_init_basic(&xrdp_init_params); 
    if (rc) {
        printf("data path init failed\n");
        goto exit;
    }

    rc = cpu_ring_and_scheduler_init();
    if (rc) {
        printf("scheduler configuration failed\n");
        goto exit;
    }

    ACCESS_LOG_ENABLE_SET(0);

exit:
    return rc;
}
