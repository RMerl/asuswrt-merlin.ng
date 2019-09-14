/* header for strgen.c
 *
 * Copyright 2010 Rainer Gerhards and Adiscon GmbH.
 *
 * This file is part of the rsyslog runtime library.
 *
 * The rsyslog runtime library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * The rsyslog runtime library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with the rsyslog runtime library.  If not, see <http://www.gnu.org/licenses/>.
 *
 * A copy of the LGPL can be found in the file "COPYING.LESSER" in this distribution.
 */
#ifndef INCLUDED_STRGEN_H
#define INCLUDED_STRGEN_H


/* we create a small helper object, a list of strgens, that we can use to
 * build a chain of them whereever this is needed.
 */
struct strgenList_s {
	strgen_t *pStrgen;
	strgenList_t *pNext;
};


/* the strgen object, a dummy because we have only static methods */
struct strgen_s {
	BEGINobjInstance;	/* Data to implement generic object - MUST be the first data element! */
	uchar *pName;		/* name of this strgen */
	modInfo_t *pModule;	/* pointer to strgen's module */
};

/* interfaces */
BEGINinterface(strgen) /* name must also be changed in ENDinterface macro! */
	rsRetVal (*Construct)(strgen_t **ppThis);
	rsRetVal (*ConstructFinalize)(strgen_t *pThis);
	rsRetVal (*Destruct)(strgen_t **ppThis);
	rsRetVal (*SetName)(strgen_t *pThis, uchar *name);
	rsRetVal (*SetModPtr)(strgen_t *pThis, modInfo_t *pMod);
	rsRetVal (*FindStrgen)(strgen_t **ppThis, uchar*name);
	rsRetVal (*InitStrgenList)(strgenList_t **pListRoot);
	rsRetVal (*DestructStrgenList)(strgenList_t **pListRoot);
	rsRetVal (*AddStrgenToList)(strgenList_t **pListRoot, strgen_t *pStrgen);
ENDinterface(strgen)
#define strgenCURR_IF_VERSION 1 /* increment whenever you change the interface above! */


/* prototypes */
PROTOTYPEObj(strgen);

#endif /* #ifndef INCLUDED_STRGEN_H */
