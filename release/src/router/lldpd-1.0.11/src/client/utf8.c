/* -*- mode: c; c-file-style: "openbsd" -*- */
/*
  Copyright (c) 2011 Joseph A. Adams (joeyadams3.14159@gmail.com)

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/

#include <stddef.h>

/*
 * Validate a single UTF-8 character starting at @s.
 * The string must be null-terminated.
 *
 * If it's valid, return its length (1 thru 4).
 * If it's invalid or clipped, return 0.
 *
 * This function implements the syntax given in RFC3629, which is
 * the same as that given in The Unicode Standard, Version 6.0.
 *
 * It has the following properties:
 *
 *  * All codepoints U+0000..U+10FFFF may be encoded,
 *    except for U+D800..U+DFFF, which are reserved
 *    for UTF-16 surrogate pair encoding.
 *  * UTF-8 byte sequences longer than 4 bytes are not permitted,
 *    as they exceed the range of Unicode.
 *  * The sixty-six Unicode "non-characters" are permitted
 *    (namely, U+FDD0..U+FDEF, U+xxFFFE, and U+xxFFFF).
 */
size_t
utf8_validate_cz(const char *s)
{
        unsigned char c = *s++;

        if (c <= 0x7F) {        /* 00..7F */
                return 1;
        } else if (c <= 0xC1) { /* 80..C1 */
                /* Disallow overlong 2-byte sequence. */
                return 0;
        } else if (c <= 0xDF) { /* C2..DF */
                /* Make sure subsequent byte is in the range 0x80..0xBF. */
                if (((unsigned char)*s++ & 0xC0) != 0x80)
                        return 0;

                return 2;
        } else if (c <= 0xEF) { /* E0..EF */
                /* Disallow overlong 3-byte sequence. */
                if (c == 0xE0 && (unsigned char)*s < 0xA0)
                        return 0;

                /* Disallow U+D800..U+DFFF. */
                if (c == 0xED && (unsigned char)*s > 0x9F)
                        return 0;

                /* Make sure subsequent bytes are in the range 0x80..0xBF. */
                if (((unsigned char)*s++ & 0xC0) != 0x80)
                        return 0;
                if (((unsigned char)*s++ & 0xC0) != 0x80)
                        return 0;

                return 3;
        } else if (c <= 0xF4) { /* F0..F4 */
                /* Disallow overlong 4-byte sequence. */
                if (c == 0xF0 && (unsigned char)*s < 0x90)
                        return 0;

                /* Disallow codepoints beyond U+10FFFF. */
                if (c == 0xF4 && (unsigned char)*s > 0x8F)
                        return 0;

                /* Make sure subsequent bytes are in the range 0x80..0xBF. */
                if (((unsigned char)*s++ & 0xC0) != 0x80)
                        return 0;
                if (((unsigned char)*s++ & 0xC0) != 0x80)
                        return 0;
                if (((unsigned char)*s++ & 0xC0) != 0x80)
                        return 0;

                return 4;
        } else {                /* F5..FF */
                return 0;
        }
}
