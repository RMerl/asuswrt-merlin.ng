/*
 * Copyright (C) 2010 Tobias Brunner
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

/* stuff defined in AndroidConfig.h, which is included using the -include
 * command-line option, thus cannot be undefined using -U CFLAGS options.
 * the reason we have to undefine these flags in the first place, is that
 * AndroidConfig.h defines them as 0, which in turn means that they are
 * actually defined. */

#undef HAVE_BACKTRACE
