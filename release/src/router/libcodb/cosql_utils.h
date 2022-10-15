#ifndef __COSQL_UTILS_H__
#define __COSQL_UTILS_H__

#include <stdint.h>
#include <sqlite3.h>
#include <time.h>

typedef enum column_type{
	COLUMN_TYPE_TEXT=0,
    COLUMN_TYPE_TEXT_MAC,
    COLUMN_TYPE_TEXT_IP,
    COLUMN_TYPE_TEXT_JSON,
    COLUMN_TYPE_INT16, /* short */
    COLUMN_TYPE_UINT16, /* unsigned short */
	COLUMN_TYPE_INT, /* int */
    COLUMN_TYPE_UINT, /* unsigned int */
    COLUMN_TYPE_INT64, /* long long */
    COLUMN_TYPE_UINT64, /* unsigned long long */
	COLUMN_TYPE_FLOAT  
} column_type_e;

typedef enum operation_type{
	OP_TYPE_EQUAL=0,
    OP_TYPE_NOT_EQUAL,
	OP_TYPE_GREATER_THAN,
	OP_TYPE_LESSER_THAN,
    OP_TYPE_GREATER_THAN_OR_EQUAL,
    OP_TYPE_LESSER_THAN_OR_EQUAL
} operation_type_e;

typedef union {
    char *t;
    int16_t i16; /* short */
    uint16_t ui16; /* unsigned short */
    int32_t i; /* int */
    uint32_t ui; /* unsigned int */
    int64_t i64; /* long long */
    uint64_t ui64; /* unsigned long long */
    double f;
} column_value_t;

typedef struct sql_column {
    char *name;
    column_type_e type;
    column_value_t value;
} sql_column_t;

typedef struct sql_column_prototype {
    char *name;
    column_type_e type;
} sql_column_prototype_t;

typedef struct sql_column_match {
    char *name;
    column_type_e type;
    operation_type_e operation;
    column_value_t value;
} sql_column_match_t;

#define DB_TABLE_NAME "DB_INFO"
#define DATA_TABLE_NAME "DATA_INFO"

#define MAX_BUF_LEN 256
#define MAX_VALUE_LEN 8192
#define MAX_VERSION_LEN 20
#define MAX_CONFIG_SIZE 20

#define COSQL_OK           0   /* Successful result */
#define COSQL_ERROR        1   /* Generic error */
#define COSQL_DB_VERSION_MISMATCH        2 /* DB version mismatch */
#define COSQL_DB_OR_DISK_FULL            3 /* Database or Disk is full */

#define FORMAT_OK           0   /* FORMAT correct */
#define FORMAT_ERROR        1   /* FORMAT error */

extern sqlite3* cosql_open(const char* db_file);

extern int cosql_close(sqlite3* pdb);

extern void cosql_enable_debug(sqlite3* pdb, int enable);

extern int cosql_create_table(sqlite3* pdb, const char* db_version, int columns_count, sql_column_prototype_t* columns);

extern int cosql_get_db_version(sqlite3* pdb, char* db_version);

extern int cosql_truncate_data_db(sqlite3* pdb);

extern int cosql_drop_db(sqlite3* pdb);

extern int cosql_integrity_check(sqlite3* pdb);

extern int cosql_get_oldest_event_time(sqlite3* pdb, time_t* ret_time);

extern int cosql_count_table(sqlite3* pdb, int* ret_count);

extern int cosql_count_matchs(sqlite3* pdb,
    int match_and_columns_count, sql_column_match_t* match_and_columns,
    int match_or_columns_count, sql_column_match_t* match_or_columns,
    int* ret_count);

extern int cosql_count_matchs_between_time(sqlite3* pdb,
    int match_and_columns_count, sql_column_match_t* match_and_columns,
    int match_or_columns_count, sql_column_match_t* match_or_columns,
    int start_data_time, int end_data_time,
    int* ret_count);

extern int cosql_insert_table(sqlite3* pdb, int columns_count, sql_column_t* columns);

extern int cosql_upsert_table(sqlite3* pdb,
    int match_and_columns_count, sql_column_match_t* match_and_columns,
    int match_or_columns_count, sql_column_match_t* match_or_columns,
    int columns_count, sql_column_t* columns);

extern int cosql_get_int_value(sqlite3* pdb, 
    int match_and_columns_count, sql_column_match_t* match_and_columns,
    int match_or_columns_count, sql_column_match_t* match_or_columns,
    const char* column_name,
    int* ret_value);

extern int cosql_get_column_values(sqlite3* pdb, 
    int match_and_columns_count, sql_column_match_t* match_and_columns,
    int match_or_columns_count, sql_column_match_t* match_or_columns,
    int query_columns_count, sql_column_prototype_t* query_columns,
    int start_data_time, int end_data_time,
    const char* order_column_name,
    const char* order_by,
    int limit,
    int* ret_rows,
    char ***ret_result);

extern int cosql_free_column_values(char **result);

extern int cosql_get_last_xth_double_value(sqlite3* pdb,
    int match_and_columns_count, sql_column_match_t* match_and_columns,
    int match_or_columns_count, sql_column_match_t* match_or_columns,
    const char* column_name, int last_xnd_index,
    double* ret_value, time_t* ret_time);

extern int cosql_sum_between_time(sqlite3* pdb,
    int match_and_columns_count, sql_column_match_t* match_and_columns,
    int match_or_columns_count, sql_column_match_t* match_or_columns,
    const char* column_name, 
    int start_data_time, int end_data_time,
    double* ret_value);

extern int cosql_avg_between_time(sqlite3* pdb,
    int match_and_columns_count, sql_column_match_t* match_and_columns,
    int match_or_columns_count, sql_column_match_t* match_or_columns,
    const char* column_name, 
    int start_data_time, int end_data_time,
    double* ret_value);

extern int cosql_sum_latest_count_limit(sqlite3* pdb,
    int match_and_columns_count, sql_column_match_t* match_and_columns,
    int match_or_columns_count, sql_column_match_t* match_or_columns,
    const char* column_name, int latest_count_limit,
    double* ret_value);

extern int cosql_avg_latest_count_limit(sqlite3* pdb,
    int match_and_columns_count, sql_column_match_t* match_and_columns,
    int match_or_columns_count, sql_column_match_t* match_or_columns,
    const char* column_name, int latest_count_limit,
    double* ret_value);

extern int cosql_get_oldest_event_time(sqlite3* pdb, time_t* ret_time);

extern int cosql_get_latest_event_time(sqlite3* pdb, time_t* ret_time);

extern int cosql_remove_data_between_time(sqlite3* pdb, int start_data_time, int end_data_time);

extern int cosql_remove_data_between_column_value(sqlite3* pdb, const char* column_name, int start_value, int end_value);

extern int cosql_resize_table_by_reserved_count(sqlite3* pdb, int reserved_newest_data_count);

extern int cosql_backup_and_remove_data_between_time(sqlite3* src_pdb, sqlite3* dst_pdb, const char* backup_data_columns, int start_data_time, int end_data_time);

extern int cosql_backup_and_remove_data_between_column_value(sqlite3* pdb, sqlite3* dst_pdb, 
    const char* column_name, int start_value, int end_value);

extern int cosql_backup_and_resize_table_by_reserved_count(sqlite3* pdb, sqlite3* dst_pdb, int reserved_newest_data_count);

#endif
