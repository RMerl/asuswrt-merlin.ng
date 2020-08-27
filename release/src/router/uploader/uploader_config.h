#ifndef __UPLOADER_CONFIG_H
#define __UPLOADER_CONFIG_H
#include <stdio.h>
#include "json.h"

//struct asuswebstorage_conf;

//extern struct asuswebstorage_conf asuswebstorage_conf;



struct google_conf {

  char client_id[256];
  char client_secret[256];
  char access_token[256];
  char refresh_token[256];
  char asus_folder_id[128];
  char router_folder_id[128];
  char log_folder_id[128];
  char date_folder_id[128];
  char file_id[128];

};

struct asuswebstorage_conf {

  char servicegateway[100];
  char serviceid[40];
  char key[40];
  //char otainfourl[150];
  //char otafileurl[150];
  char username[50];
  char password[128];
  char sid[40];
  char progKey[40];
  char uploader_path[50];

  char device_id[50];
  char device_name[50];
  char merchandise_type[20];
  char client_set[20];
  char device_manager_host[50];
  char client_type[20];
  char client_version[20];
  char manufacturer[50];
  char product_name[20];
  char machine_id[50];
  char uuid[26];
  char mac[15];
  char mac_name[30];

};


struct aicloud_conf {

  char username[50];
  char password[50];
  char url[100];
  char privateip[50];
  char path[50];
  char quota[16];
  char uploader_path[50];

};


struct storage_provision_conf {

  char timeStamp[25];
  char alarmAccount[30];
  char alarmToken[30];
  char alarmStorageType[15];
  char alarmStorageQuota[10];
  char alarmAppNotification[5];
  char alarmEmailNotificaiton[5];
  char recordAccount[30];
  char recordToken[30];
  char recordStorageType[10];
  char recordStorageURL[50];
  char recordStorageQuota[10];
  char recordStoragePath[30];
  char recordStoragePrivateIP[30];
};


struct aaews_event_conf {
  char type[6];
  char start_time[24];
  char device_id[32];
  char model[32];
  char value1[32];
  char value2[32];
  char value3[32];
};



struct aaews_provision_conf {

  char timeStamp[25];
  char account[30];
  char password[30];
  char token[128];
  char tokenExpireTime[15];
  char refreshToken[128];
  char deviceName[20];
  char timeZone[10];
};

struct sysinfo_conf {

  char deviceid[50];
  //char macaddr[20];
};


struct deploy_conf {

  char timeStamp[25];
  char aaeAppId[10];
  char aaeAppKey[35];
  char aaeAppPortal[40];
  char aaeSid[10];
  char aaeOAuthPortal[40];
  char awsAppId[10];
  char awsAppKey[35];
  char awsAppGateway[35];
  char awsAppPortal[40];
  char awsSid[10];
  char awsOAuthPortal[40];
  char otaInfoUrl[90];
  char otaFileUrl[50];
  char fbRtmpUrl[250];
};


struct basic_command_conf {

  char timeStamp[25];
  char cameraOnOff[5];
  char cameraLedOnOff[5];
  char deviceName[20];
  char speakerVolume[5];
  char motionAlarmOnOff[10];
  char audioAlarmOnOff[10];
  char recordOnOff[10];
  char wdrOnOff[5];
  char ldcOnOff[5];
};

struct system_conf {

  char fwver[20];
  char tcode[20];
  char sn[32];
  char apilevel[3];
  char recordplan[10];
  char privateip[20];
  char macaddr[20];
  char videomode[7];
  char initattachabledevice[6];
};



// google token
void google_config_parse(json_object * jobj ,struct google_conf * google_d_conf);
int process_google_config(struct google_conf * google_d_conf);

// google refresh token
int process_google_refresh_token(char * filename, struct google_conf * google_d_conf);
int google_refresh_token_parse(json_object * jobj ,struct google_conf * google_d_conf);


// google folder data
int process_google_drive_data(char * filename, struct google_conf * google_d_conf, char * foldername, char * data_type);


void google_create_folder_parse(json_object * jobj ,struct google_conf * google_d_conf, char * foldername, char * data_type);
int process_google_create_folder(char * filename, struct google_conf * google_d_conf, char * foldername, char * data_type);



// aicloud
int process_aicloud_config(struct aicloud_conf * ai_conf);
void aicloud_config_parse(json_object * jobj, struct aicloud_conf * ai_conf);



// webstorage
int process_asuswebstorage_config(struct asuswebstorage_conf * asus_conf);
void asuswebstorage_config_parse(json_object * jobj, struct asuswebstorage_conf * asus_conf);

// storage provision
int process_storage_provision_config(struct storage_provision_conf * sp_conf, char * file_path);
void storage_provision_config_parse(json_object * jobj, struct storage_provision_conf * sp_conf);

// aaews provision
int process_aaews_provision_config(struct aaews_provision_conf * ap_conf);
void aaews_provision_config_parse(json_object * jobj, struct aaews_provision_conf * ap_conf);

// aawes
int process_aaews_even_conf(char * filename, struct aaews_event_conf * ae_conf);
void aaews_event_conf_parse(json_object * jobj, struct aaews_event_conf * ae_conf);

// sysinfo
int process_sysinfo_config(struct sysinfo_conf * si_conf);
void sysinfo_config_parse(json_object * jobj, struct sysinfo_conf * si_conf) ;

// deploy
int process_deploy_config(struct deploy_conf * dp_conf);
void deploy_config_parse(json_object * jobj, struct deploy_conf * dp_conf) ;

// basic_command
int process_basic_command_config(struct basic_command_conf * bc_conf);
void basic_command_config_parse(json_object * jobj, struct basic_command_conf * bc_conf) ;

// system
int process_system_config(struct system_conf * s_conf, char * file_path);

void system_config_parse(json_object * jobj, struct system_conf * s_conf) ;


#endif