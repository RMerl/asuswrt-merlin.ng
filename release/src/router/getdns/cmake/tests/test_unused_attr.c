#include <stdio.h>

void f (char *u __attribute__((unused)));

void f(char *u)
{
}

int main (int ac, char *av[])
{
    f("str");
    return 0;
}
