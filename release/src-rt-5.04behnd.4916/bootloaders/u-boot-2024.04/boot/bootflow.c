// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2021 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#define LOG_CATEGORY UCLASS_BOOTSTD

#include <common.h>
#include <bootdev.h>
#include <bootflow.h>
#include <bootmeth.h>
#include <bootstd.h>
#include <dm.h>
#include <env_internal.h>
#include <malloc.h>
#include <serial.h>
#include <dm/device-internal.h>
#include <dm/uclass-internal.h>

/* error codes used to signal running out of things */
enum {
	BF_NO_MORE_PARTS	= -ESHUTDOWN,
	BF_NO_MORE_DEVICES	= -ENODEV,
};

/**
 * bootflow_state - name for each state
 *
 * See enum bootflow_state_t for what each of these means
 */
static const char *const bootflow_state[BOOTFLOWST_COUNT] = {
	"base",
	"media",
	"part",
	"fs",
	"file",
	"ready",
};

const char *bootflow_state_get_name(enum bootflow_state_t state)
{
	/* This doesn't need to be a useful name, since it will never occur */
	if (state < 0 || state >= BOOTFLOWST_COUNT)
		return "?";

	return bootflow_state[state];
}

int bootflow_first_glob(struct bootflow **bflowp)
{
	struct bootstd_priv *std;
	int ret;

	ret = bootstd_get_priv(&std);
	if (ret)
		return ret;

	if (list_empty(&std->glob_head))
		return -ENOENT;

	*bflowp = list_first_entry(&std->glob_head, struct bootflow,
				   glob_node);

	return 0;
}

int bootflow_next_glob(struct bootflow **bflowp)
{
	struct bootstd_priv *std;
	struct bootflow *bflow = *bflowp;
	int ret;

	ret = bootstd_get_priv(&std);
	if (ret)
		return ret;

	*bflowp = NULL;

	if (list_is_last(&bflow->glob_node, &std->glob_head))
		return -ENOENT;

	*bflowp = list_entry(bflow->glob_node.next, struct bootflow, glob_node);

	return 0;
}

void bootflow_iter_init(struct bootflow_iter *iter, int flags)
{
	memset(iter, '\0', sizeof(*iter));
	iter->first_glob_method = -1;
	iter->flags = flags;

	/* remember the first bootdevs we see */
	iter->max_devs = BOOTFLOW_MAX_USED_DEVS;
}

void bootflow_iter_uninit(struct bootflow_iter *iter)
{
	free(iter->method_order);
}

int bootflow_iter_drop_bootmeth(struct bootflow_iter *iter,
				const struct udevice *bmeth)
{
	/* We only support disabling the current bootmeth */
	if (bmeth != iter->method || iter->cur_method >= iter->num_methods ||
	    iter->method_order[iter->cur_method] != bmeth)
		return -EINVAL;

	memmove(&iter->method_order[iter->cur_method],
		&iter->method_order[iter->cur_method + 1],
		(iter->num_methods - iter->cur_method - 1) * sizeof(void *));

	iter->num_methods--;

	return 0;
}

/**
 * bootflow_iter_set_dev() - switch to the next bootdev when iterating
 *
 * This sets iter->dev, records the device in the dev_used[] list and shows a
 * message if required
 *
 * @iter: Iterator to update
 * @dev: Bootdev to use, or NULL if there are no more
 */
static void bootflow_iter_set_dev(struct bootflow_iter *iter,
				  struct udevice *dev, int method_flags)
{
	struct bootmeth_uc_plat *ucp = dev_get_uclass_plat(iter->method);

	log_debug("iter: Setting dev to %s, flags %x\n",
		  dev ? dev->name : "(none)", method_flags);
	iter->dev = dev;
	iter->method_flags = method_flags;

