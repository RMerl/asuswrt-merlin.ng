/*
 * Copyright (C) 2015 Martin Willi
 * Copyright (C) 2015 revosec AG
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
 * @defgroup chapoly_drv chapoly_drv
 * @{ @ingroup chapoly
 */

#ifndef CHAPOLY_DRV_H_
#define CHAPOLY_DRV_H_

#include <library.h>

#define CHACHA_BLOCK_SIZE 64
#define CHACHA_IV_SIZE 8
#define CHACHA_SALT_SIZE 4
#define CHACHA_KEY_SIZE 32
#define POLY_BLOCK_SIZE 16
#define POLY_ICV_SIZE 16

typedef struct chapoly_drv_t chapoly_drv_t;

/**
 * ChaCha20/Poly1305 backend implementation.
 */
struct chapoly_drv_t {

	/**
	 * Set the ChaCha20 encryption key.
	 *
	 * @param constant		16 byte key constant to use
	 * @param key			32 byte encryption key
	 * @param salt			4 byte nonce salt
	 * @return				TRUE if key set
	 */
	bool (*set_key)(chapoly_drv_t *this, u_char *constant, u_char *key,
					u_char *salt);

	/**
	 * Start an AEAD en/decryption session, reset state.
	 *
	 * @param iv			8 byte initialization vector for nonce
	 * @return				TRUE if initialized
	 */
	bool (*init)(chapoly_drv_t *this, u_char *iv);

	/**
	 * Poly1305 update multiple blocks.
	 *
	 * @param data			data to update Poly1305 for
	 * @param blocks		number of 16-byte blocks to process
	 * @return				TRUE if updated
	 */
	bool (*poly)(chapoly_drv_t *this, u_char *data, u_int blocks);

	/**
	 * Create a single ChaCha20 keystream block.
	 *
	 * @param stream		64-byte block to write key stream data to
	 * @return				TRUE if keystream returned
	 */
	bool (*chacha)(chapoly_drv_t *this, u_char *stream);

	/**
	 * Encrypt multiple blocks of data inline, update Poly1305.
	 *
	 * @param data			data to process
	 * @param blocks		number of 64-byte blocks to process
	 * @return				TRUE if encrypted
	 */
	bool (*encrypt)(chapoly_drv_t *this, u_char *data, u_int blocks);

	/**
	 * Decrypt multiple blocks of data inline, update Poly1305.
	 *
	 * @param data			data to process
	 * @param blocks		number of 64-byte blocks to process
	 * @return				TRUE if decrypted
	 */
	bool (*decrypt)(chapoly_drv_t *this, u_char *data, u_int blocks);

	/**
	 * End a AEAD encryption session, return MAC.
	 *
	 * @param mac			16-byte block to write MAC to
	 * @return				TRUE if MAC returned
	 */
	bool (*finish)(chapoly_drv_t *this, u_char *mac);

	/**
	 * Destroy a chapoly_drv_t.
	 */
	void (*destroy)(chapoly_drv_t *this);
};

/**
 * Create a chapoly_drv instance.
 */
chapoly_drv_t *chapoly_drv_probe();

#endif /** CHAPOLY_DRV_H_ @}*/
