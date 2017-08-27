/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * soup-xmlrpc.c: XML-RPC parser/generator
 *
 * Copyright (C) 2007 Red Hat, Inc.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>

#include <libxml/tree.h>

#include "soup-xmlrpc.h"
#include "soup.h"

/**
 * SECTION:soup-xmlrpc
 * @short_description: XML-RPC support
 *
 **/

static xmlNode *find_real_node (xmlNode *node);

static gboolean insert_value (xmlNode *parent, GValue *value);

static gboolean
insert_value (xmlNode *parent, GValue *value)
{
	GType type = G_VALUE_TYPE (value);
	xmlNode *xvalue;
	char buf[128];

	xvalue = xmlNewChild (parent, NULL, (const xmlChar *)"value", NULL);

	if (type == G_TYPE_INT) {
		snprintf (buf, sizeof (buf), "%d", g_value_get_int (value));
		xmlNewChild (xvalue, NULL,
			     (const xmlChar *)"int",
			     (const xmlChar *)buf);
	} else if (type == G_TYPE_BOOLEAN) {
		snprintf (buf, sizeof (buf), "%d", g_value_get_boolean (value));
		xmlNewChild (xvalue, NULL,
			     (const xmlChar *)"boolean",
			     (const xmlChar *)buf);
	} else if (type == G_TYPE_STRING) {
		xmlNewTextChild (xvalue, NULL,
				 (const xmlChar *)"string",
				 (const xmlChar *)g_value_get_string (value));
	} else if (type == G_TYPE_DOUBLE) {
		g_ascii_dtostr (buf, sizeof (buf), g_value_get_double (value));
		xmlNewChild (xvalue, NULL,
			     (const xmlChar *)"double",
			     (const xmlChar *)buf);
	} else if (type == SOUP_TYPE_DATE) {
		SoupDate *date = g_value_get_boxed (value);
		char *timestamp = soup_date_to_string (date, SOUP_DATE_ISO8601_XMLRPC);
		xmlNewChild (xvalue, NULL,
			     (const xmlChar *)"dateTime.iso8601",
			     (const xmlChar *)timestamp);
		g_free (timestamp);
	} else if (type == SOUP_TYPE_BYTE_ARRAY) {
		GByteArray *ba = g_value_get_boxed (value);
		char *encoded;

		encoded = g_base64_encode (ba->data, ba->len);
		xmlNewChild (xvalue, NULL,
			     (const xmlChar *)"base64",
			     (const xmlChar *)encoded);
		g_free (encoded);
	} else if (type == G_TYPE_HASH_TABLE) {
		GHashTable *hash = g_value_get_boxed (value);
		GHashTableIter iter;
		gpointer mname, mvalue;
		xmlNode *struct_node, *member;

		struct_node = xmlNewChild (xvalue, NULL,
					   (const xmlChar *)"struct", NULL);

		g_hash_table_iter_init (&iter, hash);

		while (g_hash_table_iter_next (&iter, &mname, &mvalue)) {
			member = xmlNewChild (struct_node, NULL,
					      (const xmlChar *)"member", NULL);
			xmlNewTextChild (member, NULL,
					 (const xmlChar *)"name",
					 (const xmlChar *)mname);
			if (!insert_value (member, mvalue)) {
				xmlFreeNode (struct_node);
				struct_node = NULL;
				break;
			}
		}

		if (!struct_node)
			return FALSE;
#ifdef G_GNUC_BEGIN_IGNORE_DEPRECATIONS
G_GNUC_BEGIN_IGNORE_DEPRECATIONS
#endif
	} else if (type == G_TYPE_VALUE_ARRAY) {
#ifdef G_GNUC_END_IGNORE_DEPRECATIONS
G_GNUC_END_IGNORE_DEPRECATIONS
#endif
		GValueArray *va = g_value_get_boxed (value);
		xmlNode *node;
		int i;

		node = xmlNewChild (xvalue, NULL,
				    (const xmlChar *)"array", NULL);
		node = xmlNewChild (node, NULL,
				    (const xmlChar *)"data", NULL);
		for (i = 0; i < va->n_values; i++) {
			if (!insert_value (node, &va->values[i]))
				return FALSE;
		}
	} else
		return FALSE;

	return TRUE;
}

