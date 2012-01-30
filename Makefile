BASECFLAGS:=-O2 -pedantic -Wall -Werror
CFLAGS+=$(BASECFLAGS) -std=c99
CXXFLAGS+=$(BASECFLAGS)
LDFLAGS+=-lzmq -lboost_program_options -lueye_api -lm 
SRC:=src

# Files which are only linked against, not built as executables
#BLACKLIST:=$(SRC)/foo.c

PRG:=$(patsubst %.cc,%,$(filter-out $(BLACKLIST),$(wildcard $(SRC)/*.cc)))
PRG+=$(patsubst %.c,%,$(filter-out $(BLACKLIST),$(wildcard $(SRC)/*.c)))

all: $(PRG)

# Example of how to add dependencies to `snap'
#$(SRC)/snap: $(SRC)/foo.o

clean:
	find . -name "*.o" -o -name "*.pyc" | xargs rm -f
	rm -f $(PRG)

.PHONY: clean

