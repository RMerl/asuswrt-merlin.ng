/***********************************************************************
 *
 *  Copyright (c) 2010  Broadcom Corporation
 *  All Rights Reserved
 *
<:label-BRCM:2012:DUAL/GPL:standard

Unless you and Broadcom execute a separate written software license
agreement governing use of this software, this software is licensed
to you under the terms of the GNU General Public License version 2
(the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
with the following added to such license:

   As a special exception, the copyright holders of this software give
   you permission to link this software with independent modules, and
   to copy and distribute the resulting executable under terms of your
   choice, provided that you also meet, for each linked independent
   module, the terms and conditions of the license of that module.
   An independent module is a module which is not derived from this
   software.  The special exception does not apply to any modifications
   of the software.

Not withstanding the above, under no circumstances may you combine
this software in any way with any other Broadcom software provided
under a license other than the GPL, without Broadcom's express prior
written consent.

:>
 *
 ************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <err.h>
#include <stdint.h>
#include <getopt.h>
#include <time.h>
#include <sys/stat.h>

/* Required for CRC table */
#define BCMTAG_EXE_USE
#include <bcmTag.h>
//#include <bcm_crc.h>
#include "include/bcmTargetEndian.h"

void dump_hex(const unsigned char *buf, uint32_t len)
{
    uint32_t i;
    
    for(i=0; i<len ;i++)
        printf("%02X ",(unsigned char)buf[i]);

    printf("\n");
}
/*
static unsigned long local_crc32_table[256] = {
    0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA,
    0x076DC419, 0x706AF48F, 0xE963A535, 0x9E6495A3,
    0x0EDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988,
    0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91,
    0x1DB71064, 0x6AB020F2, 0xF3B97148, 0x84BE41DE,
    0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7,
    0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC,
    0x14015C4F, 0x63066CD9, 0xFA0F3D63, 0x8D080DF5,
    0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172,
    0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B,
    0x35B5A8FA, 0x42B2986C, 0xDBBBC9D6, 0xACBCF940,
    0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59,
    0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116,
    0x21B4F4B5, 0x56B3C423, 0xCFBA9599, 0xB8BDA50F,
    0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924,
    0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D,
    0x76DC4190, 0x01DB7106, 0x98D220BC, 0xEFD5102A,
    0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433,
    0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818,
    0x7F6A0DBB, 0x086D3D2D, 0x91646C97, 0xE6635C01,
    0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E,
    0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457,
    0x65B0D9C6, 0x12B7E950, 0x8BBEB8EA, 0xFCB9887C,
    0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65,
    0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2,
    0x4ADFA541, 0x3DD895D7, 0xA4D1C46D, 0xD3D6F4FB,
    0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0,
    0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9,
    0x5005713C, 0x270241AA, 0xBE0B1010, 0xC90C2086,
    0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F,
    0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4,
    0x59B33D17, 0x2EB40D81, 0xB7BD5C3B, 0xC0BA6CAD,
    0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A,
    0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683,
    0xE3630B12, 0x94643B84, 0x0D6D6A3E, 0x7A6A5AA8,
    0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1,
    0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE,
    0xF762575D, 0x806567CB, 0x196C3671, 0x6E6B06E7,
    0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC,
    0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5,
    0xD6D6A3E8, 0xA1D1937E, 0x38D8C2C4, 0x4FDFF252,
    0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B,
    0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60,
    0xDF60EFC3, 0xA867DF55, 0x316E8EEF, 0x4669BE79,
    0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236,
    0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F,
    0xC5BA3BBE, 0xB2BD0B28, 0x2BB45A92, 0x5CB36A04,
    0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D,
    0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A,
    0x9C0906A9, 0xEB0E363F, 0x72076785, 0x05005713,
    0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38,
    0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21,
    0x86D3D2D4, 0xF1D4E242, 0x68DDB3F8, 0x1FDA836E,
    0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777,
    0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C,
    0x8F659EFF, 0xF862AE69, 0x616BFFD3, 0x166CCF45,
    0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2,
    0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB,
    0xAED16A4A, 0xD9D65ADC, 0x40DF0B66, 0x37D83BF0,
    0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9,
    0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6,
    0xBAD03605, 0xCDD70693, 0x54DE5729, 0x23D967BF,
    0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94,
    0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D
};


unsigned int crc_getCrc32(const unsigned char *pdata, uint32_t size, uint32_t crc)
{
   dump_hex(pdata,size);
   while (size-- > 0)
      crc = (crc >> 8) ^ local_crc32_table[(crc ^ *pdata++) & 0xff];

   return crc;
}
*/
static char usage[] = "Usage:\t%s {-h|--help}\n"
"\t#set combo header globol parameters\n"
"\t\t[--version-min] #set minimal version audiance\n"
"\t\t[--version-min] #set maximal version audiance\n"
"\t\t--blocksize {64|128}\n"
"\t\t--output-file<filename>\n"
"\t#Specify images (up to three)\n"
"\t--image <chipid>,<boardid>,<image> #Up to three images\n"
"\t\t <chipid>  #As set in BRCM_CHIP, i.e 6838\n"
"\t\t <boardid> #As set in BRCM_BOARD_ID. For example 968380FHGU\n"
"\t\t <image>   #Image file\n";

