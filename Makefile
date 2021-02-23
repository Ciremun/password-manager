CFLAGS=-Wall -Wextra -pedantic
SOURCES=src/main.c src/aes.c src/b64/encode.c src/b64/decode.c src/b64/buffer.c

main:
	$(CC) $(SOURCES) $(CFLAGS) -o pm

debug: CFLAGS += -g
debug: main

python:
	-python3 update_help.py
	$(CC) $(SOURCES) $(CFLAGS) -o pm
