
# you will want to add 'export RPMBASE=~/rpm' or somesuch to your .bash_profile

# directory to create for a 'make build'
BUILD_DIR = build

# RPM vars
MWSOFT_RPM_NAME = mwl-wilson-software
MWLARCH=i386
MWSOFT_RPM_VER=$(shell sed -e '/^Version: */!d; s///;q' $(MWSOFT_RPM_NAME).spec)
MWSOFT_RPM_REL=$(shell sed -e '/^Release: */!d; s///;q' $(MWSOFT_RPM_NAME).spec)

# set shell to bash (needed for brace expansion as used in mkdir in build target)
SHELL=/bin/bash


# passed to gcc with -D flag for use in creating -version message by apps
export MWSOFT_RPM_STRING="\"$(MWSOFT_RPM_NAME)-$(MWSOFT_RPM_VER)-$(MWSOFT_RPM_REL)\""

libs:
	mkdir -p lib
	cd src/cluster ; \
	$(MAKE) iolib.a ; \
	mv -f iolib.a ../../lib ; \


utils:
	$(MAKE) -C src atob y
	$(MAKE) -C src/cluster editheader header showcmd

xclust: libs
	$(MAKE) -C X11/xclust/ xclust
	$(MAKE) -C X11/xplot/ xplot
	$(MAKE) -C X11/xview/ xview
	$(MAKE) -C src csi
	$(MAKE) -C src/cluster findspike \
	                       spikeanal \
	                       spikeavg 
	$(MAKE) -C src/cluster/popanal popanal
	$(MAKE) -C src/cluster/behav behav

extract: libs
	$(MAKE) -C src/cluster adextract \
	                       crextract \
	                       esextract \
	                       parmextract \
	                       posextract \
	                       spikeparms2 \
	                       textract

clean: 
	$(MAKE) -C X11/xclust clean
	$(MAKE) -C X11/xplot/ clean
	$(MAKE) -C X11/xview/ clean
	$(MAKE) -C src clean
	$(MAKE) -C src/cluster clean
	$(MAKE) -C src/cluster/popanal clean
	$(MAKE) -C src/cluster/behav clean
	rm -rf build \
	       lib \
	rm -f *.rpm

all: libs utils xclust extract

default: all

build: all
	mkdir -p $(BUILD_DIR)/$(MWSOFT_RPM_NAME)/bin
	mkdir -p $(BUILD_DIR)/$(MWSOFT_RPM_NAME)/share/doc/$(MWSOFT_RPM_NAME)-$(MWSOFT_RPM_VER)-$(MWSOFT_RPM_REL)

	#xclust/analysis binaries
	cp -f X11/xclust/xclust? \
              X11/xplot/xplot \
              X11/xview/xview \
              src/csi \
              src/cluster/{findspike,spikeanal,spikeavg,popanal/popanal,behav/behav} \
              $(BUILD_DIR)/$(MWSOFT_RPM_NAME)/bin

	#data extraction binaries
	cp -f src/cluster/{adextract,crextract,esextract,parmextract,posextract,spikeparms2,textract} \
              $(BUILD_DIR)/$(MWSOFT_RPM_NAME)/bin

	#utilities
	cp -f src/{atob,y} \
              src/cluster/{editheader,header,showcmd}\
              $(BUILD_DIR)/$(MWSOFT_RPM_NAME)/bin

	#scripts
	cp -f scripts/cl2t \
              $(BUILD_DIR)/$(MWSOFT_RPM_NAME)/bin

	#docs
	cp -f RELEASE ChangeLog \
	      $(BUILD_DIR)/$(MWSOFT_RPM_NAME)/share/doc/$(MWSOFT_RPM_NAME)-$(MWSOFT_RPM_VER)-$(MWSOFT_RPM_REL)

rpm: build
	cp -f old-bin/xclust2 \
	      $(BUILD_DIR)/$(MWSOFT_RPM_NAME)/bin
	cd $(BUILD_DIR) ; \
	tar czvf $(MWSOFT_RPM_NAME).tar.gz $(MWSOFT_RPM_NAME) ; \
	mv -f $(MWSOFT_RPM_NAME).tar.gz $(RPMBASE)/SOURCES/
	rpmbuild -bb $(MWSOFT_RPM_NAME).spec
	cp $(RPMBASE)/RPMS/$(MWLARCH)/$(MWSOFT_RPM_NAME)-$(MWSOFT_RPM_VER)-$(MWSOFT_RPM_REL).$(MWLARCH).rpm . 
