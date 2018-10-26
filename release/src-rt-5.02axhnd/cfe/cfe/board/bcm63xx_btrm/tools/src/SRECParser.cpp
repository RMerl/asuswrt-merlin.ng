#include <stdio.h>
#include <cstdlib>
#include <stdint.h>
#include "SRECParser.h"

using namespace std;


#define LINE_SIZE   32

/* ---------------------------------------------------------------
    Adds a single byte to our buffer
--------------------------------------------------------------- */
static void 
setMemoryValue( 
    uint32 uAddress, 
    uint8 uByte, 
    MAPPING & clLineMap) {

    uint8 *pLine = NULL;

    pLine = clLineMap[uAddress & ~0x001F];

    // if this maps to a new line
    if (pLine == NULL) {
        pLine = new uint8[LINE_SIZE];
        pLine[uAddress % LINE_SIZE] = uByte;
        clLineMap[uAddress & ~0x001F] = pLine;

    // else line already exists, just update contents
    } else {
        pLine[uAddress % LINE_SIZE] = uByte;
    }
}

//#define BIG_ENDIAN      0
//#define LITTLE_ENDIAN   1
//int     srec_type = BIG_ENDIAN;
int     srec_type = 0;

int
parseSREC(
    const string & fname,
    MAPPING      & clLineMap,
    SYM_TABLE    & clSymTable) {

    char   buffer[201];
    const char * bufptr;
    FILE *pFile;

    char    address_str[9];
    char    count_str[3];
    char    type[3];
    char    byte_str[3];
    char    label[80];
    int     count;
    int     i;
    uint8   byte;
    uint32  address;

    cout << "Translating: " << fname.c_str() << endl;

    pFile = fopen(fname.c_str(), "r");
    if (pFile == NULL) {
        return -1;
    }

    // Go through file and read lines
    while (fgets(buffer, 200, pFile)) {
        strncpy(type,buffer,2);
        type[2] = '\0';

        if (!strcmp(type,"EL")) {
//            srec_type = LITTLE_ENDIAN;
            srec_type = 1;
            printf ("Little endian mode\n");

        } else if (!strcmp(type,"S0")) {
            bufptr = buffer;

        } else if (!strcmp(type,"S4")) {
            bufptr = buffer + 2;

            strncpy(count_str,bufptr,2);
            bufptr += 2;
            count_str[2] = '\0';

            strncpy(address_str,bufptr,8);
            bufptr += 8;
            address_str[8] = '\0';

            count   = strtoul(count_str,   '\0', 16);
            address = strtoul(address_str, '\0', 16);
            address = address & ~0xF0000000;

            strncpy(label, bufptr, (count - 11));
            label[(count - 11)] = '\0';

            clSymTable[address] = label;

        } else if (!strcmp(type,"S3")) {
            bufptr = buffer + 2;
            strncpy(count_str,bufptr,2);
            bufptr += 2;
            count_str[2] = '\0';
            strncpy(address_str,bufptr,8);
            bufptr += 8;
            address_str[8] = '\0';

            count   = strtoul(count_str,   '\0', 16);
            address = strtoul(address_str, '\0', 16);

            for (i = 0; i < (count - 5); i++) {

                strncpy(byte_str,bufptr,2);
                byte_str[2] = '\0';
                bufptr+=2;

                byte = strtoul(byte_str,'\0',16);

                setMemoryValue(address,            byte, clLineMap);
                address++;
            }
        } else if (!strcmp(type,"S1")) {
            bufptr = buffer + 2;
            strncpy(count_str,bufptr,2);
            bufptr += 2;
            count_str[2] = '\0';
            strncpy(address_str,bufptr,4);
            bufptr += 4;
            address_str[8] = '\0';

            count   = strtoul(count_str,   '\0', 16);
            address = strtoul(address_str, '\0', 16);

            for (i = 0; i < (count - 3); i++) {

                strncpy(byte_str,bufptr,2);
                byte_str[2] = '\0';
                bufptr+=2;

                byte = strtoul(byte_str,'\0',16);

                setMemoryValue(address,            byte, clLineMap);
                address++;
            }
        } else if (!strcmp(type,"S2")) {
            bufptr = buffer + 2;
            strncpy(count_str,bufptr,2);
            bufptr += 2;
            count_str[2] = '\0';
            strncpy(address_str,bufptr,6);
            bufptr += 6;
            address_str[8] = '\0';

            count   = strtoul(count_str,   '\0', 16);
            address = strtoul(address_str, '\0', 16);

            for (i = 0; i < (count - 4); i++) {

                strncpy(byte_str,bufptr,2);
                byte_str[2] = '\0';
                bufptr+=2;

                byte = strtoul(byte_str,'\0',16);

                setMemoryValue(address,            byte, clLineMap);
                address++;
            }
        } else {
            printf ("Warning, line type: %s ignored\n",type);
        }
    }

    fclose(pFile);

    return 0;
}

static uint64_t calcCRC(uint64_t d64, uint64_t crc64)
{
    uint64_t       crc_bit_63;
    crc_bit_63 = (d64>>63) ^ (crc64&1) ^ ((crc64>>8)&1) ^ ((crc64>>16)&1) ^ ((crc64>>24)&1) ^
        ((crc64>>31)&1) ^ ((crc64>>40)&1) ^ ((crc64>>49)&1) ^ ((crc64>>56)&1);
    crc64 = ((crc64>>1) ^ (d64 & 0x7fffffffffffffffull)) | (crc_bit_63 << 63);
    return crc64;
}

