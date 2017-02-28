TARGET = 1ST_READ
OBJS = main.o \
romdisk.o gamescreen.o system.o stage.o enemies.o player.o collision.o userinterface.o gamestate.o \
continuescreen.o titlescreen.o congratsscreen.o gameoverscreen.o sound.o
OPTFLAGS=-O3 -fomit-frame-pointer -fno-delayed-branch -DDREAMCAST -Wall -Werror
DEVELOP_CFLAGS= $(OPTFLAGS) -DDEVELOP
DEPLOY_CFLAGS= $(OPTFLAGS)
KOS_CFLAGS+= $(OPTFLAGS) 
KOS_ROMDISK_DIR = romdisk_boot

#all: $(TARGET).elf #problematic when working with libtari

all: complete

complete: clean build_images build

deploy: clean build_images to_romdisk build


build_images: copy_debug genlevels

copy_debug:
	$(KOS_BASE)/utils/kmgenc/kmgenc -a4 $(wildcard assets/debug/*.png)
	find assets/debug -name '*.kmg' | xargs tools/KOMPRESSOR/kompressor
	mkdir romdisk_boot
	mkdir romdisk_boot/fonts
	cp assets/fonts/* romdisk_boot/fonts
	mkdir romdisk_boot/debug
	cp assets/debug/*.pkg romdisk_boot/debug
	mkdir filesystem
	mkdir filesystem/assets

to_romdisk:
	cp -r filesystem/* romdisk_boot

genlevels:
	cp -r assets/* filesystem/assets
	
	find filesystem/assets/ -name '*.png' | xargs $(KOS_BASE)/utils/kmgenc/kmgenc -a4 
	find filesystem/assets/ -name '*.kmg' | xargs tools/KOMPRESSOR/kompressor
	find filesystem/assets/ -name '*.png' | xargs rm -f
	find filesystem/assets/ -name '*.kmg' | xargs rm -f
	find filesystem/assets/ -name '*.xcf' | xargs rm -f
	

clean:
	-rm -f $(TARGET).elf $(OBJS)
	-rm -f $(TARGET).BIN
	-rm -f assets/debug/*.pkg
	-rm -r -f romdisk_boot
	-rm -r -f filesystem
	-rm -f romdisk.img

build: $(OBJS) 
	$(KOS_CC) $(KOS_CFLAGS) -I${KOS_BASE}/../extensions/include $(KOS_LDFLAGS) \
	-o $(TARGET).elf $(KOS_START) \
	$(OBJS) -lkmg $(OPTIONAL_LIBS) -lm -ltremor -ltari $(OBJEXTRA) $(KOS_LIBS)
	$(KOS_OBJCOPY) -O binary $(TARGET).elf $(TARGET).BIN

include $(KOS_BASE)/Makefile.rules
