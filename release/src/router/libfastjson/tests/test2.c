#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#include "../json.h"
#include "../debug.h"
#include "parse_flags.h"

#ifdef TEST_FORMATTED
#define fjson_object_to_json_string(obj) fjson_object_to_json_string_ext(obj,sflags)
#else
/* no special define */
#endif


int main(int __attribute__((unused)) argc, char __attribute__((unused)) **argv)
{
	fjson_object *new_obj;
#ifdef TEST_FORMATTED
	int sflags = 0;
#endif

	MC_SET_DEBUG(1);

#ifdef TEST_FORMATTED
	sflags = parse_flags(argc, argv);
#endif

	new_obj = fjson_tokener_parse("/* more difficult test case */ { \"glossary\": { \"title\": \"example glossary\", \"GlossDiv\": { \"title\": \"S\", \"GlossList\": [ { \"ID\": \"SGML\", \"SortAs\": \"SGML\", \"GlossTerm\": \"Standard Generalized Markup Language\", \"Acronym\": \"SGML\", \"Abbrev\": \"ISO 8879:1986\", \"GlossDef\": \"A meta-markup language, used to create markup languages such as DocBook.\", \"GlossSeeAlso\": [\"GML\", \"XML\", \"markup\"] } ] } } }");
	printf("new_obj.to_string()=%s\n", fjson_object_to_json_string(new_obj));
	fjson_object_put(new_obj);

	return 0;
}
