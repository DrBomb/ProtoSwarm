

DEFINE = -DPOSIX \
	-DZCL_ENABLE_TIME_SERVER \
	-DXBEE_DEVICE_ENABLE_ATMODE \
	-DXBEE_XMODEM_TESTING

# directory for driver
DRIVER = ../xbee_ansic_library

# path to include files
INCDIR = $(DRIVER)/include

SRCDIR= $(DRIVER)/src

COMPILE = g++ -iquote$(INCDIR) -g -MMD -MP -Wall $(DEFINE)

main_OBJECTS= main.o \
			  comm.o \
			  xbee_device.o \
			  xbee_atcmd.o \
			  xbee_serial_posix.o \
			  xbee_platform_posix.o \
			  wpan_aps.o \
			  wpan_types.o \
			  hexdump.o \
			  hexstrtobyte.o \
			  swapbytes.o \
			  zigbee_zcl.o \
			  zcl_types.o 
all: main

clean :
	- rm -f *.o *.d main

main.o: main.cpp
	$(COMPILE) -x c++ -std=c++11 -c main.cpp

comm.o: comm.cpp
	$(COMPILE) -x c++ -std=c++11 -c comm.cpp

xbee_device.o:
	$(COMPILE) -c $(SRCDIR)/xbee/xbee_device.c

xbee_atcmd.o:
	$(COMPILE) -c $(SRCDIR)/xbee/xbee_atcmd.c

xbee_serial_posix.o:
	$(COMPILE) -c $(SRCDIR)/posix/xbee_serial_posix.c

xbee_platform_posix.o:
	$(COMPILE) -c $(SRCDIR)/posix/xbee_platform_posix.c

wpan_aps.o:
	$(COMPILE) -c $(SRCDIR)/wpan/wpan_aps.c

wpan_types.o:
	$(COMPILE) -c $(SRCDIR)/wpan/wpan_types.c

zigbee_zcl.o:
	$(COMPILE) -c $(SRCDIR)/zigbee/zigbee_zcl.c

zigbee_zdo.o:
	$(COMPILE) -c $(SRCDIR)/zigbee/zigbee_zdo.c

zcl_types.o:
	$(COMPILE) -c $(SRCDIR)/zigbee/zcl_types.c

hexdump.o:
	$(COMPILE) -c $(SRCDIR)/util/hexdump.c

hexstrtobyte.o:
	$(COMPILE) -c $(SRCDIR)/util/hexstrtobyte.c

swapbytes.o:
	$(COMPILE) -c $(SRCDIR)/util/swapbytes.c

main: $(main_OBJECTS)
	$(COMPILE) $(main_OBJECTS) -pthread -o main

