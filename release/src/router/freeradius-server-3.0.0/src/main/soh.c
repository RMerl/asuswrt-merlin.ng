/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
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

/*
 * $Id$
 *
 * @file soh.c
 * @brief Implements the MS-SOH parsing code. This is called from rlm_eap_peap
 *
 * @copyright 2010 Phil Mayers <p.mayers@imperial.ac.uk>
 */

RCSID("$Id$")

#include <freeradius-devel/radiusd.h>
#include <freeradius-devel/soh.h>

/*
 * This code implements parsing of MS-SOH data into FreeRadius AVPs
 * allowing for FreeRadius MS-NAP policies
 */

/**
 * EAP-SOH packet
 */
typedef struct {
	uint16_t tlv_type;	/**< ==7 for EAP-SOH */
	uint16_t tlv_len;
	uint32_t tlv_vendor;

	/**
	 * @name soh-payload
	 * @brief either an soh request or response */
	uint16_t soh_type;	/**< ==2 for request, 1 for response */
	uint16_t soh_len;

	/* an soh-response may now follow... */
} eap_soh;

/**
 * SOH response payload
 * Send by client to server
 */
typedef struct {
	uint16_t outer_type;
	uint16_t outer_len;
	uint32_t vendor;
	uint16_t inner_type;
	uint16_t inner_len;
} soh_response;

/**
 * SOH mode subheader
 * Typical microsoft binary blob nonsense
 */
typedef struct {
	uint16_t outer_type;
	uint16_t outer_len;
	uint32_t vendor;
	uint8_t corrid[24];
	uint8_t intent;
	uint8_t content_type;
} soh_mode_subheader;

/**
 * SOH type-length-value header
 */
typedef struct {
	uint16_t tlv_type;
	uint16_t tlv_len;
} soh_tlv;

/**
 * @brief read big-endian 2-byte unsigned from p
 *
 * caller must ensure enough data exists at "p"
 */
uint16_t soh_pull_be_16(uint8_t const *p) {
	uint16_t r;

	r = *p++ << 8;
	r += *p++;

	return r;
}
/**
 * @brief read big-endian 3-byte unsigned from p
 *
 * caller must ensure enough data exists at "p"
 */
uint32_t soh_pull_be_24(uint8_t const *p) {
	uint32_t r;

	r = *p++ << 16;
	r += *p++ << 8;
	r += *p++;

	return r;
}
/**
 * @brief read big-endian 4-byte unsigned from p
 *
 * caller must ensure enough data exists at "p"
 */
uint32_t soh_pull_be_32(uint8_t const *p) {
	uint32_t r;

	r = *p++ << 24;
	r += *p++ << 16;
	r += *p++ << 8;
	r += *p++;

	return r;
}

/**
 * @brief Parses the MS-SOH type/value (note: NOT type/length/value) data and
 * 	update the sohvp list
 *
 * See section 2.2.4 of MS-SOH. Because there's no "length" field we CANNOT just skip
 * unknown types; we need to know their length ahead of time. Therefore, we abort
 * if we find an unknown type. Note that sohvp may still have been modified in the
 * failure case.
 *
 * @param request Current request
 * @param p binary blob
 * @param data_len length of blob
 * @return 1 on success, 0 on failure
 */
