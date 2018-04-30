CFLAGS=-g -Wall -Wextra -pedantic -std=c11
EXE=program

ODIR=obj
CC=gcc
SOURCES=$(wildcard *.c)
OBJECTS=$(foreach o, $(patsubst %.c,%.o,$(SOURCES)), $(ODIR)/$o)

program: $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $(EXE)

reset:
	cp ../notebooks/*.nb .

clean:
	rm -f $(EXE)
	rm -f $(ODIR)/*.o
