/* Definitions for the nssel IO waiter.
 *
 * Copyright 2008-2012 Adiscon GmbH.
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

#ifndef INCLUDED_NSSEL_H
#define INCLUDED_NSSEL_H

#include "netstrms.h"

/* the nssel object */
struct nssel_s {
	BEGINobjInstance;	/* Data to implement generic object - MUST be the first data element! */
	nsd_t *pDrvrData;	/**< the driver's data elements */
	uchar *pBaseDrvrName;	/**< nsd base driver name to use, or NULL if system default */
	uchar *pDrvrName;	/**< full base driver name (set when driver is loaded) */
	nsdsel_if_t Drvr;	/**< our stream driver */
};


/* interface */
BEGINinterface(nssel) /* name must also be changed in ENDinterface macro! */
	rsRetVal (*Construct)(nssel_t **ppThis);
	rsRetVal (*ConstructFinalize)(nssel_t *pThis);
	rsRetVal (*Destruct)(nssel_t **ppThis);
	rsRetVal (*Add)(nssel_t *pThis, netstrm_t *pStrm, nsdsel_waitOp_t waitOp);
	rsRetVal (*Wait)(nssel_t *pThis, int *pNumReady);
	rsRetVal (*IsReady)(nssel_t *pThis, netstrm_t *pStrm, nsdsel_waitOp_t waitOp, int *pbIsReady,
		int *piNumReady);
	/* v2 - 2013-09-17 by rgerhards */
	rsRetVal (*SetDrvrName)(nssel_t *pThis, uchar *name);
ENDinterface(nssel)
#define nsselCURR_IF_VERSION 2 /* increment whenever you change the interface structure! */

/* prototypes */
PROTOTYPEObj(nssel);

/* the name of our library binary */
#define LM_NSSEL_FILENAME LM_NETSTRMS_FILENAME

#endif /* #ifndef INCLUDED_NSSEL_H */
