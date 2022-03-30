// SPDX-License-Identifier: GPL-2.0+
/*
 * taken from gdb/remote.c
 *
 * I am only interested in the write to memory stuff - everything else
 * has been ripped out
 *
 * all the copyright notices etc have been left in
 */

/* enough so that it will compile */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/*nicked from gcc..*/

#ifndef alloca
#ifdef __GNUC__
#define alloca __builtin_alloca
#else /* not GNU C.  */
#if (!defined (__STDC__) && defined (sparc)) || defined (__sparc__) || defined (__sparc) || defined (__sgi)
#include <alloca.h>
#else /* not sparc */
#if defined (MSDOS) && !defined (__TURBOC__)
#include <malloc.h>
#else /* not MSDOS, or __TURBOC__ */
#if defined(_AIX)
#include <malloc.h>
 #pragma alloca
#else /* not MSDOS, __TURBOC__, or _AIX */
#ifdef __hpux
#endif /* __hpux */
#endif /* not _AIX */
#endif /* not MSDOS, or __TURBOC__ */
#endif /* not sparc.  */
#endif /* not GNU C.  */
#ifdef __cplusplus
extern "C" {
#endif
    void* alloca(size_t);
#ifdef __cplusplus
}
#endif
#endif /* alloca not defined.  */


#include "serial.h"
#include "error.h"
#include "remote.h"
#define REGISTER_BYTES 0
#define fprintf_unfiltered fprintf
#define fprintf_filtered fprintf
#define fputs_unfiltered fputs
#define fputs_filtered fputs
#define fputc_unfiltered fputc
#define fputc_filtered fputc
#define printf_unfiltered printf
#define printf_filtered printf
#define puts_unfiltered puts
#define puts_filtered puts
#define putchar_unfiltered putchar
#define putchar_filtered putchar
#define fputstr_unfiltered(a,b,c) fputs((a), (c))
#define gdb_stdlog stderr
#define SERIAL_READCHAR(fd,timo)	serialreadchar((fd), (timo))
#define SERIAL_WRITE(fd, addr, len)	serialwrite((fd), (addr), (len))
#define error Error
#define perror_with_name Perror
#define gdb_flush fflush
#define max(a,b) (((a)>(b))?(a):(b))
#define min(a,b) (((a)<(b))?(a):(b))
#define target_mourn_inferior() {}
#define ULONGEST unsigned long
#define CORE_ADDR unsigned long

static int putpkt (char *);
static int putpkt_binary(char *, int);
static void getpkt (char *, int);

static int remote_debug = 0, remote_register_buf_size = 0, watchdog = 0;

int remote_desc = -1, remote_timeout = 10;

static void
fputstrn_unfiltered(char *s, int n, int x, FILE *fp)
{
    while (n-- > 0)
	fputc(*s++, fp);
}

void
remote_reset(void)
{
    SERIAL_WRITE(remote_desc, "+", 1);
}

void
remote_continue(void)
{
    putpkt("c");
}

/* Remote target communications for serial-line targets in custom GDB protocol
   Copyright 1988, 91, 92, 93, 94, 95, 96, 97, 98, 1999
   Free Software Foundation, Inc.

   This file is part of GDB.
 */
