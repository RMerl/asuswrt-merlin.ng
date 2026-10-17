// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2007 Semihalf
 *
 * Written by: Rafal Jaworowski <raj@semihalf.com>
 */

#include <config.h>
#include <command.h>
#include <common.h>
#include <env.h>
#include <malloc.h>
#include <env_internal.h>
#include <linux/delay.h>
#include <linux/types.h>
#include <api_public.h>
#include <u-boot/crc.h>

#include "api_private.h"

#define DEBUG
#undef DEBUG

/*****************************************************************************
 *
 * This is the API core.
 *
 * API_ functions are part of U-Boot code and constitute the lowest level
 * calls:
 *
 *  - they know what values they need as arguments
 *  - their direct return value pertains to the API_ "shell" itself (0 on
 *    success, some error code otherwise)
 *  - if the call returns a value it is buried within arguments
 *
 ****************************************************************************/

#ifdef DEBUG
#define debugf(fmt, args...) do { printf("%s(): ", __func__); printf(fmt, ##args); } while (0)
#else
#define debugf(fmt, args...)
#endif

typedef	int (*cfp_t)(va_list argp);

static int calls_no;

/*
 * pseudo signature:
 *
 * int API_getc(int *c)
 */
static int API_getc(va_list ap)
{
	int *c;

	if ((c = (int *)va_arg(ap, uintptr_t)) == NULL)
		return API_EINVAL;

	*c = getchar();
	return 0;
}

/*
 * pseudo signature:
 *
 * int API_tstc(int *c)
 */
static int API_tstc(va_list ap)
{
	int *t;

	if ((t = (int *)va_arg(ap, uintptr_t)) == NULL)
		return API_EINVAL;

	*t = tstc();
	return 0;
}

/*
 * pseudo signature:
 *
 * int API_putc(char *ch)
 */
static int API_putc(va_list ap)
{
	char *c;

	if ((c = (char *)va_arg(ap, uintptr_t)) == NULL)
		return API_EINVAL;

	putc(*c);
	return 0;
}

/*
 * pseudo signature:
 *
 * int API_puts(char **s)
 */
static int API_puts(va_list ap)
{
	char *s;

	if ((s = (char *)va_arg(ap, uintptr_t)) == NULL)
		return API_EINVAL;

	puts(s);
	return 0;
}

/*
 * pseudo signature:
 *
 * int API_reset(void)
 */
static int API_reset(va_list ap)
{
	do_reset(NULL, 0, 0, NULL);

	/* NOT REACHED */
	return 0;
}

/*
 * pseudo signature:
 *
 * int API_get_sys_info(struct sys_info *si)
 *
 * fill out the sys_info struct containing selected parameters about the
 * machine
 */
static int API_get_sys_info(va_list ap)
{
	struct sys_info *si;

	si = (struct sys_info *)va_arg(ap, uintptr_t);
	if (si == NULL)
		return API_ENOMEM;

	return (platform_sys_info(si)) ? 0 : API_ENODEV;
}

/*
 * pseudo signature:
 *
 * int API_udelay(unsigned long *udelay)
 */
static int API_udelay(va_list ap)
{
	unsigned long *d;

	if ((d = (unsigned long *)va_arg(ap, unsigned long)) == NULL)
		return API_EINVAL;

	udelay(*d);
	return 0;
}

/*
 * pseudo signature:
 *
 * int API_get_timer(unsigned long *current, unsigned long *base)
 */
static int API_get_timer(va_list ap)
{
	unsigned long *base, *cur;

	cur = (unsigned long *)va_arg(ap, unsigned long);
	if (cur == NULL)
		return API_EINVAL;

	base = (unsigned long *)va_arg(ap, unsigned long);
	if (base == NULL)
		return API_EINVAL;

	*cur = get_timer(*base);
	return 0;
}


/*****************************************************************************
 *
 * pseudo signature:
 *
 * int API_dev_enum(struct device_info *)
 *
 *
 * cookies uniqely identify the previously enumerated device instance and
 * provide a hint for what to inspect in current enum iteration:
 *
 *   - net: &eth_device struct address from list pointed to by eth_devices
 *
 *   - storage: struct blk_desc struct address from &ide_dev_desc[n],
 *     &scsi_dev_desc[n] and similar tables
 *
 ****************************************************************************/

