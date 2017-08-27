originDataTmp = originData;
originData = {
	fromNetworkmapd : [<% get_clientlist(); %>],
	nmpClient : [<% get_clientlist_from_json_database(); %>],
	amasClient : [<% get_cfg_clientlist(); %>],
	amasREClient : [<% get_wclientlist(); %>],
	amasREClientDetail : [<% get_allclientlist(); %>]
}
networkmap_fullscan = '<% nvram_get("networkmap_fullscan"); %>';
if(networkmap_fullscan == 1) genClientList();
