/*
 * valuepair.c	Functions to handle VALUE_PAIRs
 *
 * Version:	$Id$
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) any later version.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 *   Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this library; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 *
 * Copyright 2000,2006  The FreeRADIUS server project
 */

RCSID("$Id$")

#include <freeradius-devel/libradius.h>

#include <ctype.h>

#ifdef HAVE_PCREPOSIX_H
#  define WITH_REGEX
#  include <pcreposix.h>
#elif defined(HAVE_REGEX_H)
#  include <regex.h>
#  define WITH_REGEX

/*
 *  For POSIX Regular expressions.
 *  (0) Means no extended regular expressions.
 *  REG_EXTENDED means use extended regular expressions.
 */
#  ifndef REG_EXTENDED
#    define REG_EXTENDED (0)
#  endif

#  ifndef REG_NOSUB
#    define REG_NOSUB (0)
#  endif
#endif

static char const *months[] = {
	"jan", "feb", "mar", "apr", "may", "jun",
	"jul", "aug", "sep", "oct", "nov", "dec" };

#define attribute_eq(_x, _y) ((_x && _y) && (_x->da == _y->da) && (_x->tag == _y->tag))

/** Free a VALUE_PAIR
 *
 * @note Do not call directly, use talloc_free instead.
 *
 * @param vp to free.
 * @return 0
 */
static int _pairfree(VALUE_PAIR *vp) {
	/*
	 *	The lack of DA means something has gone wrong
	 */
	if (!vp->da) {
		fr_strerror_printf("VALUE_PAIR has NULL DICT_ATTR pointer (probably already freed)");
	/*
	 *	Only free the DICT_ATTR if it was dynamically allocated
	 *	and was marked for free when the VALUE_PAIR is freed.
	 *
	 *	@fixme This is an awful hack and needs to be removed once DICT_ATTRs are allocated by talloc.
	 */
	} else if (vp->da->flags.vp_free) {
		dict_attr_free(&(vp->da));
	}

#ifndef NDEBUG
	vp->vp_integer = FREE_MAGIC;
#endif

	return 0;
}

/** Dynamically allocate a new attribute
 *
 * Allocates a new attribute and a new dictionary attr if no DA is provided.
 *
 * @param[in] ctx for allocated memory, usually a pointer to a RADIUS_PACKET
 * @param[in] da Specifies the dictionary attribute to build the VP from.
 * @return a new value pair or NULL if an error occurred.
 */
VALUE_PAIR *pairalloc(TALLOC_CTX *ctx, DICT_ATTR const *da)
{
	VALUE_PAIR *vp;

	/*
	 *	Caller must specify a da else we don't know what the attribute type is.
	 */
	if (!da) return NULL;

	vp = talloc_zero(ctx, VALUE_PAIR);
	if (!vp) {
		fr_strerror_printf("Out of memory");
		return NULL;
	}

	vp->da = da;
	vp->op = T_OP_EQ;
	vp->type = VT_NONE;

	talloc_set_destructor(vp, _pairfree);

	return vp;
}

/** Create a new valuepair
 *
 * If attr and vendor match a dictionary entry then a VP with that DICT_ATTR
 * will be returned.
 *
 * If attr or vendor are uknown will call dict_attruknown to create a dynamic
 * DICT_ATTR of PW_TYPE_OCTETS.
 *
 * Which type of DICT_ATTR the VALUE_PAIR was created with can be determined by
 * checking @verbatim vp->da->flags.is_unknown @endverbatim.
 *
 * @param[in] ctx for allocated memory, usually a pointer to a RADIUS_PACKET
 * @param[in] attr number.
 * @param[in] vendor number.
 * @return the new valuepair or NULL on error.
 */
VALUE_PAIR *paircreate(TALLOC_CTX *ctx, unsigned int attr, unsigned int vendor)
{
	DICT_ATTR const *da;

	da = dict_attrbyvalue(attr, vendor);
	if (!da) {
		da = dict_attrunknown(attr, vendor, true);
		if (!da) {
			return NULL;
		}
	}

	return pairalloc(ctx, da);
}

/** Free memory used by a valuepair list.
 *
 * @todo TLV: needs to free all dependents of each VP freed.
 */
void pairfree(VALUE_PAIR **vps)
{
	VALUE_PAIR	*vp;
	vp_cursor_t	cursor;

	if (!vps) {
		return;
	}

	for (vp = paircursor(&cursor, vps);
	     vp;
	     vp = pairnext(&cursor)) {
		VERIFY_VP(vp);
		talloc_free(vp);
	}

	*vps = NULL;
}

/** Mark malformed or unrecognised attributed as unknown
 *
 * @param vp to change DICT_ATTR of.
 * @return 0 on success (or if already unknown) else -1 on error.
 */
int pair2unknown(VALUE_PAIR *vp)
{
	DICT_ATTR const *da;

	VERIFY_VP(vp);
	if (vp->da->flags.is_unknown) {
		return 0;
	}

	da = dict_attrunknown(vp->da->attr, vp->da->vendor, true);
	if (!da) {
		return -1;
	}

	vp->da = da;

	return 0;
}

/** Find the pair with the matching attribute
 *
 * @todo should take DAs and do a pointer comparison.
 */
VALUE_PAIR *pairfind(VALUE_PAIR *vp, unsigned int attr, unsigned int vendor,
		     int8_t tag)
{
	vp_cursor_t 	cursor;
	VALUE_PAIR	*i;

	for (i = paircursor(&cursor, &vp);
	     i;
	     i = pairnext(&cursor)) {
		VERIFY_VP(i);
		if ((i->da->attr == attr) && (i->da->vendor == vendor)
		    && ((tag == TAG_ANY) || (i->da->flags.has_tag &&
			(i->tag == tag)))) {
			return i;
		}
	}

	return NULL;
}

/** Setup a cursor to iterate over attribute pairs
 *
 * @param cursor Where to initialise the cursor (uses existing structure).
 * @param node to start from.
 */
VALUE_PAIR *paircursorc(vp_cursor_t *cursor, VALUE_PAIR const * const *node)
{
	memset(cursor, 0, sizeof(*cursor));

	if (!node || !cursor) {
		return NULL;
	}

	memcpy(&cursor->first, &node, sizeof(cursor->first));
	cursor->current = *cursor->first;

	if (cursor->current) {
		VERIFY_VP(cursor->current);
		cursor->next = cursor->current->next;
	}

	return cursor->current;
}

VALUE_PAIR *pairfirst(vp_cursor_t *cursor)
{
	cursor->current = *cursor->first;

	if (cursor->current) {
		VERIFY_VP(cursor->current);
		cursor->next = cursor->current->next;
		if (cursor->next) VERIFY_VP(cursor->next);
		cursor->found = NULL;
	}

	return cursor->current;
}

/** Iterate over attributes of a given type in the pairlist
 *
 *
 */
VALUE_PAIR *pairfindnext(vp_cursor_t *cursor, unsigned int attr, unsigned int vendor, int8_t tag)
{
	VALUE_PAIR *i;

	i = pairfind(!cursor->found ? cursor->current : cursor->found->next, attr, vendor, tag);
	if (!i) {
		cursor->next = NULL;
		cursor->current = NULL;

		return NULL;
	}

	cursor->next = i->next;
	cursor->current = i;
	cursor->found = i;

	return i;
}

/** Retrieve the next VALUE_PAIR
 *
 *
 */
VALUE_PAIR *pairnext(vp_cursor_t *cursor)
{
	cursor->current = cursor->next;
	if (cursor->current) {
		VERIFY_VP(cursor->current);

		/*
		 *	Set this now in case 'current' gets freed before
		 *	pairnext is called again.
		 */
		cursor->next = cursor->current->next;

		/*
		 *	Next call to pairfindnext will start from the current
		 *	position in the list, not the last found instance.
		 */
		cursor->found = NULL;
	}

	return cursor->current;
}

VALUE_PAIR *paircurrent(vp_cursor_t *cursor)
{
	if (cursor->current) {
		VERIFY_VP(cursor->current);
	}

	return cursor->current;
}

/** Insert a VP
 *
 * @todo don't use with pairdelete
 */
void pairinsert(vp_cursor_t *cursor, VALUE_PAIR *add)
{
	VALUE_PAIR *i;

	if (!add) {
		return;
	}

	VERIFY_VP(add);

	/*
	 *	Cursor was initialised with a pointer to a NULL value_pair
	 */
	if (!*cursor->first) {
		*cursor->first = add;
		cursor->current = add;
		cursor->next = cursor->current->next;

		return;
	}

	/*
	 *	We don't yet know where the last VALUE_PAIR is
	 *
	 *	Assume current is closer to the end of the list and use that if available.
	 */
	if (!cursor->last) {
		cursor->last = cursor->current ? cursor->current : *cursor->first;
	}

	VERIFY_VP(cursor->last);

	/*
	 *	Something outside of the cursor added another VALUE_PAIR
	 */
	if (cursor->last->next) {
		for (i = cursor->last; i; i = i->next) {
			VERIFY_VP(i);
			cursor->last = i;
		}
	}

	/*
	 *	Either current was never set, or something iterated to the end of the
	 *	attribute list.
	 */
	if (!cursor->current) {
		cursor->current = add;
	}

	cursor->last->next = add;
}

/** Remove the current pair
 *
 * @todo this is really inefficient and should be fixed...
 *
 * @param cursor to remove the current pair from.
 * @return NULL on error, else the VALUE_PAIR we just removed.
 */
