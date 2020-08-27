/*

Old naming rule:
1. "-" to ""
2. "+" to "Plus"

New naming rule:
1. " " "/" "-" "_" "&" "!" to "-"
2. "+" to "-plus"

*/

var old_model = [ 
	"",
	"DSL-AC68U", "DSL-N55U", "DSL-N55U-B", "PL-N12", "RT-AC1200HP", "RT-AC3200", "RT-AC51U", "RT-AC52U", "RT-AC53U", "RT-AC54U", 
	"RT-AC55U", "RT-AC55UHP", "RT-AC56U", "RT-AC66U", "RT-AC68U", "RT-AC87U", "RT-N12", "RT-N14UHP", "RT-N15U", "RT-N16", 
	"RT-N18U", "RT-N53", "RT-N56U", "RT-N56UB1", "RT-N65U", "RT-N66U",
	""
];
var old_string = old_model.join("#");


function supportsite_model(support_site_modelid, hwver){

var real_model_name = "";

var _key_model = "#"+support_site_modelid+"#";

if(old_string.search(_key_model) >= 0){
	real_model_name = support_site_modelid.replace("-", "");
	real_model_name = real_model_name.replace("+", "Plus");

	if(support_site_modelid.search("DSL-N55U") != -1){
		real_model_name = "DSLN55U_Annex_A";
	}
	else if(support_site_modelid.search("DSL-N55U-B") != -1){
		real_model_name = "DSLN55U_Annex_B";
	}
	else if(support_site_modelid == "RT-N56UB1"){	//MODELDEP : RT-N56UB1
		real_model_name = "RTN56U_B1";
	}
	else if(support_site_modelid == "RT-N56UB2"){	//MODELDEP : RT-N56UB2
		real_model_name = "RTN56U_B2";
	}
	else if(support_site_modelid.search("RT-N12E_B") != -1){    //RT-N12E_B or RT-N12E_B1
		real_model_name = "RTN12E_B1"; 
	}
	else if(support_site_modelid =="RT-N12" || hw_ver.search("RTN12") != -1){	//MODELDEP : RT-N12
		if( hw_ver.search("RTN12HP") != -1){	//RT-N12HP
			real_model_name = "RTN12HP";
		}
		else if(hw_ver.search("RTN12B1") != -1){ //RT-N12B1
			real_model_name = "RTN12_B1";
		}
		else if(hw_ver.search("RTN12C1") != -1){ //RT-N12C1
			real_model_name = "RTN12_C1";
		}	
		else if(hw_ver.search("RTN12D1") != -1){ //RT-N12D1
			real_model_name = "RTN12_D1";
		}
	}
	else if(support_site_modelid == "RT-AC68R"){	//MODELDEP : RT-AC68R
		real_model_name = "RT-AC68R";
	}
	else if(support_site_modelid == "RT-AC1900P"){ //MODELDEP : RT-AC1900P
		real_model_name = "RT-AC1900P";
	}
	else if(support_site_modelid == "RT-AC66U_B1" || support_site_modelid == "RT-AC1750_B1"){     //MODELDEP : RT-AC66U B1
		real_model_name = "RT-AC66U-B1";
	}
	else if(support_site_modelid == "RT-N66U_C1"){	//MODELDEP : RT-N66U_C1
		real_model_name = "RT-N66U-C1";
	}	
}
else{
	real_model_name = support_site_modelid.replace(" ", "-");
	real_model_name = real_model_name.replace("/", "-");
	real_model_name = real_model_name.replace("_", "-");
	real_model_name = real_model_name.replace("&", "-");
	real_model_name = real_model_name.replace("!", "-");
	real_model_name = real_model_name.replace("+", "-plus");

	if(support_site_modelid.search("BLUE-CAVE") != -1 || support_site_modelid.search("BLUECAVE") != -1){    //BLUE_CAVE
    	real_model_name = "Blue-Cave";
	}
	else if(support_site_modelid.search("LYRA-VOICE") != -1){    //Lyra Voice
    	real_model_name = "Lyra-Voice";
	}
	else if(support_site_modelid.search("Lyra-Mini") != -1){    //Lyra mini
    	real_model_name = "Lyra-mini";
	}
	else if(support_site_modelid.search("GT-") != -1){    //ROG models
		real_model_name = real_model_name.replace("GT-", "ROG-Rapture-GT-");
	}
	else if(support_site_modelid.search("TUF-") != -1){	   //TUF models
		real_model_name = real_model_name.replace("TUF-", "TUF-Gaming-");
	}
	else if(support_site_modelid.search("ZenWiFi_XT") != -1){    //ZenWiFi AX
		real_model_name = real_model_name.replace("ZenWiFi-XT", "ZenWiFi-AX-XT");
	}
	else if(support_site_modelid.search("ZenWiFi_CT") != -1){    //ZenWiFi AC
		real_model_name = real_model_name.replace("ZenWiFi-CT", "ZenWiFi-AC-CT");
	}	
}

return real_model_name;
}	
		