/* *INDENT-OFF* */
/* Remote communication protocol.

   A debug packet whose contents are <data>
   is encapsulated for transmission in the form:

	$ <data> # CSUM1 CSUM2

	<data> must be ASCII alphanumeric and cannot include characters
	'$' or '#'.  If <data> starts with two characters followed by
	':', then the existing stubs interpret this as a sequence number.

	CSUM1 and CSUM2 are ascii hex representation of an 8-bit
	checksum of <data>, the most significant nibble is sent first.
	the hex digits 0-9,a-f are used.

   Receiver responds with:

	+	- if CSUM is correct and ready for next packet
	-	- if CSUM is incorrect

   <data> is as follows:
   Most values are encoded in ascii hex digits.  Signal numbers are according
   to the numbering in target.h.

	Request		Packet

	set thread	Hct...		Set thread for subsequent operations.
					c = 'c' for thread used in step and
					continue; t... can be -1 for all
					threads.
					c = 'g' for thread used in other
					operations.  If zero, pick a thread,
					any thread.
	reply		OK		for success
			ENN		for an error.

	read registers  g
	reply		XX....X		Each byte of register data
					is described by two hex digits.
					Registers are in the internal order
					for GDB, and the bytes in a register
					are in the same order the machine uses.
			or ENN		for an error.

	write regs	GXX..XX		Each byte of register data
					is described by two hex digits.
	reply		OK		for success
			ENN		for an error

	write reg	Pn...=r...	Write register n... with value r...,
					which contains two hex digits for each
					byte in the register (target byte
					order).
	reply		OK		for success
			ENN		for an error
	(not supported by all stubs).

	read mem	mAA..AA,LLLL	AA..AA is address, LLLL is length.
	reply		XX..XX		XX..XX is mem contents
					Can be fewer bytes than requested
					if able to read only part of the data.
			or ENN		NN is errno

	write mem	MAA..AA,LLLL:XX..XX
					AA..AA is address,
					LLLL is number of bytes,
					XX..XX is data
	reply		OK		for success
			ENN		for an error (this includes the case
					where only part of the data was
					written).

	write mem       XAA..AA,LLLL:XX..XX
	 (binary)                       AA..AA is address,
					LLLL is number of bytes,
					XX..XX is binary data
	reply           OK              for success
			ENN             for an error

	continue	cAA..AA		AA..AA is address to resume
					If AA..AA is omitted,
					resume at same address.

	step		sAA..AA		AA..AA is address to resume
					If AA..AA is omitted,
					resume at same address.

	continue with	Csig;AA..AA	Continue with signal sig (hex signal
	signal				number).  If ;AA..AA is omitted,
					resume at same address.

	step with	Ssig;AA..AA	Like 'C' but step not continue.
	signal

	last signal     ?               Reply the current reason for stopping.
					This is the same reply as is generated
					for step or cont : SAA where AA is the
					signal number.

	detach          D               Reply OK.

	There is no immediate reply to step or cont.
	The reply comes when the machine stops.
	It is		SAA		AA is the signal number.

	or...		TAAn...:r...;n...:r...;n...:r...;
					AA = signal number
					n... = register number (hex)
					  r... = register contents
					n... = `thread'
					  r... = thread process ID.  This is
						 a hex integer.
					n... = other string not starting
					    with valid hex digit.
					  gdb should ignore this n,r pair
					  and go on to the next.  This way
					  we can extend the protocol.
	or...		WAA		The process exited, and AA is
					the exit status.  This is only
					applicable for certains sorts of
					targets.
	or...		XAA		The process terminated with signal
					AA.
	or (obsolete)	NAA;tttttttt;dddddddd;bbbbbbbb
					AA = signal number
					tttttttt = address of symbol "_start"
					dddddddd = base of data section
					bbbbbbbb = base of bss  section.
					Note: only used by Cisco Systems
					targets.  The difference between this
					reply and the "qOffsets" query is that
					the 'N' packet may arrive spontaneously
					whereas the 'qOffsets' is a query
					initiated by the host debugger.
	or...           OXX..XX	XX..XX  is hex encoding of ASCII data. This
					can happen at any time while the
					program is running and the debugger
					should continue to wait for
					'W', 'T', etc.

	thread alive	TXX		Find out if the thread XX is alive.
	reply		OK		thread is still alive
			ENN		thread is dead

	remote restart	RXX		Restart the remote server

	extended ops	!		Use the extended remote protocol.
					Sticky -- only needs to be set once.

	kill request	k

	toggle debug	d		toggle debug flag (see 386 & 68k stubs)
	reset		r		reset -- see sparc stub.
	reserved	<other>		On other requests, the stub should
					ignore the request and send an empty
					response ($#<checksum>).  This way
					we can extend the protocol and GDB
					can tell whether the stub it is
					talking to uses the old or the new.
	search		tAA:PP,MM	Search backwards starting at address
					AA for a match with pattern PP and
					mask MM.  PP and MM are 4 bytes.
					Not supported by all stubs.

	general query	qXXXX		Request info about XXXX.
	general set	QXXXX=yyyy	Set value of XXXX to yyyy.
	query sect offs	qOffsets	Get section offsets.  Reply is
					Text=xxx;Data=yyy;Bss=zzz

	Responses can be run-length encoded to save space.  A '*' means that
	the next character is an ASCII encoding giving a repeat count which
	stands for that many repititions of the character preceding the '*'.
	The encoding is n+29, yielding a printable character where n >=3
	(which is where rle starts to win).  Don't use an n > 126.

	So
	"0* " means the same as "0000".  */
/* *INDENT-ON* */