VALUE_PAIR *pairremove(vp_cursor_t *cursor)
{
	VALUE_PAIR *vp, *last;

	vp = paircurrent(cursor);
	if (!vp) {
		return NULL;
	}

	last = *cursor->first;
	while (last && (last->next != vp)) {
		last = last->next;
	}

	pairnext(cursor);	/* Advance the cursor past the one were about to delete */

	last->next = vp->next;	/* re-link the list */
	vp->next = NULL;

	return vp;
}

/** Delete matching pairs
 *
 * Delete matching pairs from the attribute list.
 *
 * @param[in,out] first VP in list.
 * @param[in] attr to match.
 * @param[in] vendor to match.
 * @param[in] tag to match. TAG_ANY matches any tag, TAG_UNUSED matches tagless VPs.
 *
 * @todo should take DAs and do a point comparison.
 */
void pairdelete(VALUE_PAIR **first, unsigned int attr, unsigned int vendor,
		int8_t tag)
{
	VALUE_PAIR *i, *next;
	VALUE_PAIR **last = first;

	for(i = *first; i; i = next) {
		VERIFY_VP(i);
		next = i->next;
		if ((i->da->attr == attr) && (i->da->vendor == vendor) &&
		    ((tag == TAG_ANY) ||
		     (i->da->flags.has_tag && (i->tag == tag)))) {
			*last = next;
			talloc_free(i);
		} else {
			last = &i->next;
		}
	}
}

/** Add a VP to the end of the list.
 *
 * Locates the end of 'first', and links an additional VP 'add' at the end.
 *
 * @param[in] first VP in linked list. Will add new VP to the end of this list.
 * @param[in] add VP to add to list.
 */
void pairadd(VALUE_PAIR **first, VALUE_PAIR *add)
{
	VALUE_PAIR *i;

	if (!add) return;

	VERIFY_VP(add);

	if (*first == NULL) {
		*first = add;
		return;
	}
	for(i = *first; i->next; i = i->next)
		VERIFY_VP(i);
	i->next = add;
}

/** Replace all matching VPs
 *
 * Walks over 'first', and replaces the first VP that matches 'replace'.
 *
 * @note Memory used by the VP being replaced will be freed.
 * @note Will not work with unknown attributes.
 *
 * @param[in,out] first VP in linked list. Will search and replace in this list.
 * @param[in] replace VP to replace.
 */
void pairreplace(VALUE_PAIR **first, VALUE_PAIR *replace)
{
	VALUE_PAIR *i, *next;
	VALUE_PAIR **prev = first;

	VERIFY_VP(replace);

	if (*first == NULL) {
		*first = replace;
		return;
	}

	/*
	 *	Not an empty list, so find item if it is there, and
	 *	replace it. Note, we always replace the first one, and
	 *	we ignore any others that might exist.
	 */
	for(i = *first; i; i = next) {
		VERIFY_VP(i);
		next = i->next;

		/*
		 *	Found the first attribute, replace it,
		 *	and return.
		 */
		if ((i->da == replace->da) &&
		    (!i->da->flags.has_tag || (i->tag == replace->tag))
		) {
			*prev = replace;

			/*
			 *	Should really assert that replace->next == NULL
			 */
			replace->next = next;
			talloc_free(i);
			return;
		}

		/*
		 *	Point to where the attribute should go.
		 */
		prev = &i->next;
	}

	/*
	 *	If we got here, we didn't find anything to replace, so
	 *	stopped at the last item, which we just append to.
	 */
	*prev = replace;
}

static void pairsort_split(VALUE_PAIR *source, VALUE_PAIR **front, VALUE_PAIR **back)
{
	VALUE_PAIR *fast;
	VALUE_PAIR *slow;

	/*
	 *	Stopping condition - no more elements left to split
	 */
	if (!source || !source->next) {
    		*front = source;
    		*back = NULL;

  		return;
  	}

	/*
	 *	Fast advances twice as fast as slow, so when it gets to the end,
	 *	slow will point to the middle of the linked list.
	 */
	slow = source;
	fast = source->next;

	while (fast) {
		fast = fast->next;
		if (fast) {
			slow = slow->next;
			fast = fast->next;
		}
	}

	*front = source;
	*back = slow->next;
	slow->next = NULL;
}

static VALUE_PAIR *pairsort_merge(VALUE_PAIR *a, VALUE_PAIR *b, bool with_tag)
{
	VALUE_PAIR *result = NULL;

	if (!a) return b;
	if (!b) return a;

 	/*
 	 *	Compare the DICT_ATTRs and tags
 	 */
	if ((with_tag && (a->tag < b->tag)) || (a->da <= b->da)) {
		result = a;
     		result->next = pairsort_merge(a->next, b, with_tag);
  	} else {
		result = b;
		result->next = pairsort_merge(a, b->next, with_tag);
	}

	return result;
}

/** Sort a linked list of VALUE_PAIRs using merge sort
 *
 * @param[in,out] vps List of VALUE_PAIRs to sort.
 * @param[in] with_tag sort by tag then by DICT_ATTR
 */
void pairsort(VALUE_PAIR **vps, bool with_tag)
{
	VALUE_PAIR *head = *vps;
	VALUE_PAIR *a;
	VALUE_PAIR *b;

	/*
	 *	If there's 0-1 elements it must already be sorted.
	 */
	if (!head || !head->next) {
		return;
	}

	pairsort_split(head, &a, &b);	/* Split into sublists */
	pairsort(&a, with_tag);		/* Traverse left */
	pairsort(&b, with_tag);		/* Traverse right */

  	/*
  	 *	merge the two sorted lists together
  	 */
  	*vps = pairsort_merge(a, b, with_tag);
}

/** Uses paircmp to verify all VALUE_PAIRs in list match the filter defined by check
 *
 * @param filter attributes to check list against.
 * @param list attributes, probably a request or reply
 */
bool pairvalidate(VALUE_PAIR *filter, VALUE_PAIR *list)
{
	vp_cursor_t filter_cursor;
	vp_cursor_t list_cursor;

	VALUE_PAIR *check, *match, *last_check = NULL, *last_match;

	if (!filter && !list) {
		return true;
	}
	if (!filter || !list) {
		return false;
	}

	/*
	 *	This allows us to verify the sets of validate and reply are equal
	 *	i.e. we have a validate rule which matches every reply attribute.
	 *
	 *	@todo this should be removed one we have sets and lists
	 */
	pairsort(&filter, true);
	pairsort(&list, true);

	paircursor(&list_cursor, &list);
	for (check = paircursor(&filter_cursor, &filter);
	     check;
	     check = pairnext(&filter_cursor)) {
	     	/*
	     	 *	Were processing check attributes of a new type.
	     	 */
	     	if (attribute_eq(last_check, check)) {
	     		/*
	     		 *	The lists have gone out of sync so we know the sets
	     		 *	of list and filter are not equal.
	     		 */
	     		if (!attribute_eq(paircurrent(&list_cursor), paircurrent(&filter_cursor))) {
	     			return false;
	     		}

	     		last_match = paircurrent(&list_cursor);
	     		paircursor(&list_cursor, &last_match);	/* not strictly needed */
	     		last_check = check;
	     	}

		for (match = pairfirst(&list_cursor);
	     	     attribute_eq(match, check);
	             match = pairnext(&list_cursor)) {
	             	/*
	             	 *	This attribute passed the filter
	             	 */
	             	if (!paircmp(check, match)) {
	             		return false;
	             	}
	        }
	}

	/*
	 *	There were additional VALUE_PAIRS left in the list
	 */
	if (paircurrent(&list_cursor)) {
		return false;
	}

	return true;
}

/** Copy a single valuepair
 *
 * Allocate a new valuepair and copy the da from the old vp.
 *
 * @param[in] ctx for talloc
 * @param[in] vp to copy.
 * @return a copy of the input VP or NULL on error.
 */
VALUE_PAIR *paircopyvp(TALLOC_CTX *ctx, VALUE_PAIR const *vp)
{
	VALUE_PAIR *n;

	if (!vp) return NULL;

	VERIFY_VP(vp);

	n = pairalloc(ctx, vp->da);
	if (!n) {
		fr_strerror_printf("out of memory");
		return NULL;
	}

	memcpy(n, vp, sizeof(*n));

	/*
	 *	Now copy the value
	 */
	if (vp->type == VT_XLAT) {
		n->value.xlat = talloc_strdup(n, n->value.xlat);
	}

	n->da = dict_attr_copy(vp->da, true);
	if (!n->da) {
		talloc_free(n);
		return NULL;
	}

	n->next = NULL;

	if ((n->da->type == PW_TYPE_TLV) ||
	    (n->da->type == PW_TYPE_OCTETS)) {
		if (n->vp_octets != NULL) {
			n->vp_octets = talloc_memdup(n, vp->vp_octets, n->length);
		}

	} else if (n->da->type == PW_TYPE_STRING) {
		if (n->vp_strvalue != NULL) {
			/*
			 *	Equivalent to, and faster than strdup.
			 */
			n->vp_strvalue = talloc_memdup(n, vp->vp_octets, n->length + 1);
		}
	}

	return n;
}

/** Copy data from one VP to another
 *
 * Allocate a new pair using da, and copy over the value from the specified
 * vp.
 *
 * @todo Should be able to do type conversions.
 *
 * @param[in] ctx for talloc
 * @param[in] da of new attribute to alloc.
 * @param[in] vp to copy data from.
 * @return the new valuepair.
 */
