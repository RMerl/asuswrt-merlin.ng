/**
 * \file confmisc.c
 * \ingroup Configuration
 * \brief Configuration helper functions
 * \author Abramo Bagnara <abramo@alsa-project.org>
 * \author Jaroslav Kysela <perex@perex.cz>
 * \date 2000-2001
 * 
 * Configuration helper functions.
 *
 * See the \ref conffunc page for more details.
 */
/*
 *  Miscellaneous configuration helper functions
 *  Copyright (c) 2000 by Abramo Bagnara <abramo@alsa-project.org>,
 *			  Jaroslav Kysela <perex@perex.cz>
 *
 *
 *   This library is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser General Public License as
 *   published by the Free Software Foundation; either version 2.1 of
 *   the License, or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this library; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 */

/*! \page conffunc

\section conffunc_ref Function reference

<UL>
  <LI>The getenv function - snd_func_getenv() - obtains
      an environment value. The result is a string.
  <LI>The igetenv function - snd_func_igetenv() - obtains
      an environment value. The result is an integer.
  <LI>The concat function - snd_func_concat() - merges all specified
      strings. The result is a string.
  <LI>The iadd function - snd_func_iadd() - sum all specified integers.
      The result is an integer.
  <LI>The imul function - snd_func_imul() - multiply all specified integers.
      The result is an integer.
  <LI>The datadir function - snd_func_datadir() - returns the
      ALSA data directory. The result is a string.
  <LI>The refer function - snd_func_refer() - copies the referred
      configuration. The result has the same type as the referred node.
  <LI>The card_inum function - snd_func_card_inum() - returns
      a card number (integers).
  <LI>The card_driver function - snd_func_card_driver() - returns
      a driver identification. The result is a string.
  <LI>The card_id function - snd_func_card_id() - returns
      a card identification. The result is a string.
  <LI>The card_name function - snd_func_card_name() - returns
      a card's name. The result is a string.
  <LI>The pcm_id function - snd_func_pcm_id() - returns
      a pcm identification. The result is a string.
  <LI>The private_string function - snd_func_private_string() - returns the
      string from the private_data node.
  <LI>The private_card_driver function - snd_func_private_card_driver() -
      returns the driver identification from the private_data node.
      The result is a string.
  <LI>The private_pcm_subdevice function - snd_func_private_pcm_subdevice() -
      returns the PCM subdevice number from the private_data node.
      The result is a string.
</UL>

*/


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "local.h"

/**
 * \brief Gets the boolean value from the given ASCII string.
 * \param ascii The string to be parsed.
 * \return 0 or 1 if successful, otherwise a negative error code.
 */
int snd_config_get_bool_ascii(const char *ascii)
{
	unsigned int k;
	static const struct {
		const char str[8];
		int val;
	} b[] = {
		{ "0", 0 },
		{ "1", 1 },
		{ "false", 0 },
		{ "true", 1 },
		{ "no", 0 },
		{ "yes", 1 },
		{ "off", 0 },
		{ "on", 1 },
	};
	for (k = 0; k < sizeof(b) / sizeof(*b); k++) {
		if (strcasecmp(b[k].str, ascii) == 0)
			return b[k].val;
	}
	return -EINVAL;
}

/**
 * \brief Gets the boolean value from a configuration node.
 * \param conf Handle to the configuration node to be parsed.
 * \return 0 or 1 if successful, otherwise a negative error code.
 */
int snd_config_get_bool(const snd_config_t *conf)
{
	long v;
	const char *str, *id;
	int err;

	err = snd_config_get_id(conf, &id);
	if (err < 0)
		return err;
	err = snd_config_get_integer(conf, &v);
	if (err >= 0) {
		if (v < 0 || v > 1) {
		_invalid_value:
			SNDERR("Invalid value for %s", id);
			return -EINVAL;
		}
		return v;
	}
	err = snd_config_get_string(conf, &str);
	if (err < 0) {
		SNDERR("Invalid type for %s", id);
		return -EINVAL;
	}
	err = snd_config_get_bool_ascii(str);
	if (err < 0)
		goto _invalid_value;
	return err;
}

/**
 * \brief Gets the control interface index from the given ASCII string.
 * \param ascii The string to be parsed.
 * \return The control interface index if successful, otherwise a negative error code.
 */ 
int snd_config_get_ctl_iface_ascii(const char *ascii)
{
	long v;
	snd_ctl_elem_iface_t idx;
	if (isdigit(ascii[0])) {
		if (safe_strtol(ascii, &v) >= 0) {
			if (v < 0 || v > SND_CTL_ELEM_IFACE_LAST)
				return -EINVAL;
			return v;
		}
	}
	for (idx = 0; idx <= SND_CTL_ELEM_IFACE_LAST; idx++) {
		if (strcasecmp(snd_ctl_elem_iface_name(idx), ascii) == 0)
			return idx;
	}
	return -EINVAL;
}

