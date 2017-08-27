/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

/*
 * $Id$
 *
 * @brief map / template functions
 * @file main/map.c
 *
 * @ingroup AVP
 *
 * @copyright 2013 The FreeRADIUS server project
 * @copyright 2013  Alan DeKok <aland@freeradius.org>
 */

RCSID("$Id$")

#include <freeradius-devel/radiusd.h>
#include <freeradius-devel/rad_assert.h>

#include <ctype.h>

/** Release memory allocated to value pair template.
 *
 * @param[in,out] tmpl to free.
 */
void radius_tmplfree(value_pair_tmpl_t **tmpl)
{
	if (*tmpl == NULL) return;

	dict_attr_free(&((*tmpl)->da));

	talloc_free(*tmpl);

	*tmpl = NULL;
}

/** Parse qualifiers to convert attrname into a value_pair_tmpl_t.
 *
 * VPTs are used in various places where we need to pre-parse configuration
 * sections into attribute mappings.
 *
 * Note: name field is just a copy of the input pointer, if you know that
 * string might be freed before you're done with the vpt use radius_attr2tmpl
 * instead.
 *
 * @param[in] name attribute name including qualifiers.
 * @param[out] vpt to modify.
 * @param[in] request_def The default request to insert unqualified
 *	attributes into.
 * @param[in] list_def The default list to insert unqualified attributes into.
 * @return -1 on error or 0 on success.
 */
int radius_parse_attr(char const *name, value_pair_tmpl_t *vpt,
		      request_refs_t request_def,
		      pair_lists_t list_def)
{
	DICT_ATTR const *da;
	char const *p;
	size_t len;

	memset(vpt, 0, sizeof(*vpt));
	vpt->name = name;
	p = name;

	vpt->request = radius_request_name(&p, request_def);
	len = p - name;
	if (vpt->request == REQUEST_UNKNOWN) {
		ERROR("Invalid request qualifier \"%.*s\"", (int) len, name);
		return -1;
	}
	name += len;

	vpt->list = radius_list_name(&p, list_def);
	if (vpt->list == PAIR_LIST_UNKNOWN) {
		len = p - name;
		ERROR("Invalid list qualifier \"%.*s\"", (int) len, name);
		return -1;
	}

	if (*p == '\0') {
		vpt->type = VPT_TYPE_LIST;
		return 0;
	}

	da = dict_attrbyname(p);
	if (!da) {
		da = dict_attrunknownbyname(p, false);
		if (!da) {
			ERROR("Unknown attribute \"%s\"", p);
			return -1;
		}
	}
	vpt->da = da;

	vpt->type = VPT_TYPE_ATTR;
	return 0;
}

/** Parse qualifiers to convert attrname into a value_pair_tmpl_t.
 *
 * VPTs are used in various places where we need to pre-parse configuration
 * sections into attribute mappings.
 *
 * @param[in] ctx for talloc
 * @param[in] name attribute name including qualifiers.
 * @param[in] request_def The default request to insert unqualified
 *	attributes into.
 * @param[in] list_def The default list to insert unqualified attributes into.
 * @return pointer to a value_pair_tmpl_t struct (must be freed with
 *	radius_tmplfree) or NULL on error.
 */
value_pair_tmpl_t *radius_attr2tmpl(TALLOC_CTX *ctx, char const *name,
				    request_refs_t request_def,
				    pair_lists_t list_def)
{
	value_pair_tmpl_t *vpt;
	char const *copy;

	vpt = talloc(ctx, value_pair_tmpl_t); /* parse_attr zeroes it */
	copy = talloc_strdup(vpt, name);

	if (radius_parse_attr(copy, vpt, request_def, list_def) < 0) {
		radius_tmplfree(&vpt);
		return NULL;
	}

	return vpt;
}

/** Convert module specific attribute id to value_pair_tmpl_t.
 *
 * @param[in] ctx for talloc
 * @param[in] name string to convert.
 * @param[in] type Type of quoting around value.
 * @return pointer to new VPT.
 */
