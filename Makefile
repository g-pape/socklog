DESTDIR=

PACKAGE=socklog-0.10.0
DIRS=doc man package src
MANPAGES=man/socklog.8 man/tryto.1 man/uncat.1 man/socklog-conf.8

all: clean .manpages $(PACKAGE).tar.gz

.manpages:
	for i in $(MANPAGES); do \
	  rman -S -f html -r '' < $$i | \
	  sed -e 's}NAME="sect\([0-9]*\)" HREF="#toc[0-9]*">\(.*\)}NAME="sect\1">\2}g ; \
	  s}<A HREF="#toc">Table of Contents</A>}<a href="http://smarden.org/pape/">G. Pape</a><br><A HREF="index.html">socklog</A><hr>}g ; \
	  s}<!--.*-->}}g' \
	  > doc/`basename $$i`.html ; \
	done ; \
	touch .manpages

$(PACKAGE).tar.gz:
	rm -rf TEMP
	mkdir -p TEMP/admin/$(PACKAGE)
	( cd src ; make clean )
	cp -a $(DIRS) TEMP/admin/$(PACKAGE)/
	chmod -R g-ws TEMP/admin
	chmod +t TEMP/admin
	find TEMP -exec touch {} \;
	su -c 'chown -R root:root TEMP/admin ; \
		( cd TEMP ; tar cpfz ../$(PACKAGE).tar.gz admin --exclude CVS ) ; \
		rm -rf TEMP'

clean:
	find . -name \*~ -exec rm -f {} \;
	find . -name .??*~ -exec rm -f {} \;
	find . -name \#?* -exec rm -f {} \;

cleaner: clean
	rm -f $(PACKAGE).tar.gz
	for i in $(MANPAGES); do rm -f doc/`basename $$i`.html; done
	rm -f .manpages
