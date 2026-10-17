version_script = '-Wl,--version-script=@0@'.format(meson.current_source_dir() / 'version.lds')
if not cc.has_link_argument(version_script)
  version_script = []
endif

sources = files(
  'fdt.c',
  'fdt_addresses.c',
  'fdt_check.c',
  'fdt_empty_tree.c',
  'fdt_overlay.c',
  'fdt_ro.c',
  'fdt_rw.c',
  'fdt_strerror.c',
  'fdt_sw.c',
  'fdt_wip.c',
)

link_args = []
if cc.has_link_argument('-Wl,--no-undefined')
    link_args += '-Wl,--no-undefined'
else
    # -undefined error is the equivalent of --no-undefined for the macOS linker,
    # but -undefined would also be understood as a valid argument for GNU ld!
    link_args += cc.get_supported_link_arguments('-Wl,-undefined,error')
endif

link_args += version_script
libfdt = both_libraries(
  'fdt', sources,
  version: meson.project_version(),
  link_args: link_args,
  link_depends: 'version.lds',
  install: true,
)

if static_build
  link_with = libfdt.get_static_lib()
else
  link_with = libfdt.get_shared_lib()
endif

libfdt_inc = include_directories('.')

libfdt_dep = declare_dependency(
  include_directories: libfdt_inc,
  link_with: link_with,
)

install_headers(
  files(
    'fdt.h',
    'libfdt.h',
    'libfdt_env.h',
  )
)

pkgconfig = import('pkgconfig')

pkgconfig.generate(
  libraries: libfdt,
  version: meson.project_version(),
  filebase: 'libfdt',
  name: 'libfdt',
  description: 'Flat Device Tree manipulation',
)
