#include <string.h>
#include "include/ebtables_u.h"

static struct ebt_u_replace replace;
void ebt_early_init_once();

extern void get_global_mutex();
extern void release_global_mutex();


int main(int argc, char *argv[])
{
	get_global_mutex();
	ebt_silent = 0;
	ebt_early_init_once();
	strcpy(replace.name, "filter");
	do_command(argc, argv, EXEC_STYLE_PRG, &replace);
	release_global_mutex();
	return 0;
}
