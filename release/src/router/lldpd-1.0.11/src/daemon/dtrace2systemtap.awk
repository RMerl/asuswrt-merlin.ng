#!/usr/bin/awk -f

# Convert a simple dtrace probe files into a tapset. Heavily inspired
# by dtrace2systemtap.pl from libvirt

($1 == "provider") {
    provider = $2
}

($1 == "probe") {
    name = substr($2, 0, index($2, "(") - 1)
    split(substr($0, index($0, "(") + 1, index($0, ")") - index($0, "(") - 1),
          args, /, /)
    printf "probe %s.%s = process(\"%s/%s\").provider(\"%s\").mark(\"%s\") {\n", provider, name, sbindir, provider, provider, name
    for (arg in args) {
        match(args[arg], /^(.+[^a-z_])([a-z_]+)$/, aarg)
        type = aarg[1]
        argname = aarg[2]
        if (type == "char *")
            printf "   %s = user_string($arg%d);\n", argname, arg
        else
            printf "   %s = $arg%d;\n", argname, arg
    }
    printf "}\n\n"
}
