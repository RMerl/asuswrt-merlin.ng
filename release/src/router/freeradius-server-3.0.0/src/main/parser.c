/*
 * parser.c	Parse various things
 *
 * Version:	$Id$
 *
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
 *
 * Copyright 2013  Alan DeKok <aland@freeradius.org>
 */

RCSID("$Id$")

#include <freeradius-devel/radiusd.h>
#include <freeradius-devel/parser.h>
#include <freeradius-devel/rad_assert.h>

#include <ctype.h>

#define PW_CAST_BASE (1850)

/*
 *	This file shouldn't use any functions from the server core.
 */

size_t fr_cond_sprint(char *buffer, size_t bufsize, fr_cond_t const *c)
{
	size_t len;
	char *p = buffer;
	char *end = buffer + bufsize - 1;

next:
	if (c->negate) {
		*(p++) = '!';	/* FIXME: only allow for child? */
	}

	switch (c->type) {
	case COND_TYPE_EXISTS:
		rad_assert(c->data.vpt != NULL);
		if (c->cast) {
			len = snprintf(p, end - p, "<%s>", fr_int2str(dict_attr_types,
								      c->cast->type, "??"));
			p += len;
		}

		len = radius_tmpl2str(p, end - p, c->data.vpt);
		p += len;
		break;

	case COND_TYPE_MAP:
		rad_assert(c->data.map != NULL);
#if 0
		*(p++) = '[';	/* for extra-clear debugging */
#endif
		if (c->cast) {
			len = snprintf(p, end - p, "<%s>", fr_int2str(dict_attr_types,
								      c->cast->type, "??"));
			p += len;
		}

		len = radius_map2str(p, end - p, c->data.map);
		p += len;
#if 0
		*(p++) = ']';
#endif
		break;

	case COND_TYPE_CHILD:
		rad_assert(c->data.child != NULL);
		*(p++) = '(';
		len = fr_cond_sprint(p, end - p, c->data.child);
		p += len;
		*(p++) = ')';
		break;

	case COND_TYPE_TRUE:
		strlcpy(buffer, "true", bufsize);
		return strlen(buffer);

	case COND_TYPE_FALSE:
		strlcpy(buffer, "false", bufsize);
		return strlen(buffer);

	default:
		*buffer = '\0';
		return 0;
	}

	if (c->next_op == COND_NONE) {
		rad_assert(c->next == NULL);
		*p = '\0';
		return p - buffer;
	}

	if (c->next_op == COND_AND) {
		strlcpy(p, " && ", end - p);
		p += strlen(p);

	} else if (c->next_op == COND_OR) {
		strlcpy(p, " || ", end - p);
		p += strlen(p);

	} else {
		rad_assert(0 == 1);
	}

	c = c->next;
	goto next;
}


/*
 *	Cast a literal vpt to a value_data_t
 */
static int cast_vpt(value_pair_tmpl_t *vpt, DICT_ATTR const *da)
{
	VALUE_PAIR *vp;
	value_data_t *data;

	rad_assert(vpt->type == VPT_TYPE_LITERAL);

	vp = pairalloc(vpt, da);
	if (!vp) return false;

	if (!pairparsevalue(vp, vpt->name)) {
		pairfree(&vp);
		return false;
	}

	vpt->length = vp->length;
	vpt->vpd = data = talloc(vpt, value_data_t);
	if (!vpt->vpd) return false;

	vpt->type = VPT_TYPE_DATA;
	vpt->da = da;

	if (vp->da->flags.is_pointer) {
		data->ptr = talloc_steal(vpt, vp->data.ptr);
		vp->data.ptr = NULL;
	} else {
		memcpy(data, &vp->data, sizeof(*data));
	}

	pairfree(&vp);

	return true;
}

