/* SPDX-License-Identifier: Apache-2.0 */
/*
 * Copyright (C) 2013 Intel Corporation
 *
 */

void history_store(const char *filename);
void history_restore(const char *filename);
void history_add_line(const char *line);
int history_get_line(int n, char *buf, int buf_size);
