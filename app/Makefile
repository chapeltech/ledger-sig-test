ifeq ($(BOLOS_SDK),)
$(error Environment variable BOLOS_SDK is not set)
endif
include $(BOLOS_SDK)/Makefile.defines

ifeq ($(file <.target),)
$(file >.target,$(TARGET_NAME)$(DEBUG))
else
ifneq ($(TARGET_NAME)$(DEBUG), $(file <.target))
$(info Target switch detected, $(file <.target) -> $(TARGET_NAME)$(DEBUG), cleaning up...)
$(file >.target,$(TARGET_NAME)$(DEBUG))
IGNORE_OUTPUT:=$(shell make clean DEBUG=$(DEBUG))
endif
endif

APPNAME = test-sig

ICONNAME=icons/1.gif
ifeq ($(TARGET_NAME), TARGET_NANOS)
APP_LOAD_FLAGS=0x800  # APPLICATION_FLAG_LIBRARY
TARGET_ID=0x31100004
endif
ifeq ($(TARGET_NAME), TARGET_NANOS2)
APP_LOAD_FLAGS=0x800  # APPLICATION_FLAG_LIBRARY
TARGET_ID=0x33100004
endif
ifeq ($(TARGET_NAME), TARGET_NANOX)
APP_LOAD_FLAGS=0xa40  # APPLICATION_FLAG_LIBRARY + APPLICATION_FLAG_BOLOS_SETTINGS + BLE SUPPORT
TARGET_ID=0x33000004
endif
APP_LOAD_PARAMS=--appFlags $(APP_LOAD_FLAGS) --curve ed25519 --curve secp256k1 --curve secp256r1 --path "44'/1729'" $(COMMON_LOAD_PARAMS)

GIT_DESCRIBE ?= $(shell git describe --tags --abbrev=8 --always --long --dirty 2>/dev/null)

VERSION_TAG ?= $(shell echo "$(GIT_DESCRIBE)" | cut -f1 -d-)
APPVERSION_M=2
APPVERSION_N=3
APPVERSION_P=2
APPVERSION=$(APPVERSION_M).$(APPVERSION_N).$(APPVERSION_P)

# Only warn about version tags if specified/inferred
ifeq ($(VERSION_TAG),)
  $(warning VERSION_TAG not checked)
else
  ifneq (v$(APPVERSION), $(VERSION_TAG))
    $(warning Version-Tag Mismatch: v$(APPVERSION) version and $(VERSION_TAG) tag)
  endif
endif

COMMIT ?= $(shell echo "$(GIT_DESCRIBE)" | awk -F'-g' '{print $$2}' | sed 's/-dirty/*/')
ifeq ($(COMMIT),)
  $(warning COMMIT not specified and could not be determined with git from "$(GIT_DESCRIBE)")
else
  $(info COMMIT=$(COMMIT))
endif

################
# Default rule #
################
all: default bin/app.toml

############
# Platform #
############

DEFINES   += OS_IO_SEPROXYHAL
DEFINES   += HAVE_BAGL HAVE_SPRINTF HAVE_UX_FLOW
DEFINES   += HAVE_IO_USB HAVE_L4_USBLIB IO_USB_MAX_ENDPOINTS=6 IO_HID_EP_LENGTH=64 HAVE_USB_APDU
DEFINES   += HAVE_LEGACY_PID
DEFINES   += VERSION=\"$(APPVERSION)\" APPVERSION_M=$(APPVERSION_M)
DEFINES   += COMMIT=\"$(COMMIT)\" APPVERSION_N=$(APPVERSION_N) APPVERSION_P=$(APPVERSION_P)
# DEFINES   += _Static_assert\(...\)=

ifeq ($(TARGET_NAME),TARGET_NANOX)
DEFINES   += HAVE_BLE BLE_COMMAND_TIMEOUT_MS=2000
DEFINES   += HAVE_BLE_APDU # basic ledger apdu transport over BLE

SDK_SOURCE_PATH  += lib_blewbxx lib_blewbxx_impl
endif

ifeq ($(TARGET_NAME),TARGET_NANOS)
DEFINES   += IO_SEPROXYHAL_BUFFER_SIZE_B=128
else
DEFINES   += IO_SEPROXYHAL_BUFFER_SIZE_B=300
DEFINES   += HAVE_GLO096
endif

# Enabling debug PRINTF
DEBUG ?= 1
ifneq ($(DEBUG),0)
        ifeq ($(TARGET_NAME),TARGET_NANOS)
                DEFINES   += HAVE_PRINTF PRINTF=screen_printf
        else
                DEFINES   += HAVE_PRINTF PRINTF=mcu_usb_printf
        endif
else
        DEFINES   += PRINTF\(...\)=
endif



##############
# Compiler #
##############
ifneq ($(BOLOS_ENV),)
$(info BOLOS_ENV=$(BOLOS_ENV))
CLANGPATH := $(BOLOS_ENV)/clang-arm-fropi/bin/
GCCPATH := $(BOLOS_ENV)/gcc-arm-none-eabi-5_3-2016q1/bin/
CFLAGS += -idirafter $(BOLOS_ENV)/gcc-arm-none-eabi-5_3-2016q1/arm-none-eabi/include
else
$(info BOLOS_ENV is not set: falling back to CLANGPATH and GCCPATH)
endif
ifeq ($(CLANGPATH),)
$(info CLANGPATH is not set: clang will be used from PATH)
endif
ifeq ($(GCCPATH),)
$(info GCCPATH is not set: arm-none-eabi-* will be used from PATH)
endif

CC       := $(CLANGPATH)clang

AS     := $(GCCPATH)arm-none-eabi-gcc

LD       := $(GCCPATH)arm-none-eabi-gcc
LDFLAGS  += -O3 -Os
LDLIBS   += -lm -lgcc -lc

# import rules to compile glyphs(/pone)
include $(BOLOS_SDK)/Makefile.glyphs

### computed variables
APP_SOURCE_PATH  += src
SDK_SOURCE_PATH  += lib_stusb lib_stusb_impl lib_ux

### U2F support
SDK_SOURCE_PATH  += lib_u2f lib_stusb_impl

DEFINES   += USB_SEGMENT_SIZE=64

DEFINES   += U2F_PROXY_MAGIC=\"XTZ\"
DEFINES   += HAVE_IO_U2F HAVE_U2F

bin/app.toml: bin/app.elf
	cp ${ICONNAME} bin/app.gif
	echo "name = \"Signature Stack Test\"" > $@
	echo "version = \"${APPVERSION}\"" >> $@
	echo "[${TARGET_ID}]" >> $@
	echo "binary = \"app.hex\"" >> $@
	echo "icon = \"app.gif\"" >> $@
	echo "flags = \"${APP_LOAD_FLAGS}\"" >> $@
	echo "dataSize = 0" >> $@
	echo "derivationPath = {curves = [\"ed25519\", \"secp256k1\", \"secp256r1\"]}" >> $@
	echo "paths = [\"44'/1729'\"]" >> $@
	echo "apiLevel = \"1\"" >> $@

# import generic rules from the sdk
include $(BOLOS_SDK)/Makefile.rules

#add dependency on custom makefile filename
dep/%.d: src/%.c Makefile

src/parser/generated_patterns.h:
	@echo "Built-in patterns not generated, please `make built-in-patterns` at toplevel"
src/parser/patten_registry.c: src/parser/generated_patterns.h

.PHONY: mrproper

mrproper: clean
	rm -f .target
