
#include "bcm_imgif.h"
#include "cms_image.h"
#include <stdio.h>
#include <stdlib.h>


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

   if (argc != 2)
   {
       fprintf(stderr, "Flash image burner, usage: %s [filename of image to burn]\n", argv[0]);
       return 0;
   }

   if ( (fp = fopen(argv[1], "r")) == 0)
   {
       fprintf(stderr, "ERROR!!! Could not open %s\n", argv[1]);
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
          fprintf(stderr, "ERROR!!! Did not successfully write image to NAND\n");
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
      fprintf(stderr, "ERROR!!! Did not successfully write image to NAND\n");
   else
      printf("\nImage flash complete, you may reboot the board\n");

   return size; // return the amount we copied
}

