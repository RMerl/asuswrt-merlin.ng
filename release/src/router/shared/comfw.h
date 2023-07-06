#ifndef __COMFW_H__
#define __COMFW_H__

#include <model.h>

#define MAX_CF  4
#define MAX_NAMELEN     128
#define OUTPUT_DIR      "comfw_dir"
#define BUFSIZE         4096
#define COMFW_MAGIC     0x20210816
#define CFID_BASE       6000
#define CFID_BASE_2     12000
#define CFID_BASE_3     18000

/*
CF_RTAC68U       // trx            
CF_RTAX58U       // w              
CF_RTAX58U_V2    // pkgtb           
CF_RPAX56        // w               
CF_RPAX58        // pkgtb              
CF_RTAX82_XD6    // w           
CF_RTAX82_XD6S   // w	
CF_TUFAX3000     // w                
CF_TUFAX3000_V2  // pkgtb              
CF_RTAX95Q       // w              
CF_RTAX95QV2     // pkgtb               
CF_RTAX95QV3     // pkgtb               
CF_RTAX55        // w
CF_RTAX3000N     // pkgtb
CF_RTAX82U       // w
CF_RTAX82U_V2    // pkgtb
CF_TUFAX5400     // w
CF_TUFAX5400_V2  // pkgtb
CF_XD6_V2        // pkgtb
*/

/* new model must be added after the latest one */
#define COMFW_MODELID {                       \
     MODELID(ASUS_MODEL),                       \
     MODELID(CF_RTAC68U),                    \
     MODELID(CF_RTAX58U),                        \
     MODELID(CF_RTAX58U_V2),                 \
     MODELID(CF_RPAX56),                        \
     MODELID(CF_RPAX58),                        \
     MODELID(CF_RTAX82_XD6),                        \
     MODELID(CF_RTAX82_XD6S),		\
     MODELID(CF_TUFAX3000),                     \
     MODELID(CF_TUFAX3000_V2),                  \
     MODELID(CF_RTAX95Q),                     \
     MODELID(CF_RTAX95QV2),                     \
     MODELID(CF_RTAX95QV3),                     \
     MODELID(CF_RTAX55),                      \
     MODELID(CF_RTAX3000N),                   \
     MODELID(CF_RTAX82U),                     \
     MODELID(CF_RTAX82U_V2),                  \
     MODELID(CF_TUFAX5400),                   \
     MODELID(CF_TUFAX5400_V2),                \
     MODELID(CF_XD6_V2),                      \
     MODELID(CF_GSAX3000),                   \
     MODELID(CF_GSAX5400),                   \
     MODELID(MAX_FTYPE),                           \
}

#define MODELID(a)       a
typedef enum COMFW_MODELID comfw_modid_e;
#undef MODELID

extern char *comfw_modid_s[];

typedef struct _comfw
{
        int magic;
        int fw_type[MAX_CF];	// origianl is type id, now as cf model id
        int fw_size[MAX_CF];
	char data[MAX_CF][16];
} comfw_head;

struct cf_data_desc {
        int catid;
        char *val;
        char *dscp;
};

enum {
	FW_386,
	FW_384,
	FW_38X,
	FW_102,
	FW_388,
	MAX_FWID
};


#endif
