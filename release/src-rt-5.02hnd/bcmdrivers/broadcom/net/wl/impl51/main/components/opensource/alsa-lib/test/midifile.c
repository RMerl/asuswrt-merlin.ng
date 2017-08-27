/*
 * midifile 1.11
 *
 * Read and write a MIDI file.  Externally-assigned function pointers are
 * called upon recognizing things in the file.
 *
 * Original release ?
 * June 1989 - Added writing capability, M. Czeiszperger.
 *
 *          The file format implemented here is called
 *          Standard MIDI Files, and is part of the Musical
 *          instrument Digital Interface specification.
 *          The spec is available from:
 *
 *               International MIDI Association
 *               5316 West 57th Street
 *               Los Angeles, CA 90056
 *
 *          An in-depth description of the spec can also be found
 *          in the article "Introducing Standard MIDI Files", published
 *          in Electronic Musician magazine, April, 1989.
 *
 * February 1993 - Minor adjustments, Greg Lee:
 *	(1) can now set the global variable Mf_interactive to 1 to prevent the
 *	    reading functions from looking for file and track headers
 *	(2) can now write system exclusive data with
 *		mf_write_midi_event(delta_time, system_exclusive, 0, data, size)
 *	(3) changed definition of 'sequencer_specific' in midifile.h to 0x7f
 *	(4) changed mf_write_tempo to take additional delta_time as first argument
 *	    (since delta need not be zero)
 *	(5) added function mf_write_seqnum(unsigned long delta_time, unsigned seqnum)
 *	(6) changed mf_write_midi_event to use running status
 *	(7) removed the code to write an end of track meta event automatically
 *		-- this must now be done by the user of the library (I changed
 *		it because I need to be able to control the time delta of this
 *		 meta event)
 *	(8) added global variables Mf_division, Mf_currtempo, Mf_realtime, which
 *		are updated by the reading functions.  Mf_realtime is useful,
 *		because Mf_currtime does not really measure time at all, since
 *		its units change value at every tempo change.  Mf_realtime is
 *		the midi-time elapsed in units of 1/16 of a centisecond (but it
 *		does not handle SMPTE times)
 *	(9) maintains a history of tempo settings to update Mf_currtempo,
 *		to handle tempo tracks.
 *	(10) if there is an Mf_error function, the error routine no longer
 *		exits, leaving it to the application to do this.
 *	(11) chanmessage skips over invalid c1 command bytes > 127 and
 *		adjusts invalid c2 argument byte > 127 to 127.
 *	(12) readmt returns EOF when it encounters a 0 or 0x1a byte instead of an expected
 *		header string (some midi files have padding at end).
 */
#define NO_LC_DEFINES
#include "midifile.h"
#ifdef NO_LC_DEFINES
#define system_exclusive      	0xf0
#define	meta_event		0xFF
#define	set_tempo		0x51
#define lowerbyte(x) ((unsigned char)(x & 0xff))
#define upperbyte(x) ((unsigned char)((x & 0xff00)>>8))
#endif

#define NULLFUNC 0

#define THINK

#ifdef THINK
#include <stdlib.h>
#endif

#include <stdio.h>
#include <values.h>

#include <string.h>
/*void exit(), free();*/

/* public stuff */

/* Functions to be called while processing the MIDI file. */
int (*Mf_getc) () = NULLFUNC;
void (*Mf_error) () = NULLFUNC;
void (*Mf_header) () = NULLFUNC;
void (*Mf_trackstart) () = NULLFUNC;
void (*Mf_trackend) () = NULLFUNC;
void (*Mf_noteon) () = NULLFUNC;
void (*Mf_noteoff) () = NULLFUNC;
void (*Mf_pressure) () = NULLFUNC;
void (*Mf_parameter) () = NULLFUNC;
void (*Mf_pitchbend) () = NULLFUNC;
void (*Mf_program) () = NULLFUNC;
void (*Mf_chanpressure) () = NULLFUNC;
void (*Mf_sysex) () = NULLFUNC;
void (*Mf_arbitrary) () = NULLFUNC;
void (*Mf_metamisc) () = NULLFUNC;
void (*Mf_seqnum) () = NULLFUNC;
void (*Mf_eot) () = NULLFUNC;
void (*Mf_smpte) () = NULLFUNC;
void (*Mf_tempo) () = NULLFUNC;
void (*Mf_timesig) () = NULLFUNC;
void (*Mf_keysig) () = NULLFUNC;
void (*Mf_seqspecific) () = NULLFUNC;
void (*Mf_text) () = NULLFUNC;

