#ifndef __MY_GETTEXT_H
#define __MY_GETTEXT_H

#ifdef USES_CURSES
#define ENABLE_NLS_TEST ENABLE_NLS_IN_CURSES
#else
#define ENABLE_NLS_TEST ENABLE_NLS
#endif

#if ENABLE_NLS_TEST
# include <libintl.h>
#else
# define gettext(msgid) (msgid)
# define textdomain(domain)
# define bindtextdomain(domain, dir)
#endif

#define _(msgid) gettext (msgid)
#define gettext_noop(msgid) msgid
#define N_(msgid) gettext_noop (msgid)

#endif /* __MY_GETTEXT_H */
