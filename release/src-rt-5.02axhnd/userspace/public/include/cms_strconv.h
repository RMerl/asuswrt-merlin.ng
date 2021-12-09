/***********************************************************************
 *
 *  Copyright (c) 2006-2007  Broadcom Corporation
 *  All Rights Reserved
 *
 * <:label-BRCM:2011:DUAL/GPL:standard
 * 
 * Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2
 * (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
 * with the following added to such license:
 * 
 *    As a special exception, the copyright holders of this software give
 *    you permission to link this software with independent modules, and
 *    to copy and distribute the resulting executable under terms of your
 *    choice, provided that you also meet, for each linked independent
 *    module, the terms and conditions of the license of that module.
 *    An independent module is a module which is not derived from this
 *    software.  The special exception does not apply to any modifications
 *    of the software.
 * 
 * Not withstanding the above, under no circumstances may you combine
 * this software in any way with any other Broadcom software provided
 * under a license other than the GPL, without Broadcom's express prior
 * written consent.
 * 
:>
 *
 ************************************************************************/

#ifndef __CMS_STRCONV_H__
#define __CMS_STRCONV_H__

#include <arpa/inet.h>  /* mwang_todo: should not include OS dependent file */


/*!\file cms_strconv.h
 * \brief Header file for the application to convert value from string to other formats.
 * This is in the cms_util library.
 */

/** Macro to determine if a string parameter is empty or not */
#define IS_EMPTY_STRING(s)    ((s == NULL) || (*s == '\0'))

/** Macro to print string, considering string being  NULL  */
#define NULL_TO_EMPTY_STRING(s)    ((s == NULL)? "" : s)

/** convert vpiVciStr (eg. "0/35") to vpi=2 and vci=35
 * @param vpiVciStr (IN) vpi/vci string
 * @param vpi (OUT) vpi in SINT32 format 
 * @param vci (OUT) vpi in SINT32 format
 * @return CmsRet enum.
 */
CmsRet cmsUtl_atmVpiVciStrToNum(const char *vpiVciStr, SINT32 *vpi, SINT32 *vci);

/**
 * @param vpiVciStr (IN) prefix:vpi/vci string where prefix is PVC:vpi/vci
 * @param vpi (OUT) vpi in SINT32 format 
 * @param vci (OUT) vpi in SINT32 format
 * @return CmsRet enum.
 */
CmsRet cmsUtl_atmVpiVciNumToStr(const SINT32 vpi, const SINT32 vci, char *vpiVciStr);

/** convert macStr to macNum (array of 6 bytes)  This accepts both delimited
 *  and non delimited formats:
 *  Ex: "0a:0b:0c:0d:0e:0f" -> 0a0b0c0d0e0f
 *  Ex: "0a0b:0c0d:0e0f" ->    0a0b0c0d0e0f
 *  Ex: "0a0b0c0d0e0f" ->      0a0b0c0d0e0f
 *
 * @param macStr (IN) macStr to be converted.
 * @param macNum (OUT) macNum must point to a buffer of at least 6 bytes which the
 *                     caller provides.
 * @return CmsRet enum.
 */
CmsRet cmsUtl_macStrToNum(const char *macStr, UINT8 *macNum);

/** convert macNum (array of 6 bytes) to macStr (in colon delimited format).
 *  Ex: 0a0b0c0d0e0f -> "0a:0b:0c:0d:0e:0f"
 * 
 * @param macNum (IN) macNum array to be converted.
 * @param macStr (OUT) macStr must point to a buffer of at least MAC_STR_LEN+1 (17+1)
 *                     to store the results.
 * @return CmsRet enum.
 */
CmsRet cmsUtl_macNumToStr(const UINT8 *macNum, char *macStr);


/** convert string to signed 32 bit integer, similar to unix strtol
 *
 * @param str (IN)        string to convert
 * @param endptr (IN/OUT) optional, if provided, will point to the first
 *                        character which caused conversion error.
 * @param base (IN)       radix to use for conversion.  If 0, then the
 *                        function will decide the radix based on input string.
 * @param val (OUT)       if conversion was successful, the resulting value.
 * @return CmsRet enum.
 */
