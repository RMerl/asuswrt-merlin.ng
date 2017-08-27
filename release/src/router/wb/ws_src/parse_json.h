#ifndef __PARSE_JSON_H__
#define __PARSE_JSON_H__

#include <json.h>

typedef int 	(*PROC_JSON_DATA)(void* data_struct, struct json_object * json_obj);

#endif
