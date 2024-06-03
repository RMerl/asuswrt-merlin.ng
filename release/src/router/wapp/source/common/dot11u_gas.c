/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * Copyright  (C) 2019-2020  MediaTek Inc. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#include <stdlib.h>
#include <stdio.h>
#include "wapp_cmm.h"

int wapp_send_anqp_req(struct wifi_app *wapp, const char *iface, 
				                 const char *peer_sta_addr, 
						         const char *anqp_req, 
					             size_t anqp_req_len)
{
	int ret;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);
	
	ret = wapp->drv_ops->drv_send_anqp_req(wapp->drv_data, iface, peer_sta_addr, 
								         anqp_req, anqp_req_len);
	return ret;

}

int wapp_send_anqp_rsp(struct wifi_app *wapp, const char *iface, 
				          const u8 *peer_mac_addr, 
				          const char *anqp_rsp, 
				          size_t anqp_rsp_len)
{
	int ret;
	
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __FUNCTION__);
	
	ret = wapp->drv_ops->drv_send_anqp_rsp(wapp->drv_data, iface, peer_mac_addr,
									     anqp_rsp, anqp_rsp_len);

	return ret;
}

/* calculate ANQP response packet len */
static size_t wapp_calc_anqp_rsp_len(struct wifi_app *wapp,
								 struct wapp_conf *conf,
								 size_t anqp_req_len,
								 const char *curpos)
{
	size_t varlen = 0, curlen = 0;
	u16 info_id, tmpbuf;
	struct anqp_capability *capability_info;
	struct oi_duple *oiduple;
	struct domain_name_field *dname_field;
	struct venue_name_duple *vname_duple;
	struct nai_realm_data *realm_data;
	struct plmn *plmn_unit;
	struct net_auth_type_unit *auth_type_unit;
	struct anqp_hs_capability *hs_capability_subtype;
	struct advice_of_charge_data *charge_data;
	struct aoc_plan_tuple_data *plan_tuple_data;
	struct venue_url_duple *vurl_duple;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	while (curlen < anqp_req_len) {
		os_memcpy(&tmpbuf, curpos, 2);
		info_id = le2cpu16(tmpbuf);
		switch(info_id) {
			case ANQP_CAPABILITY:
				if (!conf->query_anqp_capability_list) {
					conf->query_anqp_capability_list = 1;

					if (conf->have_anqp_capability_list) {
						varlen += 4;
						dl_list_for_each(capability_info, &conf->anqp_capability_list,
													struct anqp_capability, list) {
							varlen += 2;
						}
					}
			
					if (conf->have_hs_capability_list) {
                        varlen += 10;
                        dl_list_for_each(hs_capability_subtype, &conf->hs_capability_list,
                                                        struct anqp_hs_capability, list) {
                            varlen += 1;
                        }
                    }

				}
				break;
			case VENUE_NAME_INFO:
				if (!conf->query_venue_name) {
					conf->query_venue_name = 1;
					if (conf->have_venue_name) {
						/* Including venue group and venue type */
						varlen += 6;
				
						dl_list_for_each(vname_duple, &conf->venue_name_list,
												  struct venue_name_duple, list) {

							varlen += 1; /* Length Field */
							varlen += vname_duple->length;
						}
					}
				}
				break;
			case NETWORK_AUTH_TYPE_INFO:
				if (!conf->query_network_auth_type) {
					conf->query_network_auth_type = 1; 
					if (conf->have_network_auth_type) {
				
						varlen += 4;

						dl_list_for_each(auth_type_unit, &conf->network_auth_type_list,
											struct net_auth_type_unit, list) {
							varlen += 3;
							varlen += auth_type_unit->re_direct_URL_len;
						}
					}
				}

				break;
			case ROAMING_CONSORTIUM_LIST:
				if (!conf->query_roaming_consortium_list) {
					conf->query_roaming_consortium_list = 1;
					if (conf->have_roaming_consortium_list) {
						varlen += 4;
				
						dl_list_for_each(oiduple, &conf->oi_duple_list,
											struct oi_duple, list) {
							varlen += 1;
							varlen += oiduple->length;
						}
					}
				}
				break;
			case IP_ADDRESS_TYPE_AVAILABILITY_INFO:
				if (!conf->query_ip_address_type) {
					conf->query_ip_address_type = 1;
					if (conf->have_ip_address_type) {
						varlen += 4;
						varlen += 1;
					}
				}
				break;
			case NAI_REALM_LIST:
				if (!conf->query_nai_realm_list) {
					conf->query_nai_realm_list = 1;
					varlen += 6;
					dl_list_for_each(realm_data, &conf->nai_realm_list,
										struct nai_realm_data, list) {
						varlen += 2; /* NAI Realm Data Field Length */
						varlen += realm_data->nai_realm_data_field_len;
					}
				}
				break;
			case ThirdGPP_CELLULAR_NETWORK_INFO:
				if (!conf->query_3gpp_network_info) {
					conf->query_3gpp_network_info = 1;
					if (conf->have_3gpp_network_info) {
						varlen += 4;
						varlen += 5;
						dl_list_for_each(plmn_unit, &conf->plmn_list,
											struct plmn, list) {
							varlen += 3;
						}
					}
				}
				break;
			case DOMAIN_NAME_LIST:
				if (!conf->query_domain_name_list) {
					conf->query_domain_name_list = 1;
					if (conf->have_domain_name_list) {
						varlen += 4;
						dl_list_for_each(dname_field, &conf->domain_name_list,
											struct domain_name_field, list) {
							varlen += 1;
							varlen += dname_field->length;
						}
					}
				}
				break;
			/* add location anqp_rsp */
			case AP_GEOSPATIAL_LOCATION:
				if (!conf->query_ap_geospatial_location && wapp->hs->lci_IE_len > 0){
					conf->query_ap_geospatial_location = 1;
					varlen += 4;
					varlen += wapp->hs->lci_IE_len;
					DBGPRINT(RT_DEBUG_OFF, "%s AP_GEOSPATIAL_LOCATION  len(%d)\n",__FUNCTION__, wapp->hs->lci_IE_len);
				}
				break;
			case AP_CIVIC_LOCATION:
				if (!conf->query_ap_civic_location && wapp->hs->civic_IE_len > 0){
					conf->query_ap_civic_location = 1;
					varlen += 4;
					varlen += wapp->hs->civic_IE_len;
					DBGPRINT(RT_DEBUG_OFF, "%s AP_CIVIC_LOCATION  len(%d)\n",__FUNCTION__, wapp->hs->civic_IE_len);
				}
				break;
			case AP_LOCATION_PUBLIC_IDENTIFIER_URI:
				if (!conf->query_ap_location_public_uri && wapp->hs->public_id_uri_len > 0){
					conf->query_ap_location_public_uri = 1;
					varlen += 4;
					varlen += wapp->hs->public_id_uri_len;
					DBGPRINT(RT_DEBUG_OFF, "%s AP_LOCATION_PUBLIC_IDENTIFIER_URI  len(%d)\n",__FUNCTION__, wapp->hs->public_id_uri_len);
				}
				break;
			case NEIGHBOR_REPORT:
				if (!conf->query_ap_neighbor_report){
					conf->query_ap_neighbor_report = 1;

					/* 
					per MBO spec v0.0.r27-neighbor-report-anqp.docx, ANQP Neighbor Report list should be like below
					| Info_ID (272) |Info_Len (NR# * 18) |NR_ID (52) | NR_Len(16) | NR#1 |NR ID (52) | Len(16) |NR#2 |...
					*/
					if(wapp->daemon_nr_list.CurrListNum > 0) {
						//varlen += (wapp->daemon_nr_list.CurrListNum * (4 + NEIGHBOR_REPORT_IE_SIZE)); 
						/* 4: Info_ID+Info_Len , 2 + NEIGHBOR_REPORT_IE_SIZE: NR TLV */
						varlen += (4 + (wapp->daemon_nr_list.CurrListNum * (2 + NEIGHBOR_REPORT_IE_SIZE))); 
					}
					DBGPRINT(RT_DEBUG_OFF, "%s AP_NEIGHBOR_REPORT wapp->mbo->daemon_nr_list.CurrListNum(%d) len(%u)\n",
						__FUNCTION__,wapp->daemon_nr_list.CurrListNum,
						(UINT32)(4 + (wapp->daemon_nr_list.CurrListNum * (2 + NEIGHBOR_REPORT_IE_SIZE ))));
				}
				break;
			case VENUE_URL:
				if (!conf->query_venue_url ) {
					conf->query_venue_url = 1;
					if (conf->have_venue_url) {
						varlen += 4;
						dl_list_for_each(vurl_duple, &conf->venue_url_list,
										struct venue_url_duple, list) {
						/* 1:url_length 1:venue_number */
							varlen += 2;
							varlen += vurl_duple->url_length;
						}
					}
				}
				break;
			case ADVICE_OF_CHARGE:
				if (!conf->query_advice_of_charge ) {
					conf->query_advice_of_charge = 1;
					if (conf->have_advice_of_charge) {
						varlen += 4;
						dl_list_for_each(charge_data, &conf->advice_of_charge_list,
										struct advice_of_charge_data, list) {
							varlen += 2; /* Duple length field */
						/* 1:advice_of_charge_type 1:aoc_realm_encoding 1:aoc_realm_len */
							varlen += 3;
							varlen += charge_data->aoc_realm_len;
							dl_list_for_each(plan_tuple_data, &charge_data->aoc_plan_tuples_list,
													struct aoc_plan_tuple_data, list) {
							/* 2:plan_information_len 3:language 3:currency_code */
								varlen += 2;
								varlen += 3;
								varlen += 3;
								varlen += plan_tuple_data->plan_information_len;
							}
						}
					}
				}
				break;
			default:
				DBGPRINT(RT_DEBUG_OFF, "unknown query req info id(%d)\n", info_id);
				break;			
		}

		curpos += 2;
		curlen += 2;
	}

	return varlen;
}

