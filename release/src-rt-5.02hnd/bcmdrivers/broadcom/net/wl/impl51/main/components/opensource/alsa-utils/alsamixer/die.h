#ifndef DIE_H_INCLUDED
#define DIE_H_INCLUDED

void fatal_error(const char *msg) __attribute__((__noreturn__));
void fatal_alsa_error(const char *msg, int err) __attribute__((__noreturn__));

#endif
