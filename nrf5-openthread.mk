#
#    Copyright (c) 2019 Google LLC.
#    All rights reserved.
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
#
#    Description:
#      Component makefile for building OpenThread for the nRF52840 platform.
#

# ==================================================
# General settings
# ==================================================

# OpenThread source root directory
OPENTHREAD_ROOT ?= $(PROJECT_ROOT)/third_party/openthread

# Target for which OpenThread will be built.
OPENTHREAD_TARGET = nrf52840

# Archtecture for which OpenThread will be built.
OPENTHREAD_HOST_ARCH = arm-none-eabi

# Directory into which the OpenThread build system will place its output.
OPENTHREAD_OUTPUT_DIR = $(OUTPUT_DIR)/openthread

# Directory containing OpenThread libraries.
OPENTHREAD_LIB_DIR = $(if $(filter-out 0, $(USE_PREBUILT_OPENTHREAD)), \
    $(NRF5_SDK_ROOT)/external/openthread/lib/gcc, \
    $(OPENTHREAD_OUTPUT_DIR)/lib)

# Prerequisite target for anything that depends on the output of the OpenThread
# build process.
OPENTHREAD_PREREQUISITE = $(if $(filter-out 0, $(USE_PREBUILT_OPENTHREAD)),,install-thread)

# Location of OpenThread's platform config file.  This can be overridden by the
# application to use an application-specific configuration.
OPENTHREAD_PROJECT_CONFIG_FILE = openthread-core-$(OPENTHREAD_TARGET)-config.h

# Additional header files needed by the Nordic port of OpenThread
# but not installed automatically by OpenThread's build system.
OPENTHREAD_PLATFORM_HEADERS = \
    $(OPENTHREAD_ROOT)/examples/platforms/$(OPENTHREAD_TARGET)/platform-fem.h \
    $(OPENTHREAD_ROOT)/examples/platforms/$(OPENTHREAD_TARGET)/platform-softdevice.h \
    $(OPENTHREAD_ROOT)/examples/platforms/$(OPENTHREAD_TARGET)/openthread-core-$(OPENTHREAD_TARGET)-config.h \
    $(OPENTHREAD_ROOT)/examples/platforms/openthread-system.h


# ==================================================
# Build options
# ==================================================

# By default, use the prebuilt OpenThread libraries included in Nordic nRF5 SDK.
USE_PREBUILT_OPENTHREAD ?= 0


# ==================================================
# Compilation flags specific to building OpenThread
# ==================================================

OPENTHREAD_CPPFLAGS = $(STD_CFLAGS) $(DEBUG_FLAGS) $(OPT_FLAGS) -Wno-expansion-to-defined $(OPENTHREAD_DEFINE_FLAGS) $(OPENTHREAD_INC_FLAGS)
OPENTHREAD_CXXFLAGS = $(STD_CXXFLAGS) -Wno-expansion-to-defined
OPENTHREAD_DEFINE_FLAGS = $(foreach def,$(OPENTHREAD_DEFINES),-D$(def))
OPENTHREAD_INC_FLAGS = $(foreach dir,$(OPENTHREAD_INC_DIRS),-I$(dir))

OPENTHREAD_DEFINES = \
    NRF52840_XXAA \
    DISABLE_CC310=1 \
    OPENTHREAD_PROJECT_CORE_CONFIG_FILE='\"$(OPENTHREAD_PROJECT_CONFIG_FILE)\"'

OPENTHREAD_INC_DIRS = \
	$(PROJECT_ROOT) \
    $(OPENTHREAD_ROOT)/examples/platforms \
    $(OPENTHREAD_ROOT)/examples/platforms/$(OPENTHREAD_TARGET) \
    $(NRF5_SDK_ROOT)/modules/nrfx/mdk


# ==================================================
# OpenThread configuration options
# ==================================================

OPENTHREAD_CONFIGURE_OPTIONS = \
    CPP="$(CPP)" CC="$(CCACHE) $(CC)" CXX="$(CCACHE) $(CXX)" \
    CCAS="$(CCACHE) $(CC)" AS="${AS}" AR="$(AR)" RANLIB="$(RANLIB)" \
    NM="$(NM)" STRIP="$(STRIP)" OBJDUMP="$(OBJCOPY)" OBJCOPY="$(OBJCOPY)" \
    SIZE="$(SIZE)" RANLIB="$(RANLIB)" INSTALL="$(INSTALL) $(INSTALLFLAGS) -m644" \
    CPPFLAGS="$(OPENTHREAD_CPPFLAGS)" \
    CXXFLAGS="$(OPENTHREAD_CXXFLAGS)" \
    LDFLAGS="" \
    --prefix=$(OPENTHREAD_OUTPUT_DIR) \
    --exec-prefix=$(OPENTHREAD_OUTPUT_DIR) \
    --host=$(OPENTHREAD_HOST_ARCH) \
    --srcdir="$(OPENTHREAD_ROOT)" \
    --enable-ftd \
    --enable-mtd \
    --enable-diag \
    --enable-linker-map \
    --with-examples=nrf52840 \
    --disable-tools \
    --disable-tests \
    --disable-docs

# Enable / disable optimization.
ifeq ($(OPT),1)
OPENTHREAD_CONFIGURE_OPTIONS += --enable-optimization=yes
else
OPENTHREAD_CONFIGURE_OPTIONS += --enable-optimization=no
endif


