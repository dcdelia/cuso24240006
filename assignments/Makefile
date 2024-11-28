PIN_ROOT := $(HOME)/pin

# Pin's default config
CONFIG_ROOT := $(PIN_ROOT)/source/tools/Config
include $(CONFIG_ROOT)/makefile.config
include $(TOOLS_ROOT)/Config/makefile.default.rules

# custom Pin tools
profiler: localdirs obj-intel64/inscounter.so obj-intel64/icalls.so obj-intel64/callstack.so

localdirs:
	mkdir -p obj-intel64
