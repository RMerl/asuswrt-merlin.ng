#ifndef __MAC_NAME_TAB_H__
#define __MAC_NAME_TAB_H__

#include <shared.h>

#define MNT_FILE			"/tmp/MNT_DEBUG"

#define MNT_DEBUG(fmt, args...) \
	if(f_exists(MNT_FILE)) { \
		_dprintf(fmt, ## args); \
	}

#if (defined(RTCONFIG_JFFS2) || defined(RTCONFIG_JFFSV1) || defined(RTCONFIG_BRCM_NAND_JFFS2))
#define NMP_CLIENT_LIST_FILENAME	"/jffs/nmp_client_list"
#else
#define NMP_CLIENT_LIST_FILENAME	"/tmp/nmp_client_list"
#endif
#define NCL_LIMIT 524288   //nmp_client_list limit size(512KB)

extern char *search_mnt(char *mac);
#endif	/* ! __MAC_NAME_TAB_H__ */
