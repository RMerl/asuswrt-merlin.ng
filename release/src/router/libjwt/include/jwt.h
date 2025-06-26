/* Copyright (C) 2015-2023 Ben Collins <bcollins@maclara-llc.com>
   This file is part of the JWT C Library

   SPDX-License-Identifier:  MPL-2.0
   This Source Code Form is subject to the terms of the Mozilla Public
   License, v. 2.0. If a copy of the MPL was not distributed with this
   file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/**
 * @file jwt.h
 * @brief JWT C Library
 */

#ifndef JWT_H
#define JWT_H

#include <stdio.h>
#include <time.h>

#ifdef _MSC_VER

	#define DEPRECATED(func) __declspec(deprecated) func

	#define alloca _alloca
	#define strcasecmp _stricmp
	#define strdup _strdup

	#ifdef JWT_DLL_CONFIG
		#ifdef JWT_BUILD_SHARED_LIBRARY
			#define JWT_EXPORT __declspec(dllexport)
		#else
			#define JWT_EXPORT __declspec(dllimport)
		#endif
	#else
		#define JWT_EXPORT
	#endif

#else

	#define DEPRECATED(func) func __attribute__ ((deprecated))
	#define JWT_EXPORT

#endif

#ifdef __cplusplus
extern "C" {
#endif

/** Opaque JWT object. */
typedef struct jwt jwt_t;

/** Opaque JWT validation object. */
typedef struct jwt_valid jwt_valid_t;

/** JWT algorithm types. */
typedef enum jwt_alg {
	JWT_ALG_NONE = 0,
	JWT_ALG_HS256,
	JWT_ALG_HS384,
	JWT_ALG_HS512,
	JWT_ALG_RS256,
	JWT_ALG_RS384,
	JWT_ALG_RS512,
	JWT_ALG_ES256,
	JWT_ALG_ES384,
	JWT_ALG_ES512,
	JWT_ALG_PS256,
	JWT_ALG_PS384,
	JWT_ALG_PS512,
	JWT_ALG_TERM
} jwt_alg_t;

#define JWT_ALG_INVAL JWT_ALG_TERM

/** JWT Validation exception types. These are bit values. */
#define JWT_VALIDATION_SUCCESS		0x0000
#define JWT_VALIDATION_ERROR		0x0001	/* General failures */
#define JWT_VALIDATION_ALG_MISMATCH	0x0002
#define JWT_VALIDATION_EXPIRED		0x0004
#define JWT_VALIDATION_TOO_NEW		0x0008
#define JWT_VALIDATION_ISS_MISMATCH	0x0010
#define JWT_VALIDATION_SUB_MISMATCH	0x0020
#define JWT_VALIDATION_AUD_MISMATCH	0x0040
#define JWT_VALIDATION_GRANT_MISSING	0x0080
#define JWT_VALIDATION_GRANT_MISMATCH	0x0100

/** JWT Memory allocation overrides */
typedef void *(*jwt_malloc_t)(size_t);
typedef void *(*jwt_realloc_t)(void *, size_t);
typedef void (*jwt_free_t)(void *);

/** Structure used by key provider to return a key */
typedef struct {
    const unsigned char *jwt_key;
    int jwt_key_len;
} jwt_key_t;

/** Key provider - inspects the JWT to obtain the key used to verify the signature */
typedef int (*jwt_key_p_t)(const jwt_t *, jwt_key_t *);

/**
 * @defgroup jwt_new JWT Object Creation
 * Functions used to create and destroy JWT objects.
 *
 * Generally, one would use the jwt_new() function to create an object
 * from scratch and jwt_decode() to create and verify and object from an
 * existing token.
 *
 * Note, when using RSA keys (e.g. with RS256), the key is expected to be
 * a private key in PEM format. If the RSA private key requires a passphrase,
 * the default is to request it on the command line from stdin. However,
 * you can override this using OpenSSL's default_passwd routines. For
 * example, using SSL_CTX_set_default_passwd_cb().
 * @{
 */

/**
 * Allocate a new, empty, JWT object.
 *
 * This is used to create a new object for a JWT. After you have finished
 * with the object, use jwt_free() to clean up the memory used by it.
 *
 * @param jwt Pointer to a JWT object pointer. Will be allocated on
 *     success.
 * @return 0 on success, valid errno otherwise.
 */
JWT_EXPORT int jwt_new(jwt_t **jwt);

/**
 * Verify an existing JWT and allocate a new JWT object from it.
 *
 * Decodes a JWT string and verifies the signature (if one is supplied).
 * If no signature is used (JWS, alg="none") or key is NULL, then no
 * validation is done other than formatting. It is not suggested to use
 * this on a string that has a signature without passing the key to
 * verify it. If the JWT is encrypted and no key is supplied, an error
 * is returned.
 *
 * @param jwt Pointer to a JWT object pointer. Will be allocated on
 *     success.
 * @param token Pointer to a valid JWT string, nul terminated.
 * @param key Pointer to the key for validating the JWT signature or for
 *     decrypting the token or NULL if no validation is to be performed.
 * @param key_len The length of the above key.
 * @return 0 on success, valid errno otherwise.
 *
 * @remark If a key is supplied, the token must pass sig check or decrypt
 *     for it to be parsed without error. If no key is supplied, then a
 *     non-encrypted token will be parsed without any checks for a valid
 *     signature, however, standard validation of the token is still
 *     performed.
 */
JWT_EXPORT int jwt_decode(jwt_t **jwt, const char *token,
	                 const unsigned char *key, int key_len);

/**
 * Like jwt_decode(), but the key will be obtained via the key provider.
 * Key providers may use all sorts of key management techniques, e.g.
 * can check the "kid" header parameter or download the key pointed to 
 * in "x5u"
 *
 * @param jwt Pointer to a JWT object pointer. Will be allocated on
 *     success.
 * @param token Pointer to a valid JWT string, null terminated.
 * @param key_provider Pointer to a function that will obtain the key for the given JWT. 
 *      Returns 0 on success or any other value on failure.
 *      In the case of an error, the same error value will be returned to the caller.
 * @return 0 on success, valid errno otherwise.
 *
 * @remark See jwt_decode()
 */
JWT_EXPORT int jwt_decode_2(jwt_t **jwt, const char *token, jwt_key_p_t key_provider);

/**
 * Free a JWT object and any other resources it is using.
 *
 * After calling, the JWT object referenced will no longer be valid and
 * its memory will be freed.
 *
 * @param jwt Pointer to a JWT object previously created with jwt_new()
 *            or jwt_decode().
 */
JWT_EXPORT void jwt_free(jwt_t *jwt);

/**
 * Duplicate an existing JWT object.
 *
 * Copies all grants and algorithm specific bits to a new JWT object.
 *
 * @param jwt Pointer to a JWT object.
 * @return A new object on success, NULL on error with errno set
 *     appropriately.
 */
JWT_EXPORT jwt_t *jwt_dup(jwt_t *jwt);

/** @} */

/**
 * @defgroup jwt_grant JWT Grant Manipulation
 * These functions allow you to add, remove and retrieve grants from a JWT
 * object.
 * @{
 */

/**
 * Return the value of a string grant.
 *
 * Returns the string value for a grant (e.g. "iss"). If it does not exist,
 * NULL will be returned.
 *
 * @param jwt Pointer to a JWT object.
 * @param grant String containing the name of the grant to return a value
 *     for.
 * @return Returns a string for the value, or NULL when not found.
 *
 * Note, this will only return grants with JSON string values. Use
 * jwt_get_grants_json() to get the JSON representation of more complex
 * values (e.g. arrays) or use jwt_get_grant_int() to get simple integer
 * values.
 */
JWT_EXPORT const char *jwt_get_grant(jwt_t *jwt, const char *grant);

/**
 * Return the value of an integer grant.
 *
 * Returns the int value for a grant (e.g. "exp"). If it does not exist,
 * 0 will be returned.
 *
 * @param jwt Pointer to a JWT object.
 * @param grant String containing the name of the grant to return a value
 *     for.
 * @return Returns an int for the value. Sets errno to ENOENT when not
 * found.
 *
 * Note, this will only return grants with JSON integer values. Use
 * jwt_get_grants_json() to get the JSON representation of more complex
 * values (e.g. arrays) or use jwt_get_grant() to get string values.
 */
JWT_EXPORT long jwt_get_grant_int(jwt_t *jwt, const char *grant);

/**
 * Return the value of an boolean grant.
 *
 * Returns the int value for a grant (e.g. "exp"). If it does not exist,
 * 0 will be returned.
 *
 * @param jwt Pointer to a JWT object.
 * @param grant String containing the name of the grant to return a value
 *     for.
 * @return Returns a boolean for the value. Sets errno to ENOENT when not
 * found.
 *
 * Note, this will only return grants with JSON boolean values. Use
 * jwt_get_grants_json() to get the JSON representation of more complex
 * values (e.g. arrays) or use jwt_get_grant() to get string values.
 */
JWT_EXPORT int jwt_get_grant_bool(jwt_t *jwt, const char *grant);

/**
 * Return the value of a grant as JSON encoded object string.
 *
 * Returns the JSON encoded string value for a grant (e.g. "iss"). If it
 * does not exist, NULL will be returned.
 *
 * @param jwt Pointer to a JWT object.
 * @param grant String containing the name of the grant to return a value
 *     for. If this is NULL, all grants will be returned as a JSON encoded
 *     hash.
 * @return Returns a string for the value, or NULL when not found. The
 *     returned string must be freed by the caller.
 */
JWT_EXPORT char *jwt_get_grants_json(jwt_t *jwt, const char *grant);

/**
 * Add a new string grant to this JWT object.
 *
 * Creates a new grant for this object. The string for grant and val
 * are copied internally, so do not require that the pointer or string
 * remain valid for the lifetime of this object. It is an error if you
 * try to add a grant that already exists.
 *
 * @param jwt Pointer to a JWT object.
 * @param grant String containing the name of the grant to add.
 * @param val String containing the value to be saved for grant. Can be
 *     an empty string, but cannot be NULL.
 * @return Returns 0 on success, valid errno otherwise.
 *
 * Note, this only allows for string based grants. If you wish to add
 * integer grants, then use jwt_add_grant_int(). If you wish to add more
 * complex grants (e.g. an array), then use jwt_add_grants_json().
 */
JWT_EXPORT int jwt_add_grant(jwt_t *jwt, const char *grant, const char *val);

/**
 * Add a new integer grant to this JWT object.
 *
 * Creates a new grant for this object. The string for grant
 * is copied internally, so do not require that the pointer or string
 * remain valid for the lifetime of this object. It is an error if you
 * try to add a grant that already exists.
 *
 * @param jwt Pointer to a JWT object.
 * @param grant String containing the name of the grant to add.
 * @param val int containing the value to be saved for grant.
 * @return Returns 0 on success, valid errno otherwise.
 *
 * Note, this only allows for integer based grants. If you wish to add
 * string grants, then use jwt_add_grant(). If you wish to add more
 * complex grants (e.g. an array), then use jwt_add_grants_json().
 */
JWT_EXPORT int jwt_add_grant_int(jwt_t *jwt, const char *grant, long val);

/**
 * Add a new boolean grant to this JWT object.
 *
 * Creates a new grant for this object. The string for grant
 * is copied internally, so do not require that the pointer or string
 * remain valid for the lifetime of this object. It is an error if you
 * try to add a grant that already exists.
 *
 * @param jwt Pointer to a JWT object.
 * @param grant String containing the name of the grant to add.
 * @param val boolean containing the value to be saved for grant.
 * @return Returns 0 on success, valid errno otherwise.
 *
 * Note, this only allows for boolean based grants. If you wish to add
 * string grants, then use jwt_add_grant(). If you wish to add more
 * complex grants (e.g. an array), then use jwt_add_grants_json().
 */
JWT_EXPORT int jwt_add_grant_bool(jwt_t *jwt, const char *grant, int val);

/**
 * Add grants from a JSON encoded object string.
 *
 * Loads a grant from an existing JSON encoded object string. Overwrites
 * existing grant. If grant is NULL, then the JSON encoded string is
 * assumed to be a JSON hash of all grants being added and will be merged
 * into the grant listing.
 *
 * @param jwt Pointer to a JWT object.
 * @param json String containing a JSON encoded object of grants.
 * @return Returns 0 on success, valid errno otherwise.
 */
JWT_EXPORT int jwt_add_grants_json(jwt_t *jwt, const char *json);

/**
 * Delete a grant from this JWT object.
 *
 * Deletes the named grant from this object. It is not an error if there
 * is no grant matching the passed name. If grant is NULL, then all grants
 * are deleted from this JWT.
 *
 * @param jwt Pointer to a JWT object.
 * @param grant String containing the name of the grant to delete. If this
 *    is NULL, then all grants are deleted.
 * @return Returns 0 on success, valid errno otherwise.
 */
JWT_EXPORT int jwt_del_grants(jwt_t *jwt, const char *grant);

/** @} */

/**
 * @defgroup jwt_header JWT Header Manipulation
 * These functions allow you to add, remove and retrieve headers from a JWT
 * object.
 * @{
 */

/**
 * Return the value of a string header.
 *
 * Returns the string value for a header (e.g. ""). If it does not exist,
 * NULL will be returned.
 *
 * @param jwt Pointer to a JWT object.
 * @param header String containing the name of the header to return a value
 *     for.
 * @return Returns a string for the value, or NULL when not found.
 *
 * Note, this will only return headers with JSON string values. Use
 * jwt_get_header_json() to get the JSON representation of more complex
 * values (e.g. arrays) or use jwt_get_header_int() to get simple integer
 * values.
 */
JWT_EXPORT const char *jwt_get_header(jwt_t *jwt, const char *header);

/**
 * Return the value of an integer header.
 *
 * Returns the int value for a header (e.g. ""). If it does not exist,
 * 0 will be returned.
 *
 * @param jwt Pointer to a JWT object.
 * @param header String containing the name of the header to return a value
 *     for.
 * @return Returns an int for the value. Sets errno to ENOENT when not
 * found.
 *
 * Note, this will only return headers with JSON integer values. Use
 * jwt_get_header_json() to get the JSON representation of more complex
 * values (e.g. arrays) or use jwt_get_header() to get string values.
 */
JWT_EXPORT long jwt_get_header_int(jwt_t *jwt, const char *header);

/**
 * Return the value of an boolean header.
 *
 * Returns the int value for a header (e.g. ""). If it does not exist,
 * 0 will be returned.
 *
 * @param jwt Pointer to a JWT object.
 * @param header String containing the name of the header to return a value
 *     for.
 * @return Returns a boolean for the value. Sets errno to ENOENT when not
 * found.
 *
 * Note, this will only return headers with JSON boolean values. Use
 * jwt_get_header_json() to get the JSON representation of more complex
 * values (e.g. arrays) or use jwt_get_header() to get string values.
 */
JWT_EXPORT int jwt_get_header_bool(jwt_t *jwt, const char *header);

/**
 * Return the value of a header as JSON encoded object string.
 *
 * Returns the JSON encoded string value for a header (e.g. ""). If it
 * does not exist, NULL will be returned.
 *
 * @param jwt Pointer to a JWT object.
 * @param header String containing the name of the header to return a value
 *     for. If this is NULL, all headers will be returned as a JSON encoded
 *     hash.
 * @return Returns a string for the value, or NULL when not found. The
 *     returned string must be freed by the caller.
 */
JWT_EXPORT char *jwt_get_headers_json(jwt_t *jwt, const char *header);

/**
 * Add a new string header to this JWT object.
 *
 * Creates a new header for this object. The string for header and val
 * are copied internally, so do not require that the pointer or string
 * remain valid for the lifetime of this object. It is an error if you
 * try to add a header that already exists.
 *
 * @param jwt Pointer to a JWT object.
 * @param header String containing the name of the header to add.
 * @param val String containing the value to be saved for header. Can be
 *     an empty string, but cannot be NULL.
 * @return Returns 0 on success, valid errno otherwise.
 *
 * Note, this only allows for string based headers. If you wish to add
 * integer headers, then use jwt_add_header_int(). If you wish to add more
 * complex headers (e.g. an array), then use jwt_add_headers_json().
 */
JWT_EXPORT int jwt_add_header(jwt_t *jwt, const char *header, const char *val);

/**
 * Add a new integer header to this JWT object.
 *
 * Creates a new header for this object. The string for header
 * is copied internally, so do not require that the pointer or string
 * remain valid for the lifetime of this object. It is an error if you
 * try to add a header that already exists.
 *
 * @param jwt Pointer to a JWT object.
 * @param header String containing the name of the header to add.
 * @param val int containing the value to be saved for header.
 * @return Returns 0 on success, valid errno otherwise.
 *
 * Note, this only allows for integer based headers. If you wish to add
 * string headers, then use jwt_add_header(). If you wish to add more
 * complex headers (e.g. an array), then use jwt_add_headers_json().
 */
JWT_EXPORT int jwt_add_header_int(jwt_t *jwt, const char *header, long val);

/**
 * Add a new boolean header to this JWT object.
 *
 * Creates a new header for this object. The string for header
 * is copied internally, so do not require that the pointer or string
 * remain valid for the lifetime of this object. It is an error if you
 * try to add a header that already exists.
 *
 * @param jwt Pointer to a JWT object.
 * @param header String containing the name of the header to add.
 * @param val boolean containing the value to be saved for header.
 * @return Returns 0 on success, valid errno otherwise.
 *
 * Note, this only allows for boolean based headers. If you wish to add
 * string headers, then use jwt_add_header(). If you wish to add more
 * complex headers (e.g. an array), then use jwt_add_headers_json().
 */
JWT_EXPORT int jwt_add_header_bool(jwt_t *jwt, const char *header, int val);

/**
 * Add headers from a JSON encoded object string.
 *
 * Loads a header from an existing JSON encoded object string. Overwrites
 * existing header. If header is NULL, then the JSON encoded string is
 * assumed to be a JSON hash of all headers being added and will be merged
 * into the header listing.
 *
 * @param jwt Pointer to a JWT object.
 * @param json String containing a JSON encoded object of headers.
 * @return Returns 0 on success, valid errno otherwise.
 */
JWT_EXPORT int jwt_add_headers_json(jwt_t *jwt, const char *json);

/**
 * Delete a header from this JWT object.
 *
 * Deletes the named header from this object. It is not an error if there
 * is no header matching the passed name. If header is NULL, then all headers
 * are deleted from this JWT.
 *
 * @param jwt Pointer to a JWT object.
 * @param header String containing the name of the header to delete. If this
 *    is NULL, then all headers are deleted.
 * @return Returns 0 on success, valid errno otherwise.
 */
JWT_EXPORT int jwt_del_headers(jwt_t *jwt, const char *header);

/** @} */

/**
 * @defgroup jwt_encode JWT Output Functions
 * Functions that enable seeing the plain text or fully encoded version of
 * a JWT object.
 * @{
 */

/**
 * Output plain text representation to a FILE pointer.
 *
 * This function will write a plain text representation of this JWT object
 * without Base64 encoding. This only writes the header and body, and does
 * not compute the signature or encryption (if such an algorithm were being
 * used).
 *
 * Note, this may change the content of JWT header if algorithm is set
 * on the JWT object. If algorithm is set (jwt_set_alg was called
 * on the jwt object) then dumping JWT attempts to append 'typ' header.
 * If the 'typ' header already exists, then it is left untouched,
 * otherwise it is added with default value of "JWT".
 *
 * @param jwt Pointer to a JWT object.
 * @param fp Valid FILE pointer to write data to.
 * @param pretty Enables better visual formatting of output. Generally only
 *     used for debugging.
 * @return Returns 0 on success, valid errno otherwise.
 */
JWT_EXPORT int jwt_dump_fp(jwt_t *jwt, FILE *fp, int pretty);

/**
 * Return plain text representation as a string.
 *
 * Similar to jwt_dump_fp() except that a string is returned. The string
 * must be freed by the caller.
 *
 * Note, this may change the content of JWT header if algorithm is set
 * on the JWT object. If algorithm is set (jwt_set_alg was called
 * on the jwt object) then dumping JWT attempts to append 'typ' header.
 * If the 'typ' header already exists, then it is left untouched,
 * otherwise it is added with default value of "JWT".
 *
 * @param jwt Pointer to a JWT object.
 * @param pretty Enables better visual formatting of output. Generally only
 *     used for debugging.
 * @return A nul terminated string on success, NULL on error with errno
 *     set appropriately.
 */
JWT_EXPORT char *jwt_dump_str(jwt_t *jwt, int pretty);

/**
 * Return plain text representation of grants as a string.
 *
 * Similar to jwt_dump_str() except that only a string containing the
 * grants string is returned. The string must be freed by the caller.
 *
 * @param jwt Pointer to a JWT object.
 * @param pretty Enables better visual formatting of output. Generally only
 *     used for debugging.
 * @return A nul terminated string on success, NULL on error with errno
 *     set appropriately.
 */
JWT_EXPORT char *jwt_dump_grants_str(jwt_t *jwt, int pretty);

/**
 * Fully encode a JWT object and write it to FILE.
 *
 * This will create and write the complete JWT object to FILE. All parts
 * will be Base64 encoded and signatures or encryption will be applied if
 * the algorithm specified requires it.
 *
 * @param jwt Pointer to a JWT object.
 * @param fp Valid FILE pointer to write data to.
 * @return Returns 0 on success, valid errno otherwise.
 */
JWT_EXPORT int jwt_encode_fp(jwt_t *jwt, FILE *fp);

/**
 * Fully encode a JWT object and return as a string.
 *
 * Similar to jwt_encode_fp() except that a string is returned. The string
 * must be freed by the caller. If you changed the allocation method using
 * jwt_set_alloc, then you must use jwt_free_str() to free the memory.
 *
 * @param jwt Pointer to a JWT object.
 * @return A null terminated string on success, NULL on error with errno
 *     set appropriately.
 */
JWT_EXPORT char *jwt_encode_str(jwt_t *jwt);

/**
 * Free a string returned from the library.
 *
 * @param str Pointer to a string previously created with
 *     jwt_encode_str().
 */
JWT_EXPORT void jwt_free_str(char *str);

/** @} */

/**
 * @defgroup jwt_alg JWT Algorithm Functions
 * Set and check algorithms and algorithm specific values.
 *
 * When working with functions that require a key, the underlying library
 * takes care to scrub memory when the key is no longer used (e.g. when
 * calling jwt_free() or when changing the algorithm, the old key, if it
 * exists, is scrubbed).
 * @{
 */

/**
 * Set an algorithm from jwt_alg_t for this JWT object.
 *
 * Specifies an algorithm for a JWT object. If JWT_ALG_NONE is used, then
 * key must be NULL and len must be 0. All other algorithms must have a
 * valid pointer to key data, which may be specific to the algorithm (e.g
 * RS256 expects a PEM formatted RSA key).
 *
 * @param jwt Pointer to a JWT object.
 * @param alg A valid jwt_alg_t specifier.
 * @param key The key data to use for the algorithm.
 * @param len The length of the key data.
 * @return Returns 0 on success, valid errno otherwise.
 */
JWT_EXPORT int jwt_set_alg(jwt_t *jwt, jwt_alg_t alg, const unsigned char *key, int len);

/**
 * Get the jwt_alg_t set for this JWT object.
 *
 * Returns the jwt_alg_t type for this JWT object.
 *
 * @param jwt Pointer to a JWT object.
 * @returns Returns a jwt_alg_t type for this object.
 */
JWT_EXPORT jwt_alg_t jwt_get_alg(const jwt_t *jwt);

/**
 * Convert alg type to it's string representation.
 *
 * Returns a string that matches the alg type provided.
 *
 * @param alg A valid jwt_alg_t specifier.
 * @returns Returns a string (e.g. "RS256") matching the alg or NULL for
 *     invalid alg.
 */
JWT_EXPORT const char *jwt_alg_str(jwt_alg_t alg);

/**
 * Convert alg string to type.
 *
 * Returns an alg type based on the string representation.
 *
 * @param alg A valid string algorithm type (e.g. "RS256").
 * @returns Returns an alg type matching the string or JWT_ALG_INVAL if no
 *     matches were found.
 *
 * Note, this only works for algorithms that LibJWT supports or knows about.
 */
JWT_EXPORT jwt_alg_t jwt_str_alg(const char *alg);

/** @} */

/**
 * @defgroup jwt_memory JWT memory functions
 * These functions allow you to get or set memory allocation functions.
 * @{
 */

 /**
  * Set functions to be used for allocating and freeing memory.
  *
  * By default, LibJWT uses malloc, realloc, and free for memory
  * management. This function allows the user of the library to
  * specify its own memory management functions. This is especially
  * useful on Windows where mismatches in runtimes across DLLs can
  * cause problems.
  *
  * The caller can specify either a valid function pointer for
  * any of the parameters or NULL to use the corresponding default
  * allocator function.
  *
  * Note that this function will also set the memory allocator
  * for the Jansson library.
  *
  * @param pmalloc The function to use for allocating memory or
  *     NULL to use malloc
  * @param prealloc The function to use for reallocating memory or
  *     NULL to use realloc
  * @param pfree The function to use for freeing memory or
  *     NULL to use free
  * @returns 0 on success or errno otherwise.
  */
JWT_EXPORT int jwt_set_alloc(jwt_malloc_t pmalloc, jwt_realloc_t prealloc, jwt_free_t pfree);

/**
 * Get functions used for allocating and freeing memory.
 *
 * @param pmalloc Pointer to malloc function output variable, or NULL
 * @param prealloc Pointer to realloc function output variable, or NULL
 * @param pfree Pointer to free function output variable, or NULL
 */
JWT_EXPORT void jwt_get_alloc(jwt_malloc_t *pmalloc, jwt_realloc_t *prealloc, jwt_free_t *pfree);

 /** @} */

/**
 * @defgroup jwt_validate JWT validation functions
 * These functions allow you to define requirements for JWT validation.
 *
 * The most basic validation is that the JWT uses the expected algorithm.
 *
 * When replicating claims in header (usually for encrypted JWT), validation
 * tests that they match claims in the body (iss, sub, aud).
 *
 * Time-based claims can also be validated (nbf, exp).
 *
 * Finally, validation can test that claims be present and have certain value.
 *
 * @{
 */

/**
 * Validate a JWT object with a validation object.
 *
 * @param jwt Pointer to a JWT object.
 * @param jwt_valid Pointer to a JWT validation object.
 *
 * @return bitwide OR if possible validation errors or 0 on success
 */
JWT_EXPORT unsigned int jwt_validate(jwt_t *jwt, jwt_valid_t *jwt_valid);

/**
 * Allocate a new, JWT validation object.
 *
 * This is used to create a new object for a JWT validation. After you have
 * finished with the object, use jwt_valid_free() to clean up the memory used by
 * it.
 *
 * @param jwt_valid Pointer to a JWT validation object pointer. Will be allocated
 *     on success.
 * @param alg A valid jwt_alg_t specifier.
 * @return 0 on success, valid errno otherwise.
 */
JWT_EXPORT int jwt_valid_new(jwt_valid_t **jwt_valid, jwt_alg_t alg);

/**
 * Free a JWT validation object and any other resources it is using.
 *
 * After calling, the JWT validation object referenced will no longer be valid
 * and its memory will be freed.
 *
 * @param jwt_valid Pointer to a JWT validation object previously created with
 *     jwt_valid_new().
 */
JWT_EXPORT void jwt_valid_free(jwt_valid_t *jwt_valid);

/**
 * Return the status string for the validation object.
 *
 * The status of validation object is primarily for describing the reason
 * jwt_validate() failed.
 *
 * @param jwt_valid Pointer to a JWT validation object.
 * @return Returns current validation status as a bitwise OR of possible
 *   errors, or 0 if validation is currently successful.
 */
JWT_EXPORT unsigned int jwt_valid_get_status(jwt_valid_t *jwt_valid);

/**
 * Return the nbf_leeway value set.
 *
 * @param jwt_valid Pointer to a JWT validation object.
 * @return Returns current nbf_leeway value
 */
JWT_EXPORT time_t jwt_valid_get_nbf_leeway(jwt_valid_t *jwt_valid);

/**
 * Return the exp_leeway value set.
 *
 * @param jwt_valid Pointer to a JWT validation object.
 * @return Returns current exp_leeway value
 */
JWT_EXPORT time_t jwt_valid_get_exp_leeway(jwt_valid_t *jwt_valid);

/**
 * Add a new string grant requirement to this JWT validation object.
 *
 * @param jwt_valid Pointer to a JWT validation object.
 * @param grant String containing the name of the grant to add.
 * @param val String containing the value to be saved for grant. Can be
 *     an empty string, but cannot be NULL.
 * @return Returns 0 on success, valid errno otherwise.
 *
 * Note, this only allows for string based grants. If you wish to add
 * integer grants, then use jwt_valid_add_grant_int(). If you wish to add more
 * complex grants (e.g. an array), then use jwt_valid_add_grants_json().
 */
JWT_EXPORT int jwt_valid_add_grant(jwt_valid_t *jwt_valid, const char *grant, const char *val);

/**
 * Return the value of a string required grant.
 *
 * Returns the string value for a grant (e.g. "iss"). If it does not exist,
 * NULL will be returned.
 *
 * @param jwt_valid Pointer to a JWT validation object.
 * @param grant String containing the name of the grant to return a value
 *     for.
 * @return Returns a string for the value, or NULL when not found.
 *
 * Note, this will only return grants with JSON string values. Use
 * jwt_valid_get_grants_json() to get the JSON representation of more complex
 * values (e.g. arrays) or use jwt_valid_get_grant_int() to get simple integer
 * values.
 */
JWT_EXPORT const char *jwt_valid_get_grant(jwt_valid_t *jwt_valid, const char *grant);

/**
 * Add a new integer grant requirement to this JWT validation object.
 *
 * @param jwt_valid Pointer to a JWT validation object.
 * @param grant String containing the name of the grant to add.
 * @param val int containing the value to be saved for grant.
 * @return Returns 0 on success, valid errno otherwise.
 *
 * Note, this only allows for integer based grants. If you wish to add
 * string grants, then use jwt_valid_add_grant(). If you wish to add more
 * complex grants (e.g. an array), then use jwt_valid_add_grants_json().
 */
JWT_EXPORT int jwt_valid_add_grant_int(jwt_valid_t *jwt_valid, const char *grant, long val);

/**
 * Return the value of an integer required grant.
 *
 * Returns the int value for a grant (e.g. "exp"). If it does not exist,
 * 0 will be returned.
 *
 * @param jwt_valid Pointer to a JWT validation object.
 * @param grant String containing the name of the grant to return a value
 *     for.
 * @return Returns an int for the value. Sets errno to ENOENT when not
 * found.
 *
 * Note, this will only return grants with JSON integer values. Use
 * jwt_valid_get_grants_json() to get the JSON representation of more complex
 * values (e.g. arrays) or use jwt_valid_get_grant() to get string values.
 */
JWT_EXPORT long jwt_valid_get_grant_int(jwt_valid_t *jwt_valid, const char *grant);

/**
 * Add a new boolean required grant to this JWT validation object.
 *
 * Creates a new grant for this object. The string for grant
 * is copied internally, so do not require that the pointer or string
 * remain valid for the lifetime of this object. It is an error if you
 * try to add a grant that already exists.
 *
 * @param jwt_valid Pointer to a JWT validation object.
 * @param grant String containing the name of the grant to add.
 * @param val boolean containing the value to be saved for grant.
 * @return Returns 0 on success, valid errno otherwise.
 *
 * Note, this only allows for boolean based grants. If you wish to add
 * string grants, then use jwt_valid_add_grant(). If you wish to add more
 * complex grants (e.g. an array), then use jwt_valid_add_grants_json().
 */
JWT_EXPORT int jwt_valid_add_grant_bool(jwt_valid_t *jwt_valid, const char *grant, int val);

/**
 * Return the value of an boolean required grant.
 *
 * Returns the int value for a grant (e.g. "exp"). If it does not exist,
 * 0 will be returned.
 *
 * @param jwt_valid Pointer to a JWT validation object.
 * @param grant String containing the name of the grant to return a value
 *     for.
 * @return Returns a boolean for the value. Sets errno to ENOENT when not
 * found.
 *
 * Note, this will only return grants with JSON boolean values. Use
 * jwt_valid_get_grants_json() to get the JSON representation of more complex
 * values (e.g. arrays) or use jwt_valid_get_grant() to get string values.
 */
JWT_EXPORT int jwt_valid_get_grant_bool(jwt_valid_t *jwt_valid, const char *grant);

/**
 * Add required grants from a JSON encoded object string.
 *
 * Loads a grant from an existing JSON encoded object string. Overwrites
 * existing grant.
 *
 * @param jwt_valid Pointer to a JWT validation object.
 * @param json String containing a JSON encoded object of grants.
 * @return Returns 0 on success, valid errno otherwise.
 */
JWT_EXPORT int jwt_valid_add_grants_json(jwt_valid_t *jwt_valid, const char *json);

/**
 * Return the value of a grant as JSON encoded object string.
 *
 * Returns the JSON encoded string value for a grant (e.g. "iss"). If it
 * does not exist, NULL will be returned.
 *
 * @param jwt_valid Pointer to a JWT validation object.
 * @param grant String containing the name of the grant to return a value
 *     for.
 * @return Returns a string for the value, or NULL when not found. The
 *     returned string must be freed by the caller.
 */
JWT_EXPORT char* jwt_valid_get_grants_json(jwt_valid_t *jwt_valid, const char *grant);

/**
 * Delete a grant from this JWT object.
 *
 * Deletes the named grant from this object. It is not an error if there
 * is no grant matching the passed name. If grant is NULL, then all grants
 * are deleted from this JWT.
 *
 * @param jwt_valid Pointer to a JWT validation object.
 * @param grant String containing the name of the grant to delete. If this
 *    is NULL, then all grants are deleted.
 * @return Returns 0 on success, valid errno otherwise.
 */
JWT_EXPORT int jwt_valid_del_grants(jwt_valid_t *jwt_valid, const char *grant);

/**
 * Set the time for which expires and not-before claims should be evaluated.
 *
 * @param jwt_valid Pointer to a JWT validation object.
 * @param now Time to use when considering nbf and exp claims.
 * @return Returns 0 on success, valid errno otherwise.
 *
 * @remark jwt_validate() will not fail based on time if no expires or
 *     not-before claims exist in a JWT object.
 */
JWT_EXPORT int jwt_valid_set_now(jwt_valid_t *jwt_valid, const time_t now);

/**
 * Set the nbf_leeway value as defined in: https://www.rfc-editor.org/rfc/rfc7519#section-4.1.5.
 *
 * @param jwt_valid Pointer to a JWT validation object.
 * @param nbf_leeway leeway for nbf value.
 * @return Returns 0 on success, valid errno otherwise.
 *
 */
JWT_EXPORT int jwt_valid_set_nbf_leeway(jwt_valid_t *jwt_valid, const time_t nbf_leeway);

/**
 * Set the exp_leeway value as defined in: https://www.rfc-editor.org/rfc/rfc7519#section-4.1.4.
 *
 * @param jwt_valid Pointer to a JWT validation object.
 * @param exp_leeway leeway for exp value.
 * @return Returns 0 on success, valid errno otherwise.
 *
 */
JWT_EXPORT int jwt_valid_set_exp_leeway(jwt_valid_t *jwt_valid, const time_t exp_leeway);

/**
 * Set validation for replicated claims in headers.
 *
 * When set, validation tests for presence of iss, sub, aud in jwt headers and
 * tests match for same claims in body.
 *
 * @param jwt_valid Pointer to a JWT validation object.
 * @param hdr When true, test header claims
 * @return Returns 0 on success, valid errno otherwise.
 *
 * @remark jwt_validate() will not fail if iss, sub, aud are not present in JWT
 *     header or body.
 */
JWT_EXPORT int jwt_valid_set_headers(jwt_valid_t *jwt_valid, int hdr);

/**
 * Parses exceptions and returns a comma delimited and human-readable string.
 *
 * The returned string must be freed by the caller. If you changed the allocation
 * method using jwt_set_alloc, then you must use jwt_free_str() to free the memory.
 *
 * Note: This string is currently en-US ASCII only. Language support will come in the
 * future.
 *
 * @param exceptions Integer containing the exception flags.
 * @return A null terminated string on success, NULL on error with errno
 *     set appropriately.
 */
JWT_EXPORT char *jwt_exception_str(unsigned int exceptions);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* JWT_H */
