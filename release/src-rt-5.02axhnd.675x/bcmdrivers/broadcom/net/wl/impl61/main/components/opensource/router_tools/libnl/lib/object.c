/*
 * lib/object.c		Generic Cacheable Object
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2012 Thomas Graf <tgraf@suug.ch>
 */

/**
 * @ingroup core_types
 * @defgroup object Object (Cacheable)
 *
 * Generic object data type, for inheritance purposes to implement cacheable
 * data types.
 *
 * Related sections in the development guide:
 *
 * @{
 *
 * Header
 * ------
 * ~~~~{.c}
 * #include <netlink/object.h>
 * ~~~~
 */

#include <netlink-private/netlink.h>
#include <netlink/netlink.h>
#include <netlink/cache.h>
#include <netlink/object.h>
#include <netlink/utils.h>

static inline struct nl_object_ops *obj_ops(struct nl_object *obj)
{
	if (!obj->ce_ops)
		BUG();

	return obj->ce_ops;
}

/**
 * @name Object Creation/Deletion
 * @{
 */

/**
 * Allocate a new object of kind specified by the operations handle
 * @arg ops		cache operations handle
 * @return The new object or NULL
 */
struct nl_object *nl_object_alloc(struct nl_object_ops *ops)
{
	struct nl_object *new;

	if (ops->oo_size < sizeof(*new))
		BUG();

	new = calloc(1, ops->oo_size);
	if (!new)
		return NULL;

	new->ce_refcnt = 1;
	nl_init_list_head(&new->ce_list);

	new->ce_ops = ops;
	if (ops->oo_constructor)
		ops->oo_constructor(new);

	NL_DBG(4, "Allocated new object %p\n", new);

	return new;
}

/**
 * Allocate new object of kind specified by the name
 * @arg kind		name of object type
 * @arg result		Result pointer
 *
 * @return 0 on success or a negative error code.
 */
int nl_object_alloc_name(const char *kind, struct nl_object **result)
{
	struct nl_cache_ops *ops;

	ops = nl_cache_ops_lookup_safe(kind);
	if (!ops)
		return -NLE_OPNOTSUPP;

	*result = nl_object_alloc(ops->co_obj_ops);
	nl_cache_ops_put(ops);
	if (!*result)
		return -NLE_NOMEM;

	return 0;
}

struct nl_derived_object {
	NLHDR_COMMON
	char data;
};

/**
 * Allocate a new object and copy all data from an existing object
 * @arg obj		object to inherite data from
 * @return The new object or NULL.
 */
struct nl_object *nl_object_clone(struct nl_object *obj)
{
	struct nl_object *new;
	struct nl_object_ops *ops;
	int doff = offsetof(struct nl_derived_object, data);
	int size;

	if (!obj)
		return NULL;

	ops = obj_ops(obj);
	new = nl_object_alloc(ops);
	if (!new)
		return NULL;

	size = ops->oo_size - doff;
	if (size < 0)
		BUG();

	new->ce_ops = obj->ce_ops;
	new->ce_msgtype = obj->ce_msgtype;
	new->ce_mask = obj->ce_mask;

	if (size)
		memcpy((void *)new + doff, (void *)obj + doff, size);

	if (ops->oo_clone) {
		if (ops->oo_clone(new, obj) < 0) {
			nl_object_free(new);
			return NULL;
		}
	} else if (size && ops->oo_free_data)
		BUG();

	return new;
}

/**
 * Merge a cacheable object
 * @arg dst		object to be merged into
 * @arg src		new object to be merged into dst
 *
 * @return 0 or a negative error code.
 */
int nl_object_update(struct nl_object *dst, struct nl_object *src)
{
	struct nl_object_ops *ops = obj_ops(dst);

	if (ops->oo_update)
		return ops->oo_update(dst, src);

	return -NLE_OPNOTSUPP;
}

/**
 * Free a cacheable object
 * @arg obj		object to free
 *
 * @return 0 or a negative error code.
 */
void nl_object_free(struct nl_object *obj)
{
	struct nl_object_ops *ops;

	if (!obj)
		return;

	ops = obj_ops(obj);

	if (obj->ce_refcnt > 0)
		NL_DBG(1, "Warning: Freeing object in use...\n");

	if (obj->ce_cache)
		nl_cache_remove(obj);

	if (ops->oo_free_data)
		ops->oo_free_data(obj);

	NL_DBG(4, "Freed object %p\n", obj);

	free(obj);
}

