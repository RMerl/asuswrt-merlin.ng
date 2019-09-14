/* This is the header for the output channel code of rsyslog.
 * begun 2005-06-21 rgerhards
 *
 * Copyright(C) 2005-2012 Adiscon GmbH
 *
 * This file is part of rsyslog.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *       -or-
 *       see COPYING.ASL20 in the source distribution
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef OUTCHANNEL_H
#define OUTCHANNEL_H
struct outchannel {
	struct outchannel *pNext;
	char *pszName;
	int iLenName;
	uchar *pszFileTemplate;
	off_t	uSizeLimit;
	uchar *cmdOnSizeLimit;
};

struct outchannel* ochConstruct(void);
struct outchannel *ochAddLine(char* pName, unsigned char** pRestOfConfLine);
struct outchannel *ochFind(char *pName, int iLenName);
void ochDeleteAll(void);
void ochPrintList(void);
#endif /* #ifdef OUTCHANNEL_H */