void initRomCRC(uint64_t *code)
{
#define BOOTROM_WIDTH	(0x8)
//#define BOOTROM_DEPTH	(0x20000/BOOTROM_WIDTH)
#define BOOTROM_DEPTH	((64*1024)/BOOTROM_WIDTH)
#define BOOTROM_CRC		(0xD42CF71BBA3FB265ull)
//#define BOOTROM_CRC		(0x136385246c907ff5ull)
        uint64_t       d64, crc64, crc_bit_63, wordnum;
        uint32_t       d32_a, d32_b;

        crc64 = 0;
        for( wordnum=0; wordnum<BOOTROM_DEPTH; wordnum++ ) {
            if( wordnum < (BOOTROM_DEPTH - 1) ) {
                d64 = code[wordnum];
                //d64 = (code[wordnum]<<32)|(code[wordnum]>>32);
            } else {
                d64 = calcCRC(BOOTROM_CRC, crc64);
            }
            crc64 = calcCRC(d64, crc64);
            //print_log("d64[%3lld]=>0x%016llx, crc64=0x%016llx\n", wordnum, d64, crc64);
        }
        printf("... Writing crc64 PATCH of rom contents 0x%016llx\n", d64 );
        code[BOOTROM_DEPTH-1]=d64;
        printf("... crc64 of rom contents 0x%016llx\n", crc64);
}

void print_bin(FILE *ofp, uint64_t val){
	int i;
	for(i=63;i>=0;i--){
		fprintf(ofp,"%d",(val>>i)&0x1);
	}
	fprintf(ofp,"\n");
}

void print_hex(FILE *ofp, int wsize, uint64_t val){
	for(int i=0;i<wsize;i++){
		fprintf(ofp,"%02X\n",(val>>(i*8))&0xffull);
	}
}

void
generate_codefile(
    MAPPING		& clLineMap,
    SYM_TABLE 	& clSymTable,
	uint32_t	addr_mask,
	bool	le,
	int	wsize,
	uint64_t *code) {

    MAPPING::iterator j;
	FILE *ofp;

	ofp=fopen("spiflash.txt","wb");
    // Add memory line writes to the specific memory component
    for (j = clLineMap.begin(); j != clLineMap.end(); j++) {
        uint32  uAddress;

        uAddress = j->first & ~addr_mask;

		for (int i = 0; i < LINE_SIZE; i+= 8) {
			uint64_t uVal;
			uint32  uTmp;

			uVal =0;
			for (int k=0;k<wsize;k++){
				//LE Packing
				uVal  |= ((uint64_t) j->second[i + k]) << (k*8);
			}
			code[uAddress/wsize+(i/wsize)]=uVal;

			uTmp = j->first + i;
			//printf("\t%08lX: %016llX\n", uTmp, uVal);
		}
        
    }
	//initRomCRC(code);
	for(int i=0;i<BOOTROM_DEPTH;i++){
	  //print_bin(ofp, code[i]);

          print_hex(ofp, wsize, code[i]);
          //insert 64 bytes spares data for every 2048 byte page data
	  if( (i + 1) % (2048/8) == 0 )
	  {
            int j; 
	    for( j = 0; j < 8; j++ )
	      print_hex(ofp, wsize, (uint64_t)(-1));
	  }
	}

    // we do not need mapping list anymore, so delete the allocated memory
    for (j = clLineMap.begin(); j != clLineMap.end(); j++) {
        delete [] (j->second);
    }
    clLineMap.clear();
	fclose(ofp);
}


void
ShowHelp(char *argv[]) {
    fprintf(stderr, "%s [file1 file2 ...]\n",argv[0]);
    exit(1);
}


int
main(
    int    argc, 
    char * argv[]) {

    int i;
    MAPPING		clMap;
    SYM_TABLE	clSymbols;
	uint64_t	code[64*1024];
	int iResult;
	char	fname[80];


    // do command line processing here...
    // parse command line...
    for (i = 1; i < argc; i++) {
        char *ptr = argv[i];

        if (*ptr == '-') {
            ptr++;
            switch(tolower(*ptr++)) {
            case 'h':
            case '?':
                ShowHelp(argv);
                break;

            default:
                ShowHelp(argv);
                break;
            }
        } else {
			strcpy(fname,ptr);
		}

    }
	printf("set memory value\n");

	for (i=0;i<16*1024;i++) {
	      // setMemoryValue(0x9fc00000+i,0,clMap);
	      // setMemoryValue(0xffe10000+i,0,clMap);
              // setMemoryValue(0xb8100000+i,0,clMap);
              setMemoryValue(0xffd00000+i,0,clMap);
	}

	printf("parse srec \n");
	iResult = parseSREC(fname, clMap, clSymbols);

	if (iResult != 0) {
		printf("Error opening: %s\n", fname);
	}

	printf("generate codefile \n");
	// generate_codefile(clMap, clSymbols, 0xb8100000, true, 8, code);
        //generate_codefile(clMap, clSymbols, 0xffe10000, true, 8, code);
	generate_codefile(clMap, clSymbols, 0xffd00000, true, 8, code);

    return 0;
}
