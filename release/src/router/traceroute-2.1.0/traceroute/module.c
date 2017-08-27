/*
    Copyright (c)  2006, 2007		Dmitry Butskoy
					<buc@citadel.stu.neva.ru>
    License:  GPL v2 or any later

    See COPYING for the status of this software.
*/

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "traceroute.h"


static tr_module *base = NULL;

void tr_register_module (tr_module *ops) {

	ops->next = base;
	base = ops;
}

const tr_module *tr_get_module (const char *name) {
	const tr_module *ops;

	if (!name)  return 0;

	for (ops = base; ops; ops = ops->next) {
	    if (!strcasecmp (name, ops->name))
		    return ops;
	}

	return NULL;
}
