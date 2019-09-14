/* The prop object.
 *
 * This implements props within rsyslog.
 *
 * Copyright 2009-2012 Adiscon GmbH.
 *
 * This file is part of the rsyslog runtime library.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *       -or-
 *       see COPYING.ASL20 in the source distribution
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef INCLUDED_PROP_H
#define INCLUDED_PROP_H
#include "atomic.h"

/* the prop object */
struct prop_s {
	BEGINobjInstance;	/* Data to implement generic object - MUST be the first data element! */
	int iRefCount;		/* reference counter */
	union {
		uchar *psz;		/* stored string */
		uchar sz[CONF_PROP_BUFSIZE];
	} szVal;
	int len;		/* we use int intentionally, otherwise we may get some troubles... */
	DEF_ATOMIC_HELPER_MUT(mutRefCount)
};

/* interfaces */
BEGINinterface(prop) /* name must also be changed in ENDinterface macro! */
	INTERFACEObjDebugPrint(prop);
	rsRetVal (*Construct)(prop_t **ppThis);
	rsRetVal (*ConstructFinalize)(prop_t *pThis);
	rsRetVal (*Destruct)(prop_t **ppThis);
	rsRetVal (*SetString)(prop_t *pThis, const uchar* psz, const int len);
	rsRetVal (*GetString)(prop_t *pThis, uchar** ppsz, int *plen);
	int      (*GetStringLen)(prop_t *pThis);
	rsRetVal (*AddRef)(prop_t *pThis);
	rsRetVal (*CreateStringProp)(prop_t **ppThis, const uchar* psz, const int len);
	rsRetVal (*CreateOrReuseStringProp)(prop_t **ppThis, const uchar *psz, const int len);
ENDinterface(prop)
#define propCURR_IF_VERSION 1 /* increment whenever you change the interface structure! */


/* get classic c-style string */
/* Note: I know that "static inline" is not the right thing from a C99
 * PoV, but some environments treat, even in C99 mode, compile
 * non-static inline into the source even if not defined as "extern". This
 * obviously results in linker errors. Using "static inline" as below together
 * with "__attribute__((unused))" works in all cases. Note also that we
 * cannot work around this as we would otherwise need to evaluate
 * pThis more than once.
 */
static inline uchar * __attribute__((unused)) ATTR_NONNULL(1)
propGetSzStr(prop_t *pThis)
{
	return(pThis->len < CONF_PROP_BUFSIZE) ? pThis->szVal.sz : pThis->szVal.psz;
}

/* prototypes */
PROTOTYPEObj(prop);

#endif /* #ifndef INCLUDED_PROP_H */