CmsRet cmsUtl_strtol(const char *str, char **endptr, SINT32 base, SINT32 *val);


/** convert string to unsigned 32 bit integer, similar to unix strtoul
 *
 * @param str (IN)        string to convert.  A leading - sign is not allowed.
 * @param endptr (IN/OUT) optional, if provided, will point to the first
 *                        character which caused conversion error.
 * @param base (IN)       radix to use for conversion.  If 0, then the
 *                        function will decide the radix based on input string.
 * @param val (OUT)       if conversion was successful, the resulting value.
 * @return CmsRet enum.
 */
CmsRet cmsUtl_strtoul(const char *str, char **endptr, SINT32 base, UINT32 *val);


/** convert string to signed 64 bit integer, similar to unix strtoll
 *
 * @param str (IN)        string to convert
 * @param endptr (IN/OUT) optional, if provided, will point to the first
 *                        character which caused conversion error.
 * @param base (IN)       radix to use for conversion.  If 0, then the
 *                        function will decide the radix based on input string.
 * @param val (OUT)       if conversion was successful, the resulting value.
 * @return CmsRet enum.
 */
CmsRet cmsUtl_strtol64(const char *str, char **endptr, SINT32 base, SINT64 *val);


/** convert string to unsigned 64 bit integer, similar to unix strtoull
 *
 * @param str (IN)        string to convert.  A leading - sign is not allowed.
 * @param endptr (IN/OUT) optional, if provided, will point to the first
 *                        character which caused conversion error.
 * @param base (IN)       radix to use for conversion.  If 0, then the
 *                        function will decide the radix based on input string.
 * @param val (OUT)       if conversion was successful, the resulting value.
 * @return CmsRet enum.
 */
CmsRet cmsUtl_strtoul64(const char *str, char **endptr, SINT32 base, UINT64 *val);


/** convert string to lowercase string
 *
 * @param string (IN/OUT) the upper case of the characters in the string will be 
 *                        converted to lower case characters.
 */
void cmsUtl_strToLower(char *string);


/*!\enum UrlProto
 * \brief URL protocols returned by cmsUtl_parseUrl().
 */
typedef enum 
{
   URL_PROTO_HTTP=0, /**< http */
   URL_PROTO_HTTPS=1, /**< https */
   URL_PROTO_FTP=2,   /**< ftp */
   URL_PROTO_TFTP=3   /**< tftp */
} UrlProto;


/** Parse an URL and return is components.
 *
 * @param url    (IN) url to parse.
 * @param proto (OUT) If not NULL, and if URL is well formed, it will be
 *                    set to the URL protocol (URL_PROTO_HTTPD, URL_PROTO_FTP, etc.)
 * @param addr  (OUT) If not NULL, and if URL is well formed, it will be 
 *                    set to the address portion of the URL.  Caller is responsible
 *                    for freeing addr string.
 * @param port  (OUT) If not NULL, and if URL is well formed, it will be 
 *                    set to the port portion of the URL.  If no port number
 *                    is specified, then port will be set to 0.
 * @param path  (OUT) If not NULL, and if URL is well formed, it will be 
 *                    set to the path portion of the URL.  Caller is responsible for
 *                    freeing path string.
 *
 * @return CmsRet enum, specifically, CMSRET_SUCCESS if URL is well formed,
 *                      CMSRET_INVALID_ARGUMENTS if URL is not well formed.
 */
CmsRet cmsUtl_parseUrl(const char *url, UrlProto *proto, char **addr, UINT16 *port, char **path);


/*!\enum PcpMode
 * 
 */
typedef enum 
{
   PCP_MODE_DISABLE=0,
   PCP_MODE_DSLITE=1,
   PCP_MODE_NAT444=2,
} PcpMode_t;


