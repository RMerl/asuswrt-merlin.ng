int validate_unsigned_int(cfg_t *cfg, cfg_opt_t *opt)
{
    int value = cfg_opt_getnint(opt, cfg_opt_size(opt) - 1);
    if(value < 0)
    {
        cfg_error(cfg, "integer option '%s' must be positive in section '%s'",
                opt->name, cfg->name);
        return -1;
    }
    return 0;
}