static int API_dev_enum(va_list ap)
{
	struct device_info *di;

	/* arg is ptr to the device_info struct we are going to fill out */
	di = (struct device_info *)va_arg(ap, uintptr_t);
	if (di == NULL)
		return API_EINVAL;

	if (di->cookie == NULL) {
		/* start over - clean up enumeration */
		dev_enum_reset();	/* XXX shouldn't the name contain 'stor'? */
		debugf("RESTART ENUM\n");

		/* net device enumeration first */
		if (dev_enum_net(di))
			return 0;
	}

	/*
	 * The hidden assumption is there can only be one active network
	 * device and it is identified upon enumeration (re)start, so there's
	 * no point in trying to find network devices in other cases than the
	 * (re)start and hence the 'next' device can only be storage
	 */
	if (!dev_enum_storage(di))
		/* make sure we mark there are no more devices */
		di->cookie = NULL;

	return 0;
}


static int API_dev_open(va_list ap)
{
	struct device_info *di;
	int err = 0;

	/* arg is ptr to the device_info struct */
	di = (struct device_info *)va_arg(ap, uintptr_t);
	if (di == NULL)
		return API_EINVAL;

	/* Allow only one consumer of the device at a time */
	if (di->state == DEV_STA_OPEN)
		return API_EBUSY;

	if (di->cookie == NULL)
		return API_ENODEV;

	if (di->type & DEV_TYP_STOR)
		err = dev_open_stor(di->cookie);

	else if (di->type & DEV_TYP_NET)
		err = dev_open_net(di->cookie);
	else
		err = API_ENODEV;

	if (!err)
		di->state = DEV_STA_OPEN;

	return err;
}


static int API_dev_close(va_list ap)
{
	struct device_info *di;
	int err = 0;

	/* arg is ptr to the device_info struct */
	di = (struct device_info *)va_arg(ap, uintptr_t);
	if (di == NULL)
		return API_EINVAL;

	if (di->state == DEV_STA_CLOSED)
		return 0;

	if (di->cookie == NULL)
		return API_ENODEV;

	if (di->type & DEV_TYP_STOR)
		err = dev_close_stor(di->cookie);

	else if (di->type & DEV_TYP_NET)
		err = dev_close_net(di->cookie);
	else
		/*
		 * In case of unknown device we cannot change its state, so
		 * only return error code
		 */
		err = API_ENODEV;

	if (!err)
		di->state = DEV_STA_CLOSED;

	return err;
}


/*
 * pseudo signature:
 *
 * int API_dev_write(
 *	struct device_info *di,
 *	void *buf,
 *	int *len,
 *	unsigned long *start
 * )
 *
 * buf:	ptr to buffer from where to get the data to send
 *
 * len: ptr to length to be read
 *      - network: len of packet to be sent (in bytes)
 *      - storage: # of blocks to write (can vary in size depending on define)
 *
 * start: ptr to start block (only used for storage devices, ignored for
 *        network)
 */
static int API_dev_write(va_list ap)
{
	struct device_info *di;
	void *buf;
	lbasize_t *len_stor, act_len_stor;
	lbastart_t *start;
	int *len_net;
	int err = 0;

	/* 1. arg is ptr to the device_info struct */
	di = (struct device_info *)va_arg(ap, uintptr_t);
	if (di == NULL)
		return API_EINVAL;

	/* XXX should we check if device is open? i.e. the ->state ? */

	if (di->cookie == NULL)
		return API_ENODEV;

	/* 2. arg is ptr to buffer from where to get data to write */
	buf = (void *)va_arg(ap, uintptr_t);
	if (buf == NULL)
		return API_EINVAL;

	if (di->type & DEV_TYP_STOR) {
		/* 3. arg - ptr to var with # of blocks to write */
		len_stor = (lbasize_t *)va_arg(ap, uintptr_t);
		if (!len_stor)
			return API_EINVAL;
		if (*len_stor <= 0)
			return API_EINVAL;

		/* 4. arg - ptr to var with start block */
		start = (lbastart_t *)va_arg(ap, uintptr_t);

		act_len_stor = dev_write_stor(di->cookie, buf, *len_stor, *start);
		if (act_len_stor != *len_stor) {
			debugf("write @ %llu: done %llu out of %llu blocks",
				   (uint64_t)blk, (uint64_t)act_len_stor,
				   (uint64_t)len_stor);
			return API_EIO;
		}

	} else if (di->type & DEV_TYP_NET) {
		/* 3. arg points to the var with length of packet to write */
		len_net = (int *)va_arg(ap, uintptr_t);
		if (!len_net)
			return API_EINVAL;
		if (*len_net <= 0)
			return API_EINVAL;

		err = dev_write_net(di->cookie, buf, *len_net);

	} else
		err = API_ENODEV;

	return err;
}