static int wapp_calc_total_anqp_rsp_len(struct wifi_app *wapp,
									struct wapp_conf *conf,
									char *anqp_req,
									size_t total_anqp_req_len)
{
	size_t varlen = 0, curlen = 0;
	char *curpos;
	u16 tmpbuf, info_id;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	curpos = anqp_req;

	while (curlen < total_anqp_req_len) {	
		os_memcpy(&tmpbuf, curpos, 2);

		/* Info ID */
		info_id = le2cpu16(tmpbuf);
	
		if (info_id == ANQP_QUERY_LIST) {

			DBGPRINT(RT_DEBUG_TRACE, "Info ID is a ANQP query list\n");
			curpos += 2;
			curlen += 2;

			/* Length Filed */
			os_memcpy(&tmpbuf, curpos, 2);

			curpos += 2;
			curlen += 2;

			conf->calc_anqp_rsp_len = wapp_calc_anqp_rsp_len(wapp,
				 		   			 		    			conf,
									 		    			le2cpu16(tmpbuf),
									 		   		 		curpos);
			varlen += conf->calc_anqp_rsp_len;

			curpos += le2cpu16(tmpbuf);
			curlen += le2cpu16(tmpbuf);
		} else if (info_id == ACCESS_NETWORK_QUERY_PROTO_VENDOR_SPECIFIC_LIST) {
			DBGPRINT(RT_DEBUG_TRACE, "Info ID is a ANQP vendor-specific query list\n");
			curpos += 2;
			curlen += 2;
			const char wfa_oi[3] = {0x50, 0x6F, 0x9A};

			/* Length Field */
			os_memcpy(&tmpbuf, curpos, 2);			
	
			curpos += 2;
			curlen += 2;

			if (os_memcmp(curpos, wfa_oi, 3) != 0) {
				printf("oi do not match [%02x:%02x:%02x]\n", *curpos, *(curpos+1), *(curpos+2));
				return varlen;
			}

			curpos += 3;
			curlen += 3;

			/* Type HS */
			if (*curpos == WFA_TIA_HS){
				curpos++;
				curlen++;
		
				/* Subtype */
				if (*curpos == HS_QUERY_LIST) {
					DBGPRINT(RT_DEBUG_TRACE, "subtype field is HS Query list\n");

					/* Include Reserved */
					curpos += 2;
					curlen += 2;

					conf->calc_hs_anqp_rsp_len = hotspot_calc_hs_anqp_rsp_len(wapp,
											 		       					  conf,
											 		       					  le2cpu16(tmpbuf) - 6,
											 		       					  curpos);

					varlen += conf->calc_hs_anqp_rsp_len;

				} else if  (*curpos == NAI_HOME_REALM_QUERY) {
					DBGPRINT(RT_DEBUG_TRACE, "subtype is NAI home realm query\n");
				
					/* Include Reserved */
					curpos += 2;
					curlen += 2;

					conf->calc_hs_nai_home_realm_anqp_rsp_len = 
											hotspot_calc_nai_home_realm_anqp_rsp_len(wapp,
																					 conf,
																  	  				 le2cpu16(tmpbuf) - 6,
																  	  				 curpos);
					varlen += conf->calc_hs_nai_home_realm_anqp_rsp_len;

				} else if  (*curpos == ICON_REQUEST) {
					DBGPRINT(RT_DEBUG_TRACE, "subtype is ICON request\n");
					
					/* Include Reserved */
					curpos += 2;
					curlen += 2;
					
					if (wapp->hs->version >= 2)
						conf->calc_hs_icon_file_len = hotspot_calc_icon_binary_file_len(wapp,
											 		       					  	   conf,
											 		       					  	   le2cpu16(tmpbuf) - 6,
											 		       					  	   curpos);
					else
						conf->calc_hs_icon_file_len = 0;
						
					varlen += conf->calc_hs_icon_file_len;
				} else {
					DBGPRINT(RT_DEBUG_ERROR, "subtype(%d) is not HS Query list and not NAI home realm query and not ICON request\n", *curpos);
				}
				curpos += (le2cpu16(tmpbuf) - 6);
				curlen += (le2cpu16(tmpbuf) - 6);
			}
			else if(*curpos == WFA_TIA_MBO){
				/* Type MBO */
				curpos++;
				curlen++;
				/* Subtype */
				if (*curpos == MBO_QUERY_LIST) {
					DBGPRINT(RT_DEBUG_ERROR, "subtype field is MBO_QUERY_LIST\n");
					curpos++;
					curlen++;
					/* mbo query list prefix len = 5  : OUI + WFA_TIA_MBO + MBO_QUERY_LIST (50-6F-9A-12-01)  */
					conf->calc_mbo_anqp_rsp_len = wapp_calc_mbo_anqp_rsp_len(wapp,
											 		       					 conf,
											 		       					 le2cpu16(tmpbuf) - 5,
											 		       					 curpos);
					varlen += conf->calc_mbo_anqp_rsp_len;					

				} else {
					DBGPRINT(RT_DEBUG_ERROR, "subtype(%d) is not HS Query list and not NAI home realm query and not ICON request\n", *curpos);					
				}
				curpos += (le2cpu16(tmpbuf) - 5);
				curlen += (le2cpu16(tmpbuf) - 5);
				
			}
			else{
				/* Type not HS nor MBO */
				struct oi_duple *oiduple;
				DBGPRINT(RT_DEBUG_ERROR, "type(%d) field do not match WFA TIA for HS2.0 nor MBO\n", *curpos);
			
				/* Calc Roaming consortium list */
				if (conf->have_roaming_consortium_list) {
					if (!conf->query_roaming_consortium_list) {
						varlen += 4;
						dl_list_for_each(oiduple, &conf->oi_duple_list,
												struct oi_duple, list) {
							varlen += 1;
							varlen += oiduple->length;
						}
						conf->query_roaming_consortium_list = 1;
					}
				} else {
					DBGPRINT(RT_DEBUG_TRACE, "AP does not have roaming consortium list info\n");
				}
				curpos += (le2cpu16(tmpbuf) - 3);
				curlen += (le2cpu16(tmpbuf) - 3);				
			}

			
		} 
		else {
			DBGPRINT(RT_DEBUG_ERROR, "Info ID(%d) is not ANQP query list and not ANQP vendor-specific query list\n", info_id);
			curpos += 2;
			curlen += 2;

			/* Length Filed */
			os_memcpy(&tmpbuf, curpos, 2);

			curpos += (le2cpu16(tmpbuf) + 2);
			curlen += (le2cpu16(tmpbuf) + 2);
		}
	}

	return varlen;
}

