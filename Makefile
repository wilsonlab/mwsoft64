
# set shell to bash (needed for brace expansion as used in mkdir in build target)
SHELL=/bin/bash
BUILD_DIR=build


all: lib  extract util popanal behav 

lib:
	mkdir -p $(BUILD_DIR)/lib;
	cd src; \
	$(MAKE) iolib.a ; \
	cd ..;
	mv -f src/iolib.a $(BUILD_DIR)/lib; \
	cd ..;

util: lib
	mkdir -p $(BUILD_DIR)/bin;
	$(MAKE) -C src/util
	mv src/util/{atob,y,csi,carve} $(BUILD_DIR)/bin/

behav: lib
	mkdir -p $(BUILD_DIR)/bin;
	$(MAKE) -C src/behav
	mv src/behav/behav $(BUILD_DIR)/bin/

popanal: popanal
	mkdir -p $(BUILD_DIR)/bin;
	$(MAKE) -C src/popanal
	mv src/popanal/popanal $(BUILD_DIR)/bin/

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

extract: lib
	mkdir -p $(BUILD_DIR)/bin;
	$(MAKE) -C src/extract 
	mv src/extract/{spikeparms2,adextract,crextract,editheader,esextract,findspike,parmextract,posextract,showcmd,spikeanal,spikeavg,textract} $(BUILD_DIR)/bin/

						   #adextract \
	                       #crextract \
	                       #esextract \
	                       #parmextract \
	                       #posextract \
	                       #spikeparms2 
	                       #textract

clean: 
 	#$(MAKE) -C src/ clean; 
	rm -rf $(BUILD_DIR)
	$(MAKE) -C src/ clean
	$(MAKE) -C src/popanal/ clean
	$(MAKE) -C src/behav/ clean



default: lib  extract util popanal behav 

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

