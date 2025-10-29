TOP=..
CC=/opt/fcc/bin/fcc
AS=/opt/fcc/bin/asz80
LD=/opt/fcc/bin/ldz80
CPP=/usr/bin/cpp -undef -nostdinc

CFLAGS=-O2 -mz80 -I $(HOME)/dev/libcpm/include -I $(HOME)/dev/libcpm/include/arch/RETRO/
LDFLAGS=-b -C0x100
CPPFLAGS=
LDLIBS=\
	$(HOME)/dev/libcpm/lib/arch/retro/libcpm.a \
	/opt/fcc/lib/z80/libz80.a \
	/opt/fcc/lib/z80/libc.a

CRT0=$(HOME)/dev/libcpm/lib/arch/retro/crt0.o

APPNAME=bombs

.INTERMEDIATE: $(APPNAME).bin

all: $(APPNAME).com

$(APPNAME).bin:$(CRT0) drawbomb.o main.o
	$(LD) $(LDFLAGS) -o $@ $^ $(LDLIBS)

$(APPNAME).com: $(APPNAME).bin
	dd if=$^ of=$@ skip=1 bs=256

drawbomb.s: drawbomb.S
	$(CPP) $(CPPFLAGS) -o $@ $^

clean:
	rm -fv *.s
	rm -fv *.o
	rm -fv $(APPNAME).com

copy:
	cpmrm -f z80-retro-8k-8m /dev/loop0p1 0:$(APPNAME).com
	cpmcp -f z80-retro-8k-8m /dev/loop0p1 $(APPNAME).com 0:

