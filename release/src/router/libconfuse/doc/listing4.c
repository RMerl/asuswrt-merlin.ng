#include <stdio.h>
#include <confuse.h>

int main(void)
{
    cfg_opt_t opts[] =
    {
        CFG_STR_LIST("targets", "{World}", CFGF_NONE),
        CFG_INT("repeat", 1, CFGF_NONE),
        CFG_END()
    };
    cfg_t *cfg;
    int repeat;
    int i;

    cfg = cfg_init(opts, CFGF_NONE);
    if(cfg_parse(cfg, "hello.conf") == CFG_PARSE_ERROR)
        return 1;

    repeat = cfg_getint(cfg, "repeat");
    while(repeat--)
    {
        printf("Hello");
        for(i = 0; i < cfg_size(cfg, "targets"); i++)
            printf(", %s", cfg_getnstr(cfg, "targets", i));
        printf("!\n");
    }

    cfg_free(cfg);
    return 0;
}

