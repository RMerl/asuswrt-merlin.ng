# We cannot use gnome.genmarshal() here
testmarshal_h = custom_target('testmarshal_h',
  output : 'testmarshal.h',
  input : 'testmarshal.list',
  command : [
    python, glib_genmarshal,
    '--prefix=test_marshal',
    '--output=@OUTPUT@',
    '--quiet',
    '--header',
    '@INPUT@',
  ],
)

testmarshal_c = custom_target('testmarshal_c',
  output : 'testmarshal.c',
  input : 'testmarshal.list',
  command : [
    python, glib_genmarshal,
    '--prefix=test_marshal',
    '--include-header=testmarshal.h',
    '--output=@OUTPUT@',
    '--quiet',
    '--body',
    '@INPUT@',
  ],
)

gobject_tests = {
  'gvalue-test' : {'tap' : true},
  'paramspec-test' : {'tap' : true},
  'deftype' : {},
  'defaultiface' : {
    'extra_sources' : ['testmodule.c'],
  },
  'dynamictype' : {
    'extra_sources' : ['testmodule.c'],
  },
  'override' : {},
  'signals' : {},
  'singleton' : {},
  'references' : {},
  'testgobject' : {},
  'accumulator' : {
    'extra_sources' : [testmarshal_c, testmarshal_h],
  },
}

if host_system != 'windows'
  gobject_tests += {
    'timeloop-closure' : {},
  }
endif

common_c_args = test_cargs + ['-DGLIB_DISABLE_DEPRECATION_WARNINGS']
common_deps = [libm, thread_dep, libglib_dep, libgobject_dep]

foreach test_name, extra_args : gobject_tests
  source = extra_args.get('source', test_name + '.c')
  extra_sources = extra_args.get('extra_sources', [])
  install = installed_tests_enabled and extra_args.get('install', true)
  template = extra_args.get('tap', false) ? installed_tests_template_tap : installed_tests_template
  test_command_args = extra_args.get('tap', false) ? ['--tap'] : []

  if install
    test_conf = configuration_data()
    test_conf.set('installed_tests_dir', installed_tests_execdir)
    test_conf.set('program', test_name)
    configure_file(
      input: template,
      output: test_name + '.test',
      install_dir: installed_tests_metadir,
      configuration: test_conf
    )
  endif

  # FIXME? $(GLIB_DEBUG_FLAGS)
  exe = executable(test_name, [source, extra_sources],
    c_args : common_c_args + extra_args.get('c_args', []),
    dependencies : common_deps + extra_args.get('dependencies', []),
    install_dir: installed_tests_execdir,
    install: install,
  )

  suite = ['gobject'] + extra_args.get('suite', [])
  timeout = suite.contains('slow') ? test_timeout_slow : test_timeout
  # FIXME? TESTS_ENVIRONMENT = LIBCHARSET_ALIAS_DIR=$(top_builddir)/glib/libcharset
  test(test_name, exe, env : test_env, timeout : timeout, suite : suite,
    args : test_command_args)
endforeach

# Don't install these ones, and keep them out of 'make check' because they take too long...
executable('performance', 'performance.c',
    c_args : common_c_args,
    dependencies : common_deps,
    install : false)

executable('performance-threaded', 'performance-threaded.c',
    c_args : common_c_args,
    dependencies : common_deps,
    install : false)