VALUE_PAIR *paircopyvpdata(TALLOC_CTX *ctx, DICT_ATTR const *da, VALUE_PAIR const *vp)
{
	VALUE_PAIR *n;

	if (!vp) return NULL;

	VERIFY_VP(vp);

	/*
	 *	The types have to be identical, OR the "from" VP has
	 *	to be octets.
	 */
	if (da->type != vp->da->type) {
		int length;
		uint8_t *p;
		VALUE_PAIR const **pvp;

		if (vp->da->type == PW_TYPE_OCTETS) {
			/*
			 *	Decode the data.  It may be wrong!
			 */
			if (rad_data2vp(da->attr, da->vendor, vp->vp_octets, vp->length, &n) < 0) {
				return NULL;
			}

			n->type = VT_DATA;
			return n;
		}

		/*
		 *	Else the destination type is octets
		 */
		switch (vp->da->type) {
		default:
			return NULL; /* can't do it */

		case PW_TYPE_INTEGER:
		case PW_TYPE_IPADDR:
		case PW_TYPE_DATE:
		case PW_TYPE_IFID:
		case PW_TYPE_IPV6ADDR:
		case PW_TYPE_IPV6PREFIX:
		case PW_TYPE_BYTE:
		case PW_TYPE_SHORT:
		case PW_TYPE_ETHERNET:
		case PW_TYPE_SIGNED:
		case PW_TYPE_INTEGER64:
		case PW_TYPE_IPV4PREFIX:
			break;
		}

		n = pairalloc(ctx, da);
		if (!n) return NULL;

		p = talloc_array(n, uint8_t, dict_attr_sizes[vp->da->type][1] + 2);

		pvp = &vp;
		length = rad_vp2attr(NULL, NULL, NULL, pvp, p, dict_attr_sizes[vp->da->type][1]);
		if (length < 0) {
			pairfree(&n);
			return NULL;
		}

		pairmemcpy(n, p + 2, length - 2);
		talloc_free(p);
		return n;
	}

	n = pairalloc(ctx, da);
	if (!n) {
		return NULL;
	}

	memcpy(n, vp, sizeof(*n));
	n->da = da;

	if (n->type == VT_XLAT) {
		n->value.xlat = talloc_strdup(n, n->value.xlat);
	}

	switch (n->da->type) {
		case PW_TYPE_TLV:
		case PW_TYPE_OCTETS:
			if (n->vp_octets != NULL) {
				n->vp_octets = talloc_memdup(n, vp->vp_octets, n->length);
			}
			break;

		case PW_TYPE_STRING:
			if (n->vp_strvalue != NULL) {
				n->vp_strvalue = talloc_memdup(n, vp->vp_strvalue, n->length + 1);	/* NULL byte */
			}
			break;
		default:
			return NULL;
	}

	n->next = NULL;

	return n;
}


/** Copy matching pairs
 *
 * Copy pairs of a matching attribute number, vendor number and tag from the
 * the input list to a new list, and returns the head of this list.
 *
 * @param[in] ctx for talloc
 * @param[in] from whence to copy VALUE_PAIRs.
 * @param[in] attr to match, if 0 input list will not be filtered by attr.
 * @param[in] vendor to match.
 * @param[in] tag to match, TAG_ANY matches any tag, TAG_UNUSED matches tagless VPs.
 * @return the head of the new VALUE_PAIR list or NULL on error.
 */
VALUE_PAIR *paircopy2(TALLOC_CTX *ctx, VALUE_PAIR *from,
		      unsigned int attr, unsigned int vendor, int8_t tag)
{
	vp_cursor_t src, dst;

	VALUE_PAIR *out = NULL, *vp;

	paircursor(&dst, &out);
	for (vp = paircursor(&src, &from);
	     vp;
	     vp = pairnext(&src)) {
	     	VERIFY_VP(vp);

		if ((attr > 0) && ((vp->da->attr != attr) || (vp->da->vendor != vendor))) {
			continue;
		}

		if ((tag != TAG_ANY) && vp->da->flags.has_tag && (vp->tag != tag)) {
			continue;
		}

		vp = paircopyvp(ctx, vp);
		if (!vp) {
			pairfree(&out);
			return NULL;
		}
		pairinsert(&dst, vp);
	}

	return out;
}


/** Copy a pairlist.
 *
 * Copy all pairs from 'from' regardless of tag, attribute or vendor.
 *
 * @param[in] ctx for new VALUE_PAIRs to be allocated in.
 * @param[in] from whence to copy VALUE_PAIRs.
 * @return the head of the new VALUE_PAIR list or NULL on error.
 */
VALUE_PAIR *paircopy(TALLOC_CTX *ctx, VALUE_PAIR *from)
{
	vp_cursor_t src, dst;

	VALUE_PAIR *out = NULL, *vp;

	paircursor(&dst, &out);
	for (vp = paircursor(&src, &from);
	     vp;
	     vp = pairnext(&src)) {
	     	VERIFY_VP(vp);
	     	vp = paircopyvp(ctx, vp);
	     	if (!vp) {
	     		pairfree(&out);
	     		return NULL;
	     	}
		pairinsert(&dst, vp); /* paircopy sets next pointer to NULL */
	}

	return out;
}

/** Move pairs from source list to destination list respecting operator
 *
 * @note This function does some additional magic that's probably not needed
 *	 in most places. Consider using radius_pairmove in server code.
 *
 * @note pairfree should be called on the head of the source list to free
 *	 unmoved attributes (if they're no longer needed).
 *
 * @note Does not respect tags when matching.
 *
 * @param[in] ctx for talloc
 * @param[in,out] to destination list.
 * @param[in,out] from source list.
 *
 * @see radius_pairmove
 */
void pairmove(TALLOC_CTX *ctx, VALUE_PAIR **to, VALUE_PAIR **from)
{
	VALUE_PAIR **tailto, *i, *j, *next;
	VALUE_PAIR *tailfrom = NULL;
	VALUE_PAIR *found;
	int has_password = 0;

	if (!to || !from || !*from) return;

	/*
	 *	First, see if there are any passwords here, and
	 *	point "tailto" to the end of the "to" list.
	 */
	tailto = to;
	if (*to) for (i = *to; i; i = i->next) {
		VERIFY_VP(i);
		if (!i->da->vendor &&
		    (i->da->attr == PW_USER_PASSWORD ||
		     i->da->attr == PW_CRYPT_PASSWORD))
			has_password = 1;
		tailto = &i->next;
	}

	/*
	 *	Loop over the "from" list.
	 */
	for (i = *from; i; i = next) {
		VERIFY_VP(i);
		next = i->next;

		/*
		 *	If there was a password in the "to" list,
		 *	do not move any other password from the
		 *	"from" to the "to" list.
		 */
		if (has_password && !i->da->vendor &&
		    (i->da->attr == PW_USER_PASSWORD ||
		     i->da->attr == PW_CRYPT_PASSWORD)) {
			tailfrom = i;
			continue;
		}

		switch (i->op) {
			/*
			 *	These are COMPARISON attributes
			 *	from a check list, and are not
			 *	supposed to be copied!
			 */
			case T_OP_NE:
			case T_OP_GE:
			case T_OP_GT:
			case T_OP_LE:
			case T_OP_LT:
			case T_OP_CMP_TRUE:
			case T_OP_CMP_FALSE:
			case T_OP_CMP_EQ:
			case T_OP_REG_EQ:
			case T_OP_REG_NE:
				tailfrom = i;
				continue;

			default:
				break;
		}

		/*
		 *	If the attribute is already present in "to",
		 *	do not move it from "from" to "to". We make
		 *	an exception for "Hint" which can appear multiple
		 *	times, and we never move "Fall-Through".
		 */
		if (i->da->attr == PW_FALL_THROUGH ||
		    (i->da->attr != PW_HINT && i->da->attr != PW_FRAMED_ROUTE)) {


			found = pairfind(*to, i->da->attr, i->da->vendor,
					 TAG_ANY);

			switch (i->op) {

			/*
			 *	If matching attributes are found,
			 *	delete them.
			 */
			case T_OP_SUB:		/* -= */
				if (found) {
					if (!i->vp_strvalue[0] ||
					    (strcmp(found->vp_strvalue,
						    i->vp_strvalue) == 0)){
						pairdelete(to,
							   found->da->attr,
							   found->da->vendor,
							   TAG_ANY);

						/*
						 *	'tailto' may have been
						 *	deleted...
						 */
						tailto = to;
						for(j = *to; j; j = j->next) {
							tailto = &j->next;
						}
					}
				}
				tailfrom = i;
				continue;

			case T_OP_EQ:		/* = */
				/*
				 *  FIXME: Tunnel attributes with
				 *  different tags are different
				 *  attributes.
				 */
				if (found) {
					tailfrom = i;
					continue; /* with the loop */
				}
				break;

			  /*
			   *  If a similar attribute is found,
			   *  replace it with the new one.  Otherwise,
			   *  add the new one to the list.
			   */
			case T_OP_SET:		/* := */
				if (found) {
					VALUE_PAIR *mynext = found->next;

					/*
					 *	Do NOT call pairdelete()
					 *	here, due to issues with
					 *	re-writing "request->username".
					 *
					 *	Everybody calls pairmove,
					 *	and expects it to work.
					 *	We can't update request->username
					 *	here, so instead we over-write
					 *	the vp that it's pointing to.
					 */
					memcpy(found, i, sizeof(*found));
					found->next = mynext;

					pairdelete(&found->next,
						   found->da->attr,
						   found->da->vendor, TAG_ANY);

					/*
					 *	'tailto' may have been
					 *	deleted...
					 */
					for(j = found; j; j = j->next) {
						tailto = &j->next;
					}
					continue;
				}
				break;

			  /*
			   *  Add the new element to the list, even
			   *  if similar ones already exist.
			   */
			default:
			case T_OP_ADD: /* += */
				break;
			}
		}
		if (tailfrom)
			tailfrom->next = next;
		else
			*from = next;

		/*
		 *	If ALL of the 'to' attributes have been deleted,
		 *	then ensure that the 'tail' is updated to point
		 *	to the head.
		 */
		if (!*to) {
			tailto = to;
		}
		*tailto = i;
		if (i) {
			tailto = &i->next;
			i->next = NULL;
			(void) talloc_steal(ctx, i);
		}
	}
}

