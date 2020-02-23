#include "include/bcmTargetEndian.h"
#define  BCMTAG_EXE_USE
#include "bcmTag.h"
#include "boardparms.h"
#define MAX_MAC_STR_LEN     19         // mac address string 18+1 in regular format - 02:18:10:11:aa:bb 
#include "board.h"

#define OPT_LEN_IS_VALID(_maxLen) ( strlen(optarg) < (_maxLen) )

unsigned int for_script_image_base=IMAGE_BASE;
unsigned int for_script_nvram_offset=NVRAM_DATA_OFFSET;  
int for_script_numOfMac = DEFAULT_MAC_NUM;
int for_script_tpNum = DEFAULT_TP_NUM;
int for_script_psiSize = DEFAULT_PSI_SIZE;
int for_script_logSize= DEFAULT_LOG_SIZE;
int for_script_backupPsi = DEFAUT_BACKUP_PSI;
int for_script_flashBlkSize = DEFAULT_FLASHBLK_SIZE;
int for_script_auxFSPercent = DEFAULT_AUXFS_PERCENT;
int for_script_token_len = TOKEN_LEN;
int for_script_pmc_flag_bit=WFI_FLAG_HAS_PMC;

