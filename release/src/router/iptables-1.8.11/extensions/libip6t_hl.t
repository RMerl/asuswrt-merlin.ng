:INPUT,FORWARD,OUTPUT
-m hl;;FAIL
-m hl --hl-eq 42;=;OK
-m hl ! --hl-eq 42;=;OK
-m hl --hl-lt 42;=;OK
-m hl --hl-gt 42;=;OK
-m hl --hl-gt 42 --hl-eq 42;;FAIL
-m hl --hl-gt;;FAIL
