/*
 * Dropbear SSH
 * 
 * Copyright (c) 2002,2003 Matt Johnston
 * Copyright (c) 2020 by Vladislav Grishenko
 * All rights reserved.
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
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE. */

#ifndef DROPBEAR_DROPBEAR_GCM_H_
#define DROPBEAR_DROPBEAR_GCM_H_

#include "includes.h"
#include "algo.h"

#if DROPBEAR_ENABLE_GCM_MODE

#define GCM_IVFIX_LEN 4
#define GCM_IVCTR_LEN 8
#define GCM_NONCE_LEN (GCM_IVFIX_LEN + GCM_IVCTR_LEN)

typedef struct {
	gcm_state gcm;
	unsigned char iv[GCM_NONCE_LEN];
} dropbear_gcm_state;

extern const struct dropbear_cipher_mode dropbear_mode_gcm;

#endif /* DROPBEAR_ENABLE_GCM_MODE */

#endif /* DROPBEAR_DROPBEAR_GCM_H_ */
