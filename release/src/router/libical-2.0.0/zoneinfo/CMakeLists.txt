
########### install files ###############

install(FILES zones.tab DESTINATION ${SHARE_INSTALL_DIR}/libical/zoneinfo)

set(zoneinfodir
  Africa
  America
  America/North_Dakota
  America/Indiana
  America/Kentucky
  America/Argentina
  Antarctica
  Arctic
  Asia
  Atlantic
  Australia
  Europe
  Indian
  Pacific
)

foreach(_zoneinfocurrentdir ${zoneinfodir})
  file(GLOB _icsfile "${_zoneinfocurrentdir}/*.ics")
  install(
    FILES ${_icsfile}
    DESTINATION ${SHARE_INSTALL_DIR}/libical/zoneinfo/${_zoneinfocurrentdir}/
  )
endforeach()
