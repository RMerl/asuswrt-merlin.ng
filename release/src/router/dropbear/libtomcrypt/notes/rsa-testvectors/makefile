default: regen

clean:
	rm -f *.c

regen:
	python rt.py pss-vect.txt pss > pss-vect.c
	python rt.py oaep-vect.txt oaep > oaep-vect.c
	python rt.py pkcs1v15sign-vectors.txt emsa > pkcs1v15sign-vectors.c
	python rt.py pkcs1v15crypt-vectors.txt eme > pkcs1v15crypt-vectors.c

