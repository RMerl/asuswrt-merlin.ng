/* Copyright (C) 2016 by Rainer Gerhards 
 * Released under ASL 2.0 */
#include "config.h"
#include <stdio.h>
#include "../json_object.h"
#include "../json_tokener.h"
int main(void)
{
  fjson_object *json;

  json = fjson_object_new_double(1.0);
  printf("json = %s\n", json_object_to_json_string_ext(json, JSON_C_TO_STRING_PRETTY));
  json_object_put(json);
  json = fjson_object_new_double(1.23);
  printf("json = %s\n", json_object_to_json_string_ext(json, JSON_C_TO_STRING_PRETTY));
  json_object_put(json);
  json = fjson_object_new_double(123456789.0);
  printf("json = %s\n", json_object_to_json_string_ext(json, JSON_C_TO_STRING_PRETTY));
  json_object_put(json);
  json = fjson_object_new_double(123456789.123);
  printf("json = %s\n", json_object_to_json_string_ext(json, JSON_C_TO_STRING_PRETTY));
  json_object_put(json);
  return 0;
}
