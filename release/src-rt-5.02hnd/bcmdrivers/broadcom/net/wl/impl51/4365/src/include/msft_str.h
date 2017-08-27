/*
 * Windows implementations of Broadcom secure string functions
 *
 * Copyright(c) 2010 Broadcom Corp.
 * $Id: msft_str.h 278681 2011-08-19 17:50:47Z $
 */

#ifndef __MSFT_STR_H__
#define __MSFT_STR_H__

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif


int strcpy_s(char *dst, size_t noOfElements, const char *src);
int strncpy_s(char *dst, size_t noOfElements, const char *src, size_t count);
int strcat_s(char *dst, size_t noOfElements, const char *src);
int sprintf_s(char *buffer, size_t noOfElements, const char *format, ...);
int vsprintf_s(char *buffer, size_t noOfElements, const char *format, va_list argptr);
int _vsnprintf_s(char *buffer, size_t noOfElements, size_t count, const char *format,
	va_list argptr);
int wcscpy_s(wchar_t *dst, size_t noOfElements, const wchar_t *src);
int wcscat_s(wchar_t *dst, size_t noOfElements, const wchar_t *src);


static INLINE int bcm_strcpy_s(
	char *dst,
	size_t noOfElements,
	const char *src)
{
	int ret;

	ret = (int)strcpy_s(dst, noOfElements, src);
	return ret;
}

static INLINE int bcm_strncpy_s(
	char *dst,
	size_t noOfElements,
	const char *src,
	size_t count)
{
	return (int)strncpy_s(dst, noOfElements, src, count);
}

static INLINE int bcm_strcat_s(
	char *dst,
	size_t noOfElements,
	const char *src)
{
	int ret;

	ret = (int)strcat_s(dst, noOfElements, src);
	return ret;
}

static INLINE int bcm_sprintf_s(
	char *buffer,
	size_t noOfElements,
	const char *format,
	...)
{
	va_list argptr;
	int ret;

	va_start(argptr, format);
	ret = vsprintf_s(buffer, noOfElements, format, argptr);
	va_end(argptr);
	return ret;
}

static INLINE int bcm_vsprintf_s(
	char *buffer,
	size_t noOfElements,
	const char *format,
	va_list argptr)
{
	return vsprintf_s(buffer, noOfElements, format, argptr);
}

static INLINE int bcm_vsnprintf_s(
	char *buffer,
	size_t noOfElements,
	size_t count,
	const char *format,
	va_list argptr)
{
	return _vsnprintf_s(buffer, noOfElements, count, format, argptr);
}

static INLINE int bcm_wcscpy_s(
	wchar_t *dst,
	size_t noOfElements,
	const wchar_t *src)
{
	int ret;

	ret = (int)wcscpy_s(dst, noOfElements, src);
	return ret;
}

static INLINE int bcm_wcscat_s(
	wchar_t *dst,
	size_t noOfElements,
	const wchar_t *src)
{
	int ret;

	ret = (int)wcscat_s(dst, noOfElements, src);
	return ret;
}

#ifdef __cplusplus
}
#endif

#endif /* __MSFT_STR_H__ */
