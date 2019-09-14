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

#include "../json.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* this is a work-around until we manage to fix configure.ac */
#pragma GCC diagnostic ignored "-Wdeclaration-after-statement"

int main(int __attribute__((unused)) argc, char __attribute__((unused)) **argv)
{
	struct fjson_object *my_object = fjson_object_new_object();
	if (my_object == NULL) {
		perror("malloc ptr table failed:");
		exit(1);
	}

	/* add some keys */
	fjson_object_object_add_ex (my_object, "a", fjson_object_new_string("a"), 0);
	fjson_object_object_add_ex (my_object, "b", fjson_object_new_string("b"), 0);
	fjson_object_object_add_ex (my_object, "c", fjson_object_new_string("c"), 0);
	fjson_object_object_add_ex (my_object, "d", fjson_object_new_string("d"), 0);

	/* delete some keys */
	fjson_object_object_del (my_object, "a");
	fjson_object_object_del (my_object, "c");

	/* check that iteration properly skips the deleted keys */
	struct fjson_object_iterator it = fjson_object_iter_begin(my_object);
	struct fjson_object_iterator itEnd = fjson_object_iter_end(my_object);
	while (!fjson_object_iter_equal (&it, &itEnd)) {
		printf("%s: %s\n",
			fjson_object_iter_peek_name (&it),
			fjson_object_to_json_string (fjson_object_iter_peek_value(&it)));
		fjson_object_iter_next (&it);
	}

	fjson_object_put (my_object);
	return 0;
}
