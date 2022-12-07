/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2012-2014  Intel Corporation. All rights reserved.
 *
 *
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

void tester_init(int *argc, char ***argv);
int tester_run(void);

bool tester_use_quiet(void);
bool tester_use_debug(void);

void tester_print(const char *format, ...)
				__attribute__((format(printf, 1, 2)));
void tester_warn(const char *format, ...)
				__attribute__((format(printf, 1, 2)));
void tester_debug(const char *format, ...)
				__attribute__((format(printf, 1, 2)));
void tester_monitor(char dir, uint16_t cid, uint16_t psm, const void *data,
								size_t len);

typedef void (*tester_destroy_func_t)(void *user_data);
typedef void (*tester_data_func_t)(const void *test_data);

void tester_add_full(const char *name, const void *test_data,
				tester_data_func_t pre_setup_func,
				tester_data_func_t setup_func,
				tester_data_func_t test_func,
				tester_data_func_t teardown_func,
				tester_data_func_t post_teardown_func,
				unsigned int timeout,
				void *user_data, tester_destroy_func_t destroy);

void tester_add(const char *name, const void *test_data,
					tester_data_func_t setup_func,
					tester_data_func_t test_func,
					tester_data_func_t teardown_func);

void *tester_get_data(void);

void tester_pre_setup_complete(void);
void tester_pre_setup_failed(void);

void tester_setup_complete(void);
void tester_setup_failed(void);

void tester_test_passed(void);
void tester_test_failed(void);
void tester_test_abort(void);

void tester_teardown_complete(void);
void tester_teardown_failed(void);

void tester_post_teardown_complete(void);
void tester_post_teardown_failed(void);

typedef void (*tester_wait_func_t)(void *user_data);

void tester_wait(unsigned int seconds, tester_wait_func_t func,
							void *user_data);