/**
 * \brief Gets the control interface index from a configuration node.
 * \param conf Handle to the configuration node to be parsed.
 * \return The control interface index if successful, otherwise a negative error code.
 */ 
int snd_config_get_ctl_iface(const snd_config_t *conf)
{
	long v;
	const char *str, *id;
	int err;

	err = snd_config_get_id(conf, &id);
	if (err < 0)
		return err;
	err = snd_config_get_integer(conf, &v);
	if (err >= 0) {
		if (v < 0 || v > SND_CTL_ELEM_IFACE_LAST) {
		_invalid_value:
			SNDERR("Invalid value for %s", id);
			return -EINVAL;
		}
		return v;
	}
	err = snd_config_get_string(conf, &str);
	if (err < 0) {
		SNDERR("Invalid type for %s", id);
		return -EINVAL;
	}
	err = snd_config_get_ctl_iface_ascii(str);
	if (err < 0)
		goto _invalid_value;
	return err;
}

/*
 *  Helper functions for the configuration file
 */

/**
 * \brief Returns an environment value.
 * \param dst The function puts the handle to the result configuration node
 *            (with type string) at the address specified by \p dst.
 * \param root Handle to the root source node.
 * \param src Handle to the source node, with definitions for \c vars and
 *            \c default.
 * \param private_data Handle to the \c private_data node.
 * \return Zero if successful, otherwise a negative error code.
 *
 * Example:
\code
	{
		@func getenv
		vars [ MY_CARD CARD C ]
		default 0
	}
\endcode
 */ 
int snd_func_getenv(snd_config_t **dst, snd_config_t *root, snd_config_t *src,
		    snd_config_t *private_data)
{
	snd_config_t *n, *d;
	snd_config_iterator_t i, next;
	const char *res, *id;
	char *def = NULL;
	int idx = 0, err, hit;
	
	err = snd_config_search(src, "vars", &n);
	if (err < 0) {
		SNDERR("field vars not found");
		goto __error;
	}
	err = snd_config_evaluate(n, root, private_data, NULL);
	if (err < 0) {
		SNDERR("error evaluating vars");
		goto __error;
	}
	err = snd_config_search(src, "default", &d);
	if (err < 0) {
		SNDERR("field default not found");
		goto __error;
	}
	err = snd_config_evaluate(d, root, private_data, NULL);
	if (err < 0) {
		SNDERR("error evaluating default");
		goto __error;
	}
	err = snd_config_get_ascii(d, &def);
	if (err < 0) {
		SNDERR("error getting field default");
		goto __error;
	}
	do {
		hit = 0;
		snd_config_for_each(i, next, n) {
			snd_config_t *n = snd_config_iterator_entry(i);
			const char *ptr;
			long i;
			if (snd_config_get_id(n, &id) < 0)
				continue;
			if (snd_config_get_type(n) != SND_CONFIG_TYPE_STRING) {
				SNDERR("field %s is not a string", id);
				err = -EINVAL;
				goto __error;
			}
			err = safe_strtol(id, &i);
			if (err < 0) {
				SNDERR("id of field %s is not an integer", id);
				err = -EINVAL;
				goto __error;
			}
			if (i == idx) {
				idx++;
				err = snd_config_get_string(n, &ptr);
				if (err < 0) {
					SNDERR("invalid string for id %s", id);
					err = -EINVAL;
					goto __error;
				}
				res = getenv(ptr);
				if (res != NULL && *res != '\0')
					goto __ok;
				hit = 1;
			}
		}
	} while (hit);
	res = def;
      __ok:
	err = snd_config_get_id(src, &id);
	if (err >= 0)
		err = snd_config_imake_string(dst, id, res);
      __error:
	free(def);
	return err;
}
#ifndef DOC_HIDDEN
SND_DLSYM_BUILD_VERSION(snd_func_getenv, SND_CONFIG_DLSYM_VERSION_EVALUATE);
#endif

/**
 * \brief Returns an integer environment value.
 * \param dst The function puts the handle to the result configuration node
 *            (with type integer) at the address specified by \p dst.
 * \param root Handle to the root source node.
 * \param src Handle to the source node, with definitions for \c vars and
 *            \c default.
 * \param private_data Handle to the \c private_data node.
 * \return Zero if successful, otherwise a negative error code.
 *
 * Example:
\code
	{
		@func igetenv
		vars [ MY_DEVICE DEVICE D ]
		default 0
	}
\endcode
 */ 