/** Move matching pairs between VALUE_PAIR lists
 *
 * Move pairs of a matching attribute number, vendor number and tag from the
 * the input list to the output list.
 *
 * @note pairfree should be called on the head of the old list to free unmoved
 	 attributes (if they're no longer needed).
 *
 * @param[in] ctx for talloc
 * @param[in,out] to destination list.
 * @param[in,out] from source list.
 * @param[in] attr to match, if PW_VENDOR_SPECIFIC and vendor 0, only VSAs will
 *	      be copied.  If 0 and 0, all attributes will match
 * @param[in] vendor to match.
 * @param[in] tag to match, TAG_ANY matches any tag, TAG_UNUSED matches tagless VPs.
 */
void pairfilter(TALLOC_CTX *ctx, VALUE_PAIR **to, VALUE_PAIR **from, unsigned int attr, unsigned int vendor, int8_t tag)
{
	VALUE_PAIR *to_tail, *i, *next;
	VALUE_PAIR *iprev = NULL;

	/*
	 *	Find the last pair in the "to" list and put it in "to_tail".
	 *
	 *	@todo: replace the "if" with "VALUE_PAIR **tail"
	 */
	if (*to != NULL) {
		to_tail = *to;
		for(i = *to; i; i = i->next) {
			VERIFY_VP(i);
			to_tail = i;
		}
	} else
		to_tail = NULL;

	/*
	 *	Attr/vendor of 0 means "move them all".
	 *	It's better than "pairadd(foo,bar);bar=NULL"
	 */
	if ((vendor == 0) && (attr == 0)) {
		if (*to) {
			to_tail->next = *from;
		} else {
			*to = *from;
		}

		for (i = *from; i; i = i->next) {
			(void) talloc_steal(ctx, i);
		}

		*from = NULL;
		return;
	}

	for(i = *from; i; i = next) {
		VERIFY_VP(i);
		next = i->next;

		if ((tag != TAG_ANY) && i->da->flags.has_tag &&
		    (i->tag != tag)) {
			continue;
		}

		/*
		 *	vendor=0, attr = PW_VENDOR_SPECIFIC means
		 *	"match any vendor attribute".
		 */
		if ((vendor == 0) && (attr == PW_VENDOR_SPECIFIC)) {
			/*
			 *	It's a VSA: move it over.
			 */
			if (i->da->vendor != 0) goto move;

			/*
			 *	It's Vendor-Specific: move it over.
			 */
			if (i->da->attr == attr) goto move;

			/*
			 *	It's not a VSA: ignore it.
			 */
			iprev = i;
			continue;
		}

		/*
		 *	If it isn't an exact match, ignore it.
		 */
		if (!((i->da->vendor == vendor) && (i->da->attr == attr))) {
			iprev = i;
			continue;
		}

	move:
		/*
		 *	Remove the attribute from the "from" list.
		 */
		if (iprev)
			iprev->next = next;
		else
			*from = next;

		/*
		 *	Add the attribute to the "to" list.
		 */
		if (to_tail)
			to_tail->next = i;
		else
			*to = i;
		to_tail = i;
		i->next = NULL;
		(void) talloc_steal(ctx, i);
	}
}


/*
 *	Sort of strtok/strsep function.
 */
static char *mystrtok(char **ptr, char const *sep)
{
	char	*res;

	if (**ptr == 0)
		return NULL;
	while (**ptr && strchr(sep, **ptr))
		(*ptr)++;
	if (**ptr == 0)
		return NULL;
	res = *ptr;
	while (**ptr && strchr(sep, **ptr) == NULL)
		(*ptr)++;
	if (**ptr != 0)
		*(*ptr)++ = 0;
	return res;
}

/*
 *	Turn printable string into time_t
 *	Returns -1 on error, 0 on OK.
 */
static int gettime(char const *valstr, time_t *date)
{
	int		i;
	time_t		t;
	struct tm	*tm, s_tm;
	char		buf[64];
	char		*p;
	char		*f[4];
	char		*tail = '\0';

	/*
	 * Test for unix timestamp date
	 */
	*date = strtoul(valstr, &tail, 10);
	if (*tail == '\0') {
		return 0;
	}

	tm = &s_tm;
	memset(tm, 0, sizeof(*tm));
	tm->tm_isdst = -1;	/* don't know, and don't care about DST */

	strlcpy(buf, valstr, sizeof(buf));

	p = buf;
	f[0] = mystrtok(&p, " \t");
	f[1] = mystrtok(&p, " \t");
	f[2] = mystrtok(&p, " \t");
	f[3] = mystrtok(&p, " \t"); /* may, or may not, be present */
	if (!f[0] || !f[1] || !f[2]) return -1;

	/*
	 *	The time has a colon, where nothing else does.
	 *	So if we find it, bubble it to the back of the list.
	 */
	if (f[3]) {
		for (i = 0; i < 3; i++) {
			if (strchr(f[i], ':')) {
				p = f[3];
				f[3] = f[i];
				f[i] = p;
				break;
			}
		}
	}

	/*
	 *  The month is text, which allows us to find it easily.
	 */
	tm->tm_mon = 12;
	for (i = 0; i < 3; i++) {
		if (isalpha( (int) *f[i])) {
			/*
			 *  Bubble the month to the front of the list
			 */
			p = f[0];
			f[0] = f[i];
			f[i] = p;

			for (i = 0; i < 12; i++) {
				if (strncasecmp(months[i], f[0], 3) == 0) {
					tm->tm_mon = i;
					break;
				}
			}
		}
	}

	/* month not found? */
	if (tm->tm_mon == 12) return -1;

	/*
	 *  The year may be in f[1], or in f[2]
	 */
	tm->tm_year = atoi(f[1]);
	tm->tm_mday = atoi(f[2]);

	if (tm->tm_year >= 1900) {
		tm->tm_year -= 1900;

	} else {
		/*
		 *  We can't use 2-digit years any more, they make it
		 *  impossible to tell what's the day, and what's the year.
		 */
		if (tm->tm_mday < 1900) return -1;

		/*
		 *  Swap the year and the day.
		 */
		i = tm->tm_year;
		tm->tm_year = tm->tm_mday - 1900;
		tm->tm_mday = i;
	}

	/*
	 *  If the day is out of range, die.
	 */
	if ((tm->tm_mday < 1) || (tm->tm_mday > 31)) {
		return -1;
	}

	/*
	 *	There may be %H:%M:%S.  Parse it in a hacky way.
	 */
	if (f[3]) {
		f[0] = f[3];	/* HH */
		f[1] = strchr(f[0], ':'); /* find : separator */
		if (!f[1]) return -1;

		*(f[1]++) = '\0'; /* nuke it, and point to MM:SS */

		f[2] = strchr(f[1], ':'); /* find : separator */
		if (f[2]) {
		  *(f[2]++) = '\0';	/* nuke it, and point to SS */
		  tm->tm_sec = atoi(f[2]);
		}			/* else leave it as zero */

		tm->tm_hour = atoi(f[0]);
		tm->tm_min = atoi(f[1]);
	}

	/*
	 *  Returns -1 on error.
	 */
	t = mktime(tm);
	if (t == (time_t) -1) return -1;

	*date = t;

	return 0;
}

static char const *hextab = "0123456789abcdef";

/*
 *  Parse a string value into a given VALUE_PAIR
 *
 *  FIXME: we probably want to fix this function to accept
 *  octets as values for any type of attribute.  We should then
 *  double-check the parsed value, to be sure it's legal for that
 *  type (length, etc.)
 */
static uint32_t getint(char const *value, char **end)
{
	if ((value[0] == '0') && (value[1] == 'x')) {
		return strtoul(value, end, 16);
	}

	return strtoul(value, end, 10);
}

static int check_for_whitespace(char const *value)
{
	while (*value) {
		if (!isspace((int) *value)) return 0;

		value++;
	}

	return 1;
}


