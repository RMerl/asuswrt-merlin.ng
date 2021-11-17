#ifndef __COMFW_H__
#define __COMFW_H__

#define MAX_CF  4
#define MAX_NAMELEN     128
#define OUTPUT_DIR      "comfw_dir"
#define BUFSIZE         4096
#define COMFW_MAGIC     0x20210816

#define COMFW_MODELID {                       \
     MODELID(ASUS_MODEL),                       \
     MODELID(CF_RTAC68U),                    \
     MODELID(CF_RTAX58U),                        \
     MODELID(CF_RTAX58U_V2),                        \
     MODELID(CF_RPAX56),                        \
     MODELID(CF_RPAX58),                        \
     MODELID(CF_RTAX82_XD6),                        \
     MODELID(MAX_FTYPE),                           \
}

#define MODELID(a)       a
typedef enum COMFW_MODELID comfw_modid_e;
#undef MODELID

#define MODELID(a)       #a
char *comfw_modid_s[] = COMFW_MODELID;
#undef MODELID

/*
enum {
        _TRX             = 1,
        _W_RTAX58U       = 2,
        _PKGTB_RTAX58UV2 = 3,
        _W_RPAX56        = 4,
        _PKGTB_RPAX58    = 5,
        //MAX_FTYPE
};
*/

typedef struct _comfw
{
        int magic;
        int fw_type[MAX_CF];
        int fw_size[MAX_CF];
	char data[MAX_CF][16];
} comfw_head;

#endif