/**
 * soup_xmlrpc_build_method_call:
 * @method_name: the name of the XML-RPC method
 * @params: (array length=n_params): arguments to @method
 * @n_params: length of @params
 *
 * This creates an XML-RPC methodCall and returns it as a string.
 * This is the low-level method that soup_xmlrpc_request_new() is
 * built on.
 *
 * @params is an array of #GValue representing the parameters to
 * @method. (It is *not* a #GValueArray, although if you have a
 * #GValueArray, you can just pass its <literal>values</literal>f and
 * <literal>n_values</literal> fields.)
 *
 * The correspondence between glib types and XML-RPC types is:
 *
 *   int: #int (%G_TYPE_INT)
 *   boolean: #gboolean (%G_TYPE_BOOLEAN)
 *   string: #char* (%G_TYPE_STRING)
 *   double: #double (%G_TYPE_DOUBLE)
 *   datetime.iso8601: #SoupDate (%SOUP_TYPE_DATE)
 *   base64: #GByteArray (%SOUP_TYPE_BYTE_ARRAY)
 *   struct: #GHashTable (%G_TYPE_HASH_TABLE)
 *   array: #GValueArray (%G_TYPE_VALUE_ARRAY)
 *
 * For structs, use a #GHashTable that maps strings to #GValue;
 * soup_value_hash_new() and related methods can help with this.
 *
 * Return value: the text of the methodCall, or %NULL on error
 **/
char *
soup_xmlrpc_build_method_call (const char *method_name,
			       GValue *params, int n_params)
{
	xmlDoc *doc;
	xmlNode *node, *param;
	xmlChar *xmlbody;
	int i, len;
	char *body;

	doc = xmlNewDoc ((const xmlChar *)"1.0");
	doc->standalone = FALSE;
	doc->encoding = xmlCharStrdup ("UTF-8");

	node = xmlNewDocNode (doc, NULL, (const xmlChar *)"methodCall", NULL);
	xmlDocSetRootElement (doc, node);
	xmlNewChild (node, NULL, (const xmlChar *)"methodName",
		     (const xmlChar *)method_name);

	node = xmlNewChild (node, NULL, (const xmlChar *)"params", NULL);
	for (i = 0; i < n_params; i++) {
		param  = xmlNewChild (node, NULL,
				      (const xmlChar *)"param", NULL);
		if (!insert_value (param, &params[i])) {
			xmlFreeDoc (doc);
			return NULL;
		}
	}

	xmlDocDumpMemory (doc, &xmlbody, &len);
	body = g_strndup ((char *)xmlbody, len);
	xmlFree (xmlbody);
	xmlFreeDoc (doc);
	return body;
}

static SoupMessage *
soup_xmlrpc_request_newv (const char *uri, const char *method_name, va_list args)
{
	SoupMessage *msg;
	GValueArray *params;
	char *body;

	params = soup_value_array_from_args (args);
	if (!params)
		return NULL;

	body = soup_xmlrpc_build_method_call (method_name, params->values,
					      params->n_values);
#ifdef G_GNUC_BEGIN_IGNORE_DEPRECATIONS
G_GNUC_BEGIN_IGNORE_DEPRECATIONS
#endif
	g_value_array_free (params);
#ifdef G_GNUC_END_IGNORE_DEPRECATIONS
G_GNUC_END_IGNORE_DEPRECATIONS
#endif
	if (!body)
		return NULL;

	msg = soup_message_new ("POST", uri);
	soup_message_set_request (msg, "text/xml", SOUP_MEMORY_TAKE,
				  body, strlen (body));
	return msg;
}

/**
 * soup_xmlrpc_request_new:
 * @uri: URI of the XML-RPC service
 * @method_name: the name of the XML-RPC method to invoke at @uri
 * @...: parameters for @method
 *
 * Creates an XML-RPC methodCall and returns a #SoupMessage, ready
 * to send, for that method call.
 *
 * The parameters are passed as type/value pairs; ie, first a #GType,
 * and then a value of the appropriate type, finally terminated by
 * %G_TYPE_INVALID.
 *
 * Return value: (transfer full): a #SoupMessage encoding the
 * indicated XML-RPC request.
 **/
SoupMessage *
soup_xmlrpc_request_new (const char *uri, const char *method_name, ...)
{
	SoupMessage *msg;
	va_list args;

	va_start (args, method_name);
	msg = soup_xmlrpc_request_newv (uri, method_name, args);
	va_end (args);
	return msg;
}