	if (IS_ENABLED(CONFIG_BOOTSTD_FULL)) {
		/* record the device for later */
		if (dev && iter->num_devs < iter->max_devs)
			iter->dev_used[iter->num_devs++] = dev;

		if ((iter->flags & (BOOTFLOWIF_SHOW | BOOTFLOWIF_SINGLE_DEV)) ==
		    BOOTFLOWIF_SHOW) {
			if (dev)
				printf("Scanning bootdev '%s':\n", dev->name);
			else if (IS_ENABLED(CONFIG_BOOTMETH_GLOBAL) &&
				ucp->flags & BOOTMETHF_GLOBAL)
				printf("Scanning global bootmeth '%s':\n",
				iter->method->name);
			else
				printf("No more bootdevs\n");
		}
	}
}

/**
 * scan_next_in_uclass() - Scan for the next bootdev in the same media uclass
 *
 * Move through the following bootdevs until we find another in this media
 * uclass, or run out
 *
 * @devp: On entry, the device to check, on exit the new device, or NULL if
 * there is none
 */
static void scan_next_in_uclass(struct udevice **devp)
{
	struct udevice *dev = *devp;
	enum uclass_id cur_id = device_get_uclass_id(dev->parent);

	do {
		uclass_find_next_device(&dev);
	} while (dev && cur_id != device_get_uclass_id(dev->parent));

	*devp = dev;
}

/**
 * iter_incr() - Move to the next item (method, part, bootdev)
 *
 * Return: 0 if OK, BF_NO_MORE_DEVICES if there are no more bootdevs
 */
static int iter_incr(struct bootflow_iter *iter)
{
	struct udevice *dev;
	bool inc_dev = true;
	bool global;
	int ret;

	log_debug("entry: err=%d\n", iter->err);
	global = iter->doing_global;

	if (iter->err == BF_NO_MORE_DEVICES)
		return BF_NO_MORE_DEVICES;

	if (iter->err != BF_NO_MORE_PARTS) {
		/* Get the next boothmethod */
		if (++iter->cur_method < iter->num_methods) {
			iter->method = iter->method_order[iter->cur_method];
			return 0;
		}

		/*
		 * If we have finished scanning the global bootmeths, start the
		 * normal bootdev scan
		 */
		if (IS_ENABLED(CONFIG_BOOTMETH_GLOBAL) && global) {
			iter->num_methods = iter->first_glob_method;
			iter->doing_global = false;

			/*
			 * Don't move to the next dev as we haven't tried this
			 * one yet!
			 */
			inc_dev = false;
		}
	}

	/* No more bootmeths; start at the first one, and... */
	iter->cur_method = 0;
	iter->method = iter->method_order[iter->cur_method];

	if (iter->err != BF_NO_MORE_PARTS) {
		/* ...select next partition  */
		if (++iter->part <= iter->max_part)
			return 0;
	}

	/* No more partitions; start at the first one and... */
	iter->part = 0;

	/*
	 * Note: as far as we know, there is no partition table on the next
	 * bootdev, so set max_part to 0 until we discover otherwise. See
	 * bootdev_find_in_blk() for where this is set.
	 */
	iter->max_part = 0;

	/* ...select next bootdev */
	if (iter->flags & BOOTFLOWIF_SINGLE_DEV) {
		ret = -ENOENT;
	} else {
		int method_flags;

		ret = 0;
		dev = iter->dev;
		log_debug("inc_dev=%d\n", inc_dev);
		if (!inc_dev) {
			ret = bootdev_setup_iter(iter, NULL, &dev,
						 &method_flags);
		} else if (IS_ENABLED(CONFIG_BOOTSTD_FULL) &&
			   (iter->flags & BOOTFLOWIF_SINGLE_UCLASS)) {
			scan_next_in_uclass(&dev);
			if (!dev) {
				log_debug("finished uclass %s\n",
					  dev_get_uclass_name(dev));
				ret = -ENODEV;
			}
		} else if (IS_ENABLED(CONFIG_BOOTSTD_FULL) &&
			   iter->flags & BOOTFLOWIF_SINGLE_MEDIA) {
			log_debug("next in single\n");
			method_flags = 0;
			do {
				/*
				 * Move to the next bootdev child of this media
				 * device. This ensures that we cover all the
				 * available SCSI IDs and LUNs.
				 */
				device_find_next_child(&dev);
				log_debug("- next %s\n",
					dev ? dev->name : "(none)");
			} while (dev && device_get_uclass_id(dev) !=
				UCLASS_BOOTDEV);
			if (!dev) {
				log_debug("finished uclass %s\n",
					  dev_get_uclass_name(dev));
				ret = -ENODEV;
			}
		} else {
			log_debug("labels %p\n", iter->labels);
			if (iter->labels) {
				/*
				 * when the label is "mmc" we want to scan all
				 * mmc bootdevs, not just the first. See
				 * bootdev_find_by_label() where this flag is
				 * set up
				 */
				if (iter->method_flags &
				    BOOTFLOW_METHF_SINGLE_UCLASS) {
					scan_next_in_uclass(&dev);
					log_debug("looking for next device %s: %s\n",
						  iter->dev->name,
						  dev ? dev->name : "<none>");
				} else {
					dev = NULL;
				}
				if (!dev) {
					log_debug("looking at next label\n");
					ret = bootdev_next_label(iter, &dev,
								 &method_flags);
				}
			} else {
				ret = bootdev_next_prio(iter, &dev);
				method_flags = 0;
			}
		}
		log_debug("ret=%d, dev=%p %s\n", ret, dev,
			  dev ? dev->name : "none");
		if (ret) {
			bootflow_iter_set_dev(iter, NULL, 0);
		} else {
			/*
			 * Probe the bootdev. This does not probe any attached
			 * block device, since they are siblings
			 */
			ret = device_probe(dev);
			log_debug("probe %s %d\n", dev->name, ret);
			if (!log_msg_ret("probe", ret))
				bootflow_iter_set_dev(iter, dev, method_flags);
		}
	}

	/* if there are no more bootdevs, give up */
	if (ret)
		return log_msg_ret("incr", BF_NO_MORE_DEVICES);

	return 0;
}

