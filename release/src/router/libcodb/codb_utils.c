#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include "codb_utils.h"
#include "cosql_utils.h"
#include "log.h"
#include <shared.h>

static time_t get_gmt_time_from_local(time_t local_time) {
    struct tm local_tm;

    // Convert local time to a `struct tm`
    if (localtime_r(&local_time, &local_tm) == NULL) {
        return -1; // Return error if conversion fails
    }

    // Use `mktime` to calculate the GMT time by adjusting for the timezone offset
    time_t gmt_time = mktime(&local_tm) - timezone;

    return gmt_time;
}

static char* format_timestamp_to_string(time_t timestamp, char* buffer, size_t buffer_size) {

    //- This function formats a given timestamp into a human-readable string format.
    //- The format used is "YYYY-MM-DD HH:MM:SS".
    //- The function takes a time_t timestamp and a char buffer as input.

    if (buffer==NULL || buffer_size == 0) {
        return NULL;
    }
    
    struct tm time_info;

    if (localtime_r(&timestamp, &time_info) == NULL) {
        return NULL;
    }

    memset(buffer, 0, buffer_size);

    if (strftime(buffer, buffer_size, "%Y-%m-%d %H:%M:%S", &time_info) == 0) {
        return NULL;
    }

    return buffer;
}

static time_t get_zero_time_on_day(unsigned long ts){

    //- This code defines a static function get_zero_time_on_day, 
    //- whose purpose is to calculate the "zero time" of a certain day 
    //- (that is, the start time of the day, usually midnight 00:00)

    time_t current_time, zero_time;

    if(ts > 0)
        current_time = ts;
    else
        current_time = time(NULL);
    zero_time = (((current_time/675)>>7)*675)<<7;

    return zero_time;
}

static time_t get_zero_time_on_day2(unsigned long ts){

    //- This code defines a static function get_zero_time_on_day, 
    //- whose purpose is to calculate the "zero time" of a certain day 
    //- (that is, the start time of the day, usually midnight 00:00)

    time_t current_time, zero_time;
    struct tm *time_info;

    if(ts > 0)
        current_time = ts;
    else
        current_time = time(NULL);
    
    time_info = localtime(&current_time);

    // 將時間結構中的小時、分鐘和秒設為 0
    time_info->tm_hour = 0;
    time_info->tm_min = 0;
    time_info->tm_sec = 0;
    
    return mktime(time_info);
}

static int get_tmp_db_path(const char* db_name, char* db_file_path, int path_len) {

    if (db_name==NULL || db_file_path==NULL) {
        return 0;
    }

    char db_path[12] = "/tmp/.diag/";

    snprintf(db_file_path, path_len, "%s", db_path);
    strlcat(db_file_path, db_name, path_len);
    strlcat(db_file_path, ".db", path_len);

    return 1;
}

static int get_backup_db_path_by_datetime(const char* db_name, const char* db_version, int query_datetime, char* db_file_path) {

    if (db_name==NULL || db_version==NULL || query_datetime<=0 || db_file_path==NULL) {
        return 0;
    }

    char db_bakcup_path[MAX_FILE_PATH]={0};
    char* ex_db_backup_path = nvram_safe_get("ex_db_backup_path");
    if (ex_db_backup_path!=NULL && 
        strlen(ex_db_backup_path)>0 &&
        d_exists(ex_db_backup_path)) {

        snprintf(db_bakcup_path, MAX_FILE_PATH, "%s/%s", ex_db_backup_path, DIAG_DB_FOLDER);
    }
    else if (d_exists(JFFS_DIR)){
        snprintf(db_bakcup_path, MAX_FILE_PATH, "%s/%s", JFFS_DIR, DIAG_DB_FOLDER);
    }
    else {

        return 0;
    }

    if (!d_exists(db_bakcup_path)){
        return 0;
    }

    //- MUST: local to GMT time
    time_t gmt_time = get_gmt_time_from_local(query_datetime);
    // fprintf(stdout, "GMT to local time %d to %d\n", query_datetime, gmt_time);

    time_t timestamp_day = get_zero_time_on_day(gmt_time);
    fprintf(stdout, "Get db name [%d] from query_datetime [%d]\n", timestamp_day, gmt_time);
    
    // file format
    // 1632960000_sys_detect_1.0.db

    char *lasts;
    char* tmp_db_version = strdup(db_version);
    char* pch_db_version = strtok_r(tmp_db_version, ",", &lasts);

    while (pch_db_version!=NULL) {

        char file_name[40] = "\0";

        sprintf(file_name, "%lu_%s_%s.db", timestamp_day, db_name, pch_db_version);

        strncpy(db_file_path, db_bakcup_path, strlen(db_bakcup_path));
        strncat(db_file_path, "/", 1);
        strncat(db_file_path, file_name, strlen(file_name));

        if (access(db_file_path, F_OK)==0) {
            // fprintf(stdout, "find it(db_file_path=%s)!!\n", db_file_path);
            free(tmp_db_version);
            return 1;
        }

        pch_db_version = strtok_r(NULL, ",", &lasts);
    }

    free(tmp_db_version);

    return 0;
}

