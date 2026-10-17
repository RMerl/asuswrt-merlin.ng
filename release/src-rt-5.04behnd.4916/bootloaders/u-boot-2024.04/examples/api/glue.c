// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2007-2008 Semihalf, Rafal Jaworowski <raj@semihalf.com>
 */

#include <common.h>
#include <env.h>
#include <linux/types.h>
#include <api_public.h>
#include <u-boot/crc.h>

#include "glue.h"

static int valid_sig(struct api_signature *sig)
{
	uint32_t checksum;
	struct api_signature s;

	if (sig == NULL)
		return 0;
	/*
	 * Clear the checksum field (in the local copy) so as to calculate the
	 * CRC with the same initial contents as at the time when the sig was
	 * produced
	 */
	s = *sig;
	s.checksum = 0;

	checksum = crc32(0, (unsigned char *)&s, sizeof(struct api_signature));

	if (checksum != sig->checksum)
		return 0;

	return 1;
}

/*
 * Searches for the U-Boot API signature
 *
 * returns 1/0 depending on found/not found result
 */
int api_search_sig(struct api_signature **sig)
{
	unsigned char *sp;
	uint32_t search_start = 0;
	uint32_t search_end = 0;

	if (sig == NULL)
		return 0;

	if (search_hint == 0)
		search_hint = 255 * 1024 * 1024;

	search_start = search_hint & ~0x000fffff;
	search_end = search_start + API_SEARCH_LEN - API_SIG_MAGLEN;

	sp = (unsigned char *)search_start;
	while ((sp + API_SIG_MAGLEN) < (unsigned char *)search_end) {
		if (!memcmp(sp, API_SIG_MAGIC, API_SIG_MAGLEN)) {
			*sig = (struct api_signature *)sp;
			if (valid_sig(*sig))
				return 1;
		}
		sp += API_SIG_MAGLEN;
	}

	*sig = NULL;
	return 0;
}

/****************************************
 *
 * console
 *
 ****************************************/

int ub_getc(void)
{
	int c;

	if (!syscall(API_GETC, NULL, &c))
		return -1;

	return c;
}

int ub_tstc(void)
{
	int t;

	if (!syscall(API_TSTC, NULL, &t))
		return -1;

	return t;
}

void ub_putc(char c)
{
	syscall(API_PUTC, NULL, &c);
}

void ub_puts(const char *s)
{
	syscall(API_PUTS, NULL, s);
}

/****************************************
 *
 * system
 *
 ****************************************/

void ub_reset(void)
{
	syscall(API_RESET, NULL);
}

static struct mem_region mr[UB_MAX_MR];
static struct sys_info si;

struct sys_info * ub_get_sys_info(void)
{
	int err = 0;

	memset(&si, 0, sizeof(struct sys_info));
	si.mr = mr;
	si.mr_no = UB_MAX_MR;
	memset(&mr, 0, sizeof(mr));

	if (!syscall(API_GET_SYS_INFO, &err, &si))
		return NULL;

	return ((err) ? NULL : &si);
}

/****************************************
 *
 * timing
 *
 ****************************************/

void ub_udelay(unsigned long usec)
{
	syscall(API_UDELAY, NULL, &usec);
}

unsigned long ub_get_timer(unsigned long base)
{
	unsigned long cur;

	if (!syscall(API_GET_TIMER, NULL, &cur, &base))
		return 0;

	return cur;
}


/****************************************************************************
 *
 * devices
 *
 * Devices are identified by handles: numbers 0, 1, 2, ..., UB_MAX_DEV-1
 *
 ***************************************************************************/

static struct device_info devices[UB_MAX_DEV];

struct device_info * ub_dev_get(int i)
{
	return ((i < 0 || i >= UB_MAX_DEV) ? NULL : &devices[i]);
}

/*
 * Enumerates the devices: fills out device_info elements in the devices[]
 * array.
 *
 * returns:		number of devices found
 */
int ub_dev_enum(void)
{
	struct device_info *di;
	int n = 0;

	memset(&devices, 0, sizeof(struct device_info) * UB_MAX_DEV);
	di = &devices[0];

	if (!syscall(API_DEV_ENUM, NULL, di))
		return 0;

	while (di->cookie != NULL) {

		if (++n >= UB_MAX_DEV)
			break;

		/* take another device_info */
		di++;

		/* pass on the previous cookie */
		di->cookie = devices[n - 1].cookie;

		if (!syscall(API_DEV_ENUM, NULL, di))
			return 0;
	}

	return n;
}

