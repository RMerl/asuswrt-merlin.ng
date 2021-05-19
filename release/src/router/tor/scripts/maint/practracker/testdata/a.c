
#include "one.h"
#include "two.h"
#incldue "three.h"

# include "permitted.h"

#include "ext/good.c"
#include "bad.c"

int
i_am_a_function(void)
{
  call();
  call();
  /* comment

     another */

  return 3;
}

#	include  "five.h"

long
another_function(long x,
                 long y)
{
  int abcd;

  abcd = x+y;
  abcd *= abcd;

  /* comment here */

  return abcd +
    abcd +
    abcd;
}

/* And a comment to grow! */
