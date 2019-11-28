/* source: filan.h */
/* Copyright Gerhard Rieger and contributors (see file CHANGES) */

/* Published under the GNU General Public License V.2, see file COPYING */

#ifndef __filan_h_included
#define __filan_h_included 1

struct sockaddr;	/* prevent gcc from spitting silly warning */
struct sockaddr_un;	/* prevent gcc from spitting silly warning */
struct sockaddr_in;	/* prevent gcc from spitting silly warning */
struct sockaddr_in6;	/* prevent gcc from spitting silly warning */

extern bool filan_followsymlinks;
extern bool filan_rawoutput;

extern int filan_file(const char *filename, FILE *outfile);
extern int filan_fd(int fd, FILE *outfile);
extern int filan_stat(
#if HAVE_STAT64
		      struct stat64 *buf
#else
		      struct stat *buf
#endif /* !HAVE_STAT64 */
		      , int statfd, int dynfd, FILE *outfile);

extern int cdevan(int fd, FILE *outfile);

#if _WITH_SOCKET
extern int isasocket(int fd);
extern int sockan(int fd, FILE *outfile);
extern int ipan(int fd, FILE *outfile);
extern int ip6an(int fd, FILE *outfile);
#endif /* _WITH_SOCKET */

extern
   int fdname(const char *file, int fd, FILE *outfile, const char *numform);

#endif /* !defined(__filan_h_included) */