static sql_column_match_t* parse_match_columns_data(char* data, int data_count) {
    
    if (data == NULL || data_count <= 0) {
        return NULL;
    }

    //- ex. data
    //- name>type(txt/int)>value>operation

    sql_column_match_t* match_columns = malloc(sizeof(sql_column_match_t) * data_count);
    if (match_columns == NULL) {
        return NULL;
    }
    sql_column_match_t* query_match_columns_idx = match_columns;
    
    char* data_copy = strdup(data);
    if (data_copy == NULL) {
        free(match_columns);
        return NULL;
    }

    char *saveptr1 = NULL, *saveptr2 = NULL;
    char* pch = strtok_r(data_copy, ";", &saveptr1);
    while (pch != NULL) {

        int idx = 0;
        char* pch2 = strtok_r(pch, ">", &saveptr2);
        while (pch2 != NULL) {

            int len = strlen(pch2);

            if (idx == 0) {
                //- column name
                query_match_columns_idx->name = (char *)malloc(sizeof(char) * (len + 1));
                if (query_match_columns_idx->name == NULL) {
                    // Free previously allocated memory
                    for (sql_column_match_t* ptr = match_columns; ptr < query_match_columns_idx; ++ptr) {
                        free(ptr->name);
                        if (ptr->type == COLUMN_TYPE_TEXT) {
                            free(ptr->value.t);
                        }
                    }
                    free(match_columns);
                    free(data_copy);
                    return NULL;
                }
                memset(query_match_columns_idx->name, 0, len + 1);
                strncpy(query_match_columns_idx->name, pch2, len);
                query_match_columns_idx->name[len] = '\0'; // Ensure null-termination
            }
            else if (idx == 1) {
                //- column type
                if (strncmp(pch2, "txt", 3) == 0) {
                    query_match_columns_idx->type = COLUMN_TYPE_TEXT;
                }
                else if (strncmp(pch2, "int", 3) == 0) {
                    query_match_columns_idx->type = COLUMN_TYPE_INT;
                }
            }
            else if (idx == 2) {
                //- column value
                if (query_match_columns_idx->type == COLUMN_TYPE_TEXT ||
                    query_match_columns_idx->type == COLUMN_TYPE_TEXT_MAC ||
                    query_match_columns_idx->type == COLUMN_TYPE_TEXT_IP ||
                    query_match_columns_idx->type == COLUMN_TYPE_TEXT_JSON) {
                    query_match_columns_idx->value.t = (char *)malloc(sizeof(char) * (len + 1));
                    if (query_match_columns_idx->value.t == NULL) {
                        // Free previously allocated memory
                        for (sql_column_match_t* ptr = match_columns; ptr < query_match_columns_idx; ++ptr) {
                            free(ptr->name);
                            if (ptr->type == COLUMN_TYPE_TEXT) {
                                free(ptr->value.t);
                            }
                        }
                        free(query_match_columns_idx->name);
                        free(match_columns);
                        free(data_copy);
                        return NULL;
                    }
                    memset(query_match_columns_idx->value.t, 0, len + 1);
                    strncpy(query_match_columns_idx->value.t, pch2, len);
                    query_match_columns_idx->value.t[len] = '\0'; // Ensure null-termination
                }
                else if (query_match_columns_idx->type == COLUMN_TYPE_INT) {
                    query_match_columns_idx->value.i = atoi(pch2);
                }
            }
            else if (idx == 3) {
                query_match_columns_idx->operation = atoi(pch2);
            }

            pch2 = strtok_r(NULL, ">", &saveptr2);
            idx++;
        }

        pch = strtok_r(NULL, ";", &saveptr1);
        query_match_columns_idx++;
    }

    free(data_copy);

    return match_columns;   
}

static sql_column_prototype_t* parse_query_columns_data(char* data, int data_count) {
 
    if (data == NULL || data_count <= 0) {
        return NULL;
    }
 
    sql_column_prototype_t* query_columns = malloc(sizeof(sql_column_prototype_t) * data_count);
    if (query_columns == NULL) {
        return NULL;
    }
 
    sql_column_prototype_t* query_columns_idx = query_columns;
 
    char* data_copy = strdup(data);
    if (data_copy == NULL) {
        free(query_columns);
        return NULL;
    }
 
    char* pch = strtok(data_copy, ";");
    while (pch != NULL) {
        int len = strlen(pch);
 
        query_columns_idx->name = (char *)malloc(sizeof(char) * (len + 1));
        if (query_columns_idx->name == NULL) {
            // Free previously allocated memory
            for (sql_column_prototype_t* ptr = query_columns; ptr < query_columns_idx; ++ptr) {
                free(ptr->name);
            }
            free(query_columns);
            free(data_copy);
            return NULL;
        }
 
        memset(query_columns_idx->name, 0, len + 1);
        strncpy(query_columns_idx->name, pch, len);
        query_columns_idx->name[len] = '\0'; // Ensure null-termination
 
        query_columns_idx->type = COLUMN_TYPE_TEXT;
 
        pch = strtok(NULL, ";");
 
        query_columns_idx++;
    }
 
    free(data_copy);
 
    return query_columns;
}

int codb_test() {

    fprintf(stderr, "run codb_test\n");
    
    int res = 0;

    char content[50] = "data_id;ifname;mac;noise;data_time;glitch";

    int column_count = 0;
    char* tmp_content = strdup(content);
    char* pch = strtok(tmp_content, ";");
    while (pch!=NULL) {
        pch = strtok(NULL, ";");
        column_count++;
    }
    if (tmp_content!=NULL) {
        free(tmp_content);
    }

    int query_duration = 86400*2; //- 2 days
    // int query_end_time = 1641690995; //- GMT: 2022年1月9日Sunday 01:16:35
    int query_end_time = (unsigned)time(NULL);
    int query_start_time = query_end_time - query_duration;
    int qyery_limit = 1;

    char filter_data[100] = "";
    // char order_column_name[10] = "data_id";
    char order_by[10] = "DESC";

    int filter_count = 0;
    char* pch2 = strdup(filter_data);
    pch2 = strtok(pch2, ";");
    while (pch2!=NULL) {
        pch2 = strtok(NULL, ";");
        filter_count++;
    }
    if (pch2!=NULL) {
        free(pch2);
    }

    json_object *retObj = NULL;

    query_start_time= 0;
    query_end_time=0;

    // res = codb_content_query_json_field("wifi_detect", column_count, content, filter_count, filter_data, query_start_time, query_end_time, order_by, qyery_limit, &retObj);
    // res = codb_content_query_duration_json_field("stainfo", column_count, content, filter_count, filter_data, query_start_time, query_end_time, 60, &retObj);
    res = codb_latest_content_query_json_field("wifi_detect", column_count, content, filter_count, filter_data, &retObj);

    if (res==CODB_OK && retObj!=NULL) {
        fprintf(stderr, json_object_to_json_string(retObj));
    }

    if(retObj)
        json_object_put(retObj);

#if 0
    sqlite3* pdb = cosql_open("/tmp/.diag/my_test.db");
    
    cosql_enable_debug(pdb, 1);

    sql_column_prototype_t columns[3];

    columns[0].name = "node_mac";
    columns[0].type = COLUMN_TYPE_TEXT_MAC;

    columns[1].name = "node_ip";
    columns[1].type = COLUMN_TYPE_TEXT_IP;


    columns[2].name = "texst";
    columns[2].type = COLUMN_TYPE_TEXT;

    cosql_create_table(pdb, "1.0", 3, columns);

    int columns1_count = 3;
    sql_column_t columns1[3];

    columns1[0].name = "node_mac";
    columns1[0].type = COLUMN_TYPE_TEXT_MAC;
    columns1[0].value.t = "D4:5D:64:BF:D3:C2";

    columns1[1].name = "node_ip";
    columns1[1].type = COLUMN_TYPE_TEXT_IP;
    columns1[1].value.t = "1050:0000:0000:0000:0005:0600:300c:326b";

    columns1[2].name = "texst";
    columns1[2].type = COLUMN_TYPE_TEXT;
    columns1[2].value.t = "my test string5";

    res = cosql_insert_table(pdb, columns1_count, columns1);

    int columns2_count = 3;
    sql_column_t columns2[3];

    columns2[0].name = "node_mac";
    columns2[0].type = COLUMN_TYPE_TEXT_MAC;
    columns2[0].value.t = "D4:5D:64:BF:D3:C2";

    columns2[1].name = "node_ip";
    columns2[1].type = COLUMN_TYPE_TEXT_IP;
    columns2[1].value.t = "192.168.50.123";

    columns2[2].name = "texst";
    columns2[2].type = COLUMN_TYPE_TEXT;
    columns2[2].value.t = "my test string5";

    res = cosql_insert_table(pdb, columns2_count, columns2);

    cosql_close(pdb);

    fprintf(stderr, "end run codb_test\n");
#endif

#if 0
    sqlite3* pdb = cosql_open("/tmp/.diag/sys_setting.db");

    cosql_truncate_data_db(pdb);
    
    // int match_and_columns_count = 1;
    // sql_column_match_t match_and_columns[1];
    // match_and_columns[0].name = "node_mac";
    // match_and_columns[0].type = COLUMN_TYPE_TEXT;
    // match_and_columns[0].operation = OP_TYPE_EQUAL;
    // match_and_columns[0].value.t = "D4:5D:64:BF:D3:C0";

    // int columns_count = 1;
    // sql_column_t columns[1];
    // columns[0].name = "fw_ver";
    // columns[0].value.t = "my test string";

    // res = cosql_upsert_table(pdb,
    //     match_and_columns_count, match_and_columns,
    //     0, NULL,
    //     columns_count, columns);

    int match_and_columns2_count = 1;
    sql_column_match_t match_and_columns2[1];
    match_and_columns2[0].name = "node_mac";
    match_and_columns2[0].type = COLUMN_TYPE_TEXT;
    match_and_columns2[0].operation = OP_TYPE_EQUAL;
    match_and_columns2[0].value.t = "D4:5D:64:BF:D3:C2";

    int columns2_count = 4;
    sql_column_t columns2[4];
    columns2[0].name = "node_type";
    columns2[0].type = COLUMN_TYPE_TEXT;
    columns2[0].value.t = "R";

    columns2[1].name = "node_ip";
    columns2[1].type = COLUMN_TYPE_TEXT;
    columns2[1].value.t = "192.168.50.123";

    columns2[2].name = "node_mac";
    columns2[2].type = COLUMN_TYPE_TEXT;
    columns2[2].value.t = "D4:5D:64:BF:D3:C2";

    columns2[3].name = "fw_ver";
    columns2[3].type = COLUMN_TYPE_TEXT;
    columns2[3].value.t = "my test string5";

    res = cosql_upsert_table(pdb,
        match_and_columns2_count, match_and_columns2,
        0, NULL,
        columns2_count, columns2);

    cosql_close(pdb);

    fprintf(stderr, "res=%d\n", res);
#endif

    return 0;
}

