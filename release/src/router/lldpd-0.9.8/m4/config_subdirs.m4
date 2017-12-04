#
# lldp_CONFIG_SUBDIRS
#
# This is almost like AC_CONFIG_SUBDIRS but it will take additional
# arguments for ./configure. Also, ./configure is not delayed. Be sure
# to call that late enough.

AC_DEFUN([lldp_CONFIG_SUBDIRS], [
  AC_CONFIG_SUBDIRS([$1])
  ac_dir="m4_normalize([$1])"
  if test -f "$srcdir/$ac_dir/configure"; then
    ac_sub_configure_args=
    ac_prev=
    eval "set x $ac_configure_args"
    shift
    for ac_arg
    do
      if test -n "$ac_prev"; then
        ac_prev=
        continue
      fi
      case $ac_arg in
      -cache-file | --cache-file | --cache-fil | --cache-fi \
      | --cache-f | --cache- | --cache | --cach | --cac | --ca | --c)
        ac_prev=cache_file ;;
      -cache-file=* | --cache-file=* | --cache-fil=* | --cache-fi=* \
      | --cache-f=* | --cache-=* | --cache=* | --cach=* | --cac=* | --ca=* \
      | --c=*)
        ;;
      --config-cache | -C)
        ;;
      -srcdir | --srcdir | --srcdi | --srcd | --src | --sr)
        ac_prev=srcdir ;;
      -srcdir=* | --srcdir=* | --srcdi=* | --srcd=* | --src=* | --sr=*)
        ;;
      -prefix | --prefix | --prefi | --pref | --pre | --pr | --p)
        ac_prev=prefix ;;
      -prefix=* | --prefix=* | --prefi=* | --pref=* | --pre=* | --pr=* | --p=*)
        ;;
      --disable-option-checking)
        ;;
      *)
        case $ac_arg in
        *\'*) ac_arg=`AS_ECHO(["$ac_arg"]) | sed "s/'/'\\\\\\\\''/g"` ;;
        esac
        AS_VAR_APPEND([ac_sub_configure_args], [" '$ac_arg'"]) ;;
      esac
    done

    # Always prepend --prefix to ensure using the same prefix
    # in subdir configurations.
    ac_arg="--prefix=$prefix"
    case $ac_arg in
    *\'*) ac_arg=`AS_ECHO(["$ac_arg"]) | sed "s/'/'\\\\\\\\''/g"` ;;
    esac
    ac_sub_configure_args="'$ac_arg' $ac_sub_configure_args"

    # Always prepend --disable-option-checking to silence warnings, since
    # different subdirs can have different --enable and --with options.
    ac_sub_configure_args="--disable-option-checking $ac_sub_configure_args"

    # Silent rules
    case $enable_silent_rules in
      no)  ac_sub_configure_args="$ac_sub_configure_args --disable-silent-rules" ;;
      *)   ac_sub_configure_args="$ac_sub_configure_args --enable-silent-rules" ;;
    esac

    # Add additional options
    ac_sub_configure_args="$ac_sub_configure_args $2"

    ac_popdir=`pwd`

    ac_msg="=== configuring in $ac_dir (`pwd`/$ac_dir)"
    _AS_ECHO_LOG([$ac_msg])
    _AS_ECHO([$ac_msg])
    AS_MKDIR_P(["$ac_dir"])
    _AC_SRCDIRS(["$ac_dir"])

    cd "$ac_dir"

    ac_sub_configure=$ac_srcdir/configure

    # Make the cache file name correct relative to the subdirectory.
    case $cache_file in
        [[\\/]]* | ?:[[\\/]]* ) ac_sub_cache_file=$cache_file ;;
        *) # Relative name.
           ac_sub_cache_file=$ac_top_build_prefix$cache_file ;;
    esac

    AC_MSG_NOTICE([running $SHELL $ac_sub_configure $ac_sub_configure_args --cache-file=$ac_sub_cache_file --srcdir=$ac_srcdir])
    # The eval makes quoting arguments work.
    eval "\$SHELL \"\$ac_sub_configure\" $ac_sub_configure_args \
           --cache-file=\"\$ac_sub_cache_file\" --srcdir=\"\$ac_srcdir\"" ||
        AC_MSG_ERROR([$ac_sub_configure failed for $ac_dir])

    ac_msg="=== end of configure in $ac_dir (`pwd`/$ac_dir)"
    _AS_ECHO_LOG([$ac_msg])
    _AS_ECHO([$ac_msg])
    cd "$ac_popdir"
  fi
])

# Dummy AC_CONFIG_SUBDIRS for autoreconf tracing
AC_DEFUN([AC_CONFIG_SUBDIRS], [])
