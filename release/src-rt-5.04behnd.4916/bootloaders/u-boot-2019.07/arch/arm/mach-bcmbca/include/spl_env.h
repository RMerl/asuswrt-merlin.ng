/* SPDX-License-Identifier: GPL-2.0+
 *
 *  Copyright 2019 Broadcom Ltd.
 */

#ifndef _TPL_ENV_H
#define _TPL_ENV_H

#define TPL_ENV_ADDR ((CONFIG_TPL_TEXT_BASE - BOOT_BLOB_MAX_ENV_SIZE)&~0xff)

char *find_spl_env_val(const char *buffer, const char *name);

int get_spl_env_val(const char *buffer, const char *name, char *out, int maxlen);

int validate_metadata(char *cp,int *valid,int *comitted,int *seq) ;

/* UBOOT has multiple conflicting CRC32 functions... so we'll wrap the right one */
uint32_t the_env_crc32(int a, void *b, int len) ;

#define ENV_AVS_DISABLE "avs_disable"

#endif
