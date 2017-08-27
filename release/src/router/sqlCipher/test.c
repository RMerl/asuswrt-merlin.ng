#include <stdio.h>
#include "sqlite3.h"
#include <string.h>

#define SQLITEAPI(x)							\
{									\
	int ret=0;							\
   	char *errMsg = NULL;						\
	ret=sqlite_##x;							\
	if(ret != SQLITE_OK)						\
        	printf("SQL error: %s, %s\n", #x,sqlite3_errmsg (db));	\
}																	


static char *createsql_acc_user = "CREATE TABLE radcheck("
               		 "ID VARCHAR(32) NOT NULL UNIQUE,"
               		 "PASSWD TEXT NOT NULL,"
			 "xxx INTEGER NOT NULL,"
			 "FOREIGN KEY (xxx) REFERENCES radgroup (GID) ON UPDATE CASCADE ON DELETE CASCADE);";
	//		 "group_id INTEGER REFERENCES radgroup(GID));";

static char *createsql_acc_group = "CREATE TABLE radgroup("
               		 "GID INTEGER PRIMARY KEY,"
			 "GName TEXT NOT NULL UNIQUE,"
               		 "Service TEXT,"
               		 "Attr TEXT);";

static char *createsql_dev_user = "CREATE TABLE Device("
               		 "Name VARCHAR(32) NOT NULL UNIQUE,"
               		 "MAC TEXT NOT NULL,"
               		 "Device_Type INT NOT NULL,"
			 "ref INTEGER NOT NULL,"
			 "FOREIGN KEY (ref) REFERENCES Device_group (GID) ON UPDATE CASCADE ON DELETE CASCADE);";
	//		 "group_id INTEGER REFERENCES radgroup(GID));";

static char *createsql_dev_group = "CREATE TABLE Device_group("
               		 "GID INTEGER PRIMARY KEY,"
			 "GName TEXT NOT NULL UNIQUE,"
               		 "Service TEXT,"
               		 "Attr TEXT);";

static char *enable_foreign="PRAGMA foreign_keys = ON;";

//static char *insertsql = "INSERT INTO Contact VALUES(NULL, 'Fred', '09990123456');";

//static char *querysql = "SELECT * FROM Contact;";


int sqlite_getTable(sqlite3 *db, char *tableName, int *row, int *col, char ***result)
{
	char sql_str[128];
	char *errMsg;
	int ret=0;
	memset(sql_str, 0, sizeof(sql_str));
	sprintf(sql_str, "select * from %s", tableName);
	ret=sqlite3_get_table(db, sql_str, result, row, col, &errMsg);
	if(errMsg!=NULL)
	  sqlite3_free(errMsg);
	return ret;

}

int sqlite_freeTable(char **result)
{
	char sql_str[128];
	char *errMsg;
	int ret=0;
	sqlite3_free_table(result);
	return ret;

}


int sqlite_insert(sqlite3 *db, char *username, char *passwd, char *group_name)
{
	char sql_str[128];
	int ret=0;
	char *errMsg;
	memset(sql_str, 0, sizeof(sql_str));
	sprintf(sql_str, "REPLACE INTO radcheck VALUES('%s', '%s', (select GID from radgroup where GName='%s'));", username, passwd, group_name);
	ret=sqlite3_exec(db, sql_str, 0, 0, &errMsg);
	if(errMsg!=NULL)
	  sqlite3_free(errMsg);
	return ret;
}

int sqlite_insert_group(sqlite3 *db, char *groupName, char *service, char *Attr)
{
	char sql_str[128];
	int ret=0;
	char *errMsg;
	memset(sql_str, 0, sizeof(sql_str));
	service="NULL";
	Attr="NULL";
	char *id="NULL";
	sprintf(sql_str, "PEPLACE INTO radgroup VALUES(%s, '%s', %s, %s);", id, groupName, service, Attr);
	printf("sql_str=%s\n", sql_str);
	ret=sqlite3_exec(db, sql_str, 0, 0, &errMsg);
	if(errMsg!=NULL)
	  sqlite3_free(errMsg);
	return ret;
}

int sqlite_delete(sqlite3 *db, char *username)
{
	char sql_str[128];
	int ret=0;
	char *errMsg;
	memset(sql_str, 0, sizeof(sql_str));
	sprintf(sql_str, "DELETE from radcheck WHERE ID='%s'", username);
	ret=sqlite3_exec(db, sql_str, 0, 0, &errMsg);
	if(errMsg!=NULL)
	  sqlite3_free(errMsg);
	return ret;
}

int sqlite_create(sqlite3 *db)
{
	int ret=0;
	char *errMsg;
	ret=sqlite3_exec(db, createsql_group, 0, 0, &errMsg);
	if(errMsg!=NULL)
	  sqlite3_free(errMsg);
	printf("down for group\n");
	sqlite3_exec(db, enable_foreign, 0, 0, &errMsg);	
	ret|=sqlite3_exec(db, createsql_user, 0, 0, &errMsg);
	if(errMsg!=NULL)
	  sqlite3_free(errMsg);

	return ret;
}

int sqlite_open(char *path, sqlite3 **db, sqlite3 *db_key)
{
	int ret=0;
	if(path == NULL)
	   return -1;
	//char *key="12345";	
	ret= sqlite3_open_v2(path, db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
        printf("[%d]\n", __LINE__);
	//sqlite3_key(db_key, key, sizeof(key));
        printf("[%d]\n", __LINE__);
	return ret;
}

int sqlite_close(sqlite3 *db)
{
	int ret=0;
	ret= sqlite3_close_v2(db);
	return ret;
}



void main(void)
{
   int rows, cols;
   sqlite3 *db;
   char *errMsg = NULL;
   char **result;


   printf("Start...\n");
   //sqlite3_open_v2("example.db", &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
   SQLITEAPI(open("example.db3", &db, db));
  // char *key="12345";	
   //sqlite3_key(db, key, sizeof(key));


   char *test="eeee";
  
   SQLITEAPI(create(db));   
   
   SQLITEAPI(insert_group(db,"SW", NULL, NULL));   
   SQLITEAPI(insert_group(db,"HW", "NULL", "NULL"));   

   SQLITEAPI(insert(db,"John", "12312","SW"));   
   SQLITEAPI(insert(db,"Chris", "1xx12312", "SW"));   
   SQLITEAPI(insert(db,"John", "123","SW"));   
   
    
   SQLITEAPI(getTable(db, "radcheck", &rows, &cols, &result)); 

   int i=0, j=0;
   
   for (i=1;i<=rows;i++) {
       for (j=0;j<cols;j++) {
           printf("%s\t", result[i*cols+j]);
       }
       printf("\n");
   }
        
   SQLITEAPI(freeTable(result));
   
   SQLITEAPI(close(db));
   //sqlite3_close_v2(db);
}
