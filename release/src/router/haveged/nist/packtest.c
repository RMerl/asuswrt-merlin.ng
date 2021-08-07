/* --------------------------------------------------------------------------
   Title       :  The NIST Statistical Test Suite

   Date        :  December 1999

   Programmer  :  Juan Soto

   Summary     :  For use in the evaluation of the randomness of bitstreams
		  produced by cryptographic random number generators.

   Package     :  Version 1.0

   Copyright   :  (c) 1999 by the National Institute Of Standards & Technology

   History     :  Version 1.0 by J. Soto, October 1999
		  Revised by J. Soto, November 1999

   Keywords    :  Pseudorandom Number Generator (PRNG), Randomness, Statistical
                  Tests, Complementary Error functions, Incomplete Gamma
	          Function, Random Walks, Rank, Fast Fourier Transform,
                  Template, Cryptographically Secure PRNG (CSPRNG),
		  Approximate Entropy (ApEn), Secure Hash Algorithm (SHA-1),
                  Blum-Blum-Shub (BBS) CSPRNG, Micali-Schnorr (MS) CSPRNG,

   Source      :  David Banks, Elaine Barker, James Dray, Allen Heckert,
		  Stefan Leigh, Mark Levenson, James Nechvatal, Andrew Rukhin,
		  Miles Smid, Juan Soto, Mark Vangel, and San Vo.

   Technical
   Assistance  :  Lawrence Bassham, Ron Boisvert, James Filliben, Sharon Keller,
		  Daniel Lozier, and Bert Rust.

   Warning     :  Portability Issues.

   Limitation  :  Amount of memory allocated for workspace.

   Restrictions:  Permission to use, copy, and modify this software without
		  fee is hereby granted, provided that this entire notice is
		  included in all copies of any software which is or includes
                  a copy or modification of this software and in all copies
                  of the supporting documentation for such software.
   -------------------------------------------------------------------------- */

/* --------------------------------------------------------------------------
 * Derived by Andre Seznec (IRISA/INRIA - www.irisa.fr/caps) to overcome the
 * limitations on the sequences size. It should now work on sequences larger
 * than 1MB and smaller than 256MB.
 *
 * Modified on March 2002, last update on June 2002
 *
 * Modified Aug 2008 for clean compilation under gcc 4.4
 * --------------------------------------------------------------------------
 */
#ifndef NOIO
#define  IO
#endif
#define LinuxorUnix
#ifdef WIN
#ifndef CYGWIN
#undef LinuxorUnix
/* same libraries are available*/
#endif
#endif

#ifdef  LinuxorUnix

#define MAX(x,y)             ((x) <  (y)  ? (y)  : (x))
#define MOD(x,y)  (((x) % (y)) < 0 ? ((x) % (y)) + (y) : ((x) % (y)))


#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "special-functions.h"
#include "mconf.h"
#include "matrix.h"
#include "nist.h"

#define THRESHOLDFAIL 0.001
static int TotalTEST = 0;

/*
 * on Windows systems, we encountered difficulties with the memory allocation
 * libraries, therefor we used our own allocation technique in a big array
 */

#define BIGSIZE 32768*1024
char BigArray[BIGSIZE];
int PtBigArray = 0;
char *
MYCALLOC (int n, int S)
{
  int i, OldPt;
  OldPt = PtBigArray;
  PtBigArray += ((n * S + 16) & 0xfffffff0);
  if (PtBigArray >= BIGSIZE)
    {
      printf ("Pb memory allocation in PackTest\n");
      exit (1);
    }
  for (i = OldPt; i < PtBigArray; i++)
    BigArray[i] = 0;

  return (&BigArray[OldPt]);
}

void
MYFREE (char *X)
{
  PtBigArray = (int) (X - BigArray);
}

BitField **create_matrix (int M, int Q);

FILE *fileout;

