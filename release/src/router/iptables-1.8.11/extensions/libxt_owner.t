:OUTPUT,POSTROUTING
*mangle
-m owner --uid-owner root;-m owner --uid-owner 0;OK
-m owner --uid-owner 0-10;=;OK
-m owner --gid-owner root;-m owner --gid-owner 0;OK
-m owner --gid-owner 0-10;=;OK
-m owner --uid-owner root --gid-owner root;-m owner --uid-owner 0 --gid-owner 0;OK
-m owner --uid-owner 0-10 --gid-owner 0-10;=;OK
-m owner ! --uid-owner root;-m owner ! --uid-owner 0;OK
-m owner --socket-exists;=;OK
-m owner --gid-owner 0-10 --suppl-groups;=;OK
-m owner --suppl-groups --gid-owner 0-10;;FAIL
-m owner --gid-owner 0-10 ! --suppl-groups;;FAIL
-m owner --suppl-groups;;FAIL
:INPUT
-m owner --uid-owner root;;FAIL
