if not get_option('fam')
  subdir_done()
endif

fam_dep = cc.find_library('fam')
fam_c_args = ['-DG_DISABLE_DEPRECATED'] + gio_c_args
if cc.has_function('FAMNoExists', dependencies : fam_dep)
  fam_c_args += '-DHAVE_FAM_NO_EXISTS=1'
endif

deps = [
  fam_dep,
  libglib_dep,
  libgobject_dep,
  libgio_dep,
]

symbol_map = join_paths(meson.current_source_dir(), 'gfamfilemonitor.map')
fam_ldflags = cc.get_supported_link_arguments([
  '-Wl,--version-script,' + symbol_map,
  '-Wl,-no-undefined',
])

module = shared_module('giofam', 'gfamfilemonitor.c',
  include_directories : [gmoduleinc],
  dependencies : deps,
  c_args : fam_c_args,
  link_args : fam_ldflags,
  link_depends : symbol_map,
  install_dir : glib_giomodulesdir,
  install : true,
)

if not meson.is_cross_build()
  meson.add_install_script('../gio-querymodules-wrapper.py', gio_querymodules.full_path(), glib_giomodulesdir)
endif
