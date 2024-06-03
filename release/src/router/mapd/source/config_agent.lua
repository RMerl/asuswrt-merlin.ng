--[[
 * ***************************************************************************
 * *  Mediatek Inc.
 * * 4F, No. 2 Technology 5th Rd.
 * * Science-based Industrial Park
 * * Hsin-chu, Taiwan, R.O.C.
 * *
 * * (c) Copyright 2002-2019, Mediatek, Inc.
 * *
 * * All rights reserved. Mediatek's source code is an unpublished work and the
 * * use of a copyright notice does not imply otherwise. This source code
 * * contains confidential trade secret material of Ralink Tech. Any attemp
 * * or participation in deciphering, decoding, reverse engineering or in any
 * * way altering the source code is stricitly prohibited, unless the prior
 * * written consent of Mediatek, Inc. is obtained.
 * ***************************************************************************
 *
 *  Module Name:
 *  MAP Config Agent
 *
 *  Abstract:
 *  MAP Config Agent
 *
 *  Revision History:
 *  Who         When          What
 *  --------    ----------    -----------------------------------------
 *  Kapil.Gupta 2019/07/18    In-device implementation of MAP Config agent
 * ***************************************************************************
]]
-- env variables
WTS_CONF_FILE = "/etc/map/wts_bss_info_config"
SEND_1905_FRAME_CONF = "/etc/dev_send_1905_config"
P1905_DEBUG_LEVEL = "1"
WAPP_DEBUG_LEVEL = "1"
MAPD_DEBUG_LEVEL = "1"
CONFIG_AGENT_LOGS_FILE = "/tmp/config_agent.log"
MyDevRole = "agent" --default role, will be overwritten
-- from 0,3
MAX_NUM_MBSS = 3
-- from 0,2
MAX_NUM_RADIO = 2
L1_PROFILE_FILE = "/etc/wireless/l1profile.dat"

function logger(string)

	if (string == nil) then
		return
	end
	string = "config_agent: ".. string
	print(string)
	-- appends logs to logger file
	io.write(string)
	io.write('\n')
end

function os.capture(cmd, raw)
	local f = assert(io.popen(cmd, 'r'))
	local s = assert(f:read('*a'))
	f:close()
	if raw then return s end
	s = string.gsub(s, '^%s+', '')
	s = string.gsub(s, '%s+$', '')
	s = string.gsub(s, '[\n\r]+', ' ')
	return s
end

function sleep(n)
	execute("sleep " .. tonumber(n))
end

function execute(cmd)

	local log_string = ""
	if (cmd == nil) then
		logger("cannot execute nill command")
		return
	end
	log_string = "executing: " .. cmd
	logger(log_string)
	os.execute(cmd)
end

function IfaceUp(Iface)
	execute("ifconfig "..Iface.." up")
end

function IfaceDown(Iface)
	execute("ifconfig "..Iface.." down")
end

function IfaceDownUp(Iface, IfUp)
	if (IfUp == true) then
		IfaceUp(Iface)
	elseif (IfUp == false) then
		IfaceDown(Iface)
	else
		logger("invalid argument, bringing Interface up")
		IfaceUp(Iface)
	end
end

function RadioApIfaceDownUp(ExtIfname, IfUp)
	local IfName
	if (ExtIfname ~= nil and ExtIfname ~= "") then
		for BssCnt=0, MAX_NUM_MBSS do
			IfName = ExtIfname ..""..BssCnt
			IfaceDownUp(IfName, IfUp)
		end
	end
end

function ApInterfaceDownUp(IfUp)

	local IfaceState = ""
	local BssCnt, RadioCnt
	local ExtIfname, IfName
	local start_pos, second_iface

	for RadioCnt = 0, MAX_NUM_RADIO do
		ExtIfname = os.capture("wificonf -f " ..L1_PROFILE_FILE.. " get INDEX"..RadioCnt.."_ext_ifname")

		if (string.find(ExtIfname, ";")) then
			start_pos = string.find(ExtIfname, ";")
		        first_iface = string.sub(ExtIfname, 0, start_pos-1)
			RadioApIfaceDownUp(first_iface, IfUp)
		        second_iface = string.sub(ExtIfname, start_pos+1, string.len(ExtIfname))
			RadioApIfaceDownUp(second_iface, IfUp)
		else
			RadioApIfaceDownUp(ExtIfname, IfUp)
		end
	end
end