/**
 * soup_xmlrpc_build_method_response:
 * @value: the return value
 *
 * This creates a (successful) XML-RPC methodResponse and returns it
 * as a string. To create a fault response, use
 * soup_xmlrpc_build_fault().
 *
 * The glib type to XML-RPC type mapping is as with
 * soup_xmlrpc_build_method_call(), qv.
 *
 * Return value: the text of the methodResponse, or %NULL on error
 **/
char *
soup_xmlrpc_build_method_response (GValue *value)
{
	xmlDoc *doc;
	xmlNode *node;
	xmlChar *xmlbody;
	char *body;
	int len;

	doc = xmlNewDoc ((const xmlChar *)"1.0");
	doc->standalone = FALSE;
	doc->encoding = xmlCharStrdup ("UTF-8");

	node = xmlNewDocNode (doc, NULL,
			      (const xmlChar *)"methodResponse", NULL);
	xmlDocSetRootElement (doc, node);

	node = xmlNewChild (node, NULL, (const xmlChar *)"params", NULL);
	node = xmlNewChild (node, NULL, (const xmlChar *)"param", NULL);
	if (!insert_value (node, value)) {
		xmlFreeDoc (doc);
		return NULL;
	}

	xmlDocDumpMemory (doc, &xmlbody, &len);
	body = g_strndup ((char *)xmlbody, len);
	xmlFree (xmlbody);
	xmlFreeDoc (doc);
	return body;
}

static char *
soup_xmlrpc_build_faultv (int fault_code, const char *fault_format, va_list args)
{
	xmlDoc *doc;
	xmlNode *node, *member;
	GValue value;
	xmlChar *xmlbody;
	char *fault_string, *body;
	int len;

	fault_string = g_strdup_vprintf (fault_format, args);

	doc = xmlNewDoc ((const xmlChar *)"1.0");
	doc->standalone = FALSE;
	doc->encoding = xmlCharStrdup ("UTF-8");

	node = xmlNewDocNode (doc, NULL,
			      (const xmlChar *)"methodResponse", NULL);
	xmlDocSetRootElement (doc, node);
	node = xmlNewChild (node, NULL, (const xmlChar *)"fault", NULL);
	node = xmlNewChild (node, NULL, (const xmlChar *)"value", NULL);
	node = xmlNewChild (node, NULL, (const xmlChar *)"struct", NULL);

	memset (&value, 0, sizeof (value));

	member = xmlNewChild (node, NULL, (const xmlChar *)"member", NULL);
	xmlNewChild (member, NULL,
		     (const xmlChar *)"name", (const xmlChar *)"faultCode");
	g_value_init (&value, G_TYPE_INT);
	g_value_set_int (&value, fault_code);
	insert_value (member, &value);
	g_value_unset (&value);

	member = xmlNewChild (node, NULL, (const xmlChar *)"member", NULL);
	xmlNewChild (member, NULL,
		     (const xmlChar *)"name", (const xmlChar *)"faultString");
	g_value_init (&value, G_TYPE_STRING);
	g_value_take_string (&value, fault_string);
	insert_value (member, &value);
	g_value_unset (&value);

	xmlDocDumpMemory (doc, &xmlbody, &len);
	body = g_strndup ((char *)xmlbody, len);
	xmlFree (xmlbody);
	xmlFreeDoc (doc);

	return body;
}

/**
 * soup_xmlrpc_build_fault:
 * @fault_code: the fault code
 * @fault_format: a printf()-style format string
 * @...: the parameters to @fault_format
 *
 * This creates an XML-RPC fault response and returns it as a string.
 * (To create a successful response, use
 * soup_xmlrpc_build_method_response().)
 *
 * Return value: the text of the fault
 **/
char *
soup_xmlrpc_build_fault (int fault_code, const char *fault_format, ...)
{
	va_list args;
	char *body;

	va_start (args, fault_format);
	body = soup_xmlrpc_build_faultv (fault_code, fault_format, args);
	va_end (args);
	return body;
}

/**
 * soup_xmlrpc_set_response:
 * @msg: an XML-RPC request
 * @type: the type of the response value
 * @...: the response value
 *
 * Sets the status code and response body of @msg to indicate a
 * successful XML-RPC call, with a return value given by @type and the
 * following varargs argument, of the type indicated by @type.
 **/