# ==================================================
# Adjustments to standard build settings to 
#   incorporate OpenThread
# ==================================================

# Add the appropriate path to the public OpenThread headers to the application's
# compile actions.
STD_INC_DIRS += $(if $(filter-out 0, $(USE_PREBUILT_OPENTHREAD)), \
    $(NRF5_SDK_ROOT)/external/openthread/include, \
    $(OPENTHREAD_OUTPUT_DIR)/include)

# Add the location of OpenThread libraries to application link action.
STD_LDFLAGS += -L$(OPENTHREAD_LIB_DIR)

# Add OpenThread libraries to standard libraries list. 
STD_LIBS += \
    -lopenthread-diag \
    -lopenthread-ftd \
    -lopenthread-platform-utils \
    -lmbedcrypto \
    -lopenthread-nrf52840-softdevice-sdk \
    -lnrf_cc310_0.9.10
# TODO: eliminate nrf_cc310_0.9.10; appears to only be needed by the .c files included by the app
    
# Add the appropriate OpenThread target as a prerequisite to all application
# compilation targets to ensure that OpenThread gets built and its header
# files installed prior to compiling any dependent source files.
STD_COMPILE_PREREQUISITES += $(OPENTHREAD_PREREQUISITE)

# Add the OpenThread libraries as prerequisites for linking the application.
STD_LINK_PREREQUISITES += \
    $(OPENTHREAD_LIB_DIR)/libopenthread-diag.a \
    $(OPENTHREAD_LIB_DIR)/libopenthread-ftd.a \
    $(OPENTHREAD_LIB_DIR)/libopenthread-platform-utils.a \
    $(OPENTHREAD_LIB_DIR)/libmbedcrypto.a \
    $(OPENTHREAD_LIB_DIR)/libopenthread-nrf52840-softdevice-sdk.a


# ==================================================
# Late-bound build rules for OpenThread
# ==================================================

ifneq ($(USE_PREBUILT_OPENTHREAD),1)

# Add OpenThreadBuildRules to the list of late-bound build rules that
# will be evaluated when GenerateBuildRules is called. 
LATE_BOUND_RULES += OpenThreadBuildRules

# Rules for configuring, building and installing OpenThread from source.
define OpenThreadBuildRules

.PHONY : config-thread check-config-thread build-thread install-thread clean-thread

check-config-thread : | $(OPENTHREAD_OUTPUT_DIR)
	@echo $(OPENTHREAD_ROOT)/configure $(OPENTHREAD_CONFIGURE_OPTIONS) > $(OPENTHREAD_OUTPUT_DIR)/config.args.tmp; \
	(test -r $(OPENTHREAD_OUTPUT_DIR)/config.args && cmp -s $(OPENTHREAD_OUTPUT_DIR)/config.args.tmp $(OPENTHREAD_OUTPUT_DIR)/config.args) || \
	    mv $(OPENTHREAD_OUTPUT_DIR)/config.args.tmp $(OPENTHREAD_OUTPUT_DIR)/config.args; \
	rm -f $(OPENTHREAD_OUTPUT_DIR)/config.args.tmp;

$(OPENTHREAD_OUTPUT_DIR)/config.args : check-config-thread
	@: # Null action required to work around make's crazy timestamp caching behavior.

$(OPENTHREAD_OUTPUT_DIR)/config.status : $(OPENTHREAD_OUTPUT_DIR)/config.args
	@echo "$(HDR_PREFIX)CONFIGURE OPENTHREAD..."
	$(NO_ECHO)(cd $(OPENTHREAD_OUTPUT_DIR) && $(OPENTHREAD_ROOT)/configure $(OPENTHREAD_CONFIGURE_OPTIONS))

configure-thread : $(OPENTHREAD_OUTPUT_DIR)/config.status

# TODO: bootstrap openthread

build-thread : configure-thread
	@echo "$(HDR_PREFIX)BUILD OPENTHREAD..."
	MAKEFLAGS= make -C $(OPENTHREAD_OUTPUT_DIR) --no-print-directory all V=$(VERBOSE)

install-thread : | build-thread
	@echo "$(HDR_PREFIX)INSTALL OPENTHREAD..."
	$(NO_ECHO)MAKEFLAGS= make -C $(OPENTHREAD_OUTPUT_DIR) --no-print-directory install V=$(VERBOSE)
	$(NO_ECHO)$(INSTALL) $(INSTALLFLAGS) $(OPENTHREAD_PLATFORM_HEADERS) $(OPENTHREAD_OUTPUT_DIR)/include/openthread/platform

clean-thread:
	@echo "$(HDR_PREFIX)RM $(OPENTHREAD_OUTPUT_DIR)"
	$(NO_ECHO)rm -rf $(OPENTHREAD_OUTPUT_DIR)

$(OPENTHREAD_OUTPUT_DIR) :
	@echo "$(HDR_PREFIX)MKDIR $$@"
	$(NO_ECHO)mkdir -p "$$@"

endef

endif


# ==================================================
# OpenThread-specific help definitions
# ==================================================

ifneq ($(USE_PREBUILT_OPENTHREAD),1)

define TargetHelp +=


  config-thread         Run the OpenThread configure script.
  
  build-thread          Build the OpenThread libraries.
  
  install-thread        Install OpenThread libraries and headers in 
                        build output directory for use by application.
  
  clean-thread          Clean all build outputs produced by the OpenThread
                        build process.
endef

endif
