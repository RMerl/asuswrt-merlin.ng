common_gio_tests_deps = [
  libglib_dep,
  libgmodule_dep,
  libgobject_dep,
  libgio_dep,
]

test_c_args = [
  '-DG_LOG_DOMAIN="GLib-GIO"',
  '-DTEST_SERVICES="@0@/gio/tests/services"'.format(meson.build_root()),
  '-DGLIB_MKENUMS="@0@"'.format(glib_mkenums),
  '-DGLIB_COMPILE_SCHEMAS="@0@"'.format(glib_compile_schemas.full_path()),
]

if host_machine.system() == 'windows'
  common_gio_tests_deps += [iphlpapi_dep, winsock2, cc.find_library ('secur32')]
endif

subdir('gdbus-object-manager-example')

gengiotypefuncs_prog = find_program('gengiotypefuncs.py')
giotypefuncs_inc = custom_target(
  'giotypefuncs.inc',
  output : 'giotypefuncs.inc',
  input : gio_headers + [gioenumtypes_h] + gobject_install_headers,
  command: [gengiotypefuncs_prog, '@OUTPUT@', '@INPUT@'])

#  Test programs buildable on all platforms
gio_tests = {
  'appmonitor' : {},
  'async-close-output-stream' : {},
  'async-splice-output-stream' : {},
  'buffered-input-stream' : {},
  'buffered-output-stream' : {},
  'cancellable' : {},
  'contexts' : {},
  'contenttype' : {},
  'converter-stream' : {},
  'credentials' : {},
  'data-input-stream' : {},
  'data-output-stream' : {},
  'defaultvalue' : {'extra_sources' : [giotypefuncs_inc]},
  'fileattributematcher' : {},
  'filter-streams' : {},
  'giomodule' : {},
  'gsubprocess' : {},
  'g-file' : {},
  'g-file-info' : {},
  'g-icon' : {},
  'gdbus-addresses' : {},
  'gdbus-message' : {},
  'inet-address' : {},
  'io-stream' : {},
  'memory-input-stream' : {},
  'memory-output-stream' : {},
  'monitor' : {},
  'mount-operation' : {},
  'network-address' : {'extra_sources': ['mock-resolver.c']},
  'network-monitor' : {},
  'network-monitor-race' : {},
  'permission' : {},
  'pollable' : {},
  'proxy-test' : {},
  'readwrite' : {},
  'simple-async-result' : {},
  'simple-proxy' : {},
  'sleepy-stream' : {},
  'socket' : {},
  'socket-listener' : {},
  'socket-service' : {},
  'srvtarget' : {},
  'task' : {},
  'vfs' : {},
  'volumemonitor' : {},
  'glistmodel' : {},
  'testfilemonitor' : {'suite' : ['slow']},
  'thumbnail-verification' : {},
  'tls-certificate' : {'extra_sources' : ['gtesttlsbackend.c']},
  'tls-interaction' : {'extra_sources' : ['gtesttlsbackend.c']},
  'tls-database' : {'extra_sources' : ['gtesttlsbackend.c']},
}

test_extra_programs = {
  'gdbus-connection-flush-helper' : {},
  'gdbus-testserver' : {},
  'gsubprocess-testprog' : {},
}

test_env = environment()
test_env.set('G_TEST_SRCDIR', meson.current_source_dir())
test_env.set('G_TEST_BUILDDIR', meson.current_build_dir())
test_env.set('GIO_MODULE_DIR', '')
test_env.set('GIO_LAUNCH_DESKTOP', meson.build_root() + '/gio/gio-launch-desktop')

# Check for libdbus1 - Optional - is only used in the GDBus test cases
# 1.2.14 required for dbus_message_set_serial
dbus1_dep = dependency('dbus-1', required : false, version : '>= 1.2.14')
if not dbus1_dep.found()
  if cc.get_id() == 'msvc'
    # MSVC: Search for the DBus library by the configuration, which corresponds
    # to the output of CMake builds of DBus.  Note that debugoptimized
    # is really a Release build with .PDB files.
    if buildtype == 'debug'
      dbus1_dep = cc.find_library('dbus-1d', required : false)
    else
      dbus1_dep = cc.find_library('dbus-1', required : false)
    endif
  endif
endif
if dbus1_dep.found()
  glib_conf.set('HAVE_DBUS1', 1)

  gio_tests += {
    'gdbus-serialization' : {
      'extra_sources' : ['gdbus-tests.c'],
      'dependencies' : [dbus1_dep],
    }
  }
endif

