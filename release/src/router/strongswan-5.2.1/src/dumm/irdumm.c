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

#undef PACKAGE_NAME
#undef PACKAGE_TARNAME
#undef PACKAGE_VERSION
#undef PACKAGE_STRING
#undef PACKAGE_BUGREPORT
#undef PACKAGE_URL
#include <ruby.h>

#ifdef HAVE_RB_ERRINFO
#define ruby_errinfo rb_errinfo()
#endif

/**
 * main routine, parses args and reads from console
 */
int main(int argc, char *argv[])
{
	int state, i;
	char buf[512];

	ruby_init();
	ruby_init_loadpath();

	rb_eval_string_protect("require 'dumm' and include Dumm", &state);
	if (state)
	{
		rb_p(ruby_errinfo);
		printf("Please install the ruby extension first!\n");
	}
	for (i = 1; i < argc; i++)
	{
		snprintf(buf, sizeof(buf), "load \"%s\"", argv[i]);
		printf("%s\n", buf);
		rb_eval_string_protect(buf, &state);
		if (state)
		{
			rb_p(ruby_errinfo);
		}
	}
	rb_require("irb");
	rb_require("irb/completion");
	rb_eval_string_protect("IRB.start", &state);
	if (state)
	{
		rb_p(ruby_errinfo);
	}

	ruby_finalize();
	return 0;
}

