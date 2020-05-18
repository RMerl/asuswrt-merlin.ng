#include <time.h>
#include <stdlib.h>
#include "UDB.h"
#include "Date.h"

int main(void)
{
    int max, i, v;
    PID_TYPE pid;
    Datum d;
    UDB *u = UDB_new();
    UDB_setPath_(u, "test.skipdb");
    //UDB_delete(u);
    UDB_open(u);

    max = 30;

#if 0
    for (i = 0; i < max; i++) {
            UDB_beginTransaction(u);
            d.size = sizeof(int);
            v = i;
            d.data = (void *)&v;
            
            pid = UDB_allocPid(u);
            fprintf(stderr, "pid=%d\n", pid);
            UDB_at_put_(u, pid, d);
            UDB_commitTransaction(u);
    }
#endif

#if 0
    for (i = 0; i < max; i++) {
        pid = i+1;
        d = UDB_at_(u, pid);

        if (d.data == NULL || d.size == 0) {
            printf("ERROR: no record at %i\n", pid);
        } else {
            if(d.size != 4) {
                printf("ERROR: wrong size %i at pid %i\n", d.size, pid);
            } else {
                memcpy(&v, d.data, sizeof(int));
                if(v != i) {
                    printf("ERROR: wrong value %i at pid %i\n", v, i);
                }
            }
        }
    }
#endif

    pid = 1;
    d = UDB_at_(u, pid);
    printf("size:%d %s\n", d.size, d.data);

    pid = 2;
    d = UDB_at_(u, pid);
    printf("size:%d %s\n", d.size, d.data);

    pid = 3;
    d = UDB_at_(u, pid);
    printf("size:%d %s\n", d.size, d.data);

    return 0;
}

