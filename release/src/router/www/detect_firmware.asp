﻿webs_state_update = '<% nvram_get("webs_state_update"); %>';
webs_state_error = '<% nvram_get("webs_state_error"); %>';
webs_state_info = '<% nvram_get("webs_state_info_am"); %>';
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
if(cfg_sync_support){
	cfg_check = '<% nvram_get("cfg_check"); %>';
	cfg_upgrade = '<% nvram_get("cfg_upgrade"); %>';
}
if(pipefw_support || urlfw_support){
	hndwr_status = '<% nvram_get("hndwr"); %>';
}
