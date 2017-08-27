
if(OPENSSL_FOUND)

	find_program(OPENSSL_EXECUTABLE openssl openssl.exe bin/openssl.exe
		HINTS ${_OPENSSL_ROOT_HINTS}
		PATH 
			/usr/bin/ 
			bin/
	  	DOC "Openssl executable")

	mark_as_advanced(OPENSSL_EXECUTABLE)
	
	# On Windows, we need to copy the OpenSSL dlls 
	# to the output directory.
	if(WIN32)
		set(OPENSSL_BIN_FOUND 0)

		find_file(LIBEAY_BIN
			NAMES
			libeay32.dll
			HINTS
			${_OPENSSL_ROOT_HINTS}
			PATH_SUFFIXES
			bin)
		
		find_file(SSLEAY_BIN
			NAMES
			ssleay32.dll
			HINTS
			${_OPENSSL_ROOT_HINTS}
			PATH_SUFFIXES
			bin)
		
		if(LIBEAY_BIN)
			if(SSLEAY_BIN)
				set(OPENSSL_BIN_FOUND 1)
			endif(SSLEAY_BIN)
		endif(LIBEAY_BIN)
	endif(WIN32)
		
endif(OPENSSL_FOUND)

