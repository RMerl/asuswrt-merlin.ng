#find_package(LibJWT)

add_definitions(-D_GNU_SOURCE)

add_executable(jwtauth
	main-auth.c
)

target_compile_options(jwtauth PRIVATE -Werror -Wall -Wextra)
target_compile_options(jwtauth PRIVATE -g -fPIC -O2 -funwind-tables)

set_property(TARGET jwtauth PROPERTY C_STANDARD 11)

target_link_libraries(jwtauth ${PROJECT_NAME})
#target_link_libraries(jwtauth ${LIBJWT_LIBRARIES})

#target_include_directories(jwtauth
#	PUBLIC ${LIBJWT_INCLUDE_DIRS}}
#)

add_executable(jwtgen
	main-gen.c
)

target_compile_options(jwtgen PRIVATE -Werror -Wall -Wextra)
target_compile_options(jwtgen PRIVATE -g -fPIC -O2 -funwind-tables)

set_property(TARGET jwtgen PROPERTY C_STANDARD 11)

target_link_libraries(jwtgen ${PROJECT_NAME})
#target_link_libraries(jwtgen ${LIBJWT_LIBRARIES})

#target_include_directories(jwtgen
#	PUBLIC ${LIBJWT_INCLUDE_DIRS}
#)

