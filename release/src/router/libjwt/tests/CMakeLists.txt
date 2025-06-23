find_package (Check REQUIRED)

enable_testing ()

# Add the check target to behave like automake
add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND})

set (TARGET_NAMES jwt_dump jwt_ec jwt_encode jwt_grant jwt_header jwt_new jwt_rsa jwt_validate)

if (UNIX)
	set (PLATFORM_LIBRARIES pthread)
endif ()

foreach (TARGET_NAME ${TARGET_NAMES})
	add_executable (${TARGET_NAME} ${TARGET_NAME}.c)
	target_include_directories (${TARGET_NAME} PRIVATE ${CHECK_INCLUDE_DIR})
	if (UNIX AND ENABLE_LTO)
		set_property(TARGET ${TARGET_NAME} PROPERTY INTERPROCEDURAL_OPTIMIZATION True)
	endif ()
	target_compile_definitions (${TARGET_NAME} PRIVATE KEYDIR=\"${CMAKE_CURRENT_SOURCE_DIR}/keys\" _GNU_SOURCE)
	target_link_libraries (${TARGET_NAME} jwt ${CHECK_LIBRARIES} ${PLATFORM_LIBRARIES})
	add_test (${TARGET_NAME} ${TARGET_NAME})
endforeach ()
