## our modules are without the "lib" prefix

macro(ADD_AND_INSTALL_LIBRARY LIBNAME SRCFILES)
	if(BUILD_STATIC)
		add_library(${LIBNAME} STATIC ${SRCFILES})
		target_link_libraries(lighttpd ${LIBNAME})
	else()
		if(APPLE)
			add_library(${LIBNAME} MODULE ${SRCFILES})
		else()
			add_library(${LIBNAME} SHARED ${SRCFILES})
		endif()
		set(L_INSTALL_TARGETS ${L_INSTALL_TARGETS} ${LIBNAME})
		## Windows likes to link it this way back to app!
		if(WIN32)
			set_target_properties(${LIBNAME} PROPERTIES LINK_FLAGS lighttpd.lib)
		endif()

		if(APPLE)
			set_target_properties(${LIBNAME} PROPERTIES LINK_FLAGS "-flat_namespace -undefined suppress")
		endif()
	endif()
endmacro(ADD_AND_INSTALL_LIBRARY)

macro(LEMON_PARSER SRCFILE)
	get_filename_component(SRCBASE ${SRCFILE} NAME_WE)
	add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${SRCBASE}.c ${CMAKE_CURRENT_BINARY_DIR}/${SRCBASE}.h
		COMMAND ${CMAKE_BINARY_DIR}/build/lemon
		ARGS -q ${CMAKE_CURRENT_SOURCE_DIR}/${SRCFILE} ${CMAKE_SOURCE_DIR}/src/lempar.c
		DEPENDS ${CMAKE_BINARY_DIR}/build/lemon ${CMAKE_CURRENT_SOURCE_DIR}/${SRCFILE} ${CMAKE_SOURCE_DIR}/src/lempar.c
		COMMENT "Generating ${SRCBASE}.c from ${SRCFILE}"
	)
endmacro(LEMON_PARSER)

macro(ADD_TARGET_PROPERTIES _target _name)
	set(_properties)
	foreach(_prop ${ARGN})
		set(_properties "${_properties} ${_prop}")
	endforeach()
	get_target_property(_old_properties ${_target} ${_name})
	message("adding property to ${_target} ${_name}:" ${_properties})
	if(NOT _old_properties)
		# in case it's NOTFOUND
		set(_old_properties)
	endif()
	set_target_properties(${_target} PROPERTIES ${_name} "${_old_properties} ${_properties}")
endmacro(ADD_TARGET_PROPERTIES)