function RadioStaDownUp(StaIfName, IfUp)
	local ifname
	if (StaIfName ~= nil and StaIfName ~= "") then
		ifname = StaIfName .."0"
		IfaceDownUp(ifname, IfUp)
	end
end

function StaInterfaceDownUp(IfUp)

	local IfaceState = ""
	local StaIfName, RadioCnt, ifname
	local start_pos, second_iface, first_iface

	for RadioCnt=0, MAX_NUM_RADIO do
		StaIfName = os.capture("wificonf -f " ..L1_PROFILE_FILE.. " get INDEX"..RadioCnt.."_apcli_ifname")
		if (string.find(StaIfName, ";")) then
			start_pos = string.find(StaIfName, ";")
		        first_iface = string.sub(StaIfName, 0, start_pos-1)
			RadioStaDownUp(first_iface, IfUp)
		        second_iface = string.sub(StaIfName, start_pos+1, string.len(StaIfName))
			RadioStaDownUp(second_iface, IfUp)
		else
			RadioStaDownUp(StaIfName, IfUp)
		end

	end
end

function RadioStaReset(StaIfName)
	if (StaIfName ~= nil and StaIfName ~= "") then
		StaIfName = StaIfName.."0"
		execute("iwpriv "..StaIfName.." set ApCliEnable=0")
		execute("iwpriv "..StaIfName.." set ApCliSsid=")
	end
end

function StaInterfaceReset()
	local StaIfName, RadioCnt, extName

	for RadioCnt=0, MAX_NUM_RADIO do
		extName = os.capture("wificonf -f " ..L1_PROFILE_FILE.. " get INDEX"..RadioCnt.."_apcli_ifname")
		if (string.find(extName, ";")) then
			start_pos = string.find(extName, ";")
		    first_iface = string.sub(extName, 0, start_pos-1)
			RadioStaReset(first_iface)
			second_iface = string.sub(extName, start_pos+1, string.len(extName))
			RadioStaReset(second_iface)
		else
			RadioStaReset(extName)
		end
	end
end

function DevResetDefaultMediatek (Program, DevRole)
	--[[ Concurrent Mode: CntlSupport:1 AgentSupport:1 MapRoot:1 MyDevRole:agent/controller
	Stand Alone Mode: CntlSupport0/1 AgentSupport:1/0 MapRoot:1 MyDevRole:agent/controller
	MAUT: CntlSupport:0 AgentSupport:1 MapRoot:0 MyDevRole:agent

	--]]
	local i, WappEnable
	MyDevRole = DevRole

	if (Program == nil or MyDevRole == nil) then
		logger("got incorrect params in set dev role")
	end

	execute("wappctrl ra0 map reset_default")
	sleep (12)
	-- set default MAP version as R2
	command = "wificonf -f /etc/map/1905d.cfg set map_ver R2"
	execute(command)
	if ((string.upper(Program) == "MAPR1") or (string.upper(Program) == "MAP")) then
		logger("setting map version as R1")
		command = "wificonf -f /etc/map/1905d.cfg set map_ver R1"
		execute(command)
		execute("iwpriv ra0 set mapR2Enable=0")  
		execute("iwpriv rai0 set mapR2Enable=0")
		execute("iwpriv rax0 set mapR2Enable=0")
		execute("iwpriv wlan0 set mapR2Enable=0")
		execute("iwpriv ra0 set mapTSEnable=0") 
		execute("iwpriv rai0 set mapTSEnable=0")
		execute("iwpriv rax0 set mapTSEnable=0")
		execute("iwpriv wlan0 set mapTSEnable=0")
	else
		execute("iwpriv ra0 set mapR2Enable=1")  
		execute("iwpriv rai0 set mapR2Enable=1")
		execute("iwpriv rax0 set mapR2Enable=1")
		execute("iwpriv wlan0 set mapR2Enable=1")
		execute("iwpriv ra0 set mapTSEnable=1") 
		execute("iwpriv rai0 set mapTSEnable=1")
		execute("iwpriv rax0 set mapTSEnable=1")
		execute("iwpriv wlan0 set mapTSEnable=1")
	end
	logger("DevResetDefaultMediatek ".. Program.. "   "..MyDevRole.."")
	--kill 1905 first, kill after inf down up will cause CNTL mid handling issue (new msg with little mid will be dropped)
	execute ("killall -15 p1905_managerd")
	if (string.match(Program, "map") ) then
		WappEnable = 1
	else
		logger("Program is "..Program)
	end

	-- Bring down LAN(data)
	if (string.match(DevRole, "agent")) then
		execute ("brctl delif br-lan "..LAN_INTERFACE.. "")
		execute ("sed -i /^lan=/c\\#lan= /etc/map/1905d.cfg")
	else
		execute ("brctl addif br-lan "..LAN_INTERFACE.. "")
		execute ("sed -i /^lan=/c\\lan="..LAN_INTERFACE.. " /etc/map/1905d.cfg")
		execute ("sed -i /^#lan=/c\\lan="..LAN_INTERFACE.. " /etc/map/1905d.cfg")
	end

	kill_map_deamons = "killall -15 mapd; killall -15 p1905_managerd; killall -15 wapp; rm -rf /tmp/wapp_ctrl"
	-- kill MAP deamons
	execute (kill_map_deamons)

	if (string.match(DevRole, "controller") ) then
		-- bringdown all interaces
		ApInterfaceDownUp(false)
		StaInterfaceDownUp(false)

		execute("wapp_openwrt.sh")
		sleep (5)
		execute("wappctrl ra0 map reset_default")
		sleep (12)

		command = "p1905_managerd -r0 -f \"/etc/map/1905d.cfg\" -F \"/etc/map/wts_bss_info_config\" -M -d "..P1905_DEBUG_LEVEL.." &"
		sleep(5)
		execute(command)
		command = "mapd -M > /dev/console &"
		execute(command)
		logger ("Stand Alone Controller")
	else
		-- bringup all interaces
		ApInterfaceDownUp(true)
		StaInterfaceReset()
		StaInterfaceDownUp(true)

		execute("wapp_openwrt.sh")
		sleep (5)

		execute("wappctrl ra0 map reset_default")
		execute("sleep 12")

		logger ("Stand Alone Agent")
	end

	return "Success"
