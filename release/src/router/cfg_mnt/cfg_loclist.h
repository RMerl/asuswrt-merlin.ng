#ifndef __CFG_LOCLIST_H__
#define __CFG_LOCLIST_H__

#include <json.h>

typedef struct cm_loclist_transfer_country_s {
    int model;
    char src_iso_code[8];
    char dest_iso_code[8];
} cm_loclist_transfer_country_s;


extern cm_loclist_transfer_country_s tcode_country_trans[];

extern void cm_Set_location_code(struct json_object *cfgRoot, int *location_change);
extern void cm_transloclist_Parameter(struct json_object *outRoot);

#endif /* __CFG_LOCLIST_H__ */
/* End of cfg_loclist.h */