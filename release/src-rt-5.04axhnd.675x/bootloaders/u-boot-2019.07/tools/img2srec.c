/*************************************************************************
|  COPYRIGHT (c) 2000 BY ABATRON AG
|*************************************************************************
|
|  PROJECT NAME: Linux Image to S-record Conversion Utility
|  FILENAME    : img2srec.c
|
|  COMPILER    : GCC
|
|  TARGET OS   : LINUX / UNIX
|  TARGET HW   : -
|
|  PROGRAMMER  : Abatron / RD
|  CREATION    : 07.07.00
|
|*************************************************************************
|
|  DESCRIPTION :
|
|  Utility to convert a Linux Boot Image to S-record:
|  ==================================================
|
|  This command line utility can be used to convert a Linux boot image
|  (zimage.initrd) to S-Record format used for flash programming.
|  This conversion takes care of the special sections "IMAGE" and INITRD".
|
|  img2srec [-o offset] image > image.srec
|
|
|  Build the utility:
|  ==================
|
|  To build the utility use GCC as follows:
|
|  gcc img2srec.c -o img2srec
|
|
|*************************************************************************
|
|
|  UPDATES     :
|
|  DATE      NAME  CHANGES
|  -----------------------------------------------------------
|  Latest update
|
|  07.07.00  aba   Initial release
|
|*************************************************************************/

/*************************************************************************
|  INCLUDES
|*************************************************************************/

#include "os_support.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <elf.h>
#include <unistd.h>
#include <errno.h>

/*************************************************************************
|  FUNCTIONS
|*************************************************************************/

static char* ExtractHex (uint32_t* value,  char* getPtr)
{
  uint32_t num;
  uint32_t digit;
  uint8_t  c;

  while (*getPtr == ' ') getPtr++;
  num = 0;
  for (;;) {
    c = *getPtr;
    if      ((c >= '0') && (c <= '9')) digit = (uint32_t)(c - '0');
    else if ((c >= 'A') && (c <= 'F')) digit = (uint32_t)(c - 'A' + 10);
    else if ((c >= 'a') && (c <= 'f')) digit = (uint32_t)(c - 'a' + 10);
    else break;
    num <<= 4;
    num += digit;
    getPtr++;
  } /* for */
  *value = num;
  return getPtr;
} /* ExtractHex */

static char* ExtractDecimal (uint32_t* value,  char* getPtr)
{
  uint32_t num;
  uint32_t digit;
  uint8_t  c;

  while (*getPtr == ' ') getPtr++;
  num = 0;
  for (;;) {
    c = *getPtr;
    if      ((c >= '0') && (c <= '9')) digit = (uint32_t)(c - '0');
    else break;
    num *= 10;
    num += digit;
    getPtr++;
  } /* for */
  *value = num;
  return getPtr;
} /* ExtractDecimal */


static void ExtractNumber (uint32_t* value,  char* getPtr)
{
  bool  neg = false;

  while (*getPtr == ' ') getPtr++;
  if (*getPtr == '-') {
    neg = true;
    getPtr++;
  } /* if */
  if ((*getPtr == '0') && ((*(getPtr+1) == 'x') || (*(getPtr+1) == 'X'))) {
    getPtr +=2;
    (void)ExtractHex(value, getPtr);
  } /* if */
  else {
    (void)ExtractDecimal(value, getPtr);
  } /* else */
  if (neg) *value = -(*value);
} /* ExtractNumber */


static uint8_t* ExtractWord(uint16_t* value, uint8_t* buffer)
{
  uint16_t x;
  x = (uint16_t)*buffer++;
  x = (x<<8) + (uint16_t)*buffer++;
  *value = x;
  return buffer;
} /* ExtractWord */


static uint8_t* ExtractLong(uint32_t* value, uint8_t* buffer)
{
  uint32_t x;
  x = (uint32_t)*buffer++;
  x = (x<<8) + (uint32_t)*buffer++;
  x = (x<<8) + (uint32_t)*buffer++;
  x = (x<<8) + (uint32_t)*buffer++;
  *value = x;
  return buffer;
} /* ExtractLong */


