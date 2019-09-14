#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#include "../json.h"
#include "../debug.h"

/* this is a work-around until we manage to fix configure.ac */
#pragma GCC diagnostic ignored "-Wdeclaration-after-statement"

int main(int __attribute__((unused)) argc, char __attribute__((unused)) **argv)
{
	struct fjson_object_iterator it;
	struct fjson_object_iterator itEnd;
	const char *key;
	MC_SET_DEBUG(1);

	/*
	 * Check that replacing an existing object keeps the key valid,
	 * and that it keeps the order the same.
	 */
	fjson_object *my_object = fjson_object_new_object();
	fjson_object_object_add(my_object, "foo1", fjson_object_new_string("bar1"));
	fjson_object_object_add(my_object, "foo2", fjson_object_new_string("bar2"));
	fjson_object_object_add(my_object, "deleteme", fjson_object_new_string("bar2"));
	fjson_object_object_add(my_object, "foo3", fjson_object_new_string("bar3"));

	printf("==== delete-in-loop test starting ====\n");

	int orig_count = 0;
	itEnd = fjson_object_iter_end(my_object);
	it = fjson_object_iter_begin(my_object);
	while (!fjson_object_iter_equal(&it, &itEnd)) {
		key = fjson_object_iter_peek_name(&it);
		printf("Key at index %d is [%s]", orig_count, key);
		/* need to advance now, as del invalidates "it" */
		fjson_object_iter_next(&it);
		if (strcmp(key, "deleteme") == 0) {
			fjson_object_object_del(my_object, key);
			printf(" (deleted)\n");
		} else {
			printf(" (kept)\n");
		}
		orig_count++;
	}

	printf("==== replace-value first loop starting ====\n");

	const char *original_key = NULL;
	orig_count = 0;
	itEnd = fjson_object_iter_end(my_object);
	it = fjson_object_iter_begin(my_object);
	while (!fjson_object_iter_equal(&it, &itEnd)) {
		key = fjson_object_iter_peek_name(&it);
		/* need to advance now, as modify invalidates "it" */
		fjson_object_iter_next(&it);
		printf("Key at index %d is [%s]\n", orig_count, key);
		orig_count++;
		if (strcmp(key, "foo2") != 0)
			continue;
		printf("replacing value for key [%s]\n", key);
		original_key = key;
		fjson_object_object_add(my_object, key, fjson_object_new_string("zzz"));
	}

	printf("==== second loop starting ====\n");

	int new_count = 0;
	int retval = 0;
	itEnd = fjson_object_iter_end(my_object);
	it = fjson_object_iter_begin(my_object);
	while (!fjson_object_iter_equal(&it, &itEnd)) {
		key = fjson_object_iter_peek_name(&it);
		/* need to advance now, as modify invalidates "it" */
		fjson_object_iter_next(&it);
		printf("Key at index %d is [%s]\n", new_count, key);
		new_count++;
		if (strcmp(key, "foo2") != 0)
			continue;
		printf("pointer for key [%s] does %smatch\n", key,
		       (key == original_key) ? "" : "NOT ");
		if (key != original_key)
			retval = 1;
	}
	if (new_count != orig_count)
	{
		printf("mismatch between original count (%d) and new count (%d)\n",
		       orig_count, new_count);
		retval = 1;
	}

	fjson_object_put( my_object );

	return retval;
}
