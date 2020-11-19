/*
**	cfg_ctrllist.h
**
**
**
*/
#ifndef __CFG_CTRLLIST_H__
#define __CFG_CTRLLIST_H__

#include <json.h>
#include <string.h>
#include <stdio.h>
#include <shared.h>
#include <shutils.h>
#include <bcmnvram.h>
#ifdef RTCONFIG_AMAS_WGN
#include <amas_wgn_shared.h>
#endif
#include "encrypt_main.h"
#include "cfg_common.h"
#include "cfg_string.h"

int addCtrlParam(struct json_object *cfgRoot, char *feature_name, char *key, char *value, char *rc_services);
extern void cm_transCtrlParam(struct json_object *outRoot);
extern void cm_applyCtrlAction(struct json_object *cfgRoot, int *cfgChanged, char *action_script, size_t action_script_size);
#endif	/* __CFG_CTRLLIST_H__ */
