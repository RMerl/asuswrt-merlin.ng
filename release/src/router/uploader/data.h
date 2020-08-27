#include <stdint.h>
#include <pthread.h>

#ifndef DATA_H
#define DATA_H

#define MINSIZE 64
#define NORMALSIZE 512
#define MAXSIZE 1024
#define BUFSIZE	1024*16

#define ASUSWEBSTORAGE_SYNCFOLDER "MySyncFolder"
#define ASUSWEBSTORAGE 10

#define S_INITIAL		70
#define S_SYNC			71
#define S_DOWNUP		72
#define S_UPLOAD		73
#define S_DOWNLOAD		74
#define S_STOP			75
#define S_ERROR			76
#define S_NEEDCAPTCHA           77
#define S_AUTHFAIL              78
#define S_OTPFAIL               79

#define LOG_SIZE                sizeof(struct LOG_STRUCT)

//#ifndef IPKG
#define CONFIG_PATH "/tmp/smartsync/asuswebstorage/config/Cloud.conf"
//#else
//#define CONFIG_PATH "/opt/etc/Cloud.conf"
//#endif

#ifdef IPKG
#define NOTIFY_PATH "/tmp/notify/usb"

#define GET_NVRAM_SCRIPT_1 "/tmp/smartsync/asuswebstorage/script/asuswebstorage_get_nvram"
#define SH_GET_NVRAM_SCRIPT_1 "sh /tmp/smartsync/asuswebstorage/script/asuswebstorage_get_nvram"
#define NVRAM_PATH_1 "/tmp/smartsync/asuswebstorage/config/asuswebstorage_tmpconfig"

#define GET_NVRAM_SCRIPT_2 "/tmp/smartsync/asuswebstorage/script/asuswebstorage_get_nvram_link"
#define SH_GET_NVRAM_SCRIPT_2 "sh /tmp/smartsync/asuswebstorage/script/asuswebstorage_get_nvram_link"
#define NVRAM_PATH_2 "/tmp/smartsync/asuswebstorage/config/link_internet"
#endif


//#ifndef IPKG
//#define SHELL_FILE  "/tmp/asuswebstorage_write_nvram"
//#define NVRAM_TOKENFILE "asuswebstorage_tokenfile"
//#endif


/*servergetway struct*/
typedef struct SERVICEGATEWAY
{
    int status;
    char gateway[MINSIZE];
//    char liveupdateuri[NORMALSIZE];
//    char time[MINSIZE];
}Servicegateway;

/*initattachabledevice data struct*/
typedef struct INITATTACHABLEDEVICE
{
    int status;
}Initattachabledevice;

/*Browseattachabledevice data struct*/
typedef struct BROWSEATTACHABLEDEVICE
{
    int status;
    //char scrip[MINSIZE];
    //Parentfolder parentfolder;
    //int attachabledevice_number;
    //Attachabledevice **attachabledevice_list;
    char owneruserid[MINSIZE];
    char deviceid[MINSIZE];
    char attachabledevicename[MINSIZE];
    char devicemanagerhost[MINSIZE];
    int state;
    char expiredtime[MINSIZE];
    int filelifetime;
    int version;
    int scheme;

}BrowseAttachableDevice;

/*
typedef struct ATTACHABLEDEVICE
{
    int id;
//    long long treesize;
    char *display;
//    Attribute attribute;
//    int isencrypted;
//    int issharing;
//    int isowner;
//    int isbackup;
//    int isorigdeleted;
//    int ispublic;
//    char createdtime[MINSIZE];
//    int markid;
//    char metadata[NORMALSIZE];
    int isdeleted;
    int ischangeseq;
}Attachabledevice;
*/

/*get tokey struct*/
typedef struct PACKAGE
{
    int id;
    char display[NORMALSIZE];
    int capacity;
    int uploadbandwidth;
}Package;


typedef struct AAA
{
    int status;
    char service_gateway[64];
    char token[128];
    char contentrelay[MINSIZE];
    char filerelay[MINSIZE];
    char inforelay[MINSIZE];
    char jobrelay[MINSIZE];
    char rssrelay[MINSIZE];
    char searchrelay[MINSIZE];
    char webrelay[MINSIZE];
    Package package;
    char auxpasswordurl[256];
    char gateway[MINSIZE];         //for reuse token add
    char user[256];                //for reuse token add
    char pwd[256];                 //for reuse token add
}Aaa;



