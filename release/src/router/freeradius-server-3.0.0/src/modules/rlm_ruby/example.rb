#This is example radius.rb script
module Radiusd
    def Radiusd.instantiate(arg)
	radlog(L_DBG,"[ruby]Running ruby instantiate")
        p arg
	return Radiusd::RLM_MODULE_OK
    end
    def Radiusd.authenticate(arg)
    	radlog(L_DBG,"[ruby]Running ruby authenticate")
        p arg
	return Radiusd::RLM_MODULE_NOOP
    end
    def Radiusd.authorize(arg)
    	radlog(L_DBG,"[ruby]Running ruby authorize")
	p arg
	#Here we return Cleartext-Password, which could have been retrieved from DB.
	return [Radiusd::RLM_MODULE_UPDATED, [],[["Cleartext-Password","pass"]]]
    end
    def Radiusd.accounting(arg)
    	radlog(L_DBG,"[ruby]Running ruby accounting")
	p arg
	return Radiusd::RLM_MODULE_NOOP
    end

end
