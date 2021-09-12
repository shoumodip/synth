LIBS = -lSDL2 -lm
CFLAGS = -Wall -Wextra -std=c11 -pedantic -ggdb
SOURCES = $(wildcard src/*)

synth: $(SOURCES)
	$(CC) -o synth $(SOURCES) $(CFLAGS) $(LIBS)
