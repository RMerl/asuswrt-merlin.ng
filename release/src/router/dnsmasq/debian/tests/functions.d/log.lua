-- Lua script logging calls from dnsmasq

-- Open the log file in append mode 
logfile = assert(io.open("/var/log/dnsmasq-lua.log", "a"))

-- Prepend date and time to a string and write the result to the log file
function __log(str)
	logfile:write(os.date("!%FT%TZ ")..str.."\n")
end

-- flush the log file
function __flush_log()
	logfile:flush()
end

-- Log a call to init()
function init()
	__log("initialising")
	__flush_log()
end

-- Log a call to shutdown()
function shutdown()
	__log("shutting down")
	__flush_log()
end

-- Log a call to lease() including all arguments
function lease(operation, params)
	local lines = {}
	__log(operation.." lease")
	for key,value in pairs(params) do
		table.insert(lines, key..": "..value)
	end
	table.sort(lines)
	for index,line in ipairs(lines) do
		__log("\t"..line)
	end
	__flush_log()
end
