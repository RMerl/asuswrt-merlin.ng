Build Instructions for CMake
============================

1. Generate build tree.

        mkdir out
        cd out
        cmake -G "Ninja" -DBUILD_TESTS=1 ..

    NOTE: If you prefer, substitute "Unix Makefiles" or other cmake build generator.

2. Build all.

        ninja

    NOTE: If using different build tool, `cmake --build .` will work.

3. Run tests.

        ninja check

    NOTE: If using different build tool, `cmake --build . --target check` will work.

Examples
--------

Two example utilities are provided to demonstrate generating JWT and for
authenticating JWT, jwtgen and jwtauth.

1. Generate a token using RS256 signature with the sample private key.

        cd out/examples
        ./jwtgen -k ../../tests/keys/rsa_key_2048.pem -a RS256 -c iss=example.com -c sub=user0 > user0.jwt

2. Authenticate a token using RS256 signature with the sample public key.

        cd out/examples
        ./jwtauth -k ../../tests/keys/rsa_key_2048-pub.pem --alg RS256 user0.jwt

3. Authenticate a token using RS256 signature with the sample public key, verifying user is user1.

        cd out/examples
        ./jwtauth -k ../../tests/keys/rsa_key_2048-pub.pem --alg RS256 -c sub=user1 user0.jwt