int
PackTestF (int *ARRAY, int ArraySize, char *C)
{
  int i;
  int TEST = 0;
  int failure = 0;

#ifdef IO
  fileout = fopen (C, "w");
#endif
  if (ArraySize >= (1 << 26))
    {
      fprintf (fileout,
	       "This test does not work for an array larger than 256Mbytes\n");
      return (failure);
    }
#ifdef IO
  {
    printf
      ("16 NIST tests to be executed: results will be written in file %s\n\n",
       C);
    printf
      ("To maintain response time in a reasonable range,\nsome of the tests are executed on only subsequences\n\n");

    TEST++;
    fprintf (stderr, "Test %d, ", TEST);
    fprintf (stderr, "\t Frequency test \n\n");
    fprintf (fileout, " \n\n Frequency test \n\n");
  }
#endif
  for (i = 4; i < ArraySize; i = (i << 1))
    {
      int inter;
      inter = MOD (ARRAY[ArraySize - i], ArraySize - i);
      if (failure >= 8)
	{
#ifdef IO
	  fprintf (stderr,
		   "%d failures at %f threshold,\nsequence should be considered as not random\n",
		   failure, THRESHOLDFAIL);

#endif
	  return (failure);
	}


      failure += Frequency (i, &ARRAY[inter]);
    }

  if (failure >= 8)
    {
#ifdef IO
      fprintf (stderr,
		   "%d failures at %f threshold,\nsequence should be considered as not random\n",
		   failure, THRESHOLDFAIL);

#endif
      return (failure);
    }

  failure += Frequency (ArraySize, ARRAY);
#ifdef IO
  {
    TEST++;
    fprintf (stderr, "Test %d, ", TEST);
    fprintf (stderr, "\t Block Frequency test \n\n");
    fprintf (fileout, " \n\n Block Frequency test \n\n");
  }
#endif
  for (i = 4; i < ArraySize; i = (i << 1))
    {
      int inter;
      inter = MOD (ARRAY[ArraySize - i], ArraySize - i);
      failure += BlockFrequency (i, (32 * i / 99), &ARRAY[inter]);
    }

  if (failure >= 8)
    {
#ifdef IO
      fprintf (stderr,
		   "%d failures at %f threshold,\nsequence should be considered as not random\n",
		   failure, THRESHOLDFAIL);

#endif
      return (failure);
    }

  failure += BlockFrequency (ArraySize, (32 * ArraySize / 99), ARRAY);
#ifdef IO
  {
    TEST++;
    fprintf (stderr, "Test %d, ", TEST);
    fprintf (stderr, "\t Runs test \n\n");
    fprintf (fileout, " \n\n Runs test \n\n");
  }
#endif
  for (i = 4; i < ArraySize; i = (i << 1))
    {
      int inter;
      inter = MOD (ARRAY[ArraySize - i], ArraySize - i);
      if (failure >= 8)
	{
#ifdef IO


	        fprintf (stderr,
		   "%d failures at %f threshold,\nsequence should be considered as not random\n",
		   failure, THRESHOLDFAIL);

#endif
	  return (failure);
	}

      failure += Runs (i, &ARRAY[inter]);
    }
  Runs (ArraySize, ARRAY);
#ifdef IO
  {
    TEST++;
    fprintf (stderr, "Test %d, ", TEST);
    fprintf (stderr, "\t LongestRunOfOnes \n\n");
    fprintf (fileout, " \n\n LongestRunOfOnes \n\n");
  }
#endif
  for (i = 0; i < 8; i++)
    {
      int index;
      index = (ArraySize / 8) * i;
      if (ArraySize > 262144)
	index = index + (MOD (ARRAY[index], (ArraySize / 8) - 32768));
      failure += LongestRunOfOnes (((750000) >> 5) + 1, &ARRAY[index]);
if (failure >= 8)
	{
#ifdef IO


	        fprintf (stderr,
		   "%d failures at %f threshold,\nsequence should be considered as not random\n",
		   failure, THRESHOLDFAIL);

#endif
	  return (failure);
	}
    }

fclose(fileout);
return(failure);
}
int
PackTestL (int *ARRAY, int ArraySize, char *C)
{
  int i;
  int TEST = 4;
  int DATA, pt, PT;
  int failure = 0;

#ifdef IO
  fileout = fopen (C, "a");
#endif
#ifdef IO
  {
    TEST++;
    fprintf (stderr, "Test %d, ", TEST);
    fprintf (stderr,
	     "\t Binary Matrix Rank test on 8 random 38,912 bit slices \n\n");
    fprintf (fileout,
	     " \n\n Binary Matrix Rank test on 8 random 38,912 bit slices \n\n");
  }
#endif
  if (ArraySize >= (1 << 26))
    {
      fprintf (fileout,
	       "This test does not work for an  array larger than 256Mbytes\n");
      return (failure);
    }


  for (i = 0; i < 8; i++)
    {
      int index;
      index = (ArraySize / 8) * i;
      if (ArraySize > 262144)
	index = index + (MOD (ARRAY[index], (ArraySize / 8) - 32768));
      failure += Rank (38192 >> 5, &ARRAY[index]);
      if (failure >= 8)
	{
#ifdef IO


	        fprintf (stderr,
		   "%d failures at %f threshold,\nsequence should be considered as not random\n",
		   failure, THRESHOLDFAIL);

#endif
	  return (failure);
	}


    }
#ifdef IO
  {
    TEST++;
    fprintf (stderr, "Test %d, ", TEST);
    fprintf (stderr, "\t Discrete Fourier test by slice of 1 Mbits\n\n");
    fprintf (fileout, " \n\n Discrete Fourier test by slice of 1 Mbits\n\n");
    fprintf (fileout, " \n\n 8 random slices are picked \n\n");
  }
#endif
  for (i = 0; i < 8; i++)
    {
      int index;
      index = (ArraySize / 8) * i;
      if (ArraySize > 262144)
	index = index + (MOD (ARRAY[index], (ArraySize / 8) - 32768));
      if (failure >= 8)
	{
#ifdef IO


	        fprintf (stderr,
		   "%d failures at %f threshold,\nsequence should be considered as not random\n",
		   failure, THRESHOLDFAIL);

#endif
	  return (failure);
	}

      failure += DiscreteFourierTransform (32768, &ARRAY[index]);
    }
#ifdef IO
  {
    TEST++;
    fprintf (stderr, "Test %d, ", TEST);
    fprintf (stderr, "\t  NON OVERLAPPING TEMPLATE MATCHING TEST \n");
    fprintf (fileout, "\n\n\t  NON OVERLAPPING TEMPLATE MATCHING TEST \n");
    fprintf (fileout, "\t  1 random 1Mbit  slices  \n\n");
  }
#endif
  for (i = 0; i < 1; i++)
    {
      int index;
      index = (ArraySize) * i;
      if (ArraySize > 262144)
	index = index + MOD (ARRAY[index], ((ArraySize) - 32768));
      if (failure >= 8)
	{
#ifdef IO


	        fprintf (stderr,
		   "%d failures at %f threshold,\nsequence should be considered as not random\n",
		   failure, THRESHOLDFAIL);

#endif
	  return (failure);
	}

      failure +=
	NonOverlappingTemplateMatchings (9, 1024 * 1024, &ARRAY[index]);
    }
#ifdef IO
  {
    TEST++;
    fprintf (stderr, "Test %d, ", TEST);
    fprintf (stderr, "\t  OVERLAPPING TEMPLATE MATCHING TEST \n");
    fprintf (fileout, "\n\n\t  OVERLAPPING TEMPLATE MATCHING TEST \n");
    fprintf (fileout, "\t  8 random 1000000 bits slices \n\n");
  }
#endif
  for (i = 0; i < 8; i++)
    {
      int index;
      index = (ArraySize / 8) * i;
      if (ArraySize > 262144)
	index = index + (MOD (ARRAY[index], (ArraySize / 8) - 32768));
      if (failure >= 8)
	{
#ifdef IO


	        fprintf (stderr,
		   "%d failures at %f threshold,\nsequence should be considered as not random\n",
		   failure, THRESHOLDFAIL);

#endif
	  return (failure);
	}


      failure +=
	OverlappingTemplateMatchings (9, 1000000 >> 5, &ARRAY[index]);
    }
#ifdef IO
  {
    TEST++;
    fprintf (stderr, "Test %d, ", TEST);
    fprintf (stderr, "\t Maurer's Universal test\n");
    fprintf (fileout, "\n\n Maurer's Universal test\n");
    fprintf
      (fileout,
       "\n For each of the L parameters, we test  the beginning of the sequence\n\n");
  }
#endif

  if (failure >= 8)
    {
#ifdef IO
      fprintf (stderr,
		   "%d failures at %f threshold,\nsequence should be considered as not random\n",
		   failure, THRESHOLDFAIL);

#endif
      return (failure);
    }


  failure += Universal (ArraySize, ARRAY);

  TEST++;
#ifdef IO
  {
    fprintf (stderr, "Test %d, ", TEST);
    fprintf (stderr,
	     "\t  LEMPEL-ZIV COMPRESSION TEST: 8 random slices of  1000000 bits\n\n");
    fprintf (fileout,
	     "\n\n\t  LEMPEL-ZIV COMPRESSION TEST: 8 random slices of  1000000 bits\n\n");
  }
#endif
  for (i = 0; i < 8; i++)
    {
      int index;
      index = (ArraySize / 8) * i;
      if (ArraySize > 262144)
	index = index + (MOD (ARRAY[index], (ArraySize / 8) - 32768));
      DATA = ARRAY[index];
      pt = 0;
      PT = 0;
      if (failure >= 8)
	{
#ifdef IO


	        fprintf (stderr,
		   "%d failures at %f threshold,\nsequence should be considered as not random\n",
		   failure, THRESHOLDFAIL);

#endif
	  return (failure);
	}

      failure +=
	LempelZivCompression (1000000, &ARRAY[index], &DATA, &pt, &PT);
    }
  if (failure >= 8)
    {
/* in order to enable fast detection of strong failure for random sequences*/
#ifdef IO
      fprintf (fileout,
	       "%d failed individual tests with THRESHOLD %f on %d individual tests before LINEAR COMPLEXITY TEST\ndon't waste your CPU time\n",
	       failure, THRESHOLDFAIL, TotalTEST);
      fprintf (stderr,
	       "%d failed individual tests with THRESHOLD %f on %d individual tests before LINEAR COMPLEXITY TEST\ndon't waste your CPU time\n",
	       failure, THRESHOLDFAIL, TotalTEST);

#endif
      fclose (fileout);
      return (failure);
    }
#ifdef IO
  {
    TEST++;
    fprintf (stderr, "Test %d, ", TEST);
    fprintf (stderr,
	     "\t LINEAR COMPLEXITY TEST: 1 SLICE OF 4731 * 371 bits\n\n");
    fprintf (fileout,
	     "\n\n\t  LINEAR COMPLEXITY TEST: 1 SLICE OF 4731 * 371 bits\n\n");
  }
#endif

/* these parameters were chosen arbitraly, NIST recommendation are 500<=M<=5000, N>=200,  NM >=1000000*/
  for (i = 5; i <= 5; i++)
    {
      int index, inter;
      inter = (371 * 4731 + 32) >> 5;
      index = (ArraySize / 8) * i;
      if (ArraySize > 262144)
	index = index + (MOD (ARRAY[index], (ArraySize / 8) - 32768));
      if (index + inter > ArraySize)
	index = ArraySize - inter;


      failure += LinearComplexity (4731, 371, &ARRAY[index], 0);
    }
#ifdef IO
  {
    TEST++;
    fprintf (stderr, "Test %d, ", TEST);
    fprintf (stderr, "\t  SERIAL TEST  BY SLICE OF 1000000 bits\n\n");
    fprintf (fileout, "\n\n\t  SERIAL TEST  BY SLICE OF 1000000 bits\n\n");
  }
#endif
  for (i = 0; i < 8; i++)
    {
      int index;
      index = (ArraySize / 8) * i;
      if (ArraySize > 262144)
	index = index + (MOD (ARRAY[index], (ArraySize / 8) - 32768));

      if (failure >= 8)
	{
#ifdef IO


	        fprintf (stderr,
		   "%d failures at %f threshold,\nsequence should be considered as not random\n",
		   failure, THRESHOLDFAIL);

#endif
	  return (failure);
	}

      failure += Serial (14, 1000000, &ARRAY[index], 0);
    }
#ifdef IO
  {
    TEST++;
    fprintf (stderr, "Test %d, ", TEST);
    fprintf (stderr, "\t  APPROXIMATE ENTROPY TEST\n\n");
    fprintf (fileout, "\n\n\t  APPROXIMATE ENTROPY TEST\n\n");
  }
#endif
  failure += ApproximateEntropy (3, 17, 32 * ArraySize, ARRAY);
#ifdef IO
  {
    TEST++;
    fprintf (stderr, "Test %d, ", TEST);
    fprintf (stderr, "\t  CUSUM  TEST\n\n");
    fprintf (fileout, "\n\n\t  CUSUM  TEST\n\n");
  }
#endif
  for (i = 4; i < ArraySize; i = (i << 1))
    {
      int inter;
      fprintf (stderr, "\t\t   ArraySize: %d\n", i);
      inter = MOD (ARRAY[ArraySize - i], ArraySize - i);
      if (failure >= 8)
	{
#ifdef IO


	        fprintf (stderr,
		   "%d failures at %f threshold,\nsequence should be considered as not random\n",
		   failure, THRESHOLDFAIL);

#endif
	  return (failure);
	}

      failure += CumulativeSums (32 * i, &ARRAY[inter]);
    }

  if (failure >= 8)
    {
#ifdef IO
      fprintf (stderr,
		   "%d failures at %f threshold,\nsequence should be considered as not random\n",
		   failure, THRESHOLDFAIL);

#endif
      return (failure);
    }

  failure += CumulativeSums (32 * ArraySize, ARRAY);

#ifdef IO
  {
    TEST++;
    fprintf (stderr, "Test %d, ", TEST);
    fprintf (stderr,
	     "\t  RANDOM EXCURSION TEST: 8 SLICE OF 1000000 bits\n\n");
    fprintf (fileout,
	     "\n\n\t  RANDOM EXCURSION TEST: 8 SLICE OF 1000000 bits\n\n");
  }
#endif
  for (i = 0; i < 8; i++)
    {
      int index;
      fprintf (stderr, "\t\t      Slice number: %d\n", i);
      index = (ArraySize / 8) * i;
      if (ArraySize > 262144)
	index = index + (MOD (ARRAY[index], (ArraySize / 8) - 32768));

      if (failure >= 8)
	{
#ifdef IO


	        fprintf (stderr,
		   "%d failures at %f threshold,\nsequence should be considered as not random\n",
		   failure, THRESHOLDFAIL);

#endif
	  return (failure);
	}

      failure += RandomExcursions (1000000, &ARRAY[index >> 5]);
    }
#ifdef IO
  {
    TEST++;
    fprintf (stderr, "Test %d, ", TEST);
    fprintf (stderr,
	     "\t  RANDOM EXCURSION TEST VARIANT: 8 SLICES OF 1000000 bits\n\n");
    fprintf (fileout,
	     "\n\n\t  RANDOM EXCURSION TEST VARIANT: 8 SLICES OF 1000000 bits\n\n");
  }
#endif
  for (i = 0; i < 8; i++)
    {
      int index;
      index = (ArraySize / 8) * i;
      if (ArraySize > 262144)
	index = index + (MOD (ARRAY[index], (ArraySize / 8) - 32768));

      if (failure >= 8)
	{
#ifdef IO


	        fprintf (stderr,
		   "%d failures at %f threshold,\nsequence should be considered as not random\n",
		   failure, THRESHOLDFAIL);

#endif
	  return (failure);
	}

      failure += RandomExcursionsVariant (1000000, &ARRAY[index >> 5]);
    }


#ifdef IO
  {
    fprintf (stderr,
	     "%d failed individual tests with THRESHOLD %f on %d individual tests\n",
	     failure, THRESHOLDFAIL, TotalTEST);
    fprintf (fileout,
	     "%d failed individual tests with THRESHOLD %f on %d individual tests\n",
	     failure, THRESHOLDFAIL, TotalTEST);
  }
#endif
  fclose (fileout);
  return (failure);
}

