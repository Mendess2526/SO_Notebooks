CFLAGS=-g -Wall -Wextra -pedantic -std=c11
IDIR=../src
SRCDIR=../src
SRCODIR=$(SRCDIR)/obj
EXE=.test
CC=gcc

SRC=$(filter-out %main.c, $(wildcard $(SRCDIR)/*.c))
OBJECTS=$(foreach o, $(patsubst $(SRCDIR)%, $(SRCODIR)%, $(patsubst %.c, %.o, $(SRC))), $o)
TESTS=$(wildcard *.c)
TEXES=$(patsubst %.c, %$(EXE), $(TESTS))

print-% : ; @echo $* = $($*)

$(SRCODIR)/%.o :
	$(CC) $(CFLAGS) -I$(IDIR) -c -o $@ $(patsubst %.o, %.c, $(patsubst $(SRCODIR)%, $(SRCDIR)%, $@))

%$(EXE): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) $(patsubst %$(EXE), %.c, $@) -o $@ -I$(IDIR)

.PHONY: all

all: clean $(TEXES)

reset:
	cp ../notebooks/*.nb .

clean:
	rm -f $(SRCODIR)/*.o
	rm -f *$(EXE)
	rm -f $(TESTODIR)/*.o
