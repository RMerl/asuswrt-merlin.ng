
#include "bcm_flashutil.h"
#include "bcm_hwdefs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{
   if (argc != 2)
   {
      char result, string1[] = "n/a", string2[] = "n/a";
      int higher;
      int seq1 = getSequenceNumber(1);
      int seq2 = getSequenceNumber(2);
      int boot = getBootPartition();
      int reboot;

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
      printf( \
          "%c%cFirst  partition type           : %s\n" \
          "%c%cFirst  partition sequence number: %d\n" \
          "%c%cFirst  partition commit flag    : %s\n" \
          "%c%cSecond partition type           : %s\n" \
          "%c%cSecond partition sequence number: %d\n" \
          "%c%cSecond partition commit flag    : %s\n", \
             (boot == 1) ? 'B' : ' ', (reboot == 1) ? '>' : ' ', (seq1 >= 0) ? ((string1[0] == 'n') ? "JFFS2/UBI" : "pureUBI/GPT") : "NONE", \
             (boot == 1) ? 'B' : ' ', (reboot == 1) ? '>' : ' ', seq1, \
             (boot == 1) ? 'B' : ' ', (reboot == 1) ? '>' : ' ', string1, \
             (boot == 2) ? 'B' : ' ', (reboot == 2) ? '>' : ' ', (seq2 >= 0) ? ((string2[0] == 'n') ? "JFFS2/UBI" : "pureUBI/GPT") : "NONE", \
             (boot == 2) ? 'B' : ' ', (reboot == 2) ? '>' : ' ', seq2, \
             (boot == 2) ? 'B' : ' ', (reboot == 2) ? '>' : ' ', string2);

      printf("\nTo set boot state usage: %s [state], where your state options are:\n" \
                      "  1 or BOOT_SET_NEW_IMAGE\n" \
                      "  2 or BOOT_SET_OLD_IMAGE\n" \
                      "  3 or BOOT_SET_NEW_IMAGE_ONCE\n" \
                      "  4 or BOOT_SET_OLD_IMAGE_ONCE\n" \
                      "  5 or BOOT_SET_PART1_IMAGE\n" \
                      "  6 or BOOT_SET_PART1_IMAGE_ONCE\n" \
                      "  7 or BOOT_SET_PART2_IMAGE\n" \
                      "  8 or BOOT_SET_PART2_IMAGE_ONCE\n" \
                      "  -------- test options --------\n" \
                      "  +1 ------ commit    partition 1\n" \
                      "  -1 ------ uncommit  partition 1\n" \
                      "  +2 ------ commit    partition 2\n" \
                      "  -2 ------ uncommit  partition 2\n" \
                      , argv[0]);

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