bool pairparsevalue(VALUE_PAIR *vp, char const *value)
{
	char		*p;
	char const	*cp, *cs;
	int		x;
	uint64_t	y;
	size_t		length;
	DICT_VALUE	*dval;

	if (!value) return false;
	VERIFY_VP(vp);

	/*
	 *	It's a comparison, not a real VALUE_PAIR, copy the string over verbatim
	 */
	if ((vp->op == T_OP_REG_EQ) || (vp->op == T_OP_REG_NE)) {
		pairstrcpy(vp, value);	/* Icky hacky ewww */
		goto finish;
	}

	switch(vp->da->type) {
	case PW_TYPE_STRING:
		/*
		 *	Do escaping here
		 */
		p = talloc_strdup(vp, value);
		vp->vp_strvalue = p;
		cp = value;
		length = 0;

		while (*cp) {
			char c = *cp++;

			if (c == '\\') {
				switch (*cp) {
				case 'r':
					c = '\r';
					cp++;
					break;
				case 'n':
					c = '\n';
					cp++;
					break;
				case 't':
					c = '\t';
					cp++;
					break;
				case '"':
					c = '"';
					cp++;
					break;
				case '\'':
					c = '\'';
					cp++;
					break;
				case '\\':
					c = '\\';
					cp++;
					break;
				case '`':
					c = '`';
					cp++;
					break;
				case '\0':
					c = '\\'; /* no cp++ */
					break;
				default:
					if ((cp[0] >= '0') &&
					    (cp[0] <= '9') &&
					    (cp[1] >= '0') &&
					    (cp[1] <= '9') &&
					    (cp[2] >= '0') &&
					    (cp[2] <= '9') &&
					    (sscanf(cp, "%3o", &x) == 1)) {
						c = x;
						cp += 3;
					} /* else just do '\\' */
				}
			}
			*p++ = c;
			length++;
		}
		*p = '\0';
		vp->length = length;
		break;

	case PW_TYPE_IPADDR:
		/*
		 *	FIXME: complain if hostname
		 *	cannot be resolved, or resolve later!
		 */
		p = NULL;
		cs = value;

		{
			fr_ipaddr_t ipaddr;

			if (ip_hton(cs, AF_INET, &ipaddr) < 0) {
				fr_strerror_printf("Failed to find IP address for %s", cs);
				return false;
			}

			vp->vp_ipaddr = ipaddr.ipaddr.ip4addr.s_addr;
		}
		vp->length = 4;
		break;

	case PW_TYPE_BYTE:
		vp->length = 1;

		/*
		 *	Note that ALL integers are unsigned!
		 */
		vp->vp_integer = getint(value, &p);
		if (!*p) {
			if (vp->vp_integer > 255) {
				fr_strerror_printf("Byte value \"%s\" is larger than 255", value);
				return false;
			}
			break;
		}
		if (check_for_whitespace(p)) break;
		goto check_for_value;

	case PW_TYPE_SHORT:
		/*
		 *	Note that ALL integers are unsigned!
		 */
		vp->vp_integer = getint(value, &p);
		vp->length = 2;
		if (!*p) {
			if (vp->vp_integer > 65535) {
				fr_strerror_printf("Byte value \"%s\" is larger than 65535", value);
				return false;
			}
			break;
		}
		if (check_for_whitespace(p)) break;
		goto check_for_value;

	case PW_TYPE_INTEGER:
		/*
		 *	Note that ALL integers are unsigned!
		 */
		vp->vp_integer = getint(value, &p);
		vp->length = 4;
		if (!*p) break;
		if (check_for_whitespace(p)) break;

	check_for_value:
		/*
		 *	Look for the named value for the given
		 *	attribute.
		 */
		if ((dval = dict_valbyname(vp->da->attr, vp->da->vendor, value)) == NULL) {
			fr_strerror_printf("Unknown value %s for attribute %s",
				   value, vp->da->name);
			return false;
		}
		vp->vp_integer = dval->value;
		break;

	case PW_TYPE_INTEGER64:
		/*
		 *	Note that ALL integers are unsigned!
		 */
		if (sscanf(value, "%" PRIu64, &y) != 1) {
			fr_strerror_printf("Invalid value %s for attribute %s",
					   value, vp->da->name);
			return false;
		}
		vp->vp_integer64 = y;
		vp->length = 8;
		length = strspn(value, "0123456789");
		if (check_for_whitespace(value + length)) break;
		break;

	case PW_TYPE_DATE:
		{
			/*
			 *	time_t may be 64 bits, whule vp_date
			 *	MUST be 32-bits.  We need an
			 *	intermediary variable to handle
			 *	the conversions.
			 */
			time_t date;

			if (gettime(value, &date) < 0) {
				fr_strerror_printf("failed to parse time string "
					   "\"%s\"", value);
				return false;
			}

			vp->vp_date = date;
		}
		vp->length = 4;
		break;

	case PW_TYPE_ABINARY:
#ifdef WITH_ASCEND_BINARY
		if (strncasecmp(value, "0x", 2) == 0) {
			goto do_octets;
		}

		if (ascend_parse_filter(vp) < 0 ) {
			char buffer[256];

			snprintf(buffer, sizeof(buffer), "failed to parse Ascend binary attribute: %s", fr_strerror());
			fr_strerror_printf("%s", buffer);
			return false;
		}
		break;

		/*
		 *	If Ascend binary is NOT defined,
		 *	then fall through to raw octets, so that
		 *	the user can at least make them by hand...
		 */
#endif
	/* raw octets: 0x01020304... */
	case PW_TYPE_VSA:
		if (strcmp(value, "ANY") == 0) {
			vp->length = 0;
			break;
		} /* else it's hex */

	case PW_TYPE_OCTETS:
		if (strncasecmp(value, "0x", 2) == 0) {
			size_t size;
			uint8_t *us;

#ifdef WITH_ASCEND_BINARY
		do_octets:
#endif
			cp = value + 2;
			size = strlen(cp);
			vp->length = size >> 1;
			us = talloc_array(vp, uint8_t, vp->length);

			/*
			 *	Invalid.
			 */
			if ((size  & 0x01) != 0) {
				fr_strerror_printf("Hex string is not an even length string");
				return false;
			}

			if (fr_hex2bin(us, cp, vp->length) != vp->length) {
				fr_strerror_printf("Invalid hex data");
				return false;
			}
			vp->vp_octets = us;
		} else {
			pairstrcpy(vp, value);
		}
		break;

	case PW_TYPE_IFID:
		if (ifid_aton(value, (void *) &vp->vp_ifid) == NULL) {
			fr_strerror_printf("failed to parse interface-id "
				   "string \"%s\"", value);
			return false;
		}
		vp->length = 8;
		break;

	case PW_TYPE_IPV6ADDR:
		{
			fr_ipaddr_t ipaddr;

			if (ip_hton(value, AF_INET6, &ipaddr) < 0) {
				char buffer[1024];

				strlcpy(buffer, fr_strerror(), sizeof(buffer));

				fr_strerror_printf("failed to parse IPv6 address "
						   "string \"%s\": %s", value, buffer);
				return false;
			}
			vp->vp_ipv6addr = ipaddr.ipaddr.ip6addr;
			vp->length = 16; /* length of IPv6 address */
		}
		break;

	case PW_TYPE_IPV6PREFIX:
		p = strchr(value, '/');
		if (!p || ((p - value) >= 256)) {
			fr_strerror_printf("invalid IPv6 prefix "
				   "string \"%s\"", value);
			return false;
		} else {
			unsigned int prefix;
			char buffer[256], *eptr;

			memcpy(buffer, value, p - value);
			buffer[p - value] = '\0';

			if (inet_pton(AF_INET6, buffer, vp->vp_ipv6prefix + 2) <= 0) {
				fr_strerror_printf("failed to parse IPv6 address "
					   "string \"%s\"", value);
				return false;
			}

			prefix = strtoul(p + 1, &eptr, 10);
			if ((prefix > 128) || *eptr) {
				fr_strerror_printf("failed to parse IPv6 address "
					   "string \"%s\"", value);
				return false;
			}
			vp->vp_ipv6prefix[1] = prefix;
		}
		vp->length = 16 + 2;
		break;

	case PW_TYPE_IPV4PREFIX:
		p = strchr(value, '/');

		/*
		 *	192.0.2.2 is parsed as if it was /32
		 */
		if (!p) {
			vp->vp_ipv4prefix[1] = 32;

			if (inet_pton(AF_INET, value, vp->vp_ipv4prefix + 2) <= 0) {
				fr_strerror_printf("failed to parse IPv4 address "
					   "string \"%s\"", value);
				return false;
			}
			vp->length = sizeof(vp->vp_ipv4prefix);
			break;
		}

		/*
		 *	Otherwise parse the prefix
		 */
		if ((p - value) >= 256) {
			fr_strerror_printf("invalid IPv4 prefix "
				   "string \"%s\"", value);
			return false;
		} else {
			unsigned int prefix;
			char buffer[256], *eptr;

			memcpy(buffer, value, p - value);
			buffer[p - value] = '\0';

			if (inet_pton(AF_INET, buffer, vp->vp_ipv4prefix + 2) <= 0) {
				fr_strerror_printf("failed to parse IPv4 address "
					   "string \"%s\"", value);
				return false;
			}

			prefix = strtoul(p + 1, &eptr, 10);
			if ((prefix > 32) || *eptr) {
				fr_strerror_printf("failed to parse IPv4 address "
					   "string \"%s\"", value);
				return false;
			}
			vp->vp_ipv4prefix[1] = prefix;

			if (prefix < 32) {
				uint32_t addr, mask;

				memcpy(&addr, vp->vp_ipv4prefix + 2, sizeof(addr));
				mask = 1;
				mask <<= (32 - prefix);
				mask--;
				mask = ~mask;
				mask = htonl(mask);
				addr &= mask;
				memcpy(vp->vp_ipv4prefix + 2, &addr, sizeof(addr));
			}
		}
		vp->length = sizeof(vp->vp_ipv4prefix);
		break;

	case PW_TYPE_ETHERNET:
		{
			char const *c1, *c2;

			length = 0;
			cp = value;
			while (*cp) {
				if (cp[1] == ':') {
					c1 = hextab;
					c2 = memchr(hextab, tolower((int) cp[0]), 16);
					cp += 2;
				} else if ((cp[1] != '\0') &&
					   ((cp[2] == ':') ||
					    (cp[2] == '\0'))) {
					   c1 = memchr(hextab, tolower((int) cp[0]), 16);
					   c2 = memchr(hextab, tolower((int) cp[1]), 16);
					   cp += 2;
					   if (*cp == ':') cp++;
				} else {
					c1 = c2 = NULL;
				}
				if (!c1 || !c2 || (length >= sizeof(vp->vp_ether))) {
					fr_strerror_printf("failed to parse Ethernet address \"%s\"", value);
					return false;
				}
				vp->vp_ether[length] = ((c1-hextab)<<4) + (c2-hextab);
				length++;
			}
		}
		vp->length = 6;
		break;

	/*
	 *	Crazy polymorphic (IPv4/IPv6) attribute type for WiMAX.
	 *
	 *	We try and make is saner by replacing the original
	 *	da, with either an IPv4 or IPv6 da type.
	 *
	 *	These are not dynamic da, and will have the same vendor
	 *	and attribute as the original.
	 */
	case PW_TYPE_COMBO_IP:
		{
			DICT_ATTR const *da;

			if (inet_pton(AF_INET6, value, &vp->vp_ipv6addr) > 0) {
				da = dict_attrbytype(vp->da->attr, vp->da->vendor,
						     PW_TYPE_IPV6ADDR);
				if (!da) {
					return false;
				}

				vp->length = 16; /* length of IPv6 address */
			} else {
				fr_ipaddr_t ipaddr;

				da = dict_attrbytype(vp->da->attr, vp->da->vendor,
						     PW_TYPE_IPADDR);
				if (!da) {
					return false;
				}

				if (ip_hton(value, AF_INET, &ipaddr) < 0) {
					fr_strerror_printf("Failed to find IPv4 address for %s", value);
					return false;
				}

				vp->vp_ipaddr = ipaddr.ipaddr.ip4addr.s_addr;
				vp->length = 4;
			}

			vp->da = da;
		}
		break;

	case PW_TYPE_SIGNED: /* Damned code for 1 WiMAX attribute */
		vp->vp_signed = (int32_t) strtol(value, &p, 10);
		vp->length = 4;
		break;

	case PW_TYPE_TLV: /* don't use this! */
		if (strncasecmp(value, "0x", 2) != 0) {
			fr_strerror_printf("Invalid TLV specification");
			return false;
		}
		length = strlen(value + 2) / 2;
		if (vp->length < length) {
			TALLOC_FREE(vp->vp_tlv);
		}
		vp->vp_tlv = talloc_array(vp, uint8_t, length);
		if (!vp->vp_tlv) {
			fr_strerror_printf("No memory");
			return false;
		}
		if (fr_hex2bin(vp->vp_tlv, value + 2, length) != length) {
			fr_strerror_printf("Invalid hex data in TLV");
			return false;
		}
		vp->length = length;
		break;

		/*
		 *  Anything else.
		 */
	default:
		fr_strerror_printf("unknown attribute type %d", vp->da->type);
		return false;
	}

	finish:
	vp->type = VT_DATA;
	return true;
}