/* collect ANQP content */
static int wapp_collect_anqp_rsp(struct wifi_app *wapp,
	struct wapp_conf *conf,
	char *buf,
	const u8 *peer_mac_addr)
{
	char *pos;
	struct anqp_frame *anqp_rsp = (struct anqp_frame *)buf;
	u16 tmp, tmplen = 0;
	pos = buf;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (conf->query_anqp_capability_list) {
		DBGPRINT(RT_DEBUG_TRACE, "STA query ANQP capability list\n");
		if (conf->have_anqp_capability_list) {
			struct anqp_capability *capability_info;
			DBGPRINT(RT_DEBUG_TRACE, "Collect ANQP capability list\n");
			tmplen = 0;
			anqp_rsp->info_id = cpu2le16(ANQP_CAPABILITY);
			pos = anqp_rsp->variable;
			dl_list_for_each(capability_info, &conf->anqp_capability_list, 
											struct anqp_capability, list) {
				tmp = cpu2le16(capability_info->info_id);
				os_memcpy(pos, &tmp, 2);
				tmplen += 2;
				pos += 2;
			}

//JERRY
#if 1
			{
				struct anqp_hs_capability *hs_capability_subtype;
				struct hs_anqp_frame *hs_anqp_rsp = (struct hs_anqp_frame *)pos;
			    const char wfa_oi[3] = {0x50, 0x6F, 0x9A};
				u16 tmphslen = 0;

	            DBGPRINT(RT_DEBUG_TRACE, "Collect HS capability list\n");
				tmplen += 4; //0xdd 0xdd len 2bytes
        	    hs_anqp_rsp->info_id = cpu2le16(ACCESS_NETWORK_QUERY_PROTO_VENDOR_SPECIFIC_LIST);
            	os_memcpy(hs_anqp_rsp->oi, wfa_oi, 3);
	            tmplen += 3;
				tmphslen += 3;
    	        hs_anqp_rsp->type = WFA_TIA_HS;
        	    tmplen++;
				tmphslen++;
            	hs_anqp_rsp->subtype = HS_CAPABILITY;
	            tmplen += 2;
				tmphslen += 2;

    	        pos = (char *)hs_anqp_rsp->variable;
        	    if (conf->have_hs_capability_list) {
            	    dl_list_for_each(hs_capability_subtype, &conf->hs_capability_list,
                                            struct anqp_hs_capability, list) {
                	    *pos = hs_capability_subtype->subtype;
                    	pos++;
	                    tmplen++;
						tmphslen++;
    	            }
        	    }
				hs_anqp_rsp->length = cpu2le16(tmphslen);
			}
#endif

			anqp_rsp->length = cpu2le16(tmplen);
            anqp_rsp = (struct anqp_frame *)pos;
		}
		
		conf->query_anqp_capability_list = 0;
	}

	if (conf->query_venue_name) {
		DBGPRINT(RT_DEBUG_TRACE, "STA query venue name information\n");
		if (conf->have_venue_name) {
			struct venue_name_duple *vname_duple;
			DBGPRINT(RT_DEBUG_TRACE, "Collect venue name information\n");
			tmplen = 0;
			anqp_rsp->info_id = cpu2le16(VENUE_NAME_INFO);
			pos = anqp_rsp->variable;
			
			*pos = conf->venue_group;
			pos++;
			tmplen++;

			*pos = conf->venue_type;
			pos++;
			tmplen++;

			dl_list_for_each(vname_duple, &conf->venue_name_list, 
						struct venue_name_duple, list) {
				*pos = vname_duple->length;
				pos++;
				tmplen++;

				os_memcpy(pos, vname_duple->language, 3);
				pos += 3;
				tmplen += 3;

				os_memcpy(pos, vname_duple->venue_name, (vname_duple->length -3));
				pos += (vname_duple->length -3);
				tmplen += (vname_duple->length - 3);
			}
			anqp_rsp->length = cpu2le16(tmplen);
			anqp_rsp = (struct anqp_frame *)pos;
		} else
			DBGPRINT(RT_DEBUG_TRACE, "AP does not have venu name information\n");
			
		conf->query_venue_name = 0;
	}

	if (conf->query_network_auth_type) {
		DBGPRINT(RT_DEBUG_TRACE, "STA query network authentication type information\n");
		if (conf->have_network_auth_type) {
			struct net_auth_type_unit *auth_type_unit;
			u16 tmp_url_len = 0;
			DBGPRINT(RT_DEBUG_TRACE, "Collect network authentication type information\n");
			tmplen = 0;
			anqp_rsp->info_id = cpu2le16(NETWORK_AUTH_TYPE_INFO);
		
			pos = anqp_rsp->variable;
			dl_list_for_each(auth_type_unit, &conf->network_auth_type_list,
									struct net_auth_type_unit, list) {
				*pos = auth_type_unit->net_auth_type_indicator;
				pos++;
				tmplen++;
				
				tmp_url_len = auth_type_unit->re_direct_URL_len;
				tmp_url_len = cpu2le16(tmp_url_len);
				os_memcpy(pos, &tmp_url_len, 2);
				pos += 2;
				tmplen += 2;

				os_memcpy(pos, auth_type_unit->re_direct_URL, 
									auth_type_unit->re_direct_URL_len);
				tmplen += auth_type_unit->re_direct_URL_len;
				pos += auth_type_unit->re_direct_URL_len;
			}
			anqp_rsp->length = cpu2le16(tmplen);
			anqp_rsp = (struct anqp_frame *)pos;
		} else
			DBGPRINT(RT_DEBUG_TRACE, "AP does not have network auth type info\n");

		conf->query_network_auth_type = 0;
	}

	if (conf->query_roaming_consortium_list) {
		DBGPRINT(RT_DEBUG_TRACE, "STA query roaming consortium list\n");
		if (conf->have_roaming_consortium_list) {
			struct oi_duple *oiduple;
			DBGPRINT(RT_DEBUG_TRACE, "Collect roaming consortium list\n");
			tmplen = 0;
			anqp_rsp->info_id = cpu2le16(ROAMING_CONSORTIUM_LIST);
		
			pos = anqp_rsp->variable;
			dl_list_for_each(oiduple, &conf->oi_duple_list, 
											struct oi_duple, list) {
				*pos = oiduple->length;
				pos++;
				tmplen++;
				os_memcpy(pos, oiduple->oi, oiduple->length);
				tmplen += oiduple->length;
				pos += oiduple->length;
			}
			anqp_rsp->length = cpu2le16(tmplen);
			anqp_rsp = (struct anqp_frame *)pos;
		} else
			DBGPRINT(RT_DEBUG_TRACE, "AP does not have roaming consortium list info\n");
			
		conf->query_roaming_consortium_list = 0;
	}

	if (conf->query_ip_address_type) {
		DBGPRINT(RT_DEBUG_TRACE, "STA query IP address type availability\n");
		if (conf->have_ip_address_type) {
			DBGPRINT(RT_DEBUG_TRACE, "Collect IP address type availability\n");
			tmplen = 0;
			anqp_rsp->info_id = cpu2le16(IP_ADDRESS_TYPE_AVAILABILITY_INFO);
		
			pos = anqp_rsp->variable;
			*pos = conf->ipv6_address_type & ~0xFC;
			*pos = *pos | ((conf->ipv4_address_type & 0x3F) << 2);
			tmplen++;
			pos++;
			anqp_rsp->length = cpu2le16(tmplen);
			anqp_rsp = (struct anqp_frame *)pos;
		} else
			DBGPRINT(RT_DEBUG_TRACE, "AP does not have ip address type availability info\n");
		
		conf->query_ip_address_type = 0;
	}

	if (conf->query_nai_realm_list) {
		DBGPRINT(RT_DEBUG_TRACE, "STA query NAI realm list\n");
		u16 nai_realm_count  = 0;
		struct nai_realm_data *realm_data;
		u16 nai_realm_data_field_len_tmp;
		struct eap_method *eapmethod;
		struct auth_param *authparam;
		DBGPRINT(RT_DEBUG_TRACE, "Collect NAI realm list\n");
		tmplen = 0;
		anqp_rsp->info_id = cpu2le16(NAI_REALM_LIST);
		pos = anqp_rsp->variable + 2;
		tmplen += 2; /* NAI Realm Count length */
		dl_list_for_each(realm_data, &conf->nai_realm_list,
								 struct nai_realm_data, list) {
			nai_realm_count++;
			nai_realm_data_field_len_tmp = cpu2le16(realm_data->nai_realm_data_field_len);
			os_memcpy(pos, &nai_realm_data_field_len_tmp, 2);
			tmplen += 2;
			pos += 2;
				
			*pos = realm_data->nai_realm_encoding;
			tmplen += 1;
			pos++;
				
			*pos = realm_data->nai_realm_len;
			tmplen += 1;
			pos++;
				
			os_memcpy(pos, realm_data->nai_realm, realm_data->nai_realm_len);
			tmplen += realm_data->nai_realm_len;
			pos += realm_data->nai_realm_len;

			*pos = realm_data->eap_method_count;
			tmplen += 1;
			pos++;

			dl_list_for_each(eapmethod, &realm_data->eap_method_list,
										struct eap_method, list) {
				*pos = eapmethod->len;
				tmplen += 1;
				pos++;

				*pos = eapmethod->eap_method;
				tmplen += 1;
				pos++;

				*pos = eapmethod->auth_param_count;
				tmplen += 1;
				pos++;

				dl_list_for_each(authparam, &eapmethod->auth_param_list,
										struct auth_param, list) {
					*pos = authparam->id;
					tmplen += 1;
					pos++;

					*pos = authparam->len;
					tmplen += 1;
					pos++;

					os_memcpy(pos, authparam->auth_param_value, authparam->len);
					tmplen += authparam->len;
					pos += authparam->len;
				}

			}
		}
		DBGPRINT(RT_DEBUG_TRACE, "NAI Realm Count = %d\n", nai_realm_count);
		nai_realm_count = cpu2le16(nai_realm_count);
		os_memcpy(anqp_rsp->variable, &nai_realm_count, 2);
		anqp_rsp->length = cpu2le16(tmplen);
		anqp_rsp = (struct anqp_frame *)pos;
		
		conf->query_nai_realm_list = 0;
	}

	if (conf->query_3gpp_network_info) {
		DBGPRINT(RT_DEBUG_TRACE, "STA query 3GPP cellular network information\n");
		if (conf->have_3gpp_network_info) {
			struct plmn *plmn_unit;
			struct plmn_IEI *plmn_iei;
			DBGPRINT(RT_DEBUG_TRACE, "Collect 3GPP cellular network information\n");
			tmplen = 0;
			anqp_rsp->info_id = cpu2le16(ThirdGPP_CELLULAR_NETWORK_INFO);
			pos = anqp_rsp->variable;
			
			char *udhl;
			/* GUD */
			*pos = GUD_VER1;
			pos++;
			tmplen++;

			/* Skip UDHL filling */
			udhl = pos;
			pos++;
			tmplen++;

			/* IEI */
			plmn_iei = (struct plmn_IEI *)pos;

			plmn_iei->plmn_list_iei = IEI_PLMN;
			
			tmplen += 3; /* plmn list iei, length of plmn list, and number of plmn */

			
			pos = plmn_iei->variable;

			dl_list_for_each(plmn_unit, &conf->plmn_list, struct plmn, list) {
				plmn_iei->plmn_list_num++;
			
				*pos = (((plmn_unit->mcc[1] << 4) & 0xf0)  | (plmn_unit->mcc[0] & 0x0f));
				pos++;
				tmplen++;

					
				*pos = (((plmn_unit->mnc[2] << 4) & 0xf0) | (plmn_unit->mcc[2] & 0x0f));
				pos++;
				tmplen++;

				*pos = (((plmn_unit->mnc[1] << 4) & 0xf0) | (plmn_unit->mnc[0] & 0x0f));
				pos++;
				tmplen++;
			}

			plmn_iei->plmn_list_len = 1 + (plmn_iei->plmn_list_num * 3);
			
			*udhl = 2 + plmn_iei->plmn_list_len;
			anqp_rsp->length = cpu2le16(tmplen);
			anqp_rsp = (struct anqp_frame *)pos;
		} else
			DBGPRINT(RT_DEBUG_TRACE, "AP does not have 3gpp cellular network info\n");
		
		conf->query_3gpp_network_info = 0;
	}

	if (conf->query_domain_name_list) {
		DBGPRINT(RT_DEBUG_TRACE, "STA query domain name list\n");
		if (conf->have_domain_name_list) {
			struct domain_name_field *dname_field;
			DBGPRINT(RT_DEBUG_TRACE, "Collect domain name list\n");
			tmplen = 0;
			anqp_rsp->info_id = cpu2le16(DOMAIN_NAME_LIST);
			pos = anqp_rsp->variable;
			dl_list_for_each(dname_field, &conf->domain_name_list, 
											struct domain_name_field, list) {
				*pos = dname_field->length;
				pos++;
				tmplen += 1;
				os_memcpy(pos, dname_field->domain_name, dname_field->length);
				tmplen += dname_field->length;
				pos += dname_field->length;
			}
			anqp_rsp->length = cpu2le16(tmplen);
			anqp_rsp = (struct anqp_frame *)pos;
		} else
			DBGPRINT(RT_DEBUG_TRACE, "AP does not have domain name list info\n");

		conf->query_domain_name_list = 0;
	}
#if 1 //location
	if (conf->query_ap_civic_location) {
		if(wapp->hs->civic_IE_len > 0){
			DBGPRINT(RT_DEBUG_ERROR, "STA query AP civic location\n");		
			DBGPRINT(RT_DEBUG_ERROR, "Collect civic location\n");
			tmplen = 0;
			anqp_rsp->info_id = cpu2le16(AP_CIVIC_LOCATION);
			pos = anqp_rsp->variable;
			hex_dump("civic_IE == ", (UCHAR *)wapp->hs->civic_IE,wapp->hs->civic_IE_len);
			//*pos = wapp->hs->civic_IE_len;
			//pos++;
			//tmplen += 1;
			os_memcpy(pos, wapp->hs->civic_IE, wapp->hs->civic_IE_len);
			tmplen += wapp->hs->civic_IE_len;
			pos += wapp->hs->civic_IE_len;

			anqp_rsp->length = cpu2le16(tmplen);
			anqp_rsp = (struct anqp_frame *)pos;		
		}
		else
			DBGPRINT(RT_DEBUG_ERROR, "AP don't have civic location\n");
		conf->query_ap_civic_location = 0;
	}
	
	if (conf->query_ap_geospatial_location) {
		if(wapp->hs->lci_IE_len > 0){
			DBGPRINT(RT_DEBUG_ERROR, "STA query AP geospatial location\n");	
			DBGPRINT(RT_DEBUG_ERROR, "Collect geospatial location\n");
			tmplen = 0;
			anqp_rsp->info_id = cpu2le16(AP_GEOSPATIAL_LOCATION);
			pos = anqp_rsp->variable;
			hex_dump("lci_IE == ",(UCHAR *)wapp->hs->lci_IE,wapp->hs->lci_IE_len);

			//*pos = wapp->hs->lci_IE_len;
			//pos++;
			//tmplen += 1;
			os_memcpy(pos, wapp->hs->lci_IE, wapp->hs->lci_IE_len);
			tmplen += wapp->hs->lci_IE_len;
			pos += wapp->hs->lci_IE_len;

			anqp_rsp->length = cpu2le16(tmplen);
			anqp_rsp = (struct anqp_frame *)pos;		
		}
		else
			DBGPRINT(RT_DEBUG_ERROR, "AP don't have geospatial location\n");
		conf->query_ap_geospatial_location = 0;
	}

	if (conf->query_ap_location_public_uri) {
		if(wapp->hs->public_id_uri_len > 0){
			DBGPRINT(RT_DEBUG_ERROR, "STA query AP PUBLIC_IDENTIFIER_URI\n");	
			DBGPRINT(RT_DEBUG_ERROR, "Collect PUBLIC_IDENTIFIER_URI\n");
			tmplen = 0;
			anqp_rsp->info_id = cpu2le16(AP_LOCATION_PUBLIC_IDENTIFIER_URI);
			pos = anqp_rsp->variable;
			hex_dump("public_id_uri == ",(UCHAR *)wapp->hs->public_id_uri,wapp->hs->public_id_uri_len);

			//*pos = wapp->hs->public_id_uri_len;
			//pos++;
			//tmplen += 1;
			os_memcpy(pos, wapp->hs->public_id_uri, wapp->hs->public_id_uri_len);
			tmplen += wapp->hs->public_id_uri_len;
			pos += wapp->hs->public_id_uri_len;

			anqp_rsp->length = cpu2le16(tmplen);
			anqp_rsp = (struct anqp_frame *)pos;		
		}
		else
			DBGPRINT(RT_DEBUG_ERROR, "AP don't have PUBLIC_IDENTIFIER_URI\n");
		conf->query_ap_location_public_uri = 0;
	}
	
	if (conf->query_ap_neighbor_report) {
		DBGPRINT(RT_DEBUG_ERROR, "STA query AP neighbor report\n");	

		if( wapp->daemon_nr_list.CurrListNum > 0){
			u8 disassoc_imnt = FALSE;
			u8 is_steer_to_cell = FALSE;
			u8 bss_term = FALSE;
			struct wapp_sta *sta = NULL;
			tmplen = 0;

			anqp_rsp->info_id = cpu2le16(NEIGHBOR_REPORT);
			pos = anqp_rsp->variable;
			// append neighbor report
			sta = wdev_ap_client_list_lookup_for_all_bss(wapp, peer_mac_addr);
			mbo_check_sta_preference_and_append_nr_list(wapp,
			                                            sta,
                                                        pos,
                                                        &tmplen,
                                                        MBO_FRAME_ANQP,
                                                        disassoc_imnt,
                                                        bss_term,
                                                        is_steer_to_cell);
			anqp_rsp->length = tmplen;
			DBGPRINT(RT_DEBUG_TRACE, "append neighbor report anqp_rsp->length %d\n",anqp_rsp->length);			
			anqp_rsp = (struct anqp_frame *)pos;		
		}
		else
		{
			DBGPRINT(RT_DEBUG_ERROR, "AP don't have neighbor report\n");
			anqp_rsp->info_id = cpu2le16(NEIGHBOR_REPORT);
			pos = anqp_rsp->variable;
			anqp_rsp->length = 0;
			anqp_rsp = (struct anqp_frame *)pos;	
		}
		conf->query_ap_neighbor_report = 0;
	}

	if (conf->query_advice_of_charge) {
		DBGPRINT(RT_DEBUG_ERROR, "STA query advice of charge\n");
		if (conf->have_advice_of_charge) {
			struct advice_of_charge_data *charge_data;
			struct aoc_plan_tuple_data *plan_tuple_data;
			u16 plan_info_len_tmp = 0;
			char *plan_info_len_tmp_pos = NULL;
			DBGPRINT(RT_DEBUG_TRACE, "Collect advice of charge information\n");
			tmplen = 0;
			anqp_rsp->info_id = cpu2le16(ADVICE_OF_CHARGE);

			pos = anqp_rsp->variable;
			dl_list_for_each(charge_data, &conf->advice_of_charge_list,
							struct advice_of_charge_data, list) {

				plan_info_len_tmp = 0;
				plan_info_len_tmp_pos = pos;
				pos += 2;
				tmplen += 2;

				*pos = charge_data->advice_of_charge_type;
				plan_info_len_tmp++;
				pos++;
				tmplen++;

				*pos = charge_data->aoc_realm_encoding;
				plan_info_len_tmp++;
				pos++;
				tmplen++;

				*pos = charge_data->aoc_realm_len;
				plan_info_len_tmp++;
				pos++;
				tmplen++;

				os_memcpy(pos, charge_data->aoc_realm, charge_data->aoc_realm_len);
				plan_info_len_tmp += charge_data->aoc_realm_len;
				pos += charge_data->aoc_realm_len;
				tmplen += charge_data->aoc_realm_len;

				dl_list_for_each(plan_tuple_data, &charge_data->aoc_plan_tuples_list,
									struct aoc_plan_tuple_data, list) {
					plan_info_len_tmp += plan_tuple_data->plan_information_len + 8;
					*pos = plan_tuple_data->plan_information_len + 6;
					pos += 2;
					tmplen += 2;

					os_memcpy(pos, plan_tuple_data->language, 3);
					pos += 3;
					tmplen += 3;

					os_memcpy(pos, plan_tuple_data->currency_code, 3);
					pos += 3;
					tmplen += 3;

					os_memcpy(pos, plan_tuple_data->plan_information, plan_tuple_data->plan_information_len);
					pos += plan_tuple_data->plan_information_len;
					tmplen += plan_tuple_data->plan_information_len;
				}
				plan_info_len_tmp = cpu2le16(plan_info_len_tmp);
				os_memcpy(plan_info_len_tmp_pos, &plan_info_len_tmp, 2);
			}
			anqp_rsp->length = cpu2le16(tmplen);
			anqp_rsp = (struct anqp_frame *)pos;
		} else
			DBGPRINT(RT_DEBUG_TRACE, "AP does not have advice of charge information\n");

		conf->query_advice_of_charge = 0;
	}

	if (conf->query_venue_url) {
		DBGPRINT(RT_DEBUG_ERROR, "STA query venue url\n");
		if (conf->have_venue_url) {
			struct venue_url_duple *vurl_duple;
			DBGPRINT(RT_DEBUG_TRACE, "Collect venue url information\n");
			tmplen = 0;
			anqp_rsp->info_id = cpu2le16(VENUE_URL);

			pos = anqp_rsp->variable;
			dl_list_for_each(vurl_duple, &conf->venue_url_list,
							struct venue_url_duple, list) {
				*pos = vurl_duple->url_length + 1;
				pos++;
				tmplen++;

				*pos = vurl_duple->venue_number;
				pos++;
				tmplen++;

				os_memcpy(pos, vurl_duple->venue_url, vurl_duple->url_length);
				pos += vurl_duple->url_length;
				tmplen += vurl_duple->url_length;
			}
			anqp_rsp->length = cpu2le16(tmplen);
			anqp_rsp = (struct anqp_frame *)pos;
		} else
			DBGPRINT(RT_DEBUG_TRACE, "AP does not have venue url information\n");

		conf->query_venue_url = 0;
	}
#endif
	return 0;
}