/** Return the path to the CommEngine build directory in the given buffer.
 *
 * This function should only be used for unit testing on the Linux desktop,
 * but we define the symbol for modem builds as well to reduce the number
 * of ifdef's in the code.  Strangely, if this function is called on the
 * modem, the returned path is /var (really should be /).
 *
 * See also cmsUtl_getRunTimeRootDir().  This might be a better function
 * to use going forward.
 *
 * @param pathBuf   (OUT) Contains the pathBuf.
 * @param pathBufLen (IN) Length of the buffer.  If the buffer is not large
 *                        enough to hold the path, an error will be returned.
 * @return CmsRet enum.
 */
CmsRet cmsUtl_getBaseDir(char *pathBuf, UINT32 pathBufLen);


/** Return the path to the "root" directory in the given buffer.
 *
 * When run on the Linux desktop, this path would be the base build dir
 * as returned by cmsUtl_getBaseDir plus targets/{PROFILE}/fs.install
 * On the modem, it would be /
 *
 * @param pathBuf   (OUT) Contains the pathBuf.
 * @param pathBufLen (IN) Length of the buffer.  If the buffer is not large
 *                        enough to hold the path, an error will be returned.
 * @return CmsRet enum.
 */
CmsRet cmsUtl_getRunTimeRootDir(char *pathBuf, UINT32 pathBufLen);


/** Return an absolute full path to the given directory.  This function
 * is a wrapper to hide the ugliness of ifdefs and path manipulation from
 * core code which wants to access a file in the filesystem.
 *
 * When run on the Linux desktop, it will append the run time root dir
 * to the given target path.  When running on the modem, just the target
 * path is used.
 *
 * @param target    (IN)  Contains the desired path.
 * @param pathBuf   (OUT) A buffer to hold the resulting path.
 * @param pathBufLen (IN) Length of pathBuf.  If the buffer is not large
 *                        enough to hold the path, an error will be returned.
 * @return CmsRet enum.
 */
CmsRet cmsUtl_getRunTimePath(const char *target, char *pathBuf, UINT32 pathBufLen);


/** Return the Primary and Secondary DNS sever string.
 *
 *
 * @param str (IN)        comma seperated DNS Servers string to be converted
 * @param str (OUT)       if conversion was successful, Primary DNS string.
 * @param str (OUT)       if conversion was successful, Secondary DNS string.
 * @param str (IN)        is the query for IPv4 DNS server

 * @return CmsRet enum.
 */
CmsRet cmsUtl_parseDNS(const char *inDsnServers, char *outDnsPrimary, char *outDnsSecondary, UBOOL8 isIPv4);


/** Concatenate two IPv4 or IPv6 addresses into a single DNS server string.
 *  This is not quite a generic function for concatenating two IP addresses
 *  because it has some peculiar rules taylored for DNS server strings.
 *
 * @param dns1       (IN)   First IPv4 or IPv6 address string. Note that
 *                          "empty" address will not be added to list.
 *                          "empty" is defined as NULL or 0-length string.
 * @param dns2       (IN)   Second IPv4 or IPv6 address string.  Note that
 *                          zero addresses as defined by
 *                          cmsUtl_isZeroIpvxAddress will not be added to list.
 *                          But if dns1 was empty and thus skipped, then if
 *                          dns2 is zero, it will be allowed.  Thus we end
 *                          up with at most 1 zero addr.
 * @param serverList (OUT)  caller supplied buffer to hold the concatenated
 *                          result.
 * @param bufLen     (IN)   Length of serverList buffer.

 * @return CmsRet enum.
 */
CmsRet cmsUtl_concatDNS(const char *dns1, const char *dns2, char *serverList, UINT32 bufLen);


/** Return a syslog mode integer corresponding to the mode string.
 *
 * Mode refers to where the log goes.
 * The mode numbers are hard coded in logconfig.html, so there is no point in
 * defining #def's for them.
 * 1 = local circular buffer in shared mem
 * 2 = remote
 * 3 = local cirulcar buffer in shared mem and remote
 * 4 = local file (not clear if software actually supports this)
 * 5 = local file and remote (not clear if software actually supports this)
 *
 * @param modeStr (IN) mode string.
 *
 * @return mode number.
 */
