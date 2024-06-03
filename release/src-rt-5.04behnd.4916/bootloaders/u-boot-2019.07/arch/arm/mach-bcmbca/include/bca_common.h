/* SPDX-License-Identifier: GPL-2.0+
 *
 *  Copyright 2019 Broadcom Ltd.
 */

#ifndef _BCMBCA_COMMON_H
#define _BCMBCA_COMMON_H

int suffix2shift(char suffix);

int parse_env_string_plus_nums(const char *buffer, char **name, const int maxargs, unsigned long  *args, char *suffixes);
int parse_env_nums(const char *buffer, const int maxargs, unsigned long  *args, char *suffixes);
struct mtd_info *bcmbca_get_image_mtd_device(void);
struct mtd_info *bcmbca_get_nand_mtd_dev(void);
struct mtd_info *bcmbca_get_spinand_mtd_dev(void);
struct mtd_info *bcmbca_get_spinor_mtd_dev(void);
int bcmbca_is_nand_detected(void);
int bcmbca_is_spinand_detected(void);
int bcmbca_is_spinor_detected(void);
int bcmbca_is_emmc_detected(void);

void print_chipinfo(void);
void boost_cpu_clock(void);
int set_cpu_freq(int freqMHz);
u32 bcmbca_get_chipid(void);
u32 bcmbca_get_chiprev(void);

int bcm_board_boot_fit_fdt_fixup(void* fdt);
int bcm_board_boot_fdt_fixup(void* fdt);
int get_binary_from_bundle(ulong bundle, const char *conf_name, const char *name, char **bin_name, ulong *addr, ulong *size);
void init_cli_cb_arr(void);
void register_cli_job_cb(unsigned long time_period, void (*job_cb)(void));
void unregister_cli_job_cb(void (*job_cb)(void));
void run_cli_jobs(void);

#define BRCM_ROOTFS_SHA256_PROP      "brcm_rootfs_sha256"
#define BRCM_ROOTFS_IMGLEN_PROP      "brcm_rootfs_imglen"

#endif
