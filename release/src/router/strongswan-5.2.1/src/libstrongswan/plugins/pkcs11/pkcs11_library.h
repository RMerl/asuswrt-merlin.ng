/*
 * Copyright (C) 2011 Tobias Brunner
 * Hochschule fuer Technik Rapperswil
 *
 * Copyright (C) 2010 Martin Willi
 * Copyright (C) 2010 revosec AG
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

/**
 * @defgroup pkcs11_library pkcs11_library
 * @{ @ingroup pkcs11
 */

#ifndef PKCS11_LIBRARY_H_
#define PKCS11_LIBRARY_H_

typedef enum pkcs11_feature_t pkcs11_feature_t;
typedef struct pkcs11_library_t pkcs11_library_t;

#include "pkcs11.h"

#include <utils/utils.h>
#include <utils/chunk.h>
#include <collections/enumerator.h>

/**
 * Optional PKCS#11 features some libraries support, some not
 */
enum pkcs11_feature_t {
	/** CKA_TRUSTED attribute supported for certificate objects */
	PKCS11_TRUSTED_CERTS = (1<<0),
	/** CKA_ALWAYS_AUTHENTICATE attribute supported for private keys */
	PKCS11_ALWAYS_AUTH_KEYS = (1<<1),
};

/**
 * A loaded and initialized PKCS#11 library.
 */
struct pkcs11_library_t {

	/**
	 * PKCS#11 function list, as returned by C_GetFunctionList
	 */
	CK_FUNCTION_LIST_PTR f;

	/**
	 * Get the name this instance was created with.
	 *
	 * @return			name, as passed to constructor
	 */
	char* (*get_name)(pkcs11_library_t *this);

	/**
	 * Get the feature set supported by this library.
	 *
	 * @return			ORed set of features supported
	 */
	pkcs11_feature_t (*get_features)(pkcs11_library_t *this);

	/**
	 * Create an enumerator over CK_OBJECT_HANDLE using a search template.
	 *
	 * An optional attribute array is automatically filled in with the
	 * objects associated attributes. If the value of an output attribute
	 * is NULL, the value gets allocated/freed during enumeration.
	 *
	 * @param session	session to use
	 * @param tmpl		search template
	 * @param tcount	number of attributes in the search template
	 * @param attr		attributes to read from object
	 * @param acount	number of attributes to read
	 */
	enumerator_t* (*create_object_enumerator)(pkcs11_library_t *this,
			CK_SESSION_HANDLE session, CK_ATTRIBUTE_PTR tmpl, CK_ULONG tcount,
			CK_ATTRIBUTE_PTR attr, CK_ULONG acount);

	/**
	 * This is very similar to the object enumerator but is only used to
	 * easily retrieve multiple attributes from a single object for which
	 * a handle is already known.
	 *
	 * The given attribute array is automatically filled in with the
	 * associated attributes. If the value of an output attribute is NULL,
	 * the required memory gets allocated/freed during enumeration.
	 *
	 * @param session	session to use
	 * @param object	object handle
	 * @param attr		attributes to read from object
	 * @param count		number of attributes to read
	 */
	enumerator_t* (*create_object_attr_enumerator)(pkcs11_library_t *this,
			CK_SESSION_HANDLE session, CK_OBJECT_HANDLE object,
			CK_ATTRIBUTE_PTR attr, CK_ULONG count);

	/**
	 * Create an enumerator over supported mechanisms of a token.
	 *
	 * The resulting enumerator enumerates over the mechanism type, and if
	 * a non-NULL pointer is given, over the mechanism info details.
	 *
	 * @param slot		slot of the token
	 * @return			enumerator over (CK_MECHANISM_TYPE, CK_MECHANISM_INFO)
	 */
	enumerator_t* (*create_mechanism_enumerator)(pkcs11_library_t *this,
												 CK_SLOT_ID slot);

	/**
	 * Retrieve a single attribute from the given object.
	 *
	 * Memory for the data is allocated.
	 *
	 * @param session			session with the PKCS#11 library
	 * @param obj				object handle
	 * @param type				attribute type to extract
	 * @param data				extracted data
	 * @return					TRUE if successful
	 */
	bool (*get_ck_attribute)(pkcs11_library_t *this, CK_SESSION_HANDLE session,
							 CK_OBJECT_HANDLE obj, CK_ATTRIBUTE_TYPE type,
							 chunk_t *data);

	/**
	 * Destroy a pkcs11_library_t.
	 */
	void (*destroy)(pkcs11_library_t *this);
};

/**
 * Enum names for CK_RV return values
 */
extern enum_name_t *ck_rv_names;

/**
 * Enum names for CK_MECHANISM_TYPE values
 */
extern enum_name_t *ck_mech_names;

/**
 * Enum names for CK_ATTRIBUTE_TYPE values
 */
extern enum_name_t *ck_attr_names;

/**
 * Trim/null terminate a string returned by the various PKCS#11 functions.
 *
 * @param str		string to trim
 * @param len		max length of the string
 */
void pkcs11_library_trim(char *str, int len);

/**
 * Create a pkcs11_library instance.
 *
 * @param name		an arbitrary name (for debugging), cloned
 * @param file		pkcs11 library file to dlopen()
 * @param os_lock	enforce OS Locking for this library
 * @return			library abstraction
 */
pkcs11_library_t *pkcs11_library_create(char *name, char *file, bool os_lock);

#endif /** PKCS11_LIBRARY_H_ @}*/
