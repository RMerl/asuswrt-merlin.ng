#ifndef _namecheap_h_
#define _namecheap_h_

#define NC_DEFAULT_SERVER "dynamicdns.park-your-domain.com"
#define NC_DEFAULT_PORT HTTP_DEFAULT_PORT
#define NC_REQUEST "/update"

extern char *NC_fields_used[];

extern int NC_update_entry(void);
extern int NC_check_info(void);

#endif