#  Test programs buildable on UNIX only
if host_machine.system() != 'windows'
  gio_tests += {
    'file' : {},
    'gdbus-peer' : {
      'dependencies' : [libgdbus_example_objectmanager_dep],
      'install_rpath' : installed_tests_execdir
    },
    'gdbus-peer-object-manager' : {},
    'live-g-file' : {},
    'socket-address' : {},
    'stream-rw_all' : {},
    'unix-fd' : {},
    'unix-mounts' : {},
    'unix-streams' : {},
    'g-file-info-filesystem-readonly' : {},
    'gsocketclient-slow' : {
      'depends' : [
        shared_library('slow-connect-preload',
          'slow-connect-preload.c',
          name_prefix : '',
          dependencies: cc.find_library('dl'),
        )
      ],
      'env' : {
        'LD_PRELOAD': '@0@/slow-connect-preload.so'.format(meson.current_build_dir())
      }
    },
    'gschema-compile' : {'install' : false},
    'trash' : {},
  }

  # Uninstalled because of the check-for-executable logic in DesktopAppInfo
  # unable to find the installed executable
  if not glib_have_cocoa
    gio_tests += {
      'appinfo' : {
        'install' : false,
      },
      'desktop-app-info' : {
        'install' : false,
      },
    }
  endif

  test_extra_programs += {
    'basic-application' : {},
    'dbus-launch' : {},
    'appinfo-test' : {},
  }

  if not glib_have_cocoa
    test_extra_programs += {
      'apps' : {},
    }
    gio_tests += {
      'mimeapps' : {},
    }
  endif

  #  Test programs that need to bring up a session bus (requires dbus-daemon)
  have_dbus_daemon = find_program('dbus-daemon', required : false).found()
  if have_dbus_daemon
    annotate_args = [
      '--annotate', 'org.project.Bar', 'Key1', 'Value1',
      '--annotate', 'org.project.Bar', 'org.gtk.GDBus.Internal', 'Value2',
      '--annotate', 'org.project.Bar.HelloWorld()', 'Key3', 'Value3',
      '--annotate', 'org.project.Bar::TestSignal', 'Key4', 'Value4',
      '--annotate', 'org.project.Bar:ay', 'Key5', 'Value5',
      '--annotate', 'org.project.Bar.TestPrimitiveTypes()[val_int32]', 'Key6', 'Value6',
      '--annotate', 'org.project.Bar.TestPrimitiveTypes()[ret_uint32]', 'Key7', 'Value7',
      '--annotate', 'org.project.Bar::TestSignal[array_of_strings]', 'Key8', 'Value8',
    ]
    # Generate gdbus-test-codegen-generated.{c,h}
    gdbus_test_codegen_generated = custom_target('gdbus-test-codegen-generated',
        input :   ['test-codegen.xml'],
        output :  ['gdbus-test-codegen-generated.h',
                   'gdbus-test-codegen-generated.c'],
        depend_files : gdbus_codegen_built_files,
        command : [python, gdbus_codegen,
                   '--interface-prefix', 'org.project.',
                   '--output-directory', '@OUTDIR@',
                   '--generate-c-code', 'gdbus-test-codegen-generated',
                   '--c-generate-object-manager',
                   '--c-generate-autocleanup', 'all',
                   '--c-namespace', 'Foo_iGen',
                   '--generate-docbook', 'gdbus-test-codegen-generated-doc',
                   annotate_args,
                   '@INPUT@'])

    gdbus_test_codegen_generated_interface_info = [
      custom_target('gdbus-test-codegen-generated-interface-info-h',
          input :   ['test-codegen.xml'],
          output :  ['gdbus-test-codegen-generated-interface-info.h'],
          depend_files : gdbus_codegen_built_files,
          command : [python, gdbus_codegen,
                     '--interface-info-header',
                     annotate_args,
                     '--output', '@OUTPUT@',
                     '@INPUT@']),
      custom_target('gdbus-test-codegen-generated-interface-info-c',
          input :   ['test-codegen.xml'],
          output :  ['gdbus-test-codegen-generated-interface-info.c'],
          depend_files : gdbus_codegen_built_files,
          command : [python, gdbus_codegen,
                     '--interface-info-body',
                     annotate_args,
                     '--output', '@OUTPUT@',
                     '@INPUT@']),
    ]

    extra_sources = ['gdbus-sessionbus.c', 'gdbus-tests.c']

    gio_tests += {
      'actions' : {
        'extra_sources' : extra_sources,
        'suite' : ['slow'],
      },
      'gdbus-auth' : {'extra_sources' : extra_sources},
      'gdbus-bz627724' : {'extra_sources' : extra_sources},
      'gdbus-close-pending' : {'extra_sources' : extra_sources},
      'gdbus-connection' : {'extra_sources' : extra_sources},
      'gdbus-connection-loss' : {'extra_sources' : extra_sources},
      'gdbus-connection-slow' : {'extra_sources' : extra_sources},
      'gdbus-error' : {'extra_sources' : extra_sources},
      'gdbus-exit-on-close' : {'extra_sources' : extra_sources},
      'gdbus-export' : {
        'extra_sources' : extra_sources,
        'suite' : ['slow'],
      },
      'gdbus-introspection' : {'extra_sources' : extra_sources},
      'gdbus-names' : {'extra_sources' : extra_sources},
      'gdbus-proxy' : {'extra_sources' : extra_sources},
      'gdbus-proxy-threads' : {
        'extra_sources' : extra_sources,
        'dependencies' : [dbus1_dep],
      },
      'gdbus-proxy-unique-name' : {'extra_sources' : extra_sources},
      'gdbus-proxy-well-known-name' : {'extra_sources' : extra_sources},
      'gdbus-test-codegen' : {
        'extra_sources' : [extra_sources, gdbus_test_codegen_generated, gdbus_test_codegen_generated_interface_info],
      },
      'gdbus-threading' : {
        'extra_sources' : extra_sources,
        'suite' : ['slow'],
      },
      'gmenumodel' : {
        'extra_sources' : extra_sources,
        'suite' : ['slow'],
      },
      'gnotification' : {
        'extra_sources' : [extra_sources, 'gnotification-server.c'],
      },
      'gdbus-test-codegen-old' : {
        'source' : 'gdbus-test-codegen.c',
        'extra_sources' : [extra_sources, gdbus_test_codegen_generated, gdbus_test_codegen_generated_interface_info],
        'c_args' : ['-DGLIB_VERSION_MIN_REQUIRED=GLIB_VERSION_2_36',
                    '-DGLIB_VERSION_MAX_ALLOWED=GLIB_VERSION_2_36'],
      },
      'gapplication' : {'extra_sources' : extra_sources},
      'gdbus-unix-addresses' : {},
    }

    if not glib_have_cocoa
      gio_tests += {
        'dbus-appinfo' : {
          'extra_sources' : extra_sources,
        },
      }
    endif
  endif # have_dbus_daemon

  # This test is currently unreliable
  executable('gdbus-overflow', 'gdbus-overflow.c',
      c_args : test_c_args,
      dependencies : common_gio_tests_deps,
      install_dir : installed_tests_execdir,
      install : installed_tests_enabled)

  gio_tests += {
    'gdbus-connection-flush' : {
      'extra_sources' : ['test-io-stream.c', 'test-pipe-unix.c'],
    },
    'gdbus-non-socket' : {
      'extra_sources' : ['gdbus-tests.c', 'test-io-stream.c', 'test-pipe-unix.c'],
    },
  }

  # Generate test.mo from de.po using msgfmt
  msgfmt = find_program('msgfmt', required : false)
  if msgfmt.found()
    subdir('de/LC_MESSAGES')
    gio_tests += {
      'gsettings' : {
        'extra_sources' : [test_mo],
        'c_args' : ['-DSRCDIR="@0@"'.format(meson.current_source_dir()),
                    '-DTEST_LOCALE_PATH="@0@"'.format(test_mo_dir)],
        'install' : false,
      },
    }
  endif
