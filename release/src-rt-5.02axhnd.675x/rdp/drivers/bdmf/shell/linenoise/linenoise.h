/* linenoise.h -- guerrilla line editing library against the idea that a
 * line editing lib needs to be 20,000 lines of C code.
 *
 * See linenoise.c for more information.
 *
 * ------------------------------------------------------------------------
 *
 * Copyright (c) 2010, Salvatore Sanfilippo <antirez at gmail dot com>
 * Copyright (c) 2010, Pieter Noordhuis <pcnoordhuis at gmail dot com>
 *
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 
 *  *  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *  *  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __LINENOISE_H
#define __LINENOISE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct linenoiseSessionIO
{
    long fd_in;
    long fd_out;
    int (*read_char)(long fd_in, char *c);
    int (*write)(long fd_out, const char *buf, size_t len);
    int dumb_terminal; /* 1=dumb terminal. do not use escape sequences */
} linenoiseSessionIO;

typedef struct linenoiseSession linenoiseSession;

typedef int (linenoiseCompletionCallback)(linenoiseSession *session, const char *, int pos);
int linenoiseSessionOpen(const linenoiseSessionIO *io, void *session_data, linenoiseSession **session);
void linenoiseSessionClose(linenoiseSession *session);
void *linenoiseSessionData(linenoiseSession *session);
void linenoiseSetCompletionCallback(linenoiseSession *session, linenoiseCompletionCallback *);
void linenoiseSetBuffer(linenoiseSession *session, const char *buf, int pos);

char *linenoise(linenoiseSession *session, const char *prompt, char *buf, size_t size);
int linenoiseHistoryAdd(linenoiseSession *session, const char *line);
int linenoiseHistorySetMaxLen(linenoiseSession *session, int len);
int linenoiseHistorySave(linenoiseSession *session, const char *filename);
int linenoiseHistoryLoad(linenoiseSession *session, const char *filename);
void linenoiseClearScreen(linenoiseSession *session);
void linenoiseSetMultiLine(linenoiseSession *session, int ml);
void linenoiseSetDumbTerminal(linenoiseSession *session, int dumb);
int linenoiseGetMultiLine(linenoiseSession *session);
int linenoiseGetDumbTerminal(linenoiseSession *session);

#ifdef __cplusplus
}
#endif

#endif /* __LINENOISE_H */
