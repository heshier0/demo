#!/bin/sh

BIN             = AudioProcess
PROJ_DIR_NAME   = audio_process

PROJ_PATH         = $(shell pwd)
#PROJ_PATH        = $(TOP_PATH)/$(PROJ_DIR_NAME)
DEBUG = no
#include $(PROJ_PATH)/src/version.mk

CC              = arm-himix200-linux-gcc
CPP             = arm-himix200-linux-g++
AR              = arm-himix200-linux-ar
STRIP           = arm-himix200-linux-strip

INCS            = -I$(PROJ_PATH)/. \
                  -I/opt/hisi-linux/x86-arm/arm-himix200-linux/target/usr/include \
                  -I/mnt/g/workspace/plugins/libev/include \
                  -I/mnt/g/workspace/plugins/libuwsc/include \
                  -I/mnt/g/workspace/plugins/cjson/include    \
                  -I/mnt/g/workspace/plugins/openssl/include             

LIBS            = -L/opt/hisi-linux/x86-arm/arm-himix200-linux/target/usr/lib -lm -ldl -pthread \
                  -L/mnt/g/workspace/plugins/libev/lib -lev \
                  -L/mnt/g/workspace/plugins/libuwsc/lib -luwsc \
                  -L/mnt/g/workspace/plugins/cjson/lib -lcjson \
                  -L/mnt/g/workspace/plugins/openssl/lib -lssl \
                  -L/mnt/g/workspace/plugins/openssl/lib -lcrypto

CFLAGS          = -DLINUX -Wno-multichar -fPIC -O2 -fexpensive-optimizations -frename-registers -fomit-frame-pointer
CPPFLAGS        = -DLINUX -Wno-multichar -fPIC -std=c++11 -fexceptions -fpermissive -w -O2 -fexpensive-optimizations -frename-registers -fomit-frame-pointer


DEBUG_FLAGS     = -fexceptions -finstrument-functions -funwind-tables -g -rdynamic -O0
RELEASE_FLAGS   = -O3 #-Wl, -gc-sections

ifeq ($(DEBUG), no)
    CFLAGS      += $(RELEASE_FLAGS)
    CPPFLAGS    += $(RELEASE_FLAGS)
else
    CFLAGS      += $(DEBUG_FLAGS)
    CPPFLAGS    += $(DEBUG_FLAGS)
endif

BIN_SUFFIX      = $(suffix $(BIN))

SRCS            = *.c
OBJS            = *.o
BULID_CMD       = $(CC) -o $(PROJ_PATH)/$(BIN) $(OBJS) $(LIBS)
INSTALL_CMD     = $(STRIP) $(BIN)
CLEAN_CMD       = rm -f $(PROJ_PATH)/$(BIN) $(COBJS) 

all : $(BIN)

$(OBJS) : $(SRCS)
	@echo $(SRCS)
	@$(CC) -c $(SRCS) $(INCS) $(CFLAGS) $(LIBS)



$(BIN) : $(OBJS) $(CPPOBJS)
	@echo $(BULID_CMD)
	@$(BULID_CMD)
	@echo $(INSTALL_CMD)
	@$(INSTALL_CMD)
	@echo "-rm $(OBJS)"
	@-rm $(OBJS)
	@echo "$(BIN) build success!"
	
clean:
	@echo $(CLEAN_CMD)
	@$(CLEAN_CMD)
	@echo "$(BIN) clean success!"


