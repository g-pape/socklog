DESTDIR=

PACKAGE=socklog-0.3.0
DIRS=doc etc package src

all: archive

archive:
	rm -rf TEMP
	mkdir -p TEMP/admin/$(PACKAGE)
	cp -a $(DIRS) TEMP/admin/$(PACKAGE)/
	chmod +t TEMP/admin
	( cd TEMP ; tar cpfz ../$(PACKAGE).tar.gz admin )

clean:
	rm -rf TEMP
	rm -f $(PACKAGE).tar.gz
	find . -name \*~ -exec rm -f {} \;
	find . -name .??*~ -exec rm -f {} \;
	find . -name \#?* -exec rm -f {} \;

