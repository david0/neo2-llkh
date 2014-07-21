CC=i386-mingw32-gcc
CFLAGS=-std=c99
LD=$(CC)

all: keymapper.exe

keymapper.exe: main.o
				$(LD) -o keymapper.exe $<