static int eapsoh_mstlv(REQUEST *request, uint8_t const *p, unsigned int data_len) {
	VALUE_PAIR *vp;
	uint8_t c;
	int t;
	char *q;

	while (data_len > 0) {
		c = *p++;
		data_len--;

		switch (c) {
			case 1:
				/* MS-Machine-Inventory-Packet
				 * MS-SOH section 2.2.4.1
				 */
				if (data_len < 18) {
					RDEBUG("insufficient data for MS-Machine-Inventory-Packet");
					return 0;
				}
				data_len -= 18;

				vp = pairmake_packet("SoH-MS-Machine-OS-vendor", "Microsoft", T_OP_EQ);
				if (!vp) return 0;

				vp = pairmake_packet("SoH-MS-Machine-OS-version", NULL, T_OP_EQ);
				if (!vp) return 0;

				vp->vp_integer = soh_pull_be_32(p); p+=4;

				vp = pairmake_packet("SoH-MS-Machine-OS-release", NULL, T_OP_EQ);
				if (!vp) return 0;

				vp->vp_integer = soh_pull_be_32(p); p+=4;

				vp = pairmake_packet("SoH-MS-Machine-OS-build", NULL, T_OP_EQ);
				if (!vp) return 0;

				vp->vp_integer = soh_pull_be_32(p); p+=4;

				vp = pairmake_packet("SoH-MS-Machine-SP-version", NULL, T_OP_EQ);
				if (!vp) return 0;

				vp->vp_integer = soh_pull_be_16(p); p+=2;

				vp = pairmake_packet("SoH-MS-Machine-SP-release", NULL, T_OP_EQ);
				if (!vp) return 0;

				vp->vp_integer = soh_pull_be_16(p); p+=2;

				vp = pairmake_packet("SoH-MS-Machine-Processor", NULL, T_OP_EQ);
				if (!vp) return 0;

				vp->vp_integer = soh_pull_be_16(p); p+=2;
				break;

			case 2:
				/* MS-Quarantine-State - FIXME: currently unhandled
				 * MS-SOH 2.2.4.1
				 *
				 * 1 byte reserved
				 * 1 byte flags
				 * 8 bytes NT Time field (100-nanosec since 1 Jan 1601)
				 * 2 byte urilen
				 * N bytes uri
				 */
				p += 10;
				t = soh_pull_be_16(p);	/* t == uri len */
				p += 2;
				p += t;
				data_len -= 12 + t;
				break;

			case 3:
				/* MS-Packet-Info
				 * MS-SOH 2.2.4.3
				 */
				RDEBUG3("SoH MS-Packet-Info %s vers=%i", *p & 0x10 ? "request" : "response", *p & 0xf);
				p++;
				data_len--;
				break;

			case 4:
				/* MS-SystemGenerated-Ids - FIXME: currently unhandled
				 * MS-SOH 2.2.4.4
				 *
				 * 2 byte length
				 * N bytes (3 bytes IANA enterprise# + 1 byte component id#)
				 */
				t = soh_pull_be_16(p);
				p += 2;
				p += t;
				data_len -= 2 + t;
				break;

			case 5:
				/* MS-MachineName
				 * MS-SOH 2.2.4.5
				 *
				 * 1 byte namelen
				 * N bytes name
				 */
				t = soh_pull_be_16(p);
				p += 2;

				vp = pairmake_packet("SoH-MS-Machine-Name", NULL, T_OP_EQ);
				if (!vp) return 0;

				vp->vp_strvalue = q = talloc_array(vp, char, t);
				vp->type = VT_DATA;

				memcpy(q, p, t);
				q[t] = 0;

				p += t;
				data_len -= 2 + t;
				break;

			case 6:
				/* MS-CorrelationId
				 * MS-SOH 2.2.4.6
				 *
				 * 24 bytes opaque binary which we might, in future, have
				 * to echo back to the client in a final SoHR
				 */
				vp = pairmake_packet("SoH-MS-Correlation-Id", NULL, T_OP_EQ);
				if (!vp) return 0;

				pairmemcpy(vp, p, 24);
				p += 24;
				data_len -= 24;
				break;

			case 7:
				/* MS-Installed-Shvs - FIXME: currently unhandled
				 * MS-SOH 2.2.4.7
				 *
				 * 2 bytes length
				 * N bytes (3 bytes IANA enterprise# + 1 byte component id#)
				 */
				t = soh_pull_be_16(p);
				p += 2;
				p += t;
				data_len -= 2 + t;
				break;

			case 8:
				/* MS-Machine-Inventory-Ex
				 * MS-SOH 2.2.4.8
				 *
				 * 4 bytes reserved
				 * 1 byte product type (client=1 domain_controller=2 server=3)
				 */
				p += 4;
				vp = pairmake_packet("SoH-MS-Machine-Role", NULL, T_OP_EQ);
				if (!vp) return 0;

				vp->vp_integer = *p;
				p++;
				data_len -= 5;
				break;

			default:
				RDEBUG("SoH Unknown MS TV %i stopping", c);
				return 0;
		}
	}
	return 1;
}
/**
 * @brief Convert windows Health Class status into human-readable
 * 	string. Tedious, really, really tedious...
 */
static char const* clientstatus2str(uint32_t hcstatus) {
	switch (hcstatus) {
		/* this lot should all just be for windows updates */
		case 0xff0005:
			return "wua-ok";
		case 0xff0006:
			return "wua-missing";
		case 0xff0008:
			return "wua-not-started";
		case 0xc0ff000c:
			return "wua-no-wsus-server";
		case 0xc0ff000d:
			return "wua-no-wsus-clientid";
		case 0xc0ff000e:
			return "wua-disabled";
		case 0xc0ff000f:
			return "wua-comm-failure";

		/* these next 3 are for all health-classes */
		case 0xc0ff0002:
			return "not-installed";
		case 0xc0ff0003:
			return "down";
		case 0xc0ff0018:
			return "not-started";
	}
	return NULL;
}

/**
 * @brief convert a Health Class into a string
 */
static char const* healthclass2str(uint8_t hc) {
	switch (hc) {
		case 0:
			return "firewall";
		case 1:
			return "antivirus";
		case 2:
			return "antispyware";
		case 3:
			return "updates";
		case 4:
			return "security-updates";
	}
	return NULL;
}