int query_database(void *pdb, 
                   int match_and_columns_count, char **match_and_columns,
                   int match_or_columns_count, char **match_or_columns,
                   int columns_count, char **query_columns, unsigned long query_start,
                   unsigned long query_end, const char *order, int limit,
                   json_object *resultValueArrayObj, int *total_ret_rows) {
    int ret_rows = 0;
    char **result = NULL;

    if (pdb != NULL) {
        int ret = cosql_get_column_values(pdb,
                                          match_and_columns_count, match_and_columns,
                                          match_or_columns_count, match_or_columns,
                                          columns_count, query_columns,
                                          query_start,
                                          query_end,
                                          "data_id",
                                          order,
                                          limit,
                                          &ret_rows,
                                          &result);

        for (int i = 0; i < ret_rows; i++) {
            json_object *fieldValueArrayObj = json_object_new_array();

            for (int j = 0; j < columns_count; j++) {
                int start_index = columns_count + i * columns_count;
                int array_index = start_index + j;
                char *column_value = result[array_index];
                json_object *new_obj = json_object_new_string(column_value);
                json_object_array_add(fieldValueArrayObj, new_obj);
            }

            json_object_array_add(resultValueArrayObj, fieldValueArrayObj);
        }

        if (result != NULL) {
            cosql_free_column_values(result);
        }

        *total_ret_rows += ret_rows;
    }

    return ret_rows;
}

