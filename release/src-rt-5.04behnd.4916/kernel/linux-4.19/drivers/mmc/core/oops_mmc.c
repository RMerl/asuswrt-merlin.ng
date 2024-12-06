// SPDX-License-Identifier: GPL-2.0
/*
 * MMC Oops/Panic Looger
 *
 * Author: Carlos Soto <carlos.soto@broadcom.com>
 *
 * Copyright 2023 (c) Broadcom Corporation
 */

#if defined(CONFIG_BCM_KF_MMC_OOPS)

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kmsg_dump.h>
#include <linux/slab.h>
#include <linux/mmc/mmc.h>
#include <linux/mmc/host.h>
#include <linux/mmc/card.h>
#include <linux/scatterlist.h>
#include <linux/platform_device.h>
#include <linux/mmc/core.h>
#include <linux/slab.h>
#include <linux/moduleparam.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/hdreg.h>
#include <linux/kdev_t.h>
#include <linux/blkdev.h>
#include <linux/mutex.h>
#include <linux/scatterlist.h>
#include <linux/string_helpers.h>
#include <linux/delay.h>
#include <linux/capability.h>
#include <linux/compat.h>
#include <linux/pm_runtime.h>
#include <linux/mmc/ioctl.h>
#include <linux/mmc/sd.h>
#include <linux/uaccess.h>
#include <linux/file.h>
#include "core.h"

/* Enable if oops driver should attempt to send a STOP_TRANSMISSION request
 * This will force eMMC to flush its cache. Also results in stable eMMC boot
 * if RST_N is not configured (it is still recommended to enable RST_N). This 
 * does trigger several warnings as we are trying to issue a request while the 
 * last one has not been completed (due to interrupts being disabled, driver 
 * doesnt know if last request completed, we assume it completes after 2sec ). 
 * If disabling then it is recommended to enable the OOPS_MMC_REVERSE_DUMP 
 * as it will result in preserving the most recent kmsg lines
 */
#define OOPS_MMC_SEND_STOP  1

/* Enable if tail end of oops trace is not making it to the eMMC device. 
 * mmc oops is written via open ended mmc write which initially writes
 * data to the device's internal cache. If a system reset happens and
 * the emmc is unable to flush its cache completely, chances are the last
 * few blocks will be lost. By using a reverse dump we send the oops trace
 * in reverse order with the newest messages going out in the first few
 * blocks of data. This increases the chances of preserving the most
 * important sections of the oops trace, namely the trace dump */
#define OOPS_MMC_REVERSE_DUMP	1
#if OOPS_MMC_REVERSE_DUMP
#define OOPS_MMC_REV_CHUNK_DIV "[REV_OOPS]\n"
#endif

/* TODO Unify the oops header, mmtoops, ramoops, mmcoops */
#define OOPS_MMC_DUMP_SIGNATURE "BRCM-OOPS-DUMP-MMC"
#define OOPS_MMC_DUMP_HDR_LENGTH \
	((sizeof(OOPS_MMC_DUMP_SIGNATURE))-1+(sizeof(unsigned long)))
static char *dump_mark =
	"================================";
static char *dump_start_str =
	"PREVIOUS_KERNEL_OOPS_DUMP_START";
static char *dump_end_str =
	"PREVIOUS_KERNEL_OOPS_DUMP_END";

/* At a time how many blocks we read or write */
#define OOPS_MMC_NUM_RW_BLOCKS   128
#define OOPS_MMC_INVALID 0xFFFFFFFF

static struct platform_device *dummy;
static struct oops_mmc_platform_data *dummy_data;

static char mmcdev[80];
module_param_string(mmcdev, mmcdev, 80, 0400);
MODULE_PARM_DESC(mmcdev,
		"Name or path of the MMC block device to use");

static unsigned long partition_start_block = OOPS_MMC_INVALID;
module_param(partition_start_block, ulong, 0400);
MODULE_PARM_DESC(partition_start_block,
		"partition start block to dump the panic");

static unsigned long partition_size = OOPS_MMC_INVALID;
module_param(partition_size, ulong, 0400);
MODULE_PARM_DESC(partition_size,
		"size of the partition to dump the panic");

static unsigned long mmc_blksz = OOPS_MMC_INVALID;
module_param(mmc_blksz, ulong, 0400);
MODULE_PARM_DESC(mmc_blksz,
		"each block size of MMC, usually 512");

static char *dump_file_path;
module_param(dump_file_path, charp, 0400);
MODULE_PARM_DESC(dump_file_path,
		"Dump the panics to file instead of console");

