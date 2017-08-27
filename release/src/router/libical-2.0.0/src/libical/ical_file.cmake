# ORDERING OF HEADERS IS SIGNIFICANT. Don't change this ordering.
# It is required to make the combined header ical.h properly.
set(COMBINEDHEADERSICAL
  ${TOPB}/src/libical/icalversion.h
  ${TOPS}/src/libical/icaltime.h
  ${TOPS}/src/libical/icalduration.h
  ${TOPS}/src/libical/icalperiod.h
  ${TOPS}/src/libical/icalenums.h
  ${TOPS}/src/libical/icaltypes.h
  ${TOPS}/src/libical/icalarray.h
  ${TOPS}/src/libical/icalrecur.h
  ${TOPS}/src/libical/icalattach.h
  ${TOPB}/src/libical/icalderivedvalue.h
  ${TOPB}/src/libical/icalderivedparameter.h
  ${TOPS}/src/libical/icalvalue.h
  ${TOPS}/src/libical/icalparameter.h
  ${TOPB}/src/libical/icalderivedproperty.h
  ${TOPS}/src/libical/icalproperty.h
  ${TOPS}/src/libical/pvl.h
  ${TOPS}/src/libical/icalcomponent.h
  ${TOPS}/src/libical/icaltimezone.h
  ${TOPS}/src/libical/icaltz-util.h
  ${TOPS}/src/libical/icalparser.h
  ${TOPS}/src/libical/icalmemory.h
  ${TOPS}/src/libical/icalerror.h
  ${TOPS}/src/libical/icalrestriction.h
  ${TOPS}/src/libical/sspm.h
  ${TOPS}/src/libical/icalmime.h
  ${TOPS}/src/libical/icallangbind.h
)

file(WRITE ${ICAL_FILE_H_FILE} "#ifndef LIBICAL_ICAL_H\n")
file(APPEND ${ICAL_FILE_H_FILE} "#define LIBICAL_ICAL_H\n")
file(APPEND ${ICAL_FILE_H_FILE} "#ifndef S_SPLINT_S\n")
file(APPEND ${ICAL_FILE_H_FILE} "#ifdef __cplusplus\n")
file(APPEND ${ICAL_FILE_H_FILE} "extern \"C\" {\n")
file(APPEND ${ICAL_FILE_H_FILE} "#endif\n")

foreach(_current_FILE ${COMBINEDHEADERSICAL})
  file(STRINGS ${_current_FILE} _lines)
  foreach(_currentLINE ${_lines})
    string(REGEX REPLACE "#include \"ical.*\\.h\"" "" _currentLINE "${_currentLINE}")
    string(REGEX REPLACE "#include \"config.*\\.h\"" "" _currentLINE "${_currentLINE}")
    string(REGEX REPLACE "#include \"pvl\\.h\"" "" _currentLINE "${_currentLINE}" )
    file(APPEND ${ICAL_FILE_H_FILE} "${_currentLINE}\n")
  endforeach()
endforeach()

file(APPEND ${ICAL_FILE_H_FILE} "\n")
file(APPEND ${ICAL_FILE_H_FILE} "#ifdef __cplusplus\n")
file(APPEND ${ICAL_FILE_H_FILE} "}\n")
file(APPEND ${ICAL_FILE_H_FILE} "#endif\n")
file(APPEND ${ICAL_FILE_H_FILE} "#endif\n")
file(APPEND ${ICAL_FILE_H_FILE} "#endif\n")