/**
 * bootflow_check() - Check if a bootflow can be obtained
 *
 * @iter: Provides part, bootmeth to use
 * @bflow: Bootflow to update on success
 * Return: 0 if OK, -ENOSYS if there is no bootflow support on this device,
 *	BF_NO_MORE_PARTS if there are no more partitions on bootdev
 */
static int bootflow_check(struct bootflow_iter *iter, struct bootflow *bflow)
{
	struct udevice *dev;
	int ret;

	if (IS_ENABLED(CONFIG_BOOTMETH_GLOBAL) && iter->doing_global) {
		bootflow_iter_set_dev(iter, NULL, 0);
		ret = bootmeth_get_bootflow(iter->method, bflow);
		if (ret)
			return log_msg_ret("glob", ret);

		return 0;
	}

	dev = iter->dev;
	ret = bootdev_get_bootflow(dev, iter, bflow);

	/* If we got a valid bootflow, return it */
	if (!ret) {
		log_debug("Bootdev '%s' part %d method '%s': Found bootflow\n",
			  dev->name, iter->part, iter->method->name);
		return 0;
	}

	/* Unless there is nothing more to try, move to the next device */
	if (ret != BF_NO_MORE_PARTS && ret != -ENOSYS) {
		log_debug("Bootdev '%s' part %d method '%s': Error %d\n",
			  dev->name, iter->part, iter->method->name, ret);
		/*
		 * For 'all' we return all bootflows, even
		 * those with errors
		 */
		if (iter->flags & BOOTFLOWIF_ALL)
			return log_msg_ret("all", ret);
	}

	return log_msg_ret("check", ret);
}

int bootflow_scan_first(struct udevice *dev, const char *label,
			struct bootflow_iter *iter, int flags,
			struct bootflow *bflow)
{
	int ret;