static char *dump_to_console;
module_param(dump_to_console, charp, 0400);
MODULE_PARM_DESC(dump_to_console,
		"Specify whether to dump to console or not");

static unsigned long rw_blocks = OOPS_MMC_NUM_RW_BLOCKS;
module_param(rw_blocks, ulong, 0400);
MODULE_PARM_DESC(rw_blocks,
		"record size for MMC OOPS in blocks (default 128)");

static struct oops_mmc_context {
	struct kmsg_dumper	dump;
	struct mmc_card		*card;
	/* start block address to dump*/
	unsigned long		start_block_addr;
	/* max blocks available to dump */
	unsigned long		max_block_count;
	/* each MMC block size */
	unsigned long		mmc_blksz;
	/* Total bytes that we write or read each time */
	unsigned long		num_rw_bytes;
	char				*buff;
} mmc_oops_context;


struct oops_mmc_platform_data {
	struct platform_device	*pdev;
	char			*mmcdev;
	unsigned long		partition_start_block;
	unsigned long		partition_size;
	unsigned long		mmc_blksz;
	char			*dump_file_path;
	char			*dump_to_console;
};

enum oops_mmc_op {
	OOPS_MMC_READ = 0,
	OOPS_MMC_WRITE,
	OOPS_MMC_PANIC_WRITE,
};

static inline bool is_mmc_ongoing_tfr(struct mmc_host *host)
{
	struct mmc_request *ongoing_mrq = READ_ONCE(host->ongoing_mrq);

	/*
	 * If there is an ongoing transfer, wait for the command line to become
	 * available.
	 */
	if (ongoing_mrq && !completion_done(&ongoing_mrq->cmd_completion))
		return true;

	return false;
}

static void oops_mmc_panic_read_write(struct oops_mmc_context *cxt,
	char *buf, unsigned long start, int num_blocks, enum oops_mmc_op op, bool tfr)
{
	struct mmc_card *card = cxt->card;
	struct mmc_host *host = card->host;
	struct mmc_request mrq;
	struct mmc_command cmd;
	struct mmc_command stop;
	struct mmc_data data;
	struct scatterlist sg;
	atomic_t oops_thread_lock_abort;
	int ret;

	memset(&mrq, 0, sizeof(struct mmc_request));
	memset(&cmd, 0, sizeof(struct mmc_command));
	memset(&stop, 0, sizeof(struct mmc_command));
	memset(&data, 0, sizeof(struct mmc_data));

	mrq.cmd = &cmd;
	mrq.data = &data;
	mrq.stop = &stop;
	mrq.cap_cmd_during_tfr = tfr;

	data.blksz = cxt->mmc_blksz;
	data.blocks = num_blocks;
	data.sg = &sg;
	data.sg_len = 1;

	sg_init_one(&sg, buf, (cxt->mmc_blksz*num_blocks));

	if (op == OOPS_MMC_READ) {
		if (data.blocks == 1)
			cmd.opcode = MMC_READ_SINGLE_BLOCK;
		else
			cmd.opcode = MMC_READ_MULTIPLE_BLOCK;
		data.flags = MMC_DATA_READ;
	} else {
		if (data.blocks == 1)
			cmd.opcode = MMC_WRITE_BLOCK;
		else
			cmd.opcode = MMC_WRITE_MULTIPLE_BLOCK;
		data.flags = MMC_DATA_WRITE;
	}

	cmd.arg = start;
	cmd.flags = MMC_RSP_R1 | MMC_CMD_ADTC;

	if (data.blocks == 1)
		mrq.stop = NULL;
	else {
		stop.opcode = MMC_STOP_TRANSMISSION;
		stop.arg = 0;
		stop.flags = MMC_RSP_R1B | MMC_CMD_AC;
	}

	/* If host has been claimed by anyone, we want to skip the lock aqcuiring
	 * code when claiming the host. This avoids a deadlock if the host was claimed
	 * by a process which crashed and we end up calling schedule to wait for it
	 */
	atomic_set(&oops_thread_lock_abort, host->claimed);
	ret = __mmc_claim_host(host, NULL, (op == OOPS_MMC_PANIC_WRITE) ?
					    &oops_thread_lock_abort : NULL);

	if (tfr && is_mmc_ongoing_tfr(host)) {
		pr_err("oops_mmc_panic_%s[%d] not possible due to ongoing tfr!\n",
		       (op == OOPS_MMC_READ) ? "read" : "write", __LINE__);
	} else {
		/* Handle actual panic write - dont wait for command completion */
		if (op == OOPS_MMC_PANIC_WRITE)
			mmc_start_request(host, &mrq);
		else
			mmc_wait_for_req(host, &mrq);
	}

	/* In a panic write we have no expectations of getting a response,
	 * so we simply busy wait for 2 seconds for the request to be sent
	 */
	if (op == OOPS_MMC_PANIC_WRITE) {
		mdelay(2000);
#if OOPS_MMC_SEND_STOP		
		mrq.cap_cmd_during_tfr = 0;
		mrq.data = NULL;
		mrq.stop = NULL;
		cmd.data = NULL;
		cmd.arg = 0;
		cmd.retries = 0;
		cmd.opcode = MMC_STOP_TRANSMISSION,
		cmd.flags = MMC_RSP_R1 | MMC_CMD_AC;
		mrq.cmd = &cmd;
		mmc_start_request(host, &mrq);
#endif /* OOPS_MMC_SEND_STOP */		
	}

	if (!ret)
		mmc_release_host(host);

	if (cmd.error || data.error) {
		pr_err("oops_mmc_panic_%s[%d] %s error %d\n",
		       (op == OOPS_MMC_READ) ? "read" : "write", __LINE__,
		       cmd.error ? "cmd" : "data",
		       cmd.error ? cmd.error : data.error);
	}
}