/** Create a valuepair from an ASCII attribute and value
 *
 * Where the attribute name is in the form:
 *  - Attr-%d
 *  - Attr-%d.%d.%d...
 *  - Vendor-%d-Attr-%d
 *  - VendorName-Attr-%d
 *
 * @param ctx for talloc
 * @param attribute name to parse.
 * @param value to parse (must be a hex string).
 * @param op to assign to new valuepair.
 * @return new valuepair or NULL on error.
 */
static VALUE_PAIR *pairmake_any(TALLOC_CTX *ctx,
				char const *attribute, char const *value,
				FR_TOKEN op)
{
	VALUE_PAIR	*vp;
	DICT_ATTR const *da;

	uint8_t 	*data;
	size_t		size;

	da = dict_attrunknownbyname(attribute, true);
	if (!da) return NULL;

	/*
	 *	Unknown attributes MUST be of type 'octets'
	 */
	if (value && (strncasecmp(value, "0x", 2) != 0)) {
		fr_strerror_printf("Unknown attribute \"%s\" requires a hex "
				   "string, not \"%s\"", attribute, value);

		dict_attr_free(&da);
		return NULL;
	}

	/*
	 *	We've now parsed the attribute properly, Let's create
	 *	it.  This next stop also looks the attribute up in the
	 *	dictionary, and creates the appropriate type for it.
	 */
	vp = pairalloc(ctx, da);
	if (!vp) {
		dict_attr_free(&da);
		return NULL;
	}

	vp->op = (op == 0) ? T_OP_EQ : op;

	if (!value) return vp;

	size = strlen(value + 2);
	vp->length = size >> 1;
	data = talloc_array(vp, uint8_t, vp->length);

	if (fr_hex2bin(data, value + 2, size) != vp->length) {
		fr_strerror_printf("Invalid hex string");
		talloc_free(vp);
		return NULL;
	}

	vp->vp_octets = data;
	vp->type = VT_DATA;
	return vp;
}


/** Create a VALUE_PAIR from ASCII strings
 *
 * Converts an attribute string identifier (with an optional tag qualifier)
 * and value string into a VALUE_PAIR.
 *
 * The string value is parsed according to the type of VALUE_PAIR being created.
 *
 * @param[in] ctx for talloc
 * @param[in] vps list where the attribute will be added (optional)
 * @param[in] attribute name.
 * @param[in] value attribute value.
 * @param[in] op to assign to new VALUE_PAIR.
 * @return a new VALUE_PAIR.
 */
VALUE_PAIR *pairmake(TALLOC_CTX *ctx, VALUE_PAIR **vps,
		     char const *attribute, char const *value, FR_TOKEN op)
{
	DICT_ATTR const *da;
	VALUE_PAIR	*vp;
	char		*tc, *ts;
	int8_t		tag;
	int		found_tag;
	char		buffer[256];
	char const	*attrname = attribute;

	/*
	 *    Check for tags in 'Attribute:Tag' format.
	 */
	found_tag = 0;
	tag = 0;

	ts = strrchr(attribute, ':');
	if (ts && !ts[1]) {
		fr_strerror_printf("Invalid tag for attribute %s", attribute);
		return NULL;
	}

	if (ts && ts[1]) {
		strlcpy(buffer, attribute, sizeof(buffer));
		attrname = buffer;
		ts = strrchr(attrname, ':');
		if (!ts) return NULL;

		 /* Colon found with something behind it */
		 if (ts[1] == '*' && ts[2] == 0) {
			 /* Wildcard tag for check items */
			 tag = TAG_ANY;
			 *ts = 0;
		 } else if ((ts[1] >= '0') && (ts[1] <= '9')) {
			 /* It's not a wild card tag */
			 tag = strtol(ts + 1, &tc, 0);
			 if (tc && !*tc && TAG_VALID_ZERO(tag))
				 *ts = 0;
			 else tag = 0;
		 } else {
			 fr_strerror_printf("Invalid tag for attribute %s", attribute);
			 return NULL;
		 }
		 found_tag = 1;
	}

	/*
	 *	It's not found in the dictionary, so we use
	 *	another method to create the attribute.
	 */
	da = dict_attrbyname(attrname);
	if (!da) {
		vp = pairmake_any(ctx, attrname, value, op);
		if (vp) pairadd(vps, vp);
		return vp;
	}

	/*      Check for a tag in the 'Merit' format of:
	 *      :Tag:Value.  Print an error if we already found
	 *      a tag in the Attribute.
	 */

	if (value && (*value == ':' && da->flags.has_tag)) {
		/* If we already found a tag, this is invalid */
		if(found_tag) {
			fr_strerror_printf("Duplicate tag %s for attribute %s",
				   value, da->name);
			DEBUG("Duplicate tag %s for attribute %s\n",
				   value, da->name);
			return NULL;
		}
		/* Colon found and attribute allows a tag */
		if (value[1] == '*' && value[2] == ':') {
		       /* Wildcard tag for check items */
		       tag = TAG_ANY;
		       value += 3;
		} else {
		       /* Real tag */
		       tag = strtol(value + 1, &tc, 0);
		       if (tc && *tc==':' && TAG_VALID_ZERO(tag))
			    value = tc + 1;
		       else tag = 0;
		}
	}

	vp = pairalloc(ctx, da);
	if (!vp) {
		return NULL;
	}

	vp->op = (op == 0) ? T_OP_EQ : op;
	vp->tag = tag;

	switch (vp->op) {
	default:
		break;

	case T_OP_CMP_TRUE:
	case T_OP_CMP_FALSE:
		vp->vp_strvalue = NULL;
		vp->length = 0;
		value = NULL;	/* ignore it! */
		break;

		/*
		 *	Regular expression comparison of integer attributes
		 *	does a STRING comparison of the names of their
		 *	integer attributes.
		 */
	case T_OP_REG_EQ:	/* =~ */
	case T_OP_REG_NE:	/* !~ */
#ifndef WITH_REGEX
		fr_strerror_printf("Regular expressions are not supported");
		return NULL;

#else

		/*
		 *	Someone else will fill in the value.
		 */
		if (!value) break;

		talloc_free(vp);

		if (1) {
			int compare;
			regex_t reg;

			compare = regcomp(&reg, value, REG_EXTENDED);
			if (compare != 0) {
				regerror(compare, &reg, buffer, sizeof(buffer));
				fr_strerror_printf("Illegal regular expression in attribute: %s: %s",
					   attribute, buffer);
				return NULL;
			}
		}

		vp = pairmake(ctx, NULL, attribute, NULL, op);
		if (!vp) return NULL;

		if (pairmark_xlat(vp, value) < 0) {
			talloc_free(vp);
			return NULL;
		}

		value = NULL;	/* ignore it */
		break;
#endif
	}

	/*
	 *	FIXME: if (strcasecmp(attribute, vp->da->name) != 0)
	 *	then the user MAY have typed in the attribute name
	 *	as Vendor-%d-Attr-%d, and the value MAY be octets.
	 *
	 *	We probably want to fix pairparsevalue to accept
	 *	octets as values for any attribute.
	 */
	if (value && !pairparsevalue(vp, value)) {
		talloc_free(vp);
		return NULL;
	}

	if (vps) pairadd(vps, vp);
	return vp;
}

