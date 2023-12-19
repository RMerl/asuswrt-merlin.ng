#!/usr/bin/env python
# encoding: utf-8
# andersg at 0x63.nu 2007

import os
import Task, Options, Utils
from Configure import conf
from TaskGen import extension, taskgen, feature, before

xsubpp_str = '${PERL} ${XSUBPP} -noprototypes -typemap ${EXTUTILS_TYPEMAP} ${SRC} > ${TGT}'
EXT_XS = ['.xs']

@before('apply_incpaths', 'apply_type_vars', 'apply_lib_vars')
@feature('perlext')
def init_perlext(self):
	self.uselib = self.to_list(getattr(self, 'uselib', ''))
	if not 'PERL' in self.uselib: self.uselib.append('PERL')
	if not 'PERLEXT' in self.uselib: self.uselib.append('PERLEXT')
	self.env['shlib_PATTERN'] = self.env['perlext_PATTERN']

@extension(EXT_XS)
def xsubpp_file(self, node):
	outnode = node.change_ext('.c')
	self.create_task('xsubpp', node, outnode)
	self.allnodes.append(outnode)

Task.simple_task_type('xsubpp', xsubpp_str, color='BLUE', before='cc cxx', shell=False)

@conf
def check_perl_version(conf, minver=None):
	"""
	Checks if perl is installed.

	If installed the variable PERL will be set in environment.

	Perl binary can be overridden by --with-perl-binary config variable

	"""

	if getattr(Options.options, 'perlbinary', None):
		conf.env.PERL = Options.options.perlbinary
	else:
		conf.find_program('perl', var='PERL', mandatory=True)

	try:
		version = Utils.cmd_output([conf.env.PERL, '-e', 'printf "%vd",$^V'])
	except:
		conf.fatal('could not determine the perl version')

	conf.env.PERL_VERSION = version
	cver = ''
	if minver:
		try:
			ver = tuple(map(int, version.split('.')))
		except:
			conf.fatal('unsupported perl version %r' % version)
		if ver < minver:
			conf.fatal('perl is too old')

		cver = '.'.join(map(str,minver))
	conf.check_message('perl', cver, True, version)

@conf
def check_perl_module(conf, module):
	"""
	Check if specified perlmodule is installed.

	Minimum version can be specified by specifying it after modulename
	like this:

	conf.check_perl_module("Some::Module 2.92")
	"""
	cmd = [conf.env['PERL'], '-e', 'use %s' % module]
	r = Utils.pproc.call(cmd, stdout=Utils.pproc.PIPE, stderr=Utils.pproc.PIPE) == 0
	conf.check_message("perl module %s" % module, "", r)
	return r

@conf
def check_perl_ext_devel(conf):
	"""
	Check for configuration needed to build perl extensions.

	Sets different xxx_PERLEXT variables in the environment.

	Also sets the ARCHDIR_PERL variable useful as installation path,
	which can be overridden by --with-perl-archdir
	"""
	if not conf.env.PERL:
		conf.fatal('perl detection is required first')

	def read_out(cmd):
		return Utils.to_list(Utils.cmd_output([conf.env.PERL, '-MConfig', '-e', cmd]))

	conf.env.LINKFLAGS_PERLEXT = read_out('print $Config{lddlflags}')
	conf.env.CPPPATH_PERLEXT   = read_out('print "$Config{archlib}/CORE"')
	conf.env.CCFLAGS_PERLEXT   = read_out('print "$Config{ccflags} $Config{cccdlflags}"')
	conf.env.XSUBPP            = read_out('print "$Config{privlib}/ExtUtils/xsubpp$Config{exe_ext}"')
	conf.env.EXTUTILS_TYPEMAP  = read_out('print "$Config{privlib}/ExtUtils/typemap"')
	conf.env.perlext_PATTERN   = '%s.' + read_out('print $Config{dlext}')[0]

	def try_any(keys):
		for k in keys:
			conf.start_msg("Checking for perl $Config{%s}:" % k)
			try:
				v = read_out('print $Config{%s}' % k)[0]
				conf.end_msg("'%s'" % (v), 'GREEN')
				return v
			except IndexError:
				conf.end_msg(False, 'YELLOW')
				pass
		return None

	perl_arch_install_dir = None
	if getattr(Options.options, 'perl_arch_install_dir', None):
		perl_arch_install_dir = Options.options.perl_arch_install_dir
	if perl_arch_install_dir is None:
		perl_arch_install_dir = try_any(['vendorarch', 'sitearch', 'archlib'])
	if perl_arch_install_dir is None:
		conf.fatal('No perl arch install directory autodetected.' +
			   'Please define it with --with-perl-arch-install-dir.')
	conf.start_msg("PERL_ARCH_INSTALL_DIR: ")
	conf.end_msg("'%s'" % (perl_arch_install_dir), 'GREEN')
	conf.env.PERL_ARCH_INSTALL_DIR = perl_arch_install_dir

	perl_lib_install_dir = None
	if getattr(Options.options, 'perl_lib_install_dir', None):
		perl_lib_install_dir = Options.options.perl_lib_install_dir
	if perl_lib_install_dir is None:
		perl_lib_install_dir = try_any(['vendorlib', 'sitelib', 'privlib'])
	if perl_lib_install_dir is None:
		conf.fatal('No perl lib install directory autodetected. ' +
			   'Please define it with --with-perl-lib-install-dir.')
	conf.start_msg("PERL_LIB_INSTALL_DIR: ")
	conf.end_msg("'%s'" % (perl_lib_install_dir), 'GREEN')
	conf.env.PERL_LIB_INSTALL_DIR = perl_lib_install_dir

def set_options(opt):
	opt.add_option("--with-perl-binary", type="string", dest="perlbinary", help = 'Specify alternate perl binary', default=None)

	opt.add_option("--with-perl-arch-install-dir",
		       type="string",
		       dest="perl_arch_install_dir",
		       help = ('Specify directory where to install arch specific files'),
		       default=None)

	opt.add_option("--with-perl-lib-install-dir",
		       type="string",
		       dest="perl_lib_install_dir",
		       help = ('Specify directory where to install vendor specific files'),
		       default=None)