/* This variable (available to the user via "set remotebinarydownload")
   dictates whether downloads are sent in binary (via the 'X' packet).
   We assume that the stub can, and attempt to do it. This will be cleared if
   the stub does not understand it. This switch is still needed, though
   in cases when the packet is supported in the stub, but the connection
   does not allow it (i.e., 7-bit serial connection only). */
static int remote_binary_download = 1;

/* Have we already checked whether binary downloads work? */
static int remote_binary_checked;

/* Maximum number of bytes to read/write at once.  The value here
   is chosen to fill up a packet (the headers account for the 32).  */
#define MAXBUFBYTES(N) (((N)-32)/2)

/* Having this larger than 400 causes us to be incompatible with m68k-stub.c
   and i386-stub.c.  Normally, no one would notice because it only matters
   for writing large chunks of memory (e.g. in downloads).  Also, this needs
   to be more than 400 if required to hold the registers (see below, where
   we round it up based on REGISTER_BYTES).  */
/* Round up PBUFSIZ to hold all the registers, at least.  */
#define	PBUFSIZ ((REGISTER_BYTES > MAXBUFBYTES (400)) \
		 ? (REGISTER_BYTES * 2 + 32) \
		 : 400)


/* This variable sets the number of bytes to be written to the target
   in a single packet.  Normally PBUFSIZ is satisfactory, but some
   targets need smaller values (perhaps because the receiving end
   is slow).  */

static int remote_write_size = 0x7fffffff;

/* This variable sets the number of bits in an address that are to be
   sent in a memory ("M" or "m") packet.  Normally, after stripping
   leading zeros, the entire address would be sent. This variable
   restricts the address to REMOTE_ADDRESS_SIZE bits.  HISTORY: The
   initial implementation of remote.c restricted the address sent in
   memory packets to ``host::sizeof long'' bytes - (typically 32
   bits).  Consequently, for 64 bit targets, the upper 32 bits of an
   address was never sent.  Since fixing this bug may cause a break in
   some remote targets this variable is principly provided to
   facilitate backward compatibility. */

static int remote_address_size;

/* Convert hex digit A to a number.  */

static int
fromhex (int a)
{
  if (a >= '0' && a <= '9')
    return a - '0';
  else if (a >= 'a' && a <= 'f')
    return a - 'a' + 10;
  else if (a >= 'A' && a <= 'F')
    return a - 'A' + 10;
  else {
    error ("Reply contains invalid hex digit %d", a);
    return -1;
  }
}

/* Convert number NIB to a hex digit.  */

static int
tohex (int nib)
{
  if (nib < 10)
    return '0' + nib;
  else
    return 'a' + nib - 10;
}

/* Return the number of hex digits in num.  */

static int
hexnumlen (ULONGEST num)
{
  int i;

  for (i = 0; num != 0; i++)
    num >>= 4;

  return max (i, 1);
}

/* Set BUF to the hex digits representing NUM.  */

static int
hexnumstr (char *buf, ULONGEST num)
{
  int i;
  int len = hexnumlen (num);

  buf[len] = '\0';

  for (i = len - 1; i >= 0; i--)
    {
      buf[i] = "0123456789abcdef"[(num & 0xf)];
      num >>= 4;
    }

  return len;
}

/* Mask all but the least significant REMOTE_ADDRESS_SIZE bits. */

static CORE_ADDR
remote_address_masked (CORE_ADDR addr)
{
  if (remote_address_size > 0
      && remote_address_size < (sizeof (ULONGEST) * 8))
    {
      /* Only create a mask when that mask can safely be constructed
	 in a ULONGEST variable. */
      ULONGEST mask = 1;
      mask = (mask << remote_address_size) - 1;
      addr &= mask;
    }
  return addr;
}

/* Determine whether the remote target supports binary downloading.
   This is accomplished by sending a no-op memory write of zero length
   to the target at the specified address. It does not suffice to send
   the whole packet, since many stubs strip the eighth bit and subsequently
   compute a wrong checksum, which causes real havoc with remote_write_bytes.

   NOTE: This can still lose if the serial line is not eight-bit clean. In
   cases like this, the user should clear "remotebinarydownload". */
