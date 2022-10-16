/*
 * Copyright (C) 2021 Tobias Brunner, codelabs GmbH
 * Copyright (C) 2021 Thomas Egerer, secunet AG
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

/**
 * @defgroup metadata_int metadata_int
 * @{ @ingroup metadata
 */

#ifndef METADATA_INT_H_
#define METADATA_INT_H_

#include "metadata.h"

/**
 * Create a metadata object of an integer type.
 *
 * @param type      type name
 * @param args      integer of the specified type
 * @return          successfully created object, NULL on failure
 */
metadata_t *metadata_create_int(const char *type, va_list args);

#endif /** METADATA_INT_H_ @}*/
