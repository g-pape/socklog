DESTDIR=

PACKAGE=socklog-2.1.2
DIRS=doc man package src

all: clean .doc .man $(PACKAGE).tar.gz

.doc:
	cd md && ./gen-html ../doc
	touch .doc

.man:
	cd md && ./gen-man ../man
	touch .man

$(PACKAGE).tar.gz:
	rm -rf TEMP
	mkdir -p TEMP/admin/$(PACKAGE)
	( cd src ; make clean )
	cp -a $(DIRS) TEMP/admin/$(PACKAGE)/
	chmod -R g-ws TEMP/admin
	chmod +t TEMP/admin
	find TEMP -exec touch {} \;
	su -c 'chown -R root:root TEMP/admin && \
	  (cd TEMP; tar cpzf ../$(PACKAGE).tar.gz --exclude CVS admin) && \
	  rm -rf TEMP'

clean:
	find . -name \*~ -exec rm -f {} \;
	find . -name .??*~ -exec rm -f {} \;
	find . -name \#?* -exec rm -f {} \;

cleaner: clean
	rm -f $(PACKAGE).tar.gz
	rm -f doc/*.html man/*.[0-9] .doc .man
