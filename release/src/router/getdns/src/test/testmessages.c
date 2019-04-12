/**
 * \file
 * @brief display messages to support unit testing
 */

/*
 * Copyright (c) 2013, NLNet Labs, Verisign, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * * Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * * Neither the names of the copyright holders nor the
 *   names of its contributors may be used to endorse or promote products
 *   derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Verisign, Inc. BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <string.h>
#include "testmessages.h"

static char *testprog = NULL;
static char **cases = NULL;
static int ncases = 0;

void
tstmsg_prog_begin(char *prognm)
{
	if (testprog != NULL) {
		tstmsg_prog_end();
	}
	testprog = strdup(prognm);
	printf("TESTPROG %s START\n", testprog);
}				/* tstmsg_prog_begin */

void
tstmsg_prog_end()
{
	printf("TESTPROG %s END\n", testprog);
	free(testprog);
}				/* tstmsg_prog_end */

void
tstmsg_case_begin(char *casenm)
{
	ncases++;
	cases = (char **) realloc(cases, sizeof(char *) * ncases);
	cases[ncases - 1] = strdup(casenm);

	printf("TESTCASE %s:%s BEGIN\n", testprog, cases[ncases - 1]);
}				/* tstmsg_case_begin */

void
tstmsg_case_end(void)
{
	if (ncases > 0) {
		printf("TESTCASE %s:%s END\n", testprog, cases[ncases - 1]);
		ncases--;
		free(cases[ncases]);
		if (ncases) {
			cases =
			    (char **) realloc(cases, sizeof(char *) * ncases);
		} else {
			cases = NULL;
		}
	}
}				/* tstmsg_case_end */

void
tstmsg_case_msg(char *msg)
{
	printf("  %s:%s: %s\n", testprog, cases[ncases - 1], msg);
}				/* tstmsg_case_msg */

/* testmessages.c */
