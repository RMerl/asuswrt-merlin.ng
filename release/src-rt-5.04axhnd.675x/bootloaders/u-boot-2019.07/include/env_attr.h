/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2012
 * Joe Hershberger, National Instruments, joe.hershberger@ni.com
 */

#ifndef __ENV_ATTR_H__
#define __ENV_ATTR_H__

#define ENV_ATTR_LIST_DELIM	','
#define ENV_ATTR_SEP		':'

/*
 * env_attr_walk takes as input an "attr_list" that takes the form:
 *	attributes = [^,:\s]*
 *	entry = name[:attributes]
 *	list = entry[,list]
 * It will call the "callback" function with the "name" and "attributes"
 * The callback may return a non-0 to abort the list walk.
 * This return value will be passed through to the caller.
 * 0 is returned on success.
 */
int env_attr_walk(const char *attr_list,
	int (*callback)(const char *name, const char *attributes, void *priv),
	void *priv);

/*
 * env_attr_lookup takes as input an "attr_list" with the same form as above.
 * It also takes as input a "name" to look for.
 * If the name is found in the list, it's value is copied into "attributes".
 * There is no protection on attributes being too small for the value.
 * It returns -1 if attributes is NULL, 1 if "name" is not found, 2 if
 * "attr_list" is NULL.
 * Returns 0 on success.
 */
int env_attr_lookup(const char *attr_list, const char *name, char *attributes);

#endif /* __ENV_ATTR_H__ */
