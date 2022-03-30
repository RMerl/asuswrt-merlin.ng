/* SPDX-License-Identifier: GPL-2.0+
 *
 *  Copyright 2019 Broadcom Ltd.
 */

#ifndef _BCMBCA_COMMON_H
#define _BCMBCA_COMMON_H

int suffix2shift(char suffix);

int parse_env_string_plus_nums(const char *buffer, char **name, const int maxargs, unsigned long  *args, char *suffixes);
int parse_env_nums(const char *buffer, const int maxargs, unsigned long  *args, char *suffixes);
int bcmbca_get_boot_device(void);

void print_chipinfo(void);
void boost_cpu_clock(void);
int set_cpu_freq(int freqMHz);

int bcm_board_boot_fit_fdt_fixup(void* fdt);
int bcm_board_boot_fdt_fixup(void* fdt);
void init_cli_cb_arr(void);
void register_cli_job_cb(unsigned long time_period, void (*job_cb)(void));
void unregister_cli_job_cb(void (*job_cb)(void));
void run_cli_jobs(void);

#define BRCM_ROOTFS_SHA256_PROP      "brcm_rootfs_sha256"
#define BRCM_ROOTFS_IMGLEN_PROP      "brcm_rootfs_imglen"

#endif
