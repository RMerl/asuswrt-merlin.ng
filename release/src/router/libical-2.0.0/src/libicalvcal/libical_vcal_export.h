#ifndef LIBICAL_VCAL_EXPORT_H
#define LIBICAL_VCAL_EXPORT_H

#if !defined(S_SPLINT_S)

#ifdef LIBICAL_VCAL_STATIC_DEFINE
#define LIBICAL_VCAL_EXPORT
#define LIBICAL_VCAL_NO_EXPORT
#else
#if defined(_MSC_VER)
#if defined(libical_vcal_EXPORTS)
       /* We are building this library */
#define LIBICAL_VCAL_EXPORT __declspec(dllexport)
#else
       /* We are using this library */
#define LIBICAL_VCAL_EXPORT __declspec(dllimport)
#endif
#define LIBICAL_VCAL_NO_EXPORT
#else
#define LIBICAL_VCAL_EXPORT __attribute__((visibility("default")))
#define LIBICAL_VCAL_NO_EXPORT __attribute__((visibility("hidden")))
#endif
#endif

#endif

#endif