static void oops_mmc_do_dump(struct kmsg_dumper *dumper,
			enum kmsg_dump_reason reason)
{
	struct oops_mmc_context *cxt =
		container_of(dumper, struct oops_mmc_context, dump);
	char *buff = cxt->buff;
	bool rc = true;
	size_t header_size, text_len = 0;
	int text_length = 0, read_cnt = 0;

	if (num_online_cpus() > 1 || preemptible()) {
		pr_info("%s: called from non-atomic context!...\n", __func__);
		return;
	}

	kmsg_dump_rewind_nolock(dumper);

	/* Add Dump signature and
	 * block size before kernel log
	 */
	memset(buff, '\0', cxt->num_rw_bytes);
	memcpy(buff, OOPS_MMC_DUMP_SIGNATURE,
			strlen(OOPS_MMC_DUMP_SIGNATURE));
	header_size = strlen(OOPS_MMC_DUMP_SIGNATURE)
						+ sizeof(int);
	read_cnt = cxt->num_rw_bytes-header_size;
	buff += header_size;

#if OOPS_MMC_REVERSE_DUMP
	/* reverse dump the kmsg buffer in cxt->mmc_blksz*2 chunks */
	{
		int oops_chunk_size = cxt->mmc_blksz*2;
		int oops_chunk_div_size = strlen(OOPS_MMC_REV_CHUNK_DIV);
		text_length += header_size;
		while (1) {
			if( cxt->num_rw_bytes
				- text_length
				- oops_chunk_size
				- oops_chunk_div_size >= 0) {

				/* add a divider */
				memcpy(buff,OOPS_MMC_REV_CHUNK_DIV, oops_chunk_div_size);
				text_length += oops_chunk_div_size;
				buff += oops_chunk_div_size;
	
				/* ket oops chunk */
				rc = kmsg_dump_get_buffer(dumper, false,
					buff, oops_chunk_size, &text_len);
				text_length += text_len;
				buff += text_len;

				if (!rc) {
					pr_err("%s: end of read from the kmsg dump\n",
					       __func__);
					break;
				}
			} else {
				pr_err("%s: read limit reached from the kmsg dump\n",
			       __func__);
				break;
			}

		}
		/* Indicate actual size of oops data */
		text_length -= header_size;
	}
#else	
	/* dump the kmsg buffer */
	rc = kmsg_dump_get_buffer(dumper, false,
			buff, read_cnt, &text_len);
	text_length += text_len;
	if (!rc) {
		pr_err("%s: end of read from the kmsg dump\n",
		       __func__);
	} else {
		pr_err("%s: read limit reached from the kmsg dump\n",
		       __func__);
	}
#endif /* OOPS_MMC_REVERSE_DUMP */	

	pr_info("%s: writing to MMC = %d/%d\n", __func__, text_length, rw_blocks);
	buff = (char *)cxt->buff+strlen(OOPS_MMC_DUMP_SIGNATURE);
	memcpy(buff, &text_length, sizeof(int));
	/* Write the emmc flash at one iteration, the emmc write
	 * is interrupt driven, it will hang on futher emmc write
	 * since we are executing under NMI routine
	 */
	oops_mmc_panic_read_write(cxt, cxt->buff,
		(cxt->start_block_addr), rw_blocks, OOPS_MMC_PANIC_WRITE, false);
}

