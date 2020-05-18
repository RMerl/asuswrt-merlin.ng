#include <stdlib.h>
#include <time.h>
#include "JFile.h"
#include "List.h"
#include "PortableGettimeofday.h"
#include "Date.h"

int main(void)
{
    int value = 444, nv;
    JFile *j = JFile_new();
    JFile_setPath_(j, "test.jfile");

#if 0
    JFile_remove(j);
    JFile_open(j);

    JFile_begin(j);
    JFile_setPosition_(j, 0);
    JFile_writeInt_(j, value);
    JFile_commitToLog(j);
    JFile_commitToFile(j);
#else
    JFile_open(j);
#endif

    JFile_setPosition_(j, 0);
    printf("%d\n", JFile_readInt(j));
}