void
soup_xmlrpc_set_response (SoupMessage *msg, GType type, ...)
{
	va_list args;
	GValue value;
	char *body;

	va_start (args, type);
	SOUP_VALUE_SETV (&value, type, args);
	va_end (args);

	body = soup_xmlrpc_build_method_response (&value);
	g_value_unset (&value);
	soup_message_set_status (msg, SOUP_STATUS_OK);
	soup_message_set_response (msg, "text/xml", SOUP_MEMORY_TAKE,
				   body, strlen (body));
}

/**
 * soup_xmlrpc_set_fault:
 * @msg: an XML-RPC request
 * @fault_code: the fault code
 * @fault_format: a printf()-style format string
 * @...: the parameters to @fault_format
 *
 * Sets the status code and response body of @msg to indicate an
 * unsuccessful XML-RPC call, with the error described by @fault_code
 * and @fault_format.
 **/
void
soup_xmlrpc_set_fault (SoupMessage *msg, int fault_code,
		       const char *fault_format, ...)
{
	va_list args;
	char *body;

	va_start (args, fault_format);
	body = soup_xmlrpc_build_faultv (fault_code, fault_format, args);
	va_end (args);

	soup_message_set_status (msg, SOUP_STATUS_OK);
	soup_message_set_response (msg, "text/xml", SOUP_MEMORY_TAKE,
				   body, strlen (body));
}



static gboolean
parse_value (xmlNode *xmlvalue, GValue *value)
{
	xmlNode *typenode;
	const char *typename;
	xmlChar *content;

	memset (value, 0, sizeof (GValue));

	typenode = find_real_node (xmlvalue->children);
	if (!typenode) {
		/* If no type node, it's a string */
		content = xmlNodeGetContent (typenode);
		g_value_init (value, G_TYPE_STRING);
		g_value_set_string (value, (char *)content);
		xmlFree (content);
		return TRUE;
	}

	typename = (const char *)typenode->name;

	if (!strcmp (typename, "i4") || !strcmp (typename, "int")) {
		content = xmlNodeGetContent (typenode);
		g_value_init (value, G_TYPE_INT);
		g_value_set_int (value, atoi ((char *)content));
		xmlFree (content);
	} else if (!strcmp (typename, "boolean")) {
		content = xmlNodeGetContent (typenode);
		g_value_init (value, G_TYPE_BOOLEAN);
		g_value_set_boolean (value, atoi ((char *)content));
		xmlFree (content);
	} else if (!strcmp (typename, "string")) {
		content = xmlNodeGetContent (typenode);
		g_value_init (value, G_TYPE_STRING);
		g_value_set_string (value, (char *)content);
		xmlFree (content);
	} else if (!strcmp (typename, "double")) {
		content = xmlNodeGetContent (typenode);
		g_value_init (value, G_TYPE_DOUBLE);
		g_value_set_double (value, g_ascii_strtod ((char *)content, NULL));
		xmlFree (content);
	} else if (!strcmp (typename, "dateTime.iso8601")) {
		content = xmlNodeGetContent (typenode);
		g_value_init (value, SOUP_TYPE_DATE);
		g_value_take_boxed (value, soup_date_new_from_string ((char *)content));
		xmlFree (content);
	} else if (!strcmp (typename, "base64")) {
		GByteArray *ba;
		guchar *decoded;
		gsize len;

		content = xmlNodeGetContent (typenode);
		decoded = g_base64_decode ((char *)content, &len);
		ba = g_byte_array_sized_new (len);
		g_byte_array_append (ba, decoded, len);
		g_free (decoded);
		xmlFree (content);
		g_value_init (value, SOUP_TYPE_BYTE_ARRAY);
		g_value_take_boxed (value, ba);
	} else if (!strcmp (typename, "struct")) {
		xmlNode *member, *child, *mname, *mxval;
		GHashTable *hash;
		GValue mgval;
		
		hash = soup_value_hash_new ();
		for (member = find_real_node (typenode->children);
		     member;
		     member = find_real_node (member->next)) {
			if (strcmp ((const char *)member->name, "member") != 0) {
				g_hash_table_destroy (hash);
				return FALSE;
			}
			mname = mxval = NULL;
			memset (&mgval, 0, sizeof (mgval));

			for (child = find_real_node (member->children);
			     child;
			     child = find_real_node (child->next)) {
				if (!strcmp ((const char *)child->name, "name"))
					mname = child;
				else if (!strcmp ((const char *)child->name, "value"))
					mxval = child;
				else
					break;
			}

			if (!mname || !mxval || !parse_value (mxval, &mgval)) {
				g_hash_table_destroy (hash);
				return FALSE;
			}

			content = xmlNodeGetContent (mname);
			soup_value_hash_insert_value (hash, (char *)content, &mgval);
			xmlFree (content);
			g_value_unset (&mgval);
		}
		g_value_init (value, G_TYPE_HASH_TABLE);
		g_value_take_boxed (value, hash);
	} else if (!strcmp (typename, "array")) {
		xmlNode *data, *xval;
		GValueArray *array;
		GValue gval;

		data = find_real_node (typenode->children);
		if (!data || strcmp ((const char *)data->name, "data") != 0)
			return FALSE;

#ifdef G_GNUC_BEGIN_IGNORE_DEPRECATIONS
G_GNUC_BEGIN_IGNORE_DEPRECATIONS
#endif
		array = g_value_array_new (1);
		for (xval = find_real_node (data->children);
		     xval;
		     xval = find_real_node (xval->next)) {
			memset (&gval, 0, sizeof (gval));
			if (strcmp ((const char *)xval->name, "value") != 0 ||
			    !parse_value (xval, &gval)) {
				g_value_array_free (array);
				return FALSE;
			}

			g_value_array_append (array, &gval);
			g_value_unset (&gval);
		}
		g_value_init (value, G_TYPE_VALUE_ARRAY);
		g_value_take_boxed (value, array);
#ifdef G_GNUC_END_IGNORE_DEPRECATIONS
G_GNUC_END_IGNORE_DEPRECATIONS
#endif
	} else
		return FALSE;

	return TRUE;
}