static int oops_mmc_probe(struct platform_device *pdev)
{
	struct oops_mmc_platform_data *pdata = pdev->dev.platform_data;
	struct oops_mmc_context *context = &mmc_oops_context;
	int err = -EINVAL;
	int i;
	char *buf;
	int text_len = 0;
	loff_t pos = 0;
	struct file *file = NULL;
	char marker_string[200] = "";

	if (!pdata) {
		pr_err("%s: No platform data. Error!\n", __func__);
		return -EINVAL;
	}

	if (pdata->mmcdev == NULL) {
		pr_err("%s: mmcdev is NULL\n", __func__);
		return err;
	}

	context->mmc_blksz = pdata->mmc_blksz;
	context->start_block_addr = pdata->partition_start_block;
	context->max_block_count = (pdata->partition_size)/(pdata->mmc_blksz);
	context->num_rw_bytes = (rw_blocks * context->mmc_blksz);
	context->card = mmc_blk_dev_to_card(pdata->mmcdev);

	if (context->card == NULL) {
		pr_err("%s: card is NULL\n", __func__);
		return err;
	}

	if (mmc_card_mmc(context->card))
		pr_info("%s: its MMC card..\n", __func__);
	else if (mmc_card_sd(context->card))
		pr_info("%s: its SD card..\n", __func__);
	else {
		pr_err("%s: Not and SD or MMC! return error!\n", __func__);
		return err;
	}

	pr_info("%s: partition_start_block=0x%lx, mmc_blksz=0x%lx, max_blocks=%lu\n",
		__func__, context->start_block_addr,
		context->mmc_blksz, context->max_block_count);

	/* Allocate min io size buffer to be used in do_dump */
	context->buff = kmalloc(context->num_rw_bytes, GFP_KERNEL);
	if (context->buff == NULL) {
		err = -EINVAL;
		goto kmalloc_failed;
	}

	context->dump.dump = oops_mmc_do_dump;
	err = kmsg_dump_register(&context->dump);
	if (err) {
		pr_err("%s: registering kmsg dumper failed\n", __func__);
		goto kmsg_dump_register_failed;
	}

	if (pdata->dump_file_path) {
		pr_info("%s: dump_file_path = %s\n", __func__,
			pdata->dump_file_path);
	} else if ((pdata->dump_to_console) &&
			((!strcmp(pdata->dump_to_console, "n")) ||
			(!strcmp(pdata->dump_to_console, "no"))))
		pr_info("%s: dump_to_console=no, OEM has own script", __func__);
	else
		pr_info("%s: If any panic, it will be dumped to console\n",
			__func__);

	buf = (char *)context->buff;

	oops_mmc_panic_read_write(context, buf,
		context->start_block_addr, 1, OOPS_MMC_READ, false);
	if (!strncmp(OOPS_MMC_DUMP_SIGNATURE,
			buf, strlen(OOPS_MMC_DUMP_SIGNATURE))) {
		sprintf(marker_string, "\n%s%s%s\n", dump_mark, dump_start_str,
			dump_mark);
		pr_info("%s", marker_string);

		if (pdata->dump_file_path) {
			file = filp_open(pdata->dump_file_path, O_WRONLY|O_CREAT, 0644);
			if (IS_ERR(file)) {
				pr_err("%s: cannot open %s, dump to console only\n",
			       __func__, pdata->dump_file_path);
				file = NULL;
			} else {
				kernel_write(file, marker_string,
					strlen(marker_string), &pos);
				pos = pos+strlen(marker_string);
				pr_info("%s: panics dumped to the file [%s]\n",
					 __func__, pdata->dump_file_path);
			}

		}
		else if ((pdata->dump_to_console) &&
			((!strcmp(pdata->dump_to_console, "n")) ||
			(!strcmp(pdata->dump_to_console, "no")))) {
			pr_info("%s:OEM has own script to read!\n",
				__func__);
			pr_info("\n%s%s%s\n", dump_mark,
				dump_end_str, dump_mark);
			return 0;
		}

		for (i = 0; i < context->max_block_count; i += rw_blocks) {
			oops_mmc_panic_read_write(context, buf,
				(context->start_block_addr+i),
				rw_blocks, OOPS_MMC_READ, false);
			if (strncmp(OOPS_MMC_DUMP_SIGNATURE, buf,
				    strlen(OOPS_MMC_DUMP_SIGNATURE))) {
				break;
			}
			memcpy(&text_len, &buf[strlen(OOPS_MMC_DUMP_SIGNATURE)],
						sizeof(int));
			buf = buf+strlen(OOPS_MMC_DUMP_SIGNATURE)+sizeof(int);

			if ((text_len == 0)) {
				pr_info("%s:Invalid text length[%d]\n",
					__func__, text_len);
				break;
			}
			if (text_len > context->num_rw_bytes) {
				pr_info("%s:text length[%d] over maximum allowed length, trim the length of content as [%d]\n",
					__func__, text_len, context->num_rw_bytes);
				text_len = context->num_rw_bytes;
			}

			pr_info("%s: printing text length = %d\n",
				__func__, text_len);
			pr_err("%s: dump previous kernel oops log! [length: %d]\n",
                                __func__, text_len);

			if (file) {
				kernel_write(file, buf, text_len, &pos);
				pos = pos+text_len;
			} else {
				char *ptr = buf;
				char *line;

				while ((line = strsep(&ptr, "\n")) != NULL)
					pr_info("%s\n", line);
			}
			buf = (char *)context->buff;
		}
		sprintf(marker_string, "\n%s%s%s\n", dump_mark, dump_end_str,
			dump_mark);
		pr_info("%s", marker_string);
		if (file) {
			kernel_write(file, marker_string,
				strlen(marker_string), &pos);
			pos = pos+strlen(marker_string);
		}
		/* Clear buffer */
		buf = (char *)context->buff;
		memset(buf, '\0', context->num_rw_bytes);
		for (i = 0; i < context->max_block_count; i += rw_blocks) {
			oops_mmc_panic_read_write(context, buf,
				(context->start_block_addr+i),
				rw_blocks, OOPS_MMC_WRITE, false);
		}

		if (file)
			filp_close(file, NULL);
	}

	return 0;

kmsg_dump_register_failed:
	kfree(context->buff);
	context->buff = NULL;
kmalloc_failed:
	return err;

}

