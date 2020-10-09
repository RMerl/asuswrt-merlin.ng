
#pragma once

/*
 * Fall back to Linux's definitions of makedev and major are needed.
 * The search_sysfs_block() function is highly unlikely to work on
 * non-Linux systems anyway.
 */
#ifndef makedev
#define makedev(maj, min) (((maj) << 8) + (min))
#endif