/* AsusWRT Traffic Monitor utility functions */

const tm_nvram = httpApi.nvramGet(["bond_wan", "wans_lanport"]);
const scaleNames = ['KB', 'MB', 'GB'];
const scaleFactors = [1, 1024, 1048576];
const ui_locale = ui_lang.toLowerCase();

function get_friendly_ifname(ifname){
	var title;

	switch(ifname){
		case "INTERNET":
		case "INTERNET0":
			if(dualWAN_support){
				if(wans_dualwan_array[0] == "usb"){
					if(gobi_support)
						title = "<#Mobile_title#>";
					else
						title = "USB Modem";
				}
				else if(wans_dualwan_array[0] == "wan"){
					title = "WAN";
					if (based_modelid == "TUF-AX4200" || based_modelid == "TUF-AX6000")
						title = "2.5G WAN";
					if (based_modelid == "GT-AXY16000" || based_modelid == "RT-AX89U" || based_modelid == "TUF-AX4200" || based_modelid == "TUF-AX6000") {
						if (wan_bonding_support && tm_nvram.bond_wan == '1')
							title = "Bond";
					}
				}
				else if(wans_dualwan_array[0] == "wan2"){
					if (based_modelid == "GT-AXY16000" || based_modelid == "RT-AX89U")
						title = "10G base-T";
					else
						title = "WAN2";
				}
				else if(wans_dualwan_array[0] == "lan") {
					title = "LAN Port " + tm_nvram.wans_lanport;
					if (based_modelid == "TUF-AX4200" || based_modelid == "TUF-AX6000") {
						if (tm_nvram.wans_lanport == '5')
							title = "2.5G LAN";
					}
				}
				else if(wans_dualwan_array[0] == "dsl")
					title = "DSL WAN";
				else if(wans_dualwan_array[0] == "sfp+")
					title = "10G SFP+";
				else
					title = "<#dualwan_primary#>";
			}
			else
				title = "<#Internet#>";

			return title;

		case "INTERNET1":
			if(wans_dualwan_array[1] == "usb"){
				if(gobi_support)
					title = "<#Mobile_title#>";
				else
					title = "USB Modem";
			}
			else if(wans_dualwan_array[1] == "wan"){
				title = "WAN";
				if (based_modelid == "TUF-AX4200" || based_modelid == "TUF-AX6000")
					title = "2.5G WAN";
				if (based_modelid == "GT-AXY16000" || based_modelid == "RT-AX89U" || based_modelid == "TUF-AX4200" || based_modelid == "TUF-AX6000") {
					if (wan_bonding_support && tm_nvram.bond_wan == '1')
						title = "Bond";
				}
			}
			else if(wans_dualwan_array[1] == "wan2"){
				if (based_modelid == "GT-AXY16000" || based_modelid == "RT-AX89U")
					title = "10G base-T";
				else
					title = "WAN2";
			}
			else if(wans_dualwan_array[1] == "lan") {
				title = "LAN Port " + tm_nvram.wans_lanport;
				if (based_modelid == "TUF-AX4200" || based_modelid == "TUF-AX6000") {
					if (tm_nvram.wans_lanport == '5')
						title = "2.5G LAN";
				}
			}
			else if(wans_dualwan_array[1] == "sfp+")
				title = "10G SFP+";
			else
				title = "<#dualwan_secondary#>";

			return title;

		case "BRIDGE":
			return "LAN";
		case "WIRED":
			return "<#tm_wired#>";

		case "WIRELESS0":
		case "WIRELESS1":
		case "WIRELESS2":
		case "WIRELESS3":
			var num = ifname.substr(8);
			return "Wireless " + wl_nband_title[num];
	}

/* Handle multi-instanced interfaces */
	if (ifname.search(/WAGGR/) > -1){
		var bs_port_id = ifname.substr(5);
		if (bs_port_id == 0)
			return "bond-slave (WAN)";
		else if (bs_port_id >= 1 && bs_port_id <= 8)
			return "bond-slave (LAN Port "+bs_port_id+")";
		else if (bs_port_id == 30)
			return "bond-slave (10G base-T)";
		else if (bs_port_id == 31)
			return "bond-slave (10G SFP+)";
		else
			return "NotUsed";
	}
	else if (ifname.search(/LACPW/) > -1){
		var num = ifname.substr(5);
		return "bond-slave (WAN"+num+")";
	}
	else if (ifname.search("LACP") > -1){
		var num = ifname.substr(4);
		return "bond-slave (LAN Port "+num+")";
	}

	/* No friendly name, return as-is */
	return ifname;
}

function tm_switchPage(page, current){
	if (page == current) {
		return false;
	}

	switch (page) {
		case "1":
			location.href = "/Main_TrafficMonitor_realtime.asp";
			break;
		case "2":
			location.href = "/Main_TrafficMonitor_last24.asp";
			break;
		case "3":
			location.href = "/Main_TrafficMonitor_daily.asp";
			break;
		case "4":
			location.href = "/Main_TrafficMonitor_monthly.asp";
			break;
		case "5":
			location.href = "/Main_TrafficMonitor_settings.asp";
			break;
	}
}

/* isspeed: 0 = data (bytes), 1 = rate (bytes/s)  2 = bitrate (bits/s) */
function rescale_data_rate(value, isspeed){
	let unit = "K";
	let formattedValue;

	if (isspeed == 2)
		value *= 8;	// Convert to bits

	value /= 1024;

	if (value > 1024) {
		value /= 1024;
		unit = "M";
	}

	if (value > 1024) {
		value /= 1024;
		unit = "G";
	}

	switch(isspeed){
		case 1:
			unit += "B/s";
			break;
		case 2:
			unit += "bps";
			break;
		case 0:
		default:
			unit += "B";
	}

	/* Output formatting logic:  0 returns 0, above that but below 10 returns X.XX, the rest returns X.  Adds more granularity to small numbers */
	if (value < 10 && value !== 0) {
		formattedValue = value.toLocaleString(ui_locale, { minimumFractionDigits: 2, maximumFractionDigits: 2 });
	} else {
		formattedValue = value.toLocaleString(ui_locale, { maximumFractionDigits: 0 });
	}
	return String(formattedValue) + " " + unit;
}


function generateMonthsLabels(){
	let months = [];
	for (let i = 0; i < 12; i++) {
		months.push(
			new Date(2000, i, 1).toLocaleString(ui_locale, { month: 'short' })
		);
	}
	return months;
}


function rescale_value(value, scale){
	return (Number(value / scaleFactors[scale]).toLocaleString(ui_locale, { minimumFractionDigits: 2, maximumFractionDigits: 2 })) + " " + scaleNames[scale];
}
