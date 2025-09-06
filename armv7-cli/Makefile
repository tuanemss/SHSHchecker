ARCHS = armv7
TARGET = iphone:clang:6.1:6.0

include $(THEOS)/makefiles/common.mk

TOOL_NAME = shshchecker

shshchecker_FILES = main.mm
shshchecker_CFLAGS = -std=c++11 -stdlib=libc++ -I$(THEOS_PROJECT_DIR)/include
shshchecker_LDFLAGS = -lc++ $(THEOS_PROJECT_DIR)/lib/libcurl.a -lz
shshchecker_FRAMEWORKS = Foundation Security CoreFoundation

include $(THEOS_MAKE_PATH)/tool.mk
