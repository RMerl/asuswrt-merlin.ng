#ifndef _CHECK_COMPAT_H
#define _CHECK_COMPAT_H

#if (CHECK_MAJOR_VERSION == 0 && (CHECK_MINOR_VERSION < 9 || (CHECK_MINOR_VERSION == 9 && CHECK_MICRO_VERSION < 10)))
# define ck_assert_ptr_eq(X,Y) do {					\
		void* _ck_x = (X);					\
		void* _ck_y = (Y);					\
		ck_assert_msg(_ck_x == _ck_y,				\
			      "Assertion '"#X"=="#Y"' failed: "#X"==%p, "#Y"==%p", \
			      _ck_x, _ck_y);				\
	} while (0)
#endif

#endif