static struct option longopts[] = {
    { "help", no_argument, 0, 'h' },
    { "output-file",required_argument, 0, 0},
    { "header-flags",required_argument, 0, 0},
    { "blocksize", required_argument, 0, 0},
    { "version", required_argument, 0, 0},
    { "version-min", required_argument, 0, 0},
    { "version-max", required_argument, 0, 0},
    { "combo-image-revision", required_argument, 0, 0},
    { "image", required_argument, 0, 0},
    { 0, 0, 0, 0 }
};

#define DEBUG
#define COMBO_MAX_NUM_IMAGES 20
#define IMG_FULL_PATH_AND_NAME_LEN 256

typedef struct
{
    Comboimg_individual_img_tag img_tag;
    char image_full_name[IMG_FULL_PATH_AND_NAME_LEN];
} combo_image_cfg;

typedef struct 
{
    Comboimg_header_tag header_tag;
    char output_full_name[IMG_FULL_PATH_AND_NAME_LEN];
    combo_image_cfg image_cfg[COMBO_MAX_NUM_IMAGES];
    uint32_t blocksize;
} combo_header_cfg;

void dump_cfg_data(combo_header_cfg *header_cfg)
{
    int i=0;
    printf("magic_num: ");
    for(i=0; i<COMBOIMG_MAGIC_NUM_LEN; i++)
        printf("%02X ",header_cfg->header_tag.magic_num[i]);
    printf("\n");
        
    printf("header_len: %8x\n",header_cfg->header_tag.header_len);
    printf("header_crc: %8x\n",header_cfg->header_tag.header_crc);
    printf("header_version: %8x\n",header_cfg->header_tag.header_version);
    printf("version_min: %8x\n",header_cfg->header_tag.version_min);
    printf("version_max: %8x\n",header_cfg->header_tag.version_max);
    printf("combo_image_revision: %8x\n",header_cfg->header_tag.combo_image_revision);
    printf("image_count: %8x\n",header_cfg->header_tag.image_count);
    printf("header_flags: %8x\n",header_cfg->header_tag.header_flags);
    printf("next_tag_offset: %8x\n",header_cfg->header_tag.next_tag_offset);
    printf("extended_combo_header: %8x\n",header_cfg->header_tag.extended_combo_header);
    printf("blocksize: %8x\n",header_cfg->blocksize);
    printf("output_file: %s\n",header_cfg->output_full_name);
    printf("---individual image information---\n");
    for (i=0; i<header_cfg->header_tag.image_count; i++)
    {
        printf("%8x\n",header_cfg->image_cfg[i].img_tag.chip_id);
        printf("%s\n",header_cfg->image_cfg[i].img_tag.board_id);
        printf("image len %8x\n",header_cfg->image_cfg[i].img_tag.image_len);
        printf("image offset %8x\n",header_cfg->image_cfg[i].img_tag.image_offset);
        printf("%8x\n",header_cfg->image_cfg[i].img_tag.image_flags);
        printf("%8x\n",header_cfg->image_cfg[i].img_tag.next_tag_offset);
        printf("%8x\n",header_cfg->image_cfg[i].img_tag.extended_image_header);
        printf("image name %s\n",header_cfg->image_cfg[i].image_full_name);
    }
    return;
}

/***************************************************************************
// Function Name: getCrc32
// Description  : caculate the CRC 32 of the given data.
// Parameters   : pdata - array of data.
//                size - number of input data bytes.
//                crc - either CRC32_INIT_VALUE or previous return value.
// Returns      : crc.
****************************************************************************/
uint32_t getCrc32(uint8_t *pdata, uint32_t size, uint32_t crc)
{
    while (size-- > 0)
        crc = (crc >> 8) ^ Crc32_table[(crc ^ *pdata++) & 0xff];

    return crc;
}

int get_filelen(char * filename)
{
    struct stat buf;
 
    printf("in file len\n");   
    if(stat(filename, &buf))
        errx(errno,"%s %s\n","cant open file", filename);
            
    return buf.st_size;
}

static const char magic_num[]=COMBOIMG_MAGIC_NUM;

