
#define TENCENT_SESSION_FILE 					"/jffs/tencent_session.json"
#define TENCENT_SESSION_REPORT_FILE 			"/jffs/tencent_session_update.json"
#define TENCENT_SESSION_PUBLISH_FILE 			"/jffs/tencent_session_publish.json"
#define TENCENT_SESSION_TMP						"/tmp/tencent_session_tmp"
#define TENCENT_SESSION_TMP_ONE					"/tmp/tencent_session_tmp1"
#define TENCENT_SESSION_TMP_TWO					"/tmp/tencent_session_tmp2"
#define TENCENT_SESSION_TMP_THREE				"/tmp/tencent_session_tmp3"
#define TENCENT_SESSION_CHANGE_FILELIST			"/tmp/tencent_session_filelist_tmp"
#define TENCENT_SESSION_JSON_TMP				"/tmp/tencent_session_json_tmp"

#define TENCENT_SESSION_TOPIC					"/tmp/tencent_session_topic"
#define TENCENT_SESSION_OTHER					"/tmp/tencent_session_other"

#define TC_DOWNLOAD_PID_FILE 	                 "/var/run/tc_download.pid"

#define AWS_IOT_ROOT                             "$aws/things/"



#define JSON_CONTENT_SIZE  132072



// #define START_COMPARE 1
// #define RESTART_COMPARE 2

int get_session_data(const char* json_data, char * out_data);

int parse_json_format(const char* json_data);

int write_file(char *filename, char *json_data);


int read_file_data(char *filename, char *data, int data_len);

int parse_download_update(char* publish_data, char* awsiot_clientid, char *sn_hash);

int compare_tencent_session();


int run_subscribe_channel();

int receive_file_process(const int current_subscribe_topic_position);

int publish_subscribe_channe();

int publish_subscribe_channel_old();

void run_subscribe_channel_old_2020110();

void get_sha256(const char *input_str, unsigned char * output_buffer);

// int get_usb_space(long *total_space, long *used_space, long *available_space, char *tencent_download_path);
int get_usb_space(unsigned long long int *total_space, unsigned long long int *used_space, unsigned long long int *available_space, char *tencent_download_path);


int copy_tencent_update_tmp();

int save_received_session(const char* json_data, const char* subscribe_topic);

int merge_received_session(const int subscribe_topic_number);

int compare_received_session(const int subscribe_topic_number, int b_print);


void getTimeInMillis(char* timestamp_str);