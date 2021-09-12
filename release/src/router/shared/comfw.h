#ifndef __COMFW_H__
#define __COMFW_H__

#define MAX_CF  4
#define MAX_NAMELEN     128
#define OUTPUT_DIR      "comfw_dir"
#define BUFSIZE         4096
#define COMFW_MAGIC     0x20210816

enum {
        _TRX = 1,
        _W   = 2,
        _PKGTB = 3,
        MAX_FTYPE = 4
};

typedef struct _comfw
{
        int magic;
        int fw_type[MAX_CF];
        int fw_size[MAX_CF];
	char data[MAX_CF][16];
} comfw_head;

#endif