	if (dev || label)
		flags |= BOOTFLOWIF_SKIP_GLOBAL;
	bootflow_iter_init(iter, flags);

	/*
	 * Set up the ordering of bootmeths. This sets iter->doing_global and
	 * iter->first_glob_method if we are starting with the global bootmeths
	 */
	ret = bootmeth_setup_iter_order(iter, !(flags & BOOTFLOWIF_SKIP_GLOBAL));
	if (ret)
		return log_msg_ret("obmeth", -ENODEV);

	/* Find the first bootmeth (there must be at least one!) */
	iter->method = iter->method_order[iter->cur_method];

	if (!IS_ENABLED(CONFIG_BOOTMETH_GLOBAL) || !iter->doing_global) {
		struct udevice *dev = NULL;
		int method_flags;

		ret = bootdev_setup_iter(iter, label, &dev, &method_flags);
		if (ret)
			return log_msg_ret("obdev", -ENODEV);

		bootflow_iter_set_dev(iter, dev, method_flags);
	}

	ret = bootflow_check(iter, bflow);
	if (ret) {
		log_debug("check - ret=%d\n", ret);
		if (ret != BF_NO_MORE_PARTS && ret != -ENOSYS) {
			if (iter->flags & BOOTFLOWIF_ALL)
				return log_msg_ret("all", ret);
		}
		iter->err = ret;
		ret = bootflow_scan_next(iter, bflow);
		if (ret)
			return log_msg_ret("get", ret);
	}

	return 0;
}

int bootflow_scan_next(struct bootflow_iter *iter, struct bootflow *bflow)
{
	int ret;

	do {
		ret = iter_incr(iter);
		log_debug("iter_incr: ret=%d\n", ret);
		if (ret == BF_NO_MORE_DEVICES)
			return log_msg_ret("done", ret);

		if (!ret) {
			ret = bootflow_check(iter, bflow);
			log_debug("check - ret=%d\n", ret);
			if (!ret)
				return 0;
			iter->err = ret;
			if (ret != BF_NO_MORE_PARTS && ret != -ENOSYS) {
				if (iter->flags & BOOTFLOWIF_ALL)
					return log_msg_ret("all", ret);
			}
		} else {
			log_debug("incr failed, err=%d\n", ret);
			iter->err = ret;
		}

	} while (1);
}

void bootflow_init(struct bootflow *bflow, struct udevice *bootdev,
		   struct udevice *meth)
{
	memset(bflow, '\0', sizeof(*bflow));
	bflow->dev = bootdev;
	bflow->method = meth;
	bflow->state = BOOTFLOWST_BASE;
}

void bootflow_free(struct bootflow *bflow)
{
	free(bflow->name);
	free(bflow->subdir);
	free(bflow->fname);
	if (!(bflow->flags & BOOTFLOWF_STATIC_BUF))
		free(bflow->buf);
	free(bflow->os_name);
	free(bflow->fdt_fname);
	free(bflow->bootmeth_priv);
}

void bootflow_remove(struct bootflow *bflow)
{
	if (bflow->dev)
		list_del(&bflow->bm_node);
	list_del(&bflow->glob_node);

	bootflow_free(bflow);
	free(bflow);
}

#if CONFIG_IS_ENABLED(BOOTSTD_FULL)
int bootflow_read_all(struct bootflow *bflow)
{
	int ret;

	if (bflow->state != BOOTFLOWST_READY)
		return log_msg_ret("rd", -EPROTO);

	ret = bootmeth_read_all(bflow->method, bflow);
	if (ret)
		return log_msg_ret("rd2", ret);

	return 0;
}
#endif /* BOOTSTD_FULL */

int bootflow_boot(struct bootflow *bflow)
{
	int ret;

	if (bflow->state != BOOTFLOWST_READY)
		return log_msg_ret("load", -EPROTO);

	ret = bootmeth_boot(bflow->method, bflow);
	if (ret)
		return log_msg_ret("boot", ret);

	/*
	 * internal error, should not get here since we should have booted
	 * something or returned an error
	 */

	return log_msg_ret("end", -EFAULT);
}

