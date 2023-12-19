# handle substitution of variables in .in files

import Build, sys, Logs
from samba_utils import *

def subst_at_vars(task):
    '''substiture @VAR@ style variables in a file'''

    env = task.env
    src = task.inputs[0].srcpath(env)
    tgt = task.outputs[0].bldpath(env)

    f = open(src, 'r')
    s = f.read()
    f.close()
    # split on the vars
    a = re.split('(@\w+@)', s)
    out = []
    for v in a:
        if re.match('@\w+@', v):
            vname = v[1:-1]
            if not vname in task.env and vname.upper() in task.env:
                vname = vname.upper()
            if not vname in task.env:
                Logs.error("Unknown substitution %s in %s" % (v, task.name))
                sys.exit(1)
            v = SUBST_VARS_RECURSIVE(task.env[vname], task.env)
        out.append(v)
    contents = ''.join(out)
    f = open(tgt, 'w')
    s = f.write(contents)
    f.close()
    return 0

def CONFIGURE_FILE(bld, in_file, **kwargs):
    '''configure file'''

    base=os.path.basename(in_file)
    t = bld.SAMBA_GENERATOR('INFILE_%s' % base,
                            rule = subst_at_vars,
                            source = in_file + '.in',
                            target = in_file,
                            vars = kwargs)
Build.BuildContext.CONFIGURE_FILE = CONFIGURE_FILE
