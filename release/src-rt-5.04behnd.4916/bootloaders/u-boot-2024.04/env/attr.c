// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2012
 * Joe Hershberger, National Instruments, joe.hershberger@ni.com
 */

#ifdef USE_HOSTCC /* Eliminate "ANSI does not permit..." warnings */
#include <stdint.h>
#include <stdio.h>
#include <linux/linux_string.h>
#else
#include <common.h>
#include <slre.h>
#endif

#include <env_attr.h>
#include <errno.h>
#include <linux/string.h>
#include <malloc.h>

/*
 * Iterate through the whole list calling the callback for each found element.
 * "attr_list" takes the form:
 *	attributes = [^,:\s]*
 *	entry = name[:attributes]
 *	list = entry[,list]
 */
int env_attr_walk(const char *attr_list,
	int (*callback)(const char *name, const char *attributes, void *priv),
	void *priv)
{
	const char *entry, *entry_end;
	char *name, *attributes;

	if (!attr_list)
		/* list not found */
		return 1;

	entry = attr_list;
	do {
		char *entry_cpy = NULL;

		entry_end = strchr(entry, ENV_ATTR_LIST_DELIM);
		/* check if this is the last entry in the list */
		if (entry_end == NULL) {
			int entry_len = strlen(entry);

			if (entry_len) {
				/*
				 * allocate memory to copy the entry into since
				 * we will need to inject '\0' chars and squash
				 * white-space before calling the callback
				 */
				entry_cpy = malloc(entry_len + 1);
				if (entry_cpy)
					/* copy the rest of the list */
					strcpy(entry_cpy, entry);
				else
					return -ENOMEM;
			}
		} else {
			int entry_len = entry_end - entry;

			if (entry_len) {
				/*
				 * allocate memory to copy the entry into since
				 * we will need to inject '\0' chars and squash
				 * white-space before calling the callback
				 */
				entry_cpy = malloc(entry_len + 1);
				if (entry_cpy) {
					/* copy just this entry and null term */
					strncpy(entry_cpy, entry, entry_len);
					entry_cpy[entry_len] = '\0';
				} else
					return -ENOMEM;
			}
		}

		/* check if there is anything to process (e.g. not ",,,") */
		if (entry_cpy != NULL) {
			attributes = strchr(entry_cpy, ENV_ATTR_SEP);
			/* check if there is a ':' */
			if (attributes != NULL) {
				/* replace the ':' with '\0' to term name */
				*attributes++ = '\0';
				/* remove white-space from attributes */
				attributes = strim(attributes);
			}
			/* remove white-space from name */
			name = strim(entry_cpy);

			/* only call the callback if there is a name */
			if (strlen(name) != 0) {
				int retval = 0;

				retval = callback(name, attributes, priv);
				if (retval) {
					free(entry_cpy);
					return retval;
				}
			}
		}

		free(entry_cpy);
		entry = entry_end + 1;
	} while (entry_end != NULL);

	return 0;
}

#if defined(CONFIG_REGEX)
struct regex_callback_priv {
	const char *searched_for;
	char *regex;
	char *attributes;
};

static int regex_callback(const char *name, const char *attributes, void *priv)
{
	int retval = 0;
	struct regex_callback_priv *cbp = (struct regex_callback_priv *)priv;
	struct slre slre;
	char regex[strlen(name) + 3];

	/* Require the whole string to be described by the regex */
	sprintf(regex, "^%s$", name);
	if (slre_compile(&slre, regex)) {
		struct cap caps[slre.num_caps + 2];

		if (slre_match(&slre, cbp->searched_for,
			       strlen(cbp->searched_for), caps)) {
			free(cbp->regex);
			if (!attributes) {
				retval = -EINVAL;
				goto done;
			}
			cbp->regex = malloc(strlen(regex) + 1);
			if (cbp->regex) {
				strcpy(cbp->regex, regex);
			} else {
				retval = -ENOMEM;
				goto done;
			}

			free(cbp->attributes);
			cbp->attributes = malloc(strlen(attributes) + 1);
			if (cbp->attributes) {
				strcpy(cbp->attributes, attributes);
			} else {
				retval = -ENOMEM;
				free(cbp->regex);
				cbp->regex = NULL;
				goto done;
			}
		}
	} else {
		printf("Error compiling regex: %s\n", slre.err_str);
		retval = -EINVAL;
	}
done:
	return retval;
}

/*
 * Retrieve the attributes string associated with a single name in the list
 * There is no protection on attributes being too small for the value
 */
int env_attr_lookup(const char *attr_list, const char *name, char *attributes)
{
	if (!attributes)
		/* bad parameter */
		return -EINVAL;
	if (!attr_list)
		/* list not found */
		return -EINVAL;

	struct regex_callback_priv priv;
	int retval;

	priv.searched_for = name;
	priv.regex = NULL;
	priv.attributes = NULL;
	retval = env_attr_walk(attr_list, regex_callback, &priv);
	if (retval)
		return retval; /* error */

	if (priv.regex) {
		strcpy(attributes, priv.attributes);
		free(priv.attributes);
		free(priv.regex);
		/* success */
		return 0;
	}
	return -ENOENT; /* not found in list */
}
#else

/*
 * Search for the last exactly matching name in an attribute list
 */
static int reverse_name_search(const char *searched, const char *search_for,
	const char **result)
{
	int result_size = 0;
	const char *cur_searched = searched;

	if (result)
		*result = NULL;

	if (*search_for == '\0') {
		if (result)
			*result = searched;
		return strlen(searched);
	}

	for (;;) {
		const char *match = strstr(cur_searched, search_for);
		const char *prevch;
		const char *nextch;

		/* Stop looking if no new match is found */
		if (match == NULL)
			break;

		prevch = match - 1;
		nextch = match + strlen(search_for);

		/* Skip spaces */
		while (*prevch == ' ' && prevch >= searched)
			prevch--;
		while (*nextch == ' ')
			nextch++;

		/* Start looking past the current match so last is found */
		cur_searched = match + 1;
		/* Check for an exact match */
		if (match != searched &&
		    *prevch != ENV_ATTR_LIST_DELIM &&
		    prevch != searched - 1)
			continue;
		if (*nextch != ENV_ATTR_SEP &&
		    *nextch != ENV_ATTR_LIST_DELIM &&
		    *nextch != '\0')
			continue;

		if (result)
			*result = match;
		result_size = strlen(search_for);
	}

	return result_size;
}

/*
 * Retrieve the attributes string associated with a single name in the list
 * There is no protection on attributes being too small for the value
 */
int env_attr_lookup(const char *attr_list, const char *name, char *attributes)
{
	const char *entry = NULL;
	int entry_len;

	if (!attributes)
		/* bad parameter */
		return -EINVAL;
	if (!attr_list)
		/* list not found */
		return -EINVAL;

	entry_len = reverse_name_search(attr_list, name, &entry);
	if (entry != NULL) {
		int len;

		/* skip the name */
		entry += entry_len;
		/* skip spaces */
		while (*entry == ' ')
			entry++;
		if (*entry != ENV_ATTR_SEP)
			len = 0;
		else {
			const char *delim;
			static const char delims[] = {
				ENV_ATTR_LIST_DELIM, ' ', '\0'};

			/* skip the attr sep */
			entry += 1;
			/* skip spaces */
			while (*entry == ' ')
				entry++;

			delim = strpbrk(entry, delims);
			if (delim == NULL)
				len = strlen(entry);
			else
				len = delim - entry;
			memcpy(attributes, entry, len);
		}
		attributes[len] = '\0';

		/* success */
		return 0;
	}

	/* not found in list */
	return -ENOENT;
}
#endif
