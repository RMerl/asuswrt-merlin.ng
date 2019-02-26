/*
 * Copyright (c) 2014 Vyronas Tsingaras (vtsingaras@it.auth.gr)
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
 * @defgroup ext_auth ext_auth
 * @ingroup cplugins
 *
 * @defgroup ext_auth_plugin ext_auth_plugin
 * @{ @ingroup ext_auth
 */

#ifndef EXT_AUTH_PLUGIN_H_
#define EXT_AUTH_PLUGIN_H_

#include <plugins/plugin.h>

typedef struct ext_auth_plugin_t ext_auth_plugin_t;

/**
 * Plugin using an external script to authorize connections.
 */
struct ext_auth_plugin_t {

	/**
	 * Implements plugin interface.
	 */
	plugin_t plugin;
};

#endif /** EXT_AUTH_PLUGIN_H_ @}*/
