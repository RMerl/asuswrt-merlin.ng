glib_tests = {
  'array-test' : {},
  'asyncqueue' : {},
  'atomic' : {
    'c_args' : cc.get_id() == 'gcc' ? ['-Wstrict-aliasing=2'] : [],
  },
  'base64' : {},
  'bitlock' : {},
  'bookmarkfile' : {},
  'bytes' : {},
  'cache' : {},
  'charset' : {},
  'checksum' : {},
  'collate' : {},
  'cond' : {},
  'convert' : {},
  'dataset' : {},
  'date' : {},
  'dir' : {},
  'environment' : {},
  'error' : {},
  'fileutils' : {},
  'gdatetime' : {
    'suite' : ['slow'],
  },
  'guuid' : {},
  'gvariant' : {
    'suite' : ['slow'],
  },
  'gwakeup' : {
    'source' : ['gwakeuptest.c', '../gwakeup.c'],
    'install' : false,
  },
  'hash' : {},
  'hmac' : {},
  'hook' : {},
  'hostutils' : {},
  'keyfile' : {},
  'list' : {},
  'logging' : {},
  'mainloop' : {},
  'mappedfile' : {},
  'markup' : {},
  'markup-parse' : {},
  'markup-collect' : {},
  'markup-escape' : {},
  'markup-subparser' : {},
  'mem-overflow' : {},
  'mutex' : {},
  'node' : {},
  'once' : {},
  'option-context' : {},
  'option-argv0' : {},
  'overflow' : {},
  'overflow-fallback' : {
    'source' : 'overflow.c',
    'c_args' : ['-D_GLIB_TEST_OVERFLOW_FALLBACK'],
  },
  'pattern' : {},
  'private' : {},
  'protocol' : {},
  'queue' : {},
  'rand' : {},
  'rcbox' : {},
  'rec-mutex' : {},
  'refcount' : {},
  'refcount-macro' : {
    'source' : 'refcount.c',
    'c_args' : ['-DG_DISABLE_CHECKS'],
  },
  'refstring' : {},
  'regex' : {
    'dependencies' : [pcre],
    'c_args' : use_pcre_static_flag ? ['-DPCRE_STATIC'] : [],
  },
  'rwlock' : {},
  'scannerapi' : {},
  'search-utils' : {},
  'sequence' : {
    'suite' : ['slow'],
  },
  'shell' : {},
  'slice' : {},
  'slist' : {},
  'sort' : {},
  'spawn-multithreaded' : {},
  'spawn-singlethread' : {},
  'strfuncs' : {},
  'string' : {},
  'testing' : {},
  'test-printf' : {},
  'thread' : {},
  'timeout' : {},
  'timer' : {},
  'tree' : {},
  'utf8-performance' : {},
  'utf8-pointer' : {},
  'utf8-validate' : {},
  'utf8-misc' : {},
  'utils' : {},
  'unicode' : {},
  'uri' : {},
  '1bit-mutex' : {},
  '1bit-emufutex' : {
    'source' : '1bit-mutex.c',
    'c_args' : ['-DTEST_EMULATED_FUTEX'],
    'install' : false,
    'suite' : ['slow'],
  },
  '642026' : {
    'suite' : ['slow'],
  },
  '642026-ec' : {
    'source' : '642026.c',
    'c_args' : ['-DG_ERRORCHECK_MUTEXES'],
    'suite' : ['slow'],
  },
}

if cc.get_id() != 'msvc'
  glib_tests += {'autoptr' : {}}
endif

if glib_conf.has('HAVE_EVENTFD')
  glib_tests += {
    'gwakeup-fallback' : {
      'source' : ['gwakeuptest.c', '../gwakeup.c'],
      'c_args' : ['-DTEST_EVENTFD_FALLBACK'],
      'install' : false,
    },
  }
endif

if host_machine.system() != 'windows'
  glib_tests += {
    'include' : {},
    'unix' : {},
  }
endif

if installed_tests_enabled
  install_data(
    'keyfiletest.ini',
    'pages.ini',
    'keyfile.c',
    'empty',
    '4096-random-bytes',
    'echo-script',
    'echo-script.bat',
    install_dir : installed_tests_execdir,
  )
  install_subdir('bookmarks', install_dir : installed_tests_execdir)
  install_subdir('markups', install_dir : installed_tests_execdir)
endif

# Not entirely random of course, but at least it changes over time
random_number = minor_version + meson.version().split('.').get(1).to_int()

test_env = environment()
test_env.set('G_TEST_SRCDIR', meson.current_source_dir())
test_env.set('G_TEST_BUILDDIR', meson.current_build_dir())
test_env.set('G_DEBUG', 'gc-friendly')
test_env.set('MALLOC_CHECK_', '2')
test_env.set('MALLOC_PERTURB_', '@0@'.format(random_number % 256))

test_deps = [libm, thread_dep, libglib_dep]
test_cargs = ['-DG_LOG_DOMAIN="GLib"']

foreach test_name, extra_args : glib_tests
  source = extra_args.get('source', test_name + '.c')
  install = installed_tests_enabled and extra_args.get('install', true)

  if install
    test_conf = configuration_data()
    test_conf.set('installed_tests_dir', installed_tests_execdir)
    test_conf.set('program', test_name)
    configure_file(
      input: installed_tests_template_tap,
      output: test_name + '.test',
      install_dir: installed_tests_metadir,
      configuration: test_conf
    )
  endif

  exe = executable(test_name, source,
    c_args : test_cargs + extra_args.get('c_args', []),
    dependencies : test_deps + extra_args.get('dependencies', []),
    install_dir: installed_tests_execdir,
    install: install,
  )

  suite = ['glib'] + extra_args.get('suite', [])
  timeout = suite.contains('slow') ? test_timeout_slow : test_timeout
  test(test_name, exe, env : test_env, timeout : timeout, suite : suite,
    args : ['--tap'])
endforeach

# test-spawn-echo helper binary required by the spawn tests above
executable('test-spawn-echo', 'test-spawn-echo.c',
  c_args : test_cargs,
  dependencies : test_deps,
  install_dir: installed_tests_execdir,
  install: installed_tests_enabled,
)

executable('testing-helper', 'testing-helper.c',
  c_args : test_cargs,
  dependencies : test_deps,
  install_dir: installed_tests_execdir,
  install: installed_tests_enabled,
)

# some testing of gtester functionality
if not meson.is_cross_build() and host_system != 'windows'
  xmllint = find_program('xmllint', required: false)
  if xmllint.found()
    tmpsample_xml = custom_target('tmpsample.xml',
      output : 'tmpsample.xml',
      command : [ gtester, '-k', '--quiet', '-o', '@OUTPUT@',
                  '--test-arg=--gtester-selftest', gtester])

    test('gtester-xmllint-check', xmllint,
      args : ['--noout', tmpsample_xml],
      env : test_env,
      suite : ['glib'],
    )
  endif
endif
