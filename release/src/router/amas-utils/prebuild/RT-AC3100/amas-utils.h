/*
** amas-utils.h
**
**
*/
#ifndef __AMASUTILSH__
#define __AMASUTILSH__

#define AMAS_FUNC 	extern
#define AMAS_API

////////////////////////////////////////////////////////////////////////////////
//
// AMAS_RESULT 
//
////////////////////////////////////////////////////////////////////////////////
typedef enum 
{
	AMAS_RESULT_FILE_LOCK_ERROR			= -17,
	AMAS_RESULT_NBR_DATA_IS_EMPTY		= -16,
	AMAS_RESULT_NBR_SYSDESCR_NO_SEACH 	= -15,
	AMAS_RESULT_VERIFY_VSIEID_FAILED	= -14,
	AMAS_RESULT_GEN_VSIEID_FAILED		= -13,
	AMAS_RESULT_LLDPCLI_EXEC_FAILED		= -12, 
	AMAS_RESULT_BUFFER_IS_TOO_SMALL		= -11,
	AMAS_RESULT_NBR_TLV_UNABLE_TO_PARSE	= -10,
	AMAS_RESULT_NBR_TLV_TYPE_NO_FOUND	= -9,
	AMAS_RESULT_NBR_TLV_NO_SEARCH		= -8,
	AMAS_RESULT_NBR_IFACE_NO_SEARCH		= -7,
	AMAS_RESULT_JSON_UNABLE_TO_PARSE	= -6,
	AMAS_RESULT_FILE_OPERATE_FAILED		= -5,
	AMAS_RESULT_MEM_ALLOCATE_ERROR		= -4,
	AMAS_RESULT_FILE_OPEN_ERROR			= -3,
	AMAS_RESULT_INVALID_VALUE			= -2,
	AMAS_RESULT_FAILED					= -1,
	AMAS_RESULT_SUCCESS 				= 0
} AMAS_RESULT;

////////////////////////////////////////////////////////////////////////////////
//
//	Define
//
////////////////////////////////////////////////////////////////////////////////
#define MAX_VERSION_TEXT_LENGTH		512

////////////////////////////////////////////////////////////////////////////////
//
//	EXTERNAL FUNCTIONS
//
////////////////////////////////////////////////////////////////////////////////
AMAS_FUNC char* 		AMAS_API amas_utils_version_text(void);
AMAS_FUNC char* 		AMAS_API amas_utils_str_error(AMAS_RESULT code);
AMAS_FUNC void 			AMAS_API amas_utils_set_debug(unsigned int enable);
#if defined(USE_GET_TLV_SUPPORT_MAC)
AMAS_FUNC AMAS_RESULT	AMAS_API amas_get_cost(char *ifname, int bandindex, int capability5g, char *ifmac, int *cost);
#else	// USE_GET_TLV_SUPPORT_MAC
AMAS_FUNC AMAS_RESULT	AMAS_API amas_get_cost(char *ifname, int bandindex, int capability5g, int *cost);
#endif	// USE_GET_TLV_SUPPORT_MAC
AMAS_FUNC AMAS_RESULT	AMAS_API amas_set_cost(int cost);
#endif /* !__AMASUTILSH__ */
