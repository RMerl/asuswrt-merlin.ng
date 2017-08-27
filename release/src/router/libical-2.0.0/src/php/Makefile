all: LibicalWrap.so

LibicalWrap.so: LibicalWrap.o
	gcc -shared -L/usr/lib -rdynamic LibicalWrap.o ../libical/.libs/libical.a  ../libicalss/.libs/libicalss.a -o LibicalWrap.so 

LibicalWrap.o: LibicalWrap.c
	gcc -fpic -I/usr/include/php4/TSRM/ -I/usr/include/php4/ -I/usr/include/php4/Zend -I/usr/include/php4/main -c LibicalWrap.c -o LibicalWrap.o

LibicalWrap.c: LibicalWrap.i
	swig -php -o LibicalWrap.c LibicalWrap.i 

clean:
	rm LibicalWrap.so LibicalWrap.o LibicalWrap.c
