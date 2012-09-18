
# directory to create for a 'make build'
BUILD_DIR = build

# set shell to bash (needed for brace expansion as used in mkdir in build target)
SHELL=/bin/bash

libs:
	mkdir -p build/lib;
	cd src; \
	$(MAKE) iolib.a ; \
	mv -f iolib.a ../build/lib ; \
	cd ..;


# utils:
# 	$(MAKE) -C src atob y
# 	$(MAKE) -C src/cluster editheader header showcmd

# xclust: libs
# 	$(MAKE) -C X11/xclust/ xclust
# 	$(MAKE) -C X11/xplot/ xplot
# 	$(MAKE) -C X11/xview/ xview
# 	$(MAKE) -C src csi
# 	$(MAKE) -C src/cluster findspike \
# 	                       spikeanal \
# 	                       spikeavg 
# 	$(MAKE) -C src/cluster/popanal popanal
# 	$(MAKE) -C src/cluster/behav behav

extract: libs
	mkdir -p build/extract;
	$(MAKE) -C src/extract 
						   #adextract \
	                       #crextract \
	                       #esextract \
	                       #parmextract \
	                       #posextract \
	                       #spikeparms2 
	                       #textract

clean: 
 	#$(MAKE) -C src/ clean; 
	rm -rf build/lib;
	rm -rf build/extract;   
	rm -rf build/xclust;
	rm -rf build/misc;
	 
	$(MAKE) -C src/ clean	

# 	$(MAKE) -C X11/xclust clean
# 	$(MAKE) -C X11/xplot/ clean
# 	$(MAKE) -C X11/xview/ clean
# 	$(MAKE) -C src/cluster clean
# 	$(MAKE) -C src/cluster/popanal clean
# 	$(MAKE) -C src/cluster/behav clean
# 	rm -rf build\lib \
# 	       build\bin \
	

all: libs  extract 
#utils xclust

default: all

# build: all
# 	mkdir -p $(BUILD_DIR)/$(MWSOFT_RPM_NAME)/bin
# 	mkdir -p $(BUILD_DIR)/$(MWSOFT_RPM_NAME)/share/doc/$(MWSOFT_RPM_NAME)-$(MWSOFT_RPM_VER)-$(MWSOFT_RPM_REL)

# 	#xclust/analysis binaries
# 	cp -f X11/xclust/xclust? \
#               X11/xplot/xplot \
#               X11/xview/xview \
#               src/csi \
#               src/cluster/{findspike,spikeanal,spikeavg,popanal/popanal,behav/behav} \
#               $(BUILD_DIR)/$(MWSOFT_RPM_NAME)/bin

# 	#data extraction binaries
# 	cp -f src/cluster/{adextract,crextract,esextract,parmextract,posextract,spikeparms2,textract} \
#               $(BUILD_DIR)/$(MWSOFT_RPM_NAME)/bin

# 	#utilities
# 	cp -f src/{atob,y} \
#               src/cluster/{editheader,header,showcmd}\
#               $(BUILD_DIR)/$(MWSOFT_RPM_NAME)/bin

# 	#scripts
# 	cp -f scripts/cl2t \
#               $(BUILD_DIR)/$(MWSOFT_RPM_NAME)/bin

# 	#docs
# 	cp -f RELEASE ChangeLog \
# 	      $(BUILD_DIR)/$(MWSOFT_RPM_NAME)/share/doc/$(MWSOFT_RPM_NAME)-$(MWSOFT_RPM_VER)-$(MWSOFT_RPM_REL)

