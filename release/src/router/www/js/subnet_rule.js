var subnet_rulelist = decodeURIComponent("<% nvram_char_to_ascii("","subnet_rulelist"); %>");
var subnet_rulelist_row = subnet_rulelist.split('<');
var subnet_rulelist_ext = decodeURIComponent("<% nvram_char_to_ascii("","subnet_rulelist_ext"); %>");
var subnet_rulelist_ext_row = subnet_rulelist_ext.split('<');
var LanToLanRoute_array = new Array();
var cur_LanToLanRoute = new Array();
var cur_LanToLanRoute_index = -1;

function netmask_to_bits(netmask){
	var netmaskArray = netmask.split(".");
	var val = 0;
	var bit_number = 0;

	for(var i = 0; i < netmaskArray.length; i++){
		val = parseInt(netmaskArray[i], 10);
		for( var j = 0; j < 8; j++){
			if( (val >> j) & 1 )
				bit_number++;
		}
	}

	return bit_number;
}

function bits_to_netmask(bits){
	var netmaskArray = [];
	var maskInt = 0;
	var netmask = "";

	for(var i = 0; i < 4; i++){
		maskInt = 0;
		for( var j = 7; j >= 0; j--){
			if( bits > 0 ){
				maskInt = maskInt | (1 << j);
			}
			bits--;
		}
		netmaskArray.push(maskInt);
	}

	netmask = netmaskArray.join('.');
	return netmask;
}

function get_netmask(getway_ip){
	for(var i = 1; i < subnet_rulelist_row.length; i++){
		var subnet_rulelist_col = subnet_rulelist_row[i].split('>');
		if(getway_ip == subnet_rulelist_col[0]){
			return subnet_rulelist_col[1];
		}
	}
}

function parse_LanToLanRoute_to_object(){
	var subnet_rulelist_ext_array = new Array();
	LanToLanRoute_array = [];

	for(var i = 1; i < subnet_rulelist_ext_row.length; i ++) {
		var  subnet_rulelist_ext_col = subnet_rulelist_ext_row[i].split('>');
		subnet_rulelist_ext_array[i-1] = [subnet_rulelist_ext_col[0], subnet_rulelist_ext_col[1]];
	}

	for(var i = 1; i < subnet_rulelist_row.length; i++){
		var subnet_rulelist_col = subnet_rulelist_row[i].split('>');
		var LanToLanRoute_object = new Object();

		LanToLanRoute_object["subnet"] = subnet_rulelist_col[0] + "/" + netmask_to_bits(subnet_rulelist_col[1]);
		LanToLanRoute_object["LanToLanRoute"] = [];

		for(var j = 0; j < subnet_rulelist_ext_array.length; j++){
			if((subnet_rulelist_ext_array[j][0] == subnet_rulelist_col[0]) && (subnet_rulelist_ext_array[j][1].length > 1)){
				var gateway_array = subnet_rulelist_ext_array[j][1].split(',');
				var lanTolanRoute_tmp = [];

				for(var k = 0; k < gateway_array.length - 1; k++){
					var route_string = "";

					route_string = gateway_array[k] + "/" + netmask_to_bits(get_netmask(gateway_array[k]));
					lanTolanRoute_tmp.push(route_string);
				}

				LanToLanRoute_object["LanToLanRoute"] = lanTolanRoute_tmp;
			}
		}

		LanToLanRoute_array.push(LanToLanRoute_object);
	}
}

function get_LanToLanRoute(subnet){
	cur_LanToLanRoute = [];
	cur_LanToLanRoute_index = LanToLanRoute_array.length;

	for(var i = 0; i < LanToLanRoute_array.length; i++){
		if(subnet == LanToLanRoute_array[i]["subnet"]){
			cur_LanToLanRoute = LanToLanRoute_array[i]["LanToLanRoute"];
			cur_LanToLanRoute_index = i;
			break;
		}
	}
}

function rm_LanToLanRoute_rule(subnet){
	for(var i = 0; i < LanToLanRoute_array.length; i++){
		if(subnet == LanToLanRoute_array[i]["subnet"]){
			LanToLanRoute_array.splice(i, 1);
			break;
		}
	}

	for(var i = 0; i < LanToLanRoute_array.length; i++){
		var tmp_LanToLanRoute = LanToLanRoute_array[i]["LanToLanRoute"];
		for(var j = 0; j < tmp_LanToLanRoute.length; j++){
			if(subnet == tmp_LanToLanRoute[j]){
				LanToLanRoute_array[i]["LanToLanRoute"].splice(j, 1);
			}
		}
	}
}