int snd_func_igetenv(snd_config_t **dst, snd_config_t *root, snd_config_t *src,
		     snd_config_t *private_data)
{
	snd_config_t *d;
	const char *str, *id;
	int err;
	long v;

	err = snd_func_getenv(&d, root, src, private_data);
	if (err < 0)
		return err;
	err = snd_config_get_string(d, &str);
	if (err < 0) {
		snd_config_delete(d);
		return err;
	}
	err = safe_strtol(str, &v);
	if (err < 0) {
		snd_config_delete(d);
		return err;
	}
	snd_config_delete(d);
	err = snd_config_get_id(src, &id);
	if (err < 0)
		return err;
	err = snd_config_imake_integer(dst, id, v);
	if (err < 0)
		return err;
	return 0;
}
#ifndef DOC_HIDDEN
SND_DLSYM_BUILD_VERSION(snd_func_igetenv, SND_CONFIG_DLSYM_VERSION_EVALUATE);
#endif
		
/**
 * \brief Merges the given strings.
 * \param dst The function puts the handle to the result configuration node
 *            (with type string) at the address specified by \p dst.
 * \param root Handle to the root source node.
 * \param src Handle to the source node, with a definition for \c strings.
 * \param private_data Handle to the \c private_data node.
 * \return A non-negative value if successful, otherwise a negative error code.
 *
 * Example (result is "a1b2c3"):
\code
	{
		@func concat
		strings [ "a1" "b2" "c3" ]
	}
\endcode
 */ 
int snd_func_concat(snd_config_t **dst, snd_config_t *root, snd_config_t *src,
		    snd_config_t *private_data)
{
	snd_config_t *n;
	snd_config_iterator_t i, next;
	const char *id;
	char *res = NULL, *tmp;
	int idx = 0, len = 0, len1, err, hit;
	
	err = snd_config_search(src, "strings", &n);
	if (err < 0) {
		SNDERR("field strings not found");
		goto __error;
	}
	err = snd_config_evaluate(n, root, private_data, NULL);
	if (err < 0) {
		SNDERR("error evaluating strings");
		goto __error;
	}
	do {
		hit = 0;
		snd_config_for_each(i, next, n) {
			snd_config_t *n = snd_config_iterator_entry(i);
			char *ptr;
			const char *id;
			long i;
			if (snd_config_get_id(n, &id) < 0)
				continue;
			err = safe_strtol(id, &i);
			if (err < 0) {
				SNDERR("id of field %s is not an integer", id);
				err = -EINVAL;
				goto __error;
			}
			if (i == idx) {
				idx++;
				err = snd_config_get_ascii(n, &ptr);
				if (err < 0) {
					SNDERR("invalid ascii string for id %s", id);
					err = -EINVAL;
					goto __error;
				}
				len1 = strlen(ptr);
				tmp = realloc(res, len + len1 + 1);
				if (tmp == NULL) {
					free(ptr);
					free(res);
					err = -ENOMEM;
					goto __error;
				}
				memcpy(tmp + len, ptr, len1);
				free(ptr);
				len += len1;
				tmp[len] = '\0';
				res = tmp;
				hit = 1;
			}
		}
	} while (hit);
	if (res == NULL) {
		SNDERR("empty string is not accepted");
		err = -EINVAL;
		goto __error;
	}
	err = snd_config_get_id(src, &id);
	if (err >= 0)
		err = snd_config_imake_string(dst, id, res);
	free(res);
      __error:
	return err;
}
#ifndef DOC_HIDDEN
SND_DLSYM_BUILD_VERSION(snd_func_concat, SND_CONFIG_DLSYM_VERSION_EVALUATE);
#endif


static int snd_func_iops(snd_config_t **dst,
			 snd_config_t *root,
			 snd_config_t *src,
			 snd_config_t *private_data,
			 int op)
{
	snd_config_t *n;
	snd_config_iterator_t i, next;
	const char *id;
	char *res = NULL;
	long result = 0, val;
	int idx = 0, err, hit;
	
	err = snd_config_search(src, "integers", &n);
	if (err < 0) {
		SNDERR("field integers not found");
		goto __error;
	}
	err = snd_config_evaluate(n, root, private_data, NULL);
	if (err < 0) {
		SNDERR("error evaluating integers");
		goto __error;
	}
	do {
		hit = 0;
		snd_config_for_each(i, next, n) {
			snd_config_t *n = snd_config_iterator_entry(i);
			const char *id;
			long i;
			if (snd_config_get_id(n, &id) < 0)
				continue;
			err = safe_strtol(id, &i);
			if (err < 0) {
				SNDERR("id of field %s is not an integer", id);
				err = -EINVAL;
				goto __error;
			}
			if (i == idx) {
				idx++;
				err = snd_config_get_integer(n, &val);
				if (err < 0) {
					SNDERR("invalid integer for id %s", id);
					err = -EINVAL;
					goto __error;
				}
				switch (op) {
				case 0: result += val; break;
				case 1: result *= val; break;
				}
				hit = 1;
			}
		}
	} while (hit);
	err = snd_config_get_id(src, &id);
	if (err >= 0)
		err = snd_config_imake_integer(dst, id, result);
	free(res);
      __error:
	return err;
}


