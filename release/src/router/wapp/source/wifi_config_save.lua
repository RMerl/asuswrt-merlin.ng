#!/usr/bin/lua
require("datconf")

local l1dat_parser = {
    L1_DAT_PATH = "/etc/wireless/l1profile.dat",
    IF_RINDEX = "ifname_ridx",
    DEV_RINDEX = "devname_ridx",
    MAX_NUM_APCLI = 1,
    MAX_NUM_WDS = 4,
    MAX_NUM_MESH = 1,
    MAX_NUM_EXTIF = 16,
    MAX_NUM_DBDC_BAND = 2,
}

local l1cfg_options = {
            ext_ifname="",
            apcli_ifname="apcli",
            wds_ifname="wds",
            mesh_ifname="mesh"
      }

function l1dat_parser__trim(s)
  if s then return (s:gsub("^%s*(.-)%s*$", "%1")) end
end

function l1dat_parser__cfg2list(str)
    -- delimeter == ";"
    local i = 1
    local list = {}
    for k in string.gmatch(str, "([^;]+)") do
        list[i] = k
        i = i + 1
    end
    return list
end

function l1dat_parser_token_get(str, n, v)
    -- n starts from 1
    -- v is the backup in case token n is nil
    if not str then return v end
    local tmp = l1dat_parser__cfg2list(str)
    return tmp[tonumber(n)] or v
end

function l1dat_parser_add_default_value(l1cfg)
    for k, v in ipairs(l1cfg) do

        for opt, default in pairs(l1cfg_options) do
            if ( opt == "ext_ifname" ) then
                v[opt] = v[opt] or v["main_ifname"].."_"
            else
                v[opt] = v[opt] or default..k.."_"
            end
        end
    end

    return l1cfg
end

-- path to zone is 1 to 1 mapping
function l1dat_parser_l1_path_to_zone(path)
    --print("Enter l1dat_parser_l1_path_to_zone("..path..")<br>")
    if not path then return end

    local devs = l1dat_parser_load_l1_profile(l1dat_parser.L1_DAT_PATH)
    if not devs then return end

    for _, dev in pairs(devs[l1dat_parser.IF_RINDEX]) do
        if dev.profile_path == path then
            return dev.nvram_zone
        end
    end

    return
end

function l1dat_parser_l1_ifname_to_zone(ifname)
    if not ifname then return end

    local devs = l1dat_parser_load_l1_profile(l1dat_parser.L1_DAT_PATH)
    if not devs then return end

    local ridx = l1dat_parser.IF_RINDEX
    return devs[ridx][ifname] and devs[ridx][ifname].nvram_zone
end

-- input: L1 profile path.
-- output A table, devs, contains
--   1. devs[%d] = table of each INDEX# in the L1 profile
--   2. devs.ifname_ridx[ifname]
--         = table of each ifname and point to relevant contain in dev[$d]
--   3. devs.devname_ridx[devname] similar to devs.ifnameridx, but use devname.
--      devname = INDEX#_value.mainidx(.subidx)
-- Using *_ridx do not need to handle name=k1;k2 case of DBDC card.