#define MAXNUMOFTEMPLATES 148
int TEMPLATE[MAXNUMOFTEMPLATES];
int Skip[MAXNUMOFTEMPLATES];
int nu[6][MAXNUMOFTEMPLATES];
int Wj[8][MAXNUMOFTEMPLATES];
int W_obs[MAXNUMOFTEMPLATES];
/* was adapted to m =9  by A. Seznec 03/04/2002*/
int
NonOverlappingTemplateMatchings (int m, int n, int *ARRAY)
{
  FILE *fp;
  double sum, chi2, p_value, lambda;
  int i, j, k;
  char directory[FILENAME_MAX];
  int M, N, K = 5;
  int fail = 0;
  double pi[6], varWj;
  int DATA, PT, pt;
  N = 8;
  M = floor (n / N);


  lambda = (M - m + 1) / pow (2, m);
  varWj = M * (1. / pow (2., m) - (2. * m - 1.) / pow (2., 2. * m));
  sprintf (directory, "%stemplate%d", GetBaseDir(), m);
  if ((fp = fopen (directory, "r")) == NULL)
    {
#ifdef IO
      {
	fprintf (fileout, "\tTemplate file %s not existing\n", directory);
	fprintf (stderr, "\tTemplate file %s not existing\n", directory);
      }
#endif
      exit (1);

    }
  else
    {

      sum = 0.0;
      for (i = 0; i < 2; i++)
	{			/* Compute Probabilities */
	  pi[i] = exp (-lambda + i * log (lambda) - lgam (i + 1));
	  sum += pi[i];
	}
      pi[0] = sum;
      for (i = 2; i <= K; i++)
	{			/* Compute Probabilities */
	  pi[i - 1] = exp (-lambda + i * log (lambda) - lgam (i + 1));
	  sum += pi[i - 1];
	}
      pi[K] = 1 - sum;
      for (i = 0; i < MAXNUMOFTEMPLATES; i++)
	{
	  int inter;
	  TEMPLATE[i] = 0;
	  for (j = 0; j < m; j++)
	    {
	      if (fscanf (fp, "%d", &inter)<1) inter=0;
	      TEMPLATE[i] += (inter << j);
	    }
	}
      DATA = ARRAY[0] & ((1 << m) - 1);
      pt = 0;
      PT = m;

      for (i = 0; i < MAXNUMOFTEMPLATES; i++)
	for (k = 0; k <= K; k++)
	  nu[k][i] = 0;

      for (i = 0; i < N; i++)
	{
	  for (k = 0; k < MAXNUMOFTEMPLATES; k++)
	    W_obs[k] = 0;
	  for (j = 0; j < M - m + 1; j++)
	    {
	      for (k = 0; k < MAXNUMOFTEMPLATES; k++)
		{
		  if (Skip[k] == 0)
		    {
		      if (DATA == TEMPLATE[k])
			{
			  W_obs[k]++;
			  Skip[k] = m - 1;
			}
		    }
		  else
		    {
		      Skip[k]--;
		    }
		}
	      PT++;
	      if (PT == 32)
		{
		  PT = 0;
		  pt++;
		}
	      DATA = ((DATA << 1) + ((ARRAY[pt] >> PT) & 1)) & ((1 << m) - 1);
	    }
/* skipping  the final values in the slice*/
	  for (j = M - m + 1; j < M; j++)
	    {
	      PT++;
	      if (PT == 32)
		{
		  PT = 0;
		  pt++;
		}
	      DATA = ((DATA << 1) + ((ARRAY[pt] >> PT) & 1)) & ((1 << m) - 1);
	    }

	  for (k = 0; k < MAXNUMOFTEMPLATES; k++)
	    {
	      Wj[i][k] = W_obs[k];
	    }
	}
    }
  for (k = 0; k < MAXNUMOFTEMPLATES; k++)
    {
      sum = 0;
      chi2 = 0.0;		/* Compute Chi Square */
      for (i = 0; i < N; i++)
	{
	  chi2 += pow (((double) Wj[i][k] - lambda) / pow (varWj, 0.5), 2);
	}
      p_value = igamc (N / 2.0, chi2 / 2.0);
      if (p_value < THRESHOLDFAIL)
	fail++;
      else
	TotalTEST++;
#ifdef IO
      {
	fprintf (fileout, "p_value= %f\n", p_value);
      }
#endif
    }
  fclose (fp);
  return (fail);
}

