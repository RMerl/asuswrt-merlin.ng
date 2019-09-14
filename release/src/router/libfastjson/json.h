/*
 * Copyright (c) 2004, 2005 Metaparadigm Pte. Ltd.
 * Michael Clark <michael@metaparadigm.com>
 * Copyright (c) 2009 Hewlett-Packard Development Company, L.P.
 * Copyright (c) 2016 Adiscon GmbH
 * Rainer Gerhards <rgerhards@adiscon.com>
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See COPYING for details.
 *
 */

#ifndef _fj_json_h_
#define _fj_json_h_

#ifdef __cplusplus
extern "C" {
#endif

#include "json_util.h"
#include "json_object.h"
#include "json_tokener.h"
#include "json_object_iterator.h"

/**
 * Set initial size allocation for memory when creating strings,
 * as is done for example in fjson_object_to_json_string(). The
 * default size is 32, which is very conservative. If an app
 * knows it typically deals with larger strings, performance
 * can be improved by setting the initial size to a different
 * number, e.g. 1k. Note that this also means that memory
 * consumption can increase. How far entriely depens on the
 * application and its use of json-c.
 *
 * Note: each time this function is called, the initial size is
 * changed to the given value. Already existing elements are not
 * affected. This function is usually meant to be called just once
 * at start of an application, but there is no harm calling it more
 * than once. Note that the function is NOT thread-safe and must not
 * be called on different threads concurrently.
 *
 * @param size new initial size for printbuf (formatting buffer)
 */
extern void fjson_global_set_printbuf_initial_size(int size);

/**
 * Set case sensitive/insensitive comparison mode. If set to 0,
 * comparisons for JSON keys will be case-insensitive. Otherwise,
 * they will be case-sensitive.
 * NOTE: the JSON standard demands case sensitivity. By turning
 * this off, the JSON standard is not obeyed. Most importantly,
 * if keys exists which only differ in case, only partial data
 * access is possible. So use with care and only if you know
 * exactly what you are doing!
 */
extern void fjson_global_do_case_sensitive_comparison(const int newval);

/**
 * report the current libfastjson version
 */
extern const char *fjson_version(void);

/**
 * default string hash function
 */
#define FJSON_STR_HASH_DFLT 0

/**
 * perl-like string hash function
 */
#define FJSON_STR_HASH_PERLLIKE 1

#ifndef FJSON_NATIVE_API_ONLY
#define JSON_C_STR_HASH_PERLLIKE FJSON_STR_HASH_PERLLIKE
#define json_global_set_string_hash(x) /**<< no longer exists nor is needed */
#define fjson_global_set_string_hash(x) /**<< no longer exists nor is needed */
#endif

#ifdef __cplusplus
}
#endif

#endif
