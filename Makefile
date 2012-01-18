BASECFLAGS=-O2 -pedantic -Wall -Werror -I/opt/OpenCV-2.3.1/include/opencv2
CFLAGS+=$(BASECFLAGS) -std=c99
CXXFLAGS+=$(BASECFLAGS)
LDFLAGS+=-lzmq -lboost_program_options -lueye_api -lm -lopencv

PRG=$(patsubst %.cc,%,$(wildcard *.cc))
PRG+=$(patsubst %.c,%,$(wildcard *.c))

all: $(PRG)

clean:
	rm -f *.o *.pyc $(PRG)

.PHONY: clean

