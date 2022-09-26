/* $Id: testaddr_is_reserved.c,v 1.1 2020/10/15 22:12:51 nanard Exp $ */
/* vim: tabstop=4 shiftwidth=4 noexpandtab
 * Project : miniupnp
 * Web : http://miniupnp.free.fr/ or https://miniupnp.tuxfamily.org/
 * Author : Thomas BERNARD
 * copyright (c) 2005-2020 Thomas Bernard
 * This software is subjet to the conditions detailed in the
 * provided LICENSE file. */
#include <stdio.h>
#include "addr_is_reserved.h"

static const struct {
	const char * str;
	int expected_result;
} tests[] = {
{ "0.0.0.0", 1 },
{ "8.8.8.8", 0 },
{ "192.168.1.1", 1 },
{ "10.250.42.12", 1 },
{ "11.250.42.12", 0 },
{ "172.31.1.1", 1 },
{ "172.32.1.1", 0 },
{ "169.254.42.42", 1 },
{ "192.0.0.11", 1 },
{ "198.0.0.11", 0 },
{ "198.18.0.11", 1 },
{ "100.64.1.1", 1 },
{ "100.127.1.1", 1 },
{ "100.128.1.1", 0 },
{ NULL, 0 }
};

int main(int argc, char * * argv) {
	int i, result;
	(void)argc; (void)argv;

	for (i = 0; tests[i].str != NULL; i++) {
		result = addr_is_reserved(tests[i].str);
		printf("testing %s %d %d\n", tests[i].str, tests[i].expected_result, result);
		if (result != tests[i].expected_result) {
			fprintf(stderr, "*** FAILURE ***\n");
			return 1;	/* Failure */
		}
	}
	return 0;	/* success */
}
