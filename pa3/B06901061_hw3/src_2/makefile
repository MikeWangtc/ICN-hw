# CC and CFLAGS are varilables
CC = g++
C11 = -std=c++11
CFLAGS = -c
# -c option ask g++ to compile the source files, but do not link.
# -g option is for debugging version
# -O3 option is for optimized version
OPTFLAGS = -O3

all: p2
	@echo -n ""

p2: main.o graph.o vertex.o
	$(CC) $(C11) $(OPTFLAGS)  $^ -o $@
main.o : main.cpp graph.h
	$(CC) $(C11) $(OPTFLAGS) -c main.cpp
graph.o : graph.h graph.cpp vertex.h
	$(CC) $(C11) $(OPTFLAGS) -c graph.cpp 
vertex.o : vertex.cpp vertex.h 
	$(CC) $(C11) $(OPTFLAGS) -c vertex.cpp
clean:
		rm -rf *.o p2 *_out2.txt