int codb_content_query_json_field(char* db_name, int columns_count, char* columns_name, int filter_count, char* filter_data, int start, int end, char* order_by, int limit, json_object **retJsonObj) {
    
    if (db_name==NULL || columns_name==NULL) {
        return CODB_ERROR;
    }

    if (start>end) {
        return CODB_ERROR;
    }

    time_t current_time = time(NULL);

    if (start==0) {
        start = current_time - 86400;
    }

    if (end==0) {
        end = current_time;
    }
    
    sqlite3* pdb_tmp = NULL;
    char tmp_db_file_path[MAX_FILE_PATH] = "\0";
    if (get_tmp_db_path(db_name, tmp_db_file_path, MAX_FILE_PATH)==1) {
        pdb_tmp = cosql_open(tmp_db_file_path);
        
        //cosql_enable_debug(pdb_tmp, 1);
        
        if (pdb_tmp==NULL) {
            fprintf(stderr, "Fail to open tmp db file\n");
            return CODB_ERROR;
        }       
    }
    ///////////////////////////////////////////////////////////////

    int match_and_columns_count = filter_count;
    sql_column_match_t* match_and_columns = parse_match_columns_data(filter_data, filter_count);
    ///////////////////////////////////////////////////////////////

    char db_version[20] = "1.0,2.0,3.0";
    char backup_db_file_path[MAX_FILE_PATH] = "\0";
    char buffer_f_t_s[64] = "\0";
    char buffer_f_t_e[64] = "\0";
    sqlite3* pdb_backup = NULL;
    ///////////////////////////////////////////////////////////////

    sql_column_prototype_t* query_columns = parse_query_columns_data(columns_name, columns_count);

    codbg("Enter codb_content_query_json_field, query columns are %s, order_by is %s, start=%s, end=%s", 
        columns_name, order_by, format_timestamp_to_string(start, buffer_f_t_s, sizeof(buffer_f_t_s)), format_timestamp_to_string(end, buffer_f_t_e, sizeof(buffer_f_t_e)));
    ///////////////////////////////////////////////////////////////

    int i, j, start_index, array_index, ret_rows = 0, count=0;
    char **result;
    char *column_value = NULL;

    json_object *resultValueArrayObj = json_object_new_array();

    int total_ret_rows = 0;
    int query_start = 0;
    int query_end = 0;
    int query_limit = limit;
    int order_desc = strncmp(order_by, "DESC", 4)==0 ? 1 : 0;

    if (order_desc==1) {
        query_start = get_zero_time_on_day2(end); //- ex. 2022-01-09 00:00:00
        query_end = end; //- ex. 2022-01-09 09:12:35
        if (query_start<start) query_start = start;
    }
    else {
        query_start = start; //- ex. 2022-01-09 09:12:35
        query_end = get_zero_time_on_day2(start) + 86399; //- ex. 2022-01-09 23:59:59
        if (query_end>end) query_end = end;
    }

    while(1) {

        codbg("******************************", count);
        codbg("period count[%d], query_start=%s, query_end=%s", count, format_timestamp_to_string(query_start, buffer_f_t_s, sizeof(buffer_f_t_s)), format_timestamp_to_string(query_end, buffer_f_t_e, sizeof(buffer_f_t_e)));
        
        char find_backup_db_file_path[MAX_FILE_PATH] = "\0";
        if (get_backup_db_path_by_datetime(db_name, db_version, query_start, find_backup_db_file_path)==1) {

            //- open backup db.
            if (strlen(backup_db_file_path)==0 || strncmp(find_backup_db_file_path, backup_db_file_path, strlen(backup_db_file_path))!=0){
                if (pdb_backup!=NULL) {
                    cosql_close(pdb_backup);
                }
                
                pdb_backup = cosql_open(find_backup_db_file_path);
                if (pdb_backup!=NULL) {
                    strncpy(backup_db_file_path, find_backup_db_file_path, strlen(find_backup_db_file_path));
                    codbg("Open backup_db_file_path=%s", backup_db_file_path);
                }
            }
        }

        if (order_desc == 1) {
            //- get data from temp db.
            ret_rows = query_database(pdb_tmp, 
                match_and_columns_count, match_and_columns,
                0, NULL,
                columns_count, query_columns, 
                query_start, 
                query_end,
                (order_desc==1) ? "DESC" : "ASC", 
                limit, 
                resultValueArrayObj, 
                &total_ret_rows);

            codbg("The count of query result from temp db is %d.", ret_rows);

            if (query_limit>0) {
                if (ret_rows>=limit) {
                    break;
                }
                else {
                    limit = limit - ret_rows;
                }
            }

            //- get data from backup db.
            ret_rows = query_database(pdb_backup, 
                match_and_columns_count, match_and_columns,
                0, NULL,
                columns_count, query_columns, 
                query_start, 
                query_end,
                (order_desc==1) ? "DESC" : "ASC", 
                limit, 
                resultValueArrayObj, 
                &total_ret_rows);

            codbg("The count of query result from backup db is %d.", ret_rows);

            if (query_limit>0) {
                if (ret_rows>=limit) {
                    break;
                }
                else {
                    limit = limit - ret_rows;
                }
            }
        }
        else {
            //- get data from backup db.
            ret_rows = query_database(pdb_backup, 
                match_and_columns_count, match_and_columns,
                0, NULL,
                columns_count, query_columns, 
                query_start, 
                query_end,
                (order_desc==1) ? "DESC" : "ASC", 
                limit, 
                resultValueArrayObj, 
                &total_ret_rows);

            codbg("The count of query result from backup db is %d.", ret_rows);

            if (query_limit>0) {
                if (ret_rows>=limit) {
                    break;
                }
                else {
                    limit = limit - ret_rows;
                }
            }

            //- get data from temp db.
            ret_rows = query_database(pdb_tmp, 
                match_and_columns_count, match_and_columns,
                0, NULL,
                columns_count, query_columns, 
                query_start, 
                query_end,
                (order_desc==1) ? "DESC" : "ASC", 
                limit, 
                resultValueArrayObj, 
                &total_ret_rows);

            codbg("The count of query result from temp db is %d.", ret_rows);

            if (query_limit>0) {
                if (ret_rows>=limit) {
                    break;
                }
                else {
                    limit = limit - ret_rows;
                }
            }
        }
NEXT_PERIOD:
        
        count++;

        //- next period
        if (order_desc==1) {

            if (query_start<=start) {
                //- End query
                break;
            }

            query_end = query_start - 1; //- ex. 2022-01-08 23:59:59
            query_start = get_zero_time_on_day2(query_end); //- ex. 2022-01-08 00:00:00
            if (query_start<start) query_start = start;
        }
        else {
            
            if (query_end>=end) {
                //- End query
                break;
            }

            query_start = query_end + 1; //- ex. 2022-01-10 00:00:00
            query_end = get_zero_time_on_day2(query_start) + 86399; //- ex. 2022-01-10 23:59:59
            if (query_end>end) query_end = end;
        }
    }
    
    //- free match_and_columns
    cosql_free_match_columns(match_and_columns, match_and_columns_count);

    //- free query_columns
    cosql_free_query_columns(query_columns, columns_count);
    
    cosql_close(pdb_tmp);
    
    cosql_close(pdb_backup);

    json_object *rootObj = json_object_new_object();
    json_object_object_add(rootObj, "contents", resultValueArrayObj);

    *retJsonObj = rootObj;

    codbg("Leave codb_content_query_json_field.");

    return CODB_OK;
}

int codb_latest_content_query_json_field(char* db_name, int columns_count, char* columns_name, int filter_count, char* filter_data, json_object **retJsonObj) {
    
    if (db_name==NULL || columns_name==NULL) {
        return CODB_ERROR;
    }
    
    time_t current_time = time(NULL);

    sqlite3* pdb_tmp = NULL;
    char tmp_db_file_path[MAX_FILE_PATH] = "\0";
    if (get_tmp_db_path(db_name, tmp_db_file_path, MAX_FILE_PATH)==1) {
        pdb_tmp = cosql_open(tmp_db_file_path);
        
        //cosql_enable_debug(pdb_tmp, 1);
        
        if (pdb_tmp==NULL) {
            fprintf(stderr, "Fail to open tmp db file\n");
            return CODB_ERROR;
        }       
    }
    ///////////////////////////////////////////////////////////////

    int match_and_columns_count = filter_count;
    sql_column_match_t* match_and_columns = parse_match_columns_data(filter_data, filter_count);
    ///////////////////////////////////////////////////////////////

    char db_version[20] = "1.0,2.0,3.0";
    char backup_db_file_path[MAX_FILE_PATH] = "\0";
    char buffer_f_t_s[64] = "\0";
    char buffer_f_t_e[64] = "\0";
    sqlite3* pdb_backup = NULL;
    ///////////////////////////////////////////////////////////////

    sql_column_prototype_t* query_columns = parse_query_columns_data(columns_name, columns_count);

    codbg("Enter codb_latest_content_query_json_field, query columns are %s", columns_name);
    ///////////////////////////////////////////////////////////////

    int i, j, start_index, array_index, ret_rows = 0, count=0;
    char **result;
    char *column_value = NULL;

    json_object *resultValueArrayObj = json_object_new_array();

    int total_ret_rows = 0;
    int query_start = get_zero_time_on_day2(current_time);

    while(1) {

        codbg("******************************", count);
        codbg("count[%d]", count);
        
        //- get data from temp db.
        ret_rows = query_database(pdb_tmp, 
            match_and_columns_count, match_and_columns,
            0, NULL,
            columns_count, query_columns, 
            0, 
            0,
            "DESC", 
            1, 
            resultValueArrayObj, 
            &total_ret_rows);

        codbg("The count of query result from temp db is %d.", ret_rows);

        if (ret_rows<=0) {
            char find_backup_db_file_path[MAX_FILE_PATH] = "\0";
            if (get_backup_db_path_by_datetime(db_name, db_version, query_start, find_backup_db_file_path)==1) {

                //- open backup db.
                if (strlen(backup_db_file_path)==0 || strncmp(find_backup_db_file_path, backup_db_file_path, strlen(backup_db_file_path))!=0){
                    if (pdb_backup!=NULL) {
                        cosql_close(pdb_backup);
                    }
                    
                    pdb_backup = cosql_open(find_backup_db_file_path);
                    if (pdb_backup!=NULL) {
                        strncpy(backup_db_file_path, find_backup_db_file_path, strlen(find_backup_db_file_path));
                        codbg("Open backup_db_file_path=%s", backup_db_file_path);
                    }
                }
            }
            else {
                break;
            }

            //- get data from backup db.
            ret_rows = query_database(pdb_backup, 
                match_and_columns_count, match_and_columns,
                0, NULL,
                columns_count, query_columns, 
                0, 
                0,
                "DESC", 
                1, 
                resultValueArrayObj, 
                &total_ret_rows);

            codbg("The count of query result from backup db is %d.", ret_rows);
        }
NEXT_PERIOD:
        
        count++;

        //- next period
        if (ret_rows>0) {
            break;
        }

        query_start = query_start - 86400;
    }
    
    //- free match_and_columns
    cosql_free_match_columns(match_and_columns, match_and_columns_count);

    //- free query_columns
    cosql_free_query_columns(query_columns, columns_count);
    
    cosql_close(pdb_tmp);
    
    cosql_close(pdb_backup);

    json_object *rootObj = json_object_new_object();
    if (rootObj == NULL) {
        json_object_put(resultValueArrayObj);
        return CODB_ERROR;
    }

    json_object_object_add(rootObj, "contents", resultValueArrayObj);

    *retJsonObj = rootObj;

    codbg("Leave codb_content_query_json_field.");

    return CODB_OK;
}

