/*
 * sha512.h - interface to mbedTLS SHA512 hash function.
 *
 * Copyright 2017 Google Inc.
 * Author: Joe Richey (joerichey@google.com)
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this file except in compliance with the License. You may obtain a copy of
 * the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations under
 * the License.
 */

#ifndef SHA512_H
#define SHA512_H

#include <stddef.h>
#include <stdint.h>

#define SHA512_DIGEST_LENGTH 64

extern void SHA512(const uint8_t* in, size_t n,
                   uint8_t out[SHA512_DIGEST_LENGTH]);

// Zero the memory pointed to by v; this will not be optimized away.
extern void secure_wipe(uint8_t* v, uint32_t n);

#endif /* SHA512_H */
