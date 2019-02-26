/*
 * Copyright (C) 2008 Martin Willi
 * HSR Hochschule fuer Technik Rapperswil
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
 * @defgroup credential_factory credential_factory
 * @{ @ingroup credentials
 */

#ifndef CREDENTIAL_FACTORY_H_
#define CREDENTIAL_FACTORY_H_

typedef struct credential_factory_t credential_factory_t;
typedef enum credential_type_t credential_type_t;

#include <credentials/builder.h>

/**
 * Kind of credential.
 *
 * While crypto containers are not really credentials, we still use the
 * credential factory and builders create them.
 */
enum credential_type_t {
	/** private key, implemented in private_key_t */
	CRED_PRIVATE_KEY,
	/** public key, implemented in public_key_t */
	CRED_PUBLIC_KEY,
	/** certificates, implemented in certificate_t */
	CRED_CERTIFICATE,
	/** crypto container, implemented in container_t */
	CRED_CONTAINER,
};

/**
 * enum names for credential_type_t
 */
extern enum_name_t *credential_type_names;

/**
 * Manages credential construction functions and creates instances.
 */
struct credential_factory_t {

	/**
	 * Create a credential using a list of builder_part_t's.
	 *
	 * The variable argument list takes builder_part_t types followed
	 * by the type specific value. The list must be terminated using BUILD_END.
	 * All passed parts get cloned/refcounted by the builder functions,
	 * so free up allocated resources after successful and unsuccessful
	 * invocations.
	 *
	 * @param type			credential type to build
	 * @param subtype		subtype specific for type of the credential
	 * @param ...			build_part_t arguments, BUILD_END terminated.
	 * @return				type specific credential, NULL if failed
	 */
	void* (*create)(credential_factory_t *this, credential_type_t type,
					int subtype, ...);

	/**
	 * Register a credential builder function.
	 *
	 * The final flag indicates if the registered builder can build such
	 * a credential itself the most common encoding, without the need
	 * for an additional builder.
	 *
	 * @param type			type of credential the builder creates
	 * @param subtype		subtype of the credential, type specific
	 * @param final			TRUE if this build does not invoke other builders
	 * @param constructor	builder constructor function to register
	 */
	void (*add_builder)(credential_factory_t *this,
						credential_type_t type, int subtype, bool final,
						builder_function_t constructor);
	/**
	 * Unregister a credential builder function.
	 *
	 * @param constructor	constructor function to unregister.
	 */
	void (*remove_builder)(credential_factory_t *this,
						   builder_function_t constructor);

	/**
	 * Create an enumerator over registered builder types.
	 *
	 * The enumerator returns only builder types registered with the final
	 * flag set.
	 *
	 * @return				enumerator (credential_type_t, int subtype)
	 */
	enumerator_t* (*create_builder_enumerator)(credential_factory_t *this);

	/**
	 * Destroy a credential_factory instance.
	 */
	void (*destroy)(credential_factory_t *this);
};

/**
 * Create a credential_factory instance.
 */
credential_factory_t *credential_factory_create();

#endif /** CREDENTIAL_FACTORY_H_ @}*/