function l1dat_parser_load_l1_profile()
    local devs = setmetatable({}, {__index=
                     function(tbl, key)
                           local util = require("luci.util")
                           --print("metatable function:", util.serialize_data(tbl), key)
                           --print("-----------------------------------------------")
                           if ( string.match(key, "^%d+")) then
                               tbl[key] = {}
                               return tbl[key]
                           end
                     end
                 })
    local nixio = require("nixio")
    local chipset_num = {}
    local dir = io.popen("ls /etc/wireless/")
    if not dir then return end
    local path = l1dat_parser.L1_DAT_PATH
    local fd = io.open(path, "r")
    if not fd then return end

    -- convert l1 profile into lua table
    for line in fd:lines() do
        line = l1dat_parser__trim(line)
        if string.byte(line) ~= string.byte("#") then
            local i = string.find(line, "=")
            if i then
                local k, v, k1, k2
                k = l1dat_parser__trim( string.sub(line, 1, i-1) )
                v = l1dat_parser__trim( string.sub(line, i+1) )
                k1, k2 = string.match(k, "INDEX(%d+)_(.+)")
                if k1 then
                    k1 = tonumber(k1) + 1
                    if devs[k1][k2] then
                        nixio.syslog("warning", "skip repeated key"..line)
                    end
                    devs[k1][k2] = v or ""
                else
                    k1 = string.match(k, "INDEX(%d+)")
                    k1 = tonumber(k1) + 1
                    devs[k1]["INDEX"] = v

                    chipset_num[v] = (not chipset_num[v] and 1) or chipset_num[v] + 1
                    devs[k1]["mainidx"] = chipset_num[v]
                end
            else
                nixio.syslog("warning", "skip line without '=' "..line)
            end
        else
            nixio.syslog("warning", "skip comment line "..line)
        end
    end

    l1dat_parser_add_default_value(devs)
    --local util = require("luci.util")
    --local seen2 = {}
    -- print("Before setup ridx", util.serialize_data(devs, seen2))

    -- Force to setup reverse indice for quick search.
    -- Benifit: 
    --   1. O(1) search with ifname, devname
    --   2. Seperate DBDC name=k1;k2 format in the L1 profile into each 
    --      ifname, devname.
    local dbdc_if = {}
    local ridx = l1dat_parser.IF_RINDEX
    local dridx = l1dat_parser.DEV_RINDEX
    local band_num = l1dat_parser.MAX_NUM_DBDC_BAND
    local k, v, dev, i , j, last
    local devname
    devs[ridx] = {}
    devs[dridx] = {}
    for _, dev in ipairs(devs) do
        dbdc_if[band_num] = l1dat_parser_token_get(dev.main_ifname, band_num, nil)
        if dbdc_if[band_num] then
            for i = 1, band_num - 1 do
                dbdc_if[i] = l1dat_parser_token_get(dev.main_ifname, i, nil)
            end
            for i = 1, band_num do 
                devs[ridx][dbdc_if[i]] = {}
                devs[ridx][dbdc_if[i]]["subidx"] = i
                
                for k, v in pairs(dev) do
                    if  k == "INDEX" or k == "EEPROM_offset" or k == "EEPROM_size"
                       or k == "mainidx" then
                        devs[ridx][dbdc_if[i]][k] = v
                    else
                        devs[ridx][dbdc_if[i]][k] = l1dat_parser_token_get(v, i, "")
                    end
                end
                devname = dev.INDEX.."."..dev.mainidx.."."..devs[ridx][dbdc_if[i]]["subidx"]
                devs[dridx][devname] = devs[ridx][dbdc_if[i]]
            end

            local apcli_if, wds_if, ext_if, mesh_if = {}, {}, {}, {}

            for i = 1, band_num do
                ext_if[i] = l1dat_parser_token_get(dev.ext_ifname, i, nil)
                apcli_if[i] = l1dat_parser_token_get(dev.apcli_ifname, i, nil)
                wds_if[i] = l1dat_parser_token_get(dev.wds_ifname, i, nil)
                mesh_if[i] = l1dat_parser_token_get(dev.mesh_ifname, i, nil)
            end

            for i = 1, l1dat_parser.MAX_NUM_EXTIF - 1 do -- ifname idx is from 0
                for j = 1, band_num do
                    devs[ridx][ext_if[j]..i] = devs[ridx][dbdc_if[j]]
                end
            end

            for i = 0, l1dat_parser.MAX_NUM_APCLI - 1 do
                for j = 1, band_num do
                    devs[ridx][apcli_if[j]..i] = devs[ridx][dbdc_if[j]]
                end
            end

            for i = 0, l1dat_parser.MAX_NUM_WDS - 1 do
                for j = 1, band_num do
                    devs[ridx][wds_if[j]..i] = devs[ridx][dbdc_if[j]]
                end
            end

            for i = 0, l1dat_parser.MAX_NUM_MESH - 1 do
                for j = 1, band_num do
                    if mesh_if[j] then
                        devs[ridx][mesh_if[j]..i] = devs[ridx][dbdc_if[j]]
                    end
                end
            end

        else
            devs[ridx][dev.main_ifname] = dev

            devname = dev.INDEX.."."..dev.mainidx
            devs[dridx][devname] = dev

            for i = 1, l1dat_parser.MAX_NUM_EXTIF - 1 do  -- ifname idx is from 0
                devs[ridx][dev.ext_ifname..i] = dev
            end

            for i = 0, l1dat_parser.MAX_NUM_APCLI - 1 do  -- ifname idx is from 0
                devs[ridx][dev.apcli_ifname..i] = dev
            end

            for i = 0, l1dat_parser.MAX_NUM_WDS - 1 do  -- ifname idx is from 0
                devs[ridx][dev.wds_ifname..i] = dev
            end

            for i = 0, l1dat_parser.MAX_NUM_MESH - 1 do  -- ifname idx is from 0
                devs[ridx][dev.mesh_ifname..i] = dev
            end
        end
    end

    fd:close()
    return devs