static int wapp_collect_total_anqp_rsp(
	struct wifi_app *wapp,
	struct wapp_conf *conf,
	char *anqp_rsp,
	const u8 *peer_mac_addr)
{
	char *curpos = anqp_rsp;
	int ret;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	ret = wapp_collect_anqp_rsp(wapp, conf, curpos, peer_mac_addr);

	if (ret) {
		DBGPRINT(RT_DEBUG_ERROR, "Collect anqp rsp fail\n");
		return ret;
	}

	curpos += conf->calc_anqp_rsp_len;

	ret = hotspot_collect_hs_anqp_rsp(wapp, conf, curpos);

	if (ret) {
		DBGPRINT(RT_DEBUG_ERROR, "Collect hs anqp rsp fail\n");
		return ret;
	}

	curpos += conf->calc_hs_anqp_rsp_len;

	if (conf->query_nai_home_realm) {
		ret = hotspot_collect_nai_home_realm_anqp_rsp(wapp, conf, curpos);
		
		if (ret) {
			DBGPRINT(RT_DEBUG_ERROR, "Collect hs nai home realm anqp rsp fail\n");
			return ret;
		}

		curpos += conf->calc_hs_nai_home_realm_anqp_rsp_len;
	}
	

    if (wapp->hs->version >= 2)
    {
	    if (conf->query_icon_binary_file) {		
			ret = hotspot_collect_icon_binary_file(wapp, conf, curpos);
		
			if (ret) {
				DBGPRINT(RT_DEBUG_ERROR, "Collect icon binary file fail\n");
				return ret;
			}

			curpos += conf->calc_hs_icon_file_len;		
	    }
    }

    ret = wapp_collect_mbo_anqp_rsp(wapp, conf, curpos);

	if (ret) {
		DBGPRINT(RT_DEBUG_ERROR, "Collect mbo anqp rsp fail\n");
		return ret;
	}

	curpos += conf->calc_mbo_anqp_rsp_len;

    return ret;
}

