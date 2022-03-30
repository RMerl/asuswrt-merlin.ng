/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2007 Semihalf
 *
 * Written by: Rafal Jaworowski <raj@semihalf.com>
 */

#ifndef _API_PRIVATE_H_
#define _API_PRIVATE_H_

void	api_init(void);
void	platform_set_mr(struct sys_info *, unsigned long, unsigned long, int);
int	platform_sys_info(struct sys_info *);

void	dev_enum_reset(void);
int	dev_enum_storage(struct device_info *);
int	dev_enum_net(struct device_info *);

int	dev_open_stor(void *);
int	dev_open_net(void *);
int	dev_close_stor(void *);
int	dev_close_net(void *);

lbasize_t	dev_read_stor(void *, void *, lbasize_t, lbastart_t);
int		dev_read_net(void *, void *, int);
int		dev_write_net(void *, void *, int);

void dev_stor_init(void);

int display_get_info(int type, struct display_info *di);
int display_draw_bitmap(ulong bitmap, int x, int y);
void display_clear(void);

#endif /* _API_PRIVATE_H_ */
