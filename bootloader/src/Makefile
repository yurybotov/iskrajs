PROJECT = iskramsd
BUILD_DIR = bin

SHARED_DIR = ./../../common
CFILES = iskramsd.c
CFILES += depend.c
CFILES += romdisk.c
CFILES += serialcore.c
CFILES += serialbuf.c
CFILES += jumpers.c
CFILES += msc.c
CFILES += cdc.c
CFILES += usb.c
CFILES += systemtick.c
AFILES += brokers.S

# TODO - you will need to edit these two lines!
#DEVICE=stm32f407vgt6
#DEVICE=stm32f405rgt6
DEVICE=stm32f411re

OOCD_FILE = board/stm32f4discovery.cfg
#OOCD_FILE = board/stm32f411.cfg

# You shouldn't have to edit anything below here.
VPATH += $(SHARED_DIR)
INCLUDES += $(patsubst %,-I%, . $(SHARED_DIR))
OPENCM3_DIR=../libopencm3

include $(OPENCM3_DIR)/mk/genlink-config.mk
include ../rules.mk
include $(OPENCM3_DIR)/mk/genlink-rules.mk
