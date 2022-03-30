/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Fake include for Types.h
 *
 * Copyright (C) 2007-2009 Industrie Dial Face S.p.A.
 * Luigi 'Comio' Mantellini (luigi.mantellini@idf-hit.com)
 */

#ifndef __TYPES_H__FAKE__
#define __TYPES_H__FAKE__

/*
 *This avoids the collition with zlib.h Byte definition
 */
#define Byte LZByte

#include "../../lib/lzma/Types.h"

#endif
