#include <sysdeps/amas/amas_dwb.h>

#define WL_KEY_LEN              10
#define WL_SSID_RANDOM_LEN      4

extern int cm_dwbIsEnabled();

#ifndef IsNULL_PTR
#define IsNULL_PTR(__PTR) ((__PTR == NULL))
#endif  /* !IsNULL_PTR */