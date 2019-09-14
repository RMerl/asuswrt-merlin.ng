/* The interface definition for (file) signature providers.
 *
 * This is just an abstract driver interface, which needs to be
 * implemented by concrete classes.
 *
 * Copyright 2013 Adiscon GmbH.
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
#ifndef INCLUDED_SIGPROV_H
#define INCLUDED_SIGPROV_H

/* interface */
BEGINinterface(sigprov) /* name must also be changed in ENDinterface macro! */
	rsRetVal (*Construct)(void *ppThis);
	rsRetVal (*SetCnfParam)(void *ppThis, struct nvlst *lst);
	rsRetVal (*Destruct)(void *ppThis);
	rsRetVal (*OnFileOpen)(void *pThis, uchar *fn, void *pFileInstData);
	rsRetVal (*OnRecordWrite)(void *pFileInstData, uchar *rec, rs_size_t lenRec);
	rsRetVal (*OnFileClose)(void *pFileInstData);
ENDinterface(sigprov)
#define sigprovCURR_IF_VERSION 1 /* increment whenever you change the interface structure! */
#endif /* #ifndef INCLUDED_SIGPROV_H */
