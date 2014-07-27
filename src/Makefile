GET=i386-mingw32- #for mac
CC=$(TARGET)gcc
LD=$(TARGET)gcc
WINDRES=$(TARGET)windres
CFLAGS=-std=c99 -O3 -DWINVER=0x500 -DWIN32_WINNT=0x500
LDFLAGS+=-Wl,-subsystem,windows 

ifdef DEBUG
				CFLAGS+= -g
				LDFLAGS:=$(filter-out -mwindows, $(LDFLAGS))
endif

all: neo2llkh.exe

neo2llkh.exe: main.o trayicon.o resources.o
				$(LD) -o $@ $^

%.o: %.rc
				$(WINDRES) -i $^ -o $@
