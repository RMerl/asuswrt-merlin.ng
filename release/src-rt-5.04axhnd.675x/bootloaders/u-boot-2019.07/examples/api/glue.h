/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2007 Semihalf
 *
 * Written by: Rafal Jaworowski <raj@semihalf.com>
 */

/*
 * This is the header file for conveniency wrapper routines (API glue)
 */

#ifndef _API_GLUE_H_
#define _API_GLUE_H_

#define API_SEARCH_LEN		(3 * 1024 * 1024)	/* 3MB search range */

#define UB_MAX_MR	5	/* max mem regions number */
#define UB_MAX_DEV	6	/* max devices number */

extern void *syscall_ptr;
extern uint32_t search_hint;

int	syscall(int, int *, ...);
int	api_search_sig(struct api_signature **sig);

/*
 * The ub_ library calls are part of the application, not U-Boot code!  They
 * are front-end wrappers that are used by the consumer application: they
 * prepare arguments for particular syscall and jump to the low level
 * syscall()
 */

/* console */
int	ub_getc(void);
int	ub_tstc(void);
void	ub_putc(char c);
void	ub_puts(const char *s);

/* system */
void			ub_reset(void);
struct sys_info *	ub_get_sys_info(void);

/* time */
void		ub_udelay(unsigned long);
unsigned long	ub_get_timer(unsigned long);

/* env vars */
char *		ub_env_get(const char *name);
void		ub_env_set(const char *name, char *value);
const char *	ub_env_enum(const char *last);

/* devices */
int			ub_dev_enum(void);
int			ub_dev_open(int handle);
int			ub_dev_close(int handle);
int			ub_dev_read(int handle, void *buf, lbasize_t len,
				lbastart_t start, lbasize_t *rlen);
int			ub_dev_send(int handle, void *buf, int len);
int			ub_dev_recv(int handle, void *buf, int len, int *rlen);
struct device_info *	ub_dev_get(int);

/* display */
int ub_display_get_info(int type, struct display_info *di);
int ub_display_draw_bitmap(ulong bitmap, int x, int y);
void ub_display_clear(void);

#endif /* _API_GLUE_H_ */
