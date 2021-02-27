CFLAGS=-Wall -Wextra -pedantic
SOURCES=src/aes.c \
src/b64/encode.c src/b64/decode.c src/b64/buffer.c \
src/io/unix.c src/io/common.c \
src/rand.c \
src/parse.c

main:
	$(CC) src/main.c $(SOURCES) $(CFLAGS) -o pm

debug: CFLAGS += -g
debug: main

python:
	-python3 update_help.py
	$(CC) $(SOURCES) $(CFLAGS) -o pm

test:
	$(CC) tests/test.c $(SOURCES) $(CFLAGS) -ggdb -o tests/test
	cd tests/ && ./test