int codb_content_query_duration_json_field(char* db_name, int columns_count, char* columns_name, int filter_count, char* filter_data, int start, int end, int duration, json_object **retJsonObj) {
    
    if (db_name==NULL || columns_name==NULL) {
        return CODB_ERROR;
    }

    if (start>end) {
        return CODB_ERROR;
    }

    sqlite3* pdb_tmp = NULL;
    char tmp_db_file_path[MAX_FILE_PATH] = "\0";
    if (get_tmp_db_path(db_name, tmp_db_file_path, MAX_FILE_PATH)==1) {
        pdb_tmp = cosql_open(tmp_db_file_path);
        if (pdb_tmp==NULL) {
            fprintf(stderr, "Fail to open tmp db file\n");
            return CODB_ERROR;
        }       
    }
    ///////////////////////////////////////////////////////////////

    int match_and_columns_count = filter_count;
    sql_column_match_t* match_and_columns = parse_match_columns_data(filter_data, filter_count);
    ///////////////////////////////////////////////////////////////

    char db_version[20] = "1.0,2.0,3.0";
    char backup_db_file_path[MAX_FILE_PATH] = "\0";
    sqlite3* pdb_backup = NULL;
    ///////////////////////////////////////////////////////////////

    sql_column_prototype_t* query_columns = parse_query_columns_data(columns_name, columns_count);

    // sql_column_prototype_t* query_columns = malloc(sizeof(sql_column_prototype_t)*columns_count);
    // sql_column_prototype_t* query_columns_idx = query_columns;
    // char* tmp_columns_name = strdup(columns_name);
    // char* pch = strtok(tmp_columns_name, ";");
    // while (pch!=NULL) {

    //     int len = strlen(pch);

    //     //- column name
    //     query_columns_idx->name = (char *)malloc(sizeof(char)*len);
    //     memset(query_columns_idx->name, 0, len);
    //     strncpy(query_columns_idx->name, pch, len);

    //     //- column type
    //     query_columns_idx->type = COLUMN_TYPE_TEXT;
        
    //     pch = strtok(NULL, ";");

    //     // fprintf(stderr, "query_columns->name=%s\n", query_columns->name);

    //     query_columns_idx++;
    // }

    // if (tmp_columns_name!=NULL) {
    //     free(tmp_columns_name);
    // }

    fprintf(stdout, "columns_count=%d, columns_name=%s, start=%d, end=%d\n", columns_count, columns_name, start, end);
    ///////////////////////////////////////////////////////////////

    int i, j, start_index, array_index, ret_rows = 0, count=0;
    char **result;
    char *column_value = NULL;

    json_object *resultValueArrayObj = json_object_new_array();

    int data_sampling_time = 60;
    int total_ret_rows = 0;
    int query_start = start;
    int query_end = 0;

    while (query_start<end) {
        query_end = query_start + duration;
        if (query_end>end) query_end = end;

        //- get data from tmp db.
        cosql_get_column_values(pdb_tmp,
            match_and_columns_count, match_and_columns,
            0, NULL,
            columns_count, query_columns,
            query_start, 
            query_start + data_sampling_time,
            NULL,
            NULL,
            0,
            &ret_rows,
            &result);

        for (i=0; i<ret_rows; i++) {

            json_object *fieldValueArrayObj = json_object_new_array();

            for (j=0; j<columns_count; j++) {
                start_index = columns_count + i*columns_count;
                array_index = start_index + j;
                column_value = result[array_index];
                json_object *new_obj = json_object_new_string(column_value);
                json_object_array_add(fieldValueArrayObj, new_obj);
            }

            json_object_array_add(resultValueArrayObj, fieldValueArrayObj);
        }

        if (result!=NULL) {
            cosql_free_column_values(result);
        }

        if (ret_rows>0) {
            goto NEXT_PERIOD;
        }

        //- get data from backup db.
        char find_backup_db_file_path[MAX_FILE_PATH] = "\0";
        if (get_backup_db_path_by_datetime(db_name, db_version, query_start, find_backup_db_file_path)==1) {

            //- open backup db.
            if (strlen(backup_db_file_path)==0 || strncmp(find_backup_db_file_path, backup_db_file_path, strlen(backup_db_file_path))!=0){
                if (pdb_backup!=NULL) {
                    cosql_close(pdb_backup);
                }
                
                pdb_backup = cosql_open(find_backup_db_file_path);
                if (pdb_backup!=NULL) {
                    strncpy(backup_db_file_path, find_backup_db_file_path, strlen(find_backup_db_file_path));
                    fprintf(stderr, "Open backup_db_file_path=%s\n", backup_db_file_path);
                }
            }

            if (pdb_backup==NULL) {
                goto NEXT_PERIOD;
            }

            cosql_get_column_values(pdb_backup,
                match_and_columns_count, match_and_columns,
                0, NULL,
                columns_count, query_columns,
                query_start, 
                query_start + data_sampling_time,
                NULL,
                NULL,
                0,
                &ret_rows,
                &result);

            for (i=0; i<ret_rows; i++) {
                json_object *fieldValueArrayObj = json_object_new_array();

                for (j=0; j<columns_count; j++) {
                    start_index = columns_count + i*columns_count;
                    array_index = start_index + j;
                    column_value = result[array_index];
                    json_object *new_obj = json_object_new_string(column_value);
                    json_object_array_add(fieldValueArrayObj, new_obj);
                }

                json_object_array_add(resultValueArrayObj, fieldValueArrayObj);
            }

            if (result!=NULL) {
                cosql_free_column_values(result);
            }
        }
        
NEXT_PERIOD:

        fprintf(stdout, "[%d] query_start=%d, query_end=%d, ret_rows=%d\n", count, query_start, query_end, ret_rows);

        query_start = query_end;
        
        count++;

        total_ret_rows = total_ret_rows + ret_rows;
    }
    ///////////////////////////////////////////////////////////////

    //- free match_and_columns
    cosql_free_match_columns(match_and_columns, match_and_columns_count);

    //- free query_columns
    cosql_free_query_columns(query_columns, columns_count);
    
    cosql_close(pdb_tmp);
    
    cosql_close(pdb_backup);

    fprintf(stderr, "Success to open db file\n");

    json_object *rootObj = json_object_new_object();
    json_object_object_add(rootObj, "contents", resultValueArrayObj);

    *retJsonObj = rootObj;

    return CODB_OK;
}

