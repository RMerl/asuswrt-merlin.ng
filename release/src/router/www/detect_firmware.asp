webs_state_update = '<% nvram_get("webs_state_update"); %>';
webs_state_error = '<% nvram_get("webs_state_error"); %>';
webs_state_info = '<% nvram_get("webs_state_info"); %>';
webs_state_info_beta = '<% nvram_get("webs_state_info_beta"); %>';
webs_state_REQinfo = '<% nvram_get("webs_state_REQinfo"); %>';
webs_state_flag = '<% nvram_get("webs_state_flag"); %>';
webs_state_upgrade = '<% nvram_get("webs_state_upgrade"); %>';
webs_state_level = '<% nvram_get("webs_state_level"); %>';

sig_state_flag = '<% nvram_get("sig_state_flag"); %>';
sig_state_update = '<% nvram_get("sig_state_update"); %>';
sig_state_upgrade = '<% nvram_get("sig_state_upgrade"); %>';
sig_state_error = '<% nvram_get("sig_state_error"); %>';
sig_ver = '<% nvram_get("bwdpi_sig_ver"); %>';
if(isSupport("gtbooster")){
	ark_state_update = '<% nvram_get("ark_state_update"); %>';
	ark_state_upgrade = '<% nvram_get("ark_state_upgrade"); %>';
	ark_state_flag = '<% nvram_get("ark_state_flag"); %>';
	ark_sig_state = '<% nvram_get("ark_sig_state"); %>';
	ark_sig_unzip = '<% nvram_get("ark_sig_unzip"); %>';
	ark_sig_ver_tmp = '<% nvram_get("ark_sig_ver"); %>';
	ark_sig_ver = ark_sig_ver_tmp.split("_").slice(0, 2).join("_");
}
if(cfg_sync_support){
	cfg_check = '<% nvram_get("cfg_check"); %>';
	cfg_upgrade = '<% nvram_get("cfg_upgrade"); %>';
}
if(pipefw_support || urlfw_support){
	hndwr_status = '<% nvram_get("hndwr"); %>';
}
if(rbkfw_support){
	_rollback_info = JSON.parse('<% get_rbkList(); %>');
	rbk_count = Object.keys(_rollback_info).length;
}
