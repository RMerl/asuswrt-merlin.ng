#include <string.h>
#include "shared.h"
#include "version.h"

const char *rt_version = RT_VERSION;
const char *rt_serialno = RT_SERIALNO;
const char *rt_rcno = RT_RCNO;
const char *rt_extendno = RT_EXTENDNO;
const char *rt_buildname = RT_BUILD_NAME;
const char *rt_buildinfo = RT_BUILD_INFO;
const char *rt_swpjverno = RT_SWPJVERNO;
#ifdef RTCONFIG_NVRAM_ENCRYPT
const char *enc_sp_extendno = ENC_SP_EXTENDNO;
#endif
#ifdef RTCONFIG_LIVE_UPDATE_RSA
const char *live_update_rsa_ver = LIVE_UPDATE_RSA_VERSION;
#endif

void set_basic_fw_name(void){
	nvram_set("buildno", rt_serialno);
	if(rt_rcno)
		nvram_set("rcno", rt_rcno);
	else
		nvram_unset("rcno");
	nvram_set("extendno", rt_extendno);
	nvram_set("buildinfo", rt_buildinfo);
	nvram_set("swpjverno", rt_swpjverno);
#ifdef RTCONFIG_NVRAM_ENCRYPT
	nvram_set("enc_sp_extendno", enc_sp_extendno);
#endif
#ifdef RTCONFIG_LIVE_UPDATE_RSA
	nvram_set("live_update_rsa_ver", live_update_rsa_ver);
#else
	nvram_unset("live_update_rsa_ver");
#endif
}

