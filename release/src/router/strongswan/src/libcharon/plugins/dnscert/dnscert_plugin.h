/*
 * Copyright (C) 2013 Ruslan Marchenko
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
 * @defgroup dnscert dnscert
 * @ingroup cplugins
 *
 * @defgroup dnscert_plugin dnscert_plugin
 * @{ @ingroup dnscert
 */

#ifndef DNSCERT_PLUGIN_H_
#define DNSCERT_PLUGIN_H_

#include <plugins/plugin.h>

typedef struct dnscert_plugin_t dnscert_plugin_t;

/**
 * DNSCERT plugin
 *
 * The DNSCERT plugin registers a credential set for CERT RRs.
 *
 * With this credential set it is possible to authenticate tunnel endpoints
 * using CERT resource records which are retrieved from the DNS in a secure
 * way (DNSSEC).
 */
struct dnscert_plugin_t {

	/**
	 * implements plugin interface
	 */
	plugin_t plugin;
};

#endif /** DNSCERT_PLUGIN_H_ @}*/
