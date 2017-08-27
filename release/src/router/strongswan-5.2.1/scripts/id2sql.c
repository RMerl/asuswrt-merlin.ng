/*
 * Copyright (C) 2008 Martin Willi
 * Hochschule fuer Technik Rapperswil
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

#include <stdio.h>
#include <utils/identification.h>

/**
 * convert an identity to type and encoding
 */
int main(int argc, char *argv[])
{
	identification_t *id;
	chunk_t enc;
	int i;

	if (argc < 2)
	{
		return -1;
	}

	id = identification_create_from_string(argv[1]);
	if (!id)
	{
		return -2;
	}
	printf("type\tencoding\n");
	printf("%d,\t", id->get_type(id));
	enc = id->get_encoding(id);

	printf("X'");
	for (i = 0; i < enc.len; i++)
	{
		printf("%02x", (unsigned int)enc.ptr[i]);
	}
	printf("'\n");
	return 0;
}