int create_combo_image(combo_header_cfg *header_cfg)
{
   uint8_t *header_pad;
   int i=0;
   uint32_t accumulated_offset=0;
   Comboimg_header_tag *tmp_header_tag=NULL;
   Comboimg_individual_img_tag *tmp_image_tag=NULL; 
   FILE *outfd = NULL;

   outfd=fopen(header_cfg->output_full_name,"wb");

   if(!outfd)
       errx(errno,"can't open output file\n");
   /*else
      printf("here is outfd %d\n",outfd);
      */
   printf("here is outfd %d\n",1);

   printf("image_count is %d\n",header_cfg->header_tag.image_count);

   /* ready header */
   memcpy(header_cfg->header_tag.magic_num, magic_num, COMBOIMG_MAGIC_NUM_LEN);

   header_cfg->header_tag.header_len = sizeof(Comboimg_header_tag) + 
       sizeof(Comboimg_individual_img_tag) * header_cfg->header_tag.image_count;

   header_cfg->header_tag.header_version = COMBOIMG_HEADER_VERSION; 

   /*if (header_cfg->header_tag.combo_image_revision == 0)
       header_cfg->header_tag.combo_image_revision = time(NULL);
  */
  /* 4B, extended_combo_header */
   header_cfg->header_tag.next_tag_offset = 4; 

   accumulated_offset = header_cfg->blocksize;

   for(i=0; i<header_cfg->header_tag.image_count; i++)
   {
       //printf("XXXXXXX\n");
       header_cfg->image_cfg[i].img_tag.image_len
           = get_filelen(header_cfg->image_cfg[i].image_full_name);

       header_cfg->image_cfg[i].img_tag.image_offset = accumulated_offset;
       accumulated_offset += header_cfg->image_cfg[i].img_tag.image_len;
       /* 4B, extended_combo_header */
       header_cfg->image_cfg[i].img_tag.next_tag_offset = 4; 
   }

   dump_cfg_data(header_cfg);

   /*create block size header pad */
    header_pad=calloc(header_cfg->blocksize, sizeof(uint8_t));
    if(!header_pad)
        errx(errno,"%s\n","can't allocate header pad");

    
    /* put tags in pad */
    memcpy(header_pad, &header_cfg->header_tag, sizeof(Comboimg_header_tag));
  
   /*
    * //uint8_t buffer[1024]; 
    //if(!fwrite(buffer,1,1024,ofd))
    //if(!fwrite(header_pad,1,64,ofd))
    */
    for(i=0; i<header_cfg->header_tag.image_count; i++)
    {
        memcpy(header_pad+sizeof(Comboimg_header_tag) + i*sizeof(Comboimg_individual_img_tag),
            &header_cfg->image_cfg[i].img_tag, sizeof(Comboimg_individual_img_tag) );
    }
    

    tmp_header_tag=(Comboimg_header_tag*)header_pad;

   
    printf("processed configuartion\n");
    dump_cfg_data(header_cfg);

    BCM_SET_TARGET_ENDIANESS(BCM_TARGET_BIG_ENDIAN);
    tmp_header_tag->header_len = BCM_HOST_TO_TARGET32(tmp_header_tag->header_len);
    tmp_header_tag->header_version = BCM_HOST_TO_TARGET32(tmp_header_tag->header_version);
    tmp_header_tag->version_min = BCM_HOST_TO_TARGET32(tmp_header_tag->version_min);
    tmp_header_tag->version_max = BCM_HOST_TO_TARGET32(tmp_header_tag->version_max);
    tmp_header_tag->combo_image_revision = BCM_HOST_TO_TARGET32(tmp_header_tag->combo_image_revision);
    tmp_header_tag->image_count = BCM_HOST_TO_TARGET32(tmp_header_tag->image_count);
    tmp_header_tag->header_flags = BCM_HOST_TO_TARGET32(tmp_header_tag->header_flags);
    tmp_header_tag->next_tag_offset = BCM_HOST_TO_TARGET32(tmp_header_tag->next_tag_offset);
    tmp_header_tag->extended_combo_header = BCM_HOST_TO_TARGET32(tmp_header_tag->extended_combo_header);


    for(i=0; i<header_cfg->header_tag.image_count; i++)
    {
        tmp_image_tag = (Comboimg_individual_img_tag*) (header_pad 
            + sizeof(Comboimg_header_tag) + i*sizeof(Comboimg_individual_img_tag) );

        tmp_image_tag->chip_id = BCM_HOST_TO_TARGET32(tmp_image_tag->chip_id);
        tmp_image_tag->image_len = BCM_HOST_TO_TARGET32(tmp_image_tag->image_len);
        tmp_image_tag->image_offset = BCM_HOST_TO_TARGET32(tmp_image_tag->image_offset);
        tmp_image_tag->image_flags = BCM_HOST_TO_TARGET32(tmp_image_tag->image_flags);
        tmp_image_tag->next_tag_offset = BCM_HOST_TO_TARGET32(tmp_image_tag->next_tag_offset);
        tmp_image_tag->extended_image_header = BCM_HOST_TO_TARGET32(tmp_image_tag->extended_image_header);
    }

    header_cfg->header_tag.header_crc= getCrc32(header_pad,
        header_cfg->header_tag.header_len, CRC32_INIT_VALUE);

    tmp_header_tag->header_crc=header_cfg->header_tag.header_crc;


    printf("calculated crc for buffer size: %d, reducde length %d,result 0x%8X\n",
        header_cfg->header_tag.header_len,header_cfg->header_tag.header_len -20,tmp_header_tag->header_crc);
   
    tmp_header_tag->header_crc = BCM_HOST_TO_TARGET32(tmp_header_tag->header_crc);


   if(!fwrite(header_pad,1,header_cfg->blocksize,outfd))
       perror("can't write file");

    fclose(outfd);

    free(header_pad);
    return 0;
}




 

