CC = gcc
CFLAGS = -Wall -Iraylib/include
LDFLAGS = -Lraylib/lib -lraylib -lgdi32 -lwinmm

all: bin/HacksToHacks.exe

bin/HacksToHacks.exe: src/main.c src/cJSON.c src/tween.c
	if not exist bin mkdir -p bin
	cp -r locales bin/ || true
	cp -r assets bin/ || true
	$(CC) $(CFLAGS) src/main.c src/cJSON.c src/tween.c -o bin/HacksToHacks.exe $(LDFLAGS)

clean:
	if exist bin rm -rf bin
