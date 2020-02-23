#include <stdio.h>

void f(char *format, ...) __attribute__ ((format (printf, 1, 2)));

void f(char *format, ...)
{
}

int main (int ac, char *av[])
{
    f("%s", "str");
    return 0;
}