/**
 * \brief Sum the given integers.
 * \param dst The function puts the handle to the result configuration node
 *            (with type integer) at the address specified by \p dst.
 * \param root Handle to the root source node.
 * \param src Handle to the source node, with a definition for \c integers.
 * \param private_data Handle to the \c private_data node.
 * \return A non-negative value if successful, otherwise a negative error code.
 *
 * Example (result is 10):
\code
	{
		@func iadd
		integers [ 2 3 5 ]
	}
\endcode
 */ 
int snd_func_iadd(snd_config_t **dst, snd_config_t *root,
	          snd_config_t *src, snd_config_t *private_data)
{
	return snd_func_iops(dst, root, src, private_data, 0);
}
#ifndef DOC_HIDDEN
SND_DLSYM_BUILD_VERSION(snd_func_iadd, SND_CONFIG_DLSYM_VERSION_EVALUATE);
#endif

/**
 * \brief Multiply the given integers.
 * \param dst The function puts the handle to the result configuration node
 *            (with type integer) at the address specified by \p dst.
 * \param root Handle to the root source node.
 * \param src Handle to the source node, with a definition for \c integers.
 * \param private_data Handle to the \c private_data node.
 * \return A non-negative value if successful, otherwise a negative error code.
 *
 * Example (result is 12):
\code
	{
		@func imul
		integers [ 2 3 2 ]
	}
\endcode
 */ 
int snd_func_imul(snd_config_t **dst, snd_config_t *root,
		  snd_config_t *src, snd_config_t *private_data)
{
	return snd_func_iops(dst, root, src, private_data, 1);
}
#ifndef DOC_HIDDEN
SND_DLSYM_BUILD_VERSION(snd_func_imul, SND_CONFIG_DLSYM_VERSION_EVALUATE);
#endif

/**
 * \brief Returns the ALSA data directory.
 * \param dst The function puts the handle to the result configuration node
 *            (with type string) at the address specified by \p dst.
 * \param root Handle to the root source node.
 * \param src Handle to the source node.
 * \param private_data Handle to the \c private_data node. Not used.
 * \return A non-negative value if successful, otherwise a negative error code.
 *
 * Example (result is "/usr/share/alsa" using the default paths):
\code
	{
		@func datadir
	}
\endcode
 */ 
int snd_func_datadir(snd_config_t **dst, snd_config_t *root ATTRIBUTE_UNUSED,
		     snd_config_t *src, snd_config_t *private_data ATTRIBUTE_UNUSED)
{
	int err;
	const char *id;
	
	err = snd_config_get_id(src, &id);
	if (err < 0)
		return err;
	return snd_config_imake_string(dst, id, ALSA_CONFIG_DIR);
}
#ifndef DOC_HIDDEN
SND_DLSYM_BUILD_VERSION(snd_func_datadir, SND_CONFIG_DLSYM_VERSION_EVALUATE);
#endif

static int open_ctl(long card, snd_ctl_t **ctl)
{
	char name[16];
	snprintf(name, sizeof(name), "hw:%li", card);
	name[sizeof(name)-1] = '\0';
	return snd_ctl_open(ctl, name, 0);
}


/**
 * \brief Returns the string from \c private_data.
 * \param dst The function puts the handle to the result configuration node
 *            (with type string) at the address specified by \p dst.
 * \param root Handle to the root source node.
 * \param src Handle to the source node.
 * \param private_data Handle to the \c private_data node (type string,
 *                     id "string").
 * \return A non-negative value if successful, otherwise a negative error code.
 *
 * Example:
\code
	{
		@func private_string
	}
\endcode
 */ 
int snd_func_private_string(snd_config_t **dst, snd_config_t *root ATTRIBUTE_UNUSED,
			    snd_config_t *src, snd_config_t *private_data)
{
	int err;
	const char *str, *id;

	if (private_data == NULL)
		return snd_config_copy(dst, src);
	err = snd_config_test_id(private_data, "string");
	if (err) {
		SNDERR("field string not found");
		return -EINVAL;
	}
	err = snd_config_get_string(private_data, &str);
	if (err < 0) {
		SNDERR("field string is not a string");
		return err;
	}
	err = snd_config_get_id(src, &id);
	if (err >= 0)
		err = snd_config_imake_string(dst, id, str);
	return err;
}
#ifndef DOC_HIDDEN
SND_DLSYM_BUILD_VERSION(snd_func_private_string, SND_CONFIG_DLSYM_VERSION_EVALUATE);
#endif

