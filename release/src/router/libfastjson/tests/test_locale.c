#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#include "../json.h"
#include "../json_tokener.h"
#include "../debug.h"

#ifdef HAVE_LOCALE_H
#include <locale.h>
#endif /* HAVE_LOCALE_H */

int main(int __attribute__((unused)) argc, char __attribute__((unused)) **argv)
{
	fjson_object *new_obj;
#ifdef HAVE_SETLOCALE
	setlocale(LC_NUMERIC, "de_DE");
#else
	printf("No locale\n");
#endif

	MC_SET_DEBUG(1);

	new_obj = fjson_tokener_parse("[1.2,3.4,123456.78,5.0,2.3e10]");
	printf("new_obj.to_string()=%s\n", fjson_object_to_json_string(new_obj));
	printf("new_obj.to_string()=%s\n", fjson_object_to_json_string_ext(new_obj,FJSON_TO_STRING_NOZERO));
	fjson_object_put(new_obj);
	return 0;
}

