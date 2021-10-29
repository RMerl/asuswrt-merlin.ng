all: build

SRC=wpaspy.c

build: $(SRC) setup.py
	python setup.py build

install:
	python setup.py install

clean:
	python setup.py clean
	rm -f *~
	rm -rf build