#ifndef DOC_HIDDEN
int snd_determine_driver(int card, char **driver)
{
	snd_ctl_t *ctl = NULL;
	snd_ctl_card_info_t *info;
	char *res = NULL;
	int err;

	assert(card >= 0 && card <= 32);
	err = open_ctl(card, &ctl);
	if (err < 0) {
		SNDERR("could not open control for card %i", card);
		goto __error;
	}
	snd_ctl_card_info_alloca(&info);
	err = snd_ctl_card_info(ctl, info);
	if (err < 0) {
		SNDERR("snd_ctl_card_info error: %s", snd_strerror(err));
		goto __error;
	}
	res = strdup(snd_ctl_card_info_get_driver(info));
	if (res == NULL)
		err = -ENOMEM;
	else {
		*driver = res;
		err = 0;
	}
      __error:
	if (ctl)
		snd_ctl_close(ctl);
	return err;
}
#endif

/**
 * \brief Returns the driver identification from \c private_data.
 * \param dst The function puts the handle to the result configuration node
 *            (with type string) at the address specified by \p dst.
 * \param root Handle to the root source node.
 * \param src Handle to the source node.
 * \param private_data Handle to the \c private_data node (type integer,
 *                     id "card").
 * \return A non-negative value if successful, otherwise a negative error code.
 *
 * Example:
\code
	{
		@func private_card_driver
	}
\endcode
 */ 
int snd_func_private_card_driver(snd_config_t **dst, snd_config_t *root ATTRIBUTE_UNUSED, snd_config_t *src,
				 snd_config_t *private_data)
{
	char *driver;
	const char *id;
	int err;
	long card;

	err = snd_config_test_id(private_data, "card");
	if (err) {
		SNDERR("field card not found");
		return -EINVAL;
	}
	err = snd_config_get_integer(private_data, &card);
	if (err < 0) {
		SNDERR("field card is not an integer");
		return err;
	}
	if ((err = snd_determine_driver(card, &driver)) < 0)
		return err;
	err = snd_config_get_id(src, &id);
	if (err >= 0)
		err = snd_config_imake_string(dst, id, driver);
	free(driver);
	return err;
}
#ifndef DOC_HIDDEN
SND_DLSYM_BUILD_VERSION(snd_func_private_card_driver, SND_CONFIG_DLSYM_VERSION_EVALUATE);
#endif

static int parse_card(snd_config_t *root, snd_config_t *src,
		      snd_config_t *private_data)
{
	snd_config_t *n;
	char *str;
	int card, err;
	
	err = snd_config_search(src, "card", &n);
	if (err < 0) {
		SNDERR("field card not found");
		return err;
	}
	err = snd_config_evaluate(n, root, private_data, NULL);
	if (err < 0) {
		SNDERR("error evaluating card");
		return err;
	}
	err = snd_config_get_ascii(n, &str);
	if (err < 0) {
		SNDERR("field card is not an integer or a string");
		return err;
	}
	card = snd_card_get_index(str);
	if (card < 0)
		SNDERR("cannot find card '%s'", str);
	free(str);
	return card;
}

/**
 * \brief Returns the card number as integer.
 * \param dst The function puts the handle to the result configuration node
 *            (with type string) at the address specified by \p dst.
 * \param root Handle to the root source node.
 * \param src Handle to the source node, with a \c card definition.
 * \param private_data Handle to the \c private_data node.
 * \return A non-negative value if successful, otherwise a negative error code.
 *
 * Example:
\code
	{
		@func card_inum
		card '0'
	}
\endcode
 */ 
int snd_func_card_inum(snd_config_t **dst, snd_config_t *root, snd_config_t *src,
		       snd_config_t *private_data)
{
	const char *id;
	int card, err;
	
	card = parse_card(root, src, private_data);
	if (card < 0)
		return card;
	err = snd_config_get_id(src, &id);
	if (err >= 0)
		err = snd_config_imake_integer(dst, id, card);
	return err;
}
#ifndef DOC_HIDDEN
SND_DLSYM_BUILD_VERSION(snd_func_card_inum, SND_CONFIG_DLSYM_VERSION_EVALUATE);
#endif

/**
 * \brief Returns the driver identification for a card.
 * \param dst The function puts the handle to the result configuration node
 *            (with type string) at the address specified by \p dst.
 * \param root Handle to the root source node.
 * \param src Handle to the source node, with a \c card definition.
 * \param private_data Handle to the \c private_data node.
 * \return A non-negative value if successful, otherwise a negative error code.
 *
 * Example:
\code
	{
		@func card_driver
		card 0
	}
\endcode
 */ 
int snd_func_card_driver(snd_config_t **dst, snd_config_t *root, snd_config_t *src,
			 snd_config_t *private_data)
{
	snd_config_t *val;
	int card, err;
	
	card = parse_card(root, src, private_data);
	if (card < 0)
		return card;
	err = snd_config_imake_integer(&val, "card", card);
	if (err < 0)
		return err;
	err = snd_func_private_card_driver(dst, root, src, val);
	snd_config_delete(val);
	return err;
}
#ifndef DOC_HIDDEN
SND_DLSYM_BUILD_VERSION(snd_func_card_driver, SND_CONFIG_DLSYM_VERSION_EVALUATE);
#endif

