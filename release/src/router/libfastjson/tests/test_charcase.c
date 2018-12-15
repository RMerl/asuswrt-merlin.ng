#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#include "../json.h"
#include "../json_tokener.h"
#include "../debug.h"

#define CHK(x) if (!(x)) { \
	printf("%s:%d: unexpected result with '%s'\n", \
		__FILE__, __LINE__, #x); \
	exit(1); \
}

static void test_case_parse(void);

int main(int __attribute__((unused)) argc, char __attribute__((unused)) **argv)
{
	MC_SET_DEBUG(1);

	test_case_parse();
	return 0;
}

/* make sure only lowercase forms are parsed in strict mode */
static void test_case_parse(void)
{
	struct fjson_tokener *tok;
	fjson_object *new_obj;

	tok = fjson_tokener_new();
	fjson_tokener_set_flags(tok, FJSON_TOKENER_STRICT);

	new_obj = fjson_tokener_parse_ex(tok, "True", 4);
	CHK(new_obj == NULL);

	new_obj = fjson_tokener_parse_ex(tok, "False", 5);
	CHK(new_obj == NULL);

	new_obj = fjson_tokener_parse_ex(tok, "Null", 4);
	CHK(new_obj == NULL);

	printf("OK\n");

	fjson_tokener_free(tok);
}
