/* source: utils.h */
/* Copyright Gerhard Rieger and contributors (see file CHANGES) */
/* Published under the GNU General Public License V.2, see file COPYING */

#ifndef __utils_h_included
#define __utils_h_included 1

/* a generic name table entry */
struct wordent {
   const char *name;
   void *desc;
} ;

#if !HAVE_PROTOTYPE_LIB_memrchr
extern void *memrchr(const void *s, int c, size_t n);
#endif
extern void *memdup(const void *src, size_t n);
#if !HAVE_SETENV
extern int setenv(const char *name, const char *value, int overwrite);
#endif /* !HAVE_SETENV */

extern const struct wordent *keyw(const struct wordent *keywds, const char *name, unsigned int nkeys);


#define XIOSAN_ZERO_MASK                  0x000f
#define XIOSAN_ZERO_DEFAULT               0x0000
#define XIOSAN_ZERO_DOT                   0x0001
#define XIOSAN_ZERO_BACKSLASH_OCT_3       0x0002
#define XIOSAN_ZERO_BACKSLASH_OCT_4       0x0003
#define XIOSAN_ZERO_BACKSLASHX_HEX_UP     0x0004
#define XIOSAN_ZERO_BACKSLASHX_HEX_LOW    0x0005
#define XIOSAN_ZERO_PERCENT_HEX_UP        0x0006
#define XIOSAN_ZERO_PERCENT_HEX_LOW       0x0007
#define XIOSAN_CONTROL_MASK               0x00f0
#define XIOSAN_CONTROL_DEFAULT            0x0000
#define XIOSAN_CONTROL_DOT                0x0010
#define XIOSAN_CONTROL_BACKSLASH_OCT_3    0x0020
#define XIOSAN_CONTROL_BACKSLASH_OCT_4    0x0030
#define XIOSAN_CONTROL_BACKSLASHX_HEX_UP  0x0040
#define XIOSAN_CONTROL_BACKSLASHX_HEX_LOW 0x0050
#define XIOSAN_CONTROL_PERCENT_HEX_UP     0x0060
#define XIOSAN_CONTROL_PERCENT_HEX_LOW    0x0070
#define XIOSAN_UNPRINT_MASK               0x0f00
#define XIOSAN_UNPRINT_DEFAULT            0x0000
#define XIOSAN_UNPRINT_DOT                0x0100
#define XIOSAN_UNPRINT_BACKSLASH_OCT_3    0x0200
#define XIOSAN_UNPRINT_BACKSLASH_OCT_4    0x0300
#define XIOSAN_UNPRINT_BACKSLASHX_HEX_UP  0x0400
#define XIOSAN_UNPRINT_BACKSLASHX_HEX_LOW 0x0500
#define XIOSAN_UNPRINT_PERCENT_HEX_UP     0x0600
#define XIOSAN_UNPRINT_PERCENT_HEX_LOW    0x0700
#define XIOSAN_DEFAULT_MASK               0xf000
#define XIOSAN_DEFAULT_BACKSLASH_DOT      0x1000
#define XIOSAN_DEFAULT_BACKSLASH_OCT_3    0x2000
#define XIOSAN_DEFAULT_BACKSLASH_OCT_4    0x3000
#define XIOSAN_DEFAULT_BACKSLASHX_HEX_UP  0x4000
#define XIOSAN_DEFAULT_BACKSLASHX_HEX_LOW 0x5000
#define XIOSAN_DEFAULT_PERCENT_HEX_UP     0x6000
#define XIOSAN_DEFAULT_PERCENT_HEX_LOW    0x7000

extern
char *sanitize_string(const char *data,	/* input data */
		   size_t bytes,	/* length of input data, >=0 */
		   char *coded,	/* output buffer, must be long enough */
		   int style);
extern
char *xiosubstr(char *scratch, const char *str, size_t from, size_t len);

extern
int xio_snprintf(char *str, size_t size, const char *format, ...);

#endif /* !defined(__utils_h_included) */
