

#define CONNECT_MAX_ATTEMPT_COUNT  		5
#define SUBSCRIBE_TOPIC_MAX_NUMBER 		6

#define TENCENTGAME_RESET_TIME  		600
#define REMOTECONNECTION_RESET_TIME		5 
#define HTTPD_RESET_TIME				5 
#define NEWSITE_PROVISIONING_RESET_TIME	5 

#define TOPIC_TENCENTGAME       		1
#define TOPIC_REMOTECONNECTION  		2
#define TOPIC_HTTPD				  		3
#define TOPIC_NEWSITE_PROVISIONING      4

#define EXIT_AWSIOT 					-99

#define AWS_IOT_SESSION_BUF_LEN 		131072

#define CERT_PATH_LEN					128

#define MAX_TOPIC_LENGTH 				192

#define FEATURE_NAME_LENGTH 			16

#define SUCCESS 						0

#define SHADOW_NAME_REMOTE_CONNECTION            "/shadow/name/RemoteConnection/update/delta"

#define SAVE_FREE_STRING(x) \
    if(x) { \
      free(x); \
	  x=NULL; \
	}

enum {
	SUB_TOPIC_TENCENTGAME = 1,
	SUB_TOPIC_ALL_TENCENTGAME,
	SUB_TOPIC_REMOTECONNECTION,
	SUB_TOPIC_HTTPD,
	SUB_TOPIC_NEWSITE_PROVISIONING,
	SUB_TOPIC_GENERAL_DELTA_SHADOW,
	SUB_TOPIC_GENERAL_GET_ACCEPTED,
	SUB_TOPIC_GENERAL
};

enum {
	FEATURE_WIREGUARD_SERVER = 1,
	FEATURE_WIREGUARD_CLIENT,
	FEATURE_CHANGE_IP,
	FEATURE_TUNNEL_TEST
};

char rootCA[CERT_PATH_LEN + 1];
char clientCRT[CERT_PATH_LEN + 1];
char clientPrivateKey[CERT_PATH_LEN + 1];
char CurrentWD[CERT_PATH_LEN + 1];


static char certDirectory[CERT_PATH_LEN + 1] = "/jffs/awscerts";
// static char certDirectory[CERT_PATH_LEN + 1] = "/tmp/home/root/certificates";
static char awsiot_endpoint[128];
static char awsiot_clientid[128];

char aae_deviceid[64];
char oauth_dm_cusid[96];
char oauth_dm_refresh_ticket[384];
char oauth_user_email[384];

// static char wireguard_reported_template[125] = "\"%s\": {\"status\": \"%s\", \"index\": %d, \"error_code\": %d}";

void init_basic_data();

void init_topic_subscribe();


void publish_shadow_remote_connection(const int tunnel_enable, const char *state);

void publish_router_service_topic(const char *topic, const char *msg);

void publish_shadow_httpd(const char *input, const char *api_name, const char *session_id);


int reset_session_receive_time(const char *shadowRxBuf, const char *subscribe_topic,  const long int session_receive_time, const int topic_target);

void asus_remote_connection(const char * shadowRxBuf, const char * subscribe_topic, const long int session_receive_time);

void asus_httpd(const char * shadowRxBuf, const char * subscribe_topic, const long int session_receive_time);


void tencentgame_data_process(const char *shadowRxBuf, const char *subscribe_topic);

int check_mqtt_port();
