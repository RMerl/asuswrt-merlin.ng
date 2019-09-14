/*
 * Copyright (c) 2004, 2005 Metaparadigm Pte. Ltd.
 * Michael Clark <michael@metaparadigm.com>
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See COPYING for details.
 *
 */

#ifndef _fj_json_util_h_
#define _fj_json_util_h_

#include "json_object.h"

#ifndef fjson_min
#define fjson_min(a,b) ((a) < (b) ? (a) : (b))
#endif

#ifndef fjson_max
#define fjson_max(a,b) ((a) > (b) ? (a) : (b))
#endif


#ifdef __cplusplus
extern "C" {
#endif

#define FJSON_FILE_BUF_SIZE 4096

/* utility functions */
extern struct fjson_object* fjson_object_from_file(const char *filename);
extern struct fjson_object* fjson_object_from_fd(int fd);
extern int fjson_object_to_file(const char *filename, struct fjson_object *obj);
extern int fjson_object_to_file_ext(const char *filename, struct fjson_object *obj, int flags);
extern int fjson_parse_int64(const char *buf, int64_t *retval);
extern int fjson_parse_double(const char *buf, double *retval);

/**
 * Return a string describing the type of the object.
 * e.g. "int", or "object", etc...
 */
extern const char *fjson_type_to_name(enum fjson_type o_type);

#ifndef FJSON_NATIVE_API_ONLY
#define json_type_to_name fjson_type_to_name
#endif

#ifdef __cplusplus
}
#endif

#endif
