#############################
## Makefile - Jacob Adkins ##
#############################

# compilation definitions
CXX = g++
CXXFLAGS = -Wall -std=c++14 -lsqlite3

# makefile targets
all : o.o

o.o : Main.cpp 
	${CXX} $^ ${CXXFLAGS} -o $@

clean :
	\rm -f *.o *.txt *.exe

###### End of Makefile ######
