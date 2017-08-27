# ORDERING OF HEADERS IS SIGNIFICANT. Don't change this ordering.
# It is required to make the combined header icalss.h properly.
set(COMBINEDHEADERSICALSS
  ${TOPS}/src/libicalss/icalgauge.h
  ${TOPS}/src/libicalss/icalset.h
  ${TOPS}/src/libicalss/icalcluster.h
  ${TOPS}/src/libicalss/icalfileset.h
  ${TOPS}/src/libicalss/icaldirset.h
  ${TOPS}/src/libicalss/icalcalendar.h
  ${TOPS}/src/libicalss/icalclassify.h
  ${TOPS}/src/libicalss/icalspanlist.h
  ${TOPS}/src/libicalss/icalmessage.h
)
if(WITH_BDB)
  list(APPEND COMBINEDHEADERSICALSS ${TOPS}/src/libicalss/icalbdbset.h)
endif()

file(WRITE ${ICAL_FILE_H_FILE} "#ifndef LIBICAL_ICALSS_H\n")
file(APPEND ${ICAL_FILE_H_FILE} "#define LIBICAL_ICALSS_H\n")
file(APPEND ${ICAL_FILE_H_FILE} "#ifndef S_SPLINT_S\n")
file(APPEND ${ICAL_FILE_H_FILE} "#ifdef __cplusplus\n")
file(APPEND ${ICAL_FILE_H_FILE} "extern \"C\" {\n")
file(APPEND ${ICAL_FILE_H_FILE} "#endif\n")

foreach(_current_FILE ${COMBINEDHEADERSICALSS})
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