int parse_image_params(char *str, combo_image_cfg *image_cfg)
{
    char *token, *cp;
    cp = strdup(str);
    
    printf("Input:%s\n",cp); 

    token = strtok (cp,",");
    if (token)
        image_cfg->img_tag.chip_id=strtol(token,NULL,0);
    else
    {
        printf("error parsing img params, expecting chip_id. Input:%s\n",cp); 
        goto clean_and_error;
    }

    token = strtok (NULL,",");
    if (token)
        strncpy(image_cfg->img_tag.board_id,token,BOARD_ID_LEN);
    else
    {
        printf("error parsing img params, expecting board_id. Input:%s\n",cp); 
        goto clean_and_error;
    }

    token = strtok (NULL,",");
    if (token)
        strncpy(image_cfg->image_full_name,token,IMG_FULL_PATH_AND_NAME_LEN);
    else
    {
        printf("error parsing img params, expecting board_id. Input:%s\n",cp); 
        goto clean_and_error;
    }

    free(cp);
    return 0;

clean_and_error:
    free(cp);
    return 1;
}

int main(int argc, char **argv)
{
    int optchar, option_index;

    combo_header_cfg header_cfg;

    memset(&header_cfg,0x00,sizeof(header_cfg));
    header_cfg.header_tag.image_count=0; 

    while ((optchar = getopt_long (argc, argv, "h", longopts, &option_index)) != EOF) {
        switch (optchar)
        {
        case 0:
#ifdef DEBUG
            printf ("option %s", longopts[option_index].name);
            if (optarg)
                printf (" with arg %s", optarg);
            printf ("\n");
#endif
            if (strcmp(longopts[option_index].name, "image") == 0)
            { 
                if(header_cfg.header_tag.image_count < COMBO_MAX_NUM_IMAGES)
                {
                    if(!parse_image_params(optarg,
                        &header_cfg.image_cfg[header_cfg.header_tag.image_count]))
                    {
                        printf("advancing image count\n");
                        header_cfg.header_tag.image_count++;
                    }
                }
                else
                    printf("image count limit reached\n");
            }
            else if (strcmp(longopts[option_index].name, "output-file") == 0)
            {
                strncpy(header_cfg.output_full_name,optarg,IMG_FULL_PATH_AND_NAME_LEN);
            }
            else if (strcmp(longopts[option_index].name, "version-min") == 0)
            {
                header_cfg.header_tag.version_min = strtol(optarg,NULL,0);
            } 
            else if (strcmp(longopts[option_index].name, "version-max") == 0)
            {
               header_cfg.header_tag.version_max = strtol(optarg,NULL,0);
            } 
            else if (strcmp(longopts[option_index].name, "header-flags") == 0)
            {
               header_cfg.header_tag.header_flags= strtol(optarg,NULL,0);
            }
            else if (strcmp(longopts[option_index].name, "blocksize") == 0)
            {
                header_cfg.blocksize = strtol(optarg,NULL,0) * 1024;
            }
            else if (strcmp(longopts[option_index].name, "combo-image-version") == 0)
            {
                header_cfg.header_tag.combo_image_revision = strtol(optarg,NULL,0);
            }
            break;
        case 'h':
            printf("%s",usage);
            break;
        default:
            break;
        }
    }
    printf("configuration data\n");
    printf(">>image count %d\n",header_cfg.header_tag.image_count);
    dump_cfg_data(&header_cfg);
    create_combo_image(&header_cfg);
    return 0;
}