/**
 * soup_xmlrpc_parse_method_call:
 * @method_call: the XML-RPC methodCall string
 * @length: the length of @method_call, or -1 if it is NUL-terminated
 * @method_name: (out): on return, the methodName from @method_call
 * @params: (out): on return, the parameters from @method_call
 *
 * Parses @method_call to get the name and parameters, and returns the
 * parameter values in a #GValueArray; see also
 * soup_xmlrpc_extract_method_call(), which is more convenient if you
 * know in advance what the types of the parameters will be.
 *
 * Return value: success or failure.
 **/
gboolean
soup_xmlrpc_parse_method_call (const char *method_call, int length,
			       char **method_name, GValueArray **params)
{
	xmlDoc *doc;
	xmlNode *node, *param, *xval;
	xmlChar *xmlMethodName = NULL;
	gboolean success = FALSE;
	GValue value;

	doc = xmlParseMemory (method_call,
			      length == -1 ? strlen (method_call) : length);
	if (!doc)
		return FALSE;

	node = xmlDocGetRootElement (doc);
	if (!node || strcmp ((const char *)node->name, "methodCall") != 0)
		goto fail;

	node = find_real_node (node->children);
	if (!node || strcmp ((const char *)node->name, "methodName") != 0)
		goto fail;
	xmlMethodName = xmlNodeGetContent (node);

	node = find_real_node (node->next);
	if (node) {
		if (strcmp ((const char *)node->name, "params") != 0)
			goto fail;

#ifdef G_GNUC_BEGIN_IGNORE_DEPRECATIONS
G_GNUC_BEGIN_IGNORE_DEPRECATIONS
#endif
		*params = soup_value_array_new ();
		param = find_real_node (node->children);
		while (param && !strcmp ((const char *)param->name, "param")) {
			xval = find_real_node (param->children);
			if (!xval || strcmp ((const char *)xval->name, "value") != 0 ||
			    !parse_value (xval, &value)) {
				g_value_array_free (*params);
				goto fail;
			}
			g_value_array_append (*params, &value);
			g_value_unset (&value);

			param = find_real_node (param->next);
		}
#ifdef G_GNUC_END_IGNORE_DEPRECATIONS
G_GNUC_END_IGNORE_DEPRECATIONS
#endif
	} else
		*params = soup_value_array_new ();

	success = TRUE;
	*method_name = g_strdup ((char *)xmlMethodName);

fail:
	xmlFreeDoc (doc);
	if (xmlMethodName)
		xmlFree (xmlMethodName);
	return success;
}