/**
 * @brief Parse the MS-SOH response in data and update sohvp.
 *
 * Note that sohvp might still have been updated in event of a failure.
 *
 * @param request Current request
 * @param data MS-SOH blob
 * @param data_len length of MS-SOH blob
 *
 * @return 0 on success, -1 on failure
 *
 */
int soh_verify(REQUEST *request, uint8_t const *data, unsigned int data_len) {

	VALUE_PAIR *vp;
	eap_soh hdr;
	soh_response resp;
	soh_mode_subheader mode;
	soh_tlv tlv;
	int curr_shid=-1, curr_shid_c=-1, curr_hc=-1;

	hdr.tlv_type = soh_pull_be_16(data); data += 2;
	hdr.tlv_len = soh_pull_be_16(data); data += 2;
	hdr.tlv_vendor = soh_pull_be_32(data); data += 4;

	if (hdr.tlv_type != 7 || hdr.tlv_vendor != 0x137) {
		RDEBUG("SoH payload is %i %08x not a ms-vendor packet", hdr.tlv_type, hdr.tlv_vendor);
		return -1;
	}

	hdr.soh_type = soh_pull_be_16(data); data += 2;
	hdr.soh_len = soh_pull_be_16(data); data += 2;
	if (hdr.soh_type != 1) {
		RDEBUG("SoH tlv %04x is not a response", hdr.soh_type);
		return -1;
	}

	/* FIXME: check for sufficient data */
	resp.outer_type = soh_pull_be_16(data); data += 2;
	resp.outer_len = soh_pull_be_16(data); data += 2;
	resp.vendor = soh_pull_be_32(data); data += 4;
	resp.inner_type = soh_pull_be_16(data); data += 2;
	resp.inner_len = soh_pull_be_16(data); data += 2;


	if (resp.outer_type!=7 || resp.vendor != 0x137) {
		RDEBUG("SoH response outer type %i/vendor %08x not recognised", resp.outer_type, resp.vendor);
		return -1;
	}
	switch (resp.inner_type) {
		case 1:
			/* no mode sub-header */
			RDEBUG("SoH without mode subheader");
			break;
		case 2:
			mode.outer_type = soh_pull_be_16(data); data += 2;
			mode.outer_len = soh_pull_be_16(data); data += 2;
			mode.vendor = soh_pull_be_32(data); data += 4;
			memcpy(mode.corrid, data, 24); data += 24;
			mode.intent = data[0];
			mode.content_type = data[1];
			data += 2;

			if (mode.outer_type != 7 || mode.vendor != 0x137 || mode.content_type != 0) {
				RDEBUG3("SoH mode subheader outer type %i/vendor %08x/content type %i invalid", mode.outer_type, mode.vendor, mode.content_type);
				return -1;
			}
			RDEBUG3("SoH with mode subheader");
			break;
		default:
			RDEBUG("SoH invalid inner type %i", resp.inner_type);
			return -1;
	}

	/* subtract off the relevant amount of data */
	if (resp.inner_type==2) {
		data_len = resp.inner_len - 34;
	} else {
		data_len = resp.inner_len;
	}

	/* TLV
	 * MS-SOH 2.2.1
	 * See also 2.2.3
	 *
	 * 1 bit mandatory
	 * 1 bit reserved
	 * 14 bits tlv type
	 * 2 bytes tlv length
	 * N bytes payload
	 *
	 */
	while (data_len >= 4) {
		tlv.tlv_type = soh_pull_be_16(data); data += 2;
		tlv.tlv_len = soh_pull_be_16(data); data += 2;

		data_len -= 4;

		switch (tlv.tlv_type) {
			case 2:
				/* System-Health-Id TLV
				 * MS-SOH 2.2.3.1
				 *
				 * 3 bytes IANA/SMI vendor code
				 * 1 byte component (i.e. within vendor, which SoH component
				 */
				curr_shid = soh_pull_be_24(data);
				curr_shid_c = data[3];
				RDEBUG2("SoH System-Health-ID vendor %08x component=%i", curr_shid, curr_shid_c);
				break;

			case 7:
				/* Vendor-Specific packet
				 * MS-SOH 2.2.3.3
				 *
				 * 4 bytes vendor, supposedly ignored by NAP
				 * N bytes payload; for Microsoft component#0 this is the MS TV stuff
				 */
				if (curr_shid==0x137 && curr_shid_c==0) {
					RDEBUG2("SoH MS type-value payload");
					eapsoh_mstlv(request, data + 4, tlv.tlv_len - 4);
				} else {
					RDEBUG2("SoH unhandled vendor-specific TLV %08x/component=%i %i bytes payload", curr_shid, curr_shid_c, tlv.tlv_len);
				}
				break;

			case 8:
				/* Health-Class
				 * MS-SOH 2.2.3.5.6
				 *
				 * 1 byte integer
				 */
				RDEBUG2("SoH Health-Class %i", data[0]);
				curr_hc = data[0];
				break;

			case 9:
				/* Software-Version
				 * MS-SOH 2.2.3.5.7
				 *
				 * 1 byte integer
				 */
				RDEBUG2("SoH Software-Version %i", data[0]);
				break;

			case 11:
				/* Health-Class status
				 * MS-SOH 2.2.3.5.9
				 *
				 * variable data; for the MS System Health vendor, these are 4-byte
				 * integers which are a really, really dumb format:
				 *
				 *  28 bits ignore
				 *  1 bit - 1==product snoozed
				 *  1 bit - 1==microsoft product
				 *  1 bit - 1==product up-to-date
				 *  1 bit - 1==product enabled
				 */
				RDEBUG2("SoH Health-Class-Status - current shid=%08x component=%i", curr_shid, curr_shid_c);

				if (curr_shid==0x137 && curr_shid_c==128) {

					char const *s, *t;
					uint32_t hcstatus = soh_pull_be_32(data);

					RDEBUG2("SoH Health-Class-Status microsoft DWORD=%08x", hcstatus);

					vp = pairmake_packet("SoH-MS-Windows-Health-Status", NULL, T_OP_EQ);
					if (!vp) return 0;

					switch (curr_hc) {
						case 4:
							/* security updates */
							s = "security-updates";
							switch (hcstatus) {
								case 0xff0005:
									pairsprintf(vp, "%s ok all-installed", s);
									break;
								case 0xff0006:
									pairsprintf(vp, "%s warn some-missing", s);
									break;
								case 0xff0008:
									pairsprintf(vp, "%s warn never-started", s);
									break;
								case 0xc0ff000c:
									pairsprintf(vp, "%s error no-wsus-srv", s);
									break;
								case 0xc0ff000d:
									pairsprintf(vp, "%s error no-wsus-clid", s);
									break;
								case 0xc0ff000e:
									pairsprintf(vp, "%s warn wsus-disabled", s);
									break;
								case 0xc0ff000f:
									pairsprintf(vp, "%s error comm-failure", s);
									break;
								case 0xc0ff0010:
									pairsprintf(vp, "%s warn needs-reboot", s);
									break;
								default:
									pairsprintf(vp, "%s error %08x", s, hcstatus);
									break;
							}
							break;

						case 3:
							/* auto updates */
							s = "auto-updates";
							switch (hcstatus) {
								case 1:
									pairsprintf(vp, "%s warn disabled", s);
									break;
								case 2:
									pairsprintf(vp, "%s ok action=check-only", s);
									break;
								case 3:
									pairsprintf(vp, "%s ok action=download", s);
									break;
								case 4:
									pairsprintf(vp, "%s ok action=install", s);
									break;
								case 5:
									pairsprintf(vp, "%s warn unconfigured", s);
									break;
								case 0xc0ff0003:
									pairsprintf(vp, "%s warn service-down", s);
									break;
								case 0xc0ff0018:
									pairsprintf(vp, "%s warn never-started", s);
									break;
								default:
									pairsprintf(vp, "%s error %08x", s, hcstatus);
									break;
							}
							break;

						default:
							/* other - firewall, antivirus, antispyware */
							s = healthclass2str(curr_hc);
							if (s) {
								/* bah. this is vile. stupid microsoft
								 */
								if (hcstatus & 0xff000000) {
									/* top octet non-zero means an error
									 * FIXME: is this always correct? MS-WSH 2.2.8 is unclear
									 */
									t = clientstatus2str(hcstatus);
									if (t) {
										pairsprintf(vp, "%s error %s", s, t);
									} else {
										pairsprintf(vp, "%s error %08x", s, hcstatus);
									}
								} else {
									pairsprintf(vp,
											"%s ok snoozed=%i microsoft=%i up2date=%i enabled=%i",
											s,
											hcstatus & 0x8 ? 1 : 0,
											hcstatus & 0x4 ? 1 : 0,
											hcstatus & 0x2 ? 1 : 0,
											hcstatus & 0x1 ? 1 : 0
											);
								}
							} else {
								pairsprintf(vp, "%i unknown %08x", curr_hc, hcstatus);
							}
							break;
					}
				} else {
					vp = pairmake_packet("SoH-MS-Health-Other", NULL, T_OP_EQ);
					if (!vp) return 0;

					/* FIXME: what to do with the payload? */
					pairsprintf(vp, "%08x/%i ?", curr_shid, curr_shid_c);
				}
				break;

			default:
				RDEBUG("SoH Unknown TLV %i len=%i", tlv.tlv_type, tlv.tlv_len);
				break;
		}

		data += tlv.tlv_len;
		data_len -= tlv.tlv_len;
	}

	return 0;
}