static ssize_t condition_tokenize_string(TALLOC_CTX *ctx, char const *start, char **out,
					 FR_TOKEN *op, char const **error)
{
	char const *p = start;
	char *q;

	switch (*p++) {
	default:
		return -1;

	case '"':
		*op = T_DOUBLE_QUOTED_STRING;
		break;

	case '\'':
		*op = T_SINGLE_QUOTED_STRING;
		break;

	case '`':
		*op = T_BACK_QUOTED_STRING;
		break;

	case '/':
		*op = T_OP_REG_EQ; /* a bit of a hack. */
		break;

	}

	*out = talloc_array(ctx, char, strlen(start) - 1); /* + 2 - 1 */
	if (!*out) return -1;

	q = *out;

	while (*p) {
		if (*p == *start) {
			*q = '\0';
			p++;
			return (p - start);
		}

		if (*p == '\\') {
			p++;
			if (!*p) {
				*error = "End of string after escape";
				return -(p - start);
			}

			switch (*p) {
			case 'r':
				*q++ = '\r';
				break;
			case 'n':
				*q++ = '\n';
				break;
			case 't':
				*q++ = '\t';
				break;
			default:
				*q++ = *p;
				break;
			}
			p++;
			continue;
		}

		*(q++) = *(p++);
	}

	*error = "Unterminated string";
	return -1;
}

static ssize_t condition_tokenize_word(TALLOC_CTX *ctx, char const *start, char **out,
				       FR_TOKEN *op, char const **error)
{
	size_t len;
	char const *p = start;

	if ((*p == '"') || (*p == '\'') || (*p == '`') || (*p == '/')) {
		return condition_tokenize_string(ctx, start, out, op, error);
	}

	*op = T_BARE_WORD;
	if (*p == '&') p++;	/* special-case &User-Name */

	while (*p) {
		/*
		 *	The LHS should really be limited to only a few
		 *	things.  For now, we allow pretty much anything.
		 */
		if (*p == '\\') {
			*error = "Unexpected escape";
			return -(p - start);
		}

		/*
		 *	("foo") is valid.
		 */
		if (*p == ')') {
			break;
		}

		/*
		 *	Spaces or special characters delineate the word
		 */
		if (isspace((int) *p) || (*p == '&') || (*p == '|') ||
		    (*p == '!') || (*p == '=') || (*p == '<') || (*p == '>')) {
			break;
		}

		if ((*p == '"') || (*p == '\'') || (*p == '`')) {
			*error = "Unexpected start of string";
			return -(p - start);
		}

		p++;
	}

	len = p - start;
	if (!len) {
		*error = "Empty string is invalid";
		return 0;
	}

	*out = talloc_array(ctx, char, len + 1);
	memcpy(*out, start, len);
	(*out)[len] = '\0';
	return len;
}


static ssize_t condition_tokenize_cast(char const *start, DICT_ATTR const **pda, char const **error)
{
	char const *p = start;
	char const *q;
	PW_TYPE cast;

	while (isspace((int) *p)) p++; /* skip spaces before condition */

	if (*p != '<') return 0;
	p++;

	q = p;
	while (*q && *q != '>') q++;

	cast = fr_substr2int(dict_attr_types, p, PW_TYPE_INVALID, q - p);
	if (cast == PW_TYPE_INVALID) {
		*error = "Invalid data type in cast";
		return -(p - start);
	}

	*pda = dict_attrbyvalue(PW_CAST_BASE + cast, 0);
	if (!*pda) {
		*error = "Cannot cast to this data type";
		return -(p - start);
	}

	q++;

	while (isspace((int) *q)) q++; /* skip spaces after cast */

	return q - start;
}

/*
 *	Less code means less bugs
 */
#define return_P(_x) *error = _x;goto return_p
#define return_0(_x) *error = _x;goto return_0
#define return_lhs(_x) *error = _x;goto return_lhs
#define return_rhs(_x) *error = _x;goto return_rhs
#define return_SLEN goto return_slen


/** Tokenize a conditional check
 *
 *  @param[in] ctx for talloc
 *  @param[in] ci for CONF_ITEM
 *  @param[in] start the start of the string to process.  Should be "(..."
 *  @param[in] brace look for a closing brace
 *  @param[in] flags do one/two pass
 *  @param[out] pcond pointer to the returned condition structure
 *  @param[out] error the parse error (if any)
 *  @return length of the string skipped, or when negative, the offset to the offending error
 */
