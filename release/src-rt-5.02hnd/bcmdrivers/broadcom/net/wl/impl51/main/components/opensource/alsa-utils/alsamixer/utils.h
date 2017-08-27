#ifndef UTILS_H_INCLUDED
#define UTILS_H_INCLUDED

#define ARRAY_SIZE(a) (sizeof(a) / sizeof *(a))

unsigned int get_mbs_width(const char *s);
unsigned int get_max_mbs_width(const char *const *s, unsigned int count);
const char *mbs_at_width(const char *s, int *width, int dir);

#endif
