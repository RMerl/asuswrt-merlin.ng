// SPDX-License-Identifier: GPL-2.0+
/*
 * Logging support
 *
 * Copyright (c) 2017 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <display_options.h>
#include <log.h>
#include <malloc.h>
#include <asm/global_data.h>
#include <dm/uclass.h>

DECLARE_GLOBAL_DATA_PTR;

static const char *const log_cat_name[] = {
	"none",
	"arch",
	"board",
	"core",
	"driver-model",
	"device-tree",
	"efi",
	"alloc",
	"sandbox",
	"bloblist",
	"devres",
	"acpi",
	"boot",
	"event",
	"fs",
	"expo",
};

_Static_assert(ARRAY_SIZE(log_cat_name) == LOGC_COUNT - LOGC_NONE,
	       "log_cat_name size");

static const char *const log_level_name[] = {
	"EMERG",
	"ALERT",
	"CRIT",
	"ERR",
	"WARNING",
	"NOTICE",
	"INFO",
	"DEBUG",
	"CONTENT",
	"IO",
};

_Static_assert(ARRAY_SIZE(log_level_name) == LOGL_COUNT, "log_level_name size");

/* All error responses MUST begin with '<' */
const char *log_get_cat_name(enum log_category_t cat)
{
	const char *name;

	if (cat < 0 || cat >= LOGC_COUNT)
		return "<invalid>";
	if (cat >= LOGC_NONE)
		return log_cat_name[cat - LOGC_NONE];

#if CONFIG_IS_ENABLED(DM)
	name = uclass_get_name((enum uclass_id)cat);
#else
	name = NULL;
#endif

	return name ? name : "<missing>";
}

enum log_category_t log_get_cat_by_name(const char *name)
{
	enum uclass_id id;
	int i;

	for (i = LOGC_NONE; i < LOGC_COUNT; i++)
		if (!strcmp(name, log_cat_name[i - LOGC_NONE]))
			return i;
	id = uclass_get_by_name(name);
	if (id != UCLASS_INVALID)
		return (enum log_category_t)id;

	return LOGC_NONE;
}

const char *log_get_level_name(enum log_level_t level)
{
	if (level >= LOGL_COUNT)
		return "INVALID";
	return log_level_name[level];
}

enum log_level_t log_get_level_by_name(const char *name)
{
	int i;

	for (i = 0; i < LOGL_COUNT; i++) {
		if (!strcasecmp(log_level_name[i], name))
			return i;
	}

	return LOGL_NONE;
}

struct log_device *log_device_find_by_name(const char *drv_name)
{
	struct log_device *ldev;

	list_for_each_entry(ldev, &gd->log_head, sibling_node) {
		if (!strcmp(drv_name, ldev->drv->name))
			return ldev;
	}

	return NULL;
}

bool log_has_cat(enum log_category_t cat_list[], enum log_category_t cat)
{
	int i;

	for (i = 0; i < LOGF_MAX_CATEGORIES && cat_list[i] != LOGC_END; i++) {
		if (cat_list[i] == cat)
			return true;
	}

	return false;
}

bool log_has_file(const char *file_list, const char *file)
{
	int file_len = strlen(file);
	const char *s, *p;
	int substr_len;

	for (s = file_list; *s; s = p + (*p != '\0')) {
		p = strchrnul(s, ',');
		substr_len = p - s;
		if (file_len >= substr_len &&
		    !strncmp(file + file_len - substr_len, s, substr_len))
			return true;
	}

	return false;
}

/**
 * log_passes_filters() - check if a log record passes the filters for a device
 *
 * @ldev: Log device to check
 * @rec: Log record to check
 * Return: true if @rec is not blocked by the filters in @ldev, false if it is
 */
static bool log_passes_filters(struct log_device *ldev, struct log_rec *rec)
{
	struct log_filter *filt;

	if (rec->flags & LOGRECF_FORCE_DEBUG)
		return true;

	/* If there are no filters, filter on the default log level */
	if (list_empty(&ldev->filter_head)) {
		if (rec->level > gd->default_log_level)
			return false;
		return true;
	}

	list_for_each_entry(filt, &ldev->filter_head, sibling_node) {
		if (filt->flags & LOGFF_LEVEL_MIN) {
			if (rec->level < filt->level)
				continue;
		} else if (rec->level > filt->level) {
			continue;
		}

		if ((filt->flags & LOGFF_HAS_CAT) &&
		    !log_has_cat(filt->cat_list, rec->cat))
			continue;

		if (filt->file_list &&
		    !log_has_file(filt->file_list, rec->file))
			continue;

		if (filt->flags & LOGFF_DENY)
			return false;
		else
			return true;
	}

	return false;
}

