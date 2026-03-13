#ifndef LIB_GETOPT_H
#define LIB_GETOPT_H

extern char *optarg;
extern int optind, opterr, optopt;

struct option {
    const char *name;
    int has_arg;
    int *flag;
    int val;
};

enum {
    no_argument	  = 0,
    required_argument = 1,
    optional_argument = 2,
};

int getopt_long(int, char *const *, const char *,
		const struct option *, int *);

#endif /* LIB_GETOPT_H */