static int oops_mmc_remove(struct platform_device *pdev)
{
	struct oops_mmc_context *cxt = &mmc_oops_context;

	if (kmsg_dump_unregister(&cxt->dump) < 0)
		pr_err("%s: could not unregister kmsg dumper\n", __func__);
	kfree(cxt->buff);
	cxt->buff = NULL;
	return 0;
}

static struct platform_driver oops_mmc_driver = {
	.remove			= __exit_p(oops_mmc_remove),
	.probe                  = oops_mmc_probe,
	.driver			= {
		.name		= "oops_mmc",
		.owner		= THIS_MODULE,
	},
};

static int __init oops_mmc_init(void)
{
	int ret;

	ret = platform_driver_probe(&oops_mmc_driver, oops_mmc_probe);
	if (ret == -ENODEV) {
		/*
		 * If we didn't find a platform device, we use module parameters
		 * building platform data on the fly.
		 */
		pr_info("platform device not found, using module parameters\n");

		if (strlen(mmcdev) == 0) {
			pr_err("%s: mmc device (mmcdev=name) must be supplied\n",
			      __func__);
			return -EINVAL;
		}

		if (partition_start_block == OOPS_MMC_INVALID ||
				partition_size == OOPS_MMC_INVALID ||
				mmc_blksz == OOPS_MMC_INVALID){
			pr_err("%s : Invalid module params\n", __func__);
			return -EINVAL;
		}

		dummy_data = kzalloc(sizeof(struct oops_mmc_platform_data),
					GFP_KERNEL);
		if (!dummy_data)
			return -ENOMEM;

		dummy_data->mmcdev = mmcdev;
		dummy_data->partition_start_block = partition_start_block;
		dummy_data->partition_size = partition_size;
		dummy_data->mmc_blksz = mmc_blksz;
		dummy_data->dump_file_path = dump_file_path;
		dummy_data->dump_to_console = dump_to_console;
		dummy = platform_create_bundle(&oops_mmc_driver, oops_mmc_probe,
			NULL, 0, dummy_data,
			sizeof(struct oops_mmc_platform_data));
		if (IS_ERR(dummy)) {
			ret = PTR_ERR(dummy);
			kfree(dummy_data);
		} else
			ret = 0;
	}
	return ret;

}
static void __exit oops_mmc_exit(void)
{
	kfree(dummy_data);
	dummy_data = NULL;
	platform_device_unregister(dummy);
	platform_driver_unregister(&oops_mmc_driver);
	dummy = NULL;
}

module_init(oops_mmc_init);
module_exit(oops_mmc_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BROADCOM");
MODULE_DESCRIPTION("MMC Oops/Panic Looger");

#endif /* CONFIG_BCM_KF_MMC_OOPS */
