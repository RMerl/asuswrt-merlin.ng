#ifndef DROPBEAR_GENSIGNKEY_H
#define DROPBEAR_GENSIGNKEY_H

#include "signkey.h"

int signkey_generate(enum signkey_type type, int bits, const char* filename, int skip_exist);
int signkey_generate_get_bits(enum signkey_type keytype, int bits);

#endif
