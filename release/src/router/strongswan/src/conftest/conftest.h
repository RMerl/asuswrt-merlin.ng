/*
 * Copyright (C) 2010 Martin Willi
 * Copyright (C) 2010 revosec AG
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

/**
 * @defgroup conftest conftest
 */

#ifndef CONFTEST_H_
#define CONFTEST_H_

#include <library.h>
#include <daemon.h>
#include <credentials/sets/mem_cred.h>

#include "config.h"
#include "actions.h"

typedef struct conftest_t conftest_t;

/**
 * Global conftest variables.
 */
struct conftest_t {

	/**
	 * Merged suite/test configuration
	 */
	settings_t *test;

	/**
	 * Directory containing suite files
	 */
	char *suite_dir;

	/**
	 * Credentials loaded from configuration
	 */
	mem_cred_t *creds;

	/**
	 * Configurations loaded from config
	 */
	config_t *config;

	/**
	 * Loaded hooks
	 */
	linked_list_t *hooks;

	/**
	 * Action handling
	 */
	actions_t *actions;

	/**
	 * Test specific loggers
	 */
	linked_list_t *loggers;
};

/**
 * Conftest globals
 */
extern conftest_t *conftest;

#endif /** CONFTEST_H_ */
