ifeq ($V,1)
silent=
silent_stdout=
else
silent=@
silent_stdout= > /dev/null
endif

#Files left over from making the crypt.pdf.
LEFTOVERS=*.dvi *.log *.aux *.toc *.idx *.ilg *.ind *.out *.lof

#build the doxy files (requires Doxygen, tetex and patience)
.PHONY: doxygen
doxygen:
	doxygen $(silent_stdout)

patched_doxygen:
	(cat Doxyfile && echo "HAVE_DOT=no") | doxygen - $(silent_stdout)

doxy: patched_doxygen
	${MAKE} -C doxygen/latex $(silent_stdout) && mv -f doxygen/latex/refman.pdf .
	@echo The huge doxygen PDF should be available as doc/refman.pdf

#This builds the crypt.pdf file. Note that the rm -f *.pdf has been removed
#from the clean command! This is because most people would like to keep the
#nice pre-compiled crypt.pdf that comes with libtomcrypt! We only need to
#delete it if we are rebuilding it.
docs crypt.pdf: crypt.tex
	rm -f crypt.pdf $(LEFTOVERS)
	cp crypt.tex crypt.bak
	touch -r crypt.tex crypt.bak
	(printf "%s" "\def\fixedpdfdate{"; date +'D:%Y%m%d%H%M%S%:z' -d @$$(stat --format=%Y crypt.tex) | sed "s/:\([0-9][0-9]\)$$/'\1'}/g") > crypt-deterministic.tex
	printf "%s\n" "\pdfinfo{" >> crypt-deterministic.tex
	printf "%s\n" "  /CreationDate (\fixedpdfdate)" >> crypt-deterministic.tex
	printf "%s\n}\n" "  /ModDate (\fixedpdfdate)" >> crypt-deterministic.tex
	cat crypt.tex >> crypt-deterministic.tex
	mv crypt-deterministic.tex crypt.tex
	touch -r crypt.bak crypt.tex
	echo "hello" > crypt.ind
	latex crypt $(silent_stdout)
	latex crypt $(silent_stdout)
	makeindex crypt.idx $(silent_stdout)
	perl ../helper.pl --fixupind crypt.ind
	pdflatex crypt $(silent_stdout)
	sed -b -i 's,^/ID \[.*\]$$,/ID [<0> <0>],g' crypt.pdf
	mv crypt.bak crypt.tex
	rm -f $(LEFTOVERS)

docdvi: crypt.tex
	echo hello > crypt.ind
	latex crypt $(silent_stdout)
	latex crypt $(silent_stdout)
	makeindex crypt.idx
	perl ../helper.pl --fixupind crypt.ind
	latex crypt $(silent_stdout)
	latex crypt $(silent_stdout)

termdoc: docdvi
	dvi2tty crypt.dvi -w120

clean:
	rm -f $(LEFTOVERS)
	rm -rf doxygen/
