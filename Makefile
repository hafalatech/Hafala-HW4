KERNELDIR = /usr/src/linux-2.4.18-14custom
include $(KERNELDIR)/.config
CFLAGS = -D __KERNEL__ -D MODULE -I $(KERNELDIR)/include -O -Wall
all:	hw4.o