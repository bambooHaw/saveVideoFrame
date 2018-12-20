SRC:=main.c \
		global.c
APP_NAME := videoApp
PWD := $(shell pwd)

export ARCH=arm
export CROSS_COMPILE=arm-buildroot-linux-gnueabihf-

all:
	$(CROSS_COMPILE)gcc $(SRC) -o $(APP_NAME)
#cp $(APP_NAME) /opt/hxs/projBoxV3/qt5_8/lib/rootfs/opt/ -v

clean:
	rm -rf *.o $(APP_NAME)

