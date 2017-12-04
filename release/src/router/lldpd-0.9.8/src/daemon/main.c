/* -*- mode: c; c-file-style: "openbsd" -*- */
#include "lldpd.h"

/**
 * @mainpage
 *
 * lldpd is an implementation of 802.1AB (aka LLDP). It provides an interface
 * for third party clients to interact with it: querying neighbors, setting some
 * TLV. This interface is included into a library whose API can be found in @ref
 * liblldpctl
 */

int
main(int argc, char **argv, char **envp)
{
	return lldpd_main(argc, argv, envp);
}
