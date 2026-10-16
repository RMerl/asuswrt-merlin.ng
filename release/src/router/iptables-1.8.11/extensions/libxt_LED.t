:INPUT,FORWARD,OUTPUT
-j LED;;FAIL
-j LED --led-trigger-id "foo";=;OK
-j LED --led-trigger-id "foo" --led-delay 42 --led-always-blink;=;OK
