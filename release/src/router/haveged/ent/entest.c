/*
   ENT  --  Entropy calculation and analysis of putative
       random sequences.

        Designed and implemented by John "Random" Walker in May 1985.

   Multiple analyses of random sequences added in December 1985.

   Bit stream analysis added in September 1997.

   Terse mode output, getopt() command line processing,
   optional stdin input, and HTML documentation added in
   October 1998.

   Documentation for the -t (terse output) option added
   in July 2006.

   Replaced table look-up for chi square to probability
   conversion with algorithmic computation in January 2008.

        Hacked for haveged test April 2009

   For additional information and the latest version,
   see http://www.fourmilab.ch/random/

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "getopt.h"
#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#else
#include <unistd.h>
#endif

#include "iso8859.h"
#include "randtest.h"

#define UPDATE  "January 28th, 2008"

#define FALSE 0
#define TRUE  1

#ifdef M_PI
#define PI   M_PI
#else
#define PI   3.14159265358979323846
#endif

extern double pochisq(const double ax, const int df);

struct pparams {
  double chisqr;
  double entropy;
  double mean;
  double pi;
  double seq;
  double xtreme;
};

static struct pparams defaults = {
 .chisqr  = 1,          // bounds  - should be >1% and <99%
 .entropy = 7.5,        // minimum - should be close to 8
 .mean    = 127.0,   // minimum - should be close to 127.5
 .pi      = 0.5,  // maximum - error deviation in percent
 .seq     = 0.8,  // minimum - should be close to 1
 .xtreme  = 0.00001  // maximum - result deviation
};
struct pparams *params = &defaults;

static int check(long totalc, double ent, double chip, double mean, double montepi, double scc)
{
char failures[512];
int  offs=0;
double p;
double p0 = params->chisqr;
double p1 = 100-p0;
if (ent<params->entropy) {
  sprintf(failures+offs,"entropy:%f<%f ",ent,params->entropy);
  offs=strlen(failures);
}
chip *= 100;
if (chip<p0||chip>p1) {
  sprintf(failures+offs,"chisqr:%f%% not in %f-%f ",chip,p0,p1);
  offs=strlen(failures);
}
if (mean<params->mean) {
  sprintf(failures+offs,"mean:%f<%f ",mean,params->mean);
  offs=strlen(failures);
}
p = 100*(montepi-PI)/PI;
if (fabs(p)>params->pi) {
  sprintf(failures+offs,"pi:%f %f>%f ",montepi,p,params->pi);
  offs=strlen(failures);
}
if (scc>params->seq) {
  sprintf(failures+offs,"sequence:%f>%f ",scc,params->seq);
  offs=strlen(failures);
}
if (offs) {
   printf("Check Fail: %s\n",failures);
   return -1;
}
printf("Sample looks good!\n");
return 0;
}

/* Who checks the checker? The ent self-test test the various options by collecting the output
 * of the various options on a given input in a text and then doing a diff with the expected
 * output. IMHO the text comparison is pretty cheezy since doubles are involved. We make the
 * same check but do not require equality on the quantities involved
 */

static int checkSelf(long totalc, double ent, double chisq, double mean, double montepi, double scc)
{
double ref[6] = {11619,7.787095,6567.223255,110.000258,3.392562,0.098324};
double tst[6] = {totalc,ent,chisq,mean,montepi,scc};
int i;double f;
for(i=0;i<6;i++) {
 f=fabs(tst[i]-ref[i])/ref[i];
 if (f > params->xtreme) {
   printf("Self-test(%d) failed:%f?",i,f);
   return -1;
   }
 }
printf("Self-test good!\n");
return 0;
}

/*  HELP  --  Print information on how to call  */

static void help(void)
{
        printf("entest -- Test the havege RNG.");
        printf("\n");
        printf("\n        Options:   -c<min>      Chi Square Threshold");
        printf("\n                   -e<min>      Min entropy/char");
        printf("\n                   -f<file>     File");
        printf("\n                   -m<min>      Min arithmetic mean");
        printf("\n                   -p<max>      Max error in pi");
        printf("\n                   -s<min>      Min serial coeficient");
        printf("\n                   -t           Self test");
        printf("\n                   -u           Print this message\n");
        printf("\n                   -v           Verbose - show the stats");
        printf("\nAdapted from the work of John Walker");
        printf("\n   http://www.fourmilab.ch/\n\n");
}


/*  Main program  */

int main(int argc, char *argv[])
{
   int oc, opt;
   long ccount[256];       /* Bins to count occurrences of values */
   long totalc = 0;        /* Total character count */
   double montepi, chip,
          scc, ent, mean, chisq;
   FILE *fp = stdin;
   int fold = FALSE,       /* Fold upper to lower */
       binary = FALSE,        /* Treat input as a bitstream */
            stest = FALSE,
            verbose = FALSE;
        char *filename="";

        while ((opt = getopt(argc, argv, "?c:e:f:m:p:s:t:v")) != -1) {
       switch (toISOlower(opt)) {
                 case 'c':
          params->chisqr = atof(optarg);
          break;

                 case 'e':
                    params->entropy = atof(optarg);
                    break;

                 case 'f':
          filename = optarg;
          break;

       case 'm':
                    params->mean = atof(optarg);
                    break;

       case 'p':
                    params->pi = atof(optarg);
                    break;

       case 's':
                    params->seq = atof(optarg);
                    break;

                 case 't':
                    filename = optarg;
/*                    filename = "entitle.gif"; */
          stest = TRUE;
          break;

      case 'v':
         verbose = TRUE;
         break;

      default:
         help();
         return -1;
       }
   }
        if (!strlen(filename)) {
           printf("Need input file\n");
           help();
           return 2;
           }
        if ((fp = fopen(filename, "rb")) == NULL) {
           printf("Cannot open file %s\n", filename);
      return 2;
      }
   memset(ccount, 0, sizeof ccount);

   /* Initialise for calculations */

   rt_init(binary);

   /* Scan input file and count character occurrences */

   while ((oc = fgetc(fp)) != EOF) {
      unsigned char ocb;

      if (fold && isISOalpha(oc) && isISOupper(oc)) {
         oc = toISOlower(oc);
      }
      ocb = (unsigned char) oc;
      totalc += binary ? 8 : 1;
      if (binary) {
       int b;
       unsigned char ob = ocb;

       for (b = 0; b < 8; b++) {
      ccount[ob & 1]++;
      ob >>= 1;
       }
      } else {
          ccount[ocb]++;         /* Update counter for this bin */
      }
      rt_add(&ocb, 1);
   }
   fclose(fp);

   /* Complete calculation and return sequence metrics */

   rt_end(&ent, &chisq, &mean, &montepi, &scc);
        chip = pochisq(chisq, (binary ? 1 : 255));

        if (verbose)
           printf("\nTest Results\n"
                  "Sample:      %ld bytes\n"
                  "Entropy:     %f bits\n"
                  "Chi-Square:  %f(%f%%)\n"
                  "Mean:        %f\n"
                  "PI:          %f(%f%%)\n"
                  "Correlation: %f\n",
              totalc, ent, chisq, chip*100, mean, montepi,100*(montepi-PI)/PI,scc);

   if (stest)
           return checkSelf(totalc, ent, chisq, mean, montepi, scc);
        return check(totalc, ent, chip, mean, montepi, scc);
}