/**
 * \brief Returns the identification of a card.
 * \param dst The function puts the handle to the result configuration node
 *            (with type string) at the address specified by \p dst.
 * \param root Handle to the root source node.
 * \param src Handle to the source node, with a \c card definition.
 * \param private_data Handle to the \c private_data node.
 * \return A non-negative value if successful, otherwise a negative error code.
 *
 * Example:
\code
	{
		@func card_id
		card 0
	}
\endcode
 */ 
int snd_func_card_id(snd_config_t **dst, snd_config_t *root, snd_config_t *src,
		     snd_config_t *private_data)
{
	snd_ctl_t *ctl = NULL;
	snd_ctl_card_info_t *info;
	const char *id;
	int card, err;
	
	card = parse_card(root, src, private_data);
	if (card < 0)
		return card;
	err = open_ctl(card, &ctl);
	if (err < 0) {
		SNDERR("could not open control for card %i", card);
		goto __error;
	}
	snd_ctl_card_info_alloca(&info);
	err = snd_ctl_card_info(ctl, info);
	if (err < 0) {
		SNDERR("snd_ctl_card_info error: %s", snd_strerror(err));
		goto __error;
	}
	err = snd_config_get_id(src, &id);
	if (err >= 0)
		err = snd_config_imake_string(dst, id,
					      snd_ctl_card_info_get_id(info));
      __error:
      	if (ctl)
      		snd_ctl_close(ctl);
	return err;
}
#ifndef DOC_HIDDEN
SND_DLSYM_BUILD_VERSION(snd_func_card_id, SND_CONFIG_DLSYM_VERSION_EVALUATE);
#endif

/**
 * \brief Returns the name of a card.
 * \param dst The function puts the handle to the result configuration node
 *            (with type string) at the address specified by \p dst.
 * \param root Handle to the root source node.
 * \param src Handle to the source node, with a \c card definition.
 * \param private_data Handle to the \c private_data node.
 * \return A non-negative value if successful, otherwise a negative error code.
 *
 * Example:
\code
	{
		@func card_name
		card 0
	}
\endcode
 */ 
int snd_func_card_name(snd_config_t **dst, snd_config_t *root,
		       snd_config_t *src, snd_config_t *private_data)
{
	snd_ctl_t *ctl = NULL;
	snd_ctl_card_info_t *info;
	const char *id;
	int card, err;
	
	card = parse_card(root, src, private_data);
	if (card < 0)
		return card;
	err = open_ctl(card, &ctl);
	if (err < 0) {
		SNDERR("could not open control for card %i", card);
		goto __error;
	}
	snd_ctl_card_info_alloca(&info);
	err = snd_ctl_card_info(ctl, info);
	if (err < 0) {
		SNDERR("snd_ctl_card_info error: %s", snd_strerror(err));
		goto __error;
	}
	err = snd_config_get_id(src, &id);
	if (err >= 0)
		err = snd_config_imake_string(dst, id,
					      snd_ctl_card_info_get_name(info));
      __error:
      	if (ctl)
      		snd_ctl_close(ctl);
	return err;
}
#ifndef DOC_HIDDEN
SND_DLSYM_BUILD_VERSION(snd_func_card_name, SND_CONFIG_DLSYM_VERSION_EVALUATE);
#endif

#ifdef BUILD_PCM

/**
 * \brief Returns the pcm identification of a device.
 * \param dst The function puts the handle to the result configuration node
 *            (with type string) at the address specified by \p dst.
 * \param root Handle to the root source node.
 * \param src Handle to the source node, with definitions for \c card,
 *            \c device and (optionally) \c subdevice.
 * \param private_data Handle to the \c private_data node.
 * \return A non-negative value if successful, otherwise a negative error code.
 *
 * Example:
\code
	{
		@func pcm_id
		card 0
		device 0
		subdevice 0	# optional
	}
\endcode
 */ 
