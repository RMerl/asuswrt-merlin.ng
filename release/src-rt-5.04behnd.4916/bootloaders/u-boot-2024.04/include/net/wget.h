/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Duncan Hare Copyright 2017
 */

/**
 * wget_start() - begin wget
 */
void wget_start(void);

enum wget_state {
	WGET_CLOSED,
	WGET_CONNECTING,
	WGET_CONNECTED,
	WGET_TRANSFERRING,
	WGET_TRANSFERRED
};

#define DEBUG_WGET		0	/* Set to 1 for debug messages */
#define WGET_RETRY_COUNT	30
#define WGET_TIMEOUT		2000UL
