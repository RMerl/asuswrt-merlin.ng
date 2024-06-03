#!/usr/bin/lua

-- file to start/stop map config agent
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


local socket = require("socket")
local host = "127.0.0.1"
local tcp = assert(socket.tcp())

local line = "close_config_agent"

if (arg[1] == nil or arg[1] == "") then
	print(" Usages " ..arg[0] .." start/stop")
end

command = ""
server_port = os.capture("wificonf -f /etc/map_cfg.txt get config_agent_port")

if (server_port == nil or server_port == "") then
        server_port = 9010
end

tcp:connect(host, server_port);
tcp:send(line.. "\n");
tcp:close()

if (arg[1] == "start") then
	os.execute("/usr/bin/lua /usr/bin/config_agent.lua > /dev/console &")
end