int codb_avg_query_json_field(char* db_name, char* field_name, char* node_mac, int start, int end, int duration, json_object **retJsonObj) {
    
    if (db_name==NULL || field_name==NULL) {
        return CODB_ERROR;
    }

    if (start>end) {
        return CODB_ERROR;
    }

    if (duration<=0) {
        return CODB_ERROR;
    }
    ///////////////////////////////////////////////////////////////

    sqlite3* pdb_tmp = NULL;
    char tmp_db_file_path[MAX_FILE_PATH] = "\0";
    if (get_tmp_db_path(db_name, tmp_db_file_path, MAX_FILE_PATH)==1) {
        pdb_tmp = cosql_open(tmp_db_file_path);
        if (pdb_tmp==NULL) {
            fprintf(stderr, "Fail to open tmp db file\n");
            return CODB_ERROR;
        }       
    }
    ///////////////////////////////////////////////////////////////

    int match_and_columns_count = 0;
    sql_column_match_t match_and_columns[1];

    if (node_mac!=NULL && strcmp(node_mac, "")!=0) {

        match_and_columns_count = 1;

        match_and_columns[0].name = "node_mac";
        match_and_columns[0].type = COLUMN_TYPE_TEXT_MAC;
        match_and_columns[0].operation = OP_TYPE_EQUAL;
        match_and_columns[0].value.t = node_mac;
    }

    char db_version[20] = "1.0,2.0,3.0";
    char backup_db_file_path[MAX_FILE_PATH] = "\0";
    sqlite3* pdb_backup = NULL;

    json_object *rootObj = json_object_new_object();

    char* tmp_field_name = strdup(field_name);
    char* pch = strtok(tmp_field_name, ";");

    while (pch!=NULL) {

        int count=0;
        int query_start = start;
        int query_end = start + duration;

        json_object *fieldValueArrayObj = json_object_new_array();

        while (query_start<end) {
            query_end = query_start + duration;
            if (query_end>end) query_end = end;
            ///////////////////////////////////////////////////////////////

            double ret_value = 0;

            //- get data from tmp db.
            fprintf(stdout, "Get data from tmp db\n");
            if (match_and_columns_count>0) {
                cosql_avg_between_time(pdb_tmp,
                    match_and_columns_count, match_and_columns,
                    0, NULL,
                    pch, 
                    query_start, 
                    query_end,
                    &ret_value);
            }
            else {
                cosql_avg_between_time(pdb_tmp,
                    0, NULL,
                    0, NULL,
                    pch, 
                    query_start, 
                    query_end,
                    &ret_value);
            }

            if (ret_value>0) {
                goto NEXT_PERIOD;
            }
            ///////////////////////////////////////////////////////////////

            //- get data from backup db.
            char find_backup_db_file_path[MAX_FILE_PATH] = "\0";
            if (get_backup_db_path_by_datetime(db_name, db_version, query_start, find_backup_db_file_path)==1) {
                if (strlen(backup_db_file_path)==0 || strncmp(find_backup_db_file_path, backup_db_file_path, strlen(backup_db_file_path))!=0){
                    if (pdb_backup!=NULL) {
                        cosql_close(pdb_backup);
                    }

                    pdb_backup = cosql_open(find_backup_db_file_path);
                    if (pdb_backup!=NULL) {
                        strncpy(backup_db_file_path, find_backup_db_file_path, strlen(find_backup_db_file_path));
                        fprintf(stderr, "Open backup_db_file_path=%s\n", backup_db_file_path);
                    }
                }

                if (match_and_columns_count>0) {
                    cosql_avg_between_time(pdb_backup,
                        match_and_columns_count, match_and_columns,
                        0, NULL,
                        pch, 
                        query_start, 
                        query_end,
                        &ret_value);
                }
                else {
                    cosql_avg_between_time(pdb_backup,
                        0, NULL,
                        0, NULL,
                        pch, 
                        query_start, 
                        query_end,
                        &ret_value);
                }

                if (ret_value>0) {
                    goto NEXT_PERIOD;
                }
            }
            ///////////////////////////////////////////////////////////////

    NEXT_PERIOD:
            fprintf(stderr, "query_start=%d -> query_end=%d, count=%d, ret_value=%f\n", query_start, query_end, count, ret_value);

            query_start = query_end;

            json_object *new_obj = json_object_new_double(ret_value);
            json_object_array_add(fieldValueArrayObj, new_obj);
            
            count++;
        }

        json_object_object_add(rootObj, pch, fieldValueArrayObj);

        pch = strtok(NULL, ";");
    }

    if (tmp_field_name!=NULL) {
        free(tmp_field_name);
    }

    cosql_close(pdb_tmp);
    cosql_close(pdb_backup);

    fprintf(stderr, "Success to open db file\n");

    *retJsonObj = rootObj;

    return CODB_OK;
}

