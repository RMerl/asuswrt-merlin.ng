
#include "SkipDB.h"
#include <time.h>
#include <stdlib.h>
#include "Date.h"

int main(void)
{
	Datum k;
	int i, max;
	time_t t1;
	double dt; 
	char s[100];
		
	SkipDB *u = SkipDB_new();
	
	printf("SkipDB (ordered key/value database) performance test:\n");
	SkipDB_setPath_(u, "test.skipdb");
	SkipDB_delete(u);
	SkipDB_open(u);
	
	max = 1;

	t1 = Date_SecondsFrom1970ToNow();
	SkipDB_beginTransaction(u);
	for (i = 0; i < max; i ++)
	{
		sprintf(s, "%i", i);
		k = Datum_FromCString_(s);
		
		SkipDB_at_put_(u, k, k);
	}
	SkipDB_commitTransaction(u);
	dt = Date_SecondsFrom1970ToNow()-t1;;
	printf("	%i individual sequential transactional writes per second\n", (int)((double)max/dt));

	SkipDB_close(u);
	SkipDB_open(u);
	
	t1 = Date_SecondsFrom1970ToNow();
	for (i = 0; i < max; i ++)
	{
		Datum d;
		sprintf(s, "%i", i);
		k = Datum_FromCString_(s);
				
		d = SkipDB_at_(u, k);

		if (d.data == NULL || d.size == 0)
		{
			printf("ERROR: no record at %i\n", i);
			exit(-1);
		}
		else
		{
			if (strcmp(s, (char *)d.data) != 0)
			{
				printf("ERROR: bad value at %i\n", i);
				exit(-1);
			}
		}
	}
	printf("writes verified\n");
	
	printf("testing removes\n");
	SkipDB_beginTransaction(u);
	for (i = 0; i < max; i ++)
	{
		sprintf(s, "%i", i);
		k = Datum_FromCString_(s);
		SkipDB_removeAt_(u, k);
	}
	//SkipDB_show(u);
	SkipDB_commitTransaction(u);

	SkipDB_close(u);
	printf("closing after removes\n");
	SkipDB_open(u);
	
	t1 = Date_SecondsFrom1970ToNow();
	for (i = 0; i < max; i ++)
	{
		Datum d;
		sprintf(s, "%i", i);
		k = Datum_FromCString_(s);
				
		d = SkipDB_at_(u, k);

		if (d.data == NULL || d.size == 0)
		{
		}
		else
		{
			printf("ERROR: record %i not removed\n", i);
			exit(-1);
		}
	}
	printf("removes verified\n");
	
	SkipDB_close(u);
	SkipDB_delete(u);
	SkipDB_open(u);

	max = 1024*64;
	
	t1 = Date_SecondsFrom1970ToNow();
	SkipDB_beginTransaction(u);
	for (i = 0; i < max; i ++)
	{
		sprintf(s, "%i", i);
		k = Datum_FromCString_(s);

		SkipDB_at_put_(u, k, k);
	}
	SkipDB_commitTransaction(u);
	dt = Date_SecondsFrom1970ToNow()-t1;;
	printf("	%i group transactional writes per second\n", (int)((double)max/dt));


	t1 = Date_SecondsFrom1970ToNow();
	for (i = 0; i < max; i ++)
	{
		Datum d;
		k = Datum_FromCString_(s);
				
		d = SkipDB_at_(u, k);

		if (d.data == NULL || d.size == 0)
		{
			printf("ERROR: no record at %i\n", i);
			exit(-1);
		}
		else
		{
			if (strcmp(s, (char *)d.data) != 0)
			{
				printf("ERROR: bad value at %i\n", i);
				exit(-1);
			}
		}
	}
	dt = Date_SecondsFrom1970ToNow()-t1;;
	printf("	%i reads per second\n", (int)((float)max/dt));
	
	
	t1 = Date_SecondsFrom1970ToNow();
	SkipDB_beginTransaction(u);
	for (i = 0; i < max; i ++)
	{
		sprintf(s, "%i", i);
		k = Datum_FromCString_(s);
				
		SkipDB_removeAt_(u, k);
	}
	SkipDB_commitTransaction(u);
	dt = Date_SecondsFrom1970ToNow()-t1;;
	printf("	%i group transactional removes per second\n", (int)((float)max/dt));
	
	SkipDB_delete(u);
	SkipDB_free(u);
	return 0;
}
