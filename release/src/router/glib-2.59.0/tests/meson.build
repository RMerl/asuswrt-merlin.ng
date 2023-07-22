# tests

# Not entirely random of course, but at least it changes over time
random_number = minor_version + meson.version().split('.').get(1).to_int()

test_env = environment()
test_env.set('G_TEST_SRCDIR', meson.current_source_dir())
test_env.set('G_TEST_BUILDDIR', meson.current_build_dir())
test_env.set('G_DEBUG', 'gc-friendly')
test_env.set('MALLOC_CHECK_', '2')
test_env.set('MALLOC_PERTURB_', '@0@'.format(random_number % 256))

test_cargs = ['-DG_LOG_DOMAIN="GLib"']

subdir('gobject')
subdir('refcount')

tests = {
  'testglib' : {'tap' : true},
  'testgdate' : {},
  'datetime' : {},
  'atomic-test' : {},
  'bit-test' : {},
  'child-test' : {},
  'completion-test' : {},
  'dirname-test' : {},
  'file-test' : {},
  'env-test' : {},
  'gio-test' : {},
  'mainloop-test' : {},
  'mapping-test' : {},
  'onceinit' : {},
  'asyncqueue-test' : {},
  'qsort-test' : {},
  'relation-test' : {},
  'slice-concurrent' : {},
  'slice-threadinit' : {
    'dependencies' : [libgthread_dep],
  },
  'sources' : {},
  'thread-test' : {},
  'threadpool-test' : {'suite' : ['slow']},
  'type-test' : {},
  'unicode-caseconv' : {},
  'unicode-encoding' : {},
  'module-test' : {
    'dependencies' : [libgmodule_dep],
    'export_dynamic' : true,
  },
  'cxx-test' : {
    'source' : 'cxx-test.cpp',
    'include_directories' : gmoduleinc,
    'dependencies' : [libgio_dep],
  },
}

test_extra_programs = {
  'slice-test' : {
    'extra_sources' : ['memchunks.c'],
  },
  'slice-color' : {
    'extra_sources' : ['memchunks.c'],
  },
  'assert-msg-test' : {},
  'unicode-collate' : {},
}

if host_machine.system() != 'windows'
  tests += {
    'timeloop' : {},
    'spawn-test' : {},
    'iochannel-test' : {},
  }
endif

if installed_tests_enabled
  install_data(
    'iochannel-test-infile',
    'casemap.txt',
    'casefold.txt',
    'utf8.txt',
    install_dir : installed_tests_execdir,
  )
endif

foreach module : ['moduletestplugin_a', 'moduletestplugin_b']
  shared_module(module, 'lib@0@.c'.format(module),
    dependencies : [libglib_dep, libgmodule_dep],
    install_dir : installed_tests_execdir,
    install : installed_tests_enabled
  )
endforeach

common_c_args = test_cargs + ['-DGLIB_DISABLE_DEPRECATION_WARNINGS']
common_deps = [libm, thread_dep, libglib_dep]

foreach test_name, extra_args : tests
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
    export_dynamic : extra_args.get('export_dynamic', false),
    include_directories : extra_args.get('include_directories', []),
    install_dir: installed_tests_execdir,
    install: install,
  )

  suite = ['glib'] + extra_args.get('suite', [])
  timeout = suite.contains('slow') ? test_timeout_slow : test_timeout
  # FIXME? TESTS_ENVIRONMENT = LIBCHARSET_ALIAS_DIR=$(top_builddir)/glib/libcharset
  test(test_name, exe, env : test_env, timeout : timeout, suite : suite,
    args : test_command_args)
endforeach

foreach program_name, extra_args : test_extra_programs
  source = extra_args.get('source', program_name + '.c')
  extra_sources = extra_args.get('extra_sources', [])
  install = installed_tests_enabled and extra_args.get('install', true)
  executable(program_name, [source, extra_sources],
      c_args : common_c_args,
      dependencies : common_deps + extra_args.get('dependencies', []),
      install_dir : installed_tests_execdir,
      install : install,
  )
endforeach