static ssize_t condition_tokenize(TALLOC_CTX *ctx, CONF_ITEM *ci, char const *start, int brace, fr_cond_t **pcond, char const **error, int flags)
{
	ssize_t slen;
	char const *p = start;
	char const *lhs_p, *rhs_p;
	fr_cond_t *c;
	char *lhs, *rhs;
	FR_TOKEN op, lhs_type, rhs_type;

	c = talloc_zero(ctx, fr_cond_t);

	rad_assert(c != NULL);
	lhs = rhs = NULL;

	while (isspace((int) *p)) p++; /* skip spaces before condition */

	if (!*p) {
		return_P("Empty condition is invalid");
	}

	/*
	 *	!COND
	 */
	if (*p == '!') {
		p++;
		c->negate = true;
		while (isspace((int) *p)) p++; /* skip spaces after negation */

		/*
		 *  Just for stupidity
		 */
		if (*p == '!') {
			return_P("Double negation is invalid");
		}
	}

	/*
	 *	(COND)
	 */
	if (*p == '(') {
		p++;

		/*
		 *	We've already eaten one layer of
		 *	brackets.  Go recurse to get more.
		 */
		c->type = COND_TYPE_CHILD;
		slen = condition_tokenize(c, ci, p, true, &c->data.child, error, flags);
		if (slen <= 0) {
			return_SLEN;
		}

		if (!c->data.child) {
			return_P("Empty condition is invalid");
		}

		p += slen;
		while (isspace((int) *p)) p++; /* skip spaces after (COND)*/

	} else { /* it's a bare FOO==BAR */
		/*
		 *	We didn't see anything special.  The condition must be one of
		 *
		 *	FOO
		 *	FOO OP BAR
		 */

		/*
		 *	Grab the LHS
		 */
		if (*p == '/') {
			return_P("Conditional check cannot begin with a regular expression");
		}

		slen = condition_tokenize_cast(p, &c->cast, error);
		if (slen < 0) {
			return_SLEN;
		}
		p += slen;

		lhs_p = p;
		slen = condition_tokenize_word(c, p, &lhs, &lhs_type, error);
		if (slen <= 0) {
			return_SLEN;
		}
		p += slen;

		while (isspace((int)*p)) p++; /* skip spaces after LHS */

		/*
		 *	We may (or not) have an operator
		 */


		/*
		 *	(FOO)
		 */
		if (*p == ')') {
			/*
			 *	don't skip the brace.  We'll look for it later.
			 */
			goto exists;

			/*
			 *	FOO
			 */
		} else if (!*p) {
			if (brace) {
				return_P("No closing brace at end of string");
			}

			goto exists;

			/*
			 *	FOO && ...
			 */
		} else if (((p[0] == '&') && (p[1] == '&')) ||
			   ((p[0] == '|') && (p[1] == '|'))) {

		exists:
			if (c->cast) {
				return_0("Cannot do cast for existence check");
			}

			if (lhs_type == T_BARE_WORD) {
				if ((strcmp(lhs, "true") == 0) ||
				    ((lhs[0] == '1') && !lhs[1])) {
					c->type = COND_TYPE_TRUE;

				} else if ((strcmp(lhs, "false") == 0) ||
					   ((lhs[0] == '0') && !lhs[1])) {
					c->type = COND_TYPE_FALSE;

				} else {
					goto create_exists;
				}

			} else {
			create_exists:
				c->type = COND_TYPE_EXISTS;
				c->data.vpt = radius_str2tmpl(c, lhs, lhs_type);
				if (!c->data.vpt) {
					return_P("Failed creating exists");
				}
			}

		} else { /* it's an operator */
			int regex;

			/*
			 *	The next thing should now be a comparison operator.
			 */
			regex = false;
			c->type = COND_TYPE_MAP;
			switch (*p) {
			default:
				return_P("Invalid text. Expected comparison operator");

			case '!':
				if (p[1] == '=') {
					op = T_OP_NE;
					p += 2;

				} else if (p[1] == '~') {
				regex = true;

				op = T_OP_REG_NE;
				p += 2;

				} else if (p[1] == '*') {
					if (lhs_type != T_BARE_WORD) {
						return_P("Cannot use !* on a string");
					}

					op = T_OP_CMP_FALSE;
					p += 2;

				} else {
					goto invalid_operator;
				}
				break;

			case '=':
				if (p[1] == '=') {
					op = T_OP_CMP_EQ;
					p += 2;

				} else if (p[1] == '~') {
					regex = true;

					op = T_OP_REG_EQ;
					p += 2;

				} else if (p[1] == '*') {
					if (lhs_type != T_BARE_WORD) {
						return_P("Cannot use =* on a string");
					}

					op = T_OP_CMP_TRUE;
					p += 2;

				} else {
				invalid_operator:
					return_P("Invalid operator");
				}

				break;

			case '<':
				if (p[1] == '=') {
					op = T_OP_LE;
					p += 2;

				} else {
					op = T_OP_LT;
					p++;
				}
				break;

			case '>':
				if (p[1] == '=') {
					op = T_OP_GE;
					p += 2;

				} else {
					op = T_OP_GT;
					p++;
				}
				break;
			}

			while (isspace((int) *p)) p++; /* skip spaces after operator */

			if (!*p) {
				return_P("Expected text after operator");
			}

			/*
			 *	Cannot have a cast on the RHS
			 */
			if (*p == '<') {
				return_P("Unexpected cast");
			}

			/*
			 *	Grab the RHS
			 */
			rhs_p = p;
			slen = condition_tokenize_word(c, p, &rhs, &rhs_type, error);
			if (slen <= 0) {
				return_SLEN;
			}

			/*
			 *	Sanity checks for regexes.
			 */
			if (regex) {
				if (*p != '/') {
					return_P("Expected regular expression");
				}

				/*
				 *	Allow /foo/i
				 */
				if (p[slen] == 'i') {
					c->regex_i = true;
					slen++;
				}

			} else if (!regex && (*p == '/')) {
				return_P("Unexpected regular expression");
			}

			c->data.map = radius_str2map(c, lhs, lhs_type, op, rhs, rhs_type,
						     REQUEST_CURRENT, PAIR_LIST_REQUEST,
						     REQUEST_CURRENT, PAIR_LIST_REQUEST);

			if (!c->data.map) {
				return_0("Syntax error");
			}

			/*
			 *	Could have been a reference to an attribute which is registered later.
			 *	Mark it as being checked in pass2.
			 */
			if ((lhs_type == T_BARE_WORD) &&
			    (c->data.map->dst->type == VPT_TYPE_LITERAL)) {
				c->pass2_fixup = PASS2_FIXUP_ATTR;
			}

			/*
			 *	Save the CONF_ITEM for later.
			 */
			c->data.map->ci = ci;

			/*
			 *	foo =* bar is just (foo)
			 *	foo !* bar is just (!foo)
			 */
			if ((op == T_OP_CMP_TRUE) || (op == T_OP_CMP_FALSE)) {
				value_pair_tmpl_t *vpt;

				vpt = talloc_steal(c, c->data.map->dst);
				c->data.map->dst = NULL;

				talloc_free(c->data.map);
				c->type = COND_TYPE_EXISTS;
				c->data.vpt = vpt;

				/*
				 *	Invert the negation bit.
				 */
				if (op == T_OP_CMP_FALSE) {
					c->negate = !c->negate;
				}

				goto done_cond;
			}

			/*
			 *	@todo: check LHS and RHS separately, to
			 *	get better errors
			 */
			if ((c->data.map->src->type == VPT_TYPE_LIST) ||
			    (c->data.map->dst->type == VPT_TYPE_LIST)) {
				return_0("Cannot use list references in condition");
			}

			/*
			 *	Check cast type.  We can have the RHS
			 *	a string if the LHS has a cast.  But
			 *	if the RHS is an attr, it MUST be the
			 *	same type as the LHS.
			 */
			if (c->cast) {
				if ((c->data.map->src->type == VPT_TYPE_ATTR) &&
				    (c->cast->type != c->data.map->src->da->type)) {
					goto same_type;
				}

				if (c->data.map->src->type == VPT_TYPE_REGEX) {
					return_0("Cannot use cast with regex comparison");
				}

				/*
				 *	The LHS is a literal which has been cast to a data type.
				 *	Cast it to the appropriate data type.
				 */
				if ((c->data.map->dst->type == VPT_TYPE_LITERAL) &&
				    !cast_vpt(c->data.map->dst, c->cast)) {
					*error = "Failed to parse field";
					if (lhs) talloc_free(lhs);
					if (rhs) talloc_free(rhs);
					talloc_free(c);
					return -(lhs_p - start);
				}

				/*
				 *	The RHS is a literal, and the LHS has been cast to a data
				 *	type.
				 */
				if ((c->data.map->dst->type == VPT_TYPE_DATA) &&
				    (c->data.map->src->type == VPT_TYPE_LITERAL) &&
				    !cast_vpt(c->data.map->src, c->data.map->dst->da)) {
					return_rhs("Failed to parse field");
				}

				/*
				 *	Casting to a redundant type means we don't need the cast.
				 *
				 *	Do this LAST, as the rest of the code above assumes c->cast
				 *	is not NULL.
				 */
				if ((c->data.map->dst->type == VPT_TYPE_ATTR) &&
				    (c->cast->type == c->data.map->dst->da->type)) {
					c->cast = NULL;
				}

			} else {
				/*
				 *	Two attributes?  They must be of the same type
				 */
				if ((c->data.map->src->type == VPT_TYPE_ATTR) &&
				    (c->data.map->dst->type == VPT_TYPE_ATTR) &&
				    (c->data.map->dst->da->type != c->data.map->src->da->type)) {
				same_type:
					return_0("Attribute comparisons must be of the same attribute type");
				}

				/*
				 *	Without a cast, we can't compare "foo" to User-Name,
				 *	it has to be done the other way around.
				 */
				if ((c->data.map->src->type == VPT_TYPE_ATTR) &&
				    (c->data.map->dst->type != VPT_TYPE_ATTR)) {
					*error = "Cannot use attribute reference on right side of condition";
				return_0:
					if (lhs) talloc_free(lhs);
					if (rhs) talloc_free(rhs);
					talloc_free(c);
					return 0;
				}

				/*
				 *	Invalid: User-Name == bob
				 *	Valid:   User-Name == "bob"
				 */
				if ((c->data.map->dst->type == VPT_TYPE_ATTR) &&
				    (c->data.map->src->type != VPT_TYPE_ATTR) &&
				    (c->data.map->dst->da->type == PW_TYPE_STRING) &&
				    (rhs_type == T_BARE_WORD)) {
					return_rhs("Must have string as value for attribute");
				}

				/*
				 *	Quotes around non-string
				 *	attributes mean that it's
				 *	either xlat, or an exec.
				 */
				if ((c->data.map->dst->type == VPT_TYPE_ATTR) &&
				    (c->data.map->src->type != VPT_TYPE_ATTR) &&
				    (c->data.map->dst->da->type != PW_TYPE_STRING) &&
				    (c->data.map->dst->da->type != PW_TYPE_OCTETS) &&
				    (c->data.map->dst->da->type != PW_TYPE_DATE) &&
				    (rhs_type == T_SINGLE_QUOTED_STRING)) {
					*error = "Value must be an unquoted string";
				return_rhs:
					if (lhs) talloc_free(lhs);
					if (rhs) talloc_free(rhs);
					talloc_free(c);
					return -(rhs_p - start);
				}

				/*
				 *	The LHS has been cast to a data type, and the RHS is a
				 *	literal.  Cast the RHS to the type of the cast.
				 */
				if (c->cast && (c->data.map->src->type == VPT_TYPE_LITERAL) &&
				    !cast_vpt(c->data.map->src, c->cast)) {
					return_rhs("Failed to parse field");
				}

				/*
				 *	The LHS is an attribute, and the RHS is a literal.  Cast the
				 *	RHS to the data type of the LHS.
				 */
				if ((c->data.map->dst->type == VPT_TYPE_ATTR) &&
				    (c->data.map->src->type == VPT_TYPE_LITERAL) &&
				    !cast_vpt(c->data.map->src, c->data.map->dst->da)) {
					DICT_ATTR const *da = c->data.map->dst->da;

					if ((da->vendor == 0) &&
					    ((da->attr == PW_AUTH_TYPE) ||
					     (da->attr == PW_AUTZ_TYPE) ||
					     (da->attr == PW_ACCT_TYPE) ||
					     (da->attr == PW_SESSION_TYPE) ||
					     (da->attr == PW_POST_AUTH_TYPE) ||
					     (da->attr == PW_PRE_PROXY_TYPE) ||
					     (da->attr == PW_POST_PROXY_TYPE) ||
					     (da->attr == PW_PRE_ACCT_TYPE) ||
					     (da->attr == PW_RECV_COA_TYPE) ||
					     (da->attr == PW_SEND_COA_TYPE))) {
						/*
						 *	The types for these attributes are dynamically allocated
						 *	by modules.c, so we can't enforce strictness here.
						 */
						c->pass2_fixup = PASS2_FIXUP_TYPE;

					} else {
						return_rhs("Failed to parse value for attribute");
					}
				}
			}

		done_cond:
			p += slen;

			while (isspace((int) *p)) p++; /* skip spaces after RHS */
		} /* parse OP RHS */
	} /* parse a condition (COND) or FOO OP BAR*/

	/*
	 *	...COND)
	 */
	if (*p == ')') {
		if (!brace) {
			return_P("Unexpected closing brace");
		}

		p++;
		while (isspace((int) *p)) p++; /* skip spaces after closing brace */
		brace = false;
		goto done;
	}

	/*
	 *	End of string is now allowed.
	 */
	if (!*p) {
		if (brace) {
			return_P("No closing brace at end of string");
		}

		goto done;
	}

	if (!(((p[0] == '&') && (p[1] == '&')) ||
	      ((p[0] == '|') && (p[1] == '|')))) {
		*error = "Unexpected text after condition";
	return_p:
		if (lhs) talloc_free(lhs);
		if (rhs) talloc_free(rhs);
		talloc_free(c);
		return -(p - start);
	}

	/*
	 *	Recurse to parse the next condition.
	 */
	c->next_op = p[0];
	p += 2;

	/*
	 *	May still be looking for a closing brace.
	 */
	slen = condition_tokenize(c, ci, p, brace, &c->next, error, flags);
	if (slen <= 0) {
	return_slen:
		if (lhs) talloc_free(lhs);
		if (rhs) talloc_free(rhs);
		talloc_free(c);
		return slen - (p - start);
	}
	p += slen;

done:
	/*
	 *	Normalize it before returning it.
	 */

	/*
	 *	(FOO)     --> FOO
	 *	(FOO) ... --> FOO ...
	 */
	if ((c->type == COND_TYPE_CHILD) && !c->data.child->next) {
		fr_cond_t *child;

		child = talloc_steal(ctx, c->data.child);
		c->data.child = NULL;

		child->next = talloc_steal(child, c->next);
		c->next = NULL;

		child->next_op = c->next_op;

		/*
		 *	Set the negation properly
		 */
		if ((c->negate && !child->negate) ||
		    (!c->negate && child->negate)) {
			child->negate = true;
		} else {
			child->negate = false;
		}

		lhs = rhs = NULL;
		talloc_free(c);
		c = child;
	}

	/*
	 *	(FOO ...) --> FOO ...
	 *
	 *	But don't do !(FOO || BAR) --> !FOO || BAR
	 *	Because that's different.
	 */
	if ((c->type == COND_TYPE_CHILD) &&
	    !c->next && !c->negate) {
		fr_cond_t *child;

		child = talloc_steal(ctx, c->data.child);
		c->data.child = NULL;

		lhs = rhs = NULL;
		talloc_free(c);
		c = child;
	}

	/*
	 *	Normalize negation.  This doesn't really make any
	 *	difference, but it simplifies the run-time code in
	 *	evaluate.c
	 */
	if (c->type == COND_TYPE_MAP) {
		/*
		 *	!FOO !~ BAR --> FOO =~ BAR
		 */
		if (c->negate && (c->data.map->op == T_OP_REG_NE)) {
			c->negate = false;
			c->data.map->op = T_OP_REG_EQ;
		}

		/*
		 *	FOO !~ BAR --> !FOO =~ BAR
		 */
		if (!c->negate && (c->data.map->op == T_OP_REG_NE)) {
			c->negate = true;
			c->data.map->op = T_OP_REG_EQ;
		}

		/*
		 *	!FOO != BAR --> FOO == BAR
		 */
		if (c->negate && (c->data.map->op == T_OP_NE)) {
			c->negate = false;
			c->data.map->op = T_OP_CMP_EQ;
		}

		/*
		 *	This next one catches "LDAP-Group != foo",
		 *	which doesn't really work, but this hack fixes it.
		 *
		 *	FOO != BAR --> !FOO == BAR
		 */
		if (!c->negate && (c->data.map->op == T_OP_NE)) {
			c->negate = true;
			c->data.map->op = T_OP_CMP_EQ;
		}

		if ((c->data.map->dst->type == VPT_TYPE_DATA) &&
		    (c->data.map->src->type == VPT_TYPE_DATA)) {
			int rcode;

			rad_assert(c->cast != NULL);

			rcode = radius_evaluate_map(NULL, 0, 0, c);
			talloc_free(c->data.map);
			c->data.map = NULL;
			c->cast = NULL;
			c->regex_i = false;
			if (rcode) {
				c->type = COND_TYPE_TRUE;
			} else {
				c->type = COND_TYPE_FALSE;
			}
		}
	}

	if (c->type == COND_TYPE_TRUE) {
		if (c->negate) {
			c->negate = false;
			c->type = COND_TYPE_FALSE;
		}
	}

	if (c->type == COND_TYPE_FALSE) {
		if (c->negate) {
			c->negate = false;
			c->type = COND_TYPE_TRUE;
		}
	}

	/*
	 *	true && FOO --> FOO
	 */
	if ((c->type == COND_TYPE_TRUE) &&
	    (c->next_op == COND_AND)) {
		fr_cond_t *next;

		next = talloc_steal(ctx, c->next);
		c->next = NULL;

		lhs = rhs = NULL;
		talloc_free(c);
		c = next;
	}

	/*
	 *	false && FOO --> false
	 */
	if ((c->type == COND_TYPE_FALSE) &&
	    (c->next_op == COND_AND)) {
		talloc_free(c->next);
		c->next = NULL;
		c->next_op = COND_NONE;
	}

	/*
	 *	false || FOO --> FOO
	 */
	if ((c->type == COND_TYPE_FALSE) &&
	    (c->next_op == COND_OR)) {
		fr_cond_t *next;

		next = talloc_steal(ctx, c->next);
		c->next = NULL;

		lhs = rhs = NULL;
		talloc_free(c);
		c = next;
	}

	/*
	 *	true || FOO --> true
	 */
	if ((c->type == COND_TYPE_TRUE) &&
	    (c->next_op == COND_OR)) {
		talloc_free(c->next);
		c->next = NULL;
		c->next_op = COND_NONE;
	}

	if (lhs) talloc_free(lhs);
	if (rhs) talloc_free(rhs);

	*pcond = c;
	return p - start;
}