/**
 * log_dispatch() - Send a log record to all log devices for processing
 *
 * The log record is sent to each log device in turn, skipping those which have
 * filters which block the record.
 *
 * All log messages created while processing log record @rec are ignored.
 *
 * @rec:	log record to dispatch
 * Return:	0 msg sent, 1 msg not sent while already dispatching another msg
 */
static int log_dispatch(struct log_rec *rec, const char *fmt, va_list args)
{
	struct log_device *ldev;
	char buf[CONFIG_SYS_CBSIZE];

	/*
	 * When a log driver writes messages (e.g. via the network stack) this
	 * may result in further generated messages. We cannot process them here
	 * as this might result in infinite recursion.
	 */
	if (gd->processing_msg)
		return 1;

	/* Emit message */
	gd->processing_msg = true;
	list_for_each_entry(ldev, &gd->log_head, sibling_node) {
		if ((ldev->flags & LOGDF_ENABLE) &&
		    log_passes_filters(ldev, rec)) {
			if (!rec->msg) {
				int len;

				len = vsnprintf(buf, sizeof(buf), fmt, args);
				rec->msg = buf;
				gd->log_cont = len && buf[len - 1] != '\n';
			}
			ldev->drv->emit(ldev, rec);
		}
	}
	gd->processing_msg = false;
	return 0;
}

int _log(enum log_category_t cat, enum log_level_t level, const char *file,
	 int line, const char *func, const char *fmt, ...)
{
	struct log_rec rec;
	va_list args;

	if (!gd)
		return -ENOSYS;

	/* Check for message continuation */
	if (cat == LOGC_CONT)
		cat = gd->logc_prev;
	if (level == LOGL_CONT)
		level = gd->logl_prev;

	rec.cat = cat;
	rec.level = level & LOGL_LEVEL_MASK;
	rec.flags = 0;
	if (level & LOGL_FORCE_DEBUG)
		rec.flags |= LOGRECF_FORCE_DEBUG;
	if (gd->log_cont)
		rec.flags |= LOGRECF_CONT;
	rec.file = file;
	rec.line = line;
	rec.func = func;
	rec.msg = NULL;

	if (!(gd->flags & GD_FLG_LOG_READY)) {
		gd->log_drop_count++;

		/* display dropped traces with console puts and DEBUG_UART */
		if (rec.level <= CONFIG_LOG_DEFAULT_LEVEL ||
		    rec.flags & LOGRECF_FORCE_DEBUG) {
			char buf[CONFIG_SYS_CBSIZE];

			va_start(args, fmt);
			vsnprintf(buf, sizeof(buf), fmt, args);
			puts(buf);
			va_end(args);
		}

		return -ENOSYS;
	}
	va_start(args, fmt);
	if (!log_dispatch(&rec, fmt, args)) {
		gd->logc_prev = cat;
		gd->logl_prev = level;
	}
	va_end(args);

	return 0;
}

#define MAX_LINE_LENGTH_BYTES		64
#define DEFAULT_LINE_LENGTH_BYTES	16

int _log_buffer(enum log_category_t cat, enum log_level_t level,
		const char *file, int line, const char *func, ulong addr,
		const void *data, uint width, uint count, uint linelen)
{
	if (linelen * width > MAX_LINE_LENGTH_BYTES)
		linelen = MAX_LINE_LENGTH_BYTES / width;
	if (linelen < 1)
		linelen = DEFAULT_LINE_LENGTH_BYTES / width;

	while (count) {
		uint thislinelen;
		char buf[HEXDUMP_MAX_BUF_LENGTH(width * linelen)];

		thislinelen = hexdump_line(addr, data, width, count, linelen,
					   buf, sizeof(buf));
		assert(thislinelen >= 0);
		_log(cat, level, file, line, func, "%s\n", buf);

		/* update references */
		data += thislinelen * width;
		addr += thislinelen * width;
		count -= thislinelen;
	}

	return 0;
}