typedef struct ROUTERINFO
{
    char servertime[15];
    char mac[20];
    char version[15];
    char aicloud_version[128];
    char aicloud_app_type[10];
    char modalname[15];
    char computername[15];
    char usbdiskname[15];

    char DiskName[20];
    char DiskUsed[20];
    char DiskAvailable[20];
    char DiskUsedPercent[5];

}RouterInfo;



/*initbinaryupload struct*/
typedef struct INITBINARYUPLOAD
{
    int status;
    char transid[NORMALSIZE];
    int offset;
    char latestchecksum[256];
    char logmessage[NORMALSIZE];
    long long fileid;
}Initbinaryupload;

/* resume binary upload*/
typedef struct RESUMEBINARYUPLOAD
{
    int status;
}Resumebinaryupload;

/* finish binary upload */
typedef struct FINISHBINARYUPLOAD
{
    int status;
    long long fileid;
}Finishbinaryupload;

/*browse folder struct*/

typedef struct PARENTFOLDER
{
    char name[NORMALSIZE];
    int id;
}Parentfolder;

typedef struct PAGE
{
   int pageno;
   int pagesize;
   int totalcount;
   int hasnextpage;
}Page;

typedef struct ATTRIBUTE
{
//    char creationtime[MINSIZE];
//    char lastaccesstime[MINSIZE];
    char lastwritetime[16];
//    char finfo[MINSIZE];
//    char xtimeforsynccheck[NORMALSIZE];
//    char xmachinename[NORMALSIZE];
}Attribute;

typedef struct FOLDER
{
    long long id;
//    long long treesize;
    char *display;
//    Attribute attribute;
//    int isencrypted;
//    int issharing;
//    int isowner;
//    int isbackup;
//    int isorigdeleted;
//    int ispublic;
    // char *createdtime;
    char createdtime[MINSIZE];
//    int markid;
//    char metadata[NORMALSIZE];
    int isdeleted;
    int ischangeseq;
}Folder;

typedef struct FILEATTRIBUTE
{
//    char creationtime[MINSIZE];
    char lastaccesstime[16];
    char lastwritetime[16];
//    char finfo[MINSIZE];
//    char xtimeforsynccheck[NORMALSIZE];
//    char xmachinename[NORMALSIZE];
}Fileattribute;

typedef struct FILE
{
    int id;
//    int status;
    char *display;
    Fileattribute attribute;
    long long size;
//    int isencrypted;
//    int isowner;
//    int isbackup;
//    int isorigdeleted;
//    int isinfected;
//    int ispublic;
//    int headversion;
    char createdtime[MINSIZE];
    // char *createdtime;
//    int markid;
//    char metadata[NORMALSIZE];
}File;

typedef struct FILES
{
    char *id;
}Files;

typedef struct BROWSE
{
    int status;
    //char scrip[MINSIZE];
    //Parentfolder parentfolder;
    int foldernumber;
    int filenumber;
    Folder **folderlist;
    File   **filelist;
    Page page;
}Browse;

typedef struct SLIDINGBROWSE
{
    int status;
    int filenumber;
    Files  **filelist;
}SlidingBrowse;


/*struct local folder struct*/
typedef struct LOCALFOLDER
{
  //char name[NORMALSIZE];
  char *name;
  //struct LOCALFOLDER *next;
  //int isfolder;
}Localfolder;

typedef struct LOCALFILE
{
    //char name[NORMALSIZE];
    char *name;
    Attribute attribute;
    long long size;
    //struct LOCALFILE *next;
}Localfile;

typedef struct LOCAL
{
 int foldernum;
 int filenum;
 Localfolder **folderlist;
 Localfile   **filelist;
 //Localfolder *folder_head;
 //Localfolder *file_head;
}Local;

//typedef struct
typedef struct LOCALS
{
    Parentfolder parent;
    Local local;
    struct LOCALS *next;
}Locals;

/* find/propfind data struct*/
typedef struct PROPFIND
{
    int status;
    int isencrypt;
    long long size;
    unsigned long script;
    char type[MINSIZE];
    int id;
}Propfind;

/*get change seq*/
typedef struct CHANGESEQ
{
  int status;
  char scrip[MINSIZE];
  unsigned int changeseq;

}Changeseq;

/*createfolder data struct*/
typedef struct CREATEFOLDER
{
    int status;
    char scrip[MINSIZE];
    long long id;

}Createfolder;

/* get all items struct*/
typedef struct FOLDERNAME
{
    char name[512];
}Foldername;

typedef struct FOLDERS
{
  int number;
  Foldername folderlist[512];
}Folders;

/* get file and folderid*/
typedef struct ITEMID
{
    int fileID;
    int parentID;
}ItemID;

