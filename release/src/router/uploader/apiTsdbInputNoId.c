#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>
#include "uploader_config.h"

#include "log.h"

#define TSDB_DBG 0


extern struct asuswebstorage_conf asus_conf;

int toolComposeAuthorizationHeader(char *progKey);

int toolCurl(int api_id,char* api_url, char* authorization, char* cookie, char* payload, char* action, char* service, char* sid, char* token);

extern char authorization[512];


int apiTsdbInputNoId(struct asuswebstorage_conf *asus_conf, char* data_time, char* msg_type, char* device_id, char* model, char* type_value1, char* type_value2, char* type_value3)
{


  char api_url[128],payload[512],action[50],service[50],sidStr[50];  


  //snprintf(api_url,sizeof(api_url),"https://tsdbase01.asuswebstorage.com/tsdbase/entry");  
  snprintf(api_url,sizeof(api_url),"https://tsdbase02.asuswebstorage.com/tsdbase/entry");  
  
  //snprintf(payload,sizeof(payload),"{\"schemaname\":\"IOT\",\"entries\":[{\"device\":\"%s\",\"time\":\"2015-12-30 12:33:28\",\"type\":\"run01\",\"value1\":\"3km\",\"value2\":\"10mins\"}]}",device);
  snprintf(payload,sizeof(payload),"{\"schemaname\":\"VideoRecord\",\"entries\":[{\"start_time\":\"%s\",\"device_id\":\"%s\",\"type\":\"%s\",\"model\":\"%s\",\"value1\":\"%s\",\"value2\":\"%s\",\"value3\":\"%s\"}]}", data_time, device_id, msg_type, model, type_value1, type_value2, type_value3);


  snprintf(action,sizeof(action),"X-Omni-Action: PutEntries");
  //snprintf(service,sizeof(service),"X-Omni-Service: IOT%s",sid);
  // snprintf(service,sizeof(service),"X-Omni-Service: AiCAM%s","");
  snprintf(service,sizeof(service),"X-Omni-Service: IOT32070675");
  
  // snprintf(sidStr,sizeof(sidStr),"X-Omni-Sid: %s", asus_conf->sid);
  snprintf(sidStr,sizeof(sidStr),"X-Omni-Sid: %s", "32070675");


  Cdbg(TSDB_DBG, "api_url -> %s", api_url);
  Cdbg(TSDB_DBG, "payload -> %s", payload);
  Cdbg(TSDB_DBG, "action -> %s", action);
  Cdbg(TSDB_DBG, "service -> %s", service);
  Cdbg(TSDB_DBG, "sidStr -> %s", sidStr);
  // Cdbg(TSDB_DBG, "progKey -> %s", asus_conf->progKey);

  int return_value = 0;

   // tool_composeAuthorizationHeader(asus_conf->progKey);
  tool_composeAuthorizationHeader("3373647E36B04936B6BC362948F067CA");

// X-Omni-Action: PutEntries
// X-Omni-Service: AiCAM
// X-Omni-Sid: 32070675

// Authorization: signature_method="HMAC-SHA1",timestamp="1477982338204479",nonce="ad071827ac174952aeddef63c12831d2",signature="o8kbywl6AdeWOtR4qzLGaLQ3HNA%3D"
//Authorization: signature_method="HMAC-SHA1",timestamp="1477982437972432",nonce="ad071827ac174952aeddef63c12831d2",signature="ZA7blYeUIC565WgxhmwHUEHTu8Q%3D"

  
  return_value = tool_curl(1,api_url,authorization,NULL,payload,action,service,sidStr,NULL);

  //return_value = tool_curl(1,api_url,authorization,NULL,payload, NULL, NULL, NULL, NULL);
  
  return return_value; 
}



