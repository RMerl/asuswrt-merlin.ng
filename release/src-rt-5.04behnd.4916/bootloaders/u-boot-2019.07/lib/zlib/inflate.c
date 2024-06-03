/* inflate.c -- zlib decompression
 * Copyright (C) 1995-2005 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */
local void fixedtables OF((struct inflate_state FAR *state));
local int updatewindow OF((z_streamp strm, unsigned out));

int ZEXPORT inflateReset(z_streamp strm)
{
    struct inflate_state FAR *state;

    if (strm == Z_NULL || strm->state == Z_NULL) return Z_STREAM_ERROR;
    state = (struct inflate_state FAR *)strm->state;
    strm->total_in = strm->total_out = state->total = 0;
    strm->msg = Z_NULL;
    strm->adler = 1;        /* to support ill-conceived Java test suite */
    state->mode = HEAD;
    state->last = 0;
    state->havedict = 0;
    state->dmax = 32768U;
    state->head = Z_NULL;
    state->wsize = 0;
    state->whave = 0;
    state->write = 0;
    state->hold = 0;
    state->bits = 0;
    state->lencode = state->distcode = state->next = state->codes;
    WATCHDOG_RESET();
    Tracev((stderr, "inflate: reset\n"));
    return Z_OK;
}

int ZEXPORT inflateInit2_(z_streamp strm, int windowBits, const char *version,
			  int stream_size)
{
    struct inflate_state FAR *state;

    if (version == Z_NULL || version[0] != ZLIB_VERSION[0] ||
        stream_size != (int)(sizeof(z_stream)))
        return Z_VERSION_ERROR;
    if (strm == Z_NULL) return Z_STREAM_ERROR;
    strm->msg = Z_NULL;                 /* in case we return an error */
    if (strm->zalloc == (alloc_func)0) {
        strm->zalloc = zcalloc;
        strm->opaque = (voidpf)0;
    }
    if (strm->zfree == (free_func)0) strm->zfree = zcfree;
    state = (struct inflate_state FAR *)
            ZALLOC(strm, 1, sizeof(struct inflate_state));
    if (state == Z_NULL) return Z_MEM_ERROR;
    Tracev((stderr, "inflate: allocated\n"));
    strm->state = (struct internal_state FAR *)state;
    if (windowBits < 0) {
        state->wrap = 0;
        windowBits = -windowBits;
    }
    else {
        state->wrap = (windowBits >> 4) + 1;
#ifdef GUNZIP
        if (windowBits < 48) windowBits &= 15;
#endif
    }
    if (windowBits < 8 || windowBits > 15) {
        ZFREE(strm, state);
        strm->state = Z_NULL;
        return Z_STREAM_ERROR;
    }
    state->wbits = (unsigned)windowBits;
    state->window = Z_NULL;
    return inflateReset(strm);
}

int ZEXPORT inflateInit_(z_streamp strm, const char *version, int stream_size)
{
    return inflateInit2_(strm, DEF_WBITS, version, stream_size);
}

local void fixedtables(struct inflate_state FAR *state)
{
    state->lencode = lenfix;
    state->lenbits = 9;
    state->distcode = distfix;
    state->distbits = 5;
}

/*
   Update the window with the last wsize (normally 32K) bytes written before
   returning.  If window does not exist yet, create it.  This is only called
   when a window is already in use, or when output has been written during this
   inflate call, but the end of the deflate stream has not been reached yet.
   It is also called to create a window for dictionary data when a dictionary
   is loaded.

   Providing output buffers larger than 32K to inflate() should provide a speed
   advantage, since only the last 32K of output is copied to the sliding window
   upon return from inflate(), and since all distances after the first 32K of
   output will fall in the output data, making match copies simpler and faster.
   The advantage may be dependent on the size of the processor's data caches.
 */
