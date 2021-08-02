/**
 ** Data preparation for diagnostic interfaces
 **
 ** Copyright 2009-2011 Gary Wuertz gary@issiweb.com
 ** Copyright 2011 BenEleventh Consulting manolson@beneleventh.com
 **
 ** This program is free software: you can redistribute it and/or modify
 ** it under the terms of the GNU General Public License as published by
 ** the Free Software Foundation, either version 3 of the License, or
 ** (at your option) any later version.
 **
 ** This program is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 ** GNU General Public License for more details.
 **
 ** You should have received a copy of the GNU General Public License
 ** along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <math.h>

/**
 * Something borrowed......
 */
#define  APP_BUFF_SIZE  1024
#define  NDSIZECOLLECT  0x1000000

typedef unsigned int U_INT;

struct pparams {
   int   cmd;        // command to execute
   int   options;    // debug options
   char  *outpath;   // output file;
   FILE  *input;     // input file
   FILE  *output;    // output file
   U_INT limit;      // limit delta
   U_INT repeat;     // repeat values
   U_INT value;      // inject value
   U_INT bsize;      // Buffer size
   double   xs;      // X scale
};
/**
 *  For bin output
 */
#define X_BINS  512
#define Y_BINS  2048
/**
 * Divide data into bins
 */
#define X_FACTOR (1.0 * X_BINS)/(1.0 * p->bsize)
#define Y_FACTOR (0.5 * Y_BINS)/(1.0 * 0x7fffffff)
/*
 * Scale bins back to values x = 1M, y=4.0G
 */
#define X_SCALE (p->xs/X_BINS)
#define Y_SCALE (4.0/Y_BINS)

static int  inject_output(struct pparams *p);
static void matrix_output(struct pparams *p);
static void sequence_output(struct pparams *p);
static void usage(int nopts, struct option *long_options, const char **cmds);
/**
 * Data Prep
 */
int main(int argc, char **argv)
{
   static const char* cmds[] = {
      "b", "buffer",    "1", "buffer size (k): default 1024",
      "f", "file",      "1", "output file name: default is '-' (stdout)",
      "i", "inject",    "1", "inject 0=2up, 1=1up, 2=raw 1up",
      "o", "output",    "1", "[bin|delta|inject|raw|xor|wrap] data",
      "r", "repeat",    "1", "repeat inject sequence",
      "s", "start",     "1", "start value inject sequence",
      "u", "upper",     "1", "inject sequence upper bound",
      "v", "verbose",   "1", "verbose reporting",
      "h", "help",      "0", "This help"
      };
   char  *outputs[] = {"bin","delta","inject","raw","xor","wrap",NULL};
   static int nopts = sizeof(cmds)/(4*sizeof(char *));
   struct option long_options[nopts+1];
   char short_options[1+nopts*2];
   struct pparams params;
   FILE *f;
   U_INT i, j, n;
   int   c;
   char *s;

   for(i=j=0;j<(nopts*4);j+=4) {
      long_options[i].name      = cmds[j+1];
      long_options[i].has_arg   = atoi(cmds[j+2]);
      long_options[i].flag      = NULL;
      long_options[i].val       = cmds[j][0];
      strcat(short_options,cmds[j]);
      if (long_options[i].has_arg!=0) strcat(short_options,":");
      i += 1;
      }
   memset(&long_options[i], 0, sizeof(struct option));
   memset(&params, 0, sizeof(struct pparams));
   params.outpath = "-";
   params.bsize = NDSIZECOLLECT;
   params.xs    = 1.0;
   do {
      c = getopt_long (argc, argv, short_options, long_options, NULL);
      switch(c) {
         case 'b':
            params.bsize = atoi(optarg);
            params.xs = params.bsize;
            while(params.xs >= 10.0)
               params.xs /= 10.0;
            break;
         case 'f':
            params.outpath = optarg;
            break;
         case 'i':
            params.options = atoi(optarg);
            break;
         case 'o':
            n = strlen(optarg);
            for(i=0;outputs[i]!=NULL;i++)
               if (!strncmp(optarg, outputs[i], n)) {
                  params.cmd = optarg[0];
                  break;
               }
            break;
         case 'r':
            params.repeat = atoi(optarg);
            break;
         case 's':
            params.value = atoi(optarg);
            break;
         case 'u':
            params.limit = atoi(optarg);
            break;
         case 'v':
            params.cmd = atoi(optarg);
            break;
         case '?':
         case 'h':
            usage(nopts, long_options, cmds);
         case -1:
            break;
         }
      } while (c!=-1);

   if (0==params.cmd || optind != (argc-1))
      usage(nopts, long_options, cmds);
   if (!strcmp(argv[1],"-"))
      params.input = stdin;
   else {
      params.input = fopen(argv[optind], "rb");
      if (NULL == params.input) {
         fprintf(stderr, "Unable to open input %s\n", argv[optind]);
         exit(2);
         }
      }
   if (!strcmp(params.outpath, "-"))
      params.output = stdout;
   else {
      params.output = fopen(params.outpath, "wb");
      if (NULL == params.output) {
         fprintf(stderr, "Unable to open %s\n", params.outpath);
         exit(3);
         }
      fprintf(stdout, "writing to %s\n", params.outpath);
      }
   switch(params.cmd) {
         case 'i':
            while(inject_output(&params)>0)
               ;
            break;
         case 'b':
            matrix_output(&params);
            break;
         case 'd':   case 'r':  case 'x':  case 'w':
            sequence_output(&params);
            break;
      }
   if (params.output != stdout)
      fclose(params.output);
   return 0;
}
/**
 * Create injection data - input file is log10 sequence data - can be repeated
 */