SINT32 cmsUtl_syslogModeToNum(const char *modeStr);


/** Given a syslog mode number, return the associated string.
 *
 * See cmsUtl_syslogModeToNum for listing of valid mode numbers.
 *
 * @param mode (IN) mode number.
 *
 * @return mode string, this string is a pointer to a static string, so it
 * must not be modified or freed.
 */
char * cmsUtl_numToSyslogModeString(SINT32 mode);


/** Return true if the given number is a valid and supported syslog mode.
 *
 * See cmsUtl_syslogModeToNum for listing of valid mode numbers.
 * Only modes 1, 2, and 3 are supported.
 *
 * @param mode (IN) mode number.
 *
 * @return true if the given mode number is a valid and supported syslog mode.
 */
UBOOL8 cmsUtl_isValidSyslogMode(const char *modeStr);


/** Given a syslog level string, return equivalent log level.
 *
 * The log level numbers are defined in /usr/include/sys/syslog.h.
 *
 * @param levelStr (IN) The level in string format.
 *
 * @return the equivalent syslog level number.
 */
SINT32 cmsUtl_syslogLevelToNum(const char *levelStr);


/** Given a syslog level number, return the equivalent log level string.
 *
 * The log level numbers are defined in /usr/include/sys/syslog.h.
 *
 * @param level (IN) The log level number.
 *
 * @return the log level string, this string is a pointer to a static string, so it
 * must not be modified or freed.
 */
char * cmsUtl_numToSyslogLevelString(SINT32 level);


/** Return true if the given number is a valid syslog level.
 *  This is designed for the cli menu input validation.  But I put this
 *  function here to keep all the syslog level conversions together.
 *
 * @param level (IN) The log level number in string format.
 * 
 * @return TRUE if the given number is a valid syslog level.
 */
UBOOL8 cmsUtl_isValidSyslogLevel(const char *levelStr);


/** Return true if the string is a valid syslog level string.
 *
 * @param level (IN) The log level string.
 * 
 * @return TRUE if the given string is a valid syslog level string.
 */
UBOOL8 cmsUtl_isValidSyslogLevelString(const char *level);

#ifdef SUPPORT_IPV6
/** Convert an IPv6 address string to its standard format returned
 *  from inet_ntop().
 *
 * @param address (IN) IPv6 address in CIDR notation.
 * @param address (OUT) standard format IPv6 address in CIDR notation.
 *
 * @return CmsRet enum.
 */
CmsRet cmsUtl_standardizeIp6Addr(const char *address, char *stdAddr);

/** Given an IPv6 address string, return true if it is a valid IPv6 global
 *  unicast address.
 *
 * @param address (IN) IPv6 address string.
 *
 * @return TRUE if the string is a valid IPv6 global unicast address.
 */
UBOOL8 cmsUtl_isGUAorULA(const char *address);

/** Given an IPv6 prefix string, return true if it is a valid IPv6 ULA prefix.
 *
 * @param prefix (IN) IPv6 prefix string.
 *
 * @return TRUE if the string is a valid IPv6 ULA prefix.
 */
UBOOL8 cmsUtl_isUlaPrefix(const char *prefix);


/** Return the Primary and Secondary DNS sever string.
 *
 * @param str (IN)        comma seperated DNS Servers string to be converted
 * @param str (OUT)       if conversion was successful, Primary DNS string.
 * @param str (OUT)       if conversion was successful, Secondary DNS string.
 *
 * @return CmsRet enum.
 */
CmsRet cmsUtl_replaceEui64(const char *address1, char *address2);

#endif


/** Return the Primary and Secondary DNS sever string.
 *
 * @param str (IN)        comma seperated DNS Servers string to be converted
 * @param str (OUT)       if conversion was successful, Primary DNS string.
 * @param str (OUT)       if conversion was successful, Secondary DNS string.
 *
 * @return CmsRet enum.
 */
CmsRet cmsUtl_getAddrPrefix(const char *address, UINT32 plen, char *prefix);