/* Functions to implement in order to write a MIDI file */
int (*Mf_putc) () = NULLFUNC;
int (*Mf_writetrack) () = NULLFUNC;
int (*Mf_writetempotrack) () = NULLFUNC;

int Mf_nomerge = 0;		/* 1 => continue'ed system exclusives are */
 /* not collapsed. */
int Mf_interactive = 0;		/* 1 => file and track headers are not required */
unsigned long Mf_currtime = 0L;	/* current time in delta-time units */
unsigned long Mf_realtime = 0L;	/* current time in 1/16 centisecond-time units */
static double Mf_f_realtime = 0;/* as above, floating */
static double old_f_realtime = 0;
int Mf_division = 96;
unsigned long Mf_currtempo = 500000;
static unsigned long old_currtempo = 500000;
static unsigned long old_realtime = 0;
static unsigned long old_currtime = 0;
static unsigned long revised_time = 0;
static unsigned long tempo_change_time = 0;

#define MAX_HISTORY 512
static unsigned long tempo_history[MAX_HISTORY];
static unsigned long tempo_history_time[MAX_HISTORY];
static int tempo_history_count = 0;

/* private stuff */
static long Mf_toberead = 0L;
static long Mf_numbyteswritten = 0L;

static long readvarinum ();
static long read32bit ();
static long to32bit ();
static int read16bit ();
static int to16bit ();
static char *msg ();
static void readheader ();
static int readtrack ();
static void badbyte ();
static void metaevent ();
static void sysex ();
static void chanmessage ();
static void msginit ();
static int msgleng ();
static void msgadd ();
static void biggermsg ();
static int eputc (unsigned char c);

double mf_ticks2sec (unsigned long ticks, int division, unsigned long tempo);
int mf_write_meta_event ();
void mf_write_tempo ();
void mf_write_seqnum ();
void WriteVarLen ();

#ifdef READ_MODS
#include "mp_mod.c"
static int mod_file_flag = 0;
#endif /* READ_MODS */
static int force_exit;

void
mfread ()
{
  force_exit = 0;
  if (Mf_getc == NULLFUNC)
    mferror ("mfread() called without setting Mf_getc");

  readheader ();
#ifdef READ_MODS
  if (mod_file_flag)
    do_module();
  else
#endif
    while (readtrack () && !force_exit)
      ;
}

/* for backward compatibility with the original lib */
void
midifile ()
{
  mfread ();
}

static
int 
readmt (s)			/* read through the "MThd" or "MTrk" header string */
     char *s;
{
  int n = 0;
  char *p = s;
  int c;

  while (n++ < 4 && (c = (*Mf_getc) ()) != EOF)
    {
      if (c != *p++)
	{
	  char buff[32];
	  if (!c) return(EOF);
	  if (c == 0x1a) return(EOF);
	  (void) strcpy (buff, "expecting ");
	  (void) strcat (buff, s);
	  mferror (buff);
	  break;
	}
    }
  return (c);
}

static
int
egetc ()			/* read a single character and abort on EOF */
{
  int c = (*Mf_getc) ();

  if (c == EOF) {
    mferror ("premature EOF");
    force_exit = 1;
  }
  Mf_toberead--;
  return (c);
}