static void
check_binary_download (CORE_ADDR addr)
{
  if (remote_binary_download && !remote_binary_checked)
    {
      char *buf = alloca (PBUFSIZ);
      char *p;
      remote_binary_checked = 1;

      p = buf;
      *p++ = 'X';
      p += hexnumstr (p, (ULONGEST) addr);
      *p++ = ',';
      p += hexnumstr (p, (ULONGEST) 0);
      *p++ = ':';
      *p = '\0';

      putpkt_binary (buf, (int) (p - buf));
      getpkt (buf, 0);

      if (buf[0] == '\0')
	remote_binary_download = 0;
    }

  if (remote_debug)
    {
      if (remote_binary_download)
	fprintf_unfiltered (gdb_stdlog,
			    "binary downloading suppported by target\n");
      else
	fprintf_unfiltered (gdb_stdlog,
			    "binary downloading NOT suppported by target\n");
    }
}

/* Write memory data directly to the remote machine.
   This does not inform the data cache; the data cache uses this.
   MEMADDR is the address in the remote memory space.
   MYADDR is the address of the buffer in our space.
   LEN is the number of bytes.

   Returns number of bytes transferred, or 0 for error.  */

int
remote_write_bytes (memaddr, myaddr, len)
     CORE_ADDR memaddr;
     char *myaddr;
     int len;
{
  unsigned char *buf = alloca (PBUFSIZ);
  int max_buf_size;		/* Max size of packet output buffer */
  int origlen;
  extern int verbose;

  /* Verify that the target can support a binary download */
  check_binary_download (memaddr);

  /* Chop the transfer down if necessary */

  max_buf_size = min (remote_write_size, PBUFSIZ);
  if (remote_register_buf_size != 0)
    max_buf_size = min (max_buf_size, remote_register_buf_size);

  /* Subtract header overhead from max payload size -  $M<memaddr>,<len>:#nn */
  max_buf_size -= 2 + hexnumlen (memaddr + len - 1) + 1 + hexnumlen (len) + 4;

  origlen = len;
  while (len > 0)
    {
      unsigned char *p, *plen;
      int todo;
      int i;

      /* construct "M"<memaddr>","<len>":" */
      /* sprintf (buf, "M%lx,%x:", (unsigned long) memaddr, todo); */
      memaddr = remote_address_masked (memaddr);
      p = buf;
      if (remote_binary_download)
	{
	  *p++ = 'X';
	  todo = min (len, max_buf_size);
	}
      else
	{
	  *p++ = 'M';
	  todo = min (len, max_buf_size / 2);	/* num bytes that will fit */
	}

      p += hexnumstr ((char *)p, (ULONGEST) memaddr);
      *p++ = ',';

      plen = p;			/* remember where len field goes */
      p += hexnumstr ((char *)p, (ULONGEST) todo);
      *p++ = ':';
      *p = '\0';

      /* We send target system values byte by byte, in increasing byte
	 addresses, each byte encoded as two hex characters (or one
	 binary character).  */
      if (remote_binary_download)
	{
	  int escaped = 0;
	  for (i = 0;
	       (i < todo) && (i + escaped) < (max_buf_size - 2);
	       i++)
	    {
	      switch (myaddr[i] & 0xff)
		{
		case '$':
		case '#':
		case 0x7d:
		  /* These must be escaped */
		  escaped++;
		  *p++ = 0x7d;
		  *p++ = (myaddr[i] & 0xff) ^ 0x20;
		  break;
		default:
		  *p++ = myaddr[i] & 0xff;
		  break;
		}
	    }

	  if (i < todo)
	    {
	      /* Escape chars have filled up the buffer prematurely,
		 and we have actually sent fewer bytes than planned.
		 Fix-up the length field of the packet.  */

	      /* FIXME: will fail if new len is a shorter string than
		 old len.  */

	      plen += hexnumstr ((char *)plen, (ULONGEST) i);
	      *plen++ = ':';
	    }
	}
      else
	{
	  for (i = 0; i < todo; i++)
	    {
	      *p++ = tohex ((myaddr[i] >> 4) & 0xf);
	      *p++ = tohex (myaddr[i] & 0xf);
	    }
	  *p = '\0';
	}

      putpkt_binary ((char *)buf, (int) (p - buf));
      getpkt ((char *)buf, 0);

      if (buf[0] == 'E')
	{
	  /* There is no correspondance between what the remote protocol uses
	     for errors and errno codes.  We would like a cleaner way of
	     representing errors (big enough to include errno codes, bfd_error
	     codes, and others).  But for now just return EIO.  */
	  errno = EIO;
	  return 0;
	}

      /* Increment by i, not by todo, in case escape chars
	 caused us to send fewer bytes than we'd planned.  */
      myaddr += i;
      memaddr += i;
      len -= i;

      if (verbose)
	putc('.', stderr);
    }
  return origlen;
}

