#ifndef __AI_SERVICE_H__
#define __AI_SERVICE_H__

#include "ai_lib.h"

#define AI_REQUEST_RETRY               3
#define AI_REQUEST_TIMEOUT            10
#define AI_REQUEST_WAIT_TIMEOUT       60
#define AI_REQUEST_WAIT_INTERVAL       2

/* AISOM action*/
typedef enum {
	AISOM_REBOOT=16,
	AISOM_MSG=21
} ai_action_t;
typedef enum {
	AISOM_GPIO_HIGH=0,
	AISOM_GPIO_LOW=255
} ai_volt_t;

/* Docker apps on AISOM */
typedef struct {
	char *filename;
	char *repository;
	char *tag;
	char *filepath;
	char *nv_key;
	char *md5sum;
	char *extra_command;
} ai_image_t;

#define IMAGE_BASE_PATH "/ai/docker_images"

/* LOG LEVEL */
typedef enum {
	AI_LOG_ERROR,
	AI_LOG_WARN,
	AI_LOG_INFO,
	AI_LOG_DEFAULT,
	AI_LOG_DEBUG
} ai_log_level_t;

static void md5_to_string(const unsigned char *, char *);
static int compute_md5(const char *, unsigned char *);
static int read_md5(const char *, char *);
static int compare_md5(unsigned char *, const char *);
static int recv_ai_response(char *, char *, char *, char *, char *);
static int response_parser(const char *);
static int gen_docker_images_list(int);
static int gen_msg_key(char *);
static int encrypt_json(char *, char *);
static int compute_enc_json_md5(char *, char *);
static int encrypt_aes_key(char *, char *, char *);
static int trigger_aisom(ai_volt_t, ai_action_t);
static char *ai_level_to_str(ai_log_level_t);
void aiprint(ai_log_level_t, const char * logpath, const char * format, ...);
int write_to_log_overwrite(const char *log_file_path, const char *data);

