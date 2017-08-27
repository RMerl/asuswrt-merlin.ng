include_directories(
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_BINARY_DIR}/src
  ${CMAKE_SOURCE_DIR}/src/libical
  ${CMAKE_BINARY_DIR}/src/libical
  ${CMAKE_SOURCE_DIR}/src/libicalss
)

########### doesnothing target ###############

set(doesnothing_SRCS
  access_components.c
  access_properties_and_parameters.c
  errors.c
  main.c
  parse_text.c
)

add_executable(doesnothing ${doesnothing_SRCS})
add_dependencies(doesnothing ical icalss icalvcal)

if(NOT STATIC_ONLY)
  target_link_libraries(doesnothing ical icalss icalvcal)
else()
  if(NOT WIN32)
    target_link_libraries(doesnothing
      ${CMAKE_BINARY_DIR}/lib/libical.a
      ${CMAKE_BINARY_DIR}/lib/libicalss.a
      ${CMAKE_BINARY_DIR}/lib/libicalvcal.a
    )
  else()
    target_link_libraries(doesnothing
      ${CMAKE_BINARY_DIR}/bin/libical-static.lib
      ${CMAKE_BINARY_DIR}/bin/libicalss-static.lib
      ${CMAKE_BINARY_DIR}/bin/libicalvcal-static.lib
    )
  endif()
  target_link_libraries(doesnothing ${CMAKE_THREAD_LIBS_INIT})
  if(ICU_FOUND)
    target_link_libraries(doesnothing ${ICU_LIBRARY})
  endif()
  if(ICU_I18N_FOUND)
    target_link_libraries(doesnothing ${ICU_I18N_LIBRARY})
  endif()
  if(WITH_BDB)
    target_link_libraries(doesnothing ${BDB_LIBRARY})
  endif()
endif()