/** Return the Primary and Secondary DNS sever string.
 *
 * @param str (IN)        comma seperated DNS Servers string to be converted
 * @param str (OUT)       if conversion was successful, Primary DNS string.
 * @param str (OUT)       if conversion was successful, Secondary DNS string.
 *
 * @return CmsRet enum.
 */
CmsRet cmsUtl_parsePrefixAddress(const char *prefixAddr, char *address, UINT32 *plen);


/** If address is in the form of 2701:8810::16/64, truncate the /64 part
 *  by overwriting the '/' with NULL termination.
 *
 * @param ipv4AddrStr (IN/OUT)  If this IPv6 addr str contains prefix info (/64),
 *                              it will be truncated in place.
 */
void cmsUtl_truncatePrefixFromIpv6AddrStr(char *ipv6AddrStr);




/*!
 * \brief defines for ppp auto method in number
 */
#define PPP_AUTH_METHOD_AUTO        0
#define PPP_AUTH_METHOD_PAP         1
#define PPP_AUTH_METHOD_CHAP        2
#define PPP_AUTH_METHOD_MSCHAP      3


/** Given a ppp  auth method string, return the equivalent auth method number.
 *
 *
 * @param authStr (IN) The auth method string.
 *
 * @return the  auth method number
 */
SINT32 cmsUtl_pppAuthToNum(const char *authStr);

/** Given a ppp auth method number, return the equivalent auth method string.
 *
 *
 * @param authNum (IN) The auth method number.
 *
 * @return the  auth method string, this string is a pointer to a static string, so it
 * must not be modified or freed.
 */
char * cmsUtl_numToPppAuthString(SINT32 authNum);


/** Given a log level string, return the equivalent CmsLogLevel enum.
 *
 * @param logLevel (IN) The log level in string form.
 * @return CmsLogLevel enum.
 */
CmsLogLevel cmsUtl_logLevelStringToEnum(const char *logLevel);


/** Given a log destination string, return the equivalent CmsLogDestination enum.
 *
 * @param logDest (IN) The log destination in string form.
 * @return CmsLogDestination enum.
 */
CmsLogDestination cmsUtl_logDestinationStringToEnum(const char *logDest);


/** Given an IP address string, return true if it is a valid IP address.
 *
 * @param af      (IN) address family, either AF_INET or AF_INET6.
 * @param address (IN) IP address string.
 *
 * @return TRUE if the string is a valid IP address.
 */
UBOOL8 cmsUtl_isValidIpAddress(SINT32 af, const char* address);


/** Given an IP address string, return true if it is a valid IPv4 address.
 *
 * @param address (IN) IP address string.
 *
 * @return TRUE if the string is a valid IPv4 address.
 */
UBOOL8 cmsUtl_isValidIpv4Address(const char* address);


/** Given an IPv4 or IPv6 address string, return TRUE if it is an
 * "all-zero" / "unspecified" IP addr.  i.e. 0.0.0.0, 0:0:0:0:0:0:0:0, or ::
 * This function also returns TRUE if address is NULL or empty string.
 *
 */
UBOOL8 cmsUtl_isZeroIpvxAddress(UINT32 ipvx, const char *address);


/** Given a MAC address string, return true if it is a valid mac address
 *  string.  Note the string must be in the following format:
 *  xx:xx:xx:xx:xx:xx where x is a hex digit.
 *
 * @param macAddr (IN) mac address string.
 *
 * @return TRUE if the string is a valid mac address.
 */
UBOOL8 cmsUtl_isValidMacAddress(const char* macAddress);


/** Given a port number string, return true if it is a valid port number.
 *
 * @param portNumberStr (IN) port number string.
 *
 * @return TRUE if the string is a valid port number.
 */
UBOOL8 cmsUtl_isValidPortNumber(const char* portNumberStr);

/** Compare two strings.  Similar to strcmp except that NULL string will be
 *  treated as zero length string.
 *
 * @param s1 (IN) the first string.
 * @param s2 (IN) the second string.
 *
 * @return same return value as strcmp
 */
SINT32 cmsUtl_strcmp(const char *s1, const char *s2);