int codb_eth_detect_traffic_data(char* node_mac, int is_bh, int start, int end, int duration, json_object **retJsonObj) {
    
    char *db_name = "eth_detect";
    char *field_name = "tx_diff;rx_diff";
    
    if (start>end) {
        return CODB_ERROR;
    }

    if (duration<=0) {
        return CODB_ERROR;
    }
    ///////////////////////////////////////////////////////////////

    sqlite3* pdb_tmp = NULL;
    char tmp_db_file_path[MAX_FILE_PATH] = "\0";
    if (get_tmp_db_path(db_name, tmp_db_file_path, MAX_FILE_PATH)==1) {
        pdb_tmp = cosql_open(tmp_db_file_path);
        if (pdb_tmp==NULL) {
            fprintf(stderr, "Fail to open tmp db file\n");
            return CODB_ERROR;
        }       
    }
    ///////////////////////////////////////////////////////////////

    // cosql_enable_debug(pdb_tmp, 1);

    int match_and_columns_count = 1;
    sql_column_match_t match_and_columns[2];

    match_and_columns[0].name = "type";
    match_and_columns[0].type = COLUMN_TYPE_TEXT;
    match_and_columns[0].operation = OP_TYPE_EQUAL;
    match_and_columns[0].value.t = (is_bh==1) ? "BH" : "FH";

    if (node_mac!=NULL && strcmp(node_mac, "")!=0) {

        match_and_columns_count = 2;

        match_and_columns[1].name = "node_mac";
        match_and_columns[1].type = COLUMN_TYPE_TEXT_MAC;
        match_and_columns[1].operation = OP_TYPE_EQUAL;
        match_and_columns[1].value.t = node_mac;
    }

    char db_version[20] = "1.0,2.0,3.0";
    char backup_db_file_path[MAX_FILE_PATH] = "\0";
    sqlite3* pdb_backup = NULL;

    json_object *rootObj = json_object_new_object();

    char* tmp_field_name = strdup(field_name);
    char* pch = strtok(tmp_field_name, ";");

    while (pch!=NULL) {

        int count=0;
        int query_start = start;
        int query_end = start + duration;

        json_object *fieldValueArrayObj = json_object_new_array();

        while (query_start<end) {
            query_end = query_start + duration;
            if (query_end>end) query_end = end;
            ///////////////////////////////////////////////////////////////

            double ret_value = 0;

            //- get data from tmp db.
            fprintf(stdout, "Get data from tmp db\n");
            if (match_and_columns_count>0) {
                cosql_sum_between_time(pdb_tmp,
                    match_and_columns_count, match_and_columns,
                    0, NULL,
                    pch, 
                    query_start, 
                    query_end,
                    &ret_value);
            }
            else {
                cosql_sum_between_time(pdb_tmp,
                    0, NULL,
                    0, NULL,
                    pch, 
                    query_start, 
                    query_end,
                    &ret_value);
            }

            if (ret_value>0) {
                goto NEXT_PERIOD;
            }
            ///////////////////////////////////////////////////////////////

            //- get data from backup db.
            char find_backup_db_file_path[MAX_FILE_PATH] = "\0";
            if (get_backup_db_path_by_datetime(db_name, db_version, query_start, find_backup_db_file_path)==1) {
                if (strlen(backup_db_file_path)==0 || strncmp(find_backup_db_file_path, backup_db_file_path, strlen(backup_db_file_path))!=0){
                    if (pdb_backup!=NULL) {
                        cosql_close(pdb_backup);
                    }

                    pdb_backup = cosql_open(find_backup_db_file_path);
                    if (pdb_backup!=NULL) {
                        strncpy(backup_db_file_path, find_backup_db_file_path, strlen(find_backup_db_file_path));
                        fprintf(stderr, "Open backup_db_file_path=%s\n", backup_db_file_path);
                    }
                }

                if (match_and_columns_count>0) {
                    cosql_sum_between_time(pdb_backup,
                        match_and_columns_count, match_and_columns,
                        0, NULL,
                        pch, 
                        query_start, 
                        query_end,
                        &ret_value);
                }
                else {
                    cosql_sum_between_time(pdb_backup,
                        0, NULL,
                        0, NULL,
                        pch, 
                        query_start, 
                        query_end,
                        &ret_value);
                }

                if (ret_value>0) {
                    goto NEXT_PERIOD;
                }
            }
            ///////////////////////////////////////////////////////////////

    NEXT_PERIOD:
            fprintf(stderr, "query_start=%d -> query_end=%d, count=%d, ret_value=%f\n", query_start, query_end, count, ret_value);

            query_start = query_end;

            json_object *new_obj = json_object_new_double(ret_value);
            json_object_array_add(fieldValueArrayObj, new_obj);
            
            count++;
        }

        json_object_object_add(rootObj, pch, fieldValueArrayObj);

        pch = strtok(NULL, ";");
    }

    if (tmp_field_name!=NULL) {
        free(tmp_field_name);
    }

    cosql_close(pdb_tmp);
    cosql_close(pdb_backup);

    fprintf(stderr, "Success to open db file\n");

    *retJsonObj = rootObj;

    return CODB_OK;
}

// int codb_count_query_json_field(const char* db_name, const char* field_name, const char* node_mac, int start, int end, int duration, json_object **retJsonObj) {
//     if (db_name==NULL || field_name==NULL) {
//         return CODB_ERROR;
//     }

//     if (start>end) {
//         return CODB_ERROR;
//     }

//     if (duration<=0) {
//         return CODB_ERROR;
//     }

//     char db_path[12] = "/tmp/.diag/";
//     char db_file_path[MAX_FILE_PATH];
//     strncpy(db_file_path, db_path, strlen(db_path));
//     strncat(db_file_path, db_name, strlen(db_name));
//     strncat(db_file_path, ".db", 3);

//     fprintf(stderr, "db_file_path = %s, field_name=%s\n", db_file_path, field_name);

//     sqlite3* pdb = cosql_open(db_file_path);
//     if (pdb==NULL) {
//         fprintf(stderr, "Fail to open db file\n");
//         return CODB_ERROR;
//     }

//     // cosql_enable_debug(pdb, 1);

//     int match_and_columns_count = 0;
//     sql_column_match_t match_and_columns[1];

//     if (node_mac!=NULL && strcmp(node_mac, "")!=0) {

//         match_and_columns_count = 1;

//         match_and_columns[0].name = "node_mac";
//         match_and_columns[0].type = COLUMN_TYPE_TEXT;
//         match_and_columns[0].operation = OP_TYPE_EQUAL;
//         match_and_columns[0].value.t = node_mac;
//     }

//     json_object *rootObj = json_object_new_object();

//     char* tmp_field_name = strdup(field_name);
//     char* pch = strtok(tmp_field_name, ";");

//     while (pch!=NULL) {

//         fprintf(stderr, "pch=%s\n", pch);

//         int count=0;
//         int query_start = start;
//         int query_end = start + duration;

//         json_object *fieldValueArrayObj = json_object_new_array();

//         while (query_start<end) {
//             query_end = query_start + duration;
//             if (query_end>end) query_end = end;

//             int query_time = query_start + duration/2;
//             int seconds = query_time % 60;

//             //- remove seconds
//             query_time = query_time - seconds;

//             int ret_value = 0;

