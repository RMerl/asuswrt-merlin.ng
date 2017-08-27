#ifndef SELINUX_CONTEXT_H
# define SELINUX_CONTEXT_H

# include <errno.h>

/* The definition of _GL_UNUSED_PARAMETER is copied here.  */

typedef int context_t;
static inline context_t context_new (char const *s _GL_UNUSED_PARAMETER)
  { errno = ENOTSUP; return 0; }
static inline char *context_str (context_t con _GL_UNUSED_PARAMETER)
  { errno = ENOTSUP; return (void *) 0; }
static inline void context_free (context_t c _GL_UNUSED_PARAMETER) {}

static inline int context_user_set (context_t sc _GL_UNUSED_PARAMETER,
                                    char const *s _GL_UNUSED_PARAMETER)
  { errno = ENOTSUP; return -1; }
static inline int context_role_set (context_t sc _GL_UNUSED_PARAMETER,
                                    char const *s _GL_UNUSED_PARAMETER)
  { errno = ENOTSUP; return -1; }
static inline int context_range_set (context_t sc _GL_UNUSED_PARAMETER,
                                     char const *s _GL_UNUSED_PARAMETER)
  { errno = ENOTSUP; return -1; }
static inline int context_type_set (context_t sc _GL_UNUSED_PARAMETER,
                                    char const *s _GL_UNUSED_PARAMETER)
  { errno = ENOTSUP; return -1; }

#endif
