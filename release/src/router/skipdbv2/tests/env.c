#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

//TODO for shell env
int main(int argc, char **argv, char * envp[])
{
    char** env;
    char* this_env;
    char *v;

    for (env = envp; *env != 0; env++) {
        this_env = (char*)strdup(*env);
        v = strstr(this_env, "=");
        if(v != NULL) {
            *v = '\0';
            v += 1;
            printf("%s=%s\n", this_env, v);
        }
        if(this_env != NULL) {
            free(this_env);
        }
    }
  putenv("SomeVariable=SomeValue");
  return 0;
}