/** Tokenize a conditional check
 *
 *  @param[in] ctx for talloc
 *  @param[in] ci for CONF_ITEM
 *  @param[in] start the start of the string to process.  Should be "(..."
 *  @param[out] head the parsed condition structure
 *  @param[out] error the parse error (if any)
 *  @param[in] flags do one/two pass
 *  @return length of the string skipped, or when negative, the offset to the offending error
 */
ssize_t fr_condition_tokenize(TALLOC_CTX *ctx, CONF_ITEM *ci, char const *start, fr_cond_t **head, char const **error, int flags)
{
	return condition_tokenize(ctx, ci, start, false, head, error, flags);
}

/*
 *	Walk in order.
 */
bool fr_condition_walk(fr_cond_t *c, bool (*callback)(void *, fr_cond_t *), void *ctx)
{
	while (c) {
		/*
		 *	Process this one, exit on error.
		 */
		if (!callback(ctx, c)) return false;

		switch (c->type) {
		case COND_TYPE_INVALID:
			return false;

		case COND_TYPE_EXISTS:
		case COND_TYPE_MAP:
		case COND_TYPE_TRUE:
		case COND_TYPE_FALSE:
			break;

		case COND_TYPE_CHILD:
			/*
			 *	Walk over the child.
			 */
			if (!fr_condition_walk(c->data.child, callback, ctx)) {
				return false;
			}
		}

		/*
		 *	No sibling, stop.
		 */
		if (c->next_op == COND_NONE) break;

		/*
		 *	process the next sibling
		 */
		c = c->next;
	}

	return true;
}