/*
 * pseudo signature:
 *
 * int API_dev_read(
 *	struct device_info *di,
 *	void *buf,
 *	size_t *len,
 *	unsigned long *start
 *	size_t *act_len
 * )
 *
 * buf:	ptr to buffer where to put the read data
 *
 * len: ptr to length to be read
 *      - network: len of packet to read (in bytes)
 *      - storage: # of blocks to read (can vary in size depending on define)
 *
 * start: ptr to start block (only used for storage devices, ignored for
 *        network)
 *
 * act_len: ptr to where to put the len actually read
 */
static int API_dev_read(va_list ap)
{
	struct device_info *di;
	void *buf;
	lbasize_t *len_stor, *act_len_stor;
	lbastart_t *start;
	int *len_net, *act_len_net;

	/* 1. arg is ptr to the device_info struct */
	di = (struct device_info *)va_arg(ap, uintptr_t);
	if (di == NULL)
		return API_EINVAL;

	/* XXX should we check if device is open? i.e. the ->state ? */

	if (di->cookie == NULL)
		return API_ENODEV;

	/* 2. arg is ptr to buffer from where to put the read data */
	buf = (void *)va_arg(ap, uintptr_t);
	if (buf == NULL)
		return API_EINVAL;

	if (di->type & DEV_TYP_STOR) {
		/* 3. arg - ptr to var with # of blocks to read */
		len_stor = (lbasize_t *)va_arg(ap, uintptr_t);
		if (!len_stor)
			return API_EINVAL;
		if (*len_stor <= 0)
			return API_EINVAL;

		/* 4. arg - ptr to var with start block */
		start = (lbastart_t *)va_arg(ap, uintptr_t);

		/* 5. arg - ptr to var where to put the len actually read */
		act_len_stor = (lbasize_t *)va_arg(ap, uintptr_t);
		if (!act_len_stor)
			return API_EINVAL;

		*act_len_stor = dev_read_stor(di->cookie, buf, *len_stor, *start);

	} else if (di->type & DEV_TYP_NET) {

		/* 3. arg points to the var with length of packet to read */
		len_net = (int *)va_arg(ap, uintptr_t);
		if (!len_net)
			return API_EINVAL;
		if (*len_net <= 0)
			return API_EINVAL;

		/* 4. - ptr to var where to put the len actually read */
		act_len_net = (int *)va_arg(ap, uintptr_t);
		if (!act_len_net)
			return API_EINVAL;

		*act_len_net = dev_read_net(di->cookie, buf, *len_net);

	} else
		return API_ENODEV;

	return 0;
}


/*
 * pseudo signature:
 *
 * int API_env_get(const char *name, char **value)
 *
 * name: ptr to name of env var
 */
static int API_env_get(va_list ap)
{
	char *name, **value;

	if ((name = (char *)va_arg(ap, uintptr_t)) == NULL)
		return API_EINVAL;
	if ((value = (char **)va_arg(ap, uintptr_t)) == NULL)
		return API_EINVAL;

	*value = env_get(name);

	return 0;
}

/*
 * pseudo signature:
 *
 * int API_env_set(const char *name, const char *value)
 *
 * name: ptr to name of env var
 *
 * value: ptr to value to be set
 */
static int API_env_set(va_list ap)
{
	char *name, *value;

	if ((name = (char *)va_arg(ap, uintptr_t)) == NULL)
		return API_EINVAL;
	if ((value = (char *)va_arg(ap, uintptr_t)) == NULL)
		return API_EINVAL;

	env_set(name, value);

	return 0;
}

/*
 * pseudo signature:
 *
 * int API_env_enum(const char *last, char **next)
 *
 * last: ptr to name of env var found in last iteration
 */
static int API_env_enum(va_list ap)
{
	int i, buflen;
	char *last, **next, *s;
	struct env_entry *match, search;
	static char *var;

	last = (char *)va_arg(ap, unsigned long);

	if ((next = (char **)va_arg(ap, uintptr_t)) == NULL)
		return API_EINVAL;

	if (last == NULL) {
		var = NULL;
		i = 0;
	} else {
		var = strdup(last);
		s = strchr(var, '=');
		if (s != NULL)
			*s = 0;
		search.key = var;
		i = hsearch_r(search, ENV_FIND, &match, &env_htab, 0);
		if (i == 0) {
			i = API_EINVAL;
			goto done;
		}
	}

	/* match the next entry after i */
	i = hmatch_r("", i, &match, &env_htab);
	if (i == 0)
		goto done;
	buflen = strlen(match->key) + strlen(match->data) + 2;
	var = realloc(var, buflen);
	snprintf(var, buflen, "%s=%s", match->key, match->data);
	*next = var;
	return 0;

done:
	free(var);
	var = NULL;
	*next = NULL;
	return i;
}

