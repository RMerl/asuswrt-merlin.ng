# glib enables -Werror=format-nonliteral by default, but the embedded gnulib
# needs to handle user provided format strings.
extra_gnulib_args = cc.get_supported_arguments([
  '-Wno-format-nonliteral', '-Wno-duplicated-branches'])

gnulib_lib = static_library('gnulib', 'asnprintf.c', 'printf.c', 'printf-args.c', 'printf-parse.c', 'vasnprintf.c',
  include_directories : [configinc, glibinc],
  pic : true,
  c_args : [ '-DLIBDIR="@0@"'.format(get_option('libdir')), '-DGLIB_COMPILATION', '-DG_LOG_DOMAIN="GLib"' ] + glib_hidden_visibility_args + extra_gnulib_args)
