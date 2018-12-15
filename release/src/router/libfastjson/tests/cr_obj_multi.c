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

#define NUM_CREATIONS 1000000

int main(int __attribute__((unused)) argc, char __attribute__((unused)) **argv)
{
	int i;
	char pb[64];
	struct fjson_object **json = calloc(NUM_CREATIONS, sizeof(struct fjson_object *));

	if(json == NULL) {
		perror("malloc ptr table failed:");
		exit(1);
	}

	for(i = 0 ; i < NUM_CREATIONS ; ++i) {
		json[i] = fjson_object_new_object();
//fprintf(stderr, "main: json[%d] %p\n", i, json[i]);
		snprintf(pb, sizeof(pb), "%d", i);
		fjson_object_object_add(json[i], pb, fjson_object_new_string(pb));
		
	}

	/* free all objects again */
	for(i = 0 ; i < NUM_CREATIONS ; ++i) {
		fjson_object_put(json[i]);
	}

	free(json);
	return 0;
}