local int updatewindow(z_streamp strm, unsigned out)
{
    struct inflate_state FAR *state;
    unsigned copy, dist;

    state = (struct inflate_state FAR *)strm->state;

    /* if it hasn't been done already, allocate space for the window */
    if (state->window == Z_NULL) {
        state->window = (unsigned char FAR *)
                        ZALLOC(strm, 1U << state->wbits,
                               sizeof(unsigned char));
        if (state->window == Z_NULL) return 1;
    }

    /* if window not in use yet, initialize */
    if (state->wsize == 0) {
        state->wsize = 1U << state->wbits;
        state->write = 0;
        state->whave = 0;
    }

    /* copy state->wsize or less output bytes into the circular window */
    copy = out - strm->avail_out;
    if (copy >= state->wsize) {
        zmemcpy(state->window, strm->next_out - state->wsize, state->wsize);
        state->write = 0;
        state->whave = state->wsize;
    }
    else {
        dist = state->wsize - state->write;
        if (dist > copy) dist = copy;
        zmemcpy(state->window + state->write, strm->next_out - copy, dist);
        copy -= dist;
        if (copy) {
            zmemcpy(state->window, strm->next_out - copy, copy);
            state->write = copy;
            state->whave = state->wsize;
        }
        else {
            state->write += dist;
            if (state->write == state->wsize) state->write = 0;
            if (state->whave < state->wsize) state->whave += dist;
        }
    }
    return 0;
}

/* Macros for inflate(): */

/* check function to use adler32() for zlib or crc32() for gzip */
#ifdef GUNZIP
#  define UPDATE(check, buf, len) \
    (state->flags ? crc32(check, buf, len) : adler32(check, buf, len))
#else
#  define UPDATE(check, buf, len) adler32(check, buf, len)
#endif

/* check macros for header crc */
#ifdef GUNZIP
#  define CRC2(check, word) \
    do { \
        hbuf[0] = (unsigned char)(word); \
        hbuf[1] = (unsigned char)((word) >> 8); \
        check = crc32(check, hbuf, 2); \
    } while (0)

#  define CRC4(check, word) \
    do { \
        hbuf[0] = (unsigned char)(word); \
        hbuf[1] = (unsigned char)((word) >> 8); \
        hbuf[2] = (unsigned char)((word) >> 16); \
        hbuf[3] = (unsigned char)((word) >> 24); \
        check = crc32(check, hbuf, 4); \
    } while (0)
#endif

/* Load registers with state in inflate() for speed */
#define LOAD() \
    do { \
        put = strm->next_out; \
        left = strm->avail_out; \
        next = strm->next_in; \
        have = strm->avail_in; \
        hold = state->hold; \
        bits = state->bits; \
    } while (0)

/* Restore state from registers in inflate() */
#define RESTORE() \
    do { \
        strm->next_out = put; \
        strm->avail_out = left; \
        strm->next_in = next; \
        strm->avail_in = have; \
        state->hold = hold; \
        state->bits = bits; \
    } while (0)

/* Clear the input bit accumulator */
#define INITBITS() \
    do { \
        hold = 0; \
        bits = 0; \
    } while (0)

/* Get a byte of input into the bit accumulator, or return from inflate()
   if there is no input available. */
#define PULLBYTE() \
    do { \
        if (have == 0) goto inf_leave; \
        have--; \
        hold += (unsigned long)(*next++) << bits; \
        bits += 8; \
    } while (0)

/* Assure that there are at least n bits in the bit accumulator.  If there is
   not enough available input to do that, then return from inflate(). */
#define NEEDBITS(n) \
    do { \
        while (bits < (unsigned)(n)) \
            PULLBYTE(); \
    } while (0)

/* Return the low n bits of the bit accumulator (n < 16) */
#define BITS(n) \
    ((unsigned)hold & ((1U << (n)) - 1))

/* Remove n bits from the bit accumulator */
#define DROPBITS(n) \
    do { \
        hold >>= (n); \
        bits -= (unsigned)(n); \
    } while (0)

