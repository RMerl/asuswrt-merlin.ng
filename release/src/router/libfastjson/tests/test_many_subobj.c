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

#define NUM_SUBOBJ 200
#define NUM_SUBOBJ_HALF (NUM_SUBOBJ/2)
#define NUM_SUBOBJ_QUARTER (NUM_SUBOBJ/4)

int main(int __attribute__((unused)) argc, char __attribute__((unused)) **argv)
{
	int i;
	char pb[64];
	struct fjson_object *json = fjson_object_new_object();
	if (json == NULL) {
		perror("malloc ptr table failed:");
		exit(1);
	}

	/* add some keys */
	for (i = 0 ; i < NUM_SUBOBJ ; ++i) {
		snprintf(pb, sizeof(pb), "key-%d", i);
		fjson_object_object_add_ex(json, pb, fjson_object_new_int(i), 0);
		
	}
	printf("STEP1: %s\n", fjson_object_to_json_string(json));

	/* delete some keys */
	for (i = NUM_SUBOBJ_HALF - NUM_SUBOBJ_QUARTER ;
		i < NUM_SUBOBJ_HALF + NUM_SUBOBJ_QUARTER ;
		++i) {
		snprintf(pb, sizeof(pb), "key-%d", i);
		fjson_object_object_del(json, pb);
	}
	printf("STEP2: %s\n", fjson_object_to_json_string(json));

	/* add new keys */
	for (i = NUM_SUBOBJ_HALF + NUM_SUBOBJ_QUARTER - 1;
		i >= NUM_SUBOBJ_HALF - NUM_SUBOBJ_QUARTER ;
		--i) {
		snprintf(pb, sizeof(pb), "KEY-%d", i);
		fjson_object_object_add_ex(json, pb, fjson_object_new_int(i), 0);
	}
	printf("STEP3: %s\n", fjson_object_to_json_string(json));

	/* delete the new keys again, and also update key values */
	for (i = NUM_SUBOBJ_HALF - NUM_SUBOBJ_QUARTER ;
		i < NUM_SUBOBJ_HALF + NUM_SUBOBJ_QUARTER ;
		++i) {
		snprintf(pb, sizeof(pb), "KEY-%d", i);
		fjson_object_object_del(json, pb);
	}
	for (i = 0 ; i < NUM_SUBOBJ ; ++i) {
		snprintf(pb, sizeof(pb), "key-%d", i);
		fjson_object_object_add_ex(json, pb, fjson_object_new_int(i*10), 0);
		
	}
	printf("STEP4: %s\n", fjson_object_to_json_string(json));

	/* add one more key to see that adding works when extending the array */
	snprintf(pb, sizeof(pb), "key-%d", NUM_SUBOBJ);
	fjson_object_object_add(json, pb, fjson_object_new_int(NUM_SUBOBJ));
	printf("STEP5:%s\n", fjson_object_to_json_string(json));

	fjson_object_put(json);
	return 0;
}