int snd_func_pcm_id(snd_config_t **dst, snd_config_t *root, snd_config_t *src, void *private_data)
{
	snd_config_t *n;
	snd_ctl_t *ctl = NULL;
	snd_pcm_info_t *info;
	const char *id;
	long card, device, subdevice = 0;
	int err;
	
	card = parse_card(root, src, private_data);
	if (card < 0)
		return card;
	err = snd_config_search(src, "device", &n);
	if (err < 0) {
		SNDERR("field device not found");
		goto __error;
	}
	err = snd_config_evaluate(n, root, private_data, NULL);
	if (err < 0) {
		SNDERR("error evaluating device");
		goto __error;
	}
	err = snd_config_get_integer(n, &device);
	if (err < 0) {
		SNDERR("field device is not an integer");
		goto __error;
	}
	if (snd_config_search(src, "subdevice", &n) >= 0) {
		err = snd_config_evaluate(n, root, private_data, NULL);
		if (err < 0) {
			SNDERR("error evaluating subdevice");
			goto __error;
		}
		err = snd_config_get_integer(n, &subdevice);
		if (err < 0) {
			SNDERR("field subdevice is not an integer");
			goto __error;
		}
	}
	err = open_ctl(card, &ctl);
	if (err < 0) {
		SNDERR("could not open control for card %li", card);
		goto __error;
	}
	snd_pcm_info_alloca(&info);
	snd_pcm_info_set_device(info, device);
	snd_pcm_info_set_subdevice(info, subdevice);
	err = snd_ctl_pcm_info(ctl, info);
	if (err < 0) {
		SNDERR("snd_ctl_pcm_info error: %s", snd_strerror(err));
		goto __error;
	}
	err = snd_config_get_id(src, &id);
	if (err >= 0)
		err = snd_config_imake_string(dst, id, snd_pcm_info_get_id(info));
      __error:
      	if (ctl)
      		snd_ctl_close(ctl);
	return err;
}
#ifndef DOC_HIDDEN
SND_DLSYM_BUILD_VERSION(snd_func_pcm_id, SND_CONFIG_DLSYM_VERSION_EVALUATE);
#endif

/**
 * \brief Returns the pcm card and device arguments (in form CARD=N,DEV=M)
 *                for pcm specified by class and index.
 * \param dst The function puts the handle to the result configuration node
 *            (with type string) at the address specified by \p dst.
 * \param root Handle to the root source node.
 * \param src Handle to the source node, with definitions for \c class
 *            and \c index.
 * \param private_data Handle to the \c private_data node.
 * \return A non-negative value if successful, otherwise a negative error code.
 *
 * Example:
\code
	{
		@func pcm_args_by_class
		class 0
		index 0
	}
\endcode
 */ 
int snd_func_pcm_args_by_class(snd_config_t **dst, snd_config_t *root, snd_config_t *src, void *private_data)
{
	snd_config_t *n;
	snd_ctl_t *ctl = NULL;
	snd_pcm_info_t *info;
	const char *id;
	int card = -1, dev;
	long class, index;
	int idx = 0;
	int err;

	err = snd_config_search(src, "class", &n);
	if (err < 0) {
		SNDERR("field class not found");
		goto __out;
	}
	err = snd_config_evaluate(n, root, private_data, NULL);
	if (err < 0) {
		SNDERR("error evaluating class");
		goto __out;
	}
	err = snd_config_get_integer(n, &class);
	if (err < 0) {
		SNDERR("field class is not an integer");
		goto __out;
	}
	err = snd_config_search(src, "index", &n);
	if (err < 0) {
		SNDERR("field index not found");
		goto __out;
	}
	err = snd_config_evaluate(n, root, private_data, NULL);
	if (err < 0) {
		SNDERR("error evaluating index");
		goto __out;
	}
	err = snd_config_get_integer(n, &index);
	if (err < 0) {
		SNDERR("field index is not an integer");
		goto __out;
	}

	snd_pcm_info_alloca(&info);
	while(1) {
		err = snd_card_next(&card);
		if (err < 0) {
			SNDERR("could not get next card");
			goto __out;
		}
		if (card < 0)
			break;
		err = open_ctl(card, &ctl);
		if (err < 0) {
			SNDERR("could not open control for card %i", card);
			goto __out;
		}
		dev = -1;
		memset(info, 0, snd_pcm_info_sizeof());
		while(1) {
			err = snd_ctl_pcm_next_device(ctl, &dev);
			if (err < 0) {
				SNDERR("could not get next pcm for card %i", card);
				goto __out;
			}
			if (dev < 0)
				break;
			snd_pcm_info_set_device(info, dev);
			err = snd_ctl_pcm_info(ctl, info);
			if (err < 0)
				continue;
			if (snd_pcm_info_get_class(info) == (snd_pcm_class_t)class &&
					index == idx++)
				goto __out;
		}
      		snd_ctl_close(ctl);
		ctl = NULL;
	}
	err = -ENODEV;

      __out:
      	if (ctl)
      		snd_ctl_close(ctl);
	if (err < 0)
		return err;
	if((err = snd_config_get_id(src, &id)) >= 0) {
		char name[32];
		snprintf(name, sizeof(name), "CARD=%i,DEV=%i", card, dev);
		err = snd_config_imake_string(dst, id, name);
	}
	return err;
}
#ifndef DOC_HIDDEN
SND_DLSYM_BUILD_VERSION(snd_func_pcm_args_by_class, SND_CONFIG_DLSYM_VERSION_EVALUATE);
#endif