/* Remove zero to seven bits as needed to go to a byte boundary */
#define BYTEBITS() \
    do { \
        hold >>= bits & 7; \
        bits -= bits & 7; \
    } while (0)

/* Reverse the bytes in a 32-bit value */
#define REVERSE(q) \
    ((((q) >> 24) & 0xff) + (((q) >> 8) & 0xff00) + \
     (((q) & 0xff00) << 8) + (((q) & 0xff) << 24))

/*
   inflate() uses a state machine to process as much input data and generate as
   much output data as possible before returning.  The state machine is
   structured roughly as follows:

    for (;;) switch (state) {
    ...
    case STATEn:
        if (not enough input data or output space to make progress)
            return;
        ... make progress ...
        state = STATEm;
        break;
    ...
    }

   so when inflate() is called again, the same case is attempted again, and
   if the appropriate resources are provided, the machine proceeds to the
   next state.  The NEEDBITS() macro is usually the way the state evaluates
   whether it can proceed or should return.  NEEDBITS() does the return if
   the requested bits are not available.  The typical use of the BITS macros
   is:

        NEEDBITS(n);
        ... do something with BITS(n) ...
        DROPBITS(n);

   where NEEDBITS(n) either returns from inflate() if there isn't enough
   input left to load n bits into the accumulator, or it continues.  BITS(n)
   gives the low n bits in the accumulator.  When done, DROPBITS(n) drops
   the low n bits off the accumulator.  INITBITS() clears the accumulator
   and sets the number of available bits to zero.  BYTEBITS() discards just
   enough bits to put the accumulator on a byte boundary.  After BYTEBITS()
   and a NEEDBITS(8), then BITS(8) would return the next byte in the stream.

   NEEDBITS(n) uses PULLBYTE() to get an available byte of input, or to return
   if there is no input available.  The decoding of variable length codes uses
   PULLBYTE() directly in order to pull just enough bytes to decode the next
   code, and no more.

   Some states loop until they get enough input, making sure that enough
   state information is maintained to continue the loop where it left off
   if NEEDBITS() returns in the loop.  For example, want, need, and keep
   would all have to actually be part of the saved state in case NEEDBITS()
   returns:

    case STATEw:
        while (want < need) {
            NEEDBITS(n);
            keep[want++] = BITS(n);
            DROPBITS(n);
        }
        state = STATEx;
    case STATEx:

   As shown above, if the next state is also the next case, then the break
   is omitted.

   A state may also return if there is not enough output space available to
   complete that state.  Those states are copying stored data, writing a
   literal byte, and copying a matching string.

   When returning, a "goto inf_leave" is used to update the total counters,
   update the check value, and determine whether any progress has been made
   during that inflate() call in order to return the proper return code.
   Progress is defined as a change in either strm->avail_in or strm->avail_out.
   When there is a window, goto inf_leave will update the window with the last
   output written.  If a goto inf_leave occurs in the middle of decompression
   and there is no window currently, goto inf_leave will create one and copy
   output to the window for the next call of inflate().

   In this implementation, the flush parameter of inflate() only affects the
   return code (per zlib.h).  inflate() always writes as much as possible to
   strm->next_out, given the space available and the provided input--the effect
   documented in zlib.h of Z_SYNC_FLUSH.  Furthermore, inflate() always defers
   the allocation of and copying into a sliding window until necessary, which
   provides the effect documented in zlib.h for Z_FINISH when the entire input
   stream available.  So the only thing the flush parameter actually does is:
   when flush is set to Z_FINISH, inflate() cannot return Z_OK.  Instead it
   will return Z_BUF_ERROR if it has not reached the end of the stream.
 */
