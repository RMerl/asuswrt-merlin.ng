setup_py = find_program('../setup.py')
setup_py = [setup_py, '--quiet', '--top-builddir', meson.project_build_root()]

pylibfdt = custom_target(
  'pylibfdt',
  input: 'libfdt.i',
  depends: libfdt,
  output: '_libfdt.so',
  command: [setup_py, 'build_ext'],
  build_by_default: true,
)

meson.add_install_script(setup_py, 'install', '--prefix=' + get_option('prefix'), '--root=$DESTDIR')
