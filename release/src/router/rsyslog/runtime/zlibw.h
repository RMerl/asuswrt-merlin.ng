/* The zlibw object. It encapsulates the zlib functionality. The primary
 * purpose of this wrapper class is to enable rsyslogd core to be build without
 * zlib libraries.
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
#ifndef INCLUDED_ZLIBW_H
#define INCLUDED_ZLIBW_H

#include <zlib.h>

/* interfaces */
BEGINinterface(zlibw) /* name must also be changed in ENDinterface macro! */
	int (*DeflateInit)(z_streamp strm, int);
	int (*DeflateInit2)(z_streamp strm, int level, int method, int windowBits, int memLevel, int strategy);
	int (*Deflate)(z_streamp strm, int);
	int (*DeflateEnd)(z_streamp strm);
ENDinterface(zlibw)
#define zlibwCURR_IF_VERSION 1 /* increment whenever you change the interface structure! */


/* prototypes */
PROTOTYPEObj(zlibw);

/* the name of our library binary */
#define LM_ZLIBW_FILENAME "lmzlibw"

#endif /* #ifndef INCLUDED_ZLIBW_H */
