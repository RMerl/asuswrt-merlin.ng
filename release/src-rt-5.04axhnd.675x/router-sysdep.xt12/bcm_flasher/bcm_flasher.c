
#include <rtconfig.h>
#include "bcm_imgif.h"
#include "cms_image.h"
#include <stdio.h>
#include <stdlib.h>

#ifdef RTCONFIG_URLFW
#include "bcm_flashutil.h"
#include <sys/reboot.h>

extern FILE *url_fopen(const char *path, const char *mode);

#define BOOT_SET_NEW_IMAGE          '0'

#endif

CmsImageFormat parseImgHdr(UINT8 *bufP __attribute__((unused)), UINT32 bufLen __attribute__((unused)))
{
   int result = CMS_IMAGE_FORMAT_FLASH;

   return result;
}

#define BCM_FLASHER_STD_BUFFSIZE  1024

int main(int argc, char *argv[])
{
   unsigned char * buffer;
   int buffer_size = BCM_FLASHER_STD_BUFFSIZE;
   unsigned int size, amount;
   FILE *fp;
   imgif_flash_info_t flash_info;
   static IMGIF_HANDLE imgifHandle = NULL;
   int reboot_new = 0;
   int urlfw = 0;
#if defined(CUSTOM_NAND_SINGLE_IMAGE) || defined(RTCONFIG_SINGLEIMG_B)
   imgif_img_info_t img_info;
   unsigned int force_upd_img_idx =0;

    /* Usage brcm_flasher <image_name> <img_idx 1 or 2> <reboot_new>*/
   if (argc >= 4) {
	reboot_new = atoi(argv[3]);
   } else if (argc >= 3) {
        force_upd_img_idx = atoi(argv[2]);
        fprintf(stderr, "bcm_flasher argc %d argv %s \n", argc, argv[2]);
        if (force_upd_img_idx !=1 && force_upd_img_idx != 2)
            force_upd_img_idx = 0;
   } else if (argc != 2) {
       fprintf(stderr, "Flash image burner, usage: %s [filename of image to burn] \n", argv[0]);
       return 0;
   }

    fprintf(stderr, "Flash image burner,  %s  idx %d reboot_new %d, argc %d\n", argv[0], force_upd_img_idx, reboot_new, argc);
#else
   if (argc >= 3) {
   	reboot_new = atoi(argv[2]);
   } else if (argc != 2)
   {
       fprintf(stderr, "Flash image burner, usage: %s [filename of image to burn]\n", argv[0]);
       return 0;
   }
   fprintf(stderr, "Flash image burner, %s, reboot_new %d\n", argv[0], reboot_new);
#endif

#ifdef RTCONFIG_URLFW
   urlfw = 1;
   if ( (fp = url_fopen(argv[1], "r")) == 0)
#else
   urlfw = 0;
   if ( (fp = fopen(argv[1], "r")) == 0)
#endif
   {
       fprintf(stderr, "error!!! Could not %sopen %s\n", urlfw?"url-":"", argv[1]);
       return -1;
   }

   fseek(fp, 0, SEEK_END);
   size = ftell(fp);
   rewind(fp);

   printf("File size 0x%x (%d)\n", size, size);

   imgifHandle = imgif_open(parseImgHdr, NULL);

   if (imgifHandle == NULL)
   {
       fprintf(stderr, "ERROR!!! imgif_open() failed\n");
       fclose(fp);
       return -1;
   }

   if (imgif_get_flash_info(&flash_info) != 0)
   {
       fprintf(stderr, "ERROR!!! imgif_get_flash_info() failed\n");
       imgif_close(imgifHandle, 1);
       fclose(fp);
       return -1;
   }
#if defined(CUSTOM_NAND_SINGLE_IMAGE) || defined(RTCONFIG_SINGLEIMG_B)
    memset(&img_info, 0, sizeof(img_info));
    if (force_upd_img_idx) {
        img_info.force_upd_img_idx = force_upd_img_idx;
        imgif_set_image_info(imgifHandle, &img_info);
    }
   #endif

   printf("Flash type 0x%x, flash size 0x%x, block size 0x%x\n", flash_info.flashType, flash_info.flashSize, flash_info.eraseSize);

   /* If we have a valid erase size, use it for allocating our buffer */
   if ( flash_info.eraseSize )
       buffer_size = flash_info.eraseSize;

   if ( (buffer = malloc(buffer_size)) == 0)
   {
       fprintf(stderr, "ERROR!!! Could not allocate memory for file %s\n", argv[1]);
       imgif_close(imgifHandle, 1);
       fclose(fp);
       return -1;
   }

   while (size)
   {
      amount = (size > (unsigned int)buffer_size) ? (unsigned int)buffer_size : size;

      if (fread (buffer, 1, amount, fp) != amount)
      {
         fprintf(stderr, "ERROR!!! Could not read image from file %s\n", argv[1]);
         free(buffer);
         imgif_close(imgifHandle, 1);
         fclose(fp);
         return -1;
      }

      if (amount != (unsigned int)imgif_write(imgifHandle, buffer, amount))
      {
          fprintf(stderr, "ERROR!!! Failed to process %d bytes. Not flashing upgrade image!\n", amount);
          free(buffer);
          imgif_close(imgifHandle, 1);
          fclose(fp);
          return -1;
      }

      printf(".");

      size -= amount;
   }

   free(buffer);
   fclose(fp);

   if (imgif_close(imgifHandle, 0) != 0)
   {
      fprintf(stderr, "ERROR!!! Failed to flash upgrade image \n");
      return -1;
   }
   else
      printf("\nImage flash complete, you may reboot the board\n");

#ifdef RTCONFIG_URLFW
   if(reboot_new)
   {
	setBootImageState(BOOT_SET_NEW_IMAGE);
	sleep(1);
	fprintf(stderr, "ready to reboot for new img..\n");
	reboot(RB_AUTOBOOT);
   }
#endif

   return size; // return the amount we copied
}