/**
 * soup_xmlrpc_extract_method_call:
 * @method_call: the XML-RPC methodCall string
 * @length: the length of @method_call, or -1 if it is NUL-terminated
 * @method_name: (out): on return, the methodName from @method_call
 * @...: return types and locations for parameters
 *
 * Parses @method_call to get the name and parameters, and puts
 * the parameters into variables of the appropriate types.
 *
 * The parameters are handled similarly to
 * @soup_xmlrpc_build_method_call, with pairs of types and values,
 * terminated by %G_TYPE_INVALID, except that values are pointers to
 * variables of the indicated type, rather than values of the type.
 *
 * See also soup_xmlrpc_parse_method_call(), which can be used if
 * you don't know the types of the parameters.
 *
 * Return value: success or failure.
 **/
gboolean
soup_xmlrpc_extract_method_call (const char *method_call, int length,
				 char **method_name, ...)
{
	GValueArray *params;
	gboolean success;
	va_list args;

	if (!soup_xmlrpc_parse_method_call (method_call, length,
					    method_name, &params))
		return FALSE;

	va_start (args, method_name);
	success = soup_value_array_to_args (params, args);
	va_end (args);

#ifdef G_GNUC_BEGIN_IGNORE_DEPRECATIONS
G_GNUC_BEGIN_IGNORE_DEPRECATIONS
#endif
	g_value_array_free (params);
#ifdef G_GNUC_END_IGNORE_DEPRECATIONS
G_GNUC_END_IGNORE_DEPRECATIONS
#endif
	return success;
}

/**
 * soup_xmlrpc_parse_method_response:
 * @method_response: the XML-RPC methodResponse string
 * @length: the length of @method_response, or -1 if it is NUL-terminated
 * @value: (out): on return, the return value from @method_call
 * @error: error return value
 *
 * Parses @method_response and returns the return value in @value. If
 * @method_response is a fault, @value will be unchanged, and @error
 * will be set to an error of type %SOUP_XMLRPC_FAULT, with the error
 * #code containing the fault code, and the error #message containing
 * the fault string. (If @method_response cannot be parsed at all,
 * soup_xmlrpc_parse_method_response() will return %FALSE, but @error
 * will be unset.)
 *
 * Return value: %TRUE if a return value was parsed, %FALSE if the
 * response could not be parsed, or contained a fault.
 **/
gboolean
soup_xmlrpc_parse_method_response (const char *method_response, int length,
				   GValue *value, GError **error)
{
	xmlDoc *doc;
	xmlNode *node;
	gboolean success = FALSE;

	doc = xmlParseMemory (method_response,
			      length == -1 ? strlen (method_response) : length);
	if (!doc)
		return FALSE;

	node = xmlDocGetRootElement (doc);
	if (!node || strcmp ((const char *)node->name, "methodResponse") != 0)
		goto fail;

	node = find_real_node (node->children);
	if (!node)
		goto fail;

	if (!strcmp ((const char *)node->name, "fault") && error) {
		int fault_code;
		char *fault_string;
		GValue fault_val;
		GHashTable *fault_hash;

		node = find_real_node (node->children);
		if (!node || strcmp ((const char *)node->name, "value") != 0)
			goto fail;
		if (!parse_value (node, &fault_val))
			goto fail;
		if (!G_VALUE_HOLDS (&fault_val, G_TYPE_HASH_TABLE)) {
			g_value_unset (&fault_val);
			goto fail;
		}
		fault_hash = g_value_get_boxed (&fault_val);
		if (!soup_value_hash_lookup (fault_hash, "faultCode",
					     G_TYPE_INT, &fault_code) ||
		    !soup_value_hash_lookup (fault_hash, "faultString",
					     G_TYPE_STRING, &fault_string)) {
			g_value_unset (&fault_val);
			goto fail;
		}

		g_set_error (error, SOUP_XMLRPC_FAULT,
			     fault_code, "%s", fault_string);
		g_value_unset (&fault_val);
	} else if (!strcmp ((const char *)node->name, "params")) {
		node = find_real_node (node->children);
		if (!node || strcmp ((const char *)node->name, "param") != 0)
			goto fail;
		node = find_real_node (node->children);
		if (!node || strcmp ((const char *)node->name, "value") != 0)
			goto fail;
		if (!parse_value (node, value))
			goto fail;
		success = TRUE;
	}

fail:
	xmlFreeDoc (doc);
	return success;
}

