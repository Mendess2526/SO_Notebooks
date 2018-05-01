CFLAGS=-g -Wall -Wextra -pedantic -std=c11
IDIR=../src
SRCDIR=../src
SRCODIR=$(SRCDIR)/obj
TESTODIR=obj
EXE=test
CC=gcc

SRC=$(wildcard $(SRCDIR)/*.c)
OBJECTS=$(foreach o, $(patsubst $(SRCDIR)%, $(SRCODIR)%, $(patsubst %.c, %.o, $(SRC))), $o)
TESTS=$(wildcard *.c)
TOBJECTS=$(foreach o, $(patsubst %.c, %.o, $(TESTS)), $(TESTODIR)/$o)

print-% : ; @echo $* = $($*)

$(SRCODIR)/%.o :
	$(CC) $(CFLAGS) -I$(IDIR) -c -o $@ $(patsubst %.o, %.c, $(patsubst $(SRCODIR)%, $(SRCDIR)%, $@))

$(TESTODIR)/%.o : %.c
	$(CC) $(CFLAGS) -I$(IDIR) -c -o $@ $<

testes: $(OBJECTS) $(TOBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) $(TOBJECTS) -o $(EXE) -I$(IDIR)

reset:
	cp ../notebooks/*.nb .

clean:
	rm -f $(EXE)
	rm -f $(SRCODIR)/*.o
	rm -f $(TESTODIR)/*.o
