/*
 *   This program is is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License, version 2 if the
 *   License as published by the Free Software Foundation.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

/**
 * $Id$
 * @file rlm_soh.c
 * @brief Decodes Microsoft's Statement of Health sub-protocol.
 *
 * @copyright 2010 Phil Mayers <p.mayers@imperial.ac.uk>
 */
RCSID("$Id$")

#include	<freeradius-devel/radiusd.h>
#include	<freeradius-devel/modules.h>
#include	<freeradius-devel/dhcp.h>
#include	<freeradius-devel/soh.h>


typedef struct rlm_soh_t {
	char const *xlat_name;
	int dhcp;
} rlm_soh_t;


/*
 * Not sure how to make this useful yet...
 */
static ssize_t soh_xlat(UNUSED void *instance, REQUEST *request, char const *fmt, char *out, size_t outlen) {

	VALUE_PAIR* vp[6];
	char const *osname;

	/* there will be no point unless SoH-Supported = yes
	 *
	 * FIXME: should have a #define for the attribute...
	 * SoH-Supported == 2119 in dictionary.freeradius.internal
	 */
	vp[0] = pairfind(request->packet->vps, 2119, 0, TAG_ANY);
	if (!vp[0])
		return 0;


	if (strncasecmp(fmt, "OS", 2) == 0) {
		/* OS vendor */
		vp[0] = pairfind(request->packet->vps, 2100, 0, TAG_ANY);
		vp[1] = pairfind(request->packet->vps, 2101, 0, TAG_ANY);
		vp[2] = pairfind(request->packet->vps, 2102, 0, TAG_ANY);
		vp[3] = pairfind(request->packet->vps, 2103, 0, TAG_ANY);
		vp[4] = pairfind(request->packet->vps, 2104, 0, TAG_ANY);
		vp[5] = pairfind(request->packet->vps, 2105, 0, TAG_ANY);

		if (vp[0] && vp[0]->vp_integer == VENDORPEC_MICROSOFT) {
			if (!vp[1]) {
				snprintf(out, outlen, "Windows unknown");
			} else {
				switch (vp[1]->vp_integer) {
					case 7:
						osname = "7";
						break;
					case 6:
						osname = "Vista";
						break;
					case 5:
						osname = "XP";
						break;
					default:
						osname = "Other";
						break;
				}
				snprintf(out, outlen, "Windows %s %d.%d.%d sp %d.%d", osname, vp[1]->vp_integer,
						vp[2] ? vp[2]->vp_integer : 0,
						vp[3] ? vp[3]->vp_integer : 0,
						vp[4] ? vp[4]->vp_integer : 0,
						vp[5] ? vp[5]->vp_integer : 0
					);
			}
			return strlen(out);
		}
	}

	return 0;
}


static const CONF_PARSER module_config[] = {
	/*
	 * Do SoH over DHCP?
	 */
	{ "dhcp",    PW_TYPE_BOOLEAN, offsetof(rlm_soh_t,dhcp), NULL, "no" },

	{ NULL, -1, 0, NULL, NULL }		/* end the list */
};


static int mod_instantiate(CONF_SECTION *conf, void *instance)
{
	char const *name;
	rlm_soh_t *inst = instance;

	name = cf_section_name2(conf);
	if (!name) name = cf_section_name1(conf);
	inst->xlat_name = name;
	if (!inst->xlat_name) return -1;
	xlat_register(inst->xlat_name, soh_xlat, NULL, inst);

	return 0;
}

static rlm_rcode_t mod_post_auth(UNUSED void * instance, REQUEST *request)
{
#ifdef WITH_DHCP
	int rcode;
	VALUE_PAIR *vp;

	vp = pairfind(request->packet->vps, 43, DHCP_MAGIC_VENDOR, TAG_ANY);
	if (vp) {
		/*
		 * vendor-specific options contain
		 *
		 * vendor opt 220/0xdc - SoH payload, or null byte to probe, or string
		 * "NAP" to indicate server-side support for SoH in OFFERs
		 *
		 * vendor opt 222/0xde - SoH correlation ID as utf-16 string, yuck...
		 */
		uint8_t vopt, vlen;
		uint8_t const *data;

		data = vp->vp_octets;
		while (data < vp->vp_octets + vp->length) {
			vopt = *data++;
			vlen = *data++;
			switch (vopt) {
				case 220:
					if (vlen <= 1) {
						uint8_t *p;

						RDEBUG("SoH adding NAP marker to DHCP reply");
						/* client probe; send "NAP" in the reply */
						vp = paircreate(request->reply, 43, DHCP_MAGIC_VENDOR);
						vp->length = 5;
						vp->vp_octets = p = talloc_array(vp, uint8_t, vp->length);

						p[0] = 220;
						p[1] = 3;
						p[4] = 'N';
						p[3] = 'A';
						p[2] = 'P';

						pairadd(&request->reply->vps, vp);

					} else {
						RDEBUG("SoH decoding NAP from DHCP request");
						/* SoH payload */
						rcode = soh_verify(request, data, vlen);
						if (rcode < 0) {
							return RLM_MODULE_FAIL;
						}
					}
					break;
				default:
					/* nothing to do */
					break;
			}
			data += vlen;
		}
		return RLM_MODULE_OK;
	}
#endif
	return RLM_MODULE_NOOP;
}

static rlm_rcode_t mod_authorize(UNUSED void * instance, REQUEST *request)
{
	VALUE_PAIR *vp;
	int rv;

	/* try to find the MS-SoH payload */
	vp = pairfind(request->packet->vps, 55, VENDORPEC_MICROSOFT, TAG_ANY);
	if (!vp) {
		RDEBUG("SoH radius VP not found");
		return RLM_MODULE_NOOP;
	}

	RDEBUG("SoH radius VP found");
	/* decode it */
	rv = soh_verify(request, vp->vp_octets, vp->length);
	if (rv < 0) {
		return RLM_MODULE_FAIL;
	}

	return RLM_MODULE_OK;
}

module_t rlm_soh = {
	RLM_MODULE_INIT,
	"SoH",
	RLM_TYPE_THREAD_SAFE,		/* type */
	sizeof(rlm_soh_t),
	module_config,
	mod_instantiate,		/* instantiation */
	NULL,			/* detach */
	{
		NULL,			/* authenticate */
		mod_authorize,		/* authorize */
		NULL,			/* pre-accounting */
		NULL,			/* accounting */
		NULL,			/* checksimul */
		NULL,			/* pre-proxy */
		NULL,			/* post-proxy */
		mod_post_auth		/* post-auth */
	},
};
