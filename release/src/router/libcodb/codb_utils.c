#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include "codb_utils.h"
#include "cosql_utils.h"
#include "log.h"
#include <shared.h>

static time_t get_zero_time_on_day(unsigned long ts){
    time_t current_time, zero_time;

    if(ts > 0)
        current_time = ts;
    else
        current_time = time(NULL);
    zero_time = (((current_time/675)>>7)*675)<<7;

    return zero_time;
}

static int get_tmp_db_path(const char* db_name, char* db_file_path) {

    if (db_name==NULL || db_file_path==NULL) {
        return 0;
    }

    char db_path[12] = "/tmp/.diag/";

    strncpy(db_file_path, db_path, strlen(db_path));
    strncat(db_file_path, db_name, strlen(db_name));
    strncat(db_file_path, ".db", 3);

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

    time_t timestamp_day = get_zero_time_on_day(query_datetime);
    
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
    
    if (data==NULL || data_count<=0) {
        return NULL;
    }

    //- ex. data
    //- name>type(txt/int)>value>operation

    sql_column_match_t* match_columns = NULL;

    char *saveptr1=NULL, *saveptr2=NULL;
    match_columns = malloc(sizeof(sql_column_match_t)*data_count);
    sql_column_match_t* query_match_columns_idx = match_columns;
    
    char* pch = strdup(data);
    pch = strtok_r(pch, ";", &saveptr1);
    while(pch!=NULL){

        // fprintf(stderr, "pch=%s\n", pch);

        int idx = 0;
        char* pch2 = strdup(pch);
        pch2 = strtok_r(pch2, ">", &saveptr2);
        while (pch2!=NULL) {

            int len = strlen(pch2);

            if (idx==0) {
                //- column name
                query_match_columns_idx->name = (char *)malloc(sizeof(char)*len);
                memset(query_match_columns_idx->name, 0, len);
                strncpy(query_match_columns_idx->name, pch2, len);
            }
            else if (idx==1) {
                //- column type
                if (strncmp(pch2, "txt", 3)==0) {
                    query_match_columns_idx->type = COLUMN_TYPE_TEXT;
                }
                else if (strncmp(pch2, "int", 3)==0) {
                    query_match_columns_idx->type = COLUMN_TYPE_INT;
                }
            }
            else if (idx==2) {
                //- column value
                if (query_match_columns_idx->type==COLUMN_TYPE_TEXT ||
                    query_match_columns_idx->type==COLUMN_TYPE_TEXT_MAC ||
                    query_match_columns_idx->type==COLUMN_TYPE_TEXT_IP ||
                    query_match_columns_idx->type==COLUMN_TYPE_TEXT_JSON) {
                    query_match_columns_idx->value.t = (char *)malloc(sizeof(char)*len);
                    memset(query_match_columns_idx->value.t, 0, len);
                    strncpy(query_match_columns_idx->value.t, pch2, len);
                }
                else if (query_match_columns_idx->type==COLUMN_TYPE_INT) {
                    query_match_columns_idx->value.i = atoi(pch2);
                }
            }
            else if (idx==3) {
                query_match_columns_idx->operation = atoi(pch2);
            }

            pch2 = strtok_r(NULL, ">", &saveptr2);

            idx++;
        }

        if (pch2!=NULL) {
            free(pch2);
        }

        pch = strtok_r(NULL, ";", &saveptr1);

        // fprintf(stderr, "query_match_columns_idx->name=%s\n", query_match_columns_idx->name);
        // fprintf(stderr, "query_match_columns_idx->value.t=%s\n", query_match_columns_idx->value.t);

        query_match_columns_idx++;
    }

    if (pch!=NULL) {
        free(pch);
    }

    return match_columns;   
}

static sql_column_prototype_t* parse_query_columns_data(char* data, int data_count) {

    if (data==NULL || data_count<=0) {
        return NULL;
    }

    //- ex. data
    //- name

    sql_column_prototype_t* query_columns = NULL;

    query_columns = malloc(sizeof(sql_column_prototype_t)*data_count);
    sql_column_prototype_t* query_columns_idx = query_columns;

    char* pch = strdup(data);
    pch = strtok(pch, ";");
    while (pch!=NULL) {

        int len = strlen(pch);

        //- column name
        query_columns_idx->name = (char *)malloc(sizeof(char)*len);
        memset(query_columns_idx->name, 0, len);
        strncpy(query_columns_idx->name, pch, len);

        //- column type
        query_columns_idx->type = COLUMN_TYPE_TEXT;
        
        pch = strtok(NULL, ";");

        // fprintf(stderr, "query_columns->name=%s\n", query_columns->name);

        query_columns_idx++;
    }

    if (pch!=NULL) {
        free(pch);
    }

    return query_columns;
}

