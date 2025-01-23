## w32-lock-obj-pub.in - Include fragment for gpg-error.h       -*- c-*-
## Copyright (C) 2014 g10 Code GmbH
##
## This file is free software; as a special exception the author gives
## unlimited permission to copy and/or distribute it, with or without
## modifications, as long as this notice is preserved.
##
## This file is distributed in the hope that it will be useful, but
## WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
## implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
##
##
## This file defines the public version of the lock object for 32 bit
## Windows.  The actual used version is in w32-lock-obj.h.  This file
## is inserted into gpg-error.h by mkheader.c.  The tool
## gen-w32-lock-obj.c has been used to construct it.

#ifdef _WIN64

#pragma pack(push, 8)
typedef struct
{
  volatile unsigned char priv[56];
} gpgrt_lock_t;
#pragma pack(pop)

#define GPGRT_LOCK_INITIALIZER {{1,0,0,0,0,0,0,0,255,255,255,255, \
                                 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, \
                                 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, \
                                 0,0,0,0,0,0,0,0,0,0,0,0}}

#else

#pragma pack(push, 8)
typedef struct
{
  volatile unsigned char priv[36];
} gpgrt_lock_t;
#pragma pack(pop)

#define GPGRT_LOCK_INITIALIZER {{1,0,0,0,0,0,0,0,255,255,255,255, \
                                 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, \
                                 0,0,0,0,0,0,0,0}}
#endif
