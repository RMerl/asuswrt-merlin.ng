/*
 * Copyright (C) 2012 Tobias Brunner
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
 * @defgroup android_private_key android_private_key
 * @{ @ingroup android_backend
 */

#ifndef ANDROID_PRIVATE_KEY_H_
#define ANDROID_PRIVATE_KEY_H_

#include <jni.h>

#include <credentials/keys/private_key.h>

/**
 * Create a JNI backed key, stored in the Android KeyChain
 *
 * @param key		PrivateKey instance
 * @param pubkey	public key as extracted from the certificate (gets adopted)
 * @return 			private_key_t instance
 */
private_key_t *android_private_key_create(jobject key, public_key_t *pubkey);

#endif /** ANDROID_PRIVATE_KEY_H_ @}*/

