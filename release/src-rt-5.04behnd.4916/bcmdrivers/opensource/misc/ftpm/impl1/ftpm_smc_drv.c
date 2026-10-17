/*
<:copyright-BRCM:2019:DUAL/GPL:standard

   Copyright (c) 2019 Broadcom
   All Rights Reserved

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

#include <linux/acpi.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <linux/platform_device.h>
#include <linux/tee_drv.h>
#include <linux/tpm.h>
#include <linux/uuid.h>
#include <drivers/char/tpm/tpm.h>
#include <itc_rpc.h>

#include "ftpm_smc_drv.h"

static int rpc_async_complete(int tunnel, rpc_msg *msg)
{
	(void) tunnel;
	(void) msg;

	return 0;
}

static rpc_function ftpm_smc_funcs_tbl[] = {
	[FTPM_SMC_SUBMIT_COMMAND] = { rpc_async_complete, 0 },
};

static int get_rpc_tunnel_id(struct platform_device *pdev)
{
	int rc;
	struct ftpm_smc_private *pvt_data = dev_get_drvdata(&pdev->dev);
    struct device_node *of_node = pdev->dev.of_node;
	struct device_node *phan_node = 0;
    const char *dev_name;

    phan_node = of_parse_phandle(of_node, "rpc-channel", 0);
    if (!phan_node) {
        dev_err(&pdev->dev, "of_parse_phandle failed to retrieve rpc-channel phandle\n");
        rc = (-EINVAL);
		goto err;
    }

	rc = of_property_read_string(phan_node, "dev-name", &dev_name);
    if (rc) {
        dev_err(&pdev->dev, "%s: of_property_read_string failed to retrieve dev-name property\n", of_node_full_name(of_node));
		goto err;
    }
    of_node_put(phan_node);

	pvt_data->tunnel_id = rpc_get_fifo_tunnel_id((char *)dev_name);
	if (RPC_INVALID_TUNNEL == pvt_data->tunnel_id) {
		dev_err(&pdev->dev, "%s: rpc_get_fifo_tunnel_id failed\n", __func__);
		rc = (-ENXIO);
		goto err;
	}

	return 0;

err:
	if (phan_node) of_node_put(phan_node);

    return rc;
}

static int ftpm_smc_tpm_op_recv(struct tpm_chip *chip, u8 *buf, size_t count)
{
	struct device *dev = &chip->dev;
	struct ftpm_smc_private *pvt_data = dev_get_drvdata(chip->dev.parent);
	size_t len;

	len = pvt_data->resp_len;
	if (count < len) {
		dev_err(dev, "%s: invalid size in recv: count=%zd, resp_len=%zd\n", __func__, count, len);
		return (-EIO);
	}

	memcpy(buf, pvt_data->resp_buf, len);
	pvt_data->resp_len = 0;

	return len;
}

static int ftpm_smc_tpm_op_send(struct tpm_chip *chip, u8 *buf, size_t len)
{
	int rc;
	struct device *dev = &chip->dev;
	struct ftpm_smc_private *pvt_data = dev_get_drvdata(chip->dev.parent);
	struct ftpm_smc_rpc_msg msg;

	if (len > MAX_COMMAND_SIZE) {
		dev_err(dev, "%s: len=%zd exceeds MAX_COMMAND_SIZE supported by fTPM TA\n", __func__, len);
		rc = (-EIO);
		goto err;
	}

	memcpy(pvt_data->shm_buf, buf, len);

	rpc_msg_init((rpc_msg *)&msg, RPC_SERVICE_FTPM, FTPM_SMC_SUBMIT_COMMAND, 0, 0, 0, 0);

	msg.body.request.address_low = (uint32_t)(pvt_data->shm_buf_dma_addr & 0xffffffff);
	msg.body.request.address_high = (uint32_t)(pvt_data->shm_buf_dma_addr >> 32);
	msg.body.request.length = len;

	rpc_dma_sync_single_for_device(pvt_data->shm_buf_dma_addr, MAX_TRANSACTION_SIZE, DMA_TO_DEVICE);

	rc = rpc_send_request_timeout(pvt_data->tunnel_id, (rpc_msg *)&msg, FTPM_RPC_TIMEOUT);
	if (unlikely(rc)) {
		dev_err(dev, "%s: rpc_send_request_timeout failed, rc=%d\n", __func__, rc);
		goto err;
	}

	rpc_dma_sync_single_for_cpu(pvt_data->shm_buf_dma_addr, MAX_TRANSACTION_SIZE, DMA_FROM_DEVICE);

	if ((TPM_HEADER_SIZE > msg.body.response.length) || (MAX_RESPONSE_SIZE < msg.body.response.length)) {
		dev_err(dev, "%s: tpm response bad size - %d byte(s)\n", __func__, msg.body.response.length);
		rc = (-EIO);
		goto err;
	}

	if (pvt_data->resp_len) {
		dev_err(dev, "%s: race condition occured, previous response will be overwritten\n", __func__);
	}

	memcpy(pvt_data->resp_buf, (pvt_data->shm_buf + MAX_COMMAND_SIZE), msg.body.response.length);
	pvt_data->resp_len = msg.body.response.length;

	return 0;

err:

	return rc;
}

static void ftpm_smc_tpm_op_cancel(struct tpm_chip *chip)
{
	/* not supported */
}

static u8 ftpm_smc_tpm_op_status(struct tpm_chip *chip)
{
	return 0;
}

static bool ftpm_smc_tpm_op_req_canceled(struct tpm_chip *chip, u8 status)
{
	return 0;
}