int wapp_event_anqp_req(struct wifi_app *wapp, 
						   const char *iface, 
				           const u8 *peer_mac_addr, 
				           char *anqp_req, 
				           size_t anqp_req_len)
{
	char *buf;
	size_t varlen = 0;
	struct wapp_conf *conf;
	u8 is_found = 0;

	DBGPRINT(RT_DEBUG_TRACE, "\n");
	#ifdef MAP_R2
	map_send_anqp_req_tunneled_message(wapp, peer_mac_addr, anqp_req, anqp_req_len);
#endif /* MAP_SUPPORT */
	//workaround
	if (anqp_req_len < LENGTH_802_11) {
		DBGPRINT(RT_DEBUG_ERROR, "Invalid length\n");
		return -1;
	}
	anqp_req += 9;
	anqp_req_len -= 9;

	dl_list_for_each(conf, &wapp->conf_list, struct wapp_conf, list) {
	//DBGPRINT(RT_DEBUG_ERROR, "####conf->iface(%s) iface(%s)\n",conf->iface, iface);
		if (os_strcmp(conf->iface, iface) == 0) {
			is_found = 1;
			break;
		}
	}

	if (!is_found) {
		DBGPRINT(RT_DEBUG_TRACE, "Can not find match hotspot configuration file (%s)\n",iface);
		return -1;
	}

	//if (!conf->hotspot_onoff)  //location needs anqp_rep while hs is not on
		//return -1;

	varlen = wapp_calc_total_anqp_rsp_len(wapp,
										 conf,
										 anqp_req,
										 anqp_req_len);
	if (varlen > 0) {
		buf = os_zalloc(varlen);

		if (!buf) {
			DBGPRINT(RT_DEBUG_ERROR, "Memory is not available(%s)\n", __FUNCTION__);
			return -1;
		}
	
		wapp_collect_total_anqp_rsp(wapp, 
									   conf, 
									   buf,
									   peer_mac_addr);
		/* Send ANQP resonse to driver */
		wapp_send_anqp_rsp(wapp, conf->iface, peer_mac_addr, buf, varlen);

		os_free(buf);
	}else {
		DBGPRINT(RT_DEBUG_OFF, "anqp rsp cal len is zero, maybe anqp req is wrong(%s)\n", __FUNCTION__);
		return -1;
	}

	conf->calc_anqp_rsp_len = 0;
	conf->calc_hs_anqp_rsp_len = 0;
	conf->calc_hs_nai_home_realm_anqp_rsp_len = 0;
	conf->calc_mbo_anqp_rsp_len = 0;
	return 0;
}

int wapp_set_interworking_enable(struct wifi_app *wapp, const char *iface, char *enable)
{	
	int ret;
	size_t len = 1;

	DBGPRINT(RT_DEBUG_ERROR, "%s\n", __FUNCTION__);
	
	ret = wapp->drv_ops->drv_set_interworking(wapp->drv_data, iface, enable, len);
	
	return ret;	
}




