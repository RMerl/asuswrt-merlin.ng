/* ------------------------------
    ### Common ###
---------------------------------*/
enum {
	TXT=0,
	HTML
};
#define MAIL_CONTENT_INFO_PATH          "/tmp/MailContentInfo"
#define MAIL_CONF                       "/etc/email/email.conf"
#define MAIL_ITEM_PATH                  "/tmp/nc"
#define STRLEN                          256

/* Key Name Define */
#define MODEL_NAME                      "ModelName"
#define SYSTEM_LANGUAGE                 "SysLanguage"
#define MAIL_TO                         "MailTo"

typedef struct __mail_keyname__t_ {
	char   *keyName;
} MAIL_KEYNAME_T;

MAIL_KEYNAME_T KeyNameInfo[] =
{
	{MODEL_NAME},
	{SYSTEM_LANGUAGE},
	{MAIL_TO},
	/* The End */
	{0}
};

struct NC_SMTP_INFO
{
	int   index;
	char *server;
	int   port;
};

struct NC_SMTP_INFO SMTPInfo[] =  
{
	{0, "smtp.gmail.com",      587 },
	{1, "smtp.mail.yahoo.com", 587 },
	{2, "smtp.aol.com",        587 },
	{3, "smtp.qq.com",         587 },
	{4, "smtp.163.com",        25  },
	{0,0,0}
};

/* ------------------------------
    ### PROTECTION EVENT ###
---------------------------------*/
#define NMP_BUFF                        512 * 1024 // 512KB
#define NMP_PATH                        "/jffs/nmp_client_list"

#define PROTECTION_VULNERABILITY_LOG    "/jffs/.sys/AiProtectionMonitor/NT-AiMonitorVPevent.txt"
#define PROTECTION_CC_LOG               "/jffs/.sys/AiProtectionMonitor/NT-AiMonitorCCevent.txt"
#define PROTECTION_MALS_LOG             "/jffs/.sys/AiProtectionMonitor/NT-AiMonitorMALSevent.txt"

typedef struct mail_info mail_s;
struct mail_info{
	int type;
	char date[40];
	char src[32];
	char dst[128];
	char hostname[100];
	mail_s *next;
};
/* ------------------------------
    ### FIRMWARE EVENT ###
---------------------------------*/
#define FW_RELEASE_NOTE_PATH           "/tmp/New_FW_Release_Note.txt"
