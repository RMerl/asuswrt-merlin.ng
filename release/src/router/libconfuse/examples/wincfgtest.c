/* Simple Windows program that calls LoadLibrary to load the libConfuse DLL
 * into memory.
 */

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#include "confuse.h"

#ifdef __BORLANDC__
# define DLLSYM(sym) "_" ## sym /* duh! */
#else
# define DLLSYM(sym) sym
#endif

int cb_message(cfg_t *cfg, cfg_opt_t *opt, int argc, const char **argv)
{
    if(argc != 1)
        MessageBox(0, "message() requires only 1 argument", "message() function", MB_OK);
    else
        MessageBox(0, argv[0], "message() function", MB_OK);
    return 0;
}

static void display_last_error(void)
{
    LPVOID lpMsgBuf;
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
       			  FORMAT_MESSAGE_FROM_SYSTEM |
       			  FORMAT_MESSAGE_IGNORE_INSERTS,
       			  NULL,
       			  GetLastError(),
       			  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), /* Default language */
       			  (LPTSTR) &lpMsgBuf,
       			  0,
       			  NULL);

    MessageBox(NULL, (LPCTSTR)lpMsgBuf, "libConfuse error", MB_OK | MB_ICONINFORMATION );
    LocalFree( lpMsgBuf );
}

typedef cfg_t *(*CFG_INIT_FPTR)(cfg_opt_t *opts, cfg_flag_t flags);
typedef void (*CFG_FREE_FPTR)(cfg_t *cfg);
typedef int (*CFG_PARSE_FPTR)(cfg_t *cfg, const char *filename);
typedef cfg_bool_t (*CFG_GETBOOL_FPTR)(cfg_t *cfg, const char *name);
typedef long int (*CFG_GETINT_FPTR)(cfg_t *cfg, const char *name);
typedef char * (*CFG_GETSTR_FPTR)(cfg_t *cfg, const char *name);
typedef double (*CFG_GETFLOAT_FPTR)(cfg_t *cfg, const char *name);

int main(void)
{
    cfg_opt_t opts[] = {
        CFG_BOOL("bool", cfg_false, 0),
        CFG_STR("string", "default test string", 0),
        CFG_INT("number", 17, 0),
        CFG_FLOAT("float", 6.789, 0),
        CFG_FUNC("message", &cb_message),
        CFG_END()
    };
    HINSTANCE hinstLib;
    char buf[1024];

    CFG_INIT_FPTR cfg_init;
    CFG_PARSE_FPTR cfg_parse;
    CFG_FREE_FPTR cfg_free;
    CFG_GETBOOL_FPTR cfg_getbool;
    CFG_GETINT_FPTR cfg_getint;
    CFG_GETSTR_FPTR cfg_getstr;
    CFG_GETFLOAT_FPTR cfg_getfloat;

    /* Get a handle to the DLL module. */
    hinstLib = LoadLibrary("libConfuse");

    /* If the handle is valid, try to get the function address. */
    if(hinstLib != NULL) {
        cfg_init = (CFG_INIT_FPTR)GetProcAddress(hinstLib, DLLSYM("cfg_init"));
        cfg_parse = (CFG_PARSE_FPTR)GetProcAddress(hinstLib, DLLSYM("cfg_parse"));
        cfg_free = (CFG_FREE_FPTR)GetProcAddress(hinstLib, DLLSYM("cfg_free"));
        cfg_getbool = (CFG_GETBOOL_FPTR)GetProcAddress(hinstLib, DLLSYM("cfg_getbool"));
        cfg_getint = (CFG_GETINT_FPTR)GetProcAddress(hinstLib, DLLSYM("cfg_getint"));
        cfg_getstr = (CFG_GETSTR_FPTR)GetProcAddress(hinstLib, DLLSYM("cfg_getstr"));
        cfg_getfloat = (CFG_GETFLOAT_FPTR)GetProcAddress(hinstLib, DLLSYM("cfg_getfloat"));

        if(cfg_init) { /* assume the other functions also are valid */
            cfg_t *cfg = cfg_init(opts, 0);
            if(cfg_parse(cfg, "wincfgtest.conf") == CFG_FILE_ERROR)
                perror("wincfgtest.conf");

           	sprintf(buf, "bool:    %s\nstring:  %s\nnumber:  %ld\nfloat:   %f\n",
            	cfg_getbool(cfg, "bool") ? "true" : "false", cfg_getstr(cfg, "string"),
             	cfg_getint(cfg, "number"), cfg_getfloat(cfg, "float"));
           	MessageBox(NULL, buf, "libConfuse", MB_OK);

            cfg_free(cfg);
        } else
        	display_last_error();

        FreeLibrary(hinstLib);
    } else
    	display_last_error();

    return 0;
}

