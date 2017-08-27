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

/**
 * convert standard input to SQL hex binary
 */
int main(int argc, char *argv[])
{
	unsigned char byte;

	printf("X'");
	while (1)
	{
		if (fread(&byte, 1, 1, stdin) != 1)
		{
			break;
		}
		printf("%02x", (unsigned int)byte);
	}
	printf("'\n");
	return 0;
}

