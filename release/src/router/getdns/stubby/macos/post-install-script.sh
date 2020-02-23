#!/bin/sh

security authorizationdb write net.getdnsapi.stubby.daemon.run < /Applications/StubbyManager.app/Contents/MacOS/rights.daemon.run.plist
security authorizationdb write net.getdnsapi.stubby.dns.local < /Applications/StubbyManager.app/Contents/MacOS/rights.dns.local.plist
sudo chmod 6755 /Applications/StubbyManager.app/Contents/MacOS/stubby-ui-helper
sudo touch /var/log/stubby.log