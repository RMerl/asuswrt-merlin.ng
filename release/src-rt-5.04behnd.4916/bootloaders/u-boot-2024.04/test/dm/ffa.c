// SPDX-License-Identifier: GPL-2.0+
/*
 * Functional tests for UCLASS_FFA  class
 *
 * Copyright 2022-2023 Arm Limited and/or its affiliates <open-source-office@arm.com>
 *
 * Authors:
 *   Abdellatif El Khlifi <abdellatif.elkhlifi@arm.com>
 */

#include <common.h>
#include <console.h>
#include <dm.h>
#include <asm/sandbox_arm_ffa.h>
#include <asm/sandbox_arm_ffa_priv.h>
#include <dm/test.h>
#include <test/test.h>
#include <test/ut.h>

/* Functional tests for the UCLASS_FFA */

static int check_fwk_version(struct ffa_priv *uc_priv, struct unit_test_state *uts)
{
	struct ffa_sandbox_data func_data;
	u32 fwk_version = 0;

	func_data.data0 = &fwk_version;
	func_data.data0_size = sizeof(fwk_version);
	ut_assertok(sandbox_query_ffa_emul_state(FFA_VERSION, &func_data));
	ut_asserteq(uc_priv->fwk_version, fwk_version);

	return 0;
}

static int check_endpoint_id(struct ffa_priv *uc_priv, struct unit_test_state *uts)
{
	ut_asserteq(0, uc_priv->id);

	return 0;
}

static int check_rxtxbuf(struct ffa_priv *uc_priv, struct unit_test_state *uts)
{
	ut_assertnonnull(uc_priv->pair.rxbuf);
	ut_assertnonnull(uc_priv->pair.txbuf);

	return 0;
}

static int check_features(struct ffa_priv *uc_priv, struct unit_test_state *uts)
{
	ut_assert(uc_priv->pair.rxtx_min_pages == RXTX_4K ||
		  uc_priv->pair.rxtx_min_pages == RXTX_16K ||
		  uc_priv->pair.rxtx_min_pages == RXTX_64K);

	return 0;
}

static int check_rxbuf_mapped_flag(u32 queried_func_id,
				   u8 rxbuf_mapped,
				   struct unit_test_state *uts)
{
	switch (queried_func_id) {
	case FFA_RXTX_MAP:
		ut_asserteq(1, rxbuf_mapped);
		break;
	case FFA_RXTX_UNMAP:
		ut_asserteq(0, rxbuf_mapped);
		break;
	default:
		ut_assert(false);
	}

	return 0;
}

static int check_rxbuf_release_flag(u8 rxbuf_owned, struct unit_test_state *uts)
{
	ut_asserteq(0, rxbuf_owned);

	return 0;
}

static int  test_ffa_msg_send_direct_req(u16 part_id, struct unit_test_state *uts)
{
	struct ffa_send_direct_data msg;
	u8 cnt;
	struct udevice *dev;

	ut_assertok(uclass_first_device_err(UCLASS_FFA, &dev));

	ut_assertok(ffa_sync_send_receive(dev, part_id, &msg, 1));

	for (cnt = 0; cnt < sizeof(struct ffa_send_direct_data) / sizeof(u64); cnt++)
		ut_asserteq_64(-1UL, ((u64 *)&msg)[cnt]);

	return 0;
}

static int test_partitions_and_comms(const char *service_uuid,
				     struct unit_test_state *uts)
{
	struct ffa_partition_desc *descs;
	u32 count, i, j, valid_sps = 0;
	struct udevice *dev;
	struct ffa_sandbox_data func_data;
	struct ffa_partitions *partitions;

	ut_assertok(uclass_first_device_err(UCLASS_FFA, &dev));

	/* Get from the driver the count and information of the SPs matching the UUID */
	ut_assertok(ffa_partition_info_get(dev, service_uuid, &count, &descs));

	/* Make sure the count is correct */
	ut_asserteq(SANDBOX_SP_COUNT_PER_VALID_SERVICE, count);

	/* SPs found , verify the partitions information */

	func_data.data0 = &partitions;
	func_data.data0_size = sizeof(struct ffa_partitions *);
	ut_assertok(sandbox_query_ffa_emul_state(FFA_PARTITION_INFO_GET, &func_data));

	for (i = 0; i < count ; i++) {
		for (j = 0;
		     j < partitions->count;
		     j++) {
			if (descs[i].info.id ==
			   partitions->descs[j].info.id) {
				valid_sps++;
				ut_asserteq_mem(&descs[i],
						&partitions->descs[j],
						sizeof(struct ffa_partition_desc));
				/* Send and receive data from the current partition */
				test_ffa_msg_send_direct_req(descs[i].info.id, uts);
			}
		}
	}

	/* Verify expected partitions found in the emulated secure world */
	ut_asserteq(SANDBOX_SP_COUNT_PER_VALID_SERVICE, valid_sps);

	return 0;
}