/**
 * soup_xmlrpc_extract_method_response:
 * @method_response: the XML-RPC methodResponse string
 * @length: the length of @method_response, or -1 if it is NUL-terminated
 * @error: error return value
 * @type: the expected type of the return value
 * @...: location for return value
 *
 * Parses @method_response and extracts the return value into
 * a variable of the correct type.
 *
 * If @method_response is a fault, the return value will be unset,
 * and @error will be set to an error of type %SOUP_XMLRPC_FAULT, with
 * the error #code containing the fault code, and the error #message
 * containing the fault string. (If @method_response cannot be parsed
 * at all, soup_xmlrpc_extract_method_response() will return %FALSE,
 * but @error will be unset.)
 *
 * Return value: %TRUE if a return value was parsed, %FALSE if the
 * response was of the wrong type, or contained a fault.
 **/
gboolean
soup_xmlrpc_extract_method_response (const char *method_response, int length,
				     GError **error, GType type, ...)
{
	GValue value;
	va_list args;

	if (!soup_xmlrpc_parse_method_response (method_response, length,
						&value, error))
		return FALSE;
	if (!G_VALUE_HOLDS (&value, type))
		return FALSE;

	va_start (args, type);
	SOUP_VALUE_GETV (&value, type, args);
	va_end (args);

	return TRUE;
}


GQuark
soup_xmlrpc_error_quark (void)
{
	static GQuark error;
	if (!error)
		error = g_quark_from_static_string ("soup_xmlrpc_error_quark");
	return error;
}

/**
 * SOUP_XMLRPC_FAULT:
 *
 * A #GError domain representing an XML-RPC fault code. Used with
 * #SoupXMLRPCFault (although servers may also return fault codes not
 * in that enumeration).
 */

/**
 * SoupXMLRPCFault:
 * @SOUP_XMLRPC_FAULT_PARSE_ERROR_NOT_WELL_FORMED: request was not
 *   well-formed
 * @SOUP_XMLRPC_FAULT_PARSE_ERROR_UNSUPPORTED_ENCODING: request was in
 *   an unsupported encoding
 * @SOUP_XMLRPC_FAULT_PARSE_ERROR_INVALID_CHARACTER_FOR_ENCODING:
 *   request contained an invalid character
 * @SOUP_XMLRPC_FAULT_SERVER_ERROR_INVALID_XML_RPC: request was not
 *   valid XML-RPC
 * @SOUP_XMLRPC_FAULT_SERVER_ERROR_REQUESTED_METHOD_NOT_FOUND: method
 *   not found
 * @SOUP_XMLRPC_FAULT_SERVER_ERROR_INVALID_METHOD_PARAMETERS: invalid
 *   parameters
 * @SOUP_XMLRPC_FAULT_SERVER_ERROR_INTERNAL_XML_RPC_ERROR: internal
 *   error
 * @SOUP_XMLRPC_FAULT_APPLICATION_ERROR: start of reserved range for
 *   application error codes
 * @SOUP_XMLRPC_FAULT_SYSTEM_ERROR: start of reserved range for
 *   system error codes
 * @SOUP_XMLRPC_FAULT_TRANSPORT_ERROR: start of reserved range for
 *   transport error codes
 *
 * Pre-defined XML-RPC fault codes from <ulink
 * url="http://xmlrpc-epi.sourceforge.net/specs/rfc.fault_codes.php">http://xmlrpc-epi.sourceforge.net/specs/rfc.fault_codes.php</ulink>.
 * These are an extension, not part of the XML-RPC spec; you can't
 * assume servers will use them.
 */

GQuark
soup_xmlrpc_fault_quark (void)
{
	static GQuark error;
	if (!error)
		error = g_quark_from_static_string ("soup_xmlrpc_fault_quark");
	return error;
}

static xmlNode *
find_real_node (xmlNode *node)
{
	while (node && (node->type == XML_COMMENT_NODE ||
			xmlIsBlankNode (node)))
		node = node->next;
	return node;
}
