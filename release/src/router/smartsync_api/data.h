#include <sys/stat.h>
#include <curl/curl.h>
#include <curl/easy.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <utime.h>
#include <stdarg.h>

#define __DEBUG__
//#undef __DEBUG__

#ifdef __DEBUG__
//#define DEBUG(format, ...) printf("FILE:"__FILE__",LINE:%05d:"format"\n",__LINE__,##__VA_ARGS__)
#define DEBUG(format, ...) printf(format,##__VA_ARGS__)
#define CURL_DEBUG
#else
#define DEBUG(format, ...)
#define CURL_DEBUG
#endif


#define NORMALSIZE 512
#define MAXSIZE 1024
#define MINSIZE 64
#define NAMESIZE 256
#define MAX_CONTENT 256
#define S_MEMORY_FAIL                   4001

#define TMP_R "/tmp/smartsync/google/temp/"
#define Con(TMP_R,b) TMP_R#b
#define my_free(x)  if(x) {free(x);x=NULL;}
#define getb(type) (type*)malloc(sizeof(type))

#define S_INITIAL		70
#define S_SYNC			71
#define S_DOWNUP		72
#define S_UPLOAD		73
#define S_DOWNLOAD		74
#define S_STOP			75
#define S_ERROR			76

#define CA_INFO_FILE "/tmp/smartsync/google/cert/GeoTrustGlobalCA.crt"