#define AI_LOG(log_level, logpath, fmt, args...)                      \
    do {                                                              \
        aiprint(log_level, logpath, "[AISRV][%s] " fmt, ai_level_to_str(log_level), ##args); \
    } while(0)

/* LOG PATH */
#define AI_LOG_PATH "/ai/log/ai_service.log"
#define AI_LOG_DKR_IMG_PATH "/ai/log/ai_docker_images.log"
#define AI_LOG_DKR_CTN_PATH "/ai/log/ai_docker_container.log"

/* REQ    -> RES
 * query  -> system 
 * update -> progress
 * apply  -> save
 * reset  -> default
 * feedback -> feedback
 * rescue ->
 * docker -> docker
 * app    -> app
 */
#define MAX_RSP 7
#define MAX_CMD_LEN 16

#define MD5_HASH_SIZE 32
#define AISRV "[AISRV]"

#define MAX_DNS_NUM 2

#define EVENT_SIZE (sizeof(struct inotify_event))
#define BUF_LEN (1024 * (EVENT_SIZE + 16))

#define AI_STA_REQ 1<<0
#define AI_UPD_REQ 1<<1
#define AI_SET_REQ 1<<2
#define AI_RST_REQ 1<<3
#define AI_FBK_REQ 1<<4
#define AI_RSC_REQ 1<<5
#define AI_DKR_REQ 1<<6
#define AI_APP_REQ 1<<7

#define AI_NVM_FW_VERSION "ai_sys_fw_version"
#define AI_NVM_HW_VERSION "ai_sys_hw_version"
#define AI_NVM_HW_REVISION "ai_sys_hw_revision"
#define AI_NVM_PROD_DATE "ai_sys_prod_date"
#define AI_NVM_SERIAL_NUMBER "ai_sys_serial_number"

#define AI_NVM_AIBOARD_MAC "aiboard_macaddr"
#define AI_SDK_VER_CODE "ANPU"

/* request stage, 'ongoing' or 'success' */
#define AI_REQ_ST_START "ongoing"
#define AI_REQ_ST_COMPLETE "success"

#define AI_REQ_QUERY     "query"
#define AI_REQ_UPDATE    "update"
#define AI_REQ_APPLY     "apply"
#define AI_REQ_RESET     "reset"
#define AI_REQ_FEEDBACK  "feedback"
#define AI_REQ_RSC       "rescue"
#define AI_REQ_DKR       "docker"
#define AI_REQ_APP       "app"

/* file path for request */
#define BASE_PATH    "/ai/ctrl_msg"
#define BASE_KEY     "/ai/ctrl_msg/key.hex"
#define BASE_IV      "/ai/ctrl_msg/iv.hex"
#define ENC_REQ_MD5  "/ai/ctrl_msg/REQ_MSG.md5"
#define ENC_REQ_MSG  "/ai/ctrl_msg/REQ_MSG.enc"
#define ENC_REQ_AES  "/ai/ctrl_msg/REQ_KEY.enc"
#define DEC_REQ_AES  "/ai/ctrl_msg/REQ_KEY.aes"
#define DEC_REQ_JSON "/ai/ctrl_msg/REQ_MSG.json"

#define ENC_RESCUE_REQ_MD5  "/ai/ctrl_msg/RESCUE_REQ_MSG.md5"
#define ENC_RESCUE_REQ_MSG  "/ai/ctrl_msg/RESCUE_REQ_MSG.enc"
#define ENC_RESCUE_REQ_AES  "/ai/ctrl_msg/RESCUE_REQ_KEY.enc"
#define DEC_RESCUE_REQ_AES  "/ai/ctrl_msg/RESCUE_REQ_KEY.aes"
#define DEC_RESCUE_REQ_JSON "/ai/ctrl_msg/RESCUE_REQ_MSG.json"

#define ENC_IMG_MD5  "/ai/docker_images/docker_images_list_enc.md5"
#define ENC_IMG_MSG  "/ai/docker_images/docker_images_list.enc"
#define ENC_IMG_AES  "/ai/docker_images/docker_images_list_key.enc"
#define DEC_IMG_AES  "/ai/docker_images/docker_images_list.aes"
#define DEC_IMG_JSON "/ai/docker_images/docker_images_list.json"

/* file path for response and fwup from ai board */
#define ENC_RSP_MD5  "/ai/ctrl_msg/RSP_MSG.md5"
#define ENC_RSP_MSG  "/ai/ctrl_msg/RSP_MSG.enc"
#define ENC_RSP_AES  "/ai/ctrl_msg/RSP_KEY.enc"
#define DEC_RSP_AES  "/ai/ctrl_msg/RSP_KEY.aes"
#define DEC_RSP_JSON "/ai/ctrl_msg/RSP_MSG.dec"

#define ENC_FWUP_MD5  "/ai/ctrl_msg/FWUP_MSG.md5"
#define ENC_FWUP_MSG  "/ai/ctrl_msg/FWUP_MSG.enc"
#define ENC_FWUP_AES  "/ai/ctrl_msg/FWUP_KEY.enc"
#define DEC_FWUP_AES  "/ai/ctrl_msg/FWUP_KEY.aes"
#define DEC_FWUP_JSON "/ai/ctrl_msg/FWUP_MSG.dec"

#define ENC_DKR_MD5  "/ai/ctrl_msg/DOCKER_MSG.md5"
#define ENC_DKR_MSG  "/ai/ctrl_msg/DOCKER_MSG.enc"
#define ENC_DKR_AES  "/ai/ctrl_msg/DOCKER_KEY.enc"
#define DEC_DKR_AES  "/ai/ctrl_msg/DOCKER_KEY.aes"
#define DEC_DKR_JSON "/ai/ctrl_msg/DOCKER_MSG.dec"

#define ENC_APP_MD5  "/ai/ctrl_msg/APP_MSG.md5"
#define ENC_APP_MSG  "/ai/ctrl_msg/APP_MSG.enc"
#define ENC_APP_AES  "/ai/ctrl_msg/APP_KEY.enc"
#define DEC_APP_AES  "/ai/ctrl_msg/APP_KEY.aes"
#define DEC_APP_JSON "/ai/ctrl_msg/APP_MSG.dec"

#define ENC_RESCUE_MD5  "/ai/ctrl_msg/RESCUE_MSG.md5"
#define ENC_RESCUE_MSG  "/ai/ctrl_msg/RESCUE_MSG.enc"
#define ENC_RESCUE_AES  "/ai/ctrl_msg/RESCUE_KEY.enc"
#define DEC_RESCUE_AES  "/ai/ctrl_msg/RESCUE_KEY.aes"
#define DEC_RESCUE_JSON "/ai/ctrl_msg/RESCUE_MSG.dec"

#define ENC_STAT_MD5  "/ai/ctrl_msg/STAT_MSG.md5"
#define ENC_STAT_MSG  "/ai/ctrl_msg/STAT_MSG.enc"
#define ENC_STAT_AES  "/ai/ctrl_msg/STAT_KEY.enc"
#define DEC_STAT_AES  "/ai/ctrl_msg/STAT_KEY.aes"
#define DEC_STAT_JSON "/ai/ctrl_msg/STAT_MSG.dec"

/* router(rsa private key) ai board(rsa public key)*/
#define ROUTER_KEY_BASE_PATH    "/tmp/etc"
#define ROUTER_PRIVATE_KEY      "router_priv.pem"
#define ROUTER_PUBLIC_KEY       "router_pub.pem"
#define AISOM_RESCUE_PRIVATE_KEY "aisom_reset_priv.pem"
#define AISOM_RESCUE_PUBLIC_KEY  "aisom_reset_pub.pem"
#define AISOM_DUMMY_PRIVATE_KEY "aisom_dummy_priv.pem"
#define AISOM_DUMMY_PUBLIC_KEY  "aisom_dummy_pub.pem"
#define AISOM_PUBLIC_KEY        "aisom_pub.pem"
#define AI_TEST_FILE            "keytest.txt"
#define AI_ENC_TEST_FILE        "keytest.enc"
#define AI_DEC_TEST_FILE        "keytest.dec"

#define DEFAULT_FW_PATH   "firmware/aisom.swu"
#define MONITOR_RSP_FILE  "RSP_MSG.md5"
#define MONITOR_FWUP_FILE "FWUP_MSG.md5"
#define MONITOR_DKR_FILE "DOCKER_MSG.md5"
#define MONITOR_APP_FILE "APP_MSG.md5"
#define MONITOR_RESCUE_FILE "RESCUE_MSG.md5"
#define MONITOR_STAT_FILE "STAT_MSG.md5"

/* PROGRESS STATUS */
#define AI_PROG_DONE "DONE"
#define AI_PROG_PROG "PROGRESS"
#define AI_PROG_SUCC "SUCCESS"
#define AI_PROG_DNLD "DOWNLOAD"
#define AI_PROG_DKDN "DOCKER_DONE"
#define AI_PROG_DKFL "failed"

#endif
