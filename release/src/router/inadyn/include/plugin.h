/* DDNS Provider Plugin API for Inadyn
 *
 * Copyright (c) 2012-2020  Joachim Nilsson <troglobit@gmail.com>
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

#ifndef INADYN_PLUGIN_H_
#define INADYN_PLUGIN_H_

#include "queue.h"		/* BSD sys/queue.h API */

#define GENERIC_HTTP_REQUEST                                      	\
	"GET %s HTTP/1.0\r\n"						\
	"Host: %s\r\n"							\
	"User-Agent: %s\r\n\r\n"

#define PLUGIN_INIT(x) static void __attribute__ ((constructor)) x(void)
#define PLUGIN_EXIT(x) static void __attribute__ ((destructor))  x(void)

#define PLUGIN_ITERATOR(x, tmp) TAILQ_FOREACH_SAFE(x, &plugins, link, tmp)

/* Types used for DNS system specific configuration */
/* Function to prepare DNS system specific server requests */
typedef int (*setup_fn_t) (void* this, void* info, void* alias);
typedef int (*req_fn_t) (void *this, void *info, void *alias);
typedef int (*rsp_fn_t) (void *this, void *info, void *alias);

typedef struct ddns_system {
	TAILQ_ENTRY(ddns_system) link; /* BSD sys/queue.h linked list node. */

	const char    *name;

	setup_fn_t     setup;
	req_fn_t       request;
	rsp_fn_t       response;

	const int      nousername;    /* Provider does not require username='' */

	const char    *checkip_name;
	const char    *checkip_url;
	const int      checkip_ssl;

	const char    *server_name;
	const char    *server_url;
} ddns_system_t;

/* Public plugin API */
int plugin_register   (ddns_system_t *system);
int plugin_unregister (ddns_system_t *system);

/* Helper API */
ddns_system_t *plugin_find (const char *name, int loose);

/* Looks ugly, placed here due to deps. and to make it easier for plugin devs */
#include "ddns.h"

#endif /* INADYN_PLUGIN_H_ */

/**
 * Local Variables:
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