endif # unix

#  Test programs buildable on Windows only
if host_machine.system() == 'windows'
  gio_tests += {'win32-streams' : {}}
endif

if cc.get_id() != 'msvc'
  gio_tests += {
    'autoptr-gio' : {
      'source' : 'autoptr.c',
    },
  }
endif

test_extra_programs += {
  'gio-du' : {'install' : false},
  'echo-server' : {'install' : false},
  'filter-cat' : {'install' : false},
  'gapplication-example-actions' : {'install' : false},
  'gapplication-example-cmdline' : {'install' : false},
  'gapplication-example-cmdline2' : {'install' : false},
  'gapplication-example-cmdline3' : {'install' : false},
  'gapplication-example-cmdline4' : {'install' : false},
  'gapplication-example-dbushooks' : {'install' : false},
  'gapplication-example-open' : {'install' : false},
  'gdbus-daemon' : {
    'extra_sources' : gdbus_daemon_sources,
    'install' : false,
  },
  'gdbus-example-export' : {'install' : false},
  'gdbus-example-own-name' : {'install' : false},
  'gdbus-example-peer' : {'install' : false},
  'gdbus-example-proxy-subclass' : {'install' : false},
  'gdbus-example-server' : {'install' : false},
  'gdbus-example-subtree' : {'install' : false},
  'gdbus-example-watch-name' : {'install' : false},
  'gdbus-example-watch-proxy' : {'install' : false},
  'httpd' : {'install' : false},
  'proxy' : {'install' : false},
  'resolver' : {'install' : false},
  'send-data' : {'install' : false},
  'socket-server' : {'install' : false},
  'socket-client' : {
    'extra_sources' : ['gtlsconsoleinteraction.c'],
    'install' : false,
  },
}

