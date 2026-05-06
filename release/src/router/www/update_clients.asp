originDataTmp = originData;
originData = {
	fromNetworkmapd : [httpApi.hookGet("get_clientlist")],
	nmpClient : [httpApi.hookGet("get_clientlist_from_json_database")],
}
networkmap_fullscan = '<% nvram_get("networkmap_fullscan"); %>';
if(networkmap_fullscan == 2) genClientList();
