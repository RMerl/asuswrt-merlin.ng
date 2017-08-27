#include <sys/types.h>
#include <sys/stat.h>
#include <stdbool.h>

bool euidaccess_stat (struct stat const *st, int mode);
