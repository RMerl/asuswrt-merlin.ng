/*
 * Copyright (C) 2013 Endian srl
 * Author: Andrea Bonomi - <a.bonomi@endian.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

/**
 * @defgroup xauth_pam_i xauth_pam
 * @{ @ingroup xauth_pam
 */

#ifndef XAUTH_PAM_LISTENER_H_
#define XAUTH_PAM_LISTENER_H_

typedef struct xauth_pam_listener_t xauth_pam_listener_t;

#include <bus/listeners/listener.h>

/**
 * Listener
 */
struct xauth_pam_listener_t {

	/**
	 * Implements listener_t interface.
	 */
	listener_t listener;

	/**
	 * Destroy a xauth_pam_listener_t.
	 */
	void (*destroy)(xauth_pam_listener_t *this);
};

/**
 * Create a xauth_pam_listener instance.
 */
xauth_pam_listener_t *xauth_pam_listener_create();


#endif /** XAUTH_PAM_LISTENER_H_ @}*/
