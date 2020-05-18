#include "SkipDB.h"

int main(void)
{
	Datum key;
	Datum value;
	Datum key2;
	Datum value2;
	int count;

	// open
	
	SkipDB *db = SkipDB_new();
	SkipDB_setPath_(db, "test.skipdb");
	if(0 == SkipDB_open(db)) {
	    SkipDB_beginTransaction(db);
            key = Datum_FromCString_("__inner__");
            value = Datum_FromCString_("__begin__");
	    SkipDB_commitTransaction(db);
            SkipDB_close(db);
            SkipDB_open(db);
        }
	
	// write
	
#if 0
	SkipDB_beginTransaction(db);
	key = Datum_FromCString_("testKey");
	value = Datum_FromCString_("testValue");
	SkipDB_at_put_(db, key, value);

	key2 = Datum_FromCString_("testKey2");
	value2 = Datum_FromCString_("testValue2");
	SkipDB_at_put_(db, key2, value2);

	SkipDB_commitTransaction(db);
#else
	key = Datum_FromCString_("testKey");
	key2 = Datum_FromCString_("testKey2");
	//key = Datum_FromCString_("testKey88");
	//key2 = Datum_FromCString_("testKey233");
#endif
        SkipDB_show(db);
	
	// read
	value = SkipDB_at_(db, key);
        printf("%s=%s\n", key.data, value.data);
	
	value2 = SkipDB_at_(db, key2);
        printf("%s=%s\n", key2.data, value2.data);

	// count
	
	count = SkipDB_count(db);

	// remove
	
	//SkipDB_beginTransaction(db);
	//SkipDB_removeAt_(db, key);
	//SkipDB_commitTransaction(db);
	
	// there's also a cursor API
	// not shown in this example code

	// close
	
	SkipDB_close(db);
	
	return 0;
}
