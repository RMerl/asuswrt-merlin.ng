/*
New naming rule:
1. " " "/" "-" "_" "&" "!" to "-"
2. "+" to "-plus"
*/

function supportsite_model(support_site_modelid, hwver){
	
var real_model_name = "";
real_model_name = support_site_modelid.replace(" ", "-");
real_model_name = real_model_name.replace("/", "-");
real_model_name = real_model_name.replace("_", "-");
real_model_name = real_model_name.replace("&", "-");
real_model_name = real_model_name.replace("!", "-");
real_model_name = real_model_name.replace("+", "-plus");

if(support_site_modelid.search("BLUE-CAVE") != -1){    //BLUE_CAVE
    real_model_name = "Blue-Cave";
}
else if(support_site_modelid.search("GT-") != -1){    //ROG models
	real_model_name = "ROG-Rapture-"+real_model_name;
}
else if(support_site_modelid.search("LYRA-VOICE") != -1){    //Lyra Voice
    real_model_name = "Lyra-Voice";
}
else if(support_site_modelid.search("Lyra-Mini") != -1){    //Lyra mini
    real_model_name = "Lyra-mini";
}


return real_model_name;
}	
		