/*
 * pseudo signature:
 *
 * int API_display_get_info(int type, struct display_info *di)
 */
static int API_display_get_info(va_list ap)
{
	int type;
	struct display_info *di;

	type = va_arg(ap, int);
	di = va_arg(ap, struct display_info *);

	return display_get_info(type, di);
}

/*
 * pseudo signature:
 *
 * int API_display_draw_bitmap(ulong bitmap, int x, int y)
 */
static int API_display_draw_bitmap(va_list ap)
{
	ulong bitmap;
	int x, y;

	bitmap = va_arg(ap, ulong);
	x = va_arg(ap, int);
	y = va_arg(ap, int);

	return display_draw_bitmap(bitmap, x, y);
}

/*
 * pseudo signature:
 *
 * void API_display_clear(void)
 */
static int API_display_clear(va_list ap)
{
	display_clear();
	return 0;
}

static cfp_t calls_table[API_MAXCALL] = { NULL, };

/*
 * The main syscall entry point - this is not reentrant, only one call is
 * serviced until finished.
 *
 * e.g. syscall(1, int *, u_int32_t, u_int32_t, u_int32_t, u_int32_t);
 *
 * call:	syscall number
 *
 * retval:	points to the return value placeholder, this is the place the
 *		syscall puts its return value, if NULL the caller does not
 *		expect a return value
 *
 * ...		syscall arguments (variable number)
 *
 * returns:	0 if the call not found, 1 if serviced
 */
int syscall(int call, int *retval, ...)
{
	va_list	ap;
	int rv;

	if (call < 0 || call >= calls_no) {
		debugf("invalid call #%d\n", call);
		return 0;
	}

	if (calls_table[call] == NULL) {
		debugf("syscall #%d does not have a handler\n", call);
		return 0;
	}

	va_start(ap, retval);
	rv = calls_table[call](ap);
	if (retval != NULL)
		*retval = rv;

	return 1;
}

int api_init(void)
{
	struct api_signature *sig;

	/* TODO put this into linker set one day... */
	calls_table[API_RSVD] = NULL;
	calls_table[API_GETC] = &API_getc;
	calls_table[API_PUTC] = &API_putc;
	calls_table[API_TSTC] = &API_tstc;
	calls_table[API_PUTS] = &API_puts;
	calls_table[API_RESET] = &API_reset;
	calls_table[API_GET_SYS_INFO] = &API_get_sys_info;
	calls_table[API_UDELAY] = &API_udelay;
	calls_table[API_GET_TIMER] = &API_get_timer;
	calls_table[API_DEV_ENUM] = &API_dev_enum;
	calls_table[API_DEV_OPEN] = &API_dev_open;
	calls_table[API_DEV_CLOSE] = &API_dev_close;
	calls_table[API_DEV_READ] = &API_dev_read;
	calls_table[API_DEV_WRITE] = &API_dev_write;
	calls_table[API_ENV_GET] = &API_env_get;
	calls_table[API_ENV_SET] = &API_env_set;
	calls_table[API_ENV_ENUM] = &API_env_enum;
	calls_table[API_DISPLAY_GET_INFO] = &API_display_get_info;
	calls_table[API_DISPLAY_DRAW_BITMAP] = &API_display_draw_bitmap;
	calls_table[API_DISPLAY_CLEAR] = &API_display_clear;
	calls_no = API_MAXCALL;

	debugf("API initialized with %d calls\n", calls_no);

	dev_stor_init();

	/*
	 * Produce the signature so the API consumers can find it
	 */
	sig = malloc(sizeof(struct api_signature));
	if (sig == NULL) {
		printf("API: could not allocate memory for the signature!\n");
		return -ENOMEM;
	}

	env_set_hex("api_address", (unsigned long)sig);
	debugf("API sig @ 0x%lX\n", (unsigned long)sig);
	memcpy(sig->magic, API_SIG_MAGIC, 8);
	sig->version = API_SIG_VERSION;
	sig->syscall = &syscall;
	sig->checksum = 0;
	sig->checksum = crc32(0, (unsigned char *)sig,
			      sizeof(struct api_signature));
	debugf("syscall entry: 0x%lX\n", (unsigned long)sig->syscall);

	return 0;
}

void platform_set_mr(struct sys_info *si, unsigned long start, unsigned long size,
			int flags)
{
	int i;

	if (!si->mr || !size || (flags == 0))
		return;

	/* find free slot */
	for (i = 0; i < si->mr_no; i++)
		if (si->mr[i].flags == 0) {
			/* insert new mem region */
			si->mr[i].start = start;
			si->mr[i].size = size;
			si->mr[i].flags = flags;
			return;
		}
}