end

function StartWpsRegistrationMediatek(Mode, Band)
	command = "wappctrl ra0 map trigger_wps "..Mode.. " "..Band.." "
	execute(command)

	return "successful"
end

function GetInfoMediatek(Value, Device, Comment)
	local version, model, vendor, iRet, model2
	--command = "iwpriv ra0 stat |grep driver_version | cut -c 17-999"

	-- let's take the SDK version instead from now on
	--
	command = "cat /etc/sdk.version"
	version = os.capture(command)
	logger ("Cmd Result ".. version)
	command = "wificonf -f /etc/wireless/l1profile.dat get INDEX0"
	model = os.capture(command)
	if (string.match (model,"MT7622")) then
		command = "wificonf -f /etc/wireless/l1profile.dat get INDEX1"
		model2 = os.capture(command)
	end
	vendor = "MediaTek"
	if (string.match (model,"MT7622")) then
		if (string.match (model2,"MT7915")) then
			iRet = "vendor,"..vendor..",model,"..model2..",version,"..version
		else
			iRet = "vendor,"..vendor..",model,"..model..",version,"..version
		end
	else
		iRet = "vendor,"..vendor..",model,"..model..",version,"..version
	end
	return iRet
end

function GetMacAddrBySSIDMediatek (SSID, RUID)
	logger ("GetMacAddrBySSIDMediatek: "..SSID.."  " ..RUID)

	extracted_RUID = string.gsub(RUID, "0x", "")

	logger("SSID-"..SSID.." extracted_RUID -"..extracted_RUID)

	command = "wappctrl ra0 map get_macaddr "..SSID.." " ..extracted_RUID..";"
	execute(command)

	command = "cat /tmp/map_macaddr.txt |grep HWAddr | awk '{print $2}'"
	iRet = os.capture(command)
	logger ("return: -"..iRet)
	return iRet
end

