#ifndef LIBICAL_ICAL_EXPORT_H
#define LIBICAL_ICAL_EXPORT_H

#if !defined(S_SPLINT_S)

#ifdef LIBICAL_ICAL_STATIC_DEFINE
#define LIBICAL_ICAL_EXPORT
#define LIBICAL_ICAL_NO_EXPORT
#else
#if defined(_MSC_VER)
#if defined(libical_ical_EXPORTS)
       /* We are building this library */
#define LIBICAL_ICAL_EXPORT __declspec(dllexport)
#else
       /* We are using this library */
#define LIBICAL_ICAL_EXPORT __declspec(dllimport)
#endif
#define LIBICAL_ICAL_NO_EXPORT
#else
#define LIBICAL_ICAL_EXPORT __attribute__((visibility("default")))
#define LIBICAL_ICAL_NO_EXPORT __attribute__((visibility("hidden")))
#endif
#endif

#endif

#endif
