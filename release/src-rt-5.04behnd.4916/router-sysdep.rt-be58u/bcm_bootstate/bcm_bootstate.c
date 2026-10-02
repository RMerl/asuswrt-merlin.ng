
#include "bcm_flashutil.h"
#include "bcm_hwdefs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


static char identBuf[1024];
static const char *myGetIdent(int partition)
{
   int start = 0, end = 50;

   memset(identBuf, 0, sizeof(identBuf));
   bcmFlash_getIdent(partition, &start, &end, "imageversion",
                     identBuf, sizeof(identBuf)-1);
   return identBuf;
}


int main(int argc, char *argv[])
{
   if (argc != 2)
   {
      char result, string1[] = "n/a", string2[] = "n/a";
      int higher = 1;
      int boot = getBootPartition();
      int reboot;
      int seq1 = -1;
      int seq2 = -1;

      seq1 = getSequenceNumber(1);
      seq2 = getSequenceNumber(2);

      if (seq1 > seq2)
          higher = 1;
      else
          higher = 2;

      if ((seq1 == 0) && (seq2 == 999))
         higher = 1;
      else if ((seq2 == 0) && (seq1 == 999))
         higher = 2;

      result = 0;
      commit(1, &result);
      if (result)
      {
         string1[0] = result;
         string1[1] = '\0';
      }
         
      result = 0;
      commit(2, &result);
      if (result)
      {
         string2[0] = result;
         string2[1] = '\0';
      }

      printf("\n  Boot image state: ");

      switch(getBootImageState())
      {
           case(BOOT_SET_NEW_IMAGE):
              printf("BOOT_SET_NEW_IMAGE");
              reboot = higher;
              break;

           case(BOOT_SET_OLD_IMAGE):
              printf("BOOT_SET_OLD_IMAGE");
              reboot = (higher == 1) ? 2 : 1;
              break;

           case(BOOT_SET_NEW_IMAGE_ONCE):
              printf("BOOT_SET_NEW_IMAGE_ONCE");
              reboot = higher;
              break;

           case(BOOT_SET_OLD_IMAGE_ONCE):
              printf("BOOT_SET_OLD_IMAGE_ONCE");
              reboot = (higher == 1) ? 2 : 1;
              break;

           case(BOOT_SET_PART1_IMAGE):
              printf("BOOT_SET_PART1_IMAGE");
              reboot = 1;
              break;

           case(BOOT_SET_PART1_IMAGE_ONCE):
              printf("BOOT_SET_PART1_IMAGE_ONCE");
              reboot = 1;
              break;

           case(BOOT_SET_PART2_IMAGE):
              printf("BOOT_SET_PART2_IMAGE");
              reboot = 2;
              break;

           case(BOOT_SET_PART2_IMAGE_ONCE):
              printf("BOOT_SET_PART2_IMAGE_ONCE");
              reboot = 2;
              break;

           default:
              printf("UNKNOWN (boot latest image)");
              if (seq1 > seq2)
                 reboot = 1;
              else
                 reboot = 2;
              break;
      }

      printf("\n  Booted Partition: %s\n", (boot == 1) ? "First" : "Second");
      printf(  "  Reboot Partition: %s\n\n", (reboot == 1) ? "First" : "Second");

      printf("%c%cFirst  partition type           : %s\n", (boot == 1) ? 'B' : ' ', (reboot == 1) ? '>' : ' ', (seq1 >= 0) ? ((string1[0] == 'n') ? "JFFS2/UBI" : "pureUBI/GPT") : "NONE");
      printf("%c%cFirst  partition sequence number: %d\n", (boot == 1) ? 'B' : ' ', (reboot == 1) ? '>' : ' ', seq1);
      printf("%c%cFirst  partition image tag      : %s\n", (boot == 1) ? 'B' : ' ', (reboot == 1) ? '>' : ' ', myGetIdent(1));

      printf("%c%cFirst  partition commit flag    : %s\n",  (boot == 1) ? 'B' : ' ', (reboot == 1) ? '>' : ' ', string1);

      printf("%c%cSecond partition type           : %s\n", (boot == 2) ? 'B' : ' ', (reboot == 2) ? '>' : ' ', (seq2 >= 0) ? ((string2[0] == 'n') ? "JFFS2/UBI" : "pureUBI/GPT") : "NONE");
      printf("%c%cSecond partition sequence number: %d\n", (boot == 2) ? 'B' : ' ', (reboot == 2) ? '>' : ' ', seq2);
      printf("%c%cSecond partition image tag      : %s\n", (boot == 2) ? 'B' : ' ', (reboot == 2) ? '>' : ' ', myGetIdent(2));

      printf("%c%cSecond partition commit flag    : %s\n", (boot == 2) ? 'B' : ' ', (reboot == 2) ? '>' : ' ', string2);

      printf("\nTo set boot state usage: %s [state], where your state options are:\n", argv[0]);

      printf("  1 or BOOT_SET_NEW_IMAGE\n");
      printf("  2 or BOOT_SET_OLD_IMAGE\n");
      printf("  3 or BOOT_SET_NEW_IMAGE_ONCE\n");
      printf("  4 or BOOT_SET_OLD_IMAGE_ONCE\n");
      printf("  5 or BOOT_SET_PART1_IMAGE\n");
      printf("  6 or BOOT_SET_PART1_IMAGE_ONCE\n");
      printf("  7 or BOOT_SET_PART2_IMAGE\n");
      printf("  8 or BOOT_SET_PART2_IMAGE_ONCE\n");
      printf("  -------- test options --------\n");
      printf("  +1 ------ commit    partition 1\n");
      printf("  -1 ------ uncommit  partition 1\n");
      printf("  +2 ------ commit    partition 2\n");
      printf("  -2 ------ uncommit  partition 2\n");
                      
      return 0;
   }

   if (!strcmp(argv[1], "1") || !strcmp(argv[1], "BOOT_SET_NEW_IMAGE"))
      return(setBootImageState(BOOT_SET_NEW_IMAGE));

   if (!strcmp(argv[1], "2") || !strcmp(argv[1], "BOOT_SET_OLD_IMAGE"))
      return(setBootImageState(BOOT_SET_OLD_IMAGE));

   if (!strcmp(argv[1], "3") || !strcmp(argv[1], "BOOT_SET_NEW_IMAGE_ONCE"))
      return(setBootImageState(BOOT_SET_NEW_IMAGE_ONCE));

   if (!strcmp(argv[1], "4") || !strcmp(argv[1], "BOOT_SET_OLD_IMAGE_ONCE"))
      return(setBootImageState(BOOT_SET_OLD_IMAGE_ONCE));

   if (!strcmp(argv[1], "5") || !strcmp(argv[1], "BOOT_SET_PART1_IMAGE"))
      return(setBootImageState(BOOT_SET_PART1_IMAGE));

   if (!strcmp(argv[1], "6") || !strcmp(argv[1], "BOOT_SET_PART1_IMAGE_ONCE"))
      return(setBootImageState(BOOT_SET_PART1_IMAGE_ONCE));

   if (!strcmp(argv[1], "7") || !strcmp(argv[1], "BOOT_SET_PART2_IMAGE"))
      return(setBootImageState(BOOT_SET_PART2_IMAGE));

   if (!strcmp(argv[1], "8") || !strcmp(argv[1], "BOOT_SET_PART2_IMAGE_ONCE"))
      return(setBootImageState(BOOT_SET_PART2_IMAGE_ONCE));
   
   if(strlen(argv[1]) == 2)
   {
      char commit_ops = argv[1][0];
      char partition = argv[1][1] - '0';

      if( commit_ops == '+' || commit_ops == '-' )
         if( partition && (partition <= 2) )
            return(commit(partition,(commit_ops == '+' ? "1":"0")));
   }

   fprintf(stderr, "%s is an unknown boot state to set\n", argv[1]);
   return -1;
}