/** @} */

/**
 * @name Reference Management
 * @{
 */

/**
 * Acquire a reference on a object
 * @arg obj		object to acquire reference from
 */
void nl_object_get(struct nl_object *obj)
{
	obj->ce_refcnt++;
	NL_DBG(4, "New reference to object %p, total %d\n",
	       obj, obj->ce_refcnt);
}

/**
 * Release a reference from an object
 * @arg obj		object to release reference from
 */
void nl_object_put(struct nl_object *obj)
{
	if (!obj)
		return;

	obj->ce_refcnt--;
	NL_DBG(4, "Returned object reference %p, %d remaining\n",
	       obj, obj->ce_refcnt);

	if (obj->ce_refcnt < 0)
		BUG();

	if (obj->ce_refcnt <= 0)
		nl_object_free(obj);
}

/**
 * Check whether this object is used by multiple users
 * @arg obj		object to check
 * @return true or false
 */
int nl_object_shared(struct nl_object *obj)
{
	return obj->ce_refcnt > 1;
}

/** @} */

/**
 * @name Marks
 * @{
 */

/**
 * Add mark to object
 * @arg obj		Object to mark
 */
void nl_object_mark(struct nl_object *obj)
{
	obj->ce_flags |= NL_OBJ_MARK;
}

/**
 * Remove mark from object
 * @arg obj		Object to unmark
 */
void nl_object_unmark(struct nl_object *obj)
{
	obj->ce_flags &= ~NL_OBJ_MARK;
}

/**
 * Return true if object is marked
 * @arg obj		Object to check
 * @return true if object is marked, otherwise false
 */
int nl_object_is_marked(struct nl_object *obj)
{
	return (obj->ce_flags & NL_OBJ_MARK);
}

/** @} */

/**
 * @name Utillities
 * @{
 */

/**
 * Dump this object according to the specified parameters
 * @arg obj		object to dump
 * @arg params		dumping parameters
 */
void nl_object_dump(struct nl_object *obj, struct nl_dump_params *params)
{
	if (params->dp_buf)
		memset(params->dp_buf, 0, params->dp_buflen);

	dump_from_ops(obj, params);
}

void nl_object_dump_buf(struct nl_object *obj, char *buf, size_t len)
{
        struct nl_dump_params dp = {
                .dp_buf = buf,
                .dp_buflen = len,
        };

        return nl_object_dump(obj, &dp);
}

/**
 * Check if the identifiers of two objects are identical
 * @arg a		an object
 * @arg b		another object of same type
 *
 * @return true if both objects have equal identifiers, otherwise false.
 */
int nl_object_identical(struct nl_object *a, struct nl_object *b)
{
	struct nl_object_ops *ops = obj_ops(a);
	uint32_t req_attrs;

	/* Both objects must be of same type */
	if (ops != obj_ops(b))
		return 0;

	if (ops->oo_id_attrs_get) {
		int req_attrs_a = ops->oo_id_attrs_get(a);
		int req_attrs_b = ops->oo_id_attrs_get(b);
		if (req_attrs_a != req_attrs_b)
			return 0;
		req_attrs = req_attrs_a;
	} else if (ops->oo_id_attrs) {
		req_attrs = ops->oo_id_attrs;
	} else {
		req_attrs = 0xFFFFFFFF;
	}
	if (req_attrs == 0xFFFFFFFF)
		req_attrs = a->ce_mask & b->ce_mask;

	/* Both objects must provide all required attributes to uniquely
	 * identify an object */
	if ((a->ce_mask & req_attrs) != req_attrs ||
	    (b->ce_mask & req_attrs) != req_attrs)
		return 0;

	/* Can't judge unless we can compare */
	if (ops->oo_compare == NULL)
		return 0;

	return !(ops->oo_compare(a, b, req_attrs, 0));
}

/**
 * Compute bitmask representing difference in attribute values
 * @arg a		an object
 * @arg b		another object of same type
 *
 * The bitmask returned is specific to an object type, each bit set represents
 * an attribute which mismatches in either of the two objects. Unavailability
 * of an attribute in one object and presence in the other is regarded a
 * mismatch as well.
 *
 * @return Bitmask describing differences or 0 if they are completely identical.
 */
