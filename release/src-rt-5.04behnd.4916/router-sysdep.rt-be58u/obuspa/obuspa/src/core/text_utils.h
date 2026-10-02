/*
 *
 * Copyright (C) 2019-2022, Broadband Forum
 * Copyright (C) 2016-2022  CommScope, Inc
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

/**
 * \file text_utils.h
 *
 * Header file for API to convert and validate types from strings
 *
 */
#ifndef TEXT_UTILS_H
#define TEXT_UTILS_H

#include "str_vector.h"
#include "nu_ipaddr.h"
#include "data_model.h"  // for dm_hash_t

//-------------------------------------------------------------------------
// Macros associated with performing the FNV1a hashing algorithm
#define OFFSET_BASIS (0x811C9DC5)
#define FNV_PRIME (0x1000193)
#define ADD_TO_HASH(c, hash)  hash = hash*FNV_PRIME;  hash = hash^(c);

//-------------------------------------------------------------------------
// Definitions for bits within flags argument of TEXT_UTILS_PercentEncodeString() and TEXT_UTILS_ValueToHexDigit()
#define USE_UPPERCASE_HEX_DIGITS 0x00000000
#define USE_LOWERCASE_HEX_DIGITS 0x00000001

//-------------------------------------------------------------------------
// API functions
dm_hash_t TEXT_UTILS_CalcHash(char *s);
int TEXT_UTILS_StringToUnsigned(char *str, unsigned *value);
int TEXT_UTILS_StringToInteger(char *str, int *value);
int TEXT_UTILS_StringToUnsignedLongLong(char *str, unsigned long long *value);
int TEXT_UTILS_StringToLongLong(char *str, long long *value);
int TEXT_UTILS_StringToDouble(char *str, double *value);
int TEXT_UTILS_StringToBool(char *str, bool *value);
char *TEXT_UTILS_BoolToString(bool value);
int TEXT_UTILS_StringToEnum(char *str, const enum_entry_t *enums, int num_enums);
char *TEXT_UTILS_EnumToString(int value, const enum_entry_t *enums, int num_enums);
int TEXT_UTILS_StringToDateTime(char *str, time_t *value);
int TEXT_UTILS_StringToBinary(char *str, unsigned char *buf, int len, int *bytes_written);
int TEXT_UTILS_Base64StringToBinary(char *str, unsigned char *buf, int len, int *bytes_written);
int TEXT_UTILS_StringToIpAddr(char *str, nu_ipaddr_t *ip_addr);
char *TEXT_UTILS_SplitPath(char *path, char *buf, int len);
void TEXT_UTILS_ListToString(char **items, int num_items, char *buf, int len);
void TEXT_UTILS_SplitString(char *str, str_vector_t *sv, char *separator);
void TEXT_UTILS_StrncpyLen(char *dst, int dst_len, char *src, int src_len);
char *TEXT_UTILS_StrStr(char *haystack, char *needle);
int TEXT_UTILS_NullStringCompare(char *str1, char *str2);
void TEXT_UTILS_PercentEncodeString(char *src, char *dst, int dst_len, char *safe_chars, unsigned flags);
void TEXT_UTILS_PercentDecodeString(char *buf);
void TEXT_UTILS_ReplaceCharInString(char *src, char match_char, char *replacement, char *dst, int dst_len);
char *TEXT_UTILS_TrimBuffer(char *buf);
char *TEXT_UTILS_TrimDelimitedBuffer(char *buf, char *delimiters);
void TEXT_UTILS_StripChars(char *strip, char *src, char *dest, int dest_len);
int TEXT_UTILS_HexStringToValue(char *s);
int TEXT_UTILS_HexDigitToValue(char c);
char TEXT_UTILS_ValueToHexDigit(int nibble, unsigned flags);
void TEXT_UTILS_PathToSchemaForm(char *path, char *buf, int len);
int TEXT_UTILS_CountConsecutiveDigits(char *p);
char *TEXT_UTILS_StrDupWithTrailingDot(char *path);
int TEXT_UTILS_KeyValueFromString(char *buf, char **key, char **value);

#endif