function DevGetParameterMediatek(Value, Ruid, MatchSSID)

	local Param = Value
	local wapp_param

	logger ("DevGetParameter - "..Param)
	if string.match (Param, "FronthaulRadio_Supt") then
		wapp_param = "fh_radio_supt"
	elseif string.match (Param, "BackhaulSTA_Supt") then
		wapp_param = "bh_sta_radio"
	elseif string.match (Param, "FronthaulRadio_24G_BSSID") then
		wapp_param = "fh_24g_bssid"
	elseif string.match (Param, "FronthaulRadio_5G1_BSSID") then
		wapp_param = "fh_5g1_bssid"
	elseif string.match (Param, "FronthaulRadio_5G2_BSSID") then
		wapp_param = "fh_5g2_bssid"
	elseif string.match (Param, "HTCap_24G_Supt") then
		wapp_param = "ht_24g_supt"
	elseif string.match (Param, "HECap_24G_Supt") then
		wapp_param = "he_24g_supt"
	elseif string.match (Param, "HTCap_5G1_Supt") then
		wapp_param = "ht_5g1_supt"
	elseif string.match (Param, "HTCap_5G2_Supt") then
		wapp_param = "ht_5g2_supt"
	elseif string.match (Param, "VHTCap_5G1_Supt") then
		wapp_param = "vht_5g1_supt"
	elseif string.match (Param, "VHTCap_5G2_Supt") then
		wapp_param = "vht_5g2_supt"
	elseif string.match (Param, "HECap_5G1_Supt") then
		wapp_param = "he_5g1_supt"
	elseif string.match (Param, "HECap_5G2_Supt") then
		wapp_param = "he_5g2_supt"
	elseif string.match (Param, "backhaultype") then
		wapp_param = "bh_type"
	elseif (string.match ("CtrlAlid", Param)) then
		wapp_param = "ctrl_alid"
	elseif (string.match ("AgntAlid", Param)) then
		wapp_param = "agnt_alid"
	elseif (string.match (Param, "ALid")) then
		if string.match (MyDevRole, "controller") then
			wapp_param = "ctrl_alid"
		elseif string.match (MyDevRole, "agent") then
			wapp_param = "agnt_alid"
		else
			wapp_param = "agnt_alid"
		end
	elseif ((string.match (Param, "macaddr")) or (string.match (Param, "bssid"))) then
		MacAddr = GetMacAddrBySSIDMediatek(MatchSSID, Ruid)
		logger ("!!!!!!!!!!!!!!!!!!!!!!mac -"..MacAddr)
		return "macaddr,"..MacAddr
	end

	command = "wappctrl ra0 map show_param; cat /tmp/map_cfg.txt | grep "..wapp_param.." | awk '{print $2}'"
	iRet = os.capture(command)

	if (string.match (Param, "ALid")) then
		logger ("DevGetParameter - " ..Param .." value - "..iRet)
		return "ID,"..iRet
	end

	return iRet
end

function DevSetBackhaulMediatek (Program, Backhaul)
	logger ("DevSetBackhaulMediatek -"..Program.." backhaul:"..Backhaul)

	command = "mbo_nr.sh 3"
	execute(command)
	sleep (2)
	if (string.match(Backhaul, "eth")) then
		execute ("brctl addif br-lan "..LAN_INTERFACE.. "")
		execute ("sed -i /^lan=/c\\lan="..LAN_INTERFACE.. " /etc/map/1905d.cfg")
		execute ("sed -i /^#lan=/c\\lan="..LAN_INTERFACE.. " /etc/map/1905d.cfg")
		sleep(1)

		command = "p1905_managerd -r1 -f \"/etc/map/1905d.cfg\" -F \"/etc/map/wts_bss_info_config\" -M -d "..P1905_DEBUG_LEVEL.." &"
		execute(command)
		sleep (10)
		command = "mapd -M > /dev/console &"
		execute(command)
		sleep (10)
		command = "wappctrl ra0 map set_bh_type "..Backhaul
		execute(command)
		sleep (50)
	else
		extracted_RUID = string.gsub(Backhaul, "0x", "")
		logger ("extracted_RUID "..extracted_RUID)
		command = "p1905_managerd -r1 -f \"/etc/map/1905d.cfg\" -F \"/etc/map/wts_bss_info_config\" -M -d "..P1905_DEBUG_LEVEL.." &"
		execute(command)

		sleep (10)
		command = "mapd -M > /dev/console &"
		execute(command)
		sleep (10)
		command = "wappctrl ra0 map set_bh_type wifi "..extracted_RUID
		execute(command)
	end

	return "success"
end

