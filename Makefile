BASECFLAGS=-O2 -pedantic -Wall -Werror
CFLAGS+=$(BASECFLAGS) -std=c99
CXXFLAGS+=$(BASECFLAGS)
LDFLAGS+=-lzmq -lboost_program_options -lueye_api -lm 
SRC=src

PRG=$(patsubst %.cc,%,$(wildcard $(SRC)/*.cc))
PRG+=$(patsubst %.c,%,$(wildcard $(SRC)/*.c))

all: $(PRG)

clean:
	find . -name "*.o" -o -name "*.pyc" | xargs rm -f
	rm -f $(PRG)

.PHONY: clean