static
void 
readheader ()			/* read a header chunk */
{
  int format, ntrks, division;


  Mf_division = 96;
  Mf_currtempo = 500000;
  old_currtempo = 500000;
  tempo_history_count = 0;
  tempo_history[tempo_history_count] = Mf_currtempo;
  tempo_history_time[tempo_history_count] = 0;

  if (Mf_interactive)
    {
      Mf_toberead = 0;
      format = 0;
      ntrks = 1;
      division = 96;
    }
  else
#ifdef READ_MODS
    if (!strncmp(Mf_file_contents, "MThd", 4))
#endif
    {
      if (readmt ("MThd") == EOF)
	return;

      Mf_toberead = read32bit ();
      format = read16bit ();
      ntrks = read16bit ();
      Mf_division = division = read16bit ();
    }
#ifdef READ_MODS
  else
    {
      format = 0;
      ntrks = 1;
      division = Mf_division;
      Mf_toberead = 0;
      mod_file_flag = 1;
    }
#endif

  if (Mf_header)
    (*Mf_header) (format, ntrks, division);

  /* flush any extra stuff, in case the length of header is not 6 */
  while (Mf_toberead > 0 && !force_exit)
    (void) egetc ();
}


/*#define DEBUG_TIMES*/
static
unsigned long
find_tempo()
{
  int i;
  unsigned long old_tempo = Mf_currtempo;
  unsigned long new_tempo = Mf_currtempo;

  for (i = 0; i <= tempo_history_count; i++) {
    if (tempo_history_time[i] <= Mf_currtime) old_tempo = tempo_history[i];
    new_tempo = tempo_history[i];
    if (tempo_history_time[i] > revised_time) break;
  }
  if (i > tempo_history_count || tempo_history_time[i] > Mf_currtime) {
#ifdef DEBUG_TIMES
printf("[past %d, old_tempo %d]\n", tempo_history_time[i], old_tempo);
#endif
    revised_time = Mf_currtime;
    return(old_tempo);
  }
  tempo_change_time = revised_time = tempo_history_time[i];
#ifdef DEBUG_TIMES
printf("[revised_time %d, new_tempo %d]\n", revised_time, new_tempo);
#endif
  return(new_tempo);
}

