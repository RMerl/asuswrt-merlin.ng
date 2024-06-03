/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2013 The Chromium OS Authors.
 * Coypright (c) 2013 Guntermann & Drunck GmbH
 */

#ifndef __TPM_USER_UTILS_H
#define __TPM_USER_UTILS_H

void print_byte_string(u8 *data, size_t count);
void *parse_byte_string(char *bytes, u8 *data, size_t *count_ptr);
int report_return_code(int return_code);
int type_string_get_num_values(const char *type_str);
size_t type_string_get_space_size(const char *type_str);
void *type_string_alloc(const char *type_str, u32 *count);
int type_string_pack(const char *type_str, char * const values[], u8 *data);
int type_string_write_vars(const char *type_str, u8 *data, char * const vars[]);
int get_tpm(struct udevice **devp);

int do_tpm_init(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[]);
int do_tpm_info(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[]);
int do_tpm(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);

#endif /* __TPM_USER_UTILS_H */
