/* libfastjson testbench tool
 *
 * Copyright (c) 2016 Adiscon GmbH
 * Rainer Gerhards <rgerhards@adiscon.com>
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See COPYING for details.
 *
 */
#include "config.h"

#include "../json.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int __attribute__((unused)) argc, char __attribute__((unused)) **argv)
{
	if(strcmp(fjson_version(), VERSION)) {
		fprintf(stderr, "ERROR: fjson_version reports '%s', VERSION is '%s'.\n",
			fjson_version(), VERSION);
		exit(1);
	}
	return 0;
}
