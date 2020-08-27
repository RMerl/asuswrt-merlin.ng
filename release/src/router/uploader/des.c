#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/des.h>

#include "base64encode.h"
#include "base64decode.h"

#define DEBUG 0


char * keyProcess(const char* keyin);


/************************************************************************
** 本例採用：
** 3des-ecb加密方式；
** 24位金鑰，不足24位的右補0x00；
** 加密內容8位元補齊，補齊方式為：少1位補一個0x01,少2位補兩個0x02,...
** 本身已8位對齊的，後面補八個0x08。
************************************************************************/


int des_encrypt(const char* datain, unsigned char* dataout, int * dataout_len, const unsigned char* keyin, int keyin_len) {


	int j;
	int docontinue = 1;

	int data_len;
	int data_rest;
	unsigned char ch;
	
	unsigned char *src = NULL; /* 補齊後的明文 */
	//unsigned char *encode_dst = NULL; /* 加密後的資料 */
	unsigned char *dst = NULL; /* 解密後的明文 */
	
	int len;
	unsigned char tmp[8];
	unsigned char in[8];
	unsigned char out[8];

	//char *keyin = keyProcess("03805FDC4B594FDEA89183D2ADA82586");	 /* 原始金鑰 */

	int key_len;
	#define LEN_OF_KEY 24
	unsigned char key[LEN_OF_KEY]; /* 補齊後的金鑰 */
	unsigned char block_key[9];
	DES_key_schedule ks,ks2,ks3;

	/* 構造補齊後的金鑰 */
	key_len = strlen(keyin);
	memcpy(key, keyin, key_len);
	memset(key + key_len, 0x00, LEN_OF_KEY - key_len);

	memset(dataout, 0, sizeof(dataout));

	if(DEBUG) {
		printf("\nkeyin arrays:\n");
		for(j = 0; j < 24; j ++) {
			printf("%d ", key[j]);
		}
		printf("\n");
	}

	/* 分析補齊明文所需空間及補齊填充資料 */
	data_len = strlen(datain);
	//printf("\data_len:%d\n%s\n", data_len, datain);

	data_rest = data_len % 8;
	len = data_len + (8 - data_rest);
	*dataout_len = len;
	ch = 8 - data_rest;

	src = (unsigned char *)malloc(len);
	dst = (unsigned char *)malloc(len);

	if (NULL == src || NULL == dst) {
		docontinue = 0;
	}

	if (docontinue) {

		int count;
		int i;
		
		/* 構造補齊後的加密內容 */
		memset(src, 0, len);
		memcpy(src, datain, data_len);
		memset(src + data_len, ch, 8 - data_rest);

		/* 金鑰置換 */
		memset(block_key, 0, sizeof(block_key));
		memcpy(block_key, key + 0, 8);
		DES_set_key_unchecked((const_DES_cblock*)block_key, &ks);
		memcpy(block_key, key + 8, 8);
		DES_set_key_unchecked((const_DES_cblock*)block_key, &ks2);
		memcpy(block_key, key + 16, 8);
		DES_set_key_unchecked((const_DES_cblock*)block_key, &ks3);

		if(DEBUG) {
			printf("\nbefore encrypt:\n");
			for (i = 0; i < len; i++) {
				printf("%d, ", *(src + i));
			}
			printf("\n");
		}

		/* 迴圈加密/解密，每8位元組一次 */
		count = len / 8;
		for (i = 0; i < count; i++) {

			memset(tmp, 0, 8);
			memset(in, 0, 8);
			memset(out, 0, 8);
			memcpy(tmp, src + 8 * i, 8);

			/* 加密 */
			//DES_ecb3_encrypt((const_DES_cblock*)tmp, (DES_cblock*)(in+8*i), &ks, &ks2, &ks3, DES_ENCRYPT);
			DES_ecb3_encrypt((const_DES_cblock*)tmp, (DES_cblock*)in, &ks, &ks2, &ks3, DES_ENCRYPT);


			/* 加密後的內容 */
			memcpy(dataout + 8 * i, in, 8);

			
			/* 解密 */
			//DES_ecb3_encrypt((const_DES_cblock*)(in+8*i), (DES_cblock*)(out+8*i), &ks, &ks2, &ks3, DES_DECRYPT);
			DES_ecb3_encrypt((const_DES_cblock*)in, (DES_cblock*)out, &ks, &ks2, &ks3, DES_DECRYPT);

			/* 將解密的內容拷貝到解密後的明文 */
			//memcpy(dst + 8 * i, (out+8*i), 8);
			memcpy(dst + 8 * i, out, 8);
		}


		//memset(dataout, 0, sizeof(dataout));
		//memcpy(dataout, in, len);
		
		if(DEBUG) {
			printf("\ndataout after encrypt len :%d:\n", len);
		
			for (i = 0; i < len; i++) {
				printf("%d, ", *(dataout + i));
			}
			printf("\n");
		}



		if(DEBUG) {

			printf("\nafter decrypt :\n\n");
			for (i = 0; i < len; i++)
			{
				printf("%d, ", *(dst + i));
			}
			printf("\n");

		}


	}

	

	if (NULL != src) {
		free(src);
		src = NULL;
	}

	if (NULL != dst) {
		free(dst);
		dst = NULL;
	}

	return 0;
}


/* input 32 char / 2 = 16byte key -> after puls 8 bytes 0x00 */
char * keyProcess(const char *keyin) {

	//char *keyin = "03805FDC4B594FDEA89183D2ADA82586";

	int keyLen = strlen(keyin)/2;
	
	char* key = (char*)malloc(sizeof(char)*keyLen + 1);
	memset(key, 0, sizeof(char)*keyLen + 1);
	int j = 0;
	char *pEnd;
	
	
	for(j - 0; j < strlen(keyin); j +=2) {
		char buf[3] = {0};
		memcpy(buf, keyin+j, 2);
		key[j/2] = strtol(buf, &pEnd, 16);
		if(DEBUG) {
			printf("key:%d: \n", key[j/2]);
		}
	}


	return key;
}
