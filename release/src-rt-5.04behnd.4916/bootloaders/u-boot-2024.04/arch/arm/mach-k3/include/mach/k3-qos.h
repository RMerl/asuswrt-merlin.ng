/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Quality of Service (QoS) Configuration Header File
 *
 * Copyright (C) 2024 Texas Instruments Incorporated - https://www.ti.com/
 */
#ifndef _K3_QOS_H_
#define _K3_QOS_H_

#include <linux/kernel.h>

struct k3_qos_data {
	u32 reg;
	u32 val;
};

#if (IS_ENABLED(CONFIG_K3_QOS))
extern struct k3_qos_data qos_data[];
extern u32 qos_count;
#endif

#endif /* _K3_QOS_H_ */
