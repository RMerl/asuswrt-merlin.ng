#
# lldp_CHECK_XML2
#


AC_DEFUN([lldp_CHECK_XML2], [
  if test x"$with_xml" != x"no"; then
   PKG_CHECK_MODULES([XML2], [libxml-2.0], [
    dnl Found through pkg-config
    AC_DEFINE_UNQUOTED([USE_XML], 1, [Define to indicate to enable XML support])
    with_xml=yes
   ],[
    dnl Fallback to xml2-config
    AC_PATH_TOOL([XML2_CONFIG], [xml2-config], [no])
    if test x"$XML2_CONFIG" = x"no"; then
      dnl No luck
      if test x"$with_xml" = x"yes"; then
         AC_MSG_FAILURE([*** no libxml2 support found])
      fi
      with_xml=no
    else
      dnl Check that it's working as expected
      XML2_LIBS=`${XML2_CONFIG} --libs`
      XML2_CFLAGS=`${XML2_CONFIG} --cflags`

      _save_flags="$CFLAGS"
      _save_libs="$LIBS"
      CFLAGS="$CFLAGS ${XML2_CFLAGS}"
      LIBS="$LIBS ${XML2_LIBS}"
      AC_MSG_CHECKING([whether libxml-2 work as expected])
      AC_LINK_IFELSE([AC_LANG_PROGRAM([
@%:@include <libxml/encoding.h>
@%:@include <libxml/xmlwriter.h>
],[
	xmlDocPtr doc;
	xmlTextWriterPtr xw = xmlNewTextWriterDoc(&doc, 0);
        return (xw != NULL);
])],[
        AC_MSG_RESULT(yes)
        AC_SUBST([XML2_LIBS])
        AC_SUBST([XML2_CFLAGS])
        AC_DEFINE_UNQUOTED([USE_XML], 1, [Define to indicate to enable XML support])
        with_xml=yes
      ],[
        AC_MSG_RESULT(no)
        if test x"$with_xml" = x"yes"; then
            AC_MSG_FAILURE([*** libxml2 not working as expected])
        fi
        with_xml=no
      ])
      CFLAGS="$_save_flags"
      LIBS="$_save_libs"
    fi
   ])
  fi
])