/* Stuff for dealing with the packets which are part of this protocol.
   See comment at top of file for details.  */

/* Read a single character from the remote end, masking it down to 7 bits. */

static int
readchar (int timeout)
{
  int ch;

  ch = SERIAL_READCHAR (remote_desc, timeout);

  switch (ch)
    {
    case SERIAL_EOF:
      error ("Remote connection closed");
    case SERIAL_ERROR:
      perror_with_name ("Remote communication error");
    case SERIAL_TIMEOUT:
      return ch;
    default:
      return ch & 0x7f;
    }
}

static int
putpkt (buf)
     char *buf;
{
  return putpkt_binary (buf, strlen (buf));
}

/* Send a packet to the remote machine, with error checking.  The data
   of the packet is in BUF.  The string in BUF can be at most  PBUFSIZ - 5
   to account for the $, # and checksum, and for a possible /0 if we are
   debugging (remote_debug) and want to print the sent packet as a string */

static int
putpkt_binary (buf, cnt)
     char *buf;
     int cnt;
{
  int i;
  unsigned char csum = 0;
  char *buf2 = alloca (PBUFSIZ);
  char *junkbuf = alloca (PBUFSIZ);

  int ch;
  int tcount = 0;
  char *p;

  /* Copy the packet into buffer BUF2, encapsulating it
     and giving it a checksum.  */

  if (cnt > BUFSIZ - 5)		/* Prosanity check */
    abort ();

  p = buf2;
  *p++ = '$';

  for (i = 0; i < cnt; i++)
    {
      csum += buf[i];
      *p++ = buf[i];
    }
  *p++ = '#';
  *p++ = tohex ((csum >> 4) & 0xf);
  *p++ = tohex (csum & 0xf);

  /* Send it over and over until we get a positive ack.  */

  while (1)
    {
      int started_error_output = 0;

      if (remote_debug)
	{
	  *p = '\0';
	  fprintf_unfiltered (gdb_stdlog, "Sending packet: ");
	  fputstrn_unfiltered (buf2, p - buf2, 0, gdb_stdlog);
	  fprintf_unfiltered (gdb_stdlog, "...");
	  gdb_flush (gdb_stdlog);
	}
      if (SERIAL_WRITE (remote_desc, buf2, p - buf2))
	perror_with_name ("putpkt: write failed");

      /* read until either a timeout occurs (-2) or '+' is read */
      while (1)
	{
	  ch = readchar (remote_timeout);

	  if (remote_debug)
	    {
	      switch (ch)
		{
		case '+':
		case SERIAL_TIMEOUT:
		case '$':
		  if (started_error_output)
		    {
		      putchar_unfiltered ('\n');
		      started_error_output = 0;
		    }
		}
	    }

	  switch (ch)
	    {
	    case '+':
	      if (remote_debug)
		fprintf_unfiltered (gdb_stdlog, "Ack\n");
	      return 1;
	    case SERIAL_TIMEOUT:
	      tcount++;
	      if (tcount > 3)
		return 0;
	      break;		/* Retransmit buffer */
	    case '$':
	      {
		/* It's probably an old response, and we're out of sync.
		   Just gobble up the packet and ignore it.  */
		getpkt (junkbuf, 0);
		continue;	/* Now, go look for + */
	      }
	    default:
	      if (remote_debug)
		{
		  if (!started_error_output)
		    {
		      started_error_output = 1;
		      fprintf_unfiltered (gdb_stdlog, "putpkt: Junk: ");
		    }
		  fputc_unfiltered (ch & 0177, gdb_stdlog);
		}
	      continue;
	    }
	  break;		/* Here to retransmit */
	}

#if 0
      /* This is wrong.  If doing a long backtrace, the user should be
	 able to get out next time we call QUIT, without anything as
	 violent as interrupt_query.  If we want to provide a way out of
	 here without getting to the next QUIT, it should be based on
	 hitting ^C twice as in remote_wait.  */
      if (quit_flag)
	{
	  quit_flag = 0;
	  interrupt_query ();
	}
#endif
    }
}

/* Come here after finding the start of the frame.  Collect the rest
   into BUF, verifying the checksum, length, and handling run-length
   compression.  Returns 0 on any error, 1 on success.  */