static
int 
readtrack ()			/* read a track chunk */
{
  /* This array is indexed by the high half of a status byte.  It's */
  /* value is either the number of bytes needed (1 or 2) for a channel */
  /* message, or 0 (meaning it's not  a channel message). */
  static int chantype[] =
  {
    0, 0, 0, 0, 0, 0, 0, 0,	/* 0x00 through 0x70 */
    2, 2, 2, 2, 1, 1, 2, 0	/* 0x80 through 0xf0 */
  };
  long lookfor;
  int c, c1, type;
  int sysexcontinue = 0;	/* 1 if last message was an unfinished sysex */
  int running = 0;		/* 1 when running status used */
  int status = 0;		/* status value (e.g. 0x90==note-on) */
  int needed;

  if (Mf_interactive)
    {
      Mf_toberead = MAXINT;
    }
  else
    {
      if (readmt ("MTrk") == EOF)
	return (0);

      Mf_toberead = read32bit ();
    }
  Mf_currtime = Mf_realtime = 0;
  Mf_f_realtime = old_f_realtime = 0;
  old_currtime = old_realtime = 0;
  Mf_currtempo = find_tempo();

  if (Mf_trackstart)
    (*Mf_trackstart) ();

  while (!force_exit && (Mf_interactive || Mf_toberead > 0))
    {

      if (Mf_interactive)
	Mf_currtime += 1;
      else
	{
	  double delta_secs;
	  unsigned long delta_ticks = readvarinum ();
	  revised_time = Mf_currtime;
	  Mf_currtime += delta_ticks;	/* delta time */

/*
 * Step through each tempo change from old_currtime up to now,
 * revising Mf_realtime after each change.
 */

	  while (revised_time < Mf_currtime) {
	    unsigned long save_time = revised_time;
	    unsigned long save_tempo = Mf_currtempo;
	    Mf_currtempo = find_tempo();

	    if (Mf_currtempo != old_currtempo) {
	      old_currtempo = Mf_currtempo;
	      old_realtime = Mf_realtime;
	      if (revised_time != tempo_change_time) {
	        old_f_realtime = Mf_f_realtime;
	        old_currtime = save_time;
	      }
	    delta_secs = mf_ticks2sec (revised_time-old_currtime, Mf_division, save_tempo);
#ifdef DEBUG_TIMES
printf("d(rev %d - old %d, div %d, tempo %d) = %.3f\n",
revised_time, old_currtime, Mf_division, save_tempo, delta_secs * 1600.0);
#endif
	    Mf_f_realtime = old_f_realtime + delta_secs * 1600.0;
	    Mf_realtime = (unsigned long)(0.5 + Mf_f_realtime);
#ifdef DEBUG_TIMES
printf("\tt=%d ticks ( = %d csec/16 < old %.2f + %.2f)\n", Mf_currtime, Mf_realtime, 
old_f_realtime, delta_secs * 1600.0);
#endif
	      if (revised_time == tempo_change_time) {
		old_currtime = revised_time;
	      old_f_realtime = Mf_f_realtime;
	      }
	    }
	    else {
	    delta_secs = mf_ticks2sec (revised_time-old_currtime, Mf_division, Mf_currtempo);
#ifdef DEBUG_TIMES
printf("d(rev %d - old %d, div %d, tempo %d) = %.3f\n",
revised_time, old_currtime, Mf_division, Mf_currtempo, delta_secs * 1600.0);
#endif
	    Mf_f_realtime = old_f_realtime + delta_secs * 1600.0;
	    Mf_realtime = (unsigned long)(0.5 + Mf_f_realtime);
#ifdef DEBUG_TIMES
printf("\tt=%d ticks ( = %d csec/16 < old %.2f + %.2f)\n", Mf_currtime, Mf_realtime, 
old_f_realtime, delta_secs * 1600.0);
#endif
	    }


	  }
	}

      c = egetc ();

      if (sysexcontinue && c != 0xf7)
	mferror ("didn't find expected continuation of a sysex");

      if ((c & 0x80) == 0)
	{			/* running status? */
	  if (status == 0)
	    mferror ("unexpected running status");
	  running = 1;
	}
      else
	{
	  status = c;
	  running = 0;
	}

      needed = chantype[(status >> 4) & 0xf];

      if (needed)
	{			/* ie. is it a channel message? */

	  if (running)
	    c1 = c;
	  else
	    c1 = egetc ();
	  chanmessage (status, c1, (needed > 1) ? egetc () : 0);
	  continue;;
	}

      switch (c)
	{

	case 0xff:		/* meta event */

	  type = egetc ();
	  lookfor = Mf_toberead - readvarinum ();
	  msginit ();

	  while (Mf_toberead > lookfor)
	    msgadd (egetc ());

	  metaevent (type);
	  break;

	case 0xf0:		/* start of system exclusive */

	  lookfor = Mf_toberead - readvarinum ();
	  msginit ();
	  msgadd (0xf0);

	  while (Mf_toberead > lookfor)
	    msgadd (c = egetc ());

	  if (c == 0xf7 || Mf_nomerge == 0)
	    sysex ();
	  else
	    sysexcontinue = 1;	/* merge into next msg */
	  break;

	case 0xf7:		/* sysex continuation or arbitrary stuff */

	  lookfor = Mf_toberead - readvarinum ();

	  if (!sysexcontinue)
	    msginit ();

	  while (Mf_toberead > lookfor)
	    msgadd (c = egetc ());

	  if (!sysexcontinue)
	    {
	      if (Mf_arbitrary)
		(*Mf_arbitrary) (msgleng (), msg ());
	    }
	  else if (c == 0xf7)
	    {
	      sysex ();
	      sysexcontinue = 0;
	    }
	  break;
	default:
	  badbyte (c);
	  break;
	}
    }
  if (Mf_trackend)
    (*Mf_trackend) ();
  return (1);
}

static
void 
badbyte (c)
     int c;
{
  char buff[32];

  (void) sprintf (buff, "unexpected byte: 0x%02x", c);
  mferror (buff);
}