/*rename (remove folder, file, files) struct*/
typedef struct OPERATEENTRY
{
    int status;
    char script[MINSIZE];
}Operateentry;


/*move etnry*/
typedef struct MOVEENTRY
{
    int status;
    char script[MINSIZE];
    char logmessage[NORMALSIZE];
}Moveentry;

/*get info*/
typedef struct BACKUPPC
{
    char name[NORMALSIZE];
    char createdtime[NORMALSIZE];
}Backuppc;

typedef struct FEATURELIST
{

}Featurelist;

typedef struct INFOPACKAGE
{
    int id;
    char display[NORMALSIZE];
    int capacity;
    int uploadbandwidth;
    int downloadbandwidth;
    int upload;
    int download;
    int concurrentsession;
    int maxfilesize;
    int sharegroup;
    int hasencrption;
    char expire[MINSIZE];
    int maxbackuppc;

}Infopackage;



typedef struct GETINFO
{
   int status;
   int account;
   char email[NORMALSIZE];
   char regyear[MINSIZE];
   char language[MINSIZE];
   char activateddate[MINSIZE];
   int credentialstate;
   int usedbackuppc;
   Backuppc backuppc;
   Infopackage package;
   int usedcappacity;
   int freecapacity;
}Getinfo;

/*get MySyncFolder data struct*/
typedef struct GETMYSYNCFOLDER
{
    int status;
    int id;
}Getmysyncfolder;

/*get GetInitAttachableDevice data struct*/
typedef struct GETINITATTACHABLEDEVICE
{
    int status;
    int id;
}GetInitAttachableDevice;

/*get GetBrowseAttachableDevice data struct
typedef struct GETBROWSEATTACHABLEDEVICE
{
    int status;
    int id;

    char owneruserid[MINSIZE];
    char deviceid[MINSIZE];
    char attachabledevicename[MINSIZE];
    char devicemanagerhost[MINSIZE];
    int state;
    char expiredtime[MINSIZE];
    int filelifetime;
    int version;
    int scheme;

}GetBrowseAttachableDevice;
*/

/*get GetInitAttachableDevice data struct*/
typedef struct SENDAWSEMAIL
{
    int status;
    int id;
}SendAwsEmail;

/*get PersonalSystemFolder data struct*/
typedef struct GETPERSONALSYSTEMFOLDER
{
    int status;
    char script[16];
    int  folderid;
}Getpersonalsystemfolder;

/*upload and download item struct*/
typedef struct TRANSITEM
{
    int id;
    char name[NORMALSIZE];
    long long int size;
    char transid[64];

}Transitem;

/* log struct */
typedef struct LOG_STRUCT{
        int  status;
        char  error[512];
        float  progress;
        char path[512];
}Log_struc;

/*get entry info*/
typedef struct GETENTRYINFO
{
    int status;
    char scrip[MINSIZE];
    int isfolder;
    char display[NORMALSIZE];
    int parent;
    int headversion;
    Fileattribute attr;
    long long filesize;     // for file
    long long treesize; //for folder
    int ishidden;       //for folder
    int isinfected;
    int isbackup;
    int isorigdeleted;
    int ispublic;
    char createdtime[MINSIZE];
}Getentryinfo;

typedef struct my_mutex_tag
{
    pthread_mutex_t mutex;
    pthread_cond_t  cond;
    int ready;
}my_mutex_t;

/*add by alan*/
/*typedef struct socket_action{
	char buf[1024];
	struct socket_action *next;
}Socket_Action;
Socket_Action *SocketActionList;
Socket_Action *SocketActionTail;
Socket_Action *SocketActionTmp;*/

typedef struct SOCKET_CMD
{
    char cmd_name[32];
    char filename[NORMALSIZE];
    char path[1024];
    char oldname[NORMALSIZE]; //rename or move prename
    char newname[NORMALSIZE]; //rename new name
    char oldpath[1024]; //move old path
}Socket_cmd;

/*muti dir read config*/
struct asus_rule
{
    int rule;
    char path[256];
};

struct asus_config
{
    int type;
    char user[256];
    char pwd[256];
    char url[32];
    int  enable;
    char sync_path[256];
    int rule;
    char dir_name[256];
    int sync_disk_exist;
    char captcha[8];
    char otp_key[8];
};

typedef struct USERSTATE
{
    int status;
    int userstate;
    char extensionstarttime[32];
    char extensionendtime[32];
}Getuserstate;

struct MemoryStruct {
  char *memory;
  size_t size;
};

#endif