/** Mark a valuepair for xlat expansion
 *
 * Copies xlat source (unprocessed) string to valuepair value,
 * and sets value type.
 *
 * @param vp to mark for expansion.
 * @param value to expand.
 * @return 0 if marking succeeded or -1 if vp already had a value, or OOM.
 */
int pairmark_xlat(VALUE_PAIR *vp, char const *value)
{
	char *raw;

	/*
	 *	valuepair should not already have a value.
	 */
	if (vp->type != VT_NONE) {
		return -1;
	}

	raw = talloc_strdup(vp, value);
	if (!raw) {
		return -1;
	}

	vp->type = VT_XLAT;
	vp->value.xlat = raw;
	vp->length = 0;

	return 0;
}

/** Read a single valuepair from a buffer, and advance the pointer
 *
 * Sets *eol to T_EOL if end of line was encountered.
 *
 * @param[in,out] ptr to read from and update.
 * @param[out] raw The struct to write the raw VALUE_PAIR to.
 * @return the last token read.
 */
FR_TOKEN pairread(char const **ptr, VALUE_PAIR_RAW *raw)
{
	char const	*p;
	char *q;
	FR_TOKEN	ret = T_OP_INVALID, next, quote;
	char		buf[8];

	if (!ptr || !*ptr || !raw) {
		fr_strerror_printf("Invalid arguments");
		return T_OP_INVALID;
	}

	/*
	 *	Skip leading spaces
	 */
	p = *ptr;
	while ((*p == ' ') || (*p == '\t')) p++;

	if (!*p) {
		fr_strerror_printf("No token read where we expected "
				   "an attribute name");
		return T_OP_INVALID;
	}

	if (*p == '#') {
		fr_strerror_printf("Read a comment instead of a token");

		return T_HASH;
	}

	/*
	 *	Try to get the attribute name.
	 */
	q = raw->l_opand;
	*q = '\0';
	while (*p) {
		uint8_t const *t = (uint8_t const *) p;

		if (q >= (raw->l_opand + sizeof(raw->l_opand))) {
		too_long:
			fr_strerror_printf("Attribute name too long");
			return T_OP_INVALID;
		}

		/*
		 *	Only ASCII is allowed, and only a subset of that.
		 */
		if ((*t < 32) || (*t >= 128)) {
		invalid:
			fr_strerror_printf("Invalid attribute name");
			return T_OP_INVALID;
		}

		/*
		 *	This is arguably easier than trying to figure
		 *	out which operators come after the attribute
		 *	name.  Yes, our "lexer" is bad.
		 */
		if (!dict_attr_allowed_chars[(int) *t]) {
			break;
		}

		*(q++) = *(p++);
	}

	/*
	 *	ASCII, but not a valid attribute name.
	 */
	if (!*raw->l_opand) goto invalid;

	/*
	 *	Look for tag (:#).  This is different from :=, which
	 *	is an operator.
	 */
	if ((*p == ':') && (isdigit((int) p[1]))) {
		if (q >= (raw->l_opand + sizeof(raw->l_opand))) {
			goto too_long;
		}
		*(q++) = *(p++);

		while (isdigit((int) *p)) {
			if (q >= (raw->l_opand + sizeof(raw->l_opand))) {
				goto too_long;
			}
			*(q++) = *(p++);
		}
	}

	*q = '\0';
	*ptr = p;

	/* Now we should have an operator here. */
	raw->op = gettoken(ptr, buf, sizeof(buf));
	if (raw->op  < T_EQSTART || raw->op  > T_EQEND) {
		fr_strerror_printf("Expecting operator");

		return T_OP_INVALID;
	}

	/*
	 *	Read value.  Note that empty string values are allowed
	 */
	quote = gettoken(ptr, raw->r_opand, sizeof(raw->r_opand));
	if (quote == T_EOL) {
		fr_strerror_printf("Failed to get value");

		return T_OP_INVALID;
	}

	/*
	 *	Peek at the next token. Must be T_EOL, T_COMMA, or T_HASH
	 */
	p = *ptr;

	next = gettoken(&p, buf, sizeof(buf));
	switch (next) {
	case T_EOL:
	case T_HASH:
		break;

	case T_COMMA:
		*ptr = p;
		break;

	default:
		fr_strerror_printf("Expected end of line or comma");
		return T_OP_INVALID;
	}
	ret = next;

	switch (quote) {
	/*
	 *	Perhaps do xlat's
	 */
	case T_DOUBLE_QUOTED_STRING:
		/*
		 *	Only report as double quoted if it contained valid
		 *	a valid xlat expansion.
		 */
		p = strchr(raw->r_opand, '%');
		if (p && (p[1] == '{')) {
			raw->quote = quote;
		} else {
			raw->quote = T_SINGLE_QUOTED_STRING;
		}

		break;
	default:
		raw->quote = quote;

		break;
	}

	return ret;
}

/** Read one line of attribute/value pairs into a list.
 *
 * The line may specify multiple attributes separated by commas.
 *
 * @note If the function returns T_OP_INVALID, an error has occurred and
 * @note the valuepair list should probably be freed.
 *
 * @param ctx for talloc
 * @param buffer to read valuepairs from.
 * @param list where the parsed VALUE_PAIRs will be appended.
 * @return the last token parsed, or T_OP_INVALID
 */
FR_TOKEN userparse(TALLOC_CTX *ctx, char const *buffer, VALUE_PAIR **list)
{
	VALUE_PAIR	*vp, *head, **tail;
	char const	*p;
	FR_TOKEN	last_token = T_OP_INVALID;
	FR_TOKEN	previous_token;
	VALUE_PAIR_RAW	raw;

	/*
	 *	We allow an empty line.
	 */
	if (buffer[0] == 0)
		return T_EOL;

	head = NULL;
	tail = &head;

	p = buffer;
	do {
		raw.l_opand[0] = '\0';
		raw.r_opand[0] = '\0';

		previous_token = last_token;

		last_token = pairread(&p, &raw);
		if (last_token == T_OP_INVALID) break;

		if (raw.quote == T_DOUBLE_QUOTED_STRING) {
			vp = pairmake(ctx, NULL, raw.l_opand, NULL, raw.op);
			if (!vp) {
				last_token = T_OP_INVALID;
				break;
			}
			if (pairmark_xlat(vp, raw.r_opand) < 0) {
				talloc_free(vp);

				break;
			}
		} else {
			vp = pairmake(ctx, NULL, raw.l_opand, raw.r_opand, raw.op);
			if (!vp) {
				last_token = T_OP_INVALID;
				break;
			}
		}

		*tail = vp;
		tail = &((*tail)->next);
	} while (*p && (last_token == T_COMMA));

	/*
	 *	Don't tell the caller that there was a comment.
	 */
	if (last_token == T_HASH) {
		last_token = previous_token;
	}

	if (last_token == T_OP_INVALID) {
		pairfree(&head);
	} else {
		pairadd(list, head);
	}

	/*
	 *	And return the last token which we read.
	 */
	return last_token;
}

/*
 *	Read valuepairs from the fp up to End-Of-File.
 *
 *	Hmm... this function is only used by radclient..
 */
VALUE_PAIR *readvp2(TALLOC_CTX *ctx, FILE *fp, int *pfiledone, char const *errprefix)
{
	char buf[8192];
	FR_TOKEN last_token = T_EOL;
	VALUE_PAIR *vp;
	VALUE_PAIR *list;
	int error = 0;

	list = NULL;

	while (!error && fgets(buf, sizeof(buf), fp) != NULL) {
		/*
		 *      If we get a '\n' by itself, we assume that's
		 *      the end of that VP
		 */
		if ((buf[0] == '\n') && (list)) {
			return list;
		}
		if ((buf[0] == '\n') && (!list)) {
			continue;
		}

		/*
		 *	Comments get ignored
		 */
		if (buf[0] == '#') continue;

		/*
		 *	Read all of the attributes on the current line.
		 */
		vp = NULL;
		last_token = userparse(ctx, buf, &vp);
		if (!vp) {
			if (last_token != T_EOL) {
				fr_perror("%s", errprefix);
				error = 1;
				break;
			}
			break;
		}

		pairadd(&list, vp);
		buf[0] = '\0';
	}

	if (error) pairfree(&list);

	*pfiledone = 1;

	return error ? NULL: list;
}

/*
 *	We leverage the fact that IPv4 and IPv6 prefixes both
 *	have the same format:
 *
 *	reserved, prefix-len, data...
 */
