webs_state_update = '<% nvram_get("webs_state_update"); %>';
webs_state_error = '<% nvram_get("webs_state_error"); %>';
webs_state_info = '<% nvram_get("webs_state_info"); %>';
webs_state_info_beta = '<% nvram_get("webs_state_info_beta"); %>';
webs_state_upgrade = '<% nvram_get("webs_state_upgrade"); %>';
fwdl_percent="<% get_fwdl_percent(); %>";
if(cfg_sync_support){
	cfg_check = '<% nvram_get("cfg_check"); %>';
	cfg_upgrade = '<% nvram_get("cfg_upgrade"); %>';
}