static
void 
metaevent (int type)
{
  int leng = msgleng ();
  char *m = msg ();

  switch (type)
    {
    case 0x00:
      if (Mf_seqnum)
	(*Mf_seqnum) (to16bit (m[0], m[1]));
      break;
    case 0x01:			/* Text event */
    case 0x02:			/* Copyright notice */
    case 0x03:			/* Sequence/Track name */
    case 0x04:			/* Instrument name */
    case 0x05:			/* Lyric */
    case 0x06:			/* Marker */
    case 0x07:			/* Cue point */
    case 0x08:
    case 0x09:
    case 0x0a:
    case 0x0b:
    case 0x0c:
    case 0x0d:
    case 0x0e:
    case 0x0f:
      /* These are all text events */
      if (Mf_text)
	(*Mf_text) (type, leng, m);
      break;
    case 0x2f:			/* End of Track */
      if (Mf_eot)
	(*Mf_eot) ();
      break;
    case 0x51:			/* Set tempo */
      if (Mf_tempo)
	(*Mf_tempo) (Mf_currtempo = to32bit (0, m[0], m[1], m[2]));
      if (tempo_history[tempo_history_count] == Mf_currtempo) break;
      if (tempo_history_time[tempo_history_count] > Mf_currtime) break;
      if (tempo_history_count < MAX_HISTORY - 1) tempo_history_count++;
      tempo_history[tempo_history_count] = Mf_currtempo;
      tempo_history_time[tempo_history_count] = Mf_currtime;
      break;
    case 0x54:
      if (Mf_smpte)
	(*Mf_smpte) (m[0], m[1], m[2], m[3], m[4]);
      break;
    case 0x58:
      if (Mf_timesig)
	(*Mf_timesig) (m[0], m[1], m[2], m[3]);
      break;
    case 0x59:
      if (Mf_keysig)
	(*Mf_keysig) (m[0], m[1]);
      break;
    case 0x7f:
      if (Mf_seqspecific)
	(*Mf_seqspecific) (leng, m);
      break;
    default:
      if (Mf_metamisc)
	(*Mf_metamisc) (type, leng, m);
    }
}

static
void 
sysex ()
{
  if (Mf_sysex)
    (*Mf_sysex) (msgleng (), msg ());
}

static
void 
chanmessage (status, c1, c2)
     int status;
     int c1, c2;
{
  int chan = status & 0xf;

  /* I found a midi file with Mod Wheel values 128. --gl */

  if (c1 > 127) /*mferror("chanmessage: bad c1") ??*/ return;
  if (c2 > 127) c2 = 127;

  switch (status & 0xf0)
    {
    case 0x80:
      if (Mf_noteoff)
	(*Mf_noteoff) (chan, c1, c2);
      break;
    case 0x90:
      if (Mf_noteon)
	(*Mf_noteon) (chan, c1, c2);
      break;
    case 0xa0:
      if (Mf_pressure)
	(*Mf_pressure) (chan, c1, c2);
      break;
    case 0xb0:
      if (Mf_parameter)
	(*Mf_parameter) (chan, c1, c2);
      break;
    case 0xe0:
      if (Mf_pitchbend)
	(*Mf_pitchbend) (chan, c1, c2);
      break;
    case 0xc0:
      if (Mf_program)
	(*Mf_program) (chan, c1);
      break;
    case 0xd0:
      if (Mf_chanpressure)
	(*Mf_chanpressure) (chan, c1);
      break;
    }
}

/* readvarinum - read a varying-length number, and return the */
/* number of characters it took. */

static long
readvarinum ()
{
  long value;
  int c;

  c = egetc ();
  value = c;
  if (c & 0x80)
    {
      value &= 0x7f;
      do
	{
	  c = egetc ();
	  value = (value << 7) + (c & 0x7f);
	}
      while (c & 0x80);
    }
  return (value);
}

static long
to32bit (int c1, int c2, int c3, int c4)
{
  long value = 0L;

  value = (c1 & 0xff);
  value = (value << 8) + (c2 & 0xff);
  value = (value << 8) + (c3 & 0xff);
  value = (value << 8) + (c4 & 0xff);
  return (value);
}

static int
to16bit (c1, c2)
     int c1, c2;
{
  return ((c1 & 0xff) << 8) + (c2 & 0xff);
}

static long
read32bit ()
{
  int c1, c2, c3, c4;

  c1 = egetc ();
  c2 = egetc ();
  c3 = egetc ();
  c4 = egetc ();
  return to32bit (c1, c2, c3, c4);
}