static int paircmp_cidr(FR_TOKEN op, int bytes, uint8_t const *one, uint8_t const *two)
{
	int i, common;
	uint32_t mask;

	/*
	 *	Handle the case of netmasks being identical.
	 */
	if (one[1] == two[1]) {
		int compare;

		compare = memcmp(one + 2, two + 2, bytes);

		/*
		 *	If they're identical return true for
		 *	identical.
		 */
		if ((compare == 0) &&
		    ((op == T_OP_CMP_EQ) ||
		     (op == T_OP_LE) ||
		     (op == T_OP_GE))) {
			return true;
		}

		/*
		 *	Everything else returns false.
		 *
		 *	10/8 == 24/8  --> false
		 *	10/8 <= 24/8  --> false
		 *	10/8 >= 24/8  --> false
		 */
		return false;
	}

	/*
	 *	Netmasks are different.  That limits the
	 *	possible results, based on the operator.
	 */
	switch (op) {
	case T_OP_CMP_EQ:
		return false;

	case T_OP_NE:
		return true;

	case T_OP_LE:
	case T_OP_LT:	/* 192/8 < 192.168/16 --> false */
		if (one[1] < two[1]) {
			return false;
		}
		break;

	case T_OP_GE:
	case T_OP_GT:	/* 192/16 > 192.168/8 --> false */
		if (one[1] > two[1]) {
			return false;
		}
		break;

	default:
		return false;
	}

	if (one[1] < two[1]) {
		common = one[1];
	} else {
		common = two[1];
	}

	/*
	 *	Do the check byte by byte.  If the bytes are
	 *	identical, it MAY be a match.  If they're different,
	 *	it is NOT a match.
	 */
	i = 2;
	while (i < (2 + bytes)) {
		/*
		 *	All leading bytes are identical.
		 */
		if (common == 0) return true;

		/*
		 *	Doing bitmasks takes more work.
		 */
		if (common < 8) break;

		if (one[i] != two[i]) return false;

		common -= 8;
		i++;
		continue;
	}

	mask = 1;
	mask <<= (8 - common);
	mask--;
	mask = ~mask;

	if ((one[i] & mask) == ((two[i] & mask))) {
		return true;
	}

	return false;
}

/*
 *	Compare two pairs, using the operator from "one".
 *
 *	i.e. given two attributes, it does:
 *
 *	(two->data) (one->operator) (one->data)
 *
 *	e.g. "foo" != "bar"
 *
 *	Returns true (comparison is true), or false (comparison is not true);
 */
int paircmp(VALUE_PAIR *one, VALUE_PAIR *two)
{
	int compare;

	VERIFY_VP(one);
	VERIFY_VP(two);

	switch (one->op) {
	case T_OP_CMP_TRUE:
		return (two != NULL);

	case T_OP_CMP_FALSE:
		return (two == NULL);

		/*
		 *	One is a regex, compile it, print two to a string,
		 *	and then do string comparisons.
		 */
	case T_OP_REG_EQ:
	case T_OP_REG_NE:
#ifndef WITH_REGEX
		return -1;
#else
		{
			regex_t reg;
			char buffer[MAX_STRING_LEN * 4 + 1];

			compare = regcomp(&reg, one->vp_strvalue, REG_EXTENDED);
			if (compare != 0) {
				regerror(compare, &reg, buffer, sizeof(buffer));
				fr_strerror_printf("Illegal regular expression in attribute: %s: %s",
					   one->da->name, buffer);
				return -1;
			}

			vp_prints_value(buffer, sizeof(buffer), two, 0);

			/*
			 *	Don't care about substring matches,
			 *	oh well...
			 */
			compare = regexec(&reg, buffer, 0, NULL, 0);

			regfree(&reg);
			if (one->op == T_OP_REG_EQ) return (compare == 0);
			return (compare != 0);
		}
#endif

	default:		/* we're OK */
		break;
	}

	return paircmp_op(two, one->op, one);
}

/* Compare two attributes
 *
 * @param[in] one the first attribute
 * @param[in] op the operator for comparison
 * @param[in] two the second attribute
 * @return true if ONE OP TWO is true, else false.
 */
int paircmp_op(VALUE_PAIR const *one, FR_TOKEN op, VALUE_PAIR const *two)
{
	int compare;

	VERIFY_VP(one);
	VERIFY_VP(two);

	/*
	 *	Can't compare two attributes of differing types
	 *
	 *	FIXME: maybe do checks for IP OP IP/mask ??
	 */
	if (one->da->type != two->da->type) {
		return one->da->type - two->da->type;
	}

	/*
	 *	After doing the previous check for special comparisons,
	 *	do the per-type comparison here.
	 */
	switch (one->da->type) {
	case PW_TYPE_ABINARY:
	case PW_TYPE_OCTETS:
	{
		size_t length;

		if (one->length > two->length) {
			length = one->length;
		} else {
			length = two->length;
		}

		if (length) {
			compare = memcmp(one->vp_octets, two->vp_octets,
					 length);
			if (compare != 0) break;
		}

		/*
		 *	Contents are the same.  The return code
		 *	is therefore the difference in lengths.
		 *
		 *	i.e. "0x00" is smaller than "0x0000"
		 */
		compare = one->length - two->length;
	}
		break;

	case PW_TYPE_STRING:
		compare = strcmp(one->vp_strvalue, two->vp_strvalue);
		break;

	case PW_TYPE_BYTE:
	case PW_TYPE_SHORT:
	case PW_TYPE_INTEGER:
	case PW_TYPE_DATE:
		if (one->vp_integer < two->vp_integer) {
			compare = -1;
		} else if (one->vp_integer == two->vp_integer) {
			compare = 0;
		} else {
			compare = +1;
		}
		break;

	case PW_TYPE_INTEGER64:
		/*
		 *	Don't want integer overflow!
		 */
		if (one->vp_integer64 < two->vp_integer64) {
			compare = -1;
		} else if (one->vp_integer64 > two->vp_integer64) {
			compare = +1;
		} else {
			compare = 0;
		}
		break;
	case PW_TYPE_IPADDR:
		if (ntohl(one->vp_ipaddr)  < ntohl(two->vp_ipaddr)) {
			compare = -1;
		} else if (one->vp_ipaddr  == two->vp_ipaddr) {
			compare = 0;
		} else {
			compare = +1;
		}
		break;

	case PW_TYPE_IPV6ADDR:
		compare = memcmp(&one->vp_ipv6addr, &two->vp_ipv6addr,
				 sizeof(one->vp_ipv6addr));
		break;

	case PW_TYPE_IPV6PREFIX:
		return paircmp_cidr(op, 16,
				    (uint8_t const *) &one->vp_ipv6prefix,
				    (uint8_t const *) &two->vp_ipv6prefix);

	case PW_TYPE_IPV4PREFIX:
		return paircmp_cidr(op, 4,
				    (uint8_t const *) &one->vp_ipv4prefix,
				    (uint8_t const *) &two->vp_ipv4prefix);

	case PW_TYPE_IFID:
		compare = memcmp(&one->vp_ifid, &two->vp_ifid,
				 sizeof(one->vp_ifid));
		break;

	default:
		return 0;	/* unknown type */
	}

	/*
	 *	Now do the operator comparison.
	 */
	switch (op) {
	case T_OP_CMP_EQ:
		return (compare == 0);

	case T_OP_NE:
		return (compare != 0);

	case T_OP_LT:
		return (compare < 0);

	case T_OP_GT:
		return (compare > 0);

	case T_OP_LE:
		return (compare <= 0);

	case T_OP_GE:
		return (compare >= 0);

	default:
		return 0;
	}

	return 0;
}

/** Copy data into an "octets" data type.
 *
 * @param[in,out] vp to update
 * @param[in] src data to copy
 * @param[in] size of the data
 */
void pairmemcpy(VALUE_PAIR *vp, uint8_t const *src, size_t size)
{
	uint8_t *p, *q;

	VERIFY_VP(vp);

	p = talloc_memdup(vp, src, size);
	if (!p) return;

	memcpy(&q, &vp->vp_octets, sizeof(q));
	talloc_free(q);

	vp->vp_octets = p;
	vp->length = size;
}

/** Reparent an allocated octet buffer to a VALUE_PAIR
 *
 * @param[in,out] vp to update
 * @param[in] src buffer to steal.
 */
void pairmemsteal(VALUE_PAIR *vp, uint8_t *src)
{
	uint8_t *q;
	VERIFY_VP(vp);

	memcpy(&q, &vp->vp_octets, sizeof(q));
	talloc_free(q);

	vp->vp_octets = talloc_steal(vp, src);
	vp->type = VT_DATA;
	vp->length = talloc_array_length(vp->vp_strvalue);
}

/** Reparent an allocated char buffer to a VALUE_PAIR
 *
 * @param[in,out] vp to update
 * @param[in] src buffer to steal.
 */
void pairstrsteal(VALUE_PAIR *vp, char *src)
{
	uint8_t *q;
	VERIFY_VP(vp);

	memcpy(&q, &vp->vp_octets, sizeof(q));
	talloc_free(q);

	vp->vp_strvalue = talloc_steal(vp, src);
	vp->type = VT_DATA;
	vp->length = talloc_array_length(vp->vp_strvalue) - 1;
}

/** Copy data into an "string" data type.
 *
 * @param[in,out] vp to update
 * @param[in] src data to copy
 */
void pairstrcpy(VALUE_PAIR *vp, char const *src)
{
	char *p, *q;

	VERIFY_VP(vp);

	p = talloc_strdup(vp, src);
	if (!p) return;

	memcpy(&q, &vp->vp_strvalue, sizeof(q));
	talloc_free(q);

	vp->vp_strvalue = p;
	vp->type = VT_DATA;
	vp->length = strlen(vp->vp_strvalue);
}


/** Print data into an "string" data type.
 *
 * @param[in,out] vp to update
 * @param[in] fmt the format string
 */
void pairsprintf(VALUE_PAIR *vp, char const *fmt, ...)
{
	va_list ap;
	char *p, *q;

	VERIFY_VP(vp);

	va_start(ap, fmt);
	p = talloc_vasprintf(vp, fmt, ap);
	va_end(ap);

	if (!p) return;

	memcpy(&q, &vp->vp_strvalue, sizeof(q));
	talloc_free(q);

	vp->vp_strvalue = p;
	vp->type = VT_DATA;

	/*
	 *	vsnprintf returns random things on different platforms
	 */
	vp->length = strlen(vp->vp_strvalue);
}

