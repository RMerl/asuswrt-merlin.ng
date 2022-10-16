#ifndef __CODB_UTILS_H__
#define __CODB_UTILS_H__

#include <json.h>

#define CODB_OK           0   /* Successful result */
#define CODB_ERROR        1   /* Generic error */

#define MAX_FILE_PATH 125

#define DIAG_DB_FOLDER ".diag"

#define JFFS_DIR	"/jffs"

extern int codb_test();

extern int codb_content_query_json_field(char* db_name, int columns_count, char* columns_name, int filter_count, char* filter_data, int start, int end, int limit, json_object **retJsonObj);
extern int codb_content_query_duration_json_field(char* db_name, int columns_count, char* columns_name, int filter_count, char* filter_data, int start, int end, int duration, json_object **retJsonObj);
extern int codb_avg_query_json_field(char* db_name, char* field_name, char* node_mac, int start, int end, int duration, json_object **retJsonObj);
extern int codb_eth_detect_traffic_data(char* node_mac, int is_bh, int start, int end, int duration, json_object **retJsonObj);
// extern int codb_count_query_json_field(const char* db_name, const char* field_name, const char* node_mac, int start, int end, int duration, json_object **retJsonObj);

extern int codb_count_active_client(char* node_mac, int start, int end, int duration, json_object **retJsonObj);
#endif
