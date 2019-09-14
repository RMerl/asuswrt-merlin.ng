/*
 * lib/data.c		Abstract Data
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
 * @defgroup data Abstract Data
 *
 * Abstract data type representing a binary data blob.
 *
 * Related sections in the development guide:
 * - @core_doc{_abstract_data, Abstract Data}
 *
 * @{
 *
 * Header
 * ------
 * ~~~~{.c}
 * #include <netlink/data.h>
 * ~~~~
 */

#include <netlink-private/netlink.h>
#include <netlink/netlink.h>
#include <netlink/utils.h>
#include <linux/socket.h>

/**
 * @name General
 * @{
 */

/**
 * Allocate a new abstract data object.
 * @arg buf		Data buffer containing the actual data.
 * @arg size		Size of data buffer.
 *
 * Allocates a new abstract data and copies the specified data
 * buffer into the new handle.
 * 
 * @return Newly allocated data handle or NULL
 */
struct nl_data *nl_data_alloc(void *buf, size_t size)
{
	struct nl_data *data;

	data = calloc(1, sizeof(*data));
	if (!data)
		goto errout;

	data->d_data = calloc(1, size);
	if (!data->d_data) {
		free(data);
		goto errout;
	}

	data->d_size = size;

	if (buf)
		memcpy(data->d_data, buf, size);

	return data;
errout:
	return NULL;
}

/**
 * Allocate abstract data object based on netlink attribute.
 * @arg nla		Netlink attribute of unspecific type.
 *
 * Allocates a new abstract data and copies the payload of the
 * attribute to the abstract data object.
 * 
 * @see nla_data_alloc
 * @return Newly allocated data handle or NULL
 */
struct nl_data *nl_data_alloc_attr(struct nlattr *nla)
{
	return nl_data_alloc(nla_data(nla), nla_len(nla));
}

/**
 * Clone an abstract data object.
 * @arg src		Abstract data object
 *
 * @return Cloned object or NULL
 */
struct nl_data *nl_data_clone(struct nl_data *src)
{
	return nl_data_alloc(src->d_data, src->d_size);
}

/**
 * Append data to an abstract data object.
 * @arg data		Abstract data object.
 * @arg buf		Data buffer containing the data to be appended.
 * @arg size		Size of data to be apppended.
 *
 * Reallocates an abstract data and copies the specified data
 * buffer into the new handle.
 * 
 * @return 0 on success or a negative error code
 */
int nl_data_append(struct nl_data *data, void *buf, size_t size)
{
	if (size > 0) {
		data->d_data = realloc(data->d_data, data->d_size + size);
		if (!data->d_data)
			return -NLE_NOMEM;

		if (buf)
			memcpy(data->d_data + data->d_size, buf, size);
		else
			memset(data->d_data + data->d_size, 0, size);

		data->d_size += size;
	}

	return 0;
}

/**
 * Free an abstract data object.
 * @arg data		Abstract data object.
 */
void nl_data_free(struct nl_data *data)
{
	if (data)
		free(data->d_data);

	free(data);
}

/** @} */

/**
 * @name Attribute Access
 * @{
 */

/**
 * Get data buffer of abstract data object.
 * @arg data		Abstract data object.
 * @return Data buffer or NULL if empty.
 */
void *nl_data_get(struct nl_data *data)
{
	return data->d_size > 0 ? data->d_data : NULL;
}

/**
 * Get size of data buffer of abstract data object.
 * @arg data		Abstract data object.
 * @return Size of data buffer.
 */
size_t nl_data_get_size(struct nl_data *data)
{
	return data->d_size;
}

/** @} */

/**
 * @name Misc
 * @{
 */

/**
 * Compare two abstract data objects.
 * @arg a		Abstract data object.
 * @arg b		Another abstract data object.
 * @return An integer less than, equal to, or greater than zero if
 *         a is found, respectively, to be less than, to match, or
 *         be greater than b.
 */
int nl_data_cmp(struct nl_data *a, struct nl_data *b)
{
	void *a_ = nl_data_get(a);
	void *b_ = nl_data_get(b);

	if (a_ && b_)
		return memcmp(a_, b_, nl_data_get_size(a));
	else
		return -1;
}

/** @} */
/** @} */
