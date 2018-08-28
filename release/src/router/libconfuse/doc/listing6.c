#include <stdio.h>
#include <confuse.h>

int main(void)
{
    /* ... setup options ... */

    cfg = cfg_init(opts, CFGF_NONE);
    cfg_parse(cfg, "hello.conf");

    if(cfg_size(cfg, "greeting") == 0)
    {
        cfg_parse_buf(cfg, "greeting Hello {}");
    }

    /* ... print the greetings ... */
}
