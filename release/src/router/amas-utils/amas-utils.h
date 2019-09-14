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



///////////////////////////////////////////////////////////////////////////////
//
//	data structure for AMAS_SUBTYPE_OBSTATUS
//
////////////////////////////////////////////////////////////////////////////////
typedef struct _ob_status{
	unsigned char neighmac[7];
	unsigned char modelname[64];
	int obstatus; 		//1 (OB_OFF),  2 (OB_Available), 3 (OB_REQ), 4 (OB_LOCKED), 5 (OB_SUCCESS)
	int timestamp;
	int reboottime;
	int conntimeout;
	int traffictimeout;
}ob_status,*ptr_obstatus;


///////////////////////////////////////////////////////////////////////////////
//
//	data structure for AMAS_SUBTYPE_SECSTATUS
//
////////////////////////////////////////////////////////////////////////////////
typedef struct _security_status{
	unsigned char neighmac[7];
	unsigned char peermac[7];
	int secstatus; 		//1 (SS_KEY),  2 (SS_KEYACK), 3 (SS_SECURITY), 4 (SS_SUCCESS)
}sec_status,*ptr_sectatus;


///////////////////////////////////////////////////////////////////////////////
//
//	data structure for AMAS_SUBTYPE_SESSIONKEY & SECURITY KEY
//
////////////////////////////////////////////////////////////////////////////////
typedef struct _data_exchange{
	unsigned char neighmac[7];
	unsigned char peermac[7];
	unsigned char data[MAX_VERSION_TEXT_LENGTH+1];
	unsigned int  datalen;
}data_exchange,*ptr_dataexchange;



///////////////////////////////////////////////////////////////////////////////
//
//	data structure for AMAS_SUBTYPE_ID
//
////////////////////////////////////////////////////////////////////////////////
typedef struct _id_info{
	unsigned char neighmac[7];
	unsigned char newremac[7];
	unsigned char id[MAX_VERSION_TEXT_LENGTH+1];
	unsigned int  idlen;
	unsigned char modelname[64];
	int obstatus; 		//1 (OB_OFF),  2 (OB_Available), 3 (OB_REQ), 4 (OB_LOCKED), 5 (OB_SUCCESS)
	int timestamp;
}id_info,*ptr_idinfo;


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
AMAS_FUNC AMAS_RESULT	AMAS_API amas_set_obstatus(int status);
AMAS_FUNC AMAS_RESULT 	AMAS_API amas_get_obstatus(ob_status **P_obstatus, int *len);
AMAS_FUNC AMAS_RESULT   AMAS_API amas_set_peermac(unsigned char *macaddr);
AMAS_FUNC AMAS_RESULT   AMAS_API amas_get_peermac(unsigned char *macaddr);
AMAS_FUNC AMAS_RESULT   AMAS_API amas_get_newremac(unsigned char *macaddr);
AMAS_FUNC AMAS_RESULT   AMAS_API amas_set_secstatus(int status);
AMAS_FUNC AMAS_RESULT   AMAS_API amas_get_secstatus(sec_status **P_secstatus, int *len);
AMAS_FUNC AMAS_RESULT   AMAS_API amas_set_sessionkey(unsigned char *sessionkey);
//AMAS_FUNC AMAS_RESULT   AMAS_API amas_get_sessionkey(unsigned char *sessionkey, int *len);
AMAS_FUNC AMAS_RESULT   AMAS_API amas_get_sessionkey(data_exchange **P_keyexchange, int *len);
AMAS_FUNC AMAS_RESULT   AMAS_API amas_set_wifisec(char *type, unsigned char *value);
AMAS_FUNC AMAS_RESULT   AMAS_API amas_get_wifisec(char *type, unsigned char *value, int *len);
AMAS_FUNC AMAS_RESULT   AMAS_API amas_get_obdinfo(id_info **P_idinfo, int *len);
AMAS_FUNC AMAS_RESULT   AMAS_API amas_set_group(unsigned char *group);
AMAS_FUNC AMAS_RESULT   AMAS_API amas_get_group(unsigned char *group, int *len);
AMAS_FUNC void   AMAS_API amas_set_timeout(int rtime, int ctimeout, int ttimeout);
#endif /* !__AMASUTILSH__ */