static int inject_output(struct pparams *p)
{
   U_INT    buf[APP_BUFF_SIZE];
   char     ibuf[80], *s;
   U_INT    i, j;
   double   n, delta;
   int      rv = 1;
   
   n = 0;
   for(i=0;i<APP_BUFF_SIZE && rv==1;i++) {
      buf[i] = p->value;
      s = fgets(ibuf, 80, p->input);
      if (NULL == s && p->repeat != 0) {
         p->repeat -= 1;
         rewind(p->input);
         s = fgets(ibuf, 80, p->input);
         }
      if (NULL != s) {
         if (p->options!=0)
            delta = strtod(ibuf, NULL);
         else {
            n = strtod(ibuf, &s);
            delta = strtod(s, NULL);
            }
         if (p->limit != 0 && delta > p->limit)
            delta -= p->limit;
         if (p->options == 2)
            p->value = (U_INT) delta;
         else p->value += (U_INT)pow(10.0,delta);
         }
      else rv = 0;
      }
   if (i != fwrite(buf, sizeof(U_INT), i, p->output)) {
      printf("Write error\n");
      rv = -1;
      }
   return rv;
}
/**
 * Create matrix data file
 */
static void matrix_output(struct pparams *p)
{
   U_INT   buf[APP_BUFF_SIZE];
   U_INT   **matrix;
   FILE    *f = p->input;
   int     i, n, sz, x, y;
   
   matrix = (U_INT **) malloc(sizeof(U_INT **) * X_BINS);
   if (NULL == matrix) {
      fprintf(stderr, "Unable to allocate cols\n");
      return;
      }
   sz = sizeof(U_INT *) * Y_BINS;
   for (i = 0;i< X_BINS;i++) {
       matrix[i] = (U_INT *)malloc(sz);
       if (NULL == matrix[i]) {
           fprintf(stderr, "Unable to allocate row\n");
           return;
           }
       memset(matrix[i], 0, sz);
       }
   n = 0;
   while(1) {
      sz = fread(buf, sizeof(U_INT), APP_BUFF_SIZE, f);
      if (sz < 1)
          break;
      for(i=0;i<sz;i++) {
          x = (int)(n * X_FACTOR);
          y = (int)(buf[i] * Y_FACTOR);
          matrix[x][y] += 1; 
          n += 1;
          n %= p->bsize;
          }   
      }
   for(x=0;x<X_BINS;x++)
      for(y=0;y<Y_BINS;y++)
         if (matrix[x][y]!=0)
            fprintf(p->output,"%g\t%g\t%u\n", x*X_SCALE, y*Y_SCALE, matrix[x][y]);
}
/**
 * Create sequence data file
 */
static void sequence_output(struct pparams *p)
{
   U_INT   buf[APP_BUFF_SIZE];
   FILE    *f = p->input;
   int     i, m, n, sz;
   U_INT   delta, cur, prev;
   U_INT   plus, minus;

   m = p->cmd=='r'? 1 : 0;
   n = 0;
   plus = minus = 0;
   while(1) {
      sz = fread(buf, sizeof(U_INT), APP_BUFF_SIZE, f);
      if (sz < 1)
          break;
      for(i=0;i<sz;i++) {
         prev = cur;
         cur = buf[i];
         if (m==0) {
             m = 1;
             }
         else switch(p->cmd) {
            case 'd':
               if (cur < prev)
                  delta = prev - cur;
               else delta = cur - prev;
               fprintf(p->output,"%g\t%g\n", n * 10.0/1024.0, log10(delta));
               break;
            case 'x':
               fprintf(p->output,"%g\t%g\n", n * 10.0/1024.0, log10(cur^prev));
               break;
            case 'r':
               fprintf(p->output,"%g\t%g\n", n * 10.0/1024.0, 1.0 * cur);
               break;
            case 'w':
               if (cur < prev) {
                  if (p->options & 1)
                     fprintf(p->output,"rollover %d\n", n);
                  minus++;
                  }
               else plus++;
               break;
            }
         n += 1;
         n %= p->bsize;
         }
      }
   if (p->cmd=='w')
      fprintf(p->output,"Rollover %u/%u = %g\n", minus, plus, minus*100/(double)(minus+plus));
}
/**
 * usage
 */
static void usage(int nopts, struct option *long_options, const char **cmds)
{
   int i;
  
   fprintf(stderr, "\nUsage: %s [options] <file>\n\n", "data_prep");
   fprintf(stderr, "Prepare diagnostic data from <file>\n\n");
   fprintf(stderr, "  Options:\n");
   for(i=0;long_options[i].val != 0;i++) {
      fprintf(stderr,"     --%-10s, -%c %s %s\n",
         long_options[i].name, long_options[i].val,
         long_options[i].has_arg? "[]":"  ",cmds[4*i+3]);
      }
   fprintf(stderr, "\n");
   exit(1);
}

