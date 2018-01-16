originDataTmp = originData;
originData = {
	fromNetworkmapd : [<% get_clientlist(); %>],
	nmpClient : [<% get_clientlist_from_json_database(); %>]
}
get_cfg_clientlist = [<% get_cfg_clientlist(); %>][0];
get_wclientlist = [<% get_wclientlist(); %>][0];
get_wiredclientlist = [<% get_wiredclientlist(); %>][0];
get_allclientlist = [<% get_allclientlist(); %>][0];