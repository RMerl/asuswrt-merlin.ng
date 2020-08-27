custom_clientlist_array = decodeURIComponent('<% nvram_char_to_ascii("", "custom_clientlist"); %>').replace(/&#62/g, ">").replace(/&#60/g, "<").split('<');
originDataTmp = originData;
originData = {
	fromNetworkmapd : [<% get_clientlist(); %>],
	nmpClient : [<% get_clientlist_from_json_database(); %>]
}