static int
read16bit ()
{
  int c1, c2;
  c1 = egetc ();
  c2 = egetc ();
  return to16bit (c1, c2);
}

/* static */
void
mferror (s)
     char *s;
{
  if (Mf_error)
    (*Mf_error) (s);
  else exit (1);
}

/* The code below allows collection of a system exclusive message of */
/* arbitrary length.  The Msgbuff is expanded as necessary.  The only */
/* visible data/routines are msginit(), msgadd(), msg(), msgleng(). */

#define MSGINCREMENT 128
static char *Msgbuff = NULL;	/* message buffer */
static int Msgsize = 0;		/* Size of currently allocated Msg */
static int Msgindex = 0;	/* index of next available location in Msg */

static
void 
msginit ()
{
  Msgindex = 0;
}

static char *
msg ()
{
  return (Msgbuff);
}

static
int 
msgleng ()
{
  return (Msgindex);
}

static
void 
msgadd (c)
     int c;
{
  /* If necessary, allocate larger message buffer. */
  if (Msgindex >= Msgsize)
    biggermsg ();
  Msgbuff[Msgindex++] = c;
}

static
void 
biggermsg ()
{
/* 	char *malloc(); */
  char *newmess;
  char *oldmess = Msgbuff;
  int oldleng = Msgsize;

  Msgsize += MSGINCREMENT;
  newmess = (char *) malloc ((unsigned) (sizeof (char) * Msgsize));

  if (newmess == NULL)
    mferror ("malloc error!");

  /* copy old message into larger new one */
  if (oldmess != NULL)
    {
      register char *p = newmess;
      register char *q = oldmess;
      register char *endq = &oldmess[oldleng];

      for (; q != endq; p++, q++)
	*p = *q;
      free (oldmess);
    }
  Msgbuff = newmess;
}

static int laststatus = 0;

/*
 * mfwrite() - The only function you'll need to call to write out
 *             a midi file.
 *
 * format      0 - Single multi-channel track
 *             1 - Multiple simultaneous tracks
 *             2 - One or more sequentially independent
 *                 single track patterns
 * ntracks     The number of tracks in the file.
 * division    This is kind of tricky, it can represent two
 *             things, depending on whether it is positive or negative
 *             (bit 15 set or not).  If  bit  15  of division  is zero,
 *             bits 14 through 0 represent the number of delta-time
 *             "ticks" which make up a quarter note.  If bit  15 of
 *             division  is  a one, delta-times in a file correspond to
 *             subdivisions of a second similar to  SMPTE  and  MIDI
 *             time code.  In  this format bits 14 through 8 contain
 *             one of four values - 24, -25, -29, or -30,
 *             corresponding  to  the  four standard  SMPTE and MIDI
 *             time code frame per second formats, where  -29
 *             represents  30  drop  frame.   The  second  byte
 *             consisting  of  bits 7 through 0 corresponds the the
 *             resolution within a frame.  Refer the Standard MIDI
 *             Files 1.0 spec for more details.
 * fp          This should be the open file pointer to the file you
 *             want to write.  It will have be a global in order
 *             to work with Mf_putc.
 */
void
mfwrite (format, ntracks, division, fp)
     int format, ntracks, division;
     FILE *fp;
{
  int i;
  void mf_write_track_chunk (), mf_write_header_chunk ();

  if (Mf_putc == NULLFUNC)
    mferror ("mfmf_write() called without setting Mf_putc");

  if (Mf_writetrack == NULLFUNC)
    mferror ("mfmf_write() called without setting Mf_mf_writetrack");

  laststatus = 0;

  /* every MIDI file starts with a header */
  mf_write_header_chunk (format, ntracks, division);

  laststatus = 0;

  /* In format 1 files, the first track is a tempo map */
  if (format == 1 && (Mf_writetempotrack))
    {
      (*Mf_writetempotrack) ();
    }

  /* The rest of the file is a series of tracks */
  for (i = 0; i < ntracks; i++)
    mf_write_track_chunk (i, fp);
}

