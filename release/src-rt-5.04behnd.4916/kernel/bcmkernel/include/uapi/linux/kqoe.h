#ifndef _UAPI_KQOE_H
#define _UAPI_KQOE_H

#define KQOE_GENL_NAME		"kqoe"
#define KQOE_GENL_VERSION	1
#define KQOE_GENL_MC_GRP_NAME	"mcgrp"

enum kq_cmd {
	KQ_CMD_GET_CONFIG,
	KQ_CMD_SET_CONFIG,
	KQ_CMD_GET_QUEUE,
	KQ_CMD_SET_QUEUE,
	__KQ_CMD_MAX
};
#define KQ_CMD_MAX	(__KQ_CMD_MAX - 1)

enum kq_bw_attr {
	KQ_A_BW_UNSPEC,
	KQ_A_BW_MIN,
	KQ_A_BW_MAX,
	KQ_A_BW_PAD,
	__KQ_A_BW_MAX
};
#define KQ_A_BW_MAX	(__KQ_A_BW_MAX - 1)

enum kq_config_attr {
	KQ_A_CFG_UNSPEC,
	KQ_A_CFG_DIR,
	KQ_A_CFG_NUM_QUEUES,
	KQ_A_CFG_BW,
	__KQ_A_CFG_MAX
};
#define KQ_A_CFG_MAX	(__KQ_A_CFG_MAX - 1)

enum kq_queue_attr {
	KQ_A_QUEUE_UNSPEC,
	KQ_A_QUEUE_DIR,
	KQ_A_QUEUE_ID,
	KQ_A_QUEUE_BW,
	KQ_A_QUEUE_STATUS,
	__KQ_A_QUEUE_MAX
};
#define KQ_A_QUEUE_MAX	(__KQ_A_QUEUE_MAX - 1)

enum kq_dir {
	KQ_DIR_DS,
	KQ_DIR_US,
	KQ_DIR_MAX
};

enum kq_queue_status_bits {
	KQ_A_QUEUE_STATUS_ENABLED_BIT = 0,
};

enum egress_priority {
	EG_PRIO_LOW_LATENCY,
	EG_PRIO_IMPORTANT,
	EG_PRIO_NORMAL,
	EG_PRIO_BACKGROUND,

	EG_PRIO_MAX = EG_PRIO_BACKGROUND
};
#define KQ_BYPASS_QUEUE		((s16)-1)

#endif /* _UAPI_KQOE_H */