value_pair_tmpl_t *radius_str2tmpl(TALLOC_CTX *ctx, char const *name, FR_TOKEN type)
{
	value_pair_tmpl_t *vpt;

	vpt = talloc_zero(ctx, value_pair_tmpl_t);
	vpt->name = talloc_strdup(vpt, name);

	switch (type) {
	case T_BARE_WORD:
		if (*name == '&') name++;

		if (!isdigit((int) *name)) {
			request_refs_t ref;
			pair_lists_t list;
			char const *p = name;

			ref = radius_request_name(&p, REQUEST_CURRENT);
			if (ref == REQUEST_UNKNOWN) goto literal;

			list = radius_list_name(&p, PAIR_LIST_REQUEST);
			if (list == PAIR_LIST_UNKNOWN) goto literal;

			if ((p != name) && !*p) {
				vpt->type = VPT_TYPE_LIST;

			} else {
				DICT_ATTR const *da;
				da = dict_attrbyname(p);
				if (!da) {
					vpt->type = VPT_TYPE_LITERAL;
					break;
				}
				vpt->da = da;
				vpt->type = VPT_TYPE_ATTR;
			}

			vpt->request = ref;
			vpt->list = list;
			break;
		}
		/* FALL-THROUGH */

	case T_SINGLE_QUOTED_STRING:
	literal:
		vpt->type = VPT_TYPE_LITERAL;
		break;
	case T_DOUBLE_QUOTED_STRING:
		vpt->type = VPT_TYPE_XLAT;
		break;
	case T_BACK_QUOTED_STRING:
		vpt->type = VPT_TYPE_EXEC;
		break;
	case T_OP_REG_EQ: /* hack */
		vpt->type = VPT_TYPE_REGEX;
		break;
	default:
		rad_assert(0);
		return NULL;
	}

	return vpt;
}


/** Convert strings to value_pair_map_e
 *
 * Treatment of operands depends on quotation, barewords are treated
 * as attribute references, double quoted values are treated as
 * expandable strings, single quoted values are treated as literal
 * strings.
 *
 * Return must be freed with talloc_free
 *
 * @param[in] ctx for talloc
 * @param[in] lhs of the operation
 * @param[in] lhs_type type of the LHS string
 * @param[in] op the operation to perform
 * @param[in] rhs of the operation
 * @param[in] rhs_type type of the RHS string
 * @param[in] dst_request_def The default request to insert unqualified
 *	attributes into.
 * @param[in] dst_list_def The default list to insert unqualified attributes
 *	into.
 * @param[in] src_request_def The default request to resolve attribute
 *	references in.
 * @param[in] src_list_def The default list to resolve unqualified attributes
 *	in.
 * @return value_pair_map_t if successful or NULL on error.
 */
value_pair_map_t *radius_str2map(TALLOC_CTX *ctx, char const *lhs, FR_TOKEN lhs_type,
				 FR_TOKEN op, char const *rhs, FR_TOKEN rhs_type,
				 request_refs_t dst_request_def,
				 pair_lists_t dst_list_def,
				 request_refs_t src_request_def,
				 pair_lists_t src_list_def)
{
	value_pair_map_t *map;

	map = talloc_zero(ctx, value_pair_map_t);

	if ((lhs_type == T_BARE_WORD) && (*lhs == '&')) {
		map->dst = radius_attr2tmpl(map, lhs + 1, dst_request_def, dst_list_def);
	} else {
		map->dst = radius_str2tmpl(map, lhs, lhs_type);
	}

	if (!map->dst) {
	error:
		talloc_free(map);
		return NULL;
	}

	map->op = op;

	/*
	 *	Ignore the RHS if it's a true / false comparison.
	 */
	if ((map->op == T_OP_CMP_TRUE) || (map->op == T_OP_CMP_FALSE)) {
		return map;
	}

	if ((rhs_type == T_BARE_WORD) && (*rhs == '&')) {
		map->src = radius_attr2tmpl(map, rhs + 1, src_request_def, src_list_def);
	} else {
		map->src = radius_str2tmpl(map, rhs, rhs_type);
	}

	if (!map->dst) goto error;

	return map;
}


