/* Copyright (C) 2016 by Rainer Gerhards 
 * Released under ASL 2.0 */
#include "config.h"
#include <stdio.h>
#include "../json_object.h"
#include "../json_tokener.h"
int main(void)
{
  const char *s;
  json_object *json;

  s = "{ \"foo\" : \"\u00a9\" }";

  printf("string = %s\n", s);
  json = json_tokener_parse(s);
  printf("json = %s\n", json_object_to_json_string_ext(json, JSON_C_TO_STRING_PRETTY));

  json_object_put(json);
}
