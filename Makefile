CC=gcc
CFLAGS= -O2
CILK=g++
CILKFLAGS= -O2 -fcilkplus -lcilkrts
LDFLAGS= -L$(CURDIR)
AR=ar

all: bfs

bfs : bfs.cpp graph.cpp graph.h bag.cpp bag.h util.h Makefile
	$(CILK) $(CILKFLAGS) $@.cpp $(LDFLAGS) -o $@

clean :
	rm -f bfs *~