if cc.get_id() != 'msvc'
  test_extra_programs += {
    # These three are manual-run tests because they need a session bus but don't bring one up themselves
    # FIXME: these build but don't seem to work!
    'gdbus-example-objectmanager-client' : {
      'dependencies' : [libgdbus_example_objectmanager_dep],
      'install' : false,
    },
    'gdbus-example-objectmanager-server' : {
      'dependencies' : [libgdbus_example_objectmanager_dep],
      'install' : false,
    },
    'gdbus-test-fixture' : {
      'dependencies' : [libgdbus_example_objectmanager_dep],
      'install' : false,
    },
  }
endif

if host_machine.system() != 'windows'
  test_extra_programs += {
    'gdbus-example-unix-fd-client' : {
      'install' : false,
    },
  }
endif

appinfo_test_desktop_files = [
  'appinfo-test-gnome.desktop',
  'appinfo-test-notgnome.desktop',
  'appinfo-test.desktop',
  'appinfo-test2.desktop',
]

cdata = configuration_data()
if installed_tests_enabled
  cdata.set('installed_tests_dir', installed_tests_execdir)
else
  cdata.set('installed_tests_dir', meson.current_build_dir())
endif

foreach appinfo_test_desktop_file : appinfo_test_desktop_files
  configure_file(
    input: appinfo_test_desktop_file + '.in',
    output: appinfo_test_desktop_file,
    install_dir: installed_tests_execdir,
    install: installed_tests_enabled,
    configuration: cdata,
  )
endforeach

if installed_tests_enabled
  install_data(
    'contexts.c',
    'g-icon.c',
    'appinfo-test-actions.desktop',
    'appinfo-test-static.desktop',
    'file.c',
    'org.gtk.test.dbusappinfo.desktop',
    install_dir : installed_tests_execdir,
  )
  install_subdir('x-content', install_dir : installed_tests_execdir)
  install_subdir('desktop-files', install_dir : installed_tests_execdir)
  install_subdir('thumbnails', install_dir : installed_tests_execdir)
  install_subdir('cert-tests', install_dir : installed_tests_execdir)

  cdata = configuration_data()
  cdata.set('installed_tests_dir', installed_tests_execdir)
  cdata.set('program', 'static-link.py ' + glib_pkgconfigreldir)
  configure_file(
    input: installed_tests_template,
    output: 'static-link.test',
    install_dir: installed_tests_metadir,
    configuration: cdata
  )
  install_subdir('static-link', install_dir : installed_tests_execdir)
  install_data('static-link.py', install_dir : installed_tests_execdir)
endif