function DevSend1905Mediatek(cmd)
	local role = 0
	local len = 100

	if (string.match(MyDevRole, "agent")) then
		role = 1
	end

	local tlv_found
	local i = 1, tlv_value, data
	local alid, msg_type, start_pos, tlv_type, tlv_length, next_pos, tlv

	cmd = string.gsub(cmd, "dev_send_1905,", "")

	start_pos = string.find (cmd, "DestALid,")
	if (start_pos ~= nil) then
		alid = string.sub(cmd, start_pos + 9, start_pos + 9 + 16)
	end

	start_pos = string.find (cmd, "MessageTypeValue,")
	if (start_pos ~= nil) then
		msg_type = string.sub(cmd, start_pos + 17, start_pos + 17 + 5)
	end

	data = alid .. " " .. msg_type

	command = "rm -rf "..SEND_1905_FRAME_CONF.."; echo "..data.." > "..SEND_1905_FRAME_CONF..";"
	execute(command)

	-- validate which kind of msg it is(tlv1/tlv)
	start_pos = string.find(cmd, "tlv_type1")
	tlv_found = string.find(cmd, "tlv_type")
	if (start_pos == nil and tlv_found ~= nil) then
		tlv_type = string.sub(cmd, (string.find(cmd, "tlv_type") + 9),
					(string.find(cmd, "tlv_type") + 12))
		tlv_length = string.sub(cmd, (string.find(cmd, "tlv_length") + 11),
					(string.find(cmd, "tlv_length") + 16))

		tlv_value = string.sub(cmd, (string.find(cmd, "tlv_value") + 10))

		--[[ Case: Macaddress string as raw data.
		     Ideally this should have come as raw data
		     but if coming as string, convert it to raw data.
		  ]]
		if (string.match(tlv_value, "(%w+:%w+:%w+:%w+:%w+:%w+)")) then
			-- append 0x for first octact.
			tlv_value = "0x"..tlv_value
		end
		tlv_value = string.gsub(tlv_value, ":", " 0x")

		tlv_value = string.gsub(tlv_value, "{", "")
		tlv_value = string.gsub(tlv_value, "}", "")
		tlv_value = string.gsub(tlv_value, " ", "_")

		tlv = tlv_type.. " "..tlv_length.." "..tlv_value

		command = "echo "..tlv.." >> "..SEND_1905_FRAME_CONF..";"

		execute(command)
	else
		for i = 1,6 do
			start_pos = string.find(cmd, "tlv_type"..i)
			if (start_pos == nil) then break end

			tlv_type = string.sub(cmd, (string.find(cmd, "tlv_type"..i) + 10),
							(string.find(cmd, "tlv_type"..i) + 13))
			tlv_length = string.sub(cmd, (string.find(cmd, "tlv_length"..i) + 12),
							(string.find(cmd, "tlv_length"..i) + 17))

			next_pos = string.find(cmd, "tlv_type"..i+1)

			if (next_pos) then next_pos = next_pos - 2 end

			tlv_value = string.sub(cmd, (string.find(cmd, "tlv_value"..i) + 11), next_pos)
			if (string.match(tlv_value, "(%w+:%w+:%w+:%w+:%w+:%w+)")) then
				-- append 0x for first octact.
				tlv_value = "0x"..tlv_value
			end
			tlv_value = string.gsub(tlv_value, "{", "")
			tlv_value = string.gsub(tlv_value, "}", "")
			tlv_value = string.gsub(tlv_value, " ", "_")
			tlv_value = string.gsub(tlv_value, ":", "")
			
			tlv = tlv_type.. " "..tlv_length.." "..tlv_value

			command = "echo "..tlv.." >> "..SEND_1905_FRAME_CONF..";"
			execute(command)
		end
	end

	if role > 0 then
		command = "1905ctrl agent dev_send_1905 "..SEND_1905_FRAME_CONF
	else
		command = "1905ctrl controller dev_send_1905 "..SEND_1905_FRAME_CONF
	end
	execute(command)

	sleep (1)

	command = "cat /tmp/msg_id.txt | awk '{print $2}'"
	iRet = os.capture(command)

	return iRet

end

function GetInterfaceFromBssid(bssid)
	local interface

	command = "iwconfig | grep -i "..bssid.." -B 2 > /tmp/map_cfg.txt"
	execute(command)
	command = "cat /tmp/map_cfg.txt | grep -vi -e "..bssid.." -e wds -e -- -e '         ' > /tmp/1.txt"
	execute(command)
	command = "cat /tmp/1.txt  |  awk '{print $1}'"
	interface = os.capture(command, nil)

	if (interface ~= nil) then
		logger("interface is " ..interface)
	else
		logger("interface not found")
	end
	return interface
end

