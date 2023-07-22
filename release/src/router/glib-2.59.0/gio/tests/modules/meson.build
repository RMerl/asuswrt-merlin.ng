libtestmodulea = shared_module('testmodulea', 'test-module-a.c',
  dependencies : [libglib_dep, libgobject_dep, libgmodule_dep, libgio_dep],
  c_args : [ ],
  install : installed_tests_enabled,
  install_dir : join_paths(installed_tests_execdir, 'modules'),
)

libtestmoduleb = shared_module('testmoduleb', 'test-module-b.c',
  dependencies : [libglib_dep, libgobject_dep, libgmodule_dep, libgio_dep],
  c_args : [ ],
  install : installed_tests_enabled,
  install_dir : join_paths(installed_tests_execdir, 'modules'),
)