int log_add_filter_flags(const char *drv_name, enum log_category_t cat_list[],
			 enum log_level_t level, const char *file_list,
			 int flags)
{
	struct log_filter *filt;
	struct log_device *ldev;
	int ret;
	int i;

	ldev = log_device_find_by_name(drv_name);
	if (!ldev)
		return -ENOENT;
	filt = calloc(1, sizeof(*filt));
	if (!filt)
		return -ENOMEM;

	filt->flags = flags;
	if (cat_list) {
		filt->flags |= LOGFF_HAS_CAT;
		for (i = 0; ; i++) {
			if (i == ARRAY_SIZE(filt->cat_list)) {
				ret = -ENOSPC;
				goto err;
			}
			filt->cat_list[i] = cat_list[i];
			if (cat_list[i] == LOGC_END)
				break;
		}
	}
	filt->level = level;
	if (file_list) {
		filt->file_list = strdup(file_list);
		if (!filt->file_list) {
			ret = -ENOMEM;
			goto err;
		}
	}
	filt->filter_num = ldev->next_filter_num++;
	/* Add deny filters to the beginning of the list */
	if (flags & LOGFF_DENY)
		list_add(&filt->sibling_node, &ldev->filter_head);
	else
		list_add_tail(&filt->sibling_node, &ldev->filter_head);

	return filt->filter_num;

err:
	free(filt);
	return ret;
}

int log_remove_filter(const char *drv_name, int filter_num)
{
	struct log_filter *filt;
	struct log_device *ldev;

	ldev = log_device_find_by_name(drv_name);
	if (!ldev)
		return -ENOENT;

	list_for_each_entry(filt, &ldev->filter_head, sibling_node) {
		if (filt->filter_num == filter_num) {
			list_del(&filt->sibling_node);
			free(filt);

			return 0;
		}
	}

	return -ENOENT;
}

/**
 * log_find_device_by_drv() - Find a device by its driver
 *
 * @drv: Log driver
 * Return: Device associated with that driver, or NULL if not found
 */
static struct log_device *log_find_device_by_drv(struct log_driver *drv)
{
	struct log_device *ldev;

	list_for_each_entry(ldev, &gd->log_head, sibling_node) {
		if (ldev->drv == drv)
			return ldev;
	}
	/*
	 * It is quite hard to pass an invalid driver since passing an unknown
	 * LOG_GET_DRIVER(xxx) would normally produce a compilation error. But
	 * it is possible to pass NULL, for example, so this
	 */

	return NULL;
}

int log_device_set_enable(struct log_driver *drv, bool enable)
{
	struct log_device *ldev;

	ldev = log_find_device_by_drv(drv);
	if (!ldev)
		return -ENOENT;
	if (enable)
		ldev->flags |= LOGDF_ENABLE;
	else
		ldev->flags &= ~LOGDF_ENABLE;

	return 0;
}

int log_init(void)
{
	struct log_driver *drv = ll_entry_start(struct log_driver, log_driver);
	const int count = ll_entry_count(struct log_driver, log_driver);
	struct log_driver *end = drv + count;

	/*
	 * We cannot add runtime data to the driver since it is likely stored
	 * in rodata. Instead, set up a 'device' corresponding to each driver.
	 * We only support having a single device for each driver.
	 */
	INIT_LIST_HEAD((struct list_head *)&gd->log_head);
	while (drv < end) {
		struct log_device *ldev;

		ldev = calloc(1, sizeof(*ldev));
		if (!ldev) {
			debug("%s: Cannot allocate memory\n", __func__);
			return -ENOMEM;
		}
		INIT_LIST_HEAD(&ldev->filter_head);
		ldev->drv = drv;
		ldev->flags = drv->flags;
		list_add_tail(&ldev->sibling_node,
			      (struct list_head *)&gd->log_head);
		drv++;
	}
	gd->flags |= GD_FLG_LOG_READY;
	if (!gd->default_log_level)
		gd->default_log_level = CONFIG_LOG_DEFAULT_LEVEL;
	gd->log_fmt = log_get_default_format();
	gd->logc_prev = LOGC_NONE;
	gd->logl_prev = LOGL_INFO;

	return 0;
}
