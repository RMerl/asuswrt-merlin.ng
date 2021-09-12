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


function supportsite_model(support_site_modelid, hw_ver){

	var real_model_name = "";

	var _key_model = "#"+support_site_modelid+"#";

//https://www.asus.com/supportonly/DSL-N55U/HelpDesk_BIOS/
if(old_string.search(_key_model) >= 0){  //support only
	
	real_model_name = support_site_modelid.replace("-", "");
	real_model_name = real_model_name.replace("+", "Plus");

	if(support_site_modelid.search("DSL-N55U-B") != -1){
		real_model_name = "supportonly/DSL-N55U%20(VER.B1)/";
		alert(real_model_name);
	}
	else if(support_site_modelid =="RT-N12" || hw_ver.search("RTN12") != -1){	//MODELDEP : RT-N12
		if( hw_ver.search("RTN12HP") != -1){	//RT-N12HP
			real_model_name = "supportonly/RT-N12HP/";
		}
		else{
			real_model_name = "supportonly/"+real_model_name+"/";
		}
	}
	else if(support_site_modelid == "RT-AC68R"){	//MODELDEP : RT-AC68R
		real_model_name = "supportonly/RT-AC68R/";
	}
	else if(support_site_modelid == "RT-AC1900P"){ //MODELDEP : RT-AC1900P
		real_model_name = "supportonly/RT-AC1900P/";
	}
	else if(support_site_modelid == "RT-N66U_C1"){	//MODELDEP : RT-N66U_C1
		real_model_name = "supportonly/RT-N66U%20C1/";
	}
	else{
		real_model_name = "supportonly/"+real_model_name+"/";
	}
}
else{
	real_model_name = support_site_modelid.replace(" ", "-");
	real_model_name = real_model_name.replace("/", "-");
	real_model_name = real_model_name.replace("_", "-");
	real_model_name = real_model_name.replace("&", "-");
	real_model_name = real_model_name.replace("!", "-");
	real_model_name = real_model_name.replace("+", "-plus");

	if(support_site_modelid.search("LYRA_VOICE") != -1){    //Lyra Voice
		//real_model_name = "Lyra-Voice";
		real_model_name = "Networking-IoT-Servers/WiFi-Routers/ASUS-WiFi-Routers/Lyra-Voice/";

	}
	else if(support_site_modelid.search("BLUE_CAVE") != -1 || support_site_modelid.search("BLUECAVE") != -1){    //BLUE_CAVE
		//real_model_name = "Blue-Cave";
		real_model_name = "Networking-IoT-Servers/Whole-Home-Mesh-WiFi-System/AiMesh-WiFi-Routers-and-Systems/Blue-Cave/";
	}
	else if(support_site_modelid.search("Lyra_Trio") != -1){    //Lyra Trio
		//real_model_name = "Lyra-mini";
		real_model_name = "Networking-IoT-Servers/Whole-Home-Mesh-WiFi-System/AiMesh-WiFi-Routers-and-Systems/Lyra-Trio/";
	}
	else if(support_site_modelid.search("Lyra_Mini") != -1){    //Lyra Mini
		//real_model_name = "Lyra-mini";
		real_model_name = "Networking-IoT-Servers/Whole-Home-Mesh-WiFi-System/All-series/Lyra-mini/";

	}
	else if(support_site_modelid.search("Lyra") != -1){    //Lyra
		//real_model_name = "Lyra-mini";
		real_model_name = "Networking-IoT-Servers/Whole-Home-Mesh-WiFi-System/AiMesh-WiFi-Routers-and-Systems/Lyra/";

	}
	else if(support_site_modelid.search("ZenWiFi_XD4") != -1){
		real_model_name = "Networking-IoT-Servers/Whole-Home-Mesh-WiFi-System/ZenWiFi-WiFi-Systems/ASUS-ZenWiFi-AX-Mini-XD4/";
	}
	else if(support_site_modelid.search("ZenWiFi_CD6") != -1){
		real_model_name = "Networking-IoT-Servers/Whole-Home-Mesh-WiFi-System/ZenWiFi-WiFi-Systems/ASUS-ZenWiFi-AC-Mini-CD6/";
	}
	else if(support_site_modelid.search("ZenWiFi_XD6") != -1){
		real_model_name = "Networking-IoT-Servers/Whole-Home-Mesh-WiFi-System/ZenWiFi-WiFi-Systems/ASUS-ZenWiFi-XD6/";
	}
	else if(support_site_modelid.search("ZenWiFi_ET8") != -1){
		real_model_name = "Networking-IoT-Servers/Whole-Home-Mesh-WiFi-System/ZenWiFi-WiFi-Systems/ASUS-ZenWiFi-ET8/";
	}
	else if(support_site_modelid.search("ZenWiFi_C") != -1){    //ZenWiFi AC
		real_model_name = real_model_name.replace("ZenWiFi-", "ZenWiFi-AC-");
                real_model_name = "Networking-IoT-Servers/Whole-Home-Mesh-WiFi-System/ZenWiFi-WiFi-Systems/"+real_model_name+"/";
        }
	else if(support_site_modelid.search("ZenWiFi_X") != -1){    //ZenWiFi AX
		real_model_name = real_model_name.replace("ZenWiFi-", "ZenWiFi-AX-");
		real_model_name = "Networking-IoT-Servers/Whole-Home-Mesh-WiFi-System/ZenWiFi-WiFi-Systems/"+real_model_name+"/";

	}
	else if(support_site_modelid.search("GS-") != -1){
		real_model_name = "networking/rog-strix-"+real_model_name+"-model/";

	}
	else if(support_site_modelid.search("GT-") != -1){
		real_model_name = "networking/rog-rapture-"+real_model_name+"-model/";

	}
	else if(support_site_modelid.search("TUF-") != -1){	   //TUF AX models
		real_model_name = real_model_name.replace("TUF-", "TUF-Gaming-");
		real_model_name = "Networking-IoT-Servers/WiFi-6/All-series/"+real_model_name+"/";

	}
	else if(support_site_modelid.search("RP-") != -1){
		real_model_name = "Networking-IoT-Servers/Range-Extenders-/All-series/"+real_model_name+"/";

	}
	else if(support_site_modelid.search("RT-AX") != -1){	   // AX for WiFi-6
		real_model_name = "Networking-IoT-Servers/WiFi-6/All-series/"+real_model_name+"/";
	}
	else if(support_site_modelid.search("RT-AC") != -1){	   // AC for WiFi-Routers
		if(support_site_modelid == "RT-AC66U_B1" || support_site_modelid == "RT-AC1750_B1"){     //MODELDEP : RT-AC66U B1
			real_model_name = "Networking-IoT-Servers/WiFi-Routers/ASUS-WiFi-Routers/RT-AC66U-B1";
		}
		else{
			real_model_name = "Networking-IoT-Servers/WiFi-Routers/ASUS-WiFi-Routers/"+real_model_name+"/";
		}
	}
	else if(support_site_modelid.search("RT-N") != -1){	   // N for WiFi-Routers
		real_model_name = "Networking-IoT-Servers/WiFi-Routers/ASUS-WiFi-Routers/"+real_model_name+"/";
	}
	else{
		real_model_name = "supportonly/"+real_model_name+"/";
	}
}

return real_model_name;
}	
		