int bootflow_run_boot(struct bootflow_iter *iter, struct bootflow *bflow)
{
	int ret;

	printf("** Booting bootflow '%s' with %s\n", bflow->name,
	       bflow->method->name);
	if (IS_ENABLED(CONFIG_OF_HAS_PRIOR_STAGE) &&
	    (bflow->flags & BOOTFLOWF_USE_PRIOR_FDT))
		printf("Using prior-stage device tree\n");
	ret = bootflow_boot(bflow);
	if (!IS_ENABLED(CONFIG_BOOTSTD_FULL)) {
		printf("Boot failed (err=%d)\n", ret);
		return ret;
	}

	switch (ret) {
	case -EPROTO:
		printf("Bootflow not loaded (state '%s')\n",
		       bootflow_state_get_name(bflow->state));
		break;
	case -ENOSYS:
		printf("Boot method '%s' not supported\n", bflow->method->name);
		break;
	case -ENOTSUPP:
		/* Disable this bootflow for this iteration */
		if (iter) {
			int ret2;

			ret2 = bootflow_iter_drop_bootmeth(iter, bflow->method);
			if (!ret2) {
				printf("Boot method '%s' failed and will not be retried\n",
				       bflow->method->name);
			}
		}

		break;
	default:
		printf("Boot failed (err=%d)\n", ret);
		break;
	}

	return ret;
}

int bootflow_iter_check_blk(const struct bootflow_iter *iter)
{
	const struct udevice *media = dev_get_parent(iter->dev);
	enum uclass_id id = device_get_uclass_id(media);

	log_debug("uclass %d: %s\n", id, uclass_get_name(id));
	if (id != UCLASS_ETH && id != UCLASS_BOOTSTD && id != UCLASS_QFW)
		return 0;

	return -ENOTSUPP;
}

int bootflow_iter_check_sf(const struct bootflow_iter *iter)
{
	const struct udevice *media = dev_get_parent(iter->dev);
	enum uclass_id id = device_get_uclass_id(media);

	log_debug("uclass %d: %s\n", id, uclass_get_name(id));
	if (id == UCLASS_SPI_FLASH)
		return 0;

	return -ENOTSUPP;
}

int bootflow_iter_check_net(const struct bootflow_iter *iter)
{
	const struct udevice *media = dev_get_parent(iter->dev);
	enum uclass_id id = device_get_uclass_id(media);

	log_debug("uclass %d: %s\n", id, uclass_get_name(id));
	if (id == UCLASS_ETH)
		return 0;

	return -ENOTSUPP;
}

int bootflow_iter_check_system(const struct bootflow_iter *iter)
{
	const struct udevice *media = dev_get_parent(iter->dev);
	enum uclass_id id = device_get_uclass_id(media);

	log_debug("uclass %d: %s\n", id, uclass_get_name(id));
	if (id == UCLASS_BOOTSTD)
		return 0;

	return -ENOTSUPP;
}

/**
 * bootflow_cmdline_set() - Set the command line for a bootflow
 *
 * @value: New command-line string
 * Returns 0 if OK, -ENOENT if no current bootflow, -ENOMEM if out of memory
 */
int bootflow_cmdline_set(struct bootflow *bflow, const char *value)
{
	char *cmdline = NULL;

	if (value) {
		cmdline = strdup(value);
		if (!cmdline)
			return -ENOMEM;
	}

	free(bflow->cmdline);
	bflow->cmdline = cmdline;

	return 0;
}

#ifdef CONFIG_BOOTSTD_FULL
/**
 * on_bootargs() - Update the cmdline of a bootflow
 */