/**
 * \brief Returns the PCM subdevice from \c private_data.
 * \param dst The function puts the handle to the result configuration node
 *            (with type integer) at the address specified by \p dst.
 * \param root Handle to the root source node.
 * \param src Handle to the source node.
 * \param private_data Handle to the \c private_data node (type pointer,
 *                     id "pcm_handle").
 * \return A non-negative value if successful, otherwise a negative error code.
 *
 * Example:
\code
	{
		@func private_pcm_subdevice
	}
\endcode
 */ 
int snd_func_private_pcm_subdevice(snd_config_t **dst, snd_config_t *root ATTRIBUTE_UNUSED,
				   snd_config_t *src, snd_config_t *private_data)
{
	snd_pcm_info_t *info;
	const char *id;
	const void *data;
	snd_pcm_t *pcm;
	int err;

	if (private_data == NULL)
		return snd_config_copy(dst, src);
	err = snd_config_test_id(private_data, "pcm_handle");
	if (err) {
		SNDERR("field pcm_handle not found");
		return -EINVAL;
	}
	err = snd_config_get_pointer(private_data, &data);
	pcm = (snd_pcm_t *)data;
	if (err < 0) {
		SNDERR("field pcm_handle is not a pointer");
		return err;
	}
	snd_pcm_info_alloca(&info);
	err = snd_pcm_info(pcm, info);
	if (err < 0) {
		SNDERR("snd_ctl_pcm_info error: %s", snd_strerror(err));
		return err;
	}
	err = snd_config_get_id(src, &id);
	if (err >= 0)
		err = snd_config_imake_integer(dst, id, snd_pcm_info_get_subdevice(info));
	return err;
}
#ifndef DOC_HIDDEN
SND_DLSYM_BUILD_VERSION(snd_func_private_pcm_subdevice, SND_CONFIG_DLSYM_VERSION_EVALUATE);
#endif

#endif /* BUILD_PCM */

/**
 * \brief Copies the specified configuration node.
 * \param dst The function puts the handle to the result configuration node
 *            (with the same type as the specified node) at the address
 *            specified by \p dst.
 * \param root Handle to the root source node.
 * \param src Handle to the source node, with definitions for \c name and
 *            (optionally) \c file.
 * \param private_data Handle to the \c private_data node.
 * \return A non-negative value if successful, otherwise a negative error code.
 * \note The root source node can be modified!
 *
 * Example:
\code
	{
		@func refer
		file "/etc/myconf.conf"		# optional
		name "id1.id2.id3"
	}
\endcode
 */ 
int snd_func_refer(snd_config_t **dst, snd_config_t *root, snd_config_t *src,
		   snd_config_t *private_data)
{
	snd_config_t *n;
	const char *file = NULL, *name = NULL;
	int err;
	
	err = snd_config_search(src, "file", &n);
	if (err >= 0) {
		err = snd_config_evaluate(n, root, private_data, NULL);
		if (err < 0) {
			SNDERR("error evaluating file");
			goto _end;
		}
		err = snd_config_get_string(n, &file);
		if (err < 0) {
			SNDERR("file is not a string");
			goto _end;
		}
	}
	err = snd_config_search(src, "name", &n);
	if (err >= 0) {
		err = snd_config_evaluate(n, root, private_data, NULL);
		if (err < 0) {
			SNDERR("error evaluating name");
			goto _end;
		}
		err = snd_config_get_string(n, &name);
		if (err < 0) {
			SNDERR("name is not a string");
			goto _end;
		}
	}
	if (!name) {
		err = -EINVAL;
		SNDERR("name is not specified");
		goto _end;
	}
	if (file) {
		snd_input_t *input;
		err = snd_input_stdio_open(&input, file, "r");
		if (err < 0) {
			SNDERR("Unable to open file %s: %s", file, snd_strerror(err));
			goto _end;
		}
		err = snd_config_load(root, input);
		snd_input_close(input);
		if (err < 0)
			goto _end;
	}
	err = snd_config_search_definition(root, NULL, name, dst);
	if (err >= 0) {
		const char *id;
		err = snd_config_get_id(src, &id);
		if (err >= 0)
			err = snd_config_set_id(*dst, id);
	} else {
		err = snd_config_search(src, "default", &n);
		if (err < 0)
			SNDERR("Unable to find definition '%s'", name);
		else {
			const char *id;
			err = snd_config_evaluate(n, root, private_data, NULL);
			if (err < 0)
				return err;
			if ((err = snd_config_copy(dst, n)) >= 0) {
				if ((err = snd_config_get_id(src, &id)) < 0 ||
				    (err = snd_config_set_id(*dst, id)) < 0)
					snd_config_delete(*dst);
			}
		}
	}
 _end:
	return err;
}
#ifndef DOC_HIDDEN
SND_DLSYM_BUILD_VERSION(snd_func_refer, SND_CONFIG_DLSYM_VERSION_EVALUATE);
#endif
