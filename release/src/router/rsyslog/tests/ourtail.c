/* This is a quick and dirty "tail implementation", one which always
 * skips the first line, but nothing else. I have done this to prevent
 * the various incompatible options of tail come into my way. One could
 * probably work around this by using autoconf magic, but for me it
 * was much quicker writing this small C program, which really should
 * be portable across all platforms.
 *
 * Part of the testbench for rsyslog.
 *
 * Copyright 2009 Rainer Gerhards and Adiscon GmbH.
 *
 * This file is part of rsyslog.
 *
 * Rsyslog is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Rsyslog is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Rsyslog.  If not, see <http://www.gnu.org/licenses/>.
 *
 * A copy of the GPL can be found in the file "COPYING" in this distribution.
 */
#include "config.h"
#include <stdio.h>

int main(int __attribute__((unused)) argc, char __attribute__((unused)) *argv[])
{
	int c;

	for(c = getchar() ; c != EOF && c != '\n' ; c = getchar())
		/*skip to newline*/;

	if(c == '\n')
		c = getchar();

	for( ; c != EOF ; c = getchar())
		putchar(c);

	return 0;
}