static uint8_t* ExtractBlock(uint16_t count, uint8_t* data, uint8_t* buffer)
{
  while (count--) *data++ = *buffer++;
  return buffer;
} /* ExtractBlock */


static char* WriteHex(char* pa, uint8_t value, uint16_t* pCheckSum)
{
  uint16_t  temp;

  static  char ByteToHex[] = "0123456789ABCDEF";

  *pCheckSum += value;
  temp  = value / 16;
  *pa++ = ByteToHex[temp];
  temp  = value % 16;
  *pa++ = ByteToHex[temp];
  return pa;
}


static char* BuildSRecord(char* pa, uint16_t sType, uint32_t addr,
			  const uint8_t* data, int nCount)
{
  uint16_t  addrLen;
  uint16_t  sRLen;
  uint16_t  checkSum;
  uint16_t  i;

  switch (sType) {
  case 0:
  case 1:
  case 9:
    addrLen = 2;
    break;
  case 2:
  case 8:
    addrLen = 3;
    break;
  case 3:
  case 7:
    addrLen = 4;
    break;
  default:
    return pa;
  } /* switch */

  *pa++ = 'S';
  *pa++ = (char)(sType + '0');
  sRLen = addrLen + nCount + 1;
  checkSum = 0;
  pa = WriteHex(pa, (uint8_t)sRLen, &checkSum);

  /* Write address field */
  for (i = 1; i <= addrLen; i++) {
    pa = WriteHex(pa, (uint8_t)(addr >> (8 * (addrLen - i))), &checkSum);
  } /* for */

  /* Write code/data fields */
  for (i = 0; i < nCount; i++) {
    pa = WriteHex(pa, *data++, &checkSum);
  } /* for */

  /* Write checksum field */
  checkSum = ~checkSum;
  pa = WriteHex(pa, (uint8_t)checkSum, &checkSum);
  *pa++ = '\0';
  return pa;
}


