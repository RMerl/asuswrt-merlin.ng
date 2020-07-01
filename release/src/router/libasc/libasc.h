#ifndef __LIBASC_H__
#define __LIBASC_H__

//Feature definition
#define AHS 0
#define SECURITY_DAEMON 1


//return value definition
#define LIBASC_SUCCESS	0
#define LIBASC_INVALID_PARAMETER	-1
#define LIBASC_WRITE_ERROR	-2 
#define LIBASC_INIT_FAIL		-3

extern int curl_download_file(const int feature, const char *url, const char *file_path);
#endif
