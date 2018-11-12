/* HEADER Testing asprintf() */

char *strp;

OK(asprintf(&strp, "%s", "") == 0, "asprintf() (1) failed");
OKF(strp && strcmp(strp, "") == 0, ("%s <> \"\"", strp));
free(strp);
OK(asprintf(&strp, "%s", "abc") == 3, "asprintf() (1) failed");
OKF(strp && strcmp(strp, "abc") == 0, ("%s <> \"abc\"", strp));
free(strp);
OK(asprintf(&strp, "%s %s", "abc", "def") == 7, "asprintf() (2) failed");
OKF(strp && strcmp(strp, "abc def") == 0, ("%s <> \"abc def\"", strp));
free(strp);
