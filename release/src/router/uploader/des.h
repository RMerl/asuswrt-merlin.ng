#include "stdio.h"
#include "memory.h"
#include "time.h"
#include "stdlib.h"


int des_encrypt(const char* datain, unsigned char* dataout, int * dataout_len, const unsigned char* keyin, int keyin_len);
char * keyProcess(const char* keyin);