/*
 * handle:	0-based id of the device
 *
 * returns:	0 when OK, err otherwise
 */
int ub_dev_open(int handle)
{
	struct device_info *di;
	int err = 0;

	if (handle < 0 || handle >= UB_MAX_DEV)
		return API_EINVAL;

	di = &devices[handle];

	if (!syscall(API_DEV_OPEN, &err, di))
		return -1;

	return err;
}

int ub_dev_close(int handle)
{
	struct device_info *di;

	if (handle < 0 || handle >= UB_MAX_DEV)
		return API_EINVAL;

	di = &devices[handle];
	if (!syscall(API_DEV_CLOSE, NULL, di))
		return -1;

	return 0;
}

/*
 *
 * Validates device for read/write, it has to:
 *
 * - have sane handle
 * - be opened
 *
 * returns:	0/1 accordingly
 */
static int dev_valid(int handle)
{
	if (handle < 0 || handle >= UB_MAX_DEV)
		return 0;

	if (devices[handle].state != DEV_STA_OPEN)
		return 0;

	return 1;
}

static int dev_stor_valid(int handle)
{
	if (!dev_valid(handle))
		return 0;

	if (!(devices[handle].type & DEV_TYP_STOR))
		return 0;

	return 1;
}

int ub_dev_read(int handle, void *buf, lbasize_t len, lbastart_t start,
		lbasize_t *rlen)
{
	struct device_info *di;
	lbasize_t act_len;
	int err = 0;

	if (!dev_stor_valid(handle))
		return API_ENODEV;

	di = &devices[handle];
	if (!syscall(API_DEV_READ, &err, di, buf, &len, &start, &act_len))
		return API_ESYSC;

	if (!err && rlen)
		*rlen = act_len;

	return err;
}

static int dev_net_valid(int handle)
{
	if (!dev_valid(handle))
		return 0;

	if (devices[handle].type != DEV_TYP_NET)
		return 0;

	return 1;
}

int ub_dev_recv(int handle, void *buf, int len, int *rlen)
{
	struct device_info *di;
	int err = 0, act_len;

	if (!dev_net_valid(handle))
		return API_ENODEV;

	di = &devices[handle];
	if (!syscall(API_DEV_READ, &err, di, buf, &len, &act_len))
		return API_ESYSC;

	if (!err && rlen)
		*rlen = act_len;

	 return (err);
}

int ub_dev_send(int handle, void *buf, int len)
{
	struct device_info *di;
	int err = 0;

	if (!dev_net_valid(handle))
		return API_ENODEV;

	di = &devices[handle];
	if (!syscall(API_DEV_WRITE, &err, di, buf, &len))
		return API_ESYSC;

	return err;
}

/****************************************
 *
 * env vars
 *
 ****************************************/

char * ub_env_get(const char *name)
{
	char *value;

	if (!syscall(API_ENV_GET, NULL, name, &value))
		return NULL;

	return value;
}

void ub_env_set(const char *name, char *value)
{
	syscall(API_ENV_SET, NULL, name, value);
}

static char env_name[256];

const char * ub_env_enum(const char *last)
{
	const char *env, *str;
	int i;

	env = NULL;

	if (!syscall(API_ENV_ENUM, NULL, last, &env))
		return NULL;

	if (!env)
		/* no more env. variables to enumerate */
		return NULL;

	/* next enumerated env var */
	memset(env_name, 0, 256);
	for (i = 0, str = env; *str != '=' && *str != '\0';)
		env_name[i++] = *str++;

	env_name[i] = '\0';

	return env_name;
}

/****************************************
 *
 * display
 *
 ****************************************/

int ub_display_get_info(int type, struct display_info *di)
{
	int err = 0;

	if (!syscall(API_DISPLAY_GET_INFO, &err, type, di))
		return API_ESYSC;

	return err;
}

int ub_display_draw_bitmap(ulong bitmap, int x, int y)
{
	int err = 0;

	if (!syscall(API_DISPLAY_DRAW_BITMAP, &err, bitmap, x, y))
		return API_ESYSC;

	return err;
}

void ub_display_clear(void)
{
	syscall(API_DISPLAY_CLEAR, NULL);
}

__weak void *memcpy(void *dest, const void *src, size_t size)
{
	unsigned char *dptr = dest;
	const unsigned char *ptr = src;
	const unsigned char *end = src + size;

	while (ptr < end)
		*dptr++ = *ptr++;

	return dest;
}