int codb_test() {

    fprintf(stderr, "run codb_test\n");

    int res = 0;

    char content[50] = "sta_mac;txtps;txpr;data_time;sta_active";

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

    int query_duration = 7200; //- 2 hour
    int query_end_time = 1641690995; //- GMT: 2022年1月9日Sunday 01:16:35
    // int query_end_time = (unsigned)time(NULL);
    int query_start_time = query_end_time - query_duration;
    int qyery_limit = 0;

    char filter_data[100] = "node_mac>txt>04:D9:F5:B5:93:E0>0;sta_mac>txt>04:D9:F5:B5:94:78>0;sta_active>int>0>2";
    
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

    res = codb_content_query_json_field("stainfo", column_count, content, filter_count, filter_data, query_start_time, query_end_time, qyery_limit, &retObj);
    // res = codb_content_query_duration_json_field("stainfo", column_count, content, filter_count, filter_data, query_start_time, query_end_time, 60, &retObj);
    
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

int codb_content_query_json_field(char* db_name, int columns_count, char* columns_name, int filter_count, char* filter_data, int start, int end, int limit, json_object **retJsonObj) {
    
    if (db_name==NULL || columns_name==NULL) {
        return CODB_ERROR;
    }

    if (start>end) {
        return CODB_ERROR;
    }

    sqlite3* pdb_tmp = NULL;
    char tmp_db_file_path[MAX_FILE_PATH] = "\0";
    if (get_tmp_db_path(db_name, tmp_db_file_path)==1) {
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
    sqlite3* pdb_backup = NULL;
    ///////////////////////////////////////////////////////////////

    sql_column_prototype_t* query_columns = parse_query_columns_data(columns_name, columns_count);

    // sql_column_prototype_t* query_columns = malloc(sizeof(sql_column_prototype_t)*columns_count);
    // sql_column_prototype_t* query_columns_idx = query_columns;
    // char* pch = strdup(columns_name);
    // pch = strtok(pch, ";");
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

    // if (pch!=NULL) {
    //     free(pch);
    // }

    fprintf(stdout, "columns_count=%d, columns_name=%s, start=%d, end=%d\n", columns_count, columns_name, start, end);
    ///////////////////////////////////////////////////////////////

    int i, j, start_index, array_index, ret_rows = 0, count=0;
    char **result;
    char *column_value = NULL;

    json_object *resultValueArrayObj = json_object_new_array();

    int total_ret_rows = 0;
    int query_start = start;
    int query_end = 0;
    int query_all_data = (start==0&&end==0) ? 1 : 0;

    while (query_start<end || query_all_data==1) {

        if (query_all_data==0) {
            time_t timestamp_day = get_zero_time_on_day(query_start) + 86400;
            query_end = timestamp_day;
            if (query_end>end) query_end = end;
        }

        //- get data from tmp db.
        int ret = cosql_get_column_values(pdb_tmp,
            match_and_columns_count, match_and_columns,
            0, NULL,
            columns_count, query_columns,
            query_start, 
            query_end,
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
                query_end,
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

        fprintf(stdout, "[%d] ret=%d, query_start=%d, query_end=%d, ret_rows=%d\n", ret, count, query_start, query_end, ret_rows);

        query_start = query_end;
        
        count++;

        total_ret_rows = total_ret_rows + ret_rows;

        if(query_all_data==1) {
            break;
        }
    }
    
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

int codb_content_query_duration_json_field(char* db_name, int columns_count, char* columns_name, int filter_count, char* filter_data, int start, int end, int duration, json_object **retJsonObj) {
    
    if (db_name==NULL || columns_name==NULL) {
        return CODB_ERROR;
    }

    if (start>end) {
        return CODB_ERROR;
    }

    sqlite3* pdb_tmp = NULL;
    char tmp_db_file_path[MAX_FILE_PATH] = "\0";
    if (get_tmp_db_path(db_name, tmp_db_file_path)==1) {
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
    if (get_tmp_db_path(db_name, tmp_db_file_path)==1) {
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
    if (get_tmp_db_path(db_name, tmp_db_file_path)==1) {
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
    if (get_tmp_db_path(db_name, tmp_db_file_path)==1) {
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