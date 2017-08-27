=begin
  Copyright (C) 2008-2009 Tobias Brunner
  Hochschule fuer Technik Rapperswil

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by the
  Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
  or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
  for more details.
=end

require 'dumm.so'
require 'dumm/guest'

module Dumm
  
  # use guest/bridge indentifiers directly
  def method_missing(id, *args)
    if Guest.guest? id
      return Guest[id]
    end
    if Bridge.bridge? id
      return Bridge[id]
    end
    super(id, *args)
  end
  
  # shortcut for Template loading
  def template(name = nil)
    if name
      Template.load name
    else
      Template.sort.each {|t| puts t }
    end
    return Dumm
  end
  
  # unload template/overlays, reset all guests and delete bridges
  def reset
    Template.unload
    Guest.each { |guest|
      guest.reset
    }
    Bridge.each { |bridge|
      bridge.delete
    }
    return Dumm
  end
  
  # wait until all running guests have booted up
  def boot
    Guest.each {|g|
      g.boot if g.running?
    }
    return Dumm
  end
end

# vim:sw=2 ts=2 et
