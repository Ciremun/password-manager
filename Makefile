CFLAGS=-Wall -Wextra -pedantic
SOURCES=src/main.c src/aes.c src/b64/encode.c src/b64/decode.c src/b64/buffer.c

all:
	$(CC) $(SOURCES) $(CFLAGS) -o pm
