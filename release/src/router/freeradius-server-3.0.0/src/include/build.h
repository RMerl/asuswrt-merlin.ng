/*
 * $Id$
 *
 * @brief Source control functions
 *
 * @copyright 2013 The FreeRADIUS server project
 */
#ifdef __cplusplus
extern "C" {
#endif

/*
 *	The ubiquitous stringify macros
 */
#define XSTRINGIFY(x) #x
#define STRINGIFY(x) XSTRINGIFY(x)

/*
 *	Macros for controlling warnings in GCC >= 4.2 and clang >= 2.8
 */
#define DIAG_JOINSTR(x,y) XSTRINGIFY(x ## y)
#define DIAG_DO_PRAGMA(x) _Pragma (#x)

#if defined(__GNUC__) && ((__GNUC__ * 100) + __GNUC_MINOR__) >= 402
#  define DIAG_PRAGMA(x) DIAG_DO_PRAGMA(GCC diagnostic x)
#  if ((__GNUC__ * 100) + __GNUC_MINOR__) >= 406
#    define DIAG_OFF(x) DIAG_PRAGMA(push) DIAG_PRAGMA(ignored DIAG_JOINSTR(-W,x))
#    define DIAG_ON(x) DIAG_PRAGMA(pop)
#  else
#    define DIAG_OFF(x) DIAG_PRAGMA(ignored DIAG_JOINSTR(-W,x))
#    define DIAG_ON(x)  DIAG_PRAGMA(warning DIAG_JOINSTR(-W,x))
#  endif
#elif defined(__clang__) && ((__clang_major__ * 100) + __clang_minor__ >= 208)
#  define DIAG_PRAGMA(x) DIAG_DO_PRAGMA(clang diagnostic x)
#  define DIAG_OFF(x) DIAG_PRAGMA(push) DIAG_PRAGMA(ignored DIAG_JOINSTR(-W,x))
#  define DIAG_ON(x) DIAG_PRAGMA(pop)
#else
#  define DIAG_OFF(x)
#  define DIAG_ON(x)
#endif

/*
 *	For dealing with APIs which are only deprecated in OSX (like the OpenSSL API)
 */
#ifdef __APPLE__
#  define USES_APPLE_DEPRECATED_API DIAG_OFF(deprecated-declarations)
#  define USES_APPLE_RST DIAG_ON(deprecated-declarations)
#else
#  define USES_APPLE_DEPRECATED_API
#  define USES_APPLE_RST
#endif

#if defined(__GNUC__)
/* force inclusion of ident keywords in the face of optimization */
#define RCSID(id) static char const rcsid[] __attribute__ ((used)) = id;
#define RCSIDH(h, id) static char const rcsid_ ## h [] __attribute__ ((used)) = id;
#elif defined(__SUNPRO_C)
/* put ident keyword into comment section (nicer than gcc way) */
#define DO_PRAGMA(x) _Pragma(#x)
#define RCSID(id) DO_PRAGMA(sun ident id)
#define RCSIDH(h, id) DO_PRAGMA(sun ident id)
#else
#define RCSID(id)
#define RCSIDH(h, id)
#endif

#ifdef __cplusplus
}
#endif