/** Compare two strings disregarding case.  Similar to strcasecmp except that
 *  NULL string will be treated as zero length string.
 *
 * @param s1 (IN) the first string.
 * @param s2 (IN) the second string.
 *
 * @return same return value as strcasecmp
 */
SINT32 cmsUtl_strcasecmp(const char *s1, const char *s2);


/** Compare two strings.  Similar to strncmp except that NULL string will be
 *  treated as zero length string.
 *
 * @param s1 (IN) the first string.
 * @param s2 (IN) the second string.
 * @param n  (IN) number of characters to compare.
 *
 * @return same return value as strncmp
 */
SINT32 cmsUtl_strncmp(const char *s1, const char *s2, SINT32 n); 


/** Compare two strings disregarding case.  Similar to strncasecmp except that
 *  NULL string will be treated as zero length string.
 *
 * @param s1 (IN) the first string.
 * @param s2 (IN) the second string.
 * @param n  (IN) number of characters to compare.
 *
 * @return same return value as strcmp
 */
SINT32 cmsUtl_strncasecmp(const char *s1, const char *s2, SINT32 n); 


/** locate a substring.  Similar to strstr except that NUll string will be
 *  treated as zero length string.
 *
 * @param s1 (IN) the first string.
 * @param s2 (IN) the second string.
 *
 * @return same return value as strstr
 */
char *cmsUtl_strstr(const char *s1, const char *s2);

char *cmsUtl_strcasestr(const char *s1, const char *s2);

char *cmsUtl_strcpy(char *dest, const char *src);

char *cmsUtl_strcat(char *dest, const char *src);

/* Copies the string form source string(src) to destination
 *  buffer(dest). If the src string is larger than destination
 *  buffer then atmost dlen-1 bytes are copied into destination.
 *  Unlike the standard strncpy, cmsUtl_strncpy guarantees that dest will
 *  be NULL terminated.
 *
 * @param dest(OUT)- destination buffer 
 * @param src (IN) the source string.
 * @param dlen(IN)- destination buffer size 
 *
 * @return destination address(dest)
 */
char *cmsUtl_strncpy(char *dest, const char *src, SINT32 dlen);


/** Appends the string suffix to prefix.  This function differs from
 *  standard strncpy in that prefixLen is the total length of the prefix buffer.
 *  This function will never write more than prefixLen bytes into prefix.
 *  On exit, prefix is guaranteed to be NULL terminated.
 *
 * @param prefix (IN/OUT): On entry, contains the first part of the string. On exit,
 *                       contains prefix+suffix string, or a truncated but
 *                       NULL terminated string if they do not fit.
 * @param prefixLen (IN) : total length of the prefix buffer.
 * @param suffix (IN)    : suffix buffer
 *
 * @return CmsRet
 */
CmsRet cmsUtl_strncat(char *prefix, UINT32 prefixLen, const char *suffix);


/** Return the length of the source string.  Similar to standard strlen,
 *  except that NUll string will be treated as zero length string.
 *
 * @param src (IN) the source string.
 *
 * @return same return value as strlen if NULL, return 0.
 */
SINT32 cmsUtl_strlen(const char *src);


/** Compare two version strings with '.' as separator.
 *
 * @param v1 (IN) the first version string.
 * @param v2 (IN) the second version string.
 *
 * @return same return value as strcmp
 */
SINT32 cmsUtl_strverscmp(const char *v1, const char *v2);


/** locate a sub-option string in a string of sub-options separated by commas.
 * 
 * This function is useful if you have a parameter string that looks like:
 * AAAA, AAAABBB, CCC, DD
 * and you want to know if sub-option AAAA is present.  Note that strstr cannot
 * be used here because if your parameter string was
 * AAAABBB, CCC, DD
 * and you used strstr to look for option AAAA, then strstr would return a pointer
 * to AAAABBB, which is not the sub-option string you were really looking for.
 * This is required by the DSL code in dealing with the AdslModulationCfg parameter.
 *
 * @param s1 (IN) the full sub-option string.  Sub-options can be separated by any
 *                number of spaces and commas.
 * @param s2 (IN) the sub-option string.
 *
 * @return TRUE if the subOption string was found in the fullOptionString.
 */
