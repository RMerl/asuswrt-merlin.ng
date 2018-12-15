/* An implementation of the cryprov interface for libgcrypt.
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
#ifndef INCLUDED_LMCRY_GCRY_H
#define INCLUDED_LMCRY_GCRY_H
#include "cryprov.h"

/* interface is defined in cryprov.h, we just implement it! */
#define lmcry_gcryCURR_IF_VERSION cryprovCURR_IF_VERSION
typedef cryprov_if_t lmcry_gcry_if_t;

/* the lmcry_gcry object */
struct lmcry_gcry_s {
	BEGINobjInstance; /* Data to implement generic object - MUST be the first data element! */
	gcryctx ctx;
};
typedef struct lmcry_gcry_s lmcry_gcry_t;

/* prototypes */
PROTOTYPEObj(lmcry_gcry);

#endif /* #ifndef INCLUDED_LMCRY_GCRY_H */
