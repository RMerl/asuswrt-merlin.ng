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

#ifndef __CMS_TOKENIZER_H__
#define __CMS_TOKENIZER_H__


/*!\file cms_tokenizer.h
 * \brief Header file for the CMS Tokenizer API.
 *  This is in the cms_util library.
 *
 */

#include "cms.h"


struct cms_token_map_entry {
   char *keyword;
   int token;
};

#define MAX_TOKEN_VALUE_LEN             512

struct cms_token_value_pair {
   int  token;
   char valuebuf[MAX_TOKEN_VALUE_LEN];
};


/** Given a line and a map of known keyword strings to token id's,
 * return token id and value string.
 *
 *@param line (IN) a single line to tokenize
 *@param len  (IN) length of line
 *@param token_map (IN) array of cms_token_map_entry structs, terminated by
 *                      a final element whose keyword string is NULL.
 *@param pair (OUT) the found token id and value string pair.
 *
 *@return TRUE if successful, FALSE otherwise.
 */
UBOOL8 cmsTok_tokenizeLine(const char *line, UINT32 len,
                           const struct cms_token_map_entry *token_map,
                           struct cms_token_value_pair *pair);


/** Return TRUE if the given line is a non-commented and non-blank line.
 *
 *@param buf  (IN) line buffer.
 *@param len  (IN) length of line buffer.
 *
 *@return TRUE if this is a non-commented and non-blank line.
 */
UBOOL8 cmsTok_isDataLine(const char *buf, int len);


/** Get the next non-blank and non-comment line.
 *
 *@param fp (IN) file pointer
 *@param line (OUT) caller passes in a buffer to store the line
 *@param len  (IN/OUT) On entry, contains length of line buffer.  On exit,
 *                     contains number of characters in the line.
 *
 *@return TRUE on data line found.  FALSE otherwise.
 */
UBOOL8 cmsTok_getNextDataLine(FILE *fp, char *line, UINT32 *len);

#endif /* __CMS_TOKENIZER_H__ */