int ZEXPORT inflate(z_streamp strm, int flush)
{
    struct inflate_state FAR *state;
    unsigned char FAR *next;    /* next input */
    unsigned char FAR *put;     /* next output */
    unsigned have, left;        /* available input and output */
    unsigned long hold;         /* bit buffer */
    unsigned bits;              /* bits in bit buffer */
    unsigned in, out;           /* save starting available input and output */
    unsigned copy;              /* number of stored or match bytes to copy */
    unsigned char FAR *from;    /* where to copy match bytes from */
    code this;                  /* current decoding table entry */
    code last;                  /* parent table entry */
    unsigned len;               /* length to copy for repeats, bits to drop */
    int ret;                    /* return code */
#ifdef GUNZIP
    unsigned char hbuf[4];      /* buffer for gzip header crc calculation */
#endif
    static const unsigned short order[19] = /* permutation of code lengths */
        {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};

    if (strm == Z_NULL || strm->state == Z_NULL ||
        (strm->next_in == Z_NULL && strm->avail_in != 0))
        return Z_STREAM_ERROR;

    state = (struct inflate_state FAR *)strm->state;
    if (state->mode == TYPE) state->mode = TYPEDO;      /* skip check */
    LOAD();
    in = have;
    out = left;
    ret = Z_OK;
    for (;;)
        switch (state->mode) {
        case HEAD:
            if (state->wrap == 0) {
                state->mode = TYPEDO;
                break;
            }
            NEEDBITS(16);
#ifdef GUNZIP
            if ((state->wrap & 2) && hold == 0x8b1f) {  /* gzip header */
                state->check = crc32(0L, Z_NULL, 0);
                CRC2(state->check, hold);
                INITBITS();
                state->mode = FLAGS;
                break;
            }
            state->flags = 0;           /* expect zlib header */
            if (state->head != Z_NULL)
                state->head->done = -1;
            if (!(state->wrap & 1) ||   /* check if zlib header allowed */
#else
            if (
#endif
                ((BITS(8) << 8) + (hold >> 8)) % 31) {
                strm->msg = (char *)"incorrect header check";
                state->mode = BAD;
                break;
            }
            if (BITS(4) != Z_DEFLATED) {
                strm->msg = (char *)"unknown compression method";
                state->mode = BAD;
                break;
            }
            DROPBITS(4);
            len = BITS(4) + 8;
            if (len > state->wbits) {
                strm->msg = (char *)"invalid window size";
                state->mode = BAD;
                break;
            }
            state->dmax = 1U << len;
            Tracev((stderr, "inflate:   zlib header ok\n"));
            strm->adler = state->check = adler32(0L, Z_NULL, 0);
            state->mode = hold & 0x200 ? DICTID : TYPE;
            INITBITS();
            break;
#ifdef GUNZIP
        case FLAGS:
            NEEDBITS(16);
            state->flags = (int)(hold);
            if ((state->flags & 0xff) != Z_DEFLATED) {
                strm->msg = (char *)"unknown compression method";
                state->mode = BAD;
                break;
            }
            if (state->flags & 0xe000) {
                strm->msg = (char *)"unknown header flags set";
                state->mode = BAD;
                break;
            }
            if (state->head != Z_NULL)
                state->head->text = (int)((hold >> 8) & 1);
            if (state->flags & 0x0200) CRC2(state->check, hold);
            INITBITS();
            state->mode = TIME;
        case TIME:
            NEEDBITS(32);
            if (state->head != Z_NULL)
                state->head->time = hold;
            if (state->flags & 0x0200) CRC4(state->check, hold);
            INITBITS();
            state->mode = OS;
        case OS:
            NEEDBITS(16);
            if (state->head != Z_NULL) {
                state->head->xflags = (int)(hold & 0xff);
                state->head->os = (int)(hold >> 8);
            }
            if (state->flags & 0x0200) CRC2(state->check, hold);
            INITBITS();
            state->mode = EXLEN;
        case EXLEN:
            if (state->flags & 0x0400) {
                NEEDBITS(16);
                state->length = (unsigned)(hold);
                if (state->head != Z_NULL)
                    state->head->extra_len = (unsigned)hold;
                if (state->flags & 0x0200) CRC2(state->check, hold);
                INITBITS();
            }
            else if (state->head != Z_NULL)
                state->head->extra = Z_NULL;
            state->mode = EXTRA;
        case EXTRA:
            if (state->flags & 0x0400) {
                copy = state->length;
                if (copy > have) copy = have;
                if (copy) {
                    if (state->head != Z_NULL &&
                        state->head->extra != Z_NULL) {
                        len = state->head->extra_len - state->length;
                        zmemcpy(state->head->extra + len, next,
                                len + copy > state->head->extra_max ?
                                state->head->extra_max - len : copy);
                    }
                    if (state->flags & 0x0200)
                        state->check = crc32(state->check, next, copy);
                    have -= copy;
                    next += copy;
                    state->length -= copy;
                }
                if (state->length) goto inf_leave;
            }
            state->length = 0;
            state->mode = NAME;
        case NAME:
            if (state->flags & 0x0800) {
                if (have == 0) goto inf_leave;
                copy = 0;
                do {
                    len = (unsigned)(next[copy++]);
                    if (state->head != Z_NULL &&
                            state->head->name != Z_NULL &&
                            state->length < state->head->name_max)
                        state->head->name[state->length++] = len;
                } while (len && copy < have);
                if (state->flags & 0x0200)
                    state->check = crc32(state->check, next, copy);
                have -= copy;
                next += copy;
                if (len) goto inf_leave;
            }
            else if (state->head != Z_NULL)
                state->head->name = Z_NULL;
            state->length = 0;
            state->mode = COMMENT;
        case COMMENT:
            if (state->flags & 0x1000) {
                if (have == 0) goto inf_leave;
                copy = 0;
                do {
                    len = (unsigned)(next[copy++]);
                    if (state->head != Z_NULL &&
                            state->head->comment != Z_NULL &&
                            state->length < state->head->comm_max)
                        state->head->comment[state->length++] = len;
                } while (len && copy < have);
                if (state->flags & 0x0200)
                    state->check = crc32(state->check, next, copy);
                have -= copy;
                next += copy;
                if (len) goto inf_leave;
            }
            else if (state->head != Z_NULL)
                state->head->comment = Z_NULL;
            state->mode = HCRC;
        case HCRC:
            if (state->flags & 0x0200) {
                NEEDBITS(16);
                if (hold != (state->check & 0xffff)) {
                    strm->msg = (char *)"header crc mismatch";
                    state->mode = BAD;
                    break;
                }
                INITBITS();
            }
            if (state->head != Z_NULL) {
                state->head->hcrc = (int)((state->flags >> 9) & 1);
                state->head->done = 1;
            }
            strm->adler = state->check = crc32(0L, Z_NULL, 0);
            state->mode = TYPE;
            break;
#endif
        case DICTID:
            NEEDBITS(32);
            strm->adler = state->check = REVERSE(hold);
            INITBITS();
            state->mode = DICT;
        case DICT:
            if (state->havedict == 0) {
                RESTORE();
                return Z_NEED_DICT;
            }
            strm->adler = state->check = adler32(0L, Z_NULL, 0);
            state->mode = TYPE;
        case TYPE:
	    WATCHDOG_RESET();
            if (flush == Z_BLOCK) goto inf_leave;
        case TYPEDO:
            if (state->last) {
                BYTEBITS();
                state->mode = CHECK;
                break;
            }
            NEEDBITS(3);
            state->last = BITS(1);
            DROPBITS(1);
            switch (BITS(2)) {
            case 0:                             /* stored block */
                Tracev((stderr, "inflate:     stored block%s\n",
                        state->last ? " (last)" : ""));
                state->mode = STORED;
                break;
            case 1:                             /* fixed block */
                fixedtables(state);
                Tracev((stderr, "inflate:     fixed codes block%s\n",
                        state->last ? " (last)" : ""));
                state->mode = LEN;              /* decode codes */
                break;
            case 2:                             /* dynamic block */
                Tracev((stderr, "inflate:     dynamic codes block%s\n",
                        state->last ? " (last)" : ""));
                state->mode = TABLE;
                break;
            case 3:
                strm->msg = (char *)"invalid block type";
                state->mode = BAD;
            }
            DROPBITS(2);
            break;
        case STORED:
            BYTEBITS();                         /* go to byte boundary */
            NEEDBITS(32);
            if ((hold & 0xffff) != ((hold >> 16) ^ 0xffff)) {
                strm->msg = (char *)"invalid stored block lengths";
                state->mode = BAD;
                break;
            }
            state->length = (unsigned)hold & 0xffff;
            Tracev((stderr, "inflate:       stored length %u\n",
                    state->length));
            INITBITS();
            state->mode = COPY;
        case COPY:
            copy = state->length;
            if (copy) {
                if (copy > have) copy = have;
                if (copy > left) copy = left;
                if (copy == 0) goto inf_leave;
                zmemcpy(put, next, copy);
                have -= copy;
                next += copy;
                left -= copy;
                put += copy;
                state->length -= copy;
                break;
            }
            Tracev((stderr, "inflate:       stored end\n"));
            state->mode = TYPE;
            break;
        case TABLE:
            NEEDBITS(14);
            state->nlen = BITS(5) + 257;
            DROPBITS(5);
            state->ndist = BITS(5) + 1;
            DROPBITS(5);
            state->ncode = BITS(4) + 4;
            DROPBITS(4);
#ifndef PKZIP_BUG_WORKAROUND
            if (state->nlen > 286 || state->ndist > 30) {
                strm->msg = (char *)"too many length or distance symbols";
                state->mode = BAD;
                break;
            }
#endif
            Tracev((stderr, "inflate:       table sizes ok\n"));
            state->have = 0;
            state->mode = LENLENS;
        case LENLENS:
            while (state->have < state->ncode) {
                NEEDBITS(3);
                state->lens[order[state->have++]] = (unsigned short)BITS(3);
                DROPBITS(3);
            }
            while (state->have < 19)
                state->lens[order[state->have++]] = 0;
            state->next = state->codes;
            state->lencode = (code const FAR *)(state->next);
            state->lenbits = 7;
            ret = inflate_table(CODES, state->lens, 19, &(state->next),
                                &(state->lenbits), state->work);
            if (ret) {
                strm->msg = (char *)"invalid code lengths set";
                state->mode = BAD;
                break;
            }
            Tracev((stderr, "inflate:       code lengths ok\n"));
            state->have = 0;
            state->mode = CODELENS;
        case CODELENS:
            while (state->have < state->nlen + state->ndist) {
                for (;;) {
                    this = state->lencode[BITS(state->lenbits)];
                    if ((unsigned)(this.bits) <= bits) break;
                    PULLBYTE();
                }
                if (this.val < 16) {
                    NEEDBITS(this.bits);
                    DROPBITS(this.bits);
                    state->lens[state->have++] = this.val;
                }
                else {
                    if (this.val == 16) {
                        NEEDBITS(this.bits + 2);
                        DROPBITS(this.bits);
                        if (state->have == 0) {
                            strm->msg = (char *)"invalid bit length repeat";
                            state->mode = BAD;
                            break;
                        }
                        len = state->lens[state->have - 1];
                        copy = 3 + BITS(2);
                        DROPBITS(2);
                    }
                    else if (this.val == 17) {
                        NEEDBITS(this.bits + 3);
                        DROPBITS(this.bits);
                        len = 0;
                        copy = 3 + BITS(3);
                        DROPBITS(3);
                    }
                    else {
                        NEEDBITS(this.bits + 7);
                        DROPBITS(this.bits);
                        len = 0;
                        copy = 11 + BITS(7);
                        DROPBITS(7);
                    }
                    if (state->have + copy > state->nlen + state->ndist) {
                        strm->msg = (char *)"invalid bit length repeat";
                        state->mode = BAD;
                        break;
                    }
                    while (copy--)
                        state->lens[state->have++] = (unsigned short)len;
                }
            }

            /* handle error breaks in while */
            if (state->mode == BAD) break;

            /* build code tables */
            state->next = state->codes;
            state->lencode = (code const FAR *)(state->next);
            state->lenbits = 9;
            ret = inflate_table(LENS, state->lens, state->nlen, &(state->next),
                                &(state->lenbits), state->work);
            if (ret) {
                strm->msg = (char *)"invalid literal/lengths set";
                state->mode = BAD;
                break;
            }
            state->distcode = (code const FAR *)(state->next);
            state->distbits = 6;
            ret = inflate_table(DISTS, state->lens + state->nlen, state->ndist,
                            &(state->next), &(state->distbits), state->work);
            if (ret) {
                strm->msg = (char *)"invalid distances set";
                state->mode = BAD;
                break;
            }
            Tracev((stderr, "inflate:       codes ok\n"));
            state->mode = LEN;
        case LEN:
	    WATCHDOG_RESET();
            if (have >= 6 && left >= 258) {
                RESTORE();
                inflate_fast(strm, out);
                LOAD();
                break;
            }
            for (;;) {
                this = state->lencode[BITS(state->lenbits)];
                if ((unsigned)(this.bits) <= bits) break;
                PULLBYTE();
            }
            if (this.op && (this.op & 0xf0) == 0) {
                last = this;
                for (;;) {
                    this = state->lencode[last.val +
                            (BITS(last.bits + last.op) >> last.bits)];
                    if ((unsigned)(last.bits + this.bits) <= bits) break;
                    PULLBYTE();
                }
                DROPBITS(last.bits);
            }
            DROPBITS(this.bits);
            state->length = (unsigned)this.val;
            if ((int)(this.op) == 0) {
                Tracevv((stderr, this.val >= 0x20 && this.val < 0x7f ?
                        "inflate:         literal '%c'\n" :
                        "inflate:         literal 0x%02x\n", this.val));
                state->mode = LIT;
                break;
            }
            if (this.op & 32) {
                Tracevv((stderr, "inflate:         end of block\n"));
                state->mode = TYPE;
                break;
            }
            if (this.op & 64) {
                strm->msg = (char *)"invalid literal/length code";
                state->mode = BAD;
                break;
            }
            state->extra = (unsigned)(this.op) & 15;
            state->mode = LENEXT;
        case LENEXT:
            if (state->extra) {
                NEEDBITS(state->extra);
                state->length += BITS(state->extra);
                DROPBITS(state->extra);
            }
            Tracevv((stderr, "inflate:         length %u\n", state->length));
            state->mode = DIST;
        case DIST:
            for (;;) {
                this = state->distcode[BITS(state->distbits)];
                if ((unsigned)(this.bits) <= bits) break;
                PULLBYTE();
            }
            if ((this.op & 0xf0) == 0) {
                last = this;
                for (;;) {
                    this = state->distcode[last.val +
                            (BITS(last.bits + last.op) >> last.bits)];
                    if ((unsigned)(last.bits + this.bits) <= bits) break;
                    PULLBYTE();
                }
                DROPBITS(last.bits);
            }
            DROPBITS(this.bits);
            if (this.op & 64) {
                strm->msg = (char *)"invalid distance code";
                state->mode = BAD;
                break;
            }
            state->offset = (unsigned)this.val;
            state->extra = (unsigned)(this.op) & 15;
            state->mode = DISTEXT;
        case DISTEXT:
            if (state->extra) {
                NEEDBITS(state->extra);
                state->offset += BITS(state->extra);
                DROPBITS(state->extra);
            }
#ifdef INFLATE_STRICT
            if (state->offset > state->dmax) {
                strm->msg = (char *)"invalid distance too far back";
                state->mode = BAD;
                break;
            }
#endif
            if (state->offset > state->whave + out - left) {
                strm->msg = (char *)"invalid distance too far back";
                state->mode = BAD;
                break;
            }
            Tracevv((stderr, "inflate:         distance %u\n", state->offset));
            state->mode = MATCH;
        case MATCH:
            if (left == 0) goto inf_leave;
            copy = out - left;
            if (state->offset > copy) {         /* copy from window */
                copy = state->offset - copy;
                if (copy > state->write) {
                    copy -= state->write;
                    from = state->window + (state->wsize - copy);
                }
                else
                    from = state->window + (state->write - copy);
                if (copy > state->length) copy = state->length;
            }
            else {                              /* copy from output */
                from = put - state->offset;
                copy = state->length;
            }
            if (copy > left) copy = left;
            left -= copy;
            state->length -= copy;
            do {
                *put++ = *from++;
            } while (--copy);
            if (state->length == 0) state->mode = LEN;
            break;
        case LIT:
            if (left == 0) goto inf_leave;
            *put++ = (unsigned char)(state->length);
            left--;
            state->mode = LEN;
            break;
        case CHECK:
            if (state->wrap) {
                NEEDBITS(32);
                out -= left;
                strm->total_out += out;
                state->total += out;
                if (out)
                    strm->adler = state->check =
                        UPDATE(state->check, put - out, out);
                out = left;
                if ((
#ifdef GUNZIP
                     state->flags ? hold :
#endif
                     REVERSE(hold)) != state->check) {
                    strm->msg = (char *)"incorrect data check";
                    state->mode = BAD;
                    break;
                }
                INITBITS();
                Tracev((stderr, "inflate:   check matches trailer\n"));
            }
#ifdef GUNZIP
            state->mode = LENGTH;
        case LENGTH:
            if (state->wrap && state->flags) {
                NEEDBITS(32);
                if (hold != (state->total & 0xffffffffUL)) {
                    strm->msg = (char *)"incorrect length check";
                    state->mode = BAD;
                    break;
                }
                INITBITS();
                Tracev((stderr, "inflate:   length matches trailer\n"));
            }
#endif
            state->mode = DONE;
        case DONE:
            ret = Z_STREAM_END;
            goto inf_leave;
        case BAD:
            ret = Z_DATA_ERROR;
            goto inf_leave;
        case MEM:
            return Z_MEM_ERROR;
        case SYNC:
        default:
            return Z_STREAM_ERROR;
        }

    /*
       Return from inflate(), updating the total counts and the check value.
       If there was no progress during the inflate() call, return a buffer
       error.  Call updatewindow() to create and/or update the window state.
       Note: a memory error from inflate() is non-recoverable.
     */
  inf_leave:
    RESTORE();
    if (state->wsize || (state->mode < CHECK && out != strm->avail_out))
        if (updatewindow(strm, out)) {
            state->mode = MEM;
            return Z_MEM_ERROR;
        }
    in -= strm->avail_in;
    out -= strm->avail_out;
    strm->total_in += in;
    strm->total_out += out;
    state->total += out;
    if (state->wrap && out)
        strm->adler = state->check =
            UPDATE(state->check, strm->next_out - out, out);
    strm->data_type = state->bits + (state->last ? 64 : 0) +
                      (state->mode == TYPE ? 128 : 0);
    if (((in == 0 && out == 0) || flush == Z_FINISH) && ret == Z_OK)
        ret = Z_BUF_ERROR;
    return ret;
}

int ZEXPORT inflateEnd(z_streamp strm)
{
    struct inflate_state FAR *state;
    if (strm == Z_NULL || strm->state == Z_NULL || strm->zfree == (free_func)0)
        return Z_STREAM_ERROR;
    state = (struct inflate_state FAR *)strm->state;
    if (state->window != Z_NULL) {
	WATCHDOG_RESET();
	ZFREE(strm, state->window);
    }
    ZFREE(strm, strm->state);
    strm->state = Z_NULL;
    Tracev((stderr, "inflate: end\n"));
    return Z_OK;
}
