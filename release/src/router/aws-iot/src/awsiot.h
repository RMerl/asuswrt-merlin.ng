

#define CONNECT_MAX_ATTEMPT_COUNT 5
#define SUBSCRIBE_TOPIC_MAX_NUMBER 6

#define TENCENTGAME_RESET_TIME 600
#define REMOTECONNECTION_RESET_TIME 5
#define HTTPD_RESET_TIME 5
#define NEWSITE_PROVISIONING_RESET_TIME 5

#define TOPIC_TENCENTGAME 1
#define TOPIC_REMOTECONNECTION 2
#define TOPIC_HTTPD 3
#define TOPIC_NEWSITE_PROVISIONING 4

#define EXIT_AWSIOT -99

#define AWS_IOT_SESSION_BUF_LEN 131072

#define CERT_PATH_LEN 128

#define MAX_TOPIC_LENGTH 192

#define FEATURE_NAME_LENGTH 16

#define MD5_DATAT_LENGTH 33

#define SUCCESS 0

#define SHADOW_NAME_REMOTE_CONNECTION "/shadow/name/RemoteConnection/update/delta"

#define SAVE_FREE_STRING(x) \
	if (x)                  \
	{                       \
		free(x);            \
		x = NULL;           \
	}

#define AWSIOT_ACTION_REBOOT "reboot"
#define AWSIOT_ACTION_FWUPGRADE "upgradeLatestFirmware"
#define AWSIOT_ACTION_FEEDBACK "feedback"
#define AWSIOT_ACTION_ONBOARD "onboard"
#define AWSIOT_ACTION_UNBIND "unbind"
#define AWSIOT_ACTION_UPDATE_PPVERSION "update_ppversion"

#define REBOOT_MAX_WAIT_SEC 180
#define FWUPGRADE_MAX_WAIT_SEC 360

#define LOGIN_SERVER "aae-sgweb001-1.asuscomm.com"

#define IS_EULA_OR_PPV2_SIGNED() ((nvram_get_int("ASUS_EULA") == 1) || (get_ASUS_privacy_policy_state(ASUS_PP_ACCOUNT_BINDING) == 1))

enum
{
	SUB_TOPIC_TENCENTGAME = 1,
	SUB_TOPIC_ALL_TENCENTGAME,
	SUB_TOPIC_REMOTECONNECTION,
	SUB_TOPIC_HTTPD,
	SUB_TOPIC_NEWSITE_PROVISIONING,
	SUB_TOPIC_GENERAL_DELTA_SHADOW,
	SUB_TOPIC_GENERAL_GET_ACCEPTED,
	SUB_TOPIC_GENERAL
};

enum
{
	FEATURE_WIREGUARD_SERVER = 1,
	FEATURE_WIREGUARD_CLIENT,
	FEATURE_CHANGE_IP,
	FEATURE_TUNNEL_TEST,
	FEATURE_SYSTEM /* awsiot action */,
	FEATURE_WIREGUARD_SERVER_IP_CHANGE
};

enum
{
	AWSIOT_SUCCESS = 0,
	AWSIOT_WIREGUARD_CLIENT_INSTALL_CONNECT_TIMEOUT = 27000,
	AWSIOT_WIREGUARD_CLIENT_SETUP_CONNECT_TIMEOUT = 27001,
	AWSIOT_SVR_FAIL
};

char rootCA[CERT_PATH_LEN + 1];
char clientCRT[CERT_PATH_LEN + 1];
char clientPrivateKey[CERT_PATH_LEN + 1];
char CurrentWD[CERT_PATH_LEN + 1];

static char certDirectory[CERT_PATH_LEN + 1] = "/jffs/awscerts";
// static char certDirectory[CERT_PATH_LEN + 1] = "/tmp/home/root/certificates";
static char awsiot_endpoint[128];
static char awsiot_clientid[128];

// char oauth_dm_cusid[96];
// char oauth_dm_refresh_ticket[384];
// char oauth_user_email[384];

// static char wireguard_reported_template[125] = "\"%s\": {\"status\": \"%s\", \"index\": %d, \"error_code\": %d}";

void init_basic_data();

// void init_topic_subscribe();

void publish_shadow_remote_connection(const int tunnel_enable, const char *state);

void publish_router_service_topic(const char *topic, const char *msg);

void publish_shadow_httpd(const char *input, const char *api_name, const char *session_id);

// int reset_session_receive_time(const char *shadowRxBuf, const char *subscribe_topic,  const long int session_receive_time, const int topic_target);

// void asus_remote_connection(const char * shadowRxBuf, const char * subscribe_topic, const long int session_receive_time);

// void asus_httpd(const char * shadowRxBuf, const char * subscribe_topic, const long int session_receive_time);

int check_mqtt_port();

extern void publish_shadow_remote_connection(const int tunnel_enable, const char *state);
extern void publish_router_service_topic(const char *topic, const char *msg);

#ifdef RTCONFIG_MULTISITEAPP
extern void publish_shadow_tunnel_test_result(const char *source, const char *target, const char *type, int error);
extern void update_db_tunnel_test_result(const char *source, const char *target, const char *type, int error);
extern int check_wireguard_config_has_been_applied(const char *feature_name, const char *group_name, const char *unique_key);
extern int update_dynamic_db();
extern int update_db_wireguard(char *group_name, char *role, const char *content);
extern int get_db_reported_data();
#endif

#ifdef RTCONFIG_TC_GAME_ACC
extern int check_tencentgame_status();
extern int run_tc_download();
#endif

extern void enable_sub_topic(int sub_topic_id);
extern int main_loop(MQTTContext_t *pMqttContext, bool *pMqttSessionEstablished);
extern size_t hdf(char *b, size_t size, size_t nitems, void *userdata);
extern void init_topic_data();

/* api.c */
extern int split_received_tencent_session(const char *topic_url);
extern int parse_receive_remote_connection(const char *json_data, const char *subscribe_topic);
extern int parse_receive_httpd(const char *json_data, char *request_data, int request_len, char *api_name, int api_name_len, char *session_id, int session_id_len);
extern int copy_tencent_update_tmp();
extern int compare_tencent_session();
extern int tencentgame_session_id_cmp(const char *cmp_session_id, char *get_session_state, char *get_session_msg);
extern int compare_received_session_id(const char *cmp_session_id, char *session_state, char *session_update_state, int *session_error_code, const char *session_update_json);
extern int compare_received_session(const int subscribe_topic_number, int b_print);
extern int save_received_session(const char *json_data, const char *subscribe_topic);
extern int merge_received_session(const int subscribe_topic_number);
extern int read_file_data(char *filename, char *data, int data_len);
extern void get_sha256(const char *input_str, unsigned char *output_buffer);
extern int get_usb_space(unsigned long long int *total_space, unsigned long long int *used_space, unsigned long long int *available_space, char *tencent_download_path);
extern void getTimeInMillis(char *timestamp_str);
extern char *replace_str(char *st, char *orig, char *repl, char *buff);
