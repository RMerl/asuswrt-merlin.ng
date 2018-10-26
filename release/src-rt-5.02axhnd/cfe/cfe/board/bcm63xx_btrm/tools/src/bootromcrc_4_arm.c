#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#define MAJOR_VERSION	1
#define MINOR_VERSION	1

static uint64_t calcCRC(uint64_t d64, uint64_t crc64)
{
    uint64_t       crc_bit_63;
    crc_bit_63 = (d64>>63) ^ (crc64&1) ^ ((crc64>>8)&1) ^ ((crc64>>16)&1) ^ ((crc64>>24)&1) ^
        ((crc64>>31)&1) ^ ((crc64>>40)&1) ^ ((crc64>>49)&1) ^ ((crc64>>56)&1);
    crc64 = ((crc64>>1) ^ (d64 & 0x7fffffffffffffffull)) | (crc_bit_63 << 63);
    return crc64;
}


static void genCRC(uint32_t *data){

#define BOOTROM_WIDTH    (0x8)
#define BOOTROM_DEPTH    (0x20000/BOOTROM_WIDTH)
#define BOOTROM_CRC        (0xD42CF71BBA3FB265ull)
	uint64_t       d64, crc64, wordnum;
	uint32_t       d32_a, d32_b;

	crc64 = 0;
	for( wordnum=0; wordnum<BOOTROM_DEPTH; wordnum++ ) {
		if( wordnum < (BOOTROM_DEPTH - 1) ) {
			d32_a = data[wordnum*2 + 1];
			d32_b = data[wordnum*2 + 0];
			d64 = d32_a;
			d64 = (d64 << 32) | d32_b;
		} else {
			d64 = calcCRC(BOOTROM_CRC, crc64);
		}
		crc64 = calcCRC(d64, crc64);
		//print_log("d64[%3lld]=>0x%016llx, crc64=0x%016llx\n", wordnum, d64, crc64);
	}
	printf("... Writing crc64 PATCH of rom contents 0x%016llx\n", d64 );
	data[ (wordnum-1)*2 + 0]= (d64 >>  0);
	data[ (wordnum-1)*2 + 1]= (d64 >> 32);
	printf("Wordnum:%x %d\n",wordnum, wordnum);
	printf("... crc64 of rom contents 0x%016llx\n", crc64);
}
void usage(char *name){
	printf("\nUsage\n");
	printf("-----\n\n");
	printf("Version: %d.%d\n\n",MAJOR_VERSION,MINOR_VERSION);
	printf("Utility takes a binary input, calculates the 64-bit CRC and \n");
	printf("generates a output binary file and codefile format data files \n");
	printf("with the CRC appended.                                      \n\n");
	printf("Input file is padded to 128K. Rom is assumed 64-bit wide.     \n");
	printf("CRC is inserted into the \"last\" word of the 128K.         \n\n");
	printf("%s <-ibin inputfile> <-obin outputfile> <-ocode output_codefile> <-h>\n", name);
	printf("\n-ibin  <file>:  specified binary input file\n");
	printf("-obin  <file>:  specified binary output file (with CRC applied)\n");
	printf("-ocode <file>:  specified codefile format output file (with CRC)\n\n");
}
	
int main (int argc, char *argv[]){
	FILE *ifp, *ofp;
	char *ibfname, *obfname, *ocfname;
	size_t num;
	uint32_t  data[32*1024];
	bool ibin=false;
	bool obin=false;
	bool ocode=false;

	if (argc>1) {
		char **arg;
		arg=argv;
		while(arg[0]){
			if (strcmp("-ibin",arg[0])==0) {
				arg++;
				ibfname=arg[0];
				ibin=true;
				printf("Binary Input File: %s\n",ibfname);
			}
			if (strcmp("-obin",arg[0])==0) {
				arg++;
				obfname=arg[0];
				obin=true;
				printf("Binary Output File: %s\n",obfname);
			}
			if (strcmp("-ocode",arg[0])==0) {
				arg++;
				ocfname=arg[0];
				ocode=true;
				printf("Codefile Output File: %s\n",ocfname);
			}

			if ((strcmp("-h",arg[0])==0)|
				(strcmp("-?",arg[0])==0)) {
				usage(argv[0]);
				exit(0);
			}
			arg++;
		}
	} else {
		usage(argv[0]);
		return (0);
	}

	if (!ibin) {
		printf("INPUT file NOT Specified!!!\n");
		usage(argv[0]);
		exit(0);
	}

	ifp=fopen(ibfname,"rb");
	if (ifp==NULL){
		printf("Error opening input file:%s\n",ibfname);
		exit(1);
	}
	memset(data,0,128*1024);
	num=fread(data,4,128*1024,ifp);
	printf("Read %d bytes\n",num*4);
	genCRC(data);

	if (obin) {
		ofp=fopen(obfname,"wb");
		if (ofp) {
			num=fwrite(data,4,128/4*1024,ofp);
			printf("Wrote %d bytes (binary)\n",num*4);
			fclose(ofp);
		} else {
			printf("Couldn't open binary output file:%s\n",obfname);
		}
	}
	if (ocode) {
		uint64_t d64, d64i;
		char str[80];
		int word;
		int i;
		ofp=fopen(ocfname,"wb");
		if (ofp) {
			for( word=0; word<BOOTROM_DEPTH; word+=1 ) {
			  d64i=d64 = ((uint64_t)data[2*word + 1] << 32) |
						  ((uint64_t)data[2*word] );
			  str[0]='\0';
			  for(i=0;i<64;i++){
				strcat(str,((d64&0x8000000000000000ull)?"1":"0"));
				d64<<=1;
			  }
			  fprintf(ofp,"%s   ////%016llx\n",str,d64i);
			}
			fclose(ofp);
		} else {
			printf("Couldn't open codefile output file:%s\n",ocfname);
		}
	}

}