function setAssoc_Disallow (bssid, Value)
	local interface
	logger("setAssoc_Disallow: Value="..Value)

	if string.match(Value, "Enable") then
		Disallow_Reason = "1"
	elseif string.match(Value, "Disable") then
		Disallow_Reason = "0"
	else
		logger("invalid reason")
		Disallow_Reason = "0"
	end

	interface = GetInterfaceFromBssid(bssid)

	if (interface == nil) then
		return "-1"
	end

	if (MBOWappCommands and MBOWappCommands["Assoc_Disallow"]) then
		command = "wappctrl "..interface.." "..MBOWappCommands["Assoc_Disallow"].." "..Disallow_Reason
		execute (command)
	end

	return "Success"
end

-- Just a basic function for all map handling, should be verified each function and add handling
-- if needed.
function execute_wapp_command(parameter, bssid, Value)

	local interface

	if string.match(Value, "Enable") then
		Value = "1"
	elseif string.match(Value, "Disable") then
		Value = "0"
	end

	interface = GetInterfaceFromBssid(bssid)
	if (MBOWappCommands[parameter]) then
		command = "wappctrl "..interface.." "..MBOWappCommands[parameter].." "..Value
		execute (command)
		return "Success"
	else
		logger("command not found")
		return "failed"
	end

end

function DevSetConfigMediatek (cmd)

	--inform 1905 by wapp_ctrl
	command = "rm -rf " ..WTS_CONF_FILE.. "; echo \"#ucc_bss_info\" > "..WTS_CONF_FILE.." "
	execute(command)

	--parsing bss_info into /etc_ro/wts_bss_info_config

	local i = 1
	local trim_len = 0
	cmd = string.gsub(cmd, "dev_set_config,", "")
	bss_info = null

	for i = 1,16 do
		start_pos = string.find (cmd, "bss_info"..i)
		end_pos = string.find(cmd, "bss_info".. (i+1))
		if (start_pos == nil) then break end
		if (i > 9) then
			trim_len = 11
		else
			trim_len = 10
		end
		if (end_pos) then
			bss_info = string.sub(cmd, start_pos + trim_len, end_pos - 2)
		else
			bss_info = string.sub(cmd, start_pos + trim_len)
		end
		bss_info = string.gsub(bss_info, "{", "")
		bss_info = string.gsub(bss_info, "}", "")
		bss_info = string.gsub(bss_info, ",", " ")
		bss_info = string.gsub(bss_info, "tlv_type1", "")
		bss_info = string.gsub(bss_info, "tlv_type2", "")
		bss_info = string.gsub(bss_info, "tlv_length1", "")
		bss_info = string.gsub(bss_info, "tlv_length2", "")
		bss_info = string.gsub(bss_info, "tlv_value1", "")
		bss_info = string.gsub(bss_info, "tlv_value2", "")
		logger(i..","..bss_info)
		command = "echo "..i ..",".. bss_info .. "  >> "..WTS_CONF_FILE..";"
		os.execute(command)
	end

	sleep (1)
	--inform 1905
	command = "1905ctrl controller dev_set_config "..WTS_CONF_FILE
	execute(command)

	sleep (1)

	return "config_applied"
end