end

function wifi_file_exists(path)
    local fp = io.open(path, "rb")
    if fp then fp:close() end
    return fp ~= nil
end

function os_capture(cmd,raw)
    local f = assert(io.popen(cmd, 'r'))
    local s = assert(f:read('*a'))
    f:close()
    if raw then return s end
    s = string.gsub(s, '^%s+', '')
    s = string.gsub(s, '%s+$', '')
    s = string.gsub(s, '[\n\r]+', ' ')
    return s
end

function get_table_length(T)
    local count = 0
    for _ in pairs(T) do
        count = count + 1
    end
    return count
end

function save2dat_by_ifname_lsdk(ifname, param, val)
     local devs = l1dat_parser_load_l1_profile()
     local path = devs.ifname_ridx[ifname].profile_path
     local key = param

     local zone = l1dat_parser_l1_path_to_zone(path)

     if key == "Channel" then
          local chSelectValue
          if zone == "dev1" then
               chSelectValue= os_capture("nvram_get 2860 AutoChannelSelect")
          elseif zone == "dev2" then
               chSelectValue= os_capture("nvram_get rtdev AutoChannelSelect")
          elseif zone == "dev3" then
               chSelectValue= os_capture("nvram_get wifi3 AutoChannelSelect")
          else
               print("AutoChannelSelect ifname="..ifname..", invalid zone="..zone)
               return
          end

          if chSelectValue== "3" then
               print("AutoChannelSelect = 3, do not save channel")
               return
           end
     end

     if key == "Key1" or key == "Key2" or key == "Key3" or key == "Key4" then
          local appd = "Str"
          key = key..appd
     end

     if key == "SSID" or key == "WPAPSK" or key == "Key1Str" or key == "Key2Str"
        or key == "Key3Str" or key == "Key4Str" then
          local if_idx = string.match(ifname, "(%d)")+1
          key = key..if_idx
          print("ifname="..ifname..", key ="..key..", old param="..param)
     end

     if key == "AuthMode" or key == "EncrypType" or key == "HideSSID" then
          local if_idx = string.match(ifname, "(%d)")+1
          local oldVal
          if zone == "dev1" then
               oldVal= os_capture("nvram_get 2860 "..key)
          elseif zone == "dev2" then
               oldVal= os_capture("nvram_get rtdev "..key)
          elseif zone == "dev3" then
               oldVal= os_capture("nvram_get wifi3 "..key)
          else
               print("ifname="..ifname..", invalid zone="..zone)
               return
          end
          print("ifname="..ifname..", oldValue="..oldVal)

          local valueList = oldVal:split(";")
          local val_idx = 0
          local newVal
          local i = 0
          local val_len = get_table_length(valueList)
          if val_len < if_idx then
               for i = val_len+1,if_idx do
                    valueList[i] = valueList[1]
               end 
          end

          valueList[if_idx] = val

          for _, str in ipairs(valueList) do
               val_idx = val_idx+1
               if val_idx == 1 then
                    newVal = str
               else
                    newVal = newVal..";"..str
               end
          end
          val = string.format("\"%s\"", newVal)
          print("ifname="..ifname..", length ="..get_table_length(valueList)..", val="..val)
     end

     print("ifname="..ifname..", zone="..zone..", devname="..path..", key="..key..", val="..val)

     if zone == "dev1" then
          os.execute("nvram_set 2860 "..key.." "..val.." &")
     elseif zone == "dev2" then
           os.execute("nvram_set rtdev  "..key.." "..val.." &")
     elseif zone == "dev3" then
          os.execute("nvram_set wifi3 "..key.." "..val.." &")
     else
          print("ifname="..ifname..", invalid zone="..zone)
     end
