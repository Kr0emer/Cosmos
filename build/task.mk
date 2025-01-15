MAKEFLAGS = -s
KERNELCE_PATH = ../task/
HEADFILE_PATH = -I ../include -I ../include/bastypeinc -I ../include/halinc -I ../include/knlinc -I ../include/libinc -I ../include/drvinc
CCBUILDPATH = $(KERNELCE_PATH)
include krnlbuidcmd.mk
include krnlobjs.mk
.PHONY : all everything build_kernel
all: build_kernel
build_kernel:everything
everything :$(BUILD_MK_TASK_OBJS)
include krnlbuidrule.mk
