# waf build tool for building IDL files with pidl

import Build
from samba_utils import *
from samba_autoconf import *

from Configure import conf

@conf
def SAMBA_CHECK_PYTHON(conf, mandatory=True, version=(2,4,2)):
    # enable tool to build python extensions
    conf.find_program('python', var='PYTHON', mandatory=mandatory)
    conf.check_tool('python')
    path_python = conf.find_program('python')
    conf.env.PYTHON_SPECIFIED = (conf.env.PYTHON != path_python)
    conf.check_python_version(version)

@conf
def SAMBA_CHECK_PYTHON_HEADERS(conf, mandatory=True):
    if conf.env["python_headers_checked"] == []:
        conf.check_python_headers(mandatory)
        conf.env["python_headers_checked"] = "yes"
    else:
        conf.msg("python headers", "using cache")


def SAMBA_PYTHON(bld, name,
                 source='',
                 deps='',
                 public_deps='',
                 realname=None,
                 cflags='',
                 includes='',
                 init_function_sentinel=None,
                 local_include=True,
                 vars=None,
                 enabled=True):
    '''build a python extension for Samba'''

    # when we support static python modules we'll need to gather
    # the list from all the SAMBA_PYTHON() targets
    if init_function_sentinel is not None:
        cflags += '-DSTATIC_LIBPYTHON_MODULES=%s' % init_function_sentinel

    source = bld.EXPAND_VARIABLES(source, vars=vars)

    if realname is not None:
        link_name = 'python_modules/%s' % realname
    else:
        link_name = None

    bld.SAMBA_LIBRARY(name,
                      source=source,
                      deps=deps,
                      public_deps=public_deps,
                      includes=includes,
                      cflags=cflags,
                      local_include=local_include,
                      vars=vars,
                      realname=realname,
                      link_name=link_name,
                      pyext=True,
                      target_type='PYTHON',
                      install_path='${PYTHONARCHDIR}',
                      allow_undefined_symbols=True,
                      allow_warnings=True,
                      enabled=enabled)

Build.BuildContext.SAMBA_PYTHON = SAMBA_PYTHON