static int on_bootargs(const char *name, const char *value, enum env_op op,
		       int flags)
{
	struct bootstd_priv *std;
	struct bootflow *bflow;
	int ret;

	ret = bootstd_get_priv(&std);
	if (ret)
		return 0;
	bflow = std->cur_bootflow;
	if (!bflow)
		return 0;

	switch (op) {
	case env_op_create:
	case env_op_overwrite:
		ret = bootflow_cmdline_set(bflow, value);
		if (ret && ret != ENOENT)
			return 1;
		return 0;
	case env_op_delete:
		bootflow_cmdline_set(bflow, NULL);
		fallthrough;
	default:
		return 0;
	}
}
U_BOOT_ENV_CALLBACK(bootargs, on_bootargs);
#endif

/**
 * copy_in() - Copy a string into a cmdline buffer
 *
 * @buf: Buffer to copy into
 * @end: End of buffer (pointer to char after the end)
 * @arg: String to copy from
 * @len: Number of chars to copy from @arg (note that this is not usually the
 * sane as strlen(arg) since the string may contain following arguments)
 * @new_val: Value to put after arg, or BOOTFLOWCL_EMPTY to use an empty value
 * with no '=' sign
 * Returns: Number of chars written to @buf
 */
static int copy_in(char *buf, char *end, const char *arg, int len,
		   const char *new_val)
{
	char *to = buf;

	/* copy the arg name */
	if (to + len >= end)
		return -E2BIG;
	memcpy(to, arg, len);
	to += len;

	if (new_val == BOOTFLOWCL_EMPTY) {
		/* no value */
	} else {
		bool need_quote = strchr(new_val, ' ');
		len = strlen(new_val);

		/* need space for value, equals sign and maybe two quotes */
		if (to + 1 + (need_quote ? 2 : 0) + len >= end)
			return -E2BIG;
		*to++ = '=';
		if (need_quote)
			*to++ = '"';
		memcpy(to, new_val, len);
		to += len;
		if (need_quote)
			*to++ = '"';
	}

	return to - buf;
}

int cmdline_set_arg(char *buf, int maxlen, const char *cmdline,
		    const char *set_arg, const char *new_val, int *posp)
{
	bool found_arg = false;
	const char *from;
	char *to, *end;
	int set_arg_len;
	char empty = '\0';
	int ret;

	from = cmdline ?: &empty;

	/* check if the value has quotes inside */
	if (new_val && new_val != BOOTFLOWCL_EMPTY && strchr(new_val, '"'))
		return -EBADF;

	set_arg_len = strlen(set_arg);
	for (to = buf, end = buf + maxlen; *from;) {
		const char *val, *arg_end, *val_end, *p;
		bool in_quote;

		if (to >= end)
			return -E2BIG;
		while (*from == ' ')
			from++;
		if (!*from)
			break;

		/* find the end of this arg */
		val = NULL;
		arg_end = NULL;
		val_end = NULL;
		in_quote = false;
		for (p = from;; p++) {
			if (in_quote) {
				if (!*p)
					return -EINVAL;
				if (*p == '"')
					in_quote = false;
				continue;
			}
			if (*p == '=' && !arg_end) {
				arg_end = p;
				val = p + 1;
			} else if (*p == '"') {
				in_quote = true;
			} else if (!*p || *p == ' ') {
				val_end = p;
				if (!arg_end)
					arg_end = p;
				break;
			}
		}
		/*
		 * At this point val_end points to the end of the value, or the
		 * last char after the arg name, if there is no label.
		 * arg_end is the char after the arg name
		 * val points to the value, or NULL if there is none
		 * char after the value.
		 *
		 *        fred=1234
		 *        ^   ^^   ^
		 *      from  ||   |
		 *           / \    \
		 *    arg_end  val   val_end
		 */
		log_debug("from %s arg_end %ld val %ld val_end %ld\n", from,
			  (long)(arg_end - from), (long)(val - from),
			  (long)(val_end - from));

		if (to != buf) {
			if (to >= end)
				return -E2BIG;
			*to++ = ' ';
		}

		/* if this is the target arg, update it */
		if (arg_end - from == set_arg_len &&
		    !strncmp(from, set_arg, set_arg_len)) {
			if (!buf) {
				bool has_quote = val_end[-1] == '"';

				/*
				 * exclude any start/end quotes from
				 * calculations
				 */
				if (!val)
					val = val_end;
				*posp = val - cmdline + has_quote;
				return val_end - val - 2 * has_quote;
			}
			found_arg = true;
			if (!new_val) {
				/* delete this arg */
				from = val_end + (*val_end == ' ');
				log_debug("delete from: %s\n", from);
				if (to != buf)
					to--; /* drop the space we added */
				continue;
			}

			ret = copy_in(to, end, from, arg_end - from, new_val);
			if (ret < 0)
				return ret;
			to += ret;

		/* if not the target arg, copy it unchanged */
		} else if (to) {
			int len;

			len = val_end - from;
			if (to + len >= end)
				return -E2BIG;
			memcpy(to, from, len);
			to += len;
		}
		from = val_end;
	}

	/* If we didn't find the arg, add it */
	if (!found_arg) {
		/* trying to delete something that is not there */
		if (!new_val || !buf)
			return -ENOENT;
		if (to >= end)
			return -E2BIG;

		/* add a space to separate it from the previous arg */
		if (to != buf && to[-1] != ' ')
			*to++ = ' ';
		ret = copy_in(to, end, set_arg, set_arg_len, new_val);
		log_debug("ret=%d, to: %s buf: %s\n", ret, to, buf);
		if (ret < 0)
			return ret;
		to += ret;
	}

	/* delete any trailing space */
	if (to > buf && to[-1] == ' ')
		to--;

	if (to >= end)
		return -E2BIG;
	*to++ = '\0';

	return to - buf;
}

