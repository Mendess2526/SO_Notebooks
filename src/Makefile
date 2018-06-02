OS := $(shell uname)
ifeq ($(OS),Linux)
    CFLAGS= -D_POSIX_C_SOURCE -g -Wall -Wextra -pedantic -std=c11
else
    CFLAGS= -g -Wall -Wextra -pedantic -std=c11 -O2
endif


ELF=notebook
ODIR=obj
CC=gcc

SOURCES=$(wildcard *.c)
OBJECTS=$(foreach o, $(patsubst %.c, %.o, $(SOURCES)), $(ODIR)/$o)

print-% : ; @echo $* = $($*)

$(ODIR)/%o :
	$(CC) $(CFLAGS) -c -o $@ $(patsubst %.o, %.c, $(patsubst $(ODIR)/%, %, $@))

program: wizard clean $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $(ELF)

notebooks:
	cp ../notebooks/*.nb .

clean:
	rm -f $(ELF)
	rm -f $(ODIR)/*.o

wizard:
	@echo "\n"
	@cat .art/wizard