static void ConvertELF(char* fileName, uint32_t loadOffset)
{
  FILE*         file;
  int           i;
  int           rxCount;
  uint8_t          rxBlock[1024];
  uint32_t         loadSize;
  uint32_t         firstAddr;
  uint32_t         loadAddr;
  uint32_t         loadDiff = 0;
  Elf32_Ehdr    elfHeader;
  Elf32_Shdr    sectHeader[32];
  uint8_t*         getPtr;
  char          srecLine[128];
  char		*hdr_name;


  /* open file */
  if ((file = fopen(fileName,"rb")) == NULL) {
    fprintf (stderr, "Can't open %s: %s\n", fileName, strerror(errno));
    return;
  } /* if */

  /* read ELF header */
  rxCount = fread(rxBlock, 1, sizeof elfHeader, file);
  getPtr = ExtractBlock(sizeof elfHeader.e_ident, elfHeader.e_ident, rxBlock);
  getPtr = ExtractWord(&elfHeader.e_type, getPtr);
  getPtr = ExtractWord(&elfHeader.e_machine, getPtr);
  getPtr = ExtractLong((uint32_t *)&elfHeader.e_version, getPtr);
  getPtr = ExtractLong((uint32_t *)&elfHeader.e_entry, getPtr);
  getPtr = ExtractLong((uint32_t *)&elfHeader.e_phoff, getPtr);
  getPtr = ExtractLong((uint32_t *)&elfHeader.e_shoff, getPtr);
  getPtr = ExtractLong((uint32_t *)&elfHeader.e_flags, getPtr);
  getPtr = ExtractWord(&elfHeader.e_ehsize, getPtr);
  getPtr = ExtractWord(&elfHeader.e_phentsize, getPtr);
  getPtr = ExtractWord(&elfHeader.e_phnum, getPtr);
  getPtr = ExtractWord(&elfHeader.e_shentsize, getPtr);
  getPtr = ExtractWord(&elfHeader.e_shnum, getPtr);
  getPtr = ExtractWord(&elfHeader.e_shstrndx, getPtr);
  if (    (rxCount              != sizeof elfHeader)
       || (elfHeader.e_ident[0] != ELFMAG0)
       || (elfHeader.e_ident[1] != ELFMAG1)
       || (elfHeader.e_ident[2] != ELFMAG2)
       || (elfHeader.e_ident[3] != ELFMAG3)
       || (elfHeader.e_type     != ET_EXEC)
     ) {
    fclose(file);
    fprintf (stderr, "*** illegal file format\n");
    return;
  } /* if */

  /* read all section headers */
  fseek(file, elfHeader.e_shoff, SEEK_SET);
  for (i = 0; i < elfHeader.e_shnum; i++) {
    rxCount = fread(rxBlock, 1, sizeof sectHeader[0], file);
    getPtr = ExtractLong((uint32_t *)&sectHeader[i].sh_name, rxBlock);
    getPtr = ExtractLong((uint32_t *)&sectHeader[i].sh_type, getPtr);
    getPtr = ExtractLong((uint32_t *)&sectHeader[i].sh_flags, getPtr);
    getPtr = ExtractLong((uint32_t *)&sectHeader[i].sh_addr, getPtr);
    getPtr = ExtractLong((uint32_t *)&sectHeader[i].sh_offset, getPtr);
    getPtr = ExtractLong((uint32_t *)&sectHeader[i].sh_size, getPtr);
    getPtr = ExtractLong((uint32_t *)&sectHeader[i].sh_link, getPtr);
    getPtr = ExtractLong((uint32_t *)&sectHeader[i].sh_info, getPtr);
    getPtr = ExtractLong((uint32_t *)&sectHeader[i].sh_addralign, getPtr);
    getPtr = ExtractLong((uint32_t *)&sectHeader[i].sh_entsize, getPtr);
    if (rxCount != sizeof sectHeader[0]) {
      fclose(file);
      fprintf (stderr, "*** illegal file format\n");
      return;
    } /* if */
  } /* for */

  if ((hdr_name = strrchr(fileName, '/')) == NULL) {
    hdr_name = fileName;
  } else {
    ++hdr_name;
  }
  /* write start record */
  (void)BuildSRecord(srecLine, 0, 0, (uint8_t *)hdr_name, strlen(hdr_name));
  printf("%s\r\n",srecLine);

  /* write data records */
  firstAddr = ~0;
  loadAddr  =  0;
  for (i = 0; i < elfHeader.e_shnum; i++) {
    if (    (sectHeader[i].sh_type == SHT_PROGBITS)
	 && (sectHeader[i].sh_size != 0)
	 ) {
      loadSize = sectHeader[i].sh_size;
      if (sectHeader[i].sh_flags != 0) {
	loadAddr = sectHeader[i].sh_addr;
	loadDiff = loadAddr - sectHeader[i].sh_offset;
      } /* if */
      else {
	loadAddr = sectHeader[i].sh_offset + loadDiff;
      } /* else */

      if (loadAddr < firstAddr)
	firstAddr = loadAddr;

      /* build s-records */
      loadSize = sectHeader[i].sh_size;
      fseek(file, sectHeader[i].sh_offset, SEEK_SET);
      while (loadSize) {
	rxCount = fread(rxBlock, 1, (loadSize > 32) ? 32 : loadSize, file);
	if (rxCount < 0) {
	  fclose(file);
	  fprintf (stderr, "*** illegal file format\n");
	return;
	} /* if */
	(void)BuildSRecord(srecLine, 3, loadAddr + loadOffset, rxBlock, rxCount);
	loadSize -= rxCount;
	loadAddr += rxCount;
	printf("%s\r\n",srecLine);
      } /* while */
    } /* if */
  } /* for */

  /* add end record */
  (void)BuildSRecord(srecLine, 7, firstAddr + loadOffset, 0, 0);
  printf("%s\r\n",srecLine);
  fclose(file);
} /* ConvertELF */


/*************************************************************************
|  MAIN
|*************************************************************************/

int main( int argc, char *argv[ ])
{
  uint32_t offset;

  if (argc == 2) {
    ConvertELF(argv[1], 0);
  } /* if */
  else if ((argc == 4) && (strcmp(argv[1], "-o") == 0)) {
    ExtractNumber(&offset, argv[2]);
    ConvertELF(argv[3], offset);
  } /* if */
  else {
    fprintf (stderr, "Usage: img2srec [-o offset] <image>\n");
  } /* if */

  return 0;
} /* main */