uint32_t nl_object_diff(struct nl_object *a, struct nl_object *b)
{
	struct nl_object_ops *ops = obj_ops(a);

	if (ops != obj_ops(b) || ops->oo_compare == NULL)
		return UINT_MAX;

	return ops->oo_compare(a, b, ~0, 0);
}

/**
 * Match a filter against an object
 * @arg obj		object to check
 * @arg filter		object of same type acting as filter
 *
 * @return 1 if the object matches the filter or 0
 *           if no filter procedure is available or if the
 *           filter does not match.
 */
int nl_object_match_filter(struct nl_object *obj, struct nl_object *filter)
{
	struct nl_object_ops *ops = obj_ops(obj);

	if (ops != obj_ops(filter) || ops->oo_compare == NULL)
		return 0;

	return !(ops->oo_compare(obj, filter, filter->ce_mask,
				 LOOSE_COMPARISON));
}

/**
 * Convert bitmask of attributes to a character string
 * @arg obj		object of same type as attribute bitmask
 * @arg attrs		bitmask of attribute types
 * @arg buf		destination buffer
 * @arg len		length of destination buffer
 *
 * Converts the bitmask of attribute types into a list of attribute
 * names separated by comas.
 *
 * @return destination buffer.
 */
char *nl_object_attrs2str(struct nl_object *obj, uint32_t attrs,
			  char *buf, size_t len)
{
	struct nl_object_ops *ops = obj_ops(obj);

	if (ops->oo_attrs2str != NULL)
		return ops->oo_attrs2str(attrs, buf, len);
	else {
		memset(buf, 0, len);
		return buf;
	}
}

/**
 * Return list of attributes present in an object
 * @arg obj		an object
 * @arg buf		destination buffer
 * @arg len		length of destination buffer
 *
 * @return destination buffer.
 */
char *nl_object_attr_list(struct nl_object *obj, char *buf, size_t len)
{
	return nl_object_attrs2str(obj, obj->ce_mask, buf, len);
}

/**
 * Generate object hash key
 * @arg obj		the object
 * @arg hashkey		destination buffer to be used for key stream
 * @arg hashtbl_sz	hash table size
 *
 * @return hash key in destination buffer
 */
void nl_object_keygen(struct nl_object *obj, uint32_t *hashkey,
		      uint32_t hashtbl_sz)
{
	struct nl_object_ops *ops = obj_ops(obj);

	if (ops->oo_keygen)
		ops->oo_keygen(obj, hashkey, hashtbl_sz);
	else
		*hashkey = 0;

	return;
}

/** @} */

/**
 * @name Attributes
 * @{
 */

/**
 * Return number of references held
 * @arg obj		object
 *
 * @return The number of references held to this object
 */
int nl_object_get_refcnt(struct nl_object *obj)
{
	return obj->ce_refcnt;
}

/**
 * Return cache the object is associated with
 * @arg obj		object
 *
 * @note The returned pointer is not protected with a reference counter,
 *       it is your responsibility.
 *
 * @return Pointer to cache or NULL if not associated with a cache.
 */
struct nl_cache *nl_object_get_cache(struct nl_object *obj)
{
	return obj->ce_cache;
}

/**
 * Return the object's type
 * @arg obj		object
 *
 * FIXME: link to list of object types
 *
 * @return Name of the object type
 */
const char *nl_object_get_type(const struct nl_object *obj)
{
	if (!obj->ce_ops)
		BUG();

	return obj->ce_ops->oo_name;
}

/**
 * Return the netlink message type the object was derived from
 * @arg obj		object
 *
 * @return Netlink message type or 0.
 */
int nl_object_get_msgtype(const struct nl_object *obj)
{
	return obj->ce_msgtype;
}

/**
 * Return object operations structure
 * @arg obj		object
 *
 * @return Pointer to the object operations structure
 */
struct nl_object_ops *nl_object_get_ops(const struct nl_object *obj)
{
	return obj->ce_ops;
}

/**
 * Return object id attribute mask
 * @arg obj		object
 *
 * @return object id attribute mask
 */
uint32_t nl_object_get_id_attrs(struct nl_object *obj)
{
	struct nl_object_ops *ops = obj_ops(obj);
	uint32_t id_attrs;

	if (!ops)
		return 0;

	if (ops->oo_id_attrs_get)
		id_attrs = ops->oo_id_attrs_get(obj);
	else
		id_attrs = ops->oo_id_attrs;

	return id_attrs;
}

/** @} */

/** @} */