void
mf_write_track_chunk (which_track, fp)
     int which_track;
     FILE *fp;
{
  unsigned long trkhdr, trklength;
  long offset, place_marker;
  void write16bit (), write32bit ();


  laststatus = 0;

  trkhdr = MTrk;
  trklength = 0;

  /* Remember where the length was written, because we don't
	   know how long it will be until we've finished writing */
  offset = ftell (fp);

#ifdef DEBUG
  printf ("offset = %d\n", (int) offset);
#endif

  /* Write the track chunk header */
  write32bit (trkhdr);
  write32bit (trklength);

  Mf_numbyteswritten = 0L;	/* the header's length doesn't count */

  if (Mf_writetrack)
    {
      (*Mf_writetrack) (which_track);
    }

  /* mf_write End of track meta event */
/* but this does not necessarily have a delta of 0, so
 * I don't want to do it -- leave it up to the user of the
 * library functions to do
 *	--gl
	eputc(0);
	eputc(laststatus = meta_event);
	eputc(end_of_track);

 	eputc(0);
 */

  /* It's impossible to know how long the track chunk will be beforehand,
           so the position of the track length data is kept so that it can
           be written after the chunk has been generated */
  place_marker = ftell (fp);

  /* This method turned out not to be portable because the
           parameter returned from ftell is not guaranteed to be
           in bytes on every machine */
  /* track.length = place_marker - offset - (long) sizeof(track); */

#ifdef DEBUG
  printf ("length = %d\n", (int) trklength);
#endif

  if (fseek (fp, offset, 0) < 0)
    mferror ("error seeking during final stage of write");

  trklength = Mf_numbyteswritten;

  /* Re-mf_write the track chunk header with right length */
  write32bit (trkhdr);
  write32bit (trklength);

  fseek (fp, place_marker, 0);
}				/* End gen_track_chunk() */


void
mf_write_header_chunk (format, ntracks, division)
     int format, ntracks, division;
{
  unsigned long ident, length;
  void write16bit (), write32bit ();

  ident = MThd;			/* Head chunk identifier                    */
  length = 6;			/* Chunk length                             */

  /* individual bytes of the header must be written separately
       to preserve byte order across cpu types :-( */
  write32bit (ident);
  write32bit (length);
  write16bit (format);
  write16bit (ntracks);
  write16bit (division);
}				/* end gen_header_chunk() */


/*
 * mf_write_midi_event()
 *
 * Library routine to mf_write a single MIDI track event in the standard MIDI
 * file format. The format is:
 *
 *                    <delta-time><event>
 *
 * In this case, event can be any multi-byte midi message, such as
 * "note on", "note off", etc.
 *
 * delta_time - the time in ticks since the last event.
 * type - the type of meta event.
 * chan - The midi channel.
 * data - A pointer to a block of chars containing the META EVENT,
 *        data.
 * size - The length of the meta-event data.
 */
int
mf_write_midi_event (delta_time, type, chan, data, size)
     unsigned long delta_time;
     int chan, type;
     unsigned long size;
     char *data;
{
  int i;
  unsigned char c;

  WriteVarLen (delta_time);

  /* all MIDI events start with the type in the first four bits,
       and the channel in the lower four bits */
  if (type == system_exclusive || type == 0xf7)
    {
      c = type;
      laststatus = 0;
    }
  else
    c = type | chan;

  if (chan > 15)
    perror ("error: MIDI channel greater than 16\n");

  if (laststatus != c)
    eputc (laststatus = c);

  if (type == system_exclusive || type == 0xf7)
    WriteVarLen (size);

  /* write out the data bytes */
  for (i = 0; i < (int)size; i++)
    eputc (data[i]);

  return (size);
}				/* end mf_write MIDI event */

/*
 * mf_write_meta_event()
 *
 * Library routine to mf_write a single meta event in the standard MIDI
 * file format. The format of a meta event is:
 *
 *          <delta-time><FF><type><length><bytes>
 *
 * delta_time - the time in ticks since the last event.
 * type - the type of meta event.
 * data - A pointer to a block of chars containing the META EVENT,
 *        data.
 * size - The length of the meta-event data.
 */