/*
   Check whether the subnet exists in the LanToLanRoute rule or not.
   yes: return index   no: return -1
*/
function getRouteIndex(LanToLanRoute, subnet){
	for(var i = 0; i < LanToLanRoute.length; i++){
		if(subnet == LanToLanRoute[i]){
			return i;
		}
	}

	return -1;
}

function update_LanToLanRoute_array(cur_subnet){//Modify existed Lan-To-Lan route. Add new Lan-To-Lan route
	var i = 0;
	var old_subnet = cur_subnet;
	var new_LanToLanRoute = new Object();

	for(i = 0; i < cur_LanToLanRoute.length; i++){
		if(cur_subnet == cur_LanToLanRoute[i]){
			cur_LanToLanRoute.splice(i, 1);
		}
	}

	if( cur_LanToLanRoute_index == LanToLanRoute_array.length ){
		new_LanToLanRoute["subnet"] = cur_subnet;
		new_LanToLanRoute["LanToLanRoute"] = cur_LanToLanRoute;
		LanToLanRoute_array.push(new_LanToLanRoute);
	}
	else{
		if(LanToLanRoute_array[cur_LanToLanRoute_index]["subnet"] != cur_subnet){
			old_subnet = LanToLanRoute_array[cur_LanToLanRoute_index]["subnet"];
			new_LanToLanRoute["subnet"] = cur_subnet;
			new_LanToLanRoute["LanToLanRoute"] = cur_LanToLanRoute;
			LanToLanRoute_array.splice(cur_LanToLanRoute_index, 1, new_LanToLanRoute);
		}
		else
			LanToLanRoute_array[cur_LanToLanRoute_index]["LanToLanRoute"] = cur_LanToLanRoute;
	}

	for(i = 0; i < LanToLanRoute_array.length; i++){
		/* replace subnet */
		if(old_subnet != cur_subnet){
			var routeIndex = getRouteIndex(LanToLanRoute_array[i]["LanToLanRoute"], old_subnet);
			if(routeIndex >= 0){
				LanToLanRoute_array[i]["LanToLanRoute"].splice(routeIndex, 1, cur_subnet);
			}
		}

		for(var j = 0; j < cur_LanToLanRoute.length; j++){
			if(LanToLanRoute_array[i]["subnet"] == cur_LanToLanRoute[j] && getRouteIndex(LanToLanRoute_array[i]["LanToLanRoute"], cur_subnet) < 0){
				LanToLanRoute_array[i]["LanToLanRoute"].push(cur_subnet);
			}
		}

		if(getRouteIndex(cur_LanToLanRoute, LanToLanRoute_array[i]["subnet"]) < 0 && getRouteIndex(LanToLanRoute_array[i]["LanToLanRoute"], cur_subnet) >= 0 ){
			var tmp_LanToLanRoute = LanToLanRoute_array[i]["LanToLanRoute"];
			for(var k = 0; k < tmp_LanToLanRoute.length; k++){
				if(tmp_LanToLanRoute[k] == cur_subnet)
					LanToLanRoute_array[i]["LanToLanRoute"].splice(k, 1);
			}
		}
	}
}

function save_LanToLanRoute(){
	if(LanToLanRoute_array.length > 0){
		subnet_rulelist_ext = "";
		for(var i = 0; i < LanToLanRoute_array.length; i++){
			var subnet_string = LanToLanRoute_array[i]["subnet"].substring(0, LanToLanRoute_array[i]["subnet"].indexOf('/'));
			var tmp_LanToLanRoute = LanToLanRoute_array[i]["LanToLanRoute"];
			var tmp_array = [];
			var LanToLanRoute_string = "";

			for(var k = 0; k < tmp_LanToLanRoute.length; k++){
				tmp_array.push(tmp_LanToLanRoute[k].substring(0, tmp_LanToLanRoute[k].indexOf('/')));
			}

			if(tmp_array.length > 0)
				LanToLanRoute_string = tmp_array.join(',') + ',';
			subnet_rulelist_ext += '<';
			subnet_rulelist_ext += subnet_string;
			subnet_rulelist_ext += '>';
			subnet_rulelist_ext += LanToLanRoute_string;
		}
	}
}