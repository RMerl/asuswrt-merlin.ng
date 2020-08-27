
#include <time.h>
//#include <unistd.h>
#include <curl/curl.h>
#include <stdio.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/HTMLparser.h>
#include <libxml/xmlreader.h>

// system param
#include <sys/types.h>
#include <sys/wait.h> 

#include <stdlib.h>
#include <string.h>
#include <openssl/sha.h>
#include <fcntl.h>
#include <dirent.h>
//#include <sha.h>
#include <sys/stat.h>
#include <time.h>
#include <utime.h>
//#include <sys/time.h>
//#include <sys/socket.h>
#include "api.h"
#include "data.h"
#include "function.h"

#include "uploader_config.h"

#include "log.h"



#define API_DBG 1

int DEBUG = 1;


#define BUFSIZE 1024*16
#define UP_QUEUE 0
#define STOP_DOWNLOAD_AFTER_THIS_MANY_BYTES         6000
#define MINIMAL_PROGRESS_FUNCTIONALITY_INTERVAL     3

typedef enum time_status
{
    older,
    newer,
    same,
    new_file
} time_status;


typedef enum rename_type_t
{
    createfolder_action,
    createfile_action,
    rename_action,
    move_action
} rename_type ;

typedef struct server_capacity_tag
{
    int total;
    int used;
} server_capacity;

server_capacity pre_cap;

/*
2015.05.22 update
oid sid = 83379331, progKey = 03805FDC48594FDEA89183D2ADA82586

20160302 update 32070675  (is OK)
old sid = 32070675; ProgKey = 3373647E36B04936B6BC362948F067CA
ClientSecret = 3373647E36B04936B6BC362948F067CA ???

for oauth ClientID 對應ClientSecret
AiCam Smart 39747443  ClientSecret 217c033226064413805e2f2f775b4b8c
AiCam Pro 37871548 ClientSecret  3ca8de061b254646a10d9a5d4cd15825

AiCam Smart 39747443  ProgKey ： 74D5FB05D8DB4D75BDDC28FF6166002E
AiCam Pro 37871548 ProgKey ： 4FFB72DB12234FCFB1887E250F1C5CD6
*/

char *sid = "32070675";
char *progKey = "3373647E36B04936B6BC362948F067CA";

char *VERSION = "1.0.0.4";
Servicegateway sergate;


extern struct aaews_provision_conf ap_conf;

extern struct google_conf google_d_conf;


extern Aaa aaa;
extern char username[256];
extern char password[256];

extern char config_upload_path[128];
extern char config_download_path[128];

extern int space_plan;
extern int space_plan_tmp;

extern int oauth_token;

extern queue_t queue_download;
extern queue_t queue_upload;
extern struct sync_item *down_head;
//extern sync_item_t down_head;
extern struct sync_item *up_head;
//extern int sync_up;
//extern int sync_down;
extern int MySyncFolder;
extern int exit_loop;
//extern int uploading;
//extern int downloading;
extern struct sync_item *from_server_sync_head;
extern sync_item_t up_excep_fail;
//extern sync_item_t dragfolder_recursion_head;
extern sync_item_t download_only_socket_head;
extern int upload_only;
double start_time;
//extern char uploadfile_path[NORMALSIZE];
//extern char downloadfile_path[NORMALSIZE];
extern int server_space_full;
extern int local_space_full;
//extern int init_fail_item;
extern int max_upload_filesize;
extern int copying_file_number;
//extern char otp_key[8];
extern struct asus_config cfg;

extern struct asuswebstorage_conf asus_conf;
extern struct deploy_conf dp_conf;

extern int IsAccountFrozen;
int MyRecycleID;
extern int IsSyncError;
extern char token_filename[256];
extern char record_token_file[256];
extern int no_completed;
extern int pre_seq;
extern Hb_SubNode *SyncNode;
extern my_mutex_t wait_server_mutex;
extern api_count_s api_count;

typedef struct cmp_item
{
    time_status status;
    int id;
    long long size;
}cmp_item_t ;

int is_exist_case_conflicts(char *fullname,char *pre_name);
/*
int my_setsockopt_func(void *clientp, curl_socket_t curlfd, curlsocktype purpose)
{
    Cdbg(API_DBG, "entern my_setsockopt_func\n");
    struct timeval timeout;
    timeout.tv_sec = 20;
    timeout.tv_usec = 0;

     if(setsockopt(curlfd,SOL_SOCKET,SO_SNDTIMEO,(char *)&timeout,sizeof(struct timeval)))
    {
         perror("*********setsockopt send is error:*******");
     }
     else
     {
         Cdbg(API_DBG, "*********setsockopt send is ok:*******\n");
     }
     setsockopt(curlfd,SOL_SOCKET,SO_RCVTIMEO,(char *)&timeout,sizeof(struct timeval));
}
*/

size_t my_write_func(void *ptr, size_t size, size_t nmemb, FILE *stream)

{
   if(exit_loop)
        return -1;

  int len ;
  len = fwrite(ptr, size, nmemb, stream);
  //Cdbg(API_DBG, "write len is %d\n",len);
  return len;

}

static size_t
WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
  //Cdbg(API_DBG, "contents is %s\n",contents);

  size_t realsize = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)userp;

  mem->memory = realloc(mem->memory, mem->size + realsize + 1);
  if (mem->memory == NULL) {
    /* out of memory! */
    Cdbg(API_DBG, "not enough memory (realloc returned NULL)\n");
    exit(EXIT_FAILURE);
  }

  memcpy(&(mem->memory[mem->size]), contents, realsize);
  //strcpy(mem->memory,(char *)contents);
  //Cdbg(API_DBG, "!!!!!!!!! mem->memory is %s\n",mem->memory);
  mem->size += realsize;
  mem->memory[mem->size] = 0;

  return realsize;
}


size_t my_read_func(void *ptr, size_t size, size_t nmemb, FILE *stream)

{

   if(exit_loop)
       return -1;

  int len;
  len = fread(ptr, size, nmemb, stream);
  //Cdbg(API_DBG, "read len is %d\n",len);
  return len;

}

int my_progress_func(char *progress_data,

                     double t, /* dltotal */

                     double d, /* dlnow */

                     double ultotal,

                     double ulnow)

{

  //Cdbg(API_DBG, "%s %g / %g (%g %%)\n", progress_data, d, t, d*100.0/t);

#if 0
    int sec;
    double  elapsed = 0;
    elapsed = time(NULL) - start_time;
    sec = (int)elapsed;
    if( sec > 0 )
    {
    //double progress = ulnow*100.0/ultoal;
        if(sec % 5 == 0)
            Cdbg(API_DBG, "%s %g / %g (%g %%)\n", progress_data, ulnow, ultotal, ulnow*100.0/ultotal);
    }
#endif

#if 1
    if(exit_loop)
        return -1;



    if(t > 1 && d > 10 && ultotal > 0) // download
        printf("%s %10.0f / %10.0f (%g %%)\n", progress_data, d, t, d*100.0/t);
    else
        printf("%s %10.0f / %10.0f (%g %%)\n", progress_data, ulnow, ultotal, ulnow*100.0/ultotal);



#endif

  return 0;

}

#if 0
struct myprogress {
  double lastruntime;
  CURL *curl;
};

static int progress(void *p,
                    double dltotal, double dlnow,
                    double ultotal, double ulnow)
{
  struct myprogress *myp = (struct myprogress *)p;
  CURL *curl = myp->curl;
  double curtime = 0;

  curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &curtime);

  /* under certain circumstances it may be desirable for certain functionality
     to only run every N seconds, in order to do this the transaction time can
     be used */
  if((curtime - myp->lastruntime) >= MINIMAL_PROGRESS_FUNCTIONALITY_INTERVAL) {
    myp->lastruntime = curtime;
    fprintf(stderr, "TOTAL TIME: %f \r\n", curtime);
  }

  //fprintf(stderr, "UP: %g of %g  DOWN: %g of %g\r\n",
          //ulnow, ultotal, dlnow, dltotal);

  if(dltotal < dlnow){
          fprintf(stderr, "read : %f bytes\n", dlnow);
  }else{
          fprintf(stderr, "%% %.2f processed\n", (dlnow/dltotal)*100);
  }

  if(dlnow > STOP_DOWNLOAD_AFTER_THIS_MANY_BYTES)
    return 1;
  return 0;
}
#endif

/*
int if_file_exist(char *filename)
{
    FILE *fp;
    fp = fopen(filename,"r");
    if( NULL == fp )
        return 0;
    fclose(fp);
    return 1;

}*/

size_t write_data(void *ptr,size_t size,size_t nmemb,void *stream)
{
    if( exit_loop == 1)
        return -1;
    int len = fwrite(ptr,size,nmemb,(FILE *)stream);
    return len;
}

size_t read_data(void *ptr,size_t size,size_t nmemb,void *stream)
{
    return fread(ptr,size,nmemb,stream);
}

void parseGateway(xmlDocPtr doc, xmlNodePtr cur,Servicegateway *sg)
{
    xmlChar *key;
    cur = cur->xmlChildrenNode;
    while (cur != NULL)
    {
        key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
        if (!(xmlStrcmp(cur->name, (const xmlChar *)"status")))
        {
            sg->status = atoi((const char *)key);
            if(sg->status != 0)
            {
                xmlFree(key);
                return;
            }
        }

        if( !(xmlStrcmp(cur->name, (const xmlChar *)"servicegateway")))
        {
            strcpy(sg->gateway,(const char *)key);
        }
//            else if( !(xmlStrcmp(cur->name, (const xmlChar *)"liveupdateuri")))
//            {
//                strcpy(sg->liveupdateuri,key);
//            }
//            else if( !(xmlStrcmp(cur->name, (const xmlChar *)"time")))
//            {
//                strcpy(sg->time,key);
//            }

        xmlFree(key);
        cur = cur->next;
    }
}

void parseInitattachabledevice(xmlDocPtr doc, xmlNodePtr cur,Initattachabledevice *iad)
{
    xmlChar *key;
    cur = cur->xmlChildrenNode;
    while (cur != NULL)
    {
        key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
        if (!(xmlStrcmp(cur->name, (const xmlChar *)"status")))
        {
            iad->status = atoi((const char *)key);
            if(iad->status != 0)
            {
                xmlFree(key);
                return;
            }
        }

        xmlFree(key);
        cur = cur->next;
    }
}


void parseBrowseAttachableDevice(xmlDocPtr doc, xmlNodePtr cur, BrowseAttachableDevice *bad)
{


    xmlChar *key;
    cur = cur->xmlChildrenNode;
    while (cur != NULL)
    {
        key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);

        Cdbg(API_DBG, "cur->name = %s", cur->name);

        if (!(xmlStrcmp(cur->name, (const xmlChar *)"status")))
        {
            bad->status = atoi((const char *)key);
            if(bad->status != 0)
            {
                xmlFree(key);
                return;
            }
        }
        else if( !(xmlStrcmp(cur->name, (const xmlChar *)"attachabledevice")))
        {

            int get_device_id = parseAttachableDevice(doc, cur, bad);
            // get aicam device infomation
            if(get_device_id) {

                xmlFree(key);
                break;
            }
        }


        xmlFree(key);
        cur = cur->next;
    }
}



void parseBrowseAttachableDeviceList(xmlDocPtr doc, xmlNodePtr cur, BrowseAttachableDevice *bad)
{


    xmlChar *key;
    cur = cur->xmlChildrenNode;
    while (cur != NULL)
    {
        key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);

        Cdbg(API_DBG, "cur->name = %s", cur->name);

        if (!(xmlStrcmp(cur->name, (const xmlChar *)"status")))
        {
            bad->status = atoi((const char *)key);
            if(bad->status != 0)
            {
                xmlFree(key);
                return;
            }
        }

        xmlFree(key);
        cur = cur->next;
    }
}




int parseBrowseAttachableDeviceListCheck(xmlDocPtr doc, xmlNodePtr cur, char *device_id)
{

    int device_id_check = -1;

    xmlChar *key;
    cur = cur->xmlChildrenNode;
    while (cur != NULL)
    {
        key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);

        Cdbg(API_DBG, "cur->name = %s", cur->name);

        if (!(xmlStrcmp(cur->name, (const xmlChar *)"status")))
        {
            if(atoi((const char *)key) != 0)
            {
                xmlFree(key);
                return;
            }
        }
        else if( !(xmlStrcmp(cur->name, (const xmlChar *)"attachabledevice")))
        {

            device_id_check = parseAttachableDeviceListCheck(doc, cur, device_id);
            // get aicam device infomation
            if(device_id_check == 0) {

                xmlFree(key);
                break;
            }
        }


        xmlFree(key);
        cur = cur->next;
    }

    return device_id_check;
}


int parseAttachableDevice(xmlDocPtr doc, xmlNodePtr cur, BrowseAttachableDevice *bad)
{

    int deviceid_check = 0;

    xmlChar *key;
    cur = cur->xmlChildrenNode;

    while (cur != NULL)
    {
        key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);


        if(key == NULL && cur->children == NULL) //if value is null,not parse
        {
            cur = cur->next;
            continue;
        }

        if (!(xmlStrcmp(cur->name, (const xmlChar *)"deviceid")))
        {
            
            strcpy(bad->deviceid,(const char *)key);

            if( (strcmp(bad->deviceid, asus_conf.device_id) != 0)) 
            {
                Cdbg(API_DBG, "BroseAttachableDevice XML device id = %s, aicam device id = %s", bad->deviceid, asus_conf.device_id);
                xmlFree(key);
                deviceid_check = 0;
                return deviceid_check;
            } else {
                Cdbg(API_DBG, "BroseAttachableDevice XML device id = aicam device id = %s", asus_conf.device_id);
                deviceid_check = 1;
            }

            Cdbg(API_DBG, "bad->deviceid = %s", bad->deviceid);
        }
 

        if( !(xmlStrcmp(cur->name, (const xmlChar *)"owneruserid")))
        {
            Cdbg(API_DBG, "strlen(key = %d", strlen(key));
            Cdbg(API_DBG, "bad->owneruserid = %d", sizeof(bad->owneruserid));

            strcpy(bad->owneruserid,(const char *)key);
            Cdbg(API_DBG, "bad->owneruserid = %s", bad->owneruserid);
        }
        else if( !(xmlStrcmp(cur->name, (const xmlChar *)"attachabledevicename")))
        {
            strcpy(bad->attachabledevicename,(const char *)key);
            Cdbg(API_DBG, "bad->attachabledevicename = %s", bad->attachabledevicename);
        }
        else if( !(xmlStrcmp(cur->name, (const xmlChar *)"devicemanagerhost")))
        {
            strcpy(bad->devicemanagerhost,(const char *)key);
            Cdbg(API_DBG, "bad->devicemanagerhost = %s", bad->devicemanagerhost);
        }
        else if( !(xmlStrcmp(cur->name, (const xmlChar *)"state")))
        {
            bad->state = atoi((const char *)key);
            Cdbg(API_DBG, "bad->state = %d", bad->state);
        }
        else if( !(xmlStrcmp(cur->name, (const xmlChar *)"expiredtime")))
        {
            strcpy(bad->expiredtime,(const char *)key);
            Cdbg(API_DBG, "bad->expiredtime = %s", bad->expiredtime);
        }
        else if( !(xmlStrcmp(cur->name, (const xmlChar *)"filelifetime")))
        {
            bad->filelifetime = atoi((const char *)key);
            Cdbg(API_DBG, "bad->filelifetime = %d", bad->filelifetime);
        }
        else if( !(xmlStrcmp(cur->name, (const xmlChar *)"version")))
        {
            bad->version = atoi((const char *)key);
            Cdbg(API_DBG, "bad->version = %d", bad->version);
        }
        else if( !(xmlStrcmp(cur->name, (const xmlChar *)"scheme")))
        {
            bad->scheme = atoi((const char *)key);
            Cdbg(API_DBG, "bad->scheme = %d", bad->scheme);
        }

        xmlFree(key);
        cur = cur->next;
    }

    return deviceid_check;

}




int parseAttachableDeviceListCheck(xmlDocPtr doc, xmlNodePtr cur, char *device_id)
{

    int deviceid_check = -1;
    int scheme_check = -1;


    xmlChar *key;
    cur = cur->xmlChildrenNode;

    while (cur != NULL)
    {
        key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);


        if(key == NULL && cur->children == NULL) //if value is null,not parse
        {
            cur = cur->next;
            continue;
        }

        if (!(xmlStrcmp(cur->name, (const xmlChar *)"deviceid")))
        {

            if( (strcmp((const char *)key, device_id) != 0)) 
            {
                Cdbg(API_DBG, "data diff -> BroseAttachableDevice XML device id = %s, aicam device id = %s", (const char *)key, device_id);
                xmlFree(key);
                deviceid_check = 0;
                return -1;
            } else {
                Cdbg(API_DBG, "data same -> BroseAttachableDevice XML device id = aicam device id = %s", device_id);
                deviceid_check = 1;
            }
       }
 

        if( !(xmlStrcmp(cur->name, (const xmlChar *)"owneruserid")))
        {
            Cdbg(API_DBG, "strlen(key = %d", strlen(key));
            Cdbg(API_DBG, "owneruserid = %s", (const char *)key);
        }
        else if( !(xmlStrcmp(cur->name, (const xmlChar *)"attachabledevicename")))
        {
            Cdbg(API_DBG, "attachabledevicename = %s", (const char *)key);
        }
        else if( !(xmlStrcmp(cur->name, (const xmlChar *)"devicemanagerhost")))
        {
            Cdbg(API_DBG, "devicemanagerhost = %s", (const char *)key);
        }
        else if( !(xmlStrcmp(cur->name, (const xmlChar *)"state")))
        {
            Cdbg(API_DBG, "state = %d", atoi((const char *)key));
        }
        else if( !(xmlStrcmp(cur->name, (const xmlChar *)"expiredtime")))
        {
            Cdbg(API_DBG, "expiredtime = %s", (const char *)key);
        }
        else if( !(xmlStrcmp(cur->name, (const xmlChar *)"filelifetime")))
        {
            Cdbg(API_DBG, "filelifetime = %d", atoi((const char *)key));
        }
        else if( !(xmlStrcmp(cur->name, (const xmlChar *)"version")))
        {
            Cdbg(API_DBG, "version = %d", atoi((const char *)key));
        } else if( !(xmlStrcmp(cur->name, (const xmlChar *)"scheme")))
        {
            Cdbg(API_DBG, "scheme = %d", atoi((const char *)key));
            scheme_check = 1;
        } 

        xmlFree(key);
        cur = cur->next;
    }

    if((scheme_check == 1) && (deviceid_check == 1)) {
        return 0;
    } else {
        return -1;
    }

}

int parseGetmysyncfolder(xmlDocPtr doc, xmlNodePtr cur,Getmysyncfolder *gf)
{
    xmlChar *key;
    cur = cur->xmlChildrenNode;
    while (cur != NULL)
    {
        key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);

        if(key == NULL && cur->children == NULL) //if value is null,not parse
        {
            cur = cur->next;
            continue;
        }

        if (!(xmlStrcmp(cur->name, (const xmlChar *)"status")))
        {
            gf->status = atoi((const char *)key);
            if(gf->status != 0)
            {
                xmlFree(key);
                return -1;
            }
        }
        else if (!(xmlStrcmp(cur->name, (const xmlChar *)"id")))
        {
          gf->id = atoi((const char *)key);
        }


        xmlFree(key);
        cur = cur->next;
    }
    return 0;
}

int
        parseGetPersonalSystemFolder(xmlDocPtr doc, xmlNodePtr cur,Getpersonalsystemfolder *gp)
{
    xmlChar *key;
    cur = cur->xmlChildrenNode;
    while (cur != NULL)
    {
        key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);

        if(key == NULL && cur->children == NULL) //if value is null,not parse
        {
            cur = cur->next;
            continue;
        }

        if (!(xmlStrcmp(cur->name, (const xmlChar *)"status")))
        {
            gp->status = atoi((const char *)key);
        }
        else if (!(xmlStrcmp(cur->name, (const xmlChar *)"script")))
        {
            strcpy(gp->script,(const char *)key);
        }
        else if (!(xmlStrcmp(cur->name, (const xmlChar *)"folderid")))
        {
             gp->folderid = atoi((const char *)key);
        }

        xmlFree(key);
        cur = cur->next;
    }
    return 0;
}

int parseGetinfo (xmlDocPtr doc, xmlNodePtr cur,Getinfo *gi)
{
    xmlChar *key;
    cur = cur->xmlChildrenNode;
    while (cur != NULL)
    {
        key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);

        if(key == NULL && cur->children == NULL) //if value is null,not parse
        {
            cur = cur->next;
            continue;
        }

        if (!(xmlStrcmp(cur->name, (const xmlChar *)"status")))
        {
            gi->status = atoi((const char *)key);
            if(gi->status != 0)
            {
                xmlFree(key);
                return -1;
            }
        }
        if( !(xmlStrcmp(cur->name, (const xmlChar *)"account")))
        {
            gi->account = atoi((const char *)key);
        }
        else if( !(xmlStrcmp(cur->name, (const xmlChar *)"email")))
        {
            strcpy(gi->email,(const char *)key);
        }
        else if( !(xmlStrcmp(cur->name, (const xmlChar *)"regyear")))
        {
            strcpy(gi->regyear,(const char *)key);
        }
        else if( !(xmlStrcmp(cur->name, (const xmlChar *)"language")))
        {
            strcpy(gi->language,(const char *)key);
        }
        else if( !(xmlStrcmp(cur->name, (const xmlChar *)"activateddate")))
        {
            strcpy(gi->activateddate,(const char *)key);
        }
        else if( !(xmlStrcmp(cur->name, (const xmlChar *)"credentialstate")))
        {
            gi->credentialstate = atoi((const char *)key);
        }
        else if( !(xmlStrcmp(cur->name, (const xmlChar *)"usedbackuppc")))
        {
            gi->usedbackuppc = atoi((const char *)key);
        }
        else if( !(xmlStrcmp(cur->name, (const xmlChar *)"backuppc")))
        {
            parseGetinfo(doc,cur,gi);
        }
        else if( !(xmlStrcmp(cur->name, (const xmlChar *)"name")))
        {
            strcpy(gi->backuppc.name,(const char *)key);
        }
        else if( !(xmlStrcmp(cur->name, (const xmlChar *)"createdtime")))
        {
            strcpy(gi->backuppc.createdtime,(const char *)key);
        }
        else if( !(xmlStrcmp(cur->name, (const xmlChar *)"package")))
        {
            parseGetinfo(doc,cur,gi);
        }
        else if( !(xmlStrcmp(cur->name, (const xmlChar *)"id")))
        {
            gi->package.id = atoi((const char *)key);
        }
        else if( !(xmlStrcmp(cur->name, (const xmlChar *)"display")))
        {
            strcpy(gi->package.display,(const char *)key);
        }
        else if( !(xmlStrcmp(cur->name, (const xmlChar *)"capacity")))
        {
            gi->package.capacity = atoi((const char *)key);
        }
        else if( !(xmlStrcmp(cur->name, (const xmlChar *)"uploadbandwidth")))
        {
            gi->package.uploadbandwidth = atoi((const char *)key);
        }
        else if( !(xmlStrcmp(cur->name, (const xmlChar *)"downloadbandwidth")))
        {
            gi->package.downloadbandwidth = atoi((const char *)key);
        }
        else if( !(xmlStrcmp(cur->name, (const xmlChar *)"upload")))
        {
            gi->package.upload = atoi((const char *)key);
        }
        else if( !(xmlStrcmp(cur->name, (const xmlChar *)"download")))
        {
            gi->package.download = atoi((const char *)key);
        }
        else if( !(xmlStrcmp(cur->name, (const xmlChar *)"concurrentsession")))
        {
            gi->package.concurrentsession = atoi((const char *)key);
        }
        else if( !(xmlStrcmp(cur->name, (const xmlChar *)"maxfilesize")))
        {
            gi->package.maxfilesize = atoi((const char *)key);
        }
        else if( !(xmlStrcmp(cur->name, (const xmlChar *)"sharegroup")))
        {
            gi->package.sharegroup = atoi((const char *)key);
        }
        else if( !(xmlStrcmp(cur->name, (const xmlChar *)"hasencrption")))
        {
            gi->package.hasencrption = atoi((const char *)key);
        }
        else if( !(xmlStrcmp(cur->name, (const xmlChar *)"expire")))
        {
            strcpy(gi->package.expire,(const char *)key);
        }
        else if( !(xmlStrcmp(cur->name, (const xmlChar *)"maxbackuppc")))
        {
            gi->package.maxbackuppc = atoi((const char *)key);
        }
        else if( !(xmlStrcmp(cur->name, (const xmlChar *)"usedcapacity")))
        {
            gi->usedcappacity = atoi((const char *)key);
        }
        else if( !(xmlStrcmp(cur->name, (const xmlChar *)"freecapacity")))
        {
            gi->freecapacity = atoi((const char *)key);
        }

        xmlFree(key);
        cur = cur->next;
    }
    return 0;
}

static int
        parseAaa (xmlDocPtr doc, xmlNodePtr cur,Aaa *aaa)
{
    xmlChar *key;
    cur = cur->xmlChildrenNode;
    while (cur != NULL)
    {
        key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);

        /* if value is null and node is no children node,not parse */
        if(key == NULL && cur->children == NULL)
        {
            cur = cur->next;
            continue;
        }

        if (!(xmlStrcmp(cur->name, (const xmlChar *)"status")))
        {
            aaa->status = atoi((const char *)key);
        }

        if( !(xmlStrcmp(cur->name, (const xmlChar *)"token")))
        {
            strcpy(aaa->token,(const char *)key);
        }
        else if( !(xmlStrcmp(cur->name, (const xmlChar *)"contentrelay")))
        {
            strcpy(aaa->contentrelay,(const char *)key);
        }
        else if( !(xmlStrcmp(cur->name, (const xmlChar *)"filerelay")))
        {
            strcpy(aaa->filerelay,(const char *)key);
        }
        else if( !(xmlStrcmp(cur->name, (const xmlChar *)"inforelay")))
        {
            strcpy(aaa->inforelay,(const char *)key);
        }
        else if( !(xmlStrcmp(cur->name, (const xmlChar *)"jobrelay")))
        {
            strcpy(aaa->jobrelay,(const char *)key);
        }
        else if( !(xmlStrcmp(cur->name, (const xmlChar *)"rssrelay")))
        {
            strcpy(aaa->rssrelay,(const char *)key);
        }
        else if( !(xmlStrcmp(cur->name, (const xmlChar *)"searchrelay")))
        {
            strcpy(aaa->searchrelay,(const char *)key);
        }
        else if( !(xmlStrcmp(cur->name, (const xmlChar *)"webrelay")))
        {
            strcpy(aaa->webrelay,(const char *)key);
        }
        else if( !(xmlStrcmp(cur->name, (const xmlChar *)"package")))
        {
           parseAaa(doc,cur,aaa);
        }
        else if( !(xmlStrcmp(cur->name, (const xmlChar *)"auxpasswordurl")))
        {
            char *p = oauth_url_unescape((const char *)key,NULL);
            strcpy(aaa->auxpasswordurl,p);
            free(p);
        }

        xmlFree(key);
        cur = cur->next;
    }
    return 0;
}

int parseInitbinaryupload (xmlDocPtr doc, xmlNodePtr cur,Initbinaryupload *ibu)
{
    xmlChar *key;
    cur = cur->xmlChildrenNode;
    while (cur != NULL)
    {
        key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);

        if(key == NULL && cur->children == NULL) //if value is null,not parse
        {
            cur = cur->next;
            continue;
        }

        if (!(xmlStrcmp(cur->name, (const xmlChar *)"status")))
        {
            ibu->status = atoi((const char *)key);
        }

        if(ibu->status == 0)
        {
            if( !(xmlStrcmp(cur->name, (const xmlChar *)"transid")))
            {
                strcpy(ibu->transid,(const char *)key);
            }
            else if( !(xmlStrcmp(cur->name, (const xmlChar *)"offset")))
            {
                ibu->offset = atoi((const char *)key);
            }
            else if( !(xmlStrcmp(cur->name, (const xmlChar *)"fileid")))
            {
                // Cdbg(API_DBG, "ibu->fileid s: %s",(const char *)key);
                ibu->fileid = atol((const char *)key);
                // Cdbg(API_DBG, "ibu->fileid : %lli",ibu
            }
            else if( !(xmlStrcmp(cur->name, (const xmlChar *)"latestchecksum")))
            {
                strcpy(ibu->latestchecksum,(const char *)key);
            }
        }
        else
        {
            if( !(xmlStrcmp(cur->name, (const xmlChar *)"logmessage")))
            {
                strcpy(ibu->logmessage,(const char *)key);
            }
        }

        xmlFree(key);
        cur = cur->next;
    }
    return 0;
}

int
        parseResumebinaryupload (xmlDocPtr doc, xmlNodePtr cur,Resumebinaryupload *rbu)
{
    xmlChar *key;
    cur = cur->xmlChildrenNode;
    while (cur != NULL)
    {
        key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);

        if(key == NULL && cur->children == NULL) //if value is null,not parse
        {
            cur = cur->next;
            continue;
        }

        if (!(xmlStrcmp(cur->name, (const xmlChar *)"status")))
        {
            rbu->status = atoi((const char *)key);
            if(rbu->status != 0)
            {
                xmlFree(key);
                return -1;
            }
        }

        xmlFree(key);
        cur = cur->next;
    }
    return 0;
}

int
        parseFinishbinaryupload (xmlDocPtr doc, xmlNodePtr cur,Finishbinaryupload *fbu)
{
    xmlChar *key;
    cur = cur->xmlChildrenNode;
    while (cur != NULL)
    {
        key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);

        if(key == NULL && cur->children == NULL) //if value is null,not parse
        {
            cur = cur->next;
            continue;
        }

        if (!(xmlStrcmp(cur->name, (const xmlChar *)"status")))
        {
            fbu->status = atoi((const char *)key);
            if(fbu->status != 0)
            {
                xmlFree(key);
                return -1;
            }
        }
        else if( !(xmlStrcmp(cur->name, (const xmlChar *)"fileid")))
        {
            fbu->fileid = atol((const char *)key);
        }

        xmlFree(key);
        cur = cur->next;
    }
    return 0;
}

int get_server_item_size(xmlDocPtr doc, xmlNodePtr cur,Browse *browse)
{
    xmlChar *key;
    cur = cur->xmlChildrenNode;
    int folder_size = 0;
    int file_size = 0;
    while (cur != NULL)
    {
        key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);

        if(key == NULL && cur->children == NULL) //if value is null,not parse
        {
            cur = cur->next;
            continue;
        }

        if (!(xmlStrcmp(cur->name, (const xmlChar *)"status")))
        {
            browse->status = atoi((const char *)key);
            if(browse->status != 0)
            {
                xmlFree(key);
                return -1;
            }
        }

        if( !(xmlStrcmp(cur->name, (const xmlChar *)"folder")))
        {
            folder_size += 1 ;
        }
        else if( !(xmlStrcmp(cur->name, (const xmlChar *)"file")))
        {
            file_size += 1 ;
        }

//        else if( !(xmlStrcmp(cur->name, (const xmlChar *)"page")))
//        {
//            parseBrowse(doc,cur,browse);
//        }
//        else if( !(xmlStrcmp(cur->name, (const xmlChar *)"pageno")))
//        {
//            browse->page.pageno = atoi(key);
//        }
//        else if( !(xmlStrcmp(cur->name, (const xmlChar *)"pagesize")))
//        {
//            browse->page.pagesize = atoi(key);
//        }
//        else if( !(xmlStrcmp(cur->name, (const xmlChar *)"totalcount")))
//        {
//            browse->page.totalcount= atoi(key);
//        }
//        else if( !(xmlStrcmp(cur->name, (const xmlChar *)"hasnextpage")))
//        {
//            browse->page.hasnextpage = atoi(key);
//        }

        xmlFree(key);
        cur = cur->next;
    }

    //Cdbg(API_DBG, "folder size is %d,file size is %d\n",folder_size,file_size);

    if(file_size > 0)
    {
//        if(browse->filenumber > 0)
//        {
//            File *f1,f2;
//            f1 = browse->filelist;
//            f2 = (File *)calloc(f1,sizeof(File *)*file_size);
//            if(f2 == NULL)
//            {
//                handle_error(S_MEMORY_FAIL,"get_server_item_size");
//                return -1;
//            }
//            if(f2 != f1)
//                browse->filelist = f2;
//
//        }
//        else
//        {
            browse->filelist = (File **)malloc(sizeof(File *)*file_size);

            if(browse->filelist == NULL)
            {
                handle_error(S_MEMORY_FAIL,"get_server_item_size");
                return -1;
            }
//        }
    }

    if(folder_size > 0)
    {
//        if(browse->foldernumber >0)
//        {
//           Folder *f1,f2;
//           f1 = browse->folderlist;
//           f2 = (Folder *)calloc(f1,sizeof(Folder *)*folder_size);
//           if(f2 == NULL)
//           {
//               handle_error(S_MEMORY_FAIL,"get_server_item_size");
//               f1 = browse->filelist + browse->filenumber -1;
//               my_free(f1);
//               return -1;
//           }
//           if(f2 != f1)
//               browse->folderlist = f2;
//        }
//        else
//        {
            browse->folderlist = (Folder **)malloc(sizeof(Folder *)*folder_size);

            if(browse->folderlist == NULL)
            {
                handle_error(S_MEMORY_FAIL,"get_server_item_size");
                my_free(browse->filelist);
                return -1;
            }
//        }

    }

    return 0;
}


static int parseBrowse (xmlDocPtr doc, xmlNodePtr cur,Browse *browse)
{
    xmlChar *key;
    cur = cur->xmlChildrenNode;
    int foldernum = browse->foldernumber-1 ;
    int filenum = browse->filenumber-1;
    //get_server_item_size(doc,cur,browse);
    //browse->filelist = (File **)malloc
    while (cur != NULL)
    {
        key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
        //Cdbg(API_DBG, "name is %s,key is %s,type is %d\n",cur->name,key,cur->type);

        if(key == NULL && cur->children == NULL) //if value is null,not parse
        {
            //xmlFree(key);
            cur = cur->next;
            continue;
        }

        if (!(xmlStrcmp(cur->name, (const xmlChar *)"status")))
        {
            browse->status = atoi((const char *)key);
            if(browse->status != 0)
            {
                xmlFree(key);
                free_server_list(browse);
                return -1;
            }
        }
//        if( !(xmlStrcmp(cur->name, (const xmlChar *)"scrip")))
//        {
//            strcpy(browse->scrip,key);
//        }
//        else if( !(xmlStrcmp(cur->name, (const xmlChar *)"parentfolder")))
//        {
//            //get_server_item_size(doc,cur,browse);
//            parseBrowse(doc,cur,browse);
//
//        }
//        else if( !(xmlStrcmp(cur->name, (const xmlChar *)"name")))
//        {
//                strcpy(browse->parentfolder.name,key);
//        }
        else if( !(xmlStrcmp(cur->name, (const xmlChar *)"id")))
        {
//            if( !(xmlStrcmp(cur->parent->name, (const xmlChar *)"parentfolder")))
//                browse->parentfolder.id = atoi(key);
            if( !(xmlStrcmp(cur->parent->name, (const xmlChar *)"folder")))
                (browse->folderlist)[foldernum]->id = atoi((const char *)key);
            else if( !(xmlStrcmp(cur->parent->name, (const xmlChar *)"file")))
                (browse->filelist)[filenum]->id = atoi((const char *)key);
        }
        else if( !(xmlStrcmp(cur->name, (const xmlChar *)"folder")))
        {
            browse->foldernumber += 1 ;
            int num = browse->foldernumber -1;

            (browse->folderlist)[num] = (Folder *)malloc(sizeof(Folder));
            if((browse->folderlist)[num] == NULL)
            {
                handle_error(S_MEMORY_FAIL,"parse browse");
                free_server_list(browse);
                xmlFree(key);
                return -1;
            }
            memset(browse->folderlist[num],0,sizeof(Folder));
            parseBrowse(doc,cur,browse);
        }
//        else if( !(xmlStrcmp(cur->name, (const xmlChar *)"treesize")))
//        {
//            (browse->folderlist)[foldernum]->treesize = atoll(key);
//        }
        else if( !(xmlStrcmp(cur->name, (const xmlChar *)"display")))
        {
                int len = strlen((const char *)key)+1;
                if( !(xmlStrcmp(cur->parent->name, (const xmlChar *)"folder")))
                {
                    browse->folderlist[foldernum]->display = calloc(len,sizeof(char));
                    strcpy((browse->folderlist)[foldernum]->display,(const char *)key);
                }
                else if( !(xmlStrcmp(cur->parent->name, (const xmlChar *)"file")))
                {
                    (browse->filelist)[filenum]->display = calloc(len,sizeof(char));
                    strcpy((browse->filelist)[filenum]->display,(const char *)key);
                }
        }
        else if( !(xmlStrcmp(cur->name, (const xmlChar *)"attribute")))
        {
            parseBrowse(doc,cur,browse);
        }
//        else if( !(xmlStrcmp(cur->name, (const xmlChar *)"creationtime")))
//        {
//                if( !(xmlStrcmp(cur->parent->parent->name, (const xmlChar *)"folder")))
//                {
//                    strcpy((browse->folderlist)[foldernum]->attribute.creationtime,key);
//                    //Cdbg(API_DBG, "creationtime type is folder\n");
//                }
//                else if( !(xmlStrcmp(cur->parent->parent->name, (const xmlChar *)"file")))
//                {
//                    strcpy((browse->filelist)[filenum]->attribute.creationtime,key);
//                    //Cdbg(API_DBG, "creationtime type is file\n");
//                }
//                else
//                    Cdbg(API_DBG, "creationtime not find folder or file\n");
//        }
        else if( !(xmlStrcmp(cur->name, (const xmlChar *)"lastaccesstime")))
        {
//                if( !(xmlStrcmp(cur->parent->parent->name, (const xmlChar *)"folder")))
//                    strcpy((browse->folderlist)[foldernum]->attribute.lastaccesstime,key);
                if( !(xmlStrcmp(cur->parent->parent->name, (const xmlChar *)"file")))
                    strcpy((browse->filelist)[filenum]->attribute.lastaccesstime,(const char *)key);
        }
        else if( !(xmlStrcmp(cur->name, (const xmlChar *)"lastwritetime")))
        {
//                if( !(xmlStrcmp(cur->parent->parent->name, (const xmlChar *)"folder")))
//                    strcpy((browse->folderlist)[foldernum]->attribute.lastwritetime,key);
                 if( !(xmlStrcmp(cur->parent->parent->name, (const xmlChar *)"file")))
                    strcpy((browse->filelist)[filenum]->attribute.lastwritetime,(const char *)key);
        }
//        else if( !(xmlStrcmp(cur->name, (const xmlChar *)"finfo")))
//        {
//                if( !(xmlStrcmp(cur->parent->parent->name, (const xmlChar *)"folder")))
//                    strcpy((browse->folderlist)[foldernum]->attribute.finfo,key);
//                else if( !(xmlStrcmp(cur->parent->parent->name, (const xmlChar *)"file")))
//                    strcpy((browse->filelist)[filenum]->attribute.finfo,key);
//        }
//        else if( !(xmlStrcmp(cur->name, (const xmlChar *)"x-timeforsynccheck")))
//        {
//            if( !(xmlStrcmp(cur->parent->parent->name, (const xmlChar *)"folder")))
//                strcpy((browse->folderlist)[foldernum]->attribute.xtimeforsynccheck,key);
//             else if( !(xmlStrcmp(cur->parent->parent->name, (const xmlChar *)"file")))
//                 strcpy((browse->filelist)[filenum]->attribute.xtimeforsynccheck,key);
//        }
//        else if( !(xmlStrcmp(cur->name, (const xmlChar *)"x-machinename")))
//        {
//             if( !(xmlStrcmp(cur->parent->parent->name, (const xmlChar *)"folder")))
//                strcpy((browse->folderlist)[foldernum]->attribute.xmachinename,key);
//             else if( !(xmlStrcmp(cur->parent->parent->name, (const xmlChar *)"file")))
//                 strcpy((browse->filelist)[filenum]->attribute.xmachinename,key);
//        }
//        else if( !(xmlStrcmp(cur->name, (const xmlChar *)"isencrypted")))
//        {
//            if( !(xmlStrcmp(cur->parent->name, (const xmlChar *)"folder")))
//                (browse->folderlist)[foldernum]->isencrypted = atoi(key);
//            else if( !(xmlStrcmp(cur->parent->name, (const xmlChar *)"file")))
//                (browse->filelist)[filenum]->isencrypted = atoi(key);
//        }
//        else if( !(xmlStrcmp(cur->name, (const xmlChar *)"issharing")))
//        {
//            (browse->folderlist)[foldernum]->issharing = atoi(key);
//        }
//        else if( !(xmlStrcmp(cur->name, (const xmlChar *)"isowner")))
//        {
//            if( !(xmlStrcmp(cur->parent->name, (const xmlChar *)"folder")))
//                (browse->folderlist)[foldernum]->isowner = atoi(key);
//            else if( !(xmlStrcmp(cur->parent->name, (const xmlChar *)"file")))
//                (browse->filelist)[filenum]->isowner = atoi(key);
//        }
//        else if( !(xmlStrcmp(cur->name, (const xmlChar *)"isbackup")))
//        {
//            if( !(xmlStrcmp(cur->parent->name, (const xmlChar *)"folder")))
//                (browse->folderlist)[foldernum]->isbackup = atoi(key);
//            else if( !(xmlStrcmp(cur->parent->name, (const xmlChar *)"file")))
//                (browse->filelist)[filenum]->isbackup = atoi(key);
//        }
//        else if( !(xmlStrcmp(cur->name, (const xmlChar *)"isorigdeleted")))
//        {
//            if( !(xmlStrcmp(cur->parent->name, (const xmlChar *)"folder")))
//                (browse->folderlist)[foldernum]->isorigdeleted = atoi(key);
//            else if( !(xmlStrcmp(cur->parent->name, (const xmlChar *)"file")))
//                (browse->filelist)[filenum]->isorigdeleted = atoi(key);
//        }
//        else if( !(xmlStrcmp(cur->name, (const xmlChar *)"ispublic")))
//        {
//            if( !(xmlStrcmp(cur->parent->name, (const xmlChar *)"folder")))
//                (browse->folderlist)[foldernum]->ispublic = atoi(key);
//            else if( !(xmlStrcmp(cur->parent->name, (const xmlChar *)"file")))
//                (browse->filelist)[filenum]->ispublic = atoi(key);
//        }
//        else if( !(xmlStrcmp(cur->name, (const xmlChar *)"createdtime")))
//        {
//                if( !(xmlStrcmp(cur->parent->name, (const xmlChar *)"folder")))
//                    strcpy((browse->folderlist)[foldernum]->createdtime,key);
//                else if( !(xmlStrcmp(cur->parent->name, (const xmlChar *)"file")))
//                    strcpy((browse->filelist)[filenum]->createdtime,key);
//        }
//        else if( !(xmlStrcmp(cur->name, (const xmlChar *)"markid")))
//        {
//                if( !(xmlStrcmp(cur->parent->name, (const xmlChar *)"folder")))
//                    (browse->folderlist)[foldernum]->markid = atoi(key);
//                else if( !(xmlStrcmp(cur->parent->name, (const xmlChar *)"file")))
//                    (browse->filelist)[filenum]->markid = atoi(key);
//        }
//
//        else if( !(xmlStrcmp(cur->name, (const xmlChar *)"metadata")))
//        {
//
//                if( !(xmlStrcmp(cur->parent->name, (const xmlChar *)"folder")))
//                    strcpy((browse->folderlist)[foldernum]->metadata,key);
//                else if( !(xmlStrcmp(cur->parent->name, (const xmlChar *)"file")))
//                    strcpy((browse->filelist)[filenum]->metadata,key);
//        }
        else if( !(xmlStrcmp(cur->name, (const xmlChar *)"file")))
        {
            browse->filenumber += 1 ;
            int num = browse->filenumber -1;
            (browse->filelist)[num] = (File *)malloc(sizeof(File));
            if((browse->filelist)[num] == NULL)
            {
                handle_error(S_MEMORY_FAIL,"parse browse");
                free_server_list(browse);
                xmlFree(key);
                return -1;
            }
            memset(browse->filelist[num],0,sizeof(File));
            parseBrowse(doc,cur,browse);
        }
        else if( !(xmlStrcmp(cur->name, (const xmlChar *)"size")))
        {
                (browse->filelist)[filenum]->size = atoll((const char *)key);
        }
//        else if( !(xmlStrcmp(cur->name, (const xmlChar *)"headversion")))
//        {
//                (browse->filelist)[filenum]->headversion= atoi(key);
//        }
        else if( !(xmlStrcmp(cur->name, (const xmlChar *)"page")))
        {
            parseBrowse(doc,cur,browse);
        }
        if( !(xmlStrcmp(cur->name, (const xmlChar *)"pageno")))
        {
            browse->page.pageno = atoi((const char *)key);
        }
        else if( !(xmlStrcmp(cur->name, (const xmlChar *)"pagesize")))
        {
            browse->page.pagesize = atoi((const char *)key);
        }
        else if( !(xmlStrcmp(cur->name, (const xmlChar *)"totalcount")))
        {
            browse->page.totalcount= atoi((const char *)key);
        }
        else if( !(xmlStrcmp(cur->name, (const xmlChar *)"hasnextpage")))
        {
            browse->page.hasnextpage = atoi((const char *)key);
        }
        xmlFree(key);
        cur = cur->next;
    }
    return 0;
}

int find__root_node(const char *buf,const char *key)
{
    int res = 1;
    char *p = NULL;

    p = strstr(buf,key) ;
    if (p == NULL)
    {
        Cdbg(API_DBG, "Have no XML content tail!\n");
        res = 0;
    }

    return res;
}

char *get_xml_value(const char *buf,const char *key)
{
    char *start=NULL,*end=NULL,*value = NULL;
    char stag[64]={0},etag[64]={0};
    int len = 0;

    sprintf(stag,"<%s>",key);
    sprintf(etag,"</%s>",key);

    start = strstr(buf,stag);
    end = strstr(buf,etag);

    if(end && start)
    {
        len = strlen(start) - strlen(end)- strlen(stag);
        value = (char *)calloc(len+1,sizeof(char));
        strncpy(value,start+strlen(stag),len);
        // Cdbg(API_DBG, "value=%s\n",value);
    }

    return value;
}

int get_item_num(const char *buf,int isfile)
{
    char *nodeStart = NULL;
    char *nodeEnd = NULL;
    int i=0 ;
    const char *p=NULL;
    int len = 0;
    char stag[16]={0},etag[16]={0};

    p = buf;
    if(isfile)
    {
        strcpy(stag,"<file>");
        strcpy(etag,"</file>");
    }
    else
    {
        strcpy(stag,"<folder>");
        strcpy(etag,"</folder>");
    }

    nodeStart = strstr(p,stag);
    nodeEnd   = strstr(p,etag);

    while(nodeStart && nodeEnd)
    {
        i++;
        len = strlen(etag);
        p = nodeEnd + len;

        nodeStart = strstr(p,stag);
        nodeEnd   = strstr(p,etag);
    }

    return i;
}

int get_item_list(const char *buf,Browse *br,int isfile)
{
    char *nodeStart = NULL;
    char *nodeEnd = NULL;
    char stag[16]={0},etag[16]={0};
    int index = 0,tag_len=0;
    char *value = NULL;
    int len = 0;
    char *node = NULL;
    char *temp = NULL;
    char *p = NULL;

    if(isfile)
    {
        strcpy(stag,"<file>");
        strcpy(etag,"</file>");
    }
    else
    {
        strcpy(stag,"<folder>");
        strcpy(etag,"</folder>");
    }

    nodeStart = strstr(buf,stag);
    nodeEnd   = strstr(buf,etag);

    while(nodeStart && nodeEnd)
    {
        tag_len = strlen(stag);
        len = strlen(nodeStart) - strlen(nodeEnd)-tag_len;
        node = (char *)calloc(len+1,sizeof(char));
        strncpy(node,nodeStart+tag_len,len);
        //Cdbg(API_DBG, "node=%s\n",node);

        if(isfile)
            br->filelist[index] = (File *)calloc(1,sizeof(File));
        else
            br->folderlist[index] = (Folder *)calloc(1,sizeof(Folder));

        value = get_xml_value(node,"id");
        if(value)
        {
            if(isfile)
                br->filelist[index]->id = atoi(value);
            else
                br->folderlist[index]->id = atoi(value);
            my_free(value);
        }

        value = get_xml_value(node,"display");
        if(value)
        {
            temp = (char *)calloc(strlen(value)+1,sizeof(char));
            strcpy(temp,value);
            if(isfile)
                br->filelist[index]->display = temp;
            else
                br->folderlist[index]->display = temp;
            my_free(value);
        }

        // value = get_xml_value(node,"createdtime");
        // if(value)
        // {
        //     temp = (char *)calloc(strlen(value)+1,sizeof(char));
        //     strcpy(temp,value);
        //     if(isfile)
        //         br->filelist[index]->createdtime = temp;
        //     else
        //         br->folderlist[index]->createdtime = temp;
        //     my_free(value);
        // }

        value = get_xml_value(node,"createdtime");
        if(value)
        {
            if(isfile)
                strcpy(br->filelist[index]->createdtime,value);
            else
                strcpy(br->folderlist[index]->createdtime,value);

            my_free(value);
        }
        

        if(isfile)
        {
            value = get_xml_value(node,"size");
            if(value)
            {
                br->filelist[index]->size = atoll(value);
                my_free(value);
            }

            value = get_xml_value(node,"lastaccesstime");
            if(value)
            {
                strcpy(br->filelist[index]->attribute.lastaccesstime,value);
                my_free(value);
            }

            value = get_xml_value(node,"lastwritetime");
            if(value)
            {
                strcpy(br->filelist[index]->attribute.lastwritetime,value);
                my_free(value);
            }

        }

        my_free(node);
        p = nodeEnd + strlen(etag);
        nodeStart = strstr(p,stag);
        nodeEnd = strstr(p,etag);
        index++;
        //Cdbg(API_DBG, "index=%d\n",index);
    }

    return 0;
}



int get_file_list(const char *buf, SlidingBrowse *sbr)
{
    char *nodeStart = NULL;
    char *nodeEnd = NULL;
    char stag[16]={0},etag[16]={0};
    int index = 0,tag_len=0;
    char *value = NULL;
    int len = 0;
    char *node = NULL;
    char *temp = NULL;
    char *p = NULL;

    strcpy(stag,"<file>");
    strcpy(etag,"</file>");

    nodeStart = strstr(buf,stag);
    nodeEnd   = strstr(buf,etag);

    while(nodeStart && nodeEnd)
    {
        tag_len = strlen(stag);
        len = strlen(nodeStart) - strlen(nodeEnd)-tag_len;
        node = (char *)calloc(len+1,sizeof(char));
        strncpy(node,nodeStart+tag_len,len);
        //Cdbg(API_DBG, "node=%s\n",node);

        sbr->filelist[index] = (Files *)calloc(1,sizeof(Files));

        value = get_xml_value(node,"id");
        if(value)
        {
            // sbr->filelist[index]->id = atoi(value);

            temp = (char *)calloc(strlen(value)+1,sizeof(char));
            strcpy(temp,value);
            sbr->filelist[index]->id = temp;

            my_free(value);
        }

        // value = get_xml_value(node,"display");
        // if(value)
        // {
        //     temp = (char *)calloc(strlen(value)+1,sizeof(char));
        //     strcpy(temp,value);
        //     if(isfile)
        //         sbr->filelist[index]->display = temp;
        //     else
        //         sbr->folderlist[index]->display = temp;
        //     my_free(value);
        // }


        my_free(node);
        p = nodeEnd + strlen(etag);
        nodeStart = strstr(p,stag);
        nodeEnd = strstr(p,etag);
        index++;
        //Cdbg(API_DBG, "index=%d\n",index);
    }

    return 0;
}

int my_parse_browse_xml(const char *filename,Browse *br)
{
    char *value = NULL;
    int foldernum=0,filenum=0;
    char *buf = NULL;
    int len=0;

    LoadFileIntoBuffer(filename,&buf,&len);
    if(NULL == buf)
        return -1;

    if( !find__root_node(buf,"<browse>") || !find__root_node(buf,"</browse>"))
    {
        Cdbg(API_DBG, "find <browse> node fail\n");
        my_free(buf);
        return -1;
    }

    value = get_xml_value(buf,"status");
    if(value)
    {
        br->status = atoi(value);
        my_free(value);
        if(br->status != 0)
        {
            my_free(buf);
            return -2;
        }
    }

    foldernum = get_item_num(buf,0);
    if(foldernum > 0)
    {
        br->folderlist = (Folder **)calloc(foldernum,sizeof(Folder *));
        br->foldernumber = foldernum;
        get_item_list(buf,br,0);
    }

    filenum = get_item_num(buf,1);
    if(filenum > 0)
    {
        br->filelist = (File **)calloc(filenum,sizeof(File *));
        br->filenumber = filenum;
        get_item_list(buf,br,1);
    }

    my_free(buf);
    return 0;
}


int parse_sliding_browse_xml(const char *filename, SlidingBrowse *sbr)
{
    char *value = NULL;
    int filenum=0;
    char *buf = NULL;
    int len=0;

    LoadFileIntoBuffer(filename,&buf,&len);
    if(NULL == buf)
        return -1;

    if( !find__root_node(buf,"<slidingbrowse>") || !find__root_node(buf,"</slidingbrowse>"))
    {
        Cdbg(API_DBG, "find <slidingbrowse> node fail");
        my_free(buf);
        return -1;
    }

    value = get_xml_value(buf,"status");
    if(value)
    {
        sbr->status = atoi(value);
        my_free(value);
        if(sbr->status != 0)
        {
            my_free(buf);
            return -2;
        }
    }

    filenum = get_item_num(buf,1);
    Cdbg(API_DBG, "find <slidingbrowse> filenum = %d", filenum);

    if(filenum > 0)
    {
        sbr->filelist = (Files **)calloc(filenum,sizeof(Files *));
        sbr->filenumber = filenum;
        get_file_list(buf,sbr);
    }

    my_free(buf);
    return 0;
}

void parsePropfind (xmlDocPtr doc, xmlNodePtr cur,Propfind *pfind)
{
        xmlChar *key;
        cur = cur->xmlChildrenNode;
        while (cur != NULL)
        {
            key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);

            if(key == NULL && cur->children == NULL) //if value is null and no children ,not parse
            {
                //xmlFree(key);
                cur = cur->next;
                continue;
            }

            if (!(xmlStrcmp(cur->name, (const xmlChar *)"status")))
            {
                pfind->status = atoi((const char *)key);
                if(pfind->status != 0)
                {
                    xmlFree(key);
                    return;
                }
            }

            if( !(xmlStrcmp(cur->name, (const xmlChar *)"isencrypt")))
            {
                pfind->isencrypt = atoi((const char *)key);
            }
            else if( !(xmlStrcmp(cur->name, (const xmlChar *)"size")))
            {
                pfind->size = atoll((const char *)key);
            }
            else if( !(xmlStrcmp(cur->name, (const xmlChar *)"scrip")))
            {
                pfind->script = atoi((const char *)key);
            }
            else if( !(xmlStrcmp(cur->name, (const xmlChar *)"type")))
            {
                strcpy(pfind->type,(const char *)key);
            }
            else if( !(xmlStrcmp(cur->name, (const xmlChar *)"id")))
            {
                pfind->id = atoi((const char *)key);
            }


            xmlFree(key);
            cur = cur->next;
        }
}

void parseCreatefolder (xmlDocPtr doc, xmlNodePtr cur,Createfolder *createfolder)
{
        xmlChar *key;
        cur = cur->xmlChildrenNode;
        while (cur != NULL)
        {
            key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);

            if(key == NULL && cur->children == NULL) //if value is null and no children ,not parse
            {
                //xmlFree(key);
                cur = cur->next;
                continue;
            }

            if (!(xmlStrcmp(cur->name, (const xmlChar *)"status")))
            {
                createfolder->status = atoi((const char *)key);
                if(createfolder->status != 0)
                {
                    xmlFree(key);
                    return;
                }
            }


            else if( !(xmlStrcmp(cur->name, (const xmlChar *)"scrip")))
            {
               strcpy(createfolder->scrip,(const char *)key);
            }
            else if( !(xmlStrcmp(cur->name, (const xmlChar *)"id")))
            {
                createfolder->id = atol((const char *)key);
            }


            xmlFree(key);
            cur = cur->next;
        }
}

void
        parseChangeseq (xmlDocPtr doc, xmlNodePtr cur,Changeseq *changeseq)
{
        xmlChar *key;
        cur = cur->xmlChildrenNode;
        while (cur != NULL)
        {
            key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);

            if(key == NULL && cur->children == NULL) //if value is null and no children ,not parse
            {
                //xmlFree(key);
                cur = cur->next;
                continue;
            }

            if (!(xmlStrcmp(cur->name, (const xmlChar *)"status")))
            {
                changeseq->status = atoi((const char *)key);
                if(changeseq->status != 0)
                {
                    xmlFree(key);
                    return;
                }
            }

            else if( !(xmlStrcmp(cur->name, (const xmlChar *)"scrip")))
            {
               strcpy(changeseq->scrip,(const char *)key);
            }
            else if( !(xmlStrcmp(cur->name, (const xmlChar *)"changeseq")))
            {
                changeseq->changeseq = atoi((const char *)key);
            }


            xmlFree(key);
            cur = cur->next;
        }
}


static int
        parseGetEntryinfo (xmlDocPtr doc, xmlNodePtr cur,Getentryinfo *ge)
{
    xmlChar *key;
    cur = cur->xmlChildrenNode;
    while (cur != NULL)
    {
        key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);

        /* if value is null and node is no children node,not parse */
        if(key == NULL && cur->children == NULL)
        {
            cur = cur->next;
            continue;
        }

        if (!(xmlStrcmp(cur->name, (const xmlChar *)"status")))
        {
           ge->status = atoi((const char *)key);
        }
        else if( !(xmlStrcmp(cur->name, (const xmlChar *)"scrip")))
        {
           strcpy(ge->scrip,(const char *)key);
        }
        else if( !(xmlStrcmp(cur->name, (const xmlChar *)"isfolder")))
        {
           ge->isfolder = atoi((const char *)key);
        }
        else if( !(xmlStrcmp(cur->name, (const xmlChar *)"display")))
        {
           strcpy(ge->display,(const char *)key);
        }
        else if( !(xmlStrcmp(cur->name, (const xmlChar *)"parent")))
        {
           ge->parent = atoi((const char *)key);
        }
        else if( !(xmlStrcmp(cur->name, (const xmlChar *)"headversion")))
        {
           ge->headversion = atoi((const char *)key);
        }
        else if( !(xmlStrcmp(cur->name, (const xmlChar *)"attribute")))
        {
            parseGetEntryinfo(doc,cur,ge);
        }
//        else if( !(xmlStrcmp(cur->name, (const xmlChar *)"creationtime")))
//        {
//               strcpy(ge->attr.creationtime,key);
//        }
        else if( !(xmlStrcmp(cur->name, (const xmlChar *)"lastaccesstime")))
        {
             strcpy(ge->attr.lastaccesstime,(const char *)key);
        }
        else if( !(xmlStrcmp(cur->name, (const xmlChar *)"lastwritetime")))
        {
            strcpy(ge->attr.lastwritetime,(const char *)key);
        }
//        else if( !(xmlStrcmp(cur->name, (const xmlChar *)"finfo")))
//        {
//            strcpy(ge->attr.finfo,key);
//        }
//        else if( !(xmlStrcmp(cur->name, (const xmlChar *)"x-timeforsynccheck")))
//        {
//               strcpy(ge->attr.xtimeforsynccheck,key);
//        }
//        else if( !(xmlStrcmp(cur->name, (const xmlChar *)"x-machinename")))
//        {
//            strcpy(ge->attr.xmachinename,key);
//        }
        else if( !(xmlStrcmp(cur->name, (const xmlChar *)"filesize")))
        {
            ge->filesize = atoll((const char *)key);
        }
        else if( !(xmlStrcmp(cur->name, (const xmlChar *)"treesize")))
        {
            ge->treesize = atoll((const char *)key);
        }
        else if( !(xmlStrcmp(cur->name, (const xmlChar *)"ishidden")))
        {
            ge->ishidden = atoi((const char *)key);
        }
        else if( !(xmlStrcmp(cur->name, (const xmlChar *)"isinfected")))
        {
            ge->isinfected = atoi((const char *)key);
        }
        else if( !(xmlStrcmp(cur->name, (const xmlChar *)"isbackup")))
        {
            ge->isbackup = atoi((const char *)key);
        }
        else if( !(xmlStrcmp(cur->name, (const xmlChar *)"isorigdeleted")))
        {
            ge->isorigdeleted = atoi((const char *)key);
        }
        else if( !(xmlStrcmp(cur->name, (const xmlChar *)"ispublic")))
        {
            ge->ispublic = atoi((const char *)key);
        }
        else if( !(xmlStrcmp(cur->name, (const xmlChar *)"createdtime")))
        {
            strcpy(ge->createdtime,(const char *)key);
        }

        xmlFree(key);
        cur = cur->next;
    }
    return 0;
}

void parseOperateEntry(xmlDocPtr doc, xmlNodePtr cur,Operateentry *oe)
{
        xmlChar *key;
        cur = cur->xmlChildrenNode;
        while (cur != NULL)
        {
            key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);

            if(key == NULL && cur->children == NULL) //if value is null and no children ,not parse
            {
                //xmlFree(key);
                cur = cur->next;
                continue;
            }

            if (!(xmlStrcmp(cur->name, (const xmlChar *)"status")))
            {
                oe->status = atoi((const char *)key);
            }
            else if( !(xmlStrcmp(cur->name, (const xmlChar *)"scrip")))
            {
               strcpy(oe->script,(const char *)key);
            }


            xmlFree(key);
            cur = cur->next;
        }
}

void parseMoveEntry(xmlDocPtr doc, xmlNodePtr cur,Moveentry *me)
{
        xmlChar *key;
        cur = cur->xmlChildrenNode;
        while (cur != NULL)
        {
            key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);

            if(key == NULL && cur->children == NULL) //if value is null and no children ,not parse
            {
                //xmlFree(key);
                cur = cur->next;
                continue;
            }

            if (!(xmlStrcmp(cur->name, (const xmlChar *)"status")))
            {
                me->status = atoi((const char *)key);
            }
            else if( !(xmlStrcmp(cur->name, (const xmlChar *)"scrip")))
            {
               strcpy(me->script,(const char *)key);
            }
            else if( !(xmlStrcmp(cur->name, (const xmlChar *)"logmessage")))
            {
               strcpy(me->logmessage,(const char *)key);
            }

            xmlFree(key);
            cur = cur->next;
        }
}

int StrToHex(char *src,int len)
{
    int sum;
    char str[4] = {0};
    if(len == 1)
    {
       strcpy(str,src);
       sum = atoi(str);
    }
    else if(len == 2)
    {
       strncpy(str,src,1);
       sum = atoi(str)*16;

       memset(str,0,sizeof(str));
       strcpy(str,src+1);
       sum += atoi(str);
   }

    return sum;
}

void parseGetUserState(xmlDocPtr doc, xmlNodePtr cur,Getuserstate *gu)
{
        xmlChar *key;
        cur = cur->xmlChildrenNode;
        while (cur != NULL)
        {
            key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);

            if(key == NULL && cur->children == NULL) //if value is null and no children ,not parse
            {
                //xmlFree(key);
                cur = cur->next;
                continue;
            }

            if (!(xmlStrcmp(cur->name, (const xmlChar *)"status")))
            {
                gu->status = atoi((const char *)key);
            }
            else if (!(xmlStrcmp(cur->name, (const xmlChar *)"servicestate")))
            {
               parseGetUserState(doc,cur,gu);
            }
            else if( !(xmlStrcmp(cur->name, (const xmlChar *)"userstate")))
            {    
                gu->userstate = StrToHex((char *)key,strlen((const char *)key));
            }
            else if( !(xmlStrcmp(cur->name, (const xmlChar *)"extensionstarttime")))
            {
               strcpy(gu->extensionstarttime,(const char *)key);
            }
            else if( !(xmlStrcmp(cur->name, (const xmlChar *)"extensionendtime")))
            {
               strcpy(gu->extensionendtime,(const char *)key);
            }

            xmlFree(key);
            cur = cur->next;
        }
        //Cdbg(API_DBG, "parse end\n");
}

/*int parseHTMLTag(htmlDocPtr doc,htmlNodePtr cur)
{
    xmlChar *key;
    cur = cur->xmlChildrenNode;
    while (cur != NULL)
    {
        key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);

        if(key == NULL && cur->children == NULL) //if value is null and no children ,not parse
        {
            //xmlFree(key);
            cur = cur->next;
            continue;
        }

        if (!(xmlStrcmp(cur->name, (const xmlChar *)"body")))
        {
           parseHTMLTag(doc,cur);
        }

        if (!(xmlStrcmp(cur->name, (const xmlChar *)"h1")))
        {
           handle_error(S_HTTP_ERROR,key);
           xmlFree(key);
           break;
        }

        //Cdbg(API_DBG, "cur->name is %s,key is %s\n",cur->name,key);

        xmlFree(key);
        cur = cur->next;
    }
}

int myParseHTML(char *name)
{
        htmlNodePtr cur;
        htmlDocPtr          ptr = htmlParseFile( name, "utf-8" );

        if( !ptr ) {
            fprintf( stderr, "unable to parse doc %s\n", name );
        }
        else {
            fprintf( stderr, "got parsed document: %s\n", name );
        }
        xmlCleanupParser();
        xmlMemoryDump();

        cur = xmlDocGetRootElement(ptr);

        if (cur == NULL) {
            Cdbg(API_DBG, "%s empty document\n",name);

            xmlFreeDoc(ptr);
            return -1;
        }

        while (cur != NULL) {

            //Cdbg(API_DBG, "name is %s\n",cur->name);

            if ((!xmlStrcmp(cur->name, (const xmlChar *)"html")))
            {
                parseHTMLTag(ptr,cur);
            }

            cur = cur->next;
        }
        xmlFreeDoc(ptr);

        return( 0 );

}*/


int parseDoc1(char *docname,void *obj)
{
    xmlDocPtr doc;
    xmlNodePtr cur;

    // Cdbg(API_DBG, "parseDoc1 : docname : %s \n",docname);
        
        
    doc = xmlParseFile(docname);

    if (doc == NULL ) {
        //fprintf(stderr,"%s not parsed successfully. \n",docname);
        
        snprintf(dbg_msg, sizeof(dbg_msg), "parseDoc : %s parse failure. \n",docname);
        // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
        Cdbg(API_DBG, "%s", dbg_msg);

        return -1;
    } else {
        snprintf(dbg_msg, sizeof(dbg_msg), "parseDoc : %s parse successfully.",docname);
        // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
        Cdbg(API_DBG, "%s", dbg_msg);
    }

    cur = xmlDocGetRootElement(doc);

    if (cur == NULL) {
        //fprintf(stderr,"%s empty document\n",docname);
        Cdbg(API_DBG, "parseDoc : %s empty document\n",docname);

        xmlFreeDoc(doc);
        return -1;
    } else {
        snprintf(dbg_msg, sizeof(dbg_msg), "parseDoc : %s get xmlDocGetRootElement successfully\n",docname);
        // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
        Cdbg(API_DBG, "%s", dbg_msg);
    }

    while (cur != NULL) {

        if ((!xmlStrcmp(cur->name, (const xmlChar *)"requestservicegateway")))
        {
            parseGateway(doc, cur,(Servicegateway *)obj);
        }
        else  if ((!xmlStrcmp(cur->name, (const xmlChar *)"initattachabledevice")))
        {
            parseInitattachabledevice(doc,cur,(Initattachabledevice *)obj);
        }
        else  if ((!xmlStrcmp(cur->name, (const xmlChar *)"browseattachabledevice")))
        {
            parseBrowseAttachableDevice(doc,cur,(BrowseAttachableDevice *)obj);
        }
        else  if ((!xmlStrcmp(cur->name, (const xmlChar *)"aaa")))
        {
            parseAaa(doc,cur,(Aaa *)obj);
        }
        else  if ((!xmlStrcmp(cur->name, (const xmlChar *)"browse")))
        {
            if(get_server_item_size(doc,cur,(Browse *)obj) == -1)
                return -1;
            parseBrowse(doc,cur,(Browse *)obj);
        }
        else  if ((!xmlStrcmp(cur->name, (const xmlChar *)"propfind")))
        {
            parsePropfind(doc,cur,(Propfind *)obj);
        }
        else  if ((!xmlStrcmp(cur->name, (const xmlChar *)"create")))
        {
            parseCreatefolder(doc,cur,(Createfolder *)obj);
        }
        else  if ((!xmlStrcmp(cur->name, (const xmlChar *)"getchangeseq")))
        {
            parseChangeseq(doc,cur,(Changeseq *)obj);
        }
        else  if ((!xmlStrcmp(cur->name, (const xmlChar *)"initbinaryupload")))
        {
            parseInitbinaryupload(doc,cur,(Initbinaryupload *)obj);
        }
        else  if ((!xmlStrcmp(cur->name, (const xmlChar *)"resumebinaryupload")))
        {
            parseResumebinaryupload(doc,cur,(Resumebinaryupload *)obj);
        }
        else  if ((!xmlStrcmp(cur->name, (const xmlChar *)"finishbinaryupload")))
        {
            parseFinishbinaryupload(doc,cur,(Finishbinaryupload *)obj);
        }
        else  if ((!xmlStrcmp(cur->name, (const xmlChar *)"getinfo")))
        {
            parseGetinfo(doc,cur,(Getinfo *)obj);
        }
        else  if ((!xmlStrcmp(cur->name, (const xmlChar *)"getmysyncfolder")))
        {
            parseGetmysyncfolder(doc,cur,(Getmysyncfolder *)obj);
        }
        else  if ((!xmlStrcmp(cur->name, (const xmlChar *)"getentryinfo")))
        {
            parseGetEntryinfo(doc,cur,(Getentryinfo *)obj);
        }
        else  if( (!xmlStrcmp(cur->name, (const xmlChar *)"remove") ) ||
              !xmlStrcmp(cur->name, (const xmlChar *)"rename") )
        {
            parseOperateEntry(doc,cur,(Operateentry *)obj);
        }
        else  if ((!xmlStrcmp(cur->name, (const xmlChar *)"move")))
        {
            parseMoveEntry(doc,cur,(Moveentry *)obj);
        }
        else  if ((!xmlStrcmp(cur->name, (const xmlChar *)"getuserstate")))
        {
            parseGetUserState(doc,cur,(Getuserstate *)obj);
        }
        else  if ((!xmlStrcmp(cur->name, (const xmlChar *)"getpersonalsystemfolder")))
        {
            parseGetPersonalSystemFolder(doc,cur,(Getpersonalsystemfolder *)obj);
        }

        cur = cur->next;
    }
    xmlFreeDoc(doc);
        
    return 0;
}



int parseBrowseAttachableDeviceXml(char *docname,void *obj)
{
    xmlDocPtr doc;
    xmlNodePtr cur;

    // Cdbg(API_DBG, "parseDoc1 : docname : %s \n",docname);
        
        
    doc = xmlParseFile(docname);

    if (doc == NULL ) {
        //fprintf(stderr,"%s not parsed successfully. \n",docname);
        Cdbg(API_DBG, "parseDoc : %s not parsed successfully. \n",docname);
        return -1;
    } else {
        Cdbg(API_DBG, "parseDoc : %s parsed successfully. \n",docname);
    }

    cur = xmlDocGetRootElement(doc);

    if (cur == NULL) {
        //fprintf(stderr,"%s empty document\n",docname);
        Cdbg(API_DBG, "parseDoc : %s empty document\n",docname);

        xmlFreeDoc(doc);
        return -1;
    } else {
        Cdbg(API_DBG, "parseDoc : %s get xmlDocGetRootElement successfully\n",docname);
    }

    while (cur != NULL) {

        if ((!xmlStrcmp(cur->name, (const xmlChar *)"browseattachabledevice")))
        {
            parseBrowseAttachableDeviceList(doc,cur,(BrowseAttachableDevice *)obj);
        }

        cur = cur->next;
    }
    xmlFreeDoc(doc);
        
    return 0;
}



int checkBrowseAttachableDeviceXml(char *docname,char *device_id)
{

    int check_device_id_status = -1;

    xmlDocPtr doc;
    xmlNodePtr cur;

    // Cdbg(API_DBG, "parseDoc1 : docname : %s \n",docname);
        
        
    doc = xmlParseFile(docname);

    if (doc == NULL ) {
        //fprintf(stderr,"%s not parsed successfully. \n",docname);
        Cdbg(API_DBG, "parseDoc : %s not parsed successfully. \n",docname);
        return -1;
    } else {
        Cdbg(API_DBG, "parseDoc : %s parsed successfully. \n",docname);
    }

    cur = xmlDocGetRootElement(doc);

    if (cur == NULL) {
        //fprintf(stderr,"%s empty document\n",docname);
        Cdbg(API_DBG, "parseDoc : %s empty document\n",docname);

        xmlFreeDoc(doc);
        return -1;
    } else {
        Cdbg(API_DBG, "parseDoc : %s get xmlDocGetRootElement successfully\n",docname);
    }

    while (cur != NULL) {

        if ((!xmlStrcmp(cur->name, (const xmlChar *)"browseattachabledevice")))
        {
            check_device_id_status = parseBrowseAttachableDeviceListCheck(doc,cur, device_id);
        }

        cur = cur->next;
    }
    xmlFreeDoc(doc);
        
    return check_device_id_status;
}

int sendBrowseRequest(char *filename,char *url,char *postdata,char *cookie,
                      char *header,struct MemoryStruct *chunk)
{
    //Cdbg(API_DBG, "send request postdata=%s \n",postdata);

    FILE *fd;
    CURL *curl;
    CURLcode res = CURLE_FAILED_INIT;
    char cookies[NORMALSIZE];
    char err_message[NORMALSIZE] = {0};
    //int status;
    struct curl_slist *headers = NULL;

    //Cdbg(API_DBG, "open %s file \n",filename);

    fd = fopen(filename,"w");
    if(NULL == fd)
    {
        Cdbg(API_DBG, "open %s file fail\n",filename);
        return -1;
    }

    //Cdbg(API_DBG, "curl  11 \n");
    curl = curl_easy_init();
    //Cdbg(API_DBG, "curl  22 \n");

    //curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
    curl_easy_setopt(curl,CURLOPT_ERRORBUFFER,err_message);
    curl_easy_setopt(curl,CURLOPT_FAILONERROR,1);
    //Cdbg(API_DBG, "curl  aa \n");
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
    //curl_easy_setopt(curl,CURLOPT_TIMEOUT,15);
    curl_easy_setopt(curl,CURLOPT_CONNECTTIMEOUT, 30); // 2017/07/31 add


    if(cookie != NULL)
    {
        Cdbg(API_DBG, "cookie :  %s ", cookie);
        curl_easy_setopt(curl,CURLOPT_COOKIE,cookie);
    }
    else
    {
        snprintf(cookies,NORMALSIZE,"OMNISTORE_VER=1_0; path=/;sid=%s;",sid);

        //Cdbg(API_DBG, "cookies :  %s ", cookies);

        curl_easy_setopt(curl,CURLOPT_COOKIE,cookies);
    }

    
    // Cdbg(API_DBG, "header :  %s ", header);

    if(header != NULL)
    {
        headers = curl_slist_append(headers,header);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER,headers);
    }

    //Cdbg(API_DBG, "curl  bb \n");
    //curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION,write_data);
    //curl_easy_setopt(curl,CURLOPT_WRITEDATA,fd);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)chunk);

    curl_easy_setopt(curl, CURLOPT_URL, url);

    if(postdata != NULL)
    {
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS,postdata);
    }

    //curl_easy_setopt(curl,CURLOPT_TIMEOUT,30); //90 -> 60
    curl_easy_setopt(curl,CURLOPT_CONNECTTIMEOUT, 30); // 2017/07/31 add

    //Cdbg(API_DBG, "curl perform start \n");
    if(!exit_loop)
        res = curl_easy_perform(curl);
    //Cdbg(API_DBG, "curl perform end \n");

    if( res != CURLE_OK )
    {
        Cdbg(API_DBG, "error message is %s \n",err_message);
        if(chunk->memory)
            free(chunk->memory);
        Cdbg(API_DBG, "curl error is %s \n",curl_easy_strerror(res));
        //curl_easy_cleanup(curl);
        //return -1;
    }

    if(header)
        curl_slist_free_all(headers);

    curl_easy_cleanup(curl);

    fclose(fd);

      return res;
}


void checkAaewsRun()
{

    while(1) {

        FILE *in;

        in = fopen("/tmp/diag_db_cloud/aaewsRun","r");

        if (in == NULL) {

            Cdbg(API_DBG, "aaewsRun file can't open, waiting aaews run : 10 seconds");


        } else {

            fclose(in);

            Cdbg(API_DBG, "aaewsRun file exist, run uploader");

            break;

        }

        sleep(10);

        process_aaews_event_config();
        
    }
}

int writeAaewsRefreshToken() {


    FILE *in;

    char buffer[] = { 'a','e','e', 'w','s' };

    in = fopen("/tmp/diag_db_cloud/aaewsRefreshToken","w");

    if (in == NULL) {

        Cdbg(API_DBG, "aaewsRefreshToken file can't open \n");

        return 0;

    } else {

        fwrite(buffer,1,sizeof(buffer), in);

        Cdbg(API_DBG, "aaewsRefreshToken file write OK\n");

    }

    fclose(in);

    return 1;
}

int checkAaewsRefreshTokenOk()
{

    FILE *in;

    in = fopen("/tmp/diag_db_cloud/aaewsRefreshTokenOk","r");

    if (in == NULL) {

        Cdbg(API_DBG, "aaewsRefreshTokenOk file can't open, waiting aaews auth : 120 seconds \n");

        return 0;

    } else {

        Cdbg(API_DBG, "aaewsRefreshTokenOk file exist, aicam running\n");

    }

    fclose(in);

    return 1;

}

void refreshTokenDel() {

    if(remove("/tmp/diag_db_cloud/aaewsRefreshToken") == 0 ) {
        Cdbg(API_DBG, "aaewsRefreshToken file removed\n");
    } else {
        Cdbg(API_DBG, "aaewsRefreshToken file remove error\n");
    }

    if(remove("/tmp/diag_db_cloud/aaewsRefreshTokenOk") == 0 ) {
        Cdbg(API_DBG, "aaewsRefreshTokenOk file removed\n");
    } else {
        Cdbg(API_DBG, "aaewsRefreshTokenOk file remove error\n");
    }


}

void print_count_call_api()
{
    Cdbg(API_DBG, "**************\n");
    Cdbg(API_DBG, "print count call api start\n");
    Cdbg(API_DBG, "requestservicegateway=%d\n",api_count.requestservicegateway);
    Cdbg(API_DBG, "acquiretoken=%d\n",api_count.acquiretoken);
    Cdbg(API_DBG, "getinfo=%d\n",api_count.getinfo);
    Cdbg(API_DBG, "getmysyncfolder=%d\n",api_count.getmysyncfolder);
    Cdbg(API_DBG, "getpersonalsystemfolder=%d\n",api_count.getpersonalsystemfolder);
    Cdbg(API_DBG, "getuserstate=%d\n",api_count.getuserstate);
    Cdbg(API_DBG, "browse=%d\n",api_count.browse);
    Cdbg(API_DBG, "profind=%d\n",api_count.propfind);
    Cdbg(API_DBG, "create=%d\n",api_count.create);
    Cdbg(API_DBG, "rename=%d\n",api_count.rename);
    Cdbg(API_DBG, "remove=%d\n",api_count.remove);
    Cdbg(API_DBG, "move=%d\n",api_count.move);
    Cdbg(API_DBG, "getentryinfo=%d\n",api_count.getentryinfo);
    Cdbg(API_DBG, "getchangeseq=%d\n",api_count.getchangeseq);
    Cdbg(API_DBG, "initbinaryupload=%d\n",api_count.initbinaryupload);
    Cdbg(API_DBG, "resumebinaryupload=%d\n",api_count.resumebinaryupload);
    Cdbg(API_DBG, "finishbinaryupload=%d\n",api_count.finishbinaryupload);
    Cdbg(API_DBG, "directdownload=%d\n",api_count.directdownload);
    Cdbg(API_DBG, "**************\n");
}

int count_call_api(const char *url)
{
    if(strstr(url,"requestservicegateway"))
        api_count.requestservicegateway++;
    else if(strstr(url,"acquiretoken"))
        api_count.acquiretoken++;
    else if(strstr(url,"getinfo"))
        api_count.getinfo++;
    else if(strstr(url,"getmysyncfolder"))
        api_count.getmysyncfolder++;
    else if(strstr(url,"getpersonalsystemfolder"))
        api_count.getpersonalsystemfolder++;
    else if(strstr(url,"getuserstate"))
        api_count.getuserstate++;
    else if(strstr(url,"browse"))
        api_count.browse++;
    else if(strstr(url,"propfind"))
        api_count.propfind++;
    else if(strstr(url,"create"))
        api_count.create++;
    else if(strstr(url,"rename"))
        api_count.rename++;
    else if(strstr(url,"remove"))
        api_count.remove++;
    else if(strstr(url,"move"))
        api_count.move++;
    else if(strstr(url,"getentryinfo"))
        api_count.getentryinfo++;
    else if(strstr(url,"getchangeseq"))
        api_count.getchangeseq++;
    else if(strstr(url,"initbinaryupload"))
        api_count.initbinaryupload++;
    else if(strstr(url,"resumebinaryupload"))
        api_count.resumebinaryupload++;
    else if(strstr(url,"finishbinaryupload"))
        api_count.finishbinaryupload++;
    else if(strstr(url,"directdownload"))
        api_count.directdownload++;

    return 0;
}



int sendRequest(char *filename,char *url,char *postdata,char *cookie,char *header)
{
    FILE *fd;
    CURL *curl;
    CURLcode res = CURLE_FAILED_INIT;
    char cookies[NORMALSIZE];
    char err_message[NORMALSIZE] = {0};
    struct curl_slist *headers = NULL;

    fd = fopen(filename,"w");
    if(NULL == fd)
    {
        Cdbg(API_DBG, "open %s file fail\n",filename);
        return -1;
    }

    curl = curl_easy_init();

    //curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
//    curl_easy_setopt(curl,CURLOPT_ERRORBUFFER,err_message);
//    curl_easy_setopt(curl,CURLOPT_FAILONERROR,1);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
    //curl_easy_setopt(curl,CURLOPT_SSL_VERIFYHOST,0);
    //curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);

    //curl_easy_setopt(curl,CURLOPT_TIMEOUT,30); // 90 -> 60
    curl_easy_setopt(curl,CURLOPT_CONNECTTIMEOUT, 30); // 2017/07/31 add

    /* abort if slower than 30 bytes/sec during 30 seconds */
    curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, 30L);
    curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, 30L);


    if(cookie != NULL)
    {
                        
        snprintf(dbg_msg, sizeof(dbg_msg), "cookie != NULL, cookie :  %s ", cookie);
        // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
        Cdbg(API_DBG, "%s", dbg_msg);

        curl_easy_setopt(curl,CURLOPT_COOKIE,cookie);
    }
    else
    {

        snprintf(dbg_msg, sizeof(dbg_msg), "cookie = OMNISTORE_VER=1_0; path=/;sid=%s;v=%s",sid,VERSION);
        // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
        Cdbg(API_DBG, "%s", dbg_msg);

        snprintf(cookies,NORMALSIZE,"OMNISTORE_VER=1_0; path=/;sid=%s;v=%s",sid,VERSION);

        //Cdbg(API_DBG, "cookies :  %s ", cookies);

        curl_easy_setopt(curl,CURLOPT_COOKIE,cookies);
    }

    
    // Cdbg(API_DBG, "header :  %s ", header);

    if(header != NULL)
    {
        snprintf(dbg_msg, sizeof(dbg_msg), "header != null");
        // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
        Cdbg(API_DBG, "%s", dbg_msg);

        headers = curl_slist_append(headers,header);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER,headers);

    } else {

        snprintf(dbg_msg, sizeof(dbg_msg), "header == null");
        // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
        Cdbg(API_DBG, "%s", dbg_msg);
    }

    curl_easy_setopt(curl,CURLOPT_WRITEDATA,fd);
    curl_easy_setopt(curl, CURLOPT_URL, url);

    if(postdata != NULL)
    {
        snprintf(dbg_msg, sizeof(dbg_msg), "postdata != null, postdata : %s", postdata);
        // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
        Cdbg(API_DBG, "%s", dbg_msg);

        curl_easy_setopt(curl, CURLOPT_POSTFIELDS,postdata);
    } else {

        snprintf(dbg_msg, sizeof(dbg_msg), "postdata == null");
        // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
        Cdbg(API_DBG, "%s", dbg_msg);
    }

    //if(!exit_loop)
    res = curl_easy_perform(curl);

    if( res != CURLE_OK )
    {

        snprintf(dbg_msg, sizeof(dbg_msg), "error message res = %d \n", res);
        // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
        Cdbg(API_DBG, "%s", dbg_msg);

        if(CURLE_OPERATION_TIMEDOUT == res) {

            snprintf(dbg_msg, sizeof(dbg_msg), "CURLE_OPERATION_TIMEDOUT");
            // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
            Cdbg(API_DBG, "%s", dbg_msg);
        }

    } else {

        snprintf(dbg_msg, sizeof(dbg_msg), "res == CURLE_OK");
        // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
        Cdbg(API_DBG, "%s", dbg_msg);

        // count_call_api(url);
    }

    if(header)
        curl_slist_free_all(headers);

    curl_easy_cleanup(curl);

    fclose(fd);

    return res;
}


char * readFile(char *filename) {

    FILE *f = fopen(filename, "rb");

    if(NULL == f)
    {
        snprintf(dbg_msg, sizeof(dbg_msg), "open %s file fail\n",filename);
        // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
        Cdbg(API_DBG, "%s", dbg_msg);

        return NULL;
    } 


    snprintf(dbg_msg, sizeof(dbg_msg), "open %s file \n",filename);
    // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
    Cdbg(API_DBG, "%s", dbg_msg);

    fseek(f, 0, SEEK_END);
    long file_len = ftell(f);
    fseek(f, 0, SEEK_SET);


    snprintf(dbg_msg, sizeof(dbg_msg), "[%s] file len : %ld \n", filename, file_len);
    // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
    Cdbg(API_DBG, "%s", dbg_msg);



    char *buffer = (char *) malloc(file_len+1);

    buffer[file_len] = '\0';
    fread(buffer, 1, file_len, f);

    fclose(f);

    return buffer;
}



int sendGoogleRequest(char *filename, char *url, char *postdata, char *cookie, char *header, char * request_type)
{


    FILE *fd;
    CURL *curl;
    CURLcode res = CURLE_FAILED_INIT;
    char cookies[NORMALSIZE];
    char err_message[NORMALSIZE] = {0};

    struct curl_slist *headers = NULL;


    fd = fopen(filename,"wb");
    if(NULL == fd)
    {
        Cdbg(API_DBG, "open %s file fail\n",filename);
        return -1;
    }


    curl = curl_easy_init();

    // 1 : GET , 2 : POST && ( header -> Content-Type: application/x-www-form-urlencoded )
    // 3 : POST && ( header -> content-type: application/json )
    if( strcmp(request_type, HTTP_REQUEST_GET) == 0) {

        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET");

    } else if( strcmp(request_type, HTTP_REQUEST_POST) == 0) {

        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");

    } else if( strcmp(request_type, HTTP_REQUEST_POST_JSONTYPE) == 0) {

        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");

    }
    

    curl_easy_setopt(curl, CURLOPT_URL, url);

    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
//    curl_easy_setopt(curl,CURLOPT_ERRORBUFFER,err_message);
//    curl_easy_setopt(curl,CURLOPT_FAILONERROR,1);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
    //curl_easy_setopt(curl,CURLOPT_SSL_VERIFYHOST,0);
    //curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);

    //curl_easy_setopt(curl,CURLOPT_TIMEOUT,30); // 90 -> 60
    curl_easy_setopt(curl,CURLOPT_CONNECTTIMEOUT, 30); // 2017/07/31 add

    /* abort if slower than 30 bytes/sec during 30 seconds */
    curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, 30L);
    curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, 30L);

    // curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
    // curl_easy_setopt(curl,CURLOPT_READDATA,fd);

    curl_easy_setopt(curl,CURLOPT_WRITEDATA,fd);

    // curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, (curl_off_t)file_info.st_size);
    // curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (curl_off_t)file_info.st_size);


    if(header != NULL)
    {

        Cdbg(API_DBG, "header != null");

        headers = curl_slist_append(headers, header);

        if( strcmp(request_type, HTTP_REQUEST_POST_JSONTYPE) == 0) {

            headers = curl_slist_append(headers, "content-type: application/json");

        }


        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    } else {

        Cdbg(API_DBG, "header == %s", header);
    }


    if(postdata != NULL)
    {
        Cdbg(API_DBG, "postdata : %s", postdata);

        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postdata);

    } else {

        Cdbg(API_DBG, "postdata == null");
    }


    res = curl_easy_perform(curl);

    if( res != CURLE_OK )
    {

        Cdbg(API_DBG, "error message res = %d \n", res);

        if(CURLE_OPERATION_TIMEDOUT == res) {

            Cdbg(API_DBG, "CURLE_OPERATION_TIMEDOUT");
        }

    } else {

        Cdbg(API_DBG, "res == CURLE_OK");

        // count_call_api(url);
    }

    if(header)
        curl_slist_free_all(headers);


    curl_easy_cleanup(curl);

    fclose(fd);

    return res;
}



int getServiceGateway(char *username, char *password,Servicegateway *sg)
{
    int status;

    memset(sg,0,sizeof(Servicegateway));

    char *url = "https://cloudsyncportal01.asuswebstorage.com/member/requestservicegateway/";

    char postdata[512];

    snprintf(postdata,512,"<requestservicegateway><userid>%s</userid><password>%s</password><language>zh_TW</language><service>1</service><time>2008/1/1</time></requestservicegateway>",username,password);

    status = sendRequest(gateway_xml,url,postdata,NULL,NULL);

    if( status != 0 )
    {
        handle_error(status,"curl");
        return S_NETWORK_FAIL;
    }

    if( parseDoc1(gateway_xml,sg) == -1)
    {
        return S_NETWORK_FAIL;
    }

   return 0;
}

int obtainGateway(char *user,char *pwd,Servicegateway *sg)
{
    int GetGateOK = 0;
    int status = -1;

    //while(GetGateOK != 1 && exit_loop != 1)
    while(GetGateOK != 1)
    {

        status = getServiceGateway(user,pwd,sg); //fill Service_geteway struct member info

        Cdbg(API_DBG, "obtainGateway status : %d\n", status);

        if(status == S_NETWORK_FAIL)
        {
            Cdbg(API_DBG, "NETWORK_FAIL, status : %d\n", status);
            sleep(5);

           //return -1;
           //check_network_state();

            Cdbg(API_DBG, "network retry \n");
            continue;
        }

        status = sg->status;

        Cdbg(API_DBG, "obtainGateway sg->status : %d\n", status);

        if(status == S_AUTH_FAIL)
        {
            Cdbg(API_DBG, "Authentication Failed status : %d\n", status);

            return S_AUTH_FAIL;
        }

        if( status != 0 )
        {
            handle_error(status,"gateway");
            enter_sleep_time(1000*500,NULL);
            continue;
        }

        GetGateOK = 1;
    }

    return 0;
}

char *makeAuthorize()
{
    char *header = NULL;
    char header_signature_method[64];
    char header_timestamp[64];
    char header_nonce[64];
    char *header_signature = NULL;
    char prekey[128];
    unsigned long int sec;

    char query_string[1024];
    char *incode_string = NULL;
    char *sha1_string = NULL;

    snprintf(header_signature_method,64,"%s","HMAC-SHA1");
    // snprintf(prekey,128,"%s","03805FDC4B594FDEA89183D2ADA82586"); //2015.5.22 111C243AC3224439A5C619423B39F7AF -> 03805FDC4B594FDEA89183D2ADA82586
    snprintf(prekey,128,"%s", progKey);

    sec = time((time_t *)NULL);
    snprintf(header_timestamp,64,"%lu",sec);
    snprintf(header_nonce,64,"%lu",sec);

    snprintf( query_string,1024,"nonce=%s&signature_method=%s&timestamp=%s",header_nonce,header_signature_method,header_timestamp);
    incode_string = oauth_url_escape(query_string);
    if(NULL == incode_string)
    {
        handle_error(S_URL_ESCAPE_FAIL,"makeAuthorize");
        return NULL;
    }

    sha1_string = oauth_sign_hmac_sha1(incode_string,prekey);

    if(NULL == sha1_string)
    {
        handle_error(S_SHA1_FAIL,"makeAuthorize");
        my_free(incode_string);
        return NULL;
    }

    header_signature = oauth_url_escape(sha1_string);
    if(NULL == header_signature)
    {
        handle_error(S_URL_ESCAPE_FAIL,"makeAuthorize");
        my_free(incode_string);
        my_free(sha1_string);
        return NULL;
    }

    header = (char *)malloc(sizeof(char*)*1024);

    if(header == NULL)
    {
        my_free(incode_string);
        my_free(sha1_string);
        my_free(header_signature);
        return NULL;
    }

    snprintf(header,1024,"Authorization:signature_method=\"%s\",timestamp=\"%s\",nonce=%s,signature=\"%s\"",header_signature_method,header_timestamp,header_nonce,header_signature);

    my_free(incode_string);
    my_free(sha1_string);
    my_free(header_signature);

    snprintf(dbg_msg, sizeof(dbg_msg), "header: %s \n", header);
    // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
    Cdbg(API_DBG, "%s", dbg_msg);

    return header;
}



int getToken(char *username, char *password,char *key,int first)
{
    int status;
    char url[256];
    char postdata[512];
    char *header;
    char cookie[512];
    int error_type;
    char msg[512] = {0};
    char filename[512] = {0};
    memset(&aaa,0,sizeof(Aaa));

    header = makeAuthorize();

    snprintf(url,256,"https://%s/member/acquiretoken/",sergate.gateway);

    Cdbg(API_DBG, "https://%s/member/acquiretoken/\n", sergate.gateway);

    snprintf(cookie,512,"OMNISTORE_VER=1_0; path=/;sid=%s;",sid);
    snprintf(postdata,512,"<aaa><userid>%s</userid><password>%s</password><time>2008/1/1</time><auxpassword>%s</auxpassword></aaa>",username,password,key);

    Cdbg(API_DBG, "cookie: %s \n", cookie);
    Cdbg(API_DBG, "header: %s \n", header);
    Cdbg(API_DBG, "postdata: %s \n", postdata);

    status = sendRequest(token_xml,url,postdata,cookie,header);
    my_free(header);

    if( status != 0 )
    {
        handle_error(status,"curl");
        return S_NETWORK_FAIL;
    }


#if 1
    if( parseDoc1(token_xml,&aaa) == -1)
        return S_NETWORK_FAIL;

    if(aaa.status != 0 )
    {
       handle_error(aaa.status,"token");

       status = aaa.status;
       error_type = S_ERROR;

       if(status == S_AUTH_FAIL)
       {
           //error_type = S_ERROR;
           strcpy(msg,"Authentication Failed");
       }
       else if(status == S_OTP_AUTH_FAIL)
       {
           if(first)
           {
               if(strlen(cfg.otp_key)>0)
               {
                   if(strlen(cfg.captcha)>0)
                       return status;            // when need otp and captcha no write log 13/11/1
                   else
                        strncpy(msg,"Security Code error.WARNING:If the OTP authentication "
                              "Failures reaches 10 times,OTP will be locked,you must visit the "
                              "official website ,OTP can be used again after unlock it",512);
               }
               else
                   strcpy(msg,"OTP athentication failed.Please input Security Code");
           }
           else
               strcpy(msg,"Security Code has expired, please re-enter");

       }
       else if(status == S_OTP_ID_LOCKED)
       {
           strncpy(msg,"OTP Services Credential ID is LOCKED state,you must visit "
                  "the official website can be used again after unlock OTP services",512);
       }
       else if(status == S_CAPTCHA_FAIL)
       {
           error_type = S_NEEDCAPTCHA;
           strcpy(msg,"Need to enter the CAPTCHA");
           strcpy(filename,aaa.auxpasswordurl);
       }


       write_log(error_type,msg,filename);

       return aaa.status;
    }
#endif

    return 0;

}

int record_system_token(char *filename,Aaa *aa)
{
    FILE *fp = NULL;
    int len;

    fp = fopen(filename,"wb");
    if(fp == NULL)
    {
        Cdbg(API_DBG, "open %s fail\n",filename);
        return -1;
    }

    strncpy(aa->gateway,sergate.gateway,MINSIZE);
    strncpy(aa->user,username,256);
    strncpy(aa->pwd,password,256);
    len = fwrite(aa,sizeof(Aaa),1,fp);
    fclose(fp);

    return 0;
}

int obtainToken(char *user,char *pwd,struct asus_config *cfg,int first)
{
    int error_time = 0;
    int have_error_log = 0;
    char temp_key[8] = {0};
    int  otp_and_captcha = 0;
    int status = -1;

    if(strlen(cfg->captcha) > 0)
    {
        strcpy(temp_key,cfg->captcha);
        if(strlen(cfg->otp_key) >0)
            otp_and_captcha = 1;
    }
    else if(strlen(cfg->otp_key) > 0)
            strcpy(temp_key,cfg->otp_key);

    while(exit_loop != 1)
    {
        if(error_time > 5 && have_error_log != 1)
        {
            write_log(S_ERROR,"Can not get server information",""); //modify by gauss 2014/3/10
            have_error_log = 1;
        }

        status = getToken(user,pwd,temp_key,first);

        Cdbg(API_DBG, "obtainToken token status=%d\n",status);

        if(status !=0)
        {
            if(status == S_NETWORK_FAIL)
            {
                Cdbg(API_DBG, "obtainToken NETWORK_FAIL\n");
                sleep(10);
                //error_time++;
                //enter_sleep_time(1000*300,NULL);
                //check_network_state();
                continue;

            } else {

#ifdef IPKG
                char fullname[256] = {0};
                sprintf(fullname,"%s/%s",mount_path,token_filename);
                remove(fullname);
                remove(record_token_file);
#endif
                if(otp_and_captcha == 1 && status == S_OTP_AUTH_FAIL)
                {

                    Cdbg(API_DBG, "obtainToken need captcha and otp\n");

                    memset(temp_key,0,sizeof(temp_key));
                    strcpy(temp_key,cfg->otp_key);
                    otp_and_captcha = 0;
                    continue;
                }

                /*if(status == S_AUTH_FAIL)
                {
                    //write_log(S_ERROR,"Authentication Failed","");
                    AuthFailTimes++;
                    Cdbg(API_DBG, "AuthFailTimes=%d\n",AuthFailTimes);
                    sleep(1);
                    if(AuthFailTimes >105)
                        return;
                    continue;
                }*/
                if(status == S_AUTH_FAIL || status == S_OTP_AUTH_FAIL || status == S_OTP_ID_LOCKED ||
                   status == S_CAPTCHA_FAIL) {
                    Cdbg(API_DBG, "AUTH_FAIL status : %d\n", status);
                    return -1;
                } else // other error code ,such as 999
                {
                    enter_sleep_time(1000*300,NULL);
                    continue;
                }
            }
        }

        record_system_token(system_token,&aaa);
        break;
    }

    return 0;
}

Getinfo *getInfo(char *username,char *server)
{
    char url[NORMALSIZE];
    char postdata[MAXSIZE];
    char timestamp[MINSIZE];
    int status;
    int sec;
    Getinfo *gi;

    gi = getb(Getinfo);

    if(NULL == gi)
    {
        Cdbg(API_DBG, "create dynamic memory fail\n");
        return NULL;
    }

    memset(gi,0,sizeof(Getinfo));

    sec = time((time_t *)NULL);

    snprintf(timestamp,MINSIZE,"%d",sec);
    snprintf(url,NORMALSIZE,"https://%s/member/getinfo/",server);
    snprintf(postdata,MAXSIZE,"<getinfo><token>%s</token><userid>%s</userid><time>2008/1/1</time></getinfo>"
            ,aaa.token,username);

    status = sendRequest(get_info_xml,url,postdata,NULL,NULL);

    if( status != 0 )
    {
        handle_error(status,"curl");
        my_free(gi);
        return NULL;
    }

    if( parseDoc1(get_info_xml,gi) == -1)
    {
        my_free(gi);
        return NULL;
    }
    
    return gi;
}

Getmysyncfolder *getMySyncFolder(char *username)
{
    char url[NORMALSIZE];
    char postdata[MAXSIZE];
    int status;
    Getmysyncfolder *gf;
    gf = getb(Getmysyncfolder);

    if(NULL == gf)
        return NULL;

    memset(gf,0,sizeof(Getmysyncfolder));

    snprintf(url,NORMALSIZE,"https://%s/folder/getmysyncfolder/",aaa.inforelay);

    Cdbg(API_DBG, "aaa.inforelay https://%s/folder/getmysyncfolder/\n",aaa.inforelay);

    snprintf(postdata,MAXSIZE,"<getmysyncfolder><token>%s</token><userid>%s</userid></getmysyncfolder>"
            ,aaa.token,username);

    Cdbg(API_DBG, "get_sync_folder_xml : %s\n",get_sync_folder_xml);


    status = sendRequest(get_sync_folder_xml,url,postdata,NULL,NULL);

    Cdbg(API_DBG, "get_sync_folder status : %d\n", status);

    if( status != 0 )
    {
        handle_error(status,"curl");
        my_free(gf);

        return NULL;
    }

    if( parseDoc1(get_sync_folder_xml,gf) == -1)
    {
        my_free(gf);
        return NULL;
    }

    
    return gf;
}

int obtainSyncRootID(char *user)
{
    int id = -1;
    Getmysyncfolder *gf = NULL;

    while(exit_loop != 1)
    {
        gf = getMySyncFolder(user);

        if(NULL == gf)
        {
            enter_sleep_time(1000*300,NULL);
            Cdbg(API_DBG, "getMySyncFolder gf : NULL\n");
            
            my_free(gf);

            return -1;
            //check_network_state();
            //Cdbg(API_DBG, "333333344444\n");
            //continue;
        }

        if(gf->status == S_AUTH_FAIL)
        {
            my_free(gf);
            if(obtainToken(user,password,&cfg,0) == -1)
                return -1;
            else
                continue;
        }

        if(gf->status != 0)
        {
            handle_error(gf->status,"GetMySyncFolderID Fail");
            my_free(gf);
            enter_sleep_time(1000*300,NULL);
            continue;
        }

        id = gf->id;
        my_free(gf);

        break;
    }

    Cdbg(API_DBG, "obtainSyncRootID : %d\n", id);
    return id;
}


// send mail
SendAwsEmail *sendAwsEmail(char *username, struct asuswebstorage_conf *asus_conf)
{

    char url[NORMALSIZE];
    char postdata[MAXSIZE];
    int status;
    char *header;
    char cookie[512];

    SendAwsEmail *sae;
    sae = getb(SendAwsEmail);

    if(NULL == sae) {
        Cdbg(API_DBG, "SendAwsEmail create dynamic memory fail\n");
        return NULL;
    }

    memset(sae,0,sizeof(SendAwsEmail));

    header = makeAuthorize();

        
    // Cdbg(API_DBG, "header : %s \n", header);

    //snprintf(url,NORMALSIZE,"https://%s/member/initattachabledevice/",aaa.inforelay);
    snprintf(url,NORMALSIZE,"https://portal00.asuswebstorage.com/util/sendawsemail/");

    char mailargs[256];;

    memset(mailargs,0,sizeof(mailargs));

    strcpy(mailargs, "?t=136&amp;l=en_US&amp;u=");
    //strcat(mailargs, "markcool.lucky%40gmail.com");
    strcat(mailargs, username);
    //strcat(mailargs, "&amp;c=f787a423d6524e7182bebdb71ef450032&amp;r=2");

    snprintf(dbg_msg, sizeof(dbg_msg), "sendAwsEmail -> mailargs : %s\n",mailargs);
    // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
    Cdbg(API_DBG, "%s", dbg_msg);

    //char * mailargs = "?t=108&amp;l=zh_TW&amp;u=markcool.lucky%40gmail.com";

    snprintf(postdata,MAXSIZE,"<sendawsemail><userid>%s</userid><emailtype>136</emailtype><mailargs>%s</mailargs><emailscenario>62</emailscenario></sendawsemail>", 
          username, mailargs);


    // snprintf(postdata,MAXSIZE,"<sendawsemail><userid>%s</userid><emailtype>136</emailtype><mailargs>%s</mailargs><emailscenario>85</emailscenario></sendawsemail>", 
    //      "markcool.lucky@gmail.com", mailargs);

    //snprintf(postdata,MAXSIZE,"<sendawsemail><emailtype>109</emailtype><mailargs>?t=109&amp;u=markcool_hu@asus.com&amp;l=zh_TW&amp;packageid=7</mailargs><emailscenario>110</emailscenario></sendawsemail>");
    //snprintf(postdata,MAXSIZE,"<sendawsemail><userid>%s</userid><emailtype>109</emailtype><mailargs>t=109&u=doris&l=zh_TW&packageid=7</mailargs><emailscenario>110</emailscenario></sendawsemail>");

    snprintf(cookie,512,"OMNISTORE_VER=1_0; path=/;sid=%s;",sid);


    snprintf(dbg_msg, sizeof(dbg_msg), "url = %s, cookie = %s",url, cookie);
    // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
    Cdbg(API_DBG, "%s", dbg_msg);

    snprintf(dbg_msg, sizeof(dbg_msg), "postdata = %s",postdata);
    // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
    Cdbg(API_DBG, "%s", dbg_msg);

    snprintf(dbg_msg, sizeof(dbg_msg), "get_send_aws_email_xml = %s",get_send_aws_email_xml);
    // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
    Cdbg(API_DBG, "%s", dbg_msg);

    // Cdbg(API_DBG, "url %s\n",url);
    // Cdbg(API_DBG, "cookie %s\n",cookie);
    // Cdbg(API_DBG, "postdata %s\n",postdata);
    // Cdbg(API_DBG, "get_send_aws_email_xml : %s\n",get_send_aws_email_xml);



    status = sendRequest(get_send_aws_email_xml,url,postdata,cookie,header);
    my_free(header);

    snprintf(dbg_msg, sizeof(dbg_msg), "status = %d",status);
    // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
    Cdbg(API_DBG, "%s", dbg_msg);

    if( status != 0 )
    {
        handle_error(status,"curl");

        // 28 -> connect timeout
        if(status == 28) {
            sae->status = CONNECT_TIMEOUT;
        } else if(status == 6) {
            // 6 -> This error mean that curl can't resolve host name
            sae->status = CURL_CANNOT_RESOLVE_HOSTNAME;
        } else if(status == 35) {
            // 35 -> CURLE_SSL_CONNECT_ERROR
            sae->status = CURLE_SSL_CONNECT_ERROR;
        } else {
            sae->status = status;
        }

        return sae;

        //my_free(sae);

        //return NULL;
    }

    if( parseDoc1(get_send_aws_email_xml,sae) == -1)
    {
        my_free(sae);
        return NULL;
    }

    
    return sae;
}


int initSendAwsEmail(char *user, struct asuswebstorage_conf *asus_conf)
{

    int id = -1;

    SendAwsEmail *sae = NULL;

    sae = sendAwsEmail(user, asus_conf);

    if(NULL == sae) {

        Cdbg(API_DBG, "sendAwsEmail is null\n");

        my_free(sae);

        return -1;
    }


    Cdbg(API_DBG, "sae->status = %d", sae->status);


    if(sae->status != 0) {

        handle_error(sae->status,"initSendAwsEmail Fail");

        int status = sae->status;

        Cdbg(API_DBG, "initSendAwsEmail : sae->status find status is %d\n", status);
        if(status == CONNECT_TIMEOUT) {
            Cdbg(API_DBG, "initSendAwsEmail : connect time out\n");
        } else if(status == CURL_CANNOT_RESOLVE_HOSTNAME) {
            Cdbg(API_DBG, "initSendAwsEmail : curl can't resolve host name\n");
        } else if(status == CURLE_SSL_CONNECT_ERROR) {
            Cdbg(API_DBG, "initSendAwsEmail : CURLE_SSL_CONNECT_ERROR\n");
        }
        my_free(sae);

        return status;
    }

    id = sae->id;

    my_free(sae);

    return id;
}

void removeChar(char *str, char garbage) {

    char *src, *dst;
    for (src = dst = str; *src != '\0'; src++) {
        *dst = *src;
        if (*dst != garbage) dst++;
    }
    *dst = '\0';
}

void substr(char *dest, const char* src, unsigned int start, unsigned int cnt) {
  strncpy(dest, src + start, cnt);
  dest[cnt] = 0;
}

// old asuswebstorage setting
int initAsusWebstorageCfg(struct asuswebstorage_conf *asus_conf, 
    struct aaews_provision_conf * ap_conf, struct deploy_conf * dp_conf, 
    struct sysinfo_conf * si_conf, struct basic_command_conf * bc_conf,
    struct system_conf * s_conf)
{

    // setting init value
    strcpy(asus_conf->servicegateway, "cloudsyncportal01.asuswebstorage.com");


    strcpy(asus_conf->serviceid, sid);
    strcpy(asus_conf->key, progKey);

    strcpy(asus_conf->sid, sid);
    strcpy(asus_conf->progKey, progKey);


    strcpy(asus_conf->username, ap_conf->account);

    Cdbg(API_DBG, "JSON asus_conf->password : %s\n", asus_conf->password);
    Cdbg(API_DBG, "JSON ap_conf->password : %s\n", ap_conf->password);

    strcpy(asus_conf->password, ap_conf->password);
    strcpy(asus_conf->uploader_path, "/tmp/diag_db_cloud/");
    strcpy(asus_conf->device_id, ap_conf->token);


    char* mac = malloc(strlen(s_conf->macaddr)+1);
    strcpy(mac, s_conf->macaddr);
    removeChar(mac, ':');



    strcpy(asus_conf->device_id, "AiCAM_");
    strcat(asus_conf->device_id, mac);

    strcpy(asus_conf->merchandise_type, "AiCAM2");
    strcpy(asus_conf->client_set, "AiCAM_Camera");
    strcpy(asus_conf->device_manager_host, "ASUS Device Manager Host");
    strcpy(asus_conf->client_type, "AiCAM");
    strcpy(asus_conf->client_version, "1.0.1.120");
    strcpy(asus_conf->manufacturer, "ASUSTeK Computer INC.");
    strcpy(asus_conf->product_name, AICAM_PRODUCT_NAME);
    // strcpy(asus_conf->product_name, "AiCam");
    
    strcpy(asus_conf->machine_id, asus_conf->device_id);

    strcpy(asus_conf->uuid, mac);
    strcpy(asus_conf->mac, mac);

    char macSix[7];
    memset(macSix,0,sizeof(macSix));
    strncpy(macSix, mac + 6,  6);

    strcpy(asus_conf->mac_name, "AiCAM_");
    strcat(asus_conf->mac_name, mac);
    //strcat(asus_conf->mac_name, macSix);  // 2017/06/22 


    strcpy(asus_conf->device_name, asus_conf->mac_name);
    //strcpy(asus_conf->device_name, bc_conf->deviceName);


    free(mac);


    Cdbg(API_DBG, "JSON File parse, asus_conf->servicegateway : %s", asus_conf->servicegateway);
    Cdbg(API_DBG, "JSON File parse, asus_conf->serviceid : %s", asus_conf->serviceid);
    Cdbg(API_DBG, "JSON File parse, asus_conf->key : %s", asus_conf->key);
    Cdbg(API_DBG, "JSON File parse, asus_conf->username : %s", asus_conf->username);
    Cdbg(API_DBG, "JSON File parse, asus_conf->password : %s", asus_conf->password);
    Cdbg(API_DBG, "JSON File parse, asus_conf->sid : %s", asus_conf->sid);
    Cdbg(API_DBG, "JSON File parse, asus_conf->progKey : %s", asus_conf->progKey);
    Cdbg(API_DBG, "JSON File parse, asus_conf->uploader_path : %s", asus_conf->uploader_path);
    Cdbg(API_DBG, "JSON File parse, asus_conf->device_id : %s", asus_conf->device_id);
    Cdbg(API_DBG, "JSON File parse, asus_conf->device_name : %s", asus_conf->device_name);
    Cdbg(API_DBG, "JSON File parse, asus_conf->merchandise_type : %s", asus_conf->merchandise_type);
    Cdbg(API_DBG, "JSON File parse, asus_conf->client_set : %s", asus_conf->client_set);
    Cdbg(API_DBG, "JSON File parse, asus_conf->device_manager_host : %s", asus_conf->device_manager_host);
    Cdbg(API_DBG, "JSON File parse, asus_conf->client_type : %s", asus_conf->client_type);
    Cdbg(API_DBG, "JSON File parse, asus_conf->client_version : %s", asus_conf->client_version);
    Cdbg(API_DBG, "JSON File parse, asus_conf->manufacturer : %s", asus_conf->manufacturer);
    Cdbg(API_DBG, "JSON File parse, asus_conf->product_name : %s", asus_conf->product_name);
    Cdbg(API_DBG, "JSON File parse, asus_conf->machine_id : %s", asus_conf->machine_id);
    Cdbg(API_DBG, "JSON File parse, asus_conf->uuid : %s", asus_conf->uuid);
    Cdbg(API_DBG, "JSON File parse, asus_conf->mac : %s", asus_conf->mac);
    Cdbg(API_DBG, "JSON File parse, asus_conf->mac_name : %s", asus_conf->mac_name);

    return 0;
}


// old asuswebstorage setting
int initAsusWebstorageConfig(struct asuswebstorage_conf *asus_conf)
{

    // setting -> sid, progKey
    strcpy(asus_conf->sid, sid);
    strcpy(asus_conf->progKey, progKey);


    process_asuswebstorage_config(asus_conf);

    Cdbg(API_DBG, "JSON File parse, asus_conf->servicegateway : %s", asus_conf->servicegateway);
    Cdbg(API_DBG, "JSON File parse, asus_conf->serviceid : %s", asus_conf->serviceid);
    Cdbg(API_DBG, "JSON File parse, asus_conf->key : %s", asus_conf->key);
    Cdbg(API_DBG, "JSON File parse, asus_conf->username : %s", asus_conf->username);
    Cdbg(API_DBG, "JSON File parse, asus_conf->password : %s", asus_conf->password);
    Cdbg(API_DBG, "JSON File parse, asus_conf->sid : %s", asus_conf->sid);
    Cdbg(API_DBG, "JSON File parse, asus_conf->progKey : %s", asus_conf->progKey);
    Cdbg(API_DBG, "JSON File parse, asus_conf->uploader_path : %s", asus_conf->uploader_path);
    Cdbg(API_DBG, "JSON File parse, asus_conf->device_id : %s", asus_conf->device_id);
    Cdbg(API_DBG, "JSON File parse, asus_conf->device_name : %s", asus_conf->device_name);
    Cdbg(API_DBG, "JSON File parse, asus_conf->merchandise_type : %s", asus_conf->merchandise_type);
    Cdbg(API_DBG, "JSON File parse, asus_conf->device_manager_host : %s", asus_conf->device_manager_host);
    Cdbg(API_DBG, "JSON File parse, asus_conf->client_type : %s", asus_conf->client_type);
    Cdbg(API_DBG, "JSON File parse, asus_conf->client_version : %s", asus_conf->client_version);
    Cdbg(API_DBG, "JSON File parse, asus_conf->manufacturer : %s", asus_conf->manufacturer);
    Cdbg(API_DBG, "JSON File parse, asus_conf->product_name : %s", asus_conf->product_name);
    Cdbg(API_DBG, "JSON File parse, asus_conf->machine_id : %s", asus_conf->machine_id);
    Cdbg(API_DBG, "JSON File parse, asus_conf->uuid : %s", asus_conf->uuid);
    Cdbg(API_DBG, "JSON File parse, asus_conf->mac : %s", asus_conf->mac);
    Cdbg(API_DBG, "JSON File parse, asus_conf->mac_name : %s", asus_conf->mac_name);



    return 0;
}


int initGoogleToken(struct google_conf * google_d_conf)
{
    process_google_config(google_d_conf);

    Cdbg(API_DBG, "Google config file parse, access_token : %s", google_d_conf->access_token);
    Cdbg(API_DBG, "Google config file parse, refresh_token : %s", google_d_conf->refresh_token);

    return 0;
}


int initAiCloudConfig(struct aicloud_conf *ai_conf)
{
    process_aicloud_config(ai_conf);

    Cdbg(API_DBG, "JSON File parse, ai_conf->username : %s", ai_conf->username);
    Cdbg(API_DBG, "JSON File parse, ai_conf->password : %s", ai_conf->password);
    Cdbg(API_DBG, "JSON File parse, ai_conf->url : %s", ai_conf->url);
    Cdbg(API_DBG, "JSON File parse, ai_conf->privateip : %s", ai_conf->privateip);
    Cdbg(API_DBG, "JSON File parse, ai_conf->path : %s", ai_conf->path);
    Cdbg(API_DBG, "JSON File parse, ai_conf->quota : %s", ai_conf->quota);
    Cdbg(API_DBG, "JSON File parse, ai_conf->uploader_path : %s", ai_conf->uploader_path);


    return 0;
}


int initStorageProvisionConfig(struct storage_provision_conf * sp_conf, char * file_path)
{
    process_storage_provision_config(sp_conf, file_path);

    Cdbg(API_DBG, "storage provision config parse   ----------");
    Cdbg(API_DBG, "JSON File parse, sp_conf->timeStamp : %s", sp_conf->timeStamp);
    Cdbg(API_DBG, "JSON File parse, sp_conf->alarmAccount : %s", sp_conf->alarmAccount);
    Cdbg(API_DBG, "JSON File parse, sp_conf->alarmToken : %s", sp_conf->alarmToken);
    Cdbg(API_DBG, "JSON File parse, sp_conf->alarmStorageType : %s", sp_conf->alarmStorageType);
    Cdbg(API_DBG, "JSON File parse, sp_conf->alarmStorageQuota : %s", sp_conf->alarmStorageQuota);
    Cdbg(API_DBG, "JSON File parse, sp_conf->alarmAppNotification : %s", sp_conf->alarmAppNotification);
    Cdbg(API_DBG, "JSON File parse, sp_conf->alarmEmailNotificaiton : %s", sp_conf->alarmEmailNotificaiton);
    Cdbg(API_DBG, "JSON File parse, sp_conf->recordAccount : %s", sp_conf->recordAccount);
    Cdbg(API_DBG, "JSON File parse, sp_conf->recordToken : %s", sp_conf->recordToken);
    Cdbg(API_DBG, "JSON File parse, sp_conf->recordStorageType : %s", sp_conf->recordStorageType);
    Cdbg(API_DBG, "JSON File parse, sp_conf->recordStorageURL : %s", sp_conf->recordStorageURL);
    Cdbg(API_DBG, "JSON File parse, sp_conf->recordStorageQuota : %s", sp_conf->recordStorageQuota);
    Cdbg(API_DBG, "JSON File parse, sp_conf->recordStoragePath : %s", sp_conf->recordStoragePath);
    Cdbg(API_DBG, "JSON File parse, sp_conf->recordStoragePrivateIP : %s", sp_conf->recordStoragePrivateIP);


    return 0;
}


int initAaewsProvisionConfig(struct aaews_provision_conf * ap_conf)
{
    process_aaews_provision_config(ap_conf);

    Cdbg(API_DBG, "aaews provision config parse   ----------");
    Cdbg(API_DBG, "JSON File parse, ap_conf->timeStamp : %s", ap_conf->timeStamp);
    Cdbg(API_DBG, "JSON File parse, ap_conf->account : %s", ap_conf->account);
    Cdbg(API_DBG, "JSON File parse, ap_conf->password : %s", ap_conf->password);
    Cdbg(API_DBG, "JSON File parse, ap_conf->token : %s", ap_conf->token);
    Cdbg(API_DBG, "JSON File parse, ap_conf->tokenExpireTime : %s", ap_conf->tokenExpireTime);
    Cdbg(API_DBG, "JSON File parse, ap_conf->refreshToken : %s", ap_conf->refreshToken);
    Cdbg(API_DBG, "JSON File parse, ap_conf->deviceName : %s", ap_conf->deviceName);
    Cdbg(API_DBG, "JSON File parse, ap_conf->timeZone : %s", ap_conf->timeZone);


    return 0;
}



int initSysinfoConfig(struct sysinfo_conf * si_conf)
{
    process_sysinfo_config(si_conf);

    Cdbg(API_DBG, "sysinfo config parse   ----------");
    Cdbg(API_DBG, "JSON File parse, si_conf->deviceid : %s", si_conf->deviceid);
    //Cdbg(API_DBG, "JSON File parse, si_conf->macaddr : %s", si_conf->macaddr);

    return 0;
}


int initDeployConfig(struct deploy_conf * dp_conf)
{
    process_deploy_config(dp_conf);

    Cdbg(API_DBG, "deploy config parse   ----------");
    Cdbg(API_DBG, "JSON File parse, dp_conf->timeStamp : %s", dp_conf->timeStamp);
    Cdbg(API_DBG, "JSON File parse, dp_conf->aaeAppId : %s", dp_conf->aaeAppId);
    Cdbg(API_DBG, "JSON File parse, dp_conf->aaeAppKey : %s", dp_conf->aaeAppKey);
    Cdbg(API_DBG, "JSON File parse, dp_conf->aaeAppPortal : %s", dp_conf->aaeAppPortal);
    Cdbg(API_DBG, "JSON File parse, dp_conf->aaeSid : %s", dp_conf->aaeSid);
    Cdbg(API_DBG, "JSON File parse, dp_conf->aaeOAuthPortal : %s", dp_conf->aaeOAuthPortal);
    Cdbg(API_DBG, "JSON File parse, dp_conf->awsAppId : %s", dp_conf->awsAppId);
    Cdbg(API_DBG, "JSON File parse, dp_conf->awsAppKey : %s", dp_conf->awsAppKey);
    Cdbg(API_DBG, "JSON File parse, dp_conf->awsAppGateway : %s", dp_conf->awsAppGateway);
    Cdbg(API_DBG, "JSON File parse, dp_conf->awsAppPortal : %s", dp_conf->awsAppPortal);
    Cdbg(API_DBG, "JSON File parse, dp_conf->awsOAuthPortal : %s", dp_conf->awsOAuthPortal);
    Cdbg(API_DBG, "JSON File parse, dp_conf->otaInfoUrl : %s", dp_conf->otaInfoUrl);
    Cdbg(API_DBG, "JSON File parse, dp_conf->otaFileUrl : %s", dp_conf->otaFileUrl);
    Cdbg(API_DBG, "JSON File parse, dp_conf->fbRtmpUrl : %s", dp_conf->fbRtmpUrl);

    return 0;
}



int initBasicCommandConfig(struct basic_command_conf * bc_conf)
{
    process_basic_command_config(bc_conf);

    Cdbg(API_DBG, "deploy config parse   ----------");
    Cdbg(API_DBG, "JSON File parse, bc_conf->timeStamp : %s", bc_conf->timeStamp);
    Cdbg(API_DBG, "JSON File parse, bc_conf->cameraOnOff : %s", bc_conf->cameraOnOff);
    Cdbg(API_DBG, "JSON File parse, bc_conf->cameraLedOnOff : %s", bc_conf->cameraLedOnOff);
    Cdbg(API_DBG, "JSON File parse, bc_conf->deviceName : %s", bc_conf->deviceName);
    Cdbg(API_DBG, "JSON File parse, bc_conf->speakerVolume : %s", bc_conf->speakerVolume);
    Cdbg(API_DBG, "JSON File parse, bc_conf->motionAlarmOnOff : %s", bc_conf->motionAlarmOnOff);
    Cdbg(API_DBG, "JSON File parse, bc_conf->audioAlarmOnOff : %s", bc_conf->audioAlarmOnOff);
    Cdbg(API_DBG, "JSON File parse, bc_conf->recordOnOff : %s", bc_conf->recordOnOff);
    Cdbg(API_DBG, "JSON File parse, bc_conf->wdrOnOff : %s", bc_conf->wdrOnOff);
    Cdbg(API_DBG, "JSON File parse, bc_conf->ldcOnOff : %s", bc_conf->ldcOnOff);


    return 0;
}




int initSystemConfig(struct system_conf * s_conf, char * file_path)
{
    process_system_config(s_conf, file_path);

    Cdbg(API_DBG, "system config parse   ----------");
    Cdbg(API_DBG, "JSON File parse, s_conf->fwver : %s", s_conf->fwver);
    Cdbg(API_DBG, "JSON File parse, s_conf->tcode : %s", s_conf->tcode);
    Cdbg(API_DBG, "JSON File parse, s_conf->sn : %s", s_conf->sn);
    Cdbg(API_DBG, "JSON File parse, s_conf->recordplan : %s", s_conf->recordplan);
    Cdbg(API_DBG, "JSON File parse, s_conf->privateip : %s", s_conf->privateip);
    Cdbg(API_DBG, "JSON File parse, s_conf->macaddr : %s", s_conf->macaddr);
    Cdbg(API_DBG, "JSON File parse, s_conf->videomode : %s", s_conf->videomode);
    Cdbg(API_DBG, "JSON File parse, s_conf->initattachabledevice : %s", s_conf->initattachabledevice);


    return 0;
}




GetInitAttachableDevice *getInitAttachableDevice(char *username, char *base64EncodeOutput, struct asuswebstorage_conf *asus_conf)
{
    char url[NORMALSIZE];
    char postdata[MAXSIZE];
    int status;
    char *header;
    char cookie[512];

    GetInitAttachableDevice *iad;
    iad = getb(GetInitAttachableDevice);

    if(NULL == iad)
        return NULL;

    memset(iad,0,sizeof(GetInitAttachableDevice));

    header = makeAuthorize();
        
    //Cdbg(API_DBG, "aaa.inforelay https://%s/member/initattachabledevice/\n",aaa.inforelay);

    //snprintf(url,NORMALSIZE,"https://%s/member/initattachabledevice/",aaa.inforelay);
    snprintf(url,NORMALSIZE,"https://cloudportal01.asuswebstorage.com/member/initattachabledevice/");

    Cdbg(API_DBG, "%s\n", url);

    snprintf(postdata,MAXSIZE,"<initattachabledevice><userid>%s</userid><passwd>%s</passwd><deviceid>%s</deviceid><devicename>%s</devicename><merchandisetype>%s</merchandisetype><version>2</version><clientset>%s</clientset><devicemanagerhost>%s</devicemanagerhost><provision>%s</provision></initattachabledevice>", username, password, asus_conf->device_id, asus_conf->device_name, asus_conf->merchandise_type, asus_conf->client_set, asus_conf->device_manager_host, base64EncodeOutput);

    Cdbg(API_DBG, "header : %s\n",header);

    Cdbg(API_DBG, "postdata : %s\n",postdata);
                        
    Cdbg(API_DBG, "get_init_attachable_device_xml : %s\n",get_init_attachable_device_xml);

    snprintf(cookie,512,"OMNISTORE_VER=1_0; path=/;sid=%s;",sid);


    Cdbg(API_DBG, "cookie : %s\n",cookie);

    status = sendRequest(get_init_attachable_device_xml,url,postdata,cookie,header);
    my_free(header);

    Cdbg(API_DBG, "getInitAttachableDevice sendRequest->status  : %d\n", status );


    if( status != 0 )
    {
        handle_error(status,"curl");

        my_free(iad);
        return NULL;
    }


    if( parseDoc1(get_init_attachable_device_xml,iad) == -1)
    {
        my_free(iad);
        return NULL;
    }

    
    return iad;
}


int initAttachableDevice(char *user, char *base64EncodeOutput, struct asuswebstorage_conf *asus_conf)
{
    int status = -1;

    GetInitAttachableDevice *iad = NULL;

    //while(exit_loop != 1)
    while(1)
    {
        iad = getInitAttachableDevice(user, base64EncodeOutput, asus_conf);

        if(NULL == iad)
        {
            Cdbg(API_DBG, "initAttachableDevice getInitAttachableDevice -> NULL \n");

            sleep(30);
            //enter_sleep_time(1000*300,NULL);
            //check_network_state();

            continue;
        }


        if(iad->status == DEVICE_AUTH_FAIL)
        {
            my_free(iad);

            Cdbg(API_DBG, "initAttachableDevice AUTH_FAIL status -> %d\n", DEVICE_AUTH_FAIL);

            return DEVICE_AUTH_FAIL;
        }

        // 0 -> return status success
        if(iad->status != 0)
        {
            handle_error(iad->status,"GetInitAttachableDevice Fail");
            //my_free(iad);
            //sleep(30);
            //continue;
        }

        status = iad->status;
        my_free(iad);

        break;
    }

    return status;
}


writeSchemeFile(char * scheme) {

    char scheme_file_path[50];

    memset(scheme_file_path,0, sizeof(scheme_file_path));
    strcpy(scheme_file_path, "/tmp/diag_db_cloud/scheme");

    FILE *pFile;

    pFile = fopen(scheme_file_path, "w" );

    if( NULL == pFile ){

        printf( "writeAlarmFile -> open failure" );

    }else{

        printf( "writeAlarmFile -> open success\n" );
        //fwrite(buffer,1,sizeof(buffer),pFile);
        fprintf(pFile, "%s",scheme);
    }

    fclose(pFile);

}



BrowseAttachableDevice *getBrowseAttachableDevice(char *username, struct aaews_provision_conf * ap_conf)
{
    char url[NORMALSIZE];
    char postdata[MAXSIZE];
    int status;
    char *header;
    char cookie[512];

    BrowseAttachableDevice *bad;
    bad = getb(BrowseAttachableDevice);

    if(NULL == bad)
        return NULL;

    memset(bad,0,sizeof(BrowseAttachableDevice));

    header = makeAuthorize();
        

    snprintf(url,NORMALSIZE,"https://cloudportal01.asuswebstorage.com/member/browseattachabledevice/");

    Cdbg(API_DBG, "%s\n", url);

    //snprintf(postdata,MAXSIZE,"<browseattachabledevice><userid>%s</userid><passwd>%s</passwd></browseattachabledevice>", username, password);
    // snprintf(postdata,MAXSIZE,"<browseattachabledevice><userid>%s</userid><oauthaccesstoken>%s</oauthaccesstoken></browseattachabledevice>", username, aaa.token);
    if(oauth_token) {
        snprintf(postdata,MAXSIZE,"<browseattachabledevice><userid>%s</userid><oauthaccesstoken>%s</oauthaccesstoken></browseattachabledevice>", username, ap_conf->token);    
    } else {
        snprintf(postdata,MAXSIZE,"<browseattachabledevice><userid>%s</userid><passwd>%s</passwd></browseattachabledevice>", username, password);    
    }
    

    Cdbg(API_DBG, "header : %s\n",header);

    Cdbg(API_DBG, "postdata : %s\n",postdata);
                        
    Cdbg(API_DBG, "get_browse_attachable_device_xml : %s\n",get_browse_attachable_device_xml);

    snprintf(cookie,512,"OMNISTORE_VER=1_0; path=/;sid=%s;",sid);


    Cdbg(API_DBG, "cookie : %s\n",cookie);

    status = sendRequest(get_browse_attachable_device_xml,url,postdata,cookie,header);
    my_free(header);

    Cdbg(API_DBG, "browseAttachableDevice sendRequest->status  : %d\n", status );


    if( status != 0 )
    {
        handle_error(status,"curl");

        my_free(bad);
        return NULL;
    }


    if( parseDoc1(get_browse_attachable_device_xml,bad) == -1)
    {
        my_free(bad);
        return NULL;
    }

    
    return bad;
}



int browseAttachableDevice(char *user, struct aaews_provision_conf * ap_conf, int return_type)
{

    int status = -1;

    BrowseAttachableDevice *bad = NULL;

    //while(exit_loop != 1)
    while(1)
    {
        bad = getBrowseAttachableDevice(user, ap_conf);

        if(NULL == bad)
        {
            Cdbg(API_DBG, "browseAttachableDevice -> NULL \n");

            sleep(30);
            //enter_sleep_time(1000*300,NULL);
            //check_network_state();

            continue;
        }


        if(bad->status == DEVICE_AUTH_FAIL)
        {
            my_free(bad);

            Cdbg(API_DBG, "browseAttachableDevice AUTH_FAIL status -> %d\n", DEVICE_AUTH_FAIL);

            return DEVICE_AUTH_FAIL;
        }

        // 0 -> return status success
        if(bad->status != 0)
        {
            handle_error(bad->status,"BrowseAttachableDevice Fail");
            //my_free(bad);
            //sleep(30);
            //continue;
        }

       Cdbg(API_DBG, "bad->scheme -> %d", bad->scheme);


       // 1: webstorage space plan (alarm plan)
       if(bad->scheme == 1)
        {
            space_plan = SPACE_SIZE_PLAN;
            // handle_error(bad->status,"BrowseAttachableDevice Fail");
            //my_free(bad);
            //sleep(30);
            //continue;

            writeSchemeFile("1");

        } else {

            space_plan = TIME_CONTINOUS_PLAN;
            
            writeSchemeFile("");
        }

        status = bad->status;

        // state 
        if(bad->state == 2)
        {
           status = DEVICE_EXPIRED; 

        }


        my_free(bad);

        break;
    }



    if(return_type == 0) {

        return status;

    } else if(return_type == 1) {

        if(space_plan != space_plan_tmp) {
            return SPACE_PLAN_DIFF;
        } else {
            return status;
        }
    }
    
}




BrowseAttachableDevice *getBrowseAttachableDeviceList(char *username, struct aaews_provision_conf * ap_conf)
{
    char url[NORMALSIZE];
    char postdata[MAXSIZE];
    int status;
    char *header;
    char cookie[512];

    BrowseAttachableDevice *bad;
    bad = getb(BrowseAttachableDevice);

    if(NULL == bad)
        return NULL;

    memset(bad,0,sizeof(BrowseAttachableDevice));

    header = makeAuthorize();
        

    snprintf(url,NORMALSIZE,"https://cloudportal01.asuswebstorage.com/member/browseattachabledevice/");

    Cdbg(API_DBG, "%s\n", url);

    //snprintf(postdata,MAXSIZE,"<browseattachabledevice><userid>%s</userid><passwd>%s</passwd></browseattachabledevice>", username, password);
    // snprintf(postdata,MAXSIZE,"<browseattachabledevice><userid>%s</userid><oauthaccesstoken>%s</oauthaccesstoken></browseattachabledevice>", username, aaa.token);
    if(oauth_token) {
        snprintf(postdata,MAXSIZE,"<browseattachabledevice><userid>%s</userid><oauthaccesstoken>%s</oauthaccesstoken></browseattachabledevice>", username, ap_conf->token);    
    } else {
        snprintf(postdata,MAXSIZE,"<browseattachabledevice><userid>%s</userid><passwd>%s</passwd></browseattachabledevice>", username, password);    
    }
    

    Cdbg(API_DBG, "header : %s\n",header);

    Cdbg(API_DBG, "postdata : %s\n",postdata);
                        
    Cdbg(API_DBG, "get_browse_attachable_device_xml : %s\n",get_browse_attachable_device_xml);

    snprintf(cookie,512,"OMNISTORE_VER=1_0; path=/;sid=%s;",sid);


    Cdbg(API_DBG, "cookie : %s\n",cookie);

    status = sendRequest(get_browse_attachable_device_xml,url,postdata,cookie,header);
    my_free(header);

    Cdbg(API_DBG, "browseAttachableDevice sendRequest->status  : %d\n", status );


    if( status != 0 )
    {
        handle_error(status,"curl");

        my_free(bad);
        return NULL;
    }


    if( parseBrowseAttachableDeviceXml(get_browse_attachable_device_xml,bad) == -1)
    {
        my_free(bad);
        return NULL;
    }

    
    return bad;
}



int browseAttachableDeviceList(char *user, struct aaews_provision_conf * ap_conf)
{

    int status = -1;

    BrowseAttachableDevice *bad = NULL;

    //while(exit_loop != 1)
    while(1)
    {
        bad = getBrowseAttachableDeviceList(user, ap_conf);

        if(NULL == bad)
        {
            Cdbg(API_DBG, "browseAttachableDevice -> NULL \n");

            sleep(30);
            //enter_sleep_time(1000*300,NULL);
            //check_network_state();

            continue;
        }


        if(bad->status == DEVICE_AUTH_FAIL)
        {
            my_free(bad);

            Cdbg(API_DBG, "browseAttachableDevice AUTH_FAIL status -> %d, uploader end\n", DEVICE_AUTH_FAIL);
            exit(0);

            return DEVICE_AUTH_FAIL;
        }

        // 0 -> return status success
        if(bad->status != 0)
        {
            handle_error(bad->status,"BrowseAttachableDevice Fail");
            //my_free(bad);
            //sleep(30);
            //continue;
        }

        status = bad->status;

        // state 
        if(bad->state == 2)
        {
            status = DEVICE_EXPIRED; 

            Cdbg(API_DBG, "device_id : [ %s ] EXPIRED", asus_conf.device_id);
            Cdbg(API_DBG, "uploader end");

            exit(0);

        }


        my_free(bad);

        break;
    }


    return status;

}




// end : modify by markcool

Getpersonalsystemfolder *getPersonalSystemFolder(char *username,char *filename)
{
    char url[NORMALSIZE];
    char postdata[MAXSIZE];
    int status;
    Getpersonalsystemfolder *gp;
    gp = getb(Getpersonalsystemfolder);

    if(NULL == gp)
        return NULL;

    memset(gp,0,sizeof(Getpersonalsystemfolder));

    snprintf(url,NORMALSIZE,"https://%s/folder/getpersonalsystemfolder/",aaa.inforelay);
    snprintf(postdata,MAXSIZE,"<getpersonalsystemfolder><token>%s</token><userid>%s</userid><rawfoldername>%s</rawfoldername></getpersonalsystemfolder>"
            ,aaa.token,username,filename);

    status = sendRequest(get_personal_system_folder_xml,url,postdata,NULL,NULL);

    if( status != 0 )
    {
        handle_error(status,"curl");
        my_free(gp);
        return NULL;
    }

    if( parseDoc1(get_personal_system_folder_xml,gp) == -1)
    {
        my_free(gp);
        return NULL;
    }

    return gp;

}

int GetMyRecycleID(char *username,char *filename)
{
    Getpersonalsystemfolder *gp;
    int id = -10;

    while(!exit_loop)
    {
        gp = getPersonalSystemFolder(username,filename);

        if(NULL == gp)
        {
            enter_sleep_time(1000*300,NULL);
            check_network_state();
            continue;
        }

        if(gp->status == S_AUTH_FAIL)
        {
            my_free(gp);
            if(obtainToken(username,password,&cfg,0) == -1)
                return -1;
            else
                continue;
        }

        if(gp->status != 0)
        {
            handle_error(gp->status,"getpersonalsystemfolder");
            my_free(gp);
            enter_sleep_time(1000*300,NULL);
            continue;
        }

        id = gp->folderid;
        my_free(gp);

        break;
    }

    return id;
}

Getuserstate *getUserState(char *user,char *server)
{
    Getuserstate *gu;
    int status;
    char url[256] = {0};
    char postdata[256] = {0};
    char cookie[32] = {0};

    gu = getb(Getuserstate);

    if(NULL == gu)
    {
        Cdbg(API_DBG, "create dynamic memory fail\n");
        return NULL;
    }

    memset(gu,0,sizeof(Getuserstate));

    snprintf(cookie,32,"OMNISTORE_VER=1_0; path=/;");
    snprintf(url,256,"https://%s/member/getuserstate/",server);
    snprintf(postdata,256,"<getuserstate><userid>%s</userid><serviceid>1</serviceid></getuserstate>",user);

    status = sendRequest(get_user_state_xml,url,postdata,cookie,NULL);

    if( status != 0 )
    {
        handle_error(status,"curl");
        my_free(gu);
        return NULL;
    }

    if( parseDoc1(get_user_state_xml,gu) == -1)
    {
        my_free(gu);
        return NULL;
    }

    return gu;
}

int CheckUserState(char *user,char *server)
{
    int status;
    Getuserstate *gu;
    int result = 0;

    while(!exit_loop)
    {
      gu = getUserState(user,server);
      if(gu == NULL)
      {
          //usleep(1000*500);
          enter_sleep_time(1000*500,NULL);
          check_network_state();
          continue;
          //break;
      }

      if(gu->status == S_AUTH_FAIL)
      {
          my_free(gu);
          if(obtainToken(user,password,&cfg,0) == -1)
              return -1;
          else
              continue;
      }

      if(gu->status != 0)
      {
          status = gu->status;
          handle_error(status,"getUserState");
          my_free(gu);
          enter_sleep_time(1000*500,NULL);
          continue;
      }

#ifdef DEBUG
      Cdbg(API_DBG, "userstate=%d\n",gu->userstate);
#endif

      if(gu->userstate & FROZEN)   //accout state is FROZEN;
      {
          write_log(S_ERROR,"Your accout is frozen,you can't upload file","");
          result = S_ACCOUNT_FROZEN;
      }
      else if(gu->userstate & CLOSE)
      {
          write_log(S_ERROR,"Your accout is close,please active it","");
          result = S_ACCOUNT_CLOSE;
      }
      my_free(gu);

      break;
    }

    return result;

}

#if 0
/* browse file list by page andt merge list*/
Browse *linkBrowseList(Browse *all,Browse *item)
{
    Browse *temp = NULL;
    File **f1;
    Folder **fd1;
    int i;
    int index,t;

    if(all == NULL || item == NULL)
    {
        Cdbg(API_DBG, "pass point is null\n");
        return NULL;
    }

    temp = (Browse *)calloc(1,sizeof(Browse));
    if(temp == NULL)
    {
        Cdbg(API_DBG, "realloc browse fail\n");
        return NULL;
    }

    //if(item->filenumber > 0)
    {
        //        f1 = all->filelist;
        //        f2 = (File **)realloc(f1,sizeof(File *)*item->filenumber);
        //        if(f2 == NULL)
        //        {
        //            Cdbg(API_DBG, "realloc browse fail\n");
        //            return NULL;
        //        }
        //Cdbg(API_DBG, "new_file_num=%d\n",item->filenumber);
        t = all->filenumber + item->filenumber;
        if(t>0)
        {
            f1 = (File **)calloc(t,sizeof(File *));
            if(f1 == NULL)
            {
                Cdbg(API_DBG, "realloc browse fail\n");
                my_free(temp);
                return NULL;
            }
            //Cdbg(API_DBG, "full all start\n");
            for(i=0;i<all->filenumber;i++)
            {
                f1[i] = all->filelist[i];
            }
            //Cdbg(API_DBG, "full all end\n");
            index = all->filenumber;
            //Cdbg(API_DBG, "full item start\n");
            for(i=0;i<item->filenumber;i++,index++)
            {
                f1[index] = item->filelist[i];
            }
            //Cdbg(API_DBG, "full item end\n");
            temp->filelist = f1;
            temp->filenumber = t;
        }

        //Cdbg(API_DBG, "marge file end\n");
    }

    //if(item->foldernumber>0)
    {
        //        fd1 = all->folderlist;
        //        fd2 = (Folder **)realloc(fd1,sizeof(Folder *)*item->foldernumber);
        //        if(fd2 == NULL)
        //        {
        //            Cdbg(API_DBG, "realloc browse fail\n");
        //            if(item->filenumber)
        //            {
        //                f1 = temp->filelist + item->filenumber;
        //                my_free(f1);
        //            }
        //            return NULL;
        //        }
        //Cdbg(API_DBG, "new_folder_num=%d\n",item->foldernumber);
        t = all->foldernumber + item->foldernumber;
        if(t>0)
        {
            fd1 = (Folder **)calloc(t,sizeof(Folder *));
            if(fd1 == NULL)
            {
                Cdbg(API_DBG, "realloc browse fail\n");
                my_free(temp->filelist);
                my_free(temp);
                return NULL;
            }
            //Cdbg(API_DBG, "full all start\n");
            for(i=0;i<all->foldernumber;i++)
            {
                fd1[i] = all->folderlist[i];
            }
            //Cdbg(API_DBG, "full all end\n");
            index = all->foldernumber;
            //Cdbg(API_DBG, "full item start\n");
            for(i=0;i<item->foldernumber;i++,index++)
            {
                fd1[index] = item->folderlist[i];
            }
            //Cdbg(API_DBG, "full item end\n");
            temp->folderlist = fd1;
            temp->foldernumber = t;
        }


    }

    return temp;
}

Browse *GetServerList(char *username,int id,int issibiling)
{
    int pageno = 1 ;
    Browse *cur = NULL,*item = NULL,*temp;
    int haspage = 0;

    while(1)
    {
        //Cdbg(API_DBG, "11\n");
        item = browseFolder(username,id,issibiling,pageno);
        //Cdbg(API_DBG, "22\n");

        if(item == NULL)
        {
            check_network_state();
            enter_sleep_time(1000*300,NULL);
            continue;
        }

        if(item->status == 2)
        {
            handle_error(item->status,"browsefolder auth fail");
            free_server_list(item);
            free_server_list(cur);
            return NULL;
        }

        if(item->status != 0)
        {
            handle_error(item->status,"browsefolder");
            free_server_list(item);
            enter_sleep_time(1000*300,NULL);
            continue;
        }

        //Cdbg(API_DBG, "33,pageno=%d\n",pageno);
        haspage = item->page.hasnextpage;
        if(pageno > 1)
        {
            //Cdbg(API_DBG, "linkBrowseList start\n");
            temp = linkBrowseList(cur,item);
            if(temp == NULL)
            {
                free_server_list(item);
                free_server_list(cur);
                return NULL;
            }
            //Cdbg(API_DBG, "linkBrowseList end\n");
            my_free(item->filelist);
            my_free(item->folderlist);
            my_free(item);
            //Cdbg(API_DBG, "free item end\n");
            my_free(cur->filelist);
            my_free(cur->folderlist);
            my_free(cur);
            //Cdbg(API_DBG, "free cur end\n");
            cur = temp;
        }
        else
        {
           cur = item;
        }
        //Cdbg(API_DBG, "44,hasnextpage=%d\n",haspage);
        if(haspage)
            pageno++;
        else
            break;

    }

    if(pageno>1)
        Cdbg(API_DBG, "filenum=%d,foldernum=%d\n",cur->filenumber,cur->foldernumber);
    return cur;
}
#endif

int get_max_upload_filesize(char *username)
{
    Getinfo *gi = NULL;
    int filesize = 0;

    while(!exit_loop)
    {
        gi = getInfo(username,sergate.gateway);

        if(NULL == gi)
        {
            enter_sleep_time(1000*300,NULL);
            check_network_state();
            continue;
        }

        if(gi->status == S_AUTH_FAIL)
        {
            my_free(gi);
            if(obtainToken(username,password,&cfg,0) == -1)
                return -1;
            else
                continue;
        }

        if(gi->status != 0)
        {
            handle_error(gi->status,"getinfo");
            my_free(gi);
            enter_sleep_time(1000*300,NULL);
            continue;
        }

        filesize = gi->package.maxfilesize;
        my_free(gi);

        break;
    }

    return filesize;
}

int record_folder_id(Browse *br,int pid)
{
    int i,folderid;
    Changeseq *cs = NULL;

    if(pre_seq > 0)
        return 0;

    for(i=0;i<br->foldernumber;i++)
    {
       folderid = br->folderlist[i]->id;
       Cdbg(API_DBG, "folderid=%d\n",folderid);
       while( (cs = getChangeSeq(folderid)) == NULL)
       {
           enter_sleep_time(5,&wait_server_mutex);
       }
       if(cs->status != 0)
       {
          handle_error(cs->status,"get changeseq in record_folder_id");
          my_free(cs);
          return -1;

       } else {
            insert_node(pid,folderid,cs->changeseq);
            my_free(cs);
       }
    }
    return 0;
}

Browse *browseFolder(char *username,long long id,int issibiling,int pageno)
{
    int status;
    char url[NORMALSIZE];
    char postdata[MAXSIZE];
    char itemofpage[NORMALSIZE];   
    Browse *browse = getb(Browse);

    if( NULL == browse )
    {
        Cdbg(API_DBG, "create memery error\n");
        return NULL;
    }

    memset(browse,0,sizeof(Browse));

    snprintf(url,NORMALSIZE,"https://%s/folder/browse/",aaa.inforelay);
    //snprintf(url,NORMALSIZE,"https://%s/folder/browsefolder/",aaa.inforelay);
    
    snprintf(itemofpage,NORMALSIZE,"<page><pageno>%d</pageno><pagesize>200</pagesize><enable>0</enable></page>",pageno);
    //sprintf(itemoffilter,"%s","<filter><starttime></starttime><endtime></endtime></filter>");
    //sprintf(postdata,"<browse><token>%s</token><language>zh_TW</language><userid>%s</userid><folderid>%d</folderid><computerseq></computerseq><fileext></fileext>%s%s<sortby>0</sortby><sortdirection></sortdirection><issibiling>%d</issibiling></browse>"
    snprintf(postdata,MAXSIZE,"<browse><token>%s</token><language>zh_TW</language><userid>%s</userid><folderid>%lli</folderid>%s<issibiling>%d</issibiling></browse>"
            ,aaa.token,username,id,itemofpage,issibiling);

    Cdbg(API_DBG, "browseFolder postdata : %s\n", postdata);

    status = sendRequest(browse_folder_xml,url,postdata,NULL,NULL);

    if( status != 0 )
    {
        Cdbg(API_DBG, "curl error, status :%d", status);

        // 28 -> connect timeout
        if(status == 28) {
            browse->status = CONNECT_TIMEOUT;
        } else if(status == 6) {
            // 6 -> This error mean that curl can't resolve host name
            browse->status = CURL_CANNOT_RESOLVE_HOSTNAME;
        } else if(status == 35) {
            // 35 -> CURLE_SSL_CONNECT_ERROR
            browse->status = CURLE_SSL_CONNECT_ERROR;
        } else {
            browse->status = status;
        }

        return browse;
        // my_free(browse);
        // return NULL;
    }

      if(my_parse_browse_xml(browse_folder_xml,browse) == -1)
      {
          my_free(browse);
          Cdbg(API_DBG, "parse_browse_xml error\n");
          return NULL;

      } else {

        if(!upload_only)
            record_folder_id(browse,id);
        return browse;
    }

    return browse;
}


SlidingBrowse *slidingBrowseFile(char *username,long long folderid,long long timestamp)
{
    int status;
    char url[NORMALSIZE];
    char postdata[MAXSIZE];

    SlidingBrowse *browseFiles = getb(SlidingBrowse);

    if( NULL == browseFiles )
    {
        Cdbg(API_DBG, "create memery error\n");
        return NULL;
    }

    memset(browseFiles,0,sizeof(SlidingBrowse));

    snprintf(url,NORMALSIZE,"https://%s/inforelay/slidingbrowse/",aaa.inforelay);
    
    snprintf(postdata,MAXSIZE,"<slidingbrowse><token>%s</token><userid>%s</userid><folderid>%lli</folderid><timestamp>%lli</timestamp><fetchamount>60</fetchamount><isforwardfetch>1</isforwardfetch><rawfilename></rawfilename><filenamefilter></filenamefilter></slidingbrowse>"
            ,aaa.token,username,folderid,timestamp);


    Cdbg(API_DBG, "browseFiles postdata : %s\n", postdata);

    status = sendRequest(sliding_browse_file_xml,url,postdata,NULL,NULL);

    if( status != 0 )
    {

        Cdbg(API_DBG, "curl error, status :%d", status);

        // 28 -> connect timeout
        if(status == 28) {
            browseFiles->status = CONNECT_TIMEOUT;
        } else if(status == 6) {
            // 6 -> This error mean that curl can't resolve host name
            browseFiles->status = CURL_CANNOT_RESOLVE_HOSTNAME;
        } else if(status == 35) {
            // 35 -> CURLE_SSL_CONNECT_ERROR
            browseFiles->status = CURLE_SSL_CONNECT_ERROR;
        } else {
            browseFiles->status = status;
        }

        return browseFiles;

        // my_free(browseFiles);
        // return NULL;
    }

      if(parse_sliding_browse_xml(sliding_browse_file_xml, browseFiles) == -1)
      {
          my_free(browseFiles);
          Cdbg(API_DBG, "parse_sliding_browse_xml error\n");
          return NULL;
      }

      return browseFiles;
}


Propfind *checkEntryExisted(char *userid,int parentID,char *filename,char *type)
{
    char url[NORMALSIZE];
    char postdata[MAXSIZE];
    char entryName[NORMALSIZE];
    int status;
    char *encode = NULL;

    memset(url,0,sizeof(url));
    memset(postdata,0,sizeof(postdata));
    memset(entryName,0,sizeof(entryName));

    Propfind *find = getb(Propfind);

    if( NULL == find )
    {
        Cdbg(API_DBG, "create memery error\n");
        return NULL;
    }

    memset(find,0,sizeof(Propfind));

    strcpy(entryName,filename);
    encode = oauth_encode_base64(0,(const unsigned char *)entryName);

    if(NULL == encode)
    {
        handle_error(S_ENCODE_BASE64_FAIL,"checkEntryExisted");
        my_free(find);
        return NULL;
    }

    snprintf(url,NORMALSIZE,"https://%s/find/propfind/",aaa.inforelay);

    snprintf(postdata,MAXSIZE,"<propfind><token>%s</token><scrip></scrip><userid>%s</userid><parent>%d</parent><find>%s</find><type>%s</type></propfind>"
            ,aaa.token,userid,parentID,encode,type);

    Cdbg(API_DBG, "checkEntryExisted  postdata : %s\n", postdata);

    status = sendRequest(propfind_xml,url,postdata,NULL,NULL);

    my_free(encode);

    if( status != 0 )
    {
        handle_error(status,"curl");
        my_free(find);
        return NULL;
    }


    if(parseDoc1(propfind_xml,find) == -1)
    {
        my_free(find);
        return NULL;

    }
    return find;
}


Propfind *checkEntryIdExisted(char *userid,long long parentID,char *filename,char *type)
{
    char url[NORMALSIZE];
    char postdata[MAXSIZE];
    char entryName[NORMALSIZE];
    int status;
    char *encode = NULL;

    memset(url,0,sizeof(url));
    memset(postdata,0,sizeof(postdata));
    memset(entryName,0,sizeof(entryName));

    Propfind *find = getb(Propfind);

    if( NULL == find )
    {
        Cdbg(API_DBG, "create memery error\n");
        return NULL;
    }

    memset(find,0,sizeof(Propfind));

    strcpy(entryName,filename);
    encode = oauth_encode_base64(0,(const unsigned char *)entryName);

    if(NULL == encode)
    {
        handle_error(S_ENCODE_BASE64_FAIL,"checkEntryExisted");
        my_free(find);
        return NULL;
    }

    snprintf(url,NORMALSIZE,"https://%s/find/propfind/", aaa.inforelay);

    snprintf(postdata,MAXSIZE,"<propfind><token>%s</token><scrip></scrip><userid>%s</userid><parent>%lli</parent><find>%s</find><type>%s</type></propfind>"
            ,aaa.token,userid,parentID,encode,type);

    Cdbg(API_DBG, "checkEntryIdExisted  url : %s\n", url);
    Cdbg(API_DBG, "checkEntryIdExisted  postdata : %s\n", postdata);

    status = sendRequest(propfind_xml,url,postdata,NULL,NULL);

    my_free(encode);

    Cdbg(API_DBG, "checkEntryIdExisted  sendRequest->status  : %d\n", status );


    if( status != 0 )
    {
        handle_error(status,"curl");

        // 28 -> connect timeout
        if(status == 28) {
            find->status = CONNECT_TIMEOUT;
        } else if(status == 6) {
            // 6 -> This error mean that curl can't resolve host name
            find->status = CURL_CANNOT_RESOLVE_HOSTNAME;
        } else if(status == 35) {
            // 35 -> CURLE_SSL_CONNECT_ERROR
            find->status = CURLE_SSL_CONNECT_ERROR;
        } else {
            find->status = status;
        }

        return find;
    }



    if(parseDoc1(propfind_xml,find) == -1)
    {
        my_free(find);
        return NULL;

    }
    
    return find;
}





Createfolder *createFolder(char *username,int parentID,int isencrpted,char *name)
{
    
    Cdbg(API_DBG, "create Folder %s\n", name);
    Cdbg(API_DBG, "create Folder parentID %d\n", parentID);

    // Cdbg(API_DBG, "create strcmp name %d\n", strcmp(name,"Alarm"));
        
    // if(strcmp(name,"Alarm") != 0) {
    //     Cdbg(API_DBG, "create name 00000\n");
    //     //add_sync_item("create_folder_fail",name,up_excep_fail);
    // } else {
    //     Cdbg(API_DBG, "create name 11111\n");
    // }

    int status;
    char url[NORMALSIZE];
    char postdata[MAXSIZE];
    char at[MAXSIZE];
    unsigned long int sec;
    char timestamp[MINSIZE];
    char *entryName =  NULL;
    char *encode = NULL;
    Createfolder *cf = getb(Createfolder);

        
    if( NULL == cf )
    {
        Cdbg(API_DBG, "create memery error\n");
        return NULL;
    }

    memset(cf,0,sizeof(Createfolder));

        
    entryName = parse_name_from_path(name);


    Cdbg(API_DBG, "Folder name : %s\n", entryName);

    if(entryName == NULL)
    {
        my_free(cf);
        Cdbg(API_DBG, "obtain name fail by parse path");
        return NULL;
    }

    encode = oauth_encode_base64(0,(const unsigned char *)entryName);
    my_free(entryName);

    if(NULL == encode)
    {
        handle_error(S_ENCODE_BASE64_FAIL,"createFolder");
        my_free(cf);
        return NULL;
    }

    sec = time((time_t *)NULL);
    snprintf(timestamp,MINSIZE,"%lu",sec);
    snprintf(at,MAXSIZE,"<creationtime>%s</creationtime><lastaccesstime>%s</lastaccesstime><lastwritetime>%s</lastwritetime>"
            ,timestamp,timestamp,timestamp);
    snprintf(url,NORMALSIZE,"https://%s/folder/create/",aaa.inforelay);
    snprintf(postdata,MAXSIZE,"<create><token>%s</token><userid>%s</userid><parent>%d</parent><isencrypted>%d</isencrypted><display>%s</display><attribute>%s</attribute></create>"
            ,aaa.token,username,parentID,isencrpted,encode,at);

    Cdbg(API_DBG, "createFolder postdata : \n%s\n", postdata);
                        
    status = sendRequest(create_folder_xml,url,postdata,NULL,NULL);

    Cdbg(API_DBG, "status : %d\n", status);
        
    my_free(encode);

    if( status != 0 )
    {
        handle_error(status,"curl");
        my_free(cf);
        return NULL;
    }

    if(parseDoc1(create_folder_xml,cf) == -1)
    {
        my_free(cf);
        return NULL;

    } else {
        //del_sync_item("create_folder_fail",name,up_excep_fail);
        //Cdbg(API_DBG, "del_sync_item 001");
        if(cf->status == 0)
        {
            Changeseq *cs = NULL;
            int seq = 0;
            while( (cs = getChangeSeq(cf->id)) == NULL)
            {
               enter_sleep_time(5,&wait_server_mutex);
            }
            if(cs->status == 0)
                seq = cs->changeseq;
            my_free(cs);
            insert_node(parentID,cf->id,seq);
        }
        return cf;
    }
}




Createfolder *createFolderId(char *username,long long parentID,int isencrpted,char *name)
{
    
    Cdbg(API_DBG, "createFolderId -> name : %s\n", name);
    Cdbg(API_DBG, "createFolderId -> parentID : %lli\n", parentID);

    int status;
    char url[NORMALSIZE];
    char postdata[MAXSIZE];
    char at[MAXSIZE];
    unsigned long int sec;
    char timestamp[MINSIZE];
    char *entryName =  NULL;
    char *encode = NULL;
    Createfolder *cf = getb(Createfolder);

    if( NULL == cf )
    {
        Cdbg(API_DBG, "createFolderId -> create memery error\n");
        return NULL;
    }

    memset(cf,0,sizeof(Createfolder));

        
    entryName = parse_name_from_path(name);


    Cdbg(API_DBG, "createFolderId -> entryName : %s\n", entryName);

    if(entryName == NULL)
    {
        my_free(cf);
        Cdbg(API_DBG, "createFolderId -> obtain name fail by parse path");
        return NULL;
    }

    encode = oauth_encode_base64(0,(const unsigned char *)entryName);
    my_free(entryName);

    if(NULL == encode)
    {
        handle_error(S_ENCODE_BASE64_FAIL,"createFolder");
        my_free(cf);
        return NULL;
    }

    sec = time((time_t *)NULL);
    snprintf(timestamp,MINSIZE,"%lu",sec);
    snprintf(at,MAXSIZE,"<creationtime>%s</creationtime><lastaccesstime>%s</lastaccesstime><lastwritetime>%s</lastwritetime>"
            ,timestamp,timestamp,timestamp);
    snprintf(url,NORMALSIZE,"https://%s/folder/create/",aaa.inforelay);
    snprintf(postdata,MAXSIZE,"<create><token>%s</token><userid>%s</userid><parent>%lli</parent><isencrypted>%d</isencrypted><display>%s</display><attribute>%s</attribute></create>"
            ,aaa.token,username,parentID,isencrpted,encode,at);

    Cdbg(API_DBG, "createFolderId ->  postdata : \n%s\n", postdata);
                        
    status = sendRequest(create_folder_xml,url,postdata,NULL,NULL);

    my_free(encode);

    Cdbg(API_DBG, "createFolderId ->  sendRequest->status : %d\n", status);
        

    if( status != 0 )
    {
        handle_error(status,"curl");

        // 28 -> connect timeout
        if(status == 28) {
            // 28 -> timeout
            cf->status = CONNECT_TIMEOUT;
        } else if(status == 6) {
            // 6 -> This error mean that curl can't resolve host name
            cf->status = CURL_CANNOT_RESOLVE_HOSTNAME;
        } else if(status == 35) {
            // 35 -> CURLE_SSL_CONNECT_ERROR
            cf->status = CURLE_SSL_CONNECT_ERROR;
        } else {
            cf->status = status;
        }

        cf->id = 0;

        return cf;
    }


    if(parseDoc1(create_folder_xml,cf) == -1)
    {
        my_free(cf);
        return NULL;

    } else {
        //del_sync_item("create_folder_fail",name,up_excep_fail);
        //Cdbg(API_DBG, "del_sync_item 001");

        Cdbg(API_DBG, "createFolderId ->  cf->status = %d, id = %lli\n", cf->status, cf->id);
    }

    return cf;
}

Operateentry *renameEntry(char *username,int id,int isencrpted,char *newname,int isfolder)
{
    int status;
    char url[NORMALSIZE];
    char postdata[MAXSIZE];
    char type[MINSIZE];
    char *entryName = NULL;
    Operateentry *oe = getb(Operateentry);

    if( NULL == oe )
    {
        Cdbg(API_DBG, "create memery error\n");
        return NULL;
    }

    memset(oe,0,sizeof(Operateentry));

    if(isfolder)
    {
        strcpy(type,"folder");
    }
    else
    {
        strcpy(type,"file");
    }

    entryName = oauth_encode_base64(0,(const unsigned char *)newname);

    if(NULL == entryName)
    {
        handle_error(S_ENCODE_BASE64_FAIL,"renameEntry");
        my_free(oe);
        return NULL;
    }

    snprintf(url,NORMALSIZE,"https://%s/%s/rename/",aaa.inforelay,type);
    snprintf(postdata,MAXSIZE,"<rename><token>%s</token><userid>%s</userid><id>%d</id><isencrypted>%d</isencrypted><display>%s</display></rename>"
            ,aaa.token,username,id,isencrpted,entryName);

    Cdbg(API_DBG, "renameEntry postdata : %s\n", postdata);

    my_free(entryName);

    status = sendRequest(rename_xml,url,postdata,NULL,NULL);

    if( status != 0 )
    {
        handle_error(status,"curl");
        my_free(oe);
        return NULL;
    }


    if(parseDoc1(rename_xml,oe) == -1)
    {
        my_free(oe);
        return NULL;
    }

    return oe;
}

Moveentry *moveEntry(char *username,int id,char *name,int parentID,int isfolder,int pre_pid)
{
    int status;
    char url[NORMALSIZE];
    char postdata[MAXSIZE];
    char type[MINSIZE];
    char entryName[NORMALSIZE];
    char *encode = NULL;
    Moveentry *me = getb(Moveentry);

    if( NULL == me )
    {
        Cdbg(API_DBG, "create memery error\n");
        return NULL;
    }

    memset(me,0,sizeof(Moveentry));

    if(isfolder)
    {
        strcpy(type,"folder");
    }
    else
    {
        strcpy(type,"file");
    }

    if( NULL == name)
    {
        strcpy(entryName,name);
        encode = oauth_encode_base64(0,(const unsigned char *)entryName);
        if(NULL == encode)
        {
            handle_error(S_ENCODE_BASE64_FAIL,"moveEntry");
            my_free(me);
            return NULL;
        }
        snprintf(postdata,MAXSIZE,"<move><token>%s</token><userid>%s</userid><id>%d</id><display>%s</display><parent>%d</parent></move>"
                ,aaa.token,username,id,encode,parentID);
        my_free(encode);
    }
    else
    {
        strcpy(entryName,"");
        snprintf(postdata,MAXSIZE,"<move><token>%s</token><userid>%s</userid><id>%d</id><display>%s</display><parent>%d</parent></move>"
                ,aaa.token,username,id,entryName,parentID);
    }

    snprintf(url,NORMALSIZE,"https://%s/%s/move/",aaa.inforelay,type);

    status = sendRequest(move_xml,url,postdata,NULL,NULL);


    if( status != 0 )
    {
        handle_error(status,"curl");
        my_free(me);
        return NULL;
    }


    if(parseDoc1(move_xml,me) == -1)
    {
        my_free(me);
        return NULL;
    }
    else
    {
        if(isfolder)
        {
            move_node(pre_pid,id,parentID);
        }
        return me;
    }
}

Operateentry *removeEntry(char *username,int id,int ischildonly,int isfolder,int pid)
{
    int status;
    char url[NORMALSIZE];
    char postdata[MAXSIZE];
    char type[MINSIZE];
    Operateentry *oe = getb(Operateentry);

    if( NULL == oe )
    {
        Cdbg(API_DBG, "create memery error\n");
        return NULL;
    }

    memset(oe,0,sizeof(Operateentry));

    if(isfolder)
    {
        strcpy(type,"folder");
        snprintf(postdata,MAXSIZE,"<remove><token>%s</token><userid>%s</userid><id>%d</id><ischildonly>%d</ischildonly></remove>"
                ,aaa.token,username,id,ischildonly);

    }
    else
    {
        strcpy(type,"file");
        snprintf(postdata,MAXSIZE,"<remove><token>%s</token><userid>%s</userid><id>%d</id></remove>"
                ,aaa.token,username,id);
    }

    Cdbg(API_DBG, "removeEntry postdata : %s\n", postdata);

    snprintf(url,NORMALSIZE,"https://%s/%s/remove/",aaa.inforelay,type);

    status = sendRequest(remove_xml,url,postdata,NULL,NULL);

    if( status != 0 )
    {
        handle_error(status,"curl");
        my_free(oe);
        return NULL;
    }


    if(parseDoc1(remove_xml,oe) == -1)
    {
        my_free(oe);
        return NULL;

    }
    else
    {
        // if(isfolder)
        //     del_node(pid,id);
        return oe;
    }
}


Operateentry *removeFiles(char *username,char * files_id)
{
    int status;
    char url[NORMALSIZE];
    char postdata[MAXSIZE];

    Operateentry *rf = getb(Operateentry);

    if( NULL == rf )
    {
        Cdbg(API_DBG, "create memery error\n");
        return NULL;
    }

    memset(rf,0,sizeof(Operateentry));

    snprintf(postdata,MAXSIZE,"<remove><token>%s</token><userid>%s</userid><id>%s</id><cleanorigdeletedfile>1</cleanorigdeletedfile><cleanupimmediately>1</cleanupimmediately></remove>"
                ,aaa.token, username, files_id);

    snprintf(url,NORMALSIZE,"https://%s/file/remove/",aaa.inforelay);

    Cdbg(API_DBG, "remove url : %s", url);
    Cdbg(API_DBG, "remove postdata : %s", postdata);

    status = sendRequest(remove_xml,url,postdata,NULL,NULL);

    if( status != 0 )
    {


        Cdbg(API_DBG, "curl error, status :%d", status);

        // 28 -> connect timeout
        if(status == 28) {
            rf->status = CONNECT_TIMEOUT;
        } else if(status == 6) {
            // 6 -> This error mean that curl can't resolve host name
            rf->status = CURL_CANNOT_RESOLVE_HOSTNAME;
        } else if(status == 35) {
            // 35 -> CURLE_SSL_CONNECT_ERROR
            rf->status = CURLE_SSL_CONNECT_ERROR;
        } else {
            rf->status = status;
        }

        return rf;


        // my_free(rf);
        // return NULL;
    }


    if(parseDoc1(remove_xml,rf) == -1)
    {
        my_free(rf);
        Cdbg(API_DBG, "parse remove_xml error");
        return NULL;
    }

    return rf;
}

int updateEntryAttribute(char *username,int id,int parentID,int isencrpted,int isfolder)
{
    int status;
    char url[NORMALSIZE];
    char postdata[MAXSIZE];
    char at[MAXSIZE];
    unsigned long int sec;
    char timestamp[MINSIZE];
    char type[MINSIZE];

    if(isfolder)
    {
        strcpy(type,"folder");
        snprintf(postdata,MAXSIZE,"<updateattribute><token>%s</token><userid>%s</userid><folder>%d</folder><parent>%d</parent><isencrypted>%d</isencrypted><attribute>%s</attribute></updateattribute>"
                ,aaa.token,username,id,parentID,isencrpted,at);
    }
    else
    {
        strcpy(type,"file");
        snprintf(postdata,MAXSIZE,"<updateattribute><token>%s</token><userid>%s</userid><folder>%d</folder><attribute>%s</attribute></updateattribute>"
                ,aaa.token,username,id,at);
    }

    sec = time((time_t *)NULL);
    snprintf(timestamp,MINSIZE,"%lu",sec);
    snprintf(at,MAXSIZE,"<creationtime>%s</creationtime><lastaccesstime>%s</lastaccesstime><lastwritetime>%s</lastwritetime>"
            ,timestamp,timestamp,timestamp);
    snprintf(url,NORMALSIZE,"https://%s/%s/updateattribute/",aaa.inforelay,type);

    status = sendRequest(update_xml,url,postdata,NULL,NULL);

    if( status != 0 )
    {
        handle_error(status,"curl");
        return -1;
    }

    return 0;
}

Getentryinfo *getEntryInfo(int isfolder,int entryid)
{
    int status;
    char url[NORMALSIZE];
    char postdata[MAXSIZE];
    Getentryinfo *ge = getb(Getentryinfo);

    if( NULL == ge )
    {
        Cdbg(API_DBG, "create memery error\n");
        return NULL;
    }

    memset(ge,0,sizeof(Getentryinfo));

    snprintf(url,NORMALSIZE,"https://%s/fsentry/getentryinfo/",aaa.inforelay);
    snprintf(postdata,MAXSIZE,"<getentryinfo><token>%s</token><isfolder>%d</isfolder><entryid>%d</entryid></getentryinfo>"
            ,aaa.token,isfolder,entryid);

    status = sendRequest(get_entry_info_xml,url,postdata,NULL,NULL);

    if( status != 0 )
    {
        handle_error(status,"curl");
        my_free(ge);
        return NULL;
    }

    if(parseDoc1(get_entry_info_xml,ge) == -1)
    {
        my_free(ge);
        return NULL;

    }
    return ge;
}

#if 0
int getLatestChangeFiles(char *username,int top,int targetroot,int sortdirection)
{
    int status;
    char url[NORMALSIZE];
    char postdata[MAXSIZE];

    snprintf(url,NORMALSIZE,"https://%s/file/getlatestchangefiles/",aaa.inforelay);
    snprintf(postdata,MAXSIZE,"<getlatestchangefiles><userid>%s</userid><token>%s</token><top>%d</top><targetroot>%d</targetroot><sortdirection>%d</sortdirection></getlatestchangefiles>"
            ,username,aaa.token,top,targetroot,sortdirection);

    status = sendRequest(get_change_files_xml,url,postdata,NULL,NULL);

    if( status != 0 )
    {
        handle_error(status,"curl");
        return -1;
    }


    return 0;
}

int getLatestUploads(char *username,int top,int targetroot,int sortdirection)
{
    int status;
    char url[NORMALSIZE];
    char postdata[MAXSIZE];

    snprintf(url,NORMALSIZE,"https://%s/file/getlatestuploads/",aaa.inforelay);
    snprintf(postdata,MAXSIZE,"<getlatestuploads><userid>%s</userid><token>%s</token><top>%d</top><targetroot>%d</targetroot><sortdirection>%d</sortdirection></getlatestuploads>"
            ,username,aaa.token,top,targetroot,sortdirection);

    status = sendRequest(get_uploads_xml,url,postdata,NULL,NULL);

    if( status != 0 )
    {
        handle_error(status,"curl");
        return -1;
    }


    return 0;
}

int setEntryMark(int isfolder,int entryid,int markid)
{
    int status;
    char url[NORMALSIZE];
    char postdata[MAXSIZE];

    snprintf(url,NORMALSIZE,"https://%s/fsentry/setentrymark/",aaa.inforelay);
    snprintf(postdata,MAXSIZE,"<setentrymark><token>%s</token><isfolder>%d</isfolder><entryid>%d</entryid><markid>%d</markid></setentrymark>"
            ,aaa.token,isfolder,entryid,markid);

    status = sendRequest(set_mark_xml,url,postdata,NULL,NULL);

    if( status != 0 )
    {
        handle_error(status,"curl");
        return -1;
    }

    return 0;
}

int getShareCode(char *username,int entryType,int entryID,char *password,int actionType)
{
    int status;
    char url[NORMALSIZE];
    char postdata[MAXSIZE];

    snprintf(url,NORMALSIZE,"https://%s/fsentry/getsharecode/",aaa.inforelay);
    snprintf(postdata,MAXSIZE,"<getsharecode><token>%s</token><script></script><userid>%s</userid><entrytype>%d</entrytype><entryid>%d</entryid><password>%s</password><actiontype>%d</actiontype></getsharecode>"
            ,aaa.token,username,entryType,entryID,password,actionType);

    status = sendRequest(get_share_code_xml,url,postdata,NULL,NULL);

    if( status != 0 )
    {
        handle_error(status,"curl");
        return -1;
    }


    return 0;
}

int deleteShareCode(char *username,int entryType,int entryID,char *password)
{
    int status;
    char url[NORMALSIZE];
    char postdata[MAXSIZE];

    snprintf(url,NORMALSIZE,"https://%s/fsentry/deletesharecode/",aaa.inforelay);
    snprintf(postdata,MAXSIZE,"<deletesharecode><token>%s</token><script></script><userid>%s</userid><entrytype>%d</entrytype><entryid>%d</entryid><password>%s</password></deletesharecode>"
            ,aaa.token,username,entryType,entryID,password);

    status = sendRequest(del_share_code_xml,url,postdata,NULL,NULL);

    if( status != 0 )
    {
        handle_error(status,"curl");
        return -1;
    }

    return 0;
}

int getSharedEntries(char *username,int kind,int pagesize,int sortby,int sortdirection,char *firstentrybound)
{
    int status;
    char url[NORMALSIZE];
    char postdata[MAXSIZE];

    snprintf(url,NORMALSIZE,"https://%s/fsentry/getsharedentries/",aaa.inforelay);
    snprintf(postdata,MAXSIZE,"<getsharedentries><token>%s</token><userid>%s</userid><kind>%d</kind><pagesize>%d</pagesize><sortby>%d</sortby><sortdirection>%d</sortdirection><firstentrybound>%s</firstentrybound></getsharedentries>"
            ,aaa.token,username,kind,pagesize,sortby,sortdirection,firstentrybound);

    status = sendRequest(get_share_entry_xml,url,postdata,NULL,NULL);

    if( status != 0 )
    {
        handle_error(status,"curl");
        return -1;
    }

    return 0;
}

/*getSharedFiles API is nousing*/
#if 0
int getSharedFiles(char *username,int count)
{
    char *infofilename = "../../asuswebstorage/xml/aaa.xml";
    char *getsharedfiles_filename = "../../asuswebstorage/xml/getsharedfiles.xml";

    char url[NORMALSIZE];
    char postdata[MAXSIZE];

    /*obtain token and inforelay*/
    parseDoc(infofilename);

    sprintf(url,"https://%s/fsentry/getsharedfiles/",aaa.inforelay);
    sprintf(postdata,"<getsharedfiles><token>%s</token><userid>%s</userid><count>%d</count></getsharedfiles>"
            ,aaa.token,username,count);

    sendRequest(getsharedfiles_filename,url,postdata,NULL,NULL);

    return 0;
}
#endif

int checkPassword(char *username,char *suri)
{
    int status;
    char url[NORMALSIZE];
    char postdata[MAXSIZE];

    snprintf(url,NORMALSIZE,"https://%s/fsentry/checkpassword/",aaa.inforelay);
    snprintf(postdata,MAXSIZE,"<checkpassword><token>%s</token><userid>%s</userid><suri>%s</suri></checkpassword>"
            ,aaa.token,username,suri);

    status = sendRequest(check_pwd_xml,url,postdata,NULL,NULL);

    if( status != 0 )
    {
        handle_error(status,"curl");
        return -1;
    }


    return 0;
}

int comparePassword(char *username,int isfolder,int ffid,char *password)
{
    int status;
    char url[NORMALSIZE];
    char postdata[MAXSIZE];

    snprintf(url,NORMALSIZE,"https://%s/fsentry/comparepassword/",aaa.inforelay);
    snprintf(postdata,MAXSIZE,"<comparepassword><token>%s</token><userid>%s</userid><isfolder>%d</isfolder><ffid>%d</ffid><passwd>%s</passwd></comparepassword>"
            ,aaa.token,username,isfolder,ffid,password);

    status = sendRequest(cmp_pwd_xml,url,postdata,NULL,NULL);

    if( status != 0 )
    {
        handle_error(status,"curl");
        return -1;
    }


    return 0;
}
#endif

Changeseq *getChangeSeq(int folderid)
{
    int status;
    char url[NORMALSIZE];
    char postdata[MAXSIZE];
    Changeseq *cs = getb(Changeseq);

    if( NULL == cs )
    {
        Cdbg(API_DBG, "create memery error\n");
        return NULL;
    }

    memset(cs,0,sizeof(Changeseq));

    snprintf(url,NORMALSIZE,"https://%s/folder/getchangeseq/",aaa.inforelay);
    snprintf(postdata,MAXSIZE,"<getchangeseq><token>%s</token><scrip></scrip><folderid>%d</folderid></getchangeseq>"
            ,aaa.token,folderid);

    //Cdbg(API_DBG, "url is %s,postdat is %s\n",url,postdata);
    status = sendRequest(get_change_seq_xml,url,postdata,NULL,NULL);
    //Cdbg(API_DBG, "status is %d\n",status);

    if( status != 0 )
    {
        handle_error(status,"curl");
        my_free(cs);
        return NULL;
    }

    if(parseDoc1(get_change_seq_xml,cs) == -1)
    {
        my_free(cs);
        return NULL;
    }
    return cs;
}

/*encode file by sha512*/
int do_fp(FILE *f,char *checksum);
void pt(unsigned char *md,char*checksum);

int read(int, void *, unsigned int);

int do_fp(FILE *f,char *checksum)
{
    SHA512_CTX c;
    unsigned char md[SHA512_DIGEST_LENGTH];
    int fd;
    int i;
    unsigned char buf[BUFSIZE];

    fd=fileno(f);
    SHA512_Init(&c);
    for (;;)
    {
        if(exit_loop)
            return -1;
        i = read(fd,buf,BUFSIZE);
        if (i <= 0) break;
        SHA512_Update(&c,buf,(unsigned long)i);
    }
    SHA512_Final(&(md[0]),&c);
    pt(md,checksum);

    return 0;
}

void pt(unsigned char *md, char *checksum)
{
    int i;
    char temp[3];

    for (i=0; i<SHA512_DIGEST_LENGTH; i++)
    {
        memset(temp,0,sizeof(temp));
        sprintf(temp,"%02x",md[i]);
        strcat(checksum,temp);
    }
}

int sha512(char *filename,char *checksum)
{
    FILE *IN;

    IN=fopen(filename,"r");
    if (IN == NULL)
    {
       Cdbg(API_DBG, "%s can't open \n",filename);
       return -1;
    }
    if(do_fp(IN,checksum) == -1)
    {
        fclose(IN);
        return -1;
    }
    fclose(IN);

    return 0;
}

int if_server_space_full(char *filename)
{
    struct stat filestat;
    long long int filesize;
    long long int server_free_capacity;

    if( stat(filename,&filestat) == -1)
    {
        Cdbg(API_DBG, "servr sapce full stat error:%s file not exist\n",filename);
        return -1;
    }

    filesize = filestat.st_size;

    while( (server_free_capacity = check_server_space(username)) == -1)
    {
        if(exit_loop)
            return -1;
        enter_sleep_time(1000*500,NULL);
        check_network_state();
    }

    if(server_free_capacity > 0)
    {
        if( filesize > server_free_capacity * 1024 *1024)
        {
            handle_error(S_SERVER_SPACE_FULL,"upload");
            return 1;
        }
    }

    return 0;
}


int process_error_xml(char * filename) {

    FILE *pFile;
    char buffer[2048];
    memset(buffer,0,sizeof(buffer));

    pFile = fopen(filename, "r");
    if ( NULL == pFile ){
        Cdbg(API_DBG, "Open [ %s ] file failure", filename);
        return -1;
    }else{
        Cdbg(API_DBG, "Open [ %s ] file success", filename);
        fread( buffer, 2048, 1, pFile );
    }

    fclose(pFile);

    Cdbg(API_DBG, "[ %s ] content : %s", filename, buffer);

    Cdbg(API_DBG, "content len : %d", strlen(buffer));

    if( strstr(buffer, "Error Code:2")) {

        Cdbg(API_DBG, "Error Code : 2 = Validate token fail");
        return 2;

    } else if( strlen(buffer) == 0) {
        return 2;
    }

    return -1;

}



Initbinaryupload *initBinaryUpload(char *filename,long long parentID,char *transid,int fileID)
{

    int status;
    char url[MAXSIZE];
    char checksum[NORMALSIZE];
    char at[NORMALSIZE];
    char *at_encode = NULL;
    char fileCtime[MINSIZE];
    char fileAtime[MINSIZE];
    char fileMtime[MINSIZE];
    char name_raw[NORMALSIZE];
    char *finalname = NULL;
    
    //long long int server_free_capacity;
    char *encode = NULL;

    memset(checksum,0,sizeof(checksum));

    struct stat filestat;
    long long int filesize;


    Initbinaryupload *ibu = getb(Initbinaryupload);

    if( NULL == ibu )
    {
        Cdbg(API_DBG, "create memery error\n");
        return NULL;
    }

    memset(ibu,0,sizeof(Initbinaryupload));


    if( stat(filename,&filestat) == -1)
    {
        Cdbg(API_DBG, "InitBinaryUpload stat error:%s file not exist\n",filename);
        return NULL;
    }

    filesize = filestat.st_size;

    snprintf(fileCtime,MINSIZE,"%ld",filestat.st_ctime);
    snprintf(fileAtime,MINSIZE,"%ld",filestat.st_atime);
    snprintf(fileMtime,MINSIZE,"%ld",filestat.st_mtime);

    char *p = strrchr(filename,'/');

    if(NULL == p)
    {
        return NULL;
    }

    p++;
    strcpy(name_raw,p);

    encode = oauth_encode_base64(0,(const unsigned char *)name_raw);

    if(NULL == encode)
    {
        handle_error(S_ENCODE_BASE64_FAIL,"initBinaryUpload");
        return NULL;
    }

    finalname = oauth_url_escape(encode);
    my_free(encode);

    if(NULL == finalname)
    {
        handle_error(S_URL_ESCAPE_FAIL,"initBinaryUpload");
        return NULL;
    }

    snprintf(at,NORMALSIZE,"<creationtime>%s</creationtime><lastaccesstime>%s</lastaccesstime><lastwritetime>%s</lastwritetime>"
            ,fileCtime,fileAtime,fileMtime);

    at_encode = oauth_url_escape(at);

    if(NULL == at_encode)
    {
        handle_error(S_URL_ESCAPE_FAIL,"initBinaryUpload");
        my_free(finalname);
        return NULL;
    }

    if(sha512(filename,checksum) == -1)
    {
        Cdbg(API_DBG, "sha512 fail\n");
        my_free(finalname);
        my_free(at_encode);
        return NULL;
    }

    if(NULL == transid)
    {
        if(fileID == 0)
        {
            snprintf(url,MAXSIZE,"https://%s/webrelay/initbinaryupload/?tk=%s&pa=%lli&na=%s&at=%s&fs=%lld&sg=%s&sc=&dis=%s&ai=0"
               ,aaa.webrelay,aaa.token,parentID,finalname,at_encode,filesize,checksum,sid);
        }
        else if(fileID > 0)
        {
            snprintf(url,MAXSIZE,"https://%s/webrelay/initbinaryupload/?tk=%s&pa=%lli&na=%s&at=%s&fi=%d&fs=%lld&sg=%s&sc=&dis=%s&ai=0"
               ,aaa.webrelay,aaa.token,parentID,finalname,at_encode,fileID,filesize,checksum,sid);
        }
    }
    else
    {
        if(fileID == 0)
        {
            snprintf(url,MAXSIZE,"https://%s/webrelay/initbinaryupload/?tk=%s&pa=%lli&na=%s&at=%s&fs=%lld&sg=%s&sc=&dis=%s&tx=%s&ai=0"
                ,aaa.webrelay,aaa.token,parentID,finalname,at_encode,filesize,checksum,sid,transid);
        }
        else if(fileID > 0)
        {
            snprintf(url,MAXSIZE,"https://%s/webrelay/initbinaryupload/?tk=%s&pa=%lli&na=%s&at=%s&fi=%d&fs=%lld&sg=%s&sc=&dis=%s&tx=%s&ai=0"
                ,aaa.webrelay,aaa.token,parentID,finalname,at_encode,fileID,filesize,checksum,sid,transid);
        }
    }


    Cdbg(API_DBG, "Curl url : %s\n", url);


    status = sendRequest(init_upload_xml,url,NULL,NULL,NULL);


    Cdbg(API_DBG, "sendRequest status : %d\n", status);

    my_free(finalname);
    my_free(at_encode);

    if( status != 0 )
    {

        handle_error(status,"curl");

        // 28 -> connect timeout
        if(status == 28) {
            ibu->status = CONNECT_TIMEOUT;
        } else if(status == 6) {
            // 6 -> This error mean that curl can't resolve host name
            ibu->status = CURL_CANNOT_RESOLVE_HOSTNAME;
        } else if(status == 35) {
            // 35 -> CURLE_SSL_CONNECT_ERROR
            ibu->status = CURLE_SSL_CONNECT_ERROR;
        } else {
            ibu->status = status;
        }

        return ibu;


        //return NULL;
    }

    //sleep(10);

    if(parseDoc1(init_upload_xml,ibu) == -1)
    {
        Cdbg(API_DBG, "init_upload_xml parse error\n");
        

        int token_status = process_error_xml(init_upload_xml);

        if(token_status == 2) {

            ibu->status = token_status;
            return ibu;

        } else {
        
            my_free(ibu);

            return NULL;
        }

    }
    
   return ibu;

}


Resumebinaryupload *resumeBinaryUpload(char *filename, Initbinaryupload *ibu)
{
    FILE *fd;
    FILE *output;
    CURL *curl;
    CURLcode res;
    char cookies[NORMALSIZE];
    char url[NORMALSIZE];
    struct stat filestat;
    int filesize;
    Resumebinaryupload *rbu = NULL;
    char range[128];
    int offset;

    offset = ibu->offset;

    memset(range,0,sizeof(range));


    rbu = getb(Resumebinaryupload);

    if( NULL == rbu )
    {
        Cdbg(API_DBG, "resumeBinaryUpload -> create memery error\n");
        return NULL;
    }

    memset(rbu,0,sizeof(Resumebinaryupload));

    snprintf(dbg_msg, sizeof(dbg_msg), "resumeBinaryUpload -> filename : %s\n", filename);
    // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
    Cdbg(API_DBG, "%s", dbg_msg);

    if( stat(filename,&filestat) == -1)
    {
        Cdbg(API_DBG, "resumeBinaryUpload -> stat error:%s file not exist\n",filename);
        return NULL;
    }

    filesize = filestat.st_size;

    snprintf(dbg_msg, sizeof(dbg_msg), "######filesize is %d,offset is %d #####\n",filesize,offset);
    // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
    Cdbg(API_DBG, "%s", dbg_msg);

#if 1
    snprintf(cookies,NORMALSIZE,"OMNISTORE_VER=1_0; path=/;sid=%s;v=%s",sid,VERSION);
    snprintf(url,NORMALSIZE,"https://%s/webrelay/resumebinaryupload/?tk=%s&tx=%s&dis=%s&ai=0"
            ,aaa.webrelay,aaa.token,ibu->transid,sid);


    snprintf(dbg_msg, sizeof(dbg_msg), "OMNISTORE_VER=1_0; path=/;sid=%s;v=%s",sid,VERSION);
    // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
    Cdbg(API_DBG, "%s", dbg_msg);


    if( NULL==(fd= fopen(resume_upload_xml,"w")) )
    {

        snprintf(dbg_msg, sizeof(dbg_msg), "fopen 'w' [%s] == NULL ", resume_upload_xml);
        // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
        Cdbg(API_DBG, "%s", dbg_msg);

        return NULL;
    }


    if( NULL==(output= fopen(filename,"rb")) )
    {

        snprintf(dbg_msg, sizeof(dbg_msg), "fopen 'rb' [%s] == NULL ", filename);
        // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
        Cdbg(API_DBG, "%s", dbg_msg);

        fclose(fd);


        return NULL;
    }

    if(offset > 0)
        fseek(output,offset,0);

    curl = curl_easy_init();
#if 0
    //curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
    curl_easy_setopt(curl,CURLOPT_NOPROGRESS,0L);
    curl_easy_setopt(curl,CURLOPT_PROGRESSFUNCTION,my_progress_func);
    curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, progress_data);
#endif
    curl_easy_setopt(curl,CURLOPT_COOKIE,cookies);
    //CURLOPT_SSL_VERIFYHOST
    curl_easy_setopt(curl,CURLOPT_SSL_VERIFYHOST,0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    //curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
    //curl_easy_setopt(curl, CURLOPT_SSL_SESSIONID_CACHE, 0);
    //curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION,write_data);
    curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION,my_write_func);
    curl_easy_setopt(curl,CURLOPT_WRITEDATA,fd);
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
    curl_easy_setopt(curl,CURLOPT_READFUNCTION,my_read_func);
    curl_easy_setopt(curl, CURLOPT_READDATA, output);
    curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, (curl_off_t)filesize);

    //curl_easy_setopt(curl,CURLOPT_TIMEOUT,120);
    curl_easy_setopt(curl,CURLOPT_CONNECTTIMEOUT,30);   // 2017/10/06 add


    if( offset > 0)
        //curl_easy_setopt(curl, CURLOPT_RANGE,range);
        curl_easy_setopt(curl, CURLOPT_RESUME_FROM_LARGE,(curl_off_t)offset);
#if 1

    /* abort if slower than 30 bytes/sec during 30 seconds */
    curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, 30L);
    curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, 30L);

//    start_time = time(NULL);
#endif
    res = curl_easy_perform(curl);

    //curl_easy_getinfo(curl, CURLINFO_SPEED_UPLOAD, &speed_upload);
    //curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &total_time);

    //fprintf(stderr, "Speed: %.3f bytes/sec during %.3f seconds\n", speed_upload, total_time);

    if( res != 0 )
    {
        snprintf(dbg_msg, sizeof(dbg_msg), "curl != 0 , res = %d", res);
        // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
        Cdbg(API_DBG, "%s", dbg_msg);

        fclose(fd);
        fclose(output);
        curl_easy_cleanup(curl);


        // 28 -> connect timeout
        if(res == 28) {
            // 28 -> timeout
            rbu->status = CONNECT_TIMEOUT;
        } else if(res == 6) {
            // 6 -> This error mean that curl can't resolve host name
            rbu->status = CURL_CANNOT_RESOLVE_HOSTNAME;
        } else if(res == 35) {
            // 35 -> CURLE_SSL_CONNECT_ERROR
            rbu->status = CURLE_SSL_CONNECT_ERROR;
        } else {
            rbu->status = res;
        }


        return rbu;


        //return NULL;
    } else {
        
        snprintf(dbg_msg, sizeof(dbg_msg), "res = 0");
        // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
        Cdbg(API_DBG, "%s", dbg_msg);

        //count_call_api(url);
    }


    fclose(fd);
    fclose(output);

    curl_easy_cleanup(curl);

    if(parseDoc1(resume_upload_xml,rbu) == -1)
    {
        Cdbg(API_DBG, "resume_upload_xml parse error\n");
        my_free(rbu);
        return NULL;
    } else {
        snprintf(dbg_msg, sizeof(dbg_msg), "resume_upload_xml parse success\n");
        // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
        Cdbg(API_DBG, "%s", dbg_msg);
    }

    return rbu;


#endif
}

Finishbinaryupload *finishBinaryUpload(Initbinaryupload *ibu)
{
    int status;
    char url[NORMALSIZE];
    Finishbinaryupload *fbu = NULL;

    fbu = getb(Finishbinaryupload);

    if( NULL == fbu ) {

        snprintf(dbg_msg, sizeof(dbg_msg), "create memery error-> Finishbinaryupload");
        // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
        Cdbg(API_DBG, "%s", dbg_msg);

        return NULL;

    } else {

        snprintf(dbg_msg, sizeof(dbg_msg), "create memery successful -> Finishbinaryupload");
        // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
        Cdbg(API_DBG, "%s", dbg_msg);

    }

    memset(fbu,0,sizeof(Finishbinaryupload));


    if(strlen(ibu->latestchecksum) == 0)
    {

        snprintf(dbg_msg, sizeof(dbg_msg), "latestchecksum = 0");
        // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
        Cdbg(API_DBG, "%s", dbg_msg);

        snprintf(url,NORMALSIZE,"https://%s/webrelay/finishbinaryupload/?tk=%s&tx=%s&dis=%s&ai=0"
                ,aaa.webrelay,aaa.token,ibu->transid,sid);
    }
    else
    {

        snprintf(dbg_msg, sizeof(dbg_msg), "latestchecksum != 0");
        // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
        Cdbg(API_DBG, "%s", dbg_msg);

        snprintf(url,NORMALSIZE,"https://%s/webrelay/finishbinaryupload/?tk=%s&tx=%s&dis=%s&lsg=%s&ai=0"
                ,aaa.webrelay,aaa.token,ibu->transid,sid,ibu->latestchecksum);

    }

    snprintf(dbg_msg, sizeof(dbg_msg), "curl url = %s", url);
    // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
    Cdbg(API_DBG, "%s", dbg_msg);

    snprintf(dbg_msg, sizeof(dbg_msg), "finish_upload_xml = %s", finish_upload_xml);
    // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
    Cdbg(API_DBG, "%s", dbg_msg);

    status = sendRequest(finish_upload_xml,url,NULL,NULL,NULL);

    if( status != 0 )
    {
        //handle_error(status,"curl");

        snprintf(dbg_msg, sizeof(dbg_msg), "finishBinaryUpload status != 0, status -> %d", status);
        // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
        Cdbg(API_DBG, "%s", dbg_msg);

        // 28 -> connect timeout
        if(status == 28) {
            fbu->status = CONNECT_TIMEOUT;
        } else if(status == 6) {
            // 6 -> This error mean that curl can't resolve host name
            fbu->status = CURL_CANNOT_RESOLVE_HOSTNAME;
        } else if(status == 35) {
            // 35 -> CURLE_SSL_CONNECT_ERROR
            fbu->status = CURLE_SSL_CONNECT_ERROR;
        } else {
            fbu->status = status;
        }

        return fbu;
    }


    if(parseDoc1(finish_upload_xml,fbu) == -1)
    {
        snprintf(dbg_msg, sizeof(dbg_msg), "finish_upload_xml parse error\n");
        // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
        Cdbg(API_DBG, "%s", dbg_msg);

        my_free(fbu);
        return NULL;

    } else {
        snprintf(dbg_msg, sizeof(dbg_msg), "finish_upload_xml parse success\n");
        // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
        Cdbg(API_DBG, "%s", dbg_msg);
    }

    return fbu;
}

/*
int check_exist_on_server(char *username,char *filename,int parentID)
{
    Propfind *find;
    char finalname[NORMALSIZE];

    memset(finalname,0,sizeof(finalname));

    char *p = strrchr(filename,'/');

    if(p)
    {
        p++;
        strcpy(finalname,p);
    }

    find = checkEntryExisted(username,parentID,finalname,"system.file");

    if(NULL == find)
    {
        Cdbg(API_DBG, "find prop failed\n");
        return -1;
    }
    else if( find->status != 0 )
    {
        handle_error(find->status,"propfind");
        my_free(find);
        return -1;
    }
    else if( !strcmp(find->type,"system.notfound") )
    {
        my_free(find);
        return -1;
    }
    else if( !strcmp(find->type,"system.file") )
    {
        my_free(find);
        return 0;
    }
}*/

int is_local_file_newer(char *filename,int parentID,cmp_item_t *cmp,int fileID)
{
    /*check file is exist on server*/
    Propfind *find;
    Getentryinfo *getinfo;
    int server_mtime = 0 ;
    int local_mtime = 0;
    int local_atime = 0;
    int local_ctime = 0;
    struct stat filestat;
    char finalname[NORMALSIZE];
    long long int max_filesize;

    memset(&filestat,0,sizeof(struct stat));

    if( stat(filename,&filestat) == -1)
    {
        //perror("stat:");
        Cdbg(API_DBG, "is_local_file_newer stat error:%s file not exist\n",filename);
        return S_UPLOAD_DELETED;
    }

    max_filesize = (long long int)max_upload_filesize * 1024 *1024;

    if(filestat.st_size > max_filesize)
    {
        Cdbg(API_DBG, "file size is %lld\n",(long long int)filestat.st_size);
        return S_FILE_TOO_LARGE;
    }

    if(fileID > 0) //server auto backup
    {
        cmp->status = new_file;
        return 0;
    }

    local_mtime = (int)filestat.st_mtime;
    local_atime = (int)filestat.st_atime;
    local_ctime = (int)filestat.st_ctime;

    cmp->size = filestat.st_size;

    char *p = strrchr(filename,'/');

    if(p)
    {
        p++;
        strcpy(finalname,p);
    }


    find = checkEntryExisted(username,parentID,finalname,"system.file");

    if(NULL == find)
    {
        Cdbg(API_DBG, "find prop failed\n");
        return -1;
    }

    if( find->status != 0 )
    {
        handle_error(find->status,"propfind");
        my_free(find);
        return -1;
    }

    if( !strcmp(find->type,"system.file") )
    {
        cmp->id = find->id;
        my_free(find);

        getinfo = getEntryInfo(0,cmp->id);

        if(NULL == getinfo)
        {
            Cdbg(API_DBG, "get entry info failed\n");
            return -1;
        }
        else if( getinfo->status != 0 )
        {
            handle_error(getinfo->status,"getinfo");
            my_free(getinfo);
            return -1;
        }
        else
        {
             server_mtime = atoi(getinfo->attr.lastwritetime);
             //Cdbg(API_DBG, "local_mtime is %d,server_mtime is %d\n",local_mtime,server_mtime);
             my_free(getinfo);
              if(local_mtime > server_mtime)
                  cmp->status = newer;
              else if(local_mtime < server_mtime)
                  cmp->status = older;
              else
                   cmp->status = same;
              //return 0;
         }
    }
    else
    {
        my_free(find);
    }

    if(server_mtime == 0)  //server not existed this file
        cmp->status = new_file;

    return 0;
}

int uploadFile(char *filename,int parentID,char *transid,int InFileID)
{

    Cdbg(API_DBG, "#####upload %s is start,parentID=%d,fileID=%d#####\n",filename,parentID,InFileID);
#ifdef DEBUG
#endif

    char action[128];
    char excep_action[128];
    char error_message[512];
    int res;
    int error_code = -10;
    server_space_full = 0;
    memset(action,0,sizeof(action));
    cmp_item_t c_item = {-2,-1,0};
    int check_res;
    int IsExistList = 0;
    int fileID = InFileID;
    char pre_name[256] = {0};

    snprintf(excep_action,128,"up_excep_fail");
    snprintf(action,128,"uploadfile,%d,%s",parentID,transid);
    
    if(IsAccountFrozen)
    {
        check_res = CheckUserState(username,sergate.gateway);

        if(check_res == S_ACCOUNT_FROZEN)
        {
            add_sync_item(excep_action,filename,up_excep_fail);
            return 0;
        }            

        if(check_res == S_ACCOUNT_CLOSE)
        {
           exit_loop = 1;
           return 0;
        }

        if(check_res == S_AUTH_FAIL)
                return 0;

        IsAccountFrozen = 0;
    }

    check_res = is_local_file_newer(filename,parentID,&c_item,fileID);

    if(check_res != 0)
    {
        if(check_res == S_FILE_TOO_LARGE)
        {
           add_sync_item(excep_action,filename,up_excep_fail);
           snprintf(error_message,512,"%s filesize too large,max upload file size is %dMB",
                    filename,max_upload_filesize);
#ifdef DEBUG
           Cdbg(API_DBG, "%s\n",error_message);
#endif
           write_log(S_ERROR,error_message,"");
        }

        return check_res;
    }

#ifdef DEBUG
    Cdbg(API_DBG, "compare status is %d\n",c_item.status);
#endif

    switch (c_item.status)
    {
    case older: // server file newer
    case newer: //local file newer         
    case same: //server not exist this file
        res = is_exist_case_conflicts(filename,pre_name);

        if(res == -1)
            return -1;

        if(res)
        {
            res = handle_rename(parentID,filename,createfile_action,NULL,1,pre_name);
            return res;
        }
        else
            fileID = c_item.id;
         break;
    case new_file:
         break;
    default:
         break;
    }

    Cdbg(API_DBG, "fileID=%d\n",fileID);
#ifdef DEBUG
#endif

    res = if_server_space_full(filename);

    if(res == 1)
    {
        write_log(S_ERROR,"server space is not enough","");
        server_space_full = 1;
        return S_SERVER_SPACE_FULL;
    }
    else if(res == -1)
    {
        snprintf(error_message,512,"upload %s fail,file is not exist",filename);
        write_log(S_ERROR,error_message,"");
        return S_UPLOAD_DELETED;
    }

#if UP_QUEUE
    queue_entry_t  entry,entry2;

    entry = (queue_entry_t)malloc(sizeof(struct queue_entry));

    if(NULL == entry)
       return -1;
    entry->type = 2;
    entry->id = parentID;
    strcpy(entry->filename,filename);

    queue_enqueue(entry,queue_upload);
#endif
    write_log(S_UPLOAD,"",filename);
    snprintf(error_message,512,"upload %s fail\n",filename);
    Initbinaryupload *ibu;
    Resumebinaryupload *rbu;
    Finishbinaryupload *fbu;

    add_sync_item(excep_action,filename,up_excep_fail);
    res = add_sync_item(action,filename,up_head);
    if(res == -1)
        IsExistList = 1;

    ibu = initBinaryUpload(filename,parentID,transid,fileID);

    if( NULL == ibu)
    {
        write_log(S_ERROR,error_message,"");
        if(!IsExistList)
            write_trans_excep_log(filename,1,"Upload Fail");
        IsSyncError = 1;
        if(upload_only == 1)     //add for upload only mySync
                del_sync_item(excep_action,filename,up_excep_fail);
        return -1;
    }
    else if( ibu->status != 0  )
    {
        error_code = ibu->status;
        handle_error(ibu->status,"initbinaryupload");
        write_log(S_ERROR,error_message,"");

        switch(error_code)
        {
        case S_FILE_TOO_LARGE:
             del_sync_item(action,filename,up_head);
             break;
        case S_NAME_REPEAT:
             break;
        default:
             break;
        }
        
        my_free(ibu);

        return error_code;
    }


    Cdbg(API_DBG, "upload id : %lli !!!!\n",ibu->fileid);
    Cdbg(API_DBG, "upload id : %s !!!!\n",transid);



    if(ibu->fileid > 0)
    {
        del_sync_item(excep_action,filename,up_excep_fail);
        del_sync_item(action,filename,up_head);
#ifdef DEBUG
        Cdbg(API_DBG, "upload %s end!!!!\n",filename);
#endif
        my_free(ibu);
        return 0;
    }


    del_sync_item(action,filename,up_head);
    memset(action,0,sizeof(action));
    snprintf(action,128,"uploadfile,%d,%s",parentID,ibu->transid);
    add_sync_item(action,filename,up_head);

    rbu = resumeBinaryUpload(filename,ibu);

    if( NULL == rbu)
    {
        my_free(ibu);
        write_log(S_ERROR,error_message,"");
        if(!IsExistList)
            write_trans_excep_log(filename,1,"Upload Fail");
        IsSyncError = 1;
        if(upload_only == 1)   //add for upload only mySync
            del_sync_item(excep_action,filename,up_excep_fail);
        return -1;
    }
    else if( rbu->status !=0 )
    {
        error_code = rbu->status;
        handle_error(rbu->status,"resumebinaryupload");
        my_free(ibu);
        my_free(rbu);
        write_log(S_ERROR,error_message,"");
        return error_code;
    }


    fbu = finishBinaryUpload(ibu);

    if(NULL == fbu)
    {
        my_free(ibu);
        my_free(rbu);
        write_log(S_ERROR,error_message,"");
        if(!IsExistList)
            write_trans_excep_log(filename,1,"Upload Fail");
        IsSyncError = 1;
        if(upload_only == 1)    //add for upload only mySync
            del_sync_item(excep_action,filename,up_excep_fail);
        return -1;
    }


    Cdbg(API_DBG, "finishBinaryUpload upload id : %lli !!!!\n",fbu->fileid);

    if(  fbu->status != 0 )
    {
        int res_value = fbu->status;
        handle_error(fbu->status,"finishbinaryupload");
        my_free(ibu);
        my_free(rbu);
        my_free(fbu);
        write_log(S_ERROR,error_message,"");
        return res_value;
    }

#if UP_QUEUE
    if(!queue_empty(queue_upload))
    {
        entry2 = queue_dequeue(queue_upload);
        free(entry2);
    }
#endif

    del_sync_item(action,filename,up_head);
    del_sync_item(excep_action,filename,up_excep_fail);

    my_free(ibu);
    my_free(rbu);
    my_free(fbu);

#ifdef DEBUG
    Cdbg(API_DBG, "upload %s end!!!!\n",filename);
#endif

    return 0;
}


int get_google_token()
{

    char url[NORMALSIZE];
    // char header[NORMALSIZE];
    char postdata[MAXSIZE];

    memset(postdata, 0, sizeof(postdata));


    snprintf(url,NORMALSIZE,"https://www.googleapis.com/oauth2/v4/token");
    // snprintf(header,NORMALSIZE,"authorization: Bearer %s  \nContent-Type: multipart/related; boundary=foo_bar_baz", google_d_conf.access_token);


    // snprintf(dbg_msg, sizeof(dbg_msg), "headers : %s", header);
    // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);

    Cdbg(API_DBG, "Url : %s\n", url);


    strcpy(postdata, "client_id=");
    strcat(postdata, google_d_conf.client_id);
    strcat(postdata, "&client_secret=");
    strcat(postdata, google_d_conf.client_secret);
    strcat(postdata, "&refresh_token=");
    strcat(postdata, google_d_conf.refresh_token);
    strcat(postdata, "&grant_type=refresh_token");

    Cdbg(API_DBG, "postdata strlen : %d", strlen(postdata));
    Cdbg(API_DBG, "postdata : %s", postdata);


    int status = sendGoogleRequest(access_token_json, url, postdata, NULL, NULL, HTTP_REQUEST_POST);
    // int status = sendGoogleRequest(access_token_json, url, postdata, NULL, header, HTTP_REQUEST_POST);

    Cdbg(API_DBG, "sendRequest status : %d\n", status);


    if( status != 0 )
    {

        handle_error(status,"curl");

        // 28 -> connect timeout
        if(status == 28) {
            status = CONNECT_TIMEOUT;
        } else if(status == 6) {
            // 6 -> This error mean that curl can't resolve host name
            status = CURL_CANNOT_RESOLVE_HOSTNAME;
        } else if(status == 35) {
            // 35 -> CURLE_SSL_CONNECT_ERROR
            status = CURLE_SSL_CONNECT_ERROR;
        } 

        return status;


        //return NULL;
    }



    //sleep(10);


    if(process_google_refresh_token(access_token_json, &google_d_conf) == -1)
    {
        Cdbg(API_DBG, "access_token_json parse error\n");
        

        return -1;

    //     int token_status = process_error_xml(init_upload_xml);

    //     if(token_status == 2) {

    //         ibu->status = token_status;
    //         return ibu;

    //     } else {
        
    //         my_free(ibu);

    //         
    //     }

    }


    return 0;
}

#include <libgen.h>

int google_drive_upload(char *filename)
{


    Cdbg(API_DBG, "##### Upload start, filename = %s", filename);


    char url[NORMALSIZE];
    char header[NORMALSIZE];


    //snprintf(url,NORMALSIZE,"https://www.googleapis.com/upload/drive/v3/files?uploadType=media");
    snprintf(url,NORMALSIZE,"https://www.googleapis.com/upload/drive/v3/files?uploadType=multipart");
    snprintf(header,NORMALSIZE,"authorization: Bearer %s  \nContent-Type: multipart/related; boundary=foo_bar_baz", google_d_conf.access_token);


    Cdbg(API_DBG, "headers : %s", header);

    Cdbg(API_DBG, "Url : %s\n", url);


    char * buffer = readFile(filename);

    char postdata[strlen(buffer)+512];

    memset(postdata, 0, sizeof(postdata));

    strcpy(postdata, "--foo_bar_baz\r\nContent-Type: application/json; charset=utf-8\r\n\r\n");
    strcat(postdata, "{\r\n  \"name\": \"");
    strcat(postdata, basename(filename));
    strcat(postdata, "\"\r\n, \"parents\": [\"");
    strcat(postdata, google_d_conf.date_folder_id);
    strcat(postdata, "\"]\r\n");
    // strcat(postdata, "}\r\n\r\n--foo_bar_baz\r\nContent-Type: text/plain\r\n\r\n");
    strcat(postdata, "}\r\n\r\n--foo_bar_baz\r\nContent-Type: application/octet-stream\r\n\r\n");
    strcat(postdata, buffer);
    strcat(postdata, "\r\n--foo_bar_baz--");


    //Cdbg(API_DBG, "postdata strlen : %d", strlen(postdata));
    //Cdbg(API_DBG, "postdata : %s", postdata);



    free(buffer);


    int status = sendGoogleRequest(init_upload_xml, url, postdata, NULL, header, HTTP_REQUEST_POST);
    //int status = sendGoogleRequest(init_upload_xml,url,NULL,NULL,NULL);

    Cdbg(API_DBG, "sendRequest status : %d\n", status);


    if( status != 0 )
    {

        handle_error(status,"curl");

        // 28 -> connect timeout
        if(status == 28) {
            status = CONNECT_TIMEOUT;
        } else if(status == 6) {
            // 6 -> This error mean that curl can't resolve host name
            status = CURL_CANNOT_RESOLVE_HOSTNAME;
        } else if(status == 35) {
            // 35 -> CURLE_SSL_CONNECT_ERROR
            status = CURLE_SSL_CONNECT_ERROR;
        } 

        return status;


        //return NULL;
    }

    //sleep(10);

    // if(parseDoc1(init_upload_xml,ibu) == -1)
    // {
    //     Cdbg(API_DBG, "init_upload_xml parse error\n");
        

    //     int token_status = process_error_xml(init_upload_xml);

    //     if(token_status == 2) {

    //         ibu->status = token_status;
    //         return ibu;

    //     } else {
        
    //         my_free(ibu);

    //         return NULL;
    //     }

    // }


    return 0;
}



int google_drive_file_create(char *filename)
{

    Cdbg(API_DBG, "##### google_drive_file_create, filename = %s", filename);


    char url[NORMALSIZE];
    char header[NORMALSIZE];


    // snprintf(url,NORMALSIZE,"https://www.googleapis.com/upload/drive/v3/files?uploadType=media");
    snprintf(url,NORMALSIZE,"https://www.googleapis.com/drive/v3/files");
    snprintf(header,NORMALSIZE,"authorization: Bearer %s \nAccept: application/json \nContent-Type: application/json", google_d_conf.access_token);


    Cdbg(API_DBG, "headers : %s", header);
    Cdbg(API_DBG, "Url : %s", url);


    char postdata[512];

    memset(postdata, 0, sizeof(postdata));

    strcpy(postdata, "{\r\n  \"name\": \"");
    strcat(postdata, basename(filename));
    strcat(postdata, "\"\r\n, \"mimeType\": \"application/octet-stream\",\r\n");
    strcat(postdata, "\"parents\": [\"");
    strcat(postdata, google_d_conf.date_folder_id);
    strcat(postdata, "\"]\r\n");
    strcat(postdata, "}");


    Cdbg(API_DBG, "postdata strlen : %d", strlen(postdata));
    Cdbg(API_DBG, "postdata : %s", postdata);



    int status = sendGoogleRequest(search_drive_file_json, url, postdata, NULL, header, HTTP_REQUEST_POST);
    //int status = sendGoogleRequest(init_upload_xml,url,NULL,NULL,NULL);

    Cdbg(API_DBG, "sendRequest status : %d\n", status);

    if( status != 0 )
    {

        handle_error(status,"curl");

        // 28 -> connect timeout
        if(status == 28) {
            status = CONNECT_TIMEOUT;
        } else if(status == 6) {
            // 6 -> This error mean that curl can't resolve host name
            status = CURL_CANNOT_RESOLVE_HOSTNAME;
        } else if(status == 35) {
            // 35 -> CURLE_SSL_CONNECT_ERROR
            status = CURLE_SSL_CONNECT_ERROR;
        } 

        return status;


        //return NULL;
    }


    if(process_google_create_folder(search_drive_file_json, &google_d_conf, DRIVE_FILE_ID, DRIVE_FILE_TYPE) == -1)
    {
        Cdbg(API_DBG, "search_drive_file_json parse, [%s] file parse [file id] error\n", filename);

    } else {

        Cdbg(API_DBG, "search_drive_file_json parse, [%s] file parse success", filename);

    }


    return 0;
}


int upload_file_to_google_drive(char *filename)

{
    FILE *fd;
    FILE *output;
    CURL *curl;
    CURLcode res;
    char url[NORMALSIZE];
    char header[NORMALSIZE];
    struct stat filestat;
    int filesize;



    Cdbg(API_DBG, "upload_file_to_google_drive -> filename : %s\n", filename);


    if( stat(filename,&filestat) == -1)
    {
        Cdbg(API_DBG, "upload_file_to_google_drive -> stat error:%s file not exist\n",filename);
        return -1;
    }

    filesize = filestat.st_size;


    Cdbg(API_DBG,  "######filesize is %d#####\n",filesize);

    // snprintf(url,NORMALSIZE,"https://www.googleapis.com/upload/drive/v3/files?uploadType=media");
    snprintf(url,NORMALSIZE,"https://www.googleapis.com/upload/drive/v3/files/%s?uploadType=media", google_d_conf.file_id);
    snprintf(header,NORMALSIZE,"authorization: Bearer %s", google_d_conf.access_token);
    // snprintf(header,NORMALSIZE,"Authorization: Bearer %s\nContent-Type: application/octet-stream\nContent-Length: 24576", google_d_conf.access_token);




    Cdbg(API_DBG, "headers : %s", header);
    Cdbg(API_DBG, "Url : %s", url);


    if( NULL==(fd= fopen(search_drive_file_json,"w")) )
    {

        Cdbg(API_DBG, "fopen 'w' [%s] == NULL ", search_drive_file_json);

        return -1;
    }


    if( NULL==(output= fopen(filename,"rb")) )
    {

        Cdbg(API_DBG, "fopen 'rb' [%s] == NULL ", filename);

        fclose(fd);

        return -1;
    }


    curl = curl_easy_init();


    // curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");

    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);


    struct curl_slist *headers = NULL;

    headers = curl_slist_append(headers, header);

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PATCH");

#if 0
    //curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
    curl_easy_setopt(curl,CURLOPT_NOPROGRESS,0L);
    curl_easy_setopt(curl,CURLOPT_PROGRESSFUNCTION,my_progress_func);
    curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, progress_data);
#endif
    //curl_easy_setopt(curl,CURLOPT_COOKIE,cookies);
    //CURLOPT_SSL_VERIFYHOST
    // curl_easy_setopt(curl,CURLOPT_SSL_VERIFYHOST,0L);
    // curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    //curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
    //curl_easy_setopt(curl, CURLOPT_SSL_SESSIONID_CACHE, 0);

    // curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION,write_data);
    // curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION,my_write_func);

    curl_easy_setopt(curl,CURLOPT_WRITEDATA,fd);

    curl_easy_setopt(curl, CURLOPT_URL, url);
    
    
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
    // curl_easy_setopt(curl,CURLOPT_READFUNCTION,my_read_func);
    curl_easy_setopt(curl, CURLOPT_READDATA, output);
    curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, (curl_off_t)filesize);

    //curl_easy_setopt(curl,CURLOPT_TIMEOUT,120);
    curl_easy_setopt(curl,CURLOPT_CONNECTTIMEOUT,60);   // 2017/10/06 add


    /* abort if slower than 30 bytes/sec during 30 seconds */
    curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, 30L);
    curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, 30L);

    res = curl_easy_perform(curl);


    if( res != 0 )
    {

        Cdbg(API_DBG, "res = %d", res);

        fclose(fd);
        fclose(output);
        curl_easy_cleanup(curl);


        return res;


        //return NULL;
    } else {
        
        Cdbg(API_DBG, "res = 0");
    }


    fclose(fd);
    fclose(output);

    curl_easy_cleanup(curl);


    if(process_google_create_folder(search_drive_file_json, &google_d_conf, DRIVE_FILE_ID, DRIVE_FILE_TYPE) == -1)
    {
        Cdbg(API_DBG, "search_drive_file_json parse, [%s] file parse [file id] error\n", filename);



    } else {
        Cdbg(API_DBG, "search_drive_file_json parse, [%s] file parse success", filename);

    }


    return 0;



}


int google_drive_download(char *filename)
{

    Cdbg(API_DBG, "##### google_drive_download start");


    char url[NORMALSIZE];
    char header[NORMALSIZE];


    if(google_d_conf.file_id != NULL) {

        snprintf(url,NORMALSIZE, "https://www.googleapis.com/drive/v3/files/%s?alt=media", google_d_conf.file_id);

    } else {

        Cdbg(API_DBG, "file_id = NULL");

        return -1;

    }

    snprintf(header,NORMALSIZE,"authorization: Bearer %s", google_d_conf.access_token);


    Cdbg(API_DBG, "headers : %s", header);
    Cdbg(API_DBG, "Url : %s\n", url);


    int status = sendGoogleRequest(filename, url, NULL, NULL, header, HTTP_REQUEST_GET);

    Cdbg(API_DBG, "sendRequest status : %d\n", status);


    if( status != 0 )
    {

        Cdbg(API_DBG, "curl error status : %d", status);

        // 28 -> connect timeout
        if(status == 28) {
            status = CONNECT_TIMEOUT;
        } else if(status == 6) {
            // 6 -> This error mean that curl can't resolve host name
            status = CURL_CANNOT_RESOLVE_HOSTNAME;
        } else if(status == 35) {
            // 35 -> CURLE_SSL_CONNECT_ERROR
            status = CURLE_SSL_CONNECT_ERROR;
        } 

        return status;


        //return NULL;
    }


    return 0;
}




int google_folder_search(char * foldername, char * folderid)
{

    Cdbg(API_DBG, "google_folder_search start, foldername = %s, folderid = %s", foldername, folderid);

    char url[NORMALSIZE];
    char header[NORMALSIZE];


    //snprintf(url,NORMALSIZE,"https://www.googleapis.com/upload/drive/v3/files?uploadType=media");
    if(folderid == NULL) {
        snprintf(url,NORMALSIZE,
            "https://www.googleapis.com/drive/v3/files?corpus=user&q=name=%%27%s%%27%%20and%%20mimeType%%20%%3D%%20%%27application%%2Fvnd.google-apps.folder%%27%%20and%%20trashed%%21%%3Dtrue", foldername);
    } else {
        snprintf(url,NORMALSIZE,
            "https://www.googleapis.com/drive/v3/files?corpus=user&q=name=%%27%s%%27%%20and%%20mimeType%%20%%3D%%20%%27application%%2Fvnd.google-apps.folder%%27%%20and%%20%%27%s%%27%%20in%%20parents%%20and%%20trashed%%21%%3Dtrue", foldername, folderid);
    }

    snprintf(header,NORMALSIZE,"authorization: Bearer %s", google_d_conf.access_token);

    Cdbg(API_DBG, "headers : %s", header);
    Cdbg(API_DBG, "Post Url : %s\n", url);


    int status = sendGoogleRequest(search_drive_file_json, url, NULL, NULL, header, HTTP_REQUEST_GET);

    Cdbg(API_DBG, "sendRequest status : %d\n", status);



    if( status != 0 )
    {

        handle_error(status,"curl");

        // 28 -> connect timeout
        if(status == 28) {
            status = CONNECT_TIMEOUT;
        } else if(status == 6) {
            // 6 -> This error mean that curl can't resolve host name
            status = CURL_CANNOT_RESOLVE_HOSTNAME;
        } else if(status == 35) {
            // 35 -> CURLE_SSL_CONNECT_ERROR
            status = CURLE_SSL_CONNECT_ERROR;
        } 

        return status;

        //return NULL;
    }


    if(process_google_drive_data(search_drive_file_json, &google_d_conf, foldername, DRIVE_FOLDER_TYPE) == -1)
    {
        Cdbg(API_DBG, "search_drive_file_json parse, [%s] folder not exist\n", foldername);
        

        status = google_folder_create(foldername, folderid);

        Cdbg(API_DBG, "google drive [%s] folder cteate status : %d", foldername, status);



        if( status != 0 )
        {

            handle_error(status,"curl");

            // 28 -> connect timeout
            if(status == 28) {
                status = CONNECT_TIMEOUT;
            } else if(status == 6) {
                // 6 -> This error mean that curl can't resolve host name
                status = CURL_CANNOT_RESOLVE_HOSTNAME;
            } else if(status == 35) {
                // 35 -> CURLE_SSL_CONNECT_ERROR
                status = CURLE_SSL_CONNECT_ERROR;
            } 

            return status;


            //return NULL;
        }

    } 

    return 0;
}



int google_file_search(char * filename, char * folderid)
{

    Cdbg(API_DBG, "google_file_search start, filename = %s, folderid = %s", filename, folderid);


    char url[NORMALSIZE];
    char header[NORMALSIZE];

    if(folderid == NULL) {

        Cdbg(API_DBG, "folderid = NULL, date_folder_id = %s", google_d_conf.date_folder_id);

        snprintf(url,NORMALSIZE,
            "https://www.googleapis.com/drive/v3/files?corpus=user&q=name%%20contains%%20%%27%s%%27%%20and%%20mimeType%%21%%3D%%27application%%2Fvnd.google-apps.folder%%27%%20and%%20trashed%%21%%3Dtrue", filename);

        // snprintf(url,NORMALSIZE,
        //     "https://www.googleapis.com/drive/v3/files?corpus=user&q=name%%20contains%%20%%27%s%%27%%20and%%20mimeType%%21%%3D%%27application%%2Fvnd.google-apps.folder%%27", filename);

        
// snprintf(url,NORMALSIZE,
            // "https://www.googleapis.com/drive/v3/files?corpus=user&q=name%%20contains%%20%%27%s%%27%%20and%%20mimeType%%21%%3D%%27application%%2Fvnd.google-apps.folder%%27%%20and%%20mimeType%%3D%%27application%%2Foctet-stream%%27%%20and%%20trashed%%21%%3Dtrue", filename);
    } else {

        snprintf(url,NORMALSIZE,
            "https://www.googleapis.com/drive/v3/files?corpus=user&q=name%%20contains%%20%%27%s%%27%%20and%%20mimeType%%21%%3D%%27application%%2Fvnd.google-apps.folder%%27%%20and%%20mimeType%%3D%%27application%%2Foctet-stream%%27%%20and%%20%%27%s%%27%%20in%%20parents%%20and%%20trashed%%21%%3Dtrue", filename, folderid);
    }

    snprintf(header,NORMALSIZE,"authorization: Bearer %s", google_d_conf.access_token);

    Cdbg(API_DBG, "headers : %s", header);
    Cdbg(API_DBG, "Url : %s\n", url);


    int status = sendGoogleRequest(search_drive_file_json, url, NULL, NULL, header, HTTP_REQUEST_GET);

    Cdbg(API_DBG, "sendRequest status : %d\n", status);


    if( status != 0 )
    {

        handle_error(status,"curl");

        // 28 -> connect timeout
        if(status == 28) {
            status = CONNECT_TIMEOUT;
        } else if(status == 6) {
            // 6 -> This error mean that curl can't resolve host name
            status = CURL_CANNOT_RESOLVE_HOSTNAME;
        } else if(status == 35) {
            // 35 -> CURLE_SSL_CONNECT_ERROR
            status = CURLE_SSL_CONNECT_ERROR;
        } 

        return status;

        //return NULL;
    }


    if(process_google_drive_data(search_drive_file_json, &google_d_conf, DRIVE_FILE_ID, DRIVE_FILE_TYPE) == -1)
    {
        Cdbg(API_DBG, "search_drive_file_json parse, [%s] file parse [file id] error\n", filename);


        return -1;

    } 

    return 0;
}


int google_folder_create(char * foldername, char * folderid)
{

    Cdbg(API_DBG, "##### google_folder_create start, %s", foldername);
    


    char url[NORMALSIZE];
    char header[NORMALSIZE];


    snprintf(url,NORMALSIZE, "https://www.googleapis.com/drive/v3/files");

    snprintf(header,NORMALSIZE,"authorization: Bearer %s", google_d_conf.access_token);


    Cdbg(API_DBG, "headers : %s\nContent-Type: application/json;", header);
    Cdbg(API_DBG, "Url : %s\n", url);


    char postdata[512];

    memset(postdata, 0, sizeof(postdata));

    strcpy(postdata, "{");

    if(folderid != NULL) {
        strcat(postdata, "\"parents\": [ \"");
        strcat(postdata, folderid);
        strcat(postdata, "\" ],");
    }

    strcat(postdata, "\"name\": \"");
    strcat(postdata, foldername);
    strcat(postdata, "\",");
    strcat(postdata, "\"mimeType\": \"application/vnd.google-apps.folder\"");
    strcat(postdata, "}");
    

    Cdbg(API_DBG, "postdata strlen : %d", strlen(postdata));
    Cdbg(API_DBG, "postdata : %s", postdata);


    int status = sendGoogleRequest(create_folder_json, url, postdata, NULL, header, HTTP_REQUEST_POST_JSONTYPE);

    snprintf(dbg_msg, sizeof(dbg_msg), "sendRequest status : %d\n", status);
    // debugLogLevel(DEBUG_LEVEL, DBG_INFO, dbg_msg);
    Cdbg(API_DBG, "%s", dbg_msg);


    if( status != 0 )
    {

        handle_error(status,"curl");

        // 28 -> connect timeout
        if(status == 28) {
            status = CONNECT_TIMEOUT;
        } else if(status == 6) {
            // 6 -> This error mean that curl can't resolve host name
            status = CURL_CANNOT_RESOLVE_HOSTNAME;
        } else if(status == 35) {
            // 35 -> CURLE_SSL_CONNECT_ERROR
            status = CURLE_SSL_CONNECT_ERROR;
        } 

        return status;


        //return NULL;
    }


    if(process_google_create_folder(create_folder_json, &google_d_conf, foldername, DRIVE_FOLDER_TYPE) == -1)
    {
        Cdbg(API_DBG, "create_folder_json parse, [%s] folder not exist\n", foldername);
        

        return -1;

    //     int token_status = process_error_xml(init_upload_xml);

    //     if(token_status == 2) {

    //         ibu->status = token_status;
    //         return ibu;

    //     } else {
        
    //         my_free(ibu);

    //         
    //     }

    } 

    return 0;
}

int upload_file(char *filename,long long parentID,char *transid,int InFileID)
{

    snprintf(dbg_msg, sizeof(dbg_msg), "##### Upload start, filename = %s , parentID=%lli", filename, parentID);
    // debugLogLevel(DEBUG_LEVEL, DBG_INFO, dbg_msg);
    Cdbg(API_DBG, "%s", dbg_msg);


    // Cdbg(API_DBG, "transid=%s, fileID=%d#####\n", transid, InFileID);


    char action[128];
    char excep_action[128];


    int res;
    int error_code = -10;
    server_space_full = 0;
    memset(action,0,sizeof(action));
    
    int IsExistList = 0;
    int fileID = InFileID;

    Initbinaryupload *ibu;

    ibu = initBinaryUpload(filename,parentID,transid,fileID);

    if( NULL == ibu)
    {
        Cdbg(API_DBG, "initBinaryUpload is null (ibu)\n");

        //if(!IsExistList)
            //write_trans_excep_log(filename,1,"Upload Fail");
        //IsSyncError = 1;
        //if(upload_only == 1)     //add for upload only mySync
                //del_sync_item(excep_action,filename,up_excep_fail);
        
        my_free(ibu);
        return -1;

    } 


    if( ibu->status != 0) {

        error_code = ibu->status;

        // handle_error(ibu->status,"initbinaryupload");
        //write_log(S_ERROR,error_message,"");

        Cdbg(API_DBG, "initbinaryupload : error_code = %d\n", error_code);

        if(error_code == CONNECT_TIMEOUT) {
            Cdbg(API_DBG, "initbinaryupload : connect time out\n");
        } else if(error_code == CURL_CANNOT_RESOLVE_HOSTNAME) {
            Cdbg(API_DBG, "initbinaryupload : curl can't resolve host name\n");
        } else if(error_code == CURLE_SSL_CONNECT_ERROR) {
            Cdbg(API_DBG, "initbinaryupload : CURLE_SSL_CONNECT_ERROR\n");
        }

        my_free(ibu);

        return error_code;
    }

    // Cdbg(API_DBG, "upload ibu->fileid : %lli \n",ibu->fileid);
    // Cdbg(API_DBG, "upload ibu->transid : %s \n", ibu->transid);

    if(ibu->fileid > 0) {
        //del_sync_item(excep_action,filename,up_excep_fail);
        //del_sync_item(action,filename,up_head);

        snprintf(dbg_msg, sizeof(dbg_msg), "upload %s end!!!!\n",filename);
        // debugLogLevel(DEBUG_LEVEL, DBG_INFO, dbg_msg);
        Cdbg(API_DBG, "%s", dbg_msg);

        my_free(ibu);

        return 0;
    }


    //del_sync_item(action,filename,up_head);
    memset(action,0,sizeof(action));
    snprintf(action,128,"uploadfile,%lli,%s",parentID,ibu->transid);
    //add_sync_item(action,filename,up_head);


    Resumebinaryupload *rbu;
    rbu = resumeBinaryUpload(filename,ibu);

    if( NULL == rbu)
    {

        snprintf(dbg_msg, sizeof(dbg_msg), "resumeBinaryUpload : rbu == NULL");
        // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
        Cdbg(API_DBG, "%s", dbg_msg);

        my_free(ibu);
        my_free(rbu);

        //write_log(S_ERROR,error_message,"");
        //if(!IsExistList)
            //write_trans_excep_log(filename,1,"Upload Fail");
        //IsSyncError = 1;
        //if(upload_only == 1)   //add for upload only mySync
            //del_sync_item(excep_action,filename,up_excep_fail);
        return -1;

    } else {

        snprintf(dbg_msg, sizeof(dbg_msg), "resumebinaryupload : rbu != NULL");
        // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
        Cdbg(API_DBG, "%s", dbg_msg);

    }


    if(rbu->status != 0)
    {
        error_code = rbu->status;

        snprintf(dbg_msg, sizeof(dbg_msg), "resumebinaryupload : error_code = rbu->status is %d\n", error_code);
        // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
        Cdbg(API_DBG, "%s", dbg_msg);

        if(error_code == CONNECT_TIMEOUT) {
            Cdbg(API_DBG, "resumebinaryupload : connect time out\n");
        } else if(error_code == CURL_CANNOT_RESOLVE_HOSTNAME) {
            Cdbg(API_DBG, "resumebinaryupload : curl can't resolve host name\n");
        } else if(error_code == CURLE_SSL_CONNECT_ERROR) {
            Cdbg(API_DBG, "resumebinaryupload : CURLE_SSL_CONNECT_ERROR\n");
        }


        my_free(ibu);
        my_free(rbu);
        //write_log(S_ERROR,error_message,"");
        return error_code;

    } else {

        snprintf(dbg_msg, sizeof(dbg_msg), "resumebinaryupload : rbu->status is %d\n", rbu->status);
        // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
        Cdbg(API_DBG, "%s", dbg_msg);

    }

    // Cdbg(API_DBG, "resumeBinaryUpload connect ok\n");

    Finishbinaryupload *fbu;
    fbu = finishBinaryUpload(ibu);

    if(NULL == fbu)
    {

        snprintf(dbg_msg, sizeof(dbg_msg), "fbu = null");
        // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
        Cdbg(API_DBG, "%s", dbg_msg);

        my_free(ibu);
        my_free(rbu);
        my_free(fbu);

        //write_log(S_ERROR,error_message,"");
        //if(!IsExistList)
            //write_trans_excep_log(filename,1,"Upload Fail");
        //IsSyncError = 1;
        //if(upload_only == 1)    //add for upload only mySync
            //del_sync_item(excep_action,filename,up_excep_fail);
        return -1;
    }


    if(fbu->status != 0 ) {

        error_code = fbu->status;

        snprintf(dbg_msg, sizeof(dbg_msg), "finishbinaryupload : error_code = fbu->status is %d\n", error_code);
        // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
        Cdbg(API_DBG, "%s", dbg_msg);


        if(error_code == CONNECT_TIMEOUT) {
            Cdbg(API_DBG, "finishbinaryupload : connect time out\n");
        } else if(error_code == CURL_CANNOT_RESOLVE_HOSTNAME) {
            Cdbg(API_DBG, "finishbinaryupload : curl can't resolve host name\n");
        } else if(error_code == CURLE_SSL_CONNECT_ERROR) {
            Cdbg(API_DBG, "finishbinaryupload : CURLE_SSL_CONNECT_ERROR\n");
        }

        my_free(ibu);
        my_free(rbu);
        my_free(fbu);

        return error_code;

    } else {

        snprintf(dbg_msg, sizeof(dbg_msg), "finishBinaryUpload fbu->fileid : %lli, fbu->status %d \n",fbu->fileid, fbu->status);
        // debugLogLevel(DEBUG_LEVEL, DBG_INFO, dbg_msg);
        Cdbg(API_DBG, "%s", dbg_msg);
    }

    my_free(ibu);
    my_free(rbu);
    my_free(fbu);

    return 0;
}




int uploader_config_process(char *config_file) {

    FILE *fp;

    char file_info[128];

    if ((fp = fopen(config_file, "r")) == NULL) {

        Cdbg(API_DBG, "%s open_file_error", config_file);

        return -1;

    }
    
    int i = 0;

    while(fgets(file_info, 128, fp) != NULL)
    {
        i++;

        // remove \n
        if(file_info[strlen(file_info)-1] == '\n') {
            file_info[strlen(file_info)-1] = '\0';
        }

        if(i == 1) {

            memset(config_upload_path,0, sizeof(config_upload_path));

            substr(config_upload_path, file_info, 11, strlen(file_info));

            Cdbg(API_DBG, "config_upload_path : %s, length:%d", config_upload_path, strlen(config_upload_path));

        } else if(i == 2) {
            
            memset(config_download_path,0, sizeof(config_download_path));

            substr(config_download_path, file_info, 13, strlen(file_info));

            Cdbg(API_DBG, "config_download_path : %s, length:%d", config_download_path, strlen(config_download_path));

        }

    }

    fclose(fp);

    return 0;

}

/* str_replace 
* @param {char*} source 
* @param {char*} find 
* @param {char*} rep 
* */  
char *str_replace (char *source, char *find,  char *rep) {

   // find text length
   int find_L=strlen(find);  

   // rep text length
   int rep_L=strlen(rep);  

   // source text length
   int length=strlen(source)+1;  

   // Position the offset
   int gap=0;  
     
   char *result = (char*)malloc(sizeof(char) * length);  
   strcpy(result, source);      
     
   char *former=source;  

   char *location= strstr(former, find);  
     
   // 漸進搜尋欲替換的文字  
   while(location!=NULL){  
       // 增加定位偏移量  
       gap+=(location - former);  
       // 將結束符號定在搜尋到的位址上  
       result[gap]='\0';  
         
       // 計算新的長度  
       length+=(rep_L-find_L);  
       // 變更記憶體空間  
       result = (char*)realloc(result, length * sizeof(char));  
       // 替換的文字串接在結果後面  
       strcat(result, rep);  
       // 更新定位偏移量  
       gap+=rep_L;  
         
       // 更新尚未被取代的字串的位址  
       former=location+find_L;  
       // 將尚未被取代的文字串接在結果後面  
       strcat(result, former);  
         
       // 搜尋文字出現的起始位址指標  
       location= strstr(former, find);  
   }      
  
   return result;  
  
}  


char str_replace2(char *buf, const char *orig, const char *replace)
{
    int olen, rlen;
    char *s, *d;
    char *tmpbuf;

    if (!buf || !*buf || !orig || !*orig || !replace)
        return;

    tmpbuf = malloc(strlen(buf) + 1);
    if (tmpbuf == NULL)
        return;


    olen = strlen(orig);
    rlen = strlen(replace);

    s = buf;
    d = tmpbuf;

    while (*s) {
        if (strncmp(s, orig, olen) == 0) {
            strcpy(d, replace);
            s += olen;
            d += rlen;
        }
        else
            *d++ = *s++;
    }

    *d = '\0';

    strcpy(buf, tmpbuf);
    free(tmpbuf);
}


int aaewsProvisionEdit(struct aaews_provision_conf *ap_conf) {

    char aaews_provision[1024];

    memset(aaews_provision,0, sizeof(aaews_provision));

    strcpy(aaews_provision, "{\n\"timeStamp\":\"");
    strcat(aaews_provision, ap_conf->timeStamp);
    strcat(aaews_provision, "\",\n");

    strcat(aaews_provision, "\"account\":\"");
    strcat(aaews_provision, ap_conf->account);
    strcat(aaews_provision, "\",\n");

    strcat(aaews_provision, "\"password\":\"\",\n");

    strcat(aaews_provision, "\"token\":\"");
    strcat(aaews_provision, ap_conf->token);
    strcat(aaews_provision, "\",\n");


    strcat(aaews_provision, "\"tokenExpireTime\":\"");
    strcat(aaews_provision, ap_conf->tokenExpireTime);
    strcat(aaews_provision, "\",\n");

    strcat(aaews_provision, "\"refreshToken\":\"");
    strcat(aaews_provision, ap_conf->refreshToken);
    strcat(aaews_provision, "\",\n");

    strcat(aaews_provision, "\"deviceName\":\"");
    strcat(aaews_provision, ap_conf->deviceName);
    strcat(aaews_provision, "\",\n");


    strcat(aaews_provision, "\"timeZone\":\"");
    strcat(aaews_provision, ap_conf->timeStamp);
    strcat(aaews_provision, "\"\n}");
 

    printf( "ReWrite aaewsProvision content : \n%s\n", aaews_provision );

    FILE *pFile;

    pFile = fopen( "/tmp/diag_db_cloud/aaewsProvision", "wb" );
    if ( NULL == pFile ){
        printf( "Open aaewsProvision File failure" );
        return -1;
    }else{
        fwrite(aaews_provision,1,strlen(aaews_provision),pFile);

    }
    fclose(pFile);

    system("/bin/setIPCamConfig SystemConfig_aaewsProvision_password \"\"");

    return 0;
}


int systemMacAddressEdit(struct system_conf *s_conf, char * file_name) {

    char system_mac_address[1024];

    memset(system_mac_address,0, sizeof(system_mac_address));

    strcpy(system_mac_address, "{\n\"fwver\":\"");
    strcat(system_mac_address, s_conf->fwver);
    strcat(system_mac_address, "\",\n");

    strcat(system_mac_address, "\"tcode\":\"");
    strcat(system_mac_address, s_conf->tcode);
    strcat(system_mac_address, "\",\n");

    strcat(system_mac_address, "\"sn\":\"");
    strcat(system_mac_address, s_conf->sn);
    strcat(system_mac_address, "\",\n");

    strcat(system_mac_address, "\"apilevel\":\"");
    strcat(system_mac_address, s_conf->apilevel);
    strcat(system_mac_address, "\",\n");

    strcat(system_mac_address, "\"recordplan\":\"");
    strcat(system_mac_address, s_conf->recordplan);
    strcat(system_mac_address, "\",\n");

    strcat(system_mac_address, "\"privateip\":\"");
    strcat(system_mac_address, s_conf->privateip);
    strcat(system_mac_address, "\",\n");

    strcat(system_mac_address, "\"macaddr\":\"");
    strcat(system_mac_address, s_conf->macaddr);
    strcat(system_mac_address, "\",\n");

    strcat(system_mac_address, "\"videomode\":\"");
    strcat(system_mac_address, s_conf->videomode);
    strcat(system_mac_address, "\",\n");


    strcat(system_mac_address, "\"initattachabledevice\":\"");
    strcat(system_mac_address, "on");
    strcat(system_mac_address, "\"\n}");
 
    printf( "ReWrite system_mac_address content : \n%s\n", system_mac_address );

    FILE *pFile;

    pFile = fopen(file_name, "wb" );
    if ( NULL == pFile ){
        printf( "Open File [%s] failure", file_name );
        return -1;
    }else{
        fwrite(system_mac_address, 1, strlen(system_mac_address), pFile);

    }
    fclose(pFile);

    // system("/bin/setIPCamConfig SystemConfig_aaewsProvision_password \"\"");

    return 0;
}

int writeAlarmFile(char * path_file_name) {


    char* txt_file_path = str_replace(path_file_name, ".mp4",".txt");

    Cdbg(API_DBG, "writeAlarmFile -> txt_file_path: '%s'\n", txt_file_path );

    time_t t = time(0);
    char time_new[22];

    memset(time_new,0, sizeof(time_new));

    //strftime( tmp, sizeof(tmp), "%Y%m%d %X %A 本年第%j天 %z",localtime(&t) );
    strftime( time_new, sizeof(time_new), "%Y-%m-%d %X",localtime(&t) );

    Cdbg(API_DBG, "writeAlarmFile -> time_new : %s\n", time_new);


    FILE *pFile;

    //char buffer[50];
    //snprintf(buffer,sizeof(buffer),"{\"create_time\":\"%s\"}",time_new);

    pFile = fopen( txt_file_path,"w" );

    if( NULL == pFile ){

        printf( "writeAlarmFile -> open failure" );

        return 1;

    }else{

        printf( "writeAlarmFile -> open success\n" );
        //fwrite(buffer,1,sizeof(buffer),pFile);
        fprintf(pFile, "{\"create_time\":\"%s\"}",time_new);
    }

    fclose(pFile);

    printf( "writeAlarmFile -> release txt_file_path\n");

    free(txt_file_path);

    return 0;
}


int debugFileWrite() {

    FILE *pFile1;

    pFile1 = fopen( "/tmp/UPLOADER_DEBUG_FILE", "w" );

    if( NULL == pFile1 ){

        Cdbg(API_DBG, "/tmp/UPLOADER_DEBUG_FILE -> open failure" );

        return -1;

    }else{

        Cdbg(API_DBG, "/tmp/UPLOADER_DEBUG_FILE -> open success");

        fprintf(pFile1, "%s", " ");
    }

    fclose(pFile1);



    FILE *pFile2 = fopen( "/tmp/UPLOADER_DEBUG_CONSOLE", "w" );

    if( NULL == pFile2 ){

        Cdbg(API_DBG, "/tmp/UPLOADER_DEBUG_CONSOLE -> open failure" );

        return -1;

    }else{

        Cdbg(API_DBG, "/tmp/UPLOADER_DEBUG_CONSOLE -> open success" );

        fprintf(pFile2, "%s", " ");
    }

    fclose(pFile2);


    return 0;
}


int update_local_file_attr(Fileattribute *attr,char *filename)
{
   struct utimbuf tbuf;
   int server_mtime;
   int server_atime;

   /* change local file time according to server file */
       server_mtime = atoi(attr->lastwritetime);
       server_atime = atoi(attr->lastaccesstime);
       tbuf.actime = (time_t)server_atime;
       tbuf.modtime = (time_t)server_mtime;
       if(utime(filename,&tbuf) == -1)
       {
           Cdbg(API_DBG, "utime %s fail\n",filename);
           return -1;
       }

   return 0;
}

#if WRITE_DOWNLOAD_TEMP_FILE
int write_download_temp_file(const char *action,const char *name)
{
    FILE *fp;
    fp = fopen(down_item_temp_file,"w");
    if(NULL == fp)
    {
        Cdbg(API_DBG, "open %s fail",down_item_temp_file);
        return -1;
    }

    fprintf(fp,"%s,%s\n",action,name);
    fclose(fp);

    return 0;
}
#endif

int check_download_path_exist(const char *filename)
{
    char file_path[512];
    char *p = NULL;
    DIR *pDir = NULL;

    memset(file_path,0,sizeof(file_path));

    p = strrchr(filename,'/');

    if(NULL == p )
    {
        Cdbg(API_DBG, " %s path is not exist\n",filename);
        return -1;
    }

    strncpy(file_path,filename,strlen(filename)-strlen(p));

    pDir = opendir(file_path);

    if(NULL == pDir)
    {
        return -1;
    }

    closedir(pDir);
    return 1;
}

char *get_temp_name(char *fullname)
{
    char *temp_name = NULL;
    char *temp_suffix = ".asus.td";
    int len = 0;
    char path[NORMALSIZE];
    char newfilename[NORMALSIZE];

    memset(path,0,sizeof(path));
    memset(newfilename,0,sizeof(newfilename));
    char *filename = NULL;
    filename =  parse_name_from_path(fullname);
    if(filename == NULL)
       return NULL;
    len = strlen(filename);
    if(len > 247)
    {
        strncpy(path,fullname,strlen(fullname)-len-1);
        strncpy(newfilename,filename,247);
        temp_name = (char *)malloc(sizeof(char)*(strlen(path)+strlen("/")+
                     strlen(newfilename)+strlen(temp_suffix)+1));
        memset(temp_name,0,sizeof(temp_name));
        sprintf(temp_name,"%s/%s%s",path,newfilename,temp_suffix);
    }
    else
    {
        temp_name = (char *)malloc(sizeof(char)*(strlen(fullname)+strlen(temp_suffix)+1));
        memset(temp_name,0,sizeof(temp_name));
        sprintf(temp_name,"%s%s",fullname,temp_suffix);
    }

    my_free(filename);

    return temp_name;
}

int IsEntryDeletedFromServer(int fileID,int isfolder)
{
    Getentryinfo *ginfo = NULL;
    ginfo = getEntryInfo(isfolder,fileID);
    if(ginfo == NULL)
    {
        Cdbg(API_DBG, "getEntryInfo fail\n");
        return -1;
    }

    if(ginfo->status != 0)
    {

        if(ginfo->status == S_FILE_NOT_EXIST)
        {
#ifdef DEBUG
            Cdbg(API_DBG, "file has del from server\n");
#endif
            my_free(ginfo);
            return 1;
        }
        else
        {
            handle_error(ginfo->status,"getEntryInfo before download");
            my_free(ginfo);
            return -1;
        }
    }
    else
    {
        if(ginfo->parent == MyRecycleID)
        {
#ifdef DEBUG
            Cdbg(API_DBG, "file has put to Recycle\n");
#endif
            my_free(ginfo);
            return 1;
        }
        my_free(ginfo);
    }

    return 0;
}

int rename_download_file(char *temp_name,char *fullname,int fileID,Fileattribute *attr,int is_modify)
{

    if(access(fullname,0) == 0 && !is_modify)
    {
        char con_name[1024] = {0};
        char new_filename[256] = {0};
        char *name = NULL;
        Operateentry *oe = NULL;
        char tmp_name[1024] = {0};

        strncpy(tmp_name,fullname,1024);

        while(!exit_loop)
        {

            name = get_confilicted_name(tmp_name,0);

            if(name ==  NULL)
            {
                Cdbg(API_DBG, "handle_local_confilict_file fail\n");
                return -1;
            }
           if(access(name,F_OK) == 0)
           {
               memset(tmp_name,0,sizeof(tmp_name));
               snprintf(tmp_name,NORMALSIZE,"%s",name);
               my_free(name);
           }
           else
               break;
         }
 
        strncpy(con_name,name,1024);
        my_free(name);

        name = parse_name_from_path(con_name);
        strncpy(new_filename,name,256);
        my_free(name);

        while(!exit_loop)
        {
            oe = renameEntry(username,fileID,0,new_filename,0);
            if(oe == NULL)
            {
                check_network_state();
                continue;
            }
            if(oe->status != 0)
            {
                handle_error(oe->status,"renameEntry");
                my_free(oe);
                return -1;
            }

            my_free(oe);

            if(rename(temp_name,con_name) == -1)
            {
                Cdbg(API_DBG, "rename %s to %s fail",temp_name,fullname);
                return -1;
            }

            if( update_local_file_attr(attr,con_name) == -1)
                handle_error(S_UPDATE_ATTR_FAIL,"update file attr");

            break;
        }

    }
    else
    {
        if(rename(temp_name,fullname) == -1)
        {
            Cdbg(API_DBG, "rename %s to %s fail",temp_name,fullname);
            return -1;
        }

        if( update_local_file_attr(attr,fullname) == -1)
            handle_error(S_UPDATE_ATTR_FAIL,"update file attr");
    }

    return 0;
}

/*int is_download_completed(char *temp_name,long long filesize)
{
    struct stat buf;

    if( stat(temp_name,&buf) == -1)
    {
        Cdbg(API_DBG, " is_download_completed stat error:%s file not exist\n",temp_name);
        return 0;
    }

    Cdbg(API_DBG, "st_size=%lld,filesize=%lld\n",buf.st_size,filesize);
    if(buf.st_size == filesize)
        return 1;

    return 0;

}*/

int downloadFile(int fileID,char *filename,long long int size,int ismodify,Fileattribute *attr)
{
#if DOWN_QUEUE
    queue_entry_t  entry,entry2;

    entry = (queue_entry_t)malloc(sizeof(struct queue_entry));

    if(NULL == entry)
        return  -1;
    entry->type = 1;
    entry->id = fileID;
    strcpy(entry->filename,filename);
    entry->size = size;

    queue_enqueue(entry,queue_download);
#endif
    local_space_full = 0;
    char *temp_name = NULL;

#ifdef DEBUG
    Cdbg(API_DBG, "download %s is start,fildID is %d,size is %lld\n",filename,fileID,size);
#endif

    char action[256];
    long http_code = -10;

    memset(action,0,sizeof(action));
    snprintf(action,256,"downloadfile,%d,%lld",fileID,size);
   
    char url[NORMALSIZE];
    CURL *curl;
    CURLcode res;
    FILE *fd;
    char cookies[NORMALSIZE];
    long long int disk_free_size;
    struct stat filestat;
    int file_exist =  -1;
    int local_file_len = -1;
    char error_message[NORMALSIZE];
    int status;
    int IsExistList = 0 ;

    status = IsEntryDeletedFromServer(fileID,0);

    if(status == 1)
        return 0;

    if(status == -1)
        return -1;

    disk_free_size = check_disk_space(sync_path);

    if( disk_free_size <= size )
    {
            handle_error(S_LOCAL_SPACE_FULL,"download");
            write_log(S_ERROR,"usb disk space is not enough","");
            local_space_full = 1;
            return -1;
    }

    temp_name = get_temp_name(filename);

    if(NULL == temp_name)
    {
        handle_error(S_MEMORY_FAIL,"get temp name");
        return -1;
    }

    status = add_sync_item(action,temp_name,down_head);
    if(status == -1)
        IsExistList = 1;

    if( stat(temp_name,&filestat) == -1)
    {
        file_exist = 0 ;
        local_file_len = 0;
    }
    else
    {
        file_exist = 1;
        local_file_len = filestat.st_size;
        if(filestat.st_size == size)
        {
            Cdbg(API_DBG, "exist download complete file\n");
            rename_download_file(temp_name,filename,fileID,attr,ismodify);
            del_sync_item(action,temp_name,down_head);
            my_free(temp_name);
            return 0;
        }
    }

#if WRITE_DOWNLOAD_TEMP_FILE
    write_download_temp_file(action,filename);
#endif
    write_log(S_DOWNLOAD,"",filename);

    if(file_exist && ismodify != 1 )
    {
       fd = fopen(temp_name,"ab");
    }
    else
    {
       fd = fopen(temp_name,"wb");
    }


    if(NULL == fd)
    {
        Cdbg(API_DBG, "fopen %s fail,dir is not exist??\n",filename);
        my_free(temp_name);
        return -1;
    }

    snprintf(cookies,NORMALSIZE,"OMNISTORE_VER=1_0; path=/;sid=%s;v=%s",sid,VERSION);
    snprintf(url,NORMALSIZE,"https://%s/webrelay/directdownload/?tk=%s&fi=%d&pv=0&u=&of=&rn=&dis=%s"
            ,aaa.webrelay,aaa.token,fileID,sid);
    curl = curl_easy_init();
#if 0
    //curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
    curl_easy_setopt(curl,CURLOPT_NOPROGRESS,0L);
    curl_easy_setopt(curl,CURLOPT_PROGRESSFUNCTION,my_progress_func);
    curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, progress_data);
#endif
    /* resume download file*/
    //curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
    if(file_exist && ismodify != 1)
        curl_easy_setopt(curl, CURLOPT_RESUME_FROM_LARGE,(curl_off_t)local_file_len);
    curl_easy_setopt(curl,CURLOPT_COOKIE,cookies);
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);  
    curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION,my_write_func);
    curl_easy_setopt(curl,CURLOPT_READFUNCTION,my_read_func);
    curl_easy_setopt(curl,CURLOPT_LOW_SPEED_LIMIT,1);
    curl_easy_setopt(curl,CURLOPT_LOW_SPEED_TIME,30);
    curl_easy_setopt(curl,CURLOPT_WRITEDATA,fd);
    //curl_easy_setopt(curl,CURLOPT_ERRORBUFFER,curl_err_msg);

    res = curl_easy_perform(curl);

    if( res != 0 )
    {
        snprintf(error_message,NORMALSIZE,"download %s fail",filename);
        write_log(S_ERROR,error_message,"");
        if(!IsExistList)
            write_trans_excep_log(filename,1,"Download Fail");
        IsSyncError = 1;
        handle_error(res,"curl");
        fclose(fd);
        curl_easy_cleanup(curl);
        my_free(temp_name);
        return -1;
    }
    else {
        //count_call_api(url);
    }


    fclose(fd);
#if 1
    curl_easy_getinfo(curl,CURLINFO_RESPONSE_CODE,&http_code);

#endif
    curl_easy_cleanup(curl);


#if DOWN_QUEUE
    if(!queue_empty(queue_download))
    {
        entry2 = queue_dequeue(queue_download);
        free(entry2);
    }
#endif

#if WRITE_DOWN_TEMP_FILE
    remove(down_item_temp_file);
#endif
    del_sync_item(action,temp_name,down_head);

#ifdef DEBUG
    //print_all_sync_item(down_head);
#endif

#ifdef DEBUG
    Cdbg(API_DBG, "download %s is end\n",filename);
#endif

    if(http_code > 300 && http_code < 600)
    {
        Cdbg(API_DBG, "download file http code is %ld\n",http_code);
        if(remove(temp_name) == -1)
        {
           Cdbg(API_DBG, "remove downlaod excep %s fail \n",temp_name);
        }
    }
    else
    {
        rename_download_file(temp_name,filename,fileID,attr,ismodify);
    }

    my_free(temp_name);
    return 0;
}

#if 0
int getResizedPhoto(int pfd,int st,int pv)
{
    int status;
    char url[NORMALSIZE];
    char *encode = NULL;
    char query_string[MAXSIZE];
    char query_raw[NORMALSIZE];
    //struct curl_slist *headers = NULL;
    char header[NORMALSIZE];

    snprintf(header,NORMALSIZE,"Last-Modified:Tue, 17 Nov 2009 07:13:19 GMT,ETag:\"1258441999687\"");
    snprintf(query_raw,NORMALSIZE,"pfd=%d,st=%d,pv=%d",pfd,st,pv);
    //Cdbg(API_DBG, "######## query string is %s #########\n",query_string);
    //sprintf(query_string,"%s",oauth_encode_base64(0,query_string));
    encode = oauth_encode_base64(0,query_raw);

    if(encode == NULL)
   {
       handle_error(S_ENCODE_BASE64_FAIL,"getResizePhoto");
       return -1;
   }

    snprintf(query_string,MAXSIZE,"%s.jpg",encode);

    //sprintf(url,"https://%s/webrelay/getresizedphoto/%s?tk=%s&dis=%s&ecd=1"
            //,aaa.webrelay,query_string,aaa.token,sid);
    snprintf(url,NORMALSIZE,"https://%s/webrelay/getresizedphoto/%s/%s?dis=%s&ecd=1"
            ,aaa.webrelay,aaa.token,query_string,sid);

    status = sendRequest(get_resize_photo_xml,url,NULL,NULL,header);

    my_free(encode);

    if( status != 0 )
    {
        handle_error(status,"curl");
        return -1;
    }


    return 0;
}

int getFullTextCompanion(int fi,int pv,int k)
{
    int status;
    char url[NORMALSIZE];
    char query_raw[NORMALSIZE];
    char *encode = NULL;
    char query_string[MAXSIZE];
    //struct curl_slist *headers = NULL;
    char header[NORMALSIZE];

    /*obtain token and inforelay*/
    //parseDoc(TOKEN_XML);

    snprintf(header,NORMALSIZE,"Last-Modified:Tue, 17 Nov 2009 07:13:19 GMT,ETag:\"1258441999687\"");
    snprintf(query_raw,NORMALSIZE,"fi=%d,pv=%d,k=%d",fi,pv,k);
    //sprintf(query_string,"%s",oauth_encode_base64(0,query_string));
    encode = oauth_encode_base64(0,query_raw);

    if(encode == NULL)
   {
       handle_error(S_ENCODE_BASE64_FAIL,"getFullTextCompanion");
       return -1;
   }

   snprintf(query_string,MAXSIZE,"%s.txt",encode);

    //sprintf(url,"https://%s/webrelay/getfulltextcompanion/%s?tk=%s&dis=%s&ecd=1"
            //,aaa.webrelay,query_string,aaa.token,sid);
    snprintf(url,MAXSIZE,"https://%s/webrelay/getfulltextcompanion/%s/%s?dis=%s&ecd=1"
            ,aaa.webrelay,aaa.token,query_string,sid);

    status = sendRequest(get_full_txt_xml,url,NULL,NULL,header);

    my_free(encode);

    if( status != 0 )
    {
        handle_error(status,"curl");
        return -1;
    }


    return 0;
}

int getVideoSnapshot(int fi,int pv)
{
    int status;
    char url[NORMALSIZE];
    char query_raw[NORMALSIZE];
    char query_string[MAXSIZE];
    char *encode = NULL;

    snprintf(query_raw,NORMALSIZE,"fi=%d,pv=%d",fi,pv);
    //sprintf(query_string,"%s",oauth_encode_base64(0,query_string));

    encode = oauth_encode_base64(0,query_raw);

    if(encode == NULL)
   {
       handle_error(S_ENCODE_BASE64_FAIL,"getVideoSnapshot");
       return -1;
   }
    snprintf(query_string,MAXSIZE,"%s.jpg",encode);

    //sprintf(url,"https://%s/webrelay/getvideosnapshot/%s?tk=%s&dis=%s&ecd=1"
            //,aaa.webrelay,query_string,aaa.token,sid);
    snprintf(url,NORMALSIZE,"https://%s/webrelay/getvideosnapshot/%s/%s?dis=%s&ecd=1"
            ,aaa.webrelay,aaa.token,query_string,sid);


    status = sendRequest(get_video_snapshot_xml,url,NULL,NULL,NULL);

    my_free(encode);

    if( status != 0 )
    {
        handle_error(status,"curl");
        return -1;
    }


    return 0;
}
#endif

int handle_error(int code,char *type)
{
    Cdbg(API_DBG, "code is %d,type is %s\n",code,type);
    char error_message[NORMALSIZE];

    memset(error_message,0,sizeof(error_message));


    switch (code)
    {
    case S_AUTH_FAIL:

        if( strcmp(type,"gateway") == 0)
        {
#ifdef DEBUG
            Cdbg(API_DBG, "username is error \n");
            strcpy(error_message,"username is error");
            //write_system_log("error","username is error");
#endif
        }
        else
        {
#ifdef DEBUG
            Cdbg(API_DBG, "auth failed ,please check username and password\n");
            strcpy(error_message,"auth failed ,please check username and password");
            //write_system_log("error","auth failed ,please check username and password");
#endif
        }
        break;

    case CURLE_COULDNT_RESOLVE_HOST:
#ifdef DEBUG
        Cdbg(API_DBG, "can't resolve host,please check connection \n");
        strcpy(error_message,"can't resolve host,please check connection");
        //write_system_log("error","can't resolve host,please check connection");
#endif
        break;
    case CURLE_COULDNT_CONNECT:
#ifdef DEBUG
        Cdbg(API_DBG, "can't connect to host,please check connection \n");
        strcpy(error_message,"can't connect to host,please check connection");
        //write_system_log("error","can't connect to host,please check connection");
#endif
        break;
    case CURLE_PARTIAL_FILE:
        Cdbg(API_DBG, "partial file ,please check transe file size \n");
        break;
    case CURLE_QUOTE_ERROR:
        Cdbg(API_DBG, "quote error  \n");
        break;
    case CURLE_HTTP_RETURNED_ERROR:
        Cdbg(API_DBG, "http return error code is %d  \n",code);
        break;
    case CURLE_OPERATION_TIMEDOUT:
        Cdbg(API_DBG, "connect time out  \n");
        break;
    case S_LOCAL_SPACE_FULL:
#ifdef DEBUG
        Cdbg(API_DBG, "local space is not enough \n");
        strcpy(error_message,"local space is not enough");
#endif
        break;
    case S_SERVER_SPACE_FULL:
#ifdef DEBUG
        Cdbg(API_DBG, "server space is not enough \n");
        strcpy(error_message,"server space is not enough");
#endif
        break;
    case S_MEMORY_FAIL:
#ifdef DEBUG
        Cdbg(API_DBG, "create dynamic memory fail \n");
#endif
        strcpy(error_message,"create dynamic memory fail");
        break;

    case S_MKDIR_FAIL:
#ifdef DEBUG
        Cdbg(API_DBG, "create folder error,please check disk can write or dir has exist???");
#endif
        //strcpy(error_message,"create folder error,please check disk can write???");
        break;

    case S_UPDATE_ATTR_FAIL:
#ifdef DEBUG
        Cdbg(API_DBG, "update attr fail\n");
#endif
        strcpy(error_message,"update file attr fail");
        break;

    case S_OPENDIR_FAIL:
#ifdef DEBUG
        Cdbg(API_DBG, "open dir fail\n");
#endif
        strcpy(error_message,"open dir fail");
        break;

    default:
#ifdef DEBUG
        snprintf(error_message,NORMALSIZE,"code is %d,type is %s",code,type);
#endif
        break;
    }
#if SYSTEM_LOG
    write_system_log("error",error_message);
#endif
    //write_log(S_ERROR,error_message,"");

    return 0;
}

int getParentID(char *path)
{
    if(NULL == path)
        return -1;

    char *cut_path;
    char parse_path[512];
    int parentID = -5;
    Propfind *pfind;
    int sync_path_len;
    const char *split = "/";
    char *p2;

    memset(parse_path,0,sizeof(parse_path));
    strcpy(parse_path,path);

    if( !strcmp(parse_path,sync_path) ) // path is sync root path
    {
        return MySyncFolder;
    }

    sync_path_len = strlen(sync_path);
    cut_path = parse_path;
    cut_path = cut_path + sync_path_len;
    cut_path++ ; // pass first '/'

    p2 = strtok(cut_path,split);
    int j=0;
    while(p2!=NULL)
    {
        if(j == 0)
            parentID = MySyncFolder;

#ifdef DEBUG
        //Cdbg(API_DBG, "check path is %s\n",p2);
#endif

        pfind = checkEntryExisted(username,parentID,p2,"system.folder");

        if(NULL == pfind)
        {
            return -1;
        }

        if(pfind->status != 0)
        {
            my_free(pfind);
            return -1;
        }

        if(strcmp(pfind->type,"system.notfound") == 0)
        {
            my_free(pfind);
            return -2;
        }      

        parentID = pfind->id;

        my_free(pfind);
        j++;
        p2 = strtok(NULL,split);
    }

   return parentID;
}

long int check_server_space(char *username)
{
    Getinfo *gi = NULL;
    long long int size;

    gi = getInfo(username,sergate.gateway);

    if(NULL == gi)
        return -1;
    if(gi->status != 0)
    {
        handle_error(gi->status,"getinfo");
        my_free(gi);
        return -1;
    }

    size = gi->freecapacity;

    my_free(gi);
    return size;

}

int get_server_space(char *username,server_capacity *cap)
{
    Getinfo *gi = NULL;

    gi = getInfo(username,sergate.gateway);


    if(NULL == gi)
        return -1;
    if(gi->status != 0)
    {
        handle_error(gi->status,"getinfo");
        my_free(gi);
        return -1;
    }

    cap->total = gi->usedcappacity + gi->freecapacity;
    cap->used = gi->usedcappacity;
    my_free(gi);
    return 0;
}

int write_log(int status, char *message, char *filename)
{
    Log_struc log_s;
    FILE *fp;
    int mount_path_length;
    server_capacity cap;

    if(status == S_SYNC || status == S_UPLOAD)
    {
        if(get_server_space(username,&cap) != -1)
        {
            pre_cap.total = cap.total;
            pre_cap.used = cap.used;
        }
        else
        {
            cap.total = pre_cap.total;
            cap.used = pre_cap.used;
        }
    }

    mount_path_length = strlen(mount_path);

    memset(&log_s,0,LOG_SIZE);

    log_s.status = status;

    fp = fopen(general_log,"w");

    if(fp == NULL)
    {
        Cdbg(API_DBG, "open %s error\n",general_log);
        return -1;
    }

    if(log_s.status == S_ERROR)
    {
        //Cdbg(API_DBG, "******** status is ERROR *******\n");
        strcpy(log_s.error,message);
        fprintf(fp,"STATUS:%d\nERR_MSG:%s\nTOTAL_SPACE:%u\nUSED_SPACE:%u\n",log_s.status,log_s.error,pre_cap.total,pre_cap.used);

    }
    else if(log_s.status == S_NEEDCAPTCHA)
    {
        //strcpy(log_s.error,message);
        fprintf(fp,"STATUS:%d\nERR_MSG:%s\nTOTAL_SPACE:%u\nUSED_SPACE:%u\nCAPTCHA_URL:%s\n",
                status,message,pre_cap.total,pre_cap.used,filename);
    }
    else if(log_s.status == S_DOWNLOAD)
    {
        //Cdbg(API_DBG, "******** status is DOWNLOAD *******\n",log_s.status);
        strcpy(log_s.path,filename);
        fprintf(fp,"STATUS:%d\nMOUNT_PATH:%s\nFILENAME:%s\nTOTAL_SPACE:%u\nUSED_SPACE:%u\n",
                log_s.status,mount_path,log_s.path+mount_path_length,pre_cap.total,pre_cap.used);
    }
    else if(log_s.status == S_UPLOAD)
    {
        //Cdbg(API_DBG, "******** upload status is UPLOAD *******\n",log_s.status);
        strcpy(log_s.path,filename);
        fprintf(fp,"STATUS:%d\nMOUNT_PATH:%s\nFILENAME:%s\nTOTAL_SPACE:%u\nUSED_SPACE:%u\n",log_s.status,mount_path,log_s.path+mount_path_length,cap.total,cap.used);
    }
    else
    {
        /*
        if (log_s.status == S_INITIAL)
            Cdbg(API_DBG, "******** other status is INIT *******\n",log_s.status);
        else
            Cdbg(API_DBG, "******** other status is SYNC *******\n",log_s.status);
        */
        if (log_s.status == S_SYNC)
            fprintf(fp,"STATUS:%d\nTOTAL_SPACE:%u\nUSED_SPACE:%u\n",log_s.status,cap.total,cap.used);
        else
            fprintf(fp,"STATUS:%d\nTOTAL_SPACE:%u\nUSED_SPACE:%u\n",log_s.status,pre_cap.total,pre_cap.used);

    }

    fclose(fp);
    return 0;
}

int write_finish_log()
{
    if( local_space_full || server_space_full)
        return 0;

    if(IsSyncError)
    {
        write_log(S_ERROR,"Local synchronization is not entirely successful,failure information,please refer to errlog","");
        IsSyncError = 0 ;
    }
    else
        write_log(S_SYNC,"","");

    return 0;
}

int write_system_log(char *action,char *message)
{
    struct stat buf;
    FILE *fp;
    time_t rawtime;
    time(&rawtime);
    struct tm *timeinfo;
    timeinfo = localtime(&rawtime);
    char *ctime = asctime(timeinfo);

    if( ctime[ strlen(ctime)-1 ] == '\n')
        ctime[ strlen(ctime)-1 ] = '\0';

    if( stat(system_log,&buf) == -1)
    {
        fp = fopen(system_log,"w");
    }
    else
    {
        fp = fopen(system_log,"a");
    }

     if(NULL == fp)
     {
         Cdbg(API_DBG, "open %s failed\n",system_log);
         return -1;
     }

     fprintf(fp,"%s asuswebstorage [action]:%s [message]:%s\n",ctime,action,message);
     fclose(fp);

     /*wiret temp file*/
#if 0
     fp = fopen(temp_file,"w");

     if(NULL == fp)
     {
         Cdbg(API_DBG, "open %s failed\n",temp_file);
         return -1;
     }

     fprintf(fp,"%s\n",filename);

     fclose(fp);
#endif

     //Cdbg(API_DBG, "wirte system log end\n");

     return 0;

}

int write_confilicted_log(char *prename, char *confilicted_name)
{
    struct stat buf;
    FILE *fp;

    if( stat(confilicted_log,&buf) == -1)
    {
        fp = fopen(confilicted_log,"w");
    }
    else
    {
        fp = fopen(confilicted_log,"a");
    }

     if(NULL == fp)
     {
         Cdbg(API_DBG, "open %s failed\n",system_log);
         return -1;
     }

     fprintf(fp,"%s is download from server,%s is local file and rename from %s\n",prename,confilicted_name,prename);
     fclose(fp);

     return 0;
}

int write_trans_excep_log(char *fullname,int type,char *msg)
{
    FILE *fp = 0;
    char ctype[16] = {0};

    if(type == 1)
        strcpy(ctype,"Error");
    else if(type == 2)
        strcpy(ctype,"Info");
    else if(type == 3)
        strcpy(ctype,"Warning");

    if(access(trans_excep_file,0) == 0)
        fp = fopen(trans_excep_file,"a");
    else
        fp = fopen(trans_excep_file,"w");


    if(fp == NULL)
    {
        Cdbg(API_DBG, "open %s fail\n",trans_excep_file);
        return -1;
    }

    fprintf(fp,"TYPE:%s\nUSERNAME:%s\nFILENAME:%s\nMESSAGE:%s\n",ctype,username,fullname,msg);
    fclose(fp);
    return 0;
}

#if 1
int sync_all_item(char *dir,int parentID)
{
    struct dirent* ent = NULL;
    Createfolder *cf;
    char fullname[NORMALSIZE];
    int fail_flag = 0;
    char error_message[NORMALSIZE];
    int res_value = -10;

    DIR *dp = opendir(dir);

    if(dp == NULL)
    {
       snprintf(error_message,NORMALSIZE,"opendir %s fail",dir);
       handle_error(S_OPENDIR_FAIL,error_message);
       fail_flag = 1;
       return S_UPLOAD_DELETED;
    }

    while (NULL != (ent=readdir(dp)))
    {
        int id;
        int status = -10;

        if(!strcmp(ent->d_name,".") || !strcmp(ent->d_name,".."))
            continue;

        memset(fullname,0,sizeof(fullname));
        memset(error_message,0,sizeof(error_message));
        //memset(&createfolder,0,sizeof(Createfolder));

        snprintf(fullname,NORMALSIZE,"%s/%s",dir,ent->d_name);

//        if(upload_only && receve_socket)   //2014/10/14 gauss mark for rame folder when confilict
//            return -1;
        if( test_if_dir(fullname) == 1)
        {
            cf = createFolder(username,parentID,0,fullname);
            if(NULL == cf)
            {
                Cdbg(API_DBG, "sync_all_item function create folder fail\n");
                fail_flag = 1;
                continue;
            }
            else if(cf->status != 0)
            {
                 fail_flag = 1;
                 snprintf(error_message,NORMALSIZE,"createfolder %s fail on server",ent->d_name);
                 handle_error(cf->status,error_message);
                 res_value = handle_createfolder_fail_code(cf->status,parentID,dir,fullname);
                 if(res_value != 0)
                      fail_flag = 1;
                 my_free(cf);
                 continue;
            }
            else if(cf->status == 0)
            {
                //add_sync_item("createfolder",fullname,dragfolder_recursion_head);
                id = cf->id;
                my_free(cf);
                sync_all_item(fullname,id);
            }
        }
        else
        {
            status = uploadFile(fullname,parentID,NULL,0);
                if(status != 0)
                {
                   Cdbg(API_DBG, "upload %s failed\n",fullname);
                   res_value = handle_upload_fail_code(status,parentID,fullname,dir);
                   if(res_value != 0)
                        fail_flag = 1;
                }
        }
    }

    closedir(dp);

    return (fail_flag == 1) ? -1 : 0;
}


#endif

int sync_all_item_uploadonly(char *dir,int parentID)
{
    struct dirent* ent = NULL;
    Createfolder *cf;
    char fullname[NORMALSIZE];
    int fail_flag = 0;
    char error_message[NORMALSIZE];
    int res_value = -10;

    DIR *dp = opendir(dir);

    if(dp == NULL)
    {
       snprintf(error_message,NORMALSIZE,"opendir %s fail",dir);
       handle_error(S_OPENDIR_FAIL,error_message);
       fail_flag = 1;
       return S_UPLOAD_DELETED;
    }

    while (NULL != (ent=readdir(dp)))
    {
        int id;
        int status = -10;

        if(!strcmp(ent->d_name,".") || !strcmp(ent->d_name,".."))
            continue;

        memset(fullname,0,sizeof(fullname));
        memset(error_message,0,sizeof(error_message));
        snprintf(fullname,NORMALSIZE,"%s/%s",dir,ent->d_name);

        if(upload_only && receve_socket)
            return -1;

        if( test_if_dir(fullname) == 1)
        {
            cf = createFolder(username,parentID,0,fullname);
            if(NULL == cf)
            {
                Cdbg(API_DBG, "sync_all_item function create folder fail\n");
                fail_flag = 1;
                continue;
            }
            else if(cf->status != 0)
            {
                 fail_flag = 1;
                 snprintf(error_message,NORMALSIZE,"createfolder %s fail on server",ent->d_name);
                 handle_error(cf->status,error_message);
                 res_value = handle_createfolder_fail_code(cf->status,parentID,dir,fullname);
                 if(res_value != 0)
                      fail_flag = 1;
                 my_free(cf);
                 continue;
            }
            else if(cf->status == 0)
            {
                id = cf->id;
                my_free(cf);
                sync_all_item(fullname,id);
            }
        }
        else
        {
            status = uploadFile(fullname,parentID,NULL,0);
                if(status != 0)
                {
                   Cdbg(API_DBG, "upload %s failed\n",fullname);
                   res_value = handle_upload_fail_code(status,parentID,fullname,dir);
                   if(res_value != 0)
                        fail_flag = 1;
                }
        }
    }
    closedir(dp);
    return (fail_flag == 1) ? -1 : 0;
}

int add_all_download_only_socket_list(char *cmd,const char *dir)
{   
    struct dirent* ent = NULL;
    char fullname[NORMALSIZE];
    int fail_flag = 0;
    char error_message[NORMALSIZE];

    DIR *dp = opendir(dir);

    if(dp == NULL)
    {
       snprintf(error_message,NORMALSIZE,"opendir %s fail",dir);
       handle_error(S_OPENDIR_FAIL,error_message);
       fail_flag = 1;
       return -1;
    }

    add_sync_item(cmd,dir,download_only_socket_head);

    while (NULL != (ent=readdir(dp)))
    {

        if(!strcmp(ent->d_name,".") || !strcmp(ent->d_name,".."))
            continue;

        memset(fullname,0,sizeof(fullname));
        memset(error_message,0,sizeof(error_message));

        snprintf(fullname,NORMALSIZE,"%s/%s",dir,ent->d_name);

        if( test_if_dir(fullname) == 1)
        {
            add_all_download_only_socket_list("createfolder",fullname);
        }
        else
        {
            add_sync_item("createfile",fullname,download_only_socket_head);
        }
    }

    closedir(dp);

    return (fail_flag == 1) ? -1 : 0;
}

int add_all_download_only_dragfolder_socket_list(const char *dir)
{
    struct dirent* ent = NULL;
    char fullname[NORMALSIZE];
    int fail_flag = 0;
    char error_message[NORMALSIZE];

    DIR *dp = opendir(dir);

    if(dp == NULL)
    {
       snprintf(error_message,NORMALSIZE,"opendir %s fail",dir);
       handle_error(S_OPENDIR_FAIL,error_message);
       fail_flag = 1;
       return -1;
    }

    while (NULL != (ent=readdir(dp)))
    {

        if(!strcmp(ent->d_name,".") || !strcmp(ent->d_name,".."))
            continue;

        memset(fullname,0,sizeof(fullname));
        memset(error_message,0,sizeof(error_message));

        snprintf(fullname,NORMALSIZE,"%s/%s",dir,ent->d_name);

        if( test_if_dir(fullname) == 1)
        {
            add_sync_item("createfolder",fullname,download_only_socket_head);
            add_all_download_only_dragfolder_socket_list(fullname);
        }
        else
        {
            add_sync_item("createfile",fullname,download_only_socket_head);
        }
    }

    closedir(dp);

    return (fail_flag == 1) ? -1 : 0;
}

int handle_rename(int parentID,char *fullname,int type,char *prepath,
                  int is_case_conflict,char *pre_name)
{
    Propfind *find = NULL;
    int entryID = -10;
    char *confilicted_name = NULL;
    char *filename = NULL;
    char path[512];
    int isfolder = 0;
    char *newfilename = NULL;
    Operateentry *oe = NULL;
    int res_value = -10;
    char newfullname[512];
    int status = -10;
    char tmp_name[1024] = {0};

    filename = parse_name_from_path(fullname);


    if(NULL == filename)
    {
        handle_error(S_MEMORY_FAIL,"handle_rename parse filename");
        return -1;
    }

    memset(path,0,sizeof(path));
    strncpy(path,fullname,strlen(fullname)-strlen(filename)-1);

    if(test_if_dir(fullname))
        isfolder = 1;
    else
        isfolder = 0;

    strncpy(tmp_name,fullname,1024);
    while(!exit_loop)
    {

        if(is_case_conflict)
            confilicted_name = get_confilicted_name_case(tmp_name,path,pre_name,filename);
        else
            confilicted_name = get_confilicted_name(tmp_name,0);

        if(confilicted_name ==  NULL)
        {
            Cdbg(API_DBG, "handle_local_confilict_file fail\n");
            return -1;
        }
       if(access(confilicted_name,F_OK) == 0)
       {
           memset(tmp_name,0,sizeof(tmp_name));
           snprintf(tmp_name,NORMALSIZE,"%s",confilicted_name);
           my_free(confilicted_name);
       }
       else
           break;
     }

    if(upload_only && !is_case_conflict)
    {  
        find = checkEntryExisted(username,parentID,filename,"system.unknown");

        if(NULL == find)
        {
            Cdbg(API_DBG, "find prop failed\n");
            my_free(filename);
            my_free(confilicted_name);
            return -1;
        }

        if( find->status != 0 )
        {
            handle_error(find->status,"propfind");
            my_free(filename);
            my_free(confilicted_name);
            my_free(find);
            return -1;
        }

        entryID = find->id;

        if(strcmp(find->type,"system.folder") == 0)
        {
            isfolder = 1;
        }
        else if(strcmp(find->type,"system.file") == 0)
        {
            isfolder = 0;
        }

       newfilename = parse_name_from_path(confilicted_name);
       if(NULL == newfilename)
       {
           handle_error(find->status,"propfind");
           my_free(filename);
           my_free(confilicted_name);
           my_free(find);
           return -1;
       }

       oe = renameEntry(username,entryID,0,newfilename,isfolder) ;
       if(NULL == oe)
       {
           Cdbg(API_DBG, "operate rename failed\n");
           handle_error(find->status,"propfind");
           my_free(filename);
           my_free(confilicted_name);
           my_free(find);
           my_free(newfilename);
           return -1;
       }

       if( oe->status != 0 )
       {
           handle_error(oe->status,"renameEntry");
           sprintf(newfullname,"%s/%s",path,newfilename);
           res_value = handle_rename_fail_code(oe->status,parentID,newfullname,path,isfolder);
           if(res_value != 0)
           {
               my_free(filename);
               my_free(confilicted_name);
               my_free(find);
               my_free(newfilename);
               my_free(oe);
               return res_value;
           }
       }
       res_value = upload_entry(fullname,parentID,path);
       my_free(find);
       my_free(newfilename);
       my_free(oe);
    }
    else  //sync or case conflict
    {     
        status = rename(fullname,confilicted_name);

        if(status == -1)
        {
            handle_error(S_RENAME_FAIL," rename name");
            my_free(filename);
            my_free(confilicted_name);
            return -1;
        }
        add_sync_item("rename",confilicted_name,from_server_sync_head);
        res_value = upload_entry(confilicted_name,parentID,path);

    }
    my_free(filename);
    my_free(confilicted_name);

    return res_value;
}

char *strlwr(char *s)
{
    char *str = NULL;
    char *name = (char *)calloc(256,sizeof(char));
    strncpy(name,s,256);
    str = name;

    while(*str != '\0')
    {
        if(*str >= 'A' && *str <= 'Z'){
            *str += 'a'-'A';
        }
        str++;
    }
    return name;
}

int is_exist_case_conflicts(char *fullname,char *pre_name)
{
    struct dirent* ent = NULL;
    DIR *pDir;
    char *name;
    char path[1024] = {0};
    char *p1 = NULL;
    char*p2 = NULL;
    int count = 0;

    name = parse_name_from_path(fullname);

    if(name == NULL)
    {
        Cdbg(API_DBG, "parse name fail\n");
        return -1;
    }

    strncpy(path,fullname,strlen(fullname)-strlen(name)-1);
    p1 = strlwr(name);


    pDir=opendir(path);

    if(NULL == pDir)
    {
        Cdbg(API_DBG, "open %s fail\n",path);
        my_free(name);
        my_free(p1);
        return -1;
    }

    while (NULL != (ent=readdir(pDir)))
    {
        if(!strcmp(ent->d_name,".") || !strcmp(ent->d_name,"..") || !strcmp(ent->d_name,name))
            continue;

        p2 = strlwr(ent->d_name);
        if(!strcmp(p1,p2))
        {
           count++;
           strcpy(pre_name,ent->d_name);
        }
        my_free(p2);
    }

    closedir(pDir);
    my_free(p1)
    my_free(name);

    if(count>0)
        return 1;
    return 0;
}

int handle_createfolder_fail_code(int status,int parent_ID,char *path,char* fullname)
{
    int res_value = -10;
    int ID = parent_ID ;
    char error_message[512];
    char pre_name[256] = {0};

    memset(error_message,0,sizeof(error_message));

    switch(status)
    {
    case S_DIR_NOT_EXIST: //parent dir not exist
        ID = create_server_folder_r(path);
        if(ID<0)
            return -1;
        res_value = upload_entry(fullname,ID,path);
        break;
    case S_NAME_REPEAT:
        res_value = is_exist_case_conflicts(fullname,pre_name);
        if(res_value == -1)
            return -1;
        if(res_value)
            res_value = handle_rename(parent_ID,fullname,createfolder_action,NULL,1,pre_name);
        break;
    default:
        break;
    }

    return res_value;
}

int handle_upload_fail_code(int status,int parent_ID,char* fullname,const char *path)
{
    int res_value = 0;
    char up_action[256] = {0};
    int ID = -10 ;
    int res_upload = -10;

    switch(status)
    {
    case S_USER_NOSPACE:
        write_log(S_ERROR,"server space is not enough","");
        server_space_full = 1;
        res_value = 0;
        break;
    case S_SERVER_SPACE_FULL:
        snprintf(up_action,256,"uploadfile,%d,%s",parent_ID,"none");
        add_sync_item(up_action,fullname,up_head);
        add_sync_item("up_excep_fail",fullname,up_excep_fail);
        res_value = 0;
        break;
    case S_UPLOAD_DELETED:
        snprintf(up_action,256,"uploadfile,%d,%s",parent_ID,"none");
        del_sync_item(up_action,fullname,up_head);
        del_sync_item("up_excep_fail",fullname,up_excep_fail);
        res_value = 0;
        break;
    case S_FILE_TOO_LARGE:
         write_trans_excep_log(fullname,1,"Filesize Too Large");
         res_value = 0;
         no_completed = 1;
         break;
    case S_TRANS_ID_NOT_EXIST:
        res_value = 0 ;
        break;
    case S_DIR_NOT_EXIST:
        ID = create_server_folder_r(path);
        if( -1 == ID )
        {
            res_value = -1;
        }
        else if(ID > 0)
        {
            res_upload = uploadFile(fullname,ID,NULL,0);
            if(res_upload != 0)
            {
                res_value = handle_upload_fail_code(res_upload,ID,fullname,path);
            }
        }
        break;
 case S_NAME_REPEAT:
        //type = createfile_action;
        //res_value = handle_rename(parent_ID,fullname,createfile_action,NULL);
        //res_value = handle_name_repeat(parent_ID,fullname,path);
        /*
        confilicted_name = get_confilicted_name(fullname,0);
        if(NULL == confilicted_name)
        {
            Cdbg(API_DBG, "get %s confilicted name fail\n",fullname);
            return -1;
        }
        //strcpy(cur_name,confilicted_name);
        //Cdbg(API_DBG, " %s reanme to %s \n",fullname,cur_name);
        if( rename(fullname,confilicted_name) == -1)
        {
            handle_error(S_RENAME_FAIL,"renmae");
            return -1;
        }
        add_sync_item("rename",confilicted_name,from_server_sync_head);
        res_upload = uploadFile(confilicted_name,ID,NULL);
        if(res_upload != 0)
        {
            res_value = handle_upload_fail_code(res_upload,ID,confilicted_name,path);
        }
        */
        res_value = -1;
        break;
   case -1:
        //write_trans_excep_log(fullname,1,"Upload Fail");
        res_value = -1;
        break;
    default:
        break;
    }
    return res_value;
}

int handle_delete_fail_code(int status)
{
    int res_value = 0;

    switch(status)
    {
    case S_FILE_NOT_EXIST:
    case S_DIR_NOT_EXIST:
        res_value = 0;
        break;
    default:
        break;
    }

    return res_value;
}

int handle_rename_fail_code(int status,int parentID,char *fullname,char *path,int isfolder)
{
    int res_value = 0;
    int ID = parentID ;
    char error_message[512];
    char pre_name[256] = {0};

    memset(error_message,0,sizeof(error_message));

    switch(status)
    {
    case S_FILE_NOT_EXIST:
        res_value = upload_entry(fullname,parentID,path);
        break;
    case S_DIR_NOT_EXIST:
        ID = create_server_folder_r(path);
        if(ID < 0)
            return -1;
        res_value = upload_entry(fullname,ID,path);
        break;
    case S_NAME_REPEAT:
        res_value = is_exist_case_conflicts(fullname,pre_name);
        Cdbg(API_DBG, "res_value=%d\n",res_value);
        if(res_value == -1)
            return -1;
        res_value = handle_rename(parentID,fullname,rename_action,NULL,res_value,pre_name);
        break;
    default:
        break;
    }
  return res_value;
}

int handle_move_fail_code(int status,char *path,char *fullname,int parentID,char *prepath,int entryID,int isfolder)
{
    int res_value = 0;
    int ID = parentID ;
    char pre_name[256] = {0};
    Operateentry *oe;

    switch(status)
    {
    case S_FILE_NOT_EXIST:
        res_value = upload_entry(fullname,parentID,path);
        break;
    case S_NAME_REPEAT:
        oe = removeEntry(username,entryID,0,isfolder,parentID);
        if(oe == NULL)
            return -1;
        my_free(oe);
        res_value = is_exist_case_conflicts(fullname,pre_name);
        if(res_value == -1)
            return -1;
        res_value = handle_rename(parentID,fullname,rename_action,NULL,res_value,pre_name);
        break;
    case S_DIR_NOT_EXIST:
    case S_ILLEGAL_OPERATION:   //parent dir delete
        ID = create_server_folder_r(path);
        if(ID<0)
            return -1;
        res_value = upload_entry(fullname,ID,path);
        break;
    default:
        break;

    }

    return res_value;

}

int create_server_folder_r(const char *path)
{
    if(NULL == path)
        return -1;

    char *cut_path;
    char parse_path[512];
    int parentID = -5;
    Propfind *pfind;
    int sync_path_len;
    const char *split = "/";
    char *p2;
    Createfolder *cf = NULL;
    char fullname[512];

    memset(parse_path,0,sizeof(parse_path));
    memset(fullname,0,sizeof(fullname));
    strcpy(parse_path,path);
    strcpy(fullname,sync_path);

    if( !strcmp(parse_path,sync_path) ) // path is sync root path
    {
        return MySyncFolder;
    }

    sync_path_len = strlen(sync_path);
    cut_path = parse_path;
    cut_path = cut_path + sync_path_len;
    cut_path++ ; // pass first '/'

    p2 = strtok(cut_path,split);
    int j=0;
    while(p2!=NULL)
    {
        strcat(fullname,"/");
        strcat(fullname,p2);

        if(j == 0)
            parentID = MySyncFolder;

#ifdef DEBUG
        //Cdbg(API_DBG, "check path is %s\n",p2);
#endif

        pfind = checkEntryExisted(username,parentID,p2,"system.folder");

        if(NULL == pfind)
        {
            return -1;
        }
        else if(pfind->status == 0)
        {
            if(strcmp(pfind->type,"system.notfound") == 0)
            {
                    my_free(pfind);
                    cf = createFolder(username,parentID,0,fullname);
                    if(NULL == cf)
                    {
                        Cdbg(API_DBG, "cf is null\n");
                        return -1;
                    }
                    else if(cf->status == 0)
                    {
                        parentID = cf->id;
                        my_free(cf);
                    }
                    else
                    {
                        handle_error(cf->status,"createfolder");
                        my_free(cf);
                        return -1;
                    }
            }
            else
            {
                parentID = pfind->id;
                my_free(pfind);
            }
        }
        else
        {
            handle_error(pfind->status,"propfind");
            my_free(pfind);
            return -1;
        }

        j++;
        p2 = strtok(NULL,split);
    }

   return parentID;
}

int upload_entry(char *fullname,int parent_ID,char *path)
{
    int status = -10;
    int entry_ID = -10;
    Createfolder *cf = NULL;
    int res_value = 0;
#if TREE_NODE_ENABLE
    modify_tree_node(fullname,DirRootNode,ADD_TREE_NODE);
#endif
    if(test_if_dir(fullname))
    {
        cf = createFolder(username,parent_ID,0,fullname);
        if(NULL == cf)
        {
            Cdbg(API_DBG, "createfolder %s fail\n",fullname);
            return -1;
        }
        else if( cf->status != 0 )
        {
            handle_error(cf->status,"propfind");
            res_value = handle_createfolder_fail_code(cf->status,parent_ID,path,fullname);
            my_free(cf);
            return res_value;
        }
        else
        {
            entry_ID = cf->id;
            my_free(cf);
            res_value = sync_all_item(fullname,entry_ID);
            return res_value;
        }
    }
    else //entry is file
    {
        status = uploadFile(fullname,parent_ID,NULL,0);
        
        if(status != 0)
        {
            Cdbg(API_DBG, "upload %s failed\n",fullname);
            res_value = handle_upload_fail_code(status,parent_ID,fullname,path);
            return res_value;
        }
    }

    return res_value;
}

int obtain_token_from_file(const char *filename,Aaa *aaa)
{
    Cdbg(API_DBG, "system_token=%s\n",filename);

    FILE *fp = NULL;
    int len;

    if(access(filename,0) != 0)
        return -1;

    fp = fopen(filename,"rb");
    if(fp == NULL)
    {
        Cdbg(API_DBG, "open %s fail\n",filename);
        return -1;
    }
    len = fread(aaa,sizeof(Aaa),1,fp);
    fclose(fp);

    Cdbg(API_DBG, "len=%d\n",len);
    if(len <= 0)
        return -1;

    if(strcmp(aaa->user,username) || strcmp(aaa->pwd,password))
        return -1;


    strncpy(sergate.gateway,aaa->gateway,MINSIZE);

    Cdbg(API_DBG, "filrelay=%s,inforelay=%s\n",aaa->filerelay,aaa->inforelay);
    Cdbg(API_DBG, "status=%d,token=%s\n", aaa->status,aaa->token);


    // if(obtainSyncRootID(username) == -1) {
    //     Cdbg(API_DBG, "obtainSyncRootID status  -1\n");    
    //     return -1;
    // }

    return 0;
}

void writeErrorMsg(char *filename) {



    FILE *fp;
    fp = fopen(filename,"w");

    if(NULL == fp) {

        Cdbg(API_DBG, "open %s file fail\n",filename);

    }

    fclose(fp);

}


int getRouterInfo(RouterInfo *ri, struct aicloud_conf *ai_conf) {

    // router data
    char router_ip[30];
    memset(router_ip,0,sizeof(router_ip));
    strcpy(router_ip, "https://");
    strcat(router_ip, ai_conf->privateip);
    strcat(router_ip, "/");

    Cdbg(API_DBG, "getRouterInfo :  router_ip : %s\n", router_ip);

    // referer data
    char referer_ip[40];
    memset(referer_ip,0,sizeof(referer_ip));
    strcpy(referer_ip, "referer: https://");
    strcat(referer_ip, ai_conf->privateip);
    strcat(referer_ip, "/");
    
    Cdbg(API_DBG, "getRouterInfo :  referer_ip : %s\n", referer_ip);


    FILE *fd;

    fd = fopen("/tmp/diag_db_cloud/get_router_info.xml","w");
    if(NULL == fd) {
        Cdbg(API_DBG, "open get_router_info.xml file fail\n");
        return -1;
    }

    CURL *hnd = curl_easy_init();

    curl_easy_setopt(hnd, CURLOPT_CUSTOMREQUEST, "GETROUTERINFO");
    curl_easy_setopt(hnd, CURLOPT_URL, router_ip);

    /* get verbose debug output please */
    curl_easy_setopt(hnd, CURLOPT_VERBOSE,   1L);

    //curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, FALSE);
    //curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER , 1);
    //curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST , 1);
    curl_easy_setopt(hnd, CURLOPT_SSL_VERIFYPEER, 0L); 
    curl_easy_setopt(hnd, CURLOPT_SSL_VERIFYHOST, 0L);

    curl_easy_setopt(hnd,CURLOPT_WRITEDATA,fd);


    struct curl_slist *headers = NULL;


    char * authorization = accountEncodeBase64(ai_conf);

    headers = curl_slist_append(headers, "cache-control: no-cache");
    headers = curl_slist_append(headers, referer_ip);
    headers = curl_slist_append(headers, authorization);

    free(authorization);

    curl_easy_setopt(hnd, CURLOPT_HTTPHEADER, headers);

    CURLcode ret = curl_easy_perform(hnd);



    curl_easy_cleanup(hnd);

    fclose(fd);

    //memset(&router_info,0,sizeof(RouterInfo));
    //parseRouterInfoDoc("get_router_info.xml",&router_info);
    parseRouterInfoDoc("/tmp/diag_db_cloud/get_router_info.xml", ri);
  

    return 0; 
}


int parseRouterInfoDoc(char *docname,void *obj)
{


    xmlDocPtr doc;
    xmlNodePtr cur;

    
    Cdbg(API_DBG, "parseRouterInfoDoc : docname : %s \n",docname);
    
    
    doc = xmlParseFile(docname);
        // doc = xmlParseFile("get_router_info.xml");

    if (doc == NULL ) {
        //fprintf(stderr,"%s not parsed successfully. \n",docname);
        Cdbg(API_DBG, "%s not parsed successfully. \n",docname);
        return -1;
    } else {
      
      Cdbg(API_DBG, "%s parsed successfully. \n",docname);
      
    }

    cur = xmlDocGetRootElement(doc);


    if (cur == NULL) {
        //fprintf(stderr,"%s empty document\n",docname);
        Cdbg(API_DBG, "%s empty document\n",docname);

        xmlFreeDoc(doc);
        return -1;
    } else {
      Cdbg(API_DBG, "%s document\n",docname);
      
    }


    while (cur != NULL) {

      if ((!xmlStrcmp(cur->name, (const xmlChar *)"result"))) 
      {
        parseRouterInfo(doc, cur,(RouterInfo *)obj);
      }


      cur = cur->next;
    }
    xmlFreeDoc(doc);
    
    return 0;
}



void parseRouterInfo(xmlDocPtr doc, xmlNodePtr cur,RouterInfo *ri)
{
    xmlChar *key;
    cur = cur->xmlChildrenNode;

    while (cur != NULL)
    {

        key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);

        if( !(xmlStrcmp(cur->name, (const xmlChar *)"servertime")))
        {

          strcpy(ri->servertime,(const char *)key);

        } else if( !(xmlStrcmp(cur->name, (const xmlChar *)"mac")))  {

          strcpy(ri->mac,(const char *)key);

        } else if( !(xmlStrcmp(cur->name, (const xmlChar *)"version")))  {

          strcpy(ri->version,(const char *)key);

        } else if( !(xmlStrcmp(cur->name, (const xmlChar *)"aicloud_version")))  {

          strcpy(ri->aicloud_version,(const char *)key);

        } else if( !(xmlStrcmp(cur->name, (const xmlChar *)"aicloud_app_type")))  {

          strcpy(ri->aicloud_app_type,(const char *)key);

        } else if( !(xmlStrcmp(cur->name, (const xmlChar *)"modalname")))  {

          strcpy(ri->modalname,(const char *)key);


        } else if( !(xmlStrcmp(cur->name, (const xmlChar *)"computername")))  {

          strcpy(ri->computername,(const char *)key);


        } else if( !(xmlStrcmp(cur->name, (const xmlChar *)"usbdiskname")))  {

          strcpy(ri->usbdiskname,(const char *)key);

        } else if( !(xmlStrcmp(cur->name, (const xmlChar *)"disk_space")))  {


          parseDiskSpace(doc, cur, ri);

          // key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);

          // memset(ri->DiskUsedPercent,0,sizeof(ri->DiskUsedPercent));
          // strcpy(ri->DiskUsedPercent, key);
          // xmlFree(key);

        } 


        xmlFree(key);
        cur = cur->next;
    }


    Cdbg(API_DBG, "ri->servertime : %s\n", ri->servertime);
    Cdbg(API_DBG, "ri->mac : %s\n", ri->mac);
    Cdbg(API_DBG, "ri->version : %s\n", ri->version);
    Cdbg(API_DBG, "ri->aicloud_version : %s\n", ri->aicloud_version);
    Cdbg(API_DBG, "ri->aicloud_app_type : %s\n", ri->aicloud_app_type);
    Cdbg(API_DBG, "ri->modalname : %s\n", ri->modalname);
    Cdbg(API_DBG, "ri->computername : %s\n", ri->computername);
    Cdbg(API_DBG, "ri->usbdiskname : %s\n", ri->usbdiskname);
    Cdbg(API_DBG, "ri->DiskName : %s\n", ri->DiskName);
    Cdbg(API_DBG, "ri->DiskUsed : %s\n", ri->DiskUsed);
    Cdbg(API_DBG, "ri->DiskAvailable : %s\n", ri->DiskAvailable);
    Cdbg(API_DBG, "ri->DiskUsedPercent : %s\n", ri->DiskUsedPercent);

    return;
}

void parseDiskSpace(xmlDocPtr doc, xmlNodePtr cur, RouterInfo *ri) {

  cur = cur->xmlChildrenNode;

  while (cur != NULL) {

    if ((!xmlStrcmp(cur->name, (const xmlChar *)"item"))) {

      parseDiskSpaceItem(doc, cur, ri);

    }
    cur = cur->next;
  }
  return;
} 


void parseDiskSpaceItem(xmlDocPtr doc, xmlNodePtr cur, RouterInfo *ri) {

  xmlChar *key;
  cur = cur->xmlChildrenNode;

  while (cur != NULL) {

    key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);

    if ((!xmlStrcmp(cur->name, (const xmlChar *)"DiskName"))) {

      strcpy(ri->DiskName,(const char *)key);

    } else if ((!xmlStrcmp(cur->name, (const xmlChar *)"DiskUsed"))) {

      strcpy(ri->DiskUsed,(const char *)key);

    } else if ((!xmlStrcmp(cur->name, (const xmlChar *)"DiskAvailable"))) {

      strcpy(ri->DiskAvailable,(const char *)key);

    } else if ((!xmlStrcmp(cur->name, (const xmlChar *)"DiskUsedPercent"))) {

      strcpy(ri->DiskUsedPercent,(const char *)key);

    }

    xmlFree(key);

    cur = cur->next;
  }
  return;
} 


int getSgToken(Aaa *aaa, struct aaews_provision_conf *ap_conf) {

    char post_url[64];
    memset(post_url,0,sizeof(post_url));

    //strcpy(post_url, ap_conf->serviceGateway);

    strcpy(post_url, "https://");
    strcat(post_url, aaa->service_gateway);
    strcat(post_url, "/oauth/getsgtoken");


    Cdbg(API_DBG, "getSgToken -> post_url = %s\n", post_url);


    char post_content[200];
    memset(post_content,0,sizeof(post_content));

    strcpy(post_content, "<?xml version=\"1.0\" encoding=\"utf-8\" ?><getsgtoken><accesstoken>");
    strcat(post_content, ap_conf->token);
    strcat(post_content, "</accesstoken></getsgtoken>");


    Cdbg(API_DBG, "getSgToken -> post_content = %s\n", post_content);

    FILE *fd;

    fd = fopen("/tmp/diag_db_cloud/get_sg_token.xml","w");
    if(NULL == fd) {
        Cdbg(API_DBG, "open get_sg_token.xml file fail\n");
        return -1;
    }

    CURL *hnd = curl_easy_init();

    //curl_easy_setopt(hnd, CURLOPT_URL, "https://sgb02.asuswebstorage.com/oauth/getsgtoken");


    curl_easy_setopt(hnd, CURLOPT_CUSTOMREQUEST, "POST");
    curl_easy_setopt(hnd, CURLOPT_URL, post_url);

    // get verbose debug output please 
    curl_easy_setopt(hnd, CURLOPT_VERBOSE, 0);

    //curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, FALSE);
    //curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER , 1);
    //curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST , 1);
    curl_easy_setopt(hnd, CURLOPT_SSL_VERIFYPEER, 0L); 
    curl_easy_setopt(hnd, CURLOPT_SSL_VERIFYHOST, 0L);

    //curl_easy_setopt(hnd,CURLOPT_TIMEOUT, 30);
    curl_easy_setopt(hnd,CURLOPT_CONNECTTIMEOUT, 30);


    curl_easy_setopt(hnd,CURLOPT_WRITEDATA,fd);


    struct curl_slist *headers = NULL;


    headers = curl_slist_append(headers, "cache-control: no-cache");


    curl_easy_setopt(hnd, CURLOPT_HTTPHEADER, headers);

    curl_easy_setopt(hnd, CURLOPT_POSTFIELDS, post_content);

    CURLcode ret = curl_easy_perform(hnd);

    curl_easy_cleanup(hnd);

    fclose(fd);


    int parseStatus = 0;
    parseStatus = parseSgTokenDoc("/tmp/diag_db_cloud/get_sg_token.xml",aaa);
 
    // del get_sg_token.xml
    if(remove("/tmp/diag_db_cloud/get_sg_token.xml") == 0 ) {
        Cdbg(API_DBG, "get_sg_token file removed\n");
    } else {
        Cdbg(API_DBG, "get_sg_token file remove error\n");
    }

    return parseStatus;
}


int parseSgTokenDoc(char *docname,void *obj)
{


    xmlDocPtr doc;
    xmlNodePtr cur;
    
    Cdbg(API_DBG, "parseSgTokenDoc docname : %s \n",docname);
    
    doc = xmlParseFile(docname);
    // doc = xmlParseFile("get_router_info.xml");

    if (doc == NULL ) {
        //fprintf(stderr,"%s not parsed successfully. \n",docname);
        Cdbg(API_DBG, "%s not parsed successfully. \n",docname);

        return -1;

    } else {
      
      Cdbg(API_DBG, "%s parsed successfully. \n",docname);
      
    }

    cur = xmlDocGetRootElement(doc);


    if (cur == NULL) {
        //fprintf(stderr,"%s empty document\n",docname);
        Cdbg(API_DBG, "%s empty document\n",docname);

        xmlFreeDoc(doc);

        return -1;
    } else {
      Cdbg(API_DBG, "Start read %s document\n",docname);
      
    }


    while (cur != NULL) {

      if ((!xmlStrcmp(cur->name, (const xmlChar *)"getsgtoken"))) 
      {
        parseSgToken(doc, cur,(Aaa *)obj);

      } else {

        Cdbg(API_DBG, "xml content not tag: <getsgtoken> \n");  
      }

      cur = cur->next;
    }

    xmlFreeDoc(doc);
    
    return 0;
}



void parseSgToken(xmlDocPtr doc, xmlNodePtr cur, Aaa *aaa)
{
    xmlChar *key;
    cur = cur->xmlChildrenNode;

    while (cur != NULL)
    {

        key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);

        if( !(xmlStrcmp(cur->name, (const xmlChar *)"status")))
        {

          aaa->status = atoi((const char *)key);


        } else if( !(xmlStrcmp(cur->name, (const xmlChar *)"sgtoken")))  {

          strcpy(aaa->token,(const char *)key);

        } else if( !(xmlStrcmp(cur->name, (const xmlChar *)"webrelay")))  {

          strcpy(aaa->webrelay,(const char *)key);

        } else if( !(xmlStrcmp(cur->name, (const xmlChar *)"inforelay")))  {

          strcpy(aaa->inforelay,(const char *)key);

        } else if( !(xmlStrcmp(cur->name, (const xmlChar *)"filerelay")))  {

          strcpy(aaa->filerelay,(const char *)key);

        } else if( !(xmlStrcmp(cur->name, (const xmlChar *)"rssrelay")))  {

          strcpy(aaa->rssrelay,(const char *)key);


        } else if( !(xmlStrcmp(cur->name, (const xmlChar *)"searchserver")))  {

          strcpy(aaa->searchrelay,(const char *)key);


        } else if( !(xmlStrcmp(cur->name, (const xmlChar *)"jobrelay")))  {

          strcpy(aaa->jobrelay,(const char *)key);

        } else if( !(xmlStrcmp(cur->name, (const xmlChar *)"contentrelay")))  {

          strcpy(aaa->contentrelay,(const char *)key);
          
        } 


        xmlFree(key);
        cur = cur->next;
    }


    Cdbg(API_DBG, "aaa->status : %d\n", aaa->status);
    Cdbg(API_DBG, "aaa->token : %s\n", aaa->token);
    Cdbg(API_DBG, "aaa->webrelay : %s\n", aaa->webrelay);
    Cdbg(API_DBG, "aaa->inforelay : %s\n", aaa->inforelay);
    Cdbg(API_DBG, "aaa->filerelay : %s\n", aaa->filerelay);
    Cdbg(API_DBG, "aaa->rssrelay : %s\n", aaa->rssrelay);
    Cdbg(API_DBG, "aaa->searchrelay : %s\n", aaa->searchrelay);
    Cdbg(API_DBG, "aaa->jobrelay : %s\n", aaa->jobrelay);
    Cdbg(API_DBG, "aaa->contentrelay : %s\n", aaa->contentrelay);

    return;
}

static size_t read_callback(void *ptr, size_t size, size_t nmemb, void *stream)
{
  size_t retcode;
  curl_off_t nread;

  /* in real-world cases, this would probably get this data differently
     as this fread() stuff is exactly what the library already would do
     by default internally */
  retcode = fread(ptr, size, nmemb, stream);

  nread = (curl_off_t)retcode;

  fprintf(stderr, "*** We read %" CURL_FORMAT_CURL_OFF_T " bytes from file\n", nread);

  

  return retcode;
}




/* Converts an integer value to its hex character*/
// char to_hex(char code) {
//   static char hex[] = "0123456789abcdef";
//   return hex[code & 15];
// }

/* Returns a url-encoded version of str */
/* IMPORTANT: be sure to free() the returned string after use */
char *url_encode(char *str) {
  char *pstr = str, *buf = malloc(strlen(str) * 3 + 1), *pbuf = buf;
  while (*pstr) {
    if (isalnum(*pstr) || *pstr == '-' || *pstr == '_' || *pstr == '.' || *pstr == '~') 
      *pbuf++ = *pstr;
    else if (*pstr == ' ') 
      *pbuf++ = '+';
    else 
      *pbuf++ = '%', *pbuf++ = to_hex(*pstr >> 4), *pbuf++ = to_hex(*pstr & 15);
    pstr++;
  }
  *pbuf = '\0';
  return buf;
}

int httpPut(char* api_url,char* filename, struct aicloud_conf *ai_conf)
{
    CURL *curl;
    CURLcode res;

    FILE * hd_src;

    struct stat file_info;

    char *file;
    

    //Cdbg(API_DBG, "api_url : %s\n", api_url);

    file = filename;
    //char *url = api_url;
    char *url = str_replace(api_url, "#","%23");

    //Cdbg(API_DBG, "api_url Encoded: %s\n", url);

         // url = url_encode(api_url);

         // if(url) {
         //     
         // }
    


    /* get the file size of the local file */
    stat(file, &file_info);

    /* get a FILE * of the same file, could also be made with
     fdopen() from the previous descriptor, but hey this is just
     an example! */
    hd_src = fopen(file, "rb");

    /* In windows, this will init the winsock stuff */
    curl_global_init(CURL_GLOBAL_ALL);

    /* get a curl handle */
    curl = curl_easy_init();

    if(curl) {



        char * authorization = accountEncodeBase64(ai_conf);

        struct curl_slist *headers = NULL;

        headers = curl_slist_append(headers, "Expect:");
        headers = curl_slist_append(headers, "Content-Type: text/xml,charset=UTF-8");  
        headers = curl_slist_append(headers, "cache-control: no-cache");
        headers = curl_slist_append(headers, "accept: */*");
        headers = curl_slist_append(headers, "auto-createfolder: T");
        headers = curl_slist_append(headers, authorization);
        //headers = curl_slist_append(headers, "transfer-encoding: chunked");
        //headers = curl_slist_append(headers, "host: 192.168.31.228");
        //headers = curl_slist_append(headers, content_len);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        free(authorization);

        /* get verbose debug output please */
        curl_easy_setopt(curl, CURLOPT_VERBOSE,   1L);

        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L); 
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);


        //curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");

        /* we want to use our own read function */
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);

        /* we know the server is silly, ignore content-length */
        //curl_easy_setopt(curl, CURLOPT_IGNORE_CONTENT_LENGTH, 0L);

        //curl_easy_setopt(curl, CURLOPT_TRANSFER_ENCODING, 0L);

        //curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_UPLOAD, content_len); 


        /* enable uploading */
        curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

        /* HTTP PUT please */
        curl_easy_setopt(curl, CURLOPT_PUT, 1L);


        /* specify target URL, and note that this URL should include a file
           name, not only a directory */
        curl_easy_setopt(curl, CURLOPT_URL, url);


        /* now specify which file to upload */
        curl_easy_setopt(curl, CURLOPT_READDATA, hd_src);

        /* provide the size of the upload, we specicially typecast the value
           to curl_off_t since we must be sure to use the correct data size */
        curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, (curl_off_t)file_info.st_size);

        /* Now run off and do what you've been told! */
        res = curl_easy_perform(curl);

        //Cdbg(API_DBG, "\n res: %s \n:", res);
        /* Check for errors */
        if(res != CURLE_OK) {
            Cdbg(API_DBG, "thread: %s\n", curl_easy_strerror(res));
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
            curl_easy_strerror(res));
        }

        /* always cleanup */
        curl_easy_cleanup(curl);


        curl_slist_free_all (headers);
    }

    fclose(hd_src); /* close the local file */

    curl_global_cleanup();
    free(url);

    return 0;
}

char * accountEncodeBase64(struct aicloud_conf *ai_conf) {

    //  start : authorization: Basic
    char auth_data[64];
    memset(auth_data,0,sizeof(auth_data));

    strcpy(auth_data, ai_conf->username);
    strcat(auth_data, ":");
    strcat(auth_data, ai_conf->password);

    Cdbg(API_DBG, "accountEncodeBase64 :  auth_data base64 before -> %s\n", auth_data);

    //Encode To Base64
    char* base64EncodeOutput;

    Base64Encode(auth_data, (int) strlen(auth_data), &base64EncodeOutput);
    
    
    memset(auth_data,0,sizeof(auth_data));

    strcpy(auth_data, "authorization: Basic ");
    strcat(auth_data, base64EncodeOutput);

    free(base64EncodeOutput); 

    char *result = (char*)malloc(64);

    strcpy(result, auth_data);      


    Cdbg(API_DBG, "accountEncodeBase64 :  auth_data base64 after -> %s\n", result);
    // end : authorization: Basic

    return result;
}


int ipcamConfigXmlParse() {

    FILE *fp;

    char buf[513];

    fp=fopen("/tmp/ipcamConfig.xml","r");

    if( NULL == fp ) {
        Cdbg(API_DBG, "ipcamConfig file not exist, debugOff : advControl = 0");
        return 0;
    }


    while(fgets(buf,512,fp) != NULL)
    {
        if(strstr(buf,"<advControl>1</advControl>"))
        {
            Cdbg(API_DBG, "debugOn : advControl = 1");

            fclose(fp);

            return 1;
        } 

    }

    fclose(fp);

    Cdbg(API_DBG, "debugOff : advControl = 0");

    return 0;
}


// get lighttpd.user data

char * lighttpdUserParse() {

    FILE *fp;

    char buf[513];
    memset(buf,0,sizeof(buf));


    fp=fopen("/tmp/lighttpd.user","r");

    if( NULL == fp ) {
        Cdbg(API_DBG, "lighttpd.user file not exist");
        return "xxxxxxx@asus.com:xxxxxxxxxxxxxxxxxxxxxxxxx";
    }


    while(fgets(buf,512,fp) != NULL)
    {

        snprintf(dbg_msg, sizeof(dbg_msg), "buf : %s\n", buf);
        // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
        Cdbg(API_DBG, "%s", dbg_msg);

        if(strstr(buf,"sysadmin:"))
        {

            char* sys_admin = str_replace(buf, ":sysadmin:", ":");

            snprintf(dbg_msg, sizeof(dbg_msg), "sys_admin : %s\n", sys_admin);
            // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
            Cdbg(API_DBG, "%s", dbg_msg);


            // memset(buf,0,sizeof(buf));

            // strcpy(buf, sys_admin);

            // Cdbg(API_DBG, "buf : %s ", buf);

            fclose(fp);

            return sys_admin;
        } 

    }

    fclose(fp);

    return "0";
}

// 2 : Input/output error
// 256 : Operation not permitted

int fbRtmpRun() {

    FILE * pf;  
    char pfilename[20];  
    sprintf(pfilename, "/tmp/diag_db_cloud/fb_stmp_status");
  
    pf=fopen(pfilename,"w"); //w is overwrite, a is add  

    if (NULL == pf){  
        Cdbg(API_DBG, "fail to open the file fb_stmp_status!!!");
        fprintf(pf,"%s", "-1 : fail to open the file fb_stmp_status");
        return -1;  
    }  

    char * lighttp_user = lighttpdUserParse();

    snprintf(dbg_msg, sizeof(dbg_msg), "lighttp_user : %s ", lighttp_user);
    // debugLogLevel(DEBUG_LEVEL, DBG_DEBUG, dbg_msg);
    Cdbg(API_DBG, "%s", dbg_msg);

    char fbRtmpStr[350]; 
    memset(fbRtmpStr,0, sizeof(fbRtmpStr));

    // mp4 file upload
    // strcpy(fbRtmpStr, "ffmpeg -re -i input.mp4 -f flv -c:v h264 -c:a aac \"");
    // strcat(fbRtmpStr, dp_conf.fbRtmpUrl);
    // strcat(fbRtmpStr, "\"");


    // rtsp upload
    strcpy(fbRtmpStr, "ffmpeg -loglevel error -probesize 32768 -acodec libfdk_aac -ac 1 -rtsp_transport tcp -i \"");
    strcat(fbRtmpStr, "rtsp://");
    strcat(fbRtmpStr, lighttp_user);
    strcat(fbRtmpStr, "@127.0.0.1:554/CH2_P\"");
    strcat(fbRtmpStr, " -f flv -c copy \"");
    strcat(fbRtmpStr, dp_conf.fbRtmpUrl);
    strcat(fbRtmpStr, "\"");

    Cdbg(API_DBG, "fbRtmpStr : %s", fbRtmpStr);

    free(lighttp_user);

    Cdbg(API_DBG, "21system error!");

    pid_t status;  

    status = system(fbRtmpStr);  


    if (-1 == status)  
    {  
        Cdbg(API_DBG, "system error!");

        fprintf(pf,"%d : system error!", status);

    } else {  

        Cdbg(API_DBG, "system exit status = [0x%x]", status);
        Cdbg(API_DBG, "system exit status = [0d%d]", status);
        Cdbg(API_DBG, "system exit WIFEXITED(status) = [0d%d]", WIFEXITED(status));
        Cdbg(API_DBG, "system exit WEXITSTATUS(status) = [0d%d]", WEXITSTATUS(status));

        if (WIFEXITED(status))  
        {  
            if (0 == WEXITSTATUS(status))
            {  
                Cdbg(API_DBG, "system run successfully");

                fprintf(pf,"%d : system run successfully", status);
            }  
            else  
            {  

                Cdbg(API_DBG, "system run fail, exit code : %d", status);

                fprintf(pf,"%d : system run fail", status);
            }  
        }  
        else  
        {  

            Cdbg(API_DBG, "system run fail, exit code : %d", status);

            fprintf(pf,"%d : system exit status", status);
        }  
    }

    fclose(pf);
  
    return status;  
}


void fbRtmpThread()
{

    // get : fb rtmp url 
    if(strlen(dp_conf.fbRtmpUrl) < 10) {

        Cdbg(API_DBG, "fbRtmpUrl is null");

    } else {

        Cdbg(API_DBG, "fbRtmpUrl is -> %s\n", dp_conf.fbRtmpUrl);

        fbRtmpRun();  

    }


    pthread_exit(NULL);
}



void debugLogLevel(int debug_level, int debug_type, char * log_value) {

    int debug_show = 1;

    if((debug_level >= DBG_VERBOSE) && (debug_type == DBG_VERBOSE)) {

        Cdbg(debug_show, "VERBOSE : %s" , log_value);

    } else if((debug_level >= DBG_DEBUG) && (debug_type == DBG_DEBUG)) {

        Cdbg(debug_show, "DEBUG : %s" , log_value);

    } else if((debug_level >= DBG_INFO) && (debug_type == DBG_INFO)) {

        Cdbg(debug_show, "INFO : %s" , log_value);

    } else if((debug_level >= DBG_WARN) && (debug_type == DBG_WARN)) {

        Cdbg(debug_show, "WARN : %s" , log_value);

    } else if((debug_level >= DBG_ERROR) && (debug_type == DBG_ERROR)) {

        Cdbg(debug_show, "ERROR : %s" , log_value);

    } else if((debug_level >= DBG_NO) && (debug_type == DBG_NO)) {


    }

}
/*
int fbRtmp(void) {

    pthread_t rtmp_id;
    int rtmp_ret;
    rtmp_ret = pthread_create(&rtmp_id,NULL,(void *) fbRtmpThread,NULL);
    if(rtmp_ret != 0)
    {
        printf ("fbRtmpThread : Create pthread error!\n");
    }

    //pthread_join(id,NULL);//等待線程（pthread）結束

    return 0;

}
*/