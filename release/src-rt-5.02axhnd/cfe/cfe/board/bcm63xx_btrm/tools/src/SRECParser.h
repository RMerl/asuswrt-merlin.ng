
#ifndef SRECPARSER_H_
#define SRECPARSER_H_

//#ifdef  _WIN32
#pragma warning (disable : 4786) 
//#endif

#include <stdio.h>
#include <iostream>
#include <string.h>
#include <string>
#include <map>

#include <assert.h>
#include "bcmtypes.h"

using namespace std;

typedef std::map<uint32, uint8 *> MAPPING;
typedef std::map<uint32, string>  SYM_TABLE;

//extern void 
//parseSREC(
//  const string & fname);

#endif // SRECPARSER_H_