/** Convert CONFIG_PAIR (which may contain refs) to value_pair_map_t.
 *
 * Treats the left operand as an attribute reference
 * @verbatim<request>.<list>.<attribute>@endverbatim
 *
 * Treatment of left operand depends on quotation, barewords are treated as
 * attribute references, double quoted values are treated as expandable strings,
 * single quoted values are treated as literal strings.
 *
 * Return must be freed with talloc_free
 *
 * @param[in] ctx for talloc
 * @param[in] cp to convert to map.
 * @param[in] dst_request_def The default request to insert unqualified
 *	attributes into.
 * @param[in] dst_list_def The default list to insert unqualified attributes
 *	into.
 * @param[in] src_request_def The default request to resolve attribute
 *	references in.
 * @param[in] src_list_def The default list to resolve unqualified attributes
 *	in.
 * @return value_pair_map_t if successful or NULL on error.
 */
value_pair_map_t *radius_cp2map(TALLOC_CTX *ctx, CONF_PAIR *cp,
				request_refs_t dst_request_def,
				pair_lists_t dst_list_def,
				request_refs_t src_request_def,
				pair_lists_t src_list_def)
{
	value_pair_map_t *map;
	char const *attr;
	char const *value;
	FR_TOKEN type;
	CONF_ITEM *ci = cf_pairtoitem(cp);

	if (!cp) return NULL;

	map = talloc_zero(ctx, value_pair_map_t);

	attr = cf_pair_attr(cp);
	value = cf_pair_value(cp);
	if (!value) {
		cf_log_err(ci, "Missing attribute value");
		goto error;
	}

	map->dst = radius_attr2tmpl(map, attr, dst_request_def, dst_list_def);
	if (!map->dst) {
		cf_log_err(ci, "Syntax error in attribute definition");
		goto error;
	}

	/*
	 *	Bare words always mean attribute references.
	 */
	type = cf_pair_value_type(cp);
	if (type == T_BARE_WORD) {
		if (*value == '&') {
			map->src = radius_attr2tmpl(map, value + 1, src_request_def, src_list_def);

		} else {
			if (!isdigit((int) *value) &&
			    ((strchr(value, ':') != NULL) ||
			     (dict_attrbyname(value) != NULL))) {
				map->src = radius_attr2tmpl(map, value, src_request_def, src_list_def);
			}
			if (map->src) {
				WDEBUG("%s[%d]: Please add '&' for attribute reference '%s = &%s'",
				       cf_pair_filename(cp), cf_pair_lineno(cp),
				       attr, value);
			} else {
				map->src = radius_str2tmpl(map, value, type);
			}
		}
	} else {
		map->src = radius_str2tmpl(map, value, type);
	}

	if (!map->src) {
		goto error;
	}

	map->op = cf_pair_operator(cp);
	map->ci = ci;

	/*
	 *	Lots of sanity checks for insane people...
	 */

	/*
	 *	We don't support implicit type conversion,
	 *	except for "octets"
	 */
	if (map->dst->da && map->src->da &&
	    (map->src->da->type != map->dst->da->type) &&
	    (map->src->da->type != PW_TYPE_OCTETS) &&
	    (map->dst->da->type != PW_TYPE_OCTETS)) {
		cf_log_err(ci, "Attribute type mismatch");
		goto error;
	}

	/*
	 *	What exactly where you expecting to happen here?
	 */
	if ((map->dst->type == VPT_TYPE_ATTR) &&
	    (map->src->type == VPT_TYPE_LIST)) {
		cf_log_err(ci, "Can't copy list into an attribute");
		goto error;
	}

	/*
	 *	Can't copy an xlat expansion or literal into a list,
	 *	we don't know what type of attribute we'd need
	 *	to create
	 */
	if ((map->dst->type == VPT_TYPE_LIST) &&
	    ((map->src->type == VPT_TYPE_XLAT) || (map->src->type == VPT_TYPE_LITERAL))) {
		cf_log_err(ci, "Can't copy value into list (we don't know which attribute to create)");
		goto error;
	}

	/*
	 *	Depending on the attribute type, some operators are
	 *	disallowed.
	 */
	if (map->dst->type == VPT_TYPE_ATTR) {
		if ((map->op != T_OP_EQ) &&
		    (map->op != T_OP_CMP_EQ) &&
		    (map->op != T_OP_ADD) &&
		    (map->op != T_OP_SUB) &&
		    (map->op != T_OP_LE) &&
		    (map->op != T_OP_GE) &&
		    (map->op != T_OP_CMP_FALSE) &&
		    (map->op != T_OP_SET)) {
			cf_log_err(ci, "Invalid operator for attribute");
			goto error;
		}
	}

	switch (map->src->type) {
		/*
		 *	Only += and -= operators are supported for list copy.
		 */
		case VPT_TYPE_LIST:
			switch (map->op) {
			case T_OP_SUB:
			case T_OP_ADD:
				break;

			default:
				cf_log_err(ci, "Operator \"%s\" not allowed "
					   "for list copy",
					   fr_int2str(fr_tokens, map->op, "<INVALID>"));
				goto error;
			}
		break;

		default:
			break;
	}

	return map;

error:
	talloc_free(map);
	return NULL;
}

