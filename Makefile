CXX=g++
CXXFLAGS+=-I/usr/include -I/usr/local/include -Wall -Werror -pedantic -O2
LDFLAGS+=-lrt -lzmq -lboost_program_options

all: bench

clean:
	find . -maxdepth 1 -type f -perm -+x ! -name '*.*' | xargs rm -f
	rm -f *.o

.PHONY: clean

