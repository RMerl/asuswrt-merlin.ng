/* header for parser.c
 *
 * Copyright 2008-2016 Adiscon GmbH.
 *
 * This file is part of the rsyslog runtime library.
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
#ifndef INCLUDED_PARSER_H
#define INCLUDED_PARSER_H

/* we create a small helper object, a list of parsers, that we can use to
 * build a chain of them whereever this is needed (initially thought to be
 * used in ruleset.c as well as ourselvs).
 */
struct parserList_s {
	parser_t *pParser;
	parserList_t *pNext;
};


/* the parser object, a dummy because we have only static methods */
struct parser_s {
	BEGINobjInstance;	/* Data to implement generic object - MUST be the first data element! */
	uchar *pName;		/* name of this parser */
	modInfo_t *pModule;	/* pointer to parser's module */
	void *pInst;		/* instance data for the parser (v2+ module interface) */
	sbool bDoSanitazion;	/* do standard message sanitazion before calling parser? */
	sbool bDoPRIParsing;	/* do standard PRI parsing before calling parser? */
};

/* interfaces */
BEGINinterface(parser) /* name must also be changed in ENDinterface macro! */
	INTERFACEObjDebugPrint(var);
	rsRetVal (*Construct)(parser_t **ppThis);
	rsRetVal (*ConstructFinalize)(parser_t *pThis);
	rsRetVal (*Destruct)(parser_t **ppThis);
	rsRetVal (*SetName)(parser_t *pThis, uchar *name);
	rsRetVal (*SetModPtr)(parser_t *pThis, modInfo_t *pMod);
	rsRetVal (*SetDoPRIParsing)(parser_t *pThis, int);
	rsRetVal (*FindParser)(parser_t **ppThis, uchar*name);
	rsRetVal (*InitParserList)(parserList_t **pListRoot);
	rsRetVal (*DestructParserList)(parserList_t **pListRoot);
	rsRetVal (*AddParserToList)(parserList_t **pListRoot, parser_t *pParser);
	/* static functions */
	rsRetVal (*ParseMsg)(smsg_t *pMsg);
	rsRetVal (*SanitizeMsg)(smsg_t *pMsg);
	rsRetVal (*AddDfltParser)(uchar *);
ENDinterface(parser)
#define parserCURR_IF_VERSION 2 /* increment whenever you change the interface above! */
/* version changes
	2       SetDoSanitization removed, no longer needed
*/

void printParserList(parserList_t *pList);

/* prototypes */
PROTOTYPEObj(parser);
rsRetVal parserConstructViaModAndName(modInfo_t *pMod, uchar *const pName, void *parserInst);


#endif /* #ifndef INCLUDED_PARSER_H */