/** Convert an 'update' config section into an attribute map.
 *
 * Uses 'name2' of section to set default request and lists.
 *
 * @param[in] cs the update section
 * @param[out] head Where to store the head of the map.
 * @param[in] dst_list_def The default destination list, usually dictated by
 * 	the section the module is being called in.
 * @param[in] src_list_def The default source list, usually dictated by the
 *	section the module is being called in.
 * @param[in] max number of mappings to process.
 * @return -1 on error, else 0.
 */
int radius_attrmap(CONF_SECTION *cs, value_pair_map_t **head,
		   pair_lists_t dst_list_def, pair_lists_t src_list_def,
		   unsigned int max)
{
	char const *cs_list, *p;

	request_refs_t request_def = REQUEST_CURRENT;

	CONF_ITEM *ci;
	CONF_PAIR *cp;

	unsigned int total = 0;
	value_pair_map_t **tail, *map;
	TALLOC_CTX *ctx;

	*head = NULL;
	tail = head;

	if (!cs) return 0;

	/*
	 *	The first map has cs as the parent.
	 *	The rest have the previous map as the parent.
	 */
	ctx = cs;

	ci = cf_sectiontoitem(cs);

	cs_list = p = cf_section_name2(cs);
	if (cs_list) {
		request_def = radius_request_name(&p, REQUEST_CURRENT);
		if (request_def == REQUEST_UNKNOWN) {
			cf_log_err(ci, "Default request specified "
				   "in mapping section is invalid");
			return -1;
		}

		dst_list_def = fr_str2int(pair_lists, p, PAIR_LIST_UNKNOWN);
		if (dst_list_def == PAIR_LIST_UNKNOWN) {
			cf_log_err(ci, "Default list \"%s\" specified "
				   "in mapping section is invalid", p);
			return -1;
		}
	}

	for (ci = cf_item_find_next(cs, NULL);
	     ci != NULL;
	     ci = cf_item_find_next(cs, ci)) {
		if (total++ == max) {
			cf_log_err(ci, "Map size exceeded");
			goto error;
		}

		if (!cf_item_is_pair(ci)) {
			cf_log_err(ci, "Entry is not in \"attribute = "
				       "value\" format");
			goto error;
		}

		cp = cf_itemtopair(ci);
		map = radius_cp2map(ctx, cp, request_def, dst_list_def,
				    REQUEST_CURRENT, src_list_def);
		if (!map) {
			goto error;
		}

		ctx = *tail = map;
		tail = &(map->next);
	}

	return 0;
error:
	talloc_free(*head);
	return -1;
}


/**  Print a template to a string
 *
 * @param[out] buffer for the output string
 * @param[in] bufsize of the buffer
 * @param[in] vpt to print
 * @return the size of the string printed
 */