if not meson.is_cross_build() or meson.has_exe_wrapper()

  plugin_resources_c = custom_target('plugin-resources.c',
    input : 'test4.gresource.xml',
    output : 'plugin-resources.c',
    command : [glib_compile_resources,
               '--target=@OUTPUT@',
               '--sourcedir=' + meson.current_source_dir(),
               '--generate-source',
               '--c-name', '_g_plugin',
               '@INPUT@'])

  shared_module('resourceplugin', 'resourceplugin.c', plugin_resources_c,
    link_args : export_dynamic_ldflags,
    dependencies : common_gio_tests_deps,
    install_dir : installed_tests_execdir,
    install : installed_tests_enabled
  )

  # referenced by test2.gresource.xml
  big_test_resource = custom_target(
    'gresource-big-test.txt',
    input : ['gen-big-test-resource.py'],
    output : ['gresource-big-test.txt'],
    command : [python, '@INPUT0@', '@OUTPUT@'])

  test_gresource = custom_target('test.gresource',
    input : 'test.gresource.xml',
    output : 'test.gresource',
    command : [glib_compile_resources,
               '--target=@OUTPUT@',
               '--sourcedir=' + meson.current_source_dir(),
               '--sourcedir=' + meson.current_build_dir(),
               '@INPUT@'],
    install_dir : installed_tests_execdir,
    install : installed_tests_enabled)

  test_resources2_c = custom_target('test_resources2.c',
    input : 'test3.gresource.xml',
    output : 'test_resources2.c',
    command : [glib_compile_resources,
               '--target=@OUTPUT@',
               '--sourcedir=' + meson.current_source_dir(),
               '--generate',
               '--c-name', '_g_test2',
               '--manual-register',
               '@INPUT@'])

  test_resources2_h = custom_target('test_resources2.h',
    input : 'test3.gresource.xml',
    output : 'test_resources2.h',
    command : [glib_compile_resources,
               '--target=@OUTPUT@',
               '--sourcedir=' + meson.current_source_dir(),
               '--generate',
               '--c-name', '_g_test2',
               '--manual-register',
               '@INPUT@'])

  test_resources_c = custom_target('test_resources.c',
    input : 'test2.gresource.xml',
    depends : big_test_resource,
    output : 'test_resources.c',
    command : [glib_compile_resources,
               '--target=@OUTPUT@',
               '--sourcedir=' + meson.current_source_dir(),
               '--sourcedir=' + meson.current_build_dir(),
               '--generate-source',
               '--c-name', '_g_test1',
               '@INPUT@'])

  # referenced by test.gresource.xml
  test_generated_txt = configure_file(input : 'test1.txt',
    output : 'test-generated.txt',
    copy : true,
    install : false)

  # Create object file containing resource data
  # for testing the external data option
  if build_machine.system() == 'linux'
    test_gresource_binary = custom_target('test5.gresource',
      input : 'test5.gresource.xml',
      output : 'test5.gresource',
      command : [glib_compile_resources,
                 '--target=@OUTPUT@',
                 '--sourcedir=' + meson.current_source_dir(),
                 '--sourcedir=' + meson.current_build_dir(),
                 '@INPUT@'],
      install_dir : installed_tests_execdir,
      install : installed_tests_enabled)

	# Create resource data file
    test_resources_binary_c = custom_target('test_resources_binary.c',
      input : 'test5.gresource.xml',
      output : 'test_resources_binary.c',
      command : [glib_compile_resources,
                 '--target=@OUTPUT@',
                 '--sourcedir=' + meson.current_source_dir(),
                 '--sourcedir=' + meson.current_build_dir(),
                 '--generate-source',
                 '--external-data',
                 '--c-name', '_g_binary_test1',
                 '@INPUT@'])

	# Create object file containing resource data
    test_resources_binary = custom_target('test_resources.o',
      input : test_gresource_binary,
      output : 'test_resources.o',
      command : ['ld',
                 '-r',
                 '-b','binary',
                 '@INPUT@',
                 '-o','@OUTPUT@'])

	# Rename symbol to match the one in the C file
    test_resources_binary2 = custom_target('test_resources2.o',
      input : test_resources_binary,
      output : 'test_resources2.o',
      command : ['objcopy',
                 '--add-symbol','_g_binary_test1_resource_data=.data:0',
                 '@INPUT@',
                 '@OUTPUT@'])

    gio_tests += {
      'resources' : {
        'extra_sources' : [test_gresource, test_resources_c, test_resources2_c,
                           test_resources2_h, test_resources_binary_c,
                           test_resources_binary2],
      },
    }
  else
    gio_tests += {
      'resources' : {
        'extra_sources' : [test_gresource, test_resources_c, test_resources2_c,
                           test_resources2_h],
      },
    }
  endif
endif

foreach test_name, extra_args : gio_tests
  source = extra_args.get('source', test_name + '.c')
  extra_sources = extra_args.get('extra_sources', [])
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

  exe = executable(test_name, [source, extra_sources],
    c_args : test_c_args + extra_args.get('c_args', []),
    dependencies : common_gio_tests_deps + extra_args.get('dependencies', []),
    install_rpath : extra_args.get('install_rpath', ''),
    install_dir: installed_tests_execdir,
    install: install,
  )

  suite = ['gio'] + extra_args.get('suite', [])
  timeout = suite.contains('slow') ? test_timeout_slow : test_timeout
  local_test_env = test_env

  foreach var, value : extra_args.get('env', {})
    local_test_env.append(var, value)
  endforeach

  test(test_name, exe,
    env : local_test_env,
    timeout : timeout,
    suite : suite,
    args : ['--tap'],
    is_parallel : extra_args.get('is_parallel', true),
    depends : extra_args.get('depends', []),
  )
endforeach

foreach program_name, extra_args : test_extra_programs
  source = extra_args.get('source', program_name + '.c')
  extra_sources = extra_args.get('extra_sources', [])
  install = installed_tests_enabled and extra_args.get('install', true)
  executable(program_name, [source, extra_sources],
      c_args : test_c_args,
      dependencies : common_gio_tests_deps + extra_args.get('dependencies', []),
      install_dir : installed_tests_execdir,
      install : install,
  )
endforeach

# FIXME: subdir('services')
subdir('modules')
