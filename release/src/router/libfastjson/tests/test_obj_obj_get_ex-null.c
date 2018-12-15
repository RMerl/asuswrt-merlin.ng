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

#include "../json.h"
#include "../debug.h"
#include "parse_flags.h"

/* this is a work-around until we manage to fix configure.ac */
#pragma GCC diagnostic ignored "-Wdeclaration-after-statement"

#define DEBUG_SEED(s)

int main(int __attribute__((unused)) argc, char __attribute__((unused)) **argv)
{
	fjson_object *my_object;

	MC_SET_DEBUG(1);

	my_object = fjson_object_new_object();
	fjson_object_object_add_ex(my_object, "a", fjson_object_new_int(1), 0);

	int found = fjson_object_object_get_ex(my_object, "a", NULL);
	printf("found=%d\n", found);

	fjson_object_put(my_object);

	return 0;
}
