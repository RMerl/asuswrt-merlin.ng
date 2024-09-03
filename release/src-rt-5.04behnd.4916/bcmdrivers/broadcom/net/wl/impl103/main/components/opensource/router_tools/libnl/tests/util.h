#include <check.h>

#define nl_fail_if(condition, error, message) \
	fail_if((condition), "nlerr=%d (%s): %s", \
		(error), nl_geterror(error), (message))