static const struct tpm_class_ops ftpm_smc_tpm_ops = {
	.flags = TPM_OPS_AUTO_STARTUP,
	.recv = ftpm_smc_tpm_op_recv,
	.send = ftpm_smc_tpm_op_send,
	.cancel = ftpm_smc_tpm_op_cancel,
	.status = ftpm_smc_tpm_op_status,
	.req_complete_mask = 0,
	.req_complete_val = 0,
	.req_canceled = ftpm_smc_tpm_op_req_canceled,
};

static int ftpm_smc_drv_probe(struct platform_device *pdev)
{
	int rc;
	struct device *dev = &pdev->dev;
	struct tpm_chip *chip;
	struct ftpm_smc_private *pvt_data = NULL;

	dev_info(dev, "%s: probing %s\n", __func__, pdev->name);

	pvt_data = kzalloc(sizeof(struct ftpm_smc_private), GFP_KERNEL);
	if (!pvt_data) {
		dev_err(dev, "%s: kzalloc failed\n", __func__);
		rc = (-ENOMEM);
		goto err;
	}

	dev_set_drvdata(dev, pvt_data);

	rc = get_rpc_tunnel_id(pdev);
    if (rc) {
		dev_err(dev, "%s: get_rpc_tunnel_id failed\n", __func__);
        goto err;
    }

	pvt_data->shm_buf = kzalloc(MAX_TRANSACTION_SIZE, GFP_KERNEL|GFP_DMA32);
	if (!pvt_data->shm_buf) {
		dev_err(dev, "%s: kzalloc failed\n", __func__);
		rc = (-ENOMEM);
		goto err;
	}

	pvt_data->shm_buf_dma_addr = rpc_dma_map_single(pvt_data->shm_buf, MAX_TRANSACTION_SIZE, DMA_BIDIRECTIONAL);
	if (DMA_MAPPING_ERROR == pvt_data->shm_buf_dma_addr) {
		dev_err(dev, "%s: rpc_dma_map_single failed\n", __func__);
		rc = (-ENOMEM);
		goto err;
	}

    rc = rpc_register_functions(RPC_SERVICE_FTPM, ftpm_smc_funcs_tbl, (FTPM_SMC_MAX_FUNC_NUM - 1 ));
    if (rc) {
		dev_err(dev, "%s: rpc_register_functions failed\n", __func__);
        goto err;
    }

	chip = tpm_chip_alloc(dev, &ftpm_smc_tpm_ops);
	if (IS_ERR(chip)) {
		dev_err(dev, "%s: tpm_chip_alloc failed\n", __func__);
		rc = PTR_ERR(chip);
		goto out_rpc_unreg;
	}

	pvt_data->chip = chip;
	pvt_data->chip->flags |= TPM_CHIP_FLAG_TPM2;

	rc = tpm_chip_register(pvt_data->chip);
	if (rc) {
		dev_err(dev, "%s: tpm_chip_register failed\n", __func__);
		goto out_chip;
	}

	dev_info(dev, "%s: %s - ok\n", __func__, pdev->name);

	return 0;

out_chip:
	put_device(&pvt_data->chip->dev);
out_rpc_unreg:
	rpc_unregister_functions(RPC_SERVICE_FTPM);
err:
	if (pvt_data->shm_buf_dma_addr && (DMA_MAPPING_ERROR != pvt_data->shm_buf_dma_addr)) {
		rpc_dma_unmap_single(pvt_data->shm_buf_dma_addr, MAX_TRANSACTION_SIZE, DMA_BIDIRECTIONAL);
	}
	if (pvt_data->shm_buf) kfree(pvt_data->shm_buf);
	if (pvt_data) kfree(pvt_data);

	return rc;
}

static int ftpm_smc_drv_remove(struct platform_device *pdev)
{
	struct ftpm_smc_private *pvt_data = dev_get_drvdata(&pdev->dev);

	if (!pvt_data) return -1;

	tpm_chip_unregister(pvt_data->chip);
	put_device(&pvt_data->chip->dev);

	if (pvt_data->shm_buf_dma_addr && (DMA_MAPPING_ERROR != pvt_data->shm_buf_dma_addr)) {
		rpc_dma_unmap_single(pvt_data->shm_buf_dma_addr, MAX_TRANSACTION_SIZE, DMA_BIDIRECTIONAL);
	}

	if (pvt_data->shm_buf) kfree(pvt_data->shm_buf);
	if (pvt_data) kfree(pvt_data);

	return 0;
}

static void ftpm_smc_drv_shutdown(struct platform_device *pdev)
{
	dev_err(&pdev->dev, "%s: not implemented\n", __func__);
}

static const struct of_device_id of_ftpm_smc_ids[] = {
	{ .compatible = "brcm,ftpm-smc" },
	{ }
};
MODULE_DEVICE_TABLE(of, of_ftpm_smc_ids);

static struct platform_driver ftpm_smc_plat_drv = {
	.driver = {
		.name = "ftpm-smc",
		.of_match_table = of_match_ptr(of_ftpm_smc_ids),
	},
	.shutdown = ftpm_smc_drv_shutdown,
	.probe = ftpm_smc_drv_probe,
	.remove = ftpm_smc_drv_remove,
};

static int __init ftpm_smc_mod_init(void)
{
	return platform_driver_register(&ftpm_smc_plat_drv);
}

static void __exit ftpm_smc_mod_exit(void)
{
	platform_driver_unregister(&ftpm_smc_plat_drv);
}



module_init(ftpm_smc_mod_init);
module_exit(ftpm_smc_mod_exit);

MODULE_AUTHOR("Alex Malamud <alex.malamud@broadcom.com>");
MODULE_DESCRIPTION("Broadcom TPM Driver for SMC based fTPM 2.0");
MODULE_VERSION("1.1");
MODULE_LICENSE("GPL v2");
