check_include_files(fastcgi.h HAVE_FASTCGI_H)
check_include_files(fastcgi/fastcgi.h HAVE_FASTCGI_FASTCGI_H)
if(HAVE_FASTCGI_H OR HAVE_FASTCGI_FASTCGI_H)
	check_library_exists(fcgi FCGI_Accept "" HAVE_FASTCGI)
	if(HAVE_FASTCGI)
		add_executable(fcgi-auth fcgi-auth.c)
		add_executable(fcgi-responder fcgi-responder.c)
		target_link_libraries(fcgi-auth fcgi)
		target_link_libraries(fcgi-responder fcgi)
	endif()
endif()

set(T_FILES
	prepare.sh
	cachable.t
	core-404-handler.t
	core-condition.t
	core-keepalive.t
	core-request.t
	core-response.t
	core.t
	core-var-include.t
	lowercase.t
	mod-access.t
	mod-auth.t
	mod-cgi.t
	mod-compress.t
	mod-extforward.t
	mod-fastcgi.t
	mod-proxy.t
	mod-redirect.t
	mod-rewrite.t
	mod-secdownload.t
	mod-setenv.t
	mod-simplevhost.t
	mod-ssi.t
	mod-userdir.t
	request.t
	symlink.t
	cleanup.sh
)

foreach(it ${T_FILES})
	add_test(NAME ${it} COMMAND "${lighttpd_SOURCE_DIR}/tests/wrapper.sh"
		"${lighttpd_SOURCE_DIR}/tests"
		"${lighttpd_BINARY_DIR}"
		"${lighttpd_SOURCE_DIR}/tests/${it}")
endforeach()