int bootflow_cmdline_set_arg(struct bootflow *bflow, const char *set_arg,
			     const char *new_val, bool set_env)
{
	char buf[2048];
	char *cmd = NULL;
	int ret;

	ret = cmdline_set_arg(buf, sizeof(buf), bflow->cmdline, set_arg,
			      new_val, NULL);
	if (ret < 0)
		return ret;

	ret = bootflow_cmdline_set(bflow, buf);
	if (*buf) {
		cmd = strdup(buf);
		if (!cmd)
			return -ENOMEM;
	}
	free(bflow->cmdline);
	bflow->cmdline = cmd;

	if (set_env) {
		ret = env_set("bootargs", bflow->cmdline);
		if (ret)
			return ret;
	}

	return 0;
}

int cmdline_get_arg(const char *cmdline, const char *arg, int *posp)
{
	int ret;

	ret = cmdline_set_arg(NULL, 1, cmdline, arg, NULL, posp);

	return ret;
}

int bootflow_cmdline_get_arg(struct bootflow *bflow, const char *arg,
			     const char **val)
{
	int ret;
	int pos;

	ret = cmdline_get_arg(bflow->cmdline, arg, &pos);
	if (ret < 0)
		return ret;
	*val = bflow->cmdline + pos;

	return ret;
}

int bootflow_cmdline_auto(struct bootflow *bflow, const char *arg)
{
	struct serial_device_info info;
	char buf[50];
	int ret;

	ret = serial_getinfo(gd->cur_serial_dev, &info);
	if (ret)
		return ret;

	*buf = '\0';
	if (!strcmp("earlycon", arg)) {
		snprintf(buf, sizeof(buf),
			 "uart8250,mmio32,%#lx,%dn8", info.addr,
			 info.baudrate);
	} else if (!strcmp("console", arg)) {
		snprintf(buf, sizeof(buf),
			 "ttyS0,%dn8", info.baudrate);
	}

	if (!*buf) {
		printf("Unknown param '%s\n", arg);
		return -ENOENT;
	}

	ret = bootflow_cmdline_set_arg(bflow, arg, buf, true);
	if (ret)
		return ret;

	return 0;
}