int
OverlappingTemplateMatchings (int m, int n, int *ARRAY)
{
/* A. Seznec 03/03/2002:
I got some troubles with porting the function from the NIST package:
computation of array pi did not work satisfactory.
For m=9, I picked back values from page 34 in NIST report.
I do not have the values for m=10.

*/

  int i;
  double sum, chi2;
  int W_obs;
  double p_value;
  int DATA;
  int M, N, j, K = 5;
  unsigned int nu[6] = { 0, 0, 0, 0, 0, 0 };
  int fail = 0;
  double pi[6] =
    { 0.367879, 0.183940, 0.137955, 0.099634, 0.069935, 0.140657 };

/*  double pi[6] =
    { 0.143783, 0.139430, 0.137319, 0.124314, 0.106209, 0.348945 };*/


  int PT, pt;
  pt = 0;
  PT = 0;

  M = 1032;
/* N = (int) floor (n / M);*/
  N = 968;


/*
  lambda = (double) (M - m + 1) / pow (2, m);
   eta = lambda / 2.0;
   sum = 0.0;
   for (i = 0; i < K; i++)
    {

      pi[i] = Pr (i, eta);
      sum += pi[i];
    }
  pi[K] = 1 - sum;*/
  DATA = ARRAY[0] & ((1 << m) - 1);
  pt = 0;
  PT = m;
  for (i = 0; i < N; i++)
    {
      W_obs = 0;
      for (j = 0; j < M - m + 1; j++)
	{
	  if (DATA == ((1 << m) - 1))
	    W_obs++;
	  PT++;
	  if (PT == 32)
	    {
	      PT = 0;
	      pt++;
	    }
	  DATA = ((DATA << 1) + ((ARRAY[pt] >> PT) & 1)) & ((1 << m) - 1);
	}
/* skipping  the final values in the slice*/
      for (j = M - m + 1; j < M; j++)
	{
	  PT++;
	  if (PT == 32)
	    {
	      PT = 0;
	      pt++;
	    }
	  DATA = ((DATA << 1) + ((ARRAY[pt] >> PT) & 1)) & ((1 << m) - 1);
	}

      if (W_obs <= 4)
	nu[W_obs]++;
      else
	nu[K]++;
    }

  sum = 0;
  chi2 = 0.0;			/* Compute Chi Square */
  for (i = 0; i < K + 1; i++)
    {
      chi2 +=
	pow ((double) nu[i] - ((double) N * pi[i]), 2) / ((double) N * pi[i]);
      sum += nu[i];

    }
  p_value = igamc (K / 2., chi2 / 2.);
  if (p_value < THRESHOLDFAIL)
    fail++;
  else
    TotalTEST++;
#ifdef IO
  {
    fprintf (fileout, "m= %d, p_value= %f\n", m, p_value);
  }
#endif
  return (fail);
}