end

function load_profile(path)
    local cfgs = {}

    cfgobj = datconf.openfile(path)
    if cfgobj then
        cfgs = cfgobj:getall()
        cfgobj:close()
    end

    return cfgs
end

function save_profile(path, cfgs)
   if not cfgs then
        print("configuration was empty, nothing saved")
        return
    end

    local datobj = datconf.openfile(path)
    datobj:merge(cfgs)
    datobj:close(true) -- means close and commit
end

function save2dat_by_ifname_openwrt(ifname, param, val)
    local devs = l1dat_parser_load_l1_profile()
    local path = devs.ifname_ridx[ifname].profile_path
    local key = param
    local drivercfgs = {}

    drivercfgs = load_profile(path)

    if key == "Channel" and drivercfgs['AutoChannelSelect'] == "3" then
        print("AutoChannelSelect = 3, do not save current channel")
        return
    end

    if key == "Key1" or key == "Key2" or key == "Key3" or key == "Key4" then
          local appd = "Str"
          key = key..appd
     end

     if key == "SSID" or key == "WPAPSK" or key == "Key1Str" or key == "Key2Str"
        or key == "Key3Str" or key == "Key4Str" then
          local if_idx = string.match(ifname, "(%d)")+1
          key = key..if_idx
          print("ifname="..ifname..", key ="..key..", old param="..param)
     end

     if key == "AuthMode" or key == "EncrypType" or key == "HideSSID" then
          local if_idx = string.match(ifname, "(%d)")+1
          local oldVal

          oldVal = drivercfgs[key]
          print("ifname="..ifname..", oldValue="..oldVal)

          local valueList = oldVal:split(";")
          local val_idx = 0
          local newVal
          local i = 0
          local val_len = get_table_length(valueList)
          if val_len < if_idx then
               for i = val_len+1,if_idx do
                    valueList[i] = valueList[1]
               end 
          end
          valueList[if_idx] = val
          for _, str in ipairs(valueList) do
               val_idx = val_idx+1
               if val_idx == 1 then
                    newVal = str
               else
                    newVal = newVal..";"..str
               end
          end
          val = newVal
          print("ifname="..ifname..", length ="..get_table_length(valueList)..", val="..val)
    end

    print("ifname="..ifname..", devname="..path..", key="..key..", val="..val)
    drivercfgs[key] = val
    save_profile(path, drivercfgs)
end

function save2dat_by_ifname(ifname, param, val)
    if wifi_file_exists("/etc/openwrt_version") then
        save2dat_by_ifname_openwrt(ifname, param, val)
    else
        save2dat_by_ifname_lsdk(ifname, param, val)
    end
end

function save2uci(ifname)
	local devs = l1dat_parser_load_l1_profile()
	local compatible = devs.ifname_ridx[ifname].init_compatible
	if wifi_file_exists("/lib/wifi/"..compatible..".sh") then
		print("dat2uci excute")
		os.execute("dat2uci")
	end
end

ifname = arg[1]
param = arg[2]
param_val = arg[3]
print("input args: ifname ="..ifname ..", param ="..param ..", param_val="..param_val)

if string.match(ifname , "apcli") == "apcli" then 
   print("ifname = apcli, return directly")
   return
end

save2dat_by_ifname(ifname, param, param_val)
save2uci(ifname)