size_t radius_tmpl2str(char *buffer, size_t bufsize, value_pair_tmpl_t const *vpt)
{
	char c;
	char const *p;
	char *q = buffer;
	char *end;

	switch (vpt->type) {
	default:
		return 0;

	case VPT_TYPE_REGEX:
		c = '/';
		break;

	case VPT_TYPE_XLAT:
		c = '"';
		break;

	case VPT_TYPE_LITERAL:	/* single-quoted or bare word */
		/*
		 *	Hack
		 */
		for (p = vpt->name; *p != '\0'; p++) {
			if (*p == ' ') break;
			if (*p == '\'') break;
			if (!dict_attr_allowed_chars[(int) *p]) break;
		}

		if (!*p) {
			strlcpy(buffer, vpt->name, bufsize);
			return strlen(buffer);
		}

		c = '\'';
		break;

	case VPT_TYPE_EXEC:
		c = '`';
		break;

	case VPT_TYPE_ATTR:
		buffer[0] = '&';
		if (vpt->request == REQUEST_CURRENT) {
			if (vpt->list == PAIR_LIST_REQUEST) {
				strlcpy(buffer + 1, vpt->da->name, bufsize - 1);
			} else {
				snprintf(buffer + 1, bufsize - 1, "%s:%s",
					 fr_int2str(pair_lists, vpt->list, ""),
					 vpt->da->name);
			}

		} else {
			snprintf(buffer + 1, bufsize - 1, "%s.%s:%s",
				 fr_int2str(request_refs, vpt->request, ""),
				 fr_int2str(pair_lists, vpt->list, ""),
				 vpt->da->name);
		}
		return strlen(buffer);

	case VPT_TYPE_DATA:
		{
			VALUE_PAIR *vp;
			TALLOC_CTX *ctx;

			memcpy(&ctx, &vpt, sizeof(ctx)); /* hack */

			vp = pairalloc(ctx, vpt->da);
			memcpy(&vp->data, vpt->vpd, sizeof(vp->data));
			vp->length = vpt->length;

			q = vp_aprint(vp, vp);

			if ((vpt->da->type != PW_TYPE_STRING) &&
			    (vpt->da->type != PW_TYPE_DATE)) {
				strlcpy(buffer, q, bufsize);
			} else {
				/*
				 *	FIXME: properly escape the string...
				 */
				snprintf(buffer, bufsize, "\"%s\"", q);
			}

			talloc_free(q);
			pairfree(&vp);
			return strlen(buffer);
		}
	}

	if (bufsize <= 3) {
	no_room:
		*buffer = '\0';
		return 0;
	}

	p = vpt->name;
	*(q++) = c;
	end = buffer + bufsize - 3; /* quotes + EOS */

	while (*p && (q < end)) {
		if (*p == c) {
			if ((q - end) < 4) goto no_room; /* escape, char, quote, EOS */
			*(q++) = '\\';
			*(q++) = *(p++);
			continue;
		}

		switch (*p) {
		case '\\':
			if ((q - end) < 4) goto no_room;
			*(q++) = '\\';
			*(q++) = *(p++);
			break;

		case '\r':
			if ((q - end) < 4) goto no_room;
			*(q++) = '\\';
			*(q++) = 'r';
			p++;
			break;

		case '\n':
			if ((q - end) < 4) goto no_room;
			*(q++) = '\\';
			*(q++) = 'r';
			p++;
			break;

		case '\t':
			if ((q - end) < 4) goto no_room;
			*(q++) = '\\';
			*(q++) = 't';
			p++;
			break;

		default:
			*(q++) = *(p++);
			break;
		}
	}

	*(q++) = c;
	*q = '\0';

	return q - buffer;
}


/**  Print a map to a string
 *
 * @param[out] buffer for the output string
 * @param[in] bufsize of the buffer
 * @param[in] map to print
 * @return the size of the string printed
 */
size_t radius_map2str(char *buffer, size_t bufsize, value_pair_map_t const *map)
{
	size_t len;
	char *p = buffer;
	char *end = buffer + bufsize;

	len = radius_tmpl2str(buffer, bufsize, map->dst);
	p += len;

	*(p++) = ' ';
	strlcpy(p, fr_token_name(map->op), end - p);
	p += strlen(p);
	*(p++) = ' ';

	/*
	 *	The RHS doesn't matter for many operators
	 */
	if ((map->op == T_OP_CMP_TRUE) ||
	    (map->op == T_OP_CMP_FALSE)) {
		strlcpy(p, "ANY", (end - p));
		p += strlen(p);
		return p - buffer;
	}

	rad_assert(map->src != NULL);

	if ((map->dst->type == VPT_TYPE_ATTR) &&
	    (map->dst->da->type == PW_TYPE_STRING) &&
	    (map->src->type == VPT_TYPE_LITERAL)) {
		*(p++) = '\'';
		len = radius_tmpl2str(p, end - p, map->src);
		p += len;
		*(p++) = '\'';
		*p = '\0';
	} else {
		len = radius_tmpl2str(p, end - p, map->src);
		p += len;
	}

	return p - buffer;
}