-- This function should be extended for all the config agent functionality
-- Since UCC commands comes in string txt, scipts are better way to handle
-- these commands rather than C
function process_ucc_command(line)

	local Program, iRet = ""

	if (line == nil) then
		return
	end
	RESULT = "Status,Error"
	logger(line)

	if (string.match(line, "dev_reset_default,")) then
		line = string.gsub(line, "dev_reset_default,", "")
		for k, v in string.gmatch(line, "(%w+),(%w+)") do
			if (k == "name") then
				Name = v
			end
			if (k == "program") then
				Program = v
			end
			if (k == "devrole") then
				DevRole = v
			end
		end
		iRet = DevResetDefaultMediatek(Program, DevRole)
		RESULT = "status,COMPLETE,"..iRet
		return RESULT

	elseif (string.match(line, "ap_reset_default,")) then
		return "status,COMPLETE,0"
	elseif (string.match(line, "start_wps_registration,")) then
		line = string.gsub(line, "start_wps_registration,", "")
		for k, v in string.gmatch(line, "(%w+),(%w+)") do
			if (k == "ALid") then
				ALid = v
			end
			if (k == "WpsConfigMethod") then
				Mode = v
			end
			if (k == "band") then
				Band = v
			end
		end
		iRet = StartWpsRegistrationMediatek(Mode, Band)
		RESULT = "status,COMPLETE,wps started"
		return RESULT
	elseif (string.match(line, "device_get_info")) then
		iRet = GetInfoMediatek(NULL, "Mediatek", NULL)
		RESULT = "status,COMPLETE,"..iRet
		return RESULT
	elseif (string.match(line, "dev_get_parameter,")) then
		line = string.gsub(line, "dev_get_parameter,", "")
		local ruid, ssid, parameter
                if string.match(line, "ruid") then
                        start_pos = string.find(line, "ruid,")
                        end_pos = string.find(line, "," , start_pos + 5)
                        ruid = string.sub(line, start_pos + 5, end_pos - 1)
                end
                if string.match(line, "ssid") then
                        start_pos = string.find(line, "ssid,")
                        end_pos = string.find(line, "," , start_pos + 5)
                        ssid = string.sub(line, start_pos + 5, end_pos - 1)
                end

                if string.match(line, "parameter") then
                        start_pos = string.find(line, "parameter,")
                        end_pos = string.find(line, "," , start_pos + 10)
                        parameter = string.sub(line, start_pos + 10, end_pos)
                end

		if (ssid == nil) then
			ssid = "BHSTA"
		end
		iRet = DevGetParameterMediatek(parameter, ruid, ssid)
		RESULT = "status,COMPLETE,"..iRet
		return RESULT
	elseif (string.match(line, "dev_set_config,")) then
		line = string.gsub(line, "dev_set_config,", "")
		logger( "dev_set_config " ..line)
		--for k, v in string.gmatch(line, "(%w+),(%w+)") do
		for k, v in string.gmatch(line, "(%w+_%w+),(%w+)") do
			if (k == "program") then
				Program = v
			end
			if (k == "bss_info1") then
				iRet = DevSetConfigMediatek(line)
				RESULT = "status,COMPLETE,"..iRet
				return RESULT
			end
		end
		for k, v in string.gmatch(line, "(%w+),(%w+)") do
			if (k == "program") then
				Program = v
			end
			if (k == "backhaul") then
				backhaul = v
				iRet = DevSetBackhaulMediatek(Program, backhaul)
				RESULT = "status,COMPLETE,"..iRet
				return RESULT
			end
		end

	elseif (string.find(line, "dev_send_1905") == 1) then
		iRet = DevSend1905Mediatek(line)
		RESULT = "status,COMPLETE,MID,"..iRet
		return RESULT
	elseif (string.find(line, "ap_ca_version") == 1) then
		RESULT = "status,COMPLETE,Wi-Fi Test Suite-9.0.0"
		return RESULT
	elseif (string.find(line, "ca_get_version") == 1) then
		RESULT = "status,COMPLETE,Wi-Fi Test Suite-9.3.1"
		return RESULT
	elseif (string.find(line, "dev_set_rfeature") == 1) then
                local local_program, bssid, parameter, value
                if string.match(line, "type") then
                        start_pos = string.find(line, "type,")
                        end_pos = string.find(line, "," , start_pos + 5)
                        local_program = string.sub(line, start_pos + 5, end_pos - 1)
                end
                if string.match(line, "bssid") then
                        start_pos = string.find(line, "bssid,")
                        end_pos = string.find(line, "," , start_pos + 6)
                        bssid = string.sub(line, start_pos + 6, end_pos - 1)
                end

                start_pos = end_pos + 1
                end_pos = string.find(line, "," , start_pos)
                parameter = string.sub(line, start_pos, end_pos - 1)

                start_pos = end_pos + 1
                end_pos = string.find(line, "," , start_pos)
                if (end_pos == nil) then
                        end_pos = string.len(line)
                end
                value = string.sub(line, start_pos, end_pos)

                local fn = _G["set"..parameter]
		if (fn) then
	                iRet = fn(bssid, value)
		else
			logger("direct function handler not found, execute generic handler")
			iRet = execute_wapp_command(parameter, bssid, value)
		end
		RESULT = "status,COMPLETE,"..iRet
		return RESULT
	elseif (string.find(line, "dev_send_frame") == 1) then
		logger("command not supported, got :"..line)
	elseif (string.find(line, "ap_send_tsmrpt_req") == 1) then
		logger("command not supported, got :"..line)
	elseif (string.find(line, "ap_deauth_sta") == 1) then
		logger("command not supported, got :"..line)
	elseif (string.find(line, "ap_send_bcnrpt_req") == 1) then
		logger("command not supported, got :"..line)
	elseif (string.find(line, "ap_send_tsmrpt_req") == 1) then
		logger("command not supported, got :"..line)
	elseif (string.find(line, "ap_send_bsstrans_mgmt_req") == 1) then
		logger("command not supported, got :"..line)
	elseif (string.find(line, "ap_send_link_mea_req") == 1) then
		logger("command not supported, got :"..line)
	elseif (string.find(line, "ap_set_security") == 1) then
		logger("command not supported, got :"..line)
	elseif (string.find(line, "ap_set") == 1) then
		logger("command not supported, got :"..line)
	elseif (string.find(line, "ap_get_mac_address") == 1) then
		logger("command not supported, got :"..line)
	elseif (string.find(line, "close_config_agent") == 1) then
		EXIT = 1
	else
		logger("command not supported, got :"..line)
	end

	return "OK"
