/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * soup-method.c: declarations of _SOUP_METHOD_* variables
 *
 * Copyright (C) 2008 Red Hat, Inc.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "soup-method.h"

/* Explicit assignment to NULL is to help the OS X linker not be
 * stupid. #522957
 */
gpointer _SOUP_METHOD_CONNECT = NULL;
gpointer _SOUP_METHOD_COPY = NULL;
gpointer _SOUP_METHOD_DELETE = NULL;
gpointer _SOUP_METHOD_GET = NULL;
gpointer _SOUP_METHOD_HEAD = NULL;
gpointer _SOUP_METHOD_LOCK = NULL;
gpointer _SOUP_METHOD_MKCOL = NULL;
gpointer _SOUP_METHOD_MOVE = NULL;
gpointer _SOUP_METHOD_OPTIONS = NULL;
gpointer _SOUP_METHOD_POST = NULL;
gpointer _SOUP_METHOD_PROPFIND = NULL;
gpointer _SOUP_METHOD_PROPPATCH = NULL;
gpointer _SOUP_METHOD_PUT = NULL;
gpointer _SOUP_METHOD_TRACE = NULL;
gpointer _SOUP_METHOD_UNLOCK = NULL;

/**
 * SOUP_METHOD_OPTIONS:
 *
 * "OPTIONS" as an interned string.
 **/
/**
 * SOUP_METHOD_GET:
 *
 * "GET" as an interned string.
 **/
/**
 * SOUP_METHOD_HEAD:
 *
 * "HEAD" as an interned string.
 **/
/**
 * SOUP_METHOD_POST:
 *
 * "POST" as an interned string.
 **/
/**
 * SOUP_METHOD_PUT:
 *
 * "PUT" as an interned string.
 **/
/**
 * SOUP_METHOD_DELETE:
 *
 * "DELETE" as an interned string.
 **/
/**
 * SOUP_METHOD_TRACE:
 *
 * "TRACE" as an interned string.
 **/
/**
 * SOUP_METHOD_CONNECT:
 *
 * "CONNECT" as an interned string.
 **/
/**
 * SOUP_METHOD_PROPFIND:
 *
 * "PROPFIND" as an interned string.
 **/
/**
 * SOUP_METHOD_PROPPATCH:
 *
 * "PROPPATCH" as an interned string.
 **/
/**
 * SOUP_METHOD_MKCOL:
 *
 * "MKCOL" as an interned string.
 **/
/**
 * SOUP_METHOD_COPY:
 *
 * "COPY" as an interned string.
 **/
/**
 * SOUP_METHOD_MOVE:
 *
 * "MOVE" as an interned string.
 **/
/**
 * SOUP_METHOD_LOCK:
 *
 * "LOCK" as an interned string.
 **/
/**
 * SOUP_METHOD_UNLOCK:
 *
 * "UNLOCK" as an interned string.
 **/
