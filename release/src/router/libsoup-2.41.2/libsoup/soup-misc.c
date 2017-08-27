/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * soup-misc.c: Miscellaneous functions

 * Copyright (C) 2000-2003, Ximian, Inc.
 */

#include <string.h>

#include "soup-misc.h"
#include "soup-misc-private.h"

/**
 * SECTION:soup-misc
 * @short_description: Miscellaneous functions
 *
 **/

const gboolean soup_ssl_supported = TRUE;

/**
 * soup_str_case_hash:
 * @key: ASCII string to hash
 *
 * Hashes @key in a case-insensitive manner.
 *
 * Return value: the hash code.
 **/
guint
soup_str_case_hash (gconstpointer key)
{
	const char *p = key;
	guint h = g_ascii_toupper(*p);

	if (h)
		for (p += 1; *p != '\0'; p++)
			h = (h << 5) - h + g_ascii_toupper(*p);

	return h;
}

/**
 * soup_str_case_equal:
 * @v1: an ASCII string
 * @v2: another ASCII string
 *
 * Compares @v1 and @v2 in a case-insensitive manner
 *
 * Return value: %TRUE if they are equal (modulo case)
 **/
gboolean
soup_str_case_equal (gconstpointer v1,
		     gconstpointer v2)
{
	const char *string1 = v1;
	const char *string2 = v2;

	return g_ascii_strcasecmp (string1, string2) == 0;
}

/**
 * soup_add_io_watch: (skip)
 * @async_context: (allow-none): the #GMainContext to dispatch the I/O
 * watch in, or %NULL for the default context
 * @chan: the #GIOChannel to watch
 * @condition: the condition to watch for
 * @function: the callback to invoke when @condition occurs
 * @data: user data to pass to @function
 *
 * Adds an I/O watch as with g_io_add_watch(), but using the given
 * @async_context.
 *
 * Return value: a #GSource, which can be removed from @async_context
 * with g_source_destroy().
 **/
GSource *
soup_add_io_watch (GMainContext *async_context,
		   GIOChannel *chan, GIOCondition condition,
		   GIOFunc function, gpointer data)
{
	GSource *watch = g_io_create_watch (chan, condition);
	g_source_set_callback (watch, (GSourceFunc) function, data, NULL);
	g_source_attach (watch, async_context);
	g_source_unref (watch);
	return watch;
}

/**
 * soup_add_idle: (skip)
 * @async_context: (allow-none): the #GMainContext to dispatch the I/O
 * watch in, or %NULL for the default context
 * @function: the callback to invoke at idle time
 * @data: user data to pass to @function
 *
 * Adds an idle event as with g_idle_add(), but using the given
 * @async_context.
 *
 * If you want @function to run "right away", use
 * soup_add_completion(), since that sets a higher priority on the
 * #GSource than soup_add_idle() does.
 *
 * Return value: a #GSource, which can be removed from @async_context
 * with g_source_destroy().
 **/
GSource *
soup_add_idle (GMainContext *async_context,
	       GSourceFunc function, gpointer data)
{
	GSource *source = g_idle_source_new ();
	g_source_set_callback (source, function, data, NULL);
	g_source_attach (source, async_context);
	g_source_unref (source);
	return source;
}

GSource *
soup_add_completion_reffed (GMainContext *async_context,
			    GSourceFunc   function,
			    gpointer      data)
{
	GSource *source = g_idle_source_new ();

	g_source_set_priority (source, G_PRIORITY_DEFAULT);
	g_source_set_callback (source, function, data, NULL);
	g_source_attach (source, async_context);
	return source;
}

/**
 * soup_add_completion: (skip)
 * @async_context: (allow-none): the #GMainContext to dispatch the I/O
 * watch in, or %NULL for the default context
 * @function: the callback to invoke
 * @data: user data to pass to @function
 *
 * Adds @function to be executed from inside @async_context with the
 * default priority. Use this when you want to complete an action in
 * @async_context's main loop, as soon as possible.
 *
 * Return value: a #GSource, which can be removed from @async_context
 * with g_source_destroy().
 *
 * Since: 2.24
 **/
GSource *
soup_add_completion (GMainContext *async_context,
	             GSourceFunc function, gpointer data)
{
	GSource *source;

	source = soup_add_completion_reffed (async_context, function, data);
	g_source_unref (source);
	return source;
}

/**
 * soup_add_timeout: (skip)
 * @async_context: (allow-none): the #GMainContext to dispatch the I/O
 * watch in, or %NULL for the default context
 * @interval: the timeout interval, in milliseconds
 * @function: the callback to invoke at timeout time
 * @data: user data to pass to @function
 *
 * Adds a timeout as with g_timeout_add(), but using the given
 * @async_context.
 *
 * Return value: a #GSource, which can be removed from @async_context
 * with g_source_destroy().
 **/
GSource *
soup_add_timeout (GMainContext *async_context,
		  guint interval,
		  GSourceFunc function, gpointer data)
{
	GSource *source = g_timeout_source_new (interval);
	g_source_set_callback (source, function, data, NULL);
	g_source_attach (source, async_context);
	g_source_unref (source);
	return source;
}

/* 00 URI_UNRESERVED
 * 01 URI_PCT_ENCODED
 * 02 URI_GEN_DELIMS
 * 04 URI_SUB_DELIMS
 * 08 HTTP_SEPARATOR
 * 10 HTTP_CTL
 */
const char soup_char_attributes[] = {
	/* 0x00 - 0x07 */
	0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,
	/* 0x08 - 0x0f */
	0x11, 0x19, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,
	/* 0x10 - 0x17 */
	0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,
	/* 0x18 - 0x1f */
	0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,
	/*  !"#$%&' */
	0x09, 0x04, 0x09, 0x02, 0x04, 0x01, 0x04, 0x04,
	/* ()*+,-./ */
	0x0c, 0x0c, 0x04, 0x04, 0x0c, 0x00, 0x00, 0x0a,
	/* 01234567 */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* 89:;<=>? */
	0x00, 0x00, 0x0a, 0x0c, 0x09, 0x0a, 0x09, 0x0a,
	/* @ABCDEFG */
	0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* HIJKLMNO */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* PQRSTUVW */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* XYZ[\]^_ */
	0x00, 0x00, 0x00, 0x0a, 0x09, 0x0a, 0x01, 0x00,
	/* `abcdefg */
	0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* hijklmno */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* pqrstuvw */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* xyz{|}~  */
	0x00, 0x00, 0x00, 0x09, 0x01, 0x09, 0x00, 0x11,
	/* 0x80 - 0xFF */
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01
};