static int dm_test_ffa_ack(struct unit_test_state *uts)
{
	struct ffa_priv *uc_priv;
	struct ffa_sandbox_data func_data;
	u8 rxbuf_flag = 0;
	const char *svc1_uuid = SANDBOX_SERVICE1_UUID;
	const char *svc2_uuid = SANDBOX_SERVICE2_UUID;
	struct udevice *dev;

	/* Test probing the sandbox FF-A bus */
	ut_assertok(uclass_first_device_err(UCLASS_FFA, &dev));

	/* Get a pointer to the sandbox FF-A bus private data */
	uc_priv = dev_get_uclass_priv(dev);

	/* Make sure the private data pointer is retrieved */
	ut_assertnonnull(uc_priv);

	/* Test FFA_VERSION */
	check_fwk_version(uc_priv, uts);

	/* Test FFA_ID_GET */
	check_endpoint_id(uc_priv, uts);

	/* Test FFA_FEATURES */
	check_features(uc_priv, uts);

	/*  Test RX/TX buffers */
	check_rxtxbuf(uc_priv, uts);

	/* Test FFA_RXTX_MAP */
	func_data.data0 = &rxbuf_flag;
	func_data.data0_size = sizeof(rxbuf_flag);

	rxbuf_flag = 0;
	sandbox_query_ffa_emul_state(FFA_RXTX_MAP, &func_data);
	check_rxbuf_mapped_flag(FFA_RXTX_MAP, rxbuf_flag, uts);

	/* FFA_PARTITION_INFO_GET / FFA_MSG_SEND_DIRECT_REQ */
	test_partitions_and_comms(svc1_uuid, uts);

	/* Test FFA_RX_RELEASE */
	rxbuf_flag = 1;
	sandbox_query_ffa_emul_state(FFA_RX_RELEASE, &func_data);
	check_rxbuf_release_flag(rxbuf_flag, uts);

	/* FFA_PARTITION_INFO_GET / FFA_MSG_SEND_DIRECT_REQ */
	test_partitions_and_comms(svc2_uuid, uts);

	/* Test FFA_RX_RELEASE */
	rxbuf_flag = 1;
	ut_assertok(sandbox_query_ffa_emul_state(FFA_RX_RELEASE, &func_data));
	check_rxbuf_release_flag(rxbuf_flag, uts);

	return 0;
}

DM_TEST(dm_test_ffa_ack, UT_TESTF_SCAN_FDT | UT_TESTF_CONSOLE_REC);

static int dm_test_ffa_nack(struct unit_test_state *uts)
{
	struct ffa_priv *uc_priv;
	const char *valid_svc_uuid = SANDBOX_SERVICE1_UUID;
	const char *unvalid_svc_uuid = SANDBOX_SERVICE3_UUID;
	const char *unvalid_svc_uuid_str = SANDBOX_SERVICE4_UUID;
	struct ffa_send_direct_data msg;
	int ret;
	u32 count;
	u16 part_id = 0;
	struct udevice *dev;
	struct ffa_partition_desc *descs = NULL;

	/* Test probing the sandbox FF-A bus */
	ut_assertok(uclass_first_device_err(UCLASS_FFA, &dev));

	/* Get a pointer to the sandbox FF-A bus private data */
	uc_priv = dev_get_uclass_priv(dev);

	/* Make sure the private data pointer is retrieved */
	ut_assertnonnull(uc_priv);

	/* Query partitions count using  invalid arguments */
	ret = ffa_partition_info_get(dev, NULL, NULL, NULL);
	ut_asserteq(-EINVAL, ret);
	ret = ffa_partition_info_get(dev, unvalid_svc_uuid, NULL, NULL);
	ut_asserteq(-EINVAL, ret);
	ret = ffa_partition_info_get(dev, unvalid_svc_uuid, &count, NULL);
	ut_asserteq(-EINVAL, ret);

	/* Query partitions count using an invalid UUID  string */
	ret = ffa_partition_info_get(dev, unvalid_svc_uuid_str, &count, &descs);
	ut_asserteq(-EINVAL, ret);

	/* Query partitions count using an invalid UUID (no matching SP) */
	count = 0;
	ret = ffa_partition_info_get(dev, unvalid_svc_uuid, &count, &descs);
	ut_asserteq(0, count);

	/* Query partitions data using a valid UUID */
	count = 0;
	ut_assertok(ffa_partition_info_get(dev, valid_svc_uuid, &count, &descs));
	/* Make sure partitions are detected */
	ut_asserteq(SANDBOX_SP_COUNT_PER_VALID_SERVICE, count);
	ut_assertnonnull(descs);

	/* Send data to an invalid partition */
	ret = ffa_sync_send_receive(dev, part_id, &msg, 1);
	ut_asserteq(-EINVAL, ret);

	/* Send data to a valid partition */
	part_id = uc_priv->partitions.descs[0].info.id;
	ut_assertok(ffa_sync_send_receive(dev, part_id, &msg, 1));

	return 0;
}

DM_TEST(dm_test_ffa_nack, UT_TESTF_SCAN_FDT | UT_TESTF_CONSOLE_REC);
