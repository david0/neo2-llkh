CFLAGS=-std=c99
LD=$(CC)

all: keymapper.exe

keymapper.exe: main.o
				$(LD) -o keymapper.exe $<
