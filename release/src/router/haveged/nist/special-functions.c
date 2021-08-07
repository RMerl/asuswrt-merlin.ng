#include <stdio.h>
#include <math.h>
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
                     S P E C I A L  F U N C T I O N S 
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#define LinuxorUnix
#ifdef WIN
#ifndef CYGWIN
#undef LinuxorUnix
/* same libraries are available*/
#endif
#endif

#ifdef LinuxorUnix
double normal(double x)
{
  double arg, result, sqrt2=1.414213562373095048801688724209698078569672;

  if (x > 0) {
    arg = x/sqrt2;
    result = 0.5 * ( 1 + erf(arg) );
  }
  else {
    arg = -x/sqrt2;
    result = 0.5 * ( 1 - erf(arg) );
  }
  return( result);
}

double normal2(double a)
{ return (1.0-0.5*erfc(a/sqrt(2.0))); }
#endif
