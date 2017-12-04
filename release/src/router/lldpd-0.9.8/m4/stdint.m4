#
# lldp_CHECK_STDINT
#
AC_DEFUN([lldp_CHECK_STDINT], [
  AC_CHECK_TYPES([u_int32_t, uint32_t])
  if test "_$ac_cv_type_uint32_t" = _yes; then
  if test "_$ac_cv_type_u_int32_t" = _no; then
    AC_DEFINE(u_int8_t, uint8_t, [Compatibility with Linux u_int8_t])
    AC_DEFINE(u_int16_t, uint16_t, [Compatibility with Linux u_int16_t])
    AC_DEFINE(u_int32_t, uint32_t, [Compatibility with Linux u_int32_t])
    AC_DEFINE(u_int64_t, uint64_t, [Compatibility with Linux u_int64_t])
  fi
  fi])
