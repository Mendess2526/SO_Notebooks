CFLAGS=-g -Wall -Wextra -pedantic -std=c11
EXE=program

ODIR=obj
CC=gcc
SOURCES=$(wildcard *.c)
OBJECTS=$(foreach o, $(patsubst %.c, %.o, $(SOURCES)), $(ODIR)/$o)

print-% : ; @echo $* = $($*)

$(ODIR)/%o :
	$(CC) $(CFLAGS) -c -o $@ $(patsubst %.o, %.c, $(patsubst $(ODIR)/%, %, $@))

program: $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $(EXE)

notebooks:
	cp ../notebooks/*.nb .

clean:
	rm -f $(EXE)
	rm -f $(ODIR)/*.o
