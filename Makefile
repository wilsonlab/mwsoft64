
# set shell to bash (needed for brace expansion as used in mkdir in build target)
SHELL=/bin/bash
BUILD_DIR=build


all: lib extract util popanal behav xclust xplot xview test

lib:
	@echo -e "\n\t\tMWSoft Building: Library";
	@echo "------------------------------------------------------"
	@echo "Building:LIB";
	@mkdir -p $(BUILD_DIR)/lib;
	@cd src; \
	$(MAKE) iolib.a ; \
	cd ..;
	mv -f src/iolib.a $(BUILD_DIR)/lib; \
	cd ..;

util: lib
	@echo -e "\n\t\tMWSoft Building: Utilities";
	@echo "------------------------------------------------------"
	@echo "Building:LIB";
	@mkdir -p $(BUILD_DIR)/bin;
	@$(MAKE) -C src/util
	mv src/util/{atob,y,csi,carve} $(BUILD_DIR)/bin/

behav: lib
	@echo -e "\n\t\tMWSoft Building: behav";
	@echo "------------------------------------------------------"
	@echo "Building:LIB";
	@mkdir -p $(BUILD_DIR)/bin;
	@$(MAKE) -C src/behav
	mv src/behav/behav $(BUILD_DIR)/bin/

popanal: popanal
	@echo -e "\n\t\tMWSoft Building: popanal";
	@echo "------------------------------------------------------"
	@echo "Building:LIB";
	@mkdir -p $(BUILD_DIR)/bin;
	@$(MAKE) -C src/popanal
	mv src/popanal/popanal $(BUILD_DIR)/bin/

xclust: lib
	@echo -e "\n\t\tMWSoft Building: xclust";
	@echo "------------------------------------------------------"
	@mkdir -p $(BUILD_DIR)/bin;	
	@$(MAKE) -C src/xclust/ xclust
	mv src/xclust/xclust3 $(BUILD_DIR)/bin/	

xplot: lib
	@echo -e "\n\t\tMWSoft Building: xplot";
	@echo "------------------------------------------------------"
	@mkdir -p $(BUILD_DIR)/bin;	
	@$(MAKE) -C src/xplot xplot
	mv src/xplot/xplot $(BUILD_DIR)/bin/

xview: lib
	@echo -e "\n\t\tMWSoft Building: xview";
	@echo "------------------------------------------------------"
	@mkdir -p $(BUILD_DIR)/bin;	
	@$(MAKE) -C src/xview xview
	mv src/xview/xview $(BUILD_DIR)/bin/

extract: lib
	@mkdir -p $(BUILD_DIR)/bin;
	@$(MAKE) -C src/extract 
	mv src/extract/{spikeparms2,adextract,crextract,editheader,esextract,findspike,parmextract,posextract,showcmd,spikeanal,spikeavg,textract} $(BUILD_DIR)/bin/

test:
	@./run_test

clean: 
 	#$(MAKE) -C src/ clean; 
	@rm -rf $(BUILD_DIR)
	@$(MAKE) -C src/ clean
	@$(MAKE) -C src/behav/ clean
	@$(MAKE) -C src/extract/ clean
	@$(MAKE) -C src/popanal/ clean
	@$(MAKE) -C src/util/ clean
	@$(MAKE) -C src/xclust/ clean
	@$(MAKE) -C src/xplot/ clean
	@$(MAKE) -C src/xview/ clean

local-install:
	@mkdir -p ~/bin
	@cp ./build/bin/* ~/bin/

install:
	@cp ./build/bin/* /usr/local/bin/

local-uninstall:
	@rm ~/bin/{adextract,carve,editheader,parmextract,showcmd,spikeparms2,xplot,atob,crextract,esextract,popanal,spikeanal,textract,xview,behav,csi,findspike,posextract,spikeavg,xclust3,y}

uninstall:
	@rm /usr/local/bin/{adextract,carve,editheader,parmextract,showcmd,spikeparms2,xplot,atob,crextract,esextract,popanal,spikeanal,textract,xview,behav,csi,findspike,posextract,spikeavg,xclust3,y}



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

