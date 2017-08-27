#ifndef _VERSION_H_
#define _VERSION_H_
#include "first.h"

#ifdef HAVE_VERSION_H
# include "versionstamp.h"
#else
# define REPO_VERSION ""
#endif

#define PACKAGE_DESC PACKAGE_NAME "/" PACKAGE_VERSION REPO_VERSION

#endif
