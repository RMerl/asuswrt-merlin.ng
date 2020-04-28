#include <EXTERN.h>               /* from the Perl distribution     */
#include <perl.h>                 /* from the Perl distribution     */

extern void xs_init _((void));

static PerlInterpreter *iperl;  /***    The Perl interpreter    ***/

int
perl_main(int argc, char **argv, char **env)
{
	int	r;

	iperl = perl_alloc();
	perl_construct(iperl);
	perl_parse(iperl, xs_init, argc, argv, (char **)NULL);
	r = perl_run(iperl);

PerlIO_flush(PerlIO_stdout());
PerlIO_flush(PerlIO_stderr());

	perl_destruct(iperl);
	perl_free(iperl);
	return (r);
}
