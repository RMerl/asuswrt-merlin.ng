/* SPDX-License-Identifier: GPL-2.0+ */

#ifndef __LEGACY_MTD_UTILS_H
#define __LEGACY_MTD_UTILS_H

int mtd_arg_off(const char *arg, int *idx, loff_t *off, loff_t *size,
		loff_t *maxsize, int devtype, uint64_t chipsize);
int mtd_arg_off_size(int argc, char *const argv[], int *idx, loff_t *off,
		     loff_t *size, loff_t *maxsize, int devtype,
		     uint64_t chipsize);

#endif /* LEGACY_MTD_UTILS_H */
