#ifndef LIBICAL_ICALSS_EXPORT_H
#define LIBICAL_ICALSS_EXPORT_H

#if !defined(S_SPLINT_S)

#ifdef LIBICAL_ICALSS_STATIC_DEFINE
#define LIBICAL_ICALSS_EXPORT
#define LIBICAL_ICALSS_NO_EXPORT
#else
#if defined(_MSC_VER)
#if defined(libical_icalss_EXPORTS)
       /* We are building this library */
#define LIBICAL_ICALSS_EXPORT __declspec(dllexport)
#else
       /* We are using this library */
#define LIBICAL_ICALSS_EXPORT __declspec(dllimport)
#endif
#define LIBICAL_ICALSS_NO_EXPORT
#else
#define LIBICAL_ICALSS_EXPORT __attribute__((visibility("default")))
#define LIBICAL_ICALSS_NO_EXPORT __attribute__((visibility("hidden")))
#endif
#endif

#endif

#endif
