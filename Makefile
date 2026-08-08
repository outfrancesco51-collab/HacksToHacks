CC = gcc
CFLAGS = -Wall -Iraylib/include
LDFLAGS = -Lraylib/lib -lraylib -lgdi32 -lwinmm

all: bin/HacksToHacks.exe

bin/HacksToHacks.exe: src/main.c
	if not exist bin mkdir bin
	$(CC) $(CFLAGS) src/main.c -o bin/HacksToHacks.exe $(LDFLAGS)

clean:
	if exist bin rmdir /s /q bin
