#include <linux/console.h>
#include <linux/types.h>
#include <linux/wait.h>

#include "speakup.h"
#include "spk_priv.h"

#define SYNTH_BUF_SIZE 8192	/* currently 8K bytes */

static u_char synth_buffer[SYNTH_BUF_SIZE];	/* guess what this is for! */
static u_char *buff_in = synth_buffer;
static u_char *buff_out = synth_buffer;
static u_char *buffer_end = synth_buffer + SYNTH_BUF_SIZE - 1;

/* These try to throttle applications by stopping the TTYs
 * Note: we need to make sure that we will restart them eventually, which is
 * usually not possible to do from the notifiers. TODO: it should be possible
 * starting from linux 2.6.26.
 *
 * So we only stop when we know alive == 1 (else we discard the data anyway),
 * and the alive synth will eventually call start_ttys from the thread context.
 */
void speakup_start_ttys(void)
{
	int i;

	for (i = 0; i < MAX_NR_CONSOLES; i++) {
		if (speakup_console[i] && speakup_console[i]->tty_stopped)
			continue;
		if ((vc_cons[i].d != NULL) && (vc_cons[i].d->port.tty != NULL))
			start_tty(vc_cons[i].d->port.tty);
	}
}
EXPORT_SYMBOL_GPL(speakup_start_ttys);

static void speakup_stop_ttys(void)
{
	int i;

	for (i = 0; i < MAX_NR_CONSOLES; i++)
		if ((vc_cons[i].d != NULL) && (vc_cons[i].d->port.tty != NULL))
			stop_tty(vc_cons[i].d->port.tty);
}

static int synth_buffer_free(void)
{
	int bytes_free;

	if (buff_in >= buff_out)
		bytes_free = SYNTH_BUF_SIZE - (buff_in - buff_out);
	else
		bytes_free = buff_out - buff_in;
	return bytes_free;
}

int synth_buffer_empty(void)
{
	return (buff_in == buff_out);
}
EXPORT_SYMBOL_GPL(synth_buffer_empty);

void synth_buffer_add(char ch)
{
	if (!synth->alive) {
		/* This makes sure that we won't stop TTYs if there is no synth
		 * to restart them */
		return;
	}
	if (synth_buffer_free() <= 100) {
		synth_start();
		speakup_stop_ttys();
	}
	if (synth_buffer_free() <= 1)
		return;
	*buff_in++ = ch;
	if (buff_in > buffer_end)
		buff_in = synth_buffer;
}

char synth_buffer_getc(void)
{
	char ch;

	if (buff_out == buff_in)
		return 0;
	ch = *buff_out++;
	if (buff_out > buffer_end)
		buff_out = synth_buffer;
	return ch;
}
EXPORT_SYMBOL_GPL(synth_buffer_getc);

char synth_buffer_peek(void)
{
	if (buff_out == buff_in)
		return 0;
	return *buff_out;
}
EXPORT_SYMBOL_GPL(synth_buffer_peek);

void synth_buffer_clear(void)
{
	buff_in = buff_out = synth_buffer;
}
EXPORT_SYMBOL_GPL(synth_buffer_clear);
