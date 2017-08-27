/*
 * Copyright (C) 2012 Reto Buerki
 * Copyright (C) 2012 Adrian-Ken Rueegsegger
 * Hochschule fuer Technik Rapperswil
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

/**
 * @defgroup tkm tkm
 *
 * @addtogroup tkm
 * @{
 *
 * Untrusted IKEv2 component used with Trusted Key Manager for IKE
 * disaggregation.
 *
 * The untrusted IKEv2 component used in conjunction with the Trusted Key
 * Manager infrastructure is implemented as a separate charon instance located
 * in its own directory below the strongSwan top-level source directory
 * (src/charon-tkm). This has the advantage that the TKM code is contained and
 * does not mix with other strongSwan files. The charon-tkm binary startup code
 * is modeled after the charon-nm instance, a special charon daemon variant to
 * be used with the GNOME NetworkManager project. The major difference is the
 * registration of custom TKM plugins as the final step of the startup phase.
 * The charon-tkm daemon does not rely on the dynamic plugin loading mechanism
 * for its core plugins, they are statically registered before entering the main
 * processing loop.
 *
 * The following diagram shows the main components of the system and how they
 * communicate.
   @verbatim

       +------------+            +------------+             +------------+
       | xfrm-proxy |<-[tkm-rpc->| charon-tkm |<-[tkm-rpc]->|    TKM     |
       +------------+            +------------+             +------------+
             ^                                                    ^
    [Netlink | XFRM]                                        [XFRM | Netlink]
             |                                                    v
       +-----------------------------------------------------------------+
       |                            Kernel                               |
       +-----------------------------------------------------------------+

   @endverbatim
 * Since the charon-tkm code uses the tkm-rpc library written in Ada, the daemon
 * has to be built using an Ada-aware toolchain. The integration of Ada code
 * into the strongSwan codebase is explained in the TKM documentation, section
 * 5.4.1: http://www.codelabs.ch/tkm#anchor-doc.
 *
 * The Trusted Key Manager (TKM) is a minimal Trusted Computing Base which
 * implements security-critical functions of the IKEv2 protocol.
 *
 * The xfrm-proxy receives XFRM Acquire and Expiry events from the kernel and
 * forwards them to the charon-tkm IKE daemon for further processing.
 *
 * The underlying concept of IKE disaggregation and the design of TKM and all
 * related components, of which charon-tkm is one component, is presented in
 * detail in the project documentation found at
 * http://www.codelabs.ch/tkm#anchor-doc.
 */

#ifndef TKM_H_
#define TKM_H_

#include "tkm_id_manager.h"
#include "tkm_chunk_map.h"

typedef struct tkm_t tkm_t;

/**
 * Trusted key manager context, contains tkm related globals.
 */
struct tkm_t {

	/**
	 * Context ID manager.
	 */
	tkm_id_manager_t *idmgr;

	/**
	 * Chunk-to-ID mappings.
	 */
	tkm_chunk_map_t *chunk_map;

};

/**
 * Initialize trusted key manager, creates "tkm" instance.
 *
 * @return				FALSE if initialization error occurred
 */
bool tkm_init();

/**
 * Deinitialize trusted key manager, destroys "tkm" instance.
 */
void tkm_deinit();

/**
 * Trusted key manager instance, set after tkm_init() and before tkm_deinit()
 * calls.
 */
extern tkm_t *tkm;

#endif /** TKM_H_ @}*/
