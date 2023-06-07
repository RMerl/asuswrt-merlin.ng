/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __RTM_MAP_H__
#define __RTM_MAP_H__ 1

char *rtnl_rtntype_n2a(int id, char *buf, int len);
int rtnl_rtntype_a2n(int *id, char *arg);

int get_rt_realms_or_raw(__u32 *realms, char *arg);

#endif /* __RTM_MAP_H__ */