end

function start_agent_server(conf)
	local socket = require("socket")
	local server = assert(socket.bind("*", conf.server_port))
	local tcp = assert(socket.tcp())

	logger("Started config server at "..conf.server_port.."")

	return server, tcp
end

function config_agent_main(conf)
	server, tcp = start_agent_server(conf)

	local result, err, partial
	local line = nil
	local client = nil
	while 1 do

		if client == nil then
			client = server:accept()
		end
		line, err, partial = client:receive()

		-- data received
		if line ~= nil then
			-- UCC needs this, no response will be processed without this
			client:send("status,RUNNING\n")
			-- pcall is for seg fault during processing, TODO add this for each fn call
			if (pcall(process_ucc_command(line))) then
				EXIT = 1
				break
			end

			logger(RESULT)
			client:send(RESULT.."\n")
		elseif err == "closed" then
			-- connection has closed, cleanup the client
			logger("connection is closed")
			client:close()
			client = nil
		elseif err == "timeout" then
			logger(err .. " partial: " .. partial)
			client:close()
			client = nil
		end

		if (EXIT == 1) then
			logger("closing tcp port and exiting, command : "..line)
			break
		end
		line = nil
	end

	if (tcp ~= nil) then
		tcp:close()
	end
	if (client ~= nil) then
		client:close()
	end
	if (server ~= nil) then
		server:close()
	end
end

MBOWappCommands = {}

MBOWappCommands["Cellular_Cap_Pref"] = "mbo ap_cdcp"
MBOWappCommands["BTMReq_DisAssoc_Imnt"] = "mbo disassoc_imnt"
MBOWappCommands["BTMReq_Term_Bit"] = "mbo bss_term_onoff"
MBOWappCommands["BSS_Term_Duration"] = "mbo bss_term_duration"
MBOWappCommands["BSS_Term_TSF"] =  "mbo bss_term_tsf"
MBOWappCommands["Assoc_Disallow"] = "mbo assoc_disallow"
MBOWappCommands["Nebor_BSSID"] =  "mbo nebor_bssid"
MBOWappCommands["Nebor_Op_Class"] =  "mbo nebor_op_class"
MBOWappCommands["Nebor_Op_Ch"] = "mbo nebor_op_ch"
MBOWappCommands["Disassoc_Timer"] = "mbo disassoc_timer"
MBOWappCommands["Assoc_Delay"] = "mbo retry_delay"
MBOWappCommands["Nebor_Pref"] = "mbo nebor_pref"
MBOWappCommands["Nebor_Test"] = "mbo add_test_nr"
MBOWappCommands["send_BTMReq"] = "mbo send_btm_req"
MBOWappCommands["Gas_CB_Delay"] = "set gas_cb_delay"
MBOWappCommands["Mpdu_Size"] = "set mmpdu_size"

-- read configurations here from nvram/db
local conf
conf = {}
RESULT = ""
EXIT = 0
-- global variable
log_file = ""

command = ""
conf.server_port = os.capture("wificonf -f /etc/map/1905d.cfg get config_agent_port")

if (conf.server_port == nil or conf.server_port == "") then
	conf.server_port = 9215
end

LAN_INTERFACE = os.capture("uci get network.lan.ifname")
if (LAN_INTERFACE == nil) then
	-- set default interface
	LAN_INTERFACE = "eth0"
end

-- remove older logs file
execute ("rm -rf "..CONFIG_AGENT_LOGS_FILE)

execute ("touch "..CONFIG_AGENT_LOGS_FILE)
-- Opens a file in append mode
log_file = io.open(CONFIG_AGENT_LOGS_FILE, "a")

-- sets the default output file as logger file
io.output(log_file)

-- run deamon
config_agent_main(conf)
