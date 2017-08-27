AC_DEFUN([SAVE_LIBRARY_VERSION], [
AC_MSG_CHECKING(for library version)
SND_LIB_VERSION=$VERSION
echo $VERSION > $srcdir/version
AC_DEFINE_UNQUOTED(VERSION, "$SND_LIB_VERSION", [sound library version string])
AC_SUBST(SND_LIB_VERSION)
SND_LIB_MAJOR=`echo $VERSION | cut -d . -f 1`
AC_SUBST(SND_LIB_MAJOR)
SND_LIB_MINOR=`echo $VERSION | cut -d . -f 2`
AC_SUBST(SND_LIB_MINOR)
SND_LIB_SUBMINOR=`echo $VERSION | cut -d . -f 3 | sed -e 's/^\([[^[:alpha:]]]*\)\(.*\)$/\1/g'`
AC_SUBST(SND_LIB_SUBMINOR)
SND_LIB_EXTRASTR=`echo $VERSION | cut -d . -f 3 | sed -e 's/^\([[^[:alpha:]]]*\)\([[[:alpha:]]]*\)\([[[:digit:]]]*\)\(.*\)$/\2/g'`
SND_LIB_EXTRAVER=`echo $VERSION | cut -d . -f 3 | sed -e 's/^\([[^[:alpha:]]]*\)\([[[:alpha:]]]*\)\([[[:digit:]]]*\)\(.*\)$/\3/g'`
case "$SND_LIB_EXTRASTR" in
  pre)   SND_LIB_EXTRAVER=`expr $SND_LIB_EXTRAVER + 00000` ;;
  alpha) SND_LIB_EXTRAVER=`expr $SND_LIB_EXTRAVER + 10000` ;;
  beta)  SND_LIB_EXTRAVER=`expr $SND_LIB_EXTRAVER + 20000` ;;
  rc)	 SND_LIB_EXTRAVER=`expr $SND_LIB_EXTRAVER + 100000` ;;
  *)     SND_LIB_EXTRAVER=1000000 ;;
esac
AC_MSG_RESULT(major $SND_LIB_MAJOR minor $SND_LIB_MINOR subminor $SND_LIB_SUBMINOR extrastr $SND_LIB_EXTRASTR extraver $SND_LIB_EXTRAVER)
AC_SUBST(SND_LIB_EXTRAVER)
])
