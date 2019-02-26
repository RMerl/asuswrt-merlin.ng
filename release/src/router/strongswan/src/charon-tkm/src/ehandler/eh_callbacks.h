/*
 * Copyright (C) 2012 Reto Buerki
 * Copyright (C) 2012 Adrian-Ken Rueegsegger
 * HSR Hochschule fuer Technik Rapperswil
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

/**
 * @defgroup tkm-ehandler exception handler
 * @{ @ingroup tkm
 *
 * The exception handler callback is registered as global exception action in
 * the Ada runtime. If an exception is raised in Ada code this callback is
 * executed.
 */

#ifndef EH_CALLBACKS_H_
#define EH_CALLBACKS_H_

/**
 * Log given message and terminate charon.
 */
void charon_terminate(char *msg);

#endif /** EH_CALLBACKS_H_ @}*/
