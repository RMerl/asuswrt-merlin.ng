/*
 * Dropbear - a SSH2 server
 * 
 * Copyright (c) 2002,2003 Matt Johnston
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

#ifndef DROPBEAR_CURVE25519_H
#define DROPBEAR_CURVE25519_H

void dropbear_curve25519_scalarmult(unsigned char *q, const unsigned char *n, const unsigned char *p);
void dropbear_ed25519_make_key(unsigned char *pk, unsigned char  *sk);
void dropbear_ed25519_sign(const unsigned char *m, unsigned long mlen,
			  unsigned char *s, unsigned long *slen,
			  const unsigned char *sk, const unsigned char *pk);
int dropbear_ed25519_verify(const unsigned char *m, unsigned long mlen,
			    const unsigned char *s, unsigned long slen,
			    const unsigned char *pk);

#endif /* DROPBEAR_CURVE25519_H */
