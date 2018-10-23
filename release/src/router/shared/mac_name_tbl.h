#ifndef __MAC_NAME_TBL_H__
#define __MAC_NAME_TBL_H__

#include <shared.h>

#define MNT_FILE			"/tmp/MNT_DEBUG"

#if !defined(RTCONFIG_RALINK) && !defined(HND_ROUTER)
#define MNT_DEBUG(fmt, args...) \
	if(f_exists(MNT_FILE)) { \
		_dprintf(fmt, ## args); \
	}
#else
#define MNT_DEBUG(fmt, args...) \
	if(f_exists(MNT_FILE)) { \
		printf(fmt, ## args); \
	}
#endif

#if (defined(RTCONFIG_JFFS2) || defined(RTCONFIG_JFFSV1) || defined(RTCONFIG_BRCM_NAND_JFFS2))
#define NMP_CL_JSON_FILE		"/jffs/nmp_cl_json.js"
#else
#define NMP_CL_JSON_FILE		"/tmp/nmp_cl_json.js"
#endif

#define NCL_LIMIT		14336   //database limit to 14KB to avoid UI glitch

extern char *search_mnt(char *mac);
#endif	/* ! __MAC_NAME_TBL_H__ */
