# Just a skeleton lib for backwards compatibility since all the functionaliy
# has been moved into glib now

gthread_sources = ['gthread-impl.c']
if host_system == 'windows'
  gthread_win_rc = configure_file(
    input: 'gthread.rc.in',
    output: 'gthread.rc',
    configuration: glibconfig_conf,
  )
  gthread_win_res = windows.compile_resources(gthread_win_rc)
  gthread_sources += [gthread_win_res]
endif

libgthread = library('gthread-2.0',
  sources : gthread_sources,
  version : library_version,
  soversion : soversion,
  darwin_versions : darwin_versions,
  install : true,
  dependencies : [libglib_dep],
  c_args : ['-DG_LOG_DOMAIN="GThread"' ] + glib_hidden_visibility_args,
  link_args : glib_link_flags,
)

pkg.generate(libgthread,
  libraries : [thread_dep],
  requires : ['glib-2.0'],
  version : glib_version,
  install_dir : glib_pkgconfigreldir,
  filebase : 'gthread-2.0',
  name : 'GThread',
  description : 'Thread support for GLib',
)

libgthread_dep = declare_dependency(link_with : libgthread)