UBOOL8 cmsUtl_isSubOptionPresent(const char *fullOptionString, const char *subOption);


/** Retrieve WAN protocol name of the given protocol ID
 *
 * @param prtcl     (IN) network protocol
 * @param portId  (OUT) name - protocol name for html
 *
 * @return None.
 */
void cmsUtl_getWanProtocolName(UINT8 protocol, char *name) ;


/** Convert a linear array of DHCP Vendor Ids into a single comma separated string.
 *
 * @param vendorIds (IN) This is a sequence of DHCP Vendor Ids laid out as a single linear buffer.
 *                       There are MAX_PORTMAPPING_DHCP_VENDOR_IDS (5) chunks in the buffer.
 *                       Each chunk is DHCP_VENDOR_ID_LEN + 1 bytes long.  Within each chunk
 *                       there may be a single DHCP vendor id.
 *
 * @return comma separated string of vendor ids.  The caller is responsible for freeing
 *         this buffer.
 */
char *cmsUtl_getAggregateStringFromDhcpVendorIds(const char *vendorIds);

/** Convert a comma separated string of vendor ids into a linear array of DHCP vendor ids.
 *
 * @param aggregateString (IN) This is a sequence of up to MAX_PORTMAPPING_DHCP_VENDOR_IDS(5) comma 
 *                             separated dhcp vendor id strings.
 *
 * @return a linear buffer of vendor ids.  The caller is responsible for freeing this buffer.
 */
char *cmsUtl_getDhcpVendorIdsFromAggregateString(const char *aggregateString);


/** Get number of DHCP vendor ids in the aggregate string
 * 
 * This is not implemented yet because it is not needed.  But it seems like a
 * useful function to have.
 * 
 * @param aggregateString (IN) This is a sequence of up to MAX_PORTMAPPING_DHCP_VENDOR_IDS(5) comma 
 *                             separated dhcp vendor id strings.
 * 
 * @return number of DHCP vendor ids.
 */
UINT32 cmsUtl_getNumberOfDhcpVendorIds(const char *aggregateString);


#define DSL_LINK_DESTINATION_PREFIX_PVC   "PVC:"
#define DSL_LINK_DESTINATION_PREFIX_SVC   "SVC:"


/** Convert connection mode string to a ConnectionModeType number
 *
 * @param connModeStr (IN) MDMVS_ string defined in mdm 
 *
 * @return a ConnectionModeType number
 */
ConnectionModeType cmsUtl_connectionModeStrToNum(const char *connModeStr);

void cmsUtl_generateUuidStrFromRandom(char *str, int len);

void cmsUtl_generateUuidStrFromName(const char *name,
                                    UINT32 nameLen,
                                    char *strUuid,
                                    UINT32 strUuidLen);

UBOOL8 cmsUtl_isValidUuid(const char *uuid);

/** Encode a data stream into hex string format.  Each nibble is encoded into
 *  a printable character by adding 0x30 to it.
 *  eg. 0x12 0xAB -> "12:;"
 *
 * @param pDst   (OUT) Pointer to destination buffer (will be NULL terminated)
 * @param dstLen (OUT) Destination buffer length
 * @param pSrc   (IN)  Pointer to source buffer
 * @param srcLen (IN)  Source buffer length
 *
 * @return Nothing
 */
void cmsUtl_encodeHexStr(char *pDst,
                         unsigned int dstLen,
                         const char *pSrc,
                         unsigned int srcLen);

/** Decode a hex string into a data stream.
 *
 * @param pDst   (OUT) Pointer to destination buffer
 * @param dstLen (OUT) Destination buffer length
 * @param pSrc   (IN)  Pointer to source buffer (must be NULL terminated)
 *
 * @return Number of bytes written to destination buffer.
 */
int cmsUtl_decodeHexStr(char *pDst,
                        unsigned int dstLen,
                        const char *pSrc);


#endif /* __CMS_STRCONV_H__ */