//             if (match_and_columns_count>0) {
//                 cosql_count_matchs_between_time(pdb, 
//                     match_and_columns_count, match_and_columns, 
//                     0, NULL,
//                     query_time, 
//                     query_time + 59,
//                     &ret_value);
//             }
//             else {
//                 cosql_count_matchs_between_time(pdb, 
//                     0, NULL, 
//                     0, NULL,
//                     query_time, 
//                     query_time + 59,
//                     &ret_value);
//             }

//             fprintf(stderr, "query_start=%d -> query_end=%d, query_time=%d, count=%d, ret_value=%d\n", query_start, query_end, query_time, count, ret_value);

//             query_start = query_end;

//             json_object *new_obj = json_object_new_double(ret_value);
//             json_object_array_add(fieldValueArrayObj, new_obj);
            
//             count++;
//         }

//         json_object_object_add(rootObj, pch, fieldValueArrayObj);

//         pch = strtok(NULL, ";");
//     }

//     if (tmp_field_name!=NULL) {
//         free(tmp_field_name);
//     }

//     cosql_close(pdb);

//     fprintf(stderr, "Success to open db file\n");

//     *retJsonObj = rootObj;

//     return CODB_OK;
// }

int codb_count_active_client(char* node_mac, int start, int end, int duration, json_object **retJsonObj) {
    
    if (start>end) {
        return CODB_ERROR;
    }

    if (duration<=0) {
        return CODB_ERROR;
    }

    char db_name[8] = "stainfo";
    ///////////////////////////////////////////////////////////////

    sqlite3* pdb_tmp = NULL;
    char tmp_db_file_path[MAX_FILE_PATH] = "\0";
    if (get_tmp_db_path(db_name, tmp_db_file_path, MAX_FILE_PATH)==1) {
        pdb_tmp = cosql_open(tmp_db_file_path);
        if (pdb_tmp==NULL) {
            fprintf(stderr, "Fail to open tmp db file\n");
            return CODB_ERROR;
        }       
    }
    ///////////////////////////////////////////////////////////////

    // cosql_enable_debug(pdb, 1);

    //- and condition
    int match_and_columns_count = 2;
    sql_column_match_t match_and_columns[3];
    
    match_and_columns[0].name = "sta_active";
    match_and_columns[0].type = COLUMN_TYPE_TEXT;
    match_and_columns[0].operation = OP_TYPE_EQUAL;
    match_and_columns[0].value.t = "1";

    match_and_columns[1].name = "sta_rssi";
    match_and_columns[1].type = COLUMN_TYPE_INT;
    match_and_columns[1].operation = OP_TYPE_LESSER_THAN;
    match_and_columns[1].value.i = 0;

    if (node_mac!=NULL && strcmp(node_mac, "")!=0) {
        match_and_columns_count = 3;
        match_and_columns[2].name = "node_mac";
        match_and_columns[2].type = COLUMN_TYPE_TEXT_MAC;
        match_and_columns[2].operation = OP_TYPE_EQUAL;
        match_and_columns[2].value.t = node_mac;
    }
    ///////////////////////////////////////////////////////////////

    //- or condition
    int match_or_columns_count = 4;
    sql_column_match_t match_or_columns[4];

    match_or_columns[0].name = "sta_band";
    match_or_columns[0].type = COLUMN_TYPE_TEXT;
    match_or_columns[0].operation = OP_TYPE_EQUAL;
    match_or_columns[0].value.t = "2G";

    match_or_columns[1].name = "sta_band";
    match_or_columns[1].type = COLUMN_TYPE_TEXT;
    match_or_columns[1].operation = OP_TYPE_EQUAL;
    match_or_columns[1].value.t = "5G";

    match_or_columns[2].name = "sta_band";
    match_or_columns[2].type = COLUMN_TYPE_TEXT;
    match_or_columns[2].operation = OP_TYPE_EQUAL;
    match_or_columns[2].value.t = "5G1";

    match_or_columns[3].name = "sta_band";
    match_or_columns[3].type = COLUMN_TYPE_TEXT;
    match_or_columns[3].operation = OP_TYPE_EQUAL;
    match_or_columns[3].value.t = "6G";
    ///////////////////////////////////////////////////////////////

    int count=0;
    int query_start = start;
    int query_end = start + duration;
    char backup_db_file_path[MAX_FILE_PATH] = "\0";
    sqlite3* pdb_backup = NULL;

    json_object *rootObj = json_object_new_object();
    json_object *fieldValueArrayObj = json_object_new_array();

    while (query_start<end) {
        query_end = query_start + duration;
        if (query_end>end) query_end = end;

        int query_time = query_start + duration/2;
        int seconds = query_time % 60;

        //- remove seconds from query_time
        query_time = query_time - seconds;
        ///////////////////////////////////////////////////////////////

        int ret_value = 0;

        //- get data from tmp db.
        // fprintf(stdout, "get data from tmp db\n");
        cosql_count_matchs_between_time(pdb_tmp, 
            match_and_columns_count, match_and_columns, 
            match_or_columns_count, match_or_columns,
            query_time, 
            query_time + 59,
            &ret_value);

        if (ret_value>0) {
            goto NEXT_PERIOD;
        }
        ///////////////////////////////////////////////////////////////

        //- get data from backup db.
        char find_backup_db_file_path[MAX_FILE_PATH] = "\0";
        if (get_backup_db_path_by_datetime(db_name, "1.0,2.0,3.0", query_time, find_backup_db_file_path)==1) {
            if (strlen(backup_db_file_path)==0 || strncmp(find_backup_db_file_path, backup_db_file_path, strlen(backup_db_file_path))!=0){
                if (pdb_backup!=NULL) {
                    cosql_close(pdb_backup);
                }

                pdb_backup = cosql_open(find_backup_db_file_path);
                if (pdb_backup!=NULL) {
                    strncpy(backup_db_file_path, find_backup_db_file_path, strlen(find_backup_db_file_path));
                    fprintf(stderr, "open backup_db_file_path=%s\n", backup_db_file_path);
                }
            }

            cosql_count_matchs_between_time(pdb_backup, 
                match_and_columns_count, match_and_columns, 
                match_or_columns_count, match_or_columns,
                query_time, 
                query_time + 59,
                &ret_value);

            if (ret_value>0) {
                goto NEXT_PERIOD;
            }
        }
        ///////////////////////////////////////////////////////////////

    NEXT_PERIOD:

        fprintf(stderr, "query_start=%d -> query_end=%d, query_time=%d, count=%d, ret_value=%d\n", query_start, query_end, query_time, count, ret_value);

        query_start = query_end;

        json_object *new_obj = json_object_new_double(ret_value);
        json_object_array_add(fieldValueArrayObj, new_obj);
        ///////////////////////////////////////////////////////////////
    }

    json_object_object_add(rootObj, "count", fieldValueArrayObj);

    cosql_close(pdb_backup);
    cosql_close(pdb_tmp);

    fprintf(stderr, "Success to open db file\n");

    *retJsonObj = rootObj;

    return CODB_OK;
}