/*
 * Tests if casting within the fjson_object_get_* functions work correctly.
 * Also checks the fjson_object_get_type and fjson_object_is_type functions.
 */

#include "config.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>

#include "../json_object.h"
#include "../json_tokener.h"
#include "../json_util.h"

/* this is a work-around until we manage to fix configure.ac */
#pragma GCC diagnostic ignored "-Wdeclaration-after-statement"

static void getit(struct fjson_object *new_obj, const char *field);
static void checktype_header(void);
static void checktype(struct fjson_object *new_obj, const char *field);

int main(int __attribute__((unused)) argc, char __attribute__((unused)) **argv)
{
	const char *input = "{\n\
		\"string_of_digits\": \"123\",\n\
		\"regular_number\": 222,\n\
		\"decimal_number\": 99.55,\n\
		\"boolean_true\": true,\n\
		\"boolean_false\": false,\n\
		\"big_number\": 2147483649,\n\
		\"a_null\": null,\n\
	}";
	/* Note: 2147483649 = INT_MAX + 2 */

	struct fjson_object *new_obj;

	new_obj = fjson_tokener_parse(input);
	printf("Parsed input: %s\n", input);
	printf("Result is %s\n", (new_obj == NULL) ? "NULL (error!)" : "not NULL");
	if (!new_obj)
		return 1; // oops, we failed.

	getit(new_obj, "string_of_digits");
	getit(new_obj, "regular_number");
	getit(new_obj, "decimal_number");
	getit(new_obj, "boolean_true");
	getit(new_obj, "boolean_false");
	getit(new_obj, "big_number");
	getit(new_obj, "a_null");

	// Now check the behaviour of the fjson_object_is_type() function.
	printf("\n================================\n");
	checktype_header();
	checktype(new_obj, NULL);
	checktype(new_obj, "string_of_digits");
	checktype(new_obj, "regular_number");
	checktype(new_obj, "decimal_number");
	checktype(new_obj, "boolean_true");
	checktype(new_obj, "boolean_false");
	checktype(new_obj, "big_number");
	checktype(new_obj, "a_null");

	fjson_object_put(new_obj);

	return 0;
}

static void getit(struct fjson_object *new_obj, const char *field)
{
	struct fjson_object *o = NULL;
	if (!fjson_object_object_get_ex(new_obj, field, &o))
		printf("Field %s does not exist\n", field);

	enum fjson_type o_type = fjson_object_get_type(o);
	printf("new_obj.%s fjson_object_get_type()=%s\n", field,
	       fjson_type_to_name(o_type));
	printf("new_obj.%s fjson_object_get_int()=%d\n", field,
	       fjson_object_get_int(o));
	printf("new_obj.%s fjson_object_get_int64()=%" PRId64 "\n", field,
	       fjson_object_get_int64(o));
	printf("new_obj.%s fjson_object_get_boolean()=%d\n", field,
	       fjson_object_get_boolean(o));
	printf("new_obj.%s fjson_object_get_double()=%f\n", field,
	       fjson_object_get_double(o));
}

static void checktype_header(void)
{
	printf("fjson_object_is_type: %s,%s,%s,%s,%s,%s,%s\n",
		fjson_type_to_name(fjson_type_null),
		fjson_type_to_name(fjson_type_boolean),
		fjson_type_to_name(fjson_type_double),
		fjson_type_to_name(fjson_type_int),
		fjson_type_to_name(fjson_type_object),
		fjson_type_to_name(fjson_type_array),
		fjson_type_to_name(fjson_type_string));
}
static void checktype(struct fjson_object *new_obj, const char *field)
{
	struct fjson_object *o = new_obj;
	if (field && !fjson_object_object_get_ex(new_obj, field, &o))
		printf("Field %s does not exist\n", field);
			
	printf("new_obj%s%-18s: %d,%d,%d,%d,%d,%d,%d\n",
		field ? "." : " ", field ? field : "",
		fjson_object_is_type(o, fjson_type_null),
		fjson_object_is_type(o, fjson_type_boolean),
		fjson_object_is_type(o, fjson_type_double),
		fjson_object_is_type(o, fjson_type_int),
		fjson_object_is_type(o, fjson_type_object),
		fjson_object_is_type(o, fjson_type_array),
		fjson_object_is_type(o, fjson_type_string));
}
