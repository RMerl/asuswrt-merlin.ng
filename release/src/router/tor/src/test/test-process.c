/* Copyright (c) 2011-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

#include "orconfig.h"
#include <stdio.h>
#ifdef _WIN32
#define WINDOWS_LEAN_AND_MEAN
#include <windows.h>
#else
#include <unistd.h>
#endif /* defined(_WIN32) */
#include <string.h>
#include <stdlib.h>

#ifdef _WIN32
#define SLEEP(sec) Sleep((sec)*1000)
#else
#define SLEEP(sec) sleep(sec)
#endif

/* Trivial test program to test process_t. */
int
main(int argc, char **argv)
{
  /* Does our process get the right arguments? */
  for (int i = 0; i < argc; ++i) {
    fprintf(stdout, "argv[%d] = '%s'\n", i, argv[i]);
    fflush(stdout);
  }

  /* Make sure our process got our environment variable. */
  fprintf(stdout, "Environment variable TOR_TEST_ENV = '%s'\n",
          getenv("TOR_TEST_ENV"));
  fflush(stdout);

  /* Test line handling on stdout and stderr. */
  fprintf(stdout, "Output on stdout\nThis is a new line\n");
  fflush(stdout);

  fprintf(stderr, "Output on stderr\nThis is a new line\n");
  fflush(stderr);

  fprintf(stdout, "Partial line on stdout ...");
  fflush(stdout);

  fprintf(stderr, "Partial line on stderr ...");
  fflush(stderr);

  SLEEP(2);

  fprintf(stdout, "end of partial line on stdout\n");
  fflush(stdout);
  fprintf(stderr, "end of partial line on stderr\n");
  fflush(stderr);

  /* Echo input from stdin. */
  char buffer[1024];

  int count = 0;

  while (fgets(buffer, sizeof(buffer), stdin)) {
    /* Strip the newline. */
    size_t size = strlen(buffer);

    if (size >= 1 && buffer[size - 1] == '\n') {
      buffer[size - 1] = '\0';
      --size;
    }

    if (size >= 1 && buffer[size - 1] == '\r') {
      buffer[size - 1] = '\0';
      --size;
    }

    fprintf(stdout, "Read line from stdin: '%s'\n", buffer);
    fflush(stdout);

    if (++count == 3)
      break;
  }

  fprintf(stdout, "We are done for here, thank you!\n");

  return 0;
}
