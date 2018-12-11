#ifndef _Noreturn
# if 201103 <= (defined __cplusplus ? __cplusplus : 0)
#  define _Noreturn [[noreturn]]
# elif (201112 <= (defined __STDC_VERSION__ ? __STDC_VERSION__ : 0) \
        || 4 < __GNUC__ + (7 <= __GNUC_MINOR__))
   /* _Noreturn works as-is.  */
# elif 2 < __GNUC__ + (8 <= __GNUC_MINOR__) || 0x5110 <= __SUNPRO_C
#  define _Noreturn __attribute__ ((__noreturn__))
# elif 1200 <= (defined _MSC_VER ? _MSC_VER : 0)
#  define _Noreturn __declspec (noreturn)
# else
#  define _Noreturn
# endif
#endif