int
RandomExcursionsVariant (int n, int *ARRAY)
{
  int i, p, J, x, constraint;
  double p_value;
  int stateX[18] =
    { -9, -8, -7, -6, -5, -4, -3, -2, -1, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
  int count;
  int fail = 0;
  int *S_k;
  int pt, PT;
  if ((S_k = (int *) MYCALLOC (n, sizeof (int))) == NULL)
    {

    }
  else
    {
      J = 0;
      S_k[0] = 2 * (ARRAY[0] & 1) - 1;
      pt = 0;
      PT = 1;
      for (i = 1; i < n; i++)
	{
	  S_k[i] = S_k[i - 1] + 2 * ((ARRAY[pt] >> PT) & 1) - 1;
	  PT++;
	  if (PT == 32)
	    {
	      pt++;
	      PT = 0;
	    }
	  if (S_k[i] == 0)
	    J++;
	}
      if (S_k[n - 1] != 0)
	J++;

      constraint = MAX (0.005 * pow (n, 0.5), 500);
      if (J < constraint)
	{

#ifdef IO
	  {
	    fprintf (fileout,
		     "\n\t\tWARNING:  TEST NOT APPLICABLE.  THERE ARE ");
	    fprintf (fileout, "AN\n\t  INSUFFICIENT NUMBER OF CYCLES.\n");
	    fprintf (fileout,
		     "\t\t---------------------------------------------");
	    fprintf (fileout, "\n");
	  }
#endif
	}

      else
	{
	  for (p = 0; p <= 17; p++)
	    {
	      x = stateX[p];
	      count = 0;
	      for (i = 0; i < n; i++)
		if (S_k[i] == x)
		  count++;
	      p_value =
		erfc (fabs (count - J) /
		      (sqrt (2. * J * (4. * fabs (x) - 2))));
	      if (p_value < THRESHOLDFAIL)
		fail++;
	      else
		TotalTEST++;

#ifdef IO
	      {
		fprintf (fileout, "p_value= %f \n", p_value);
	      }
#endif

	    }
	}
    }
#ifdef IO
  {
    fprintf (fileout, "\n");
  }
#endif
  MYFREE ((char*)S_k);
  return (fail);
}

int
RandomExcursions (int n, int *ARRAY)
{
  int b, i, j, k, J, x;
  int cycleStart, cycleStop, *cycle, *S_k;
  int stateX[8] = { -4, -3, -2, -1, 1, 2, 3, 4 };
  int counter[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
  int PT, pt;
  int fail = 0;
  double p_value, sum, constraint, nu[6][8];
  double pi[5][6] = {
	 {0.00000000,   0.00000000,    0.00000000,    0.00000000,    0.00000000,    0.00000000},
	 {0.5000000000, 0.2500000000,  0.1250000000,  0.06250000000, 0.03125000000, 0.0312500000},
	 {0.7500000000, 0.06250000000, 0.04687500000, 0.03515625000, 0.02636718750, 0.0791015625},
	 {0.8333333333, 0.02777777778, 0.02314814815, 0.01929012346, 0.01607510288, 0.0803755143},
	 {0.8750000000, 0.01562500000, 0.01367187500, 0.01196289063, 0.01046752930, 0.0732727051}
  };


  S_k = (int *) MYCALLOC (n, sizeof (int));
  cycle = (int *) MYCALLOC (MAX (1000, n / 200), sizeof (int));
  {
    J = 0;			/* DETERMINE CYCLES */

    S_k[0] = 2 * (ARRAY[0] & 1) - 1;
    pt = 0;
    PT = 1;
    for (i = 1; i < n; i++)
      {
	S_k[i] = S_k[i - 1] + 2 * ((ARRAY[pt] >> PT) & 1) - 1;
	PT++;
	if (PT == 32)
	  {
	    pt++;
	    PT = 0;
	  }
	if (S_k[i] == 0)
	  {
	    J++;
	    if (J > MAX (1000, n / 128))
	      {
#ifdef IO
		{
		  fprintf
		    (fileout,
		     "ERROR IN FUNCTION randomExcursions:  EXCEEDING THE MAX");
		  fprintf (fileout, " NUMBER OF CYCLES EXPECTED\n.");
		}
#endif
		MYFREE ((char*)cycle);
		MYFREE ((char*)S_k);

		return (fail);
	      }
	    cycle[J] = i;
	  }
      }
    if (S_k[n - 1] != 0)
      {
	J++;

      }


    constraint = MAX (0.005 * pow (n, 0.5), 500);
    if (J < constraint)
      {

#ifdef IO
	{
	  fprintf (fileout,
		   "\t\t---------------------------------------------");
	  fprintf (fileout,
		   "\n\t\tWARNING:  TEST NOT APPLICABLE.  THERE ARE ");
	  fprintf (fileout, "AN\n\t  INSUFFICIENT NUMBER OF CYCLES.\n");
	  fprintf (fileout,
		   "\t\t---------------------------------------------");
	  fprintf (fileout, "\n");
	}
#endif

      }
    else
      {
	cycleStart = 0;
	cycleStop = cycle[1];
	for (k = 0; k < 6; k++)
	  for (i = 0; i < 8; i++)
	    nu[k][i] = 0.;
	for (j = 1; j <= J; j++)
	  {			/* FOR EACH CYCLE */
	    for (i = 0; i < 8; i++)
	      counter[i] = 0;
	    for (i = cycleStart; i < cycleStop; i++)
	      {
		if ((S_k[i] >= 1 && S_k[i] <= 4)
		    || (S_k[i] >= -4 && S_k[i] <= -1))
		  {
		    if (S_k[i] < 0)
		      b = 4;
		    else
		      b = 3;
		    counter[S_k[i] + b]++;
		  }
	      }
	    cycleStart = cycle[j] + 1;
	    if (j < J)
	      cycleStop = cycle[j + 1];
	    else
	      cycleStop = n;

	    for (i = 0; i < 8; i++)
	      {
		if (counter[i] >= 0 && counter[i] <= 4)
		  nu[counter[i]][i]++;
		else if (counter[i] >= 5)
		  nu[5][i]++;
	      }
	  }
	for (i = 0; i < 8; i++)
	  {
	    x = stateX[i];
	    sum = 0.;
	    for (k = 0; k < 6; k++)
	      {
		sum += pow (nu[k][i] - J * pi[(int) fabs (x)][k], 2) /
		  (J * pi[(int) fabs (x)][k]);
	      }
	    p_value = igamc (2.5, sum / 2.);

	    if (p_value < THRESHOLDFAIL)
	      fail++;
	    else
	      TotalTEST++;

#ifdef IO
	    {
	      fprintf (fileout, "p_value= %f\n", p_value);
	    }
#endif



	  }
      }

#ifdef IO
    {
      fprintf (fileout, "\n");
    }
#endif

    MYFREE ((char*)cycle);
    MYFREE ((char*)S_k);
  }
  return (fail);
}

int
CumulativeSums (int n, int *ARRAY)
{
  int i, k, start, finish, mode;
  double p_value, cusum, sum, sum1, sum2;
  int z;
  int pt, PT;
  int fail = 0;
  for (mode = 0; mode < 2; mode++)
    {				/* mode = {0,1}  => {forward,reverse} */
      sum = 0.0;
      cusum = 1.0;
      if (mode == 0)
	{
	  pt = 0;
	  PT = 0;

	  for (i = 0; i < n; i++)
	    {
	      sum += (double) (2 * ((ARRAY[pt] >> PT) & 1) - 1);
	      PT++;
	      if (PT == 32)
		{
		  PT = 0;
		  pt++;
		}
	      cusum = MAX (cusum, fabs (sum));
	    }
	}
      else if (mode == 1)
	{
	  pt = (n >> 5);
	  PT = 31;
	  for (i = n - 1; i >= 0; i--)
	    {
	      sum += (double) (2 * ((ARRAY[pt] >> PT) & 1) - 1);
	      PT--;
	      if (PT == -1)
		{
		  PT = 31;
		  pt--;
		}
	      cusum = MAX (cusum, fabs (sum));
	    }
	}
      z = (int) cusum;

      sum1 = 0.0;
      start = (-n / z + 1) / 4;
      finish = (n / z - 1) / 4;
      for (k = start; k <= finish; k++)
	sum1 +=
	  (normal ((4 * k + 1) * z / sqrt (n)) -
	   normal ((4 * k - 1) * z / sqrt (n)));

      sum2 = 0.0;
      start = (-n / z - 3) / 4;
      finish = (n / z - 1) / 4;
      for (k = start; k <= finish; k++)
	sum2 +=
	  (normal ((double) ((4 * k + 3) * z) / sqrt ((double) n)) -
	   normal ((double) ((4 * k + 1) * z) / sqrt (n)));
      p_value = 1.0 - sum1 + sum2;
      if (mode == 1)
{	if (p_value < THRESHOLDFAIL)
	  fail++;
	else
	  TotalTEST++;

#ifdef IO
      {
	fprintf (fileout, "%d bits sequence, reverse p_value= %f \n", n,
		 p_value);
      }
#endif
 }
     if (mode == 0){
	if (p_value < THRESHOLDFAIL)
	  fail++;
	else
	  TotalTEST++;

#ifdef IO
      {
	fprintf (fileout, "%d bits sequence, forward p_value= %f \n", n,
		 p_value);
      }
#endif
}
    }
  return (fail);

}



int
ApproximateEntropy (int mmin, int mmax, int n, int *ARRAY)
{
  int i, blockSize, seqLength;
  int powLen;
  double sum, numOfBlocks, ApEn[25], apen, chi_squared, p_value;
  unsigned int *P[25];
  int pt, PT, DATA;
  int fail = 0;
  int MaskEnt[25] =
    { 0, 1, 3, 7, 0xf, 0x1f, 0x3f, 0x7f, 0xff, 0x1ff, 0x3ff, 0x7ff, 0xfff,
    0x1fff, 0x3fff, 0x7fff, 0xffff, 0x1ffff, 0x3ffff, 0x7ffff, 0xfffff,
    0x1fffff,
    0x3fffff, 0x7fffff, 0xffffff
  };

  seqLength = n - (mmax);
  numOfBlocks = (double) seqLength;
  for (blockSize = mmin; blockSize <= mmax; blockSize++)
    {
      powLen = (int) pow (2, blockSize + 1);
      if ((P[blockSize] =
	   (unsigned int *) MYCALLOC (powLen, sizeof (unsigned int))) == NULL)
	{

#ifdef IO
	  {
	    fprintf (fileout, "ApEn:  Insufficient memory available.\n");
	  }
#endif
	  exit (1);
	  return (fail);
	}
      for (i = 0; i < powLen; i++)
	P[blockSize][i] = 0;
    }

  for (blockSize = mmin; blockSize <= mmax; blockSize++)
    {
      DATA = ARRAY[0];
      pt = 0;
      PT = mmax;
      for (i = 0; i < seqLength; i++)
	{			/* COMPUTE FREQUENCY */

	  (P[blockSize][DATA & MaskEnt[blockSize]])++;
	  PT++;
	  if (PT == 32)
	    {
	      PT = 0;
	      pt++;
	    };
	  DATA = (DATA << 1) + ((ARRAY[pt] >> PT) & 1);
	}
    }

  for (blockSize = mmin; blockSize <= mmax; blockSize++)
    {
      sum = 0.0;

      for (i = 0; i < (int) pow (2, blockSize); i++)
	{
	  if (P[blockSize][i] > 0)
	    sum +=
	      (((double) P[blockSize][i]) / (numOfBlocks)) *
	      log ((double) P[blockSize][i] / numOfBlocks);
	}

      ApEn[blockSize] = sum;
    }
  for (blockSize = mmax; blockSize >= mmin; blockSize--)
    MYFREE ((char*)P[blockSize]);

  for (i = mmin; i < mmax; i++)
    {
      apen = ApEn[i] - ApEn[i + 1];
      chi_squared = 2.0 * ((double) seqLength) * (log (2) - apen);
      p_value = igamc (pow (2, i - 1), chi_squared / 2.);
      if (p_value < THRESHOLDFAIL)
	fail++;
      else
	TotalTEST++;
#ifdef IO
      {
	fprintf (fileout, "m= %d,\t p_value= %f, Entropy per %d bits %f \n",
		 i, p_value, i, -ApEn[i] / log (2));
      }
#endif
    }

  return (fail);
}

int
Serial (int m, int n, int *ARRAY, int PT)
{
  double p_value1, p_value2, psim0, psim1, psim2, del1, del2;
  int fail = 0;
  psim0 = psi2 (m, n, &ARRAY[PT >> 5], (PT & 31));
  psim1 = psi2 (m - 1, n, &ARRAY[PT >> 5], (PT & 31));
  psim2 = psi2 (m - 2, n, &ARRAY[PT >> 5], (PT & 31));
  del1 = psim0 - psim1;
  del2 = psim0 - 2.0 * psim1 + psim2;
  p_value1 = igamc (pow (2, m - 1) / 2, del1 / 2.0);
  p_value2 = igamc (pow (2, m - 2) / 2, del2 / 2.0);

  if (p_value1 < THRESHOLDFAIL)
    fail++;
  else
    TotalTEST++;
#ifdef IO
  {
    fprintf (fileout, "p_value1= %f\n", p_value1);
  }
#endif
  if (p_value1 < THRESHOLDFAIL)
    fail++;
  else
    TotalTEST++;
#ifdef IO
  {
    fprintf (fileout, "p_value2= %f\n", p_value2);
  }
#endif
  return (fail);
}


double
psi2 (int m, int n, int *ARRAY, int PT)
{
  int i, j, k;
  double sum, numOfBlocks;
  unsigned int *P;

  if ((m == 0) || (m == -1))
    return 0.0;
  numOfBlocks = n;

  P = (unsigned int*) MYCALLOC (65536, sizeof (unsigned int));
  for (i = 0; i < numOfBlocks; i++)
    {				/* COMPUTE FREQUENCY */
      k = 1;
      for (j = 0; j < m; j++)
	{

	  if (((ARRAY[(MOD (PT + i + j, n) >> 5)] >>
		(MOD (PT + i + j, n) & 31)) & 1) == 0)
	    k *= 2;
	  else
	    k = 2 * k + 1;
	}
      P[k - 1]++;
    }
  sum = 0.0;
  for (i = (int) pow (2, m) - 1; i < (int) pow (2, m + 1) - 1; i++)
    sum += pow (P[i], 2);
  sum = (sum * pow (2, m) / (double) n) - (double) n;
  MYFREE ((char*)P);
  return sum;
}


int
LinearComplexity (int M, int N, int *ARRAY, int PT)
{
  int i, ii, j, d;
  int L, m, N_, parity, sign;
  double p_value, T_, mean;
  int fail = 0;
  int K = 6;
  double pi[7] =
    { 0.01047, 0.03125, 0.12500, 0.50000, 0.25000, 0.06250, 0.020833 };
  double nu[7], chi2;
  int *T, *P, *B_, *C;

  B_ = (int *) MYCALLOC (M, sizeof (int));
  C = (int *) MYCALLOC (M, sizeof (int));
  P = (int *) MYCALLOC (M, sizeof (int));
  T = (int *) MYCALLOC (M, sizeof (int));

  for (i = 0; i < K + 1; i++)
    nu[i] = 0.00;
  for (ii = 0; ii < N; ii++)
    {
      for (i = 0; i < M; i++)
	{
	  B_[i] = 0;
	  C[i] = 0;
	  T[i] = 0;
	  P[i] = 0;
	}
      L = 0;
      m = -1;
      d = 0;
      C[0] = 1;
      B_[0] = 1;
      /* DETERMINE LINEAR COMPLEXITY */
      N_ = 0;

      while (N_ < M)
	{
	  d =
	    ((ARRAY[(ii * M + N_ + PT) >> 5]) >>
	     ((ii * M + N_ + PT) & 31)) & 1;

	  for (i = 1; i <= L; i++)

	    d +=
	      (C[i] &
	       (((ARRAY[(ii * M + N_ - i + PT) >> 5]) >>
		 ((ii * M + N_ - i + PT) & 31)) & 1));
	  d = d & 1;
	  if (d == 1)
	    {
	      for (i = 0; i < M; i++)
		{
		  T[i] = C[i];
		  P[i] = 0;
		}
	      for (j = 0; j < M; j++)
		if (B_[j] == 1)
		  P[j + N_ - m] = 1;
	      for (i = 0; i < M; i++)
		C[i] = (C[i] + P[i]) & 1;
	      if (L <= N_ / 2)
		{
		  L = N_ + 1 - L;
		  m = N_;
		  for (i = 0; i < M; i++)
		    B_[i] = T[i];
		}
	    }
	  N_++;
	}
      if ((parity = (M + 1) % 2) == 0)
	sign = -1;
      else
	sign = 1;
      mean =
	M / 2. + (9. + sign) / 36. - 1. / pow (2, M) * (M / 3. + 2. / 9.);
      if ((parity = M % 2) == 0)
	sign = 1;
      else
	sign = -1;
      T_ = sign * (L - mean) + 2. / 9.;

      if (T_ <= -2.5)
	nu[0]++;
      else if (T_ > -2.5 && T_ <= -1.5)
	nu[1]++;
      else if (T_ > -1.5 && T_ <= -0.5)
	nu[2]++;
      else if (T_ > -0.5 && T_ <= 0.5)
	nu[3]++;
      else if (T_ > 0.5 && T_ <= 1.5)
	nu[4]++;
      else if (T_ > 1.5 && T_ <= 2.5)
	nu[5]++;
      else
	nu[6]++;


    }
  chi2 = 0.00;
  for (i = 0; i < K + 1; i++)
    chi2 += pow (nu[i] - N * pi[i], 2) / (N * pi[i]);
  p_value = igamc (K / 2.0, chi2 / 2.0);
  if (p_value < THRESHOLDFAIL)
    fail++;
  else
    TotalTEST++;
#ifdef IO
  {
    fprintf (fileout, "p_value= %f\n", p_value);
  }
#endif

  MYFREE ((char*)B_);
  return (fail);
}


int
LempelZivCompression (int n, int *ARRAY, int *DATA, int *pt, int *PT)
{
  int W;			/* Number of words */
  int i, j, k, prev_I, powLen, max_len;
  int done = 0;
  double p_value, mean=0.0, variance=0.0;
  BitField *P;
  int fail = 0;
  W = 0;
  k = (int) log (n) / log (2) + 6;
  powLen = pow (2, k);

  if ((P = (BitField *) MYCALLOC (powLen, sizeof (BitField))) == NULL)
    {

#ifdef IO
      {
	fprintf (fileout, "\t\tLempel-Ziv Compression has been aborted,\n");
	fprintf (fileout, "\t\tdue to insufficient workspace!\n");
      }
#endif
    }
  else
    {
      for (i = 0; i < powLen; i++)
	P[i].b = 0;
      i = 0;
      max_len = 0;
      while (i <= n - 1)
	{
	  done = 0;
	  j = 1;
	  prev_I = i;
	  while (!done)
	    {
	      if (2 * j + 1 <= powLen)
		{
		  if ((*DATA & 1) == 0)
		    {
		      if (P[2 * j].b == 1)
			{
			  j *= 2;
			}
		      else
			{
			  P[2 * j].b = 1;
			  done = 1;
			}
		    }
		  else
		    {
		      if (P[2 * j + 1].b == 1)
			{
			  j = 2 * j + 1;
			}
		      else
			{
			  P[2 * j + 1].b = 1;
			  done = 1;
			}
		    }
		  (*PT)++;
		  if (*PT == 32)
		    {
		      (*pt)++;
		      *DATA = ARRAY[*pt];
		      *PT = 0;
		    }
		  else
		    *DATA = *DATA >> 1;
		  i++;
		  if (i > n - 1)
		    {
		      done = 1;
		    }
		  if (done)
		    {
		      W++;
		      max_len = MAX (max_len, i - prev_I);
		    }
		}
	      else
		{

#ifdef IO
		  {
		    fprintf (fileout,
			     "\t\tWARNING: Segmentation Violation Imminent.");
		    fprintf (fileout,
			     "\n\t Lempel-Ziv Compression Terminated.\n");
		    fprintf (fileout,
			     "\t\t-----------------------------------------\n");
		    fflush (fileout);
		  }
#endif
		  done = 1;
		  i = n;
		}
	    }
	}
    }
  switch (n)
    {
    case 100000:
      mean = 8782.500000;
      variance = 11.589744;
      break;
    case 200000:
      mean = 16292.1000;
      variance = 21.4632;
      break;
    case 400000:
      mean = 30361.9500;
      variance = 58.7868;
      break;
    case 600000:
      mean = 43787.5000;
      variance = 70.1579;
      break;
    case 800000:
      mean = 56821.9500;
      variance = 67.4184;
      break;
    case 1000000:		/* Updated Mean and Variance 10/26/99 */
      mean = 69588.20190000;
      variance = 73.23726011;
      /* Previous Mean and Variance
         mean = 69586.250000;
         variance = 70.448718;
       */
      break;
    default:
      break;
    }
  p_value = 0.5 * erfc ((mean - W) / pow (2.0 * variance, 0.5));
  if (p_value < THRESHOLDFAIL)
    fail++;
  else
    TotalTEST++;
#ifdef IO
  {
    fprintf (fileout, "p_value= %f\n", p_value);
  }
#endif
  MYFREE ((char*)P);
  return (fail);
}


int
DiscreteFourierTransform (int N, int *ARRAY)
{
  double p_value, upperBound, *m, *X;
  int i, count;
  double N_l, N_o, d;
  double *wsave;
  int n, j, J, *ifac;
  int fail = 0;
  n = 32 * N;
  X = (double *) MYCALLOC (n, sizeof (double));
  wsave = (double *) MYCALLOC (2 * n + 15, sizeof (double));
  ifac = (int *) MYCALLOC (15, sizeof (double));
  m = (double *) MYCALLOC (n / 2 + 1, sizeof (double));
  {
    J = 0;
    for (i = 0; i < N; i++)
      for (j = 0; j < 32; j++)
	{
	  X[J] = (2 * ((ARRAY[i] >> j) & 1)) - 1;
	  J++;
	}
    __ogg_fdrffti (n, wsave, ifac);	/* INITIALIZE WORK ARRAYS */
    __ogg_fdrfftf (n, X, wsave, ifac);	/* APPLY FORWARD FFT      */

    m[0] = sqrt (X[0] * X[0]);	/* COMPUTE MAGNITUDE      */


    for (i = 0; i < n / 2; i++)
      {				/* DISPLAY FOURIER POINTS */
	m[i + 1] = sqrt (pow (X[2 * i + 1], 2) + pow (X[2 * i + 2], 2));
      }
    count = 0;			/* CONFIDENCE INTERVAL */
    upperBound = sqrt (3 * n);
    for (i = 0; i < n / 2; i++)
      if (m[i] < upperBound)
	count++;
    N_l = (double) count;	/* number of peaks less than h = sqrt(3*n) */
    N_o = (double) 0.95 *n / 2.;
    d = (N_l - N_o) / sqrt (n / 2. * 0.95 * 0.05);
    p_value = erfc (fabs (d) / sqrt (2.));

    if (p_value < THRESHOLDFAIL)
      fail++;
    else
      TotalTEST++;

#ifdef IO
    {
      fprintf (fileout, "p_value= %f\n", p_value);
    }
#endif
  }
  MYFREE ((char*)m);
  MYFREE ((char*)ifac);
  MYFREE ((char*)wsave);
  MYFREE ((char*)X);
  return (fail);
}

int
LongestRunOfOnes (int n, int *ARRAY)
{
  double p_value, sum, chi2;
  int N, i, j;
  int run, v_n_obs;
  int fail = 0;
/* since we are not interested in short sequences we used only 10000*/


  double pi[7] = { 0.0882, 0.2092, 0.2483, 0.1933, 0.1208, 0.0675, 0.0727 };
  double K = 6;
  unsigned int nu[7] = { 0, 0, 0, 0, 0, 0, 0 };
  int M = 10000;
  int PT;
  int pt;
  int DATA;
  pt = 0;
  PT = 0;
  DATA = ARRAY[0];
  N = (int) floor ((32 * n) / M);
  for (i = 0; i < N; i++)
    {
      v_n_obs = 0;
      run = 0;
      for (j = i * M; j < (i + 1) * M; j++)
	{
	  if (DATA & 1)
	    {
	      run++;
	      v_n_obs = MAX (v_n_obs, run);
	    }
	  else
	    run = 0;
	  PT++;
	  if (PT == 32)
	    {
	      PT = 0;
	      pt++;
	      DATA = ARRAY[pt];
	    }
	  else
	    DATA = DATA >> 1;
	}
      if (v_n_obs <= 10)
	nu[0]++;
      else if (v_n_obs == 11)
	nu[1]++;
      else if (v_n_obs == 12)
	nu[2]++;
      else if (v_n_obs == 13)
	nu[3]++;
      else if (v_n_obs == 14)
	nu[4]++;
      else if (v_n_obs == 15)
	nu[5]++;
      else if (v_n_obs >= 16)
	nu[6]++;
    }
  chi2 = 0.0;			/* Compute Chi Square */
  sum = 0;
  for (i = 0; i < ((int) K) + 1; i++)
    {
      chi2 +=
	pow ((double) nu[i] - ((double) N * pi[i]), 2) / ((double) N * pi[i]);
      sum += nu[i];
    }
  p_value = igamc (K / 2., chi2 / 2.);
  if (p_value < THRESHOLDFAIL)
    fail++;
  else
    TotalTEST++;
#ifdef IO
  {
    fprintf (fileout, "p_value= %f\n", p_value);
  }
#endif
  return (fail);
}

int
Runs (int n, int *ARRAY)
{
  int i, j;
  double argument, pi, V_n_obs, tau;
  double p_value, product;
  int SUM;
  double N;
  int fail = 0;
  int V_N_obs;
  N = (double) (32 * (n - 1) + 1);

  SUM = 0;
  for (i = 0; i < n - 1; i++)
    for (j = 0; j < 32; j++)
      {
	SUM += (ARRAY[i] >> j) & 1;
      }
  SUM += (ARRAY[n] & 1);

  pi = ((double) SUM) / N;
  tau = 2.0 / sqrt (N);

  if (fabs (pi - 0.5) < tau)
    {
      V_N_obs = 0;
      for (i = 0; i < n - 1; i++)
	{
	  for (j = 0; j < 31; j++)
	    V_N_obs += (((ARRAY[i] >> j) ^ (ARRAY[i] >> (j + 1))) & 1);
	  V_N_obs += ((ARRAY[i] >> 31) ^ (ARRAY[i + 1])) & 1;
	}

      V_N_obs++;
      V_n_obs = (double) V_N_obs;
      product = pi * (1.e0 - pi);
      argument =
	fabs (V_n_obs -
	      2.e0 * N * product) / (2.e0 * sqrt (2.e0 * N) * product);
      p_value = erfc (argument);
    }
  else
    {
      p_value = 0.0;
    }
  if (p_value < THRESHOLDFAIL)
    fail++;
  else
    TotalTEST++;
#ifdef IO
  {
    fprintf (fileout, "p_value=%f\n", p_value);
  }
#endif
  return (fail);
}

int
Frequency (int n, int *ARRAY)
{
  int i, j;
  double f, s_obs, p_value;
  double sqrt2 = 1.41421356237309504880;
  int SUM;
  int fail = 0;
  SUM = 0;

  for (i = 0; i < n; i++)
    {
      for (j = 0; j < 32; j++)
	SUM += (2 * ((ARRAY[i] >> j) & 1)) - 1;
    }
  s_obs = fabs ((double) SUM) / sqrt (32.0 * ((double) n));
  f = s_obs / sqrt2;
  p_value = erfc (f);
  if (p_value < THRESHOLDFAIL)
    fail++;
  else
    TotalTEST++;
#ifdef IO
  {
    fprintf (fileout, "%d bits sequence, p_value= %f\n", 32 * n, p_value);
  }
#endif
  return (fail);
}

int
BlockFrequency (int ArraySize, int m, int *ARRAY)
{
  int i, j, N, n;
  double arg1, arg2, p_value;
  double sum, pi, v, chi_squared;
  int BlockSum;
  int PT;
  int pt;
  int DATA;
  int fail = 0;
  n = ArraySize * 32;
  pt = 0;
  PT = 0;
  DATA = ARRAY[0];
  N = (int) floor ((double) n / (double) m);
  sum = 0.0;
  for (i = 0; i < N; i++)
    {
      pi = 0.0;
      BlockSum = 0;
      for (j = 0; j < m; j++)
	{
	  BlockSum += (DATA & 1);
	  PT++;
	  if (PT == 32)
	    {
	      PT = 0;
	      pt++;
	      DATA = ARRAY[pt];
	    }
	  else
	    DATA = DATA >> 1;
	}

      pi = (double) BlockSum / (double) m;
      v = pi - 0.5;
      sum += v * v;
    }
  chi_squared = 4.0 * m * sum;
  arg1 = (double) N / 2.e0;
  arg2 = chi_squared / 2.e0;
  p_value = igamc (arg1, arg2);

  if (p_value < THRESHOLDFAIL)
    fail++;
  else
    TotalTEST++;
#ifdef IO
  {
    fprintf (fileout, " %d bits sequence, p_value= %f\n", n, p_value);
  }
#endif
  return (fail);
}






int SizeMin[18] =
  { 0, 0, 0, 0, 0, 0, 387840, 904960, 2068480, 4654080, 10342400, 22753280,
  49643520, 107560960, 231669760, 496435200, 1059061760, 0x7fffffff
};


int
Universal (int n, int *ARRAY)
{
  int I, param;
  int fail = 0;
  if (32 * n < SizeMin[6])
    {

#ifdef IO
      {
	fprintf (fileout, "Too small\n");
      }
#endif
      return (fail);
    }
  I = 6;
  while ((32 * n >= SizeMin[I]) & (I < 17))
    {
      if (32 * n >= SizeMin[I + 1])
	param = SizeMin[I + 1] - 1;
      else
	param = 32 * n;
      fail += UNIVERSAL (param, ARRAY);
      I++;
    }
  return (fail);
}




int
UNIVERSAL (int n, int *ARRAY)
{
  int i, j, p, K, L, Q;
  double arg, sqrt2, sigma, phi, sum, p_value, c;
  long *T, decRep;
  double expected_value[17] = {
    0, 0, 0, 0, 0, 0, 5.2177052, 6.1962507, 7.1836656,
    8.1764248, 9.1723243, 10.170032, 11.168765,
    12.168070, 13.167693, 14.167488, 15.167379
  };
  double variance[17] = {
    0, 0, 0, 0, 0, 0, 2.954, 3.125, 3.238, 3.311, 3.356, 3.384,
    3.401, 3.410, 3.416, 3.419, 3.421
  };
  int PT, DATA, pt;
  int fail = 0;
  double Pow[20];
  double POW;
  if (n >= 387840)
    L = 6;
  if (n >= 904960)
    L = 7;
  if (n >= 2068480)
    L = 8;
  if (n >= 4654080)
    L = 9;
  if (n >= 10342400)
    L = 10;
  if (n >= 22753280)
    L = 11;
  if (n >= 49643520)
    L = 12;
  if (n >= 107560960)
    L = 13;
  if (n >= 231669760)
    L = 14;
  if (n >= 496435200)
    L = 15;
  if (n >= 1059061760)
    L = 16;
  PT = 0;
  pt = 0;
  DATA = ARRAY[pt];
  POW = pow (2, L);
  for (i = 0; i <= L; i++)
    Pow[i] = pow (2, i);
  Q = (int) 10 *POW;
  K = (int) (floor (n / L) - (double) Q);
  c =
    0.7 - 0.8 / (double) L + (4 + 32 / (double) L) * pow (K,
							  -3 /
							  (double) L) / 15;
  sigma = c * sqrt (variance[L] / (double) K);
  sqrt2 = sqrt (2);
  sum = 0.0;
  p = (int) pow (2, L);
  T = (long *) MYCALLOC (p, sizeof (long));
  for (i = 0; i < p; i++)
    T[i] = 0;
  for (i = 1; i <= Q; i++)
    {
      decRep = 0;
      for (j = 0; j < L; j++)
	{
	  decRep += (DATA & 1) * Pow[L - 1 - j];
	  PT++;
	  if (PT == 32)
	    {
	      pt++;
	      DATA = ARRAY[pt];
	      PT = 0;
	    }
	  else
	    DATA = DATA >> 1;
	}
      T[decRep] = i;
    }
  for (i = Q + 1; i <= Q + K; i++)
    {
      decRep = 0;
      for (j = 0; j < L; j++)
	{
	  decRep += (DATA & 1) * Pow[L - 1 - j];
	  PT++;
	  if (PT == 32)
	    {
	      pt++;
	      DATA = ARRAY[pt];
	      PT = 0;
	    }
	  else
	    DATA = DATA >> 1;
	}
      sum += log (i - T[decRep]) / log (2);
      T[decRep] = i;
    }
  phi = (double) (sum / (double) K);
  arg = fabs (phi - expected_value[L]) / (sqrt2 * sigma);
  p_value = erfc (arg);
  MYFREE ((char*)T);
  if (p_value < THRESHOLDFAIL)
    fail++;
  else
    TotalTEST++;
#ifdef IO
  {
    fprintf (fileout, "L= %d\tp_value %f\texp_value= %f\tphi = %f\n", L,
	     p_value, expected_value[L], phi);
  }
#endif
  return (fail);
}


BitField **
create_matrix (int M, int Q)
{
  int i;
  BitField **matrix;

  if ((matrix = (BitField **) MYCALLOC (M, sizeof (BitField *))) == NULL)
    {
      printf
	("ERROR IN FUNCTION create_matrix:  Insufficient memory available.");
      printf ("\n");
      fprintf (stderr,
	       "CREATE_MATRIX:  Insufficient memory for %dx%d matrix.\n", M,
	       M);
      return matrix;
    }
  else
    {
      for (i = 0; i < M; i++)
	{
	  if ((matrix[i] = (BitField *)MYCALLOC (Q, sizeof (BitField))) == NULL)
	    {
	      fprintf (stderr,
		       "CREATE_MATRIX:  Insufficient memory for %dx%d ", M,
		       M);
	      fprintf (stderr, "matrix.\n");
	      printf
		("ERROR IN FUNCTION create_matrix: Insufficient memory for ");
	      printf ("%dx%d matrix.\n", M, M);
	      return NULL;
	    }
	}
      return matrix;
    }
}


int
Rank (int n, int *ARRAY)
{
  int N = (int) floor (n / (32));	/* NUMBER OF MATRICES     */
  int r;
  double p_value, product;
  int i, k;
  double chi_squared, arg1;
  double p_32, p_31, p_30;	/* PROBABILITIES */
  double R;			/* RANK          */
  double F_32, F_31, F_30;	/* FREQUENCIES   */
  BitField **matrix = create_matrix (32, 32);
  int pt, PT, DATA;
  int fail = 0;
  pt = 0;
  PT = 0;
  DATA = ARRAY[0];
  r = 32;			/* COMPUTE PROBABILITIES */
  product = 1;
  for (i = 0; i <= r - 1; i++)
    product *=
      ((1.e0 - pow (2, i - 32)) * (1.e0 - pow (2, i - 32))) / (1.e0 -
							       pow (2,
								    i - r));
  p_32 = pow (2, r * (32 + 32 - r) - 32 * 32) * product;

  r = 31;
  product = 1;
  for (i = 0; i <= r - 1; i++)
    product *=
      ((1.e0 - pow (2, i - 32)) * (1.e0 - pow (2, i - 32))) / (1.e0 -
							       pow (2,
								    i - r));
  p_31 = pow (2, r * (32 + 32 - r) - 32 * 32) * product;

  p_30 = 1 - (p_32 + p_31);

  F_32 = 0;
  F_31 = 0;
  for (k = 0; k < N; k++)
    {				/* FOR EACH 32x32 MATRIX   */
      def_matrix (32, 32, matrix, k, &pt, &PT, &DATA, ARRAY);
      R = computeRank (32, 32, matrix);
      if (R == 32)
	F_32++;			/* DETERMINE FREQUENCIES */
      if (R == 31)
	F_31++;
    }
  F_30 = (double) N - (F_32 + F_31);

  chi_squared = (pow (F_32 - N * p_32, 2) / (double) (N * p_32) +
		 pow (F_31 - N * p_31, 2) / (double) (N * p_31) +
		 pow (F_30 - N * p_30, 2) / (double) (N * p_30));

  arg1 = -chi_squared / 2.e0;

  p_value = exp (arg1);

  MYFREE ((char*)matrix);

  if (p_value < THRESHOLDFAIL)
    fail++;
  else
    TotalTEST++;
#ifdef IO
  {
    fprintf (fileout, "p_value= %f\n", p_value);
  }
#endif
  return (fail);
}
#else
#include <stdio.h>
int
PackTestF (int *ARRAY, int ArraySize, char *C)
{
  fprintf (stderr,
	   "Sorry, on-line analysis is implemented using the CygWin math libraries,\nyou must recompile the application under the CygWin environment to allow online analysis\n");
}
int
PackTestL (int *ARRAY, int ArraySize, char *C)
{
  fprintf (stderr,
	   "Sorry, on-line analysis is implemented using the CygWin math libraries,\nyou must recompile the application under the CygWin environment to allow online analysis\n");
}
#endif