int 
mf_write_meta_event (delta_time, type, data, size)
     unsigned long delta_time;
     unsigned char *data, type;
     unsigned long size;
{
  int i;

  WriteVarLen (delta_time);

  /* This marks the fact we're writing a meta-event */
  eputc (laststatus = meta_event);

  /* The type of meta event */
  eputc (type);

  /* The length of the data bytes to follow */
  WriteVarLen (size);

  for (i = 0; i < (int)size; i++)
    {
      if (eputc (data[i]) != data[i])
	return (-1);
    }
  return (size);
}				/* end mf_write_meta_event */

void
mf_write_tempo (delta_time, tempo)
     unsigned long delta_time;
     unsigned long tempo;
{
  /* Write tempo */
  /* all tempos are written as 120 beats/minute, */
  /* expressed in microseconds/quarter note     */

  WriteVarLen (delta_time);
  eputc (laststatus = meta_event);
  eputc (set_tempo);

  eputc (3);
  eputc ((unsigned) (0xff & (tempo >> 16)));
  eputc ((unsigned) (0xff & (tempo >> 8)));
  eputc ((unsigned) (0xff & tempo));
}

void
mf_write_seqnum (delta_time, seqnum)
     unsigned long delta_time;
     unsigned seqnum;
{

  WriteVarLen (delta_time);
  eputc (laststatus = meta_event);
  eputc (0);

  eputc ((unsigned) (0xff & (seqnum >> 8)));
  eputc ((unsigned) (0xff & seqnum));
}

unsigned long
mf_sec2ticks (secs, division, tempo)
     int division;
     unsigned long tempo;
     double secs;
{
  return (unsigned long) (((secs * 1000.0) / 4.0 * division) / tempo);
}

/*
 * Write multi-length bytes to MIDI format files
 */
void
WriteVarLen (value)
     unsigned long value;
{
  unsigned long buffer;

  buffer = value & 0x7f;
  while ((value >>= 7) > 0)
    {
      buffer <<= 8;
      buffer |= 0x80;
      buffer += (value & 0x7f);
    }
  while (1)
    {
      eputc ((unsigned) (buffer & 0xff));

      if (buffer & 0x80)
	buffer >>= 8;
      else
	return;
    }
}				/* end of WriteVarLen */

/*
 * This routine converts delta times in ticks into seconds. The
 * else statement is needed because the formula is different for tracks
 * based on notes and tracks based on SMPTE times.
 *
 */
double
mf_ticks2sec (ticks, division, tempo)
     int division;
     unsigned long tempo;
     unsigned long ticks;
{
  double smpte_format, smpte_resolution;

  if (division > 0)
    return ((double) (((double) (ticks) * (double) (tempo)) / ((double) (division) * 1000000.0)));
  else
    {
      smpte_format = upperbyte (division);
      smpte_resolution = lowerbyte (division);
      return (double) ((double) ticks / (smpte_format * smpte_resolution * 1000000.0));
    }
}				/* end of ticks2sec() */


/*
 * write32bit()
 * write16bit()
 *
 * These routines are used to make sure that the byte order of
 * the various data types remains constant between machines. This
 * helps make sure that the code will be portable from one system
 * to the next.  It is slightly dangerous that it assumes that longs
 * have at least 32 bits and ints have at least 16 bits, but this
 * has been true at least on PCs, UNIX machines, and Macintosh's.
 *
 */
void
write32bit (data)
     unsigned long data;
{
  eputc ((unsigned) ((data >> 24) & 0xff));
  eputc ((unsigned) ((data >> 16) & 0xff));
  eputc ((unsigned) ((data >> 8) & 0xff));
  eputc ((unsigned) (data & 0xff));
}

void
write16bit (data)
     int data;
{
  eputc ((unsigned) ((data & 0xff00) >> 8));
  eputc ((unsigned) (data & 0xff));
}

/* write a single character and abort on error */
static int
eputc (c)
     unsigned char c;
{
  int return_val;

  if ((Mf_putc) == NULLFUNC)
    {
      mferror ("Mf_putc undefined");
      return (-1);
    }

  return_val = (*Mf_putc) (c);

  if (return_val == EOF)
    mferror ("error writing");

  Mf_numbyteswritten++;
  return (return_val);
}