static int
read_frame (char *buf)
{
  unsigned char csum;
  char *bp;
  int c;

  csum = 0;
  bp = buf;

  while (1)
    {
      c = readchar (remote_timeout);

      switch (c)
	{
	case SERIAL_TIMEOUT:
	  if (remote_debug)
	    fputs_filtered ("Timeout in mid-packet, retrying\n", gdb_stdlog);
	  return 0;
	case '$':
	  if (remote_debug)
	    fputs_filtered ("Saw new packet start in middle of old one\n",
			    gdb_stdlog);
	  return 0;		/* Start a new packet, count retries */
	case '#':
	  {
	    unsigned char pktcsum;

	    *bp = '\000';

	    pktcsum = fromhex (readchar (remote_timeout)) << 4;
	    pktcsum |= fromhex (readchar (remote_timeout));

	    if (csum == pktcsum)
	      {
		return 1;
	      }

	    if (remote_debug)
	      {
		fprintf_filtered (gdb_stdlog,
			      "Bad checksum, sentsum=0x%x, csum=0x%x, buf=",
				  pktcsum, csum);
		fputs_filtered (buf, gdb_stdlog);
		fputs_filtered ("\n", gdb_stdlog);
	      }
	    return 0;
	  }
	case '*':		/* Run length encoding */
	  csum += c;
	  c = readchar (remote_timeout);
	  csum += c;
	  c = c - ' ' + 3;	/* Compute repeat count */

	  if (c > 0 && c < 255 && bp + c - 1 < buf + PBUFSIZ - 1)
	    {
	      memset (bp, *(bp - 1), c);
	      bp += c;
	      continue;
	    }

	  *bp = '\0';
	  printf_filtered ("Repeat count %d too large for buffer: ", c);
	  puts_filtered (buf);
	  puts_filtered ("\n");
	  return 0;
	default:
	  if (bp < buf + PBUFSIZ - 1)
	    {
	      *bp++ = c;
	      csum += c;
	      continue;
	    }

	  *bp = '\0';
	  puts_filtered ("Remote packet too long: ");
	  puts_filtered (buf);
	  puts_filtered ("\n");

	  return 0;
	}
    }
}

/* Read a packet from the remote machine, with error checking, and
   store it in BUF.  BUF is expected to be of size PBUFSIZ.  If
   FOREVER, wait forever rather than timing out; this is used while
   the target is executing user code.  */

static void
getpkt (buf, forever)
     char *buf;
     int forever;
{
  int c;
  int tries;
  int timeout;
  int val;

  strcpy (buf, "timeout");

  if (forever)
    {
      timeout = watchdog > 0 ? watchdog : -1;
    }

  else
    timeout = remote_timeout;

#define MAX_TRIES 3

  for (tries = 1; tries <= MAX_TRIES; tries++)
    {
      /* This can loop forever if the remote side sends us characters
	 continuously, but if it pauses, we'll get a zero from readchar
	 because of timeout.  Then we'll count that as a retry.  */

      /* Note that we will only wait forever prior to the start of a packet.
	 After that, we expect characters to arrive at a brisk pace.  They
	 should show up within remote_timeout intervals.  */

      do
	{
	  c = readchar (timeout);

	  if (c == SERIAL_TIMEOUT)
	    {
	      if (forever)	/* Watchdog went off.  Kill the target. */
		{
		  target_mourn_inferior ();
		  error ("Watchdog has expired.  Target detached.\n");
		}
	      if (remote_debug)
		fputs_filtered ("Timed out.\n", gdb_stdlog);
	      goto retry;
	    }
	}
      while (c != '$');

      /* We've found the start of a packet, now collect the data.  */

      val = read_frame (buf);

      if (val == 1)
	{
	  if (remote_debug)
	    {
	      fprintf_unfiltered (gdb_stdlog, "Packet received: ");
	      fputstr_unfiltered (buf, 0, gdb_stdlog);
	      fprintf_unfiltered (gdb_stdlog, "\n");
	    }
	  SERIAL_WRITE (remote_desc, "+", 1);
	  return;
	}

      /* Try the whole thing again.  */
    retry:
      SERIAL_WRITE (remote_desc, "-", 1);
    }

  /* We have tried hard enough, and just can't receive the packet.  Give up. */

  printf_unfiltered ("Ignoring packet error, continuing...\n");
  SERIAL_WRITE (remote_desc, "+", 1);
}
