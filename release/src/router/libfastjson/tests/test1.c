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

static int sort_fn (const void *j1, const void *j2)
{
	fjson_object * const *jso1, * const *jso2;
	int i1, i2;

	jso1 = (fjson_object* const*)j1;
	jso2 = (fjson_object* const*)j2;
	if (!*jso1 && !*jso2)
		return 0;
	if (!*jso1)
		return -1;
	if (!*jso2)
		return 1;

	i1 = fjson_object_get_int(*jso1);
	i2 = fjson_object_get_int(*jso2);

	return i1 - i2;
}

#ifdef TEST_FORMATTED
#define fjson_object_to_json_string(obj) fjson_object_to_json_string_ext(obj,sflags)
#else
/* no special define */
#endif

int main(int __attribute__((unused)) argc, char __attribute__((unused)) **argv)
{
	fjson_object *my_string, *my_int, *my_object, *my_array;
	int i;
#ifdef TEST_FORMATTED
	int sflags = 0;
#endif

	MC_SET_DEBUG(1);

#ifdef TEST_FORMATTED
	sflags = parse_flags(argc, argv);
#endif

	my_string = fjson_object_new_string("\t");
	printf("my_string=%s\n", fjson_object_get_string(my_string));
	printf("my_string.to_string()=%s\n", fjson_object_to_json_string(my_string));
	fjson_object_put(my_string);

	my_string = fjson_object_new_string("\\");
	printf("my_string=%s\n", fjson_object_get_string(my_string));
	printf("my_string.to_string()=%s\n", fjson_object_to_json_string(my_string));
	fjson_object_put(my_string);

	my_string = fjson_object_new_string("foo");
	printf("my_string=%s\n", fjson_object_get_string(my_string));
	printf("my_string.to_string()=%s\n", fjson_object_to_json_string(my_string));

	my_int = fjson_object_new_int(9);
	printf("my_int=%d\n", fjson_object_get_int(my_int));
	printf("my_int.to_string()=%s\n", fjson_object_to_json_string(my_int));

	my_array = fjson_object_new_array();
	fjson_object_array_add(my_array, fjson_object_new_int(1));
	fjson_object_array_add(my_array, fjson_object_new_int(2));
	fjson_object_array_add(my_array, fjson_object_new_int(3));
	fjson_object_array_put_idx(my_array, 4, fjson_object_new_int(5));
	printf("my_array=\n");
	for(i=0; i < fjson_object_array_length(my_array); i++)
	{
		fjson_object *obj = fjson_object_array_get_idx(my_array, i);
		printf("\t[%d]=%s\n", i, fjson_object_to_json_string(obj));
	}
	printf("my_array.to_string()=%s\n", fjson_object_to_json_string(my_array));

	fjson_object_put(my_array);

	my_array = fjson_object_new_array();
	fjson_object_array_add(my_array, fjson_object_new_int(3));
	fjson_object_array_add(my_array, fjson_object_new_int(1));
	fjson_object_array_add(my_array, fjson_object_new_int(2));
	fjson_object_array_put_idx(my_array, 4, fjson_object_new_int(0));
	printf("my_array=\n");
	for(i=0; i < fjson_object_array_length(my_array); i++)
	{
		fjson_object *obj = fjson_object_array_get_idx(my_array, i);
		printf("\t[%d]=%s\n", i, fjson_object_to_json_string(obj));
	}
	printf("my_array.to_string()=%s\n", fjson_object_to_json_string(my_array));
	fjson_object_array_sort(my_array, sort_fn);
	printf("my_array=\n");
	for(i=0; i < fjson_object_array_length(my_array); i++)
	{
		fjson_object *obj = fjson_object_array_get_idx(my_array, i);
		printf("\t[%d]=%s\n", i, fjson_object_to_json_string(obj));
	}
	printf("my_array.to_string()=%s\n", fjson_object_to_json_string(my_array));

	my_object = fjson_object_new_object();
	fjson_object_object_add(my_object, "abc", fjson_object_new_int(12));
	fjson_object_object_add(my_object, "foo", fjson_object_new_string("bar"));
	fjson_object_object_add(my_object, "bool0", fjson_object_new_boolean(0));
	fjson_object_object_add(my_object, "bool1", fjson_object_new_boolean(1));
	fjson_object_object_add(my_object, "baz", fjson_object_new_string("bang"));

	fjson_object *baz_obj = fjson_object_new_string("fark");
	fjson_object_get(baz_obj);
	fjson_object_object_add(my_object, "baz", baz_obj);
	fjson_object_object_del(my_object, "baz");

	/* baz_obj should still be valid */
	printf("baz_obj.to_string()=%s\n", fjson_object_to_json_string(baz_obj));
	fjson_object_put(baz_obj);

	/*fjson_object_object_add(my_object, "arr", my_array);*/
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

	fjson_object_put(my_string);
	fjson_object_put(my_int);
	fjson_object_put(my_object);
	fjson_object_put(my_array);

	return 0;
}
