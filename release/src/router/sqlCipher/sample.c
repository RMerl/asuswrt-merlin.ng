
#include "sqlite3.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "sqlite3APIs_2.h"

#if 0
#define SQLITEAPI(x)							\
{									\
	int ret=0;							\
   	char *errMsg = NULL;						\
	ret=sqlite_##x;							\
	if(ret != SQLITE_OK)						\
        	printf("SQL error: %s, %s\n", #x,sqlite3_errmsg (db));	\
}																	

#define TABLE_ACC_USER	 	0 
#define TABLE_ACC_GROUP    	1
#define TABLE_DEV_ACCOUNT 	2
#define TABLE_DEV_GROUP    	3 
#define TABLE_USER_GROUP   	4
#define TABLE_SERVICES		5


#define USER2GROUP 		0
#define GROUP2USER		1


static char *createsql_acc_user = "CREATE TABLE User("
               		 "ID INTEGER PRIMARY KEY,"
               		 "Name VARCHAR NOT NULL UNIQUE,"
               		 "PASSWD TEXT NOT NULL,"
			 "Des TEXT,"
			 "EMAIL TEXT,"
			 "MOD_RIGHT TEXT,"
			 "STOP_TIME TEXT);";

static char *createsql_acc_group = "CREATE TABLE User_group("
               		 "GID INTEGER PRIMARY KEY,"
			 "group_Name TEXT NOT NULL UNIQUE,"
               		 "Des TEXT);";

static char *createsql_dev_user = "CREATE TABLE Device("
               		 "ID INTEGER PRIMARY KEY,"
               		 "MAC TEXT NOT NULL,"
               		 "Des TEXT);";

static char *createsql_dev_group = "CREATE TABLE Device_group("
               		 "GID INTEGER PRIMARY KEY,"
			 "group_Name TEXT NOT NULL UNIQUE);";
               		 

static char *createsql_user_group = "CREATE TABLE User2Group("
               		 "userName TEXT NOT NULL,"
			 "groupName TEXT NOT NULL)";


//static char *insertsql = "INSERT INTO Contact VALUES(NULL, 'Fred', '09990123456');";

//static char *querysql = "SELECT * FROM Contact;";

#endif


int sqlite_getTable(sqlite3 *db, int table, int *row, int *col, char ***result, char *Name, int Direction)
{
	char sql_str[128];
	char *errMsg;
	int ret=0;
	
	char *key="12345";	
   	sqlite3_key(db, key, sizeof(key));


	memset(sql_str, 0, sizeof(sql_str));
	switch (table){
		case  TABLE_ACC_USER:
			if(!strcmp(Name, "*")){
				sprintf(sql_str, "%s", "SELECT * from User;");
			}else{
				sprintf(sql_str, "SELECT ID from User where Name='%s'", Name);
			}		
			break;
		case  TABLE_ACC_GROUP:
			if(!strcmp(Name, "*")){
				sprintf(sql_str, "%s", "SELECT * from User_group;");
			}else{
				sprintf(sql_str, "SELECT ID from User_group where group_Name='%s'", Name);
			}		
			break;
		case  TABLE_DEV_ACCOUNT:
			if(!strcmp(Name, "*")){
				sprintf(sql_str, "%s", "SELECT * from Device;");		
			}else{
				sprintf(sql_str, "SELECT ID from Device where MAC='%s'", Name);
			}
			break;
		case  TABLE_DEV_GROUP: 
			if(!strcmp(Name, "*")){
				sprintf(sql_str, "%s", "SELECT * from Device_group;");		
			}else{
				sprintf(sql_str, "SELECT ID from Device_group where group_Name='%s'", Name);
			}
			break;
		case  TABLE_USER_GROUP: 
			if(!strcmp(Name, "*")){
				sprintf(sql_str, "%s", "SELECT * from User2Group;");		
			}else{
				if (Direction == USER2GROUP){
					sprintf(sql_str, "SELECT ID from User2Group where userName='%s'", Name);
				}else if (Direction == GROUP2USER){
					sprintf(sql_str, "SELECT ID from User2Group where goupName='%s'", Name);
				}else{
				}
			}
			break;
		case  TABLE_SERVICES:
			break;
		default:
			break;
	}

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

int sqlite_update(sqlite3 *db, int table, char *para[])
{
	char sql_str[256];
	int ret=0;
	char *errMsg;
	char *key="12345";	
   	sqlite3_key(db, key, sizeof(key));



	memset(sql_str, 0, sizeof(sql_str));
	switch (table){
		case  TABLE_ACC_USER:
			sprintf(sql_str, "REPLACE INTO User VALUES(NULL, %s, %s, %s, %s, %s, %s);", 
				para[0], para[1], para[2], "NULL", "NULL", "NULL", "NULL");
		//	        para[0], para[1], para[2], para[3], para[4], para[5], para[6]);		
		//		para[0], para[1], para[2], "NULL", "NULL", "NULL", "NULL");		
			break;
		case  TABLE_ACC_GROUP:
			sprintf(sql_str, "REPLACE INTO User_group VALUES(NULL, %s, %s);", 
				para[0], para[1]);		
			printf("sql=%s\n", sql_str);
			break;
		case  TABLE_DEV_ACCOUNT:
			sprintf(sql_str, "REPLACE INTO Device VALUES(NULL, '%s', '%s');", 
				para[0], para[1]);		
			break;
		case  TABLE_DEV_GROUP: 
			sprintf(sql_str, "REPLACE INTO Device_group VALUES(NULL, '%s', '%s');", 
				para[0], para[1]);		
			break;
		case  TABLE_USER_GROUP: 
			sprintf(sql_str, "REPLACE INTO User2Group VALUES(NULL, '%s', '%s');", 
				para[0], para[1]);		
			break;
		case  TABLE_SERVICES:
			break;
		default:
			break;
	}
	ret=sqlite3_exec(db, sql_str, 0, 0, &errMsg);
	if(errMsg!=NULL)
	  sqlite3_free(errMsg);
	return ret;
}

int sqlite_delete(sqlite3 *db, int table, char *Name)
{
	char sql_str[256];
	char sql_str1[256];
	int ret=0;
	char *errMsg;
	
	char *key="12345";	
   	sqlite3_key(db, key, sizeof(key));

	memset(sql_str, 0, sizeof(sql_str));
	memset(sql_str1, 0, sizeof(sql_str1));
	switch (table){
		case  TABLE_ACC_USER:
			sprintf(sql_str, "DELETE from User WHERE Name='%s';", Name);		
			sprintf(sql_str1, "DELETE from  User2Group WHERE userName='%s';", Name);		
			break;
		case  TABLE_ACC_GROUP:
			sprintf(sql_str, "DELETE from User_group WHERE group_Name='%s';", Name);		
			sprintf(sql_str1, "DELETE from  User2Group WHERE groupName='%s';", Name);		
			break;
		case  TABLE_DEV_ACCOUNT:
			sprintf(sql_str, "DELETE from Device WHERE MAC='%s';", Name);		
			sprintf(sql_str1, "DELETE from  Dev2Group WHERE devName='%s';", Name);		
			break;
		case  TABLE_DEV_GROUP: 
			sprintf(sql_str, "DELETE from Device_group WHERE group_Name='%s';", Name);		
			sprintf(sql_str1, "DELETE from  Dev2Group WHERE groupName='%s';", Name);		
			break;
		case  TABLE_SERVICES:
			break;
		default:
			break;
	}
	ret=sqlite3_exec(db, sql_str, 0, 0, &errMsg);
	ret|=sqlite3_exec(db, sql_str1, 0, 0, &errMsg);
	if(errMsg!=NULL)
	  sqlite3_free(errMsg);
	return ret;
}


int sqlite_create(sqlite3 *db)
{
	int ret=0;
	char *errMsg;
	
	printf("down for group\n");
	
	char *key="12345";	
   	sqlite3_key(db, key, sizeof(key));

	//sqlite3_exec(db, enable_foreign, 0, 0, &errMsg);	
	ret|=sqlite3_exec(db, createsql_acc_user, 0, 0, &errMsg);
	ret|=sqlite3_exec(db, createsql_acc_group, 0, 0, &errMsg);
	ret|=sqlite3_exec(db, createsql_dev_user, 0, 0, &errMsg);
	ret|=sqlite3_exec(db, createsql_dev_group, 0, 0, &errMsg);
	ret|=sqlite3_exec(db, createsql_user_group, 0, 0, &errMsg);
	ret|=sqlite3_exec(db, createsql_dev_group, 0, 0, &errMsg);
	if(errMsg!=NULL)
	  sqlite3_free(errMsg);
	return ret;
}

int sqlite_open(char *path, sqlite3 **db, sqlite3 *db_key)
{
	int ret=0;
	if(path == NULL)
	   return -1;
	char *key="12345";	
	ret= sqlite3_open_v2(path, db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
	//sqlite3_key(db_key, key, sizeof(key));
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
   char *parameters[16]={NULL};

   printf("Start...\n");
   //sqlite3_open_v2("example.db", &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
   SQLITEAPI(open("example.db3", &db, db));
  // char *key="12345";	
   //sqlite3_key(db, key, sizeof(key));


   char *test="eeee";
  
   SQLITEAPI(create(db));   
  
   parameters[0]="'HW'";
   parameters[1]="'hardware Department'";
   SQLITEAPI(update(db, TABLE_ACC_GROUP, parameters));   

   parameters[0]="'SW'";
   parameters[1]="'software Department'";
   SQLITEAPI(update(db, TABLE_ACC_GROUP, parameters));   


   parameters[0]="'John'";
   parameters[1]="'1232145'";
   parameters[2]="'SW team'";
   SQLITEAPI(update(db, TABLE_ACC_USER, parameters));   
   
   parameters[0]="'Chris'";
   parameters[1]="'1ddddd'";
   parameters[2]="'SW team'";
   SQLITEAPI(update(db, TABLE_ACC_USER, parameters));   

   SQLITEAPI(getTable(db, TABLE_ACC_USER, &rows, &cols, &result, "*", 0)); 

   int i=0, j=0;
   
   for (i=1;i<=rows;i++) {
       for (j=0;j<cols;j++) {
           printf("%s\t", result[i*cols+j]);
       }
       printf("\n");
   }
        
   SQLITEAPI(freeTable(result));
   
   SQLITEAPI(getTable(db, TABLE_ACC_GROUP, &rows, &cols, &result, "*", 0)); 

   for (i=1;i<=rows;i++) {
       for (j=0;j<cols;j++) {
           printf("%s\t", result[i*cols+j]);
       }
       printf("\n");
   }
        
   SQLITEAPI(freeTable(result));
   SQLITEAPI(delete(db, TABLE_ACC_USER, "John"));


   SQLITEAPI(close(db));
   //sqlite3_close_v2(db);
}
