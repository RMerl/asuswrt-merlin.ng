/* libfastjson testbench tool
 *
 * Copyright (c) 2016 Adiscon GmbH
 * Rainer Gerhards <rgerhards@adiscon.com>
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See COPYING for details.
 *
 */
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <assert.h>

#include "../json.h"
#include "../debug.h"
#include "parse_flags.h"

/* this is a work-around until we manage to fix configure.ac */
#pragma GCC diagnostic ignored "-Wdeclaration-after-statement"

#define DEBUG_SEED(s)

#ifdef TEST_FORMATTED
#define fjson_object_to_json_string(obj) fjson_object_to_json_string_ext(obj,sflags)
#else
/* no special define */
#endif

int main(int __attribute__((unused)) argc, char __attribute__((unused)) **argv)
{
	fjson_object *my_object;
#ifdef TEST_FORMATTED
	int sflags = 0;
#endif

	MC_SET_DEBUG(1);

#ifdef TEST_FORMATTED
	sflags = parse_flags(argc, argv);
#endif

	my_object = fjson_object_new_object();
	fjson_object_object_add_ex(my_object, "abc", fjson_object_new_int(12), 0);
	fjson_object_object_add_ex(my_object, "foo", fjson_object_new_string("bar"),
		FJSON_OBJECT_ADD_KEY_IS_NEW);
	fjson_object_object_add_ex(my_object, "bool0", fjson_object_new_boolean(0),
		FJSON_OBJECT_KEY_IS_CONSTANT);
	fjson_object_object_add_ex(my_object, "bool1", fjson_object_new_boolean(1), 
		FJSON_OBJECT_ADD_KEY_IS_NEW | FJSON_OBJECT_KEY_IS_CONSTANT);

	printf("my_object=\n");
	struct fjson_object_iterator it = fjson_object_iter_begin(my_object);
	struct fjson_object_iterator itEnd = fjson_object_iter_end(my_object);
	while (!fjson_object_iter_equal(&it, &itEnd)) {
		printf("\t%s: %s\n",
			fjson_object_iter_peek_name(&it),
			fjson_object_to_json_string(fjson_object_iter_peek_value(&it)));
		fjson_object_iter_next(&it);
	}

	printf("my_object.to_string()=%s\n", fjson_object_to_json_string(my_object));

	fjson_object_put(my_object);

	return 0;
}
