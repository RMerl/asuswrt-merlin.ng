/*
 * Implementations of Broadcom secure string functions for non-Windows platforms
 *
 * Copyright(c) 2010 Broadcom Corp.
 * $Id: nonms_str.h,v 1.1.2.3 2010-09-01 01:39:11 $
 */

#ifndef __BCM_STR_H__
#define __BCM_STR_H__

#define bcm_strcpy_s(dst, noOfElements, src) \
	strcpy((dst), (src))
#define bcm_strncpy_s(dst, noOfElements, src, count) \
	strncpy((dst), (src), (count))
#define bcm_strcat_s(dst, noOfElements, src) \
	strcat((dst), (src))
#define bcm_sprintf_s(buffer, noOfElements, format, ...) \
	sprintf((buffer), (format) , ## __VA_ARGS__)

#endif /* __BCM_STR_H__ */
