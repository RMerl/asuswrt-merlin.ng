cur_control_channel = [<% wl_control_channel(); %>][0];
if(based_modelid === 'GT-AXE16000'){
	var _t = cur_control_channel[3];
	cur_control_channel[3] = cur_control_channel[2];
	cur_control_channel[2] = cur_control_channel[1];
	cur_control_channel[1] = cur_control_channel[0];
	cur_control_channel[0] = _t;
}
