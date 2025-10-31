/* lib_test.c -- simple libcap-ng test suite
 * Copyright 2009,2012-13 Red Hat Inc.
 * All Rights Reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; see the file COPYING.LIB. If not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor
 * Boston, MA 02110-1335, USA.
 *
 * Authors:
 *      Steve Grubb <sgrubb@redhat.com>
 */

#include "config.h"
#include "../cap-ng.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>

static unsigned int get_last_cap(void)
{
	int fd;

	fd = open("/proc/sys/kernel/cap_last_cap", O_RDONLY);
	if (fd == -1) {
		return CAP_LAST_CAP;
	} else {
		char buf[8];
		int num = read(fd, buf, sizeof(buf));
		if (num > 0) {
			errno = 0;
			unsigned int val = strtoul(buf, NULL, 10);
			if (errno == 0)
				return val;
		}
		close(fd);
	}
	return CAP_LAST_CAP;
}

int main(void)
{
	int rc;
	unsigned int i, len, last = get_last_cap();
	char *text;
	void *saved;

	puts("Doing basic bit tests...");
	capng_clear(CAPNG_SELECT_BOTH);
	if (capng_have_capabilities(CAPNG_SELECT_BOTH) != CAPNG_NONE) {
		puts("Failed clearing capabilities");
		abort();
	}
	saved = capng_save_state();
	capng_fill(CAPNG_SELECT_BOTH);
	if (capng_have_capabilities(CAPNG_SELECT_BOTH) != CAPNG_FULL) {
		puts("Failed filling capabilities");
		abort();
	}
	// Need to detect if version 1 or 2 capabilities
	text = capng_print_caps_numeric(CAPNG_PRINT_BUFFER, CAPNG_SELECT_CAPS);
	len = strlen(text);
	free(text);
	if (len < 80 && last > 30)	// The kernel & headers are mismatched
		last = 30;
	// Now test that restore still works
	capng_restore_state(&saved);
	if (capng_have_capabilities(CAPNG_SELECT_BOTH) != CAPNG_NONE) {
		puts("Failed restoring capabilities");
		abort();
	}
	printf("Doing advanced bit tests for %d capabilities...\n", last);
	for (i=0; i<=last; i++) {
		const char *name;
		capng_clear(CAPNG_SELECT_BOTH);
		rc = capng_update(CAPNG_ADD, CAPNG_EFFECTIVE, i);
		if (rc) {
			puts("Failed update test 1");
			abort();
		}
		rc = capng_have_capability(CAPNG_EFFECTIVE, i);
		if (rc == 0) {
			puts("Failed have capability test 1");
			capng_print_caps_numeric(CAPNG_PRINT_STDOUT,
					CAPNG_SELECT_CAPS);
			abort();
		}
		if(capng_have_capabilities(CAPNG_SELECT_CAPS)!=CAPNG_PARTIAL){
			puts("Failed have capabilities test 1");
			capng_print_caps_numeric(CAPNG_PRINT_STDOUT,
					CAPNG_SELECT_CAPS);
			abort();
		}
#if CAP_LAST_CAP > 31
		rc = capng_update(CAPNG_ADD, CAPNG_BOUNDING_SET, i);
		if (rc) {
			puts("Failed bset update test 2");
			abort();
		}
		rc = capng_have_capability(CAPNG_BOUNDING_SET, i);
		if (rc == 0) {
			puts("Failed bset have capability test 2");
			capng_print_caps_numeric(CAPNG_PRINT_STDOUT,
					CAPNG_SELECT_BOTH);
			abort();
		}
		if(capng_have_capabilities(CAPNG_SELECT_BOUNDS)!=CAPNG_PARTIAL){
			puts("Failed bset have capabilities test 2");
			capng_print_caps_numeric(CAPNG_PRINT_STDOUT,
					CAPNG_SELECT_BOTH);
			abort();
		}
#endif
		text=capng_print_caps_text(CAPNG_PRINT_BUFFER, CAPNG_EFFECTIVE);
		if (text == NULL) {
			puts("Failed getting print text to buffer");
			abort();
		}
		name = capng_capability_to_name(i);
		if (name == NULL) {
			printf("Failed converting capability %d to name\n", i);
			abort();
		}
		if (strcmp(text, name)) {
			puts("Failed print text comparison");
			printf("%s != %s\n", text, name);
			abort();
		}
		free(text);
		// Now make sure the mask part is working
		capng_fill(CAPNG_SELECT_BOTH);
		rc = capng_update(CAPNG_DROP, CAPNG_EFFECTIVE, i);
		if (rc) {
			puts("Failed update test 3");
			abort();
		}
		// Should be partial
		if(capng_have_capabilities(CAPNG_SELECT_CAPS)!=CAPNG_PARTIAL){
			puts("Failed have capabilities test 3");
			capng_print_caps_numeric(CAPNG_PRINT_STDOUT,
					CAPNG_SELECT_CAPS);
			abort();
		}
		// Add back the bit and should be full capabilities
		rc = capng_update(CAPNG_ADD, CAPNG_EFFECTIVE, i);
		if (rc) {
			puts("Failed update test 4");
			abort();
		}
		if (capng_have_capabilities(CAPNG_SELECT_CAPS) != CAPNG_FULL){
			puts("Failed have capabilities test 4");
			capng_print_caps_numeric(CAPNG_PRINT_STDOUT,
					CAPNG_SELECT_CAPS);
			abort();
		}
	}
	// Now test the updatev function
	capng_clear(CAPNG_SELECT_BOTH);
	rc = capng_updatev(CAPNG_ADD, CAPNG_EFFECTIVE,
			CAP_CHOWN, CAP_FOWNER, CAP_KILL, -1);
	if (rc) {
		puts("Failed updatev test");
		abort();
	}
	rc = capng_have_capability(CAPNG_EFFECTIVE, CAP_CHOWN) &&
		capng_have_capability(CAPNG_EFFECTIVE, CAP_FOWNER) &&
		capng_have_capability(CAPNG_EFFECTIVE, CAP_KILL);
	if (rc == 0) {
		puts("Failed have updatev capability test");
		capng_print_caps_numeric(CAPNG_PRINT_STDOUT,
				CAPNG_SELECT_CAPS);
		abort();
	}

	return EXIT_SUCCESS;
}

