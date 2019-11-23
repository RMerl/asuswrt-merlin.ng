/* source: vsnprintf_r.h */
/* Copyright Gerhard Rieger and contributors (see file CHANGES) */
/* Published under the GNU General Public License V.2, see file COPYING */

#ifndef __vsnprintf_r_h_included
#define __vsnprintf_r_h_included 1

int vsnprintf_r(char *str, size_t size, const char *format, va_list ap);
int snprintf_r(char *str, size_t size, const char *format, ...);

#endif /* !defined(__vsnprintf_r_h_included) */
