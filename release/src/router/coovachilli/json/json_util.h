#ifndef _json_util_h_
#define _json_util_h_

#include "json_object.h"


#define JSON_FILE_BUF_SIZE 4096

/* utlitiy functions */
extern struct json_object* json_object_from_file(char *filename);
extern int json_object_to_file(char *filename, struct json_object *obj);

#endif
