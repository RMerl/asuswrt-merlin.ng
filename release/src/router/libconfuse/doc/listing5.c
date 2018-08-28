#include <stdio.h>
#include <confuse.h>

int main(void)
{
    cfg_opt_t greet_opts[] =
    {
        CFG_STR_LIST("targets", "{World}", CFGF_NONE),
        CFG_INT("repeat", 1, CFGF_NONE),
        CFG_END()
    };
    cfg_opt_t opts[] =
    {
        CFG_SEC("greeting", greet_opts, CFGF_TITLE | CFGF_MULTI),
        CFG_END()
    };
    cfg_t *cfg, *cfg_greet;
    int repeat;
    int i, j;

    cfg = cfg_init(opts, CFGF_NONE);
    if(cfg_parse(cfg, "hello.conf") == CFG_PARSE_ERROR)
        return 1;

    for(j = 0; j < cfg_size(cfg, "greeting"); j++)
    {
        cfg_greet = cfg_getnsec(cfg, "greeting", j);

        repeat = cfg_getint(cfg_greet, "repeat");
        while(repeat--)
        {
            printf("%s", cfg_title(cfg_greet));
            for(i = 0; i < cfg_size(cfg_greet, "targets"); i++)
                printf(", %s", cfg_getnstr(cfg_greet, "targets", i));
            printf("!\n");
        }
    }

    cfg_free(cfg);
    return 0;
}

