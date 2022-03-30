/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * An inteface for configuring a hardware via u-boot environment.
 *
 * Copyright (c) 2009  MontaVista Software, Inc.
 * Copyright 2011 Freescale Semiconductor, Inc.
 *
 * Author: Anton Vorontsov <avorontsov@ru.mvista.com>
 */

#ifndef _HWCONFIG_H
#define _HWCONFIG_H

#include <linux/types.h>
#include <linux/errno.h>

#ifdef CONFIG_HWCONFIG

extern int hwconfig_f(const char *opt, char *buf);
extern const char *hwconfig_arg_f(const char *opt, size_t *arglen, char *buf);
extern int hwconfig_arg_cmp_f(const char *opt, const char *arg, char *buf);
extern int hwconfig_sub_f(const char *opt, const char *subopt, char *buf);
extern const char *hwconfig_subarg_f(const char *opt, const char *subopt,
				     size_t *subarglen, char *buf);
extern int hwconfig_subarg_cmp_f(const char *opt, const char *subopt,
				 const char *subarg, char *buf);
#else

static inline int hwconfig_f(const char *opt, char *buf)
{
	return -ENOSYS;
}

static inline const char *hwconfig_arg_f(const char *opt, size_t *arglen,
					 char *buf)
{
	*arglen = 0;
	return "";
}

static inline int hwconfig_arg_cmp_f(const char *opt, const char *arg,
				     char *buf)
{
	return -ENOSYS;
}

static inline int hwconfig_sub_f(const char *opt, const char *subopt, char *buf)
{
	return -ENOSYS;
}

static inline const char *hwconfig_subarg_f(const char *opt, const char *subopt,
					    size_t *subarglen, char *buf)
{
	*subarglen = 0;
	return "";
}

static inline int hwconfig_subarg_cmp_f(const char *opt, const char *subopt,
					const char *subarg, char *buf)
{
	return -ENOSYS;
}

#endif /* CONFIG_HWCONFIG */

static inline int hwconfig(const char *opt)
{
	return hwconfig_f(opt, NULL);
}

static inline const char *hwconfig_arg(const char *opt, size_t *arglen)
{
	return hwconfig_arg_f(opt, arglen, NULL);
}

static inline int hwconfig_arg_cmp(const char *opt, const char *arg)
{
	return hwconfig_arg_cmp_f(opt, arg, NULL);
}

static inline int hwconfig_sub(const char *opt, const char *subopt)
{
	return hwconfig_sub_f(opt, subopt, NULL);
}

static inline const char *hwconfig_subarg(const char *opt, const char *subopt,
					  size_t *subarglen)
{
	return hwconfig_subarg_f(opt, subopt, subarglen, NULL);
}

static inline int hwconfig_subarg_cmp(const char *opt, const char *subopt,
				      const char *subarg)
{
	return hwconfig_subarg_cmp_f(opt, subopt, subarg, NULL);
}

#endif /* _HWCONFIG_H */
