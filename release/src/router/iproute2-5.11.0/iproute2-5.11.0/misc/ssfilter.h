#include <stdbool.h>

enum {
	SSF_DCOND,
	SSF_SCOND,
	SSF_OR,
	SSF_AND,
	SSF_NOT,
	SSF_D_GE,
	SSF_D_LE,
	SSF_S_GE,
	SSF_S_LE,
	SSF_S_AUTO,
	SSF_DEVCOND,
	SSF_MARKMASK,
	SSF_CGROUPCOND,
	SSF__MAX
};

bool ssfilter_is_supported(int type);

struct ssfilter
{
	int type;
	struct ssfilter *post;
	struct ssfilter *pred;
};

int ssfilter_parse(struct ssfilter **f, int argc, char **argv, FILE *fp);
void *parse_hostcond(char *addr, bool is_port);
void *parse_devcond(char *name);
void *parse_markmask(const char *markmask);
void *parse_cgroupcond(const char *path);
